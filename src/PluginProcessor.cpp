#include "PluginProcessor.h"
#include "PluginEditor.h"
#include <cmath>

DoctorsDistortionProcessor::DoctorsDistortionProcessor()
    : AudioProcessor(BusesProperties()
          .withInput("Input", juce::AudioChannelSet::stereo(), true)
          .withOutput("Output", juce::AudioChannelSet::stereo(), true)),
      apvts(*this, nullptr, "Parameters", createParameterLayout())
{
    filter1TypeParam   = apvts.getRawParameterValue("filter1Type");
    filter1CutoffParam = apvts.getRawParameterValue("filter1Cutoff");
    filter1ResoParam   = apvts.getRawParameterValue("filter1Reso");
    filter1DriveParam  = apvts.getRawParameterValue("filter1Drive");
    filter2TypeParam   = apvts.getRawParameterValue("filter2Type");
    filter2CutoffParam = apvts.getRawParameterValue("filter2Cutoff");
    filter2ResoParam   = apvts.getRawParameterValue("filter2Reso");
    filter2DriveParam  = apvts.getRawParameterValue("filter2Drive");
    routingModeParam   = apvts.getRawParameterValue("routingMode");
    distTypeParam      = apvts.getRawParameterValue("distType");
    distDriveParam     = apvts.getRawParameterValue("distDrive");
    distMixParam       = apvts.getRawParameterValue("distMix");
    distToneParam      = apvts.getRawParameterValue("distTone");
    distOSParam        = apvts.getRawParameterValue("distOS");
    limiterTypeParam   = apvts.getRawParameterValue("limiterType");
    limiterThreshParam = apvts.getRawParameterValue("limiterThresh");
    limiterReleaseParam= apvts.getRawParameterValue("limiterRelease");
    limiterMakeupParam = apvts.getRawParameterValue("limiterMakeup");
    inputGainParam     = apvts.getRawParameterValue("inputGain");
    delayTypeParam     = apvts.getRawParameterValue("delayType");
    delayTimeParam     = apvts.getRawParameterValue("delayTime");
    delayFbParam       = apvts.getRawParameterValue("delayFb");
    delayMixParam      = apvts.getRawParameterValue("delayMix");
    fbAmountParam      = apvts.getRawParameterValue("fbAmount");
    fbDelayParam       = apvts.getRawParameterValue("fbDelay");
    dryWetParam        = apvts.getRawParameterValue("dryWet");
    outputGainParam    = apvts.getRawParameterValue("outputGain");
}

DoctorsDistortionProcessor::~DoctorsDistortionProcessor() {}

