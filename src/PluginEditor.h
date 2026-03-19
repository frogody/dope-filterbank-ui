#pragma once
#include "PluginProcessor.h"
#include <juce_gui_extra/juce_gui_extra.h>

class DoctorsDistortionEditor : public juce::AudioProcessorEditor
{
public:
    explicit DoctorsDistortionEditor(DoctorsDistortionProcessor&);
    ~DoctorsDistortionEditor() override;

    void paint(juce::Graphics&) override;
    void resized() override;

private:
    DoctorsDistortionProcessor& processorRef;

    // Web Relays for Knobs
    juce::WebSliderRelay rF1Cutoff{"f1Cutoff"}, rF2Cutoff{"f2Cutoff"};
    juce::WebSliderRelay rF1Reso{"f1Reso"}, rF2Reso{"f2Reso"};
    juce::WebSliderRelay rDistDrive{"distDrive"}, rDistMix{"distMix"}, rDistTone{"distTone"};
    juce::WebSliderRelay rLimThresh{"limThresh"}, rLimRelease{"limRelease"};
    juce::WebSliderRelay rDryWet{"dryWet"};
    juce::WebSliderRelay rInGain{"inGain"}, rOutGain{"outGain"};

    // Web Relays for Combos
    juce::WebComboBoxRelay rF1Type{"f1Type"}, rF2Type{"f2Type"};
    juce::WebComboBoxRelay rRouting{"routingMode"};
    juce::WebComboBoxRelay rDistType{"distType"};
    juce::WebComboBoxRelay rLimType{"limiterType"};

    // APVTS Attachments linking Web Relays to DSP params
    juce::WebSliderParameterAttachment aF1Cutoff, aF2Cutoff;
    juce::WebSliderParameterAttachment aF1Reso, aF2Reso;
    juce::WebSliderParameterAttachment aDistDrive, aDistMix, aDistTone;
    juce::WebSliderParameterAttachment aLimThresh, aLimRelease;
    juce::WebSliderParameterAttachment aDryWet;
    juce::WebSliderParameterAttachment aInGain, aOutGain;

    juce::WebComboBoxParameterAttachment aF1Type, aF2Type;
    juce::WebComboBoxParameterAttachment aRouting;
    juce::WebComboBoxParameterAttachment aDistType;
    juce::WebComboBoxParameterAttachment aLimType;

    juce::WebBrowserComponent webComponent;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(DoctorsDistortionEditor)
};
