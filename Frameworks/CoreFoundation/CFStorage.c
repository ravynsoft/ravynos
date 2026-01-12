/*
 * Copyright (c) 2015 Apple Inc. All rights reserved.
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

/*	CFStorage.c
 Copyright (c) 1999-2014, Apple Inc. All rights reserved.
 Responsibility: Ali Ozer
 */

/*
 2-3 tree storing arbitrary sized values.
 
 ??? Currently elementSize cannot be greater than storage->maxLeafCapacity, which is less than or equal to __CFStorageMaxLeafCapacity
 
 CFStorage is thread-safe for multiple readers, but not thread safe for simultaneous reading and  writing.
 */


/* pammon notes on FrozenStorage
 A CFStorage can be frozen, or more specifically, some or all of the nodes can be frozen.  Frozen nodes can be safely shared between CFStorage instances.  CFStorages containing frozen nodes are still considered mutable: frozen nodes are discarded and recreated on demand.  It is a form of copy-on-write.
 
 Freezing is usually one-way: there is no going back.   However, if a node's reference count is 1, we know that no other CFStorage can reference it; and if we are in a mutating function, we know that it can be safely unfrozen.
 
 If a node is frozen, all of its descendants should be considered frozen.  
 
 The root node, which is defined inline within the CFStorage struct itself, can never be frozen.
 
 */

#define NO_SHIFTER ((uint32_t)(-1))

#include <CoreFoundation/CFStorage.h>
#include "CFInternal.h"
#if DEPLOYMENT_TARGET_MACOSX || DEPLOYMENT_TARGET_EMBEDDED || DEPLOYMENT_TARGET_WINDOWS
#include <dispatch/dispatch.h>
#endif

#if DEPLOYMENT_TARGET_WINDOWS
// No C99 support
#define restrict

// Replace bzero
#define bzero(dst, size)    ZeroMemory(dst, size)

#endif

#if !defined(PAGE_SIZE)
#define PAGE_SIZE 4096
#endif

// Above 15K, malloc switches (??? or used to in early Leopard) to using vm allocates for blocks; it seems best to avoid that.
// Also, tests with StorageTimer.c done in 4/07 indicate that 4096 * 3 is better than smaller or larger node sizes.
#define __CFStorageMaxLeafCapacity (4096 * 3)

#define COPYMEM(src,dst,n) objc_memmove_collectable((dst), (src), (n))
#define PAGE_LIMIT ((CFIndex)PAGE_SIZE / 2)

CF_INLINE int32_t roundToPage(int32_t num) {
    return (num + PAGE_SIZE - 1) & ~(PAGE_SIZE - 1);
}

typedef const struct __CFStorage *ConstCFStorageRef;

/* Each node in the storage.  isLeaf determines whether the node is a leaf node or a node inside the tree. If latter, number of children are determined by the number of non-NULL entries in child[]. (NULL entries are at the end.)
 */
typedef struct __CFStorageNode {
    CFIndex numBytes;	/* Number of actual bytes in this node and all its children */
    uint32_t refCount;    /* Refcount of the node.  Is always at least 1 for a normal node.  For root nodes, which are stored directly in the CFStorage, this is 0.  CFStorageRetain/ReleaseNode checks for a refcount of 0 and does not retain/release such nodes. */
    bool isFrozen;	    /* Indicates that the node is frozen, i.e. may be shared. */
    bool isLeaf;
    union {
        struct {
            CFIndex capacityInBytes;	// capacityInBytes is capacity of memory; this is either 0, or >= numBytes
            uint8_t *memory;
	    CFRange cachedRange;       //the absolute range of this node, in "value" units.  This is valid only if this node is referenced by storage->cacheNode, and used by the cache.  In general this is not valid, and the offset needs to be passed down from the tree
        } leaf;
        struct {
            struct __CFStorageNode *child[3];
        } notLeaf;
    } info;
} CFStorageNode;


/* A struct used for insertion into frozen nodes, which may need to return a new version of themselves, and a new friend!  The child field is a replacement for the child that was inserted into; if the child does not need to be replaced (i.e. it was unfrozen and there was space to perform the insertion) then the child that was inserted into is returned here.  The sibling field is a new child that should also be inserted (or NULL if none).  */
typedef struct {
    CFStorageNode *child;
    CFStorageNode *sibling;
} CFStorageDoubleNodeReturn;

CF_INLINE CFStorageDoubleNodeReturn CFStorageDoubleNodeReturnMake(CFStorageNode *child, CFStorageNode *sibling) {
    CFStorageDoubleNodeReturn v;
    v.child = child;
    v.sibling = sibling;
    return v;
}

/* The CFStorage object.
 */
struct __CFStorage {
    CFRuntimeBase base;
    CFIndex valueSize;
    uint32_t byteToValueShifter;
    CFLock_t cacheReaderMemoryAllocationLock;
    bool alwaysFrozen;
    CFStorageNode * volatile cacheNode;
    CFIndex maxLeafCapacity;	    // In terms of bytes
    CFStorageNode rootNode;
    CFOptionFlags nodeHint;	    // __kCFAllocatorGCScannedMemory or 0.
};

/* Helper function to return the intersection of two ranges */
static inline CFRange intersectionRange(CFRange a, CFRange b) {
    CFIndex start = __CFMax(a.location, b.location);
    CFIndex end = __CFMin(a.location + a.length, b.location + b.length);
    if (end <= start) {
	return CFRangeMake(0, 0);
    }
    else {
	return CFRangeMake(start, end - start);
    }
}

/* Allocates the memory and initializes the capacity in a leaf.   This locks not for mutations (mutations are not thread-safe in general), but for lazy allocation of storage during reading.
 */
CF_INLINE void __CFStorageAllocLeafNodeMemory(CFAllocatorRef allocator, CFStorageRef storage, CFStorageNode *node, CFIndex cap, bool compact) {
    if (cap > PAGE_LIMIT) {
        cap = roundToPage(cap);
	if (cap > storage->maxLeafCapacity) cap = storage->maxLeafCapacity;
    } else {
        cap = (((cap + 63) / 64) * 64);
    }
    /* We must be careful here, because another thread may be trying to allocate this memory at the same time (8203146).  This may happen if two threads both attempt to read from a lazily-allocated node. */
    if ((compact ? (cap != node->info.leaf.capacityInBytes) : (cap > node->info.leaf.capacityInBytes))) {
	__CFLock(&(storage->cacheReaderMemoryAllocationLock));
	/* Check again now that we've acquired the lock.  We know that we can do this because two simulaneous readers will always pass the same capacity.  This is the fix for 8203146.  This probably needs a memory barrier. */
	if ((compact ? (cap != node->info.leaf.capacityInBytes) : (cap > node->info.leaf.capacityInBytes))) {
	    __CFAssignWithWriteBarrier((void **)&node->info.leaf.memory, _CFAllocatorReallocateGC(allocator, node->info.leaf.memory, cap, storage->nodeHint));	// This will free...
	    if (__CFOASafe) __CFSetLastAllocationEventName(node->info.leaf.memory, "CFStorage (node bytes)");
	    node->info.leaf.capacityInBytes = cap;
	}
	__CFUnlock(&(storage->cacheReaderMemoryAllocationLock));	    
    }
}

#if 0
#define ASSERT(x) do { if (! (x)) { fprintf(stderr, "Assertion %s failed on line %d\n", #x, __LINE__); HALT; } } while (0)
#else
#define ASSERT(x) do { if (0 && ! (x)) { } } while(0)
#endif

static void __CFStorageCheckIntegrity(CFStorageRef storage);
#define CHECK_INTEGRITY() do { if (0) __CFStorageCheckIntegrity(storage); } while (0)

static void __CFStorageCheckNodeIntegrity(ConstCFStorageRef storage, const CFStorageNode *node);
#define CHECK_NODE_INTEGRITY(X) do { if (0) __CFStorageCheckNodeIntegrity(storage, X); } while (0)

#pragma mark Byte <-> Value Functions

CF_INLINE CFIndex __CFStorageConvertByteToValue(ConstCFStorageRef storage, CFIndex byte) {
    if (storage->byteToValueShifter != NO_SHIFTER) {
	return byte >> storage->byteToValueShifter;
    }
    return byte / storage->valueSize;
}

CF_INLINE CFRange __CFStorageConvertBytesToValueRange(ConstCFStorageRef storage, CFIndex offset, CFIndex length) {
    if (storage->byteToValueShifter != NO_SHIFTER) {
	return CFRangeMake(offset >> storage->byteToValueShifter, length >> storage->byteToValueShifter);
    }
    return CFRangeMake(offset / storage->valueSize, length / storage->valueSize);
}

CF_INLINE CFIndex __CFStorageConvertValueToByte(ConstCFStorageRef storage, CFIndex value) {
    if (storage->byteToValueShifter != NO_SHIFTER) {
	return value << storage->byteToValueShifter;
    }
    return value * storage->valueSize;
}

CF_INLINE CFRange __CFStorageConvertValuesToByteRange(ConstCFStorageRef storage, CFIndex offset, CFIndex length) {
    if (storage->byteToValueShifter != NO_SHIFTER) {
	return CFRangeMake(offset << storage->byteToValueShifter, length << storage->byteToValueShifter);
    }
    return CFRangeMake(offset * storage->valueSize, length * storage->valueSize);
}


#pragma mark Node reference counting and freezing

CF_INLINE CFStorageNode *__CFStorageRetainNode(CFStorageNode *node) {
    if (node->refCount > 0) OSAtomicIncrement32((int32_t *)&node->refCount);
    return node;
}

/* A faster version of __CFStorageRetainNode that is not thread safe.  This can be used from the Unfrozen (aka unshared) calls, or when a node was just allocated and there's no opportunity for it to have escaped yet */
CF_INLINE CFStorageNode *__CFStorageRetainNodeThreadUnsafe(CFStorageNode *node) {
    if (node->refCount > 0) node->refCount++;
    return node;
}

static void __CFStorageDeallocateNode(CFStorageRef storage, CFStorageNode *node);

CF_INLINE void __CFStorageReleaseNode(CFStorageRef storage, CFStorageNode *node) {
    if (node->refCount > 0) {
	uint32_t newRefCount = OSAtomicDecrement32((int32_t *)&node->refCount);
	if (newRefCount == 0) {
	    __CFStorageDeallocateNode(storage, node);
	}
    }
}

CF_INLINE void __CFStorageReleaseNodeWithNullCheck(CFStorageRef storage, CFStorageNode *node) {
    if (node) __CFStorageReleaseNode(storage, node);
}

