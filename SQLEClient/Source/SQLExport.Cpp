///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//  Copyright © NetworkDLS 2002, All rights reserved
//
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF 
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO 
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A 
// PARTICULAR PURPOSE.
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef _SQLEXPORT_CPP
#define _SQLEXPORT_CPP
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include <Windows.H>
#include <WindowsX.H>
#include <ShellAPI.H>
#include <Stdio.H>
#include <Stdlib.H>
#include <SQL.H>
#include <SQLExt.H>

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "Entry.H"

#include "Command.H"
#include "SQLExport.H"
#include "Console.H"

#include "../../SharedClasses/CGetPKs/CGetPKs.H"

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool RTrimType(const int iSQLType)
{
	return(iSQLType == TSQL_NCHAR || iSQLType == TSQL_CHAR);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool IsComplexType(const int iSQLType)
{
	if(iSQLType == TSQL_NTEXT
		|| iSQLType == TSQL_NCHAR
		|| iSQLType == TSQL_NVARCHAR
		|| iSQLType == TSQL_SQL_VARIANT)
	{
		return true;
	}
	else if(iSQLType == TSQL_TEXT
		|| iSQLType == TSQL_CHAR
		|| iSQLType == TSQL_VARCHAR)
	{
		return true;
	}
	else if(iSQLType == TSQL_BINARY
		|| iSQLType == TSQL_VARBINARY
		|| iSQLType == TSQL_IMAGE)
	{
		return true;
	}

	return false;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool GetExportType(CSocketServer *pSock, CSocketClient *pClient, const int iDataType, int *piSQLType)
{
	if(iDataType == TSQL_NTEXT
		|| iDataType == TSQL_NCHAR
		|| iDataType == TSQL_NVARCHAR
		|| iDataType == TSQL_SQL_VARIANT)
	{
		*piSQLType = SQL_UNICODE_CHAR;
		return true;
	}
	else if(iDataType == TSQL_TEXT
		|| iDataType == TSQL_CHAR
		|| iDataType == TSQL_VARCHAR)
	{
		*piSQLType = SQL_CHAR;
		return true;
	}
	else if(iDataType == TSQL_BINARY
		|| iDataType == TSQL_VARBINARY
		|| iDataType == TSQL_IMAGE)
	{
		*piSQLType = SQL_BINARY;
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

bool BufferDataToFile(CSocketServer *pSock, CSocketClient *pClient,
					  CRecordSet *rsLocal, short iColumn,
					  int iDataType, short iSQLType, FILE *hTarget,
					  char *sBuf, SQLLEN iBufSz)
{
	char sText[1024];
	SQLLEN iWriteLen = 0;
	SQLLEN iOriginalLength = 0;
	SQLLEN iWrittenLength = 0;
	short iResult = 0;

	if(rsLocal->GetData(iColumn, iSQLType, sBuf, 0, &iOriginalLength, &iResult))
	{
		fwrite(&iOriginalLength, sizeof(iOriginalLength), 1, hTarget); //Write the data length (bytes).
		if(iOriginalLength == -1)
		{
			return true;
		}

		do {
			if(rsLocal->GetData(iColumn, iSQLType, sBuf, iBufSz, &iWriteLen, &iResult))
			{
				if(iResult == SQL_SUCCESS_WITH_INFO || iResult == SQL_SUCCESS)
				{
					if(iResult == SQL_SUCCESS_WITH_INFO)
					{
						if(rsLocal->GetSQLState(sText))
						{
							if(strcmp(sText, "01004") == 0) //SQLSTATE 01004 (String data, right truncated)
							{
								iWriteLen = iBufSz;

								if(iSQLType != SQL_C_BINARY)
								{
									//Trim off the NULL terminator (could be double byte too): 
									for(short iNullCheck = 0;
										sBuf[iWriteLen - 1] == '\0' && iNullCheck < 2;
										iNullCheck++)
									{
										iWriteLen--;
									}
								}
							}
							else{
								sprintf_s(sText, sizeof(sText), 
									"Unknown SQL State (Column:%d, Type: %d, Result: %d, State: %s).",
									iColumn, iSQLType, iResult, sText);
								WriteLog(pClient->PeerID(), sText, EVENT_WARN);
								return false;
							}
						}
						else{
							sprintf_s(sText, sizeof(sText), 
								"Failed the retrieve SQL state (Column:%d, Type: %d, Result: %d).",
								iColumn, iSQLType, iResult);
							WriteLog(pClient->PeerID(), sText, EVENT_WARN);
							return false;
						}
					}

					fwrite(sBuf, sizeof(byte), iWriteLen, hTarget);
					iWrittenLength += iWriteLen;
				}
				else {
					sprintf_s(sText, sizeof(sText), 
						"Failed the retrieve data from the SQL column (Column:%d, Type: %d, Result: %d).",
						iColumn, iSQLType, iResult);
					WriteLog(pClient->PeerID(), sText, EVENT_WARN);
					return false;
				}
			}
			else{
				sprintf_s(sText, sizeof(sText), 
					"Failed the retrieve data from the SQL column (Column:%d, Type: %d, Result: %d).",
					iColumn, iSQLType, iResult);
				WriteLog(pClient->PeerID(), sText, EVENT_WARN);
				return false;
			}
		} while(iResult != SQL_SUCCESS);
	}
	else{
		sprintf_s(sText, sizeof(sText), 
			"Failed to retrieve the initial size of SQL column (Column:%d, Type: %d).",
			iColumn, iSQLType);
		WriteLog(pClient->PeerID(), sText, EVENT_WARN);
		return false;
	}

	if(iWrittenLength != iOriginalLength)
	{
		sprintf_s(sText, sizeof(sText), 
			"Failed to read the exact dataset size (Column:%d, Type: %d).",
			iColumn, iSQLType);
		WriteLog(pClient->PeerID(), sText, EVENT_WARN);
		return false;
	}

	return true;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int ExportSQLResults(CSocketServer *pSock, CSocketClient *pClient, int iID, char *Statement,
					 char *sDB, char *sSchema, char *sImportTable, int iOperation, char *OutFileName)
{
	FILE *hTarget = NULL;

	CRecordSet rsTemp;

    SQLRETURN SQLResult = 0;

    char sStatusText[MAX_STATUS_TEXT]; //Just some RAM for status text and error messages.
    char sOutColumnName[256];

	int iTemp = 0;
	int iInColumnNameLen = 256;
	SQLULEN *iOutColumnSize = 0;
    int iOutColumnNameLen = 0;
    int *iOutDataType = 0;
    int iOutDecimalDigits = 0;
    int iOutNullable = 0;
    int iColumn = 0;
	int iRowCountPlaceholder = 0;
	unsigned int iProcessedRowCount = 0;

	rsTemp.ReplaceSingleQuotes(false);
	rsTemp.ThrowErrors(true);
	rsTemp.TrimCharData(gbTrimStringDataOnExport);

	/*
	if(!CCI.cCustSQL.Focus(sDB))
	{
		WriteLog(pClient->PeerID(),
			"Failed to change client database context. Performance will be affected.", EVENT_WARN);
	}
	*/

	char *sSQLBuf = NULL;
	int iSQLBufSz = (32 * 1024);

	if(gbDebugMode)
	{
		sprintf_s(sStatusText, sizeof(sStatusText),
			"Executing SQL statement on: [%s].[%s].[%s]",
			sDB, sSchema, sImportTable);
		WriteLog(pClient->PeerID(), sStatusText, EVENT_NONE);
	}

	if(!CCI.cCustSQL.Execute(Statement, &rsTemp, SQL_CURSOR_FORWARD_ONLY))
    {
		WriteLog(pClient->PeerID(), "ExportSQLResults SELECT Execute failed. (See log file for more information)", EVENT_ERROR);
		WriteLog(pClient->PeerID(), Statement, EVENT_INFO);

		char *sNativeError = (char *)pMem->Allocate(sizeof(char), MAX_STATUS_TEXT);

		//Need to do some advanced error reporting.
		int iNativeErr = 0;
		if(CCI.cCustSQL.GetErrorMessage(&iNativeErr, sNativeError, MAX_STATUS_TEXT, rsTemp.hSTMT))
		{
			sprintf_s(sStatusText, sizeof(sStatusText), "Error Msg: %s", sNativeError);
			WriteLog(pClient->PeerID(), sStatusText, EVENT_ERROR);
		}

		pMem->Free(sNativeError);

		return SQL_EXPORT_RESULT_ERROR;
    }

	if(!rsTemp.Fetch()) //This grabs the first row!
	{
		if(gbDebugMode)
		{
			WriteLog(pClient->PeerID(), "Query returned zero rows.", EVENT_NONE);
		}

        rsTemp.Close();

        return SQL_EXPORT_RESULT_ZEROROWS;
	}

	int iOutColCount = (int)rsTemp.ColumnCount;

	sprintf_s(sStatusText, sizeof(sStatusText),
		"Exporting transaction data: [%s].[%s].[%s]",
		sDB, sSchema, sImportTable);
	WriteLog(pClient->PeerID(), sStatusText, EVENT_NONE);

    if(fopen_s(&hTarget, OutFileName, "wb") != 0)
    {
        sprintf_s(sStatusText, sizeof(sStatusText),
			"TargetFile: Binary write error with (%s).", OutFileName);
		WriteLog(pClient->PeerID(), sStatusText, EVENT_ERROR);

        rsTemp.Close();

        return SQL_EXPORT_RESULT_ERROR;
    }

	if(gbDebugMode)
	{
		WriteLog(pClient->PeerID(), "Writing export file header.", EVENT_NONE);
	}

	fwrite(&iOperation, sizeof(iOperation), 1, hTarget);     // Write the operation flag.

	iTemp = (int)strlen(sDB);
    fwrite(&iTemp, sizeof(iTemp), 1, hTarget);               // Write import database name len.
    fwrite(sDB, sizeof(char), iTemp, hTarget);               // Write import database name.

	iTemp = (int)strlen(sSchema);
	fwrite(&iTemp, sizeof(iTemp), 1, hTarget);               // Write import schema name len.
    fwrite(sSchema, sizeof(char), iTemp, hTarget);           // Write import schema name.

	iTemp = (int)strlen(sImportTable);
	fwrite(&iTemp, sizeof(iTemp), 1, hTarget);               // Write import table name len.
    fwrite(sImportTable, sizeof(char), iTemp, hTarget);      // Write import table name.

	fwrite(&iOutColCount, sizeof(iOutColCount), 1, hTarget); // Write number of columns.
	iRowCountPlaceholder = ftell(hTarget);
    fwrite(&iProcessedRowCount, sizeof(iProcessedRowCount), 1, hTarget); // Write row count placeholder.

    fwrite(&iID, sizeof(iID), 1, hTarget); // Write the row ID of the associated Statments row.

    //---------------( Get column Information )---

	if(gbDebugMode)
	{
		WriteLog(pClient->PeerID(), "Retreving column set information.", EVENT_NONE);
	}

	iOutDataType = (int *) pMem->Allocate( sizeof(int), iOutColCount);
	Assert(!iOutDataType, "Memory Allocation Error.");
	iOutColumnSize = (SQLULEN *) pMem->Allocate( sizeof(SQLULEN), iOutColCount);
	Assert(!iOutColumnSize, "Memory Allocation Error.");

	while(rsTemp.GetColumnInfo(iColumn + 1, sOutColumnName, iInColumnNameLen, &iOutColumnNameLen,
		&iOutDataType[iColumn], &iOutColumnSize[iColumn], &iOutDecimalDigits, &iOutNullable))
	{
		bool bComplexType = IsComplexType(iOutDataType[iColumn]);
		fwrite(&bComplexType, sizeof(bComplexType), 1, hTarget);
		fwrite(&iOutDataType[iColumn], sizeof(iOutDataType[iColumn]), 1, hTarget);
		fwrite(&iOutColumnSize[iColumn], sizeof(iOutColumnSize[iColumn]), 1, hTarget);
		fwrite(&iOutColumnNameLen, sizeof(iOutColumnNameLen), 1, hTarget);
		fwrite(sOutColumnName, sizeof(char), iOutColumnNameLen, hTarget);

		iColumn++;
    }

	if(gbDebugMode)
	{
		WriteLog(pClient->PeerID(), "Exporting column set data.", EVENT_NONE);
	}

	if(!(sSQLBuf = (char *) pMem->Allocate(sizeof(char), iSQLBufSz)))
	{
		Assert(!sSQLBuf, "Memory Allocation Error.");
	}

	//---------------( Get column Data )---
    do {
        iColumn = 0;

		EnterCriticalSection(&csTransPerSecond);
		giTransProcessed++;
		LeaveCriticalSection(&csTransPerSecond);

		while(iColumn < iOutColCount)
        {

			if(!pClient->IsConnected())
			{
                sprintf_s(sStatusText, sizeof(sStatusText),
					"Disconnected durring data export. Table: [%s]", sImportTable);
				WriteLog(pClient->PeerID(), sStatusText, EVENT_WARN);

				pMem->Free(sSQLBuf);
				pMem->Free(iOutColumnSize);
				pMem->Free(iOutDataType);

				rsTemp.Close();

				if(hTarget) fclose(hTarget);

			    return SQL_EXPORT_RESULT_ERROR;				
			}

            int iDataType = iOutDataType[iColumn];
            SQLLEN iDataLen = iOutColumnSize[iColumn];

			//------------------------------------------------------------------------------------------
			if(iDataType == TSQL_DATETIME
				|| iDataType == TSQL_SMALLDATETIME
				|| iDataType == TSQL_UNIQUEIDENTIFIER)
			{
				if(rsTemp.sColumnEx(iColumn + 1, sSQLBuf, iSQLBufSz, &iDataLen))
                {
                    if(iDataLen < 0)
                    {
                        fwrite(&iDataLen, sizeof(iDataLen), 1, hTarget); //NULL Value.
                    }
                    else{
						sSQLBuf[iDataLen] = '\0';

						iDataLen = (iDataLen + 2); // To make room for the single quotes
						fwrite(&iDataLen, sizeof(iDataLen), 1, hTarget); //Write the data length.
						iDataLen = (iDataLen - 2); // Because we only write the original trimmed length

						fwrite("'", sizeof(char), 1, hTarget); // Wrap the data in single quotes
						fwrite(sSQLBuf, sizeof(char), iDataLen, hTarget); //Write the data.
						fwrite("'", sizeof(char), 1, hTarget); // Wrap the data in single quotes
					}
				}
				else{
	                sprintf_s(sStatusText, sizeof(sStatusText),
						"Failed to retreive data from column. Table: %s, Column: %d, Type: %d",
						sImportTable, iColumn, iDataType);
					WriteLog(pClient->PeerID(), sStatusText, EVENT_ERROR);

					pMem->Free(sSQLBuf);
					pMem->Free(iOutColumnSize);
					pMem->Free(iOutDataType);

					rsTemp.Close();

					if(hTarget) fclose(hTarget);

					return SQL_EXPORT_RESULT_ERROR;
				}
            }
			//------------------------------------------------------------------------------------------
			else if(iDataType == TSQL_BIGINT     || iDataType == TSQL_BIT        ||
					iDataType == TSQL_DECIMAL    || iDataType == TSQL_FLOAT      ||
					iDataType == TSQL_INT        || iDataType == TSQL_MONEY      ||
					iDataType == TSQL_NUMERIC    || iDataType == TSQL_REAL       ||
					iDataType == TSQL_SMALLINT   || iDataType == TSQL_SMALLMONEY ||
					iDataType == TSQL_TINYINT)
            {
                if(rsTemp.sColumnEx(iColumn + 1, sSQLBuf, iSQLBufSz, &iDataLen))
				{
                    if(iDataLen == -1)
                    {
                        fwrite(&iDataLen, sizeof(iDataLen), 1, hTarget); //NULl value.
                    }
                    else{
						fwrite(&iDataLen, sizeof(iDataLen), 1, hTarget);  //Write the data length.
						fwrite(sSQLBuf, sizeof(char), iDataLen, hTarget); //Write the data.
					}
                }
				else{
	                sprintf_s(sStatusText, sizeof(sStatusText),
						"Failed to retreive data from column. Table: %s, Column: %d, Type: %d",
						sImportTable, iColumn, iDataType);
					WriteLog(pClient->PeerID(), sStatusText, EVENT_ERROR);

					pMem->Free(sSQLBuf);
					pMem->Free(iOutColumnSize);
					pMem->Free(iOutDataType);

					rsTemp.Close();

					if(hTarget) fclose(hTarget);

					return SQL_EXPORT_RESULT_ERROR;
				}
            }
			//------------------------------------------------------------------------------------------
			else{
				int iSQLType = 0;

				if(GetExportType(pSock, pClient, iDataType, &iSQLType))
				{
					if(!BufferDataToFile(pSock, pClient, &rsTemp,
						iColumn + 1, iDataType, iSQLType, hTarget, sSQLBuf, iSQLBufSz))
					{
						sprintf_s(sStatusText, sizeof(sStatusText),
							"Failed to buffer data to destination file."
							"Table: %s, Select column: %d, Type: %d",
							sImportTable, iColumn, iDataType);
						WriteLog(pClient->PeerID(), sStatusText, EVENT_ERROR);

						pMem->Free(sSQLBuf);
						pMem->Free(iOutColumnSize);
						pMem->Free(iOutDataType);

						rsTemp.Close();

						if(hTarget) fclose(hTarget);

						return SQL_EXPORT_RESULT_ERROR;
					}
				}
				else{
					sprintf_s(sStatusText, sizeof(sStatusText),
						"Unknown SQL data type. Table: %s, Select column: %d, Type: %d",
						sImportTable, iColumn, iDataType);
					WriteLog(pClient->PeerID(), sStatusText, EVENT_ERROR);

					pMem->Free(sSQLBuf);
					pMem->Free(iOutColumnSize);
					pMem->Free(iOutDataType);

					rsTemp.Close();

					if(hTarget) fclose(hTarget);

					return SQL_EXPORT_RESULT_ERROR;
				}
			}
			//------------------------------------------------------------------------------------------

            iColumn++;
        }

		iProcessedRowCount++;

	} while(rsTemp.Fetch());

	fseek(hTarget, iRowCountPlaceholder, SEEK_SET);   //Seek back to the "Row Count Place holder".
    fwrite(&iProcessedRowCount, sizeof(iProcessedRowCount), 1, hTarget); // Write number of rows.

	if(gbDebugMode)
	{
		WriteLog(pClient->PeerID(), "Ending the export successfully.", EVENT_NONE);
	}

	pMem->Free(sSQLBuf);
	pMem->Free(iOutColumnSize);
	pMem->Free(iOutDataType);

	rsTemp.Close();

    if(hTarget) fclose(hTarget);

	return SQL_EXPORT_RESULT_OK;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#endif
