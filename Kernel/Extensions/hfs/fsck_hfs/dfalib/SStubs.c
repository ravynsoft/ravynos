/*
 * Copyright (c) 1999, 2002-2003, 2005-2008 Apple Inc. All rights reserved.
 *
 * @APPLE_LICENSE_HEADER_START@
 * 
 * This file contains Original Code and/or Modifications of Original Code
 * as defined in and that are subject to the Apple Public Source License
 * Version 2.0 (the 'License'). You may not use this file except in
 * compliance with the License. Please obtain a copy of the License at
 * http://www.opensource.apple.com/apsl/ and read it before using this
 * file.
 * 
 * The Original Code and all software distributed under the License are
 * distributed on an 'AS IS' basis, WITHOUT WARRANTY OF ANY KIND, EITHER
 * EXPRESS OR IMPLIED, AND APPLE HEREBY DISCLAIMS ALL SUCH WARRANTIES,
 * INCLUDING WITHOUT LIMITATION, ANY WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, QUIET ENJOYMENT OR NON-INFRINGEMENT.
 * Please see the License for the specific language governing rights and
 * limitations under the License.
 * 
 * @APPLE_LICENSE_HEADER_END@
 */
/* SStubs.c */


#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <sys/time.h>

#include "Scavenger.h"
#include "../fsck_messages.h"


/*
 *	This is the straight GMT conversion constant:
 *	00:00:00 January 1, 1970 - 00:00:00 January 1, 1904
 *	(3600 * 24 * ((365 * (1970 - 1904)) + (((1970 - 1904) / 4) + 1)))
 */
#define MAC_GMT_FACTOR		2082844800UL

/*
 * GetTimeUTC - get the GMT Mac OS time (in seconds since 1/1/1904)
 *
 */
UInt32 GetTimeUTC(void)
{
	struct timeval time;
	struct timezone zone;

	(void) gettimeofday(&time, &zone);

    // Value will be bigger than UIN32_MAX in 2040
	return (UInt32)(time.tv_sec + MAC_GMT_FACTOR);
}

/*
 * GetTimeLocal - get the local Mac OS time (in seconds since 1/1/1904)
 *
 */
UInt32 GetTimeLocal(Boolean forHFS)
{
	struct timeval time;
	struct timezone zone;
	time_t localTime;

	(void) gettimeofday(&time, &zone);
	localTime = time.tv_sec + MAC_GMT_FACTOR - (zone.tz_minuteswest * 60);

	if (forHFS && zone.tz_dsttime)
		localTime += 3600;

	return (UInt32)localTime;
}


OSErr FlushVol(ConstStr63Param volName, short vRefNum)
{
	sync();
	
	return (0);
}


OSErr MemError()
{
	return (0);
}

void DebugStr(ConstStr255Param debuggerMsg)
{
	/* DebugStr is only called when built with DEBUG_BUILD set */
	plog ("\t%.*s\n", debuggerMsg[0], &debuggerMsg[1]);
}


UInt32 TickCount()
{
	return (0);
}


OSErr GetVolumeFeatures( SGlobPtr GPtr )
{
	GPtr->volumeFeatures = supportsTrashVolumeCacheFeatureMask + supportsHFSPlusVolsFeatureMask;

	return( noErr );
}


Handle NewHandleClear(Size byteCount)
{
	return NewHandle(byteCount);
}

Handle NewHandle(Size byteCount)
{
	Handle h;
	Ptr p = NULL;

	if (!(h = malloc(sizeof(Ptr) + sizeof(Size))))
		return NULL;
		
	if (byteCount)
		if (!(p = calloc(1, byteCount)))
		{
			free(h);
			return NULL;
		}
	
	*h = p;
	
	*((Size *)(h + 1)) = byteCount;	
	
	return h;
}

void DisposeHandle(Handle h)
{
	if (h)
	{
		if (*h)
			free(*h);
		free(h);
	}
}

Size GetHandleSize(Handle h)
{
	return h ? *((Size *)(h + 1)) : 0;
}

void SetHandleSize(Handle h, Size newSize)
{
	Ptr p = NULL;

	if (!h)
		return;

	if ((p = realloc(*h, newSize)))
	{
		*h = p;
		*((Size *)(h + 1)) = newSize;
	}
}


OSErr PtrAndHand(const void *ptr1, Handle hand2, long size)
{
	Ptr p = NULL;
	Size old_size = 0;

	if (!hand2)
		return -109;
	
	if (!ptr1 || size < 1)
		return 0;
		
	old_size = *((Size *)(hand2 + 1));

	if (!(p = realloc(*hand2, size + old_size)))
		return -108;

	*hand2 = p;
	*((Size *)(hand2 + 1)) = size + old_size;
	
	memcpy(*hand2 + old_size, ptr1, size);
	
	return 0;
}


/* deprecated call, use fsckPrint() instead */
void WriteError( SGlobPtr GPtr, short msgID, UInt32 tarID, UInt64 tarBlock )  
{
	fsckPrint(GPtr->context, msgID);

	if ((fsckGetVerbosity(GPtr->context) > 0) && 
	    (fsckGetOutputStyle(GPtr->context) == fsckOutputTraditional) &&
	    (tarID | tarBlock) != 0) {
		plog("(%ld, %qd)\n", (long)tarID, tarBlock);
	}
}
