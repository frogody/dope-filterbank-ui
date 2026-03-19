#include "FilterEngine.h"
#include <cmath>

const std::array<FilterEngine::TypeMapping, FilterEngine::kNumTypes>&
FilterEngine::getTypeMap()
{
    static const std::array<TypeMapping, kNumTypes> map = {{
        // SVF types (0-20)
        {Category::SVF, 0,  "LP 6dB"},
        {Category::SVF, 1,  "LP 12dB"},
        {Category::SVF, 2,  "LP 18dB"},
        {Category::SVF, 3,  "LP 24dB"},
        {Category::SVF, 4,  "LP 36dB"},
        {Category::SVF, 5,  "LP 48dB"},
        {Category::SVF, 6,  "HP 6dB"},
        {Category::SVF, 7,  "HP 12dB"},
        {Category::SVF, 8,  "HP 18dB"},
        {Category::SVF, 9,  "HP 24dB"},
        {Category::SVF, 10, "HP 36dB"},
        {Category::SVF, 11, "HP 48dB"},
        {Category::SVF, 12, "BP 6dB"},
        {Category::SVF, 13, "BP 12dB"},
        {Category::SVF, 14, "BP 24dB"},
        {Category::SVF, 15, "BP 36dB"},
        {Category::SVF, 16, "Notch 6dB"},
        {Category::SVF, 17, "Notch 12dB"},
        {Category::SVF, 18, "Notch 24dB"},
        {Category::SVF, 19, "AP 6dB"},
        {Category::SVF, 20, "AP 12dB"},
        // 21: AP 24dB
        {Category::SVF, 21, "AP 24dB"},
        // Ladder (22-24)
        {Category::Ladder, 0, "Ladder LP"},
        {Category::Ladder, 1, "Ladder HP"},
        {Category::Ladder, 2, "Ladder BP"},
        // Comb (25-27)
        {Category::Comb, 0, "Comb+"},
        {Category::Comb, 1, "Comb-"},
        {Category::Comb, 2, "Comb BP"},
        // Resonator (28-30)
        {Category::Resonator, 0, "Reso Mono"},
        {Category::Resonator, 1, "Reso Stereo"},
        {Category::Resonator, 2, "Reso Tuned"},
        // Formant (31-36)
        {Category::Formant, 0, "Vowel A"},
        {Category::Formant, 1, "Vowel E"},
        {Category::Formant, 2, "Vowel I"},
        {Category::Formant, 3, "Vowel O"},
        {Category::Formant, 4, "Vowel U"},
        {Category::Formant, 5, "Vowel Morph"},
        // Phaser (37-39)
        {Category::Phaser, 0, "Phase 4"},
        {Category::Phaser, 1, "Phase 8"},
        {Category::Phaser, 2, "Phase 12"},
        // EQ (40-43)
        {Category::EQ, 0, "Low Shelf"},
        {Category::EQ, 1, "High Shelf"},
        {Category::EQ, 2, "Peak"},
        {Category::EQ, 3, "Tilt"},
        // FM (44-46)
        {Category::FM, 0, "FM LP"},
        {Category::FM, 1, "FM HP"},
        {Category::FM, 2, "FM BP"},
        // AM (47-48)
        {Category::AM, 0, "AM Ring"},
        {Category::AM, 1, "AM Tremolo"},
        // Vocal (49-51) — renumbered from Special to fit 52
        {Category::Vocal, 0, "Vocal Male"},
        {Category::Vocal, 1, "Vocal Female"},
        {Category::Vocal, 2, "Vocal Whisper"},
    }};
    return map;
}

const char* FilterEngine::getTypeName(int index)
{
    if (index < 0 || index >= kNumTypes) return "Unknown";
    return getTypeMap()[index].name;
}

void FilterEngine::prepare(double sr, int blockSize)
{
    sampleRate = sr;
    svf.prepare(sr, blockSize);
    ladder.prepare(sr, blockSize);
    comb.prepare(sr, blockSize);
    resonator.prepare(sr, blockSize);
    phaser.prepare(sr, blockSize);
    formant.prepare(sr, blockSize);
    vocal.prepare(sr, blockSize);
    eq.prepare(sr, blockSize);
    fm.prepare(sr, blockSize);
    am.prepare(sr, blockSize);
    special.prepare(sr, blockSize);
}

