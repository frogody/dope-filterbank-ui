#pragma once
#include <vector>
#include <array>
#include <cmath>
#include <algorithm>

class CombFilter
{
public:
    enum class Type { Positive, Negative, Bandpass };

    void prepare(double sampleRate, int blockSize);
    void setType(Type t) { type = t; }
    void setCutoff(float freqHz); // sets delay time from frequency
    void setResonance(float reso); // feedback amount 0..1
    void reset();

    float processSample(int channel, float input);

private:
    Type type = Type::Positive;
    double sr = 44100.0;
    float feedback = 0.f;
    float delaySamples = 44.1f; // ~1kHz at 44.1kHz

    static constexpr int kMaxDelay = 4410; // down to ~10Hz
    struct ChannelState
    {
        std::vector<float> buffer;
        int writePos = 0;
    };
    std::array<ChannelState, 2> state;
};
