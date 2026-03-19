# Doctors Distortion — Plugin Design Document

**Datum:** 2026-03-15
**Status:** Goedgekeurd
**Doel:** Commercieel audio effect plugin (VST3 + AU)

---

## 1. Product Visie

Een commerciële distortion/filter effect plugin geïnspireerd door de architectuur van multi-filter plugins. Eigen naam, eigen GUI, eigen code — alle DSP uit gepubliceerde literature.

- **Type:** Puur effect plugin (geen synthesizer/oscillators)
- **Doelgroep:** Producers, sound designers, mixers
- **Platforms:** macOS 11+ (ARM + Intel), Windows 10+ (x64)
- **Formaten:** VST3 + AU (Mac), VST3 (Windows)
- **DAW compatibiliteit:** Cubase, Ableton, Logic Pro, FL Studio, etc.
- **Naam:** Werknaam "Doctors Distortion" — medisch/chirurgisch thema, definitieve naam TBD

## 2. Signaalflow

```
Audio Input (Stereo)
    │
    ▼
┌─────────────────────────────────────────────────┐
│              FILTER ROUTING ENGINE               │
│                                                  │
│   [Filter 1]  ◄── mode ──►  [Filter 2]         │
│   (52 types)    SER/PAR/    (52 types)          │
│   Cutoff, Res   SPLIT/SUB   Cutoff, Res         │
│   Drive, Pan                 Drive, Pan          │
└─────────────────┬───────────────────────────────┘
                  │
                  ▼
┌─────────────────────────────────────────────────┐
│            DISTORTION ENGINE                     │
│   12 types, 4x oversampled                      │
│   Drive, Mix, Tone, Pre/Post EQ                 │
└─────────────────┬───────────────────────────────┘
                  │
                  ▼
┌─────────────────────────────────────────────────┐
│          DELAY / REVERB                          │
│   20 types, BPM sync                            │
│   ◄──► Distortion (volgorde instelbaar)         │
└─────────────────┬───────────────────────────────┘
                  │
                  │◄──── FEEDBACK PATH (terug naar filters)
                  ▼
           Output (Stereo) + Dry/Wet Mix
```

### Filter Routing Modes
1. **Serial (SER):** Filter 1 → Filter 2
2. **Parallel (PAR):** Filter 1 + Filter 2 gesommeerd
3. **Stereo Split (SPLIT):** Filter 1 → L, Filter 2 → R
4. **Subtractive (SUB):** Filter 1 minus Filter 2

## 3. Filter Types (52 stuks)

Alle geïmplementeerd uit gepubliceerde DSP literature (SVF, Bristow-Johnson EQ Cookbook, Välimäki/Smith).

| Categorie | Types |
|-----------|-------|
| Lowpass | LP 6dB, 12dB, 18dB, 24dB, 36dB, 48dB |
| Highpass | HP 6dB, 12dB, 18dB, 24dB, 36dB, 48dB |
| Bandpass | BP 6dB, 12dB, 24dB, 36dB |
| Notch | Notch 6dB, 12dB, 24dB |
| Allpass | AP 6dB, 12dB, 24dB |
| Ladder | Ladder LP, Ladder HP, Ladder BP |
| Comb | Comb+, Comb-, Comb BP |
| Resonator | Reso Mono, Reso Stereo, Reso Tuned |
| Formant | Vowel A, E, I, O, U, Morph |
| Phaser | Phase 4-stage, 8-stage, 12-stage |
| EQ | Low Shelf, High Shelf, Peak, Tilt |
| FM | FM LP, FM HP, FM BP |
| AM | AM Ring, AM Tremolo |
| Vocal | Vocal Male, Vocal Female, Vocal Whisper |
| Special | Octave Up, Octave Down, Bitfilter, M-Shape, Elliptic |

### Per Filter Parameters
- Cutoff (20Hz - 20kHz)
- Resonance (0 - 100%, self-oscillation mogelijk)
- Drive (pre-filter saturation)
- Pan (stereo placement)
- Key tracking (on/off)

