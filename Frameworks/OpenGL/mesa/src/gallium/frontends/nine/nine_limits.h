/*
 * Copyright 2015 Axel Davy <axel.davy@ens.fr>
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

#ifndef _NINE_LIMITS_H_
#define _NINE_LIMITS_H_

#include "assert.h"
#include "d3d9types.h"

// state can be any value
#define NINE_STATE_NO_LIMIT 0
// value is clamped if below min or max
#define NINE_STATE_CLAMP 1
// boolean: 0 -> false; any other value -> true
#define NINE_STATE_BOOL 2
// a mask is applied on the value
#define NINE_STATE_MASK 3
// if outside a range, state value is changed to a default value
#define NINE_STATE_RANGE_DEF_VAL 4

struct nine_state_behaviour {
  unsigned state_value_behaviour;
  union {
    struct {
      unsigned min;
      unsigned max;
    } clamp;
    unsigned mask;
    struct {
      unsigned min;
      unsigned max;
      unsigned default_val;
    } range_def_val;
  } u;
};

#define __NO_LIMIT_RS(o) \
    [D3DRS_##o] = {NINE_STATE_NO_LIMIT}

#define __CLAMP_RS(o, m, M) \
    [D3DRS_##o] = {NINE_STATE_CLAMP, {.clamp = {m, M}}}

#define __BOOLEAN_RS(o) \
    [D3DRS_##o] = {NINE_STATE_BOOL}

#define __MASK_RS(o, m) \
    [D3DRS_##o] = {NINE_STATE_MASK, {.mask = m}}

#define __RANGE_DEF_VAL_RS(o, m, M, d) \
    [D3DRS_##o] = {NINE_STATE_RANGE_DEF_VAL, {.range_def_val = {m, M, d}}}

#define __TO_DETERMINE_RS(o, m, M) \
    [D3DRS_##o] = {NINE_STATE_NO_LIMIT}

static const struct nine_state_behaviour
render_state_limits_table[D3DRS_BLENDOPALPHA + 1] = {
    __TO_DETERMINE_RS(ZENABLE, 0, 3),
    __TO_DETERMINE_RS(FILLMODE, 1, 3),
    __CLAMP_RS(SHADEMODE, 1, 3),
    __BOOLEAN_RS(ZWRITEENABLE),
    __BOOLEAN_RS(ALPHATESTENABLE),
    __BOOLEAN_RS(LASTPIXEL),
    __RANGE_DEF_VAL_RS(SRCBLEND, 1, 17, D3DBLEND_ZERO),
    __RANGE_DEF_VAL_RS(DESTBLEND, 1, 17, D3DBLEND_ZERO),
    __CLAMP_RS(CULLMODE, 1, 3),
    __CLAMP_RS(ZFUNC, 1, 8),
    __MASK_RS(ALPHAREF, 0x000000FF),
    __CLAMP_RS(ALPHAFUNC, 1, 8),
    __BOOLEAN_RS(DITHERENABLE),
    __BOOLEAN_RS(ALPHABLENDENABLE),
    __BOOLEAN_RS(FOGENABLE),
    __BOOLEAN_RS(SPECULARENABLE),
    __NO_LIMIT_RS(FOGCOLOR),
    __MASK_RS(FOGTABLEMODE, 0x00000003),
    __NO_LIMIT_RS(FOGSTART), /* a bit more complex than that, lets ignore */
    __NO_LIMIT_RS(FOGEND),
    __NO_LIMIT_RS(FOGDENSITY), /* actually should be between 0.0 and 1.0 */
    __BOOLEAN_RS(RANGEFOGENABLE),
    __BOOLEAN_RS(STENCILENABLE),
    __CLAMP_RS(STENCILFAIL, 1, 8),
    __CLAMP_RS(STENCILZFAIL, 1, 8),
    __CLAMP_RS(STENCILPASS, 1, 8),
    __CLAMP_RS(STENCILFUNC, 1, 8),
    __NO_LIMIT_RS(STENCILREF),
    __NO_LIMIT_RS(STENCILMASK),
    __NO_LIMIT_RS(STENCILWRITEMASK),
    __NO_LIMIT_RS(TEXTUREFACTOR),
    __TO_DETERMINE_RS(WRAP0, 0, 15),
    __TO_DETERMINE_RS(WRAP1, 0, 15),
    __TO_DETERMINE_RS(WRAP2, 0, 15),
    __TO_DETERMINE_RS(WRAP3, 0, 15),
    __TO_DETERMINE_RS(WRAP4, 0, 15),
    __TO_DETERMINE_RS(WRAP5, 0, 15),
    __TO_DETERMINE_RS(WRAP6, 0, 15),
    __TO_DETERMINE_RS(WRAP7, 0, 15),
    __BOOLEAN_RS(CLIPPING),
    __BOOLEAN_RS(LIGHTING),
    __NO_LIMIT_RS(AMBIENT),
    __MASK_RS(FOGVERTEXMODE, 0x00000003),
    __BOOLEAN_RS(COLORVERTEX),
    __BOOLEAN_RS(LOCALVIEWER),
    __BOOLEAN_RS(NORMALIZENORMALS),
    __TO_DETERMINE_RS(DIFFUSEMATERIALSOURCE, 0, 2),
    __TO_DETERMINE_RS(SPECULARMATERIALSOURCE, 0, 2),
    __TO_DETERMINE_RS(AMBIENTMATERIALSOURCE, 0, 2),
    __TO_DETERMINE_RS(EMISSIVEMATERIALSOURCE, 0, 2),
    __TO_DETERMINE_RS(VERTEXBLEND, 0, 256), /* values between 4 and 254 -both included- are forbidden too */
    __NO_LIMIT_RS(CLIPPLANEENABLE), /* expected check seems complex */
    __TO_DETERMINE_RS(POINTSIZE, 0, 0xFFFFFFFF),
    __TO_DETERMINE_RS(POINTSIZE_MIN, 0, 0x7FFFFFFF), /* float >= 0.0 */
    __BOOLEAN_RS(POINTSPRITEENABLE),
    __BOOLEAN_RS(POINTSCALEENABLE),
    __TO_DETERMINE_RS(POINTSCALE_A, 0, 0x7FFFFFFF), /* float >= 0.0 */
    __TO_DETERMINE_RS(POINTSCALE_B, 0, 0x7FFFFFFF), /* float >= 0.0 */
    __TO_DETERMINE_RS(POINTSCALE_C, 0, 0x7FFFFFFF), /* float >= 0.0 */
    __BOOLEAN_RS(MULTISAMPLEANTIALIAS),
    __NO_LIMIT_RS(MULTISAMPLEMASK),
    __TO_DETERMINE_RS(PATCHEDGESTYLE, 0, 1),
    __TO_DETERMINE_RS(DEBUGMONITORTOKEN, 0, 1),
    __TO_DETERMINE_RS(POINTSIZE_MAX, 0, 0x7FFFFFFF), /* check more complex than that */
    __BOOLEAN_RS(INDEXEDVERTEXBLENDENABLE),
    __TO_DETERMINE_RS(COLORWRITEENABLE, 0, 15),
    __NO_LIMIT_RS(TWEENFACTOR),
    __CLAMP_RS(BLENDOP, 1, 5),
    __TO_DETERMINE_RS(POSITIONDEGREE, 1, 5), /* can actually be only 1 or 5 */
    __TO_DETERMINE_RS(NORMALDEGREE, 1, 2),
    __BOOLEAN_RS(SCISSORTESTENABLE),
    __NO_LIMIT_RS(SLOPESCALEDEPTHBIAS),
    __BOOLEAN_RS(ANTIALIASEDLINEENABLE),
    __NO_LIMIT_RS(MINTESSELLATIONLEVEL),
    __NO_LIMIT_RS(MAXTESSELLATIONLEVEL),
    __NO_LIMIT_RS(ADAPTIVETESS_X),
    __NO_LIMIT_RS(ADAPTIVETESS_Y),
    __NO_LIMIT_RS(ADAPTIVETESS_Z),
    __NO_LIMIT_RS(ADAPTIVETESS_W),
    __BOOLEAN_RS(ENABLEADAPTIVETESSELLATION),
    __BOOLEAN_RS(TWOSIDEDSTENCILMODE),
    __CLAMP_RS(CCW_STENCILFAIL, 1, 8),
    __CLAMP_RS(CCW_STENCILZFAIL, 1, 8),
    __CLAMP_RS(CCW_STENCILPASS, 1, 8),
    __CLAMP_RS(CCW_STENCILFUNC, 1, 8),
    __TO_DETERMINE_RS(COLORWRITEENABLE1, 0, 15),
    __TO_DETERMINE_RS(COLORWRITEENABLE2, 0, 15),
    __TO_DETERMINE_RS(COLORWRITEENABLE3, 0, 15),
    __NO_LIMIT_RS(BLENDFACTOR),
    __BOOLEAN_RS(SRGBWRITEENABLE),
    __NO_LIMIT_RS(DEPTHBIAS),
    __TO_DETERMINE_RS(WRAP8, 0, 15),
    __TO_DETERMINE_RS(WRAP9, 0, 15),
    __TO_DETERMINE_RS(WRAP10, 0, 15),
    __TO_DETERMINE_RS(WRAP11, 0, 15),
    __TO_DETERMINE_RS(WRAP12, 0, 15),
    __TO_DETERMINE_RS(WRAP13, 0, 15),
    __TO_DETERMINE_RS(WRAP14, 0, 15),
    __TO_DETERMINE_RS(WRAP15, 0, 15),
    __BOOLEAN_RS(SEPARATEALPHABLENDENABLE),
    __RANGE_DEF_VAL_RS(SRCBLENDALPHA, 1, 17, D3DBLEND_ZERO),
    __RANGE_DEF_VAL_RS(DESTBLENDALPHA, 1, 17, D3DBLEND_ZERO),
    __CLAMP_RS(BLENDOPALPHA, 1, 5)
};

