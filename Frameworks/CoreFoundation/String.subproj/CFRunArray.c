// This source file is part of the Swift.org open source project
//
// Copyright (c) 2014 - 2018 Apple Inc. and the Swift project authors
// Licensed under Apache License v2.0 with Runtime Library Exception
//
// See http://swift.org/LICENSE.txt for license information
// See http://swift.org/CONTRIBUTORS.txt for the list of Swift project authors
//


/*	CFRunArray.c
	Copyright (c) 2004-2019, Apple Inc. All rights reserved.
*/

#include "CFRunArray.h"
#include <CoreFoundation/CFString.h>
#include "CFInternal.h"
#include "CFRuntime_Internal.h"

/* CFRunArrayGuts (which holds an array of CFRunArrayItems) is the actual data keeper; the objects just point at these... CFRunArrayGuts has copy-on-write behavior.
*/
typedef struct {
    CFIndex length;
    CFTypeRef obj;
} CFRunArrayItem;

typedef struct _CFRunArrayGuts {	/* Variable sized block. */
    CFIndex numRefs;                        /* For "copy on write" behavior */
    CFIndex length;                         /* Total count of values stored by the CFRunArrayItems in list */
    CFIndex numBlocks, maxBlocks;           /* These describe the number of CFRunArrayItems in list */
    CFIndex cachedBlock, cachedLocation;    /* Cache from last lookup */
    CFRunArrayItem list[0]; /* GCC */
} CFRunArrayGuts;

/* Definition of the CF struct for CFRunArray
*/
struct __CFRunArray {
    CFRuntimeBase base;
    CFRunArrayGuts *guts;
};

/* EXTERNCOPY is used with objects coming in from the outside world, COPY is used with objects that have already been EXTERNCOPY'ed once.
*/
#define FREE(obj) CFRelease(obj)
#define COPY(obj) CFRetain(obj)
#define EXTERNCOPY(obj) CFRetain(obj)
#define ISSAME(obj1, obj2) (CFEqual(obj1, obj2))

/* To protect accesses to the refcounts of shared CFRunArrayGuts
*/
static CFLock_t runArrayLock = CFLockInit;	
#define RLEARRAYLOCK {__CFLock(&runArrayLock);}
#define RLEARRAYUNLOCK {__CFUnlock(&runArrayLock);}



/*** Internal utility ***/

/* Return the block number in the run array. Use cache if possible.
*/
static CFIndex blockForLocation(CFRunArrayGuts *guts, CFIndex location, CFRange *effectiveRange) {
    CFIndex loc, block;
    if (location > (guts->cachedLocation / 2)) {	/* Yes, we can use the cached values as our starting point */
        loc = guts->cachedLocation;
        block = guts->cachedBlock;
    } else {
        loc = block = 0;
    }
    if (loc <= location) {	/* Going forwards */
        while (loc + guts->list[block].length <= location) loc += guts->list[block++].length;
    } else {	/* Going backwards */
        do loc -= guts->list[--block].length; while ((block > 0) && (loc > location));
    }
    guts->cachedLocation = loc;
    guts->cachedBlock = block;
    if (effectiveRange) {
        effectiveRange->location = loc;
        effectiveRange->length = guts->list[block].length;
    }
    return block;
}

/* Gives the receiver its own copy of the argument list, reduces the ref count of the original. The original list is assumed to have a ref count > 1 (it's not freed). If oldGuts is not NULL, should be called when protected by a lock. Note that this may change array->guts, so any caches of this should be refreshed!
*/
static void __CFRunArrayMakeNewList(CFRunArrayRef array, CFRunArrayGuts *oldGuts) {
    CFIndex maxBlocks = (oldGuts ? oldGuts->numBlocks : 2);
    CFRunArrayGuts *newGuts = (CFRunArrayGuts *)CFAllocatorAllocate(CFGetAllocator(array), sizeof(CFRunArrayGuts) + maxBlocks * sizeof(CFRunArrayItem), 0);
    newGuts->maxBlocks = maxBlocks;
    if (oldGuts) {
	CFIndex cnt;
	for (cnt = 0; cnt < oldGuts->numBlocks; cnt++) {
	    newGuts->list[cnt].length = oldGuts->list[cnt].length;
	    newGuts->list[cnt].obj = COPY(oldGuts->list[cnt].obj);
	}
	newGuts->numBlocks = oldGuts->numBlocks;
	newGuts->cachedBlock = oldGuts->cachedBlock;
	newGuts->cachedLocation = oldGuts->cachedLocation;
	newGuts->length = oldGuts->length;
	oldGuts->numRefs--;	// !!! We assume the caller has locked; if we have separate locks per RLEArray, need one here
    } else {
	newGuts->length = newGuts->numBlocks = newGuts->cachedBlock = newGuts->cachedLocation = 0;
    }
    newGuts->numRefs = 1;
    ((struct __CFRunArray *)array)->guts = newGuts;
}

