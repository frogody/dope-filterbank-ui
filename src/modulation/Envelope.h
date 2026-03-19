#pragma once
#include <cmath>
#include <algorithm>

// ADSR Envelope with delay, hold, and curve shaping
class Envelope
{
public:
    void prepare(double sampleRate);
    void setDelay(float ms);
    void setAttack(float ms);
    void setHold(float ms);
    void setDecay(float ms);
    void setSustain(float level);   // 0..1
    void setRelease(float ms);
    void setRetrigger(bool r) { retrigger = r; }
    void trigger();
    void release();
    void reset();

    float process(); // returns 0..1

private:
    enum class Stage { Idle, Delay, Attack, Hold, Decay, Sustain, Release };

    Stage stage = Stage::Idle;
    double sr = 44100.0;
    float output = 0.f;

    float delayMs = 0.f, attackMs = 10.f, holdMs = 0.f;
    float decayMs = 100.f, sustainLevel = 0.7f, releaseMs = 200.f;
    bool retrigger = true;

    float stageCounter = 0.f;
    float stageSamples = 0.f;

    float msToSamples(float ms) const { return ms * static_cast<float>(sr) / 1000.f; }
};
