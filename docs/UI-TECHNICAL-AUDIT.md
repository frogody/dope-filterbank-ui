# DØPE FilterBank — UI Technical Audit

**Date:** 2026-03-17
**Auditor:** Claude Code
**Scope:** Full GUI implementation review — `src/gui/`, `src/PluginEditor.cpp/.h`

---

## 1. Tech Stack

| Layer | Technology |
|-------|-----------|
| Framework | JUCE 8 (`juce::AudioProcessorEditor`, `juce::LookAndFeel_V4`) |
| Rendering | CPU-side `juce::Graphics` (Software renderer) |
| Blur/Shadow | `melatonin_blur` — `melatonin::DropShadow` for knob drop shadows |
| Font | HK Grotesk Wide (Bold + Medium), embedded via `juce_add_binary_data` |
| Animation | `juce::Timer` — 30Hz editor, 60Hz FilterDisplay |
| Background cache | `juce::Image` (ARGB, rebuilt on resize) |
| Parameter binding | `juce::AudioProcessorValueTreeState::SliderAttachment` / `ComboBoxAttachment` |
| Build | CMake 3.22, C++17, JUCE as git submodule |
| Formats | VST3 + AU + Standalone |

---

## 2. Component Hierarchy

```
DoctorsDistortionEditor (juce::AudioProcessorEditor, juce::Timer @ 30Hz)
├── FilterDisplay (juce::Component, juce::Timer @ 60Hz)
├── f1Cutoff.slider  (juce::Slider — heroLnf)
├── f1Reso.slider    (juce::Slider — smallRedLnf)
├── f1TypeBox        (juce::ComboBox — heroLnf)
├── f2Cutoff.slider  (juce::Slider — heroLnf)
├── f2Reso.slider    (juce::Slider — smallRedLnf)
├── f2TypeBox        (juce::ComboBox — heroLnf)
├── routingBox       (juce::ComboBox — heroLnf)
├── distTypeBox      (juce::ComboBox — heroLnf)
├── distDrive.slider (juce::Slider — heroLnf)
├── distMix.slider   (juce::Slider — heroLnf)
├── distTone.slider  (juce::Slider — heroLnf)
├── limiterTypeBox   (juce::ComboBox — heroLnf)
├── limThresh.slider (juce::Slider — smallRedLnf)
├── limRelease.slider(juce::Slider — smallRedLnf)
├── dryWet.slider    (juce::Slider — smallRedLnf)
├── inGain.slider    (juce::Slider — smallGreyLnf)
└── outGain.slider   (juce::Slider — smallGreyLnf)

LookAndFeel instances (3, owned by editor):
├── heroLnf     (KnobLookAndFeel, accent = #00bbff, isHero = true)
├── smallRedLnf (KnobLookAndFeel, accent = #00bbff, isHero = false)
└── smallGreyLnf(KnobLookAndFeel, accent = #0a4a7a, isHero = false)
```

---

## 3. Layout Method

No layout manager (no `juce::FlexBox`, no `juce::Grid`). Layout is computed manually in `paint()` and `resized()` from proportional fractions of the window bounds.

**Window bounds at default size (740×540):**

```
[0, 0, 740, 44]          Header bar
[0, 44, 44, 496]         Content area (H - 44 - 48 = 448px)
  Top section (56% of 448 = 251px):
    [0, 44, 333, 120]      Filter 1 panel     (45% W, top half)
    [0, 174, 333, 120]     Filter 2 panel     (45% W, bottom half)
    [343, 44, 397, 251]    FilterDisplay      (remaining W)
  Bottom section (remaining ~187px):
    [0, 305, 207, 187]     Distortion panel   (28% W)
    [217, 305, 177, 187]   Limiter panel      (24% W)
    [404, 305, 133, 187]   Output panel       (18% W)
    [547, 305, 193, 187]   DØPE logo panel    (remaining)
[0, 492, 740, 48]        Meter bar
```

Proportions used: `filtersW = W * 0.45`, `topH = contentH * 0.56`, `distW = W * 0.28`, `limW = W * 0.24`, `outW = W * 0.18`.

Knob positions are set via `layoutKnob(k, cx, cy, size)` — center-based absolute coordinates computed from panel bounds.

---

## 4. Paint Performance Analysis

### Background (hot path eliminated)
- `rebuildCachedBg()` pre-renders: base gradient, X-ray image, scan lines, noise grain (`W*H/20` pixels), corner glows, edge lines, horizontal/vertical accent lines, watermark skull
- Total: single `juce::Image` blit per frame — O(1) regardless of complexity
- Rebuild condition: window resize only (guard: `cachedBgW != W || cachedBgH != H`)
- **Cost: negligible per frame**

### Section panels (paint path, called 7 times per frame)
Per `drawPanel()` call:
1. `fillRoundedRectangle` — panel gradient fill
2. `drawImage` — X-ray tile blit (clipped to panel)
3. 2× `drawRoundedRectangle` — border glow
4. `fillRect` — 1px top highlight
5. 2× `fillRect` — vertical edge lines
6. 4× `fillEllipse` × 3 layers — corner LED dots (12 draws)
7. For-loop scan lines: `H/3` horizontal line draws
8. LED dot + label text for labeled panels

Scan lines are the most expensive per-panel path — e.g. at 540px height, a panel with 150px height draws 50 horizontal lines. Across 7 panels: ~350 `drawHorizontalLine` calls per repaint.

**Recommendation:** Cache panel backgrounds similarly to the main BG if CPU usage becomes a concern.

