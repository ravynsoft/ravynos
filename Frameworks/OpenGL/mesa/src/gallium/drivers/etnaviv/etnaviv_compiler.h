/*
 * Copyright (c) 2012-2015 Etnaviv Project
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sub license,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the
 * next paragraph) shall be included in all copies or substantial portions
 * of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT. IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 *
 * Authors:
 *    Wladimir J. van der Laan <laanwj@gmail.com>
 */

#ifndef H_ETNAVIV_COMPILER
#define H_ETNAVIV_COMPILER

#include "etnaviv_context.h"
#include "etnaviv_internal.h"
#include "etnaviv_shader.h"
#include "util/compiler.h"
#include "pipe/p_shader_tokens.h"
#include "compiler/shader_enums.h"
#include "util/disk_cache.h"

/* XXX some of these are pretty arbitrary limits, may be better to switch
 * to dynamic allocation at some point.
 */
#define ETNA_MAX_TEMPS (64) /* max temp register count of all Vivante hw */
#define ETNA_MAX_TOKENS (2048)
#define ETNA_MAX_IMM (1024) /* max const+imm in 32-bit words */
#define ETNA_MAX_DEPTH (32)
#define ETNA_MAX_INSTRUCTIONS (2048)

/**
 * Compiler state saved across compiler invocations, for any expensive global
 * setup.
 */
struct etna_compiler {
   uint32_t shader_count;
   struct ra_regs *regs;

   nir_shader_compiler_options options;
   struct disk_cache *disk_cache;
};

/* compiler output per input/output */
struct etna_shader_inout {
   int reg; /* native register */
   int slot; /* nir: gl_varying_slot or gl_vert_attrib */
   int num_components;
};

struct etna_shader_io_file {
   size_t num_reg;
   struct etna_shader_inout reg[ETNA_NUM_INPUTS];
};

/* shader object, for linking */
struct etna_shader_variant {
   uint32_t id; /* for debug */

   /* shader variants form a linked list */
   struct etna_shader_variant *next;

   /* replicated here to avoid passing extra ptrs everywhere */
   struct etna_shader *shader;
   struct etna_shader_key key;

   struct etna_bo *bo; /* cached code memory bo handle (for icache) */

   /*
    * Below here is serialized when written to disk cache:
    */
   uint32_t *code;
   struct etna_shader_uniform_info uniforms;

   /*
    * The following macros are used by the shader disk cache save/
    * restore paths to serialize/deserialize the variant.  Any
    * pointers that require special handling in store_variant()
    * and retrieve_variant() should go above here.
    */
#define VARIANT_CACHE_START    offsetof(struct etna_shader_variant, stage)
#define VARIANT_CACHE_PTR(v)   (((char *)v) + VARIANT_CACHE_START)
#define VARIANT_CACHE_SIZE     (sizeof(struct etna_shader_variant) - VARIANT_CACHE_START)

   gl_shader_stage stage;
   uint32_t code_size; /* code size in uint32 words */
   unsigned num_loops;
   unsigned num_temps;

   /* ETNA_DIRTY_* flags that, when set in context dirty, mean that the
    * uniforms have to get (partial) reloaded. */
   uint32_t uniforms_dirty_bits;

   /* inputs (for linking) for fs, the inputs must be in register 1..N */
   struct etna_shader_io_file infile;

   /* outputs (for linking) */
   struct etna_shader_io_file outfile;

   /* special inputs/outputs (vs only) */
   int vs_id_in_reg; /* vertexid+instanceid input */
   int vs_pos_out_reg; /* VS position output */
   int vs_pointsize_out_reg; /* VS point size output */
   uint32_t vs_load_balancing;

   /* special outputs (ps only) */
   int ps_color_out_reg; /* color output register */
   int ps_depth_out_reg; /* depth output register */

   /* unknown input property (XX_INPUT_COUNT, field UNK8) */
   uint32_t input_count_unk8;

   /* shader is larger than GPU instruction limit, thus needs icache */
   bool needs_icache;

   /* shader uses pixel kill/discard */
   bool uses_discard;
};

struct etna_varying {
   uint32_t pa_attributes;
   uint8_t num_components;
   uint8_t use[4];
   uint8_t reg;
};

struct etna_shader_link_info {
   /* each PS input is annotated with the VS output reg */
   unsigned num_varyings;
   struct etna_varying varyings[ETNA_NUM_INPUTS];
   int pcoord_varying_comp_ofs;
};

struct etna_compiler *
etna_compiler_create(const char *renderer, const struct etna_specs *specs);

void
etna_compiler_destroy(const struct etna_compiler *compiler);

const nir_shader_compiler_options *
etna_compiler_get_options(struct etna_compiler *compiler);

bool
etna_compile_shader(struct etna_shader_variant *shader);

void
etna_dump_shader(const struct etna_shader_variant *shader);

void
etna_link_shader(struct etna_shader_link_info *info,
                 const struct etna_shader_variant *vs,
                 const struct etna_shader_variant *fs);

void
etna_destroy_shader(struct etna_shader_variant *shader);

#endif
