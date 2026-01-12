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

/*  CFBurstTrie.c
    Copyright (c) 2008-2014, Apple Inc. All rights reserved.
    Responsibility: Jennifer Moore
*/

#include "CFInternal.h"
#include "CFBurstTrie.h"
#include "CFByteOrder.h"
#include "CFNumber.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <limits.h>
#if DEPLOYMENT_TARGET_MACOSX || DEPLOYMENT_TARGET_EMBEDDED || DEPLOYMENT_TARGET_EMBEDDED_MINI || DEPLOYMENT_TARGET_LINUX
#include <unistd.h>
#include <sys/param.h>
#include <sys/mman.h>
#endif

#include <errno.h>
#include <assert.h>

#if DEPLOYMENT_TARGET_WINDOWS
#define open _NS_open
#define statinfo _stat
#define stat(x,y) _NS_stat(x,y)
#define __builtin_memcmp(x, y, z) memcmp(x, y, z)
#define __builtin_popcountll(x) popcountll(x)
#define bzero(dst, size)    ZeroMemory(dst, size)
#define S_IWUSR 0
#define S_IRUSR 0

static int pwrite(int fd, const void *buf, size_t nbyte, off_t offset) {
    // Get where we are
    long pos = _tell(fd);
    
    // Move to new offset
    _lseek(fd, offset, SEEK_SET);
    
    // Write data
    int res = _write(fd, buf, nbyte);
    
    // Return to previous offset
    _lseek(fd, pos, SEEK_SET);
    
    return res;
}

#else
#define statinfo stat
#endif

#if 0
#pragma mark Types and Utilities
#endif

#define MAX_STRING_ALLOCATION_SIZE  342
#define MAX_STRING_SIZE             1024
#define MAX_KEY_LENGTH              MAX_STRING_SIZE * 4
#define CHARACTER_SET_SIZE          256
#define MAX_LIST_SIZE               256 // 64
#define MAX_BITMAP_SIZE             200
#define MAX_BUFFER_SIZE             (4096<<2)

#define NextTrie_GetPtr(p)  (p & ((~(uintptr_t)0)-3))
#define NextTrie_GetKind(p) (p & 3)
#define NextTrie_SetKind(p, kind) (p |= (3&kind))

#define DiskNextTrie_GetPtr(map,offset)  (((uintptr_t)map) + (uintptr_t)(offset & ((~(uintptr_t)0)-3)))
#define DiskNextTrie_GetKind(p) (p & 3)
#define DiskNextTrie_SetKind(p, kind) (p |= (3&kind))

// Use this macro to avoid forgetting to check the pointer before assigning value to it.
#define SetPayload(pointer, value) do { if (pointer) *pointer = value; } while (0)

enum { Nothing = 0, TrieKind = 1, ListKind = 2, CompactTrieKind = 3 };
typedef enum { FailedInsert = 0, NewTerm = 1, ExistingTerm = 2 } CFBTInsertCode; 

#pragma pack (1)
typedef uintptr_t NextTrie;

typedef struct _TrieLevel {
    NextTrie slots[CHARACTER_SET_SIZE];
    uint32_t weight;        
    uint32_t payload;
} TrieLevel;
typedef TrieLevel *TrieLevelRef;

typedef struct _MapTrieLevel {
    uint32_t slots[CHARACTER_SET_SIZE];
    uint32_t payload;
} MapTrieLevel;
typedef MapTrieLevel *MapTrieLevelRef;

typedef struct _CompactMapTrieLevel {
    uint64_t bitmap[CHARACTER_SET_SIZE / 64];
    uint32_t payload;
    uint32_t slots[];
} CompactMapTrieLevel;
typedef CompactMapTrieLevel *CompactMapTrieLevelRef;

typedef struct _ListNode {
    struct _ListNode *next;
    uint32_t weight;
    uint32_t payload;
    uint16_t length;
    UInt8 string[];
}* ListNodeRef;

typedef struct _Page {
    uint32_t length;
    char data[];
} Page;

typedef struct _PageEntryPacked {
    uint8_t pfxLen;
    uint16_t strlen;
    uint32_t payload;
    UInt8 string[];
} PageEntryPacked;

typedef struct _PageEntry {
    uint16_t strlen;
    uint32_t payload;
    UInt8 string[];
} PageEntry;

typedef struct _TrieHeader {
    uint32_t signature;
    uint32_t rootOffset; 
    uint32_t count; 
    uint32_t size; 
    uint32_t flags;
    uint64_t reserved[16];
} TrieHeader;

typedef struct _TrieCursor {
    uint64_t signature;
    uint64_t counter;
    NextTrie next;
    uint32_t keylen;
    uint32_t prefixlen;
    const uint8_t *prefix;
    uint8_t key[MAX_KEY_LENGTH];
} TrieCursor;

typedef struct _MapCursor {
    uint64_t signature;
    TrieHeader *header;
    uint32_t next;
    uint32_t prefixlen;
    uint32_t keylen;
    const uint8_t *prefix;
    uint8_t key[MAX_STRING_SIZE*4];
} MapCursor;

typedef struct _CompactMapCursor {
    uint32_t next;
    uint32_t entryOffsetInPage;
    uint32_t offsetInEntry;
    uint32_t payload;
    // On a page, the first entry could has 0 strlen. So we need this variable to tell us whether
    // the cursor is merely pointing at the beginning of the page, or the first entry.
    // For example, if the trie contains "ab" and "abc", where "a" is stored on an array level,
    // while "b" and "bc" are stored on a page level. If we creat a cursor for string "a", this cursor
    // will point at the beginning of the page, but not at any particular key. The both entryOffsetInPage and
    // offsetInEntry fields of the cursor are set to 0 in this case. Now if we add "a" to the 
    // trie. the page level will actually contains three entries. The first entry corresponds to string "a".
    // That entry has 0 strlen value. If we creat a cursor for string "a" again, this cursor will
    // point at the first entry on the page. But the entryOffsetInPage and offsetInEntry fields are still
    // set to 0s. So we need an additional variable to make distinction between these two situations.
    BOOL isOnPage;
} CompactMapCursor;
typedef struct _CompactMapCursor *MapCursorRef;

enum {
    _kCFBurstTrieCursorTrieType = 0,
    _kCFBurstTrieCursorMapType
};

typedef struct _CFBurstTrieCursor {
    CompactMapCursor mapCursor;
    CFIndex cursorType;
    CFBurstTrieRef trie;
} _CFBurstTrieCursor;

// ** Legacy
typedef struct _DiskTrieLevel {
    uint32_t slots[CHARACTER_SET_SIZE];
    uint32_t weight;        
    uint32_t payload;
} DiskTrieLevel;
typedef DiskTrieLevel *DiskTrieLevelRef;

typedef struct _CompactDiskTrieLevel {
    uint64_t bitmap[CHARACTER_SET_SIZE / 64]; // CHARACTER_SET_SIZE / 64bits per word 
    uint32_t weight;        
    uint32_t payload;
    uint32_t slots[];
} CompactDiskTrieLevel;
typedef CompactDiskTrieLevel *CompactDiskTrieLevelRef;

typedef struct _StringPage {
    uint32_t length;
    char data[];
} StringPage;

typedef struct _StringPageEntryPacked {
    uint8_t pfxLen;
    uint16_t strlen;    // make uint8_t if possible
    uint32_t payload;
    char string[];
} StringPageEntryPacked;

typedef struct _StringPageEntry {
    uint16_t strlen;    // make uint8_t if possible
    uint32_t payload;
    char string[];
} StringPageEntry;

typedef struct _fileHeader {
    uint32_t signature;
    uint32_t rootOffset; 
    uint32_t count; 
    uint32_t size; 
    uint32_t flags;
} fileHeader;
// **
#pragma pack()

struct _CFBurstTrie {
    union {
        TrieLevel root;
        DiskTrieLevel diskRoot;
        MapTrieLevel maproot;
    };    
    char *mapBase;
    uint32_t mapSize;
    uint32_t mapOffset;
    uint32_t cflags;
    uint32_t count;
    uint32_t containerSize;
    int retain;
#if DEPLOYMENT_TARGET_WINDOWS
    HANDLE mapHandle;
    HANDLE mappedFileHandle;
#endif
};

#if 0
#pragma mark -
#pragma mark Forward declarations
#endif

typedef struct _TraverseContext {
    void *context;
    void (*callback)(void*, const UInt8*, uint32_t, uint32_t);
} TraverseContext;

static bool foundKey(void *context, const uint8_t *key, uint32_t payload, bool exact)
{
    if (context != NULL) {
        TraverseContext *ctx = (TraverseContext *)context;
        if (ctx->context && ctx->callback) {
            ctx->callback(ctx->context, key, 1, payload);
        }
    }
    return false;
}

void CFBurstTrieTraverseWithCursor(CFBurstTrieRef trie, const uint8_t *prefix, uint32_t prefixLen, void **cursor, void *ctx, bool (*callback)(void *, const uint8_t *, uint32_t, bool));

static CFBTInsertCode addCFBurstTrieLevel(CFBurstTrieRef trie, TrieLevelRef root, const uint8_t *key, uint32_t keylen, uint32_t weight, uint32_t payload);

static void findCFBurstTrieLevel(CFBurstTrieRef trie, TrieCursor *cursor, bool exactmatch, void *ctx, bool (*callback)(void*, const uint8_t*, uint32_t, bool));
static void findCFBurstTrieMappedLevel(CFBurstTrieRef trie, MapCursor *cursor, bool exactmatch, void *ctx, bool (*callback)(void*, const uint8_t*, uint32_t, bool));
static void traverseCFBurstTrieLevel(CFBurstTrieRef trie, TrieLevelRef root, TrieCursor *cursor, bool exactmatch, void *ctx, bool (*callback)(void *, const uint8_t *, uint32_t, bool));
static void traverseCFBurstTrieMappedLevel(CFBurstTrieRef trie, MapTrieLevelRef root, MapCursor *cursor, bool exactmatch, void *ctx, bool (*callback)(void *, const uint8_t *, uint32_t, bool));
static void traverseCFBurstTrieCompactMappedLevel(CFBurstTrieRef trie, CompactMapTrieLevelRef root, MapCursor *cursor, bool exactmatch, void *ctx, bool (*callback)(void *, const uint8_t *, uint32_t, bool));
static void traverseCFBurstTrieWithCursor(CFBurstTrieRef trie, const uint8_t *prefix, uint32_t prefixLen, void **cursor, bool exactmatch, void *ctx, bool (*callback)(void *, const uint8_t *, uint32_t, bool));

static size_t serializeCFBurstTrie(CFBurstTrieRef trie, size_t start_offset, int fd);

static Boolean burstTrieMappedFind(DiskTrieLevelRef trie, char *map, const UInt8 *key, uint32_t length, uint32_t *payload, bool prefix);
static Boolean burstTrieMappedPageFind(StringPage *page, const UInt8 *key, uint32_t length, uint32_t *payload, bool prefix);
static Boolean burstTrieCompactTrieMappedFind(CompactDiskTrieLevelRef trie, char *map, const UInt8 *key, uint32_t length, uint32_t *payload, bool prefix);

static void destroyCFBurstTrie(CFBurstTrieRef trie);
static void finalizeCFBurstTrie(TrieLevelRef trie);
static void finalizeCFBurstTrieList(ListNodeRef node); 