static void __CFStorageDeallocateNode(CFStorageRef storage, CFStorageNode *node) {
    CFAllocatorRef allocator = CFGetAllocator(storage);
    if (node->isLeaf) {
	if (node->info.leaf.memory) _CFAllocatorDeallocateGC(allocator, node->info.leaf.memory);
    } else {
	__CFStorageReleaseNodeWithNullCheck(storage, node->info.notLeaf.child[0]);
	__CFStorageReleaseNodeWithNullCheck(storage, node->info.notLeaf.child[1]);
	__CFStorageReleaseNodeWithNullCheck(storage, node->info.notLeaf.child[2]);
    }
    _CFAllocatorDeallocateGC(allocator, node);
}

static inline void __CFStorageFreezeNode(CFStorageNode *node) {
    node->isFrozen = true;
}

/* If a node is frozen, but its reference count is 1, then we are the only reference to it and we can unfreeze it.  In general, it's unsafe to do this because it may race with other threads that depend on it being frozen (e.g. we are about to copy the node).  However, we do not support concurrent access while mutating a CFStorage, so if we are in a mutation function, know we have exclusive access and we can unfreeze the node. */
static inline bool __CFStorageThawNodeDuringMutation(CFStorageRef storage, CFStorageNode *node) {
    if (node->refCount == 1) {
	node->isFrozen = false;
	return true;
    }
    return false;
}

static inline void __CFStorageSetChild(CFStorageNode *parentNode, CFIndex childIndex, CFStorageNode *newChild) {
    ASSERT(! parentNode->isLeaf);
    ASSERT(childIndex < 3);
    __CFAssignWithWriteBarrier((void **)&parentNode->info.notLeaf.child[childIndex], newChild);
}

static inline void __CFStorageGetChildren(const CFStorageNode *parent, CFStorageNode ** restrict resultArray, bool shouldRetain, bool shouldFreeze) {
    ASSERT(! parent->isLeaf);
    CFIndex i;
    for (i=0; i < 3; i++) {
	CFStorageNode *node = parent->info.notLeaf.child[i];
	if (node != NULL && shouldRetain) __CFStorageRetainNode(node);
	if (node != NULL && shouldFreeze) __CFStorageFreezeNode(node);
	resultArray[i] = node;
    }
}

#pragma mark Storage cache handling


/* Sets the cache to point at the specified node. loc and len are in terms of values, not bytes. To clear the cache set these two to 0.
 At least one of node or memory should be non-NULL. memory is consulted first when using the cache.
 */
CF_INLINE void __CFStorageSetCache(CFStorageRef storage, CFStorageNode *node, CFIndex locInBytes) {    
    if (node) {
	ASSERT(node->isLeaf);
	node->info.leaf.cachedRange = __CFStorageConvertBytesToValueRange(storage, locInBytes, node->numBytes);
    }
    storage->cacheNode = node;
}

/* Gets the location for the specified absolute loc from the cached info.
 Returns NULL if the location is not in the cache.
 */
CF_INLINE uint8_t *__CFStorageGetFromCache(CFStorageRef storage, CFIndex loc, CFRange * restrict validConsecutiveValueRange, bool requireUnfrozenNode) {
    CFStorageNode * const cachedNode = storage->cacheNode; /* It's important we read from this field no more than once, for thread safety with other concurrent reads; that is why the field is marked volatile. */
    if (! cachedNode) return NULL; /* No cache */
    
    /* We only allow caching leaf nodes. */
    ASSERT(cachedNode->isLeaf);
    
    /* If the node is frozen, and we require an unfrozen node, then return NULL */
    if (requireUnfrozenNode && cachedNode->isFrozen) return NULL;
    
    /* If there's no memory allocated yet, then allocate it now*/
    if (! cachedNode->info.leaf.memory) {
	__CFStorageAllocLeafNodeMemory(CFGetAllocator(storage), storage, cachedNode, cachedNode->numBytes, false);
    }
    
    /* If the node's range starts after loc, or ends before or at loc, return NULL */
    CFIndex nodeOffset = cachedNode->info.leaf.cachedRange.location;
    CFIndex nodeLength = cachedNode->info.leaf.cachedRange.length;
    if (loc < nodeOffset || loc >= nodeOffset + nodeLength) {
	return NULL;
    }
    
    /* The cache is valid, so return it */
    validConsecutiveValueRange->location = nodeOffset;
    validConsecutiveValueRange->length = nodeLength;
    uint8_t *result = cachedNode->info.leaf.memory + __CFStorageConvertValueToByte(storage, loc - nodeOffset);
    return result;
}


/* Returns the number of the child containing the desired value and the relative index of the value in that child.
 forInsertion = true means that we are looking for the child in which to insert or delete; this changes the behavior when the index is at the end of a child
 relativeByteNum (not optional, for performance reasons) returns the relative byte number of the specified byte in the child.
 Don't call with leaf nodes!
 */
CF_INLINE CFStorageNode *__CFStorageFindChild(const CFStorageNode * restrict node, CFIndex byteNum, bool forInsertionOrDeletion, CFIndex * restrict childNum, CFIndex * restrict relativeByteNum) {
    if (forInsertionOrDeletion) byteNum--;	/* If for insertion, we do <= checks, not <, so this accomplishes the same thing */
    CFStorageNode *result;
    result = node->info.notLeaf.child[0];
    if (byteNum < result->numBytes) *childNum = 0;
    else {
	byteNum -= result->numBytes;
	result = node->info.notLeaf.child[1];
        if (byteNum < result->numBytes) *childNum = 1;
        else {
            byteNum -= result->numBytes;
            *childNum = 2;
	    result = node->info.notLeaf.child[2];
        }
    }
    if (forInsertionOrDeletion) byteNum++;
    *relativeByteNum = byteNum;
    return result;
}

static CFStorageNode *__CFStorageCopyNode(CFStorageRef storage, const CFStorageNode *node);

/* Finds the location where the specified byte is stored. If validConsecutiveByteRange is not NULL, returns
 the range of bytes that are consecutive with this one.
 !!! Assumes the byteNum is within the range of this node.
 */
static void *__CFStorageFindByte(CFStorageRef storage, CFStorageNode *node, CFIndex byteNum, CFIndex absoluteByteOffsetOfNode, CFStorageNode **resultNode, CFRange *validConsecutiveByteRange, bool requireUnfreezing) {
    if (node->isLeaf) {
        *validConsecutiveByteRange = CFRangeMake(absoluteByteOffsetOfNode, node->numBytes);
	*resultNode = node;
        __CFStorageAllocLeafNodeMemory(CFGetAllocator(storage), storage, node, node->numBytes, false);
        return node->info.leaf.memory + byteNum;
    } else {
        CFIndex childNum;
        CFIndex relativeByteNum;
        CFStorageNode *child = __CFStorageFindChild(node, byteNum, false, &childNum, &relativeByteNum);
	if (requireUnfreezing && child->isFrozen && ! __CFStorageThawNodeDuringMutation(storage, child)) {
	    /* Replace the child with an unfrozen variant */
	    CFStorageNode *unfrozenReplacement = __CFStorageCopyNode(storage, child);
	    /* Release the old node, set the new one */
	    __CFStorageSetChild(node, childNum, unfrozenReplacement);
	    __CFStorageReleaseNode(storage, child);
	    child = unfrozenReplacement;
	}
        return __CFStorageFindByte(storage, child, relativeByteNum, absoluteByteOffsetOfNode + (byteNum - relativeByteNum), resultNode, validConsecutiveByteRange, requireUnfreezing);
    }
}

/* Guts of CFStorageGetValueAtIndex(); note that validConsecutiveValueRange is not optional.
 Consults and updates cache.
 */
CF_INLINE void *__CFStorageGetValueAtIndex(CFStorageRef storage, CFIndex idx, CFRange *validConsecutiveValueRange, bool requireUnfreezing) {
    uint8_t *result;
    if (!(result = __CFStorageGetFromCache(storage, idx, validConsecutiveValueRange, requireUnfreezing))) {
	CFStorageNode *resultNode;
	CFRange rangeInBytes;
	result = (uint8_t *)__CFStorageFindByte(storage, &storage->rootNode, __CFStorageConvertValueToByte(storage, idx), 0, &resultNode, &rangeInBytes, requireUnfreezing);
        __CFStorageSetCache(storage, resultNode, rangeInBytes.location);
	*validConsecutiveValueRange = __CFStorageConvertBytesToValueRange(storage, rangeInBytes.location, rangeInBytes.length);
    }
    return result;
}

/* Copies data in the range srcRange from srcLeaf to index dstLocation in dstLeaf.  Both srcLeaf and dstLeaf must be leaf nodes, and dstLeaf must not be frozen.  If srcRange has a nonzero length, then both must have their memory properly allocated.  This does not modify the numBytes of srcLeaf or dstLeaf.
 */
static void __CFLeafCopyRangeToOffset(const CFStorageNode *srcLeaf, CFRange srcRange, CFStorageNode *dstLeaf, CFIndex dstLocation) {
    ASSERT(srcLeaf->isLeaf);
    ASSERT(dstLeaf->isLeaf);
    if (srcRange.length > 0) {
	ASSERT(srcLeaf->info.leaf.memory != NULL);
	ASSERT(dstLeaf->info.leaf.memory != NULL);
	ASSERT(srcRange.location + srcRange.length <= srcLeaf->numBytes);
	ASSERT(dstLocation + srcRange.length <= dstLeaf->info.leaf.capacityInBytes);
	COPYMEM(srcLeaf->info.leaf.memory + srcRange.location, dstLeaf->info.leaf.memory + dstLocation, srcRange.length);
    }
}

#pragma mark Node creation and destruction

// returns a node with a refCount of 1, and an auto_zone_retain() under GC
static CFStorageNode *__CFStorageCreateNode(CFAllocatorRef allocator, CFStorageRef storage, bool isLeaf, CFIndex numBytes) {
    CFStorageNode *newNode = (CFStorageNode *)CFAllocatorAllocate(allocator, sizeof(CFStorageNode), __kCFAllocatorGCScannedMemory);
    if (__CFOASafe) __CFSetLastAllocationEventName(newNode, "CFStorage (node)");
    if (CF_IS_COLLECTABLE_ALLOCATOR(allocator)) {
	auto_zone_release(objc_collectableZone(), newNode); //remove the implicit retain so we can be collected
    }
    newNode->refCount = 1;
    newNode->isFrozen = storage->alwaysFrozen;
    newNode->isLeaf = isLeaf;
    newNode->numBytes = numBytes;
    if (isLeaf) {
        newNode->info.leaf.capacityInBytes = 0;
        newNode->info.leaf.memory = NULL;
    } else {
        newNode->info.notLeaf.child[0] = newNode->info.notLeaf.child[1] = newNode->info.notLeaf.child[2] = NULL;
    }
    return newNode;
}

