#include "EnvFollower.h"

void EnvFollower::prepare(double sampleRate)
{
    sr = sampleRate;
    reset();
    setAttack(1.f);
    setRelease(50.f);
}

void EnvFollower::setAttack(float ms)
{
    ms = std::max(0.1f, ms);
    attackCoeff = std::exp(-1.f / (ms * static_cast<float>(sr) / 1000.f));
}

void EnvFollower::setRelease(float ms)
{
    ms = std::max(0.1f, ms);
    releaseCoeff = std::exp(-1.f / (ms * static_cast<float>(sr) / 1000.f));
}

void EnvFollower::setSensitivity(float s)
{
    sensitivity = std::clamp(s, 0.f, 1.f);
}

void EnvFollower::reset()
{
    envelope = 0.f;
}

float EnvFollower::process(float inputL, float inputR)
{
    // Take max of both channels
    float level = std::max(std::abs(inputL), std::abs(inputR));

    // Scale by sensitivity (1..100x gain)
    level *= (1.f + sensitivity * 99.f);

    // Envelope follower with separate attack/release
    if (level > envelope)
        envelope = attackCoeff * envelope + (1.f - attackCoeff) * level;
    else
        envelope = releaseCoeff * envelope + (1.f - releaseCoeff) * level;

    return std::min(envelope, 1.f);
}