static int nodeWeightCompare(const void *a, const void *b);
static int nodeStringCompare(const void *a, const void *b);

static bool foundKey(void *context, const uint8_t *key, uint32_t payload, bool exact);
static bool containsKey(void *context, const uint8_t *key, uint32_t payload, bool exact);

static CFIndex burstTrieConvertCharactersToUTF8(UniChar *chars, CFIndex numChars, UInt8 *buffer);

static Boolean advanceMapCursor(CFBurstTrieRef trie, CompactMapCursor *cursor, const UInt8* bytes, CFIndex length);
static Boolean getMapCursorPayload(CFBurstTrieRef trie, const CompactMapCursor *cursor, uint32_t *payload);
static void copyMapCursor(const CompactMapCursor *source, CompactMapCursor* destination);
static Boolean areMapCursorsEqual(const CompactMapCursor *lhs, const CompactMapCursor *rhs);
static void traverseFromMapCursor(CFBurstTrieRef trie, CompactMapCursor *cursor, UInt8* bytes, uint32_t capacity, uint32_t length, Boolean *stop, void *ctx, CFBurstTrieTraversalCallback callback);
static Boolean getMapCursorPayloadFromPackedPageEntry(PageEntryPacked *entry, const CompactMapCursor *cursor, uint32_t *payload);
static Boolean getMapCursorPayloadFromPageEntry(PageEntry *entry, const CompactMapCursor *cursor, uint32_t *payload);

CFBurstTrieRef CFBurstTrieCreateWithOptions(CFDictionaryRef options) {
    CFBurstTrieRef trie = NULL;
    trie = (CFBurstTrieRef) calloc(1, sizeof(struct _CFBurstTrie));
    trie->containerSize = MAX_LIST_SIZE;

    CFNumberRef valueAsCFNumber;
    if (CFDictionaryGetValueIfPresent(options, kCFBurstTrieCreationOptionNameContainerSize, (const void **)&valueAsCFNumber)) {
        int value;
        CFNumberGetValue(valueAsCFNumber, kCFNumberIntType, &value);
        trie->containerSize = value > 2 && value < 4096 ? value : MAX_LIST_SIZE;
    }
    trie->retain = 1;
    return trie;
}

CFBurstTrieRef CFBurstTrieCreate() {
    CFBurstTrieRef trie = NULL;
    int listSize = MAX_LIST_SIZE;
    CFNumberRef value = CFNumberCreate(kCFAllocatorDefault, kCFNumberIntType, &listSize);
    CFMutableDictionaryRef options = CFDictionaryCreateMutable(kCFAllocatorDefault, 1, NULL, NULL);
    CFDictionarySetValue(options, kCFBurstTrieCreationOptionNameContainerSize, value);
    trie = CFBurstTrieCreateWithOptions(options);
    CFRelease(value);
    CFRelease(options);
    return trie;
}

CFBurstTrieRef CFBurstTrieCreateFromFile(CFStringRef path) {
    struct statinfo sb;
    char filename[PATH_MAX];
    int fd;
    
    /* Check valid path name */
    if (!CFStringGetCString(path, filename, PATH_MAX, kCFStringEncodingUTF8)) return NULL;
    
    /* Check if file exists */
    if (stat(filename, &sb) != 0) return NULL;

    /* Check if file can be opened */
    if ((fd=open(filename, CF_OPENFLGS|O_RDONLY)) < 0) return NULL;
    
#if DEPLOYMENT_TARGET_WINDOWS
    HANDLE mappedFileHandle = (HANDLE)_get_osfhandle(fd);   
    if (!mappedFileHandle) return NULL;
    
    HANDLE mapHandle = CreateFileMapping(mappedFileHandle, NULL, PAGE_READONLY, 0, 0, NULL);
    if (!mapHandle) return NULL;
    
    char *map = (char *)MapViewOfFile(mapHandle, FILE_MAP_READ, 0, 0, sb.st_size);
    if (!map) return NULL;
#else            
    char *map = mmap(0, sb.st_size, PROT_READ, MAP_FILE|MAP_SHARED, fd, 0);
#endif
    
    CFBurstTrieRef trie = NULL;
    TrieHeader *header = (TrieHeader *)map;

    if (((uint32_t*)map)[0] == 0xbabeface) {
        trie = (CFBurstTrieRef) calloc(1, sizeof(struct _CFBurstTrie));
        trie->mapBase = map;
        trie->mapSize = CFSwapInt32LittleToHost(sb.st_size); 
        trie->mapOffset = CFSwapInt32LittleToHost(((fileHeader*)trie->mapBase)->rootOffset);
        trie->cflags = CFSwapInt32LittleToHost(((fileHeader*)trie->mapBase)->flags);
        trie->count = CFSwapInt32LittleToHost(((fileHeader*)trie->mapBase)->count);
        trie->retain = 1;
#if DEPLOYMENT_TARGET_WINDOWS
        trie->mappedFileHandle = mappedFileHandle;
        trie->mapHandle = mapHandle;
#else
        // On Windows, the file being mapped must stay open as long as the map exists. Don't close it early. Other platforms close it here.
        close(fd);
#endif
    } else if (header->signature == 0xcafebabe || header->signature == 0x0ddba11) {
        trie = (CFBurstTrieRef) calloc(1, sizeof(struct _CFBurstTrie));
        trie->mapBase = map;
        trie->mapSize = CFSwapInt32LittleToHost(sb.st_size); 
        trie->cflags = CFSwapInt32LittleToHost(header->flags);
        trie->count = CFSwapInt32LittleToHost(header->count);
        trie->retain = 1;
#if DEPLOYMENT_TARGET_WINDOWS
        trie->mappedFileHandle = mappedFileHandle;
        trie->mapHandle = mapHandle;
#else
        // On Windows, the file being mapped must stay open as long as the map exists. Don't close it early. Other platforms close it here.
        close(fd);
#endif
    } else {
        close(fd);
    }
    return trie;
}

CFBurstTrieRef CFBurstTrieCreateFromMapBytes(char *mapBase) {
    CFBurstTrieRef trie = NULL;
    TrieHeader *header = (TrieHeader *)mapBase;

    if (mapBase && ((uint32_t*)mapBase)[0] == 0xbabeface) {
        trie = (CFBurstTrieRef) malloc(sizeof(struct _CFBurstTrie));
        trie->mapBase = mapBase;
        trie->mapSize = CFSwapInt32LittleToHost(((fileHeader*)trie->mapBase)->size);
        trie->mapOffset = CFSwapInt32LittleToHost(((fileHeader*)trie->mapBase)->rootOffset);
        trie->cflags = CFSwapInt32LittleToHost(((fileHeader*)trie->mapBase)->flags);
        trie->count = CFSwapInt32LittleToHost(((fileHeader*)trie->mapBase)->count);
        trie->retain = 1;
    } else if (mapBase && (header->signature == 0xcafebabe || header->signature == 0x0ddba11)) {
        trie = (CFBurstTrieRef) malloc(sizeof(struct _CFBurstTrie));
        trie->mapBase = mapBase;
        trie->mapSize = CFSwapInt32LittleToHost(header->size);
        trie->cflags = CFSwapInt32LittleToHost(header->flags);
        trie->count = CFSwapInt32LittleToHost(header->count);
        trie->retain = 1;
    }
    return trie;
}

Boolean CFBurstTrieInsert(CFBurstTrieRef trie, CFStringRef term, CFRange termRange, CFIndex payload) {
    return CFBurstTrieAddWithWeight(trie, term, termRange, 1, (uint32_t)payload);
}

Boolean CFBurstTrieAdd(CFBurstTrieRef trie, CFStringRef term, CFRange termRange, uint32_t payload) {
    return CFBurstTrieAddWithWeight(trie, term, termRange, 1, payload);
}

Boolean CFBurstTrieInsertCharacters(CFBurstTrieRef trie, UniChar *chars, CFIndex numChars, CFIndex payload) {
    return CFBurstTrieAddCharactersWithWeight(trie, chars, numChars, 1, (uint32_t)payload);
}

Boolean CFBurstTrieAddCharacters(CFBurstTrieRef trie, UniChar *chars, CFIndex numChars, uint32_t payload) {
    return CFBurstTrieAddCharactersWithWeight(trie, chars, numChars, 1, payload);
}

Boolean CFBurstTrieInsertUTF8String(CFBurstTrieRef trie, UInt8 *chars, CFIndex numChars, CFIndex payload) {
    return CFBurstTrieAddUTF8StringWithWeight(trie, chars, numChars, 1, (uint32_t)payload);
}

Boolean CFBurstTrieAddUTF8String(CFBurstTrieRef trie, UInt8 *chars, CFIndex numChars, uint32_t payload) {
    return CFBurstTrieAddUTF8StringWithWeight(trie, chars, numChars, 1, payload);
}

Boolean CFBurstTrieInsertWithWeight(CFBurstTrieRef trie, CFStringRef term, CFRange termRange, CFIndex weight, CFIndex payload) {
    return CFBurstTrieAddWithWeight(trie, term, termRange, weight, (uint32_t)payload);
}

Boolean CFBurstTrieAddWithWeight(CFBurstTrieRef trie, CFStringRef term, CFRange termRange, uint32_t weight, uint32_t payload) {
    Boolean success = false;
    CFIndex size = MAX_STRING_ALLOCATION_SIZE;
    CFIndex bytesize = termRange.length * 4; //** 4-byte max character size
    if (!trie->mapBase && termRange.length < MAX_STRING_SIZE && payload > 0) {
        CFIndex length;
        UInt8 buffer[MAX_STRING_ALLOCATION_SIZE + 1];
        UInt8 *key = buffer;
        if (bytesize >= size) {
            size = bytesize; 
            key = (UInt8 *) malloc(sizeof(UInt8) * size + 1);
        }
        CFStringGetBytes(term, termRange, kCFStringEncodingUTF8, (UInt8)'-', (Boolean)0, key, size, &length);
        key[length] = 0;
        
        success = CFBurstTrieAddUTF8StringWithWeight(trie, key, length, weight, payload);
        if (buffer != key) free(key);
    }
    return success;
}

Boolean CFBurstTrieInsertCharactersWithWeight(CFBurstTrieRef trie, UniChar *chars, CFIndex numChars, CFIndex weight, CFIndex payload) {
    return CFBurstTrieAddCharactersWithWeight(trie, chars, numChars, weight, (uint32_t)payload);
}

Boolean CFBurstTrieAddCharactersWithWeight(CFBurstTrieRef trie, UniChar *chars, CFIndex numChars, uint32_t weight, uint32_t payload) {
    Boolean success = false;
    CFIndex size = MAX_STRING_ALLOCATION_SIZE;
    CFIndex bytesize = numChars * 4; //** 4-byte max character size
    if (!trie->mapBase && numChars < MAX_STRING_SIZE && payload > 0) {
        CFIndex length;
        UInt8 buffer[MAX_STRING_ALLOCATION_SIZE + 1];
        UInt8 *key = buffer;
        if (bytesize >= size) {
            size = bytesize; 
            key = (UInt8 *) malloc(sizeof(UInt8) * size + 1);
        }
        length = burstTrieConvertCharactersToUTF8(chars, numChars, key);
        key[length] = 0;
        
        success = CFBurstTrieAddUTF8StringWithWeight(trie, key, length, weight, payload);
        if (buffer != key) free(key);
    }
    return success;
}

