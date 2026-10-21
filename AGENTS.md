# Instructions for AI agents working on mnKarp

mnKarp is the rack of mnKord and mnArpee, with its own copies of both. The rules of mnKord's `AGENTS.md` apply here too: C++23, Semantic Versioning in **one** place (`project(mnKarp VERSION x.y.z)` in
`CMakeLists.txt`), and every change adds a line to `CHANGELOG.md` under `## [Unreleased] - x.y.z` (the same number),
written for the plugin's user. A MAJOR bump needs the owner's say-so.

## Build and test

```bash
cmake -S . -B build -DMNKARP_BUILD_TESTS=ON
cmake --build build --config Release
build/Release/mnKarpTests.exe        # must end with 0 failures
```

## Rules

- mnKarp builds on its own: `Source/Kord` and `Source/Arpee` are **copies** of mnKord and mnArpee, changed for the rack
  (see `Source/ORIGIN.md`, which says from which commit and what changed). Edit them here freely; to take a fix from the
  single plugins, carry it over by hand and update `Source/ORIGIN.md`.
- The rack talks to the two processors through their public interface and `createEditorIfNeeded()`.
- `Source/MnAudioUi.h` is the shared MNAudio look (the same file as in mnKord and mnArpee); `installer/MnAudioInstaller.iss`
  and `site/site.css` are copies of mnKord's too.
- The two plugin editors must never be included in one translation unit (they have global names that clash); the rack
  creates them through the processors' `createEditor()`.

## Documentation and installer

- The user guide is `docs/USER_GUIDE.md` (English) and `docs/USER_GUIDE.hu.md` (Hungarian); update **both** when a control or
  behaviour changes. The screenshot `docs/images/main.png` is made by `build/Release/mnKarpTests.exe --screenshot docs/images/main.png`.
- The PDFs (`docs/mnKarp-User-Guide.pdf`, `docs/mnKarp-Felhasznaloi-utmutato.pdf`) go into the installer: run `python docs/build-pdf.py`
  after changing a guide (it uses `docs/pdf_converter.py`, mnKord's converter, and needs Edge or Chrome) and commit them.
- The web pages `docs/index.html`, `docs/hu/index.html` and `docs/sitemap.xml` are generated: edit `site/build-site.py` and run
  `python site/build-site.py` (it reads the version from `CMakeLists.txt` and the stylesheet from `site/site.css`).
- The installer (`installer/mnKarp.iss`) is built by the normal Release build when Inno Setup 6 is installed; it includes `installer/MnAudioInstaller.iss`.
