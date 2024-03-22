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

#ifndef _NINE_SHADER_H_
#define _NINE_SHADER_H_

#include "d3d9types.h"
#include "d3d9caps.h"
#include "nine_defines.h"
#include "nine_helpers.h"
#include "nine_state.h"
#include "pipe/p_state.h" /* PIPE_MAX_ATTRIBS */
#include "util/u_memory.h"
#include "tgsi/tgsi_ureg.h"

struct NineDevice9;
struct NineVertexDeclaration9;
struct ureg_program;

struct nine_lconstf /* NOTE: both pointers should be FREE'd by the user */
{
    struct nine_range *ranges; /* single MALLOC, but next-pointers valid */
    float *data;
};

struct nine_shader_constant_combination;

struct nine_shader_info
{
    unsigned type; /* in, PIPE_SHADER_x */

    uint8_t version; /* (major << 4) | minor */

    const DWORD *byte_code; /* in, pointer to shader tokens */
    DWORD        byte_size; /* out, size of data at byte_code */

    void *cso; /* out, pipe cso for bind_vs,fs_state */

    uint16_t input_map[PIPE_MAX_ATTRIBS]; /* VS input -> NINE_DECLUSAGE_x */
    uint8_t num_inputs; /* there may be unused inputs (NINE_DECLUSAGE_NONE) */

    bool position_t; /* out, true if VP writes pre-transformed position */
    bool point_size; /* out, true if VP writes point size */
    float point_size_min;
    float point_size_max;

    uint32_t sampler_ps1xtypes; /* 2 bits per sampler */
    uint16_t sampler_mask; /* out, which samplers are being used */
    uint16_t sampler_mask_shadow; /* in, which samplers use depth compare */
    uint8_t rt_mask; /* out, which render targets are being written */

    uint8_t fog_enable;
    uint8_t fog_mode;
    uint8_t zfog;
    uint8_t force_color_in_centroid;
    uint8_t color_flatshade;
    uint8_t projected; /* ps 1.1 to 1.3 */
    uint16_t fetch4;
    uint8_t alpha_test_emulation;
    uint8_t clip_plane_emulation;
    bool emulate_features;

    unsigned const_i_base; /* in vec4 (16 byte) units */
    unsigned const_b_base; /* in vec4 (16 byte) units */
    unsigned const_used_size;

    bool int_slots_used[NINE_MAX_CONST_I];
    bool bool_slots_used[NINE_MAX_CONST_B];

    unsigned const_float_slots;
    unsigned const_int_slots;
    unsigned const_bool_slots;

    unsigned *const_ranges;

    struct nine_lconstf lconstf; /* out, NOTE: members to be free'd by user */
    uint8_t bumpenvmat_needed;

    struct {
        struct nine_shader_constant_combination* c_combination;
        bool (*int_const_added)[NINE_MAX_CONST_I];
        bool (*bool_const_added)[NINE_MAX_CONST_B];
    } add_constants_defs;

    bool swvp_on;

    bool process_vertices;
    struct NineVertexDeclaration9 *vdecl_out;
    struct pipe_stream_output_info so;
};

struct nine_vs_output_info
{
    BYTE output_semantic;
    int output_semantic_index;
    int mask;
    int output_index;
};

void *
nine_create_shader_with_so_and_destroy(struct ureg_program *p,
                                       struct pipe_context *pipe,
                                       const struct pipe_stream_output_info *so);

HRESULT
nine_translate_shader(struct NineDevice9 *device,
                      struct nine_shader_info *,
                      struct pipe_context *);


struct nine_shader_variant
{
    struct nine_shader_variant *next;
    void *cso;
    unsigned *const_ranges;
    unsigned const_used_size;
    uint64_t key;
};

static inline void *
nine_shader_variant_get(struct nine_shader_variant *list,
                        unsigned **const_ranges,
                        unsigned *const_used_size,
                        uint64_t key)
{
    while (list->key != key && list->next)
        list = list->next;
    if (list->key == key) {
        *const_ranges = list->const_ranges;
        *const_used_size = list->const_used_size;
        return list->cso;
    }
    return NULL;
}

static inline bool
nine_shader_variant_add(struct nine_shader_variant *list,
                        uint64_t key, void *cso,
                        unsigned *const_ranges,
                        unsigned const_used_size)
{
    while (list->next) {
        assert(list->key != key);
        list = list->next;
    }
    list->next = MALLOC_STRUCT(nine_shader_variant);
    if (!list->next)
        return false;
    list->next->next = NULL;
    list->next->key = key;
    list->next->cso = cso;
    list->next->const_ranges = const_ranges;
    list->next->const_used_size = const_used_size;
    return true;
}

static inline void
nine_shader_variants_free(struct nine_shader_variant *list)
{
    while (list->next) {
        struct nine_shader_variant *ptr = list->next;
        list->next = ptr->next;
        FREE(ptr);
    }
}

