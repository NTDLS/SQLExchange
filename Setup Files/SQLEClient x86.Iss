;-----------------------------------------------------------------------
;  Copyright © NetworkDLS 2002, All rights reserved
;
; THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
; ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
; THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
; PARTICULAR PURPOSE.
;-----------------------------------------------------------------------

[Setup]
;-- Main Setup Information
 AppName                  = SQL-Exchange Client x86
 AppVerName               = SQL-Exchange Client x86 1.0.6.8
 AppCopyright             = Copyright © 1995-2010 NetworkDLS
 DefaultDirName           = {pf}\NetworkDLS\SQL-Exchange\Client x86
 DefaultGroupName         = NetworkDLS\SQL-Exchange Client x86
 UninstallDisplayIcon     = {app}\SQLEClient.Exe
 PrivilegesRequired       = PowerUser
 Uninstallable            = Yes
 AppMutex                 = SQL-Exchange Client x86
 Compression              = BZIP/9
 OutputBaseFilename       = "..\..\Bin\SQLEClient x86 1.0.6.8"
 DisableStartupPrompt     = Yes
 DirExistsWarning         = No
 VersionInfoVersion       = 1.0.6.8
 MinVersion               = 0.0,5.0

;-- Windows 2000 & XP (Support Dialog)
 AppPublisher    = NetworkDLS
 AppPublisherURL = http://www.NetworkDLS.com/
 AppUpdatesURL   = http://www.NetworkDLS.com/
 AppVersion      = 1.0.6.8

[Tasks]
 Name: CreateShortcuts;             Description: "Create start menu icons"; GroupDescription: "Shortcut Options:";
 Name: ConfigureClient;             Description: "Configure the Client";    GroupDescription: "Configure:";
 Name: InstallService;              Description: "Install as service";      GroupDescription: "System Service:";
 Name: InstallService\StartService; Description: "Start the service";       GroupDescription: "System Service:";
 Name: InstallService\InstallAgent; Description: "Install service agent";   GroupDescription: "System Service:"; Flags: UnChecked;

[Files]
 Source: "..\Bin\Win32\SQLEClient.Exe";                  DestDir: "{app}"; Flags: IgnoreVersion RestartReplace;
 Source: "..\Bin\Win32\AutoUpdate.Exe";                  DestDir: "{app}"; Flags: IgnoreVersion RestartReplace;
 Source: "..\Bin\Win32\ServiceAgent.Exe";                DestDir: "{app}"; Flags: IgnoreVersion RestartReplace;

 Source: "Resources\OpenDlg.ico";                           DestDir: "{app}"; Flags: IgnoreVersion;
 Source: "Resources\Start.ico";                             DestDir: "{app}"; Flags: IgnoreVersion;
 Source: "Resources\Stop.ico";                              DestDir: "{app}"; Flags: IgnoreVersion;
 Source: "Resources\OpenCon.ico";                           DestDir: "{app}"; Flags: IgnoreVersion;

 Source: "../../@Libraries\EventLog\EventMsg\EventMsg.dll"; DestDir: "{app}"; Flags: IgnoreVersion RestartReplace;

[Icons]
 Name: "{group}\Open Client";             Filename: "{app}\SQLEClient.Exe";   WorkingDir: "{app}"; Parameters: "/OpenMainDialog";             IconFilename: "{app}\OpenDlg.ico";                    Tasks: CreateShortcuts AND InstallService
;Name: "{group}\Open Console";            Filename: "{app}\SQLEClient.Exe";   WorkingDir: "{app}"; Parameters: "/OpenConsole";                IconFilename: "{app}\OpenCon.ico";                    Tasks: CreateShortcuts AND InstallService
 Name: "{group}\Start Service";           Filename: "{app}\SQLEClient.Exe";   WorkingDir: "{app}"; Parameters: "/Start";                      IconFilename: "{app}\Start.ico";                      Tasks: CreateShortcuts AND InstallService
 Name: "{group}\Stop Service";            Filename: "{app}\SQLEClient.Exe";   WorkingDir: "{app}"; Parameters: "/Stop";                       IconFilename: "{app}\Stop.ico";                       Tasks: CreateShortcuts AND InstallService
 Name: "{group}\Run StandAlone";          Filename: "{app}\SQLEClient.Exe";   WorkingDir: "{app}"; Parameters: "/NonService";                 IconFilename: "{app}\SQLEClient.exe";   IconIndex: 0; Tasks: CreateShortcuts
 Name: "{commonstartup}\SQLEC Agent x86"; Filename: "{app}\ServiceAgent.Exe"; WorkingDir: "{app}"; Parameters: """SQL-Exchange Client x86"""; IconFilename: "{app}\ServiceAgent.exe"; IconIndex: 0; Tasks: CreateShortcuts AND InstallService\InstallAgent

