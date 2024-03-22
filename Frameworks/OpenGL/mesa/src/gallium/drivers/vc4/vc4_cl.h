/*
 * Copyright © 2014 Broadcom
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

#ifndef VC4_CL_H
#define VC4_CL_H

#include <stdint.h>

#include "util/u_math.h"
#include "util/macros.h"

struct vc4_bo;
struct vc4_job;
struct vc4_cl;

/**
 * Undefined structure, used for typechecking that you're passing the pointers
 * to these functions correctly.
 */
struct vc4_cl_out;

/** A reference to a BO used in the CL packing functions */
struct vc4_cl_reloc {
        struct vc4_bo *bo;
        uint32_t offset;
};

static inline void cl_pack_emit_reloc(struct vc4_cl *cl, const struct vc4_cl_reloc *);

#define __gen_user_data struct vc4_cl
#define __gen_address_type struct vc4_cl_reloc
#define __gen_address_offset(reloc) ((reloc)->offset)
#define __gen_emit_reloc cl_pack_emit_reloc

#include "kernel/vc4_packet.h"
#include "broadcom/cle/v3d_packet_v21_pack.h"

struct vc4_cl {
        void *base;
        struct vc4_job *job;
        struct vc4_cl_out *next;
        struct vc4_cl_out *reloc_next;
        uint32_t size;
#ifndef NDEBUG
        uint32_t reloc_count;
#endif
};

void vc4_init_cl(struct vc4_job *job, struct vc4_cl *cl);
void vc4_reset_cl(struct vc4_cl *cl);
uint32_t vc4_gem_hindex(struct vc4_job *job, struct vc4_bo *bo);

struct PACKED unaligned_16 { uint16_t x; };
struct PACKED unaligned_32 { uint32_t x; };

static inline uint32_t cl_offset(struct vc4_cl *cl)
{
        return (char *)cl->next - (char *)cl->base;
}

static inline void
cl_advance(struct vc4_cl_out **cl, uint32_t n)
{
        (*cl) = (struct vc4_cl_out *)((char *)(*cl) + n);
}

static inline struct vc4_cl_out *
cl_start(struct vc4_cl *cl)
{
        return cl->next;
}

static inline void
cl_end(struct vc4_cl *cl, struct vc4_cl_out *next)
{
        cl->next = next;
        assert(cl_offset(cl) <= cl->size);
}


static inline void
put_unaligned_32(struct vc4_cl_out *ptr, uint32_t val)
{
        struct unaligned_32 *p = (void *)ptr;
        p->x = val;
}

static inline void
put_unaligned_16(struct vc4_cl_out *ptr, uint16_t val)
{
        struct unaligned_16 *p = (void *)ptr;
        p->x = val;
}

static inline void
cl_u8(struct vc4_cl_out **cl, uint8_t n)
{
        *(uint8_t *)(*cl) = n;
        cl_advance(cl, 1);
}

static inline void
cl_u16(struct vc4_cl_out **cl, uint16_t n)
{
        put_unaligned_16(*cl, n);
        cl_advance(cl, 2);
}

static inline void
cl_u32(struct vc4_cl_out **cl, uint32_t n)
{
        put_unaligned_32(*cl, n);
        cl_advance(cl, 4);
}

static inline void
cl_aligned_u32(struct vc4_cl_out **cl, uint32_t n)
{
        *(uint32_t *)(*cl) = n;
        cl_advance(cl, 4);
}

static inline void
cl_ptr(struct vc4_cl_out **cl, void *ptr)
{
        *(struct vc4_cl_out **)(*cl) = ptr;
        cl_advance(cl, sizeof(void *));
}

static inline void
cl_f(struct vc4_cl_out **cl, float f)
{
        cl_u32(cl, fui(f));
}

static inline void
cl_aligned_f(struct vc4_cl_out **cl, float f)
{
        cl_aligned_u32(cl, fui(f));
}

