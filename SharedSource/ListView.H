///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//  Copyright © NetworkDLS 2002, All rights reserved
//
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF 
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO 
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A 
// PARTICULAR PURPOSE.
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef _ListView_H
#define _ListView_H
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#define SORT_UP        1
#define SORT_DOWN      2

#define COL_STRING     1
#define COL_INTEGER    2
#define COL_DOUBLE     3
#define COL_DATE       4

typedef struct _tagSORTDATA{
    HWND hwndList;
    int isortSubItem;
    int iUPDOWN;
    int iColType;
} SORTDATA;

extern int iSortCol;
extern int iSortUpOrDown;

int CALLBACK CompList(LPARAM lParam1, LPARAM lParam2, LPARAM lParam3);
int CALLBACK CompListEx(LPARAM lParam1, LPARAM lParam2, LPARAM lParam3);

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#endif
