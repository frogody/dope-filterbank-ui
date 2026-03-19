#include "Envelope.h"

void Envelope::prepare(double sampleRate)
{
    sr = sampleRate;
    reset();
}

void Envelope::setDelay(float ms) { delayMs = std::max(0.f, ms); }
void Envelope::setAttack(float ms) { attackMs = std::max(0.1f, ms); }
void Envelope::setHold(float ms) { holdMs = std::max(0.f, ms); }
void Envelope::setDecay(float ms) { decayMs = std::max(0.1f, ms); }
void Envelope::setSustain(float level) { sustainLevel = std::clamp(level, 0.f, 1.f); }
void Envelope::setRelease(float ms) { releaseMs = std::max(0.1f, ms); }

void Envelope::trigger()
{
    if (retrigger || stage == Stage::Idle)
    {
        if (delayMs > 0.f)
        {
            stage = Stage::Delay;
            stageCounter = 0.f;
            stageSamples = msToSamples(delayMs);
        }
        else
        {
            stage = Stage::Attack;
            stageCounter = 0.f;
            stageSamples = msToSamples(attackMs);
        }
    }
}

void Envelope::release()
{
    if (stage != Stage::Idle)
    {
        stage = Stage::Release;
        stageCounter = 0.f;
        stageSamples = msToSamples(releaseMs);
    }
}

void Envelope::reset()
{
    stage = Stage::Idle;
    output = 0.f;
    stageCounter = 0.f;
}

float Envelope::process()
{
    switch (stage)
    {
        case Stage::Idle:
            output = 0.f;
            break;

        case Stage::Delay:
            output = 0.f;
            stageCounter += 1.f;
            if (stageCounter >= stageSamples)
            {
                stage = Stage::Attack;
                stageCounter = 0.f;
                stageSamples = msToSamples(attackMs);
            }
            break;

        case Stage::Attack:
        {
            float t = stageCounter / stageSamples;
            // Exponential curve (concave up)
            output = 1.f - std::exp(-5.f * t);
            output = std::min(output / (1.f - std::exp(-5.f)), 1.f);
            stageCounter += 1.f;
            if (stageCounter >= stageSamples)
            {
                output = 1.f;
                if (holdMs > 0.f)
                {
                    stage = Stage::Hold;
                    stageCounter = 0.f;
                    stageSamples = msToSamples(holdMs);
                }
                else
                {
                    stage = Stage::Decay;
                    stageCounter = 0.f;
                    stageSamples = msToSamples(decayMs);
                }
            }
            break;
        }

        case Stage::Hold:
            output = 1.f;
            stageCounter += 1.f;
            if (stageCounter >= stageSamples)
            {
                stage = Stage::Decay;
                stageCounter = 0.f;
                stageSamples = msToSamples(decayMs);
            }
            break;

        case Stage::Decay:
        {
            float t = stageCounter / stageSamples;
            // Exponential decay from 1 to sustain
            output = sustainLevel + (1.f - sustainLevel) * std::exp(-5.f * t);
            stageCounter += 1.f;
            if (stageCounter >= stageSamples)
            {
                output = sustainLevel;
                stage = Stage::Sustain;
            }
            break;
        }

        case Stage::Sustain:
            output = sustainLevel;
            break;

        case Stage::Release:
        {
            float t = stageCounter / stageSamples;
            float startLevel = output; // release from current level
            output = startLevel * std::exp(-5.f * t);
            stageCounter += 1.f;
            if (stageCounter >= stageSamples || output < 0.001f)
            {
                output = 0.f;
                stage = Stage::Idle;
            }
            break;
        }
    }

    return output;
}
