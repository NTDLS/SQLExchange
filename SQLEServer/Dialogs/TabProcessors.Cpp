#ifndef _DIALOGS_TABPROCESSORS_CPP
#define _DIALOGS_TABPROCESSORS_CPP
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include <WindowsX.H>
#include <Windows.H>
#include <Stdio.H>
#include <Stdlib.H>
#include <ShlOBJ.H>
#include <CommCtrl.h>

#include "../Source/Entry.H"

#include "../Dialogs/MainDlg.H"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

using namespace NSWFL::Windows;
using namespace NSWFL::Registry;
using namespace NSWFL::String;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

INT_PTR CALLBACK TabProcessorsDialog(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    static HWND hCPUs = NULL;

	if(uMsg == WM_INITDIALOG)
    {
		char sRegBuf[255];
		char sTemp[255];

		DWORD dwRegBufSz = 0;
		float fCPUSpeed = 0;

		LV_COLUMN lvMyLVColumb;
		LV_ITEM lvMyLVItem;
		HIMAGELIST hMyImageList;

		memset(&lvMyLVColumb, 0, sizeof(lvMyLVColumb));
		memset(&lvMyLVItem, 0, sizeof(lvMyLVItem));
		memset(&hMyImageList, 0, sizeof(hMyImageList));

		hCPUs = GetDlgItem(hWnd, IDC_CPUS);

		if(gbUseProcessBoost) CheckDlgButton(hWnd, IDC_PROCESSBOOST, BST_CHECKED);
		if(gbUseThreadBoost) CheckDlgButton(hWnd, IDC_THREADBOOST, BST_CHECKED);
		if(gbUseNTFibers) CheckDlgButton(hWnd, IDC_USENTFIBERS, BST_CHECKED);

		SYSTEM_INFO SI;
        GetSystemInfo(&SI);
		
		int iLVStyle = LVS_EX_GRIDLINES | LVS_EX_FULLROWSELECT | LVS_EX_CHECKBOXES;
        SendMessage(hCPUs, (UINT)LVM_SETEXTENDEDLISTVIEWSTYLE, (WPARAM)0, (LPARAM)iLVStyle);

        // Create ImageList begin
        hMyImageList = ImageList_Create(GetSystemMetrics(SM_CXSMICON), GetSystemMetrics(SM_CYSMICON), ILC_MASK|ILC_COLOR32, 1, 1);

        HICON hIcon = NULL;

        hIcon = LoadIcon(ghAppInstance, MAKEINTRESOURCE(IDI_SMCPU));
        ImageList_AddIcon(hMyImageList, (HICON)hIcon);
        DeleteObject((HICON)hIcon);

        ListView_SetImageList(hCPUs, hMyImageList, LVSIL_SMALL);

        // Create Listview columns
        lvMyLVColumb.cx        = 180;
        lvMyLVColumb.pszText   = "CPU";
        lvMyLVColumb.iSubItem  = 0;
        lvMyLVColumb.mask      = LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
        lvMyLVColumb.fmt       = LVCFMT_LEFT;
        ListView_InsertColumn(hCPUs, 0, &lvMyLVColumb);

        lvMyLVColumb.cx        = 45;
        lvMyLVColumb.pszText   = "Model";
        lvMyLVColumb.iSubItem  = 1;
        lvMyLVColumb.mask      = LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
        lvMyLVColumb.fmt       = LVCFMT_LEFT;
        ListView_InsertColumn(hCPUs, 1, &lvMyLVColumb);

        lvMyLVColumb.cx        = 55;
        lvMyLVColumb.pszText   = "Rev.";
        lvMyLVColumb.iSubItem  = 2;
        lvMyLVColumb.mask      = LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
        lvMyLVColumb.fmt       = LVCFMT_LEFT;
        ListView_InsertColumn(hCPUs, 2, &lvMyLVColumb);

        lvMyLVColumb.cx        = 65;
        lvMyLVColumb.pszText   = "Granularity";
        lvMyLVColumb.iSubItem  = 3;
        lvMyLVColumb.mask      = LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
        lvMyLVColumb.fmt       = LVCFMT_LEFT;
        ListView_InsertColumn(hCPUs, 3, &lvMyLVColumb);

		dwRegBufSz = sizeof(sRegBuf);
		memset(sRegBuf, 0, dwRegBufSz);
		if(Get_StringRegistryValue(HKEY_LOCAL_MACHINE, "HARDWARE\\DESCRIPTION\\System\\CentralProcessor\\0", "ProcessorNameString", sRegBuf, dwRegBufSz))
		{
			int iFirst = GetFirstCharPos(sRegBuf, dwRegBufSz);
			int iLast = GetLastCharPos(sRegBuf, dwRegBufSz);

			StringScan(sRegBuf, iFirst, iLast - iFirst, sTemp, sizeof(sTemp));

			Set_Text(GetDlgItem(hWnd, IDC_CPUDESCR), sTemp);
		}

		if(!Get_DWORDRegistryValue(HKEY_LOCAL_MACHINE, "HARDWARE\\DESCRIPTION\\System\\CentralProcessor\\0", "~MHz", dwRegBufSz))
			dwRegBufSz = 0;

		fCPUSpeed = (float)dwRegBufSz;

		DWORD dwItem = 0;
        while(dwItem < SI.dwNumberOfProcessors)
        {
			dwRegBufSz = sizeof(sRegBuf);
			memset(sRegBuf, 0, dwRegBufSz);
			if(!Get_StringRegistryValue(HKEY_LOCAL_MACHINE, "HARDWARE\\DESCRIPTION\\System\\CentralProcessor\\0", "VendorIdentifier", sRegBuf, dwRegBufSz))
				strcpy_s(sRegBuf, sizeof(sRegBuf), "Unkonwn");

			if(fCPUSpeed > 1000)
				sprintf_s(sTemp, sizeof(sTemp), "%s %.1f0 GHz", sRegBuf, fCPUSpeed / 1000.0f);
			else sprintf_s(sTemp, sizeof(sTemp), "%s %.2f MHz", sRegBuf, fCPUSpeed);
			
			lvMyLVItem.pszText = sTemp;
            lvMyLVItem.mask = LVIF_TEXT|LVIF_IMAGE;
            lvMyLVItem.iImage = 0;
            lvMyLVItem.iItem  = dwItem;
            lvMyLVItem.iSubItem = 0;
            ListView_InsertItem(hCPUs, &lvMyLVItem);

			sprintf_s(sTemp, sizeof(sTemp), "%d", HIWORD(SI.wProcessorRevision));
			lvMyLVItem.pszText = sTemp;
            lvMyLVItem.mask = LVIF_TEXT|LVIF_IMAGE;
            lvMyLVItem.iImage = 0;
            lvMyLVItem.iItem  = dwItem;
            lvMyLVItem.iSubItem = 1;
            ListView_SetItem(hCPUs, &lvMyLVItem);

			sprintf_s(sTemp, sizeof(sTemp), "%d", LOWORD(SI.wProcessorRevision));
            lvMyLVItem.pszText = sTemp;
            lvMyLVItem.mask = LVIF_TEXT|LVIF_IMAGE;
            lvMyLVItem.iImage = 0;
            lvMyLVItem.iItem  = dwItem;
            lvMyLVItem.iSubItem = 2;
            ListView_SetItem(hCPUs, &lvMyLVItem);

			sprintf_s(sTemp, sizeof(sTemp), "%0.2f K", ((float)SI.dwAllocationGranularity / 1024.0f));
            lvMyLVItem.pszText = sTemp;
            lvMyLVItem.mask = LVIF_TEXT|LVIF_IMAGE;
            lvMyLVItem.iImage = 0;
            lvMyLVItem.iItem  = dwItem;
            lvMyLVItem.iSubItem = 3;
            ListView_SetItem(hCPUs, &lvMyLVItem);

			if(IsBitSet(gdwCPUAffinityMask, dwItem))
			{
				ListView_SetCheckState(hCPUs, dwItem, TRUE);
			}
			else {
				ListView_SetCheckState(hCPUs, dwItem, FALSE);
			}

            dwItem++;
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
			DWORD_PTR dwCPUAffinityMask = 0;
			DWORD iItems = ListView_GetItemCount(hCPUs);
			DWORD dwItem = 0;

			while(dwItem < iItems)
			{
				if(ListView_GetCheckState(hCPUs, dwItem))
				{
					SetBit(dwCPUAffinityMask, dwItem);
				}

				dwItem++;
			}

			if(dwCPUAffinityMask == 0)
			{
                MessageBox(hWnd, "You must select at least one processor.", gsTitleCaption, MB_ICONASTERISK);
                SetWindowLongPtr(hWnd, DWLP_MSGRESULT, PSNRET_INVALID_NOCHANGEPAGE); // Prevent window from closing
                return TRUE;
			}

			if((IsDlgButtonChecked(hWnd, IDC_USENTFIBERS) == BST_CHECKED) != gbUseNTFibers)
			{
                int iResponse = MessageBox(hWnd,
					"Lightweight thread pooling (NT Fibers) is an advanced feature that allows"
					" SQL-Exchange to schedule it own threads."
					"\r\nThis feature should only be used if you have a specific need for Fibers over Threads."
					"\r\n\r\nNote: This application will shutdown if this option is changed and must be manually restarted."
				, "Use Lightweight thread pooling?", MB_YESNO | MB_ICONQUESTION);

				if(iResponse != IDYES)
				{
					SetWindowLongPtr(hWnd, DWLP_MSGRESULT, PSNRET_INVALID_NOCHANGEPAGE); // Prevent window from closing
	                return TRUE;
				}

				gbThreadFiberConfigChanged = true;
			}

			gbUseProcessBoost = (IsDlgButtonChecked(hWnd, IDC_PROCESSBOOST) == BST_CHECKED);
			gbUseThreadBoost = (IsDlgButtonChecked(hWnd, IDC_THREADBOOST) == BST_CHECKED);
			gbUseNTFibers = (IsDlgButtonChecked(hWnd, IDC_USENTFIBERS) == BST_CHECKED);

			//Save the mask.
			gdwCPUAffinityMask = dwCPUAffinityMask;

			//Apply settings.
			SetProcessAffinityMask(GetCurrentProcess(), gdwCPUAffinityMask);
			SetProcessPriorityBoost(GetCurrentProcess(), !gbUseProcessBoost);

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
