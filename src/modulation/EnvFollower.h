#pragma once
#include <cmath>
#include <algorithm>

class EnvFollower
{
public:
    void prepare(double sampleRate);
    void setAttack(float ms);
    void setRelease(float ms);
    void setSensitivity(float s); // 0..1
    void reset();

    float process(float inputL, float inputR); // returns 0..1

private:
    double sr = 44100.0;
    float attackCoeff = 0.f;
    float releaseCoeff = 0.f;
    float sensitivity = 0.5f;
    float envelope = 0.f;
};
