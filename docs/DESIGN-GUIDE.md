# DØPE FilterBank — Design Guide

**Last updated:** 2026-03-17
**Plugin:** DØPE - FilterBank (com.dopefxs.DopeFilterBank)
**Manufacturer:** DOPE FXs (DcAu)

---

## 1. Color Token System

All colors are defined as `uint32_t` constants in `KnobLookAndFeel.h` and as local `constexpr` in `PluginEditor.cpp`. The palette is a dark X-ray / medical imaging theme: near-black backgrounds with bright cyan-blue accents and white-hot highlights.

### Primary Palette (PluginEditor.cpp)

| Token | Hex | Role |
|-------|-----|------|
| `kBlueBright` | `#00bbff` | Primary accent, arc fill, border glow |
| `kBlueAccent` | `#1e90ff` | Secondary accent (reserved) |
| `kBlueDim` | `#0a4a7a` | Muted accent, disabled states, grey knob arc |
| `kBlueGlow` | `#00ddff` | Glow bloom, LED dots, pointer highlights |
| `kBgDark` | `#080c14` | Deepest background |
| `kBgPanel` | `#0c1220` | Panel fill (dark layer) |
| `kBgPanelLite` | `#101828` | Panel fill (light layer, gradient top) |

### Knob Palette (KnobLookAndFeel.h)

| Token | Hex | Role |
|-------|-----|------|
| `kRed` | `#e60020` | Default accent (overridden to blue in editor) |
| `kRedDark` | `#aa0018` | Dark variant |
| `kRedGlow` | `#FF1744` | Glow variant |
| `kBgLight` | `#f2f2f2` | (Unused in current theme) |
| `kBlack` | `#111111` | (Unused in current theme) |
| `kBorder` | `#222222` | (Unused in current theme) |
| `kTextDim` | `#888888` | Dim label text |
| `kTextBrt` | `#222222` | (Unused in current theme) |
| `kTrack` | `#e0e0e0` | (Unused in current theme) |

Note: `KnobLookAndFeel` initializes with `kRed` but the editor calls `setAccentColour(kBlueBright)` on all three LookAndFeel instances, so the actual rendered color is always `kBlueBright` / `kBlueDim`.

### Knob Body Gradient

Two-stop radial gradient for the 3D raised knob effect:

- Outer ring top: `#2a3040`
- Outer ring bottom: `#141a24`
- Body top: `#222a36`
- Body mid (40%): `#1c2430`
- Body bottom: `#141a24`
- Top highlight: `#ffffff` at 9% opacity, fading to transparent

---

## 2. Spacing — 8px Grid

All layout values in `resized()` are multiples of 8px or derived from the total bounds. The plugin window is 740×540px (default), resizable 620–1480 × 460–1080.

Key layout constants derived from `paint()` / `resized()`:

| Element | Value |
|---------|-------|
| Header height | 44px |
| Meter bar height | 48px |
| Panel corner radius | 8px |
| LED dot inset from corner | 8px |
| Knob label offset below knob | 8px |
| Scan line spacing | 2px (bg), 3px (panels) |
| Filter display frame | 6px corner radius |

---

## 3. Knob Strategy

Three `KnobLookAndFeel` instances are created in the editor:

| Instance | Role | Accent | Hero |
|----------|------|--------|------|
| `heroLnf` | Large cutoff knobs (f1Cutoff, f2Cutoff, distDrive, distMix, distTone) | `kBlueBright` | `true` |
| `smallRedLnf` | Small parameter knobs (f1Reso, f2Reso, limThresh, limRelease, dryWet) | `kBlueBright` | `false` |
| `smallGreyLnf` | Utility knobs (inGain, outGain) | `kBlueDim` | `false` |

**Hero knob sizing factors** (relative to bounding box):
- Arc radius: `size * 0.42`
- Knob radius: `radius * 0.78` (hero) / `radius * 0.72` (small)
- Arc stroke: 3px bright core + 10px mid glow + 20px wide bloom (hero); 2px / 6px / 12px (small)
- Drop shadow radius: 16px (hero) / 10px (small)
- Pointer line width: 2.5px (hero) / 2px (small)

**Knob assets** (in `assets/`): `knob_body.png`, `knob_body_alt.png`, `knob_filmstrip.png`, and quadrant crops (`knob_tl`, `knob_tr`, `knob_br`, `knob_crop_*`) are present for a potential filmstrip-based approach. The current implementation uses code-drawn knobs via `KnobLookAndFeel`, not filmstrip rendering. The filmstrip assets are available for a future switch if needed.

---

## 4. Animation

### Timer-driven Updates
- `DoctorsDistortionEditor::timerCallback()` fires at **30Hz** (started with `startTimerHz(30)`)
- Per tick: reads APVTS parameter values → pushes to `FilterDisplay` via `setFilter1Params()` / `setFilter2Params()` → increments `pulsePhase` → calls `repaint()` on bottom meter strip only (`repaint(0, H - 52, W, 52)`)

