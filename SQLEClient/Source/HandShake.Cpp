///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//  Copyright © NetworkDLS 2002, All rights reserved
//
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF 
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO 
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A 
// PARTICULAR PURPOSE.
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef _HANDSHAKE_CPP
#define _HANDSHAKE_CPP
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include <Windows.H>
#include <WindowsX.H>
#include <ShellAPI.H>
#include <Stdio.H>
#include <Stdlib.H>

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "../../SharedSource/HashKey.H"
//#include "../../SharedSource/CRC32.H"
#include "../../../@Libraries/CSHA1/CSHA1.H"

#include "../../../@Libraries/CXML/CXML.H"

#include "Entry.H"
#include "HandShake.H"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

using namespace NSWFL::DateTime;
using namespace NSWFL::String;
using namespace NSWFL::Registry;
using namespace NSWFL::Windows;
using namespace NSWFL::File;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void BinPrint(const char *sData, int iLen)
{
	int iRPos = 0;
	while(iRPos < iLen)
	{
		printf("%X\n", sData[iRPos]);
		iRPos++;
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*
	int PerformHandShake(CSocketClient *pClient, char *sRecvBuf, int iRecvBufSz)

	Performs a hand shake. Also know as Client/Server Authentication.

	Possible return Values:
		AUTH_FAILED   //The authentication process failed.
		AUTH_SUCCESS  //The authentication process was a success.
		AUTH_OK       //The authentication process is still in process, all is well.
		AUTH_ERROR    //The authentication process failed due to an error.
*/
int PerformHandShake(CSocketServer *pSock, CSocketClient *pClient, LPBASICHUNK lpCmd)
{
    char sTemp[SENDBUFSZ + RECVBUFSZ];

	if(pSock->ParseCommand(lpCmd, "::RawHash->"))
    {
		if(gbDebugMode)
		{
			WriteLog(pClient->PeerID(), "Server requested key hash.", EVENT_NONE);
		}

		HashKey((const unsigned char *)lpCmd->pBuffer, (unsigned char *)lpCmd->pBuffer, lpCmd->iLength, lpCmd->iLength);

		char sSHA1HAsh[41];

		if(!SimpleSHA1((char *)lpCmd->pBuffer, lpCmd->iLength, sSHA1HAsh, sizeof(sSHA1HAsh)))
		{
			WriteLog(pClient->PeerID(), "Failed to hash the random key.", EVENT_ERROR);
			pClient->Disconnect();
			return 0;
		}

		pClient->SetNextSendData(sSHA1HAsh);
		return AUTH_OK;
	}
	else if(pSock->ParseCommand(lpCmd, "<::UpdateSchedule>"))
    {
		if(gbDebugMode)
		{
			WriteLog(pClient->PeerID(), "Server is updating the schedule.", EVENT_NONE);
		}

		CXMLReader MyAuth((char *)lpCmd->pBuffer, lpCmd->iLength);
		gbWeekDays[0] = MyAuth.ToBoolean("Sunday");
		gbWeekDays[1] = MyAuth.ToBoolean("Monday");
		gbWeekDays[2] = MyAuth.ToBoolean("Tuesday");
		gbWeekDays[3] = MyAuth.ToBoolean("Wednesday");
		gbWeekDays[4] = MyAuth.ToBoolean("Thursday");
		gbWeekDays[5] = MyAuth.ToBoolean("Friday");
		gbWeekDays[6] = MyAuth.ToBoolean("Saturday");

		gdwStartTime = MyAuth.ToUInteger("StartTime");
		gdwStopTime = MyAuth.ToUInteger("EndTime");

		gdwConnectInterval = MyAuth.ToUInteger("Increment");

		MyAuth.Destroy();

		if(SaveRegistryValues())
		{
			pClient->SetNextSendData("::ScheduleUpdateSuccessful");
			return AUTH_OK;
		}
		else{
			pClient->SetNextSendData("::ScheduleUpdateFailed");
		}

		return AUTH_ERROR;
	}
	else if(pSock->IsCommand(lpCmd, "::RequestAuthString"))
    {
		if(gbDebugMode)
		{
			WriteLog(pClient->PeerID(), "Server requested authentication.", EVENT_NONE);
		}

		pClient->SendCommand("::AuthString->", gsAuthString, (int)strlen(gsAuthString), lpCmd);
		return AUTH_OK;
	}
	else if(pSock->ParseCommand(lpCmd, "::NewAuthKey->"))
    {
		if(gbDebugMode)
		{
			WriteLog(pClient->PeerID(), "Server issued new cryptography key.", EVENT_NONE);
		}

		if(gbDebugMode)
		{
			WriteLog(pClient->PeerID(), "Reinitializing Cryptography Set...", EVENT_NONE);
		}

		CCI.SC.DestroyCryptography();

		if(!CCI.SC.InitializeCryptography((char *)lpCmd->pBuffer, lpCmd->iLength, true))
		{
			WriteLog(pClient->PeerID(), "Failed to Initialize the cryptography set.", EVENT_ERROR);
		}

		return AUTH_OK;
	}
	else if(pSock->IsCommand(lpCmd, "::RequestAuthentication"))
    {
		CXMLWriter MyAuth("::Authentication");

		char sPwdHash[41];
		if(!SimpleSHA1(gsCompanyPassword, (int)strlen(gsCompanyPassword), sPwdHash, sizeof(sPwdHash)))
		{
			WriteLog(pClient->PeerID(), "Failed to hash the random key.", EVENT_ERROR);
			pClient->Disconnect();
			MyAuth.Destroy();
			return 0;
		}

		MyAuth.Add("Version", gsFileVersion);
		MyAuth.Add("Architecture", GetBuildArchitecture());
		MyAuth.Add("Company", gsCompanyName);
		MyAuth.Add("Password", sPwdHash);
		SendCommand(pClient, &MyAuth);
		MyAuth.Destroy();

		return AUTH_OK;
	}
	else if(pSock->IsCommand(lpCmd, "::AuthenticationSuccess"))
    {
		WriteLog(pClient->PeerID(), "Authentication success.", EVENT_NONE);
		pClient->SetNextSendData("::AwaitingCommand");
		return AUTH_SUCCESS;
	}
	else if(pSock->ParseCommand(lpCmd, "::AutoUpdate->"))
    {
		WriteLog(pClient->PeerID(), "Starting automatic update process.", EVENT_INFO);

        char sOldExe[MAX_PATH];
        char sURL[MAX_PATH];
        char sNewExe[MAX_PATH];
        char sTempPath[MAX_PATH];
        char sCurrentDirectory[MAX_PATH];

		if(Get_TempDirectory(sTempPath, sizeof(sTempPath)))
		{
			sprintf_s(sOldExe, sizeof(sOldExe), "%s\\%s", gsPath, "AutoUpdate.Exe");
			sprintf_s(sNewExe, sizeof(sNewExe), "%s\\%s", sTempPath, "SQLEUpdate.Exe");
			if(CopyFile(sOldExe, sNewExe, FALSE))
			{
				//lpCmd->sBuffer = Http://SQLExch.Com/Updates/x.x.x.x_<Arch>.exe
				ReplaceStrings((char *)lpCmd->pBuffer, "<Arch>", GetBuildArchitecture(), sURL, sizeof(sURL));

				if(Get_CurrentDirectory(sCurrentDirectory, sizeof(sCurrentDirectory)))
				{
					if(((int)ShellExecute(GetActiveWindow(), "Open", sNewExe, sURL, sCurrentDirectory, SW_SHOWNORMAL)) > 32)
					{
						pClient->Disconnect();

						if(MDI.hWnd) EndDialog(MDI.hWnd, 0);

						if(gService.IsInstalled() == _SVC_YES_ || gService.IsActive() == _SVC_YES_)
						{
							gService.Stop();
						}

						return AUTH_SUCCESS;
					}
				}
			}
		}
		WriteLog(pClient->PeerID(), "The Auto-Update process failed.", EVENT_WARN);
		return AUTH_FAILED;
	}
	else if(pSock->ParseCommand(lpCmd, "::AuthenticationFailed->"))
    {
		sprintf_s(sTemp, sizeof(sTemp), "The server reported an authentication failure: %s", (char *)lpCmd->pBuffer);
		WriteLog(pClient->PeerID(), sTemp, EVENT_WARN);
		return AUTH_FAILED;
	}
	else if(pSock->IsCommand(lpCmd, "::AuthenticationDeclined"))
    {
		WriteLog(pClient->PeerID(), "The server declined authentication.", EVENT_NONE);
		return AUTH_FAILED;
	}
	else if(pSock->IsCommand(lpCmd, "::AuthenticationFailed"))
    {
		WriteLog(pClient->PeerID(), "The server reported an authentication failure.", EVENT_WARN);
		return AUTH_FAILED;
	}
	//------------------------------------------------------------------------------------------------
	//This is a general processing message from the server.
	//	All messages are file logged and written to the event log.
	else if(pSock->ParseCommand(lpCmd, "::Msg->"))
    {
		if(gbDebugMode)
		{
			WriteLog(pClient->PeerID(), "Received a message from the server.", EVENT_NONE);
		}

		WriteLog(pClient->PeerID(), (char *)lpCmd->pBuffer, EVENT_INFO);

		return AUTH_OK;
	}

	//------------------------------------------------------------------------------------------------
	WriteLog(pClient->PeerID(), "Received unknown command.", EVENT_ERROR);
	//------------------------------------------------------------------------------------------------

	if(lpCmd->iLength > 1024)
	{
		((char *)lpCmd->pBuffer)[1024] = '\0';
	}
	WriteLog(pClient->PeerID(), (char *)lpCmd->pBuffer, EVENT_ERROR);
	return AUTH_ERROR;	
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#endif
