# Doctors Distortion — Implementation Plan

> **For Claude:** REQUIRED SUB-SKILL: Use superpowers:executing-plans to implement this plan task-by-task.

**Goal:** Build a commercial-grade VST3/AU distortion+filter effect plugin with 52 filter types, 12 distortion types, full modulation engine, and futuristic GUI.

**Architecture:** JUCE 8 monolith — single C++ codebase builds VST3 + AU. DSP engine processes stereo audio through configurable filter routing (SER/PAR/SPLIT/SUB), distortion with oversampling, delay/reverb, and feedback loop. Modulation engine with 6 sources feeds into 8-slot mod matrix. GUI uses JUCE's OpenGL-accelerated custom components.

**Tech Stack:** C++17, JUCE 8, CMake, Catch2 (testing), OpenGL (GUI rendering)

**Juridisch:** Alle DSP uit gepubliceerde literature. Geen Tone2 trademarks. Eigen GUI/code/presets.

---

## Phase 0: Environment & Project Setup

### Task 0.1: Install Build Tools

**Files:** None (system setup)

**Step 1: Install CMake via Homebrew**

```bash
brew install cmake
```

Expected: `cmake --version` returns 3.x

**Step 2: Clone JUCE 8 as submodule**

```bash
cd ~/doctors-distortion
git submodule add https://github.com/juce-framework/JUCE.git libs/JUCE
cd libs/JUCE && git checkout master
```

Expected: `libs/JUCE/CMakeLists.txt` exists

**Step 3: Commit**

```bash
git add .gitmodules libs/JUCE
git commit -m "chore: add JUCE 8 as git submodule"
```

---

### Task 0.2: CMake Project Scaffold

**Files:**
- Create: `CMakeLists.txt`
- Create: `src/PluginProcessor.h`
- Create: `src/PluginProcessor.cpp`
- Create: `src/PluginEditor.h`
- Create: `src/PluginEditor.cpp`

**Step 1: Write root CMakeLists.txt**

```cmake
cmake_minimum_required(VERSION 3.22)
project(DoctorsDistortion VERSION 1.0.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_subdirectory(libs/JUCE)

juce_add_plugin(DoctorsDistortion
    COMPANY_NAME "DoctorsAudio"
    IS_SYNTH FALSE
    NEEDS_MIDI_INPUT FALSE
    NEEDS_MIDI_OUTPUT FALSE
    IS_MIDI_EFFECT FALSE
    EDITOR_WANTS_KEYBOARD_FOCUS FALSE
    COPY_PLUGIN_AFTER_BUILD TRUE
    PLUGIN_MANUFACTURER_CODE DcAu
    PLUGIN_CODE DcDs
    FORMATS AU VST3
    PRODUCT_NAME "Doctors Distortion"
)

target_sources(DoctorsDistortion PRIVATE
    src/PluginProcessor.cpp
    src/PluginEditor.cpp
)

target_compile_definitions(DoctorsDistortion PUBLIC
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
    JUCE_VST3_CAN_REPLACE_VST2=0
    JUCE_DISPLAY_SPLASH_SCREEN=1
)

target_link_libraries(DoctorsDistortion
    PRIVATE
        juce::juce_audio_utils
        juce::juce_dsp
        juce::juce_opengl
    PUBLIC
        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
        juce::juce_recommended_warning_flags
)
```

**Step 2: Write minimal PluginProcessor.h**

```cpp
#pragma once
#include <juce_audio_processors/juce_audio_processors.h>

class DoctorsDistortionProcessor : public juce::AudioProcessor
{
public:
    DoctorsDistortionProcessor();
    ~DoctorsDistortionProcessor() override;

    void prepareToPlay(double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;
    void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }

    const juce::String getName() const override { return JucePlugin_Name; }
    bool acceptsMidi() const override { return false; }
    bool producesMidi() const override { return false; }
    double getTailLengthSeconds() const override { return 0.0; }

    int getNumPrograms() override { return 1; }
    int getCurrentProgram() override { return 0; }
    void setCurrentProgram(int) override {}
    const juce::String getProgramName(int) override { return {}; }
    void changeProgramName(int, const juce::String&) override {}

    void getStateInformation(juce::MemoryBlock& destData) override;
    void setStateInformation(const void* data, int sizeInBytes) override;

    juce::AudioProcessorValueTreeState apvts;

private:
    juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(DoctorsDistortionProcessor)
};
```

**Step 3: Write minimal PluginProcessor.cpp**

