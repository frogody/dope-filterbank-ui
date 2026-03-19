#pragma once
#include <array>
#include <cmath>
#include <algorithm>

class StepSequencer
{
public:
    static constexpr int kMaxSteps = 32;

    void prepare(double sampleRate);
    void setNumSteps(int n);
    void setStepValue(int step, float value); // -1..1
    void setGlide(float amount);              // 0..1
    void setSwing(float amount);              // 0..1 (0.5 = no swing)
    void setBPM(double bpm, int division = 16);
    void reset();

    float process(); // returns -1..1

private:
    double sr = 44100.0;
    int numSteps = 16;
    std::array<float, kMaxSteps> steps{};
    float glide = 0.f;
    float swing = 0.5f;
    double bpm = 120.0;
    int division = 16;

    double sampleCounter = 0.0;
    double samplesPerStep = 0.0;
    int currentStep = 0;
    float currentValue = 0.f;
    float targetValue = 0.f;
    float glideCoeff = 0.f;

    void updateTiming();
};
