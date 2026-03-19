#include "VocalFilter.h"

void VocalFilter::prepare(double sampleRate, int blockSize)
{
    formant.prepare(sampleRate, blockSize);
}

void VocalFilter::setType(Type t)
{
    type = t;
    switch (type)
    {
        case Type::Male:
            cutoffMultiplier = 0.8f;  // lower formants
            formant.setType(FormantFilter::Type::Morph);
            break;
        case Type::Female:
            cutoffMultiplier = 1.2f;  // higher formants
            formant.setType(FormantFilter::Type::Morph);
            break;
        case Type::Whisper:
            cutoffMultiplier = 1.0f;  // breathy, high resonance
            formant.setType(FormantFilter::Type::Morph);
            break;
    }
}

void VocalFilter::setCutoff(float freqHz)
{
    formant.setCutoff(freqHz * cutoffMultiplier);
    // Use cutoff to morph through vowels: 20Hz=A, 20kHz=U
    float normalizedCutoff = (std::log2(freqHz) - std::log2(20.f))
                             / (std::log2(20000.f) - std::log2(20.f));
    formant.setMorph(std::clamp(normalizedCutoff, 0.f, 1.f));
}

void VocalFilter::setResonance(float reso)
{
    float adjustedReso = reso;
    if (type == Type::Whisper)
        adjustedReso = std::min(reso * 0.5f, 0.5f); // wider, breathier

    formant.setResonance(adjustedReso);
}

void VocalFilter::reset()
{
    formant.reset();
}

float VocalFilter::processSample(int channel, float input)
{
    return formant.processSample(channel, input);
}
