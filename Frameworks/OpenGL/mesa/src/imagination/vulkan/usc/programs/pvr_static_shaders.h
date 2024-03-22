/*
 * Copyright © 2022 Imagination Technologies Ltd.
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

#ifndef PVR_STATIC_SHADERS_H
#define PVR_STATIC_SHADERS_H

#include <stdint.h>
#include <stddef.h>

/* TODO: Remove this once compiler is integrated. */
#define PVR_INVALID_INST (~0)

struct pvr_static_buffer {
   uint32_t dst_idx;
   uint32_t value;
};

struct pvr_shader_factory_info {
   uint32_t temps_required;
   uint32_t const_shared_regs;
   uint32_t coeff_regs;
   uint32_t input_regs;
   uint32_t explicit_const_start_offset;
   uint32_t code_size;
   const uint8_t *const shader_code;
   uint32_t const_calc_prog_inst_bytes;
   uint32_t sec_temp_regs;
   const uint8_t *const_calc_program;
   uint32_t coeff_update_prog_start;
   uint32_t coeff_update_temp_regs;
   const uint32_t *driver_const_location_map;
   uint32_t num_driver_consts;
   const struct pvr_static_buffer *static_const_buffer;
   uint32_t num_static_const;
   uint32_t msaa_sample_count;
};

static const uint8_t availability_query_write_shader[144] = { 0 };

static const uint32_t availability_query_write_location_map[1] = {
   0,
};

static const struct pvr_static_buffer
   availability_query_write_static_consts[3] = {
      { 0, 0 },
      { 0, 0 },
      { 0, 0 },
   };

static const struct pvr_shader_factory_info availability_query_write_info = {
   0,
   0,
   0,
   0,
   0,
   sizeof(availability_query_write_shader),
   availability_query_write_shader,
   0,
   0,
   NULL,
   PVR_INVALID_INST,
   0,
   availability_query_write_location_map,
   0,
   availability_query_write_static_consts,
   0,
   ~0,
};

static const uint8_t copy_query_results_shader[384] = { 0 };

static const uint32_t copy_query_results_location_map[7] = {
   0, 0, 0, 0, 0, 0, 0,
};

static const struct pvr_static_buffer copy_query_results_static_consts[2] = {
   { 0, 0 },
   { 0, 0 },
};

static const struct pvr_shader_factory_info copy_query_results_info = {
   0,
   0,
   0,
   0,
   0,
   sizeof(copy_query_results_shader),
   copy_query_results_shader,
   0,
   0,
   NULL,
   PVR_INVALID_INST,
   0,
   copy_query_results_location_map,
   0,
   copy_query_results_static_consts,
   0,
   ~0,
};

static const uint8_t reset_query_shader_code[136] = { 0 };

static const uint32_t reset_query_location_map[1] = {
   0,
};

static const struct pvr_static_buffer reset_query_static_consts[2] = {
   { 0, 0 },
   { 0, 0 },
};

static const struct pvr_shader_factory_info reset_query_info = {
   0,
   0,
   0,
   0,
   0,
   sizeof(reset_query_shader_code),
   reset_query_shader_code,
   0,
   0,
   NULL,
   PVR_INVALID_INST,
   0,
   reset_query_location_map,
   0,
   reset_query_static_consts,
   0,
   ~0,
};

static const struct pvr_shader_factory_info
   *const copy_query_results_collection[1] = {
      &copy_query_results_info,
   };

static const struct pvr_shader_factory_info *const reset_query_collection[1] = {
   &reset_query_info,
};

static const uint8_t clear_attachments_1_dw_0_offt_out_reg_shader_code[8] = {
   0
};
static const uint32_t clear_attachments_1_dw_0_offt_out_reg_const_dest[6] = {
   0
};
static const struct pvr_shader_factory_info
   clear_attachments_1_dw_0_offt_out_reg_info = {
      0,
      0,
      0,
      0,
      0,
      sizeof(clear_attachments_1_dw_0_offt_out_reg_shader_code),
      clear_attachments_1_dw_0_offt_out_reg_shader_code,
      0,
      0,
      NULL,
      0,
      0,
      clear_attachments_1_dw_0_offt_out_reg_const_dest,
      0,
      NULL,
      0,
      1,
   };

static const uint8_t clear_attachments_1_dw_0_offt_out_mem_shader_code[8] = {
   0,
};
static const uint32_t clear_attachments_1_dw_0_offt_out_mem_const_dest[6] = {
   0,
};
static const struct pvr_shader_factory_info
   clear_attachments_1_dw_0_offt_out_mem_info = {
      0,
      0,
      0,
      0,
      0,
      sizeof(clear_attachments_1_dw_0_offt_out_mem_shader_code),
      clear_attachments_1_dw_0_offt_out_mem_shader_code,
      0,
      0,
      NULL,
      0,
      0,
      clear_attachments_1_dw_0_offt_out_mem_const_dest,
      0,
      NULL,
      0,
      1,
   };

static const uint8_t clear_attachments_2_dw_0_offt_out_reg_shader_code[8] = {
   0
};
static const uint32_t clear_attachments_2_dw_0_offt_out_reg_const_dest[6] = {
   0,
};
static const struct pvr_shader_factory_info
   clear_attachments_2_dw_0_offt_out_reg_info = {
      0,
      0,
      0,
      0,
      0,
      sizeof(clear_attachments_2_dw_0_offt_out_reg_shader_code),
      clear_attachments_2_dw_0_offt_out_reg_shader_code,
      0,
      0,
      NULL,
      0,
      0,
      clear_attachments_2_dw_0_offt_out_reg_const_dest,
      0,
      NULL,
      0,
      1,
   };

static const uint8_t clear_attachments_2_dw_0_offt_out_mem_shader_code[8] = {
   0
};
static const uint32_t clear_attachments_2_dw_0_offt_out_mem_const_dest[6] = {
   0,
};
static const struct pvr_shader_factory_info
   clear_attachments_2_dw_0_offt_out_mem_info = {
      0,
      0,
      0,
      0,
      0,
      sizeof(clear_attachments_2_dw_0_offt_out_mem_shader_code),
      clear_attachments_2_dw_0_offt_out_mem_shader_code,
      0,
      0,
      NULL,
      0,
      0,
      clear_attachments_2_dw_0_offt_out_mem_const_dest,
      0,
      NULL,
      0,
      1,
   };

static const uint8_t clear_attachments_3_dw_0_offt_out_reg_shader_code[8] = {
   0
};
static const uint32_t clear_attachments_3_dw_0_offt_out_reg_const_dest[6] = {
   0,
};
static const struct pvr_shader_factory_info
   clear_attachments_3_dw_0_offt_out_reg_info = {
      0,
      0,
      0,
      0,
      0,
      sizeof(clear_attachments_3_dw_0_offt_out_reg_shader_code),
      clear_attachments_3_dw_0_offt_out_reg_shader_code,
      0,
      0,
      NULL,
      0,
      0,
      clear_attachments_3_dw_0_offt_out_reg_const_dest,
      0,
      NULL,
      0,
      1,
   };

static const uint8_t clear_attachments_3_dw_0_offt_out_mem_shader_code[8] = {
   0
};
static const uint32_t clear_attachments_3_dw_0_offt_out_mem_const_dest[6] = {
   0,
};
static const struct pvr_shader_factory_info
   clear_attachments_3_dw_0_offt_out_mem_info = {
      0,
      0,
      0,
      0,
      0,
      sizeof(clear_attachments_3_dw_0_offt_out_mem_shader_code),
      clear_attachments_3_dw_0_offt_out_mem_shader_code,
      0,
      0,
      NULL,
      0,
      0,
      clear_attachments_3_dw_0_offt_out_mem_const_dest,
      0,
      NULL,
      0,
      1,
   };

static const uint8_t clear_attachments_4_dw_0_offt_out_reg_shader_code[8] = {
   0
};
static const uint32_t clear_attachments_4_dw_0_offt_out_reg_const_dest[6] = {
   0,
};
static const struct pvr_shader_factory_info
   clear_attachments_4_dw_0_offt_out_reg_info = {
      0,
      0,
      0,
      0,
      0,
      sizeof(clear_attachments_4_dw_0_offt_out_reg_shader_code),
      clear_attachments_4_dw_0_offt_out_reg_shader_code,
      0,
      0,
      NULL,
      0,
      0,
      clear_attachments_4_dw_0_offt_out_reg_const_dest,
      0,
      NULL,
      0,
      1,
   };

static const uint8_t clear_attachments_4_dw_0_offt_out_mem_shader_code[8] = {
   0
};
static const uint32_t clear_attachments_4_dw_0_offt_out_mem_const_dest[6] = {
   0,
};
static const struct pvr_shader_factory_info
   clear_attachments_4_dw_0_offt_out_mem_info = {
      0,
      0,
      0,
      0,
      0,
      sizeof(clear_attachments_4_dw_0_offt_out_mem_shader_code),
      clear_attachments_4_dw_0_offt_out_mem_shader_code,
      0,
      0,
      NULL,
      0,
      0,
      clear_attachments_4_dw_0_offt_out_mem_const_dest,
      0,
      NULL,
      0,
      1,
   };

static const uint8_t clear_attachments_1_dw_1_offt_out_reg_shader_code[8] = {
   0
};
static const uint32_t clear_attachments_1_dw_1_offt_out_reg_const_dest[6] = {
   0,
};
static const struct pvr_shader_factory_info
   clear_attachments_1_dw_1_offt_out_reg_info = {
      0,
      0,
      0,
      0,
      0,
      sizeof(clear_attachments_1_dw_1_offt_out_reg_shader_code),
      clear_attachments_1_dw_1_offt_out_reg_shader_code,
      0,
      0,
      NULL,
      0,
      0,
      clear_attachments_1_dw_1_offt_out_reg_const_dest,
      0,
      NULL,
      0,
      1,
   };

