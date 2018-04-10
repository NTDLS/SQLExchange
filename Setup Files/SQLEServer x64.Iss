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
 AppName                  = SQL-Exchange Server x64
 AppVerName               = SQL-Exchange Server x64 1.0.6.8
 AppCopyright             = Copyright © 1995-2010 NetworkDLS
 DefaultDirName           = {pf}\NetworkDLS\SQL-Exchange\Server x64
 DefaultGroupName         = NetworkDLS\SQL-Exchange Server x64
 UninstallDisplayIcon     = {app}\SQLEServer.Exe
 PrivilegesRequired       = PowerUser
 Uninstallable            = Yes
 AppMutex                 = SQL-Exchange Server x64
 Compression              = BZIP/9
 OutputBaseFilename       = "..\..\Bin\SQLEServer x64 1.0.6.8"
 DisableStartupPrompt     = Yes
 DirExistsWarning         = No
 VersionInfoVersion       = 1.0.6.8
 MinVersion               = 0.0,5.0
 ArchitecturesInstallIn64BitMode = x64
 ArchitecturesAllowed            = x64

;-- Windows 2000 & XP (Support Dialog)
 AppPublisher    = NetworkDLS
 AppPublisherURL = http://www.NetworkDLS.com/
 AppUpdatesURL   = http://www.NetworkDLS.com/
 AppVersion      = 1.0.6.8

[Tasks]
 Name: ConfigureServer;                Description: "Configure the Server";    GroupDescription: "Configure:";
 Name: InstallService;                 Description: "Install as service";      GroupDescription: "System Service:";
 Name: InstallService\StartService;    Description: "Start the service";       GroupDescription: "System Service:";
 Name: InstallService\InstallAgent;    Description: "Install service agent";   GroupDescription: "System Service:"; Flags: UnChecked;

[Files]
 Source: "..\Database Scripts\Monolithic.sql";        DestDir: "{app}\Database Script";  Flags: IgnoreVersion;

 Source: "..\Bin\x64\SQLEServer.Exe";                 DestDir: "{app}";      Flags: IgnoreVersion RestartReplace;
 Source: "..\Bin\x64\ServiceAgent.Exe";               DestDir: "{app}";      Flags: IgnoreVersion RestartReplace;

 Source: "Resources\OpenDlg.ico";                     DestDir: "{app}";      Flags: IgnoreVersion;
 Source: "Resources\Start.ico";                       DestDir: "{app}";      Flags: IgnoreVersion;
 Source: "Resources\Stop.ico";                        DestDir: "{app}";      Flags: IgnoreVersion;
 Source: "Resources\OpenCon.ico";                     DestDir: "{app}";      Flags: IgnoreVersion;

 Source: "../../@Libraries\EventLog\EventMsg\EventMsg.dll"; DestDir: "{app}"; Flags: IgnoreVersion RestartReplace;

[Icons]
 Name: "{group}\Open Server";             Filename: "{app}\SQLEServer.Exe";   WorkingDir: "{app}"; Parameters: "/OpenMainDialog";             IconFilename: "{app}\OpenDlg.ico";                          Tasks: InstallService
 Name: "{group}\Open Console";            Filename: "{app}\SQLEServer.Exe";   WorkingDir: "{app}"; Parameters: "/OpenConsole";                IconFilename: "{app}\OpenCon.ico";                          Tasks: InstallService
 Name: "{group}\Start Service";           Filename: "{app}\SQLEServer.Exe";   WorkingDir: "{app}"; Parameters: "/Start";                      IconFilename: "{app}\Start.ico";                            Tasks: InstallService
 Name: "{group}\Stop Service";            Filename: "{app}\SQLEServer.Exe";   WorkingDir: "{app}"; Parameters: "/Stop";                       IconFilename: "{app}\Stop.ico";                             Tasks: InstallService
 Name: "{group}\Run StandAlone";          Filename: "{app}\SQLEServer.Exe";   WorkingDir: "{app}"; Parameters: "/NonService";                 IconFilename: "{app}\SQLEServer.exe";   IconIndex: 0;
 Name: "{commonstartup}\SQLES Agent x64"; Filename: "{app}\ServiceAgent.Exe"; WorkingDir: "{app}"; Parameters: """SQL-Exchange Server x64"""; IconFilename: "{app}\ServiceAgent.exe"; IconIndex: 0;       Tasks: InstallService\InstallAgent

