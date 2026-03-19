#pragma once
#include <cmath>
#include <array>
#include <algorithm>

// Tight bandpass biquad resonator
class Resonator
{
public:
    enum class Type { Mono, Stereo, Tuned };

    void prepare(double sampleRate, int blockSize);
    void setType(Type t) { type = t; }
    void setCutoff(float freqHz);
    void setResonance(float reso); // 0..1 → Q 1..200
    void reset();

    float processSample(int channel, float input);

private:
    void updateCoefficients();

    Type type = Type::Mono;
    double sr = 44100.0;
    float cutoff = 1000.f;
    float resonance = 0.5f;

    // Biquad coefficients
    double b0 = 0.0, b1 = 0.0, b2 = 0.0;
    double a1 = 0.0, a2 = 0.0;

    struct ChannelState
    {
        double x1 = 0.0, x2 = 0.0;
        double y1 = 0.0, y2 = 0.0;
    };
    std::array<ChannelState, 2> state{};

    // Stereo mode: slight detuning
    double stereoOffset = 0.0;
    double b0R = 0.0, b1R = 0.0, b2R = 0.0;
    double a1R = 0.0, a2R = 0.0;
};
