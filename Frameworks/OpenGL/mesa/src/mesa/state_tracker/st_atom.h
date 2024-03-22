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
    

#ifndef ST_ATOM_H
#define ST_ATOM_H

#include "util/glheader.h"
#include "main/mtypes.h"

#ifdef __cplusplus
extern "C" {
#endif

struct st_context;
struct gl_vertex_program;
struct st_common_variant;
struct pipe_vertex_buffer;
struct pipe_vertex_element;
struct cso_velems_state;

void
st_setup_arrays(struct st_context *st,
                const struct gl_vertex_program *vp,
                const struct st_common_variant *vp_variant,
                struct cso_velems_state *velements,
                struct pipe_vertex_buffer *vbuffer, unsigned *num_vbuffers);

void
st_setup_current_user(struct st_context *st,
                      const struct gl_vertex_program *vp,
                      const struct st_common_variant *vp_variant,
                      struct cso_velems_state *velements,
                      struct pipe_vertex_buffer *vbuffer, unsigned *num_vbuffers);

void
st_update_array_with_popcnt(struct st_context *st);

struct pipe_vertex_state *
st_create_gallium_vertex_state(struct gl_context *ctx,
                               const struct gl_vertex_array_object *vao,
                               struct gl_buffer_object *indexbuf,
                               uint32_t enabled_attribs);

/* Define ST_NEW_xxx_INDEX */
enum {
#define ST_STATE(FLAG, st_update) FLAG##_INDEX,
#include "st_atom_list.h"
#undef ST_STATE
   ST_NUM_ATOMS,
};

/* Define ST_NEW_xxx values as static const uint64_t values.
 * We can't use an enum type because MSVC doesn't allow 64-bit enum values.
 */
#define ST_STATE(FLAG, st_update) static const uint64_t FLAG = 1ull << FLAG##_INDEX;
#include "st_atom_list.h"
#undef ST_STATE

/* Declare function prototypes. */
#define ST_STATE(FLAG, st_update) void st_update(struct st_context *st);
#include "st_atom_list.h"
#undef ST_STATE

/* Combined state flags. */
#define ST_NEW_SAMPLERS         (ST_NEW_VS_SAMPLERS | \
                                 ST_NEW_TCS_SAMPLERS | \
                                 ST_NEW_TES_SAMPLERS | \
                                 ST_NEW_GS_SAMPLERS | \
                                 ST_NEW_FS_SAMPLERS | \
                                 ST_NEW_CS_SAMPLERS)

#define ST_NEW_FRAMEBUFFER      (ST_NEW_FB_STATE | \
                                 ST_NEW_SAMPLE_STATE | \
                                 ST_NEW_SAMPLE_SHADING)

#define ST_NEW_VERTEX_PROGRAM(ctx, p) ((p)->affected_states | \
                                      (st_user_clip_planes_enabled(ctx) ? \
                                       ST_NEW_CLIP_STATE : 0))

#define ST_NEW_CONSTANTS        (ST_NEW_VS_CONSTANTS | \
                                 ST_NEW_TCS_CONSTANTS | \
                                 ST_NEW_TES_CONSTANTS | \
                                 ST_NEW_FS_CONSTANTS | \
                                 ST_NEW_GS_CONSTANTS | \
                                 ST_NEW_CS_CONSTANTS)

#define ST_NEW_UNIFORM_BUFFER   (ST_NEW_VS_UBOS | \
                                 ST_NEW_TCS_UBOS | \
                                 ST_NEW_TES_UBOS | \
                                 ST_NEW_FS_UBOS | \
                                 ST_NEW_GS_UBOS | \
                                 ST_NEW_CS_UBOS)

#define ST_NEW_SAMPLER_VIEWS    (ST_NEW_VS_SAMPLER_VIEWS | \
                                 ST_NEW_FS_SAMPLER_VIEWS | \
                                 ST_NEW_GS_SAMPLER_VIEWS | \
                                 ST_NEW_TCS_SAMPLER_VIEWS | \
                                 ST_NEW_TES_SAMPLER_VIEWS | \
                                 ST_NEW_CS_SAMPLER_VIEWS)

#define ST_NEW_ATOMIC_BUFFER    (ST_NEW_VS_ATOMICS | \
                                 ST_NEW_TCS_ATOMICS | \
                                 ST_NEW_TES_ATOMICS | \
                                 ST_NEW_FS_ATOMICS | \
                                 ST_NEW_GS_ATOMICS | \
                                 ST_NEW_CS_ATOMICS)

#define ST_NEW_STORAGE_BUFFER   (ST_NEW_VS_SSBOS | \
                                 ST_NEW_TCS_SSBOS | \
                                 ST_NEW_TES_SSBOS | \
                                 ST_NEW_FS_SSBOS | \
                                 ST_NEW_GS_SSBOS | \
                                 ST_NEW_CS_SSBOS)

#define ST_NEW_IMAGE_UNITS      (ST_NEW_VS_IMAGES | \
                                 ST_NEW_TCS_IMAGES | \
                                 ST_NEW_TES_IMAGES | \
                                 ST_NEW_GS_IMAGES | \
                                 ST_NEW_FS_IMAGES | \
                                 ST_NEW_CS_IMAGES)

#define ST_ALL_SHADER_RESOURCES (ST_NEW_SAMPLER_VIEWS | \
                                 ST_NEW_SAMPLERS | \
                                 ST_NEW_UNIFORM_BUFFER | \
                                 ST_NEW_ATOMIC_BUFFER | \
                                 ST_NEW_STORAGE_BUFFER | \
                                 ST_NEW_IMAGE_UNITS)

/* All state flags within each group: */
#define ST_PIPELINE_RENDER_STATE_MASK  (ST_NEW_CS_STATE - 1)
#define ST_PIPELINE_RENDER_STATE_MASK_NO_VARRAYS \
   (ST_PIPELINE_RENDER_STATE_MASK & ~ST_NEW_VERTEX_ARRAYS)
#define ST_PIPELINE_CLEAR_STATE_MASK (ST_NEW_FB_STATE | \
                                      ST_NEW_SCISSOR | \
                                      ST_NEW_WINDOW_RECTANGLES)
#define ST_PIPELINE_META_STATE_MASK ST_PIPELINE_RENDER_STATE_MASK_NO_VARRAYS
/* For ReadPixels, ReadBuffer, GetSamplePosition: */
#define ST_PIPELINE_UPDATE_FB_STATE_MASK (ST_NEW_FB_STATE)

/* We add the ST_NEW_FB_STATE bit here as well, because glBindFramebuffer
 * acts as a barrier that breaks feedback loops between the framebuffer
 * and textures bound to the framebuffer, even when those textures are
 * accessed by compute shaders; so we must inform the driver of new
 * framebuffer state.
 */
#define ST_PIPELINE_COMPUTE_STATE_MASK ((0xffull << ST_NEW_CS_STATE_INDEX) | \
                                        ST_NEW_FB_STATE)

#define ST_ALL_STATES_MASK (ST_PIPELINE_RENDER_STATE_MASK | \
                            ST_PIPELINE_COMPUTE_STATE_MASK)

#ifdef __cplusplus
}
#endif

#endif
