#pragma once
#include "Oversampler.h"
#include "EQFilter.h"
#include <juce_audio_basics/juce_audio_basics.h>

class Distortion
{
public:
    enum class Type
    {
        HardClip, SoftClip, TubeAmp, Tape,
        Wavefold, Waveshape, Bitcrush, Rectify,
        Fuzz, Square, Cubic, Decimate
    };

    static constexpr int kNumTypes = 12;

    void prepare(double sampleRate, int blockSize);
    void setType(Type t) { type = t; }
    void setDrive(float d);
    void setMix(float m);
    void setTone(float t);
    void setOversampleFactor(int factor);

    void processStereo(juce::AudioBuffer<float>& buffer);

    static const char* getTypeName(int index);

private:
    float processSample(float input);

    Type type = Type::SoftClip;
    float drive = 0.5f;
    float mix = 1.f;
    float tone = 0.f;
    double sampleRate = 44100.0;

    Oversampler oversampler;
    EQFilter toneFilter;

    // Decimate state
    float decimateHold[2] = {0.f, 0.f};
    int decimateCount[2] = {0, 0};
};
