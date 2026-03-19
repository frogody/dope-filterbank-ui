#include "FMFilter.h"

void FMFilter::prepare(double sampleRate, int blockSize)
{
    sr = sampleRate;
    svf.prepare(sampleRate, blockSize);
    phase = 0.0;
}

void FMFilter::setType(Type t)
{
    type = t;
    switch (t)
    {
        case Type::LP: svf.setType(SVFilter::Type::LP12); break;
        case Type::HP: svf.setType(SVFilter::Type::HP12); break;
        case Type::BP: svf.setType(SVFilter::Type::BP12); break;
    }
}

void FMFilter::setCutoff(float freqHz)
{
    baseCutoff = std::clamp(freqHz, 20.f, 20000.f);
}

void FMFilter::setResonance(float reso)
{
    svf.setResonance(reso);
}

void FMFilter::setFMDepth(float depth)
{
    fmDepth = std::clamp(depth, 0.f, 1.f);
}

void FMFilter::reset()
{
    svf.reset();
    phase = 0.0;
}

float FMFilter::processSample(int channel, float input)
{
    // Use input signal to modulate cutoff frequency
    float modAmount = input * fmDepth * baseCutoff;
    float modCutoff = std::clamp(baseCutoff + modAmount, 20.f, static_cast<float>(sr * 0.49));
    svf.setCutoff(modCutoff);

    return svf.processSample(channel, input);
}