```cpp
#include "PluginProcessor.h"
#include "PluginEditor.h"

DoctorsDistortionProcessor::DoctorsDistortionProcessor()
    : AudioProcessor(BusesProperties()
          .withInput("Input", juce::AudioChannelSet::stereo(), true)
          .withOutput("Output", juce::AudioChannelSet::stereo(), true)),
      apvts(*this, nullptr, "Parameters", createParameterLayout())
{
}

DoctorsDistortionProcessor::~DoctorsDistortionProcessor() {}

juce::AudioProcessorValueTreeState::ParameterLayout
DoctorsDistortionProcessor::createParameterLayout()
{
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> params;

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{"dryWet", 1}, "Dry/Wet",
        juce::NormalisableRange<float>(0.f, 1.f), 1.f));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{"outputGain", 1}, "Output Gain",
        juce::NormalisableRange<float>(-36.f, 12.f, 0.1f), 0.f));

    return { params.begin(), params.end() };
}

void DoctorsDistortionProcessor::prepareToPlay(double sampleRate, int samplesPerBlock)
{
    juce::ignoreUnused(sampleRate, samplesPerBlock);
}

void DoctorsDistortionProcessor::releaseResources() {}

void DoctorsDistortionProcessor::processBlock(juce::AudioBuffer<float>& buffer,
                                                juce::MidiBuffer& midiMessages)
{
    juce::ignoreUnused(midiMessages);
    juce::ScopedNoDenormals noDenormals;

    // Passthrough for now — DSP added in later tasks
    auto totalNumInputChannels = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear(i, 0, buffer.getNumSamples());
}

void DoctorsDistortionProcessor::getStateInformation(juce::MemoryBlock& destData)
{
    auto state = apvts.copyState();
    std::unique_ptr<juce::XmlElement> xml(state.createXml());
    copyXmlToBinary(*xml, destData);
}

void DoctorsDistortionProcessor::setStateInformation(const void* data, int sizeInBytes)
{
    std::unique_ptr<juce::XmlElement> xml(getXmlFromBinary(data, sizeInBytes));
    if (xml != nullptr && xml->hasTagName(apvts.state.getType()))
        apvts.replaceState(juce::ValueTree::fromXml(*xml));
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new DoctorsDistortionProcessor();
}
```

**Step 4: Write minimal PluginEditor.h**

```cpp
#pragma once
#include "PluginProcessor.h"

class DoctorsDistortionEditor : public juce::AudioProcessorEditor
{
public:
    explicit DoctorsDistortionEditor(DoctorsDistortionProcessor&);
    ~DoctorsDistortionEditor() override;

    void paint(juce::Graphics&) override;
    void resized() override;

private:
    DoctorsDistortionProcessor& processorRef;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(DoctorsDistortionEditor)
};
```

**Step 5: Write minimal PluginEditor.cpp**

```cpp
#include "PluginEditor.h"

DoctorsDistortionEditor::DoctorsDistortionEditor(DoctorsDistortionProcessor& p)
    : AudioProcessorEditor(&p), processorRef(p)
{
    setSize(1000, 600);
    setResizable(true, true);
    setResizeLimits(750, 450, 2000, 1200);
}

DoctorsDistortionEditor::~DoctorsDistortionEditor() {}

void DoctorsDistortionEditor::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colour(0xff0a0e17));
    g.setColour(juce::Colour(0xff00f0ff));
    g.setFont(24.0f);
    g.drawText("DOCTORS DISTORTION", getLocalBounds(), juce::Justification::centred);
}

void DoctorsDistortionEditor::resized() {}

juce::AudioProcessorEditor* DoctorsDistortionProcessor::createEditor()
{
    return new DoctorsDistortionEditor(*this);
}
```

**Step 6: Build and verify**

```bash
cd ~/doctors-distortion
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release -j$(sysctl -n hw.ncpu)
```

Expected: Build succeeds, plugin files created at:
- `~/Library/Audio/Plug-Ins/VST3/Doctors Distortion.vst3`
- `~/Library/Audio/Plug-Ins/Components/Doctors Distortion.component`

**Step 7: Commit**

```bash
git add CMakeLists.txt src/
git commit -m "feat: minimal JUCE plugin scaffold with passthrough audio"
```

---

## Phase 1: Core DSP — Filters

### Task 1.1: State Variable Filter (SVF) Base Class

**Files:**
- Create: `src/dsp/SVFilter.h`
- Create: `src/dsp/SVFilter.cpp`
- Create: `tests/dsp/SVFilterTest.cpp`
- Modify: `CMakeLists.txt` (add source + test target)

**Step 1: Add Catch2 and test target to CMakeLists.txt**

Add after the `target_link_libraries` block:

```cmake
# Tests
include(FetchContent)
FetchContent_Declare(Catch2 GIT_REPOSITORY https://github.com/catchorg/Catch2.git GIT_TAG v3.5.2)
FetchContent_MakeAvailable(Catch2)

add_executable(DoctorsDistortionTests
    tests/dsp/SVFilterTest.cpp
    src/dsp/SVFilter.cpp
)
target_include_directories(DoctorsDistortionTests PRIVATE src)
target_link_libraries(DoctorsDistortionTests PRIVATE Catch2::Catch2WithMain juce::juce_dsp juce::juce_audio_basics)
include(Catch2::Catch2WithMain)
```

**Step 2: Write failing test — SVF lowpass at 1kHz should attenuate 10kHz**

