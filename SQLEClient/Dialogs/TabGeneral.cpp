///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//  Copyright � NetworkDLS 2002, All rights reserved
//
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF 
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO 
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A 
// PARTICULAR PURPOSE.
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef _TABGENERAL_CPP
#define _TABGENERAL_CPP
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include <Windows.H>
#include <Stdio.H>
#include <Stdlib.H>

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "../Source/Entry.H"
#include "../Dialogs/MainDlg.H"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

using namespace NSWFL::Windows;
using namespace NSWFL::System;
using namespace NSWFL::File;
using namespace NSWFL::DateTime;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

BOOL CALLBACK TabGeneralDialog(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    static HWND hProductText;
    static HWND hVersionText;
    static HWND hBuildDateText;
    static HWND hExpireDate;
	static HWND hOSText;
    static HWND hMemoryText;
    static HWND hProcesorsText;

    if(uMsg == WM_INITDIALOG)
    {
        hProductText     = GetDlgItem(hWnd, IDC_PRODUCTTEXT);
        hVersionText     = GetDlgItem(hWnd, IDC_VERSIONTEXT);
        hBuildDateText   = GetDlgItem(hWnd, IDC_BUILDDATETEXT);
        hExpireDate		 = GetDlgItem(hWnd, IDC_EXPIREDATE);
        hOSText          = GetDlgItem(hWnd, IDC_OSTEXT);
        hMemoryText      = GetDlgItem(hWnd, IDC_MEMORYTEXT);
        hProcesorsText   = GetDlgItem(hWnd, IDC_PROCESORSTEXT);

        //------------------------------------(Fill in general information begin)
        SYSTEM_INFO SI;
        GetSystemInfo(&SI);

        MEMORYSTATUSEX MS;
		memset(&MS, 0, sizeof(MS));
        MS.dwLength = sizeof(MS);
        GlobalMemoryStatusEx(&MS);

        char sTempText[1024];

        Set_Text(hProductText, gsTitleCaption);

        sprintf_s(sTempText, sizeof(sTempText), "%s at %s", __DATE__, __TIME__);
        Set_Text(hBuildDateText, sTempText);

        Set_Text(hVersionText, gsFileVersion);

        Get_OsVersion(sTempText, sizeof(sTempText));
        Set_Text(hOSText, sTempText);

        Set_Text(hMemoryText, FileSizeFriendly(MS.ullTotalPhys, 2, sTempText, sizeof(sTempText)));

        sprintf_s(sTempText, sizeof(sTempText), "%d", SI.dwNumberOfProcessors);
        Set_Text(hProcesorsText, sTempText);

		int iYear = 0;
		int iMonth = 0;
		int iDay = 0;

		LongToDate(dwExpiryDate, &iYear, &iMonth, &iDay);

		SYSTEMTIME ST;
		GetLocalTime(&ST);
		
		ST.wYear = iYear;
		ST.wMonth = iMonth;
		ST.wDay = iDay;

		GetDateFormat(LOCALE_USER_DEFAULT, DATE_LONGDATE, &ST, NULL, sTempText, sizeof(sTempText));
        Set_Text(hExpireDate, sTempText);
		// ------------------------------------(Fill in general information end)

        return TRUE;
    }

    if(uMsg == WM_COMMAND)
    {

        return FALSE;
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
