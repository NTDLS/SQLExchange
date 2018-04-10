///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//  Copyright © NetworkDLS 2002, All rights reserved
//
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF 
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO 
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A 
// PARTICULAR PURPOSE.
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef _FileExchange_CPP
#define _FileExchange_CPP
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include <Windows.H>
#include <Stdio.H>
#include <Stdlib.H>
#include <CommCtrl.H>

#include "FileExchange.H"

#include "../../@Libraries/CSHA1/CSHA1.H"
#include "../../@Libraries/CCRC32/CCRC32.H"

#include "../../@Libraries/Compression/RLE/RLEEncapsulation.H"
#include "../../@Libraries/Compression/LZARI/LZARIEncapsulation.H"
#include "../../@Libraries/Compression/zLib/ZLibEncapsulation.H"
#include "../../@Libraries/Compression/LZSS/LZSSEncapsulation.H"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

using namespace NSWFL::Conversion;
using namespace NSWFL::File;

CCRC32 *pCRC32 = NULL;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#ifdef _SQLESERVER
	//No equivalent at this time.
#elif _SQLECLIENT
int SimpleTransfer(CSocketServer *pSock, CSocketClient *pClient, const char *sFileName, CXMLWriter *lpXML)
{
	lpXML->Add("CompressionMethod", CCI.sCompressionMethod);
	lpXML->Add("CompressionLevel", CCI.dwCompressionLevel);
	return SendFileData(pSock, pClient, sFileName, lpXML);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int SimpleTransfer(CSocketServer *pSock, CSocketClient *pClient, const char *sFileName, char *sFileTag)
{
	CXMLWriter MyWXML(sFileTag);
	MyWXML.Add("CompressionMethod", CCI.sCompressionMethod);
	MyWXML.Add("CompressionLevel", CCI.dwCompressionLevel);
	int iResult = SendFileData(pSock, pClient, sFileName, &MyWXML);
	MyWXML.Destroy();
	return iResult;
}
#else
	#error "You must define either _SQLESERVER or _SQLECLIENT"
#endif

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/*
This function requires that you pass in an XMLBuilder that contains at least the following attributes:
	1) CompressionMethod : The method of compression to use.
	2) CompionLevel      :  The compression level to use.

The XMLBuilder must have a valid name. This name will be changed to <::Sending::NAME>
	before it is sent to the remote host.

The SendFileData will add the following attributes to the passed in XMLBuilder:
	1) CompletionTag      : The name of the XML structure that will be sent at file transfer completion.
	2) FileSize           : The original size of the transferring file before any compression.
	3) Compressed         : Boolean - True=[File is compressed], False=[File is not compressed].
	4) CompressedFileSize : The size of the transferring file after compression.
	5) CompressionMethod  : The compression method used to compress the transferring file.

After the file transfer is complete a "::EOF" flag fill be sent, followed directly by another
	XML structure named <::Sent::%s> that contains the following attributes:
	1) CRC32: The CRC32 value of the file that was transferred.

Notes:
	Any other attributes added to the passed in XMLBuilder will also be sent to the remote host
		prior to the file transfer.
*/

int SendFileData(CSocketServer *pSock, CSocketClient *pClient, const char *sFileName, CXMLWriter *lpXML)
{
    FILE *hSourceHandle = NULL;

	char sXMLTag[255];
	char sFullFile[MAX_PATH];
    char sFileBuffer[FILEBUFFERSZ + 1];
	int iBytesRead = 0;
	DWORD dwDataCRC = 0xffffffff;
	
	//---------------------------------------------------------------------------------
	//Obtain required settings:
	int iCompressionLevel = 0;
	char sCompressionMethod[100];
	char sFileTag[255];

	CXMLReader XMLParser;

	lpXML->GetName(sFileTag, sizeof(sFileTag)); //The file tag is the name of the XML child.
	lpXML->ToReader(&XMLParser);
	XMLParser.ToString("CompressionMethod", sCompressionMethod, sizeof(sCompressionMethod));
	iCompressionLevel = XMLParser.ToInteger("CompressionLevel");
	XMLParser.Destroy();

	//---------------------------------------------------------------------------------
	__int64 i64BeforeSz = 0; //Before Compression.
	__int64 i64AfterSz = 0; //After compression.

	FileSize(sFileName, &i64BeforeSz);

	if(_strcmpi(sCompressionMethod, "NONE") != 0)
	{
		char sBeforeSz[64];
		char sAfterSz[64];

		FileSizeFriendly(i64BeforeSz, sBeforeSz, sizeof(sBeforeSz));

		sprintf_s(sFullFile, sizeof(sFullFile), "%s.sec", sFileName);
		sprintf_s(sFileBuffer, sizeof(sFileBuffer), "Compressing %s file.", sBeforeSz);
		WriteLog(pClient->PeerID(), sFileBuffer, EVENT_NONE);

		if(_strcmpi(sCompressionMethod, "GZIP") == 0){
			GZipDeflateFile(sFileName, sFullFile, iCompressionLevel);
		}
		else if(_strcmpi(sCompressionMethod, "LZSS") == 0){
			LZSSCompressFile(sFileName, sFullFile, iCompressionLevel);
		}
		else if(_strcmpi(sCompressionMethod, "LZARI") == 0){
			LZARICompressFile( sFileName, sFullFile);
		}
		else if(_strcmpi(sCompressionMethod, "RLE") == 0){
			RLEEncodeFile(sFileName, sFullFile);
		}

		FileSize(sFullFile, &i64AfterSz);
		FileSizeFriendly(i64AfterSz, sAfterSz, sizeof(sAfterSz));

		sprintf_s(sFileBuffer, sizeof(sFileBuffer),
			"File size reduced from %s to %s. (%.2f %% Compression).",
			sBeforeSz, sAfterSz, (100.0 - (((double)i64AfterSz / (double)i64BeforeSz) * 100.0)));
		WriteLog(pClient->PeerID(), sFileBuffer, EVENT_NONE);
	}
	else{
		strcpy_s(sFullFile, sizeof(sFullFile), sFileName);
	}

	//---------------------------------------------------------------------------------
	//Set transfer options:

	sprintf_s(sXMLTag, sizeof(sXMLTag), "::Sending::%s", sFileTag);
	lpXML->ChangeName(sXMLTag);

	sprintf_s(sXMLTag, sizeof(sXMLTag), "<::Sent::%s>", sFileTag);
	lpXML->Add("CompletionTag", sXMLTag);
	lpXML->Add("FileSize", i64BeforeSz);
	lpXML->Add("Compressed", (_strcmpi(sCompressionMethod, "NONE") != 0));
	lpXML->Add("CompressedFileSize", i64AfterSz);
	lpXML->Add("CompressionMethod", sCompressionMethod);
	SendCommand(pClient, lpXML);
	//---------------------------------------------------------------------------------

	WriteLog(pClient->PeerID(), "Begining file transfer.", EVENT_NONE);
	if(fopen_s(&hSourceHandle, sFullFile, "rb") != 0)
    {
		WriteLog(pClient->PeerID(), "Failed to open export file for binary read.", EVENT_ERROR);
        return FILEEXCHRESULT_ERROR;
    }

    do{
        iBytesRead = (int)fread(sFileBuffer, sizeof(char), FILEBUFFERSZ, hSourceHandle);
        pCRC32->PartialCRC((unsigned int*)&dwDataCRC, (unsigned char *)sFileBuffer, iBytesRead);

		if(!pClient->SetNextSendData(sFileBuffer, iBytesRead))
		{
			pClient->Disconnect();
			break;
		}
    } while(iBytesRead == FILEBUFFERSZ && pClient->IsConnected());

	dwDataCRC = (dwDataCRC ^ 0xffffffff);
	
	fclose(hSourceHandle);

	DeleteFile(sFullFile);
	DeleteFile(sFileName);

	if(pClient->IsConnected())
	{
		WriteLog(pClient->PeerID(), "File Transfer complete.", EVENT_NONE);
		pClient->SetNextSendData("::EOF");

		//Send the data CRC.
		sprintf_s(sFullFile, sizeof(sFullFile), "::Sent::%s", sFileTag);
		CXMLWriter MyOpt(sFullFile);
		MyOpt.Add("CRC32", dwDataCRC);
		SendCommand(pClient, &MyOpt);
		MyOpt.Destroy();

		return FILEEXCHRESULT_OK;
	}
	else return FILEEXCHRESULT_DISCONNECT;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int ReceiveFileData(CSocketServer *pSock, CSocketClient *pClient, CXMLReader *lpXML, char *sFileName)
{
    FILE *hTargetHandle = NULL;

    char sText[1024];
    char sCompressionMethod[100];
	char sFullFile[MAX_PATH];
	char sCompletionTag[255];

	__int64 i64FileSize = 0;

	DWORD dwDataCRC = 0xffffffff;

	if(gbDebugMode)
	{
		WriteLog(pClient->PeerID(), "Begining file transfer.", EVENT_NONE);
	}

	lpXML->ToString("CompletionTag", sCompletionTag, sizeof(sCompletionTag));
	//lpXML->ToString("FileSize", sCompletionTag, sizeof(sCompletionTag));
	//lpXML->ToString("Compressed", sCompletionTag, sizeof(sCompletionTag));
	//lpXML->ToString("CompressedFileSize", sCompletionTag, sizeof(sCompletionTag));
	lpXML->ToString("CompressionMethod", sCompressionMethod, sizeof(sCompressionMethod));

	if(_strcmpi(sCompressionMethod, "None") != 0)
	{
		sprintf_s(sFullFile, sizeof(sFullFile), "%s.sec", sFileName);
	}
	else strcpy_s(sFullFile, sizeof(sFullFile), sFileName);

	if(fopen_s(&hTargetHandle, sFullFile, "wb") != 0)
    {
		WriteLog(pClient->PeerID(), "Failed to open export file for binary write.", EVENT_ERROR);
        return FILEEXCHRESULT_ERROR;
    }

	if(gbDebugMode)
	{
		sprintf_s(sText, sizeof(sText), "File opened successfully. [%s]", sFullFile);
		WriteLog(pClient->PeerID(), sText, EVENT_NONE);
	}

	BASICHUNK Chunk;
	if(!pSock->Alloc(&Chunk))
	{
		return FILEEXCHRESULT_ERROR;
	}

	while(pClient->IsConnected())
    {
		if(pClient->GetClientData(&Chunk))
        {
			if(Chunk.iLength == 5)
			{
				((char *)Chunk.pBuffer)[Chunk.iLength] = '\0';
				if(strcmp((char *)Chunk.pBuffer, "::EOF") == 0)
				{
					break; // All is well.
				}
			}
		
            pCRC32->PartialCRC((unsigned int *)&dwDataCRC, (unsigned char *)Chunk.pBuffer, Chunk.iLength);
			fwrite(Chunk.pBuffer, sizeof(BYTE), Chunk.iLength, hTargetHandle);
		}
		else Sleep(1);
	}

	if(!pClient->IsConnected())
	{
		fclose(hTargetHandle);

		char sFSize[64];
		__int64 i64FileSize = 0;
		if(FileSize(sFullFile, &i64FileSize))
		{
			FileSizeFriendly(i64FileSize, sFSize, sizeof(sFSize));
		}
		else{
			strcpy_s(sFSize, sizeof(sFSize), "0KB");
		}

		sprintf_s(sText, sizeof(sText), "Disconnected durring file transfer after %s.", sFSize);
		WriteLog(pClient->PeerID(), sText, EVENT_NONE);

		pSock->Free(&Chunk);

		return FILEEXCHRESULT_DISCONNECT;
	}

	dwDataCRC = (dwDataCRC ^ 0xffffffff);

	sprintf_s(sText, sizeof(sText), "Data transfer complete. (CRC: %X)", dwDataCRC);
	WriteLog(pClient->PeerID(), sText, EVENT_NONE);

	fclose(hTargetHandle);

	if(!FileSize(sFullFile, &i64FileSize))
	{
		WriteLog(pClient->PeerID(), "Failed to retreve size of received file.", EVENT_ERROR);
		pSock->Free(&Chunk);
		return FILEEXCHRESULT_ERROR;
	}

	if(i64FileSize <= 0)
	{
		WriteLog(pClient->PeerID(), "Received a zero byte transaction file.", EVENT_ERROR);
		pSock->Free(&Chunk);
		return FILEEXCHRESULT_ERROR;
	}

	if(pClient->GetNextClientData(&Chunk))
	{
		if(pSock->ParseCommand(&Chunk, sCompletionTag))
		{
			CXMLReader MyOpt((char *)Chunk.pBuffer, Chunk.iLength);

			if(dwDataCRC != MyOpt.ToUInteger("CRC32"))
			{
				sprintf_s(sText, sizeof(sText), "Received a corrupt file transfer. %X/%X", dwDataCRC, MyOpt.ToUInteger("CRC32"));
				WriteLog(pClient->PeerID(), sText, EVENT_WARN);
				MyOpt.Destroy();
				pSock->Free(&Chunk);
				return FILEEXCHRESULT_CORRUPT;
			}
			else if(gbDebugMode){
				WriteLog(pClient->PeerID(), "File integrity verified.", EVENT_NONE);
			}

			MyOpt.Destroy();
		}
		else{
			WriteLog(pClient->PeerID(), "Missing CRC after transfer completion.", EVENT_ERROR);
			pSock->Free(&Chunk);
			return FILEEXCHRESULT_ERROR;
		}
	}

	pSock->Free(&Chunk);

	if(_strcmpi(sCompressionMethod, "None") != 0)
	{
		__int64 i64BeforeSz = 0;
		__int64 i64AfterSz = 0;

		sprintf_s(sFullFile, sizeof(sFullFile), "%s.sec", sFileName);

		FileSize(sFullFile, &i64BeforeSz);

		char sBeforeSz[64];
		char sAfterSz[64];

		FileSizeFriendly(i64BeforeSz, sBeforeSz, sizeof(sBeforeSz));

		sprintf_s(sText, sizeof(sText), "Decompressing file. (%s)", sBeforeSz);
		WriteLog(pClient->PeerID(), sText, EVENT_NONE);

		if(_strcmpi(sCompressionMethod, "GZIP") == 0){
			GZipInflateFile(sFullFile, sFileName);
		}
		else if(_strcmpi(sCompressionMethod, "LZSS") == 0){
			LZSSUncompressFile(sFullFile, sFileName);
		}
		else if(_strcmpi(sCompressionMethod, "LZARI") == 0){
			LZARIDecompressFile(sFullFile, sFileName);
		}
		else if(_strcmpi(sCompressionMethod, "RLE") == 0){
			RLEDecodeFile(sFullFile, sFileName);
		}

		FileSize(sFileName, &i64AfterSz);
		FileSizeFriendly(i64AfterSz, sAfterSz, sizeof(sAfterSz));

		sprintf_s(sText, sizeof(sText),
			"File size increased from %s to %s. (%.2f %% Decompression).",
			sBeforeSz, sAfterSz, (100.0 - (((double)i64BeforeSz / (double)i64AfterSz) * 100.0)));
		WriteLog(pClient->PeerID(), sText, EVENT_NONE);

		DeleteFile(sFullFile);
	}

	if(pClient->IsConnected())
	{
		return FILEEXCHRESULT_OK;
	}

	WriteLog(pClient->PeerID(), "Failed to receive client data. Client disconncted.", EVENT_ERROR);

	return FILEEXCHRESULT_DISCONNECT;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#endif