static const uint8_t clear_attachments_1_dw_1_offt_out_mem_shader_code[8] = {
   0
};
static const uint32_t clear_attachments_1_dw_1_offt_out_mem_const_dest[6] = {
   0,
};
static const struct pvr_shader_factory_info
   clear_attachments_1_dw_1_offt_out_mem_info = {
      0,
      0,
      0,
      0,
      0,
      sizeof(clear_attachments_1_dw_1_offt_out_mem_shader_code),
      clear_attachments_1_dw_1_offt_out_mem_shader_code,
      0,
      0,
      NULL,
      0,
      0,
      clear_attachments_1_dw_1_offt_out_mem_const_dest,
      0,
      NULL,
      0,
      1,
   };

static const uint8_t clear_attachments_2_dw_1_offt_out_reg_shader_code[8] = {
   0
};
static const uint32_t clear_attachments_2_dw_1_offt_out_reg_const_dest[6] = {
   0,
};
static const struct pvr_shader_factory_info
   clear_attachments_2_dw_1_offt_out_reg_info = {
      0,
      0,
      0,
      0,
      0,
      sizeof(clear_attachments_2_dw_1_offt_out_reg_shader_code),
      clear_attachments_2_dw_1_offt_out_reg_shader_code,
      0,
      0,
      NULL,
      0,
      0,
      clear_attachments_2_dw_1_offt_out_reg_const_dest,
      0,
      NULL,
      0,
      1,
   };

static const uint8_t clear_attachments_2_dw_1_offt_out_mem_shader_code[8] = {
   0
};
static const uint32_t clear_attachments_2_dw_1_offt_out_mem_const_dest[6] = {
   0,
};
static const struct pvr_shader_factory_info
   clear_attachments_2_dw_1_offt_out_mem_info = {
      0,
      0,
      0,
      0,
      0,
      sizeof(clear_attachments_2_dw_1_offt_out_mem_shader_code),
      clear_attachments_2_dw_1_offt_out_mem_shader_code,
      0,
      0,
      NULL,
      0,
      0,
      clear_attachments_2_dw_1_offt_out_mem_const_dest,
      0,
      NULL,
      0,
      1,
   };

static const uint8_t clear_attachments_3_dw_1_offt_out_reg_shader_code[8] = {
   0
};
static const uint32_t clear_attachments_3_dw_1_offt_out_reg_const_dest[6] = {
   0,
};
static const struct pvr_shader_factory_info
   clear_attachments_3_dw_1_offt_out_reg_info = {
      0,
      0,
      0,
      0,
      0,
      sizeof(clear_attachments_3_dw_1_offt_out_reg_shader_code),
      clear_attachments_3_dw_1_offt_out_reg_shader_code,
      0,
      0,
      NULL,
      0,
      0,
      clear_attachments_3_dw_1_offt_out_reg_const_dest,
      0,
      NULL,
      0,
      1,
   };

static const uint8_t clear_attachments_3_dw_1_offt_out_mem_shader_code[8] = {
   0
};
static const uint32_t clear_attachments_3_dw_1_offt_out_mem_const_dest[6] = {
   0,
};
static const struct pvr_shader_factory_info
   clear_attachments_3_dw_1_offt_out_mem_info = {
      0,
      0,
      0,
      0,
      0,
      sizeof(clear_attachments_3_dw_1_offt_out_mem_shader_code),
      clear_attachments_3_dw_1_offt_out_mem_shader_code,
      0,
      0,
      NULL,
      0,
      0,
      clear_attachments_3_dw_1_offt_out_mem_const_dest,
      0,
      NULL,
      0,
      1,
   };

static const uint8_t clear_attachments_4_dw_1_offt_out_reg_shader_code[8] = {
   0
};
static const uint32_t clear_attachments_4_dw_1_offt_out_reg_const_dest[6] = {
   0,
};
static const struct pvr_shader_factory_info
   clear_attachments_4_dw_1_offt_out_reg_info = {
      0,
      0,
      0,
      0,
      0,
      sizeof(clear_attachments_4_dw_1_offt_out_reg_shader_code),
      clear_attachments_4_dw_1_offt_out_reg_shader_code,
      0,
      0,
      NULL,
      0,
      0,
      clear_attachments_4_dw_1_offt_out_reg_const_dest,
      0,
      NULL,
      0,
   };

static const uint8_t clear_attachments_4_dw_1_offt_out_mem_shader_code[8] = {
   0
};
static const uint32_t clear_attachments_4_dw_1_offt_out_mem_const_dest[6] = {
   0,
};
static const struct pvr_shader_factory_info
   clear_attachments_4_dw_1_offt_out_mem_info = {
      0,
      0,
      0,
      0,
      0,
      sizeof(clear_attachments_4_dw_1_offt_out_mem_shader_code),
      clear_attachments_4_dw_1_offt_out_mem_shader_code,
      0,
      0,
      NULL,
      0,
      0,
      clear_attachments_4_dw_1_offt_out_mem_const_dest,
      0,
      NULL,
      0,
   };

static const uint8_t clear_attachments_1_dw_2_offt_out_reg_shader_code[8] = {
   0
};
static const uint32_t clear_attachments_1_dw_2_offt_out_reg_const_dest[6] = {
   0,
};
static const struct pvr_shader_factory_info
   clear_attachments_1_dw_2_offt_out_reg_info = {
      0,
      0,
      0,
      0,
      0,
      sizeof(clear_attachments_1_dw_2_offt_out_reg_shader_code),
      clear_attachments_1_dw_2_offt_out_reg_shader_code,
      0,
      0,
      NULL,
      0,
      0,
      clear_attachments_1_dw_2_offt_out_reg_const_dest,
      0,
      NULL,
      0,
      1,
   };

static const uint8_t clear_attachments_1_dw_2_offt_out_mem_shader_code[8] = {
   0
};
static const uint32_t clear_attachments_1_dw_2_offt_out_mem_const_dest[6] = {
   0,
};
static const struct pvr_shader_factory_info
   clear_attachments_1_dw_2_offt_out_mem_info = {
      0,
      0,
      0,
      0,
      0,
      sizeof(clear_attachments_1_dw_2_offt_out_mem_shader_code),
      clear_attachments_1_dw_2_offt_out_mem_shader_code,
      0,
      0,
      NULL,
      0,
      0,
      clear_attachments_1_dw_2_offt_out_mem_const_dest,
      0,
      NULL,
      0,
      1,
   };

static const uint8_t clear_attachments_2_dw_2_offt_out_reg_shader_code[8] = {
   0
};
static const uint32_t clear_attachments_2_dw_2_offt_out_reg_const_dest[6] = {
   0,
};
static const struct pvr_shader_factory_info
   clear_attachments_2_dw_2_offt_out_reg_info = {
      0,
      0,
      0,
      0,
      0,
      sizeof(clear_attachments_2_dw_2_offt_out_reg_shader_code),
      clear_attachments_2_dw_2_offt_out_reg_shader_code,
      0,
      0,
      NULL,
      0,
      0,
      clear_attachments_2_dw_2_offt_out_reg_const_dest,
      0,
      NULL,
      0,
      1,
   };

static const uint8_t clear_attachments_2_dw_2_offt_out_mem_shader_code[8] = {
   0
};
static const uint32_t clear_attachments_2_dw_2_offt_out_mem_const_dest[6] = {
   0,
};
static const struct pvr_shader_factory_info
   clear_attachments_2_dw_2_offt_out_mem_info = {
      0,
      0,
      0,
      0,
      0,
      sizeof(clear_attachments_2_dw_2_offt_out_mem_shader_code),
      clear_attachments_2_dw_2_offt_out_mem_shader_code,
      0,
      0,
      NULL,
      0,
      0,
      clear_attachments_2_dw_2_offt_out_mem_const_dest,
      0,
      NULL,
      0,
      1,
   };

static const uint8_t clear_attachments_3_dw_2_offt_out_reg_shader_code[8] = {
   0
};
static const uint32_t clear_attachments_3_dw_2_offt_out_reg_const_dest[6] = {
   0,
};
static const struct pvr_shader_factory_info
   clear_attachments_3_dw_2_offt_out_reg_info = {
      0,
      0,
      0,
      0,
      0,
      sizeof(clear_attachments_3_dw_2_offt_out_reg_shader_code),
      clear_attachments_3_dw_2_offt_out_reg_shader_code,
      0,
      0,
      NULL,
      0,
      0,
      clear_attachments_3_dw_2_offt_out_reg_const_dest,
      0,
      NULL,
      0,
   };

static const uint8_t clear_attachments_3_dw_2_offt_out_mem_shader_code[8] = {
   0
};
static const uint32_t clear_attachments_3_dw_2_offt_out_mem_const_dest[6] = {
   0,
};
static const struct pvr_shader_factory_info
   clear_attachments_3_dw_2_offt_out_mem_info = {
      0,
      0,
      0,
      0,
      0,
      sizeof(clear_attachments_3_dw_2_offt_out_mem_shader_code),
      clear_attachments_3_dw_2_offt_out_mem_shader_code,
      0,
      0,
      NULL,
      0,
      0,
      clear_attachments_3_dw_2_offt_out_mem_const_dest,
      0,
      NULL,
      0,
   };

static const uint8_t clear_attachments_4_dw_2_offt_out_reg_shader_code[8] = {
   0
};
static const uint32_t clear_attachments_4_dw_2_offt_out_reg_const_dest[6] = {
   0,
};
static const struct pvr_shader_factory_info
   clear_attachments_4_dw_2_offt_out_reg_info = {
      0,
      0,
      0,
      0,
      0,
      sizeof(clear_attachments_4_dw_2_offt_out_reg_shader_code),
      clear_attachments_4_dw_2_offt_out_reg_shader_code,
      0,
      0,
      NULL,
      0,
      0,
      clear_attachments_4_dw_2_offt_out_reg_const_dest,
      0,
      NULL,
      0,
   };

static const uint8_t clear_attachments_4_dw_2_offt_out_mem_shader_code[8] = {
   0
};
static const uint32_t clear_attachments_4_dw_2_offt_out_mem_const_dest[6] = {
   0,
};
static const struct pvr_shader_factory_info
   clear_attachments_4_dw_2_offt_out_mem_info = {
      0,
      0,
      0,
      0,
      0,
      sizeof(clear_attachments_4_dw_2_offt_out_mem_shader_code),
      clear_attachments_4_dw_2_offt_out_mem_shader_code,
      0,
      0,
      NULL,
      0,
      0,
      clear_attachments_4_dw_2_offt_out_mem_const_dest,
      0,
      NULL,
      0,
   };

