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

#ifndef PVR_PDS_H
#define PVR_PDS_H

#include <stdbool.h>

#include "pvr_device_info.h"
#include "pvr_limits.h"
#include "pds/pvr_rogue_pds_defs.h"
#include "util/macros.h"

#ifdef __cplusplus
#   define restrict __restrict__
#endif

/*****************************************************************************
 Macro definitions
*****************************************************************************/

/* Based on Maximum number of passes that may emit DOUTW x Maximum number that
 * might be emitted.
 */
#define PVR_PDS_MAX_TOTAL_NUM_DWORD_DOUTW 6
/* Based on Maximum number of passes that may emit DOUTW x Maximum number that
 * might be emitted.
 */
#define PVR_PDS_MAX_TOTAL_NUM_QWORD_DOUTW 3
/* Based on max(max(UBOs,cbuffers), numTextures). */
#define PVR_PDS_MAX_NUM_DMA_KICKS 32
#define PVR_PDS_NUM_VERTEX_STREAMS 32
#define PVR_PDS_NUM_VERTEX_ELEMENTS 32
#define PVR_MAXIMUM_ITERATIONS 128

#define PVR_PDS_NUM_COMPUTE_INPUT_REGS 3

#define PVR_NEED_SW_COMPUTE_PDS_BARRIER(dev_info)       \
   PVR_HAS_FEATURE(dev_info, compute_morton_capable) && \
      !PVR_HAS_ERN(dev_info, 45493)

/* FIXME: Change BIL to SPV. */
/* Any variable location can have at most 4 32-bit components. */
#define BIL_COMPONENTS_PER_LOCATION 4

/* Maximum number of DDMAD's that may be performed (Num attribs * Num DMA's per
 * attribute).
 */
#define PVR_MAX_VERTEX_ATTRIB_DMAS \
   (PVR_MAX_VERTEX_INPUT_BINDINGS * BIL_COMPONENTS_PER_LOCATION)

/*****************************************************************************
 Typedefs
*****************************************************************************/

/* FIXME: We might need to change some bools to this. */
typedef uint32_t PVR_PDS_BOOL;

/*****************************************************************************
 Enums
*****************************************************************************/

enum pvr_pds_generate_mode {
   PDS_GENERATE_SIZES,
   PDS_GENERATE_CODE_SEGMENT,
   PDS_GENERATE_DATA_SEGMENT,
   PDS_GENERATE_CODEDATA_SEGMENTS
};

enum pvr_pds_store_type { PDS_COMMON_STORE, PDS_UNIFIED_STORE };

enum pvr_pds_vertex_attrib_program_type {
   PVR_PDS_VERTEX_ATTRIB_PROGRAM_BASIC,
   PVR_PDS_VERTEX_ATTRIB_PROGRAM_BASE_INSTANCE,
   PVR_PDS_VERTEX_ATTRIB_PROGRAM_DRAW_INDIRECT,
   PVR_PDS_VERTEX_ATTRIB_PROGRAM_COUNT
};

enum pvr_pds_addr_literal_type {
   PVR_PDS_ADDR_LITERAL_DESC_SET_ADDRS_TABLE,
   PVR_PDS_ADDR_LITERAL_PUSH_CONSTS,
   PVR_PDS_ADDR_LITERAL_BLEND_CONSTANTS,
};

/*****************************************************************************
 Structure definitions
*****************************************************************************/

struct pvr_psc_register {
   uint32_t num;

   unsigned int size; /* size of each element. */
   unsigned int dim : 4; /* max number of elements. */
   unsigned int index; /* offset into array. */

   unsigned int cast;

   unsigned int type;
   uint64_t name;
   bool auto_assign;
   unsigned int original_type;
};

struct pvr_psc_program_output {
   const uint32_t *code;

   struct pvr_psc_register *data;
   unsigned int data_count;

   unsigned int data_size_aligned;
   unsigned int code_size_aligned;
   unsigned int temp_size_aligned;

   unsigned int data_size;
   unsigned int code_size;
   unsigned int temp_size;

   void (*write_data)(void *data, uint32_t *buffer);
};

struct pvr_pds_usc_task_control {
   uint64_t src0;
};

/* Up to 4 64-bit state words currently supported. */
#define PVR_PDS_MAX_NUM_DOUTW_CONSTANTS 4

/* Structure for DOUTW. */
struct pvr_pds_doutw_control {
   enum pvr_pds_store_type dest_store;
   uint32_t num_const64;
   uint64_t doutw_data[PVR_PDS_MAX_NUM_DOUTW_CONSTANTS];
   bool last_instruction;

   uint32_t *data_segment;
   uint32_t data_size;
   uint32_t code_size;
};

/* Structure representing the PDS pixel event program.
 *
 * data_segment - pointer to the data segment
 * task_control - USC task control words
 * emit_words - array of Emit words
 * data_size - size of data segment
 * code_size - size of code segment
 */
struct pvr_pds_event_program {
   uint32_t *data_segment;
   struct pvr_pds_usc_task_control task_control;

   uint32_t num_emit_word_pairs;
   uint32_t *emit_words;

   uint32_t data_size;
   uint32_t code_size;
};

/*
 * Structure representing the PDS pixel shader secondary attribute program.
 *
 * data_segment - pointer to the data segment
 *
 * num_uniform_dma_kicks - number of Uniform DMA kicks
 * uniform_dma_control - array of Uniform DMA control words
 * uniform_dma_address - array of Uniform DMA address words
 *
 * num_texture_dma_kicks - number of Texture State DMA kicks
 * texture_dma_control - array of Texture State DMA control words
 * texture_dma_address - array of Texture State DMA address words
 *
 * data_size - size of data segment
 * code_size - size of code segment
 *
 * temps_used - PDS Temps
 */
