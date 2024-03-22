/*
 * Copyright © Microsoft Corporation
 * Copyright © 2022 Valve Corporation
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
 * USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#include "nir_builder.h"
#include "nir_builtin_builder.h"


static const struct glsl_type *
make_2darray_sampler_from_cubemap(const struct glsl_type *type)
{
   return  glsl_get_sampler_dim(type) == GLSL_SAMPLER_DIM_CUBE ?
            glsl_sampler_type(
               GLSL_SAMPLER_DIM_2D,
               false, true,
               glsl_get_sampler_result_type(type)) : type;
}

static const struct glsl_type *
make_2darray_from_cubemap_with_array(const struct glsl_type *type)
{
   if (glsl_type_is_array(type)) {
      const struct glsl_type *new_type = glsl_without_array(type);
      return new_type != type ? glsl_array_type(make_2darray_from_cubemap_with_array(glsl_without_array(type)),
                                                glsl_get_length(type), 0) : type;
   }
   return make_2darray_sampler_from_cubemap(type);
}

static bool
lower_cubemap_to_array_filter(const nir_instr *instr, const void *mask)
{
   const uint32_t *nonseamless_cube_mask = mask;
   if (instr->type == nir_instr_type_tex) {
      nir_tex_instr *tex = nir_instr_as_tex(instr);

      if (tex->sampler_dim != GLSL_SAMPLER_DIM_CUBE)
         return false;

      switch (tex->op) {
      case nir_texop_tex:
      case nir_texop_txb:
      case nir_texop_txd:
      case nir_texop_txl:
      case nir_texop_txs:
      case nir_texop_lod:
      case nir_texop_tg4:
         break;
      default:
         return false;
      }
      return (BITFIELD_BIT(tex->sampler_index) & (*nonseamless_cube_mask)) != 0;
   }

   return false;
}

typedef struct {
   nir_def *rx;
   nir_def *ry;
   nir_def *rz;
   nir_def *arx;
   nir_def *ary;
   nir_def *arz;
   nir_def *array;
} coord_t;


/* This is taken from from sp_tex_sample:convert_cube */
static nir_def *
evaluate_face_x(nir_builder *b, coord_t *coord)
{
   nir_def *sign = nir_fsign(b, coord->rx);
   nir_def *positive = nir_fge_imm(b, coord->rx, 0.0);
   nir_def *ima = nir_fdiv(b, nir_imm_float(b, -0.5), coord->arx);

   nir_def *x = nir_fadd_imm(b, nir_fmul(b, nir_fmul(b, sign, ima), coord->rz), 0.5);
   nir_def *y = nir_fadd_imm(b, nir_fmul(b, ima, coord->ry), 0.5);
   nir_def *face = nir_bcsel(b, positive, nir_imm_float(b, 0.0), nir_imm_float(b, 1.0));

   if (coord->array)
      face = nir_fadd(b, face, coord->array);

   return nir_vec3(b, x,y, face);
}

static nir_def *
evaluate_face_y(nir_builder *b, coord_t *coord)
{
   nir_def *sign = nir_fsign(b, coord->ry);
   nir_def *positive = nir_fge_imm(b, coord->ry, 0.0);
   nir_def *ima = nir_fdiv(b, nir_imm_float(b, 0.5), coord->ary);

   nir_def *x = nir_fadd_imm(b, nir_fmul(b, ima, coord->rx), 0.5);
   nir_def *y = nir_fadd_imm(b, nir_fmul(b, nir_fmul(b, sign, ima), coord->rz), 0.5);
   nir_def *face = nir_bcsel(b, positive, nir_imm_float(b, 2.0), nir_imm_float(b, 3.0));

   if (coord->array)
      face = nir_fadd(b, face, coord->array);
   
   return nir_vec3(b, x,y, face);
}

static nir_def *
evaluate_face_z(nir_builder *b, coord_t *coord)
{
   nir_def *sign = nir_fsign(b, coord->rz);
   nir_def *positive = nir_fge_imm(b, coord->rz, 0.0);
   nir_def *ima = nir_fdiv(b, nir_imm_float(b, -0.5), coord->arz);

   nir_def *x = nir_fadd_imm(b, nir_fmul(b, nir_fmul(b, sign, ima), nir_fneg(b, coord->rx)), 0.5);
   nir_def *y = nir_fadd_imm(b, nir_fmul(b, ima, coord->ry), 0.5);
   nir_def *face = nir_bcsel(b, positive, nir_imm_float(b, 4.0), nir_imm_float(b, 5.0));

   if (coord->array)
      face = nir_fadd(b, face, coord->array);
   
   return nir_vec3(b, x,y, face);
}