/* Creates an (unfrozen) copy of the given node.  This is shallow in the sense that it shares children for branches, but deep in that it copies memory for leaves.  */
static CFStorageNode *__CFStorageCopyNode(CFStorageRef storage, const CFStorageNode *node) {
    CFAllocatorRef allocator = CFGetAllocator(storage);
    CFStorageNode *result = __CFStorageCreateNode(allocator, storage, node->isLeaf, node->numBytes);
    if (node->isLeaf) {
	if (node->info.leaf.memory != NULL) {
	    __CFStorageAllocLeafNodeMemory(allocator, storage, result, result->numBytes, false);
	    COPYMEM(node->info.leaf.memory, result->info.leaf.memory, result->numBytes);
	}
    }
    else {
	CFStorageNode *child = node->info.notLeaf.child[0];
	__CFStorageSetChild(result, 0, __CFStorageRetainNode(child));
	if ((child = node->info.notLeaf.child[1])) __CFStorageSetChild(result, 1, __CFStorageRetainNode(child));
	if ((child = node->info.notLeaf.child[2])) __CFStorageSetChild(result, 2, __CFStorageRetainNode(child));
	
	/* If we are copying children from a frozen node to an unfrozen node, we need to freeze the children */
	if (node->isFrozen) {
	    __CFStorageFreezeNode(result->info.notLeaf.child[0]);
	    if ((child = result->info.notLeaf.child[1])) __CFStorageFreezeNode(child);
	    if ((child = result->info.notLeaf.child[2])) __CFStorageFreezeNode(child);
	}
    }
    return result;
}


static void __CFStorageDeallocateNode(CFStorageRef storage, CFStorageNode *node);


#pragma mark Insertion and Deletion prototypes
/* Prototypes for deletion and insertion.  The *Frozen and *Unfrozen variants should only be called for nodes that we know are frozen or unfrozen.  Frozen nodes may only have frozen children, so it makes sense for the Frozen functions to call other Frozen functions.  Unfrozen nodes may have frozen or unfrozen children, so they should call the non-suffixed variants (which dispatch on whether the node is frozen or not).  
 
 The only acceptable time to directly call the Unfrozen variant is for the root node of a CFStorage, because root nodes may never be frozen.  The isRootNode parameter determines whether we are in this case.
 
 The Insertion functions return two nodes.  As an awful performance hack, if the first node returned from __CFStorageInsert* is the same as the node passed in, that node is *not* retained, so should not be relased.  If it is a different node, it is retained.
 */
static CFStorageDoubleNodeReturn __CFStorageInsert(CFAllocatorRef allocator, CFStorageRef storage, CFStorageNode *node, CFIndex byteNum, CFIndex size, CFIndex absoluteByteNum);
static CFStorageNode *__CFStorageDelete(CFAllocatorRef allocator, CFStorageRef storage, CFStorageNode *node, CFRange range, bool compact);

static CFStorageDoubleNodeReturn __CFStorageInsertFrozen(CFAllocatorRef allocator, CFStorageRef storage, const CFStorageNode *node, CFIndex byteNum, CFIndex size, CFIndex absoluteByteNum);
static CFStorageNode *__CFStorageDeleteFrozen(CFAllocatorRef allocator, CFStorageRef storage, const CFStorageNode *node, CFRange range);

static CFStorageDoubleNodeReturn __CFStorageInsertUnfrozen(CFAllocatorRef allocator, CFStorageRef storage, CFStorageNode *node, CFIndex byteNum, CFIndex size, CFIndex absoluteByteNum);
static CFStorageNode *__CFStorageDeleteUnfrozen(CFAllocatorRef allocator, CFStorageRef storage, CFStorageNode *node, CFRange range, bool compact, bool isRootNode);

#pragma mark Frozen Deletion

static CFStorageNode *__CFStorageDeleteLeafFrozen(CFAllocatorRef allocator, CFStorageRef storage, const CFStorageNode *node, CFRange range) {
    ASSERT(node->isLeaf);
    const CFIndex rangeToDeleteEnd = range.location + range.length;
    ASSERT(rangeToDeleteEnd <= node->numBytes);
    CFIndex remainingBytes = node->numBytes - range.length;
    if (remainingBytes == 0) {
	/* The range to delete encompasses our entire range of bytes.  Return NULL to indicate that we should be deleted. */
	return NULL;
    }
    else {
	/* Need to create a new node */
	CFStorageNode *newNode = __CFStorageCreateNode(allocator, storage, true, remainingBytes);
	if (node->info.leaf.memory) {
	    /* Our node had memory allocated, so copy in the non-deleted portion */
	    CFRange nonDeletedPrefix = CFRangeMake(0, range.location);
	    CFRange nonDeletedSuffix = CFRangeMake(rangeToDeleteEnd, node->numBytes - rangeToDeleteEnd);
	    ASSERT(nonDeletedPrefix.length + nonDeletedSuffix.length == remainingBytes);
	    __CFStorageAllocLeafNodeMemory(allocator, storage, newNode, remainingBytes, false);  // no point in compacting since we're freshly allocated
	    __CFLeafCopyRangeToOffset(node, nonDeletedPrefix, newNode, 0);
	    __CFLeafCopyRangeToOffset(node, nonDeletedSuffix, newNode, nonDeletedPrefix.length);
	}
	return newNode;
    }
}

/* Helper function for both frozen and unfrozen branch deletion.  Walks the children of the node, calling __CFStorageDelete (or __CFStorageDeleteFrozen if childrenAreDefinitelyFrozen is YES), and assigning the results back to newChildren.  Returns the number of new children.  The newChildren nodes all acquire a reference count! 
 */
static inline CFIndex __CFStoragePopulateBranchChildrenAfterDeletion(CFAllocatorRef allocator, CFStorageRef storage, const CFStorageNode *node, CFRange range, CFStorageNode *newChildren[3], bool childrenAreDefinitelyFrozen, bool compact) {
    CFIndex newChildIndex = 0;
    CFIndex childByteOffset = 0; //will track the current start byte of this child
    for (CFIndex existingChildIndex = 0; existingChildIndex < 3; existingChildIndex++) {
	CFStorageNode *existingChild = node->info.notLeaf.child[existingChildIndex];
	if (! existingChild) break; //no more children
	const CFIndex existingChildLength = existingChild->numBytes;
	/* The child's range is {byteOffset, existingChildLength}.  Figure out what part of the range to delete is intersected by this child's range */
	CFRange deletionRangeIntersectedByChild = intersectionRange(range, CFRangeMake(childByteOffset, existingChildLength));
	if (! deletionRangeIntersectedByChild.length) {
	    /* The range to delete does not overlap this child's range, so preserve the child */
	    newChildren[newChildIndex++] = __CFStorageRetainNode(existingChild); //bump the refcount like we promised we would
	    if (childrenAreDefinitelyFrozen) {
		/* Because we are about to add this child from a frozen node to a possibly unfrozen node, mark the child as frozen */
		__CFStorageFreezeNode(existingChild);
	    }
	}
	else {
	    /* We have something from this child to delete */
	    CFRange rangeOfChildToDelete = CFRangeMake(deletionRangeIntersectedByChild.location - childByteOffset, deletionRangeIntersectedByChild.length);
	    CFStorageNode *newChild;
	    if (childrenAreDefinitelyFrozen) {
		newChild = __CFStorageDeleteFrozen(allocator, storage, existingChild, rangeOfChildToDelete);
	    }
	    else {
		newChild = __CFStorageDelete(allocator, storage, existingChild, rangeOfChildToDelete, compact);
	    }
	    /* We may get null back if we deleted the entire child */
	    if (newChild != NULL) {
		newChildren[newChildIndex++] = newChild; // Transfers the reference count
	    }
	    
	    if (rangeOfChildToDelete.length == existingChildLength) {
		ASSERT(newChild == NULL); //should have deleted everything
	    }
	    else {
		ASSERT(newChild != NULL);
		ASSERT(newChild->numBytes == existingChildLength - rangeOfChildToDelete.length);
	    }
	}
	childByteOffset += existingChildLength;
    }
    return newChildIndex;
}

static CFStorageNode *__CFStorageDeleteBranchFrozen(CFAllocatorRef allocator, CFStorageRef storage, const CFStorageNode *node, CFRange range) {
    ASSERT(! node->isLeaf);
    ASSERT(range.location + range.length <= node->numBytes);
    if (range.length == node->numBytes) {
	/* They're deleting everything, so return NULL to indicate that this node should be deleted. */
	return NULL;
    }
    
    /* Construct our new children in this array. */
    CFStorageNode *newChildren[3];
    CFIndex newChildIndex = __CFStoragePopulateBranchChildrenAfterDeletion(allocator, storage, node, range, newChildren, true/*childrenAreDefinitelyFrozen*/, false/*compact*/);
    
    /* We do not have to freeze anything in newChildren.  __CFStoragePopulateBranchChildrenAfterDeletion() will properly freeze any existing children, and new children we get should not be marked frozen. */
    
    /* Now we have the children of the new node in newChildren.  We expect to have at least one child (if we got no children, we should have returned NULL up above because they deleted everything. */
    ASSERT(newChildIndex >= 1);
    if (newChildIndex == 1) {
	/* Only one child, so just return it, transferring its retain count */
	return newChildren[0];
    }
    else {
	CFStorageNode *result = __CFStorageCreateNode(allocator, storage, false, 0);
	while (newChildIndex--) {
	    __CFStorageSetChild(result, newChildIndex, newChildren[newChildIndex]); //transfers the reference count
	}
	result->numBytes = node->numBytes - range.length;
	return result;
    }
}

/* Returns a new node, or NULL if the entire thing was deleted.
 */
static CFStorageNode *__CFStorageDeleteFrozen(CFAllocatorRef allocator, CFStorageRef storage, const CFStorageNode *node, CFRange range) {
    if (node->isLeaf) {
	return __CFStorageDeleteLeafFrozen(allocator, storage, node, range);
    }
    else {
	return __CFStorageDeleteBranchFrozen(allocator, storage, node, range);
    }
}

#pragma mark Unfrozen Deletion

/* The boolean compact indicates whether leaf nodes that get smaller should be realloced.
 */
