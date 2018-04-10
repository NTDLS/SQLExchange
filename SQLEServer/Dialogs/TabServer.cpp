///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//  Copyright © NetworkDLS 2002, All rights reserved
//
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF 
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO 
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A 
// PARTICULAR PURPOSE.
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef _TABSERVER_H
#define _TABSERVER_H
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

INT_PTR CALLBACK TabServerDialog(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    static HWND hListenPort;
    static HWND hMaxClients;
    static HWND hNextClientID;
    static HWND hCompressMethod;

    if(uMsg == WM_INITDIALOG)
    {
        hListenPort     = GetDlgItem(hWnd, IDC_LISTENPORT);
        hMaxClients     = GetDlgItem(hWnd, IDC_MAXCONNECTS);
		hNextClientID   = GetDlgItem(hWnd, IDD_NEXTCLIENTID);
		hCompressMethod = GetDlgItem(hWnd, IDC_COMPRESSMETHOD);

		Set_Long(hListenPort, gdwListenPort);
        Set_Long(hMaxClients, gdwMaxClients);
        Set_Long(hNextClientID, gdwNextClientID);

        SendMessage(hCompressMethod, (UINT)CB_ADDSTRING, (WPARAM)0, (LPARAM)"None");
		SendMessage(hCompressMethod, (UINT)CB_ADDSTRING, (WPARAM)0, (LPARAM)"GZIP");
		SendMessage(hCompressMethod, (UINT)CB_ADDSTRING, (WPARAM)0, (LPARAM)"LZSS");
        SendMessage(hCompressMethod, (UINT)CB_ADDSTRING, (WPARAM)0, (LPARAM)"LZARI");
        SendMessage(hCompressMethod, (UINT)CB_ADDSTRING, (WPARAM)0, (LPARAM)"RLE");

		int iIndex = (int)SendMessage(hCompressMethod, (UINT)CB_FINDSTRING, (WPARAM)-1, (LPARAM) gsCompressionMethod);
		SendMessage(hCompressMethod, (UINT)CB_SETCURSEL, (WPARAM)iIndex, (LPARAM)NULL);

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
            DWORD dwListenPort = Get_Long(hListenPort);
            if(dwListenPort == 0 || dwListenPort > 65535)
            {
                MessageBox(hWnd, "You must enter a valid server port. Between 1 and 65,535.", gsTitleCaption, MB_ICONASTERISK);
                SetWindowLongPtr(hWnd, DWLP_MSGRESULT, PSNRET_INVALID_NOCHANGEPAGE); // Prevent window from closing
                return TRUE;
            }
			
			DWORD dwMaxClients = Get_Long(hMaxClients);
            if(dwMaxClients == 0 || dwMaxClients > FD_SETSIZE)
            {
				char sText[255];
				sprintf_s(sText, sizeof(sText), "You must enter a valid number of server connections. Between 1 and %d.", FD_SETSIZE);
                MessageBox(hWnd, sText, gsTitleCaption, MB_ICONASTERISK);
                SetWindowLongPtr(hWnd, DWLP_MSGRESULT, PSNRET_INVALID_NOCHANGEPAGE); // Prevent window from closing
                return TRUE;
            }

			gdwMaxClients = dwMaxClients;
			gdwListenPort = dwListenPort;

			gdwNextClientID = Get_Long(hNextClientID);
			
			Get_Text(hCompressMethod, gsCompressionMethod, sizeof(gsCompressionMethod));
			if(_strcmpi(gsCompressionMethod, "GZIP") == 0){
				gdwCompressionLevel = 6;
			}
			else if(_strcmpi(gsCompressionMethod, "LZSS") == 0){
				gdwCompressionLevel = 0;
			}
			else if(_strcmpi(gsCompressionMethod, "LZARI") == 0){
				gdwCompressionLevel = 0; //Not used.
			}
			else if(_strcmpi(gsCompressionMethod, "RLE") == 0){
				gdwCompressionLevel = 0; //Not used.
			}

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
