/**************************************************************************
 *
 * Copyright 2009 VMware, Inc.
 * All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sub license, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice (including the
 * next paragraph) shall be included in all copies or substantial portions
 * of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT.
 * IN NO EVENT SHALL VMWARE AND/OR ITS SUPPLIERS BE LIABLE FOR
 * ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 **************************************************************************/

/**
 * @file
 * C - JIT interfaces
 *
 * @author Jose Fonseca <jfonseca@vmware.com>
 */

#include <llvm/Config/llvm-config.h>

#include "util/u_memory.h"
#include "gallivm/lp_bld_init.h"
#include "gallivm/lp_bld_debug.h"
#include "gallivm/lp_bld_format.h"
#include "lp_context.h"
#include "lp_debug.h"
#include "lp_memory.h"
#include "lp_screen.h"
#include "lp_jit.h"

static void
lp_jit_create_types(struct lp_fragment_shader_variant *lp)
{
   struct gallivm_state *gallivm = lp->gallivm;
   LLVMContextRef lc = gallivm->context;
   LLVMTypeRef viewport_type;
   LLVMTypeRef linear_elem_type;

   /* struct lp_jit_viewport */
   {
      LLVMTypeRef elem_types[LP_JIT_VIEWPORT_NUM_FIELDS];

      elem_types[LP_JIT_VIEWPORT_MIN_DEPTH] =
      elem_types[LP_JIT_VIEWPORT_MAX_DEPTH] = LLVMFloatTypeInContext(lc);

      viewport_type = LLVMStructTypeInContext(lc, elem_types,
                                              ARRAY_SIZE(elem_types), 0);

      LP_CHECK_MEMBER_OFFSET(struct lp_jit_viewport, min_depth,
                             gallivm->target, viewport_type,
                             LP_JIT_VIEWPORT_MIN_DEPTH);
      LP_CHECK_MEMBER_OFFSET(struct lp_jit_viewport, max_depth,
                             gallivm->target, viewport_type,
                             LP_JIT_VIEWPORT_MAX_DEPTH);
      LP_CHECK_STRUCT_SIZE(struct lp_jit_viewport,
                           gallivm->target, viewport_type);
   }


   /* struct lp_jit_context */
   {
      LLVMTypeRef elem_types[LP_JIT_CTX_COUNT];
      LLVMTypeRef context_type;

      elem_types[LP_JIT_CTX_ALPHA_REF] = LLVMFloatTypeInContext(lc);
      elem_types[LP_JIT_CTX_SAMPLE_MASK] =
      elem_types[LP_JIT_CTX_STENCIL_REF_FRONT] =
      elem_types[LP_JIT_CTX_STENCIL_REF_BACK] = LLVMInt32TypeInContext(lc);
      elem_types[LP_JIT_CTX_U8_BLEND_COLOR] = LLVMPointerType(LLVMInt8TypeInContext(lc), 0);
      elem_types[LP_JIT_CTX_F_BLEND_COLOR] = LLVMPointerType(LLVMFloatTypeInContext(lc), 0);
      elem_types[LP_JIT_CTX_VIEWPORTS] = LLVMPointerType(viewport_type, 0);

      context_type = LLVMStructTypeInContext(lc, elem_types,
                                             ARRAY_SIZE(elem_types), 0);

      LP_CHECK_MEMBER_OFFSET(struct lp_jit_context, alpha_ref_value,
                             gallivm->target, context_type,
                             LP_JIT_CTX_ALPHA_REF);
      LP_CHECK_MEMBER_OFFSET(struct lp_jit_context, stencil_ref_front,
                             gallivm->target, context_type,
                             LP_JIT_CTX_STENCIL_REF_FRONT);
      LP_CHECK_MEMBER_OFFSET(struct lp_jit_context, stencil_ref_back,
                             gallivm->target, context_type,
                             LP_JIT_CTX_STENCIL_REF_BACK);
      LP_CHECK_MEMBER_OFFSET(struct lp_jit_context, u8_blend_color,
                             gallivm->target, context_type,
                             LP_JIT_CTX_U8_BLEND_COLOR);
      LP_CHECK_MEMBER_OFFSET(struct lp_jit_context, f_blend_color,
                             gallivm->target, context_type,
                             LP_JIT_CTX_F_BLEND_COLOR);
      LP_CHECK_MEMBER_OFFSET(struct lp_jit_context, viewports,
                             gallivm->target, context_type,
                             LP_JIT_CTX_VIEWPORTS);
      LP_CHECK_MEMBER_OFFSET(struct lp_jit_context, sample_mask,
                             gallivm->target, context_type,
                             LP_JIT_CTX_SAMPLE_MASK);
      LP_CHECK_STRUCT_SIZE(struct lp_jit_context,
                           gallivm->target, context_type);

      lp->jit_context_type = context_type;
      lp->jit_context_ptr_type = LLVMPointerType(context_type, 0);
      lp->jit_resources_type = lp_build_jit_resources_type(gallivm);
      lp->jit_resources_ptr_type = LLVMPointerType(lp->jit_resources_type, 0);
   }

   /* struct lp_jit_thread_data */
   {
      LLVMTypeRef elem_types[LP_JIT_THREAD_DATA_COUNT];
      LLVMTypeRef thread_data_type;

      elem_types[LP_JIT_THREAD_DATA_CACHE] =
            LLVMPointerType(lp_build_format_cache_type(gallivm), 0);
      elem_types[LP_JIT_THREAD_DATA_VIS_COUNTER] = LLVMInt64TypeInContext(lc);
      elem_types[LP_JIT_THREAD_DATA_PS_INVOCATIONS] = LLVMInt64TypeInContext(lc);
      elem_types[LP_JIT_THREAD_DATA_RASTER_STATE_VIEWPORT_INDEX] =
      elem_types[LP_JIT_THREAD_DATA_RASTER_STATE_VIEW_INDEX] =
            LLVMInt32TypeInContext(lc);

      thread_data_type = LLVMStructTypeInContext(lc, elem_types,
                                                 ARRAY_SIZE(elem_types), 0);

      lp->jit_thread_data_type = thread_data_type;
      lp->jit_thread_data_ptr_type = LLVMPointerType(thread_data_type, 0);
   }

   /*
    * lp_linear_elem
    *
    * XXX: it can be instanced only once due to the use of opaque types, and
    * the fact that screen->module is also a global.
    */
   {
      LLVMTypeRef ret_type;
      LLVMTypeRef arg_types[1];
      LLVMTypeRef func_type;

      ret_type = LLVMPointerType(LLVMVectorType(LLVMInt8TypeInContext(lc), 16), 0);

      arg_types[0] = LLVMPointerType(LLVMInt8TypeInContext(lc), 0);

      /* lp_linear_func */
      func_type = LLVMFunctionType(ret_type, arg_types, ARRAY_SIZE(arg_types), 0);

      /*
       * We actually define lp_linear_elem not as a structure but simply as a
       * lp_linear_func pointer
       */
      lp->jit_linear_func_type = func_type;
      linear_elem_type = LLVMPointerType(func_type, 0);
   }

   /* struct lp_jit_linear_context */
   {
      LLVMTypeRef linear_elem_ptr_type = LLVMPointerType(linear_elem_type, 0);
      LLVMTypeRef elem_types[LP_JIT_LINEAR_CTX_COUNT];
      LLVMTypeRef linear_context_type;


      elem_types[LP_JIT_LINEAR_CTX_CONSTANTS] = LLVMPointerType(LLVMInt8TypeInContext(lc), 0);
      elem_types[LP_JIT_LINEAR_CTX_TEX] =
      lp->jit_linear_textures_type =
            LLVMArrayType(linear_elem_ptr_type, LP_MAX_LINEAR_TEXTURES);

      elem_types[LP_JIT_LINEAR_CTX_INPUTS] =
      lp->jit_linear_inputs_type =
            LLVMArrayType(linear_elem_ptr_type, LP_MAX_LINEAR_INPUTS);
      elem_types[LP_JIT_LINEAR_CTX_COLOR0] = LLVMPointerType(LLVMInt8TypeInContext(lc), 0);
      elem_types[LP_JIT_LINEAR_CTX_BLEND_COLOR] = LLVMInt32TypeInContext(lc);
      elem_types[LP_JIT_LINEAR_CTX_ALPHA_REF] = LLVMInt8TypeInContext(lc);

      linear_context_type = LLVMStructTypeInContext(lc, elem_types,
                                                    ARRAY_SIZE(elem_types), 0);

      LP_CHECK_MEMBER_OFFSET(struct lp_jit_linear_context, constants,
                             gallivm->target, linear_context_type,
                             LP_JIT_LINEAR_CTX_CONSTANTS);
      LP_CHECK_MEMBER_OFFSET(struct lp_jit_linear_context, tex,
                             gallivm->target, linear_context_type,
                             LP_JIT_LINEAR_CTX_TEX);
      LP_CHECK_MEMBER_OFFSET(struct lp_jit_linear_context, inputs,
                             gallivm->target, linear_context_type,
                             LP_JIT_LINEAR_CTX_INPUTS);
      LP_CHECK_MEMBER_OFFSET(struct lp_jit_linear_context, color0,
                             gallivm->target, linear_context_type,
                             LP_JIT_LINEAR_CTX_COLOR0);
      LP_CHECK_MEMBER_OFFSET(struct lp_jit_linear_context, blend_color,
                             gallivm->target, linear_context_type,
                             LP_JIT_LINEAR_CTX_BLEND_COLOR);
      LP_CHECK_MEMBER_OFFSET(struct lp_jit_linear_context, alpha_ref_value,
                             gallivm->target, linear_context_type,
                             LP_JIT_LINEAR_CTX_ALPHA_REF);
      LP_CHECK_STRUCT_SIZE(struct lp_jit_linear_context,
                           gallivm->target, linear_context_type);

      lp->jit_linear_context_type = linear_context_type;
      lp->jit_linear_context_ptr_type = LLVMPointerType(linear_context_type, 0);
   }

   if (gallivm_debug & GALLIVM_DEBUG_IR) {
      char *str = LLVMPrintModuleToString(gallivm->module);
      fprintf(stderr, "%s", str);
      LLVMDisposeMessage(str);
   }
}


