#define MyAppName "SAIL"
#define MyAppPublisher "Dmitry Baryshev"
#define MyAppURL "https://github.com/smoked-herring/sail"
#define MyAppVersion "0.9.0@SAIL_DIST_VERSION_SUFFIX@"

[Setup]
AppId={{62D4D6DD-890E-43CE-9CC9-FB6DE5023DE9}
AppName={#MyAppName}
AppVersion={#MyAppVersion}
AppPublisher={#MyAppPublisher}
AppPublisherURL={#MyAppURL}
AppSupportURL={#MyAppURL}
AppUpdatesURL={#MyAppURL}/releases
DefaultDirName={sd}\{#MyAppName}
DefaultGroupName={#MyAppName}
LicenseFile=LICENSE.txt
OutputDir=.
OutputBaseFilename=sail-setup-{#MyAppVersion}
;SetupIconFile=sail.ico
Compression=lzma
SolidCompression=yes
UninstallDisplayName={#MyAppName}
;UninstallDisplayIcon={app}\.exe
MinVersion=0,6.0
ArchitecturesAllowed=x64
ArchitecturesInstallIn64BitMode=x64

[Languages]
Name: "AAAenglish"; MessagesFile: "compiler:Default.isl"
Name: "Armenian"; MessagesFile: "compiler:Languages\Armenian.isl"
Name: "BrazilianPortuguese"; MessagesFile: "compiler:Languages\BrazilianPortuguese.isl"
Name: "Catalan"; MessagesFile: "compiler:Languages\Catalan.isl"
Name: "Corsican"; MessagesFile: "compiler:Languages\Corsican.isl"
Name: "Czech"; MessagesFile: "compiler:Languages\Czech.isl"
Name: "Danish"; MessagesFile: "compiler:Languages\Danish.isl"
Name: "Dutch"; MessagesFile: "compiler:Languages\Dutch.isl"
Name: "Finnish"; MessagesFile: "compiler:Languages\Finnish.isl"
Name: "French"; MessagesFile: "compiler:Languages\French.isl"
Name: "German"; MessagesFile: "compiler:Languages\German.isl"
Name: "Hebrew"; MessagesFile: "compiler:Languages\Hebrew.isl"
Name: "Icelandic"; MessagesFile: "compiler:Languages\Icelandic.isl"
Name: "Italian"; MessagesFile: "compiler:Languages\Italian.isl"
Name: "Japanese"; MessagesFile: "compiler:Languages\Japanese.isl"
Name: "Norwegian"; MessagesFile: "compiler:Languages\Norwegian.isl"
Name: "Polish"; MessagesFile: "compiler:Languages\Polish.isl"
Name: "Portuguese"; MessagesFile: "compiler:Languages\Portuguese.isl"
Name: "Russian"; MessagesFile: "compiler:Languages\Russian.isl"
Name: "Slovak"; MessagesFile: "compiler:Languages\Slovak.isl"
Name: "Slovenian"; MessagesFile: "compiler:Languages\Slovenian.isl"
Name: "Spanish"; MessagesFile: "compiler:Languages\Spanish.isl"
Name: "Turkish"; MessagesFile: "compiler:Languages\Turkish.isl"
Name: "Ukrainian"; MessagesFile: "compiler:Languages\Ukrainian.isl"

[Types]

[Files]
Source: "C:\SAIL\*"; DestDir: "{app}"; Flags: ignoreversion recursesubdirs
Source: "vc_redist_2019.x64.exe"; DestDir: "{tmp}"; Flags: deleteafterinstall

[Tasks]

[Icons]
Name: "{group}\{cm:UninstallProgram,{#MyAppName}}"; Filename: "{uninstallexe}"

[Registry]

[Run]
Filename: {tmp}\vc_redist_2019.x64.exe; Parameters: "/install /q /norestart";  StatusMsg: "Installing VC++ Redistributable..."
Filename: "{#MyAppURL}/blob/master/README.md"; Flags: nowait shellexec

[Code]
procedure CurStepChanged(CurStep: TSetupStep);
var
  ResultCode: Integer;
  Uninstall: String;
  UninstallQuery : String;
begin
  UninstallQuery := ExpandConstant('Software\Microsoft\Windows\CurrentVersion\Uninstall\{#SetupSetting("AppId")}_is1');
  if (CurStep = ssInstall) then begin
    if RegQueryStringValue(HKLM, UninstallQuery, 'UninstallString', Uninstall) or RegQueryStringValue(HKCU, UninstallQuery, 'UninstallString', Uninstall) then begin
      Uninstall := RemoveQuotes(Uninstall)
      if (FileExists(Uninstall)) AND (not Exec(RemoveQuotes(Uninstall), '/VERYSILENT', '', SW_SHOWNORMAL, ewWaitUntilTerminated, ResultCode)) then begin
        MsgBox(SysErrorMessage(ResultCode), mbCriticalError, MB_OK);
        Abort();
      end;
    end;
  end;
end;