Boolean CFBurstTrieInsertUTF8StringWithWeight(CFBurstTrieRef trie, UInt8 *chars, CFIndex numChars, CFIndex weight, CFIndex payload) {
    return CFBurstTrieAddUTF8StringWithWeight(trie, chars, numChars, weight, (uint32_t)payload);
}

Boolean CFBurstTrieAddUTF8StringWithWeight(CFBurstTrieRef trie, UInt8 *chars, CFIndex numChars, uint32_t weight, uint32_t payload) {
    CFBTInsertCode code = FailedInsert;
    
    if (!trie->mapBase && numChars < MAX_STRING_SIZE*4 && payload > 0) {
        code = addCFBurstTrieLevel(trie, &trie->root, chars, numChars, weight, payload);
        if (code == NewTerm) trie->count++;
    }
    return code > FailedInsert;
}

Boolean CFBurstTrieFind(CFBurstTrieRef trie, CFStringRef term, CFRange termRange, CFIndex *payload) {
    uint32_t p;
    if (CFBurstTrieContains(trie, term, termRange, &p)) {
        SetPayload(payload, p);
        return true;
    }
    return false;
}

Boolean CFBurstTrieContains(CFBurstTrieRef trie, CFStringRef term, CFRange termRange, uint32_t *payload) {
    Boolean success = false;
    CFIndex size = MAX_STRING_ALLOCATION_SIZE;
    CFIndex bytesize = termRange.length * 4; //** 4-byte max character size
    if (termRange.length < MAX_STRING_SIZE) {
        CFIndex length;
        UInt8 buffer[MAX_STRING_ALLOCATION_SIZE+1];
        UInt8 *key = buffer;
        if (bytesize >= size) {
            size = bytesize;
            key = (UInt8 *) malloc(sizeof(UInt8) * size + 1);
        }
        CFStringGetBytes(term, termRange, kCFStringEncodingUTF8, (UInt8)'-', (Boolean)0, key, size, &length);
        key[length] = 0;
        
        success = CFBurstTrieContainsUTF8String(trie, key, length, payload);
        if (buffer != key) free(key);
    }
    return success;
}

Boolean CFBurstTrieFindCharacters(CFBurstTrieRef trie, UniChar *chars, CFIndex numChars, CFIndex *payload) {
    uint32_t p;
    if (CFBurstTrieContainsCharacters(trie, chars, numChars, &p)) {
        SetPayload(payload, p);
        return true;
    }
    return false;
}

Boolean CFBurstTrieContainsCharacters(CFBurstTrieRef trie, UniChar *chars, CFIndex numChars, uint32_t *payload) {
    Boolean success = false;
    CFIndex size = MAX_STRING_ALLOCATION_SIZE;
    CFIndex bytesize = numChars * 4; //** 4-byte max character size
    if (numChars < MAX_STRING_SIZE) {
        CFIndex length;
        UInt8 buffer[MAX_STRING_ALLOCATION_SIZE + 1];
        UInt8 *key = buffer;
        if (bytesize >= size) {
            size = bytesize;
            key = (UInt8 *) malloc(sizeof(UInt8) * size + 1);
        }
        length = burstTrieConvertCharactersToUTF8(chars, numChars, key);
        key[length] = 0;
        
        success = CFBurstTrieContainsUTF8String(trie, key, length, payload);
        if (buffer != key) free(key);
    }
    return success;
}

Boolean CFBurstTrieFindUTF8String(CFBurstTrieRef trie, UInt8 *key, CFIndex length, CFIndex *payload) {
    uint32_t p;
    if (CFBurstTrieContainsUTF8String(trie, key, length, &p)) {
        SetPayload(payload, p);
        return true;
    }
    return false;
}

Boolean CFBurstTrieContainsUTF8String(CFBurstTrieRef trie, UInt8 *key, CFIndex length, uint32_t *payload) {
    Boolean success = false;
    if (length < MAX_STRING_SIZE) {
        if (trie->mapBase && ((fileHeader *)trie->mapBase)->signature == 0xbabeface) {
            bool prefix = (trie->cflags & kCFBurstTriePrefixCompression);
            success = burstTrieMappedFind((DiskTrieLevelRef)(trie->mapBase+CFSwapInt32LittleToHost((((uint32_t*)trie->mapBase)[1]))), trie->mapBase, key, length, payload, prefix);
        } else if (trie->mapBase && trie->cflags & (kCFBurstTriePrefixCompression | kCFBurstTrieSortByKey)) {
            _CFBurstTrieCursor cursor;
            if (!CFBurstTrieSetCursorForBytes(trie, &cursor, key, length))
                return FALSE;
            return CFBurstTrieCursorGetPayload(&cursor, payload);
        } else {
            uint32_t found = 0;
            void *cursor = 0;
            traverseCFBurstTrieWithCursor(trie, key, length, &cursor, true, &found, containsKey);
            if (found) SetPayload(payload, found);
            success = found > 0;
        }
    }
    return success;
}

Boolean CFBurstTrieSerialize(CFBurstTrieRef trie, CFStringRef path, CFBurstTrieOpts opts) {    
    Boolean success = false;    
    if (trie->mapBase) {
        return success;
    } else {
        int fd;
        char filename[PATH_MAX];
        
        /* Check valid path name */
        if (!CFStringGetCString(path, filename, PATH_MAX, kCFStringEncodingUTF8)) return success;
        
        /* Check if file can be opened */
        if ((fd=open(filename, CF_OPENFLGS|O_RDWR|O_CREAT|O_TRUNC, S_IRUSR|S_IWUSR)) < 0) return success;
        
        if (CFBurstTrieSerializeWithFileDescriptor(trie, fd, opts)) success = true;
        
        close(fd);
    }
    return success;
}

Boolean CFBurstTrieSerializeWithFileDescriptor(CFBurstTrieRef trie, int fd, CFBurstTrieOpts opts) {
    Boolean success = false;
    if (!trie->mapBase && fd >= 0) {
        off_t start_offset = lseek(fd, 0, SEEK_END);

        trie->cflags = opts;
        trie->mapSize = serializeCFBurstTrie(trie, start_offset, fd);
        
#if DEPLOYMENT_TARGET_WINDOWS
        HANDLE mappedFileHandle = (HANDLE)_get_osfhandle(fd);
        // We need to make sure we have our own handle to keep this file open as long as the mmap lasts
        DuplicateHandle(GetCurrentProcess(), mappedFileHandle, GetCurrentProcess(), &mappedFileHandle, 0, 0, DUPLICATE_SAME_ACCESS);
        HANDLE mapHandle = CreateFileMapping(mappedFileHandle, NULL, PAGE_READONLY, 0, 0, NULL);
        if (!mapHandle) return NULL;
        char *map = (char *)MapViewOfFile(mapHandle, FILE_MAP_READ, 0, start_offset, trie->mapSize);
        if (!map) return NULL;
        trie->mapBase = map;
        trie->mapHandle = mapHandle;
        trie->mappedFileHandle = mappedFileHandle;
#else
        trie->mapBase = mmap(0, trie->mapSize, PROT_READ, MAP_FILE|MAP_SHARED, fd, start_offset);
#endif
        success = true;
    }
    
    return success;
}

void CFBurstTrieTraverse(CFBurstTrieRef trie, void *ctx, void (*callback)(void*, const UInt8*, uint32_t, uint32_t)) {
    TrieHeader *header = (TrieHeader *)trie->mapBase;
    if (!trie->mapBase || (header->signature == 0xcafebabe || header->signature == 0x0ddba11)) {
        void *cursor = 0;
        TraverseContext context;
        context.context = ctx;
        context.callback = callback;
        traverseCFBurstTrieWithCursor(trie, (const uint8_t *)"", 0, &cursor, false, &context, foundKey);
    }
}


void CFBurstTrieTraverseWithCursor(CFBurstTrieRef trie, const uint8_t *prefix, uint32_t prefixLen, void **cursor, void *ctx, bool (*callback)(void *, const uint8_t *, uint32_t, bool))
{
    traverseCFBurstTrieWithCursor(trie, prefix, prefixLen, cursor, false, ctx, callback);
}

void CFBurstTriePrint(CFBurstTrieRef trie) {

}

CFIndex CFBurstTrieGetCount(CFBurstTrieRef trie) {
    return trie->count;
}

CFBurstTrieRef CFBurstTrieRetain(CFBurstTrieRef trie) {
    trie->retain++;
    return trie;
}

void CFBurstTrieRelease(CFBurstTrieRef trie) {
    trie->retain--;
    if (trie->retain == 0) destroyCFBurstTrie(trie);
    return;
}

Boolean CFBurstTrieSetCursorForBytes(CFBurstTrieRef trie, CFBurstTrieCursorRef cursor, const UInt8* bytes, CFIndex length)
{
    if (!trie->mapBase || !(trie->cflags & (kCFBurstTriePrefixCompression | kCFBurstTrieSortByKey))) {
        //fprintf(stderr, "CFBurstTrieCreateCursorForBytes() only support file based trie in prefix compression format.\n");
        return FALSE;
    }

    TrieHeader *header = (TrieHeader*)trie->mapBase;
    if (length < 0 || !trie)
        return FALSE;

    cursor->trie = trie;
    if (trie->mapBase) {
        cursor->cursorType = _kCFBurstTrieCursorMapType;
        cursor->mapCursor.next = header->rootOffset;
        cursor->mapCursor.isOnPage = FALSE;
        cursor->mapCursor.entryOffsetInPage = 0;
        cursor->mapCursor.offsetInEntry = 0;
        cursor->mapCursor.payload = 0;
    } else
        assert(false);

    if (!bytes || length == 0)
        return TRUE;

    return CFBurstTrieCursorAdvanceForBytes(cursor, bytes, length);
}


CFBurstTrieCursorRef CFBurstTrieCreateCursorForBytes(CFBurstTrieRef trie, const UInt8* bytes, CFIndex length)
{
    CFBurstTrieCursorRef cursor = (CFBurstTrieCursorRef)calloc(sizeof(_CFBurstTrieCursor), 1);
    if (!CFBurstTrieSetCursorForBytes(trie, cursor, bytes, length)) {
        CFBurstTrieCursorRelease(cursor);
        return NULL;
    }
    return cursor;
}

CFBurstTrieCursorRef CFBurstTrieCursorCreateByCopy(CFBurstTrieCursorRef cursor)
{
    if (!cursor)
        return NULL;

    CFBurstTrieCursorRef newCursor = (CFBurstTrieCursorRef)calloc(sizeof(_CFBurstTrieCursor), 1);
    switch (cursor->cursorType) {
        case _kCFBurstTrieCursorMapType:
            copyMapCursor(&cursor->mapCursor, &newCursor->mapCursor);
            break;
        case _kCFBurstTrieCursorTrieType:
            assert(false);
            break;
    }
    newCursor->cursorType = cursor->cursorType;
    newCursor->trie = cursor->trie;
    return newCursor;
}

