///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//  Copyright © NetworkDLS 2002, All rights reserved
//
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF 
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO 
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A 
// PARTICULAR PURPOSE.
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef _ReplicationDlg_CPP
#define _ReplicationDlg_CPP
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include <Windows.H>
#include <Stdio.H>
#include <Stdlib.H>
#include <CommCtrl.h>

#include "../../SharedClasses/CStatusDlg/CStatusDlg.H"
#include "../../SharedSource/ListView.H"

#include "../Source/Entry.H"
#include "../Source/Replication.H"

#include "../Dialogs/MainDlg.H"
#include "../Dialogs/ReplicationDlg.H"
#include "../Dialogs/RepIssues.H"
#include "../Dialogs/PendingTrans.H"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

using namespace NSWFL::Windows;
using namespace NSWFL::String;
using namespace NSWFL::ListView;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//---------------------(Local Procs)
void AddReplicateEditControlProc(HWND hOwner, HWND hWnd);
LRESULT CALLBACK ReplicateEditControlProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
DWORD WINAPI ProcessTablesGrid_Thread(LPVOID lpData);
DWORD WINAPI PopTablesGrid_Thread(LPVOID lpData);

//---------------------(Variable Declarations)
WNDPROC gpOldReplicationListProc;
HWND hReplication = NULL;

HWND hTablesList = NULL;

bool bProcessing = false;
bool bReplicationDLGopen = false;

HANDLE hReplicationThread = NULL;
DWORD dwReplicationThreadID = 0;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

