///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//  Copyright © NetworkDLS 2002, All rights reserved
//
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF 
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO 
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A 
// PARTICULAR PURPOSE.
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef _WINSERVICE_CPP
#define _WINSERVICE_CPP
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include <Windows.H>
#include <StdIO.H>
#include <StdLib.H>

#include "Common.H"

#ifdef _SQLESERVER
	#include "../SQLEServer/Source/Entry.H"
#elif _SQLECLIENT
	#include "../SQLEClient/Source/Entry.H"
#else
	#error "You must define either _SQLESERVER or _SQLECLIENT"
#endif

#include "WinService.H"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

DWORD dwServiceThread_ID = 0;

HANDLE hOpenConsole_Handle = NULL;
HANDLE hOpenMainDialog_Handle = NULL;
HANDLE hServiceEventHandle = NULL;
HANDLE hServiceThread_Handle = NULL;

SERVICE_STATUS stServiceStatus;
SERVICE_STATUS_HANDLE sthServiceStatusHandler = NULL;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

DWORD WINAPI ServiceThread(LPVOID lpVoid)
{
    if(!MainEntry())
    {
    	SetEvent( hServiceEventHandle );
        return _SVC_ERROR_;
    }

	SetEvent( hServiceEventHandle );

	return _SVC_SUCCESS_;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int ServiceEntry(void)
{
	// Create Service Data Array (Contains Information About Service)
	SERVICE_TABLE_ENTRY SrvTable[2];

	// Verify if service is running
	hServiceEventHandle = OpenEvent( SYNCHRONIZE, FALSE, ServiceEventName );
	if( hServiceEventHandle )
	{
		MessageBox(NULL, "The service is already active!", gsTitleCaption, MB_ICONINFORMATION);
		CloseHandle( hServiceEventHandle );
		return _SVC_ERROR_;
	}

	SECURITY_DESCRIPTOR SD;
	SECURITY_ATTRIBUTES SA;

	InitializeSecurityDescriptor(&SD, SECURITY_DESCRIPTOR_REVISION);

	SetSecurityDescriptorDacl(&SD, TRUE, 0, FALSE);

	SA.nLength = sizeof(SA);
	SA.lpSecurityDescriptor = &SD;
	SA.bInheritHandle = FALSE;

    hOpenMainDialog_Handle = CreateEvent( &SA, FALSE, FALSE, OpenMainDialogEventName);
	if( hOpenMainDialog_Handle == NULL)
	{
		MessageBox(NULL, "OpenMainDialog_Handle failed!", gsTitleCaption, MB_TASKMODAL|MB_ICONERROR);
		CloseHandle( hOpenMainDialog_Handle );
		return _SVC_ERROR_;
	}

    hOpenConsole_Handle = CreateEvent( &SA, FALSE, FALSE, OpenServerConsoleEventName);
	if( hOpenMainDialog_Handle == NULL)
	{
		MessageBox(NULL, "OpenConsole_Handle failed!", gsTitleCaption, MB_TASKMODAL|MB_ICONERROR);
		CloseHandle( hOpenConsole_Handle );
		return _SVC_ERROR_;
	}

	// Init Global Vars
	memset( &stServiceStatus, 0x00, sizeof(stServiceStatus) );
	hServiceEventHandle = NULL;

	// Load service
	SrvTable[0].lpServiceName = ServiceName;   // Service name
	SrvTable[0].lpServiceProc = ServiceMain;  // Service EntryPoint
	SrvTable[1].lpServiceName = NULL;		   // Nothing
	SrvTable[1].lpServiceProc = NULL;		   // Nothing

	// Pass the control to ServiceMain
	if( !StartServiceCtrlDispatcher(SrvTable) )
	{
		MessageBox(NULL, "Error starting the service!", gsTitleCaption, MB_TASKMODAL|MB_ICONERROR);
		return _SVC_ERROR_;
	}

	return _SVC_SUCCESS_;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void WINAPI ServiceMain(DWORD dwNumServiceArgs, LPTSTR *lpServiceArgs)
{
	DWORD Result = 0;

	// Register function that will receive messages from service manager
	// Must be the first action in Service Main
	sthServiceStatusHandler = RegisterServiceCtrlHandler( ServiceName, (LPHANDLER_FUNCTION)ControlHandler );
	if( sthServiceStatusHandler == (SERVICE_STATUS_HANDLE)NULL )
	{
		MessageBox( NULL, "Error in ServiceMain-RegisterServiceCtrlHandler !", gsTitleCaption, MB_TASKMODAL|MB_ICONERROR);
		return;
	}

	// Create a semaphore to keep service alive
	hServiceEventHandle = CreateEvent( NULL, FALSE, FALSE, ServiceEventName );

	stServiceStatus.dwServiceType 			  = SERVICE_WIN32 | SERVICE_INTERACTIVE_PROCESS;
	stServiceStatus.dwCurrentState			  = SERVICE_RUNNING;
	stServiceStatus.dwControlsAccepted		  = SERVICE_ACCEPT_STOP | SERVICE_ACCEPT_SHUTDOWN;
	stServiceStatus.dwWin32ExitCode			  = NO_ERROR;
	stServiceStatus.dwServiceSpecificExitCode = NO_ERROR;
	stServiceStatus.dwCheckPoint			  = 0;
	stServiceStatus.dwWaitHint				  = 0;

	// Make the first Status update
	Result = SetServiceStatus( sthServiceStatusHandler, &stServiceStatus );
	if(!Result)
	{
		MessageBox( NULL, "Error in ServiceMain-SetServiceStatus !", gsTitleCaption, MB_TASKMODAL|MB_ICONERROR);
		return;
	}

	// Start Main Service Thread (Program Will Brantch From That Thread)
	hServiceThread_Handle = CreateThread(NULL, 0, ServiceThread, (LPVOID) 0, 0, &dwServiceThread_ID);

	// Keep Service Running
	WaitForSingleObject( hServiceEventHandle, INFINITE );

	/* Service received a termination command */

	WaitOnApplicationToExit();

	// Terminate Precision Timer Thread
	TerminateThread(hServiceThread_Handle,0);

	// Close Precision Timer Thread Handle
	CloseHandle(hServiceThread_Handle);

	stServiceStatus.dwCurrentState = SERVICE_STOPPED;
	SetServiceStatus( sthServiceStatusHandler, &stServiceStatus );

	return;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void WINAPI ControlHandler(DWORD dwMessage)
{
	DWORD Result = 0;

	// Receive messages from service control dispatcher
	switch (dwMessage)
	{
//		case SERVICE_CONTROL_PAUSE :
//			stServiceStatus.dwWaitHint	 = 1000;
//			stServiceStatus.dwCurrentState = SERVICE_PAUSED;
//			break;

//		case SERVICE_CONTROL_CONTINUE :
//			stServiceStatus.dwWaitHint	 = 1000;
//			stServiceStatus.dwCurrentState = SERVICE_RUNNING;
//			break;

		case SERVICE_CONTROL_INTERROGATE :
			stServiceStatus.dwWaitHint	 = 500;
			break;

		case SERVICE_CONTROL_SHUTDOWN:
			stServiceStatus.dwWaitHint	 = 5000;
			stServiceStatus.dwCurrentState = SERVICE_STOP_PENDING;
			break;

		case SERVICE_CONTROL_STOP :
			stServiceStatus.dwWaitHint	 = 5000;
			stServiceStatus.dwCurrentState = SERVICE_STOP_PENDING;
			break;
	}

	// Update service status
	Result = SetServiceStatus( sthServiceStatusHandler, &stServiceStatus );
	if (!Result)
	{
		MessageBox( NULL, "Error in ControlHandler-SetServiceStatus !", gsTitleCaption, MB_TASKMODAL|MB_ICONERROR);
		SetEvent( hServiceEventHandle );
	}

	// If message is a stop or shutdown so, service is going to be stoped
	if ( (dwMessage == SERVICE_CONTROL_STOP) || (dwMessage == SERVICE_CONTROL_SHUTDOWN) )
	{
		SetEvent( hServiceEventHandle );
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/*
SERVICE_CONTROL_STOP
	Requests the service to stop. The hService handle must have SERVICE_STOP access.
SERVICE_CONTROL_PAUSE
	Requests the service to pause. The hService handle must have SERVICE_PAUSE_CONTINUE access.
SERVICE_CONTROL_CONTINUE
	Requests the paused service to resume. The hService handle must have SERVICE_PAUSE_CONTINUE access.
SERVICE_CONTROL_INTERROGATE
	Requests the service to update immediately its current status information to the service control manager. The hService handle must have SERVICE_INTERROGATE access.
SERVICE_CONTROL_SHUTDOWN
	The ControlService function fails if this control code is specified.
*/

int CommandService(DWORD dwCommand)
{
	SERVICE_STATUS stLocalServiceStatus;
	SC_HANDLE hServiceControlHandle = NULL;
	SC_HANDLE hServiceHandle 	   = NULL;

	BOOL Result = FALSE;

	// UnInstall The Service From Service Control Manager Database
	hServiceControlHandle = OpenSCManager( NULL, NULL, SC_MANAGER_CREATE_SERVICE );
	if( hServiceControlHandle == NULL )
	{
		MessageBox( NULL, "Error in CommandService-OpenSCManager !", gsTitleCaption, MB_TASKMODAL|MB_ICONERROR);
		return _SVC_ERROR_;
	}

	hServiceHandle = OpenService( hServiceControlHandle, ServiceName, SERVICE_ALL_ACCESS);

	if( hServiceHandle == NULL )
	{
		MessageBox( NULL, "Error in CommandService-OpenService !", gsTitleCaption, MB_TASKMODAL|MB_ICONERROR);
		CloseServiceHandle(hServiceControlHandle);
		return _SVC_ERROR_;
	}

	Result = ControlService(
		hServiceHandle,
		dwCommand,
		&stLocalServiceStatus 
	);

	if(Result == FALSE)
	{
		MessageBox( NULL, "Error in CommandService-ControlService !", gsTitleCaption, MB_TASKMODAL|MB_ICONERROR);
		CloseServiceHandle(hServiceHandle);
		CloseServiceHandle(hServiceControlHandle);
		return _SVC_ERROR_;
	}

	CloseServiceHandle(hServiceHandle);
	CloseServiceHandle(hServiceControlHandle);

	return _SVC_SUCCESS_;

}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int StartService(void)
{
	SC_HANDLE hServiceControlHandle = NULL;
	SC_HANDLE hServiceHandle = NULL;
	BOOL Result = FALSE;

	// UnInstall The Service From Service Control Manager Database
	hServiceControlHandle = OpenSCManager( NULL, NULL, SC_MANAGER_CREATE_SERVICE );
	if( hServiceControlHandle == NULL )
	{
		MessageBox( NULL, "Error in StartService-OpenSCManager !", gsTitleCaption, MB_TASKMODAL|MB_ICONERROR );
		return _SVC_ERROR_;
	}

	hServiceHandle = OpenService( hServiceControlHandle, ServiceName, SERVICE_ALL_ACCESS	);

	if( hServiceHandle == NULL )
	{
		MessageBox( NULL, "Error in StartService-OpenService !", gsTitleCaption, MB_TASKMODAL|MB_ICONERROR );
		CloseServiceHandle(hServiceControlHandle);
		return _SVC_ERROR_;
	}

	Result = StartService(
		hServiceHandle,	// Handle Of Service
		NULL,			// Number Of Arguments
		NULL			// Address Of Array Of Argument String Pointers
	);

	if(Result == FALSE)
	{
		MessageBox( NULL, "Error in StartService-StartService !", gsTitleCaption, MB_TASKMODAL|MB_ICONERROR);
		CloseServiceHandle(hServiceHandle);
		CloseServiceHandle(hServiceControlHandle);
		return _SVC_ERROR_;
	}

	CloseServiceHandle(hServiceHandle);
	CloseServiceHandle(hServiceControlHandle);

	return _SVC_SUCCESS_;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int InstallService(char *sModuleName)
{
	SC_HANDLE hServiceControlHandle;
	SC_HANDLE hServiceHandle;

	// Install the Service In The Control Manager Database
	hServiceControlHandle = OpenSCManager( NULL, NULL, SC_MANAGER_CREATE_SERVICE );
	if( hServiceControlHandle == NULL )
	{
		MessageBox( NULL, "Error in InstallService-OpenSCManager !", gsTitleCaption, MB_TASKMODAL|MB_ICONERROR);
		return _SVC_ERROR_;
	}

	hServiceHandle = CreateService(
		hServiceControlHandle,
		ServiceName,
		DisplayName,
		SERVICE_ALL_ACCESS,
		SERVICE_WIN32_OWN_PROCESS | SERVICE_INTERACTIVE_PROCESS,
		SERVICE_AUTO_START,
		SERVICE_ERROR_NORMAL,
		sModuleName,
		NULL,
		NULL,
		NULL,
		NULL,
		NULL );

	if( hServiceHandle == NULL )
	{
		MessageBox( NULL, "Error in InstallService-CreateService !", gsTitleCaption, MB_TASKMODAL|MB_ICONERROR);
		CloseServiceHandle( hServiceControlHandle );
		return _SVC_ERROR_;
	}

	CloseServiceHandle( hServiceHandle );
	CloseServiceHandle( hServiceControlHandle );

	return _SVC_SUCCESS_;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int UninstallService(void)
{
	SC_HANDLE hServiceControlHandle;
	SC_HANDLE hServiceHandle;

	// UnInstall The Service From Service Control Manager Database
	hServiceControlHandle = OpenSCManager( NULL, NULL, SC_MANAGER_CREATE_SERVICE);
	if( hServiceControlHandle == NULL )
	{
		MessageBox( NULL, "Error in UninstallService-OpenSCManager !", gsTitleCaption, MB_TASKMODAL|MB_ICONERROR);
		return _SVC_ERROR_;
	}

	hServiceHandle = OpenService( hServiceControlHandle, ServiceName, SERVICE_ALL_ACCESS);

	if( hServiceHandle == NULL )
	{
		MessageBox( NULL, "Error in UninstallService-OpenService !", gsTitleCaption, MB_TASKMODAL|MB_ICONERROR);
		CloseServiceHandle(hServiceControlHandle);
		return _SVC_ERROR_;
	}

	if( !DeleteService(hServiceHandle) )
	{
		MessageBox( NULL, "Error in UninstallService-DeleteService !", gsTitleCaption, MB_TASKMODAL|MB_ICONERROR);
		CloseServiceHandle(hServiceHandle);
		CloseServiceHandle(hServiceControlHandle);
		return _SVC_ERROR_;
	}

	CloseServiceHandle(hServiceHandle);
	CloseServiceHandle(hServiceControlHandle);

	return _SVC_SUCCESS_;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int IsServiceInstalled(void)
{
	SC_HANDLE hServiceControlHandle;
	SC_HANDLE hServiceHandle;

	hServiceControlHandle = OpenSCManager( NULL,  NULL, SC_MANAGER_CREATE_SERVICE);
	if( hServiceControlHandle == NULL )
	{
		MessageBox( NULL, "Error in IsServiceInstalled-OpenSCManager !", gsTitleCaption, MB_TASKMODAL|MB_ICONERROR);
		return _SVC_ERROR_;
	}

	hServiceHandle = OpenService( hServiceControlHandle, ServiceName, SERVICE_ALL_ACCESS);

	if( hServiceHandle == NULL)
	{
		CloseServiceHandle( hServiceHandle );
		CloseServiceHandle( hServiceControlHandle );
		return _SVC_NO_;
	}

	CloseServiceHandle( hServiceHandle );
	CloseServiceHandle( hServiceControlHandle );

	return _SVC_YES_;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int IsServiceActive(void)
{
	HANDLE hEventHandle = OpenEvent(SYNCHRONIZE, FALSE, ServiceEventName);
	if(hEventHandle)
	{
		CloseHandle( hEventHandle );
		return _SVC_YES_;
	}

    return _SVC_NO_;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int TriggerEvent(char *sEventName)
{
	HANDLE hLocalEventHandle = NULL;
	int iResult = _SVC_ERROR_;

	if((hLocalEventHandle = OpenEvent(EVENT_MODIFY_STATE, FALSE, sEventName)) == NULL)
	{
        return _SVC_ERROR_;
	}

    if(!SetEvent(hLocalEventHandle))
	{
		iResult = _SVC_ERROR_;
	}
	else{
		iResult = _SVC_SUCCESS_;
	}

	CloseHandle(hLocalEventHandle);

	return iResult;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int SetServiceDescription(char *sDescr)
{
	SC_HANDLE hServiceControlHandle = NULL;
	SC_HANDLE hServiceHandle = NULL;
	BOOL Result = FALSE;

	// UnInstall The Service From Service Control Manager Database
	hServiceControlHandle = OpenSCManager( NULL, NULL, SC_MANAGER_CREATE_SERVICE );
	if( hServiceControlHandle == NULL )
	{
		MessageBox( NULL, "Error in SetServiceDescription-OpenSCManager !", gsTitleCaption, MB_TASKMODAL|MB_ICONERROR);
		return _SVC_ERROR_;
	}

	hServiceHandle = OpenService( hServiceControlHandle, ServiceName, SERVICE_ALL_ACCESS);

	if( hServiceHandle == NULL )
	{
		MessageBox( NULL, "Error in SetServiceDescription-OpenService !", gsTitleCaption, MB_TASKMODAL|MB_ICONERROR);
		CloseServiceHandle(hServiceControlHandle);
		return _SVC_ERROR_;
	}

	SERVICE_DESCRIPTION MySD;

	memset(&MySD, 0, sizeof(MySD));

	MySD.lpDescription = sDescr;

	ChangeServiceConfig2(hServiceHandle, SERVICE_CONFIG_DESCRIPTION, &MySD);

	CloseServiceHandle(hServiceHandle);
	CloseServiceHandle(hServiceControlHandle);

	return _SVC_SUCCESS_;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#endif
