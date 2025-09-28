/*
 * Copyright (c) 2009 Remy Demarest
 * Portions Copyright (c) 2009 David Chisnall
 *
 *  Permission is hereby granted, free of charge, to any person
 *  obtaining a copy of this software and associated documentation
 *  files (the "Software"), to deal in the Software without
 *  restriction, including without limitation the rights to use,
 *  copy, modify, merge, publish, distribute, sublicense, and/or sell
 *  copies of the Software, and to permit persons to whom the
 *  Software is furnished to do so, subject to the following
 *  conditions:
 *
 *  The above copyright notice and this permission notice shall be
 *  included in all copies or substantial portions of the Software.
 *
 *  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 *  EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 *  OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 *  NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 *  HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 *  WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 *  FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 *  OTHER DEALINGS IN THE SOFTWARE.
 */
#import "objc/blocks_runtime.h"
#import "objc/runtime.h"
#import "objc/objc-arc.h"
#include "blocks_runtime.h"
#include "gc_ops.h"
#include "visibility.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <assert.h>


static void *_HeapBlockByRef = (void*)1;


OBJC_PUBLIC bool _Block_has_signature(void *b)
{
	const struct Block_layout *block = (struct Block_layout*)b;
	return ((NULL != block) && (block->flags & BLOCK_HAS_SIGNATURE));
}
/**
 * Returns the Objective-C type encoding for the block.
 */
OBJC_PUBLIC const char * _Block_signature(void *b)
{
	const struct Block_layout *block = (struct Block_layout*)b;
	if ((NULL == block) || !(block->flags & BLOCK_HAS_SIGNATURE))
	{
		return NULL;
	}
	if (!(block->flags & BLOCK_HAS_COPY_DISPOSE))
	{
		return ((struct Block_descriptor_basic*)block->descriptor)->encoding;
	}
	return block->descriptor->encoding;
}
OBJC_PUBLIC const char *block_getType_np(const void *b)
{
	return _Block_signature((void*)b);
}

static int increment24(int *ref)
{
	int old = *ref;
	int val = old & BLOCK_REFCOUNT_MASK;
	if (val == BLOCK_REFCOUNT_MASK)
	{
		return val;
	}
	assert(val < BLOCK_REFCOUNT_MASK);
	if (!__sync_bool_compare_and_swap(ref, old, old+1))
	{
		return increment24(ref);
	}
	return val + 1;
}

static int decrement24(int *ref)
{
	int old = *ref;
	int val = old & BLOCK_REFCOUNT_MASK;
	if (val == BLOCK_REFCOUNT_MASK)
	{
		return val;
	}
	assert(val > 0);
	if (!__sync_bool_compare_and_swap(ref, old, old-1))
	{
		return decrement24(ref);
	}
	return val - 1;
}

// This is a really ugly hack that works around a buggy register allocator in
// GCC.  Compiling nontrivial code using __sync_bool_compare_and_swap() with
// GCC (4.2.1, at least), causes the register allocator to run out of registers
// and fall over and die.  We work around this by wrapping this CAS in a
// function, which means the register allocator can trivially handle it.  Do
// not remove the noinline attribute - without it, gcc will inline it early on
// and then crash later.
#ifndef __clang__
__attribute__((noinline))
static int cas(void *ptr, void *old, void *new)
{
	return __sync_bool_compare_and_swap((void**)ptr, old, new);
}
#define __sync_bool_compare_and_swap cas
#endif

/* Certain field types require runtime assistance when being copied to the
 * heap.  The following function is used to copy fields of types: blocks,
 * pointers to byref structures, and objects (including
 * __attribute__((NSObject)) pointers.  BLOCK_FIELD_IS_WEAK is orthogonal to
 * the other choices which are mutually exclusive.  Only in a Block copy helper
 * will one see BLOCK_FIELD_IS_BYREF.
 */