static CFStorageNode *__CFStorageDeleteUnfrozen(CFAllocatorRef allocator, CFStorageRef storage, CFStorageNode *node, CFRange range, bool compact, bool isRootNode) {
    ASSERT(! node->isFrozen);
    ASSERT(range.location + range.length <= node->numBytes);
    CHECK_NODE_INTEGRITY(node);
    
    if (range.length == node->numBytes) {
	/* We are deleting everything, so return NULL */
	return NULL;
    }
    
    if (node->isLeaf) {
	node->numBytes -= range.length;
	// If this node had memory allocated, readjust the bytes...
	if (node->info.leaf.memory) {
	    COPYMEM(node->info.leaf.memory + range.location + range.length, node->info.leaf.memory + range.location, node->numBytes - range.location);
	    if (compact) __CFStorageAllocLeafNodeMemory(allocator, storage, node, node->numBytes, true);
	}
	CHECK_NODE_INTEGRITY(node);
	return __CFStorageRetainNodeThreadUnsafe(node); //we can use the ThreadUnsafe calls because this is the Unfrozen variant, so we are not shared
    } else {
	CFStorageNode *newChildren[3] = {NULL, NULL, NULL};
	CFIndex newChildIndex = __CFStoragePopulateBranchChildrenAfterDeletion(allocator, storage, node, range, newChildren, false/*childrenAreDefinitelyFrozen*/, compact);
	node->numBytes -= range.length;
	ASSERT(newChildIndex >= 1); //we expect to have at least one child; else we would have deleted everything up above
	
	/* Release all of our existing children.  Either we are about to return a new child in place of us; or we are about to set our children to the new ones */
	__CFStorageReleaseNode(storage, node->info.notLeaf.child[0]);
	__CFStorageReleaseNodeWithNullCheck(storage, node->info.notLeaf.child[1]);
	__CFStorageReleaseNodeWithNullCheck(storage, node->info.notLeaf.child[2]);
	node->info.notLeaf.child[0] = node->info.notLeaf.child[1] = node->info.notLeaf.child[2] = NULL;
	
	if (newChildIndex == 1) {
	    /* We have only one child, so return it, transferring the refcount that __CFStoragePopulate gives it */
	    return newChildren[0];
	}
	else {
	    CFIndex i;
	    for (i=0; i < 3; i++) {
		__CFStorageSetChild(node, i, newChildren[i]); //set the new child, transferring the refcount to us (or set NULL)
	    }
	    CHECK_NODE_INTEGRITY(node);
	    return __CFStorageRetainNodeThreadUnsafe(node);
	}
    }
}

#pragma mark Frozen Insertion

/* Insertion into an frozen leaf.  We return two nodes, either of which may be 'node', or possibly two new nodes.  This always sets the cache. */
static CFStorageDoubleNodeReturn __CFStorageInsertLeafFrozen(CFAllocatorRef allocator, CFStorageRef storage, const CFStorageNode *node, CFIndex byteNum, CFIndex size, CFIndex absoluteByteNum) {
    /* Inserting into a frozen leaf.  If we can fit the new data along with our existing data into a single node, then do so (i.e. if we can return one node, do it).  Otherwise, all of the data would have to fit into a second node (we are never called with more data than storage->maxLeafCapacity) so just make a new node with the data and return that. */
    CFStorageNode *leftResult, *rightResult;
    CHECK_NODE_INTEGRITY(node);
    ASSERT(byteNum <= node->numBytes);
    CFIndex newTotalSize = size + node->numBytes;
    if (newTotalSize <= storage->maxLeafCapacity) {
	/* We can fit into a single node */
	rightResult = NULL;
	leftResult = __CFStorageCreateNode(allocator, storage, true, newTotalSize);
	if (node->info.leaf.memory != NULL) { // Beware lazy memory allocation
	    __CFStorageAllocLeafNodeMemory(allocator, storage, leftResult, newTotalSize, false);
	    COPYMEM(node->info.leaf.memory, leftResult->info.leaf.memory, byteNum); //copy first byteNum bytes from existing node
	    //middle we don't touch
	    COPYMEM(node->info.leaf.memory + byteNum, leftResult->info.leaf.memory + byteNum + size, node->numBytes - byteNum); //copy last part from existing node
	}
	__CFStorageSetCache(storage, leftResult, absoluteByteNum - byteNum);
    }
    else {
	/* We cannot fit into a single node.  See if we can preserve self (i.e. we're inserting at beginning or end). */
	if (byteNum == node->numBytes) {
	    /* Inserting at end, so left is our existing node and right is the new node.   Do not retain left node, because it is the same as the given node */
	    leftResult = (CFStorageNode *)node;
	    rightResult = __CFStorageCreateNode(allocator, storage, true, size);
	    __CFStorageSetCache(storage, rightResult, absoluteByteNum);
	}
	else if (byteNum == 0) {
	    /* Inserting at beginning, so right is our existing node and left is the new node.  Do retain left node, because it is different than the given node. */
	    rightResult = __CFStorageRetainNode((CFStorageNode *)node);
	    leftResult = __CFStorageCreateNode(allocator, storage, true, size);
	    __CFStorageSetCache(storage, leftResult, absoluteByteNum);
	}
	else {
	    /* Inserting in middle.  We will need to create two nodes because we overflow one.  We could be lazy and only allocate up to byteNum, but since it's likely that they're about to insert into us and we'd have to reallocate, just allocate everything requested up front. */
	    CFIndex leftAmount = storage->maxLeafCapacity, rightAmount = newTotalSize - storage->maxLeafCapacity;
	    leftResult = __CFStorageCreateNode(allocator, storage, true, leftAmount);
	    rightResult = __CFStorageCreateNode(allocator, storage, true, rightAmount);
	    __CFStorageAllocLeafNodeMemory(allocator, storage, leftResult, leftAmount, false);
	    __CFStorageAllocLeafNodeMemory(allocator, storage, rightResult, rightAmount, false);
	    ASSERT(node->info.leaf.capacityInBytes >= node->numBytes);
	    
	    /* The existing node has length node->numBytes, so it has the following range: {0, node->numBytes}
	     
             We are inserting (garbage) data of length 'size' into offset 'byteNum'.  Therefore we end up with the following two logical ranges, expressed as {start, length}:
             {0, byteNum}, {byteNum + size, node->numBytes - byteNum}
	     
             We need to divide these among our new nodes with the following logical ranges:
             {0, leftAmount}, {leftAmount, rightAmount}
	     
             The first range must be fit entirely within the left  node (byteNum <= leftAmount).  The second range may or may be divided between the left and right nodes.     
	     */
	    
	    ASSERT(byteNum <= leftAmount);
	    COPYMEM(node->info.leaf.memory, leftResult->info.leaf.memory, byteNum);
	    
	    const CFRange leftNodeRange = {0, leftAmount}, rightNodeRange = {leftAmount, rightAmount};
	    const CFRange preservedData = {byteNum + size, node->numBytes - byteNum};
	    CFRange overlap;
	    if ((overlap = intersectionRange(leftNodeRange, preservedData)).length > 0) COPYMEM(node->info.leaf.memory + overlap.location - size, leftResult->info.leaf.memory + overlap.location, overlap.length);
	    if ((overlap = intersectionRange(rightNodeRange, preservedData)).length > 0) COPYMEM(node->info.leaf.memory + overlap.location - size, rightResult->info.leaf.memory + overlap.location - leftAmount, overlap.length);
	    __CFStorageSetCache(storage, leftResult, absoluteByteNum - byteNum);
	}
    }
    return CFStorageDoubleNodeReturnMake(leftResult, rightResult);
}

static CFStorageDoubleNodeReturn __CFStorageInsertBranchFrozen(CFAllocatorRef allocator, CFStorageRef storage, const CFStorageNode *node, CFIndex byteNum, CFIndex size, CFIndex absoluteByteNum) {
    /* Inserting into a frozen branch.  We definitely will need to make a new copy of us, so make that up front.  We may or may not need to make a new sibling.  Note that in some cases, we may be able to get away with not creating a new copy of us, e.g. inserting at the very end of the tree.  In that case, we could preserve us and make a sibling containing exactly one node.  However, we do not really want to have branches with exactly one child; because then why not just return the child?  And then the whole tree can become unbalanced.  So then instead, always distribute the children equally among our nodes. */
    CHECK_NODE_INTEGRITY(node);
    CFStorageNode *copyOfMe = __CFStorageCreateNode(allocator, storage, false, 0);
    CFStorageNode *siblingOfMe = NULL;
    CFIndex relativeByteNum;
    CFIndex childNum;
    CFStorageNode *child = __CFStorageFindChild(node, byteNum, true, &childNum, &relativeByteNum);
    ASSERT(childNum >= 0 && childNum <= 2);
    CFStorageDoubleNodeReturn childReturn = __CFStorageInsertFrozen(allocator, storage, child, relativeByteNum, size, absoluteByteNum);
    ASSERT(childReturn.child); //we always get at least one back
    
    /* Make a local array of all new children (retained).  We'll then move them to the new nodes. */
    CFStorageNode *newChildren[4] = {NULL};
    __CFStorageGetChildren(node, newChildren, true/*retain*/, true/*freeze*/);
    if (newChildren[childNum] != childReturn.child) {
	__CFStorageReleaseNode(storage, newChildren[childNum]);
	newChildren[childNum] = childReturn.child; // Transfers the retain
    }
    if (childReturn.sibling != NULL) {
	if (childNum < 2) newChildren[3] = newChildren[2];
	if (childNum < 1) newChildren[2] = newChildren[1];
	newChildren[childNum + 1] = childReturn.sibling; // Transfers the retain
    }
    
    /* First two always go to our clone */
    __CFStorageSetChild(copyOfMe, 0, newChildren[0]);
    __CFStorageSetChild(copyOfMe, 1, newChildren[1]);
    if (newChildren[3] == NULL) {
	/* We have three or fewer children to distribute, i.e. we don't need a sibling.  Put them all into copy of me.  Our clone's byte count is larger than our own by 'size'. */
	__CFStorageSetChild(copyOfMe, 2, newChildren[2]);
	copyOfMe->numBytes = node->numBytes + size;	
    }
    else {
	/* We have four children to distribute.  The first two go to us, the last two go to our new sibling.  */
	siblingOfMe = __CFStorageCreateNode(allocator, storage, false, 0);
	__CFStorageSetChild(siblingOfMe, 0, newChildren[2]);
	__CFStorageSetChild(siblingOfMe, 1, newChildren[3]);
	copyOfMe->numBytes = newChildren[0]->numBytes + newChildren[1]->numBytes;
	siblingOfMe->numBytes = newChildren[2]->numBytes + newChildren[3]->numBytes;
    }
    CHECK_NODE_INTEGRITY(node);
    CHECK_NODE_INTEGRITY(copyOfMe);
    if (siblingOfMe) CHECK_NODE_INTEGRITY(siblingOfMe);
    return CFStorageDoubleNodeReturnMake(copyOfMe, siblingOfMe);
}

static CFStorageDoubleNodeReturn __CFStorageInsertFrozen(CFAllocatorRef allocator, CFStorageRef storage, const CFStorageNode *node, CFIndex byteNum, CFIndex size, CFIndex absoluteByteNum) {
    if (node->isLeaf) {
	return __CFStorageInsertLeafFrozen(allocator, storage, node, byteNum, size, absoluteByteNum); 
    }
    else {
	return __CFStorageInsertBranchFrozen(allocator, storage, node, byteNum, size, absoluteByteNum);
    }
}


