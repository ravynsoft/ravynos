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
 * The rast code is concerned with rasterization of command bins.
 * Each screen tile has a bin associated with it.  To render the
 * scene we iterate over the tile bins and execute the commands
 * in each bin.
 * We'll do that with multiple threads...
 */


#ifndef LP_RAST_H
#define LP_RAST_H

#include "util/compiler.h"
#include "util/u_pack_color.h"
#include "util/u_rect.h"
#include "lp_jit.h"


struct lp_rasterizer;
struct lp_scene;
struct lp_fence;
struct cmd_bin;

#define FIXED_TYPE_WIDTH 64
/** For sub-pixel positioning */
#define FIXED_ORDER 8
#define FIXED_ONE (1<<FIXED_ORDER)
#define FIXED_SHIFT (FIXED_TYPE_WIDTH - 1)
/** Maximum length of an edge in a primitive in pixels.
 *  If the framebuffer is large we have to think about fixed-point
 *  integer overflow. Coordinates need ((FIXED_TYPE_WIDTH/2) - 1) bits
 *  to be able to fit product of two such coordinates inside
 *  FIXED_TYPE_WIDTH, any larger and we could overflow a
 *  FIXED_TYPE_WIDTH_-bit int.
 */
#define MAX_FIXED_LENGTH (1 << (((FIXED_TYPE_WIDTH/2) - 1) - FIXED_ORDER))

#define MAX_FIXED_LENGTH32 (1 << (((32/2) - 1) - FIXED_ORDER))

/* Rasterizer output size going to jit fs, width/height */
#define LP_RASTER_BLOCK_SIZE 4

#define LP_MAX_ACTIVE_BINNED_QUERIES 64

#define IMUL64(a, b) (((int64_t)(a)) * ((int64_t)(b)))

struct lp_rasterizer_task;

extern const float lp_sample_pos_4x[4][2];


/**
 * Rasterization state.
 * Objects of this type are put into the shared data bin and pointed
 * to by commands in the per-tile bins.
 */
struct lp_rast_state {
   /* State for the shader.  This also contains state which feeds into
    * the fragment shader, such as blend color and alpha ref value.
    */
   struct lp_jit_context jit_context;
   struct lp_jit_resources jit_resources;

   /* The shader itself.  Probably we also need to pass a pointer to
    * the tile color/z/stencil data somehow
     */
   struct lp_fragment_shader_variant *variant;
};


/**
 * Texture blit offsets.
 */
struct lp_rast_blit {
   int16_t x0;
   int16_t y0;
};


/**
 * Coefficients necessary to run the shader at a given location.
 * First coefficient is position.
 * These pointers point into the bin data buffer.
 */
struct lp_rast_shader_inputs {
   unsigned frontfacing:1;      /** True for front-facing */
   unsigned disable:1;          /** Partially binned, disable this command */
   unsigned is_blit:1;          /* blit */
   unsigned viewport_index:4;  /* viewport index */
   unsigned layer:11;
   unsigned view_index:14;
   unsigned stride;             /* how much to advance data between a0, dadx, dady */
   unsigned pad[2];
   /* followed by a0, dadx, dady and planes[] */
};


struct lp_rast_plane {
   /* edge function values at minx,miny ?? */
   int64_t c;

   int32_t dcdx;
   int32_t dcdy;

   /* one-pixel sized trivial reject offsets for each plane */
   uint32_t eo;
   /*
    * We rely on this struct being 64bit aligned (ideally it would be 128bit
    * but that's quite the waste) and therefore on 32bit we need padding
    * since otherwise (even with the 64bit number in there) it wouldn't be.
    */
   uint32_t pad;
};


/**
 * Rasterization information for a triangle known to be in this bin,
 * plus inputs to run the shader:
 * These fields are tile- and bin-independent.
 * Objects of this type are put into the lp_setup_context::data buffer.
 */
struct lp_rast_triangle {
#ifdef DEBUG
   float v[3][2];
   float pad0;
   float pad1;
#endif

   /* inputs for the shader */
   struct lp_rast_shader_inputs inputs;
   /* planes are also allocated here */
};


#define RECT_PLANE_LEFT   0x1
#define RECT_PLANE_RIGHT  0x2
#define RECT_PLANE_TOP    0x4
#define RECT_PLANE_BOTTOM 0x8

/**
 * Rasterization information for a screen-aligned rectangle known to
 * be in this bin, plus inputs to run the shader:
 * These fields are tile- and bin-independent.
 * Objects of this type are put into the lp_setup_context::data buffer.
 */
struct lp_rast_rectangle {
#ifdef DEBUG
   float v[2][2]; /**< diagonal corners */
#endif