```cpp
// tests/dsp/SVFilterTest.cpp
#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>
#include "dsp/SVFilter.h"
#include <cmath>

TEST_CASE("SVFilter LP12 attenuates above cutoff", "[svf]")
{
    SVFilter filter;
    filter.setType(SVFilter::Type::LP12);
    filter.prepare(44100.0, 512);
    filter.setCutoff(1000.f);
    filter.setResonance(0.f);

    // Generate 10kHz sine (well above 1kHz cutoff)
    const int numSamples = 4096;
    std::vector<float> buffer(numSamples);
    for (int i = 0; i < numSamples; ++i)
        buffer[i] = std::sin(2.0 * M_PI * 10000.0 * i / 44100.0);

    // Measure input RMS
    float inputRms = 0.f;
    for (auto s : buffer) inputRms += s * s;
    inputRms = std::sqrt(inputRms / numSamples);

    // Process
    for (int i = 0; i < numSamples; ++i)
        buffer[i] = filter.processSample(0, buffer[i]);

    // Measure output RMS (skip first 512 samples for transient)
    float outputRms = 0.f;
    for (int i = 512; i < numSamples; ++i) outputRms += buffer[i] * buffer[i];
    outputRms = std::sqrt(outputRms / (numSamples - 512));

    // 10kHz should be attenuated by at least 20dB through LP12 @ 1kHz
    float attenuationDb = 20.f * std::log10(outputRms / inputRms);
    REQUIRE(attenuationDb < -20.f);
}

TEST_CASE("SVFilter LP12 passes signal below cutoff", "[svf]")
{
    SVFilter filter;
    filter.setType(SVFilter::Type::LP12);
    filter.prepare(44100.0, 512);
    filter.setCutoff(10000.f);
    filter.setResonance(0.f);

    // Generate 100Hz sine (well below 10kHz cutoff)
    const int numSamples = 4096;
    std::vector<float> buffer(numSamples);
    for (int i = 0; i < numSamples; ++i)
        buffer[i] = std::sin(2.0 * M_PI * 100.0 * i / 44100.0);

    float inputRms = 0.f;
    for (auto s : buffer) inputRms += s * s;
    inputRms = std::sqrt(inputRms / numSamples);

    for (int i = 0; i < numSamples; ++i)
        buffer[i] = filter.processSample(0, buffer[i]);

    float outputRms = 0.f;
    for (int i = 512; i < numSamples; ++i) outputRms += buffer[i] * buffer[i];
    outputRms = std::sqrt(outputRms / (numSamples - 512));

    // Signal below cutoff should pass with minimal loss (<1dB)
    float attenuationDb = 20.f * std::log10(outputRms / inputRms);
    REQUIRE(attenuationDb > -1.f);
}
```

**Step 3: Run tests to verify they fail**

```bash
cd ~/doctors-distortion
cmake -B build && cmake --build build --target DoctorsDistortionTests -j$(sysctl -n hw.ncpu)
./build/DoctorsDistortionTests
```

Expected: Compilation fails (SVFilter not yet implemented)

**Step 4: Implement SVFilter**

`src/dsp/SVFilter.h`:
```cpp
#pragma once
#include <cmath>
#include <array>

class SVFilter
{
public:
    enum class Type
    {
        LP6, LP12, LP18, LP24, LP36, LP48,
        HP6, HP12, HP18, HP24, HP36, HP48,
        BP6, BP12, BP24, BP36,
        Notch6, Notch12, Notch24,
        AP6, AP12, AP24
    };

    void prepare(double sampleRate, int blockSize);
    void setType(Type newType);
    void setCutoff(float freqHz);
    void setResonance(float reso); // 0..1
    void setDrive(float driveAmount); // 0..1
    void reset();

    float processSample(int channel, float input);

private:
    void updateCoefficients();

    Type type = Type::LP12;
    double sr = 44100.0;
    float cutoff = 1000.f;
    float resonance = 0.f;
    float drive = 0.f;

    // Per-stage state (max 8 stages for 48dB)
    static constexpr int kMaxStages = 8;
    struct FilterState
    {
        double ic1eq = 0.0;
        double ic2eq = 0.0;
    };
    std::array<std::array<FilterState, kMaxStages>, 2> state{}; // [channel][stage]

    double g = 0.0, k = 0.0; // coefficients
    int numStages = 1;
    enum class BaseType { LP, HP, BP, Notch, AP } baseType = BaseType::LP;
};
```

