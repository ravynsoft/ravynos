/*
 * Copyright (c) 2012 Apple Computer, Inc. All rights reserved.
 *
 * @APPLE_LICENSE_HEADER_START@
 * 
 * The contents of this file constitute Original Code as defined in and
 * are subject to the Apple Public Source License Version 1.1 (the
 * "License").  You may not use this file except in compliance with the
 * License.  Please obtain a copy of the License at
 * http://www.apple.com/publicsource and read it before using this file.
 * 
 * This Original Code and all software distributed under the License are
 * distributed on an "AS IS" basis, WITHOUT WARRANTY OF ANY KIND, EITHER
 * EXPRESS OR IMPLIED, AND APPLE HEREBY DISCLAIMS ALL SUCH WARRANTIES,
 * INCLUDING WITHOUT LIMITATION, ANY WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE OR NON-INFRINGEMENT.  Please see the
 * License for the specific language governing rights and limitations
 * under the License.
 * 
 * @APPLE_LICENSE_HEADER_END@
 */

#ifndef KERNEL

/*
cc rballoc.c -o /tmp/rballoc -Wall -framework IOKit  -framework CoreFoundation -g 
*/

#include <assert.h>
#include <CoreFoundation/CoreFoundation.h>
#include <IOKit/IOKitLib.h>
#include <IOKit/IOKitKeys.h>
#include <Kernel/libkern/tree.h>

#define IONew(type,number)        (type*)malloc(sizeof(type) * (number) )
#define IODelete(ptr,type,number) free( (ptr) )

#define atop(n) ((n) >> 12)

typedef uint32_t vtd_rbaddr_t;

struct vtd_rblock
{
	RB_ENTRY(vtd_rblock) address_link;
	RB_ENTRY(vtd_rblock) size_link;

	vtd_rbaddr_t start;
	vtd_rbaddr_t end;
};

RB_HEAD(vtd_rbaddr_list, vtd_rblock);
RB_HEAD(vtd_rbsize_list, vtd_rblock);

struct vtd_space
{
	struct vtd_rbaddr_list rbaddr_list;
	struct vtd_rbsize_list rbsize_list;
};
typedef struct vtd_space vtd_space_t;

#define vtd_space_fault(x,y,z)

#define VTLOG(fmt, args...) printf(fmt, ## args);

#define vtassert(x)	assert(x)

int main(int argc, char **argv)
{
	vtd_space_t _list;
	vtd_space_t * bf = &_list;

	RB_INIT(&bf->rbaddr_list);
	RB_INIT(&bf->rbsize_list);

	int idx;
	vtd_rbaddr_t allocs[20];
	vtd_rbaddr_t sizes [20] = { 0x100, 0x100, 0x300, 0x100, 0x300, 0x100, 0 };
	vtd_rbaddr_t aligns[20] = { atop(2*1024*1024), atop(2*1024*1024), atop(2*1024*1024), atop(2*1024*1024), atop(2*1024*1024), atop(2*1024*1024), 0 };


	vtd_rbfree(bf, 0, 0);
	vtd_rbfree(bf, (1<<20), (1 << 21));

	vtd_rblog(bf);

#if 0

	vtd_rballoc_fixed(bf, 0x100, 0x80);
	vtd_rblog(bf);
	vtd_rballoc_fixed(bf, 0x180, 0x80);
	vtd_rblog(bf);
	vtd_rballoc_fixed(bf, 0x1, 0x1);
	vtd_rblog(bf);
	vtd_rballoc_fixed(bf, 0x2, 0xfe);
	vtd_rblog(bf);

	vtd_rbfree(bf, 50, 50);
	vtd_rblog(bf);
	vtd_rbfree(bf, 1, 49);
	vtd_rblog(bf);
	vtd_rbfree(bf, 100, 100);
	vtd_rblog(bf);
	vtd_rbfree(bf, 400, 100);
	vtd_rblog(bf);
	vtd_rbfree(bf, 250, 50);
	vtd_rblog(bf);
#endif

	for (idx = 0; sizes[idx]; idx++)
	{
		allocs[idx] = vtd_rballoc(bf, sizes[idx], aligns[idx]);
		VTLOG("alloc(0x%x) 0x%x\n", sizes[idx], allocs[idx]);
		vtd_rblog(bf);
		vtassert(allocs[idx]);
	}

	for (idx = 0; sizes[idx]; idx++)
	{
		vtd_rbfree(bf, allocs[idx], sizes[idx]);
		VTLOG("free(0x%x, 0x%x)\n", allocs[idx], sizes[idx]);
		vtd_rblog(bf);
	}


#if 0
	vtd_rbfree(bf, 300, 100);
	vtd_rblog(bf);
	vtd_rbfree(bf, 200, 50);
	vtd_rblog(bf);
#endif

    exit(0);    
}

