/*
 * Copyright © 2023 Imagination Technologies Ltd.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the next
 * paragraph) shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#ifndef PVR_USCGEN_H
#define PVR_USCGEN_H

#include <stdbool.h>
#include <stdint.h>

#include "pvr_common.h"
#include "pvr_formats.h"
#include "util/u_dynarray.h"

enum pvr_int_coord_set_floats {
   PVR_INT_COORD_SET_FLOATS_0 = 0,
   PVR_INT_COORD_SET_FLOATS_4 = 1,
   /* For rate changes to 0 base screen space. */
   PVR_INT_COORD_SET_FLOATS_6 = 2,
   PVR_INT_COORD_SET_FLOATS_NUM = 3
};

struct pvr_tq_shader_properties {
   /* Controls whether this is an iterated shader. */
   bool iterated;

   /* Controls whether this is meant to be running at full rate. */
   bool full_rate;

   /* Sample specific channel of pixel. */
   bool pick_component;

   struct pvr_tq_layer_properties {
      /* Controls whether we need to send the sample count to the TPU. */
      bool msaa;

      /* In case we run pixel rate, to do an USC resolve - but still in MSAA TPU
       * samples.
       */
      uint32_t sample_count;

      enum pvr_resolve_op resolve_op;

      /* Selects the pixel conversion that we have to perform. */
      enum pvr_transfer_pbe_pixel_src pbe_format;

      /* Sampling from a 3D texture with a constant Z position. */
      bool sample;

      /* Number of float coefficients to get from screen space to texture space.
       */
      enum pvr_int_coord_set_floats layer_floats;

      /* Unaligned texture address in bytes. */
      uint32_t byte_unwind;

      /* Enable bilinear filter in shader. */
      bool linear;
   } layer_props;
};

/* All offsets are in dwords. */
/* Devices may have more than 256 sh regs but we're expecting to use vary few so
 * let's use uint8_t.
 */
struct pvr_tq_frag_sh_reg_layout {
   struct {
      /* How many image sampler descriptors are present. */
      uint8_t count;
      /* TODO: See if we ever need more than one combined image sampler
       * descriptor. If this is linked to the amount of layers used, we only
       * ever use one layer so this wouldn't need to be an array.
       */
      struct {
         uint8_t image;
         uint8_t sampler;
      } offsets[PVR_TRANSFER_MAX_IMAGES];
   } combined_image_samplers;

   /* TODO: Dynamic consts are used for various things so do this properly by
    * having an actual layout instead of chucking them all together using an
    * implicit layout.
    */
   struct {
      /* How many dynamic consts regs have been allocated. */
      uint8_t count;
      uint8_t offset;
   } dynamic_consts;

   /* Total sh regs allocated by the driver. It does not include the regs
    * necessary for compiler_out.
    */
   uint8_t driver_total;

   /* Provided by the compiler to the driver to be appended to the shareds. */
   /* No offset field since these will be appended at the end so driver_total
    * can be used instead.
    */
   struct {
      struct {
         /* TODO: Remove this count and just use `compiler_out_total`? Or remove
          * that one and use this one?
          */
         uint8_t count;
         /* TODO: The array size is chosen arbitrarily based on the max
          * constants currently produced by the compiler. Make this dynamic?
          */
         /* Values to fill in into each shared reg used for usc constants. */
         uint32_t values[10];
      } usc_constants;
   } compiler_out;

   /* Total extra sh regs needed by the compiler that need to be appended to the
    * shareds by the driver.
    */
   uint8_t compiler_out_total;
};

/* TODO: Shader caching (not pipeline caching) support. */

void pvr_uscgen_eot(const char *name,
                    uint32_t emit_count,
                    const uint32_t *emit_state,
                    unsigned *temps_used,
                    struct util_dynarray *binary);

void pvr_uscgen_nop(struct util_dynarray *binary);

void pvr_uscgen_tq_frag(const struct pvr_tq_shader_properties *shader_props,
                        struct pvr_tq_frag_sh_reg_layout *sh_reg_layout,
                        unsigned *temps_used,
                        struct util_dynarray *binary);

void pvr_uscgen_tq_eot(unsigned rt_count,
                       const uint64_t *pbe_regs,
                       struct util_dynarray *binary);

#endif /* PVR_USCGEN_H */
