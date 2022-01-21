/*
 * Copyright (c) 2012-2013 Apple Inc. All rights reserved.
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

/*
 * IMPORTANT: This header file describes INTERNAL interfaces to libdispatch
 * which are subject to change in future releases of Mac OS X. Any applications
 * relying on these interfaces WILL break.
 */

#ifndef __DISPATCH_SHIMS_ATOMIC_SFB__
#define __DISPATCH_SHIMS_ATOMIC_SFB__

#if __clang__ && __clang_major__ < 5 // <rdar://problem/13833871>
#define __builtin_ffs(x) __builtin_ffs((unsigned int)(x))
#endif

// Returns UINT_MAX if all the bits in p were already set.
#define dispatch_atomic_set_first_bit(p,m) _dispatch_atomic_set_first_bit(p,m)

DISPATCH_ALWAYS_INLINE
static inline unsigned int
_dispatch_atomic_set_first_bit(volatile unsigned long *p,
		unsigned int max_index)
{
	unsigned int index;
	unsigned long b, mask, b_masked;

	for (;;) {
		b = *p;
		// ffs returns 1 + index, or 0 if none set.
		index = (unsigned int)__builtin_ffsl((long)~b);
		if (slowpath(index == 0)) {
			return UINT_MAX;
		}
		index--;
		if (slowpath(index > max_index)) {
			return UINT_MAX;
		}
		mask = ((typeof(b))1) << index;
		b_masked = b | mask;
		if (__sync_bool_compare_and_swap(p, b, b_masked)) {
			return index;
		}
	}
}

#if defined(__x86_64__) || defined(__i386__)

#undef dispatch_atomic_set_first_bit
DISPATCH_ALWAYS_INLINE
static inline unsigned int
dispatch_atomic_set_first_bit(volatile unsigned long *p, unsigned int max)
{
	unsigned long val, bit;
	if (max > (sizeof(val) * 8)) {
		__asm__ (
				 "1: \n\t"
				 "mov	%[_p], %[_val] \n\t"
				 "not	%[_val] \n\t"
				 "bsf	%[_val], %[_bit] \n\t" /* val is 0 => set zf */
				 "jz	2f \n\t"
				 "lock \n\t"
				 "bts	%[_bit], %[_p] \n\t" /* cf = prev bit val */
				 "jc	1b \n\t" /* lost race, retry */
				 "jmp	3f \n\t"
				 "2: \n\t"
				 "mov	%[_all_ones], %[_bit]" "\n\t"
				 "3: \n\t"
				 : [_p] "=m" (*p), [_val] "=&r" (val), [_bit] "=&r" (bit)
				 : [_all_ones] "i" ((typeof(bit))UINT_MAX) : "memory", "cc");
	} else {
		__asm__ (
				 "1: \n\t"
				 "mov	%[_p], %[_val] \n\t"
				 "not	%[_val] \n\t"
				 "bsf	%[_val], %[_bit] \n\t" /* val is 0 => set zf */
				 "jz	2f \n\t"
				 "cmp	%[_max], %[_bit] \n\t"
				 "jg	2f \n\t"
				 "lock \n\t"
				 "bts	%[_bit], %[_p] \n\t" /* cf = prev bit val */
				 "jc	1b \n\t" /* lost race, retry */
				 "jmp	3f \n\t"
				 "2: \n\t"
				 "mov	%[_all_ones], %[_bit]" "\n\t"
				 "3: \n\t"
				 : [_p] "=m" (*p), [_val] "=&r" (val), [_bit] "=&r" (bit)
				 : [_all_ones] "i" ((typeof(bit))UINT_MAX),
				   [_max] "g" ((typeof(bit))max) : "memory", "cc");
	}
	return (unsigned int)bit;
}

#endif


#endif // __DISPATCH_SHIMS_ATOMIC_SFB__
