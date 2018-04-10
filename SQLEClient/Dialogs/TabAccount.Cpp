///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//  Copyright © NetworkDLS 2002, All rights reserved
//
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF 
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO 
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A 
// PARTICULAR PURPOSE.
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef _DIALOGS_TABACCOUNT_CPP
#define _DIALOGS_TABACCOUNT_CPP
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include <Windows.H>
#include <PrSht.H>
#include <Stdio.H>
#include <Stdlib.H>
#include <CommCtrl.h>

#include "../Source/Entry.H"

#include "../Dialogs/MainDlg.H"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

using namespace NSWFL::Windows;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

BOOL CALLBACK TabAccountDialog(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    static HWND hCompanyName     = NULL;
    static HWND hCompanyPwd      = NULL;
    static HWND hServerAddress   = NULL;
    static HWND hServerPort      = NULL;

    if(uMsg == WM_INITDIALOG)
    {
        hCompanyName     = GetDlgItem(hWnd, IDC_COMPANYNAME);
        hCompanyPwd      = GetDlgItem(hWnd, IDC_COMPANYPWD);

        hServerAddress   = GetDlgItem(hWnd, IDC_SERVERADDRESS);
        hServerPort      = GetDlgItem(hWnd, IDC_SERVERPORT);

        Set_Text(hCompanyName, gsCompanyName);
        Set_Text(hCompanyPwd, gsCompanyPassword);

        Set_Text(hServerAddress, gsServerAddress);
        Set_Long(hServerPort, gdwServerPort);

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
            char lsCompanyName[sizeof(gsCompanyName)];
            int liCompanyNameLen = Get_Text(hCompanyName, lsCompanyName, sizeof(lsCompanyName));
            if(liCompanyNameLen == 0 || liCompanyNameLen >= sizeof(lsCompanyName))
            {
                MessageBox(hWnd, "You must enter a valid company name. Between 1 and 64 characters.", gsTitleCaption, MB_ICONASTERISK);
                SetWindowLongPtr(hWnd, DWLP_MSGRESULT, PSNRET_INVALID_NOCHANGEPAGE); // Prevent window from closing
                return TRUE;
            }

            char lsCompanyPwd[sizeof(gsCompanyPassword)];
            int liCompanyPwdLen = Get_Text(hCompanyPwd, lsCompanyPwd, sizeof(lsCompanyPwd));
            if(liCompanyPwdLen == 0 || liCompanyPwdLen >= sizeof(lsCompanyPwd))
            {
                MessageBox(hWnd, "You must enter a valid company password. Between 1 and 64 characters.", gsTitleCaption, MB_ICONASTERISK);
                SetWindowLongPtr(hWnd, DWLP_MSGRESULT, PSNRET_INVALID_NOCHANGEPAGE); // Prevent window from closing
                return TRUE;
            }

            char lsServerAddress[sizeof(gsServerAddress)];
            int liServerAddressLen = Get_Text(hServerAddress, lsServerAddress, sizeof(lsServerAddress));
            if(liServerAddressLen == 0 || liServerAddressLen >= sizeof(lsServerAddress))
            {
                MessageBox(hWnd, "You must enter a valid server address. Between 1 and 255 characters.", gsTitleCaption, MB_ICONASTERISK);
                SetWindowLongPtr(hWnd, DWLP_MSGRESULT, PSNRET_INVALID_NOCHANGEPAGE); // Prevent window from closing
                return TRUE;
            }

            int liServerPort = Get_Long(hServerPort);
            if(liServerPort == 0 || liServerPort > 65535)
            {
                MessageBox(hWnd, "You must enter a valid server port. Between 1 and 65,535.", gsTitleCaption, MB_ICONASTERISK);
                SetWindowLongPtr(hWnd, DWLP_MSGRESULT, PSNRET_INVALID_NOCHANGEPAGE); // Prevent window from closing
                return TRUE;
            }

            strcpy_s(gsCompanyName, sizeof(lsCompanyName), lsCompanyName);
            strcpy_s(gsCompanyPassword, sizeof(gsCompanyPassword), lsCompanyPwd);
            strcpy_s(gsServerAddress, sizeof(gsServerAddress), lsServerAddress);

            gdwServerPort = liServerPort;

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