Boolean CFBurstTrieCursorIsEqual(CFBurstTrieCursorRef lhs, CFBurstTrieCursorRef rhs)
{
    if (lhs->trie != rhs->trie || lhs->cursorType != rhs->cursorType)
        return FALSE;

    if (lhs->cursorType == _kCFBurstTrieCursorMapType)
        return areMapCursorsEqual(&lhs->mapCursor, &rhs->mapCursor);
    else
        return FALSE;
}

Boolean CFBurstTrieCursorAdvanceForBytes(CFBurstTrieCursorRef cursor, const UInt8* bytes, CFIndex length)
{
    switch (cursor->cursorType) {
        case _kCFBurstTrieCursorMapType: {
            CompactMapCursor tempCursor;
            copyMapCursor(&cursor->mapCursor, &tempCursor);
            if (advanceMapCursor(cursor->trie, (CompactMapCursor*)&cursor->mapCursor, bytes, length))
                return TRUE;
            else {
                copyMapCursor(&tempCursor, &cursor->mapCursor);
                return FALSE;
            }
        }
        case _kCFBurstTrieCursorTrieType:
            return FALSE;
    }
    return FALSE;
}

Boolean CFBurstTrieCursorGetPayload(CFBurstTrieCursorRef cursor, uint32_t *payload)
{
    switch (cursor->cursorType) {
        case _kCFBurstTrieCursorMapType:
            return getMapCursorPayload(cursor->trie, (CompactMapCursor*)&cursor->mapCursor, payload);
        case _kCFBurstTrieCursorTrieType:
            return FALSE;
    }
    return FALSE;
}

void CFBurstTrieCursorRelease(CFBurstTrieCursorRef cursor)
{
    if (!cursor)
        return;
    free(cursor);
}

void CFBurstTrieTraverseFromCursor(CFBurstTrieCursorRef cursor, void *ctx, CFBurstTrieTraversalCallback callback)
{
    if (!cursor)
        return;

    UInt8 *bytes = (UInt8*)calloc(1, MAX_KEY_LENGTH);
    uint32_t capacity = MAX_KEY_LENGTH;
    uint32_t length = 0;
    Boolean stop = FALSE;
    switch (cursor->cursorType) {
        case _kCFBurstTrieCursorMapType: {
            CompactMapCursor tempCursor;
            copyMapCursor(&cursor->mapCursor, &tempCursor);
            traverseFromMapCursor(cursor->trie, &tempCursor, bytes, capacity,length, &stop, ctx, callback);
            break;
        }
        case _kCFBurstTrieCursorTrieType:
            break;
    }
    free(bytes);
}

#if 0
#pragma mark -
#pragma mark Insertion
#endif

static ListNodeRef makeCFBurstTrieListNode(const uint8_t *key, uint32_t keylen, uint32_t weight, uint32_t payload) {
    ListNodeRef node = (ListNodeRef) calloc(1, sizeof(*node) + keylen + 1);
    memcpy(node->string, key, keylen);
    node->string[keylen] = 0;
    node->next = 0;
    node->length = keylen;
    node->weight = weight;
    node->payload = payload;
    return node;
}

static void addCFBurstTrieBurstLevel(CFBurstTrieRef trie, TrieLevelRef root, const uint8_t *key, uint32_t keylen, uint32_t weight, uint32_t payload) {
    if (keylen) {
        NextTrie next = root->slots[*key];
        ListNodeRef newNode = makeCFBurstTrieListNode(key+1, keylen-1, weight, payload);
        newNode->weight = weight;
        newNode->next = (ListNodeRef) NextTrie_GetPtr(next);
        next = (uintptr_t) newNode;
        NextTrie_SetKind(next, ListKind);
        root->slots[*key] = next;
    } else { 
        // ** Handle payload.
        root->weight = weight;
        root->payload = payload;
    }
}

static TrieLevelRef burstCFBurstTrieLevel(CFBurstTrieRef trie, ListNodeRef list, uint32_t listCount) {
    TrieLevelRef newLevel = (TrieLevelRef) calloc(1, sizeof(struct _TrieLevel));
    while (list) {
        addCFBurstTrieBurstLevel(trie, newLevel, list->string, list->length, list->weight, list->payload);
        ListNodeRef temp = list;
        list = list->next;
        free(temp);
    }
    return newLevel;
}

static CFBTInsertCode addCFBurstTrieListNode(CFBurstTrieRef trie, ListNodeRef list, const uint8_t *key, uint32_t keylen, uint32_t weight, uint32_t payload, uint32_t *listCount)
{
    CFBTInsertCode code = FailedInsert;
    uint32_t count = 1;
    
    ListNodeRef last = list;
    while (list) {
        if (list->length == keylen && memcmp(key, list->string, keylen) == 0) {
            list->weight += weight;
            list->payload = payload;
            code = ExistingTerm;
            break;
        } else {
            count++;
            last = list;
            list = list->next;
        }
    }
    
    if (!list) {
        last->next = makeCFBurstTrieListNode(key, keylen, weight, payload);
        code = NewTerm;
    }
    
    *listCount = count;
    return code;
}

static CFBTInsertCode addCFBurstTrieLevel(CFBurstTrieRef trie, TrieLevelRef root, const uint8_t *key, uint32_t keylen, uint32_t weight, uint32_t payload)
{
    CFBTInsertCode code = FailedInsert;
    if (keylen) {
        NextTrie next = root->slots[*key];
        if (NextTrie_GetKind(next) == TrieKind) {
            TrieLevelRef nextLevel = (TrieLevelRef) NextTrie_GetPtr(next);
            code = addCFBurstTrieLevel(trie, nextLevel, key+1, keylen-1, weight, payload);
        } else {
            if (NextTrie_GetKind(next) == ListKind) {
                uint32_t listCount;
                ListNodeRef listNode = (ListNodeRef) NextTrie_GetPtr(next);
                code = addCFBurstTrieListNode(trie, listNode, key+1, keylen-1, weight, payload, &listCount);
                if (listCount > trie->containerSize) {
                    next = (uintptr_t) burstCFBurstTrieLevel(trie, listNode, listCount);
                    NextTrie_SetKind(next, TrieKind);
                }
            } else {
                // ** Make a new list node
                next = (uintptr_t) makeCFBurstTrieListNode(key+1, keylen-1, weight, payload);
                NextTrie_SetKind(next, ListKind);
                code = NewTerm;
            }
            root->slots[*key] = next;
        }
    } else {
        // ** Handle payload
        if (!root->weight) code = NewTerm;
        else code = ExistingTerm;
        root->weight += weight;
        root->payload = payload;
    }
    
    return code;
}
#if 0
#pragma mark -
#pragma mark Searching
#endif
static void findCFBurstTrieList(CFBurstTrieRef trie, TrieCursor *cursor, void *ctx, bool (*callback)(void*, const uint8_t*, uint32_t, bool))
{
    ListNodeRef list = (ListNodeRef)NextTrie_GetPtr(cursor->next);
    int len = cursor->prefixlen-cursor->keylen;
    len = len <= 0 ? 0 : len;
    while (list) {
        int lencompare = list->length-len;
        if (list->length >= len && 
            (len == 0 || memcmp(list->string, cursor->prefix+cursor->keylen, len) == 0)) {
            memcpy(cursor->key+cursor->keylen, list->string, list->length);
            cursor->key[cursor->keylen+list->length] = 0;
            cursor->next = (NextTrie)list;
            if (list->payload && callback(ctx, cursor->key, list->payload, lencompare==0)) return;
        }
        list = list->next;
    }
}

static void findCFBurstTrieMappedPage(CFBurstTrieRef trie, MapCursor *cursor, void *ctx, bool (*callback)(void*, const uint8_t*, uint32_t, bool))
{
    Page *page = (Page *)DiskNextTrie_GetPtr(trie->mapBase, cursor->next);
    uint32_t end = page->length;
    uint32_t cur = 0;
    int len = cursor->prefixlen-cursor->keylen;
    len = len <= 0 ? 0 : len;
    if (trie->cflags & kCFBurstTriePrefixCompression) {
        uint8_t pfx[CHARACTER_SET_SIZE];
        PageEntryPacked *lastEntry = 0;
        while (cur < end) {
            PageEntryPacked *entry = (PageEntryPacked *)&page->data[cur];
            int lencompare = (entry->strlen+entry->pfxLen)-len;
            if (lastEntry && entry->pfxLen>lastEntry->pfxLen) memcpy(pfx+lastEntry->pfxLen, lastEntry->string, entry->pfxLen-lastEntry->pfxLen);
            if (lencompare >= 0 &&
                (len == 0 || (__builtin_memcmp(pfx, cursor->prefix+cursor->keylen, entry->pfxLen) == 0 && 
                              __builtin_memcmp(entry->string, cursor->prefix+cursor->keylen+entry->pfxLen, cursor->prefixlen-cursor->keylen-entry->pfxLen) == 0))) {
                memcpy(cursor->key+cursor->keylen, pfx, entry->pfxLen);
                memcpy(cursor->key+cursor->keylen+entry->pfxLen, entry->string, entry->strlen);
                cursor->key[cursor->keylen+entry->pfxLen+entry->strlen] = 0;
                if (entry->payload && callback(ctx, (const uint8_t *)cursor->key, entry->payload, lencompare==0)) return;
            }
            lastEntry = entry;
            cur += sizeof(*entry) + entry->strlen;
        }
    } else {
        while (cur < end) {
            PageEntry *entry = (PageEntry *)&page->data[cur];
            int lencompare = entry->strlen-len;
            if (lencompare >= 0 && __builtin_memcmp(entry->string, cursor->prefix+cursor->keylen, len) == 0) {
                memcpy(cursor->key+cursor->keylen, entry->string, entry->strlen);
                cursor->key[cursor->keylen+entry->strlen] = 0;
                if (entry->payload && callback(ctx, (const uint8_t *)cursor->key, entry->payload, lencompare==0)) return;
            }
            cur += sizeof(*entry) + entry->strlen;
        }
    }
}


static void findCFBurstTrieLevel(CFBurstTrieRef trie, TrieCursor *cursor, bool exactmatch, void *ctx, bool (*callback)(void*, const uint8_t*, uint32_t, bool))
{
    if (cursor->keylen < cursor->prefixlen) {
        cursor->next = ((TrieLevelRef)NextTrie_GetPtr(cursor->next))->slots[cursor->prefix[cursor->keylen]];
        cursor->key[cursor->keylen++] = cursor->prefix[cursor->keylen];
        
        if (NextTrie_GetKind(cursor->next) == TrieKind) {
            findCFBurstTrieLevel(trie, cursor, exactmatch, ctx, callback);
        } else if (NextTrie_GetKind(cursor->next) == ListKind) {
            findCFBurstTrieList(trie, cursor, ctx, callback);
        }
    } else {
        TrieLevelRef root = (TrieLevelRef)NextTrie_GetPtr(cursor->next);
        if (root->payload && callback(ctx, cursor->key, root->payload, cursor->prefixlen==cursor->keylen)) return;
        if (cursor->keylen == cursor->prefixlen && exactmatch) return;
        traverseCFBurstTrieLevel(trie, root, cursor, exactmatch, ctx, callback);
    }
}