struct nine_shader_variant_so
{
    struct nine_shader_variant_so *next;
    struct NineVertexDeclaration9 *vdecl;
    struct pipe_stream_output_info so;
    void *cso;
};

static inline void *
nine_shader_variant_so_get(struct nine_shader_variant_so *list,
                           struct NineVertexDeclaration9 *vdecl,
                           struct pipe_stream_output_info *so)
{
    while (list->vdecl != vdecl && list->next)
        list = list->next;
    if (list->vdecl == vdecl) {
        *so = list->so;
        return list->cso;
    }
    return NULL;
}

static inline bool
nine_shader_variant_so_add(struct nine_shader_variant_so *list,
                           struct NineVertexDeclaration9 *vdecl,
                           struct pipe_stream_output_info *so, void *cso)
{
    if (list->vdecl == NULL) { /* first shader */
        list->next = NULL;
        nine_bind(&list->vdecl, vdecl);
        list->so = *so;
        list->cso = cso;
        return true;
    }
    while (list->next) {
        assert(list->vdecl != vdecl);
        list = list->next;
    }
    list->next = MALLOC_STRUCT(nine_shader_variant_so);
    if (!list->next)
        return false;
    list->next->next = NULL;
    nine_bind(&list->vdecl, vdecl);
    list->next->so = *so;
    list->next->cso = cso;
    return true;
}

static inline void
nine_shader_variants_so_free(struct nine_shader_variant_so *list)
{
    while (list->next) {
        struct nine_shader_variant_so *ptr = list->next;
        list->next = ptr->next;
        nine_bind(&ptr->vdecl, NULL);
        FREE(ptr);
    }
    if (list->vdecl)
        nine_bind(&list->vdecl, NULL);
}

struct nine_shader_constant_combination
{
    struct nine_shader_constant_combination *next;
    int const_i[NINE_MAX_CONST_I][4];
    BOOL const_b[NINE_MAX_CONST_B];
};

#define NINE_MAX_CONSTANT_COMBINATION_VARIANTS 32

static inline uint8_t
nine_shader_constant_combination_key(struct nine_shader_constant_combination **list,
                                     bool *int_slots_used,
                                     bool *bool_slots_used,
                                     int *const_i,
                                     BOOL *const_b)
{
    int i;
    uint8_t index = 0;
    bool match;
    struct nine_shader_constant_combination **next_allocate = list, *current = *list;

    assert(int_slots_used);
    assert(bool_slots_used);
    assert(const_i);
    assert(const_b);

    while (current) {
        index++; /* start at 1. 0 is for the variant without constant replacement */
        match = true;
        for (i = 0; i < NINE_MAX_CONST_I; ++i) {
            if (int_slots_used[i])
                match &= !memcmp(const_i + 4*i, current->const_i[i], sizeof(current->const_i[0]));
        }
        for (i = 0; i < NINE_MAX_CONST_B; ++i) {
            if (bool_slots_used[i])
                match &= const_b[i] == current->const_b[i];
        }
        if (match)
            return index;
        next_allocate = &current->next;
        current = current->next;
    }

    if (index < NINE_MAX_CONSTANT_COMBINATION_VARIANTS) {
        *next_allocate = MALLOC_STRUCT(nine_shader_constant_combination);
        current = *next_allocate;
        index++;
        current->next = NULL;
        memcpy(current->const_i, const_i, sizeof(current->const_i));
        memcpy(current->const_b, const_b, sizeof(current->const_b));
        return index;
    }

    return 0; /* Too many variants, revert to no replacement */
}

static inline struct nine_shader_constant_combination *
nine_shader_constant_combination_get(struct nine_shader_constant_combination *list, uint8_t index)
{
    if (index == 0)
        return NULL;
    while (index) {
        assert(list != NULL);
        index--;
        if (index == 0)
            return list;
        list = list->next;
    }
    assert(false);
    return NULL;
}

static inline void
nine_shader_constant_combination_free(struct nine_shader_constant_combination *list)
{
    if (!list)
        return;

    while (list->next) {
        struct nine_shader_constant_combination *ptr = list->next;
        list->next = ptr->next;
        FREE(ptr);
    }

    FREE(list);
}

/* Returns corresponding opposite test */
static inline unsigned
pipe_comp_to_tgsi_opposite(BYTE flags)
{
    switch (flags) {
    case PIPE_FUNC_GREATER: return TGSI_OPCODE_SLE;
    case PIPE_FUNC_EQUAL: return TGSI_OPCODE_SNE;
    case PIPE_FUNC_GEQUAL: return TGSI_OPCODE_SLT;
    case PIPE_FUNC_LESS: return TGSI_OPCODE_SGE;
    case PIPE_FUNC_NOTEQUAL: return TGSI_OPCODE_SEQ;
    case PIPE_FUNC_LEQUAL: return TGSI_OPCODE_SGT;
    default:
        assert(!"invalid comparison flags");
        return TGSI_OPCODE_SGT;
    }
}

#endif /* _NINE_SHADER_H_ */
