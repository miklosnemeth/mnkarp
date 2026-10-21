# mnKarp user guide

**English** · [Magyar](USER_GUIDE.hu.md)

mnKarp is a rack of two MIDI effects in one plugin: **mnKord** (one key plays a chord) above **mnArpee** (the notes are
played as a pattern). The MIDI goes through mnKord first and then through mnArpee, and each of the two can be switched off.

```
MIDI in -> [ mnKord ] -> [ mnArpee ] -> MIDI out
              on/off        on/off
```

The two windows are the real mnKord and mnArpee, so everything in their own user guides (chord mapping, the pattern
grid, the preset browsers, Undo / Redo) works here the same way. This guide is about what the rack adds.

## Installing

Run `mnKarp-<version>-win-x64-setup.exe`. It installs the VST3 to `Common Files\VST3\MNAudio\mnKarp.vst3`, which every DAW
scans, and this guide to `Program Files\MNAudio\mnKarp`. Close your DAW first if it has mnKarp loaded. mnKord and mnArpee
do not have to be installed: mnKarp has both inside. If you have them, the three work side by side and share their preset
libraries.

## Using it in a DAW

Put mnKarp on an **instrument track**, as you would mnKord or mnArpee. It receives the MIDI of the track and sends its MIDI to
the next device, a synth or a sampler. It makes no sound itself.

## The window

![The mnKarp window](images/main.png)

From top to bottom:

- **The top bar.** The title and the version, the rack presets in the middle, and the size button at the right.
- **The mnKord section.** A strip with the power switch, the name and the fold arrow, and under it mnKord's window.
- **The note line.** The notes that mnKord gives to mnArpee, live: when you press a key you see the chord here. It is a quick way to
  hear with your eyes what the first stage does.
- **The mnArpee section.** Its strip and mnArpee's window.

### Switching a stage off

The round **power switch** at the left of a strip switches the stage off or on. A stage that is off does nothing: the MIDI goes
through it untouched. Its window is dimmed, with the words "OFF - the MIDI passes through" in the strip, and can still be edited.

| mnKord | mnArpee | What you get |
|--------|---------|--------------|
| on | on | a key plays a chord, and the chord is played as a pattern |
| on | off | a key plays a chord |
| off | on | the keys you hold are played as a pattern |
| off | off | the MIDI goes through as it is |

The switches are host parameters named `mnKord On` and `mnArpee On`, so you can automate them or put them on a controller.
When a stage is switched off, what it is sounding (the chord, the arpeggio) is released, so no note hangs.

### Folding a window

The arrow at the right end of a strip hides the stage's window (and shows it again). It does not change the sound. It is useful
to make the rack shorter while you work on the other stage. The project remembers what is folded.

### Size and width

The **size button** in the top bar (75% to start with) sets the size of both windows together, from 60% to 115%. At 75% the rack is
about 950 pixels high; if it does not fit your screen, choose a smaller size.

The two windows are **always equally wide**. mnArpee's window can be resized with the corner at its bottom right, and mnKord's window
follows it, and the other way round. The size and the width are kept with the project.

## Rack presets

A rack preset holds the **current presets of mnKord and mnArpee** and the **two switches** under one name, so a whole sound
("Pop chords with a 16th arp") comes back with one click. It does not hold the size of the window.

In the top bar:

- the arrows step to the previous and the next preset;
- the name opens the menu: the list of the presets (the current one is ticked), **Save**, **Save as...**, **Delete** and
  **Show the preset folder**;
- **Save** writes the current preset; when the project has none yet, it asks for a name. A name that is taken asks before it replaces it.

The presets are files (`.mnkarp`) in `Documents\MNAudio\mnKarp\Presets`, shared by every project and every instance of mnKarp. Copy
them to another computer to take them with you. A rack preset stores the *content* of the two presets, so it plays the same even
if you later edit the preset in the mnKord or mnArpee library. Delete moves the file to the recycle bin.

A project saved with mnKarp keeps the full state of both stages, whether or not a rack preset is selected.

## Where the files are

| What | Where |
|------|-------|
| The plugin | `C:\Program Files\Common Files\VST3\MNAudio\mnKarp.vst3` |
| Rack presets | `Documents\MNAudio\mnKarp\Presets` |
| mnKord presets | `Documents\MNAudio\mnKord` (the same as mnKord's own) |
| mnArpee presets | `Documents\MNAudio\mnArpee` (the same as mnArpee's own) |

## In the DAW: automation and the program list

**Parameters.** The host sees these parameters of the rack, and you can automate, record and map them to a controller:

| Parameter | What it is |
|-----------|------------|
| `mnKord On`, `mnArpee On` | the two power switches |
| `mnArpee Rate`, `Octave`, `Gate %`, `Swing %`, `Steps` | the values of mnArpee's header |
| `mnArpee Latch`, `Group octaves`, `Cut on release` | mnArpee's three switches |

They are the very controls of mnArpee's window: move one in the window and the host records it, automate one in the host and the
control follows. mnKord has no parameters of its own.

**Presets from the DAW.** The host's preset (program) list is the list of the **rack presets**, oldest first, so a new preset goes to
the end and the numbers a DAW stores keep their meaning. There are always 128 slots; the empty ones are called "-". Choosing a
program in the DAW loads that rack preset, as the name menu does. A preset file copied into the folder shows up when the menu is
opened (or when the DAW asks again).

The order of the stages is fixed: mnKord first, then mnArpee.

## Questions

**Nothing is played when I press a key.** Check that the track is an instrument track and that the synth after mnKarp gets MIDI from it.
Check the two switches, and that mnKord has a mapping for the key (see its guide).

**The rack is taller than my screen.** Choose a smaller size with the size button, or fold the window you are not using.

**I switched a stage off while holding a key, and now the key is silent.** The held key was released with the stage. Press it again.

**Does it replace mnKord and mnArpee?** No: they stay as they are, and mnKarp uses the same code. If you want only a chord
mapper or only an arpeggiator, use them alone.

## License

mnKarp is free software under the AGPL-3.0, like mnKord and mnArpee. Vibecoded by Miklós Németh.