static nir_def *
create_array_tex_from_cube_tex(nir_builder *b, nir_tex_instr *tex, nir_def *coord, nir_texop op)
{
   nir_tex_instr *array_tex;

   unsigned num_srcs = tex->num_srcs;
   if (op == nir_texop_txf && nir_tex_instr_src_index(tex, nir_tex_src_comparator) != -1)
      num_srcs--;
   array_tex = nir_tex_instr_create(b->shader, num_srcs);
   array_tex->op = op;
   array_tex->sampler_dim = GLSL_SAMPLER_DIM_2D;
   array_tex->is_array = true;
   array_tex->is_shadow = tex->is_shadow;
   array_tex->is_sparse = tex->is_sparse;
   array_tex->is_new_style_shadow = tex->is_new_style_shadow;
   array_tex->texture_index = tex->texture_index;
   array_tex->sampler_index = tex->sampler_index;
   array_tex->dest_type = tex->dest_type;
   array_tex->coord_components = 3;

   nir_src coord_src = nir_src_for_ssa(coord);
   unsigned s = 0;
   for (unsigned i = 0; i < tex->num_srcs; i++) {
      if (op == nir_texop_txf && tex->src[i].src_type == nir_tex_src_comparator)
         continue;
      nir_src *psrc = (tex->src[i].src_type == nir_tex_src_coord) ?
                         &coord_src : &tex->src[i].src;

      array_tex->src[s].src_type = tex->src[i].src_type;
      if (psrc->ssa->num_components != nir_tex_instr_src_size(array_tex, s)) {
         nir_def *c = nir_trim_vector(b, psrc->ssa,
                                          nir_tex_instr_src_size(array_tex, s));
         array_tex->src[s].src = nir_src_for_ssa(c);
      } else
         array_tex->src[s].src = nir_src_for_ssa(psrc->ssa);
      s++;
   }

   nir_def_init(&array_tex->instr, &array_tex->def,
                nir_tex_instr_dest_size(array_tex),
                tex->def.bit_size);
   nir_builder_instr_insert(b, &array_tex->instr);
   return &array_tex->def;
}

