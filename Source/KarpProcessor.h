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

    // The rack has no presets of its own: the presets are mnKord's and mnArpee's, chosen in their windows.
    int getNumPrograms() override { return 1; }
    int getCurrentProgram() override { return 0; }
    void setCurrentProgram(int) override {}
    const juce::String getProgramName(int) override { return {}; }
    void changeProgramName(int, const juce::String&) override {}

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
    void timerCallback() override { collectRecording(); }
    void record(const juce::MidiBuffer& midi, int numSamples);

    // Runs one stage on the block, or passes the MIDI through when it is off (or not loaded).
    void runStage(Stage stage, int numSamples, juce::MidiBuffer& midi);
    void loadPlugins();

    static juce::AudioProcessorValueTreeState::ParameterLayout makeLayout();
    void trackNotes(Stage stage, const juce::MidiBuffer& midi);

    juce::AudioPluginFormatManager formats;
    std::array<std::unique_ptr<juce::AudioPluginInstance>, numStages> plugins;
    std::array<juce::String, numStages> loadErrors;
    juce::AudioBuffer<float> stageAudio;   // the plugins' audio buses (they have none, but a host must pass one)

    juce::AudioProcessorValueTreeState apvts;
    std::array<std::atomic<float>*, numStages> stageOn {};
    std::array<std::atomic<bool>, numStages> folded {};
    std::atomic<float> zoom { defaultZoom };
    std::atomic<int> windowWidth { 0 };

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
