# DØPE FilterBank v3.0 — Complete GUI Redesign

## MISSION

Transform the DØPE FilterBank plugin from its current flat 2D panel layout into a **premium hardware-unit aesthetic** — like a real physical hardware synthesizer/effects processor. The design should look like a 3D product render: a sleek black metal chassis with a large glowing display on top and machined metal knobs on the bottom. Think Native Instruments Komplete Kontrol, Arturia V Collection hardware mode, or Output's plugins.

**This is a COMPLETE visual overhaul.** You may rearrange the entire layout, resize everything, change the proportions. The ONLY constraint is: all existing features/parameters must remain accessible. The DSP/audio processing code must NOT be touched.

---

## DESIGN REFERENCE (Study these carefully)

Look at the reference images in `~/.claude/skills/design-inspiration/references/`:
- `hardware-knobs-cyan-glow.png` — **KEY REFERENCE**: 3D metallic knobs with cyan LED glow rings underneath, brushed aluminum caps, physical feel
- `hardware-spectrum-display.png` — **KEY REFERENCE**: Large waveform/spectrum display on top, row of hardware knobs below, LED strip indicators, cyan-on-black
- `hardware-waveform-close.png` — Close-up: presets bar, labeled knobs (CUTOFF, RESO, DRIVE, TONE), LED indicators between sections
- `dope-concept-holographic.png` — The DOPE brand concept: holographic energy display, side-panel controls, glowing borders
- `dark-medical-dashboard.png` — Skull/X-ray theme, pink+cyan dual color accents, dense information panels, scan line effects

### Design Language Summary

| Element | Current v2 | Target v3 |
|---------|-----------|-----------|
| **Overall feel** | Flat 2D panels | 3D hardware unit / product render |
| **Knobs** | Flat circles with arc glow | Machined metal cylinders with LED glow rings |
| **Layout** | 4-column grid | Top: large display / Bottom: knob row |
| **Display** | Small 55% width panel | Full-width cinematic display (60% height) |
| **Background** | Dark blue hex pattern | Pitch black with subtle brushed-metal texture |
| **Borders** | Thin blue glow lines | Beveled metal edges, amber corner hardware |
| **Spacing** | Tight | Generous, breathing room |
| **Window size** | 740×540 | **1100×700** (bigger = more premium) |
| **Color** | Mono cyan | Primary cyan + accent amber/orange for hardware |

---

## NEW LAYOUT ARCHITECTURE

```
┌═══════════════════════════════════════════════════════════════════════════════┐
│ ▌DØPE FILTERBANK▐        [Serial ▾]    ◆◆◆              v1.0.0  │ 40px HEADER
├───────────────────────────────────────────────────────────────────────────────┤
│                                                                               │
│              ┌─────────────────────────────────────────────┐                  │
│              │                                             │                  │
│              │        FREQUENCY RESPONSE DISPLAY           │  ~380px
│              │        (full-width, cinematic)               │  DISPLAY
│              │        Combined F1+F2 curve                  │  ZONE
│              │        Grid: freq + dB                       │
│              │        Scanlines + CRT glow                  │
│              │                                             │
│              └─────────────────────────────────────────────┘                  │
│                                                                               │
│  ┌─── FILTER 1 ───┐ ┌─── FILTER 2 ───┐ ┌─ DISTORTION ─┐ ┌── OUTPUT ──┐    │
│  │ [Type ▾]        │ │ [Type ▾]        │ │ [Type ▾]      │ │            │    │
│  │                 │ │                 │ │               │ │            │    │ ~240px
│  │ ◉CUTOFF  ◉RESO │ │ ◉CUTOFF  ◉RESO │ │ ◉DRIVE  ◉MIX │ │  ◉DRY/WET │    │ CONTROLS
│  │                 │ │                 │ │ ◉TONE        │ │            │    │ ZONE
│  └─────────────────┘ └─────────────────┘ └──────────────┘ └────────────┘    │
│                                                                               │
│  ┌─ LIMITER ──────┐                                                          │
│  │[Type▾] ◉TH ◉RE │   ◉INPUT          METER BAR              ◉OUTPUT       │ 50px
│  └────────────────┘                                                          │ METER
├═══════════════════════════════════════════════════════════════════════════════┤
│  ▬▬▬ brushed metal bottom edge ▬▬▬                                           │ 8px
└═══════════════════════════════════════════════════════════════════════════════┘
```

### Key Layout Changes:
1. **Display takes center stage** — full width, ~55% of window height
2. **Controls in a single row below** — like hardware knob banks
3. **Sections separated by subtle dividers** — not bordered panels
4. **Limiter moved to bottom-left** — secondary control, smaller
5. **Logo panel removed** — brand identity is in the header, not a wasted panel
6. **Window: 1100×700** — min 900×580, max 1650×1050

