/**************************************************************************
 *
 * Copyright 2007-2008 VMware, Inc.
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
  * Constant State Object (CSO) cache.
  *
  * The basic idea is that the states are created via the
  * create_state/bind_state/delete_state semantics. The driver is expected to
  * perform as much of the Gallium state translation to whatever its internal
  * representation is during the create call. Gallium then has a caching
  * mechanism where it stores the created states. When the pipeline needs an
  * actual state change, a bind call is issued. In the bind call the driver
  * gets its already translated representation.
  *
  * Those semantics mean that the driver doesn't do the repeated translations
  * of states on every frame, but only once, when a new state is actually
  * created.
  *
  * Even on hardware that doesn't do any kind of state cache, it makes the
  * driver look a lot neater, plus it avoids all the redundant state
  * translations on every frame.
  *
  * Currently our constant state objects are:
  * - alpha test
  * - blend
  * - depth stencil
  * - fragment shader
  * - rasterizer (old setup)
  * - sampler
  * - vertex shader
  * - vertex elements
  *
  * Things that are not constant state objects include:
  * - blend_color
  * - clip_state
  * - clear_color_state
  * - constant_buffer
  * - feedback_state
  * - framebuffer_state
  * - polygon_stipple
  * - scissor_state
  * - texture_state
  * - viewport_state
  *
  * @author Zack Rusin <zackr@vmware.com>
  */

#ifndef CSO_CACHE_H
#define CSO_CACHE_H

#include "pipe/p_context.h"
#include "pipe/p_state.h"

/* cso_hash.h is necessary for cso_hash_iter, as MSVC requires structures
 * returned by value to be fully defined */
#include "cso_hash.h"


#ifdef __cplusplus
extern "C" {
#endif

enum cso_cache_type {
   CSO_RASTERIZER,
   CSO_BLEND,
   CSO_DEPTH_STENCIL_ALPHA,
   CSO_SAMPLER,
   CSO_VELEMENTS,
   CSO_CACHE_MAX,
};

typedef void (*cso_delete_cso_callback)(void *ctx, void *state,
                                        enum cso_cache_type type);

typedef void (*cso_state_callback)(void *ctx, void *obj);

typedef void (*cso_sanitize_callback)(struct cso_hash *hash,
                                      enum cso_cache_type type,
                                      int max_size,
                                      void *user_data);

struct cso_cache {
   struct cso_hash hashes[CSO_CACHE_MAX];
   int max_size;

   cso_sanitize_callback sanitize_cb;
   void *sanitize_data;

   cso_delete_cso_callback delete_cso;
   void *delete_cso_ctx;
};

struct cso_blend {
   struct pipe_blend_state state;
   void *data;
};

struct cso_depth_stencil_alpha {
   struct pipe_depth_stencil_alpha_state state;
   void *data;
};

struct cso_rasterizer {
   struct pipe_rasterizer_state state;
   void *data;
};

struct cso_sampler {
   struct pipe_sampler_state state;
   void *data;
   unsigned hash_key;
};

struct cso_velems_state {
   unsigned count;
   struct pipe_vertex_element velems[PIPE_MAX_ATTRIBS];
};

struct cso_velements {
   struct cso_velems_state state;
   void *data;
};


void
cso_cache_init(struct cso_cache *sc, struct pipe_context *pipe);

void
cso_cache_delete(struct cso_cache *sc);

void
cso_cache_set_sanitize_callback(struct cso_cache *sc,
                                cso_sanitize_callback cb,
                                void *user_data);
void
cso_cache_set_delete_cso_callback(struct cso_cache *sc,
                                  cso_delete_cso_callback delete_cso,
                                  void *ctx);

struct cso_hash_iter
cso_insert_state(struct cso_cache *sc,
                 unsigned hash_key, enum cso_cache_type type,
                 void *state);

void
cso_set_maximum_cache_size(struct cso_cache *sc, int number);

void
cso_delete_state(struct pipe_context *pipe, void *state,
                 enum cso_cache_type type);


static ALWAYS_INLINE unsigned
cso_construct_key(const void *key, int key_size)
{
   unsigned hash = 0;
   const unsigned *ikey = (const unsigned *)key;
   unsigned num_elements = key_size / 4;

   assert(key_size % 4 == 0);

   for (unsigned i = 0; i < num_elements; i++)
      hash ^= ikey[i];

   return hash;
}

static ALWAYS_INLINE struct cso_hash_iter
cso_find_state_template(struct cso_cache *sc, unsigned hash_key,
                        enum cso_cache_type type, const void *key,
                        unsigned key_size)
{
   struct cso_hash *hash = &sc->hashes[type];
   struct cso_hash_iter iter = cso_hash_find(hash, hash_key);

   while (!cso_hash_iter_is_null(iter)) {
      void *iter_data = cso_hash_iter_data(iter);
      if (!memcmp(iter_data, key, key_size))
         return iter;
      iter = cso_hash_iter_next(iter);
   }
   return iter;
}

#ifdef __cplusplus
}
#endif

#endif
