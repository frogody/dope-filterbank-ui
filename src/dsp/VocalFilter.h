#pragma once
#include "FormantFilter.h"

// Vocal filter: extended formant filter with male/female/whisper presets
// Uses FormantFilter internally with different frequency ranges
class VocalFilter
{
public:
    enum class Type { Male, Female, Whisper };

    void prepare(double sampleRate, int blockSize);
    void setType(Type t);
    void setCutoff(float freqHz);
    void setResonance(float reso);
    void reset();

    float processSample(int channel, float input);

private:
    Type type = Type::Male;
    FormantFilter formant;
    float cutoffMultiplier = 1.f;
};
