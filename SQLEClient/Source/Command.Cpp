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

#include "../../../@Libraries/CXML/CXML.H"
#include "../../SharedClasses/CGetPKs/CGetPKs.H"

#include "../../SharedSource/FileExchange.H"

#include "Entry.H"
#include "Command.H"
#include "Console.H"

#include "SQLExport.H"
#include "DBInit.H"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/*
	sTemp is passed in to be used as shared RAM, it contains no usable value.
*/
int SendTransactionData(CSocketServer *pSock, _ConnectedClientInfo *lpCCI,
		CXMLWriter *lpXML, LPBASICHUNK lpCmd, char *sFileName)
{
	SimpleTransfer(pSock, lpCCI->pClient, sFileName, lpXML);

	if(lpCCI->pClient->GetNextClientData(lpCmd))
	{
		if(pSock->ParseCommand(lpCmd, "<::ImportResult>"))
		{
			bool bImportResult = false;
			char sResult[255];

			//Read the import result. (OK, FAIL, RESYNC)
			CXMLReader MyRXML((char *)lpCmd->pBuffer, lpCmd->iLength);
			MyRXML.ToString("Result", sResult, sizeof(sResult));
			MyRXML.Destroy();

			if(_strcmpi(sResult, "OK") == 0)
			{
				WriteLog(lpCCI->pClient->PeerID(),
					"The server successfully imported the transaction data.", EVENT_NONE);
			}
			else if(_strcmpi(sResult, "RESYNC") == 0)
			{
				//FIXFIX: Implement RESYNC durring Initialization (not critical).
				WriteLog(lpCCI->pClient->PeerID(),
					"Resync is not yet implemented.", EVENT_ERROR);
				return CMD_ERROR;
			}
			else if(_strcmpi(sResult, "FAIL") == 0)
			{
				WriteLog(lpCCI->pClient->PeerID(),
					"The server failed to import the transaction data.", EVENT_ERROR);
				return CMD_ERROR;
			}
			else{
				WriteLog(lpCCI->pClient->PeerID(),
					"Received an unknown import result from the server.", EVENT_ERROR);
				return CMD_ERROR;
			}
		}
		else{
			WriteLog(lpCCI->pClient->PeerID(),
				"Received invalid command while waiting on the import confirmation.", EVENT_WARN);
			return CMD_ERROR;
		}
	}
	else{
		WriteLog(lpCCI->pClient->PeerID(),
			"Failed to receive import confirmation from the server.", EVENT_WARN);
		return CMD_ERROR;
	}

	return CMD_OK;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/*
	sTemp is passed in to be used as shared RAM, it contains no usable value.
*/
int InitializeSQLData(CSocketServer *pSock,
						  _ConnectedClientInfo *lpCCI, LPBASICHUNK lpCmd, char *sTempFile)

{
	char sTemp[5120];

	int iResult = CMD_OK;

	sprintf_s(sTemp, sizeof(sTemp), 
		"SELECT"
		"	S.[ID], S.[Database], S.[Schema], S.[Table]"
		" FROM"
		"	[%s].[%s].[SQLExch_Replications] AS S"
		" WHERE"
		"	S.[ID] > %d"
		" ORDER BY"
		"	S.[ID] ASC",
		gsReplicationDB, gsDefaultDBO, lpCCI->iInitStep);

	CBoundRecordSet rsTables;

	if(lpCCI->cCustSQL.Execute(sTemp, &rsTables))
	{
		while(rsTables.Fetch())
		{
			int iInitializeStep = rsTables.Values("ID")->ToIntegerS();
			char *sDatabase = rsTables.Values("Database")->ToString();
			char *sSchema = rsTables.Values("Schema")->ToString();
			char *sTable = rsTables.Values("Table")->ToString();

			//----------------------------------------------------------------------------------------------

			sprintf_s(sTemp, sizeof(sTemp),
				"SELECT\n"
				"	SysC.name AS [ColumnName],\n"
				"	CASE WHEN UPPER(SysT.name) IN ('CHAR', 'NCHAR') THEN 1 ELSE 0 END AS [TrimType]\n"
				"FROM\n"
				"	[%s].[%s].sysobjects AS SysO\n"
				"INNER JOIN [%s].[%s].syscolumns AS SysC\n"
				"	ON SysC.ID = SysO.ID\n"
				"INNER JOIN [%s].[%s].systypes AS SysT\n"
				"	ON SysT.xusertype = SysC.xusertype\n"
				"WHERE\n"
				"	SysO.id = object_id('[%s].[%s].[%s]')\n",
			sDatabase, gsDefaultDBO, sDatabase, gsDefaultDBO, sDatabase, gsDefaultDBO, sDatabase, sSchema, sTable);

			CStringBuilder SQL("SELECT ");

			CRecordSet rsColumns;
			if(lpCCI->cCustSQL.Execute(sTemp, &rsColumns))
			{
				SQLLEN iDataLength = 0;
				while(rsColumns.Fetch())
				{
					if(rsColumns.sColumnEx(1, sTemp, sizeof(sTemp), &iDataLength)) //ColumnName
					{
						bool bTrim = (rsColumns.lColumn(2) > 0); //TrimType
						if(bTrim)
						{
							SQL.Append("RTrim(");
						}
						SQL.Append("[");
						SQL.Append(sTemp);
						SQL.Append("]");
						if(bTrim)
						{
							SQL.Append(") AS [");
							SQL.Append(sTemp);
							SQL.Append("]");
						}
						SQL.Append(",");
					}
					else{
						WriteLog(lpCCI->pClient->PeerID(),
							"Failed to get export table column.", EVENT_WARN);
						SQL.Destroy();
						return CMD_ERROR;
					}
				}
				SQL.Length--; //Remove the last comma.
				SQL.Terminate(); //Remove the last comma.
			}
			else{
				WriteLog(lpCCI->pClient->PeerID(),
					"Failed to get export table column collection.", EVENT_WARN);
				SQL.Destroy();
				return CMD_ERROR;
			}

			//Next, grab the pending inserts/updates.
			sprintf_s(sTemp, sizeof(sTemp), "\n FROM [%s].[%s].[%s]\n", sDatabase, sSchema, sTable);

			SQL.Append(sTemp);
			SQL.Terminate();

			//----------------------------------------------------------------------------------------------

			int iExportResult = ExportSQLResults
				(pSock, lpCCI->pClient, iInitializeStep, SQL.Buffer, sDatabase, sSchema, sTable, OPERATION_INSERT, sTempFile);

			SQL.Destroy();

			if(iExportResult == SQL_EXPORT_RESULT_ZEROROWS)
			{
				//Nothing do do.
			}
			else if(iExportResult == SQL_EXPORT_RESULT_OK){
				CXMLWriter MyWXML("TransactionData");
				MyWXML.Add("InitializeStep", iInitializeStep);
				MyWXML.Add("Database", sDatabase);
				MyWXML.Add("Schema", sSchema);
				MyWXML.Add("Table", sTable);
				if((iResult = SendTransactionData(pSock, lpCCI, &MyWXML, lpCmd, sTempFile)) != CMD_OK)
				{
					//An error has occured.
					MyWXML.Destroy();
					break; 
				}
				MyWXML.Destroy();
			}
			else if(iExportResult == SQL_EXPORT_RESULT_ERROR)
			{
				//Error or Warning message was written by ExportSQLResults().
				iResult = CMD_ERROR;
				break;
			}
		}

		rsTables.Close();
	}

	if(iResult == CMD_OK)
	{
		if(!CCI.cCustSQL.Focus("master"))
		{
			WriteLog(lpCCI->pClient->PeerID(),
				"Failed to change the master database context. Performance will be affected.", EVENT_WARN);
		}

		WriteLog(lpCCI->pClient->PeerID(),
			"Initialization completed successfully", EVENT_NONE);

		lpCCI->pClient->SetNextSendData("::InitializationComplete");
	}

	return iResult;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/*
	sTemp is passed in to be used as shared RAM, it contains no usable value.
*/
int InitializeStructures(CSocketServer *pSock,
						  _ConnectedClientInfo *lpCCI, char *sTempFile)
{
	int iResult = GenerateDBCreationSQL(&lpCCI->cCustSQL, pSock, lpCCI->pClient, sTempFile);
	if(iResult == CMD_OK)
	{
		if(SimpleTransfer(pSock, lpCCI->pClient, sTempFile, "DatabaseCreationScript") == FILEEXCHRESULT_OK)
		{
			return CMD_OK;
		}
		else{
			WriteLog(lpCCI->pClient->PeerID(),
				"Failed to send the database creation scripts.", EVENT_ERROR);
			return CMD_ERROR;
		}
	}
	else if(iResult == CMD_ERROR){
		WriteLog(lpCCI->pClient->PeerID(),
			"Failed to generate initialization scripts.", EVENT_ERROR);
		return CMD_ERROR;
	}

	return iResult;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/*
	lpCmd is passed in to be used as shared RAM, it contains no usable value.
*/
int ReplicateTransactions(CSocketServer *pSock, _ConnectedClientInfo *lpCCI, LPBASICHUNK lpCmd, char *sTempFile)
{
	CBoundRecordSet rsReplicate;

	char sTemp[5120];

	sprintf_s(sTemp, sizeof(sTemp),
		//We don't want to reserve any more replications if they are any already reserved.
		//	We MUST replicate in a transactional fashion.
		"IF NOT EXISTS (SELECT TOP 1 1 FROM [%s].[%s].SQLExch_Transactions WHERE [Locked] = 1)\n"
		"BEGIN\n"
			//Update the replication status, this will reserve the records for the replication process.
		"	UPDATE [%s].[%s].SQLExch_Transactions SET [Locked] = 1\n"
		"END\n",
		gsReplicationDB, gsDefaultDBO, gsReplicationDB, gsDefaultDBO);
	if(!lpCCI->cCustSQL.Execute(sTemp))
	{
		WriteLog(lpCCI->pClient->PeerID(),
			"Failed to lock pending replication transactions.", EVENT_ERROR);
		return CMD_ERROR;
	}

	sprintf_s(sTemp, sizeof(sTemp),
		//Get the Database, Schema & Tables that have pending replications.
		"SELECT DISTINCT\n"
		"	[Database],\n"
		"	[Schema],\n"
		"	[Table]\n"
		"FROM\n"
		"	[%s].[%s].SQLExch_Transactions\n"
		"WHERE\n"
		"	[Locked] = 1\n",
		gsReplicationDB, gsDefaultDBO);
	if(lpCCI->cCustSQL.Execute(sTemp, &rsReplicate))
	{
		if(rsReplicate.RowCount > 0)
		{
			//Loop through the pending replications, processing each one.
			while(rsReplicate.Fetch())
			{
				char sDatabase[256];
				char sSchema[256];
				char sTable[256];

				int iExportResult = 0;

				rsReplicate.Values("Database")->ToString(sDatabase, sizeof(sDatabase));
				rsReplicate.Values("Schema")->ToString(sSchema, sizeof(sSchema));
				rsReplicate.Values("Table")->ToString(sTable, sizeof(sTable));

				sprintf_s(sTemp, sizeof(sTemp),
					//We don't want to reserve any more replications if there are any already reserved.
					//	We MUST replicate in a transactional fashion.
					"IF NOT EXISTS (SELECT TOP 1 1 FROM [%s].[%s].[%s_%s] WHERE SQLExch_Locked = 1)\n"
					"BEGIN\n"
						//Update the replication status, this will reserve the records for the replication process.
					"	UPDATE [%s].[%s].[%s_%s] SET SQLExch_Locked = 1\n"
					"END\n",
					gsReplicationDB, sSchema, sDatabase, sTable,
					gsReplicationDB, sSchema, sDatabase, sTable);
				if(!lpCCI->cCustSQL.Execute(sTemp))
				{
					WriteLog(lpCCI->pClient->PeerID(),
						"Failed to lock pending replications.", EVENT_ERROR);
					rsReplicate.Close();
					return CMD_ERROR;
				}

				CGetPKs MyPKs;

				CStringBuilder sPKJoin;
				CStringBuilder sPKList;

				//Create the JOIN cretirea to join the actual data table to the table's replication table.
				//	This loop also generates a cona sepearted list of the PK columns (with the char & nchars trimmed).
				if(MyPKs.Get(&lpCCI->cCustSQL, sDatabase, sSchema, sTable))
				{
					for(int iPK = 0; iPK < MyPKs.Columns.Count; iPK++)
					{
						bool bTrimType = //We want to tell SQL server to RTrim() fixed length char fields.
							_strcmpi(MyPKs.Columns.Array[0].DataType, "char") == 0 ||
							_strcmpi(MyPKs.Columns.Array[0].DataType, "nchar") == 0;

						if(bTrimType)
						{
							sPKList.Append("RTrim(");
						}

						sPKList.Append("[");
						sPKList.Append(MyPKs.Columns.Array[iPK].Name);
						sPKList.Append("]");

						if(bTrimType)
						{
							sPKList.Append(") AS [");
							sPKList.Append(MyPKs.Columns.Array[iPK].Name);
							sPKList.Append("]");
						}

						sPKJoin.AppendF("[U].[%s] = [D].[%s]", MyPKs.Columns.Array[iPK].Name, MyPKs.Columns.Array[iPK].Name);

						if(iPK < (MyPKs.Columns.Count - 1))
						{
							sPKList.Append(", ");
							sPKJoin.Append(" AND ");
						}
					}
					MyPKs.Free();
				}
				else{
					rsReplicate.Close();

					sPKJoin.Destroy();
					sPKList.Destroy();
					return CMD_ERROR;
				}

				sprintf_s(sTemp, sizeof(sTemp),
					//Remove "Delete" replications that have Insert/Update replications for the same keys.
					"DELETE\n"
					"	D\n"
					"FROM\n"
					"	[%s].[%s].[%s_%s] AS D\n"
					"INNER JOIN [%s].[%s].[%s_%s] AS U\n"
					"	ON (%s)\n"
					"	AND U.SQLExch_Locked = 1\n"
					"	AND U.SQLExch_Action = 0\n"
					"WHERE\n"
					"	D.SQLExch_Locked = 1\n"
					"	AND D.SQLExch_Action = 1\n",
					gsReplicationDB, sSchema, sDatabase, sTable,
					gsReplicationDB, sSchema, sDatabase, sTable,
					sPKJoin.Buffer);

				if(!lpCCI->cCustSQL.Execute(sTemp))
				{
					WriteLog(lpCCI->pClient->PeerID(),
						"Failed to remove duplicate replications.", EVENT_ERROR);

					rsReplicate.Close();

					sPKJoin.Destroy();
					sPKList.Destroy();
					return CMD_ERROR;
				}

				CBoundRecordSet rsTrans;

				sprintf_s(sTemp, sizeof(sTemp),
					//Grab the pending deletes, replicate them first.
					"SELECT\n"
					"	%s\n"
					"FROM\n"
					"	[%s].[%s].[%s_%s]\n"
					"WHERE\n"
					"	SQLExch_Locked = 1\n"
					"	AND SQLExch_Action = 1\n", //Deletes
					sPKList.Buffer, gsReplicationDB, sSchema, sDatabase, sTable);

				iExportResult = ExportSQLResults
					(pSock, lpCCI->pClient, 0, sTemp, sDatabase, sSchema, sTable, OPERATION_DELETE, sTempFile);

				if(iExportResult == SQL_EXPORT_RESULT_ZEROROWS)
				{
					//Nothing do do. Move along...
				}
				else if(iExportResult == SQL_EXPORT_RESULT_OK){
					//Transfer "delete" transactions.

					CXMLWriter MyWXML("TransactionData");
					MyWXML.Add("Database", sDatabase);
					MyWXML.Add("Schema", sSchema);
					MyWXML.Add("Table", sTable);
					MyWXML.Add("TransactionType", "DELETE");
					if(SendTransactionData(pSock, lpCCI, &MyWXML, lpCmd, sTempFile) != CMD_OK)
					{
						//An error has occured.
						rsReplicate.Close();

						sPKJoin.Destroy();
						sPKList.Destroy();
						MyWXML.Destroy();
						return CMD_ERROR; 
					}
					MyWXML.Destroy();
				}
				else if(iExportResult == SQL_EXPORT_RESULT_ERROR){
					//Error message is handled by ExportSQLResults.
					rsReplicate.Close();

					sPKJoin.Destroy();
					sPKList.Destroy();
					return CMD_ERROR;
				}

				sprintf_s(sTemp, sizeof(sTemp),
					//Get a list of all columsn in the source table so that we can identifiy which ones need to be trimmed.
					"SELECT\n"
					"	SysC.name AS [ColumnName],\n"
					"	CASE WHEN UPPER(SysT.name) IN ('CHAR', 'NCHAR') THEN 1 ELSE 0 END AS [TrimType]\n"
					"FROM\n"
					"	[%s].[%s].sysobjects AS SysO\n"
					"INNER JOIN [%s].[%s].syscolumns AS SysC\n"
					"	ON SysC.ID = SysO.ID\n"
					"INNER JOIN [%s].[%s].systypes AS SysT\n"
					"	ON SysT.xusertype = SysC.xusertype\n"
					"WHERE\n"
					"	SysO.id = object_id('[%s].[%s].[%s]')\n",
				sDatabase, sSchema, sDatabase, sSchema, sDatabase, sSchema, sDatabase, sSchema, sTable);

				CStringBuilder SQL("SELECT ");

				CRecordSet rsColumns;
				if(lpCCI->cCustSQL.Execute(sTemp, &rsColumns))
				{
					SQLLEN iDataLength = 0;
					//Loop through all columns in the table, trimming the nessisary ones.
					while(rsColumns.Fetch())
					{
						if(rsColumns.sColumnEx(1, sTemp, sizeof(sTemp), &iDataLength)) //ColumnName
						{
							bool bTrim = (rsColumns.lColumn(2) > 0); //Trim Type
							if(bTrim)
							{
								SQL.Append("RTrim(");
							}
							SQL.Append("U.[");
							SQL.Append(sTemp);
							SQL.Append("]");
							if(bTrim)
							{
								SQL.Append(") AS [");
								SQL.Append(sTemp);
								SQL.Append("]");
							}
							SQL.Append(",");
						}
						else{
							WriteLog(lpCCI->pClient->PeerID(),
								"Failed to get export table column.", EVENT_WARN);
							SQL.Destroy();
							rsReplicate.Close();
							sPKJoin.Destroy();
							sPKList.Destroy();
							return CMD_ERROR;
						}
					}
					SQL.Length--; //Remove the last comma.
					SQL.Terminate(); //Remove the last comma.
				}
				else{
					WriteLog(lpCCI->pClient->PeerID(),
						"Failed to get export table column collection.", EVENT_WARN);
					SQL.Destroy();
					rsReplicate.Close();
					sPKJoin.Destroy();
					sPKList.Destroy();
					return CMD_ERROR;
				}

				//Next, grab the pending inserts/updates.
				sprintf_s(sTemp, sizeof(sTemp),
					"\n" 
					"FROM\n" 
					"	[%s].[%s].[%s] AS U\n" 
					"INNER JOIN [%s].[%s].[%s_%s] AS D\n" //inserts/updates
					"	ON %s\n" 
					"	AND D.SQLExch_Locked = 1\n" 
					"	AND D.SQLExch_Action = 0\n",
					sDatabase, sSchema, sTable,
					gsReplicationDB, sSchema, sDatabase, sTable,
					sPKJoin.Buffer);

				SQL.Append(sTemp);
				SQL.Terminate();

				iExportResult = ExportSQLResults
					(pSock,lpCCI->pClient, 0, SQL.Buffer, sDatabase, sSchema, sTable, OPERATION_UPDATE, sTempFile);

				SQL.Destroy();

				if(iExportResult == SQL_EXPORT_RESULT_ZEROROWS)
				{
					//Nothing do do. Move along...
				}
				else if(iExportResult == SQL_EXPORT_RESULT_OK){
					//Transfer "insert/update" transactions.

					CXMLWriter MyWXML("TransactionData");
					MyWXML.Add("Database", sDatabase);
					MyWXML.Add("Schema", sSchema);
					MyWXML.Add("Table", sTable);
					MyWXML.Add("TransactionType", "UPDATE");
					if(SendTransactionData(pSock, lpCCI, &MyWXML, lpCmd, sTempFile) != CMD_OK)
					{
						//An error has occured.
						rsReplicate.Close();

						sPKJoin.Destroy();
						sPKList.Destroy();
						MyWXML.Destroy();
						return CMD_ERROR; 
					}
					MyWXML.Destroy();
				}
				else if(iExportResult == SQL_EXPORT_RESULT_ERROR){
					//Error message is handled by ExportSQLResults.
					rsReplicate.Close();

					sPKJoin.Destroy();
					sPKList.Destroy();
					return CMD_ERROR;
				}

				sPKJoin.Destroy();
				sPKList.Destroy();

				sprintf_s(sTemp, sizeof(sTemp),
					//Delete the locked replication records.
					"DELETE FROM [%s].[%s].[%s_%s] WHERE SQLExch_Locked = 1",
					gsReplicationDB, sSchema, sDatabase, sTable);
				if(!lpCCI->cCustSQL.Execute(sTemp))
				{
					WriteLog(lpCCI->pClient->PeerID(),
						"Failed to unlock and remove replications.", EVENT_ERROR);
					rsReplicate.Close();
					return CMD_ERROR;
				}

				sprintf_s(sTemp, sizeof(sTemp),
					//Delete the locked replication transactions records.
					"DELETE FROM [%s].[%s].SQLExch_Transactions WHERE [Locked] = 1",
					gsReplicationDB, gsDefaultDBO);
				if(!lpCCI->cCustSQL.Execute(sTemp))
				{
					WriteLog(lpCCI->pClient->PeerID(),
						"Failed to unlock and remove replication transactions.", EVENT_ERROR);
					rsReplicate.Close();
					return CMD_ERROR;
				}
			}
		}
		else{
			WriteLog(lpCCI->pClient->PeerID(),
				"No transactional replications exist, nothing to do.", EVENT_NONE);
		}

		rsReplicate.Close();
	}
	else{
		WriteLog(lpCCI->pClient->PeerID(),
			"Failed to obtain a list of locked replication transactions.", EVENT_ERROR);
		return CMD_ERROR;
	}


	SendCommand(pSock, lpCCI->pClient, "::TransactionReplicationComplete");
	
	return CMD_OK;
}
		
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*
	int ProcessCommand(CSocketServer *pSock, CSocketClient *pClient, LPBASICHUNK lpCmd)

	Processes commands from the peer.

	Possible return Values:
		CMD_OK    The command was a success, all is well.
		CMD_DONE  The command was a success, close the connection.
		CMD_ERROR The command failed due to an error.
*/
int ProcessCommand(CSocketServer *pSock, _ConnectedClientInfo *lpCCI, LPBASICHUNK lpCmd)
{
	//char sText[5120];

	/*-----------------------------------------------------------------------------------------------------------------------
		In response to:
			::AwaitingCommand
		
		The server is sending us its settings, read them and send our own back to the server.

		Notes:
			1) Send any settings to the server here.
	*/
	if(pSock->ParseCommand(lpCmd, "<::Settings>"))
    {
		if(gbDebugMode)
		{
			WriteLog(lpCCI->pClient->PeerID(),
				"Received settings from the server.", EVENT_NONE);
		}

		//Read settings from peer.
		CXMLReader MyRXML((char *)lpCmd->pBuffer, lpCmd->iLength);
		MyRXML.ToString("CompressionMethod", lpCCI->sCompressionMethod, sizeof(lpCCI->sCompressionMethod));
		MyRXML.ToUInteger("CompressionLevel", &lpCCI->dwCompressionLevel);
		MyRXML.Destroy();

		if(gbDebugMode)
		{
			WriteLog(lpCCI->pClient->PeerID(),
				"Sending settings to the server.", EVENT_NONE);
		}

		//Send settings to peer.
		CXMLWriter MyWXML("::Settings");
		MyWXML.Add("AllowOffLineDataProcessing", gbAllowOffLineDataProcessing);
		SendCommand(lpCCI->pClient, &MyWXML);
		MyWXML.Destroy();

		return CMD_OK;
	}
	/*-----------------------------------------------------------------------------------------------------------------------
		In response to:
			<::ClientSideRequest>
		
		The server is letting us know that there is nothing special to do, so start
			the normal transaction replication process.

		Notes:
			1) This is the place to do anything special.
	*/
	else if(pSock->IsCommand(lpCmd, "::ReplicateTransactions"))
    {
		if(gbDebugMode)
		{
			WriteLog(lpCCI->pClient->PeerID(),
				"Server requesting transaction replications.", EVENT_NONE);
		}

		char sTempFile[MAX_PATH];
		GetTempFileName(gsTempFilesPath, "SEC", 0, sTempFile);
		int iReplicationResult = ReplicateTransactions(pSock, lpCCI, lpCmd, sTempFile);
		DeleteFile(sTempFile);

		return iReplicationResult;
	}
	/*-----------------------------------------------------------------------------------------------------------------------
		In response to:
			<::Settings>
		
		The server is asking us if we have anything to request before it starts its normal process.

		Notes:
			1) This is the place to do anything special.
	*/
	else if(pSock->IsCommand(lpCmd, "::ClientSideRequest"))
    {
		if(gbDebugMode)
		{
			WriteLog(lpCCI->pClient->PeerID(),
				"Server is asking for any requests before normal process.", EVENT_NONE);
		}

		//Tell the server what we would "like" to do.
		CXMLWriter MyWXML("::ClientSideRequest");
		if(gbRequestInitialization)
		{
			MyWXML.Add("Request", "Initialization");
		}
		else{
			MyWXML.Add("Request", "None");
		}
		SendCommand(lpCCI->pClient, &MyWXML);
		MyWXML.Destroy();

		return CMD_OK;
	}
	/*-----------------------------------------------------------------------------------------------------------------------
		In response to:
			<::ClientSideRequest>
		
		The server is letting us know that it needs a fresh new script to to create the
			replicating database on its side.

		Notes:
			1) 
	*/
	else if(pSock->IsCommand(lpCmd, "::InitializeStructures"))
    {
		WriteLog(lpCCI->pClient->PeerID(),
			"Server is requesting that the structures be initialized.", EVENT_INFO);

		//if the server is asking for initialization specific data, we nolonger need to
		//	request initialization if we were doing so.
		gbRequestInitialization = false;

		char sTempFile[MAX_PATH];
		GetTempFileName(gsTempFilesPath, "SEC", 0, sTempFile);
		int iResult = InitializeStructures(pSock, lpCCI, sTempFile);
		DeleteFile(sTempFile);
		return iResult;
	}
	/*-----------------------------------------------------------------------------------------------------------------------
		In response to:
			<::Sending::DatabaseCreationScript>
		
		The server is requesting the data from every table in every database that we replicate
			starting at step X.

		Notes:
			1) 
	*/
	else if(pSock->ParseCommand(lpCmd, "<::InitializeData>"))
    {
		WriteLog(lpCCI->pClient->PeerID(),
			"Server is requesting that the data be initialized.", EVENT_INFO);

		//Read init data from peer.
		CXMLReader MyRXML((char *)lpCmd->pBuffer, lpCmd->iLength);
		MyRXML.ToInteger("Step", &lpCCI->iInitStep);
		MyRXML.Destroy();

		if(lpCCI->iInitStep > 0)
		{
			WriteLog(lpCCI->pClient->PeerID(),
				"Resuming a previously broken initialization process.", EVENT_WARN);
		}

		char sTempFile[MAX_PATH];
		GetTempFileName(gsTempFilesPath, "SEC", 0, sTempFile);
		int iResult = InitializeSQLData(pSock, lpCCI, lpCmd, sTempFile);
		DeleteFile(sTempFile);

		return iResult;
	}
	//------------------------------------------------------------------------------------------------
	//This is a general processing message from the server.
	//	All messages are file logged and written to the event log.
	else if(pSock->ParseCommand(lpCmd, "::Msg->"))
    {
		if(gbDebugMode)
		{
			WriteLog(lpCCI->pClient->PeerID(), "Received a message from the server.", EVENT_NONE);
		}

		WriteLog(lpCCI->pClient->PeerID(), (char *)lpCmd->pBuffer, EVENT_INFO);

		return CMD_OK;
	}

	//------------------------------------------------------------------------------------------------
	WriteLog(lpCCI->pClient->PeerID(), "Received unknown command.", EVENT_ERROR);
	//------------------------------------------------------------------------------------------------

	if(lpCmd->iLength > 1024)
	{
		((char *)lpCmd->pBuffer)[1024] = '\0';
	}
	WriteLog(lpCCI->pClient->PeerID(), (char *)lpCmd->pBuffer, EVENT_ERROR);

	return CMD_ERROR;	
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//If this routine returns ((bool)TRUE) then we need to connect.
bool CheckForPendingTransactions(CSQL *lpSQL)
{
	bool bRetVal = true;

	if(!gbReportNoTransactions)
	{
		char sSQL[1024];
		CBoundRecordSet rsTemp;

		sprintf_s(sSQL, sizeof(sSQL),
			"SELECT TOP 1 1 AS [Records] FROM [%s].[%s].SQLExch_Transactions",
			gsReplicationDB, gsDefaultDBO);

		if(!lpSQL->Execute(sSQL, &rsTemp))
		{
			WriteSysLogEx("Failed to get the pending transaction count.", EVENT_ERROR);
			bRetVal = false;
		}
		else{
			if(rsTemp.RowCount <= 0)
			{
				bRetVal = false;
			}
			else{
				if(rsTemp.Fetch())
				{
					if(rsTemp.Values("Records")->ToIntegerS() <= 0)
					{
						bRetVal = false;
					}
				}
				else{
					bRetVal = false;
				}
			}
		}
		rsTemp.Close();
	}

	return bRetVal;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//Return ((bool)TRUE) if this function succeeded. Only return ((bool)FALSE) is an **ERROR** occurs.
bool ConnectToServer(bool bForcedConnect)
{
	memset(&CCI, 0, sizeof(_ConnectedClientInfo));

	if(OnlyGoodForOneYear())
	{
		WriteSysLogEx("This version of the SQL-Exchange has expired. Please obtain a newer version.", EVENT_ERROR);
		return false;
	}

	if(!IsClientConfigured(false))
	{
		WriteSysLogEx("Cannot connect to the SQL-Exchange server: Not configured.", EVENT_WARN);
		return false;
	}

	if(gServer.Client(0)->IsConnected())
	{
		WriteSysLogEx("Failed to connect to the SQL-Exchange server: Already connected.", EVENT_WARN);
		return false;
	}

	if(ConnectToSQLServer(&CCI.cCustSQL, gsSQLServer, gsSQLUserID, gsSQLPassword, "master",
		gbUseTCPIPConnection, gbUseTrustedConnection, gdwSQLServerPort))
	{
		if(gbDebugMode)
		{
			WriteSysLogEx("Successfully connected to the SQL master database.", EVENT_NONE);
		}
		CCI.bCustDBConnected = true;
	}
	else{
		WriteSysLogEx("Failed to connected to the SQL master database.", EVENT_ERROR);
		return false;						
	}

	if(!bForcedConnect)
	{
		if(!CheckForPendingTransactions(&CCI.cCustSQL))
		{
			WriteSysLogEx("No new transactions are available, no need to connect.", EVENT_NONE);
			gServer.Client(0)->Disconnect();
			return true;
		}
	}

	if(gServer.ConnectAndCreateThread(gsServerAddress, gdwServerPort, &CCI.pClient))
	{
		if(gbDebugMode)
		{
			WriteSysLogEx("Successfully Connected to the SQL-Exchange server.", EVENT_NONE);
		}
	}
	else{
		WriteSysLogEx("Failed to connect to the SQL-Exchange server.", EVENT_WARN);

		if(gServer.Client(0)->IsConnected())
		{
			WriteSysLogEx("Dropping a possible locked connection.", EVENT_WARN);
			gServer.Client(0)->Disconnect();
		}

		CCI.cCustSQL.Disconnect();
		CCI.bCustDBConnected = false;
		return false;
	}

	return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#endif
