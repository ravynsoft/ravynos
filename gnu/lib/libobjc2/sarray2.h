/**
 * Sparse Array
 *
 * Author: David Chisnall
 * 
 * License: See COPYING.MIT
 *
 */

#ifndef _SARRAY_H_INCLUDED_
#define _SARRAY_H_INCLUDED_
#include <stdint.h>
#include <stdlib.h>
#include "visibility.h"


/**
 * The size of the data array.  The sparse array is a tree with this many
 * children at each node depth.
 */
static const uint32_t data_size = 256;
/**
 * The mask used to access the elements in the data array in a sparse array
 * node.
 */
static const uint32_t data_mask = data_size - 1;
/**
 * Sparse arrays, used to implement dispatch tables.  Current implementation is
 * quite RAM-intensive and could be optimised.  Maps 32-bit integers to pointers.
 *
 * Note that deletion from the array is not supported.  This allows accesses to
 * be done without locking; the worst that can happen is that the caller gets
 * an old value (and if this is important to you then you should be doing your
 * own locking).  For this reason, you should be very careful when deleting a
 * sparse array that there are no references to it held by other threads.
 */
typedef struct 
{
	/**
	 * Number of bits that the masked value should be right shifted by to get
	 * the index in the subarray.  If this value is greater than zero, then the
	 * value in the array is another SparseArray*.
	 */
	uint32_t shift;
	/**
	 * The reference count for this.  Used for copy-on-write.  When making a
	 * copy of a sparse array, we only copy the root node, and increment the
	 * reference count of the remaining nodes.  When modifying any leaf node,
	 * we copy if its reference count is greater than one.
	 */
	uint32_t refCount;
	/**
	 * The data stored in this sparse array node.
	 */
	void *data[data_size];
} SparseArray;

/**
 * Turn an index in the array into an index in the current depth.
 */
#define MASK_INDEX(index) \
	((index >> sarray->shift) & 0xff)

#define SARRAY_EMPTY ((void*)0)
/**
 * Look up the specified value in the sparse array.  This is used in message
 * dispatch and so has been put in the header to allow compilers to inline it,
 * even though this breaks the abstraction.
 */
static inline void* SparseArrayLookup(SparseArray * sarray, uint32_t index)
{
	// This unrolled version of the commented-out segment below only works with
	// sarrays that use one-byte leafs.  It's really ugly, but seems to be faster.
	// With this version, we get the same performance as the old GNU code, but
	// with about half the memory usage.
	uint32_t i = index;
	switch (sarray->shift)
	{
		default: UNREACHABLE("broken sarray");
		case 0:
			return sarray->data[i & 0xff];
		case 8:
			return 
				((SparseArray*)sarray->data[(i & 0xff00)>>8])->data[(i & 0xff)];
		case 16:
			return 
				((SparseArray*)((SparseArray*)
					sarray->data[(i & 0xff0000)>>16])->
						data[(i & 0xff00)>>8])->data[(i & 0xff)];
		case 24:
			return 
				((SparseArray*)((SparseArray*)((SparseArray*)
					sarray->data[(i & 0xff000000)>>24])->
						data[(i & 0xff0000)>>16])->
							data[(i & 0xff00)>>8])->data[(i & 0xff)];
	}
	/*
	while(sarray->shift > 0)
	{
		uint32_t i = MASK_INDEX(index);
		sarray = (SparseArray*) sarray->data[i];
	}
	uint32_t i = index & sarray->mask;
	return sarray->data[i];
	*/
}
/**
 * Create a new sparse array.
 */
SparseArray *SparseArrayNew();
/**
 * Creates a new sparse array with the specified capacity.  The depth indicates
 * the number of bits to use for the key.  Must be a value between 8 and 32 and
 * should ideally be a multiple of base_shift.
 */
SparseArray *SparseArrayNewWithDepth(uint32_t depth);
/**
 * Returns a new sparse array created by adding this one as the first child
 * node in an expanded one.
 */
SparseArray *SparseArrayExpandingArray(SparseArray *sarray, uint32_t new_depth);
/**
 * Insert a value at the specified index.
 */
void SparseArrayInsert(SparseArray * sarray, uint32_t index, void * value);
/**
 * Destroy the sparse array.  Note that calling this while other threads are
 * performing lookups is guaranteed to break.
 */
void SparseArrayDestroy(SparseArray * sarray);
/**
 * Iterate through the array.  Returns the next non-NULL value after index and
 * sets index to the following value.  For example, an array containing values
 * at 0 and 10 will, if called with index set to 0 first return the value at 0
 * and set index to 1.  A subsequent call with index set to 1 will return the
 * value at 10 and set index to 11.
 */
void * SparseArrayNext(SparseArray * sarray, uint32_t * index);

/**
 * Creates a copy of the sparse array.
 */
SparseArray *SparseArrayCopy(SparseArray * sarray);

/**
 * Returns the total memory usage of a sparse array.  
 */
int SparseArraySize(SparseArray *sarray);

#endif //_SARRAY_H_INCLUDED_
