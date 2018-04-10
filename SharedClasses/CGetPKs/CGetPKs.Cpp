///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//  Copyright © NetworkDLS 2002, All rights reserved
//
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF 
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO 
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A 
// PARTICULAR PURPOSE.
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#ifndef _CGETPKS_CPP
#define _CGETPKS_CPP
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include <Windows.H>
#include <WindowsX.H>
#include <ShellAPI.H>
#include <Stdio.H>
#include <Stdlib.H>
#include <SQL.H>
#include <SQLExt.H>

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "CGetPKs.H"

#ifdef _USE_GLOBAL_MEMPOOL
#include "../CMemPool/CMemPool.H"
extern CMemPool *pMem; //gMem must be defined and initalized elsewhere.
#endif

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void *CGetPKs::Alloc(int iSize, int iCount)
{
	#ifdef _USE_GLOBAL_MEMPOOL
		return pMem->Allocate(iSize, iCount);
	#else
		return calloc(iSize, iCount);
	#endif
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CGetPKs::Free(void *pMemory)
{
	#ifdef _USE_GLOBAL_MEMPOOL
		pMem->Free(pMemory);
	#else
		free(pMemory);
	#endif
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool CGetPKs::Get(CSQL *pcSQL, char *sDatabase, char *sSchema, char *sTable)
{
	int iTempSz = 0;

	char *sSQL = NULL;
	char *sBaseSQL = NULL;

	bool bGetPKName = true;
	bool bRetVal = false;

	CBoundRecordSet rsTemp;

	this->Columns.Count = 0;
/*
	sBaseSQL =
		"SELECT"
		" [Objects].[name] AS [ConstraintName],"
		" [Columns].[name] AS [KeyColumn],"
		" [Types].[name] AS [KeyColumnType],"
		" [Columns].[length] AS [KeyColumnLength],"
		" (CASE WHEN [Types].[name] IN ('binary', 'char', 'nchar', 'nvarchar', 'varbinary', 'varchar') THEN 1 ELSE 0 END) AS [TypeStatus]"
		" FROM [%s].[%s].[sysobjects] AS [Objects]"
		" INNER JOIN [%s].[%s].[syscolumns] AS [Columns] ON [Objects].Parent_Obj = [Columns].ID"
		" INNER JOIN [%s].[%s].[systypes] AS [Types] ON [Types].[xtype] = [Columns].[xtype]"
		" INNER JOIN [%s].[%s].[sysindexkeys] AS [IndexKeys] ON [IndexKeys].colid = [Columns].colid"
		" INNER JOIN [%s].[%s].[sysindexes] AS [Indexes] ON [Objects].name = [Indexes].name"
			" AND [Indexes].indid = [IndexKeys].indid"
			" AND [Indexes].id = [IndexKeys].id"
		" WHERE [Objects].[xtype] = 'PK' AND [Objects].[parent_obj] = object_id('[%s].[%s].[%s]')"
		" ORDER BY [IndexKeys].[keyno]";
*/
	sBaseSQL =
		"SELECT"
		"	Keys.CONSTRAINT_NAME COLLATE SQL_Latin1_General_CP1_CI_AS AS [Constraint],"
		"	KeyCols.COLUMN_NAME COLLATE SQL_Latin1_General_CP1_CI_AS AS [Column],"
		"	IsNull(Cols.COLLATION_NAME, '') AS Collation,"
		"	Upper(Cols.DATA_TYPE) COLLATE SQL_Latin1_General_CP1_CI_AS AS [DataType],"
		"	IsNull(Cols.CHARACTER_MAXIMUM_LENGTH, 0) AS CharLength,"
		"	(CASE WHEN Cols.CHARACTER_MAXIMUM_LENGTH IS NULL THEN 0 ELSE 1 END) AS IsChar,"
		"	CASE WHEN Upper(Cols.DATA_TYPE) IN ('DECIMAL', 'NUMERIC') THEN Cols.NUMERIC_PRECISION ELSE 0 END AS [Precision],"
		"	CASE WHEN Upper(Cols.DATA_TYPE) IN ('DECIMAL', 'NUMERIC') THEN Cols.NUMERIC_SCALE ELSE 0 END AS [Scale]"
		" FROM"
		"	[%s].INFORMATION_SCHEMA.TABLE_CONSTRAINTS AS Keys"
		" INNER JOIN [%s].INFORMATION_SCHEMA.KEY_COLUMN_USAGE AS KeyCols"
		"	ON KeyCols.TABLE_CATALOG = Keys.TABLE_CATALOG COLLATE SQL_Latin1_General_CP1_CI_AS"
		"	AND KeyCols.TABLE_SCHEMA = Keys.TABLE_SCHEMA COLLATE SQL_Latin1_General_CP1_CI_AS"
		"	AND KeyCols.TABLE_NAME = Keys.TABLE_NAME COLLATE SQL_Latin1_General_CP1_CI_AS"
		"	AND KeyCols.CONSTRAINT_NAME = Keys.CONSTRAINT_NAME COLLATE SQL_Latin1_General_CP1_CI_AS"
		" INNER JOIN [%s].INFORMATION_SCHEMA.COLUMNS AS Cols"
		"	ON Cols.TABLE_CATALOG = KeyCols.TABLE_CATALOG COLLATE SQL_Latin1_General_CP1_CI_AS"
		"	AND Cols.TABLE_SCHEMA = KeyCols.TABLE_SCHEMA COLLATE SQL_Latin1_General_CP1_CI_AS"
		"	AND Cols.TABLE_NAME = KeyCols.TABLE_NAME COLLATE SQL_Latin1_General_CP1_CI_AS"
		"	AND Cols.COLUMN_NAME = KeyCols.COLUMN_NAME COLLATE SQL_Latin1_General_CP1_CI_AS"
		" WHERE"
		"	Keys.CONSTRAINT_TYPE = 'PRIMARY KEY'"
		"	AND Keys.TABLE_CATALOG = '%s'"
		"	AND Keys.TABLE_SCHEMA = '%s'"
		"	AND Keys.TABLE_NAME = '%s'"
		" ORDER BY"
		"	Keys.TABLE_CATALOG,"
		"	Keys.TABLE_SCHEMA,"
		"	Keys.TABLE_NAME,"
		"	Keys.CONSTRAINT_NAME,"
		"	KeyCols.ORDINAL_POSITION";

	iTempSz = (int)strlen(sBaseSQL) + ((int)strlen(sDatabase) * 4) + (int)strlen(sSchema) + (int)strlen(sTable) + 1;

	if(!(sSQL = (char *) this->Alloc(iTempSz + 1, sizeof(char))))
	{
		Assert(!sSQL, "Memory Allocation Error.");
		return false;
	}

	sprintf_s(sSQL, iTempSz, sBaseSQL, sDatabase, sDatabase, sDatabase, sDatabase, sSchema, sTable);
	bRetVal = pcSQL->Execute(sSQL, &rsTemp);

	this->Free(sSQL);

	if(!bRetVal)
	{
		rsTemp.Close();
		return false;
	}

	strcpy_s(this->Table, sizeof(this->Table), sTable);
    strcpy_s(this->Schema, sizeof(this->Schema), sSchema);
    strcpy_s(this->Database, sizeof(this->Database), sDatabase);

	if(rsTemp.RowCount > 0)
	{
		this->Columns.Array = (CPKCOLUMN *)this->Alloc(sizeof(CPKCOLUMN), (int)rsTemp.RowCount);

		bAlloc = true;

		while(rsTemp.Fetch())
		{
			iTempSz = 0;

			if(bGetPKName)
			{
				//Constraint Name.
				rsTemp.Values("Constraint")->ToString(this->Constraint, sizeof(this->Constraint));
				bGetPKName = false;
			}

			iTempSz = (int)rsTemp.Values("Column")->Size();
			this->Columns.Array[this->Columns.Count].Name = (char *) this->Alloc(sizeof(char), iTempSz + 1);
			Assert(!this->Columns.Array[this->Columns.Count].Name, "Memory Allocation Error.");
			rsTemp.Values("Column")->ToString(this->Columns.Array[this->Columns.Count].Name, iTempSz + 1);

			iTempSz = (int)rsTemp.Values("Collation")->Size();
			this->Columns.Array[this->Columns.Count].Collation = (char *) this->Alloc(sizeof(char), iTempSz + 1);
			Assert(!this->Columns.Array[this->Columns.Count].Collation, "Memory Allocation Error.");
			rsTemp.Values("Collation")->ToString(this->Columns.Array[this->Columns.Count].Collation, iTempSz + 1);

			iTempSz = (int)rsTemp.Values("DataType")->Size();
			this->Columns.Array[this->Columns.Count].DataType = (char *) this->Alloc(sizeof(char), iTempSz + 1);
			Assert(!this->Columns.Array[this->Columns.Count].DataType, "Memory Allocation Error.");
			rsTemp.Values("DataType")->ToString(this->Columns.Array[this->Columns.Count].DataType, iTempSz + 1);

			this->Columns.Array[this->Columns.Count].MaxCharData = rsTemp.Values("CharLength")->ToIntegerS();
			this->Columns.Array[this->Columns.Count].IsCharType = rsTemp.Values("IsChar")->ToBoolean();
			this->Columns.Array[this->Columns.Count].Precision = rsTemp.Values("Precision")->ToIntegerS();
			this->Columns.Array[this->Columns.Count].Scale = rsTemp.Values("Scale")->ToIntegerS();

			this->Columns.Count++;
		}
		bRetVal = true;
	}
	else{
		bRetVal = false;
	}

	rsTemp.Close();

	return bRetVal;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CGetPKs::Free(void)
{
	if(bAlloc)
	{
		bAlloc = false;

		for(int iKey = 0; iKey < this->Columns.Count; iKey++)
		{
			this->Free(this->Columns.Array[iKey].Name);
			this->Free(this->Columns.Array[iKey].Collation);
			this->Free(this->Columns.Array[iKey].DataType);
		}

		this->Free(this->Columns.Array);
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

CGetPKs::CGetPKs()
{
	bAlloc = false;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

CGetPKs::~CGetPKs()
{
	bAlloc = false;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#endif
