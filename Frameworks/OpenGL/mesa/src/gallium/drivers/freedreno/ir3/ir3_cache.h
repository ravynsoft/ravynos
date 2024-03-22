/*
 * Copyright (C) 2018 Rob Clark <robclark@freedesktop.org>
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
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 * Authors:
 *    Rob Clark <robclark@freedesktop.org>
 */

#ifndef IR3_CACHE_H_
#define IR3_CACHE_H_

#include "pipe/p_state.h"

#include "ir3/ir3_shader.h"

BEGINC;

/*
 * An in-memory cache for mapping shader state objects plus shader key to
 * hw specific state object for the specified shader variant.  This is to
 * allow re-using things like the register setup for varying linkage, etc.
 */

/* key into program state cache */
struct ir3_cache_key {
   struct ir3_shader_state *vs, *hs, *ds, *gs, *fs; // 5 pointers
   struct ir3_shader_key key;                       // 7 dwords

   /* Additional state that effects the cached program state, but
    * not the compiled shader:
    */
   unsigned clip_plane_enable : PIPE_MAX_CLIP_PLANES;
   unsigned patch_vertices;
};

/* per-gen backend program state object should subclass this for it's
 * state object, mainly because we need a copy of the key that is not
 * allocated on the stack
 */
struct ir3_program_state {
   struct ir3_cache_key key;
};

struct ir3_cache_funcs {
   struct ir3_program_state *(*create_state)(
      void *data, const struct ir3_shader_variant *bs, /* binning pass vs */
      const struct ir3_shader_variant *vs, const struct ir3_shader_variant *hs,
      const struct ir3_shader_variant *ds, const struct ir3_shader_variant *gs,
      const struct ir3_shader_variant *fs, const struct ir3_cache_key *key);
   void (*destroy_state)(void *data, struct ir3_program_state *state);
};

struct ir3_cache;

/* construct a shader cache.  Free with ralloc_free() */
struct ir3_cache *ir3_cache_create(const struct ir3_cache_funcs *funcs,
                                   void *data);
void ir3_cache_destroy(struct ir3_cache *cache);

/* debug callback is used for shader-db logs in case the lookup triggers
 * shader variant compilation.
 */
struct ir3_program_state *ir3_cache_lookup(struct ir3_cache *cache,
                                           const struct ir3_cache_key *key,
                                           struct util_debug_callback *debug);

/* call when an API level state object is destroyed, to invalidate
 * cache entries which reference that state object.
 */
void ir3_cache_invalidate(struct ir3_cache *cache, void *stobj);

ENDC;

#endif /* IR3_CACHE_H_ */
