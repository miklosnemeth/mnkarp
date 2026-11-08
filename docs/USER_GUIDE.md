# mnKarp user guide

**English** · [Magyar](USER_GUIDE.hu.md)

mnKarp is a rack for two MIDI effects: it loads **mnKord** (one key plays a chord) and, under it, **mnArpee** (the notes are
played as a pattern), the plugins you have installed. The MIDI goes through mnKord first and then through mnArpee, and each of the two can be switched off.

```
MIDI in -> [ mnKord ] -> [ mnArpee ] -> MIDI out
              on/off        on/off
```

The two windows are the windows of mnKord and mnArpee themselves, so their user guides (the pattern grid, the preset browsers,
chord symbols, scales, Undo / Redo) apply here too. This guide is about what the rack adds.

## Installing

Run `mnKarp-<version>-win-x64-setup.exe`. It installs the VST3 to `Common Files\VST3\MNAudio\mnKarp.vst3`, which every DAW
scans, and this guide to `Program Files\MNAudio\mnKarp`. Close your DAW first if it has mnKarp loaded.

**mnKord and mnArpee must be installed too** (mnKord 1.7.0 or later, for its single keyboard): mnKarp loads them from the VST3
folder (`Common Files\VST3\MNAudio`, or anywhere in `Common Files\VST3`). When one of them is missing, its section says so and
the MIDI passes it; the installer warns too. An update of mnKord or mnArpee is an update of the rack as well.

## Using it in a DAW

Put mnKarp on an **instrument track**, as you would mnKord or mnArpee. It receives the MIDI of the track and sends its MIDI to
the next device, a synth or a sampler. It makes no sound itself.

## The window

![The mnKarp window](images/main.png)

From top to bottom:

- **The top bar.** The title and the version, then **Drag MIDI** and the size button at the right.
- **The mnKord section.** A strip with the power switch, the name and the fold arrow, and under it mnKord's window (its
  **TRIGGER** lane and keyboard are described in mnKord's guide).
- **The note line.** The notes that mnKord gives to mnArpee, live: when you press a key you see the chord here. It is a quick way to
  hear with your eyes what the first stage does.
- **The mnArpee section.** Its strip and mnArpee's window.

### Switching a stage off

The round **power switch** at the left of a strip switches the stage off or on. A stage that is off does nothing: the MIDI goes
through it untouched. The strip says "OFF - the MIDI passes through", and the window can still be edited.

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
follows it, and the other way round (mnKord's window takes any width from 848 to 1600 pixels at 100%). The size and the width are
kept with the project.

## Presets

The rack has no presets of its own. The presets are **mnKord's and mnArpee's**: choose, save and browse them in their windows, as in
the single plugins (the same libraries). A project saved with mnKarp keeps the full state of both stages and the two switches.

## Where the files are

| What | Where |
|------|-------|
| The plugin | `C:\Program Files\Common Files\VST3\MNAudio\mnKarp.vst3` |
| mnKord presets | `Documents\MNAudio\mnKord` (the same as mnKord's own) |
| mnArpee presets | `Documents\MNAudio\mnArpee` (the same as mnArpee's own) |

## Drag MIDI: the result as a clip

mnKarp keeps what it has sent out in the last 10 minutes. **Drag MIDI** in the top bar hands the **last take** to the DAW: drag the button
onto a track (or the arranger) and the notes land there as a MIDI clip. Click it instead to save the take as a `.mid` file.

- The take is the latest stretch of playing: it starts with the first note after at least 2 seconds in which nothing sounded (or where
  the DAW's loop jumped back) and runs to the last note.
- While the DAW's transport runs, the clip starts on the bar of its first note, so it lands in time when you drop it on that bar. Without
  the transport it starts with its first note.
- The tempo it was played at is in the file; 960 ticks per quarter note.
- The button is grey while there is nothing to drag.

## In the DAW: automation

**Parameters.** The host sees these parameters of the rack, and you can automate, record and map them to a controller:

| Parameter | What it is |
|-----------|------------|
| `mnKord On`, `mnArpee On` | the two power switches |
| `mnArpee Rate`, `Octave`, `Gate %`, `Swing %`, `Steps` | the values of mnArpee's header |
| `mnArpee Latch`, `Group octaves`, `Cut on release` | mnArpee's three switches |

They are the very controls of mnArpee's window: move one in the window and the host records it, automate one in the host and the
control follows. mnKord has no parameters of its own.

The order of the stages is fixed: mnKord first, then mnArpee.

## Questions

**Nothing is played when I press a key.** Check that the track is an instrument track and that the synth after mnKarp gets MIDI from it.
Check the two switches, and that mnKord has a mapping for the key (see its guide).

**The rack is taller than my screen.** Choose a smaller size with the size button, or fold the window you are not using.

**I switched a stage off while holding a key, and now the key is silent.** The held key was released with the stage. Press it again.

**Does it replace mnKord and mnArpee?** No: mnKarp loads them, so they must be installed. If you want only a chord mapper or only
an arpeggiator, use them alone.

## License

mnKarp is free software under the AGPL-3.0, like mnKord and mnArpee. Vibecoded by Miklós Németh.