`src/dsp/SVFilter.cpp`:
```cpp
#include "SVFilter.h"
#include <algorithm>

void SVFilter::prepare(double sampleRate, int /*blockSize*/)
{
    sr = sampleRate;
    reset();
    updateCoefficients();
}

void SVFilter::setType(Type newType)
{
    type = newType;
    switch (type)
    {
        case Type::LP6:  baseType = BaseType::LP; numStages = 1; break;
        case Type::LP12: baseType = BaseType::LP; numStages = 1; break;
        case Type::LP18: baseType = BaseType::LP; numStages = 2; break;
        case Type::LP24: baseType = BaseType::LP; numStages = 2; break;
        case Type::LP36: baseType = BaseType::LP; numStages = 3; break;
        case Type::LP48: baseType = BaseType::LP; numStages = 4; break;
        case Type::HP6:  baseType = BaseType::HP; numStages = 1; break;
        case Type::HP12: baseType = BaseType::HP; numStages = 1; break;
        case Type::HP18: baseType = BaseType::HP; numStages = 2; break;
        case Type::HP24: baseType = BaseType::HP; numStages = 2; break;
        case Type::HP36: baseType = BaseType::HP; numStages = 3; break;
        case Type::HP48: baseType = BaseType::HP; numStages = 4; break;
        case Type::BP6:  baseType = BaseType::BP; numStages = 1; break;
        case Type::BP12: baseType = BaseType::BP; numStages = 1; break;
        case Type::BP24: baseType = BaseType::BP; numStages = 2; break;
        case Type::BP36: baseType = BaseType::BP; numStages = 3; break;
        case Type::Notch6:  baseType = BaseType::Notch; numStages = 1; break;
        case Type::Notch12: baseType = BaseType::Notch; numStages = 1; break;
        case Type::Notch24: baseType = BaseType::Notch; numStages = 2; break;
        case Type::AP6:  baseType = BaseType::AP; numStages = 1; break;
        case Type::AP12: baseType = BaseType::AP; numStages = 1; break;
        case Type::AP24: baseType = BaseType::AP; numStages = 2; break;
    }
    updateCoefficients();
}

void SVFilter::setCutoff(float freqHz)
{
    cutoff = std::clamp(freqHz, 20.f, static_cast<float>(sr * 0.49));
    updateCoefficients();
}

void SVFilter::setResonance(float reso)
{
    resonance = std::clamp(reso, 0.f, 1.f);
    updateCoefficients();
}

void SVFilter::setDrive(float driveAmount)
{
    drive = std::clamp(driveAmount, 0.f, 1.f);
}

void SVFilter::reset()
{
    for (auto& ch : state)
        for (auto& s : ch)
            s = {0.0, 0.0};
}

void SVFilter::updateCoefficients()
{
    g = std::tan(M_PI * cutoff / sr);
    // Q range: 0.5 (no reso) to 25 (self-oscillation)
    double Q = 0.5 + resonance * 24.5;
    k = 1.0 / Q;
}

float SVFilter::processSample(int channel, float input)
{
    // Pre-filter drive (soft clip)
    if (drive > 0.f)
    {
        float d = 1.f + drive * 9.f;
        input = std::tanh(input * d) / std::tanh(d);
    }

    double v0 = static_cast<double>(input);

    for (int stage = 0; stage < numStages; ++stage)
    {
        auto& s = state[channel][stage];

        // Cytomic SVF (Andrew Simper's topology-preserving transform)
        double a1 = 1.0 / (1.0 + g * (g + k));
        double a2 = g * a1;
        double a3 = g * a2;

        double v3 = v0 - s.ic2eq;
        double v1 = a1 * s.ic1eq + a2 * v3;
        double v2 = s.ic2eq + a2 * s.ic1eq + a3 * v3;

        s.ic1eq = 2.0 * v1 - s.ic1eq;
        s.ic2eq = 2.0 * v2 - s.ic2eq;

        switch (baseType)
        {
            case BaseType::LP:    v0 = v2; break;
            case BaseType::HP:    v0 = v0 - k * v1 - v2; break;
            case BaseType::BP:    v0 = v1; break;
            case BaseType::Notch: v0 = v0 - k * v1; break;
            case BaseType::AP:    v0 = v0 - 2.0 * k * v1; break;
        }
    }

    return static_cast<float>(v0);
}
```

**Step 5: Build and run tests**

```bash
cmake -B build && cmake --build build --target DoctorsDistortionTests -j$(sysctl -n hw.ncpu)
./build/DoctorsDistortionTests
```

Expected: All tests PASS

**Step 6: Commit**

```bash
git add src/dsp/ tests/ CMakeLists.txt
git commit -m "feat: SVF base with LP/HP/BP/Notch/AP types and tests"
```

---

### Task 1.2: Ladder Filter

**Files:**
- Create: `src/dsp/LadderFilter.h`
- Create: `src/dsp/LadderFilter.cpp`
- Create: `tests/dsp/LadderFilterTest.cpp`
- Modify: `CMakeLists.txt` (add sources)

**Overview:** Moog-style ladder filter (Välimäki/Smith improved model). 3 types: Ladder LP, Ladder HP, Ladder BP. Self-oscillation capable.

**DSP Reference:** Välimäki & Smith, "Revisiting the Moog Ladder Filter" (2006). Zero-delay feedback topology.

**Step 1: Write failing test** — ladder LP should self-oscillate at high resonance

**Step 2: Implement LadderFilter** using zero-delay feedback 4-pole structure

**Step 3: Run tests, verify pass**

**Step 4: Commit**

```bash
git commit -m "feat: Moog-style ladder filter (LP/HP/BP) with self-oscillation"
```

---

### Task 1.3: Comb, Resonator, and Phaser Filters

**Files:**
- Create: `src/dsp/CombFilter.h/.cpp`
- Create: `src/dsp/Resonator.h/.cpp`
- Create: `src/dsp/PhaserFilter.h/.cpp`
- Create: `tests/dsp/CombFilterTest.cpp`
- Create: `tests/dsp/ResonatorTest.cpp`
- Create: `tests/dsp/PhaserFilterTest.cpp`

**Overview:**
- **Comb filters** (Comb+, Comb-, Comb BP): Delay-line based with feedback/feedforward
- **Resonator** (Mono, Stereo, Tuned): Tight bandpass biquad with high Q
- **Phaser** (4/8/12-stage): Cascaded allpass filters with modulation input

