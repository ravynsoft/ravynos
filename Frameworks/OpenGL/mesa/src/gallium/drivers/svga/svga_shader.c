/**********************************************************
 * Copyright 2008-2023 VMware, Inc.  All rights reserved.
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without
 * restriction, including without limitation the rights to use, copy,
 * modify, merge, publish, distribute, sublicense, and/or sell copies
 * of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
 * BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
 * ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 **********************************************************/

#include "util/u_bitmask.h"
#include "util/u_memory.h"
#include "util/format/u_format.h"
#include "svga_context.h"
#include "svga_cmd.h"
#include "svga_format.h"
#include "svga_shader.h"
#include "svga_tgsi.h"
#include "svga_resource_texture.h"
#include "VGPU10ShaderTokens.h"

#include "compiler/nir/nir.h"
#include "compiler/glsl/gl_nir.h"
#include "nir/nir_to_tgsi.h"


/**
 * This bit isn't really used anywhere.  It only serves to help
 * generate a unique "signature" for the vertex shader output bitmask.
 * Shader input/output signatures are used to resolve shader linking
 * issues.
 */
#define FOG_GENERIC_BIT (((uint64_t) 1) << 63)


/**
 * Use the shader info to generate a bitmask indicating which generic
 * inputs are used by the shader.  A set bit indicates that GENERIC[i]
 * is used.
 */
uint64_t
svga_get_generic_inputs_mask(const struct tgsi_shader_info *info)
{
   unsigned i;
   uint64_t mask = 0x0;

   for (i = 0; i < info->num_inputs; i++) {
      if (info->input_semantic_name[i] == TGSI_SEMANTIC_GENERIC) {
         unsigned j = info->input_semantic_index[i];
         assert(j < sizeof(mask) * 8);
         mask |= ((uint64_t) 1) << j;
      }
   }

   return mask;
}


/**
 * Scan shader info to return a bitmask of written outputs.
 */
uint64_t
svga_get_generic_outputs_mask(const struct tgsi_shader_info *info)
{
   unsigned i;
   uint64_t mask = 0x0;

   for (i = 0; i < info->num_outputs; i++) {
      switch (info->output_semantic_name[i]) {
      case TGSI_SEMANTIC_GENERIC:
         {
            unsigned j = info->output_semantic_index[i];
            assert(j < sizeof(mask) * 8);
            mask |= ((uint64_t) 1) << j;
         }
         break;
      case TGSI_SEMANTIC_FOG:
         mask |= FOG_GENERIC_BIT;
         break;
      }
   }

   return mask;
}



/**
 * Given a mask of used generic variables (as returned by the above functions)
 * fill in a table which maps those indexes to small integers.
 * This table is used by the remap_generic_index() function in
 * svga_tgsi_decl_sm30.c
 * Example: if generics_mask = binary(1010) it means that GENERIC[1] and
 * GENERIC[3] are used.  The remap_table will contain:
 *   table[1] = 0;
 *   table[3] = 1;
 * The remaining table entries will be filled in with the next unused
 * generic index (in this example, 2).
 */
void
svga_remap_generics(uint64_t generics_mask,
                    int8_t remap_table[MAX_GENERIC_VARYING])
{
   /* Note texcoord[0] is reserved so start at 1 */
   unsigned count = 1, i;

   for (i = 0; i < MAX_GENERIC_VARYING; i++) {
      remap_table[i] = -1;
   }

   /* for each bit set in generic_mask */
   while (generics_mask) {
      unsigned index = ffsll(generics_mask) - 1;
      remap_table[index] = count++;
      generics_mask &= ~((uint64_t) 1 << index);
   }
}


/**
 * Use the generic remap table to map a TGSI generic varying variable
 * index to a small integer.  If the remapping table doesn't have a
 * valid value for the given index (the table entry is -1) it means
 * the fragment shader doesn't use that VS output.  Just allocate
 * the next free value in that case.  Alternately, we could cull
 * VS instructions that write to register, or replace the register
 * with a dummy temp register.
 * XXX TODO: we should do one of the later as it would save precious
 * texcoord registers.
 */
