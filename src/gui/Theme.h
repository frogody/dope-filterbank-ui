#pragma once
#include "BinaryData.h"
#include <juce_graphics/juce_graphics.h>

namespace Theme
{

// ── Neon Cyan Accent ──────────────────────────────────────────────────────────
inline constexpr uint32_t kCyanBright  = 0xff00E5FF;  // primary accent
inline constexpr uint32_t kCyanAccent  = 0xff00BCD4;  // secondary
inline constexpr uint32_t kCyanDim     = 0xff0a4a5a;  // muted / inactive
inline constexpr uint32_t kCyanGlow    = 0xff00FFFF;  // emissive / LED

// ── Background hierarchy (elevation through brightness) ──────────────────────
inline constexpr uint32_t kBgDeepest   = 0xff060a10;  // L0
inline constexpr uint32_t kBgDark      = 0xff080c14;  // L1
inline constexpr uint32_t kBgPanel     = 0xff111115;  // L2 — charcoal
inline constexpr uint32_t kBgPanelLite = 0xff1a1a1e;  // L3
inline constexpr uint32_t kBgActive    = 0xff182030;  // L4

// ── Hardware / UI utility ─────────────────────────────────────────────────────
inline constexpr uint32_t kGreyHeader    = 0xff2a2a2a;
inline constexpr uint32_t kGreyHeaderLt  = 0xff363636;
inline constexpr uint32_t kTextPrimary   = 0xffeeecea;
inline constexpr uint32_t kTextSecondary = 0xaaeeecea;
inline constexpr uint32_t kTextDisabled  = 0xff555555;
inline constexpr uint32_t kTextBlack     = 0xff0a0a0a;
inline constexpr uint32_t kBorderSubtle  = 0xff1a1a1e;

// ── Meter colors ──────────────────────────────────────────────────────────────
inline constexpr uint32_t kMeterSafe   = 0xff00E5FF;
inline constexpr uint32_t kMeterWarn   = 0xffFFC107;
inline constexpr uint32_t kMeterClip   = 0xffFF1744;
inline constexpr uint32_t kMeterBgSlot = 0xff0a1018;

// ── Barrel metal tones (3D cylinder lighting) ───────────────────────────────
inline constexpr uint32_t kBarrelLit      = 0xff4A4C50;
inline constexpr uint32_t kBarrelMid      = 0xff353538;
inline constexpr uint32_t kBarrelShadow   = 0xff1A1C1E;
inline constexpr uint32_t kBarrelAO       = 0xff0E1012;

// ── Top cap tones (brushed aluminum) ────────────────────────────────────────
inline constexpr uint32_t kCapBase        = 0xff484A4E;
inline constexpr uint32_t kCapShadow      = 0xff2A2C30;

// ── Neon glow ring (LED ring at knob base) ──────────────────────────────────
inline constexpr uint32_t kGlowCore       = 0xff00F0FF;
inline constexpr uint32_t kGlowInner      = 0xff00D4E8;
inline constexpr uint32_t kGlowMid        = 0xff00B8CC;
inline constexpr uint32_t kGlowOuter      = 0xff006E7A;

// ── Base flange tones ───────────────────────────────────────────────────────
inline constexpr uint32_t kFlangeLit      = 0xff2A2D30;
inline constexpr uint32_t kFlangeShadow   = 0xff1E2022;

// ── Scale markers ───────────────────────────────────────────────────────────
inline constexpr uint32_t kDotMarker      = 0xBFC8C8CC;
inline constexpr uint32_t kNotchDeep      = 0xff080808;

// ── Spacing constants (8px-based grid) ───────────────────────────────────────
inline constexpr int kGap          = 10;
inline constexpr int kHeaderH      = 44;
inline constexpr int kCornerRadius = 8;
inline constexpr int kComboH       = 22;

// ── Font accessor ─────────────────────────────────────────────────────────────
inline juce::Font getFont(float size, bool bold = false)
{
    static auto boldTypeface = juce::Typeface::createSystemTypefaceFor(
        BinaryData::HKGroteskWideBold_otf, BinaryData::HKGroteskWideBold_otfSize);
    static auto mediumTypeface = juce::Typeface::createSystemTypefaceFor(
        BinaryData::HKGroteskWideMedium_otf, BinaryData::HKGroteskWideMedium_otfSize);
    return juce::Font(bold ? boldTypeface : mediumTypeface).withHeight(size);
}

} // namespace Theme
