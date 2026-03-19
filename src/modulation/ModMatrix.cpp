#include "ModMatrix.h"

const char* ModMatrix::getSourceName(int id)
{
    static const char* names[] = {
        "LFO 1", "LFO 2", "Env 1", "Env 2", "Env Follow", "Step Seq"
    };
    if (id < 0 || id >= kNumSources) return "None";
    return names[id];
}

const char* ModMatrix::getDestName(int id)
{
    static const char* names[] = {
        "F1 Cutoff", "F1 Reso", "F1 Drive", "F1 Pan",
        "F2 Cutoff", "F2 Reso", "F2 Drive", "F2 Pan",
        "Dist Drive", "Dist Mix", "Dist Tone",
        "Delay Time", "Delay Fb", "Delay Mix",
        "LFO1 Rate", "LFO2 Rate",
        "Output Level", "Dry/Wet",
        "Fb Amount", "Fb Delay"
    };
    if (id < 0 || id >= kNumDestinations) return "None";
    return names[id];
}

void ModMatrix::prepare(double sampleRate)
{
    lfo1.prepare(sampleRate);
    lfo2.prepare(sampleRate);
    env1.prepare(sampleRate);
    env2.prepare(sampleRate);
    envFollower.prepare(sampleRate);
    stepSeq.prepare(sampleRate);
    reset();
}

void ModMatrix::reset()
{
    lfo1.reset();
    lfo2.reset();
    env1.reset();
    env2.reset();
    envFollower.reset();
    stepSeq.reset();
    sourceValues.fill(0.f);
    modOffsets.fill(0.f);
}

void ModMatrix::setSlot(int index, int sourceId, int destId, float amount)
{
    if (index < 0 || index >= kNumSlots) return;
    slots[index].sourceId = sourceId;
    slots[index].destId = destId;
    slots[index].amount = std::clamp(amount, -1.f, 1.f);
}

void ModMatrix::process(float inputL, float inputR)
{
    // Process all sources
    sourceValues[0] = lfo1.process();
    sourceValues[1] = lfo2.process();
    sourceValues[2] = env1.process();
    sourceValues[3] = env2.process();
    sourceValues[4] = envFollower.process(inputL, inputR);
    sourceValues[5] = stepSeq.process();

    // Clear offsets
    modOffsets.fill(0.f);

    // Apply routing slots
    for (const auto& slot : slots)
    {
        if (slot.sourceId < 0 || slot.sourceId >= kNumSources) continue;
        if (slot.destId < 0 || slot.destId >= kNumDestinations) continue;
        if (slot.amount == 0.f) continue;

        modOffsets[slot.destId] += sourceValues[slot.sourceId] * slot.amount;
    }
}

float ModMatrix::getModOffset(Destination dest) const
{
    return modOffsets[static_cast<int>(dest)];
}

float ModMatrix::getModOffset(int destId) const
{
    if (destId < 0 || destId >= kNumDestinations) return 0.f;
    return modOffsets[destId];
}
