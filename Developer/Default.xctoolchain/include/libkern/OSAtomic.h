/*
 * Copyright (c) 2004-2006 Apple Computer, Inc. All rights reserved.
 *
 * @APPLE_LICENSE_HEADER_START@
 * 
 * This file contains Original Code and/or Modifications of Original Code
 * as defined in and that are subject to the Apple Public Source License
 * Version 2.0 (the 'License'). You may not use this file except in
 * compliance with the License. Please obtain a copy of the License at
 * http://www.opensource.apple.com/apsl/ and read it before using this
 * file.
 * 
 * The Original Code and all software distributed under the License are
 * distributed on an 'AS IS' basis, WITHOUT WARRANTY OF ANY KIND, EITHER
 * EXPRESS OR IMPLIED, AND APPLE HEREBY DISCLAIMS ALL SUCH WARRANTIES,
 * INCLUDING WITHOUT LIMITATION, ANY WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, QUIET ENJOYMENT OR NON-INFRINGEMENT.
 * Please see the License for the specific language governing rights and
 * limitations under the License.
 * 
 * @APPLE_LICENSE_HEADER_END@
 */

#ifndef _OSATOMIC_H_
#define _OSATOMIC_H_

#include    <stddef.h>
#include    <sys/cdefs.h>
#include    <stdint.h>
#include    <stdbool.h>

/* These are the preferred versions of the atomic and synchronization operations.
 * Their implementation is customized at boot time for the platform, including
 * late-breaking errata fixes as necessary.   They are thread safe.
 *
 * WARNING: all addresses passed to these functions must be "naturally aligned", ie
 * int32_t's must be 32-bit aligned (low 2 bits of address zero), and int64_t's
 * must be 64-bit aligned (low 3 bits of address zero.)
 *
 * Note that some versions of the atomic functions incorporate memory barriers,
 * and some do not.  Barriers strictly order memory access on a weakly-ordered
 * architecture such as PPC.  All loads and stores executed in sequential program
 * order before the barrier will complete before any load or store executed after
 * the barrier.  On a uniprocessor, the barrier operation is typically a nop.
 * On a multiprocessor, the barrier can be quite expensive on some platforms,
 * eg PPC.
 *
 * Most code will want to use the barrier functions to insure that memory shared
 * between threads is properly synchronized.  For example, if you want to initialize
 * a shared data structure and then atomically increment a variable to indicate
 * that the initialization is complete, then you must use OSAtomicIncrement32Barrier()
 * to ensure that the stores to your data structure complete before the atomic add.
 * Likewise, the consumer of that data structure must use OSAtomicDecrement32Barrier(),
 * in order to ensure that their loads of the structure are not executed before
 * the atomic decrement.  On the other hand, if you are simply incrementing a global
 * counter, then it is safe and potentially faster to use OSAtomicIncrement32().
 *
 * If you are unsure which version to use, prefer the barrier variants as they are
 * safer.
 *
 * The spinlock and queue operations always incorporate a barrier.
 */ 
__BEGIN_DECLS


/* Arithmetic functions.  They return the new value.
 */
int32_t	OSAtomicAdd32( int32_t __theAmount, volatile int32_t *__theValue );
int32_t	OSAtomicAdd32Barrier( int32_t __theAmount, volatile int32_t *__theValue );

__inline static
int32_t	OSAtomicIncrement32( volatile int32_t *__theValue )
            { return OSAtomicAdd32(  1, __theValue); }
__inline static
int32_t	OSAtomicIncrement32Barrier( volatile int32_t *__theValue )
            { return OSAtomicAdd32Barrier(  1, __theValue); }

__inline static
int32_t	OSAtomicDecrement32( volatile int32_t *__theValue )
            { return OSAtomicAdd32( -1, __theValue); }
__inline static
int32_t	OSAtomicDecrement32Barrier( volatile int32_t *__theValue )
            { return OSAtomicAdd32Barrier( -1, __theValue); }

/* cctools-port: added defined(__ppc__) || */
#if defined(__ppc__) || defined(__ppc64__) || defined(__i386__) || defined(__x86_64__) || defined(__arm__) || defined(__arm64__)

int64_t	OSAtomicAdd64( int64_t __theAmount, volatile int64_t *__theValue );
int64_t	OSAtomicAdd64Barrier( int64_t __theAmount, volatile int64_t *__theValue );

__inline static
int64_t	OSAtomicIncrement64( volatile int64_t *__theValue )
            { return OSAtomicAdd64(  1, __theValue); }
__inline static
int64_t	OSAtomicIncrement64Barrier( volatile int64_t *__theValue )
            { return OSAtomicAdd64Barrier(  1, __theValue); }

__inline static
int64_t	OSAtomicDecrement64( volatile int64_t *__theValue )
            { return OSAtomicAdd64( -1, __theValue); }
__inline static
int64_t	OSAtomicDecrement64Barrier( volatile int64_t *__theValue )
            { return OSAtomicAdd64Barrier( -1, __theValue); }

#endif  /* defined(__ppc__) || defined(__ppc64__) || defined(__i386__) || defined(__x86_64__) || defined(__arm__) || defined(__arm64__) */


/* Boolean functions (and, or, xor.)  These come in four versions for each operation:
 * with and without barriers, and returning the old or new value of the operation.
 * The "Orig" versions return the original value, ie before the operation, the non-Orig
 * versions return the value after the operation.  All are layered on top of
 * compare-and-swap.
 */
