cmake_minimum_required(VERSION 3.22)

# The one place the version is set (Semantic Versioning, see AGENTS.md).
project(mnKarp VERSION 1.0.0)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

# mnKarp does not copy mnKord and mnArpee: it compiles their sources in place, so a fix in either
# plugin is in the rack with the next build.
set(MNKARP_KORD_DIR  "${CMAKE_CURRENT_SOURCE_DIR}/../mnKord"  CACHE PATH "The mnKord checkout")
set(MNKARP_ARPEE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../mnArpee" CACHE PATH "The mnArpee checkout")
set(MNKARP_JUCE_DIR  "${CMAKE_CURRENT_SOURCE_DIR}/../JUCE"    CACHE PATH "Local JUCE checkout")

foreach(_dir IN ITEMS "${MNKARP_KORD_DIR}/Source" "${MNKARP_ARPEE_DIR}/Source")
    if(NOT IS_DIRECTORY "${_dir}")
        message(FATAL_ERROR "${_dir} not found: set MNKARP_KORD_DIR and MNKARP_ARPEE_DIR")
    endif()
endforeach()

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

# The versions of the two plugins, read from their own CMakeLists.txt (shown in their title bars).
function(mnkarp_read_version file out)
    file(STRINGS "${file}" _line REGEX "^project\\(.*VERSION")
    if(_line MATCHES "VERSION ([0-9]+\\.[0-9]+\\.[0-9]+)")
        set(${out} "${CMAKE_MATCH_1}" PARENT_SCOPE)
    else()
        set(${out} "dev" PARENT_SCOPE)
    endif()
    set_property(DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS "${file}")
endfunction()
mnkarp_read_version("${MNKARP_KORD_DIR}/CMakeLists.txt" MNKARP_KORD_VERSION)
mnkarp_read_version("${MNKARP_ARPEE_DIR}/CMakeLists.txt" MNKARP_ARPEE_VERSION)
message(STATUS "mnKarp ${PROJECT_VERSION} = mnKord ${MNKARP_KORD_VERSION} + mnArpee ${MNKARP_ARPEE_VERSION}")

# mnKord's editor includes the generated MnKordVersion.h.
file(CONFIGURE OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/generated/MnKordVersion.h" @ONLY CONTENT
"#pragma once
#define MNKORD_VERSION        \"@MNKARP_KORD_VERSION@\"
#define MNKORD_VERSION_STRING \"@MNKARP_KORD_VERSION@\"
")

# Both plugins have a PluginProcessor.h, so the rack includes them as <mnKord/PluginProcessor.h> and
# <mnArpee/PluginProcessor.h>: these forwarding headers point at the real files.
function(mnkarp_forward_header plugin dir header)
    file(CONFIGURE OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/generated/${plugin}/${header}" @ONLY CONTENT
"#pragma once
#include \"@dir@/Source/@header@\"
")
endfunction()
mnkarp_forward_header(mnKord  "${MNKARP_KORD_DIR}"  PluginProcessor.h)
mnkarp_forward_header(mnKord  "${MNKARP_KORD_DIR}"  MnAudioUi.h)
mnkarp_forward_header(mnArpee "${MNKARP_ARPEE_DIR}" PluginProcessor.h)

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

# One copy of the fonts for all three (the two plugins ship the same files).
juce_add_binary_data(mnKarpFonts
    SOURCES
        "${MNKARP_KORD_DIR}/Resources/Fonts/Montserrat.ttf"
        "${MNKARP_KORD_DIR}/Resources/Fonts/RobotoCondensed.ttf"
)

set(MNKARP_KORD_SOURCES
    "${MNKARP_KORD_DIR}/Source/PluginProcessor.cpp"
    "${MNKARP_KORD_DIR}/Source/PluginEditor.cpp"
    "${MNKARP_KORD_DIR}/Source/NoteName.cpp"
    "${MNKARP_KORD_DIR}/Source/ChordSymbol.cpp")
set(MNKARP_ARPEE_SOURCES
    "${MNKARP_ARPEE_DIR}/Source/PatternData.cpp"
    "${MNKARP_ARPEE_DIR}/Source/ArpEngine.cpp"
    "${MNKARP_ARPEE_DIR}/Source/PresetLibrary.cpp"
    "${MNKARP_ARPEE_DIR}/Source/PluginProcessor.cpp"
    "${MNKARP_ARPEE_DIR}/Source/PluginEditor.cpp"
    "${MNKARP_ARPEE_DIR}/Source/StepGrid.cpp"
    "${MNKARP_ARPEE_DIR}/Source/Fonts.cpp")

# Each of the two plugin processors defines the host's entry point createPluginFilter(); the rack
# has its own (Source/KarpProcessor.cpp), so theirs are renamed.
set_source_files_properties("${MNKARP_KORD_DIR}/Source/PluginProcessor.cpp"
    PROPERTIES COMPILE_DEFINITIONS "createPluginFilter=mnKordEntryPointNotUsed")
set_source_files_properties("${MNKARP_ARPEE_DIR}/Source/PluginProcessor.cpp"
    PROPERTIES COMPILE_DEFINITIONS "createPluginFilter=mnArpeeEntryPointNotUsed")

target_sources(mnKarp PRIVATE
    Source/KarpProcessor.cpp
    Source/KarpEditor.cpp
    ${MNKARP_KORD_SOURCES}
    ${MNKARP_ARPEE_SOURCES})

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
