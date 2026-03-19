#pragma once
#include <cmath>
#include <array>
#include <algorithm>

class SVFilter
{
public:
    enum class Type
    {
        LP6, LP12, LP18, LP24, LP36, LP48,
        HP6, HP12, HP18, HP24, HP36, HP48,
        BP6, BP12, BP24, BP36,
        Notch6, Notch12, Notch24,
        AP6, AP12, AP24
    };

    void prepare(double sampleRate, int blockSize);
    void setType(Type newType);
    void setCutoff(float freqHz);
    void setResonance(float reso);
    void setDrive(float driveAmount);
    void reset();

    float processSample(int channel, float input);

private:
    void updateCoefficients();

    Type type = Type::LP12;
    double sr = 44100.0;
    float cutoff = 1000.f;
    float resonance = 0.f;
    float drive = 0.f;

    static constexpr int kMaxStages = 4;
    struct FilterState
    {
        double ic1eq = 0.0;
        double ic2eq = 0.0;
    };
    std::array<std::array<FilterState, kMaxStages>, 2> state{};

    double g = 0.0, k = 0.0;
    int numStages = 1;
    bool onePole = false;
    enum class BaseType { LP, HP, BP, Notch, AP } baseType = BaseType::LP;
};