int32_t	OSAtomicOr32( uint32_t __theMask, volatile uint32_t *__theValue );
int32_t	OSAtomicOr32Barrier( uint32_t __theMask, volatile uint32_t *__theValue );
int32_t	OSAtomicOr32Orig( uint32_t __theMask, volatile uint32_t *__theValue );
int32_t	OSAtomicOr32OrigBarrier( uint32_t __theMask, volatile uint32_t *__theValue );

int32_t	OSAtomicAnd32( uint32_t __theMask, volatile uint32_t *__theValue ); 
int32_t	OSAtomicAnd32Barrier( uint32_t __theMask, volatile uint32_t *__theValue ); 
int32_t	OSAtomicAnd32Orig( uint32_t __theMask, volatile uint32_t *__theValue ); 
int32_t	OSAtomicAnd32OrigBarrier( uint32_t __theMask, volatile uint32_t *__theValue ); 

int32_t	OSAtomicXor32( uint32_t __theMask, volatile uint32_t *__theValue );
int32_t	OSAtomicXor32Barrier( uint32_t __theMask, volatile uint32_t *__theValue );
int32_t	OSAtomicXor32Orig( uint32_t __theMask, volatile uint32_t *__theValue );
int32_t	OSAtomicXor32OrigBarrier( uint32_t __theMask, volatile uint32_t *__theValue );
 

/* Compare and swap.  They return true if the swap occured.  There are several versions,
 * depending on data type and whether or not a barrier is used.
 */
bool    OSAtomicCompareAndSwap32( int32_t __oldValue, int32_t __newValue, volatile int32_t *__theValue );
bool    OSAtomicCompareAndSwap32Barrier( int32_t __oldValue, int32_t __newValue, volatile int32_t *__theValue );
bool	OSAtomicCompareAndSwapPtr( void *__oldValue, void *__newValue, void * volatile *__theValue );
bool	OSAtomicCompareAndSwapPtrBarrier( void *__oldValue, void *__newValue, void * volatile *__theValue );
bool	OSAtomicCompareAndSwapInt( int __oldValue, int __newValue, volatile int *__theValue );
bool	OSAtomicCompareAndSwapIntBarrier( int __oldValue, int __newValue, volatile int *__theValue );
bool	OSAtomicCompareAndSwapLong( long __oldValue, long __newValue, volatile long *__theValue );
bool	OSAtomicCompareAndSwapLongBarrier( long __oldValue, long __newValue, volatile long *__theValue );

#if defined(__ppc64__) || defined(__i386__) || defined(__x86_64__) || defined(__arm__) || defined(__arm64__)

bool    OSAtomicCompareAndSwap64( int64_t __oldValue, int64_t __newValue, volatile int64_t *__theValue );
bool    OSAtomicCompareAndSwap64Barrier( int64_t __oldValue, int64_t __newValue, volatile int64_t *__theValue );

#endif  /* defined(__ppc64__) || defined(__i386__) || defined(__x86_64__) || defined(__arm__) || defined(__arm64__) */


/* Test and set.  They return the original value of the bit, and operate on bit (0x80>>(n&7))
 * in byte ((char*)theAddress + (n>>3)).
 */
bool    OSAtomicTestAndSet( uint32_t __n, volatile void *__theAddress );
bool    OSAtomicTestAndSetBarrier( uint32_t __n, volatile void *__theAddress );
bool    OSAtomicTestAndClear( uint32_t __n, volatile void *__theAddress );
bool    OSAtomicTestAndClearBarrier( uint32_t __n, volatile void *__theAddress );
 

/* Spinlocks.  These use memory barriers as required to synchronize access to shared
 * memory protected by the lock.  The lock operation spins, but employs various strategies
 * to back off if the lock is held, making it immune to most priority-inversion livelocks.
 * The try operation immediately returns false if the lock was held, true if it took the
 * lock.  The convention is that unlocked is zero, locked is nonzero.
 */
#define	OS_SPINLOCK_INIT    0

typedef int32_t OSSpinLock;

bool    OSSpinLockTry( volatile OSSpinLock *__lock );
void    OSSpinLockLock( volatile OSSpinLock *__lock );
void    OSSpinLockUnlock( volatile OSSpinLock *__lock );


/* Lockless atomic enqueue and dequeue.  These routines manipulate singly
 * linked LIFO lists.  Ie, a dequeue will return the most recently enqueued
 * element, or NULL if the list is empty.  The "offset" parameter is the offset
 * in bytes of the link field within the data structure being queued.  The
 * link field should be a pointer type.  Memory barriers are incorporated as 
 * needed to permit thread-safe access to the queue element.
 */
#if defined(__x86_64__)

typedef volatile struct {
	void	*opaque1;
	long	 opaque2;
} OSQueueHead __attribute__ ((aligned (16)));

#else

typedef volatile struct {
	void	*opaque1;
	long	 opaque2;
} OSQueueHead;

#endif

#define	OS_ATOMIC_QUEUE_INIT	{ NULL, 0 }

void  OSAtomicEnqueue( OSQueueHead *__list, void *__new, size_t __offset);
void* OSAtomicDequeue( OSQueueHead *__list, size_t __offset);


/* Memory barrier.  It is both a read and write barrier.
 */
void    OSMemoryBarrier( void );


__END_DECLS

#endif /* _OSATOMIC_H_ */