static const uint8_t clear_attachments_1_dw_3_offt_out_reg_shader_code[8] = {
   0
};
static const uint32_t clear_attachments_1_dw_3_offt_out_reg_const_dest[6] = {
   0,
};
static const struct pvr_shader_factory_info
   clear_attachments_1_dw_3_offt_out_reg_info = {
      0,
      0,
      0,
      0,
      0,
      sizeof(clear_attachments_1_dw_3_offt_out_reg_shader_code),
      clear_attachments_1_dw_3_offt_out_reg_shader_code,
      0,
      0,
      NULL,
      0,
      0,
      clear_attachments_1_dw_3_offt_out_reg_const_dest,
      0,
      NULL,
      0,
      1,
   };

static const uint8_t clear_attachments_1_dw_3_offt_out_mem_shader_code[8] = {
   0
};
static const uint32_t clear_attachments_1_dw_3_offt_out_mem_const_dest[6] = {
   0,
};
static const struct pvr_shader_factory_info
   clear_attachments_1_dw_3_offt_out_mem_info = {
      0,
      0,
      0,
      0,
      0,
      sizeof(clear_attachments_1_dw_3_offt_out_mem_shader_code),
      clear_attachments_1_dw_3_offt_out_mem_shader_code,
      0,
      0,
      NULL,
      0,
      0,
      clear_attachments_1_dw_3_offt_out_mem_const_dest,
      0,
      NULL,
      0,
   };

static const uint8_t clear_attachments_2_dw_3_offt_out_reg_shader_code[8] = {
   0
};
static const uint32_t clear_attachments_2_dw_3_offt_out_reg_const_dest[6] = {
   0,
};
static const struct pvr_shader_factory_info
   clear_attachments_2_dw_3_offt_out_reg_info = {
      0,
      0,
      0,
      0,
      0,
      sizeof(clear_attachments_2_dw_3_offt_out_reg_shader_code),
      clear_attachments_2_dw_3_offt_out_reg_shader_code,
      0,
      0,
      NULL,
      0,
      0,
      clear_attachments_2_dw_3_offt_out_reg_const_dest,
      0,
      NULL,
      0,
   };

static const uint8_t clear_attachments_2_dw_3_offt_out_mem_shader_code[8] = {
   0
};
static const uint32_t clear_attachments_2_dw_3_offt_out_mem_const_dest[6] = {
   0,
};
static const struct pvr_shader_factory_info
   clear_attachments_2_dw_3_offt_out_mem_info = {
      0,
      0,
      0,
      0,
      0,
      sizeof(clear_attachments_2_dw_3_offt_out_mem_shader_code),
      clear_attachments_2_dw_3_offt_out_mem_shader_code,
      0,
      0,
      NULL,
      0,
      0,
      clear_attachments_2_dw_3_offt_out_mem_const_dest,
      0,
      NULL,
      0,
   };

static const uint8_t clear_attachments_3_dw_3_offt_out_reg_shader_code[8] = {
   0
};
static const uint32_t clear_attachments_3_dw_3_offt_out_reg_const_dest[6] = {
   0,
};
static const struct pvr_shader_factory_info
   clear_attachments_3_dw_3_offt_out_reg_info = {
      0,
      0,
      0,
      0,
      0,
      sizeof(clear_attachments_3_dw_3_offt_out_reg_shader_code),
      clear_attachments_3_dw_3_offt_out_reg_shader_code,
      0,
      0,
      NULL,
      0,
      0,
      clear_attachments_3_dw_3_offt_out_reg_const_dest,
      0,
      NULL,
      0,
   };

static const uint8_t clear_attachments_3_dw_3_offt_out_mem_shader_code[8] = {
   0
};
static const uint32_t clear_attachments_3_dw_3_offt_out_mem_const_dest[6] = {
   0,
};
static const struct pvr_shader_factory_info
   clear_attachments_3_dw_3_offt_out_mem_info = {
      0,
      0,
      0,
      0,
      0,
      sizeof(clear_attachments_3_dw_3_offt_out_mem_shader_code),
      clear_attachments_3_dw_3_offt_out_mem_shader_code,
      0,
      0,
      NULL,
      0,
      0,
      clear_attachments_3_dw_3_offt_out_mem_const_dest,
      0,
      NULL,
      0,
   };

static const uint8_t clear_attachments_4_dw_3_offt_out_reg_shader_code[8] = {
   0
};
static const uint32_t clear_attachments_4_dw_3_offt_out_reg_const_dest[6] = {
   0,
};
static const struct pvr_shader_factory_info
   clear_attachments_4_dw_3_offt_out_reg_info = {
      0,
      0,
      0,
      0,
      0,
      sizeof(clear_attachments_4_dw_3_offt_out_reg_shader_code),
      clear_attachments_4_dw_3_offt_out_reg_shader_code,
      0,
      0,
      NULL,
      0,
      0,
      clear_attachments_4_dw_3_offt_out_reg_const_dest,
      0,
      NULL,
      0,
   };

static const uint8_t clear_attachments_4_dw_3_offt_out_mem_shader_code[8] = {
   0
};
static const uint32_t clear_attachments_4_dw_3_offt_out_mem_const_dest[6] = {
   0,
};
static const struct pvr_shader_factory_info
   clear_attachments_4_dw_3_offt_out_mem_info = {
      0,
      0,
      0,
      0,
      0,
      sizeof(clear_attachments_4_dw_3_offt_out_mem_shader_code),
      clear_attachments_4_dw_3_offt_out_mem_shader_code,
      0,
      0,
      NULL,
      0,
      0,
      clear_attachments_4_dw_3_offt_out_mem_const_dest,
      0,
      NULL,
      0,
   };

static const uint8_t clear_attachments_1_dw_4_offt_out_reg_shader_code[8] = {
   0
};
static const uint32_t clear_attachments_1_dw_4_offt_out_reg_const_dest[6] = {
   0,
};
static const struct pvr_shader_factory_info
   clear_attachments_1_dw_4_offt_out_reg_info = {
      0,
      0,
      0,
      0,
      0,
      sizeof(clear_attachments_1_dw_4_offt_out_reg_shader_code),
      clear_attachments_1_dw_4_offt_out_reg_shader_code,
      0,
      0,
      NULL,
      0,
      0,
      clear_attachments_1_dw_4_offt_out_reg_const_dest,
      0,
      NULL,
      0,
   };

static const uint8_t clear_attachments_1_dw_4_offt_out_mem_shader_code[8] = {
   0
};
static const uint32_t clear_attachments_1_dw_4_offt_out_mem_const_dest[6] = {
   0,
};
static const struct pvr_shader_factory_info
   clear_attachments_1_dw_4_offt_out_mem_info = {
      0,
      0,
      0,
      0,
      0,
      sizeof(clear_attachments_1_dw_4_offt_out_mem_shader_code),
      clear_attachments_1_dw_4_offt_out_mem_shader_code,
      0,
      0,
      NULL,
      0,
      0,
      clear_attachments_1_dw_4_offt_out_mem_const_dest,
      0,
      NULL,
      0,
   };

static const uint8_t clear_attachments_2_dw_4_offt_out_reg_shader_code[8] = {
   0
};
static const uint32_t clear_attachments_2_dw_4_offt_out_reg_const_dest[6] = {
   0,
};
static const struct pvr_shader_factory_info
   clear_attachments_2_dw_4_offt_out_reg_info = {
      0,
      0,
      0,
      0,
      0,
      sizeof(clear_attachments_2_dw_4_offt_out_reg_shader_code),
      clear_attachments_2_dw_4_offt_out_reg_shader_code,
      0,
      0,
      NULL,
      0,
      0,
      clear_attachments_2_dw_4_offt_out_reg_const_dest,
      0,
      NULL,
      0,
   };

static const uint8_t clear_attachments_2_dw_4_offt_out_mem_shader_code[8] = {
   0
};
static const uint32_t clear_attachments_2_dw_4_offt_out_mem_const_dest[6] = {
   0,
};
static const struct pvr_shader_factory_info
   clear_attachments_2_dw_4_offt_out_mem_info = {
      0,
      0,
      0,
      0,
      0,
      sizeof(clear_attachments_2_dw_4_offt_out_mem_shader_code),
      clear_attachments_2_dw_4_offt_out_mem_shader_code,
      0,
      0,
      NULL,
      0,
      0,
      clear_attachments_2_dw_4_offt_out_mem_const_dest,
      0,
      NULL,
      0,
   };

static const uint8_t clear_attachments_3_dw_4_offt_out_reg_shader_code[8] = {
   0
};
static const uint32_t clear_attachments_3_dw_4_offt_out_reg_const_dest[6] = {
   0,
};
static const struct pvr_shader_factory_info
   clear_attachments_3_dw_4_offt_out_reg_info = {
      0,
      0,
      0,
      0,
      0,
      sizeof(clear_attachments_3_dw_4_offt_out_reg_shader_code),
      clear_attachments_3_dw_4_offt_out_reg_shader_code,
      0,
      0,
      NULL,
      0,
      0,
      clear_attachments_3_dw_4_offt_out_reg_const_dest,
      0,
      NULL,
      0,
   };

static const uint8_t clear_attachments_3_dw_4_offt_out_mem_shader_code[8] = {
   0
};
static const uint32_t clear_attachments_3_dw_4_offt_out_mem_const_dest[6] = {
   0,
};
static const struct pvr_shader_factory_info
   clear_attachments_3_dw_4_offt_out_mem_info = {
      0,
      0,
      0,
      0,
      0,
      sizeof(clear_attachments_3_dw_4_offt_out_mem_shader_code),
      clear_attachments_3_dw_4_offt_out_mem_shader_code,
      0,
      0,
      NULL,
      0,
      0,
      clear_attachments_3_dw_4_offt_out_mem_const_dest,
      0,
      NULL,
      0,
   };

