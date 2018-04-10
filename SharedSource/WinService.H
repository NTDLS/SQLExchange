///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//  Copyright © NetworkDLS 2002, All rights reserved
//
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF 
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO 
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A 
// PARTICULAR PURPOSE.
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef _WINSERVICE_H
#define _WINSERVICE_H
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#ifdef _SQLESERVER
	#define SERVICESTUB "SQL-Exchange Server"
	#ifdef _X64_
		#define DisplayName                "NetworkDLS "SERVICESTUB" x64"
		#define ServiceName                SERVICESTUB" x64"
		#define ServiceEventName           SERVICESTUB" Event x64"

		#define OpenMainDialogEventName    SERVICESTUB" OpenMainDialog Event x64"
		#define OpenServerConsoleEventName SERVICESTUB" OpenConsole Event x64"
	#elif _IA64_
		#define DisplayName                "NetworkDLS "SERVICESTUB" IA64"
		#define ServiceName                SERVICESTUB" IA64"
		#define ServiceEventName           SERVICESTUB" Event IA64"

		#define OpenMainDialogEventName    SERVICESTUB" OpenMainDialog Event IA64"
		#define OpenServerConsoleEventName SERVICESTUB" OpenConsole Event IA64"
	#else
		#define DisplayName                "NetworkDLS "SERVICESTUB" x86"
		#define ServiceName                SERVICESTUB" x86"
		#define ServiceEventName           SERVICESTUB" Event x86"

		#define OpenMainDialogEventName    SERVICESTUB" OpenMainDialog Event x86"
		#define OpenServerConsoleEventName SERVICESTUB" OpenConsole Event x86"
	#endif
#elif _SQLECLIENT
	#define SERVICESTUB "SQL-Exchange Client"
	#ifdef _X64_
		#define DisplayName                "NetworkDLS "SERVICESTUB" x64"
		#define ServiceName                SERVICESTUB" x64"
		#define ServiceEventName           SERVICESTUB" Event x64"

		#define OpenMainDialogEventName    SERVICESTUB" OpenMainDialog Event x64"
		#define OpenServerConsoleEventName SERVICESTUB" OpenConsole Event x64"
	#elif _IA64_
		#define DisplayName                "NetworkDLS "SERVICESTUB" IA64"
		#define ServiceName                SERVICESTUB" IA64"
		#define ServiceEventName           SERVICESTUB" Event IA64"

		#define OpenMainDialogEventName    SERVICESTUB" OpenMainDialog Event IA64"
		#define OpenServerConsoleEventName SERVICESTUB" OpenConsole Event IA64"
	#else
		#define DisplayName                "NetworkDLS "SERVICESTUB" x86"
		#define ServiceName                SERVICESTUB" x86"
		#define ServiceEventName           SERVICESTUB" Event x86"

		#define OpenMainDialogEventName    SERVICESTUB" OpenMainDialog Event x86"
		#define OpenServerConsoleEventName SERVICESTUB" OpenConsole Event x86"
	#endif
#else
	#error "You must define either _SQLESERVER or _SQLECLIENT"
#endif

#define _SVC_YES_	  1
#define _SVC_NO_	  2
#define _SVC_ERROR_   3
#define _SVC_SUCCESS_ 4

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

DWORD WINAPI ServiceThread(LPVOID lpVoid);

int CommandService(DWORD dwCommand);
int InstallService(char *sModuleName);
int IsServiceActive(void);
int IsServiceInstalled(void);
int ServiceEntry(void);
int StartService(void);
int TriggerEvent(char *sEventName);
int UninstallService(void);
int SetServiceDescription(char *sDescr);

void WINAPI ControlHandler(DWORD dwMessage);
void WINAPI ServiceMain(DWORD dwNumServiceArgs, LPTSTR *lpServiceArgs);

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

extern DWORD dwServiceThread_ID;

extern HANDLE hOpenConsole_Handle;
extern HANDLE hOpenMainDialog_Handle;
extern HANDLE hServiceEventHandle;
extern HANDLE hServiceThread_Handle;

extern SERVICE_STATUS stServiceStatus;
extern SERVICE_STATUS_HANDLE sthServiceStatusHandler;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#endif