/* When you call this, don't forget to recache guts afterwards, if it was cached in a local
*/
static void __CFRunArraySetBlockCapacity(CFRunArrayRef array, CFIndex desiredCount) {
    #define MINSIZE 1
    if (desiredCount < MINSIZE) desiredCount = MINSIZE;
    /* We realloc either when there isn't enough room, or when the needed size is less than half of what's there. In both cases we allocate 33% extra. */
    if ((array->guts->maxBlocks < desiredCount) || (array->guts->maxBlocks / 2) > desiredCount) {
	CFIndex newCapacity = ((desiredCount + 3) / 3) * 4;
	((struct __CFRunArray *)array)->guts = __CFSafelyReallocateWithAllocator(CFGetAllocator(array), array->guts, sizeof(CFRunArrayGuts) + newCapacity * sizeof(CFRunArrayItem), 0, NULL);
	array->guts->maxBlocks = newCapacity;
    }
}



/*** "Polymorphic" functions ***/

static Boolean __CFRunArrayEqual(CFTypeRef cf1, CFTypeRef cf2) {
    return false;   // ???
}

static CFHashCode __CFRunArrayHash(CFTypeRef cf) {
    CFRunArrayRef array = (CFRunArrayRef)cf;
    return CFRunArrayGetCount(array); // ???
}

static CFStringRef __CFRunArrayCopyDescription(CFTypeRef cf) {
    CFRunArrayRef array = (CFRunArrayRef)cf;
    CFRunArrayGuts *guts = array->guts;
    CFIndex cnt;
    CFMutableStringRef string = CFStringCreateMutable(kCFAllocatorSystemDefault, 0);
    CFStringAppendFormat(string, NULL, CFSTR("%ld blocks used, total length %ld (%ld blocks, block %ld is at %ld)\n"), (long)guts->numBlocks, (long)guts->length, (long)guts->maxBlocks, (long)guts->cachedBlock, (long)guts->cachedLocation);
    for (cnt = 0; cnt < guts->numBlocks; cnt++) CFStringAppendFormat(string, NULL, CFSTR(" %ld %p %@\n"), (long)guts->list[cnt].length, guts->list[cnt].obj, guts->list[cnt].obj);
    return string;
}

static void __CFRunArrayDeallocate(CFTypeRef cf) {
    CFRunArrayRef array = (CFRunArrayRef)cf;
    CFRunArrayGuts *guts = array->guts;
    
    RLEARRAYLOCK;
    if (guts->numRefs <= 1) {
        RLEARRAYUNLOCK;
        CFIndex cnt;
        for (cnt = 0; cnt < guts->numBlocks; cnt++) {
            FREE(guts->list[cnt].obj);
        }
        CFAllocatorDeallocate(CFGetAllocator(array), guts);
    } else {
        guts->numRefs--;
        RLEARRAYUNLOCK;
    }
}

const CFRuntimeClass __CFRunArrayClass = {
    0,
    "CFRunArray",
    NULL,	// init
    NULL,	// copy
    __CFRunArrayDeallocate,
    __CFRunArrayEqual,
    __CFRunArrayHash,
    NULL,	// 
    __CFRunArrayCopyDescription
};

CFTypeID CFRunArrayGetTypeID(void) {
    return _kCFRuntimeIDCFRunArray;
}


/*** CFRunArray ***/

CFRunArrayRef _CFRunArrayCreateWithGuts(CFAllocatorRef allocator, struct _CFRunArrayGuts *runArrayGuts) {
    CFIndex size = sizeof(struct __CFRunArray) - sizeof(CFRuntimeBase);
    CFRunArrayRef array = (CFRunArrayRef)_CFRuntimeCreateInstance(allocator, CFRunArrayGetTypeID(), size, NULL);
    if (array == NULL) return NULL;

    if (runArrayGuts) {
        array->guts = runArrayGuts;
        RLEARRAYLOCK;
        array->guts->numRefs++;
        RLEARRAYUNLOCK;
    } else {
        __CFRunArrayMakeNewList(array, NULL);
    }

    return array;
}