OBJC_PUBLIC void _Block_object_assign(void *destAddr, const void *object, const int flags)
{
	//printf("Copying %x to %x with flags %x\n", object, destAddr, flags);
	// FIXME: Needs to be implemented
	//if(flags & BLOCK_FIELD_IS_WEAK)
	{
	}
	//else
	{
		if (IS_SET(flags, BLOCK_FIELD_IS_BYREF))
		{
			struct block_byref_obj *src = (struct block_byref_obj *)object;
			struct block_byref_obj **dst = destAddr;
			src = src->forwarding;

			if ((src->flags & BLOCK_REFCOUNT_MASK) == 0)
			{
				*dst = gc->malloc(src->size);
				memcpy(*dst, src, src->size);
				(*dst)->isa = _HeapBlockByRef;
				// Refcount must be two; one for the copy and one for the
				// on-stack version that will point to it.
				(*dst)->flags += 2;
				if (IS_SET(src->flags, BLOCK_HAS_COPY_DISPOSE))
				{
					src->byref_keep(*dst, src);
				}
				(*dst)->forwarding = *dst;
				// Concurrency.  If we try copying the same byref structure
				// from two threads simultaneously, we could end up with two
				// versions on the heap that are unaware of each other.  That
				// would be bad.  So we first set up the copy, then try to do
				// an atomic compare-and-exchange to point the old version at
				// it.  If the forwarding pointer in src has changed, then we
				// recover - clean up and then return the structure that the
				// other thread created.
				if (!__sync_bool_compare_and_swap(&src->forwarding, src, *dst))
				{
					if((size_t)src->size >= sizeof(struct block_byref_obj))
					{
						src->byref_dispose(*dst);
					}
					gc->free(*dst);
					*dst = src->forwarding;
				}
			}
			else
			{
				*dst = (struct block_byref_obj*)src;
				increment24(&(*dst)->flags);
			}
		}
		else if (IS_SET(flags, BLOCK_FIELD_IS_BLOCK))
		{
			struct Block_layout *src = (struct Block_layout*)object;
			struct Block_layout **dst = destAddr;

			*dst = Block_copy(src);
		}
		else if (IS_SET(flags, BLOCK_FIELD_IS_OBJECT) &&
		         !IS_SET(flags, BLOCK_BYREF_CALLER))
		{
			id src = (id)object;
			void **dst = destAddr;
			*dst = src;
			if (!isGCEnabled)
			{
				*dst = objc_retain(src);
			}
		}
	}
}

/* Similarly a compiler generated dispose helper needs to call back for each
 * field of the byref data structure.  (Currently the implementation only packs
 * one field into the byref structure but in principle there could be more).
 * The same flags used in the copy helper should be used for each call
 * generated to this function:
 */
OBJC_PUBLIC void _Block_object_dispose(const void *object, const int flags)
{
	// FIXME: Needs to be implemented
	//if(flags & BLOCK_FIELD_IS_WEAK)
	{
	}
	//else
	{
		if (IS_SET(flags, BLOCK_FIELD_IS_BYREF))
		{
			struct block_byref_obj *src =
				(struct block_byref_obj*)object;
			src = src->forwarding;
			if (src->isa == _HeapBlockByRef)
			{
				int refcount = (src->flags & BLOCK_REFCOUNT_MASK) == 0 ? 0 : decrement24(&src->flags);
				if (refcount == 0)
				{
					if(IS_SET(src->flags, BLOCK_HAS_COPY_DISPOSE) && (0 != src->byref_dispose))
					{
						src->byref_dispose(src);
					}
					gc->free(src);
				}
			}
		}
		else if (IS_SET(flags, BLOCK_FIELD_IS_BLOCK))
		{
			struct Block_layout *src = (struct Block_layout*)object;
			Block_release(src);
		}
		else if (IS_SET(flags, BLOCK_FIELD_IS_OBJECT) &&
		         !IS_SET(flags, BLOCK_BYREF_CALLER))
		{
			id src = (id)object;
			if (!isGCEnabled)
			{
				objc_release(src);
			}
		}
	}
}


// Copy a block to the heap if it's still on the stack or increments its retain count.
OBJC_PUBLIC void *_Block_copy(const void *src)
{
	if (NULL == src) { return NULL; }
	struct Block_layout *self = (struct Block_layout*)src;
	struct Block_layout *ret = self;

	extern void _NSConcreteStackBlock;
	extern void _NSConcreteMallocBlock;

	// If the block is Global, there's no need to copy it on the heap.
	if(self->isa == &_NSConcreteStackBlock)
	{
		ret = gc->malloc(self->descriptor->size);
		memcpy(ret, self, self->descriptor->size);
		ret->isa = &_NSConcreteMallocBlock;
		if(self->flags & BLOCK_HAS_COPY_DISPOSE)
		{
			self->descriptor->copy_helper(ret, self);
		}
		// We don't need any atomic operations here, because on-stack blocks
		// can not be aliased across threads (unless you've done something
		// badly wrong).
		ret->reserved = 1;
	}
	else if (self->isa == &_NSConcreteMallocBlock)
	{
		// We need an atomic increment for malloc'd blocks, because they may be
		// shared.
		__sync_fetch_and_add(&ret->reserved, 1);
	}
	return ret;
}

// Release a block and frees the memory when the retain count hits zero.
OBJC_PUBLIC void _Block_release(const void *src)
{
	if (NULL == src) { return; }
	struct Block_layout *self = (struct Block_layout*)src;

	extern void _NSConcreteStackBlock;
	extern void _NSConcreteMallocBlock;

	if (&_NSConcreteStackBlock == self->isa)
	{
		fprintf(stderr, "Block_release called upon a stack Block: %p, ignored\n", self);
	}
	else if (&_NSConcreteMallocBlock == self->isa)
	{
		if (__sync_sub_and_fetch(&self->reserved, 1) == 0)
		{
			if(self->flags & BLOCK_HAS_COPY_DISPOSE)
				self->descriptor->dispose_helper(self);
			objc_delete_weak_refs((id)self);
			gc->free(self);
		}
	}
}

PRIVATE void* block_load_weak(void *block)
{
	struct Block_layout *self = block;
	return (self->reserved) > 0 ? block : 0;
}
