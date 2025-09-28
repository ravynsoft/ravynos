/* Copyright (c) 2006-2007 Christopher J. W. Lloyd

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */

#import <Foundation/NSObject.h>
#import <Foundation/NSRange.h>

// an ordered, nonoverlapping set of NSRanges and related value
typedef struct NSRangeEntries NSRangeEntries;

typedef struct {
    NSRangeEntries *self;
    NSUInteger index;
} NSRangeEnumerator;

FOUNDATION_EXPORT NSRangeEntries *NSCreateRangeToOwnedPointerEntries(NSUInteger capacity);
FOUNDATION_EXPORT NSRangeEntries *NSCreateRangeToCopiedObjectEntries(NSUInteger capacity);

FOUNDATION_EXPORT void NSFreeRangeEntries(NSRangeEntries *self);
FOUNDATION_EXPORT void NSResetRangeEntries(NSRangeEntries *self);
FOUNDATION_EXPORT NSUInteger NSCountRangeEntries(NSRangeEntries *self);
FOUNDATION_EXPORT void NSRangeEntriesRemoveEntryAtIndex(NSRangeEntries *self, NSUInteger index);

FOUNDATION_EXPORT void NSRangeEntryInsert(NSRangeEntries *self, NSRange range, void *value);
FOUNDATION_EXPORT void *NSRangeEntryAtIndex(NSRangeEntries *self, NSUInteger index, NSRange *effectiveRange);
FOUNDATION_EXPORT void *NSRangeEntryAtRange(NSRangeEntries *self, NSRange range);

FOUNDATION_EXPORT NSRangeEnumerator NSRangeEntryEnumerator(NSRangeEntries *self);
FOUNDATION_EXPORT BOOL NSNextRangeEnumeratorEntry(NSRangeEnumerator *state, NSRange *rangep, void **value);

FOUNDATION_EXPORT void NSRangeEntriesExpandAndWipe(NSRangeEntries *self, NSRange range, NSInteger delta);
FOUNDATION_EXPORT void NSRangeEntriesDivideAndConquer(NSRangeEntries *self, NSRange range);
FOUNDATION_EXPORT void NSRangeEntriesDump(NSRangeEntries *self);
FOUNDATION_EXPORT void NSRangeEntriesVerify(NSRangeEntries *self, NSUInteger length);
