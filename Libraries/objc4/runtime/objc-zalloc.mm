/*
 * Copyright (c) 2007 Apple Inc.  All Rights Reserved.
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

/**
 * @file objc-zalloc.h
 *
 * "zone allocator" for objc.
 *
 * Provides packed allocation for data structures the runtime
 * almost never frees.
 */

#include "objc-private.h"
#include "objc-zalloc.h"

namespace objc {

void *AtomicQueue::pop()
{
    AtomicQueue l1, l2;

    l1.pair = pair; // non atomic on purpose

    do {
        if (l1.head == nullptr) {
            return nullptr;
        }
        l2.head = l1.head->next;
        l2.gen  = l1.gen + 1;
    } while (!atomic_pair.compare_exchange_weak(l1.pair, l2.pair, relaxed, relaxed));

    return reinterpret_cast<void *>(l1.head);
}

void AtomicQueue::push_list(void *_head, void *_tail)
{
    Entry *head = reinterpret_cast<Entry *>(_head);
    Entry *tail = reinterpret_cast<Entry *>(_tail);
    AtomicQueue l1, l2;

    l1.pair = pair; // non atomic load on purpose
    do {
        tail->next = l1.head;
        l2.head = head;
        l2.gen = l1.gen + 1;
    } while (!atomic_pair.compare_exchange_weak(l1.pair, l2.pair, release, relaxed));
}

template<class T>
constexpr inline
T gcd(T a, T b)
{
    return b == 0 ? a : gcd(b, a % b);
}

template<class T>
AtomicQueue Zone<T, false>::_freelist;

template<class T>
T *Zone<T, false>::alloc_slow()
{
    // our malloc aligns to 16 bytes and this code should be used for sizes
    // small enough that this should always be an actual malloc bucket.
    //
    // The point of this code is *NOT* speed but optimal density
    constexpr size_t n_elem = MALLOC_ALIGNMENT / gcd(sizeof(T), size_t{MALLOC_ALIGNMENT});
    Element *slab = reinterpret_cast<Element *>(::calloc(n_elem, sizeof(T)));
    for (size_t i = 1; i < n_elem - 1; i++) {
        slab[i].next = &slab[i + 1];
    }
    _freelist.push_list(reinterpret_cast<void *>(&slab[1]),
                        reinterpret_cast<void *>(&slab[n_elem - 1]));
    return reinterpret_cast<T *>(&slab[0]);
}

template<class T>
T *Zone<T, false>::alloc()
{
    void *e = _freelist.pop();
    if (e) {
        __builtin_bzero(e, sizeof(void *));
        return reinterpret_cast<T *>(e);
    }
    return alloc_slow();
}

template<class T>
void Zone<T, false>::free(T *ptr)
{
    if (ptr) {
        Element *e = reinterpret_cast<Element *>(ptr);
        __builtin_bzero(e->buf, sizeof(e->buf));
        _freelist.push(e);
    }
}

#if __OBJC2__
#define ZoneInstantiate(type) \
	template class Zone<type, sizeof(type) % MALLOC_ALIGNMENT == 0>

ZoneInstantiate(class_rw_t);
ZoneInstantiate(class_rw_ext_t);
#endif

}
