///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//  Copyright © NetworkDLS 2002, All rights reserved
//
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF 
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO 
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A 
// PARTICULAR PURPOSE.
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef _SQLIMPORT_H
#define _SQLIMPORT_H
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "Routines.H"

#include "../../../@Libraries/CXML/CXML.H"
#include "../../SharedClasses/CGetPKs/CGetPKs.H"
#include "../../../@Libraries/CSocketServer/CSocketServer.H"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#define IMPORT_RES_UNDEFINED	0   //Undefined
#define IMPORT_RES_OK			100 //All is well.
#define IMPORT_RES_ERROR		101 //An error occured.
#define IMPORT_RES_SYNCTABLE	102 //The table structure is out of sync.
#define IMPORT_RES_OFFLINEOK    103 //Finnished processing off-line - All is well.

#define COL_ALLOC_LEVEL_0 0 //Not allocated
#define COL_ALLOC_LEVEL_1 1 //Arrays initialized
#define COL_ALLOC_LEVEL_2 2 //Column info initialized. 
#define COL_ALLOC_LEVEL_3 3 //Column data initialized.

#define TSQL_BIGINT            65531
#define TSQL_BIT               65529
#define TSQL_DECIMAL           3
#define TSQL_FLOAT             6
#define TSQL_INT               4
#define TSQL_MONEY             3
#define TSQL_NUMERIC           2
#define TSQL_REAL              7
#define TSQL_SMALLINT          5
#define TSQL_SMALLMONEY        3
#define TSQL_TINYINT           65530

#define TSQL_BINARY            65534
#define TSQL_IMAGE             65532
#define TSQL_VARBINARY         65533

#define TSQL_NTEXT             65526
#define TSQL_TEXT              65535

#define TSQL_DATETIME          93
#define TSQL_SMALLDATETIME     93
#define TSQL_TIMESTAMP         65534 //Time stamps cannot be replicated.

#define TSQL_CHAR              1
#define TSQL_NCHAR             65528
#define TSQL_NVARCHAR          65527
#define TSQL_SQL_VARIANT       65386
#define TSQL_UNIQUEIDENTIFIER  65525
#define TSQL_VARCHAR           12

typedef struct _TAG_SQLIMPORTCOLUMN {
	//Column Info section.
	char *Name;
    SQLULEN iDataLength;
	bool IsPK;
	bool bComplexType;
	int iDataType;
	SQLLEN iDefferBufSize;

	//Column data section.
	char *Data;
    SQLLEN iDataSz;
	int iFileDataPos; //Starting file position of the actual column data.

} SQLIMPORTCOLUMN, *LPSQLIMPORTCOLUMN;

typedef struct _TAG_SQLIMPORTCOLUMNS {
	SQLIMPORTCOLUMN *Columns;

	//Misc. section.
	unsigned short iAllocationLevel;
	bool bIsPKsAlloc;
	int iColumnCount;
	int *iPKColPos; // Contains the Column position of the uniuqe Columns.

	CGetPKs PKs;

} SQLIMPORTCOLUMNS, *LPSQLIMPORTCOLUMNS;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void FreeColumnRowData(LPSQLIMPORTCOLUMNS lpColumns);
bool AllocColumns(LPSQLIMPORTCOLUMNS lpColumns, CSQL *pcSQL, char *sDB, char *sDBO, char *sTable);
bool DestroyColumns(LPSQLIMPORTCOLUMNS lpColumns);
int ImportSQLResults(CSocketServer *pSock, CSocketClient *pClient, _ConnectedClientInfo *lpCCI, char *InFileName);
int ImportSQLResultsEx(CSocketServer *pSock, CSocketClient *pClient, _ConnectedClientInfo *lpCCI,
					   LPSQLIMPORTCOLUMNS lpColumns, char *sSQLBuf, int iSQLBufSz,
					   CStringBuilder *lpSrvSQL, CStringBuilder *lpWhereSQL, FILE *hSource,
					   int *iOutID, char *InFileName);

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#endif
