/*
Original Author: Michael Ash on 11/9/08.
Copyright (c) 2008 Rogue Amoeba Software LLC

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

*/

#import "NSAtomicList.h"

#import <Foundation/NSAtomicCompareAndSwap.h>
#include <stdlib.h>

static int OSAtomicCompareAndSwapPtrBarrier(void* oldValue, void* newValue, void* volatile *theValue) {
   return __sync_bool_compare_and_swap(theValue,oldValue,newValue);
}

struct NSAtomicListNode
{
	struct NSAtomicListNode *next;
	void *elt;
};

void NSAtomicListInsert( NSAtomicListRef *listPtr, void *elt )
{
	struct NSAtomicListNode *node = malloc( sizeof( *node ) );
	node->elt = elt;

	do {
		node->next = *listPtr;
	} while( !OSAtomicCompareAndSwapPtrBarrier( node->next, node, (void **)listPtr ) );
}

NSAtomicListRef NSAtomicListSteal( NSAtomicListRef *listPtr )
{
	NSAtomicListRef ret;
	do {
		ret = *listPtr;
	} while( !OSAtomicCompareAndSwapPtrBarrier( ret, NULL, (void **)listPtr ) );
	return ret;
}

void NSAtomicListReverse( NSAtomicListRef *listPtr )
{
	struct NSAtomicListNode *cur = *listPtr;
	struct NSAtomicListNode *prev = NULL;
	struct NSAtomicListNode *next = NULL;

	if( !cur )
		return;

	do {
		next = cur->next;
		cur->next = prev;

		if( next )
		{
			prev = cur;
			cur = next;
		}
	} while( next );

	*listPtr = cur;
}

void NSAtomicListAddToArray(NSAtomicListRef *listPtr, NSMutableArray *array)
{
	struct NSAtomicListNode *node = NSAtomicListSteal(listPtr);
	while (node) {
		[array addObject:(id)node->elt];
		 node = node->next;
	};
}

void *NSAtomicListPop( NSAtomicListRef *listPtr)
{
	struct NSAtomicListNode *node = *listPtr;
	if( !node )
		return NULL;

	*listPtr = node->next;

	void *elt = node->elt;
	free( node );
	return elt;
}

