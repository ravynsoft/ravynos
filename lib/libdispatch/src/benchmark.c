/*
 * Copyright (c) 2008-2013 Apple Inc. All rights reserved.
 *
 * @APPLE_APACHE_LICENSE_HEADER_START@
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * @APPLE_APACHE_LICENSE_HEADER_END@
 */

#include "internal.h"

struct __dispatch_benchmark_data_s {
#if HAVE_MACH_ABSOLUTE_TIME
	mach_timebase_info_data_t tbi;
#endif
	uint64_t loop_cost;
	void (*func)(void *);
	void *ctxt;
	size_t count;
};

static void
_dispatch_benchmark_init(void *context)
{
	struct __dispatch_benchmark_data_s *bdata = context;
	// try and simulate performance of real benchmark as much as possible
	// keep 'f', 'c' and 'cnt' in registers
	register void (*f)(void *) = bdata->func;
	register void *c = bdata->ctxt;
	register size_t cnt = bdata->count;
	size_t i = 0;
	uint64_t start, delta;
#if defined(__LP64__)
	__uint128_t lcost;
#else
	long double lcost;
#endif
#if HAVE_MACH_ABSOLUTE_TIME
	kern_return_t kr;

	kr = mach_timebase_info(&bdata->tbi);
	dispatch_assert_zero(kr);
#endif

	start = _dispatch_absolute_time();
	do {
		i++;
		f(c);
	} while (i < cnt);
	delta = _dispatch_absolute_time() - start;

	lcost = delta;
#if HAVE_MACH_ABSOLUTE_TIME
	lcost *= bdata->tbi.numer;
	lcost /= bdata->tbi.denom;
#endif
	lcost /= cnt;

	bdata->loop_cost = lcost > UINT64_MAX ? UINT64_MAX : (uint64_t)lcost;
}

#ifdef __BLOCKS__
uint64_t
dispatch_benchmark(size_t count, void (^block)(void))
{
	return dispatch_benchmark_f(count, block, _dispatch_Block_invoke(block));
}
#endif

static void
_dispatch_benchmark_dummy_function(void *ctxt DISPATCH_UNUSED)
{
}

uint64_t
dispatch_benchmark_f(size_t count, register void *ctxt,
		register void (*func)(void *))
{
	static struct __dispatch_benchmark_data_s bdata = {
		.func = _dispatch_benchmark_dummy_function,
		.count = 10000000ul, // ten million
	};
	static dispatch_once_t pred;
	uint64_t ns, start, delta;
#if defined(__LP64__)
	__uint128_t conversion, big_denom;
#else
	long double conversion, big_denom;
#endif
	size_t i = 0;

	dispatch_once_f(&pred, &bdata, _dispatch_benchmark_init);

	if (slowpath(count == 0)) {
		return 0;
	}

	start = _dispatch_absolute_time();
	do {
		i++;
		func(ctxt);
	} while (i < count);
	delta = _dispatch_absolute_time() - start;

	conversion = delta;
#if HAVE_MACH_ABSOLUTE_TIME
	conversion *= bdata.tbi.numer;
	big_denom = bdata.tbi.denom;
#else
	big_denom = delta;
#endif
	big_denom *= count;
	conversion /= big_denom;
	ns = conversion > UINT64_MAX ? UINT64_MAX : (uint64_t)conversion;

	return ns - bdata.loop_cost;
}