## 4. Distortion Types (12 stuks, alle oversampled)

| Type | Algoritme | Karakter |
|------|-----------|----------|
| Hard Clip | `max(-1, min(1, x * drive))` | Agressief, digitaal |
| Soft Clip | `tanh(x * drive)` | Warm, tube-achtig |
| Tube Amp | Asymmetrische waveshaping | Vintage warmte |
| Tape | Hysteresis model | Subtiele saturatie |
| Wavefold | `sin(x * drive)` varianten | Metalig, harmonisch rijk |
| Waveshape | Chebyshev polynomials | Controleerbare harmonischen |
| Bitcrush | Bit depth + sample rate reduction | Lo-fi, retro |
| Rectify | Half/full wave rectification | Octave-up effect |
| Fuzz | Diode clipper model | Gitaar fuzz |
| Square | `sign(x)` blending | Harsh, square wave |
| Cubic | `x - x³/3` soft saturation | Subtiel, musicaal |
| Decimate | Sample & hold + jitter | Glitchy, destruktief |

### Distortion Parameters
- Type selector
- Drive (0 - 100%)
- Mix (dry/wet per unit)
- Tone (post-distortion tilt EQ)
- Oversampling (2x/4x/8x selecteerbaar)

## 5. Modulatie Engine

### Bronnen (6 stuks)
| Bron | Features |
|------|----------|
| LFO 1 | 8 waveforms, 0.01Hz - 5kHz (audio-rate), BPM sync, fade-in |
| LFO 2 | Zelfde als LFO 1, onafhankelijk |
| Envelope 1 | ADSR + delay + hold, retrigger, curve shaping |
| Envelope 2 | Zelfde als Envelope 1, onafhankelijk |
| Envelope Follower | Input dynamics tracking, attack/release, sensitivity |
| Step Sequencer | 1-32 steps, BPM sync, glide, swing, pattern opslag |

### Modulatie Matrix
- **8 slots:** Bron → Doel → Amount (-100% tot +100%)
- **~20 doelen:** Filter 1/2 cutoff/reso/drive/pan, Distortion drive/mix/tone, Delay time/feedback/mix, LFO rate/depth, Output level/dry-wet, Feedback amount
- **Visueel:** Gekleurde lijnen op GUI tonen routing
- **Audio-rate modulatie:** LFOs tot 5kHz voor FM-effecten

## 6. GUI Design

