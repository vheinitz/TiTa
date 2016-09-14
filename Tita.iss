;InnoSetup installer-builder script for Tita.
;V.Heinitz, 2012-05-03


[Setup]
AppName=Tita
AppVerName=Tita 0.1.0
OutputBaseFilename=TitaSetup_0.1.0
AppPublisher=Valentin Heinitz
AppPublisherURL=http://www.heinitz-it.de
AppSupportURL=http://www.heinitz-it.de
AppUpdatesURL=http://www.heinitz-it.de
AppCopyright=Copyright © 2015 Valentin Heinitz
DefaultDirName={pf}\\heinitz-it.de\\Tita
DisableDirPage=no
DefaultGroupName=heinitz-it.de\Tita
DisableProgramGroupPage=yes
AllowNoIcons=yes
OutputDir=.
;SetupIconFile=..\Tita\tita.ico
Compression=lzma
;WizardImageFile=.\res\TitaSetup.bmp
;WizardSmallImageFile=.\res\TitaSetup_s.bmp
SolidCompression=yes
BackColor=clGreen

[Languages]
Name: "en"; MessagesFile: "compiler:Default.isl"

[Tasks]
Name: "desktopicon"; Description: "{cm:CreateDesktopIcon}"; GroupDescription: "{cm:AdditionalIcons}"; Flags: unchecked
Name: "quicklaunchicon"; Description: "{cm:CreateQuickLaunchIcon}"; GroupDescription: "{cm:AdditionalIcons}"; Flags: unchecked

[Files]

Source: "c:\Qt\4.8.4\bin\QtCore4.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "c:\Qt\4.8.4\bin\QtGui4.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "c:\Qt\4.8.4\bin\QtMultimedia4.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "release\Tita.exe"; DestDir: "{app}"; Flags: ignoreversion; Permissions: admins-full;


[Icons]
Name: "{group}\Tita"; Filename: "{app}\Tita.exe"
Name: "{group}\{cm:UninstallProgram,Tita}"; Filename: "{uninstallexe}"
Name: "{userdesktop}\Tita"; Filename: "{app}\Tita.exe"; Tasks: desktopicon

[Run]
Filename: "{app}\Tita.exe"; Description: "{cm:LaunchProgram,Tita}"; Flags: nowait postinstall skipifsilent
