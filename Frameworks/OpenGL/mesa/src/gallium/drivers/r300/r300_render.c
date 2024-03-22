/*
 * Copyright 2009 Corbin Simpson <MostAwesomeDude@gmail.com>
 * Copyright 2010 Marek Olšák <maraeo@gmail.com>
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

/* r300_render: Vertex and index buffer primitive emission. Contains both
 * HW TCL fastpath rendering, and SW TCL Draw-assisted rendering. */

#include "draw/draw_context.h"
#include "draw/draw_vbuf.h"

#include "util/u_inlines.h"

#include "util/format/u_format.h"
#include "util/u_draw.h"
#include "util/u_memory.h"
#include "util/u_upload_mgr.h"
#include "util/u_prim.h"

#include "r300_cs.h"
#include "r300_context.h"
#include "r300_screen_buffer.h"
#include "r300_emit.h"
#include "r300_reg.h"
#include "r300_vs.h"
#include "r300_fs.h"

#include <limits.h>

#define IMMD_DWORDS 32

static uint32_t r300_translate_primitive(unsigned prim)
{
    static const int prim_conv[] = {
        R300_VAP_VF_CNTL__PRIM_POINTS,
        R300_VAP_VF_CNTL__PRIM_LINES,
        R300_VAP_VF_CNTL__PRIM_LINE_LOOP,
        R300_VAP_VF_CNTL__PRIM_LINE_STRIP,
        R300_VAP_VF_CNTL__PRIM_TRIANGLES,
        R300_VAP_VF_CNTL__PRIM_TRIANGLE_STRIP,
        R300_VAP_VF_CNTL__PRIM_TRIANGLE_FAN,
        R300_VAP_VF_CNTL__PRIM_QUADS,
        R300_VAP_VF_CNTL__PRIM_QUAD_STRIP,
        R300_VAP_VF_CNTL__PRIM_POLYGON,
        -1,
        -1,
        -1,
        -1
    };
    unsigned hwprim = prim_conv[prim];

    assert(hwprim != -1);
    return hwprim;
}

static uint32_t r300_provoking_vertex_fixes(struct r300_context *r300,
                                            unsigned mode)
{
    struct r300_rs_state* rs = (struct r300_rs_state*)r300->rs_state.state;
    uint32_t color_control = rs->color_control;

    /* By default (see r300_state.c:r300_create_rs_state) color_control is
     * initialized to provoking the first vertex.
     *
     * Triangle fans must be reduced to the second vertex, not the first, in
     * Gallium flatshade-first mode, as per the GL spec.
     * (http://www.opengl.org/registry/specs/ARB/provoking_vertex.txt)
     *
     * Quads never provoke correctly in flatshade-first mode. The first
     * vertex is never considered as provoking, so only the second, third,
     * and fourth vertices can be selected, and both "third" and "last" modes
     * select the fourth vertex. This is probably due to D3D lacking quads.
     *
     * Similarly, polygons reduce to the first, not the last, vertex, when in
     * "last" mode, and all other modes start from the second vertex.
     *
     * ~ C.
     */

    if (rs->rs.flatshade_first) {
        switch (mode) {
            case MESA_PRIM_TRIANGLE_FAN:
                color_control |= R300_GA_COLOR_CONTROL_PROVOKING_VERTEX_SECOND;
                break;
            case MESA_PRIM_QUADS:
            case MESA_PRIM_QUAD_STRIP:
            case MESA_PRIM_POLYGON:
                color_control |= R300_GA_COLOR_CONTROL_PROVOKING_VERTEX_LAST;
                break;
            default:
                color_control |= R300_GA_COLOR_CONTROL_PROVOKING_VERTEX_FIRST;
                break;
        }
    } else {
        color_control |= R300_GA_COLOR_CONTROL_PROVOKING_VERTEX_LAST;
    }

    return color_control;
}

void r500_emit_index_bias(struct r300_context *r300, int index_bias)
{
    CS_LOCALS(r300);

    BEGIN_CS(2);
    OUT_CS_REG(R500_VAP_INDEX_OFFSET,
               (index_bias & 0xFFFFFF) | (index_bias < 0 ? 1<<24 : 0));
    END_CS;
}

static void r300_emit_draw_init(struct r300_context *r300, unsigned mode,
                                unsigned max_index)
{
    CS_LOCALS(r300);

    assert(max_index < (1 << 24));

    BEGIN_CS(5);
    OUT_CS_REG(R300_GA_COLOR_CONTROL,
            r300_provoking_vertex_fixes(r300, mode));
    OUT_CS_REG_SEQ(R300_VAP_VF_MAX_VTX_INDX, 2);
    OUT_CS(max_index);
    OUT_CS(0);
    END_CS;
}

/* This function splits the index bias value into two parts:
 * - buffer_offset: the value that can be safely added to buffer offsets
 *   in r300_emit_vertex_arrays (it must yield a positive offset when added to
 *   a vertex buffer offset)
 * - index_offset: the value that must be manually subtracted from indices
 *   in an index buffer to achieve negative offsets. */
static void r300_split_index_bias(struct r300_context *r300, int index_bias,
                                  int *buffer_offset, int *index_offset)
{
    struct pipe_vertex_buffer *vb, *vbufs = r300->vertex_buffer;
    struct pipe_vertex_element *velem = r300->velems->velem;
    unsigned i, size;
    int max_neg_bias;

    if (index_bias < 0) {
        /* See how large index bias we may subtract. We must be careful
         * here because negative buffer offsets are not allowed
         * by the DRM API. */
        max_neg_bias = INT_MAX;
        for (i = 0; i < r300->velems->count; i++) {
            vb = &vbufs[velem[i].vertex_buffer_index];
            size = (vb->buffer_offset + velem[i].src_offset) / velem[i].src_stride;
            max_neg_bias = MIN2(max_neg_bias, size);
        }

        /* Now set the minimum allowed value. */
        *buffer_offset = MAX2(-max_neg_bias, index_bias);
    } else {
        /* A positive index bias is OK. */
        *buffer_offset = index_bias;
    }

    *index_offset = index_bias - *buffer_offset;
}

