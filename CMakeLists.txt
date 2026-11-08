cmake_minimum_required(VERSION 3.22)

# The one place the version is set (Semantic Versioning, see AGENTS.md).
project(mnKarp VERSION 2.0.0)

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

# mnKarp hosts the installed mnKord.vst3 and mnArpee.vst3 (see MnKarpAudioProcessor::findPlugin()); it has none of
# their code.

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

# Montserrat (titles) and Roboto Condensed (UI text), SIL Open Font License, for the rack's own bar and strips.
juce_add_binary_data(mnKarpFonts
    SOURCES
        Resources/Fonts/Montserrat.ttf
        Resources/Fonts/RobotoCondensed.ttf
)

target_sources(mnKarp PRIVATE
    Source/KarpProcessor.cpp
    Source/KarpEditor.cpp)

target_include_directories(mnKarp PRIVATE Source)

target_compile_definitions(mnKarp
    PUBLIC
        JucePlugin_Name="mnKarp"
        MNKARP_VERSION_STRING="${PROJECT_VERSION}"
        JUCE_PLUGINHOST_VST3=1
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
        juce::juce_audio_processors
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

# Console tests (no host): the chain, the bypass and the project state, with the real plugins loaded.
#   cmake -S . -B build -DMNKARP_BUILD_TESTS=ON
#   cmake --build build --config Release --target mnKarpTests
#   build/Release/mnKarpTests.exe
# They load the plugins built next to this folder (../mnKord/build, ../mnArpee/build, Release) when those exist, else
# the installed ones; MNKARP_MNKORD_VST3 and MNKARP_MNARPEE_VST3 in the environment name others.
option(MNKARP_BUILD_TESTS "Build the console tests (mnKarpTests)" OFF)
if(MNKARP_BUILD_TESTS)
    set(MNKARP_TEST_KORD_VST3 "${CMAKE_CURRENT_SOURCE_DIR}/../mnKord/build/mnKord_artefacts/Release/VST3/mnKord.vst3"
        CACHE FILEPATH "The mnKord.vst3 the tests load when it exists")
    set(MNKARP_TEST_ARPEE_VST3 "${CMAKE_CURRENT_SOURCE_DIR}/../mnArpee/build/mnArpee_artefacts/Release/VST3/mnArpee.vst3"
        CACHE FILEPATH "The mnArpee.vst3 the tests load when it exists")

    add_executable(mnKarpTests Tests/RackTests.cpp)
    target_include_directories(mnKarpTests PRIVATE
        Source
        $<TARGET_PROPERTY:mnKarp,INCLUDE_DIRECTORIES>)
    target_compile_definitions(mnKarpTests PRIVATE $<TARGET_PROPERTY:mnKarp,COMPILE_DEFINITIONS>
        MNKARP_TEST_KORD_VST3="${MNKARP_TEST_KORD_VST3}"
        MNKARP_TEST_ARPEE_VST3="${MNKARP_TEST_ARPEE_VST3}")
    if(MSVC)
        target_compile_options(mnKarpTests PRIVATE /utf-8 /Zc:__cplusplus)
        target_link_options(mnKarpTests PRIVATE /STACK:8388608)
    endif()
    target_link_libraries(mnKarpTests PRIVATE mnKarp)
endif()
