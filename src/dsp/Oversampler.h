#pragma once
#include <juce_dsp/juce_dsp.h>
#include <memory>
#include <functional>

// Wraps juce::dsp::Oversampling for 2x/4x/8x modes
class Oversampler
{
public:
    void prepare(double sampleRate, int blockSize);
    void setFactor(int factor); // 1, 2, 4, or 8
    int getFactor() const { return currentFactor; }
    int getLatencySamples() const;

    // Process a stereo block through the oversampled callback
    void process(juce::AudioBuffer<float>& buffer,
                 std::function<void(float*, float*, int)> processCallback);

private:
    void createOversampler(int blockSize);

    int currentFactor = 4;
    double baseSampleRate = 44100.0;
    int baseBlockSize = 512;
    bool needsInit = true;

    std::unique_ptr<juce::dsp::Oversampling<float>> oversampling;
};
