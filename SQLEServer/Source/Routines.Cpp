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
#include <Time.H>
#include <PSAPI.H>

#include "Entry.H"
#include "Init.H"
#include "Console.H"

#include "../Dialogs/MainDlg.H"
#include "../Dialogs/TabClientDBS.H"
#include "../Dialogs/TabGeneral.H"
#include "../Dialogs/TabSecurity.H"
#include "../Dialogs/TabServer.H"
#include "../Dialogs/TabAdvanced.H"
#include "../Dialogs/TabWPIndex.H"
#include "../Dialogs/TabProcessors.H"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

using namespace NSWFL::Registry;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

CSocketServer gServer;

volatile int giTransProcessed = 0;
CRITICAL_SECTION csTransPerSecond;

char gsPath[MAX_PATH];
char gsSessionLogDir[MAX_PATH];
char gsImportTempDir[MAX_PATH];
char gsAuthKey[MAX_AUTH_LENGTH];
char gsAuthString[MAX_AUTH_LENGTH];
char gsCompressionMethod[255];
char gsGUIPWDHash[256];

char gsSQLDataFiles[MAX_PATH]; // Relative to server.
char gsSQLLogFiles[MAX_PATH]; // Relative to server.
char gsDefaultDBO[255];

char gsSQLDriver[255];

char gsSQLCustPassword[255];
char gsSQLCustServer[255];
char gsSQLCustUserID[255];
DWORD gdwSQLCustServerPort = 0; //Used when gbSQLCustUseTCPIPConnection = true
bool gbSQLCustUseTCPIPConnection = false; //SQL Connection Flag.
bool gbSQLCustUseTrustedConnection = false; //SQL Connection Flag.

char gsSQLIndexDatabase[255];
char gsSQLIndexPassword[255];
char gsSQLIndexServer[255];
char gsSQLIndexUserID[255];
DWORD gdwSQLIndexServerPort = 0; //Used when gbSQLIndexUseTCPIPConnection = true
bool gbSQLIndexUseTCPIPConnection = false; //SQL Connection Flag.
bool gbSQLIndexUseTrustedConnection = false; //SQL Connection Flag.

DWORD_PTR gdwCPUAffinityMask = 0;
DWORD gdwCompressionLevel = 0;
DWORD gdwErrorCount = 0;
DWORD gdwHashTimeout = 0;
DWORD gdwKeyGenLength = 0;
DWORD gdwListenPort = 0;
DWORD gdwMaxClients = 0;
DWORD gdwNextClientID = 0;
DWORD gdwMaxInactivityTime = 0;
DWORD gdwSQLTimeout = 30;
DWORD gdwRawHashLen = 0;
DWORD gdwTotalConnects = 0;
DWORD gdwWarningCount = 0;
DWORD gdwMaxRowsPerTransaction = 0;

bool gbDebugMode = false;
bool gbConsoleOpen = false;
bool gbShowSplashScreen = false;
bool gbStartAutomatically = false;
bool gbCacheLogFile = false;
bool gbRTOFDWI = false; //Retry transfer of corrupt files when initializing database.
bool gbExcludeInit = false;
bool gbVisualAlertOnError = false;
bool gbVisualAlertOnWarn = false;
bool gbForceAllowMultiInstance = false; //This is a debug flag, to be set at compile time.
bool gbUseApplicationName = false; //SQL Connection Flag.
bool gbUseProcessBoost = false;
bool gbUseThreadBoost = false;
bool gbUseNTFibers = false;
bool gbThreadFiberConfigChanged = false;
bool gbAllowOffLineDataProcessing = false;
bool gbUseEncryption = true;

FILE *gfhSessionLog = NULL;

CNASCCL gSC;