[Dirs]
 Name: "{app}"
 Name: "{app}\Temp"
 Name: "{app}\DB"
 Name: "{app}\Logs"

[Registry]
 Root: HKLM; Subkey: "Software\NetworkDLS\SQL-Exchange\Client x86"; Flags: UninsDeleteKey

 Root: HKLM; Subkey: "Software\NetworkDLS\SQL-Exchange\Client x86"; ValueName: "Path";                          ValueType: String; ValueData: "{app}";

 Root: HKLM; Subkey: "Software\NetworkDLS\SQL-Exchange\Client x86"; ValueName: "SQLDataFiles";                  ValueType: String; ValueData: "{app}\DB";             Flags: CreateValueIfDoesntExist;
 Root: HKLM; Subkey: "Software\NetworkDLS\SQL-Exchange\Client x86"; ValueName: "SQLLogFiles";                   ValueType: String; ValueData: "{app}\DB";             Flags: CreateValueIfDoesntExist;
 Root: HKLM; Subkey: "Software\NetworkDLS\SQL-Exchange\Client x86"; ValueName: "DefaultDBO";                    ValueType: String; ValueData: "dbo";                  Flags: CreateValueIfDoesntExist;
 Root: HKLM; Subkey: "Software\NetworkDLS\SQL-Exchange\Client x86"; ValueName: "ReplicationDB";                 ValueType: String; ValueData: "SQLExch_Replication";  Flags: CreateValueIfDoesntExist;
 Root: HKLM; Subkey: "Software\NetworkDLS\SQL-Exchange\Client x86"; ValueName: "GUIPWD";                        ValueType: String; ValueData: "";                     Flags: CreateValueIfDoesntExist;

 Root: HKLM; Subkey: "Software\NetworkDLS\SQL-Exchange\Client x86"; ValueName: "SessionLogDir";                 ValueType: String; ValueData: "{app}\Logs";           Flags: CreateValueIfDoesntExist;
 Root: HKLM; Subkey: "Software\NetworkDLS\SQL-Exchange\Client x86"; ValueName: "TempFilesPath";                 ValueType: String; ValueData: "{app}\Temp";           Flags: CreateValueIfDoesntExist;
 Root: HKLM; Subkey: "Software\NetworkDLS\SQL-Exchange\Client x86"; ValueName: "SQLDriver";                     ValueType: String; ValueData: "{{SQL Server}";        Flags: CreateValueIfDoesntExist;
 Root: HKLM; Subkey: "Software\NetworkDLS\SQL-Exchange\Client x86"; ValueName: "SQLPassword";                   ValueType: Binary; ValueData: "";                     Flags: CreateValueIfDoesntExist;
 Root: HKLM; Subkey: "Software\NetworkDLS\SQL-Exchange\Client x86"; ValueName: "SQLServer";                     ValueType: String; ValueData: "(local)";              Flags: CreateValueIfDoesntExist;
 Root: HKLM; Subkey: "Software\NetworkDLS\SQL-Exchange\Client x86"; ValueName: "SQLUserID";                     ValueType: String; ValueData: "sa";                   Flags: CreateValueIfDoesntExist;
 Root: HKLM; Subkey: "Software\NetworkDLS\SQL-Exchange\Client x86"; ValueName: "CompanyName";                   ValueType: String; ValueData: "";                     Flags: CreateValueIfDoesntExist;
 Root: HKLM; Subkey: "Software\NetworkDLS\SQL-Exchange\Client x86"; ValueName: "CompanyPassword";               ValueType: Binary; ValueData: "";                     Flags: CreateValueIfDoesntExist;
 Root: HKLM; Subkey: "Software\NetworkDLS\SQL-Exchange\Client x86"; ValueName: "ServerAddress";                 ValueType: String; ValueData: "SQLExch.com";          Flags: CreateValueIfDoesntExist;
 Root: HKLM; Subkey: "Software\NetworkDLS\SQL-Exchange\Client x86"; ValueName: "ShowSplashScreen";              ValueType: Dword;  ValueData: "1";                    Flags: CreateValueIfDoesntExist;
 Root: HKLM; Subkey: "Software\NetworkDLS\SQL-Exchange\Client x86"; ValueName: "CacheLogFile";                  ValueType: Dword;  ValueData: "1";                    Flags: CreateValueIfDoesntExist;
 Root: HKLM; Subkey: "Software\NetworkDLS\SQL-Exchange\Client x86"; ValueName: "ReportNoTransactions";          ValueType: Dword;  ValueData: "1";                    Flags: CreateValueIfDoesntExist;
 Root: HKLM; Subkey: "Software\NetworkDLS\SQL-Exchange\Client x86"; ValueName: "CPUAffinityMask";               ValueType: Dword;  ValueData: "0";                    Flags: CreateValueIfDoesntExist;

 Root: HKLM; Subkey: "Software\NetworkDLS\SQL-Exchange\Client x86"; ValueName: "UseTrustedConnection";          ValueType: Dword;  ValueData: "1";                    Flags: CreateValueIfDoesntExist;
 Root: HKLM; Subkey: "Software\NetworkDLS\SQL-Exchange\Client x86"; ValueName: "UseTCPIPConnection";            ValueType: Dword;  ValueData: "0";                    Flags: CreateValueIfDoesntExist;
 Root: HKLM; Subkey: "Software\NetworkDLS\SQL-Exchange\Client x86"; ValueName: "UseApplicationName";            ValueType: Dword;  ValueData: "1";                    Flags: CreateValueIfDoesntExist;
 Root: HKLM; Subkey: "Software\NetworkDLS\SQL-Exchange\Client x86"; ValueName: "SQLServerPort";                 ValueType: Dword;  ValueData: "1433";                 Flags: CreateValueIfDoesntExist;
 Root: HKLM; Subkey: "Software\NetworkDLS\SQL-Exchange\Client x86"; ValueName: "UseProcessBoost";               ValueType: Dword;  ValueData: "1";                    Flags: CreateValueIfDoesntExist;
 Root: HKLM; Subkey: "Software\NetworkDLS\SQL-Exchange\Client x86"; ValueName: "UseThreadBoost";                ValueType: Dword;  ValueData: "1";                    Flags: CreateValueIfDoesntExist;
 Root: HKLM; Subkey: "Software\NetworkDLS\SQL-Exchange\Client x86"; ValueName: "UseNTFibers";                   ValueType: Dword;  ValueData: "0";                    Flags: CreateValueIfDoesntExist;
 Root: HKLM; Subkey: "Software\NetworkDLS\SQL-Exchange\Client x86"; ValueName: "AllowOffLineDataProcessing";    ValueType: Dword;  ValueData: "1";                    Flags: CreateValueIfDoesntExist;
 Root: HKLM; Subkey: "Software\NetworkDLS\SQL-Exchange\Client x86"; ValueName: "TrimStringDataOnExport";        ValueType: Dword;  ValueData: "1";                    Flags: CreateValueIfDoesntExist;
 Root: HKLM; Subkey: "Software\NetworkDLS\SQL-Exchange\Client x86"; ValueName: "RequestInitialization";         ValueType: Dword;  ValueData: "0";                    Flags: CreateValueIfDoesntExist;

 Root: HKLM; Subkey: "Software\NetworkDLS\SQL-Exchange\Client x86"; ValueName: "ConnectDays";                   ValueType: Binary; ValueData: "00 01 01 01 01 01 00"; Flags: CreateValueIfDoesntExist;

 Root: HKLM; Subkey: "Software\NetworkDLS\SQL-Exchange\Client x86"; ValueName: "AuthString";                    ValueType: String; ValueData: "trIeSPiUhouDlUwRLe5hie2H5awRoupiaDi8zI0fo3Y7u6R703r7e6rLacrouChi";
 Root: HKLM; Subkey: "Software\NetworkDLS\SQL-Exchange\Client x86"; ValueName: "AuthKey";                       ValueType: String; ValueData: "6riupiekieB8uphiuspi0tr6azl2piAth6us6ieko2zoap2818UviapHlEvo5joe";

 Root: HKLM; Subkey: "Software\NetworkDLS\SQL-Exchange\Client x86"; ValueName: "DebugMode";                     ValueType: Dword;  ValueData: "0";                    Flags: CreateValueIfDoesntExist;
 Root: HKLM; Subkey: "Software\NetworkDLS\SQL-Exchange\Client x86"; ValueName: "ServerPort";                    ValueType: Dword;  ValueData: "8100";
 Root: HKLM; Subkey: "Software\NetworkDLS\SQL-Exchange\Client x86"; ValueName: "ConnectInterval";               ValueType: Dword;  ValueData: "10";                   Flags: CreateValueIfDoesntExist;
 Root: HKLM; Subkey: "Software\NetworkDLS\SQL-Exchange\Client x86"; ValueName: "SQLTimeout";                    ValueType: Dword;  ValueData: "600";

 Root: HKLM; Subkey: "Software\NetworkDLS\SQL-Exchange\Client x86"; ValueName: "TimeWindowBegin";               ValueType: Dword;  ValueData: "32400";                Flags: CreateValueIfDoesntExist;
 Root: HKLM; Subkey: "Software\NetworkDLS\SQL-Exchange\Client x86"; ValueName: "TimeWindowEnd";                 ValueType: Dword;  ValueData: "61200";                Flags: CreateValueIfDoesntExist;
 Root: HKLM; Subkey: "Software\NetworkDLS\SQL-Exchange\Client x86"; ValueName: "MaxInactivityTime";             ValueType: Dword;  ValueData: "3600";                 Flags: CreateValueIfDoesntExist;

 Root: HKLM; Subkey: "Software\NetworkDLS\SQL-Exchange"; ValueName: "TerminateAgent";                           ValueType: Dword;  ValueData: "0";