static void findCFBurstTrieCompactMappedLevel(CFBurstTrieRef trie, MapCursor *cursor, bool exactmatch, void *ctx, bool (*callback)(void*, const uint8_t*, uint32_t, bool))
{
    CompactMapTrieLevelRef root = (CompactMapTrieLevelRef)DiskNextTrie_GetPtr(trie->mapBase, cursor->next);
    if (cursor->keylen < cursor->prefixlen) {
        uint8_t mykey = *(cursor->prefix+cursor->keylen);
        cursor->key[cursor->keylen++] = *(cursor->prefix+cursor->keylen);
        
        uint8_t slot = mykey / 64;
        uint8_t bit = mykey % 64;
        uint32_t item = 0;
        uint64_t bword = root->bitmap[slot];
        
        if (bword & (1ull << bit)) {
            // ** Count all the set bits up to this bit
            for (int i=0; i < slot; i++) item += __builtin_popcountll(root->bitmap[i]);
            item += __builtin_popcountll(bword & ((1ull << bit)-1));
            uint32_t offset = root->slots[item]; 
            cursor->next = offset;
            
            if (DiskNextTrie_GetKind(offset) == TrieKind) {
                findCFBurstTrieMappedLevel(trie, cursor, exactmatch, ctx, callback);
            } else if (DiskNextTrie_GetKind(offset) == CompactTrieKind) {
                findCFBurstTrieCompactMappedLevel(trie, cursor, exactmatch, ctx, callback);
            } else if (DiskNextTrie_GetKind(offset) == ListKind) {
                findCFBurstTrieMappedPage(trie, cursor, ctx, callback);
            }
        }
    } else {
        if(root->payload && callback(ctx, cursor->key, root->payload, cursor->prefixlen==cursor->keylen)) return;
        if (cursor->keylen == cursor->prefixlen && exactmatch) return;
        traverseCFBurstTrieCompactMappedLevel(trie, root, cursor,  exactmatch, ctx, callback);
    }
}

static void findCFBurstTrieMappedLevel(CFBurstTrieRef trie, MapCursor *cursor, bool exactmatch, void *ctx, bool (*callback)(void*, const uint8_t*, uint32_t, bool))
{
    MapTrieLevelRef root = (MapTrieLevelRef)DiskNextTrie_GetPtr(trie->mapBase, cursor->next);
    if (cursor->keylen < cursor->prefixlen) {
        uint8_t slot = *(cursor->prefix+cursor->keylen);
        cursor->next = root->slots[slot];
        cursor->key[cursor->keylen++] = slot;
        
        if (DiskNextTrie_GetKind(cursor->next) == TrieKind) {
            findCFBurstTrieMappedLevel(trie, cursor, exactmatch, ctx, callback);
        } else if (DiskNextTrie_GetKind(cursor->next) == CompactTrieKind) {
            findCFBurstTrieCompactMappedLevel(trie, cursor, exactmatch, ctx, callback);
        } else if (DiskNextTrie_GetKind(cursor->next) == ListKind) {
            findCFBurstTrieMappedPage(trie, cursor, ctx, callback);
        }
    }  else {
        if (root->payload && callback(ctx, cursor->key, root->payload, cursor->prefixlen==cursor->keylen)) return;
        if (cursor->keylen == cursor->prefixlen && exactmatch) return;
        traverseCFBurstTrieMappedLevel(trie, root, cursor, exactmatch, ctx, callback);
    }
}

static void traverseCFBurstTrieLevel(CFBurstTrieRef trie, TrieLevelRef root, TrieCursor *cursor, bool exactmatch, void *ctx, bool (*callback)(void *, const uint8_t *, uint32_t, bool))
{       
    cursor->key[cursor->keylen] = 0;    
    uint32_t len = cursor->keylen;
    for (int i=0; i < CHARACTER_SET_SIZE; i++) {
        NextTrie next = root->slots[i];
        cursor->keylen = len;
        cursor->key[cursor->keylen++] = i;

        if (NextTrie_GetKind(next) == TrieKind) {
            TrieLevelRef level = (TrieLevelRef)NextTrie_GetPtr(next);
            if (level->payload && callback(ctx, cursor->key, level->payload, cursor->prefixlen==cursor->keylen)) return;
            if (cursor->keylen == cursor->prefixlen && exactmatch) return;
            traverseCFBurstTrieLevel(trie, level, cursor, exactmatch, ctx, callback);
        } else if (NextTrie_GetKind(next) == ListKind) {
            cursor->next = next;
            cursor->key[cursor->keylen] = 0;
            findCFBurstTrieList(trie, cursor, ctx, callback);
        }
    }
}

static void traverseCFBurstTrieMappedLevel(CFBurstTrieRef trie, MapTrieLevelRef root, MapCursor *cursor, bool exactmatch, void *ctx, bool (*callback)(void *, const uint8_t *, uint32_t, bool))
{
    cursor->key[cursor->keylen] = 0;    
    uint32_t len = cursor->keylen;
    
    for (int i=0; i < CHARACTER_SET_SIZE; i++) {
        uint32_t offset = (uint32_t)root->slots[i];
        cursor->keylen = len;
        cursor->key[cursor->keylen++] = i;
        
        if (DiskNextTrie_GetKind(offset) == TrieKind) {
            MapTrieLevelRef level = (MapTrieLevelRef)DiskNextTrie_GetPtr(trie->mapBase, offset);
            if (level->payload && callback(ctx, cursor->key, level->payload, cursor->prefixlen==cursor->keylen)) return;
            if (cursor->keylen == cursor->prefixlen && exactmatch) return;
            traverseCFBurstTrieMappedLevel(trie, level, cursor, exactmatch, ctx, callback);
        } else if (DiskNextTrie_GetKind(offset) == CompactTrieKind) {
            CompactMapTrieLevelRef level = (CompactMapTrieLevelRef)DiskNextTrie_GetPtr(trie->mapBase, offset);
            if (level->payload && callback(ctx, cursor->key, level->payload, cursor->prefixlen==cursor->keylen)) return;
            if (cursor->keylen == cursor->prefixlen && exactmatch) return;
            traverseCFBurstTrieCompactMappedLevel(trie, level, cursor, exactmatch, ctx, callback);
        } else if (DiskNextTrie_GetKind(offset) == ListKind) {
            cursor->next = offset;
            cursor->key[cursor->keylen] = 0;
            findCFBurstTrieMappedPage(trie, cursor, ctx, callback);
        }
    }
}

static void traverseCFBurstTrieCompactMappedLevel(CFBurstTrieRef trie, CompactMapTrieLevelRef root, MapCursor *cursor, bool exactmatch, void *ctx, bool (*callback)(void *, const uint8_t *, uint32_t, bool))
{
    cursor->key[cursor->keylen] = 0;
    uint32_t len = cursor->keylen;    
    for (uint32_t c=0; c < CHARACTER_SET_SIZE; c++) {
        //** This could be optimized to remember what the last slot/item was and not count bits over and over.
        uint8_t mykey = c;
        uint32_t slot = mykey / 64;
        uint32_t bit = mykey % 64;
        uint32_t item = 0;
        uint64_t bword = root->bitmap[slot];
        cursor->keylen = len;
        
        if (bword & (1ull << bit)) {
            // ** Count all the set bits up to this bit
            for (int i=0; i < slot; i++) item += __builtin_popcountll(root->bitmap[i]);
            item += __builtin_popcountll(bword & ((1ull << bit)-1));
            uint32_t offset = root->slots[item];
            cursor->key[cursor->keylen++] = mykey;
            
            if(DiskNextTrie_GetKind(offset) == CompactTrieKind) {
                CompactMapTrieLevelRef level = (CompactMapTrieLevelRef)DiskNextTrie_GetPtr(trie->mapBase, offset);
                if (level->payload && callback(ctx, cursor->key, level->payload, cursor->prefixlen==cursor->keylen)) return;
                if (cursor->keylen == cursor->prefixlen && exactmatch) return;
                traverseCFBurstTrieCompactMappedLevel(trie, level, cursor, exactmatch, ctx, callback);
            } else if(DiskNextTrie_GetKind(offset) == TrieKind) {
                traverseCFBurstTrieMappedLevel(trie, (MapTrieLevelRef)DiskNextTrie_GetPtr(trie->mapBase, offset), cursor, exactmatch, ctx, callback);
            } else if (DiskNextTrie_GetKind(offset) == ListKind) {
                cursor->next = offset;
                cursor->key[cursor->keylen] = 0;
                findCFBurstTrieMappedPage(trie, cursor, ctx, callback);
            }
        }
    }
}

static void traverseCFBurstTrieWithCursor(CFBurstTrieRef trie, const uint8_t *prefix, uint32_t prefixLen, void **cursor, bool exactmatch, void *ctx, bool (*callback)(void *, const uint8_t *, uint32_t, bool)) {
    if (trie->mapBase) {
        if (trie->cflags & kCFBurstTriePrefixCompression) {
            fprintf(stderr, "Please use CFBurstTrieCursorRef API for file based trie.\n");
            return;
        } else {
            TrieHeader *header = (TrieHeader *)trie->mapBase;
            MapCursor csr;
            csr.next = header->rootOffset;
            csr.prefix = prefix;
            csr.prefixlen = prefixLen;
            csr.key[0] = 0;
            csr.keylen = 0;
            findCFBurstTrieMappedLevel(trie, &csr, exactmatch, ctx, callback);
        }
    } else {    
        TrieCursor csr;
        csr.next = ((uintptr_t)&trie->root)|TrieKind;
        csr.prefix = prefix;
        csr.prefixlen = prefixLen;
        csr.key[0] = 0;
        csr.keylen = 0;
        findCFBurstTrieLevel(trie, &csr, exactmatch, ctx, callback);
    }
}

CF_INLINE uint32_t getPackedPageEntrySize(PageEntryPacked *entry)
{
    return sizeof(PageEntryPacked) + entry->strlen;
}

CF_INLINE uint32_t getPageEntrySize(PageEntry *entry)
{
    return sizeof(PageEntry) + entry->strlen;
}

/*
static void _printPageEntry(PageEntryPacked *entry) {
    printf("entry 0x%p:\n", entry);
    printf("pfxLen = %d, strLen = %d\n", entry->pfxLen, entry->strlen);
    if (entry->strlen > 0) {
        printf("string = ");
        for (int i = 0; i < entry->strlen; ++i)
            printf("%d ", entry->string[i]);
        printf("\n");
    }
    printf("\n");
}
*/
static Boolean advanceCursorOnMappedPageForByte(Page *page, CompactMapCursor *cursor, UInt8 byte) {
    PageEntryPacked *entry;
    Boolean found = FALSE;
    uint32_t minPrefixLength = 0;

    if (cursor->isOnPage) {
        entry = (PageEntryPacked *)&page->data[cursor->entryOffsetInPage];
        //_printPageEntry(entry);
        BOOL shouldContinue = TRUE;

        if (!(cursor->entryOffsetInPage  == 0 && entry->strlen == 0)) {
            if (cursor->entryOffsetInPage == entry->strlen - 1) {
                minPrefixLength = entry->pfxLen + entry->strlen;
                cursor->entryOffsetInPage += getPackedPageEntrySize(entry);
            } else {
                cursor->offsetInEntry++;
                if (entry->string[cursor->offsetInEntry] == byte)
                    found = TRUE;
                else if (entry->string[cursor->offsetInEntry] > byte)
                    shouldContinue = FALSE;
                else {
                    minPrefixLength = entry->pfxLen + cursor->offsetInEntry;
                    cursor->entryOffsetInPage += getPackedPageEntrySize(entry);
                }
            }
        }

        if (found) {
            cursor->isOnPage = TRUE;
            return TRUE;
        } else if (!shouldContinue)
            return FALSE;
    } else {
        cursor->entryOffsetInPage = 0;
    }

    uint32_t pageSize = page->length - sizeof(Page);
    while (cursor->entryOffsetInPage < pageSize) {
        entry = (PageEntryPacked *)&page->data[cursor->entryOffsetInPage];
        //_printPageEntry(entry);
        if (minPrefixLength > entry->pfxLen)
            break;
        else if (minPrefixLength < entry->pfxLen)
            cursor->entryOffsetInPage += getPackedPageEntrySize(entry);
        else {
            if (entry->strlen == 0)
                cursor->entryOffsetInPage += getPackedPageEntrySize(entry);
            else {
                if (entry->string[0] > byte)
                    // Entries are sorted alphabetically
                    break;
                else if (entry->string[0] < byte)
                    cursor->entryOffsetInPage += getPackedPageEntrySize(entry);
                else {
                    cursor->offsetInEntry = 0;
                    found = TRUE;
                    break;
                }
            }
        }
    }

    if (found)
        cursor->isOnPage = TRUE;

    return found;
}

