/**
 * buffer.h defines a simple dynamic array that is used to store temporary
 * values for later processing.  Define BUFFER_TYPE before including this file.
 */

#include <stdlib.h>

#define BUFFER_SIZE 128
static BUFFER_TYPE buffered_object_buffer[BUFFER_SIZE];
static BUFFER_TYPE *buffered_object_overflow;
static int buffered_objects;
static int buffered_object_overflow_space;

static void set_buffered_object_at_index(BUFFER_TYPE cat, unsigned int i)
{
	if (i < BUFFER_SIZE)
	{
		buffered_object_buffer[i] = cat;
	}
	else
	{
		i -= BUFFER_SIZE;
		if (NULL == buffered_object_overflow)
		{
			buffered_object_overflow =
				calloc(BUFFER_SIZE, sizeof(BUFFER_TYPE));
			buffered_object_overflow_space = BUFFER_SIZE;
		}
		while (i >= buffered_object_overflow_space)
		{
			buffered_object_overflow_space <<= 1;
			buffered_object_overflow = realloc(buffered_object_overflow,
					buffered_object_overflow_space * sizeof(BUFFER_TYPE));
		}
		buffered_object_overflow[i] = cat;
	}
}

static BUFFER_TYPE buffered_object_at_index(unsigned int i)
{
	if (i<BUFFER_SIZE)
	{
		return buffered_object_buffer[i];
	}
	return buffered_object_overflow[i-BUFFER_SIZE];
}

static void compact_buffer(void)
{
	// Move up all of the non-NULL pointers
	unsigned size = buffered_objects;
	unsigned insert = 0;
	for (unsigned i=0 ; i<size ; i++)
	{
		BUFFER_TYPE c = buffered_object_at_index(i);
		if (c != NULL)
		{
			set_buffered_object_at_index(c, insert++);
		}
	}
	buffered_objects = insert;
}
