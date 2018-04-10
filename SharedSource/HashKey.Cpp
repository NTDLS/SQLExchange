///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//  Copyright © NetworkDLS 2002, All rights reserved
//
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF 
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO 
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A 
// PARTICULAR PURPOSE.
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef NASCCL_HashKey_Cpp
#define NASCCL_HashKey_Cpp
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*
    The HashKey function will take any valid key and resize it to any size
    (between MaxKeyLength & MinKeyLength). This function will generate the same
    "Shorter or Longer" key every time it is called as long as the input key
    (InputKey) and the final key size (FinalSize) stay the same.

    This functions can also be used to remove Null, CR & LF from your keys.
*/
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "HashKey.H"

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool HashKey(const unsigned char *sUserKey, unsigned char *sHashKey,
			 const int inKeyLen, const int inOutLen)
{
    int iPos = 0;
    int iKeyPos = 0;

    while(iPos < inOutLen)
    {
        sHashKey[iPos] = (sUserKey[iKeyPos] + ((inKeyLen ^ iPos) +
			(inOutLen ^ iPos))) + sUserKey[iKeyPos] ^ iKeyPos;

		if(iKeyPos == inKeyLen)
		{
			iKeyPos = -1;
		}

        iPos++;
        iKeyPos++;
    }

    return true;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#endif

