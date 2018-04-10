///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//  Copyright © NetworkDLS 2002, All rights reserved
//
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF 
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO 
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A 
// PARTICULAR PURPOSE.
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef _DIALOGS_TABADVANCED_CPP
#define _DIALOGS_TABADVANCED_CPP
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include <Windows.H>
#include <Stdio.H>
#include <Stdlib.H>
#include <CommCtrl.h>
#include <ShlObj.H>

#include "../Source/Entry.H"

#include "../Dialogs/MainDlg.H"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

using namespace NSWFL::Windows;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

INT_PTR CALLBACK TabAdvancedDialog(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	static HWND hSQLDatFileLoc = NULL;
	static HWND hSQLLogFileLoc = NULL;
	static HWND hHashTimeout = NULL;
	static HWND hMaxInactivity = NULL;
    static HWND hTempDir = NULL;

	if(uMsg == WM_INITDIALOG)
    {
		hSQLDatFileLoc	= GetDlgItem(hWnd, IDC_SQL_DATA_LOC);
		hSQLLogFileLoc	= GetDlgItem(hWnd, IDC_SQL_LOG_LOC);
		hHashTimeout	= GetDlgItem(hWnd, IDC_HASHTIMEOUT);
		hMaxInactivity	= GetDlgItem(hWnd, IDC_MAXINACTIVITY);
        hTempDir		= GetDlgItem(hWnd, IDC_TEMPDIR);

		Set_Text(hSQLDatFileLoc, gsSQLDataFiles);
		Set_Text(hSQLLogFileLoc, gsSQLLogFiles);
		Set_Long(hHashTimeout, gdwHashTimeout);
		Set_Long(hMaxInactivity, gdwMaxInactivityTime);
        Set_Text(hTempDir, gsImportTempDir);

		if(gbCacheLogFile)
			CheckDlgButton(hWnd, IDC_CACHELOGFILE, BST_CHECKED);

		if(gbRTOFDWI)
			CheckDlgButton(hWnd, IDC_RTOFDWI, BST_CHECKED);

		if(gbStartAutomatically)
			CheckDlgButton(hWnd, IDC_AUTORUN, BST_CHECKED);

		if(gbShowSplashScreen)
			CheckDlgButton(hWnd, IDC_SHOWSPLASHSCREEN, BST_CHECKED);

		if(gbVisualAlertOnError)
			CheckDlgButton(hWnd, IDC_ALERTONERRORS, BST_CHECKED);

		if(gbVisualAlertOnWarn)
			CheckDlgButton(hWnd, IDC_ALERTONWARNING, BST_CHECKED);

		if(gbAllowOffLineDataProcessing)
			CheckDlgButton(hWnd, IDC_OFFLINEPROCESSING, BST_CHECKED);

		return TRUE;
    }

	if(uMsg == WM_COMMAND)
    {
		if(wParam == IDC_BROWSE_DAT_FOLDER)
		{
			char sDirLoc[MAX_PATH];

			if(BrowseDirectory(hWnd,
				"Select the directoy where the server will store SQL data files.",
				BIF_RETURNONLYFSDIRS|BIF_USENEWUI, sDirLoc, sizeof(sDirLoc)))
			{
				Set_Text(hSQLDatFileLoc, sDirLoc);
			}

			return TRUE;
		}
		else if(wParam == IDC_BROWSE_LOG_FOLDER)
		{
			char sDirLoc[MAX_PATH];

			if(BrowseDirectory(hWnd,
				"Select the directoy where the server will store SQL log files.",
				BIF_RETURNONLYFSDIRS|BIF_USENEWUI, sDirLoc, sizeof(sDirLoc)))
			{
				Set_Text(hSQLLogFileLoc, sDirLoc);
			}

			return TRUE;
		}
		if(wParam == IDC_BROWSE_TEMP_FOLDER)
		{
			char sDirLoc[MAX_PATH];

			if(BrowseDirectory(hWnd,
				"Select the directoy where the server will store temporary client files.",
				BIF_RETURNONLYFSDIRS|BIF_USENEWUI, sDirLoc, sizeof(sDirLoc)))
			{
				Set_Text(hTempDir, sDirLoc);
			}

			return TRUE;
		}
		return TRUE;
	}

	if(uMsg == WM_NOTIFY)
    {
        LPNMHDR pNMH = (LPNMHDR)lParam;
        if(pNMH->code == PSN_KILLACTIVE)
        {
            return FALSE;
        }
        else if(pNMH->code == PSN_APPLY) // Ok
        {
			char sSQLDataFiles[MAX_PATH];
			char sSQLLogFiles[MAX_PATH];
			char sImportTempDir[MAX_PATH];
			int iSz = 0;

			DWORD dwHashTimeout = Get_Long(hHashTimeout);
			if(dwHashTimeout < 0 || dwHashTimeout > 57600)
			{
                MessageBox(hWnd, "You must enter a valid \"Hash Timeout\" between 1 and 57,600 or 0 for disabled.", gsTitleCaption, MB_ICONASTERISK);
                SetWindowLongPtr(hWnd, DWLP_MSGRESULT, PSNRET_INVALID_NOCHANGEPAGE); // Prevent window from closing
				return TRUE;
			}

			DWORD dwMaxInactivityTime = Get_Long(hMaxInactivity);
			if((dwMaxInactivityTime < 100 && dwMaxInactivityTime != 0) || dwMaxInactivityTime > 57600)
			{
                MessageBox(hWnd, "You must enter a valid \"Max Inactivity\" between 1 and 57,600 or 0 for disabled.", gsTitleCaption, MB_ICONASTERISK);
                SetWindowLongPtr(hWnd, DWLP_MSGRESULT, PSNRET_INVALID_NOCHANGEPAGE); // Prevent window from closing
				return TRUE;
			}

			gbCacheLogFile = (IsDlgButtonChecked(hWnd, IDC_CACHELOGFILE) == BST_CHECKED);
			gbRTOFDWI = (IsDlgButtonChecked(hWnd, IDC_RTOFDWI) == BST_CHECKED);
			gbStartAutomatically = (IsDlgButtonChecked(hWnd, IDC_AUTORUN) == BST_CHECKED);
			gbShowSplashScreen = (IsDlgButtonChecked(hWnd, IDC_SHOWSPLASHSCREEN) == BST_CHECKED);
			gbAllowOffLineDataProcessing = (IsDlgButtonChecked(hWnd, IDC_OFFLINEPROCESSING) == BST_CHECKED);
			gbVisualAlertOnError = (IsDlgButtonChecked(hWnd, IDC_ALERTONERRORS) == BST_CHECKED);
			gbVisualAlertOnWarn = (IsDlgButtonChecked(hWnd, IDC_ALERTONWARNING) == BST_CHECKED);
			gdwHashTimeout = dwHashTimeout;
			gdwMaxInactivityTime = dwMaxInactivityTime;

			iSz = Get_Text(hTempDir, sImportTempDir, sizeof(sImportTempDir));
			if(iSz <= 3 || iSz > sizeof(sImportTempDir))
			{
                MessageBox(hWnd, "You must enter a valid \"Temporary File\" location.", gsTitleCaption, MB_ICONASTERISK);
                SetWindowLongPtr(hWnd, DWLP_MSGRESULT, PSNRET_INVALID_NOCHANGEPAGE); // Prevent window from closing
				return TRUE;
			}

			iSz = Get_Text(hSQLDatFileLoc, sSQLDataFiles, sizeof(sSQLDataFiles));
			if(iSz <= 3 || iSz > sizeof(sSQLDataFiles))
			{
                MessageBox(hWnd, "You must enter a valid \"SQL Data File\" location.", gsTitleCaption, MB_ICONASTERISK);
                SetWindowLongPtr(hWnd, DWLP_MSGRESULT, PSNRET_INVALID_NOCHANGEPAGE); // Prevent window from closing
				return TRUE;
			}

			iSz = Get_Text(hSQLLogFileLoc, sSQLLogFiles, sizeof(sSQLLogFiles));
			if(iSz <= 3 || iSz > sizeof(sSQLLogFiles))
			{
                MessageBox(hWnd, "You must enter a valid \"SQL Log File\" location.", gsTitleCaption, MB_ICONASTERISK);
                SetWindowLongPtr(hWnd, DWLP_MSGRESULT, PSNRET_INVALID_NOCHANGEPAGE); // Prevent window from closing
				return TRUE;
			}

			strcpy_s(gsSQLDataFiles, sizeof(gsSQLDataFiles), sSQLDataFiles);
			strcpy_s(gsSQLLogFiles, sizeof(gsSQLLogFiles), sSQLLogFiles);
			strcpy_s(gsImportTempDir, sizeof(gsImportTempDir), sImportTempDir);

			return TRUE;
        }
        if(pNMH->code == PSN_RESET) // Cancel
        {
            return FALSE;
        }
    }

    return FALSE;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#endif
