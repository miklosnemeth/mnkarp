#include "KarpProcessor.h"

#include "KarpEditor.h"

//==============================================================================
juce::AudioProcessorValueTreeState::ParameterLayout MnKarpAudioProcessor::makeLayout()
{
    juce::AudioProcessorValueTreeState::ParameterLayout layout;
    for (const auto stage : { kordStage, arpeeStage })
        layout.add(std::make_unique<juce::AudioParameterBool>(
            juce::ParameterID { stageParamId(stage), 1 }, juce::String(stageName(stage)) + " On", true));
    return layout;
}

MnKarpAudioProcessor::MnKarpAudioProcessor()
    : AudioProcessor(BusesProperties())
    , apvts(*this, nullptr, "mnKarp", makeLayout())
{
    for (const auto stage : { kordStage, arpeeStage })
    {
        stageOn[(size_t) stage] = apvts.getRawParameterValue(stageParamId(stage));
        folded[(size_t) stage].store(false);
    }

    arpee.setZoom(defaultZoom);

    // mnArpee's own parameters, so that the host can automate them (the rack owns the mirrors, mnArpee the source).
    for (const char* id : { "rate", "octave", "gate", "swing", "steps", "latch", "group", "cut" })
        if (auto* source = dynamic_cast<juce::RangedAudioParameter*>(arpee.getApvts().getParameter(id)))
        {
            auto* mirror = new MirrorParameter(*source, "arp_", "mnArpee ");
            mirrors.push_back(mirror);
            addParameter(mirror);
        }

    refreshProgramList();
}

MnKarpAudioProcessor::~MnKarpAudioProcessor()
{
    cancelPendingUpdate();

    // The mirrors outlive the members below (the processor base class owns them): let go of mnArpee first.
    for (auto* mirror : mirrors)
        mirror->detach();
}

//==============================================================================
void MnKarpAudioProcessor::prepareToPlay(double sampleRate, int samplesPerBlock)
{
    for (juce::AudioProcessor* child : { static_cast<juce::AudioProcessor*>(&kord),
                                         static_cast<juce::AudioProcessor*>(&arpee) })
    {
        child->setRateAndBufferSizeDetails(sampleRate, samplesPerBlock);
        child->prepareToPlay(sampleRate, samplesPerBlock);
    }

    releaseBuffer.ensureSize(16384);
    wasOn = { true, true };
}

void MnKarpAudioProcessor::releaseResources()
{
    kord.releaseResources();
    arpee.releaseResources();
}

template <typename Processor>
void MnKarpAudioProcessor::runStage(Stage stage, Processor& processor, juce::AudioBuffer<float>& audio,
                                    juce::MidiBuffer& midi)
{
    const bool on = isStageOn(stage);
    bool& was = wasOn[(size_t) stage];

    if (on)
    {
        processor.processBlock(audio, midi);
    }
    else if (was)
    {
        // Just switched off: the stage may be sounding notes (a chord, an arpeggio). Let it release
        // them through its own all-notes-off handling; what that gives out goes on to the next stage.
        // The block's own MIDI follows, untouched.
        releaseBuffer.clear();
        for (int channel = 1; channel <= 16; ++channel)
            releaseBuffer.addEvent(juce::MidiMessage::allNotesOff(channel), 0);

        processor.processBlock(audio, releaseBuffer);
        releaseBuffer.addEvents(midi, 0, audio.getNumSamples(), 0);
        midi.swapWith(releaseBuffer);
    }

    was = on;
}

void MnKarpAudioProcessor::processBlock(juce::AudioBuffer<float>& audio, juce::MidiBuffer& midi)
{
    juce::ScopedNoDenormals noDenormals;

    // The children are not hosted by anyone: the rack hands them the host's transport.
    kord.setPlayHead(getPlayHead());
    arpee.setPlayHead(getPlayHead());

    runStage(kordStage, kord, audio, midi);
    trackNotes(kordStage, midi);
    runStage(arpeeStage, arpee, audio, midi);
    trackNotes(arpeeStage, midi);
}

void MnKarpAudioProcessor::setZoom(float factor)
{
    zoom.store(juce::jlimit(0.5f, 1.5f, factor));
    arpee.setZoom(zoom.load());
}

//==============================================================================
juce::AudioProcessorEditor* MnKarpAudioProcessor::createEditor()
{
    return new MnKarpAudioProcessorEditor(*this);
}