;--Create registry keys and values for Statistics
 Root: HKLM; Subkey: "Software\NetworkDLS\SQL-Exchange\Client x86\Statistics"; ValueName: "SessionConnections"; ValueType: Dword;  ValueData: "0";                    Flags: CreateValueIfDoesntExist;
 Root: HKLM; Subkey: "Software\NetworkDLS\SQL-Exchange\Client x86\Statistics"; ValueName: "CurrentConnections"; ValueType: Dword;  ValueData: "0";                    Flags: CreateValueIfDoesntExist;
 Root: HKLM; Subkey: "Software\NetworkDLS\SQL-Exchange\Client x86\Statistics"; ValueName: "SessionErrors";      ValueType: Dword;  ValueData: "0";                    Flags: CreateValueIfDoesntExist;
 Root: HKLM; Subkey: "Software\NetworkDLS\SQL-Exchange\Client x86\Statistics"; ValueName: "SessionWarnings";    ValueType: Dword;  ValueData: "0";                    Flags: CreateValueIfDoesntExist;
 Root: HKLM; Subkey: "Software\NetworkDLS\SQL-Exchange\Client x86\Statistics"; ValueName: "SessionUpTime";      ValueType: Dword;  ValueData: "0";                    Flags: CreateValueIfDoesntExist;
 Root: HKLM; Subkey: "Software\NetworkDLS\SQL-Exchange\Client x86\Statistics"; ValueName: "SessionBytesSent";   ValueType: Dword;  ValueData: "0";                    Flags: CreateValueIfDoesntExist;
 Root: HKLM; Subkey: "Software\NetworkDLS\SQL-Exchange\Client x86\Statistics"; ValueName: "SessionBytesRecv";   ValueType: Dword;  ValueData: "0";                    Flags: CreateValueIfDoesntExist;

