///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//  Copyright © NetworkDLS 2002, All rights reserved
//
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF 
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO 
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A 
// PARTICULAR PURPOSE.
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef _CustomProcs_Cpp
#define _CustomProcs_Cpp
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#define _WIN32_WINNT 0x0500

#include <Windows.H>
#include <Stdio.H>
#include <Stdlib.H>

#ifdef _SQLESERVER
	#include "../SQLEServer/Source/Entry.H"

#elif _SQLECLIENT
	#include "../SQLEClient/Source/Entry.H"
#else
	#error "You must define either _SQLESERVER or _SQLECLIENT"
#endif

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

using namespace NSWFL::Windows;

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#define INITHANDLERPROC (WM_USER + 1000) //Can be used to init all the custom procs.

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

typedef struct TAG_PWDWeightHandler{
	WNDPROC lpOldProc;
	HWND hPwdWeight;
}INITPWDWEIGHTHANDLERINFO, *LPINITPWDWEIGHTHANDLERINFO;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

LRESULT CALLBACK PWDWeightHandlerProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	static WNDPROC lpOldProc;
	static HWND hOwner;
	static INITPWDWEIGHTHANDLERINFO IPWHI;

    if(uMsg == WM_GETDLGCODE)
	{
		return (DLGC_WANTCHARS | CallWindowProc(IPWHI.lpOldProc, hWnd, uMsg, wParam, lParam));
	}
	else if(uMsg == INITHANDLERPROC)
	{
		memcpy(&IPWHI, (WNDPROC *)lParam, sizeof(INITPWDWEIGHTHANDLERINFO));
		memcpy(&hOwner, (WNDPROC *)wParam, sizeof(HWND));
	}
	else if(uMsg == WM_CHAR)
	{
		if(IPWHI.lpOldProc)
		{
			CNASCCL MyCode;
			char sPwd[1024 + 1];
			int iPwdLen = Get_Text(hWnd, sPwd, sizeof(sPwd));
			if(iPwdLen < sizeof(sPwd))
			{
				Set_Text(IPWHI.hPwdWeight, MyCode.KeyWeightString(sPwd, iPwdLen));
			}
		}
	}

	if(IPWHI.lpOldProc)
	{
		return CallWindowProc(IPWHI.lpOldProc, hWnd, uMsg, wParam, lParam);
	}
	else{
		return 0;
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void AddPWDWeightHandler(HWND hOwner, HWND hWnd, HWND hPwdWeight)
{
	INITPWDWEIGHTHANDLERINFO IPWHI;

	IPWHI.lpOldProc = (WNDPROC) SetWindowLongPtr(hWnd, GWLP_WNDPROC, (INT_PTR) &PWDWeightHandlerProc);
	IPWHI.hPwdWeight = hPwdWeight;

	SendMessage(hWnd, INITHANDLERPROC, (WPARAM)&hOwner, (LPARAM) &IPWHI);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#endif