void
lp_jit_screen_cleanup(struct llvmpipe_screen *screen)
{
   /* nothing */
}


bool
lp_jit_screen_init(struct llvmpipe_screen *screen)
{
   return lp_build_init();
}


void
lp_jit_init_types(struct lp_fragment_shader_variant *lp)
{
   if (!lp->jit_context_ptr_type)
      lp_jit_create_types(lp);
}

static void
lp_jit_create_cs_types(struct lp_compute_shader_variant *lp)
{
   struct gallivm_state *gallivm = lp->gallivm;
   LLVMContextRef lc = gallivm->context;

   /* struct lp_jit_cs_thread_data */
   {
      LLVMTypeRef elem_types[LP_JIT_CS_THREAD_DATA_COUNT];
      LLVMTypeRef thread_data_type;

      elem_types[LP_JIT_CS_THREAD_DATA_CACHE] =
            LLVMPointerType(lp_build_format_cache_type(gallivm), 0);

      elem_types[LP_JIT_CS_THREAD_DATA_SHARED] = LLVMPointerType(LLVMInt32TypeInContext(lc), 0);

      elem_types[LP_JIT_CS_THREAD_DATA_PAYLOAD] = LLVMPointerType(LLVMInt8TypeInContext(lc), 0);
      thread_data_type = LLVMStructTypeInContext(lc, elem_types,
                                                 ARRAY_SIZE(elem_types), 0);

      lp->jit_cs_thread_data_type = thread_data_type;
      lp->jit_cs_thread_data_ptr_type = LLVMPointerType(thread_data_type, 0);
   }

   /* struct lp_jit_cs_context */
   {
      LLVMTypeRef elem_types[LP_JIT_CS_CTX_COUNT];
      LLVMTypeRef cs_context_type;

      elem_types[LP_JIT_CS_CTX_KERNEL_ARGS] = LLVMPointerType(LLVMInt8TypeInContext(lc), 0);
      elem_types[LP_JIT_CS_CTX_SHARED_SIZE] = LLVMInt32TypeInContext(lc);

      cs_context_type = LLVMStructTypeInContext(lc, elem_types,
                                             ARRAY_SIZE(elem_types), 0);

      LP_CHECK_MEMBER_OFFSET(struct lp_jit_cs_context, kernel_args,
                             gallivm->target, cs_context_type,
                             LP_JIT_CS_CTX_KERNEL_ARGS);
      LP_CHECK_MEMBER_OFFSET(struct lp_jit_cs_context, shared_size,
                             gallivm->target, cs_context_type,
                             LP_JIT_CS_CTX_SHARED_SIZE);
      LP_CHECK_STRUCT_SIZE(struct lp_jit_cs_context,
                           gallivm->target, cs_context_type);

      lp->jit_cs_context_type = cs_context_type;
      lp->jit_cs_context_ptr_type = LLVMPointerType(cs_context_type, 0);
      lp->jit_resources_type = lp_build_jit_resources_type(gallivm);
      lp->jit_resources_ptr_type = LLVMPointerType(lp->jit_resources_type, 0);
   }

   if (gallivm_debug & GALLIVM_DEBUG_IR) {
      char *str = LLVMPrintModuleToString(gallivm->module);
      fprintf(stderr, "%s", str);
      LLVMDisposeMessage(str);
   }
}

