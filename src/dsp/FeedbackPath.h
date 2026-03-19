#pragma once
#include <vector>
#include <array>
#include <cmath>
#include <algorithm>

// Feedback path: takes output signal, delays it, feeds back to filter input
// Includes safety limiter to prevent runaway oscillation
class FeedbackPath
{
public:
    void prepare(double sampleRate, int blockSize);
    void setAmount(float a);     // 0..1
    void setDelayMs(float ms);   // 0..500ms
    void reset();

    // Get feedback samples to add to input, then store new output
    void processStereo(float* left, float* right, int numSamples);

    // Get current feedback to mix into input before processing
    float getFeedbackSample(int channel) const;

    // Store output sample for next feedback cycle
    void pushOutputSample(int channel, float sample);

private:
    double sr = 44100.0;
    float amount = 0.f;
    float delaySamples = 0.f;

    static constexpr int kMaxDelay = 48000; // 500ms at 96kHz

    struct DelayLine
    {
        std::vector<float> buffer;
        int writePos = 0;
        int readPos = 0;
    };
    std::array<DelayLine, 2> delayLines;
};
