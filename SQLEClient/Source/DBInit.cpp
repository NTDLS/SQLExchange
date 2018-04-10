///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//  Copyright © NetworkDLS 2002, All rights reserved
//
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF 
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO 
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A 
// PARTICULAR PURPOSE.
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef _DBINIT_CPP
#define _DBINIT_CPP
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include <Windows.H>
#include <WindowsX.H>
#include <ShellAPI.H>
#include <Stdio.H>
#include <Stdlib.H>
#include <SQL.H>
#include <SQLExt.H>

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//#include "../../SharedClasses/CGetPKs/CGetPKs.H"

#include "Entry.H"
//#include "../../SharedSource/CRC32.H"

#include "Init.H"
#include "Console.H"
#include "DBInit.H"

#include "../Dialogs/ReplicationDlg.H"
#include "../Dialogs/MainDlg.H"
#include "../../../@Libraries/CStringBuilder/CStringBuilder.H"

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//FIXFIX: Implement the use of the CStringBuilder
bool GenerateTableCreationSQL(
	CSQL *lpCSQL, CSocketServer *pSock, CSocketClient *pClient,
	const char *sDB, const char *sSchema, const char *sTable,
	char *sOutPK, unsigned int iOutPKSz, CStringBuilder *pSQL)
{
	char sCol[1024];
	char sSmlTmp[64];
	char sType[255];
	char *sTmpPtr = NULL;

	int iColLen = 0;
	int iColScale = 0;
	int iColPrec = 0;
	int iColStat = 0;
	bool bIsPKCol = 0;
	int iLen = 0;

	CStringBuilder PKSQL;
	CBoundRecordSet rsTemp;

	lpCSQL->ThrowErrors(true);
	rsTemp.ThrowErrors(true);

	sTmpPtr = 
		"SELECT"
		"	TC.CONSTRAINT_NAME AS [PrimaryKey]"
		" FROM"
		"	[%s].INFORMATION_SCHEMA.TABLE_CONSTRAINTS AS TC"
		" WHERE"
		"	TC.CONSTRAINT_TYPE = 'PRIMARY KEY'"
		"	AND TC.TABLE_CATALOG = '%s'"
		"	AND TC.TABLE_SCHEMA = '%s'"
		"	AND TC.TABLE_NAME = '%s'";

	pSQL->Clear();
	pSQL->Resize((int)strlen(sTmpPtr) + ((int)strlen(sDB) * 2) + (int)strlen(sSchema) + (int)strlen(sTable) + 1);
	pSQL->AppendF(sTmpPtr, sDB, sDB, sSchema, sTable);
	lpCSQL->Execute(pSQL->Buffer, &rsTemp);

	if(rsTemp.Fetch())
	{
		rsTemp.Values("PrimaryKey")->ToString(sOutPK, iOutPKSz);
	}
	else{
		PKSQL.Destroy();

		rsTemp.Close();
		return false;
		//FIXFIX: Need to handle this error.
	}

	rsTemp.Close();

	sTmpPtr = 
		"SELECT"
		"	[Columns].[name] AS ColumnName,"
		"	[Types].[name] AS TypeName,"
		"	CASE WHEN [Types].[name] IN ('nchar', 'nvarchar') THEN"
		"			[Columns].[prec] ELSE [Columns].[length]"
		"	END AS [ColumnLength],"
		"	[Columns].[prec] ColumnPrecision,"
		"	[Columns].[xscale] AS ColumnScale,"
		"	CASE"
		"		WHEN [Types].[name] IN ('binary', 'char', 'nchar', 'nvarchar', 'varbinary', 'varchar')"
		"			THEN 1"	//Requires column Length to be specified.
		"		WHEN [Types].[name] IN ('decimal')"
		"			THEN 2"	//Requires column Precision and Scale to be specified
		"		ELSE"
		"			0"		//Column does not require Length, Precision or Scale to be specified
		"		END AS [TypeStatus],"
		"	IsNull("
		"		("
		"			SELECT"
		"				1"
		"			FROM"
		"				[%s].[%s].[sysindexkeys] AS IK"
		"			INNER JOIN [%s].[%s].[sysindexes] AS I"
		"				ON I.[indid] = IK.[indid]"
		"				AND I.[id] = IK.[id]"
		"			INNER JOIN [%s].[%s].[sysobjects] AS O"
		"				ON O.[xtype] = 'PK'"
		"				AND O.[name] = I.[name]"
		"				AND O.[parent_obj] = [Columns].[id]"
		"				AND O.[parent_obj] = [Columns].[id]"
		"			WHERE"
		"				IK.[colid] = [Columns].[colid]"
		"	), 0) AS [IsPK]"
		" FROM"
		"	[%s].[%s].[syscolumns] AS [Columns]"
		" INNER JOIN [%s].[%s].[systypes] AS [Types]"
		"	ON [Columns].[xtype] = [Types].[xusertype]"
		" WHERE"
		"	[Columns].[id] = OBJECT_ID('[%s].[%s].[%s]')"
		" ORDER BY"
		"	[Columns].[colorder]";
	pSQL->Clear();
	pSQL->Resize((int)strlen(sTmpPtr) + ((int)strlen(sDB) * 6)
		+ ((int)strlen(gsDefaultDBO) * 5) + (int)strlen(sSchema) + (int)strlen(sTable) + 1);
	pSQL->AppendF(sTmpPtr, sDB, gsDefaultDBO, sDB, gsDefaultDBO,
		sDB, gsDefaultDBO, sDB, gsDefaultDBO, sDB, gsDefaultDBO, sDB, sSchema, sTable);
	lpCSQL->Execute(pSQL->Buffer, &rsTemp);

	pSQL->Clear();
	pSQL->Append("CREATE TABLE [{SQLExch_TAG_DB}].[");
	pSQL->Append(sSchema);
	pSQL->Append("].[");
	pSQL->Append(sTable);
	pSQL->Append("] (\r\n");

	PKSQL.Append("\tCONSTRAINT [");
	PKSQL.Append(sOutPK);
	PKSQL.Append("] PRIMARY KEY CLUSTERED\r\n");
	PKSQL.Append("\t(\r\n");

	while(rsTemp.Fetch())
	{
		rsTemp.Values("ColumnName")->ToString(sCol, sizeof(sCol));
		rsTemp.Values("TypeName")->ToString(sType, sizeof(sType));
		iColLen = rsTemp.Values("ColumnLength")->ToShortS();
		iColPrec = rsTemp.Values("ColumnPrecision")->ToShortS();
		iColScale = rsTemp.Values("ColumnScale")->ToShortS();
		iColStat = rsTemp.Values("TypeStatus")->ToShortS();
		bIsPKCol = rsTemp.Values("IsPK")->ToBoolean();

		pSQL->Append("\t[");
		pSQL->Append(sCol);
		pSQL->Append("] ");
		pSQL->Append(sType);

		if(iColStat == 1)
		{
			if(iColLen <= 0)
			{
				pSQL->Append("(MAX)");
			}
			else{
				sprintf_s(sSmlTmp, sizeof(sSmlTmp), " (%d)", iColLen);
				pSQL->Append(sSmlTmp);
			}
		}
		else if(iColStat == 2)
		{
			sprintf_s(sSmlTmp, sizeof(sSmlTmp), " (%d, %d)", iColPrec, iColScale);
			pSQL->Append(sSmlTmp);
		}

		if(bIsPKCol)
		{
			PKSQL.Append("\t\t[");
			PKSQL.Append(sCol);
			PKSQL.Append("],\r\n");
			pSQL->Append(" NOT NULL,\r\n");
		}
		else{
			pSQL->Append(" NULL,\r\n");
		}
	}

	rsTemp.Close();

	PKSQL.Terminate(PKSQL.Length - 3); //Remove the trailing CR, LF & Comma.
	pSQL->Terminate(pSQL->Length - 3); //Remove the trailing CR, LF & Comma.

	PKSQL.Append("\r\n\t) ON [PRIMARY]");

	pSQL->Append("\r\n");
	pSQL->Append(PKSQL.Buffer);
	pSQL->Append("\r\n");
	pSQL->Append(") ON [PRIMARY]\r\n");
	pSQL->Terminate();

	PKSQL.Destroy();

	return true;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int GenerateDBCreationSQL(CSQL *lpCSQL, CSocketServer *pSock, CSocketClient *pClient, char *sTargetFile)
{
	//WriteCon("GenerateDBCreationSQL\n");

	char sDB[1024];
	char sText[1024];
	char sTable[1024];
	char sSchema[1024];
	char sPKName[1024];
	int iTempSz = 0;
	int iLen = 0;
	int iTables = 0;
	char *sTempPtr = NULL;

	CStringBuilder SQL;

	CBoundRecordSet rsTables;
	CBoundRecordSet rsDatabases;

	FILE *hTarget = NULL;

	if(fopen_s(&hTarget, sTargetFile, "wb") != 0)
	{
		SQL.Destroy();
		return CMD_ERROR;
	}

	lpCSQL->ThrowErrors(true);
	rsDatabases.ThrowErrors(true);

	sTempPtr =
		"SELECT"
		"	[name] AS DatabaseName"
		" FROM"
		"	[sysdatabases]"
		" WHERE"
		"	[name] NOT LIKE 'SQLExch_%'"
		"	AND [name] NOT IN ('master', 'model', 'msdb', 'tempdb')"
		" ORDER BY"
		"	[name]";
	lpCSQL->Execute(sTempPtr, &rsDatabases);	

	while(rsDatabases.Fetch())
	{
		rsDatabases.Values("DatabaseName")->ToString(sDB, sizeof(sDB));
		if(!lpCSQL->Focus(sDB))
		{
			sprintf_s(sText, sizeof(sText), "Failed to set database context. Does [%s] exist?", sDB);
			WriteLog(pClient->PeerID(), sText, EVENT_WARN);
		}
		else{
			//Create a record set containing all of the table names that contain SQLExch Triggers
			sTempPtr =
				"SELECT"
				"	T.TABLE_SCHEMA AS [Schema],"
				"	T.TABLE_NAME AS [Table]"
				" FROM"
				"	[%s].INFORMATION_SCHEMA.TABLES AS T"
				" WHERE"
				"	T.TABLE_TYPE = 'BASE TABLE'"
				"	AND OBJECT_ID(T.TABLE_CATALOG + '.' + T.TABLE_SCHEMA + '.' + T.TABLE_NAME) IN"
				"	("
				"		SELECT DISTINCT"
				"			TR.parent_obj"
				"		FROM"
				"			[%s].[%s].sysobjects AS TR"
				"		WHERE TR.xtype = 'TR'"
				"			AND TR.name LIKE 'SQLExch[_]%%'"
				"	)"
				" ORDER BY"
				"	T.TABLE_NAME";
			SQL.Clear();
			SQL.AppendF(sTempPtr, sDB, sDB, gsDefaultDBO);
			lpCSQL->Execute(SQL.Buffer, &rsTables);

			if(rsTables.RowCount > 0)
			{
				fwrite(&rsTables.RowCount, sizeof(rsTables.RowCount), 1, hTarget);

				//Loop through all of the tables that have triggers.
				while(rsTables.Fetch())
				{
					//Get the table name, save it in sTable[]
					rsTables.Values("Table")->ToString(sTable, sizeof(sTable));
					rsTables.Values("Schema")->ToString(sSchema, sizeof(sSchema));

					sprintf_s(sText, sizeof(sText), "Generating SQL for %s.%s.%s", sDB, sSchema, sTable);
					WriteLog(pClient->PeerID(), sText, EVENT_NONE);

					if(!GenerateTableCreationSQL(lpCSQL, pSock, pClient, sDB, sSchema, sTable, sPKName, sizeof(sPKName), &SQL))
					{
						//FIXFIX: Handle error.
					}

					//Write Database Name.
					iLen = (int)strlen(sDB);
					fwrite(&iLen, sizeof(iLen), 1, hTarget);
					fwrite(sDB, sizeof(char), iLen, hTarget);

					//Write Schema Name.
					iLen = (int)strlen(sSchema);
					fwrite(&iLen, sizeof(iLen), 1, hTarget);
					fwrite(sSchema, sizeof(char), iLen, hTarget);

					//Write Table Name.
					iLen = (int)strlen(sTable);
					fwrite(&iLen, sizeof(iLen), 1, hTarget);
					fwrite(sTable, sizeof(char), iLen, hTarget);

					//Write PK Name.
					iLen = (int)strlen(sPKName);
					fwrite(&iLen, sizeof(iLen), 1, hTarget);
					fwrite(sPKName, sizeof(char), iLen, hTarget);

					//Write SQL.
					fwrite(&SQL.Length, sizeof(SQL.Length), 1, hTarget);
					fwrite(SQL.Buffer, sizeof(char), SQL.Length, hTarget);			

					if(!pClient->IsConnected())
					{
						rsTables.Close();
						rsDatabases.Close();
						fclose(hTarget);
						SQL.Destroy();
						return false;
					}
				}
				iTables++;
			}

			rsTables.Close();
		}

		if(!pClient->IsConnected())
		{
			rsDatabases.Close();
			fclose(hTarget);
			SQL.Destroy();
			return CMD_ERROR;
		}
	}	

	rsDatabases.Close();
	fclose(hTarget);
	SQL.Destroy();

	if(iTables <= 0)
	{
		WriteLog(pClient->PeerID(),
			"Nothing has been setup for replication.", EVENT_WARN);
		return CMD_DONE;
	}

	return CMD_OK;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#endif
