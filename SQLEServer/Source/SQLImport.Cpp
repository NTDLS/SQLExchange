///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//  Copyright © NetworkDLS 2002, All rights reserved
//
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF 
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO 
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A 
// PARTICULAR PURPOSE.
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef _SQLIMPORT_CPP
#define _SQLIMPORT_CPP
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#define SQLERROR_INVALID_COLUMN_NAME		207
#define SQLERROR_STRING_WOULD_BE_TRUNCATED	8152
#define SQLERROR_INVALID_OBJECT_NAME		208


#include <Windows.H>
#include <Stdio.H>
#include <SQL.H>
#include <SQLExt.H>

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "../../../@Libraries/CStringBuilder/CStringBuilder.H"
#include "../../SharedClasses/CGetPKs/CGetPKs.H"
#include "../../../@Libraries/CSocketServer/CSocketServer.H"

#include "../../SharedSource/Common.H"

#include "Entry.H"
#include "Console.H"
#include "SQLImport.H"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

using namespace NSWFL::Conversion;
using namespace NSWFL::String;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool GetImportType(CSocketClient *pClient, const int iDataType, int *piSQLType, bool bIsPK)
{
	if(iDataType == TSQL_NTEXT)
	{
		if(bIsPK)
		{
			return false;
		}
		*piSQLType = SQL_UNICODE_LONGVARCHAR;
		return true;
	}
	else if(iDataType == TSQL_NCHAR
		|| iDataType == TSQL_NVARCHAR
		|| iDataType == TSQL_SQL_VARIANT)
	{
		*piSQLType = SQL_UNICODE_VARCHAR;
		return true;
	}
	else if(iDataType == TSQL_TEXT)
	{
		if(bIsPK)
		{
			return false;
		}
		*piSQLType = SQL_LONGVARCHAR;
		return true;
	}
	else if(iDataType == TSQL_CHAR
		|| iDataType == TSQL_VARCHAR)
	{
		*piSQLType = SQL_CHAR;
		return true;
	}
	else if(iDataType == TSQL_BINARY
		|| iDataType == TSQL_VARBINARY)
	{
		if(bIsPK)
		{
			*piSQLType = SQL_VARBINARY;
		}
		else{
			*piSQLType = SQL_LONGVARBINARY;
		}
		return true;
	}
	else if(iDataType == TSQL_IMAGE)
	{
		if(bIsPK)
		{
			*piSQLType = SQL_VARBINARY;
		}
		else{
			*piSQLType = SQL_LONGVARBINARY;
		}
		return true;
	}
	else{
		char sText[1024];
		sprintf_s(sText, sizeof(sText), "Unsupported data type %d.", iDataType);
		WriteLog(pClient->PeerID(), sText, EVENT_WARN);
		return false;
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void FreeColumnRowData(LPSQLIMPORTCOLUMNS lpColumns)
{
    Assert(!lpColumns, "Proc: FreeSingleRowDataColumns");
	
	for(int iCol = 0; iCol < lpColumns->iColumnCount; iCol++)
    {
		if(!lpColumns->Columns[iCol].bComplexType && lpColumns->Columns[iCol].iDataSz > 0)
		{
			pMem->Free(lpColumns->Columns[iCol].Data);
		}
        lpColumns->Columns[iCol].Data = NULL;
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool AllocColumns(CSocketServer *pSock, _ConnectedClientInfo *lpCCI,
				  LPSQLIMPORTCOLUMNS lpColumns, char *sDB, char *sDBO, char *sTable)
{
	char sStatusText[1024];

	//Get a list of primary keys for the import table.
	if(lpColumns->PKs.Get(&lpCCI->cCustSQL, sDB, sDBO, sTable))
	{
		lpColumns->bIsPKsAlloc = true;
	}
	else{
		sprintf_s(sStatusText, sizeof(sStatusText),
			"Error in AllocColumns:: Failed to retrieve PK for [%s].[%s].[%s].",
			sDB, sDBO, sTable);
		WriteLog(lpCCI->pClient->PeerID(), sStatusText, EVENT_ERROR);
		return false;
	}

	if(lpColumns->PKs.Columns.Count <= 0)
	{
		sprintf_s(sStatusText, sizeof(sStatusText),
			"Error in AllocColumns:: [%s].[%s].[%s] does not have a primary key.",
			sDB, sDBO, sTable);
		WriteLog(lpCCI->pClient->PeerID(), sStatusText, EVENT_ERROR);
		return false;
	}

	lpColumns->iPKColPos = (int *) pMem->Allocate(sizeof(int), lpColumns->PKs.Columns.Count);
	Assert(!lpColumns->iPKColPos, "Memory Allocation Error.");

	lpColumns->Columns = (SQLIMPORTCOLUMN *)
		pMem->Allocate(sizeof(SQLIMPORTCOLUMN), lpColumns->iColumnCount);
	Assert(!lpColumns->Columns, "Memory Allocation Error.");

	lpColumns->iAllocationLevel = COL_ALLOC_LEVEL_1;

	return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool DestroyColumns(LPSQLIMPORTCOLUMNS lpColumns)
{
	if(lpColumns->iAllocationLevel == COL_ALLOC_LEVEL_3)
	{
		FreeColumnRowData(lpColumns);
		lpColumns->iAllocationLevel--;
	}

	if(lpColumns->iAllocationLevel == COL_ALLOC_LEVEL_2)
	{
		for(int iCol = 0; iCol < lpColumns->iColumnCount; iCol++)
		{
			pMem->Free(lpColumns->Columns[iCol].Name);
			lpColumns->Columns[iCol].Name = NULL;
		}
		lpColumns->iAllocationLevel--;
	}

	if(lpColumns->iAllocationLevel == COL_ALLOC_LEVEL_1)
	{
		pMem->Free(lpColumns->iPKColPos);
		pMem->Free(lpColumns->Columns);

		lpColumns->iColumnCount = 0;
		lpColumns->iAllocationLevel--;
	}

	if(lpColumns->bIsPKsAlloc)
	{
		lpColumns->PKs.Free();
	}

	return true;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int ImportSQLResults(CSocketServer *pSock, CSocketClient *pClient, _ConnectedClientInfo *lpCCI, char *InFileName)
{
	int iReplicationID = 0;
	char sStatusText[1024];

	SQLIMPORTCOLUMNS Columns;
	memset(&Columns, 0, sizeof(SQLIMPORTCOLUMNS));

	Columns.iAllocationLevel = COL_ALLOC_LEVEL_0;

    FILE *hSource = NULL;

	int iSQLBufSz = (32 * 1024);
	char *sSQLBuf = (char *)pMem->Allocate(sizeof(char), iSQLBufSz + 1);
	if(!sSQLBuf)
	{
		Assert(!sSQLBuf, "Memory Allocation Error.");
        return IMPORT_RES_ERROR;
	}

    if(fopen_s(&hSource, InFileName, "rb") != 0)
    {
		sprintf_s(sStatusText, sizeof(sStatusText),
			"Error in ImportSQLResults:: TargetFile open error. Name: \"%s\"", InFileName);
		WriteLog(lpCCI->pClient->PeerID(), sStatusText, EVENT_ERROR);
		pMem->Free(sSQLBuf);
        return IMPORT_RES_ERROR;
    }
	
	if(lpCCI->bAllowOffLineDataProcessing)
	{
		pClient->Reserved(true);
	}

	CStringBuilder sSrvSQL;
	CStringBuilder sWhereSQL;

	int iResult = ImportSQLResultsEx(pSock,	pClient, lpCCI, &Columns, sSQLBuf, iSQLBufSz,
		&sSrvSQL, &sWhereSQL, hSource, &iReplicationID, InFileName);

	while(lpCCI->cCustSQL.TransactionDepth() > 0)
	{
		lpCCI->cCustSQL.CommitTransaction();
	}

	pMem->Free(sSQLBuf);

	sSrvSQL.Destroy();
	sWhereSQL.Destroy();
	fclose(hSource);

	DestroyColumns(&Columns);

	if(iResult == IMPORT_RES_OK)
	{
		//If were initializing, keep track of the last completed step for resume purposes.
		if(lpCCI->bRequestInit)
		{
			char sSQL[1024];

			sprintf_s(sSQL, sizeof(sSQL), 
				"EXEC [%s].[%s].SetCompanyInitStepByName"
				" @CompanyName = '%s',"
				" @InitializeStep = %d",
				gsSQLIndexDatabase, gsDefaultDBO, lpCCI->sCompanyName, iReplicationID);
			lpCCI->cIndexSQL.Execute(sSQL);
		}
	}

	if(lpCCI->bAllowOffLineDataProcessing)
	{
		if(iResult == IMPORT_RES_OK && pClient->IsDisconnectSet())
		{
			iResult = IMPORT_RES_OFFLINEOK;
		}
		pClient->Reserved(false);
	}

	return iResult;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*
	Operations:
		INSERT : The data contains straight inserts (no updates).
		UPDATE : The data contains updates AND inserts.
		DELETE : The data contains the PK columns for deletion purposes.
*/
int ImportSQLResultsEx(CSocketServer *pSock, CSocketClient *pClient,
	_ConnectedClientInfo *lpCCI, LPSQLIMPORTCOLUMNS lpColumns,
	char *sSQLBuf, int iSQLBufSz, CStringBuilder *lpSrvSQL,
	CStringBuilder *lpWhereSQL, FILE *hSource, int *iOutID,
	char *InFileName)
{
	Assert(pSock == NULL, "Proc: ImportSQLResults");

	char sImportDB[1024];                 //The name of the database that were going to be importing into.
	char sImportSchema[1024];             //The name of the schema that were going to be importing into.
	char sImportTable[1024];              //The name of the table that were going to be importing into.
    char sStatusText[MAX_STATUS_TEXT];    //Just some RAM for status text and error messages.
    char sAltTemp[MAX_STATUS_TEXT];       //Just some RAM for status text and error messages.

	char sFormat1[32];
	char sFormat2[32];

    int iUniuqeKeysFound = 0;             // The number of uniuqe keys we have found.
    int iTemp = 0;
    int iRowCount = 0;
    int iColNameLen = 0;
	int iID = 0;
	int iOperation = 0;                   //OPERATION_INSERT, OPERATION_UPDATE or OPERATION_DELETE.
	int iComplexTypeCount = 0;            //The count of columns whose buffering will be deffered.
	int iComplexTypePKCount = 0;		  //The count of PK columns whose buffering will be deffered.

	CBoundRecordSet rsTemp;

    fread(&iOperation, sizeof(iOperation), 1, hSource); // Read the operation flag.

	fread(&iTemp, sizeof(iTemp), 1, hSource); // Read import database name len.
    fread(sAltTemp, sizeof(char), iTemp, hSource); // Read database name.
    sAltTemp[iTemp] = '\0';

	strcpy_s(sImportDB, sizeof(sImportDB), "SQLExch_");
	strcat_s(sImportDB, sizeof(sImportDB), lpCCI->sCompanyDBPrefix);
	strcat_s(sImportDB, sizeof(sImportDB), "_");
	strcat_s(sImportDB, sizeof(sImportDB), sAltTemp);

	if(!lpCCI->cCustSQL.Focus(sImportDB))
	{
		WriteLog(lpCCI->pClient->PeerID(),
			"Failed to change client database context. Performance will be affected.", EVENT_WARN);
	}

	fread(&iTemp, sizeof(iTemp), 1, hSource); // Read import schema name len.
    fread(sImportSchema, sizeof(char), iTemp, hSource); // Read import schema name.
    sImportSchema[iTemp] = '\0';

	fread(&iTemp, sizeof(iTemp), 1, hSource); // Read import table name len.
    fread(sImportTable, sizeof(char), iTemp, hSource); // Read import table name.
    sImportTable[iTemp] = '\0';

	fread(&lpColumns->iColumnCount, sizeof(lpColumns->iColumnCount), 1, hSource); // Read number of Columns.
    fread(&iRowCount, sizeof(iRowCount), 1, hSource);       // Read number of rows.
    fread(&iID, sizeof(iID), 1, hSource);                   // Read the row ID of the associated Statments row.

	*iOutID = iID;

	FormatInteger(sFormat1, sizeof(sFormat1), iRowCount);
	FormatInteger(sFormat2, sizeof(sFormat2), lpColumns->iColumnCount);

	sprintf_s(sStatusText, sizeof(sStatusText),
		"Processing transaction data. [%s.%s.%s - %s Rows, %s Columns]", 
		sImportDB, sImportSchema, sImportTable, sFormat1, sFormat2);
	WriteLog(lpCCI->pClient->PeerID(), sStatusText, EVENT_NONE);

	char sRight[100];
	Right(sRight, sizeof(sRight), sImportTable, iTemp, 15);

	if(!AllocColumns(pSock, CCI, lpColumns, sImportDB, sImportSchema, sImportTable))
	{
		//No primary keys, cannot import this data.
		WriteLog(lpCCI->pClient->PeerID(),
			"Error in ImportSQLResults:: Failed to allocate column structure.", EVENT_ERROR);
        return IMPORT_RES_ERROR;
	}

	if(lpColumns->PKs.Columns.Count == 0)
	{
		//No primary keys, cannot import this data.
		WriteLog(lpCCI->pClient->PeerID(),
			"Error in ImportSQLResults:: Could not obtain a list of primary keys.", EVENT_ERROR);
        return IMPORT_RES_SYNCTABLE;
	}

	//Loop through, reading all of the column names from the file.
    for(int iColIndex = 0; iColIndex < lpColumns->iColumnCount; iColIndex++)
    {
        fread(&lpColumns->Columns[iColIndex].bComplexType, sizeof(bool), 1, hSource);
		if(lpColumns->Columns[iColIndex].bComplexType)
		{
			iComplexTypeCount++;
		}

        fread(&lpColumns->Columns[iColIndex].iDataType, sizeof(int), 1, hSource);
        fread(&lpColumns->Columns[iColIndex].iDataLength, sizeof(SQLULEN), 1, hSource);

		fread(&iColNameLen, sizeof(iColNameLen), 1, hSource);
        lpColumns->Columns[iColIndex].Name = (char *) pMem->Allocate(sizeof(char), iColNameLen + 1);
		Assert(!lpColumns->Columns[iColIndex].Name, "Memory Allocation Error.");

		fread(lpColumns->Columns[iColIndex].Name, sizeof(char), iColNameLen, hSource);
        lpColumns->Columns[iColIndex].Name[iColNameLen] = '\0';

		lpColumns->Columns[iColIndex].IsPK = false;

        //If this column name matches one of our Primary Keys then record its position in lpColumns->iPKColPos[].
        if(iUniuqeKeysFound < lpColumns->PKs.Columns.Count)
        {
            for(int iPK = 0; iPK < lpColumns->PKs.Columns.Count; iPK++)
            {
				if(_strcmpi(lpColumns->PKs.Columns.Array[iPK].Name, lpColumns->Columns[iColIndex].Name) == 0)
                {
                    lpColumns->iPKColPos[iUniuqeKeysFound] = iColIndex;
					lpColumns->Columns[iColIndex].IsPK = true;
					if(lpColumns->Columns[iColIndex].bComplexType)
					{
						iComplexTypePKCount++;
					}

					iUniuqeKeysFound++;
                }
            }
        }
    }

	lpColumns->iAllocationLevel = COL_ALLOC_LEVEL_2;

    if(iUniuqeKeysFound != lpColumns->PKs.Columns.Count)
    {
        sprintf_s(sStatusText, sizeof(sStatusText),
			"Error in ImportSQLResults:: Required uniuqe keys were not found for: '%s'.", sImportTable);
		WriteLog(lpCCI->pClient->PeerID(), sStatusText, EVENT_ERROR);
        return IMPORT_RES_ERROR;
    }

    if(!pClient->IsConnected())
    {
		if(iOperation == OPERATION_DELETE)
		{
			WriteLog(lpCCI->pClient->PeerID(), "Disconnected before data deletion.", EVENT_WARN);
		}
		else{
			WriteLog(lpCCI->pClient->PeerID(), "Disconnected before data import.", EVENT_WARN);
		}

        return IMPORT_RES_ERROR;
    }

	//If we are initializing, clear the target table.
	if(lpCCI->bRequestInit)
	{
		lpSrvSQL->Clear();
		lpSrvSQL->Append("DELETE FROM [");
		lpSrvSQL->Append(sImportDB);
		lpSrvSQL->Append("].[");
		lpSrvSQL->Append(sImportSchema);
		lpSrvSQL->Append("].[");
		lpSrvSQL->Append(sImportTable);
		lpSrvSQL->Append("]");
		lpSrvSQL->Terminate();

		if(!lpCCI->cCustSQL.Execute(lpSrvSQL->Buffer))
		{
			sprintf_s(sStatusText, sizeof(sStatusText),
				"Failed to clear table [%s] for initialization.", sImportTable);
			WriteLog(lpCCI->pClient->PeerID(), sStatusText, EVENT_ERROR);
			return IMPORT_RES_ERROR;
		}
	}

	DWORD dwRowsInTransation = 0;

	for(int iRowIndex = 0; iRowIndex < iRowCount; iRowIndex++)
    {
		if(gdwMaxRowsPerTransaction > 0)
		{
			if(dwRowsInTransation == gdwMaxRowsPerTransaction)
			{
				lpCCI->cCustSQL.CommitTransaction();
				dwRowsInTransation = 0;
			}

			if(dwRowsInTransation == 0)
			{
				lpCCI->cCustSQL.BeginTransaction();
			}

			dwRowsInTransation++;
		}
		
		// Read row data from the file.
	    for(int iColIndex = 0; iColIndex < lpColumns->iColumnCount; iColIndex++)
        {
			if(lpColumns->Columns[iColIndex].bComplexType)
			{
				//Read the data size.
				fread(&lpColumns->Columns[iColIndex].iDataSz,
					sizeof(lpColumns->Columns[iColIndex].iDataSz), 1, hSource);

				//Save the file position of the data, so we can seek back later and read the data in.
				lpColumns->Columns[iColIndex].iFileDataPos = ftell(hSource);

				//If the data is non-NULL, then we need to skip it.
				if(lpColumns->Columns[iColIndex].iDataSz > 0)
				{
					fseek(hSource, (long)lpColumns->Columns[iColIndex].iDataSz, SEEK_CUR); //Skip the column data.
				}
			}
			else{
				//Read the data size.
				fread(&lpColumns->Columns[iColIndex].iDataSz,
					sizeof(lpColumns->Columns[iColIndex].iDataSz), 1, hSource);

				if(lpColumns->Columns[iColIndex].iDataSz > 0)
				{
					if((lpColumns->Columns[iColIndex].Data = (char *) 
						pMem->Allocate(sizeof(char), (unsigned int)lpColumns->Columns[iColIndex].iDataSz + 1)) == NULL)
					{
						WriteLog(lpCCI->pClient->PeerID(), "Failed to allocate memory for Data[].", EVENT_ERROR);
						Assert(!lpColumns->Columns[iColIndex].Data, "Memory Allocation Error.");
						return IMPORT_RES_ERROR;
					}

					fread(lpColumns->Columns[iColIndex].Data, sizeof(char),
						lpColumns->Columns[iColIndex].iDataSz, hSource);

					lpColumns->Columns[iColIndex].Data[lpColumns->Columns[iColIndex].iDataSz] = '\0';
				}
				else{
					//Value is blank or NULL.
				}
			}
        }

		lpColumns->iAllocationLevel = COL_ALLOC_LEVEL_3;

        // Assemble the where statement from the row data, the unique key names and positions.
        if(iOperation != OPERATION_INSERT)
        {
			lpWhereSQL->Clear();

			// Assemble the where statement
			lpWhereSQL->Append("WHERE [");
			lpWhereSQL->Append(lpColumns->PKs.Columns.Array[0].Name);
			lpWhereSQL->Append("]=");
			if(lpColumns->Columns[lpColumns->iPKColPos[0]].iDataSz < 0) //The column is NULL.
			{
				lpWhereSQL->Append("NULL"); //This is actually invalid, PK members CANNOT be NULL.
			}
			else if(lpColumns->Columns[lpColumns->iPKColPos[0]].iDataSz == 0) //The column is blank.
			{
				lpWhereSQL->Append("''");
			}
			else{
				if(lpColumns->Columns[lpColumns->iPKColPos[0]].bComplexType)  //The column needs to be buffered.
				{
					lpWhereSQL->Append("?");
				}
				else{
					lpWhereSQL->Append(lpColumns->Columns[lpColumns->iPKColPos[0]].Data);
				}
			}

            for(int iPK = 1; iPK < iUniuqeKeysFound; iPK++)
            {
				lpWhereSQL->Append(" AND [");
				lpWhereSQL->Append(lpColumns->PKs.Columns.Array[iPK].Name);
				lpWhereSQL->Append("]=");

				if(lpColumns->Columns[lpColumns->iPKColPos[iPK]].iDataSz < 0) //The column is NULL.
				{
					lpWhereSQL->Append("NULL"); //This is actually invalid, PK members CANNOT be NULL.
				}
				else if(lpColumns->Columns[lpColumns->iPKColPos[iPK]].iDataSz == 0) //The column is blank.
				{
					lpWhereSQL->Append("''");
				}
				else{
					if(lpColumns->Columns[lpColumns->iPKColPos[iPK]].bComplexType)
					{
						lpWhereSQL->Append("?"); //The column needs to be buffered.
					}
					else{
						lpWhereSQL->Append(lpColumns->Columns[lpColumns->iPKColPos[iPK]].Data);
					}
				}
            }
			lpWhereSQL->Terminate();
        }

        bool bRowAlreadyExists = false;

		if(iOperation == OPERATION_UPDATE)
		{
			if(iUniuqeKeysFound > 0)
			{
				lpSrvSQL->Clear();

				// Assemble the check existence statement
				lpSrvSQL->Append("SELECT TOP 1 1 FROM [");
				lpSrvSQL->Append(sImportDB);
				lpSrvSQL->Append("].[");
				lpSrvSQL->Append(sImportSchema);
				lpSrvSQL->Append("].[");
				lpSrvSQL->Append(sImportTable);
				lpSrvSQL->Append("] ");
				lpSrvSQL->Append(lpWhereSQL->Buffer);
				lpSrvSQL->Terminate();

				if(iComplexTypePKCount == 0)
				{
					if(!lpCCI->cCustSQL.Execute(lpSrvSQL->Buffer, &rsTemp))
					{
						WriteLog(lpCCI->pClient->PeerID(),
							"Error in ImportSQLResults:: Execute failed on CheckExistanceStatement. (See log file for more information)",
							EVENT_ERROR);

						sprintf_s(sStatusText, sizeof(sStatusText), "Statement: %s", lpSrvSQL->Buffer);
						WriteLog(lpCCI->pClient->PeerID(), sStatusText, EVENT_NONE);

						//Need to do some advanced error reporting.
						int iNativeErr = 0;
						if(lpCCI->cCustSQL.GetErrorMessage(&iNativeErr, sAltTemp, sizeof(sAltTemp), rsTemp.hSTMT))
						{
							sprintf_s(sStatusText, sizeof(sStatusText), "Error Msg: %s", sAltTemp);
							WriteLog(lpCCI->pClient->PeerID(), sStatusText, EVENT_NONE);
						}

						return IMPORT_RES_ERROR;
					}

					bRowAlreadyExists = (rsTemp.RowCount > 0);

					rsTemp.Close();
				}
				else{
					int iBindIndex = 1;
					bool bExecutionResult = true;

					//Deferred data needs to be buffered.
					HSTMT hSTMT = lpCCI->cCustSQL.Prepare(lpSrvSQL->Buffer);
					if(hSTMT)
					{
						//Bind the appropriate columns (where Statement).
						for(int iColIndex = 0; iColIndex < lpColumns->iColumnCount; iColIndex++)
						{
							int iSQLType = 0;

							if(lpColumns->Columns[iColIndex].bComplexType
								&& lpColumns->Columns[iColIndex].IsPK
								&& lpColumns->Columns[iColIndex].iDataSz > 0)
							{
								if(GetImportType(pClient, lpColumns->Columns[iColIndex].iDataType, &iSQLType, true))
								{
									lpColumns->Columns[iColIndex].iDefferBufSize = lpColumns->Columns[iColIndex].iDataSz;

									lpCCI->cCustSQL.BindParameter(hSTMT,
										iBindIndex++, (void *)iColIndex, iSQLType, &lpColumns->Columns[iColIndex].iDefferBufSize);
								}
								else{
									bExecutionResult = false;
									break;
								}
							}
						}

						short iExecStat = 0;
						if((iExecStat = lpCCI->cCustSQL.Execute(hSTMT)) == SQL_NEED_DATA)
						{
							int iOriginalFilePosition = ftell(hSource);

							void *piColumnIndex;
							while(iExecStat == SQL_NEED_DATA)
							{
								if((iExecStat = lpCCI->cCustSQL.ParamData(hSTMT, &piColumnIndex)) == SQL_NEED_DATA)
								{
									int iBoundColIndex = (int)piColumnIndex;

									fseek(hSource, lpColumns->Columns[iBoundColIndex].iFileDataPos, SEEK_SET);

									SQLLEN iRemainBytes = lpColumns->Columns[iBoundColIndex].iDataSz;
									SQLLEN iReadBytes = iSQLBufSz;

									for(int iByte = 0; iRemainBytes > 0;)
									{
										if(iRemainBytes < iReadBytes)
										{
											iReadBytes = iRemainBytes;
										}
										fread(sSQLBuf, sizeof(byte), iReadBytes, hSource);
										if(!SQL_SUCCEEDED(lpCCI->cCustSQL.PutData(hSTMT, sSQLBuf, iReadBytes)))
										{
											bExecutionResult = false;
											break;
										}
										iRemainBytes -= iReadBytes;
									}
								}
								else if(!SQL_SUCCEEDED(iExecStat)) {
									bExecutionResult = false;
									break;
								}
							}
							fseek(hSource, iOriginalFilePosition, SEEK_SET);

							bRowAlreadyExists = (lpCCI->cCustSQL.RowCount(hSTMT) > 0);
						}
						else if(!SQL_SUCCEEDED(iExecStat)) {
							bExecutionResult = false;
						}

						lpCCI->cCustSQL.FreeHandle(hSTMT);
					}
					else{
						bExecutionResult = false;
					}

					if(!bExecutionResult)
					{
						WriteLog(lpCCI->pClient->PeerID(),
							"Error in ImportSQLResults:: Execute failed on CheckExistanceStatement. (See log file for more information)",
							EVENT_ERROR);

						sprintf_s(sStatusText, sizeof(sStatusText), "Statement: %s", lpSrvSQL->Buffer);
						WriteLog(lpCCI->pClient->PeerID(), sStatusText, EVENT_NONE);

						//Need to do some advanced error reporting.
						int iNativeErr = 0;
						if(lpCCI->cCustSQL.GetErrorMessage(&iNativeErr, sAltTemp, sizeof(sAltTemp), rsTemp.hSTMT))
						{
							sprintf_s(sStatusText, sizeof(sStatusText), "Error Msg: %s", sAltTemp);
							WriteLog(lpCCI->pClient->PeerID(), sStatusText, EVENT_NONE);
						}

						return IMPORT_RES_ERROR;
					}
				}
			}
		}

        //----------------------------------------------------------------------------------------------

		lpSrvSQL->Clear();

        //------------------------------------------
		if(iOperation == OPERATION_DELETE)
		{
			lpSrvSQL->Append("DELETE FROM [");
			lpSrvSQL->Append(sImportDB);
			lpSrvSQL->Append("].[");
			lpSrvSQL->Append(sImportSchema);
			lpSrvSQL->Append("].[");
			lpSrvSQL->Append(sImportTable);
			lpSrvSQL->Append("] ");
			lpSrvSQL->Append(lpWhereSQL->Buffer);
		}
        //------------------------------------------
        else if(bRowAlreadyExists) // Do update
		{
			lpSrvSQL->Append("UPDATE [");
			lpSrvSQL->Append(sImportDB);
			lpSrvSQL->Append("].[");
			lpSrvSQL->Append(sImportSchema);
			lpSrvSQL->Append("].[");
			lpSrvSQL->Append(sImportTable);
			lpSrvSQL->Append("] SET");

            for(int iColIndex = 0; iColIndex < lpColumns->iColumnCount; iColIndex++)
            {
				//Do not update PK columns, they do not change.
				if(!lpColumns->Columns[iColIndex].IsPK)
				{
					lpSrvSQL->Append("[");
					lpSrvSQL->Append(lpColumns->Columns[iColIndex].Name);
					lpSrvSQL->Append("]=");

					if(lpColumns->Columns[iColIndex].iDataSz < 0) //The column is NULL.
					{
						lpSrvSQL->Append("NULL");
					}
					else if(lpColumns->Columns[iColIndex].iDataSz == 0) //The column is blank.
					{
						lpSrvSQL->Append("''");
					}
					else{
						if(lpColumns->Columns[iColIndex].bComplexType)
						{
							lpSrvSQL->Append("?"); //The column needs to be buffered.
						}
						else{
							lpSrvSQL->Append(lpColumns->Columns[iColIndex].Data);
						}
					}

					lpSrvSQL->Append(",");
				}
            }

			lpSrvSQL->Length--; //overwrite the last comma with a space.
			lpSrvSQL->Append(" ");

            lpSrvSQL->Append(lpWhereSQL->Buffer);
        }
        //------------------------------------------
		else // Do insert.
        {
			lpSrvSQL->Append("INSERT INTO [");
			lpSrvSQL->Append(sImportDB);
			lpSrvSQL->Append("].[");
			lpSrvSQL->Append(sImportSchema);
			lpSrvSQL->Append("].[");
			lpSrvSQL->Append(sImportTable);
			lpSrvSQL->Append("] (");

		    for(int iColIndex = 0; iColIndex < lpColumns->iColumnCount; iColIndex++)
            {
                lpSrvSQL->Append("[");
                lpSrvSQL->Append(lpColumns->Columns[iColIndex].Name);
                lpSrvSQL->Append("]");
                if(iColIndex < lpColumns->iColumnCount - 1)
                {
                    lpSrvSQL->Append(",");
                }
            }
            lpSrvSQL->Append(") VALUES(");

            for(int iColIndex = 0; iColIndex < lpColumns->iColumnCount; iColIndex++)
            {
				if(lpColumns->Columns[iColIndex].iDataSz < 0) //The column is NULL.
				{
					lpSrvSQL->Append("NULL");
				}
				else if(lpColumns->Columns[iColIndex].iDataSz == 0) //The column is blank.
				{
					lpSrvSQL->Append("''");
				}
				else{ //The column needs to be buffered.
					if(lpColumns->Columns[iColIndex].bComplexType)
					{
						lpSrvSQL->Append("?");
					}
					else{
						lpSrvSQL->Append(lpColumns->Columns[iColIndex].Data);
					}
				}

                if(iColIndex < lpColumns->iColumnCount - 1)
                {
                    lpSrvSQL->Append(",");
                }
            }
            lpSrvSQL->Append(") ");
        }
        //------------------------------------------

        //----------------------------------------------------------------------------------------------

        if(lpSrvSQL->Length > 0)
        {
			HSTMT hSTMT = NULL;

			lpSrvSQL->Terminate();

			bool bExecutionResult = true;
			int iBindIndex = 1;

			if(iComplexTypeCount == 0)
			{
				//Direct execute, nice and fast.
				bExecutionResult = lpCCI->cCustSQL.Execute(lpSrvSQL->Buffer);
			}
			else{
				//Deferred data needs to be buffered.
				hSTMT = lpCCI->cCustSQL.Prepare(lpSrvSQL->Buffer);
				if(hSTMT)
				{
					//Bind the appropriate columns (column values).
					if(iOperation != OPERATION_DELETE) //Where statement only, no need to bind data columns.
					{
						//Bind the appropriate columns.
						for(int iColIndex = 0; iColIndex < lpColumns->iColumnCount; iColIndex++)
						{
							int iSQLType = 0;

							//Again, we do not update PK columns. So only bind the PK column if we are inserting a row.
							if(!(iOperation == OPERATION_UPDATE && lpColumns->Columns[iColIndex].IsPK) || !bRowAlreadyExists)
							{
								if(lpColumns->Columns[iColIndex].bComplexType && lpColumns->Columns[iColIndex].iDataSz > 0)
								{
									if(GetImportType(pClient, lpColumns->Columns[iColIndex].iDataType, &iSQLType, false))
									{
										lpColumns->Columns[iColIndex].iDefferBufSize = lpColumns->Columns[iColIndex].iDataSz;

										lpCCI->cCustSQL.BindParameter(hSTMT,
											iBindIndex++, (void *)iColIndex, iSQLType, &lpColumns->Columns[iColIndex].iDefferBufSize);
									}
									else{
										bExecutionResult = false;
										break;
									}
								}
							}
						}
					}

					//Bind the appropriate columns (where clause).
					if(iComplexTypePKCount > 0 && iOperation != OPERATION_INSERT)
					{
						for(int iColIndex = 0; iColIndex < lpColumns->iColumnCount; iColIndex++)
						{
							int iSQLType = 0;

							if(lpColumns->Columns[iColIndex].bComplexType
								&& lpColumns->Columns[iColIndex].IsPK
								&& lpColumns->Columns[iColIndex].iDataSz > 0)
							{
								if(GetImportType(pClient, lpColumns->Columns[iColIndex].iDataType, &iSQLType, true))
								{
									lpColumns->Columns[iColIndex].iDefferBufSize = lpColumns->Columns[iColIndex].iDataSz;

									lpCCI->cCustSQL.BindParameter(hSTMT,
										iBindIndex++, (void *)iColIndex, iSQLType, &lpColumns->Columns[iColIndex].iDefferBufSize);
								}
								else{
									bExecutionResult = false;
									break;
								}
							}
						}
					}

					if(bExecutionResult)
					{
						short iExecStat = 0;
						//Execute the SQL statement.
						if((iExecStat = lpCCI->cCustSQL.Execute(hSTMT)) == SQL_NEED_DATA)
						{
							int iOriginalFilePosition = ftell(hSource);

							void *piColumnIndex;
							while(iExecStat == SQL_NEED_DATA)
							{
								if((iExecStat = lpCCI->cCustSQL.ParamData(hSTMT, &piColumnIndex)) == SQL_NEED_DATA)
								{
									int iBoundColIndex = (int)piColumnIndex;

									fseek(hSource, lpColumns->Columns[iBoundColIndex].iFileDataPos, SEEK_SET);

									SQLLEN iRemainBytes = lpColumns->Columns[iBoundColIndex].iDataSz;
									SQLLEN iReadBytes = iSQLBufSz;

									for(int iByte = 0; iRemainBytes > 0;)
									{
										if(iRemainBytes < iReadBytes)
										{
											iReadBytes = iRemainBytes;
										}
										fread(sSQLBuf, sizeof(byte), iReadBytes, hSource);

										if(!SQL_SUCCEEDED(lpCCI->cCustSQL.PutData(hSTMT, sSQLBuf, iReadBytes)))
										{
											bExecutionResult = false;
											break;
										}
										iRemainBytes -= iReadBytes;
									}
								}
								else if(!SQL_SUCCEEDED(iExecStat) && iExecStat != SQL_NO_DATA) {
									bExecutionResult = false;
									break;
								}
							}
							fseek(hSource, iOriginalFilePosition, SEEK_SET);
						}
						else if(!SQL_SUCCEEDED(iExecStat)) {
							bExecutionResult = false;
						}
					}
				}
				else{
					bExecutionResult = false;
				}
			}

			if(!bExecutionResult)
			{
				int iResult = IMPORT_RES_ERROR;

				//Need to do some advanced error reporting.
				int iNativeErr = 0;

				if(lpCCI->cCustSQL.GetErrorMessage(&iNativeErr, sAltTemp, sizeof(sAltTemp), hSTMT))
				{
					if(_strnicmp(sAltTemp, "Conversion Failed ", 18) == 0)
					{
						iResult = IMPORT_RES_SYNCTABLE;
					}
					else if(iNativeErr == SQLERROR_INVALID_COLUMN_NAME){
						iResult = IMPORT_RES_SYNCTABLE;
					}
					else if(iNativeErr == SQLERROR_INVALID_OBJECT_NAME){
						iResult = IMPORT_RES_SYNCTABLE;
					}
					else if(iNativeErr == SQLERROR_STRING_WOULD_BE_TRUNCATED){
						iResult = IMPORT_RES_SYNCTABLE;
					}
					else{
						sprintf_s(sStatusText, sizeof(sStatusText), "Error Msg: %s", sAltTemp);
						WriteLog(lpCCI->pClient->PeerID(), sStatusText, EVENT_NONE);
					}
				}

				if(iResult == IMPORT_RES_ERROR)
				{
					WriteLog(lpCCI->pClient->PeerID(),
						"Error in ImportSQLResults:: Execute failed on lpSrvSQL (See log file for more information)", EVENT_ERROR);

					if(lpSrvSQL->Length > (sizeof(sStatusText) - 25))
					{
						strcpy_s(sStatusText, sizeof(sStatusText), "Partial-Statement: ");
						strncat_s(sStatusText, sizeof(sStatusText), lpSrvSQL->Buffer, (sizeof(sStatusText) - 25));
					}
					else {
						sprintf_s(sStatusText, sizeof(sStatusText), "Statement: %s", lpSrvSQL->Buffer);
					}
					WriteLog(lpCCI->pClient->PeerID(), sStatusText, EVENT_NONE);
				}

				if(hSTMT) lpCCI->cCustSQL.FreeHandle(hSTMT);

				return iResult;
			}

			if(hSTMT) lpCCI->cCustSQL.FreeHandle(hSTMT);
		}

		lpSrvSQL->Clear();

        FreeColumnRowData(lpColumns);
		lpColumns->iAllocationLevel = COL_ALLOC_LEVEL_2;

        if(!pClient->IsConnected())
        {
			FormatInteger(sFormat1, sizeof(sFormat1), iRowIndex);

			sprintf_s(sStatusText, sizeof(sStatusText),
				"Disconnected durring data import, after %s successful rows.", sFormat1);
			WriteLog(lpCCI->pClient->PeerID(), sStatusText, EVENT_WARN);

            return IMPORT_RES_ERROR;
        }

		EnterCriticalSection(&csTransPerSecond);
		giTransProcessed++;
		LeaveCriticalSection(&csTransPerSecond);
    }

	if(iOperation == OPERATION_DELETE)
	{
		FormatInteger(sFormat1, sizeof(sFormat1), iRowCount);

		sprintf_s(sStatusText, sizeof(sStatusText),
			"Successfully deleted %s client data rows.", sFormat1);
	}
	else {
		FormatInteger(sFormat1, sizeof(sFormat1), iRowCount);

		sprintf_s(sStatusText, sizeof(sStatusText),
			"Successfully imported %s client data rows.", sFormat1);
	}
	WriteLog(lpCCI->pClient->PeerID(), sStatusText, EVENT_NONE);

    return IMPORT_RES_OK;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#endif