struct pvr_pds_pixel_shader_sa_program {
   uint32_t *data_segment;

   uint32_t num_dword_doutw;
   uint32_t dword_doutw_value[PVR_PDS_MAX_TOTAL_NUM_DWORD_DOUTW];
   uint32_t dword_doutw_control[PVR_PDS_MAX_TOTAL_NUM_DWORD_DOUTW];

   uint32_t num_q_word_doutw;
   uint32_t q_word_doutw_value[2 * PVR_PDS_MAX_TOTAL_NUM_QWORD_DOUTW];
   uint32_t q_word_doutw_control[PVR_PDS_MAX_TOTAL_NUM_QWORD_DOUTW];

   uint32_t num_uniform_dma_kicks;
   uint64_t uniform_dma_address[PVR_PDS_MAX_NUM_DMA_KICKS];
   uint32_t uniform_dma_control[PVR_PDS_MAX_NUM_DMA_KICKS];

   uint32_t num_texture_dma_kicks;
   uint64_t texture_dma_address[PVR_PDS_MAX_NUM_DMA_KICKS];
   uint32_t texture_dma_control[PVR_PDS_MAX_NUM_DMA_KICKS];

   bool kick_usc;
   bool write_tile_position;
   uint32_t tile_position_attr_dest;
   struct pvr_pds_usc_task_control usc_task_control;

   bool clear;
   uint32_t *clear_color;
   uint32_t clear_color_dest_reg;
   bool packed_clear;

   uint32_t data_size;
   uint32_t code_size;

   uint32_t temps_used;
};

/* Structure representing the PDS pixel shader program.
 *
 * data_segment - pointer to the data segment
 * usc_task_control - array of USC task control words
 *
 * data_size - size of data segment
 * code_size - size of code segment
 */
struct pvr_pds_kickusc_program {
   uint32_t *data_segment;
   struct pvr_pds_usc_task_control usc_task_control;

   uint32_t data_size;
   uint32_t code_size;
};

/* Structure representing the PDS fence/doutc program.
 *
 * data_segment - pointer to the data segment
 * data_size - size of data segment
 * code_size - size of code segment
 */
struct pvr_pds_fence_program {
   uint32_t *data_segment;
   uint32_t fence_constant_word;
   uint32_t data_size;
   uint32_t code_size;
};

/* Structure representing the PDS coefficient loading.
 *
 * data_segment - pointer to the data segment
 * num_fpu_iterators - number of FPU iterators
 * FPU_iterators - array of FPU iterator control words
 * destination - array of Common Store destinations
 *
 * data_size - size of data segment
 * code_size - size of code segment
 */
struct pvr_pds_coeff_loading_program {
   uint32_t *data_segment;
   uint32_t num_fpu_iterators;
   uint32_t FPU_iterators[PVR_MAXIMUM_ITERATIONS];
   uint32_t destination[PVR_MAXIMUM_ITERATIONS];

   uint32_t data_size;
   uint32_t code_size;

   uint32_t temps_used;
};

/* Structure representing the PDS vertex shader secondary attribute program.
 *
 * data_segment - pointer to the data segment
 * num_dma_kicks - number of DMA kicks
 * dma_control - array of DMA control words
 * dma_address - array of DMA address words
 *
 * data_size - size of data segment
 * code_size - size of code segment
 */
struct pvr_pds_vertex_shader_sa_program {
   uint32_t *data_segment;

   /* num_uniform_dma_kicks, uniform_dma_address, uniform_dma_control, are not
    * used for generating PDS data section and code section, they are currently
    * only used to simpler the driver implementation. The driver should correct
    * these information into num_dma_kicks, dma_address and dma_control to get
    * the PDS properly generated.
    */

   uint32_t num_dword_doutw;
   uint32_t dword_doutw_value[PVR_PDS_MAX_TOTAL_NUM_DWORD_DOUTW];
   uint32_t dword_doutw_control[PVR_PDS_MAX_TOTAL_NUM_DWORD_DOUTW];

   uint32_t num_q_word_doutw;
   uint32_t q_word_doutw_value[2 * PVR_PDS_MAX_TOTAL_NUM_QWORD_DOUTW];
   uint32_t q_word_doutw_control[PVR_PDS_MAX_TOTAL_NUM_QWORD_DOUTW];

   uint32_t num_uniform_dma_kicks;
   uint64_t uniform_dma_address[PVR_PDS_MAX_NUM_DMA_KICKS];
   uint32_t uniform_dma_control[PVR_PDS_MAX_NUM_DMA_KICKS];

   uint32_t num_texture_dma_kicks;
   uint64_t texture_dma_address[PVR_PDS_MAX_NUM_DMA_KICKS];
   uint32_t texture_dma_control[PVR_PDS_MAX_NUM_DMA_KICKS];

   uint32_t num_dma_kicks;
   uint64_t dma_address[PVR_PDS_MAX_NUM_DMA_KICKS];
   uint32_t dma_control[PVR_PDS_MAX_NUM_DMA_KICKS];

   bool kick_usc;
   struct pvr_pds_usc_task_control usc_task_control;

   /* Shared register buffer base address (VDM/CDM context load case only). */
   bool clear_pds_barrier;

   uint32_t data_size;
   uint32_t code_size;
};

