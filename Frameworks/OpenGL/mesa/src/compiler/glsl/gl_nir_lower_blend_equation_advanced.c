/*
 * Copyright © 2023 Timothy Arceri <tarceri@itsqueeze.com>
 * Copyright © 2016 Intel Corporation
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
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */


#include "nir.h"
#include "nir_builder.h"
#include "gl_nir.h"
#include "program/prog_instruction.h"

#include "util/bitscan.h"
#include "main/shader_types.h"

#define imm1(b, x) nir_imm_float(b, x)
#define imm3(b, x) nir_imm_vec3(b, x, x, x)

static nir_def *
swizzle(nir_builder *b, nir_def *src, int swizzle, int components)
{
   unsigned swizzle_arr[4];
   swizzle_arr[0] = GET_SWZ(swizzle, 0);
   swizzle_arr[1] = GET_SWZ(swizzle, 1);
   swizzle_arr[2] = GET_SWZ(swizzle, 2);
   swizzle_arr[3] = GET_SWZ(swizzle, 3);

   return nir_swizzle(b, src, swizzle_arr, components);
}

static nir_def *
swizzle_x(nir_builder *b, nir_def *src)
{
   return nir_channel(b, src, 0);
}

static nir_def *
swizzle_y(nir_builder *b, nir_def *src)
{
   return nir_channel(b, src, 1);
}

static nir_def *
swizzle_z(nir_builder *b, nir_def *src)
{
   return nir_channel(b, src, 2);
}

static nir_def *
swizzle_w(nir_builder *b, nir_def *src)
{
   return nir_channel(b, src, 3);
}

static nir_def *
blend_multiply(nir_builder *b, nir_def *src, nir_def *dst)
{
   /* f(Cs,Cd) = Cs*Cd */
   return nir_fmul(b, src, dst);
}

static nir_def *
blend_screen(nir_builder *b, nir_def *src, nir_def *dst)
{
   /* f(Cs,Cd) = Cs+Cd-Cs*Cd */
   return nir_fsub(b, nir_fadd(b, src, dst), nir_fmul(b, src, dst));
}

static nir_def *
blend_overlay(nir_builder *b, nir_def *src, nir_def *dst)
{
   /* f(Cs,Cd) = 2*Cs*Cd, if Cd <= 0.5
    *            1-2*(1-Cs)*(1-Cd), otherwise
    */
   nir_def *rule_1 = nir_fmul(b, nir_fmul(b, src, dst), imm3(b, 2.0));
   nir_def *rule_2 =
      nir_fsub(b, imm3(b, 1.0), nir_fmul(b, nir_fmul(b, nir_fsub(b, imm3(b, 1.0), src), nir_fsub(b, imm3(b, 1.0), dst)), imm3(b, 2.0)));
   return nir_bcsel(b, nir_fge(b, imm3(b, 0.5f), dst), rule_1, rule_2);
}

static nir_def *
blend_darken(nir_builder *b, nir_def *src, nir_def *dst)
{
   /* f(Cs,Cd) = min(Cs,Cd) */
   return nir_fmin(b, src, dst);
}

static nir_def *
blend_lighten(nir_builder *b, nir_def *src, nir_def *dst)
{
   /* f(Cs,Cd) = max(Cs,Cd) */
   return nir_fmax(b, src, dst);
}

static nir_def *
blend_colordodge(nir_builder *b, nir_def *src, nir_def *dst)
{
   /* f(Cs,Cd) =
    *   0, if Cd <= 0
    *   min(1,Cd/(1-Cs)), if Cd > 0 and Cs < 1
    *   1, if Cd > 0 and Cs >= 1
    */
   return nir_bcsel(b, nir_fge(b, imm3(b, 0.0), dst), imm3(b, 0.0),
                    nir_bcsel(b, nir_fge(b, src, imm3(b, 1.0)), imm3(b, 1.0),
                              nir_fmin(b, imm3(b, 1.0), nir_fdiv(b, dst, nir_fsub(b, imm3(b, 1.0), src)))));
}

static nir_def *
blend_colorburn(nir_builder *b, nir_def *src, nir_def *dst)
{
   /* f(Cs,Cd) =
    *   1, if Cd >= 1
    *   1 - min(1,(1-Cd)/Cs), if Cd < 1 and Cs > 0
    *   0, if Cd < 1 and Cs <= 0
    */
   return nir_bcsel(b, nir_fge(b, dst, imm3(b, 1.0)), imm3(b, 1.0),
                    nir_bcsel(b, nir_fge(b, imm3(b, 0.0), src), imm3(b, 0.0),
                              nir_fsub(b, imm3(b, 1.0), nir_fmin(b, imm3(b, 1.0), nir_fdiv(b, nir_fsub(b, imm3(b, 1.0), dst), src)))));
}

