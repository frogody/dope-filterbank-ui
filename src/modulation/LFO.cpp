#include "LFO.h"

void LFO::prepare(double sampleRate)
{
    sr = sampleRate;
    reset();
    updateRate();
}

void LFO::setRate(float hz)
{
    rate = std::clamp(hz, 0.01f, 5000.f);
    updateRate();
}

void LFO::setBPMSync(bool sync, double newBpm, int div)
{
    bpmSync = sync;
    bpm = newBpm;
    division = div;
    updateRate();
}

void LFO::setFadeIn(float seconds)
{
    fadeInSamples = seconds * static_cast<float>(sr);
}

void LFO::reset()
{
    phase = 0.0;
    fadeInCounter = 0.f;
    shValue = 0.f;
    prevRandom = 0.f;
    nextRandom = 0.f;
}

void LFO::updateRate()
{
    if (bpmSync && bpm > 0)
    {
        double beatHz = bpm / 60.0;
        double noteHz = beatHz * division / 4.0;
        phaseInc = noteHz / sr;
    }
    else
    {
        phaseInc = rate / sr;
    }
}

float LFO::process()
{
    float output = 0.f;
    bool newCycle = false;

    switch (waveform)
    {
        case Waveform::Sine:
            output = static_cast<float>(std::sin(2.0 * M_PI * phase));
            break;

        case Waveform::Triangle:
        {
            double p = phase;
            output = static_cast<float>(p < 0.5 ? 4.0 * p - 1.0 : 3.0 - 4.0 * p);
            break;
        }

        case Waveform::SawUp:
            output = static_cast<float>(2.0 * phase - 1.0);
            break;

        case Waveform::SawDown:
            output = static_cast<float>(1.0 - 2.0 * phase);
            break;

        case Waveform::Square:
            output = phase < 0.5 ? 1.f : -1.f;
            break;

        case Waveform::SampleAndHold:
        {
            double prevPhase = phase - phaseInc;
            if (prevPhase < 0) prevPhase += 1.0;
            if (phase < prevPhase) // wrapped
            {
                std::uniform_real_distribution<float> dist(-1.f, 1.f);
                shValue = dist(rng);
            }
            output = shValue;
            break;
        }

        case Waveform::RandomSmooth:
        {
            double prevPhase = phase - phaseInc;
            if (prevPhase < 0) prevPhase += 1.0;
            if (phase < prevPhase) // wrapped
            {
                prevRandom = nextRandom;
                std::uniform_real_distribution<float> dist(-1.f, 1.f);
                nextRandom = dist(rng);
            }
            // Cosine interpolation between random values
            float t = static_cast<float>(phase);
            float interp = (1.f - std::cos(t * static_cast<float>(M_PI))) * 0.5f;
            output = prevRandom + (nextRandom - prevRandom) * interp;
            break;
        }

        case Waveform::Stepped:
        {
            // 8 quantized steps per cycle
            int step = static_cast<int>(phase * 8.0) % 8;
            output = (step / 3.5f) - 1.f; // -1..1 in 8 steps
            break;
        }
    }

    // Advance phase
    phase += phaseInc;
    if (phase >= 1.0) phase -= 1.0;

    // Apply fade-in
    if (fadeInSamples > 0.f && fadeInCounter < fadeInSamples)
    {
        output *= fadeInCounter / fadeInSamples;
        fadeInCounter += 1.f;
    }

    return output;
}