enum r300_prepare_flags {
    PREP_EMIT_STATES    = (1 << 0), /* call emit_dirty_state and friends? */
    PREP_VALIDATE_VBOS  = (1 << 1), /* validate VBOs? */
    PREP_EMIT_VARRAYS       = (1 << 2), /* call emit_vertex_arrays? */
    PREP_EMIT_VARRAYS_SWTCL = (1 << 3), /* call emit_vertex_arrays_swtcl? */
    PREP_INDEXED        = (1 << 4)  /* is this draw_elements? */
};

/**
 * Check if the requested number of dwords is available in the CS and
 * if not, flush.
 * \param r300          The context.
 * \param flags         See r300_prepare_flags.
 * \param cs_dwords     The number of dwords to reserve in CS.
 * \return TRUE if the CS was flushed
 */
static bool r300_reserve_cs_dwords(struct r300_context *r300,
                                   enum r300_prepare_flags flags,
                                   unsigned cs_dwords)
{
    bool flushed        = false;
    bool emit_states    = flags & PREP_EMIT_STATES;
    bool emit_vertex_arrays       = flags & PREP_EMIT_VARRAYS;
    bool emit_vertex_arrays_swtcl = flags & PREP_EMIT_VARRAYS_SWTCL;

    /* Add dirty state, index offset, and AOS. */
    if (emit_states)
        cs_dwords += r300_get_num_dirty_dwords(r300);

    if (r300->screen->caps.is_r500)
        cs_dwords += 2; /* emit_index_offset */

    if (emit_vertex_arrays)
        cs_dwords += 55; /* emit_vertex_arrays */

    if (emit_vertex_arrays_swtcl)
        cs_dwords += 7; /* emit_vertex_arrays_swtcl */

    cs_dwords += r300_get_num_cs_end_dwords(r300);

    /* Reserve requested CS space. */
    if (!r300->rws->cs_check_space(&r300->cs, cs_dwords)) {
        r300_flush(&r300->context, PIPE_FLUSH_ASYNC, NULL);
        flushed = true;
    }

    return flushed;
}

/**
 * Validate buffers and emit dirty state.
 * \param r300          The context.
 * \param flags         See r300_prepare_flags.
 * \param index_buffer  The index buffer to validate. The parameter may be NULL.
 * \param buffer_offset The offset passed to emit_vertex_arrays.
 * \param index_bias    The index bias to emit.
 * \param instance_id   Index of instance to render
 * \return TRUE if rendering should be skipped
 */
static bool r300_emit_states(struct r300_context *r300,
                             enum r300_prepare_flags flags,
                             struct pipe_resource *index_buffer,
                             int buffer_offset,
                             int index_bias, int instance_id)
{
    bool emit_states    = flags & PREP_EMIT_STATES;
    bool emit_vertex_arrays       = flags & PREP_EMIT_VARRAYS;
    bool emit_vertex_arrays_swtcl = flags & PREP_EMIT_VARRAYS_SWTCL;
    bool indexed        = flags & PREP_INDEXED;
    bool validate_vbos  = flags & PREP_VALIDATE_VBOS;

    /* Validate buffers and emit dirty state if needed. */
    if (emit_states || (emit_vertex_arrays && validate_vbos)) {
        if (!r300_emit_buffer_validate(r300, validate_vbos,
                                       index_buffer)) {
           fprintf(stderr, "r300: CS space validation failed. "
                   "(not enough memory?) Skipping rendering.\n");
           return false;
        }
    }

    if (emit_states)
        r300_emit_dirty_state(r300);

    if (r300->screen->caps.is_r500) {
        if (r300->screen->caps.has_tcl)
            r500_emit_index_bias(r300, index_bias);
        else
            r500_emit_index_bias(r300, 0);
    }

    if (emit_vertex_arrays &&
        (r300->vertex_arrays_dirty ||
         r300->vertex_arrays_indexed != indexed ||
         r300->vertex_arrays_offset != buffer_offset ||
         r300->vertex_arrays_instance_id != instance_id)) {
        r300_emit_vertex_arrays(r300, buffer_offset, indexed, instance_id);

        r300->vertex_arrays_dirty = false;
        r300->vertex_arrays_indexed = indexed;
        r300->vertex_arrays_offset = buffer_offset;
        r300->vertex_arrays_instance_id = instance_id;
    }

    if (emit_vertex_arrays_swtcl)
        r300_emit_vertex_arrays_swtcl(r300, indexed);

    return true;
}

/**
 * Check if the requested number of dwords is available in the CS and
 * if not, flush. Then validate buffers and emit dirty state.
 * \param r300          The context.
 * \param flags         See r300_prepare_flags.
 * \param index_buffer  The index buffer to validate. The parameter may be NULL.
 * \param cs_dwords     The number of dwords to reserve in CS.
 * \param buffer_offset The offset passed to emit_vertex_arrays.
 * \param index_bias    The index bias to emit.
 * \param instance_id The instance to render.
 * \return TRUE if rendering should be skipped
 */
static bool r300_prepare_for_rendering(struct r300_context *r300,
                                       enum r300_prepare_flags flags,
                                       struct pipe_resource *index_buffer,
                                       unsigned cs_dwords,
                                       int buffer_offset,
                                       int index_bias,
                                       int instance_id)
{
    /* Make sure there is enough space in the command stream and emit states. */
    if (r300_reserve_cs_dwords(r300, flags, cs_dwords))
        flags |= PREP_EMIT_STATES;

    return r300_emit_states(r300, flags, index_buffer, buffer_offset,
                            index_bias, instance_id);
}

