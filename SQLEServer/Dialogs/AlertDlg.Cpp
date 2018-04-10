///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//  Copyright © NetworkDLS 2002, All rights reserved
//
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF 
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO 
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A 
// PARTICULAR PURPOSE.
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef _AlertDlg_CPP
#define _AlertDlg_CPP
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include <Windows.H>
#include <Stdio.H>
#include <Stdlib.H>
#include <CommCtrl.H>

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "../Source/Entry.H"

#include "AlertDlg.H"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

using namespace NSWFL::Windows;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

HANDLE hAlertDlgThread = NULL;
DWORD dwAlertDlgThread = 0;

HWND hCaption = NULL;
HWND hMessage = NULL;
HWND hButton1 = NULL;
HWND hButton2 = NULL;
HWND hAlertDialog = NULL;

bool bTempDlg = false;
bool bStartMoving = false;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void ResetAlertDlgVars(void)
{
	dwAlertDlgThread = 0;

	bTempDlg = false;
	bStartMoving = false;

	hCaption = NULL;
	hMessage = NULL;
	hButton1 = NULL;
	hButton2 = NULL;
	hAlertDialog = NULL;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

DWORD WINAPI AlertDlgThread(LPVOID lpVoid)
{
	DialogBox(ghAppInstance, MAKEINTRESOURCE(IDD_ALERT), 0, (DLGPROC)AlertDialog);

	ResetAlertDlgVars();

	return 0;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool StartAlertDlg(const char *sCaption, const char *sMessage, bool bTemporary)
{
	char sTemp[255];

	if(hAlertDlgThread)
	{
		CloseHandle(hAlertDlgThread);
		hAlertDlgThread = NULL;
	}

	if(dwAlertDlgThread > 0)
	{
		return false;
	}

	ResetAlertDlgVars();

	bTempDlg = bTemporary;

	hAlertDlgThread = CreateThread(NULL, 0, AlertDlgThread, (LPVOID)0, 0, &dwAlertDlgThread);

	while(!hAlertDialog)
	{
		Sleep(1);
	}

    sprintf_s(sTemp, sizeof(sTemp), "%s - %s", gsTitleCaption, sCaption);
	Set_Text(hCaption, sTemp);
	Set_Text(hMessage, (char *)sMessage);

	bStartMoving = true;

	return 0;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

INT_PTR CALLBACK AlertDialog(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    //--------------------------------------------------------------------------

    static HWND hTimerCt = NULL;

	static int iMoveTo = 0;
	static int iOrigBottom = 0;
	static int iBottom = 0;
	static int iLeft = 0;
	static int iHideDelay = 0;

    //--------------------------------------------------------------------------

    if(uMsg == WM_INITDIALOG) // Received A Initilize Dialog Message
    {
		iHideDelay = 10;

		hCaption = GetDlgItem(hWnd, IDC_ALERTCAPTION);
		hMessage = GetDlgItem(hWnd, IDC_ALERTMESSAGE);
		hButton1 = GetDlgItem(hWnd, IDC_ALERTBUTTON1);
		hButton2 = GetDlgItem(hWnd, IDC_ALERTBUTTON2);
		hTimerCt = GetDlgItem(hWnd, IDC_ALERTTIMER);

		SetTimer(hWnd, 0, 1, NULL);

		SetWindowPos(hWnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOSIZE);

		RECT rcDesktop;
		RECT rcAlertDlg;
		memset(&rcDesktop, 0, sizeof(RECT));
		memset(&rcAlertDlg, 0, sizeof(RECT));

		GetWindowRect(hWnd, &rcAlertDlg);

		SystemParametersInfo(SPI_GETWORKAREA, 0, &rcDesktop, 0);

		iBottom = GetSystemMetrics(SM_CYSCREEN) + 5;
		iMoveTo = (rcDesktop.bottom - rcAlertDlg.bottom) - 1;
		iLeft = (rcDesktop.right - rcAlertDlg.right) - 1;
		iOrigBottom = iBottom;

		SetWindowPos(hWnd, HWND_TOPMOST, iLeft, iBottom, 0, 0, SWP_NOSIZE);

		if(!bTempDlg)
		{
			ShowWindow(hTimerCt, SW_HIDE);
		}
		
		hAlertDialog = hWnd;

		return TRUE; // Return TRUE to set the keyboard focus, Otherwise return FALSE
    }

    //--------------------------------------------------------------------------

    if(uMsg == WM_CTLCOLORSTATIC)
    {
		if((HANDLE)lParam == hTimerCt)
		{
            return Set_Color(RGB(255, 200, 200), RGB(0, 102, 204), wParam);
		}

		if((HANDLE)lParam == hCaption)
		{
            return Set_Color(RGB(255, 255, 255), RGB(0, 102, 204), wParam);
		}

		if((HANDLE)lParam == hMessage)
		{
            return Set_Color(RGB(0, 0, 0), RGB(255, 255, 255), wParam);
		}

        return FALSE;
    }

    //--------------------------------------------------------------------------

    if(uMsg == WM_TIMER)
    {
		if(wParam == 0 && bStartMoving)
		{
			if(iBottom > iMoveTo)
			{
				int iStep = 10;
				
				if((iBottom - iMoveTo) < iStep)
				{
					iBottom = iMoveTo;
					KillTimer(hWnd, 0);

					if(bTempDlg)
					{
						SetTimer(hWnd, 1, 1000, NULL);
					}
				}
				else iBottom = (iBottom - iStep);

				SetWindowPos(hWnd, HWND_TOPMOST, iLeft, iBottom, 0, 0, SWP_NOSIZE);
			}
		}

		if(wParam == 1)
		{
			iHideDelay--;

			if(iHideDelay > 0)
			{
				Set_Long(hTimerCt, iHideDelay);
			}
			else{
				ShowWindow(hTimerCt, SW_HIDE);
			}

			if(iHideDelay < 0)
			{
				iMoveTo = iOrigBottom;

				KillTimer(hWnd, 1);

				SetTimer(hWnd, 2, 10, NULL);
			}
		}

		if(wParam == 2)
		{
			if(iBottom < iMoveTo)
			{
				iBottom = (iBottom + 10);

				SetWindowPos(hWnd, HWND_TOPMOST, iLeft, iBottom, 0, 0, SWP_NOSIZE);

				if(iBottom > iMoveTo)
				{
					KillTimer(hWnd, 2);
					EndDialog(hWnd, 0);
				}
			}
		}

		return TRUE;
    }

    //--------------------------------------------------------------------------

    if(uMsg == WM_COMMAND)
    {
		if(wParam == IDC_ALERTBUTTON1)
		{
			iMoveTo = iOrigBottom;

			KillTimer(hWnd, 1);

			SetTimer(hWnd, 2, 10, NULL);
			return TRUE;
		}

		if(wParam == IDC_ALERTBUTTON2)
		{
            EndDialog(hWnd, 0);
			return TRUE;
		}
		
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

