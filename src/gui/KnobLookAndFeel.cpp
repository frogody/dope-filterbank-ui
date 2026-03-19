#include "KnobLookAndFeel.h"
#include "Theme.h"
#include <melatonin_blur/melatonin_blur.h>

KnobLookAndFeel::KnobLookAndFeel()
{
    accentColour = juce::Colour(Theme::kCyanBright);

    knobShadow.setRadius(12);
    knobShadow.setOffset({1, 5});
    knobShadow.setColor(juce::Colour(0x60000000));

    heroShadow.setRadius(20);
    heroShadow.setOffset({2, 8});
    heroShadow.setColor(juce::Colour(0x60000000));
}

void KnobLookAndFeel::drawRotarySlider(juce::Graphics& g,
                                         int x, int y, int width, int height,
                                         float sliderPos,
                                         float rotaryStartAngle,
                                         float rotaryEndAngle,
                                         juce::Slider& slider)
{
    auto bounds = juce::Rectangle<int>(x, y, width, height).toFloat();
    auto size   = juce::jmin(bounds.getWidth(), bounds.getHeight());
    auto centreX = bounds.getCentreX();
    auto centreY = bounds.getCentreY();
    auto angle   = rotaryStartAngle + sliderPos * (rotaryEndAngle - rotaryStartAngle);

    // ── Dimensions ──
    auto knobR   = size * (isHero ? 0.30f : 0.26f);
    float flangeR = knobR * 1.15f;
    float barrelR = knobR * 0.92f;
    float topR    = knobR * 0.88f;
    float barrelH = knobR * (isHero ? 0.35f : 0.25f);
    float flangeY = centreY + barrelH * 0.3f;
    float topCY   = flangeY - barrelH;
    float arcRadius = flangeR + size * 0.08f;

    const float lightAngle = -0.6f; // ~10 o'clock

    // Read dynamic scale properties from slider
    auto& props = slider.getProperties();
    juce::String scaleType  = props.getWithDefault("scaleType", "none").toString();
    juce::String labelLeft  = props.getWithDefault("labelLeft", "").toString();
    juce::String labelRight = props.getWithDefault("labelRight", "").toString();

    if (scaleType == "none" || scaleType.isEmpty())
    {
        auto name = slider.getName().toLowerCase();
        if (name.contains("cutoff") || name.contains("freq"))
        {
            scaleType = "endpoints";
            if (labelLeft.isEmpty())  labelLeft  = "10";
            if (labelRight.isEmpty()) labelRight = "48";
        }
        else if (name.contains("reso") || name.contains("drive") ||
                 name.contains("mix") || name.contains("tone"))
            scaleType = "dots";
        else if (name.contains("thresh"))
        {
            scaleType = "endpoints";
            if (labelLeft.isEmpty())  labelLeft  = "-40";
            if (labelRight.isEmpty()) labelRight = "+10dB";
        }
        else if (name.contains("release"))
        {
            scaleType = "endpoints";
            if (labelLeft.isEmpty())  labelLeft  = "10ms";
            if (labelRight.isEmpty()) labelRight = "2.5s";
        }
        else if (name.contains("dry") || name.contains("wet"))
        {
            scaleType = "endpoints";
            if (labelLeft.isEmpty())  labelLeft  = "DRY";
            if (labelRight.isEmpty()) labelRight = "WET";
        }
    }

    // ═══════════════════════════════════════════════════════════════════════════
    // LAYER 1 — Drop shadow
    // ═══════════════════════════════════════════════════════════════════════════
    {
        float shadowR = flangeR + 3.f;
        juce::Path shadowPath;
        shadowPath.addEllipse(centreX - shadowR, flangeY - shadowR + 1.f,
                              shadowR * 2.f, shadowR * 2.f);
        auto& shadow = isHero ? heroShadow : knobShadow;
        shadow.render(g, shadowPath);
    }

    // ═══════════════════════════════════════════════════════════════════════════
    // LAYER 2 — Glow ring spill on panel (behind everything)
    // ═══════════════════════════════════════════════════════════════════════════
    {
        float glowBase = 0.05f + 0.10f * sliderPos;
        // Outer spill
        juce::ColourGradient spillOuter(
            juce::Colour(Theme::kGlowOuter).withAlpha(glowBase * 0.5f), centreX, flangeY,
            juce::Colour(Theme::kGlowOuter).withAlpha(0.0f), centreX + flangeR * 1.8f, flangeY, true);
        g.setGradientFill(spillOuter);
        g.fillEllipse(centreX - flangeR * 1.8f, flangeY - flangeR * 1.8f,
                      flangeR * 3.6f, flangeR * 3.6f);

        // Mid spill
        juce::ColourGradient spillMid(
            juce::Colour(Theme::kGlowMid).withAlpha(glowBase * 0.7f), centreX, flangeY,
            juce::Colour(Theme::kGlowMid).withAlpha(0.0f), centreX + flangeR * 1.3f, flangeY, true);
        g.setGradientFill(spillMid);
        g.fillEllipse(centreX - flangeR * 1.3f, flangeY - flangeR * 1.3f,
                      flangeR * 2.6f, flangeR * 2.6f);
    }

    // ═══════════════════════════════════════════════════════════════════════════
    // LAYER 3 — Base flange (wide metal disc)
    // ═══════════════════════════════════════════════════════════════════════════
    {
        auto flangeRect = juce::Rectangle<float>(flangeR * 2.f, flangeR * 2.f)
                              .withCentre({centreX, flangeY});

        juce::ColourGradient flangeGrad(
            juce::Colour(Theme::kFlangeLit), centreX - flangeR * 0.5f, flangeRect.getY(),
            juce::Colour(Theme::kFlangeShadow), centreX + flangeR * 0.5f, flangeRect.getBottom(), false);
        flangeGrad.addColour(0.4, juce::Colour(0xff252828));
        flangeGrad.addColour(0.7, juce::Colour(0xff181a1c));
        g.setGradientFill(flangeGrad);
        g.fillEllipse(flangeRect);

        // Rim highlight
        juce::ColourGradient rimGrad(
            juce::Colour(0x30ffffff), centreX + flangeR * std::sin(lightAngle), flangeRect.getY(),
            juce::Colour(0x05000000), centreX, flangeRect.getBottom(), false);
        g.setGradientFill(rimGrad);
        g.drawEllipse(flangeRect.reduced(0.5f), 1.2f);

        // Bottom edge shadow
        g.setColour(juce::Colour(0x40000000));
        g.drawEllipse(flangeRect.expanded(0.5f), 0.8f);
    }

    // ═══════════════════════════════════════════════════════════════════════════
    // LAYER 4 — Neon glow ring (full 360° LED ring at barrel base)
    // ═══════════════════════════════════════════════════════════════════════════
    {
        float ringR = (barrelR + flangeR) * 0.5f;
        float ringY = flangeY;

        juce::Path ringPath;
        ringPath.addEllipse(centreX - ringR, ringY - ringR, ringR * 2.f, ringR * 2.f);

        // Bloom layers: outer → inner → core → white-hot
        struct GlowLayer { float width; float alpha; juce::Colour colour; };
        GlowLayer layers[] = {
            { isHero ? 28.f : 18.f, 0.06f, juce::Colour(Theme::kGlowOuter) },
            { isHero ? 16.f : 10.f, 0.12f, juce::Colour(Theme::kGlowMid)   },
            { isHero ?  8.f :  5.f, 0.30f, juce::Colour(Theme::kGlowInner) },
            { isHero ?  3.f :  2.f, 0.90f, juce::Colour(Theme::kGlowCore)  },
            { isHero ?  1.2f: 0.8f, 0.50f, juce::Colours::white            },
        };
        for (auto& layer : layers)
        {
            g.setColour(layer.colour.withAlpha(layer.alpha));
            g.strokePath(ringPath, juce::PathStrokeType(layer.width,
                juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
        }
    }

    // ═══════════════════════════════════════════════════════════════════════════
    // LAYER 5 — Barrel side wall (the visible cylinder between cap and flange)
    // ═══════════════════════════════════════════════════════════════════════════
    {
        // Build barrel band path: straight sides + curved bottom
        juce::Path barrelBand;
        barrelBand.startNewSubPath(centreX - barrelR, topCY + topR * 0.15f);
        barrelBand.lineTo(centreX - barrelR, flangeY);
        barrelBand.quadraticTo(centreX, flangeY + barrelR * 0.18f,
                               centreX + barrelR, flangeY);
        barrelBand.lineTo(centreX + barrelR, topCY + topR * 0.15f);
        barrelBand.closeSubPath();

        // Side-lit gradient (left lighter, right darker)
        juce::ColourGradient barrelGrad(
            juce::Colour(Theme::kBarrelLit), centreX - barrelR, flangeY,
            juce::Colour(Theme::kBarrelShadow), centreX + barrelR, flangeY, false);
        barrelGrad.addColour(0.20, juce::Colour(0xff505255));
        barrelGrad.addColour(0.40, juce::Colour(Theme::kBarrelMid));
        barrelGrad.addColour(0.65, juce::Colour(0xff222224));
        g.setGradientFill(barrelGrad);
        g.fillPath(barrelBand);

        // Horizontal machining grooves on barrel side
        {
            g.saveState();
            g.reduceClipRegion(barrelBand);

            int numGrooves = isHero ? 28 : 16;
            float bandTop = topCY + topR * 0.15f;
            float bandBot = flangeY;
            for (int i = 0; i < numGrooves; ++i)
            {
                float t = (float)i / (float)(numGrooves - 1);
                float ly = bandTop + t * (bandBot - bandTop);
                // Brighter at top, dimmer at bottom
                float grooveBright = 0.04f + 0.08f * (1.f - t) * (1.f - t);
                g.setColour(juce::Colours::white.withAlpha(grooveBright));
                g.drawHorizontalLine((int)ly, centreX - barrelR + 2.f, centreX + barrelR - 2.f);
                g.setColour(juce::Colour(0x18000000));
                g.drawHorizontalLine((int)ly + 1, centreX - barrelR + 2.f, centreX + barrelR - 2.f);
            }
            g.restoreState();
        }

        // Specular highlight band on barrel (vertical strip on lit side)
        {
            g.saveState();
            g.reduceClipRegion(barrelBand);
            float specX = centreX - barrelR * 0.3f;
            juce::ColourGradient specGrad(
                juce::Colour(0x00ffffff), specX - barrelR * 0.3f, flangeY,
                juce::Colour(0x00ffffff), specX + barrelR * 0.3f, flangeY, false);
            specGrad.addColour(0.4, juce::Colour(0x18ffffff));
            specGrad.addColour(0.6, juce::Colour(0x18ffffff));
            g.setGradientFill(specGrad);
            g.fillPath(barrelBand);
            g.restoreState();
        }

        // Top edge chamfer (bright edge where barrel meets cap)
        {
            float chamferY = topCY + topR * 0.12f;
            g.saveState();
            juce::Path chamferClip;
            chamferClip.addRectangle(centreX - barrelR, chamferY - 2.f, barrelR * 2.f, 4.f);
            g.reduceClipRegion(barrelBand);
            juce::ColourGradient chamGrad(
                juce::Colour(0x30ffffff), centreX - barrelR, chamferY,
                juce::Colour(0x08000000), centreX + barrelR, chamferY, false);
            g.setGradientFill(chamGrad);
            g.fillRect(centreX - barrelR, chamferY - 1.f, barrelR * 2.f, 2.f);
            g.restoreState();
        }

        // AO at barrel base (where barrel meets glow ring)
        {
            g.saveState();
            g.reduceClipRegion(barrelBand);
            juce::ColourGradient aoGrad(
                juce::Colour(0x00000000), centreX, flangeY - barrelH * 0.3f,
                juce::Colour(Theme::kBarrelAO).withAlpha(0.6f), centreX, flangeY, false);
            g.setGradientFill(aoGrad);
            g.fillPath(barrelBand);
            g.restoreState();
        }
    }

    // ═══════════════════════════════════════════════════════════════════════════
    // LAYER 6 — Cyan reflection on barrel (glow ring illuminating the barrel base)
    // ═══════════════════════════════════════════════════════════════════════════
    {
        float spillAlpha = 0.08f + 0.15f * sliderPos;
        juce::Path barrelClip;
        // Approximate barrel band for clipping
        barrelClip.startNewSubPath(centreX - barrelR, topCY + topR * 0.15f);
        barrelClip.lineTo(centreX - barrelR, flangeY);
        barrelClip.quadraticTo(centreX, flangeY + barrelR * 0.18f,
                               centreX + barrelR, flangeY);
        barrelClip.lineTo(centreX + barrelR, topCY + topR * 0.15f);
        barrelClip.closeSubPath();

        g.saveState();
        g.reduceClipRegion(barrelClip);

        juce::ColourGradient cyanSpill(
            accentColour.withAlpha(0.0f), centreX, topCY,
            accentColour.withAlpha(spillAlpha), centreX, flangeY, false);
        cyanSpill.addColour(0.6, accentColour.withAlpha(spillAlpha * 0.15f));
        g.setGradientFill(cyanSpill);
        g.fillRect(centreX - barrelR, topCY, barrelR * 2.f, barrelH + barrelR * 0.2f);
        g.restoreState();
    }

    // ═══════════════════════════════════════════════════════════════════════════
    // LAYER 7 — Top cap (brushed aluminum with concentric lathe rings)
    // ═══════════════════════════════════════════════════════════════════════════
    {
        auto topRect = juce::Rectangle<float>(topR * 2.f, topR * 2.f)
                           .withCentre({centreX, topCY});

        // Base gradient — side-lit
        juce::ColourGradient topGrad(
            juce::Colour(Theme::kCapBase), centreX - topR * 0.4f, topRect.getY(),
            juce::Colour(0xff181818), centreX + topR * 0.4f, topRect.getBottom(), false);
        topGrad.addColour(0.2, juce::Colour(0xff3c3c3c));
        topGrad.addColour(0.5, juce::Colour(Theme::kCapShadow));
        topGrad.addColour(0.8, juce::Colour(0xff1a1a1a));
        g.setGradientFill(topGrad);
        g.fillEllipse(topRect);

        // Concentric lathe rings
        {
            juce::Path topClip;
            topClip.addEllipse(topRect);
            g.saveState();
            g.reduceClipRegion(topClip);

            int numRings = isHero ? 55 : 30;
            for (int i = 1; i <= numRings; ++i)
            {
                float ringR = topR * ((float)i / (float)numRings);
                float phase = (float)i * 0.7f;
                float ringBright = 0.03f + 0.07f * (0.5f + 0.5f * std::sin(phase));
                ringBright *= (0.5f + 0.5f * ((float)i / (float)numRings));
                g.setColour(juce::Colours::white.withAlpha(ringBright));
                g.drawEllipse(centreX - ringR, topCY - ringR,
                              ringR * 2.f, ringR * 2.f, isHero ? 0.6f : 0.4f);
            }

            // Anisotropic specular — bright band across brushed metal
            {
                float specCX = centreX + topR * 0.12f * std::sin(lightAngle);
                float specCY2 = topCY - topR * 0.12f * std::cos(lightAngle);

                // Broad specular wash
                juce::ColourGradient broadSpec(
                    juce::Colour(0x45ffffff), specCX, specCY2,
                    juce::Colour(0x00ffffff), specCX, specCY2 + topR * 0.8f, true);
                g.setGradientFill(broadSpec);
                g.fillEllipse(specCX - topR * 0.8f, specCY2 - topR * 0.8f,
                              topR * 1.6f, topR * 1.6f);

                // Tight hotspot
                juce::ColourGradient tightSpec(
                    juce::Colour(0x55ffffff), specCX, specCY2,
                    juce::Colour(0x00ffffff), specCX, specCY2 + topR * 0.25f, true);
                g.setGradientFill(tightSpec);
                g.fillEllipse(specCX - topR * 0.25f, specCY2 - topR * 0.25f,
                              topR * 0.5f, topR * 0.5f);
            }

            // Ambient bounce (opposite side)
            {
                float bX = centreX - topR * 0.3f * std::sin(lightAngle);
                float bY = topCY + topR * 0.3f * std::cos(lightAngle);
                juce::ColourGradient bounceGrad(
                    juce::Colour(0x0cffffff), bX, bY,
                    juce::Colour(0x00ffffff), bX, bY + topR * 0.4f, true);
                g.setGradientFill(bounceGrad);
                g.fillEllipse(bX - topR * 0.35f, bY - topR * 0.35f,
                              topR * 0.7f, topR * 0.7f);
            }

            g.restoreState();
        }

        // Chamfer ring
        {
            juce::ColourGradient chamferGrad(
                juce::Colour(0x45ffffff),
                centreX + topR * std::sin(lightAngle), topCY - topR * std::cos(lightAngle),
                juce::Colour(0x08000000),
                centreX - topR * std::sin(lightAngle), topCY + topR * std::cos(lightAngle),
                false);
            g.setGradientFill(chamferGrad);
            g.drawEllipse(topRect, 1.8f);
        }

        // Dark seam between barrel and cap
        {
            g.setColour(juce::Colour(0xff040404));
            g.drawEllipse(topRect.expanded(1.f), 1.2f);
            juce::ColourGradient seamHL(
                juce::Colour(0x12ffffff), centreX, topRect.getY() - 1.f,
                juce::Colour(0x00ffffff), centreX, topCY, false);
            g.setGradientFill(seamHL);
            g.drawEllipse(topRect.expanded(1.5f), 0.5f);
        }
    }

    // ═══════════════════════════════════════════════════════════════════════════
    // LAYER 8 — Indicator notch (extends from cap into barrel)
    // ═══════════════════════════════════════════════════════════════════════════
    {
        float notchW = isHero ? 3.5f : 2.5f;

        // A. Notch on top cap
        {
            auto topRect = juce::Rectangle<float>(topR * 2.f, topR * 2.f)
                               .withCentre({centreX, topCY});
            juce::Path capClip;
            capClip.addEllipse(topRect);
            g.saveState();
            g.reduceClipRegion(capClip);

            float notchOuter = topR * 0.97f;
            float notchInner = topR * 0.08f;

            // Dark groove body
            juce::Path groove;
            groove.addRectangle(-notchW * 0.5f, -notchOuter, notchW, notchOuter - notchInner);
            groove.applyTransform(juce::AffineTransform::rotation(angle)
                                    .translated(centreX, topCY));
            g.setColour(juce::Colour(Theme::kNotchDeep));
            g.fillPath(groove);

            // Shadow edge (right)
            juce::Path gShadow;
            gShadow.addRectangle(notchW * 0.25f, -notchOuter, 1.0f, notchOuter - notchInner);
            gShadow.applyTransform(juce::AffineTransform::rotation(angle)
                                     .translated(centreX, topCY));
            g.setColour(juce::Colour(0x50000000));
            g.fillPath(gShadow);

            // Light edge (left)
            juce::Path gLight;
            gLight.addRectangle(-notchW * 0.5f - 0.5f, -notchOuter, 0.8f, notchOuter - notchInner);
            gLight.applyTransform(juce::AffineTransform::rotation(angle)
                                    .translated(centreX, topCY));
            g.setColour(juce::Colour(0x20ffffff));
            g.fillPath(gLight);

            g.restoreState();
        }

        // B. Notch extending down into barrel side
        {
            juce::Path barrelClip;
            barrelClip.startNewSubPath(centreX - barrelR, topCY + topR * 0.15f);
            barrelClip.lineTo(centreX - barrelR, flangeY);
            barrelClip.quadraticTo(centreX, flangeY + barrelR * 0.18f,
                                   centreX + barrelR, flangeY);
            barrelClip.lineTo(centreX + barrelR, topCY + topR * 0.15f);
            barrelClip.closeSubPath();

            g.saveState();
            g.reduceClipRegion(barrelClip);

            float barrelNotchLen = barrelH * 0.45f;
            juce::Path bGroove;
            bGroove.addRectangle(-notchW * 0.5f, -topR, notchW, barrelNotchLen + topR * 0.2f);
            bGroove.applyTransform(juce::AffineTransform::rotation(angle)
                                     .translated(centreX, topCY));
            g.setColour(juce::Colour(Theme::kNotchDeep));
            g.fillPath(bGroove);

            g.restoreState();
        }

        // Center convergence point
        {
            float cpR = isHero ? 3.0f : 2.0f;
            g.setColour(juce::Colour(0xff080808));
            g.fillEllipse(centreX - cpR, topCY - cpR, cpR * 2.f, cpR * 2.f);
            g.setColour(juce::Colour(0x30000000));
            g.drawEllipse(centreX - cpR, topCY - cpR, cpR * 2.f, cpR * 2.f, 0.8f);
            g.setColour(juce::Colour(0x20ffffff));
            g.fillEllipse(centreX - cpR * 0.4f, topCY - cpR * 0.7f, cpR * 0.8f, cpR * 0.5f);
        }
    }

    // ═══════════════════════════════════════════════════════════════════════════
    // LAYER 9 — Active value arc (neon glow arc outside the knob)
    // ═══════════════════════════════════════════════════════════════════════════
    {
        // Dim background arc
        juce::Path arcBg;
        arcBg.addCentredArc(centreX, centreY, arcRadius, arcRadius,
                            0.f, rotaryStartAngle, rotaryEndAngle, true);
        g.setColour(juce::Colour(0x15ffffff));
        g.strokePath(arcBg, juce::PathStrokeType(isHero ? 2.0f : 1.5f,
            juce::PathStrokeType::curved, juce::PathStrokeType::rounded));

        // Active arc with glow
        if (sliderPos > 0.005f)
        {
            juce::Path arcActive;
            arcActive.addCentredArc(centreX, centreY, arcRadius, arcRadius,
                                    0.f, rotaryStartAngle, angle, true);

            g.setColour(accentColour.withAlpha(0.06f));
            g.strokePath(arcActive, juce::PathStrokeType(isHero ? 20.f : 12.f,
                juce::PathStrokeType::curved, juce::PathStrokeType::rounded));

            g.setColour(accentColour.withAlpha(0.14f));
            g.strokePath(arcActive, juce::PathStrokeType(isHero ? 8.f : 5.f,
                juce::PathStrokeType::curved, juce::PathStrokeType::rounded));

            g.setColour(accentColour.withAlpha(0.9f));
            g.strokePath(arcActive, juce::PathStrokeType(isHero ? 2.5f : 1.8f,
                juce::PathStrokeType::curved, juce::PathStrokeType::rounded));

            g.setColour(juce::Colour(0x50ffffff));
            g.strokePath(arcActive, juce::PathStrokeType(isHero ? 1.0f : 0.7f,
                juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
        }
    }

    // ═══════════════════════════════════════════════════════════════════════════
    // LAYER 10 — Scale dots & labels
    // ═══════════════════════════════════════════════════════════════════════════
    {
        float dotR = arcRadius + (isHero ? 10.f : 7.f);
        int numDots = 21;
        float dotSize = isHero ? 2.5f : 1.8f;

        // Draw round dots around the arc
        for (int i = 0; i <= numDots; ++i)
        {
            float t = (float)i / (float)numDots;
            float dotAngle = rotaryStartAngle + t * (rotaryEndAngle - rotaryStartAngle);
            float dx = centreX + dotR * std::sin(dotAngle);
            float dy = centreY - dotR * std::cos(dotAngle);
            g.setColour(juce::Colour(Theme::kDotMarker));
            g.fillEllipse(dx - dotSize * 0.5f, dy - dotSize * 0.5f, dotSize, dotSize);
        }

        // Endpoint labels
        if (scaleType == "endpoints" && (labelLeft.isNotEmpty() || labelRight.isNotEmpty()))
        {
            float labelR = dotR + 12.f;
            g.setColour(juce::Colours::white.withAlpha(0.85f));
            g.setFont(Theme::getFont(isHero ? 10.f : 8.f, true));
            int textW = 36, textH = 14;

            if (labelLeft.isNotEmpty())
            {
                float aL = rotaryStartAngle;
                float lx = centreX + labelR * std::sin(aL) - textW * 0.5f;
                float ly = centreY - labelR * std::cos(aL) - textH * 0.5f;
                g.drawText(labelLeft, (int)lx, (int)ly, textW, textH, juce::Justification::centred);
            }
            if (labelRight.isNotEmpty())
            {
                float aR = rotaryEndAngle;
                float rx = centreX + labelR * std::sin(aR) - textW * 0.5f;
                float ry = centreY - labelR * std::cos(aR) - textH * 0.5f;
                g.drawText(labelRight, (int)rx, (int)ry, textW, textH, juce::Justification::centred);
            }
        }
    }
}

// ══════════════════════════════════════════════════════════════════════════════
// ComboBox (unchanged)
// ══════════════════════════════════════════════════════════════════════════════

void KnobLookAndFeel::drawComboBox(juce::Graphics& g, int width, int height,
                                     bool /*isButtonDown*/,
                                     int /*buttonX*/, int /*buttonY*/,
                                     int /*buttonW*/, int /*buttonH*/,
                                     juce::ComboBox& box)
{
    auto bounds = juce::Rectangle<float>(0, 0, (float)width, (float)height);
    g.setColour(juce::Colour(Theme::kBgPanel).withAlpha(0.92f));
    g.fillRoundedRectangle(bounds, 6.f);

    bool hovered = box.isMouseOver();
    bool focused = box.hasKeyboardFocus(false);
    float borderAlpha = (focused || hovered) ? 0.5f : 0.15f;
    auto borderColour = (focused || hovered)
        ? juce::Colour(Theme::kCyanBright) : juce::Colour(Theme::kCyanDim);
    g.setColour(borderColour.withAlpha(borderAlpha));
    g.drawRoundedRectangle(bounds.reduced(0.5f), 6.f, 1.f);

    juce::Path arrow;
    float arrowX = (float)width - 14.f;
    float arrowY = (float)height * 0.5f;
    arrow.addTriangle(arrowX - 4.f, arrowY - 2.f,
                      arrowX + 4.f, arrowY - 2.f,
                      arrowX, arrowY + 3.f);
    g.setColour(juce::Colour(Theme::kCyanBright).withAlpha(hovered ? 0.9f : 0.5f));
    g.fillPath(arrow);
}

void KnobLookAndFeel::drawPopupMenuBackground(juce::Graphics& g, int width, int height)
{
    auto bounds = juce::Rectangle<float>(0, 0, (float)width, (float)height);
    g.setColour(juce::Colour(Theme::kBgPanel).withAlpha(0.95f));
    g.fillRoundedRectangle(bounds, 6.f);
    g.setColour(juce::Colour(Theme::kCyanDim).withAlpha(0.3f));
    g.drawRoundedRectangle(bounds.reduced(0.5f), 6.f, 1.f);
    g.setColour(juce::Colour(Theme::kCyanBright).withAlpha(0.15f));
    g.fillRect(6.f, 0.5f, (float)width - 12.f, 1.f);
}

void KnobLookAndFeel::drawPopupMenuItem(juce::Graphics& g, const juce::Rectangle<int>& area,
                                          bool isSeparator, bool isActive, bool isHighlighted,
                                          bool isTicked, bool /*hasSubMenu*/,
                                          const juce::String& text,
                                          const juce::String& /*shortcutKeyText*/,
                                          const juce::Drawable* /*icon*/,
                                          const juce::Colour* /*textColour*/)
{
    if (isSeparator)
    {
        g.setColour(juce::Colour(Theme::kBorderSubtle));
        g.fillRect(area.reduced(5, 0).withHeight(1).withY(area.getCentreY()));
        return;
    }

    auto r = area.reduced(4, 1);
    if (isHighlighted && isActive)
    {
        g.setColour(juce::Colour(Theme::kCyanBright).withAlpha(0.12f));
        g.fillRoundedRectangle(r.toFloat(), 4.f);
        g.setColour(juce::Colour(Theme::kCyanGlow));
    }
    else
    {
        g.setColour(isActive ? juce::Colour(Theme::kTextPrimary)
                             : juce::Colour(Theme::kTextDisabled));
    }

    if (isTicked)
    {
        g.setColour(juce::Colour(Theme::kCyanBright));
        g.fillEllipse(r.getX() + 4.f, (float)r.getCentreY() - 2.f, 4.f, 4.f);
    }

    g.setFont(Theme::getFont(12.f));
    g.drawText(text, r.withTrimmedLeft(isTicked ? 16 : 8), juce::Justification::centredLeft);
}
