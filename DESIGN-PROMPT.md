# DOPE FilterBank — Design Upgrade Prompt

Kopieer alles hieronder en plak het in een nieuwe Claude Code terminal:

---

Ga verder met de DOPE FilterBank plugin in `~/doctors-distortion/`.

Gebruik de **juce-plugin-design** skill en **context7** (library ID: `/websites/juce_master`) voor JUCE API docs.

## Opdracht

Upgrade de visuele kwaliteit van de plugin GUI zodat het eruitziet als een premium €500 hardware rack unit — niet als een standaard software UI. Het moet voelen als een Halo UNSC console: strak, militair-precies, futuristisch.

## Huidige staat

De plugin werkt en heeft alle features. De GUI heeft:
- Witte/off-white achtergrond met lichte texture overlay (7% opacity)
- Rode header bar + rode meter bar onderaan
- Zilveren VOX-style knobs met rode arc glow
- Panelen met beveled edges en gradient fill
- LED dots bij section headers
- Circuit trace lines tussen filter panelen
- DOPE skull watermark op 3%
- HK Grotesk Wide font (embedded via BinaryData)
- melatonin_blur voor drop shadows
- Plugin window: 740x540 (resizable, min 620x460)

## Wat er mis is — gebaseerd op analyse van professionele plugins

### Spacing is te krap (grootste probleem)
- `gap = 3px` → professionals gebruiken **10-20px** tussen secties
- `headerH = 36px` → standaard is **48px**
- Knob labels staan 1px onder de knob → moet **8px** zijn
- Alles plakt aan elkaar, er is geen ademruimte

### Verhoudingen kloppen niet
- Hero:small knob ratio is ~1.8:1 → moet **3:1** zijn (bijv. 110px hero vs 36px small)
- Section header font 13px → moet **16px** zijn
- Knob label font 12px → moet **10-11px** zijn (kleiner = minder visueel druk)

### Achtergrond is te leeg
- Texture op 7% opacity = onzichtbaar. Pro's doen **15-25%**
- Noise grain: slechts 600 dots = niks zichtbaar
- Geen achtergrond-fill technieken (hex grid, scan lines)

### Referentie: hoe pro's ruimte vullen
- **FabFilter**: spectrum analyzer IS de achtergrond — data vult 100% van het scherm
- **Universal Audio 1176**: brushed aluminum textuur vult ALLE lege ruimte — textuur = oppervlak, niet leegte
- **Soundtoys Decapitator**: groot VU meter als focal point + constrained dark faceplate + hout side panels
- **Valhalla**: bewuste leegte maar met GROTE knobs en perfecte spacing op 8px grid
- **Arturia**: silkscreen labels en panel graphics vullen alle gaten tussen controls
- **Brainworx console strips**: 70-80% van het oppervlak is controls — extreem dicht

## Professionele maten referentietabel

Gebruik deze waarden als richtlijn:

```
ELEMENT                    PRO STANDAARD     JOUW HUIDIG     ACTIE
─────────────────────────────────────────────────────────────────────
Plugin venster             1024x600+         740x540         Overweeg vergroten
Header bar hoogte          48px              36px            → 48px
Sectie gap                 10-20px           3px             → 12px
Panel padding (intern)     8-16px            4-6px           → 10px
Hero knob                  80-120px          ~dynamisch      Vergroot
Kleine knob                30-40px           55% van hero    Verklein naar 33%
Hero:klein ratio           3:1               1.8:1           → 3:1
Section header font        14-18px           13px            → 16px
Knob label font            10-12px           12px            → 10px
Value readout font         13-16px           (geen)          Overweeg toevoegen
Label onder knob spacing   6-10px            1px             → 8px
Knob-tot-knob spacing      16-32px           variabel        → 20px minimum
Background texture opacity 15-25%            7%              → 20%
Noise grain density        (W*H)/6           600 dots        → (W*H)/6
LED glow diameter          12-16px           8px             → 14px
LED core diameter          6-8px             5px             → 8px
Panel corner radius        4-8px             4px             → 6px
Grid base unit             8px               geen            → 8px grid
```

## Achtergrond afbeelding — GEBRUIK DEZE

Er staat een nieuwe achtergrond-textuur klaar: `assets/halo_bg.png`
Dit is een witte futuristische circuit board textuur met rode LED glows — Halo UNSC stijl.

**Stappen om deze als achtergrond te gebruiken:**

1. Voeg `assets/halo_bg.png` toe aan `CMakeLists.txt` in de `juce_add_binary_data` sectie
2. Laad in de constructor: `bgTexture = ImageCache::getFromMemory(BinaryData::halo_bg_png, BinaryData::halo_bg_pngSize);`
3. Vervang de huidige `texture_bg.jpg` referentie door `halo_bg.png`
4. Teken in paint() op **25-35% opacity** — deze textuur is al wit-gebaseerd dus hij blended perfect met de witte achtergrond
5. De textuur heeft al rode LED glows erin — laat die doorschijnen, dat geeft het levend gevoel
6. Stretch de image over het hele venster: `g.drawImage(bgTexture, 0, 0, W, H, 0, 0, bgTexture.getWidth(), bgTexture.getHeight());`

**BELANGRIJK**: Deze textuur vervangt de noodzaak voor procedurele hex grids en circuit traces in paint(). De afbeelding doet dat al. Focus de procedurele effecten op scan lines en noise grain BOVENOP deze textuur.

## Specifieke taken

### 1. Spacing & Layout fix (PluginEditor.cpp `resized()` + `paint()`)
Dit is de BELANGRIJKSTE taak — doe dit EERST.
- Verander `gap` van 3 naar 12
- Verander `headerH` van 36 naar 48
- Pas `layoutKnob()` aan: label spacing van 1px naar 8px onder de knob
- Herbereken alle panel bounds met de nieuwe gap waarden
- Zorg dat hero knobs ~3x zo groot zijn als kleine knobs
- Verklein knob label font van 12px naar 10px
- Vergroot section header font van 13px naar 16px
- Laat alles op een 8px grid vallen waar mogelijk