### FilterDisplay (60Hz, separate component)
Per frame:
- 2× magnitude response recalculation (256-point analytical formula with a nested loop over stages)
- Combined magnitude sum (256 additions)
- Full repaint: background gradient, center glow, scan lines (H/2 draws), border, frequency grid (8 vertical + 7 horizontal), labels, curve fill path, 3-stroke curve draw

The 256-point magnitude calculation runs per-filter per-frame. Each point evaluates a biquad transfer function with a stage loop (up to 4 iterations). At 60Hz this is ~30720 iterations/sec per filter. Not a bottleneck at this scale, but worth noting if `kNumPoints` is increased significantly.

### Knob rendering (per `drawRotarySlider` call)
- 1× `melatonin::DropShadow::render()` — GPU-accelerated shadow blit
- 3× `strokePath` (arc glow layers)
- 4× ellipse fills (knob body gradient, ambient ring, outer ring, body)
- 1× ellipse highlight
- 1× concentric ring
- 3× `strokePath` (pointer)
- 1× `fillEllipse` (pointer tip)

At 12 knobs, each repaint triggers 12 full knob redraws. JUCE's dirty-region system will only repaint knobs that are dirtied (e.g., value changes), so in practice most frames only repaint the meter strip (via the partial `repaint(0, H-52, W, 52)` call in `timerCallback`).

---

## 5. Known Issues

### Issue 1: Popup menu colors not fully applying
**Location:** `PluginEditor.cpp` `styleCombo()` lambda
**Description:** `juce::PopupMenu::backgroundColourId` is set on the `ComboBox` component, but popup menus in JUCE V4 source their colors from the `LookAndFeel`, not the component. The `drawPopupMenuItem()` override in `KnobLookAndFeel` handles item rendering, but the menu background may still show the default dark JUCE color on some platforms.
**Impact:** Low — cosmetic only.

### Issue 2: FilterDisplay only draws combined curve
**Location:** `FilterDisplay.cpp` `paint()`
**Description:** `f1Mags` and `f2Mags` are calculated and cached separately, but `paint()` only calls `drawCurve()` for `combinedMags` (blue). The individual F1 (cyan) and F2 (green) curves described in the design doc are not drawn.
**Impact:** Medium — missing visual clarity on dual-filter interaction.

### Issue 3: Partial repaint covers bottom 52px only
**Location:** `PluginEditor.cpp` `timerCallback()`
**Description:** `repaint(0, getHeight() - 52, getWidth(), 52)` only repaints the meter strip. The filter display (which has its own 60Hz timer) repaints itself. However, `pulsePhase` is incremented but not used in any visible animation — it is set up for a future effect that hasn't been implemented.
**Impact:** Low — dead code, no visual artifact.

### Issue 4: Knob label bounds may clip at small sizes
**Location:** `PluginEditor.cpp` `layoutKnob()`
**Description:** Label bounds are `size + 20` wide, positioned `size/2 + 8` below center. At small plugin sizes (near 620px minimum), overlapping labels between adjacent knobs is possible.
**Impact:** Low — visible only at minimum resize.

### Issue 5: `xrayBg` source rect calculation may produce zero-size source
**Location:** `PluginEditor.cpp` `drawPanel()`
**Description:** `srcW = std::min(area.getWidth() * 2, xrayBg.getWidth() - srcX)` — if `srcX` is close to `xrayBg.getWidth()`, `srcW` can be very small or zero, resulting in a distorted tile or no-op draw. The `% std::max(1, ...)` guard prevents division by zero but doesn't guarantee a valid source rect.
**Impact:** Low — cosmetic tiling artifact possible on very narrow panels.

### Issue 6: LookAndFeel ownership
**Location:** `PluginEditor.cpp` destructor
**Description:** Knob sliders have their LookAndFeel explicitly cleared to `nullptr` in the destructor. ComboBoxes also have `setLookAndFeel(nullptr)` called. This is correct — JUCE requires this to avoid dangling LNF pointers when the editor is destroyed before the LNF. Currently handled correctly.
**Impact:** None — correctly implemented.

---

## 6. Test Coverage (as of 2026-03-17)

| File | Tests | Coverage |
|------|-------|----------|
| `tests/dsp/SVFilterTest.cpp` | 2 tests | LP12 attenuation above cutoff; LP12 passthrough below cutoff |
| `tests/dsp/AllFiltersTest.cpp` | ~15 tests | Multi-type filter smoke tests |
| `tests/modulation/` | 0 files | No modulation tests yet |

GUI components have no automated tests (expected — JUCE GUI testing requires headless component harness).

---

## 7. Resize Behavior

- `setResizeLimits(620, 460, 1480, 1080)` — 2:1 range in both axes
- All layout is proportional (no fixed pixel coordinates except header/meter heights)
- `cachedBg` automatically rebuilds on any size change
- Knob sizes should be set proportionally in `resized()` — currently all knob sizes are fixed pixel values, meaning knobs do not scale with window size. **This is a known limitation** — knobs will appear too small at 1480px width.

---

## 8. Build Configuration

| Setting | Value |
|---------|-------|
| Company | DOPE FXs |
| Bundle ID | com.dopefxs.DopeFilterBank |
| Product name | DØPE - FilterBank |
| Manufacturer code | DcAu |
| Plugin code | DcDs |
| Formats | AU, VST3, Standalone |
| C++ standard | C++17 |
| `JUCE_WEB_BROWSER` | 0 |
| `JUCE_USE_CURL` | 0 |
| `JUCE_DISPLAY_SPLASH_SCREEN` | 1 |
| Copy after build | true |
