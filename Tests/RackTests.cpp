// mnKarp tests: drive the real rack processor, with the real mnKord.vst3 and mnArpee.vst3 loaded, without a host.
// Build: -DMNKARP_BUILD_TESTS=ON, target mnKarpTests. The plugins are the ones built next to this repository (see
// CMakeLists.txt) when they exist, else the installed ones. mnArpee's presets go to a temporary folder.

#include "KarpEditor.h"
#include "KarpProcessor.h"

#include <cstdio>
#include <cstdlib>
#include <memory>
#include <set>

namespace
{
int failures = 0;

void check(bool condition, const char* name)
{
    if (! condition)
        ++failures;
    std::printf("%s: %s\n", condition ? "ok" : "FAIL", name);
}

constexpr double sampleRate = 44100.0;
constexpr int blockSize = 512;

struct Notes
{
    std::multiset<int> on, off;
    bool allNotesOff = false;
};

// Renders `blocks` blocks (the first one gets `in`) and collects what comes out.
Notes run(MnKarpAudioProcessor& rack, int blocks, const juce::MidiBuffer& in = {})
{
    Notes out;
    juce::AudioBuffer<float> audio(0, blockSize);
    for (int b = 0; b < blocks; ++b)
    {
        juce::MidiBuffer midi;
        if (b == 0)
            midi = in;
        rack.processBlock(audio, midi);
        for (const auto meta : midi)
        {
            const auto msg = meta.getMessage();
            if (msg.isNoteOn())
                out.on.insert(msg.getNoteNumber());
            else if (msg.isNoteOff())
                out.off.insert(msg.getNoteNumber());
            else if (msg.isAllNotesOff())
                out.allNotesOff = true;
        }
    }
    return out;
}

juce::MidiBuffer noteOn(int note)
{
    juce::MidiBuffer m;
    m.addEvent(juce::MidiMessage::noteOn(1, note, (juce::uint8) 100), 0);
    return m;
}

void setStage(MnKarpAudioProcessor& rack, MnKarpAudioProcessor::Stage stage, bool on)
{
    rack.getApvts().getParameter(MnKarpAudioProcessor::stageParamId(stage))->setValueNotifyingHost(on ? 1.0f : 0.0f);
}

// The host program of mnKord with this name (its programs are its presets).
int kordProgram(MnKarpAudioProcessor& rack, const juce::String& name)
{
    auto* kord = rack.getStageProcessor(MnKarpAudioProcessor::kordStage);
    for (int i = 0; kord != nullptr && i < kord->getNumPrograms(); ++i)
        if (kord->getProgramName(i) == name)
            return i;
    return -1;
}

// mnKord's "Default" preset: 60 plays the chord 60 64 67 (62 and 64 have chords too).
std::unique_ptr<MnKarpAudioProcessor> makeRack()
{
    auto rack = std::make_unique<MnKarpAudioProcessor>();
    rack->prepareToPlay(sampleRate, blockSize);
    if (auto* kord = rack->getStageProcessor(MnKarpAudioProcessor::kordStage))
        kord->setCurrentProgram(juce::jmax(0, kordProgram(*rack, "Default")));
    return rack;
}

// One of mnArpee's parameters, by its name.
juce::AudioProcessorParameter* arpeeParameter(MnKarpAudioProcessor& rack, const juce::String& name)
{
    if (auto* arpee = rack.getStageProcessor(MnKarpAudioProcessor::arpeeStage))
        for (auto* parameter : arpee->getParameters())
            if (parameter->getName(64) == name)
                return parameter;
    return nullptr;
}

void setVariable(const char* variable, const juce::String& value)
{
   #if JUCE_WINDOWS
    _putenv_s(variable, value.toRawUTF8());
   #else
    setenv(variable, value.toRawUTF8(), 1);
   #endif
}

void useTestPlugin(const char* variable, const juce::File& file)
{
    if (juce::SystemStats::getEnvironmentVariable(variable, {}).isEmpty() && file.exists())
        setVariable(variable, file.getFullPathName());
}
}

