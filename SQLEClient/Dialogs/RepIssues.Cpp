///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//  Copyright © NetworkDLS 2002, All rights reserved
//
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF 
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO 
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A 
// PARTICULAR PURPOSE.
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef _RepIssues_Cpp
#define _RepIssues_Cpp
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include <Windows.H>
#include <Stdio.H>
#include <Stdlib.H>
#include <CommCtrl.h>

#include "../Source/Entry.H"
#include "../Source/Replication.H"

#include "../Dialogs/MainDlg.H"
#include "../Dialogs/RepIssues.H"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

using namespace NSWFL::Windows;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//---------------------(Variable Declarations)
HWND hRepIssuesDlg = NULL;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

BOOL CALLBACK RepIssuesDialog(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    //--------------------------------------------------------------------------

	static HWND hTablesList = NULL;

    //--------------------------------------------------------------------------

    if(uMsg == WM_INITDIALOG)
    {
        hRepIssuesDlg = hWnd;

        char sCaption[1024];
		sprintf_s(sCaption, sizeof(sCaption), "%s :: %s", gsTitleCaption, "Replication Issues");
        SendMessage(hWnd, (UINT)WM_SETTEXT, (WPARAM)0, (LPARAM)sCaption);
        SendMessage(hWnd, WM_SETICON, TRUE, (LPARAM) LoadIcon(ghAppInstance, MAKEINTRESOURCE(IDI_MAIN)) );

		LV_COLUMN MyLVColumb;
		memset(&MyLVColumb, 0, sizeof(MyLVColumb));

		hTablesList = GetDlgItem(hWnd, IDC_TALBES);

        int iLVStyle = LVS_EX_GRIDLINES;
        SendMessage(hTablesList, (UINT)LVM_SETEXTENDEDLISTVIEWSTYLE, (WPARAM)0, (LPARAM)iLVStyle);

		// Create Listview columns
        MyLVColumb.cx        = 300;
        MyLVColumb.pszText   = "Table Name";
        MyLVColumb.iSubItem  = 0;
        MyLVColumb.mask      = LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
        MyLVColumb.fmt       = LVCFMT_LEFT;
        ListView_InsertColumn(hTablesList, 0, &MyLVColumb);

        MyLVColumb.cx        = 150;
        MyLVColumb.pszText   = "Created Date";
        MyLVColumb.iSubItem  = 1;
        MyLVColumb.mask      = LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
        MyLVColumb.fmt       = LVCFMT_LEFT;
        ListView_InsertColumn(hTablesList, 1, &MyLVColumb);

		PopRepIssuesGrid(hTablesList);

		CenterOverOwner(hWnd);

        return TRUE;
    }

    //--------------------------------------------------------------------------

    if(uMsg == WM_COMMAND)
    {
        if(wParam == ID_CLOSE)
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

bool PopRepIssuesGrid(HWND hTablesList)
{
	int iItem = 0;

	CSQL cSQL;
	CBoundRecordSet rsTemp;
	
	LV_ITEM MyLVItem;
	memset(&MyLVItem, 0, sizeof(MyLVItem));

	char *sSQL = "SELECT"
		" [B].[name] AS TableName,"
		" [B].[crdate] AS CreatedDate"
		" FROM [sysobjects] AS [B]"
		" WHERE [B].[xtype] = 'U' AND LEFT([B].[name], 8) <> 'SQLExch_'"
		" AND [id] NOT IN (SELECT [A].[id] FROM [syscolumns] as A"
		" WHERE [A].[colstat] = 1)"
		" ORDER BY [B].[name]";
	
	if(!ConnectToSQLServer(&cSQL, gsSQLServer, gsSQLUserID, gsSQLPassword, "master",
		gbUseTCPIPConnection, gbUseTrustedConnection, gdwSQLServerPort))
	{
		MessageBox(NULL,
			"Failed to connect to the SQL server.",
			gsTitleCaption, MB_TASKMODAL|MB_ICONERROR);

		return false;
	}

	cSQL.Execute(sSQL, &rsTemp);

	while(rsTemp.Fetch())
	{
		MyLVItem.pszText = rsTemp.Values("TableName")->ToString();
        MyLVItem.mask = LVIF_TEXT;
        MyLVItem.iSubItem = 0;
		MyLVItem.iItem = iItem;
        ListView_InsertItem(hTablesList, &MyLVItem);

        MyLVItem.pszText = rsTemp.Values("CreatedDate")->ToString();
        MyLVItem.mask = LVIF_TEXT;
        MyLVItem.iSubItem = 1;
		MyLVItem.iItem = iItem;
        ListView_SetItem(hTablesList, &MyLVItem);

		iItem++;
	}

	cSQL.Disconnect();

	return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#endif

