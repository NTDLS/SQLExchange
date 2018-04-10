///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//  Copyright © NetworkDLS 2002, All rights reserved
//
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF 
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO 
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A 
// PARTICULAR PURPOSE.
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef _ROUTINES_CPP
#define _ROUTINES_CPP
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include <Windows.H>
#include <Stdio.H>
#include <Stdlib.H>
#include <CommCtrl.h>
#include <Time.H>
#include <PSAPI.H>

#include "Entry.H"
#include "Command.H"
#include "Routines.H"

#include "../Dialogs/MainDlg.H"
#include "../Dialogs/TabAccount.H"
#include "../Dialogs/TabAdvanced.H"
#include "../Dialogs/TabGeneral.H"
#include "../Dialogs/TabSecurity.H"
#include "../Dialogs/TabSQL.H"
#include "../Dialogs/TabSched.H"
#include "../Dialogs/TabProcessors.H"
#include "../Dialogs/ReplicationDlg.H"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

using namespace NSWFL::DateTime;
using namespace NSWFL::String;
using namespace NSWFL::Registry;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

const char *gsWeekDayName[] = {
    "Sunday",
    "Monday",
    "Tuesday",
    "Wedensday",
    "Thursday",
    "Friday",
    "Saturday",
	NULL
};

CSocketServer gServer;

bool gbWeekDays[7] = {0,1,1,1,1,1,0};
bool gbDfltWeekDays[7] = {0,1,1,1,1,1,0};

char gsSessionLogDir[MAX_PATH];
char gsTempFilesPath[MAX_PATH];
char gsPath[MAX_PATH];
char gsServerAddress[255];
char gsGUIPWDHash[256];

char gsAuthKey[MAX_AUTH_LENGTH];
char gsAuthString[MAX_AUTH_LENGTH];

char gsSQLDataFiles[MAX_PATH]; // Relative to server.
char gsSQLLogFiles[MAX_PATH]; // Relative to server.
char gsDefaultDBO[255];
char gsReplicationDB[255];

char gsCompanyName[64];
char gsCompanyPassword[64];

char gsSQLDriver[255];
char gsSQLPassword[255];
char gsSQLServer[255];
char gsSQLUserID[255];

bool gbReportNoTransactions = false;
bool gbShowSplashScreen = false;
bool gbConnectNow = false;
bool gbPacketMode = true;
bool gbAutoConnect = false;
bool gbCacheLogFile = false;
bool gbDebugMode = false;
bool gbExcludeInit = false;
bool gbForceAllowMultiInstance = false; //This is a debug flag, to be set at compile time.
bool gbUseTrustedConnection = false; //SQL Connection Flag.
bool gbUseTCPIPConnection = false; //SQL Connection Flag.
bool gbUseApplicationName = false; //SQL Connection Flag.
bool gbNextConnectTimeCalculated = false;
bool gbUseProcessBoost = false;
bool gbUseThreadBoost = false;
bool gbUseNTFibers = false;
bool gbThreadFiberConfigChanged = false;
bool gbAllowOffLineDataProcessing = false;
bool gbTrimStringDataOnExport = false;
bool gbUseEncryption = true;
bool gbRequestInitialization = false;

int giNextDay = -1;
int giTransProcessed = 0;

DWORD_PTR gdwCPUAffinityMask = 0;
DWORD gdwConnectInterval = 0;
DWORD gdwMaxInactivityTime = 0;
DWORD gdwErrorCount = 0;
DWORD gdwMaxClients = 1;
DWORD gdwNextTime = 0;
DWORD gdwServerPort = 0;
DWORD gdwStartTime = 0;
DWORD gdwStopTime = 0;
DWORD gdwTotalConnects = 0;
DWORD gdwWarningCount = 0;
DWORD gdwSQLServerPort = 0;
DWORD gdwSQLTimeout = 30;

HANDLE Timer_Thread_Handle = NULL;

CRITICAL_SECTION csTransPerSecond;

FILE *gfhSessionLog = NULL;

CNASCCL gSC;

