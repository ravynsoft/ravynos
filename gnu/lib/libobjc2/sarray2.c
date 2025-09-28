#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <assert.h>

#include "sarray2.h"
#include "visibility.h"

const static SparseArray EmptyArray = { 0, 0, .data[0 ... 255] = 0 };
const static SparseArray EmptyArray8 = { 8, 0, .data[0 ... 255] = (void*)&EmptyArray};
const static SparseArray EmptyArray16 = { 16, 0, .data[0 ... 255] = (void*)&EmptyArray8};
const static SparseArray EmptyArray24 = { 24, 0, .data[0 ... 255] = (void*)&EmptyArray16};

#define MAX_INDEX(sarray) (0xff)

// Tweak this value to trade speed for memory usage.  Bigger values use more
// memory, but give faster lookups.  
#define base_shift 8
#define base_mask ((1<<base_shift) - 1)

static void *EmptyChildForShift(uint32_t shift)
{
	switch(shift)
	{
		default: UNREACHABLE("Broken sparse array");
		case 8:
			return (void*)&EmptyArray;
		case 16:
			return (void*)&EmptyArray8;
		case 24:
			return (void*)&EmptyArray16;
	}
}

static void init_pointers(SparseArray * sarray)
{
	if(sarray->shift != 0)
	{
		void *data = EmptyChildForShift(sarray->shift);
		for(unsigned i=0 ; i<=MAX_INDEX(sarray) ; i++)
		{
			sarray->data[i] = data;
		}
	}
}

PRIVATE SparseArray * SparseArrayNewWithDepth(uint32_t depth)
{
	SparseArray * sarray = calloc(1, sizeof(SparseArray));
	sarray->refCount = 1;
	sarray->shift = depth-base_shift;
	init_pointers(sarray);
	return sarray;
}

PRIVATE SparseArray *SparseArrayNew()
{
	return SparseArrayNewWithDepth(32);
}
PRIVATE SparseArray *SparseArrayExpandingArray(SparseArray *sarray, uint32_t new_depth)
{
	if (new_depth == sarray->shift)
	{
		return sarray;
	}
	assert(new_depth > sarray->shift);
	// Expanding a child sarray has undefined results.
	assert(sarray->refCount == 1);
	SparseArray *new = calloc(1, sizeof(SparseArray));
	new->refCount = 1;
	new->shift = sarray->shift + 8;
	new->data[0] = sarray;
	void *data = EmptyChildForShift(new->shift);
	for(unsigned i=1 ; i<=MAX_INDEX(sarray) ; i++)
	{
		new->data[i] = data;
	}
	// Now, any lookup in sarray for any value less than its capacity will have
	// all non-zero values shifted away, resulting in 0.  All lookups will
	// therefore go to the new sarray.
	return new;
}

static void *SparseArrayFind(SparseArray * sarray, uint32_t * index)
{
	uint32_t j = MASK_INDEX((*index));
	uint32_t max = MAX_INDEX(sarray);
	if (sarray->shift == 0)
	{
		while (j<=max)
		{
			if (sarray->data[j] != SARRAY_EMPTY)
			{
				return sarray->data[j];
			}
			(*index)++;
			j++;
		}
	}
	else while (j<max)
	{
		// If the shift is not 0, then we need to recursively look at child
		// nodes.
		uint32_t zeromask = ~((0xff << sarray->shift) >> base_shift);
		while (j<max)
		{
			//Look in child nodes
			SparseArray *child = sarray->data[j];
			// Skip over known-empty children
			if ((&EmptyArray == child) ||
			    (&EmptyArray8 == child) ||
			    (&EmptyArray16 == child) ||
			    (&EmptyArray24 == child))
			{
				//Add 2^n to index so j is still correct
				(*index) += 1<<sarray->shift;
				//Zero off the next component of the index so we don't miss any.
				*index &= zeromask;
			}
			else
			{
				// The recursive call will set index to the correct value for
				// the next index, but won't update j
				void * ret = SparseArrayFind(child, index);
				if (ret != SARRAY_EMPTY)
				{
					return ret;
				}
			}
			//Go to the next child
			j++;
		}
	}
	return SARRAY_EMPTY;
}

