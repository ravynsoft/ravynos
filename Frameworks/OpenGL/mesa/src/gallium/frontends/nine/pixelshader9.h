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

#ifndef _NINE_PIXELSHADER9_H_
#define _NINE_PIXELSHADER9_H_

#include "iunknown.h"
#include "nine_shader.h"
#include "nine_state.h"
#include "basetexture9.h"
#include "nine_ff.h"
#include "surface9.h"

struct nine_lconstf;

struct NinePixelShader9
{
    struct NineUnknown base;
    struct nine_shader_variant variant;

    struct {
        const DWORD *tokens;
        DWORD size;
        uint8_t version; /* (major << 4) | minor */
    } byte_code;

    uint8_t bumpenvmat_needed;
    uint16_t sampler_mask;
    uint8_t rt_mask;

    bool int_slots_used[NINE_MAX_CONST_I];
    bool bool_slots_used[NINE_MAX_CONST_B];

    unsigned const_int_slots;
    unsigned const_bool_slots;

    struct nine_shader_constant_combination *c_combinations;

    uint64_t ff_key[6];
    void *ff_cso;

    uint64_t last_key;
    void *last_cso;
    unsigned *last_const_ranges;
    unsigned last_const_used_size; /* in bytes */

    uint64_t next_key;
};
static inline struct NinePixelShader9 *
NinePixelShader9( void *data )
{
    return (struct NinePixelShader9 *)data;
}

static inline BOOL
NinePixelShader9_UpdateKey( struct NinePixelShader9 *ps,
                            struct nine_context *context )
{
    uint16_t samplers_shadow;
    uint16_t samplers_fetch4;
    uint16_t samplers_ps1_types;
    uint8_t projected;
    uint64_t key;
    BOOL res;

    samplers_shadow = (uint16_t)((context->samplers_shadow & NINE_PS_SAMPLERS_MASK) >> NINE_SAMPLER_PS(0));
    samplers_fetch4 = (uint16_t)((context->samplers_fetch4 & NINE_PS_SAMPLERS_MASK) >> NINE_SAMPLER_PS(0));
    key = samplers_shadow & ps->sampler_mask;
    samplers_fetch4 &= ps->sampler_mask;

    if (unlikely(ps->byte_code.version < 0x20)) {
        /* variable targets */
        uint32_t m = ps->sampler_mask;
        samplers_ps1_types = 0;
        while (m) {
            int s = ffs(m) - 1;
            m &= ~(1 << s);
            samplers_ps1_types |= (context->texture[s].enabled ? context->texture[s].pstype : 1) << (s * 2);
        }
        /* Note: For ps 1.X, only samplers 0 1 2 and 3 are available (except 1.4 where 4 and 5 are available).
         * ps < 1.4: samplers_shadow 4b, samplers_ps1_types 8b, projected 8b
         * ps 1.4: samplers_shadow 6b, samplers_ps1_types 12b
         * Tot ps X.X samplers_shadow + extra: 20b */
        assert((ps->byte_code.version < 0x14 && !(ps->sampler_mask & 0xFFF0)) || !(ps->sampler_mask & 0xFFC0));

        if (unlikely(ps->byte_code.version < 0x14)) {
            key |= samplers_ps1_types << 4;
            projected = nine_ff_get_projected_key_programmable(context);
            key |= ((uint64_t) projected) << 12;
        } else {
            key |= samplers_ps1_types << 6;
        }
    }

    if (ps->byte_code.version < 0x30 && context->rs[D3DRS_FOGENABLE]) {
        key |= 1 << 20;
        key |= ((uint64_t)context->rs[D3DRS_FOGTABLEMODE]) << 21; /* 2 bits */
        key |= ((uint64_t)context->zfog) << 23;
    }

    if ((ps->const_int_slots > 0 || ps->const_bool_slots > 0) && context->inline_constants)
        key |= ((uint64_t)nine_shader_constant_combination_key(&ps->c_combinations,
                                                               ps->int_slots_used,
                                                               ps->bool_slots_used,
                                                               (void *)context->ps_const_i,
                                                               context->ps_const_b)) << 24;

    key |= ((uint64_t)(context->rs[NINED3DRS_FETCH4] & samplers_fetch4)) << 32; /* 16 bits */

    /* centroid interpolation automatically used for color ps inputs */
    if (context->rt[0]->base.info.nr_samples)
        key |= ((uint64_t)1) << 48;
    key |= ((uint64_t)(context->rs[NINED3DRS_EMULATED_ALPHATEST] & 0x7)) << 49; /* 3 bits */
    if (context->rs[D3DRS_SHADEMODE] == D3DSHADE_FLAT)
        key |= ((uint64_t)1) << 52;

    res = ps->last_key != key;
    if (res)
        ps->next_key = key;
    return res;
}

void *
NinePixelShader9_GetVariant( struct NinePixelShader9 *ps,
                             unsigned **const_ranges,
                             unsigned *const_used_size );

/*** public ***/

HRESULT
NinePixelShader9_new( struct NineDevice9 *pDevice,
                      struct NinePixelShader9 **ppOut,
                      const DWORD *pFunction, void *cso );

HRESULT
NinePixelShader9_ctor( struct NinePixelShader9 *,
                       struct NineUnknownParams *pParams,
                       const DWORD *pFunction, void *cso );

void
NinePixelShader9_dtor( struct NinePixelShader9 * );

HRESULT NINE_WINAPI
NinePixelShader9_GetFunction( struct NinePixelShader9 *This,
                              void *pData,
                              UINT *pSizeOfData );

#endif /* _NINE_PIXELSHADER9_H_ */
