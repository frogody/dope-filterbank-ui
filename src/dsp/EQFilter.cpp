#include "EQFilter.h"

void EQFilter::prepare(double sampleRate, int /*blockSize*/)
{
    sr = sampleRate;
    reset();
    updateCoefficients();
}

void EQFilter::setType(Type t)
{
    type = t;
    updateCoefficients();
}

void EQFilter::setCutoff(float freqHz)
{
    cutoff = std::clamp(freqHz, 20.f, static_cast<float>(sr * 0.49));
    updateCoefficients();
}

void EQFilter::setResonance(float reso)
{
    resonance = std::clamp(reso, 0.f, 1.f);
    updateCoefficients();
}

void EQFilter::reset()
{
    for (auto& ch : state)
        ch = {0, 0, 0, 0};
}

void EQFilter::updateCoefficients()
{
    double w0 = 2.0 * M_PI * cutoff / sr;
    double cosw0 = std::cos(w0);
    double sinw0 = std::sin(w0);

    // Gain: resonance 0..1 maps to -12dB..+12dB
    double dBGain = (resonance - 0.5) * 24.0;
    double A = std::pow(10.0, dBGain / 40.0);
    double Q = 0.707; // Butterworth default

    switch (type)
    {
        case Type::LowShelf:
        {
            double alpha = sinw0 / 2.0 * std::sqrt((A + 1.0/A) * (1.0/0.707 - 1.0) + 2.0);
            double sqA = std::sqrt(A);
            a0 =        (A+1) + (A-1)*cosw0 + 2*sqA*alpha;
            b0 =    A*((A+1) - (A-1)*cosw0 + 2*sqA*alpha) / a0;
            b1 =  2*A*((A-1) - (A+1)*cosw0)               / a0;
            b2 =    A*((A+1) - (A-1)*cosw0 - 2*sqA*alpha) / a0;
            a1 =   -2*((A-1) + (A+1)*cosw0)               / a0;
            a2 =       ((A+1) + (A-1)*cosw0 - 2*sqA*alpha) / a0;
            break;
        }
        case Type::HighShelf:
        {
            double alpha = sinw0 / 2.0 * std::sqrt((A + 1.0/A) * (1.0/0.707 - 1.0) + 2.0);
            double sqA = std::sqrt(A);
            a0 =        (A+1) - (A-1)*cosw0 + 2*sqA*alpha;
            b0 =    A*((A+1) + (A-1)*cosw0 + 2*sqA*alpha) / a0;
            b1 = -2*A*((A-1) + (A+1)*cosw0)               / a0;
            b2 =    A*((A+1) + (A-1)*cosw0 - 2*sqA*alpha) / a0;
            a1 =    2*((A-1) - (A+1)*cosw0)               / a0;
            a2 =       ((A+1) - (A-1)*cosw0 - 2*sqA*alpha) / a0;
            break;
        }
        case Type::Peak:
        {
            double alpha = sinw0 / (2.0 * Q);
            a0 = 1.0 + alpha / A;
            b0 = (1.0 + alpha * A) / a0;
            b1 = (-2.0 * cosw0)    / a0;
            b2 = (1.0 - alpha * A) / a0;
            a1 = b1;
            a2 = (1.0 - alpha / A) / a0;
            break;
        }
        case Type::Tilt:
        {
            // Tilt EQ: low shelf + inverted high shelf at same frequency
            double alpha = sinw0 / 2.0 * std::sqrt((A + 1.0/A) * (1.0/0.707 - 1.0) + 2.0);
            double sqA = std::sqrt(A);
            a0 =        (A+1) + (A-1)*cosw0 + 2*sqA*alpha;
            b0 =    A*((A+1) - (A-1)*cosw0 + 2*sqA*alpha) / a0;
            b1 =  2*A*((A-1) - (A+1)*cosw0)               / a0;
            b2 =    A*((A+1) - (A-1)*cosw0 - 2*sqA*alpha) / a0;
            a1 =   -2*((A-1) + (A+1)*cosw0)               / a0;
            a2 =       ((A+1) + (A-1)*cosw0 - 2*sqA*alpha) / a0;
            break;
        }
    }
    a0 = 1.0; // normalized
}

float EQFilter::processSample(int channel, float input)
{
    auto& s = state[channel];
    double x0 = static_cast<double>(input);

    double y0 = b0 * x0 + b1 * s.x1 + b2 * s.x2 - a1 * s.y1 - a2 * s.y2;

    s.x2 = s.x1; s.x1 = x0;
    s.y2 = s.y1; s.y1 = y0;

    return static_cast<float>(y0);
}