_MainDialogInfo MDI;
_ConnectedClientInfo CCI;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool IsClientConfigured(bool bAlertUser)
{
	char sMsg[2048];
	char sText[2048];

	HWND hActive = GetActiveWindow();	

	//bool gbWeekDays[7] = {0,1,1,1,1,1,0};

	strcpy_s(sText, sizeof(sText), "");

	if(strlen(gsSessionLogDir) == 0)
	{
		if(!bAlertUser) return false;
		strcat_s(sText, "\tThe 'Session Log Directory' is not set\r\n");
	}

	if(strlen(gsTempFilesPath) == 0)
	{
		if(!bAlertUser) return false;
		strcat_s(sText, "\tThe 'Temp File Directory' is not set\r\n");
	}
	
	if(strlen(gsPath) == 0)
	{
		if(!bAlertUser) return false;
		strcat_s(sText, sizeof(sText), "\tThe SQL Exchange client 'Path' is not set\r\n");
	}
	
	if(strlen(gsServerAddress) == 0)
	{
		if(!bAlertUser) return false;
		strcat_s(sText, sizeof(sText), "\tThe SQL Exchange 'Server Address' is not set\r\n");
	}

	if(strlen(gsAuthKey) == 0)
	{
		if(!bAlertUser) return false;
		strcat_s(sText, sizeof(sText), "\tThe 'Auth Key' is not set\r\n");
	}
	
	if(strlen(gsAuthString) == 0)
	{
		if(!bAlertUser) return false;
		strcat_s(sText, sizeof(sText), "\tThe 'Auth String' is not set\r\n");
	}

	if(strlen(gsSQLDataFiles) == 0)
	{
		if(!bAlertUser) return false;
		strcat_s(sText, sizeof(sText), "\tThe 'SQL Data Files' location is not set\r\n");
	}

	if(strlen(gsSQLLogFiles) == 0)
	{
		if(!bAlertUser) return false;
		strcat_s(sText, sizeof(sText), "\tThe 'SQL Log Files' location is not set\r\n");
	}

	if(strlen(gsDefaultDBO) == 0)
	{
		if(!bAlertUser) return false;
		strcat_s(sText, sizeof(sText), "\tThe 'Default SQL DBO' is not set\r\n");
	}
	
	if(strlen(gsReplicationDB) == 0)
	{
		if(!bAlertUser) return false;
		strcat_s(sText, sizeof(sText), "\tThe 'Replication DB' name is not set\r\n");
	}

	if(strlen(gsCompanyName) == 0)
	{
		if(!bAlertUser) return false;
		strcat_s(sText, sizeof(sText), "\tThe 'Company Name' is not set\r\n");
	}

	if(strlen(gsCompanyPassword) == 0)
	{
		if(!bAlertUser) return false;
		strcat_s(sText, sizeof(sText), "\tThe 'Company Password' is not set\r\n");
	}

	if(strlen(gsSQLDriver) == 0)
	{
		if(!bAlertUser) return false;
		strcat_s(sText, sizeof(sText), "\tThe 'SQL Driver' is not set\r\n");
	}

	if(strlen(gsSQLServer) == 0)
	{
		if(!bAlertUser) return false;
		strcat_s(sText, sizeof(sText), "\tThe 'SQL Server' name is not set\r\n");
	}

	if(strlen(gsSQLUserID) == 0)
	{
		if(!bAlertUser) return false;
		strcat_s(sText, sizeof(sText), "\tThe 'SQL User ID' is not set\r\n");
	}

#ifndef _DEBUG
	if(gdwConnectInterval < 5)
	{
		if(!bAlertUser) return false;
		strcat_s(sText, sizeof(sText), "\tThe 'Connection Interval' is invalid.\r\n");
	}
#endif

	if(gdwMaxClients != 1)
	{
		if(!bAlertUser) return false;
		strcat_s(sText, sizeof(sText), "\tThe 'Max Clients' must be set to 1\r\n");
		gdwMaxClients = 1;
	}

	if(gdwServerPort <= 0)
	{
		if(!bAlertUser) return false;
		strcat_s(sText, sizeof(sText), "\tThe SQL Exchange 'Server Port' is not set\r\n");
	}

	if(bAlertUser && strlen(sText) > 0)
	{
		strcpy_s(sMsg, sizeof(sMsg), 
			"The SQL-Exchange Client has not been configured, the following items need"
			"\r\n  to be setup before the Client can begin operating.\r\n\r\n");
		strcat_s(sMsg, sizeof(sMsg), sText);
		strcat_s(sMsg, sizeof(sMsg), "\r\nWould you like to configure these items now?");

		if(MessageBox(NULL, sMsg, gsTitleCaption, MB_ICONQUESTION | MB_YESNO | MB_TOPMOST) == IDYES)
		{
			LaunchConfigDlg(GetActiveWindow());
			return IsClientConfigured(bAlertUser);
		}
		return false;
	}

	return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool GetRegistryValues(void)
{
	DWORD dwTemp = 0;
	DWORD iSizeofString = 0;
	int iLen = 0;

	iSizeofString = sizeof(gsGUIPWDHash);
	memset(gsGUIPWDHash, 0, iSizeofString);
    if(!Get_StringRegistryValue(HKEY_LOCAL_MACHINE, gsRegistryKey, "GUIPWD", gsGUIPWDHash, iSizeofString))
        return false;

	iSizeofString = sizeof(gsReplicationDB);
	memset(gsReplicationDB, 0, iSizeofString);
    if(!Get_StringRegistryValue(HKEY_LOCAL_MACHINE, gsRegistryKey, "ReplicationDB", gsReplicationDB, iSizeofString))
        return false;

	iSizeofString = sizeof(gsDefaultDBO);
	memset(gsDefaultDBO, 0, iSizeofString);
    if(!Get_StringRegistryValue(HKEY_LOCAL_MACHINE, gsRegistryKey, "DefaultDBO", gsDefaultDBO, iSizeofString))
        return false;

	iSizeofString = sizeof(gsSQLDataFiles);
	memset(gsSQLDataFiles, 0, iSizeofString);
    if(!Get_StringRegistryValue(HKEY_LOCAL_MACHINE, gsRegistryKey, "SQLDataFiles", gsSQLDataFiles, iSizeofString))
        return false;

	iSizeofString = sizeof(gsSQLLogFiles);
	memset(gsSQLLogFiles, 0, iSizeofString);
    if(!Get_StringRegistryValue(HKEY_LOCAL_MACHINE, gsRegistryKey, "SQLLogFiles", gsSQLLogFiles, iSizeofString))
        return false;

	iSizeofString = sizeof(gsSessionLogDir);
	memset(gsSessionLogDir, 0, iSizeofString);
    if(!Get_StringRegistryValue(HKEY_LOCAL_MACHINE, gsRegistryKey, "SessionLogDir", gsSessionLogDir, iSizeofString))
        return false;
	
    iSizeofString = sizeof(gsServerAddress);
	memset(gsServerAddress, 0, iSizeofString);
    if(!Get_StringRegistryValue(HKEY_LOCAL_MACHINE, gsRegistryKey, "ServerAddress", gsServerAddress, iSizeofString))
        return false;
	
    iSizeofString = sizeof(gsTempFilesPath);
	memset(gsTempFilesPath, 0, iSizeofString);
    if(!Get_StringRegistryValue(HKEY_LOCAL_MACHINE, gsRegistryKey, "TempFilesPath", gsTempFilesPath, iSizeofString))
        return false;
	
    iSizeofString = sizeof(gsPath);
	memset(gsPath, 0, iSizeofString);
    if(!Get_StringRegistryValue(HKEY_LOCAL_MACHINE, gsRegistryKey, "Path", gsPath, iSizeofString))
        return false;
	
	iSizeofString = sizeof(gsAuthKey);
	memset(gsAuthKey, 0, iSizeofString);
    if(!Get_StringRegistryValue(HKEY_LOCAL_MACHINE, gsRegistryKey, "AuthKey", gsAuthKey, iSizeofString))
        return false;
	
	iSizeofString = sizeof(gsAuthString);
	memset(gsAuthString, 0, iSizeofString);
    if(!Get_StringRegistryValue(HKEY_LOCAL_MACHINE, gsRegistryKey, "AuthString", gsAuthString, iSizeofString))
        return false;
	
	iSizeofString = sizeof(gsCompanyName);
	memset(gsCompanyName, 0, iSizeofString);
    if(!Get_StringRegistryValue(HKEY_LOCAL_MACHINE, gsRegistryKey, "CompanyName", gsCompanyName, iSizeofString))
        return false;
	
	iSizeofString = sizeof(gsCompanyPassword);
	memset(gsCompanyPassword, 0, iSizeofString);
    if(!Get_BinaryRegistryValue(HKEY_LOCAL_MACHINE, gsRegistryKey, "CompanyPassword", gsCompanyPassword, iSizeofString))
        return false;

	iLen = iSizeofString;
	if(iLen > 0)
	{
		gSC.Cipher(gsCompanyPassword, iLen);
	}

	iSizeofString = sizeof(gsSQLDriver);
	memset(gsSQLDriver, 0, iSizeofString);
    if(!Get_StringRegistryValue(HKEY_LOCAL_MACHINE, gsRegistryKey, "SQLDriver", gsSQLDriver, iSizeofString))
        return false;

	iSizeofString = sizeof(gsSQLPassword);
	memset(gsSQLPassword, 0, iSizeofString);
    if(!Get_BinaryRegistryValue(HKEY_LOCAL_MACHINE, gsRegistryKey, "SQLPassword", gsSQLPassword, iSizeofString))
        return false;

	iLen = iSizeofString;
	if(iLen > 0)
	{
		gSC.Cipher(gsSQLPassword, iLen);
	}

	iSizeofString = sizeof(gsSQLServer);
	memset(gsSQLServer, 0, iSizeofString);
    if(!Get_StringRegistryValue(HKEY_LOCAL_MACHINE, gsRegistryKey, "SQLServer", gsSQLServer, iSizeofString))
        return false;

	iSizeofString = sizeof(gsSQLUserID);
	memset(gsSQLUserID, 0, iSizeofString);
    if(!Get_StringRegistryValue(HKEY_LOCAL_MACHINE, gsRegistryKey, "SQLUserID", gsSQLUserID, iSizeofString))
        return false;

	if(!Get_DWORDRegistryValue(HKEY_LOCAL_MACHINE, gsRegistryKey, "CPUAffinityMask", gdwCPUAffinityMask))
        return false;

	if(!Get_DWORDRegistryValue(HKEY_LOCAL_MACHINE, gsRegistryKey, "ConnectInterval", gdwConnectInterval))
        return false;

	if(!Get_DWORDRegistryValue(HKEY_LOCAL_MACHINE, gsRegistryKey, "MaxInactivityTime", gdwMaxInactivityTime))
        return false;

	if(!Get_DWORDRegistryValue(HKEY_LOCAL_MACHINE, gsRegistryKey, "ServerPort", gdwServerPort))
        return false;

	if(!Get_DWORDRegistryValue(HKEY_LOCAL_MACHINE, gsRegistryKey, "SQLServerPort", gdwSQLServerPort))
        return false;

	if(!Get_DWORDRegistryValue(HKEY_LOCAL_MACHINE, gsRegistryKey, "SQLTimeout", gdwSQLTimeout))
        return false;

	gbUseProcessBoost = Get_BOOLRegistryValue(HKEY_LOCAL_MACHINE, gsRegistryKey, "UseProcessBoost");
	gbUseThreadBoost = Get_BOOLRegistryValue(HKEY_LOCAL_MACHINE, gsRegistryKey, "UseThreadBoost");
	gbUseNTFibers = Get_BOOLRegistryValue(HKEY_LOCAL_MACHINE, gsRegistryKey, "UseNTFibers");
	gbDebugMode = Get_BOOLRegistryValue(HKEY_LOCAL_MACHINE, gsRegistryKey, "DebugMode");
	gbUseTrustedConnection = Get_BOOLRegistryValue(HKEY_LOCAL_MACHINE, gsRegistryKey, "UseTrustedConnection");
	gbUseTCPIPConnection = Get_BOOLRegistryValue(HKEY_LOCAL_MACHINE, gsRegistryKey, "UseTCPIPConnection");
	gbUseApplicationName = Get_BOOLRegistryValue(HKEY_LOCAL_MACHINE, gsRegistryKey, "UseApplicationName");
	gbAllowOffLineDataProcessing = Get_BOOLRegistryValue(HKEY_LOCAL_MACHINE, gsRegistryKey, "AllowOffLineDataProcessing");
	gbTrimStringDataOnExport = Get_BOOLRegistryValue(HKEY_LOCAL_MACHINE, gsRegistryKey, "TrimStringDataOnExport");
	gbRequestInitialization = Get_BOOLRegistryValue(HKEY_LOCAL_MACHINE, gsRegistryKey, "RequestInitialization");

	gbShowSplashScreen = Get_BOOLRegistryValue(HKEY_LOCAL_MACHINE, gsRegistryKey, "ShowSplashScreen");
	gbCacheLogFile = Get_BOOLRegistryValue(HKEY_LOCAL_MACHINE, gsRegistryKey, "CacheLogFile");
	gbReportNoTransactions = Get_BOOLRegistryValue(HKEY_LOCAL_MACHINE, gsRegistryKey, "ReportNoTransactions");

	if(!Get_DWORDRegistryValue(HKEY_LOCAL_MACHINE, gsRegistryKey, "TimeWindowBegin", gdwStartTime))
        return false;

	if(!Get_DWORDRegistryValue(HKEY_LOCAL_MACHINE, gsRegistryKey, "TimeWindowEnd", gdwStopTime))
        return false;

	iSizeofString = sizeof(gbWeekDays);
	memset(&gbWeekDays, 0, iSizeofString);
    if(!Get_BinaryRegistryValue(HKEY_LOCAL_MACHINE, gsRegistryKey, "ConnectDays", &gbWeekDays, iSizeofString))
	{
        return false;
	}

	if(!IsAtleastOneDaySelected())
	{
		memcpy(&gbWeekDays, gbDfltWeekDays, sizeof(gbWeekDays));
	}

	return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool SaveRegistryValues(void)
{
	char sLocalBuf[1024];
	int iBufSz = 0;
	DWORD dwTemp = 0;

	Set_StringRegistryValue(HKEY_LOCAL_MACHINE, gsRegistryKey, "ReplicationDB", gsReplicationDB);
	Set_StringRegistryValue(HKEY_LOCAL_MACHINE, gsRegistryKey, "DefaultDBO", gsDefaultDBO);
	Set_StringRegistryValue(HKEY_LOCAL_MACHINE, gsRegistryKey, "SQLDataFiles", gsSQLDataFiles);
	Set_StringRegistryValue(HKEY_LOCAL_MACHINE, gsRegistryKey, "SQLLogFiles", gsSQLLogFiles);
    Set_StringRegistryValue(HKEY_LOCAL_MACHINE, gsRegistryKey, "SessionLogDir", gsSessionLogDir);
    Set_StringRegistryValue(HKEY_LOCAL_MACHINE, gsRegistryKey, "ServerAddress", gsServerAddress);
    Set_StringRegistryValue(HKEY_LOCAL_MACHINE, gsRegistryKey, "TempFilesPath", gsTempFilesPath);
    Set_StringRegistryValue(HKEY_LOCAL_MACHINE, gsRegistryKey, "Path", gsPath);
    Set_StringRegistryValue(HKEY_LOCAL_MACHINE, gsRegistryKey, "AuthKey", gsAuthKey);
    Set_StringRegistryValue(HKEY_LOCAL_MACHINE, gsRegistryKey, "AuthString", gsAuthString);
    Set_StringRegistryValue(HKEY_LOCAL_MACHINE, gsRegistryKey, "CompanyName", gsCompanyName);
    Set_StringRegistryValue(HKEY_LOCAL_MACHINE, gsRegistryKey, "SQLDriver", gsSQLDriver);

	iBufSz = (int)strlen(gsCompanyPassword);
	dwTemp = (DWORD) iBufSz;
	gSC.Cipher(gsCompanyPassword, sLocalBuf, iBufSz);
	sLocalBuf[iBufSz] = '\0';
	Set_BinaryRegistryValue(HKEY_LOCAL_MACHINE, gsRegistryKey, "CompanyPassword", sLocalBuf, dwTemp);

	iBufSz = (int)strlen(gsSQLPassword);
	dwTemp = (DWORD) iBufSz;
	gSC.Cipher(gsSQLPassword, sLocalBuf, iBufSz);
	sLocalBuf[iBufSz] = '\0';
	Set_BinaryRegistryValue(HKEY_LOCAL_MACHINE, gsRegistryKey, "SQLPassword", sLocalBuf, dwTemp);

	Set_BinaryRegistryValue(HKEY_LOCAL_MACHINE, gsRegistryKey, "ConnectDays", &gbWeekDays, sizeof(gbWeekDays));

	Set_StringRegistryValue(HKEY_LOCAL_MACHINE, gsRegistryKey, "SQLServer", gsSQLServer);
    Set_StringRegistryValue(HKEY_LOCAL_MACHINE, gsRegistryKey, "SQLUserID", gsSQLUserID);
	
	Set_DWORDRegistryValue(HKEY_LOCAL_MACHINE, gsRegistryKey, "SQLTimeout", gdwSQLTimeout);
	Set_DWORDRegistryValue(HKEY_LOCAL_MACHINE, gsRegistryKey, "SQLServerPort", gdwSQLServerPort);
	Set_DWORDRegistryValue(HKEY_LOCAL_MACHINE, gsRegistryKey, "ConnectInterval", gdwConnectInterval);
	Set_DWORDRegistryValue(HKEY_LOCAL_MACHINE, gsRegistryKey, "MaxInactivityTime", gdwMaxInactivityTime);
	Set_DWORDRegistryValue(HKEY_LOCAL_MACHINE, gsRegistryKey, "ServerPort", gdwServerPort);
	Set_DWORDRegistryValue(HKEY_LOCAL_MACHINE, gsRegistryKey, "TimeWindowEnd", gdwStopTime);
	Set_DWORDRegistryValue(HKEY_LOCAL_MACHINE, gsRegistryKey, "TimeWindowBegin", gdwStartTime);
	Set_DWORDRegistryValue(HKEY_LOCAL_MACHINE, gsRegistryKey, "CPUAffinityMask", gdwCPUAffinityMask);

	Set_BOOLRegistryValue(HKEY_LOCAL_MACHINE, gsRegistryKey, "RequestInitialization", gbRequestInitialization);
	Set_BOOLRegistryValue(HKEY_LOCAL_MACHINE, gsRegistryKey, "DebugMode", gbDebugMode);
	Set_BOOLRegistryValue(HKEY_LOCAL_MACHINE, gsRegistryKey, "UseApplicationName", gbUseApplicationName);
	Set_BOOLRegistryValue(HKEY_LOCAL_MACHINE, gsRegistryKey, "UseTCPIPConnection", gbUseTCPIPConnection);
	Set_BOOLRegistryValue(HKEY_LOCAL_MACHINE, gsRegistryKey, "UseTrustedConnection", gbUseTrustedConnection);
	Set_BOOLRegistryValue(HKEY_LOCAL_MACHINE, gsRegistryKey, "ShowSplashScreen", gbShowSplashScreen);
    Set_BOOLRegistryValue(HKEY_LOCAL_MACHINE, gsRegistryKey, "ReportNoTransactions", gbReportNoTransactions);
	Set_BOOLRegistryValue(HKEY_LOCAL_MACHINE, gsRegistryKey, "CacheLogFile", gbCacheLogFile);
	Set_BOOLRegistryValue(HKEY_LOCAL_MACHINE, gsRegistryKey, "UseProcessBoost", gbUseProcessBoost);
	Set_BOOLRegistryValue(HKEY_LOCAL_MACHINE, gsRegistryKey, "UseThreadBoost", gbUseThreadBoost);
	Set_BOOLRegistryValue(HKEY_LOCAL_MACHINE, gsRegistryKey, "UseNTFibers", gbUseNTFibers);
	Set_BOOLRegistryValue(HKEY_LOCAL_MACHINE, gsRegistryKey, "AllowOffLineDataProcessing", gbAllowOffLineDataProcessing);
	Set_BOOLRegistryValue(HKEY_LOCAL_MACHINE, gsRegistryKey, "TrimStringDataOnExport", gbTrimStringDataOnExport);

	return true;
}	

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool IsAtleastOneDaySelected(void)
{
    int iDay = 0;

    while(iDay < 7)
    {
        if(gbWeekDays[iDay])
		{
            return true;
		}

        iDay++;
    }

    return false;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int GetNextRunDay(int iToday)
{
    int iDay = iToday;
	int iStartDay = iDay;

	if(!IsAtleastOneDaySelected())
	{
		return -1;
	}

	//KB000501.
	{	
		if(iDay == 7)
		{
			iDay = 0;
		}
	}

	while(iDay < 7)
    {
        if(gbWeekDays[iDay])
		{
            return iDay;
		}

		if(iDay == 6)
		{
			iDay = 0;
		}

        iDay++;

		if(iDay == iStartDay)
		{
			//FIXFIX: This is a very bad patch to fix a sheduling
			//	problem in CalculateNextConnectTime(). See the
			//	FIXFIX section in CalculateNextConnectTime().
			return iStartDay;
		}
    }

    return -1;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CalculateNextConnectTime(void)
{
	DWORD dwCurrentTime = 0;

	gbNextConnectTimeCalculated = true;

	int iToday = 0;

	SYSTEMTIME ST;
	GetLocalTime(&ST);

	if(!IsAtleastOneDaySelected())
	{
		return;
	}

	iToday = ST.wDayOfWeek;

	giNextDay = GetNextRunDay(iToday);

	if(giNextDay != iToday)
	{
		gdwNextTime = gdwStartTime;
	}

	dwCurrentTime = TimeToLong(ST.wHour, ST.wMinute, ST.wSecond);

	//Keep incrementing next connect time until it is greater than the current time.
	while(gdwNextTime < dwCurrentTime)
	{
		gdwNextTime = (gdwNextTime + (gdwConnectInterval * 60));
	}

	//If the new 'next connect time' overflows 12AM,
	//	then we have to push the connect day out another day. 
	if(gdwNextTime > 86399)
	{
		//FIXFIX: If there is only one 'Run day' selected and the new
		//	'next connect time' overflows 12AM then GetNextRunDay()
		//	will go into an infinite loop.

		gdwNextTime = gdwStartTime;
		
		giNextDay = GetNextRunDay(giNextDay + 1);
	}
	else{
		//Is the new 'next connect time' outside the window?
		if(dwCurrentTime >= gdwStopTime)
		{
			gdwNextTime = gdwStartTime;
			giNextDay = GetNextRunDay(giNextDay + 1);
		}
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void PrintNextRunTime(void)
{
	if(IsAtleastOneDaySelected())
	{
		while(!gbNextConnectTimeCalculated)
		{
			Sleep(1);
		}

		int iHour, iMinute, iSecond;

		SYSTEMTIME ST;
		char sNextTime[64];
		char sMessage[255];

		GetLocalTime(&ST);

		LongToTime(gdwNextTime, &iHour, &iMinute, &iSecond);
		ST.wHour   = iHour;
		ST.wMinute = iMinute;
		ST.wSecond = iSecond;

		GetTimeFormat(LOCALE_USER_DEFAULT, 0, &ST, NULL, sNextTime, 64);

		sprintf_s(sMessage, sizeof(sMessage),
			"Next transfer will occur on %s @ %s", gsWeekDayName[giNextDay], sNextTime);
		WriteSysLogEx(sMessage, EVENT_NONE);
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

DWORD WINAPI Timer_Thread(LPVOID data)
{
	DWORD dwCurrentTime = 0;
	DWORD dwStatusTime = 0;
	SYSTEMTIME ST;

	//GetSystemTime(&ST);
	GetLocalTime(&ST);

	dwCurrentTime = TimeToLong(ST.wHour, ST.wMinute, ST.wSecond);

	//Set the next connect time to the beginning of the time window.
	gdwNextTime = gdwStartTime;

	CalculateNextConnectTime();

	while(gService.Alive())
	{
		//GetSystemTime(&ST);
		GetLocalTime(&ST);

		if(ST.wDayOfWeek == giNextDay || gbConnectNow)
		{
			dwCurrentTime = TimeToLong(ST.wHour, ST.wMinute, ST.wSecond);

			//Are we within the time window?
			if((dwCurrentTime >= gdwStartTime && dwCurrentTime <= gdwStopTime) || gbConnectNow)
			{
				//Is it time to connect?
				if(dwCurrentTime >= gdwNextTime || gbConnectNow)
				{
					if(!gService.RunningAsService() && !gbConnectNow)
					{
						WriteSysLogEx("The client is not running as a service. Auto-Connection will not occur.", EVENT_WARN);
					}
					else if(bReplicationDLGopen)
					{
						WriteSysLogEx("The replication dialog is open. Auto-Connection will not occur.", EVENT_WARN);
					}
					else{
						if(ConnectToServer(gbConnectNow))
						{
							while(gServer.Client(0)->IsConnected())
							{
								Sleep(1);
							}
							Sleep(1000);
						}

						if(!gbConnectNow)
						{
							gdwNextTime = (gdwNextTime + (gdwConnectInterval * 60));
						}
					}

					gbConnectNow = false;
				}
			}
			else gdwNextTime = gdwStartTime;
		}

		CalculateNextConnectTime();

		//If the 'next connect time' has changed, log it.
		if(dwStatusTime != gdwNextTime)
		{
			PrintNextRunTime();
			dwStatusTime = gdwNextTime;
		}

		Sleep(1000);
	}

	return 0;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//This function will set the row, column text of a list view.
void SetStatus(HWND hGrid, int iCol, int iItem, char *sText)
{
	LV_ITEM MyLVItem;

	memset(&MyLVItem, 0, sizeof(MyLVItem));

	MyLVItem.pszText  = sText;
	MyLVItem.mask     = LVIF_TEXT;
	MyLVItem.iSubItem = iCol;
	MyLVItem.iItem    = iItem;

	ListView_EnsureVisible(hGrid, iItem, FALSE);
	ListView_SetItem(hGrid, &MyLVItem);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool LaunchConfigDlg(HWND hOwner)
{
	PROPSHEETPAGE psp[7];
    PROPSHEETHEADER psh;
	int iPage = 0;

    memset(&psp[iPage], '\0', sizeof(PROPSHEETPAGE));
    psp[iPage].dwSize      = sizeof(PROPSHEETPAGE);
    psp[iPage].dwFlags     = PSP_DEFAULT|PSP_USETITLE;
    psp[iPage].hInstance   = ghAppInstance;
    psp[iPage].pszTemplate = MAKEINTRESOURCE(IDD_GENERALTAB);
    psp[iPage].pszIcon     = NULL;
    psp[iPage].pszTitle    = "General";
    psp[iPage].pfnDlgProc  = (DLGPROC)TabGeneralDialog;
    psp[iPage].lParam      = 0;
    psp[iPage].pfnCallback = NULL;
    psp[iPage].pcRefParent = NULL;
	iPage++;

    memset(&psp[iPage], '\0', sizeof(PROPSHEETPAGE));
    psp[iPage].dwSize      = sizeof(PROPSHEETPAGE);
    psp[iPage].dwFlags     = PSP_DEFAULT|PSP_USETITLE;
    psp[iPage].hInstance   = ghAppInstance;
    psp[iPage].pszTemplate = MAKEINTRESOURCE(IDD_PROCESSORSTAB);
    psp[iPage].pszIcon     = NULL;
    psp[iPage].pszTitle    = "Processor Settings";
    psp[iPage].pfnDlgProc  = (DLGPROC)TabProcessorsDialog;
    psp[iPage].lParam      = 0;
    psp[iPage].pfnCallback = NULL;
    psp[iPage].pcRefParent = NULL;
	iPage++;
	
	memset(&psp[iPage], '\0', sizeof(PROPSHEETPAGE));
    psp[iPage].dwSize      = sizeof(PROPSHEETPAGE);
    psp[iPage].dwFlags     = PSP_DEFAULT|PSP_USETITLE;
    psp[iPage].hInstance   = ghAppInstance;
    psp[iPage].pszTemplate = MAKEINTRESOURCE(IDD_ACCOUNTTAB);
    psp[iPage].pszIcon     = NULL;
    psp[iPage].pszTitle    = "Account";
    psp[iPage].pfnDlgProc  = (DLGPROC)TabAccountDialog;
    psp[iPage].lParam      = 0;
    psp[iPage].pfnCallback = NULL;
    psp[iPage].pcRefParent = NULL;
	iPage++;

	memset(&psp[iPage], '\0', sizeof(PROPSHEETPAGE));
    psp[iPage].dwSize      = sizeof(PROPSHEETPAGE);
    psp[iPage].dwFlags     = PSP_DEFAULT|PSP_USETITLE;
    psp[iPage].hInstance   = ghAppInstance;
    psp[iPage].pszTemplate = MAKEINTRESOURCE(IDD_SQLTAB);
    psp[iPage].pszIcon     = NULL;
    psp[iPage].pszTitle    = "SQL";
    psp[iPage].pfnDlgProc  = (DLGPROC)TabSQLDialog;
    psp[iPage].lParam      = 0;
    psp[iPage].pfnCallback = NULL;
    psp[iPage].pcRefParent = NULL;
	iPage++;

    memset(&psp[iPage], '\0', sizeof(PROPSHEETPAGE));
    psp[iPage].dwSize      = sizeof(PROPSHEETPAGE);
    psp[iPage].dwFlags     = PSP_DEFAULT|PSP_USETITLE;
    psp[iPage].hInstance   = ghAppInstance;
    psp[iPage].pszTemplate = MAKEINTRESOURCE(IDD_SECURITYTAB);
    psp[iPage].pszIcon     = NULL;
    psp[iPage].pszTitle    = "Security";
    psp[iPage].pfnDlgProc  = (DLGPROC)TabSecurityDialog;
    psp[iPage].lParam      = 0;
    psp[iPage].pfnCallback = NULL;
    psp[iPage].pcRefParent = NULL;
	iPage++;

    memset(&psp[iPage], '\0', sizeof(PROPSHEETPAGE));
    psp[iPage].dwSize      = sizeof(PROPSHEETPAGE);
    psp[iPage].dwFlags     = PSP_DEFAULT|PSP_USETITLE;
    psp[iPage].hInstance   = ghAppInstance;
    psp[iPage].pszTemplate = MAKEINTRESOURCE(IDD_SCHEDTAB);
    psp[iPage].pszIcon     = NULL;
    psp[iPage].pszTitle    = "Schedule";
    psp[iPage].pfnDlgProc  = (DLGPROC)TabSchedDialog;
    psp[iPage].lParam      = 0;
    psp[iPage].pfnCallback = NULL;
    psp[iPage].pcRefParent = NULL;
	iPage++;

	memset(&psp[iPage], '\0', sizeof(PROPSHEETPAGE));
    psp[iPage].dwSize      = sizeof(PROPSHEETPAGE);
    psp[iPage].dwFlags     = PSP_DEFAULT|PSP_USETITLE;
    psp[iPage].hInstance   = ghAppInstance;
    psp[iPage].pszTemplate = MAKEINTRESOURCE(IDD_ADVANCEDTAB);
    psp[iPage].pszIcon     = NULL;
    psp[iPage].pszTitle    = "Advanced";
    psp[iPage].pfnDlgProc  = (DLGPROC)TabAdvancedDialog;
    psp[iPage].lParam      = 0;
    psp[iPage].pfnCallback = NULL;
    psp[iPage].pcRefParent = NULL;
	iPage++;

    memset(&psh, '\0', sizeof(PROPSHEETHEADER));
    psh.dwSize      = sizeof(PROPSHEETHEADER);
    psh.dwFlags     = PSH_USEHICON | PSH_PROPSHEETPAGE | PSH_NOAPPLYNOW;
    psh.hwndParent  = hOwner;
    psh.hInstance   = ghAppInstance;
    psh.hIcon       = LoadIcon(ghAppInstance, MAKEINTRESOURCE(IDI_MAIN));
    psh.pszCaption  = "Client Settings";
    psh.nPages      = iPage;
    psh.nStartPage  = 0;
    psh.ppsp        = (LPCPROPSHEETPAGE) &psp;
    psh.pfnCallback = NULL;

    PropertySheet(&psh);

    SaveRegistryValues();

	if(gbThreadFiberConfigChanged)
	{
		MessageBox(NULL,
			"The Thread/Fiber configuration was changed, so this application must be shutdown.",
			gsTitleCaption, MB_ICONASTERISK | MB_SYSTEMMODAL | MB_TASKMODAL);
		gService.StopRunning();
	}

	return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

__time64_t FileTimeToTimeT(FILETIME *ft) 
{ 
	SYSTEMTIME st_utc;
	SYSTEMTIME st_localtz;
	TIME_ZONE_INFORMATION TimeZoneInfo;
	struct tm tm;

	FileTimeToSystemTime(ft, &st_utc);
	GetTimeZoneInformation(&TimeZoneInfo);
	SystemTimeToTzSpecificLocalTime(&TimeZoneInfo, &st_utc, &st_localtz);

	tm.tm_sec = st_localtz.wSecond;
	tm.tm_min = st_localtz.wMinute;
	tm.tm_hour = st_localtz.wHour;
	tm.tm_mday = st_localtz.wDay;
	tm.tm_mon = (st_localtz.wMonth - 1);
	tm.tm_year = (st_localtz.wYear - 1900);
	tm.tm_wday = st_localtz.wDayOfWeek;
	tm.tm_yday = -1;
	tm.tm_isdst = -1;

	return mktime(&tm); 
} 

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void UpdateStatistics(void)
{
	char sRegKey[255];

	sprintf_s(sRegKey, sizeof(sRegKey), "%s\\Statistics", gsRegistryKey);

	if(Get_BOOLRegistryValue(HKEY_LOCAL_MACHINE, sRegKey, "ClearStatistics"))
	{
		gdwErrorCount = 0;
		gdwWarningCount = 0;
	}

	//Print out the processes's current memory usage.
	PROCESS_MEMORY_COUNTERS PMC;
	memset(&PMC, 0, sizeof(PMC));
	if(GetProcessMemoryInfo(GetCurrentProcess(), &PMC, sizeof(PMC)))
	{
		Set_DWORDRegistryValue(HKEY_LOCAL_MACHINE, sRegKey, "PageFaultCount", PMC.PageFaultCount);
		Set_DWORDRegistryValue(HKEY_LOCAL_MACHINE, sRegKey, "PagefileUsage", PMC.PagefileUsage);
		Set_DWORDRegistryValue(HKEY_LOCAL_MACHINE, sRegKey, "PeakPagefileUsage", PMC.PeakPagefileUsage);
		Set_DWORDRegistryValue(HKEY_LOCAL_MACHINE, sRegKey, "WorkingSetSize", PMC.WorkingSetSize);
		Set_DWORDRegistryValue(HKEY_LOCAL_MACHINE, sRegKey, "PeakWorkingSetSize", PMC.PeakWorkingSetSize);
	}

	Set_BOOLRegistryValue(HKEY_LOCAL_MACHINE, sRegKey, "ClearStatistics", false);
	Set_DWORDRegistryValue(HKEY_LOCAL_MACHINE, sRegKey, "SessionConnections", gdwTotalConnects);
	Set_DWORDRegistryValue(HKEY_LOCAL_MACHINE, sRegKey, "CurrentConnections", (DWORD)gServer.CurrentConnections());
	Set_DWORDRegistryValue(HKEY_LOCAL_MACHINE, sRegKey, "SessionErrors", gdwErrorCount);
	Set_DWORDRegistryValue(HKEY_LOCAL_MACHINE, sRegKey, "SessionWarnings", gdwWarningCount);
	//Set_DWORDRegistryValue(HKEY_LOCAL_MACHINE, sRegKey, "SessionBytesSent", -1);
	//Set_DWORDRegistryValue(HKEY_LOCAL_MACHINE, sRegKey, "SessionBytesRecv", -1);

	FILETIME CreationTimeFT;
	FILETIME DummyFT;

	memset(&CreationTimeFT, 0, sizeof(CreationTimeFT));

	if(GetProcessTimes(GetCurrentProcess(), &CreationTimeFT, &DummyFT, &DummyFT, &DummyFT))
	{
		__time64_t CurrentTimeTT;
		__time64_t CreationTimeTT = FileTimeToTimeT(&CreationTimeFT);
		time(&CurrentTimeTT);

		double dUpTime = _difftime64(CurrentTimeTT, CreationTimeTT);

		Set_DWORDRegistryValue(HKEY_LOCAL_MACHINE, sRegKey, "SessionUpTime", (DWORD)dUpTime);
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool StartClient(void)
{
	if(OnlyGoodForOneYear())
	{
		WriteSysLogEx("This version of the SQL-Exchange has expired. Please obtain a newer version.", EVENT_ERROR);
	}
	else{
		if(!gServer.IsActive())
		{
			if(!gServer.Start())
			{
				WriteSysLogEx("Failed to start the client.", EVENT_ERROR);
				return false;
			}
			WriteSysLogEx("Client started.", EVENT_INFO);
			return true;
		}
		WriteSysLogEx("The client is already active.", EVENT_INFO);
	}
	return false;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool StopClient(void)
{
	if(gServer.IsActive())
	{
		if(!gServer.Stop())
		{
			WriteSysLogEx("Failed to stop the client.", EVENT_ERROR);
			return false;
		}
		WriteSysLogEx("Client stopped.", EVENT_INFO);
		return true;
	}
	WriteSysLogEx("The client is not active.", EVENT_INFO);
	return false;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#endif