static nir_def *
blend_hardlight(nir_builder *b, nir_def *src, nir_def *dst)
{
   /* f(Cs,Cd) = 2*Cs*Cd, if Cs <= 0.5
    *            1-2*(1-Cs)*(1-Cd), otherwise
    */
   nir_def *rule_1 = nir_fmul(b, imm3(b, 2.0), nir_fmul(b, src, dst));
   nir_def *rule_2 =
      nir_fsub(b, imm3(b, 1.0), nir_fmul(b, imm3(b, 2.0), nir_fmul(b, nir_fsub(b, imm3(b, 1.0), src), nir_fsub(b, imm3(b, 1.0), dst))));
   return nir_bcsel(b, nir_fge(b, imm3(b, 0.5), src), rule_1, rule_2);
}

static nir_def *
blend_softlight(nir_builder *b, nir_def *src, nir_def *dst)
{
   /* f(Cs,Cd) =
    *   Cd-(1-2*Cs)*Cd*(1-Cd),
    *     if Cs <= 0.5
    *   Cd+(2*Cs-1)*Cd*((16*Cd-12)*Cd+3),
    *     if Cs > 0.5 and Cd <= 0.25
    *   Cd+(2*Cs-1)*(sqrt(Cd)-Cd),
    *     if Cs > 0.5 and Cd > 0.25
    *
    * We can simplify this to
    *
    * f(Cs,Cd) = Cd+(2*Cs-1)*g(Cs,Cd) where
    * g(Cs,Cd) = Cd*Cd-Cd             if Cs <= 0.5
    *            Cd*((16*Cd-12)*Cd+3) if Cs > 0.5 and Cd <= 0.25
    *            sqrt(Cd)-Cd,         otherwise
    */
   nir_def *factor_1 = nir_fmul(b, dst, nir_fsub(b, imm3(b, 1.0), dst));
   nir_def *factor_2 =
      nir_fmul(b, dst, nir_fadd(b, nir_fmul(b, nir_fsub(b, nir_fmul(b, imm3(b, 16.0), dst), imm3(b, 12.0)), dst), imm3(b, 3.0)));
   nir_def *factor_3 = nir_fsub(b, nir_fsqrt(b, dst), dst);
   nir_def *factor = nir_bcsel(b, nir_fge(b, imm3(b, 0.5), src), factor_1,
                                   nir_bcsel(b, nir_fge(b, imm3(b, 0.25), dst), factor_2, factor_3));
   return nir_fadd(b, dst, nir_fmul(b, nir_fsub(b, nir_fmul(b, imm3(b, 2.0), src), imm3(b, 1.0)), factor));
}

static nir_def *
blend_difference(nir_builder *b, nir_def *src, nir_def *dst)
{
   return nir_fabs(b, nir_fsub(b, dst, src));
}

static nir_def *
blend_exclusion(nir_builder *b, nir_def *src, nir_def *dst)
{
   return nir_fadd(b, src, nir_fsub(b, dst, nir_fmul(b, imm3(b, 2.0), nir_fmul(b, src, dst))));
}

/* Return the minimum of a vec3's components */
static nir_def *
minv3(nir_builder *b, nir_def *v)
{
   return nir_fmin(b, nir_fmin(b, swizzle_x(b, v), swizzle_y(b, v)), swizzle_z(b, v));
}

/* Return the maximum of a vec3's components */
static nir_def *
maxv3(nir_builder *b, nir_def *v)
{
   return nir_fmax(b, nir_fmax(b, swizzle_x(b, v), swizzle_y(b, v)), swizzle_z(b, v));
}

static nir_def *
lumv3(nir_builder *b, nir_def *c)
{
   return nir_fdot(b, c, nir_imm_vec3(b, 0.30, 0.59, 0.11));
}

static nir_def *
satv3(nir_builder *b, nir_def *c)
{
   return nir_fsub(b, maxv3(b, c), minv3(b, c));
}

