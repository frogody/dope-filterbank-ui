#include "AMFilter.h"

void AMFilter::prepare(double sampleRate, int /*blockSize*/)
{
    sr = sampleRate;
    reset();
}

void AMFilter::setCutoff(float freqHz)
{
    modFreq = std::clamp(freqHz, 0.1f, static_cast<float>(sr * 0.49));
}

void AMFilter::setResonance(float reso)
{
    depth = std::clamp(reso, 0.f, 1.f);
}

void AMFilter::reset()
{
    phase = {0.0, 0.0};
}

float AMFilter::processSample(int channel, float input)
{
    double mod = std::sin(2.0 * M_PI * phase[channel]);
    phase[channel] += modFreq / sr;
    if (phase[channel] >= 1.0) phase[channel] -= 1.0;

    switch (type)
    {
        case Type::Ring:
            // Bipolar modulation: -1..1
            return static_cast<float>(input * mod * depth + input * (1.0 - depth));

        case Type::Tremolo:
            // Unipolar modulation: 0..1
            return static_cast<float>(input * (1.0 - depth * 0.5 + mod * depth * 0.5));
    }
    return input;
}
