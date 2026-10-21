# Where the plugin code comes from

mnKarp has its own copies of the two plugins, so the repository builds on its own:

| Folder | Copied from | Version | Commit |
|--------|-------------|---------|--------|
| `Source/Kord` | mnKord (github.com/miklosnemeth/mnkord), `Source/` | 1.6.1 | `fc27613` |
| `Source/Arpee` | mnArpee (github.com/miklosnemeth/mnArpee), `Source/` | 1.0.0 | `c5ec3e1` |
| `Source/MnAudioUi.h` | the same file in both | | |

What changed in the copies:

- Both: the plugin entry point `createPluginFilter()` is removed (the rack has its own, in `KarpProcessor.cpp`).
- `Kord/PluginEditor.*`: one keyboard instead of the INPUT and CHORD keyboards. The trigger keys are chosen in a lane
  above the keys; the keys themselves edit the chord. Colours tell trigger keys, chord notes and playing notes apart.

To take a fix from mnKord or mnArpee, compare the copy with the plugin at the commit above and carry the change over by
hand; then update this table.