[Dirs]
 Name: "{app}\Logs"
 Name: "{app}\Temp"
 Name: "{app}\DB"
 
[Registry]
 Root: HKLM; Subkey: "Software\NetworkDLS\SQL-Exchange\Server x64"; Flags: UninsDeleteKey

 Root: HKLM; Subkey: "Software\NetworkDLS\SQL-Exchange\Server x64"; ValueName: "CompressionMethod";             ValueType: String; ValueData: "GZIP";            Flags: CreateValueIfDoesntExist;
 Root: HKLM; Subkey: "Software\NetworkDLS\SQL-Exchange\Server x64"; ValueName: "Path";                          ValueType: String; ValueData: "{app}";           Flags: CreateValueIfDoesntExist;
 Root: HKLM; Subkey: "Software\NetworkDLS\SQL-Exchange\Server x64"; ValueName: "SessionLogDir";                 ValueType: String; ValueData: "{app}\Logs";      Flags: CreateValueIfDoesntExist;
 Root: HKLM; Subkey: "Software\NetworkDLS\SQL-Exchange\Server x64"; ValueName: "SQLDataFiles";                  ValueType: String; ValueData: "{app}\DB";        Flags: CreateValueIfDoesntExist;
 Root: HKLM; Subkey: "Software\NetworkDLS\SQL-Exchange\Server x64"; ValueName: "SQLLogFiles";                   ValueType: String; ValueData: "{app}\DB";        Flags: CreateValueIfDoesntExist;
 Root: HKLM; Subkey: "Software\NetworkDLS\SQL-Exchange\Server x64"; ValueName: "DefaultDBO";                    ValueType: String; ValueData: "dbo";             Flags: CreateValueIfDoesntExist;
 Root: HKLM; Subkey: "Software\NetworkDLS\SQL-Exchange\Server x64"; ValueName: "GUIPWD";                        ValueType: String; ValueData: "";                Flags: CreateValueIfDoesntExist;

 Root: HKLM; Subkey: "Software\NetworkDLS\SQL-Exchange\Server x64"; ValueName: "AuthString";                    ValueType: String; ValueData: "trIeSPiUhouDlUwRLe5hie2H5awRoupiaDi8zI0fo3Y7u6R703r7e6rLacrouChi";
 Root: HKLM; Subkey: "Software\NetworkDLS\SQL-Exchange\Server x64"; ValueName: "AuthKey";                       ValueType: String; ValueData: "6riupiekieB8uphiuspi0tr6azl2piAth6us6ieko2zoap2818UviapHlEvo5joe";

 Root: HKLM; Subkey: "Software\NetworkDLS\SQL-Exchange\Server x64"; ValueName: "ImportTempDir";                 ValueType: String; ValueData: "{app}\Temp";

 Root: HKLM; Subkey: "Software\NetworkDLS\SQL-Exchange\Server x64"; ValueName: "SQLIndexDatabase";              ValueType: String; ValueData: "SQLExch_Index";   Flags: CreateValueIfDoesntExist;
 Root: HKLM; Subkey: "Software\NetworkDLS\SQL-Exchange\Server x64"; ValueName: "SQLIndexDriver";                ValueType: String; ValueData: "{{SQL Server}";   Flags: CreateValueIfDoesntExist;
 Root: HKLM; Subkey: "Software\NetworkDLS\SQL-Exchange\Server x64"; ValueName: "SQLIndexPassword";              ValueType: Binary; ValueData: "";                Flags: CreateValueIfDoesntExist;
 Root: HKLM; Subkey: "Software\NetworkDLS\SQL-Exchange\Server x64"; ValueName: "SQLIndexServer";                ValueType: String; ValueData: "(Local)";         Flags: CreateValueIfDoesntExist;
 Root: HKLM; Subkey: "Software\NetworkDLS\SQL-Exchange\Server x64"; ValueName: "SQLIndexUserID";                ValueType: String; ValueData: "sa";              Flags: CreateValueIfDoesntExist;
 Root: HKLM; Subkey: "Software\NetworkDLS\SQL-Exchange\Server x64"; ValueName: "SQLIndexUseTrustedConnection";  ValueType: Dword; ValueData: "1";                Flags: CreateValueIfDoesntExist;
 Root: HKLM; Subkey: "Software\NetworkDLS\SQL-Exchange\Server x64"; ValueName: "SQLIndexUseTCPIPConnection";    ValueType: Dword; ValueData: "0";                Flags: CreateValueIfDoesntExist;
 Root: HKLM; Subkey: "Software\NetworkDLS\SQL-Exchange\Server x64"; ValueName: "SQLIndexServerPort";            ValueType: Dword; ValueData: "1433";             Flags: CreateValueIfDoesntExist;

 Root: HKLM; Subkey: "Software\NetworkDLS\SQL-Exchange\Server x64"; ValueName: "SQLCustDriver";                 ValueType: String; ValueData: "{{SQL Server}";   Flags: CreateValueIfDoesntExist;
 Root: HKLM; Subkey: "Software\NetworkDLS\SQL-Exchange\Server x64"; ValueName: "SQLCustPassword";               ValueType: Binary; ValueData: "";                Flags: CreateValueIfDoesntExist;
 Root: HKLM; Subkey: "Software\NetworkDLS\SQL-Exchange\Server x64"; ValueName: "SQLCustServer";                 ValueType: String; ValueData: "(Local)";         Flags: CreateValueIfDoesntExist;
 Root: HKLM; Subkey: "Software\NetworkDLS\SQL-Exchange\Server x64"; ValueName: "SQLCustUserID";                 ValueType: String; ValueData: "sa";              Flags: CreateValueIfDoesntExist;
 Root: HKLM; Subkey: "Software\NetworkDLS\SQL-Exchange\Server x64"; ValueName: "SQLCustUseTrustedConnection";   ValueType: Dword; ValueData: "1";                Flags: CreateValueIfDoesntExist;
 Root: HKLM; Subkey: "Software\NetworkDLS\SQL-Exchange\Server x64"; ValueName: "SQLCustUseTCPIPConnection";     ValueType: Dword; ValueData: "0";                Flags: CreateValueIfDoesntExist;
 Root: HKLM; Subkey: "Software\NetworkDLS\SQL-Exchange\Server x64"; ValueName: "SQLCustServerPort";             ValueType: Dword; ValueData: "1433";             Flags: CreateValueIfDoesntExist;

 Root: HKLM; Subkey: "Software\NetworkDLS\SQL-Exchange\Server x64"; ValueName: "UseApplicationName";            ValueType: Dword; ValueData: "1";                Flags: CreateValueIfDoesntExist;

 Root: HKLM; Subkey: "Software\NetworkDLS\SQL-Exchange\Server x64"; ValueName: "MaxInactivityTime";             ValueType: Dword;  ValueData: "3600";            Flags: CreateValueIfDoesntExist;
 Root: HKLM; Subkey: "Software\NetworkDLS\SQL-Exchange\Server x64"; ValueName: "ListenPort";                    ValueType: Dword;  ValueData: "8100";            Flags: CreateValueIfDoesntExist;
 Root: HKLM; Subkey: "Software\NetworkDLS\SQL-Exchange\Server x64"; ValueName: "MaxClients";                    ValueType: Dword;  ValueData: "10";              Flags: CreateValueIfDoesntExist;
 Root: HKLM; Subkey: "Software\NetworkDLS\SQL-Exchange\Server x64"; ValueName: "ShowSplashScreen";              ValueType: Dword;  ValueData: "1";               Flags: CreateValueIfDoesntExist;
 Root: HKLM; Subkey: "Software\NetworkDLS\SQL-Exchange\Server x64"; ValueName: "CacheLogFile";                  ValueType: Dword;  ValueData: "1";               Flags: CreateValueIfDoesntExist;
 Root: HKLM; Subkey: "Software\NetworkDLS\SQL-Exchange\Server x64"; ValueName: "RTOFDWI";                       ValueType: Dword;  ValueData: "1";               Flags: CreateValueIfDoesntExist;
 Root: HKLM; Subkey: "Software\NetworkDLS\SQL-Exchange\Server x64"; ValueName: "VisualAlertOnError";            ValueType: Dword;  ValueData: "0";               Flags: CreateValueIfDoesntExist;
 Root: HKLM; Subkey: "Software\NetworkDLS\SQL-Exchange\Server x64"; ValueName: "VisualAlertOnWarn";             ValueType: Dword;  ValueData: "0";               Flags: CreateValueIfDoesntExist;

 Root: HKLM; Subkey: "Software\NetworkDLS\SQL-Exchange\Server x64"; ValueName: "DebugMode";                     ValueType: Dword;  ValueData: "0";               Flags: CreateValueIfDoesntExist;
 Root: HKLM; Subkey: "Software\NetworkDLS\SQL-Exchange\Server x64"; ValueName: "StartAutomatically";            ValueType: Dword;  ValueData: "1";               Flags: CreateValueIfDoesntExist;
 Root: HKLM; Subkey: "Software\NetworkDLS\SQL-Exchange\Server x64"; ValueName: "DropBadPackets";                ValueType: Dword;  ValueData: "0";               Flags: CreateValueIfDoesntExist;
 Root: HKLM; Subkey: "Software\NetworkDLS\SQL-Exchange\Server x64"; ValueName: "HashTimeout";                   ValueType: Dword;  ValueData: "5";               Flags: CreateValueIfDoesntExist;
 Root: HKLM; Subkey: "Software\NetworkDLS\SQL-Exchange\Server x64"; ValueName: "KeyGenLength";                  ValueType: Dword;  ValueData: "128";             Flags: CreateValueIfDoesntExist;
 Root: HKLM; Subkey: "Software\NetworkDLS\SQL-Exchange\Server x64"; ValueName: "NextClientID";                  ValueType: Dword;  ValueData: "1";               Flags: CreateValueIfDoesntExist;
 Root: HKLM; Subkey: "Software\NetworkDLS\SQL-Exchange\Server x64"; ValueName: "RawHashLen";                    ValueType: Dword;  ValueData: "32";              Flags: CreateValueIfDoesntExist;
 Root: HKLM; Subkey: "Software\NetworkDLS\SQL-Exchange\Server x64"; ValueName: "CompressionLevel";              ValueType: Dword;  ValueData: "6";               Flags: CreateValueIfDoesntExist;
 Root: HKLM; Subkey: "Software\NetworkDLS\SQL-Exchange\Server x64"; ValueName: "CPUAffinityMask";               ValueType: Dword;  ValueData: "0";               Flags: CreateValueIfDoesntExist;
 Root: HKLM; Subkey: "Software\NetworkDLS\SQL-Exchange\Server x64"; ValueName: "UseProcessBoost";               ValueType: Dword;  ValueData: "1";               Flags: CreateValueIfDoesntExist;
 Root: HKLM; Subkey: "Software\NetworkDLS\SQL-Exchange\Server x64"; ValueName: "UseThreadBoost";                ValueType: Dword;  ValueData: "1";               Flags: CreateValueIfDoesntExist;
 Root: HKLM; Subkey: "Software\NetworkDLS\SQL-Exchange\Server x64"; ValueName: "UseNTFibers";                   ValueType: Dword;  ValueData: "0";               Flags: CreateValueIfDoesntExist;
 Root: HKLM; Subkey: "Software\NetworkDLS\SQL-Exchange\Server x64"; ValueName: "AllowOffLineDataProcessing";    ValueType: Dword;  ValueData: "1";               Flags: CreateValueIfDoesntExist;
 Root: HKLM; Subkey: "Software\NetworkDLS\SQL-Exchange\Server x64"; ValueName: "SQLTimeout";                    ValueType: Dword;  ValueData: "600";             Flags: CreateValueIfDoesntExist;
 Root: HKLM; Subkey: "Software\NetworkDLS\SQL-Exchange\Server x64"; ValueName: "MaxRowsPerTransaction";         ValueType: Dword;  ValueData: "10000";           Flags: CreateValueIfDoesntExist;