/* Structure representing a PDS vertex stream element.
 *
 * There are two types of element, repeat DMA and non-repeat DMA.
 *
 * Non repeat DMA are the classic DMA of some number of bytes from an offset
 * into contiguous registers. It is assumed the address and size are dword
 * aligned. To use this, specify 0 for the component size. Each four bytes read
 * will go to the next HW register.
 *
 * Repeat DMA enables copying of sub dword amounts at non dword aligned
 * addresses. To use this, specify the component size as either 1,2,3 or 4
 * bytes. Size specifies the number of components, and each component read
 * will go to the next HW register.
 *
 * In both cases, HW registers are written contiguously.
 *
 * offset - offset of the vertex stream element
 * size - size of the vertex stream element in bytes for non repeat DMA, or
 *        number of components for repeat DMA.
 * reg - first vertex stream element register to DMA to.
 * component_size - Size of component for repeat DMA, or 0 for non repeat dma.
 */
struct pvr_pds_vertex_element {
   uint32_t offset;
   uint32_t size;
   uint16_t reg;
   uint16_t component_size;
};

/* Structure representing a PDS vertex stream.
 *
 * instance_data - flag whether the vertex stream is indexed or instance data
 * read_back - If True, vertex is reading back data output by GPU earlier in
 *             same kick. This will enable MCU coherency if relevant.
 * multiplier - vertex stream frequency multiplier
 * shift - vertex stream frequency shift
 * address - vertex stream address in bytes
 * buffer_size_in_bytes - buffer size in bytes if vertex attribute is sourced
 *                        from buffer object
 * stride - vertex stream stride in bytes
 * num_vertices - number of vertices in buffer. Used for OOB checking.
                - 0 = disable oob checking.
 * num_elements - number of vertex stream elements
 * elements - array of vertex stream elements
 * use_ddmadt - When the has_pds_ddmadt feature is enabled. Boolean allowing
 *              DDMADT to be use per stream element.
 */
struct pvr_pds_vertex_stream {
   bool current_state;
   bool instance_data;
   bool read_back;
   uint32_t multiplier;
   uint32_t shift;
   uint64_t address;
   uint32_t buffer_size_in_bytes;
   uint32_t stride;
   uint32_t num_vertices;
   uint32_t num_elements;
   struct pvr_pds_vertex_element elements[PVR_PDS_NUM_VERTEX_ELEMENTS];

   bool use_ddmadt;
};

/* Structure representing the PDS vertex shader program.
 *
 * This structure describes the USC code and vertex buffers required
 * by the PDS vertex loading program.
 *
 * data_segment - Pointer to the data segment.
 * usc_task_control - Description of USC task for vertex shader program.
 * num_streams - Number of vertex streams.
 * iterate_vtx_id - If set, the vertex id should be iterated.
 * vtx_id_register - The register to iterate the VertexID into (if applicable)
 * vtx_id_modifier - Value to pvr_add/SUB from index value received by PDS.
 *                   This is used because the index value received by PDS has
 *                   INDEX_OFFSET added, and generally VertexID wouldn't.
 * vtx_id_sub_modifier - If true, vtx_id_modifier is subtracted, else added.
 * iterate_instance_id - If set, the instance id should be iterated.
 * instance_id_register - The register to iterate the InstanceID into (if
 *                        applicable). The vertex and instance id will both be
 *                        iterated as unsigned ints
 *
 * iterate_remap_id - Should be set to true if vertex shader needs
 *                    VS_REMAPPED_INDEX_ID (e.g. Another TA shader runs after
 *                    it).
 * null_idx - Indicates no index buffer is bound, so every index should be
 *            null_idx_value.
 * null_idx_value - The value to use as index if null_idx set.
 * data_size - Size of data segment, in dwords. Output by call to
 *             pvr_pds_vertex_shader, and used as input when generating data.
 * code_size - Size of code segment. Output by call to pvr_pds_vertex_shader.
 *             This is the number of dword instructions that are/were generated.
 * temps_used - Number of temporaries used. Output by call to
 *              pvr_pds_vertex_shader.
 */
struct pvr_pds_vertex_shader_program {
   uint32_t *data_segment;
   struct pvr_pds_usc_task_control usc_task_control;
   uint32_t num_streams;

   bool iterate_vtx_id;
   uint32_t vtx_id_register;
   uint32_t vtx_id_modifier;
   bool vtx_id_sub_modifier;

   bool iterate_instance_id;
   uint32_t instance_id_register;
   uint32_t instance_id_modifier;
   uint32_t base_instance;

   bool iterate_remap_id;

   bool null_idx;
   uint32_t null_idx_value;

   uint32_t *stream_patch_offsets;
   uint32_t num_stream_patches;

   uint32_t data_size;
   uint32_t code_size;
   uint32_t temps_used;
   uint32_t ddmadt_enables;
   uint32_t skip_stream_flag;

   bool draw_indirect;
   bool indexed;

   struct pvr_pds_vertex_stream streams[PVR_PDS_NUM_VERTEX_STREAMS];
};

/* Structure representing PDS shared reg storing program. */
struct pvr_pds_shared_storing_program {
   struct pvr_pds_doutw_control doutw_control; /*!< DOUTW state */
   struct pvr_pds_kickusc_program usc_task; /*!< DOUTU state */
   bool cc_enable; /*!< cc bit is set on the doutu instruction. */
   uint32_t data_size; /*!< total data size, non-aligned. */
   uint32_t code_size; /*!< total code size, non-aligned. */
};

#define PVR_MAX_STREAMOUT_BUFFERS 4

/* Structure representing stream out init PDS programs. */
struct pvr_pds_stream_out_init_program {
   /* --- Input to PDS_STREAM_OUT_INT_PROGRAM --- */

   /* Number of buffers to load/store.
    * This indicates the number of entries in the next two arrays.
    * Data is loaded/stored contiguously to persistent temps.
    */
   uint32_t num_buffers;