int
svga_remap_generic_index(int8_t remap_table[MAX_GENERIC_VARYING],
                         int generic_index)
{
   assert(generic_index < MAX_GENERIC_VARYING);

   if (generic_index >= MAX_GENERIC_VARYING) {
      /* just don't return a random/garbage value */
      generic_index = MAX_GENERIC_VARYING - 1;
   }

   if (remap_table[generic_index] == -1) {
      /* This is a VS output that has no matching PS input.  Find a
       * free index.
       */
      int i, max = 0;
      for (i = 0; i < MAX_GENERIC_VARYING; i++) {
         max = MAX2(max, remap_table[i]);
      }
      remap_table[generic_index] = max + 1;
   }

   return remap_table[generic_index];
}

static const enum pipe_swizzle copy_alpha[PIPE_SWIZZLE_MAX] = {
   PIPE_SWIZZLE_X,
   PIPE_SWIZZLE_Y,
   PIPE_SWIZZLE_Z,
   PIPE_SWIZZLE_W,
   PIPE_SWIZZLE_0,
   PIPE_SWIZZLE_1,
   PIPE_SWIZZLE_NONE
};

static const enum pipe_swizzle set_alpha[PIPE_SWIZZLE_MAX] = {
   PIPE_SWIZZLE_X,
   PIPE_SWIZZLE_Y,
   PIPE_SWIZZLE_Z,
   PIPE_SWIZZLE_1,
   PIPE_SWIZZLE_0,
   PIPE_SWIZZLE_1,
   PIPE_SWIZZLE_NONE
};

static const enum pipe_swizzle set_000X[PIPE_SWIZZLE_MAX] = {
   PIPE_SWIZZLE_0,
   PIPE_SWIZZLE_0,
   PIPE_SWIZZLE_0,
   PIPE_SWIZZLE_X,
   PIPE_SWIZZLE_0,
   PIPE_SWIZZLE_1,
   PIPE_SWIZZLE_NONE
};

static const enum pipe_swizzle set_XXXX[PIPE_SWIZZLE_MAX] = {
   PIPE_SWIZZLE_X,
   PIPE_SWIZZLE_X,
   PIPE_SWIZZLE_X,
   PIPE_SWIZZLE_X,
   PIPE_SWIZZLE_0,
   PIPE_SWIZZLE_1,
   PIPE_SWIZZLE_NONE
};

static const enum pipe_swizzle set_XXX1[PIPE_SWIZZLE_MAX] = {
   PIPE_SWIZZLE_X,
   PIPE_SWIZZLE_X,
   PIPE_SWIZZLE_X,
   PIPE_SWIZZLE_1,
   PIPE_SWIZZLE_0,
   PIPE_SWIZZLE_1,
   PIPE_SWIZZLE_NONE
};

static const enum pipe_swizzle set_XXXY[PIPE_SWIZZLE_MAX] = {
   PIPE_SWIZZLE_X,
   PIPE_SWIZZLE_X,
   PIPE_SWIZZLE_X,
   PIPE_SWIZZLE_Y,
   PIPE_SWIZZLE_0,
   PIPE_SWIZZLE_1,
   PIPE_SWIZZLE_NONE
};

static const enum pipe_swizzle set_YYYY[PIPE_SWIZZLE_MAX] = {
   PIPE_SWIZZLE_Y,
   PIPE_SWIZZLE_Y,
   PIPE_SWIZZLE_Y,
   PIPE_SWIZZLE_Y,
   PIPE_SWIZZLE_0,
   PIPE_SWIZZLE_1,
   PIPE_SWIZZLE_NONE
};


static VGPU10_RESOURCE_RETURN_TYPE
vgpu10_return_type(enum pipe_format format)
{
   if (util_format_is_unorm(format))
      return VGPU10_RETURN_TYPE_UNORM;
   else if (util_format_is_snorm(format))
      return VGPU10_RETURN_TYPE_SNORM;
   else if (util_format_is_pure_uint(format))
      return VGPU10_RETURN_TYPE_UINT;
   else if (util_format_is_pure_sint(format))
      return VGPU10_RETURN_TYPE_SINT;
   else if (util_format_is_float(format))
      return VGPU10_RETURN_TYPE_FLOAT;
   else
      return VGPU10_RETURN_TYPE_MAX;
}


/**
 * A helper function to return TRUE if the specified format
 * is a supported format for sample_c instruction.
 */
static bool
isValidSampleCFormat(enum pipe_format format)
{
   return util_format_is_depth_or_stencil(format);
}


/**
 * Initialize the shader-neutral fields of svga_compile_key from context
 * state.  This is basically the texture-related state.
 */
