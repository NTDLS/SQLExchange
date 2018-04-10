///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//  Copyright © NetworkDLS 2002, All rights reserved
//
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF 
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO 
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A 
// PARTICULAR PURPOSE.
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#ifndef _ListView_Cpp
#define _ListView_Cpp
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include <Windows.H>
#include <Stdio.H>
#include <Stdlib.H>
#include <CommCtrl.h>

#include "../../@Libraries/NSWFL/NSWFL.H"

#include "ListView.H"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

using namespace NSWFL::ListView;
using namespace NSWFL::Conversion;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int iSortCol = -1;
int iSortUpOrDown = -1;

int CALLBACK CompList(LPARAM lParam1, LPARAM lParam2, LPARAM lParam3)
{
    int RetVal = 0;
    SORTDATA *lpsd;

    lpsd = (SORTDATA *)lParam3;

    if(lParam1 > lParam2)
        RetVal = 1;
    else if(lParam1 < lParam2)
        RetVal = -1;
    else return 0;

    if(lpsd->iUPDOWN == SORT_DOWN)
    {
        if(RetVal == 1)
            return -1;
        else return 1;
    }
    else return RetVal;

    return 0;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int CALLBACK CompListEx(LPARAM lParam1, LPARAM lParam2, LPARAM lParam3)
{
    char buf1[MAX_PATH];
    char buf2[MAX_PATH];

    int RetVal = 0;

    SORTDATA *lpsd;

    lpsd = (SORTDATA *)lParam3;

    ListView_GetItemText(lpsd->hwndList, lParam1, (int)lpsd->isortSubItem, buf1, sizeof(buf1));
    ListView_GetItemText(lpsd->hwndList, lParam2, (int)lpsd->isortSubItem, buf2, sizeof(buf2));

    if(lpsd->iColType == COL_STRING)
    {
        RetVal = _strcmpi(buf1, buf2);
    }
    else if(lpsd->iColType == COL_INTEGER)
    {
        __int64 i64Val1 = i64FormattedValue(buf1);
        __int64 i64Val2 = i64FormattedValue(buf2);

        if(i64Val1 > i64Val2)
            RetVal = 1;
        else if(i64Val1 < i64Val2)
            RetVal = -1;
        else return 0;
    }
    else if(lpsd->iColType == COL_DOUBLE)
    {
        double Val1 = fFormattedValue(buf1);
        double Val2 = fFormattedValue(buf2);

        if(Val1 > Val2)
            RetVal = 1;
        else if(Val1 < Val2)
            RetVal = -1;
        else return 0;
    }

    if(RetVal == 0)
        return 0;

    if(lpsd->iUPDOWN == SORT_DOWN)
    {
        if(RetVal == 1)
            return -1;
        else return 1;
    }
    else return RetVal;

    return 0;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#endif
