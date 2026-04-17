/*
 * Copyright (c) 2014-2015 Apple Inc. All rights reserved.
 *
 * @APPLE_OSREFERENCE_LICENSE_HEADER_START@
 *
 * This file contains Original Code and/or Modifications of Original Code
 * as defined in and that are subject to the Apple Public Source License
 * Version 2.0 (the 'License'). You may not use this file except in
 * compliance with the License. The rights granted to you under the License
 * may not be used to create, or enable the creation or redistribution of,
 * unlawful or unlicensed copies of an Apple operating system, or to
 * circumvent, violate, or enable the circumvention or violation of, any
 * terms of an Apple operating system software license agreement.
 *
 * Please obtain a copy of the License at
 * http://www.opensource.apple.com/apsl/ and read it before using this file.
 *
 * The Original Code and all software distributed under the License are
 * distributed on an 'AS IS' basis, WITHOUT WARRANTY OF ANY KIND, EITHER
 * EXPRESS OR IMPLIED, AND APPLE HEREBY DISCLAIMS ALL SUCH WARRANTIES,
 * INCLUDING WITHOUT LIMITATION, ANY WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, QUIET ENJOYMENT OR NON-INFRINGEMENT.
 * Please see the License for the specific language governing rights and
 * limitations under the License.
 *
 * @APPLE_OSREFERENCE_LICENSE_HEADER_END@
 */

#include "hfs_extents_test.h"
#include "../core/hfs_extents.h"

typedef struct extent_group {
	LIST_ENTRY(extent_group) chain;
	uint32_t start_block;
	HFSPlusExtentRecord extents;
	bool deleted;
} extent_group_t;

int32_t BTDeleteRecord(filefork_t    *ff,
					   BTreeIterator *iterator)
{
	(void)ff;

	assert(!iterator->group->deleted);

	LIST_REMOVE(iterator->group, chain);

	iterator->group->deleted = true;

	return 0;
}

errno_t hfs_ext_iter_next_group(struct hfs_ext_iter *iter)
{
	off_t offset = (iter->bt_iter.key.startBlock + iter->group_block_count) * 4096;

	errno_t ret = hfs_ext_find(iter->vp, offset, iter);

	if (ret)
		return ret;

	if (iter->bt_iter.key.startBlock != offset / 4096)
		return ESTALE;

	return 0;
}

errno_t hfs_ext_iter_update(hfs_ext_iter_t *iter,
							HFSPlusExtentDescriptor *extents,
							int count,
							HFSPlusExtentRecord cat_extents)
{
	if (count % kHFSPlusExtentDensity) {
		// Zero out last group
		int to_zero = kHFSPlusExtentDensity - (count % 8);
		bzero(&extents[count], to_zero * sizeof(*extents));
		count += to_zero;
	}

	extent_group_t *group = NULL;

	if (!iter->bt_iter.key.startBlock) {
		memcpy(cat_extents, extents, sizeof(HFSPlusExtentRecord));
		group = LIST_FIRST(&iter->vp->ffork->groups);
		hfs_ext_copy_rec(extents, group->extents);

		iter->bt_iter.key.startBlock += hfs_total_blocks(extents, 8);

		count -= 8;
		extents += 8;
	} else {
		extent_group_t *next;
		LIST_FOREACH(next, &iter->vp->ffork->groups, chain) {
			if (iter->bt_iter.key.startBlock < next->start_block)
				break;
			group = next;
		}
	}

	iter->bt_iter.group = NULL;

	while (count > 0) {
		extent_group_t *new_group = malloc(sizeof(*new_group));
		hfs_ext_copy_rec(extents, new_group->extents);
		new_group->start_block = iter->bt_iter.key.startBlock;
		new_group->deleted = false;

		if (group) {
			LIST_INSERT_AFTER(group, new_group, chain);
		} else
			LIST_INSERT_HEAD(&iter->vp->ffork->groups, new_group, chain);
		group = new_group;

		iter->bt_iter.key.startBlock += hfs_total_blocks(extents, 8);

		count -= 8;
		extents += 8;
	}

	return 0;
}

