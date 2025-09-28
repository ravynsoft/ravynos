/*
 * Copyright 2014-2015 iXsystems, Inc.
 * All rights reserved
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted providing that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING
 * IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 */

#include <sys/types.h>
#include <machine/atomic.h>
 
typedef int OSSpinLock;

#define OSAtomicIncrement32Barrier(v) atomic_fetchadd_int((volatile int *)v, 1)
#define OSAtomicDecrement32Barrier(v) atomic_fetchadd_int((volatile int *)v, -1)
#define OSAtomicTestAndSetBarrier(i, v) atomic_testandset_int((volatile int *)v, i)
#define OSAtomicAdd64(i, v) atomic_add_64((volatile int *)v, i)
#define OSAtomicIncrement32(v) atomic_fetchadd_32((volatile int *)v, 1)
#define OSAtomicDecrement32(v) atomic_fetchadd_32((volatile int *)v, -1)
#define OSAtomicCompareAndSwapLongBarrier(o, i, v) atomic_cmpset_long(v, o, i)
#define OSAtomicCompareAndSwap32Barrier(o, i, v) atomic_cmpset_32(v, o, i)
#define OSSpinLockLock(l) while (atomic_testandset_int((volatile int *)l, 1) == 1)
#define OSSpinLockUnlock(l) atomic_clear_int((volatile int *)l, 1)
