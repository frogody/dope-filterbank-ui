#include "FilterRouter.h"
#include <algorithm>
#include <cstring>

void FilterRouter::prepare(double sampleRate, int blockSize)
{
    filter1.prepare(sampleRate, blockSize);
    filter2.prepare(sampleRate, blockSize);
}

void FilterRouter::processStereo(float* left, float* right, int numSamples)
{
    int n = std::min(numSamples, kMaxBlockSize);

    switch (mode)
    {
        case Mode::Serial:
        {
            // Input → Filter1 → Filter2 → Output
            filter1.processStereo(left, right, n);
            filter2.processStereo(left, right, n);
            break;
        }

        case Mode::Parallel:
        {
            // Output = (Filter1(input) + Filter2(input)) * 0.5
            std::memcpy(tempL, left, n * sizeof(float));
            std::memcpy(tempR, right, n * sizeof(float));

            filter1.processStereo(left, right, n);
            filter2.processStereo(tempL, tempR, n);

            for (int i = 0; i < n; ++i)
            {
                left[i]  = (left[i] + tempL[i]) * 0.5f;
                right[i] = (right[i] + tempR[i]) * 0.5f;
            }
            break;
        }

        case Mode::StereoSplit:
        {
            // Filter1 → Left, Filter2 → Right
            std::memcpy(tempL, left, n * sizeof(float));
            std::memcpy(tempR, right, n * sizeof(float));

            // Filter1 processes left channel (mono from left input)
            filter1.processStereo(left, tempL, n);
            // Filter2 processes right channel (mono from right input)
            filter2.processStereo(tempR, right, n);

            // Take left from filter1, right from filter2
            // left is already filter1 left output
            // right is already filter2 right output
            break;
        }

        case Mode::Subtractive:
        {
            // Output = Filter1(input) - Filter2(input)
            std::memcpy(tempL, left, n * sizeof(float));
            std::memcpy(tempR, right, n * sizeof(float));

            filter1.processStereo(left, right, n);
            filter2.processStereo(tempL, tempR, n);

            for (int i = 0; i < n; ++i)
            {
                left[i]  -= tempL[i];
                right[i] -= tempR[i];
            }
            break;
        }
    }
}
