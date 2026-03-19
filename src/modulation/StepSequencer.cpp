#include "StepSequencer.h"

void StepSequencer::prepare(double sampleRate)
{
    sr = sampleRate;
    reset();
    updateTiming();
}

void StepSequencer::setNumSteps(int n)
{
    numSteps = std::clamp(n, 1, kMaxSteps);
}

void StepSequencer::setStepValue(int step, float value)
{
    if (step >= 0 && step < kMaxSteps)
        steps[step] = std::clamp(value, -1.f, 1.f);
}

void StepSequencer::setGlide(float amount)
{
    glide = std::clamp(amount, 0.f, 1.f);
    if (glide > 0.f)
        glideCoeff = std::exp(-1.f / (glide * static_cast<float>(samplesPerStep) * 0.5f + 1.f));
    else
        glideCoeff = 0.f;
}

void StepSequencer::setSwing(float amount)
{
    swing = std::clamp(amount, 0.f, 1.f);
}

void StepSequencer::setBPM(double newBpm, int div)
{
    bpm = std::max(20.0, newBpm);
    division = std::max(1, div);
    updateTiming();
}

void StepSequencer::reset()
{
    sampleCounter = 0.0;
    currentStep = 0;
    currentValue = steps[0];
    targetValue = steps[0];
}

void StepSequencer::updateTiming()
{
    // Samples per step based on BPM and division
    double beatsPerSecond = bpm / 60.0;
    double stepsPerBeat = division / 4.0;
    samplesPerStep = sr / (beatsPerSecond * stepsPerBeat);
}

float StepSequencer::process()
{
    sampleCounter += 1.0;

    // Apply swing: even steps are longer, odd steps shorter
    double currentStepLength = samplesPerStep;
    if (currentStep % 2 == 0)
        currentStepLength *= (1.0 + (swing - 0.5) * 0.5);
    else
        currentStepLength *= (1.0 - (swing - 0.5) * 0.5);

    if (sampleCounter >= currentStepLength)
    {
        sampleCounter -= currentStepLength;
        currentStep = (currentStep + 1) % numSteps;
        targetValue = steps[currentStep];
    }

    // Glide
    if (glide > 0.f)
        currentValue = currentValue * glideCoeff + targetValue * (1.f - glideCoeff);
    else
        currentValue = targetValue;

    return currentValue;
}
