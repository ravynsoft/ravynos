/*
 * Copyright 2011 Joakim Sindholt <opensource@zhasha.com>
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

#ifndef _NINE_HELPERS_H_
#define _NINE_HELPERS_H_

#include "iunknown.h"
#include "nine_lock.h"

/*
 * Note: we use these function rather than the MIN2, MAX2, CLAMP macros to
 * avoid evaluating arguments (which are often function calls) more than once.
 */

static inline unsigned _min(unsigned a, unsigned b)
{
   return (a < b) ? a : b;
}


/* Sshhh ... */
#define nine_reference(a, b) _nine_reference((void **)(a), (b))

static inline void _nine_reference(void **ref, void *ptr)
{
    if (*ref != ptr) {
        if (*ref)
            NineUnknown_Release(*ref);
        if (ptr)
            NineUnknown_AddRef(ptr);
        *ref = ptr;
    }
}

#define nine_reference_set(a, b) _nine_reference_set((void **)(a), (b))

static inline void _nine_reference_set(void **ref, void *ptr)
{
    *ref = ptr;
    if (ptr)
        NineUnknown_AddRef(ptr);
}

#define nine_bind(a, b) _nine_bind((void **)(a), (b))

static inline void _nine_bind(void **dst, void *obj)
{
    if (*dst != obj) {
        if (*dst)
            NineUnknown_Unbind(*dst);
        if (obj)
            NineUnknown_Bind(obj);
        *dst = obj;
    }
}

#define NINE_DEVICE_CHILD_NEW(nine, out, dev, ...) \
    { \
        struct NineUnknownParams __params; \
        struct Nine##nine *__data; \
         \
        __data = CALLOC_STRUCT(Nine##nine); \
        if (!__data) { return E_OUTOFMEMORY; } \
         \
        __params.vtable = ((dev)->params.BehaviorFlags & D3DCREATE_MULTITHREADED) ? &Lock##nine##_vtable : &Nine##nine##_vtable; \
        __params.guids = Nine##nine##_IIDs; \
        __params.dtor = (void *)Nine##nine##_dtor; \
        __params.container = NULL; \
        __params.device = dev; \
        __params.start_with_bind_not_ref = false; \
        { \
            HRESULT __hr = Nine##nine##_ctor(__data, &__params, ## __VA_ARGS__); \
            if (FAILED(__hr)) { \
                Nine##nine##_dtor(__data); \
                return __hr; \
            } \
        } \
         \
        *(out) = __data; \
    } \
    return D3D_OK

#define NINE_DEVICE_CHILD_BIND_NEW(nine, out, dev, ...) \
    { \
        struct NineUnknownParams __params; \
        struct Nine##nine *__data; \
         \
        __data = CALLOC_STRUCT(Nine##nine); \
        if (!__data) { return E_OUTOFMEMORY; } \
         \
        __params.vtable = ((dev)->params.BehaviorFlags & D3DCREATE_MULTITHREADED) ? &Lock##nine##_vtable : &Nine##nine##_vtable; \
        __params.guids = Nine##nine##_IIDs; \
        __params.dtor = (void *)Nine##nine##_dtor; \
        __params.container = NULL; \
        __params.device = dev; \
        __params.start_with_bind_not_ref = true; \
        { \
            HRESULT __hr = Nine##nine##_ctor(__data, &__params, ## __VA_ARGS__); \
            if (FAILED(__hr)) { \
                Nine##nine##_dtor(__data); \
                return __hr; \
            } \
        } \
         \
        *(out) = __data; \
    } \
    return D3D_OK

#define NINE_NEW(nine, out, lock, ...) \
    { \
        struct NineUnknownParams __params; \
        struct Nine##nine *__data; \
         \
        __data = CALLOC_STRUCT(Nine##nine); \
        if (!__data) { return E_OUTOFMEMORY; } \
         \
        __params.vtable = (lock) ? &Lock##nine##_vtable : &Nine##nine##_vtable; \
        __params.guids = Nine##nine##_IIDs; \
        __params.dtor = (void *)Nine##nine##_dtor; \
        __params.container = NULL; \
        __params.device = NULL; \
        __params.start_with_bind_not_ref = false; \
        { \
            HRESULT __hr = Nine##nine##_ctor(__data, &__params, ## __VA_ARGS__); \
            if (FAILED(__hr)) { \
                Nine##nine##_dtor(__data); \
                return __hr; \
            } \
        } \
         \
        *(out) = __data; \
    } \
    return D3D_OK

static inline float asfloat(DWORD value)
{
    union {
        float f;
        DWORD w;
    } u;
    u.w = value;
    return u.f;
}

struct nine_range
{
    struct nine_range *next;
    int16_t bgn; /* inclusive */
    int16_t end; /* exclusive */
};

/* We won't ever need more than 256 ranges, so just allocate once. */
struct nine_range_pool
{
    struct nine_range *free;
    struct nine_range **slabs;
    unsigned num_slabs;
    unsigned num_slabs_max;
};

static inline void
nine_range_pool_put(struct nine_range_pool *pool, struct nine_range *r)
{
    r->next = pool->free;
    pool->free = r;
}

static inline void
nine_range_pool_put_chain(struct nine_range_pool *pool,
                          struct nine_range *head,
                          struct nine_range *tail)
{
    tail->next = pool->free;
    pool->free = head;
}

void
nine_ranges_insert(struct nine_range **head, int16_t bgn, int16_t end,
                   struct nine_range_pool *pool);

#endif /* _NINE_HELPERS_H_ */
