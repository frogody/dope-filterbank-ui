#include "FilterDisplay.h"
#include <cmath>

FilterDisplay::FilterDisplay()
{
    startTimerHz(60);
}

void FilterDisplay::setFilter1Params(int type, float cutoff, float reso)
{
    f1Type = type;
    f1CutoffTarget = cutoff;
    f1ResoTarget = reso;
}

void FilterDisplay::setFilter2Params(int type, float cutoff, float reso)
{
    f2Type = type;
    f2CutoffTarget = cutoff;
    f2ResoTarget = reso;
}

void FilterDisplay::timerCallback()
{
    const float smoothing = 0.35f;
    f1Cutoff += (f1CutoffTarget - f1Cutoff) * smoothing;
    f1Reso  += (f1ResoTarget - f1Reso) * smoothing;
    f2Cutoff += (f2CutoffTarget - f2Cutoff) * smoothing;
    f2Reso  += (f2ResoTarget - f2Reso) * smoothing;

    calcMagnitudeResponse(f1Cutoff, f1Reso, f1Type, f1Mags);
    calcMagnitudeResponse(f2Cutoff, f2Reso, f2Type, f2Mags);

    for (int i = 0; i < kNumPoints; ++i)
        combinedMags[i] = f1Mags[i] + f2Mags[i];

    repaint();
}

float FilterDisplay::freqToX(float freq) const
{
    float minLog = std::log10(20.f);
    float maxLog = std::log10(20000.f);
    return (std::log10(std::max(freq, 20.f)) - minLog) / (maxLog - minLog) * getWidth();
}

float FilterDisplay::dbToY(float db) const
{
    return (18.f - db) / 60.f * getHeight();
}

float FilterDisplay::xToFreq(float x) const
{
    float minLog = std::log10(20.f);
    float maxLog = std::log10(20000.f);
    return std::pow(10.f, minLog + (maxLog - minLog) * x / getWidth());
}

void FilterDisplay::calcMagnitudeResponse(float cutoff, float reso, int filterType,
                                            std::array<float, kNumPoints>& magnitudes)
{
    enum class Shape { LP, HP, BP, Notch, AP, Other };
    Shape shape = Shape::LP;
    int poles = 2;

    if (filterType <= 5)       { shape = Shape::LP; poles = (filterType + 1); }
    else if (filterType <= 11) { shape = Shape::HP; poles = (filterType - 5); }
    else if (filterType <= 15) { shape = Shape::BP; poles = 2; }
    else if (filterType <= 18) { shape = Shape::Notch; poles = 2; }
    else if (filterType <= 21) { shape = Shape::AP; poles = 2; }
    else if (filterType <= 24) { shape = Shape::LP; poles = 4; }
    else                       { shape = Shape::Other; poles = 2; }

    int stages = std::max(1, std::min(poles, 8));
    float Q = 0.707f + reso * 15.f;

    for (int i = 0; i < kNumPoints; ++i)
    {
        float freq = xToFreq(static_cast<float>(i) / (kNumPoints - 1) * getWidth());
        double ratio = freq / static_cast<double>(cutoff);
        double r2 = ratio * ratio;
        double qInv = 1.0 / Q;
        double magSq = 1.0;

        for (int s = 0; s < (stages + 1) / 2; ++s)
        {
            double denom = (1.0 - r2) * (1.0 - r2) + (ratio * qInv) * (ratio * qInv);
            if (denom < 1e-10) denom = 1e-10;

            switch (shape)
            {
                case Shape::LP:    magSq *= 1.0 / denom; break;
                case Shape::HP:    magSq *= (r2 * r2) / denom; break;
                case Shape::BP:    magSq *= (ratio * qInv) * (ratio * qInv) / denom; break;
                case Shape::Notch: magSq *= ((1.0 - r2) * (1.0 - r2)) / denom; break;
                case Shape::AP:    magSq *= 1.0; break;
                case Shape::Other: magSq *= 1.0 / denom; break;
            }
        }

        magnitudes[i] = std::clamp(10.f * std::log10(static_cast<float>(std::max(magSq, 1e-10))),
                                    -42.f, 18.f);
    }
}