void
svga_init_shader_key_common(const struct svga_context *svga,
                            enum pipe_shader_type shader_type,
                            const struct svga_shader *shader,
                            struct svga_compile_key *key)
{
   unsigned i, idx = 0;
   unsigned sampler_slots = 0;

   assert(shader_type < ARRAY_SIZE(svga->curr.num_sampler_views));

   /* In case the number of samplers and sampler_views doesn't match,
    * loop over the upper of the two counts.
    */
   key->num_textures = MAX2(svga->curr.num_sampler_views[shader_type],
                            svga->curr.num_samplers[shader_type]);

   if (!shader->info.uses_samplers)
      key->num_textures = 0;

   key->num_samplers = 0;

   /* Set sampler_state_mapping only if GL43 is supported and
    * the number of samplers exceeds SVGA limit or the sampler state
    * mapping env is set.
    */
   bool sampler_state_mapping =
      svga_use_sampler_state_mapping(svga, svga->curr.num_samplers[shader_type]);

   key->sampler_state_mapping =
      key->num_textures && sampler_state_mapping ? 1 : 0;

   for (i = 0; i < key->num_textures; i++) {
      struct pipe_sampler_view *view = svga->curr.sampler_views[shader_type][i];
      const struct svga_sampler_state
         *sampler = svga->curr.sampler[shader_type][i];

      if (view) {
         assert(view->texture);

         enum pipe_texture_target target = view->target;
         assert(target < (1 << 4)); /* texture_target:4 */

	 key->tex[i].target = target;
	 key->tex[i].sampler_return_type = vgpu10_return_type(view->format);
	 key->tex[i].sampler_view = 1;

         /* 1D/2D array textures with one slice and cube map array textures
          * with one cube are treated as non-arrays by the SVGA3D device.
          * Set the is_array flag only if we know that we have more than 1
          * element.  This will be used to select shader instruction/resource
          * types during shader translation.
          */
         switch (target) {
         case PIPE_TEXTURE_1D_ARRAY:
         case PIPE_TEXTURE_2D_ARRAY:
            key->tex[i].is_array = view->texture->array_size > 1;
            break;
         case PIPE_TEXTURE_CUBE_ARRAY:
            key->tex[i].is_array = view->texture->array_size > 6;
            break;
         default:
            ; /* nothing / silence compiler warning */
         }

         assert(view->texture->nr_samples < (1 << 5)); /* 5-bit field */
         key->tex[i].num_samples = view->texture->nr_samples;

         const enum pipe_swizzle *swizzle_tab;
         if (target == PIPE_BUFFER) {
            SVGA3dSurfaceFormat svga_format;
            unsigned tf_flags;

            assert(view->texture->target == PIPE_BUFFER);

            /* Apply any special swizzle mask for the view format if needed */

            svga_translate_texture_buffer_view_format(view->format,
                                                      &svga_format, &tf_flags);
            if (tf_flags & TF_000X)
               swizzle_tab = set_000X;
            else if (tf_flags & TF_XXXX)
               swizzle_tab = set_XXXX;
            else if (tf_flags & TF_XXX1)
               swizzle_tab = set_XXX1;
            else if (tf_flags & TF_XXXY)
               swizzle_tab = set_XXXY;
            else
               swizzle_tab = copy_alpha;
         }
         else {
            /* If we have a non-alpha view into an svga3d surface with an
             * alpha channel, then explicitly set the alpha channel to 1
             * when sampling. Note that we need to check the
             * actual device format to cover also imported surface cases.
             */
            swizzle_tab =
               (!util_format_has_alpha(view->format) &&
                svga_texture_device_format_has_alpha(view->texture)) ?
                set_alpha : copy_alpha;

            if (view->texture->format == PIPE_FORMAT_DXT1_RGB ||
                view->texture->format == PIPE_FORMAT_DXT1_SRGB)
               swizzle_tab = set_alpha;

            if (view->format == PIPE_FORMAT_X24S8_UINT ||
                view->format == PIPE_FORMAT_X32_S8X24_UINT)
               swizzle_tab = set_YYYY;

            /* Save the compare function as we need to handle
             * depth compare in the shader.
             */
            key->tex[i].compare_mode = sampler->compare_mode;
            key->tex[i].compare_func = sampler->compare_func;

            /* Set the compare_in_shader bit if the view format
             * is not a supported format for shadow compare.
             * In this case, we'll do the comparison in the shader.
             */
            if ((sampler->compare_mode == PIPE_TEX_COMPARE_R_TO_TEXTURE) &&
                !isValidSampleCFormat(view->format)) {
               key->tex[i].compare_in_shader = true;
            }
         }

         key->tex[i].swizzle_r = swizzle_tab[view->swizzle_r];
         key->tex[i].swizzle_g = swizzle_tab[view->swizzle_g];
         key->tex[i].swizzle_b = swizzle_tab[view->swizzle_b];
         key->tex[i].swizzle_a = swizzle_tab[view->swizzle_a];
      }
      else {
	 key->tex[i].sampler_view = 0;
      }

      if (sampler) {
         if (!sampler->normalized_coords) {
            if (view) {
               assert(idx < (1 << 5));  /* width_height_idx:5 bitfield */
               key->tex[i].width_height_idx = idx++;
	    }
            key->tex[i].unnormalized = true;
            ++key->num_unnormalized_coords;

            if (sampler->magfilter == SVGA3D_TEX_FILTER_NEAREST ||
                sampler->minfilter == SVGA3D_TEX_FILTER_NEAREST) {
                key->tex[i].texel_bias = true;
            }
         }

         if (!sampler_state_mapping) {
            /* Use the same index if sampler state mapping is not supported */
            key->tex[i].sampler_index = i;
            key->num_samplers = i + 1;
         }
         else {

            /* The current samplers list can have redundant entries.
             * In order to allow the number of bound samplers within the
             * max limit supported by SVGA, we'll recreate the list with
             * unique sampler state objects only.
             */

            /* Check to see if this sampler is already on the list.
             * If so, set the sampler index of this sampler to the
             * same sampler index.
             */
            for (unsigned j = 0; j <= i; j++) {
               if (svga->curr.sampler[shader_type][j] == sampler) {

                  if (!(sampler_slots & (1 << j))) {

                     /* if this sampler is not added to the new list yet,
                      * set its sampler index to the next sampler index,
                      * increment the sampler count, and mark this
                      * sampler as added to the list.
                      */

                     unsigned next_index =
                        MIN2(key->num_samplers, SVGA3D_DX_MAX_SAMPLERS-1);

                     key->tex[i].sampler_index = next_index;
                     key->num_samplers = next_index + 1;

                     if (sampler->compare_mode == PIPE_TEX_COMPARE_R_TO_TEXTURE) {
                        /* reserve one slot for the alternate sampler */
                        key->num_samplers++;
                     }

                     sampler_slots |= (1 << j);
                  }
                  else {
                     key->tex[i].sampler_index = key->tex[j].sampler_index;
                  }
                  break;
               }
            }
         }
      }
   }