static nir_def *
handle_cube_edge(nir_builder *b, nir_def *x, nir_def *y, nir_def *face, nir_def *array_slice_cube_base, nir_def *tex_size)
{
   enum cube_remap
   {
      cube_remap_zero = 0,
      cube_remap_x,
      cube_remap_y,
      cube_remap_tex_size,
      cube_remap_tex_size_minus_x,
      cube_remap_tex_size_minus_y,

      cube_remap_size,
   };

   struct cube_remap_table
   {
      enum cube_remap remap_x;
      enum cube_remap remap_y;
      uint32_t        remap_face;
   };

   static const struct cube_remap_table cube_remap_neg_x[6] =
   {
       {cube_remap_tex_size,         cube_remap_y,         4},
       {cube_remap_tex_size,         cube_remap_y,         5},
       {cube_remap_y,                cube_remap_zero,      1},
       {cube_remap_tex_size_minus_y, cube_remap_tex_size,  1},
       {cube_remap_tex_size,         cube_remap_y,         1},
       {cube_remap_tex_size,         cube_remap_y,         0},
   };

   static const struct cube_remap_table cube_remap_pos_x[6] =
   {
       {cube_remap_zero,             cube_remap_y,         5},
       {cube_remap_zero,             cube_remap_y,         4},
       {cube_remap_tex_size_minus_y, cube_remap_zero,      0},
       {cube_remap_y,                cube_remap_tex_size,  0},
       {cube_remap_zero,             cube_remap_y,         0},
       {cube_remap_zero,             cube_remap_y,         1},
   };

   static const struct cube_remap_table cube_remap_neg_y[6] =
   {
       {cube_remap_tex_size,         cube_remap_tex_size_minus_x, 2},
       {cube_remap_zero,             cube_remap_x,                2},
       {cube_remap_tex_size_minus_x, cube_remap_zero,             5},
       {cube_remap_x,                cube_remap_tex_size,         4},
       {cube_remap_x,                cube_remap_tex_size,         2},
       {cube_remap_tex_size_minus_x, cube_remap_zero,             2},
   };

   static const struct cube_remap_table cube_remap_pos_y[6] =
   {
       {cube_remap_tex_size,         cube_remap_x,                   3},
       {cube_remap_zero,             cube_remap_tex_size_minus_x,    3},
       {cube_remap_x,                cube_remap_zero,                4},
       {cube_remap_tex_size_minus_x, cube_remap_tex_size,            5},
       {cube_remap_x,                cube_remap_zero,                3},
       {cube_remap_tex_size_minus_x, cube_remap_tex_size,            3},
   };

   static const struct cube_remap_table* remap_tables[4] = {
      cube_remap_neg_x,
      cube_remap_pos_x,
      cube_remap_neg_y,
      cube_remap_pos_y
   };

   nir_def *zero = nir_imm_int(b, 0);
   
   /* Doesn't matter since the texture is square */
   tex_size = nir_channel(b, tex_size, 0);

   nir_def *x_on = nir_iand(b, nir_ige(b, x, zero), nir_ige(b, tex_size, x));
   nir_def *y_on = nir_iand(b, nir_ige(b, y, zero), nir_ige(b, tex_size, y));
   nir_def *one_on = nir_ixor(b, x_on, y_on);

   /* If the sample did not fall off the face in either dimension, then set output = input */
   nir_def *x_result = x;
   nir_def *y_result = y;
   nir_def *face_result = face;

   /* otherwise, if the sample fell off the face in either the X or the Y direction, remap to the new face */
   nir_def *remap_predicates[4] =
   {
      nir_iand(b, one_on, nir_ilt(b, x, zero)),
      nir_iand(b, one_on, nir_ilt(b, tex_size, x)),
      nir_iand(b, one_on, nir_ilt(b, y, zero)),
      nir_iand(b, one_on, nir_ilt(b, tex_size, y)),
   };

   nir_def *remap_array[cube_remap_size];

   remap_array[cube_remap_zero] = zero;
   remap_array[cube_remap_x] = x;
   remap_array[cube_remap_y] = y;
   remap_array[cube_remap_tex_size] = tex_size;
   remap_array[cube_remap_tex_size_minus_x] = nir_isub(b, tex_size, x);
   remap_array[cube_remap_tex_size_minus_y] = nir_isub(b, tex_size, y);

   /* For each possible way the sample could have fallen off */
   for (unsigned i = 0; i < 4; i++) {
      const struct cube_remap_table* remap_table = remap_tables[i];

      /* For each possible original face */
      for (unsigned j = 0; j < 6; j++) {
         nir_def *predicate = nir_iand(b, remap_predicates[i], nir_ieq_imm(b, face, j));

         x_result = nir_bcsel(b, predicate, remap_array[remap_table[j].remap_x], x_result);
         y_result = nir_bcsel(b, predicate, remap_array[remap_table[j].remap_y], y_result);
         face_result = nir_bcsel(b, predicate, remap_array[remap_table[j].remap_face], face_result);
      }
   }

   return nir_vec3(b, x_result, y_result, nir_iadd(b, face_result, array_slice_cube_base));
}

static nir_def *
handle_cube_gather(nir_builder *b, nir_tex_instr *tex, nir_def *coord)
{
   tex->is_array = true;
   nir_def *tex_size = nir_get_texture_size(b, tex);

   /* nir_get_texture_size puts the cursor before the tex op */
   b->cursor = nir_after_instr(coord->parent_instr);

   nir_def *const_05 = nir_imm_float(b, 0.5f);
   nir_def *texel_coords = nir_fmul(b, nir_trim_vector(b, coord, 2),
                                        nir_i2f32(b, nir_trim_vector(b, tex_size, 2)));

   nir_def *x_orig = nir_channel(b, texel_coords, 0);
   nir_def *y_orig = nir_channel(b, texel_coords, 1);

   nir_def *x_pos = nir_f2i32(b, nir_fadd(b, x_orig, const_05));
   nir_def *x_neg = nir_f2i32(b, nir_fsub(b, x_orig, const_05));
   nir_def *y_pos = nir_f2i32(b, nir_fadd(b, y_orig, const_05));
   nir_def *y_neg = nir_f2i32(b, nir_fsub(b, y_orig, const_05));
   nir_def *coords[4][2] = {
      { x_neg, y_pos },
      { x_pos, y_pos },
      { x_pos, y_neg },
      { x_neg, y_neg },
   };

   nir_def *array_slice_2d = nir_f2i32(b, nir_channel(b, coord, 2));
   nir_def *face = nir_imod_imm(b, array_slice_2d, 6);
   nir_def *array_slice_cube_base = nir_isub(b, array_slice_2d, face);

   nir_def *channels[4];
   for (unsigned i = 0; i < 4; ++i) {
      nir_def *final_coord = handle_cube_edge(b, coords[i][0], coords[i][1], face, array_slice_cube_base, tex_size);
      nir_def *sampled_val = create_array_tex_from_cube_tex(b, tex, final_coord, nir_texop_txf);
      channels[i] = nir_channel(b, sampled_val, tex->component);
   }

   return nir_vec(b, channels, 4);
}

