///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//  Copyright © NetworkDLS 2002, All rights reserved
//
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF 
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO 
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A 
// PARTICULAR PURPOSE.
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef _ENTRY_CPP
#define _ENTRY_CPP
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include <Windows.H>
#include <Stdio.H>
#include <Stdlib.H>

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "Entry.H"
#include "Init.H"

#include "../Dialogs/MainDlg.H"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

HINSTANCE ghAppInstance = NULL;

char gsAppCopyRight[64];
char gsFileVersion[32];
char gsTitleCaption[64];
char gsCommonRegistryKey[64];
char gsRegistryKey[64];

char gsServiceControlExe[MAX_PATH];
char gsPath[MAX_PATH];
char gsHttpURL[1024];

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

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevious, LPSTR sCmdLine, int iCmdShow)
{
    #ifdef _DEBUG
	ghAppInstance = GetModuleHandle(NULL);
    #else
	ghAppInstance = hInstance;
    #endif

	HANDLE xMutex = NULL;

    if(!InitializeApp())
    {
        return 1;
    }

	char sMutexName[1024];
	if( (xMutex = OpenMutex(SYNCHRONIZE, FALSE, GetBuildArchitecture(gsTitleCaption, sizeof(sMutexName), sMutexName))) )
	{
		MessageBox(NULL, "Only one instance can run at a time!", gsTitleCaption, MB_ICONASTERISK | MB_TASKMODAL);
		ReleaseMutex(xMutex);
		CloseHandle(xMutex);
		return 0;
	}
	else xMutex = CreateMutex(0, 0, GetBuildArchitecture(gsTitleCaption, sizeof(sMutexName), sMutexName));

	if(strlen(sCmdLine) == 0)
    {
        MessageBox(NULL, "No HTTP URL was given.", gsTitleCaption, MB_ICONSTOP|MB_TASKMODAL);
        RestartClientService();
        return 0;
    }

    if(strlen(sCmdLine) > sizeof(gsHttpURL))
    {
        MessageBox(NULL, "An invalid HTTP URL was given.", gsTitleCaption, MB_ICONSTOP|MB_TASKMODAL);
        RestartClientService();
        return 0;
    }

    strcpy_s(gsHttpURL, sizeof(gsHttpURL), sCmdLine);

    DialogBox(ghAppInstance, MAKEINTRESOURCE(IDD_MAIN), NULL, (DLGPROC)MainDialog);

    if(!UninitializeApp())
    {
        return 1;
    }

	ReleaseMutex(xMutex);
	CloseHandle(xMutex);

	return 0;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool MyExecute(char *sFile, char *sParameters)
{
    SHELLEXECUTEINFO seiExecuteInfo;

    memset(&seiExecuteInfo, 0, sizeof(seiExecuteInfo));
    
    seiExecuteInfo.cbSize       = sizeof(seiExecuteInfo);
    seiExecuteInfo.fMask        = 0;                
    seiExecuteInfo.hwnd         = 0;                
    seiExecuteInfo.lpVerb       = "open";
    seiExecuteInfo.lpFile       = sFile;
    seiExecuteInfo.lpParameters = sParameters;
    seiExecuteInfo.lpDirectory  = NULL;
    seiExecuteInfo.nShow        = SW_SHOW;
    seiExecuteInfo.hInstApp     = 0;
    
    if(ShellExecuteEx(&seiExecuteInfo))
        return true;
    else return false;

    return false;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool RestartClientService(void)
{
    return MyExecute(gsServiceControlExe, "/Start");
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#endif