juce::AudioProcessorValueTreeState::ParameterLayout
DoctorsDistortionProcessor::createParameterLayout()
{
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> params;

    // Filter 1
    params.push_back(std::make_unique<juce::AudioParameterInt>(
        juce::ParameterID{"filter1Type", 1}, "Filter 1 Type",
        0, FilterEngine::kNumTypes - 1, 1));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{"filter1Cutoff", 1}, "Filter 1 Cutoff",
        juce::NormalisableRange<float>(20.f, 20000.f, 0.f, 0.3f), 1000.f));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{"filter1Reso", 1}, "Filter 1 Resonance",
        juce::NormalisableRange<float>(0.f, 1.f), 0.f));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{"filter1Drive", 1}, "Filter 1 Drive",
        juce::NormalisableRange<float>(0.f, 1.f), 0.f));

    // Filter 2
    params.push_back(std::make_unique<juce::AudioParameterInt>(
        juce::ParameterID{"filter2Type", 1}, "Filter 2 Type",
        0, FilterEngine::kNumTypes - 1, 7));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{"filter2Cutoff", 1}, "Filter 2 Cutoff",
        juce::NormalisableRange<float>(20.f, 20000.f, 0.f, 0.3f), 5000.f));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{"filter2Reso", 1}, "Filter 2 Resonance",
        juce::NormalisableRange<float>(0.f, 1.f), 0.f));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{"filter2Drive", 1}, "Filter 2 Drive",
        juce::NormalisableRange<float>(0.f, 1.f), 0.f));

    // Routing
    params.push_back(std::make_unique<juce::AudioParameterInt>(
        juce::ParameterID{"routingMode", 1}, "Routing Mode",
        0, 3, 0));

    // Distortion
    params.push_back(std::make_unique<juce::AudioParameterInt>(
        juce::ParameterID{"distType", 1}, "Distortion Type",
        0, Distortion::kNumTypes - 1, 1)); // default Soft Clip

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{"distDrive", 1}, "Distortion Drive",
        juce::NormalisableRange<float>(0.f, 1.f), 0.3f));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{"distMix", 1}, "Distortion Mix",
        juce::NormalisableRange<float>(0.f, 1.f), 1.f));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{"distTone", 1}, "Distortion Tone",
        juce::NormalisableRange<float>(-1.f, 1.f), 0.f));

    params.push_back(std::make_unique<juce::AudioParameterInt>(
        juce::ParameterID{"distOS", 1}, "Oversampling",
        1, 8, 4)); // default 4x

    // Limiter (post-distortion)
    params.push_back(std::make_unique<juce::AudioParameterInt>(
        juce::ParameterID{"limiterType", 1}, "Limiter Type",
        0, Limiter::kNumTypes - 1, 0)); // default Off

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{"limiterThresh", 1}, "Limiter Threshold",
        juce::NormalisableRange<float>(-24.f, 0.f, 0.1f), -6.f));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{"limiterRelease", 1}, "Limiter Release",
        juce::NormalisableRange<float>(5.f, 500.f, 1.f, 0.4f), 50.f));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{"limiterMakeup", 1}, "Limiter Makeup",
        juce::NormalisableRange<float>(0.f, 12.f, 0.1f), 0.f));

    // Delay/Reverb
    params.push_back(std::make_unique<juce::AudioParameterInt>(
        juce::ParameterID{"delayType", 1}, "Delay Type",
        0, DelayReverb::kNumTypes - 1, 0));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{"delayTime", 1}, "Delay Time",
        juce::NormalisableRange<float>(1.f, 2000.f, 1.f, 0.4f), 250.f));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{"delayFb", 1}, "Delay Feedback",
        juce::NormalisableRange<float>(0.f, 0.95f), 0.3f));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{"delayMix", 1}, "Delay Mix",
        juce::NormalisableRange<float>(0.f, 1.f), 0.f));

    // Feedback Path
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{"fbAmount", 1}, "Feedback Amount",
        juce::NormalisableRange<float>(0.f, 0.95f), 0.f));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{"fbDelay", 1}, "Feedback Delay",
        juce::NormalisableRange<float>(0.f, 500.f, 1.f), 100.f));

    // Input/Output
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{"inputGain", 1}, "Input Gain",
        juce::NormalisableRange<float>(-36.f, 12.f, 0.1f), 0.f));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{"dryWet", 1}, "Dry/Wet",
        juce::NormalisableRange<float>(0.f, 1.f), 1.f));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{"outputGain", 1}, "Output Gain",
        juce::NormalisableRange<float>(-36.f, 12.f, 0.1f), 0.f));

    return { params.begin(), params.end() };
}

void DoctorsDistortionProcessor::prepareToPlay(double sampleRate, int samplesPerBlock)
{
    filterRouter.prepare(sampleRate, samplesPerBlock);
    distortion.prepare(sampleRate, samplesPerBlock);
    limiter.prepare(sampleRate, samplesPerBlock);
    delayReverb.prepare(sampleRate, samplesPerBlock);
    feedbackPath.prepare(sampleRate, samplesPerBlock);
    modMatrix.prepare(sampleRate);
}

void DoctorsDistortionProcessor::releaseResources() {}

