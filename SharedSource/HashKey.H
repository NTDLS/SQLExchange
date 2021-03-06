///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//  Copyright � NetworkDLS 2002, All rights reserved
//
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF 
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO 
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A 
// PARTICULAR PURPOSE.
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef NASCCL_HashKey_H
#define NASCCL_HashKey_H
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool HashKey(const unsigned char *sUserKey, unsigned char *sHashKey,
			 const  int inKeyLen, const int inOutLen);

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#endif

