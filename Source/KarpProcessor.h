#pragma once
#include <JuceHeader.h>

#include "MirrorParameter.h"

#include <array>
#include <atomic>

//==============================================================================
// MnKarpAudioProcessor: a host for mnKord and mnArpee, like two devices in a rack.
//
// The rack loads the installed plugins (mnKord.vst3 and mnArpee.vst3, see findPlugin()) and runs
// the MIDI through mnKord (key -> chord) and then through mnArpee (chord -> pattern). Each of the
// two can be switched off, and then the MIDI passes it untouched. The two are the plugins
// themselves, so presets, the preset libraries and the editors are theirs. A plugin that is not
// installed is a stage that passes the MIDI through.
//
// Switching a stage off releases what it is sounding (an "all notes off" goes through it
// first), so no note is left hanging in the next stage or the synth.
//
// Project state: the rack's own values (the two switches and which stages are folded) and the
// state of each of the two processors.
//==============================================================================
class MnKarpAudioProcessor : public juce::AudioProcessor,
                             private juce::AsyncUpdater,
                             private juce::Timer
{
public:
    enum Stage { kordStage = 0, arpeeStage = 1 };
    static constexpr int numStages = 2;

    MnKarpAudioProcessor();
    ~MnKarpAudioProcessor() override;

    void prepareToPlay(double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;
    void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;
    using AudioProcessor::processBlock;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }

    const juce::String getName() const override { return "mnKarp"; }
    bool acceptsMidi() const override { return true; }
    bool producesMidi() const override { return true; }
    bool isMidiEffect() const override { return true; }
    double getTailLengthSeconds() const override { return 0.0; }

    // The host's program list is the list of the rack presets, oldest first (a new preset goes to the end, so the
    // numbers a host has stored keep their meaning), padded with empty slots named "-" to a number that never
    // changes. Some hosts change the program from the audio thread; the preset is then loaded on the message thread.
    static constexpr int numHostPrograms = 128;
    int getNumPrograms() override { return numHostPrograms; }
    int getCurrentProgram() override;
    void setCurrentProgram(int index) override;
    const juce::String getProgramName(int index) override;
    void changeProgramName(int, const juce::String&) override {}
    // Reads the preset folder again (message thread): after a save or delete, or files added from outside.
    void refreshProgramList();

    void getStateInformation(juce::MemoryBlock& destData) override;
    void setStateInformation(const void* data, int sizeInBytes) override;

    // The loaded plugin of a stage, or nullptr when it could not be loaded (then getLoadError() says why).
    juce::AudioPluginInstance* getStageProcessor(Stage stage) const { return plugins[(size_t) stage].get(); }
    juce::String getLoadError(Stage stage) const { return loadErrors[(size_t) stage]; }
    // Where the plugin of a stage is: the file named by the environment variable MNKARP_MNKORD_VST3 or
    // MNKARP_MNARPEE_VST3 when it is set, else the plugin in the system's VST3 folder (in its MNAudio folder,
    // directly in it, or anywhere below it). An empty File when there is none.
    static juce::File findPlugin(Stage stage);
    juce::AudioProcessorValueTreeState& getApvts() { return apvts; }

    static const char* stageParamId(Stage stage) { return stage == kordStage ? "kordOn" : "arpeeOn"; }
    static const char* stageName(Stage stage) { return stage == kordStage ? "mnKord" : "mnArpee"; }
    bool isStageOn(Stage stage) const { return stageOn[(size_t) stage]->load() > 0.5f; }

    // The size of the two windows (0.5 .. 1.5, default 0.75: with the bars the rack is then under 1000 px
    // tall). The editor asks both plugins to scale their windows by it.
    static constexpr float defaultZoom = 0.75f;
    float getZoom() const { return zoom.load(); }
    void setZoom(float factor);

    // The width both windows have on the screen. They are always equally wide: mnKord's window and mnArpee's
    // both take any width from 848 to 1600 (times the zoom), and whichever one is resized sets it for the other.
    // 0 = not known yet (the editor takes the plugins' own width then).
    int getWidth() const { return windowWidth.load(); }
    void setWidth(int width) { windowWidth.store(juce::jmax(0, width)); }

    // Rack presets: the two plugins' presets (as they are now) and the two switches under one name, kept as
    // files in Documents\MNAudio\mnKarp\Presets, shared by every project. The size and the folding of the
    // windows are not part of a preset. Message thread only.
    static juce::File getPresetFolder();
    // Relocates the folder (for tests). An empty File restores the default.
    static void overridePresetFolder(const juce::File& folder);
    static juce::StringArray listPresets();   // the names, sorted
    static juce::StringArray listPresetsByAge();   // the names, oldest first (the order of the host's programs)
    // The preset last saved or loaded in this project ("" = none).
    juce::String getPresetName() const { return presetName; }
    bool savePreset(const juce::String& name);
    bool loadPreset(const juce::String& name);
    bool deletePreset(const juce::String& name);
    // Whether the current state differs from the rack preset as it was loaded or saved (false without a preset).
    bool isPresetDirty();

    // The notes sounding after a stage (0 = after mnKord, 1 = after mnArpee), for the editor's display:
    // bit n of the 128-bit set is note n.
    struct NoteBits
    {
        std::uint64_t low = 0, high = 0;
        bool contains(int note) const { return note < 64 ? ((low >> note) & 1u) != 0 : ((high >> (note - 64)) & 1u) != 0; }
    };
    NoteBits getNotesAfter(Stage stage) const;

    // The take: the rack's MIDI output, to drag into the DAW as a clip. The audio thread queues every note,
    // controller and pitch bend the rack gives out, the message thread keeps the last 10 minutes of them. The take
    // is the latest stretch of playing: it starts with the first note after 2 seconds in which no note sounded
    // (or where the host's transport jumped back, at a loop), and goes on to the last event.
    struct RecordedEvent
    {
        double seconds = 0.0;   // since the rack started playing
        double beat = 0.0;      // the host's position in quarter notes while its transport runs, else a free count
        double bpm = 120.0;
        bool hostBeat = false;  // beat is the host's position (the take can then start on a bar)
        std::uint8_t bytes[3] {};
        std::uint8_t size = 0;
    };
    static constexpr double takeGapSeconds = 2.0;
    static constexpr double keptSeconds = 600.0;
    // Moves what the audio thread recorded into the history (message thread; a timer does it ten times a second).
    void collectRecording();
    std::vector<RecordedEvent> getTake() const;
    bool hasTake() const;
    // Writes the take as a standard MIDI file (960 ticks per quarter note, with the tempo it was played at).
    bool writeTake(const juce::File& file) const;
    void clearRecording();

    // Folding a stage hides its editor (the rack gets shorter); it does not change the sound.
    bool isStageFolded(Stage stage) const { return folded[(size_t) stage].load(); }
    void setStageFolded(Stage stage, bool shouldBeFolded) { folded[(size_t) stage].store(shouldBeFolded); }

private:
    void handleAsyncUpdate() override;
    void timerCallback() override { collectRecording(); }
    void notifyHostProgramsChanged();
    void record(const juce::MidiBuffer& midi, int numSamples);

    // Runs one stage on the block, or passes the MIDI through when it is off (or not loaded).
    void runStage(Stage stage, int numSamples, juce::MidiBuffer& midi);
    void loadPlugins();

    static juce::AudioProcessorValueTreeState::ParameterLayout makeLayout();
    std::unique_ptr<juce::XmlElement> createStateXml(bool forPreset);
    void applyStateXml(const juce::XmlElement& root, bool fromPreset);
    static juce::File presetFile(const juce::String& name);
    void trackNotes(Stage stage, const juce::MidiBuffer& midi);
    // The state a rack preset holds, as text.
    juce::String presetSnapshot();
    std::unique_ptr<juce::XmlElement> createPresetXml();

    juce::AudioPluginFormatManager formats;
    std::array<std::unique_ptr<juce::AudioPluginInstance>, numStages> plugins;
    std::array<juce::String, numStages> loadErrors;
    juce::AudioBuffer<float> stageAudio;   // the plugins' audio buses (they have none, but a host must pass one)

    juce::AudioProcessorValueTreeState apvts;
    std::array<std::atomic<float>*, numStages> stageOn {};
    std::array<std::atomic<bool>, numStages> folded {};
    std::atomic<float> zoom { defaultZoom };
    std::atomic<int> windowWidth { 0 };

    juce::String presetName;
    juce::String savedSnapshot;   // presetSnapshot() when the preset was loaded or saved
    juce::CriticalSection programLock;
    juce::StringArray programNames;           // the rack presets, as the host's programs
    std::atomic<int> pendingProgram { -1 };
    std::atomic<bool> hostNotificationPending { false };
    // The parameters of mnArpee that the rack shows to the host.
    std::vector<MirrorParameter*> mirrors;

    // The notes after each stage: kept by the audio thread, published for the editor.
    std::array<NoteBits, numStages> tracked {};
    std::array<std::array<std::atomic<std::uint64_t>, 2>, numStages> published {};

    // The take. The fifo is written by the audio thread only, read by the message thread only.
    static constexpr int recordCapacity = 8192;
    juce::AbstractFifo recordFifo { recordCapacity };
    std::vector<RecordedEvent> recordBuffer = std::vector<RecordedEvent>(recordCapacity);
    std::vector<RecordedEvent> history;   // message thread
    double recordSeconds = 0.0;           // audio thread: the clock of the recording
    double freeBeat = 0.0;

    // Audio thread.
    std::array<bool, numStages> wasOn { true, true };
    juce::MidiBuffer releaseBuffer;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MnKarpAudioProcessor)
};