static nir_variable *
add_temp_var(nir_builder *b, char *name, const struct glsl_type *type)
{
   nir_variable *var = rzalloc(b->shader, nir_variable);
   var->type = type;
   var->name = ralloc_strdup(var, name);
   var->data.mode = nir_var_function_temp;
   nir_function_impl_add_variable(b->impl, var);

  return var;
}

/* Take the base RGB color <cbase> and override its luminosity with that
 * of the RGB color <clum>.
 *
 * This follows the equations given in the ES 3.2 (June 15th, 2016)
 * specification.  Revision 16 of GL_KHR_blend_equation_advanced and
 * revision 9 of GL_NV_blend_equation_advanced specify a different set
 * of equations.  Older revisions match ES 3.2's text, and dEQP expects
 * the ES 3.2 rules implemented here.
 */
static void
set_lum(nir_builder *b,
        nir_variable *color,
        nir_variable *cbase,
        nir_variable *clum)
{
   nir_def *cbase_def = nir_load_var(b, cbase);
   nir_store_var(b, color, nir_fadd(b, cbase_def, nir_fsub(b, lumv3(b, nir_load_var(b, clum)), lumv3(b, cbase_def))), ~0);

   nir_variable *llum = add_temp_var(b, "__blend_lum", glsl_float_type());
   nir_variable *mincol = add_temp_var(b, "__blend_mincol", glsl_float_type());
   nir_variable *maxcol = add_temp_var(b, "__blend_maxcol", glsl_float_type());

   nir_def *color_def = nir_load_var(b, color);
   nir_store_var(b, llum, lumv3(b, color_def), ~0);
   nir_store_var(b, mincol, minv3(b, color_def), ~0);
   nir_store_var(b, maxcol, maxv3(b, color_def), ~0);

   nir_def *mincol_def = nir_load_var(b, mincol);
   nir_def *llum_def = nir_load_var(b, llum);
   nir_if *nif = nir_push_if(b, nir_flt(b, mincol_def, imm1(b, 0.0)));

   /* Add then block */
   nir_store_var(b, color, nir_fadd(b, llum_def, nir_fdiv(b, nir_fmul(b, nir_fsub(b, color_def, llum_def), llum_def), nir_fsub(b, llum_def, mincol_def))), ~0);

   /* Add else block */
   nir_push_else(b, nif);
   nir_def *maxcol_def = nir_load_var(b, maxcol);
   nir_if *nif2 = nir_push_if(b, nir_flt(b, imm1(b, 1.0), maxcol_def));
   nir_store_var(b, color, nir_fadd(b, llum_def, nir_fdiv(b, nir_fmul(b, nir_fsub(b, color_def, llum_def), nir_fsub(b, imm3(b, 1.0), llum_def)), nir_fsub(b, maxcol_def, llum_def))), ~0);
   nir_pop_if(b, nif2);
   nir_pop_if(b, nif);
}

/* Take the base RGB color <cbase> and override its saturation with
 * that of the RGB color <csat>.  The override the luminosity of the
 * result with that of the RGB color <clum>.
 */
static void
set_lum_sat(nir_builder *b,
            nir_variable *color,
            nir_variable *cbase,
            nir_variable *csat,
            nir_variable *clum)
{
   nir_def *cbase_def = nir_load_var(b, cbase);
   nir_def *csat_def = nir_load_var(b, csat);

   nir_variable *sbase = add_temp_var(b, "__blend_sbase", glsl_float_type());
   nir_store_var(b, sbase, satv3(b, cbase_def), ~0);

   /* Equivalent (modulo rounding errors) to setting the
    * smallest (R,G,B) component to 0, the largest to <ssat>,
    * and interpolating the "middle" component based on its
    * original value relative to the smallest/largest.
    */
   nir_def *sbase_def = nir_load_var(b, sbase);
   nir_if *nif = nir_push_if(b, nir_flt(b, imm1(b, 0.0), sbase_def));
   nir_def *ssat = satv3(b, csat_def);
   nir_def *minbase = minv3(b, cbase_def);
   nir_store_var(b, color, nir_fdiv(b, nir_fmul(b, nir_fsub(b, cbase_def, minbase), ssat), sbase_def), ~0);
   nir_push_else(b, nif);
   nir_store_var(b, color, imm3(b, 0.0), ~0);
   nir_pop_if(b, nif);

   set_lum(b, color, color, clum);
}

static nir_def *
is_mode(nir_builder *b, nir_variable *mode, enum gl_advanced_blend_mode q)
{
   return nir_ieq_imm(b, nir_load_var(b, mode), (unsigned) q);
}

