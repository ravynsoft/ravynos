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

/*	CFBurstTrie.h
        Copyright (c) 2008-2014, Apple Inc. All rights reserved.
*/

#if !defined(__COREFOUNDATION_CFBURSTTRIE__)
#define __COREFOUNDATION_CFBURSTTRIE__ 1

#include <CoreFoundation/CFString.h>
#include <CoreFoundation/CFDictionary.h>

CF_EXTERN_C_BEGIN

typedef struct _CFBurstTrie *CFBurstTrieRef;
typedef struct _CFBurstTrieCursor *CFBurstTrieCursorRef;

typedef CF_OPTIONS(CFOptionFlags, CFBurstTrieOpts) {
        /*!
         BurstTrie Options
         Use one or more of these options with CFBurstTrieCreate to tailor optimizations to the data
         structure for a specific kind of application. Default is no read-write, no compression.
         */
    
    /*  kCFBurstTrieReadOnly
        When specified, the dictionary file will be serialized in an optimized format so as to be
        memory-mapped on the next read. Once a trie is serialized as read-only, insertions can no
        longer occur.
    */
    kCFBurstTrieReadOnly            = 1<<1,
    
    /*  kCFBurstTrieBitmapCompression
        This option can only be used with a read-only trie, and can be used to reduce on disk file size.
    */
    kCFBurstTrieBitmapCompression   = 1<<2,
    
    /*
        kCFBurstTriePrefixCompression
        This option can only be used with a read-only trie, and can be used to reduce on-disk file size.
        It is important to note that any optimizations based on word frequency will be lost; recommended
        for applications that often search for infrequent or uncommon words. This also allow you to use
        cursor interface.
    */
    kCFBurstTriePrefixCompression   = 1<<3,

    /*
        kCFBurstTriePrefixCompression
        By default, keys at list level are sorted by weight. Use this option to sort them by key value.
        This allow you to use cursor interface.
     */
    kCFBurstTrieSortByKey = 1 << 4
};

// Value for this option should be a CFNumber which contains an int.
#define kCFBurstTrieCreationOptionNameContainerSize CFSTR("ContainerSize")

typedef void (*CFBurstTrieTraversalCallback)(void* context, const UInt8* key, uint32_t keyLength, uint32_t payload, Boolean *stop);

CF_EXPORT 
CFBurstTrieRef CFBurstTrieCreate() CF_AVAILABLE(10_7, 4_2);

CF_EXPORT
CFBurstTrieRef CFBurstTrieCreateWithOptions(CFDictionaryRef options) CF_AVAILABLE(10_8, 6_0);

CF_EXPORT 
CFBurstTrieRef CFBurstTrieCreateFromFile(CFStringRef path) CF_AVAILABLE(10_7, 4_2);

CF_EXPORT
CFBurstTrieRef CFBurstTrieCreateFromMapBytes(char *mapBase) CF_AVAILABLE(10_7, 4_2);

CF_EXPORT 
Boolean CFBurstTrieInsert(CFBurstTrieRef trie, CFStringRef term, CFRange termRange, CFIndex payload) CF_AVAILABLE(10_7, 4_2);

CF_EXPORT 
Boolean CFBurstTrieAdd(CFBurstTrieRef trie, CFStringRef term, CFRange termRange, uint32_t payload) CF_AVAILABLE(10_7, 5_0);


CF_EXPORT 
Boolean CFBurstTrieInsertCharacters(CFBurstTrieRef trie, UniChar *chars, CFIndex numChars, CFIndex payload) CF_AVAILABLE(10_7, 4_2);

CF_EXPORT 
Boolean CFBurstTrieAddCharacters(CFBurstTrieRef trie, UniChar *chars, CFIndex numChars, uint32_t payload) CF_AVAILABLE(10_7, 5_0);


CF_EXPORT 
Boolean CFBurstTrieInsertUTF8String(CFBurstTrieRef trie, UInt8 *chars, CFIndex numChars, CFIndex payload) CF_AVAILABLE(10_7, 4_2);

CF_EXPORT 
Boolean CFBurstTrieAddUTF8String(CFBurstTrieRef trie, UInt8 *chars, CFIndex numChars, uint32_t payload) CF_AVAILABLE(10_7, 5_0);


CF_EXPORT
Boolean CFBurstTrieInsertWithWeight(CFBurstTrieRef trie, CFStringRef term, CFRange termRange, CFIndex weight, CFIndex payload) CF_AVAILABLE(10_7, 4_2);

CF_EXPORT
Boolean CFBurstTrieAddWithWeight(CFBurstTrieRef trie, CFStringRef term, CFRange termRange, uint32_t weight, uint32_t payload) CF_AVAILABLE(10_7, 5_0);


CF_EXPORT
Boolean CFBurstTrieInsertCharactersWithWeight(CFBurstTrieRef trie, UniChar *chars, CFIndex numChars, CFIndex weight, CFIndex payload) CF_AVAILABLE(10_7, 4_2);

