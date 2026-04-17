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

#include <stdlib.h>

#define KERNEL 1
#define HFS 1
#define RANGELIST_TEST	1

static void *hfs_malloc(size_t size)
{
    return malloc(size);
}

static void hfs_free(void *ptr, __unused size_t size)
{
    return free(ptr);
}

#include "../core/rangelist.c"

#include "test-utils.h"

int main (void)
{
	struct rl_entry r = { .rl_start = 10, .rl_end = 20 };

#define CHECK(s, e, res)	\
	assert_equal_int(rl_overlap(&r, s, e), res)

	CHECK(0, 9, RL_NOOVERLAP);
	CHECK(0, 10, RL_OVERLAPENDSAFTER);
	CHECK(0, 19, RL_OVERLAPENDSAFTER);
	CHECK(0, 20, RL_OVERLAPISCONTAINED);
	CHECK(0, 21, RL_OVERLAPISCONTAINED);

	CHECK(9, 9, RL_NOOVERLAP);
	CHECK(9, 10, RL_OVERLAPENDSAFTER);
	CHECK(9, 19, RL_OVERLAPENDSAFTER);
	CHECK(9, 20, RL_OVERLAPISCONTAINED);
	CHECK(9, 21, RL_OVERLAPISCONTAINED);

	CHECK(10, 10, RL_OVERLAPCONTAINSRANGE);
	CHECK(10, 19, RL_OVERLAPCONTAINSRANGE);
	CHECK(10, 20, RL_MATCHINGOVERLAP);
	CHECK(10, 21, RL_OVERLAPISCONTAINED);

	CHECK(19, 19, RL_OVERLAPCONTAINSRANGE);
	CHECK(19, 20, RL_OVERLAPCONTAINSRANGE);
	CHECK(19, 21, RL_OVERLAPSTARTSBEFORE);

	CHECK(20, 20, RL_OVERLAPCONTAINSRANGE);
	CHECK(20, 21, RL_OVERLAPSTARTSBEFORE);

	CHECK(21, 21, RL_NOOVERLAP);

	printf("[PASSED] rangelist_test\n");

	return 0;
}