   /* Number of persistent temps in dword to load/store for each buffer. */
   uint32_t pds_buffer_data_size[PVR_MAX_STREAMOUT_BUFFERS];
   /* The device address for loading/storing persistent temps for each buffer.
    * If address is zero, then no data is loaded/stored
    * into pt registers for the buffer.
    */
   uint64_t dev_address_for_buffer_data[PVR_MAX_STREAMOUT_BUFFERS];

   /* PDS state update Stream Out Init Programs. */
   uint32_t stream_out_init_pds_data_size;
   uint32_t stream_out_init_pds_code_size;
};

/* Structure representing stream out terminate PDS program. */
struct pvr_pds_stream_out_terminate_program {
   /* Input to PDS_STREAM_OUT_TERMINATE_PROGRAM.
    *
    * Number of persistent temps in dword used in stream out PDS programs needs
    * to be stored.
    * The terminate program writes pds_persistent_temp_size_to_store number
    * persistent temps to dev_address_for_storing_persistent_temp.
    */
   uint32_t pds_persistent_temp_size_to_store;

   /* The device address for storing persistent temps. */
   uint64_t dev_address_for_storing_persistent_temp;

   /* PPP state update Stream Out Program for stream out terminate. */
   uint32_t stream_out_terminate_pds_data_size;
   uint32_t stream_out_terminate_pds_code_size;
};

/*  Structure representing the PDS compute shader program.
 *	This structure describes the USC code and compute buffers required
 *	by the PDS compute task loading program
 *
 *	data_segment
 *		pointer to the data segment
 *	usc_task_control
 *		Description of USC task for compute shader program.
 *	data_size
 *		Size of data segment, in dwords.
 *		Output by call to pvr_pds_compute_shader, and used as input when
 *   generating data. code_size Size of code segment. Output by call to
 *   pvr_pds_compute_shader. This is the number of dword instructions that
 *   are/were generated. temps_used Number of temporaries used. Output by call
 *to pvr_pds_compute_shader. highest_temp The highest temp number used. Output
 *by call to pvr_pds_compute_shader coeff_update_task_branch_size The number of
 *   instructions we need to branch over to skip the coefficient update task.
 */

struct pvr_pds_compute_shader_program {
   uint32_t *data_segment;
   struct pvr_pds_usc_task_control usc_task_control;
   struct pvr_pds_usc_task_control usc_task_control_coeff_update;

   uint32_t data_size;
   uint32_t code_size;

   uint32_t temps_used;
   uint32_t highest_temp;

   uint32_t local_input_regs[3];
   uint32_t work_group_input_regs[3];
   uint32_t global_input_regs[3];

   uint32_t barrier_coefficient;

   bool fence;

   bool flattened_work_groups;

   bool clear_pds_barrier;

   bool has_coefficient_update_task;

   uint32_t coeff_update_task_branch_size;

   bool add_base_workgroup;
   uint32_t base_workgroup_constant_offset_in_dwords[3];

   bool kick_usc;

   bool conditional_render;
   uint32_t cond_render_const_offset_in_dwords;
   uint32_t cond_render_pred_temp;
};
struct pvr_pds_ldst_control {
   uint64_t cache_control_const;
};

/* Define a value we can use as a register number in the driver to denote that
 * the value is unused.
 */
#define PVR_PDS_COMPUTE_INPUT_REG_UNUSED 0xFFFFFFFFU

static inline void pvr_pds_compute_shader_program_init(
   struct pvr_pds_compute_shader_program *program)
{
   *program = (struct pvr_pds_compute_shader_program){
      .local_input_regs = {
         PVR_PDS_COMPUTE_INPUT_REG_UNUSED,
         PVR_PDS_COMPUTE_INPUT_REG_UNUSED,
         PVR_PDS_COMPUTE_INPUT_REG_UNUSED,
      },
      .work_group_input_regs = {
         PVR_PDS_COMPUTE_INPUT_REG_UNUSED,
         PVR_PDS_COMPUTE_INPUT_REG_UNUSED,
         PVR_PDS_COMPUTE_INPUT_REG_UNUSED,
      },
      .global_input_regs = {
         PVR_PDS_COMPUTE_INPUT_REG_UNUSED,
         PVR_PDS_COMPUTE_INPUT_REG_UNUSED,
         PVR_PDS_COMPUTE_INPUT_REG_UNUSED,
      },
      .barrier_coefficient = PVR_PDS_COMPUTE_INPUT_REG_UNUSED,
   };
}

/*****************************************************************************
 function declarations
*****************************************************************************/

/*****************************************************************************
 Constructors
*****************************************************************************/

void pvr_pds_pixel_shader_sa_initialize(
   struct pvr_pds_pixel_shader_sa_program *program);
void pvr_pds_compute_shader_initialize(
   struct pvr_pds_compute_shader_program *program);

/* Utility */

uint32_t pvr_pds_append_constant64(uint32_t *constants,
                                   uint64_t constant_value,
                                   uint32_t *data_size);

uint32_t pvr_pds_encode_dma_burst(uint32_t *dma_control,
                                  uint64_t *dma_address,
                                  uint32_t dest_offset,
                                  uint32_t dma_size,
                                  uint64_t src_address,
                                  bool last,
                                  const struct pvr_device_info *dev_info);

void pvr_pds_setup_doutu(struct pvr_pds_usc_task_control *usc_task_control,
                         uint64_t execution_address,
                         uint32_t usc_temps,
                         uint32_t sample_rate,
                         bool phase_rate_change);

/* Pixel */
#define pvr_pds_set_sizes_pixel_shader(X) \
   pvr_pds_kick_usc(X, NULL, 0, false, PDS_GENERATE_SIZES)
