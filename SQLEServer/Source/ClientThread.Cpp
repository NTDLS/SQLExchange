#ifndef _ClientThread_CPP
#define _ClientThread_CPP
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include <Windows.H>
#include <WindowsX.H>
#include <ShellAPI.H>
#include <Stdio.H>
#include <Stdlib.H>
#include <Time.H>
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "Entry.H"
#include "ClientThread.H"
#include "Command.H"
#include "HandShake.H"

#include "../../SharedSource/HashKey.H"
#include "../../../@Libraries/CSHA1/CSHA1.H"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

using namespace NSWFL::String;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Sock_ClientThread(CSocketServer *pSock, CSocketClient *pClient, LPBASICHUNK pChunk)
{
	_ConnectedClientInfo *pCCI = ((_ConnectedClientInfo *)pClient->pUserData);

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

	//--------------------------------------------------------------------------------------------------------
	//(This BELOW helps to prevent Denial-of-Service attacks) (BEGIN)
	if(gdwHashTimeout > 0)
	{
		time_t ttNow;
		time_t ttSent;

		char sRawHash[1024];
		char sSHA1HAsh[41];

		if(!NewGUID(sRawHash, sizeof(sRawHash)))
		{
			WriteLog(pClient->PeerID(), "Failed to generate random hash key.", EVENT_ERROR);
			return;
		}

		//Send a plain random text string to the client. The first data received back from
		//	the client is expected to be the SHA1 hash of this string after it has been ciphered.
		pClient->SendCommand("::RawHash->", sRawHash, gdwRawHashLen, pChunk);

		//Cipher and calculate the SHA1 of our copy of the random data.
		HashKey((const unsigned char *)sRawHash, (unsigned char *)sRawHash, gdwRawHashLen, gdwRawHashLen);
		if(!SimpleSHA1(sRawHash, gdwRawHashLen, sSHA1HAsh, sizeof(sSHA1HAsh)))
		{
			WriteLog(pClient->PeerID(), "Failed to hash the random key.", EVENT_ERROR);
			return;
		}

		pClient->WaitOnDataToBeSent();
		time(&ttSent);

		while(pClient->IsConnected())
		{
			//If the client is still connected, receive their data.
			if(pClient->GetClientData(pChunk))
			{
				//Check to see if the dat recevied is the string hash.
				if(strcmp(sSHA1HAsh, (char *)pChunk->pBuffer) == 0)
				{
					break; //Good hash value.
				}
				else{
					WriteLog(pClient->PeerID(), "Client returned incorrect hash value. Possible DOS attack.", EVENT_WARN);
					return;
				}
			}

			if(difftime(time(&ttNow), ttSent) > gdwHashTimeout)
			{
				WriteLog(pClient->PeerID(), "Client did not respond. Possible DOS attack.", EVENT_WARN);
				return;				
			}

			Sleep(10);
		}

		if(!pClient->IsConnected())
		{
			WriteLog(pClient->PeerID(), "Client disconnected without responsing to hash. Possible DOS attack.", EVENT_WARN);
			return;
		}
	}
	//(The ABOVE code helps to prevent Denial-of-Service attacks) (END)//
	//--------------------------------------------------------------------------------------------------------

	pClient->SetNextSendData("::RequestAuthString");

	while(pClient->IsConnected())
	{
		if(pClient->GetClientData(pChunk))
		{
			if(pCCI->bIsAuthenticated == false)
			{
				int iAuthResult = PerformHandShake(pSock, pClient, pChunk);
				if(iAuthResult == AUTH_FAILED || iAuthResult == AUTH_ERROR)
				{
					WriteLog(pClient->PeerID(), "Authentication failed.", EVENT_AUDIT_FAILURE);
					pClient->WaitOnDataToBeSent();
					return;
				}
				else if(iAuthResult == AUTH_DECLINE)
				{
					WriteLog(pClient->PeerID(), "Authentication was declined.", EVENT_NONE);
					pClient->WaitOnDataToBeSent();
					return;
				}
				else if(iAuthResult == AUTH_SUCCESS){
					pCCI->bIsAuthenticated = true;
				}
				else if(iAuthResult == AUTH_OK){
					//The authentication process is still in process, all is well.
				}
			}
			else{
				int iCmdResult = ProcessCommand(pSock, pClient, pChunk);
				if(iCmdResult == CMD_ERROR)
				{
					WriteLog(pClient->PeerID(), "Process command returned with an error.", EVENT_ERROR);
					pClient->WaitOnDataToBeSent();
					return;
				}
				else if(iCmdResult == CMD_DONE) {
					pClient->WaitOnDataToBeSent();
					return;
				}
				else if(iCmdResult == CMD_OK) {
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
	_ConnectedClientInfo *pCCI = ((_ConnectedClientInfo *)pClient->pUserData);

	pCCI->bAllowOffLineDataProcessing = false;
	pCCI->bIsAuthenticated = false;
	pCCI->bIsEncryptionInit = false;
	pCCI->bIndexConnected = false;
	pCCI->bCustomerConnected = false;
	pCCI->bRequestInit = false;
	pCCI->bUseACK = false;
	pCCI->dwCorruptInitRetry = 0;
	pCCI->pClient = pClient;

	if(gbDebugMode)
	{
		WriteLog(pClient->PeerID(), "Initializing Cryptography Set...", EVENT_NONE);
	}

	memset(&pCCI->SC, 0, sizeof(pCCI->SC));

	//Used to decrypt the AuthString.
	if(pCCI->SC.InitializeCryptography(gsAuthKey, (int)strlen(gsAuthKey), true))
	{
		pCCI->bIsEncryptionInit = true;
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Sock_OnEndClientThread(CSocketServer *pSock, CSocketClient *pClient)
{
	_ConnectedClientInfo *pCCI = &CCI[pClient->Slot()];

	if(pCCI->bIndexConnected)
	{
		pCCI->cIndexSQL.Disconnect();
	}

	if(pCCI->bCustomerConnected)
	{
		pCCI->cCustSQL.Disconnect();
	}

	if(pCCI->bIsEncryptionInit)
	{
		if(gbDebugMode)
		{
			WriteLog(pClient->PeerID(), "Uninitializing Cryptography Set...", EVENT_NONE);
		}
		
		pCCI->SC.DestroyCryptography();

		pCCI->bIsEncryptionInit = false;
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool Sock_OnAccept(CSocketServer *pSock, CSocketClient *pClient)
{
	_ConnectedClientInfo *pCCI = &CCI[pClient->Slot()];
	memset(pCCI, 0, sizeof(_ConnectedClientInfo));
	pClient->pUserData = pCCI;

	if(pClient->PeerID() >= 0)
	{
		WriteLog(pClient->PeerID(), "Connected.", EVENT_NONE);
	}
	else WriteSysLogEx("Connected.", EVENT_NONE);

	memset(&pCCI->stConnectTime, 0, sizeof(pCCI->stConnectTime));

	GetSystemTime(&pCCI->stConnectTime);

	pCCI->dwBytesSent = 0;
	pCCI->dwBytesRecv = 0;

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

bool Sock_OnSend(CSocketServer *pSock, CSocketClient *pClient, LPBASICHUNK pChunk)
{
	((_ConnectedClientInfo *)pClient->pUserData)->dwBytesSent += (DWORD)pChunk->iLength;
	return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool Sock_OnRecv(CSocketServer *pSock, CSocketClient *pClient, LPBASICHUNK pChunk)
{
	((_ConnectedClientInfo *)pClient->pUserData)->dwBytesRecv += (DWORD)pChunk->iLength;
	return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool Sock_OnBeforePacketize(CSocketServer *pSock, CSocketClient *pClient, LPBASICHUNK pChunk)
{
	((_ConnectedClientInfo *)pClient->pUserData)->SC.Cipher((char *)pChunk->pBuffer, pChunk->iLength);
	return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool Sock_OnAfterDePacketize(CSocketServer *pSock, CSocketClient *pClient, LPBASICHUNK pChunk)
{
	((_ConnectedClientInfo *)pClient->pUserData)->SC.Cipher((char *)pChunk->pBuffer, pChunk->iLength);
	return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#endif