   if (svga_have_gl43(svga)) {

      /* Save info about which constant buffers are to be viewed
       * as srv raw buffers in the shader key.
       */
      if (shader->info.const_buffers_declared &
          svga->state.raw_constbufs[shader_type]) {
         key->raw_constbufs = svga->state.raw_constbufs[shader_type] &
                              shader->info.const_buffers_declared;
      }

      /* beginning index for srv for raw constant buffers */
      key->srv_raw_constbuf_index = PIPE_MAX_SAMPLERS;

      if (shader->info.uses_images || shader->info.uses_hw_atomic ||
          shader->info.uses_shader_buffers) {

         /* Save the uavSpliceIndex which is the index used for the first uav
          * in the draw pipeline. For compute, uavSpliceIndex is always 0.
          */
         if (shader_type != PIPE_SHADER_COMPUTE)
            key->uav_splice_index = svga->state.hw_draw.uavSpliceIndex;

         unsigned uav_splice_index = key->uav_splice_index;

         /* Also get the texture data type to be used in the uav declaration */
         const struct svga_image_view *cur_image_view =
            &svga->curr.image_views[shader_type][0];

         for (unsigned i = 0; i < ARRAY_SIZE(svga->curr.image_views[shader_type]);
              i++, cur_image_view++) {

            struct pipe_resource *resource = cur_image_view->desc.resource;

            if (resource) {
               key->images[i].return_type =
                  svga_get_texture_datatype(cur_image_view->desc.format);

               key->images[i].is_array = resource->array_size > 1;

               /* Save the image resource target in the shader key because
                * for single layer image view, the resource target in the
                * tgsi shader is changed to a different texture target.
                */
               key->images[i].resource_target = resource->target;
               if (resource->target == PIPE_TEXTURE_3D ||
                   resource->target == PIPE_TEXTURE_1D_ARRAY ||
                   resource->target == PIPE_TEXTURE_2D_ARRAY ||
                   resource->target == PIPE_TEXTURE_CUBE ||
                   resource->target == PIPE_TEXTURE_CUBE_ARRAY) {
                  key->images[i].is_single_layer =
                     cur_image_view->desc.u.tex.first_layer ==
                     cur_image_view->desc.u.tex.last_layer;
               }

               key->images[i].uav_index = cur_image_view->uav_index + uav_splice_index;
            }
            else
               key->images[i].uav_index = SVGA3D_INVALID_ID;
         }

         const struct svga_shader_buffer *cur_sbuf =
            &svga->curr.shader_buffers[shader_type][0];

         /* Save info about which shader buffers are to be viewed
          * as srv raw buffers in the shader key.
          */
         if (shader->info.shader_buffers_declared &
             svga->state.raw_shaderbufs[shader_type]) {
            key->raw_shaderbufs = svga->state.raw_shaderbufs[shader_type] &
                                  shader->info.shader_buffers_declared;
            key->srv_raw_shaderbuf_index = key->srv_raw_constbuf_index +
		                           SVGA_MAX_CONST_BUFS;
         }

         for (unsigned i = 0; i < ARRAY_SIZE(svga->curr.shader_buffers[shader_type]);
              i++, cur_sbuf++) {

            if (cur_sbuf->resource && (!(key->raw_shaderbufs & (1 << i))))
               key->shader_buf_uav_index[i] = cur_sbuf->uav_index + uav_splice_index;
            else
               key->shader_buf_uav_index[i] = SVGA3D_INVALID_ID;
         }

         const struct svga_shader_buffer *cur_buf = &svga->curr.atomic_buffers[0];

         for (unsigned i = 0; i < ARRAY_SIZE(svga->curr.atomic_buffers);
              i++, cur_buf++) {

            if (cur_buf->resource)
               key->atomic_buf_uav_index[i] = cur_buf->uav_index + uav_splice_index;
            else
               key->atomic_buf_uav_index[i] = SVGA3D_INVALID_ID;
         }

         key->image_size_used = shader->info.uses_image_size;
      }

   }