;--Create registry keys and values for Statistics
 Root: HKLM; Subkey: "Software\NetworkDLS\SQL-Exchange\Server x64\Statistics"; ValueName: "SessionConnections"; ValueType: Dword;  ValueData: "0";               Flags: CreateValueIfDoesntExist;
 Root: HKLM; Subkey: "Software\NetworkDLS\SQL-Exchange\Server x64\Statistics"; ValueName: "CurrentConnections"; ValueType: Dword;  ValueData: "0";               Flags: CreateValueIfDoesntExist;
 Root: HKLM; Subkey: "Software\NetworkDLS\SQL-Exchange\Server x64\Statistics"; ValueName: "SessionErrors";      ValueType: Dword;  ValueData: "0";               Flags: CreateValueIfDoesntExist;
 Root: HKLM; Subkey: "Software\NetworkDLS\SQL-Exchange\Server x64\Statistics"; ValueName: "SessionWarnings";    ValueType: Dword;  ValueData: "0";               Flags: CreateValueIfDoesntExist;
 Root: HKLM; Subkey: "Software\NetworkDLS\SQL-Exchange\Server x64\Statistics"; ValueName: "SessionUpTime";      ValueType: Dword;  ValueData: "0";               Flags: CreateValueIfDoesntExist;
 Root: HKLM; Subkey: "Software\NetworkDLS\SQL-Exchange\Server x64\Statistics"; ValueName: "SessionBytesSent";   ValueType: Dword;  ValueData: "0";               Flags: CreateValueIfDoesntExist;
 Root: HKLM; Subkey: "Software\NetworkDLS\SQL-Exchange\Server x64\Statistics"; ValueName: "SessionBytesRecv";   ValueType: Dword;  ValueData: "0";               Flags: CreateValueIfDoesntExist;

 Root: HKLM; Subkey: "Software\NetworkDLS\SQL-Exchange"; ValueName: "TerminateAgent";                           ValueType: Dword;  ValueData: "0";

