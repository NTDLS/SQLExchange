///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//  Copyright © NetworkDLS 2002, All rights reserved
//
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF 
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO 
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A 
// PARTICULAR PURPOSE.
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef _SQLEXPORT_H
#define _SQLEXPORT_H
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#define MAX_SQL_DATA_SZ            (512 * 1024)

#define SQL_EXPORT_RESULT_OK       1
#define SQL_EXPORT_RESULT_ERROR    2
#define SQL_EXPORT_RESULT_ZEROROWS 3

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

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void WriteNull(FILE *fTarget);
int ExportSQLResults(CSocketServer *pSock, CSocketClient *pClient, int iID, char *Statement,
					 char *sDB, char *sDBO, char *sImportTable, int iOperation, char *OutFileName);
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#endif
