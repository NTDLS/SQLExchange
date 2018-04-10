///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//  Copyright © NetworkDLS 2002, All rights reserved
//
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF 
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO 
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A 
// PARTICULAR PURPOSE.
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef _Command_CPP
#define _Command_CPP
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include <Windows.H>
#include <WindowsX.H>
#include <ShellAPI.H>
#include <Stdio.H>
#include <Stdlib.H>

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "../../../@Libraries/CSocketServer/CSocketServer.H"

#include "../../../@Libraries/CXML/CXML.H"

#include "../../SharedSource/FileExchange.H"

#include "Entry.H"
#include "Command.H"
#include "Console.H"

#include "SQLImport.H"
#include "NewDB.H"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

using namespace NSWFL::File;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*
	int ProcessCommand(CSocketServer *pSock, CSocketClient *pClient, BASICHUNK *lpCmd)

	Processes commands from the peer.

	Possible return Values:
		CMD_OK    The command was a success, all is well.
		CMD_DONE  The command was a success, close the connection.
		CMD_ERROR The command failed due to an error.
*/
int ProcessCommand(CSocketServer *pSock, CSocketClient *pClient, BASICHUNK *lpCmd)
{
	_ConnectedClientInfo *pCCI = ((_ConnectedClientInfo *)pClient->pUserData);

	char *sTextPtr = NULL;
	char sText[5120];

	/*-----------------------------------------------------------------------------------------------------------------------
		In response to:
			::AuthenticationSuccess
		
		The client is telling us that is done with authentication and ready to start processing commands.

		Notes:
			1) Send any settings to the client here.
			2) The client will respond with its own <::Settings>.
	*/
	if(pSock->IsCommand(lpCmd, "::AwaitingCommand"))
    {
		if(gbDebugMode)
		{
			WriteLog(pClient->PeerID(), "Client is ready and awaiting commands.", EVENT_NONE);
		}

		//Send settings to peer.
		CXMLWriter MyWXML("::Settings");
		MyWXML.Add("CompressionMethod", gsCompressionMethod);
		MyWXML.Add("CompressionLevel", gdwCompressionLevel);
		SendCommand(pClient, &MyWXML);
		MyWXML.Destroy();

		if(_strcmpi(gsCompressionMethod, "None") == 0)
		{
			WriteLog(pClient->PeerID(),
				"Compression is disabled. This process will overwork the network.", EVENT_WARN);
		}

		return CMD_OK;
	}
	/*-----------------------------------------------------------------------------------------------------------------------
		In response to:
			::ReplicateTransactions
		
		The client is telling us what it would "like" to do before we start the normal process.

		Notes:
	*/
	else if(pSock->IsCommand(lpCmd, "::TransactionReplicationComplete"))
	{
		WriteLog(pClient->PeerID(),
			"Transaction replication is complete.", EVENT_NONE);
		return CMD_DONE;
	}
	/*-----------------------------------------------------------------------------------------------------------------------
		In response to:
			::ClientSideRequest
		
		The client is telling us what it would "like" to do before we start the normal process.

		Notes:
	*/
	else if(pSock->ParseCommand(lpCmd, "<::ClientSideRequest>"))
    {
		if(gbDebugMode)
		{
			WriteLog(pClient->PeerID(), "Received client processing request.", EVENT_NONE);
		}

		char sRequest[255];

		//Read the request.
		CXMLReader MyRXML((char *)lpCmd->pBuffer, lpCmd->iLength);
		MyRXML.ToString("Request", sRequest, sizeof(sRequest));
		MyRXML.Destroy();

		if(_strcmpi(sRequest, "None") == 0 || _strcmpi(sRequest, "Initialization") == 0)
		{
			//The client has nothing to request or is requesting database initialization.

			if(_strcmpi(sRequest, "Initialization") == 0)
			{
				//If the client requests initialization, start from square one.
				pCCI->bRequestInit = true;
				pCCI->iInitStep = 0;

				//Update the company record to set the initialization step to 0.
				sprintf_s(sText, sizeof(sText),
					"EXEC [%s].[%s].SetBasicCompanyInfoByName"
					" @CompanyName = '%s',"
					" @InitializeStep = 0",
					gsSQLIndexDatabase, gsDefaultDBO,
					pCCI->sCompanyName);
				pCCI->cIndexSQL.Execute(sText);
			}

			//Do we need to initialize the database?
			if(pCCI->bRequestInit)
			{
				//If the iInitStep is 0, then we need to request a fresh database structure.
				if(pCCI->iInitStep == 0)
				{
					WriteLog(pClient->PeerID(),
						"Requesting database structures from client.", EVENT_NONE);

					//Send request for the database structures.
					pClient->SetNextSendData("::InitializeStructures");
				}
				else{
					WriteLog(pClient->PeerID(),
						"Resuming a previously broken initialization process.", EVENT_WARN);

					//Send request for all data.
					CXMLWriter MyWXML("::InitializeData");
					MyWXML.Add("Step", pCCI->iInitStep);
					SendCommand(pClient, &MyWXML);
					MyWXML.Destroy();
				}
			}
			else{
				//Tell the client to start the normal replication process.
				pClient->SetNextSendData("::ReplicateTransactions");
			}

			return CMD_OK;
		}
		else{
			sprintf_s(sText, sizeof(sText), "Received an unknown client request: [%s]", sRequest);
			WriteLog(pClient->PeerID(), sText, EVENT_WARN);
			return CMD_ERROR;
		}

		return CMD_ERROR;
	}
	/*-----------------------------------------------------------------------------------------------------------------------
		In response to:
			"<::Settings>" 

		The client is responding with its settings after we gave it ours.

		Notes:
			1) Ask the client if it has anything to request.
	*/
	else if(pSock->ParseCommand(lpCmd, "<::Settings>"))
    {
		if(gbDebugMode)
		{
			WriteLog(pClient->PeerID(), "Received client settings.", EVENT_NONE);
		}

		//Read settings from peer.
		CXMLReader MyRXML((char *)lpCmd->pBuffer, lpCmd->iLength);
		MyRXML.ToBoolean("AllowOffLineDataProcessing", &pCCI->bAllowOffLineDataProcessing);
		MyRXML.Destroy();

		//Ask the client if it has anything to request.
		pClient->SetNextSendData("::ClientSideRequest");

		return CMD_OK;
	}
	/*-----------------------------------------------------------------------------------------------------------------------
		In response to:
			"<::InitializeData>" 

		The client is letting us know that there are no more tables to dump data from. It's done.

		Notes:
	*/
	else if(pSock->IsCommand(lpCmd, "::InitializationComplete"))
    {
		WriteLog(pClient->PeerID(),
			"Initialization completed successfully.", EVENT_NONE);

		char sSQL[1024];
		sprintf_s(sSQL, sizeof(sSQL), 
			"EXEC [%s].[%s].SetCompanyInitStepByName"
			" @CompanyName = '%s',"
			" @InitializeStep = -1",
			gsSQLIndexDatabase, gsDefaultDBO, pCCI->sCompanyName);
		pCCI->cIndexSQL.Execute(sSQL);

		//FixMe: Request replications here. No need to disconnect.

		return CMD_DONE;
	}
	/*-----------------------------------------------------------------------------------------------------------------------
		In response to:
			::InitializeData
		
		The client is sending us the data for a table.

		Notes:
	*/
	else if(pSock->ParseCommand(lpCmd, "<::Sending::TransactionData>"))
    {
		int iResult = CMD_OK;

		char sTempFile[MAX_PATH];

		int iInitializeStep = 0;
		char sDatabase[255];
		char sSchema[255];
		char sTable[255];

		CXMLReader MyRXML((char *)lpCmd->pBuffer, lpCmd->iLength);
		MyRXML.ToString("Database", sDatabase, sizeof(sDatabase));
		MyRXML.ToString("Schema", sSchema, sizeof(sSchema));
		MyRXML.ToString("Table", sTable, sizeof(sTable));
		MyRXML.ToInteger("InitializeStep", &iInitializeStep);

		if(MyRXML.ToBoolean("Compressed"))
		{
			char sFriendlySize[64];
			FileSizeFriendly(MyRXML.ToI64("CompressedFileSize"), sFriendlySize, sizeof(sFriendlySize));

			sprintf_s(sText, sizeof(sText), "Receiving transaction data for [%s].[%s].[%s] (%s)",
				sDatabase, sSchema, sTable, sFriendlySize);
			WriteLog(pClient->PeerID(), sText, EVENT_NONE);
		}
		else{
			char sFriendlySize[64];
			FileSizeFriendly(MyRXML.ToI64("FileSize"), sFriendlySize, sizeof(sFriendlySize));

			sprintf_s(sText, sizeof(sText), "Receiving transaction data for [%s].[%s].[%s] (%s)",
				sDatabase, sSchema, sTable, sFriendlySize);
			WriteLog(pClient->PeerID(), sText, EVENT_NONE);
		}

		GetTempFileName(gsImportTempDir, "WPD", 0, sTempFile);

		MyRXML.Destroy();

		MyRXML.Initialize((char *)lpCmd->pBuffer, lpCmd->iLength);
		int iFileExchResult = ReceiveFileData(pSock, pClient, &MyRXML, sTempFile);
		MyRXML.Destroy();

		if(iFileExchResult == FILEEXCHRESULT_OK)
		{
			int iID = 0;

			bool bOldThrowValue = pCCI->cCustSQL.ThrowErrors(false);
			int iImportResult = ImportSQLResults(pSock, pClient, pCCI, sTempFile);
			pCCI->cCustSQL.ThrowErrors(bOldThrowValue);

			if(iImportResult == IMPORT_RES_OFFLINEOK)
			{
				//We have been disconnected and completed off-line data processing.
				WriteLog(pClient->PeerID(),
					"Off-line data processing complete, cleaning up the connection.", EVENT_INFO);
			}
			else{
				CXMLWriter MyWXML("::ImportResult");
				if(iImportResult == IMPORT_RES_OK)
				{
					MyWXML.Add("Result", "OK");

					if(!CCI->cCustSQL.Focus("master"))
					{
						WriteLog(pClient->PeerID(),
							"Failed to change master database context. Performance will be affected.", EVENT_WARN);
					}
				}
				else if(iImportResult == IMPORT_RES_SYNCTABLE)
				{
					if(pCCI->bRequestInit)
					{
						//FixMe: Implement RESYNC durring Initialization (not critical).
						WriteLog(pClient->PeerID(),
							"Resync is not supported durring Initialization.", EVENT_ERROR);
						iResult = CMD_ERROR;
					}
					else{
						//Request a new copy of the table creation script.
						MyWXML.Add("Result", "RESYNC");
						MyWXML.Add("Database", sDatabase);
						MyWXML.Add("Schema", sSchema);
						MyWXML.Add("Table", sTable);
					}
				}
				else{
					MyWXML.Add("Result", "FAIL");
					iResult = CMD_ERROR;
				}
				SendCommand(pClient, &MyWXML);
				MyWXML.Destroy();
			}
		}
		else{
			WriteLog(pClient->PeerID(),
				"Failed to receive transactional data from the client.", EVENT_NONE);
			iResult = CMD_ERROR;
		}

		DeleteFile(sTempFile);

		return iResult;
	}
	/*-----------------------------------------------------------------------------------------------------------------------
		In response to:
			::InitializeStructures
		
		The client is giving us the script required to create all of the replicating databases.

		Notes:
	*/
	else if(pSock->ParseCommand(lpCmd, "<::Sending::DatabaseCreationScript>"))
    {
		char sTempFile[MAX_PATH];
		int iResult = 0;

		GetTempFileName(gsImportTempDir, "WPD", 0, sTempFile);

		//Read the XML from (char *)lpCmd->pBuffer, it contains data needed by ReceiveFileData.
		CXMLReader MyOpt((char *)lpCmd->pBuffer, lpCmd->iLength);
		iResult = ReceiveFileData(pSock, pClient, &MyOpt, sTempFile);
		MyOpt.Destroy();

		if(iResult == FILEEXCHRESULT_OK)
		{
			//Create the client database structure skeleton.
			bool bResult = CreateNewDBFromDBCreationScript
				(&pCCI->cCustSQL, pSock, pClient, sTempFile);

			DeleteFile(sTempFile);

			if(bResult)
			{
				WriteLog(pClient->PeerID(),
					"Database structure created successfully. Requesting data....", EVENT_NONE);

				//Send request for all data.
				CXMLWriter MyWXML("::InitializeData");
				MyWXML.Add("Step", 0);
				SendCommand(pClient, &MyWXML);
				MyWXML.Destroy();
			}
			else{
				sTextPtr = "Failed to create the replication target database.";
				WriteLog(pClient->PeerID(), sTextPtr, EVENT_ERROR);

				sprintf_s(sText, sizeof(sText), "::Msg->%s", sTextPtr);
				pClient->SetNextSendData(sText);
				return CMD_ERROR;
			}

			return CMD_OK;
		}

		return CMD_ERROR;
	}

	//------------------------------------------------------------------------------------------------
	WriteLog(pClient->PeerID(), "Received unknown command.", EVENT_ERROR);
	//------------------------------------------------------------------------------------------------

	if(lpCmd->iLength > 1024)
	{
		((char *)lpCmd->pBuffer)[1024] = '\0';
	}
	WriteLog(pClient->PeerID(), (char *)lpCmd->pBuffer, EVENT_ERROR);

	return CMD_ERROR;	
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool GetClientThreadTime(HANDLE hThread, double *iKernelTime, double *iUserTime)
{
	FILETIME CreationTime;
	FILETIME ExitTime;
	FILETIME KernelTime;
	FILETIME UserTime;

	if(GetThreadTimes(GetCurrentThread(), &CreationTime, &ExitTime, &KernelTime, &UserTime) )
	{
		LARGE_INTEGER li;

		li.LowPart = KernelTime.dwLowDateTime;
		li.HighPart = KernelTime.dwHighDateTime;
		*iKernelTime = ((double)(li.QuadPart) / (10 * 1000 * 1000));

		li.LowPart = UserTime.dwLowDateTime;
		li.HighPart = UserTime.dwHighDateTime;
		*iUserTime = ((double)(li.QuadPart) / (10 * 1000 * 1000));

		return true;
	}

	return false;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool UpdateClientStats(CSocketServer *pSock, CSocketClient *pClient, CSQL *lpSQL)
{
	_ConnectedClientInfo *pCCI = ((_ConnectedClientInfo *)pClient->pUserData);

	SYSTEMTIME ST;

	char sSQL[2048];
	char lsDate[64];
	char lsTime[64];

	char sConnectTime[128];
	char sDisconnectTime[128];

	GetDateFormat(NULL, 0, &pCCI->stConnectTime, "MM/dd/yyyy", lsDate, sizeof(lsDate));
	GetTimeFormat(NULL, 0, &pCCI->stConnectTime, "HH:mm:ss", lsTime, sizeof(lsTime));
	sprintf_s(sConnectTime, sizeof(sConnectTime), "%s %s", lsDate, lsTime);

	GetLocalTime(&ST);
	GetDateFormat(NULL, 0, &ST, "MM/dd/yyyy", lsDate, sizeof(lsDate));
	GetTimeFormat(NULL, 0, &ST, "HH:mm:ss", lsTime, sizeof(lsTime));
	sprintf_s(sDisconnectTime, sizeof(sDisconnectTime), "%s %s", lsDate, lsTime);

	double dKernelTime = 0;
	double dUserTime = 0;

	if(!GetClientThreadTime(pClient->ThreadHandle(), &dKernelTime, &dUserTime))
	{
		dKernelTime = -1;
		dUserTime = -1;
	}

	sprintf_s(sSQL, sizeof(sSQL),
		"EXEC [%s].[%s].InsertCompanyStatistics"
		" @CompanyName = '%s',"
		" @ConnectTime = '%s',"
		" @DisconnectTime = '%s',"
		" @BytesSent = '%d',"
		" @BytesReceived = '%d',"
		" @KernelTime = '%f',"
		" @UserTime = '%f',"
		" @WasInitialize = '%d'",
		gsSQLIndexDatabase, gsDefaultDBO, pCCI->sCompanyName,
		sConnectTime, sDisconnectTime, pCCI->dwBytesSent,
		pCCI->dwBytesRecv, dKernelTime, dUserTime, pCCI->bRequestInit);

	lpSQL->Execute(sSQL);

	return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool ResetOfflineTableName(CSocketServer *pSock, CSocketClient *pClient)

{
	_ConnectedClientInfo *pCCI = ((_ConnectedClientInfo *)pClient->pUserData);

	char sTemp[2048];
	sprintf_s(sTemp, sizeof(sTemp),
		"EXEC [%s].[%s].SetOfflineProcessingByCompanyName"
		" @CompanyName = '%s',"
		" @DatabaseName = NULL,"
		" @SchemaName = NULL,"
		" @TableName = NULL",
		gsSQLIndexDatabase, gsDefaultDBO, pCCI->sCompanyName);

	return pCCI->cIndexSQL.Execute(sTemp);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool SendLastOfflineProcessedTableName(CSocketServer *pSock, CSocketClient *pClient)
{
	_ConnectedClientInfo *pCCI = ((_ConnectedClientInfo *)pClient->pUserData);

	char sTemp[2048];
	int iOutSz = 0;

	bool bSent = false;
	CBoundRecordSet rsTemp;

	sprintf_s(sTemp, sizeof(sTemp), 
		"EXEC [%s].[%s].GetOfflineProcessingTableByCompanyName"
		" @CompanyName = '%s'",
		gsSQLIndexDatabase, gsDefaultDBO, pCCI->sCompanyName);
	pCCI->cIndexSQL.Execute(sTemp, &rsTemp);

	if(rsTemp.Fetch())
	{
		iOutSz = (int)rsTemp.Values("TableName")->ToString(sTemp, sizeof(sTemp));

		if(iOutSz > 0)
		{
			iOutSz = (int)strlen(sTemp);
			if(iOutSz > 0 && iOutSz < sizeof(sTemp))
			{
				char sSend[2048];
				sprintf_s(sSend, sizeof(sSend), "::DeleteLastReplication->%s", sTemp);
				bSent = pClient->SetNextSendData(sSend);
			}
		}
	}

	rsTemp.Close();

	return ResetOfflineTableName(pSock, pClient) && bSent;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#endif
