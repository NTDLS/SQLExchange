///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//  Copyright © NetworkDLS 2002, All rights reserved
//
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF 
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO 
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A 
// PARTICULAR PURPOSE.
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef _CreateNewDB_CPP
#define _CreateNewDB_CPP
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include <Windows.H>
#include <Stdio.H>
#include <Stdlib.H>
#include <CommCtrl.h>

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "../Source/Entry.H"
#include "../Source/Console.H"
#include "../Source/NewDB.H"

#include "../Dialogs/MainDlg.H"
#include "../Dialogs/SysStats.H"
#include "../Dialogs/CreateNewDB.H"

#include "../../../@Libraries/CSocketServer/CSocketServer.H"
#include "../../SharedClasses/CStatusDlg/CStatusDlg.H"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

using namespace NSWFL::Windows;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

HWND CreateNewDB_hWnd = NULL;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool GetCompanyDBPrefix(HWND CompanyAcct_hWnd, char *sName, int iMaxSz)
{
	CSQL MySQL;
	char sSQL[1024];
	int iOutSz = 0;
	
	if(!Get_Text(CompanyAcct_hWnd, sName, iMaxSz))
	{
		return false;
	}

	if(ConnectToSQLServer(&MySQL, gsSQLIndexServer, gsSQLIndexUserID, gsSQLIndexPassword, gsSQLIndexDatabase,
		gbSQLIndexUseTCPIPConnection, gbSQLIndexUseTrustedConnection, gdwSQLIndexServerPort))
	{
		CBoundRecordSet rsAccounts;

		sprintf_s(sSQL, sizeof(sSQL),
			"EXEC [%s].[%s].GetCompanyDatabasePrefixByName '%s'", 
			gsSQLIndexDatabase, gsDefaultDBO, sName);

		if(MySQL.Execute(sSQL, &rsAccounts))
		{
			if(rsAccounts.Fetch())
			{
				rsAccounts.Values("DatabasePrefix")->ToString(sName, iMaxSz);
			}
			return true;
		}
	}
	else {
		MsgBox("Error connecting to the [master] database.");
	}

	return false;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool PopCompanyAccounts(HWND CompanyAcct_hWnd)
{
	CSQL MySQL;
	char sSQL[1024];
	int iOutSz = 0;
	
	if(ConnectToSQLServer(&MySQL, gsSQLIndexServer, gsSQLIndexUserID, gsSQLIndexPassword, gsSQLIndexDatabase,
		gbSQLIndexUseTCPIPConnection, gbSQLIndexUseTrustedConnection, gdwSQLIndexServerPort))
	{
		CBoundRecordSet rsAccounts;

		sprintf_s(sSQL, sizeof(sSQL),
			"EXEC [%s].[%s].GetAllCompanyNames",
			gsSQLIndexDatabase, gsDefaultDBO);

		if(MySQL.Execute(sSQL, &rsAccounts))
		{
			while(rsAccounts.Fetch())
			{
				SendMessage(CompanyAcct_hWnd, (UINT)CB_ADDSTRING,
					(WPARAM)0, (LPARAM)rsAccounts.Values("Name")->ToString());
			}
			rsAccounts.Close();
			return true;
		}
	}
	else {
		MsgBox("Error connecting to the [master] database.");
	}

	return false;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

INT_PTR CALLBACK CreateNewDBDialog(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    //--------------------------------------------------------------------------

	static HWND BackupName_hWnd = NULL;
	static HWND CompanyAcct_hWnd = NULL;
	static HWND DBO_hWnd = NULL;
	static HWND DataFileLOC_hWnd = NULL;
	static HWND LogFileLOC_hWnd = NULL;

    //--------------------------------------------------------------------------

    if(uMsg == WM_INITDIALOG)
    {
        CreateNewDB_hWnd = hWnd;

        char sCaption[1024];
		sprintf_s(sCaption, sizeof(sCaption), "%s :: %s", gsTitleCaption, "Create Database");
		SendMessage(hWnd, (UINT)WM_SETTEXT, (WPARAM)0, (LPARAM)sCaption);
        SendMessage(hWnd, WM_SETICON, TRUE, (LPARAM) LoadIcon(ghAppInstance, MAKEINTRESOURCE(IDI_MAIN)) );

		BackupName_hWnd  = GetDlgItem(hWnd, IDC_BACKUPNAME);
		CompanyAcct_hWnd = GetDlgItem(hWnd, IDC_COMPANYACCT);
		DBO_hWnd         = GetDlgItem(hWnd, IDC_DBO);
		DataFileLOC_hWnd = GetDlgItem(hWnd, IDC_DATAFILELOC);
		LogFileLOC_hWnd = GetDlgItem(hWnd, IDC_LOGFILELOC);

		PopCompanyAccounts(CompanyAcct_hWnd);

		Set_Text(BackupName_hWnd, "");

		Set_Text(DBO_hWnd, gsDefaultDBO);
		Set_Text(DataFileLOC_hWnd, gsSQLDataFiles);
		Set_Text(LogFileLOC_hWnd, gsSQLLogFiles);

		CenterOverOwner(hWnd);

        return TRUE;
    }

    //--------------------------------------------------------------------------

    if(uMsg == WM_COMMAND)
    {
		if(wParam == IDC_CREATE)
        {
			char sBackupFile[MAX_PATH];
			char sName[1024 + 1];
			int iLen = 0;

			if(!GetCompanyDBPrefix(CompanyAcct_hWnd, sName, sizeof(sName)))
			{
				MsgBox("You must select a company account.");
				return TRUE;			
			}

			if(strlen(sName) == 0)
			{
				MsgBox("You must select a company account.");
				return TRUE;			
			}

			iLen = Get_Text(BackupName_hWnd, sBackupFile, sizeof(sBackupFile));
			if(iLen == 0 || iLen > sizeof(sBackupFile))
			{
				MsgBox("You must enter an existing backup file name.");
				return TRUE;
			}

			iLen = Get_Text(DataFileLOC_hWnd, gsSQLDataFiles, sizeof(gsSQLDataFiles));
			if(iLen == 0 || iLen > sizeof(gsSQLDataFiles))
			{
				MsgBox("You must enter a data file location.");
				return TRUE;
			}

			iLen = Get_Text(LogFileLOC_hWnd, gsSQLLogFiles, sizeof(gsSQLLogFiles));
			if(iLen == 0 || iLen > sizeof(gsSQLLogFiles))
			{
				MsgBox("You must enter a log file location.");
				return TRUE;
			}

			iLen = Get_Text(DBO_hWnd, gsDefaultDBO, sizeof(gsDefaultDBO));
			if(iLen == 0 || iLen > sizeof(gsDefaultDBO))
			{
				//MsgBox("You must enter a database owner.");
				//return TRUE;
			}

			CStatusDlg MyStatDlg;

			MyStatDlg.Load(hWnd, "Setting up new client database.", IDI_MAIN, false);
			MyStatDlg.Show();

			if(SetupNewClient(sBackupFile, sName, gsDefaultDBO, true, true, true, &MyStatDlg))
			{
				MsgBox("New database creation complete!");
				EndDialog(hWnd,0);
			}

			MyStatDlg.SetButtonText("Ok");

			MyStatDlg.EnableButton();

			while(MyStatDlg.IsDlgVisible())
			{
				Sleep(1);
			}

			MyStatDlg.Unload();

			SetForegroundWindow(hWnd);

			return TRUE;
        }

        if(wParam == IDC_CANCEL)
        {
			EndDialog(hWnd,0);
			DestroyWindow(hWnd);
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

    if(uMsg == WM_CLOSE) 
    {
        EndDialog(hWnd,0);
        DestroyWindow(hWnd);
        return TRUE;
    }

    //--------------------------------------------------------------------------

    return FALSE;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#endif

