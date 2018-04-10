///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//  Copyright © NetworkDLS 2002, All rights reserved
//
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF 
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO 
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A 
// PARTICULAR PURPOSE.
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef _NEW_DB_H
#define _NEW_DB_H
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "../../../@Libraries/CSocketServer/CSocketServer.H"

#include "../../SharedClasses/CStatusDlg/CStatusDlg.H"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool CreateNewDB(CSQL *lpMySQL, char *sDBName);
bool SetupNewClient(char *sClientDBFileName, char *sDBPrefix, char *sDBO, bool bImportStructure,
					bool bImportData, bool bDropImportTable, CStatusDlg *lpDlg);

bool RestoreDBForImport(CSQL *lpMySQL, char *sClientDBFileName, char *sDBName);
bool ImportDBStructure(CSQL *lpMySQL, char *sDBO, char *sDBName, bool bImportData, CStatusDlg *lpDlg);
bool CreateNewDBFromDBCreationScript(CSQL *lpMySQL, CSocketServer *pSock, CSocketClient *pClient, char *sFileName);
bool CreateDatabaseIfNotExist(CSQL *lpMySQL, CSocketServer *pSock, CSocketClient *pClient, const char *sDB);
bool GetDBNameFromBackup(CSQL *lpMySQL, const char *sBackupFile, char *sOutName, int iMaxSz);

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#endif