void FilterDisplay::drawCurve(juce::Graphics& g,
                                const std::array<float, kNumPoints>& mags,
                                juce::Colour colour, float strokeWidth, float glowWidth)
{
    int W = getWidth();
    int H = getHeight();
    if (W < 10 || H < 10) return;

    juce::Path curvePath;
    bool started = false;

    for (int i = 0; i < kNumPoints; ++i)
    {
        float xp = static_cast<float>(i) / (kNumPoints - 1) * W;
        float yp = dbToY(std::clamp(mags[i], -42.f, 18.f));
        if (!started) { curvePath.startNewSubPath(xp, yp); started = true; }
        else curvePath.lineTo(xp, yp);
    }

    // Outer bloom
    g.setColour(colour.withAlpha(0.04f));
    g.strokePath(curvePath, juce::PathStrokeType(glowWidth * 2.f,
        juce::PathStrokeType::curved, juce::PathStrokeType::rounded));

    // Mid glow
    g.setColour(colour.withAlpha(0.10f));
    g.strokePath(curvePath, juce::PathStrokeType(glowWidth,
        juce::PathStrokeType::curved, juce::PathStrokeType::rounded));

    // Core line
    g.setColour(colour.withAlpha(0.85f));
    g.strokePath(curvePath, juce::PathStrokeType(strokeWidth,
        juce::PathStrokeType::curved, juce::PathStrokeType::rounded));

    // White-hot center
    g.setColour(juce::Colours::white.withAlpha(0.25f));
    g.strokePath(curvePath, juce::PathStrokeType(strokeWidth * 0.4f,
        juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
}

void FilterDisplay::paint(juce::Graphics& g)
{
    auto bounds = getLocalBounds().toFloat();
    int W = getWidth();
    int H = getHeight();

    // Dark background matching X-ray theme
    juce::ColourGradient bgGrad(
        juce::Colour(0xff0c1220), 0.f, 0.f,
        juce::Colour(0xff080c16), 0.f, bounds.getHeight(), false);
    g.setGradientFill(bgGrad);
    g.fillRoundedRectangle(bounds, 6.f);

    // Subtle blue radial glow from center
    {
        float cx = bounds.getCentreX(), cy = bounds.getCentreY();
        juce::ColourGradient glow(
            juce::Colour(0x0800bbff), cx, cy,
            juce::Colour(0x00000000), cx + W * 0.4f, cy, true);
        g.setGradientFill(glow);
        g.fillRoundedRectangle(bounds, 6.f);
    }

    // Scan lines
    g.setColour(juce::Colour(0x0400bbff));
    for (int sy = 0; sy < H; sy += 2)
        g.drawHorizontalLine(sy, 0.f, static_cast<float>(W));

    // Blue border
    g.setColour(juce::Colour(0xff00bbff).withAlpha(0.15f));
    g.drawRoundedRectangle(bounds.reduced(0.5f), 6.f, 1.f);

    if (W < 10 || H < 10) return;

    // Grid — vertical freq lines (blue tinted)
    float gridFreqs[] = {50.f, 100.f, 200.f, 500.f, 1000.f, 2000.f, 5000.f, 10000.f};
    for (auto f : gridFreqs)
    {
        float xp = freqToX(f);
        g.setColour(juce::Colour(0x0c00bbff));
        for (int sy = 0; sy < H; sy += 6)
            g.drawVerticalLine(static_cast<int>(xp), (float)sy, (float)(sy + 3));

        // Blue dot at top
        g.setColour(juce::Colour(0x3000bbff));
        g.fillEllipse(xp - 2.f, 3.f, 4.f, 4.f);
        g.setColour(juce::Colour(0x8000ddff));
        g.fillEllipse(xp - 1.f, 4.f, 2.f, 2.f);
    }

    // Horizontal dB lines
    float gridDbs[] = {12.f, 6.f, 0.f, -6.f, -12.f, -24.f, -36.f};
    for (auto db : gridDbs)
    {
        float yp = dbToY(db);
        if (db == 0.f)
        {
            g.setColour(juce::Colour(0x2000bbff));
            for (int sx = 0; sx < W; sx += 8)
                g.drawHorizontalLine(static_cast<int>(yp), (float)sx, (float)(sx + 4));
        }
        else
        {
            g.setColour(juce::Colour(0x08ffffff));
            g.drawHorizontalLine(static_cast<int>(yp), 0.f, static_cast<float>(W));
        }
    }

    // Freq labels
    g.setColour(juce::Colour(0x5000bbff));
    g.setFont(8.f);
    auto drawFreqLabel = [&](float freq, const char* text)
    {
        g.drawText(text, static_cast<int>(freqToX(freq)) - 12, H - 13, 24, 10,
                   juce::Justification::centred);
    };
    drawFreqLabel(50.f, "50");
    drawFreqLabel(100.f, "100");
    drawFreqLabel(500.f, "500");
    drawFreqLabel(1000.f, "1k");
    drawFreqLabel(5000.f, "5k");
    drawFreqLabel(10000.f, "10k");

    // dB labels
    g.setColour(juce::Colour(0x4000bbff));
    g.setFont(7.f);
    for (auto db : {12.f, 0.f, -12.f, -24.f})
    {
        juce::String txt = (db > 0 ? "+" : "") + juce::String((int)db);
        g.drawText(txt, 2, static_cast<int>(dbToY(db)) - 5, 22, 10,
                   juce::Justification::left);
    }

    // --- Combined curve (blue glow) ---
    {
        juce::Path curvePath;
        bool started = false;

        for (int i = 0; i < kNumPoints; ++i)
        {
            float xp = static_cast<float>(i) / (kNumPoints - 1) * W;
            float yp = dbToY(std::clamp(combinedMags[i], -42.f, 18.f));
            if (!started) { curvePath.startNewSubPath(xp, yp); started = true; }
            else curvePath.lineTo(xp, yp);
        }

        // Gradient fill under curve (blue)
        juce::Path fillPath(curvePath);
        fillPath.lineTo(static_cast<float>(W), static_cast<float>(H));
        fillPath.lineTo(0.f, static_cast<float>(H));
        fillPath.closeSubPath();

        juce::ColourGradient fillGrad(
            juce::Colour(0x2000bbff), 0.f, dbToY(6.f),
            juce::Colour(0x0200bbff), 0.f, static_cast<float>(H), false);
        g.setGradientFill(fillGrad);
        g.fillPath(fillPath);

        // Draw combined with blue glow
        drawCurve(g, combinedMags, juce::Colour(0xff00bbff), 2.5f, 10.f);
    }
}
