#include "Limiter.h"

const char* Limiter::getTypeName(int index)
{
    static const char* names[] = { "Off", "Soft Clip", "Hard Limit", "Compressor" };
    if (index < 0 || index >= kNumTypes) return "Unknown";
    return names[index];
}

void Limiter::prepare(double sampleRate, int /*blockSize*/)
{
    sr = sampleRate;
    envState[0] = envState[1] = 0.f;
    // Default release: 50ms
    releaseCoeff = std::exp(-1.0 / (0.05 * sr));
    attackCoeff = std::exp(-1.0 / (0.001 * sr)); // 1ms attack
}

void Limiter::setThreshold(float threshDb_)
{
    thresholdDb = std::clamp(threshDb_, -24.f, 0.f);
    threshold = std::pow(10.f, thresholdDb / 20.f);
}

void Limiter::setRelease(float releaseMs)
{
    float ms = std::clamp(releaseMs, 5.f, 500.f);
    releaseCoeff = static_cast<float>(std::exp(-1.0 / (ms * 0.001 * sr)));
}

void Limiter::setMakeup(float gainDb)
{
    float db = std::clamp(gainDb, 0.f, 12.f);
    makeupGain = std::pow(10.f, db / 20.f);
}

float Limiter::processSampleSoftClip(float input)
{
    // Soft clip with variable knee based on threshold
    // At threshold, start saturating. Uses tanh curve scaled to threshold.
    float x = input / threshold;
    return threshold * std::tanh(x) * makeupGain;
}

float Limiter::processSampleHardLimit(float input)
{
    // Brick wall limiter at threshold
    float limited = std::clamp(input, -threshold, threshold);
    return limited * makeupGain;
}

float Limiter::processSampleCompressor(float input, int channel)
{
    // Feed-forward compressor with ~inf:1 ratio above threshold (acts as limiter)
    float absIn = std::abs(input);

    // Envelope follower (peak detection)
    if (absIn > envState[channel])
        envState[channel] = attackCoeff * envState[channel] + (1.f - attackCoeff) * absIn;
    else
        envState[channel] = releaseCoeff * envState[channel] + (1.f - releaseCoeff) * absIn;

    // Compute gain reduction
    float env = envState[channel];
    float gainReduction = 1.f;
    if (env > threshold && env > 0.0001f)
    {
        // Ratio ~20:1 (effectively limiting)
        float dbOver = 20.f * std::log10(env / threshold);
        float dbReduction = dbOver * (1.f - 1.f / 20.f); // 20:1 ratio
        gainReduction = std::pow(10.f, -dbReduction / 20.f);
    }

    return input * gainReduction * makeupGain;
}

void Limiter::processStereo(float* left, float* right, int numSamples)
{
    if (type == Type::Off) return;

    for (int i = 0; i < numSamples; ++i)
    {
        switch (type)
        {
            case Type::SoftClip:
                left[i]  = processSampleSoftClip(left[i]);
                right[i] = processSampleSoftClip(right[i]);
                break;

            case Type::HardLimit:
                left[i]  = processSampleHardLimit(left[i]);
                right[i] = processSampleHardLimit(right[i]);
                break;

            case Type::Compressor:
                left[i]  = processSampleCompressor(left[i], 0);
                right[i] = processSampleCompressor(right[i], 1);
                break;

            case Type::Off:
                break;
        }
    }
}