BOOL CALLBACK ReplicationDialog(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    //--------------------------------------------------------------------------

	static HIMAGELIST hImgList;
	static HWND hFindText;
	static char sLastText[1024];
	static bool bFirstActivate = false;

    //--------------------------------------------------------------------------

    if(uMsg == WM_INITDIALOG)
    {
		bReplicationDLGopen = true;
		bFirstActivate = true;

        hReplication = hWnd;

        char sCaption[1024];
		sprintf_s(sCaption, sizeof(sCaption), "%s :: %s", gsTitleCaption, "Replication");
        SendMessage(hWnd, (UINT)WM_SETTEXT, (WPARAM)0, (LPARAM)sCaption);
        SendMessage(hWnd, WM_SETICON, TRUE, (LPARAM) LoadIcon(ghAppInstance, MAKEINTRESOURCE(IDI_MAIN)) );

		LV_COLUMN MyLVColumb;
		memset(&MyLVColumb, 0, sizeof(MyLVColumb));

		hTablesList = GetDlgItem(hWnd, IDC_TALBES);
		hFindText = GetDlgItem(hWnd, IDC_FINDTEXT);

		AddReplicateEditControlProc(hWnd, hFindText);

        gpOldReplicationListProc = (WNDPROC) SetWindowLongPtr(hTablesList, GWLP_WNDPROC, (INT_PTR) &ReplicationListProc);

		// Create Listview columns
        MyLVColumb.cx        = 150;
        MyLVColumb.pszText   = "Database";
        MyLVColumb.iSubItem  = REP_COLUMN_DATABASE;
        MyLVColumb.mask      = LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
        MyLVColumb.fmt       = LVCFMT_LEFT;
        ListView_InsertColumn(hTablesList, REP_COLUMN_DATABASE, &MyLVColumb);

        MyLVColumb.cx        = 100;
        MyLVColumb.pszText   = "Schema";
        MyLVColumb.iSubItem  = REP_COLUMN_SCHEMA;
        MyLVColumb.mask      = LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
        MyLVColumb.fmt       = LVCFMT_LEFT;
        ListView_InsertColumn(hTablesList, REP_COLUMN_SCHEMA, &MyLVColumb);

		MyLVColumb.cx        = 250;
        MyLVColumb.pszText   = "Table";
        MyLVColumb.iSubItem  = REP_COLUMN_TABLE;
        MyLVColumb.mask      = LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
        MyLVColumb.fmt       = LVCFMT_LEFT;
        ListView_InsertColumn(hTablesList, REP_COLUMN_TABLE, &MyLVColumb);

        MyLVColumb.cx        = 0;
        MyLVColumb.pszText   = "Primary Key";
        MyLVColumb.iSubItem  = REP_COLUMN_PRIMARYKEY;
        MyLVColumb.mask      = LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
        MyLVColumb.fmt       = LVCFMT_LEFT;
        ListView_InsertColumn(hTablesList, REP_COLUMN_PRIMARYKEY, &MyLVColumb);

        MyLVColumb.cx        = 300;
        MyLVColumb.pszText   = "Status";
        MyLVColumb.iSubItem  = REP_COLUMN_STATUS;
        MyLVColumb.mask      = LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
        MyLVColumb.fmt       = LVCFMT_LEFT;
        ListView_InsertColumn(hTablesList, REP_COLUMN_STATUS, &MyLVColumb);

		hImgList = ImageList_Create(GetSystemMetrics(SM_CXSMICON), GetSystemMetrics(SM_CYSMICON), ILC_MASK|ILC_COLOR32, 1, 1);

		HICON hIcon = NULL;
		
        hIcon = LoadIcon(ghAppInstance, MAKEINTRESOURCE(IDI_AVAILFORREP));
        ImageList_AddIcon(hImgList, (HICON)hIcon);
        DeleteObject((HICON)hIcon);

        hIcon = LoadIcon(ghAppInstance, MAKEINTRESOURCE(IDI_NOTAVAILFORREP));
        ImageList_AddIcon(hImgList, (HICON)hIcon);
        DeleteObject((HICON)hIcon);

        hIcon = LoadIcon(ghAppInstance, MAKEINTRESOURCE(IDI_REPLICATING));
        ImageList_AddIcon(hImgList, (HICON)hIcon);
        DeleteObject((HICON)hIcon);

        hIcon = LoadIcon(ghAppInstance, MAKEINTRESOURCE(IDI_REPERROR));
        ImageList_AddIcon(hImgList, (HICON)hIcon);
        DeleteObject((HICON)hIcon);

        ListView_SetImageList(hTablesList, hImgList, LVSIL_SMALL);

		CenterOverOwner(hWnd);

        return TRUE;
    }

    //--------------------------------------------------------------------------

    else if(uMsg == WM_ACTIVATE)
    {
		if(bFirstActivate)
		{
			bFirstActivate = false;
			hReplicationThread =
				CreateThread(NULL, 0, PopTablesGrid_Thread, (LPVOID)0, 0, &dwReplicationThreadID);
		}

		return FALSE;
	}

    //--------------------------------------------------------------------------

    else if(uMsg == WM_COMMAND)
    {
		if(wParam == IDC_FIND)
        {
			char sText[1024];
			Get_Text(hFindText, sText, sizeof(sText));

			bool bFindNext = (_strcmpi(sLastText, sText) == 0);
			Get_Text(hFindText, sLastText, sizeof(sLastText));

			LCase(sText);

			int iLastItem = 0;
			int iMatches = 0;

			int iItemPos = -1;
			while( (iItemPos = ListView_GetNextItem(hTablesList, iItemPos, LVNI_SELECTED)) != -1)
			{
				iLastItem = iItemPos;
				ListView_SetItemState(hTablesList, iItemPos, 0, LVIS_SELECTED);
			}

			if(bFindNext)
			{
				iItemPos = iLastItem;
			}
			else{
				iItemPos = -1;
			}

			while( (iItemPos = ListView_GetNextItem(hTablesList, iItemPos, LVNI_ALL)) != -1)
			{
				char sDatabase[2048];
				char sTable[2048];

				ListView_GetItemText(hTablesList, iItemPos, REP_COLUMN_DATABASE, sDatabase, sizeof(sDatabase));
				ListView_GetItemText(hTablesList, iItemPos, REP_COLUMN_TABLE, sTable, sizeof(sTable));

				LCase(sDatabase);
				LCase(sTable);

				int iDatabaseSz = (int)strlen(sDatabase);
				int iTableSz = (int)strlen(sTable);

				if(iDatabaseSz > 0)
				{
					if(InStr(sText, sDatabase, iDatabaseSz, 0) >= 0)
					{
						iMatches++;
					}
				}

				if(iTableSz > 0)
				{
					if(InStr(sText, sTable, iTableSz, 0) >= 0)
					{
						iMatches++;
					}
				}

				if(iMatches > 0)
				{
					ListView_SetItemState(hTablesList, iItemPos, LVIS_SELECTED, LVIS_SELECTED);
					ListView_EnsureVisible(hTablesList, iItemPos, FALSE);
					return TRUE;
				}
			}

			ListView_SetItemState(hTablesList, iLastItem, LVIS_SELECTED, LVIS_SELECTED);
			ListView_EnsureVisible(hTablesList, iLastItem, FALSE);

			MessageBox(hWnd, "Item not found.", gsTitleCaption, MB_ICONINFORMATION);

			return TRUE;
		}

		if(wParam == ID_REPLICATION_SELECTALL)
        {
			CheckGridRows(hTablesList, true);
			return TRUE;
		}

		if(wParam == ID_REPLICATION_DESELECTALL)
        {
			CheckGridRows(hTablesList, false);
			return TRUE;
		}

		if(wParam == ID_REPLICATION_SELECTALLOFTHISDATABASE)
        {
			CheckGridDatabaseRows(hTablesList, true);
			return TRUE;
		}

		if(wParam == ID_REPLICATION_DESELECTALLOFTHISDATABASE)
        {
			CheckGridDatabaseRows(hTablesList, false);
			return TRUE;
		}

		if(wParam == ID_REPLICATION_SELECTALLHIGHLIGHTED)
        {
			CheckGridHighlightedRows(hTablesList, true);
			return TRUE;
		}

		if(wParam == ID_REPLICATION_DESELECTALLHIGHLIGHTED)
        {
			CheckGridHighlightedRows(hTablesList, false);
			return TRUE;
		}

		if(wParam == ID_REPLICATION_CANCEL)
        {
			return TRUE;
		}

		if(wParam == ID_PENDINGTRANSACTIONS)
        {
			DialogBox(ghAppInstance, MAKEINTRESOURCE(IDD_PENDINGTRANS), hWnd, (DLGPROC)PendingTransDialog);
			return TRUE;
		}

		if(wParam == ID_CHKALL)
        {
			CheckGridRows(hTablesList, true);
			return TRUE;
		}

        if(wParam == ID_CHKNONE)
        {
			CheckGridRows(hTablesList, false);
			return TRUE;
		}

		if(wParam == ID_APPLY)
        {
			bProcessing = true;
			hReplicationThread =
				CreateThread(NULL, 0, ProcessTablesGrid_Thread, (LPVOID)0, 0, &dwReplicationThreadID);

			int iLVStyle = LVS_EX_GRIDLINES | LVS_EX_FULLROWSELECT;
			SendMessage(hTablesList, (UINT)LVM_SETEXTENDEDLISTVIEWSTYLE, (WPARAM)0, (LPARAM)iLVStyle);

			return TRUE;
        }

        if(wParam == ID_CANCEL)
        {
			bReplicationDLGopen = false;
			EndDialog(hWnd,0);
			DestroyWindow(hWnd);
			return TRUE;
        }
		
		return FALSE;
    }

	//--------------------------------------------------------------------------

	else if(uMsg == WM_NOTIFY)
    {
        LPNMHDR lpnmhdr = (LPNMHDR)lParam;

		if(lpnmhdr->hwndFrom == hTablesList)
        {
            if(lpnmhdr->code == LVN_COLUMNCLICK)
			{
                NMLISTVIEW *pNMLV = (NMLISTVIEW *)lParam;

	            SORTDATA SortData;
                memset(&SortData, 0, sizeof(SortData));

                if(iSortCol == pNMLV->iSubItem)
                {
                    if(iSortUpOrDown == SORT_UP)
					{
                        iSortUpOrDown = SORT_DOWN;
					}
                    else iSortUpOrDown = SORT_UP;
                }
                else {
                    iSortCol = pNMLV->iSubItem;
                    iSortUpOrDown = SORT_UP;
                }

                SortData.hwndList = hTablesList;
                SortData.isortSubItem = iSortCol;
                SortData.iUPDOWN = iSortUpOrDown;
                SortData.iColType = COL_STRING;

                ListView_SortItemsEx(hTablesList, CompListEx, (LPARAM)&SortData);

				int SelectedIndex = ListView_GetNextItem(hTablesList, -1, LVNI_SELECTED);
				if(SelectedIndex >= 0)
				{
					ListView_EnsureVisible(hTablesList, SelectedIndex, FALSE);
				}
			}
		}

		return TRUE;
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

    else if(uMsg == WM_CLOSE) 
    {
        if(bProcessing)
		{
			MessageBox(hWnd, "Please wait until the process is complete.", gsTitleCaption, NULL);
		}
		else{
			bReplicationDLGopen = false;
			EndDialog(hWnd,0);
			DestroyWindow(hWnd);
		}
        return TRUE;
    }

    //--------------------------------------------------------------------------

    return FALSE;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void AddReplicateEditControlProc(HWND hOwner, HWND hWnd)
{
	WNDPROC gpOldProc = (WNDPROC) SetWindowLongPtr(hWnd, GWLP_WNDPROC, (INT_PTR) &ReplicateEditControlProc);
	SendMessage(hWnd, INIT_CUSTOM_PROC, (WPARAM)&hOwner, (LPARAM) &gpOldProc);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

LRESULT CALLBACK ReplicateEditControlProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	static WNDPROC lpOldProc;
	static HWND hOwner;

    if(uMsg == WM_GETDLGCODE)
	{
		return (DLGC_WANTALLKEYS | CallWindowProc(lpOldProc, hWnd, uMsg, wParam, lParam));
	}
	else if(uMsg == INIT_CUSTOM_PROC)
	{
		memcpy(&lpOldProc, (WNDPROC *)lParam, sizeof(WNDPROC));
		memcpy(&hOwner, (WNDPROC *)wParam, sizeof(HWND));
	}
	else if(uMsg == WM_CHAR)
	{
		if(wParam == VK_RETURN)
		{
			return 0;
		}
		else if(wParam == VK_TAB)
		{
			return 0;
		}
	}
	else if(uMsg == WM_KEYDOWN)
	{
		if(wParam == VK_TAB)
		{
			PostMessage(hOwner, WM_NEXTDLGCTL, 0, 0L);
			return 0;
		}
		else if(wParam == VK_RETURN)
		{
			SendMessage(hOwner, WM_COMMAND, IDC_FIND, 0);
		}
	}

	if(lpOldProc)
	{
		return CallWindowProc(lpOldProc, hWnd, uMsg, wParam, lParam);
	}
	else{
		return 0;
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

LRESULT CALLBACK ReplicationListProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	LRESULT lResult = CallWindowProc(gpOldReplicationListProc, hWnd, uMsg, wParam, lParam);

	if(uMsg == WM_RBUTTONDOWN)
    {
		int iSelected =  ListView_GetSelectedCount(hWnd);

		// Load menu
    	HMENU hMenu = LoadMenu(ghAppInstance, MAKEINTRESOURCE(IDR_REPLICATIONLIST) );
   		hMenu = GetSubMenu(hMenu, 0);

		if(iSelected > 1)
		{
			//EnableMenuItem(hMenu, ID_REPLICATION_SELECTALL, MF_ENABLED);
			//EnableMenuItem(hMenu, ID_REPLICATION_DESELECTALL, MF_ENABLED);
			//EnableMenuItem(hMenu, ID_REPLICATION_SELECTALLOFTHISDATABASE, MF_ENABLED);
			//EnableMenuItem(hMenu, ID_REPLICATION_DESELECTALLOFTHISDATABASE, MF_ENABLED);
			EnableMenuItem(hMenu, ID_REPLICATION_SELECTALLHIGHLIGHTED, MF_ENABLED);
			EnableMenuItem(hMenu, ID_REPLICATION_DESELECTALLHIGHLIGHTED, MF_ENABLED);
			EnableMenuItem(hMenu, ID_REPLICATION_CANCEL, MF_ENABLED);
		}
		else{
			EnableMenuItem(hMenu, ID_REPLICATION_SELECTALL, MF_ENABLED);
			EnableMenuItem(hMenu, ID_REPLICATION_DESELECTALL, MF_ENABLED);
			EnableMenuItem(hMenu, ID_REPLICATION_SELECTALLOFTHISDATABASE, MF_ENABLED);
			EnableMenuItem(hMenu, ID_REPLICATION_DESELECTALLOFTHISDATABASE, MF_ENABLED);
			//EnableMenuItem(hMenu, ID_REPLICATION_SELECTALLHIGHLIGHTED, MF_ENABLED);
			//EnableMenuItem(hMenu, ID_REPLICATION_DESELECTALLHIGHLIGHTED, MF_ENABLED);
			EnableMenuItem(hMenu, ID_REPLICATION_CANCEL, MF_ENABLED);
		}

        POINT xy;
        GetCursorPos(&xy);
        TrackPopupMenuEx(hMenu, TPM_LEFTALIGN|TPM_RIGHTBUTTON, (xy.x) + 1, xy.y, hReplication, NULL);
        DestroyMenu(hMenu);
    }

    return lResult;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CheckGridHighlightedRows(HWND hGrid, bool bCheck)
{
	int iItem = -1;

	while((iItem = ListView_GetNextItem(hGrid, iItem, LVNI_SELECTED)) != -1)
	{
		ListView_SetCheckState(hGrid, iItem, (BOOL) bCheck);
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CheckGridDatabaseRows(HWND hGrid, bool bCheck)
{
	char sDatabase[1024];
	char sText[1024];
	int iItem = -1;

	iItem = ListView_GetNextItem(hGrid, iItem, LVNI_SELECTED);

	if(iItem != -1)
	{
		ListView_GetItemText(hGrid, iItem, 0, sDatabase, sizeof(sDatabase));

		iItem = -1;
		
		while((iItem = ListView_GetNextItem(hGrid, iItem, LVNI_ALL)) != -1)
		{
			ListView_GetItemText(hGrid, iItem, 0, sText, sizeof(sText));

			if(_strcmpi(sDatabase, sText) == 0)
			{
				ListView_SetCheckState(hGrid, iItem, (BOOL) bCheck);
			}
			
		}
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CheckGridRows(HWND hGrid, bool bCheck)
{
	int iRow = 0;
	int iItems = ListView_GetItemCount(hGrid);

	while(iRow < iItems)
    {
		ListView_SetCheckState(hGrid, iRow, (BOOL) bCheck);
		iRow++;
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

DWORD WINAPI PopTablesGrid_Thread(LPVOID lpData)
{
	CStatusDlg MyStatDlg;
	MyStatDlg.Load(hReplication, "Please wait...", IDI_MAIN, false);
	MyStatDlg.Show();

	PopTablesGrid(hTablesList, &MyStatDlg);

	MyStatDlg.SetButtonText("Ok");
	MyStatDlg.EnableButton();

	while(MyStatDlg.IsDlgVisible())
	{
		Sleep(1);
	}

	MyStatDlg.Unload();

	SetForegroundWindow(hReplication);

	return 0;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool PopTablesGrid(HWND hGrid, CStatusDlg *lpStatDlg)
{
	int iRepTriggers = 0;
	int iRow = 0;
	int iItem = 0;
	int iTempSz = 0;
	int iChecked = 0;
	int iImage = 0;
	bool bGotPK = true;

	char sDatabase[255];
	char sTemp[100];
	char sSQL[5120];

	CSQL cSQL;
	CBoundRecordSet rsDatabases;
	CBoundRecordSet rsTemp;

	LV_ITEM MyLVItem;
	memset(&MyLVItem, 0, sizeof(MyLVItem));

    int iLVStyle = LVS_EX_GRIDLINES | LVS_EX_FULLROWSELECT | LVS_EX_CHECKBOXES;
    SendMessage(hGrid, (UINT)LVM_SETEXTENDEDLISTVIEWSTYLE, (WPARAM)0, (LPARAM)iLVStyle);

	if(!ConnectToSQLServer(&cSQL, gsSQLServer, gsSQLUserID, gsSQLPassword, "master",
		gbUseTCPIPConnection, gbUseTrustedConnection, gdwSQLServerPort))
	{
		lpStatDlg->SetText("Failed to connect to the SQL server.");
		return false;
	}

	strcpy_s(sSQL, sizeof(sSQL), 
		"SELECT"
		"	SD.[name]"
		" FROM"
		"	[sysdatabases] AS SD"
		" WHERE"
		"	SD.[name] NOT LIKE 'SQLExch_%'"
		"	AND SD.[name] NOT IN ('master', 'model', 'msdb', 'tempdb')"
		" ORDER BY"
		"	SD.[name]");
	cSQL.Execute(sSQL, &rsDatabases);	

	while(rsDatabases.Fetch())
	{
		rsDatabases.Values("name")->ToString(sDatabase, sizeof(sDatabase));

		sprintf_s(sTemp, sizeof(sTemp), "Retrieving SQL Resources from %s", sDatabase);
		lpStatDlg->SetText(sTemp);

		sprintf_s(sSQL, sizeof(sSQL),
			"SELECT"
			"	T.TABLE_CATALOG AS [Database],"
			"	T.TABLE_SCHEMA AS [Schema],"
			"	T.TABLE_NAME AS [Table],"
			"	TC.CONSTRAINT_NAME AS [PrimaryKey],"
			"	OBJECT_ID(T.TABLE_CATALOG + '.' + T.TABLE_SCHEMA + '.' + T.TABLE_NAME) AS [ObjectID],"
			"	("
			"		SELECT"
			"			COUNT(0) AS [Records]"
			"		FROM"
			"			[%s].[%s].sysobjects AS TR"
			"		WHERE"
			"			TR.xtype = 'TR'"
			"			AND TR.parent_obj = OBJECT_ID(T.TABLE_CATALOG + '.' + T.TABLE_SCHEMA + '.' + T.TABLE_NAME)"
			"			AND TR.name LIKE 'SQLExch[_]%%'"
			"	) AS [SQLExchTriggers],"
			"	("
			"		SELECT"
			"			COUNT(0) AS [Records]"
			"		FROM"
			"			[%s].[%s].sysobjects AS TR"
			"		WHERE"
			"			TR.xtype = 'TR'"
			"			AND TR.parent_obj = OBJECT_ID(T.TABLE_CATALOG + '.' + T.TABLE_SCHEMA + '.' + T.TABLE_NAME)"
			"			AND TR.name LIKE 'SQLExch[_]' + T.TABLE_CATALOG + '[_]' + T.TABLE_SCHEMA + '[_]' + T.TABLE_NAME + '[_]%%'"
			"	) AS [ValidSQLExchTriggers]"
			" FROM"
			"	[%s].INFORMATION_SCHEMA.TABLES AS T"
			" LEFT OUTER JOIN [%s].INFORMATION_SCHEMA.TABLE_CONSTRAINTS AS TC"
			"	ON TC.TABLE_CATALOG = T.TABLE_CATALOG"
			"	AND TC.TABLE_SCHEMA = T.TABLE_SCHEMA"
			"	AND TC.TABLE_NAME = T.TABLE_NAME"
			"	AND TC.CONSTRAINT_TYPE = 'PRIMARY KEY'"
			" WHERE"
			"	T.TABLE_TYPE = 'BASE TABLE'",
			sDatabase, gsDefaultDBO, sDatabase, gsDefaultDBO, sDatabase, sDatabase);

		cSQL.ThrowErrors(false);
	
		if(cSQL.Execute(sSQL, &rsTemp))
		{
			iRow = 0;

			lpStatDlg->SetProgressRange(0, (int)rsTemp.RowCount);

			sprintf_s(sTemp, sizeof(sTemp), "Loading SQL Resources from %s", sDatabase);
			lpStatDlg->SetText(sTemp);

			while(rsTemp.Fetch())
			{
				MyLVItem.pszText  = rsTemp.Values("Database")->ToString();
				MyLVItem.mask     = LVIF_TEXT;
				MyLVItem.iSubItem = REP_COLUMN_DATABASE;
				MyLVItem.iItem    = iItem;
				ListView_InsertItem(hGrid, &MyLVItem);

				MyLVItem.pszText  = rsTemp.Values("Schema")->ToString();
				MyLVItem.mask     = LVIF_TEXT;
				MyLVItem.iSubItem = REP_COLUMN_SCHEMA;
				MyLVItem.iItem    = iItem;
				ListView_SetItem(hGrid, &MyLVItem);

				MyLVItem.pszText  = rsTemp.Values("Table")->ToString();
				MyLVItem.mask     = LVIF_TEXT;
				MyLVItem.iSubItem = REP_COLUMN_TABLE;
				MyLVItem.iItem    = iItem;
				ListView_SetItem(hGrid, &MyLVItem);

				MyLVItem.pszText  = rsTemp.Values("PrimaryKey")->ToString();
				MyLVItem.mask     = LVIF_TEXT;
				MyLVItem.iSubItem = REP_COLUMN_PRIMARYKEY;
				MyLVItem.iItem    = iItem;
				ListView_SetItem(hGrid, &MyLVItem);

				if(rsTemp.Values("PrimaryKey")->Size() > 0)
				{
					bGotPK = true;
				}
				else {
					bGotPK = false;
				}

				//Number of replication triggers.
				if((iRepTriggers = rsTemp.Values("SQLExchTriggers")->ToIntegerS()) > 0)
				{
					ListView_SetCheckState(hGrid, iItem, TRUE);
					iChecked = 1;
				}
				else{
					ListView_SetCheckState(hGrid, iItem, FALSE);
					iChecked = 0;
				}

				//Number of valid replication triggers
				if(iRepTriggers != rsTemp.Values("ValidSQLExchTriggers")->ToIntegerS())
				{
					ListView_SetCheckState(hGrid, iItem, FALSE);
					iChecked = 0;

					strcpy_s(sTemp, sizeof(sTemp), "This replication is invalid and should be dropped.");
					iImage = 3;
				}
				else{
					if(iChecked == 1) {
						strcpy_s(sTemp, sizeof(sTemp), "Replicating.");
						iImage = 2;
					}
					else if(bGotPK) {
						strcpy_s(sTemp, sizeof(sTemp), "Not Replicating.");
						iImage = 0;
					}
					else{
						strcpy_s(sTemp, sizeof(sTemp), "Cannot Replicate because no Primary Key exists.");
						iImage = 1;
					}
				}

				MyLVItem.mask = LVIF_PARAM|LVIF_IMAGE;
				MyLVItem.iSubItem = REP_COLUMN_DATABASE;
				MyLVItem.iImage   = iImage;
				MyLVItem.lParam   = iImage;
				MyLVItem.iItem    = iItem;
				ListView_SetItem(hGrid, &MyLVItem);

				MyLVItem.pszText  = sTemp;
				MyLVItem.mask     = LVIF_TEXT;
				MyLVItem.iSubItem = REP_COLUMN_STATUS;
				MyLVItem.iItem    = iItem;
				ListView_SetItem(hGrid, &MyLVItem);

				iItem++;
				iRow++;

				lpStatDlg->SetProgressPos(iRow);
				//Sleep(1);
			}
			rsTemp.Close();
		}
		else{
			sprintf_s(sTemp, sizeof(sTemp), "Failed to load resource list from [%s]", sDatabase);
			MessageBox(lpStatDlg->Hwnd(), sTemp, gsTitleCaption, MB_ICONWARNING);
		}
	}

	rsDatabases.Close();

	lpStatDlg->SetText("Resource load is complete!");
	lpStatDlg->SetCaption("Complete.");

	cSQL.Disconnect();

    SORTDATA SortData;
    memset(&SortData, 0, sizeof(SortData));
    SortData.hwndList = hTablesList;
    SortData.isortSubItem = 0;
    SortData.iUPDOWN = SORT_UP;
    SortData.iColType = COL_STRING;

    ListView_SortItemsEx(hGrid, CompListEx, (LPARAM)&SortData);

	LVAutoSizeColumns(hGrid);

	return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

DWORD WINAPI ProcessTablesGrid_Thread(LPVOID lpData)
{
	HWND hCmdApply = GetDlgItem(hReplication, ID_APPLY);
	HWND hCmdClose = GetDlgItem(hReplication, ID_CANCEL);
	HWND hCmdAll   = GetDlgItem(hReplication, ID_CHKALL);
	HWND hCmdNone  = GetDlgItem(hReplication, ID_CHKNONE);

	EnableWindow(hCmdApply, FALSE);
	EnableWindow(hCmdClose, FALSE);
	EnableWindow(hCmdAll, FALSE);
	EnableWindow(hCmdNone, FALSE);

	CStatusDlg MyStatDlg;
	MyStatDlg.Load(hReplication, "Creating replications.", IDI_MAIN, false);
	MyStatDlg.Show();

	ProcessTablesGrid(hTablesList, &MyStatDlg);

	MyStatDlg.SetButtonText("Ok");
	MyStatDlg.EnableButton();

	while(MyStatDlg.IsDlgVisible())
	{
		Sleep(1);
	}

	MyStatDlg.Unload();

	EnableWindow(hCmdClose, TRUE);

	bProcessing = false;

	return 0;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool ProcessTablesGrid(HWND hGrid, CStatusDlg *lpStatDlg)
{
    int iRow = 0;
	LPARAM lParam = 0;
	int iTempSz = 0;
	int iItems = ListView_GetItemCount(hGrid);
	char sLastDB[255];

	char sDatabase[256];
	char sSchema[256];
	char sTable[256];

	CSQL cSQL;
	CBoundRecordSet rsDatabases;

	memset(sLastDB, 0, sizeof(sLastDB));

	lpStatDlg->SetProgressRange(0, iItems);

	Sleep(1500);

	lpStatDlg->SetText("Connecting to the SQL server.");

	if(!ConnectToSQLServer(&cSQL, gsSQLServer, gsSQLUserID, gsSQLPassword, "master",
		gbUseTCPIPConnection, gbUseTrustedConnection, gdwSQLServerPort))
	{
		lpStatDlg->SetText("Failed to connect to the SQL server.");
		return false;
	}

	if(!CreateReplicationDatabaseAndTable(&cSQL, NULL))
	{
		lpStatDlg->SetText("The replication creation process failed.");
		cSQL.Disconnect();
		return false;
	}

	while(iRow < iItems)
    {
		ListView_GetItemText(hGrid, iRow, REP_COLUMN_DATABASE, sDatabase, sizeof(sDatabase));
		ListView_GetItemText(hGrid, iRow, REP_COLUMN_SCHEMA, sSchema, sizeof(sSchema));
		ListView_GetItemText(hGrid, iRow, REP_COLUMN_TABLE, sTable, sizeof(sTable));

		if(_strcmpi(sDatabase, sLastDB) != 0)
		{
			lpStatDlg->SetText("Changing SQL connection focus.");
			cSQL.Focus(sDatabase);
			lpStatDlg->SetText("Updating replication settings.");
			strcpy_s(sLastDB, sizeof(sLastDB), sDatabase);
		}

		/*
			lParam:
				0: Not (But Can) replicating.
				1: Cannot replicate.
				2: Currently replicating.
				3: Currently replicating.
					(Trigger name does not match database name, Bad replication!)
		*/
		lParam = GetListItemlParam(hGrid, iRow, 0);

		//If the item is checked.
		if(ListView_GetCheckState(hGrid, iRow))
		{
			//If the item was previously unchecked.
			if(lParam == 0)
			{
				if(CreateReplicationEx(&cSQL, sDatabase, sSchema, sTable, hGrid, REP_COLUMN_STATUS, iRow))
				{
					SetStatus(hGrid, REP_COLUMN_STATUS, iRow, "Create OK.");
				}
				else SetStatus(hGrid, REP_COLUMN_STATUS, iRow, "Create Failed!");
			}
		}
		else { //If the item is unchecked.

			//If the item was previously checked.
			if(lParam == 2 || lParam == 3)
			{
				SetStatus(hGrid, REP_COLUMN_STATUS, iRow, "Dropping Triggers.");
				if(DropReplicationTriggers(&cSQL, sDatabase, sSchema, sTable))
				{
					SetStatus(hGrid, REP_COLUMN_STATUS, iRow, "Dropping tables.");
					if(DropReplicationTables(&cSQL, sDatabase, sSchema, sTable))
					{
						SetStatus(hGrid, REP_COLUMN_STATUS, iRow, "Drop OK.");
					}
					else SetStatus(hGrid, REP_COLUMN_STATUS, iRow, "Drop Failed!");
				}
				else SetStatus(hGrid, REP_COLUMN_STATUS, iRow, "Drop Failed!");
			}
		}

        iRow++;

		lpStatDlg->SetProgressPos(iRow);
		//Sleep(1);
    }

	GenerateReplicationScripts(&cSQL, lpStatDlg);

	if(strlen(sLastDB) > 0)
	{
		cSQL.Disconnect();
	}

	lpStatDlg->SetText("Complete.");

	return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#endif