static nir_variable *
calc_blend_result(nir_builder *b,
                  nir_variable *mode,
                  nir_variable *fb,
                  nir_def *blend_src,
                  GLbitfield blend_qualifiers)
{
   nir_variable *result = add_temp_var(b, "__blend_result", glsl_vec4_type());

   /* If we're not doing advanced blending, just write the original value. */
   nir_if *if_blending = nir_push_if(b, is_mode(b, mode, BLEND_NONE));
   nir_store_var(b, result, blend_src, ~0);

   nir_push_else(b, if_blending);

   /* (Rs', Gs', Bs') =
    *   (0, 0, 0),              if As == 0
    *   (Rs/As, Gs/As, Bs/As),  otherwise
    */
   nir_variable *src_rgb = add_temp_var(b, "__blend_src_rgb", glsl_vec_type(3));
   nir_variable *src_alpha = add_temp_var(b, "__blend_src_a", glsl_float_type());

   /* (Rd', Gd', Bd') =
    *   (0, 0, 0),              if Ad == 0
    *   (Rd/Ad, Gd/Ad, Bd/Ad),  otherwise
    */
   nir_variable *dst_rgb = add_temp_var(b, "__blend_dst_rgb", glsl_vec_type(3));
   nir_variable *dst_alpha = add_temp_var(b, "__blend_dst_a", glsl_float_type());

   nir_def *fb_def = nir_load_var(b, fb);
   nir_store_var(b, dst_alpha, swizzle_w(b, fb_def), ~0);

   nir_def *dst_alpha_def = nir_load_var(b, dst_alpha);
   nir_if *nif = nir_push_if(b, nir_feq(b, dst_alpha_def, imm1(b, 0.0)));
   nir_store_var(b, dst_rgb, imm3(b, 0.0), ~0);
   nir_push_else(b, nif);
   nir_store_var(b, dst_rgb, nir_bcsel(b, nir_feq(b, nir_trim_vector(b, fb_def, 3), swizzle(b, fb_def, SWIZZLE_WWWW, 3)), imm3(b, 1.0), nir_fdiv(b, nir_trim_vector(b, fb_def, 3), dst_alpha_def)), ~0);
   nir_pop_if(b, nif);

   nir_store_var(b, src_alpha, swizzle_w(b, blend_src), ~0);
   nir_def *src_alpha_def = nir_load_var(b, src_alpha);
   nif = nir_push_if(b, nir_feq(b, src_alpha_def, imm1(b, 0.0)));
   nir_store_var(b, src_rgb, imm3(b, 0.0), ~0);
   nir_push_else(b, nif);
   nir_store_var(b, src_rgb, nir_bcsel(b, nir_feq(b, nir_trim_vector(b, blend_src, 3), swizzle(b, blend_src, SWIZZLE_WWWW, 3)), imm3(b, 1.0), nir_fdiv(b, nir_trim_vector(b, blend_src, 3), src_alpha_def)), ~0);
   nir_pop_if(b, nif);

   nir_variable *factor = add_temp_var(b, "__blend_factor", glsl_vec_type(3));

   nir_def *src_rgb_def = nir_load_var(b, src_rgb);
   nir_def *dst_rgb_def = nir_load_var(b, dst_rgb);

   unsigned choices = blend_qualifiers;
   while (choices) {
      enum gl_advanced_blend_mode choice = (enum gl_advanced_blend_mode)u_bit_scan(&choices);

      nir_if *iff = nir_push_if(b, is_mode(b, mode, choice));
      nir_def *val = NULL;

      switch (choice) {
      case BLEND_MULTIPLY:
         val = blend_multiply(b, src_rgb_def, dst_rgb_def);
         break;
      case BLEND_SCREEN:
         val = blend_screen(b, src_rgb_def, dst_rgb_def);
         break;
      case BLEND_OVERLAY:
         val = blend_overlay(b, src_rgb_def, dst_rgb_def);
         break;
      case BLEND_DARKEN:
         val = blend_darken(b, src_rgb_def, dst_rgb_def);
         break;
      case BLEND_LIGHTEN:
         val = blend_lighten(b, src_rgb_def, dst_rgb_def);
         break;
      case BLEND_COLORDODGE:
         val = blend_colordodge(b, src_rgb_def, dst_rgb_def);
         break;
      case BLEND_COLORBURN:
         val = blend_colorburn(b, src_rgb_def, dst_rgb_def);
         break;
      case BLEND_HARDLIGHT:
         val = blend_hardlight(b, src_rgb_def, dst_rgb_def);
         break;
      case BLEND_SOFTLIGHT:
         val = blend_softlight(b, src_rgb_def, dst_rgb_def);
         break;
      case BLEND_DIFFERENCE:
         val = blend_difference(b, src_rgb_def, dst_rgb_def);
         break;
      case BLEND_EXCLUSION:
         val = blend_exclusion(b, src_rgb_def, dst_rgb_def);
         break;
      case BLEND_HSL_HUE:
         set_lum_sat(b, factor, src_rgb, dst_rgb, dst_rgb);
         break;
      case BLEND_HSL_SATURATION:
         set_lum_sat(b, factor, dst_rgb, src_rgb, dst_rgb);
         break;
      case BLEND_HSL_COLOR:
         set_lum(b, factor, src_rgb, dst_rgb);
         break;
      case BLEND_HSL_LUMINOSITY:
         set_lum(b, factor, dst_rgb, src_rgb);
         break;
      case BLEND_NONE:
         unreachable("not real cases");
      }

      if (val)
         nir_store_var(b, factor, val, ~0);

      nir_push_else(b, iff);
   }

   /* reset cursor to the outtermost if-statements else block */
   b->cursor = nir_after_block(nir_if_last_else_block(if_blending));

   /* p0(As,Ad) = As*Ad
    * p1(As,Ad) = As*(1-Ad)
    * p2(As,Ad) = Ad*(1-As)
    */
   nir_variable *p0 = add_temp_var(b, "__blend_p0", glsl_float_type());
   nir_variable *p1 = add_temp_var(b, "__blend_p1", glsl_float_type());
   nir_variable *p2 = add_temp_var(b, "__blend_p2", glsl_float_type());

   nir_store_var(b, p0, nir_fmul(b, src_alpha_def, dst_alpha_def), ~0);
   nir_store_var(b, p1, nir_fmul(b, src_alpha_def, nir_fsub(b, imm1(b, 1.0), dst_alpha_def)), ~0);
   nir_store_var(b, p2, nir_fmul(b, dst_alpha_def, nir_fsub(b, imm1(b, 1.0), src_alpha_def)), ~0);

   /* R = f(Rs',Rd')*p0(As,Ad) + Y*Rs'*p1(As,Ad) + Z*Rd'*p2(As,Ad)
    * G = f(Gs',Gd')*p0(As,Ad) + Y*Gs'*p1(As,Ad) + Z*Gd'*p2(As,Ad)
    * B = f(Bs',Bd')*p0(As,Ad) + Y*Bs'*p1(As,Ad) + Z*Bd'*p2(As,Ad)
    * A =          X*p0(As,Ad) +     Y*p1(As,Ad) +     Z*p2(As,Ad)
    *
    * <X, Y, Z> is always <1, 1, 1>, so we can ignore it.
    *
    * In vector form, this is:
    * RGB = factor * p0 + Cs * p1 + Cd * p2
    *   A = p0 + p1 + p2
    */
   src_rgb_def = nir_load_var(b, src_rgb);
   dst_rgb_def = nir_load_var(b, dst_rgb);
   /* WRITEMASK_XYZ */
   nir_store_var(b, result, nir_pad_vec4(b, nir_fadd(b, nir_fadd(b, nir_fmul(b, nir_load_var(b, factor), nir_load_var(b, p0)), nir_fmul(b, src_rgb_def, nir_load_var(b, p1))), nir_fmul(b, dst_rgb_def, nir_load_var(b, p2)))), 0x7);
   /* WRITEMASK_W */
   nir_def *val = nir_fadd(b, nir_fadd(b, nir_load_var(b, p0), nir_load_var(b, p1)), nir_load_var(b, p2));
   nir_store_var(b, result, nir_vec4(b, val, val, val, val), 0x8);

   /* reset cursor to the end of the main function */
   b->cursor = nir_after_impl(b->impl);

   return result;
}

