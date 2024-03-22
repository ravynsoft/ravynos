/**************************************************************************
 *
 * Copyright 2007 VMware, Inc.
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

#ifndef ST_TEXTURE_H
#define ST_TEXTURE_H


#include "pipe/p_context.h"
#include "util/u_sampler.h"
#include "util/simple_mtx.h"

#include "main/mtypes.h"


struct pipe_resource;


struct st_texture_image_transfer
{
   struct pipe_transfer *transfer;

   /* For compressed texture fallback. */
   GLubyte *temp_data; /**< Temporary compressed texture storage. */
   unsigned temp_stride; /**< Stride of the compressed texture storage. */
   struct pipe_box box; /**< Region of the transfer's resource to write. */
};


/**
 * Container for one context's validated sampler view.
 */
struct st_sampler_view
{
   struct pipe_sampler_view *view;

   /** The context which created this view */
   struct st_context *st;

   /** The glsl version of the shader seen during validation */
   bool glsl130_or_later;
   /** Derived from the sampler's sRGBDecode state during validation */
   bool srgb_skip_decode;

   /* This mechanism allows passing sampler view references to the driver
    * without using atomics to increase the reference count.
    *
    * This private refcount can be decremented without atomics but only one
    * context (st above) can use this counter (so that it's only used by
    * 1 thread).
    *
    * This number is atomically added to view->reference.count at
    * initialization. If it's never used, the same number is atomically
    * subtracted from view->reference.count before destruction. If this
    * number is decremented, we can pass one reference to the driver without
    * touching reference.count with atomics. At destruction we only subtract
    * the number of references we have not returned. This can possibly turn
    * a million atomic increments into 1 add and 1 subtract atomic op over
    * the whole lifetime of an app.
    */
   int private_refcount;
};


/**
 * Container for per-context sampler views of a texture.
 */
struct st_sampler_views
{
   struct st_sampler_views *next;
   uint32_t max;
   uint32_t count;
   struct st_sampler_view views[0];
};

struct st_compressed_data
{
   struct pipe_reference reference;
   GLubyte *ptr;
};

static inline const struct gl_texture_image *
st_texture_image_const(const struct gl_texture_image *img)
{
   return (const struct gl_texture_image *) img;
}

static inline const struct gl_texture_object *
st_texture_object_const(const struct gl_texture_object *obj)
{
   return (const struct gl_texture_object *) obj;
}


static inline struct pipe_resource *
st_get_texobj_resource(struct gl_texture_object *texObj)
{
   return texObj ? texObj->pt : NULL;
}


static inline struct pipe_resource *
st_get_stobj_resource(struct gl_texture_object *stObj)
{
   return stObj ? stObj->pt : NULL;
}


static inline struct gl_texture_object *
st_get_texture_object(struct gl_context *ctx,
                      const struct gl_program *prog,
                      unsigned unit)
{
   const GLuint texUnit = prog->SamplerUnits[unit];
   return ctx->Texture.Unit[texUnit]._Current;
}

static inline enum pipe_format
st_get_view_format(struct gl_texture_object *stObj)
{
   if (!stObj)
      return PIPE_FORMAT_NONE;
   return stObj->surface_based ? stObj->surface_format : stObj->pt->format;
}


extern struct pipe_resource *
st_texture_create(struct st_context *st,
                  enum pipe_texture_target target,
                  enum pipe_format format,
                  GLuint last_level,
                  GLuint width0,
                  GLuint height0,
                  GLuint depth0,
                  GLuint layers,
                  GLuint nr_samples,
                  GLuint tex_usage,
                  bool sparse);


extern void
st_gl_texture_dims_to_pipe_dims(GLenum texture,
                                unsigned widthIn,
                                uint16_t heightIn,
                                uint16_t depthIn,
                                unsigned *widthOut,
                                uint16_t *heightOut,
                                uint16_t *depthOut,
                                uint16_t *layersOut);

/* Check if an image fits into an existing texture object.
 */
extern GLboolean
st_texture_match_image(struct st_context *st,
                       const struct pipe_resource *pt,
                       const struct gl_texture_image *image);

/* Insert a transfer pointer into the image's transfer array at the specified
 * index. The array is reallocated if necessary.
 */
void
st_texture_image_insert_transfer(struct gl_texture_image *stImage,
                                 unsigned index,
                                 struct pipe_transfer *transfer);

/**
 * Returns the level of the gl_texture_image with respect to the resource it
 * is allocated within. Example: returns 0 for non-finalized texture.
 */
GLuint
st_texture_image_resource_level(struct gl_texture_image *stImage);

/* Return a pointer to an image within a texture.  Return image stride as
 * well.
 */
extern GLubyte *
st_texture_image_map(struct st_context *st, struct gl_texture_image *stImage,
                     enum pipe_map_flags usage,
                     GLuint x, GLuint y, GLuint z,
                     GLuint w, GLuint h, GLuint d,
                     struct pipe_transfer **transfer);

extern void
st_texture_image_unmap(struct st_context *st,
                       struct gl_texture_image *stImage, unsigned slice);


/* Return pointers to each 2d slice within an image.  Indexed by depth
 * value.
 */
extern const GLuint *
st_texture_depth_offsets(struct pipe_resource *pt, GLuint level);

/* Copy an image between two textures
 */
extern void
st_texture_image_copy(struct pipe_context *pipe,
                      struct pipe_resource *dst, GLuint dstLevel,
                      struct pipe_resource *src, GLuint srcLevel,
                      GLuint face);


extern struct pipe_resource *
st_create_color_map_texture(struct gl_context *ctx);

void
st_destroy_bound_texture_handles(struct st_context *st);

void
st_destroy_bound_image_handles(struct st_context *st);

bool
st_astc_format_fallback(const struct st_context *st, mesa_format format);

bool
st_compressed_format_fallback(struct st_context *st, mesa_format format);

void
st_convert_image(const struct st_context *st, const struct gl_image_unit *u,
                 struct pipe_image_view *img, enum gl_access_qualifier shader_access);

void
st_convert_image_from_unit(const struct st_context *st,
                           struct pipe_image_view *img,
                           GLuint imgUnit,
                           enum gl_access_qualifier shader_access);

void
st_convert_sampler(const struct st_context *st,
                   const struct gl_texture_object *texobj,
                   const struct gl_sampler_object *msamp,
                   float tex_unit_lod_bias,
                   struct pipe_sampler_state *sampler,
                   bool seamless_cube_map,
                   bool ignore_srgb_decode,
                   bool glsl130_or_later);

void
st_convert_sampler_from_unit(const struct st_context *st,
                             struct pipe_sampler_state *sampler,
                             GLuint texUnit,
                             bool glsl130_or_later);

struct pipe_sampler_view *
st_update_single_texture(struct st_context *st,
                         GLuint texUnit, bool glsl130_or_later,
                         bool ignore_srgb_decode, bool get_reference);

unsigned
st_get_sampler_views(struct st_context *st,
                     enum pipe_shader_type shader_stage,
                     const struct gl_program *prog,
                     struct pipe_sampler_view **sampler_views);

void
st_make_bound_samplers_resident(struct st_context *st,
                                struct gl_program *prog);

void
st_make_bound_images_resident(struct st_context *st,
                              struct gl_program *prog);

#endif