void
lp_jit_init_cs_types(struct lp_compute_shader_variant *lp)
{
   if (!lp->jit_cs_context_ptr_type)
      lp_jit_create_cs_types(lp);
}

void
lp_jit_buffer_from_pipe(struct lp_jit_buffer *jit, const struct pipe_shader_buffer *buffer)
{
   const uint8_t *current_data = NULL;

   /* resource buffer */
   if (buffer->buffer)
      current_data = (uint8_t *)llvmpipe_resource_data(buffer->buffer);

   if (current_data) {
      current_data += buffer->buffer_offset;
      jit->u = (const uint32_t *)current_data;
      jit->num_elements = buffer->buffer_size;
   } else {
      jit->u = NULL;
      jit->num_elements = 0;
   }
}

void
lp_jit_buffer_from_bda(struct lp_jit_buffer *jit, void *mem, size_t size)
{
   const uint8_t *current_data = mem;

   if (current_data) {
      jit->u = (const uint32_t *)current_data;
      jit->num_elements = size;
   } else {
      jit->u = NULL;
      jit->num_elements = 0;
   }
}

void
lp_jit_buffer_from_pipe_const(struct lp_jit_buffer *jit, const struct pipe_constant_buffer *buffer, struct pipe_screen *screen)
{
   uint64_t current_size = buffer->buffer_size;

   const uint8_t *current_data = NULL;
   if (buffer->buffer) {
      /* resource buffer */
      current_data = (uint8_t *)llvmpipe_resource_data(buffer->buffer);
   } else if (buffer->user_buffer) {
      /* user-space buffer */
      current_data = (uint8_t *)buffer->user_buffer;
   }

   if (current_data && current_size >= sizeof(float)) {
      current_data += buffer->buffer_offset;
      jit->f = (const float *)current_data;
      jit->num_elements = DIV_ROUND_UP(current_size, lp_get_constant_buffer_stride(screen));
   } else {
      static const float fake_const_buf[4];
      jit->f = fake_const_buf;
      jit->num_elements = 0;
   }
}