;--Create registry keys and values for Event Logs
 Root: HKLM; Subkey: "SYSTEM\CurrentControlSet\Services\Eventlog\SQL-Exchange\SQL-Exchange (Server) x64"; Flags: UninsDeleteKey

 Root: HKLM; Subkey: "SYSTEM\CurrentControlSet\Services\Eventlog\SQL-Exchange\SQL-Exchange (Server) x64"; ValueName: "TypesSupported";   ValueType: Dword;  ValueData: "7";                   Flags: CreateValueIfDoesntExist;
 Root: HKLM; Subkey: "SYSTEM\CurrentControlSet\Services\Eventlog\SQL-Exchange\SQL-Exchange (Server) x64"; ValueName: "EventMessageFile"; ValueType: String; ValueData: "{app}\EventMsg.dll";  Flags: CreateValueIfDoesntExist;

[UninstallDelete]
 Type: FilesAndOrDirs; Name: "{app}\Logs"

[Run]
 Filename: "{app}\SQLEServer.Exe";   Parameters: "/Config";                                    Tasks: ConfigureServer;
 Filename: "{app}\SQLEServer.Exe";   Parameters: "/Install";                                   Tasks: InstallService;
 Filename: "{app}\SQLEServer.Exe";   Parameters: "/Start";                                     Tasks: InstallService\StartService;
 Filename: "{app}\ServiceAgent.Exe"; Parameters: """SQL-Exchange Server x64"""; Flags: NoWait; Tasks: InstallService\InstallAgent

[UninstallRun]
 Filename: "{app}\SQLEServer.Exe";    Parameters: "/Stop";   RunOnceId: "StopService";   MinVersion: 0, 4.0; Tasks: InstallService;
 Filename: "{app}\SQLEServer.Exe";    Parameters: "/Delete"; RunOnceId: "DeleteService"; MinVersion: 0, 4.0; Tasks: InstallService;
 Filename: "{app}\ServiceAgent.Exe";  Parameters: "/Close";  RunOnceId: "CloseAgent";    MinVersion: 0, 4.0; Tasks: InstallService\InstallAgent
 
