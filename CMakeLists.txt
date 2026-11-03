cmake_minimum_required(VERSION 3.22)
project(DevilFan VERSION 1.0.0)
option(DEVILFAN_COPY "Copy plugin after build" ON)

include(FetchContent)
FetchContent_Declare(JUCE
    GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
    GIT_TAG 7.0.12)
FetchContent_MakeAvailable(JUCE)

juce_add_plugin(DevilFan
    COMPANY_NAME "DevilAudio"
    PLUGIN_MANUFACTURER_CODE Devl
    PLUGIN_CODE Dfan
    FORMATS VST3 Standalone
    PRODUCT_NAME "Devil Fan"
    IS_SYNTH FALSE
    NEEDS_MIDI_INPUT FALSE
    NEEDS_MIDI_OUTPUT FALSE
    COPY_PLUGIN_AFTER_BUILD ${DEVILFAN_COPY}
    VST3_CATEGORIES Fx Modulation)

target_sources(DevilFan PRIVATE
    Source/PluginProcessor.cpp
    Source/PluginEditor.cpp)

target_compile_definitions(DevilFan PUBLIC
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
    JUCE_VST3_CAN_REPLACE_VST2=0)

target_link_libraries(DevilFan
    PRIVATE juce::juce_audio_utils
    PUBLIC juce::juce_recommended_config_flags
           juce::juce_recommended_lto_flags
           juce::juce_recommended_warning_flags)
