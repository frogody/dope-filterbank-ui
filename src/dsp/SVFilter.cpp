#include "SVFilter.h"

void SVFilter::prepare(double sampleRate, int /*blockSize*/)
{
    sr = sampleRate;
    reset();
    updateCoefficients();
}

void SVFilter::setType(Type newType)
{
    type = newType;
    onePole = false;

    switch (type)
    {
        case Type::LP6:    baseType = BaseType::LP; numStages = 1; onePole = true; break;
        case Type::LP12:   baseType = BaseType::LP; numStages = 1; break;
        case Type::LP18:   baseType = BaseType::LP; numStages = 2; onePole = true; break;
        case Type::LP24:   baseType = BaseType::LP; numStages = 2; break;
        case Type::LP36:   baseType = BaseType::LP; numStages = 3; break;
        case Type::LP48:   baseType = BaseType::LP; numStages = 4; break;
        case Type::HP6:    baseType = BaseType::HP; numStages = 1; onePole = true; break;
        case Type::HP12:   baseType = BaseType::HP; numStages = 1; break;
        case Type::HP18:   baseType = BaseType::HP; numStages = 2; onePole = true; break;
        case Type::HP24:   baseType = BaseType::HP; numStages = 2; break;
        case Type::HP36:   baseType = BaseType::HP; numStages = 3; break;
        case Type::HP48:   baseType = BaseType::HP; numStages = 4; break;
        case Type::BP6:    baseType = BaseType::BP; numStages = 1; onePole = true; break;
        case Type::BP12:   baseType = BaseType::BP; numStages = 1; break;
        case Type::BP24:   baseType = BaseType::BP; numStages = 2; break;
        case Type::BP36:   baseType = BaseType::BP; numStages = 3; break;
        case Type::Notch6:  baseType = BaseType::Notch; numStages = 1; onePole = true; break;
        case Type::Notch12: baseType = BaseType::Notch; numStages = 1; break;
        case Type::Notch24: baseType = BaseType::Notch; numStages = 2; break;
        case Type::AP6:    baseType = BaseType::AP; numStages = 1; onePole = true; break;
        case Type::AP12:   baseType = BaseType::AP; numStages = 1; break;
        case Type::AP24:   baseType = BaseType::AP; numStages = 2; break;
    }
    updateCoefficients();
}

void SVFilter::setCutoff(float freqHz)
{
    cutoff = std::clamp(freqHz, 20.f, static_cast<float>(sr * 0.49));
    updateCoefficients();
}

void SVFilter::setResonance(float reso)
{
    resonance = std::clamp(reso, 0.f, 1.f);
    updateCoefficients();
}

void SVFilter::setDrive(float driveAmount)
{
    drive = std::clamp(driveAmount, 0.f, 1.f);
}

void SVFilter::reset()
{
    for (auto& ch : state)
        for (auto& s : ch)
            s = {0.0, 0.0};
}

void SVFilter::updateCoefficients()
{
    g = std::tan(M_PI * cutoff / sr);
    double Q = 0.5 + resonance * 24.5;
    k = 1.0 / Q;
}

float SVFilter::processSample(int channel, float input)
{
    if (drive > 0.f)
    {
        float d = 1.f + drive * 9.f;
        input = std::tanh(input * d) / std::tanh(d);
    }

    double v0 = static_cast<double>(input);

    for (int stage = 0; stage < numStages; ++stage)
    {
        auto& s = state[channel][stage];

        if (onePole && stage == numStages - 1)
        {
            // One-pole (6dB/oct) for odd-order filters
            double v1 = (v0 - s.ic1eq) * g / (1.0 + g);
            double lp = v1 + s.ic1eq;
            s.ic1eq = lp + v1;

            switch (baseType)
            {
                case BaseType::LP:    v0 = lp; break;
                case BaseType::HP:    v0 = v0 - lp; break;
                case BaseType::BP:    v0 = lp; break;
                case BaseType::Notch: v0 = v0 - lp; break;
                case BaseType::AP:    v0 = 2.0 * lp - v0; break;
            }
        }
        else
        {
            // Two-pole (12dB/oct) Cytomic SVF
            double a1 = 1.0 / (1.0 + g * (g + k));
            double a2 = g * a1;
            double a3 = g * a2;

            double v3 = v0 - s.ic2eq;
            double v1 = a1 * s.ic1eq + a2 * v3;
            double v2 = s.ic2eq + a2 * s.ic1eq + a3 * v3;

            s.ic1eq = 2.0 * v1 - s.ic1eq;
            s.ic2eq = 2.0 * v2 - s.ic2eq;

            switch (baseType)
            {
                case BaseType::LP:    v0 = v2; break;
                case BaseType::HP:    v0 = v0 - k * v1 - v2; break;
                case BaseType::BP:    v0 = v1; break;
                case BaseType::Notch: v0 = v0 - k * v1; break;
                case BaseType::AP:    v0 = v0 - 2.0 * k * v1; break;
            }
        }
    }

    return static_cast<float>(v0);
}