   key->clamp_vertex_color = svga->curr.rast ?
                             svga->curr.rast->templ.clamp_vertex_color : 0;
}


/** Search for a compiled shader variant with the same compile key */
struct svga_shader_variant *
svga_search_shader_key(const struct svga_shader *shader,
                       const struct svga_compile_key *key)
{
   struct svga_shader_variant *variant = shader->variants;

   assert(key);

   for ( ; variant; variant = variant->next) {
      if (svga_compile_keys_equal(key, &variant->key))
         return variant;
   }
   return NULL;
}

/** Search for a shader with the same token key */
struct svga_shader *
svga_search_shader_token_key(struct svga_shader *pshader,
                             const struct svga_token_key *key)
{
   struct svga_shader *shader = pshader;

   assert(key);

   for ( ; shader; shader = shader->next) {
      if (memcmp(key, &shader->token_key, sizeof(struct svga_token_key)) == 0)
         return shader;
   }
   return NULL;
}

/**
 * Helper function to define a gb shader for non-vgpu10 device
 */
static enum pipe_error
define_gb_shader_vgpu9(struct svga_context *svga,
                       struct svga_shader_variant *variant,
                       unsigned codeLen)
{
   struct svga_winsys_screen *sws = svga_screen(svga->pipe.screen)->sws;
   enum pipe_error ret;

   /**
    * Create gb memory for the shader and upload the shader code.
    * Kernel module will allocate an id for the shader and issue
    * the DefineGBShader command.
    */
   variant->gb_shader = sws->shader_create(sws, variant->type,
                                           variant->tokens, codeLen);

   svga->hud.shader_mem_used += codeLen;

   if (!variant->gb_shader)
      return PIPE_ERROR_OUT_OF_MEMORY;

   ret = SVGA3D_BindGBShader(svga->swc, variant->gb_shader);

   return ret;
}

/**
 * Helper function to define a gb shader for vgpu10 device
 */