;--Create registry keys and values for Event Logs
 Root: HKLM; Subkey: "SYSTEM\CurrentControlSet\Services\Eventlog\SQL-Exchange\SQL-Exchange (Client) x86"; Flags: UninsDeleteKey

 Root: HKLM; Subkey: "SYSTEM\CurrentControlSet\Services\Eventlog\SQL-Exchange\SQL-Exchange (Client) x86"; ValueName: "TypesSupported";   ValueType: Dword;  ValueData: "7";                   Flags: CreateValueIfDoesntExist;
 Root: HKLM; Subkey: "SYSTEM\CurrentControlSet\Services\Eventlog\SQL-Exchange\SQL-Exchange (Client) x86"; ValueName: "EventMessageFile"; ValueType: String; ValueData: "{app}\EventMsg.dll";  Flags: CreateValueIfDoesntExist;

[UninstallDelete]
 Type: FilesAndOrDirs; Name: "{app}\Logs"

[Run]
 Filename: "{app}\SQLEClient.Exe";   Parameters: "/Config";                                    Tasks: ConfigureClient;
 Filename: "{app}\SQLEClient.Exe";   Parameters: "/Install";                                   Tasks: InstallService;
 Filename: "{app}\SQLEClient.Exe";   Parameters: "/Start";                                     Tasks: InstallService\StartService;
 Filename: "{app}\ServiceAgent.Exe"; Parameters: """SQL-Exchange Client x86"""; Flags: NoWait; Tasks: InstallService\InstallAgent

[UninstallRun]
 Filename: "{app}\SQLEClient.Exe";    Parameters: "/Stop";   RunOnceId: "StopService";   MinVersion: 0, 4.0; Tasks: InstallService;
 Filename: "{app}\SQLEClient.Exe";    Parameters: "/Delete"; RunOnceId: "DeleteService"; MinVersion: 0, 4.0; Tasks: InstallService;
 Filename: "{app}\ServiceAgent.Exe";  Parameters: "/Close";  RunOnceId: "CloseAgent";    MinVersion: 0, 4.0; Tasks: InstallService\InstallAgent