static DWORD inline
nine_fix_render_state_value(D3DRENDERSTATETYPE State,
                            DWORD Value)
{
    struct nine_state_behaviour behaviour = render_state_limits_table[State];

    switch (behaviour.state_value_behaviour) {
    case NINE_STATE_NO_LIMIT:
        break;
    case NINE_STATE_CLAMP:
        if (Value < behaviour.u.clamp.min)
            Value = behaviour.u.clamp.min;
        else if (Value > behaviour.u.clamp.max)
            Value = behaviour.u.clamp.max;
        break;
    case NINE_STATE_BOOL:
        Value = Value ? 1 : 0;
        break;
    case NINE_STATE_MASK:
        Value = Value & behaviour.u.mask;
        break;
    case NINE_STATE_RANGE_DEF_VAL:
        if (Value < behaviour.u.range_def_val.min || Value > behaviour.u.range_def_val.max)
            Value = behaviour.u.range_def_val.default_val;
        break;
    }

    return Value;
}

struct nine_limits
{
    unsigned min;
    unsigned max;
};

#define __VALUE_SAMP(o, m, M) \
    [D3DSAMP_##o] = {m, M}

static const struct nine_limits
sampler_state_limits_table[D3DRS_BLENDOPALPHA + 1] = {
    __VALUE_SAMP(ADDRESSU, 1, 5),
    __VALUE_SAMP(ADDRESSV, 1, 5),
    __VALUE_SAMP(ADDRESSW, 1, 5),
    __VALUE_SAMP(BORDERCOLOR, 0, 0xFFFFFFFF),
    __VALUE_SAMP(MAGFILTER, 0, 8), /* 4-5 should be forbidden */
    __VALUE_SAMP(MINFILTER, 0, 8), /* same */
    __VALUE_SAMP(MIPFILTER, 0, 8), /* same */
    __VALUE_SAMP(MIPMAPLODBIAS, 0, 0xFFFFFFFF),
    __VALUE_SAMP(MAXMIPLEVEL, 0, 0xFFFFFFFF),
    __VALUE_SAMP(MAXANISOTROPY, 1, 0xFFFFFFFF), /* Max value should be pCaps->MaxAnisotropy */
    __VALUE_SAMP(SRGBTEXTURE, 0, 1),
    __VALUE_SAMP(ELEMENTINDEX, 0, 0xFFFFFFFF),
    __VALUE_SAMP(DMAPOFFSET, 0, 0xFFFFFFFF)
};

static BOOL inline
nine_check_sampler_state_value(D3DSAMPLERSTATETYPE State,
                               DWORD Value)
{
    struct nine_limits limit;

    limit = sampler_state_limits_table[State];
    return (limit.min <= Value && Value <= limit.max);
}

#endif /* _NINE_HELPERS_H_ */