static enum pipe_error
define_gb_shader_vgpu10(struct svga_context *svga,
                        struct svga_shader_variant *variant,
                        unsigned codeLen)
{
   struct svga_winsys_context *swc = svga->swc;
   enum pipe_error ret;
   unsigned len = codeLen + variant->signatureLen;

   /**
    * Shaders in VGPU10 enabled device reside in the device COTable.
    * SVGA driver will allocate an integer ID for the shader and
    * issue DXDefineShader and DXBindShader commands.
    */
   variant->id = util_bitmask_add(svga->shader_id_bm);
   if (variant->id == UTIL_BITMASK_INVALID_INDEX) {
      return PIPE_ERROR_OUT_OF_MEMORY;
   }

   /* Create gb memory for the shader and upload the shader code */
   variant->gb_shader = swc->shader_create(swc,
                                           variant->id, variant->type,
                                           variant->tokens, codeLen,
                                           variant->signature,
                                           variant->signatureLen);

   svga->hud.shader_mem_used += len;

   if (!variant->gb_shader) {
      /* Free the shader ID */
      assert(variant->id != UTIL_BITMASK_INVALID_INDEX);
      goto fail_no_allocation;
   }

   /**
    * Since we don't want to do any flush within state emission to avoid
    * partial state in a command buffer, it's important to make sure that
    * there is enough room to send both the DXDefineShader & DXBindShader
    * commands in the same command buffer. So let's send both
    * commands in one command reservation. If it fails, we'll undo
    * the shader creation and return an error.
    */
   ret = SVGA3D_vgpu10_DefineAndBindShader(swc, variant->gb_shader,
                                           variant->id, variant->type,
                                           len);

   if (ret != PIPE_OK)
      goto fail;

   return PIPE_OK;

fail:
   swc->shader_destroy(swc, variant->gb_shader);
   variant->gb_shader = NULL;

fail_no_allocation:
   util_bitmask_clear(svga->shader_id_bm, variant->id);
   variant->id = UTIL_BITMASK_INVALID_INDEX;

   return PIPE_ERROR_OUT_OF_MEMORY;
}

/**
 * Issue the SVGA3D commands to define a new shader.
 * \param variant  contains the shader tokens, etc.  The result->id field will
 *                 be set here.
 */
enum pipe_error
svga_define_shader(struct svga_context *svga,
                   struct svga_shader_variant *variant)
{
   unsigned codeLen = variant->nr_tokens * sizeof(variant->tokens[0]);
   enum pipe_error ret;

   SVGA_STATS_TIME_PUSH(svga_sws(svga), SVGA_STATS_TIME_DEFINESHADER);

   variant->id = UTIL_BITMASK_INVALID_INDEX;

   if (svga_have_gb_objects(svga)) {
      if (svga_have_vgpu10(svga))
         ret = define_gb_shader_vgpu10(svga, variant, codeLen);
      else
         ret = define_gb_shader_vgpu9(svga, variant, codeLen);
   }
   else {
      /* Allocate an integer ID for the shader */
      variant->id = util_bitmask_add(svga->shader_id_bm);
      if (variant->id == UTIL_BITMASK_INVALID_INDEX) {
         ret = PIPE_ERROR_OUT_OF_MEMORY;
         goto done;
      }

      /* Issue SVGA3D device command to define the shader */
      ret = SVGA3D_DefineShader(svga->swc,
                                variant->id,
                                variant->type,
                                variant->tokens,
                                codeLen);
      if (ret != PIPE_OK) {
         /* free the ID */
         assert(variant->id != UTIL_BITMASK_INVALID_INDEX);
         util_bitmask_clear(svga->shader_id_bm, variant->id);
         variant->id = UTIL_BITMASK_INVALID_INDEX;
      }
   }

done:
   SVGA_STATS_TIME_POP(svga_sws(svga));
   return ret;
}


/**
 * Issue the SVGA3D commands to set/bind a shader.
 * \param result  the shader to bind.
 */
enum pipe_error
svga_set_shader(struct svga_context *svga,
                SVGA3dShaderType type,
                struct svga_shader_variant *variant)
{
   enum pipe_error ret;
   unsigned id = variant ? variant->id : SVGA3D_INVALID_ID;

