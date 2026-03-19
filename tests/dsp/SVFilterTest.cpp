#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>
#include "dsp/SVFilter.h"
#include <cmath>
#include <vector>

static float measureRms(const std::vector<float>& buffer, int startSample = 0)
{
    float sum = 0.f;
    int count = static_cast<int>(buffer.size()) - startSample;
    for (int i = startSample; i < static_cast<int>(buffer.size()); ++i)
        sum += buffer[i] * buffer[i];
    return std::sqrt(sum / count);
}

static std::vector<float> generateSine(float freqHz, double sampleRate, int numSamples)
{
    std::vector<float> buf(numSamples);
    for (int i = 0; i < numSamples; ++i)
        buf[i] = std::sin(2.0 * M_PI * freqHz * i / sampleRate);
    return buf;
}

static float measureAttenuation(SVFilter::Type type, float cutoffHz, float testFreqHz)
{
    SVFilter filter;
    filter.setType(type);
    filter.prepare(44100.0, 512);
    filter.setCutoff(cutoffHz);
    filter.setResonance(0.f);

    const int numSamples = 8192;
    auto buffer = generateSine(testFreqHz, 44100.0, numSamples);
    float inputRms = measureRms(buffer);

    for (int i = 0; i < numSamples; ++i)
        buffer[i] = filter.processSample(0, buffer[i]);

    float outputRms = measureRms(buffer, 1024);
    return 20.f * std::log10(outputRms / inputRms);
}

// --- Lowpass Tests ---

TEST_CASE("SVFilter LP12 attenuates above cutoff", "[svf][lp]")
{
    float atten = measureAttenuation(SVFilter::Type::LP12, 1000.f, 10000.f);
    REQUIRE(atten < -20.f);
}

TEST_CASE("SVFilter LP12 passes signal below cutoff", "[svf][lp]")
{
    float atten = measureAttenuation(SVFilter::Type::LP12, 10000.f, 100.f);
    REQUIRE(atten > -1.f);
}

TEST_CASE("SVFilter LP24 attenuates more steeply than LP12", "[svf][lp]")
{
    float atten12 = measureAttenuation(SVFilter::Type::LP12, 1000.f, 8000.f);
    float atten24 = measureAttenuation(SVFilter::Type::LP24, 1000.f, 8000.f);
    REQUIRE(atten24 < atten12 - 10.f);
}

TEST_CASE("SVFilter LP6 provides gentle slope", "[svf][lp]")
{
    float atten = measureAttenuation(SVFilter::Type::LP6, 1000.f, 10000.f);
    // 6dB/oct at 10x frequency = ~20dB attenuation, allow margin
    REQUIRE(atten < -8.f);
    REQUIRE(atten > -30.f);
}

// --- Highpass Tests ---

TEST_CASE("SVFilter HP12 attenuates below cutoff", "[svf][hp]")
{
    float atten = measureAttenuation(SVFilter::Type::HP12, 5000.f, 200.f);
    REQUIRE(atten < -20.f);
}

TEST_CASE("SVFilter HP12 passes signal above cutoff", "[svf][hp]")
{
    float atten = measureAttenuation(SVFilter::Type::HP12, 200.f, 5000.f);
    REQUIRE(atten > -1.f);
}

// --- Bandpass Tests ---

TEST_CASE("SVFilter BP12 passes at cutoff frequency", "[svf][bp]")
{
    float atten = measureAttenuation(SVFilter::Type::BP12, 1000.f, 1000.f);
    REQUIRE(atten > -7.f);
}

TEST_CASE("SVFilter BP12 attenuates far from cutoff", "[svf][bp]")
{
    float attenLow = measureAttenuation(SVFilter::Type::BP12, 1000.f, 50.f);
    float attenHigh = measureAttenuation(SVFilter::Type::BP12, 1000.f, 15000.f);
    REQUIRE(attenLow < -10.f);
    REQUIRE(attenHigh < -10.f);
}

// --- Notch Tests ---

TEST_CASE("SVFilter Notch12 attenuates at cutoff", "[svf][notch]")
{
    float attenAtCutoff = measureAttenuation(SVFilter::Type::Notch12, 1000.f, 1000.f);
    float attenAway = measureAttenuation(SVFilter::Type::Notch12, 1000.f, 100.f);
    REQUIRE(attenAtCutoff < attenAway - 5.f);
}

// --- Stability Tests ---

TEST_CASE("SVFilter does not explode at extreme settings", "[svf][stability]")
{
    SVFilter filter;
    filter.setType(SVFilter::Type::LP12);
    filter.prepare(44100.0, 512);
    filter.setCutoff(20.f);
    filter.setResonance(1.f);

    float maxVal = 0.f;
    for (int i = 0; i < 44100; ++i)
    {
        float input = (i == 0) ? 1.f : 0.f; // impulse
        float output = filter.processSample(0, input);
        maxVal = std::max(maxVal, std::abs(output));
    }
    // Self-oscillation should ring but not grow unbounded
    REQUIRE(maxVal < 100.f);
}

TEST_CASE("SVFilter reset clears state", "[svf]")
{
    SVFilter filter;
    filter.setType(SVFilter::Type::LP12);
    filter.prepare(44100.0, 512);
    filter.setCutoff(1000.f);

    // Process some audio
    for (int i = 0; i < 1000; ++i)
        filter.processSample(0, 0.5f);

    filter.reset();

    // After reset, processing silence should give silence
    float output = filter.processSample(0, 0.f);
    REQUIRE(output == 0.f);
}

TEST_CASE("SVFilter stereo channels are independent", "[svf]")
{
    SVFilter filter;
    filter.setType(SVFilter::Type::LP12);
    filter.prepare(44100.0, 512);
    filter.setCutoff(1000.f);

    // Process only channel 0
    for (int i = 0; i < 100; ++i)
        filter.processSample(0, 0.5f);

    // Channel 1 should still output silence
    float output = filter.processSample(1, 0.f);
    REQUIRE(output == 0.f);
}