#endif	/* KERNEL */
 


static int
vtd_rbaddr_compare(struct vtd_rblock *node, struct vtd_rblock *parent)
{
	if (node->start < parent->start) return (-1);
	if (node->start >= parent->end)  return (1);
	return (0);
}

static int
vtd_rbsize_compare(struct vtd_rblock *node, struct vtd_rblock *parent)
{
	vtd_rbaddr_t sizen = node->end - node->start;
	vtd_rbaddr_t sizep = parent->end - parent->start;

	if (sizen < sizep) return (1);
    // never a dup
	return (-1);
}


RB_PROTOTYPE_SC(static, vtd_rbaddr_list, vtd_rblock, address_link, vtd_rbaddr_compare);
RB_PROTOTYPE_SC(static, vtd_rbsize_list, vtd_rblock, size_link, vtd_rbsize_compare);

RB_GENERATE(vtd_rbaddr_list, vtd_rblock, address_link, vtd_rbaddr_compare);
RB_GENERATE(vtd_rbsize_list, vtd_rblock, size_link, vtd_rbsize_compare);

static void __unused
vtd_rblog(vtd_space_t * bf)
{
	struct vtd_rblock * elem;

	RB_FOREACH(elem, vtd_rbaddr_list, &bf->rbaddr_list)
	{
		VTLOG("[0x%x, 0x%x)\n", elem->start, elem->end);
	}
	RB_FOREACH(elem, vtd_rbsize_list, &bf->rbsize_list)
	{
		VTLOG("S[0x%x, 0x%x)\n", elem->start, elem->end);
	}

	if (!elem) vtd_rbaddr_list_RB_FIND(NULL, NULL);
	if (!elem) vtd_rbsize_list_RB_FIND(NULL, NULL);

	VTLOG("\n");
}

static vtd_rbaddr_t __unused
vtd_rbtotal(vtd_space_t * bf)
{
	struct vtd_rblock * elem;
	vtd_rbaddr_t alistsize;
	vtd_rbaddr_t slistsize;

	vtd_rblog(bf);

	alistsize = slistsize = 0;
	RB_FOREACH(elem, vtd_rbaddr_list, &bf->rbaddr_list)
	{
		alistsize += elem->end - elem->start;
	}
	RB_FOREACH(elem, vtd_rbsize_list, &bf->rbsize_list)
	{
		slistsize += elem->end - elem->start;
	}

	if (alistsize != slistsize) panic("rblists mismatch");

	return (alistsize);
}

static void
vtd_rbfree(vtd_space_t * bf, vtd_rbaddr_t addr, vtd_rbaddr_t size)
{
	struct vtd_rblock * next = RB_ROOT(&bf->rbaddr_list);
	struct vtd_rblock * prior = NULL;
	vtd_rbaddr_t        end;

	end = addr + size;

	while (next)
	{
		vtassert(addr != next->start);
		if (addr > next->start)
		{
			prior = next;
			next = RB_RIGHT(next, address_link);
		}
		else
		{
			next = RB_LEFT(next, address_link);
		}
	}			

	if (prior)
	{
		next = RB_NEXT(vtd_rbaddr_list, &bf->rbaddr_list, prior);
		if (addr != prior->end)
		{
			prior = NULL;
		}
		else
		{
			// coalesce to end of prior
			addr = prior->start;
		}
	
		if (next && (end == next->start))
		{
			if (!prior)
			{
				// coalesce to start of next
				prior = next;
				end = next->end;
			}
			else
			{
				end = next->end;
				RB_REMOVE(vtd_rbaddr_list, &bf->rbaddr_list, next);
				RB_REMOVE(vtd_rbsize_list, &bf->rbsize_list, next);
				bf->rentries--;
				IODelete(next, typeof(*next), 1);
			}
		}
	}

	if (prior)
	{
		// recolor?
		RB_REMOVE(vtd_rbaddr_list, &bf->rbaddr_list, prior);
		RB_REMOVE(vtd_rbsize_list, &bf->rbsize_list, prior);
		prior->start = addr;
		prior->end   = end;
		next = RB_INSERT(vtd_rbaddr_list, &bf->rbaddr_list, prior);
		vtassert(NULL == next);
		next = RB_INSERT(vtd_rbsize_list, &bf->rbsize_list, prior);
		vtassert(NULL == next);
	}
	else
	{
		next = IONew(typeof(*next), 1);
		bf->rentries++;
#if VTASRT
		memset(next, 0xef, sizeof(*next));
#endif
		next->start = addr;
		next->end   = end;
		prior = RB_INSERT(vtd_rbaddr_list, &bf->rbaddr_list, next);
		vtassert(NULL == prior);
		prior = RB_INSERT(vtd_rbsize_list, &bf->rbsize_list, next);
		vtassert(NULL == prior);
	}
}