   /* Rectangle boundaries in integer pixels:
    */
   struct u_rect box;

   /* inputs for the shader */
   struct lp_rast_shader_inputs inputs;
};


struct lp_rast_clear_rb {
   union util_color color_val;
   unsigned cbuf;
};


/*
 * Return the address (as float[][4]) of the FS input values which
 * are immediately after the 'inputs' object.
 */
static inline float(*
GET_A0(const struct lp_rast_shader_inputs *inputs))[4]
{
   return (float (*)[4]) (inputs + 1);
}

/*
 * Return the address (as float[][4]) of the FS input partial derivatives
 * (w.r.t. X) which are after the 'inputs' object.
 */
static inline float(*
GET_DADX(const struct lp_rast_shader_inputs *inputs))[4]
{
   const uint8_t *p = (const uint8_t *) (inputs + 1);
   return (float (*)[4]) (p + 1 * inputs->stride);
}

/*
 * Return the address (as float[][4]) of the FS input partial derivatives
 * (w.r.t. Y) which are after the 'inputs' object.
 */
static inline float(*
GET_DADY(const struct lp_rast_shader_inputs *inputs))[4]
{
   const uint8_t *p = (const uint8_t *) (inputs + 1);
   return (float (*)[4]) (p + 2 * inputs->stride);
}

static inline struct lp_rast_plane *
GET_PLANES(const struct lp_rast_triangle *tri)
{
   const uint8_t *p = (const uint8_t *) (&tri->inputs + 1);
   return (struct lp_rast_plane *) (p + 3 * tri->inputs.stride);
}


struct lp_rasterizer *
lp_rast_create(unsigned num_threads);

void
lp_rast_destroy(struct lp_rasterizer *);

void
lp_rast_queue_scene(struct lp_rasterizer *rast,
                     struct lp_scene *scene);

void
lp_rast_finish(struct lp_rasterizer *rast);


union lp_rast_cmd_arg {
   const struct lp_rast_shader_inputs *shade_tile;
   struct {
      const struct lp_rast_triangle *tri;
      unsigned plane_mask;
   } triangle;
   const struct lp_rast_rectangle *rectangle;
   const struct lp_rast_state *set_state;
   const struct lp_rast_clear_rb *clear_rb;
   struct {
      uint64_t value;
      uint64_t mask;
   } clear_zstencil;
   struct lp_fence *fence;
   struct llvmpipe_query *query_obj;
};


/* Cast wrappers.  Hopefully these compile to noops!
 */
static inline union lp_rast_cmd_arg
lp_rast_arg_inputs(const struct lp_rast_shader_inputs *shade_tile)
{
   union lp_rast_cmd_arg arg;
   arg.shade_tile = shade_tile;
   return arg;
}


static inline union lp_rast_cmd_arg
lp_rast_arg_triangle(const struct lp_rast_triangle *triangle,
                      unsigned plane_mask)
{
   union lp_rast_cmd_arg arg;
   arg.triangle.tri = triangle;
   arg.triangle.plane_mask = plane_mask;
   return arg;
}


/**
 * Build argument for a contained triangle.
 *
 * All planes are enabled, so instead of the plane mask we pass the upper
 * left coordinates of the a block that fully encloses the triangle.
 */
static inline union lp_rast_cmd_arg
lp_rast_arg_triangle_contained(const struct lp_rast_triangle *triangle,
                                unsigned x, unsigned y)
{
   union lp_rast_cmd_arg arg;
   arg.triangle.tri = triangle;
   arg.triangle.plane_mask = x | (y << 8);
   return arg;
}


static inline union lp_rast_cmd_arg
lp_rast_arg_rectangle(const struct lp_rast_rectangle *rectangle)
{
   union lp_rast_cmd_arg arg;
   arg.rectangle = rectangle;
   return arg;
}


static inline union lp_rast_cmd_arg
lp_rast_arg_state(const struct lp_rast_state *state)
{
   union lp_rast_cmd_arg arg;
   arg.set_state = state;
   return arg;
}


static inline union lp_rast_cmd_arg
lp_rast_arg_fence(struct lp_fence *fence)
{
   union lp_rast_cmd_arg arg;
   arg.fence = fence;
   return arg;
}


static inline union lp_rast_cmd_arg
lp_rast_arg_clearzs(uint64_t value, uint64_t mask)
{
   union lp_rast_cmd_arg arg;
   arg.clear_zstencil.value = value;
   arg.clear_zstencil.mask = mask;
   return arg;
}


