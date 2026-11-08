#pragma once
#include <JuceHeader.h>

#include <atomic>

//==============================================================================
// MirrorParameter: a parameter of the rack that is a parameter of a loaded plugin (here a mnArpee one), so
// that the host can see, automate and record it. The two always show the same value:
//  - the host sets the mirror: the source is set, and so is mnArpee's window;
//  - the source changes (the user moves a control in mnArpee's window, a project is loaded): the host is told,
//    and so are the gestures of the control, so the host can record the automation.
// The value is the source's normalised value (0..1), as the plugin gives it to a host.
//
// Without a source (the plugin is not installed) the mirror keeps its value and does nothing. The rack must
// detach() the mirrors before the plugin that owns the source goes away.
//==============================================================================
class MirrorParameter : public juce::RangedAudioParameter,
                        private juce::AudioProcessorParameter::Listener
{
public:
    MirrorParameter(juce::AudioProcessorParameter* sourceParameter, const juce::String& id, const juce::String& name)
        : juce::RangedAudioParameter(juce::ParameterID { id, 1 }, name,
                                     juce::AudioProcessorParameterWithIDAttributes().withLabel(
                                         sourceParameter != nullptr ? sourceParameter->getLabel() : juce::String())),
          source(sourceParameter),
          defaultNormalised(sourceParameter != nullptr ? sourceParameter->getDefaultValue() : 0.0f),
          steps(sourceParameter != nullptr ? sourceParameter->getNumSteps() : getDefaultNumParameterSteps()),
          discrete(sourceParameter != nullptr && sourceParameter->isDiscrete()),
          boolean(sourceParameter != nullptr && sourceParameter->isBoolean()),
          valueStrings(sourceParameter != nullptr ? sourceParameter->getAllValueStrings() : juce::StringArray()),
          lastValue(defaultNormalised)
    {
        if (source != nullptr)
            source->addListener(this);
    }

    ~MirrorParameter() override { detach(); }

    void detach()
    {
        if (source != nullptr)
        {
            lastValue = source->getValue();
            source->removeListener(this);
            source = nullptr;
        }
    }

    const juce::NormalisableRange<float>& getNormalisableRange() const override { return range; }

    float getValue() const override { return source != nullptr ? source->getValue() : lastValue; }

    void setValue(float newValue) override
    {
        if (source == nullptr)
        {
            lastValue = newValue;
            return;
        }

        // The source tells its own listeners (mnArpee's window) and then this one, which must not answer.
        forwarding.store(true);
        source->setValueNotifyingHost(newValue);
        forwarding.store(false);
    }

    float getDefaultValue() const override { return defaultNormalised; }
    juce::String getText(float normalised, int length) const override
    {
        return source != nullptr ? source->getText(normalised, length) : juce::String(normalised, 2).substring(0, length);
    }
    float getValueForText(const juce::String& text) const override
    {
        return source != nullptr ? source->getValueForText(text) : text.getFloatValue();
    }
    int getNumSteps() const override { return steps; }
    bool isDiscrete() const override { return discrete; }
    bool isBoolean() const override { return boolean; }
    juce::StringArray getAllValueStrings() const override { return valueStrings; }

private:
    void parameterValueChanged(int, float newValue) override
    {
        if (! forwarding.load())
            sendValueChangedMessageToListeners(newValue);
    }

    void parameterGestureChanged(int, bool gestureIsStarting) override
    {
        if (forwarding.load())
            return;

        if (gestureIsStarting)
            beginChangeGesture();
        else
            endChangeGesture();
    }

    juce::AudioProcessorParameter* source;
    const juce::NormalisableRange<float> range { 0.0f, 1.0f };
    const float defaultNormalised;
    const int steps;
    const bool discrete, boolean;
    const juce::StringArray valueStrings;
    float lastValue;
    std::atomic<bool> forwarding { false };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MirrorParameter)
};