**Step 1:** Write failing tests for each filter type
**Step 2:** Implement each from published DSP literature
**Step 3:** Run tests, commit

```bash
git commit -m "feat: comb, resonator, and phaser filter implementations"
```

---

### Task 1.4: Formant, Vocal, and EQ Filters

**Files:**
- Create: `src/dsp/FormantFilter.h/.cpp`
- Create: `src/dsp/VocalFilter.h/.cpp`
- Create: `src/dsp/EQFilter.h/.cpp`
- Create: `tests/dsp/FormantFilterTest.cpp`

**Overview:**
- **Formant** (A/E/I/O/U/Morph): Parallel biquad banks at vowel formant frequencies
- **Vocal** (Male/Female/Whisper): Multi-formant model with different frequency ranges
- **EQ** (Low Shelf/High Shelf/Peak/Tilt): Bristow-Johnson Audio EQ Cookbook direct

**DSP Reference:** Bristow-Johnson "Cookbook formulae for audio EQ biquad filter coefficients"

**Step 1-4:** Same TDD pattern as above

```bash
git commit -m "feat: formant, vocal, and EQ filter implementations"
```

---

### Task 1.5: FM, AM, and Special Filters

**Files:**
- Create: `src/dsp/FMFilter.h/.cpp`
- Create: `src/dsp/AMFilter.h/.cpp`
- Create: `src/dsp/SpecialFilter.h/.cpp`

**Overview:**
- **FM** (FM LP/HP/BP): SVF with frequency-modulated cutoff input
- **AM** (Ring/Tremolo): Amplitude modulation (ring = bipolar, tremolo = unipolar)
- **Special** (Octave Up/Down, Bitfilter, M-Shape, Elliptic): Waveshaping + filter combos

**Step 1-4:** TDD pattern

```bash
git commit -m "feat: FM, AM, and special filter types"
```

---

### Task 1.6: FilterEngine — Unified Filter Selector

**Files:**
- Create: `src/dsp/FilterEngine.h`
- Create: `src/dsp/FilterEngine.cpp`
- Create: `tests/dsp/FilterEngineTest.cpp`

**Overview:** Wraps all 52 filter types behind a single interface. Handles type switching without clicks (crossfade on type change). Exposes: `setType(int)`, `setCutoff()`, `setResonance()`, `setDrive()`, `setPan()`, `processStereo()`.

```cpp
// FilterEngine.h — key interface
class FilterEngine
{
public:
    void prepare(double sampleRate, int blockSize);
    void setType(int typeIndex); // 0-51
    void setCutoff(float hz);
    void setResonance(float reso);
    void setDrive(float drive);
    void setPan(float pan); // -1..1
    void processStereo(float* left, float* right, int numSamples);
    // Returns magnitude response for GUI display
    void getMagnitudeResponse(const float* frequencies, float* magnitudes, int numPoints);
};
```

**Step 1:** Write test: switching types during processing should not produce clicks
**Step 2:** Implement FilterEngine with enum-to-filter mapping
**Step 3:** Run tests, commit

```bash
git commit -m "feat: unified FilterEngine wrapping all 52 types"
```

---

### Task 1.7: Filter Router (SER/PAR/SPLIT/SUB)

**Files:**
- Create: `src/dsp/FilterRouter.h`
- Create: `src/dsp/FilterRouter.cpp`
- Create: `tests/dsp/FilterRouterTest.cpp`

**Overview:** Routes audio through Filter 1 and Filter 2 in 4 modes.

```cpp
// Key interface
class FilterRouter
{
public:
    enum class Mode { Serial, Parallel, StereoSplit, Subtractive };

    void prepare(double sampleRate, int blockSize);
    void setMode(Mode m);
    FilterEngine& getFilter1();
    FilterEngine& getFilter2();
    void processStereo(float* left, float* right, int numSamples);
};
```

**Routing logic per mode:**
- **Serial:** `input → F1 → F2 → output`
- **Parallel:** `output = (F1(input) + F2(input)) * 0.5`
- **StereoSplit:** `left = F1(input_left), right = F2(input_right)`
- **Subtractive:** `output = F1(input) - F2(input)`

**Step 1:** Write tests for each mode
**Step 2:** Implement FilterRouter
**Step 3:** Integrate into PluginProcessor.processBlock
**Step 4:** Build plugin, load in DAW, verify filter works
**Step 5:** Commit

```bash
git commit -m "feat: filter router with SER/PAR/SPLIT/SUB modes"
```

---

## Phase 2: Core DSP — Distortion

### Task 2.1: Oversampler

**Files:**
- Create: `src/dsp/Oversampler.h/.cpp`
- Create: `tests/dsp/OversamplerTest.cpp`

**Overview:** Wraps `juce::dsp::Oversampling` for 2x/4x/8x modes. Anti-aliasing FIR filters on up/down sample.

**Step 1:** Write test: oversampled hard clip of 5kHz sine should have less aliasing than non-oversampled
**Step 2:** Implement using `juce::dsp::Oversampling<float>`
**Step 3:** Commit

```bash
git commit -m "feat: oversampler wrapper with 2x/4x/8x modes"
```

---

### Task 2.2: Distortion Engine — All 12 Types

