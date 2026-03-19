#pragma once
#include <vector>
#include <array>
#include <cmath>
#include <algorithm>

class DelayReverb
{
public:
    enum class Type
    {
        SimpleDelay, PingPong, StereoDelay, PlateReverb, RoomReverb
    };

    static constexpr int kNumTypes = 5;

    void prepare(double sampleRate, int blockSize);
    void setType(Type t) { type = t; }
    void setTime(float ms);       // 0..2000ms
    void setFeedback(float fb);   // 0..0.95
    void setMix(float m);         // 0..1
    void setBPMSync(bool sync, double bpm = 120.0, int division = 4);
    void reset();

    void processStereo(float* left, float* right, int numSamples);

    static const char* getTypeName(int index);

private:
    Type type = Type::SimpleDelay;
    double sr = 44100.0;
    float delayMs = 250.f;
    float feedback = 0.3f;
    float mix = 0.5f;
    bool bpmSync = false;
    double bpm = 120.0;
    int division = 4; // 1=whole, 2=half, 4=quarter, 8=eighth, 16=sixteenth

    static constexpr int kMaxDelaySamples = 192000; // ~2s at 96kHz

    struct DelayLine
    {
        std::vector<float> buffer;
        int writePos = 0;

        void init(int maxSize)
        {
            buffer.resize(maxSize, 0.f);
            writePos = 0;
        }

        void clear()
        {
            std::fill(buffer.begin(), buffer.end(), 0.f);
            writePos = 0;
        }

        void write(float sample)
        {
            buffer[writePos] = sample;
            writePos = (writePos + 1) % static_cast<int>(buffer.size());
        }

        float read(float delaySamples) const
        {
            float readPos = static_cast<float>(writePos) - delaySamples;
            while (readPos < 0.f) readPos += static_cast<float>(buffer.size());
            int idx1 = static_cast<int>(readPos) % static_cast<int>(buffer.size());
            int idx2 = (idx1 + 1) % static_cast<int>(buffer.size());
            float frac = readPos - std::floor(readPos);
            return buffer[idx1] * (1.f - frac) + buffer[idx2] * frac;
        }
    };

    std::array<DelayLine, 2> delayLines;

    // Reverb: 4 comb filters + 2 allpass per channel
    struct ReverbState
    {
        std::array<DelayLine, 4> combs;
        std::array<DelayLine, 2> allpass;
        std::array<float, 4> combFb;
        std::array<float, 2> apFb;
    };
    std::array<ReverbState, 2> reverb;

    float getDelaySamples() const;
    void processDelay(float* left, float* right, int numSamples, bool pingPong);
    void processReverb(float* left, float* right, int numSamples, bool plate);
};