errno_t hfs_ext_find(vnode_t vp, off_t offset, hfs_ext_iter_t *iter)
{
	(void)vp;
	const uint32_t block = (uint32_t)(offset / 4096);

	iter->vp = vp;
	extent_group_t *next, *group = NULL;
	LIST_FOREACH(next, &vp->ffork->groups, chain) {
		if (block < next->start_block)
			break;
		group = next;
	}

	if (!group)
		return ENOENT;

	iter->file_block = iter->bt_iter.key.startBlock = group->start_block;
	iter->bt_iter.group = group;

	for (iter->ndx = 0; iter->ndx < 8; ++iter->ndx) {
		HFSPlusExtentDescriptor *ext = &group->extents[iter->ndx];
		if (iter->file_block + ext->blockCount > block) {
			hfs_ext_copy_rec(group->extents, iter->group);
			return hfs_ext_iter_check_group(iter);
		}
		iter->file_block += ext->blockCount;
	}

	return ENOENT;
}

void dump_extents(struct extent_groups *groups)
{
	extent_group_t *group;
	int i = 0;

	LIST_FOREACH(group, groups, chain) {
		for (int j = 0; j < 8; ++j) {
			HFSPlusExtentDescriptor *ext = &group->extents[j];
			if (!ext->blockCount) {
				// Make sure all the reset are NULL
				if (LIST_NEXT(group, chain))
					goto keep_going;

				for (int k = j; k < 8; ++k) {
					if (group->extents[k].startBlock
						|| group->extents[k].blockCount) {
						goto keep_going;
					}
				}

				break;
			}

		keep_going:

			printf("%s{ %u, %u }", (i == 0 ? "" : i % 4 == 0 ? ",\n" : ", "),
				   ext->startBlock, ext->blockCount);
			++i;
		}
	}
	printf("\n");
}

void check_extents(unsigned line, vnode_t vnode,
				   const HFSPlusExtentDescriptor *extents, int count)
{
	uint32_t block = 0;
	extent_group_t *group;
	LIST_FOREACH(group, &vnode->ffork->groups, chain) {
		if (group->start_block != block)
			goto bad;
		int cnt = MIN(count, 8);
		if (memcmp(group->extents, extents, cnt * sizeof(*extents)))
			goto bad;
		while (cnt < 8) {
			if (group->extents[cnt].startBlock || group->extents[cnt].blockCount)
				goto bad;
			++cnt;
		}
		if ((count -= 8) <= 0)
			break;
		block += hfs_total_blocks(extents, 8);
		extents += 8;
	}

	if (LIST_NEXT(group, chain))
		goto bad;

	return;

bad:

	printf("hfs_extents_test:%u: error: unexpected extents:\n", line);
	dump_extents(&vnode->ffork->groups);
	exit(1);
}

