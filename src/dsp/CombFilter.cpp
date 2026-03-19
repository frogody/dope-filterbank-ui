#include "CombFilter.h"

void CombFilter::prepare(double sampleRate, int /*blockSize*/)
{
    sr = sampleRate;
    for (auto& ch : state)
    {
        ch.buffer.resize(kMaxDelay, 0.f);
        ch.writePos = 0;
    }
    reset();
}

void CombFilter::setCutoff(float freqHz)
{
    freqHz = std::clamp(freqHz, 10.f, static_cast<float>(sr * 0.49));
    delaySamples = static_cast<float>(sr / freqHz);
}

void CombFilter::setResonance(float reso)
{
    feedback = std::clamp(reso, 0.f, 0.98f); // cap at 0.98 to prevent runaway
}

void CombFilter::reset()
{
    for (auto& ch : state)
    {
        std::fill(ch.buffer.begin(), ch.buffer.end(), 0.f);
        ch.writePos = 0;
    }
}

float CombFilter::processSample(int channel, float input)
{
    auto& ch = state[channel];

    // Linear interpolation for fractional delay
    int delayInt = static_cast<int>(delaySamples);
    float frac = delaySamples - delayInt;

    int readPos1 = ch.writePos - delayInt;
    if (readPos1 < 0) readPos1 += kMaxDelay;
    int readPos2 = readPos1 - 1;
    if (readPos2 < 0) readPos2 += kMaxDelay;

    float delayed = ch.buffer[readPos1] * (1.f - frac) + ch.buffer[readPos2] * frac;

    float output;
    switch (type)
    {
        case Type::Positive:
            output = input + delayed * feedback;
            break;
        case Type::Negative:
            output = input - delayed * feedback;
            break;
        case Type::Bandpass:
            output = delayed;
            break;
    }

    // Write to buffer (feedback comb)
    ch.buffer[ch.writePos] = input + delayed * feedback;
    ch.writePos = (ch.writePos + 1) % kMaxDelay;

    return output;
}
