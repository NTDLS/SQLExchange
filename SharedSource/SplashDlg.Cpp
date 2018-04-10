///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//  Copyright © NetworkDLS 2002, All rights reserved
//
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF 
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO 
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A 
// PARTICULAR PURPOSE.
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//  Copyright © NetworkDLS 2002, All rights reserved
//
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF 
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO 
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A 
// PARTICULAR PURPOSE.
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef _DIALOGS_SPLASH_CPP
#define _DIALOGS_SPLASH_CPP
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include <Windows.H>
#include <Stdio.H>
#include <Stdlib.H>
#include <CommCtrl.h>

#ifdef _SQLESERVER
	#include "../SQLEServer/Source/Entry.H"
	#include "../SQLEServer/Dialogs/MainDlg.H"

#elif _SQLECLIENT
	#include "../SQLEClient/Source/Entry.H"
	#include "../SQLEClient/Dialogs/MainDlg.H"
#else
	#error "You must define either _SQLESERVER or _SQLECLIENT"
#endif

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

using namespace NSWFL::Windows;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

HWND hSplashProgress = NULL;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

INT_PTR CALLBACK SplashDialog(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    //--------------------------------------------------------------------------

    static HWND hVersionTag = NULL;

    static int StatusPos = 0;

    //--------------------------------------------------------------------------

    if(uMsg == WM_INITDIALOG)
    {
        StatusPos = 0;

        SendMessage(hWnd, (UINT)WM_SETTEXT, (WPARAM)0, (LPARAM)"Loading...");
        SendMessage(hWnd, WM_SETICON, TRUE, (LPARAM) LoadIcon(ghAppInstance, MAKEINTRESOURCE(IDI_MAIN)) );

        hSplashProgress = GetDlgItem(hWnd, IDC_SPLASHPROGRESS);
        hVersionTag     = GetDlgItem(hWnd, IDC_VERSIONTAG);

        //char sVersionTag[255];
        //sprintf_s(sVersionTag, sizeof(sVersionTag), "%s [Version: %s]", gsTitleCaption, gsFileVersion);
        //Set_Text(hVersionTag, gsFileVersion);
        Set_Text(hVersionTag, "");

		SetTimer(hWnd, 0, 25, NULL);

        CenterWindow(hWnd);

        return TRUE;
    }

    //--------------------------------------------------------------------------

    if(uMsg == WM_CTLCOLORSTATIC)
    {
        if((HANDLE)lParam == hVersionTag)
		{
            return Set_Color(RGB(14, 26, 105), RGB(255, 255, 255), wParam);
		}

        return FALSE;
    }

    //--------------------------------------------------------------------------

    if(uMsg == WM_TIMER)
    {
        StatusPos = (StatusPos + 2);
        SendMessage(hSplashProgress, (UINT)PBM_SETPOS, (WPARAM)StatusPos, (LPARAM)0);

        if(StatusPos >= 100)
        {
            StatusPos = 0;
            KillTimer(hWnd, 0);
            EndDialog(hWnd,0);
            DestroyWindow(hWnd);
            SetForegroundWindow(MDI.hWnd);
        }

        return TRUE;
    }

    //--------------------------------------------------------------------------

    if(uMsg == WM_COMMAND)
    {

        return FALSE;
    }

    //--------------------------------------------------------------------------

	if(uMsg == WM_PAINT)
    {
        HDC ThisHDC;
        PAINTSTRUCT ThisPS;

        ThisHDC = BeginPaint(hWnd, &ThisPS);

        // Any painting should be done here

        EndPaint(hWnd, &ThisPS);
        return TRUE;
    }

    //--------------------------------------------------------------------------

    if(uMsg == WM_CLOSE)  // Received Close Message
    {
        KillTimer(hWnd, 0);
        EndDialog(hWnd,0);
        DestroyWindow(hWnd);
        return TRUE;
    }

    //--------------------------------------------------------------------------

    return FALSE;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#endif

