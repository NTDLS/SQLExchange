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

#include "../Resources/Resource.H"

#include "../../SharedSource/Common.H"

#include "Routines.H"
#include "Entry.H"
#include "Console.H"
#include "ConsoleProcs.H"

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

HANDLE Console_Thread_Handle = NULL;
DWORD Console_Thread_ID = 0;

_ConIO ConIO;

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
	{
        return false;
	}

    if(AllocConsole())
    {
        gbConsoleOpen = true;
        CheckMenuItem(MDI.hMenu, ID_SETTINGS_OPENCONSOLE, MF_BYCOMMAND|MF_CHECKED);

        SetConsoleCtrlHandler(ConsoleHandlerRoutine, TRUE);

        char sConTitle[255];
        sprintf_s(sConTitle, sizeof(sConTitle), "%s [Version: %s]", gsTitleCaption, gsFileVersion);
        SetConsoleTitle(sConTitle);

		for(int iRetry = 0; iRetry < 50; iRetry++)
		{
			HWND hWnd = FindWindow(NULL, sConTitle);
			if(hWnd)
			{
				SetWindowLong(hWnd, GWL_STYLE, GetWindowLong(hWnd, GWL_STYLE) & ~WS_SYSMENU);
				SetWindowPos(hWnd, NULL, 0, 0, 0, 0, SWP_NOZORDER|SWP_NOMOVE|SWP_NOSIZE|SWP_NOACTIVATE|SWP_DRAWFRAME);
				break;
			}
			Sleep(1);
		}

        ConIO.StdIn  = GetStdHandle(STD_INPUT_HANDLE);
        ConIO.StdOut = GetStdHandle(STD_OUTPUT_HANDLE);
        ConIO.StdErr = GetStdHandle(STD_ERROR_HANDLE);

        Console_Thread_Handle = CreateThread(NULL, 0, Console_Thread, (LPVOID)0, 0,&Console_Thread_ID);
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
        CloseHandle(Console_Thread_Handle);
        CloseHandle(ConIO.StdIn);
        CloseHandle(ConIO.StdOut);
        CloseHandle(ConIO.StdErr);

        gbConsoleOpen = false;
        CheckMenuItem(MDI.hMenu, ID_SETTINGS_OPENCONSOLE, MF_BYCOMMAND|MF_UNCHECKED);
        return true;
    }
    return false;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool WriteConEx(const char *Buffer, int Length)
{
    DWORD NumberOfBytesWritten = 0;
    return WriteFile(ConIO.StdOut, Buffer, Length, &NumberOfBytesWritten, 0) > 0;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int WriteCon(const char *sFormat, ...)
{
	va_list ArgList;
	va_start(ArgList, sFormat);

	int iMemoryRequired = _vscprintf(sFormat, ArgList);

	#ifdef _USE_GLOBAL_MEMPOOL
		char *sBuf = (char *) pMem->Allocate(sizeof(char), iMemoryRequired + 1);
	#else
		char *sBuf = (char *) calloc(sizeof(char), iMemoryRequired + 1);
	#endif

	int iSz = _vsprintf_s_l(sBuf, iMemoryRequired + 1, sFormat, NULL, ArgList);
	va_end(ArgList);

	bool bResult = WriteConEx(sBuf, iSz);

	#ifdef _USE_GLOBAL_MEMPOOL
		pMem->Free(sBuf);
	#else
		free(sBuf);
	#endif

	return bResult;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

DWORD GetConsoleInput(char *sRetBuffer, int iMaxLength)
{
    DWORD iNumberOfBytesRead = 0;
    ReadFile(ConIO.StdIn, sRetBuffer, iMaxLength, &iNumberOfBytesRead, 0);
    iNumberOfBytesRead = (iNumberOfBytesRead - 2);
    if(iNumberOfBytesRead > 0)
    {
        sRetBuffer[iNumberOfBytesRead] = '\0';
        return iNumberOfBytesRead;
    }
    return 0;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

DWORD WINAPI Console_Thread(LPVOID lpData)
{
    char sCmd[1025];

	WriteCon("To exit console mode, type Exit. For more information, type Help\n\n");

	WriteCon("> ");
    while(1)
    {
        int iCmdLen = GetConsoleInput(sCmd, sizeof(sCmd));
        if(iCmdLen > 0)
        {
            int Result = ProcessConsoleCoreCommand(sCmd);
            if(Result == CONSOLE_CORE_RES_CLOSE)
            {
                CloseConsole();
                return 0;
            }
            WriteCon("\n> ");
        }
        else WriteCon("> ");
        Sleep(1);
    }

	return 0;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#endif
