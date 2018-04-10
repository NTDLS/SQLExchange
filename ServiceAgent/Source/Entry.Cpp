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

#include "../../../@Libraries/CTrayIcon/CTrayIcon.H"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

using namespace NSWFL::Registry;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#define ID_STARTSERVER WM_USER + 6
#define ID_STOPSERVER  WM_USER + 7
#define ID_CANCEL      WM_USER + 8
#define ID_EXIT        WM_USER + 9

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

HINSTANCE ghAppInstance = NULL;

char gsAppCopyRight[64];
char gsFileVersion[32];
char gsTitleCaption[64];
char gsRegistryKey[64];
char gsServiceName[256];

char gsServiceControlExe[MAX_PATH];
char gsPath[MAX_PATH];

CMemPool *pMem;
CTrayIcon TI;

HANDLE hServiceEventHandle = NULL;
HANDLE hServiceThread_Handle = NULL;
HANDLE hExitApplication = NULL;

SERVICE_STATUS stServiceStatus;
SERVICE_STATUS_HANDLE sthServiceStatusHandler = NULL;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void MyTrayIconProc(CTrayIcon *lpTI, HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	if(uMsg == WM_CTRAYICON_INITIALIZE)
	{
		SetTimer(hWnd, 0, 1000, NULL);
	}
	else if(uMsg == WM_TIMER)
	{
		if(Get_BOOLRegistryValue(HKEY_LOCAL_MACHINE, gsRegistryKey, "TerminateAgent"))
		{
			DestroyWindow(hWnd);
		}
	}
	else if(uMsg == WM_COMMAND)
    {
        if(LOWORD(wParam) == ID_EXIT)
        {
            DestroyWindow(hWnd);
        }
		else if(LOWORD(wParam) == ID_STARTSERVER)
        {
			StartService();
        }
		else if(LOWORD(wParam) == ID_STOPSERVER)
        {
			StopService();
        }
		else if(LOWORD(wParam) == ID_CANCEL)
        {
        }
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

DWORD WINAPI ExitThread(LPVOID lpVoid)
{
	WaitForSingleObject(hExitApplication, INFINITE);

	SendMessage(TI.hTrayHandle, WM_COMMAND, ID_EXIT, 0);

	return 0;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool IsServiceInstalled(void)
{
	SC_HANDLE hServiceControlHandle = NULL;
	SC_HANDLE hServiceHandle = NULL;

	if((hServiceControlHandle = OpenSCManager(NULL,  NULL, SC_MANAGER_CREATE_SERVICE)) == NULL)
	{
		MessageBox(NULL, "Error in [IsServiceInstalled]->OpenSCManager()", gsTitleCaption, MB_ICONERROR);
		return false;
	}

	if((hServiceHandle = OpenService(hServiceControlHandle, gsServiceName, SERVICE_INTERROGATE)) == NULL)
	{
		CloseServiceHandle(hServiceHandle);
		CloseServiceHandle(hServiceControlHandle);
		return false;
	}

	CloseServiceHandle(hServiceHandle);
	CloseServiceHandle(hServiceControlHandle);

	return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool IsServiceActive(void)
{
	SC_HANDLE hServiceControlHandle = NULL;
	SC_HANDLE hServiceHandle = NULL;

	SERVICE_STATUS SvcStat;

	memset(&SvcStat, 0, sizeof(SvcStat));

	if((hServiceControlHandle = OpenSCManager(NULL,  NULL, SC_MANAGER_CREATE_SERVICE)) == NULL)
	{
		MessageBox(NULL, "Error in [IsServiceActive]->OpenSCManager()", gsTitleCaption, MB_ICONERROR);
		return false;
	}

	if((hServiceHandle = OpenService(hServiceControlHandle, gsServiceName, SERVICE_QUERY_STATUS)) == NULL)
	{
		CloseServiceHandle(hServiceHandle);
		CloseServiceHandle(hServiceControlHandle);
		return false;
	}

	if(!QueryServiceStatus(hServiceHandle, &SvcStat))
	{
		MessageBox(NULL, "Error in [IsServiceActive]->QueryServiceStatus()", gsTitleCaption, MB_ICONERROR);
		return false;
	}

	CloseServiceHandle(hServiceHandle);
	CloseServiceHandle(hServiceControlHandle);

	return (SvcStat.dwCurrentState == SERVICE_RUNNING);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool StopService(void)
{
	SERVICE_STATUS stLocalServiceStatus;
	SC_HANDLE hServiceControlHandle = NULL;
	SC_HANDLE hServiceHandle = NULL;

	// UnInstall The Service From Service Control Manager Database
	if((hServiceControlHandle = OpenSCManager( NULL, NULL, SC_MANAGER_CREATE_SERVICE)) == NULL)
	{
		MessageBox( NULL, "Error in [StopService]->OpenSCManager()", gsTitleCaption, MB_TASKMODAL|MB_ICONERROR);
		return false;
	}

	if((hServiceHandle = OpenService(hServiceControlHandle, gsServiceName, SERVICE_ALL_ACCESS)) == NULL)
	{
		MessageBox( NULL, "Error in ]StopService]->OpenService()", gsTitleCaption, MB_TASKMODAL|MB_ICONERROR);
		CloseServiceHandle(hServiceControlHandle);
		return false;
	}

	if(!ControlService(hServiceHandle, SERVICE_CONTROL_STOP, &stLocalServiceStatus))
	{
		MessageBox( NULL, "Error in [StopService]->ControlService()", gsTitleCaption, MB_TASKMODAL|MB_ICONERROR);
		CloseServiceHandle(hServiceHandle);
		CloseServiceHandle(hServiceControlHandle);
		return false;
	}

	CloseServiceHandle(hServiceHandle);
	CloseServiceHandle(hServiceControlHandle);

	return true;

}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool StartService(void)
{
	SC_HANDLE hServiceControlHandle = NULL;
	SC_HANDLE hServiceHandle = NULL;

	// UnInstall The Service From Service Control Manager Database
	if((hServiceControlHandle = OpenSCManager(NULL, NULL, SC_MANAGER_CREATE_SERVICE)) == NULL)
	{
		MessageBox( NULL, "Error in [StartService]->OpenSCManager()", gsTitleCaption, MB_TASKMODAL|MB_ICONERROR);
		return false;
	}

	if((hServiceHandle = OpenService(hServiceControlHandle, gsServiceName, SERVICE_ALL_ACCESS)) == NULL)
	{
		MessageBox(NULL, "Error in [StartService]->OpenService()", gsTitleCaption, MB_TASKMODAL|MB_ICONERROR);
		CloseServiceHandle(hServiceControlHandle);
		return false;
	}

	if(!StartService(hServiceHandle, NULL, NULL))
	{
		MessageBox( NULL, "Error in [StartService]->StartService()", gsTitleCaption, MB_TASKMODAL|MB_ICONERROR);
		CloseServiceHandle(hServiceHandle);
		CloseServiceHandle(hServiceControlHandle);
		return false;
	}

	CloseServiceHandle(hServiceHandle);
	CloseServiceHandle(hServiceControlHandle);

	return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

DWORD WINAPI ServiceThread(LPVOID lpVoid)
{
	char sCaption[1024];

	while(true)
	{
		while(!IsServiceInstalled())
		{
			sprintf_s(sCaption, sizeof(sCaption), "%s (Service is not installed)", gsServiceName);
			TI.Update(IDI_STOPPED, sCaption);
			Sleep(5000);
		}

		while(IsServiceActive())
		{
			sprintf_s(sCaption, sizeof(sCaption), "%s (Service is active)", gsServiceName);
			TI.Update(IDI_RUNNING, sCaption);
			Sleep(2500);
		}

		sprintf_s(sCaption, sizeof(sCaption), "%s (Service is inactive)", gsServiceName);
		TI.Update(IDI_STOPPED, sCaption);

		Sleep(2500);
	}
	
	return 0;
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

int TriggerEvent(char *sEventName)
{
	HANDLE hLocalEventHandle = NULL;
	bool bResult = false;

	if((hLocalEventHandle = OpenEvent(EVENT_MODIFY_STATE, FALSE, sEventName)) == NULL)
	{
        return false;
	}

    bResult = (SetEvent(hLocalEventHandle) == TRUE);

	CloseHandle(hLocalEventHandle);

	return bResult;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool CreateExitEvent(void)
{
	SECURITY_DESCRIPTOR SD;
	SECURITY_ATTRIBUTES SA;

	InitializeSecurityDescriptor(&SD, SECURITY_DESCRIPTOR_REVISION);

	SetSecurityDescriptorDacl(&SD, TRUE, 0, FALSE);

	SA.nLength = sizeof(SA);
	SA.lpSecurityDescriptor = &SD;
	SA.bInheritHandle = FALSE;

	if((hExitApplication = CreateEvent( &SA, FALSE, FALSE, EXIT_APPLICATION_EVENT)) == NULL)
	{
        return false;
	}

	return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevious, LPSTR sCmdLine, int iCmdShow)
{
    #ifdef _DEBUG
	ghAppInstance = GetModuleHandle(NULL);
    #else
	ghAppInstance = hInstance;
    #endif

	HANDLE hMutex = NULL;

    if(!InitializeApp())
    {
        return 1;
    }

	if(Get_BOOLRegistryValue(HKEY_LOCAL_MACHINE, gsRegistryKey, "TerminateAgent"))
	{
		return 0;
	}

	if(strlen(sCmdLine) == 0)
    {
        MessageBox(NULL, "No service name was specified.", gsTitleCaption, MB_ICONSTOP|MB_TASKMODAL);
        return false;
    }

	CCmdLine *pCmd = new CCmdLine;

	if(pCmd->Count() != 2)
	{
		MessageBox(NULL, "Incorrect number of command line parameters.", gsTitleCaption, MB_ICONSTOP|MB_TASKMODAL);
		delete pCmd;
		return 1;
	}
	else if(pCmd->Length(1) >= sizeof(gsServiceName))
	{
		MessageBox(NULL, "An invalid parameter was supplied.", gsTitleCaption, MB_ICONSTOP|MB_TASKMODAL);
		delete pCmd;
		return 1;
	}
	else if(pCmd->IsCommand(1, "/Close"))
	{
		TriggerEvent(EXIT_APPLICATION_EVENT);
		delete pCmd;
		return 1;
	}

	CreateExitEvent();
	strcpy_s(gsServiceName, sizeof(gsServiceName), pCmd->Item(1));

	delete pCmd;

	char sMutexName[1024];
	sprintf_s(sMutexName, sizeof(sMutexName), "%s_%s", gsTitleCaption, sCmdLine);
	if((hMutex = OpenMutex(SYNCHRONIZE, FALSE, sMutexName)))
	{
		MessageBox(NULL, "Only one instance can run at a time!", gsTitleCaption, MB_ICONASTERISK | MB_TASKMODAL);
		ReleaseMutex(hMutex);
		CloseHandle(hMutex);
		return 0;
	}
	else hMutex = CreateMutex(0, 0, GetBuildArchitecture(gsTitleCaption, sizeof(sMutexName), sMutexName));

	HANDLE hExitThread = CreateThread(NULL, 0, ExitThread, (LPVOID) 0, 0, NULL);
	HANDLE hThread = CreateThread(NULL, 0, ServiceThread, (LPVOID) 0, 0, NULL);

    HMENU hPopupMenu = CreateMenu();
	AppendMenu(hPopupMenu, MF_STRING, ID_STARTSERVER, "&Start Service");
	AppendMenu(hPopupMenu, MF_STRING, ID_STOPSERVER, "S&top Service");
    AppendMenu(hPopupMenu, MF_SEPARATOR, 0, "");
    AppendMenu(hPopupMenu, MF_STRING, ID_CANCEL, "&Cancel");
    AppendMenu(hPopupMenu, MF_SEPARATOR, 0, "");
	AppendMenu(hPopupMenu, MF_STRING, ID_EXIT, "E&xit");

	TI.Create(IDI_MAIN, gsServiceName, hPopupMenu, MyTrayIconProc);

	TI.Destroy();

	CloseHandle(hExitThread);
	CloseHandle(hThread);

	UninitializeApp();

	ReleaseMutex(hMutex);
	CloseHandle(hMutex);

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
