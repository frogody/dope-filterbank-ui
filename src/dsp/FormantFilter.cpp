#include "FormantFilter.h"

void FormantFilter::prepare(double sampleRate, int /*blockSize*/)
{
    sr = sampleRate;
    reset();
    updateCoefficients();
}

void FormantFilter::setType(Type t)
{
    type = t;
    updateCoefficients();
}

void FormantFilter::setCutoff(float freqHz)
{
    // Use cutoff as a shift multiplier relative to 1kHz center
    cutoffShift = std::clamp(freqHz, 20.f, 20000.f) / 1000.f;
    updateCoefficients();
}

void FormantFilter::setResonance(float reso)
{
    resonance = std::clamp(reso, 0.f, 1.f);
    updateCoefficients();
}

void FormantFilter::setMorph(float m)
{
    morphValue = std::clamp(m, 0.f, 1.f);
    if (type == Type::Morph)
        updateCoefficients();
}

void FormantFilter::reset()
{
    for (auto& ch : state)
        for (auto& s : ch)
            s = {0, 0, 0, 0};
}

void FormantFilter::calcFormantBiquad(double freq, double bw, double gain,
                                       double& ob0, double& ob1, double& ob2,
                                       double& oa1, double& oa2)
{
    // Bandpass peak filter (Bristow-Johnson)
    double A = std::pow(10.0, gain / 40.0);
    double w0 = 2.0 * M_PI * std::min(freq, sr * 0.49) / sr;
    double bwScaled = bw * (1.0 + (1.0 - resonance) * 3.0); // wider at low resonance
    double Q = freq / bwScaled;
    double alpha = std::sin(w0) / (2.0 * Q);

    double norm = 1.0 / (1.0 + alpha / A);
    ob0 = (1.0 + alpha * A) * norm;
    ob1 = (-2.0 * std::cos(w0)) * norm;
    ob2 = (1.0 - alpha * A) * norm;
    oa1 = ob1; // same as b1 for peak filter
    oa2 = (1.0 - alpha / A) * norm;
}

void FormantFilter::updateCoefficients()
{
    int vowelIdx = 0;
    double vowelBlend[5] = {0, 0, 0, 0, 0};

    if (type == Type::Morph)
    {
        // Morph across all 5 vowels
        float pos = morphValue * 4.f;
        int idx = std::min(static_cast<int>(pos), 3);
        float frac = pos - idx;
        vowelBlend[idx] = 1.0 - frac;
        vowelBlend[idx + 1] = frac;
    }
    else
    {
        vowelIdx = static_cast<int>(type);
        vowelBlend[vowelIdx] = 1.0;
    }

    for (int f = 0; f < kNumFormants; ++f)
    {
        // Interpolate formant parameters across active vowels
        double freq = 0, bw = 0, gain = 0;
        for (int v = 0; v < 5; ++v)
        {
            if (vowelBlend[v] > 0)
            {
                freq += formantData[v][f][0] * vowelBlend[v];
                bw   += formantData[v][f][1] * vowelBlend[v];
                gain += formantData[v][f][2] * vowelBlend[v];
            }
        }

        freq *= cutoffShift;
        calcFormantBiquad(freq, bw, gain,
                          coeffs[f].b0, coeffs[f].b1, coeffs[f].b2,
                          coeffs[f].a1, coeffs[f].a2);
    }
}

float FormantFilter::processSample(int channel, float input)
{
    double x0 = static_cast<double>(input);
    double output = 0.0;

    // Parallel formant bands
    for (int f = 0; f < kNumFormants; ++f)
    {
        auto& s = state[channel][f];
        auto& c = coeffs[f];

        double y = c.b0 * x0 + c.b1 * s.x1 + c.b2 * s.x2
                   - c.a1 * s.y1 - c.a2 * s.y2;

        s.x2 = s.x1; s.x1 = x0;
        s.y2 = s.y1; s.y1 = y;

        output += y;
    }

    return static_cast<float>(output / kNumFormants);
}
