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

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "../../../@Libraries/CCRC32/CCRC32.H"

#include "../../SharedSource/Common.H"
#include "../../SharedSource/NTFibers.H"
#include "../../SharedSource/FileExchange.H"

#include "Init.H"
#include "Entry.H"
#include "Routines.H"
#include "ClientThread.H"

#include "../../../@Libraries/CSocketServer/CSocketServer.H"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

using namespace NSWFL::Registry;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool InitializeApp(void)
{
    LoadString(ghAppInstance, IDS_APPCOPYRIGHT, gsAppCopyRight, sizeof(gsAppCopyRight));
    LoadString(ghAppInstance, IDS_FILEVERSION, gsFileVersion, sizeof(gsFileVersion));
    LoadString(ghAppInstance, IDS_TITLECAPTION, gsTitleCaption, sizeof(gsTitleCaption));
    LoadString(ghAppInstance, IDS_REGISTRYKEY, gsRegistryKey, sizeof(gsRegistryKey));
    LoadString(ghAppInstance, IDS_EVENTNAME, gsEventName, sizeof(gsEventName));

	GetBuildArchitecture(gsRegistryKey, sizeof(gsRegistryKey), gsRegistryKey);
	GetBuildArchitecture(gsEventName, sizeof(gsEventName), gsEventName);

	if(!(pMem = new CMemPool()))
	{
        MessageBox(NULL, "Failed to Initialize the Memory Pool.", gsTitleCaption, MB_TASKMODAL|MB_ICONERROR);
		return false;
	}

	InitializeCriticalSection(&csTransPerSecond);
	InitializeCriticalSection(&csLogFile);

	InitExpiryDate();
	
	if (pCRC32 == NULL)
	{
		pCRC32 = new CCRC32();
		pCRC32->Initialize();
	}

	if(!gbExcludeInit)
	{
		if(IsApplicationRunning())
		{
			return false;
		}

		//Set SessionStartTime in registry.
		FILETIME CreationTimeFT;
		FILETIME DummyFT;
		char sRegKey[255];

		sprintf_s(sRegKey, sizeof(sRegKey), "%s\\Statistics", gsRegistryKey);

		memset(&CreationTimeFT, 0, sizeof(CreationTimeFT));

		if(GetProcessTimes(GetCurrentProcess(), &CreationTimeFT, &DummyFT, &DummyFT, &DummyFT))
		{
			SYSTEMTIME CreationTimeST;

			memset(&CreationTimeST, 0, sizeof(CreationTimeST));

			if(FileTimeToSystemTime(&CreationTimeFT, &CreationTimeST))
			{
				char sTime[100];
				char sDate[100];
				char sStartTime[200];

				GetDateFormat(LOCALE_USER_DEFAULT, DATE_SHORTDATE, &CreationTimeST, NULL, sDate, 64);
				GetTimeFormat(LOCALE_USER_DEFAULT, 0, &CreationTimeST, NULL, sTime, 64);

				sprintf_s(sStartTime, sizeof(sStartTime), "%s %s", sDate, sTime);

				Set_StringRegistryValue(HKEY_LOCAL_MACHINE, sRegKey, "SessionStartTime", sStartTime);
			}
		}
	}

	memset(&MDI, 0, sizeof(MDI));
	memset(&gServer, 0, sizeof(gServer));

	MDI.Queu.Initialize();
	
	if(!gSC.InitializeCryptography(CRYPTKEY, (int)strlen(CRYPTKEY), true))
	{
        MessageBox(NULL, "Failed to Initialize Cryptography Set CSocketServer.", gsTitleCaption, MB_TASKMODAL|MB_ICONERROR);
		RecordError(gsEventName, "Failed to Initialize Cryptography Set CSocketServer.");
        return false;
	}

	if(!GetRegistryValues())
	{
		MessageBox(NULL, "Failed to get registry values.", gsTitleCaption, MB_TASKMODAL|MB_ICONERROR);
        return false;
	}

	//Set the default Affinity Mask if it is not set.
	if(gdwCPUAffinityMask == 0)
	{
		gdwCPUAffinityMask = 0xfffffffful;
	}

	SetProcessAffinityMask(GetCurrentProcess(), gdwCPUAffinityMask);
	SetProcessPriorityBoost(GetCurrentProcess(), !gbUseProcessBoost);

	gNTFibers.Init();

	if(!gbExcludeInit)
	{
		if(!OpenLogFile())
		{
			MessageBox(NULL, "Failed to open log file.", gsTitleCaption, MB_TASKMODAL|MB_ICONERROR);
			return false;
		}

		if(!gServer.Initialize(gdwListenPort, gdwMaxClients, &Sock_ClientThread, &Sock_OnSockError))
		{
			WriteSysLogEx("Failed to Initialize the CSocketServer.", EVENT_ERROR);
			return false;
		}

		gServer.SetOnAccept(&Sock_OnAccept);
		gServer.SetOnBeginClientThread(&Sock_OnBeginClientThread);
		gServer.SetOnEndClientThread(&Sock_OnEndClientThread);
		gServer.SetOnDisconnect(&Sock_OnDisconnect);
		gServer.SetOnSend(&Sock_OnSend);
		gServer.SetOnRecv(&Sock_OnRecv);
		gServer.SetOnBeforePacketize(&Sock_OnBeforePacketize);
		gServer.SetOnAfterDePacketize(&Sock_OnAfterDePacketize);
		gServer.NextPeerID(gdwNextClientID);
		gServer.MaxInactivityTime(gdwMaxInactivityTime);

		if((CCI = (_ConnectedClientInfo *) pMem->Allocate(gdwMaxClients, sizeof(_ConnectedClientInfo))) == NULL)
		{
			WriteSysLogEx("Failed to Initialize the ClientInfo structure.", EVENT_ERROR);
			Assert(!CCI, "Memory Allocation Error.");
			return false;
		}
	}

	if(OnlyGoodForOneYear())
	{
		WriteSysLogEx("This version of the SQL-Exchange has expired. Please obtain a newer version.", EVENT_ERROR);
	}

	return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool UninitializeApp(void)
{
	SaveRegistryValues();

	if(!gbExcludeInit)
	{
		if(!gServer.Destroy())
		{
			WriteSysLogEx("Failed to UnInitialize the CSocketServer.", EVENT_ERROR);

			return 0;
		}

		pMem->Free(CCI);

		CloseLogFile();
	}

	MDI.Queu.Destroy();
	gSC.DestroyCryptography();
	gNTFibers.Destroy();

	DeleteCriticalSection(&csTransPerSecond);
	DeleteCriticalSection(&csLogFile);

	if (pCRC32 != NULL)
	{
		delete pCRC32;
	}

	delete pMem;

	return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#endif
