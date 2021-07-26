/* Copyright (c) 2006-2007 Christopher J. W. Lloyd

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */

#import <Foundation/NSObjCRuntime.h>
#import <Foundation/NSZone.h>

@class NSArray;

typedef struct NSHashTable NSHashTable;

typedef struct {
    NSHashTable *table;
    NSUInteger i;
    struct NSHashBucket *j;
} NSHashEnumerator;

typedef struct {
    NSUInteger (*hash)(NSHashTable *table, const void *);
    BOOL (*isEqual)(NSHashTable *table, const void *, const void *);
    void (*retain)(NSHashTable *table, const void *);
    void (*release)(NSHashTable *table, void *);
    NSString *(*describe)(NSHashTable *table, const void *);
} NSHashTableCallBacks;

FOUNDATION_EXPORT const NSHashTableCallBacks NSIntHashCallBacks;
FOUNDATION_EXPORT const NSHashTableCallBacks NSNonOwnedPointerHashCallBacks;
FOUNDATION_EXPORT const NSHashTableCallBacks NSNonRetainedObjectHashCallBacks;
FOUNDATION_EXPORT const NSHashTableCallBacks NSObjectHashCallBacks;
FOUNDATION_EXPORT const NSHashTableCallBacks NSOwnedObjectIdentityHashCallBacks;
FOUNDATION_EXPORT const NSHashTableCallBacks NSOwnedPointerHashCallBacks;
FOUNDATION_EXPORT const NSHashTableCallBacks NSPointerToStructHashCallBacks;

FOUNDATION_EXPORT NSHashTable *NSCreateHashTable(NSHashTableCallBacks callBacks,
    NSUInteger capacity);
FOUNDATION_EXPORT NSHashTable *NSCreateHashTableWithZone(NSHashTableCallBacks callBacks,
    NSUInteger capacity, NSZone *zone);
FOUNDATION_EXPORT NSHashTable *NSCopyHashTableWithZone(NSHashTable *table, NSZone *zone);
FOUNDATION_EXPORT void NSFreeHashTable(NSHashTable *table);
FOUNDATION_EXPORT void NSResetHashTable(NSHashTable *table);
FOUNDATION_EXPORT BOOL NSCompareHashTables(NSHashTable *table1, NSHashTable *table2);
FOUNDATION_EXPORT NSUInteger NSCountHashTable(NSHashTable *table);
FOUNDATION_EXPORT void *NSHashGet(NSHashTable *table, const void *pointer);
FOUNDATION_EXPORT NSArray *NSAllHashTableObjects(NSHashTable *table);
FOUNDATION_EXPORT NSHashEnumerator NSEnumerateHashTable(NSHashTable *table);
FOUNDATION_EXPORT void *NSNextHashEnumeratorItem(NSHashEnumerator *enumerator);
FOUNDATION_EXPORT void NSHashInsert(NSHashTable *table, const void *pointer);
FOUNDATION_EXPORT void NSHashInsertKnownAbsent(NSHashTable *table, const void *pointer);
FOUNDATION_EXPORT void *NSHashInsertIfAbsent(NSHashTable *table, const void *pointer);
FOUNDATION_EXPORT void NSHashRemove(NSHashTable *table, const void *pointer);
FOUNDATION_EXPORT NSString *NSStringFromHashTable(NSHashTable *table);