void
lp_jit_texture_from_pipe(struct lp_jit_texture *jit, const struct pipe_sampler_view *view)
{
   struct pipe_resource *res = view->texture;
   struct llvmpipe_resource *lp_tex = llvmpipe_resource(res);

   if (!lp_tex->dt) {
      /* regular texture - setup array of mipmap level offsets */
      unsigned first_level = 0;
      unsigned last_level = 0;

      if (llvmpipe_resource_is_texture(res)) {
         first_level = view->u.tex.first_level;
         last_level = view->u.tex.last_level;
         assert(first_level <= last_level);
         assert(last_level <= res->last_level);
         jit->base = lp_tex->tex_data;
      } else {
         jit->base = lp_tex->data;
      }

      if (LP_PERF & PERF_TEX_MEM) {
         /* use dummy tile memory */
         jit->base = lp_dummy_tile;
         jit->width = TILE_SIZE/8;
         jit->height = TILE_SIZE/8;
         jit->depth = 1;
         jit->first_level = 0;
         jit->last_level = 0;
         jit->mip_offsets[0] = 0;
         jit->mip_offsets[LP_JIT_TEXTURE_SAMPLE_STRIDE] = 0;
         jit->row_stride[0] = 0;
         jit->img_stride[0] = 0;
      } else {
         jit->width = res->width0;
         jit->height = res->height0;
         jit->depth = res->depth0;
         jit->first_level = first_level;
         jit->last_level = last_level;
         jit->mip_offsets[0] = 0;

         if (llvmpipe_resource_is_texture(res)) {
            if (res->nr_samples > 1) {
               jit->last_level = res->nr_samples;
               jit->mip_offsets[LP_JIT_TEXTURE_SAMPLE_STRIDE] = lp_tex->sample_stride;
               jit->row_stride[0] = lp_tex->row_stride[0];
               jit->img_stride[0] = lp_tex->img_stride[0];
            } else {
               for (unsigned j = first_level; j <= last_level; j++) {
                  jit->mip_offsets[j] = lp_tex->mip_offsets[j];
                  jit->row_stride[j] = lp_tex->row_stride[j];
                  jit->img_stride[j] = lp_tex->img_stride[j];
               }
            }

            if (res->target == PIPE_TEXTURE_1D_ARRAY ||
                res->target == PIPE_TEXTURE_2D_ARRAY ||
                res->target == PIPE_TEXTURE_CUBE ||
                res->target == PIPE_TEXTURE_CUBE_ARRAY ||
                (res->target == PIPE_TEXTURE_3D && view->target == PIPE_TEXTURE_2D)) {
               /*
                * For array textures, we don't have first_layer, instead
                * adjust last_layer (stored as depth) plus the mip level
                * offsets (as we have mip-first layout can't just adjust
                * base ptr).  XXX For mip levels, could do something
                * similar.
                */
               jit->depth = view->u.tex.last_layer - view->u.tex.first_layer + 1;
               for (unsigned j = first_level; j <= last_level; j++) {
                  jit->mip_offsets[j] += view->u.tex.first_layer *
                                             lp_tex->img_stride[j];
               }
               if (view->target == PIPE_TEXTURE_CUBE ||
                   view->target == PIPE_TEXTURE_CUBE_ARRAY) {
                  assert(jit->depth % 6 == 0);
               }
               assert(view->u.tex.first_layer <= view->u.tex.last_layer);
               if (res->target == PIPE_TEXTURE_3D)
                  assert(view->u.tex.last_layer < res->depth0);
               else
                  assert(view->u.tex.last_layer < res->array_size);
            }
         } else {
            /*
             * For tex2d_from_buf, adjust width and height with application
             * values. If is_tex2d_from_buf is false (1D images),
             * adjust using size value (stored as width).
             */
            unsigned view_blocksize = util_format_get_blocksize(view->format);

            jit->mip_offsets[0] = 0;
            jit->img_stride[0] = 0;

            /* If it's not a 2D texture view of a buffer, adjust using size. */
            if (!view->is_tex2d_from_buf) {
               /* everything specified in number of elements here. */
               jit->width = view->u.buf.size / view_blocksize;
               jit->row_stride[0] = 0;

               /* Adjust base pointer with offset. */
               jit->base = (uint8_t *)jit->base + view->u.buf.offset;

               /* XXX Unsure if we need to sanitize parameters? */
               assert(view->u.buf.offset + view->u.buf.size <= res->width0);
            } else {
               jit->width = view->u.tex2d_from_buf.width;
               jit->height = view->u.tex2d_from_buf.height;
               jit->row_stride[0] = view->u.tex2d_from_buf.row_stride * view_blocksize;

               jit->base = (uint8_t *)jit->base +
                  view->u.tex2d_from_buf.offset * view_blocksize;
            }
         }
      }
   } else {
      /* display target texture/surface */
      jit->base = llvmpipe_resource_map(res, 0, 0, LP_TEX_USAGE_READ);
      jit->row_stride[0] = lp_tex->row_stride[0];
      jit->img_stride[0] = lp_tex->img_stride[0];
      jit->mip_offsets[0] = 0;
      jit->width = res->width0;
      jit->height = res->height0;
      jit->depth = res->depth0;
      jit->first_level = jit->last_level = 0;
      if (res->nr_samples > 1)
         jit->last_level = res->nr_samples;
      assert(jit->base);
   }
}

