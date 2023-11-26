; Script generated by the Inno Setup Script Wizard.
; SEE THE DOCUMENTATION FOR DETAILS ON CREATING INNO SETUP SCRIPT FILES!

#define MyAppName "JKXR - Jedi Academy"
#define MyAppVersion "1.0.20"
#define MyAppPublisher "Team Beef VR"
#define MyAppURL "https://www.patreon.com/teambeef"
#define MyAppExeName "openjk_sp.x86_64.exe"

[Setup]
; NOTE: The value of AppId uniquely identifies this application. Do not use the same AppId value in installers for other applications.
; (To generate a new GUID, click Tools | Generate GUID inside the IDE.)
AppId={{330568D5-5C62-44D3-8C54-5D51FFEEDFFC}
AppName={#MyAppName}
AppVersion={#MyAppVersion}
;AppVerName={#MyAppName} {#MyAppVersion}
AppPublisher={#MyAppPublisher}
AppPublisherURL={#MyAppURL}
AppSupportURL={#MyAppURL}
AppUpdatesURL={#MyAppURL}
DefaultDirName={autopf}\Steam\steamapps\common\Jedi Academy\GameData\
DisableProgramGroupPage=yes
; Uncomment the following line to run in non administrative install mode (install for current user only.)
;PrivilegesRequired=lowest
OutputDir=C:\Dev\Quest\JKXR\JKXR-PCVR-Installer
OutputBaseFilename=JKXR_JKA_Setup
Compression=lzma
SolidCompression=yes
WizardStyle=modern
AlwaysShowDirOnReadyPage=yes
AppendDefaultDirName=no

[Languages]
Name: "english"; MessagesFile: "compiler:Default.isl"

[Tasks]
Name: "desktopicon"; Description: "{cm:CreateDesktopIcon}"; GroupDescription: "{cm:AdditionalIcons}"; Flags: unchecked

[Files]
Source: "C:\Dev\Quest\JKXR\JKXR-PCVR-Installer\JKA\{#MyAppExeName}"; DestDir: "{app}"; Flags: ignoreversion
Source: "C:\Dev\Quest\JKXR\JKXR-PCVR-Installer\JKA\jagamex86_64.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "C:\Dev\Quest\JKXR\JKXR-PCVR-Installer\JKA\OpenAL32.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "C:\Dev\Quest\JKXR\JKXR-PCVR-Installer\JKA\rdsp-vanilla_x86_64.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "C:\Dev\Quest\JKXR\JKXR-PCVR-Installer\JKA\SDL2.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "C:\Dev\Quest\JKXR\JKXR-PCVR-Installer\JKA\base\*"; DestDir: "{app}\base"; Flags: ignoreversion recursesubdirs createallsubdirs
; NOTE: Don't use "Flags: ignoreversion" on any shared system files

[Icons]
Name: "{autoprograms}\{#MyAppName}"; Filename: "{app}\{#MyAppExeName}"
Name: "{autodesktop}\{#MyAppName}"; Filename: "{app}\{#MyAppExeName}"; Tasks: desktopicon

[Run]
Filename: "{app}\{#MyAppExeName}"; Description: "{cm:LaunchProgram,{#StringChange(MyAppName, '&', '&&')}}"; Flags: nowait postinstall skipifsilent