int main(void)
{
	cnode_t cnode = {};
	hfsmount_t mount = {
		.blockSize = 4096,
		.hfs_extents_cp = &cnode,
	};
	filefork_t ffork = {
		.ff_blocks			= 200,
		.ff_unallocblocks	= 100,
		.groups = LIST_HEAD_INITIALIZER(ffork.groups),
	};
	struct vnode vnode = {
		.ffork = &ffork,
		.mount = &mount,
		.is_rsrc = false,
		.cnode = &cnode,
	};

	extent_group_t group = {
		.extents = { { 100, 100 } },
	};

	LIST_INSERT_HEAD(&ffork.groups, &group, chain);

	HFSPlusExtentRecord cat_extents;

#define E(...)			(HFSPlusExtentDescriptor[]) { __VA_ARGS__ }
#define lengthof(x)		(sizeof(x) / sizeof(*x))

#define CHECK_EXTENTS(...)										\
	do {														\
		HFSPlusExtentDescriptor extents_[] = __VA_ARGS__;		\
		check_extents(__LINE__, &vnode, extents_,				\
					  lengthof(extents_));						\
	} while (0)

#define CHECK_REPLACE(file_block, repl, expected)								\
	do {																		\
		HFSPlusExtentDescriptor repl_[] = repl;									\
		hfs_ext_replace(&mount, &vnode, file_block, repl_,						\
						lengthof(repl_), cat_extents);							\
		CHECK_EXTENTS(expected);												\
	} while (0)
	
	CHECK_REPLACE(10, E({ 200, 10 }),
				  E({ 100, 10 }, { 200, 10 }, { 120, 80 }));

	CHECK_REPLACE(5, E({ 300, 10 }),
				  E({ 100, 5 }, { 300, 10 }, { 205, 5 }, { 120, 80 }));

	CHECK_REPLACE(20, E({ 400, 1 }),
				  E({ 100, 5 }, { 300, 10 }, { 205, 5 }, { 400, 1 },
					{ 121, 79 }));

	CHECK_REPLACE(21, E({ 402, 1 }),
				  E({ 100, 5 }, { 300, 10 }, { 205, 5 }, { 400, 1 },
					{ 402, 1 }, { 122, 78 }));
	
	CHECK_REPLACE(22, E({ 404, 1 }),
				  E({ 100, 5 }, { 300, 10 }, { 205, 5 }, { 400, 1 },
					{ 402, 1 }, { 404, 1 }, { 123, 77 }));

	CHECK_REPLACE(23, E({ 406, 1 }),
				  E({ 100, 5 }, { 300, 10 }, { 205, 5 }, { 400, 1 },
					{ 402, 1 }, { 404, 1 }, { 406, 1 }, { 124, 76 }));

	CHECK_REPLACE(24, E({ 408, 1 }),
				  E({ 100, 5 }, { 300, 10 }, { 205, 5 }, { 400, 1 },
					{ 402, 1 }, { 404, 1 }, { 406, 1 }, { 408, 1 },
					{ 125, 75 }));

	/*
	 * OK, now we won't see any padding until we get the number of extents
	 * over 32.  So let's get to that point now.
	 */

	for (int i = 25, j = 500; i < 25 + 24; ++i, j += 2) {
		hfs_ext_replace(&mount, &vnode, i, E({ j, 1 }),
						1, cat_extents);
	}

	CHECK_EXTENTS(E({ 100, 5 }, { 300, 10 }, { 205, 5 }, { 400, 1 },
					{ 402, 1 }, { 404, 1 }, { 406, 1 }, { 408, 1 },
					{ 500, 1 }, { 502, 1 }, { 504, 1 }, { 506, 1 },
					{ 508, 1 }, { 510, 1 }, { 512, 1 }, { 514, 1 },
					{ 516, 1 }, { 518, 1 }, { 520, 1 }, { 522, 1 },
					{ 524, 1 }, { 526, 1 }, { 528, 1 }, { 530, 1 },
					{ 532, 1 }, { 534, 1 }, { 536, 1 }, { 538, 1 },
					{ 540, 1 }, { 542, 1 }, { 544, 1 }, { 546, 1 },
					{ 149, 51 }));

	/*
	 * So if we replace an extent in the first group, we should see it pad using
	 * the 205 extent and some of the 300 extent.
	 */

	CHECK_REPLACE(2, E({ 600, 1 }),
				  E({ 100, 2 }, { 600, 1 }, { 103, 2 }, { 300, 8 },
					{ 308, 1 }, { 309, 1 }, { 205, 1 }, { 206, 1 },
					{ 207, 1 }, { 208, 1 }, { 209, 1 }, { 400, 1 },
					{ 402, 1 }, { 404, 1 }, { 406, 1 }, { 408, 1 },
					{ 500, 1 }, { 502, 1 }, { 504, 1 }, { 506, 1 },
					{ 508, 1 }, { 510, 1 }, { 512, 1 }, { 514, 1 },
					{ 516, 1 }, { 518, 1 }, { 520, 1 }, { 522, 1 },
					{ 524, 1 }, { 526, 1 }, { 528, 1 }, { 530, 1 },
					{ 532, 1 }, { 534, 1 }, { 536, 1 }, { 538, 1 },
					{ 540, 1 }, { 542, 1 }, { 544, 1 }, { 546, 1 },
					{ 149, 51 }));

	/*
	 * Now try and test the case where it fails to pad and is forced to move
	 * to the next group.  First, merge the 544 and 546 extents to reduce
	 * the number of extents by 1.
	 */

	CHECK_REPLACE(47, E({ 700, 2 }),
				  E({ 100, 2 }, { 600, 1 }, { 103, 2 }, { 300, 8 },
					{ 308, 1 }, { 309, 1 }, { 205, 1 }, { 206, 1 },
					{ 207, 1 }, { 208, 1 }, { 209, 1 }, { 400, 1 },
					{ 402, 1 }, { 404, 1 }, { 406, 1 }, { 408, 1 },
					{ 500, 1 }, { 502, 1 }, { 504, 1 }, { 506, 1 },
					{ 508, 1 }, { 510, 1 }, { 512, 1 }, { 514, 1 },
					{ 516, 1 }, { 518, 1 }, { 520, 1 }, { 522, 1 },
					{ 524, 1 }, { 526, 1 }, { 528, 1 }, { 530, 1 },
					{ 532, 1 }, { 534, 1 }, { 536, 1 }, { 538, 1 },
					{ 540, 1 }, { 542, 1 }, { 700, 2 }, { 149, 51 }));

	// Now force 207-209 to be merged

	CHECK_REPLACE(21, E({ 800, 1 }),
				  E({ 100, 2 }, { 600, 1 }, { 103, 2 }, { 300, 8 },
					{ 308, 1 }, { 309, 1 }, { 205, 1 }, { 206, 1 },
					{ 207, 3 }, { 400, 1 }, { 800, 1 }, { 404, 1 },
					{ 406, 1 }, { 408, 1 }, { 500, 1 }, { 502, 1 },
					{ 504, 1 }, { 506, 1 }, { 508, 1 }, { 510, 1 },
					{ 512, 1 }, { 514, 1 }, { 516, 1 }, { 518, 1 },
					{ 520, 1 }, { 522, 1 }, { 524, 1 }, { 526, 1 },
					{ 528, 1 }, { 530, 1 }, { 532, 1 }, { 534, 1 },
					{ 536, 1 }, { 538, 1 }, { 540, 1 }, { 542, 1 },
					{ 700, 2 }, { 149, 51 }));

	// Now let's push the last extent into a new group

	CHECK_REPLACE(50, E({ 800, 1 }),
				  E({ 100, 2 }, { 600, 1 }, { 103, 2 }, { 300, 8 },
					{ 308, 1 }, { 309, 1 }, { 205, 1 }, { 206, 1 },
					{ 207, 3 }, { 400, 1 }, { 800, 1 }, { 404, 1 },
					{ 406, 1 }, { 408, 1 }, { 500, 1 }, { 502, 1 },
					{ 504, 1 }, { 506, 1 }, { 508, 1 }, { 510, 1 },
					{ 512, 1 }, { 514, 1 }, { 516, 1 }, { 518, 1 },
					{ 520, 1 }, { 522, 1 }, { 524, 1 }, { 526, 1 },
					{ 528, 1 }, { 530, 1 }, { 532, 1 }, { 534, 1 },
					{ 536, 1 }, { 538, 1 }, { 540, 1 }, { 542, 1 },
					{ 700, 2 }, { 149, 1 }, { 800, 1 }, { 151, 49 }));

	CHECK_REPLACE(52, E({ 802, 1 }),
				  E({ 100, 2 }, { 600, 1 }, { 103, 2 }, { 300, 8 },
					{ 308, 1 }, { 309, 1 }, { 205, 1 }, { 206, 1 },
					{ 207, 3 }, { 400, 1 }, { 800, 1 }, { 404, 1 },
					{ 406, 1 }, { 408, 1 }, { 500, 1 }, { 502, 1 },
					{ 504, 1 }, { 506, 1 }, { 508, 1 }, { 510, 1 },
					{ 512, 1 }, { 514, 1 }, { 516, 1 }, { 518, 1 },
					{ 520, 1 }, { 522, 1 }, { 524, 1 }, { 526, 1 },
					{ 528, 1 }, { 530, 1 }, { 532, 1 }, { 534, 1 },
					{ 536, 1 }, { 538, 1 }, { 540, 1 }, { 542, 1 },
					{ 700, 2 }, { 149, 1 }, { 800, 1 }, { 151, 1 },
					{ 802, 1 }, { 153, 47 }));

	/*
	 * And now if we split the 207 extent, it will fail to pad within the
	 * 32 extents and so it will be forced to pull in more extents.
	 */

	CHECK_REPLACE(18, E({ 900, 1 }),
				  E({ 100, 2 }, { 600, 1 }, { 103, 2 }, { 300, 8 },
					{ 308, 1 }, { 309, 1 }, { 205, 1 }, { 206, 1 },
					{ 207, 1 }, { 900, 1 }, { 209, 1 }, { 400, 1 },
					{ 800, 1 }, { 404, 1 }, { 406, 1 }, { 408, 1 },
					{ 500, 1 }, { 502, 1 }, { 504, 1 }, { 506, 1 },
					{ 508, 1 }, { 510, 1 }, { 512, 1 }, { 514, 1 },
					{ 516, 1 }, { 518, 1 }, { 520, 1 }, { 522, 1 },
					{ 524, 1 }, { 526, 1 }, { 528, 1 }, { 530, 1 },
					{ 532, 1 }, { 534, 1 }, { 536, 1 }, { 538, 1 },
					{ 540, 1 }, { 542, 1 }, { 700, 2 }, { 149, 1 },
					{ 800, 1 }, { 151, 1 }, { 802, 1 }, { 153, 47 }));

	// Some tests covering replacing the beginning and end

	CHECK_REPLACE(0, E({ 100, 100 }), E({ 100, 100 }));
	CHECK_REPLACE(10, E({ 200, 90 }), E({ 100, 10 }, { 200, 90 }));
	CHECK_REPLACE(0, E({ 300, 10 }), E({ 300, 10 }, { 200, 90 }));

	// Test replacing with multiple extents
	CHECK_REPLACE(5, E({ 400, 1 }, { 400, 2 }),
				  E({ 300, 5 }, { 400, 1 }, { 400, 2 }, { 308, 2 }, { 200, 90 }));

	/*
	 * Test an unlikely case where we have lots of extents that could
	 * be coalesced.  When replacing extents here, we can't coalesce
	 * all of them because we will not be able to roll back.
	 */

	// First, set things up
	hfs_ext_iter_t iter;

	assert(!hfs_ext_find(&vnode, 0, &iter));

	for (int i = 0; i < 32768; i += 8) {
		HFSPlusExtentRecord r = { { i, 1 }, { i + 1, 1 }, { i + 2, 1 }, { i + 3, 1 }, 
								  { i + 4, 1 }, { i + 5, 1 }, { i + 6, 1 }, { i + 7, 1 } };

		hfs_ext_iter_update(&iter, r, 8, cat_extents);
	}

	ffork.ff_blocks = 32768;
	ffork.ff_unallocblocks = 0;

	/*
	 * Now we have 32768 extents that could be coalesced.  Check the
	 * following succeeds.
	 */
	assert(!hfs_ext_replace(&mount, &vnode, 0, 
							E({ 1, 8 }), 1, cat_extents));

	printf("[PASSED] hfs_extents_test\n");

	return 0;
}