static nir_def *
lower_cube_coords(nir_builder *b, nir_def *coord, bool is_array)
{
   coord_t coords;
   coords.rx = nir_channel(b, coord, 0);
   coords.ry = nir_channel(b, coord, 1);
   coords.rz = nir_channel(b, coord, 2);
   coords.arx = nir_fabs(b, coords.rx);
   coords.ary = nir_fabs(b, coords.ry);
   coords.arz = nir_fabs(b, coords.rz);
   coords.array = NULL;
   if (is_array)
      coords.array = nir_fmul_imm(b, nir_channel(b, coord, 3), 6.0f);

   nir_def *use_face_x = nir_iand(b,
                                      nir_fge(b, coords.arx, coords.ary),
                                      nir_fge(b, coords.arx, coords.arz));

   nir_if *use_face_x_if = nir_push_if(b, use_face_x);
   nir_def *face_x_coord = evaluate_face_x(b, &coords);
   nir_if *use_face_x_else = nir_push_else(b, use_face_x_if);

   nir_def *use_face_y = nir_iand(b,
                                      nir_fge(b, coords.ary, coords.arx),
                                      nir_fge(b, coords.ary, coords.arz));

   nir_if *use_face_y_if = nir_push_if(b, use_face_y);
   nir_def *face_y_coord = evaluate_face_y(b, &coords);
   nir_if *use_face_y_else = nir_push_else(b, use_face_y_if);

   nir_def *face_z_coord = evaluate_face_z(b, &coords);

   nir_pop_if(b, use_face_y_else);
   nir_def *face_y_or_z_coord = nir_if_phi(b, face_y_coord, face_z_coord);
   nir_pop_if(b, use_face_x_else);

   // This contains in xy the normalized sample coordinates, and in z the face index
   nir_def *coord_and_face = nir_if_phi(b, face_x_coord, face_y_or_z_coord);

   return coord_and_face;
}

static void
rewrite_cube_var_type(nir_builder *b, nir_tex_instr *tex)
{
   unsigned index = tex->texture_index;
   nir_variable *sampler = NULL;
   int highest = -1;
   nir_foreach_variable_with_modes(var, b->shader, nir_var_uniform) {
      if (!glsl_type_is_sampler(glsl_without_array(var->type)))
         continue;
      unsigned size = glsl_type_is_array(var->type) ? glsl_get_length(var->type) : 1;
      if (var->data.driver_location == index ||
          (var->data.driver_location < index && var->data.driver_location + size > index)) {
         sampler = var;
         break;
      }
      /* handle array sampler access: use the next-closest sampler */
      if (var->data.driver_location > highest && var->data.driver_location < index) {
         highest = var->data.driver_location;
         sampler = var;
      }
   }
   assert(sampler);
   sampler->type = make_2darray_from_cubemap_with_array(sampler->type);
}