static bool immd_is_good_idea(struct r300_context *r300,
                              unsigned count)
{
    if (DBG_ON(r300, DBG_NO_IMMD)) {
        return false;
    }

    if (count * r300->velems->vertex_size_dwords > IMMD_DWORDS) {
        return false;
    }

    /* Buffers can only be used for read by r300 (except query buffers, but
     * those can't be bound by an gallium frontend as vertex buffers). */
    return true;
}

/*****************************************************************************
 * The HWTCL draw functions.                                                 *
 ****************************************************************************/

static void r300_draw_arrays_immediate(struct r300_context *r300,
                                       const struct pipe_draw_info *info,
                                       const struct pipe_draw_start_count_bias *draw)
{
    struct pipe_vertex_element* velem;
    struct pipe_vertex_buffer* vbuf;
    unsigned vertex_element_count = r300->velems->count;
    unsigned i, v, vbi;

    /* Size of the vertex, in dwords. */
    unsigned vertex_size = r300->velems->vertex_size_dwords;

    /* The number of dwords for this draw operation. */
    unsigned dwords = 4 + draw->count * vertex_size;

    /* Size of the vertex element, in dwords. */
    unsigned size[PIPE_MAX_ATTRIBS];

    /* Stride to the same attrib in the next vertex in the vertex buffer,
     * in dwords. */
    unsigned stride[PIPE_MAX_ATTRIBS];

    /* Mapped vertex buffers. */
    uint32_t* map[PIPE_MAX_ATTRIBS] = {0};
    uint32_t* mapelem[PIPE_MAX_ATTRIBS];

    CS_LOCALS(r300);

    if (!r300_prepare_for_rendering(r300, PREP_EMIT_STATES, NULL, dwords, 0, 0, -1))
        return;

    /* Calculate the vertex size, offsets, strides etc. and map the buffers. */
    for (i = 0; i < vertex_element_count; i++) {
        velem = &r300->velems->velem[i];
        size[i] = r300->velems->format_size[i] / 4;
        vbi = velem->vertex_buffer_index;
        vbuf = &r300->vertex_buffer[vbi];
        stride[i] = velem->src_stride / 4;

        /* Map the buffer. */
        if (!map[vbi]) {
            map[vbi] = (uint32_t*)r300->rws->buffer_map(r300->rws,
                r300_resource(vbuf->buffer.resource)->buf,
                &r300->cs, PIPE_MAP_READ | PIPE_MAP_UNSYNCHRONIZED);
            map[vbi] += (vbuf->buffer_offset / 4) + stride[i] * draw->start;
        }
        mapelem[i] = map[vbi] + (velem->src_offset / 4);
    }

    r300_emit_draw_init(r300, info->mode, draw->count-1);

    BEGIN_CS(dwords);
    OUT_CS_REG(R300_VAP_VTX_SIZE, vertex_size);
    OUT_CS_PKT3(R300_PACKET3_3D_DRAW_IMMD_2, draw->count * vertex_size);
    OUT_CS(R300_VAP_VF_CNTL__PRIM_WALK_VERTEX_EMBEDDED | (draw->count << 16) |
            r300_translate_primitive(info->mode));

    /* Emit vertices. */
    for (v = 0; v < draw->count; v++) {
        for (i = 0; i < vertex_element_count; i++) {
            OUT_CS_TABLE(&mapelem[i][stride[i] * v], size[i]);
        }
    }
    END_CS;
}

static void r300_emit_draw_arrays(struct r300_context *r300,
                                  unsigned mode,
                                  unsigned count)
{
    bool alt_num_verts = count > 65535;
    CS_LOCALS(r300);

    if (count >= (1 << 24)) {
        fprintf(stderr, "r300: Got a huge number of vertices: %i, "
                "refusing to render.\n", count);
        return;
    }

    r300_emit_draw_init(r300, mode, count-1);

    BEGIN_CS(2 + (alt_num_verts ? 2 : 0));
    if (alt_num_verts) {
        OUT_CS_REG(R500_VAP_ALT_NUM_VERTICES, count);
    }
    OUT_CS_PKT3(R300_PACKET3_3D_DRAW_VBUF_2, 0);
    OUT_CS(R300_VAP_VF_CNTL__PRIM_WALK_VERTEX_LIST | (count << 16) |
           r300_translate_primitive(mode) |
           (alt_num_verts ? R500_VAP_VF_CNTL__USE_ALT_NUM_VERTS : 0));
    END_CS;
}

