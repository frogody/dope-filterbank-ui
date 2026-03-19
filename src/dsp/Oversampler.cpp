#include "Oversampler.h"
#include <cmath>

void Oversampler::prepare(double sampleRate, int blockSize)
{
    baseSampleRate = sampleRate;
    baseBlockSize = blockSize;
    needsInit = true; // lazy init on first process call
}

void Oversampler::setFactor(int factor)
{
    factor = std::max(1, std::min(8, factor));
    if (factor != currentFactor)
    {
        currentFactor = factor;
        createOversampler(baseBlockSize);
    }
}

int Oversampler::getLatencySamples() const
{
    if (oversampling && currentFactor > 1)
        return static_cast<int>(oversampling->getLatencyInSamples());
    return 0;
}

void Oversampler::createOversampler(int blockSize)
{
    if (currentFactor <= 1)
    {
        oversampling.reset();
        return;
    }

    // Calculate order: 2^order = factor
    int order = 0;
    int f = currentFactor;
    while (f > 1) { f >>= 1; order++; }

    oversampling = std::make_unique<juce::dsp::Oversampling<float>>(
        2, // stereo
        order,
        juce::dsp::Oversampling<float>::filterHalfBandPolyphaseIIR,
        true // use maximum quality
    );

    oversampling->initProcessing(static_cast<size_t>(blockSize));
}

void Oversampler::process(juce::AudioBuffer<float>& buffer,
                           std::function<void(float*, float*, int)> processCallback)
{
    // Lazy init — don't block plugin open
    if (needsInit)
    {
        createOversampler(baseBlockSize);
        needsInit = false;
    }

    if (!oversampling || currentFactor <= 1)
    {
        // No oversampling — process directly
        processCallback(buffer.getWritePointer(0),
                       buffer.getWritePointer(1),
                       buffer.getNumSamples());
        return;
    }

    // Wrap buffer in AudioBlock
    juce::dsp::AudioBlock<float> block(buffer);

    // Upsample
    auto oversampledBlock = oversampling->processSamplesUp(block);

    // Process at oversampled rate
    float* left  = oversampledBlock.getChannelPointer(0);
    float* right = oversampledBlock.getChannelPointer(1);
    int numSamples = static_cast<int>(oversampledBlock.getNumSamples());

    processCallback(left, right, numSamples);

    // Downsample
    oversampling->processSamplesDown(block);
}
