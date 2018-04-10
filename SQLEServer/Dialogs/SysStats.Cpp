///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//  Copyright © NetworkDLS 2002, All rights reserved
//
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF 
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO 
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A 
// PARTICULAR PURPOSE.
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef _SYSSTATS_CPP
#define _SYSSTATS_CPP
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include <Windows.H>
#include <Stdio.H>
#include <Stdlib.H>
#include <CommCtrl.h>
#include <Psapi.H>

#include "../Resources/Resource.H"

#include "../../../@Libraries/CSocketServer/CSocketServer.H"

#include "../../SharedSource/Common.H"
#include "../Source/Entry.H"
#include "../Source/Routines.H"
#include "../Source/NewDB.H"

#include "../Dialogs/MainDlg.H"
#include "../Dialogs/SysStats.H"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

using namespace NSWFL::Windows;
using namespace NSWFL::Conversion;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//---------------------(Variable Declarations)
HWND hSysStats = NULL;
HANDLE SysStats_Thread_Handle;
DWORD SysStats_Thread_ID;
bool bRunSysStatThread = false;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

DWORD WINAPI SysStats_Thread(LPVOID lpData)
{
	char sText[255];
	PROCESS_MEMORY_COUNTERS MyPMC;

	BOOL bResult = FALSE;

	HANDLE hProcess = GetCurrentProcess();

	HWND hPageFaults         = NULL;
	HWND hMemoryUsed         = NULL;
	HWND hPagedPoolUsage     = NULL;
	HWND hNPPoolUsage        = NULL;
	HWND hPageFileUsage      = NULL;
	HWND hPeakMemUsage       = NULL;
	HWND hPeakPagedPoolUsage = NULL;
	HWND hPeakNPPoolUsage    = NULL;
	HWND hPeakPageFileUsage  = NULL;

	memset(&MyPMC, 0, sizeof(MyPMC));

	hPageFaults         = GetDlgItem(hSysStats, ID_PAGEFAULTS);
	hMemoryUsed         = GetDlgItem(hSysStats, ID_MEMORYUSED);
	hPagedPoolUsage     = GetDlgItem(hSysStats, ID_PAGEDPOOLUSAGE);
	hNPPoolUsage        = GetDlgItem(hSysStats, ID_NPPOOLUSAGE);
	hPageFileUsage      = GetDlgItem(hSysStats, ID_PAGEFILEUSAGE);
	hPeakMemUsage       = GetDlgItem(hSysStats, ID_PEAKMEMUSAGE);
	hPeakPagedPoolUsage = GetDlgItem(hSysStats, ID_PEAKPAGEDPOOLUSAGE);
	hPeakNPPoolUsage    = GetDlgItem(hSysStats, ID_PEAKNPPOOLUSAGE);
	hPeakPageFileUsage  = GetDlgItem(hSysStats, ID_PEAKPAGEFILEUSAGE);

	while(bRunSysStatThread)
	{
        bResult = GetProcessMemoryInfo(hProcess, &MyPMC, sizeof(MyPMC));

		FormatInteger(sText, sizeof(sText), MyPMC.PageFaultCount);
		Set_Text(hPageFaults, sText);

		I64FormatInteger(sText, sizeof(sText), MyPMC.WorkingSetSize / 1024);
		strcat_s(sText, sizeof(sText), " KB");
		Set_Text(hMemoryUsed, sText);

		I64FormatInteger(sText, sizeof(sText), MyPMC.QuotaPagedPoolUsage / 1024);
		strcat_s(sText, sizeof(sText), " KB");
		Set_Text(hPagedPoolUsage, sText);

		I64FormatInteger(sText, sizeof(sText), MyPMC.QuotaPeakNonPagedPoolUsage / 1024);
		strcat_s(sText, sizeof(sText), " KB");
		Set_Text(hNPPoolUsage, sText);

		I64FormatInteger(sText, sizeof(sText), MyPMC.PagefileUsage / 1024);
		strcat_s(sText, sizeof(sText), " KB");
		Set_Text(hPageFileUsage, sText);

		I64FormatInteger(sText, sizeof(sText), MyPMC.PeakWorkingSetSize / 1024);
		strcat_s(sText, sizeof(sText), " KB");
		Set_Text(hPeakMemUsage, sText);

		I64FormatInteger(sText, sizeof(sText), MyPMC.QuotaPeakPagedPoolUsage / 1024);
		strcat_s(sText, sizeof(sText), " KB");
		Set_Text(hPeakPagedPoolUsage, sText);

		I64FormatInteger(sText, sizeof(sText), MyPMC.QuotaPeakNonPagedPoolUsage / 1024);
		strcat_s(sText, sizeof(sText), " KB");
		Set_Text(hPeakNPPoolUsage, sText);

		I64FormatInteger(sText, sizeof(sText), MyPMC.PeakPagefileUsage / 1024);
		strcat_s(sText, sizeof(sText), " KB");
		Set_Text(hPeakPageFileUsage, sText);

		Sleep(250);
	}

	//CloseHandle() IS NOT necessary because GetCurrentProcess()
	//	retrieves a pseudo handle for the current process.
	//CloseHandle(hProcess); 

	return 0;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

INT_PTR CALLBACK SysStatsDialog(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    //--------------------------------------------------------------------------

    if(uMsg == WM_INITDIALOG)
    {
        hSysStats = hWnd;

        char sCaption[1024];
		sprintf_s(sCaption, sizeof(sCaption), "%s :: %s", gsTitleCaption, "System Statistics");
		SendMessage(hWnd, (UINT)WM_SETTEXT, (WPARAM)0, (LPARAM)sCaption);
        SendMessage(hWnd, WM_SETICON, TRUE, (LPARAM) LoadIcon(ghAppInstance, MAKEINTRESOURCE(IDI_MAIN)) );

		bRunSysStatThread = true;
		SysStats_Thread_Handle = CreateThread(NULL, 0, SysStats_Thread, (LPVOID)0, 0, &SysStats_Thread_ID);

		CenterOverOwner(hWnd);

        return TRUE;
    }

    //--------------------------------------------------------------------------
/*
    if(uMsg == WM_COMMAND)
    {
        if(wParam == IDC_CANCEL)
        {
			EndDialog(hWnd,0);
			DestroyWindow(hWnd);
			return TRUE;
        }
		
		return FALSE;
    }
*/
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

    if(uMsg == WM_CLOSE) 
    {
        bRunSysStatThread = false;
		Sleep(500);
		EndDialog(hWnd,0);
        DestroyWindow(hWnd);
        return TRUE;
    }

    //--------------------------------------------------------------------------

    return FALSE;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#endif