void
lp_jit_texture_buffer_from_bda(struct lp_jit_texture *jit, void *mem, size_t size, enum pipe_format format)
{
   jit->base = mem;

   if (LP_PERF & PERF_TEX_MEM) {
      /* use dummy tile memory */
      jit->base = lp_dummy_tile;
      jit->width = TILE_SIZE/8;
      jit->height = TILE_SIZE/8;
      jit->depth = 1;
      jit->first_level = 0;
      jit->last_level = 0;
      jit->mip_offsets[0] = 0;
      jit->row_stride[0] = 0;
      jit->img_stride[0] = 0;
   } else {
      jit->height = 1;
      jit->depth = 1;
      jit->first_level = 0;
      jit->last_level = 0;

      /*
       * For buffers, we don't have "offset", instead adjust
       * the size (stored as width) plus the base pointer.
       */
      const unsigned view_blocksize = util_format_get_blocksize(format);
      /* probably don't really need to fill that out */
      jit->mip_offsets[0] = 0;
      jit->row_stride[0] = 0;
      jit->img_stride[0] = 0;

      /* everything specified in number of elements here. */
      jit->width = size / view_blocksize;
   }
}

void
lp_jit_sampler_from_pipe(struct lp_jit_sampler *jit, const struct pipe_sampler_state *sampler)
{
   jit->min_lod = sampler->min_lod;
   jit->max_lod = sampler->max_lod;
   jit->lod_bias = sampler->lod_bias;
   jit->max_aniso = sampler->max_anisotropy;
   COPY_4V(jit->border_color, sampler->border_color.f);
}

