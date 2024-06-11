﻿; Script generated by the Inno Setup Script Wizard.
; SEE THE DOCUMENTATION FOR DETAILS ON CREATING INNO SETUP SCRIPT FILES!

#define MyAppName "Warband Script Enhancer 2"
#ifndef MyAppVersion
  #define MyAppVersion "1.1.1.6"
#endif
#define MyAppPublisher "K700, cmpxchg8b, AgentSmith"
#define MyAppURL "https://forums.taleworlds.com/index.php?threads/warband-script-enhancer-2-v1-1-1-5.384882/"
#define MyAppExeName "wse2_launcher.exe"

[Setup]
; NOTE: The value of AppId uniquely identifies this application. Do not use the same AppId value in installers for other applications.
; (To generate a new GUID, click Tools | Generate GUID inside the IDE.)
AppId={{C92628E7-333E-4CDA-B4B9-AB3EB028E15E}
AppName={#MyAppName}
AppVersion={#MyAppVersion}
AppVerName={#MyAppName} (v{#MyAppVersion})
AppPublisher={#MyAppPublisher}
AppPublisherURL={#MyAppURL}
AppSupportURL={#MyAppURL}
AppUpdatesURL={#MyAppURL}
AppendDefaultDirName=no
DefaultDirName={reg:HKLM\SOFTWARE\Mount&Blade Warband,Install_Path|{commonpf}\Mount&Blade Warband}\
DefaultGroupName={#MyAppName}
DisableProgramGroupPage=yes
DirExistsWarning=no
OutputBaseFilename=WSE2_Installer
; Remove the following line to run in administrative install mode (install for all users.)
PrivilegesRequired=lowest
SetupIconFile=Images\iconwb.ico
Compression=lzma
SolidCompression=yes
Uninstallable=no
WizardStyle=classic
WizardSmallImageFile=Images\mb_inst_top.bmp
WizardImageFile=Images\mb_inst_left.bmp

[Languages]
Name: "english"; MessagesFile: "compiler:Default.isl"

[Tasks]
Name: "desktopicon"; Description: "{cm:CreateDesktopIcon}"; GroupDescription: "{cm:AdditionalIcons}"; Flags: unchecked
Name: "steam_shortcut"; Description: "Add to your steam library";  Check: has_shortcuts
Name: "copy_profiles"; Description: "Copy profiles"; Check: can_copy_profiles

[Files]
Source: ".\files\WSE\*"; DestDir: "{app}"; Flags: recursesubdirs createallsubdirs
Source: ".\files\vdf-shortcut-editor.exe"; DestDir: "{app}"; Flags: deleteafterinstall

Source: "{userappdata}\Mount&Blade Warband\profiles.dat"; DestDir: "{userappdata}\Mount&Blade Warband WSE2\"; Check: can_copy_profiles() and WizardIsTaskSelected('copy_profiles'); Flags: onlyifdoesntexist external

[Icons]
Name: "{autodesktop}\{#MyAppName}"; Filename: "{app}\{#MyAppExeName}"; Tasks: desktopicon

[Run]
Filename: "{app}\vdf-shortcut-editor.exe"; Parameters: """{code:find_shortcuts_path}"" -a -1287593386 ""Mount & Blade: Warband WSE2"" ""{app}\{#MyAppExeName}"""; StatusMsg: "Adding to steam..."; Flags: runhidden; Check: WizardIsTaskSelected('steam_shortcut');

[Code]
var
  DirOk_Label: TNewStaticText;

function find_shortcuts_path(Param: string): string;
var
  success: boolean;
  active_usr : Cardinal;
  shortcut_path, usr_dir, found_dir: string;
  info : TFindRec;
begin
  result := '';
  
  success := RegQueryStringValue(HKEY_CURRENT_USER, 'Software\Valve\Steam', 'SteamPath', usr_dir);
  if not success then begin Exit; end;
  usr_dir := usr_dir + '/userdata/';
  if not DirExists(usr_dir) then begin Exit; end;
  
  //First see if a user is logged into steam and use that id
  success := RegQueryDWordValue(HKEY_CURRENT_USER, 'Software\Valve\Steam\ActiveProcess', 'ActiveUser', active_usr);
  if success and (active_usr <> 0) then begin
    shortcut_path := usr_dir + IntToStr(active_usr) + '/config/shortcuts.vdf';
    
    if FileExists(shortcut_path) then begin
      result := shortcut_path;
      Exit;  
    end; 
  end;
  
  //Next we look if there is a single user folder (not counting "0")
  If FindFirst(usr_dir + '*', info) then
  begin
    found_dir := '';
    try
      Repeat
        if info.Attributes and FILE_ATTRIBUTE_DIRECTORY <> 0 then begin
          if (info.Name <> '0') and (info.Name <> '.') and (info.Name <> '..') then begin
            if found_dir <> '' then begin //if we already found a folder, it means there are multiple accounts. Abort
              result := '';
              Exit;
            end;
            found_dir := info.Name;
          end;
        end;
      Until not FindNext(info);
    finally
      FindClose(info);
    end;
  end;
  
  if found_dir <> '' then begin
    shortcut_path := usr_dir + found_dir + '/config/shortcuts.vdf';
    if FileExists(shortcut_path) then begin
      result := shortcut_path;
      Exit;  
    end;  
  end;   
end;

function has_shortcuts(): boolean;
begin
  result := (find_shortcuts_path('') <> '');
end;

function can_copy_profiles(): boolean;
begin
  result := FileExists(ExpandConstant('{userappdata}\Mount&Blade Warband\profiles.dat')) and not FileExists(ExpandConstant('{userappdata}\Mount&Blade Warband WSE2\profiles.dat'))
end;

procedure OnDirChanged(Sender: TObject);
begin
  if FileExists(WizardForm.DirEdit.Text + '\mb_warband.exe') then begin
    DirOk_Label.Caption := '✓';
    DirOk_Label.Font.Color := clGreen;
  end
  else begin
    DirOk_Label.Caption := 'x Couldn''t find mb_warband.exe - Select your M&&B Warband folder.';
    DirOk_Label.Font.Color := clMaroon;
  end;
end;

procedure InitializeWizard();
begin
  WizardForm.DirEdit.OnChange := @OnDirChanged;
  
  DirOk_Label := TNewStaticText.Create(WizardForm.SelectDirPage);
  DirOk_Label.Parent := WizardForm.SelectDirPage;
  DirOk_Label.Top := WizardForm.DirEdit.Top + WizardForm.DirEdit.Height + ScaleY(8);
  DirOk_Label.Left := WizardForm.DirEdit.Left;
  DirOk_Label.Caption := 'My checkbox';
  // See https://stackoverflow.com/q/30469660/850848
  DirOk_Label.Height := ScaleY(DirOk_Label.Height);
end;

procedure CurPageChanged(CurPageID: Integer);
var
  p: string;
begin
  if CurPageID = wpSelectDir then begin                                                                          
    OnDirChanged(nil);
  end
  else if (CurPageID = wpReady) and WizardIsTaskSelected('steam_shortcut') then begin
    p := find_shortcuts_path('');
    with Wizardform.ReadyMemo.Lines do begin
      Add('');
      Add('**Notice**');
      Add('    In order to add WSE2 to your steam library, setup will modify');
      Add('    "' + p + '"');
      Add('    A backup is created in the same folder with .bak ending');
      Add('    If something goes wrong, you can restore from this file.');
      Add('    You must restart Steam to see the new shortcut.');  
    end;
  end
end;