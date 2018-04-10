///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//  Copyright © NetworkDLS 2002, All rights reserved
//
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF 
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO 
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A 
// PARTICULAR PURPOSE.
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef _TABSECURITY_H
#define _TABSECURITY_H
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include <Windows.H>
#include <Stdio.H>
#include <Stdlib.H>

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "../../../@Libraries/CCRC32/CCRC32.H"
#include "../../SharedSource/FileExchange.H"

#include "../Source/Entry.H"

#include "../Dialogs/MainDlg.H"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

using namespace NSWFL::Windows;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

INT_PTR CALLBACK TabSecurityDialog(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    static HWND hAuthString;
    static HWND hAuthKey;
    static HWND hEncryptKeyLength;
    static HWND hAdminPassword;
    static HWND hLocalCryptKey;

    if(uMsg == WM_INITDIALOG)
    {
        char sText[1024];
		int iLength = 0;

		hAuthString        = GetDlgItem(hWnd, IDC_AUTHSTRING);
        hAuthKey           = GetDlgItem(hWnd, IDC_AUTHKEY);
        hEncryptKeyLength  = GetDlgItem(hWnd, IDC_ENCRYPTKEYLEN);
        hAdminPassword     = GetDlgItem(hWnd, IDC_ADMINPASSWORD);
        hLocalCryptKey     = GetDlgItem(hWnd, IDC_LOCALCRYPTKEY);

        iLength = (int)strlen(gsAuthString);
		sprintf_s(sText, sizeof(sText), "Strength: %d bits, CRC: %X",
			iLength * 8, pCRC32->FullCRC((unsigned char *)gsAuthString, iLength));
		Set_Text(hAuthString, sText);

		iLength = (int)strlen(gsAuthKey);
		sprintf_s(sText, sizeof(sText), "Strength: %d bits, CRC: %X",
			iLength * 8, pCRC32->FullCRC((unsigned char *)gsAuthKey, iLength));
		Set_Text(hAuthKey, sText);

		sprintf_s(sText, sizeof(sText), "Strength: %d bits", gdwKeyGenLength * 8);
		Set_Text(hEncryptKeyLength, sText);

		iLength = (int)strlen(CRYPTKEY);
		sprintf_s(sText, sizeof(sText), "Strength: %d bits, CRC: %X",
			iLength * 8, pCRC32->FullCRC((unsigned char *)CRYPTKEY, iLength));
		Set_Text(hLocalCryptKey, sText);

		iLength = (int)strlen(gsGUIPWDHash);
		if(iLength > 0)
		{
			sprintf_s(sText, sizeof(sText), "Strength: %d bits, CRC: %X",
				iLength * 8, pCRC32->FullCRC((unsigned char *)gsGUIPWDHash, iLength));
			Set_Text(hAdminPassword, sText);
		}
		else{
			Set_Text(hAdminPassword, "Password not set");
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