static Boolean advanceCursorMappedPageWithPerfixCompression(Page *page, CompactMapCursor *cursor, const UInt8* bytes, CFIndex length)
{
    if (length == 0) {
        PageEntryPacked *entry = (PageEntryPacked*)&page->data[0];
        if (!cursor->isOnPage) {
            cursor->entryOffsetInPage = 0;
            cursor->offsetInEntry = 0;
            cursor->isOnPage = entry->pfxLen == 0 && entry->strlen == 0;
        }
        getMapCursorPayloadFromPackedPageEntry(entry, cursor, &cursor->payload);
        return TRUE;
    }

    for (CFIndex i = 0; i < length; ++i) {
        if (!advanceCursorOnMappedPageForByte(page, cursor, bytes[i]))
            return FALSE;
    }
    PageEntryPacked *entry = (PageEntryPacked*)&page->data[cursor->entryOffsetInPage];
    getMapCursorPayloadFromPackedPageEntry(entry, cursor, &cursor->payload);
    return TRUE;
}

static Boolean advanceCursorMappedPageSortedByKey(Page *page, CompactMapCursor *cursor, const UInt8* bytes, CFIndex length)
{
    if (length == 0) {
        PageEntry*entry = (PageEntry*)&page->data[0];
        if (!cursor->isOnPage) {
            cursor->entryOffsetInPage = 0;
            cursor->offsetInEntry = 0;
            cursor->isOnPage = entry->strlen == 0;
        }
        getMapCursorPayloadFromPageEntry(entry, cursor, &cursor->payload);
        return TRUE;
    }

    PageEntry *entry;
    uint32_t pageSize = page->length - sizeof(Page);
    const UInt8 * prefix = NULL;
    uint32_t prefixLength = 0;

    if (cursor->isOnPage) {
        entry = (PageEntry*)&page->data[cursor->entryOffsetInPage];
        prefix = entry->string;
        prefixLength = cursor->offsetInEntry + 1;
    }

    while (cursor->entryOffsetInPage < pageSize) {
        PageEntry *entry = (PageEntry*)&page->data[cursor->entryOffsetInPage];
        if (entry->strlen == 0) {
            cursor->entryOffsetInPage += getPageEntrySize(entry);
            continue;
        }

        if (entry->strlen <= prefixLength)
            return FALSE;
        else {
            int cmpResult = 0;
            if (prefixLength > 0)
                cmpResult = __builtin_memcmp(entry->string, prefix, prefixLength);
            if (cmpResult > 0)
                return FALSE;
            else if (cmpResult == 0) {
                if (entry->strlen < prefixLength + length) {
                    int cmpResult2 = __builtin_memcmp(entry->string + prefixLength, bytes, entry->strlen - prefixLength);
                    if (cmpResult2 > 0)
                        return FALSE;
                    else
                        cursor->entryOffsetInPage += getPageEntrySize(entry);
                } else {
                    int cmpResult2 = __builtin_memcmp(entry->string + prefixLength, bytes, length);
                    if (cmpResult2 > 0)
                        return FALSE;
                    else if (cmpResult2 == 0) {
                        cursor->isOnPage = TRUE;
                        cursor->offsetInEntry = prefixLength + length - 1;
                        getMapCursorPayloadFromPageEntry(entry, cursor, &cursor->payload);
                        return TRUE;
                    } else
                        cursor->entryOffsetInPage += getPageEntrySize(entry);
                }
            } else
                cursor->entryOffsetInPage += getPageEntrySize(entry);
        }
    }
    return FALSE;
}

static Boolean advanceCursorMappedPage(CFBurstTrieRef trie, CompactMapCursor *cursor, const UInt8* bytes, CFIndex length)
{
    if (!bytes || length < 0)
        return FALSE;

    Page *page = (Page *)DiskNextTrie_GetPtr(trie->mapBase, cursor->next);
    uint32_t pageSize = page->length - sizeof(Page);
    if (pageSize == 0)
        return FALSE;

    if (trie->cflags & kCFBurstTrieSortByKey)
        return advanceCursorMappedPageSortedByKey(page, cursor, bytes, length);
    else if (trie->cflags & kCFBurstTriePrefixCompression)
        return advanceCursorMappedPageWithPerfixCompression(page, cursor, bytes, length);
    else
        return FALSE;
}

static Boolean advanceCursorCompactMappedLevel(CFBurstTrieRef trie, CompactMapCursor *cursor, const UInt8* bytes, CFIndex length)
{
    if (!bytes || length < 0)
        return FALSE;

    CompactMapTrieLevelRef root = (CompactMapTrieLevelRef)DiskNextTrie_GetPtr(trie->mapBase, cursor->next);
    if (length == 0) {
        cursor->payload = root->payload;
        return TRUE;
    }

    uint8_t slot = bytes[0] / 64;
    uint8_t bit = bytes[0] % 64;
    uint32_t item = 0;
    uint64_t bword = root->bitmap[slot];
    if (bword & (1ull << bit)) {
        // Count all the set bits up to this bit
        for (int i = 0; i < slot; ++i)
            item += __builtin_popcountll(root->bitmap[i]);
        item += __builtin_popcountll(bword & ((1ull << bit)-1));
        cursor->next = root->slots[item];
        return advanceMapCursor(trie, cursor, bytes + 1, length - 1);
    } else {
        return FALSE;
    }
}

static Boolean advanceCursorMappedLevel(CFBurstTrieRef trie, CompactMapCursor *cursor, const UInt8* bytes, CFIndex length)
{
    if (!bytes || length < 0)
        return FALSE;

    MapTrieLevelRef root = (MapTrieLevelRef)DiskNextTrie_GetPtr(trie->mapBase, cursor->next);
    if (length == 0) {
        cursor->payload = root->payload;
        return TRUE;
    }

    cursor->next = root->slots[bytes[0]];
    return advanceMapCursor(trie, cursor, bytes + 1, length - 1);
}

static Boolean advanceMapCursor(CFBurstTrieRef trie, CompactMapCursor *cursor, const UInt8* bytes, CFIndex length)
{
    bool result = FALSE;
    switch (DiskNextTrie_GetKind(cursor->next)) {
        case TrieKind:
            result = advanceCursorMappedLevel(trie, cursor, bytes, length);
            break;
        case CompactTrieKind:
            result = advanceCursorCompactMappedLevel(trie, cursor, bytes, length);
            break;
        case ListKind:
            result = advanceCursorMappedPage(trie, cursor, bytes, length);
            break;
        case Nothing: {
            TrieHeader *header = (TrieHeader*)trie->mapBase;
            if (cursor->next == header->rootOffset)
                result = advanceCursorMappedLevel(trie, cursor, bytes, length);
        }
    }

    return result;
}

static void traverseFromMapCursorMappedLevel(CFBurstTrieRef trie, CompactMapCursor *cursor, UInt8* bytes, uint32_t capacity, uint32_t length, Boolean *stop, void *ctx, CFBurstTrieTraversalCallback callback)
{
    MapTrieLevelRef root = (MapTrieLevelRef)DiskNextTrie_GetPtr(trie->mapBase, cursor->next);
    if (root->payload) {
        callback(ctx, bytes, length, root->payload, stop);
        if (*stop)
            return;
    }

    if (length >= capacity)
        return;

    for (int i = 0; i < CHARACTER_SET_SIZE; ++i) {i =
        bytes[length] = i;
        cursor->next = (uint32_t)root->slots[i];;
        cursor->isOnPage = FALSE;
        cursor->entryOffsetInPage = 0;
        cursor->offsetInEntry = 0;
        cursor->payload = 0;
        traverseFromMapCursor(trie, cursor, bytes, capacity - 1, length + 1, stop, ctx, callback);
        if (*stop)
            return;
    }
}

static void traverseFromMapCursorCompactMappedLevel(CFBurstTrieRef trie, CompactMapCursor *cursor, UInt8* bytes, uint32_t capacity, uint32_t length, Boolean *stop, void *ctx, CFBurstTrieTraversalCallback callback)
{
    CompactMapTrieLevelRef root = (CompactMapTrieLevelRef)DiskNextTrie_GetPtr(trie->mapBase, cursor->next);
    if (root->payload) {
        callback(ctx, bytes, length, root->payload, stop);
        if (*stop)
            return;
    }

    if (length >= capacity)
        return;
    for (int c = 0; c < CHARACTER_SET_SIZE; ++c) {
        bytes[length] = c;
        //This could be optimized to remember what the last slot/item was and not count bits over and over.
        uint8_t slot = c / 64;
        uint8_t bit = c % 64;
        uint32_t item = 0;
        uint64_t bword = root->bitmap[slot];
        if (bword & (1ull << bit)) {
            // Count all the set bits up to this bit
            for (int i = 0; i < slot; ++i)
                item += __builtin_popcountll(root->bitmap[i]);
            item += __builtin_popcountll(bword & ((1ull << bit)-1));
            cursor->next = root->slots[item];
            cursor->isOnPage = FALSE;
            cursor->entryOffsetInPage = 0;
            cursor->offsetInEntry = 0;
            cursor->payload = 0;
            traverseFromMapCursor(trie, cursor, bytes, capacity - 1, length + 1, stop, ctx, callback);
            if (*stop)
                return;
        }
    }
}

