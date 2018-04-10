///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//  Copyright © NetworkDLS 2002, All rights reserved
//
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF 
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO 
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A 
// PARTICULAR PURPOSE.
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef _DIALOGS_TABSQL_CPP
#define _DIALOGS_TABSQL_CPP
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include <Windows.H>
#include <Stdio.H>
#include <Stdlib.H>
#include <CommCtrl.h>

#include "../Source/Entry.H"
#include "../Dialogs/MainDlg.H"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

using namespace NSWFL::Windows;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

BOOL CALLBACK TabSQLDialog(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    static HWND hSQLServer   = NULL;
    static HWND hSQLDriver   = NULL;
    static HWND hSQLUsername = NULL;
    static HWND hSQLPassword = NULL;
    static HWND hSQLPort = NULL;

    if(uMsg == WM_INITDIALOG)
    {
        hSQLServer   = GetDlgItem(hWnd, IDC_SERVER);
        hSQLDriver   = GetDlgItem(hWnd, IDC_SQLDRIVER);
        hSQLUsername = GetDlgItem(hWnd, IDC_SQLUSERNAME);
        hSQLPassword = GetDlgItem(hWnd, IDC_SQLPASSWORD);
		hSQLPort = GetDlgItem(hWnd, IDC_SQLPORT);

        Set_Text(hSQLServer, gsSQLServer);
        Set_Text(hSQLDriver, gsSQLDriver);

		if(gbUseTCPIPConnection)
		{
			CheckDlgButton(hWnd, IDC_USESQLTCPIP, BST_CHECKED);
			EnableWindow(hSQLPort, TRUE);
		}
		else{
			EnableWindow(hSQLPort, FALSE);
		}

		if(gbUseTrustedConnection)
		{
			CheckDlgButton(hWnd, IDC_USETRUSTEDCONN, BST_CHECKED);
			EnableWindow(hSQLUsername, FALSE);
			EnableWindow(hSQLPassword, FALSE);
		}
		else{
			EnableWindow(hSQLUsername, TRUE);
			EnableWindow(hSQLPassword, TRUE);
		}

		Set_Text(hSQLUsername, gsSQLUserID);
        Set_Text(hSQLPassword, gsSQLPassword);
        Set_Long(hSQLPort, gdwSQLServerPort);

        return TRUE;
    }

    if(uMsg == WM_COMMAND)
    {
		if(wParam == IDC_USESQLTCPIP)
		{
			BOOL bChecked = (IsDlgButtonChecked(hWnd, IDC_USESQLTCPIP) == BST_CHECKED);
			EnableWindow(hSQLPort, bChecked);
		}

		if(wParam == IDC_USETRUSTEDCONN)
		{
			BOOL bChecked = (IsDlgButtonChecked(hWnd, IDC_USETRUSTEDCONN) == BST_CHECKED);
			EnableWindow(hSQLUsername, !bChecked);
			EnableWindow(hSQLPassword, !bChecked);
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
        if(pNMH->code == PSN_APPLY) // Ok
        {
			gdwSQLServerPort = Get_Long(hSQLPort);

            char lsSQLServer[255 + 1];
            int liSQLServerLen = Get_Text(hSQLServer, lsSQLServer, 255);
            if(liSQLServerLen == 0 || liSQLServerLen > 255)
            {
                MessageBox(hWnd, "You must enter a valid SQL server name. Between 1 and 255 characters.", gsTitleCaption, MB_ICONASTERISK);
				SetWindowLongPtr(hWnd, DWLP_MSGRESULT, PSNRET_INVALID_NOCHANGEPAGE); // Prevent window from closing
                return TRUE;
            }

            char lsSQLDriver[255 + 1];
            int liSQLDriverLen = Get_Text(hSQLDriver, lsSQLDriver, 255);
            if(liSQLDriverLen == 0 || liSQLDriverLen > 255)
            {
                MessageBox(hWnd, "You must enter a valid SQL driver name. Between 1 and 255 characters.", gsTitleCaption, MB_ICONASTERISK);
                SetWindowLongPtr(hWnd, DWLP_MSGRESULT, PSNRET_INVALID_NOCHANGEPAGE); // Prevent window from closing
                return TRUE;
            }

            char lsSQLUsername[255 + 1];
            int liSQLUsernameLen = Get_Text(hSQLUsername, lsSQLUsername, 255);
            if(liSQLUsernameLen > 255)
            {
                MessageBox(hWnd, "You must enter a valid SQL username. LEss than 255 characters.", gsTitleCaption, MB_ICONASTERISK);
                SetWindowLongPtr(hWnd, DWLP_MSGRESULT, PSNRET_INVALID_NOCHANGEPAGE); // Prevent window from closing
                return TRUE;
            }

            char lsSQLPassword[255 + 1];
            int liSQLPasswordLen = Get_Text(hSQLPassword, lsSQLPassword, 255);
            if(liSQLPasswordLen < 0 || liSQLPasswordLen > 255)
            {
                MessageBox(hWnd, "You must enter a valid SQL password. Less than 255 characters.", gsTitleCaption, MB_ICONASTERISK);
                SetWindowLongPtr(hWnd, DWLP_MSGRESULT, PSNRET_INVALID_NOCHANGEPAGE); // Prevent window from closing
                return TRUE;
            }

			gbUseTrustedConnection = (IsDlgButtonChecked(hWnd, IDC_USETRUSTEDCONN) == BST_CHECKED);
			gbUseTCPIPConnection  = (IsDlgButtonChecked(hWnd, IDC_USESQLTCPIP) == BST_CHECKED);

			strcpy_s(gsSQLServer, sizeof(lsSQLServer), lsSQLServer);
            strcpy_s(gsSQLUserID, sizeof(lsSQLUsername), lsSQLUsername);
            strcpy_s(gsSQLPassword, sizeof(lsSQLPassword), lsSQLPassword);
            strcpy_s(gsSQLDriver, sizeof(lsSQLDriver), lsSQLDriver);

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
