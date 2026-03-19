#pragma once
#include <cmath>
#include <array>
#include <algorithm>

// Moog-style ladder filter using improved Huovilainen model
// with per-stage nonlinear saturation for analog "fatness"
// Reference: Välimäki & Smith, "Revisiting the Moog Ladder Filter" (2006)
class LadderFilter
{
public:
    enum class Type { LP, HP, BP };

    void prepare(double sampleRate, int blockSize);
    void setType(Type t) { type = t; }
    void setCutoff(float freqHz);
    void setResonance(float reso); // 0..1, >0.9 = self-oscillation
    void setDrive(float d);        // 0..1, controls per-stage saturation amount
    void reset();

    float processSample(int channel, float input);

private:
    void updateCoefficients();

    // Fast tanh approximation (Pade 3/2) — cheaper than std::tanh for per-stage use
    static inline float fastTanh(float x)
    {
        float x2 = x * x;
        return x * (27.f + x2) / (27.f + 9.f * x2);
    }

    Type type = Type::LP;
    double sr = 44100.0;
    float cutoff = 1000.f;
    float resonance = 0.f;
    float drive = 0.f;           // per-stage saturation amount

    double g = 0.0;              // tuning coefficient
    double resonanceScaled = 0.0;
    float saturationAmount = 0.f; // derived from drive: 0 = clean, 1 = heavy saturation

    // Thermal drift: slight cutoff modulation based on signal energy
    float thermalState[2] = {0.f, 0.f};
    float thermalCoeff = 0.9999f;

    // 4 one-pole stages per channel
    struct ChannelState
    {
        double s[4] = {0.0, 0.0, 0.0, 0.0};
    };
    std::array<ChannelState, 2> state{};
};
