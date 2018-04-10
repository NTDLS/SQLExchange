///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//  Copyright © NetworkDLS 2002, All rights reserved
//
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF 
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO 
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A 
// PARTICULAR PURPOSE.
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef _Common_CPP
#define _Common_CPP
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include <Windows.H>
#include <ShlObj.H>
#include <Stdio.H>
#include <ShlOBJ.H>
#include <Stdlib.H>
#include <Share.H>

#ifdef _SQLESERVER
	#include "../SQLEServer/Source/Entry.H"
	#include "../SQLEServer/Source/Routines.H"
	#include "../SQLEServer/Dialogs/AlertDlg.H"
#elif _SQLECLIENT
	#include "../SQLEClient/Source/Entry.H"
	#include "../SQLEClient/Source/Routines.H"
#else
	#error "You must define either _SQLESERVER or _SQLECLIENT"
#endif

#include "Common.H"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

using namespace NSWFL::DateTime;
using namespace NSWFL::String;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

CRITICAL_SECTION csLogFile;

int iLogFileDay = 0;
int iLogFileMonth = 0;
int iLogFileYear = 0;
DWORD dwExpiryDate = 0;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

char *sExcludedFromInit[] = {
	"/OpenConsole",
	"/OpenMainDialog",
	"/Install",
	"/Start",
	"/Delete",
	"/Stop",
	NULL
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int SQLErrorHandler(void *pCaller, const char *sSource, const char *sErrorMsg, const int iErrorNumber)
{
	WriteSysLog(sErrorMsg);
	//MessageBox(NULL, sErrorMsg, sSource, 0);
	return 0;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool OnlyGoodForOneYear(void)
{
	SYSTEMTIME ST;
	memset(&ST, 0, sizeof(ST));

	GetLocalTime(&ST);

	DWORD dwToday = DateToLong(ST.wYear, ST.wMonth, ST.wDay);

	if(dwToday < dwExpiryDate)
	{
		return false; //not Expired.
	}

	return true; //expired.
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool InitExpiryDate(void)
{
	char *sMonths[] = {"Jan","Feb","Mar","Apr","May","Jun","Jul","Aug","Sep","Oct","Nov","Dec",NULL};
	char sRawDate[100];

	ReplaceStrings(__DATE__, "  ", " ", sRawDate, sizeof(sRawDate));

	char **sDate = Split(sRawDate, " ");

	if(UBound(sDate) != 3)
	{
		return false;
	}

	for(int iMonth = 0; sMonths[iMonth] != NULL; iMonth++)
	{
		if(_strcmpi(sDate[0], sMonths[iMonth]) == 0)
		{
			dwExpiryDate = DateToLong(atol(sDate[2]) + 1, iMonth + 1, atol(sDate[1]));
			FreeArray(sDate);
			return true;
		}
	}

	dwExpiryDate = -1;

	return false;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

char *GetBuildArchitecture(const char *sBase, const char *sPrefix, int iMaxSz, char *sOutString)
{
	strcpy_s(sOutString, iMaxSz, sBase);
	strcat_s(sOutString, iMaxSz, "_");
	strcat_s(sOutString, iMaxSz, sPrefix);

	#ifdef _X64_
		strcat_s(sOutString, iMaxSz, " x64");
	#elif _IA64_
		strcat_s(v, iMaxSz, " IA64");
	#else
		strcat_s(sOutString, iMaxSz, " x86");
	#endif

	return sOutString;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

char *GetBuildArchitecture(const char *sBase, int iMaxSz, char *sOutString)
{
	strcpy_s(sOutString, iMaxSz, sBase);

	#ifdef _X64_
		strcat_s(sOutString, iMaxSz, " x64");
	#elif _IA64_
		strcat_s(v, iMaxSz, " IA64");
	#else
		strcat_s(sOutString, iMaxSz, " x86");
	#endif

	return sOutString;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

const char *GetBuildArchitecture(void)
{
	#ifdef _X64_
		return "x64";
	#elif _IA64_
		return "IA64";
	#else
		return "x86";
	#endif

	return NULL;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool IsApplicationRunning(void)
{
	if(IsApplicationRunningEx())
	{
		if(!gbForceAllowMultiInstance)
		{
			MessageBox(NULL, "The application is already running.", gsTitleCaption, MB_TASKMODAL|MB_ICONINFORMATION);
			return true;
		}
	}
	return false;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool IsApplicationRunningEx(void)
{
	bool bResult = false;
	HANDLE hMutex = NULL;
	SECURITY_DESCRIPTOR SD;
	SECURITY_ATTRIBUTES SA;

	if(InitializeSecurityDescriptor(&SD, SECURITY_DESCRIPTOR_REVISION) == FALSE)
	{
		return false;
	}

	if(SetSecurityDescriptorDacl(&SD, TRUE, 0, FALSE) == FALSE)
	{
		return false;
	}

	SA.nLength				= sizeof(SA);
	SA.lpSecurityDescriptor = &SD;
	SA.bInheritHandle		= FALSE;

	char sMutexName[1024];
	if((hMutex = CreateMutex(&SA, FALSE, GetBuildArchitecture(gsTitleCaption, sizeof(sMutexName), sMutexName))) != NULL)
	{
		if(GetLastError() == ERROR_ALREADY_EXISTS)
		{
			bResult = true;
		}

		ReleaseMutex(hMutex);
		CloseHandle(hMutex);
	}

	return bResult;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool ExcludedFromInit(const char *sCmdLine)
{
	int iItem = 0;

	if(strlen(sCmdLine) == 0)
	{
		return false;
	}

	while(sExcludedFromInit[iItem])
	{
		if(_strcmpi(sExcludedFromInit[iItem], sCmdLine) == 0)
		{
			return true;
		}

		iItem++;
	}

	return false;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*
	This function would require that XP_CMDSHELL be enabled, it is
		NOT enabled by default and would be a security risk if enabled.
*/
bool DoesServerFileExist(const char *sFile)
{
/*
	CSQL MySQL;
	CBoundRecordSet rsTemp;

	char sBuf[1024];

	int iLen = 0;

	memset(sBuf, 0, sizeof(sBuf));

	sprintf_s(sBuf, sizeof(sBuf), "XP_CmdShell 'IF EXIST \"%s\" ECHO FILE_EXIST'", sFile);

	if(MySQL.Connect(gsSQLServer, gsSQLUserID, gsSQLPassword, "master", &SQLErrorHandler))
	{
		if(MySQL.Execute(sBuf, &rsTemp))
		{
			memset(sBuf, 0, sizeof(sBuf));

			if(rsTemp.Fetch())
			{
				if(rsTemp.sColumnEx(1, sBuf, sizeof(sBuf), &iLen))
				{
					if(_strcmpi(sBuf, "FILE_EXIST") == 0)
					{
						return true;
					}
				}
			}
		}
	}
*/
	return false;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

LPARAM GetListItemlParam(HWND hWnd, int iItem, int iSubItem)
{
	LV_ITEM MyLVItem;

	memset(&MyLVItem, 0, sizeof(MyLVItem));
	MyLVItem.mask       = LVIF_PARAM;
    MyLVItem.iSubItem   = iSubItem;
	MyLVItem.iItem      = iItem;

	if(ListView_GetItem(hWnd, &MyLVItem))
	{
		return MyLVItem.lParam;
	}

	return 0;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool OpenLogFile(void)
{
    char sFullPath[MAX_PATH];

    SYSTEMTIME ST;

    GetLocalTime(&ST);

    char DateTemp[64 + 1];

    GetDateFormat(LOCALE_USER_DEFAULT, NULL, &ST, "MM-dd-yy", DateTemp,64);

	sprintf_s(sFullPath, sizeof(sFullPath), "%s\\%s", gsSessionLogDir, DateTemp);

	iLogFileDay = ST.wDay;
	iLogFileMonth = ST.wMonth;
	iLogFileYear = ST.wYear;

	if(gbForceAllowMultiInstance)
	{
		char sUnique[100];
		srand(GetTickCount());
		sprintf_s(sUnique, sizeof(sUnique), " (%d%d)", rand(), GetTickCount());
	    strcat_s(sFullPath, sizeof(sFullPath), sUnique);
	}

    strcat_s(sFullPath, sizeof(sFullPath), ".txt");

	if((gfhSessionLog = _fsopen(sFullPath, "ab", _SH_DENYWR)) == NULL)
	{
        return false;
	}

	fprintf(gfhSessionLog, "########################################################################################################\r\n");
	fprintf(gfhSessionLog, "#\r\n");
	fprintf(gfhSessionLog, "# %s (%s)\r\n", gsTitleCaption, gsFileVersion);
	fprintf(gfhSessionLog, "#\r\n");
	fprintf(gfhSessionLog, "########################################################################################################\r\n");
	fprintf(gfhSessionLog, "#\r\n");
	if(gbCacheLogFile)
	{
		fprintf(gfhSessionLog, "# Log file caching is on, so this file will not be up-to-date untill the file is flushed.\r\n");
	}
	else if(gbDebugMode)
	{
		fprintf(gfhSessionLog, "# Debug mode is enabled, this file contains current data.\r\n");
	}
	else{
		fprintf(gfhSessionLog, "# Log file caching is off, this file contains current data.\r\n");
	}
	fprintf(gfhSessionLog, "#\r\n");
	fprintf(gfhSessionLog, "# Message level indicators are as follows:\r\n");
	fprintf(gfhSessionLog, "#\t0: Verbose information, not logged to event logs.\r\n");
	fprintf(gfhSessionLog, "#\t1: Informational Only\r\n");
	fprintf(gfhSessionLog, "#\t2: Warning\r\n");
	fprintf(gfhSessionLog, "#\t3: Error\r\n");
	fprintf(gfhSessionLog, "#\t4: Success Audit\r\n");
	fprintf(gfhSessionLog, "#\t5: Fail Audit\r\n");
	fprintf(gfhSessionLog, "#\r\n");
	fprintf(gfhSessionLog, "########################################################################################################\r\n");
	fprintf(gfhSessionLog, "#\r\n");
	fprintf(gfhSessionLog, "# Log file format:\r\n");
	fprintf(gfhSessionLog, "#\t(Date Time) [Warning Level] [Client ID] Text message.\r\n");
	fprintf(gfhSessionLog, "#\r\n");
	fprintf(gfhSessionLog, "########################################################################################################\r\n");

    return true;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool CloseLogFile(void)
{
	if(gfhSessionLog)
	{
		fclose(gfhSessionLog);
		gfhSessionLog = NULL;
		return true;
	}
	else return false;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void WriteAnEvent(const char *sMessage, int iEventType)
{
	if(iEventType == EVENT_NONE)
	{
		//No need to record an event.
	}
	else if(iEventType == EVENT_INFO)
	{
		RecordInfo(gsEventName, sMessage);
	}
	else if(iEventType == EVENT_WARN)
	{
		RecordWarn(gsEventName, sMessage);
		gdwWarningCount++;
	}
	else if(iEventType == EVENT_ERROR)
	{
		RecordError(gsEventName, sMessage);
		gdwErrorCount++;
	}
	else if(iEventType == EVENT_AUDIT_SUCCESS)
	{
		RecordSuccessAudit(gsEventName, sMessage);
	}
	else if(iEventType == EVENT_AUDIT_FAILURE)
	{
		RecordFailureAudit(gsEventName, sMessage);
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void DoLogWrite(const char *sMessage)
{
	EnterCriticalSection(&csLogFile);

	if(MDI.hWnd || !gService.RunningAsService())
	{
		MDI.Queu.Push(sMessage);
	}

	SYSTEMTIME ST;

	GetLocalTime(&ST);

	if(iLogFileDay != ST.wDay || iLogFileMonth != ST.wMonth || iLogFileYear != ST.wYear)
	{
		fprintf(gfhSessionLog, "########################################################################################################\r\n");
		fprintf(gfhSessionLog, "# Cycling log file\r\n");
		fprintf(gfhSessionLog, "########################################################################################################\r\n");

		CloseLogFile();
		OpenLogFile();
	}

	fprintf(gfhSessionLog, "%s\r\n", sMessage);
 	
	if(!gbCacheLogFile || gbDebugMode)
	{
		fflush(gfhSessionLog);
	}

	LeaveCriticalSection(&csLogFile);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void WriteSysLogEx(const char *sMessage, int iEventType)
{
	if(!gbExcludeInit && gfhSessionLog)
	{
		SYSTEMTIME ST;
		GetLocalTime(&ST);

		char lsDate[64];
		char lsTime[64];
		char lsTemp[MAX_STATUS_TEXT + (64 * 2) + 1];
		unsigned short iMsgLevel = 0;

		GetDateFormat(LOCALE_USER_DEFAULT, DATE_SHORTDATE, &ST, NULL, lsDate, 64);
		GetTimeFormat(LOCALE_USER_DEFAULT, 0, &ST, NULL, lsTime, 64);

		sprintf_s(lsTemp, sizeof(lsTemp), "(%s %s) [%d] [*] %s", lsDate, lsTime, iEventType, sMessage);
		DoLogWrite(lsTemp);

		WriteAnEvent(lsTemp, iEventType);

		#ifdef _SQLESERVER
		if(iEventType == EVENT_WARN && gbVisualAlertOnWarn)
		{
			StartAlertDlg("Warning", sMessage, true);
		}

		if(iEventType == EVENT_ERROR && gbVisualAlertOnError)
		{
			StartAlertDlg("Error", sMessage, true);
		}
		#endif
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void WriteSysLog(const char *sMessage)
{
	WriteSysLogEx(sMessage, EVENT_NONE);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void WriteLog(int iClientId, const char *sMessage, int iEventType)
{
	if(!gbExcludeInit && gfhSessionLog)
	{
		SYSTEMTIME ST;
		GetLocalTime(&ST);

		char lsDate[64];
		char lsTime[64];
		char lsTemp[MAX_STATUS_TEXT + (64 * 2) + 1];
		unsigned short iMsgLevel = 0;

		GetDateFormat(LOCALE_USER_DEFAULT, DATE_SHORTDATE, &ST, NULL, lsDate, 64);
		GetTimeFormat(LOCALE_USER_DEFAULT, 0, &ST, NULL, lsTime, 64);

		sprintf_s(lsTemp, sizeof(lsTemp), "(%s %s) [%d] [%d] %s", lsDate, lsTime, iEventType, iClientId, sMessage);
		DoLogWrite(lsTemp);

		WriteAnEvent(lsTemp, iEventType);

		#ifdef _SQLESERVER
		if(iEventType == EVENT_WARN && gbVisualAlertOnWarn)
		{
			StartAlertDlg("Warning", sMessage, true);
		}

		if(iEventType == EVENT_ERROR && gbVisualAlertOnError)
		{
			StartAlertDlg("Error", sMessage, true);
		}
		#endif
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void WriteLog(int iClientId, const char *sMessage)
{
	WriteLog(iClientId, sMessage, EVENT_NONE);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool SendCommandWithResponse(CSocketClient *pClient, CXMLWriter *lpXML, LPBASICHUNK pChunk)
{
	pClient->WaitForEmptyRecvBuffer();

	if(SendCommand(pClient, lpXML))
	{
		if(pClient->GetNextClientData(pChunk))
		{
			((char *)pChunk->pBuffer)[pChunk->iLength] = '\0';
			return true;
		}
	}

	return false;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool SendCommand(CSocketClient *pClient, CXMLWriter *lpXML)
{
	int iBufferSize = 0;
	char *sBuffer = lpXML->Text(&iBufferSize);

	if(sBuffer)
	{
		if(iBufferSize > 0)
		{
			pClient->SetNextSendData(sBuffer, iBufferSize);
			return true;
		}
		free(sBuffer);
	}

	return false;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool SendCommand(CSocketServer *pSock, CSocketClient *pClient, const char *sCmd)
{
	return pClient->SetNextSendData(sCmd, (int)strlen(sCmd));
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool SendCommand(CSocketServer *pSock, CSocketClient *pClient, const char *sCmd,
				 const void *sData, const int iDataSz, void *sOutBuf)
{
	int iSz = AppendDataToCmd(sCmd, sData, iDataSz, sOutBuf);
	return pClient->SetNextSendData((char *)sOutBuf, iSz);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int AppendDataToCmd(const char *sCmd, CXMLWriter *lpXML, void *sOutBuf, int iMaxSz)
{
	int iCmdSz = (int)strlen(sCmd);
	int iRWos = 0;

	while(iRWos < iCmdSz)
	{
		((char *)sOutBuf)[iRWos] = sCmd[iRWos];
		iRWos++;
	}

	iRWos += lpXML->Text((char *)sOutBuf + iRWos, iMaxSz - iRWos);

	return iRWos;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int AppendDataToCmd(const char *sCmd, const void *sData, const int iDataSz, void *sOutBuf)
{
	int iCmdSz = (int)strlen(sCmd);
	int iRPos = 0;
	int iRWos = 0;

	while(iRPos < iCmdSz)
	{
		((char *)sOutBuf)[iRWos++] = sCmd[iRPos++];
	}

	iRPos = 0;

	while(iRPos < iDataSz)
	{
		((char *)sOutBuf)[iRWos++] = ((char *)sData)[iRPos++];
	}

	return iRWos;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int BreakCmdFromData(const void *sBuf, const int iCmdLen, const int iBufSz, void *sOutBuf)
{
	int iRPos = iCmdLen;
	int iWPos = 0; 

	while(iRPos < iBufSz)
	{
		((char *)sOutBuf)[iWPos++] = ((char *)sBuf)[iRPos++];
	}

	((char *)sOutBuf)[iWPos] = '\0';

	return iWPos;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

char *ResolveHost(const char *sHostName, int iAddress)
{
	struct in_addr Address;

	struct hostent *Host;

	memset(&Host, 0, sizeof(Host));
	memset(&Address, 0, sizeof(Address));

	if((Host = gethostbyname(sHostName)) == NULL)
	{
		return NULL;
	}

	memcpy(&Address.s_addr, (*Host).h_addr_list[iAddress], (*Host).h_length);

	return inet_ntoa(Address);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool GetIPAddress(char *sOutAddr, int iMaxSz, const char *sInputAddr)
{
	bool ValidIP = true;

    if(inet_addr(sInputAddr) == INADDR_NONE) ValidIP = false;
    if(strlen(sInputAddr) < 7)               ValidIP = false;
    if(strlen(sInputAddr) > 15)              ValidIP = false;

	if(ValidIP == false)
	{
		char *Resolved = ResolveHost(sInputAddr, 0);
		if(Resolved == NULL)
		{
			return false;
		}
		strcpy_s(sOutAddr, iMaxSz, Resolved);
	}
	else{
		strcpy_s(sOutAddr, iMaxSz, sInputAddr);
	}

	return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void FatalErrorEx(const char *sFile, const char *sProc, int iLine, char *sErrorMsg)
{
    #ifdef _DEBUG_SOCK_SERVER
    printf("DEBUG: [In Proc]> FatalError.\n");
    #endif

    char sErrorMessage[5120];
    sprintf_s(sErrorMessage, sizeof(sErrorMessage),
		"Error in: \"%s\"\r\n"
		"Within: \"%s\"\r\n"
		"On line: %d\r\n"
		"Message: %s",
		sFile, sProc, iLine, sErrorMsg);

	WriteSysLogEx(sErrorMessage, EVENT_ERROR);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool ConnectToSQLServer(CSQL *lpSQL, const char *sServer, const char *sUID, const char *sPWD,
	const char *sDatabase, bool bUseTCPIPConnection, bool bUseTrustedConnection, int iSQLServerPort)
{
	SQLCONNECTIONSTRING SCS;
	memset(&SCS, 0, sizeof(SCS));

	if(gbUseApplicationName)
	{
		char sArch[255];
		GetBuildArchitecture(gsTitleCaption, sizeof(sArch), sArch);

		sprintf_s(SCS.sApplicationName, sizeof(SCS.sApplicationName), "%s (%s)", sArch, gsFileVersion);
	}

	SCS.bUseTCPIPConnection = bUseTCPIPConnection;
	SCS.bUseTrustedConnection = bUseTrustedConnection;
	SCS.iPort = iSQLServerPort;

	strcpy_s(SCS.sDatabase, sizeof(SCS.sDatabase), sDatabase);
	strcpy_s(SCS.sDriver, sizeof(SCS.sDriver), gsSQLDriver);
	strcpy_s(SCS.sPwd, sizeof(SCS.sPwd), sPWD);
	strcpy_s(SCS.sServer, sizeof(SCS.sServer), sServer);
	strcpy_s(SCS.sUID, sizeof(SCS.sUID), sUID);

	bool bResult = lpSQL->Connect(&SCS, &SQLErrorHandler);

	lpSQL->ThrowErrors(true);

	if(bResult)
	{
		lpSQL->TimeOut(gdwSQLTimeout);
	}

	return bResult;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool DoesDatabaseExist(CSQL *lpMySQL, const char *sDB)
{
	bool bExists = false;

	char sTemp[5120];
	CBoundRecordSet rsExist;

	sprintf_s(sTemp, sizeof(sTemp),
		"SELECT COUNT(0) AS RecordCount"
		" FROM [sysdatabases]"
		" WHERE [name] = '%s'", sDB);
	lpMySQL->Execute("master", sTemp, &rsExist);

	if(rsExist.Fetch())
	{
		bExists = (rsExist.Values("RecordCount")->ToIntegerS() > 0);
	}

	rsExist.Close();

	return bExists;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool CreateNewDB(CSQL *lpMySQL, const char *sDBName)
{
	bool bResult = false;

	const char *sBaseSQL = 
		"CREATE DATABASE [%s] ON"
		"("
		    "NAME = 'SQLExch_%s_Dat',"
			"FILENAME = '%s\\%s_Dat.mdf',"
			"SIZE = 10,"
			"FILEGROWTH = 5"
		")"
		"LOG ON("
		    "NAME = 'SQLExch_%s_Log',"
			"FILENAME = '%s\\%s_Log.ldf',"
			"SIZE = 5MB,"
			"FILEGROWTH = 5MB"
		");"
		"ALTER DATABASE %s SET RECOVERY SIMPLE;";

	int iMemoryRequired =
		(int)strlen(sBaseSQL)			//Base SQL.
		+ ((int)strlen(sDBName) * 6)		//Database name referenced 6 times.
		+ (int)strlen(gsSQLDataFiles)	//The SQL Database file location.
		+ (int)strlen(gsSQLLogFiles)		//The SQL log file location.
		+ 1;						//The NULL terminator.

	char *sSQL = (char *) calloc(iMemoryRequired, sizeof(char));
	Assert(!sSQL, "CreateNewDB :: Memory Allocation Error.");

	sprintf_s(sSQL, iMemoryRequired, sBaseSQL, sDBName, sDBName,
		gsSQLDataFiles, sDBName, sDBName, gsSQLLogFiles, sDBName, sDBName);

	if(lpMySQL->Execute(sSQL))
	{
		bResult = DoesDatabaseExist(lpMySQL, sDBName);
	}

	free(sSQL);

	return bResult;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int StrRepInPlace(char *sInOutBuf, int iMaxOutSz, const char *sReplace, const char *sWith)
{
	char *sReplBuf = NULL;
	int iBufSz = (int)strlen(sInOutBuf);
	int iPos = 0;
	int iOccurrences  = 0; // The number of time "sReplace" occurs in "sBuf".
	int iMemRequired = 0;
	int iMemDiff = iMemDiff = (int)strlen(sWith) - (int)strlen(sReplace);

	while( (iPos = InStr(sReplace, sInOutBuf, iBufSz, iPos)) > 0)
	{
		iPos++;
		iOccurrences++;
	}

	if(iOccurrences == 0)
	{
		return 0;
	}

	iMemRequired = (iBufSz + (iMemDiff * iOccurrences)) + 1;

	sReplBuf = (char *) pMem->Allocate(sizeof(char), iMemRequired);
	Assert(!sReplBuf, "Memory Allocation Error.");

	ReplaceStrings(sInOutBuf, sReplace, sWith, sReplBuf, iMemRequired);
	memcpy_s(sInOutBuf, iMaxOutSz, sReplBuf, iMemRequired);
	pMem->Free(sReplBuf);

	return iOccurrences;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#endif