### 2. Achtergrond (PluginEditor.cpp `paint()`)
- Vervang `texture_bg.jpg` door `halo_bg.png` (zie sectie hierboven)
- Teken `halo_bg.png` op **30% opacity** over de witte base fill
- Voeg BOVENOP de textuur lichte noise grain toe: `(W * H) / 10` dots, alpha 0x05 — cache naar juce::Image
- Voeg horizontale scan lines toe BOVENOP alles (elke 3px, alpha 0x04)
- Maak skull watermark iets meer zichtbaar: van 0.03f naar 0.06f
- VERWIJDER de procedurele hex grid code — de halo_bg.png heeft al circuit/hex patronen
- VERWIJDER de procedurele circuit trace code in paint() — de textuur doet dit al

### 3. Panelen (drawPanel functie)
- Vervang ronde hoeken door angular cuts (45-graden chamfer op top-right hoek, 8px cut)
- Voeg brushed metal effect toe: horizontale lijnen (elke 2px, alpha 0x08) over de panel gradient
- Maak inset shadow donkerder: van alpha 0x10 naar 0x25
- Voeg een dunne rode lijn (1px, alpha 0x33) langs de bovenkant van elk panel
- Voeg een subtiele rode inner glow toe langs de linker- en bovenrand (2px, alpha 0x0C)

### 4. Section Headers
- Maak LED dots groter: glow van 8px naar 14px, core van 5px naar 8px
- Verhoog LED glow alpha van 0x30 naar 0x50
- Voeg een tweede glow-ring toe (20px diameter, alpha 0x15) achter de eerste voor bloom effect
- Vergroot de section header font naar 16px bold

### 5. Circuit Traces
- Verhoog trace opacity van 0x18 naar 0x35
- Maak traces dikker: van 1px naar 1.5px
- Voeg verticale traces toe tussen top en bottom panelen (bij elke sectie-overgang)
- Voeg "node" dots toe op kruispunten (4px diameter, gevulde cirkel, alpha 0x40)
- Voeg pulserende glow toe via timerCallback: bereken `float pulse = 0.15f + 0.10f * std::sin(pulsePhase)` met `pulsePhase += 0.05f` per frame. Gebruik pulse als alpha voor trace glow.

### 6. Knobs (KnobLookAndFeel.cpp)
- Voeg rode outer glow ring toe: cirkel 12px groter dan knob bounds, alpha 0x20, kleur kAccent
- Maak active arc glow breder: van 8px naar 14px stroke width
- Voeg subtiele concentrische ringen toe op knob-oppervlak (2 ringen, alpha 0x08) voor texture
- Verander pointer line kleur van 0xff333333 naar kRed (rood)
- Overweeg: maak de knob body iets donkerder (meer contrast met witte achtergrond)

### 7. Filter Display (FilterDisplay.cpp)
- Voeg grid-achtergrond toe: verticale + horizontale lijnen elke 40px, alpha 0x0C
- Maak response curve dikker: van 2px naar 3px stroke
- Voeg glow toe achter de curve: 8px stroke in accentColour.withAlpha(0.15f)
- Voeg rode dots toe op de frequency markers (50, 100, 500, 1k, 5k, 10k) — 3px diameter

### 8. Meter Bar
- Voeg scan line effect toe: horizontale lijnen elke 2px, alpha 0x15 over de rode gradient
- Maak meter fill bars feller: van 0xcc naar 0xff (100% white)
- Voeg tick marks toe: 6 verticale streepjes verdeeld over de meter breedte, 1px wit, alpha 0x80

## Regels

- Dit is JUCE 8 C++ — gebruik `juce::Graphics& g` methods, GEEN CSS
- Gebruik `melatonin_blur` voor drop shadows waar mogelijk
- Gebruik `Random(42)` seed voor procedurele effecten (voorkomt flicker bij repaint)
- Cache dure effecten (noise grain, hex grid) naar `juce::Image` — bouw opnieuw bij `resized()`
- Gebruik context7 (`/websites/juce_master`) als je een JUCE API moet opzoeken
- Build na elke grote wijziging: `cd ~/doctors-distortion/build && cmake --build . --target DoctorsDistortion_AU 2>&1 | tail -5`
- Test in Logic Pro of een andere DAW na de build
- ALLE wijzigingen moeten compileren zonder errors
- Voeg een `float pulsePhase = 0.f;` member toe aan PluginEditor.h voor de pulserende traces
- Bij nieuwe `juce::Image` members: declareer in header, initialiseer in constructor of `resized()`

## Bestanden om te wijzigen

1. `src/PluginEditor.h` — nieuwe members (cachedGrain Image, pulsePhase float)
2. `src/PluginEditor.cpp` — paint(), resized(), drawPanel(), timerCallback(), layoutKnob()
3. `src/gui/KnobLookAndFeel.cpp` — drawRotarySlider()
4. `src/gui/FilterDisplay.cpp` — paint()
5. `src/gui/SectionComponent.h` — paint() (als je panels hier wilt upgraden)

## Volgorde van aanpak

1. **EERST**: Spacing & layout fix (taak 1) — dit is de basis
2. **DAN**: Achtergrond textuur + grain caching (taak 2)
3. **DAN**: Panel styling (taak 3)
4. **DAN**: Headers, traces, knobs (taken 4-6)
5. **LAATST**: Meter bar polish (taken 7-8)

Build en test na elke stap. Ga pas door naar de volgende stap als de vorige compileert en er goed uitziet.