static vtd_rbaddr_t 
vtd_rballoc(vtd_space_t * bf, vtd_rbaddr_t size, vtd_rbaddr_t align,
		    uint32_t mapOptions, const upl_page_info_t * pageList)
{
	vtd_rbaddr_t        addr = 0;
	vtd_rbaddr_t        end, head, tail;
	struct vtd_rblock * next = RB_ROOT(&bf->rbsize_list);
	struct vtd_rblock * prior = NULL;

	vtassert(align);
	align--;

	while (next)
	{
		head = (next->start + align) & ~align;
		if ((head + size) <= next->end)			
		{
			prior = next;
			next = RB_RIGHT(next, size_link);
		}
		else 
		{
			next = RB_LEFT(next, size_link);
		}
	}			

	if (prior)
	{
		addr = (prior->start + align) & ~align;

		vtassert((addr + size) <= prior->end);

		// recolor?
		RB_REMOVE(vtd_rbaddr_list, &bf->rbaddr_list, prior);
		RB_REMOVE(vtd_rbsize_list, &bf->rbsize_list, prior);

		end = addr + size;
		tail = prior->end - end;
		head = addr - prior->start;

		if (!head && !tail)
		{
			bf->rentries--;
			IODelete(prior, typeof(*prior), 1);
		}
		else
		{
			if (tail)
			{
				prior->start = end;
				next = RB_INSERT(vtd_rbaddr_list, &bf->rbaddr_list, prior);
				vtassert(NULL == next);
				next = RB_INSERT(vtd_rbsize_list, &bf->rbsize_list, prior);
				vtassert(NULL == next);
			}
			if (head)
			{
				if (tail)
				{
					prior = IONew(typeof(*prior), 1);
					bf->rentries++;
#if VASRT
					memset(prior, 0xef, sizeof(*prior));
#endif
					prior->start = addr - head;
					prior->end   = addr;
				}
				else
				{
					prior->end = addr;
				}
				next = RB_INSERT(vtd_rbaddr_list, &bf->rbaddr_list, prior);
				vtassert(NULL == next);
				next = RB_INSERT(vtd_rbsize_list, &bf->rbsize_list, prior);
				vtassert(NULL == next);
			}
		}
	}

	return (addr);
}

static vtd_rbaddr_t
vtd_rballoc_fixed(vtd_space_t * bf, vtd_rbaddr_t addr, vtd_rbaddr_t size)
{
	vtd_rbaddr_t end, head, tail;
	struct vtd_rblock * prior = RB_ROOT(&bf->rbaddr_list);
	struct vtd_rblock * next  = NULL;

	end = addr + size;
	while (prior)
	{
		if (addr >= prior->start)
		{
			if (end <= prior->end) break;
			prior = RB_RIGHT(prior, address_link);
		}
		else
		{
			prior = RB_LEFT(prior, address_link);
		}
	}

	if (prior)
	{
		// recolor?
		RB_REMOVE(vtd_rbaddr_list, &bf->rbaddr_list, prior);
		RB_REMOVE(vtd_rbsize_list, &bf->rbsize_list, prior);

		tail = prior->end - end;
		head = addr - prior->start;

		if (tail)
		{
			prior->start = end;
			next = RB_INSERT(vtd_rbaddr_list, &bf->rbaddr_list, prior);
			vtassert(NULL == next);
			next = RB_INSERT(vtd_rbsize_list, &bf->rbsize_list, prior);
			vtassert(NULL == next);
		}
		if (head)
		{
			if (tail)
			{
				prior = IONew(typeof(*prior), 1);
				bf->rentries++;
#if VASRT
				memset(prior, 0xef, sizeof(*prior));
#endif
				prior->start = addr - head;
				prior->end   = addr;
			}
			else
			{
				prior->end = addr;
			}
			next = RB_INSERT(vtd_rbaddr_list, &bf->rbaddr_list, prior);
			vtassert(NULL == next);
			next = RB_INSERT(vtd_rbsize_list, &bf->rbsize_list, prior);
			vtassert(NULL == next);
		}
	}

	return (addr);
}


static void
vtd_rballocator_init(vtd_space_t * bf, ppnum_t start, ppnum_t size)
{
	RB_INIT(&bf->rbaddr_list);
	RB_INIT(&bf->rbsize_list);

	vtd_rbfree(bf, 0, 0);
	vtd_rbfree(bf, start, size);
}

static void
vtd_rballocator_free(vtd_space_t * bf)
{
	struct vtd_rblock * elem;
	struct vtd_rblock * next;

	RB_FOREACH_SAFE(elem, vtd_rbaddr_list, &bf->rbaddr_list, next)
	{
		RB_REMOVE(vtd_rbaddr_list, &bf->rbaddr_list, elem);
		bf->rentries--;
		IODelete(elem, typeof(*elem), 1);
	}
}