**Files:**
- Create: `src/dsp/Distortion.h`
- Create: `src/dsp/Distortion.cpp`
- Create: `tests/dsp/DistortionTest.cpp`

**Overview:** 12 distortion algorithms, each a pure function `float process(float input, float drive)`.

```cpp
class Distortion
{
public:
    enum class Type
    {
        HardClip, SoftClip, TubeAmp, Tape,
        Wavefold, Waveshape, Bitcrush, Rectify,
        Fuzz, Square, Cubic, Decimate
    };

    void prepare(double sampleRate, int blockSize);
    void setType(Type t);
    void setDrive(float d);   // 0..1
    void setMix(float m);     // 0..1 dry/wet
    void setTone(float t);    // -1..1 tilt EQ
    void setOversampleFactor(int factor); // 2, 4, or 8
    void processStereo(float* left, float* right, int numSamples);
};
```

**Algoritmes (uit gepubliceerde DSP bronnen):**

| Type | Formule |
|------|---------|
| HardClip | `clamp(x * drive, -1, 1)` |
| SoftClip | `tanh(x * drive)` |
| TubeAmp | Asymmetric: `x >= 0 ? 1 - exp(-x*d) : -1 + exp(x*d)` with bias |
| Tape | `x * (abs(x) + drive) / (x*x + (drive-1)*abs(x) + 1)` |
| Wavefold | `sin(x * drive * PI)` |
| Waveshape | Chebyshev polynomial: `T_n(x)` weighted sum |
| Bitcrush | `round(x * 2^bits) / 2^bits` + sample rate reduction |
| Rectify | `abs(x)` (full) or `max(0, x)` (half) blended by drive |
| Fuzz | Diode: `sign(x) * (1 - exp(-abs(x * drive)))` |
| Square | `lerp(x, sign(x), drive)` |
| Cubic | `x - (x*x*x)/3` scaled by drive |
| Decimate | Sample-and-hold at reduced rate + jitter |

**Step 1:** Write test for each type: drive at 0 should be ~passthrough, drive at 1 should clip/distort
**Step 2:** Implement all 12 types
**Step 3:** Add tone control (post-distortion tilt EQ)
**Step 4:** Integrate oversampler
**Step 5:** Run tests, commit

```bash
git commit -m "feat: distortion engine with 12 types, oversampling, and tone control"
```

---

### Task 2.3: Integrate Distortion into Signal Chain

**Files:**
- Modify: `src/PluginProcessor.h/.cpp`

**Step 1:** Add FilterRouter + Distortion to processBlock: `input → FilterRouter → Distortion → output`
**Step 2:** Add all parameters to APVTS (filter1/2 cutoff, reso, drive, type; distortion type, drive, mix, tone; routing mode; dry/wet; output gain)
**Step 3:** Build, load in DAW, test with audio
**Step 4:** Commit

```bash
git commit -m "feat: integrate filter router + distortion into audio chain"
```

---

## Phase 3: Delay, Reverb & Feedback

### Task 3.1: Delay/Reverb Module

**Files:**
- Create: `src/dsp/DelayReverb.h/.cpp`
- Create: `tests/dsp/DelayReverbTest.cpp`

**Overview:** Start with 5 core types (Simple Delay, Ping-Pong, Stereo Delay, Plate Reverb, Room Reverb). BPM sync. Remaining 15 types added later.

**Step 1-4:** TDD pattern, commit

```bash
git commit -m "feat: delay/reverb module with 5 initial types and BPM sync"
```

---

### Task 3.2: Feedback Path

**Files:**
- Create: `src/dsp/FeedbackPath.h/.cpp`

**Overview:** Takes output signal, delays it, and feeds it back to the filter input. Parameters: amount (0-100%), delay time (0-500ms, BPM syncable).

**Important:** Must include a limiter/clipper in the feedback loop to prevent runaway oscillation.

**Step 1-4:** TDD pattern, commit

```bash
git commit -m "feat: feedback path with safety limiter"
```

---

### Task 3.3: Full Signal Chain Integration

**Files:**
- Modify: `src/PluginProcessor.h/.cpp`

**Step 1:** Wire up: `input → FilterRouter → Distortion ↔ DelayReverb → FeedbackPath → output`
**Step 2:** Add delay/reverb order toggle (pre/post distortion)
**Step 3:** Add all remaining APVTS parameters
**Step 4:** Build, test in DAW, commit

```bash
git commit -m "feat: complete signal chain with delay, reverb, and feedback"
```

---

## Phase 4: Modulation Engine

### Task 4.1: LFO

**Files:**
- Create: `src/modulation/LFO.h/.cpp`
- Create: `tests/modulation/LFOTest.cpp`

**Overview:** 8 waveforms (sine, tri, saw up, saw down, square, S&H, random smooth, stepped). Range 0.01Hz - 5kHz. BPM sync. Fade-in control.

**Step 1:** Test: sine LFO at 1Hz should complete one cycle in 44100 samples
**Step 2:** Implement LFO with all 8 waveforms
**Step 3:** Add BPM sync (note values: 1/1, 1/2, 1/4, 1/8, 1/16, dotted, triplet)
**Step 4:** Commit