int main(int argc, char** argv)
{
    std::setvbuf(stdout, nullptr, _IONBF, 0);
    juce::ScopedJuceInitialiser_GUI juceInit;

    useTestPlugin("MNKARP_MNKORD_VST3", juce::File(MNKARP_TEST_KORD_VST3));
    useTestPlugin("MNKARP_MNARPEE_VST3", juce::File(MNKARP_TEST_ARPEE_VST3));
    const auto arpeePresets = juce::File::getSpecialLocation(juce::File::tempDirectory).getChildFile("mnKarpTestArpee");
    setVariable("MNARPEE_PRESET_DIR", arpeePresets.getFullPathName());

    {
        MnKarpAudioProcessor rack;
        std::printf("mnKord:  %s\nmnArpee: %s\n",
                    MnKarpAudioProcessor::findPlugin(MnKarpAudioProcessor::kordStage).getFullPathName().toRawUTF8(),
                    MnKarpAudioProcessor::findPlugin(MnKarpAudioProcessor::arpeeStage).getFullPathName().toRawUTF8());
        check(rack.getStageProcessor(MnKarpAudioProcessor::kordStage) != nullptr, "plugins: mnKord.vst3 is loaded");
        check(rack.getStageProcessor(MnKarpAudioProcessor::arpeeStage) != nullptr, "plugins: mnArpee.vst3 is loaded");
        if (rack.getStageProcessor(MnKarpAudioProcessor::kordStage) == nullptr
            || rack.getStageProcessor(MnKarpAudioProcessor::arpeeStage) == nullptr)
        {
            std::printf("%s\n%s\n", rack.getLoadError(MnKarpAudioProcessor::kordStage).toRawUTF8(),
                        rack.getLoadError(MnKarpAudioProcessor::arpeeStage).toRawUTF8());
            return 1;
        }
    }

    {
        auto rack = makeRack();
        setStage(*rack, MnKarpAudioProcessor::kordStage, false);
        setStage(*rack, MnKarpAudioProcessor::arpeeStage, false);
        const auto out = run(*rack, 1, noteOn(60));
        check(out.on.size() == 1 && out.on.count(60) == 1, "both off: the MIDI passes untouched");
    }

    {
        auto rack = makeRack();
        setStage(*rack, MnKarpAudioProcessor::arpeeStage, false);
        const auto out = run(*rack, 1, noteOn(60));
        check(out.on == std::multiset<int> { 60, 64, 67 }, "mnKord on, mnArpee off: the key plays the chord");
        check(run(*rack, 1, noteOn(61)).on.empty(), "mnKord on, mnArpee off: a key without a chord plays nothing");
    }

    {
        auto rack = makeRack();
        setStage(*rack, MnKarpAudioProcessor::kordStage, false);
        const auto out = run(*rack, 100, noteOn(60));
        check(! out.on.empty() && out.on.count(60) == out.on.size(),
              "mnKord off, mnArpee on: the single key is arpeggiated");
    }

    {
        auto rack = makeRack();
        const auto out = run(*rack, 100, noteOn(60));
        std::set<int> distinct(out.on.begin(), out.on.end());
        check(! out.on.empty() && distinct.size() >= 2, "both on: the chord is arpeggiated");
        bool inChord = true;
        for (const int n : distinct)
            inChord = inChord && (n % 12 == 0 || n % 12 == 4 || n % 12 == 7);
        check(inChord, "both on: only the notes of the chord are played");
    }

    {
        auto rack = makeRack();
        setStage(*rack, MnKarpAudioProcessor::arpeeStage, false);
        run(*rack, 1, noteOn(60));
        setStage(*rack, MnKarpAudioProcessor::kordStage, false);
        const auto out = run(*rack, 1);
        check(out.off == std::multiset<int> { 60, 64, 67 }, "switching mnKord off releases its chord");
    }

    {
        auto rack = makeRack();
        setStage(*rack, MnKarpAudioProcessor::kordStage, false);
        run(*rack, 40, noteOn(60));
        setStage(*rack, MnKarpAudioProcessor::arpeeStage, false);
        const auto out = run(*rack, 1);
        check(out.allNotesOff || ! out.off.empty(), "switching mnArpee off releases its arpeggio");
        const auto later = run(*rack, 20);
        check(later.on.empty(), "mnArpee off: nothing keeps playing");
    }

    {
        auto rack = makeRack();
        setStage(*rack, MnKarpAudioProcessor::arpeeStage, false);
        rack->setStageFolded(MnKarpAudioProcessor::arpeeStage, true);
        juce::MemoryBlock state;
        rack->getStateInformation(state);

        auto other = std::make_unique<MnKarpAudioProcessor>();
        other->setStateInformation(state.getData(), (int) state.getSize());
        check(other->isStageOn(MnKarpAudioProcessor::kordStage), "state: mnKord stays on");
        check(! other->isStageOn(MnKarpAudioProcessor::arpeeStage), "state: mnArpee stays off");
        check(other->isStageFolded(MnKarpAudioProcessor::arpeeStage)
                  && ! other->isStageFolded(MnKarpAudioProcessor::kordStage),
              "state: the folded stages are kept");
    }

    {
        auto rack = makeRack();
        std::unique_ptr<juce::AudioProcessorEditor> editor(rack->createEditor());
        check(editor != nullptr && editor->getHeight() > 300 && editor->getWidth() >= 600,
              "editor: both plugin windows in the rack");
        std::printf("editor size %d x %d\n", editor->getWidth(), editor->getHeight());
    }

    {
        using Rack = MnKarpAudioProcessor;
        auto rack = makeRack();
        std::unique_ptr<juce::AudioProcessorEditor> base(rack->createEditor());
        auto& editor = static_cast<MnKarpAudioProcessorEditor&>(*base);

        check(editor.getStageEditorWidth(Rack::kordStage) == editor.getStageEditorWidth(Rack::arpeeStage),
              "widths: the two windows are equally wide at the start");
        const auto before = editor.getStageVisualSize(Rack::kordStage).x;
        check(editor.getStageVisualSize(Rack::arpeeStage).x == before, "widths: equal on the screen too");

        auto* arpeeEditor = editor.getStageEditor(Rack::arpeeStage);
        arpeeEditor->setSize(before + 120, arpeeEditor->getHeight());
        editor.updateSize();
        std::printf("widths after resizing mnArpee: %d %d\n", editor.getStageEditorWidth(Rack::kordStage),
                    editor.getStageEditorWidth(Rack::arpeeStage));
        check(editor.getStageEditorWidth(Rack::kordStage) == editor.getStageEditorWidth(Rack::arpeeStage)
                  && editor.getStageEditorWidth(Rack::kordStage) > before,
              "widths: resizing mnArpee resizes mnKord to match");

        auto* kordEditor = editor.getStageEditor(Rack::kordStage);
        kordEditor->setSize(before + 40, kordEditor->getHeight());
        editor.updateSize();
        check(editor.getStageEditorWidth(Rack::arpeeStage) == editor.getStageEditorWidth(Rack::kordStage),
              "widths: resizing mnKord resizes mnArpee to match");
    }

    {
        auto rack = makeRack();
        setStage(*rack, MnKarpAudioProcessor::arpeeStage, false);
        run(*rack, 1, noteOn(60));
        const auto after = rack->getNotesAfter(MnKarpAudioProcessor::kordStage);
        check(after.contains(60) && after.contains(64) && after.contains(67) && ! after.contains(61),
              "flow: the notes after mnKord are the chord");
        run(*rack, 1, [] { juce::MidiBuffer m; m.addEvent(juce::MidiMessage::noteOff(1, 60), 0); return m; }());
        const auto released = rack->getNotesAfter(MnKarpAudioProcessor::kordStage);
        check(! released.contains(60) && ! released.contains(64) && ! released.contains(67),
              "flow: released notes are gone from the display");
    }

    {
        const auto folder = juce::File::getSpecialLocation(juce::File::tempDirectory).getChildFile("mnKarpTestPresets");
        folder.deleteRecursively();
        MnKarpAudioProcessor::overridePresetFolder(folder);

        auto rack = makeRack();
        setStage(*rack, MnKarpAudioProcessor::arpeeStage, false);
        check(rack->savePreset("Pad chords"), "presets: save");
        check(MnKarpAudioProcessor::listPresets() == juce::StringArray { "Pad chords" }, "presets: the list has it");
        check(rack->getPresetName() == "Pad chords", "presets: it is the current one");

        // Change everything, then load it back.
        setStage(*rack, MnKarpAudioProcessor::arpeeStage, true);
        rack->getStageProcessor(MnKarpAudioProcessor::kordStage)->setCurrentProgram(kordProgram(*rack, "Power Chords"));
        setStage(*rack, MnKarpAudioProcessor::arpeeStage, false);
        check(run(*rack, 1, noteOn(60)).on != std::multiset<int> { 60, 64, 67 }, "presets: another mnKord preset");
        run(*rack, 1, [] { juce::MidiBuffer m; m.addEvent(juce::MidiMessage::allNotesOff(1), 0); return m; }());
        setStage(*rack, MnKarpAudioProcessor::arpeeStage, true);
        rack->setZoom(0.7f);
        check(rack->loadPreset("Pad chords"), "presets: load");
        check(! rack->isStageOn(MnKarpAudioProcessor::arpeeStage), "presets: the switches come back");
        check(rack->getZoom() == 0.7f, "presets: the size is not part of a preset");
        check(run(*rack, 1, noteOn(60)).on == std::multiset<int> { 60, 64, 67 }, "presets: mnKord's chords come back");

        juce::MemoryBlock state;
        rack->getStateInformation(state);
        MnKarpAudioProcessor other;
        other.setStateInformation(state.getData(), (int) state.getSize());
        check(other.getPresetName() == "Pad chords", "presets: the project keeps the preset name");

        check(! rack->loadPreset("No such preset"), "presets: a missing preset does not load");
        check(rack->deletePreset("Pad chords") && MnKarpAudioProcessor::listPresets().isEmpty()
                  && rack->getPresetName().isEmpty(),
              "presets: delete");

        MnKarpAudioProcessor::overridePresetFolder({});
        folder.deleteRecursively();
    }

    {
        // The host sees mnArpee's parameters, and they follow each other.
        struct Counter : public juce::AudioProcessorListener
        {
            void audioProcessorParameterChanged(juce::AudioProcessor*, int, float) override { ++changes; }
            void audioProcessorChanged(juce::AudioProcessor*, const ChangeDetails&) override {}
            int changes = 0;
        };

        auto rack = makeRack();
        auto findParameter = [&](const juce::String& id) -> juce::AudioProcessorParameterWithID*
        {
            for (auto* parameter : rack->getParameters())
                if (auto* withId = dynamic_cast<juce::AudioProcessorParameterWithID*>(parameter))
                    if (withId->paramID == id)
                        return withId;
            return nullptr;
        };

        for (const char* id : { "arp_rate", "arp_octave", "arp_gate", "arp_swing", "arp_steps", "arp_latch", "arp_group", "arp_cut" })
            check(findParameter(id) != nullptr, (juce::String("host parameter ") + id).toRawUTF8());

        auto* gate = findParameter("arp_gate");
        auto* source = arpeeParameter(*rack, "Gate %");
        check(gate != nullptr && source != nullptr && gate->getName(64) == "mnArpee Gate %",
              "parameters: named after mnArpee's");

        gate->setValueNotifyingHost(gate->getValue() == 1.0f ? 0.0f : 1.0f);
        check(std::abs(source->getValue() - gate->getValue()) < 1.0e-6f, "parameters: the host sets mnArpee's value");

        Counter counter;
        rack->addListener(&counter);
        source->setValueNotifyingHost(0.5f);
        check(std::abs(gate->getValue() - 0.5f) < 1.0e-6f, "parameters: mnArpee's own change shows in the host's value");
        check(counter.changes == 1, "parameters: the host is told once about mnArpee's change");

        // The same through a rack preset: the loaded value reaches the host.
        const auto folder = juce::File::getSpecialLocation(juce::File::tempDirectory).getChildFile("mnKarpTestParams");
        folder.deleteRecursively();
        MnKarpAudioProcessor::overridePresetFolder(folder);
        source->setValueNotifyingHost(0.25f);
        rack->savePreset("Gate 25");
        source->setValueNotifyingHost(0.9f);
        counter.changes = 0;
        rack->loadPreset("Gate 25");
        check(std::abs(gate->getValue() - source->getValue()) < 1.0e-6f && counter.changes > 0,
              "parameters: a loaded preset is shown to the host");
        rack->removeListener(&counter);
        MnKarpAudioProcessor::overridePresetFolder({});
        folder.deleteRecursively();
    }

    {
        // The host's program list is the list of the rack presets, oldest first.
        const auto folder = juce::File::getSpecialLocation(juce::File::tempDirectory).getChildFile("mnKarpTestPrograms");
        folder.deleteRecursively();
        MnKarpAudioProcessor::overridePresetFolder(folder);

        auto rack = makeRack();
        check(rack->getNumPrograms() == 128 && rack->getProgramName(0) == "-", "programs: 128 slots, empty ones are \"-\"");

        setStage(*rack, MnKarpAudioProcessor::arpeeStage, false);
        rack->savePreset("Zebra");
        juce::Thread::sleep(40);
        setStage(*rack, MnKarpAudioProcessor::arpeeStage, true);
        rack->savePreset("Apple");

        check(rack->getProgramName(0) == "Zebra" && rack->getProgramName(1) == "Apple" && rack->getProgramName(2) == "-",
              "programs: the presets, oldest first");
        check(rack->getCurrentProgram() == 1, "programs: the current one is the last saved");

        rack->setCurrentProgram(0);
        check(rack->getPresetName() == "Zebra" && ! rack->isStageOn(MnKarpAudioProcessor::arpeeStage),
              "programs: choosing a program loads the preset");
        check(rack->getCurrentProgram() == 0, "programs: and it is the current one");

        rack->setCurrentProgram(50);
        check(rack->getPresetName() == "Zebra", "programs: an empty slot changes nothing");

        // A preset file copied in from outside shows up after a refresh.
        folder.getChildFile("Zebra.mnkarp").copyFileTo(folder.getChildFile("Copy.mnkarp"));
        rack->refreshProgramList();
        check(rack->getProgramName(2) == "Copy", "programs: a file added from outside is listed");

        MnKarpAudioProcessor::overridePresetFolder({});
        folder.deleteRecursively();
    }

    {
        // Unsaved changes of a rack preset.
        const auto folder = juce::File::getSpecialLocation(juce::File::tempDirectory).getChildFile("mnKarpTestDirty");
        folder.deleteRecursively();
        MnKarpAudioProcessor::overridePresetFolder(folder);

        auto rack = makeRack();
        check(! rack->isPresetDirty(), "dirty: no preset, nothing to save");
        rack->savePreset("Dirty test");
        check(! rack->isPresetDirty(), "dirty: not after saving");
        setStage(*rack, MnKarpAudioProcessor::arpeeStage, false);
        check(rack->isPresetDirty(), "dirty: a switch changed");
        rack->savePreset("Dirty test");
        check(! rack->isPresetDirty(), "dirty: saved again");
        rack->setZoom(0.6f);
        check(! rack->isPresetDirty(), "dirty: the size is not a change of the preset");
        arpeeParameter(*rack, "Gate %")->setValueNotifyingHost(0.1f);
        check(rack->isPresetDirty(), "dirty: an mnArpee value changed");
        rack->loadPreset("Dirty test");
        check(! rack->isPresetDirty(), "dirty: not after loading");

        juce::MemoryBlock state;
        rack->getStateInformation(state);
        MnKarpAudioProcessor other;
        other.setStateInformation(state.getData(), (int) state.getSize());
        check(other.getPresetName() == "Dirty test" && ! other.isPresetDirty(), "dirty: a project with the saved preset");
        setStage(other, MnKarpAudioProcessor::kordStage, false);
        juce::MemoryBlock edited;
        other.getStateInformation(edited);
        MnKarpAudioProcessor third;
        third.setStateInformation(edited.getData(), (int) edited.getSize());
        check(third.isPresetDirty(), "dirty: a project with edits after the save shows them as unsaved");

        MnKarpAudioProcessor::overridePresetFolder({});
        folder.deleteRecursively();
    }

    {
        // The take: the last stretch of playing, as a MIDI file.
        auto rack = makeRack();
        setStage(*rack, MnKarpAudioProcessor::arpeeStage, false);
        rack->clearRecording();
        rack->collectRecording();
        check(! rack->hasTake(), "take: nothing played, nothing to drag");

        const auto noteOff = [] { juce::MidiBuffer m; m.addEvent(juce::MidiMessage::noteOff(1, 60), 0); return m; };
        run(*rack, 10, noteOn(60));
        run(*rack, 1, noteOff());
        const int silentBlocks = (int) (3.0 * sampleRate / blockSize);   // longer than the 2 s gap
        run(*rack, silentBlocks);
        run(*rack, 20, noteOn(60));
        run(*rack, 1, noteOff());
        rack->collectRecording();
        check(rack->hasTake(), "take: there is one after playing");

        const auto file = juce::File::getSpecialLocation(juce::File::tempDirectory).getChildFile("mnKarpTestTake.mid");
        check(rack->writeTake(file), "take: written as a MIDI file");

        juce::MidiFile midiFile;
        juce::FileInputStream stream(file);
        check(stream.openedOk() && midiFile.readFrom(stream) && midiFile.getNumTracks() == 1, "take: the file reads back");
        if (midiFile.getNumTracks() == 1)
        {
            std::multiset<int> ons;
            double firstOn = -1.0, tempo = 0.0;
            for (const auto* holder : *midiFile.getTrack(0))
            {
                if (holder->message.isNoteOn())
                {
                    ons.insert(holder->message.getNoteNumber());
                    if (firstOn < 0.0)
                        firstOn = holder->message.getTimeStamp();
                }
                if (holder->message.isTempoMetaEvent())
                    tempo = holder->message.getTempoSecondsPerQuarterNote();
            }
            check(ons == std::multiset<int> { 60, 64, 67 }, "take: only the last phrase (the chord once)");
            check(firstOn == 0.0, "take: it starts with its first note");
            check(std::abs(tempo - 0.5) < 1.0e-3, "take: the tempo is in the file (120 BPM without a host)");
        }
        file.deleteFile();
    }

    std::printf("%d failures\n", failures);
    return failures == 0 ? 0 : 1;
}
