///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//  Copyright © NetworkDLS 2002, All rights reserved
//
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF 
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO 
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A 
// PARTICULAR PURPOSE.
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef _FileExchange_H
#define _FileExchange_H
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#define FILEEXCHRESULT_OK          0
#define FILEEXCHRESULT_ERROR      -1
#define FILEEXCHRESULT_CORRUPT    -2
#define FILEEXCHRESULT_DISCONNECT -3

#define FILEBUFFERSZ 4000

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#ifdef _SQLESERVER
	#include "../SQLEServer/Source/Entry.H"
#elif _SQLECLIENT
	#include "../SQLEClient/Source/Entry.H"
#else
	#error "You must define either _SQLESERVER or _SQLECLIENT"
#endif

#include "../../@Libraries/CXML/CXML.H"
#include "../../@Libraries/CCRC32/CCRC32.H"

extern CCRC32 *pCRC32;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int SendFileData(CSocketServer *pSock, CSocketClient *pClient, const char *sFileName, CXMLWriter *lpXML);
int ReceiveFileData(CSocketServer *pSock, CSocketClient *pClient, CXMLReader *lpXML, char *sFileName);

#ifdef _SQLESERVER
	//No equivalent at this time.
#elif _SQLECLIENT
int SimpleTransfer(CSocketServer *pSock, CSocketClient *pClient, const char *sFileName, CXMLWriter *lpXML);
int SimpleTransfer(CSocketServer *pSock, CSocketClient *pClient, const char *sFileName, char *sFileTag);
#else
	#error "You must define either _SQLESERVER or _SQLECLIENT"
#endif


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#endif

