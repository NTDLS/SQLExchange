///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//  Copyright © NetworkDLS 2002, All rights reserved
//
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF 
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO 
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A 
// PARTICULAR PURPOSE.
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef _Common_H
#define _Common_H
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "../../@Libraries/CSocketServer/CSocketServer.H"
#include "../../@Libraries/CSQLBinding/CSQLEncapsulation.H"
#include "../../@Libraries/CXML/CXML.H"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#define OPERATION_INSERT	1 //Used by SQL Import/Export.
#define OPERATION_UPDATE	2 //Used by SQL Import/Export.
#define OPERATION_DELETE	3 //Used by SQL Import/Export.

#define EVENT_NONE			0
#define EVENT_INFO			1
#define EVENT_WARN			2
#define EVENT_ERROR			3
#define EVENT_AUDIT_SUCCESS	4
#define EVENT_AUDIT_FAILURE	5

#define INIT_CUSTOM_PROC (WM_USER + 1000)

#define FatalError(sErrorMsg) FatalErrorEx(__FILE__, __FUNCTION__, __LINE__, sErrorMsg)

typedef struct _TAG_CLIENT_SCHEDULE{
	bool bSunday;
	bool bMonday;
	bool bTuesday;
	bool bWednesday;
	bool bThursday;
	bool bFriday;
	bool bSaturday;

	DWORD dwBeginTime;
	DWORD dwEndTime;

	int iIncrement;

} CLIENT_SCHEDULE, *LPCLIENT_SCHEDULE;

extern CRITICAL_SECTION csLogFile;

extern DWORD dwExpiryDate;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool CloseLogFile(void);
bool GetIPAddress(char *sOutAddr, int iMaxSz, const char *sInputAddr);
bool DoesServerFileExist(const char *sFile);
bool OpenLogFile(void);
bool ExcludedFromInit(const char *sCmdLine);
bool IsApplicationRunning(void);
bool IsApplicationRunningEx(void);
bool OnlyGoodForOneYear(void);
bool InitExpiryDate(void);
bool CreateNewDB(CSQL *lpMySQL, const char *sDBName);
bool DoesDatabaseExist(CSQL *lpMySQL, const char *sDB);

char *ResolveHost(const char *sHostName, int iAddress);
const char *GetBuildArchitecture(void);
char *GetBuildArchitecture(const char *sBase, int iMaxSz, char *sOutString);
char *GetBuildArchitecture(const char *sBase, const char *sPrefix, int iMaxSz, char *sOutString);

bool SendCommand(CSocketServer *pSock, CSocketClient *pClient, const char *sCmd,
				 const void *sData, const int iDataSz, void *sUseBuffer);
bool SendCommand(CSocketClient *pClient, CXMLWriter *lpXML);

bool SendCommandWithResponse(CSocketClient *pClient, CXMLWriter *lpXML, LPBASICHUNK pChunk);
bool SendCommand(CSocketServer *pSock, CSocketClient *pClient, const char *sCmd);

int AppendDataToCmd(const char *sCmd, const void *sData, const int iDataSz, void *sOutBuf);
int AppendDataToCmd(const char *sCmd, CXMLWriter *MyXML, void *sOutBuf, int iMaxSz);
LPARAM GetListItemlParam(HWND hWnd, int iItem, int iSubItem);
int SQLErrorHandler(void *pCaller, const char *sSource, const char *sErrorMsg, const int iErrorNumber);

void WriteAnEvent(const char *sMessage, int iEventType);
void WriteLog(int iClientId, const char *sMessage);
void WriteLog(int iClientId, const char *sMessage, int iEventType);
void WriteSysLog(const char *sMessage);
void WriteSysLogEx(const char *sMessage, int iEventType);
void FatalErrorEx(const char *sFile, const char *sProc, int iLine, char *sErrorMsg);
bool ConnectToSQLServer(CSQL *lpSQL, const char *sServer, const char *sUID, const char *sPWD,
	const char *sDatabase, bool bUseTCPIPConnection, bool bUseTrustedConnection, int iSQLServerPort);
int StrRepInPlace(char *sInOutBuf, int iMaxOutSz, const char *sReplace, const char *sWith);

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#endif
