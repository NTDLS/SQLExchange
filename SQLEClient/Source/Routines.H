///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//  Copyright © NetworkDLS 2002, All rights reserved
//
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF 
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO 
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A 
// PARTICULAR PURPOSE.
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef _ROUTINES_H
#define _ROUTINES_H
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include <SQL.H>

#include "Entry.H"
#include "Init.H"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#define CRYPTKEY        "XHU7wSIv8f3h5tpmx6G1kCubtpFegXhO"
#define CRYPTFLAGS      ROUNDRESULT | NONREPETITION | ENSUREOFFSET
#define MAX_STATUS_TEXT 10240
#define SENDBUFSZ       4096
#define RECVBUFSZ       5120

#define MAX_AUTH_LENGTH 1024 //Max size of the string used for authentication
#define MAX_KEY_LENGTH  1024 //Max size of the encryption key. (Both Registry & Generated)

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool SaveRegistryValues(void);
bool GetRegistryValues(void);
bool ConnectToServer(void);
bool IsClientConfigured(bool bAlertUser);
bool LaunchConfigDlg(HWND hOwner);
void PrintNextRunTime(void);

void SetStatus(HWND hGrid, int iCol, int iItem, char *sText);
bool IsAtleastOneDaySelected(void);
int GetNextRunDay(int iToday);
void UpdateStatistics(void);
bool StopClient(void);
bool StartClient(void);

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

extern CNASCCL gSC;
extern CSocketServer gServer;

struct _MainDialogInfo{
	HWND hLogList;
	HWND hStatusBar;
	HWND hWnd;
	HMENU hMenu;
	CQueu Queu;
}; extern _MainDialogInfo MDI;

struct _ConnectedClientInfo{
	int iInitStep;
	CSocketClient *pClient;

	DWORD dwCompressionLevel;

	bool bIsAuthenticated;
	bool bIsEncryptionInit;
	bool bCustDBConnected;

	CNASCCL SC;

	char sCompressionMethod[255];
	
	char sTransDatabase[255];
	char sTransTable[255];
	char sTransSchema[255];

	char sOnSuccess[2048 + 1];
	char sOnFailure[2048 + 1];

    HSTMT hStatementHandle;

	CSQL cCustSQL;
	CBoundRecordSet rsTrans;

}; extern _ConnectedClientInfo CCI;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

extern CRITICAL_SECTION csTransPerSecond;

DWORD WINAPI Timer_Thread(LPVOID);

extern const char *gsWeekDayName[];

extern bool gbWeekDays[7];

extern char gsSessionLogDir[MAX_PATH];
extern char gsTempFilesPath[MAX_PATH];
extern char gsPath[MAX_PATH];
extern char gsServerAddress[255];
extern char gsAuthKey[MAX_AUTH_LENGTH];
extern char gsAuthString[MAX_AUTH_LENGTH];
extern char gsGUIPWDHash[256];

extern char gsCompanyName[64];
extern char gsCompanyPassword[64];

extern char gsSQLDataFiles[MAX_PATH]; // Relative to server.
extern char gsSQLLogFiles[MAX_PATH]; // Relative to server.
extern char gsDefaultDBO[255];
extern char gsReplicationDB[255];

extern char gsSQLDriver[255];
extern char gsSQLPassword[255];
extern char gsSQLServer[255];
extern char gsSQLUserID[255];

extern bool gbReportNoTransactions;
extern bool gbShowSplashScreen;
extern bool gbConnectNow;
extern bool gbThrowSQLErrors;
extern bool gbReplaceSngleQteSQLData;
extern bool gbRTrimSQLData;
extern bool gbPacketMode;
extern bool gbAutoConnect;
extern bool gbCacheLogFile;
extern bool gbDebugMode;
extern bool gbExcludeInit;
extern bool gbForceAllowMultiInstance;
extern bool gbUseTrustedConnection; //SQL Connection Flag.
extern bool gbUseTCPIPConnection; //SQL Connection Flag.
extern bool gbUseApplicationName; //SQL Connection Flag.
extern bool gbNextConnectTimeCalculated;
extern bool gbUseProcessBoost;
extern bool gbUseThreadBoost;
extern bool gbUseNTFibers;
extern bool gbThreadFiberConfigChanged;
extern bool gbAllowOffLineDataProcessing;
extern bool gbTrimStringDataOnExport;
extern bool gbUseEncryption;
extern bool gbRequestInitialization;

extern int giNextDay;			 // The day on which the next connection will occur.
extern int giTransProcessed;

extern DWORD_PTR gdwCPUAffinityMask;
extern DWORD gdwConnectInterval; // The interval on which each connection will occur.
extern DWORD gdwErrorCount;
extern DWORD gdwMaxClients;
extern DWORD gdwNextTime;		 // The time at which the next connection will occur.
extern DWORD gdwServerPort;
extern DWORD gdwStartTime;		 // The beginning of the connection time window.
extern DWORD gdwStopTime;		 // The End of the connection time window.
extern DWORD gdwTotalConnects;
extern DWORD gdwWarningCount;
extern DWORD gdwSQLServerPort;   // Used when gbUseTCPIPConnection = true
extern DWORD gdwSQLTimeout;
extern DWORD gdwMaxInactivityTime;
extern DWORD gdwConnectInterval;

extern HANDLE Timer_Thread_Handle;

extern FILE *gfhSessionLog;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#endif
