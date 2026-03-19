#pragma once
#include <cmath>
#include <array>
#include <algorithm>

// Parallel biquad formant filter for vowel sounds
class FormantFilter
{
public:
    // A, E, I, O, U + morphable
    enum class Type { VowelA, VowelE, VowelI, VowelO, VowelU, Morph };

    void prepare(double sampleRate, int blockSize);
    void setType(Type t);
    void setCutoff(float freqHz); // shifts all formants
    void setResonance(float reso); // formant bandwidth
    void setMorph(float m); // 0..1 for Morph type (A→E→I→O→U)
    void reset();

    float processSample(int channel, float input);

private:
    void updateCoefficients();
    void calcFormantBiquad(double freq, double bw, double gain,
                           double& b0, double& b1, double& b2,
                           double& a1, double& a2);

    Type type = Type::VowelA;
    double sr = 44100.0;
    float cutoffShift = 1.f; // multiplier from setCutoff
    float resonance = 0.5f;
    float morphValue = 0.f;

    static constexpr int kNumFormants = 3;

    struct BiquadCoeffs { double b0, b1, b2, a1, a2; };
    struct BiquadState { double x1, x2, y1, y2; };

    std::array<BiquadCoeffs, kNumFormants> coeffs{};
    std::array<std::array<BiquadState, kNumFormants>, 2> state{};

    // Formant frequencies and bandwidths for each vowel (Hz)
    // [vowel][formant] = {freq, bandwidth, gain_dB}
    static constexpr double formantData[5][3][3] = {
        // A:  F1=800,  F2=1200, F3=2500
        {{800, 80, 0}, {1200, 90, -6}, {2500, 120, -12}},
        // E:  F1=400,  F2=2200, F3=2800
        {{400, 60, 0}, {2200, 100, -6}, {2800, 120, -12}},
        // I:  F1=300,  F2=2700, F3=3500
        {{300, 50, 0}, {2700, 110, -6}, {3500, 130, -12}},
        // O:  F1=500,  F2=800,  F3=2500
        {{500, 70, 0}, {800, 80, -6}, {2500, 120, -12}},
        // U:  F1=350,  F2=600,  F3=2500
        {{350, 60, 0}, {600, 70, -6}, {2500, 120, -12}},
    };
};