/**
 * Dereference var, or var[0] if it's an array.
 */
static nir_def *
load_output(nir_builder *b, nir_variable *var)
{
   nir_def *var_def;
   if (glsl_type_is_array(var->type)) {
      var_def = nir_load_array_var_imm(b, var, 0);
   } else {
      var_def = nir_load_var(b, var);
   }

   return var_def;
}

bool
gl_nir_lower_blend_equation_advanced(nir_shader *sh, bool coherent)
{
   assert(sh->info.stage == MESA_SHADER_FRAGMENT);

   /* All functions should have been inlined at this point */
   assert(exec_list_length(&sh->functions) == 1);
   nir_function_impl *impl = nir_shader_get_entrypoint(sh);

   if (sh->info.fs.advanced_blend_modes == 0) {
      nir_metadata_preserve(impl, nir_metadata_all);
      return false;
   }

   sh->info.fs.uses_sample_shading = true;

   nir_builder b = nir_builder_at(nir_after_impl(impl));

   nir_variable *fb = nir_variable_create(sh, nir_var_shader_out,
                                          glsl_vec4_type(),
                                          "__blend_fb_fetch");
   fb->data.location = -1; /* We will set the location at the end of this pass */
   fb->data.read_only = 1;
   fb->data.fb_fetch_output = 1;
   if (coherent)
      fb->data.access = ACCESS_COHERENT;
   fb->data.how_declared = nir_var_hidden;

   nir_variable *mode = nir_variable_create(sh, nir_var_uniform,
                                            glsl_uint_type(),
                                            "gl_AdvancedBlendModeMESA");
   mode->data.how_declared = nir_var_hidden;
   mode->state_slots = rzalloc_array(mode, nir_state_slot, 1);
   mode->num_state_slots = 1;
   mode->state_slots[0].tokens[0] = STATE_ADVANCED_BLENDING_MODE;

   /* Gather any output variables referring to render target 0.
    *
    * ARB_enhanced_layouts irritatingly allows the shader to specify
    * multiple output variables for the same render target, each of
    * which writes a subset of the components, starting at location_frac.
    * The variables can't overlap, thankfully.
    */
   nir_variable *outputs[4] = { NULL, NULL, NULL, NULL };
   nir_foreach_variable_with_modes(var, sh, nir_var_shader_out) {
      if (var->data.location == FRAG_RESULT_DATA0 ||
          var->data.location == FRAG_RESULT_COLOR) {
         const int components =
            glsl_get_vector_elements(glsl_without_array(var->type));

         for (int i = 0; i < components; i++) {
            if (outputs[var->data.location_frac + i] == NULL)
               outputs[var->data.location_frac + i] = var;
         }
      }
   }

   /* Combine values written to outputs into a single RGBA blend source.
    * We assign <0, 0, 0, 1> to any components with no corresponding output.
    */
   nir_def *blend_source;
   if (outputs[0] &&
       glsl_get_vector_elements(glsl_without_array(outputs[0]->type)) == 4) {
      blend_source = load_output(&b, outputs[0]);
   } else {
      nir_def *blend_comps[4];
      for (int i = 0; i < 4; i++) {
         nir_variable *var = outputs[i];
         if (var) {
            blend_comps[i] = swizzle(&b, load_output(&b, outputs[i]),
                                     i - outputs[i]->data.location_frac, 1);
         } else {
            blend_comps[i] = nir_imm_float(&b, i < 3 ? 0.0f : 1.0f);
         }
      }

      blend_source = nir_vec(&b, blend_comps, 4);
   }

   nir_variable *result_dest =
      calc_blend_result(&b, mode, fb, blend_source,
                        sh->info.fs.advanced_blend_modes);

   /* Copy the result back to the original values. */
   for (int i = 0; i < 4; i++) {
      if (!outputs[i])
         continue;

      if (glsl_type_is_array(outputs[i]->type)) {
         nir_store_array_var_imm(&b, outputs[i], 0, nir_load_var(&b, result_dest), 1 << i);
      } else {
         nir_def *val = swizzle(&b, nir_load_var(&b, result_dest), i, 1);
         nir_store_var(&b, outputs[i], nir_vec4(&b, val, val, val, val), 1 << i);
      }
   }

   nir_metadata_preserve(impl, nir_metadata_none);

   /* Remove any dead writes before assigning location to __blend_fb_fetch
    * otherwise they will be unable to be removed.
    */
   NIR_PASS_V(sh, nir_split_var_copies);
   NIR_PASS_V(sh, nir_opt_dead_write_vars);

   nir_foreach_variable_with_modes(var, sh, nir_var_shader_out) {
      if (strcmp(var->name, "__blend_fb_fetch") == 0) {
         var->data.location = FRAG_RESULT_DATA0;
         break;
      }
   }

   nir_validate_shader(sh, "after lower blend equation advanced");
   return true;
}
