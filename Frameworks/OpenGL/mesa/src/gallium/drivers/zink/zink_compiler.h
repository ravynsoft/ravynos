/*
 * Copyright 2018 Collabora Ltd.
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

#ifndef ZINK_COMPILER_H
#define ZINK_COMPILER_H

#include "zink_types.h"

#define ZINK_WORKGROUP_SIZE_X 1
#define ZINK_WORKGROUP_SIZE_Y 2
#define ZINK_WORKGROUP_SIZE_Z 3
#define ZINK_VARIABLE_SHARED_MEM 4
#define ZINK_INLINE_VAL_FLAT_MASK 0
#define ZINK_INLINE_VAL_PV_LAST_VERT 2

/* stop inlining shaders if they have >limit ssa vals after inlining:
 * recompile time isn't worth the inline
 */
#define ZINK_ALWAYS_INLINE_LIMIT 1500

struct zink_shader_key;
struct spirv_shader;

struct tgsi_token;

static inline gl_shader_stage
clamp_stage(const shader_info *info)
{
   return info->stage == MESA_SHADER_KERNEL ? MESA_SHADER_COMPUTE : info->stage;
}

const void *
zink_get_compiler_options(struct pipe_screen *screen,
                          enum pipe_shader_ir ir,
                          gl_shader_stage shader);

struct nir_shader *
zink_tgsi_to_nir(struct pipe_screen *screen, const struct tgsi_token *tokens);

nir_shader*
zink_create_quads_emulation_gs(const nir_shader_compiler_options *options,
                               const nir_shader *prev_stage);

bool
zink_lower_system_values_to_inlined_uniforms(nir_shader *nir);

void
zink_screen_init_compiler(struct zink_screen *screen);
void
zink_compiler_assign_io(struct zink_screen *screen, nir_shader *producer, nir_shader *consumer);
/* pass very large shader key data with extra_data */
struct zink_shader_object
zink_shader_compile(struct zink_screen *screen, bool can_shobj, struct zink_shader *zs, nir_shader *nir, const struct zink_shader_key *key, const void *extra_data, struct zink_program *pg);
struct zink_shader_object
zink_shader_compile_separate(struct zink_screen *screen, struct zink_shader *zs);
struct zink_shader *
zink_shader_create(struct zink_screen *screen, struct nir_shader *nir);

char *
zink_shader_finalize(struct pipe_screen *pscreen, void *nirptr);

void
zink_shader_free(struct zink_screen *screen, struct zink_shader *shader);
void
zink_gfx_shader_free(struct zink_screen *screen, struct zink_shader *shader);

struct zink_shader_object
zink_shader_spirv_compile(struct zink_screen *screen, struct zink_shader *zs, struct spirv_shader *spirv, bool can_shobj, struct zink_program *pg);
struct zink_shader_object
zink_shader_tcs_compile(struct zink_screen *screen, struct zink_shader *zs, unsigned patch_vertices, bool can_shobj, struct zink_program *pg);
struct zink_shader *
zink_shader_tcs_create(struct zink_screen *screen, nir_shader *tes, unsigned vertices_per_patch, nir_shader **nir_ret);

static inline bool
zink_shader_descriptor_is_buffer(struct zink_shader *zs, enum zink_descriptor_type type, unsigned i)
{
   return zs->bindings[type][i].type == VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER ||
          zs->bindings[type][i].type == VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER;
}

bool
zink_shader_has_cubes(nir_shader *nir);
nir_shader *
zink_shader_blob_deserialize(struct zink_screen *screen, struct blob *blob);
nir_shader *
zink_shader_deserialize(struct zink_screen *screen, struct zink_shader *zs);
void
zink_shader_serialize_blob(nir_shader *nir, struct blob *blob);
void
zink_print_shader(struct zink_screen *screen, struct zink_shader *zs, FILE *fp);
#endif