//==============================================================================
namespace
{
void storeChild(juce::XmlElement& root, const char* name, juce::AudioProcessor& child)
{
    juce::MemoryBlock block;
    child.getStateInformation(block);
    root.createNewChildElement(name)->addTextElement(block.toBase64Encoding());
}

void restoreChild(const juce::XmlElement& root, const char* name, juce::AudioProcessor& child)
{
    if (auto* element = root.getChildByName(name))
    {
        juce::MemoryBlock block;
        if (block.fromBase64Encoding(element->getAllSubText()) && block.getSize() > 0)
            child.setStateInformation(block.getData(), (int) block.getSize());
    }
}

juce::File presetFolderOverride;
}

std::unique_ptr<juce::XmlElement> MnKarpAudioProcessor::createStateXml(bool forPreset)
{
    auto root = std::make_unique<juce::XmlElement>(forPreset ? "mnKarpPreset" : "mnKarpState");
    root->setAttribute("version", 1);
    for (const auto stage : { kordStage, arpeeStage })
    {
        root->setAttribute(stageParamId(stage), isStageOn(stage));
        if (! forPreset)
            root->setAttribute(juce::String(stageParamId(stage)) + "Folded", isStageFolded(stage));
    }

    if (! forPreset)
    {
        root->setAttribute("zoom", (double) getZoom());
        root->setAttribute("width", getWidth());
        root->setAttribute("preset", presetName);
    }

    storeChild(*root, "Kord", kord);
    storeChild(*root, "Arpee", arpee);
    return root;
}

void MnKarpAudioProcessor::applyStateXml(const juce::XmlElement& root, bool fromPreset)
{
    for (const auto stage : { kordStage, arpeeStage })
    {
        if (auto* parameter = apvts.getParameter(stageParamId(stage)))
            parameter->setValueNotifyingHost(root.getBoolAttribute(stageParamId(stage), true) ? 1.0f : 0.0f);
        if (! fromPreset)
            setStageFolded(stage, root.getBoolAttribute(juce::String(stageParamId(stage)) + "Folded", false));
    }

    restoreChild(root, "Kord", kord);
    restoreChild(root, "Arpee", arpee);

    // The rack's size wins over the zoom mnArpee kept in its own state.
    if (! fromPreset)
    {
        setZoom((float) root.getDoubleAttribute("zoom", defaultZoom));
        setWidth(root.getIntAttribute("width", defaultWidth));
        presetName = root.getStringAttribute("preset");
    }
    else
    {
        setZoom(getZoom());
    }
}

void MnKarpAudioProcessor::getStateInformation(juce::MemoryBlock& destData)
{
    copyXmlToBinary(*createStateXml(false), destData);
}

void MnKarpAudioProcessor::setStateInformation(const void* data, int sizeInBytes)
{
    const auto root = getXmlFromBinary(data, sizeInBytes);
    if (root != nullptr && root->hasTagName("mnKarpState"))
        applyStateXml(*root, false);
}

//==============================================================================
juce::File MnKarpAudioProcessor::getPresetFolder()
{
    if (presetFolderOverride != juce::File())
        return presetFolderOverride;
    return juce::File::getSpecialLocation(juce::File::userDocumentsDirectory)
        .getChildFile("MNAudio").getChildFile("mnKarp").getChildFile("Presets");
}

void MnKarpAudioProcessor::overridePresetFolder(const juce::File& folder)
{
    presetFolderOverride = folder;
}

juce::File MnKarpAudioProcessor::presetFile(const juce::String& name)
{
    return getPresetFolder().getChildFile(juce::File::createLegalFileName(name) + ".mnkarp");
}

juce::StringArray MnKarpAudioProcessor::listPresetsByAge()
{
    auto files = getPresetFolder().findChildFiles(juce::File::findFiles, false, "*.mnkarp");
    std::sort(files.begin(), files.end(), [](const juce::File& a, const juce::File& b)
              {
                  const auto timeA = a.getCreationTime(), timeB = b.getCreationTime();
                  return timeA != timeB ? timeA < timeB : a.getFileName().compareNatural(b.getFileName()) < 0;
              });

    juce::StringArray names;
    for (const auto& file : files)
        names.add(file.getFileNameWithoutExtension());
    return names;
}

juce::StringArray MnKarpAudioProcessor::listPresets()
{
    juce::StringArray names;
    for (const auto& file : getPresetFolder().findChildFiles(juce::File::findFiles, false, "*.mnkarp"))
        names.add(file.getFileNameWithoutExtension());
    names.sortNatural();
    return names;
}

