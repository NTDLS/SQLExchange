///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//  Copyright © NetworkDLS 2002, All rights reserved
//
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF 
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO 
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A 
// PARTICULAR PURPOSE.
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef _INIT_CPP
#define _INIT_CPP
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include <Windows.H>
#include <WindowsX.H>
#include <ShellAPI.H>
#include <Stdio.H>
#include <Stdlib.H>
#include <CommCtrl.H>

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "Entry.H"
#include "Init.H"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

using namespace NSWFL::Registry;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool InitializeApp(void)
{
    LoadString(ghAppInstance, IDS_APPCOPYRIGHT, gsAppCopyRight, sizeof(gsAppCopyRight));
    LoadString(ghAppInstance, IDS_FILEVERSION, gsFileVersion, sizeof(gsFileVersion));
    LoadString(ghAppInstance, IDS_TITLECAPTION, gsTitleCaption, sizeof(gsTitleCaption));
    LoadString(ghAppInstance, IDS_REGISTRYKEY, gsRegistryKey, sizeof(gsRegistryKey));
    LoadString(ghAppInstance, IDS_COMREGISTRYKEY, gsCommonRegistryKey, sizeof(gsCommonRegistryKey));

	GetBuildArchitecture(gsRegistryKey, sizeof(gsRegistryKey), gsRegistryKey);

	if(!GetRegistryValues())
    {
        MessageBox(NULL, "Failed to get registry values.", gsTitleCaption, MB_TASKMODAL|MB_ICONERROR);
        return 0;
    }

	INITCOMMONCONTROLSEX CommCtl;
    CommCtl.dwSize = sizeof(CommCtl);
    CommCtl.dwICC = ICC_PROGRESS_CLASS;

	if(!InitCommonControlsEx(&CommCtl))
	{
		MessageBox(NULL, "Filed to init common controls.", gsTitleCaption, MB_ICONSTOP|MB_TASKMODAL);
		return false;
	}

	return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool UninitializeApp(void)
{
    return true;
}


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool GetRegistryValues(void)
{
	DWORD dwBufSz = 0;

    dwBufSz = sizeof(gsPath);
	memset(gsPath, 0, dwBufSz);

    if(!Get_StringRegistryValue(HKEY_LOCAL_MACHINE, gsRegistryKey, "Path", gsPath, dwBufSz))
        return false;

    sprintf_s(gsServiceControlExe, sizeof(gsServiceControlExe), "%s\\SQLEClient.Exe", gsPath);

    return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#endif

