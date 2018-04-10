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

#include "../../SharedSource/Common.H"

#include "Entry.H"
#include "Init.H"
#include "Routines.H"
#include "Console.H"

#include "../Dialogs/MainDlg.H"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

using namespace NSWFL::System;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

HINSTANCE ghAppInstance = NULL;

char gsAppCopyRight[64];
char gsFileVersion[32];
char gsTitleCaption[64];
char gsRegistryKey[255];
char gsEventName[255];

HANDLE hOpenConsole_Handle = NULL;
HANDLE hOpenMainDialog_Handle = NULL;

CMemPool *pMem;		//The global memory manager.
CService gService;	//The global service.

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Service_OnShutDown(void)
{
    if(MDI.hWnd)
    {
		SendMessage(MDI.hWnd, WM_CLOSE, 0, 0);
    }

	if(gbConsoleOpen)
	{
		CloseConsole();
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool Service_Thread(LPVOID lpVoid)
{
	HANDLE MutexHandle = NULL;
	char sEventName[1024];

	//-------------------------------------------------------------------------------
	SECURITY_DESCRIPTOR SD;
	SECURITY_ATTRIBUTES SA;
	if(!CreateGenericSecurityDescriptor(&SD, &SA))
	{
		MessageBox(NULL, "Failed to initialize security descriptor!",
			gsTitleCaption, MB_TASKMODAL|MB_ICONERROR);
		CloseHandle( hOpenMainDialog_Handle );
		return false;
	}

	GetBuildArchitecture(gsTitleCaption, sizeof(sEventName), sEventName);
	if((MutexHandle = CreateMutex(&SA, FALSE, sEventName)) != NULL)
	{
		if(!gbForceAllowMultiInstance)
		{
			if(GetLastError() == ERROR_ALREADY_EXISTS)
			{
				MessageBox(NULL, "The application is already running.",
					gsTitleCaption, MB_SYSTEMMODAL + MB_TASKMODAL);
				return false;
			}
		}
	}

	if(!IsServerConfigured(true))
	{
		return false;
	}

	WriteSysLogEx("Service started.", EVENT_INFO);

	if(gbStartAutomatically)
	{
		if(!StartServer())
		{
			return false;
		}
	}
	else WriteSysLogEx("Server is not set to start automatically.", EVENT_INFO);

	if(gService.RunningAsService())
	{
		GetBuildArchitecture(gsTitleCaption, "MainDialog", sizeof(sEventName), sEventName);
		if((hOpenMainDialog_Handle = CreateEvent(&SA, FALSE, FALSE, sEventName)) == NULL)
		{
			MessageBox(NULL, "Failed to initialize the open dialog event!",
				gsTitleCaption, MB_TASKMODAL|MB_ICONERROR);
			CloseHandle( hOpenMainDialog_Handle );
			return false;
		}

		GetBuildArchitecture(gsTitleCaption, "Console", sizeof(sEventName), sEventName);
		if((hOpenConsole_Handle = CreateEvent(&SA, FALSE, FALSE, sEventName)) == NULL)
		{
			MessageBox(NULL, "Failed to initialize the open console event!",
				gsTitleCaption, MB_TASKMODAL|MB_ICONERROR);
			CloseHandle( hOpenConsole_Handle );
			return false;
		}

		int iStatCount = 10000;
		while(gService.Alive())
		{
			if(WaitForSingleObject(hOpenMainDialog_Handle, 0) == WAIT_OBJECT_0)
			{
				DialogBox(ghAppInstance, MAKEINTRESOURCE(IDD_MAIN), NULL, (DLGPROC)MainDialog);
			}

    		if(WaitForSingleObject(hOpenConsole_Handle, 0) == WAIT_OBJECT_0)
			{
				if(gbConsoleOpen)
				{
					CloseConsole();
				}
				else{
					OpenConsole();
				}
			}

			if(iStatCount++ > 1000)
			{
				UpdateStatistics();
				iStatCount = 0;
			}

			Sleep(1);
		}

		CloseHandle(hOpenMainDialog_Handle);
		CloseHandle(hOpenConsole_Handle);
	}
	else{
		DialogBox(ghAppInstance, MAKEINTRESOURCE(IDD_MAIN), 0, (DLGPROC)MainDialog);
	}

	WriteSysLogEx("Service Stopped.", EVENT_INFO);

	if(gServer.IsActive())
	{
		if(!StopServer())
		{
			return false;
		}
	}

	return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevious, LPSTR sCmdLine, int iCmdShow)
{
	int iReturnVal = 0;
	
	ghAppInstance = hInstance;

	if(!IsWinNT())
	{
		MessageBox(
			NULL,
			"This application was designed to run only on NT based systems.",
			"SQL-Exchange Server",
			MB_ICONASTERISK | MB_SYSTEMMODAL | MB_TASKMODAL
		);
		return 2;
	}

	gbExcludeInit = ExcludedFromInit(sCmdLine);

	if(!InitializeApp())
    {
        return 1;
    }

	char sEventName[1024];
	GetBuildArchitecture(gsTitleCaption, sizeof(sEventName), sEventName);
	gService.Initialize(sEventName, &Service_Thread, &Service_OnShutDown);

	if(strlen(sCmdLine) > 0)
	{
		CCmdLine *pCmd = new CCmdLine;
		iReturnVal = ProcessCommandLine(pCmd);
		delete pCmd;
	}
	else{
		if(IsApplicationRunning())
		{
			return 0;
		}

		iReturnVal =  gService.RunAsStandAlone();
	}

	if(!UninitializeApp())
    {
        return 1;
    }

	return iReturnVal;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int ProcessCommandLine(CCmdLine *lpCmd)
{
	for(int iArg = 0; iArg < lpCmd->Count(); iArg++)
	{
		//------------------------------------------------------------------------------------------
		if(lpCmd->IsCommand(iArg, "/NonService"))
		{
			if(IsApplicationRunning())
			{
				return 0;
			}
			return gService.RunAsStandAlone();
		}
		//------------------------------------------------------------------------------------------
		else if(lpCmd->IsCommand(iArg, "/RunService"))
		{
			if(IsApplicationRunning())
			{
				return 0;
			}
			return gService.RunAsService();
		}
		//------------------------------------------------------------------------------------------
		else if(lpCmd->IsCommand(iArg, "/Config"))
		{
			LaunchConfigDlg(GetActiveWindow());
			return 0;
		}
		//------------------------------------------------------------------------------------------
		else if(lpCmd->IsCommand(iArg, "/OpenConsole"))
		{
			if( gService.IsInstalled() == _SVC_NO_ )
   			{
       			MessageBox(NULL, "The service is not installed.", gsTitleCaption, MB_ICONINFORMATION);
   				return 0;
			}

			if(gService.IsActive() == _SVC_NO_)
			{
				if(IsApplicationRunning())
				{
					return 0;
				}

				if(MessageBox(NULL, "The service is not started. Would you like to start it now?",
					gsTitleCaption, MB_YESNO|MB_APPLMODAL|MB_ICONQUESTION) == IDNO)
				{
					return 0;
				}

    			if(gService.Start() != _SVC_SUCCESS_)
				{
        			if(IsServerConfigured(false))
					{
						MessageBox(NULL, "Failed to start the service.", gsTitleCaption, MB_ICONINFORMATION);
					}
       				return 0;
				}
			}

			char sEventName[1024];
			GetBuildArchitecture(gsTitleCaption, "Console", sizeof(sEventName), sEventName);
			if(gService.TriggerEvent(sEventName) != _SVC_SUCCESS_)
			{
				MessageBox(NULL, "Failed to modify service event state.", gsTitleCaption, MB_ICONINFORMATION);
			}

			return 0;
		}
		//------------------------------------------------------------------------------------------
		else if(lpCmd->IsCommand(iArg, "/OpenMainDialog"))
		{
			if(gService.IsInstalled() == _SVC_NO_)
   			{
       			MessageBox(NULL, "The service is not installed.", gsTitleCaption, MB_ICONINFORMATION);
   				return 0;
			}

			if(gService.IsActive() == _SVC_NO_)
			{
				if(IsApplicationRunning())
				{
					return 0;
				}

				if(MessageBox(NULL, "The service is not started. Would you like to start it now?",
					gsTitleCaption, MB_YESNO|MB_APPLMODAL|MB_ICONQUESTION) == IDNO)
				{
					return 0;
				}

    			if(gService.Start() != _SVC_SUCCESS_)
				{
        			if(IsServerConfigured(false))
					{
						MessageBox(NULL, "Failed to start the service.", gsTitleCaption, MB_ICONINFORMATION);
					}
       				return 0;
				}
			}

			char sEventName[1024];
			GetBuildArchitecture(gsTitleCaption, "MainDialog", sizeof(sEventName), sEventName);
			if(gService.TriggerEvent(sEventName) != _SVC_SUCCESS_)
			{
				MessageBox(NULL, "Failed to modify service event state.", gsTitleCaption, MB_ICONINFORMATION);
			}

			return 0;
		}
		//------------------------------------------------------------------------------------------
		else if(lpCmd->IsCommand(iArg, "/Install"))
		{
			char sApplication[257];
			char sEventName[1024];

			if(gService.IsInstalled() == _SVC_YES_)
			{
    			//MessageBox(NULL, "The service is already installed.", gsTitleCaption, MB_ICONINFORMATION);
				return 0;
			}

			sprintf_s(sApplication, sizeof(sApplication), "NetworkDLS %s", gsTitleCaption);
			GetBuildArchitecture(sApplication, sizeof(sEventName), sEventName);
			if(gService.Install(sEventName) == _SVC_SUCCESS_ )
			{
				gService.SetDescription("Provides clients with secure high performance database replication over TCP/IP.");
				return 0;
			}
			else{
				MessageBox(NULL, "Failed to install the service.", gsTitleCaption, MB_ICONINFORMATION);
				return 0;
			}

			return 0;
		}
		//------------------------------------------------------------------------------------------
		else if(lpCmd->IsCommand(iArg, "/Start"))
		{
			if(IsApplicationRunning())
			{
				return 0;
			}
			else if(gService.IsInstalled() == _SVC_NO_)
			{
    			MessageBox(NULL, "The service is not installed.", gsTitleCaption, MB_ICONINFORMATION);
				return 0;
			}
			else if(gService.IsActive() == _SVC_YES_)
			{
				return 0;
			}
			else if(gService.Start() != _SVC_SUCCESS_)
			{
    			MessageBox(NULL, "Failed to start the service.", gsTitleCaption, MB_ICONINFORMATION);
   				return 0;
			}

			return 0;
		}
		//------------------------------------------------------------------------------------------
		else if(lpCmd->IsCommand(iArg, "/Delete"))
		{
			if(gService.IsInstalled() == _SVC_NO_)
			{
				return 0;
			}
			else if(gService.IsActive() == _SVC_YES_)
			{
				gService.Stop(); // Failure would not be an error.
				return 0;
			}
    		else if(gService.Uninstall() != _SVC_SUCCESS_)
    		{
    			MessageBox(NULL, "Failed to mark the service for deletion.", gsTitleCaption, MB_ICONINFORMATION);
				return 0;
			}

			return 0;
		}
		//------------------------------------------------------------------------------------------
		else if(lpCmd->IsCommand(iArg, "/Stop"))
		{
			if(gService.IsInstalled() == _SVC_NO_)
			{
    			MessageBox(NULL, "The service is not installed.", gsTitleCaption, MB_ICONINFORMATION);
				return 0;
			}
			else if(gService.IsActive() == _SVC_NO_)
			{
				return 0; //Already stopped.
			}
    		else if(gService.Stop() != _SVC_SUCCESS_)
    		{
    			MessageBox(NULL, "Failed to stop the service.", gsTitleCaption, MB_ICONINFORMATION);
				return 0;
			}
			return 0;
		}
		//------------------------------------------------------------------------------------------
	}

	return 0;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#endif
