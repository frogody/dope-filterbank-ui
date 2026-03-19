#pragma once
#include "SVFilter.h"

// SVF with frequency-modulated cutoff
class FMFilter
{
public:
    enum class Type { LP, HP, BP };

    void prepare(double sampleRate, int blockSize);
    void setType(Type t);
    void setCutoff(float freqHz);
    void setResonance(float reso);
    void setFMDepth(float depth); // 0..1, modulation depth
    void reset();

    float processSample(int channel, float input);

private:
    Type type = Type::LP;
    double sr = 44100.0;
    float baseCutoff = 1000.f;
    float fmDepth = 0.5f;
    SVFilter svf;
    double phase = 0.0;
};
