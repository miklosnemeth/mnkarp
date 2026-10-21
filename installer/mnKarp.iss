; Windows installer for the mnKarp VST3 (the Standalone is not included).
; It is built by the normal Release build (the CMake target mnKarpInstaller, when Inno Setup 6
; is installed). By hand, from a Release build, giving the version on the command line:
;   ISCC.exe /DMyAppVersion=1.0.0 installer\mnKarp.iss
; The result is build\installer\mnKarp-<version>-win-x64-setup.exe.
; What the MNAudio installers share (the folders, closing the DAW, the old version) is in
; MnAudioInstaller.iss, the same file as in mnKord and mnArpee.

#ifndef MyAppVersion
  #error Pass the version: ISCC.exe /DMyAppVersion=x.y.z installer\mnKarp.iss
#endif

; The built VST3 bundle and the output folder; CMake passes them, these are the defaults.
#ifndef Vst3Bundle
  #define Vst3Bundle "..\build\mnKarp_artefacts\Release\VST3\mnKarp.vst3"
#endif
#ifndef InstallerOutputDir
  #define InstallerOutputDir "..\build\installer"
#endif

#define MyAppName "mnKarp"
#define MyAppGuid "D41C8E27-6A93-4B05-9F1D-3E7A5C20B8F6"
#define MyRepo "mnkarp"
#define MyDocsLabel "User guide and licenses"
#include "MnAudioInstaller.iss"

[Setup]
; The same GUID as MyAppGuid ({{ is the escape of a single brace). The old uninstaller is found with it.
AppId={{{#MyAppGuid}}
AppName={#MyAppName}
AppVersion={#MyAppVersion}
OutputDir={#InstallerOutputDir}

[Files]
; The plugin goes to the standard VST3 folder every host scans, in a folder of its own for MNAudio.
Source: "{#Vst3Bundle}\*"; DestDir: "{#MyVst3Dest}"; Flags: ignoreversion recursesubdirs createallsubdirs
; Licenses and the readme sit in the install folder.
Source: "..\LICENSE"; DestDir: "{app}"; Flags: ignoreversion
Source: "..\README.md"; DestDir: "{app}"; Flags: ignoreversion
Source: "..\CHANGELOG.md"; DestDir: "{app}"; Flags: ignoreversion
Source: "..\Resources\Fonts\Montserrat-OFL.txt"; DestDir: "{app}\Licenses"; Flags: ignoreversion
Source: "..\Resources\Fonts\RobotoCondensed-LICENSE.txt"; DestDir: "{app}\Licenses"; Flags: ignoreversion
; The user guide (English and Hungarian), with Start menu shortcuts.
Source: "..\docs\mnKarp-User-Guide.pdf"; DestDir: "{app}\Docs"; Flags: ignoreversion
Source: "..\docs\mnKarp-Felhasznaloi-utmutato.pdf"; DestDir: "{app}\Docs"; Flags: ignoreversion

[Icons]
Name: "{autoprograms}\MNAudio\mnKarp user guide"; Filename: "{app}\Docs\mnKarp-User-Guide.pdf"
Name: "{autoprograms}\MNAudio\mnKarp felhasznaloi utmutato"; Filename: "{app}\Docs\mnKarp-Felhasznaloi-utmutato.pdf"