```bash
git commit -m "feat: LFO with 8 waveforms, audio-rate, and BPM sync"
```

---

### Task 4.2: Envelope (ADSR)

**Files:**
- Create: `src/modulation/Envelope.h/.cpp`
- Create: `tests/modulation/EnvelopeTest.cpp`

**Overview:** ADSR + delay + hold stages. Curve shaping per segment (linear, exponential, logarithmic). Retrigger mode.

**Step 1-4:** TDD, commit

```bash
git commit -m "feat: ADSR envelope with curve shaping and retrigger"
```

---

### Task 4.3: Envelope Follower

**Files:**
- Create: `src/modulation/EnvFollower.h/.cpp`
- Create: `tests/modulation/EnvFollowerTest.cpp`

**Overview:** Tracks input signal amplitude. Attack/release smoothing. Sensitivity control.

```bash
git commit -m "feat: envelope follower with attack/release smoothing"
```

---

### Task 4.4: Step Sequencer

**Files:**
- Create: `src/modulation/StepSequencer.h/.cpp`
- Create: `tests/modulation/StepSequencerTest.cpp`

**Overview:** 1-32 steps. BPM sync. Per-step value + glide. Swing control. Multiple patterns storable.

```bash
git commit -m "feat: step sequencer with glide, swing, and BPM sync"
```

---

### Task 4.5: Modulation Matrix

**Files:**
- Create: `src/modulation/ModMatrix.h/.cpp`
- Create: `tests/modulation/ModMatrixTest.cpp`

**Overview:** 8 routing slots. Each slot: source (LFO1/2, Env1/2, Follower, SeqStep) → destination (any parameter) → amount (-100% to +100%). Processes per-sample for audio-rate modulation.

```cpp
class ModMatrix
{
public:
    struct Slot
    {
        int sourceId = -1;   // -1 = disabled
        int destId = -1;
        float amount = 0.f;  // -1..1
    };

    void prepare(double sampleRate, int blockSize);
    void setSlot(int index, int sourceId, int destId, float amount);
    // Called once per sample — returns modulation offset for each destination
    void process(float* modOffsets, int numDestinations);
};
```

**Step 1-4:** TDD, commit

```bash
git commit -m "feat: 8-slot modulation matrix with per-sample processing"
```

---

### Task 4.6: Integrate Modulation into Signal Chain

**Files:**
- Modify: `src/PluginProcessor.h/.cpp`

**Step 1:** Create all 6 mod sources in processor
**Step 2:** Wire mod matrix outputs to filter/distortion/delay parameters per-sample
**Step 3:** Add all mod parameters to APVTS
**Step 4:** Build, test in DAW, verify modulation works
**Step 5:** Commit

```bash
git commit -m "feat: integrate full modulation engine into signal chain"
```

---

## Phase 5: GUI — Futuristic "Surgical Futurism"

### Task 5.1: Custom Knob Component

**Files:**
- Create: `src/gui/KnobLookAndFeel.h/.cpp`