static const uint8_t clear_attachments_4_dw_4_offt_out_reg_shader_code[8] = {
   0
};
static const uint32_t clear_attachments_4_dw_4_offt_out_reg_const_dest[6] = {
   0,
};
static const struct pvr_shader_factory_info
   clear_attachments_4_dw_4_offt_out_reg_info = {
      0,
      0,
      0,
      0,
      0,
      sizeof(clear_attachments_4_dw_4_offt_out_reg_shader_code),
      clear_attachments_4_dw_4_offt_out_reg_shader_code,
      0,
      0,
      NULL,
      0,
      0,
      clear_attachments_4_dw_4_offt_out_reg_const_dest,
      0,
      NULL,
      0,
   };

static const uint8_t clear_attachments_4_dw_4_offt_out_mem_shader_code[8] = {
   0
};
static const uint32_t clear_attachments_4_dw_4_offt_out_mem_const_dest[6] = {
   0,
};
static const struct pvr_shader_factory_info
   clear_attachments_4_dw_4_offt_out_mem_info = {
      0,
      0,
      0,
      0,
      0,
      sizeof(clear_attachments_4_dw_4_offt_out_mem_shader_code),
      clear_attachments_4_dw_4_offt_out_mem_shader_code,
      0,
      0,
      NULL,
      0,
      0,
      clear_attachments_4_dw_4_offt_out_mem_const_dest,
      0,
      NULL,
      0,
   };

static const uint8_t clear_attachments_1_dw_5_offt_out_reg_shader_code[8] = {
   0
};
static const uint32_t clear_attachments_1_dw_5_offt_out_reg_const_dest[6] = {
   0,
};
static const struct pvr_shader_factory_info
   clear_attachments_1_dw_5_offt_out_reg_info = {
      0,
      0,
      0,
      0,
      0,
      sizeof(clear_attachments_1_dw_5_offt_out_reg_shader_code),
      clear_attachments_1_dw_5_offt_out_reg_shader_code,
      0,
      0,
      NULL,
      0,
      0,
      clear_attachments_1_dw_5_offt_out_reg_const_dest,
      0,
      NULL,
      0,
   };

static const uint8_t clear_attachments_1_dw_5_offt_out_mem_shader_code[8] = {
   0
};
static const uint32_t clear_attachments_1_dw_5_offt_out_mem_const_dest[6] = {
   0,
};
static const struct pvr_shader_factory_info
   clear_attachments_1_dw_5_offt_out_mem_info = {
      0,
      0,
      0,
      0,
      0,
      sizeof(clear_attachments_1_dw_5_offt_out_mem_shader_code),
      clear_attachments_1_dw_5_offt_out_mem_shader_code,
      0,
      0,
      NULL,
      0,
      0,
      clear_attachments_1_dw_5_offt_out_mem_const_dest,
      0,
      NULL,
      0,
   };

static const uint8_t clear_attachments_2_dw_5_offt_out_reg_shader_code[8] = {
   0
};
static const uint32_t clear_attachments_2_dw_5_offt_out_reg_const_dest[6] = {
   0,
};
static const struct pvr_shader_factory_info
   clear_attachments_2_dw_5_offt_out_reg_info = {
      0,
      0,
      0,
      0,
      0,
      sizeof(clear_attachments_2_dw_5_offt_out_reg_shader_code),
      clear_attachments_2_dw_5_offt_out_reg_shader_code,
      0,
      0,
      NULL,
      0,
      0,
      clear_attachments_2_dw_5_offt_out_reg_const_dest,
      0,
      NULL,
      0,
   };

static const uint8_t clear_attachments_2_dw_5_offt_out_mem_shader_code[8] = {
   0
};
static const uint32_t clear_attachments_2_dw_5_offt_out_mem_const_dest[6] = {
   0,
};
static const struct pvr_shader_factory_info
   clear_attachments_2_dw_5_offt_out_mem_info = {
      0,
      0,
      0,
      0,
      0,
      sizeof(clear_attachments_2_dw_5_offt_out_mem_shader_code),
      clear_attachments_2_dw_5_offt_out_mem_shader_code,
      0,
      0,
      NULL,
      0,
      0,
      clear_attachments_2_dw_5_offt_out_mem_const_dest,
      0,
      NULL,
      0,
   };

static const uint8_t clear_attachments_3_dw_5_offt_out_reg_shader_code[8] = {
   0
};
static const uint32_t clear_attachments_3_dw_5_offt_out_reg_const_dest[6] = {
   0,
};
static const struct pvr_shader_factory_info
   clear_attachments_3_dw_5_offt_out_reg_info = {
      0,
      0,
      0,
      0,
      0,
      sizeof(clear_attachments_3_dw_5_offt_out_reg_shader_code),
      clear_attachments_3_dw_5_offt_out_reg_shader_code,
      0,
      0,
      NULL,
      0,
      0,
      clear_attachments_3_dw_5_offt_out_reg_const_dest,
      0,
      NULL,
      0,
   };

static const uint8_t clear_attachments_3_dw_5_offt_out_mem_shader_code[8] = {
   0
};
static const uint32_t clear_attachments_3_dw_5_offt_out_mem_const_dest[6] = {
   0,
};
static const struct pvr_shader_factory_info
   clear_attachments_3_dw_5_offt_out_mem_info = {
      0,
      0,
      0,
      0,
      0,
      sizeof(clear_attachments_3_dw_5_offt_out_mem_shader_code),
      clear_attachments_3_dw_5_offt_out_mem_shader_code,
      0,
      0,
      NULL,
      0,
      0,
      clear_attachments_3_dw_5_offt_out_mem_const_dest,
      0,
      NULL,
      0,
   };

static const uint8_t clear_attachments_1_dw_6_offt_out_reg_shader_code[8] = {
   0
};
static const uint32_t clear_attachments_1_dw_6_offt_out_reg_const_dest[6] = {
   0,
};
static const struct pvr_shader_factory_info
   clear_attachments_1_dw_6_offt_out_reg_info = {
      0,
      0,
      0,
      0,
      0,
      sizeof(clear_attachments_1_dw_6_offt_out_reg_shader_code),
      clear_attachments_1_dw_6_offt_out_reg_shader_code,
      0,
      0,
      NULL,
      0,
      0,
      clear_attachments_1_dw_6_offt_out_reg_const_dest,
      0,
      NULL,
      0,
   };

static const uint8_t clear_attachments_1_dw_6_offt_out_mem_shader_code[8] = {
   0
};
static const uint32_t clear_attachments_1_dw_6_offt_out_mem_const_dest[6] = {
   0,
};
static const struct pvr_shader_factory_info
   clear_attachments_1_dw_6_offt_out_mem_info = {
      0,
      0,
      0,
      0,
      0,
      sizeof(clear_attachments_1_dw_6_offt_out_mem_shader_code),
      clear_attachments_1_dw_6_offt_out_mem_shader_code,
      0,
      0,
      NULL,
      0,
      0,
      clear_attachments_1_dw_6_offt_out_mem_const_dest,
      0,
      NULL,
      0,
   };

static const uint8_t clear_attachments_2_dw_6_offt_out_reg_shader_code[8] = {
   0
};
static const uint32_t clear_attachments_2_dw_6_offt_out_reg_const_dest[6] = {
   0,
};
static const struct pvr_shader_factory_info
   clear_attachments_2_dw_6_offt_out_reg_info = {
      0,
      0,
      0,
      0,
      0,
      sizeof(clear_attachments_2_dw_6_offt_out_reg_shader_code),
      clear_attachments_2_dw_6_offt_out_reg_shader_code,
      0,
      0,
      NULL,
      0,
      0,
      clear_attachments_2_dw_6_offt_out_reg_const_dest,
      0,
      NULL,
      0,
   };

static const uint8_t clear_attachments_2_dw_6_offt_out_mem_shader_code[8] = {
   0
};
static const uint32_t clear_attachments_2_dw_6_offt_out_mem_const_dest[6] = {
   0,
};
static const struct pvr_shader_factory_info
   clear_attachments_2_dw_6_offt_out_mem_info = {
      0,
      0,
      0,
      0,
      0,
      sizeof(clear_attachments_2_dw_6_offt_out_mem_shader_code),
      clear_attachments_2_dw_6_offt_out_mem_shader_code,
      0,
      0,
      NULL,
      0,
      0,
      clear_attachments_2_dw_6_offt_out_mem_const_dest,
      0,
      NULL,
      0,
   };

static const uint8_t clear_attachments_1_dw_7_offt_out_reg_shader_code[8] = {
   0
};
static const uint32_t clear_attachments_1_dw_7_offt_out_reg_const_dest[6] = {
   0,
};
static const struct pvr_shader_factory_info
   clear_attachments_1_dw_7_offt_out_reg_info = {
      0,
      0,
      0,
      0,
      0,
      sizeof(clear_attachments_1_dw_7_offt_out_reg_shader_code),
      clear_attachments_1_dw_7_offt_out_reg_shader_code,
      0,
      0,
      NULL,
      0,
      0,
      clear_attachments_1_dw_7_offt_out_reg_const_dest,
      0,
      NULL,
      0,
   };

static const uint8_t clear_attachments_1_dw_7_offt_out_mem_shader_code[8] = {
   0
};
static const uint32_t clear_attachments_1_dw_7_offt_out_mem_const_dest[6] = {
   0,
};
static const struct pvr_shader_factory_info
   clear_attachments_1_dw_7_offt_out_mem_info = {
      0,
      0,
      0,
      0,
      0,
      sizeof(clear_attachments_1_dw_7_offt_out_mem_shader_code),
      clear_attachments_1_dw_7_offt_out_mem_shader_code,
      0,
      0,
      NULL,
      0,
      0,
      clear_attachments_1_dw_7_offt_out_mem_const_dest,
      0,
      NULL,
      0,
      1,
   };