#pragma mark Unfrozen Insertion

/* Insertion into an unfrozen leaf.  We return two nodes, one of which is 'node'.  This always sets the cache. */
static CFStorageDoubleNodeReturn __CFStorageInsertLeafUnfrozen(CFAllocatorRef allocator, CFStorageRef storage, CFStorageNode *node, CFIndex byteNum, CFIndex size, CFIndex absoluteByteNum) {
    if (size + node->numBytes > storage->maxLeafCapacity) {	// Need to create more child nodes
	CFStorageNode *newNode;
	if (byteNum == node->numBytes) {	// Inserting at end; easy...
	    newNode = __CFStorageCreateNode(allocator, storage, true, size);
	    __CFStorageSetCache(storage, newNode, absoluteByteNum);
	} else if (byteNum == 0) {	// Inserting at front; also easy, but we need to swap node and newNode
	    newNode = __CFStorageCreateNode(allocator, storage, true, 0);
	    
	    /* Transfer our memory to the new node */
	    newNode->numBytes = node->numBytes;
	    newNode->info.leaf.capacityInBytes = node->info.leaf.capacityInBytes;
	    __CFAssignWithWriteBarrier((void **)&newNode->info.leaf.memory, node->info.leaf.memory);
	    
	    /* Stomp on our existing node */
	    node->numBytes = size;
	    node->info.leaf.capacityInBytes = 0;
	    node->info.leaf.memory = NULL;
	    
	    /* Cache our existing node */
	    __CFStorageSetCache(storage, node, absoluteByteNum);
	} else if (byteNum + size <= storage->maxLeafCapacity) {	// Inserting at middle; inserted region will fit into existing child
	    // Create new node to hold the overflow
	    newNode = __CFStorageCreateNode(allocator, storage, true, node->numBytes - byteNum);
	    if (node->info.leaf.memory) {	// We allocate memory lazily...
		__CFStorageAllocLeafNodeMemory(allocator, storage, newNode, node->numBytes - byteNum, false);
		COPYMEM(node->info.leaf.memory + byteNum, newNode->info.leaf.memory, node->numBytes - byteNum);
		__CFStorageAllocLeafNodeMemory(allocator, storage, node, byteNum + size, false);
	    }
	    node->numBytes = byteNum + size;
	    __CFStorageSetCache(storage, node, absoluteByteNum - byteNum);
	} else {	// Inserting some of new into one node, rest into another; remember that the assumption is size <= storage->maxLeafCapacity
	    newNode = __CFStorageCreateNode(allocator, storage, true, node->numBytes + size - storage->maxLeafCapacity);	// New stuff
	    if (node->info.leaf.memory) {	// We allocate memory lazily...
		__CFStorageAllocLeafNodeMemory(allocator, storage, newNode, node->numBytes + size - storage->maxLeafCapacity, false);
		COPYMEM(node->info.leaf.memory + byteNum, newNode->info.leaf.memory + byteNum + size - storage->maxLeafCapacity, node->numBytes - byteNum);
		__CFStorageAllocLeafNodeMemory(allocator, storage, node, storage->maxLeafCapacity, false);
	    }
	    __CFStorageSetCache(storage, node, absoluteByteNum - byteNum);
	    node->numBytes = storage->maxLeafCapacity;
	}
	return CFStorageDoubleNodeReturnMake(node, newNode); // We do not retain 'node' because it is the given node.
    } else {	// No need to create new nodes!
	if (node->info.leaf.memory) {
	    __CFStorageAllocLeafNodeMemory(allocator, storage, node, node->numBytes + size, false);
	    COPYMEM(node->info.leaf.memory + byteNum, node->info.leaf.memory + byteNum + size, node->numBytes - byteNum);
	}
	node->numBytes += size;
	__CFStorageSetCache(storage, node, absoluteByteNum - byteNum);
	return CFStorageDoubleNodeReturnMake(node, NULL); //return the existing node, meaning the parent does not have to do anything.  Do not retain it because it is the given node.
    }
}

static CFStorageDoubleNodeReturn __CFStorageInsertBranchUnfrozen(CFAllocatorRef allocator, CFStorageRef storage, CFStorageNode *node, CFIndex byteNum, CFIndex size, CFIndex absoluteByteNum) {
    CFIndex relativeByteNum;
    CFIndex childNum; // we will insert after childNum, i.e. if childNum is 0, any new node becomes index 1.  This can have value 0, 1, or 2.
    CFStorageNode *childNode = __CFStorageFindChild(node, byteNum, true, &childNum, &relativeByteNum);
    CFStorageDoubleNodeReturn newNodes = __CFStorageInsert(allocator, storage, childNode, relativeByteNum, size, absoluteByteNum);
    CFStorageDoubleNodeReturn result = {node, NULL}; // the default return value meaning we did all of the work ourselves and our parent does not need to do anything
    ASSERT(childNode != NULL);
    ASSERT(newNodes.child != NULL);
    
    if (newNodes.child != childNode) {
	/* We got back a replacement for the current child, so replace it. */
	__CFStorageReleaseNode(storage, childNode);
	__CFStorageSetChild(node, childNum, newNodes.child);
    }
    
    if (newNodes.sibling == NULL) {
	/* The insertion happened successfully without requiring us to add any new nodes. */
	node->numBytes += size;
    } else {
	/* We got back an additional node to insert. */
	CFStorageNode *newChild = newNodes.sibling;
	if (node->info.notLeaf.child[2] == NULL) {	// There's an empty slot for the new node, cool
	    if (childNum == 0) __CFStorageSetChild(node, 2, node->info.notLeaf.child[1]); // Make room
	    __CFStorageSetChild(node, childNum+1, newChild);
	    node->numBytes += size;
	} else {
	    CFStorageNode *anotherNode = __CFStorageCreateNode(allocator, storage, false, 0);	// Create another node
	    if (childNum == 0) {	// Last two children go to new node
		__CFStorageSetChild(anotherNode, 0, node->info.notLeaf.child[1]);
		__CFStorageSetChild(anotherNode, 1, node->info.notLeaf.child[2]);
		__CFStorageSetChild(node, 1, newChild);
		node->info.notLeaf.child[2] = NULL;
	    } else if (childNum == 1) {	// Last child goes to new node
		__CFStorageSetChild(anotherNode, 0, newChild);
		__CFStorageSetChild(anotherNode, 1, node->info.notLeaf.child[2]);
		node->info.notLeaf.child[2] = NULL;
	    } else {	// New node contains the new comers...
		__CFStorageSetChild(anotherNode, 0, node->info.notLeaf.child[2]);
		__CFStorageSetChild(anotherNode, 1, newChild);
		node->info.notLeaf.child[2] = NULL;
	    }
	    node->numBytes = node->info.notLeaf.child[0]->numBytes + node->info.notLeaf.child[1]->numBytes;
	    anotherNode->numBytes = anotherNode->info.notLeaf.child[0]->numBytes + anotherNode->info.notLeaf.child[1]->numBytes;
	    /* node should not be retained because it is the passed in node.  anotherNode was created so we transfer its retain count */
	    result.sibling = anotherNode;
	}
    } 
    return result;
}



/* Returns NULL or additional node to come after this node
 Assumption: size is never > storage->maxLeafCapacity
 */
static CFStorageDoubleNodeReturn __CFStorageInsertUnfrozen(CFAllocatorRef allocator, CFStorageRef storage, CFStorageNode *node, CFIndex byteNum, CFIndex size, CFIndex absoluteByteNum) {
    ASSERT(! node->isFrozen);
    if (node->isLeaf) {
	return __CFStorageInsertLeafUnfrozen(allocator, storage, node, byteNum, size, absoluteByteNum);
    } else {
	return __CFStorageInsertBranchUnfrozen(allocator, storage, node, byteNum, size, absoluteByteNum);
    }
}

#pragma mark Frozen or Unfrozen Dispatch Functions

static CFStorageDoubleNodeReturn __CFStorageInsert(CFAllocatorRef allocator, CFStorageRef storage, CFStorageNode *node, CFIndex byteNum, CFIndex size, CFIndex absoluteByteNum) {
    if (node->isFrozen && ! __CFStorageThawNodeDuringMutation(storage, node)) {
	return __CFStorageInsertFrozen(allocator, storage, node, byteNum, size, absoluteByteNum);
    }
    else {
	return __CFStorageInsertUnfrozen(allocator, storage, node, byteNum, size, absoluteByteNum);
    }
}

static CFStorageNode *__CFStorageDelete(CFAllocatorRef allocator, CFStorageRef storage, CFStorageNode *node, CFRange range, bool compact) {
    if (node->isFrozen && ! __CFStorageThawNodeDuringMutation(storage, node)) {
	return __CFStorageDeleteFrozen(allocator, storage, node, range);
    }
    else {
	return __CFStorageDeleteUnfrozen(allocator, storage, node, range, compact, false/*isRootNode*/);
    }
}


#pragma mark Utility functions

CF_INLINE CFIndex __CFStorageGetCount(CFStorageRef storage) {
    return __CFStorageConvertByteToValue(storage, storage->rootNode.numBytes);
}

static Boolean __CFStorageEqual(CFTypeRef cf1, CFTypeRef cf2) {
    CFStorageRef storage1 = (CFStorageRef)cf1;
    CFStorageRef storage2 = (CFStorageRef)cf2;
    CFIndex loc, count, valueSize;
    CFRange range1, range2;
    uint8_t *ptr1, *ptr2;
    
    count = __CFStorageGetCount(storage1);
    if (count != __CFStorageGetCount(storage2)) return false;
    
    valueSize = __CFStorageGetValueSize(storage1);
    if (valueSize != __CFStorageGetValueSize(storage2)) return false;
    
    loc = range1.location = range1.length = range2.location = range2.length = 0;
    ptr1 = ptr2 = NULL;
    
    while (loc < count) {
	CFIndex cntThisTime;
	if (loc >= range1.location + range1.length) ptr1 = (uint8_t *)CFStorageGetValueAtIndex(storage1, loc, &range1);
	if (loc >= range2.location + range2.length) ptr2 = (uint8_t *)CFStorageGetValueAtIndex(storage2, loc, &range2);
	cntThisTime = range1.location + range1.length;
	if (range2.location + range2.length < cntThisTime) cntThisTime = range2.location + range2.length;
	cntThisTime -= loc;
	if (memcmp(ptr1, ptr2, valueSize * cntThisTime) != 0) return false;
	ptr1 += valueSize * cntThisTime;
	ptr2 += valueSize * cntThisTime;
	loc += cntThisTime;
    }
    return true;
}

static CFHashCode __CFStorageHash(CFTypeRef cf) {
    CFStorageRef Storage = (CFStorageRef)cf;
    return __CFStorageGetCount(Storage);
}

