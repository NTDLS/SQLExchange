///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//  Copyright © NetworkDLS 2002, All rights reserved
//
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF 
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO 
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A 
// PARTICULAR PURPOSE.
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef _CONSOLE_CPP
#define _CONSOLE_CPP
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#define _WIN32_WINNT 0x0500

#include <Windows.H>
#include <Stdio.H>
#include <Stdlib.H>
#include <WinCon.h>
#include <ConIO.H>
#include <SQL.H>

#include "Entry.H"
#include "Console.H"

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

_ConIO ConIO;
bool gbConsoleOpen = false;

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

BOOL WINAPI ConsoleHandlerRoutine(DWORD dwCtrlType)
{
    if(dwCtrlType == CTRL_CLOSE_EVENT)
    {
		return true;
    }

    return FALSE;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool OpenConsole(void)
{
    if(gbConsoleOpen)
        return false;

    if(AllocConsole())
    {
        gbConsoleOpen = true;

        SetConsoleCtrlHandler(ConsoleHandlerRoutine, TRUE);

        char sConTitle[255];
        sprintf_s(sConTitle, sizeof(sConTitle), "%s [Version: %s]", gsTitleCaption, gsFileVersion);
        SetConsoleTitle(sConTitle);

        ConIO.StdIn  = GetStdHandle(STD_INPUT_HANDLE);
        ConIO.StdOut = GetStdHandle(STD_OUTPUT_HANDLE);
        ConIO.StdErr = GetStdHandle(STD_ERROR_HANDLE);

        return true;
    }
    return false;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool CloseConsole(void)
{
    if(!gbConsoleOpen)
        return false;

    if(FreeConsole())
    {
        CloseHandle(ConIO.StdIn);
        CloseHandle(ConIO.StdOut);
        CloseHandle(ConIO.StdErr);

        gbConsoleOpen = false;
        return true;
    }
    return false;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

BOOL WriteConEx(char *Buffer, int Length)
{
    DWORD NumberOfBytesWritten = 0;
    return WriteFile(ConIO.StdOut, Buffer, Length, &NumberOfBytesWritten, 0);;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

BOOL WriteCon(char *Buffer)
{
    return WriteConEx(Buffer, (int)strlen(Buffer));
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

DWORD GetConsoleInput(char *RetBuffer, int MaxLength)
{
    DWORD NumberOfBytesRead = 0;
    ReadFile(ConIO.StdIn, RetBuffer, MaxLength, &NumberOfBytesRead, 0);
    NumberOfBytesRead = (NumberOfBytesRead - 2);
    if(NumberOfBytesRead > 0)
    {
        RetBuffer[NumberOfBytesRead] = '\0';
        return NumberOfBytesRead;
    }
    return 0;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#endif