static struct {
   const uint8_t *code;
   const uint32_t size;
   const struct pvr_shader_factory_info *info;
} const clear_attachment_collection[64] = {
   { clear_attachments_1_dw_0_offt_out_reg_shader_code,
     sizeof(clear_attachments_1_dw_0_offt_out_reg_shader_code),
     &clear_attachments_1_dw_0_offt_out_reg_info },
   { clear_attachments_1_dw_0_offt_out_mem_shader_code,
     sizeof(clear_attachments_1_dw_0_offt_out_mem_shader_code),
     &clear_attachments_1_dw_0_offt_out_mem_info },
   { clear_attachments_1_dw_1_offt_out_reg_shader_code,
     sizeof(clear_attachments_1_dw_1_offt_out_reg_shader_code),
     &clear_attachments_1_dw_1_offt_out_reg_info },
   { clear_attachments_1_dw_1_offt_out_mem_shader_code,
     sizeof(clear_attachments_1_dw_1_offt_out_mem_shader_code),
     &clear_attachments_1_dw_1_offt_out_mem_info },
   { clear_attachments_1_dw_2_offt_out_reg_shader_code,
     sizeof(clear_attachments_1_dw_2_offt_out_reg_shader_code),
     &clear_attachments_1_dw_2_offt_out_reg_info },
   { clear_attachments_1_dw_2_offt_out_mem_shader_code,
     sizeof(clear_attachments_1_dw_2_offt_out_mem_shader_code),
     &clear_attachments_1_dw_2_offt_out_mem_info },
   { clear_attachments_1_dw_3_offt_out_reg_shader_code,
     sizeof(clear_attachments_1_dw_3_offt_out_reg_shader_code),
     &clear_attachments_1_dw_3_offt_out_reg_info },
   { clear_attachments_1_dw_3_offt_out_mem_shader_code,
     sizeof(clear_attachments_1_dw_3_offt_out_mem_shader_code),
     &clear_attachments_1_dw_3_offt_out_mem_info },
   { clear_attachments_1_dw_4_offt_out_reg_shader_code,
     sizeof(clear_attachments_1_dw_4_offt_out_reg_shader_code),
     &clear_attachments_1_dw_4_offt_out_reg_info },
   { clear_attachments_1_dw_4_offt_out_mem_shader_code,
     sizeof(clear_attachments_1_dw_4_offt_out_mem_shader_code),
     &clear_attachments_1_dw_4_offt_out_mem_info },
   { clear_attachments_1_dw_5_offt_out_reg_shader_code,
     sizeof(clear_attachments_1_dw_5_offt_out_reg_shader_code),
     &clear_attachments_1_dw_5_offt_out_reg_info },
   { clear_attachments_1_dw_5_offt_out_mem_shader_code,
     sizeof(clear_attachments_1_dw_5_offt_out_mem_shader_code),
     &clear_attachments_1_dw_5_offt_out_mem_info },
   { clear_attachments_1_dw_6_offt_out_reg_shader_code,
     sizeof(clear_attachments_1_dw_6_offt_out_reg_shader_code),
     &clear_attachments_1_dw_6_offt_out_reg_info },
   { clear_attachments_1_dw_6_offt_out_mem_shader_code,
     sizeof(clear_attachments_1_dw_6_offt_out_mem_shader_code),
     &clear_attachments_1_dw_6_offt_out_mem_info },
   { clear_attachments_1_dw_7_offt_out_reg_shader_code,
     sizeof(clear_attachments_1_dw_7_offt_out_reg_shader_code),
     &clear_attachments_1_dw_7_offt_out_reg_info },
   { clear_attachments_1_dw_7_offt_out_mem_shader_code,
     sizeof(clear_attachments_1_dw_7_offt_out_mem_shader_code),
     &clear_attachments_1_dw_7_offt_out_mem_info },
   { clear_attachments_2_dw_0_offt_out_reg_shader_code,
     sizeof(clear_attachments_2_dw_0_offt_out_reg_shader_code),
     &clear_attachments_2_dw_0_offt_out_reg_info },
   { clear_attachments_2_dw_0_offt_out_mem_shader_code,
     sizeof(clear_attachments_2_dw_0_offt_out_mem_shader_code),
     &clear_attachments_2_dw_0_offt_out_mem_info },
   { clear_attachments_2_dw_1_offt_out_reg_shader_code,
     sizeof(clear_attachments_2_dw_1_offt_out_reg_shader_code),
     &clear_attachments_2_dw_1_offt_out_reg_info },
   { clear_attachments_2_dw_1_offt_out_mem_shader_code,
     sizeof(clear_attachments_2_dw_1_offt_out_mem_shader_code),
     &clear_attachments_2_dw_1_offt_out_mem_info },
   { clear_attachments_2_dw_2_offt_out_reg_shader_code,
     sizeof(clear_attachments_2_dw_2_offt_out_reg_shader_code),
     &clear_attachments_2_dw_2_offt_out_reg_info },
   { clear_attachments_2_dw_2_offt_out_mem_shader_code,
     sizeof(clear_attachments_2_dw_2_offt_out_mem_shader_code),
     &clear_attachments_2_dw_2_offt_out_mem_info },
   { clear_attachments_2_dw_3_offt_out_reg_shader_code,
     sizeof(clear_attachments_2_dw_3_offt_out_reg_shader_code),
     &clear_attachments_2_dw_3_offt_out_reg_info },
   { clear_attachments_2_dw_3_offt_out_mem_shader_code,
     sizeof(clear_attachments_2_dw_3_offt_out_mem_shader_code),
     &clear_attachments_2_dw_3_offt_out_mem_info },
   { clear_attachments_2_dw_4_offt_out_reg_shader_code,
     sizeof(clear_attachments_2_dw_4_offt_out_reg_shader_code),
     &clear_attachments_2_dw_4_offt_out_reg_info },
   { clear_attachments_2_dw_4_offt_out_mem_shader_code,
     sizeof(clear_attachments_2_dw_4_offt_out_mem_shader_code),
     &clear_attachments_2_dw_4_offt_out_mem_info },
   { clear_attachments_2_dw_5_offt_out_reg_shader_code,
     sizeof(clear_attachments_2_dw_5_offt_out_reg_shader_code),
     &clear_attachments_2_dw_5_offt_out_reg_info },
   { clear_attachments_2_dw_5_offt_out_mem_shader_code,
     sizeof(clear_attachments_2_dw_5_offt_out_mem_shader_code),
     &clear_attachments_2_dw_5_offt_out_mem_info },
   { clear_attachments_2_dw_6_offt_out_reg_shader_code,
     sizeof(clear_attachments_2_dw_6_offt_out_reg_shader_code),
     &clear_attachments_2_dw_6_offt_out_reg_info },
   { clear_attachments_2_dw_6_offt_out_mem_shader_code,
     sizeof(clear_attachments_2_dw_6_offt_out_mem_shader_code),
     &clear_attachments_2_dw_6_offt_out_mem_info },
   { NULL, 0, NULL },
   { NULL, 0, NULL },
   { clear_attachments_3_dw_0_offt_out_reg_shader_code,
     sizeof(clear_attachments_3_dw_0_offt_out_reg_shader_code),
     &clear_attachments_3_dw_0_offt_out_reg_info },
   { clear_attachments_3_dw_0_offt_out_mem_shader_code,
     sizeof(clear_attachments_3_dw_0_offt_out_mem_shader_code),
     &clear_attachments_3_dw_0_offt_out_mem_info },
   { clear_attachments_3_dw_1_offt_out_reg_shader_code,
     sizeof(clear_attachments_3_dw_1_offt_out_reg_shader_code),
     &clear_attachments_3_dw_1_offt_out_reg_info },
   { clear_attachments_3_dw_1_offt_out_mem_shader_code,
     sizeof(clear_attachments_3_dw_1_offt_out_mem_shader_code),
     &clear_attachments_3_dw_1_offt_out_mem_info },
   { clear_attachments_3_dw_2_offt_out_reg_shader_code,
     sizeof(clear_attachments_3_dw_2_offt_out_reg_shader_code),
     &clear_attachments_3_dw_2_offt_out_reg_info },
   { clear_attachments_3_dw_2_offt_out_mem_shader_code,
     sizeof(clear_attachments_3_dw_2_offt_out_mem_shader_code),
     &clear_attachments_3_dw_2_offt_out_mem_info },
   { clear_attachments_3_dw_3_offt_out_reg_shader_code,
     sizeof(clear_attachments_3_dw_3_offt_out_reg_shader_code),
     &clear_attachments_3_dw_3_offt_out_reg_info },
   { clear_attachments_3_dw_3_offt_out_mem_shader_code,
     sizeof(clear_attachments_3_dw_3_offt_out_mem_shader_code),
     &clear_attachments_3_dw_3_offt_out_mem_info },
   { clear_attachments_3_dw_4_offt_out_reg_shader_code,
     sizeof(clear_attachments_3_dw_4_offt_out_reg_shader_code),
     &clear_attachments_3_dw_4_offt_out_reg_info },
   { clear_attachments_3_dw_4_offt_out_mem_shader_code,
     sizeof(clear_attachments_3_dw_4_offt_out_mem_shader_code),
     &clear_attachments_3_dw_4_offt_out_mem_info },
   { clear_attachments_3_dw_5_offt_out_reg_shader_code,
     sizeof(clear_attachments_3_dw_5_offt_out_reg_shader_code),
     &clear_attachments_3_dw_5_offt_out_reg_info },
   { clear_attachments_3_dw_5_offt_out_mem_shader_code,
     sizeof(clear_attachments_3_dw_5_offt_out_mem_shader_code),
     &clear_attachments_3_dw_5_offt_out_mem_info },
   { NULL, 0, NULL },
   { NULL, 0, NULL },
   { NULL, 0, NULL },
   { NULL, 0, NULL },
   { clear_attachments_4_dw_0_offt_out_reg_shader_code,
     sizeof(clear_attachments_4_dw_0_offt_out_reg_shader_code),
     &clear_attachments_4_dw_0_offt_out_reg_info },
   { clear_attachments_4_dw_0_offt_out_mem_shader_code,
     sizeof(clear_attachments_4_dw_0_offt_out_mem_shader_code),
     &clear_attachments_4_dw_0_offt_out_mem_info },
   { clear_attachments_4_dw_1_offt_out_reg_shader_code,
     sizeof(clear_attachments_4_dw_1_offt_out_reg_shader_code),
     &clear_attachments_4_dw_1_offt_out_reg_info },
   { clear_attachments_4_dw_1_offt_out_mem_shader_code,
     sizeof(clear_attachments_4_dw_1_offt_out_mem_shader_code),
     &clear_attachments_4_dw_1_offt_out_mem_info },
   { clear_attachments_4_dw_2_offt_out_reg_shader_code,
     sizeof(clear_attachments_4_dw_2_offt_out_reg_shader_code),
     &clear_attachments_4_dw_2_offt_out_reg_info },
   { clear_attachments_4_dw_2_offt_out_mem_shader_code,
     sizeof(clear_attachments_4_dw_2_offt_out_mem_shader_code),
     &clear_attachments_4_dw_2_offt_out_mem_info },
   { clear_attachments_4_dw_3_offt_out_reg_shader_code,
     sizeof(clear_attachments_4_dw_3_offt_out_reg_shader_code),
     &clear_attachments_4_dw_3_offt_out_reg_info },
   { clear_attachments_4_dw_3_offt_out_mem_shader_code,
     sizeof(clear_attachments_4_dw_3_offt_out_mem_shader_code),
     &clear_attachments_4_dw_3_offt_out_mem_info },
   { clear_attachments_4_dw_4_offt_out_reg_shader_code,
     sizeof(clear_attachments_4_dw_4_offt_out_reg_shader_code),
     &clear_attachments_4_dw_4_offt_out_reg_info },
   { clear_attachments_4_dw_4_offt_out_mem_shader_code,
     sizeof(clear_attachments_4_dw_4_offt_out_mem_shader_code),
     &clear_attachments_4_dw_4_offt_out_mem_info },
   { NULL, 0, NULL },
   { NULL, 0, NULL },
   { NULL, 0, NULL },
   { NULL, 0, NULL },
   { NULL, 0, NULL },
   { NULL, 0, NULL },
};

