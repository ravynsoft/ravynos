/*
 * Copyright 2009 Nicolai Hähnle <nhaehnle@gmail.com>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * on the rights to use, copy, modify, merge, publish, distribute, sub
 * license, and/or sell copies of the Software, and to permit persons to whom
 * the Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the next
 * paragraph) shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT. IN NO EVENT SHALL
 * THE AUTHOR(S) AND/OR THEIR SUPPLIERS BE LIABLE FOR ANY CLAIM,
 * DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
 * OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE
 * USE OR OTHER DEALINGS IN THE SOFTWARE. */

#include "memory_pool.h"

#include <assert.h>
#include <stdlib.h>
#include <string.h>


#define POOL_LARGE_ALLOC 4096
#define POOL_ALIGN 8


struct memory_block {
	struct memory_block * next;
};

void memory_pool_init(struct memory_pool * pool)
{
	memset(pool, 0, sizeof(struct memory_pool));
}


void memory_pool_destroy(struct memory_pool * pool)
{
	while(pool->blocks) {
		struct memory_block * block = pool->blocks;
		pool->blocks = block->next;
		free(block);
	}
}

static void refill_pool(struct memory_pool * pool)
{
	unsigned int blocksize = pool->total_allocated;
	struct memory_block * newblock;

	if (!blocksize)
		blocksize = 2*POOL_LARGE_ALLOC;

	newblock = malloc(blocksize);
	newblock->next = pool->blocks;
	pool->blocks = newblock;

	pool->head = (unsigned char*)(newblock + 1);
	pool->end = ((unsigned char*)newblock) + blocksize;
	pool->total_allocated += blocksize;
}


void * memory_pool_malloc(struct memory_pool * pool, unsigned int bytes)
{
	if (bytes < POOL_LARGE_ALLOC) {
		void * ptr;

		if (pool->head + bytes > pool->end)
			refill_pool(pool);

		assert(pool->head + bytes <= pool->end);

		ptr = pool->head;

		pool->head += bytes;
		pool->head = (unsigned char*)(((unsigned long)pool->head + POOL_ALIGN - 1) & ~(POOL_ALIGN - 1));

		return ptr;
	} else {
		struct memory_block * block = malloc(bytes + sizeof(struct memory_block));

		block->next = pool->blocks;
		pool->blocks = block;

		return (block + 1);
	}
}


