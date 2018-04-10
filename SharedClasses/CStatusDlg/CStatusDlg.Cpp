///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//  Copyright © NetworkDLS 2002, All rights reserved
//
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF 
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO 
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A 
// PARTICULAR PURPOSE.
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#ifndef _CStatusDlg_CPP
#define _CStatusDlg_CPP
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include <Windows.H>
#include <Stdio.H>
#include <Stdlib.H>
#include <CommCtrl.h>

#include "CStatusDlg.H"

#ifdef _SQLESERVER
	#include "../../SQLEServer/Source/Entry.H"
#elif _SQLECLIENT
	#include "../../SQLEClient/Source/Entry.H"
#else
	#error "You must define either _SQLESERVER or _SQLECLIENT"
#endif

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

using namespace NSWFL::Windows;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

CStatusDlg::CStatusDlg()
{
	hEvent = NULL;
	hDialogHandle = NULL;
	hOwnerHandle = NULL;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

CStatusDlg::~CStatusDlg()
{
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

DWORD WINAPI CStatusDlg_Thread(LPVOID lpData)
{
	CStatusDlg *lpDlg = ((CStatusDlg *) lpData);
	
    MSG MyMsg;

	memset(&MyMsg, 0, sizeof(MyMsg));

	lpDlg->hDialogHandle = CreateDialog(
		ghAppInstance,
		MAKEINTRESOURCE(IDD_STATUSDLG),
		lpDlg->hOwnerHandle,
		CStatusDlgProc
	);

	EnableWindow(lpDlg->hOwnerHandle, FALSE);

	SetEvent(lpDlg->hEvent);

	if(lpDlg->hDialogHandle == NULL)
	{
		MessageBox(GetActiveWindow(), "Failed to load the status dialog box.", gsTitleCaption, 0);
		return 0;
	}

	while(GetMessage(&MyMsg, lpDlg->hDialogHandle, 0, 0) > 0)
    {
        TranslateMessage(&MyMsg);
        DispatchMessage(&MyMsg);
    }

	EnableWindow(lpDlg->hOwnerHandle, TRUE);

	SendMessage(lpDlg->hDialogHandle,  WM_QUIT, 0, 0);

	lpDlg->hDialogHandle = NULL;

	return 0;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

HWND CStatusDlg::Hwnd(void)
{
	return this->hDialogHandle;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool CStatusDlg::SetButtonText(char *sText)
{
	Set_Text(hCmdCancel_hWnd, sText);
	return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool CStatusDlg::EnableButton(void)
{
	EnableWindow(hCmdCancel_hWnd, true);
	return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool CStatusDlg::DisableButton(void)
{
	EnableWindow(hCmdCancel_hWnd, false);
	return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool CStatusDlg::SetProgressPos(long lPos)
{
	SendMessage(hProgress_hWnd, (UINT) PBM_SETPOS, lPos, 0);
	return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool CStatusDlg::SetProgressRange(long lMin, long lMax)
{
	SendMessage(hProgress_hWnd, (UINT) PBM_SETRANGE, 0, MAKELPARAM(lMin, lMax));
	return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool CStatusDlg::SetText(char *sText)
{
	Set_Text(hText_hWnd, sText);
	return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool CStatusDlg::Unload(void)
{
	if(hDialogHandle)
	{
		SendMessage(hDialogHandle, WM_COMMAND, IDC_CANCEL, NULL);
	}

	while(IsWindow(hDialogHandle))
	{
		Sleep(1);
	}

	return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

HWND CStatusDlg::Load(HWND hOwner, char *sTitle, int iIcon, bool bAllowCancel)
{
	hOwnerHandle = hOwner;
	
	hEvent = CreateEvent(NULL, TRUE, FALSE, "CStatusDlg_Thread_Function");

	CStatusDlg_Thread_Handle =
		CreateThread(NULL, 0, CStatusDlg_Thread, (LPVOID)this, 0, &CStatusDlg_Thread_ID);

    if(WaitForSingleObject(hEvent, 5000) == WAIT_TIMEOUT)
    {
        CloseHandle(hEvent);
        return NULL;
    }

    SendMessage(hDialogHandle, (UINT)WM_SETTEXT, (WPARAM)0, (LPARAM)sTitle);
    SendMessage(hDialogHandle, WM_SETICON, TRUE, (LPARAM) LoadIcon(ghAppInstance, MAKEINTRESOURCE(iIcon)));

	hText_hWnd      = GetDlgItem(hDialogHandle, IDC_STATTEXT);
	hProgress_hWnd  = GetDlgItem(hDialogHandle, IDC_STATPROGRESS);
	hCmdCancel_hWnd = GetDlgItem(hDialogHandle, IDC_CANCEL);

	if(!bAllowCancel)
	{
		EnableWindow(hCmdCancel_hWnd, false);
	}

	CloseHandle(hEvent);

	return hDialogHandle;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool CStatusDlg::SetIcon(int iIcon)
{
	SendMessage(hDialogHandle, WM_SETICON, TRUE, (LPARAM) LoadIcon(ghAppInstance, MAKEINTRESOURCE(iIcon)));
	return true;
}


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool CStatusDlg::SetCaption(char *sCaption)
{
	SendMessage(hDialogHandle, (UINT)WM_SETTEXT, (WPARAM)0, (LPARAM)sCaption);
	return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool CStatusDlg::Show(void)
{
	ShowWindow(hDialogHandle, SW_SHOW);

	SetForegroundWindow(hDialogHandle);

	return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool CStatusDlg::IsDlg(void)
{
	return IsWindow(hDialogHandle) == TRUE;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool CStatusDlg::IsDlgVisible(void)
{
	if(IsWindowVisible(hDialogHandle))
	{
		return true;
	}
	return false;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool CStatusDlg::Hide(void)
{
	if(ShowWindow(hDialogHandle, SW_HIDE))
	{
		return true;
	}
	return false;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

INT_PTR CALLBACK CStatusDlgProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    if(uMsg == WM_INITDIALOG)
    {
		CenterOverOwner(hWnd);
        return TRUE;
    }

    if(uMsg == WM_COMMAND)
    {
		if(wParam == IDC_CANCEL)
		{
			PostMessage(hWnd, WM_QUIT, 0, 0);
			return TRUE;
		}

		return FALSE;
    }

	if(uMsg == WM_PAINT)
    {
        HDC ThisHDC;
        PAINTSTRUCT ThisPS;

        ThisHDC = BeginPaint(hWnd, &ThisPS);

        // Any painting should be done here

        EndPaint(hWnd, &ThisPS);
        return TRUE;
    }

    return FALSE;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#endif