   assert(type == SVGA3D_SHADERTYPE_VS ||
          type == SVGA3D_SHADERTYPE_GS ||
          type == SVGA3D_SHADERTYPE_PS ||
          type == SVGA3D_SHADERTYPE_HS ||
          type == SVGA3D_SHADERTYPE_DS ||
          type == SVGA3D_SHADERTYPE_CS);

   if (svga_have_gb_objects(svga)) {
      struct svga_winsys_gb_shader *gbshader =
         variant ? variant->gb_shader : NULL;

      if (svga_have_vgpu10(svga))
         ret = SVGA3D_vgpu10_SetShader(svga->swc, type, gbshader, id);
      else
         ret = SVGA3D_SetGBShader(svga->swc, type, gbshader);
   }
   else {
      ret = SVGA3D_SetShader(svga->swc, type, id);
   }

   return ret;
}


struct svga_shader_variant *
svga_new_shader_variant(struct svga_context *svga, enum pipe_shader_type type)
{
   struct svga_shader_variant *variant;

   switch (type) {
   case PIPE_SHADER_FRAGMENT:
      variant = CALLOC(1, sizeof(struct svga_fs_variant));
      break;
   case PIPE_SHADER_GEOMETRY:
      variant = CALLOC(1, sizeof(struct svga_gs_variant));
      break;
   case PIPE_SHADER_VERTEX:
      variant = CALLOC(1, sizeof(struct svga_vs_variant));
      break;
   case PIPE_SHADER_TESS_EVAL:
      variant = CALLOC(1, sizeof(struct svga_tes_variant));
      break;
   case PIPE_SHADER_TESS_CTRL:
      variant = CALLOC(1, sizeof(struct svga_tcs_variant));
      break;
   case PIPE_SHADER_COMPUTE:
      variant = CALLOC(1, sizeof(struct svga_cs_variant));
      break;
   default:
      return NULL;
   }

   if (variant) {
      variant->type = svga_shader_type(type);
      svga->hud.num_shaders++;
   }
   return variant;
}


void
svga_destroy_shader_variant(struct svga_context *svga,
                            struct svga_shader_variant *variant)
{
   if (svga_have_gb_objects(svga) && variant->gb_shader) {
      if (svga_have_vgpu10(svga)) {
         struct svga_winsys_context *swc = svga->swc;
         swc->shader_destroy(swc, variant->gb_shader);
         SVGA_RETRY(svga, SVGA3D_vgpu10_DestroyShader(svga->swc, variant->id));
         util_bitmask_clear(svga->shader_id_bm, variant->id);
      }
      else {
         struct svga_winsys_screen *sws = svga_screen(svga->pipe.screen)->sws;
         sws->shader_destroy(sws, variant->gb_shader);
      }
      variant->gb_shader = NULL;
   }
   else {
      if (variant->id != UTIL_BITMASK_INVALID_INDEX) {
         SVGA_RETRY(svga, SVGA3D_DestroyShader(svga->swc, variant->id,
                                               variant->type));
         util_bitmask_clear(svga->shader_id_bm, variant->id);
      }
   }

   FREE(variant->signature);
   FREE((unsigned *)variant->tokens);
   FREE(variant);

   svga->hud.num_shaders--;
}

/*
 * Rebind shaders.
 * Called at the beginning of every new command buffer to ensure that
 * shaders are properly paged-in. Instead of sending the SetShader
 * command, this function sends a private allocation command to
 * page in a shader. This avoids emitting redundant state to the device
 * just to page in a resource.
 */