bool MnKarpAudioProcessor::savePreset(const juce::String& name)
{
    if (juce::File::createLegalFileName(name).isEmpty())
        return false;

    const auto file = presetFile(name);
    if (! file.getParentDirectory().createDirectory().wasOk())
        return false;

    if (! createStateXml(true)->writeTo(file))
        return false;

    presetName = file.getFileNameWithoutExtension();
    refreshProgramList();
    return true;
}

bool MnKarpAudioProcessor::loadPreset(const juce::String& name)
{
    const auto file = presetFile(name);
    const auto root = juce::XmlDocument::parse(file);
    if (root == nullptr || ! root->hasTagName("mnKarpPreset"))
        return false;

    applyStateXml(*root, true);
    presetName = file.getFileNameWithoutExtension();
    return true;
}

bool MnKarpAudioProcessor::deletePreset(const juce::String& name)
{
    const auto file = presetFile(name);
    if (! file.existsAsFile() || ! file.moveToTrash())
        return false;

    if (presetName == file.getFileNameWithoutExtension())
        presetName = {};
    refreshProgramList();
    return true;
}

//==============================================================================
void MnKarpAudioProcessor::refreshProgramList()
{
    auto names = listPresetsByAge();
    {
        const juce::ScopedLock lock(programLock);
        if (names == programNames)
            return;
        programNames = std::move(names);
    }
    notifyHostProgramsChanged();
}

void MnKarpAudioProcessor::notifyHostProgramsChanged()
{
    // The host's component handler belongs to the message thread.
    if (! juce::MessageManager::existsAndIsCurrentThread())
    {
        hostNotificationPending.store(true);
        triggerAsyncUpdate();
        return;
    }

    hostNotificationPending.store(false);
    updateHostDisplay(ChangeDetails().withProgramChanged(true));
}

int MnKarpAudioProcessor::getCurrentProgram()
{
    const int pending = pendingProgram.load();
    if (pending >= 0)
        return pending;

    const juce::ScopedLock lock(programLock);
    return juce::jmax(0, programNames.indexOf(presetName));
}

void MnKarpAudioProcessor::setCurrentProgram(int index)
{
    juce::String name;
    {
        const juce::ScopedLock lock(programLock);
        if (index >= 0 && index < programNames.size())
            name = programNames[index];
    }

    // An empty slot: stay on the current preset, and point the host back at it.
    if (name.isEmpty())
    {
        notifyHostProgramsChanged();
        return;
    }

    if (juce::MessageManager::existsAndIsCurrentThread())
    {
        pendingProgram.store(-1);
        cancelPendingUpdate();
        loadPreset(name);
        return;
    }

    // Loading reads files and sets the two plugins; the audio thread must not wait for that.
    pendingProgram.store(index);
    triggerAsyncUpdate();
}

const juce::String MnKarpAudioProcessor::getProgramName(int index)
{
    const juce::ScopedLock lock(programLock);
    return index >= 0 && index < programNames.size() ? programNames[index] : juce::String("-");
}

void MnKarpAudioProcessor::handleAsyncUpdate()
{
    const int index = pendingProgram.exchange(-1);
    if (index >= 0)
    {
        juce::String name;
        {
            const juce::ScopedLock lock(programLock);
            if (index < programNames.size())
                name = programNames[index];
        }
        if (name.isNotEmpty())
            loadPreset(name);
    }

    if (hostNotificationPending.load())
        notifyHostProgramsChanged();
}

//==============================================================================
void MnKarpAudioProcessor::trackNotes(Stage stage, const juce::MidiBuffer& midi)
{
    auto& bits = tracked[(size_t) stage];
    for (const auto meta : midi)
    {
        const auto message = meta.getMessage();
        if (message.isNoteOnOrOff())
        {
            const int note = message.getNoteNumber();
            auto& word = note < 64 ? bits.low : bits.high;
            const auto mask = std::uint64_t(1) << (note & 63);
            if (message.isNoteOn())
                word |= mask;
            else
                word &= ~mask;
        }
        else if (message.isAllNotesOff() || message.isAllSoundOff())
        {
            bits = {};
        }
    }

    published[(size_t) stage][0].store(bits.low, std::memory_order_relaxed);
    published[(size_t) stage][1].store(bits.high, std::memory_order_relaxed);
}

MnKarpAudioProcessor::NoteBits MnKarpAudioProcessor::getNotesAfter(Stage stage) const
{
    return { published[(size_t) stage][0].load(std::memory_order_relaxed),
             published[(size_t) stage][1].load(std::memory_order_relaxed) };
}

//==============================================================================
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new MnKarpAudioProcessor();
}
