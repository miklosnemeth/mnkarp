#pragma once
#include <JuceHeader.h>

#include <atomic>

//==============================================================================
// MirrorParameter: a parameter of the rack that is another processor's parameter (here a mnArpee one), so
// that the host can see, automate and record it. The two always show the same value:
//  - the host sets the mirror: the source is set, and so is mnArpee's window;
//  - the source changes (the user moves a control in mnArpee's window, a project is loaded): the host is told,
//    and so are the gestures of the control, so the host can record the automation.
//
// The rack must detach() the mirrors before the processor that owns the source goes away.
//==============================================================================
class MirrorParameter : public juce::RangedAudioParameter,
                        private juce::AudioProcessorParameter::Listener
{
public:
    MirrorParameter(juce::RangedAudioParameter& sourceParameter, const juce::String& idPrefix,
                    const juce::String& namePrefix)
        : juce::RangedAudioParameter(juce::ParameterID { idPrefix + sourceParameter.paramID, 1 },
                                     namePrefix + sourceParameter.getName(64),
                                     juce::AudioProcessorParameterWithIDAttributes().withLabel(sourceParameter.getLabel())),
          source(&sourceParameter),
          range(sourceParameter.getNormalisableRange()),
          defaultNormalised(sourceParameter.getDefaultValue()),
          steps(sourceParameter.getNumSteps()),
          discrete(sourceParameter.isDiscrete()),
          boolean(sourceParameter.isBoolean()),
          valueStrings(sourceParameter.getAllValueStrings())
    {
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
        return source != nullptr ? source->getText(normalised, length)
                                 : juce::String(range.convertFrom0to1(normalised)).substring(0, length);
    }
    float getValueForText(const juce::String& text) const override
    {
        return source != nullptr ? source->getValueForText(text) : 0.0f;
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

    juce::RangedAudioParameter* source;
    const juce::NormalisableRange<float> range;
    const float defaultNormalised;
    const int steps;
    const bool discrete, boolean;
    const juce::StringArray valueStrings;
    float lastValue = 0.0f;
    std::atomic<bool> forwarding { false };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MirrorParameter)
};
