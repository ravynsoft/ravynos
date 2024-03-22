/**************************************************************************
 * 
 * Copyright 2003 VMware, Inc.
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

 /*
  * Authors:
  *   Keith Whitwell <keithw@vmware.com>
  */
    

#ifndef ST_PROGRAM_H
#define ST_PROGRAM_H

#include "main/atifragshader.h"
#include "program/program.h"
#include "pipe/p_state.h"
#include "tgsi/tgsi_from_mesa.h"
#include "st_context.h"
#include "st_texture.h"

#ifdef __cplusplus
extern "C" {
#endif

struct st_external_sampler_key
{
   GLuint lower_nv12;             /**< bitmask of 2 plane YUV samplers */
   GLuint lower_nv21;
   GLuint lower_iyuv;             /**< bitmask of 3 plane YUV samplers */
   GLuint lower_xy_uxvx;          /**< bitmask of 2 plane YUV samplers */
   GLuint lower_xy_vxux;          /**< bitmask of 2 plane YUV samplers */
   GLuint lower_yx_xuxv;          /**< bitmask of 2 plane YUV samplers */
   GLuint lower_yx_xvxu;          /**< bitmask of 2 plane YUV samplers */
   GLuint lower_ayuv;
   GLuint lower_xyuv;
   GLuint lower_yuv;
   GLuint lower_yu_yv;
   GLuint lower_yv_yu;
   GLuint lower_y41x;
   GLuint bt709;
   GLuint bt2020;
   GLuint yuv_full_range;
};

static inline struct st_external_sampler_key
st_get_external_sampler_key(struct st_context *st, struct gl_program *prog)
{
   unsigned mask = prog->ExternalSamplersUsed;
   struct st_external_sampler_key key;

   memset(&key, 0, sizeof(key));

   while (unlikely(mask)) {
      unsigned unit = u_bit_scan(&mask);
      struct gl_texture_object *stObj =
            st_get_texture_object(st->ctx, prog, unit);
      enum pipe_format format = st_get_view_format(stObj);

      /* if resource format matches then YUV wasn't lowered */
      if (format == stObj->pt->format)
         continue;

      switch (format) {
      case PIPE_FORMAT_NV12:
         if (stObj->pt->format == PIPE_FORMAT_R8_G8B8_420_UNORM) {
            key.lower_yuv |= (1 << unit);
            break;
         }
         FALLTHROUGH;
      case PIPE_FORMAT_P010:
      case PIPE_FORMAT_P012:
      case PIPE_FORMAT_P016:
      case PIPE_FORMAT_P030:
         key.lower_nv12 |= (1 << unit);
         break;
      case PIPE_FORMAT_NV21:
         if (stObj->pt->format == PIPE_FORMAT_R8_B8G8_420_UNORM) {
            key.lower_yuv |= (1 << unit);
            break;
         }
         key.lower_nv21 |= (1 << unit);
         break;
      case PIPE_FORMAT_IYUV:
         if (stObj->pt->format == PIPE_FORMAT_R8_G8_B8_420_UNORM ||
             stObj->pt->format == PIPE_FORMAT_R8_B8_G8_420_UNORM) {
            key.lower_yuv |= (1 << unit);
            break;
         }
         key.lower_iyuv |= (1 << unit);
         break;
      case PIPE_FORMAT_YUYV:
         if (stObj->pt->format == PIPE_FORMAT_R8G8_R8B8_UNORM) {
            key.lower_yu_yv |= (1 << unit);
            break;
         }
         FALLTHROUGH;
      case PIPE_FORMAT_Y210:
      case PIPE_FORMAT_Y212:
      case PIPE_FORMAT_Y216:
         key.lower_yx_xuxv |= (1 << unit);
         break;
      case PIPE_FORMAT_UYVY:
         if (stObj->pt->format == PIPE_FORMAT_G8R8_B8R8_UNORM) {
            key.lower_yu_yv |= (1 << unit);
            break;
         }
         key.lower_xy_uxvx |= (1 << unit);
         break;
      case PIPE_FORMAT_VYUY:
         if (stObj->pt->format == PIPE_FORMAT_B8R8_G8R8_UNORM) {
            key.lower_yv_yu |= (1 << unit);
            break;
         }
         key.lower_xy_vxux |= (1 << unit);
         break;
      case PIPE_FORMAT_YVYU:
         if (stObj->pt->format == PIPE_FORMAT_R8B8_R8G8_UNORM) {
            key.lower_yv_yu |= (1 << unit);
            break;
         }
         key.lower_yx_xvxu |= (1 << unit);
         break;
      case PIPE_FORMAT_AYUV:
         key.lower_ayuv |= (1 << unit);
         break;
      case PIPE_FORMAT_XYUV:
         key.lower_xyuv |= (1 << unit);
         break;
      case PIPE_FORMAT_Y410:
      case PIPE_FORMAT_Y412:
      case PIPE_FORMAT_Y416:
         key.lower_y41x |= (1 << unit);
         break;
      default:
         printf("mesa: st_get_external_sampler_key: unhandled pipe format %u\n",
                format);
         break;
      }

      switch (stObj->yuv_color_space) {
      case GL_TEXTURE_YUV_COLOR_SPACE_REC601:
         break;
      case GL_TEXTURE_YUV_COLOR_SPACE_REC709:
         key.bt709 |= (1 << unit);
         break;
      case GL_TEXTURE_YUV_COLOR_SPACE_REC2020:
         key.bt2020 |= (1 << unit);
         break;
      }

      if (stObj->yuv_full_range)
         key.yuv_full_range |= (1 << unit);
   }

   return key;
}

/** Fragment program variant key
 *
 * Please update st_get_fp_variant() perf_debug() when adding fields.
 */
struct st_fp_variant_key
{
   struct st_context *st;         /**< variants are per-context */

   /** for glBitmap */
   GLuint bitmap:1;               /**< glBitmap variant? */

   /** for glDrawPixels */
   GLuint drawpixels:1;           /**< glDrawPixels variant */
   GLuint scaleAndBias:1;         /**< glDrawPixels w/ scale and/or bias? */
   GLuint pixelMaps:1;            /**< glDrawPixels w/ pixel lookup map? */

   /** for ARB_color_buffer_float */
   GLuint clamp_color:1;

   /** for ARB_sample_shading */
   GLuint persample_shading:1;

   /** needed for ATI_fragment_shader */
   GLuint fog:2;

   /** for OpenGL 1.0 on modern hardware */
   GLuint lower_two_sided_color:1;

   GLuint lower_flatshade:1;
   unsigned lower_alpha_func:3;

   /** needed for ATI_fragment_shader */
   uint8_t texture_index[MAX_NUM_FRAGMENT_REGISTERS_ATI];

   struct st_external_sampler_key external;

   /* bitmask of sampler units; PIPE_CAP_GL_CLAMP */
   uint32_t gl_clamp[3];

   /* bitmask of shadow samplers with depth textures in them for ARB programs; */
   GLbitfield depth_textures;
};

/**
 * Base class for shader variants.
 */
struct st_variant
{
   /** next in linked list */
   struct st_variant *next;

   /** st_context from the shader key */
   struct st_context *st;

   void *driver_shader;
};

/**
 * Variant of a fragment program.
 */
struct st_fp_variant
{
   struct st_variant base;

   /** Parameters which generated this version of fragment program */
   struct st_fp_variant_key key;

   /** For glBitmap variants */
   uint bitmap_sampler;

   /** For glDrawPixels variants */
   unsigned drawpix_sampler;
   unsigned pixelmap_sampler;
};


/** Shader key shared by other shaders.
 *
 * Please update st_get_common_variant() perf_debug() when adding fields.
 */
struct st_common_variant_key
{
   struct st_context *st;          /**< variants are per-context */
   bool passthrough_edgeflags;

   /** for ARB_color_buffer_float */
   bool clamp_color;

   /** lower glPointSize to gl_PointSize */
   bool export_point_size;

   /* for user-defined clip-planes */
   uint8_t lower_ucp;

   /* Whether st_variant::driver_shader is for the draw module,
    * not for the driver.
    */
   bool is_draw_shader;

   /* bitmask of sampler units; PIPE_CAP_GL_CLAMP */
   uint32_t gl_clamp[3];
};


/**
 * Common shader variant.
 */
struct st_common_variant
{
   struct st_variant base;

   /* Parameters which generated this variant. */
   struct st_common_variant_key key;

   /* Bitfield of VERT_BIT_* bits matching vertex shader inputs. */
   GLbitfield vert_attrib_mask;
};

static inline struct st_common_variant *
st_common_variant(struct st_variant *v)
{
   return (struct st_common_variant*)v;
}

static inline struct st_fp_variant *
st_fp_variant(struct st_variant *v)
{
   return (struct st_fp_variant*)v;
}

/**
 * This defines mapping from Mesa VARYING_SLOTs to TGSI GENERIC slots.
 */
static inline unsigned
st_get_generic_varying_index(struct st_context *st, GLuint attr)
{
   return tgsi_get_generic_gl_varying_index((gl_varying_slot)attr,
                                            st->needs_texcoord_semantic);
}

extern void
st_set_prog_affected_state_flags(struct gl_program *prog);


extern struct st_fp_variant *
st_get_fp_variant(struct st_context *st,
                  struct gl_program *stfp,
                  const struct st_fp_variant_key *key);

extern struct st_common_variant *
st_get_common_variant(struct st_context *st,
                      struct gl_program *p,
                      const struct st_common_variant_key *key);

extern void
st_release_variants(struct st_context *st, struct gl_program *p);

extern void
st_release_program(struct st_context *st, struct gl_program **p);

extern void
st_destroy_program_variants(struct st_context *st);

extern void
st_finalize_nir_before_variants(struct nir_shader *nir);

extern void
st_prepare_vertex_program(struct gl_program *stvp);

extern void
st_translate_stream_output_info(struct gl_program *prog);

extern void
st_serialize_nir(struct gl_program *stp);

extern void
st_finalize_program(struct st_context *st, struct gl_program *prog);

struct pipe_shader_state *
st_create_nir_shader(struct st_context *st, struct pipe_shader_state *state);

GLboolean st_program_string_notify(struct gl_context *ctx,
                                   GLenum target,
                                   struct gl_program *prog);

#ifdef __cplusplus
}
#endif

#endif
