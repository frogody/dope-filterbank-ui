#pragma once
#include <cmath>
#include <array>
#include <algorithm>

// Phaser: cascaded allpass filters
class PhaserFilter
{
public:
    enum class Type { Stage4, Stage8, Stage12 };

    void prepare(double sampleRate, int blockSize);
    void setType(Type t);
    void setCutoff(float freqHz);
    void setResonance(float reso); // feedback amount
    void reset();

    float processSample(int channel, float input);

private:
    void updateCoefficients();

    Type type = Type::Stage4;
    double sr = 44100.0;
    float cutoff = 1000.f;
    float feedback = 0.f;
    int numStages = 4;

    double apCoeff = 0.0; // allpass coefficient

    static constexpr int kMaxStages = 12;
    struct ChannelState
    {
        double apState[kMaxStages] = {};
        double lastOutput = 0.0;
    };
    std::array<ChannelState, 2> state{};
};
