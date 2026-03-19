#include "Distortion.h"
#include <cmath>
#include <algorithm>

const char* Distortion::getTypeName(int index)
{
    static const char* names[] = {
        "Hard Clip", "Soft Clip", "Tube Amp", "Tape",
        "Wavefold", "Waveshape", "Bitcrush", "Rectify",
        "Fuzz", "Square", "Cubic", "Decimate"
    };
    if (index < 0 || index >= kNumTypes) return "Unknown";
    return names[index];
}

void Distortion::prepare(double sr, int blockSize)
{
    sampleRate = sr;
    oversampler.prepare(sr, blockSize);
    toneFilter.prepare(sr, blockSize);
    toneFilter.setType(EQFilter::Type::Tilt);
    decimateHold[0] = decimateHold[1] = 0.f;
    decimateCount[0] = decimateCount[1] = 0;
}

void Distortion::setDrive(float d)
{
    drive = std::clamp(d, 0.f, 1.f);
}

void Distortion::setMix(float m)
{
    mix = std::clamp(m, 0.f, 1.f);
}

void Distortion::setTone(float t)
{
    tone = std::clamp(t, -1.f, 1.f);
    // Map -1..1 to resonance 0..1 for tilt EQ
    toneFilter.setResonance((tone + 1.f) * 0.5f);
    toneFilter.setCutoff(1000.f);
}

void Distortion::setOversampleFactor(int factor)
{
    oversampler.setFactor(factor);
}

float Distortion::processSample(float input)
{
    // Scale drive: 0..1 maps to 1..50x gain
    float d = 1.f + drive * 49.f;
    float x = input * d;

    switch (type)
    {
        case Type::HardClip:
            return std::clamp(x, -1.f, 1.f);

        case Type::SoftClip:
            return std::tanh(x);

        case Type::TubeAmp:
        {
            // Asymmetric waveshaping — positive clips harder
            if (x >= 0.f)
                return 1.f - std::exp(-x);
            else
                return -(1.f - std::exp(x)) * 0.8f; // slight asymmetry
        }

        case Type::Tape:
        {
            // Tape saturation approximation
            float absX = std::abs(x);
            return x * (absX + drive) / (x * x + (drive) * absX + 1.f);
        }

        case Type::Wavefold:
        {
            // Sine wavefolder
            return std::sin(x * static_cast<float>(M_PI));
        }

        case Type::Waveshape:
        {
            // Chebyshev polynomial T3(x) = 4x^3 - 3x
            float clipped = std::clamp(x / d, -1.f, 1.f); // normalize back
            float x2 = clipped * clipped;
            float t2 = 2.f * x2 - 1.f;
            float t3 = clipped * (4.f * x2 - 3.f);
            // Blend between harmonics based on drive
            return clipped * (1.f - drive) + (t2 * 0.3f + t3 * 0.7f) * drive;
        }

        case Type::Bitcrush:
        {
            // Bit depth reduction: drive controls bits (16 down to 1)
            float bits = 16.f - drive * 15.f;
            float scale = std::pow(2.f, bits);
            return std::round(input * scale) / scale;
        }

        case Type::Rectify:
        {
            // Full/half wave rectification blended by drive
            float full = std::abs(input);
            float half = std::max(0.f, input);
            return full * drive + half * (1.f - drive);
        }

        case Type::Fuzz:
        {
            // Diode clipper model
            float sign = (x >= 0.f) ? 1.f : -1.f;
            return sign * (1.f - std::exp(-std::abs(x)));
        }

        case Type::Square:
        {
            // Blend between clean and square wave
            float sq = (input >= 0.f) ? 1.f : -1.f;
            return input * (1.f - drive) + sq * drive * std::abs(input);
        }

        case Type::Cubic:
        {
            // Cubic soft saturation: x - x^3/3
            float clipped = std::clamp(x, -1.5f, 1.5f);
            return clipped - (clipped * clipped * clipped) / 3.f;
        }

        case Type::Decimate:
        {
            // Sample-and-hold at reduced rate
            // Drive controls reduction: 1x to 64x
            // Handled per-channel in processStereo, not here
            return input;
        }
    }
    return input;
}

void Distortion::processStereo(juce::AudioBuffer<float>& buffer)
{
    if (buffer.getNumChannels() < 2) return;

    // Store dry signal
    juce::AudioBuffer<float> dryBuffer;
    if (mix < 1.f)
        dryBuffer.makeCopyOf(buffer);

    oversampler.process(buffer, [this](float* left, float* right, int numSamples)
    {
        if (type == Type::Decimate)
        {
            // Decimate needs special per-channel state
            int holdLength = 1 + static_cast<int>(drive * 63.f);
            for (int i = 0; i < numSamples; ++i)
            {
                // Left channel
                if (decimateCount[0] >= holdLength)
                {
                    decimateHold[0] = left[i];
                    decimateCount[0] = 0;
                }
                left[i] = decimateHold[0];
                decimateCount[0]++;

                // Right channel
                if (decimateCount[1] >= holdLength)
                {
                    decimateHold[1] = right[i];
                    decimateCount[1] = 0;
                }
                right[i] = decimateHold[1];
                decimateCount[1]++;
            }
        }
        else
        {
            for (int i = 0; i < numSamples; ++i)
            {
                left[i]  = processSample(left[i]);
                right[i] = processSample(right[i]);
            }
        }
    });

    // Apply tone (tilt EQ)
    if (tone != 0.f)
    {
        float* left  = buffer.getWritePointer(0);
        float* right = buffer.getWritePointer(1);
        for (int i = 0; i < buffer.getNumSamples(); ++i)
        {
            left[i]  = toneFilter.processSample(0, left[i]);
            right[i] = toneFilter.processSample(1, right[i]);
        }
    }

    // Apply dry/wet mix
    if (mix < 1.f)
    {
        float* left  = buffer.getWritePointer(0);
        float* right = buffer.getWritePointer(1);
        for (int i = 0; i < buffer.getNumSamples(); ++i)
        {
            left[i]  = left[i] * mix + dryBuffer.getSample(0, i) * (1.f - mix);
            right[i] = right[i] * mix + dryBuffer.getSample(1, i) * (1.f - mix);
        }
    }
}
