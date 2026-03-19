cmake_minimum_required(VERSION 3.22)
project(DoctorsDistortion VERSION 1.0.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_subdirectory(libs/JUCE)
juce_add_module(libs/melatonin_blur)

juce_add_plugin(DoctorsDistortion
    COMPANY_NAME "DOPE FXs"
    BUNDLE_ID "com.dopefxs.DopeFilterBank"
    IS_SYNTH FALSE
    NEEDS_MIDI_INPUT FALSE
    NEEDS_MIDI_OUTPUT FALSE
    IS_MIDI_EFFECT FALSE
    EDITOR_WANTS_KEYBOARD_FOCUS FALSE
    COPY_PLUGIN_AFTER_BUILD TRUE
    PLUGIN_MANUFACTURER_CODE DcAu
    PLUGIN_CODE DcDs
    FORMATS AU VST3 Standalone
    PRODUCT_NAME "DØPE - FilterBank"
)

file(GLOB_RECURSE DSP_SOURCES src/dsp/*.cpp)
file(GLOB_RECURSE MOD_SOURCES src/modulation/*.cpp)
file(GLOB_RECURSE GUI_SOURCES src/gui/*.cpp)

target_sources(DoctorsDistortion PRIVATE
    src/PluginProcessor.cpp
    src/PluginEditor.cpp
    ${DSP_SOURCES}
    ${MOD_SOURCES}
    ${GUI_SOURCES}
)

juce_add_binary_data(DopeAssets SOURCES
    assets/dope_logo.png
    assets/texture_bg.jpg
    assets/circuit_bg.png
    assets/panel_bg.png
    assets/dark_hex_bg.png
    assets/halo_bg.png
    assets/xray_bg.jpg
    assets/HKGroteskWide-Bold.otf
    assets/HKGroteskWide-Medium.otf
    assets/WebUI/index.html
    assets/WebUI/styles.css
    assets/WebUI/script.js
)

target_link_libraries(DoctorsDistortion PRIVATE DopeAssets)

target_compile_definitions(DoctorsDistortion PUBLIC
    JUCE_WEB_BROWSER=1
    JUCE_USE_CURL=0
    JUCE_VST3_CAN_REPLACE_VST2=0
    JUCE_DISPLAY_SPLASH_SCREEN=1
)

target_link_libraries(DoctorsDistortion
    PRIVATE
        juce::juce_audio_utils
        juce::juce_dsp
        juce::juce_opengl
        juce::juce_gui_extra
        melatonin_blur
    PUBLIC
        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
        juce::juce_recommended_warning_flags
)

# --- Tests ---
include(FetchContent)
FetchContent_Declare(Catch2
    GIT_REPOSITORY https://github.com/catchorg/Catch2.git
    GIT_TAG v3.5.2
)
FetchContent_MakeAvailable(Catch2)

file(GLOB_RECURSE TEST_SOURCES tests/*.cpp)

add_executable(DoctorsDistortionTests
    ${TEST_SOURCES}
    ${DSP_SOURCES}
    ${MOD_SOURCES}
)
target_include_directories(DoctorsDistortionTests PRIVATE src)
target_link_libraries(DoctorsDistortionTests PRIVATE
    Catch2::Catch2WithMain
    juce::juce_dsp
    juce::juce_audio_basics
)
target_compile_definitions(DoctorsDistortionTests PRIVATE
    JUCE_WEB_BROWSER=1
    JUCE_USE_CURL=0
)