static void __CFStorageDescribeNode(CFStorageNode *node, CFMutableStringRef str, CFIndex level) {
    int cnt;
    for (cnt = 0; cnt < level; cnt++) CFStringAppendCString(str, "  ", CFStringGetSystemEncoding());
    
    if (node->isLeaf) {
        CFStringAppendFormat(str, NULL, CFSTR("Leaf %ld/%ld (%p) refcount: %u frozen: %s\n"), node->numBytes, node->info.leaf.capacityInBytes, node, node->refCount, node->isFrozen ? "yes" : "no");
    } else {
        CFStringAppendFormat(str, NULL, CFSTR("Node %ld (%p) refcount: %u frozen: %s\n"), node->numBytes, node, node->refCount, node->isFrozen ? "yes" : "no");
        for (cnt = 0; cnt < 3; cnt++) if (node->info.notLeaf.child[cnt]) __CFStorageDescribeNode(node->info.notLeaf.child[cnt], str, level+1);
    }
}

static CFIndex __CFStorageGetNodeCapacity(CFStorageNode *node) {
    if (!node) return 0;
    if (node->isLeaf) return node->info.leaf.capacityInBytes;
    return __CFStorageGetNodeCapacity(node->info.notLeaf.child[0]) + __CFStorageGetNodeCapacity(node->info.notLeaf.child[1]) + __CFStorageGetNodeCapacity(node->info.notLeaf.child[2]);
}

CFIndex __CFStorageGetCapacity(CFStorageRef storage) {
    return __CFStorageConvertByteToValue(storage, __CFStorageGetNodeCapacity(&storage->rootNode));
}

CFIndex __CFStorageGetValueSize(CFStorageRef storage) {
    return storage->valueSize;
}

static CFStringRef __CFStorageCopyDescription(CFTypeRef cf) {
    CFStorageRef storage = (CFStorageRef)cf;
    CFMutableStringRef result;
    CFAllocatorRef allocator = CFGetAllocator(storage);
    result = CFStringCreateMutable(allocator, 0);
    CFStringAppendFormat(result, NULL, CFSTR("<CFStorage %p [%p]>[count = %lu, capacity = %lu]\n"), storage, allocator, (unsigned long)__CFStorageGetCount(storage), (unsigned long)__CFStorageGetCapacity(storage));
    __CFStorageDescribeNode(&storage->rootNode, result, 0);
    return result;
}

/* Returns true if enumeration should stop, false if it should continue. */
static bool __CFStorageEnumerateNodesInByteRangeWithBlock(CFStorageRef storage, CFStorageNode *node, CFIndex globalOffsetOfNode, CFRange range, CFIndex concurrencyToken, CFStorageApplierBlock applier) {
    bool stop = false;
    if (node->isLeaf) {
	CFIndex start = range.location;
	CFIndex length = __CFMin(range.length, node->numBytes - start);
	if (! node->info.leaf.memory) {
	    __CFStorageAllocLeafNodeMemory(CFGetAllocator(storage), storage, node, node->numBytes, false);
	}
	applier(node->info.leaf.memory + start, __CFStorageConvertBytesToValueRange(storage, globalOffsetOfNode + start, length), &stop);
    }
    else {
	CFStorageNode *children[3] = {node->info.notLeaf.child[0], node->info.notLeaf.child[1], node->info.notLeaf.child[2]};
	const CFIndex lengths[3] = {children[0]->numBytes, children[1] ? children[1]->numBytes : 0, children[2] ? children[2]->numBytes : 0};
	const CFIndex offsets[3] = {0, lengths[0], lengths[0] + lengths[1]};	
	const CFRange overlaps[3] = {intersectionRange(CFRangeMake(offsets[0], lengths[0]), range), intersectionRange(CFRangeMake(offsets[1], lengths[1]), range), intersectionRange(CFRangeMake(offsets[2], lengths[2]), range)};
	CFIndex numOverlappingChildren = (!! overlaps[0].length + !! overlaps[1].length + !! overlaps[2].length);
	if (numOverlappingChildren > 1) concurrencyToken--;
	
	if (concurrencyToken >= 0 && numOverlappingChildren > 1) {
	    CFIndex numChildren = 1 + !!children[1] + !!children[2];
	    const CFRange * overlapsPtr = overlaps; //blocks don't let us reference arrays :(
	    const CFIndex * offsetsPtr = offsets;
	    CFStorageNode ** childrenPtr = children;
#if DEPLOYMENT_TARGET_MACOSX || DEPLOYMENT_TARGET_EMBEDDED || DEPLOYMENT_TARGET_WINDOWS
	    __block bool blockStop = false;
	    dispatch_apply(numChildren, __CFDispatchQueueGetGenericMatchingCurrent(), ^(size_t ind) {
		if (! blockStop && overlapsPtr[ind].length > 0) {
		    if (__CFStorageEnumerateNodesInByteRangeWithBlock(storage, childrenPtr[ind], globalOffsetOfNode + offsetsPtr[ind], CFRangeMake(overlapsPtr[ind].location - offsetsPtr[ind], overlapsPtr[ind].length), concurrencyToken, applier)) {
			blockStop = true;
		    }
		}
	    });
	    stop = blockStop;
#else
        for (CFIndex ind = 0; ind < numChildren; ind++) {
            if (overlapsPtr[ind].length > 0) {
                if (__CFStorageEnumerateNodesInByteRangeWithBlock(storage, childrenPtr[ind], globalOffsetOfNode + offsetsPtr[ind], CFRangeMake(overlapsPtr[ind].location - offsetsPtr[ind], overlapsPtr[ind].length), concurrencyToken, applier)) {
                    stop = true;
                    break;
		        }
		    }
	    }
#endif
	} else {
	    if (overlaps[0].length > 0) {
		stop = stop || __CFStorageEnumerateNodesInByteRangeWithBlock(storage, children[0], globalOffsetOfNode + offsets[0], CFRangeMake(overlaps[0].location - offsets[0], overlaps[0].length), concurrencyToken, applier);
	    }
	    if (overlaps[1].length > 0) {
		stop = stop || __CFStorageEnumerateNodesInByteRangeWithBlock(storage, children[1], globalOffsetOfNode + offsets[1], CFRangeMake(overlaps[1].location - offsets[1], overlaps[1].length), concurrencyToken, applier);
	    }
	    if (overlaps[2].length > 0) {
		stop = stop || __CFStorageEnumerateNodesInByteRangeWithBlock(storage, children[2], globalOffsetOfNode + offsets[2], CFRangeMake(overlaps[2].location - offsets[2], overlaps[2].length), concurrencyToken, applier);
	    }
	}
    }
    return stop;
}

static CFStorageNode *_CFStorageFindNodeContainingByteRange(ConstCFStorageRef storage, const CFStorageNode *node, CFRange nodeRange, CFIndex globalOffsetOfNode, CFRange *outGlobalByteRangeOfResult) {
    if (! node->isLeaf) {
	/* See how many children are overlapped by this range.  If it's only 1, call us recursively on that node; otherwise we're it! */
	CFStorageNode *children[3] = {node->info.notLeaf.child[0], node->info.notLeaf.child[1], node->info.notLeaf.child[2]};
	const CFIndex lengths[3] = {children[0]->numBytes, children[1] ? children[1]->numBytes : 0, children[2] ? children[2]->numBytes : 0};
	const CFIndex offsets[3] = {0, lengths[0], lengths[0] + lengths[1]};	
	const CFRange overlaps[3] = {intersectionRange(CFRangeMake(offsets[0], lengths[0]), nodeRange), intersectionRange(CFRangeMake(offsets[1], lengths[1]), nodeRange), intersectionRange(CFRangeMake(offsets[2], lengths[2]), nodeRange)};
	CFIndex numOverlappingChildren = (!! overlaps[0].length + !! overlaps[1].length + !! overlaps[2].length);
	ASSERT(numOverlappingChildren > 0);
	if (numOverlappingChildren == 1) {
	    CFIndex overlappingChild = (overlaps[0].length ? 0 : (overlaps[1].length ? 1 : 2));
	    return _CFStorageFindNodeContainingByteRange(storage, children[overlappingChild], CFRangeMake(nodeRange.location - offsets[overlappingChild], nodeRange.length), globalOffsetOfNode + offsets[overlappingChild], outGlobalByteRangeOfResult);
	}
    }
    
    /* Either we are a leaf, in which case we contain the range, or we are a branch with multiple overlapping children.  Either way, we are the minimum node containing the range in question. */
    *outGlobalByteRangeOfResult = CFRangeMake(globalOffsetOfNode, node->numBytes);
    return (CFStorageNode *)node;
    
    
}

/* Frees all memory associated with the root node, effectively emptying the CFStorage */
static void __CFStorageClearRootNode(CFStorageRef storage) {
    CFAllocatorRef allocator = CFGetAllocator(storage);
    /* Have to release our children if we are a branch, or free our memory if we are a leaf */
    if (storage->rootNode.isLeaf) {
	_CFAllocatorDeallocateGC(allocator, storage->rootNode.info.leaf.memory);
    }
    else {
	__CFStorageReleaseNodeWithNullCheck(storage, storage->rootNode.info.notLeaf.child[0]);
	__CFStorageReleaseNodeWithNullCheck(storage, storage->rootNode.info.notLeaf.child[1]);
	__CFStorageReleaseNodeWithNullCheck(storage, storage->rootNode.info.notLeaf.child[2]);
    }
    storage->rootNode.isLeaf = true;
    storage->rootNode.numBytes = 0;
    storage->rootNode.info.leaf.capacityInBytes = 0;
    storage->rootNode.info.leaf.memory = NULL;
}

static void __CFStorageDeallocate(CFTypeRef cf) {
    /* CFStorage is used in CFArray.  Under GC, CFArray references us strongly, but not retained.  Thus we may be finalized before the array.  When the array itself is finalized, it will call any custom deallocate callback on all of its contents, which means it has to walk the array.  Thus CFStorage must be careful to not perturb its structure in Deallocate under GC.
     
     CFStorage nodes have a reference count, and if a node has a reference count of one, and we are in a mutating function, we conclude that this CFStorage has exclusive ownership of the node, and we can treat it as mutable even if it's marked as frozen (see __CFStorageThawNodeDuringMutation).  Therefore it would be nice if we could decrement our nodes' refcounts in Deallocate.  However, if we did so, then another CFStorage might treat a node that we reference as mutable and modify it, which must not happen, because we must not perturb the structure of a CFStorage in Deallocate.  Thus we just "leak" a reference count under GC.  Of course, these reference counts don't actually keep the memory alive in GC, so it's not a real leak.
     */
    CFStorageRef storage = (CFStorageRef)cf;
    if (! CF_IS_COLLECTABLE_ALLOCATOR(CFGetAllocator(storage))) {
        __CFStorageClearRootNode(storage);
    }
}