**Overview:** Draggable rotary knob with neon glow ring. Colors from theme (#00f0ff cyan, #00ff88 green). Glow intensity follows value. Tooltip on hover.

**Implementation:** Custom `juce::LookAndFeel_V4` subclass overriding `drawRotarySlider()`.

**Step 1:** Implement custom knob rendering with glow effect using `juce::Graphics` path + gradient
**Step 2:** Add mouse drag behavior (vertical drag = value change)
**Step 3:** Add hover tooltip
**Step 4:** Build, verify visually
**Step 5:** Commit

```bash
git commit -m "feat: custom futuristic rotary knob with neon glow"
```

---

### Task 5.2: Main Layout

**Files:**
- Modify: `src/PluginEditor.h/.cpp`
- Create: `src/gui/HeaderBar.h/.cpp`
- Create: `src/gui/FilterSection.h/.cpp`
- Create: `src/gui/DistortionSection.h/.cpp`
- Create: `src/gui/ModulationSection.h/.cpp`
- Create: `src/gui/BottomBar.h/.cpp`

**Overview:** Implement the full layout from the design doc. Dark background (#0a0e17) with grid lines. All sections as separate components.

**Layout structure:**
```
┌──────── HeaderBar (logo, preset, menu) ────────┐
├──────┬──────────────────┬──────────────────────┤
│ F1   │  FilterDisplay   │  F2 + Distortion     │
├──────┴──────────────────┴──────────────────────┤
│            ModulationSection                     │
├─────────────────────────────────────────────────┤
│            BottomBar (delay/fb/output)           │
└─────────────────────────────────────────────────┘
```

**Step 1:** Create each section component with placeholder content
**Step 2:** Implement FlexBox layout in `resized()`
**Step 3:** Apply dark theme colors
**Step 4:** Connect knobs to APVTS parameters
**Step 5:** Build, verify layout, commit

```bash
git commit -m "feat: main GUI layout with all sections and dark theme"
```

---

### Task 5.3: Live Filter Display

**Files:**
- Create: `src/gui/FilterDisplay.h/.cpp`

**Overview:** Central frequency response display. Filter 1 = cyan curve, Filter 2 = green curve, combined = white dashed. Draggable curves. 60fps OpenGL rendering.

**Implementation:**
- `FilterEngine::getMagnitudeResponse()` returns dB values for N frequency points
- Draw frequency response as smooth path using `juce::Path`
- Mouse drag on curve adjusts cutoff (x-axis) + resonance (y-axis)
- Modulation makes curves animate in realtime

**Step 1:** Implement magnitude response calculation in FilterEngine
**Step 2:** Create FilterDisplay component with OpenGL rendering
**Step 3:** Add draggable interaction
**Step 4:** Add modulation animation
**Step 5:** Commit

```bash
git commit -m "feat: live filter display with draggable dual curves"
```

---

### Task 5.4: Modulation Visualization

**Files:**
- Create: `src/gui/ModDragOverlay.h/.cpp`
- Create: `src/gui/LFODisplay.h/.cpp`
- Create: `src/gui/StepSequencerDisplay.h/.cpp`

**Overview:**
- LFO waveform display (realtime)
- Step sequencer grid editor
- Drag-to-modulate: drag from mod source label to any knob to create routing
- Colored rings on knobs showing modulation amount

**Step 1-4:** Implement each component, commit

```bash
git commit -m "feat: modulation visualization with drag-to-modulate"
```

---

### Task 5.5: Preset Browser

**Files:**
- Create: `src/gui/PresetBrowser.h/.cpp`
- Create: `src/PresetManager.h/.cpp`

**Overview:** Save/load presets as XML. Categories (Bass, Lead, FX, Pad, Drums, etc). Search. A/B comparison. Factory presets directory.

**Step 1:** Implement PresetManager (save/load XML, scan preset directory)
**Step 2:** Create PresetBrowser GUI component
**Step 3:** Add A/B state comparison
**Step 4:** Commit

```bash
git commit -m "feat: preset browser with categories, search, and A/B comparison"
```

---

### Task 5.6: MIDI Learn + Undo/Redo

**Files:**
- Modify: `src/PluginProcessor.h/.cpp`
- Create: `src/MidiLearnManager.h/.cpp`

**Overview:** Right-click any knob → "MIDI Learn" → move MIDI controller → mapped. Undo/redo stack for parameter changes.

```bash
git commit -m "feat: MIDI learn and undo/redo support"
```

---

## Phase 6: Polish & Release

### Task 6.1: Remaining Delay/Reverb Types

Add remaining 15 types to reach 20 total. Prioritize: Chorus Delay, Flanger Delay, Reverse Delay, Shimmer Reverb, Spring Reverb, Hall Reverb, etc.

```bash
git commit -m "feat: expand delay/reverb to 20 types"
```

---

### Task 6.2: Factory Presets

Create 50+ factory presets across categories:
- Bass (10): Sub bass, growl, filtered bass, etc.
- Lead (10): Screaming lead, vocal lead, etc.
- FX (10): Risers, drops, glitch, etc.
- Drums (10): Drum bus crunch, snare smash, etc.
- Creative (10+): Ambient textures, feedback loops, etc.

```bash
git commit -m "feat: 50+ factory presets across 5 categories"
```

---

### Task 6.3: CPU Optimization

- Profile with Instruments (macOS)
- SIMD optimization for filter processing (juce::dsp::SIMDRegister)
- Optimize mod matrix (skip disabled slots)
- Verify <5% CPU target at 44.1kHz/4x oversampling

```bash
git commit -m "perf: SIMD optimization and CPU profiling pass"
```

---

### Task 6.4: Cross-Platform Build & Installers

**Mac:**
- Build universal binary (ARM + Intel)
- Create `.pkg` installer with `pkgbuild`
- Sign with Developer ID (requires Apple Developer account, $99/year)

**Windows:**
- Set up GitHub Actions CI with MSVC
- Create NSIS or Inno Setup installer
- Code signing (optional for launch)

```bash
git commit -m "chore: cross-platform build pipeline and installers"
```

---

### Task 6.5: Final QA & DAW Testing

Test in:
- Cubase (primary target)
- Ableton Live
- Logic Pro
- FL Studio
- Reaper

Verify:
- Plugin loads without crash
- Audio processes correctly
- Preset save/load works
- GUI resizes correctly
- CPU usage acceptable
- No memory leaks (use JUCE leak detector)
- State recall works (close/reopen project)

```bash
git commit -m "test: full QA pass across 5 DAWs"
```

---

## Summary

| Phase | Tasks | Estimated Complexity |
|-------|-------|---------------------|
| Phase 0: Setup | 2 tasks | Light |
| Phase 1: Filters | 7 tasks | Heavy (core DSP) |
| Phase 2: Distortion | 3 tasks | Medium |
| Phase 3: Delay/Reverb/Feedback | 3 tasks | Medium |
| Phase 4: Modulation | 6 tasks | Heavy (mod matrix) |
| Phase 5: GUI | 6 tasks | Heavy (custom rendering) |
| Phase 6: Polish | 5 tasks | Medium |
| **Total** | **32 tasks** | |

**Critical path:** Phase 0 → Phase 1 → Phase 2 → Phase 3 → Phase 4 → Phase 5 → Phase 6

**Parallelizable:** Phase 1-4 (DSP) and Phase 5 (GUI) can partially overlap — GUI can use placeholder DSP while real DSP is being built.
