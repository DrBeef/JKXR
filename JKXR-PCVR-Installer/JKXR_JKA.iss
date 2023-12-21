; Script generated by the Inno Setup Script Wizard.
; SEE THE DOCUMENTATION FOR DETAILS ON CREATING INNO SETUP SCRIPT FILES!

#define MyAppName "JKXR - Jedi Academy"
#define MyAppVersion "1.1.26"
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
Compression=zip
SolidCompression=yes
WizardStyle=modern
AlwaysShowDirOnReadyPage=yes
AppendDefaultDirName=no   
DisableWelcomePage=yes

[Languages]
Name: "english"; MessagesFile: "compiler:Default.isl"

[Tasks]
Name: "desktopicon"; Description: "{cm:CreateDesktopIcon}"; GroupDescription: "{cm:AdditionalIcons}"; Flags: unchecked

[Files]
Source: "C:\Dev\Quest\JKXR\JKXR-PCVR-Installer\JKA\{#MyAppExeName}"; DestDir: "{app}"; Flags: ignoreversion
Source: "C:\Dev\Quest\JKXR\JKXR-PCVR-Installer\JKA\jagamex86_64.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "C:\Dev\Quest\JKXR\JKXR-PCVR-Installer\JKA\OpenAL32.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "C:\Dev\Quest\JKXR\JKXR-PCVR-Installer\JKA\rdsp-vanilla_x86_64.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "C:\Dev\Quest\JKXR\JKXR-PCVR-Installer\packaged_mods_credits.txt"; DestDir: "{app}"; Flags: ignoreversion
Source: "C:\Dev\Quest\JKXR\JKXR-PCVR-Installer\JKA\SDL2.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "C:\Dev\Quest\JKXR\JKXR-PCVR-Installer\JKA\base\*"; DestDir: "{app}\base"; Flags: ignoreversion recursesubdirs createallsubdirs     
Source: "C:\Dev\Quest\JKXR\JKXR-PCVR-Installer\vr_splash.bmp"; Flags: dontcopy
; NOTE: Don't use "Flags: ignoreversion" on any shared system files

[Icons]
Name: "{autoprograms}\{#MyAppName}"; Filename: "{app}\{#MyAppExeName}"
Name: "{autodesktop}\{#MyAppName}"; Filename: "{app}\{#MyAppExeName}"; Tasks: desktopicon

[Run]
Filename: "{app}\{#MyAppExeName}"; Description: "{cm:LaunchProgram,{#StringChange(MyAppName, '&', '&&')}}"; Flags: nowait postinstall skipifsilent

[Code]
var
  Page: TWizardPage;

procedure InitializeWizard();
var
  BitmapImage: TBitmapImage;
  BitmapFileName: String;

begin
  Page := CreateCustomPage(wpWelcome, 'JKXR - Jedi Academy Installer', 'Join our Patreon at:   patreon.com/teambeef');
  BitmapFileName := ExpandConstant('{tmp}\vr_splash.bmp');
  ExtractTemporaryFile(ExtractFileName(BitmapFileName));
  BitmapImage := TBitmapImage.Create(Page);    
  BitmapImage.Bitmap.LoadFromFile(BitmapFileName); 
  BitmapImage.Center := True;
  BitmapImage.Stretch := True;
  BitmapImage.Parent := Page.Surface;
  with BitmapImage do begin
    Width := Page.SurfaceHeight + 10;
    Height := Page.SurfaceHeight + 10;
    Parent := Page.Surface;
  end;
end;

function NextButtonClick(CurPageID: Integer): Boolean;
begin
  { Validate certain pages before allowing the user to proceed }
  if CurPageID = Page.ID then begin
    SuppressibleMsgBox('If installing into the same location as an existing Jedi Academy install, please ensure you choose the directory that contains the single player executable and the "base" folder (probably called Jedi Academy\GameData)', mbConfirmation, MB_OK, IDOK);
  end;
  Result := True;
end;