enum pipe_error
svga_rebind_shaders(struct svga_context *svga)
{
   struct svga_winsys_context *swc = svga->swc;
   struct svga_hw_draw_state *hw = &svga->state.hw_draw;
   enum pipe_error ret;

   assert(svga_have_vgpu10(svga));

   /**
    * If the underlying winsys layer does not need resource rebinding,
    * just clear the rebind flags and return.
    */
   if (swc->resource_rebind == NULL) {
      svga->rebind.flags.vs = 0;
      svga->rebind.flags.gs = 0;
      svga->rebind.flags.fs = 0;
      svga->rebind.flags.tcs = 0;
      svga->rebind.flags.tes = 0;

      return PIPE_OK;
   }

   if (svga->rebind.flags.vs && hw->vs && hw->vs->gb_shader) {
      ret = swc->resource_rebind(swc, NULL, hw->vs->gb_shader, SVGA_RELOC_READ);
      if (ret != PIPE_OK)
         return ret;
   }
   svga->rebind.flags.vs = 0;

   if (svga->rebind.flags.gs && hw->gs && hw->gs->gb_shader) {
      ret = swc->resource_rebind(swc, NULL, hw->gs->gb_shader, SVGA_RELOC_READ);
      if (ret != PIPE_OK)
         return ret;
   }
   svga->rebind.flags.gs = 0;

   if (svga->rebind.flags.fs && hw->fs && hw->fs->gb_shader) {
      ret = swc->resource_rebind(swc, NULL, hw->fs->gb_shader, SVGA_RELOC_READ);
      if (ret != PIPE_OK)
         return ret;
   }
   svga->rebind.flags.fs = 0;

   if (svga->rebind.flags.tcs && hw->tcs && hw->tcs->gb_shader) {
      ret = swc->resource_rebind(swc, NULL, hw->tcs->gb_shader, SVGA_RELOC_READ);
      if (ret != PIPE_OK)
         return ret;
   }
   svga->rebind.flags.tcs = 0;

   if (svga->rebind.flags.tes && hw->tes && hw->tes->gb_shader) {
      ret = swc->resource_rebind(swc, NULL, hw->tes->gb_shader, SVGA_RELOC_READ);
      if (ret != PIPE_OK)
         return ret;
   }
   svga->rebind.flags.tes = 0;

   return PIPE_OK;
}


/**
 * Helper function to create a shader object.
 */
struct svga_shader *
svga_create_shader(struct pipe_context *pipe,
                   const struct pipe_shader_state *templ,
                   enum pipe_shader_type stage,
                   unsigned shader_structlen)
{
   struct svga_context *svga = svga_context(pipe);
   struct svga_shader *shader = CALLOC(1, shader_structlen);
   nir_shader *nir = (nir_shader *)templ->ir.nir;

   if (shader == NULL)
      return NULL;

   shader->id = svga->debug.shader_id++;
   shader->stage = stage;

   if (templ->type == PIPE_SHADER_IR_NIR) {
      /* nir_to_tgsi requires lowered images */
      NIR_PASS_V(nir, gl_nir_lower_images, false);
   }
   shader->tokens = pipe_shader_state_to_tgsi_tokens(pipe->screen, templ);
   shader->type = PIPE_SHADER_IR_TGSI;

   /* Collect basic info of the shader */
   svga_tgsi_scan_shader(shader);

   /* check for any stream output declarations */
   if (templ->stream_output.num_outputs) {
      shader->stream_output = svga_create_stream_output(svga, shader,
                                                        &templ->stream_output);
   }

   return shader;
}


/**
 * Helper function to compile a shader.
 * Depending on the shader IR type, it calls the corresponding
 * compile shader function.
 */
enum pipe_error
svga_compile_shader(struct svga_context *svga,
                    struct svga_shader *shader,
                    const struct svga_compile_key *key,
                    struct svga_shader_variant **out_variant)
{
   struct svga_shader_variant *variant = NULL;
   enum pipe_error ret = PIPE_ERROR;

   if (shader->type == PIPE_SHADER_IR_TGSI) {
      variant = svga_tgsi_compile_shader(svga, shader, key);
   } else {
      debug_printf("Unexpected nir shader\n");
      assert(0);
   }

   if (variant == NULL) {
      if (shader->get_dummy_shader != NULL) {
         debug_printf("Failed to compile shader, using dummy shader.\n");
         variant = shader->get_dummy_shader(svga, shader, key);
      }
   }
   else if (svga_shader_too_large(svga, variant)) {
      /* too big, use shader */
      if (shader->get_dummy_shader != NULL) {
         debug_printf("Shader too large (%u bytes), using dummy shader.\n",
                      (unsigned)(variant->nr_tokens
                                 * sizeof(variant->tokens[0])));

         /* Free the too-large variant */
         svga_destroy_shader_variant(svga, variant);

         /* Use simple pass-through shader instead */
         variant = shader->get_dummy_shader(svga, shader, key);
      }
   }

   if (variant == NULL)
      return PIPE_ERROR;

   ret = svga_define_shader(svga, variant);
   if (ret != PIPE_OK) {
      svga_destroy_shader_variant(svga, variant);
      return ret;
   }

   *out_variant = variant;

   /* insert variant at head of linked list */
   variant->next = shader->variants;
   shader->variants = variant;

   return PIPE_OK;
}
