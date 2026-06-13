; =====================================================================
;  Talay - Inno Setup script
;  Builds a self-contained classic installer (setup.exe) from the
;  Release|x64 self-contained build output.
;
;  Prerequisites, in order:
;    1. Build ->  MSBuild Talay.sln /t:Build /p:Configuration=Release /p:Platform=x64
;                 (the Release build self-trims to a runtime-only OutDir via the
;                  TrimReleaseOutput target in Talay.vcxproj; the .pdb is emitted
;                  to x64\Release\Symbols, not the deploy folder)
;    2. Pack  ->  ISCC.exe Talay.iss
;  Output: installer\Talay-Setup-<version>.exe
; =====================================================================

#define MyAppName "Talay"
#define MyAppVersion "1.0.0"
#define MyAppPublisher "kurtc"
#define MyAppExeName "Talay.exe"
#define SourceDir "x64\Release\Talay"

[Setup]
; Stable identity (reused from Package.appxmanifest) so upgrades/uninstall match.
AppId={{7812be43-7c1f-4760-acbb-403e16b41839}
AppName={#MyAppName}
AppVersion={#MyAppVersion}
AppPublisher={#MyAppPublisher}
DefaultDirName={autopf}\{#MyAppName}
DefaultGroupName={#MyAppName}
UninstallDisplayIcon={app}\{#MyAppExeName}
DisableProgramGroupPage=yes
; Self-contained x64 payload.
ArchitecturesAllowed=x64compatible
ArchitecturesInstallIn64BitMode=x64compatible
Compression=lzma2/max
SolidCompression=yes
WizardStyle=modern
; Icon shown for setup.exe itself and the uninstaller.
SetupIconFile=Assets\talay-icon.ico
OutputDir=installer
OutputBaseFilename=Talay-Setup-{#MyAppVersion}

[Languages]
Name: "english"; MessagesFile: "compiler:Default.isl"
Name: "turkish"; MessagesFile: "compiler:Languages\Turkish.isl"

[Tasks]
Name: "desktopicon"; Description: "{cm:CreateDesktopIcon}"; GroupDescription: "{cm:AdditionalIcons}"; Flags: unchecked

[Files]
; The Release build already self-trims to a runtime-only set (TrimReleaseOutput
; in Talay.vcxproj). The Excludes are defence-in-depth: the dev/metadata files
; in case that target is ever disabled, and settings.ini / *.library.db so
; per-user data can never ship if the app was ever run from the build folder.
Source: "{#SourceDir}\*"; DestDir: "{app}"; Flags: recursesubdirs createallsubdirs ignoreversion; Excludes: "*.pdb,*.lib,*.exp,*.winmd,Microsoft.UI.Designer.dll,settings.ini,*.library.db"

[Icons]
Name: "{group}\{#MyAppName}"; Filename: "{app}\{#MyAppExeName}"
Name: "{autodesktop}\{#MyAppName}"; Filename: "{app}\{#MyAppExeName}"; Tasks: desktopicon

[Run]
; Shell integration is registered by the app itself, per user under HKCU —
; runasoriginaluser makes the write land in the logged-in user's hive, not the
; elevated admin's. --register-shell lists every supported image type under
; "Open with" AND adds the "Convert with Talay" context-menu verb; the verb is
; also self-healed on every normal start, so the extension list lives only in
; C++ (talay::Shell::OShellIntegration).
Filename: "{app}\{#MyAppExeName}"; Parameters: "--register-shell"; Flags: runhidden waituntilterminated runasoriginaluser
Filename: "{app}\{#MyAppExeName}"; Description: "{cm:LaunchProgram,{#StringChange(MyAppName, '&', '&&')}}"; Flags: nowait postinstall skipifsilent

[UninstallRun]
; Mirror of the [Run] hook: removes the context-menu verb (both hives, best
; effort) and the per-user "Open with" registration before the files go.
Filename: "{app}\{#MyAppExeName}"; Parameters: "--unregister-shell"; Flags: runhidden waituntilterminated; RunOnceId: "TalayUnregisterShell"