CFRunArrayRef CFRunArrayCreate(CFAllocatorRef allocator) {
    CFRunArrayRef array = _CFRunArrayCreateWithGuts(allocator, NULL);
    return array;
}

CFIndex CFRunArrayGetCount(CFRunArrayRef array) {
    return array->guts->length;
}

CFTypeRef CFRunArrayGetValueAtIndex(CFRunArrayRef array, CFIndex loc, CFRange *effectiveRange, CFIndex *blockIndexPtr) {
    // ??? if (loc >= array->guts->length) BoundsError;
    CFIndex blockIndex = blockForLocation(array->guts, loc, effectiveRange);
    if (blockIndexPtr) *blockIndexPtr = blockIndex;
    return array->guts->list[blockIndex].obj;
}

CFTypeRef CFRunArrayGetValueAtRunArrayIndex(CFRunArrayRef array, CFIndex blockIndex, CFIndex *lengthPtr) {
    if (blockIndex >= array->guts->numBlocks) return NULL;
    if (lengthPtr) *lengthPtr = array->guts->list[blockIndex].length;
    return array->guts->list[blockIndex].obj;
}

void CFRunArrayInsert(CFRunArrayRef array, CFRange range, CFTypeRef obj) {
    CFRunArrayGuts *guts = array->guts;
    
    // ??? if (range.location > guts->length) BoundsError;
    if (range.length == 0) return;

    RLEARRAYLOCK;
    if (guts->numRefs > 1) {
	__CFRunArrayMakeNewList(array, guts); 
	guts = array->guts;	// Refresh
    }
    RLEARRAYUNLOCK;
    
    
    if (range.location == guts->length) {	// Append
	if (guts->length > 0) {	// The list isn't empty
	    if (ISSAME(obj, guts->list[guts->numBlocks - 1].obj)) {
		guts->list[guts->numBlocks - 1].length += range.length;
		// We might have invalidated the cached info, fix it up!
		if (guts->cachedBlock >= guts->numBlocks) guts->cachedLocation += range.length;
	    } else {
                __CFRunArraySetBlockCapacity(array, guts->numBlocks + 1);
		guts = array->guts; // Recache
		guts->list[guts->numBlocks].obj = EXTERNCOPY(obj);
		guts->list[guts->numBlocks].length = range.length;
		guts->numBlocks++;
	    }
	} else {	// The list is empty...
            __CFRunArraySetBlockCapacity(array, 1);
	    guts = array->guts; // Recache
	    guts->list[0].obj = EXTERNCOPY(obj);
	    guts->list[0].length = range.length;
	    guts->numBlocks++;
	}
    } else {	// At this stage we are inserting, and the length of the list is > 0.
	CFRange blockRange;
	CFIndex cnt, block = blockForLocation(guts, range.location, &blockRange);
	if (ISSAME(obj, guts->list[block].obj)) {
	    guts->list[block].length += range.length;
	} else if ((block > 0) && (blockRange.location == range.location) && (ISSAME(obj, guts->list[block - 1].obj))) {
	    guts->list[block - 1].length += range.length;
	    // We might have invalidated the cached info, fix it up!
	    if (guts->cachedBlock >= block) guts->cachedLocation += range.length;
	} else if (blockRange.location == range.location) {
            __CFRunArraySetBlockCapacity(array, guts->numBlocks + 1);
	    guts = array->guts; // Recache
	    for (cnt = guts->numBlocks; cnt > block; cnt--) guts->list[cnt] = guts->list[cnt - 1];
	    guts->list[block].obj = EXTERNCOPY(obj);
	    guts->list[block].length = range.length;
	    guts->numBlocks++;
	} else {
            __CFRunArraySetBlockCapacity(array, guts->numBlocks + 2);
	    guts = array->guts; // Recache
	    for (cnt = guts->numBlocks + 1; cnt >= block + 2; cnt--) guts->list[cnt] = guts->list[cnt - 2];
	    guts->list[block + 1].obj = EXTERNCOPY(obj);
	    guts->list[block + 1].length = range.length;
	    guts->list[block].length = (range.location - blockRange.location);
	    guts->list[block + 2].obj = COPY(guts->list[block].obj);
	    guts->list[block + 2].length = blockRange.length - (range.location - blockRange.location);
	    guts->numBlocks += 2;
	}
    }
    guts->length += range.length;
}

void CFRunArrayDelete(CFRunArrayRef array, CFRange range) {
    CFRunArrayReplace(array, range, NULL, 0);
}

