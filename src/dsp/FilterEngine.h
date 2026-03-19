#pragma once
#include "SVFilter.h"
#include "LadderFilter.h"
#include "CombFilter.h"
#include "Resonator.h"
#include "PhaserFilter.h"
#include "FormantFilter.h"
#include "VocalFilter.h"
#include "EQFilter.h"
#include "FMFilter.h"
#include "AMFilter.h"
#include "SpecialFilter.h"
#include <array>

class FilterEngine
{
public:
    // All 52 filter types indexed 0-51
    static constexpr int kNumTypes = 52;

    void prepare(double sampleRate, int blockSize);
    void setType(int typeIndex);
    void setCutoff(float hz);
    void setResonance(float reso);
    void setDrive(float drive);
    void setPan(float pan);
    void reset();

    void processStereo(float* left, float* right, int numSamples);

    int getTypeIndex() const { return currentType; }
    static const char* getTypeName(int index);

private:
    enum class Category { SVF, Ladder, Comb, Resonator, Phaser,
                          Formant, Vocal, EQ, FM, AM, Special };

    struct TypeMapping
    {
        Category category;
        int subType;
        const char* name;
    };

    static const std::array<TypeMapping, kNumTypes>& getTypeMap();

    int currentType = 1; // LP12 default
    double sampleRate = 44100.0;
    float pan = 0.f; // -1..1

    // One instance of each filter category
    SVFilter svf;
    LadderFilter ladder;
    CombFilter comb;
    Resonator resonator;
    PhaserFilter phaser;
    FormantFilter formant;
    VocalFilter vocal;
    EQFilter eq;
    FMFilter fm;
    AMFilter am;
    SpecialFilter special;
};
