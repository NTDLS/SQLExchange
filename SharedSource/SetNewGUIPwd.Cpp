///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//  Copyright © NetworkDLS 2002, All rights reserved
//
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF 
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO 
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef _SETGUIPWD_CPP
#define _SETGUIPWD_CPP
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include <Windows.H>
#include <Stdio.H>
#include <Stdlib.H>
#include <CommCtrl.H>

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#ifdef _SQLESERVER
	#include "../SQLEServer/Source/Entry.H"

#elif _SQLECLIENT
	#include "../SQLEClient/Source/Entry.H"
#else
	#error "You must define either _SQLESERVER or _SQLECLIENT"
#endif

#include "../../@Libraries/CSHA1/CSHA1.H"

#include "CustomProcs.H"
#include "SetNewGUIPwd.H"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

using namespace NSWFL::Windows;
using namespace NSWFL::Registry;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

INT_PTR CALLBACK SetGUIPwdDialog(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    //--------------------------------------------------------------------------

    static HWND hPassword1 = NULL;
    static HWND hPassword2 = NULL;
    static HWND hPwdWeight = NULL;

    //--------------------------------------------------------------------------

    if(uMsg == WM_INITDIALOG)
    {
        char sCaption[1024];
		sprintf_s(sCaption, sizeof(sCaption), "%s :: %s", gsTitleCaption, "Set Password");
        SendMessage(hWnd, (UINT)WM_SETTEXT, (WPARAM)0, (LPARAM)sCaption);
        SendMessage(hWnd, WM_SETICON, TRUE, (LPARAM) LoadIcon(ghAppInstance, MAKEINTRESOURCE(IDI_MAIN)) );

        hPassword1 = GetDlgItem(hWnd, IDC_PASSWORD1);
        hPassword2 = GetDlgItem(hWnd, IDC_PASSWORD2);
        hPwdWeight = GetDlgItem(hWnd, IDC_PWDWEIGHT);

		SendMessage(hPassword1, EM_LIMITTEXT, 1024, 0);
		SendMessage(hPassword2, EM_LIMITTEXT, 1024, 0);

		AddPWDWeightHandler(hWnd, hPassword1, hPwdWeight);
		AddPWDWeightHandler(hWnd, hPassword2, hPwdWeight);

        Set_Text(hPassword1, "");
        Set_Text(hPassword2, "");

        CenterOverOwner(hWnd);

        return TRUE; // Return TRUE to set the keyboard focus, Otherwise return FALSE
    }

    //--------------------------------------------------------------------------

    if(uMsg == WM_COMMAND)
    {
		if(wParam == IDC_CMDOK)
		{
			char sPwd1[1024 + 1];
			char sPwd2[1024 + 1];

			int iSz1 = Get_Text(hPassword1, sPwd1, sizeof(sPwd1));
			int iSz2 = Get_Text(hPassword2, sPwd2, sizeof(sPwd2));

			if(iSz1 == 0 && iSz2 == 0)
			{
				if(MessageBox(hWnd, "Are you sure you want to use a blank password?", gsTitleCaption,
					MB_YESNO| MB_ICONQUESTION | MB_DEFBUTTON2) == IDYES)
				{
					if(Set_StringRegistryValue(HKEY_LOCAL_MACHINE, gsRegistryKey, "GUIPWD", ""))
					{
						strcpy_s(gsGUIPWDHash, sizeof(gsGUIPWDHash), "");
						EndDialog(hWnd, 0);
					}
					else{
						MessageBox(hWnd, "Failed to write your password.", gsTitleCaption, MB_ICONERROR);
						return TRUE;
					}
				}
			}
			else if(iSz1 > 0 && iSz1 < sizeof(sPwd1))
			{
				if(strcmp(sPwd1, sPwd2) == 0)
				{
					char sHash[41];

					if(SimpleSHA1(sPwd1, iSz1, sHash, sizeof(sHash)))
					{
						if(Set_StringRegistryValue(HKEY_LOCAL_MACHINE, gsRegistryKey, "GUIPWD", sHash))
						{
							strcpy_s(gsGUIPWDHash, sizeof(gsGUIPWDHash), sHash);
							EndDialog(hWnd, 0);
						}
						else{
							MessageBox(hWnd, "Failed to write your password.", gsTitleCaption, MB_ICONERROR);
							return TRUE;
						}
					}
					else{
						MessageBox(hWnd, "Failed to hash your password.", gsTitleCaption, MB_ICONERROR);
						return TRUE;
					}
				}
				else{
					MessageBox(hWnd, "The passwords you entered do not match.", gsTitleCaption, MB_ICONINFORMATION);
					return TRUE;
				}
			}

			return TRUE;
		}

		if(wParam == IDC_CMDCANCEL)
		{
			EndDialog(hWnd, 0);
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

    if(uMsg == WM_CLOSE)  // Received Close Message
    {
        EndDialog(hWnd, 0);
        DestroyWindow(hWnd);
        return TRUE;
    }

    //--------------------------------------------------------------------------

    return FALSE;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#endif

