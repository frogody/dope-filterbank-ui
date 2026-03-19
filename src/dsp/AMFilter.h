#pragma once
#include <cmath>
#include <array>
#include <algorithm>

class AMFilter
{
public:
    enum class Type { Ring, Tremolo };

    void prepare(double sampleRate, int blockSize);
    void setType(Type t) { type = t; }
    void setCutoff(float freqHz); // modulation frequency
    void setResonance(float reso); // modulation depth
    void reset();

    float processSample(int channel, float input);

private:
    Type type = Type::Ring;
    double sr = 44100.0;
    float modFreq = 1000.f;
    float depth = 1.f;
    std::array<double, 2> phase = {0.0, 0.0};
};
