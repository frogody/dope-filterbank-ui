#pragma once
#include "FilterEngine.h"

class FilterRouter
{
public:
    enum class Mode { Serial, Parallel, StereoSplit, Subtractive };

    void prepare(double sampleRate, int blockSize);
    void setMode(Mode m) { mode = m; }
    Mode getMode() const { return mode; }

    FilterEngine& getFilter1() { return filter1; }
    FilterEngine& getFilter2() { return filter2; }

    void processStereo(float* left, float* right, int numSamples);

private:
    Mode mode = Mode::Serial;
    FilterEngine filter1;
    FilterEngine filter2;

    // Temp buffers for parallel/subtractive processing
    static constexpr int kMaxBlockSize = 4096;
    float tempL[kMaxBlockSize] = {};
    float tempR[kMaxBlockSize] = {};
};