static void r300_emit_draw_elements(struct r300_context *r300,
                                    struct pipe_resource* indexBuffer,
                                    unsigned indexSize,
                                    unsigned max_index,
                                    unsigned mode,
                                    unsigned start,
                                    unsigned count,
                                    uint16_t *imm_indices3)
{
    uint32_t count_dwords, offset_dwords;
    bool alt_num_verts = count > 65535;
    CS_LOCALS(r300);

    if (count >= (1 << 24)) {
        fprintf(stderr, "r300: Got a huge number of vertices: %i, "
                "refusing to render (max_index: %i).\n", count, max_index);
        return;
    }

    DBG(r300, DBG_DRAW, "r300: Indexbuf of %u indices, max %u\n",
        count, max_index);

    r300_emit_draw_init(r300, mode, max_index);

    /* If start is odd, render the first triangle with indices embedded
     * in the command stream. This will increase start by 3 and make it
     * even. We can then proceed without a fallback. */
    if (indexSize == 2 && (start & 1) &&
        mode == MESA_PRIM_TRIANGLES) {
        BEGIN_CS(4);
        OUT_CS_PKT3(R300_PACKET3_3D_DRAW_INDX_2, 2);
        OUT_CS(R300_VAP_VF_CNTL__PRIM_WALK_INDICES | (3 << 16) |
               R300_VAP_VF_CNTL__PRIM_TRIANGLES);
        OUT_CS(imm_indices3[1] << 16 | imm_indices3[0]);
        OUT_CS(imm_indices3[2]);
        END_CS;

        start += 3;
        count -= 3;
        if (!count)
           return;
    }

    offset_dwords = indexSize * start / sizeof(uint32_t);

    BEGIN_CS(8 + (alt_num_verts ? 2 : 0));
    if (alt_num_verts) {
        OUT_CS_REG(R500_VAP_ALT_NUM_VERTICES, count);
    }
    OUT_CS_PKT3(R300_PACKET3_3D_DRAW_INDX_2, 0);
    if (indexSize == 4) {
        count_dwords = count;
        OUT_CS(R300_VAP_VF_CNTL__PRIM_WALK_INDICES | (count << 16) |
               R300_VAP_VF_CNTL__INDEX_SIZE_32bit |
               r300_translate_primitive(mode) |
               (alt_num_verts ? R500_VAP_VF_CNTL__USE_ALT_NUM_VERTS : 0));
    } else {
        count_dwords = (count + 1) / 2;
        OUT_CS(R300_VAP_VF_CNTL__PRIM_WALK_INDICES | (count << 16) |
               r300_translate_primitive(mode) |
               (alt_num_verts ? R500_VAP_VF_CNTL__USE_ALT_NUM_VERTS : 0));
    }

    OUT_CS_PKT3(R300_PACKET3_INDX_BUFFER, 2);
    OUT_CS(R300_INDX_BUFFER_ONE_REG_WR | (R300_VAP_PORT_IDX0 >> 2) |
           (0 << R300_INDX_BUFFER_SKIP_SHIFT));
    OUT_CS(offset_dwords << 2);
    OUT_CS(count_dwords);
    OUT_CS_RELOC(r300_resource(indexBuffer));
    END_CS;
}

static void r300_draw_elements_immediate(struct r300_context *r300,
                                         const struct pipe_draw_info *info,
                                         const struct pipe_draw_start_count_bias *draw)
{
    const uint8_t *ptr1;
    const uint16_t *ptr2;
    const uint32_t *ptr4;
    unsigned index_size = info->index_size;
    unsigned i, count_dwords = index_size == 4 ? draw->count :
                                                 (draw->count + 1) / 2;
    CS_LOCALS(r300);

    /* 19 dwords for r300_draw_elements_immediate. Give up if the function fails. */
    if (!r300_prepare_for_rendering(r300,
            PREP_EMIT_STATES | PREP_VALIDATE_VBOS | PREP_EMIT_VARRAYS |
            PREP_INDEXED, NULL, 2+count_dwords, 0, draw->index_bias, -1))
        return;

    r300_emit_draw_init(r300, info->mode, info->max_index);

    BEGIN_CS(2 + count_dwords);
    OUT_CS_PKT3(R300_PACKET3_3D_DRAW_INDX_2, count_dwords);

    switch (index_size) {
    case 1:
        ptr1 = (uint8_t*)info->index.user;
        ptr1 += draw->start;

        OUT_CS(R300_VAP_VF_CNTL__PRIM_WALK_INDICES | (draw->count << 16) |
               r300_translate_primitive(info->mode));

        if (draw->index_bias && !r300->screen->caps.is_r500) {
            for (i = 0; i < draw->count-1; i += 2)
                OUT_CS(((ptr1[i+1] + draw->index_bias) << 16) |
                        (ptr1[i]   + draw->index_bias));

            if (draw->count & 1)
                OUT_CS(ptr1[i] + draw->index_bias);
        } else {
            for (i = 0; i < draw->count-1; i += 2)
                OUT_CS(((ptr1[i+1]) << 16) |
                        (ptr1[i]  ));

            if (draw->count & 1)
                OUT_CS(ptr1[i]);
        }
        break;

    case 2:
        ptr2 = (uint16_t*)info->index.user;
        ptr2 += draw->start;

        OUT_CS(R300_VAP_VF_CNTL__PRIM_WALK_INDICES | (draw->count << 16) |
               r300_translate_primitive(info->mode));

        if (draw->index_bias && !r300->screen->caps.is_r500) {
            for (i = 0; i < draw->count-1; i += 2)
                OUT_CS(((ptr2[i+1] + draw->index_bias) << 16) |
                        (ptr2[i]   + draw->index_bias));

            if (draw->count & 1)
                OUT_CS(ptr2[i] + draw->index_bias);
        } else {
            OUT_CS_TABLE(ptr2, count_dwords);
        }
        break;

    case 4:
        ptr4 = (uint32_t*)info->index.user;
        ptr4 += draw->start;

        OUT_CS(R300_VAP_VF_CNTL__PRIM_WALK_INDICES | (draw->count << 16) |
               R300_VAP_VF_CNTL__INDEX_SIZE_32bit |
               r300_translate_primitive(info->mode));

        if (draw->index_bias && !r300->screen->caps.is_r500) {
            for (i = 0; i < draw->count; i++)
                OUT_CS(ptr4[i] + draw->index_bias);
        } else {
            OUT_CS_TABLE(ptr4, count_dwords);
        }
        break;
    }
    END_CS;
}