_MainDialogInfo MDI;
_ConnectedClientInfo *CCI;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool IsServerConfigured(bool bAlertUser)
{
	char sMsg[2048];
	char sText[2048];

	HWND hActive = GetActiveWindow();	

	strcpy_s(sText, sizeof(sText), "");

	if(strlen(gsSessionLogDir) == 0)
	{
		if(!bAlertUser) return false;
		strcat_s(sText, sizeof(sText), "\tThe 'Session Log Directory' is not set\r\n");
	}

	if(strlen(gsImportTempDir) == 0)
	{
		if(!bAlertUser) return false;
		strcat_s(sText, sizeof(sText), "\tThe 'Temp File Directory' is not set\r\n");
	}
	
	if(strlen(gsPath) == 0)
	{
		if(!bAlertUser) return false;
		strcat_s(sText, sizeof(sText), "\tThe SQL Exchange client 'Path' is not set\r\n");
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

	if(strlen(gsSQLIndexDatabase) == 0)
	{
		if(!bAlertUser) return false;
		strcat_s(sText, sizeof(sText), "\tThe 'Default Database' is not set\r\n");
	}

	if(strlen(gsDefaultDBO) == 0)
	{
		if(!bAlertUser) return false;
		strcat_s(sText, sizeof(sText), "\tThe 'Default SQL DBO' is not set\r\n");
	}
	
	if(strlen(gsSQLDriver) == 0)
	{
		if(!bAlertUser) return false;
		strcat_s(sText, sizeof(sText), "\tThe 'SQL Driver' is not set\r\n");
	}

	if(strlen(gsSQLIndexServer) == 0)
	{
		if(!bAlertUser) return false;
		strcat_s(sText, sizeof(sText), "\tThe 'Index SQL Server' name is not set\r\n");
	}

	if(strlen(gsSQLIndexUserID) == 0)
	{
		if(!bAlertUser) return false;
		strcat_s(sText, sizeof(sText), "\tThe 'Index SQL User ID' is not set\r\n");
	}

	if(strlen(gsSQLCustServer) == 0)
	{
		if(!bAlertUser) return false;
		strcat_s(sText, sizeof(sText), "\tThe 'Customer SQL Server' name is not set\r\n");
	}

	if(strlen(gsSQLCustUserID) == 0)
	{
		if(!bAlertUser) return false;
		strcat_s(sText, sizeof(sText), "\tThe 'Customer SQL User ID' is not set\r\n");
	}

	if(gdwMaxClients <= 0 || gdwMaxClients > FD_SETSIZE)
	{
		if(!bAlertUser) return false;
		strcat_s(sText, sizeof(sText), "\tThe 'Max Clients' is not set\r\n");
	}

	if(gdwListenPort <= 0)
	{
		if(!bAlertUser) return false;
		strcat_s(sText, sizeof(sText), "\tThe SQL Exchange 'Server Port' is not set\r\n");
	}

	if(bAlertUser && strlen(sText) > 0)
	{
		strcpy_s(sMsg, sizeof(sText), "The SQL-Exchange Server has not been configured, the following items need"
			"\r\n  to be setup before the server can begin operating.\r\n\r\n");
		strcat_s(sMsg, sizeof(sText), sText);
		strcat_s(sMsg, sizeof(sText), "\r\nWould you like to configure these items now?");

		if(MessageBox(NULL, sMsg, gsTitleCaption, MB_TASKMODAL | MB_ICONQUESTION | MB_YESNO | MB_TOPMOST) == IDYES)
		{
			LaunchConfigDlg(GetActiveWindow());
			return IsServerConfigured(bAlertUser);
		}
		return false;
	}

	return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool GetRegistryValues(void)
{
	DWORD iSizeofString = 0;
	DWORD dwTemp = 0;
	int iLen = 0;

	iSizeofString = sizeof(gsGUIPWDHash);
	memset(gsGUIPWDHash, 0, iSizeofString);
    if(!Get_BinaryRegistryValue(HKEY_LOCAL_MACHINE, gsRegistryKey, "GUIPWD", gsGUIPWDHash, iSizeofString))
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

	iSizeofString = sizeof(gsAuthKey);
	memset(gsAuthKey, 0, iSizeofString);
    if(!Get_StringRegistryValue(HKEY_LOCAL_MACHINE, gsRegistryKey, "AuthKey", gsAuthKey, iSizeofString))
        return false;

	iSizeofString = sizeof(gsImportTempDir);
	memset(gsImportTempDir, 0, iSizeofString);
    if(!Get_StringRegistryValue(HKEY_LOCAL_MACHINE, gsRegistryKey, "ImportTempDir", gsImportTempDir, iSizeofString))
        return false;

	iSizeofString = sizeof(gsAuthString);
	memset(gsAuthString, 0, iSizeofString);
    if(!Get_StringRegistryValue(HKEY_LOCAL_MACHINE, gsRegistryKey, "AuthString", gsAuthString, iSizeofString))
        return false;

	iSizeofString = sizeof(gsSQLDriver);
	memset(gsSQLDriver, 0, iSizeofString);
    if(!Get_StringRegistryValue(HKEY_LOCAL_MACHINE, gsRegistryKey, "SQLCustDriver", gsSQLDriver, iSizeofString))
        return false;

	iSizeofString = sizeof(gsSQLCustPassword);
	memset(gsSQLCustPassword, 0, iSizeofString);
    if(!Get_BinaryRegistryValue(HKEY_LOCAL_MACHINE, gsRegistryKey, "SQLCustPassword", gsSQLCustPassword, iSizeofString))
        return false;

	if(iSizeofString > 0)
	{
		gSC.Cipher(gsSQLCustPassword, iSizeofString);
	}

	iSizeofString = sizeof(gsSQLCustServer);
	memset(gsSQLCustServer, 0, iSizeofString);
    if(!Get_StringRegistryValue(HKEY_LOCAL_MACHINE, gsRegistryKey, "SQLCustServer", gsSQLCustServer, iSizeofString))
        return false;

	iSizeofString = sizeof(gsSQLCustUserID);
	memset(gsSQLCustUserID, 0, iSizeofString);
    if(!Get_StringRegistryValue(HKEY_LOCAL_MACHINE, gsRegistryKey, "SQLCustUserID", gsSQLCustUserID, iSizeofString))
        return false;

	gbSQLCustUseTrustedConnection = Get_BOOLRegistryValue(HKEY_LOCAL_MACHINE, gsRegistryKey, "SQLCustUseTrustedConnection");
	gbSQLCustUseTCPIPConnection = Get_BOOLRegistryValue(HKEY_LOCAL_MACHINE, gsRegistryKey, "SQLCustUseTCPIPConnection");
	if(!Get_DWORDRegistryValue(HKEY_LOCAL_MACHINE, gsRegistryKey, "SQLCustServerPort", gdwSQLCustServerPort))
        return false;

	iSizeofString = sizeof(gsSQLIndexDatabase);
	memset(gsSQLIndexDatabase, 0, iSizeofString);
    if(!Get_StringRegistryValue(HKEY_LOCAL_MACHINE, gsRegistryKey, "SQLIndexDatabase", gsSQLIndexDatabase, iSizeofString))
        return false;

	iSizeofString = sizeof(gsSQLIndexPassword);
	memset(gsSQLIndexPassword, 0, iSizeofString);
    if(!Get_BinaryRegistryValue(HKEY_LOCAL_MACHINE, gsRegistryKey, "SQLIndexPassword", gsSQLIndexPassword, iSizeofString))
        return false;

	if(iSizeofString > 0)
	{
		gSC.Cipher(gsSQLIndexPassword, iSizeofString);
	}

	iSizeofString = sizeof(gsSQLIndexServer);
	memset(gsSQLIndexServer, 0, iSizeofString);
    if(!Get_StringRegistryValue(HKEY_LOCAL_MACHINE, gsRegistryKey, "SQLIndexServer", gsSQLIndexServer, iSizeofString))
        return false;

	iSizeofString = sizeof(gsSQLIndexUserID);
	memset(gsSQLIndexUserID, 0, iSizeofString);
    if(!Get_StringRegistryValue(HKEY_LOCAL_MACHINE, gsRegistryKey, "SQLIndexUserID", gsSQLIndexUserID, iSizeofString))
        return false;

	gbSQLIndexUseTrustedConnection = Get_BOOLRegistryValue(HKEY_LOCAL_MACHINE, gsRegistryKey, "SQLIndexUseTrustedConnection");
	gbSQLIndexUseTCPIPConnection = Get_BOOLRegistryValue(HKEY_LOCAL_MACHINE, gsRegistryKey, "SQLIndexUseTCPIPConnection");
	if(!Get_DWORDRegistryValue(HKEY_LOCAL_MACHINE, gsRegistryKey, "SQLIndexServerPort", gdwSQLIndexServerPort))
        return false;

	iSizeofString = sizeof(gsPath);
	memset(gsPath, 0, iSizeofString);
    if(!Get_StringRegistryValue(HKEY_LOCAL_MACHINE, gsRegistryKey, "Path", gsPath, iSizeofString))
        return false;

    iSizeofString = sizeof(gsCompressionMethod);
	memset(gsCompressionMethod, 0, iSizeofString);
    if(!Get_StringRegistryValue(HKEY_LOCAL_MACHINE, gsRegistryKey, "CompressionMethod", gsCompressionMethod, iSizeofString))
        return false;

	if(!Get_DWORDRegistryValue(HKEY_LOCAL_MACHINE, gsRegistryKey, "MaxRowsPerTransaction", gdwMaxRowsPerTransaction))
        return false;

	if(!Get_DWORDRegistryValue(HKEY_LOCAL_MACHINE, gsRegistryKey, "CPUAffinityMask", gdwCPUAffinityMask))
        return false;

	if(!Get_DWORDRegistryValue(HKEY_LOCAL_MACHINE, gsRegistryKey, "CompressionLevel", gdwCompressionLevel))
        return false;

	if(!Get_DWORDRegistryValue(HKEY_LOCAL_MACHINE, gsRegistryKey, "RawHashLen", gdwRawHashLen))
        return false;

	if(!Get_DWORDRegistryValue(HKEY_LOCAL_MACHINE, gsRegistryKey, "HashTimeout", gdwHashTimeout))
        return false;

	if(!Get_DWORDRegistryValue(HKEY_LOCAL_MACHINE, gsRegistryKey, "ListenPort", gdwListenPort))
        return false;

	if(!Get_DWORDRegistryValue(HKEY_LOCAL_MACHINE, gsRegistryKey, "MaxClients", gdwMaxClients))
        return false;

	if(!Get_DWORDRegistryValue(HKEY_LOCAL_MACHINE, gsRegistryKey, "KeyGenLength", gdwKeyGenLength))
        return false;

	if(!Get_DWORDRegistryValue(HKEY_LOCAL_MACHINE, gsRegistryKey, "NextClientID", gdwNextClientID))
        return false;

	if(!Get_DWORDRegistryValue(HKEY_LOCAL_MACHINE, gsRegistryKey, "MaxInactivityTime", gdwMaxInactivityTime))
        return false;

	if(!Get_DWORDRegistryValue(HKEY_LOCAL_MACHINE, gsRegistryKey, "SQLTimeout", gdwSQLTimeout))
        return false;

	gbUseProcessBoost = Get_BOOLRegistryValue(HKEY_LOCAL_MACHINE, gsRegistryKey, "UseProcessBoost");
	gbUseThreadBoost = Get_BOOLRegistryValue(HKEY_LOCAL_MACHINE, gsRegistryKey, "UseThreadBoost");
	gbUseNTFibers = Get_BOOLRegistryValue(HKEY_LOCAL_MACHINE, gsRegistryKey, "UseNTFibers");
	gbDebugMode = Get_BOOLRegistryValue(HKEY_LOCAL_MACHINE, gsRegistryKey, "DebugMode");
	gbUseApplicationName = Get_BOOLRegistryValue(HKEY_LOCAL_MACHINE, gsRegistryKey, "UseApplicationName");
	gbAllowOffLineDataProcessing = Get_BOOLRegistryValue(HKEY_LOCAL_MACHINE, gsRegistryKey, "AllowOffLineDataProcessing");

	gbCacheLogFile = Get_BOOLRegistryValue(HKEY_LOCAL_MACHINE, gsRegistryKey, "CacheLogFile");
	gbRTOFDWI = Get_BOOLRegistryValue(HKEY_LOCAL_MACHINE, gsRegistryKey, "RTOFDWI");
	gbShowSplashScreen = Get_BOOLRegistryValue(HKEY_LOCAL_MACHINE, gsRegistryKey, "ShowSplashScreen");
	gbStartAutomatically = Get_BOOLRegistryValue(HKEY_LOCAL_MACHINE, gsRegistryKey, "StartAutomatically");
	gbVisualAlertOnError = Get_BOOLRegistryValue(HKEY_LOCAL_MACHINE, gsRegistryKey, "VisualAlertOnError");
	gbVisualAlertOnWarn = Get_BOOLRegistryValue(HKEY_LOCAL_MACHINE, gsRegistryKey, "VisualAlertOnWarn");

	return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool SaveRegistryValues(void)
{
	char sLocalBuf[1024];
	int iBufSz = 0;
	DWORD dwTemp = 0;

	Set_StringRegistryValue(HKEY_LOCAL_MACHINE, gsRegistryKey, "Path", gsPath);
	Set_StringRegistryValue(HKEY_LOCAL_MACHINE, gsRegistryKey, "DefaultDBO", gsDefaultDBO);
	Set_StringRegistryValue(HKEY_LOCAL_MACHINE, gsRegistryKey, "SQLDataFiles", gsSQLDataFiles);
	Set_StringRegistryValue(HKEY_LOCAL_MACHINE, gsRegistryKey, "SQLLogFiles", gsSQLLogFiles);
	Set_StringRegistryValue(HKEY_LOCAL_MACHINE, gsRegistryKey, "SessionLogDir", gsSessionLogDir);
    Set_StringRegistryValue(HKEY_LOCAL_MACHINE, gsRegistryKey, "AuthKey", gsAuthKey);
    Set_StringRegistryValue(HKEY_LOCAL_MACHINE, gsRegistryKey, "ImportTempDir", gsImportTempDir);
    Set_StringRegistryValue(HKEY_LOCAL_MACHINE, gsRegistryKey, "AuthString", gsAuthString);
    Set_StringRegistryValue(HKEY_LOCAL_MACHINE, gsRegistryKey, "SQLDriver", gsSQLDriver);
    Set_StringRegistryValue(HKEY_LOCAL_MACHINE, gsRegistryKey, "SQLCustServer", gsSQLCustServer);
    Set_StringRegistryValue(HKEY_LOCAL_MACHINE, gsRegistryKey, "SQLCustUserID", gsSQLCustUserID);
    Set_StringRegistryValue(HKEY_LOCAL_MACHINE, gsRegistryKey, "SQLIndexDatabase", gsSQLIndexDatabase);
        Set_StringRegistryValue(HKEY_LOCAL_MACHINE, gsRegistryKey, "SQLIndexServer", gsSQLIndexServer);
    Set_StringRegistryValue(HKEY_LOCAL_MACHINE, gsRegistryKey, "SQLIndexUserID", gsSQLIndexUserID);
    Set_StringRegistryValue(HKEY_LOCAL_MACHINE, gsRegistryKey, "CompressionMethod", gsCompressionMethod);

	iBufSz = (int)strlen(gsSQLIndexPassword);
	dwTemp = (DWORD) iBufSz;
	gSC.Cipher(gsSQLIndexPassword, sLocalBuf, iBufSz);
	sLocalBuf[iBufSz] = '\0';
	Set_BinaryRegistryValue(HKEY_LOCAL_MACHINE, gsRegistryKey, "SQLIndexPassword", sLocalBuf, iBufSz);

	iBufSz = (int)strlen(gsSQLCustPassword);
	dwTemp = (DWORD) iBufSz;
	gSC.Cipher(gsSQLCustPassword, sLocalBuf, iBufSz);
	sLocalBuf[iBufSz] = '\0';
    Set_BinaryRegistryValue(HKEY_LOCAL_MACHINE, gsRegistryKey, "SQLCustPassword", sLocalBuf, iBufSz);

	gdwNextClientID = gServer.NextPeerID();

	Set_BOOLRegistryValue(HKEY_LOCAL_MACHINE, gsRegistryKey, "DebugMode", gbDebugMode);
   	Set_BOOLRegistryValue(HKEY_LOCAL_MACHINE, gsRegistryKey, "VisualAlertOnError", gbVisualAlertOnError);
	Set_BOOLRegistryValue(HKEY_LOCAL_MACHINE, gsRegistryKey, "CacheLogFile", gbCacheLogFile);
	Set_BOOLRegistryValue(HKEY_LOCAL_MACHINE, gsRegistryKey, "RTOFDWI", gbRTOFDWI);
	Set_BOOLRegistryValue(HKEY_LOCAL_MACHINE, gsRegistryKey, "ShowSplashScreen", gbShowSplashScreen);
	Set_BOOLRegistryValue(HKEY_LOCAL_MACHINE, gsRegistryKey, "StartAutomatically", gbStartAutomatically);
	Set_BOOLRegistryValue(HKEY_LOCAL_MACHINE, gsRegistryKey, "VisualAlertOnWarn", gbVisualAlertOnWarn);
	Set_BOOLRegistryValue(HKEY_LOCAL_MACHINE, gsRegistryKey, "UseApplicationName", gbUseApplicationName);
	Set_BOOLRegistryValue(HKEY_LOCAL_MACHINE, gsRegistryKey, "UseProcessBoost", gbUseProcessBoost);
	Set_BOOLRegistryValue(HKEY_LOCAL_MACHINE, gsRegistryKey, "UseThreadBoost", gbUseThreadBoost);
	Set_BOOLRegistryValue(HKEY_LOCAL_MACHINE, gsRegistryKey, "UseNTFibers", gbUseNTFibers);
	Set_BOOLRegistryValue(HKEY_LOCAL_MACHINE, gsRegistryKey, "AllowOffLineDataProcessing", gbAllowOffLineDataProcessing);

	Set_BOOLRegistryValue(HKEY_LOCAL_MACHINE, gsRegistryKey, "SQLIndexUseTCPIPConnection", gbSQLIndexUseTCPIPConnection);
	Set_BOOLRegistryValue(HKEY_LOCAL_MACHINE, gsRegistryKey, "SQLIndexUseTrustedConnection", gbSQLIndexUseTrustedConnection);
	Set_DWORDRegistryValue(HKEY_LOCAL_MACHINE, gsRegistryKey, "SQLIndexServerPort", gdwSQLIndexServerPort);
	Set_BOOLRegistryValue(HKEY_LOCAL_MACHINE, gsRegistryKey, "SQLCustUseTCPIPConnection", gbSQLCustUseTCPIPConnection);
	Set_BOOLRegistryValue(HKEY_LOCAL_MACHINE, gsRegistryKey, "SQLCustUseTrustedConnection", gbSQLCustUseTrustedConnection);
	Set_DWORDRegistryValue(HKEY_LOCAL_MACHINE, gsRegistryKey, "SQLCustServerPort", gdwSQLCustServerPort);
	Set_DWORDRegistryValue(HKEY_LOCAL_MACHINE, gsRegistryKey, "SQLTimeout", gdwSQLTimeout);

	Set_DWORDRegistryValue(HKEY_LOCAL_MACHINE, gsRegistryKey, "CompressionLevel", gdwCompressionLevel);
	Set_DWORDRegistryValue(HKEY_LOCAL_MACHINE, gsRegistryKey, "HashTimeout", gdwHashTimeout);
	Set_DWORDRegistryValue(HKEY_LOCAL_MACHINE, gsRegistryKey, "KeyGenLength", gdwKeyGenLength);
	Set_DWORDRegistryValue(HKEY_LOCAL_MACHINE, gsRegistryKey, "ListenPort", gdwListenPort);
	Set_DWORDRegistryValue(HKEY_LOCAL_MACHINE, gsRegistryKey, "MaxClients", gdwMaxClients);
	Set_DWORDRegistryValue(HKEY_LOCAL_MACHINE, gsRegistryKey, "NextClientID", gdwNextClientID);
	Set_DWORDRegistryValue(HKEY_LOCAL_MACHINE, gsRegistryKey, "MaxInactivityTime", gdwMaxInactivityTime);
	Set_DWORDRegistryValue(HKEY_LOCAL_MACHINE, gsRegistryKey, "RawHashLen", gdwRawHashLen);
	Set_DWORDRegistryValue(HKEY_LOCAL_MACHINE, gsRegistryKey, "CPUAffinityMask", gdwCPUAffinityMask);

	return true;
}	

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool StartServer(void)
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
				WriteSysLogEx("Failed to start the server.", EVENT_ERROR);
				return false;
			}
			WriteSysLogEx("Server started.", EVENT_INFO);
			return true;
		}
		WriteSysLogEx("The server is already active.", EVENT_INFO);
	}
	return false;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool StopServer(void)
{
	if(gServer.IsActive())
	{
		if(!gServer.Stop())
		{
			WriteSysLogEx("Failed to stop the server.", EVENT_ERROR);
			return false;
		}
		WriteSysLogEx("Server stopped.", EVENT_INFO);
		return true;
	}
	WriteSysLogEx("The server is not active.", EVENT_INFO);
	return false;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool LaunchConfigDlg(HWND hOwner)
{
	PROPSHEETPAGE   psp[7];
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
    psp[iPage].pszTemplate = MAKEINTRESOURCE(IDD_SERVERTAB);
    psp[iPage].pszIcon     = NULL;
    psp[iPage].pszTitle    = "Server";
    psp[iPage].pfnDlgProc  = (DLGPROC)TabServerDialog;
    psp[iPage].lParam      = 0;
    psp[iPage].pfnCallback = NULL;
    psp[iPage].pcRefParent = NULL;
	iPage++;

    memset(&psp[iPage], '\0', sizeof(PROPSHEETPAGE));
    psp[iPage].dwSize      = sizeof(PROPSHEETPAGE);
    psp[iPage].dwFlags     = PSP_DEFAULT|PSP_USETITLE;
    psp[iPage].hInstance   = ghAppInstance;
    psp[iPage].pszTemplate = MAKEINTRESOURCE(IDD_INDEXDATABASE);
    psp[iPage].pszIcon     = NULL;
    psp[iPage].pszTitle    = "Index Database";
    psp[iPage].pfnDlgProc  = (DLGPROC)TabWPIndexDialog;
    psp[iPage].lParam      = 0;
    psp[iPage].pfnCallback = NULL;
    psp[iPage].pcRefParent = NULL;
	iPage++;

    memset(&psp[iPage], '\0', sizeof(PROPSHEETPAGE));
    psp[iPage].dwSize      = sizeof(PROPSHEETPAGE);
    psp[iPage].dwFlags     = PSP_DEFAULT|PSP_USETITLE;
    psp[iPage].hInstance   = ghAppInstance;
    psp[iPage].pszTemplate = MAKEINTRESOURCE(IDD_CLIENTDBSTAB);
    psp[iPage].pszIcon     = NULL;
    psp[iPage].pszTitle    = "Client Databases";
    psp[iPage].pfnDlgProc  = (DLGPROC)TabClientDBSDialog;
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
    psh.pszCaption  = "Server Settings";
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
#endif
