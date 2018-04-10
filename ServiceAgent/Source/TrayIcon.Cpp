#ifndef _TRAYICON_CPP
#define _TRAYICON_CPP
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include <Windows.H>
#include <Stdio.H>
#include <Stdlib.H>

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "../Resources/Resource.H"

#include "Entry.H"
#include "Init.H"
#include "TrayIcon.H"
#include "TrayHandler.H"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

UINT TB_CREATED = -1;

NOTIFYICONDATA MyTrayNotifiy; // Tray Icon Structure

HMENU hMainMenu = NULL; //Tray Icon Popup Menu
HMENU hFileMenu = NULL;      //Tray Icon Popup Menu

HWND hTrayHandle = NULL;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool LoadTrayIcon(int iIcon, char *sTitle)
{
    WNDCLASS MyWC;
    MSG MyMsg;

    MyWC.style         = CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS;
    MyWC.lpfnWndProc   = TrayIconProc;
    MyWC.cbClsExtra    = 0;
    MyWC.cbWndExtra    = 0;
    MyWC.hInstance     = ghAppInstance;
    MyWC.hIcon         = LoadIcon(NULL, IDI_WINLOGO);
    MyWC.hCursor       = LoadCursor(NULL, IDC_ARROW);
    MyWC.hbrBackground = GetSysColorBrush(COLOR_BTNFACE);
    MyWC.lpszMenuName  = NULL;
    MyWC.lpszClassName = gsTitleCaption;

    RegisterClass(&MyWC);

    hTrayHandle = CreateWindow(
		gsTitleCaption,
		sTitle,
		WS_POPUP|WS_SYSMENU,
		0, 0, 0, 0,
		NULL,
		(HMENU) NULL,
		ghAppInstance,
		NULL
		);

    MyTrayNotifiy.cbSize           = sizeof(NOTIFYICONDATA);
    MyTrayNotifiy.hWnd             = hTrayHandle;
    MyTrayNotifiy.hIcon            = LoadIcon(ghAppInstance, MAKEINTRESOURCE(iIcon) );
    MyTrayNotifiy.uFlags           = NIF_ICON | NIF_TIP | NIF_MESSAGE;
    MyTrayNotifiy.uCallbackMessage = WM_SHELLNOTIFY;

    strcpy_s(MyTrayNotifiy.szTip, sizeof(MyTrayNotifiy.szTip), sTitle);

    Shell_NotifyIcon(NIM_ADD, &MyTrayNotifiy);

    hMainMenu = CreateMenu();
    hFileMenu = CreateMenu();

	AppendMenu(hFileMenu, MF_STRING, ID_STARTSERVER, "&Start Service");
	AppendMenu(hFileMenu, MF_STRING, ID_STOPSERVER, "S&top Service");
    AppendMenu(hFileMenu, MF_SEPARATOR, 0, "");
    AppendMenu(hFileMenu, MF_STRING, ID_CANCEL, "&Cancel");
    AppendMenu(hFileMenu, MF_SEPARATOR, 0, "");
	AppendMenu(hFileMenu, MF_STRING, ID_EXIT, "E&xit");
    InsertMenu(hMainMenu, 0, MF_POPUP, (UINT)hFileMenu, "invisible menu");

    while( GetMessage(&MyMsg, NULL, 0, 0) )
    {
        if( !IsWindow(hTrayHandle) || !IsDialogMessage(hTrayHandle, &MyMsg) )
        {
            TranslateMessage(&MyMsg);
            DispatchMessage(&MyMsg);
        }
    }

    return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void UpdateTrayIcon(int iIcon, char *sTitle)
{
    if(iIcon != -1)
	{
		MyTrayNotifiy.hIcon = LoadIcon(ghAppInstance, MAKEINTRESOURCE(iIcon) );
	}

	if(sTitle)
	{
		strcpy_s(MyTrayNotifiy.szTip, sizeof(MyTrayNotifiy.szTip), sTitle);
	}

    Shell_NotifyIcon(NIM_MODIFY, &MyTrayNotifiy);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool DestroyTrayIcon(void)
{
    return (Shell_NotifyIcon(NIM_DELETE, &MyTrayNotifiy) == TRUE);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#endif