### FilterDisplay Animation
- `FilterDisplay::timerCallback()` fires at **60Hz**
- Per tick: exponential smoothing on cutoff + reso targets (`smoothing = 0.35f` per frame)
- Recalculates both magnitude response arrays (256 points each)
- Calls `repaint()` on itself

### Background Caching
- `rebuildCachedBg()` renders the full background (X-ray image, scan lines, grain, edge glows, watermark skull) into a `juce::Image` cached as `cachedBg`
- Cache is only rebuilt when window size changes (`cachedBgW != W || cachedBgH != H`)
- Painted in `paint()` via `g.drawImageAt(cachedBg, 0, 0)` — single blit, no per-frame re-render

### JUCE 8 VBlankAnimatorBuilder
Not currently used. All animation is timer-based. If switching to `VBlankAnimatorBuilder` (JUCE 8 GPU-sync path), attach to the editor's `timerCallback` or use `juce::VBlankAttachment`.

---

## 5. Rendering Phases

Each `paint()` call layers in this order:

1. **Cached background blit** — `cachedBg` image (deep black + X-ray + scan lines + grain + edge glows + skull watermark)
2. **Header bar** — gradient fill + blue accent line + DØPE / FILTERBANK text + LED dots
3. **Section panels** — `drawPanel()` for each section: gradient fill → X-ray texture tile (15% opacity) → outer glow border → top highlight line → vertical edge lines → corner LED dots → inner scan lines → section label
4. **Knobs** — painted by JUCE component tree (each `juce::Slider` in rotary mode calls `KnobLookAndFeel::drawRotarySlider()`)
5. **FilterDisplay** — painted by its own `paint()`: background gradient → center radial glow → scan lines → blue border → frequency/dB grid → combined curve fill + glow
6. **Meter bars** — `drawLedSegmentMeter()` called in `paint()`: track fill → blue gradient fill → glow tip

---

## 6. Glass / Glow Effects

All glow is layered multi-stroke or `ColourGradient` — no native blur. Uses `melatonin_blur` only for knob drop shadows.

### Knob Arc Glow (triple-stroke)
```
strokeWidth * 8  @  4% opacity   — wide bloom
strokeWidth * 4  @ 15% opacity   — mid halo
strokeWidth      @ 90% opacity   — bright core
strokeWidth * 0.4 @ 31% opacity  — white-hot center
```

### Panel Border Glow (double-line)
```
border expanded by 1px  @  6% opacity  — outer bloom
border at exact size    @ 15% opacity  — sharp edge
```

### Panel Top Highlight Line
Horizontal gradient on the top 1px of each panel:
- Left/right: transparent
- Center: `kBlueGlow` at 20% opacity

### FilterDisplay Center Glow
Radial `ColourGradient` from center: `kBlueBright` at 3% → transparent at 40% of width.

### Cursor / LED Dots
Three-layer circle at corner positions:
- 12px circle at 12% opacity — bloom
- 5px circle at 70–80% — bright core
- 3px circle at 50% white — hot center

---

## 7. Reference Repos / Inspirations

- **Cytomic SVF** (Andrew Simper) — filter topology used in `SVFilter.cpp`
- **Välimäki & Smith (2006)** — Moog ladder filter model used in `LadderFilter.cpp`
- **Bristow-Johnson EQ Cookbook** — biquad coefficients used in `EQFilter.cpp`
- **melatonin_blur** (`libs/melatonin_blur`) — drop shadow rendering for knobs
- **JUCE 8** (`libs/JUCE`) — framework, `juce::dsp::Oversampling`, `juce::AudioProcessorValueTreeState`
- Visual reference: Lunaris (panel corner LEDs, top highlight lines), hospital/X-ray imaging equipment aesthetic

---

## 8. Asset Inventory (`assets/`)

| File | Usage |
|------|-------|
| `dope_logo.png` | Skull logo, drawn as background watermark at 4% opacity |
| `xray_bg.jpg` | Primary background texture (25% opacity base + 15% per panel) |
| `texture_bg.jpg` | Secondary texture (loaded, not currently composited) |
| `circuit_bg.png` | Circuit board texture (loaded, not currently composited) |
| `panel_bg.png` | Panel texture (loaded, not currently composited) |
| `dark_hex_bg.png` | Black hexagonal pattern (loaded, not currently composited) |
| `halo_bg.png` | Halo glow asset (loaded, not currently composited) |
| `HKGroteskWide-Bold.otf` | Primary font — section labels, header, knob labels |
| `HKGroteskWide-Medium.otf` | Secondary font — version string, tooltips |
| `knob_filmstrip.png` | Filmstrip knob sprite (not active — code-drawn knobs in use) |
| `knob_body*.png` | Knob body variants (not active — code-drawn knobs in use) |

Embedded into binary via `juce_add_binary_data(DopeAssets)` in `CMakeLists.txt`.