static void r300_draw_elements(struct r300_context *r300,
                               const struct pipe_draw_info *info,
                               const struct pipe_draw_start_count_bias *draw,
                               int instance_id)
{
    struct pipe_resource *indexBuffer =
       info->has_user_indices ? NULL : info->index.resource;
    unsigned indexSize = info->index_size;
    struct pipe_resource* orgIndexBuffer = indexBuffer;
    unsigned start = draw->start;
    unsigned count = draw->count;
    bool alt_num_verts = r300->screen->caps.is_r500 &&
                            count > 65536;
    unsigned short_count;
    int buffer_offset = 0, index_offset = 0; /* for index bias emulation */
    uint16_t indices3[3];

    if (draw->index_bias && !r300->screen->caps.is_r500) {
        r300_split_index_bias(r300, draw->index_bias, &buffer_offset,
                              &index_offset);
    }

    r300_translate_index_buffer(r300, info, &indexBuffer,
                                &indexSize, index_offset, &start, count);

    /* Fallback for misaligned ushort indices. */
    if (indexSize == 2 && (start & 1) && indexBuffer) {
        /* If we got here, then orgIndexBuffer == indexBuffer. */
        uint16_t *ptr = r300->rws->buffer_map(r300->rws, r300_resource(orgIndexBuffer)->buf,
                                              &r300->cs,
                                              PIPE_MAP_READ |
                                              PIPE_MAP_UNSYNCHRONIZED);

        if (info->mode == MESA_PRIM_TRIANGLES) {
           memcpy(indices3, ptr + start, 6);
        } else {
            /* Copy the mapped index buffer directly to the upload buffer.
             * The start index will be aligned simply from the fact that
             * every sub-buffer in the upload buffer is aligned. */
            r300_upload_index_buffer(r300, &indexBuffer, indexSize, &start,
                                     count, (uint8_t*)ptr);
        }
    } else {
        if (info->has_user_indices)
            r300_upload_index_buffer(r300, &indexBuffer, indexSize,
                                     &start, count,
                                     info->index.user);
    }

    /* 19 dwords for emit_draw_elements. Give up if the function fails. */
    if (!r300_prepare_for_rendering(r300,
            PREP_EMIT_STATES | PREP_VALIDATE_VBOS | PREP_EMIT_VARRAYS |
            PREP_INDEXED, indexBuffer, 19, buffer_offset, draw->index_bias,
            instance_id))
        goto done;

    if (alt_num_verts || count <= 65535) {
        r300_emit_draw_elements(r300, indexBuffer, indexSize,
                                info->max_index, info->mode, start, count,
                                indices3);
    } else {
        do {
            /* The maximum must be divisible by 4 and 3,
             * so that quad and triangle lists are split correctly.
             *
             * Strips, loops, and fans won't work. */
            short_count = MIN2(count, 65532);

            r300_emit_draw_elements(r300, indexBuffer, indexSize,
                                     info->max_index,
                                     info->mode, start, short_count, indices3);

            start += short_count;
            count -= short_count;

            /* 15 dwords for emit_draw_elements */
            if (count) {
                if (!r300_prepare_for_rendering(r300,
                        PREP_VALIDATE_VBOS | PREP_EMIT_VARRAYS | PREP_INDEXED,
                        indexBuffer, 19, buffer_offset, draw->index_bias,
                        instance_id))
                    goto done;
            }
        } while (count);
    }

done:
    if (indexBuffer != orgIndexBuffer) {
        pipe_resource_reference( &indexBuffer, NULL );
    }
}

static void r300_draw_arrays(struct r300_context *r300,
                             const struct pipe_draw_info *info,
                             const struct pipe_draw_start_count_bias *draw,
                             int instance_id)
{
    bool alt_num_verts = r300->screen->caps.is_r500 &&
                            draw->count > 65536;
    unsigned start = draw->start;
    unsigned count = draw->count;
    unsigned short_count;

    /* 9 spare dwords for emit_draw_arrays. Give up if the function fails. */
    if (!r300_prepare_for_rendering(r300,
                                    PREP_EMIT_STATES | PREP_VALIDATE_VBOS | PREP_EMIT_VARRAYS,
                                    NULL, 9, start, 0, instance_id))
        return;

    if (alt_num_verts || count <= 65535) {
        r300_emit_draw_arrays(r300, info->mode, count);
    } else {
        do {
            /* The maximum must be divisible by 4 and 3,
             * so that quad and triangle lists are split correctly.
             *
             * Strips, loops, and fans won't work. */
            short_count = MIN2(count, 65532);
            r300_emit_draw_arrays(r300, info->mode, short_count);

            start += short_count;
            count -= short_count;

            /* 9 spare dwords for emit_draw_arrays. Give up if the function fails. */
            if (count) {
                if (!r300_prepare_for_rendering(r300,
                                                PREP_VALIDATE_VBOS | PREP_EMIT_VARRAYS, NULL, 9,
                                                start, 0, instance_id))
                    return;
            }
        } while (count);
    }
}

static void r300_draw_arrays_instanced(struct r300_context *r300,
                                       const struct pipe_draw_info *info,
                                       const struct pipe_draw_start_count_bias *draw)
{
    int i;

    for (i = 0; i < info->instance_count; i++)
        r300_draw_arrays(r300, info, draw, i);
}

static void r300_draw_elements_instanced(struct r300_context *r300,
                                         const struct pipe_draw_info *info,
                                         const struct pipe_draw_start_count_bias *draw)
{
    int i;

    for (i = 0; i < info->instance_count; i++)
        r300_draw_elements(r300, info, draw, i);
}

static unsigned r300_max_vertex_count(struct r300_context *r300)
{
   unsigned i, nr = r300->velems->count;
   struct pipe_vertex_element *velems = r300->velems->velem;
   unsigned result = ~0;

   for (i = 0; i < nr; i++) {
      struct pipe_vertex_buffer *vb =
            &r300->vertex_buffer[velems[i].vertex_buffer_index];
      unsigned size, max_count, value;

      /* We're not interested in constant and per-instance attribs. */
      if (!vb->buffer.resource ||
          !velems[i].src_stride ||
          velems[i].instance_divisor) {
         continue;
      }

      size = vb->buffer.resource->width0;

      /* Subtract buffer_offset. */
      value = vb->buffer_offset;
      if (value >= size) {
         return 0;
      }
      size -= value;

      /* Subtract src_offset. */
      value = velems[i].src_offset;
      if (value >= size) {
         return 0;
      }
      size -= value;

      /* Compute the max count. */
      max_count = 1 + size / velems[i].src_stride;
      result = MIN2(result, max_count);
   }
   return result;
}


