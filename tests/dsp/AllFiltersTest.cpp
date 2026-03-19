#include <catch2/catch_test_macros.hpp>
#include "dsp/LadderFilter.h"
#include "dsp/CombFilter.h"
#include "dsp/Resonator.h"
#include "dsp/PhaserFilter.h"
#include "dsp/FormantFilter.h"
#include "dsp/VocalFilter.h"
#include "dsp/EQFilter.h"
#include "dsp/FMFilter.h"
#include "dsp/AMFilter.h"
#include "dsp/SpecialFilter.h"
#include <cmath>
#include <vector>

static float measureRms(const std::vector<float>& buf, int start = 0)
{
    float sum = 0.f;
    int count = static_cast<int>(buf.size()) - start;
    for (int i = start; i < static_cast<int>(buf.size()); ++i)
        sum += buf[i] * buf[i];
    return std::sqrt(sum / count);
}

static std::vector<float> makeSine(float freq, double sr, int n)
{
    std::vector<float> buf(n);
    for (int i = 0; i < n; ++i)
        buf[i] = std::sin(2.0 * M_PI * freq * i / sr);
    return buf;
}

// --- Ladder Filter ---

TEST_CASE("LadderFilter LP attenuates above cutoff", "[ladder]")
{
    LadderFilter f;
    f.setType(LadderFilter::Type::LP);
    f.prepare(44100.0, 512);
    f.setCutoff(1000.f);
    f.setResonance(0.f);

    auto buf = makeSine(10000.f, 44100.0, 8192);
    float inRms = measureRms(buf);
    for (int i = 0; i < 8192; ++i)
        buf[i] = f.processSample(0, buf[i]);
    float outRms = measureRms(buf, 1024);

    REQUIRE(20.f * std::log10(outRms / inRms) < -20.f);
}

TEST_CASE("LadderFilter self-oscillates at high resonance", "[ladder]")
{
    LadderFilter f;
    f.setType(LadderFilter::Type::LP);
    f.prepare(44100.0, 512);
    f.setCutoff(440.f);
    f.setResonance(0.99f);

    // Feed impulse then silence
    f.processSample(0, 1.f);
    float maxVal = 0.f;
    for (int i = 0; i < 4096; ++i)
    {
        float out = f.processSample(0, 0.f);
        maxVal = std::max(maxVal, std::abs(out));
    }
    // Should still be ringing (self-oscillation)
    REQUIRE(maxVal > 0.001f);
}

// --- Comb Filter ---

TEST_CASE("CombFilter positive creates resonance", "[comb]")
{
    CombFilter f;
    f.setType(CombFilter::Type::Positive);
    f.prepare(44100.0, 512);
    f.setCutoff(440.f);
    f.setResonance(0.8f);

    // Feed impulse
    float out = f.processSample(0, 1.f);
    REQUIRE(out != 0.f);

    // Should still have output after delay — check max over a range
    int delaySamples = static_cast<int>(44100.0 / 440.0);
    float maxOut = 0.f;
    for (int i = 0; i < delaySamples * 2; ++i)
    {
        out = f.processSample(0, 0.f);
        maxOut = std::max(maxOut, std::abs(out));
    }
    REQUIRE(maxOut > 0.01f);
}

// --- Resonator ---

TEST_CASE("Resonator passes at center frequency", "[resonator]")
{
    Resonator r;
    r.setType(Resonator::Type::Mono);
    r.prepare(44100.0, 512);
    r.setCutoff(1000.f);
    r.setResonance(0.5f);

    auto buf = makeSine(1000.f, 44100.0, 8192);
    float inRms = measureRms(buf);
    for (int i = 0; i < 8192; ++i)
        buf[i] = r.processSample(0, buf[i]);
    float outRms = measureRms(buf, 1024);

    float atten = 20.f * std::log10(outRms / inRms);
    REQUIRE(atten > -6.f);
}

// --- Phaser ---

TEST_CASE("PhaserFilter modifies signal", "[phaser]")
{
    PhaserFilter p;
    p.setType(PhaserFilter::Type::Stage4);
    p.prepare(44100.0, 512);
    p.setCutoff(1000.f);
    p.setResonance(0.5f);

    auto buf = makeSine(1000.f, 44100.0, 4096);
    float inRms = measureRms(buf);
    for (int i = 0; i < 4096; ++i)
        buf[i] = p.processSample(0, buf[i]);
    float outRms = measureRms(buf, 512);

    // Phaser should change the signal (not passthrough or silence)
    float ratio = outRms / inRms;
    REQUIRE(ratio > 0.01f);
    REQUIRE(ratio < 2.f);
}

