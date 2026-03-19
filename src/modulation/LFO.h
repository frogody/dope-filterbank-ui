#pragma once
#include <cmath>
#include <algorithm>
#include <random>

class LFO
{
public:
    enum class Waveform
    {
        Sine, Triangle, SawUp, SawDown,
        Square, SampleAndHold, RandomSmooth, Stepped
    };

    void prepare(double sampleRate);
    void setWaveform(Waveform w) { waveform = w; }
    void setRate(float hz);           // 0.01..5000 Hz
    void setBPMSync(bool sync, double bpm = 120.0, int division = 4);
    void setFadeIn(float seconds);    // 0..10s
    void reset();

    float process(); // returns -1..1

private:
    Waveform waveform = Waveform::Sine;
    double sr = 44100.0;
    float rate = 1.f;
    bool bpmSync = false;
    double bpm = 120.0;
    int division = 4;

    double phase = 0.0;
    double phaseInc = 0.0;

    // Fade-in
    float fadeInSamples = 0.f;
    float fadeInCounter = 0.f;

    // S&H / Random
    float shValue = 0.f;
    float prevRandom = 0.f;
    float nextRandom = 0.f;
    std::mt19937 rng{42};

    void updateRate();
};
