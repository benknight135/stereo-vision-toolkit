; -- Stereo Vision Toolkit --
; Inno installer script for Stereo Vision Toolkit
; MUST be installed on x64 bit machine

#define AppName "Stereo Vision Toolkit"
#define InstallerName "Stereo Vision Toolkit Installer"
#define ExeName "stereo_vision_toolkit.exe"
#define IconName "i3dr_logo.ico"

[Setup]
AppId={{4A16961A-FA66-4C80-A04E-D5370D76763B}}
AppName={#AppName}
AppVersion=1.1.0
AppPublisher=i3D Robotics Ltd.
AppPublisherURL=http://www.i3drobotics.com/
AppSupportURL=https://github.com/i3drobotics/stereo-vision-toolkit
AppUpdatesURL=https://github.com/i3drobotics/stereo-vision-toolkit/releases
DefaultDirName={pf64}/i3DR/Stereo Vision Toolkit
DefaultGroupName=i3D Robotics
LicenseFile=../LICENSE
OutputBaseFilename={#InstallerName}
SetupIconFile="../{#IconName}"
Compression=lzma2
SolidCompression=yes
ArchitecturesAllowed=x64
ArchitecturesInstallIn64BitMode=x64

[Languages]
Name: "english"; MessagesFile: "compiler:Default.isl"

[Tasks]
Name: "desktopicon"; Description: "{cm:CreateDesktopIcon}"; \
    GroupDescription: "{cm:AdditionalIcons}"; Flags: unchecked

[Files]
Source: "../../build/release/*"; Excludes: "\params\*.xml"; DestDir: "{app}"; Flags: ignoreversion createallsubdirs recursesubdirs
Source: "../../build/release/vcredist_x64.exe"; DestDir: {tmp}; Flags: deleteafterinstall
Source: "../licenses/*"; DestDir: "{app}/licenses"; Flags: ignoreversion createallsubdirs recursesubdirs
Source: "../LICENSE"; DestDir: "{app}/licenses"
Source: "../{#IconName}"; DestDir: "{app}"

[Run]
Filename: {tmp}\vcredist_x64.exe; Parameters: "/q /passive /Q:a /c:""msiexec /q /i vcredist.msi"" "; StatusMsg: Installing VC++ Redistributable...;

[Icons]
Name: "{group}\{cm:UninstallProgram,Stereo Vision Toolkit}"; Filename: "{uninstallexe}"
Name: "{group}\{#AppName}"; Filename: "{app}\{#ExeName}"; IconFilename: "{app}\{#IconName}"
Name: "{commondesktop}\{#AppName}"; Filename: "{app}\{#ExeName}"; IconFilename: "{app}\{#IconName}"; Tasks: desktopicon