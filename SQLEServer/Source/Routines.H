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

#define CLIENTVER_AUTOUPDATE 0x1
#define CLIENTVER_CURRENT    0x2
#define CLIENTVER_ERROR      0x3
#define CLIENTVER_OUTDATED   0x4
#define CLIENTVER_SUPPORTED  0x5

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool GetRegistryValues(void);
bool SaveRegistryValues(void);
bool IsServerConfigured(bool bAlertUser);
bool StartServer(void);
bool StopServer(void);
bool LaunchConfigDlg(HWND hOwner);
void UpdateStatistics(void);

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

struct _MainDialogInfo{
	HWND hStatusBar;
	HWND hWnd;
	HMENU hMenu;
	HWND hLogList;
	CQueu Queu;
}; extern _MainDialogInfo MDI;

struct _ConnectedClientInfo{
	char sCompanyName[64];
	char sClientVersion[16];
	char sCompanyDBPrefix[50];
	
	char sTransDatabase[255];
	char sTransTable[255];
	char sTransSchema[255];

	bool bIsAuthenticated;
	bool bIsEncryptionInit;
	bool bIndexConnected;
	bool bCustomerConnected;
	bool bRequestInit;
	bool bUseACK;
	bool bAllowOffLineDataProcessing;

	SYSTEMTIME stConnectTime;

	DWORD dwBytesRecv;
	DWORD dwBytesSent;
	DWORD dwCorruptInitRetry;
	DWORD dwTransFileSize;
	
	int iInitStep;
	CSocketClient *pClient;

	CNASCCL SC;

	CSQL cIndexSQL;
	CSQL cCustSQL;

}; extern _ConnectedClientInfo *CCI;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

extern CSocketServer gServer;

extern volatile int giTransProcessed;
extern CRITICAL_SECTION csTransPerSecond;

extern char gsPath[MAX_PATH];
extern char gsSessionLogDir[MAX_PATH];
extern char gsImportTempDir[MAX_PATH];
extern char gsAuthKey[MAX_AUTH_LENGTH];
extern char gsAuthString[MAX_AUTH_LENGTH];
extern char gsCompressionMethod[255];
extern char gsGUIPWDHash[256];

extern char gsSQLDriver[255];
extern char gsSQLCustPassword[255];
extern char gsSQLCustServer[255];
extern char gsSQLCustUserID[255];
extern DWORD gdwSQLCustServerPort; //Used when gbSQLCustUseTCPIPConnection = true
extern bool gbSQLCustUseTCPIPConnection; //SQL Connection Flag.
extern bool gbSQLCustUseTrustedConnection; //SQL Connection Flag.

extern char gsSQLDataFiles[MAX_PATH];
extern char gsSQLLogFiles[MAX_PATH];
extern char gsDefaultDBO[255];

extern char gsSQLIndexDatabase[255];
extern char gsSQLIndexPassword[255];
extern char gsSQLIndexServer[255];
extern char gsSQLIndexUserID[255];
extern DWORD gdwSQLIndexServerPort; //Used when gbSQLIndexUseTCPIPConnection = true
extern bool gbSQLIndexUseTCPIPConnection; //SQL Connection Flag.
extern bool gbSQLIndexUseTrustedConnection; //SQL Connection Flag.

extern DWORD_PTR gdwCPUAffinityMask;
extern DWORD gdwCompressionLevel;
extern DWORD gdwErrorCount;
extern DWORD gdwHashTimeout;
extern DWORD gdwKeyGenLength;
extern DWORD gdwListenPort;
extern DWORD gdwMaxClients;
extern DWORD gdwNextClientID;
extern DWORD gdwMaxInactivityTime;
extern DWORD gdwSQLTimeout;
extern DWORD gdwRawHashLen;
extern DWORD gdwTotalConnects;
extern DWORD gdwWarningCount;
extern DWORD gdwMaxRowsPerTransaction;

extern bool gbDebugMode;
extern bool gbConsoleOpen;
extern bool gbShowSplashScreen;
extern bool gbStartAutomatically;
extern bool gbCacheLogFile;
extern bool gbRTOFDWI;
extern bool gbExcludeInit;
extern bool gbVisualAlertOnError;
extern bool gbVisualAlertOnWarn;
extern bool gbForceAllowMultiInstance;
extern bool gbUseApplicationName; //SQL Connection Flag.
extern bool gbUseProcessBoost;
extern bool gbUseThreadBoost;
extern bool gbUseNTFibers;
extern bool gbThreadFiberConfigChanged;
extern bool gbAllowOffLineDataProcessing;
extern bool gbUseEncryption;

extern CNASCCL gSC;

extern FILE *gfhSessionLog;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#endif