void DoctorsDistortionProcessor::processBlock(juce::AudioBuffer<float>& buffer,
                                                juce::MidiBuffer& midiMessages)
{
    juce::ignoreUnused(midiMessages);
    juce::ScopedNoDenormals noDenormals;

    auto totalNumInputChannels = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear(i, 0, buffer.getNumSamples());

    if (buffer.getNumChannels() < 2) return;

    int numSamples = buffer.getNumSamples();
    float* left  = buffer.getWritePointer(0);
    float* right = buffer.getWritePointer(1);

    // Update filter parameters
    filterRouter.getFilter1().setType(static_cast<int>(filter1TypeParam->load()));
    filterRouter.getFilter1().setCutoff(filter1CutoffParam->load());
    filterRouter.getFilter1().setResonance(filter1ResoParam->load());
    filterRouter.getFilter1().setDrive(filter1DriveParam->load());

    filterRouter.getFilter2().setType(static_cast<int>(filter2TypeParam->load()));
    filterRouter.getFilter2().setCutoff(filter2CutoffParam->load());
    filterRouter.getFilter2().setResonance(filter2ResoParam->load());
    filterRouter.getFilter2().setDrive(filter2DriveParam->load());

    filterRouter.setMode(static_cast<FilterRouter::Mode>(
        static_cast<int>(routingModeParam->load())));

    // Update distortion parameters
    distortion.setType(static_cast<Distortion::Type>(
        static_cast<int>(distTypeParam->load())));
    distortion.setDrive(distDriveParam->load());
    distortion.setMix(distMixParam->load());
    distortion.setTone(distToneParam->load());
    distortion.setOversampleFactor(static_cast<int>(distOSParam->load()));

    // Store dry signal for global mix
    float dryWet = dryWetParam->load();
    juce::AudioBuffer<float> dryBuffer;
    if (dryWet < 1.f)
        dryBuffer.makeCopyOf(buffer);

    // Update delay/reverb parameters
    delayReverb.setType(static_cast<DelayReverb::Type>(
        static_cast<int>(delayTypeParam->load())));
    delayReverb.setTime(delayTimeParam->load());
    delayReverb.setFeedback(delayFbParam->load());
    delayReverb.setMix(delayMixParam->load());

    // Update feedback path
    feedbackPath.setAmount(fbAmountParam->load());
    feedbackPath.setDelayMs(fbDelayParam->load());

    // Apply input gain
    float inGainDb = inputGainParam->load();
    if (std::abs(inGainDb) > 0.05f)
    {
        float inGain = std::pow(10.f, inGainDb / 20.f);
        for (int i = 0; i < numSamples; ++i)
        {
            left[i] *= inGain;
            right[i] *= inGain;
        }
    }

    // Measure input levels
    inputLevelL.store(buffer.getMagnitude(0, 0, numSamples));
    inputLevelR.store(buffer.getMagnitude(1, 0, numSamples));

    // Signal chain: Feedback → Filters → Distortion → Limiter → Delay/Reverb
    feedbackPath.processStereo(left, right, numSamples);
    filterRouter.processStereo(left, right, numSamples);
    distortion.processStereo(buffer);
    left  = buffer.getWritePointer(0);
    right = buffer.getWritePointer(1);

    // Limiter (post-distortion, pre-delay)
    limiter.setType(static_cast<Limiter::Type>(static_cast<int>(limiterTypeParam->load())));
    limiter.setThreshold(limiterThreshParam->load());
    limiter.setRelease(limiterReleaseParam->load());
    limiter.setMakeup(limiterMakeupParam->load());
    limiter.processStereo(left, right, numSamples);

    delayReverb.processStereo(left, right, numSamples);

    // Apply global dry/wet mix
    if (dryWet < 1.f)
    {
        left  = buffer.getWritePointer(0);
        right = buffer.getWritePointer(1);
        for (int i = 0; i < numSamples; ++i)
        {
            left[i]  = left[i] * dryWet + dryBuffer.getSample(0, i) * (1.f - dryWet);
            right[i] = right[i] * dryWet + dryBuffer.getSample(1, i) * (1.f - dryWet);
        }
    }

    // Apply output gain
    float gainDb = outputGainParam->load();
    float gain = std::pow(10.f, gainDb / 20.f);
    left  = buffer.getWritePointer(0);
    right = buffer.getWritePointer(1);
    for (int i = 0; i < numSamples; ++i)
    {
        left[i]  *= gain;
        right[i] *= gain;
    }

    // Measure output levels
    outputLevelL.store(buffer.getMagnitude(0, 0, numSamples));
    outputLevelR.store(buffer.getMagnitude(1, 0, numSamples));
}

void DoctorsDistortionProcessor::getStateInformation(juce::MemoryBlock& destData)
{
    auto state = apvts.copyState();
    std::unique_ptr<juce::XmlElement> xml(state.createXml());
    copyXmlToBinary(*xml, destData);
}

void DoctorsDistortionProcessor::setStateInformation(const void* data, int sizeInBytes)
{
    std::unique_ptr<juce::XmlElement> xml(getXmlFromBinary(data, sizeInBytes));
    if (xml != nullptr && xml->hasTagName(apvts.state.getType()))
        apvts.replaceState(juce::ValueTree::fromXml(*xml));
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new DoctorsDistortionProcessor();
}
