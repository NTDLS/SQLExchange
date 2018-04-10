///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//  Copyright © NetworkDLS 2002, All rights reserved
//
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF 
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO 
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A 
// PARTICULAR PURPOSE.
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef _MAINDLG_CPP
#define _MAINDLG_CPP
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include <Windows.H>
#include <Stdio.H>
#include <Stdlib.H>
#include <CommCtrl.H>
#include <PSAPI.H>

#include "../Source/Entry.H"
#include "../Source/Console.H"
#include "../Source/NewDB.H"

#include "../Dialogs/MainDlg.H"
#include "../Dialogs/SysStats.H"
#include "../Dialogs/CreateNewDB.H"

#include "../../SharedSource/SetNewGUIPwd.H"
#include "../../SharedSource/AskPwd.H"
#include "../../SharedSource/SplashDlg.H"
#include "../../SharedSource/NetLogoDlg.H"

#include "../../../@Libraries/CSocketServer/CSocketServer.H"
#include "../../../@Libraries/CGraph/CGraph.H"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

using namespace NSWFL::Windows;
using namespace NSWFL::ListBox;
using namespace NSWFL::Conversion;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

INT_PTR CALLBACK MainDialog(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	//--------------------------------------------------------------------------

    static HWND hStaticFrame = NULL;
    static HWND hConnections = NULL;
    static HWND hTotalConnects = NULL;
    static HWND hWarnings = NULL;
    static HWND hErrors = NULL;
    static HWND hGraph = NULL;

	static HMENU SystemMenu = NULL;

	static int iLastWidth = 0;
	static int iLastHeight = 0;

	static CGraph MyGraph;

	//--------------------------------------------------------------------------

    if(uMsg == WM_INITDIALOG)
    {
		if(strlen(gsGUIPWDHash) > 0)
		{
			if(DialogBox(ghAppInstance, MAKEINTRESOURCE(IDD_ASKPWD), hWnd, (DLGPROC)AskPwdDialog) != 100)
			{
				EndDialog(hWnd, 0);
				return TRUE;
			}
		}

        MDI.hWnd = hWnd;

        SystemMenu = GetSystemMenu(hWnd, FALSE);
        AppendMenu(SystemMenu, MF_SEPARATOR, 0, 0);
        AppendMenu(SystemMenu, MF_STRING, 1000, "About");

        SendMessage(hWnd, (UINT)WM_SETTEXT, (WPARAM)0, (LPARAM)gsTitleCaption);
        SendMessage(hWnd, WM_SETICON, TRUE, (LPARAM) LoadIcon(ghAppInstance, MAKEINTRESOURCE(IDI_MAIN)) );

        MDI.hMenu = LoadMenu(ghAppInstance, MAKEINTRESOURCE(IDR_SERVERDLGMENU));
		MDI.hLogList = GetDlgItem(hWnd, IDC_LOGLIST);
		MDI.hStatusBar = CreateStatusWindow(WS_CHILD|WS_VISIBLE, "Transactions: 0/s", hWnd, 1);
		SendMessage(MDI.hLogList, LB_SETHORIZONTALEXTENT, 2048, 0);

        int Widths[] = {200, -1};
        SendMessage(MDI.hStatusBar, (UINT)SB_SETPARTS, (WPARAM)2, (LPARAM)&Widths);

		SetMenu(hWnd, MDI.hMenu);

		hGraph		   = GetDlgItem(hWnd, IDC_USAGEGRAPH);
		hStaticFrame   = GetDlgItem(hWnd, IDC_STATICFRAME);
        hConnections   = GetDlgItem(hWnd, IDC_CONNECTIONS);
        hTotalConnects = GetDlgItem(hWnd, IDC_TOTALCONNECTS);
        hWarnings      = GetDlgItem(hWnd, IDC_WARNINGS);
        hErrors        = GetDlgItem(hWnd, IDC_ERRORS);

		if(gbShowSplashScreen)
		{
			DialogBox(ghAppInstance, MAKEINTRESOURCE(IDD_SPLASH), hWnd, (DLGPROC)SplashDialog);
		}

		Set_Long(hConnections, gServer.CurrentConnections());
        Set_Long(hTotalConnects, gdwTotalConnects);
        Set_Long(hWarnings, gdwWarningCount);
        Set_Long(hErrors, gdwErrorCount);

		EnterCriticalSection(&csTransPerSecond);
		giTransProcessed = 0;
		LeaveCriticalSection(&csTransPerSecond);

		CenterOverOwner(hWnd);

		RECT rcWindow;
		memset(&rcWindow, 0, sizeof(rcWindow));
        GetClientRect(hWnd, &rcWindow);
		SendMessage(hWnd, WM_SIZE, 0, MAKELONG(rcWindow.right, rcWindow.bottom));

		MyGraph.Initialize(hGraph, RGB(210, 210, 210), RGB(150, 200, 150), 10, GRAPH_RIGHT, true);
		MyGraph.AddSeries(RGB(70, 70, 225), gServer.MaxConnections());
		MyGraph.AddSeries(RGB(225, 70, 70), 100);

		if(!IsServerConfigured(true))
		{
		}

		if(OnlyGoodForOneYear())
		{
			WriteSysLogEx("This version of the SQL-Exchange has expired. Please obtain a newer version.", EVENT_ERROR);
		}

		SetTimer(hWnd, 0, 500, NULL);
		SetTimer(hWnd, 1, 100, NULL);

		return TRUE; // Return TRUE to set the keyboard focus, Otherwise return FALSE.
    }

    //--------------------------------------------------------------------------

    if(uMsg == WM_COMMAND)
    {
		if(wParam == ID_SETTINGS_CLEARSTATISTICS)
        {
			gdwWarningCount = 0;
			gdwErrorCount = 0;
            return TRUE;
        }

		if(wParam == ID_SETTINGS_SETGUIPWD)
        {
			DialogBox(ghAppInstance, MAKEINTRESOURCE(IDD_SETGUIPWD), hWnd, (DLGPROC)SetGUIPwdDialog);
            return TRUE;
        }

		if(wParam == ID_REPLICATION_SETUPNEWDATABASE)
        {
			DialogBox(ghAppInstance, MAKEINTRESOURCE(IDD_CREATENEWDB), hWnd, (DLGPROC)CreateNewDBDialog);
            return TRUE;
        }

		if(wParam == ID_SERVER_SYSTEMSTATISTICS)
        {
			DialogBox(ghAppInstance, MAKEINTRESOURCE(IDD_SYSSTATS), hWnd, (DLGPROC)SysStatsDialog);
            return TRUE;
        }

		if(wParam == ID_SETTINGS_OPENCONSOLE)
        {
            if(gbConsoleOpen)
                CloseConsole();
            else OpenConsole();
            return TRUE;
        }

        if(wParam == ID_HELP_ABOUT)
        {
            _AboutDialogInfo ADI;

            ADI.DisplayIcon  = LoadIcon(ghAppInstance, MAKEINTRESOURCE(IDI_MAIN));
            ADI.TitleCaption = gsTitleCaption;
            ADI.FileVersion  = gsFileVersion;
            ADI.BuildDate    = __DATE__;
            ADI.BuildTime    = __TIME__;
            ADI.CopyRight    = gsAppCopyRight;
            ADI.OwnerHandle  = hWnd;

            NetLogo(&ADI);
            return true;
        }

        if(wParam == ID_SETTINGS_DEBUGMODE)
        {
            if(gbDebugMode)
            {
                CheckMenuItem(MDI.hMenu, ID_SETTINGS_DEBUGMODE, MF_BYCOMMAND|MF_UNCHECKED);
                gbDebugMode = false;
            }
            else{
                CheckMenuItem(MDI.hMenu, ID_SETTINGS_DEBUGMODE, MF_BYCOMMAND|MF_CHECKED);
                gbDebugMode = true;
            }
            return TRUE;
        }

        if(wParam == ID_SERVER_START)
        {
            StartServer();
			if(gServer.IsActive())
			{
				EnableMenuItem(MDI.hMenu, ID_SERVER_STOP, MF_BYCOMMAND|MF_ENABLED);
				EnableMenuItem(MDI.hMenu, ID_SERVER_START, MF_BYCOMMAND|MF_GRAYED|MF_DISABLED);
			}
			else{
				EnableMenuItem(MDI.hMenu, ID_SERVER_STOP, MF_BYCOMMAND|MF_GRAYED|MF_DISABLED);
				EnableMenuItem(MDI.hMenu, ID_SERVER_START, MF_BYCOMMAND|MF_ENABLED);
			}
            return TRUE;
        }

        if(wParam == ID_SERVER_STOP)
        {
            StopServer();
			if(gServer.IsActive())
			{
				EnableMenuItem(MDI.hMenu, ID_SERVER_STOP, MF_BYCOMMAND|MF_ENABLED);
				EnableMenuItem(MDI.hMenu, ID_SERVER_START, MF_BYCOMMAND|MF_GRAYED|MF_DISABLED);
			}
			else{
				EnableMenuItem(MDI.hMenu, ID_SERVER_STOP, MF_BYCOMMAND|MF_GRAYED|MF_DISABLED);
				EnableMenuItem(MDI.hMenu, ID_SERVER_START, MF_BYCOMMAND|MF_ENABLED);
			}
            return TRUE;
        }

		if(wParam == ID_SERVER_CLOSE)
        {
			KillTimer(hWnd, 0);
			MyGraph.Destroy();
            EndDialog(hWnd,0);
			MDI.hWnd = NULL;
            return TRUE;
        }

        if(wParam == ID_SETTINGS_SETTINGS)
        {
			LaunchConfigDlg(hWnd);
            return TRUE;
        }

        return FALSE;
    }

	//--------------------------------------------------------------------------

	if(uMsg == WM_TIMER)
    {
		if(wParam == 0)
		{
			char sText[255];
			char sNumber1[64];
			double iGraphValues[3];
			int iConnects = gServer.CurrentConnections();

			Set_Long(hConnections, iConnects);
			Set_Long(hTotalConnects, gdwTotalConnects);
			Set_Long(hWarnings, gdwWarningCount);
			Set_Long(hErrors, gdwErrorCount);

			PROCESS_MEMORY_COUNTERS PMC;
			memset(&PMC, 0, sizeof(PMC));
			if(GetProcessMemoryInfo(GetCurrentProcess(), &PMC, sizeof(PMC)))
			{
				iGraphValues[0] = iConnects;
				iGraphValues[1] =
					(int)(((((double)PMC.WorkingSetSize / (double)PMC.PeakWorkingSetSize) * 100.0) +
					(((double)PMC.PagefileUsage / (double)PMC.PeakPagefileUsage) * 100.0)) / 2.0);
				MyGraph.Update(iGraphValues);
			}
			MyGraph.Invalidate();

			static DWORD dwLastTime = 0;

			EnterCriticalSection(&csTransPerSecond);
			DWORD dwThisTime = GetTickCount();
			int iTx = giTransProcessed;
			giTransProcessed = 0;
			LeaveCriticalSection(&csTransPerSecond);

			if(dwThisTime != dwLastTime)
			{
				sprintf_s(sText, sizeof(sText), "Transactions: %s/s",
					FormatDouble(sNumber1, sizeof(sNumber1),
					iTx * (1000.0 / ((double)dwThisTime - (double)dwLastTime)), 0));
				SendMessage(MDI.hStatusBar, (UINT)SB_SETTEXT, (WPARAM)0, (LPARAM) sText);
			}

			dwLastTime = GetTickCount();

			if(gbDebugMode)
				CheckMenuItem(MDI.hMenu, ID_SETTINGS_DEBUGMODE, MF_BYCOMMAND|MF_CHECKED);
			else CheckMenuItem(MDI.hMenu, ID_SETTINGS_DEBUGMODE, MF_BYCOMMAND|MF_UNCHECKED);

			if(gServer.IsActive())
			{
				EnableMenuItem(MDI.hMenu, ID_SERVER_STOP, MF_BYCOMMAND|MF_ENABLED);
				EnableMenuItem(MDI.hMenu, ID_SERVER_START, MF_BYCOMMAND|MF_GRAYED|MF_DISABLED);
			}
			else{
				EnableMenuItem(MDI.hMenu, ID_SERVER_STOP, MF_BYCOMMAND|MF_GRAYED|MF_DISABLED);
				EnableMenuItem(MDI.hMenu, ID_SERVER_START, MF_BYCOMMAND|MF_ENABLED);
			}

			#if _MEMPOOL_DEBUG_LEVEL >= 1
			char sNumber2[64];
			char sNumber3[64];
			sprintf_s(sText, sizeof(sText),
				"Allocations: %s/%s (%s)",
				FormatInteger(sNumber1, sizeof(sNumber1), pMem->GetMemAllocationCount()),
				FormatInteger(sNumber2, sizeof(sNumber2), pMem->GetSlotAllocationCount()),
				FileSizeFriendly(pMem->ReservedBytes(), 2, sNumber3, sizeof(sNumber3))
			);

			SendMessage(MDI.hStatusBar, (UINT)SB_SETTEXT, (WPARAM)1, (LPARAM) sText);
			#endif
		}
		else if(wParam == 1)
		{
			EnterCriticalSection(&csLogFile);
			char *sBuffer = NULL;
			while((sBuffer = (char *) MDI.Queu.Peek()))
			{
				LeaveCriticalSection(&csLogFile);

				InsertListBoxItem(MDI.hLogList, sBuffer, 100, -1);

				EnterCriticalSection(&csLogFile);
				MDI.Queu.Pop();
			}
			LeaveCriticalSection(&csLogFile);
		}

		return true;
    }

	//--------------------------------------------------------------------------

	if(uMsg == WM_SIZE)
    {
		int nWidth = LOWORD(lParam);
		int nHeight = HIWORD(lParam);

		if((iLastWidth != nWidth || iLastHeight != nHeight) && nHeight > 0 && nWidth > 0)
		{
			MoveWindow(MDI.hLogList, 0, 101, nWidth, (nHeight-20) - 101, TRUE);
			MoveWindow(MDI.hStatusBar, nWidth-20, nHeight-20, nWidth, nHeight, TRUE);
			MoveWindow(hStaticFrame, 0, 0, nWidth, 100, TRUE);
			MoveWindow(hGraph, 195, 10, nWidth-200, 84, TRUE);

			MyGraph.Reset();
			iLastWidth = nWidth;
			iLastHeight = nHeight;
		}

		return FALSE;
    }

	//--------------------------------------------------------------------------

	else if(uMsg == WM_GETMINMAXINFO)
	{
		MINMAXINFO *pMMI = (MINMAXINFO *)lParam;
		pMMI->ptMinTrackSize.x = 500;
		pMMI->ptMinTrackSize.y = 300;
		return 0;
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

    if(uMsg == WM_SYSCOMMAND) // Received A System Menu Message
    {
        if(LOWORD(wParam) == 1000) //- About (SysMenu Command)
        {
            _AboutDialogInfo ADI;
    
            ADI.DisplayIcon  = LoadIcon(ghAppInstance, MAKEINTRESOURCE(IDI_MAIN));
            ADI.TitleCaption = gsTitleCaption;
            ADI.FileVersion  = gsFileVersion;
            ADI.BuildDate    = __DATE__;
            ADI.BuildTime    = __TIME__;
            ADI.CopyRight    = gsAppCopyRight;
            ADI.OwnerHandle  = hWnd;

            NetLogo(&ADI);
            return TRUE;
        }

        return FALSE;
    }

    //--------------------------------------------------------------------------

    if(uMsg == WM_CLOSE)  // Received Close Message
    {
	    KillTimer(hWnd, 0);
		MyGraph.Destroy();
		EndDialog(hWnd, 0);
		MDI.hWnd = NULL;
        return TRUE;
    }

    //--------------------------------------------------------------------------

    return FALSE;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#endif
