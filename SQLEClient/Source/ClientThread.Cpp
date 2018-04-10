///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//  Copyright © NetworkDLS 2002, All rights reserved
//
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF 
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO 
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A 
// PARTICULAR PURPOSE.
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#ifndef _ClientThread_CPP
#define _ClientThread_CPP
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include <Windows.H>
#include <WindowsX.H>
#include <ShellAPI.H>
#include <Stdio.H>
#include <Stdlib.H>

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "Entry.H"
#include "ClientThread.H"
#include "Command.H"
#include "HandShake.H"

#include "../../SharedSource/HashKey.H"
#include "../../../@Libraries/CSHA1/CSHA1.H"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Sock_ClientThread(CSocketServer *pSock, CSocketClient *pClient, LPBASICHUNK pChunk)
{
	SetThreadPriorityBoost(GetCurrentThread(), !gbUseThreadBoost);

	if(gbUseNTFibers)
	{
		//------------------------------------------------------------------------------------
		//This does not actually work. Need to create the fiber with CreateFiber().
		//Then call this Client_Thread_Function() function from the fiber proc.
		//ConvertThreadToFiber() is used to convert the thread that is going to be scheduling
		//	the fibers to a fiber.
		//------------------------------------------------------------------------------------
		//ConvertThreadToFiber(pvThread);
	}

	while(pClient->IsConnected())
	{
		if(pClient->GetClientData(pChunk))
		{
			if(CCI.bIsAuthenticated == false)
			{
				int iAuthResult = PerformHandShake(pSock, pClient, pChunk);
				if(iAuthResult == AUTH_FAILED || iAuthResult == AUTH_ERROR)
				{
					return;
				}
				else if(iAuthResult == AUTH_SUCCESS)
				{
					CCI.bIsAuthenticated = true;
				}
				else if(iAuthResult == AUTH_OK)
				{
					//The authentication process is still in process, all is well.
				}
			}
			else{
				int iCmdResult = ProcessCommand(pSock, &CCI, pChunk);
				if(iCmdResult == CMD_ERROR)
				{
					WriteLog(pClient->PeerID(),
						"Process command returned with an error. Check the event log.", EVENT_WARN);
					return;
				}
				else if(iCmdResult == CMD_DONE)
				{
					return;
				}
				else if(iCmdResult == CMD_OK)
				{
					//All is well.
				}
			}
		}
		else{
			Sleep(1);
		}
	}

	return;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Sock_OnSockError(CSocketServer *pSock, CSocketClient *pClient, int iErrorNumber, const char *sErrorMsg)
{
	if(pClient)
	{
		WriteLog(pClient->PeerID(), sErrorMsg, EVENT_NONE);
	}
	else{
		WriteSysLogEx(sErrorMsg, EVENT_NONE);
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Sock_OnBeginClientThread(CSocketServer *pSock, CSocketClient *pClient, HANDLE hThread)
{
	CCI.bIsAuthenticated = false;
	CCI.bIsEncryptionInit = false;
	CCI.iInitStep = 0;
	CCI.pClient = pClient;

	if(gbDebugMode)
	{
		WriteLog(pClient->PeerID(), "Initializing Cryptography Set...", EVENT_NONE);
	}

	//Used to encrypt the AuthString.
	if(CCI.SC.InitializeCryptography(gsAuthKey, (int)strlen(gsAuthKey), true))
	{
		CCI.bIsEncryptionInit = true;
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Sock_OnEndClientThread(CSocketServer *pSock, CSocketClient *pClient)
{
	if(CCI.bCustDBConnected)
	{
		CCI.cCustSQL.Disconnect();
		CCI.bCustDBConnected = false;
	}

	if(CCI.bIsEncryptionInit)
	{
		if(gbDebugMode)
		{
			WriteLog(pClient->PeerID(), "Uninitializing Cryptography Set...", EVENT_NONE);
		}

		CCI.SC.DestroyCryptography();
		CCI.bIsEncryptionInit = false;
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool Sock_OnConnect(CSocketServer *pSock, CSocketClient *pClient)
{
	pClient->pUserData = &CCI;

	if(pClient->PeerID() >= 0)
	{
		WriteLog(pClient->PeerID(), "Connected.", EVENT_NONE);
	}
	else WriteSysLogEx("Connected.", EVENT_NONE);

	gdwTotalConnects++;

	return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Sock_OnDisconnect(CSocketServer *pSock, CSocketClient *pClient)
{
	if(pClient->PeerID() >= 0)
	{
		WriteLog(pClient->PeerID(), "Disconnected.", EVENT_NONE);
	}
	else WriteSysLogEx("Disconnected.", EVENT_NONE);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool Sock_OnBeforePacketize(CSocketServer *pSock, CSocketClient *pClient, LPBASICHUNK pChunk)
{
	CCI.SC.Cipher(pChunk->pBuffer, pChunk->iLength);
	return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool Sock_OnAfterDePacketize(CSocketServer *pSock, CSocketClient *pClient, LPBASICHUNK pChunk)
{
	CCI.SC.Cipher(pChunk->pBuffer, pChunk->iLength);
	return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#endif
