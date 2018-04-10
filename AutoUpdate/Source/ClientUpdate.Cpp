///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//  Copyright © NetworkDLS 2002, All rights reserved
//
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF 
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO 
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A 
// PARTICULAR PURPOSE.
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef _CLIENT_UPDATE_CPP
#define _CLIENT_UPDATE_CPP
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include <Windows.H>
#include <Stdio.H>
#include <Stdlib.H>
#include <WinLDap.H>
#include <ShlWapi.H>
#include <CommCtrl.H>
#include <UrlMon.H>

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "Entry.H"
#include "Init.H"
#include "ClientUpdate.H"

#include "../Dialogs/MainDlg.H"

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

using namespace NSWFL::Windows;
using namespace NSWFL::Registry;

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

CCallback::CCallback()
{
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

CCallback::~CCallback()
{
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

HRESULT CCallback::OnProgress(ULONG ulProgress, ULONG ulProgressMax, ULONG ulStatusCode, LPCWSTR wszStatusText)
{
    static char lsAmtDownloaded[256];
    static char lsTotalSize[256];
//  static char lsStatusText[256];
    static char lsCustomStatusMsg[256];
    static ULONG LastProgress = 0;

/*
    if(wszStatusText != NULL)
    {
        int CWStrLen = wcslen(wszStatusText);
        LdapUnicodeToUTF8(wszStatusText, CWStrLen, lsStatusText, 255);
        lsStatusText[CWStrLen] = '\0';
        Set_Text(StatusText_hWnd, lsStatusText);
    }
*/

    StrFormatByteSize(ulProgress, lsAmtDownloaded, 255);
    StrFormatByteSize(ulProgressMax, lsTotalSize, 255);

    if(0 != ulProgressMax)
    {
        if(ulProgress > (LastProgress + 10000))
        {
            float lfProgress    = (float) ulProgress;
            float lfProgressMax = (float) ulProgressMax;
            float lfPercentage  = (float) ((lfProgress / lfProgressMax) * 100);

            sprintf_s(lsCustomStatusMsg, sizeof(lsCustomStatusMsg),
				"Downloaded %s of %s (%.1f%%)", lsAmtDownloaded, lsTotalSize, lfPercentage);
            Set_Text(StatusText_hWnd, lsCustomStatusMsg);
            SendMessage(ProgressBar_hWnd, PBM_SETPOS, (ULONG)lfPercentage, 0);
            LastProgress = ulProgress;
        }
    }
    else{
        if(ulProgress > (LastProgress + 10000))
        {
            sprintf_s(lsCustomStatusMsg, sizeof(lsCustomStatusMsg),
				"Downloaded %s (total size unknown)", lsAmtDownloaded);
            Set_Text(StatusText_hWnd, lsCustomStatusMsg);
            LastProgress = ulProgress;
        }
    }

    return S_OK;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool ClientUpdate(char *HTTPURL)
{
    char StatusTemp[255 + 1];
    char TargetFile[255 + 1];

    CCallback Callback;

    sprintf_s(TargetFile, sizeof(TargetFile), "%s\\%s", gsPath, "Update.Exe");

    MyExecute(gsServiceControlExe, "/stop");

	HRESULT HResult = URLDownloadToFile(NULL, HTTPURL, TargetFile, 0, &Callback);

    if(SUCCEEDED(HResult))
    {
		Set_Text(StatusText_hWnd, "Download completed successfully!");
        Sleep(2500);

		Set_Text(StatusText_hWnd, "Terminating agents and services.");

		Set_BOOLRegistryValue(HKEY_LOCAL_MACHINE, gsCommonRegistryKey, "TerminateAgent", 1);
		
	    MyExecute(gsServiceControlExe, "/stop");
        Sleep(5000);

        MyExecute(TargetFile, "/sp- /silent");
    }
    else{
        sprintf_s(StatusTemp, sizeof(StatusTemp), "Download failed. Error = 0x%08lX", HResult);
        Set_Text(StatusText_hWnd, StatusTemp);
        MessageBox(MainDialog_hWnd, StatusTemp, gsTitleCaption, MB_ICONSTOP|MB_TASKMODAL);

		Set_BOOLRegistryValue(HKEY_LOCAL_MACHINE, gsCommonRegistryKey, "TerminateAgent", 0);

		RestartClientService();
	}

    EndDialog(MainDialog_hWnd, 0);

    return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#endif