/* txb(s, coord, bias) = txl(s, coord, lod(s, coord).y + bias) */
/* tex(s, coord) = txl(s, coord, lod(s, coord).x) */
static nir_tex_instr *
lower_tex_to_txl(nir_builder *b, nir_tex_instr *tex)
{
   b->cursor = nir_after_instr(&tex->instr);
   int bias_idx = nir_tex_instr_src_index(tex, nir_tex_src_bias);
   unsigned num_srcs = bias_idx >= 0 ? tex->num_srcs : tex->num_srcs + 1;
   nir_tex_instr *txl = nir_tex_instr_create(b->shader, num_srcs);

   txl->op = nir_texop_txl;
   txl->sampler_dim = tex->sampler_dim;
   txl->dest_type = tex->dest_type;
   txl->coord_components = tex->coord_components;
   txl->texture_index = tex->texture_index;
   txl->sampler_index = tex->sampler_index;
   txl->is_array = tex->is_array;
   txl->is_shadow = tex->is_shadow;
   txl->is_sparse = tex->is_sparse;
   txl->is_new_style_shadow = tex->is_new_style_shadow;

   unsigned s = 0;
   for (int i = 0; i < tex->num_srcs; i++) {
      if (i == bias_idx)
         continue;
      txl->src[s].src = nir_src_for_ssa(tex->src[i].src.ssa);
      txl->src[s].src_type = tex->src[i].src_type;
      s++;
   }
   nir_def *lod = nir_get_texture_lod(b, tex);

   if (bias_idx >= 0)
      lod = nir_fadd(b, lod, tex->src[bias_idx].src.ssa);
   lod = nir_fadd_imm(b, lod, -1.0);
   txl->src[s] = nir_tex_src_for_ssa(nir_tex_src_lod, lod);

   b->cursor = nir_before_instr(&tex->instr);
   nir_def_init(&txl->instr, &txl->def,
                tex->def.num_components,
                tex->def.bit_size);
   nir_builder_instr_insert(b, &txl->instr);
   nir_def_rewrite_uses(&tex->def, &txl->def);
   return txl;
}

static nir_def *
lower_cube_sample(nir_builder *b, nir_tex_instr *tex)
{
   if (!tex->is_shadow && (tex->op == nir_texop_txb || tex->op == nir_texop_tex)) {
      tex = lower_tex_to_txl(b, tex);
   }
 
   int coord_index = nir_tex_instr_src_index(tex, nir_tex_src_coord);
   assert(coord_index >= 0);

   /* Evaluate the face and the xy coordinates for a 2D tex op */
   nir_def *coord = tex->src[coord_index].src.ssa;
   nir_def *coord_and_face = lower_cube_coords(b, coord, tex->is_array);

   rewrite_cube_var_type(b, tex);

   if (tex->op == nir_texop_tg4 && !tex->is_shadow)
      return handle_cube_gather(b, tex, coord_and_face);
   else
      return create_array_tex_from_cube_tex(b, tex, coord_and_face, tex->op);
}

static nir_def *
lower_cube_txs(nir_builder *b, nir_tex_instr *tex)
{
   b->cursor = nir_after_instr(&tex->instr);

   rewrite_cube_var_type(b, tex);
   unsigned num_components = tex->def.num_components;
   /* force max components to unbreak textureSize().xy */
   tex->def.num_components = 3;
   tex->is_array = true;
   nir_def *array_dim = nir_channel(b, &tex->def, 2);
   nir_def *cube_array_dim = nir_idiv(b, array_dim, nir_imm_int(b, 6));
   nir_def *size = nir_vec3(b, nir_channel(b, &tex->def, 0),
                                   nir_channel(b, &tex->def, 1),
                                   cube_array_dim);
   return nir_trim_vector(b, size, num_components);
}

static nir_def *
lower_cubemap_to_array_tex(nir_builder *b, nir_tex_instr *tex)
{
   switch (tex->op) {
   case nir_texop_tex:
   case nir_texop_txb:
   case nir_texop_txd:
   case nir_texop_txl:
   case nir_texop_lod:
   case nir_texop_tg4:
      return lower_cube_sample(b, tex);
   case nir_texop_txs:
      return lower_cube_txs(b, tex);
   default:
      unreachable("Unsupported cupe map texture operation");
   }
}

static nir_def *
lower_cubemap_to_array_impl(nir_builder *b, nir_instr *instr,
                               UNUSED void *_options)
{
   if (instr->type == nir_instr_type_tex)
      return lower_cubemap_to_array_tex(b, nir_instr_as_tex(instr));
   return NULL;
}

bool
zink_lower_cubemap_to_array(nir_shader *s, uint32_t nonseamless_cube_mask);
bool
zink_lower_cubemap_to_array(nir_shader *s, uint32_t nonseamless_cube_mask)
{
   return nir_shader_lower_instructions(s,
                                        lower_cubemap_to_array_filter,
                                        lower_cubemap_to_array_impl,
                                        &nonseamless_cube_mask);
}
