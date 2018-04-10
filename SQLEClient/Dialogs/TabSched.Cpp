///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//  Copyright © NetworkDLS 2002, All rights reserved
//
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF 
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO 
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A 
// PARTICULAR PURPOSE.
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef _DIALOGS_TABSCHED_CPP
#define _DIALOGS_TABSCHED_CPP
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include <Windows.H>
#include <CommCtrl.H>
#include <Stdio.H>
#include <Stdlib.H>

#include "../Source/Entry.H"

#include "../Dialogs/TabSched.H"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

using namespace NSWFL::Windows;
using namespace NSWFL::DateTime;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int GetTheNumberOfCheckedDays(HWND hWnd)
{
	int iChecked = 0;

	if(IsDlgButtonChecked(hWnd, ID_DAY_SUN) == BST_CHECKED) iChecked++;
	if(IsDlgButtonChecked(hWnd, ID_DAY_MON) == BST_CHECKED) iChecked++;
	if(IsDlgButtonChecked(hWnd, ID_DAY_TUE) == BST_CHECKED) iChecked++;
	if(IsDlgButtonChecked(hWnd, ID_DAY_WED) == BST_CHECKED) iChecked++;
	if(IsDlgButtonChecked(hWnd, ID_DAY_THU) == BST_CHECKED) iChecked++;
	if(IsDlgButtonChecked(hWnd, ID_DAY_FRI) == BST_CHECKED) iChecked++;
	if(IsDlgButtonChecked(hWnd, ID_DAY_SAT) == BST_CHECKED) iChecked++;

	return iChecked;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void LoadWeekDaysFromDialog(HWND hWnd)
{
	gbWeekDays[0] = (IsDlgButtonChecked(hWnd, ID_DAY_SUN) == BST_CHECKED);
	gbWeekDays[1] = (IsDlgButtonChecked(hWnd, ID_DAY_MON) == BST_CHECKED);
	gbWeekDays[2] = (IsDlgButtonChecked(hWnd, ID_DAY_TUE) == BST_CHECKED);
	gbWeekDays[3] = (IsDlgButtonChecked(hWnd, ID_DAY_WED) == BST_CHECKED);
	gbWeekDays[4] = (IsDlgButtonChecked(hWnd, ID_DAY_THU) == BST_CHECKED);
	gbWeekDays[5] = (IsDlgButtonChecked(hWnd, ID_DAY_FRI) == BST_CHECKED);
	gbWeekDays[6] = (IsDlgButtonChecked(hWnd, ID_DAY_SAT) == BST_CHECKED);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void LoadWeekDaysIntoDialog(HWND hWnd)
{
	if(gbWeekDays[0]) CheckDlgButton(hWnd, ID_DAY_SUN, BST_CHECKED);
	if(gbWeekDays[1]) CheckDlgButton(hWnd, ID_DAY_MON, BST_CHECKED);
	if(gbWeekDays[2]) CheckDlgButton(hWnd, ID_DAY_TUE, BST_CHECKED);
	if(gbWeekDays[3]) CheckDlgButton(hWnd, ID_DAY_WED, BST_CHECKED);
	if(gbWeekDays[4]) CheckDlgButton(hWnd, ID_DAY_THU, BST_CHECKED);
	if(gbWeekDays[5]) CheckDlgButton(hWnd, ID_DAY_FRI, BST_CHECKED);
	if(gbWeekDays[6]) CheckDlgButton(hWnd, ID_DAY_SAT, BST_CHECKED);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CheckAll(HWND hWnd, bool bCheck)
{
	int iDay = 0;
	int iState = 0;

	if(bCheck)
	{
		iState = BST_CHECKED;
	}
	else{
		iState = BST_UNCHECKED;
	}

	CheckDlgButton(hWnd, ID_DAY_SUN, iState);
	CheckDlgButton(hWnd, ID_DAY_MON, iState);
	CheckDlgButton(hWnd, ID_DAY_TUE, iState);
	CheckDlgButton(hWnd, ID_DAY_WED, iState);
	CheckDlgButton(hWnd, ID_DAY_THU, iState);
	CheckDlgButton(hWnd, ID_DAY_FRI, iState);
	CheckDlgButton(hWnd, ID_DAY_SAT, iState);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

BOOL CALLBACK TabSchedDialog(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    static HWND hTimeWindowBegin     = NULL;
    static HWND hTimeWindowEnd       = NULL;

	static HWND hHours = NULL;
	static HWND hMinutes = NULL;
	static HWND hHoursUD = NULL;
	static HWND hMinutesUD = NULL;

    if(uMsg == WM_INITDIALOG)
    {
        hTimeWindowBegin     = GetDlgItem(hWnd, ID_TIMEWINDBEGIN);
        hTimeWindowEnd       = GetDlgItem(hWnd, ID_TIMEWINDEND);

        hHours       = GetDlgItem(hWnd, ID_HOURS);
        hMinutes     = GetDlgItem(hWnd, ID_MINUTES);
        hHoursUD     = GetDlgItem(hWnd, ID_HOURUD);
        hMinutesUD   = GetDlgItem(hWnd, ID_MINUD);

		SendMessage(hHoursUD,(UINT)UDM_SETBUDDY, (WPARAM)hHours, (LPARAM)NULL);
		SendMessage(hHoursUD,(UINT)UDM_SETRANGE, (WPARAM)NULL,(LPARAM)MAKELONG(23, 0));

		SendMessage(hMinutesUD,(UINT)UDM_SETBUDDY, (WPARAM)hMinutes, (LPARAM)NULL);
		SendMessage(hMinutesUD,(UINT)UDM_SETRANGE, (WPARAM)NULL, (LPARAM)MAKELONG(59, 0));

		if(gdwConnectInterval > 0)
		{
			Set_Long(hHours, gdwConnectInterval / 60);
			Set_Long(hMinutes, gdwConnectInterval % 60);
		}
		else{
			Set_Long(hHours, 0);
			Set_Long(hMinutes, 0);
		}

		SYSTEMTIME MyST;

		GetLocalTime(&MyST);

		LongToTime(gdwStartTime, (int *)&MyST.wHour, (int *)&MyST.wMinute, (int *)&MyST.wSecond);
    	SendMessage(hTimeWindowBegin, MCM_SETCURSEL, (WPARAM) 0, (LPARAM) (LPSYSTEMTIME) &MyST);

		LongToTime(gdwStopTime, (int *)&MyST.wHour, (int *)&MyST.wMinute, (int *)&MyST.wSecond);
		SendMessage(hTimeWindowEnd, MCM_SETCURSEL, (WPARAM) 0, (LPARAM) (LPSYSTEMTIME) &MyST);

		LoadWeekDaysIntoDialog(hWnd);

        return TRUE;
    }

    if(uMsg == WM_COMMAND)
    {
		if(wParam == ID_SCHED_ALL)
		{
			CheckAll(hWnd, true);
		}

		if(wParam == ID_SCHED_NONE)
		{
			CheckAll(hWnd, false);
		}

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
			if(GetTheNumberOfCheckedDays(hWnd) == 0)
			{
				MessageBox(hWnd, "You must check at least one schedule day.", gsTitleCaption, MB_ICONASTERISK);
				SetWindowLongPtr(hWnd, DWLP_MSGRESULT, PSNRET_INVALID_NOCHANGEPAGE); // Prevent window from closing
				return TRUE;
			}

			SYSTEMTIME MyST;

			DWORD ldwHours       = Get_Long(hHours);
			DWORD ldwMinutes     = Get_Long(hMinutes);
			DWORD ldwConInterval = (ldwHours * 60) + ldwMinutes;
			DWORD ldwStartTime   = 0;
			DWORD ldwStopTime    = 0;

			GetLocalTime(&MyST);

    		SendMessage(hTimeWindowBegin, MCM_GETCURSEL, (WPARAM) 0, (LPARAM) (LPSYSTEMTIME) &MyST);
			ldwStartTime = TimeToLong(MyST.wHour, MyST.wMinute, MyST.wSecond);

    		SendMessage(hTimeWindowEnd, MCM_GETCURSEL, (WPARAM) 0, (LPARAM) (LPSYSTEMTIME) &MyST);
			ldwStopTime = TimeToLong(MyST.wHour, MyST.wMinute, MyST.wSecond);

			if(ldwConInterval == 0)
			{
				MessageBox(hWnd, "You must specify a connection interval.", gsTitleCaption, MB_ICONASTERISK);
				SetWindowLongPtr(hWnd, DWLP_MSGRESULT, PSNRET_INVALID_NOCHANGEPAGE); // Prevent window from closing
				return TRUE;
			}

			if(gdwStartTime > gdwStopTime)
			{
				MessageBox(hWnd, "The time window that you have selected is invalid.", gsTitleCaption, MB_ICONASTERISK);
				SetWindowLongPtr(hWnd, DWLP_MSGRESULT, PSNRET_INVALID_NOCHANGEPAGE); // Prevent window from closing
				return TRUE;
			}

			gdwStartTime = ldwStartTime;
			gdwStopTime = ldwStopTime;

			gdwConnectInterval = ldwConInterval;
			
			LoadWeekDaysFromDialog(hWnd);

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