#define pvr_pds_generate_pixel_shader_program(X, Y) \
   pvr_pds_kick_usc(X, Y, 0, false, PDS_GENERATE_CODEDATA_SEGMENTS)

#define pvr_pds_generate_VDM_sync_program(X, Y) \
   pvr_pds_kick_usc(X, Y, 0, false, PDS_GENERATE_CODEDATA_SEGMENTS)

uint32_t *pvr_pds_generate_doutc(struct pvr_pds_fence_program *restrict program,
                                 uint32_t *restrict buffer,
                                 enum pvr_pds_generate_mode gen_mode);

uint32_t *
pvr_pds_generate_doutw(struct pvr_pds_doutw_control *restrict psControl,
                       uint32_t *restrict buffer,
                       enum pvr_pds_generate_mode gen_mode,
                       const struct pvr_device_info *dev_info);

uint32_t *pvr_pds_kick_usc(struct pvr_pds_kickusc_program *restrict program,
                           uint32_t *restrict buffer,
                           uint32_t start_next_constant,
                           bool cc_enabled,
                           enum pvr_pds_generate_mode gen_mode);

/* Pixel Secondary */
#define pvr_pds_set_sizes_pixel_shader_sa_uniform_data(X, Y)     \
   pvr_pds_pixel_shader_uniform_texture_data(X,                  \
                                             NULL,               \
                                             PDS_GENERATE_SIZES, \
                                             true,               \
                                             Y)
#define pvr_pds_set_sizes_pixel_shader_sa_texture_data(X, Y)     \
   pvr_pds_pixel_shader_uniform_texture_data(X,                  \
                                             NULL,               \
                                             PDS_GENERATE_SIZES, \
                                             false,              \
                                             Y)
#define pvr_pds_set_sizes_pixel_shader_uniform_texture_code(X) \
   pvr_pds_pixel_shader_uniform_texture_code(X, NULL, PDS_GENERATE_SIZES)

#define pvr_pds_generate_pixel_shader_sa_texture_state_data(X, Y, Z)    \
   pvr_pds_pixel_shader_uniform_texture_data(X,                         \
                                             Y,                         \
                                             PDS_GENERATE_DATA_SEGMENT, \
                                             false,                     \
                                             Z)

#define pvr_pds_generate_pixel_shader_sa_code_segment(X, Y) \
   pvr_pds_pixel_shader_uniform_texture_code(X, Y, PDS_GENERATE_CODE_SEGMENT)

uint32_t *pvr_pds_pixel_shader_uniform_texture_data(
   struct pvr_pds_pixel_shader_sa_program *restrict program,
   uint32_t *restrict buffer,
   enum pvr_pds_generate_mode gen_mode,
   bool uniform,
   const struct pvr_device_info *dev_info);

uint32_t *pvr_pds_pixel_shader_uniform_texture_code(
   struct pvr_pds_pixel_shader_sa_program *restrict program,
   uint32_t *restrict buffer,
   enum pvr_pds_generate_mode gen_mode);

/* Vertex */
#define pvr_pds_set_sizes_vertex_shader(X, Y) \
   pvr_pds_vertex_shader(X, NULL, PDS_GENERATE_SIZES, Y)

#define pvr_pds_generate_vertex_shader_data_segment(X, Y, Z) \
   pvr_pds_vertex_shader(X, Y, PDS_GENERATE_DATA_SEGMENT, Z)

#define pvr_pds_generate_vertex_shader_code_segment(X, Y, Z) \
   pvr_pds_vertex_shader(X, Y, PDS_GENERATE_CODE_SEGMENT, Z)

uint32_t *
pvr_pds_vertex_shader(struct pvr_pds_vertex_shader_program *restrict program,
                      uint32_t *restrict buffer,
                      enum pvr_pds_generate_mode gen_mode,
                      const struct pvr_device_info *dev_info);

/* Compute */
uint32_t *
pvr_pds_compute_shader(struct pvr_pds_compute_shader_program *restrict program,
                       uint32_t *restrict buffer,
                       enum pvr_pds_generate_mode gen_mode,
                       const struct pvr_device_info *dev_info);

/* Vertex Secondary */
#define pvr_pds_set_sizes_vertex_shader_sa(X, Y) \
   pvr_pds_vertex_shader_sa(X, NULL, PDS_GENERATE_SIZES, Y)

#define pvr_pds_generate_vertex_shader_sa_data_segment(X, Y, Z) \
   pvr_pds_vertex_shader_sa(X, Y, PDS_GENERATE_DATA_SEGMENT, Z)

#define pvr_pds_generate_vertex_shader_sa_code_segment(X, Y, Z) \
   pvr_pds_vertex_shader_sa(X, Y, PDS_GENERATE_CODE_SEGMENT, Z)

uint32_t *pvr_pds_vertex_shader_sa(
   struct pvr_pds_vertex_shader_sa_program *restrict program,
   uint32_t *restrict buffer,
   enum pvr_pds_generate_mode gen_mode,
   const struct pvr_device_info *dev_info);

/* Pixel Event */
#define pvr_pds_set_sizes_pixel_event(X, Y) \
   pvr_pds_generate_pixel_event(X, NULL, PDS_GENERATE_SIZES, Y)

#define pvr_pds_generate_pixel_event_data_segment(X, Y, Z) \
   pvr_pds_generate_pixel_event(X, Y, PDS_GENERATE_DATA_SEGMENT, Z)

#define pvr_pds_generate_pixel_event_code_segment(X, Y, Z) \
   pvr_pds_generate_pixel_event(X, Y, PDS_GENERATE_CODE_SEGMENT, Z)

uint32_t *
pvr_pds_generate_pixel_event(struct pvr_pds_event_program *restrict program,
                             uint32_t *restrict buffer,
                             enum pvr_pds_generate_mode gen_mode,
                             const struct pvr_device_info *dev_info);

