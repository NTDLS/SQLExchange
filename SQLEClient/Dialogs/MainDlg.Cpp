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
#include <PrSht.H>

#include "../Source/Entry.H"
#include "../Source/Replication.H"

#include "../Dialogs/MainDlg.H"
#include "../Dialogs/ReplicationDlg.H"
#include "../Dialogs/PendingTrans.H"

#include "../../SharedSource/AskPwd.H"
#include "../../SharedSource/SetNewGUIPwd.H"
#include "../../SharedSource/NetLogoDlg.H"
#include "../../SharedSource/SplashDlg.H"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

using namespace NSWFL::Windows;
using namespace NSWFL::Conversion;
using namespace NSWFL::ListBox;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

BOOL CALLBACK MainDialog(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    //--------------------------------------------------------------------------

    static HWND hStaticFrame = NULL;
    static HWND hTotalConnects = NULL;
    static HWND hWarnings = NULL;
    static HWND hErrors = NULL;

	static HMENU hSystemMenu = NULL;

	static int iLastWidth = 0;
	static int iLastHeight = 0;

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

		hSystemMenu = GetSystemMenu(hWnd, FALSE);
        AppendMenu(hSystemMenu, MF_SEPARATOR, 0, 0);
        AppendMenu(hSystemMenu, MF_STRING, 1000, "About");

        SendMessage(hWnd, (UINT)WM_SETTEXT, (WPARAM)0, (LPARAM)gsTitleCaption);
        SendMessage(hWnd, WM_SETICON, TRUE, (LPARAM) LoadIcon(ghAppInstance, MAKEINTRESOURCE(IDI_MAIN)) );

        MDI.hMenu      = LoadMenu(ghAppInstance, MAKEINTRESOURCE(IDR_MAINMENU));
		MDI.hStatusBar = CreateStatusWindow(WS_CHILD|WS_VISIBLE, "Transactions: 0/s", hWnd, 1);
        MDI.hLogList   = GetDlgItem(hWnd, IDC_LOGLIST);
		hStaticFrame   = GetDlgItem(hWnd, IDC_STATICFRAME);
        hTotalConnects = GetDlgItem(hWnd, IDC_TOTALCONNECTS);
        hWarnings      = GetDlgItem(hWnd, IDC_WARNINGS);
        hErrors        = GetDlgItem(hWnd, IDC_ERRORS);
		SendMessage(MDI.hLogList, LB_SETHORIZONTALEXTENT, 2048, 0);

        int Widths[] = {200, -1};
        SendMessage(MDI.hStatusBar, (UINT)SB_SETPARTS, (WPARAM)2, (LPARAM)&Widths);
		SetMenu(hWnd, MDI.hMenu);

		RECT WindowRECT;
		memset(&WindowRECT, 0, sizeof(RECT));
        GetClientRect(hWnd, &WindowRECT);
        MoveWindow(MDI.hLogList, 0, 86, WindowRECT.right, ((WindowRECT.bottom-20) - 86), TRUE);
        MoveWindow(hStaticFrame, 0, 0, WindowRECT.right, 85, TRUE);

		Set_Long(hTotalConnects, gdwTotalConnects);
        Set_Long(hWarnings, gdwWarningCount);
        Set_Long(hErrors, gdwErrorCount);

		SetTimer(hWnd, 0, 500, NULL);
		SetTimer(hWnd, 1, 100, NULL);

		CenterOverOwner(hWnd);

        if(gbDebugMode)
			CheckMenuItem(MDI.hMenu, ID_ADVANCED_DEBUGMODE, MF_BYCOMMAND|MF_CHECKED);
		else CheckMenuItem(MDI.hMenu, ID_ADVANCED_DEBUGMODE, MF_BYCOMMAND|MF_UNCHECKED);

		if(gbShowSplashScreen)
		{
			DialogBox(ghAppInstance, MAKEINTRESOURCE(IDD_SPLASH), hWnd, (DLGPROC)SplashDialog);
		}

		if(gbAutoConnect)
		{
			SendMessage(hWnd, WM_COMMAND, ID_CONNECTION_CONNECTNOW, 0);
		}

        MDI.hWnd = hWnd;

		if(!IsClientConfigured(true))
		{
			WriteSysLogEx("Cannot connect to the SQL-Exchange server: Not configured.", EVENT_WARN);
		}
		if(OnlyGoodForOneYear())
		{
			WriteSysLogEx("This version of the SQL-Exchange has expired. Please obtain a newer version.", EVENT_ERROR);
		}

		return TRUE;
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

	else if(uMsg == WM_PAINT)
    {
        HDC ThisHDC;
        PAINTSTRUCT ThisPS;

        ThisHDC = BeginPaint(hWnd, &ThisPS);

        // Any painting should be done here

        EndPaint(hWnd, &ThisPS);
        return TRUE;
    }

	//--------------------------------------------------------------------------

	else if(uMsg == WM_COMMAND)
    {
		if(wParam == ID_ADVANCED_CLEARSTATISTICS)
        {
			gdwWarningCount = 0;
			gdwErrorCount = 0;
			return TRUE;
		}

		if(wParam == ID_SETTINGS_SETPASSWORD)
        {
			DialogBox(ghAppInstance, MAKEINTRESOURCE(IDD_SETGUIPWD), hWnd, (DLGPROC)SetGUIPwdDialog);
			return TRUE;
		}

		if(wParam == ID_ADVANCED_PENDING_TRANSACTIONS)
        {
			DialogBox(ghAppInstance, MAKEINTRESOURCE(IDD_PENDINGTRANS), hWnd, (DLGPROC)PendingTransDialog);
			return TRUE;
		}

		if(wParam == ID_ADVANCED_DEBUGMODE)
        {
            if(gbDebugMode)
            {
                CheckMenuItem(MDI.hMenu, ID_ADVANCED_DEBUGMODE, MF_BYCOMMAND|MF_UNCHECKED);
                gbDebugMode = false;
            }
            else{
                CheckMenuItem(MDI.hMenu, ID_ADVANCED_DEBUGMODE, MF_BYCOMMAND|MF_CHECKED);
                gbDebugMode = true;
            }
            return TRUE;
        }

		if(wParam == ID_SETTINGS_REPLICATION)
        {
			//ShowWindow(hWnd, SW_HIDE);
			DialogBox(ghAppInstance, MAKEINTRESOURCE(IDD_REPLICATION), hWnd, (DLGPROC)ReplicationDialog);
			//ShowWindow(hWnd, SW_SHOW);

			return TRUE;
        }
        
		if(wParam == ID_CONNECTION_CONNECTNOW)
        {
			if(gServer.CurrentConnections() > 0)
			{
				WriteSysLogEx("The client is already connected.", EVENT_NONE);
			}
			else{
				WriteSysLogEx("Opening manual connection to server.", EVENT_NONE);
				gbConnectNow = true;
			}
            return TRUE;
        }

        if(wParam == ID_CONNECTION_FORCEDISCONNECT)
        {
			if(gServer.Client(0)->IsConnected())
			{
				int iRet = MessageBox(hWnd, "Disconnecting during an active session may result in data loss.\r\n"
					"Are you sure you want to forcfully disconnect from the server?",
					gsTitleCaption, MB_YESNO | MB_APPLMODAL | MB_ICONQUESTION);

				if(iRet == IDYES)
				{
	                gServer.Client(0)->Disconnect();
					return 0;
				}
			}
			else{
				MessageBox(hWnd, "No connection is currently active.", gsTitleCaption, MB_APPLMODAL|MB_ICONASTERISK);
			}

            return TRUE;
        }

        if(wParam == ID_SETTINGS_GENERALSETTINGS)
		{
			LaunchConfigDlg(hWnd);
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
            return TRUE;
        }

        if(wParam == ID_FILE_CLOSEDLG)
        {
			KillTimer(hWnd, 0);
			KillTimer(hWnd, 1);
			EndDialog(hWnd,0);
			DestroyWindow(hWnd);
			MDI.hWnd = NULL;
            return TRUE;
        }

		return FALSE;
    }

    //--------------------------------------------------------------------------

	else if(uMsg == WM_SIZE)
    {
		int nWidth = LOWORD(lParam);
		int nHeight = HIWORD(lParam);

		if(nWidth < 300) nWidth = iLastWidth;

		if(nHeight < 150) nHeight = iLastHeight;

		iLastWidth = nWidth;
		iLastHeight = nHeight;

		MoveWindow(MDI.hLogList, 0, 86, nWidth, (nHeight-20) - 86, TRUE);
		MoveWindow(MDI.hStatusBar, nWidth-20, nHeight-20, nWidth, nHeight, TRUE);
		MoveWindow(hStaticFrame, 0, 0, nWidth, 85, TRUE);

		return FALSE;//Return false of we process WM_SIZE.
    }

	//--------------------------------------------------------------------------

	else if(uMsg == WM_TIMER)
    {
        if(wParam == 0)
		{
			Set_Long(hTotalConnects, gdwTotalConnects);
			Set_Long(hWarnings, gdwWarningCount);
			Set_Long(hErrors, gdwErrorCount);

			char sText[255];
			char sNumber1[64];

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

    else if(uMsg == WM_SYSCOMMAND) // Received A System Menu Message
    {
        if(LOWORD(wParam) == 1000)
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

    else if(uMsg == WM_CLOSE) 
    {
	    KillTimer(hWnd, 0);
		KillTimer(hWnd, 1);
        EndDialog(hWnd,0);
        DestroyWindow(hWnd);
		MDI.hWnd = NULL;
        return TRUE;
    }

    //--------------------------------------------------------------------------

    return FALSE;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#endif

