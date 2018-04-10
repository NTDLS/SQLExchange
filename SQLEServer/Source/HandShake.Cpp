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
#include <SQL.H>
#include <SQLExt.H>

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "../Resources/Resource.H"
#include "../../SharedSource/Common.H"

#include "../../../@Libraries/CSHA1/CSHA1.H"
#include "../../../@Libraries/CSocketServer/CSocketServer.H"
#include "../../../@Libraries/CXML/CXML.H"

#include "Init.H"
#include "Routines.H"
#include "HandShake.H"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

using namespace NSWFL::KeyGeneration;
using namespace NSWFL::DateTime;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool IsValidCompanyAccount(CSocketServer *pSock, CSocketClient *pClient, const char *sCompanyName, char *sSHA1Pwd);
int IsValidClientVersion(CSocketServer *pSock, CSocketClient *pClient, char *sOutUpdateURL, int iMaxSz);
int RefreshSchedule(CSocketServer *pSock, CSocketClient *pClient, LPBASICHUNK pChunk);

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

enum AuthenticationStep{
	AUTH_STEP_00,
	AUTH_STEP_01,
	AUTH_STEP_02,
	AUTH_STEP_03
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int PerformHandShake(CSocketServer *pSock, CSocketClient *pClient, BASICHUNK *lpCmd)
{
	_ConnectedClientInfo *pCCI = (_ConnectedClientInfo *)pClient->pUserData;

	char sTemp[1024];

	int iSendBufSz = 0;
	int iCmdDataSz = 0;

	//------------------------------------------------------------------------------------------------
	if(pSock->IsCommand(lpCmd, "::Option-UseACK"))
	{
		pCCI->bUseACK = true;
		return AUTH_OK;
	}
	//------------------------------------------------------------------------------------------------
	else if(pSock->ParseCommand(lpCmd, "::AuthString->"))
    {
		char sNewAuthKey[MAX_KEY_LENGTH];

		if(gbDebugMode)
		{
			WriteLog(pClient->PeerID(), "Received authentication string.", EVENT_NONE);
		}

		if(strcmp((char *)lpCmd->pBuffer, gsAuthString) != 0)
		{
			WriteLog(pClient->PeerID(), "Authentication failed due to invalid Authentication String.", EVENT_WARN);
			pClient->SetNextSendData("::AuthenticationFailed");
			return AUTH_FAILED;
		}
		
		if(gbDebugMode)
		{
			WriteLog(pClient->PeerID(), "Validation of Authentication String verified.", EVENT_NONE);
		}
		
		if(!GenerateKey(gdwKeyGenLength, (GetTickCount()+pClient->Slot()), GKUPPER_AZ|GKNUMBERS|GKNONREPETITION, sNewAuthKey))
		{
			WriteLog(pClient->PeerID(), "PerformHandShake failed to generate a key.", EVENT_ERROR);
			pClient->SetNextSendData("::AuthenticationFailed");
			return AUTH_ERROR;
		}


		pClient->SendCommand("::NewAuthKey->", sNewAuthKey, gdwKeyGenLength, lpCmd);
		pClient->WaitOnDataToBeSent();

		if(gbDebugMode)
		{
			WriteLog(pClient->PeerID(), "Reinitializing Cryptography Set...", EVENT_NONE);
		}

		pCCI->SC.DestroyCryptography();

		if(!pCCI->SC.InitializeCryptography(sNewAuthKey, gdwKeyGenLength, true))
		{
			WriteLog(pClient->PeerID(), "Failed to Initialize the cryptography set.", EVENT_ERROR);
		}

		pClient->SetNextSendData("::RequestAuthentication");

		return AUTH_OK;
	}
	//------------------------------------------------------------------------------------------------
	else if(pSock->ParseCommand(lpCmd, "<::Authentication>"))
	{
		char sPassword[64];
		char sArchitecture[64];

		CXMLReader MyAuth((char *)lpCmd->pBuffer, lpCmd->iLength);
		MyAuth.ToString("Version", pCCI->sClientVersion, sizeof(pCCI->sClientVersion));
		MyAuth.ToString("Architecture", sArchitecture, sizeof(sArchitecture));
		MyAuth.ToString("Company", pCCI->sCompanyName, sizeof(pCCI->sCompanyName));
		MyAuth.ToString("Password", sPassword, sizeof(sPassword));
		MyAuth.Destroy();

		if(gbDebugMode)
		{
			sprintf_s(sTemp, sizeof(sTemp), "Client Version: %s %s", pCCI->sClientVersion, sArchitecture);
			WriteLog(pClient->PeerID(), sTemp, EVENT_NONE);

			sprintf_s(sTemp, sizeof(sTemp), "Company Name: %s", pCCI->sCompanyName);
			WriteLog(pClient->PeerID(), sTemp, EVENT_NONE);

			sprintf_s(sTemp, sizeof(sTemp), "Password: %s", sPassword);
			WriteLog(pClient->PeerID(), sTemp, EVENT_NONE);
		}

		if(_strcmpi(sArchitecture, GetBuildArchitecture()) != 0)
		{
			sprintf_s(sTemp, sizeof(sTemp),
				"Client / Server architecture mismatch (Server is %s, Client is %s)", GetBuildArchitecture(), sArchitecture);
			WriteLog(pClient->PeerID(), sTemp, EVENT_WARN);
			pClient->Disconnect();
			return AUTH_DECLINE;
		}

		if(ConnectToSQLServer(&pCCI->cIndexSQL, gsSQLIndexServer, gsSQLIndexUserID, gsSQLIndexPassword,
			gsSQLIndexDatabase, gbSQLIndexUseTCPIPConnection, gbSQLIndexUseTrustedConnection, gdwSQLIndexServerPort))
		{
			if(gbDebugMode)
			{
				WriteLog(pClient->PeerID(), "Successfully connected to the Index database.", EVENT_NONE);
			}
			pCCI->bIndexConnected = true;
		}
		else{
			WriteLog(pClient->PeerID(), "Failed to connect to the Index database.", EVENT_ERROR);
			return AUTH_ERROR;
		}

		//------------------------------------------------(Check Client Version)
        char sUpdateHTTPURL[1024 + 1];

        int iCVResult = IsValidClientVersion(pSock, pClient, sUpdateHTTPURL, sizeof(sUpdateHTTPURL));
        if(iCVResult == CLIENTVER_CURRENT || iCVResult == CLIENTVER_SUPPORTED)
		{
	        if(iCVResult == CLIENTVER_SUPPORTED)
			{
				WriteLog(pClient->PeerID(),
					"Client software is outdated but still supported. Update soon!", EVENT_NONE);
			}
	        else if(iCVResult == CLIENTVER_CURRENT && gbDebugMode)
			{
				WriteLog(pClient->PeerID(), "Client software is up-to-date.", EVENT_NONE);
			}

			//------------------------------------------------(Check Company Name & Password)

			//Check to see if we are performing off-line processing for this company.
			CSocketClient *pC = NULL;
			for(int iC = 0; (pC = pSock->Client(iC)); iC++)
			{
				if(pC->Reserved())
				{
					if(_strcmpi(((_ConnectedClientInfo *)pC->pUserData)->sCompanyName, pCCI->sCompanyName) == 0)
					{
						WriteLog(pClient->PeerID(), "The server is still processing a previous workload.", EVENT_INFO);
						pClient->Disconnect();
						return AUTH_DECLINE;
					}
				}
			}

			if(!IsValidCompanyAccount(pSock, pClient, pCCI->sCompanyName, sPassword))
			{
				WriteLog(pClient->PeerID(), "Invalid company account.", EVENT_WARN);
				pClient->SetNextSendData("::AuthenticationFailed");
				return AUTH_FAILED;
			}

			int iAuthResult = RefreshSchedule(pSock, pClient, lpCmd);
			if(iAuthResult == AUTH_DECLINE)
			{
				pClient->SetNextSendData("::AuthenticationDeclined");
				pClient->WaitOnDataToBeSent();
				return AUTH_DECLINE;
			}
			else if(iAuthResult != AUTH_OK)
			{
				pClient->SetNextSendData("::AuthenticationFailed");
				return AUTH_FAILED;
			}

			//--------------------------------------------------------

			//Connect to the master database.
			if(ConnectToSQLServer(&pCCI->cCustSQL, gsSQLCustServer, gsSQLCustUserID, gsSQLCustPassword, "master",
				gbSQLCustUseTCPIPConnection, gbSQLCustUseTrustedConnection, gdwSQLCustServerPort))
			{
				if(gbDebugMode)
				{
					WriteLog(pClient->PeerID(), "Successfully connected to the master database.", EVENT_NONE);
				}
				pCCI->bCustomerConnected = true;
			}
			else{
				WriteLog(pClient->PeerID(), "Failed to connect to the master database.", EVENT_ERROR);
				return AUTH_FAILED;						
			}

			//----------------------------------------------------------------------------------------------------------------
			WriteLog(pClient->PeerID(), "Authentication success.", EVENT_NONE);
			pClient->SetNextSendData("::AuthenticationSuccess");
			//----------------------------------------------------------------------------------------------------------------
			return AUTH_SUCCESS;
		}
        else if(iCVResult == CLIENTVER_OUTDATED){
			WriteLog(pClient->PeerID(), "Client software is outdated. Update now!", EVENT_NONE);
            return AUTH_ERROR;
        }
        else if(iCVResult == CLIENTVER_AUTOUPDATE){
			WriteLog(pClient->PeerID(), "Performing client autoupdate.", EVENT_INFO);

            sprintf_s(sTemp, sizeof(sTemp), "::AutoUpdate->%s", sUpdateHTTPURL);
            pClient->SetNextSendData(sTemp);
            return AUTH_OK;
        }
        else{
			WriteLog(pClient->PeerID(), "Error verifing client version.", EVENT_ERROR);
            return AUTH_ERROR;
        }

		WriteLog(pClient->PeerID(), "Logic failure in [::Authentication->]!", EVENT_ERROR);
		return AUTH_ERROR;
	}
	//------------------------------------------------------------------------------------------------
	else if(pSock->IsCommand(lpCmd, "::AuthenticationDeclined"))
    {
		WriteLog(pClient->PeerID(), "The client declined authentication.", EVENT_NONE);
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

DWORD MakeTimeFromString(const char *sTime)
{
	int iRPos = 0;
	int iWPos = 0;
	DWORD dwTime = 0;

	DWORD iPart = 0;

	char sPart[10];

	while(true)
	{
		if(sTime[iRPos] == ':' || sTime[iRPos] == '\0')
		{
			sPart[iWPos] = '\0';

			if(iPart == 0) {
				dwTime = (atol(sPart) * 60l);
			}
			else if(iPart == 1) {
				dwTime += atol(sPart);
			}
			else if(iPart == 2) {
				dwTime = ((dwTime * 60l) + atol(sPart));
				//break;
			}

			iWPos = 0;
			iPart++;
		}
		else {
			sPart[iWPos++] = sTime[iRPos];
		}

		if(sTime[iRPos] == '\0')
		{
			break;
		}

		iRPos++;
	}

	if(iPart < 3)
	{
		dwTime *= 60l;
	}

	return dwTime;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int RefreshSchedule(CSocketServer *pSock, CSocketClient *pClient, LPBASICHUNK pChunk)
{
	_ConnectedClientInfo *pCCI = (_ConnectedClientInfo *)pClient->pUserData;

	char sMessage[1024];
	char sSQL[1024];

	bool bEnforceSchedule = false;

	CLIENT_SCHEDULE CS;

	memset(&CS, 0, sizeof(CS));

	CBoundRecordSet rsTemp;

	sprintf_s(sSQL, sizeof(sSQL),
		"EXEC GetCompanyScheduleByName '%s'",
		pCCI->sCompanyName);

	if(!pCCI->cIndexSQL.Execute(sSQL, &rsTemp))
    {
        WriteLog(pClient->PeerID(), "Error in RefreshSchedule :: Execute failed.", EVENT_ERROR);
        return AUTH_ERROR;
    }

    if(rsTemp.RowCount != 1)
	{
		//No schedule record, use client configured schedule.
		rsTemp.Close();
		return AUTH_OK;
	}

	if(!rsTemp.Fetch())
    {
        WriteLog(pClient->PeerID(), "Error in RefreshSchedule :: DBFetch failed.", EVENT_ERROR);
        rsTemp.Close();
        return AUTH_ERROR;
    }

	CS.bSunday		 = rsTemp.Values("Sunday")->ToBoolean();
	CS.bMonday		 = rsTemp.Values("Monday")->ToBoolean();
	CS.bTuesday		 = rsTemp.Values("Tuesday")->ToBoolean();
	CS.bWednesday	 = rsTemp.Values("Wednesday")->ToBoolean();
	CS.bThursday	 = rsTemp.Values("Thursday")->ToBoolean();
	CS.bFriday		 = rsTemp.Values("Friday")->ToBoolean();
	CS.bSaturday	 = rsTemp.Values("Saturday")->ToBoolean();
	CS.dwBeginTime   = MakeTimeFromString(rsTemp.Values("StartTime")->ToString());
	CS.dwEndTime     = MakeTimeFromString(rsTemp.Values("EndTime")->ToString());
	CS.iIncrement    = rsTemp.Values("Increment")->ToIntegerS();
	bEnforceSchedule = rsTemp.Values("Enforce")->ToBoolean();
	rsTemp.Close();

	CXMLWriter MyAuth("::UpdateSchedule");
	MyAuth.Add("Sunday", CS.bSunday);
	MyAuth.Add("Monday", CS.bMonday);
	MyAuth.Add("Tuesday", CS.bTuesday);
	MyAuth.Add("Wednesday", CS.bWednesday);
	MyAuth.Add("Thursday", CS.bThursday);
	MyAuth.Add("Friday", CS.bFriday);
	MyAuth.Add("Saturday", CS.bSaturday);
	MyAuth.Add("StartTime", (unsigned int)CS.dwBeginTime);
	MyAuth.Add("EndTime", (unsigned int)CS.dwEndTime);
	MyAuth.Add("Increment", CS.iIncrement);

	if(SendCommandWithResponse(pClient, &MyAuth, pChunk))
	{
		if(_strcmpi((char *)pChunk->pBuffer, "::ScheduleUpdateSuccessful") == 0)
		{
			if(gbDebugMode)
			{
				WriteLog(pClient->PeerID(), "The client schedule was successfully updated.");
			}
		}
		else if(_strcmpi((char *)pChunk->pBuffer, "::ScheduleUpdateFailed") == 0)
		{
			WriteLog(pClient->PeerID(), "Failed to update the client schedule.", EVENT_WARN);
			MyAuth.Destroy();
			return AUTH_ERROR;
		}
		else{
			WriteLog(pClient->PeerID(), "Received an unexpected command while updating the schedule.", EVENT_WARN);
			MyAuth.Destroy();
			return AUTH_ERROR;
		}
	}
	else{
		WriteLog(pClient->PeerID(), "Failed to get client data while updating the schedule.", EVENT_WARN);
		MyAuth.Destroy();
		return AUTH_ERROR;
	}

	MyAuth.Destroy();

	if(bEnforceSchedule) // Enforce schedule?
	{
		bool bResult = false; //Default to fail.

		SYSTEMTIME ST;

		memset(&ST, 0, sizeof(ST));

		GetLocalTime(&ST);

		DWORD dwTime = TimeToLong(ST.wHour, ST.wMinute, ST.wSecond);

		sprintf_s(sMessage, sizeof(sMessage), "Customer is not allowed to connect on ");

		if(ST.wDayOfWeek == 0 && !CS.bSunday) {
			strcat_s(sMessage, sizeof(sMessage), "Sundays");
		}
		else if(ST.wDayOfWeek == 1 && !CS.bMonday){
			strcat_s(sMessage, sizeof(sMessage), "Mondays");
		}
		else if(ST.wDayOfWeek == 2 && !CS.bTuesday){
			strcat_s(sMessage, sizeof(sMessage), "Tuesdays");
		}
		else if(ST.wDayOfWeek == 3 && !CS.bWednesday){
			strcat_s(sMessage, sizeof(sMessage), "Wednesdays");
		}
		else if(ST.wDayOfWeek == 4 && !CS.bThursday){
			strcat_s(sMessage, sizeof(sMessage), "Thursdays");
		}
		else if(ST.wDayOfWeek == 5 && !CS.bFriday){
			strcat_s(sMessage, sizeof(sMessage), "Fridays");
		}
		else if(ST.wDayOfWeek == 6 && !CS.bSaturday){
			strcat_s(sMessage, sizeof(sMessage), "Saturdays");
		}
		else if(dwTime < (CS.dwBeginTime - 600)){ //Allow 10 minutes outside of time window.
			sprintf_s(sMessage, sizeof(sMessage), "Connection is out of time window. Too early.");
		}
		else if(dwTime > (CS.dwEndTime + 600)){ //Allow 10 minutes outside of time window.
			sprintf_s(sMessage, sizeof(sMessage), "Connection is out of time window. Too Late.");
		}
		else{
			bResult = true; //All is well.
		}

		if(!bResult)
		{
			WriteLog(pClient->PeerID(), sMessage, EVENT_NONE);
			return AUTH_DECLINE;
		}
	}

	return AUTH_OK;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool IsValidCompanyAccount(CSocketServer *pSock, CSocketClient *pClient, const char *sCompanyName, char *sSHA1Pwd)
{
	_ConnectedClientInfo *pCCI = (_ConnectedClientInfo *)pClient->pUserData;

	char sSQL[255];
    char sPwd[1024 + 1];

	CBoundRecordSet rsTemp;

	sprintf_s(sSQL, sizeof(sSQL), 
		"EXEC GetBasicCompanyInfoByName '%s'",
		sCompanyName);

	if(!pCCI->cIndexSQL.Execute(sSQL, &rsTemp))
    {
        WriteLog(pClient->PeerID(), "Error in IsValidCompanyAccount :: Execute failed.", EVENT_ERROR);
        return false;
    }

    if(rsTemp.RowCount < 1) //No matching company name
	{
		rsTemp.Close();
		return false;
	}

	if(!rsTemp.Fetch())
    {
        WriteLog(pClient->PeerID(), "Error in IsValidCompanyAccount :: DBFetch failed.", EVENT_ERROR);
        rsTemp.Close();
        return false;
    }

	//If the client active? (Enabled)
	if(!rsTemp.Values("Active")->ToBoolean())
	{
	    rsTemp.Close();
		return false;
	}

	//Get the company password.
	if(!rsTemp.Values("Password")->ToString(sPwd, sizeof(sPwd)))
	{
	    WriteLog(pClient->PeerID(),
			"Error in IsValidCompanyAccount :: Failed to get Company password.", EVENT_ERROR);
        rsTemp.Close();
        return false;
	}

	char sSHA1HAsh[41];

	if(!SimpleSHA1(sPwd, sSHA1HAsh, sizeof(sSHA1HAsh)))
	{
		WriteLog(pClient->PeerID(), "Failed to hash the random key.", EVENT_WARN);
		pClient->Disconnect();
		return 0;
	}

	//Check the password.
	if(_strcmpi(sSHA1HAsh, sSHA1Pwd) != 0)
	{
	    rsTemp.Close();
		return false;
	}

	//If the client set to be initilized? If so, what step?
	pCCI->iInitStep = rsTemp.Values("InitializeStep")->ToIntegerS();

	//If the client set to be initilized? If so, where do we start the replication?
	if(pCCI->iInitStep > -1)
	{
		pCCI->bRequestInit = true;
	}
	else {
		pCCI->bRequestInit = false;
	}

	//Grab the designated "Database Prefix" for the customer?
	if(!rsTemp.Values("DatabasePrefix")->ToString(pCCI->sCompanyDBPrefix, sizeof(pCCI->sCompanyDBPrefix)))
	{
	    WriteLog(pClient->PeerID(),
			"Error in IsValidCompanyAccount :: Failed to get Company DB prefix.", EVENT_ERROR);
        rsTemp.Close();
        return false;
	}

	sprintf_s(sSQL, sizeof(sSQL), 
		"[%s].[%s].SetBasicCompanyInfoByName"
		" @CompanyName = '%s',"
		" @IPAddress= '%s'",
		gsSQLIndexDatabase, gsDefaultDBO,
		pCCI->sCompanyName, pClient->PeerAddress());

	pCCI->cIndexSQL.Execute(sSQL);

	rsTemp.Close();

    return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int IsValidClientVersion(CSocketServer *pSock, CSocketClient *pClient, char *sOutUpdateURL, int iMaxSz)
{
	_ConnectedClientInfo *pCCI = (_ConnectedClientInfo *)pClient->pUserData;

	char sSQL[255];

    char sVerStatus[25 + 1];

	CBoundRecordSet rsTemp;

	sprintf_s(sSQL, sizeof(sSQL), 
		"EXEC GetClientVersionInfo '%s'",
		pCCI->sClientVersion);

	if(!pCCI->cIndexSQL.Execute(sSQL, &rsTemp))
    {
		WriteLog(pClient->PeerID(), "Error in IsValidClientVersion :: Execute failed.", EVENT_ERROR);
        return CLIENTVER_ERROR;
    }

	if(rsTemp.Fetch())
	{
		rsTemp.Values("Status")->ToString(sVerStatus, sizeof(sVerStatus));
		rsTemp.Values("UpdateURL")->ToString(sOutUpdateURL, iMaxSz);
	}
	else{
		WriteLog(pClient->PeerID(), "Error in IsValidClientVersion :: Unknown client version.", EVENT_ERROR);
        rsTemp.Close();
        return CLIENTVER_ERROR;
	}

    rsTemp.Close();

    if(_strcmpi((char *)sVerStatus, "current") == 0){ // Current version
        return CLIENTVER_CURRENT;
    }
    else if(_strcmpi((char *)sVerStatus, "supported") == 0){ // Old but supported
        return CLIENTVER_SUPPORTED;
    }
    else if(_strcmpi((char *)sVerStatus, "outdated") == 0){ // Old and nolonger supported
        return CLIENTVER_OUTDATED;
    }
    else if(_strcmpi((char *)sVerStatus, "autoupdate") == 0){ // Old and nolonger supported
        return CLIENTVER_AUTOUPDATE;
    }
    else{
		WriteLog(pClient->PeerID(), "Error in IsValidClientVersion :: Unknown version identifier.", EVENT_ERROR);
        return CLIENTVER_ERROR;
    }

    return CLIENTVER_ERROR;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#endif

