cmake_minimum_required(VERSION 3.22)

project(JazzArchitect VERSION 0.1.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_subdirectory(JUCE)

juce_add_gui_app(JazzArchitect
    PRODUCT_NAME "Jazz Architect"
    COMPANY_NAME "MADZINE"
    BUNDLE_ID "com.madzine.jazzarchitect"
)

juce_generate_juce_header(JazzArchitect)

target_sources(JazzArchitect
    PRIVATE
        Source/Main.cpp
        Source/MainComponent.cpp
        Source/Core/ChordSymbol.cpp
        Source/Core/AudioEngine.cpp
        Source/Grammar/PCFG.cpp
        Source/Grammar/Generator.cpp
        Source/Style/StyleVector.cpp
        Source/Style/StyleEngine.cpp
        Source/VoiceLeading/GuideTone.cpp
        Source/Synthesis/ChordSynth.cpp
        Source/MIDI/MIDIOutput.cpp
)

target_compile_definitions(JazzArchitect
    PRIVATE
        JUCE_WEB_BROWSER=0
        JUCE_USE_CURL=0
        JUCE_APPLICATION_NAME_STRING="$<TARGET_PROPERTY:JazzArchitect,JUCE_PRODUCT_NAME>"
        JUCE_APPLICATION_VERSION_STRING="$<TARGET_PROPERTY:JazzArchitect,JUCE_VERSION>"
)

target_link_libraries(JazzArchitect
    PRIVATE
        juce::juce_audio_basics
        juce::juce_audio_devices
        juce::juce_audio_formats
        juce::juce_audio_processors
        juce::juce_audio_utils
        juce::juce_core
        juce::juce_data_structures
        juce::juce_events
        juce::juce_graphics
        juce::juce_gui_basics
        juce::juce_gui_extra
    PUBLIC
        juce::juce_recommended_config_flags
        juce::juce_recommended_warning_flags
)
