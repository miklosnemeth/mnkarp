# mnKarp

**mnKarp** is a rack of two MIDI effects in one plugin: **mnKord** (a key plays a chord) above **mnArpee**
(the chord is played as a pattern). Each of the two can be switched off, and then the MIDI passes it untouched.
Built with [JUCE](https://juce.com) 9 as **VST3** and **Standalone**.

```
MIDI in -> [ mnKord ] -> [ mnArpee ] -> MIDI out
              on/off        on/off
```

## Using it

- Put mnKarp on an instrument track, like mnKord or mnArpee: its MIDI output feeds the next device.
- Each section has a round **power switch** at the left of its strip. It is a host parameter (`mnKord On`,
  `mnArpee On`), so it can be automated or mapped to a controller. A switched-off section is dimmed and can still be edited.
- Switching a section off releases what it is sounding (the chord, the arpeggio), so no note hangs.
- The top bar has the **rack presets** (previous / next, the name menu with Save as, Delete and Show the preset folder, Save) and
  the size button (75% to start with), which sets the size of both windows, 60% to 115%; it is kept with the project.
  A rack preset holds the current presets of mnKord and mnArpee and the two switches (not the size); the files are in
  `Documents\MNAudio\mnKarp\Presets`.
- The line between the two windows shows the notes that mnKord gives to mnArpee, live.
- The two windows are always equally wide: resize one (mnArpee has a resize corner) and the other follows.
- The arrow at the right of a strip **folds** the section's window away, to make the rack shorter.
- The windows are the plugins' own: presets, the preset libraries (`Documents\MNAudio\mnKord`, `...\mnArpee`),
  Undo / Redo and mnArpee's zoom and resize corner work as in the single plugins. The project keeps the state of both.

In the host: the two switches and mnArpee's parameters (rate, octave, gate, swing, steps, latch, group, cut) are host parameters
(`mnKord On`, `mnArpee On`, `mnArpee ...`), and the host's program list is the list of the rack presets (128 slots, oldest first).
The order of the stages is fixed.

## How it is built

mnKarp does not copy the two plugins: its `CMakeLists.txt` compiles their sources in place
(`../mnKord`, `../mnArpee`; set `MNKARP_KORD_DIR` and `MNKARP_ARPEE_DIR` if they are elsewhere), so a fix in either one is in
the rack with the next build. `Source/KarpProcessor.*` is the rack (it owns one `MnKordAudioProcessor` and one
`MnArpeeAudioProcessor` and runs the MIDI through them), `Source/KarpEditor.*` the two windows one above the other.

## Build (Windows)

```powershell
cmake -S . -B build -G "Visual Studio 18 2026" -A x64
cmake --build build --config Release
```

The VST3 is `build\mnKarp_artefacts\Release\VST3\mnKarp.vst3`, the standalone `build\mnKarp_artefacts\Release\Standalone\mnKarp.exe`.
JUCE is taken from `../JUCE` when it is there, otherwise fetched (9.0.1).

Tests (the chain, the switches, releasing notes, the project state, the editor), without a host:

```powershell
cmake -S . -B build -DMNKARP_BUILD_TESTS=ON
cmake --build build --config Release --target mnKarpTests
build\Release\mnKarpTests.exe
```
