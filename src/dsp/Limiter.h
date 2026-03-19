#pragma once
#include <cmath>
#include <algorithm>

class Limiter
{
public:
    enum class Type { Off, SoftClip, HardLimit, Compressor };
    static constexpr int kNumTypes = 4;

    void prepare(double sampleRate, int blockSize);
    void setType(Type t) { type = t; }
    void setThreshold(float threshDb); // -24 to 0 dB
    void setRelease(float releaseMs);  // 5 to 500 ms
    void setMakeup(float gainDb);      // 0 to 12 dB

    void processStereo(float* left, float* right, int numSamples);

    static const char* getTypeName(int index);

private:
    float processSampleSoftClip(float input);
    float processSampleHardLimit(float input);
    float processSampleCompressor(float input, int channel);

    Type type = Type::Off;
    float threshold = 1.f;      // linear threshold
    float thresholdDb = 0.f;
    float makeupGain = 1.f;
    double sr = 44100.0;

    // Compressor envelope state per channel
    float envState[2] = {0.f, 0.f};
    float releaseCoeff = 0.999f;
    float attackCoeff = 0.01f;
};
