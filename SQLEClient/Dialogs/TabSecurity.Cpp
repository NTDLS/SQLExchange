///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//  Copyright © NetworkDLS 2002, All rights reserved
//
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF 
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO 
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A 
// PARTICULAR PURPOSE.
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef _DIALOGS_TABSECURITY_CPP
#define _DIALOGS_TABSECURITY_CPP
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include <Windows.H>
#include <PrSht.H>
#include <Stdio.H>
#include <Stdlib.H>
#include <CommCtrl.h>

#include "../../../@Libraries/CCRC32/CCRC32.H"

#include "../Source/Entry.H"

#include "../Dialogs/MainDlg.H"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

using namespace NSWFL::Windows;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

BOOL CALLBACK TabSecurityDialog(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    static HWND hAuthString     = NULL;
    static HWND hAuthKey        = NULL;
    static HWND hLocalCryptKey  = NULL;
    static HWND hAdminPassword  = NULL;

    if(uMsg == WM_INITDIALOG)
    {
        char sText[1024];
		int iLength = 0;

		hAuthString     = GetDlgItem(hWnd, IDC_AUTHSTRING);
        hAuthKey        = GetDlgItem(hWnd, IDC_AUTHKEY);

        hLocalCryptKey  = GetDlgItem(hWnd, IDC_LOCALCRYPTKEY);
        hAdminPassword  = GetDlgItem(hWnd, IDC_ADMINPASSWORD);
 
		CCRC32 *pCRC32 = new CCRC32();

        iLength = (int)strlen(gsAuthString);
		sprintf_s(sText, sizeof(sText), "Strength: %d bits, CRC: %X",
			iLength * 8, pCRC32->FullCRC((unsigned char *)gsAuthString, iLength));
		Set_Text(hAuthString, sText);

		iLength = (int)strlen(gsAuthKey);
		sprintf_s(sText, sizeof(sText), "Strength: %d bits, CRC: %X",
			iLength * 8, pCRC32->FullCRC((unsigned char *)gsAuthKey, iLength));
		Set_Text(hAuthKey, sText);

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

		delete pCRC32;

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
/*
			char lsAuthString[1024 + 1];
            int liAuthStringLen = Get_Text(hAuthString, lsAuthString, 1024);
            if(liAuthStringLen == 0 || liAuthStringLen > 1024)
            {
                MessageBox(hWnd, "You must enter a valid authentication string. Between 1 and 1024 characters.", gsTitleCaption, MB_ICONASTERISK);
                SetWindowLongPtr(hWnd, DWLP_MSGRESULT, PSNRET_INVALID_NOCHANGEPAGE); // Prevent window from closing
                return TRUE;
            }

            char lsAuthKey[1024 + 1];
            int liAuthKeyLen = Get_Text(hAuthKey, lsAuthKey, 1024);
            if(liAuthKeyLen == 0 || liAuthKeyLen > 1024)
            {
                MessageBox(hWnd, "You must enter a valid authentication key. Between 1 and 1024 characters.", gsTitleCaption, MB_ICONASTERISK);
                SetWindowLongPtr(hWnd, DWLP_MSGRESULT, PSNRET_INVALID_NOCHANGEPAGE); // Prevent window from closing
                return TRUE;
            }
*/
			/*
            char lsLocalCryptKey[255 + 1];
            int liLocalCryptKeyLen = Get_Text(hLocalCryptKey, lsLocalCryptKey, 255);
            if(liLocalCryptKeyLen == 0 || liLocalCryptKeyLen > 255)
            {
                MessageBox(hWnd, "You must enter a valid local crypt key. Between 1 and 255 characters.", gsTitleCaption, MB_ICONASTERISK);
                SetWindowLongPtr(hWnd, DWLP_MSGRESULT, PSNRET_INVALID_NOCHANGEPAGE); // Prevent window from closing
                return TRUE;
            }
*/
            //strcpy_s(gsAuthKey, sizeof(lsAuthKey), lsAuthKey);
            //strcpy_s(gsAuthString, sizeof(lsAuthString), lsAuthString);
            //strcpy_s(gsLocalCryptKey, sizeof(lsLocalCryptKey), lsLocalCryptKey);
            //strcpy_s(gsAdminPassword, sizeof(lsAdminPassword1), lsAdminPassword1);

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
