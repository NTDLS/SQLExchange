///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//  Copyright © NetworkDLS 2002, All rights reserved
//
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF 
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO 
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A 
// PARTICULAR PURPOSE.
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef _TABCLIENTDBS_H
#define _TABCLIENTDBS_H
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

INT_PTR CALLBACK TabClientDBSDialog(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    static HWND hSQLServer;
    static HWND hSQLDriver;
    static HWND hSQLUserName;
    static HWND hSQLPassword;
    static HWND hSQLPort;

    if(uMsg == WM_INITDIALOG)
    {
        hSQLServer   = GetDlgItem(hWnd, IDC_SQLSERVER);
        hSQLDriver   = GetDlgItem(hWnd, IDC_SQLDRIVER);
        hSQLUserName = GetDlgItem(hWnd, IDC_SQLUSERNAME);
        hSQLPassword = GetDlgItem(hWnd, IDC_SQLPASSWORD);
		hSQLPort     = GetDlgItem(hWnd, IDC_SQLCUSTPORT);

        Set_Text(hSQLServer, gsSQLCustServer);
        Set_Text(hSQLDriver, gsSQLDriver);
        Set_Text(hSQLUserName, gsSQLCustUserID);
        Set_Text(hSQLPassword, gsSQLCustPassword);
        Set_Long(hSQLPort, gdwSQLIndexServerPort);

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

			char sSQLCustServer[sizeof(gsSQLCustServer)];
			char sSQLCustDriver[sizeof(gsSQLDriver)];
			char sSQLCustUserID[sizeof(gsSQLCustUserID)];
			char sSQLCustPassword[sizeof(gsSQLCustPassword)];

			gdwSQLCustServerPort = Get_Long(hSQLPort);
			gbSQLCustUseTrustedConnection = (IsDlgButtonChecked(hWnd, IDC_USETRUSTEDCONN) == BST_CHECKED);
			gbSQLCustUseTCPIPConnection  = (IsDlgButtonChecked(hWnd, IDC_USESQLTCPIP) == BST_CHECKED);

			Get_Text(hSQLServer, sSQLCustServer, sizeof(sSQLCustServer));
			Get_Text(hSQLDriver, sSQLCustDriver, sizeof(sSQLCustDriver));
			Get_Text(hSQLUserName, sSQLCustUserID, sizeof(sSQLCustUserID));
			Get_Text(hSQLPassword, sSQLCustPassword, sizeof(sSQLCustPassword));

			strcpy_s(gsSQLCustServer, sizeof(gsSQLCustServer), sSQLCustServer);
			strcpy_s(gsSQLDriver, sizeof(gsSQLDriver), sSQLCustDriver);
			strcpy_s(gsSQLCustUserID, sizeof(gsSQLCustUserID), sSQLCustUserID);
			strcpy_s(gsSQLCustPassword, sizeof(gsSQLCustPassword), sSQLCustPassword);
				
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