/* Coefficient Loading */
#define pvr_pds_set_sizes_coeff_loading(X) \
   pvr_pds_coefficient_loading(X, NULL, PDS_GENERATE_SIZES)

#define pvr_pds_generate_coeff_loading_program(X, Y) \
   pvr_pds_coefficient_loading(X, Y, PDS_GENERATE_CODE_SEGMENT)

uint32_t *pvr_pds_coefficient_loading(
   struct pvr_pds_coeff_loading_program *restrict program,
   uint32_t *restrict buffer,
   enum pvr_pds_generate_mode gen_mode);

/* Compute DM barrier-specific conditional code */
uint32_t *pvr_pds_generate_compute_barrier_conditional(
   uint32_t *buffer,
   enum pvr_pds_generate_mode gen_mode);

/* Shared register storing */
uint32_t *pvr_pds_generate_shared_storing_program(
   struct pvr_pds_shared_storing_program *restrict program,
   uint32_t *restrict buffer,
   enum pvr_pds_generate_mode gen_mode,
   const struct pvr_device_info *dev_info);

/*Shared register loading */
uint32_t *pvr_pds_generate_fence_terminate_program(
   struct pvr_pds_fence_program *restrict program,
   uint32_t *restrict buffer,
   enum pvr_pds_generate_mode gen_mode,
   const struct pvr_device_info *dev_info);

/* CDM Shared register loading */
uint32_t *pvr_pds_generate_compute_shared_loading_program(
   struct pvr_pds_shared_storing_program *restrict program,
   uint32_t *restrict buffer,
   enum pvr_pds_generate_mode gen_mode,
   const struct pvr_device_info *dev_info);

/* Stream out */
uint32_t *pvr_pds_generate_stream_out_init_program(
   struct pvr_pds_stream_out_init_program *restrict program,
   uint32_t *restrict buffer,
   bool store_mode,
   enum pvr_pds_generate_mode gen_mode,
   const struct pvr_device_info *dev_info);

uint32_t *pvr_pds_generate_stream_out_terminate_program(
   struct pvr_pds_stream_out_terminate_program *restrict program,
   uint32_t *restrict buffer,
   enum pvr_pds_generate_mode gen_mode,
   const struct pvr_device_info *dev_info);

/* Structure representing DrawIndirect PDS programs. */
struct pvr_pds_drawindirect_program {
   /* --- Input to pvr_pds_drawindirect_program --- */

   /* Address of the index list block in the VDM control stream.
    * This must point to a 128-bit aligned index list header.
    */
   uint64_t index_list_addr_buffer;
   /* Address of arguments for Draw call. Layout is defined by eArgFormat. */
   uint64_t arg_buffer;

   /* Address of index buffer. */
   uint64_t index_buffer;

   /* The raw (without addr msb in [7:0]) index block header. */
   uint32_t index_block_header;

   /* Number of bytes per index. */
   uint32_t index_stride;

   /* Used during/after compilation to fill in constant buffer. */
   struct pvr_psc_register data[32];

   /* Results of compilation. */
   struct pvr_psc_program_output program;

   /* This is used for ARB_multi_draw_indirect. */
   unsigned int count;
   unsigned int stride;

   /* Internal stuff. */
   unsigned int num_views;

   bool support_base_instance;
   bool increment_draw_id;
};

void pvr_pds_generate_draw_arrays_indirect(
   struct pvr_pds_drawindirect_program *restrict program,
   uint32_t *restrict buffer,
   enum pvr_pds_generate_mode gen_mode,
   const struct pvr_device_info *dev_info);
void pvr_pds_generate_draw_elements_indirect(
   struct pvr_pds_drawindirect_program *restrict program,
   uint32_t *restrict buffer,
   enum pvr_pds_generate_mode gen_mode,
   const struct pvr_device_info *dev_info);

uint64_t pvr_pds_encode_st_src0(uint64_t src,
                                uint64_t count4,
                                uint64_t dst_add,
                                bool write_through,
                                const struct pvr_device_info *dev_info);

uint64_t pvr_pds_encode_ld_src0(uint64_t dest,
                                uint64_t count8,
                                uint64_t src_add,
                                bool cached,
                                const struct pvr_device_info *dev_info);

uint32_t *pvr_pds_generate_single_ldst_instruction(
   bool ld,
   const struct pvr_pds_ldst_control *control,
   uint32_t temp_index,
   uint64_t address,
   uint32_t count,
   uint32_t *next_constant,
   uint32_t *total_data_size,
   uint32_t *total_code_size,
   uint32_t *buffer,
   bool data_fence,
   enum pvr_pds_generate_mode gen_mode,
   const struct pvr_device_info *dev_info);
struct pvr_pds_descriptor_set {
   unsigned int descriptor_set; /* id of the descriptor set. */
   unsigned int size_in_dwords; /* Number of dwords to transfer. */
   unsigned int destination; /* Destination shared register to which
                              * descriptor entries should be loaded.
                              */
   bool primary; /* Primary or secondary? */
   unsigned int offset_in_dwords; /* Offset from the start of the descriptor
                                   * set to start DMA'ing from.
                                   */
};

struct pvr_pds_addr_literal {
   enum pvr_pds_addr_literal_type type;
   unsigned int destination;
};

#define PVR_BUFFER_TYPE_UBO (0)
#define PVR_BUFFER_TYPE_COMPILE_TIME (1)
#define PVR_BUFFER_TYPE_BLEND_CONSTS (2)
#define PVR_BUFFER_TYPE_PUSH_CONSTS (3)
#define PVR_BUFFER_TYPE_BUFFER_LENGTHS (4)
#define PVR_BUFFER_TYPE_DYNAMIC (5)
#define PVR_BUFFER_TYPE_UBO_ZEROING (6)
#define PVR_BUFFER_TYPE_INVALID (~0)