static void r300_draw_vbo(struct pipe_context* pipe,
                          const struct pipe_draw_info *dinfo,
                          unsigned drawid_offset,
                          const struct pipe_draw_indirect_info *indirect,
                          const struct pipe_draw_start_count_bias *draws,
                          unsigned num_draws)
{
   if (num_draws > 1) {
      util_draw_multi(pipe, dinfo, drawid_offset, indirect, draws, num_draws);
      return;
   }

    struct r300_context* r300 = r300_context(pipe);
    struct pipe_draw_info info = *dinfo;
    struct pipe_draw_start_count_bias draw = draws[0];

    if (r300->skip_rendering ||
        !u_trim_pipe_prim(info.mode, &draw.count)) {
        return;
    }

    if (r300->sprite_coord_enable != 0 ||
        r300_fs(r300)->shader->inputs.pcoord != ATTR_UNUSED) {
        if ((info.mode == MESA_PRIM_POINTS) != r300->is_point) {
            r300->is_point = !r300->is_point;
            r300_mark_atom_dirty(r300, &r300->rs_block_state);
        }
    }

    r300_update_derived_state(r300);

    /* Skip draw if we failed to compile the vertex shader. */
    if (r300_vs(r300)->shader->dummy)
        return;

    /* Draw. */
    if (info.index_size) {
        unsigned max_count = r300_max_vertex_count(r300);

        if (!max_count) {
           fprintf(stderr, "r300: Skipping a draw command. There is a buffer "
                   " which is too small to be used for rendering.\n");
           return;
        }

        if (max_count == ~0) {
           /* There are no per-vertex vertex elements. Use the hardware maximum. */
           max_count = 0xffffff;
        }

        info.max_index = max_count - 1;

        if (info.instance_count <= 1) {
            if (draw.count <= 8 && info.has_user_indices) {
                r300_draw_elements_immediate(r300, &info, &draw);
            } else {
                r300_draw_elements(r300, &info, &draw, -1);
            }
        } else {
            r300_draw_elements_instanced(r300, &info, &draw);
        }
    } else {
        if (info.instance_count <= 1) {
            if (immd_is_good_idea(r300, draw.count)) {
                r300_draw_arrays_immediate(r300, &info, &draw);
            } else {
                r300_draw_arrays(r300, &info, &draw, -1);
            }
        } else {
            r300_draw_arrays_instanced(r300, &info, &draw);
        }
    }
}

/****************************************************************************
 * The rest of this file is for SW TCL rendering only. Please be polite and *
 * keep these functions separated so that they are easier to locate. ~C.    *
 ***************************************************************************/

/* SW TCL elements, using Draw. */
static void r300_swtcl_draw_vbo(struct pipe_context* pipe,
                                const struct pipe_draw_info *info,
                                unsigned drawid_offset,
                                const struct pipe_draw_indirect_info *indirect,
                                const struct pipe_draw_start_count_bias *draws,
                                unsigned num_draws)
{
   if (num_draws > 1) {
      util_draw_multi(pipe, info, drawid_offset, indirect, draws, num_draws);
      return;
   }

    struct r300_context* r300 = r300_context(pipe);
    struct pipe_draw_start_count_bias draw = draws[0];

    if (r300->skip_rendering) {
        return;
    }

    if (!u_trim_pipe_prim(info->mode, &draw.count))
       return;

    if (info->index_size) {
        draw_set_indexes(r300->draw,
                         info->has_user_indices ?
                             info->index.user :
                             r300_resource(info->index.resource)->malloced_buffer,
                         info->index_size, ~0);
    }

    if (r300->sprite_coord_enable != 0 ||
        r300_fs(r300)->shader->inputs.pcoord != ATTR_UNUSED) {
        if ((info->mode == MESA_PRIM_POINTS) != r300->is_point) {
            r300->is_point = !r300->is_point;
            r300_mark_atom_dirty(r300, &r300->rs_block_state);
        }
    }


    r300_update_derived_state(r300);

    draw_vbo(r300->draw, info, drawid_offset, NULL, &draw, 1, 0);
    draw_flush(r300->draw);
}

/* Object for rendering using Draw. */
struct r300_render {
    /* Parent class */
    struct vbuf_render base;

    /* Pipe context */
    struct r300_context* r300;

    /* Vertex information */
    size_t vertex_size;
    unsigned prim;
    unsigned hwprim;

    /* VBO */
    size_t vbo_max_used;
    uint8_t *vbo_ptr;
};

static inline struct r300_render*
r300_render(struct vbuf_render* render)
{
    return (struct r300_render*)render;
}

static const struct vertex_info*
r300_render_get_vertex_info(struct vbuf_render* render)
{
    struct r300_render* r300render = r300_render(render);
    struct r300_context* r300 = r300render->r300;

    return &r300->vertex_info;
}

static bool r300_render_allocate_vertices(struct vbuf_render* render,
                                          uint16_t vertex_size,
                                          uint16_t count)
{
    struct r300_render* r300render = r300_render(render);
    struct r300_context* r300 = r300render->r300;
    struct radeon_winsys *rws = r300->rws;
    size_t size = (size_t)vertex_size * (size_t)count;

    DBG(r300, DBG_DRAW, "r300: render_allocate_vertices (size: %d)\n", size);

    if (!r300->vbo || size + r300->draw_vbo_offset > r300->vbo->size) {
	radeon_bo_reference(r300->rws, &r300->vbo, NULL);
        r300->vbo = NULL;
        r300render->vbo_ptr = NULL;

        r300->vbo = rws->buffer_create(rws,
                                       MAX2(R300_MAX_DRAW_VBO_SIZE, size),
                                       R300_BUFFER_ALIGNMENT,
                                       RADEON_DOMAIN_GTT,
                                       RADEON_FLAG_NO_INTERPROCESS_SHARING);
        if (!r300->vbo) {
            return false;
        }
        r300->draw_vbo_offset = 0;
        r300render->vbo_ptr = rws->buffer_map(rws, r300->vbo, &r300->cs,
                                              PIPE_MAP_WRITE);
    }

    r300render->vertex_size = vertex_size;
    return true;
}