static const uint8_t spm_load_1X_1_regs_shader_code[8] = { 0 };

static const struct pvr_shader_factory_info spm_load_1X_1_regs_info = {
   0,
   0,
   0,
   0,
   0,
   sizeof(spm_load_1X_1_regs_shader_code),
   spm_load_1X_1_regs_shader_code,
   0,
   0,
   NULL,
   0,
   0,
   NULL,
   0,
   NULL,
   0,
   1,
};

static const uint8_t spm_load_1X_2_regs_shader_code[8] = { 0 };

static const struct pvr_shader_factory_info spm_load_1X_2_regs_info = {
   0,
   0,
   0,
   0,
   0,
   sizeof(spm_load_1X_2_regs_shader_code),
   spm_load_1X_2_regs_shader_code,
   0,
   0,
   NULL,
   0,
   0,
   NULL,
   0,
   NULL,
   0,
   1,
};

static const uint8_t spm_load_1X_4_regs_shader_code[8] = { 0 };

static const struct pvr_shader_factory_info spm_load_1X_4_regs_info = {
   0,
   0,
   0,
   0,
   0,
   sizeof(spm_load_1X_4_regs_shader_code),
   spm_load_1X_4_regs_shader_code,
   0,
   0,
   NULL,
   0,
   0,
   NULL,
   0,
   NULL,
   0,
   1,
};

static const uint8_t spm_load_1X_1_buffers_shader_code[8] = { 0 };

static const uint32_t spm_load_1X_1_buffers_const_dest[14] = { 0 };

static const struct pvr_shader_factory_info spm_load_1X_1_buffers_info = {
   0,
   0,
   0,
   0,
   0,
   sizeof(spm_load_1X_1_buffers_shader_code),
   spm_load_1X_1_buffers_shader_code,
   0,
   0,
   NULL,
   0,
   0,
   spm_load_1X_1_buffers_const_dest,
   0,
   NULL,
   0,
   1,
};

static const uint8_t spm_load_1X_2_buffers_shader_code[8] = { 0 };

static const uint32_t spm_load_1X_2_buffers_const_dest[14] = { 0 };

static const struct pvr_shader_factory_info spm_load_1X_2_buffers_info = {
   0,
   0,
   0,
   0,
   0,
   sizeof(spm_load_1X_2_buffers_shader_code),
   spm_load_1X_2_buffers_shader_code,
   0,
   0,
   NULL,
   0,
   0,
   spm_load_1X_2_buffers_const_dest,
   0,
   NULL,
   0,
   1,
};

static const uint8_t spm_load_1X_3_buffers_shader_code[8] = { 0 };

static const uint32_t spm_load_1X_3_buffers_const_dest[14] = { 0 };

static const struct pvr_shader_factory_info spm_load_1X_3_buffers_info = {
   0,
   0,
   0,
   0,
   0,
   sizeof(spm_load_1X_3_buffers_shader_code),
   spm_load_1X_3_buffers_shader_code,
   0,
   0,
   NULL,
   0,
   0,
   spm_load_1X_3_buffers_const_dest,
   0,
   NULL,
   0,
   1,
};

static const uint8_t spm_load_1X_4_buffers_shader_code[8] = { 0 };

static const uint32_t spm_load_1X_4_buffers_const_dest[14] = { 0 };

static const struct pvr_shader_factory_info spm_load_1X_4_buffers_info = {
   0,
   0,
   0,
   0,
   0,
   sizeof(spm_load_1X_4_buffers_shader_code),
   spm_load_1X_4_buffers_shader_code,
   0,
   0,
   NULL,
   0,
   0,
   spm_load_1X_4_buffers_const_dest,
   0,
   NULL,
   0,
   1,
};

static const uint8_t spm_load_1X_5_buffers_shader_code[8] = { 0 };

static const uint32_t spm_load_1X_5_buffers_const_dest[14] = { 0 };

static const struct pvr_shader_factory_info spm_load_1X_5_buffers_info = {
   0,
   0,
   0,
   0,
   0,
   sizeof(spm_load_1X_5_buffers_shader_code),
   spm_load_1X_5_buffers_shader_code,
   0,
   0,
   NULL,
   0,
   0,
   spm_load_1X_5_buffers_const_dest,
   0,
   NULL,
   0,
   1,
};

static const uint8_t spm_load_1X_6_buffers_shader_code[8] = { 0 };

static const uint32_t spm_load_1X_6_buffers_const_dest[14] = { 0 };

static const struct pvr_shader_factory_info spm_load_1X_6_buffers_info = {
   0,
   0,
   0,
   0,
   0,
   sizeof(spm_load_1X_6_buffers_shader_code),
   spm_load_1X_6_buffers_shader_code,
   0,
   0,
   NULL,
   0,
   0,
   spm_load_1X_6_buffers_const_dest,
   0,
   NULL,
   0,
   1,
};

static const uint8_t spm_load_1X_7_buffers_shader_code[8] = { 0 };

static const uint32_t spm_load_1X_7_buffers_const_dest[14] = { 0 };

static const struct pvr_shader_factory_info spm_load_1X_7_buffers_info = {
   0,
   0,
   0,
   0,
   0,
   sizeof(spm_load_1X_7_buffers_shader_code),
   spm_load_1X_7_buffers_shader_code,
   0,
   0,
   NULL,
   0,
   0,
   spm_load_1X_7_buffers_const_dest,
   0,
   NULL,
   0,
   1,
};

static const uint8_t spm_load_2X_1_regs_shader_code[8] = { 0 };

static const struct pvr_shader_factory_info spm_load_2X_1_regs_info = {
   0,
   0,
   0,
   0,
   0,
   sizeof(spm_load_2X_1_regs_shader_code),
   spm_load_2X_1_regs_shader_code,
   0,
   0,
   NULL,
   0,
   0,
   NULL,
   0,
   NULL,
   0,
   2,
};

static const uint8_t spm_load_2X_2_regs_shader_code[8] = { 0 };

static const struct pvr_shader_factory_info spm_load_2X_2_regs_info = {
   0,
   0,
   0,
   0,
   0,
   sizeof(spm_load_2X_2_regs_shader_code),
   spm_load_2X_2_regs_shader_code,
   0,
   0,
   NULL,
   0,
   0,
   NULL,
   0,
   NULL,
   0,
   2,
};

static const uint8_t spm_load_2X_4_regs_shader_code[8] = { 0 };

static const struct pvr_shader_factory_info spm_load_2X_4_regs_info = {
   0,
   0,
   0,
   0,
   0,
   sizeof(spm_load_2X_4_regs_shader_code),
   spm_load_2X_4_regs_shader_code,
   0,
   0,
   NULL,
   0,
   0,
   NULL,
   0,
   NULL,
   0,
   2,
};

static const uint8_t spm_load_2X_1_buffers_shader_code[8] = { 0 };

static const uint32_t spm_load_2X_1_buffers_const_dest[14] = { 0 };

static const struct pvr_shader_factory_info spm_load_2X_1_buffers_info = {
   0,
   0,
   0,
   0,
   0,
   sizeof(spm_load_2X_1_buffers_shader_code),
   spm_load_2X_1_buffers_shader_code,
   0,
   0,
   NULL,
   0,
   0,
   spm_load_2X_1_buffers_const_dest,
   0,
   NULL,
   0,
   2,
};

static const uint8_t spm_load_2X_2_buffers_shader_code[8] = { 0 };

static const uint32_t spm_load_2X_2_buffers_const_dest[14] = { 0 };

static const struct pvr_shader_factory_info spm_load_2X_2_buffers_info = {
   0,
   0,
   0,
   0,
   0,
   sizeof(spm_load_2X_2_buffers_shader_code),
   spm_load_2X_2_buffers_shader_code,
   0,
   0,
   NULL,
   0,
   0,
   spm_load_2X_2_buffers_const_dest,
   0,
   NULL,
   0,
   2,
};

static const uint8_t spm_load_2X_3_buffers_shader_code[8] = { 0 };

static const uint32_t spm_load_2X_3_buffers_const_dest[14] = { 0 };