---

## KNOB REDESIGN (Most Important Visual Change!)

The knobs must look like **real machined metal knobs** — this is the #1 thing that separates amateur from pro plugins.

### New Knob Rendering (KnobLookAndFeel::drawRotarySlider)

**HERO KNOBS (Cutoff, Drive, Dry/Wet) — ~70px diameter:**

```
Layer stack (bottom to top):
1. LED GLOW RING — the "wow" factor
   - Full circle BEHIND the knob body
   - Cyan (#00CCFF) at 0.3 alpha, blurred 12px (melatonin_blur)
   - Only visible portion = the arc from start to current value
   - When value = 0, ring is dark; value = 1.0, full bright ring
   - This creates the "knob sitting on a glowing ring" effect from references

2. DROP SHADOW
   - Black shadow, 20px blur, offset (0, 6px)
   - Creates depth, lifts knob off surface

3. KNOB BODY — 3D metallic cylinder
   - Outer ring: dark gunmetal gradient (#2A2D35 → #1A1D25), 3px border
   - Main body: radial gradient simulating a dome/convex metal surface
     - Center highlight: #4A4D55 (lighter brushed metal)
     - Edge: #1A1D25 (darker, shadow)
     - NOT flat color — must have directional lighting (top-left light source)
   - Top reflection: horizontal white gradient stripe (0.08 alpha) across upper 40%
     - This mimics the metal reflection you see on real knobs
   - Knurled edge texture: tiny radial lines around the perimeter (0.03 alpha)
     - 60 lines, 2px each, evenly distributed
     - Suggests the grip texture of a real metal knob

4. POINTER — machined indicator line
   - NOT just a line — a milled groove with light/shadow
   - Main line: white (#FFFFFF at 0.9), 2px wide, extends from 35% to 85% radius
   - Left shadow: dark (#000000 at 0.3), offset 1px left
   - Right highlight: bright white (#FFFFFF at 0.15), offset 1px right
   - TIP: small cyan dot (4px) at the end of the pointer for visibility

5. CENTER CAP — recessed screw hole look
   - Dark circle (#0A0C14), radius 4px
   - Subtle ring: gunmetal (#3A3D45 at 0.5), 1px stroke
   - Inner dot: near-black (#050608), 2px — the "screw head"
```

**SMALL KNOBS (Reso, Tone, Thresh, Release) — ~50px diameter:**
- Same layer structure but scaled down
- LED glow ring: 8px blur instead of 12px
- Thinner knurled edge (40 lines)
- Pointer: 1.5px width

**MICRO KNOBS (Input, Output gain) — ~32px diameter:**
- Simplified: no knurling, smaller glow
- Same metallic gradient body
- Simple pointer line

### LED Glow Ring Implementation Detail

This is the signature visual. Here's exactly how to paint it:

```cpp
// In drawRotarySlider, BEFORE drawing the knob body:
{
    float glowRadius = radius + 8.0f; // Ring sits outside the knob
    juce::Path glowArc;
    glowArc.addCentredArc(centreX, centreY, glowRadius, glowRadius,
                           0.0f, rotaryStartAngle, angle, true);

    // Layer 1: Wide soft glow
    g.setColour(juce::Colour(0x00CCFF).withAlpha(0.15f));
    g.strokePath(glowArc, juce::PathStrokeType(14.0f,
        juce::PathStrokeType::curved, juce::PathStrokeType::rounded));

    // Layer 2: Medium glow
    g.setColour(juce::Colour(0x00CCFF).withAlpha(0.35f));
    g.strokePath(glowArc, juce::PathStrokeType(6.0f, ...));

    // Layer 3: Bright core
    g.setColour(juce::Colour(0x00DDFF).withAlpha(0.85f));
    g.strokePath(glowArc, juce::PathStrokeType(2.5f, ...));

    // Layer 4: White-hot center
    g.setColour(juce::Colour(0xFFFFFF).withAlpha(0.4f));
    g.strokePath(glowArc, juce::PathStrokeType(1.0f, ...));
}
```

---

## DISPLAY ZONE REDESIGN

The FilterDisplay component should become the visual centerpiece — a premium CRT-style oscilloscope display.

### New FilterDisplay Features:

