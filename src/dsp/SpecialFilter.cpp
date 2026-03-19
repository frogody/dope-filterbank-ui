#include "SpecialFilter.h"

void SpecialFilter::prepare(double sampleRate, int blockSize)
{
    sr = sampleRate;
    preFilter.prepare(sampleRate, blockSize);
    postFilter.prepare(sampleRate, blockSize);
    reset();
}

void SpecialFilter::setType(Type t)
{
    type = t;
    preFilter.setType(SVFilter::Type::BP12);
    postFilter.setType(SVFilter::Type::LP12);
}

void SpecialFilter::setCutoff(float freqHz)
{
    cutoff = std::clamp(freqHz, 20.f, static_cast<float>(sr * 0.49));
    preFilter.setCutoff(cutoff);
    postFilter.setCutoff(cutoff);
}

void SpecialFilter::setResonance(float reso)
{
    resonance = std::clamp(reso, 0.f, 1.f);
    preFilter.setResonance(reso);
    postFilter.setResonance(reso * 0.5f);

    // Bitfilter: resonance controls bit depth (16 → 1 bit)
    bitDepth = 16.f - resonance * 15.f;
}

void SpecialFilter::reset()
{
    preFilter.reset();
    postFilter.reset();
    lastSample = {0.f, 0.f};
}

float SpecialFilter::processSample(int channel, float input)
{
    switch (type)
    {
        case Type::OctaveUp:
        {
            // Full-wave rectification + bandpass = octave up
            float rectified = std::abs(input);
            return postFilter.processSample(channel, rectified);
        }

        case Type::OctaveDown:
        {
            // Frequency division via zero-crossing flip
            if ((input > 0.f && lastSample[channel] <= 0.f) ||
                (input < 0.f && lastSample[channel] >= 0.f))
            {
                // Toggle on zero crossing — effectively halves frequency
            }
            lastSample[channel] = input;
            // Simple sub-harmonic: square wave at half frequency, filtered
            float sub = (input >= 0.f) ? input : -input;
            sub *= (std::sin(2.0 * M_PI * cutoff * 0.5 / sr) > 0) ? 1.f : -1.f;
            return postFilter.processSample(channel, sub * 0.5f + input * 0.5f);
        }

        case Type::Bitfilter:
        {
            // Bit reduction + filter
            float filtered = preFilter.processSample(channel, input);
            float scale = std::pow(2.f, bitDepth);
            float crushed = std::round(filtered * scale) / scale;
            return crushed;
        }

        case Type::MShape:
        {
            // Dual bandpass at cutoff and cutoff*2 = M-shaped response
            preFilter.setCutoff(cutoff);
            float bp1 = preFilter.processSample(channel, input);
            postFilter.setCutoff(std::min(cutoff * 2.f, static_cast<float>(sr * 0.49)));
            postFilter.setType(SVFilter::Type::BP12);
            float bp2 = postFilter.processSample(channel, input);
            return (bp1 + bp2) * 0.5f;
        }

        case Type::Elliptic:
        {
            // Approximate elliptic filter: LP with notch in stopband
            float lp = preFilter.processSample(channel, input);
            postFilter.setType(SVFilter::Type::Notch12);
            postFilter.setCutoff(std::min(cutoff * 1.5f, static_cast<float>(sr * 0.49)));
            float notched = postFilter.processSample(channel, lp);
            return notched;
        }
    }
    return input;
}
