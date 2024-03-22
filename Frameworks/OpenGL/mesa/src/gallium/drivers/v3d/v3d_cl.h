/*
 * Copyright © 2014-2017 Broadcom
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the next
 * paragraph) shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 */

#ifndef V3D_CL_H
#define V3D_CL_H

#include <stdint.h>

#include "util/u_math.h"
#include "util/macros.h"
#include "broadcom/cle/v3d_packet_helpers.h"

struct v3d_bo;
struct v3d_job;
struct v3d_cl;

/**
 * Undefined structure, used for typechecking that you're passing the pointers
 * to these functions correctly.
 */
struct v3d_cl_out;

/** A reference to a BO used in the CL packing functions */
struct v3d_cl_reloc {
        struct v3d_bo *bo;
        uint32_t offset;
};

static inline void cl_pack_emit_reloc(struct v3d_cl *cl, const struct v3d_cl_reloc *);

#define __gen_user_data struct v3d_cl
#define __gen_address_type struct v3d_cl_reloc
#define __gen_address_offset(reloc) (((reloc)->bo ? (reloc)->bo->offset : 0) + \
                                     (reloc)->offset)
#define __gen_emit_reloc cl_pack_emit_reloc
#define __gen_unpack_address(cl, s, e) __unpack_address(cl, s, e)

static inline struct v3d_cl_reloc
__unpack_address(const uint8_t *cl, uint32_t s, uint32_t e)
{
    struct v3d_cl_reloc reloc =
            { NULL, __gen_unpack_uint(cl, s, e) << (31 - (e - s)) };
    return reloc;
}

struct v3d_cl {
        void *base;
        struct v3d_job *job;
        struct v3d_cl_out *next;
        struct v3d_bo *bo;
        uint32_t size;
};

void v3d_init_cl(struct v3d_job *job, struct v3d_cl *cl);
void v3d_destroy_cl(struct v3d_cl *cl);
void v3d_dump_cl(void *cl, uint32_t size, bool is_render);
uint32_t v3d_gem_hindex(struct v3d_job *job, struct v3d_bo *bo);

struct PACKED unaligned_16 { uint16_t x; };
struct PACKED unaligned_32 { uint32_t x; };

static inline uint32_t cl_offset(struct v3d_cl *cl)
{
        return (char *)cl->next - (char *)cl->base;
}

static inline struct v3d_cl_reloc cl_get_address(struct v3d_cl *cl)
{
        return (struct v3d_cl_reloc){ .bo = cl->bo, .offset = cl_offset(cl) };
}

static inline void
cl_advance(struct v3d_cl_out **cl, uint32_t n)
{
        (*cl) = (struct v3d_cl_out *)((char *)(*cl) + n);
}

static inline struct v3d_cl_out *
cl_start(struct v3d_cl *cl)
{
        return cl->next;
}

static inline void
cl_end(struct v3d_cl *cl, struct v3d_cl_out *next)
{
        cl->next = next;
        assert(cl_offset(cl) <= cl->size);
}


static inline void
put_unaligned_32(struct v3d_cl_out *ptr, uint32_t val)
{
        struct unaligned_32 *p = (void *)ptr;
        p->x = val;
}

static inline void
put_unaligned_16(struct v3d_cl_out *ptr, uint16_t val)
{
        struct unaligned_16 *p = (void *)ptr;
        p->x = val;
}

static inline void
cl_u8(struct v3d_cl_out **cl, uint8_t n)
{
        *(uint8_t *)(*cl) = n;
        cl_advance(cl, 1);
}

static inline void
cl_u16(struct v3d_cl_out **cl, uint16_t n)
{
        put_unaligned_16(*cl, n);
        cl_advance(cl, 2);
}

static inline void
cl_u32(struct v3d_cl_out **cl, uint32_t n)
{
        put_unaligned_32(*cl, n);
        cl_advance(cl, 4);
}

static inline void
cl_aligned_u32(struct v3d_cl_out **cl, uint32_t n)
{
        *(uint32_t *)(*cl) = n;
        cl_advance(cl, 4);
}

static inline void
cl_aligned_reloc(struct v3d_cl *cl,
                 struct v3d_cl_out **cl_out,
                 struct v3d_bo *bo, uint32_t offset)
{
        cl_aligned_u32(cl_out, bo->offset + offset);
        v3d_job_add_bo(cl->job, bo);
}

static inline void
cl_ptr(struct v3d_cl_out **cl, void *ptr)
{
        *(struct v3d_cl_out **)(*cl) = ptr;
        cl_advance(cl, sizeof(void *));
}

static inline void
cl_f(struct v3d_cl_out **cl, float f)
{
        cl_u32(cl, fui(f));
}