static void traverseFromMapCursorMappedPageWithPrefixCompression(Page *page, CompactMapCursor *cursor, UInt8* bytes, uint32_t capacity, uint32_t length, Boolean *stop, void *ctx, CFBurstTrieTraversalCallback callback)
{
    uint32_t pageSize = page->length - sizeof(Page);
    uint32_t offset = cursor->entryOffsetInPage;
    uint32_t minPrefixLength = 0;
    if (cursor->isOnPage) {
        PageEntryPacked *entry = (PageEntryPacked*)&page->data[offset];
        int32_t remainingLength = entry->strlen - cursor->offsetInEntry - 1;
        if (remainingLength >= 0 && remainingLength <= capacity) {
            memcpy(bytes + length, entry->string + cursor->offsetInEntry + 1, remainingLength);
            callback(ctx, bytes, length + remainingLength, entry->payload, stop);
            if (*stop)
                return;
        }
        minPrefixLength = entry->pfxLen + cursor->offsetInEntry;
        offset += getPackedPageEntrySize(entry);
    }
    PageEntryPacked *previousEntry = NULL;
    while (offset < pageSize) {
        PageEntryPacked *entry = (PageEntryPacked*)&page->data[offset];
        if (minPrefixLength > entry->pfxLen)
            break;
        else if (entry->payload && entry->strlen <= capacity) {
            if (previousEntry)
                length -=   previousEntry->strlen + previousEntry->pfxLen - entry->pfxLen;
            memcpy(bytes + length, entry->string, entry->strlen);
            callback(ctx, bytes, length + entry->strlen, entry->payload, stop);
            length += entry->strlen;
            if (*stop)
                return;
        }
        previousEntry = entry;
        offset += getPackedPageEntrySize(entry);
    }
}

static void traverseFromMapCursorMappedPageSortedByKey(Page *page, CompactMapCursor *cursor, UInt8* bytes, uint32_t capacity, uint32_t length, Boolean *stop, void *ctx, CFBurstTrieTraversalCallback callback)
{
    uint32_t pageSize = page->length - sizeof(Page);
    uint32_t offset = cursor->entryOffsetInPage;
    uint32_t prefixLength = 0;
    const UInt8 *prefix = NULL;
    if (cursor->isOnPage) {
        PageEntry *entry = (PageEntry*)&page->data[offset];
        int32_t remainingLength = entry->strlen - cursor->offsetInEntry - 1;
        if (remainingLength >= 0 && remainingLength <= capacity) {
            memcpy(bytes + length, entry->string + cursor->offsetInEntry, remainingLength);
            callback(ctx, bytes, length + remainingLength, entry->payload, stop);
            if (*stop)
                return;
        }
        prefixLength = cursor->offsetInEntry + 1;
        prefix = entry->string;
        offset += getPageEntrySize(entry);
    }

    while (offset < pageSize) {
        PageEntry *entry = (PageEntry*)&page->data[offset];
        if (entry->strlen < prefixLength)
            break;
        else {
            int cmpResult = __builtin_memcmp(entry->string, prefix, prefixLength);
            if (cmpResult > 0)
                break;
            else if (entry->payload && entry->strlen <= capacity) {
                if (entry->strlen > 0)
                    memcpy(bytes + length, entry->string + prefixLength, entry->strlen - prefixLength);
                callback(ctx, bytes, length + entry->strlen - prefixLength, entry->payload, stop);
                if (*stop)
                    return;
            }
            offset += getPageEntrySize(entry);
        }
    }
}

static void traverseFromMapCursorMappedPage(CFBurstTrieRef trie, CompactMapCursor *cursor, UInt8* bytes, uint32_t capacity, uint32_t length, Boolean *stop, void *ctx, CFBurstTrieTraversalCallback callback)
{
    Page *page = (Page *)DiskNextTrie_GetPtr(trie->mapBase, cursor->next);
    if (trie->cflags & kCFBurstTrieSortByKey)
        traverseFromMapCursorMappedPageSortedByKey(page, cursor, bytes, capacity, length, stop, ctx, callback);
    else if (trie->cflags & kCFBurstTriePrefixCompression)
        traverseFromMapCursorMappedPageWithPrefixCompression(page, cursor, bytes, capacity, length, stop, ctx, callback);
}

void traverseFromMapCursor(CFBurstTrieRef trie, CompactMapCursor *cursor, UInt8* bytes, uint32_t capacity, uint32_t length, Boolean *stop, void *ctx, CFBurstTrieTraversalCallback callback)
{
    switch (DiskNextTrie_GetKind(cursor->next)) {
        case TrieKind:
            traverseFromMapCursorMappedLevel(trie, cursor, bytes, capacity, length, stop, ctx, callback);
            break;
        case CompactTrieKind:
            traverseFromMapCursorCompactMappedLevel(trie, cursor, bytes, capacity, length, stop, ctx, callback);
            break;
        case ListKind:
            traverseFromMapCursorMappedPage(trie, cursor, bytes, capacity, length, stop, ctx, callback);
            break;
        case Nothing: {
            TrieHeader *header = (TrieHeader*)trie->mapBase;
            if (cursor->next == header->rootOffset) {
                traverseFromMapCursorMappedLevel(trie, cursor, bytes, capacity, length, stop, ctx, callback);
            }
            break;
        }
    }
}

void copyMapCursor(const CompactMapCursor *source, CompactMapCursor* destination)
{
    destination->next = source->next;
    destination->entryOffsetInPage = source->entryOffsetInPage;
    destination->offsetInEntry = source->offsetInEntry;
    destination->isOnPage = source->isOnPage;
    destination->payload = source->payload;
}

Boolean areMapCursorsEqual(const CompactMapCursor *lhs, const CompactMapCursor *rhs)
{
    return lhs->entryOffsetInPage == rhs->entryOffsetInPage && lhs->isOnPage == rhs->isOnPage && lhs->next == rhs->next && lhs->offsetInEntry == rhs->offsetInEntry;
}

static Boolean getMapCursorPayloadFromPackedPageEntry(PageEntryPacked *entry, const CompactMapCursor *cursor, uint32_t *payload)
{
    if (payload)
        *payload = 0;
    if (cursor->entryOffsetInPage == 0 && cursor->offsetInEntry == 0 && entry->strlen == 0) {
        if (payload)
            *payload = entry->payload;
        return TRUE;
    } else if (cursor->offsetInEntry != entry->strlen - 1)
        return FALSE;
    else {
        if (payload)
            *payload = entry->payload;
        return TRUE;
    }
}

static Boolean getMapCursorPayloadFromPageEntry(PageEntry *entry, const CompactMapCursor *cursor, uint32_t *payload)
{
    if (payload)
        *payload = 0;
    if (cursor->entryOffsetInPage == 0 && cursor->offsetInEntry == 0 && entry->strlen == 0) {
        if (payload)
            *payload = entry->payload;
        return TRUE;
    } else if (cursor->offsetInEntry != entry->strlen - 1)
        return FALSE;
    else {
        if (payload)
            *payload = entry->payload;
        return TRUE;
    }
}

Boolean getMapCursorPayload(CFBurstTrieRef trie, const CompactMapCursor *cursor, uint32_t *payload)
{
    if (!cursor)
        return FALSE;
    if (cursor->payload) {
        if (payload)
            *payload  = cursor->payload;
        return TRUE;
    }
    return FALSE;
}

// Legacy

static Boolean burstTrieMappedFind(DiskTrieLevelRef trie, char *map, const UInt8 *key, uint32_t length, uint32_t *payload, bool prefix) {
    Boolean success = false;
    if (length) {
        uint32_t offset = CFSwapInt32LittleToHost((uint32_t)trie->slots[*key]);
        if(DiskNextTrie_GetKind(offset) == TrieKind) {
            return burstTrieMappedFind((DiskTrieLevelRef)DiskNextTrie_GetPtr(map,offset), map, key+1, length-1, payload, prefix);
        } else if(DiskNextTrie_GetKind(offset) == CompactTrieKind) {
            return burstTrieCompactTrieMappedFind((CompactDiskTrieLevelRef)DiskNextTrie_GetPtr(map, offset), map, key+1, length-1, payload, prefix);
        } else {
            if(DiskNextTrie_GetKind(offset) == ListKind) {
                return burstTrieMappedPageFind((StringPage *)DiskNextTrie_GetPtr(map, offset), key+1, length-1, payload, prefix);
            } else {
                return success;
            }
        }
    } else {
        if (trie->weight) {
            SetPayload(payload, CFSwapInt32LittleToHost(trie->payload));
            success = true;
        }
    }
    return success;
}

static Boolean burstTrieCompactTrieMappedFind(CompactDiskTrieLevelRef trie, char *map, const UInt8 *key, uint32_t length, uint32_t *payload, bool prefix) {
    Boolean success = false;
    if (length) {
        uint32_t mykey = *key;
        uint32_t slot = mykey / 64;
        uint32_t bit = mykey % 64;
        uint32_t item = 0;
        uint64_t bword = CFSwapInt64LittleToHost(trie->bitmap[slot]);
        if (bword & (1ull << bit)) {
            /* Count all the set bits up to this bit */
            for (int i=0; i < slot; i++) {
                item += __builtin_popcountll(trie->bitmap[i]);
            }
            item += __builtin_popcountll(bword & ((1ull << bit)-1));
            uint32_t offset = CFSwapInt32LittleToHost((uint32_t)trie->slots[item]);
            if(DiskNextTrie_GetKind(offset) == TrieKind) {
                return burstTrieMappedFind((DiskTrieLevelRef)DiskNextTrie_GetPtr(map, offset), map, key+1, length-1, payload, prefix);
            } else if(DiskNextTrie_GetKind(offset) == CompactTrieKind) {
                return burstTrieCompactTrieMappedFind((CompactDiskTrieLevelRef)DiskNextTrie_GetPtr(map, offset), map, key+1, length-1, payload, prefix);
            } 
            else {
                if(DiskNextTrie_GetKind(offset) == ListKind) {
                    return burstTrieMappedPageFind((StringPage *)DiskNextTrie_GetPtr(map, offset), key+1, length-1, payload, prefix);
                } else {
                    return success;
                }
            }
        }
    } else {
        if (trie->weight) {
            SetPayload(payload, CFSwapInt32LittleToHost(trie->payload));
            success = true;
        }
    }
    return success;
}

static Boolean burstTrieMappedPageFind(StringPage *page, const UInt8 *key, uint32_t length, uint32_t *payload, bool prefix) {
    Boolean success = false;
    uint32_t end = CFSwapInt32LittleToHost(page->length);
    uint32_t cur = 0;
    if (prefix) {
        uint8_t pfx[256];
        while (cur < end) {
            StringPageEntryPacked *entry = (StringPageEntryPacked *)&page->data[cur];
            uint16_t strlen = entry->pfxLen+CFSwapInt16LittleToHost(entry->strlen);
            if (strlen == length && __builtin_memcmp(pfx, key, entry->pfxLen) == 0 && __builtin_memcmp(entry->string, key+entry->pfxLen, length-entry->pfxLen) == 0) {
                SetPayload(payload, CFSwapInt32LittleToHost(entry->payload));
                success = true;
                return success;
            } else {
                memcpy(pfx+entry->pfxLen, entry->string, MIN(255-entry->pfxLen, length-entry->pfxLen));
                cur += sizeof(*entry) + strlen - entry->pfxLen;
            }
        }        
    } else {
        while (cur < end) {
            StringPageEntry *entry = (StringPageEntry *)&page->data[cur];
            uint16_t strlen = CFSwapInt16LittleToHost(entry->strlen);
            if (strlen == length && __builtin_memcmp(entry->string, key, length) == 0) {
                SetPayload(payload, CFSwapInt32LittleToHost(entry->payload));
                success = true;
                return success;
            } else {
                cur += sizeof(*entry) + strlen;
            }
        }        
        
    }
    return success;
}


#if 0
#pragma mark -
#pragma mark Serialization
#endif

