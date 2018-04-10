///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//  Copyright © NetworkDLS 2002, All rights reserved
//
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF 
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO 
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A 
// PARTICULAR PURPOSE.
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef _NEW_DB_CPP
#define _NEW_DB_CPP
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/*
This is old functionality that will be going away in the future as it nolonger required.
*/

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#include <Windows.H>
#include <WindowsX.H>
#include <ShellAPI.H>
#include <Stdio.H>
#include <Stdlib.H>
#include <SQL.H>
#include <SQLExt.H>

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "../../SharedClasses/CGetPKs/CGetPKs.H"

#include "../../../@Libraries/CSocketServer/CSocketServer.H"

#include "../../SharedClasses/CStatusDlg/CStatusDlg.H"

#include "../../SharedSource/Common.H"

#include "Init.H"
#include "Routines.H"
#include "Console.H"
#include "SQLImport.H"
#include "NewDB.H"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

using namespace NSWFL::Windows;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool SetupNewClient(char *sClientDBFileName, char *sDBPrefix, char *sDBO, bool bImportStructure, 
					bool bImportData, bool bDropImportTable, CStatusDlg *lpDlg)
{
	/*
		sClientDBFileName: Contains the name of the backup file that contains the users existing
			data and structure. (Relative to the server)

		sDBName: Contains the base name of the new DB to create for the new client.

		bImportStructure: Should we import the structure of the sClientDBFileName to our new DB?

		bImportData: Should we import the data from the sClientDBFileName to our new DB?
	*/

    CSQL MySQL;
	CSQL CustSQL;
	CBoundRecordSet rsObjs;

	char sSQL[1024];
	char sFullDBName[1024];
	char sDBName[1024];

	if(bImportData && !bImportStructure)
	{
		lpDlg->SetText("You cannot import data if you opt not to import the DB structure.");
		return false;
	}

	lpDlg->SetText("Connecting to the SQL server.");
	if(!ConnectToSQLServer(&MySQL, gsSQLCustServer, gsSQLCustUserID, gsSQLCustPassword, "master",
		gbSQLCustUseTCPIPConnection, gbSQLCustUseTrustedConnection, gdwSQLCustServerPort))
    {
        lpDlg->SetText("Error connecting to the [master] database.");
        return false;
    }

	if(!GetDBNameFromBackup(&MySQL, sClientDBFileName, sDBName, sizeof(sDBName)))
	{
        lpDlg->SetText("Failed to load the database backup file information.");
        return false;
	}

	sprintf_s(sFullDBName, sizeof(sFullDBName), "SQLExch_%s_%s", sDBPrefix, sDBName);

	if(bImportStructure || bImportData)
	{
		lpDlg->SetText("Restoring the import database. (This could take some time)");
		if(!RestoreDBForImport(&MySQL, sClientDBFileName, sFullDBName))
		{
			lpDlg->SetText("Failed to restore the clients database.");
			MySQL.Disconnect();
			return false;
		}
	}

	lpDlg->SetText("Creating the new client database.");
	if(!CreateDatabaseIfNotExist(&MySQL, NULL, NULL, sFullDBName))
	{
		lpDlg->SetText("Failed to create the new database.");
		MySQL.Disconnect();
		return false;
	}

	lpDlg->SetText("Connecting to the SQL server.");
	if(!ConnectToSQLServer(&CustSQL, gsSQLCustServer, gsSQLCustUserID, gsSQLCustPassword, sFullDBName,
		gbSQLCustUseTCPIPConnection, gbSQLCustUseTrustedConnection, gdwSQLCustServerPort))
    {
        lpDlg->SetText("Error connecting to the [master] database.");
        return false;
    }

	lpDlg->SetText("Dropping any existing database structure.");
	sprintf_s(sSQL, sizeof(sSQL),
		"SELECT [name], [xtype]"
		" FROM [%s].[%s].[sysobjects]"
		" WHERE [xtype] IN ('U', 'TR')",
		sFullDBName, sDBO);

	CustSQL.Execute(sSQL, &rsObjs);

	char sType[100];
	char sObj[1024];
	int iSz = 0;
	int iProgress = 0;

	CustSQL.ThrowErrors(false);
	rsObjs.ThrowErrors(false);

	iProgress = 0;
	lpDlg->SetProgressRange(0, (int)rsObjs.RowCount);
	
	while(rsObjs.Fetch())
	{
		rsObjs.Values("name")->ToString(sObj, sizeof(sObj));
		rsObjs.Values("xtype")->ToString(sType, sizeof(sType));

		if(_strcmpi(sType, "U") == 0)
		{
			sprintf_s(sSQL, sizeof(sSQL), "DROP TABLE [%s]", sObj);
		}
		else if(_strcmpi(sType, "TR") == 0)
		{
			sprintf_s(sSQL, sizeof(sSQL), "DROP TRIGGER [%s]", sObj);
		}
		CustSQL.Execute(sSQL);
		
		iProgress++;

		lpDlg->SetProgressPos(iProgress);
	}

	rsObjs.Close();

	CustSQL.Disconnect();

	if(bImportStructure || bImportData)
	{
		lpDlg->SetText("Importing the database structure.");
		if(!ImportDBStructure(&MySQL, sDBO, sFullDBName, bImportData, lpDlg))
		{
			lpDlg->SetText("Failed to import the database structure.");
			MySQL.Disconnect();
			return false;
		}
	}

	if(bDropImportTable)
	{
		lpDlg->SetText("Dropping the import database.");
		sprintf_s(sSQL, sizeof(sSQL), "DROP DATABASE [%s_TMP]", sFullDBName);
		MySQL.Execute(sSQL);
	}

	MySQL.Disconnect();

	return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool ImportDBStructure(CSQL *lpMySQL, char *sDBO, char *sDBName, bool bImportData, CStatusDlg *lpDlg)
{
	char *sBaseSQL = NULL;
	char sSQL[5120];

	int iTempSz = 0;

	FILE *hSource = NULL;
	
	char *sLargeSQL = NULL;
	char sTable[1024];
	long lTableID = 0;

	char sColName[1024];
	char sColType[255];
	long lColLen = 0;
	long lColStatus = 0;
	long lColNullable = 0;

	char sSmlTmp[32];
	char sLgTmp[1024];

	int iPK = 0;
	int iRow = 0;
	int iLargeSQLSz = 1024 * 1024;

	CGetPKs cMyPKs;

	CBoundRecordSet rsTables;
	CBoundRecordSet rsColumns;

	sBaseSQL = "SELECT [name], [id]"
		" FROM [%s_TMP].[%s].[sysobjects]"
		" WHERE xtype = 'U' AND [name] <> 'dtproperties' AND LEFT([name], 8) <> 'SQLExch_'"
		" ORDER BY [name]";
	sprintf_s(sSQL, sizeof(sSQL), sBaseSQL, sDBName, sDBO);

	if(!lpMySQL->Execute(sSQL, &rsTables))
	{
		int iNativeErr = 0;
		lpMySQL->GetErrorMessage(&iNativeErr, sLgTmp, sizeof(sLgTmp), rsTables.hSTMT);
		
		sprintf_s(sSQL, sizeof(sSQL), "Failed to import the database structure: %s", sLgTmp);
		WriteSysLogEx(sSQL, EVENT_ERROR);

		return false;
	}

	lpDlg->SetProgressRange(0, (int)rsTables.RowCount);
	
	sLargeSQL = (char *) pMem->Allocate(iLargeSQLSz, sizeof(char));
	Assert(!sLargeSQL, "Memory Allocation Error.");

	while(rsTables.Fetch())
	{
		rsTables.Values("name")->ToString(sTable, sizeof(sTable));
		lTableID = rsTables.Values("id")->ToIntegerS();

		sBaseSQL =
			"SELECT [Columns].[name] AS ColName, [Columns].[length] AS ColLen, [Types].[name] AS TypeName,"
			" (CASE WHEN [Types].[name] IN ('binary', 'char', 'nchar', 'nvarchar', 'varbinary', 'varchar') THEN 1 ELSE 0 END) AS [TypeStatus],"
			" [Columns].[isnullable] AS IsNullable"
			" FROM [%s_TMP].[%s].[syscolumns] AS [Columns]"
			" INNER JOIN [%s_TMP].[%s].[systypes] AS [Types] ON [Columns].[xtype] = [Types].[xusertype]"
			" WHERE [Columns].[id] = %d"
			" ORDER BY [Columns].[colorder]";
		sprintf_s(sSQL, sizeof(sSQL), sBaseSQL, sDBName, sDBO, sDBName, sDBO, lTableID);

		if(!lpMySQL->Execute(sSQL, &rsColumns))
		{
			int iNativeErr = 0;
			lpMySQL->GetErrorMessage(&iNativeErr, sLgTmp, sizeof(sLgTmp), rsColumns.hSTMT);
			sprintf_s(sSQL, sizeof(sSQL), "Failed to import the database structure: %s", sLgTmp);
			WriteSysLogEx(sSQL, EVENT_ERROR);

			rsTables.Close();
			pMem->Free(sLargeSQL);
			return false;
		}

		//------------------------------------------------------------------------------------------
		//Drop the existing table
		//------------------------------------------------------------------------------------------
		lpMySQL->ThrowErrors(false);
		sprintf_s(sSQL, sizeof(sSQL), "DROP TABLE [%s].[%s].[%s]", sDBName, sDBO, sTable);
		lpMySQL->Execute(sSQL);
		lpMySQL->ThrowErrors(true);
		//------------------------------------------------------------------------------------------

		sprintf_s(sLargeSQL, iLargeSQLSz,
			"CREATE TABLE [%s].[%s].[%s] (\r\n", sDBName, sDBO, sTable);

		sprintf_s(sLgTmp, sizeof(sLgTmp), "%s_TMP", sDBName);
		cMyPKs.Get(lpMySQL, sLgTmp, sDBO, sTable);

		while(rsColumns.Fetch())
		{
			rsColumns.Values("ColName")->ToString(sColName, sizeof(sColName));
			lColLen = rsColumns.Values("ColLen")->ToIntegerS();
			rsColumns.Values("TypeName")->ToString(sColType, sizeof(sColType));
			lColStatus = rsColumns.Values("TypeStatus")->ToIntegerS();
			lColNullable = rsColumns.Values("IsNullable")->ToIntegerS();

			strcat_s(sLargeSQL, iLargeSQLSz, "\t[");
			strcat_s(sLargeSQL, iLargeSQLSz, sColName);
			strcat_s(sLargeSQL, iLargeSQLSz, "] [");
			strcat_s(sLargeSQL, iLargeSQLSz, sColType);
			strcat_s(sLargeSQL, iLargeSQLSz, "]");
			if(lColStatus == 1)
			{
				if(lColLen <= 0)
				{
					strcat_s(sLargeSQL, iLargeSQLSz, "(MAX)");
				}
				else{
					sprintf_s(sSmlTmp, sizeof(sSmlTmp), " (%d)", lColLen);
					strcat_s(sLargeSQL, iLargeSQLSz, sSmlTmp);
				}
			}
			
			if(lColNullable == 0)
			{
				strcat_s(sLargeSQL, iLargeSQLSz, " NOT NULL,\r\n");
			}
			else {
				strcat_s(sLargeSQL, iLargeSQLSz, " NULL,\r\n");
			}
		}

		iTempSz = (int)strlen(sLargeSQL);
		sLargeSQL[iTempSz - 1] = '\0';
		sLargeSQL[iTempSz - 2] = '\0';
		sLargeSQL[iTempSz - 3] = '\0';
		strcat_s(sLargeSQL, iLargeSQLSz, "\r\n");

		if(cMyPKs.Columns.Count > 0)
		{
			strcat_s(sLargeSQL, iLargeSQLSz, "\tCONSTRAINT [");
			strcat_s(sLargeSQL, iLargeSQLSz, cMyPKs.Constraint);
			strcat_s(sLargeSQL, iLargeSQLSz, "] PRIMARY KEY CLUSTERED\r\n");
			strcat_s(sLargeSQL, iLargeSQLSz, "\t(\r\n");

			iPK = 0;
			while(iPK < cMyPKs.Columns.Count)
			{
				strcat_s(sLargeSQL, iLargeSQLSz, "\t\t[");
				strcat_s(sLargeSQL, iLargeSQLSz, cMyPKs.Columns.Array[iPK].Name);
				strcat_s(sLargeSQL, iLargeSQLSz, "]");

				if(iPK != (cMyPKs.Columns.Count - 1))
				{
					strcat_s(sLargeSQL, iLargeSQLSz, ",\r\n");
				}
				iPK++;
			}
			strcat_s(sLargeSQL, iLargeSQLSz, "\r\n\t) ON [PRIMARY]\r\n");
			
		}

		strcat_s(sLargeSQL, iLargeSQLSz, ") ON [PRIMARY]");

		lpMySQL->Execute(sLargeSQL);

		if(bImportData)
		{
			//------------------------------------------------------------------------------------------
			//Import the data.
			//------------------------------------------------------------------------------------------
			sprintf_s(sSQL, sizeof(sSQL), 
				"INSERT INTO [%s].[%s].[%s]"
				" SELECT * FROM [%s_TMP].[%s].[%s]",
				sDBName, sDBO, sTable, sDBName, sDBO, sTable);
			lpMySQL->Execute(sSQL);
			//------------------------------------------------------------------------------------------
		}

		cMyPKs.Free();
		rsColumns.Close();

		iRow++;

		lpDlg->SetProgressPos(iRow);
	}
	
	pMem->Free(sLargeSQL);
	rsTables.Close();

	return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool GetDBNameFromBackup(CSQL *lpMySQL, const char *sBackupFile, char *sOutName, int iMaxSz)
{
	char sSQL[5120];

	bool bResult = false;

	CBoundRecordSet rsTemp;

	sprintf_s(sSQL, sizeof(sSQL), "RESTORE HEADERONLY FROM DISK = '%s'", sBackupFile);
	if(!lpMySQL->Execute(sSQL, &rsTemp))
	{
		MsgBox("The backup file specified is either invalid or does not exist.");
		return false;
	}

	if(rsTemp.Fetch())
	{
		rsTemp.Values("DatabaseName")->ToString(sOutName, iMaxSz);
		bResult = true;
	}

	rsTemp.Close();

	return bResult;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool RestoreDBForImport(CSQL *lpMySQL, char *sClientDBFileName, char *sDBName)
{
	char *sBaseSQL = NULL;
	char sSQL[5120];

	char sLogicalData[1024];
	char sLogicalLog[1024];

	CBoundRecordSet rsTemp;

	sprintf_s(sSQL, sizeof(sSQL), "RESTORE FILELISTONLY FROM DISK = '%s'", sClientDBFileName);
	if(!lpMySQL->Execute(sSQL, &rsTemp))
	{
		MsgBox("The backup file specified is either invalid or does not exist.");
		return false;
	}

	strcpy_s(sLogicalData, sizeof(sLogicalData), "");
	strcpy_s(sLogicalLog, sizeof(sLogicalLog), "");

	while(rsTemp.Fetch())
	{
		if(_strcmpi(rsTemp.Values("Type")->ToString(), "D") == 0)
		{
			//Data File.
			strcpy_s(sLogicalData, sizeof(sLogicalLog), rsTemp.Values("LogicalName")->ToString());
		}
		else if(_strcmpi(rsTemp.Values("Type")->ToString(), "L") == 0)
		{
			//Log File.
			strcpy_s(sLogicalLog, sizeof(sLogicalLog), rsTemp.Values("LogicalName")->ToString());
		}
	}

	rsTemp.Close();

	if(strlen(sLogicalData) > 0 && strlen(sLogicalLog) > 0)
	{
		sBaseSQL = "RESTORE DATABASE [%s_TMP] FROM DISK = '%s'\n"
			"WITH REPLACE, RECOVERY, MOVE '%s' TO '%s\\%s_TMP_Dat.mdf',\n"
			"MOVE '%s' TO '%s\\%s_TMP_Log.ldf'\n";

		sprintf_s(sSQL, sizeof(sSQL), sBaseSQL, sDBName, sClientDBFileName,
			sLogicalData, gsSQLDataFiles, sDBName,
			sLogicalLog, gsSQLLogFiles, sDBName);
	}
	else{
		MsgBox("Failed to obtain a list of logical file names from the backup file specified.");
		return false;
	}

	return lpMySQL->Execute(sSQL);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool CreateDatabaseIfNotExist(CSQL *lpMySQL, CSocketServer *pSock, CSocketClient *pClient, const char *sDB)
{
	_ConnectedClientInfo *pCCI = (_ConnectedClientInfo *)pClient->pUserData;

	char sTemp[5120];

	if(!DoesDatabaseExist(lpMySQL, sDB))
	{
		if(pSock)
		{
			sprintf_s(sTemp, sizeof(sTemp), "The database [%s] does not exist. Creating it...", sDB);
			WriteLog(pClient->PeerID(), sTemp, EVENT_NONE);
		}

		if(CreateNewDB(lpMySQL, sDB))
		{
			sprintf_s(sTemp, sizeof(sTemp), 
				"UPDATE Company"
				" SET InitializeStep = 0"
				" WHERE [Name] = '%s'",
				pCCI->sCompanyName);

			pCCI->cIndexSQL.Execute(sTemp);
			pCCI->bRequestInit = true;
		}
		else{
			if(pSock)
			{
				sprintf_s(sTemp, sizeof(sTemp), "Failed to create the database [%s].", sDB);
				WriteLog(pClient->PeerID(), sTemp, EVENT_ERROR);
			}
			return false;			
		}
	}

	return true;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool CreateNewDBFromDBCreationScript(CSQL *lpMySQL, CSocketServer *pSock, CSocketClient *pClient, char *sFileName)
{
	_ConnectedClientInfo *pCCI = (_ConnectedClientInfo *)pClient->pUserData;

	FILE *fSource = NULL;
	int iLen = 0;
	SQLLEN iTables = 0;

	bool bThrowSQLErr = lpMySQL->ThrowErrors();

	bool bResult = true;

	char sTemp[1024];
	char sSQL[1024];
	char sDB[255];
	char sTable[255];
	char sSchema[255];
	char sPKName[255];
	char sCurrentFocus[255];
	char *sLargeSQL1 = NULL;
	int iLargeSQLAlloc = 1024 * 100;

	//Get the name of the current database. (we'll need to reset it)
	if(!lpMySQL->GetFocus(sCurrentFocus, sizeof(sCurrentFocus)))
	{
		//fixme: handle error.
	}

	if(fopen_s(&fSource, sFileName, "rb") != 0)
	{
		return false;
	}

	sLargeSQL1 = (char *) pMem->Allocate(sizeof(char), iLargeSQLAlloc);
	Assert(!sLargeSQL1, "Memory Allocation Error.");

	//----------------------------------------------------------------------------------------------------

	int iBytesRead = 0;

	while(true)
	{
		//Read Table count.
		if(fread(&iTables, sizeof(iTables), 1, fSource) == 0)
		{
			bResult = true;
			break;
		}

		for(int iTable = 0; iTable < iTables && pClient->IsConnected(); iTable++)
		{
			//Read Database Name.
			fread(&iLen, sizeof(iLen), 1, fSource);
			fread(sDB, sizeof(char), iLen, fSource);
			sDB[iLen] = '\0';

			strcpy_s(sTemp, sizeof(sTemp), sDB);
			strcpy_s(sDB, sizeof(sDB), "SQLExch_");
			strcat_s(sDB, sizeof(sDB), pCCI->sCompanyDBPrefix);
			strcat_s(sDB, sizeof(sDB), "_");
			strcat_s(sDB, sizeof(sDB), sTemp);
			
			if(iTable == 0) //Only check DB existence one time per table.
			{
				if(!CreateDatabaseIfNotExist(&pCCI->cCustSQL, pSock, pClient, sDB))
				{
					bResult = false;
					break;
				}

				pCCI->cCustSQL.Focus(sDB); 
			}

			//Read Schema Name.
			fread(&iLen, sizeof(iLen), 1, fSource);
			fread(sSchema, sizeof(char), iLen, fSource);
			sSchema[iLen] = '\0';

			//Read Table Name.
			fread(&iLen, sizeof(iLen), 1, fSource);
			fread(sTable, sizeof(char), iLen, fSource);
			sTable[iLen] = '\0';

			sprintf_s(sSQL, sizeof(sSQL), "Creating table: %s.%s.%s", sDB, sSchema, sTable);
			WriteLog(pClient->PeerID(), sSQL, EVENT_NONE);

			//Read PK Name.
			fread(&iLen, sizeof(iLen), 1, fSource);
			fread(sPKName, sizeof(char), iLen, fSource);
			sPKName[iLen] = '\0';

			//Read SQL.
			fread(&iLen, sizeof(iLen), 1, fSource);
			fread(sLargeSQL1, sizeof(char), iLen, fSource);
			sLargeSQL1[iLen] = '\0';

			//Drop the existing table.
			lpMySQL->ThrowErrors(false);
			sprintf_s(sSQL, sizeof(sSQL), "DROP TABLE [%s].[%s].[%s]", sDB, sSchema, sTable);
			lpMySQL->Execute(sSQL);

			//Create the table schema.
			sprintf_s(sSQL, sizeof(sSQL), "CREATE SCHEMA [%s]", sSchema);
			lpMySQL->Execute(sSQL);
			lpMySQL->ThrowErrors(true);

			StrRepInPlace(sLargeSQL1, iLargeSQLAlloc, "{SQLExch_TAG_DB}", sDB);

			//Recreate the table.
			if(!lpMySQL->Execute(sLargeSQL1))
			{
				bResult = false;
				break;
			}
		}

		if(bResult == false)
		{
			break;
		}

		if(!pClient->IsConnected())
		{
			break;
		}

	}

	lpMySQL->ThrowErrors(bThrowSQLErr);

	fclose(fSource);
	pMem->Free(sLargeSQL1);

	lpMySQL->Focus(sCurrentFocus);

	if(pClient->IsConnected() && bResult == true)
	{
		return true;
	}

	return false;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#endif
