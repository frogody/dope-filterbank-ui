#include "Resonator.h"

void Resonator::prepare(double sampleRate, int /*blockSize*/)
{
    sr = sampleRate;
    reset();
    updateCoefficients();
}

void Resonator::setCutoff(float freqHz)
{
    cutoff = std::clamp(freqHz, 20.f, static_cast<float>(sr * 0.49));
    updateCoefficients();
}

void Resonator::setResonance(float reso)
{
    resonance = std::clamp(reso, 0.f, 1.f);
    updateCoefficients();
}

void Resonator::reset()
{
    for (auto& ch : state)
        ch = {0.0, 0.0, 0.0, 0.0};
}

void Resonator::updateCoefficients()
{
    // Bandpass biquad (Bristow-Johnson)
    double Q = 1.0 + resonance * 199.0; // Q range 1..200
    double w0 = 2.0 * M_PI * cutoff / sr;
    double alpha = std::sin(w0) / (2.0 * Q);

    double norm = 1.0 / (1.0 + alpha);
    b0 = alpha * norm;
    b1 = 0.0;
    b2 = -alpha * norm;
    a1 = -2.0 * std::cos(w0) * norm;
    a2 = (1.0 - alpha) * norm;

    // Stereo: detune right channel by ~3 cents
    if (type == Type::Stereo)
    {
        double detuned = cutoff * std::pow(2.0, 3.0 / 1200.0);
        double w0R = 2.0 * M_PI * detuned / sr;
        double alphaR = std::sin(w0R) / (2.0 * Q);
        double normR = 1.0 / (1.0 + alphaR);
        b0R = alphaR * normR;
        b1R = 0.0;
        b2R = -alphaR * normR;
        a1R = -2.0 * std::cos(w0R) * normR;
        a2R = (1.0 - alphaR) * normR;
    }
}

float Resonator::processSample(int channel, float input)
{
    auto& s = state[channel];
    double x0 = static_cast<double>(input);

    double cb0, cb2, ca1, ca2;
    if (type == Type::Stereo && channel == 1)
    {
        cb0 = b0R; cb2 = b2R; ca1 = a1R; ca2 = a2R;
    }
    else
    {
        cb0 = b0; cb2 = b2; ca1 = a1; ca2 = a2;
    }

    double y0 = cb0 * x0 + b1 * s.x1 + cb2 * s.x2 - ca1 * s.y1 - ca2 * s.y2;

    s.x2 = s.x1; s.x1 = x0;
    s.y2 = s.y1; s.y1 = y0;

    return static_cast<float>(y0);
}
