#pragma once
#include <juce_gui_basics/juce_gui_basics.h>
#include <array>

class FilterDisplay : public juce::Component, public juce::Timer
{
public:
    FilterDisplay();

    void paint(juce::Graphics& g) override;
    void timerCallback() override;

    void setFilter1Params(int type, float cutoff, float reso);
    void setFilter2Params(int type, float cutoff, float reso);

private:
    static constexpr int kNumPoints = 256;

    void calcMagnitudeResponse(float cutoff, float reso, int filterType,
                                std::array<float, kNumPoints>& magnitudes);
    void drawCurve(juce::Graphics& g, const std::array<float, kNumPoints>& mags,
                   juce::Colour colour, float strokeWidth, float glowWidth);
    float freqToX(float freq) const;
    float dbToY(float db) const;
    float xToFreq(float x) const;

    // Smoothed params (interpolated for animation)
    float f1Cutoff = 1000.f, f1Reso = 0.f;
    float f2Cutoff = 5000.f, f2Reso = 0.f;
    int f1Type = 1, f2Type = 7;

    // Target params (set by user)
    float f1CutoffTarget = 1000.f, f1ResoTarget = 0.f;
    float f2CutoffTarget = 5000.f, f2ResoTarget = 0.f;

    // Cached magnitude arrays
    std::array<float, kNumPoints> f1Mags{}, f2Mags{}, combinedMags{};
};