struct pvr_pds_buffer {
   uint16_t type;

   uint16_t size_in_dwords;
   uint32_t destination;

   union {
      uint32_t *data;
      struct {
         uint32_t buffer_id;
         uint16_t desc_set;
         uint16_t binding;
         uint32_t source_offset;
      };
   };
};

#define PVR_PDS_MAX_BUFFERS (24)

struct pvr_pds_descriptor_program_input {
   /* User-specified descriptor sets. */
   unsigned int descriptor_set_count;
   struct pvr_pds_descriptor_set descriptor_sets[8];

   unsigned int addr_literal_count;
   struct pvr_pds_addr_literal addr_literals[8];

   /* "State" buffers, including:
    * compile-time constants
    * blend constants
    * push constants
    * UBOs that have been hoisted.
    */
   uint32_t buffer_count;
   struct pvr_pds_buffer buffers[PVR_PDS_MAX_BUFFERS];

   uint32_t blend_constants_used_mask;

   bool secondary_program_present;
   struct pvr_pds_usc_task_control secondary_task_control;

   bool must_not_be_empty;
};

#define PVR_PDS_VERTEX_FLAGS_VERTEX_ID_REQUIRED BITFIELD_BIT(0U)
#define PVR_PDS_VERTEX_FLAGS_INSTANCE_ID_REQUIRED BITFIELD_BIT(1U)
#define PVR_PDS_VERTEX_FLAGS_DRAW_INDIRECT_VARIANT BITFIELD_BIT(2U)
#define PVR_PDS_VERTEX_FLAGS_BASE_INSTANCE_VARIANT BITFIELD_BIT(3U)
#define PVR_PDS_VERTEX_FLAGS_BASE_INSTANCE_REQUIRED BITFIELD_BIT(4U)

/* BaseVertex is used in shader. */
#define PVR_PDS_VERTEX_FLAGS_BASE_VERTEX_REQUIRED BITFIELD_BIT(5U)

#define PVR_PDS_VERTEX_FLAGS_DRAW_INDEX_REQUIRED BITFIELD_BIT(6U)

#define PVR_PDS_VERTEX_DMA_FLAGS_INSTANCE_RATE BITFIELD_BIT(0U)

struct pvr_pds_vertex_dma {
   /* Try and keep this structure packing as small as possible. */
   uint16_t offset;
   uint16_t stride;

   uint8_t flags;
   uint8_t size_in_dwords;
   uint8_t component_size_in_bytes;
   uint8_t destination;
   uint8_t binding_index;
   uint32_t divisor;

   uint16_t robustness_buffer_offset;
};

struct pvr_pds_vertex_primary_program_input {
   /* Control for the DOUTU that kicks the vertex USC shader. */
   struct pvr_pds_usc_task_control usc_task_control;
   /* List of DMAs (of size dma_count). */
   const struct pvr_pds_vertex_dma *dma_list;
   uint32_t dma_count;

   /* ORd bitfield of PVR_PDS_VERTEX_FLAGS_* */
   uint32_t flags;

   uint16_t vertex_id_register;
   uint16_t instance_id_register;

   /* API provided baseInstance (i.e. not from drawIndirect). */
   uint32_t base_instance;

   uint16_t base_instance_register;
   uint16_t base_vertex_register;
   uint16_t draw_index_register;
};

#define PVR_PDS_CONST_MAP_ENTRY_TYPE_NULL (0)
#define PVR_PDS_CONST_MAP_ENTRY_TYPE_LITERAL64 (1)
#define PVR_PDS_CONST_MAP_ENTRY_TYPE_LITERAL32 (2)
#define PVR_PDS_CONST_MAP_ENTRY_TYPE_DESCRIPTOR_SET (3)
#define PVR_PDS_CONST_MAP_ENTRY_TYPE_CONSTANT_BUFFER (4)
#define PVR_PDS_CONST_MAP_ENTRY_TYPE_SPECIAL_BUFFER (5)
#define PVR_PDS_CONST_MAP_ENTRY_TYPE_DOUTU_ADDRESS (6)
#define PVR_PDS_CONST_MAP_ENTRY_TYPE_VERTEX_ATTRIBUTE_ADDRESS (7)
#define PVR_PDS_CONST_MAP_ENTRY_TYPE_ROBUST_VERTEX_ATTRIBUTE_ADDRESS (8)

/* Use if pds_ddmadt is enabled. */
#define PVR_PDS_CONST_MAP_ENTRY_TYPE_VERTEX_ATTR_DDMADT_OOB_BUFFER_SIZE (9)

/* Use if pds_ddmadt is not enabled. */
#define PVR_PDS_CONST_MAP_ENTRY_TYPE_VERTEX_ATTRIBUTE_MAX_INDEX (9)

#define PVR_PDS_CONST_MAP_ENTRY_TYPE_BASE_INSTANCE (10)
#define PVR_PDS_CONST_MAP_ENTRY_TYPE_CONSTANT_BUFFER_ZEROING (11)
#define PVR_PDS_CONST_MAP_ENTRY_TYPE_BASE_VERTEX (12)
#define PVR_PDS_CONST_MAP_ENTRY_TYPE_BASE_WORKGROUP (13)
#define PVR_PDS_CONST_MAP_ENTRY_TYPE_COND_RENDER (14)

#define PVR_PDS_CONST_MAP_ENTRY_TYPE_ADDR_LITERAL_BUFFER (15)
#define PVR_PDS_CONST_MAP_ENTRY_TYPE_ADDR_LITERAL (16)

