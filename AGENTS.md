# Instructions for AI agents working on mnKarp

mnKarp is the rack of mnKord and mnArpee: a VST3 host that loads the installed `mnKord.vst3` and `mnArpee.vst3`. The rules of mnKord's `AGENTS.md` apply here too: C++23, Semantic Versioning in **one** place (`project(mnKarp VERSION x.y.z)` in
`CMakeLists.txt`), and every change adds a line to `CHANGELOG.md` under `## [Unreleased] - x.y.z` (the same number),
written for the plugin's user. A MAJOR bump needs the owner's say-so.

## Build and test

```bash
cmake -S . -B build -DMNKARP_BUILD_TESTS=ON
cmake --build build --config Release
build/Release/mnKarpTests.exe        # must end with 0 failures
```

## Rules

- mnKarp has **no copy** of mnKord or mnArpee: a change of one of them is made in its own repository, and the rack gets it when
  the plugin is installed (or built: the tests load `../mnKord/build/...` and `../mnArpee/build/...`, see `CMakeLists.txt`).
- The rack talks to the two plugins only as a host does (`juce::AudioPluginInstance`: processBlock, state, parameters, programs,
  `createEditorIfNeeded()`). mnArpee's parameters are found by their names; keep the `arp_...` IDs of the mirrors, hosts store them.
- The plugin windows are native windows: nothing of the rack can be drawn over them (menus open as windows of their own, a dialog
  hides them).
- `Source/MnAudioUi.h` is the shared MNAudio look (the same file as in mnKord and mnArpee); `installer/MnAudioInstaller.iss`
  and `site/site.css` are copies of mnKord's too.
- The two plugin editors must never be included in one translation unit (they have global names that clash); the rack
  creates them through the processors' `createEditor()`.

## Documentation and installer

- The user guide is `docs/USER_GUIDE.md` (English) and `docs/USER_GUIDE.hu.md` (Hungarian); update **both** when a control or
  behaviour changes. The screenshot `docs/images/main.png` is a capture of the Standalone's window (the plugin windows cannot be rendered offscreen).
- The PDFs (`docs/mnKarp-User-Guide.pdf`, `docs/mnKarp-Felhasznaloi-utmutato.pdf`) go into the installer: run `python docs/build-pdf.py`
  after changing a guide (it uses `docs/pdf_converter.py`, mnKord's converter, and needs Edge or Chrome) and commit them.
- The web pages `docs/index.html`, `docs/hu/index.html` and `docs/sitemap.xml` are generated: edit `site/build-site.py` and run
  `python site/build-site.py` (it reads the version from `CMakeLists.txt` and the stylesheet from `site/site.css`).
- The installer (`installer/mnKarp.iss`) is built by the normal Release build when Inno Setup 6 is installed; it includes `installer/MnAudioInstaller.iss`.