### Thema: "Surgical Futurism"
- **Achtergrond:** Donker metallic (#0a0e17), subtiele grid lines
- **Accenten:** Neon cyan (#00f0ff), surgical green (#00ff88), warning red (#ff2244)
- **Knoppen:** Custom circular knobs met glow-ring
- **Meters:** ECG/hartmonitor-stijl
- **Font:** Clean monospace/tech font
- **Animaties:** Subtiele pulse op actieve modulatie

### Layout (1000x600px, resizable 75%-200%)
```
┌─────────────────────────────────────────────────────────────┐
│  [Logo]  DOCTORS DISTORTION              [Preset ▼] [≡]    │
├────────────┬────────────────────────┬───────────────────────┤
│ FILTER 1   │   ┌──────────────────┐ │  FILTER 2            │
│ Type [▼]   │   │  LIVE FILTER     │ │  Type [▼]            │
│ ◉ Cutoff   │   │  DISPLAY         │ │  ◉ Cutoff            │
│ ◉ Reso     │   │  F1=cyan F2=grn  │ │  ◉ Reso              │
│ ◉ Drive    │   │  Combined=white  │ │  ◉ Drive             │
│ ◉ Pan      │   │  Draggable       │ │  ◉ Pan               │
│             │   │  20Hz─────20kHz  │ │                       │
│ [SER|PAR|  │   └──────────────────┘ │  ┌─DISTORTION──┐     │
│  SPLIT|SUB]│                         │  │ Type [▼]    │     │
│             │                         │  │ ◉ Drive     │     │
│             │                         │  │ ◉ Mix/Tone  │     │
│             │                         │  └─────────────┘     │
├────────────┴────────────────────────┴───────────────────────┤
│  [LFO1] [LFO2] [ENV1] [ENV2] [FOLLOWER] [SEQ]  │ MOD MTX  │
├─────────────────────────────────────────────────────────────┤
│  [DELAY/REVERB]  │  [FEEDBACK]  │  [OUTPUT] Dry/Wet  Gain  │
└─────────────────────────────────────────────────────────────┘
```

### Filter Display (centerpiece)
- Filter 1 als cyan curve, Filter 2 als green curve — beide realtime
- Gecombineerde response als subtiele witte gestippelde lijn
- Direct draggen op de curve voor cutoff + resonance
- Modulatie zichtbaar als bewegende curves

### UX Principes
- Simpel en effectief — een leek moet het kunnen gebruiken
- Alles zichtbaar — geen verborgen menu's voor hoofdfuncties
- Drag-to-modulate — sleep modbron naar knob
- Tooltips op elke knob
- Preset browser met categorieën, zoek, A/B vergelijking
- MIDI learn op elke knob

## 7. Technische Specificaties

| Spec | Waarde |
|------|--------|
| Sample rates | 44.1kHz - 192kHz |
| Interne processing | 64-bit floating point |
| Oversampling | 2x/4x/8x (selecteerbaar) |
| Latency | Zero-latency mode + oversampling mode |
| Buffer sizes | 32 - 4096 samples |
| CPU target | <5% per instantie bij 44.1kHz/4x OS |
| GUI framerate | 60fps, OpenGL accelerated |
| Preset formaat | XML-based, forward compatible |
| Undo/Redo | Per parameter wijziging |

## 8. Framework & Tooling

- **Framework:** JUCE 8 (C++)
- **Build:** CMake
- **Compiler:** Xcode (Mac), MSVC (Windows)
- **Licentie:** JUCE gratis tot $50K omzet/jaar

## 9. Projectstructuur

```
doctors-distortion/
├── CMakeLists.txt
├── libs/JUCE/                  (submodule)
├── src/
│   ├── PluginProcessor.cpp/.h  (main audio engine)
│   ├── PluginEditor.cpp/.h     (main GUI)
│   ├── dsp/
│   │   ├── FilterEngine.cpp    (52 filter types)
│   │   ├── FilterRouter.cpp    (SER/PAR/SPLIT/SUB)
│   │   ├── Distortion.cpp      (12 distortion types)
│   │   ├── DelayReverb.cpp
│   │   ├── Feedback.cpp
│   │   └── Oversampler.cpp
│   ├── modulation/
│   │   ├── LFO.cpp
│   │   ├── Envelope.cpp
│   │   ├── EnvFollower.cpp
│   │   ├── StepSequencer.cpp
│   │   └── ModMatrix.cpp
│   └── gui/
│       ├── FilterDisplay.cpp   (live spectrum view)
│       ├── KnobLookAndFeel.cpp
│       ├── ModDragOverlay.cpp
│       └── PresetBrowser.cpp
├── presets/Factory/
├── assets/fonts/
├── installers/mac/ + windows/
└── docs/plans/
```

## 10. Juridische Overwegingen

- **Eigen naam** — geen "FilterBank" of Tone2 trademarks
- **Eigen GUI** — futuristisch thema, visueel totaal anders
- **Eigen code** — alle DSP uit gepubliceerde textboeken
- **Eigen presets** — originele sounds
- **IP quickscan** door advocaat aangeraden voor launch (~€200-500)
- **JUCE licentie** — gratis tot $50K, daarna $40-130/mo

## 11. Launch Strategie

Feature-complete v1.0 launch:
- 52 filter types
- 12 distortion types
- Volledige modulatie engine (6 bronnen, 8-slot matrix)
- Gepolijste futuristische GUI
- Factory presets
- Mac + Windows installers
