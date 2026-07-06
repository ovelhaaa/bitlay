cmake_minimum_required(VERSION 3.15)
project(Bitlay VERSION 1.0.0)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)
FetchContent_Declare(
    JUCE
    GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
    GIT_TAG        master
)
FetchContent_MakeAvailable(JUCE)

# Core DSP Library
add_library(CVSDDelayCore STATIC CVSDDelayCore.cpp)
target_include_directories(CVSDDelayCore PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

# Test Harness
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/test/test_harness.cpp")
    add_executable(test_harness test/test_harness.cpp)
    target_link_libraries(test_harness PRIVATE CVSDDelayCore)
endif()

# JUCE Plugin (VST3 and Standalone)
juce_add_plugin(Bitlay
    IS_SYNTH FALSE
    NEEDS_MIDI_INPUT FALSE
    NEEDS_MIDI_OUTPUT FALSE
    IS_MIDI_EFFECT FALSE
    EDITOR_WANTS_KEYBOARD_FOCUS TRUE
    COPY_PLUGIN_AFTER_BUILD FALSE
    PLUGIN_MANUFACTURER_CODE "Bita"
    PLUGIN_CODE "Bitl"
    FORMATS VST3 Standalone
    PRODUCT_NAME "Bitlay"
)

target_sources(Bitlay PRIVATE
    PluginProcessor.cpp
    PluginProcessor.h
    PluginEditor.cpp
    PluginEditor.h
)

target_compile_definitions(Bitlay PUBLIC
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
    JUCE_VST3_CAN_REPLACE_VST2=0
)

target_link_libraries(Bitlay PRIVATE
    juce::juce_audio_utils
    juce::juce_audio_processors
    CVSDDelayCore
)
