cmake_minimum_required(VERSION 3.22)

# The one place the version is set (Semantic Versioning, see AGENTS.md).
project(mnKarp VERSION 1.0.0)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

# JUCE: a local checkout next to this folder when there is one, else fetched.
set(MNKARP_JUCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../JUCE" CACHE PATH "Local JUCE checkout")

if(EXISTS "${MNKARP_JUCE_DIR}/CMakeLists.txt")
    add_subdirectory("${MNKARP_JUCE_DIR}" JUCE)
else()
    include(FetchContent)
    FetchContent_Declare(juce
        GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
        GIT_TAG 9.0.1
    )
    FetchContent_MakeAvailable(juce)
endif()

# mnKarp has its own copies of mnKord and mnArpee (Source/Kord, Source/Arpee; see Source/ORIGIN.md). Their title bars
# show the version they were taken from.
set(MNKARP_KORD_VERSION "1.6.1")
set(MNKARP_ARPEE_VERSION "1.0.0")

# mnKord's editor includes the generated MnKordVersion.h.
file(CONFIGURE OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/generated/MnKordVersion.h" @ONLY CONTENT
"#pragma once
#define MNKORD_VERSION        \"@MNKARP_KORD_VERSION@\"
#define MNKORD_VERSION_STRING \"@MNKARP_KORD_VERSION@\"
")

juce_add_plugin(mnKarp
    COMPANY_NAME "MNAudio"
    PLUGIN_MANUFACTURER_CODE MnAu
    PLUGIN_CODE MnKp
    FORMATS VST3 Standalone
    IS_SYNTH FALSE
    IS_MIDI_EFFECT TRUE
    NEEDS_MIDI_INPUT TRUE
    NEEDS_MIDI_OUTPUT TRUE
    NEEDS_AUDIO_INPUT FALSE
    NEEDS_AUDIO_OUTPUT FALSE
    # As in mnKord and mnArpee: hosts put an "Instrument" on an instrument track, where its MIDI
    # output feeds the next device.
    VST3_CATEGORIES Instrument
    PRODUCT_NAME "mnKarp"
    COPY_PLUGIN_AFTER_BUILD FALSE
)

juce_generate_juce_header(mnKarp)

# Montserrat (titles) and Roboto Condensed (UI text), SIL Open Font License; one copy for the rack and both plugins.
juce_add_binary_data(mnKarpFonts
    SOURCES
        Resources/Fonts/Montserrat.ttf
        Resources/Fonts/RobotoCondensed.ttf
)

target_sources(mnKarp PRIVATE
    Source/KarpProcessor.cpp
    Source/KarpEditor.cpp
    Source/Kord/PluginProcessor.cpp
    Source/Kord/PluginEditor.cpp
    Source/Kord/NoteName.cpp
    Source/Kord/ChordSymbol.cpp
    Source/Arpee/PatternData.cpp
    Source/Arpee/ArpEngine.cpp
    Source/Arpee/PresetLibrary.cpp
    Source/Arpee/PluginProcessor.cpp
    Source/Arpee/PluginEditor.cpp
    Source/Arpee/StepGrid.cpp
    Source/Arpee/Fonts.cpp)

target_include_directories(mnKarp PRIVATE
    Source
    "${CMAKE_CURRENT_BINARY_DIR}/generated")

target_compile_definitions(mnKarp
    PUBLIC
        JucePlugin_Name="mnKarp"
        MNKARP_VERSION_STRING="${PROJECT_VERSION}"
        MNARPEE_VERSION_STRING="${MNKARP_ARPEE_VERSION}"
        JUCE_VST3_CAN_REPLACE_VST2=0
        JUCE_WEB_BROWSER=0
        JUCE_USE_CURL=0
)

if(MSVC)
    target_compile_options(mnKarp PRIVATE /utf-8 /Zc:__cplusplus)
endif()

target_link_libraries(mnKarp
    PRIVATE
        mnKarpFonts
        juce::juce_audio_devices
        juce::juce_audio_utils
        juce::juce_gui_extra
    PUBLIC
        juce::juce_recommended_config_flags)

# The Windows installer is part of the normal build: "cmake --build build --config Release" makes the VST3,
# the Standalone and the installer (build/installer), when Inno Setup 6 is installed and the guides' PDFs exist
# (python docs/build-pdf.py).
if(WIN32)
    set(mnkarp_program_files_x86 "$ENV{ProgramFiles\(x86\)}")
    find_program(MNKARP_ISCC ISCC.exe
        HINTS "$ENV{LOCALAPPDATA}/Programs/Inno Setup 6" "$ENV{ProgramFiles}/Inno Setup 6"
              "${mnkarp_program_files_x86}/Inno Setup 6")
    if(MNKARP_ISCC)
        add_custom_target(mnKarpInstaller ALL
            COMMAND ${CMAKE_COMMAND} -DCONFIG=$<CONFIG> "-DISCC=${MNKARP_ISCC}" -DVERSION=${PROJECT_VERSION}
                    "-DBUNDLE=${CMAKE_BINARY_DIR}/mnKarp_artefacts/$<CONFIG>/VST3/mnKarp.vst3"
                    "-DOUTPUT=${CMAKE_BINARY_DIR}/installer"
                    "-DISS=${CMAKE_SOURCE_DIR}/installer/mnKarp.iss"
                    -P "${CMAKE_SOURCE_DIR}/installer/build-installer.cmake"
            COMMENT "Building the Windows installer"
            VERBATIM)
        add_dependencies(mnKarpInstaller mnKarp_VST3 mnKarp_Standalone)
    else()
        message(STATUS "Inno Setup 6 not found: the installer is not built")
    endif()
endif()

# Console tests (no host): the chain, the bypass and the project state.
#   cmake -S . -B build -DMNKARP_BUILD_TESTS=ON
#   cmake --build build --config Release --target mnKarpTests
#   build/Release/mnKarpTests.exe
option(MNKARP_BUILD_TESTS "Build the console tests (mnKarpTests)" OFF)
if(MNKARP_BUILD_TESTS)
    add_executable(mnKarpTests Tests/RackTests.cpp)
    target_include_directories(mnKarpTests PRIVATE
        Source
        "${CMAKE_CURRENT_BINARY_DIR}/generated"
        $<TARGET_PROPERTY:mnKarp,INCLUDE_DIRECTORIES>)
    target_compile_definitions(mnKarpTests PRIVATE $<TARGET_PROPERTY:mnKarp,COMPILE_DEFINITIONS>)
    if(MSVC)
        target_compile_options(mnKarpTests PRIVATE /utf-8 /Zc:__cplusplus)
        target_link_options(mnKarpTests PRIVATE /STACK:8388608)
    endif()
    target_link_libraries(mnKarpTests PRIVATE mnKarp)
endif()
