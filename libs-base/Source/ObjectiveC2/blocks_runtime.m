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
#import "ObjectiveC2/objc/blocks_runtime.h"
#import "ObjectiveC2/objc/runtime.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>

/* Makes the compiler happy even without Foundation */
@interface Dummy
- (id)retain;
- (void)release;
@end

// Descriptor attributes
enum {
  BLOCK_HAS_COPY_DISPOSE =  (1 << 25),
  BLOCK_HAS_CTOR =          (1 << 26), // helpers have C++ code
  BLOCK_IS_GLOBAL =         (1 << 28),
  BLOCK_HAS_DESCRIPTOR =    (1 << 29), // interim until complete world build is accomplished
};

// _Block_object_assign() and _Block_object_dispose() flag helpers.
enum {
  BLOCK_FIELD_IS_OBJECT   =  3,  // id, NSObject, __attribute__((NSObject)), block, ...
  BLOCK_FIELD_IS_BLOCK    =  7,  // a block variable
  BLOCK_FIELD_IS_BYREF    =  8,  // the on stack structure holding the __block variable
  
  BLOCK_FIELD_IS_WEAK     = 16,  // declared __weak
  
  BLOCK_BYREF_CALLER      = 128, // called from byref copy/dispose helpers
};

// Helper structure
struct psy_block_literal {
  void *isa; // initialized to &_NSConcreteStackBlock or &_NSConcreteGlobalBlock
  int flags;
  int reserved;
  void (*invoke)(void *, ...);
  struct {
    unsigned long int reserved;	// NULL
    unsigned long int size;  // sizeof(struct Block_literal_1)
    // optional helper functions
    void (*copy_helper)(void *dst, void *src);
    void (*dispose_helper)(void *src); 
  } *descriptor;
  const char *types;
};

// Helper structure
struct psy_block_byref_obj {
  void *isa;  // uninitialized
  struct psy_block_byref_obj *forwarding;
  int flags;   //refcount;
  int size;
  void (*byref_keep)(struct psy_block_byref_obj *dst, struct psy_block_byref_obj *src);
  void (*byref_dispose)(struct psy_block_byref_obj *);
};

/* Certain field types require runtime assistance when being copied to the
 * heap.  The following function is used to copy fields of types: blocks,
 * pointers to byref structures, and objects (including
 * __attribute__((NSObject)) pointers.  BLOCK_FIELD_IS_WEAK is orthogonal to
 * the other choices which are mutually exclusive.  Only in a Block copy helper
 * will one see BLOCK_FIELD_IS_BYREF.
 */
void
_Block_object_assign(void *destAddr, void *object, const int flags)
{
  //printf("Copying %x to %x with flags %x\n", object, destAddr, flags);
  // FIXME: Needs to be implemented
  if (flags & BLOCK_FIELD_IS_WEAK)
    {
    }
  else
    {
      if (flags & BLOCK_FIELD_IS_BYREF)
        {
	  struct psy_block_byref_obj *src = object;
	  struct psy_block_byref_obj **dst = destAddr;
            
	  /* I followed Apple's specs saying byref's "flags" field should
	   * represent the refcount but it still contains real flag, so this
	   * is a little hack...
	   */
	  if ((src->flags & ~BLOCK_HAS_COPY_DISPOSE) == 0)
            {
	      *dst = malloc(src->size);
	      memcpy(*dst, src, src->size);
	      if (src->forwarding == src)
		{
		  (*dst)->forwarding = *dst;
		}
	      if (src->size >= sizeof(struct psy_block_byref_obj))
		{
		  src->byref_keep(*dst, src);
		}
            }
	  else
	    {
	      *dst = src;
	    }
            
	  (*dst)->flags++;
        }
      else if ((flags & BLOCK_FIELD_IS_BLOCK) == BLOCK_FIELD_IS_BLOCK)
        {
	  struct psy_block_literal *src = object;
	  struct psy_block_literal **dst = destAddr;
            
	  *dst = Block_copy(src);
        }
      else if ((flags & BLOCK_FIELD_IS_OBJECT) == BLOCK_FIELD_IS_OBJECT)
        {
	  id src = object;
	  id *dst = destAddr;

	  *dst = [src retain];
        }
    }
}

/* Similarly a compiler generated dispose helper needs to call back for each
 * field of the byref data structure.  (Currently the implementation only packs
 * one field into the byref structure but in principle there could be more).
 * The same flags used in the copy helper should be used for each call
 * generated to this function:
 */
void
_Block_object_dispose(void *object, const int flags)
{
  // FIXME: Needs to be implemented
  if (flags & BLOCK_FIELD_IS_WEAK)
    {
    }
  else
    {
      if (flags & BLOCK_FIELD_IS_BYREF)
        {
	  struct psy_block_byref_obj *src = object;
	  
	  src->flags--;
	  if ((src->flags & ~BLOCK_HAS_COPY_DISPOSE) == 0)
            {
	      if (src->size >= sizeof(struct psy_block_byref_obj))
		src->byref_dispose(src);
                
	      free(src);
            }
        }
      else if ((flags & ~BLOCK_BYREF_CALLER) == BLOCK_FIELD_IS_BLOCK)
        {
	  struct psy_block_literal *src = object;
	  Block_release(src);
        }
      else if ((flags & ~BLOCK_BYREF_CALLER) == BLOCK_FIELD_IS_OBJECT)
        {
	  id src = object;
	  [src release];
        }
    }
}

struct StackBlockClass {
  void *isa; // initialized to &_NSConcreteStackBlock or &_NSConcreteGlobalBlock
  int flags;
  int reserved;
  void (*invoke)(void *, ...);
  struct {
    unsigned long int reserved;  // NULL
    unsigned long int size;  // sizeof(struct Block_literal_1)
      // optional helper functions
    void (*copy_helper)(void *dst, void *src);
    void (*dispose_helper)(void *src); 
  } *descriptor;
  const char *types;
};


/* Copy a block to the heap if it's still on the stack or
 * increments its retain count.
 */
void *
_Block_copy(const void *src)
{
  struct StackBlockClass *self = (struct StackBlockClass *)src;
  struct StackBlockClass *ret = self;
  extern const void _NSConcreteStackBlock;
    
  // If the block is Global, there's no need to copy it on the heap.
  if (self->isa == &_NSConcreteStackBlock
    && self->flags & BLOCK_HAS_DESCRIPTOR)
    {
      if (self->reserved == 0)
        {
	  ret = malloc(self->descriptor->size);
	  memcpy(ret, self, self->descriptor->size);
	  if (self->flags & BLOCK_HAS_COPY_DISPOSE)
	    self->descriptor->copy_helper(ret, self);
	  memcpy(self, ret, self->descriptor->size);
        }
      ret->reserved++;
    }
  return ret;
}

// Release a block and frees the memory when the retain count hits zero.
void
_Block_release(const void *src)
{
  struct StackBlockClass *self = (struct StackBlockClass *)src;
  extern const void _NSConcreteStackBlock;

  if (self->isa == &_NSConcreteStackBlock
    // A Global block doesn't need to be released
    && self->flags & BLOCK_HAS_DESCRIPTOR
    // Should always be true...
    && self->reserved > 0)
    // If false, then it's not allocated on the heap, we won't release auto memory !
    {
      self->reserved--;
      if (self->reserved == 0)
        {
	  if (self->flags & BLOCK_HAS_COPY_DISPOSE)
	    self->descriptor->dispose_helper(self);
	  free(self);
        }
    }
}

const char *
_Block_get_types(const void *blk)
{
  const struct psy_block_literal *block = blk;
  return block->types;
}