static CFTypeID __kCFStorageTypeID = _kCFRuntimeNotATypeID;

static const CFRuntimeClass __CFStorageClass = {
    _kCFRuntimeScannedObject,
    "CFStorage",
    NULL,	// init
    NULL,	// copy
    __CFStorageDeallocate,
    __CFStorageEqual,
    __CFStorageHash,
    NULL,	// 
    __CFStorageCopyDescription
};

/*** Public API ***/

CFStorageRef CFStorageCreate(CFAllocatorRef allocator, CFIndex valueSize) {
    CFStorageRef storage;
    CFIndex size = sizeof(struct __CFStorage) - sizeof(CFRuntimeBase);
    storage = (CFStorageRef)_CFRuntimeCreateInstance(allocator, CFStorageGetTypeID(), size, NULL);
    if (NULL == storage) {
	return NULL;
    }
    storage->valueSize = valueSize;
    /* if valueSize is a power of 2, then set the shifter to the log base 2 of valueSize.  Otherwise set it to NO_SHIFTER */
    if (valueSize > 0 && !(valueSize & (valueSize - 1))) {
#if DEPLOYMENT_TARGET_MACOSX || DEPLOYMENT_TARGET_EMBEDDED
	storage->byteToValueShifter = __builtin_ctzl(valueSize);
#else
	CFIndex tempSize = valueSize;
	storage->byteToValueShifter = 0;
	while (tempSize > 1) {
	    storage->byteToValueShifter++;
	    tempSize >>= 1;
	}
#endif
    }
    else {
	storage->byteToValueShifter = NO_SHIFTER;
    }
    
    CF_LOCK_INIT_FOR_STRUCTS(storage->cacheReaderMemoryAllocationLock);
    storage->maxLeafCapacity = __CFStorageMaxLeafCapacity;
    if (valueSize && ((storage->maxLeafCapacity % valueSize) != 0)) {	
        storage->maxLeafCapacity = (storage->maxLeafCapacity / valueSize) * valueSize;	// Make it fit perfectly (3406853)
    }
    memset(&(storage->rootNode), 0, sizeof(CFStorageNode));
    storage->rootNode.isLeaf = true;
    storage->rootNode.refCount = 0;
    if (valueSize >= sizeof(void *)) {
	storage->nodeHint = __kCFAllocatorGCScannedMemory;
    }
    else {
	// Don't scan nodes if the value size is smaller than a pointer (8198596)
	storage->nodeHint = 0;
    }
    if (__CFOASafe) __CFSetLastAllocationEventName(storage, "CFStorage");
    return storage;    
}

CFStorageRef CFStorageCreateWithSubrange(CFStorageRef mutStorage, CFRange range) {
    const ConstCFStorageRef storage = mutStorage; //we expect this to never modify the storage, so use a const variable to help enforce that
    CFStorageRef result = CFStorageCreate(CFGetAllocator(storage), storage->valueSize);
    
    if (range.length > 0) {
	/* Start by finding the node that contains the entire range.  Bump the reference count of its children and add them to the root of our new copy. */
	const CFRange byteRange = __CFStorageConvertValuesToByteRange(storage, range.location, range.length);
	CFRange byteRangeOfContainingNode;
	CFStorageNode *nodeContainingEntireRange = _CFStorageFindNodeContainingByteRange(storage, &storage->rootNode, byteRange, 0, &byteRangeOfContainingNode);
	ASSERT(nodeContainingEntireRange != NULL);
	
	/* If the result is a leaf, insert the portion we care about */
	if (nodeContainingEntireRange->isLeaf) {
	    CFStorageInsertValues(result, CFRangeMake(0, range.length));
	    if (nodeContainingEntireRange->info.leaf.memory) {
		CFIndex offsetIntoNode = byteRange.location - byteRangeOfContainingNode.location;
		ASSERT(offsetIntoNode >= 0);
		CFStorageReplaceValues(result, CFRangeMake(0, range.length), nodeContainingEntireRange->info.leaf.memory + offsetIntoNode);
	    }
	}
	else {
	    /* The result is not a leaf.  Insert all of its children into our root. */
	    ASSERT(byteRangeOfContainingNode.length == nodeContainingEntireRange->numBytes);
	    result->rootNode.isLeaf = false;
	    result->rootNode.numBytes = byteRangeOfContainingNode.length;
	    result->rootNode.info.notLeaf.child[0] = result->rootNode.info.notLeaf.child[1] = result->rootNode.info.notLeaf.child[2] = NULL;
	    for (CFIndex i=0; i < 3; i++) {
		CFStorageNode *newNode = nodeContainingEntireRange->info.notLeaf.child[i];
		if (! newNode) break;
		__CFStorageFreezeNode(newNode);
		__CFStorageSetChild(&result->rootNode, i, __CFStorageRetainNode(newNode));
	    }
	    
	    /* Now delete from the beginning or end to trim this to the right size */
	    CFRange rangeOfContainingNode = __CFStorageConvertBytesToValueRange(result, byteRangeOfContainingNode.location, byteRangeOfContainingNode.length);
	    CFIndex prefixToTrim = range.location - rangeOfContainingNode.location;
	    CFIndex suffixToTrim = (rangeOfContainingNode.location + rangeOfContainingNode.length) - (range.location + range.length);
	    ASSERT(prefixToTrim >= 0);
	    ASSERT(suffixToTrim >= 0);
	    ASSERT(CFStorageGetCount(result) == rangeOfContainingNode.length);
	    if (suffixToTrim > 0) CFStorageDeleteValues(result, CFRangeMake(rangeOfContainingNode.length - suffixToTrim, suffixToTrim));
	    if (prefixToTrim > 0) CFStorageDeleteValues(result, CFRangeMake(0, prefixToTrim));
	}
    }
    
    ASSERT(CFStorageGetCount(result) == range.length);
    return result;
}

CFTypeID CFStorageGetTypeID(void) {
    static dispatch_once_t initOnce;
    dispatch_once(&initOnce, ^{ __kCFStorageTypeID = _CFRuntimeRegisterClass(&__CFStorageClass); });
    return __kCFStorageTypeID;
}

CFIndex CFStorageGetCount(CFStorageRef storage) {
    return __CFStorageGetCount(storage);
}

/* Returns pointer to the specified value
 index and validConsecutiveValueRange are in terms of values
 */
void *CFStorageGetValueAtIndex(CFStorageRef storage, CFIndex idx, CFRange *validConsecutiveValueRange) {
    CFRange dummy;
    return __CFStorageGetValueAtIndex(storage, idx, validConsecutiveValueRange ? validConsecutiveValueRange : &dummy, true/*requireUnfreezing*/);
}

const void *CFStorageGetConstValueAtIndex(CFStorageRef storage, CFIndex idx, CFRange *validConsecutiveValueRange) {
    CFRange dummy;
    return __CFStorageGetValueAtIndex(storage, idx, validConsecutiveValueRange ? validConsecutiveValueRange : &dummy, false/*requireUnfreezing*/);
}


/* Makes space for range.length values at location range.location
 This function deepens the tree if necessary...
 */
void CFStorageInsertValues(CFStorageRef storage, CFRange range) {
    CFIndex numBytesToInsert = __CFStorageConvertValueToByte(storage, range.length);
    CFIndex byteNum = __CFStorageConvertValueToByte(storage, range.location);
    const CFIndex expectedByteCount = storage->rootNode.numBytes + numBytesToInsert;
    const CFAllocatorRef allocator = CFGetAllocator(storage);
    const CFIndex insertionChunkSize = storage->maxLeafCapacity;
    while (numBytesToInsert > 0) {
	CHECK_INTEGRITY();
        const CFIndex insertThisTime = __CFMin(numBytesToInsert, insertionChunkSize);
        CFStorageDoubleNodeReturn newNodes = __CFStorageInsertUnfrozen(allocator, storage, &storage->rootNode, byteNum, insertThisTime, byteNum); //we don't have to call the frozen variant because the root node is never frozen
	ASSERT(newNodes.child == &storage->rootNode);// unfrozen variant should always give us our node back.  We may have another node to insert in newNodes.sibling
        if (newNodes.sibling != NULL) {
	    CFStorageNode *newNode = newNodes.sibling;
	    /* Need to create a new root node.  Copy our existing root node's contents to a new heap node. */
	    CFStorageNode *heapRoot = __CFStorageCreateNode(allocator, storage, storage->rootNode.isLeaf, storage->rootNode.numBytes);	// Will copy the (static) rootNode over to this
	    objc_memmove_collectable(&heapRoot->info, &storage->rootNode.info, sizeof heapRoot->info);
	    
	    /* Our root is about to become a branch.  If our root node is currently a leaf, we need to clear the cache, because if the cache points at the root then the cache is about to start pointing at a branch node (which is not allowed) */
	    if (storage->rootNode.isLeaf) {
		__CFStorageSetCache(storage, NULL, 0);
		storage->rootNode.isLeaf = false;
	    }
	    
	    /* Set the new children in our root.  Note that it's important that we overwrite the root node's info, because we wanted to transfer the refcounts of our children (or our allocated memory, if we are a leaf) to the new heap root */
	    __CFStorageSetChild(&storage->rootNode, 0, heapRoot);
	    __CFStorageSetChild(&storage->rootNode, 1, newNode);
            storage->rootNode.info.notLeaf.child[2] = NULL;
            storage->rootNode.numBytes = heapRoot->numBytes + newNode->numBytes;
	}
        numBytesToInsert -= insertThisTime;
        byteNum += insertThisTime;
	ASSERT(storage->rootNode.numBytes + numBytesToInsert == expectedByteCount);
    }
    ASSERT(expectedByteCount == storage->rootNode.numBytes);
    CHECK_INTEGRITY();
}

/* Deletes the values in the specified range
 This function gets rid of levels if necessary...
 */