static inline void
cl_aligned_f(struct v3d_cl_out **cl, float f)
{
        cl_aligned_u32(cl, fui(f));
}

/**
 * Reference to a BO with its associated offset, used in the pack process.
 */
static inline struct v3d_cl_reloc
cl_address(struct v3d_bo *bo, uint32_t offset)
{
        struct v3d_cl_reloc reloc = {
                .bo = bo,
                .offset = offset,
        };
        return reloc;
}

uint32_t v3d_cl_ensure_space(struct v3d_cl *cl, uint32_t size, uint32_t align);
void v3d_cl_ensure_space_with_branch(struct v3d_cl *cl, uint32_t size);

#define cl_packet_header(packet) V3DX(packet ## _header)
#define cl_packet_length(packet) V3DX(packet ## _length)
#define cl_packet_pack(packet)   V3DX(packet ## _pack)
#define cl_packet_struct(packet) V3DX(packet)

static inline void *
cl_get_emit_space(struct v3d_cl_out **cl, size_t size)
{
        void *addr = *cl;
        cl_advance(cl, size);
        return addr;
}

/* Macro for setting up an emit of a CL struct.  A temporary unpacked struct
 * is created, which you get to set fields in of the form:
 *
 * cl_emit(bcl, FLAT_SHADE_FLAGS, flags) {
 *     .flags.flat_shade_flags = 1 << 2,
 * }
 *
 * or default values only can be emitted with just:
 *
 * cl_emit(bcl, FLAT_SHADE_FLAGS, flags);
 *
 * The trick here is that we make a for loop that will execute the body
 * (either the block or the ';' after the macro invocation) exactly once.
 */
#define cl_emit(cl, packet, name)                                \
        for (struct cl_packet_struct(packet) name = {            \
                cl_packet_header(packet)                         \
        },                                                       \
        *_loop_terminate = &name;                                \
        __builtin_expect(_loop_terminate != NULL, 1);            \
        ({                                                       \
                struct v3d_cl_out *cl_out = cl_start(cl);        \
                cl_packet_pack(packet)(cl, (uint8_t *)cl_out, &name); \
                cl_advance(&cl_out, cl_packet_length(packet));   \
                cl_end(cl, cl_out);                              \
                _loop_terminate = NULL;                          \
        }))                                                      \

#define cl_emit_with_prepacked(cl, packet, prepacked, name)      \
        for (struct cl_packet_struct(packet) name = {            \
                cl_packet_header(packet)                         \
        },                                                       \
        *_loop_terminate = &name;                                \
        __builtin_expect(_loop_terminate != NULL, 1);            \
        ({                                                       \
                struct v3d_cl_out *cl_out = cl_start(cl);        \
                uint8_t packed[cl_packet_length(packet)];         \
                cl_packet_pack(packet)(cl, packed, &name);       \
                for (int _i = 0; _i < cl_packet_length(packet); _i++) \
                        ((uint8_t *)cl_out)[_i] = packed[_i] | (prepacked)[_i]; \
                cl_advance(&cl_out, cl_packet_length(packet));   \
                cl_end(cl, cl_out);                              \
                _loop_terminate = NULL;                          \
        }))                                                      \

#define cl_emit_prepacked_sized(cl, packet, size) do {                \
        memcpy((cl)->next, packet, size);             \
        cl_advance(&(cl)->next, size);                \
} while (0)

#define cl_emit_prepacked(cl, packet) \
        cl_emit_prepacked_sized(cl, packet, sizeof(*(packet)))

#define v3dx_pack(packed, packet, name)                          \
        for (struct cl_packet_struct(packet) name = {            \
                cl_packet_header(packet)                         \
        },                                                       \
        *_loop_terminate = &name;                                \
        __builtin_expect(_loop_terminate != NULL, 1);            \
        ({                                                       \
                cl_packet_pack(packet)(NULL, (uint8_t *)packed, &name); \
                VG(VALGRIND_CHECK_MEM_IS_DEFINED((uint8_t *)packed, \
                                                 cl_packet_length(packet))); \
                _loop_terminate = NULL;                          \
        }))                                                      \

/**
 * Helper function called by the XML-generated pack functions for filling in
 * an address field in shader records.
 *
 * Since we have a private address space as of V3D, our BOs can have lifelong
 * offsets, and all the kernel needs to know is which BOs need to be paged in
 * for this exec.
 */
static inline void
cl_pack_emit_reloc(struct v3d_cl *cl, const struct v3d_cl_reloc *reloc)
{
        if (reloc->bo)
                v3d_job_add_bo(cl->job, reloc->bo);
}

#endif /* V3D_CL_H */