static const struct pvr_shader_factory_info spm_load_2X_3_buffers_info = {
   0,
   0,
   0,
   0,
   0,
   sizeof(spm_load_2X_3_buffers_shader_code),
   spm_load_2X_3_buffers_shader_code,
   0,
   0,
   NULL,
   0,
   0,
   spm_load_2X_3_buffers_const_dest,
   0,
   NULL,
   0,
   2,
};

static const uint8_t spm_load_2X_4_buffers_shader_code[8] = { 0 };

static const uint32_t spm_load_2X_4_buffers_const_dest[14] = { 0 };

static const struct pvr_shader_factory_info spm_load_2X_4_buffers_info = {
   0,
   0,
   0,
   0,
   0,
   sizeof(spm_load_2X_4_buffers_shader_code),
   spm_load_2X_4_buffers_shader_code,
   0,
   0,
   NULL,
   0,
   0,
   spm_load_2X_4_buffers_const_dest,
   0,
   NULL,
   0,
   2,
};

static const uint8_t spm_load_2X_5_buffers_shader_code[8] = { 0 };

static const uint32_t spm_load_2X_5_buffers_const_dest[14] = { 0 };

static const struct pvr_shader_factory_info spm_load_2X_5_buffers_info = {
   0,
   0,
   0,
   0,
   0,
   sizeof(spm_load_2X_5_buffers_shader_code),
   spm_load_2X_5_buffers_shader_code,
   0,
   0,
   NULL,
   0,
   0,
   spm_load_2X_5_buffers_const_dest,
   0,
   NULL,
   0,
   2,
};

static const uint8_t spm_load_2X_6_buffers_shader_code[8] = { 0 };

static const uint32_t spm_load_2X_6_buffers_const_dest[14] = { 0 };

static const struct pvr_shader_factory_info spm_load_2X_6_buffers_info = {
   0,
   0,
   0,
   0,
   0,
   sizeof(spm_load_2X_6_buffers_shader_code),
   spm_load_2X_6_buffers_shader_code,
   0,
   0,
   NULL,
   0,
   0,
   spm_load_2X_6_buffers_const_dest,
   0,
   NULL,
   0,
   2,
};

static const uint8_t spm_load_2X_7_buffers_shader_code[8] = { 0 };

static const uint32_t spm_load_2X_7_buffers_const_dest[14] = { 0 };

static const struct pvr_shader_factory_info spm_load_2X_7_buffers_info = {
   0,
   0,
   0,
   0,
   0,
   sizeof(spm_load_2X_7_buffers_shader_code),
   spm_load_2X_7_buffers_shader_code,
   0,
   0,
   NULL,
   0,
   0,
   spm_load_2X_7_buffers_const_dest,
   0,
   NULL,
   0,
   2,
};

static const uint8_t spm_load_4X_1_regs_shader_code[8] = { 0 };

static const struct pvr_shader_factory_info spm_load_4X_1_regs_info = {
   0,
   0,
   0,
   0,
   0,
   sizeof(spm_load_4X_1_regs_shader_code),
   spm_load_4X_1_regs_shader_code,
   0,
   0,
   NULL,
   0,
   0,
   NULL,
   0,
   NULL,
   0,
   4,
};

static const uint8_t spm_load_4X_2_regs_shader_code[8] = { 0 };

static const struct pvr_shader_factory_info spm_load_4X_2_regs_info = {
   0,
   0,
   0,
   0,
   0,
   sizeof(spm_load_4X_2_regs_shader_code),
   spm_load_4X_2_regs_shader_code,
   0,
   0,
   NULL,
   0,
   0,
   NULL,
   0,
   NULL,
   0,
   4,
};

static const uint8_t spm_load_4X_4_regs_shader_code[8] = { 0 };

static const struct pvr_shader_factory_info spm_load_4X_4_regs_info = {
   0, 0, 0,    0, 0,    0, spm_load_4X_4_regs_shader_code, 0, 0, NULL,
   0, 0, NULL, 0, NULL, 0, 4
};

static const uint8_t spm_load_4X_1_buffers_shader_code[8] = { 0 };

static const uint32_t spm_load_4X_1_buffers_const_dest[14] = { 0 };

static const struct pvr_shader_factory_info spm_load_4X_1_buffers_info = {
   0,
   0,
   0,
   0,
   0,
   sizeof(spm_load_4X_1_buffers_shader_code),
   spm_load_4X_1_buffers_shader_code,
   0,
   0,
   NULL,
   0,
   0,
   spm_load_4X_1_buffers_const_dest,
   0,
   NULL,
   0,
   4,
};

static const uint8_t spm_load_4X_2_buffers_shader_code[8] = { 0 };

static const uint32_t spm_load_4X_2_buffers_const_dest[14] = { 0 };

static const struct pvr_shader_factory_info spm_load_4X_2_buffers_info = {
   0,
   0,
   0,
   0,
   0,
   sizeof(spm_load_4X_2_buffers_shader_code),
   spm_load_4X_2_buffers_shader_code,
   0,
   0,
   NULL,
   0,
   0,
   spm_load_4X_2_buffers_const_dest,
   0,
   NULL,
   0,
   4,
};

static const uint8_t spm_load_4X_3_buffers_shader_code[8] = { 0 };

static const uint32_t spm_load_4X_3_buffers_const_dest[14] = { 0 };

static const struct pvr_shader_factory_info spm_load_4X_3_buffers_info = {
   0,
   0,
   0,
   0,
   0,
   sizeof(spm_load_4X_3_buffers_shader_code),
   spm_load_4X_3_buffers_shader_code,
   0,
   0,
   NULL,
   0,
   0,
   spm_load_4X_3_buffers_const_dest,
   0,
   NULL,
   0,
   4,
};

static const uint8_t spm_load_4X_4_buffers_shader_code[8] = { 0 };

static const uint32_t spm_load_4X_4_buffers_const_dest[14] = { 0 };

static const struct pvr_shader_factory_info spm_load_4X_4_buffers_info = {
   0,
   0,
   0,
   0,
   0,
   sizeof(spm_load_4X_4_buffers_shader_code),
   spm_load_4X_4_buffers_shader_code,
   0,
   0,
   NULL,
   0,
   0,
   spm_load_4X_4_buffers_const_dest,
   0,
   NULL,
   0,
   4,
};

static const uint8_t spm_load_4X_5_buffers_shader_code[8] = { 0 };

static const uint32_t spm_load_4X_5_buffers_const_dest[14] = { 0 };

static const struct pvr_shader_factory_info spm_load_4X_5_buffers_info = {
   0,
   0,
   0,
   0,
   0,
   sizeof(spm_load_4X_5_buffers_shader_code),
   spm_load_4X_5_buffers_shader_code,
   0,
   0,
   NULL,
   0,
   0,
   spm_load_4X_5_buffers_const_dest,
   0,
   NULL,
   0,
   4,
};

static const uint8_t spm_load_4X_6_buffers_shader_code[8] = { 0 };

static const uint32_t spm_load_4X_6_buffers_const_dest[14] = { 0 };

static const struct pvr_shader_factory_info spm_load_4X_6_buffers_info = {
   0,
   0,
   0,
   0,
   0,
   sizeof(spm_load_4X_6_buffers_shader_code),
   spm_load_4X_6_buffers_shader_code,
   0,
   0,
   NULL,
   0,
   0,
   spm_load_4X_6_buffers_const_dest,
   0,
   NULL,
   0,
   4,
};

static const uint8_t spm_load_4X_7_buffers_shader_code[8] = { 0 };

static const uint32_t spm_load_4X_7_buffers_const_dest[14] = { 0 };

static const struct pvr_shader_factory_info spm_load_4X_7_buffers_info = {
   0,
   0,
   0,
   0,
   0,
   sizeof(spm_load_4X_7_buffers_shader_code),
   spm_load_4X_7_buffers_shader_code,
   0,
   0,
   NULL,
   0,
   0,
   spm_load_4X_7_buffers_const_dest,
   0,
   NULL,
   0,
   4,
};

static const uint8_t spm_load_8X_1_regs_shader_code[8] = { 0 };

static const struct pvr_shader_factory_info spm_load_8X_1_regs_info = {
   0,
   0,
   0,
   0,
   0,
   sizeof(spm_load_8X_1_regs_shader_code),
   spm_load_8X_1_regs_shader_code,
   0,
   0,
   NULL,
   0,
   0,
   NULL,
   0,
   NULL,
   0,
   8,
};

static const uint8_t spm_load_8X_2_regs_shader_code[8] = { 0 };

static const struct pvr_shader_factory_info spm_load_8X_2_regs_info = {
   0,
   0,
   0,
   0,
   0,
   sizeof(spm_load_8X_2_regs_shader_code),
   spm_load_8X_2_regs_shader_code,
   0,
   0,
   NULL,
   0,
   0,
   NULL,
   0,
   NULL,
   0,
   8,
};

static const uint8_t spm_load_8X_4_regs_shader_code[8] = { 0 };

static const struct pvr_shader_factory_info spm_load_8X_4_regs_info = {
   0,
   0,
   0,
   0,
   0,
   sizeof(spm_load_8X_4_regs_shader_code),
   spm_load_8X_4_regs_shader_code,
   0,
   0,
   NULL,
   0,
   0,
   NULL,
   0,
   NULL,
   0,
   8,
};

static const uint8_t spm_load_8X_1_buffers_shader_code[8] = { 0 };

static const uint32_t spm_load_8X_1_buffers_const_dest[14] = { 0 };

static const struct pvr_shader_factory_info spm_load_8X_1_buffers_info = {
   0,
   0,
   0,
   0,
   0,
   sizeof(spm_load_8X_1_buffers_shader_code),
   spm_load_8X_1_buffers_shader_code,
   0,
   0,
   NULL,
   0,
   0,
   spm_load_8X_1_buffers_const_dest,
   0,
   NULL,
   0,
   8,
};

static const uint8_t spm_load_8X_2_buffers_shader_code[8] = { 0 };

static const uint32_t spm_load_8X_2_buffers_const_dest[14] = { 0 };

