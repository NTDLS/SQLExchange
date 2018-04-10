///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//  Copyright © NetworkDLS 2002, All rights reserved
//
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF 
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO 
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A 
// PARTICULAR PURPOSE.
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef _CREATETRIGGERS_CPP
#define _CREATETRIGGERS_CPP
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include <Windows.H>
#include <WindowsX.H>
#include <ShellAPI.H>
#include <Stdio.H>
#include <Stdlib.H>

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "Entry.H"
#include "Console.H"
#include "Replication.H"

#include "../../SharedClasses/CGetPKs/CGetPKs.H"

#include "../Dialogs/ReplicationDlg.H"
#include "../Dialogs/MainDlg.H"

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool DropReplicationTriggers(CSQL *lpCSQL, char *sDB, char *sSchema, char *sTable)
{
	//WriteCon("DropReplicationTriggers\n");

	char sSQL[1024];

	bool bThrowErrors = lpCSQL->ThrowErrors(true);

	CBoundRecordSet rsTriggers;

	sprintf_s(sSQL, sizeof(sSQL),
		"SELECT"
		"	TR.[name] AS [TriggerName]"
		" FROM"
		"	[%s].[%s].sysobjects AS TR"
		" WHERE"
		"	TR.parent_obj = OBJECT_ID('%s.%s.%s')"
		"	AND TR.xtype = 'TR'"
		"	AND TR.name LIKE 'SQLExch[_]%%'",
		sDB, gsDefaultDBO, sDB, sSchema, sTable);
	lpCSQL->Execute(sSQL, &rsTriggers);

	while(rsTriggers.Fetch())
	{
		sprintf_s(sSQL, sizeof(sSQL), "DROP TRIGGER [%s].[%s]", sSchema, rsTriggers.Values("TriggerName")->ToString());
		lpCSQL->Execute(sSQL);
	}

	rsTriggers.Close();

	lpCSQL->ThrowErrors(bThrowErrors);

	return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//This function is complete!
bool DropReplicationTables(CSQL *lpCSQL, char *sDB, char *sSchema, char *sTable)
{
	//WriteCon("DropReplicationTables\n");

	char sSQL[1024];
	bool bThrowErrors = lpCSQL->ThrowErrors(false);

	sprintf_s(sSQL, sizeof(sSQL), "DROP TABLE [%s].[%s].[%s_%s]", gsReplicationDB, sSchema, sDB, sTable);
	lpCSQL->Execute(sSQL);

	sprintf_s(sSQL, sizeof(sSQL), "DELETE FROM [%s].[%s].[SQLExch_Transactions]"
		" WHERE [Database] = '%s' AND [Schema] = '%s' AND [Table] = '%s'",
		gsReplicationDB, gsDefaultDBO, sDB, sSchema, sTable);
	lpCSQL->Execute(sSQL);

	lpCSQL->ThrowErrors(bThrowErrors);

	return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//This function is complete!
bool CreateReplicationTables(CSQL *lpCSQL, char *sDB, char *sSchema, char *sTable)
{
	//WriteCon("CreateReplicationTables::");
	//WriteCon(sTable);
	//WriteCon("\n");

	char sSmlTmp[64];
	char sLargeSQL[10240];

	bool bThrowErrors = lpCSQL->ThrowErrors(false);
	
	CGetPKs cGetPKs;

	lpCSQL->Focus(gsReplicationDB);
	sprintf_s(sLargeSQL, sizeof(sLargeSQL), "CREATE SCHEMA [%s]", sSchema);

	lpCSQL->Execute(sLargeSQL);
	lpCSQL->ThrowErrors(bThrowErrors);

	lpCSQL->Focus(sDB);

	if(cGetPKs.Get(lpCSQL, sDB, sSchema, sTable))
	{
		int iKey = 0;

		sprintf_s(sLargeSQL, sizeof(sLargeSQL),
			"CREATE TABLE [%s].[%s].[%s_%s] (\r\n",
			gsReplicationDB, sSchema, sDB, sTable);

		while(iKey < cGetPKs.Columns.Count)
		{
			strcat_s(sLargeSQL, sizeof(sLargeSQL), "\t[");
			strcat_s(sLargeSQL, sizeof(sLargeSQL), cGetPKs.Columns.Array[iKey].Name);
			strcat_s(sLargeSQL, sizeof(sLargeSQL), "] ");

			strcat_s(sLargeSQL, sizeof(sLargeSQL), cGetPKs.Columns.Array[iKey].DataType);

			if(cGetPKs.Columns.Array[iKey].IsCharType && cGetPKs.Columns.Array[iKey].MaxCharData > 0)
			{
				sprintf_s(sSmlTmp, sizeof(sSmlTmp), " (%d)", cGetPKs.Columns.Array[iKey].MaxCharData);
				strcat_s(sLargeSQL, sizeof(sLargeSQL), sSmlTmp);
			}
			else{
				if(cGetPKs.Columns.Array[iKey].Precision > 0 || cGetPKs.Columns.Array[iKey].Scale > 0)
				{
					sprintf_s(sSmlTmp, sizeof(sSmlTmp), " (%d, %d)",
						cGetPKs.Columns.Array[iKey].Precision,
						cGetPKs.Columns.Array[iKey].Scale);
					strcat_s(sLargeSQL, sizeof(sLargeSQL), sSmlTmp);
				}
			}

			if(strlen(cGetPKs.Columns.Array[iKey].Collation) > 0)
			{
				strcat_s(sLargeSQL, sizeof(sLargeSQL), " COLLATE ");
				strcat_s(sLargeSQL, sizeof(sLargeSQL), cGetPKs.Columns.Array[iKey].Collation);
			}

			strcat_s(sLargeSQL, sizeof(sLargeSQL), " NULL,\r\n");

			iKey++;
		}

		//-------------------------------------------------------------
		//   ** THIS IS AN OLD COMMAND AND HAS NOT BEEN REVIEWED **
		//-------------------------------------------------------------
		//Need to add a SQLExch_Locked Bit column to each of the new tables.
		//	We use this column because of the following scenario:
		//		1) When a transfer begins we need to update SQLExch_Locked = 1.
		//		2) Next we need to select all of the rows where SQLExch_Locked = 1.
		//		3) While the Server and client exchange data, and do imports and exports,
		//			the data may have changed, adding more rows to the table with a
		//			SQLExch_Locked flag equal to NULL
		//		4) If the import was a success then we delete from the table where SQLExch_Locked = 1
		//		5) The rows that were added durring the server/client conversation are
		//			preserved for the next transfer
		//-------------------------------------------------------------
		strcat_s(sLargeSQL, sizeof(sLargeSQL), "\t[SQLExch_Action] SmallInt NOT NULL,\r\n"); //0=Insert,Update | 1=Delete
		strcat_s(sLargeSQL, sizeof(sLargeSQL), "\t[SQLExch_Locked] Bit\r\n");
		strcat_s(sLargeSQL, sizeof(sLargeSQL), ") ON [PRIMARY]\r\n");

		lpCSQL->Execute(sLargeSQL);

		cGetPKs.Free();
	}

	return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//This function is complete!
bool CreateReplicationTriggers(CSQL *lpCSQL, char *sDB, char *sSchema, char *sTable)
{
	//WriteCon("CreateReplicationTriggers\n");

	char sPKList[10240];
	char sPKJoin[10240];
	char sTrigger[10240];
	char sIfUpdate[10240];

	CGetPKs cGetPKs;

	if(cGetPKs.Get(lpCSQL, sDB, sSchema, sTable))
	{
		int iKey = 0;

		strcpy_s(sPKList, sizeof(sPKList), "");
		strcpy_s(sPKJoin, sizeof(sPKJoin), "");
		strcpy_s(sIfUpdate, sizeof(sIfUpdate), "");

		while(iKey < cGetPKs.Columns.Count)
		{
			strcat_s(sPKList, sizeof(sPKList), "[");
			strcat_s(sPKList, sizeof(sPKList), cGetPKs.Columns.Array[iKey].Name);
			strcat_s(sPKList, sizeof(sPKList), "]");

			strcat_s(sIfUpdate, sizeof(sIfUpdate), "UPDATE([");
			strcat_s(sIfUpdate, sizeof(sIfUpdate), cGetPKs.Columns.Array[iKey].Name);
			strcat_s(sIfUpdate, sizeof(sIfUpdate), "])");

			strcat_s(sPKJoin, sizeof(sPKJoin), "[A].[");
			strcat_s(sPKJoin, sizeof(sPKJoin), cGetPKs.Columns.Array[iKey].Name);
			strcat_s(sPKJoin, sizeof(sPKJoin), "] = ");
			strcat_s(sPKJoin, sizeof(sPKJoin), "[B].[");
			strcat_s(sPKJoin, sizeof(sPKJoin), cGetPKs.Columns.Array[iKey].Name);
			strcat_s(sPKJoin, sizeof(sPKJoin), "]");

			if(iKey != (cGetPKs.Columns.Count - 1))
			{
				strcat_s(sPKList, sizeof(sPKList), ", ");
				strcat_s(sPKJoin, sizeof(sPKJoin), " AND ");
				strcat_s(sIfUpdate, sizeof(sIfUpdate), " OR ");
			}

			iKey++;
		}

		cGetPKs.Free();

		sprintf_s(sTrigger, sizeof(sTrigger),
			"CREATE TRIGGER [%s].[SQLExch_%s_%s_%s_U] ON [%s].[%s]\n"
			"AFTER INSERT, UPDATE AS\n"
			"BEGIN\n"

			"	SET NOCOUNT ON;\n\n"

			"	DELETE A FROM [%s].[%s].[%s_%s] AS A\n"
			"	INNER JOIN Inserted AS B ON %s\n"
			"	WHERE A.[SQLExch_Locked] IS NULL\n\n"

			"	IF(%s)\n"
			"	BEGIN\n"
			"		DELETE A FROM [%s].[%s].[%s_%s] AS A\n"
			"		INNER JOIN Deleted AS B ON %s\n"
			"		WHERE A.[SQLExch_Locked] IS NULL\n\n"

			"		INSERT INTO [%s].[%s].[%s_%s]\n"
			"		(%s, [SQLExch_Action])\n"
			"		SELECT %s, '1' FROM [deleted]\n"
			"	END\n\n"

			"	INSERT INTO [%s].[%s].[%s_%s]\n"
			"	(%s, [SQLExch_Action])\n"
			"	SELECT %s, '0' FROM [inserted]\n\n"

			"	INSERT INTO [%s].[%s].[SQLExch_Transactions]\n"
			"	([Database], [Schema], [Table])\n"
			"	SELECT TOP 1 '%s', '%s', '%s' FROM [inserted]\n"
			"END\n",

			sSchema, sDB, sSchema, sTable, sSchema, sTable,
			gsReplicationDB, sSchema, sDB, sTable, sPKJoin,
			sIfUpdate,
			gsReplicationDB, sSchema, sDB, sTable, sPKJoin,
			gsReplicationDB, sSchema, sDB, sTable, sPKList, sPKList,

			gsReplicationDB, sSchema, sDB, sTable, sPKList, sPKList,
			gsReplicationDB, gsDefaultDBO, sDB, sSchema, sTable
		);
		lpCSQL->Execute(sTrigger);

		sprintf_s(sTrigger, sizeof(sTrigger),
			"CREATE TRIGGER [%s].[SQLExch_%s_%s_%s_D] ON [%s].[%s]\n"
			"AFTER DELETE AS\n"
			"BEGIN\n"
			"	SET NOCOUNT ON;\n\n"
			
			"	DELETE A FROM [%s].[%s].[%s_%s] AS A\n"
			"	INNER JOIN Deleted AS B ON %s\n"
			"	WHERE A.[SQLExch_Locked] IS NULL\n\n"

			"	INSERT INTO [%s].[%s].[%s_%s]\n"
			"	(%s, [SQLExch_Action])\n"
			"	SELECT %s, '1' FROM [deleted]\n\n"

			"	INSERT INTO [%s].[%s].[SQLExch_Transactions]\n"
			"	([Database], [Schema], [Table])\n"
			"	SELECT TOP 1 '%s', '%s', '%s' FROM [deleted]\n"
			"END\n",

			sSchema, sDB, sSchema, sTable, sSchema, sTable,
			gsReplicationDB, sSchema, sDB, sTable, sPKJoin,
			gsReplicationDB, sSchema, sDB, sTable, sPKList, sPKList,
			gsReplicationDB, gsDefaultDBO, sDB, sSchema, sTable
		);
		lpCSQL->Execute(sTrigger);
	}

	return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//This function is complete!
bool CreateReplicationEx(CSQL *lpCSQL, char *sDB, char *sSchema, char *sTable, HWND hGrid, int iCol, int iItem)
{
	//WriteCon("CreateReplication\n");

	bool bThrowErrors = lpCSQL->ThrowErrors(false);
	SetStatus(hGrid, iCol, iItem, "Dropping Triggers.");
	DropReplicationTriggers(lpCSQL, sDB, sSchema, sTable);

	SetStatus(hGrid, iCol, iItem, "Dropping Tables.");
	DropReplicationTables(lpCSQL, sDB, sSchema, sTable);
	lpCSQL->ThrowErrors(bThrowErrors);

	SetStatus(hGrid, iCol, iItem, "Creating Tables.");
	if(CreateReplicationTables(lpCSQL, sDB, sSchema, sTable))
	{
		SetStatus(hGrid, iCol, iItem, "Creating Triggers.");
		return CreateReplicationTriggers(lpCSQL, sDB, sSchema, sTable);
	}

	return false;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//This function is complete!
bool CreateReplication(CSQL *lpCSQL, char *sDB, char *sSchema, char *sTable)
{
	return CreateReplicationEx(lpCSQL, sDB, sSchema, sTable, NULL, REP_COLUMN_STATUS, 0);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool CreateReplicationDatabaseAndTable(CSQL *lpSQL, CStatusDlg *lpDlg)
{
	char sSQL[5120];

	bool bDatabaseExists = false;

	CBoundRecordSet rsTemp;

	if(lpDlg)
	{
		lpDlg->SetText("Generating scripts required for replication.");
		lpDlg->SetProgressPos(0);
	}

	//------------------------------------------------------------------------------------------------------

	sprintf_s(sSQL, sizeof(sSQL), "SELECT IsNull(db_id('%s'), 0) AS [Exists]", gsReplicationDB);
	if(!lpSQL->Execute(sSQL, &rsTemp))
	{
		MessageBox(NULL,
			"Failed to check replicaton database existence.",
			gsTitleCaption, MB_TASKMODAL|MB_ICONERROR);
		return false;
	}

	if(rsTemp.Fetch())
	{
		bDatabaseExists = (rsTemp.Values("Exists")->ToShortS() > 0);
	}

	rsTemp.Close();

	//------------------------------------------------------------------------------------------------------
	if(!bDatabaseExists)
	{
		sprintf_s(sSQL, sizeof(sSQL),
			" CREATE DATABASE [%s]"
			" ON("
			"	NAME = 'SQLExch_%s_Dat',"
			"	FILENAME = '%s\\%s_Dat.mdf',"
			"	SIZE = 10,"
			"	FILEGROWTH = 5"
			" )"
			" LOG ON("
			"	NAME = 'SQLExch_%s_Log',"
			"	FILENAME = '%s\\%s_Log.ldf',"
			"	SIZE = 5MB,"
			"	FILEGROWTH = 5MB"
			" )",
			gsReplicationDB, gsReplicationDB, gsSQLDataFiles, gsReplicationDB, gsReplicationDB, gsSQLLogFiles, gsReplicationDB);

		if(!lpSQL->Execute(sSQL))
		{
			MessageBox(NULL,
				"Failed to create the replicaton database, all existing replications should be removed.",
				gsTitleCaption, MB_TASKMODAL|MB_ICONERROR);
			return false;
		}

		//------------------------------------------------------------------------------------------------------

		sprintf_s(sSQL, sizeof(sSQL), "ALTER DATABASE [%s] SET RECOVERY SIMPLE", gsReplicationDB);
		if(!lpSQL->Execute(sSQL))
		{
			MessageBox(NULL,
				"Failed to set the replication database recovery model, all existing replications should be removed.",
				gsTitleCaption, MB_TASKMODAL|MB_ICONERROR);
			return false;
		}
	}

	//------------------------------------------------------------------------------------------------------
	//Drop the SQLExch_Replications table.
	sprintf_s(sSQL, sizeof(sSQL), "DROP TABLE [%s].[%s].[SQLExch_Replications]", gsReplicationDB, gsDefaultDBO);

	bool bThrowErrors = lpSQL->ThrowErrors(false);
	lpSQL->Execute(sSQL); 
	lpSQL->ThrowErrors(bThrowErrors);

	//Recreate SQLExch_Replications table.
	sprintf_s(sSQL, sizeof(sSQL),
		"CREATE TABLE [%s].[%s].[SQLExch_Replications]"
		"("
		"	[ID] INT IDENTITY (1,1) NOT NULL,"
		"	[Database] VarChar (255) NULL,"
		"	[Schema] VarChar (255) NULL,"
		"	[Table] VarChar (255) NULL,"
		"	CONSTRAINT PK_SQLExch_Replications PRIMARY KEY CLUSTERED"
		"	("
		"		[ID]"
		"	)  ON [PRIMARY]"
		") ON [PRIMARY]",
		gsReplicationDB, gsDefaultDBO);

	if(!lpSQL->Execute(sSQL))
	{
		MessageBox(NULL,
			"Failed to create the replicaton table.",
			gsTitleCaption, MB_TASKMODAL|MB_ICONERROR);
		return false;
	}

	//------------------------------------------------------------------------------------------------------

	if(!lpSQL->Focus(gsReplicationDB))
	{
		MessageBox(NULL,
			"Failed to set replicaton table focus.",
			gsTitleCaption, MB_TASKMODAL|MB_ICONERROR);
		return false;
	}

	//------------------------------------------------------------------------------------------------------

	//Recreate SQLExch_Replications covering index.
	sprintf_s(sSQL, sizeof(sSQL),
		"CREATE NONCLUSTERED INDEX [IDX_Covering] ON [%s].[SQLExch_Replications] "
		"("
		"	[ID] ASC,"
		"	[Database] ASC,"
		"	[Schema] ASC,"
		"	[Table] ASC"
		")",
		gsDefaultDBO);

	if(!lpSQL->Execute(sSQL))
	{
		MessageBox(NULL,
			"Failed to create the replicaton table.",
			gsTitleCaption, MB_TASKMODAL|MB_ICONERROR);
		return false;
	}

	//------------------------------------------------------------------------------------------------------

	//We don't ever want to drop the [SQLExch_Transactions] table.
	//Create SQLExch_Transactions table, if it doesn't exists.
	sprintf_s(sSQL, sizeof(sSQL),
		"IF NOT EXISTS (SELECT TOP 1 1 FROM [%s].[%s].sysobjects WHERE xtype = 'U' AND name = 'SQLExch_Transactions')"
		" BEGIN"
		"	CREATE TABLE [%s].[%s].[SQLExch_Transactions]"
		"	("
		"		[Database] varchar(255) NULL,"
		"		[Schema] varchar(255) NULL,"
		"		[Table] varchar(255) NULL,"
		"		[Locked] bit NULL"
		"	) ON [PRIMARY]"
		" END",
		gsReplicationDB, gsDefaultDBO, 
		gsReplicationDB, gsDefaultDBO);

	if(!lpSQL->Execute(sSQL))
	{
		MessageBox(NULL,
			"Failed to create the replicaton transaction table.",
			gsTitleCaption, MB_TASKMODAL|MB_ICONERROR);
		return false;
	}

	//------------------------------------------------------------------------------------------------------

	return true;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool GenerateReplicationScripts(CSQL *lpSQL, CStatusDlg *lpDlg)
{
	char sSQL[5120];
	char sText[1024];

	char sDatabase[256];
	char sSchema[256];
	char sTable[256];

	if(lpDlg)
	{
		lpDlg->SetText("Generating scripts required for replication.");
		lpDlg->SetProgressPos(0);
	}

	//------------------------------------------------------------------------------------------------------

	CBoundRecordSet rsDatabases;
	CBoundRecordSet rsTables;

	sprintf_s(sSQL, sizeof(sSQL),
		"SELECT"
		"	[name] AS DatabaseName"
		" FROM"
		"	[master].[%s].[sysdatabases]"
		" WHERE"
		"	[name] NOT LIKE 'SQLExch_%%'"
		"	AND [name] NOT IN ('master', 'model', 'msdb', 'tempdb')"
		" ORDER BY"
		"	[name]",
		gsDefaultDBO);
	if(lpSQL->Execute(sSQL, &rsDatabases))
	{
		while(rsDatabases.Fetch())
		{
			rsDatabases.Values("DatabaseName")->ToString(sDatabase, sizeof(sDatabase));

			//Create a record set containing all of the table names that contain SQLExch_ triggers
			sprintf_s(sSQL, sizeof(sSQL),
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
				"	T.TABLE_NAME",
				sDatabase, sDatabase, gsDefaultDBO);
			if(lpSQL->Execute(sSQL, &rsTables))
			{
				if(rsTables.RowCount > 0)
				{
					int iTables = 1;

					if(lpDlg)
					{
						sprintf_s(sText, sizeof(sText), "Creating replications. [%s]", sDatabase);
						lpDlg->SetText(sText);
						lpDlg->SetProgressRange(0, (int)rsTables.RowCount);
						lpDlg->SetProgressPos(0);
						Sleep(1000);
					}

					//Loop through all of the tables that have triggers.
					while(rsTables.Fetch())
					{
						rsTables.Values("Schema")->ToString(sSchema, sizeof(sSchema));
						rsTables.Values("Table")->ToString(sTable, sizeof(sTable));

						sprintf_s(sSQL, sizeof(sSQL),
							"INSERT INTO [%s].[%s].SQLExch_Replications ([Database], [Schema], [Table])"
							" VALUES('%s', '%s', '%s')",
							gsReplicationDB, gsDefaultDBO, sDatabase, sSchema, sTable);
						if(!lpSQL->Execute(sSQL))
						{
							MessageBox(NULL,
								"Failed to create insert replication.",
								gsTitleCaption, MB_TASKMODAL|MB_ICONERROR);

							rsTables.Close();
							rsDatabases.Close();

							lpSQL->Disconnect();
							return false;
						}

						if(lpDlg)
						{
							lpDlg->SetProgressPos(iTables++);
						}
					}
				}
			}
			rsTables.Close();
		}
		rsDatabases.Close();
	}
	else{
	}

	//------------------------------------------------------------------------------------------------------
	
/*
	char sSQL[10240];
	int iTempSz = 0;
	int iLen = 0;
	long lLoop = 0;

	FILE *hSource = NULL;

	char sWhereClause[10240];
	char sLargeSQL[10240];
	char sPKList[5120];
	char sTemp[1024];

	char sDatabase[256];
	char sSchema[256];
	char sTable[256];

	CGetPKs cGetPKs;

	CSQL cSQL;
	CBoundRecordSet rsTables;
	CBoundRecordSet rsDatabases;

	if(!bCreate)
	{
		cSQL.Disconnect();
		return true;
	}

	strcpy_s(sSQL, sizeof(sSQL),
		"SELECT"
		"	[name] AS DatabaseName"
		" FROM"
		"	[sysdatabases]"
		" WHERE"
		"	[name] NOT LIKE 'SQLExch_%'"
		"	AND [name] NOT IN ('master', 'model', 'msdb', 'tempdb')"
		" ORDER BY"
		"	[name]");
	cSQL.Execute(sSQL, &rsDatabases);	

	while(rsDatabases.Fetch())
	{
		rsDatabases.Values("DatabaseName")->ToString(sDB, sizeof(sDB));

		//----------------------------------------------------------------------------------------------------

		//Create a record set containing all of the table names that contain SQLExch_ triggers
		sprintf_s(sSQL, sizeof(sSQL),
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
			"	T.TABLE_NAME",
			sDB, sDB, gsDefaultDBO);
		cSQL.Execute(sSQL, &rsTables);

		if(rsTables.RowCount > 0)
		{
			if(lpDlg)
			{
				sprintf_s(sTemp, sizeof(sTemp), "Creating replications. [%s]", sDB);
				lpDlg->SetText(sTemp);
				lpDlg->SetProgressRange(0, rsTables.RowCount);
				lpDlg->SetProgressPos(0);
				lLoop = 0;
				Sleep(1000);
			}

			//Loop through all of the tables that have triggers.
			while(rsTables.Fetch())
			{
				//Get the table name, save it in sTable[]
				rsTables.Values("Table")->ToString(sTable, sizeof(sTable));
				rsTables.Values("Schema")->ToString(sSchema, sizeof(sSchema));

				sprintf_s(sRepTable, sizeof(sRepTable), "SQLExch_%s_%s_Trans", sDB, sTable);

				//Get a list of primary keys for the table.
				if(cGetPKs.Get(&gMem, &cSQL, sDB, sSchema, sTable))
				{
					int iTrgTable = 0;

					//----------------------------------------------------------------------------------------
					//Insert the first statement into the "SQLExch_Replications" table.
					//----------------------------------------------------------------------------------------

					sprintf_s(sSQL, sizeof(sSQL),
						"INSERT INTO [%s].[%s].SQLExch_Replications ([Database], [Shema], [Table])"
						" VALUES('%s', '%s', '%s')",
						sDB, sSchema, sRepTable);
					cSQL.Execute(sSQL);
					lSeq = (lSeq + 10);

					//----------------------------------------------------------------------------------------

					int iKey = 0;

					strcpy_s(sWhereClause, sizeof(sWhereClause), "WHERE");

					strcpy_s(sPKList, sizeof(sPKList), "");

					//Loop through all of the primary keys
					while(iKey < cGetPKs.ColumnCount)
					{
						strcat_s(sWhereClause, sizeof(sWhereClause), " [");
						strcat_s(sWhereClause, sizeof(sWhereClause), sTable);
						strcat_s(sWhereClause, sizeof(sWhereClause), "].");

						strcat_s(sWhereClause, sizeof(sWhereClause), "[");
						strcat_s(sWhereClause, sizeof(sWhereClause), cGetPKs.Columns[iKey]);
						strcat_s(sWhereClause, sizeof(sWhereClause), "] = ");

						strcat_s(sWhereClause, sizeof(sWhereClause), "[SQLExch_Trans].");

						strcat_s(sWhereClause, sizeof(sWhereClause), "[");
						strcat_s(sWhereClause, sizeof(sWhereClause), cGetPKs.Columns[iKey]);
						strcat_s(sWhereClause, sizeof(sWhereClause), "] AND");

						strcat_s(sPKList, sizeof(sPKList), "[");
						strcat_s(sPKList, sizeof(sPKList), cGetPKs.Columns[iKey]);
						strcat_s(sPKList, sizeof(sPKList), "]");
						if(iKey != cGetPKs.ColumnCount - 1)
						{
							strcat_s(sPKList, sizeof(sPKList), ", ");
						}

						iKey++;
					}

					//----------------------------------------------------------------------------------------
					//Write the 'Updated / Inserted Records' statement.
					//----------------------------------------------------------------------------------------

					//Default OnFailure SQL.
					sprintf_s(sOnFailure, sizeof(sOnFailure),
						"UPDATE [%s].[%s].[SQLExch_Trans]"
						" SET [SQLExch_Pending] = 0"
						" WHERE [SQLExch_Pending] = 1 AND [TransTable] = ''%s''",
						gsReplicationDB, gsDefaultDBO, sTable);

					//Default sOnSuccess SQL.
					sprintf_s(sOnSuccess, sizeof(sOnSuccess),
						"DELETE FROM [%s].[%s].[SQLExch_%s_%s_Trans]"
						" WHERE [SQLExch_Pending] = 1 AND [SQLExch_Action] = 1",
						gsReplicationDB, gsDefaultDBO, sDB, sTable);

					//We need to be able to select DISTINCT, but the ntext, text, image types dont like it.
					//sprintf_s(sLargeSQL, "SELECT DISTINCT %s.*"
					sprintf_s(sLargeSQL, sizeof(sLargeSQL),
						"SELECT [%s].*"
						" FROM [%s].[%s].[%s] AS [%s], [%s].[%s].[SQLExch_Temp] AS [SQLExch_Trans]"
						" %s [SQLExch_Trans].[SQLExch_Pending] = 1 AND"
						" [SQLExch_Trans].[SQLExch_Action] = 1",
						sTable, sDB, gsDefaultDBO, sTable, sTable, gsReplicationDB, gsDefaultDBO, sWhereClause);

					sprintf_s(sSQL, sizeof(sSQL),
						"INSERT INTO [%s].[%s].SQLExch_Replications ([Statement], [OnSuccess],"
						" [OnFailure], [ImportTable], [Sequence], [Active], [TransTable], [TransDB], [TransSchema],"
						" [RepTable], [PrimarySelect], [Comments])"
						" VALUES('%s', '%s', '%s', '%s', '%d', '1', '%s', '%s', '%s', '%s', 0, 'Auto Generated.')",
						gsReplicationDB, gsDefaultDBO, sLargeSQL, sOnSuccess, sOnFailure, sTable, lSeq, sTable, sDB, sSchema, sRepTable);
					cSQL.Execute(sSQL);
					lSeq = (lSeq + 10);

					//----------------------------------------------------------------------------------------
					//Write the 'Deleted Records' statement.
					//----------------------------------------------------------------------------------------

					//No default OnFailure SQL.
					strcpy_s(sOnFailure, sizeof(sOnFailure), "");

					//Default sOnSuccess SQL.
					sprintf_s(sOnSuccess, sizeof(sOnSuccess),
						"DELETE FROM [%s].[%s].[SQLExch_%s_%s_Trans]"
						" WHERE [SQLExch_Pending] = 1 AND [SQLExch_Action] = 2 AND [SQLExch_Action] = 2",
						gsReplicationDB, gsDefaultDBO, sDB, sTable);

					sprintf_s(sLargeSQL, sizeof(sLargeSQL),
						"SELECT DISTINCT %s"
						" FROM [%s].[%s].[SQLExch_%s_%s_Trans]"
						" WHERE [SQLExch_Pending] = 1 AND [SQLExch_Action] = 2",
						sPKList, gsReplicationDB, gsDefaultDBO, sDB, sTable);

					sprintf_s(sSQL, sizeof(sSQL),
						"INSERT INTO [%s].[%s].SQLExch_Replications ([Statement], [OnSuccess],"
						" [ImportTable], [Sequence], [Active], [TransTable], [TransDB], [TransSchema], [RepTable],"
						" [PrimarySelect], [Comments])"
						" VALUES('%s', '%s', '%s_SQLExch_Delete', '%d', '1',"
						" '%s', '%s', '%s', '%s', 1, 'Auto Generated.')",
						gsReplicationDB, gsDefaultDBO, sLargeSQL, sOnSuccess, sTable, lSeq, sTable, sDB, sSchema, sRepTable);
					cSQL.Execute(sSQL);
					lSeq = (lSeq + 10);

					cGetPKs.Free(&gMem);
				}

				if(lpDlg)
				{
					lLoop++;
					lpDlg->SetProgressPos(lLoop);
				}
			}
		}

		rsTables.Close();
	}

	rsDatabases.Close();

	cSQL.Disconnect();
*/
	return true;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#endif
