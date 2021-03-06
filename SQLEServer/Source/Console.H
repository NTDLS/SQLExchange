///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//  Copyright � NetworkDLS 2002, All rights reserved
//
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF 
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO 
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A 
// PARTICULAR PURPOSE.
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef _CONSOLE_H
#define _CONSOLE_H
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//#define _WIN32_WINNT 0x0500

#include <WinCon.h>
#include <ConIO.H>

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#define CONSOLE_CORE_RES_CLOSE    0x1
#define CONSOLE_CORE_RES_ERROR    0x2
#define CONSOLE_CORE_RES_OK       0x3
#define CONSOLE_CORE_RES_UNKNOWN  0x4

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

BOOL WINAPI ConsoleHandlerRoutine(DWORD dwCtrlType);
bool OpenConsole(void);
bool CloseConsole(void);
bool WriteConEx(const char *Buffer, int Length);
int WriteCon(const char *sFormat, ...);
DWORD GetConsoleInput(char *sRetBuffer, int iMaxLength);

DWORD WINAPI Console_Thread(LPVOID);

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

struct _ConIO{
    HANDLE StdIn;
    HANDLE StdOut;
    HANDLE StdErr;
};

extern _ConIO ConIO;

extern HANDLE Console_Thread_Handle;
extern DWORD Console_Thread_ID;

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#endif
