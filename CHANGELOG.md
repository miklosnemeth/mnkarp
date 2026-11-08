# Changelog

All notable changes to mnKarp. The format follows [Keep a Changelog](https://keepachangelog.com/en/1.1.0/),
and the version numbers follow [Semantic Versioning](https://semver.org/).

## [Unreleased] - 2.0.0

### Changed
- mnKarp is now a rack that loads the **installed** mnKord and mnArpee plugins (`mnKord.vst3`, 1.7.0 or later, and `mnArpee.vst3`
  from the VST3 folder) instead of having its own copies of them. The windows, presets and libraries are the plugins' own, and an
  update of mnKord or mnArpee is an update of the rack too. mnKord's window is the new one-keyboard window of mnKord 1.7.0.
- When one of the two is not installed, its section says so and the MIDI passes it; the installer warns too.
- The two windows are still always equally wide: mnKord's window now takes any width, like mnArpee's.
- A switched-off stage is no longer dimmed (the plugin windows are windows of their own); the strip still says it is off.
- The rack's menus open as windows of their own.

### Removed
- The rack presets: the presets are mnKord's and mnArpee's own, chosen in their windows (the project still keeps the state of
  both and the switches). The top bar has no preset picker, and the host's program list is no longer the rack presets (the
  rack has a single program). The installer deletes the rack preset files of 1.0 (`Documents\MNAudio\mnKarp\Presets`).
- Projects of 1.0 keep their switches, size and folding, but not the state of mnKord and mnArpee in them
  (it was the state of the built-in copies, which the installed plugins cannot read): they start with their current presets.

## [1.0.0] - 2026-10-21

### Added
- The first version: mnKord and mnArpee in one plugin, one above the other, MIDI going through mnKord and then mnArpee.
- A power switch for each of the two (a host parameter), a fold arrow to hide a window, and a dimmed look for a switched-off one.
- Switching a section off releases the notes it is sounding.
- The two windows are shown at 75% by default (so the rack is shorter); the size button in the top bar sets both (60% to 115%) and the project keeps it.
- The two windows are always equally wide: resizing mnArpee (its corner) resizes mnKord's window to match, and the project keeps the width.
- Rack presets: the presets of mnKord and mnArpee and the two switches saved under one name (top bar: previous / next, the name menu, Save). They are files in `Documents\MNAudio\mnKarp\Presets`, shared by every project.
- Between the two windows a line shows the notes mnKord gives to mnArpee, live.
- A Windows installer and a user guide in English and Hungarian (PDF).
- mnArpee's parameters (rate, octave, gate, swing, steps, latch, group, cut) are host parameters of the rack, so they can be automated, recorded and mapped; they follow the controls of mnArpee's window.
- The host's preset (program) list is the list of the rack presets (128 slots, oldest first): choosing a program in the DAW loads that rack preset.
- Drag MIDI: drag the last take (what the rack played since the last pause) into the DAW as a MIDI clip, starting on its bar and with its tempo, or click to save it as a file.
- A dot by the rack preset's name marks unsaved changes; choosing another preset asks to save them first. The menu has Revert to saved, and Delete asks first.
- The questions (the name of a preset, replacing or deleting one) open inside the plugin window, in its own look.
- A home page in English and Hungarian.
- mnKord has one keyboard in the rack: the trigger keys are chosen in a lane above the keys, the chord notes are clicked on the same keyboard, and colours tell trigger keys (red), chord notes (green), played keys and sounding notes apart. The rack is shorter for it.
- mnKarp builds and installs on its own, with its own copies of mnKord and mnArpee.