static void* r300_render_map_vertices(struct vbuf_render* render)
{
    struct r300_render* r300render = r300_render(render);
    struct r300_context* r300 = r300render->r300;

    DBG(r300, DBG_DRAW, "r300: render_map_vertices\n");

    assert(r300render->vbo_ptr);
    return r300render->vbo_ptr + r300->draw_vbo_offset;
}

static void r300_render_unmap_vertices(struct vbuf_render* render,
                                       uint16_t min,
                                       uint16_t max)
{
    struct r300_render* r300render = r300_render(render);
    struct r300_context* r300 = r300render->r300;

    DBG(r300, DBG_DRAW, "r300: render_unmap_vertices\n");

    r300render->vbo_max_used = MAX2(r300render->vbo_max_used,
                                    r300render->vertex_size * (max + 1));
}

static void r300_render_release_vertices(struct vbuf_render* render)
{
    struct r300_render* r300render = r300_render(render);
    struct r300_context* r300 = r300render->r300;

    DBG(r300, DBG_DRAW, "r300: render_release_vertices\n");

    r300->draw_vbo_offset += r300render->vbo_max_used;
    r300render->vbo_max_used = 0;
}

static void r300_render_set_primitive(struct vbuf_render* render,
                                      enum mesa_prim prim)
{
    struct r300_render* r300render = r300_render(render);

    r300render->prim = prim;
    r300render->hwprim = r300_translate_primitive(prim);
}

static void r300_render_draw_arrays(struct vbuf_render* render,
                                    unsigned start,
                                    unsigned count)
{
    struct r300_render* r300render = r300_render(render);
    struct r300_context* r300 = r300render->r300;
    uint8_t* ptr;
    unsigned i;
    unsigned dwords = 6;

    CS_LOCALS(r300);
    (void) i; (void) ptr;

    assert(start == 0);
    assert(count < (1 << 16));

    DBG(r300, DBG_DRAW, "r300: render_draw_arrays (count: %d)\n", count);

    if (!r300_prepare_for_rendering(r300,
                                    PREP_EMIT_STATES | PREP_EMIT_VARRAYS_SWTCL,
                                    NULL, dwords, 0, 0, -1)) {
        return;
    }

    BEGIN_CS(dwords);
    OUT_CS_REG(R300_GA_COLOR_CONTROL,
            r300_provoking_vertex_fixes(r300, r300render->prim));
    OUT_CS_REG(R300_VAP_VF_MAX_VTX_INDX, count - 1);
    OUT_CS_PKT3(R300_PACKET3_3D_DRAW_VBUF_2, 0);
    OUT_CS(R300_VAP_VF_CNTL__PRIM_WALK_VERTEX_LIST | (count << 16) |
           r300render->hwprim);
    END_CS;
}

static void r300_render_draw_elements(struct vbuf_render* render,
                                      const uint16_t* indices,
                                      uint count)
{
    struct r300_render* r300render = r300_render(render);
    struct r300_context* r300 = r300render->r300;
    unsigned max_index = (r300->vbo->size - r300->draw_vbo_offset) /
                         (r300render->r300->vertex_info.size * 4) - 1;
    struct pipe_resource *index_buffer = NULL;
    unsigned index_buffer_offset;

    CS_LOCALS(r300);
    DBG(r300, DBG_DRAW, "r300: render_draw_elements (count: %d)\n", count);

    u_upload_data(r300->uploader, 0, count * 2, 4, indices,
                  &index_buffer_offset, &index_buffer);
    if (!index_buffer) {
        return;
    }

    if (!r300_prepare_for_rendering(r300,
                                    PREP_EMIT_STATES |
                                    PREP_EMIT_VARRAYS_SWTCL | PREP_INDEXED,
                                    index_buffer, 12, 0, 0, -1)) {
        pipe_resource_reference(&index_buffer, NULL);
        return;
    }

    BEGIN_CS(12);
    OUT_CS_REG(R300_GA_COLOR_CONTROL,
               r300_provoking_vertex_fixes(r300, r300render->prim));
    OUT_CS_REG(R300_VAP_VF_MAX_VTX_INDX, max_index);

    OUT_CS_PKT3(R300_PACKET3_3D_DRAW_INDX_2, 0);
    OUT_CS(R300_VAP_VF_CNTL__PRIM_WALK_INDICES | (count << 16) |
           r300render->hwprim);

    OUT_CS_PKT3(R300_PACKET3_INDX_BUFFER, 2);
    OUT_CS(R300_INDX_BUFFER_ONE_REG_WR | (R300_VAP_PORT_IDX0 >> 2));
    OUT_CS(index_buffer_offset);
    OUT_CS((count + 1) / 2);
    OUT_CS_RELOC(r300_resource(index_buffer));
    END_CS;

    pipe_resource_reference(&index_buffer, NULL);
}

static void r300_render_destroy(struct vbuf_render* render)
{
    FREE(render);
}

static struct vbuf_render* r300_render_create(struct r300_context* r300)
{
    struct r300_render* r300render = CALLOC_STRUCT(r300_render);

    r300render->r300 = r300;

    r300render->base.max_vertex_buffer_bytes = R300_MAX_DRAW_VBO_SIZE;
    r300render->base.max_indices = 16 * 1024;

    r300render->base.get_vertex_info = r300_render_get_vertex_info;
    r300render->base.allocate_vertices = r300_render_allocate_vertices;
    r300render->base.map_vertices = r300_render_map_vertices;
    r300render->base.unmap_vertices = r300_render_unmap_vertices;
    r300render->base.set_primitive = r300_render_set_primitive;
    r300render->base.draw_elements = r300_render_draw_elements;
    r300render->base.draw_arrays = r300_render_draw_arrays;
    r300render->base.release_vertices = r300_render_release_vertices;
    r300render->base.destroy = r300_render_destroy;

    return &r300render->base;
}