void
lp_jit_image_from_pipe(struct lp_jit_image *jit, const struct pipe_image_view *view)
{
   struct pipe_resource *res = view->resource;
   struct llvmpipe_resource *lp_res = llvmpipe_resource(res);

   if (!lp_res->dt) {
      /* regular texture - setup array of mipmap level offsets */
      if (llvmpipe_resource_is_texture(res)) {
         jit->base = lp_res->tex_data;
      } else {
         jit->base = lp_res->data;
      }

      jit->width = res->width0;
      jit->height = res->height0;
      jit->depth = res->depth0;
      jit->num_samples = res->nr_samples;

      if (llvmpipe_resource_is_texture(res)) {
         uint32_t mip_offset = lp_res->mip_offsets[view->u.tex.level];

         jit->width = u_minify(jit->width, view->u.tex.level);
         jit->height = u_minify(jit->height, view->u.tex.level);

         if (res->target == PIPE_TEXTURE_1D_ARRAY ||
             res->target == PIPE_TEXTURE_2D_ARRAY ||
             res->target == PIPE_TEXTURE_3D ||
             res->target == PIPE_TEXTURE_CUBE ||
             res->target == PIPE_TEXTURE_CUBE_ARRAY) {
            /*
             * For array textures, we don't have first_layer, instead
             * adjust last_layer (stored as depth) plus the mip level offsets
             * (as we have mip-first layout can't just adjust base ptr).
             * XXX For mip levels, could do something similar.
             */
            jit->depth = view->u.tex.last_layer - view->u.tex.first_layer + 1;
            mip_offset += view->u.tex.first_layer * lp_res->img_stride[view->u.tex.level];
         } else
            jit->depth = u_minify(jit->depth, view->u.tex.level);

         jit->row_stride = lp_res->row_stride[view->u.tex.level];
         jit->img_stride = lp_res->img_stride[view->u.tex.level];
         jit->sample_stride = lp_res->sample_stride;
         jit->base = (uint8_t *)jit->base + mip_offset;
      } else {
         unsigned image_blocksize = util_format_get_blocksize(view->format);

         jit->img_stride = 0;

         /* If it's not a 2D image view of a buffer, adjust using size. */
         if (!(view->access & PIPE_IMAGE_ACCESS_TEX2D_FROM_BUFFER)) {
            /* everything specified in number of elements here. */
            jit->width = view->u.buf.size / image_blocksize;
            jit->row_stride = 0;

            /* Adjust base pointer with offset. */
            jit->base = (uint8_t *)jit->base + view->u.buf.offset;

            /* XXX Unsure if we need to sanitize parameters? */
            assert(view->u.buf.offset + view->u.buf.size <= res->width0);
         } else {
            jit->width = view->u.tex2d_from_buf.width;
            jit->height = view->u.tex2d_from_buf.height;
            jit->row_stride = view->u.tex2d_from_buf.row_stride * image_blocksize;

            jit->base = (uint8_t *)jit->base +
               view->u.tex2d_from_buf.offset * image_blocksize;
         }
      }
   }
}

void
lp_jit_image_buffer_from_bda(struct lp_jit_image *jit, void *mem, size_t size, enum pipe_format format)
{
   jit->base = mem;

   jit->height = 1;
   jit->depth = 1;
   jit->num_samples = 1;

   unsigned view_blocksize = util_format_get_blocksize(format);
   jit->width = size / view_blocksize;
}
