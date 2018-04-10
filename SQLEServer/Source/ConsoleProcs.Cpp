///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//  Copyright © NetworkDLS 2002, All rights reserved
//
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF 
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO 
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A 
// PARTICULAR PURPOSE.
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef _CONSOLEPROCS_CPP
#define _CONSOLEPROCS_CPP
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include <Windows.H>
#include <StdIO.H>
#include <STDLib.H>
#include <PSAPI.H>
#include <SQL.H>

#include "../../SharedSource/Common.H"

#include "Entry.H"
#include "Init.H"
#include "Routines.H"
#include "Console.H"
#include "Console.H"
#include "ConsoleProcs.H"

#include "../../../@Libraries/CSocketServer/CSocketServer.H"

#include "../Dialogs/MainDlg.H"
#include "../Dialogs/AlertDlg.H"

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

using namespace NSWFL::Conversion;
using namespace NSWFL::DateTime;

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool DisplayConnectionInfo(void)
{
	gServer.Lock();
	gServer.PausePumpThread();

	WriteCon("Connections: (Current: %d of %d, Total: %d)\n\n",
		gServer.CurrentConnections(), gServer.MaxConnections(), gdwTotalConnects);

	for(int iConnection = 0; iConnection < gServer.MaxConnections(); iConnection++)
	{
		CSocketClient *pClient = gServer.Client(iConnection);

		if(pClient && pClient->IsConnected())
		{
			int iSlot = pClient->Slot();

			WriteCon(
				"Connection %d, ID: %d, IP: [%s], Version: [%s]\n"
				"\tAuthenticated: %d\n"
				"\tInitializing: %d, Step: %d\n"
				"\tDB Connect: (Index: %d / Customer: %d)\n"
				"\tBytes: (Sent: %d, Recvd: %d)\n"
				"\n",
				iSlot,
				pClient->PeerID(),
				pClient->PeerAddress(),
				CCI[iSlot].sClientVersion,
				CCI[iSlot].bIsAuthenticated,
				CCI[iSlot].bRequestInit,
				CCI[iSlot].iInitStep,
				CCI[iSlot].bIndexConnected,
				CCI[iSlot].bCustomerConnected,
				CCI[iSlot].dwBytesSent,
				CCI[iSlot].dwBytesRecv
			);

			WriteCon(CONSOLEHR"\n\n");
		}
	}

	gServer.ResumePumpThread();
	gServer.Unlock();

	WriteCon("Complete.\n");

	return true;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool ConsoleSysStats(void)
{
	char sInt[64];

	PROCESS_MEMORY_COUNTERS MyPMC;

	HANDLE hProcess = GetCurrentProcess();

	memset(&MyPMC, 0, sizeof(MyPMC));

    if(!GetProcessMemoryInfo(hProcess, &MyPMC, sizeof(MyPMC)))
	{
		return false;
	}

	I64FormatInteger(sInt, sizeof(sInt), MyPMC.PageFaultCount);
	WriteCon("Page Faults: %s\n", sInt);

	I64FormatInteger(sInt, sizeof(sInt), MyPMC.WorkingSetSize / 1024);
	WriteCon("Memory Used: %sKB\n", sInt);

	I64FormatInteger(sInt, sizeof(sInt), MyPMC.QuotaPagedPoolUsage / 1024);
	WriteCon("Paged Pool Usage: %sKB\n", sInt);

	I64FormatInteger(sInt, sizeof(sInt), MyPMC.QuotaPeakNonPagedPoolUsage / 1024);
	WriteCon("Non-Paged Pool Usage: %sKB\n", sInt);

	I64FormatInteger(sInt, sizeof(sInt), MyPMC.PagefileUsage / 1024);
	WriteCon("Page File Usage: %sKB\n", sInt);

	I64FormatInteger(sInt, sizeof(sInt), MyPMC.PeakWorkingSetSize / 1024);
	WriteCon("Peak Memory Usage: %sKB\n", sInt);

	I64FormatInteger(sInt, sizeof(sInt), MyPMC.QuotaPeakPagedPoolUsage / 1024);
	WriteCon("Peak Paged Pool Usage: %sKB\n", sInt);

	I64FormatInteger(sInt, sizeof(sInt), MyPMC.QuotaPeakNonPagedPoolUsage / 1024);
	WriteCon("Peak Non-Paged Pool Usage: %sKB\n", sInt);

	I64FormatInteger(sInt, sizeof(sInt), MyPMC.PeakPagefileUsage / 1024);
	WriteCon("Peak Page File Usage: %sKB\n", sInt);

	return true;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool DumpSQLStatsToConsole(HSTMT hStmt)
{
    return 0;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool DumpSQLResultsToConsole(HSTMT hStmt)
{
    return 0;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool AskForConfirmation(const char *sMsg)
{
	if(sMsg)
	{
		WriteCon(sMsg);
	}
	else{
		WriteCon("Are you sure?\n");
	}

	WriteCon("(Y/N): ");

	char sAwnser[30];

	if(GetConsoleInput(sAwnser, sizeof(sAwnser)))
	{
		return (_strcmpi(sAwnser, "Y") == 0 || _strcmpi(sAwnser, "Yes") == 0);
	}

	return false;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool ConsoleConnect(void)
{
	return false;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void TestAlertDlg(const char *sText)
{
	StartAlertDlg("Test", sText, true);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#if _MEMPOOL_DEBUG_LEVEL >= 1
	void PrintMemMap(void)
	{
		//int iAlloc = 0;

		//char sText[1024];
		//
		//WriteCon("-----------------------------------------------------\n");
		//WriteCon("Allocated Memory Map:\n\n");
		//sprintf_s(sText, sizeof(sText), "Allocations %d of %d.\n", 
		//	pMem->GetMemAllocationCount(), pMem->GetSlotAllocationCount());
		//WriteCon(sText);

		////Print all the memory stored in the AllocPool structure.
		//for(unsigned long ulSlot = 0; ulSlot < pMem->Items.iAllocated; ulSlot++)
		//{
		//	if(pMem->Items.Array[ulSlot].bFree == false)
		//	{
		//		sprintf_s(sText, sizeof(sText), "Allocation: %d, Slot: %d, Address: 0x%X, Bytes: %d\n",
		//			iAlloc++, ulSlot,
		//			pMem->Items.Array[ulSlot].iAddress,
		//			pMem->Items.Array[ulSlot].iSize);
		//		WriteCon(sText);
		//	}
		//}

		//WriteCon("-----------------------------------------------------\n");
	}
#endif

void ConsoleDisconnectAll()
{
	gServer.Lock();
	gServer.PausePumpThread();

	int iDisconnects = 0;

	WriteCon("Connections: (Current: %d of %d, Total: %d)\n\n",
		gServer.CurrentConnections(), gServer.MaxConnections(), gdwTotalConnects);

	for(int iConnection = 0; iConnection < gServer.MaxConnections(); iConnection++)
	{
		CSocketClient *pClient = gServer.Client(iConnection);

		if(pClient && pClient->IsConnected())
		{
			if(pClient->Disconnect())
			{
				iDisconnects++;
			}
		}
	}

	gServer.ResumePumpThread();
	gServer.Unlock();

	WriteCon("Disconnected %d clients.\n", iDisconnects);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

DWORD ProcessConsoleCoreCommand(char *sCommand)
{
	char sTempText[1024];

    if(_strcmpi(sCommand, "help") == 0)
    {
        WriteCon(" Version\t Displays version info.\n");
        WriteCon(" Cls\t\t Clears the screen.\n");
        WriteCon(" Connections\t Displays conections info.\n");
        WriteCon(" KillAll\t Kills all current client connections.\n");
		WriteCon(" Kill:[PeerID]\t Disconnects a specified connection.\n");
        //WriteCon(" Connect\t Connects to a database and execute commands.\n");
        WriteCon(" Exit\t\t Closes the console.\n");
		WriteCon(" SysStats\t\t Displays basic system information.\n");
        WriteCon(" Start\t\t Starts the server.\n");
        WriteCon(" Stop\t\t Stoppes the server.\n");
        WriteCon(" FlushIO\t Flushes all IO streams to disk.\n");
        WriteCon(" ReadRegistry\t Re-Reads all registry values.\n");
        WriteCon(" WriteRegistry\t Writes all registry values.\n");
        WriteCon(" PauseTCP\t Pauses the TCPLoop thread.\n");
        WriteCon(" ResumeTCP\t Resumes the TCPLoop thread.\n");
        WriteCon(" Accept:ON\t Accepts incomming connections.\n");
        WriteCon(" Accept:OFF\t Rejects incomming connections.\n");
		#if _MEMPOOL_DEBUG_LEVEL >= 1
			WriteCon(" MemMap\t\t Prints the allocation tables.\n");
		#endif
		return CONSOLE_CORE_RES_OK;
    }
	#if _MEMPOOL_DEBUG_LEVEL >= 1
		else if(_strcmpi(sCommand, "MemMap") == 0)
		{
			PrintMemMap();
			return CONSOLE_CORE_RES_OK;
		}
	#endif
	else if(_strcmpi(sCommand, "Cls") == 0)
    {
        system("cls");
        return CONSOLE_CORE_RES_OK;
    }
	else if(_strcmpi(sCommand, "SysStats") == 0)
    {
        if(!ConsoleSysStats())
		{
			WriteCon("ConsoleSysStats failed.\n");
		}
        return CONSOLE_CORE_RES_OK;
    }
	else if(_strcmpi(sCommand, "Accept:ON") == 0)
    {
        gServer.AllowNewConnections(true);
        return CONSOLE_CORE_RES_OK;
    }
    else if(_strcmpi(sCommand, "Accept:Off") == 0)
    {
        gServer.AllowNewConnections(false);
        return CONSOLE_CORE_RES_OK;
    }
	else if(_strcmpi(sCommand, "PauseTCP") == 0)
    {
		WriteCon("Suspending packet handling process:");
		gServer.Lock();
		bool bResult = gServer.PausePumpThread();
		gServer.Unlock();

		WriteCon("%s.\n", bResult ? "Ok" : "Fail");
        return CONSOLE_CORE_RES_OK;
    }
    else if(_strcmpi(sCommand, "ResumeTCP") == 0)
    {
		WriteCon("Resuming packet handling process:");
		gServer.Lock();
		bool bResult = gServer.ResumePumpThread();
		gServer.Unlock();

		WriteCon("%s.\n", bResult ? "Ok" : "Fail");
        return CONSOLE_CORE_RES_OK;
    }
    else if(_strcmpi(sCommand, "ReadRegistry") == 0)
    {
		WriteCon("Reading registry values: ");

		gServer.Lock();
		gServer.PausePumpThread();

		WriteCon("%s.\n", GetRegistryValues() ? "Ok" : "Fail");

		gServer.ResumePumpThread();
		gServer.Unlock();

        return CONSOLE_CORE_RES_OK;
    }
    else if(_strcmpi(sCommand, "WriteRegistry") == 0)
    {
		WriteCon("Reading registry values: ");

		gServer.Lock();
		gServer.PausePumpThread();

		WriteCon("%s.\n", SaveRegistryValues() ? "Ok" : "Fail");

		gServer.ResumePumpThread();
		gServer.Unlock();

        return CONSOLE_CORE_RES_OK;
    }
    else if(_strcmpi(sCommand, "FlushIO") == 0)
    {
        WriteCon("Flushed %d IO streames to disk.\n", _flushall());
        return CONSOLE_CORE_RES_OK;
    }
	else if(_strnicmp(sCommand, "Kill:", 5) == 0)
    {
		int iPeerID = atol(sCommand + 5);
		
		if(iPeerID <= 0 || iPeerID > gServer.MaxConnections())
		{
			WriteCon("%d is an invalid Peer ID.\n", iPeerID);
			return CONSOLE_CORE_RES_OK;
		}

		sprintf_s(sTempText, sizeof(sTempText), "Disconnect the client with Peer ID %d?\n", iPeerID);
		if(AskForConfirmation(sTempText))
		{
			CSocketClient *pClient = gServer.Peer(iPeerID);

			if(pClient && pClient->IsConnected())
			{
				WriteCon("%s.\n", pClient->Disconnect() ? "Ok" : "Fail");
			}
			else{
				WriteCon("Peer %d is not connected.", iPeerID);
			}
		}

        return CONSOLE_CORE_RES_OK;
    }
	else if(_strcmpi(sCommand, "KillAll") == 0)
    {
		if(AskForConfirmation("Disconnect all existing connections?\n"))
		{
			ConsoleDisconnectAll();
		}
        return CONSOLE_CORE_RES_OK;
    }
    else if(_strcmpi(sCommand, "Start") == 0)
    {
        if(StartServer())
		{
            WriteCon("Server started.\n");
		}
		else{
			WriteCon("Failed to start the server.\n");
		}
        return CONSOLE_CORE_RES_OK;
    }
    else if(_strcmpi(sCommand, "Stop") == 0)
    {
        if(StopServer())
		{
            WriteCon("Server stopped.\n");
		}
		else{
			WriteCon("Failed to stop the server.\n");
		}
        return CONSOLE_CORE_RES_OK;
    }
    else if(_strcmpi(sCommand, "Version") == 0)
    {
        WriteCon("%s - [Version: %s] (Built: %s at %s)\n",
			gsTitleCaption, gsFileVersion, __DATE__, __TIME__);
        return CONSOLE_CORE_RES_OK;
    }
    else if(_strcmpi(sCommand, "connections") == 0)
    {
		DisplayConnectionInfo();
        return CONSOLE_CORE_RES_OK;
    }
	else if(_strnicmp(sCommand, "Alert ", 6) == 0)
	{
		TestAlertDlg(sCommand + 6);
	}
	else if(_strcmpi(sCommand, "Connect") == 0)
    {
        ConsoleConnect();
        return CONSOLE_CORE_RES_OK;
    }
    else if(_strcmpi(sCommand, "exit") == 0)
    {
        return CONSOLE_CORE_RES_CLOSE;
    }
    else{
        WriteCon("Unknown command: '%s'.\n", sCommand);
        return CONSOLE_CORE_RES_UNKNOWN;
    }

    return CONSOLE_CORE_RES_ERROR;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#endif
