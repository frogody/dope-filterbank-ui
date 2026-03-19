#include "FeedbackPath.h"

void FeedbackPath::prepare(double sampleRate, int /*blockSize*/)
{
    sr = sampleRate;
    for (auto& dl : delayLines)
    {
        dl.buffer.resize(kMaxDelay, 0.f);
        dl.writePos = 0;
    }
    reset();
}

void FeedbackPath::setAmount(float a)
{
    amount = std::clamp(a, 0.f, 0.95f); // safety cap
}

void FeedbackPath::setDelayMs(float ms)
{
    ms = std::clamp(ms, 0.f, 500.f);
    delaySamples = ms * static_cast<float>(sr) / 1000.f;
}

void FeedbackPath::reset()
{
    for (auto& dl : delayLines)
    {
        std::fill(dl.buffer.begin(), dl.buffer.end(), 0.f);
        dl.writePos = 0;
    }
}

float FeedbackPath::getFeedbackSample(int channel) const
{
    auto& dl = delayLines[channel];
    int readIdx = dl.writePos - static_cast<int>(delaySamples);
    if (readIdx < 0) readIdx += kMaxDelay;

    float sample = dl.buffer[readIdx] * amount;

    // Safety limiter: soft clip feedback to prevent runaway
    return std::tanh(sample);
}

void FeedbackPath::pushOutputSample(int channel, float sample)
{
    auto& dl = delayLines[channel];
    dl.buffer[dl.writePos] = sample;
    dl.writePos = (dl.writePos + 1) % kMaxDelay;
}

void FeedbackPath::processStereo(float* left, float* right, int numSamples)
{
    if (amount < 0.001f) return; // skip if no feedback

    for (int i = 0; i < numSamples; ++i)
    {
        // Add feedback to input
        left[i]  += getFeedbackSample(0);
        right[i] += getFeedbackSample(1);

        // Store current output for next cycle
        pushOutputSample(0, left[i]);
        pushOutputSample(1, right[i]);
    }
}
