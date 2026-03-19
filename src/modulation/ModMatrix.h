#pragma once
#include "LFO.h"
#include "Envelope.h"
#include "EnvFollower.h"
#include "StepSequencer.h"
#include <array>

class ModMatrix
{
public:
    static constexpr int kNumSlots = 8;
    static constexpr int kNumSources = 6;
    static constexpr int kNumDestinations = 20;

    enum class Source
    {
        LFO1, LFO2, Env1, Env2, EnvFollow, StepSeq
    };

    enum class Destination
    {
        F1Cutoff, F1Reso, F1Drive, F1Pan,
        F2Cutoff, F2Reso, F2Drive, F2Pan,
        DistDrive, DistMix, DistTone,
        DelayTime, DelayFb, DelayMix,
        LFO1Rate, LFO2Rate,
        OutputLevel, DryWet,
        FbAmount, FbDelay
    };

    struct Slot
    {
        int sourceId = -1;  // -1 = disabled
        int destId = -1;
        float amount = 0.f; // -1..1
    };

    void prepare(double sampleRate);
    void reset();

    // Access modulation sources
    LFO& getLFO1() { return lfo1; }
    LFO& getLFO2() { return lfo2; }
    Envelope& getEnv1() { return env1; }
    Envelope& getEnv2() { return env2; }
    EnvFollower& getEnvFollower() { return envFollower; }
    StepSequencer& getStepSeq() { return stepSeq; }

    void setSlot(int index, int sourceId, int destId, float amount);
    const Slot& getSlot(int index) const { return slots[index]; }

    // Process all sources and calculate modulation offsets
    // Call once per sample, pass input for envelope follower
    void process(float inputL, float inputR);

    // Get the modulation offset for a destination (after process())
    float getModOffset(Destination dest) const;
    float getModOffset(int destId) const;

    static const char* getSourceName(int id);
    static const char* getDestName(int id);

private:
    LFO lfo1, lfo2;
    Envelope env1, env2;
    EnvFollower envFollower;
    StepSequencer stepSeq;

    std::array<Slot, kNumSlots> slots{};
    std::array<float, kNumSources> sourceValues{};
    std::array<float, kNumDestinations> modOffsets{};
};