void CFRunArrayReplace(CFRunArrayRef array, CFRange range, CFTypeRef newObject, CFIndex newLength) {
    CFRunArrayGuts *guts = array->guts;
    CFRange blockRange;
    CFIndex block, toBeDeleted, firstEmptyBlock, lastEmptyBlock;

    // ??? if (range.location + range.length > guts->length) BoundsError;
    if (range.length == 0) return;

    if (newLength == 0) newObject = NULL;
    
    RLEARRAYLOCK;
    if (guts->numRefs > 1) {
	__CFRunArrayMakeNewList(array, guts);
	guts = array->guts;	// Refresh
    }
    RLEARRAYUNLOCK;
    
    /* This call also sets the cache to point to this block */
    block = blockForLocation(guts, range.location, &blockRange);
    guts->length -= range.length;

    /* Figure out how much to delete from this block */
    toBeDeleted = blockRange.length - (range.location - blockRange.location);
    if (toBeDeleted > range.length) toBeDeleted = range.length;

    /* Delete that count */
    if ((guts->list[block].length -= toBeDeleted) == 0) FREE(guts->list[block].obj);
    range.length -= toBeDeleted;
    firstEmptyBlock = (guts->list[block].length == 0) ? block : block + 1;

    while (range.length) {
	block++;
	toBeDeleted = range.length;
	if (toBeDeleted >= guts->list[block].length) toBeDeleted = guts->list[block].length;
	if ((guts->list[block].length -= toBeDeleted) == 0) FREE(guts->list[block].obj);
	range.length -= toBeDeleted;
    }

    lastEmptyBlock = (block == 0 || guts->list[block].length == 0) ? block : block - 1;

    if (firstEmptyBlock <= lastEmptyBlock) {	/* Do we have any blocks that need to be removed? */
        /* One assumption at this point is that the cache isn't beyond firstEmptyBlock */
        if ((firstEmptyBlock > 0) && (firstEmptyBlock == guts->cachedBlock)) {	/* Update the cachedBlock it's pointing at the hole */
            guts->cachedLocation -= guts->list[firstEmptyBlock - 1].length;
            guts->cachedBlock--;	/* Simply make the cached block be the previous one */
        }
        if (newObject) {	/* See if the new object can be merged with one of the end blocks */
            if ((firstEmptyBlock > 0) && ISSAME(guts->list[firstEmptyBlock - 1].obj, newObject)) {
                guts->list[firstEmptyBlock - 1].length += newLength;
                guts->length += newLength;
		newObject = NULL;
            } else if ((lastEmptyBlock + 1 < guts->numBlocks) && ISSAME(guts->list[lastEmptyBlock + 1].obj, newObject)) {
                guts->list[lastEmptyBlock + 1].length += newLength;
                guts->length += newLength;
                newObject = NULL;
            }
        }
        if (!newObject && (firstEmptyBlock > 0) && (lastEmptyBlock + 1 < guts->numBlocks) && ISSAME(guts->list[firstEmptyBlock - 1].obj, guts->list[lastEmptyBlock + 1].obj)) {	/* Can we merge the blocks around the empty space? */
	    /* Often the cachedBlock will point before the firstEmptyBlock; however, if it happens to be pointing at the firstEmptyBlock, we need to move it back. */
	    lastEmptyBlock++;
	    guts->list[firstEmptyBlock - 1].length += guts->list[lastEmptyBlock].length;
	    FREE(guts->list[lastEmptyBlock].obj);
	}
        if (newObject && (firstEmptyBlock < guts->numBlocks) /* Sanity check */) {	/* Slip this into the first empty block */
            guts->list[firstEmptyBlock].obj = EXTERNCOPY(newObject);
            guts->list[firstEmptyBlock].length = newLength;
            guts->length += newLength;
            firstEmptyBlock++;
            newObject = NULL;
        }
        if (firstEmptyBlock <= lastEmptyBlock) {	/* Do we still need to shift anything over? */
            CFIndex toBeMoved = guts->numBlocks - lastEmptyBlock - 1;
            CFIndex firstFullBlock = lastEmptyBlock + 1;
            for (block = 0; block < toBeMoved; block++) {
                guts->list[firstEmptyBlock + block] = guts->list[firstFullBlock + block];
            }
            guts->numBlocks -= (firstFullBlock - firstEmptyBlock);
            __CFRunArraySetBlockCapacity(array, guts->numBlocks);
        }
    }
    if (newObject) {	/* If this is still set, that means we didn't get to insert it... Do it now. */
        CFRunArrayInsert(array, CFRangeMake(range.location, newLength), newObject);
    }
}