/* We pack all the following structs tightly into a buffer using += sizeof(x)
 * offsets, this can lead to data that is not native aligned. Supplying the
 * packed attribute indicates that unaligned accesses may be required, and the
 * aligned attribute causes the size of the structure to be aligned to a
 * specific boundary.
 */
#define PVR_ALIGNED __attribute__((packed, aligned(1)))

struct pvr_const_map_entry {
   uint8_t type;
   uint8_t const_offset;
} PVR_ALIGNED;

struct pvr_const_map_entry_literal32 {
   uint8_t type;
   uint8_t const_offset;

   uint32_t literal_value;
} PVR_ALIGNED;

struct pvr_const_map_entry_literal64 {
   uint8_t type;
   uint8_t const_offset;

   uint64_t literal_value;
} PVR_ALIGNED;

struct pvr_const_map_entry_descriptor_set {
   uint8_t type;
   uint8_t const_offset;

   uint32_t descriptor_set;
   PVR_PDS_BOOL primary;
   uint32_t offset_in_dwords;
} PVR_ALIGNED;

struct pvr_const_map_entry_constant_buffer {
   uint8_t type;
   uint8_t const_offset;

   uint16_t buffer_id;
   uint16_t desc_set;
   uint16_t binding;
   uint32_t offset;
   uint32_t size_in_dwords;
} PVR_ALIGNED;

struct pvr_const_map_entry_constant_buffer_zeroing {
   uint8_t type;
   uint8_t const_offset;

   uint16_t buffer_id;
   uint32_t offset;
   uint32_t size_in_dwords;
} PVR_ALIGNED;

struct pvr_const_map_entry_special_buffer {
   uint8_t type;
   uint8_t const_offset;

   uint8_t buffer_type;
   uint32_t buffer_index;
} PVR_ALIGNED;

struct pvr_const_map_entry_doutu_address {
   uint8_t type;
   uint8_t const_offset;

   uint64_t doutu_control;
} PVR_ALIGNED;

struct pvr_const_map_entry_vertex_attribute_address {
   uint8_t type;
   uint8_t const_offset;

   uint16_t offset;
   uint16_t stride;
   uint8_t binding_index;
   uint8_t size_in_dwords;
} PVR_ALIGNED;

struct pvr_const_map_entry_robust_vertex_attribute_address {
   uint8_t type;
   uint8_t const_offset;

   uint16_t offset;
   uint16_t stride;
   uint8_t binding_index;
   uint8_t size_in_dwords;
   uint16_t robustness_buffer_offset;
   uint8_t component_size_in_bytes;
} PVR_ALIGNED;

struct pvr_const_map_entry_vertex_attribute_max_index {
   uint8_t type;
   uint8_t const_offset;

   uint8_t binding_index;
   uint8_t size_in_dwords;
   uint16_t offset;
   uint16_t stride;
   uint8_t component_size_in_bytes;
} PVR_ALIGNED;

struct pvr_const_map_entry_base_instance {
   uint8_t type;
   uint8_t const_offset;
} PVR_ALIGNED;

struct pvr_const_map_entry_base_vertex {
   uint8_t type;
   uint8_t const_offset;
};

struct pvr_pds_const_map_entry_base_workgroup {
   uint8_t type;
   uint8_t const_offset;
   uint8_t workgroup_component;
} PVR_ALIGNED;

struct pvr_pds_const_map_entry_vertex_attr_ddmadt_oob_buffer_size {
   uint8_t type;
   uint8_t const_offset;
   uint8_t binding_index;
} PVR_ALIGNED;

struct pvr_pds_const_map_entry_cond_render {
   uint8_t type;
   uint8_t const_offset;

   uint32_t cond_render_pred_temp;
} PVR_ALIGNED;

struct pvr_pds_const_map_entry_addr_literal_buffer {
   uint8_t type;
   uint8_t const_offset;

   uint32_t size;
} PVR_ALIGNED;

struct pvr_pds_const_map_entry_addr_literal {
   uint8_t type;
   enum pvr_pds_addr_literal_type addr_type;
} PVR_ALIGNED;

struct pvr_pds_info {
   uint32_t temps_required;
   uint32_t code_size_in_dwords;
   uint32_t data_size_in_dwords;

   uint32_t entry_count;
   size_t entries_size_in_bytes;
   size_t entries_written_size_in_bytes;
   struct pvr_const_map_entry *entries;
};

void pvr_pds_generate_descriptor_upload_program(
   struct pvr_pds_descriptor_program_input *input_program,
   uint32_t *code_section,
   struct pvr_pds_info *info);
void pvr_pds_generate_vertex_primary_program(
   struct pvr_pds_vertex_primary_program_input *input_program,
   uint32_t *code,
   struct pvr_pds_info *info,
   bool use_robust_vertex_fetch,
   const struct pvr_device_info *dev_info);

/**
 * Generate USC address.
 *
 * \param doutu Location to write the generated address.
 * \param execution_address Address to generate from.
 */
static ALWAYS_INLINE void
pvr_set_usc_execution_address64(uint64_t *doutu, uint64_t execution_address)
{
   doutu[0] |= (((execution_address >>
                  PVR_ROGUE_PDSINST_DOUT_FIELDS_DOUTU_SRC0_EXE_OFF_ALIGNSHIFT)
                 << PVR_ROGUE_PDSINST_DOUT_FIELDS_DOUTU_SRC0_EXE_OFF_SHIFT) &
                ~PVR_ROGUE_PDSINST_DOUT_FIELDS_DOUTU_SRC0_EXE_OFF_CLRMSK);
}

#endif /* PVR_PDS_H */