CF_EXPORT
Boolean CFBurstTrieAddCharactersWithWeight(CFBurstTrieRef trie, UniChar *chars, CFIndex numChars, uint32_t weight, uint32_t payload) CF_AVAILABLE(10_7, 5_0);


CF_EXPORT
Boolean CFBurstTrieInsertUTF8StringWithWeight(CFBurstTrieRef trie, UInt8 *chars, CFIndex numChars, CFIndex weight, CFIndex payload) CF_AVAILABLE(10_7, 4_2);

CF_EXPORT
Boolean CFBurstTrieAddUTF8StringWithWeight(CFBurstTrieRef trie, UInt8 *chars, CFIndex numChars, uint32_t weight, uint32_t payload) CF_AVAILABLE(10_7, 5_0);


CF_EXPORT 
Boolean CFBurstTrieFind(CFBurstTrieRef trie, CFStringRef term, CFRange termRange, CFIndex *payload) CF_AVAILABLE(10_7, 4_2);

CF_EXPORT 
Boolean CFBurstTrieContains(CFBurstTrieRef trie, CFStringRef term, CFRange termRange, uint32_t *payload) CF_AVAILABLE(10_7, 5_0);


CF_EXPORT 
Boolean CFBurstTrieFindCharacters(CFBurstTrieRef trie, UniChar *chars, CFIndex numChars, CFIndex *payload) CF_AVAILABLE(10_7, 4_2);

CF_EXPORT 
Boolean CFBurstTrieContainsCharacters(CFBurstTrieRef trie, UniChar *chars, CFIndex numChars, uint32_t *payload) CF_AVAILABLE(10_7, 5_0);


CF_EXPORT 
Boolean CFBurstTrieFindUTF8String(CFBurstTrieRef trie, UInt8 *key, CFIndex length, CFIndex *payload) CF_AVAILABLE(10_7, 4_2);

CF_EXPORT 
Boolean CFBurstTrieContainsUTF8String(CFBurstTrieRef trie, UInt8 *key, CFIndex length, uint32_t *payload) CF_AVAILABLE(10_7, 5_0);


CF_EXPORT 
Boolean CFBurstTrieSerialize(CFBurstTrieRef trie, CFStringRef path, CFBurstTrieOpts opts) CF_AVAILABLE(10_7, 4_2);

CF_EXPORT
Boolean CFBurstTrieSerializeWithFileDescriptor(CFBurstTrieRef trie, int fd, CFBurstTrieOpts opts) CF_AVAILABLE(10_7, 4_2);

CF_EXPORT 
void CFBurstTrieTraverse(CFBurstTrieRef trie, void *ctx, void (*callback)(void*, const UInt8*, uint32_t, uint32_t)) CF_AVAILABLE(10_7, 4_2);

CF_EXPORT 
CFIndex CFBurstTrieGetCount(CFBurstTrieRef trie) CF_AVAILABLE(10_7, 4_2);

CF_EXPORT 
CFBurstTrieRef CFBurstTrieRetain(CFBurstTrieRef trie) CF_AVAILABLE(10_7, 4_2);

CF_EXPORT 
void CFBurstTrieRelease(CFBurstTrieRef trie) CF_AVAILABLE(10_7, 4_2);

CF_EXPORT
CFBurstTrieCursorRef CFBurstTrieCreateCursorForBytes(CFBurstTrieRef trie, const UInt8* bytes, CFIndex length) CF_AVAILABLE(10_8, 6_0);

CF_EXPORT
CFBurstTrieCursorRef CFBurstTrieCursorCreateByCopy(CFBurstTrieCursorRef cursor) CF_AVAILABLE(10_8, 6_0);

CF_EXPORT
Boolean CFBurstTrieSetCursorForBytes(CFBurstTrieRef trie, CFBurstTrieCursorRef cursor, const UInt8* bytes, CFIndex length) CF_AVAILABLE(10_8, 6_0);

CF_EXPORT
Boolean CFBurstTrieCursorIsEqual(CFBurstTrieCursorRef lhs, CFBurstTrieCursorRef rhs) CF_AVAILABLE(10_8, 6_0);

CF_EXPORT
Boolean CFBurstTrieCursorAdvanceForBytes(CFBurstTrieCursorRef cursor, const UInt8* bytes, CFIndex length) CF_AVAILABLE(10_8, 6_0);

CF_EXPORT
Boolean CFBurstTrieCursorGetPayload(CFBurstTrieCursorRef cursor, uint32_t *payload) CF_AVAILABLE(10_8, 6_0);

CF_EXPORT
void CFBurstTrieTraverseFromCursor(CFBurstTrieCursorRef cursor, void *ctx, CFBurstTrieTraversalCallback callback) CF_AVAILABLE(10_8, 6_0);

CF_EXPORT
void CFBurstTrieCursorRelease(CFBurstTrieCursorRef cursor) CF_AVAILABLE(10_8, 6_0);

CF_EXTERN_C_END

#endif /* __COREFOUNDATION_CFBURSTTRIE__ */