static inline union lp_rast_cmd_arg
lp_rast_arg_query(struct llvmpipe_query *pq)
{
   union lp_rast_cmd_arg arg;
   arg.query_obj = pq;
   return arg;
}


static inline union lp_rast_cmd_arg
lp_rast_arg_null(void)
{
   union lp_rast_cmd_arg arg;
   arg.set_state = NULL;
   return arg;
}


/**
 * Binnable Commands.
 * These get put into bins by the setup code and are called when
 * the bins are executed.
 */
enum lp_rast_op {
  LP_RAST_OP_CLEAR_COLOR =       0x0,
  LP_RAST_OP_CLEAR_ZSTENCIL =    0x1,
  LP_RAST_OP_TRIANGLE_1 =        0x2,
  LP_RAST_OP_TRIANGLE_2 =        0x3,
  LP_RAST_OP_TRIANGLE_3 =        0x4,
  LP_RAST_OP_TRIANGLE_4 =        0x5,
  LP_RAST_OP_TRIANGLE_5 =        0x6,
  LP_RAST_OP_TRIANGLE_6 =        0x7,
  LP_RAST_OP_TRIANGLE_7 =        0x8,
  LP_RAST_OP_TRIANGLE_8 =        0x9,
  LP_RAST_OP_TRIANGLE_3_4 =      0xa,
  LP_RAST_OP_TRIANGLE_3_16 =     0xb,
  LP_RAST_OP_TRIANGLE_4_16 =     0xc,
  LP_RAST_OP_SHADE_TILE =        0xd,
  LP_RAST_OP_SHADE_TILE_OPAQUE = 0xe,
  LP_RAST_OP_BEGIN_QUERY =       0xf,
  LP_RAST_OP_END_QUERY =         0x10,
  LP_RAST_OP_SET_STATE =         0x11,
  LP_RAST_OP_TRIANGLE_32_1 =     0x12,
  LP_RAST_OP_TRIANGLE_32_2 =     0x13,
  LP_RAST_OP_TRIANGLE_32_3 =     0x14,
  LP_RAST_OP_TRIANGLE_32_4 =     0x15,
  LP_RAST_OP_TRIANGLE_32_5 =     0x16,
  LP_RAST_OP_TRIANGLE_32_6 =     0x17,
  LP_RAST_OP_TRIANGLE_32_7 =     0x18,
  LP_RAST_OP_TRIANGLE_32_8 =     0x19,
  LP_RAST_OP_TRIANGLE_32_3_4 =   0x1a,
  LP_RAST_OP_TRIANGLE_32_3_16 =  0x1b,
  LP_RAST_OP_TRIANGLE_32_4_16 =  0x1c,
  LP_RAST_OP_MS_TRIANGLE_1 =     0x1d,
  LP_RAST_OP_MS_TRIANGLE_2 =     0x1e,
  LP_RAST_OP_MS_TRIANGLE_3 =     0x1f,
  LP_RAST_OP_MS_TRIANGLE_4 =     0x20,
  LP_RAST_OP_MS_TRIANGLE_5 =     0x21,
  LP_RAST_OP_MS_TRIANGLE_6 =     0x22,
  LP_RAST_OP_MS_TRIANGLE_7 =     0x23,
  LP_RAST_OP_MS_TRIANGLE_8 =     0x24,
  LP_RAST_OP_MS_TRIANGLE_3_4 =   0x25,
  LP_RAST_OP_MS_TRIANGLE_3_16 =  0x26,
  LP_RAST_OP_MS_TRIANGLE_4_16 =  0x27,
  LP_RAST_OP_RECTANGLE =         0x28,  /* Keep at end */
  LP_RAST_OP_BLIT =              0x29,  /* Keep at end */
  LP_RAST_OP_MAX =               0x2a,
  LP_RAST_OP_MASK =              0xff
};


/* Returned by characterize_bin:
 */
#define LP_RAST_FLAGS_TRI            (0x1)
#define LP_RAST_FLAGS_RECT           (0x2)
#define LP_RAST_FLAGS_TILE           (0x4)
#define LP_RAST_FLAGS_BLIT           (0x8)

struct lp_bin_info {
   unsigned type:8;    // bitmask of LP_RAST_FLAGS_x
   unsigned count:24;
};

struct lp_bin_info
lp_characterize_bin(const struct cmd_bin *bin);

void
lp_debug_bins(struct lp_scene *scene);

void
lp_debug_draw_bins_by_cmd_length(struct lp_scene *scene);

void
lp_debug_draw_bins_by_coverage(struct lp_scene *scene);

void
lp_rast_fence(struct lp_rasterizer *rast,
              struct lp_fence **fence);

#endif
