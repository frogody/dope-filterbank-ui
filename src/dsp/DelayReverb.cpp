#include "DelayReverb.h"

const char* DelayReverb::getTypeName(int index)
{
    static const char* names[] = {
        "Simple Delay", "Ping Pong", "Stereo Delay",
        "Plate Reverb", "Room Reverb"
    };
    if (index < 0 || index >= kNumTypes) return "Unknown";
    return names[index];
}

void DelayReverb::prepare(double sampleRate, int /*blockSize*/)
{
    sr = sampleRate;
    for (auto& dl : delayLines)
        dl.init(kMaxDelaySamples);

    // Init reverb comb/allpass delays
    // Plate reverb: longer times, Room: shorter
    const int combLengths[4] = {1116, 1188, 1277, 1356};
    const int apLengths[2] = {225, 556};

    for (int ch = 0; ch < 2; ++ch)
    {
        for (int i = 0; i < 4; ++i)
        {
            int len = static_cast<int>(combLengths[i] * sr / 44100.0);
            // Slight stereo offset for right channel
            if (ch == 1) len += 23;
            reverb[ch].combs[i].init(len + 1);
            reverb[ch].combFb[i] = 0.84f + i * 0.01f;
        }
        for (int i = 0; i < 2; ++i)
        {
            int len = static_cast<int>(apLengths[i] * sr / 44100.0);
            if (ch == 1) len += 13;
            reverb[ch].allpass[i].init(len + 1);
            reverb[ch].apFb[i] = 0.5f;
        }
    }
    reset();
}

void DelayReverb::setTime(float ms)
{
    delayMs = std::clamp(ms, 1.f, 2000.f);
}

void DelayReverb::setFeedback(float fb)
{
    feedback = std::clamp(fb, 0.f, 0.95f);
}

void DelayReverb::setMix(float m)
{
    mix = std::clamp(m, 0.f, 1.f);
}

void DelayReverb::setBPMSync(bool sync, double newBpm, int div)
{
    bpmSync = sync;
    bpm = newBpm;
    division = div;
}

void DelayReverb::reset()
{
    for (auto& dl : delayLines) dl.clear();
    for (auto& ch : reverb)
    {
        for (auto& c : ch.combs) c.clear();
        for (auto& a : ch.allpass) a.clear();
    }
}

float DelayReverb::getDelaySamples() const
{
    if (bpmSync && bpm > 0)
    {
        double beatMs = 60000.0 / bpm;
        double noteMs = beatMs * 4.0 / division;
        return static_cast<float>(noteMs * sr / 1000.0);
    }
    return delayMs * static_cast<float>(sr) / 1000.f;
}

void DelayReverb::processDelay(float* left, float* right, int numSamples, bool pingPong)
{
    float delaySamp = std::min(getDelaySamples(), static_cast<float>(kMaxDelaySamples - 1));

    for (int i = 0; i < numSamples; ++i)
    {
        float dryL = left[i], dryR = right[i];

        float delL = delayLines[0].read(delaySamp);
        float delR = delayLines[1].read(delaySamp);

        if (pingPong)
        {
            // Ping pong: L feeds R delay, R feeds L delay
            delayLines[0].write(dryL + delR * feedback);
            delayLines[1].write(dryR + delL * feedback);
        }
        else
        {
            delayLines[0].write(dryL + delL * feedback);
            delayLines[1].write(dryR + delR * feedback);
        }

        left[i]  = dryL * (1.f - mix) + delL * mix;
        right[i] = dryR * (1.f - mix) + delR * mix;
    }
}

void DelayReverb::processReverb(float* left, float* right, int numSamples, bool plate)
{
    // Adjust feedback for plate (longer tail) vs room (shorter)
    float fbScale = plate ? 1.0f : 0.8f;

    for (int i = 0; i < numSamples; ++i)
    {
        float dryL = left[i], dryR = right[i];
        float wetL = 0.f, wetR = 0.f;

        for (int ch = 0; ch < 2; ++ch)
        {
            float input = (ch == 0) ? dryL : dryR;
            float combOut = 0.f;

            // Parallel comb filters
            for (int c = 0; c < 4; ++c)
            {
                float combLen = static_cast<float>(reverb[ch].combs[c].buffer.size() - 1);
                // Scale comb length by time parameter
                float scaledLen = combLen * (delayMs / 500.f);
                scaledLen = std::clamp(scaledLen, 1.f, combLen);

                float del = reverb[ch].combs[c].read(scaledLen);
                reverb[ch].combs[c].write(input + del * reverb[ch].combFb[c] * feedback * fbScale);
                combOut += del;
            }
            combOut *= 0.25f;

            // Series allpass filters
            for (int a = 0; a < 2; ++a)
            {
                float apLen = static_cast<float>(reverb[ch].allpass[a].buffer.size() - 1);
                float del = reverb[ch].allpass[a].read(apLen);
                float apOut = -combOut + del;
                reverb[ch].allpass[a].write(combOut + del * reverb[ch].apFb[a]);
                combOut = apOut;
            }

            if (ch == 0) wetL = combOut;
            else wetR = combOut;
        }

        left[i]  = dryL * (1.f - mix) + wetL * mix;
        right[i] = dryR * (1.f - mix) + wetR * mix;
    }
}

void DelayReverb::processStereo(float* left, float* right, int numSamples)
{
    switch (type)
    {
        case Type::SimpleDelay:
            processDelay(left, right, numSamples, false);
            break;
        case Type::PingPong:
            processDelay(left, right, numSamples, true);
            break;
        case Type::StereoDelay:
        {
            // Stereo delay with slightly different L/R times
            float delaySamp = std::min(getDelaySamples(), static_cast<float>(kMaxDelaySamples - 1));
            float delayR = delaySamp * 0.75f; // Right is 3/4 of left time

            for (int i = 0; i < numSamples; ++i)
            {
                float dryL = left[i], dryR = right[i];
                float delL = delayLines[0].read(delaySamp);
                float delR = delayLines[1].read(delayR);
                delayLines[0].write(dryL + delL * feedback);
                delayLines[1].write(dryR + delR * feedback);
                left[i]  = dryL * (1.f - mix) + delL * mix;
                right[i] = dryR * (1.f - mix) + delR * mix;
            }
            break;
        }
        case Type::PlateReverb:
            processReverb(left, right, numSamples, true);
            break;
        case Type::RoomReverb:
            processReverb(left, right, numSamples, false);
            break;
    }
}
