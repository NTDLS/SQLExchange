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
#include "../../../@Libraries/CSQLBinding/CSQLEncapsulation.H"
#include "../../../@Libraries/CStringBuilder/CStringBuilder.H"
#include "../../../@Libraries/EventLog/EventLog.H"
#include "../../../@Libraries/CNASCCL (Stream)/CNASCCL.H"
#include "../../../@Libraries/CSocketServer/CSocketServer.H"
#include "../../../@Libraries/CService/CService.H"
#include "../../../@Libraries/CQueu/CQueu.H"

#include "../Resources/Resource.H"

#include "Routines.H"
#include "../../SharedSource/Common.H"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

extern HINSTANCE ghAppInstance;

extern char gsAppCopyRight[64];
extern char gsFileVersion[32];
extern char gsTitleCaption[64];
extern char gsRegistryKey[255];
extern char gsEventName[255];

extern CMemPool *pMem;		//The global memory manager.
extern CService gService;	//The global service.

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#endif