struct draw_stage* r300_draw_stage(struct r300_context* r300)
{
    struct vbuf_render* render;
    struct draw_stage* stage;

    render = r300_render_create(r300);

    if (!render) {
        return NULL;
    }

    stage = draw_vbuf_stage(r300->draw, render);

    if (!stage) {
        render->destroy(render);
        return NULL;
    }

    draw_set_render(r300->draw, render);

    return stage;
}

/****************************************************************************
 *                         End of SW TCL functions                          *
 ***************************************************************************/

/* This functions is used to draw a rectangle for the blitter module.
 *
 * If we rendered a quad, the pixels on the main diagonal
 * would be computed and stored twice, which makes the clear/copy codepaths
 * somewhat inefficient. Instead we use a rectangular point sprite. */
void r300_blitter_draw_rectangle(struct blitter_context *blitter,
                                 void *vertex_elements_cso,
                                 blitter_get_vs_func get_vs,
                                 int x1, int y1, int x2, int y2,
                                 float depth, unsigned num_instances,
                                 enum blitter_attrib_type type,
                                 const union blitter_attrib *attrib)
{
    struct r300_context *r300 = r300_context(util_blitter_get_pipe(blitter));
    unsigned last_sprite_coord_enable = r300->sprite_coord_enable;
    unsigned last_is_point = r300->is_point;
    unsigned width = x2 - x1;
    unsigned height = y2 - y1;
    unsigned vertex_size =
            type == UTIL_BLITTER_ATTRIB_COLOR || !r300->draw ? 8 : 4;
    unsigned dwords = 13 + vertex_size +
                      (type == UTIL_BLITTER_ATTRIB_TEXCOORD_XY ? 7 : 0);
    static const union blitter_attrib zeros;
    CS_LOCALS(r300);

    /* XXX workaround for a lockup in MSAA resolve on SWTCL chipsets, this
     * function most probably doesn't handle type=NONE correctly */
    if ((!r300->screen->caps.has_tcl && type == UTIL_BLITTER_ATTRIB_NONE) ||
        type == UTIL_BLITTER_ATTRIB_TEXCOORD_XYZW ||
        num_instances > 1) {
        util_blitter_draw_rectangle(blitter, vertex_elements_cso, get_vs,
                                    x1, y1, x2, y2,
                                    depth, num_instances, type, attrib);
        return;
    }

    if (r300->skip_rendering)
        return;

    r300->context.bind_vertex_elements_state(&r300->context, vertex_elements_cso);
    r300->context.bind_vs_state(&r300->context, get_vs(blitter));

    if (type == UTIL_BLITTER_ATTRIB_TEXCOORD_XY) {
        r300->sprite_coord_enable = 1;
        r300->is_point = true;
    }

    r300_update_derived_state(r300);

    /* Mark some states we don't care about as non-dirty. */
    r300->viewport_state.dirty = false;

    if (!r300_prepare_for_rendering(r300, PREP_EMIT_STATES, NULL, dwords, 0, 0, -1))
        goto done;

    DBG(r300, DBG_DRAW, "r300: draw_rectangle\n");

    BEGIN_CS(dwords);
    /* Set up GA. */
    OUT_CS_REG(R300_GA_POINT_SIZE, (height * 6) | ((width * 6) << 16));

    if (type == UTIL_BLITTER_ATTRIB_TEXCOORD_XY) {
        /* Set up the GA to generate texcoords. */
        OUT_CS_REG(R300_GB_ENABLE, R300_GB_POINT_STUFF_ENABLE |
                   (R300_GB_TEX_STR << R300_GB_TEX0_SOURCE_SHIFT));
        OUT_CS_REG_SEQ(R300_GA_POINT_S0, 4);
        OUT_CS_32F(attrib->texcoord.x1);
        OUT_CS_32F(attrib->texcoord.y2);
        OUT_CS_32F(attrib->texcoord.x2);
        OUT_CS_32F(attrib->texcoord.y1);
    }

    /* Set up VAP controls. */
    OUT_CS_REG(R300_VAP_CLIP_CNTL, R300_CLIP_DISABLE);
    OUT_CS_REG(R300_VAP_VTE_CNTL, R300_VTX_XY_FMT | R300_VTX_Z_FMT);
    OUT_CS_REG(R300_VAP_VTX_SIZE, vertex_size);
    OUT_CS_REG_SEQ(R300_VAP_VF_MAX_VTX_INDX, 2);
    OUT_CS(1);
    OUT_CS(0);

    /* Draw. */
    OUT_CS_PKT3(R300_PACKET3_3D_DRAW_IMMD_2, vertex_size);
    OUT_CS(R300_VAP_VF_CNTL__PRIM_WALK_VERTEX_EMBEDDED | (1 << 16) |
           R300_VAP_VF_CNTL__PRIM_POINTS);

    OUT_CS_32F(x1 + width * 0.5f);
    OUT_CS_32F(y1 + height * 0.5f);
    OUT_CS_32F(depth);
    OUT_CS_32F(1);

    if (vertex_size == 8) {
        if (!attrib)
            attrib = &zeros;
        OUT_CS_TABLE(attrib->color, 4);
    }
    END_CS;

done:
    /* Restore the state. */
    r300_mark_atom_dirty(r300, &r300->rs_state);
    r300_mark_atom_dirty(r300, &r300->viewport_state);

    r300->sprite_coord_enable = last_sprite_coord_enable;
    r300->is_point = last_is_point;
}

void r300_init_render_functions(struct r300_context *r300)
{
    /* Set draw functions based on presence of HW TCL. */
    if (r300->screen->caps.has_tcl) {
        r300->context.draw_vbo = r300_draw_vbo;
    } else {
        r300->context.draw_vbo = r300_swtcl_draw_vbo;
    }

    /* Plug in the two-sided stencil reference value fallback if needed. */
    if (!r300->screen->caps.is_r500)
        r300_plug_in_stencil_ref_fallback(r300);
}
