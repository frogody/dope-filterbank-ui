#pragma once
#include <juce_audio_processors/juce_audio_processors.h>
#include "dsp/FilterRouter.h"
#include "dsp/Distortion.h"
#include "dsp/DelayReverb.h"
#include "dsp/FeedbackPath.h"
#include "dsp/Limiter.h"
#include "modulation/ModMatrix.h"

class DoctorsDistortionProcessor : public juce::AudioProcessor
{
public:
    DoctorsDistortionProcessor();
    ~DoctorsDistortionProcessor() override;

    void prepareToPlay(double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;
    void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }

    const juce::String getName() const override { return JucePlugin_Name; }
    bool acceptsMidi() const override { return false; }
    bool producesMidi() const override { return false; }
    double getTailLengthSeconds() const override { return 0.0; }

    int getNumPrograms() override { return 1; }
    int getCurrentProgram() override { return 0; }
    void setCurrentProgram(int) override {}
    const juce::String getProgramName(int) override { return {}; }
    void changeProgramName(int, const juce::String&) override {}

    void getStateInformation(juce::MemoryBlock& destData) override;
    void setStateInformation(const void* data, int sizeInBytes) override;

    juce::AudioProcessorValueTreeState apvts;

private:
    juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();

    FilterRouter filterRouter;
    Distortion distortion;
    Limiter limiter;
    DelayReverb delayReverb;
    FeedbackPath feedbackPath;
    ModMatrix modMatrix;

    // Cached parameter pointers for real-time access
    std::atomic<float>* filter1TypeParam    = nullptr;
    std::atomic<float>* filter1CutoffParam  = nullptr;
    std::atomic<float>* filter1ResoParam    = nullptr;
    std::atomic<float>* filter1DriveParam   = nullptr;
    std::atomic<float>* filter2TypeParam    = nullptr;
    std::atomic<float>* filter2CutoffParam  = nullptr;
    std::atomic<float>* filter2ResoParam    = nullptr;
    std::atomic<float>* filter2DriveParam   = nullptr;
    std::atomic<float>* routingModeParam    = nullptr;
    std::atomic<float>* distTypeParam       = nullptr;
    std::atomic<float>* distDriveParam      = nullptr;
    std::atomic<float>* distMixParam        = nullptr;
    std::atomic<float>* distToneParam       = nullptr;
    std::atomic<float>* distOSParam         = nullptr;
    std::atomic<float>* delayTypeParam      = nullptr;
    std::atomic<float>* delayTimeParam     = nullptr;
    std::atomic<float>* delayFbParam       = nullptr;
    std::atomic<float>* delayMixParam      = nullptr;
    std::atomic<float>* limiterTypeParam    = nullptr;
    std::atomic<float>* limiterThreshParam  = nullptr;
    std::atomic<float>* limiterReleaseParam = nullptr;
    std::atomic<float>* limiterMakeupParam  = nullptr;
    std::atomic<float>* fbAmountParam      = nullptr;
    std::atomic<float>* fbDelayParam       = nullptr;
    std::atomic<float>* inputGainParam      = nullptr;
    std::atomic<float>* dryWetParam         = nullptr;
    std::atomic<float>* outputGainParam     = nullptr;

public:
    // Audio level metering (read by GUI)
    std::atomic<float> inputLevelL{0.f}, inputLevelR{0.f};
    std::atomic<float> outputLevelL{0.f}, outputLevelR{0.f};

private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(DoctorsDistortionProcessor)
};
