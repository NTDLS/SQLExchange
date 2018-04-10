///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//  Copyright © NetworkDLS 2002, All rights reserved
//
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF 
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO 
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A 
// PARTICULAR PURPOSE.
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef _ENTRY_H
#define _ENTRY_H
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "../../../@Libraries/Debug/Debug.H"
#include "../../../@Libraries/CMemPool/CMemPool.H"
#include "../../../@Libraries/CCmdLine/CCmdLine.H"
#include "../../../@Libraries/NSWFL/NSWFL.H"

#include "../Resources/Resource.H"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#define EXIT_APPLICATION_EVENT "SQL_EXCHANGE_SERVICE_MONITOR_100"

extern HINSTANCE ghAppInstance;

extern char gsAppCopyRight[64];
extern char gsFileVersion[32];
extern char gsTitleCaption[64];
extern char gsRegistryKey[64];

extern char gsServiceControlExe[MAX_PATH];
extern char gsPath[MAX_PATH];

extern CMemPool *pMem;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool RestartClientService(void);
bool MyExecute(char *sFile, char *sParameters);
char *GetBuildArchitecture(const char *sBase, int iMaxSz, char *sOutString);
bool StartService(void);
bool StopService(void);

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#endif