static const struct pvr_shader_factory_info spm_load_8X_2_buffers_info = {
   0,
   0,
   0,
   0,
   0,
   sizeof(spm_load_8X_2_buffers_shader_code),
   spm_load_8X_2_buffers_shader_code,
   0,
   0,
   NULL,
   0,
   0,
   spm_load_8X_2_buffers_const_dest,
   0,
   NULL,
   0,
   8,
};

static const uint8_t spm_load_8X_3_buffers_shader_code[8] = { 0 };

static const uint32_t spm_load_8X_3_buffers_const_dest[14] = { 0 };

static const struct pvr_shader_factory_info spm_load_8X_3_buffers_info = {
   0,
   0,
   0,
   0,
   0,
   sizeof(spm_load_8X_3_buffers_shader_code),
   spm_load_8X_3_buffers_shader_code,
   0,
   0,
   NULL,
   0,
   0,
   spm_load_8X_3_buffers_const_dest,
   0,
   NULL,
   0,
   8,
};

static const uint8_t spm_load_8X_4_buffers_shader_code[8] = { 0 };

static const uint32_t spm_load_8X_4_buffers_const_dest[14] = { 0 };

static const struct pvr_shader_factory_info spm_load_8X_4_buffers_info = {
   0,
   0,
   0,
   0,
   0,
   sizeof(spm_load_8X_4_buffers_shader_code),
   spm_load_8X_4_buffers_shader_code,
   0,
   0,
   NULL,
   0,
   0,
   spm_load_8X_4_buffers_const_dest,
   0,
   NULL,
   0,
   8,
};

static const uint8_t spm_load_8X_5_buffers_shader_code[8] = { 0 };

static const uint32_t spm_load_8X_5_buffers_const_dest[14] = { 0 };

static const struct pvr_shader_factory_info spm_load_8X_5_buffers_info = {
   0,
   0,
   0,
   0,
   0,
   sizeof(spm_load_8X_5_buffers_shader_code),
   spm_load_8X_5_buffers_shader_code,
   0,
   0,
   NULL,
   0,
   0,
   spm_load_8X_5_buffers_const_dest,
   0,
   NULL,
   0,
   8,
};

static const uint8_t spm_load_8X_6_buffers_shader_code[8] = { 0 };

static const uint32_t spm_load_8X_6_buffers_const_dest[14] = { 0 };

static const struct pvr_shader_factory_info spm_load_8X_6_buffers_info = {
   0,
   0,
   0,
   0,
   0,
   sizeof(spm_load_8X_6_buffers_shader_code),
   spm_load_8X_6_buffers_shader_code,
   0,
   0,
   NULL,
   0,
   0,
   spm_load_8X_6_buffers_const_dest,
   0,
   NULL,
   0,
   8,
};

static const uint8_t spm_load_8X_7_buffers_shader_code[8] = { 0 };

static const uint32_t spm_load_8X_7_buffers_const_dest[14] = { 0 };

static const struct pvr_shader_factory_info spm_load_8X_7_buffers_info = {
   0,
   0,
   0,
   0,
   0,
   sizeof(spm_load_8X_7_buffers_shader_code),
   spm_load_8X_7_buffers_shader_code,
   0,
   0,
   NULL,
   0,
   0,
   spm_load_8X_7_buffers_const_dest,
   0,
   NULL,
   0,
   8,
};
static struct {
   const uint8_t *code;
   const uint32_t size;
   const struct pvr_shader_factory_info *info;
} const spm_load_collection[40] = {
   { spm_load_1X_1_regs_shader_code,
     sizeof(spm_load_1X_1_regs_shader_code),
     &spm_load_1X_1_regs_info },
   { spm_load_1X_2_regs_shader_code,
     sizeof(spm_load_1X_2_regs_shader_code),
     &spm_load_1X_2_regs_info },
   { spm_load_1X_4_regs_shader_code,
     sizeof(spm_load_1X_4_regs_shader_code),
     &spm_load_1X_4_regs_info },
   { spm_load_1X_1_buffers_shader_code,
     sizeof(spm_load_1X_1_buffers_shader_code),
     &spm_load_1X_1_buffers_info },
   { spm_load_1X_2_buffers_shader_code,
     sizeof(spm_load_1X_2_buffers_shader_code),
     &spm_load_1X_2_buffers_info },
   { spm_load_1X_3_buffers_shader_code,
     sizeof(spm_load_1X_3_buffers_shader_code),
     &spm_load_1X_3_buffers_info },
   { spm_load_1X_4_buffers_shader_code,
     sizeof(spm_load_1X_4_buffers_shader_code),
     &spm_load_1X_4_buffers_info },
   { spm_load_1X_5_buffers_shader_code,
     sizeof(spm_load_1X_5_buffers_shader_code),
     &spm_load_1X_5_buffers_info },
   { spm_load_1X_6_buffers_shader_code,
     sizeof(spm_load_1X_6_buffers_shader_code),
     &spm_load_1X_6_buffers_info },
   { spm_load_1X_7_buffers_shader_code,
     sizeof(spm_load_1X_7_buffers_shader_code),
     &spm_load_1X_7_buffers_info },
   { spm_load_2X_1_regs_shader_code,
     sizeof(spm_load_2X_1_regs_shader_code),
     &spm_load_2X_1_regs_info },
   { spm_load_2X_2_regs_shader_code,
     sizeof(spm_load_2X_2_regs_shader_code),
     &spm_load_2X_2_regs_info },
   { spm_load_2X_4_regs_shader_code,
     sizeof(spm_load_2X_4_regs_shader_code),
     &spm_load_2X_4_regs_info },
   { spm_load_2X_1_buffers_shader_code,
     sizeof(spm_load_2X_1_buffers_shader_code),
     &spm_load_2X_1_buffers_info },
   { spm_load_2X_2_buffers_shader_code,
     sizeof(spm_load_2X_2_buffers_shader_code),
     &spm_load_2X_2_buffers_info },
   { spm_load_2X_3_buffers_shader_code,
     sizeof(spm_load_2X_3_buffers_shader_code),
     &spm_load_2X_3_buffers_info },
   { spm_load_2X_4_buffers_shader_code,
     sizeof(spm_load_2X_4_buffers_shader_code),
     &spm_load_2X_4_buffers_info },
   { spm_load_2X_5_buffers_shader_code,
     sizeof(spm_load_2X_5_buffers_shader_code),
     &spm_load_2X_5_buffers_info },
   { spm_load_2X_6_buffers_shader_code,
     sizeof(spm_load_2X_6_buffers_shader_code),
     &spm_load_2X_6_buffers_info },
   { spm_load_2X_7_buffers_shader_code,
     sizeof(spm_load_2X_7_buffers_shader_code),
     &spm_load_2X_7_buffers_info },
   { spm_load_4X_1_regs_shader_code,
     sizeof(spm_load_4X_1_regs_shader_code),
     &spm_load_4X_1_regs_info },
   { spm_load_4X_2_regs_shader_code,
     sizeof(spm_load_4X_2_regs_shader_code),
     &spm_load_4X_2_regs_info },
   { spm_load_4X_4_regs_shader_code,
     sizeof(spm_load_4X_4_regs_shader_code),
     &spm_load_4X_4_regs_info },
   { spm_load_4X_1_buffers_shader_code,
     sizeof(spm_load_4X_1_buffers_shader_code),
     &spm_load_4X_1_buffers_info },
   { spm_load_4X_2_buffers_shader_code,
     sizeof(spm_load_4X_2_buffers_shader_code),
     &spm_load_4X_2_buffers_info },
   { spm_load_4X_3_buffers_shader_code,
     sizeof(spm_load_4X_3_buffers_shader_code),
     &spm_load_4X_3_buffers_info },
   { spm_load_4X_4_buffers_shader_code,
     sizeof(spm_load_4X_4_buffers_shader_code),
     &spm_load_4X_4_buffers_info },
   { spm_load_4X_5_buffers_shader_code,
     sizeof(spm_load_4X_5_buffers_shader_code),
     &spm_load_4X_5_buffers_info },
   { spm_load_4X_6_buffers_shader_code,
     sizeof(spm_load_4X_6_buffers_shader_code),
     &spm_load_4X_6_buffers_info },
   { spm_load_4X_7_buffers_shader_code,
     sizeof(spm_load_4X_7_buffers_shader_code),
     &spm_load_4X_7_buffers_info },
   { spm_load_8X_1_regs_shader_code,
     sizeof(spm_load_8X_1_regs_shader_code),
     &spm_load_8X_1_regs_info },
   { spm_load_8X_2_regs_shader_code,
     sizeof(spm_load_8X_2_regs_shader_code),
     &spm_load_8X_2_regs_info },
   { spm_load_8X_4_regs_shader_code,
     sizeof(spm_load_8X_4_regs_shader_code),
     &spm_load_8X_4_regs_info },
   { spm_load_8X_1_buffers_shader_code,
     sizeof(spm_load_8X_1_buffers_shader_code),
     &spm_load_8X_1_buffers_info },
   { spm_load_8X_2_buffers_shader_code,
     sizeof(spm_load_8X_2_buffers_shader_code),
     &spm_load_8X_2_buffers_info },
   { spm_load_8X_3_buffers_shader_code,
     sizeof(spm_load_8X_3_buffers_shader_code),
     &spm_load_8X_3_buffers_info },
   { spm_load_8X_4_buffers_shader_code,
     sizeof(spm_load_8X_4_buffers_shader_code),
     &spm_load_8X_4_buffers_info },
   { spm_load_8X_5_buffers_shader_code,
     sizeof(spm_load_8X_5_buffers_shader_code),
     &spm_load_8X_5_buffers_info },
   { spm_load_8X_6_buffers_shader_code,
     sizeof(spm_load_8X_6_buffers_shader_code),
     &spm_load_8X_6_buffers_info },
   { spm_load_8X_7_buffers_shader_code,
     sizeof(spm_load_8X_7_buffers_shader_code),
     &spm_load_8X_7_buffers_info },
};

#endif /* PVR_STATIC_SHADERS_H */
