///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//  Copyright © NetworkDLS 2002, All rights reserved
//
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF 
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO 
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A 
// PARTICULAR PURPOSE.
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef _PENDINGTRANS_CPP
#define _PENDINGTRANS_CPP
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include <Windows.H>
#include <Stdio.H>
#include <Stdlib.H>
#include <CommCtrl.h>

#include "../Source/Entry.H"
#include "../Source/Replication.H"

#include "../Dialogs/MainDlg.H"
#include "../Dialogs/PendingTrans.H"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

using namespace NSWFL::Windows;
using namespace NSWFL::Conversion;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//---------------------(Variable Declarations)
HWND hPendingTransDlg = NULL;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

BOOL CALLBACK PendingTransDialog(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    //--------------------------------------------------------------------------

	static HWND hTablesList = NULL;

    //--------------------------------------------------------------------------

    if(uMsg == WM_INITDIALOG)
    {
		hPendingTransDlg = hWnd;

        char sCaption[1024];
		sprintf_s(sCaption, sizeof(sCaption), "%s :: %s", gsTitleCaption, "Pending Transactions");
		SendMessage(hWnd, (UINT)WM_SETTEXT, (WPARAM)0, (LPARAM)sCaption);
        SendMessage(hWnd, WM_SETICON, TRUE, (LPARAM) LoadIcon(ghAppInstance, MAKEINTRESOURCE(IDI_MAIN)) );

		LV_COLUMN MyLVColumb;
		memset(&MyLVColumb, 0, sizeof(MyLVColumb));

		hTablesList = GetDlgItem(hWnd, IDC_TALBES);

        int iLVStyle = LVS_EX_GRIDLINES;
        SendMessage(hTablesList, (UINT)LVM_SETEXTENDEDLISTVIEWSTYLE, (WPARAM)0, (LPARAM)iLVStyle);

		// Create Listview columns
        MyLVColumb.cx        = 60;
        MyLVColumb.pszText   = "Batches";
        MyLVColumb.iSubItem  = 0;
        MyLVColumb.mask      = LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM | LVCF_FMT;
        MyLVColumb.fmt       = LVCFMT_RIGHT;
        ListView_InsertColumn(hTablesList, 0, &MyLVColumb);

        MyLVColumb.cx        = 135;
        MyLVColumb.pszText   = "Database";
        MyLVColumb.iSubItem  = 1;
        MyLVColumb.mask      = LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM | LVCF_FMT;
        MyLVColumb.fmt       = LVCFMT_LEFT;
        ListView_InsertColumn(hTablesList, 1, &MyLVColumb);

        MyLVColumb.cx        = 135;
        MyLVColumb.pszText   = "Schema";
        MyLVColumb.iSubItem  = 2;
        MyLVColumb.mask      = LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM | LVCF_FMT;
        MyLVColumb.fmt       = LVCFMT_LEFT;
        ListView_InsertColumn(hTablesList, 2, &MyLVColumb);

		MyLVColumb.cx        = 185;
        MyLVColumb.pszText   = "Table";
        MyLVColumb.iSubItem  = 3;
        MyLVColumb.mask      = LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM | LVCF_FMT;
        MyLVColumb.fmt       = LVCFMT_LEFT;
        ListView_InsertColumn(hTablesList, 3, &MyLVColumb);

		CenterOverOwner(hWnd);

		if(!PopPendingTransGrid(hTablesList))
		{
			EndDialog(hWnd,0);
			DestroyWindow(hWnd);
		}

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

bool PopPendingTransGrid(HWND hTablesList)
{
	bool bResult = true;
	int iItem = 0;
	
	int iTempSz = 0;

	CSQL cSQL;
	CBoundRecordSet rsTemp;
	
	LV_ITEM MyLVItem;
	memset(&MyLVItem, 0, sizeof(MyLVItem));

	char sSQL[1024];
	sprintf_s(sSQL, sizeof(sSQL),
		"SELECT"
		"	COUNT(0) AS Records,"
		"	[Database], [Schema], [Table]"
		" FROM"
		"	[%s].[%s].SQLExch_Transactions"
		" GROUP BY"
		"	[Database], [Schema], [Table]"
		" ORDER BY"
		"	[Database], [Schema], [Table]",
		gsReplicationDB, gsDefaultDBO);

	if(!ConnectToSQLServer(&cSQL, gsSQLServer, gsSQLUserID, gsSQLPassword, "master",
		gbUseTCPIPConnection, gbUseTrustedConnection, gdwSQLServerPort))
	{
		MessageBox(NULL, "Failed to connect to the SQL server.",
			gsTitleCaption, MB_TASKMODAL|MB_ICONERROR);

		return false;
	}

	cSQL.ThrowErrors(false);

	char sNumber[65];

	if(cSQL.Execute(sSQL, &rsTemp))
	{
		cSQL.ThrowErrors(true);

		while(rsTemp.Fetch())
		{
			FormatInteger(sNumber, sizeof(sNumber), rsTemp.Values("Records")->ToIntegerS());
			MyLVItem.pszText = sNumber;
			MyLVItem.mask = LVIF_TEXT;
			MyLVItem.iSubItem = 0; 
			MyLVItem.iItem = iItem;
			ListView_InsertItem(hTablesList, &MyLVItem);

			MyLVItem.pszText = rsTemp.Values("Database")->ToString();
			MyLVItem.mask = LVIF_TEXT;
			MyLVItem.iSubItem = 1;
			MyLVItem.iItem = iItem;
			ListView_SetItem(hTablesList, &MyLVItem);

			MyLVItem.pszText = rsTemp.Values("Schema")->ToString();
			MyLVItem.mask = LVIF_TEXT;
			MyLVItem.iSubItem = 2;
			MyLVItem.iItem = iItem;
			ListView_SetItem(hTablesList, &MyLVItem);

			MyLVItem.pszText = rsTemp.Values("Table")->ToString();
			MyLVItem.mask = LVIF_TEXT;
			MyLVItem.iSubItem = 3;
			MyLVItem.iItem = iItem;
			ListView_SetItem(hTablesList, &MyLVItem);
			iItem++;
		}
	}
	else{
		//MessageBox(NULL, "No databases have been initialized for replication.",
		//	gsTitleCaption, MB_TASKMODAL|MB_ICONINFORMATION);
		//bResult = false;
	}

	rsTemp.Close();

	cSQL.Disconnect();

	return bResult;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#endif

