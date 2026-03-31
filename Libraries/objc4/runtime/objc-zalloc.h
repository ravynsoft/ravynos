/*
 * Copyright (c) 2019 Apple Inc.  All Rights Reserved.
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

#ifndef _OBJC_ZALLOC_H
#define _OBJC_ZALLOC_H

#include <cstdint>
#include <atomic>
#include <cstdlib>

namespace objc {

// Darwin malloc always aligns to 16 bytes
#define MALLOC_ALIGNMENT 16

class AtomicQueue {
#if __LP64__
    using pair_t = __int128_t;
#else
    using pair_t = uint64_t;
#endif
    static constexpr auto relaxed = std::memory_order_relaxed;
    static constexpr auto release = std::memory_order_release;

    struct Entry {
        struct Entry *next;
    };

    union {
        struct {
            Entry        *head;
            unsigned long gen;
        };
        std::atomic<pair_t> atomic_pair;
        pair_t pair;
    };

public:
    void *pop();
    void push_list(void *_head, void *_tail);
    inline void push(void *head)
    {
        push_list(head, head);
    }
};

template<class T, bool useMalloc>
class Zone {
};

template<class T>
class Zone<T, false> {
    struct Element {
        Element *next;
        char buf[sizeof(T) - sizeof(void *)];
    } __attribute__((packed));

    static AtomicQueue _freelist;
    static T *alloc_slow();

public:
    static T *alloc();
    static void free(T *);
};

template<class T>
class Zone<T, true> {
public:
    static inline T *alloc() {
        return reinterpret_cast<T *>(::calloc(sizeof(T), 1));
    }
    static inline void free(T *ptr) {
        ::free(ptr);
    }
};

/*
 * This allocator returns always zeroed memory,
 * and the template needs to be instantiated in objc-zalloc.mm
 */

template<class T>
T *zalloc()
{
    return Zone<T, sizeof(T) % MALLOC_ALIGNMENT == 0>::alloc();
}

template<class T>
void zfree(T *e)
{
    Zone<T, sizeof(T) % MALLOC_ALIGNMENT == 0>::free(e);
}

};

#endif