// --- Formant ---

TEST_CASE("FormantFilter vowel A passes signal", "[formant]")
{
    FormantFilter f;
    f.setType(FormantFilter::Type::VowelA);
    f.prepare(44100.0, 512);
    f.setCutoff(1000.f);
    f.setResonance(0.5f);

    auto buf = makeSine(800.f, 44100.0, 4096);
    for (int i = 0; i < 4096; ++i)
        buf[i] = f.processSample(0, buf[i]);
    float outRms = measureRms(buf, 512);

    REQUIRE(outRms > 0.01f);
}

TEST_CASE("FormantFilter morph transitions smoothly", "[formant]")
{
    FormantFilter f;
    f.setType(FormantFilter::Type::Morph);
    f.prepare(44100.0, 512);
    f.setCutoff(1000.f);
    f.setResonance(0.5f);

    // Process at morph=0 and morph=1, should produce different output
    auto buf1 = makeSine(500.f, 44100.0, 4096);
    auto buf2 = buf1;

    f.setMorph(0.f);
    for (int i = 0; i < 4096; ++i) buf1[i] = f.processSample(0, buf1[i]);

    f.reset();
    f.setMorph(1.f);
    for (int i = 0; i < 4096; ++i) buf2[i] = f.processSample(0, buf2[i]);

    float rms1 = measureRms(buf1, 512);
    float rms2 = measureRms(buf2, 512);

    // Different morph values should produce different amplitudes
    REQUIRE(std::abs(rms1 - rms2) > 0.001f);
}

// --- EQ ---

TEST_CASE("EQFilter peak boosts at cutoff", "[eq]")
{
    EQFilter f;
    f.setType(EQFilter::Type::Peak);
    f.prepare(44100.0, 512);
    f.setCutoff(1000.f);
    f.setResonance(0.9f); // boost

    auto buf = makeSine(1000.f, 44100.0, 8192);
    float inRms = measureRms(buf);
    for (int i = 0; i < 8192; ++i) buf[i] = f.processSample(0, buf[i]);
    float outRms = measureRms(buf, 1024);

    REQUIRE(outRms > inRms);
}

// --- AM ---

TEST_CASE("AMFilter ring mod produces sidebands", "[am]")
{
    AMFilter f;
    f.setType(AMFilter::Type::Ring);
    f.prepare(44100.0, 512);
    f.setCutoff(440.f);
    f.setResonance(1.f);

    auto buf = makeSine(1000.f, 44100.0, 4096);
    for (int i = 0; i < 4096; ++i) buf[i] = f.processSample(0, buf[i]);
    float outRms = measureRms(buf, 512);

    REQUIRE(outRms > 0.01f);
}

// --- Stability: all filter types should not explode ---

TEST_CASE("All filter types are stable with impulse", "[stability]")
{
    auto testStability = [](auto& filter) {
        float maxVal = 0.f;
        filter.processSample(0, 1.f);
        for (int i = 0; i < 44100; ++i)
        {
            float out = filter.processSample(0, 0.f);
            maxVal = std::max(maxVal, std::abs(out));
        }
        REQUIRE(maxVal < 100.f);
    };

    SECTION("Ladder LP")
    {
        LadderFilter f;
        f.setType(LadderFilter::Type::LP);
        f.prepare(44100.0, 512);
        f.setCutoff(1000.f);
        f.setResonance(0.5f);
        testStability(f);
    }

    SECTION("Comb Positive")
    {
        CombFilter f;
        f.setType(CombFilter::Type::Positive);
        f.prepare(44100.0, 512);
        f.setCutoff(440.f);
        f.setResonance(0.8f);
        testStability(f);
    }

    SECTION("Resonator")
    {
        Resonator f;
        f.setType(Resonator::Type::Mono);
        f.prepare(44100.0, 512);
        f.setCutoff(1000.f);
        f.setResonance(0.8f);
        testStability(f);
    }

    SECTION("Phaser")
    {
        PhaserFilter f;
        f.setType(PhaserFilter::Type::Stage8);
        f.prepare(44100.0, 512);
        f.setCutoff(1000.f);
        f.setResonance(0.8f);
        testStability(f);
    }

    SECTION("EQ Peak")
    {
        EQFilter f;
        f.setType(EQFilter::Type::Peak);
        f.prepare(44100.0, 512);
        f.setCutoff(1000.f);
        f.setResonance(0.9f);
        testStability(f);
    }
}