1. **Full-width layout** — spans the entire content area with 20px margins on each side
2. **CRT Monitor Aesthetic:**
   - Subtle screen curvature (round the corners more: 12px radius)
   - Inner bezel: 2px dark border (#0A0C14) + 1px bright blue edge (#00BBFF at 0.3)
   - Scan line overlay: horizontal lines every 2px at 0.02 alpha
   - Subtle vignette: darken edges by 10% using radial gradient
   - Phosphor glow: the curve itself should "burn" into the screen with residual bloom

3. **Improved Curve Rendering:**
   - Main curve: 5-layer glow (wider bloom than current)
     - 30px wide bloom (#00BBFF at 0.02)
     - 16px glow (#00BBFF at 0.06)
     - 8px glow (#00BBFF at 0.15)
     - 3px core (#00CCFF at 0.9)
     - 1px white highlight (#FFFFFF at 0.3)
   - Gradient fill under curve: taller, more visible (#00BBFF at 0.12 → transparent)

4. **Grid:**
   - Dotted lines (not solid) — 2px dots every 6px
   - Frequency labels at bottom edge (inside the display)
   - dB labels at left edge
   - 0dB reference line slightly brighter (#00BBFF at 0.15)

5. **Display Header Bar** (inside the display, top):
   - Left: "FREQUENCY RESPONSE" label (8px, dim cyan)
   - Right: Current filter types shown as text ("SVF LP → Ladder HP")
   - Background: slightly lighter bar (#0E1424)

---

## HEADER REDESIGN

```
┌─────────────────────────────────────────────────────────────────┐
│  △ DØPE FILTERBANK     [Serial ▾]  [Routing ▾]     ◆◆◆ v1.0.0 │
│  ─────────────────── cyan accent line ──────────────────────── │
└─────────────────────────────────────────────────────────────────┘
```

- Height: 40px
- Background: pitch black (#050608) — NOT gradient
- "DØPE" in bold 20px, cyan (#00CCFF) with subtle text glow (3px blur)
- "FILTERBANK" in medium 13px, white at 0.5 alpha
- Routing ComboBox moved to center-area of header (saves panel space)
- Right side: 3 LED dots (amber, not blue — hardware accent color) + version
- Bottom: 1.5px cyan accent line with 4px glow bloom

---

## CONTROL ZONE DESIGN

The bottom section should feel like a **hardware control surface** — a single brushed-metal strip with knob banks.

### Section Dividers (between Filter1/Filter2/Distortion/Output):
- NOT bordered panels
- Instead: subtle vertical line dividers
  - 1px line, white at 0.06 alpha
  - Small LED dot at top of divider (cyan, 3px)
  - This mimics hardware panel section markers

### Section Labels:
- Small text ABOVE each section: "FILTER 1", "FILTER 2", "DISTORTION", "OUTPUT"
- 9px, uppercase, letter-spacing +1px, cyan at 0.4 alpha
- Left-aligned with small LED dot indicator (matching current style but smaller)

### ComboBoxes:
- ABOVE the knobs, inline with section label
- Smaller, more compact (20px height)
- Dark fill (#0A0C14), 1px cyan border at 0.15 alpha
- Rounded 4px
- Dropdown arrow: small cyan triangle

### Knob Labels:
- BELOW each knob (not above)
- 9px uppercase, white at 0.5 alpha, letter-spacing +0.5px
- 8px gap between knob bottom and label
- Value readout centered below label: 10px, cyan at 0.7 alpha

### Background for control zone:
- NOT panels with borders
- Subtle horizontal gradient: slightly lighter than main bg (#0E1220 center → #0A0E18 edges)
- This creates a "raised control surface" feel
- Optional: very subtle brushed-metal horizontal line texture (0.015 alpha)

---

## METER BAR REDESIGN

```
┌─────────────────────────────────────────────────────────────────┐
│ ◉IN  ▐████████████░░░░░░░░░▌ -12.3 dB    -6.1 dB ▐███████████████░░░░▌  ◉OUT │
└─────────────────────────────────────────────────────────────────┘
```

- Height: 44px
- Background: darkest black (#040608)
- Meters: segmented LED style (individual rectangles, 3px wide, 1px gap)
  - -60 to -12 dB: cyan (#00BBFF)
  - -12 to -3 dB: amber (#FFAA00)  ← WARNING ZONE
  - -3 to 0 dB: red (#FF3333) ← DANGER ZONE
- Inactive segments: very dark (#0A0E14)
- Peak hold indicator: bright white line, 1px, holds for 1.5 seconds
- Input/Output knobs: micro size (32px), at far left and far right
- dB readout: between knob and meter, 10px cyan text

---

## COLOR PALETTE v3

```cpp
// Primary
const uint32 kCyan        = 0xff00CCFF;  // Main accent (slightly warmer than before)
const uint32 kCyanGlow    = 0xff00DDFF;  // Bright glow
const uint32 kCyanDim     = 0xff0A5A8A;  // Dim labels

// Hardware accent (NEW — amber/orange for physical hardware details)
const uint32 kAmber       = 0xffFF9500;  // Hardware dots, brackets, warn meters
const uint32 kAmberDim    = 0xff6B4000;  // Dim amber

// Metals (NEW — for knob rendering)
const uint32 kMetalLight  = 0xff4A4D55;  // Knob highlight
const uint32 kMetalMid    = 0xff2A2D35;  // Knob body
const uint32 kMetalDark   = 0xff1A1D25;  // Knob shadow edge
const uint32 kMetalRing   = 0xff3A3D45;  // Knob outer ring

// Backgrounds
const uint32 kBgBlack     = 0xff050608;  // Main background (true black)
const uint32 kBgPanel     = 0xff0A0E18;  // Control zone bg
const uint32 kBgDisplay   = 0xff080C14;  // Display interior

// Meters
const uint32 kMeterGreen  = 0xff00CCFF;  // Normal level (cyan)
const uint32 kMeterAmber  = 0xffFFAA00;  // Warning (-12 to -3)
const uint32 kMeterRed    = 0xffFF3333;  // Clip (above -3)
```

---

## WINDOW & SIZING

```cpp
// In constructor:
setSize(1100, 700);
setResizable(true, true);
setResizeLimits(900, 580, 1650, 1050);
getConstrainer()->setFixedAspectRatio(1100.0 / 700.0);
```

---

## FILES TO MODIFY

1. **`src/PluginEditor.h`** — Update window size, add new color constants, add any new member variables
2. **`src/PluginEditor.cpp`** — Complete rewrite of `paint()`, `resized()`, `drawPanel()` (rename to `drawSection()`), `drawLedSegmentMeter()`, `rebuildCachedBg()`, header drawing
3. **`src/gui/KnobLookAndFeel.h`** — Add LED glow ring rendering, metal knob constants
4. **`src/gui/KnobLookAndFeel.cpp`** — Complete rewrite of `drawRotarySlider()` for 3D metal aesthetic with LED glow rings
5. **`src/gui/FilterDisplay.cpp`** — Enhanced CRT display with wider bloom, scanlines, vignette, display header bar

### Files to NOT modify:
- `src/PluginProcessor.cpp/h` — NO DSP changes
- `src/dsp/*` — NO DSP changes
- `CMakeLists.txt` — No new assets needed (use programmatic rendering)
- `tests/*` — Audio tests stay as-is

---

## IMPLEMENTATION ORDER

1. **Start with colors + window size** — Update constants and setSize in PluginEditor
2. **Rewrite KnobLookAndFeel** — This is the biggest visual impact. Get the metal knobs + LED glow rings right first.
3. **Rewrite PluginEditor layout** — New top-display/bottom-controls arrangement in `resized()`
4. **Rewrite paint()** — New background, section dividers, header, meter bar
5. **Enhance FilterDisplay** — CRT aesthetic, wider bloom, scanlines, vignette
6. **Polish** — Fine-tune spacing, add subtle animations, check resize behavior

---

## IMPORTANT NOTES

- The plugin uses **melatonin_blur** for drop shadows — continue using it for knob shadows and glow effects
- The font is **HK Grotesk Wide** (Bold + Medium) embedded via BinaryData — keep using it
- **dark_hex_bg.png** is the background — you can reduce its opacity further or replace with pure black + subtle programmatic texture
- **dope_logo.png** (skull) can remain as a subtle watermark in the display area
- The **FilterDisplay** component is a separate class — update it in place, don't merge into PluginEditor
- All knob attachments and parameter connections stay unchanged — only visual rendering changes
- The **ComboBox popup style** should also be updated to match (dark, cyan accent, rounded)
- Build with: `cd ~/doctors-distortion && cmake --build build --config Release`
- Test with: `cd ~/doctors-distortion && cmake --build build --target DopeFilterBankTests && ./build/tests/DopeFilterBankTests`
- After building, the plugin is at `~/doctors-distortion/build/src/DopeFilterBank_artefacts/Release/VST3/` and `AU/`

---

## QUALITY BAR

When done, the plugin should look like it costs $149 — not like a free open-source project. Every detail matters:
- Knobs should look like you could reach out and turn them
- The display should glow like a real CRT oscilloscope
- The overall impression should be "this is a physical hardware unit rendered on screen"
- Compare against: FabFilter Pro-Q 3, Arturia Pigments, Output Portal, Spectrasonics Omnisphere

Make it **premium**. Make it **DØPE**.