static bool serializeCFBurstTrieLevels(CFBurstTrieRef trie, TrieLevelRef root, uint32_t *offset, off_t start_offset, bool dispose, bool isroot, int fd)
{
    bool dense = true;
    int count = 0;
    
    for (int i=0; i < CHARACTER_SET_SIZE; i++) if (root->slots[i]) count++;
    
    uint32_t this_offset = *offset;
    
    if ((trie->cflags & kCFBurstTrieBitmapCompression) && count < MAX_BITMAP_SIZE && !isroot) {
        size_t size = sizeof(CompactMapTrieLevel) + sizeof(uint32_t) * count;
        int offsetSlot = 0;
        
        CompactMapTrieLevel *maptrie = (CompactMapTrieLevel *)alloca(size);
        bzero(maptrie, size);
        *offset += size;
        
        for (int i=0; i < CHARACTER_SET_SIZE; i++) {
            NextTrie next = root->slots[i];
            if (next) {
                uint32_t slot = i / 64;
                uint32_t bit = i % 64;
                maptrie->bitmap[slot] |= 1ull<<bit;
                if (NextTrie_GetKind(next) == TrieKind) {
                    TrieLevelRef nextLevel = (TrieLevelRef)NextTrie_GetPtr(next);
                    uint32_t childOffset = *offset;
                    if (serializeCFBurstTrieLevels(trie, nextLevel, offset, start_offset, true, false, fd)) {
                        maptrie->slots[offsetSlot] = (TrieKind|childOffset);
                    } else {
                        maptrie->slots[offsetSlot] = (CompactTrieKind|childOffset);
                    }
                } else {
                    maptrie->slots[offsetSlot] = next;
                }
                offsetSlot++;
            }
        }
        maptrie->payload = root->payload;
        
        int bitcount = 0;
        for (int i=0; i < 4; i++) bitcount += __builtin_popcountll(maptrie->bitmap[i]);
        assert(bitcount == count);
        
        pwrite(fd, maptrie, size, this_offset+start_offset);
        dense = false;
    } else {
        MapTrieLevel maptrie;
        *offset += sizeof(maptrie);
        
        for (int i=0; i < CHARACTER_SET_SIZE; i++) {
            NextTrie next = root->slots[i];
            if (NextTrie_GetKind(next) == TrieKind) {
                TrieLevelRef nextLevel = (TrieLevelRef)NextTrie_GetPtr(next);
                uint32_t childOffset = *offset;
                if (serializeCFBurstTrieLevels(trie, nextLevel, offset, start_offset, true, false, fd)) {
                    maptrie.slots[i] = (TrieKind|childOffset);
                } else {
                    maptrie.slots[i] = (CompactTrieKind|childOffset);
                }
            } else {
                maptrie.slots[i] = next;
            }
        }
        maptrie.payload = root->payload;
        pwrite(fd, &maptrie, sizeof(maptrie), this_offset+start_offset);
    }
    
    if (dispose) free(root);
    return dense;
}

static void serializeCFBurstTrieList(CFBurstTrieRef trie, ListNodeRef listNode, int fd)
{
    uint32_t listCount;
    size_t size = trie->containerSize;
    
    // ** Temp list of nodes to sort
    ListNodeRef *nodes = (ListNodeRef *)malloc(sizeof(ListNodeRef) * size);
    for (listCount = 0; listNode; listCount++) {
        if (listCount >= size) {
            size *= 2;
            nodes = (ListNodeRef *)realloc(nodes, sizeof(ListNodeRef) * size);
        }
        nodes[listCount] = listNode;
        listNode = listNode->next;
    }
    
    char _buffer[MAX_BUFFER_SIZE];
    size_t bufferSize = (sizeof(Page) + size * (sizeof(PageEntryPacked) + MAX_STRING_SIZE));
    char *buffer = bufferSize < MAX_BUFFER_SIZE ? _buffer : (char *) malloc(bufferSize);
    
    Page *page = (Page *)buffer;
    uint32_t current = 0;
    
    if (trie->cflags & kCFBurstTriePrefixCompression) {
        qsort(nodes, listCount, sizeof(ListNodeRef), nodeStringCompare);
        
        ListNodeRef last = 0;
        for (int i=0; i < listCount; i++) {
            listNode = nodes[i];
            uint8_t pfxLen = 0;
            if (last) {
                for ( ; 
                     pfxLen < CHARACTER_SET_SIZE-1 && 
                     pfxLen < listNode->length && 
                     pfxLen < last->length && 
                     listNode->string[pfxLen] == last->string[pfxLen]; 
                     pfxLen++);
            }
            
            PageEntryPacked *entry = (PageEntryPacked *)(&page->data[current]);
            entry->strlen = listNode->length - pfxLen;
            entry->payload = listNode->payload;
            entry->pfxLen = pfxLen;
            memcpy(entry->string, listNode->string+pfxLen, listNode->length-pfxLen);
            current += listNode->length - pfxLen + sizeof(PageEntryPacked);
            last = listNode;
        }
    } else {
        if (trie->cflags & kCFBurstTrieSortByKey)
            qsort(nodes, listCount, sizeof(ListNodeRef), nodeStringCompare);
        else
            qsort(nodes, listCount, sizeof(ListNodeRef), nodeWeightCompare);
        
        for (int i=0; i < listCount; i++) {
            listNode = nodes[i];
            PageEntry *entry = (PageEntry *)(&page->data[current]);
            entry->strlen = listNode->length;
            entry->payload = listNode->payload;
            memcpy(entry->string, listNode->string, listNode->length);
            current += listNode->length + sizeof(PageEntry);
        }
    }
    
    size_t len = (sizeof(Page) + current + 3) & ~3;
    page->length = current;
    write(fd, page, len);
    
    free(nodes);
    if (buffer != _buffer) free(buffer);
}

static void serializeCFBurstTrieLists(CFBurstTrieRef trie, TrieLevelRef root, off_t start_offset, int fd)
{
    for (int i=0; i < CHARACTER_SET_SIZE; i++) {
        NextTrie next = root->slots[i];
        uint32_t offset;
        if (NextTrie_GetKind(next) == TrieKind) {
            TrieLevelRef nextLevel = (TrieLevelRef)NextTrie_GetPtr(next);
            serializeCFBurstTrieLists(trie, nextLevel, start_offset, fd);
        } else {
            if (NextTrie_GetKind(next) == ListKind) {
                ListNodeRef listNode = (ListNodeRef)NextTrie_GetPtr(next);
                offset = lseek(fd, 0, SEEK_CUR) - start_offset;
                serializeCFBurstTrieList(trie, listNode, fd);
                finalizeCFBurstTrieList(listNode);
                //assert((offset & 3)==0);
                root->slots[i] = (offset|ListKind);
            }
        }
    }
}

static size_t serializeCFBurstTrie(CFBurstTrieRef trie, size_t start_offset, int fd)
{
    TrieHeader header;
    header.signature = 0x0ddba11;
    header.rootOffset = 0;
    header.count = trie->count;
    header.size = 0;
    header.flags = trie->cflags;
    header.reserved[0] = 0;
    
    uint32_t offset;
    lseek(fd, start_offset, SEEK_SET);
    
    size_t header_size = sizeof(header);
    write(fd, &header, header_size);
    
    serializeCFBurstTrieLists(trie, &trie->root, start_offset, fd);
    
    offset = lseek(fd, 0, SEEK_CUR) - start_offset;
    size_t off = offsetof(TrieHeader, rootOffset);
    size_t offsize = sizeof(offset);
    pwrite(fd, &offset, offsize, off+start_offset);
    
    serializeCFBurstTrieLevels(trie, &trie->root, &offset, start_offset, false, true, fd);
    
    size_t off2 = offsetof(TrieHeader, size);
    offsize = sizeof(offset);
    pwrite(fd, &offset, offsize, off2+start_offset);
    
    offset = lseek(fd, 0, SEEK_END);
    return (size_t)(offset-start_offset);
}

#if 0
#pragma mark -
#pragma mark Release
#endif

static void destroyCFBurstTrie(CFBurstTrieRef trie) {
    if (trie->mapBase) {
#if DEPLOYMENT_TARGET_WINDOWS
        UnmapViewOfFile(trie->mapBase);
        CloseHandle(trie->mapHandle);
        CloseHandle(trie->mappedFileHandle);
#else
        munmap(trie->mapBase, trie->mapSize);
#endif
    } else {
        finalizeCFBurstTrie(&trie->root);
    }
    free(trie);
    return;
}

static void finalizeCFBurstTrie(TrieLevelRef trie) {
    for (int i=0; i < CHARACTER_SET_SIZE; i++) {
        if (NextTrie_GetKind(trie->slots[i]) == TrieKind) {
            finalizeCFBurstTrie((TrieLevelRef)NextTrie_GetPtr(trie->slots[i]));
            free((void *)NextTrie_GetPtr(trie->slots[i]));
        } else if (NextTrie_GetKind(trie->slots[i]) == ListKind) {
            finalizeCFBurstTrieList((ListNodeRef)NextTrie_GetPtr(trie->slots[i]));
        }
    }
}

static void finalizeCFBurstTrieList(ListNodeRef node) {
    do {
        ListNodeRef next = node->next;
        free(node);
        node = next;
    } while(node);
}

#if 0
#pragma mark -
#pragma mark Helpers
#endif

static int nodeWeightCompare(const void *a, const void *b) {
    ListNodeRef nodeA = *(ListNodeRef *)a;
    ListNodeRef nodeB = *(ListNodeRef *)b;
    return (nodeB->weight - nodeA->weight);
}

static int nodeStringCompare(const void *a, const void *b) {
    ListNodeRef nodeA = *(ListNodeRef *)a;
    ListNodeRef nodeB = *(ListNodeRef *)b;
    int result = memcmp((char *)nodeA->string, (char *)nodeB->string, MIN(nodeA->length, nodeB->length));
    if (result == 0) result = nodeA->length-nodeB->length;
    return result;
}

static bool containsKey(void *context, const uint8_t *key, uint32_t payload, bool exact)
{
    uint32_t *ctx = (uint32_t *)context;
    if (exact) *ctx = payload;
    return exact;
}

static CFIndex burstTrieConvertCharactersToUTF8(UniChar *chars, CFIndex numChars, UInt8 *buffer) {
    uint32_t i, j;
    for (i = j = 0; i < numChars; i++) {
        UniChar c = chars[i];
        if (CFStringIsSurrogateHighCharacter(c) && i + 1 < numChars && CFStringIsSurrogateLowCharacter(chars[i + 1])) {
            UTF32Char lc = CFStringGetLongCharacterForSurrogatePair(c, chars[i + 1]);
            buffer[j++] = 0xf0 + (lc >> 18);
            buffer[j++] = 0x80 + ((lc & 0x3ffff) >> 12);
            buffer[j++] = 0x80 + ((lc & 0xfff) >> 6);
            buffer[j++] = 0x80 + (lc & 0x3f);
            i++;
        } else {
            if (c < 0x80) {
                buffer[j++] = c;
            } else if (c < 0x800) {
                buffer[j++] = 0xc0 + (c >> 6);
                buffer[j++] = 0x80 + (c & 0x3f);
            } else {
                buffer[j++] = 0xe0 + (c >> 12);
                buffer[j++] = 0x80 + ((c & 0xfff) >> 6);
                buffer[j++] = 0x80 + (c & 0x3f);
            }
        }
    }
    buffer[j] = 0;
    return j;
}

