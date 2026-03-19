#pragma once
#include "SVFilter.h"
#include <cmath>
#include <array>
#include <algorithm>

// Special/creative filter types
class SpecialFilter
{
public:
    enum class Type { OctaveUp, OctaveDown, Bitfilter, MShape, Elliptic };

    void prepare(double sampleRate, int blockSize);
    void setType(Type t);
    void setCutoff(float freqHz);
    void setResonance(float reso);
    void reset();

    float processSample(int channel, float input);

private:
    Type type = Type::OctaveUp;
    double sr = 44100.0;
    float cutoff = 1000.f;
    float resonance = 0.5f;

    SVFilter preFilter;
    SVFilter postFilter;

    // For bitfilter
    float bitDepth = 16.f;

    // For octave tracking
    std::array<float, 2> lastSample = {0.f, 0.f};
};
