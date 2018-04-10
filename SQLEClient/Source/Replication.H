///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//  Copyright © NetworkDLS 2002, All rights reserved
//
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF 
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO 
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A 
// PARTICULAR PURPOSE.
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef _CREATETRIGGERS_H
#define _CREATETRIGGERS_H
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "../../SharedClasses/CStatusDlg/CStatusDlg.H"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#define COL_TYPE_TEXT  35
#define COL_TYPE_NTEXT 99
#define COL_TYPE_IMAGE 34

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//------------------------------------------------------------------------------------------
//The primary function for creating a replication.
bool CreateReplication(CSQL *lpCSQL, char *sDB, char *sSchema, char *sTable);
bool CreateReplicationEx(CSQL *lpCSQL, char *sDB, char *sSchema, char *sTable, HWND hGrid, int iCol, int iItem);
//------------------------------------------------------------------------------------------

//------------------------------------------------------------------------------------------
//The primary functions for dropping a replication.
bool DropReplicationTriggers(CSQL *lpCSQL, char *sDB, char *sSchema, char *sTable);
bool DropReplicationTables(CSQL *lpCSQL, char *sDB, char *sSchema, char *sTable);
//------------------------------------------------------------------------------------------

//------------------------------------------------------------------------------------------
bool CreateReplicationDatabaseAndTable(CSQL *lpSQL, CStatusDlg *lpDlg);
bool CreateReplicationTables(CSQL *lpCSQL, char *sDB, char *sSchema, char *sTable);
bool CreateReplicationTriggers(CSQL *lpCSQL, char *sDB, char *sSchema, char *sTable);
bool GenerateReplicationScripts(CSQL *lpSQL, CStatusDlg *lpDlg);
//------------------------------------------------------------------------------------------

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#endif