PRIVATE void *SparseArrayNext(SparseArray * sarray, uint32_t * idx)
{
	(*idx)++;
	return SparseArrayFind(sarray, idx);
}

PRIVATE void SparseArrayInsert(SparseArray * sarray, uint32_t index, void *value)
{
	if (sarray->shift > 0)
	{
		uint32_t i = MASK_INDEX(index);
		SparseArray *child = sarray->data[i];
		if ((&EmptyArray == child) ||
		    (&EmptyArray8 == child) ||
		    (&EmptyArray16 == child) ||
		    (&EmptyArray24 == child))
		{
			// Insert missing nodes
			SparseArray * newsarray = calloc(1, sizeof(SparseArray));
			newsarray->refCount = 1;
			if (base_shift >= sarray->shift)
			{
				newsarray->shift = 0;
			}
			else
			{
				newsarray->shift = sarray->shift - base_shift;
			}
			init_pointers(newsarray);
			sarray->data[i] = newsarray;
			child = newsarray;
		}
		else if (child->refCount > 1)
		{
			// Copy the copy-on-write part of the tree
			sarray->data[i] = SparseArrayCopy(child);
			SparseArrayDestroy(child);
			child = sarray->data[i];
		}
		SparseArrayInsert(child, index, value);
	}
	else
	{
		sarray->data[MASK_INDEX(index)] = value;
	}
}

PRIVATE SparseArray *SparseArrayCopy(SparseArray * sarray)
{
	SparseArray *copy = calloc(sizeof(SparseArray), 1);
	memcpy(copy, sarray, sizeof(SparseArray));
	copy->refCount = 1;
	// If the sarray has children, increase their refcounts and link them
	if (sarray->shift > 0)
	{
		for (unsigned int i = 0 ; i<=MAX_INDEX(sarray); i++)
		{
			SparseArray *child = copy->data[i];
			if (!(child == &EmptyArray ||
			    child == &EmptyArray8 ||
			    child == &EmptyArray16 ||
			    child == &EmptyArray24))
			{
				__sync_fetch_and_add(&child->refCount, 1);
			}
			// Non-lazy copy.  Uncomment if debugging 
			// copy->data[i] = SparseArrayCopy(copy->data[i]);
		}
	}
	return copy;
}

PRIVATE void SparseArrayDestroy(SparseArray * sarray)
{
	// Don't really delete this sarray if its ref count is > 0
	if (sarray == &EmptyArray ||
	    sarray == &EmptyArray8 ||
	    sarray == &EmptyArray16 ||
	    sarray == &EmptyArray24 ||
		(__sync_sub_and_fetch(&sarray->refCount, 1) > 0))
 	{
		return;
	}

	if(sarray->shift > 0)
	{
		for(uint32_t i=0 ; i<data_size ; i++)
		{
			SparseArrayDestroy((SparseArray*)sarray->data[i]);
		}
	}
	free(sarray);
}

#if 0
// Unused function, but helpful when debugging.
PRIVATE int SparseArraySize(SparseArray *sarray)
{
	int size = 0;
	if (sarray->shift == 0)
	{
		return 256*sizeof(void*) + sizeof(SparseArray);
	}
	size += 256*sizeof(void*) + sizeof(SparseArray);
	for(unsigned i=0 ; i<=MAX_INDEX(sarray) ; i++)
	{
		SparseArray *child = sarray->data[i];
		if (child == &EmptyArray || 
		    child == &EmptyArray8 || 
		    child == &EmptyArray16)
		{
			continue;
		}
		size += SparseArraySize(child);
	}
	return size;
}
#endif
