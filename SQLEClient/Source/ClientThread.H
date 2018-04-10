///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//  Copyright © NetworkDLS 2002, All rights reserved
//
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF 
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO 
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A 
// PARTICULAR PURPOSE.
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#ifndef _ClientThread_H
#define _ClientThread_H
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "Entry.H"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Sock_ClientThread(CSocketServer *pSock, CSocketClient *pClient, LPBASICHUNK pChunk);
void Sock_OnSockError(CSocketServer *pSock, CSocketClient *pClient, int iErrorNumber, const char *sErrorMsg);
void Sock_OnBeginClientThread(CSocketServer *pSock, CSocketClient *pClient, HANDLE hThread);
void Sock_OnEndClientThread(CSocketServer *pSock, CSocketClient *pClient);
bool Sock_OnConnect(CSocketServer *pSock, CSocketClient *pClient);
void Sock_OnDisconnect(CSocketServer *pSock, CSocketClient *pClient);
bool Sock_OnBeforePacketize(CSocketServer *pSock, CSocketClient *pClient, LPBASICHUNK pChunk);
bool Sock_OnAfterDePacketize(CSocketServer *pSock, CSocketClient *pClient, LPBASICHUNK pChunk);

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#endif

