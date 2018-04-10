///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//  Copyright © NetworkDLS 2002, All rights reserved
//
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF 
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO 
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A 
// PARTICULAR PURPOSE.
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#ifndef _CGETPKS_H
#define _CGETPKS_H
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "../../../@Libraries/Debug/Debug.H"
#include "../../../@Libraries/CMemPool/CMemPool.H"
#include "../../../@Libraries/CSQLBinding/CSQLEncapsulation.H"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

typedef struct _tag_CGetPKs_Column {
	char *Name;
	char *Collation;
	char *DataType;
	int MaxCharData;
	bool IsCharType;
	int Precision;
	int Scale;
} CPKCOLUMN, *LPCPKCOLUMN;

typedef struct _tag_CGetPKs_Columns {
	CPKCOLUMN *Array;
	int Count;
} CPKCOLUMNS, *LPCPKCOLUMNS;

class CGetPKs{
public:
	bool Get(CSQL *pcSQL, char *sDatabase, char *sSchema, char *sTable);
	void Free(void);
	CGetPKs();
	~CGetPKs();

	CPKCOLUMNS Columns;

	char Table[1024];
	char Schema[1024];
	char Database[1024];
	char Constraint[1024];

private:
	bool bAlloc;
	void Free(void *pMemory);
	void *Alloc(int iSize, int iCount);
};

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#endif