void FilterEngine::setType(int typeIndex)
{
    if (typeIndex < 0 || typeIndex >= kNumTypes) return;
    currentType = typeIndex;

    auto& map = getTypeMap();
    auto& entry = map[typeIndex];

    switch (entry.category)
    {
        case Category::SVF:
            svf.setType(static_cast<SVFilter::Type>(entry.subType));
            break;
        case Category::Ladder:
            ladder.setType(static_cast<LadderFilter::Type>(entry.subType));
            break;
        case Category::Comb:
            comb.setType(static_cast<CombFilter::Type>(entry.subType));
            break;
        case Category::Resonator:
            resonator.setType(static_cast<Resonator::Type>(entry.subType));
            break;
        case Category::Phaser:
            phaser.setType(static_cast<PhaserFilter::Type>(entry.subType));
            break;
        case Category::Formant:
            formant.setType(static_cast<FormantFilter::Type>(entry.subType));
            break;
        case Category::Vocal:
            vocal.setType(static_cast<VocalFilter::Type>(entry.subType));
            break;
        case Category::EQ:
            eq.setType(static_cast<EQFilter::Type>(entry.subType));
            break;
        case Category::FM:
            fm.setType(static_cast<FMFilter::Type>(entry.subType));
            break;
        case Category::AM:
            am.setType(static_cast<AMFilter::Type>(entry.subType));
            break;
        case Category::Special:
            special.setType(static_cast<SpecialFilter::Type>(entry.subType));
            break;
    }
}

void FilterEngine::setCutoff(float hz)
{
    svf.setCutoff(hz);
    ladder.setCutoff(hz);
    comb.setCutoff(hz);
    resonator.setCutoff(hz);
    phaser.setCutoff(hz);
    formant.setCutoff(hz);
    vocal.setCutoff(hz);
    eq.setCutoff(hz);
    fm.setCutoff(hz);
    am.setCutoff(hz);
    special.setCutoff(hz);
}

void FilterEngine::setResonance(float reso)
{
    svf.setResonance(reso);
    ladder.setResonance(reso);
    comb.setResonance(reso);
    resonator.setResonance(reso);
    phaser.setResonance(reso);
    formant.setResonance(reso);
    vocal.setResonance(reso);
    eq.setResonance(reso);
    fm.setResonance(reso);
    am.setResonance(reso);
    special.setResonance(reso);
}

void FilterEngine::setDrive(float drive)
{
    svf.setDrive(drive);
    // Other filters don't have a separate drive parameter
}

void FilterEngine::setPan(float p)
{
    pan = std::clamp(p, -1.f, 1.f);
}

void FilterEngine::reset()
{
    svf.reset();
    ladder.reset();
    comb.reset();
    resonator.reset();
    phaser.reset();
    formant.reset();
    vocal.reset();
    eq.reset();
    fm.reset();
    am.reset();
    special.reset();
}

void FilterEngine::processStereo(float* left, float* right, int numSamples)
{
    auto& map = getTypeMap();
    auto category = map[currentType].category;

    // Pan law: constant power
    float panL = std::cos((pan + 1.f) * 0.25f * static_cast<float>(M_PI));
    float panR = std::sin((pan + 1.f) * 0.25f * static_cast<float>(M_PI));

    for (int i = 0; i < numSamples; ++i)
    {
        float outL, outR;

        switch (category)
        {
            case Category::SVF:
                outL = svf.processSample(0, left[i]);
                outR = svf.processSample(1, right[i]);
                break;
            case Category::Ladder:
                outL = ladder.processSample(0, left[i]);
                outR = ladder.processSample(1, right[i]);
                break;
            case Category::Comb:
                outL = comb.processSample(0, left[i]);
                outR = comb.processSample(1, right[i]);
                break;
            case Category::Resonator:
                outL = resonator.processSample(0, left[i]);
                outR = resonator.processSample(1, right[i]);
                break;
            case Category::Phaser:
                outL = phaser.processSample(0, left[i]);
                outR = phaser.processSample(1, right[i]);
                break;
            case Category::Formant:
                outL = formant.processSample(0, left[i]);
                outR = formant.processSample(1, right[i]);
                break;
            case Category::Vocal:
                outL = vocal.processSample(0, left[i]);
                outR = vocal.processSample(1, right[i]);
                break;
            case Category::EQ:
                outL = eq.processSample(0, left[i]);
                outR = eq.processSample(1, right[i]);
                break;
            case Category::FM:
                outL = fm.processSample(0, left[i]);
                outR = fm.processSample(1, right[i]);
                break;
            case Category::AM:
                outL = am.processSample(0, left[i]);
                outR = am.processSample(1, right[i]);
                break;
            case Category::Special:
                outL = special.processSample(0, left[i]);
                outR = special.processSample(1, right[i]);
                break;
            default:
                outL = left[i];
                outR = right[i];
                break;
        }

        left[i] = outL * panL;
        right[i] = outR * panR;
    }
}