static inline struct vc4_cl_out *
cl_start_shader_reloc(struct vc4_cl *cl, uint32_t n)
{
        assert(cl->reloc_count == 0);
#ifndef NDEBUG
        cl->reloc_count = n;
#endif
        cl->reloc_next = cl->next;

        /* Reserve the space where hindex will be written. */
        cl_advance(&cl->next, n * 4);

        return cl->next;
}

static inline void
cl_reloc(struct vc4_job *job, struct vc4_cl *cl, struct vc4_cl_out **cl_out,
         struct vc4_bo *bo, uint32_t offset)
{
        *(uint32_t *)cl->reloc_next = vc4_gem_hindex(job, bo);
        cl_advance(&cl->reloc_next, 4);

#ifndef NDEBUG
        cl->reloc_count--;
#endif

        cl_u32(cl_out, offset);
}

static inline void
cl_aligned_reloc(struct vc4_job *job, struct vc4_cl *cl,
                 struct vc4_cl_out **cl_out,
                 struct vc4_bo *bo, uint32_t offset)
{
        *(uint32_t *)cl->reloc_next = vc4_gem_hindex(job, bo);
        cl_advance(&cl->reloc_next, 4);

#ifndef NDEBUG
        cl->reloc_count--;
#endif

        cl_aligned_u32(cl_out, offset);
}

/**
 * Reference to a BO with its associated offset, used in the pack process.
 */
static inline struct vc4_cl_reloc
cl_address(struct vc4_bo *bo, uint32_t offset)
{
        struct vc4_cl_reloc reloc = {
                .bo = bo,
                .offset = offset,
        };
        return reloc;
}

void cl_ensure_space(struct vc4_cl *cl, uint32_t size);

#define cl_packet_header(packet) V3D21_ ## packet ## _header
#define cl_packet_length(packet) V3D21_ ## packet ## _length
#define cl_packet_pack(packet)   V3D21_ ## packet ## _pack
#define cl_packet_struct(packet)   V3D21_ ## packet

static inline void *
cl_get_emit_space(struct vc4_cl_out **cl, size_t size)
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
 * Also, *dst is actually of the wrong type, it's the
 * uint8_t[cl_packet_length()] in the CL, not a cl_packet_struct(packet).
 */
#define cl_emit(cl, packet, name)                                \
        for (struct cl_packet_struct(packet) name = {            \
                cl_packet_header(packet)                         \
        },                                                       \
        *_loop_terminate = &name;                                \
        __builtin_expect(_loop_terminate != NULL, 1);            \
        ({                                                       \
                struct vc4_cl_out *cl_out = cl_start(cl);        \
                cl_packet_pack(packet)(cl, (uint8_t *)cl_out, &name); \
                VG(VALGRIND_CHECK_MEM_IS_DEFINED(cl_out,         \
                                                 cl_packet_length(packet))); \
                cl_advance(&cl_out, cl_packet_length(packet));   \
                cl_end(cl, cl_out);                              \
                _loop_terminate = NULL;                          \
        }))                                                      \

#define cl_emit_prepacked(cl, packet) do {                       \
        memcpy((cl)->next, packet, sizeof(*packet));             \
        cl_advance(&(cl)->next, sizeof(*packet));                \
} while (0)

/**
 * Helper function called by the XML-generated pack functions for filling in
 * an address field in shader records.
 *
 * Relocations for shader recs and texturing involve the packet (or uniforms
 * stream) being preceded by the handles to the BOs, and the offset within the
 * BO being in the stream (the output of this function).
 */
static inline void
cl_pack_emit_reloc(struct vc4_cl *cl, const struct vc4_cl_reloc *reloc)
{
        *(uint32_t *)cl->reloc_next = vc4_gem_hindex(cl->job, reloc->bo);
        cl_advance(&cl->reloc_next, 4);

#ifndef NDEBUG
        cl->reloc_count--;
#endif
}

#endif /* VC4_CL_H */
