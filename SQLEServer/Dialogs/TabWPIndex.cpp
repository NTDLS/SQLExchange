///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//  Copyright © NetworkDLS 2002, All rights reserved
//
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF 
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO 
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A 
// PARTICULAR PURPOSE.
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef _TABWPINDEX_H
#define _TABWPINDEX_H
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include <Windows.H>
#include <Stdio.H>
#include <Stdlib.H>

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "../Source/Entry.H"

#include "../Dialogs/MainDlg.H"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

using namespace NSWFL::Windows;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

INT_PTR CALLBACK TabWPIndexDialog(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    static HWND hSQLServer;
    static HWND hSQLDriver;
    static HWND hSQLUserName;
    static HWND hSQLPassword;
    static HWND hSQLPort = NULL;

    if(uMsg == WM_INITDIALOG)
    {
        hSQLServer   = GetDlgItem(hWnd, IDC_SQLSERVER);
        hSQLDriver   = GetDlgItem(hWnd, IDC_SQLDRIVER);
        hSQLUserName = GetDlgItem(hWnd, IDC_SQLUSERNAME);
        hSQLPassword = GetDlgItem(hWnd, IDC_SQLPASSWORD);
		hSQLPort     = GetDlgItem(hWnd, IDC_SQLPORT);

		if(gbSQLIndexUseTCPIPConnection)
		{
			CheckDlgButton(hWnd, IDC_USESQLTCPIP, BST_CHECKED);
			EnableWindow(hSQLPort, TRUE);
		}
		else{
			EnableWindow(hSQLPort, FALSE);
		}

		if(gbSQLIndexUseTrustedConnection)
		{
			CheckDlgButton(hWnd, IDC_USETRUSTEDCONN, BST_CHECKED);
			EnableWindow(hSQLUserName, FALSE);
			EnableWindow(hSQLPassword, FALSE);
		}
		else{
			EnableWindow(hSQLUserName, TRUE);
			EnableWindow(hSQLPassword, TRUE);
		}

		Set_Text(hSQLServer, gsSQLIndexServer);
        Set_Text(hSQLDriver, gsSQLDriver);
        Set_Text(hSQLUserName, gsSQLIndexUserID);
        Set_Text(hSQLPassword, gsSQLIndexPassword);
        Set_Long(hSQLPort, gdwSQLIndexServerPort);

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
			EnableWindow(hSQLUserName, !bChecked);
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
			char sSQLIndexServer[sizeof(gsSQLIndexServer)];
			char sSQLIndexDriver[sizeof(gsSQLDriver)];
			char sSQLIndexUserID[sizeof(gsSQLIndexUserID)];
			char sSQLIndexPassword[sizeof(gsSQLIndexPassword)];

			gdwSQLIndexServerPort = Get_Long(hSQLPort);

			Get_Text(hSQLServer, sSQLIndexServer, sizeof(sSQLIndexServer));
			Get_Text(hSQLDriver, sSQLIndexDriver, sizeof(sSQLIndexDriver));
			Get_Text(hSQLUserName, sSQLIndexUserID, sizeof(sSQLIndexUserID));
			Get_Text(hSQLPassword, sSQLIndexPassword, sizeof(sSQLIndexPassword));

			strcpy_s(gsSQLIndexServer, sizeof(gsSQLIndexServer), sSQLIndexServer);
			strcpy_s(gsSQLDriver, sizeof(gsSQLDriver), sSQLIndexDriver);
			strcpy_s(gsSQLIndexUserID, sizeof(gsSQLIndexUserID), sSQLIndexUserID);
			strcpy_s(gsSQLIndexPassword, sizeof(gsSQLIndexPassword), sSQLIndexPassword);

			gbSQLIndexUseTrustedConnection = (IsDlgButtonChecked(hWnd, IDC_USETRUSTEDCONN) == BST_CHECKED);
			gbSQLIndexUseTCPIPConnection  = (IsDlgButtonChecked(hWnd, IDC_USESQLTCPIP) == BST_CHECKED);

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
