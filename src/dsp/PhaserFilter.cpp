#include "PhaserFilter.h"

void PhaserFilter::prepare(double sampleRate, int /*blockSize*/)
{
    sr = sampleRate;
    reset();
    updateCoefficients();
}

void PhaserFilter::setType(Type t)
{
    type = t;
    switch (type)
    {
        case Type::Stage4:  numStages = 4;  break;
        case Type::Stage8:  numStages = 8;  break;
        case Type::Stage12: numStages = 12; break;
    }
    updateCoefficients();
}

void PhaserFilter::setCutoff(float freqHz)
{
    cutoff = std::clamp(freqHz, 20.f, static_cast<float>(sr * 0.49));
    updateCoefficients();
}

void PhaserFilter::setResonance(float reso)
{
    feedback = std::clamp(reso, 0.f, 0.95f);
}

void PhaserFilter::reset()
{
    for (auto& ch : state)
    {
        for (auto& s : ch.apState) s = 0.0;
        ch.lastOutput = 0.0;
    }
}

void PhaserFilter::updateCoefficients()
{
    // First-order allpass coefficient from cutoff frequency
    double w0 = 2.0 * M_PI * cutoff / sr;
    apCoeff = (1.0 - std::tan(w0 * 0.5)) / (1.0 + std::tan(w0 * 0.5));
}

float PhaserFilter::processSample(int channel, float input)
{
    auto& ch = state[channel];

    // Apply feedback
    double x = input - feedback * ch.lastOutput;

    // Cascade of first-order allpass filters
    for (int i = 0; i < numStages; ++i)
    {
        double ap = apCoeff * x + ch.apState[i];
        ch.apState[i] = x - apCoeff * ap;
        x = ap;
    }

    ch.lastOutput = x;

    // Mix: original + phase-shifted (notches at cancellation points)
    return static_cast<float>((input + x) * 0.5);
}