void CFStorageDeleteValues(CFStorageRef storage, CFRange range) {
    CHECK_INTEGRITY();
    CFAllocatorRef allocator = CFGetAllocator(storage);
    CFRange byteRange = __CFStorageConvertValuesToByteRange(storage, range.location, range.length);
    const CFIndex expectedByteCount = storage->rootNode.numBytes - byteRange.length;
    
    /* We don't try to mantain the cache across deletion */
    __CFStorageSetCache(storage, NULL, 0);
    
    /* The root node can never be frozen, so it's always OK to modify it */
    ASSERT(! storage->rootNode.isFrozen);    
    CFStorageNode *newRoot = __CFStorageDeleteUnfrozen(allocator, storage, &storage->rootNode, byteRange, true/*compact*/, true/*isRootNode*/);
    
    /* There are three return values we can get:
     NULL -> everything was deleted
     the root node -> something was deleted, but no nodes became empty, so we don't have to replace any children
     a different node -> represents the new root
     */
    if (newRoot == NULL) {
	__CFStorageClearRootNode(storage);
    }
    else if (newRoot == &storage->rootNode) {
	/* No need to replace any children, nothing to do for this case */
    }
    else {
	/* Got a legitimately new root back.  If it is unfrozen, we can just acquire its guts.  If it is frozen, we have more work to do.  Note that we do not have to worry about releasing any existing children of the root, beacuse __CFStorageDeleteUnfrozen already did that.  Also note that if we got a legitimately new root back, we must be a branch node, because if we were a leaf node, we would have been unfrozen and gotten ourself back. */
	storage->rootNode.numBytes = newRoot->numBytes;
	storage->rootNode.isLeaf = newRoot->isLeaf;
	bzero(&storage->rootNode.info, sizeof storage->rootNode.info); //be a little paranoid here
	if (newRoot->isLeaf) {
	    if (! newRoot->isFrozen) {
		/* If the leaf is not frozen, we can just steal its memory (if any)!  If it is frozen, we must copy it. */
		__CFAssignWithWriteBarrier((void **)&storage->rootNode.info.leaf.memory, newRoot->info.leaf.memory);
		/* Clear out the old node, because we stole its memory and we don't want it to deallocate it when teh node is destroyed below. */
		bzero(&newRoot->info, sizeof newRoot->info);
	    }
	    else {
		/* The leaf is frozen, so we have to copy its memory.   */
		if (newRoot->info.leaf.memory) {
		    __CFStorageAllocLeafNodeMemory(allocator, storage, &storage->rootNode, storage->rootNode.numBytes, false);
		    COPYMEM(newRoot->info.leaf.memory, storage->rootNode.info.leaf.memory, newRoot->numBytes);
		}
	    }
	} else {
	    /* New root is a branch. */
	    ASSERT(newRoot->info.notLeaf.child[0] && newRoot->info.notLeaf.child[1]); //never expect to get back a node with only one child
	    __CFStorageSetChild(&storage->rootNode, 0, __CFStorageRetainNode(newRoot->info.notLeaf.child[0]));
	    __CFStorageSetChild(&storage->rootNode, 1, __CFStorageRetainNode(newRoot->info.notLeaf.child[1]));
	    if (newRoot->info.notLeaf.child[2]) __CFStorageSetChild(&storage->rootNode, 2, __CFStorageRetainNode(newRoot->info.notLeaf.child[2]));
	}	
    }
    __CFStorageReleaseNodeWithNullCheck(storage, newRoot); //balance the retain from __CFStorageDeleteUnfrozen
    ASSERT(expectedByteCount == storage->rootNode.numBytes);
    CHECK_INTEGRITY();
}

void CFStorageGetValues(CFStorageRef storage, CFRange range, void *values) {
    CHECK_INTEGRITY();
    while (range.length > 0) {
        CFRange leafRange;
        void *storagePtr = __CFStorageGetValueAtIndex(storage, range.location, &leafRange, false/*requireUnfreezing*/);
        CFIndex cntThisTime = __CFMin(range.length, leafRange.length - (range.location - leafRange.location));
	CFIndex byteCntThisTime = __CFStorageConvertValueToByte(storage, cntThisTime);
        COPYMEM(storagePtr, values, byteCntThisTime);
        values = (uint8_t *)values + byteCntThisTime;
        range.location += cntThisTime;
        range.length -= cntThisTime;
    }
}

unsigned long _CFStorageFastEnumeration(CFStorageRef storage, struct __objcFastEnumerationStateEquivalent *state, void *stackbuffer, unsigned long count) {
    // without trying to understand the data structure, each time through search for block containing index
    CFRange leafRange;
    if (state->state == 0) { /* first time, get length */
        state->extra[0] = __CFStorageGetCount(storage);
    }
    if (state->state >= state->extra[0]) return 0;
    state->itemsPtr = (unsigned long *)CFStorageGetValueAtIndex(storage, state->state, &leafRange);
    state->state += leafRange.length;
    return leafRange.length;
}

void CFStorageApplyFunction(CFStorageRef storage, CFRange range, CFStorageApplierFunction applier, void *context) {
    CHECK_INTEGRITY();
    CFIndex valueSize = storage->valueSize;
    CFStorageApplyBlock(storage, range, 0, ^(const void *storagePtr, CFRange subrange, bool *stop){
	while (subrange.length--) {
	    applier(storagePtr, context);
	    storagePtr = valueSize + (const char *)storagePtr;
	}
    });
}

void CFStorageApplyBlock(CFStorageRef storage, CFRange range, CFStorageEnumerationOptionFlags options, CFStorageApplierBlock applier) {
    if (! range.length) return;
    CFRange byteRange = __CFStorageConvertValuesToByteRange(storage, range.location, range.length);
    /* As we descend the tree, if we find we need to go down two or more children, and the concurrency token is not zero, then we decrement the concurrency token and do it concurrently.  Since we have 3 children, a concurrency token of 3 yields up to 3**3 == 27 threads, which is a lot!  Concurrency benefits start to kick in around one million elements */
    CFIndex concurrencyToken = 0;
    if ((options & kCFStorageEnumerationConcurrent) && (range.length >= 1024 * 1024)) {
	concurrencyToken = 3;
    }
    __CFStorageEnumerateNodesInByteRangeWithBlock(storage, &storage->rootNode, 0/*globalOffsetOfNode*/, byteRange, concurrencyToken, applier);
}

void CFStorageReplaceValues(CFStorageRef storage, CFRange range, const void *values) {
    CHECK_INTEGRITY();
    while (range.length > 0) {
        CFRange leafRange;
        void *storagePtr = __CFStorageGetValueAtIndex(storage, range.location, &leafRange, true/*requireUnfreezing*/);
	ASSERT(range.location >= leafRange.location);
	ASSERT(range.location < leafRange.location + leafRange.length);
        CFIndex cntThisTime = __CFMin(range.length, leafRange.length - (range.location - leafRange.location));
	CFIndex byteCntThisTime = __CFStorageConvertValueToByte(storage, cntThisTime);
        COPYMEM(values, storagePtr, byteCntThisTime);
	values = (const uint8_t *)values + byteCntThisTime;
        range.location += cntThisTime;
        range.length -= cntThisTime;
    }
}

static void __CFStorageApplyNodeBlockInterior(CFStorageRef storage, CFStorageNode *node, void (^block)(CFStorageRef storage, CFStorageNode *node)) {
    block(storage, node);
    if (! node->isLeaf) {
	CFStorageNode *childNode;
	if ((childNode = node->info.notLeaf.child[0])) __CFStorageApplyNodeBlockInterior(storage, childNode, block);
	if ((childNode = node->info.notLeaf.child[1])) __CFStorageApplyNodeBlockInterior(storage, childNode, block);
	if ((childNode = node->info.notLeaf.child[2])) __CFStorageApplyNodeBlockInterior(storage, childNode, block);
    }
}

static void __CFStorageApplyNodeBlock(CFStorageRef storage, void (^block)(CFStorageRef storage, CFStorageNode *node)) {
    __CFStorageApplyNodeBlockInterior(storage, &storage->rootNode, block);
}

static CFIndex __CFStorageEstimateTotalAllocatedSize(CFStorageRef storage) __attribute__((unused));
static CFIndex __CFStorageEstimateTotalAllocatedSize(CFStorageRef storage) {
    __block CFIndex nodeResult = 0;
    __block CFIndex contentsResult = 0;
    __CFStorageApplyNodeBlock(storage, ^(CFStorageRef storage, CFStorageNode *node) {
	if (node != &storage->rootNode) {
	    nodeResult += malloc_size(node);
	    if (node->isLeaf && node->info.leaf.memory != NULL) {
		contentsResult += malloc_size(node->info.leaf.memory);
	    }
	}
    });
    return nodeResult + contentsResult;
}

void __CFStorageSetAlwaysFrozen(CFStorageRef storage, bool alwaysFrozen) {
    storage->alwaysFrozen = alwaysFrozen;
}

static CFIndex __CFStorageCheckNodeCachedLengthIntegrity(ConstCFStorageRef storage, const CFStorageNode *node) {
    if (node->isLeaf) {
	ASSERT(node->numBytes > 0 || node == &storage->rootNode);
	return node->numBytes;
    } else {
	/* Branch */
	CFStorageNode *childNode;
	CFIndex expectedResult = __CFStorageCheckNodeCachedLengthIntegrity(storage, node->info.notLeaf.child[0]);
	if ((childNode = node->info.notLeaf.child[1])) {
	    expectedResult += __CFStorageCheckNodeCachedLengthIntegrity(storage, childNode);
	    if ((childNode = node->info.notLeaf.child[2])) {
		expectedResult += __CFStorageCheckNodeCachedLengthIntegrity(storage, childNode);
	    }	    
	}
	ASSERT(expectedResult == node->numBytes);
	return expectedResult;
    }
}

static void __CFStorageCheckNodeIntegrity(ConstCFStorageRef storage, const CFStorageNode *node) {
    ASSERT(node->isFrozen == 0 || node->isFrozen == 1);
    
    __CFStorageCheckNodeCachedLengthIntegrity(storage, node);
    /* If we are a branch, make sure that we have at least one child, and that there is not a non-NULL child after a NULL child */
    if (! node->isLeaf) {
	ASSERT(node->info.notLeaf.child[0] != NULL);
	if (node->info.notLeaf.child[1] == NULL) ASSERT(node->info.notLeaf.child[2] == NULL);	
    }
}

static void __CFStorageCheckIntegrity(CFStorageRef storage) {
    __CFStorageApplyNodeBlock(storage, ^(CFStorageRef storage, CFStorageNode *node) {
	__CFStorageCheckNodeIntegrity(storage, node);
    });
}

/* Used by CFArray.c */

static void __CFStorageNodeSetUnscanned(CFStorageNode *node, auto_zone_t *zone) {
    if (node->isLeaf) {
        auto_zone_set_unscanned(zone, node->info.leaf.memory);
    } else {
        CFStorageNode **children = node->info.notLeaf.child;
        if (children[0]) __CFStorageNodeSetUnscanned(children[0], zone);
        if (children[1]) __CFStorageNodeSetUnscanned(children[1], zone);
        if (children[2]) __CFStorageNodeSetUnscanned(children[2], zone);
    }
}

CF_PRIVATE void _CFStorageSetWeak(CFStorageRef storage) {
    storage->nodeHint = 0;
    __CFStorageNodeSetUnscanned(&storage->rootNode, (auto_zone_t *)objc_collectableZone());
}

#undef COPYMEM
#undef PAGE_LIMIT

