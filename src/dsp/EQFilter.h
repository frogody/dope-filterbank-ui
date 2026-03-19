#pragma once
#include <cmath>
#include <array>
#include <algorithm>

// EQ filters from Bristow-Johnson Audio EQ Cookbook
class EQFilter
{
public:
    enum class Type { LowShelf, HighShelf, Peak, Tilt };

    void prepare(double sampleRate, int blockSize);
    void setType(Type t);
    void setCutoff(float freqHz);
    void setResonance(float reso); // gain amount for shelf/peak, Q for tilt
    void reset();

    float processSample(int channel, float input);

private:
    void updateCoefficients();

    Type type = Type::LowShelf;
    double sr = 44100.0;
    float cutoff = 1000.f;
    float resonance = 0.5f;

    double b0 = 1.0, b1 = 0.0, b2 = 0.0;
    double a0 = 1.0, a1 = 0.0, a2 = 0.0;

    struct ChannelState { double x1, x2, y1, y2; };
    std::array<ChannelState, 2> state{};
};
