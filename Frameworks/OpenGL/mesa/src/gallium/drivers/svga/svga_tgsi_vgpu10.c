/**********************************************************
 * Copyright 1998-2022 VMware, Inc.  All rights reserved.
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without
 * restriction, including without limitation the rights to use, copy,
 * modify, merge, publish, distribute, sublicense, and/or sell copies
 * of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
 * BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
 * ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 **********************************************************/

/**
 * @file svga_tgsi_vgpu10.c
 *
 * TGSI -> VGPU10 shader translation.
 *
 * \author Mingcheng Chen
 * \author Brian Paul
 */

#include "util/compiler.h"
#include "pipe/p_shader_tokens.h"
#include "pipe/p_defines.h"
#include "tgsi/tgsi_dump.h"
#include "tgsi/tgsi_info.h"
#include "tgsi/tgsi_parse.h"
#include "tgsi/tgsi_scan.h"
#include "tgsi/tgsi_strings.h"
#include "tgsi/tgsi_two_side.h"
#include "tgsi/tgsi_aa_point.h"
#include "tgsi/tgsi_util.h"
#include "util/u_math.h"
#include "util/u_memory.h"
#include "util/u_bitmask.h"
#include "util/u_debug.h"
#include "util/u_pstipple.h"

#include "svga_context.h"
#include "svga_debug.h"
#include "svga_link.h"
#include "svga_shader.h"
#include "svga_tgsi.h"

#include "VGPU10ShaderTokens.h"


#define INVALID_INDEX 99999
#define MAX_INTERNAL_TEMPS 4
#define MAX_SYSTEM_VALUES 4
#define MAX_IMMEDIATE_COUNT \
        (VGPU10_MAX_IMMEDIATE_CONSTANT_BUFFER_ELEMENT_COUNT/4)
#define MAX_TEMP_ARRAYS 64  /* Enough? */

/**
 * Clipping is complicated.  There's four different cases which we
 * handle during VS/GS shader translation:
 */
enum clipping_mode
{
   CLIP_NONE,     /**< No clipping enabled */
   CLIP_LEGACY,   /**< The shader has no clipping declarations or code but
                   * one or more user-defined clip planes are enabled.  We
                   * generate extra code to emit clip distances.
                   */
   CLIP_DISTANCE, /**< The shader already declares clip distance output
                   * registers and has code to write to them.
                   */
   CLIP_VERTEX    /**< The shader declares a clip vertex output register and
                  * has code that writes to the register.  We convert the
                  * clipvertex position into one or more clip distances.
                  */
};


/* Shader signature info */
struct svga_shader_signature
{
   SVGA3dDXShaderSignatureHeader header;
   SVGA3dDXShaderSignatureEntry inputs[PIPE_MAX_SHADER_INPUTS];
   SVGA3dDXShaderSignatureEntry outputs[PIPE_MAX_SHADER_OUTPUTS];
   SVGA3dDXShaderSignatureEntry patchConstants[PIPE_MAX_SHADER_OUTPUTS];
};

static inline void
set_shader_signature_entry(SVGA3dDXShaderSignatureEntry *e,
                           unsigned index,
                           SVGA3dDXSignatureSemanticName sgnName,
                           unsigned mask,
                           SVGA3dDXSignatureRegisterComponentType compType,
                           SVGA3dDXSignatureMinPrecision minPrecision)
{
   e->registerIndex = index;
   e->semanticName = sgnName;
   e->mask = mask;
   e->componentType = compType;
   e->minPrecision = minPrecision;
};

static const SVGA3dDXSignatureSemanticName
tgsi_semantic_to_sgn_name[TGSI_SEMANTIC_COUNT] = {
   SVGADX_SIGNATURE_SEMANTIC_NAME_POSITION,
   SVGADX_SIGNATURE_SEMANTIC_NAME_UNDEFINED,
   SVGADX_SIGNATURE_SEMANTIC_NAME_UNDEFINED,
   SVGADX_SIGNATURE_SEMANTIC_NAME_UNDEFINED,
   SVGADX_SIGNATURE_SEMANTIC_NAME_UNDEFINED,
   SVGADX_SIGNATURE_SEMANTIC_NAME_UNDEFINED,
   SVGADX_SIGNATURE_SEMANTIC_NAME_UNDEFINED,
   SVGADX_SIGNATURE_SEMANTIC_NAME_IS_FRONT_FACE,
   SVGADX_SIGNATURE_SEMANTIC_NAME_UNDEFINED,
   SVGADX_SIGNATURE_SEMANTIC_NAME_PRIMITIVE_ID,
   SVGADX_SIGNATURE_SEMANTIC_NAME_INSTANCE_ID,
   SVGADX_SIGNATURE_SEMANTIC_NAME_VERTEX_ID,
   SVGADX_SIGNATURE_SEMANTIC_NAME_UNDEFINED,
   SVGADX_SIGNATURE_SEMANTIC_NAME_CLIP_DISTANCE,
   SVGADX_SIGNATURE_SEMANTIC_NAME_UNDEFINED,
   SVGADX_SIGNATURE_SEMANTIC_NAME_UNDEFINED,
   SVGADX_SIGNATURE_SEMANTIC_NAME_UNDEFINED,
   SVGADX_SIGNATURE_SEMANTIC_NAME_UNDEFINED,
   SVGADX_SIGNATURE_SEMANTIC_NAME_UNDEFINED,
   SVGADX_SIGNATURE_SEMANTIC_NAME_UNDEFINED,
   SVGADX_SIGNATURE_SEMANTIC_NAME_UNDEFINED,
   SVGADX_SIGNATURE_SEMANTIC_NAME_VIEWPORT_ARRAY_INDEX,
   SVGADX_SIGNATURE_SEMANTIC_NAME_RENDER_TARGET_ARRAY_INDEX,
   SVGADX_SIGNATURE_SEMANTIC_NAME_SAMPLE_INDEX,
   SVGADX_SIGNATURE_SEMANTIC_NAME_UNDEFINED,
   SVGADX_SIGNATURE_SEMANTIC_NAME_UNDEFINED,
   SVGADX_SIGNATURE_SEMANTIC_NAME_INSTANCE_ID,
   SVGADX_SIGNATURE_SEMANTIC_NAME_VERTEX_ID,
   SVGADX_SIGNATURE_SEMANTIC_NAME_UNDEFINED,
   SVGADX_SIGNATURE_SEMANTIC_NAME_UNDEFINED,
   SVGADX_SIGNATURE_SEMANTIC_NAME_UNDEFINED,
   SVGADX_SIGNATURE_SEMANTIC_NAME_UNDEFINED,
   SVGADX_SIGNATURE_SEMANTIC_NAME_UNDEFINED,
   SVGADX_SIGNATURE_SEMANTIC_NAME_UNDEFINED,
   SVGADX_SIGNATURE_SEMANTIC_NAME_UNDEFINED,
   SVGADX_SIGNATURE_SEMANTIC_NAME_UNDEFINED,
   SVGADX_SIGNATURE_SEMANTIC_NAME_UNDEFINED,
   SVGADX_SIGNATURE_SEMANTIC_NAME_UNDEFINED,
   SVGADX_SIGNATURE_SEMANTIC_NAME_UNDEFINED,
   SVGADX_SIGNATURE_SEMANTIC_NAME_UNDEFINED,
   SVGADX_SIGNATURE_SEMANTIC_NAME_UNDEFINED,
   SVGADX_SIGNATURE_SEMANTIC_NAME_UNDEFINED,
   SVGADX_SIGNATURE_SEMANTIC_NAME_UNDEFINED,
   SVGADX_SIGNATURE_SEMANTIC_NAME_UNDEFINED,
   SVGADX_SIGNATURE_SEMANTIC_NAME_UNDEFINED
};


/**
 * Map tgsi semantic name to SVGA signature semantic name
 */
static inline SVGA3dDXSignatureSemanticName
map_tgsi_semantic_to_sgn_name(enum tgsi_semantic name)
{
   assert(name < TGSI_SEMANTIC_COUNT);

   /* Do a few asserts here to spot check the mapping */
   assert(tgsi_semantic_to_sgn_name[TGSI_SEMANTIC_PRIMID] ==
          SVGADX_SIGNATURE_SEMANTIC_NAME_PRIMITIVE_ID);
   assert(tgsi_semantic_to_sgn_name[TGSI_SEMANTIC_VIEWPORT_INDEX] ==
          SVGADX_SIGNATURE_SEMANTIC_NAME_VIEWPORT_ARRAY_INDEX);
   assert(tgsi_semantic_to_sgn_name[TGSI_SEMANTIC_INVOCATIONID] ==
          SVGADX_SIGNATURE_SEMANTIC_NAME_INSTANCE_ID);

   return tgsi_semantic_to_sgn_name[name];
}

enum reemit_mode {
   REEMIT_FALSE = 0,
   REEMIT_TRUE = 1,
   REEMIT_IN_PROGRESS = 2
};

struct svga_raw_buf_tmp {
   bool indirect;
   unsigned buffer_index:8;
   unsigned element_index:8;
   unsigned element_rel:8;
};

struct svga_shader_emitter_v10
{
   /* The token output buffer */
   unsigned size;
   char *buf;
   char *ptr;

   /* Information about the shader and state (does not change) */
   struct svga_compile_key key;
   struct tgsi_shader_info info;
   unsigned unit;
   unsigned version; /**< Either 40, 41, 50 or 51 at this time */

   unsigned cur_tgsi_token;     /**< current tgsi token position */
   unsigned inst_start_token;
   bool discard_instruction; /**< throw away current instruction? */
   bool reemit_instruction;  /**< reemit current instruction */
   bool reemit_tgsi_instruction;  /**< reemit current tgsi instruction */
   bool skip_instruction;    /**< skip current instruction */
   bool use_sampler_state_mapping; /* use sampler state mapping */
   enum reemit_mode reemit_rawbuf_instruction;

   union tgsi_immediate_data immediates[MAX_IMMEDIATE_COUNT][4];
   double (*immediates_dbl)[2];
   unsigned num_immediates;      /**< Number of immediates emitted */
   unsigned common_immediate_pos[20];  /**< literals for common immediates */
   unsigned num_common_immediates;
   unsigned num_immediates_emitted;
   unsigned num_new_immediates;        /** pending immediates to be declared */
   unsigned immediates_block_start_token;
   unsigned immediates_block_next_token;

   unsigned num_outputs;      /**< include any extra outputs */
                              /**  The first extra output is reserved for
                               *   non-adjusted vertex position for
                               *   stream output purpose
                               */

   /* Temporary Registers */
   unsigned num_shader_temps; /**< num of temps used by original shader */
   unsigned internal_temp_count;  /**< currently allocated internal temps */
   struct {
      unsigned start, size;
   } temp_arrays[MAX_TEMP_ARRAYS];
   unsigned num_temp_arrays;

   /** Map TGSI temp registers to VGPU10 temp array IDs and indexes */
   struct {
      unsigned arrayId, index;
      bool initialized;
   } temp_map[VGPU10_MAX_TEMPS]; /**< arrayId, element */

   unsigned initialize_temp_index;

   /** Number of constants used by original shader for each constant buffer.
    * The size should probably always match with that of svga_state.constbufs.
    */
   unsigned num_shader_consts[SVGA_MAX_CONST_BUFS];

   /* Raw constant buffers */
   unsigned raw_buf_srv_start_index;  /* starting srv index for raw buffers */
   unsigned raw_bufs;                 /* raw buffers bitmask */
   unsigned raw_buf_tmp_index;        /* starting temp index for raw buffers */
   unsigned raw_buf_cur_tmp_index;    /* current temp index for raw buffers */
   struct svga_raw_buf_tmp raw_buf_tmp[3]; /* temporaries for raw buf source */

   /* Samplers */
   unsigned num_samplers;
   bool sampler_view[PIPE_MAX_SAMPLERS];  /**< True if sampler view exists*/
   uint8_t sampler_target[PIPE_MAX_SAMPLERS];  /**< TGSI_TEXTURE_x */
   uint8_t sampler_return_type[PIPE_MAX_SAMPLERS];  /**< TGSI_RETURN_TYPE_x */

   /* Images */
   unsigned num_images;
   unsigned image_mask;
   struct tgsi_declaration_image image[PIPE_MAX_SHADER_IMAGES];
   unsigned image_size_index;  /* starting index to cbuf for image size */

   /* Shader buffers */
   unsigned num_shader_bufs;
   unsigned raw_shaderbuf_srv_start_index;  /* starting srv index for raw shaderbuf */
   uint64_t raw_shaderbufs;                 /* raw shader buffers bitmask */

   /* HW atomic buffers */
   unsigned num_atomic_bufs;
   unsigned atomic_bufs_mask;
   unsigned max_atomic_counter_index;
   VGPU10_OPCODE_TYPE cur_atomic_opcode;    /* current atomic opcode */

   bool uav_declared;  /* True if uav is declared */

   /* Index Range declaration */
   struct {
      unsigned start_index;
      unsigned count;
      bool required;
      unsigned operandType;
      unsigned size;
      unsigned dim;
   } index_range;

   /* Address regs (really implemented with temps) */
   unsigned num_address_regs;
   unsigned address_reg_index[MAX_VGPU10_ADDR_REGS];

   /* Output register usage masks */
   uint8_t output_usage_mask[PIPE_MAX_SHADER_OUTPUTS];

   /* To map TGSI system value index to VGPU shader input indexes */
   uint8_t system_value_indexes[MAX_SYSTEM_VALUES];

   struct {
      /* vertex position scale/translation */
      unsigned out_index;  /**< the real position output reg */
      unsigned tmp_index;  /**< the fake/temp position output reg */
      unsigned so_index;   /**< the non-adjusted position output reg */
      unsigned prescale_cbuf_index;  /* index to the const buf for prescale */
      unsigned prescale_scale_index, prescale_trans_index;
      unsigned num_prescale;      /* number of prescale factor in const buf */
      unsigned viewport_index;
      unsigned need_prescale:1;
      unsigned have_prescale:1;
   } vposition;

   /* Shader limits */
   unsigned max_vs_inputs;
   unsigned max_vs_outputs;
   unsigned max_gs_inputs;

   /* For vertex shaders only */
   struct {
      /* viewport constant */
      unsigned viewport_index;

      unsigned vertex_id_bias_index;
      unsigned vertex_id_sys_index;
      unsigned vertex_id_tmp_index;

      /* temp index of adjusted vertex attributes */
      unsigned adjusted_input[PIPE_MAX_SHADER_INPUTS];
   } vs;

   /* For fragment shaders only */
   struct {
      unsigned color_out_index[PIPE_MAX_COLOR_BUFS];  /**< the real color output regs */
      unsigned num_color_outputs;
      unsigned color_tmp_index;  /**< fake/temp color output reg */
      unsigned alpha_ref_index;  /**< immediate constant for alpha ref */

      /* front-face */
      unsigned face_input_index; /**< real fragment shader face reg (bool) */
      unsigned face_tmp_index;   /**< temp face reg converted to -1 / +1 */

      unsigned pstipple_sampler_unit;
      unsigned pstipple_sampler_state_index;

      unsigned fragcoord_input_index;  /**< real fragment position input reg */
      unsigned fragcoord_tmp_index;    /**< 1/w modified position temp reg */

      unsigned sample_id_sys_index;  /**< TGSI index of sample id sys value */

      unsigned sample_pos_sys_index; /**< TGSI index of sample pos sys value */
      unsigned sample_pos_tmp_index; /**< which temp reg has the sample pos */

      /** TGSI index of sample mask input sys value */
      unsigned sample_mask_in_sys_index;

      /* layer */
      unsigned layer_input_index;    /**< TGSI index of layer */
      unsigned layer_imm_index;      /**< immediate for default layer 0 */

      bool forceEarlyDepthStencil;  /**< true if Early Depth stencil test is enabled */
   } fs;

   /* For geometry shaders only */
   struct {
      VGPU10_PRIMITIVE prim_type;/**< VGPU10 primitive type */
      VGPU10_PRIMITIVE_TOPOLOGY prim_topology; /**< VGPU10 primitive topology */
      unsigned input_size;       /**< size of input arrays */
      unsigned prim_id_index;    /**< primitive id register index */
      unsigned max_out_vertices; /**< maximum number of output vertices */
      unsigned invocations;
      unsigned invocation_id_sys_index;

      unsigned viewport_index_out_index;
      unsigned viewport_index_tmp_index;
   } gs;

   /* For tessellation control shaders only */
   struct {
      unsigned vertices_per_patch_index;     /**< vertices_per_patch system value index */
      unsigned imm_index;                    /**< immediate for tcs */
      unsigned invocation_id_sys_index;      /**< invocation id */
      unsigned invocation_id_tmp_index;
      unsigned instruction_token_pos;        /* token pos for the first instruction */
      unsigned control_point_input_index;    /* control point input register index */
      unsigned control_point_addr_index;     /* control point input address register */
      unsigned control_point_out_index;      /* control point output register index */
      unsigned control_point_tmp_index;      /* control point temporary register */
      unsigned control_point_out_count;      /* control point output count */
      bool  control_point_phase;          /* true if in control point phase */
      bool  fork_phase_add_signature;     /* true if needs to add signature in fork phase */
      unsigned patch_generic_out_count;      /* per-patch generic output count */
      unsigned patch_generic_out_index;      /* per-patch generic output register index*/
      unsigned patch_generic_tmp_index;      /* per-patch generic temporary register index*/
      unsigned prim_id_index;                /* primitive id */
      struct {
         unsigned out_index;      /* real tessinner output register */
         unsigned temp_index;     /* tessinner temp register */
         unsigned tgsi_index;     /* tgsi tessinner output register */
      } inner;
      struct {
         unsigned out_index;      /* real tessouter output register */
         unsigned temp_index;     /* tessouter temp register */
         unsigned tgsi_index;     /* tgsi tessouter output register */
      } outer;
   } tcs;

   /* For tessellation evaluation shaders only */
   struct {
      enum mesa_prim prim_mode;
      enum pipe_tess_spacing spacing;
      bool vertices_order_cw;
      bool point_mode;
      unsigned tesscoord_sys_index;
      unsigned swizzle_max;
      unsigned prim_id_index;                /* primitive id */
      struct {
         unsigned in_index;       /* real tessinner input register */
         unsigned temp_index;     /* tessinner temp register */
         unsigned tgsi_index;     /* tgsi tessinner input register */
      } inner;
      struct {
         unsigned in_index;       /* real tessouter input register */
         unsigned temp_index;     /* tessouter temp register */
         unsigned tgsi_index;     /* tgsi tessouter input register */
      } outer;
   } tes;

   struct {
      unsigned block_width;       /* thread group size in x dimension */
      unsigned block_height;      /* thread group size in y dimension */
      unsigned block_depth;       /* thread group size in z dimension */
      unsigned thread_id_index;   /* thread id tgsi index */
      unsigned block_id_index;    /* block id tgsi index */
      bool shared_memory_declared;    /* set if shared memory is declared */
      struct {
         unsigned tgsi_index;   /* grid size tgsi index */
         unsigned imm_index;    /* grid size imm index */
      } grid_size;
   } cs;

   /* For vertex or geometry shaders */
   enum clipping_mode clip_mode;
   unsigned clip_dist_out_index; /**< clip distance output register index */
   unsigned clip_dist_tmp_index; /**< clip distance temporary register */
   unsigned clip_dist_so_index;  /**< clip distance shadow copy */

   /** Index of temporary holding the clipvertex coordinate */
   unsigned clip_vertex_out_index; /**< clip vertex output register index */
   unsigned clip_vertex_tmp_index; /**< clip vertex temporary index */

   /* user clip plane constant slot indexes */
   unsigned clip_plane_const[PIPE_MAX_CLIP_PLANES];

   unsigned num_output_writes;
   bool constant_color_output;

   bool uses_flat_interp;

   unsigned reserved_token;        /* index to the reserved token */
   bool uses_precise_qualifier;

   /* For all shaders: const reg index for RECT coord scaling */
   unsigned texcoord_scale_index[PIPE_MAX_SAMPLERS];

   /* For all shaders: const reg index for texture buffer size */
   unsigned texture_buffer_size_index[PIPE_MAX_SAMPLERS];

   /** Which texture units are doing shadow comparison in the shader code */
   unsigned shadow_compare_units;

   /* VS/TCS/TES/GS/FS Linkage info */
   struct shader_linkage linkage;
   struct tgsi_shader_info *prevShaderInfo;

   /* Shader signature */
   struct svga_shader_signature signature;

   bool register_overflow;  /**< Set if we exceed a VGPU10 register limit */

   /* For util_debug_message */
   struct util_debug_callback svga_debug_callback;

   /* current loop depth in shader */
   unsigned current_loop_depth;
};


static void emit_tcs_input_declarations(struct svga_shader_emitter_v10 *emit);
static void emit_tcs_output_declarations(struct svga_shader_emitter_v10 *emit);
static bool emit_temporaries_declaration(struct svga_shader_emitter_v10 *emit);
static bool emit_constant_declaration(struct svga_shader_emitter_v10 *emit);
static bool emit_sampler_declarations(struct svga_shader_emitter_v10 *emit);
static bool emit_resource_declarations(struct svga_shader_emitter_v10 *emit);
static bool emit_vgpu10_immediates_block(struct svga_shader_emitter_v10 *emit);
static bool emit_index_range_declaration(struct svga_shader_emitter_v10 *emit);
static void emit_image_declarations(struct svga_shader_emitter_v10 *emit);
static void emit_shader_buf_declarations(struct svga_shader_emitter_v10 *emit);
static void emit_atomic_buf_declarations(struct svga_shader_emitter_v10 *emit);
static void emit_temp_prescale_instructions(struct svga_shader_emitter_v10 *emit);

static bool
emit_post_helpers(struct svga_shader_emitter_v10 *emit);

static bool
emit_vertex(struct svga_shader_emitter_v10 *emit,
            const struct tgsi_full_instruction *inst);

static bool
emit_vgpu10_instruction(struct svga_shader_emitter_v10 *emit,
                        unsigned inst_number,
                        const struct tgsi_full_instruction *inst);

static void
emit_input_declaration(struct svga_shader_emitter_v10 *emit,
                       VGPU10_OPCODE_TYPE opcodeType,
                       VGPU10_OPERAND_TYPE operandType,
                       VGPU10_OPERAND_INDEX_DIMENSION dim,
                       unsigned index, unsigned size,
                       VGPU10_SYSTEM_NAME name,
                       VGPU10_OPERAND_NUM_COMPONENTS numComp,
                       VGPU10_OPERAND_4_COMPONENT_SELECTION_MODE selMode,
                       unsigned usageMask,
                       VGPU10_INTERPOLATION_MODE interpMode,
                       bool addSignature,
                       SVGA3dDXSignatureSemanticName sgnName);

static bool
emit_rawbuf_instruction(struct svga_shader_emitter_v10 *emit,
                        unsigned inst_number,
                        const struct tgsi_full_instruction *inst);

static void
create_temp_array(struct svga_shader_emitter_v10 *emit,
                  unsigned arrayID, unsigned first, unsigned count,
                  unsigned startIndex);

static char err_buf[128];

static bool
expand(struct svga_shader_emitter_v10 *emit)
{
   char *new_buf;
   unsigned newsize = emit->size * 2;

   if (emit->buf != err_buf)
      new_buf = REALLOC(emit->buf, emit->size, newsize);
   else
      new_buf = NULL;

   if (!new_buf) {
      emit->ptr = err_buf;
      emit->buf = err_buf;
      emit->size = sizeof(err_buf);
      return false;
   }

   emit->size = newsize;
   emit->ptr = new_buf + (emit->ptr - emit->buf);
   emit->buf = new_buf;
   return true;
}

/**
 * Create and initialize a new svga_shader_emitter_v10 object.
 */
static struct svga_shader_emitter_v10 *
alloc_emitter(void)
{
   struct svga_shader_emitter_v10 *emit = CALLOC(1, sizeof(*emit));

   if (!emit)
      return NULL;

   /* to initialize the output buffer */
   emit->size = 512;
   if (!expand(emit)) {
      FREE(emit);
      return NULL;
   }
   return emit;
}

/**
 * Free an svga_shader_emitter_v10 object.
 */
static void
free_emitter(struct svga_shader_emitter_v10 *emit)
{
   assert(emit);
   FREE(emit->buf);    /* will be NULL if translation succeeded */
   FREE(emit);
}

static inline bool
reserve(struct svga_shader_emitter_v10 *emit,
        unsigned nr_dwords)
{
   while (emit->ptr - emit->buf + nr_dwords * sizeof(uint32) >= emit->size) {
      if (!expand(emit))
         return false;
   }

   return true;
}

static bool
emit_dword(struct svga_shader_emitter_v10 *emit, uint32 dword)
{
   if (!reserve(emit, 1))
      return false;

   *(uint32 *)emit->ptr = dword;
   emit->ptr += sizeof dword;
   return true;
}

static bool
emit_dwords(struct svga_shader_emitter_v10 *emit,
            const uint32 *dwords,
            unsigned nr)
{
   if (!reserve(emit, nr))
      return false;

   memcpy(emit->ptr, dwords, nr * sizeof *dwords);
   emit->ptr += nr * sizeof *dwords;
   return true;
}

/** Return the number of tokens in the emitter's buffer */
static unsigned
emit_get_num_tokens(const struct svga_shader_emitter_v10 *emit)
{
   return (emit->ptr - emit->buf) / sizeof(unsigned);
}


/**
 * Check for register overflow.  If we overflow we'll set an
 * error flag.  This function can be called for register declarations
 * or use as src/dst instruction operands.
 * \param type  register type.  One of VGPU10_OPERAND_TYPE_x
                or VGPU10_OPCODE_DCL_x
 * \param index  the register index
 */
static void
check_register_index(struct svga_shader_emitter_v10 *emit,
                     unsigned operandType, unsigned index)
{
   bool overflow_before = emit->register_overflow;

   switch (operandType) {
   case VGPU10_OPERAND_TYPE_TEMP:
   case VGPU10_OPERAND_TYPE_INDEXABLE_TEMP:
   case VGPU10_OPCODE_DCL_TEMPS:
      if (index >= VGPU10_MAX_TEMPS) {
         emit->register_overflow = true;
      }
      break;
   case VGPU10_OPERAND_TYPE_CONSTANT_BUFFER:
   case VGPU10_OPCODE_DCL_CONSTANT_BUFFER:
      if (index >= VGPU10_MAX_CONSTANT_BUFFER_ELEMENT_COUNT) {
         emit->register_overflow = true;
      }
      break;
   case VGPU10_OPERAND_TYPE_INPUT:
   case VGPU10_OPERAND_TYPE_INPUT_PRIMITIVEID:
   case VGPU10_OPCODE_DCL_INPUT:
   case VGPU10_OPCODE_DCL_INPUT_SGV:
   case VGPU10_OPCODE_DCL_INPUT_SIV:
   case VGPU10_OPCODE_DCL_INPUT_PS:
   case VGPU10_OPCODE_DCL_INPUT_PS_SGV:
   case VGPU10_OPCODE_DCL_INPUT_PS_SIV:
      if ((emit->unit == PIPE_SHADER_VERTEX &&
           index >= emit->max_vs_inputs) ||
          (emit->unit == PIPE_SHADER_GEOMETRY &&
           index >= emit->max_gs_inputs) ||
          (emit->unit == PIPE_SHADER_FRAGMENT &&
           index >= VGPU10_MAX_FS_INPUTS) ||
          (emit->unit == PIPE_SHADER_TESS_CTRL &&
           index >= VGPU11_MAX_HS_INPUT_CONTROL_POINTS) ||
          (emit->unit == PIPE_SHADER_TESS_EVAL &&
           index >= VGPU11_MAX_DS_INPUT_CONTROL_POINTS)) {
         emit->register_overflow = true;
      }
      break;
   case VGPU10_OPERAND_TYPE_OUTPUT:
   case VGPU10_OPCODE_DCL_OUTPUT:
   case VGPU10_OPCODE_DCL_OUTPUT_SGV:
   case VGPU10_OPCODE_DCL_OUTPUT_SIV:
      /* Note: we are skipping two output indices in tcs for
       * tessinner/outer levels. Implementation will not exceed
       * number of output count but it allows index to go beyond
       * VGPU11_MAX_HS_OUTPUTS.
       * Index will never be >= index >= VGPU11_MAX_HS_OUTPUTS + 2
       */
      if ((emit->unit == PIPE_SHADER_VERTEX &&
           index >= emit->max_vs_outputs) ||
          (emit->unit == PIPE_SHADER_GEOMETRY &&
           index >= VGPU10_MAX_GS_OUTPUTS) ||
          (emit->unit == PIPE_SHADER_FRAGMENT &&
           index >= VGPU10_MAX_FS_OUTPUTS) ||
          (emit->unit == PIPE_SHADER_TESS_CTRL &&
           index >= VGPU11_MAX_HS_OUTPUTS + 2) ||
          (emit->unit == PIPE_SHADER_TESS_EVAL &&
           index >= VGPU11_MAX_DS_OUTPUTS)) {
         emit->register_overflow = true;
      }
      break;
   case VGPU10_OPERAND_TYPE_SAMPLER:
   case VGPU10_OPCODE_DCL_SAMPLER:
      if (index >= VGPU10_MAX_SAMPLERS) {
         emit->register_overflow = true;
      }
      break;
   case VGPU10_OPERAND_TYPE_RESOURCE:
   case VGPU10_OPCODE_DCL_RESOURCE:
      if (index >= VGPU10_MAX_RESOURCES) {
         emit->register_overflow = true;
      }
      break;
   case VGPU10_OPERAND_TYPE_IMMEDIATE_CONSTANT_BUFFER:
      if (index >= MAX_IMMEDIATE_COUNT) {
         emit->register_overflow = true;
      }
      break;
   case VGPU10_OPERAND_TYPE_OUTPUT_COVERAGE_MASK:
   case VGPU10_OPERAND_TYPE_INPUT_GS_INSTANCE_ID:
   case VGPU10_OPERAND_TYPE_OUTPUT_CONTROL_POINT_ID:
   case VGPU10_OPERAND_TYPE_INPUT_CONTROL_POINT:
   case VGPU10_OPERAND_TYPE_INPUT_DOMAIN_POINT:
   case VGPU10_OPERAND_TYPE_INPUT_PATCH_CONSTANT:
   case VGPU10_OPERAND_TYPE_INPUT_THREAD_GROUP_ID:
   case VGPU10_OPERAND_TYPE_INPUT_THREAD_ID_IN_GROUP:
      /* nothing */
      break;
   default:
      assert(0);
      ; /* nothing */
   }

   if (emit->register_overflow && !overflow_before) {
      debug_printf("svga: vgpu10 register overflow (reg %u, index %u)\n",
                   operandType, index);
   }
}


/**
 * Examine misc state to determine the clipping mode.
 */
static void
determine_clipping_mode(struct svga_shader_emitter_v10 *emit)
{
   /* num_written_clipdistance in the shader info for tessellation
    * control shader is always 0 because the TGSI_PROPERTY_NUM_CLIPDIST_ENABLED
    * is not defined for this shader. So we go through all the output declarations
    * to set the num_written_clipdistance. This is just to determine the
    * clipping mode.
    */
   if (emit->unit == PIPE_SHADER_TESS_CTRL) {
      unsigned i;
      for (i = 0; i < emit->info.num_outputs; i++) {
         if (emit->info.output_semantic_name[i] == TGSI_SEMANTIC_CLIPDIST) {
            emit->info.num_written_clipdistance =
               4 * (emit->info.output_semantic_index[i] + 1);
         }
      }
   }

   if (emit->info.num_written_clipdistance > 0) {
      emit->clip_mode = CLIP_DISTANCE;
   }
   else if (emit->info.writes_clipvertex) {
      emit->clip_mode = CLIP_VERTEX;
   }
   else if (emit->key.clip_plane_enable && emit->key.last_vertex_stage) {
      /*
       * Only the last shader in the vertex processing stage needs to
       * handle the legacy clip mode.
       */
      emit->clip_mode = CLIP_LEGACY;
   }
   else {
      emit->clip_mode = CLIP_NONE;
   }
}


/**
 * For clip distance register declarations and clip distance register
 * writes we need to mask the declaration usage or instruction writemask
 * (respectively) against the set of the really-enabled clipping planes.
 *
 * The piglit test spec/glsl-1.30/execution/clipping/vs-clip-distance-enables
 * has a VS that writes to all 8 clip distance registers, but the plane enable
 * flags are a subset of that.
 *
 * This function is used to apply the plane enable flags to the register
 * declaration or instruction writemask.
 *
 * \param writemask  the declaration usage mask or instruction writemask
 * \param clip_reg_index  which clip plane register is being declared/written.
 *                        The legal values are 0 and 1 (two clip planes per
 *                        register, for a total of 8 clip planes)
 */
static unsigned
apply_clip_plane_mask(struct svga_shader_emitter_v10 *emit,
                      unsigned writemask, unsigned clip_reg_index)
{
   unsigned shift;

   assert(clip_reg_index < 2);

   /* four clip planes per clip register: */
   shift = clip_reg_index * 4;
   writemask &= ((emit->key.clip_plane_enable >> shift) & 0xf);

   return writemask;
}


/**
 * Translate gallium shader type into VGPU10 type.
 */
static VGPU10_PROGRAM_TYPE
translate_shader_type(unsigned type)
{
   switch (type) {
   case PIPE_SHADER_VERTEX:
      return VGPU10_VERTEX_SHADER;
   case PIPE_SHADER_GEOMETRY:
      return VGPU10_GEOMETRY_SHADER;
   case PIPE_SHADER_FRAGMENT:
      return VGPU10_PIXEL_SHADER;
   case PIPE_SHADER_TESS_CTRL:
      return VGPU10_HULL_SHADER;
   case PIPE_SHADER_TESS_EVAL:
      return VGPU10_DOMAIN_SHADER;
   case PIPE_SHADER_COMPUTE:
      return VGPU10_COMPUTE_SHADER;
   default:
      assert(!"Unexpected shader type");
      return VGPU10_VERTEX_SHADER;
   }
}


/**
 * Translate a TGSI_OPCODE_x into a VGPU10_OPCODE_x
 * Note: we only need to translate the opcodes for "simple" instructions,
 * as seen below.  All other opcodes are handled/translated specially.
 */
static VGPU10_OPCODE_TYPE
translate_opcode(enum tgsi_opcode opcode)
{
   switch (opcode) {
   case TGSI_OPCODE_MOV:
      return VGPU10_OPCODE_MOV;
   case TGSI_OPCODE_MUL:
      return VGPU10_OPCODE_MUL;
   case TGSI_OPCODE_ADD:
      return VGPU10_OPCODE_ADD;
   case TGSI_OPCODE_DP3:
      return VGPU10_OPCODE_DP3;
   case TGSI_OPCODE_DP4:
      return VGPU10_OPCODE_DP4;
   case TGSI_OPCODE_MIN:
      return VGPU10_OPCODE_MIN;
   case TGSI_OPCODE_MAX:
      return VGPU10_OPCODE_MAX;
   case TGSI_OPCODE_MAD:
      return VGPU10_OPCODE_MAD;
   case TGSI_OPCODE_SQRT:
      return VGPU10_OPCODE_SQRT;
   case TGSI_OPCODE_FRC:
      return VGPU10_OPCODE_FRC;
   case TGSI_OPCODE_FLR:
      return VGPU10_OPCODE_ROUND_NI;
   case TGSI_OPCODE_FSEQ:
      return VGPU10_OPCODE_EQ;
   case TGSI_OPCODE_FSGE:
      return VGPU10_OPCODE_GE;
   case TGSI_OPCODE_FSNE:
      return VGPU10_OPCODE_NE;
   case TGSI_OPCODE_DDX:
      return VGPU10_OPCODE_DERIV_RTX;
   case TGSI_OPCODE_DDY:
      return VGPU10_OPCODE_DERIV_RTY;
   case TGSI_OPCODE_RET:
      return VGPU10_OPCODE_RET;
   case TGSI_OPCODE_DIV:
      return VGPU10_OPCODE_DIV;
   case TGSI_OPCODE_IDIV:
      return VGPU10_OPCODE_VMWARE;
   case TGSI_OPCODE_DP2:
      return VGPU10_OPCODE_DP2;
   case TGSI_OPCODE_BRK:
      return VGPU10_OPCODE_BREAK;
   case TGSI_OPCODE_IF:
      return VGPU10_OPCODE_IF;
   case TGSI_OPCODE_ELSE:
      return VGPU10_OPCODE_ELSE;
   case TGSI_OPCODE_ENDIF:
      return VGPU10_OPCODE_ENDIF;
   case TGSI_OPCODE_CEIL:
      return VGPU10_OPCODE_ROUND_PI;
   case TGSI_OPCODE_I2F:
      return VGPU10_OPCODE_ITOF;
   case TGSI_OPCODE_NOT:
      return VGPU10_OPCODE_NOT;
   case TGSI_OPCODE_TRUNC:
      return VGPU10_OPCODE_ROUND_Z;
   case TGSI_OPCODE_SHL:
      return VGPU10_OPCODE_ISHL;
   case TGSI_OPCODE_AND:
      return VGPU10_OPCODE_AND;
   case TGSI_OPCODE_OR:
      return VGPU10_OPCODE_OR;
   case TGSI_OPCODE_XOR:
      return VGPU10_OPCODE_XOR;
   case TGSI_OPCODE_CONT:
      return VGPU10_OPCODE_CONTINUE;
   case TGSI_OPCODE_EMIT:
      return VGPU10_OPCODE_EMIT;
   case TGSI_OPCODE_ENDPRIM:
      return VGPU10_OPCODE_CUT;
   case TGSI_OPCODE_BGNLOOP:
      return VGPU10_OPCODE_LOOP;
   case TGSI_OPCODE_ENDLOOP:
      return VGPU10_OPCODE_ENDLOOP;
   case TGSI_OPCODE_ENDSUB:
      return VGPU10_OPCODE_RET;
   case TGSI_OPCODE_NOP:
      return VGPU10_OPCODE_NOP;
   case TGSI_OPCODE_END:
      return VGPU10_OPCODE_RET;
   case TGSI_OPCODE_F2I:
      return VGPU10_OPCODE_FTOI;
   case TGSI_OPCODE_IMAX:
      return VGPU10_OPCODE_IMAX;
   case TGSI_OPCODE_IMIN:
      return VGPU10_OPCODE_IMIN;
   case TGSI_OPCODE_UDIV:
   case TGSI_OPCODE_UMOD:
   case TGSI_OPCODE_MOD:
      return VGPU10_OPCODE_UDIV;
   case TGSI_OPCODE_IMUL_HI:
      return VGPU10_OPCODE_IMUL;
   case TGSI_OPCODE_INEG:
      return VGPU10_OPCODE_INEG;
   case TGSI_OPCODE_ISHR:
      return VGPU10_OPCODE_ISHR;
   case TGSI_OPCODE_ISGE:
      return VGPU10_OPCODE_IGE;
   case TGSI_OPCODE_ISLT:
      return VGPU10_OPCODE_ILT;
   case TGSI_OPCODE_F2U:
      return VGPU10_OPCODE_FTOU;
   case TGSI_OPCODE_UADD:
      return VGPU10_OPCODE_IADD;
   case TGSI_OPCODE_U2F:
      return VGPU10_OPCODE_UTOF;
   case TGSI_OPCODE_UCMP:
      return VGPU10_OPCODE_MOVC;
   case TGSI_OPCODE_UMAD:
      return VGPU10_OPCODE_UMAD;
   case TGSI_OPCODE_UMAX:
      return VGPU10_OPCODE_UMAX;
   case TGSI_OPCODE_UMIN:
      return VGPU10_OPCODE_UMIN;
   case TGSI_OPCODE_UMUL:
   case TGSI_OPCODE_UMUL_HI:
      return VGPU10_OPCODE_UMUL;
   case TGSI_OPCODE_USEQ:
      return VGPU10_OPCODE_IEQ;
   case TGSI_OPCODE_USGE:
      return VGPU10_OPCODE_UGE;
   case TGSI_OPCODE_USHR:
      return VGPU10_OPCODE_USHR;
   case TGSI_OPCODE_USLT:
      return VGPU10_OPCODE_ULT;
   case TGSI_OPCODE_USNE:
      return VGPU10_OPCODE_INE;
   case TGSI_OPCODE_SWITCH:
      return VGPU10_OPCODE_SWITCH;
   case TGSI_OPCODE_CASE:
      return VGPU10_OPCODE_CASE;
   case TGSI_OPCODE_DEFAULT:
      return VGPU10_OPCODE_DEFAULT;
   case TGSI_OPCODE_ENDSWITCH:
      return VGPU10_OPCODE_ENDSWITCH;
   case TGSI_OPCODE_FSLT:
      return VGPU10_OPCODE_LT;
   case TGSI_OPCODE_ROUND:
      return VGPU10_OPCODE_ROUND_NE;
   /* Begin SM5 opcodes */
   case TGSI_OPCODE_F2D:
      return VGPU10_OPCODE_FTOD;
   case TGSI_OPCODE_D2F:
      return VGPU10_OPCODE_DTOF;
   case TGSI_OPCODE_DMUL:
      return VGPU10_OPCODE_DMUL;
   case TGSI_OPCODE_DADD:
      return VGPU10_OPCODE_DADD;
   case TGSI_OPCODE_DMAX:
      return VGPU10_OPCODE_DMAX;
   case TGSI_OPCODE_DMIN:
      return VGPU10_OPCODE_DMIN;
   case TGSI_OPCODE_DSEQ:
      return VGPU10_OPCODE_DEQ;
   case TGSI_OPCODE_DSGE:
      return VGPU10_OPCODE_DGE;
   case TGSI_OPCODE_DSLT:
      return VGPU10_OPCODE_DLT;
   case TGSI_OPCODE_DSNE:
      return VGPU10_OPCODE_DNE;
   case TGSI_OPCODE_IBFE:
      return VGPU10_OPCODE_IBFE;
   case TGSI_OPCODE_UBFE:
      return VGPU10_OPCODE_UBFE;
   case TGSI_OPCODE_BFI:
      return VGPU10_OPCODE_BFI;
   case TGSI_OPCODE_BREV:
      return VGPU10_OPCODE_BFREV;
   case TGSI_OPCODE_POPC:
      return VGPU10_OPCODE_COUNTBITS;
   case TGSI_OPCODE_LSB:
      return VGPU10_OPCODE_FIRSTBIT_LO;
   case TGSI_OPCODE_IMSB:
      return VGPU10_OPCODE_FIRSTBIT_SHI;
   case TGSI_OPCODE_UMSB:
      return VGPU10_OPCODE_FIRSTBIT_HI;
   case TGSI_OPCODE_INTERP_CENTROID:
      return VGPU10_OPCODE_EVAL_CENTROID;
   case TGSI_OPCODE_INTERP_SAMPLE:
      return VGPU10_OPCODE_EVAL_SAMPLE_INDEX;
   case TGSI_OPCODE_BARRIER:
      return VGPU10_OPCODE_SYNC;
   case TGSI_OPCODE_DFMA:
      return VGPU10_OPCODE_DFMA;
   case TGSI_OPCODE_FMA:
      return VGPU10_OPCODE_MAD;

   /* DX11.1 Opcodes */
   case TGSI_OPCODE_DDIV:
      return VGPU10_OPCODE_DDIV;
   case TGSI_OPCODE_DRCP:
      return VGPU10_OPCODE_DRCP;
   case TGSI_OPCODE_D2I:
      return VGPU10_OPCODE_DTOI;
   case TGSI_OPCODE_D2U:
      return VGPU10_OPCODE_DTOU;
   case TGSI_OPCODE_I2D:
      return VGPU10_OPCODE_ITOD;
   case TGSI_OPCODE_U2D:
      return VGPU10_OPCODE_UTOD;

   case TGSI_OPCODE_SAMPLE_POS:
      /* Note: we never actually get this opcode because there's no GLSL
       * function to query multisample resource sample positions.  There's
       * only the TGSI_SEMANTIC_SAMPLEPOS system value which contains the
       * position of the current sample in the render target.
       */
      FALLTHROUGH;
   case TGSI_OPCODE_SAMPLE_INFO:
      /* NOTE: we never actually get this opcode because the GLSL compiler
       * implements the gl_NumSamples variable with a simple constant in the
       * constant buffer.
       */
      FALLTHROUGH;
   default:
      assert(!"Unexpected TGSI opcode in translate_opcode()");
      return VGPU10_OPCODE_NOP;
   }
}


/**
 * Translate a TGSI register file type into a VGPU10 operand type.
 * \param array  is the TGSI_FILE_TEMPORARY register an array?
 */
static VGPU10_OPERAND_TYPE
translate_register_file(enum tgsi_file_type file, bool array)
{
   switch (file) {
   case TGSI_FILE_CONSTANT:
      return VGPU10_OPERAND_TYPE_CONSTANT_BUFFER;
   case TGSI_FILE_INPUT:
      return VGPU10_OPERAND_TYPE_INPUT;
   case TGSI_FILE_OUTPUT:
      return VGPU10_OPERAND_TYPE_OUTPUT;
   case TGSI_FILE_TEMPORARY:
      return array ? VGPU10_OPERAND_TYPE_INDEXABLE_TEMP
                   : VGPU10_OPERAND_TYPE_TEMP;
   case TGSI_FILE_IMMEDIATE:
      /* all immediates are 32-bit values at this time so
       * VGPU10_OPERAND_TYPE_IMMEDIATE64 is not possible at this time.
       */
      return VGPU10_OPERAND_TYPE_IMMEDIATE_CONSTANT_BUFFER;
   case TGSI_FILE_SAMPLER:
      return VGPU10_OPERAND_TYPE_SAMPLER;
   case TGSI_FILE_SYSTEM_VALUE:
      return VGPU10_OPERAND_TYPE_INPUT;

   /* XXX TODO more cases to finish */

   default:
      assert(!"Bad tgsi register file!");
      return VGPU10_OPERAND_TYPE_NULL;
   }
}


/**
 * Emit a null dst register
 */
static void
emit_null_dst_register(struct svga_shader_emitter_v10 *emit)
{
   VGPU10OperandToken0 operand;

   operand.value = 0;
   operand.operandType = VGPU10_OPERAND_TYPE_NULL;
   operand.numComponents = VGPU10_OPERAND_0_COMPONENT;

   emit_dword(emit, operand.value);
}


/**
 * If the given register is a temporary, return the array ID.
 * Else return zero.
 */
static unsigned
get_temp_array_id(const struct svga_shader_emitter_v10 *emit,
                  enum tgsi_file_type file, unsigned index)
{
   if (file == TGSI_FILE_TEMPORARY) {
      return emit->temp_map[index].arrayId;
   }
   else {
      return 0;
   }
}


/**
 * If the given register is a temporary, convert the index from a TGSI
 * TEMPORARY index to a VGPU10 temp index.
 */
static unsigned
remap_temp_index(const struct svga_shader_emitter_v10 *emit,
                 enum tgsi_file_type file, unsigned index)
{
   if (file == TGSI_FILE_TEMPORARY) {
      return emit->temp_map[index].index;
   }
   else {
      return index;
   }
}


/**
 * Setup the operand0 fields related to indexing (1D, 2D, relative, etc).
 * Note: the operandType field must already be initialized.
 * \param file  the register file being accessed
 * \param indirect  using indirect addressing of the register file?
 * \param index2D  if true, 2-D indexing is being used (const or temp registers)
 * \param indirect2D  if true, 2-D indirect indexing being used (for const buf)
 */
static VGPU10OperandToken0
setup_operand0_indexing(struct svga_shader_emitter_v10 *emit,
                        VGPU10OperandToken0 operand0,
                        enum tgsi_file_type file,
                        bool indirect,
                        bool index2D, bool indirect2D)
{
   VGPU10_OPERAND_INDEX_REPRESENTATION index0Rep, index1Rep;
   VGPU10_OPERAND_INDEX_DIMENSION indexDim;

   /*
    * Compute index dimensions
    */
   if (operand0.operandType == VGPU10_OPERAND_TYPE_IMMEDIATE32 ||
       operand0.operandType == VGPU10_OPERAND_TYPE_INPUT_PRIMITIVEID ||
       operand0.operandType == VGPU10_OPERAND_TYPE_INPUT_GS_INSTANCE_ID ||
       operand0.operandType == VGPU10_OPERAND_TYPE_INPUT_THREAD_ID ||
       operand0.operandType == VGPU10_OPERAND_TYPE_INPUT_THREAD_ID_IN_GROUP ||
       operand0.operandType == VGPU10_OPERAND_TYPE_OUTPUT_CONTROL_POINT_ID) {
      /* there's no swizzle for in-line immediates */
      indexDim = VGPU10_OPERAND_INDEX_0D;
      assert(operand0.selectionMode == 0);
   }
   else if (operand0.operandType == VGPU10_OPERAND_TYPE_INPUT_DOMAIN_POINT) {
      indexDim = VGPU10_OPERAND_INDEX_0D;
   }
   else {
      indexDim = index2D ? VGPU10_OPERAND_INDEX_2D : VGPU10_OPERAND_INDEX_1D;
   }

   /*
    * Compute index representation(s) (immediate vs relative).
    */
   if (indexDim == VGPU10_OPERAND_INDEX_2D) {
      index0Rep = indirect2D ? VGPU10_OPERAND_INDEX_IMMEDIATE32_PLUS_RELATIVE
         : VGPU10_OPERAND_INDEX_IMMEDIATE32;

      index1Rep = indirect ? VGPU10_OPERAND_INDEX_IMMEDIATE32_PLUS_RELATIVE
         : VGPU10_OPERAND_INDEX_IMMEDIATE32;
   }
   else if (indexDim == VGPU10_OPERAND_INDEX_1D) {
      index0Rep = indirect ? VGPU10_OPERAND_INDEX_IMMEDIATE32_PLUS_RELATIVE
         : VGPU10_OPERAND_INDEX_IMMEDIATE32;

      index1Rep = 0;
   }
   else {
      index0Rep = 0;
      index1Rep = 0;
   }

   operand0.indexDimension = indexDim;
   operand0.index0Representation = index0Rep;
   operand0.index1Representation = index1Rep;

   return operand0;
}


/**
 * Emit the operand for expressing an address register for indirect indexing.
 * Note that the address register is really just a temp register.
 * \param addr_reg_index  which address register to use
 */
static void
emit_indirect_register(struct svga_shader_emitter_v10 *emit,
                       unsigned addr_reg_index)
{
   unsigned tmp_reg_index;
   VGPU10OperandToken0 operand0;

   assert(addr_reg_index < MAX_VGPU10_ADDR_REGS);

   tmp_reg_index = emit->address_reg_index[addr_reg_index];

   /* operand0 is a simple temporary register, selecting one component */
   operand0.value = 0;
   operand0.operandType = VGPU10_OPERAND_TYPE_TEMP;
   operand0.numComponents = VGPU10_OPERAND_4_COMPONENT;
   operand0.indexDimension = VGPU10_OPERAND_INDEX_1D;
   operand0.index0Representation = VGPU10_OPERAND_INDEX_IMMEDIATE32;
   operand0.selectionMode = VGPU10_OPERAND_4_COMPONENT_SELECT_1_MODE;
   operand0.swizzleX = 0;
   operand0.swizzleY = 1;
   operand0.swizzleZ = 2;
   operand0.swizzleW = 3;

   emit_dword(emit, operand0.value);
   emit_dword(emit, remap_temp_index(emit, TGSI_FILE_TEMPORARY, tmp_reg_index));
}


/**
 * Translate the dst register of a TGSI instruction and emit VGPU10 tokens.
 * \param emit  the emitter context
 * \param reg  the TGSI dst register to translate
 */
static void
emit_dst_register(struct svga_shader_emitter_v10 *emit,
                  const struct tgsi_full_dst_register *reg)
{
   enum tgsi_file_type file = reg->Register.File;
   unsigned index = reg->Register.Index;
   const enum tgsi_semantic sem_name = emit->info.output_semantic_name[index];
   const unsigned sem_index = emit->info.output_semantic_index[index];
   unsigned writemask = reg->Register.WriteMask;
   const bool indirect = reg->Register.Indirect;
   unsigned tempArrayId = get_temp_array_id(emit, file, index);
   bool index2d = reg->Register.Dimension || tempArrayId > 0;
   VGPU10OperandToken0 operand0;

   if (file == TGSI_FILE_TEMPORARY) {
      emit->temp_map[index].initialized = true;
   }

   if (file == TGSI_FILE_OUTPUT) {
      if (emit->unit == PIPE_SHADER_VERTEX ||
          emit->unit == PIPE_SHADER_GEOMETRY ||
          emit->unit == PIPE_SHADER_TESS_EVAL) {
         if (index == emit->vposition.out_index &&
             emit->vposition.tmp_index != INVALID_INDEX) {
            /* replace OUTPUT[POS] with TEMP[POS].  We need to store the
             * vertex position result in a temporary so that we can modify
             * it in the post_helper() code.
             */
            file = TGSI_FILE_TEMPORARY;
            index = emit->vposition.tmp_index;
         }
         else if (sem_name == TGSI_SEMANTIC_CLIPDIST &&
                  emit->clip_dist_tmp_index != INVALID_INDEX) {
            /* replace OUTPUT[CLIPDIST] with TEMP[CLIPDIST].
             * We store the clip distance in a temporary first, then
             * we'll copy it to the shadow copy and to CLIPDIST with the
             * enabled planes mask in emit_clip_distance_instructions().
             */
            file = TGSI_FILE_TEMPORARY;
            index = emit->clip_dist_tmp_index + sem_index;
         }
         else if (sem_name == TGSI_SEMANTIC_CLIPVERTEX &&
                  emit->clip_vertex_tmp_index != INVALID_INDEX) {
            /* replace the CLIPVERTEX output register with a temporary */
            assert(emit->clip_mode == CLIP_VERTEX);
            assert(sem_index == 0);
            file = TGSI_FILE_TEMPORARY;
            index = emit->clip_vertex_tmp_index;
         }
         else if (sem_name == TGSI_SEMANTIC_COLOR &&
                  emit->key.clamp_vertex_color) {

            /* set the saturate modifier of the instruction
             * to clamp the vertex color.
             */
            VGPU10OpcodeToken0 *token =
               (VGPU10OpcodeToken0 *)emit->buf + emit->inst_start_token;
            token->saturate = true;
         }
         else if (sem_name == TGSI_SEMANTIC_VIEWPORT_INDEX &&
                  emit->gs.viewport_index_out_index != INVALID_INDEX) {
            file = TGSI_FILE_TEMPORARY;
            index = emit->gs.viewport_index_tmp_index;
         }
      }
      else if (emit->unit == PIPE_SHADER_FRAGMENT) {
         if (sem_name == TGSI_SEMANTIC_POSITION) {
            /* Fragment depth output register */
            operand0.value = 0;
            operand0.operandType = VGPU10_OPERAND_TYPE_OUTPUT_DEPTH;
            operand0.indexDimension = VGPU10_OPERAND_INDEX_0D;
            operand0.numComponents = VGPU10_OPERAND_1_COMPONENT;
            emit_dword(emit, operand0.value);
            return;
         }
         else if (sem_name == TGSI_SEMANTIC_SAMPLEMASK) {
            /* Fragment sample mask output */
            operand0.value = 0;
            operand0.operandType = VGPU10_OPERAND_TYPE_OUTPUT_COVERAGE_MASK;
            operand0.indexDimension = VGPU10_OPERAND_INDEX_0D;
            operand0.numComponents = VGPU10_OPERAND_1_COMPONENT;
            emit_dword(emit, operand0.value);
            return;
         }
         else if (index == emit->fs.color_out_index[0] &&
             emit->fs.color_tmp_index != INVALID_INDEX) {
            /* replace OUTPUT[COLOR] with TEMP[COLOR].  We need to store the
             * fragment color result in a temporary so that we can read it
             * it in the post_helper() code.
             */
            file = TGSI_FILE_TEMPORARY;
            index = emit->fs.color_tmp_index;
         }
         else {
            /* Typically, for fragment shaders, the output register index
             * matches the color semantic index.  But not when we write to
             * the fragment depth register.  In that case, OUT[0] will be
             * fragdepth and OUT[1] will be the 0th color output.  We need
             * to use the semantic index for color outputs.
             */
            assert(sem_name == TGSI_SEMANTIC_COLOR);
            index = emit->info.output_semantic_index[index];

            emit->num_output_writes++;
         }
      }
      else if (emit->unit == PIPE_SHADER_TESS_CTRL) {
         if (index == emit->tcs.inner.tgsi_index) {
            /* replace OUTPUT[TESSLEVEL] with temp. We are storing it
             * in temporary for now so that will be store into appropriate
             * registers in post_helper() in patch constant phase.
             */
            if (emit->tcs.control_point_phase) {
               /* Discard writing into tessfactor in control point phase */
               emit->discard_instruction =  true;
            }
            else {
               file = TGSI_FILE_TEMPORARY;
               index = emit->tcs.inner.temp_index;
            }
         }
         else if (index == emit->tcs.outer.tgsi_index) {
            /* replace OUTPUT[TESSLEVEL] with temp. We are storing it
             * in temporary for now so that will be store into appropriate
             * registers in post_helper().
             */
            if (emit->tcs.control_point_phase) {
               /* Discard writing into tessfactor in control point phase */
               emit->discard_instruction =  true;
            }
            else {
               file = TGSI_FILE_TEMPORARY;
               index = emit->tcs.outer.temp_index;
            }
         }
         else if (index >= emit->tcs.patch_generic_out_index &&
                  index < (emit->tcs.patch_generic_out_index +
                          emit->tcs.patch_generic_out_count)) {
            if (emit->tcs.control_point_phase) {
               /* Discard writing into generic patch constant outputs in
                  control point phase */
               emit->discard_instruction =  true;
            }
            else {
               if (emit->reemit_instruction) {
                  /* Store results of reemitted instruction in temporary register. */
                  file = TGSI_FILE_TEMPORARY;
                  index = emit->tcs.patch_generic_tmp_index +
                          (index - emit->tcs.patch_generic_out_index);
                  /**
                   * Temporaries for patch constant data can be done
                   * as indexable temporaries.
                   */
                  tempArrayId = get_temp_array_id(emit, file, index);
                  index2d = tempArrayId > 0;

                  emit->reemit_instruction = false;
               }
               else {
                  /* If per-patch outputs is been read in shader, we
                   * reemit instruction and store results in temporaries in
                   * patch constant phase. */
                  if (emit->info.reads_perpatch_outputs) {
                     emit->reemit_instruction = true;
                  }
               }
            }
         }
         else if (reg->Register.Dimension) {
            /* Only control point outputs are declared 2D in tgsi */
            if (emit->tcs.control_point_phase) {
               if (emit->reemit_instruction) {
                  /* Store results of reemitted instruction in temporary register. */
                  index2d = false;
                  file = TGSI_FILE_TEMPORARY;
                  index = emit->tcs.control_point_tmp_index +
                          (index - emit->tcs.control_point_out_index);
                  emit->reemit_instruction = false;
               }
               else {
                  /* The mapped control point outputs are 1-D */
                  index2d = false;
                  if (emit->info.reads_pervertex_outputs) {
                     /* If per-vertex outputs is been read in shader, we
                      * reemit instruction and store results in temporaries
                      * control point phase. */
                     emit->reemit_instruction = true;
                  }
               }

               if (sem_name == TGSI_SEMANTIC_CLIPDIST &&
                   emit->clip_dist_tmp_index != INVALID_INDEX) {
                  /* replace OUTPUT[CLIPDIST] with TEMP[CLIPDIST].
                   * We store the clip distance in a temporary first, then
                   * we'll copy it to the shadow copy and to CLIPDIST with the
                   * enabled planes mask in emit_clip_distance_instructions().
                   */
                  file = TGSI_FILE_TEMPORARY;
                  index = emit->clip_dist_tmp_index + sem_index;
               }
               else if (sem_name == TGSI_SEMANTIC_CLIPVERTEX &&
                        emit->clip_vertex_tmp_index != INVALID_INDEX) {
                  /* replace the CLIPVERTEX output register with a temporary */
                  assert(emit->clip_mode == CLIP_VERTEX);
                  assert(sem_index == 0);
                  file = TGSI_FILE_TEMPORARY;
                  index = emit->clip_vertex_tmp_index;
               }
            }
            else {
               /* Discard writing into control point outputs in
                  patch constant phase */
               emit->discard_instruction =  true;
            }
         }
      }
   }

   /* init operand tokens to all zero */
   operand0.value = 0;

   operand0.numComponents = VGPU10_OPERAND_4_COMPONENT;

   /* the operand has a writemask */
   operand0.selectionMode = VGPU10_OPERAND_4_COMPONENT_MASK_MODE;

   /* Which of the four dest components to write to. Note that we can use a
    * simple assignment here since TGSI writemasks match VGPU10 writemasks.
    */
   STATIC_ASSERT(TGSI_WRITEMASK_X == VGPU10_OPERAND_4_COMPONENT_MASK_X);
   operand0.mask = writemask;

   /* translate TGSI register file type to VGPU10 operand type */
   operand0.operandType = translate_register_file(file, tempArrayId > 0);

   check_register_index(emit, operand0.operandType, index);

   operand0 = setup_operand0_indexing(emit, operand0, file, indirect,
                                      index2d, false);

   /* Emit tokens */
   emit_dword(emit, operand0.value);
   if (tempArrayId > 0) {
      emit_dword(emit, tempArrayId);
   }

   emit_dword(emit, remap_temp_index(emit, file, index));

   if (indirect) {
      emit_indirect_register(emit, reg->Indirect.Index);
   }
}


/**
 * Check if temporary register needs to be initialize when
 * shader is not using indirect addressing for temporary and uninitialized
 * temporary is not used in loop. In these two scenarios, we cannot
 * determine if temporary is initialized or not.
 */
static bool
need_temp_reg_initialization(struct svga_shader_emitter_v10 *emit,
                             unsigned index)
{
   if (!(emit->info.indirect_files & (1u << TGSI_FILE_TEMPORARY))
       && emit->current_loop_depth == 0) {
      if (!emit->temp_map[index].initialized &&
          emit->temp_map[index].index < emit->num_shader_temps) {
         return true;
      }
   }

   return false;
}


/**
 * Translate a src register of a TGSI instruction and emit VGPU10 tokens.
 * In quite a few cases, we do register substitution.  For example, if
 * the TGSI register is the front/back-face register, we replace that with
 * a temp register containing a value we computed earlier.
 */
static void
emit_src_register(struct svga_shader_emitter_v10 *emit,
                  const struct tgsi_full_src_register *reg)
{
   enum tgsi_file_type file = reg->Register.File;
   unsigned index = reg->Register.Index;
   bool indirect = reg->Register.Indirect;
   unsigned tempArrayId = get_temp_array_id(emit, file, index);
   bool index2d = (reg->Register.Dimension ||
                            tempArrayId > 0 ||
                            file == TGSI_FILE_CONSTANT);
   unsigned index2 = tempArrayId > 0 ? tempArrayId : reg->Dimension.Index;
   bool indirect2d = reg->Dimension.Indirect;
   unsigned swizzleX = reg->Register.SwizzleX;
   unsigned swizzleY = reg->Register.SwizzleY;
   unsigned swizzleZ = reg->Register.SwizzleZ;
   unsigned swizzleW = reg->Register.SwizzleW;
   const bool absolute = reg->Register.Absolute;
   const bool negate = reg->Register.Negate;
   VGPU10OperandToken0 operand0;
   VGPU10OperandToken1 operand1;

   operand0.value = operand1.value = 0;

   if (emit->unit == PIPE_SHADER_FRAGMENT){
      if (file == TGSI_FILE_INPUT) {
         if (index == emit->fs.face_input_index) {
            /* Replace INPUT[FACE] with TEMP[FACE] */
            file = TGSI_FILE_TEMPORARY;
            index = emit->fs.face_tmp_index;
         }
         else if (index == emit->fs.fragcoord_input_index) {
            /* Replace INPUT[POSITION] with TEMP[POSITION] */
            file = TGSI_FILE_TEMPORARY;
            index = emit->fs.fragcoord_tmp_index;
         }
         else if (index == emit->fs.layer_input_index) {
            /* Replace INPUT[LAYER] with zero.x */
            file = TGSI_FILE_IMMEDIATE;
            index = emit->fs.layer_imm_index;
            swizzleX = swizzleY = swizzleZ = swizzleW = TGSI_SWIZZLE_X;
         }
         else {
            /* We remap fragment shader inputs to that FS input indexes
             * match up with VS/GS output indexes.
             */
            index = emit->linkage.input_map[index];
         }
      }
      else if (file == TGSI_FILE_SYSTEM_VALUE) {
         if (index == emit->fs.sample_pos_sys_index) {
            assert(emit->version >= 41);
            /* Current sample position is in a temp register */
            file = TGSI_FILE_TEMPORARY;
            index = emit->fs.sample_pos_tmp_index;
         }
         else if (index == emit->fs.sample_mask_in_sys_index) {
            /* Emitted as vCoverage0.x */
            /* According to GLSL spec, the gl_SampleMaskIn array has ceil(s / 32)
             * elements where s is the maximum number of color samples supported
             * by the implementation.
             */
            operand0.value = 0;
            operand0.operandType = VGPU10_OPERAND_TYPE_INPUT_COVERAGE_MASK;
            operand0.indexDimension = VGPU10_OPERAND_INDEX_0D;
            operand0.numComponents = VGPU10_OPERAND_4_COMPONENT;
            operand0.selectionMode = VGPU10_OPERAND_4_COMPONENT_SELECT_1_MODE;
            emit_dword(emit, operand0.value);
            return;
         }
         else {
            /* Map the TGSI system value to a VGPU10 input register */
            assert(index < ARRAY_SIZE(emit->system_value_indexes));
            file = TGSI_FILE_INPUT;
            index = emit->system_value_indexes[index];
         }
      }
   }
   else if (emit->unit == PIPE_SHADER_GEOMETRY) {
      if (file == TGSI_FILE_INPUT) {
         if (index == emit->gs.prim_id_index) {
            operand0.numComponents = VGPU10_OPERAND_0_COMPONENT;
            operand0.operandType = VGPU10_OPERAND_TYPE_INPUT_PRIMITIVEID;
         }
         index = emit->linkage.input_map[index];
      }
      else if (file == TGSI_FILE_SYSTEM_VALUE &&
               index == emit->gs.invocation_id_sys_index) {
         /* Emitted as vGSInstanceID0.x */
         operand0.numComponents = VGPU10_OPERAND_4_COMPONENT;
         operand0.operandType = VGPU10_OPERAND_TYPE_INPUT_GS_INSTANCE_ID;
         index = 0;
      }
   }
   else if (emit->unit == PIPE_SHADER_VERTEX) {
      if (file == TGSI_FILE_INPUT) {
         /* if input is adjusted... */
         if ((emit->key.vs.adjust_attrib_w_1 |
              emit->key.vs.adjust_attrib_itof |
              emit->key.vs.adjust_attrib_utof |
              emit->key.vs.attrib_is_bgra |
              emit->key.vs.attrib_puint_to_snorm |
              emit->key.vs.attrib_puint_to_uscaled |
              emit->key.vs.attrib_puint_to_sscaled) & (1 << index)) {
            file = TGSI_FILE_TEMPORARY;
            index = emit->vs.adjusted_input[index];
         }
      }
      else if (file == TGSI_FILE_SYSTEM_VALUE) {
         if (index == emit->vs.vertex_id_sys_index &&
             emit->vs.vertex_id_tmp_index != INVALID_INDEX) {
            file = TGSI_FILE_TEMPORARY;
            index = emit->vs.vertex_id_tmp_index;
            swizzleX = swizzleY = swizzleZ = swizzleW = TGSI_SWIZZLE_X;
         }
         else {
            /* Map the TGSI system value to a VGPU10 input register */
            assert(index < ARRAY_SIZE(emit->system_value_indexes));
            file = TGSI_FILE_INPUT;
            index = emit->system_value_indexes[index];
         }
      }
   }
   else if (emit->unit == PIPE_SHADER_TESS_CTRL) {

      if (file == TGSI_FILE_SYSTEM_VALUE) {
         if (index == emit->tcs.vertices_per_patch_index) {
            /**
             * if source register is the system value for vertices_per_patch,
             * replace it with the immediate.
             */
            file = TGSI_FILE_IMMEDIATE;
            index = emit->tcs.imm_index;
            swizzleX = swizzleY = swizzleZ = swizzleW = TGSI_SWIZZLE_X;
         }
         else if (index == emit->tcs.invocation_id_sys_index) {
            if (emit->tcs.control_point_phase) {
               /**
                * Emitted as vOutputControlPointID.x
                */
               operand0.numComponents = VGPU10_OPERAND_1_COMPONENT;
               operand0.operandType = VGPU10_OPERAND_TYPE_OUTPUT_CONTROL_POINT_ID;
               operand0.selectionMode = VGPU10_OPERAND_4_COMPONENT_MASK_MODE;
               operand0.mask = 0;
               emit_dword(emit, operand0.value);
               return;
            }
            else {
               /* There is no control point ID input declaration in
                * the patch constant phase in hull shader.
                * Since for now we are emitting all instructions in
                * the patch constant phase, we are replacing the
                * control point ID reference with the immediate 0.
                */
               file = TGSI_FILE_IMMEDIATE;
               index = emit->tcs.imm_index;
               swizzleX = swizzleY = swizzleZ = swizzleW = TGSI_SWIZZLE_W;
            }
         }
         else if (index == emit->tcs.prim_id_index) {
            /**
             * Emitted as vPrim.x
             */
            operand0.numComponents = VGPU10_OPERAND_1_COMPONENT;
            operand0.operandType = VGPU10_OPERAND_TYPE_INPUT_PRIMITIVEID;
            index = 0;
         }
      }
      else if (file == TGSI_FILE_INPUT) {
         index = emit->linkage.input_map[index];
         if (!emit->tcs.control_point_phase) {
            /* Emitted as vicp */
            operand0.numComponents = VGPU10_OPERAND_4_COMPONENT;
            operand0.operandType = VGPU10_OPERAND_TYPE_INPUT_CONTROL_POINT;
            assert(reg->Register.Dimension);
         }
      }
      else if (file == TGSI_FILE_OUTPUT) {
         if ((index >= emit->tcs.patch_generic_out_index &&
             index < (emit->tcs.patch_generic_out_index +
                      emit->tcs.patch_generic_out_count)) ||
             index == emit->tcs.inner.tgsi_index ||
             index == emit->tcs.outer.tgsi_index) {
            if (emit->tcs.control_point_phase) {
               emit->discard_instruction = true;
            }
            else {
               /* Device doesn't allow reading from output so
                * use corresponding temporary register as source */
               file = TGSI_FILE_TEMPORARY;
               if (index == emit->tcs.inner.tgsi_index) {
                  index = emit->tcs.inner.temp_index;
               }
               else if (index == emit->tcs.outer.tgsi_index) {
                  index = emit->tcs.outer.temp_index;
               }
               else {
                  index = emit->tcs.patch_generic_tmp_index +
                          (index - emit->tcs.patch_generic_out_index);
               }

               /**
                * Temporaries for patch constant data can be done
                * as indexable temporaries.
                */
               tempArrayId = get_temp_array_id(emit, file, index);
               index2d = tempArrayId > 0;
               index2 = tempArrayId > 0 ? tempArrayId : reg->Dimension.Index;
            }
         }
         else if (index2d) {
            if (emit->tcs.control_point_phase) {
               /* Device doesn't allow reading from output so
                * use corresponding temporary register as source */
               file = TGSI_FILE_TEMPORARY;
               index2d = false;
               index = emit->tcs.control_point_tmp_index +
                       (index - emit->tcs.control_point_out_index);
            }
            else {
               emit->discard_instruction = true;
            }
         }
      }
   }
   else if (emit->unit == PIPE_SHADER_TESS_EVAL) {
      if (file == TGSI_FILE_SYSTEM_VALUE) {
         if (index == emit->tes.tesscoord_sys_index) {
            /**
             * Emitted as vDomain
             */
            operand0.numComponents = VGPU10_OPERAND_4_COMPONENT;
            operand0.operandType = VGPU10_OPERAND_TYPE_INPUT_DOMAIN_POINT;
            index = 0;

            /* Make sure swizzles are of those components allowed according
             * to the tessellator domain.
             */
            swizzleX = MIN2(swizzleX, emit->tes.swizzle_max);
            swizzleY = MIN2(swizzleY, emit->tes.swizzle_max);
            swizzleZ = MIN2(swizzleZ, emit->tes.swizzle_max);
            swizzleW = MIN2(swizzleW, emit->tes.swizzle_max);
         }
         else if (index == emit->tes.inner.tgsi_index) {
            file = TGSI_FILE_TEMPORARY;
            index = emit->tes.inner.temp_index;
         }
         else if (index == emit->tes.outer.tgsi_index) {
            file = TGSI_FILE_TEMPORARY;
            index = emit->tes.outer.temp_index;
         }
         else if (index == emit->tes.prim_id_index) {
            /**
             * Emitted as vPrim.x
             */
            operand0.numComponents = VGPU10_OPERAND_1_COMPONENT;
            operand0.operandType = VGPU10_OPERAND_TYPE_INPUT_PRIMITIVEID;
            index = 0;
         }

      }
      else if (file == TGSI_FILE_INPUT) {
         if (index2d) {
            /* 2D input is emitted as vcp (input control point). */
            operand0.operandType = VGPU10_OPERAND_TYPE_INPUT_CONTROL_POINT;
            operand0.numComponents = VGPU10_OPERAND_4_COMPONENT;

            /* index specifies the element index and is remapped
             * to align with the tcs output index.
             */
            index = emit->linkage.input_map[index];

            assert(index2 < emit->key.tes.vertices_per_patch);
         }
         else {
            if (index < emit->key.tes.tessfactor_index)
               /* index specifies the generic patch index.
                * Remapped to match up with the tcs output index.
                */
               index = emit->linkage.input_map[index];

            operand0.operandType = VGPU10_OPERAND_TYPE_INPUT_PATCH_CONSTANT;
            operand0.numComponents = VGPU10_OPERAND_4_COMPONENT;
         }
      }
   }
   else if (emit->unit == PIPE_SHADER_COMPUTE) {
      if (file == TGSI_FILE_SYSTEM_VALUE) {
         if (index == emit->cs.thread_id_index) {
            operand0.numComponents = VGPU10_OPERAND_4_COMPONENT;
            operand0.operandType = VGPU10_OPERAND_TYPE_INPUT_THREAD_ID_IN_GROUP;
            index = 0;
         } else if (index == emit->cs.block_id_index) {
            operand0.value = 0;
            operand0.numComponents = VGPU10_OPERAND_4_COMPONENT;
            operand0.operandType = VGPU10_OPERAND_TYPE_INPUT_THREAD_GROUP_ID;
            operand0.indexDimension = VGPU10_OPERAND_INDEX_0D;
            operand0.selectionMode = VGPU10_OPERAND_4_COMPONENT_SWIZZLE_MODE;
            operand0.swizzleX = swizzleX;
            operand0.swizzleY = swizzleY;
            operand0.swizzleZ = swizzleZ;
            operand0.swizzleW = swizzleW;
            emit_dword(emit, operand0.value);
            return;
         } else if (index == emit->cs.grid_size.tgsi_index) {
            file = TGSI_FILE_IMMEDIATE;
            index = emit->cs.grid_size.imm_index;
         }
      }
   }

   if (file == TGSI_FILE_ADDRESS) {
      index = emit->address_reg_index[index];
      file = TGSI_FILE_TEMPORARY;
   }

   if (file == TGSI_FILE_CONSTANT) {
      /**
       * If this constant buffer is to be bound as srv raw buffer,
       * then we have to load the constant to a temp first before
       * it can be used as a source in the instruction.
       * This is accomplished in two passes. The first pass is to
       * identify if there is any constbuf to rawbuf translation.
       * If there isn't, emit the instruction as usual.
       * If there is, then we save the constant buffer reference info,
       * and then instead of emitting the instruction at the end
       * of the instruction, it will trigger a second pass of parsing
       * this instruction. Before it starts the parsing, it will
       * load the referenced raw buffer elements to temporaries.
       * Then it will emit the instruction that replaces the
       * constant buffer replaces with the corresponding temporaries.
       */
      if (emit->raw_bufs & (1 << index2)) {
         if (emit->reemit_rawbuf_instruction != REEMIT_IN_PROGRESS) {
            unsigned tmpIdx = emit->raw_buf_cur_tmp_index;

            emit->raw_buf_tmp[tmpIdx].buffer_index = index2;

            /* Save whether the element index is indirect indexing */
            emit->raw_buf_tmp[tmpIdx].indirect = indirect;

            /* If it is indirect index, save the temporary
             * address index, otherwise, save the immediate index.
             */
            if (indirect) {
               emit->raw_buf_tmp[tmpIdx].element_index =
                  emit->address_reg_index[reg->Indirect.Index];
               emit->raw_buf_tmp[tmpIdx].element_rel =
                  reg->Register.Index;
            }
            else {
               emit->raw_buf_tmp[tmpIdx].element_index = index;
               emit->raw_buf_tmp[tmpIdx].element_rel = 0;
            }

            emit->raw_buf_cur_tmp_index++;
            emit->reemit_rawbuf_instruction = REEMIT_TRUE;
            emit->discard_instruction = true;
            emit->reemit_tgsi_instruction = true;
         }
         else {
            /* In the reemitting process, replace the constant buffer
             * reference with temporary.
             */
            file = TGSI_FILE_TEMPORARY;
            index = emit->raw_buf_cur_tmp_index + emit->raw_buf_tmp_index;
            index2d = false;
            indirect = false;
            emit->raw_buf_cur_tmp_index++;
         }
      }
   }

   if (file == TGSI_FILE_TEMPORARY) {
      if (need_temp_reg_initialization(emit, index)) {
         emit->initialize_temp_index = index;
         emit->discard_instruction = true;
      }
   }

   if (operand0.value == 0) {
      /* if operand0 was not set above for a special case, do the general
       * case now.
       */
      operand0.numComponents = VGPU10_OPERAND_4_COMPONENT;
      operand0.operandType = translate_register_file(file, tempArrayId > 0);
   }
   operand0 = setup_operand0_indexing(emit, operand0, file, indirect,
                                      index2d, indirect2d);

   if (operand0.operandType != VGPU10_OPERAND_TYPE_IMMEDIATE32 &&
       operand0.operandType != VGPU10_OPERAND_TYPE_INPUT_PRIMITIVEID) {
      /* there's no swizzle for in-line immediates */
      if (swizzleX == swizzleY &&
          swizzleX == swizzleZ &&
          swizzleX == swizzleW) {
         operand0.selectionMode = VGPU10_OPERAND_4_COMPONENT_SELECT_1_MODE;
      }
      else {
         operand0.selectionMode = VGPU10_OPERAND_4_COMPONENT_SWIZZLE_MODE;
      }

      operand0.swizzleX = swizzleX;
      operand0.swizzleY = swizzleY;
      operand0.swizzleZ = swizzleZ;
      operand0.swizzleW = swizzleW;

      if (absolute || negate) {
         operand0.extended = 1;
         operand1.extendedOperandType = VGPU10_EXTENDED_OPERAND_MODIFIER;
         if (absolute && !negate)
            operand1.operandModifier = VGPU10_OPERAND_MODIFIER_ABS;
         if (!absolute && negate)
            operand1.operandModifier = VGPU10_OPERAND_MODIFIER_NEG;
         if (absolute && negate)
            operand1.operandModifier = VGPU10_OPERAND_MODIFIER_ABSNEG;
      }
   }

   check_register_index(emit, operand0.operandType, index);

   /* Emit the operand tokens */
   emit_dword(emit, operand0.value);
   if (operand0.extended)
      emit_dword(emit, operand1.value);

   if (operand0.operandType == VGPU10_OPERAND_TYPE_IMMEDIATE32) {
      /* Emit the four float/int in-line immediate values */
      unsigned *c;
      assert(index < ARRAY_SIZE(emit->immediates));
      assert(file == TGSI_FILE_IMMEDIATE);
      assert(swizzleX < 4);
      assert(swizzleY < 4);
      assert(swizzleZ < 4);
      assert(swizzleW < 4);
      c = (unsigned *) emit->immediates[index];
      emit_dword(emit, c[swizzleX]);
      emit_dword(emit, c[swizzleY]);
      emit_dword(emit, c[swizzleZ]);
      emit_dword(emit, c[swizzleW]);
   }
   else if (operand0.indexDimension >= VGPU10_OPERAND_INDEX_1D) {
      /* Emit the register index(es) */
      if (index2d) {
         emit_dword(emit, index2);

         if (indirect2d) {
            emit_indirect_register(emit, reg->DimIndirect.Index);
         }
      }

      emit_dword(emit, remap_temp_index(emit, file, index));

      if (indirect) {
         assert(operand0.operandType != VGPU10_OPERAND_TYPE_TEMP);
         emit_indirect_register(emit, reg->Indirect.Index);
      }
   }
}


/**
 * Emit a resource operand (for use with a SAMPLE instruction).
 */
static void
emit_resource_register(struct svga_shader_emitter_v10 *emit,
                       unsigned resource_number)
{
   VGPU10OperandToken0 operand0;

   check_register_index(emit, VGPU10_OPERAND_TYPE_RESOURCE, resource_number);

   /* init */
   operand0.value = 0;

   operand0.operandType = VGPU10_OPERAND_TYPE_RESOURCE;
   operand0.indexDimension = VGPU10_OPERAND_INDEX_1D;
   operand0.numComponents = VGPU10_OPERAND_4_COMPONENT;
   operand0.selectionMode = VGPU10_OPERAND_4_COMPONENT_SWIZZLE_MODE;
   operand0.swizzleX = VGPU10_COMPONENT_X;
   operand0.swizzleY = VGPU10_COMPONENT_Y;
   operand0.swizzleZ = VGPU10_COMPONENT_Z;
   operand0.swizzleW = VGPU10_COMPONENT_W;

   emit_dword(emit, operand0.value);
   emit_dword(emit, resource_number);
}


/**
 * Emit a sampler operand (for use with a SAMPLE instruction).
 */
static void
emit_sampler_register(struct svga_shader_emitter_v10 *emit,
                      unsigned unit)
{
   VGPU10OperandToken0 operand0;
   unsigned sampler_number;

   sampler_number = emit->key.tex[unit].sampler_index;

   if ((emit->shadow_compare_units & (1 << unit)) && emit->use_sampler_state_mapping)
      sampler_number++;

   check_register_index(emit, VGPU10_OPERAND_TYPE_SAMPLER, sampler_number);

   /* init */
   operand0.value = 0;

   operand0.operandType = VGPU10_OPERAND_TYPE_SAMPLER;
   operand0.indexDimension = VGPU10_OPERAND_INDEX_1D;

   emit_dword(emit, operand0.value);
   emit_dword(emit, sampler_number);
}


/**
 * Emit an operand which reads the IS_FRONT_FACING register.
 */
static void
emit_face_register(struct svga_shader_emitter_v10 *emit)
{
   VGPU10OperandToken0 operand0;
   unsigned index = emit->linkage.input_map[emit->fs.face_input_index];

   /* init */
   operand0.value = 0;

   operand0.operandType = VGPU10_OPERAND_TYPE_INPUT;
   operand0.indexDimension = VGPU10_OPERAND_INDEX_1D;
   operand0.selectionMode = VGPU10_OPERAND_4_COMPONENT_SELECT_1_MODE;
   operand0.numComponents = VGPU10_OPERAND_4_COMPONENT;

   operand0.swizzleX = VGPU10_COMPONENT_X;
   operand0.swizzleY = VGPU10_COMPONENT_X;
   operand0.swizzleZ = VGPU10_COMPONENT_X;
   operand0.swizzleW = VGPU10_COMPONENT_X;

   emit_dword(emit, operand0.value);
   emit_dword(emit, index);
}


/**
 * Emit tokens for the "rasterizer" register used by the SAMPLE_POS
 * instruction.
 */
static void
emit_rasterizer_register(struct svga_shader_emitter_v10 *emit)
{
   VGPU10OperandToken0 operand0;

   /* init */
   operand0.value = 0;

   /* No register index for rasterizer index (there's only one) */
   operand0.operandType = VGPU10_OPERAND_TYPE_RASTERIZER;
   operand0.indexDimension = VGPU10_OPERAND_INDEX_0D;
   operand0.numComponents = VGPU10_OPERAND_4_COMPONENT;
   operand0.selectionMode = VGPU10_OPERAND_4_COMPONENT_SWIZZLE_MODE;
   operand0.swizzleX = VGPU10_COMPONENT_X;
   operand0.swizzleY = VGPU10_COMPONENT_Y;
   operand0.swizzleZ = VGPU10_COMPONENT_Z;
   operand0.swizzleW = VGPU10_COMPONENT_W;

   emit_dword(emit, operand0.value);
}


/**
 * Emit tokens for the "stream" register used by the 
 * DCL_STREAM, CUT_STREAM, EMIT_STREAM instructions.
 */
static void
emit_stream_register(struct svga_shader_emitter_v10 *emit, unsigned index)
{
   VGPU10OperandToken0 operand0;

   /* init */
   operand0.value = 0;

   /* No register index for rasterizer index (there's only one) */
   operand0.operandType = VGPU10_OPERAND_TYPE_STREAM;
   operand0.indexDimension = VGPU10_OPERAND_INDEX_1D;
   operand0.numComponents = VGPU10_OPERAND_0_COMPONENT;

   emit_dword(emit, operand0.value);
   emit_dword(emit, index);
}


/**
 * Emit the token for a VGPU10 opcode, with precise parameter.
 * \param saturate   clamp result to [0,1]?
 */
static void
emit_opcode_precise(struct svga_shader_emitter_v10 *emit,
                    unsigned vgpu10_opcode, bool saturate, bool precise)
{
   VGPU10OpcodeToken0 token0;

   token0.value = 0;  /* init all fields to zero */
   token0.opcodeType = vgpu10_opcode;
   token0.instructionLength = 0; /* Filled in by end_emit_instruction() */
   token0.saturate = saturate;

   /* Mesa's GLSL IR -> TGSI translator will set the TGSI precise flag for
    * 'invariant' declarations.  Only set preciseValues=1 if we have SM5.
    */
   token0.preciseValues = precise && emit->version >= 50;

   emit_dword(emit, token0.value);

   emit->uses_precise_qualifier |= token0.preciseValues;
}


/**
 * Emit the token for a VGPU10 opcode.
 * \param saturate   clamp result to [0,1]?
 */
static void
emit_opcode(struct svga_shader_emitter_v10 *emit,
            unsigned vgpu10_opcode, bool saturate)
{
   emit_opcode_precise(emit, vgpu10_opcode, saturate, false);
}


/**
 * Emit the token for a VGPU10 resinfo instruction.
 * \param modifier   return type modifier, _uint or _rcpFloat.
 *                   TODO: We may want to remove this parameter if it will
 *                   only ever be used as _uint.
 */
static void
emit_opcode_resinfo(struct svga_shader_emitter_v10 *emit,
                    VGPU10_RESINFO_RETURN_TYPE modifier)
{
   VGPU10OpcodeToken0 token0;

   token0.value = 0;  /* init all fields to zero */
   token0.opcodeType = VGPU10_OPCODE_RESINFO;
   token0.instructionLength = 0; /* Filled in by end_emit_instruction() */
   token0.resinfoReturnType = modifier;

   emit_dword(emit, token0.value);
}


/**
 * Emit opcode tokens for a texture sample instruction.  Texture instructions
 * can be rather complicated (texel offsets, etc) so we have this specialized
 * function.
 */
static void
emit_sample_opcode(struct svga_shader_emitter_v10 *emit,
                   unsigned vgpu10_opcode, bool saturate,
                   const int offsets[3])
{
   VGPU10OpcodeToken0 token0;
   VGPU10OpcodeToken1 token1;

   token0.value = 0;  /* init all fields to zero */
   token0.opcodeType = vgpu10_opcode;
   token0.instructionLength = 0; /* Filled in by end_emit_instruction() */
   token0.saturate = saturate;

   if (offsets[0] || offsets[1] || offsets[2]) {
      assert(offsets[0] >= VGPU10_MIN_TEXEL_FETCH_OFFSET);
      assert(offsets[1] >= VGPU10_MIN_TEXEL_FETCH_OFFSET);
      assert(offsets[2] >= VGPU10_MIN_TEXEL_FETCH_OFFSET);
      assert(offsets[0] <= VGPU10_MAX_TEXEL_FETCH_OFFSET);
      assert(offsets[1] <= VGPU10_MAX_TEXEL_FETCH_OFFSET);
      assert(offsets[2] <= VGPU10_MAX_TEXEL_FETCH_OFFSET);

      token0.extended = 1;
      token1.value = 0;
      token1.opcodeType = VGPU10_EXTENDED_OPCODE_SAMPLE_CONTROLS;
      token1.offsetU = offsets[0];
      token1.offsetV = offsets[1];
      token1.offsetW = offsets[2];
   }

   emit_dword(emit, token0.value);
   if (token0.extended) {
      emit_dword(emit, token1.value);
   }
}


/**
 * Emit a DISCARD opcode token.
 * If nonzero is set, we'll discard the fragment if the X component is not 0.
 * Otherwise, we'll discard the fragment if the X component is 0.
 */
static void
emit_discard_opcode(struct svga_shader_emitter_v10 *emit, bool nonzero)
{
   VGPU10OpcodeToken0 opcode0;

   opcode0.value = 0;
   opcode0.opcodeType = VGPU10_OPCODE_DISCARD;
   if (nonzero)
      opcode0.testBoolean = VGPU10_INSTRUCTION_TEST_NONZERO;

   emit_dword(emit, opcode0.value);
}


/**
 * We need to call this before we begin emitting a VGPU10 instruction.
 */
static void
begin_emit_instruction(struct svga_shader_emitter_v10 *emit)
{
   assert(emit->inst_start_token == 0);
   /* Save location of the instruction's VGPU10OpcodeToken0 token.
    * Note, we can't save a pointer because it would become invalid if
    * we have to realloc the output buffer.
    */
   emit->inst_start_token = emit_get_num_tokens(emit);
}


/**
 * We need to call this after we emit the last token of a VGPU10 instruction.
 * This function patches in the opcode token's instructionLength field.
 */
static void
end_emit_instruction(struct svga_shader_emitter_v10 *emit)
{
   VGPU10OpcodeToken0 *tokens = (VGPU10OpcodeToken0 *) emit->buf;
   unsigned inst_length;

   assert(emit->inst_start_token > 0);

   if (emit->discard_instruction) {
      /* Back up the emit->ptr to where this instruction started so
       * that we discard the current instruction.
       */
      emit->ptr = (char *) (tokens + emit->inst_start_token);
   }
   else {
      /* Compute instruction length and patch that into the start of
       * the instruction.
       */
      inst_length = emit_get_num_tokens(emit) - emit->inst_start_token;

      assert(inst_length > 0);

      tokens[emit->inst_start_token].instructionLength = inst_length;
   }

   emit->inst_start_token = 0; /* reset to zero for error checking */
   emit->discard_instruction = false;
}


/**
 * Return index for a free temporary register.
 */
static unsigned
get_temp_index(struct svga_shader_emitter_v10 *emit)
{
   assert(emit->internal_temp_count < MAX_INTERNAL_TEMPS);
   return emit->num_shader_temps + emit->internal_temp_count++;
}


/**
 * Release the temporaries which were generated by get_temp_index().
 */
static void
free_temp_indexes(struct svga_shader_emitter_v10 *emit)
{
   emit->internal_temp_count = 0;
}


/**
 * Create a tgsi_full_src_register.
 */
static struct tgsi_full_src_register
make_src_reg(enum tgsi_file_type file, unsigned index)
{
   struct tgsi_full_src_register reg;

   memset(&reg, 0, sizeof(reg));
   reg.Register.File = file;
   reg.Register.Index = index;
   reg.Register.SwizzleX = TGSI_SWIZZLE_X;
   reg.Register.SwizzleY = TGSI_SWIZZLE_Y;
   reg.Register.SwizzleZ = TGSI_SWIZZLE_Z;
   reg.Register.SwizzleW = TGSI_SWIZZLE_W;
   return reg;
}


/**
 * Create a tgsi_full_src_register with a swizzle such that all four
 * vector components have the same scalar value.
 */
static struct tgsi_full_src_register
make_src_scalar_reg(enum tgsi_file_type file, unsigned index, unsigned component)
{
   struct tgsi_full_src_register reg;

   assert(component >= TGSI_SWIZZLE_X);
   assert(component <= TGSI_SWIZZLE_W);

   memset(&reg, 0, sizeof(reg));
   reg.Register.File = file;
   reg.Register.Index = index;
   reg.Register.SwizzleX =
   reg.Register.SwizzleY =
   reg.Register.SwizzleZ =
   reg.Register.SwizzleW = component;
   return reg;
}


/**
 * Create a tgsi_full_src_register for a temporary.
 */
static struct tgsi_full_src_register
make_src_temp_reg(unsigned index)
{
   return make_src_reg(TGSI_FILE_TEMPORARY, index);
}


/**
 * Create a tgsi_full_src_register for a constant.
 */
static struct tgsi_full_src_register
make_src_const_reg(unsigned index)
{
   return make_src_reg(TGSI_FILE_CONSTANT, index);
}


/**
 * Create a tgsi_full_src_register for an immediate constant.
 */
static struct tgsi_full_src_register
make_src_immediate_reg(unsigned index)
{
   return make_src_reg(TGSI_FILE_IMMEDIATE, index);
}


/**
 * Create a tgsi_full_dst_register.
 */
static struct tgsi_full_dst_register
make_dst_reg(enum tgsi_file_type file, unsigned index)
{
   struct tgsi_full_dst_register reg;

   memset(&reg, 0, sizeof(reg));
   reg.Register.File = file;
   reg.Register.Index = index;
   reg.Register.WriteMask = TGSI_WRITEMASK_XYZW;
   return reg;
}


/**
 * Create a tgsi_full_dst_register for a temporary.
 */
static struct tgsi_full_dst_register
make_dst_temp_reg(unsigned index)
{
   return make_dst_reg(TGSI_FILE_TEMPORARY, index);
}


/**
 * Create a tgsi_full_dst_register for an output.
 */
static struct tgsi_full_dst_register
make_dst_output_reg(unsigned index)
{
   return make_dst_reg(TGSI_FILE_OUTPUT, index);
}


/**
 * Create negated tgsi_full_src_register.
 */
static struct tgsi_full_src_register
negate_src(const struct tgsi_full_src_register *reg)
{
   struct tgsi_full_src_register neg = *reg;
   neg.Register.Negate = !reg->Register.Negate;
   return neg;
}

/**
 * Create absolute value of a tgsi_full_src_register.
 */
static struct tgsi_full_src_register
absolute_src(const struct tgsi_full_src_register *reg)
{
   struct tgsi_full_src_register absolute = *reg;
   absolute.Register.Absolute = 1;
   return absolute;
}


/** Return the named swizzle term from the src register */
static inline unsigned
get_swizzle(const struct tgsi_full_src_register *reg, enum tgsi_swizzle term)
{
   switch (term) {
   case TGSI_SWIZZLE_X:
      return reg->Register.SwizzleX;
   case TGSI_SWIZZLE_Y:
      return reg->Register.SwizzleY;
   case TGSI_SWIZZLE_Z:
      return reg->Register.SwizzleZ;
   case TGSI_SWIZZLE_W:
      return reg->Register.SwizzleW;
   default:
      assert(!"Bad swizzle");
      return TGSI_SWIZZLE_X;
   }
}


/**
 * Create swizzled tgsi_full_src_register.
 */
static struct tgsi_full_src_register
swizzle_src(const struct tgsi_full_src_register *reg,
            enum tgsi_swizzle swizzleX, enum tgsi_swizzle swizzleY,
            enum tgsi_swizzle swizzleZ, enum tgsi_swizzle swizzleW)
{
   struct tgsi_full_src_register swizzled = *reg;
   /* Note: we swizzle the current swizzle */
   swizzled.Register.SwizzleX = get_swizzle(reg, swizzleX);
   swizzled.Register.SwizzleY = get_swizzle(reg, swizzleY);
   swizzled.Register.SwizzleZ = get_swizzle(reg, swizzleZ);
   swizzled.Register.SwizzleW = get_swizzle(reg, swizzleW);
   return swizzled;
}


/**
 * Create swizzled tgsi_full_src_register where all the swizzle
 * terms are the same.
 */
static struct tgsi_full_src_register
scalar_src(const struct tgsi_full_src_register *reg, enum tgsi_swizzle swizzle)
{
   struct tgsi_full_src_register swizzled = *reg;
   /* Note: we swizzle the current swizzle */
   swizzled.Register.SwizzleX =
   swizzled.Register.SwizzleY =
   swizzled.Register.SwizzleZ =
   swizzled.Register.SwizzleW = get_swizzle(reg, swizzle);
   return swizzled;
}


/**
 * Create new tgsi_full_dst_register with writemask.
 * \param mask  bitmask of TGSI_WRITEMASK_[XYZW]
 */
static struct tgsi_full_dst_register
writemask_dst(const struct tgsi_full_dst_register *reg, unsigned mask)
{
   struct tgsi_full_dst_register masked = *reg;
   masked.Register.WriteMask = mask;
   return masked;
}


/**
 * Check if the register's swizzle is XXXX, YYYY, ZZZZ, or WWWW.
 */
static bool
same_swizzle_terms(const struct tgsi_full_src_register *reg)
{
   return (reg->Register.SwizzleX == reg->Register.SwizzleY &&
           reg->Register.SwizzleY == reg->Register.SwizzleZ &&
           reg->Register.SwizzleZ == reg->Register.SwizzleW);
}


/**
 * Search the vector for the value 'x' and return its position.
 */
static int
find_imm_in_vec4(const union tgsi_immediate_data vec[4],
                 union tgsi_immediate_data x)
{
   unsigned i;
   for (i = 0; i < 4; i++) {
      if (vec[i].Int == x.Int)
         return i;
   }
   return -1;
}


/**
 * Helper used by make_immediate_reg(), make_immediate_reg_4().
 */
static int
find_immediate(struct svga_shader_emitter_v10 *emit,
               union tgsi_immediate_data x, unsigned startIndex)
{
   const unsigned endIndex = emit->num_immediates;
   unsigned i;

   assert(emit->num_immediates_emitted > 0);

   /* Search immediates for x, y, z, w */
   for (i = startIndex; i < endIndex; i++) {
      if (x.Int == emit->immediates[i][0].Int ||
          x.Int == emit->immediates[i][1].Int ||
          x.Int == emit->immediates[i][2].Int ||
          x.Int == emit->immediates[i][3].Int) {
         return i;
      }
   }
   /* immediate not declared yet */
   return -1;
}


/**
 * As above, but search for a double[2] pair.
 */
static int
find_immediate_dbl(struct svga_shader_emitter_v10 *emit,
                   double x, double y)
{
   const unsigned endIndex = emit->num_immediates;
   unsigned i;

   assert(emit->num_immediates_emitted > 0);

   /* Search immediates for x, y, z, w */
   for (i = 0; i < endIndex; i++) {
      if (x == emit->immediates_dbl[i][0] &&
          y == emit->immediates_dbl[i][1]) {
         return i;
      }
   }
   /* Should never try to use an immediate value that wasn't pre-declared */
   assert(!"find_immediate_dbl() failed!");
   return -1;
}



/**
 * Return a tgsi_full_src_register for an immediate/literal
 * union tgsi_immediate_data[4] value.
 * Note: the values must have been previously declared/allocated in
 * emit_pre_helpers().  And, all of x,y,z,w must be located in the same
 * vec4 immediate.
 */
static struct tgsi_full_src_register
make_immediate_reg_4(struct svga_shader_emitter_v10 *emit,
                     const union tgsi_immediate_data imm[4])
{
   struct tgsi_full_src_register reg;
   unsigned i;

   for (i = 0; i < emit->num_common_immediates; i++) {
      /* search for first component value */
      int immpos = find_immediate(emit, imm[0], i);
      int x, y, z, w;

      assert(immpos >= 0);

      /* find remaining components within the immediate vector */
      x = find_imm_in_vec4(emit->immediates[immpos], imm[0]);
      y = find_imm_in_vec4(emit->immediates[immpos], imm[1]);
      z = find_imm_in_vec4(emit->immediates[immpos], imm[2]);
      w = find_imm_in_vec4(emit->immediates[immpos], imm[3]);

      if (x >=0 &&  y >= 0 && z >= 0 && w >= 0) {
         /* found them all */
         memset(&reg, 0, sizeof(reg));
         reg.Register.File = TGSI_FILE_IMMEDIATE;
         reg.Register.Index = immpos;
         reg.Register.SwizzleX = x;
         reg.Register.SwizzleY = y;
         reg.Register.SwizzleZ = z;
         reg.Register.SwizzleW = w;
         return reg;
      }
      /* else, keep searching */
   }

   assert(!"Failed to find immediate register!");

   /* Just return IMM[0].xxxx */
   memset(&reg, 0, sizeof(reg));
   reg.Register.File = TGSI_FILE_IMMEDIATE;
   return reg;
}


/**
 * Return a tgsi_full_src_register for an immediate/literal
 * union tgsi_immediate_data value of the form {value, value, value, value}.
 * \sa make_immediate_reg_4() regarding allowed values.
 */
static struct tgsi_full_src_register
make_immediate_reg(struct svga_shader_emitter_v10 *emit,
                   union tgsi_immediate_data value)
{
   struct tgsi_full_src_register reg;
   int immpos = find_immediate(emit, value, 0);

   assert(immpos >= 0);

   memset(&reg, 0, sizeof(reg));
   reg.Register.File = TGSI_FILE_IMMEDIATE;
   reg.Register.Index = immpos;
   reg.Register.SwizzleX =
   reg.Register.SwizzleY =
   reg.Register.SwizzleZ =
   reg.Register.SwizzleW = find_imm_in_vec4(emit->immediates[immpos], value);

   return reg;
}


/**
 * Return a tgsi_full_src_register for an immediate/literal float[4] value.
 * \sa make_immediate_reg_4() regarding allowed values.
 */
static struct tgsi_full_src_register
make_immediate_reg_float4(struct svga_shader_emitter_v10 *emit,
                          float x, float y, float z, float w)
{
   union tgsi_immediate_data imm[4];
   imm[0].Float = x;
   imm[1].Float = y;
   imm[2].Float = z;
   imm[3].Float = w;
   return make_immediate_reg_4(emit, imm);
}


/**
 * Return a tgsi_full_src_register for an immediate/literal float value
 * of the form {value, value, value, value}.
 * \sa make_immediate_reg_4() regarding allowed values.
 */
static struct tgsi_full_src_register
make_immediate_reg_float(struct svga_shader_emitter_v10 *emit, float value)
{
   union tgsi_immediate_data imm;
   imm.Float = value;
   return make_immediate_reg(emit, imm);
}


/**
 * Return a tgsi_full_src_register for an immediate/literal int[4] vector.
 */
static struct tgsi_full_src_register
make_immediate_reg_int4(struct svga_shader_emitter_v10 *emit,
                        int x, int y, int z, int w)
{
   union tgsi_immediate_data imm[4];
   imm[0].Int = x;
   imm[1].Int = y;
   imm[2].Int = z;
   imm[3].Int = w;
   return make_immediate_reg_4(emit, imm);
}


/**
 * Return a tgsi_full_src_register for an immediate/literal int value
 * of the form {value, value, value, value}.
 * \sa make_immediate_reg_4() regarding allowed values.
 */
static struct tgsi_full_src_register
make_immediate_reg_int(struct svga_shader_emitter_v10 *emit, int value)
{
   union tgsi_immediate_data imm;
   imm.Int = value;
   return make_immediate_reg(emit, imm);
}


static struct tgsi_full_src_register
make_immediate_reg_double(struct svga_shader_emitter_v10 *emit, double value)
{
   struct tgsi_full_src_register reg;
   int immpos = find_immediate_dbl(emit, value, value);

   assert(immpos >= 0);

   memset(&reg, 0, sizeof(reg));
   reg.Register.File = TGSI_FILE_IMMEDIATE;
   reg.Register.Index = immpos;
   reg.Register.SwizzleX = TGSI_SWIZZLE_X;
   reg.Register.SwizzleY = TGSI_SWIZZLE_Y;
   reg.Register.SwizzleZ = TGSI_SWIZZLE_Z;
   reg.Register.SwizzleW = TGSI_SWIZZLE_W;

   return reg;
}


/**
 * Allocate space for a union tgsi_immediate_data[4] immediate.
 * \return  the index/position of the immediate.
 */
static unsigned
alloc_immediate_4(struct svga_shader_emitter_v10 *emit,
                  const union tgsi_immediate_data imm[4])
{
   unsigned n = emit->num_immediates++;
   assert(n < ARRAY_SIZE(emit->immediates));
   emit->immediates[n][0] = imm[0];
   emit->immediates[n][1] = imm[1];
   emit->immediates[n][2] = imm[2];
   emit->immediates[n][3] = imm[3];
   return n;
}


/**
 * Allocate space for a float[4] immediate.
 * \return  the index/position of the immediate.
 */
static unsigned
alloc_immediate_float4(struct svga_shader_emitter_v10 *emit,
                       float x, float y, float z, float w)
{
   union tgsi_immediate_data imm[4];
   imm[0].Float = x;
   imm[1].Float = y;
   imm[2].Float = z;
   imm[3].Float = w;
   return alloc_immediate_4(emit, imm);
}


/**
 * Allocate space for an int[4] immediate.
 * \return  the index/position of the immediate.
 */
static unsigned
alloc_immediate_int4(struct svga_shader_emitter_v10 *emit,
                       int x, int y, int z, int w)
{
   union tgsi_immediate_data imm[4];
   imm[0].Int = x;
   imm[1].Int = y;
   imm[2].Int = z;
   imm[3].Int = w;
   return alloc_immediate_4(emit, imm);
}


/**
 * Add a new immediate after the immediate block has been declared.
 * Any new immediates will be appended to the immediate block after the
 * shader has been parsed.
 * \return  the index/position of the immediate.
 */
static unsigned
add_immediate_int(struct svga_shader_emitter_v10 *emit, int x)
{
   union tgsi_immediate_data imm[4];
   imm[0].Int = x;
   imm[1].Int = x+1;
   imm[2].Int = x+2;
   imm[3].Int = x+3;

   unsigned immpos = alloc_immediate_4(emit, imm);
   emit->num_new_immediates++;

   return immpos;
}


static unsigned
alloc_immediate_double2(struct svga_shader_emitter_v10 *emit,
                        double x, double y)
{
   unsigned n = emit->num_immediates++;
   assert(!emit->num_immediates_emitted);
   assert(n < ARRAY_SIZE(emit->immediates));
   emit->immediates_dbl[n][0] = x;
   emit->immediates_dbl[n][1] = y;
   return n;

}


/**
 * Allocate a shader input to store a system value.
 */
static unsigned
alloc_system_value_index(struct svga_shader_emitter_v10 *emit, unsigned index)
{
   const unsigned n = emit->linkage.input_map_max + 1 + index;
   assert(index < ARRAY_SIZE(emit->system_value_indexes));
   emit->system_value_indexes[index] = n;
   return n;
}


/**
 * Translate a TGSI immediate value (union tgsi_immediate_data[4]) to VGPU10.
 */
static bool
emit_vgpu10_immediate(struct svga_shader_emitter_v10 *emit,
                      const struct tgsi_full_immediate *imm)
{
   /* We don't actually emit any code here.  We just save the
    * immediate values and emit them later.
    */
   alloc_immediate_4(emit, imm->u);
   return true;
}


/**
 * Emit a VGPU10_CUSTOMDATA_DCL_IMMEDIATE_CONSTANT_BUFFER block
 * containing all the immediate values previously allocated
 * with alloc_immediate_4().
 */
static bool
emit_vgpu10_immediates_block(struct svga_shader_emitter_v10 *emit)
{
   VGPU10OpcodeToken0 token;

   assert(!emit->num_immediates_emitted);

   token.value = 0;
   token.opcodeType = VGPU10_OPCODE_CUSTOMDATA;
   token.customDataClass = VGPU10_CUSTOMDATA_DCL_IMMEDIATE_CONSTANT_BUFFER;

   emit->immediates_block_start_token =
      (emit->ptr - emit->buf) / sizeof(VGPU10OpcodeToken0);

   /* Note: no begin/end_emit_instruction() calls */
   emit_dword(emit, token.value);
   emit_dword(emit, 2 + 4 * emit->num_immediates);
   emit_dwords(emit, (unsigned *) emit->immediates, 4 * emit->num_immediates);

   emit->num_immediates_emitted = emit->num_immediates;

   emit->immediates_block_next_token =
      (emit->ptr - emit->buf) / sizeof(VGPU10OpcodeToken0);

   return true;
}


/**
 * Reemit the immediate constant buffer block to include the new
 * immediates that are allocated after the block is declared. Those
 * immediates are used as constant indices to constant buffers.
 */
static bool
reemit_immediates_block(struct svga_shader_emitter_v10 *emit)
{
   unsigned num_tokens = emit_get_num_tokens(emit);
   unsigned num_new_immediates = emit->num_new_immediates;

   /* Reserve room for the new immediates */
   if (!reserve(emit, 4 * num_new_immediates))
      return false;

   /* Move the tokens after the immediates block to make room for the
    * new immediates.
    */
   VGPU10ProgramToken *tokens = (VGPU10ProgramToken *)emit->buf;
   char *next = (char *) (tokens + emit->immediates_block_next_token);
   char *new_next = (char *) (tokens + emit->immediates_block_next_token +
                                 num_new_immediates * 4);

   char *end = emit->ptr;
   unsigned len = end - next;
   memmove(new_next, next, len);

   /* Append the new immediates to the end of the immediates block */
   char *start = (char *) (tokens + emit->immediates_block_start_token+1);
   unsigned immediates_block_size = *(uint32 *)start;

   char *new_immediates = (char *)&emit->immediates[emit->num_immediates_emitted][0];
   *(uint32 *)start = immediates_block_size + 4 * num_new_immediates;
   memcpy(next, new_immediates, 4 * num_new_immediates * sizeof(uint32));

   emit->ptr = (char *) (tokens + num_tokens + 4 * num_new_immediates);

   return true;
}



/**
 * Translate a fragment shader's TGSI_INTERPOLATE_x mode to a vgpu10
 * interpolation mode.
 * \return a VGPU10_INTERPOLATION_x value
 */
static unsigned
translate_interpolation(const struct svga_shader_emitter_v10 *emit,
                        enum tgsi_interpolate_mode interp,
                        enum tgsi_interpolate_loc interpolate_loc)
{
   if (interp == TGSI_INTERPOLATE_COLOR) {
      interp = emit->key.fs.flatshade ?
         TGSI_INTERPOLATE_CONSTANT : TGSI_INTERPOLATE_PERSPECTIVE;
   }

   switch (interp) {
   case TGSI_INTERPOLATE_CONSTANT:
      return VGPU10_INTERPOLATION_CONSTANT;
   case TGSI_INTERPOLATE_LINEAR:
      if (interpolate_loc == TGSI_INTERPOLATE_LOC_CENTROID) {
         return VGPU10_INTERPOLATION_LINEAR_NOPERSPECTIVE_CENTROID;
      } else if (interpolate_loc == TGSI_INTERPOLATE_LOC_SAMPLE &&
                 emit->version >= 41) {
         return VGPU10_INTERPOLATION_LINEAR_NOPERSPECTIVE_SAMPLE;
      } else {
         return VGPU10_INTERPOLATION_LINEAR_NOPERSPECTIVE;
      }
      break;
   case TGSI_INTERPOLATE_PERSPECTIVE:
      if (interpolate_loc == TGSI_INTERPOLATE_LOC_CENTROID) {
         return VGPU10_INTERPOLATION_LINEAR_CENTROID;
      } else if (interpolate_loc == TGSI_INTERPOLATE_LOC_SAMPLE &&
                 emit->version >= 41) {
         return VGPU10_INTERPOLATION_LINEAR_SAMPLE;
      } else {
         return VGPU10_INTERPOLATION_LINEAR;
      }
      break;
   default:
      assert(!"Unexpected interpolation mode");
      return VGPU10_INTERPOLATION_CONSTANT;
   }
}


/**
 * Translate a TGSI property to VGPU10.
 * Don't emit any instructions yet, only need to gather the primitive property
 * information.  The output primitive topology might be changed later. The
 * final property instructions will be emitted as part of the pre-helper code.
 */
static bool
emit_vgpu10_property(struct svga_shader_emitter_v10 *emit,
                     const struct tgsi_full_property *prop)
{
   static const VGPU10_PRIMITIVE primType[] = {
      VGPU10_PRIMITIVE_POINT,           /* MESA_PRIM_POINTS */
      VGPU10_PRIMITIVE_LINE,            /* MESA_PRIM_LINES */
      VGPU10_PRIMITIVE_LINE,            /* MESA_PRIM_LINE_LOOP */
      VGPU10_PRIMITIVE_LINE,            /* MESA_PRIM_LINE_STRIP */
      VGPU10_PRIMITIVE_TRIANGLE,        /* MESA_PRIM_TRIANGLES */
      VGPU10_PRIMITIVE_TRIANGLE,        /* MESA_PRIM_TRIANGLE_STRIP */
      VGPU10_PRIMITIVE_TRIANGLE,        /* MESA_PRIM_TRIANGLE_FAN */
      VGPU10_PRIMITIVE_UNDEFINED,       /* MESA_PRIM_QUADS */
      VGPU10_PRIMITIVE_UNDEFINED,       /* MESA_PRIM_QUAD_STRIP */
      VGPU10_PRIMITIVE_UNDEFINED,       /* MESA_PRIM_POLYGON */
      VGPU10_PRIMITIVE_LINE_ADJ,        /* MESA_PRIM_LINES_ADJACENCY */
      VGPU10_PRIMITIVE_LINE_ADJ,        /* MESA_PRIM_LINE_STRIP_ADJACENCY */
      VGPU10_PRIMITIVE_TRIANGLE_ADJ,    /* MESA_PRIM_TRIANGLES_ADJACENCY */
      VGPU10_PRIMITIVE_TRIANGLE_ADJ     /* MESA_PRIM_TRIANGLE_STRIP_ADJACENCY */
   };

   static const VGPU10_PRIMITIVE_TOPOLOGY primTopology[] = {
      VGPU10_PRIMITIVE_TOPOLOGY_POINTLIST,     /* MESA_PRIM_POINTS */
      VGPU10_PRIMITIVE_TOPOLOGY_LINELIST,      /* MESA_PRIM_LINES */
      VGPU10_PRIMITIVE_TOPOLOGY_LINELIST,      /* MESA_PRIM_LINE_LOOP */
      VGPU10_PRIMITIVE_TOPOLOGY_LINESTRIP,     /* MESA_PRIM_LINE_STRIP */
      VGPU10_PRIMITIVE_TOPOLOGY_TRIANGLELIST,  /* MESA_PRIM_TRIANGLES */
      VGPU10_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP, /* MESA_PRIM_TRIANGLE_STRIP */
      VGPU10_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP, /* MESA_PRIM_TRIANGLE_FAN */
      VGPU10_PRIMITIVE_TOPOLOGY_UNDEFINED,     /* MESA_PRIM_QUADS */
      VGPU10_PRIMITIVE_TOPOLOGY_UNDEFINED,     /* MESA_PRIM_QUAD_STRIP */
      VGPU10_PRIMITIVE_TOPOLOGY_UNDEFINED,     /* MESA_PRIM_POLYGON */
      VGPU10_PRIMITIVE_TOPOLOGY_LINELIST_ADJ,  /* MESA_PRIM_LINES_ADJACENCY */
      VGPU10_PRIMITIVE_TOPOLOGY_LINELIST_ADJ,  /* MESA_PRIM_LINE_STRIP_ADJACENCY */
      VGPU10_PRIMITIVE_TOPOLOGY_TRIANGLELIST_ADJ, /* MESA_PRIM_TRIANGLES_ADJACENCY */
      VGPU10_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP_ADJ /* MESA_PRIM_TRIANGLE_STRIP_ADJACENCY */
   };

   static const unsigned inputArraySize[] = {
      0,       /* VGPU10_PRIMITIVE_UNDEFINED */
      1,       /* VGPU10_PRIMITIVE_POINT */
      2,       /* VGPU10_PRIMITIVE_LINE */
      3,       /* VGPU10_PRIMITIVE_TRIANGLE */
      0,
      0,
      4,       /* VGPU10_PRIMITIVE_LINE_ADJ */
      6        /* VGPU10_PRIMITIVE_TRIANGLE_ADJ */
   };

   switch (prop->Property.PropertyName) {
   case TGSI_PROPERTY_GS_INPUT_PRIM:
      assert(prop->u[0].Data < ARRAY_SIZE(primType));
      emit->gs.prim_type = primType[prop->u[0].Data];
      assert(emit->gs.prim_type != VGPU10_PRIMITIVE_UNDEFINED);
      emit->gs.input_size = inputArraySize[emit->gs.prim_type];
      break;

   case TGSI_PROPERTY_GS_OUTPUT_PRIM:
      assert(prop->u[0].Data < ARRAY_SIZE(primTopology));
      emit->gs.prim_topology = primTopology[prop->u[0].Data];
      assert(emit->gs.prim_topology != VGPU10_PRIMITIVE_TOPOLOGY_UNDEFINED);
      break;

   case TGSI_PROPERTY_GS_MAX_OUTPUT_VERTICES:
      emit->gs.max_out_vertices = prop->u[0].Data;
      break;

   case TGSI_PROPERTY_GS_INVOCATIONS:
      emit->gs.invocations = prop->u[0].Data;
      break;

   case TGSI_PROPERTY_FS_COLOR0_WRITES_ALL_CBUFS:
   case TGSI_PROPERTY_NEXT_SHADER:
   case TGSI_PROPERTY_NUM_CLIPDIST_ENABLED:
      /* no-op */
      break;

   case TGSI_PROPERTY_TCS_VERTICES_OUT:
      /* This info is already captured in the shader key */
      break;

   case TGSI_PROPERTY_TES_PRIM_MODE:
      emit->tes.prim_mode = prop->u[0].Data;
      break;

   case TGSI_PROPERTY_TES_SPACING:
      emit->tes.spacing = prop->u[0].Data;
      break;

   case TGSI_PROPERTY_TES_VERTEX_ORDER_CW:
      emit->tes.vertices_order_cw = prop->u[0].Data;
      break;

   case TGSI_PROPERTY_TES_POINT_MODE:
      emit->tes.point_mode = prop->u[0].Data;
      break;

   case TGSI_PROPERTY_CS_FIXED_BLOCK_WIDTH:
      emit->cs.block_width = prop->u[0].Data;
      break;

   case TGSI_PROPERTY_CS_FIXED_BLOCK_HEIGHT:
      emit->cs.block_height = prop->u[0].Data;
      break;

   case TGSI_PROPERTY_CS_FIXED_BLOCK_DEPTH:
      emit->cs.block_depth = prop->u[0].Data;
      break;

   case TGSI_PROPERTY_FS_EARLY_DEPTH_STENCIL:
      emit->fs.forceEarlyDepthStencil = true;
      break;

   default:
      debug_printf("Unexpected TGSI property %s\n",
                   tgsi_property_names[prop->Property.PropertyName]);
   }

   return true;
}


static void
emit_property_instruction(struct svga_shader_emitter_v10 *emit,
                          VGPU10OpcodeToken0 opcode0, unsigned nData,
                          unsigned data)
{
   begin_emit_instruction(emit);
   emit_dword(emit, opcode0.value);
   if (nData)
      emit_dword(emit, data);
   end_emit_instruction(emit);
}


/**
 * Emit property instructions
 */
static void
emit_property_instructions(struct svga_shader_emitter_v10 *emit)
{
   VGPU10OpcodeToken0 opcode0;

   assert(emit->unit == PIPE_SHADER_GEOMETRY);

   /* emit input primitive type declaration */
   opcode0.value = 0;
   opcode0.opcodeType = VGPU10_OPCODE_DCL_GS_INPUT_PRIMITIVE;
   opcode0.primitive = emit->gs.prim_type;
   emit_property_instruction(emit, opcode0, 0, 0);

   /* emit max output vertices */
   opcode0.value = 0;
   opcode0.opcodeType = VGPU10_OPCODE_DCL_MAX_OUTPUT_VERTEX_COUNT;
   emit_property_instruction(emit, opcode0, 1, emit->gs.max_out_vertices);

   if (emit->version >= 50 && emit->gs.invocations > 0) {
      opcode0.value = 0;
      opcode0.opcodeType = VGPU10_OPCODE_DCL_GS_INSTANCE_COUNT;
      emit_property_instruction(emit, opcode0, 1, emit->gs.invocations);
   }
}


/**
 * A helper function to declare tessellator domain in a hull shader or
 * in the domain shader.
 */
static void
emit_tessellator_domain(struct svga_shader_emitter_v10 *emit,
                        enum mesa_prim prim_mode)
{
   VGPU10OpcodeToken0 opcode0;

   opcode0.value = 0;
   opcode0.opcodeType = VGPU10_OPCODE_DCL_TESS_DOMAIN;
   switch (prim_mode) {
   case MESA_PRIM_QUADS:
   case MESA_PRIM_LINES:
      opcode0.tessDomain = VGPU10_TESSELLATOR_DOMAIN_QUAD;
      break;
   case MESA_PRIM_TRIANGLES:
      opcode0.tessDomain = VGPU10_TESSELLATOR_DOMAIN_TRI;
      break;
   default:
      debug_printf("Invalid tessellator prim mode %d\n", prim_mode);
      opcode0.tessDomain = VGPU10_TESSELLATOR_DOMAIN_UNDEFINED;
   }
   begin_emit_instruction(emit);
   emit_dword(emit, opcode0.value);
   end_emit_instruction(emit);
}


/**
 * Emit domain shader declarations.
 */
static void
emit_domain_shader_declarations(struct svga_shader_emitter_v10 *emit)
{
   VGPU10OpcodeToken0 opcode0;

   assert(emit->unit == PIPE_SHADER_TESS_EVAL);

   /* Emit the input control point count */
   assert(emit->key.tes.vertices_per_patch >= 0 &&
          emit->key.tes.vertices_per_patch <= 32);

   opcode0.value = 0;
   opcode0.opcodeType = VGPU10_OPCODE_DCL_INPUT_CONTROL_POINT_COUNT;
   opcode0.controlPointCount = emit->key.tes.vertices_per_patch;
   begin_emit_instruction(emit);
   emit_dword(emit, opcode0.value);
   end_emit_instruction(emit);

   emit_tessellator_domain(emit, emit->tes.prim_mode);

   /* Specify a max for swizzles of the domain point according to the
    * tessellator domain type.
    */
   emit->tes.swizzle_max = emit->tes.prim_mode == MESA_PRIM_TRIANGLES ?
                              TGSI_SWIZZLE_Z : TGSI_SWIZZLE_Y;
}


/**
 * Some common values like 0.0, 1.0, 0.5, etc. are frequently needed
 * to implement some instructions.  We pre-allocate those values here
 * in the immediate constant buffer.
 */
static void
alloc_common_immediates(struct svga_shader_emitter_v10 *emit)
{
   unsigned n = 0;

   emit->common_immediate_pos[n++] =
      alloc_immediate_float4(emit, 0.0f, 1.0f, 0.5f, -1.0f);

   if (emit->info.opcode_count[TGSI_OPCODE_LIT] > 0) {
      emit->common_immediate_pos[n++] =
         alloc_immediate_float4(emit, 128.0f, -128.0f, 0.0f, 0.0f);
   }

   emit->common_immediate_pos[n++] =
      alloc_immediate_int4(emit, 0, 1, 2, -1);

   emit->common_immediate_pos[n++] =
      alloc_immediate_int4(emit, 3, 4, 5, 6);

   if (emit->info.opcode_count[TGSI_OPCODE_IMSB] > 0 ||
       emit->info.opcode_count[TGSI_OPCODE_UMSB] > 0) {
      emit->common_immediate_pos[n++] =
         alloc_immediate_int4(emit, 31, 0, 0, 0);
   }

   if (emit->info.opcode_count[TGSI_OPCODE_UBFE] > 0 ||
       emit->info.opcode_count[TGSI_OPCODE_IBFE] > 0 ||
       emit->info.opcode_count[TGSI_OPCODE_BFI] > 0) {
      emit->common_immediate_pos[n++] =
         alloc_immediate_int4(emit, 32, 0, 0, 0);
   }

   if (emit->key.vs.attrib_puint_to_snorm) {
      emit->common_immediate_pos[n++] =
         alloc_immediate_float4(emit, -2.0f, 2.0f, 3.0f, -1.66666f);
   }

   if (emit->key.vs.attrib_puint_to_uscaled) {
      emit->common_immediate_pos[n++] =
         alloc_immediate_float4(emit, 1023.0f, 3.0f, 0.0f, 0.0f);
   }

   if (emit->key.vs.attrib_puint_to_sscaled) {
      emit->common_immediate_pos[n++] =
         alloc_immediate_int4(emit, 22, 12, 2, 0);

      emit->common_immediate_pos[n++] =
         alloc_immediate_int4(emit, 22, 30, 0, 0);
   }

   if (emit->vposition.num_prescale > 1) {
      unsigned i;
      for (i = 0; i < emit->vposition.num_prescale; i+=4) {
         emit->common_immediate_pos[n++] =
            alloc_immediate_int4(emit, i, i+1, i+2, i+3);
      }
   }

   emit->immediates_dbl = (double (*)[2]) emit->immediates;

   if (emit->info.opcode_count[TGSI_OPCODE_DNEG] > 0) {
      emit->common_immediate_pos[n++] =
         alloc_immediate_double2(emit, -1.0, -1.0);
   }

   if (emit->info.opcode_count[TGSI_OPCODE_DSQRT] > 0 ||
       emit->info.opcode_count[TGSI_OPCODE_DTRUNC] > 0) {
      emit->common_immediate_pos[n++] =
         alloc_immediate_double2(emit, 0.0, 0.0);
      emit->common_immediate_pos[n++] =
         alloc_immediate_double2(emit, 1.0, 1.0);
   }

   if (emit->info.opcode_count[TGSI_OPCODE_INTERP_OFFSET] > 0) {
      emit->common_immediate_pos[n++] =
         alloc_immediate_float4(emit, 16.0f, -16.0f, 0.0, 0.0);
   }

   assert(n <= ARRAY_SIZE(emit->common_immediate_pos));

   unsigned i;

   for (i = 0; i < PIPE_MAX_SAMPLERS; i++) {
      if (emit->key.tex[i].texel_bias) {
         /* Replace 0.0f if more immediate float value is needed */
         emit->common_immediate_pos[n++] =
            alloc_immediate_float4(emit, 0.0001f, 0.0f, 0.0f, 0.0f);
         break;
      }
   }

   /** TODO: allocate immediates for all possible element byte offset?
    */
   if (emit->raw_bufs) {
      unsigned i;
      for (i = 7; i < 12; i+=4) {
         emit->common_immediate_pos[n++] =
            alloc_immediate_int4(emit, i, (i+1), (i+2), (i+3));
      }
   }

   if (emit->info.indirect_files &
       (1 << TGSI_FILE_IMAGE | 1 << TGSI_FILE_BUFFER)) {
      unsigned i;
      for (i = 7; i < 8; i+=4) {
         emit->common_immediate_pos[n++] =
            alloc_immediate_int4(emit, i, (i+1), (i+2), (i+3));
      }
   }

   assert(n <= ARRAY_SIZE(emit->common_immediate_pos));
   emit->num_common_immediates = n;
}


/**
 * Emit hull shader declarations.
*/
static void
emit_hull_shader_declarations(struct svga_shader_emitter_v10 *emit)
{
   VGPU10OpcodeToken0 opcode0;

   /* Emit the input control point count */
   assert(emit->key.tcs.vertices_per_patch > 0 &&
          emit->key.tcs.vertices_per_patch <= 32);

   opcode0.value = 0;
   opcode0.opcodeType = VGPU10_OPCODE_DCL_INPUT_CONTROL_POINT_COUNT;
   opcode0.controlPointCount = emit->key.tcs.vertices_per_patch;
   begin_emit_instruction(emit);
   emit_dword(emit, opcode0.value);
   end_emit_instruction(emit);

   /* Emit the output control point count */
   assert(emit->key.tcs.vertices_out >= 0 && emit->key.tcs.vertices_out <= 32);

   opcode0.value = 0;
   opcode0.opcodeType = VGPU10_OPCODE_DCL_OUTPUT_CONTROL_POINT_COUNT;
   opcode0.controlPointCount = emit->key.tcs.vertices_out;
   begin_emit_instruction(emit);
   emit_dword(emit, opcode0.value);
   end_emit_instruction(emit);

   /* Emit tessellator domain */
   emit_tessellator_domain(emit, emit->key.tcs.prim_mode);

   /* Emit tessellator output primitive */
   opcode0.value = 0;
   opcode0.opcodeType = VGPU10_OPCODE_DCL_TESS_OUTPUT_PRIMITIVE;
   if (emit->key.tcs.point_mode) {
      opcode0.tessOutputPrimitive = VGPU10_TESSELLATOR_OUTPUT_POINT;
   }
   else if (emit->key.tcs.prim_mode == MESA_PRIM_LINES) {
      opcode0.tessOutputPrimitive = VGPU10_TESSELLATOR_OUTPUT_LINE;
   }
   else {
      assert(emit->key.tcs.prim_mode == MESA_PRIM_QUADS ||
             emit->key.tcs.prim_mode == MESA_PRIM_TRIANGLES);

      if (emit->key.tcs.vertices_order_cw)
         opcode0.tessOutputPrimitive = VGPU10_TESSELLATOR_OUTPUT_TRIANGLE_CCW;
      else
         opcode0.tessOutputPrimitive = VGPU10_TESSELLATOR_OUTPUT_TRIANGLE_CW;
   }
   begin_emit_instruction(emit);
   emit_dword(emit, opcode0.value);
   end_emit_instruction(emit);

   /* Emit tessellator partitioning */
   opcode0.value = 0;
   opcode0.opcodeType = VGPU10_OPCODE_DCL_TESS_PARTITIONING;
   switch (emit->key.tcs.spacing) {
   case PIPE_TESS_SPACING_FRACTIONAL_ODD:
      opcode0.tessPartitioning = VGPU10_TESSELLATOR_PARTITIONING_FRACTIONAL_ODD;
      break;
   case PIPE_TESS_SPACING_FRACTIONAL_EVEN:
      opcode0.tessPartitioning = VGPU10_TESSELLATOR_PARTITIONING_FRACTIONAL_EVEN;
      break;
   case PIPE_TESS_SPACING_EQUAL:
      opcode0.tessPartitioning = VGPU10_TESSELLATOR_PARTITIONING_INTEGER;
      break;
   default:
      debug_printf("invalid tessellator spacing %d\n", emit->key.tcs.spacing);
      opcode0.tessPartitioning = VGPU10_TESSELLATOR_PARTITIONING_UNDEFINED;
   }
   begin_emit_instruction(emit);
   emit_dword(emit, opcode0.value);
   end_emit_instruction(emit);

   alloc_common_immediates(emit);

   /* Declare constant registers */
   emit_constant_declaration(emit);

   /* Declare samplers and resources */
   emit_sampler_declarations(emit);
   emit_resource_declarations(emit);

   /* Declare images */
   emit_image_declarations(emit);

   /* Declare shader buffers */
   emit_shader_buf_declarations(emit);

   /* Declare atomic buffers */
   emit_atomic_buf_declarations(emit);

   int nVertices = emit->key.tcs.vertices_per_patch;
   emit->tcs.imm_index =
      alloc_immediate_int4(emit, nVertices, nVertices, nVertices, 0);

   /* Now, emit the constant block containing all the immediates
    * declared by shader, as well as the extra ones seen above.
    */
   emit_vgpu10_immediates_block(emit);

}


/**
 * A helper function to determine if control point phase is needed.
 * Returns TRUE if there is control point output.
 */
static bool
needs_control_point_phase(struct svga_shader_emitter_v10 *emit)
{
   unsigned i;

   assert(emit->unit == PIPE_SHADER_TESS_CTRL);

   /* If output control point count does not match the input count,
    * we need a control point phase to explicitly set the output control
    * points.
    */
   if ((emit->key.tcs.vertices_per_patch != emit->key.tcs.vertices_out) &&
       emit->key.tcs.vertices_out)
      return true;

   for (i = 0; i < emit->info.num_outputs; i++) {
      switch (emit->info.output_semantic_name[i]) {
      case TGSI_SEMANTIC_PATCH:
      case TGSI_SEMANTIC_TESSOUTER:
      case TGSI_SEMANTIC_TESSINNER:
         break;
      default:
         return true;
      }
   }
   return false;
}


/**
 * A helper function to add shader signature for passthrough control point
 * phase. This signature is also generated for passthrough control point
 * phase from HLSL compiler and is needed by Metal Renderer.
 */
static void
emit_passthrough_control_point_signature(struct svga_shader_emitter_v10 *emit)
{
   struct svga_shader_signature *sgn = &emit->signature;
   SVGA3dDXShaderSignatureEntry *sgnEntry;
   unsigned i;

   for (i = 0; i < emit->info.num_inputs; i++) {
      unsigned index = emit->linkage.input_map[i];
      enum tgsi_semantic sem_name = emit->info.input_semantic_name[i];

      sgnEntry = &sgn->inputs[sgn->header.numInputSignatures++];

      set_shader_signature_entry(sgnEntry, index,
                                 tgsi_semantic_to_sgn_name[sem_name],
                                 VGPU10_OPERAND_4_COMPONENT_MASK_ALL,
                                 SVGADX_SIGNATURE_REGISTER_COMPONENT_UNKNOWN,
                                 SVGADX_SIGNATURE_MIN_PRECISION_DEFAULT);

      sgnEntry = &sgn->outputs[sgn->header.numOutputSignatures++];

      set_shader_signature_entry(sgnEntry, i,
                                 tgsi_semantic_to_sgn_name[sem_name],
                                 VGPU10_OPERAND_4_COMPONENT_MASK_ALL,
                                 SVGADX_SIGNATURE_REGISTER_COMPONENT_UNKNOWN,
                                 SVGADX_SIGNATURE_MIN_PRECISION_DEFAULT);
   }
}


/**
 * A helper function to emit an instruction to start the control point phase
 * in the hull shader.
 */
static void
emit_control_point_phase_instruction(struct svga_shader_emitter_v10 *emit)
{
   VGPU10OpcodeToken0 opcode0;

   opcode0.value = 0;
   opcode0.opcodeType = VGPU10_OPCODE_HS_CONTROL_POINT_PHASE;
   begin_emit_instruction(emit);
   emit_dword(emit, opcode0.value);
   end_emit_instruction(emit);
}


/**
 * Start the hull shader control point phase
 */
static bool
emit_hull_shader_control_point_phase(struct svga_shader_emitter_v10 *emit)
{
   /* If there is no control point output, skip the control point phase. */
   if (!needs_control_point_phase(emit)) {
      if (!emit->key.tcs.vertices_out) {
         /**
          * If the tcs does not explicitly generate any control point output
          * and the tes does not use any input control point, then
          * emit an empty control point phase with zero output control
          * point count.
          */
         emit_control_point_phase_instruction(emit);

         /**
          * Since this is an empty control point phase, we will need to
          * add input signatures when we parse the tcs again in the
          * patch constant phase.
          */
         emit->tcs.fork_phase_add_signature = true;
      }
      else {
         /**
          * Before skipping the control point phase, add the signature for
          * the passthrough control point.
          */
         emit_passthrough_control_point_signature(emit);
      }
      return false;
   }

   /* Start the control point phase in the hull shader */
   emit_control_point_phase_instruction(emit);

   /* Declare the output control point ID */
   if (emit->tcs.invocation_id_sys_index == INVALID_INDEX) {
      /* Add invocation id declaration if it does not exist */
      emit->tcs.invocation_id_sys_index = emit->info.num_system_values + 1;
   }

   emit_input_declaration(emit, VGPU10_OPCODE_DCL_INPUT,
                          VGPU10_OPERAND_TYPE_OUTPUT_CONTROL_POINT_ID,
                          VGPU10_OPERAND_INDEX_0D,
                          0, 1,
                          VGPU10_NAME_UNDEFINED,
                          VGPU10_OPERAND_0_COMPONENT, 0,
                          0,
                          VGPU10_INTERPOLATION_CONSTANT, true,
                          SVGADX_SIGNATURE_SEMANTIC_NAME_UNDEFINED);

   if (emit->tcs.prim_id_index != INVALID_INDEX) {
      emit_input_declaration(emit, VGPU10_OPCODE_DCL_INPUT,
                             VGPU10_OPERAND_TYPE_INPUT_PRIMITIVEID,
                             VGPU10_OPERAND_INDEX_0D,
                             0, 1,
                             VGPU10_NAME_UNDEFINED,
                             VGPU10_OPERAND_0_COMPONENT,
                             VGPU10_OPERAND_4_COMPONENT_MASK_MODE,
                             0,
                             VGPU10_INTERPOLATION_UNDEFINED, true,
                             SVGADX_SIGNATURE_SEMANTIC_NAME_PRIMITIVE_ID);
   }

   return true;
}


/**
 * Start the hull shader patch constant phase and
 * do the second pass of the tcs translation and emit
 * the relevant declarations and instructions for this phase.
 */
static bool
emit_hull_shader_patch_constant_phase(struct svga_shader_emitter_v10 *emit,
                                      struct tgsi_parse_context *parse)
{
   unsigned inst_number = 0;
   bool ret = true;
   VGPU10OpcodeToken0 opcode0;

   emit->skip_instruction = false;

   /* Start the patch constant phase */
   opcode0.value = 0;
   opcode0.opcodeType = VGPU10_OPCODE_HS_FORK_PHASE;
   begin_emit_instruction(emit);
   emit_dword(emit, opcode0.value);
   end_emit_instruction(emit);

   /* Set the current phase to patch constant phase */
   emit->tcs.control_point_phase = false;

   if (emit->tcs.prim_id_index != INVALID_INDEX) {
      emit_input_declaration(emit, VGPU10_OPCODE_DCL_INPUT,
                             VGPU10_OPERAND_TYPE_INPUT_PRIMITIVEID,
                             VGPU10_OPERAND_INDEX_0D,
                             0, 1,
                             VGPU10_NAME_UNDEFINED,
                             VGPU10_OPERAND_0_COMPONENT,
                             VGPU10_OPERAND_4_COMPONENT_MASK_MODE,
                             0,
                             VGPU10_INTERPOLATION_UNDEFINED, true,
                             SVGADX_SIGNATURE_SEMANTIC_NAME_PRIMITIVE_ID);
   }

   /* Emit declarations for this phase */
   emit->index_range.required =
      emit->info.indirect_files & (1 << TGSI_FILE_INPUT) ? true : false;
   emit_tcs_input_declarations(emit);

   if (emit->index_range.start_index != INVALID_INDEX) {
      emit_index_range_declaration(emit);
   }

   emit->index_range.required =
      emit->info.indirect_files & (1 << TGSI_FILE_OUTPUT) ? true : false;
   emit_tcs_output_declarations(emit);

   if (emit->index_range.start_index != INVALID_INDEX) {
      emit_index_range_declaration(emit);
   }
   emit->index_range.required = false;

   emit_temporaries_declaration(emit);

   /* Reset the token position to the first instruction token
    * in preparation for the second pass of the shader
    */
   parse->Position = emit->tcs.instruction_token_pos;

   while (!tgsi_parse_end_of_tokens(parse)) {
      tgsi_parse_token(parse);

      assert(parse->FullToken.Token.Type == TGSI_TOKEN_TYPE_INSTRUCTION);
      ret = emit_vgpu10_instruction(emit, inst_number++,
                                    &parse->FullToken.FullInstruction);

      /* Usually this applies to TCS only. If shader is reading output of
       * patch constant in fork phase, we should reemit all instructions
       * which are writting into output of patch constant in fork phase
       * to store results into temporaries.
       */
      assert(!(emit->reemit_instruction && emit->reemit_rawbuf_instruction));
      if (emit->reemit_instruction) {
         assert(emit->unit == PIPE_SHADER_TESS_CTRL);
         ret = emit_vgpu10_instruction(emit, inst_number,
                                       &parse->FullToken.FullInstruction);
      } else if (emit->reemit_rawbuf_instruction) {
         ret = emit_rawbuf_instruction(emit, inst_number,
                                       &parse->FullToken.FullInstruction);
      }

      if (!ret)
         return false;
   }

   return true;
}


/**
 * Emit the thread group declaration for compute shader.
 */
static void
emit_compute_shader_declarations(struct svga_shader_emitter_v10 *emit)
{
   VGPU10OpcodeToken0 opcode0;

   opcode0.value = 0;
   opcode0.opcodeType = VGPU10_OPCODE_DCL_THREAD_GROUP;
   begin_emit_instruction(emit);
   emit_dword(emit, opcode0.value);
   emit_dword(emit, emit->cs.block_width);
   emit_dword(emit, emit->cs.block_height);
   emit_dword(emit, emit->cs.block_depth);
   end_emit_instruction(emit);
}


/**
 * Emit index range declaration.
 */
static bool
emit_index_range_declaration(struct svga_shader_emitter_v10 *emit)
{
   if (emit->version < 50)
      return true;

   assert(emit->index_range.start_index != INVALID_INDEX);
   assert(emit->index_range.count != 0);
   assert(emit->index_range.required);
   assert(emit->index_range.operandType != VGPU10_NUM_OPERANDS);
   assert(emit->index_range.dim != 0);
   assert(emit->index_range.size != 0);

   VGPU10OpcodeToken0 opcode0;
   VGPU10OperandToken0 operand0;

   opcode0.value = 0;
   opcode0.opcodeType = VGPU10_OPCODE_DCL_INDEX_RANGE;

   operand0.value = 0;
   operand0.numComponents = VGPU10_OPERAND_4_COMPONENT;
   operand0.indexDimension = emit->index_range.dim;
   operand0.operandType = emit->index_range.operandType;
   operand0.mask = VGPU10_OPERAND_4_COMPONENT_MASK_ALL;
   operand0.index0Representation = VGPU10_OPERAND_INDEX_IMMEDIATE32;

   if (emit->index_range.dim == VGPU10_OPERAND_INDEX_2D)
      operand0.index1Representation = VGPU10_OPERAND_INDEX_IMMEDIATE32;

   begin_emit_instruction(emit);
   emit_dword(emit, opcode0.value);
   emit_dword(emit, operand0.value);

   if (emit->index_range.dim == VGPU10_OPERAND_INDEX_2D) {
      emit_dword(emit, emit->index_range.size);
      emit_dword(emit, emit->index_range.start_index);
      emit_dword(emit, emit->index_range.count);
   }
   else {
      emit_dword(emit, emit->index_range.start_index);
      emit_dword(emit, emit->index_range.count);
   }

   end_emit_instruction(emit);

   /* Reset fields in emit->index_range struct except
    * emit->index_range.required which will be reset afterwards
    */
   emit->index_range.count = 0;
   emit->index_range.operandType = VGPU10_NUM_OPERANDS;
   emit->index_range.start_index = INVALID_INDEX;
   emit->index_range.size = 0;
   emit->index_range.dim = 0;

   return true;
}


/**
 * Emit a vgpu10 declaration "instruction".
 * \param index  the register index
 * \param size   array size of the operand. In most cases, it is 1,
 *               but for inputs to geometry shader, the array size varies
 *               depending on the primitive type.
 */
static void
emit_decl_instruction(struct svga_shader_emitter_v10 *emit,
                      VGPU10OpcodeToken0 opcode0,
                      VGPU10OperandToken0 operand0,
                      VGPU10NameToken name_token,
                      unsigned index, unsigned size)
{
   assert(opcode0.opcodeType);
   assert(operand0.mask ||
          (operand0.operandType == VGPU10_OPERAND_TYPE_OUTPUT) ||
          (operand0.operandType == VGPU10_OPERAND_TYPE_OUTPUT_DEPTH) ||
          (operand0.operandType == VGPU10_OPERAND_TYPE_OUTPUT_COVERAGE_MASK) ||
          (operand0.operandType == VGPU10_OPERAND_TYPE_OUTPUT_CONTROL_POINT_ID) ||
          (operand0.operandType == VGPU10_OPERAND_TYPE_INPUT_PRIMITIVEID) ||
          (operand0.operandType == VGPU10_OPERAND_TYPE_INPUT_GS_INSTANCE_ID) ||
          (operand0.operandType == VGPU10_OPERAND_TYPE_INPUT_COVERAGE_MASK) ||
          (operand0.operandType == VGPU10_OPERAND_TYPE_STREAM));

   begin_emit_instruction(emit);
   emit_dword(emit, opcode0.value);

   emit_dword(emit, operand0.value);

   if (operand0.indexDimension == VGPU10_OPERAND_INDEX_1D) {
      /* Next token is the index of the register to declare */
      emit_dword(emit, index);
   }
   else if (operand0.indexDimension >= VGPU10_OPERAND_INDEX_2D) {
      /* Next token is the size of the register */
      emit_dword(emit, size);

      /* Followed by the index of the register */
      emit_dword(emit, index);
   }

   if (name_token.value) {
      emit_dword(emit, name_token.value);
   }

   end_emit_instruction(emit);
}


/**
 * Emit the declaration for a shader input.
 * \param opcodeType  opcode type, one of VGPU10_OPCODE_DCL_INPUTx
 * \param operandType operand type, one of VGPU10_OPERAND_TYPE_INPUT_x
 * \param dim         index dimension
 * \param index       the input register index
 * \param size        array size of the operand. In most cases, it is 1,
 *                    but for inputs to geometry shader, the array size varies
 *                    depending on the primitive type. For tessellation control
 *                    shader, the array size is the vertex count per patch.
 * \param name        one of VGPU10_NAME_x
 * \parma numComp     number of components
 * \param selMode     component selection mode
 * \param usageMask   bitfield of VGPU10_OPERAND_4_COMPONENT_MASK_x values
 * \param interpMode  interpolation mode
 */
static void
emit_input_declaration(struct svga_shader_emitter_v10 *emit,
                       VGPU10_OPCODE_TYPE opcodeType,
                       VGPU10_OPERAND_TYPE operandType,
                       VGPU10_OPERAND_INDEX_DIMENSION dim,
                       unsigned index, unsigned size,
                       VGPU10_SYSTEM_NAME name,
                       VGPU10_OPERAND_NUM_COMPONENTS numComp,
                       VGPU10_OPERAND_4_COMPONENT_SELECTION_MODE selMode,
                       unsigned usageMask,
                       VGPU10_INTERPOLATION_MODE interpMode,
                       bool addSignature,
                       SVGA3dDXSignatureSemanticName sgnName)
{
   VGPU10OpcodeToken0 opcode0;
   VGPU10OperandToken0 operand0;
   VGPU10NameToken name_token;

   assert(usageMask <= VGPU10_OPERAND_4_COMPONENT_MASK_ALL);
   assert(opcodeType == VGPU10_OPCODE_DCL_INPUT ||
          opcodeType == VGPU10_OPCODE_DCL_INPUT_SIV ||
          opcodeType == VGPU10_OPCODE_DCL_INPUT_SGV ||
          opcodeType == VGPU10_OPCODE_DCL_INPUT_PS ||
          opcodeType == VGPU10_OPCODE_DCL_INPUT_PS_SIV ||
          opcodeType == VGPU10_OPCODE_DCL_INPUT_PS_SGV);
   assert(operandType == VGPU10_OPERAND_TYPE_INPUT ||
          operandType == VGPU10_OPERAND_TYPE_INPUT_GS_INSTANCE_ID ||
          operandType == VGPU10_OPERAND_TYPE_INPUT_COVERAGE_MASK ||
          operandType == VGPU10_OPERAND_TYPE_INPUT_PRIMITIVEID ||
          operandType == VGPU10_OPERAND_TYPE_OUTPUT_CONTROL_POINT_ID ||
          operandType == VGPU10_OPERAND_TYPE_INPUT_DOMAIN_POINT ||
          operandType == VGPU10_OPERAND_TYPE_INPUT_CONTROL_POINT ||
          operandType == VGPU10_OPERAND_TYPE_INPUT_PATCH_CONSTANT ||
          operandType == VGPU10_OPERAND_TYPE_INPUT_THREAD_ID ||
          operandType == VGPU10_OPERAND_TYPE_INPUT_THREAD_GROUP_ID ||
          operandType == VGPU10_OPERAND_TYPE_INPUT_THREAD_ID_IN_GROUP);

   assert(numComp <= VGPU10_OPERAND_4_COMPONENT);
   assert(selMode <= VGPU10_OPERAND_4_COMPONENT_MASK_MODE);
   assert(dim <= VGPU10_OPERAND_INDEX_3D);
   assert(name == VGPU10_NAME_UNDEFINED ||
          name == VGPU10_NAME_POSITION ||
          name == VGPU10_NAME_INSTANCE_ID ||
          name == VGPU10_NAME_VERTEX_ID ||
          name == VGPU10_NAME_PRIMITIVE_ID ||
          name == VGPU10_NAME_IS_FRONT_FACE ||
          name == VGPU10_NAME_SAMPLE_INDEX ||
          name == VGPU10_NAME_RENDER_TARGET_ARRAY_INDEX ||
          name == VGPU10_NAME_VIEWPORT_ARRAY_INDEX);

   assert(interpMode == VGPU10_INTERPOLATION_UNDEFINED ||
          interpMode == VGPU10_INTERPOLATION_CONSTANT ||
          interpMode == VGPU10_INTERPOLATION_LINEAR ||
          interpMode == VGPU10_INTERPOLATION_LINEAR_CENTROID ||
          interpMode == VGPU10_INTERPOLATION_LINEAR_NOPERSPECTIVE ||
          interpMode == VGPU10_INTERPOLATION_LINEAR_NOPERSPECTIVE_CENTROID ||
          interpMode == VGPU10_INTERPOLATION_LINEAR_SAMPLE ||
          interpMode == VGPU10_INTERPOLATION_LINEAR_NOPERSPECTIVE_SAMPLE);

   check_register_index(emit, opcodeType, index);

   opcode0.value = operand0.value = name_token.value = 0;

   opcode0.opcodeType = opcodeType;
   opcode0.interpolationMode = interpMode;

   operand0.operandType = operandType;
   operand0.numComponents = numComp;
   operand0.selectionMode = selMode;
   operand0.mask = usageMask;
   operand0.indexDimension = dim;
   operand0.index0Representation = VGPU10_OPERAND_INDEX_IMMEDIATE32;
   if (dim == VGPU10_OPERAND_INDEX_2D)
      operand0.index1Representation = VGPU10_OPERAND_INDEX_IMMEDIATE32;

   name_token.name = name;

   emit_decl_instruction(emit, opcode0, operand0, name_token, index, size);

   if (addSignature) {
      struct svga_shader_signature *sgn = &emit->signature;
      if (operandType == VGPU10_OPERAND_TYPE_INPUT_PATCH_CONSTANT) {
         /* Set patch constant signature */
         SVGA3dDXShaderSignatureEntry *sgnEntry =
            &sgn->patchConstants[sgn->header.numPatchConstantSignatures++];
         set_shader_signature_entry(sgnEntry, index,
                                    sgnName, usageMask,
                                    SVGADX_SIGNATURE_REGISTER_COMPONENT_UNKNOWN,
                                    SVGADX_SIGNATURE_MIN_PRECISION_DEFAULT);

      } else if (operandType == VGPU10_OPERAND_TYPE_INPUT ||
                 operandType == VGPU10_OPERAND_TYPE_INPUT_CONTROL_POINT) {
         /* Set input signature */
         SVGA3dDXShaderSignatureEntry *sgnEntry =
            &sgn->inputs[sgn->header.numInputSignatures++];
         set_shader_signature_entry(sgnEntry, index,
                                    sgnName, usageMask,
                                    SVGADX_SIGNATURE_REGISTER_COMPONENT_UNKNOWN,
                                    SVGADX_SIGNATURE_MIN_PRECISION_DEFAULT);
      }
   }

   if (emit->index_range.required) {
      /* Here, index_range declaration is only applicable for opcodeType
       * VGPU10_OPCODE_DCL_INPUT and VGPU10_OPCODE_DCL_INPUT_PS and
       * for operandType VGPU10_OPERAND_TYPE_INPUT,
       * VGPU10_OPERAND_TYPE_INPUT_CONTROL_POINT and
       * VGPU10_OPERAND_TYPE_INPUT_PATCH_CONSTANT.
       */
      if ((opcodeType != VGPU10_OPCODE_DCL_INPUT &&
           opcodeType != VGPU10_OPCODE_DCL_INPUT_PS) ||
          (operandType != VGPU10_OPERAND_TYPE_INPUT &&
           operandType != VGPU10_OPERAND_TYPE_INPUT_CONTROL_POINT &&
           operandType != VGPU10_OPERAND_TYPE_INPUT_PATCH_CONSTANT)) {
         if (emit->index_range.start_index != INVALID_INDEX) {
            emit_index_range_declaration(emit);
         }
         return;
      }

      if (emit->index_range.operandType == VGPU10_NUM_OPERANDS) {
         /* Need record new index_range */
         emit->index_range.count = 1;
         emit->index_range.operandType = operandType;
         emit->index_range.start_index = index;
         emit->index_range.size = size;
         emit->index_range.dim = dim;
      }
      else if (index !=
               (emit->index_range.start_index + emit->index_range.count) ||
               emit->index_range.operandType != operandType) {
         /* Input index is not contiguous with index range or operandType is
          * different from index range's operandType. We need to emit current
          * index_range first and then start recording next index range.
          */
         emit_index_range_declaration(emit);

         emit->index_range.count = 1;
         emit->index_range.operandType = operandType;
         emit->index_range.start_index = index;
         emit->index_range.size = size;
         emit->index_range.dim = dim;
      }
      else if (emit->index_range.operandType == operandType) {
         /* Since input index is contiguous with index range and operandType
          * is same as index range's operandType, increment index range count.
          */
         emit->index_range.count++;
      }
   }
}


/**
 * Emit the declaration for a shader output.
 * \param type  one of VGPU10_OPCODE_DCL_OUTPUTx
 * \param index  the output register index
 * \param name  one of VGPU10_NAME_x
 * \param usageMask  bitfield of VGPU10_OPERAND_4_COMPONENT_MASK_x values
 */
static void
emit_output_declaration(struct svga_shader_emitter_v10 *emit,
                        VGPU10_OPCODE_TYPE type, unsigned index,
                        VGPU10_SYSTEM_NAME name,
                        unsigned writemask,
                        bool addSignature,
                        SVGA3dDXSignatureSemanticName sgnName)
{
   VGPU10OpcodeToken0 opcode0;
   VGPU10OperandToken0 operand0;
   VGPU10NameToken name_token;

   assert(writemask <= VGPU10_OPERAND_4_COMPONENT_MASK_ALL);
   assert(type == VGPU10_OPCODE_DCL_OUTPUT ||
          type == VGPU10_OPCODE_DCL_OUTPUT_SGV ||
          type == VGPU10_OPCODE_DCL_OUTPUT_SIV);
   assert(name == VGPU10_NAME_UNDEFINED ||
          name == VGPU10_NAME_POSITION ||
          name == VGPU10_NAME_PRIMITIVE_ID ||
          name == VGPU10_NAME_RENDER_TARGET_ARRAY_INDEX ||
          name == VGPU10_NAME_VIEWPORT_ARRAY_INDEX ||
          name == VGPU10_NAME_CLIP_DISTANCE);

   check_register_index(emit, type, index);

   opcode0.value = operand0.value = name_token.value = 0;

   opcode0.opcodeType = type;
   operand0.operandType = VGPU10_OPERAND_TYPE_OUTPUT;
   operand0.numComponents = VGPU10_OPERAND_4_COMPONENT;
   operand0.selectionMode = VGPU10_OPERAND_4_COMPONENT_MASK_MODE;
   operand0.mask = writemask;
   operand0.indexDimension = VGPU10_OPERAND_INDEX_1D;
   operand0.index0Representation = VGPU10_OPERAND_INDEX_IMMEDIATE32;

   name_token.name = name;

   emit_decl_instruction(emit, opcode0, operand0, name_token, index, 1);

   /* Capture output signature */
   if (addSignature) {
      struct svga_shader_signature *sgn = &emit->signature;
      SVGA3dDXShaderSignatureEntry *sgnEntry =
         &sgn->outputs[sgn->header.numOutputSignatures++];
      set_shader_signature_entry(sgnEntry, index,
                                 sgnName, writemask,
                                 SVGADX_SIGNATURE_REGISTER_COMPONENT_UNKNOWN,
                                 SVGADX_SIGNATURE_MIN_PRECISION_DEFAULT);
   }

   if (emit->index_range.required) {
      /* Here, index_range declaration is only applicable for opcodeType
       * VGPU10_OPCODE_DCL_OUTPUT and for operandType
       * VGPU10_OPERAND_TYPE_OUTPUT.
       */
      if (type != VGPU10_OPCODE_DCL_OUTPUT) {
         if (emit->index_range.start_index != INVALID_INDEX) {
            emit_index_range_declaration(emit);
         }
         return;
      }

      if (emit->index_range.operandType == VGPU10_NUM_OPERANDS) {
         /* Need record new index_range */
         emit->index_range.count = 1;
         emit->index_range.operandType = VGPU10_OPERAND_TYPE_OUTPUT;
         emit->index_range.start_index = index;
         emit->index_range.size = 1;
         emit->index_range.dim = VGPU10_OPERAND_INDEX_1D;
      }
      else if (index !=
               (emit->index_range.start_index + emit->index_range.count)) {
         /* Output index is not contiguous with index range. We need to
          * emit current index_range first and then start recording next
          * index range.
          */
         emit_index_range_declaration(emit);

         emit->index_range.count = 1;
         emit->index_range.operandType = VGPU10_OPERAND_TYPE_OUTPUT;
         emit->index_range.start_index = index;
         emit->index_range.size = 1;
         emit->index_range.dim = VGPU10_OPERAND_INDEX_1D;
      }
      else {
         /* Since output index is contiguous with index range, increment
          * index range count.
          */
         emit->index_range.count++;
      }
   }
}


/**
 * Emit the declaration for the fragment depth output.
 */
static void
emit_fragdepth_output_declaration(struct svga_shader_emitter_v10 *emit)
{
   VGPU10OpcodeToken0 opcode0;
   VGPU10OperandToken0 operand0;
   VGPU10NameToken name_token;

   assert(emit->unit == PIPE_SHADER_FRAGMENT);

   opcode0.value = operand0.value = name_token.value = 0;

   opcode0.opcodeType = VGPU10_OPCODE_DCL_OUTPUT;
   operand0.operandType = VGPU10_OPERAND_TYPE_OUTPUT_DEPTH;
   operand0.numComponents = VGPU10_OPERAND_1_COMPONENT;
   operand0.indexDimension = VGPU10_OPERAND_INDEX_0D;
   operand0.mask = 0;

   emit_decl_instruction(emit, opcode0, operand0, name_token, 0, 1);
}


/**
 * Emit the declaration for the fragment sample mask/coverage output.
 */
static void
emit_samplemask_output_declaration(struct svga_shader_emitter_v10 *emit)
{
   VGPU10OpcodeToken0 opcode0;
   VGPU10OperandToken0 operand0;
   VGPU10NameToken name_token;

   assert(emit->unit == PIPE_SHADER_FRAGMENT);
   assert(emit->version >= 41);

   opcode0.value = operand0.value = name_token.value = 0;

   opcode0.opcodeType = VGPU10_OPCODE_DCL_OUTPUT;
   operand0.operandType = VGPU10_OPERAND_TYPE_OUTPUT_COVERAGE_MASK;
   operand0.numComponents = VGPU10_OPERAND_0_COMPONENT;
   operand0.indexDimension = VGPU10_OPERAND_INDEX_0D;
   operand0.mask = 0;

   emit_decl_instruction(emit, opcode0, operand0, name_token, 0, 1);
}


/**
 * Emit output declarations for fragment shader.
 */
static void
emit_fs_output_declarations(struct svga_shader_emitter_v10 *emit)
{
   unsigned int i;

   for (i = 0; i < emit->info.num_outputs; i++) {
      /*const unsigned usage_mask = emit->info.output_usage_mask[i];*/
      const enum tgsi_semantic semantic_name =
         emit->info.output_semantic_name[i];
      const unsigned semantic_index = emit->info.output_semantic_index[i];
      unsigned index = i;

      if (semantic_name == TGSI_SEMANTIC_COLOR) {
         assert(semantic_index < ARRAY_SIZE(emit->fs.color_out_index));

         emit->fs.color_out_index[semantic_index] = index;

         emit->fs.num_color_outputs = MAX2(emit->fs.num_color_outputs,
                                              index + 1);

         /* The semantic index is the shader's color output/buffer index */
         emit_output_declaration(emit,
                                 VGPU10_OPCODE_DCL_OUTPUT, semantic_index,
                                 VGPU10_NAME_UNDEFINED,
                                 VGPU10_OPERAND_4_COMPONENT_MASK_ALL,
                                 true,
                                 map_tgsi_semantic_to_sgn_name(semantic_name));

         if (semantic_index == 0) {
            if (emit->key.fs.write_color0_to_n_cbufs > 1) {
               /* Emit declarations for the additional color outputs
                * for broadcasting.
                */
               unsigned j;
               for (j = 1; j < emit->key.fs.write_color0_to_n_cbufs; j++) {
                  /* Allocate a new output index */
                  unsigned idx = emit->info.num_outputs + j - 1;
                  emit->fs.color_out_index[j] = idx;
                  emit_output_declaration(emit,
                                        VGPU10_OPCODE_DCL_OUTPUT, idx,
                                        VGPU10_NAME_UNDEFINED,
                                        VGPU10_OPERAND_4_COMPONENT_MASK_ALL,
                                        true,
                                        map_tgsi_semantic_to_sgn_name(semantic_name));
                  emit->info.output_semantic_index[idx] = j;
               }

               emit->fs.num_color_outputs =
                     emit->key.fs.write_color0_to_n_cbufs;
            }
         }
      }
      else if (semantic_name == TGSI_SEMANTIC_POSITION) {
         /* Fragment depth output */
         emit_fragdepth_output_declaration(emit);
      }
      else if (semantic_name == TGSI_SEMANTIC_SAMPLEMASK) {
         /* Sample mask output */
         emit_samplemask_output_declaration(emit);
      }
      else {
         assert(!"Bad output semantic name");
      }
   }
}


/**
 * Emit common output declaration for vertex processing.
 */
static void
emit_vertex_output_declaration(struct svga_shader_emitter_v10 *emit,
                               unsigned index, unsigned writemask,
                               bool addSignature)
{
   const enum tgsi_semantic semantic_name =
         emit->info.output_semantic_name[index];
   const unsigned semantic_index = emit->info.output_semantic_index[index];
   unsigned name, type;
   unsigned final_mask = VGPU10_OPERAND_4_COMPONENT_MASK_ALL;

   assert(emit->unit != PIPE_SHADER_FRAGMENT &&
          emit->unit != PIPE_SHADER_COMPUTE);

   switch (semantic_name) {
   case TGSI_SEMANTIC_POSITION:
      if (emit->unit == PIPE_SHADER_TESS_CTRL) {
         /* position will be declared in control point only */
         assert(emit->tcs.control_point_phase);
         type = VGPU10_OPCODE_DCL_OUTPUT;
         name = VGPU10_NAME_UNDEFINED;
         emit_output_declaration(emit, type, index, name, final_mask, true,
                                 SVGADX_SIGNATURE_SEMANTIC_NAME_UNDEFINED);
         return;
      }
      else {
         type = VGPU10_OPCODE_DCL_OUTPUT_SIV;
         name = VGPU10_NAME_POSITION;
      }
      /* Save the index of the vertex position output register */
      emit->vposition.out_index = index;
      break;
   case TGSI_SEMANTIC_CLIPDIST:
      type = VGPU10_OPCODE_DCL_OUTPUT_SIV;
      name = VGPU10_NAME_CLIP_DISTANCE;
      /* save the starting index of the clip distance output register */
      if (semantic_index == 0)
         emit->clip_dist_out_index = index;
      final_mask = apply_clip_plane_mask(emit, writemask, semantic_index);
      if (final_mask == 0x0)
         return; /* discard this do-nothing declaration */
      break;
   case TGSI_SEMANTIC_CLIPVERTEX:
      type = VGPU10_OPCODE_DCL_OUTPUT;
      name = VGPU10_NAME_UNDEFINED;
      emit->clip_vertex_out_index = index;
      break;
   default:
      /* generic output */
      type = VGPU10_OPCODE_DCL_OUTPUT;
      name = VGPU10_NAME_UNDEFINED;
   }

   emit_output_declaration(emit, type, index, name, final_mask, addSignature,
                           map_tgsi_semantic_to_sgn_name(semantic_name));
}


/**
 * Emit declaration for outputs in vertex shader.
 */
static void
emit_vs_output_declarations(struct svga_shader_emitter_v10 *emit)
{
   unsigned i;
   for (i = 0; i < emit->info.num_outputs; i++) {
      emit_vertex_output_declaration(emit, i, emit->output_usage_mask[i], true);
   }
}


/**
 * A helper function to determine the writemask for an output
 * for the specified stream.
 */
static unsigned
output_writemask_for_stream(unsigned stream, uint8_t output_streams,
                            uint8_t output_usagemask)
{
   unsigned i;
   unsigned writemask = 0;

   for (i = 0; i < 4; i++) {
      if ((output_streams & 0x3) == stream)
         writemask |= (VGPU10_OPERAND_4_COMPONENT_MASK_X << i);
      output_streams >>= 2;
   }
   return writemask & output_usagemask;
}


/**
 * Emit declaration for outputs in geometry shader.
 */
static void
emit_gs_output_declarations(struct svga_shader_emitter_v10 *emit)
{
   unsigned i;
   VGPU10OpcodeToken0 opcode0;
   unsigned numStreamsSupported = 1;
   int s;

   if (emit->version >= 50) {
      numStreamsSupported = ARRAY_SIZE(emit->info.num_stream_output_components);
   }

   /**
    * Start emitting from the last stream first, so we end with
    * stream 0, so any of the auxiliary output declarations will
    * go to stream 0.
    */
   for (s = numStreamsSupported-1; s >= 0; s--) { 

      if (emit->info.num_stream_output_components[s] == 0)
         continue;

      if (emit->version >= 50) {
         /* DCL_STREAM stream */
         begin_emit_instruction(emit);
         emit_opcode(emit, VGPU10_OPCODE_DCL_STREAM, false);
         emit_stream_register(emit, s);
         end_emit_instruction(emit);
      }

      /* emit output primitive topology declaration */
      opcode0.value = 0;
      opcode0.opcodeType = VGPU10_OPCODE_DCL_GS_OUTPUT_PRIMITIVE_TOPOLOGY;
      opcode0.primitiveTopology = emit->gs.prim_topology;
      emit_property_instruction(emit, opcode0, 0, 0);

      for (i = 0; i < emit->info.num_outputs; i++) {
         unsigned writemask;

         /* find out the writemask for this stream */
         writemask = output_writemask_for_stream(s, emit->info.output_streams[i],
                                                 emit->output_usage_mask[i]);

         if (writemask) {
            enum tgsi_semantic semantic_name =
               emit->info.output_semantic_name[i];

            /* TODO: Still need to take care of a special case where a
             *       single varying spans across multiple output registers.
             */
            switch(semantic_name) {
            case TGSI_SEMANTIC_PRIMID:
               emit_output_declaration(emit,
                                       VGPU10_OPCODE_DCL_OUTPUT_SGV, i,
                                       VGPU10_NAME_PRIMITIVE_ID,
                                       VGPU10_OPERAND_4_COMPONENT_MASK_ALL,
                                       false,
                                       map_tgsi_semantic_to_sgn_name(semantic_name));
               break;
            case TGSI_SEMANTIC_LAYER:
               emit_output_declaration(emit,
                                       VGPU10_OPCODE_DCL_OUTPUT_SIV, i,
                                       VGPU10_NAME_RENDER_TARGET_ARRAY_INDEX,
                                       VGPU10_OPERAND_4_COMPONENT_MASK_X,
                                       false,
                                       map_tgsi_semantic_to_sgn_name(semantic_name));
               break;
            case TGSI_SEMANTIC_VIEWPORT_INDEX:
               emit_output_declaration(emit,
                                       VGPU10_OPCODE_DCL_OUTPUT_SIV, i,
                                       VGPU10_NAME_VIEWPORT_ARRAY_INDEX,
                                       VGPU10_OPERAND_4_COMPONENT_MASK_X,
                                       false,
                                       map_tgsi_semantic_to_sgn_name(semantic_name));
               emit->gs.viewport_index_out_index = i;
               break;
            default:
               emit_vertex_output_declaration(emit, i, writemask, false);
            }
         }
      }
   }

   /* For geometry shader outputs, it is possible the same register is
    * declared multiple times for different streams. So to avoid
    * redundant signature entries, geometry shader output signature is done
    * outside of the declaration.
    */
   struct svga_shader_signature *sgn = &emit->signature;
   SVGA3dDXShaderSignatureEntry *sgnEntry;

   for (i = 0; i < emit->info.num_outputs; i++) {
      if (emit->output_usage_mask[i]) {
         enum tgsi_semantic sem_name = emit->info.output_semantic_name[i];

         sgnEntry = &sgn->outputs[sgn->header.numOutputSignatures++];
         set_shader_signature_entry(sgnEntry, i,
                                    map_tgsi_semantic_to_sgn_name(sem_name),
                                    emit->output_usage_mask[i],
                                    SVGADX_SIGNATURE_REGISTER_COMPONENT_UNKNOWN,
                                    SVGADX_SIGNATURE_MIN_PRECISION_DEFAULT);
      }
   }
}


/**
 * Emit the declaration for the tess inner/outer output.
 * \param opcodeType either VGPU10_OPCODE_DCL_OUTPUT_SIV or _INPUT_SIV
 * \param operandType either VGPU10_OPERAND_TYPE_OUTPUT or _INPUT
 * \param name VGPU10_NAME_FINAL_*_TESSFACTOR value
 */
static void
emit_tesslevel_declaration(struct svga_shader_emitter_v10 *emit,
                           unsigned index, unsigned opcodeType,
                           unsigned operandType, VGPU10_SYSTEM_NAME name,
                           SVGA3dDXSignatureSemanticName sgnName)
{
   VGPU10OpcodeToken0 opcode0;
   VGPU10OperandToken0 operand0;
   VGPU10NameToken name_token;

   assert(emit->version >= 50);
   assert(name >= VGPU10_NAME_FINAL_QUAD_U_EQ_0_EDGE_TESSFACTOR ||
          (emit->key.tcs.prim_mode == MESA_PRIM_LINES &&
           name == VGPU10_NAME_UNDEFINED));
   assert(name <= VGPU10_NAME_FINAL_LINE_DENSITY_TESSFACTOR);

   assert(operandType == VGPU10_OPERAND_TYPE_OUTPUT ||
          operandType == VGPU10_OPERAND_TYPE_INPUT_PATCH_CONSTANT);

   opcode0.value = operand0.value = name_token.value = 0;

   opcode0.opcodeType = opcodeType;
   operand0.operandType = operandType;
   operand0.numComponents = VGPU10_OPERAND_4_COMPONENT;
   operand0.indexDimension = VGPU10_OPERAND_INDEX_1D;
   operand0.mask = VGPU10_OPERAND_4_COMPONENT_MASK_X;
   operand0.selectionMode = VGPU10_OPERAND_4_COMPONENT_MASK_MODE;
   operand0.index0Representation = VGPU10_OPERAND_INDEX_IMMEDIATE32;

   name_token.name = name;
   emit_decl_instruction(emit, opcode0, operand0, name_token, index, 1);

   /* Capture patch constant signature */
   struct svga_shader_signature *sgn = &emit->signature;
   SVGA3dDXShaderSignatureEntry *sgnEntry =
      &sgn->patchConstants[sgn->header.numPatchConstantSignatures++];
   set_shader_signature_entry(sgnEntry, index,
                              sgnName, VGPU10_OPERAND_4_COMPONENT_MASK_X,
                              SVGADX_SIGNATURE_REGISTER_COMPONENT_UNKNOWN,
                              SVGADX_SIGNATURE_MIN_PRECISION_DEFAULT);
}


/**
 * Emit output declarations for tessellation control shader.
 */
static void
emit_tcs_output_declarations(struct svga_shader_emitter_v10 *emit)
{
   unsigned int i;
   unsigned outputIndex = emit->num_outputs;
   struct svga_shader_signature *sgn = &emit->signature;

   /**
    * Initialize patch_generic_out_count so it won't be counted twice
    * since this function is called twice, one for control point phase
    * and another time for patch constant phase.
    */
   emit->tcs.patch_generic_out_count = 0;

   for (i = 0; i < emit->info.num_outputs; i++) {
      unsigned index = i;
      const enum tgsi_semantic semantic_name =
         emit->info.output_semantic_name[i];

      switch (semantic_name) {
      case TGSI_SEMANTIC_TESSINNER:
         emit->tcs.inner.tgsi_index = i;

         /* skip per-patch output declarations in control point phase */
         if (emit->tcs.control_point_phase)
            break;

         emit->tcs.inner.out_index = outputIndex;
         switch (emit->key.tcs.prim_mode) {
         case MESA_PRIM_QUADS:
            emit_tesslevel_declaration(emit, outputIndex++,
               VGPU10_OPCODE_DCL_OUTPUT_SIV, VGPU10_OPERAND_TYPE_OUTPUT,
               VGPU10_NAME_FINAL_QUAD_U_INSIDE_TESSFACTOR,
               SVGADX_SIGNATURE_SEMANTIC_NAME_FINAL_QUAD_U_INSIDE_TESSFACTOR);

            emit_tesslevel_declaration(emit, outputIndex++,
               VGPU10_OPCODE_DCL_OUTPUT_SIV, VGPU10_OPERAND_TYPE_OUTPUT,
               VGPU10_NAME_FINAL_QUAD_V_INSIDE_TESSFACTOR,
               SVGADX_SIGNATURE_SEMANTIC_NAME_FINAL_QUAD_V_INSIDE_TESSFACTOR);
            break;
         case MESA_PRIM_TRIANGLES:
            emit_tesslevel_declaration(emit, outputIndex++,
               VGPU10_OPCODE_DCL_OUTPUT_SIV, VGPU10_OPERAND_TYPE_OUTPUT,
               VGPU10_NAME_FINAL_TRI_INSIDE_TESSFACTOR,
               SVGADX_SIGNATURE_SEMANTIC_NAME_FINAL_TRI_INSIDE_TESSFACTOR);
            break;
         case MESA_PRIM_LINES:
            break;
         default:
            debug_printf("Unsupported primitive type");
         }
         break;

      case TGSI_SEMANTIC_TESSOUTER:
         emit->tcs.outer.tgsi_index = i;

         /* skip per-patch output declarations in control point phase */
         if (emit->tcs.control_point_phase)
            break;

         emit->tcs.outer.out_index = outputIndex;
         switch (emit->key.tcs.prim_mode) {
         case MESA_PRIM_QUADS:
            for (int j = 0; j < 4; j++) {
               emit_tesslevel_declaration(emit, outputIndex++,
                  VGPU10_OPCODE_DCL_OUTPUT_SIV, VGPU10_OPERAND_TYPE_OUTPUT,
                  VGPU10_NAME_FINAL_QUAD_U_EQ_0_EDGE_TESSFACTOR + j,
                  SVGADX_SIGNATURE_SEMANTIC_NAME_FINAL_QUAD_U_EQ_0_EDGE_TESSFACTOR + j);
            }
            break;
         case MESA_PRIM_TRIANGLES:
            for (int j = 0; j < 3; j++) {
               emit_tesslevel_declaration(emit, outputIndex++,
                  VGPU10_OPCODE_DCL_OUTPUT_SIV, VGPU10_OPERAND_TYPE_OUTPUT,
                  VGPU10_NAME_FINAL_TRI_U_EQ_0_EDGE_TESSFACTOR + j,
                  SVGADX_SIGNATURE_SEMANTIC_NAME_FINAL_TRI_U_EQ_0_EDGE_TESSFACTOR + j);
            }
            break;
         case MESA_PRIM_LINES:
            for (int j = 0; j < 2; j++) {
               emit_tesslevel_declaration(emit, outputIndex++,
                  VGPU10_OPCODE_DCL_OUTPUT_SIV, VGPU10_OPERAND_TYPE_OUTPUT,
                  VGPU10_NAME_FINAL_LINE_DETAIL_TESSFACTOR + j,
                  SVGADX_SIGNATURE_SEMANTIC_NAME_FINAL_LINE_DETAIL_TESSFACTOR + j);
            }
            break;
         default:
            debug_printf("Unsupported primitive type");
         }
         break;

      case TGSI_SEMANTIC_PATCH:
         if (emit->tcs.patch_generic_out_index == INVALID_INDEX)
            emit->tcs.patch_generic_out_index= i;
         emit->tcs.patch_generic_out_count++;

         /* skip per-patch output declarations in control point phase */
         if (emit->tcs.control_point_phase)
            break;

         emit_output_declaration(emit, VGPU10_OPCODE_DCL_OUTPUT, index,
                                 VGPU10_NAME_UNDEFINED,
                                 VGPU10_OPERAND_4_COMPONENT_MASK_ALL,
                                 false,
                                 map_tgsi_semantic_to_sgn_name(semantic_name));

         SVGA3dDXShaderSignatureEntry *sgnEntry =
            &sgn->patchConstants[sgn->header.numPatchConstantSignatures++];
         set_shader_signature_entry(sgnEntry, index,
                                    map_tgsi_semantic_to_sgn_name(semantic_name),
                                    VGPU10_OPERAND_4_COMPONENT_MASK_ALL,
                                    SVGADX_SIGNATURE_REGISTER_COMPONENT_UNKNOWN,
                                    SVGADX_SIGNATURE_MIN_PRECISION_DEFAULT);

         break;

      default:
         /* save the starting index of control point outputs */
         if (emit->tcs.control_point_out_index == INVALID_INDEX)
            emit->tcs.control_point_out_index = i;
         emit->tcs.control_point_out_count++;

         /* skip control point output declarations in patch constant phase */
         if (!emit->tcs.control_point_phase)
            break;

         emit_vertex_output_declaration(emit, i, emit->output_usage_mask[i],
                                        true);

      }
   }

   if (emit->tcs.control_point_phase) {
      /**
       * Add missing control point output in control point phase.
       */
      if (emit->tcs.control_point_out_index == INVALID_INDEX) {
         /* use register index after tessellation factors */
         switch (emit->key.tcs.prim_mode) {
         case MESA_PRIM_QUADS:
            emit->tcs.control_point_out_index = outputIndex + 6;
            break;
         case MESA_PRIM_TRIANGLES:
            emit->tcs.control_point_out_index = outputIndex + 4;
            break;
         default:
            emit->tcs.control_point_out_index = outputIndex + 2;
            break;
         }
         emit->tcs.control_point_out_count++;
         emit_output_declaration(emit, VGPU10_OPCODE_DCL_OUTPUT_SIV,
                                 emit->tcs.control_point_out_index,
                                 VGPU10_NAME_POSITION,
                                 VGPU10_OPERAND_4_COMPONENT_MASK_ALL,
                                 true,
                                 SVGADX_SIGNATURE_SEMANTIC_NAME_POSITION);

         /* If tcs does not output any control point output,
          * we can end the hull shader control point phase here
          * after emitting the default control point output.
          */
         emit->skip_instruction = true;
      }
   }
   else {
      if (emit->tcs.outer.out_index == INVALID_INDEX) {
         /* since the TCS did not declare out outer tess level output register,
          * we declare it here for patch constant phase only.
          */
         emit->tcs.outer.out_index = outputIndex;
         if (emit->key.tcs.prim_mode == MESA_PRIM_QUADS) {
            for (int i = 0; i < 4; i++) {
               emit_tesslevel_declaration(emit, outputIndex++,
                  VGPU10_OPCODE_DCL_OUTPUT_SIV, VGPU10_OPERAND_TYPE_OUTPUT,
                  VGPU10_NAME_FINAL_QUAD_U_EQ_0_EDGE_TESSFACTOR + i,
                  SVGADX_SIGNATURE_SEMANTIC_NAME_FINAL_QUAD_U_EQ_0_EDGE_TESSFACTOR + i);
            }
         }
         else if (emit->key.tcs.prim_mode == MESA_PRIM_TRIANGLES) {
            for (int i = 0; i < 3; i++) {
               emit_tesslevel_declaration(emit, outputIndex++,
                  VGPU10_OPCODE_DCL_OUTPUT_SIV, VGPU10_OPERAND_TYPE_OUTPUT,
                  VGPU10_NAME_FINAL_TRI_U_EQ_0_EDGE_TESSFACTOR + i,
                  SVGADX_SIGNATURE_SEMANTIC_NAME_FINAL_TRI_U_EQ_0_EDGE_TESSFACTOR + i);
            }
         }
      }

      if (emit->tcs.inner.out_index == INVALID_INDEX) {
         /* since the TCS did not declare out inner tess level output register,
          * we declare it here
          */
         emit->tcs.inner.out_index = outputIndex;
         if (emit->key.tcs.prim_mode == MESA_PRIM_QUADS) {
            emit_tesslevel_declaration(emit, outputIndex++,
               VGPU10_OPCODE_DCL_OUTPUT_SIV, VGPU10_OPERAND_TYPE_OUTPUT,
               VGPU10_NAME_FINAL_QUAD_U_INSIDE_TESSFACTOR,
               SVGADX_SIGNATURE_SEMANTIC_NAME_FINAL_QUAD_U_INSIDE_TESSFACTOR);
            emit_tesslevel_declaration(emit, outputIndex++,
               VGPU10_OPCODE_DCL_OUTPUT_SIV, VGPU10_OPERAND_TYPE_OUTPUT,
               VGPU10_NAME_FINAL_QUAD_V_INSIDE_TESSFACTOR,
               SVGADX_SIGNATURE_SEMANTIC_NAME_FINAL_QUAD_V_INSIDE_TESSFACTOR);
         }
         else if (emit->key.tcs.prim_mode == MESA_PRIM_TRIANGLES) {
            emit_tesslevel_declaration(emit, outputIndex++,
               VGPU10_OPCODE_DCL_OUTPUT_SIV, VGPU10_OPERAND_TYPE_OUTPUT,
               VGPU10_NAME_FINAL_TRI_INSIDE_TESSFACTOR,
               SVGADX_SIGNATURE_SEMANTIC_NAME_FINAL_TRI_INSIDE_TESSFACTOR);
         }
      }
   }
   emit->num_outputs = outputIndex;
}


/**
 * Emit output declarations for tessellation evaluation shader.
 */
static void
emit_tes_output_declarations(struct svga_shader_emitter_v10 *emit)
{
   unsigned int i;

   for (i = 0; i < emit->info.num_outputs; i++) {
      emit_vertex_output_declaration(emit, i, emit->output_usage_mask[i], true);
   }
}


/**
 * Emit the declaration for a system value input/output.
 */
static void
emit_system_value_declaration(struct svga_shader_emitter_v10 *emit,
                              enum tgsi_semantic semantic_name, unsigned index)
{
   switch (semantic_name) {
   case TGSI_SEMANTIC_INSTANCEID:
      index = alloc_system_value_index(emit, index);
      emit_input_declaration(emit, VGPU10_OPCODE_DCL_INPUT_SIV,
                             VGPU10_OPERAND_TYPE_INPUT,
                             VGPU10_OPERAND_INDEX_1D,
                             index, 1,
                             VGPU10_NAME_INSTANCE_ID,
                             VGPU10_OPERAND_4_COMPONENT,
                             VGPU10_OPERAND_4_COMPONENT_MASK_MODE,
                             VGPU10_OPERAND_4_COMPONENT_MASK_X,
                             VGPU10_INTERPOLATION_UNDEFINED, true,
                             map_tgsi_semantic_to_sgn_name(semantic_name));
      break;
   case TGSI_SEMANTIC_VERTEXID:
      emit->vs.vertex_id_sys_index = index;
      index = alloc_system_value_index(emit, index);
      emit_input_declaration(emit, VGPU10_OPCODE_DCL_INPUT_SIV,
                             VGPU10_OPERAND_TYPE_INPUT,
                             VGPU10_OPERAND_INDEX_1D,
                             index, 1,
                             VGPU10_NAME_VERTEX_ID,
                             VGPU10_OPERAND_4_COMPONENT,
                             VGPU10_OPERAND_4_COMPONENT_MASK_MODE,
                             VGPU10_OPERAND_4_COMPONENT_MASK_X,
                             VGPU10_INTERPOLATION_UNDEFINED, true,
                             map_tgsi_semantic_to_sgn_name(semantic_name));
      break;
   case TGSI_SEMANTIC_SAMPLEID:
      assert(emit->unit == PIPE_SHADER_FRAGMENT);
      emit->fs.sample_id_sys_index = index;
      index = alloc_system_value_index(emit, index);
      emit_input_declaration(emit, VGPU10_OPCODE_DCL_INPUT_PS_SIV,
                             VGPU10_OPERAND_TYPE_INPUT,
                             VGPU10_OPERAND_INDEX_1D,
                             index, 1,
                             VGPU10_NAME_SAMPLE_INDEX,
                             VGPU10_OPERAND_4_COMPONENT,
                             VGPU10_OPERAND_4_COMPONENT_MASK_MODE,
                             VGPU10_OPERAND_4_COMPONENT_MASK_X,
                             VGPU10_INTERPOLATION_CONSTANT, true,
                             map_tgsi_semantic_to_sgn_name(semantic_name));
      break;
   case TGSI_SEMANTIC_SAMPLEPOS:
      /* This system value contains the position of the current sample
       * when using per-sample shading.  We implement this by calling
       * the VGPU10_OPCODE_SAMPLE_POS instruction with the current sample
       * index as the argument.  See emit_sample_position_instructions().
       */
      assert(emit->version >= 41);
      emit->fs.sample_pos_sys_index = index;
      index = alloc_system_value_index(emit, index);
      break;
   case TGSI_SEMANTIC_INVOCATIONID:
      /* Note: invocation id input is mapped to different register depending
       * on the shader type. In GS, it will be mapped to vGSInstanceID#.
       * In TCS, it will be mapped to vOutputControlPointID#.
       * Since in both cases, the mapped name is unique rather than
       * just a generic input name ("v#"), so there is no need to remap
       * the index value.
       */
      assert(emit->unit == PIPE_SHADER_GEOMETRY ||
             emit->unit == PIPE_SHADER_TESS_CTRL);
      assert(emit->version >= 50);

      if (emit->unit == PIPE_SHADER_GEOMETRY) {
         emit->gs.invocation_id_sys_index = index;
         emit_input_declaration(emit, VGPU10_OPCODE_DCL_INPUT,
                                VGPU10_OPERAND_TYPE_INPUT_GS_INSTANCE_ID,
                                VGPU10_OPERAND_INDEX_0D,
                                index, 1,
                                VGPU10_NAME_UNDEFINED,
                                VGPU10_OPERAND_0_COMPONENT,
                                VGPU10_OPERAND_4_COMPONENT_MASK_MODE,
                                0,
                                VGPU10_INTERPOLATION_UNDEFINED, true,
                                SVGADX_SIGNATURE_SEMANTIC_NAME_UNDEFINED);
      } else if (emit->unit == PIPE_SHADER_TESS_CTRL) {
         /* The emission of the control point id will be done
          * in the control point phase in emit_hull_shader_control_point_phase().
          */
         emit->tcs.invocation_id_sys_index = index;
      }
      break;
   case TGSI_SEMANTIC_SAMPLEMASK:
      /* Note: the PS sample mask input has a unique name ("vCoverage#")
       * rather than just a generic input name ("v#") so no need to remap the
       * index value.
       */
      assert(emit->unit == PIPE_SHADER_FRAGMENT);
      assert(emit->version >= 50);
      emit->fs.sample_mask_in_sys_index = index;
      emit_input_declaration(emit, VGPU10_OPCODE_DCL_INPUT,
                             VGPU10_OPERAND_TYPE_INPUT_COVERAGE_MASK,
                             VGPU10_OPERAND_INDEX_0D,
                             index, 1,
                             VGPU10_NAME_UNDEFINED,
                             VGPU10_OPERAND_1_COMPONENT,
                             VGPU10_OPERAND_4_COMPONENT_MASK_MODE,
                             0,
                             VGPU10_INTERPOLATION_CONSTANT, true,
                             SVGADX_SIGNATURE_SEMANTIC_NAME_UNDEFINED);
      break;
   case TGSI_SEMANTIC_TESSCOORD:
      assert(emit->version >= 50);

      unsigned usageMask = 0;

      if (emit->tes.prim_mode == MESA_PRIM_TRIANGLES) {
         usageMask = VGPU10_OPERAND_4_COMPONENT_MASK_XYZ;
      }
      else if (emit->tes.prim_mode == MESA_PRIM_LINES ||
               emit->tes.prim_mode == MESA_PRIM_QUADS) {
         usageMask = VGPU10_OPERAND_4_COMPONENT_MASK_XY;
      }

      emit->tes.tesscoord_sys_index = index;
      emit_input_declaration(emit, VGPU10_OPCODE_DCL_INPUT,
                             VGPU10_OPERAND_TYPE_INPUT_DOMAIN_POINT,
                             VGPU10_OPERAND_INDEX_0D,
                             index, 1,
                             VGPU10_NAME_UNDEFINED,
                             VGPU10_OPERAND_4_COMPONENT,
                             VGPU10_OPERAND_4_COMPONENT_MASK_MODE,
                             usageMask,
                             VGPU10_INTERPOLATION_UNDEFINED, true,
                             SVGADX_SIGNATURE_SEMANTIC_NAME_UNDEFINED);
      break;
   case TGSI_SEMANTIC_TESSINNER:
      assert(emit->version >= 50);
      emit->tes.inner.tgsi_index = index;
      break;
   case TGSI_SEMANTIC_TESSOUTER:
      assert(emit->version >= 50);
      emit->tes.outer.tgsi_index = index;
      break;
   case TGSI_SEMANTIC_VERTICESIN:
      assert(emit->unit == PIPE_SHADER_TESS_CTRL);
      assert(emit->version >= 50);

      /* save the system value index */
      emit->tcs.vertices_per_patch_index = index;
      break;
   case TGSI_SEMANTIC_PRIMID:
      assert(emit->version >= 50);
      if (emit->unit == PIPE_SHADER_TESS_CTRL) {
         emit->tcs.prim_id_index = index;
      }
      else if (emit->unit == PIPE_SHADER_TESS_EVAL) {
         emit->tes.prim_id_index = index;
         emit_input_declaration(emit, VGPU10_OPCODE_DCL_INPUT,
                                VGPU10_OPERAND_TYPE_INPUT_PRIMITIVEID,
                                VGPU10_OPERAND_INDEX_0D,
                                index, 1,
                                VGPU10_NAME_UNDEFINED,
                                VGPU10_OPERAND_0_COMPONENT,
                                VGPU10_OPERAND_4_COMPONENT_MASK_MODE,
                                0,
                                VGPU10_INTERPOLATION_UNDEFINED, true,
                                map_tgsi_semantic_to_sgn_name(semantic_name));
      }
      break;
   case TGSI_SEMANTIC_THREAD_ID:
      assert(emit->unit >= PIPE_SHADER_COMPUTE);
      assert(emit->version >= 50);
      emit->cs.thread_id_index = index;
      emit_input_declaration(emit, VGPU10_OPCODE_DCL_INPUT,
                             VGPU10_OPERAND_TYPE_INPUT_THREAD_ID_IN_GROUP,
                             VGPU10_OPERAND_INDEX_0D,
                             index, 1,
                             VGPU10_NAME_UNDEFINED,
                             VGPU10_OPERAND_4_COMPONENT,
                             VGPU10_OPERAND_4_COMPONENT_MASK_MODE,
                             VGPU10_OPERAND_4_COMPONENT_MASK_ALL,
                             VGPU10_INTERPOLATION_UNDEFINED, true,
                             map_tgsi_semantic_to_sgn_name(semantic_name));
      break;
   case TGSI_SEMANTIC_BLOCK_ID:
      assert(emit->unit >= PIPE_SHADER_COMPUTE);
      assert(emit->version >= 50);
      emit->cs.block_id_index = index;
      emit_input_declaration(emit, VGPU10_OPCODE_DCL_INPUT,
                             VGPU10_OPERAND_TYPE_INPUT_THREAD_GROUP_ID,
                             VGPU10_OPERAND_INDEX_0D,
                             index, 1,
                             VGPU10_NAME_UNDEFINED,
                             VGPU10_OPERAND_4_COMPONENT,
                             VGPU10_OPERAND_4_COMPONENT_MASK_MODE,
                             VGPU10_OPERAND_4_COMPONENT_MASK_ALL,
                             VGPU10_INTERPOLATION_UNDEFINED, true,
                             map_tgsi_semantic_to_sgn_name(semantic_name));
      break;
   case TGSI_SEMANTIC_GRID_SIZE:
      assert(emit->unit == PIPE_SHADER_COMPUTE);
      assert(emit->version >= 50);
      emit->cs.grid_size.tgsi_index = index;
      break;
   default:
      debug_printf("unexpected system value semantic index %u / %s\n",
                   semantic_name, tgsi_semantic_names[semantic_name]);
   }
}

/**
 * Translate a TGSI declaration to VGPU10.
 */
static bool
emit_vgpu10_declaration(struct svga_shader_emitter_v10 *emit,
                        const struct tgsi_full_declaration *decl)
{
   switch (decl->Declaration.File) {
   case TGSI_FILE_INPUT:
      /* do nothing - see emit_input_declarations() */
      return true;

   case TGSI_FILE_OUTPUT:
      assert(decl->Range.First == decl->Range.Last);
      emit->output_usage_mask[decl->Range.First] = decl->Declaration.UsageMask;
      return true;

   case TGSI_FILE_TEMPORARY:
      /* Don't declare the temps here.  Just keep track of how many
       * and emit the declaration later.
       */
      if (decl->Declaration.Array) {
         /* Indexed temporary array.  Save the start index of the array
          * and the size of the array.
          */
         const unsigned arrayID = MIN2(decl->Array.ArrayID, MAX_TEMP_ARRAYS);
         assert(arrayID < ARRAY_SIZE(emit->temp_arrays));

         /* Save this array so we can emit the declaration for it later */
         create_temp_array(emit, arrayID, decl->Range.First,
                           decl->Range.Last - decl->Range.First + 1,
                           decl->Range.First);
      }

      /* for all temps, indexed or not, keep track of highest index */
      emit->num_shader_temps = MAX2(emit->num_shader_temps,
                                    decl->Range.Last + 1);
      return true;

   case TGSI_FILE_CONSTANT:
      /* Don't declare constants here.  Just keep track and emit later. */
      {
         unsigned constbuf = 0, num_consts;
         if (decl->Declaration.Dimension) {
            constbuf = decl->Dim.Index2D;
         }
         /* We throw an assertion here when, in fact, the shader should never
          * have linked due to constbuf index out of bounds, so we shouldn't
          * have reached here.
          */
         assert(constbuf < ARRAY_SIZE(emit->num_shader_consts));

         num_consts = MAX2(emit->num_shader_consts[constbuf],
                           decl->Range.Last + 1);

         if (num_consts > VGPU10_MAX_CONSTANT_BUFFER_ELEMENT_COUNT) {
            debug_printf("Warning: constant buffer is declared to size [%u]"
                         " but [%u] is the limit.\n",
                         num_consts,
                         VGPU10_MAX_CONSTANT_BUFFER_ELEMENT_COUNT);
            emit->register_overflow = true;
         }
         /* The linker doesn't enforce the max UBO size so we clamp here */
         emit->num_shader_consts[constbuf] =
            MIN2(num_consts, VGPU10_MAX_CONSTANT_BUFFER_ELEMENT_COUNT);
      }
      return true;

   case TGSI_FILE_IMMEDIATE:
      assert(!"TGSI_FILE_IMMEDIATE not handled yet!");
      return false;

   case TGSI_FILE_SYSTEM_VALUE:
      emit_system_value_declaration(emit, decl->Semantic.Name,
                                    decl->Range.First);
      return true;

   case TGSI_FILE_SAMPLER:
      /* Don't declare samplers here.  Just keep track and emit later. */
      emit->num_samplers = MAX2(emit->num_samplers, decl->Range.Last + 1);
      return true;

#if 0
   case TGSI_FILE_RESOURCE:
      /*opcode0.opcodeType = VGPU10_OPCODE_DCL_RESOURCE;*/
      /* XXX more, VGPU10_RETURN_TYPE_FLOAT */
      assert(!"TGSI_FILE_RESOURCE not handled yet");
      return false;
#endif

   case TGSI_FILE_ADDRESS:
      emit->num_address_regs = MAX2(emit->num_address_regs,
                                    decl->Range.Last + 1);
      return true;

   case TGSI_FILE_SAMPLER_VIEW:
      {
         unsigned unit = decl->Range.First;
         assert(decl->Range.First == decl->Range.Last);
         emit->sampler_target[unit] = decl->SamplerView.Resource;

         /* Note: we can ignore YZW return types for now */
         emit->sampler_return_type[unit] = decl->SamplerView.ReturnTypeX;
         emit->sampler_view[unit] = true;
      }
      return true;

   case TGSI_FILE_IMAGE:
      {
         unsigned unit = decl->Range.First;
         assert(decl->Range.First == decl->Range.Last);
         assert(unit < PIPE_MAX_SHADER_IMAGES);
         emit->image[unit] = decl->Image;
         emit->image_mask |= 1 << unit;
         emit->num_images++;
      }
      return true;

   case TGSI_FILE_HW_ATOMIC:
      /* Declare the atomic buffer if it is not already declared. */
      if (!(emit->atomic_bufs_mask & (1 << decl->Dim.Index2D))) {
         emit->num_atomic_bufs++;
         emit->atomic_bufs_mask |= (1 << decl->Dim.Index2D);
      }

      /* Remember the maximum atomic counter index encountered */
      emit->max_atomic_counter_index =
         MAX2(emit->max_atomic_counter_index, decl->Range.Last);
      return true;

   case TGSI_FILE_MEMORY:
      /* Record memory has been used. */
      if (emit->unit == PIPE_SHADER_COMPUTE &&
          decl->Declaration.MemType == TGSI_MEMORY_TYPE_SHARED) {
         emit->cs.shared_memory_declared = true;
      }

      return true;

   case TGSI_FILE_BUFFER:
      assert(emit->version >= 50);
      emit->num_shader_bufs++;
      return true;

   default:
      assert(!"Unexpected type of declaration");
      return false;
   }
}


/**
 * Emit input declarations for fragment shader.
 */
static void
emit_fs_input_declarations(struct svga_shader_emitter_v10 *emit)
{
   unsigned i;

   for (i = 0; i < emit->linkage.num_inputs; i++) {
      enum tgsi_semantic semantic_name = emit->info.input_semantic_name[i];
      unsigned usage_mask = emit->info.input_usage_mask[i];
      unsigned index = emit->linkage.input_map[i];
      unsigned type, interpolationMode, name;
      unsigned mask = VGPU10_OPERAND_4_COMPONENT_MASK_ALL;

      if (usage_mask == 0)
         continue;  /* register is not actually used */

      if (semantic_name == TGSI_SEMANTIC_POSITION) {
         /* fragment position input */
         type = VGPU10_OPCODE_DCL_INPUT_PS_SGV;
         interpolationMode = VGPU10_INTERPOLATION_LINEAR;
         name = VGPU10_NAME_POSITION;
         if (usage_mask & TGSI_WRITEMASK_W) {
            /* we need to replace use of 'w' with '1/w' */
            emit->fs.fragcoord_input_index = i;
         }
      }
      else if (semantic_name == TGSI_SEMANTIC_FACE) {
         /* fragment front-facing input */
         type = VGPU10_OPCODE_DCL_INPUT_PS_SGV;
         interpolationMode = VGPU10_INTERPOLATION_CONSTANT;
         name = VGPU10_NAME_IS_FRONT_FACE;
         emit->fs.face_input_index = i;
      }
      else if (semantic_name == TGSI_SEMANTIC_PRIMID) {
         /* primitive ID */
         type = VGPU10_OPCODE_DCL_INPUT_PS_SGV;
         interpolationMode = VGPU10_INTERPOLATION_CONSTANT;
         name = VGPU10_NAME_PRIMITIVE_ID;
      }
      else if (semantic_name == TGSI_SEMANTIC_SAMPLEID) {
         /* sample index / ID */
         type = VGPU10_OPCODE_DCL_INPUT_PS_SGV;
         interpolationMode = VGPU10_INTERPOLATION_CONSTANT;
         name = VGPU10_NAME_SAMPLE_INDEX;
      }
      else if (semantic_name == TGSI_SEMANTIC_LAYER) {
         /* render target array index */
         if (emit->key.fs.layer_to_zero) {
            /**
             * The shader from the previous stage does not write to layer,
             * so reading the layer index in fragment shader should return 0.
             */
            emit->fs.layer_input_index = i;
            continue;
         } else {
            type = VGPU10_OPCODE_DCL_INPUT_PS_SGV;
            interpolationMode = VGPU10_INTERPOLATION_CONSTANT;
            name = VGPU10_NAME_RENDER_TARGET_ARRAY_INDEX;
            mask = VGPU10_OPERAND_4_COMPONENT_MASK_X;
         }
      }
      else if (semantic_name == TGSI_SEMANTIC_VIEWPORT_INDEX) {
         /* viewport index */
         type = VGPU10_OPCODE_DCL_INPUT_PS_SGV;
         interpolationMode = VGPU10_INTERPOLATION_CONSTANT;
         name = VGPU10_NAME_VIEWPORT_ARRAY_INDEX;
         mask = VGPU10_OPERAND_4_COMPONENT_MASK_X;
      }
      else {
         /* general fragment input */
         type = VGPU10_OPCODE_DCL_INPUT_PS;
         interpolationMode =
               translate_interpolation(emit,
                                       emit->info.input_interpolate[i],
                                       emit->info.input_interpolate_loc[i]);

         /* keeps track if flat interpolation mode is being used */
         emit->uses_flat_interp = emit->uses_flat_interp ||
               (interpolationMode == VGPU10_INTERPOLATION_CONSTANT);

         name = VGPU10_NAME_UNDEFINED;
      }

      emit_input_declaration(emit, type,
                             VGPU10_OPERAND_TYPE_INPUT,
                             VGPU10_OPERAND_INDEX_1D, index, 1,
                             name,
                             VGPU10_OPERAND_4_COMPONENT,
                             VGPU10_OPERAND_4_COMPONENT_MASK_MODE,
                             mask,
                             interpolationMode, true,
                             map_tgsi_semantic_to_sgn_name(semantic_name));
   }
}


/**
 * Emit input declarations for vertex shader.
 */
static void
emit_vs_input_declarations(struct svga_shader_emitter_v10 *emit)
{
   unsigned i;

   for (i = 0; i < emit->info.file_max[TGSI_FILE_INPUT] + 1; i++) {
      unsigned usage_mask = emit->info.input_usage_mask[i];
      unsigned index = i;

      if (usage_mask == 0)
         continue;  /* register is not actually used */

      emit_input_declaration(emit, VGPU10_OPCODE_DCL_INPUT,
                             VGPU10_OPERAND_TYPE_INPUT,
                             VGPU10_OPERAND_INDEX_1D, index, 1,
                             VGPU10_NAME_UNDEFINED,
                             VGPU10_OPERAND_4_COMPONENT,
                             VGPU10_OPERAND_4_COMPONENT_MASK_MODE,
                             VGPU10_OPERAND_4_COMPONENT_MASK_ALL,
                             VGPU10_INTERPOLATION_UNDEFINED, true,
                             SVGADX_SIGNATURE_SEMANTIC_NAME_UNDEFINED);
   }
}


/**
 * Emit input declarations for geometry shader.
 */
static void
emit_gs_input_declarations(struct svga_shader_emitter_v10 *emit)
{
   unsigned i;

   for (i = 0; i < emit->info.num_inputs; i++) {
      enum tgsi_semantic semantic_name = emit->info.input_semantic_name[i];
      unsigned usage_mask = emit->info.input_usage_mask[i];
      unsigned index = emit->linkage.input_map[i];
      unsigned opcodeType, operandType;
      unsigned numComp, selMode;
      unsigned name;
      unsigned dim;

      if (usage_mask == 0)
         continue;  /* register is not actually used */

      opcodeType = VGPU10_OPCODE_DCL_INPUT;
      operandType = VGPU10_OPERAND_TYPE_INPUT;
      numComp = VGPU10_OPERAND_4_COMPONENT;
      selMode = VGPU10_OPERAND_4_COMPONENT_MASK_MODE;
      name = VGPU10_NAME_UNDEFINED;

      /* all geometry shader inputs are two dimensional except
       * gl_PrimitiveID
       */
      dim = VGPU10_OPERAND_INDEX_2D;

      if (semantic_name == TGSI_SEMANTIC_PRIMID) {
         /* Primitive ID */
         operandType = VGPU10_OPERAND_TYPE_INPUT_PRIMITIVEID;
         dim = VGPU10_OPERAND_INDEX_0D;
         numComp = VGPU10_OPERAND_0_COMPONENT;
         selMode = 0;

         /* also save the register index so we can check for
          * primitive id when emit src register. We need to modify the
          * operand type, index dimension when emit primitive id src reg.
          */
          emit->gs.prim_id_index = i;
      }
      else if (semantic_name == TGSI_SEMANTIC_POSITION) {
         /* vertex position input */
         opcodeType = VGPU10_OPCODE_DCL_INPUT_SIV;
         name = VGPU10_NAME_POSITION;
      }

      emit_input_declaration(emit, opcodeType, operandType,
                             dim, index,
                             emit->gs.input_size,
                             name,
                             numComp, selMode,
                             VGPU10_OPERAND_4_COMPONENT_MASK_ALL,
                             VGPU10_INTERPOLATION_UNDEFINED, true,
                             map_tgsi_semantic_to_sgn_name(semantic_name));
   }
}


/**
 * Emit input declarations for tessellation control shader.
 */
static void
emit_tcs_input_declarations(struct svga_shader_emitter_v10 *emit)
{
   unsigned i;
   unsigned size = emit->key.tcs.vertices_per_patch;
   bool addSignature = true;

   if (!emit->tcs.control_point_phase)
      addSignature = emit->tcs.fork_phase_add_signature;

   for (i = 0; i < emit->info.num_inputs; i++) {
      unsigned usage_mask = emit->info.input_usage_mask[i];
      unsigned index = emit->linkage.input_map[i];
      enum tgsi_semantic semantic_name = emit->info.input_semantic_name[i];
      VGPU10_SYSTEM_NAME name = VGPU10_NAME_UNDEFINED;
      VGPU10_OPERAND_TYPE operandType = VGPU10_OPERAND_TYPE_INPUT;
      SVGA3dDXSignatureSemanticName sgn_name =
         map_tgsi_semantic_to_sgn_name(semantic_name);

      if (semantic_name == TGSI_SEMANTIC_POSITION ||
          index == emit->linkage.position_index) {
         /* save the input control point index for later use */
         emit->tcs.control_point_input_index = i;
      }
      else if (usage_mask == 0) {
         continue;  /* register is not actually used */
      }
      else if (semantic_name == TGSI_SEMANTIC_CLIPDIST) {
         /* The shadow copy is being used here. So set the signature name
          * to UNDEFINED.
          */
         sgn_name = SVGADX_SIGNATURE_SEMANTIC_NAME_UNDEFINED;
      }

      /* input control points in the patch constant phase are emitted in the
       * vicp register rather than the v register.
       */
      if (!emit->tcs.control_point_phase) {
         operandType = VGPU10_OPERAND_TYPE_INPUT_CONTROL_POINT;
      }

      /* Tessellation control shader inputs are two dimensional.
       * The array size is determined by the patch vertex count.
       */
      emit_input_declaration(emit, VGPU10_OPCODE_DCL_INPUT,
                             operandType,
                             VGPU10_OPERAND_INDEX_2D,
                             index, size, name,
                             VGPU10_OPERAND_4_COMPONENT,
                             VGPU10_OPERAND_4_COMPONENT_MASK_MODE,
                             VGPU10_OPERAND_4_COMPONENT_MASK_ALL,
                             VGPU10_INTERPOLATION_UNDEFINED,
                             addSignature, sgn_name);
   }

   if (emit->tcs.control_point_phase) {

      /* Also add an address register for the indirection to the
       * input control points
       */
      emit->tcs.control_point_addr_index = emit->num_address_regs++;
   }
}


static void
emit_tessfactor_input_declarations(struct svga_shader_emitter_v10 *emit)
{

   /* In tcs, tess factors are emitted as extra outputs.
    * The starting register index for the tess factors is captured
    * in the compile key.
    */
   unsigned inputIndex = emit->key.tes.tessfactor_index;

   if (emit->tes.prim_mode == MESA_PRIM_QUADS) {
      if (emit->key.tes.need_tessouter) {
         emit->tes.outer.in_index = inputIndex;
         for (int i = 0; i < 4; i++) {
            emit_tesslevel_declaration(emit, inputIndex++,
               VGPU10_OPCODE_DCL_INPUT_SIV,
               VGPU10_OPERAND_TYPE_INPUT_PATCH_CONSTANT,
               VGPU10_NAME_FINAL_QUAD_U_EQ_0_EDGE_TESSFACTOR + i,
               SVGADX_SIGNATURE_SEMANTIC_NAME_FINAL_QUAD_U_EQ_0_EDGE_TESSFACTOR + i);
         }
      }

      if (emit->key.tes.need_tessinner) {
         emit->tes.inner.in_index = inputIndex;
         emit_tesslevel_declaration(emit, inputIndex++,
            VGPU10_OPCODE_DCL_INPUT_SIV,
            VGPU10_OPERAND_TYPE_INPUT_PATCH_CONSTANT,
            VGPU10_NAME_FINAL_QUAD_U_INSIDE_TESSFACTOR,
            SVGADX_SIGNATURE_SEMANTIC_NAME_FINAL_QUAD_U_INSIDE_TESSFACTOR);

         emit_tesslevel_declaration(emit, inputIndex++,
            VGPU10_OPCODE_DCL_INPUT_SIV,
            VGPU10_OPERAND_TYPE_INPUT_PATCH_CONSTANT,
            VGPU10_NAME_FINAL_QUAD_V_INSIDE_TESSFACTOR,
            SVGADX_SIGNATURE_SEMANTIC_NAME_FINAL_QUAD_V_INSIDE_TESSFACTOR);
      }
   }
   else if (emit->tes.prim_mode == MESA_PRIM_TRIANGLES) {
      if (emit->key.tes.need_tessouter) {
         emit->tes.outer.in_index = inputIndex;
         for (int i = 0; i < 3; i++) {
            emit_tesslevel_declaration(emit, inputIndex++,
               VGPU10_OPCODE_DCL_INPUT_SIV,
               VGPU10_OPERAND_TYPE_INPUT_PATCH_CONSTANT,
               VGPU10_NAME_FINAL_TRI_U_EQ_0_EDGE_TESSFACTOR + i,
               SVGADX_SIGNATURE_SEMANTIC_NAME_FINAL_TRI_U_EQ_0_EDGE_TESSFACTOR + i);
         }
      }

      if (emit->key.tes.need_tessinner) {
         emit->tes.inner.in_index = inputIndex;
         emit_tesslevel_declaration(emit, inputIndex++,
            VGPU10_OPCODE_DCL_INPUT_SIV,
            VGPU10_OPERAND_TYPE_INPUT_PATCH_CONSTANT,
            VGPU10_NAME_FINAL_TRI_INSIDE_TESSFACTOR,
            SVGADX_SIGNATURE_SEMANTIC_NAME_FINAL_TRI_INSIDE_TESSFACTOR);
      }
   }
   else if (emit->tes.prim_mode == MESA_PRIM_LINES) {
      if (emit->key.tes.need_tessouter) {
         emit->tes.outer.in_index = inputIndex;
         emit_tesslevel_declaration(emit, inputIndex++,
            VGPU10_OPCODE_DCL_INPUT_SIV,
            VGPU10_OPERAND_TYPE_INPUT_PATCH_CONSTANT,
            VGPU10_NAME_FINAL_LINE_DETAIL_TESSFACTOR,
            SVGADX_SIGNATURE_SEMANTIC_NAME_FINAL_LINE_DETAIL_TESSFACTOR);

         emit_tesslevel_declaration(emit, inputIndex++,
            VGPU10_OPCODE_DCL_INPUT_SIV,
            VGPU10_OPERAND_TYPE_INPUT_PATCH_CONSTANT,
            VGPU10_NAME_FINAL_LINE_DENSITY_TESSFACTOR,
            SVGADX_SIGNATURE_SEMANTIC_NAME_FINAL_LINE_DENSITY_TESSFACTOR);
      }
   }
}


/**
 * Emit input declarations for tessellation evaluation shader.
 */
static void
emit_tes_input_declarations(struct svga_shader_emitter_v10 *emit)
{
   unsigned i;

   for (i = 0; i < emit->info.num_inputs; i++) {
      unsigned usage_mask = emit->info.input_usage_mask[i];
      unsigned index = emit->linkage.input_map[i];
      unsigned size;
      const enum tgsi_semantic semantic_name =
         emit->info.input_semantic_name[i];
      SVGA3dDXSignatureSemanticName sgn_name;
      VGPU10_OPERAND_TYPE operandType;
      VGPU10_OPERAND_INDEX_DIMENSION dim;

      if (usage_mask == 0)
         usage_mask = 1;  /* at least set usage mask to one */

      if (semantic_name == TGSI_SEMANTIC_PATCH) {
         operandType = VGPU10_OPERAND_TYPE_INPUT_PATCH_CONSTANT;
         dim = VGPU10_OPERAND_INDEX_1D;
         size = 1;
         sgn_name = map_tgsi_semantic_to_sgn_name(semantic_name);
      }
      else {
         operandType = VGPU10_OPERAND_TYPE_INPUT_CONTROL_POINT;
         dim = VGPU10_OPERAND_INDEX_2D;
         size = emit->key.tes.vertices_per_patch;
         sgn_name = SVGADX_SIGNATURE_SEMANTIC_NAME_UNDEFINED;
      }

      emit_input_declaration(emit, VGPU10_OPCODE_DCL_INPUT, operandType,
                             dim, index, size, VGPU10_NAME_UNDEFINED,
                             VGPU10_OPERAND_4_COMPONENT,
                             VGPU10_OPERAND_4_COMPONENT_MASK_MODE,
                             VGPU10_OPERAND_4_COMPONENT_MASK_ALL,
                             VGPU10_INTERPOLATION_UNDEFINED,
                             true, sgn_name);
   }

   emit_tessfactor_input_declarations(emit);

   /* DX spec requires DS input controlpoint/patch-constant signatures to match
    * the HS output controlpoint/patch-constant signatures exactly.
    * Add missing input declarations even if they are not used in the shader.
    */
   if (emit->linkage.num_inputs < emit->linkage.prevShader.num_outputs) {
      struct tgsi_shader_info *prevInfo = emit->prevShaderInfo;
      for (i = 0; i < emit->linkage.prevShader.num_outputs; i++) {

          /* If a tcs output does not have a corresponding input register in
           * tes, add one.
           */
          if (emit->linkage.prevShader.output_map[i] >
              emit->linkage.input_map_max) {
             const enum tgsi_semantic sem_name = prevInfo->output_semantic_name[i];

             if (sem_name == TGSI_SEMANTIC_PATCH) {
                emit_input_declaration(emit, VGPU10_OPCODE_DCL_INPUT,
                                       VGPU10_OPERAND_TYPE_INPUT_PATCH_CONSTANT,
                                       VGPU10_OPERAND_INDEX_1D,
                                       i, 1, VGPU10_NAME_UNDEFINED,
                                       VGPU10_OPERAND_4_COMPONENT,
                                       VGPU10_OPERAND_4_COMPONENT_MASK_MODE,
                                       VGPU10_OPERAND_4_COMPONENT_MASK_ALL,
                                       VGPU10_INTERPOLATION_UNDEFINED,
                                       true,
                                       map_tgsi_semantic_to_sgn_name(sem_name));

             } else if (sem_name != TGSI_SEMANTIC_TESSINNER &&
                        sem_name != TGSI_SEMANTIC_TESSOUTER) {
                emit_input_declaration(emit, VGPU10_OPCODE_DCL_INPUT,
                                       VGPU10_OPERAND_TYPE_INPUT_CONTROL_POINT,
                                       VGPU10_OPERAND_INDEX_2D,
                                       i, emit->key.tes.vertices_per_patch,
                                       VGPU10_NAME_UNDEFINED,
                                       VGPU10_OPERAND_4_COMPONENT,
                                       VGPU10_OPERAND_4_COMPONENT_MASK_MODE,
                                       VGPU10_OPERAND_4_COMPONENT_MASK_ALL,
                                       VGPU10_INTERPOLATION_UNDEFINED,
                                       true,
                                       map_tgsi_semantic_to_sgn_name(sem_name));
             }
             /* tessellation factors are taken care of in
              * emit_tessfactor_input_declarations().
              */
         }
      }
   }
}


/**
 * Emit all input declarations.
 */
static bool
emit_input_declarations(struct svga_shader_emitter_v10 *emit)
{
   emit->index_range.required =
      emit->info.indirect_files & (1 << TGSI_FILE_INPUT) ? true : false;

   switch (emit->unit) {
   case PIPE_SHADER_FRAGMENT:
      emit_fs_input_declarations(emit);
      break;
   case PIPE_SHADER_GEOMETRY:
      emit_gs_input_declarations(emit);
      break;
   case PIPE_SHADER_VERTEX:
      emit_vs_input_declarations(emit);
      break;
   case PIPE_SHADER_TESS_CTRL:
      emit_tcs_input_declarations(emit);
      break;
   case PIPE_SHADER_TESS_EVAL:
      emit_tes_input_declarations(emit);
      break;
   case PIPE_SHADER_COMPUTE:
      //XXX emit_cs_input_declarations(emit);
      break;
   default:
      assert(0);
   }

   if (emit->index_range.start_index != INVALID_INDEX) {
      emit_index_range_declaration(emit);
   }
   emit->index_range.required = false;
   return true;
}


/**
 * Emit all output declarations.
 */
static bool
emit_output_declarations(struct svga_shader_emitter_v10 *emit)
{
   emit->index_range.required =
      emit->info.indirect_files & (1 << TGSI_FILE_OUTPUT) ? true : false;

   switch (emit->unit) {
   case PIPE_SHADER_FRAGMENT:
      emit_fs_output_declarations(emit);
      break;
   case PIPE_SHADER_GEOMETRY:
      emit_gs_output_declarations(emit);
      break;
   case PIPE_SHADER_VERTEX:
      emit_vs_output_declarations(emit);
      break;
   case PIPE_SHADER_TESS_CTRL:
      emit_tcs_output_declarations(emit);
      break;
   case PIPE_SHADER_TESS_EVAL:
      emit_tes_output_declarations(emit);
      break;
   case PIPE_SHADER_COMPUTE:
      //XXX emit_cs_output_declarations(emit);
      break;
   default:
      assert(0);
   }

   if (emit->vposition.so_index != INVALID_INDEX &&
       emit->vposition.out_index != INVALID_INDEX) {

      assert(emit->unit != PIPE_SHADER_FRAGMENT);

      /* Emit the declaration for the non-adjusted vertex position
       * for stream output purpose
       */
      emit_output_declaration(emit, VGPU10_OPCODE_DCL_OUTPUT,
                              emit->vposition.so_index,
                              VGPU10_NAME_UNDEFINED,
                              VGPU10_OPERAND_4_COMPONENT_MASK_ALL,
                              true,
                              SVGADX_SIGNATURE_SEMANTIC_NAME_POSITION);
   }

   if (emit->clip_dist_so_index != INVALID_INDEX &&
       emit->clip_dist_out_index != INVALID_INDEX) {

      assert(emit->unit != PIPE_SHADER_FRAGMENT);

      /* Emit the declaration for the clip distance shadow copy which
       * will be used for stream output purpose and for clip distance
       * varying variable. Note all clip distances
       * will be written regardless of the enabled clipping planes.
       */
      emit_output_declaration(emit, VGPU10_OPCODE_DCL_OUTPUT,
                              emit->clip_dist_so_index,
                              VGPU10_NAME_UNDEFINED,
                              VGPU10_OPERAND_4_COMPONENT_MASK_ALL,
                              true,
                              SVGADX_SIGNATURE_SEMANTIC_NAME_UNDEFINED);

      if (emit->info.num_written_clipdistance > 4) {
         /* for the second clip distance register, each handles 4 planes */
         emit_output_declaration(emit, VGPU10_OPCODE_DCL_OUTPUT,
                                 emit->clip_dist_so_index + 1,
                                 VGPU10_NAME_UNDEFINED,
                                 VGPU10_OPERAND_4_COMPONENT_MASK_ALL,
                                 true,
                                 SVGADX_SIGNATURE_SEMANTIC_NAME_UNDEFINED);
      }
   }

   if (emit->index_range.start_index != INVALID_INDEX) {
      emit_index_range_declaration(emit);
   }
   emit->index_range.required = false;
   return true;
}


/**
 * A helper function to create a temporary indexable array
 * and initialize the corresponding entries in the temp_map array.
 */
static void
create_temp_array(struct svga_shader_emitter_v10 *emit,
                  unsigned arrayID, unsigned first, unsigned count,
                  unsigned startIndex)
{
   unsigned i, tempIndex = startIndex;

   emit->num_temp_arrays = MAX2(emit->num_temp_arrays, arrayID + 1);
   assert(emit->num_temp_arrays <= MAX_TEMP_ARRAYS);
   emit->num_temp_arrays = MIN2(emit->num_temp_arrays, MAX_TEMP_ARRAYS);

   emit->temp_arrays[arrayID].start = first;
   emit->temp_arrays[arrayID].size = count;

   /* Fill in the temp_map entries for this temp array */
   for (i = 0; i < count; i++, tempIndex++) {
      emit->temp_map[tempIndex].arrayId = arrayID;
      emit->temp_map[tempIndex].index = i;
   }
}


/**
 * Emit the declaration for the temporary registers.
 */
static bool
emit_temporaries_declaration(struct svga_shader_emitter_v10 *emit)
{
   unsigned total_temps, reg, i;

   total_temps = emit->num_shader_temps;

   /* If there is indirect access to non-indexable temps in the shader,
    * convert those temps to indexable temps. This works around a bug
    * in the GLSL->TGSI translator exposed in piglit test
    * glsl-1.20/execution/fs-const-array-of-struct-of-array.shader_test.
    * Internal temps added by the driver remain as non-indexable temps.
    */
   if ((emit->info.indirect_files & (1 << TGSI_FILE_TEMPORARY)) &&
       emit->num_temp_arrays == 0) {
      create_temp_array(emit, 1, 0, total_temps, 0);
   }

   /* Allocate extra temps for specially-implemented instructions,
    * such as LIT.
    */
   total_temps += MAX_INTERNAL_TEMPS;

   /* Allocate extra temps for clip distance or clip vertex.
    */
   if (emit->clip_mode == CLIP_DISTANCE) {
      /* We need to write the clip distance to a temporary register
       * first. Then it will be copied to the shadow copy for
       * the clip distance varying variable and stream output purpose.
       * It will also be copied to the actual CLIPDIST register
       * according to the enabled clip planes
       */
      emit->clip_dist_tmp_index = total_temps++;
      if (emit->info.num_written_clipdistance > 4)
         total_temps++; /* second clip register */
   }
   else if (emit->clip_mode == CLIP_VERTEX && emit->key.last_vertex_stage) {
      /* If the current shader is in the last vertex processing stage,
       * We need to convert the TGSI CLIPVERTEX output to one or more
       * clip distances.  Allocate a temp reg for the clipvertex here.
       */
      assert(emit->info.writes_clipvertex > 0);
      emit->clip_vertex_tmp_index = total_temps;
      total_temps++;
   }

   if (emit->info.uses_vertexid) {
      assert(emit->unit == PIPE_SHADER_VERTEX);
      emit->vs.vertex_id_tmp_index = total_temps++;
   }

   if (emit->unit == PIPE_SHADER_VERTEX || emit->unit == PIPE_SHADER_GEOMETRY) {
      if (emit->vposition.need_prescale || emit->key.vs.undo_viewport ||
          emit->key.clip_plane_enable ||
          emit->vposition.so_index != INVALID_INDEX) {
         emit->vposition.tmp_index = total_temps;
         total_temps += 1;
      }

      if (emit->vposition.need_prescale) {
         emit->vposition.prescale_scale_index = total_temps++;
         emit->vposition.prescale_trans_index = total_temps++;
      }

      if (emit->unit == PIPE_SHADER_VERTEX) {
         unsigned attrib_mask = (emit->key.vs.adjust_attrib_w_1 |
                                 emit->key.vs.adjust_attrib_itof |
                                 emit->key.vs.adjust_attrib_utof |
                                 emit->key.vs.attrib_is_bgra |
                                 emit->key.vs.attrib_puint_to_snorm |
                                 emit->key.vs.attrib_puint_to_uscaled |
                                 emit->key.vs.attrib_puint_to_sscaled);
         while (attrib_mask) {
            unsigned index = u_bit_scan(&attrib_mask);
            emit->vs.adjusted_input[index] = total_temps++;
         }
      }
      else if (emit->unit == PIPE_SHADER_GEOMETRY) {
         if (emit->key.gs.writes_viewport_index)
            emit->gs.viewport_index_tmp_index = total_temps++;
      }
   }
   else if (emit->unit == PIPE_SHADER_FRAGMENT) {
      if (emit->key.fs.alpha_func != SVGA3D_CMP_ALWAYS ||
          emit->key.fs.write_color0_to_n_cbufs > 1) {
         /* Allocate a temp to hold the output color */
         emit->fs.color_tmp_index = total_temps;
         total_temps += 1;
      }

      if (emit->fs.face_input_index != INVALID_INDEX) {
         /* Allocate a temp for the +/-1 face register */
         emit->fs.face_tmp_index = total_temps;
         total_temps += 1;
      }

      if (emit->fs.fragcoord_input_index != INVALID_INDEX) {
         /* Allocate a temp for modified fragment position register */
         emit->fs.fragcoord_tmp_index = total_temps;
         total_temps += 1;
      }

      if (emit->fs.sample_pos_sys_index != INVALID_INDEX) {
         /* Allocate a temp for the sample position */
         emit->fs.sample_pos_tmp_index = total_temps++;
      }
   }
   else if (emit->unit == PIPE_SHADER_TESS_EVAL) {
      if (emit->vposition.need_prescale) {
         emit->vposition.tmp_index = total_temps++;
         emit->vposition.prescale_scale_index = total_temps++;
         emit->vposition.prescale_trans_index = total_temps++;
      }

      if (emit->tes.inner.tgsi_index) {
         emit->tes.inner.temp_index = total_temps;
         total_temps += 1;
      }

      if (emit->tes.outer.tgsi_index) {
         emit->tes.outer.temp_index = total_temps;
         total_temps += 1;
      }
   }
   else if (emit->unit == PIPE_SHADER_TESS_CTRL) {
      if (emit->tcs.inner.tgsi_index != INVALID_INDEX) {
         if (!emit->tcs.control_point_phase) {
            emit->tcs.inner.temp_index = total_temps;
            total_temps += 1;
         }
      }
      if (emit->tcs.outer.tgsi_index != INVALID_INDEX) {
         if (!emit->tcs.control_point_phase) {
            emit->tcs.outer.temp_index = total_temps;
            total_temps += 1;
         }
      }

      if (emit->tcs.control_point_phase &&
          emit->info.reads_pervertex_outputs) {
         emit->tcs.control_point_tmp_index = total_temps;
         total_temps += emit->tcs.control_point_out_count;
      }
      else if (!emit->tcs.control_point_phase &&
               emit->info.reads_perpatch_outputs) {

         /* If there is indirect access to the patch constant outputs
          * in the control point phase, then an indexable temporary array
          * will be created for these patch constant outputs.
          * Note, indirect access can only be applicable to
          * patch constant outputs in the control point phase.
          */
         if (emit->info.indirect_files & (1 << TGSI_FILE_OUTPUT)) {
            unsigned arrayID =
               emit->num_temp_arrays ? emit->num_temp_arrays : 1;
            create_temp_array(emit, arrayID, 0,
                              emit->tcs.patch_generic_out_count, total_temps);
         }
         emit->tcs.patch_generic_tmp_index = total_temps;
         total_temps += emit->tcs.patch_generic_out_count;
      }

      emit->tcs.invocation_id_tmp_index = total_temps++;
   }

   if (emit->raw_bufs) {
      /**
       * Add 3 more temporaries if we need to translate constant buffer
       * to srv raw buffer. Since we need to load the value to a temporary
       * before it can be used as a source. There could be three source
       * register in an instruction.
       */
      emit->raw_buf_tmp_index = total_temps;
      total_temps+=3;
   }

   for (i = 0; i < emit->num_address_regs; i++) {
      emit->address_reg_index[i] = total_temps++;
   }

   /* Initialize the temp_map array which maps TGSI temp indexes to VGPU10
    * temp indexes.  Basically, we compact all the non-array temp register
    * indexes into a consecutive series.
    *
    * Before, we may have some TGSI declarations like:
    *   DCL TEMP[0..1], LOCAL
    *   DCL TEMP[2..4], ARRAY(1), LOCAL
    *   DCL TEMP[5..7], ARRAY(2), LOCAL
    *   plus, some extra temps, like TEMP[8], TEMP[9] for misc things
    *
    * After, we'll have a map like this:
    *   temp_map[0] = { array 0, index 0 }
    *   temp_map[1] = { array 0, index 1 }
    *   temp_map[2] = { array 1, index 0 }
    *   temp_map[3] = { array 1, index 1 }
    *   temp_map[4] = { array 1, index 2 }
    *   temp_map[5] = { array 2, index 0 }
    *   temp_map[6] = { array 2, index 1 }
    *   temp_map[7] = { array 2, index 2 }
    *   temp_map[8] = { array 0, index 2 }
    *   temp_map[9] = { array 0, index 3 }
    *
    * We'll declare two arrays of 3 elements, plus a set of four non-indexed
    * temps numbered 0..3
    *
    * Any time we emit a temporary register index, we'll have to use the
    * temp_map[] table to convert the TGSI index to the VGPU10 index.
    *
    * Finally, we recompute the total_temps value here.
    */
   reg = 0;
   for (i = 0; i < total_temps; i++) {
      if (emit->temp_map[i].arrayId == 0) {
         emit->temp_map[i].index = reg++;
      }
   }

   if (0) {
      debug_printf("total_temps %u\n", total_temps);
      for (i = 0; i < total_temps; i++) {
         debug_printf("temp %u ->  array %u  index %u\n",
                      i, emit->temp_map[i].arrayId, emit->temp_map[i].index);
      }
   }

   total_temps = reg;

   /* Emit declaration of ordinary temp registers */
   if (total_temps > 0) {
      VGPU10OpcodeToken0 opcode0;

      opcode0.value = 0;
      opcode0.opcodeType = VGPU10_OPCODE_DCL_TEMPS;

      begin_emit_instruction(emit);
      emit_dword(emit, opcode0.value);
      emit_dword(emit, total_temps);
      end_emit_instruction(emit);
   }

   /* Emit declarations for indexable temp arrays.  Skip 0th entry since
    * it's unused.
    */
   for (i = 1; i < emit->num_temp_arrays; i++) {
      unsigned num_temps = emit->temp_arrays[i].size;

      if (num_temps > 0) {
         VGPU10OpcodeToken0 opcode0;

         opcode0.value = 0;
         opcode0.opcodeType = VGPU10_OPCODE_DCL_INDEXABLE_TEMP;

         begin_emit_instruction(emit);
         emit_dword(emit, opcode0.value);
         emit_dword(emit, i); /* which array */
         emit_dword(emit, num_temps);
         emit_dword(emit, 4); /* num components */
         end_emit_instruction(emit);

         total_temps += num_temps;
      }
   }

   /* Check that the grand total of all regular and indexed temps is
    * under the limit.
    */
   check_register_index(emit, VGPU10_OPCODE_DCL_TEMPS, total_temps - 1);

   return true;
}


static bool
emit_rawbuf_declaration(struct svga_shader_emitter_v10 *emit,
                        unsigned index)
{
   VGPU10OpcodeToken0 opcode1;
   VGPU10OperandToken0 operand1;

   opcode1.value = 0;
   opcode1.opcodeType = VGPU10_OPCODE_DCL_RESOURCE_RAW;
   opcode1.resourceDimension = VGPU10_RESOURCE_DIMENSION_UNKNOWN;

   operand1.value = 0;
   operand1.numComponents = VGPU10_OPERAND_0_COMPONENT;
   operand1.operandType = VGPU10_OPERAND_TYPE_RESOURCE;
   operand1.indexDimension = VGPU10_OPERAND_INDEX_1D;
   operand1.index0Representation = VGPU10_OPERAND_INDEX_IMMEDIATE32;

   begin_emit_instruction(emit);
   emit_dword(emit, opcode1.value);
   emit_dword(emit, operand1.value);
   emit_dword(emit, index);
   end_emit_instruction(emit);

   return true;
}


static bool
emit_constant_declaration(struct svga_shader_emitter_v10 *emit)
{
   VGPU10OpcodeToken0 opcode0;
   VGPU10OperandToken0 operand0;
   unsigned total_consts, i;

   opcode0.value = 0;
   opcode0.opcodeType = VGPU10_OPCODE_DCL_CONSTANT_BUFFER;
   opcode0.accessPattern = VGPU10_CB_IMMEDIATE_INDEXED;
   /* XXX or, access pattern = VGPU10_CB_DYNAMIC_INDEXED */

   operand0.value = 0;
   operand0.numComponents = VGPU10_OPERAND_4_COMPONENT;
   operand0.indexDimension = VGPU10_OPERAND_INDEX_2D;
   operand0.index0Representation = VGPU10_OPERAND_INDEX_IMMEDIATE32;
   operand0.index1Representation = VGPU10_OPERAND_INDEX_IMMEDIATE32;
   operand0.operandType = VGPU10_OPERAND_TYPE_CONSTANT_BUFFER;
   operand0.selectionMode = VGPU10_OPERAND_4_COMPONENT_SWIZZLE_MODE;
   operand0.swizzleX = 0;
   operand0.swizzleY = 1;
   operand0.swizzleZ = 2;
   operand0.swizzleW = 3;

   /**
    * Emit declaration for constant buffer [0].  We also allocate
    * room for the extra constants here.
    */
   total_consts = emit->num_shader_consts[0];

   /* Now, allocate constant slots for the "extra" constants.
    * Note: it's critical that these extra constant locations
    * exactly match what's emitted by the "extra" constants code
    * in svga_state_constants.c
    */

   /* Vertex position scale/translation */
   if (emit->vposition.need_prescale) {
      emit->vposition.prescale_cbuf_index = total_consts;
      total_consts += (2 * emit->vposition.num_prescale);
   }

   if (emit->unit == PIPE_SHADER_VERTEX) {
      if (emit->key.vs.undo_viewport) {
         emit->vs.viewport_index = total_consts++;
      }
      if (emit->key.vs.need_vertex_id_bias) {
         emit->vs.vertex_id_bias_index = total_consts++;
      }
   }

   /* user-defined clip planes */
   if (emit->key.clip_plane_enable) {
      unsigned n = util_bitcount(emit->key.clip_plane_enable);
      assert(emit->unit != PIPE_SHADER_FRAGMENT &&
             emit->unit != PIPE_SHADER_COMPUTE);
      for (i = 0; i < n; i++) {
         emit->clip_plane_const[i] = total_consts++;
      }
   }

   for (i = 0; i < emit->num_samplers; i++) {

      if (emit->key.tex[i].sampler_view) {
         /* Texcoord scale factors for RECT textures */
         if (emit->key.tex[i].unnormalized) {
            emit->texcoord_scale_index[i] = total_consts++;
         }

         /* Texture buffer sizes */
         if (emit->key.tex[i].target == PIPE_BUFFER) {
            emit->texture_buffer_size_index[i] = total_consts++;
         }
      }
   }
   if (emit->key.image_size_used) {
      emit->image_size_index = total_consts;
      total_consts += emit->num_images;
   }

   if (total_consts > 0) {
      if (total_consts > VGPU10_MAX_CONSTANT_BUFFER_ELEMENT_COUNT) {
         debug_printf("Warning: Too many constants [%u] declared in constant"
                      " buffer 0. %u is the limit.\n",
                      total_consts,
                      VGPU10_MAX_CONSTANT_BUFFER_ELEMENT_COUNT);
         total_consts = VGPU10_MAX_CONSTANT_BUFFER_ELEMENT_COUNT;
         emit->register_overflow = true;
      }
      begin_emit_instruction(emit);
      emit_dword(emit, opcode0.value);
      emit_dword(emit, operand0.value);
      emit_dword(emit, 0);  /* which const buffer slot */
      emit_dword(emit, total_consts);
      end_emit_instruction(emit);
   }

   /* Declare remaining constant buffers (UBOs) */

   for (i = 1; i < ARRAY_SIZE(emit->num_shader_consts); i++) {
      if (emit->num_shader_consts[i] > 0) {
         if (emit->raw_bufs & (1 << i)) {
            /* UBO declared as srv raw buffer */
            emit_rawbuf_declaration(emit, i + emit->raw_buf_srv_start_index);
         }
         else {

            /* UBO declared as const buffer */
            begin_emit_instruction(emit);
            emit_dword(emit, opcode0.value);
            emit_dword(emit, operand0.value);
            emit_dword(emit, i);  /* which const buffer slot */
            emit_dword(emit, emit->num_shader_consts[i]);
            end_emit_instruction(emit);
         }
      }
   }

   return true;
}


/**
 * Emit declarations for samplers.
 */
static bool
emit_sampler_declarations(struct svga_shader_emitter_v10 *emit)
{
   unsigned i;

   for (i = 0; i < emit->key.num_samplers; i++) {

      VGPU10OpcodeToken0 opcode0;
      VGPU10OperandToken0 operand0;

      opcode0.value = 0;
      opcode0.opcodeType = VGPU10_OPCODE_DCL_SAMPLER;
      opcode0.samplerMode = VGPU10_SAMPLER_MODE_DEFAULT;

      operand0.value = 0;
      operand0.numComponents = VGPU10_OPERAND_0_COMPONENT;
      operand0.operandType = VGPU10_OPERAND_TYPE_SAMPLER;
      operand0.indexDimension = VGPU10_OPERAND_INDEX_1D;
      operand0.index0Representation = VGPU10_OPERAND_INDEX_IMMEDIATE32;

      begin_emit_instruction(emit);
      emit_dword(emit, opcode0.value);
      emit_dword(emit, operand0.value);
      emit_dword(emit, i);
      end_emit_instruction(emit);
   }

   return true;
}


/**
 * Translate PIPE_TEXTURE_x to VGPU10_RESOURCE_DIMENSION_x.
 */
static unsigned
pipe_texture_to_resource_dimension(enum tgsi_texture_type target,
                                   unsigned num_samples,
                                   bool is_array,
                                   bool is_uav)
{
   switch (target) {
   case PIPE_BUFFER:
      return VGPU10_RESOURCE_DIMENSION_BUFFER;
   case PIPE_TEXTURE_1D:
      return VGPU10_RESOURCE_DIMENSION_TEXTURE1D;
   case PIPE_TEXTURE_2D:
      return num_samples > 2 ? VGPU10_RESOURCE_DIMENSION_TEXTURE2DMS :
         VGPU10_RESOURCE_DIMENSION_TEXTURE2D;
   case PIPE_TEXTURE_RECT:
      return VGPU10_RESOURCE_DIMENSION_TEXTURE2D;
   case PIPE_TEXTURE_3D:
      return VGPU10_RESOURCE_DIMENSION_TEXTURE3D;
   case PIPE_TEXTURE_CUBE:
      return VGPU10_RESOURCE_DIMENSION_TEXTURECUBE;
   case PIPE_TEXTURE_1D_ARRAY:
      return is_array ? VGPU10_RESOURCE_DIMENSION_TEXTURE1DARRAY
         : VGPU10_RESOURCE_DIMENSION_TEXTURE1D;
   case PIPE_TEXTURE_2D_ARRAY:
      if (num_samples > 2 && is_array)
         return VGPU10_RESOURCE_DIMENSION_TEXTURE2DMSARRAY;
      else if (is_array)
         return VGPU10_RESOURCE_DIMENSION_TEXTURE2DARRAY;
      else
         return VGPU10_RESOURCE_DIMENSION_TEXTURE2D;
   case PIPE_TEXTURE_CUBE_ARRAY:
      return is_uav ? VGPU10_RESOURCE_DIMENSION_TEXTURE2DARRAY :
             (is_array ? VGPU10_RESOURCE_DIMENSION_TEXTURECUBEARRAY :
                         VGPU10_RESOURCE_DIMENSION_TEXTURECUBE);
   default:
      assert(!"Unexpected resource type");
      return VGPU10_RESOURCE_DIMENSION_TEXTURE2D;
   }
}


/**
 * Translate TGSI_TEXTURE_x to VGPU10_RESOURCE_DIMENSION_x.
 */
static unsigned
tgsi_texture_to_resource_dimension(enum tgsi_texture_type target,
                                   unsigned num_samples,
                                   bool is_array,
                                   bool is_uav)
{
   if (target == TGSI_TEXTURE_2D_MSAA && num_samples < 2) {
      target = TGSI_TEXTURE_2D;
   }
   else if (target == TGSI_TEXTURE_2D_ARRAY_MSAA && num_samples < 2) {
      target = TGSI_TEXTURE_2D_ARRAY;
   }

   switch (target) {
   case TGSI_TEXTURE_BUFFER:
      return VGPU10_RESOURCE_DIMENSION_BUFFER;
   case TGSI_TEXTURE_1D:
      return VGPU10_RESOURCE_DIMENSION_TEXTURE1D;
   case TGSI_TEXTURE_2D:
   case TGSI_TEXTURE_RECT:
      return VGPU10_RESOURCE_DIMENSION_TEXTURE2D;
   case TGSI_TEXTURE_3D:
      return VGPU10_RESOURCE_DIMENSION_TEXTURE3D;
   case TGSI_TEXTURE_CUBE:
   case TGSI_TEXTURE_SHADOWCUBE:
      return is_uav ? VGPU10_RESOURCE_DIMENSION_TEXTURE2DARRAY :
                      VGPU10_RESOURCE_DIMENSION_TEXTURECUBE;
   case TGSI_TEXTURE_SHADOW1D:
      return VGPU10_RESOURCE_DIMENSION_TEXTURE1D;
   case TGSI_TEXTURE_SHADOW2D:
   case TGSI_TEXTURE_SHADOWRECT:
      return VGPU10_RESOURCE_DIMENSION_TEXTURE2D;
   case TGSI_TEXTURE_1D_ARRAY:
   case TGSI_TEXTURE_SHADOW1D_ARRAY:
      return is_array ? VGPU10_RESOURCE_DIMENSION_TEXTURE1DARRAY
         : VGPU10_RESOURCE_DIMENSION_TEXTURE1D;
   case TGSI_TEXTURE_2D_ARRAY:
   case TGSI_TEXTURE_SHADOW2D_ARRAY:
      return is_array ? VGPU10_RESOURCE_DIMENSION_TEXTURE2DARRAY
         : VGPU10_RESOURCE_DIMENSION_TEXTURE2D;
   case TGSI_TEXTURE_2D_MSAA:
      return VGPU10_RESOURCE_DIMENSION_TEXTURE2DMS;
   case TGSI_TEXTURE_2D_ARRAY_MSAA:
      return is_array ? VGPU10_RESOURCE_DIMENSION_TEXTURE2DMSARRAY
         : VGPU10_RESOURCE_DIMENSION_TEXTURE2DMS;
   case TGSI_TEXTURE_CUBE_ARRAY:
      return is_uav ? VGPU10_RESOURCE_DIMENSION_TEXTURE2DARRAY :
             (is_array ? VGPU10_RESOURCE_DIMENSION_TEXTURECUBEARRAY :
                         VGPU10_RESOURCE_DIMENSION_TEXTURECUBE);
   case TGSI_TEXTURE_SHADOWCUBE_ARRAY:
      return is_array ? VGPU10_RESOURCE_DIMENSION_TEXTURECUBEARRAY
         : VGPU10_RESOURCE_DIMENSION_TEXTURECUBE;
   default:
      assert(!"Unexpected resource type");
      return VGPU10_RESOURCE_DIMENSION_TEXTURE2D;
   }
}


/**
 * Given a tgsi_return_type, return true iff it is an integer type.
 */
static bool
is_integer_type(enum tgsi_return_type type)
{
   switch (type) {
      case TGSI_RETURN_TYPE_SINT:
      case TGSI_RETURN_TYPE_UINT:
         return true;
      case TGSI_RETURN_TYPE_FLOAT:
      case TGSI_RETURN_TYPE_UNORM:
      case TGSI_RETURN_TYPE_SNORM:
         return false;
      case TGSI_RETURN_TYPE_COUNT:
      default:
         assert(!"is_integer_type: Unknown tgsi_return_type");
         return false;
   }
}


/**
 * Emit declarations for resources.
 * XXX When we're sure that all TGSI shaders will be generated with
 * sampler view declarations (Ex: DCL SVIEW[n], 2D, UINT) we may
 * rework this code.
 */
static bool
emit_resource_declarations(struct svga_shader_emitter_v10 *emit)
{
   unsigned i;

   /* Emit resource decl for each sampler */
   for (i = 0; i < emit->num_samplers; i++) {
      if (!(emit->info.samplers_declared & (1 << i)))
         continue;

      VGPU10OpcodeToken0 opcode0;
      VGPU10OperandToken0 operand0;
      VGPU10ResourceReturnTypeToken return_type;
      VGPU10_RESOURCE_RETURN_TYPE rt;

      opcode0.value = 0;
      opcode0.opcodeType = VGPU10_OPCODE_DCL_RESOURCE;
      if (emit->sampler_view[i] || !emit->key.tex[i].sampler_view) {
         opcode0.resourceDimension =
            tgsi_texture_to_resource_dimension(emit->sampler_target[i],
                                               emit->key.tex[i].num_samples,
                                               emit->key.tex[i].is_array,
                                               false);
      }
      else {
         opcode0.resourceDimension =
            pipe_texture_to_resource_dimension(emit->key.tex[i].target,
                                               emit->key.tex[i].num_samples,
                                               emit->key.tex[i].is_array,
                                               false);
      }
      opcode0.sampleCount = emit->key.tex[i].num_samples;
      operand0.value = 0;
      operand0.numComponents = VGPU10_OPERAND_0_COMPONENT;
      operand0.operandType = VGPU10_OPERAND_TYPE_RESOURCE;
      operand0.indexDimension = VGPU10_OPERAND_INDEX_1D;
      operand0.index0Representation = VGPU10_OPERAND_INDEX_IMMEDIATE32;

#if 1
      /* convert TGSI_RETURN_TYPE_x to VGPU10_RETURN_TYPE_x */
      STATIC_ASSERT(VGPU10_RETURN_TYPE_UNORM == TGSI_RETURN_TYPE_UNORM + 1);
      STATIC_ASSERT(VGPU10_RETURN_TYPE_SNORM == TGSI_RETURN_TYPE_SNORM + 1);
      STATIC_ASSERT(VGPU10_RETURN_TYPE_SINT == TGSI_RETURN_TYPE_SINT + 1);
      STATIC_ASSERT(VGPU10_RETURN_TYPE_UINT == TGSI_RETURN_TYPE_UINT + 1);
      STATIC_ASSERT(VGPU10_RETURN_TYPE_FLOAT == TGSI_RETURN_TYPE_FLOAT + 1);
      assert(emit->sampler_return_type[i] <= TGSI_RETURN_TYPE_FLOAT);
      if (emit->sampler_view[i] || !emit->key.tex[i].sampler_view) {
         rt = emit->sampler_return_type[i] + 1;
      }
      else {
         rt = emit->key.tex[i].sampler_return_type;
      }
#else
      switch (emit->sampler_return_type[i]) {
         case TGSI_RETURN_TYPE_UNORM: rt = VGPU10_RETURN_TYPE_UNORM; break;
         case TGSI_RETURN_TYPE_SNORM: rt = VGPU10_RETURN_TYPE_SNORM; break;
         case TGSI_RETURN_TYPE_SINT:  rt = VGPU10_RETURN_TYPE_SINT;  break;
         case TGSI_RETURN_TYPE_UINT:  rt = VGPU10_RETURN_TYPE_UINT;  break;
         case TGSI_RETURN_TYPE_FLOAT: rt = VGPU10_RETURN_TYPE_FLOAT; break;
         case TGSI_RETURN_TYPE_COUNT:
         default:
            rt = VGPU10_RETURN_TYPE_FLOAT;
            assert(!"emit_resource_declarations: Unknown tgsi_return_type");
      }
#endif

      return_type.value = 0;
      return_type.component0 = rt;
      return_type.component1 = rt;
      return_type.component2 = rt;
      return_type.component3 = rt;

      begin_emit_instruction(emit);
      emit_dword(emit, opcode0.value);
      emit_dword(emit, operand0.value);
      emit_dword(emit, i);
      emit_dword(emit, return_type.value);
      end_emit_instruction(emit);
   }

   return true;
}


/**
 * Emit instruction to declare uav for the shader image
 */
static void
emit_image_declarations(struct svga_shader_emitter_v10 *emit)
{
   unsigned i = 0;
   unsigned unit = 0;
   unsigned uav_mask = 0;

   /* Emit uav decl for each image */
   for (i = 0; i < emit->num_images; i++, unit++) {

      /* Find the unit index of the next declared image.
       */
      while (!(emit->image_mask & (1 << unit))) {
         unit++;
      }

      VGPU10OpcodeToken0 opcode0;
      VGPU10OperandToken0 operand0;
      VGPU10ResourceReturnTypeToken return_type;

      /* If the corresponding uav for the image is already declared,
       * skip this image declaration.
       */
      if (uav_mask & (1 << emit->key.images[unit].uav_index))
         continue;

      opcode0.value = 0;
      opcode0.opcodeType = VGPU10_OPCODE_DCL_UAV_TYPED;
      opcode0.uavResourceDimension =
         tgsi_texture_to_resource_dimension(emit->image[unit].Resource,
                                            0, emit->key.images[unit].is_array,
                                            true);

      if (emit->key.images[unit].is_single_layer &&
          emit->key.images[unit].resource_target == PIPE_TEXTURE_3D) {
         opcode0.uavResourceDimension = VGPU10_RESOURCE_DIMENSION_TEXTURE3D;
      }

      /* Declare the uav as global coherent if the shader includes memory
       * barrier instructions.
       */
      opcode0.globallyCoherent =
         (emit->info.opcode_count[TGSI_OPCODE_MEMBAR] > 0) ? 1 : 0;

      operand0.value = 0;
      operand0.numComponents = VGPU10_OPERAND_0_COMPONENT;
      operand0.operandType = VGPU10_OPERAND_TYPE_UAV;
      operand0.indexDimension = VGPU10_OPERAND_INDEX_1D;
      operand0.index0Representation = VGPU10_OPERAND_INDEX_IMMEDIATE32;

      return_type.value = 0;
      return_type.component0 =
         return_type.component1 =
         return_type.component2 =
         return_type.component3 = emit->key.images[unit].return_type + 1;

      assert(emit->key.images[unit].uav_index != SVGA3D_INVALID_ID);
      begin_emit_instruction(emit);
      emit_dword(emit, opcode0.value);
      emit_dword(emit, operand0.value);
      emit_dword(emit, emit->key.images[unit].uav_index);
      emit_dword(emit, return_type.value);
      end_emit_instruction(emit);

      /* Mark the uav is already declared */
      uav_mask |= 1 << emit->key.images[unit].uav_index;
   }

   emit->uav_declared |= uav_mask;
}


/**
 * Emit instruction to declare uav for the shader buffer
 */
static void
emit_shader_buf_declarations(struct svga_shader_emitter_v10 *emit)
{
   unsigned i;
   unsigned uav_mask = 0;

   /* Emit uav decl for each shader buffer */
   for (i = 0; i < emit->num_shader_bufs; i++) {
      VGPU10OpcodeToken0 opcode0;
      VGPU10OperandToken0 operand0;

      if (emit->raw_shaderbufs & (1 << i)) {
         emit_rawbuf_declaration(emit, i + emit->raw_shaderbuf_srv_start_index);
         continue;
      }

      /* If the corresponding uav for the shader buf is already declared,
       * skip this shader buffer declaration.
       */
      if (uav_mask & (1 << emit->key.shader_buf_uav_index[i]))
         continue;

      opcode0.value = 0;
      opcode0.opcodeType = VGPU10_OPCODE_DCL_UAV_RAW;

      /* Declare the uav as global coherent if the shader includes memory
       * barrier instructions.
       */
      opcode0.globallyCoherent =
         (emit->info.opcode_count[TGSI_OPCODE_MEMBAR] > 0) ? 1 : 0;

      operand0.value = 0;
      operand0.numComponents = VGPU10_OPERAND_0_COMPONENT;
      operand0.operandType = VGPU10_OPERAND_TYPE_UAV;
      operand0.indexDimension = VGPU10_OPERAND_INDEX_1D;
      operand0.index0Representation = VGPU10_OPERAND_INDEX_IMMEDIATE32;

      assert(emit->key.shader_buf_uav_index[i] != SVGA3D_INVALID_ID);
      begin_emit_instruction(emit);
      emit_dword(emit, opcode0.value);
      emit_dword(emit, operand0.value);
      emit_dword(emit, emit->key.shader_buf_uav_index[i]);
      end_emit_instruction(emit);

      /* Mark the uav is already declared */
      uav_mask |= 1 << emit->key.shader_buf_uav_index[i];
   }

   emit->uav_declared |= uav_mask;
}


/**
 * Emit instruction to declare thread group shared memory(tgsm) for shared memory
 */
static void
emit_memory_declarations(struct svga_shader_emitter_v10 *emit)
{
   if (emit->cs.shared_memory_declared) {
      VGPU10OpcodeToken0 opcode0;
      VGPU10OperandToken0 operand0;

      opcode0.value = 0;
      opcode0.opcodeType = VGPU10_OPCODE_DCL_TGSM_RAW;

      /* Declare the uav as global coherent if the shader includes memory
       * barrier instructions.
       */
      opcode0.globallyCoherent =
         (emit->info.opcode_count[TGSI_OPCODE_MEMBAR] > 0) ? 1 : 0;

      operand0.value = 0;
      operand0.numComponents = VGPU10_OPERAND_0_COMPONENT;
      operand0.operandType = VGPU10_OPERAND_TYPE_THREAD_GROUP_SHARED_MEMORY;
      operand0.indexDimension = VGPU10_OPERAND_INDEX_1D;
      operand0.index0Representation = VGPU10_OPERAND_INDEX_IMMEDIATE32;

      begin_emit_instruction(emit);
      emit_dword(emit, opcode0.value);
      emit_dword(emit, operand0.value);

      /* Current state tracker only declares one shared memory for GLSL.
       * Use index 0 for this shared memory.
       */
      emit_dword(emit, 0);
      emit_dword(emit, emit->key.cs.mem_size); /* byte Count */
      end_emit_instruction(emit);
   }
}


/**
 * Emit instruction to declare uav for atomic buffers
 */
static void
emit_atomic_buf_declarations(struct svga_shader_emitter_v10 *emit)
{
   unsigned atomic_bufs_mask = emit->atomic_bufs_mask;
   unsigned uav_mask = 0;

   /* Emit uav decl for each atomic buffer */
   while (atomic_bufs_mask) {
      unsigned buf_index = u_bit_scan(&atomic_bufs_mask);
      unsigned uav_index = emit->key.atomic_buf_uav_index[buf_index];

      /* If the corresponding uav for the shader buf is already declared,
       * skip this shader buffer declaration.
       */
      if (uav_mask & (1 << uav_index))
         continue;

      VGPU10OpcodeToken0 opcode0;
      VGPU10OperandToken0 operand0;

      assert(uav_index != SVGA3D_INVALID_ID);

      opcode0.value = 0;
      opcode0.opcodeType = VGPU10_OPCODE_DCL_UAV_RAW;
      opcode0.uavResourceDimension = VGPU10_RESOURCE_DIMENSION_BUFFER;

      /* Declare the uav as global coherent if the shader includes memory
       * barrier instructions.
       */
      opcode0.globallyCoherent =
         (emit->info.opcode_count[TGSI_OPCODE_MEMBAR] > 0) ? 1 : 0;
      opcode0.uavHasCounter = 1;

      operand0.value = 0;
      operand0.numComponents = VGPU10_OPERAND_0_COMPONENT;
      operand0.operandType = VGPU10_OPERAND_TYPE_UAV;
      operand0.indexDimension = VGPU10_OPERAND_INDEX_1D;
      operand0.index0Representation = VGPU10_OPERAND_INDEX_IMMEDIATE32;

      begin_emit_instruction(emit);
      emit_dword(emit, opcode0.value);
      emit_dword(emit, operand0.value);
      emit_dword(emit, uav_index);
      end_emit_instruction(emit);

      /* Mark the uav is already declared */
      uav_mask |= 1 << uav_index;
   }

   emit->uav_declared |= uav_mask;

   /* Allocate immediates to be used for index to the atomic buffers */
   unsigned j = 0;
   for (unsigned i = 0; i <= emit->num_atomic_bufs / 4; i++, j+=4) {
      alloc_immediate_int4(emit, j+0, j+1, j+2, j+3);
   }

   /* Allocate immediates for the atomic counter index */
   for (; j <= emit->max_atomic_counter_index; j+=4) {
      alloc_immediate_int4(emit, j+0, j+1, j+2, j+3);
   }
}


/**
 * Emit instruction with n=1, 2 or 3 source registers.
 */
static void
emit_instruction_opn(struct svga_shader_emitter_v10 *emit,
                     unsigned opcode,
                     const struct tgsi_full_dst_register *dst,
                     const struct tgsi_full_src_register *src1,
                     const struct tgsi_full_src_register *src2,
                     const struct tgsi_full_src_register *src3,
                     bool saturate, bool precise)
{
   begin_emit_instruction(emit);
   emit_opcode_precise(emit, opcode, saturate, precise);
   emit_dst_register(emit, dst);
   emit_src_register(emit, src1);
   if (src2) {
      emit_src_register(emit, src2);
   }
   if (src3) {
      emit_src_register(emit, src3);
   }
   end_emit_instruction(emit);
}

static void
emit_instruction_op1(struct svga_shader_emitter_v10 *emit,
                     unsigned opcode,
                     const struct tgsi_full_dst_register *dst,
                     const struct tgsi_full_src_register *src)
{
   emit_instruction_opn(emit, opcode, dst, src, NULL, NULL, false, false);
}

static void
emit_instruction_op2(struct svga_shader_emitter_v10 *emit,
                     VGPU10_OPCODE_TYPE opcode,
                     const struct tgsi_full_dst_register *dst,
                     const struct tgsi_full_src_register *src1,
                     const struct tgsi_full_src_register *src2)
{
   emit_instruction_opn(emit, opcode, dst, src1, src2, NULL, false, false);
}

static void
emit_instruction_op3(struct svga_shader_emitter_v10 *emit,
                     VGPU10_OPCODE_TYPE opcode,
                     const struct tgsi_full_dst_register *dst,
                     const struct tgsi_full_src_register *src1,
                     const struct tgsi_full_src_register *src2,
                     const struct tgsi_full_src_register *src3)
{
   emit_instruction_opn(emit, opcode, dst, src1, src2, src3, false, false);
}

static void
emit_instruction_op0(struct svga_shader_emitter_v10 *emit,
                     VGPU10_OPCODE_TYPE opcode)
{
   begin_emit_instruction(emit);
   emit_opcode(emit, opcode, false);
   end_emit_instruction(emit);
}

/**
 * Tessellation inner/outer levels needs to be store into its
 * appropriate registers depending on prim_mode.
 */
static void
store_tesslevels(struct svga_shader_emitter_v10 *emit)
{
   int i;

   /* tessellation levels are required input/out in hull shader.
    * emitting the inner/outer tessellation levels, either from
    * values provided in tcs or fallback default values which is 1.0
    */
   if (emit->key.tcs.prim_mode == MESA_PRIM_QUADS) {
      struct tgsi_full_src_register temp_src;

      if (emit->tcs.inner.tgsi_index != INVALID_INDEX)
         temp_src = make_src_temp_reg(emit->tcs.inner.temp_index);
      else
         temp_src = make_immediate_reg_float(emit, 1.0f);

      for (i = 0; i < 2; i++) {
         struct tgsi_full_src_register src =
            scalar_src(&temp_src, TGSI_SWIZZLE_X + i);
         struct tgsi_full_dst_register dst =
            make_dst_reg(TGSI_FILE_OUTPUT, emit->tcs.inner.out_index + i);
         dst = writemask_dst(&dst, TGSI_WRITEMASK_X);
         emit_instruction_op1(emit, VGPU10_OPCODE_MOV, &dst, &src);
      }

      if (emit->tcs.outer.tgsi_index != INVALID_INDEX)
         temp_src = make_src_temp_reg(emit->tcs.outer.temp_index);
      else
         temp_src = make_immediate_reg_float(emit, 1.0f);

      for (i = 0; i < 4; i++) {
         struct tgsi_full_src_register src =
            scalar_src(&temp_src, TGSI_SWIZZLE_X + i);
         struct tgsi_full_dst_register dst =
            make_dst_reg(TGSI_FILE_OUTPUT, emit->tcs.outer.out_index + i);
         dst = writemask_dst(&dst, TGSI_WRITEMASK_X);
         emit_instruction_op1(emit, VGPU10_OPCODE_MOV, &dst, &src);
      }
   }
   else if (emit->key.tcs.prim_mode == MESA_PRIM_TRIANGLES) {
      struct tgsi_full_src_register temp_src;

      if (emit->tcs.inner.tgsi_index != INVALID_INDEX)
         temp_src = make_src_temp_reg(emit->tcs.inner.temp_index);
      else
         temp_src = make_immediate_reg_float(emit, 1.0f);

      struct tgsi_full_src_register src =
         scalar_src(&temp_src, TGSI_SWIZZLE_X);
      struct tgsi_full_dst_register dst =
         make_dst_reg(TGSI_FILE_OUTPUT, emit->tcs.inner.out_index);
      dst = writemask_dst(&dst, TGSI_WRITEMASK_X);
      emit_instruction_op1(emit, VGPU10_OPCODE_MOV, &dst, &src);

      if (emit->tcs.outer.tgsi_index != INVALID_INDEX)
         temp_src = make_src_temp_reg(emit->tcs.outer.temp_index);
      else
         temp_src = make_immediate_reg_float(emit, 1.0f);

      for (i = 0; i < 3; i++) {
         struct tgsi_full_src_register src =
            scalar_src(&temp_src, TGSI_SWIZZLE_X + i);
         struct tgsi_full_dst_register dst =
            make_dst_reg(TGSI_FILE_OUTPUT, emit->tcs.outer.out_index + i);
         dst = writemask_dst(&dst, TGSI_WRITEMASK_X);
         emit_instruction_op1(emit, VGPU10_OPCODE_MOV, &dst, &src);
      }
   }
   else if (emit->key.tcs.prim_mode ==  MESA_PRIM_LINES) {
      if (emit->tcs.outer.tgsi_index != INVALID_INDEX) {
         struct tgsi_full_src_register temp_src =
            make_src_temp_reg(emit->tcs.outer.temp_index);
         for (i = 0; i < 2; i++) {
            struct tgsi_full_src_register src =
               scalar_src(&temp_src, TGSI_SWIZZLE_X + i);
            struct tgsi_full_dst_register dst =
               make_dst_reg(TGSI_FILE_OUTPUT,
                            emit->tcs.outer.out_index + i);
            dst = writemask_dst(&dst, TGSI_WRITEMASK_X);
            emit_instruction_op1(emit, VGPU10_OPCODE_MOV, &dst, &src);
         }
      }
   }
   else {
      debug_printf("Unsupported primitive type");
   }
}


/**
 * Emit the actual clip distance instructions to be used for clipping
 * by copying the clip distance from the temporary registers to the
 * CLIPDIST registers written with the enabled planes mask.
 * Also copy the clip distance from the temporary to the clip distance
 * shadow copy register which will be referenced by the input shader
 */
static void
emit_clip_distance_instructions(struct svga_shader_emitter_v10 *emit)
{
   struct tgsi_full_src_register tmp_clip_dist_src;
   struct tgsi_full_dst_register clip_dist_dst;

   unsigned i;
   unsigned clip_plane_enable = emit->key.clip_plane_enable;
   unsigned clip_dist_tmp_index = emit->clip_dist_tmp_index;
   int num_written_clipdist = emit->info.num_written_clipdistance;

   assert(emit->clip_dist_out_index != INVALID_INDEX);
   assert(emit->clip_dist_tmp_index != INVALID_INDEX);

   /**
    * Temporary reset the temporary clip dist register index so
    * that the copy to the real clip dist register will not
    * attempt to copy to the temporary register again
    */
   emit->clip_dist_tmp_index = INVALID_INDEX;

   for (i = 0; i < 2 && num_written_clipdist > 0; i++, num_written_clipdist-=4) {

      tmp_clip_dist_src = make_src_temp_reg(clip_dist_tmp_index + i);

      /**
       * copy to the shadow copy for use by varying variable and
       * stream output. All clip distances
       * will be written regardless of the enabled clipping planes.
       */
      clip_dist_dst = make_dst_reg(TGSI_FILE_OUTPUT,
                                   emit->clip_dist_so_index + i);

      /* MOV clip_dist_so, tmp_clip_dist */
      emit_instruction_op1(emit, VGPU10_OPCODE_MOV, &clip_dist_dst,
                           &tmp_clip_dist_src);

      /**
       * copy those clip distances to enabled clipping planes
       * to CLIPDIST registers for clipping
       */
      if (clip_plane_enable & 0xf) {
         clip_dist_dst = make_dst_reg(TGSI_FILE_OUTPUT,
                                      emit->clip_dist_out_index + i);
         clip_dist_dst = writemask_dst(&clip_dist_dst, clip_plane_enable & 0xf);

         /* MOV CLIPDIST, tmp_clip_dist */
         emit_instruction_op1(emit, VGPU10_OPCODE_MOV, &clip_dist_dst,
                              &tmp_clip_dist_src);
      }
      /* four clip planes per clip register */
      clip_plane_enable >>= 4;
   }
   /**
    * set the temporary clip dist register index back to the
    * temporary index for the next vertex
    */
   emit->clip_dist_tmp_index = clip_dist_tmp_index;
}

/* Declare clip distance output registers for user-defined clip planes
 * or the TGSI_CLIPVERTEX output.
 */
static void
emit_clip_distance_declarations(struct svga_shader_emitter_v10 *emit)
{
   unsigned num_clip_planes = util_bitcount(emit->key.clip_plane_enable);
   unsigned index = emit->num_outputs;
   unsigned plane_mask;

   assert(emit->unit != PIPE_SHADER_FRAGMENT);
   assert(num_clip_planes <= 8);

   if (emit->clip_mode != CLIP_LEGACY &&
       emit->clip_mode != CLIP_VERTEX) {
      return;
   }

   if (num_clip_planes == 0)
      return;

   /* Convert clip vertex to clip distances only in the last vertex stage */
   if (!emit->key.last_vertex_stage)
      return;

   /* Declare one or two clip output registers.  The number of components
    * in the mask reflects the number of clip planes.  For example, if 5
    * clip planes are needed, we'll declare outputs similar to:
    * dcl_output_siv o2.xyzw, clip_distance
    * dcl_output_siv o3.x, clip_distance
    */
   emit->clip_dist_out_index = index; /* save the starting clip dist reg index */

   plane_mask = (1 << num_clip_planes) - 1;
   if (plane_mask & 0xf) {
      unsigned cmask = plane_mask & VGPU10_OPERAND_4_COMPONENT_MASK_ALL;
      emit_output_declaration(emit, VGPU10_OPCODE_DCL_OUTPUT_SIV, index,
                              VGPU10_NAME_CLIP_DISTANCE, cmask, true,
                              SVGADX_SIGNATURE_SEMANTIC_NAME_CLIP_DISTANCE);
      emit->num_outputs++;
   }
   if (plane_mask & 0xf0) {
      unsigned cmask = (plane_mask >> 4) & VGPU10_OPERAND_4_COMPONENT_MASK_ALL;
      emit_output_declaration(emit, VGPU10_OPCODE_DCL_OUTPUT_SIV, index + 1,
                              VGPU10_NAME_CLIP_DISTANCE, cmask, true,
                              SVGADX_SIGNATURE_SEMANTIC_NAME_CLIP_DISTANCE);
      emit->num_outputs++;
   }
}


/**
 * Emit the instructions for writing to the clip distance registers
 * to handle legacy/automatic clip planes.
 * For each clip plane, the distance is the dot product of the vertex
 * position (found in TEMP[vpos_tmp_index]) and the clip plane coefficients.
 * This is not used when the shader has an explicit CLIPVERTEX or CLIPDISTANCE
 * output registers already declared.
 */
static void
emit_clip_distance_from_vpos(struct svga_shader_emitter_v10 *emit,
                             unsigned vpos_tmp_index)
{
   unsigned i, num_clip_planes = util_bitcount(emit->key.clip_plane_enable);

   assert(emit->clip_mode == CLIP_LEGACY);
   assert(num_clip_planes <= 8);

   assert(emit->unit == PIPE_SHADER_VERTEX ||
          emit->unit == PIPE_SHADER_GEOMETRY ||
          emit->unit == PIPE_SHADER_TESS_EVAL);

   for (i = 0; i < num_clip_planes; i++) {
      struct tgsi_full_dst_register dst;
      struct tgsi_full_src_register plane_src, vpos_src;
      unsigned reg_index = emit->clip_dist_out_index + i / 4;
      unsigned comp = i % 4;
      unsigned writemask = VGPU10_OPERAND_4_COMPONENT_MASK_X << comp;

      /* create dst, src regs */
      dst = make_dst_reg(TGSI_FILE_OUTPUT, reg_index);
      dst = writemask_dst(&dst, writemask);

      plane_src = make_src_const_reg(emit->clip_plane_const[i]);
      vpos_src = make_src_temp_reg(vpos_tmp_index);

      /* DP4 clip_dist, plane, vpos */
      emit_instruction_op2(emit, VGPU10_OPCODE_DP4, &dst,
                           &plane_src, &vpos_src);
   }
}


/**
 * Emit the instructions for computing the clip distance results from
 * the clip vertex temporary.
 * For each clip plane, the distance is the dot product of the clip vertex
 * position (found in a temp reg) and the clip plane coefficients.
 */
static void
emit_clip_vertex_instructions(struct svga_shader_emitter_v10 *emit)
{
   const unsigned num_clip = util_bitcount(emit->key.clip_plane_enable);
   unsigned i;
   struct tgsi_full_dst_register dst;
   struct tgsi_full_src_register clipvert_src;
   const unsigned clip_vertex_tmp = emit->clip_vertex_tmp_index;

   assert(emit->unit == PIPE_SHADER_VERTEX ||
          emit->unit == PIPE_SHADER_GEOMETRY ||
          emit->unit == PIPE_SHADER_TESS_EVAL);

   assert(emit->clip_mode == CLIP_VERTEX);

   clipvert_src = make_src_temp_reg(clip_vertex_tmp);

   for (i = 0; i < num_clip; i++) {
      struct tgsi_full_src_register plane_src;
      unsigned reg_index = emit->clip_dist_out_index + i / 4;
      unsigned comp = i % 4;
      unsigned writemask = VGPU10_OPERAND_4_COMPONENT_MASK_X << comp;

      /* create dst, src regs */
      dst = make_dst_reg(TGSI_FILE_OUTPUT, reg_index);
      dst = writemask_dst(&dst, writemask);

      plane_src = make_src_const_reg(emit->clip_plane_const[i]);

      /* DP4 clip_dist, plane, vpos */
      emit_instruction_op2(emit, VGPU10_OPCODE_DP4, &dst,
                           &plane_src, &clipvert_src);
   }

   /* copy temporary clip vertex register to the clip vertex register */

   assert(emit->clip_vertex_out_index != INVALID_INDEX);

   /**
    * temporary reset the temporary clip vertex register index so
    * that copy to the clip vertex register will not attempt
    * to copy to the temporary register again
    */
   emit->clip_vertex_tmp_index = INVALID_INDEX;

   /* MOV clip_vertex, clip_vertex_tmp */
   dst = make_dst_reg(TGSI_FILE_OUTPUT, emit->clip_vertex_out_index);
   emit_instruction_op1(emit, VGPU10_OPCODE_MOV,
                        &dst, &clipvert_src);

   /**
    * set the temporary clip vertex register index back to the
    * temporary index for the next vertex
    */
   emit->clip_vertex_tmp_index = clip_vertex_tmp;
}

/**
 * Emit code to convert RGBA to BGRA
 */
static void
emit_swap_r_b(struct svga_shader_emitter_v10 *emit,
                     const struct tgsi_full_dst_register *dst,
                     const struct tgsi_full_src_register *src)
{
   struct tgsi_full_src_register bgra_src =
      swizzle_src(src, TGSI_SWIZZLE_Z, TGSI_SWIZZLE_Y, TGSI_SWIZZLE_X, TGSI_SWIZZLE_W);

   begin_emit_instruction(emit);
   emit_opcode(emit, VGPU10_OPCODE_MOV, false);
   emit_dst_register(emit, dst);
   emit_src_register(emit, &bgra_src);
   end_emit_instruction(emit);
}


/** Convert from 10_10_10_2 normalized to 10_10_10_2_snorm */
static void
emit_puint_to_snorm(struct svga_shader_emitter_v10 *emit,
                    const struct tgsi_full_dst_register *dst,
                    const struct tgsi_full_src_register *src)
{
   struct tgsi_full_src_register half = make_immediate_reg_float(emit, 0.5f);
   struct tgsi_full_src_register two =
      make_immediate_reg_float4(emit, 2.0f, 2.0f, 2.0f, 3.0f);
   struct tgsi_full_src_register neg_two =
      make_immediate_reg_float4(emit, -2.0f, -2.0f, -2.0f, -1.66666f);

   unsigned val_tmp = get_temp_index(emit);
   struct tgsi_full_dst_register val_dst = make_dst_temp_reg(val_tmp);
   struct tgsi_full_src_register val_src = make_src_temp_reg(val_tmp);

   unsigned bias_tmp = get_temp_index(emit);
   struct tgsi_full_dst_register bias_dst = make_dst_temp_reg(bias_tmp);
   struct tgsi_full_src_register bias_src = make_src_temp_reg(bias_tmp);

   /* val = src * 2.0 */
   emit_instruction_op2(emit, VGPU10_OPCODE_MUL, &val_dst, src, &two);

   /* bias = src > 0.5 */
   emit_instruction_op2(emit, VGPU10_OPCODE_GE, &bias_dst, src, &half);

   /* bias = bias & -2.0 */
   emit_instruction_op2(emit, VGPU10_OPCODE_AND, &bias_dst,
                        &bias_src, &neg_two);

   /* dst = val + bias */
   emit_instruction_op2(emit, VGPU10_OPCODE_ADD, dst,
                        &val_src, &bias_src);

   free_temp_indexes(emit);
}


/** Convert from 10_10_10_2_unorm to 10_10_10_2_uscaled */
static void
emit_puint_to_uscaled(struct svga_shader_emitter_v10 *emit,
                      const struct tgsi_full_dst_register *dst,
                      const struct tgsi_full_src_register *src)
{
   struct tgsi_full_src_register scale =
      make_immediate_reg_float4(emit, 1023.0f, 1023.0f, 1023.0f, 3.0f);

   /* dst = src * scale */
   emit_instruction_op2(emit, VGPU10_OPCODE_MUL, dst, src, &scale);
}


/** Convert from R32_UINT to 10_10_10_2_sscaled */
static void
emit_puint_to_sscaled(struct svga_shader_emitter_v10 *emit,
                      const struct tgsi_full_dst_register *dst,
                      const struct tgsi_full_src_register *src)
{
   struct tgsi_full_src_register lshift =
      make_immediate_reg_int4(emit, 22, 12, 2, 0);
   struct tgsi_full_src_register rshift =
      make_immediate_reg_int4(emit, 22, 22, 22, 30);

   struct tgsi_full_src_register src_xxxx = scalar_src(src, TGSI_SWIZZLE_X);

   unsigned tmp = get_temp_index(emit);
   struct tgsi_full_dst_register tmp_dst = make_dst_temp_reg(tmp);
   struct tgsi_full_src_register tmp_src = make_src_temp_reg(tmp);

   /*
    * r = (pixel << 22) >> 22;   # signed int in [511, -512]
    * g = (pixel << 12) >> 22;   # signed int in [511, -512]
    * b = (pixel <<  2) >> 22;   # signed int in [511, -512]
    * a = (pixel <<  0) >> 30;   # signed int in [1, -2]
    * dst = i_to_f(r,g,b,a);     # convert to float
    */
   emit_instruction_op2(emit, VGPU10_OPCODE_ISHL, &tmp_dst,
                        &src_xxxx, &lshift);
   emit_instruction_op2(emit, VGPU10_OPCODE_ISHR, &tmp_dst,
                        &tmp_src, &rshift);
   emit_instruction_op1(emit, VGPU10_OPCODE_ITOF, dst, &tmp_src);

   free_temp_indexes(emit);
}


/**
 * Emit code for TGSI_OPCODE_ARL or TGSI_OPCODE_UARL instruction.
 */
static bool
emit_arl_uarl(struct svga_shader_emitter_v10 *emit,
              const struct tgsi_full_instruction *inst)
{
   unsigned index = inst->Dst[0].Register.Index;
   struct tgsi_full_dst_register dst;
   VGPU10_OPCODE_TYPE opcode;

   assert(index < MAX_VGPU10_ADDR_REGS);
   dst = make_dst_temp_reg(emit->address_reg_index[index]);
   dst = writemask_dst(&dst, inst->Dst[0].Register.WriteMask);

   /* ARL dst, s0
    * Translates into:
    * FTOI address_tmp, s0
    *
    * UARL dst, s0
    * Translates into:
    * MOV address_tmp, s0
    */
   if (inst->Instruction.Opcode == TGSI_OPCODE_ARL)
      opcode = VGPU10_OPCODE_FTOI;
   else
      opcode = VGPU10_OPCODE_MOV;

   emit_instruction_op1(emit, opcode, &dst, &inst->Src[0]);

   return true;
}


/**
 * Emit code for TGSI_OPCODE_CAL instruction.
 */
static bool
emit_cal(struct svga_shader_emitter_v10 *emit,
         const struct tgsi_full_instruction *inst)
{
   unsigned label = inst->Label.Label;
   VGPU10OperandToken0 operand;
   operand.value = 0;
   operand.operandType = VGPU10_OPERAND_TYPE_LABEL;

   begin_emit_instruction(emit);
   emit_dword(emit, operand.value);
   emit_dword(emit, label);
   end_emit_instruction(emit);

   return true;
}


/**
 * Emit code for TGSI_OPCODE_IABS instruction.
 */
static bool
emit_iabs(struct svga_shader_emitter_v10 *emit,
          const struct tgsi_full_instruction *inst)
{
   /* dst.x = (src0.x < 0) ? -src0.x : src0.x
    * dst.y = (src0.y < 0) ? -src0.y : src0.y
    * dst.z = (src0.z < 0) ? -src0.z : src0.z
    * dst.w = (src0.w < 0) ? -src0.w : src0.w
    *
    * Translates into
    *   IMAX dst, src, neg(src)
    */
   struct tgsi_full_src_register neg_src = negate_src(&inst->Src[0]);
   emit_instruction_op2(emit, VGPU10_OPCODE_IMAX, &inst->Dst[0],
                        &inst->Src[0], &neg_src);

   return true;
}


/**
 * Emit code for TGSI_OPCODE_CMP instruction.
 */
static bool
emit_cmp(struct svga_shader_emitter_v10 *emit,
         const struct tgsi_full_instruction *inst)
{
   /* dst.x = (src0.x < 0) ? src1.x : src2.x
    * dst.y = (src0.y < 0) ? src1.y : src2.y
    * dst.z = (src0.z < 0) ? src1.z : src2.z
    * dst.w = (src0.w < 0) ? src1.w : src2.w
    *
    * Translates into
    *   LT tmp, src0, 0.0
    *   MOVC dst, tmp, src1, src2
    */
   struct tgsi_full_src_register zero = make_immediate_reg_float(emit, 0.0f);
   unsigned tmp = get_temp_index(emit);
   struct tgsi_full_src_register tmp_src = make_src_temp_reg(tmp);
   struct tgsi_full_dst_register tmp_dst = make_dst_temp_reg(tmp);

   emit_instruction_opn(emit, VGPU10_OPCODE_LT, &tmp_dst,
                        &inst->Src[0], &zero, NULL, false,
                        inst->Instruction.Precise);
   emit_instruction_opn(emit, VGPU10_OPCODE_MOVC, &inst->Dst[0],
                        &tmp_src, &inst->Src[1], &inst->Src[2],
                        inst->Instruction.Saturate, false);

   free_temp_indexes(emit);

   return true;
}


/**
 * Emit code for TGSI_OPCODE_DST instruction.
 */
static bool
emit_dst(struct svga_shader_emitter_v10 *emit,
         const struct tgsi_full_instruction *inst)
{
   /*
    * dst.x = 1
    * dst.y = src0.y * src1.y
    * dst.z = src0.z
    * dst.w = src1.w
    */

   struct tgsi_full_src_register s0_yyyy =
      scalar_src(&inst->Src[0], TGSI_SWIZZLE_Y);
   struct tgsi_full_src_register s0_zzzz =
      scalar_src(&inst->Src[0], TGSI_SWIZZLE_Z);
   struct tgsi_full_src_register s1_yyyy =
      scalar_src(&inst->Src[1], TGSI_SWIZZLE_Y);
   struct tgsi_full_src_register s1_wwww =
      scalar_src(&inst->Src[1], TGSI_SWIZZLE_W);

   /*
    * If dst and either src0 and src1 are the same we need
    * to create a temporary for it and insert a extra move.
    */
   unsigned tmp_move = get_temp_index(emit);
   struct tgsi_full_src_register move_src = make_src_temp_reg(tmp_move);
   struct tgsi_full_dst_register move_dst = make_dst_temp_reg(tmp_move);

   /* MOV dst.x, 1.0 */
   if (inst->Dst[0].Register.WriteMask & TGSI_WRITEMASK_X) {
      struct tgsi_full_dst_register dst_x =
         writemask_dst(&move_dst, TGSI_WRITEMASK_X);
      struct tgsi_full_src_register one = make_immediate_reg_float(emit, 1.0f);

      emit_instruction_op1(emit, VGPU10_OPCODE_MOV, &dst_x, &one);
   }

   /* MUL dst.y, s0.y, s1.y */
   if (inst->Dst[0].Register.WriteMask & TGSI_WRITEMASK_Y) {
      struct tgsi_full_dst_register dst_y =
         writemask_dst(&move_dst, TGSI_WRITEMASK_Y);

      emit_instruction_opn(emit, VGPU10_OPCODE_MUL, &dst_y, &s0_yyyy,
                           &s1_yyyy, NULL, inst->Instruction.Saturate,
                           inst->Instruction.Precise);
   }

   /* MOV dst.z, s0.z */
   if (inst->Dst[0].Register.WriteMask & TGSI_WRITEMASK_Z) {
      struct tgsi_full_dst_register dst_z =
         writemask_dst(&move_dst, TGSI_WRITEMASK_Z);

      emit_instruction_opn(emit, VGPU10_OPCODE_MOV,
                           &dst_z, &s0_zzzz, NULL, NULL,
                           inst->Instruction.Saturate,
                           inst->Instruction.Precise);
  }

   /* MOV dst.w, s1.w */
   if (inst->Dst[0].Register.WriteMask & TGSI_WRITEMASK_W) {
      struct tgsi_full_dst_register dst_w =
         writemask_dst(&move_dst, TGSI_WRITEMASK_W);

      emit_instruction_opn(emit, VGPU10_OPCODE_MOV,
                           &dst_w, &s1_wwww, NULL, NULL,
                           inst->Instruction.Saturate,
                           inst->Instruction.Precise);
   }

   emit_instruction_op1(emit, VGPU10_OPCODE_MOV, &inst->Dst[0], &move_src);
   free_temp_indexes(emit);

   return true;
}


/**
 * A helper function to return the stream index as specified in
 * the immediate register
 */
static inline unsigned
find_stream_index(struct svga_shader_emitter_v10 *emit,
                  const struct tgsi_full_src_register *src)
{
   return emit->immediates[src->Register.Index][src->Register.SwizzleX].Int;
}


/**
 * Emit code for TGSI_OPCODE_ENDPRIM (GS only)
 */
static bool
emit_endprim(struct svga_shader_emitter_v10 *emit,
             const struct tgsi_full_instruction *inst)
{
   assert(emit->unit == PIPE_SHADER_GEOMETRY);

   begin_emit_instruction(emit);
   if (emit->version >= 50) {
      unsigned streamIndex = find_stream_index(emit, &inst->Src[0]);

      if (emit->info.num_stream_output_components[streamIndex] == 0) {
         /**
          * If there is no output for this stream, discard this instruction.
          */
         emit->discard_instruction = true;
      }
      else {
         emit_opcode(emit, VGPU10_OPCODE_CUT_STREAM, false);
         assert(inst->Src[0].Register.File == TGSI_FILE_IMMEDIATE);
         emit_stream_register(emit, streamIndex);
      }
   }
   else {
      emit_opcode(emit, VGPU10_OPCODE_CUT, false);
   }
   end_emit_instruction(emit);
   return true;
}


/**
 * Emit code for TGSI_OPCODE_EX2 (2^x) instruction.
 */
static bool
emit_ex2(struct svga_shader_emitter_v10 *emit,
         const struct tgsi_full_instruction *inst)
{
   /* Note that TGSI_OPCODE_EX2 computes only one value from src.x
    * while VGPU10 computes four values.
    *
    * dst = EX2(src):
    *   dst.xyzw = 2.0 ^ src.x
    */

   struct tgsi_full_src_register src_xxxx =
      swizzle_src(&inst->Src[0], TGSI_SWIZZLE_X, TGSI_SWIZZLE_X,
                  TGSI_SWIZZLE_X, TGSI_SWIZZLE_X);

   /* EXP tmp, s0.xxxx */
   emit_instruction_opn(emit, VGPU10_OPCODE_EXP, &inst->Dst[0], &src_xxxx,
                        NULL, NULL,
                        inst->Instruction.Saturate,
                        inst->Instruction.Precise);

   return true;
}


/**
 * Emit code for TGSI_OPCODE_EXP instruction.
 */
static bool
emit_exp(struct svga_shader_emitter_v10 *emit,
         const struct tgsi_full_instruction *inst)
{
   /*
    * dst.x = 2 ^ floor(s0.x)
    * dst.y = s0.x - floor(s0.x)
    * dst.z = 2 ^ s0.x
    * dst.w = 1.0
    */

   struct tgsi_full_src_register src_xxxx =
      scalar_src(&inst->Src[0], TGSI_SWIZZLE_X);
   unsigned tmp = get_temp_index(emit);
   struct tgsi_full_src_register tmp_src = make_src_temp_reg(tmp);
   struct tgsi_full_dst_register tmp_dst = make_dst_temp_reg(tmp);

   /*
    * If dst and src are the same we need to create
    * a temporary for it and insert a extra move.
    */
   unsigned tmp_move = get_temp_index(emit);
   struct tgsi_full_src_register move_src = make_src_temp_reg(tmp_move);
   struct tgsi_full_dst_register move_dst = make_dst_temp_reg(tmp_move);

   /* only use X component of temp reg */
   tmp_dst = writemask_dst(&tmp_dst, TGSI_WRITEMASK_X);
   tmp_src = scalar_src(&tmp_src, TGSI_SWIZZLE_X);

   /* ROUND_NI tmp.x, s0.x */
   emit_instruction_op1(emit, VGPU10_OPCODE_ROUND_NI, &tmp_dst,
                        &src_xxxx); /* round to -infinity */

   /* EXP dst.x, tmp.x */
   if (inst->Dst[0].Register.WriteMask & TGSI_WRITEMASK_X) {
      struct tgsi_full_dst_register dst_x =
         writemask_dst(&move_dst, TGSI_WRITEMASK_X);

      emit_instruction_opn(emit, VGPU10_OPCODE_EXP, &dst_x, &tmp_src,
                           NULL, NULL,
                           inst->Instruction.Saturate,
                           inst->Instruction.Precise);
   }

   /* ADD dst.y, s0.x, -tmp */
   if (inst->Dst[0].Register.WriteMask & TGSI_WRITEMASK_Y) {
      struct tgsi_full_dst_register dst_y =
         writemask_dst(&move_dst, TGSI_WRITEMASK_Y);
      struct tgsi_full_src_register neg_tmp_src = negate_src(&tmp_src);

      emit_instruction_opn(emit, VGPU10_OPCODE_ADD, &dst_y, &src_xxxx,
                           &neg_tmp_src, NULL,
                           inst->Instruction.Saturate,
                           inst->Instruction.Precise);
   }

   /* EXP dst.z, s0.x */
   if (inst->Dst[0].Register.WriteMask & TGSI_WRITEMASK_Z) {
      struct tgsi_full_dst_register dst_z =
         writemask_dst(&move_dst, TGSI_WRITEMASK_Z);

      emit_instruction_opn(emit, VGPU10_OPCODE_EXP, &dst_z, &src_xxxx,
                           NULL, NULL,
                           inst->Instruction.Saturate,
                           inst->Instruction.Precise);
   }

   /* MOV dst.w, 1.0 */
   if (inst->Dst[0].Register.WriteMask & TGSI_WRITEMASK_W) {
      struct tgsi_full_dst_register dst_w =
         writemask_dst(&move_dst, TGSI_WRITEMASK_W);
      struct tgsi_full_src_register one = make_immediate_reg_float(emit, 1.0f);

      emit_instruction_op1(emit, VGPU10_OPCODE_MOV, &dst_w, &one);
   }

   emit_instruction_op1(emit, VGPU10_OPCODE_MOV, &inst->Dst[0], &move_src);

   free_temp_indexes(emit);

   return true;
}


/**
 * Emit code for TGSI_OPCODE_IF instruction.
 */
static bool
emit_if(struct svga_shader_emitter_v10 *emit,
        const struct tgsi_full_src_register *src)
{
   VGPU10OpcodeToken0 opcode0;

   /* The src register should be a scalar */
   assert(src->Register.SwizzleX == src->Register.SwizzleY &&
          src->Register.SwizzleX == src->Register.SwizzleZ &&
          src->Register.SwizzleX == src->Register.SwizzleW);

   /* The only special thing here is that we need to set the
    * VGPU10_INSTRUCTION_TEST_NONZERO flag since we want to test if
    * src.x is non-zero.
    */
   opcode0.value = 0;
   opcode0.opcodeType = VGPU10_OPCODE_IF;
   opcode0.testBoolean = VGPU10_INSTRUCTION_TEST_NONZERO;

   begin_emit_instruction(emit);
   emit_dword(emit, opcode0.value);
   emit_src_register(emit, src);
   end_emit_instruction(emit);

   return true;
}


/**
 * Emit code for conditional discard instruction (discard fragment if any of
 * the register components are negative).
 */
static bool
emit_cond_discard(struct svga_shader_emitter_v10 *emit,
                  const struct tgsi_full_instruction *inst)
{
   unsigned tmp = get_temp_index(emit);
   struct tgsi_full_src_register tmp_src = make_src_temp_reg(tmp);
   struct tgsi_full_dst_register tmp_dst = make_dst_temp_reg(tmp);

   struct tgsi_full_src_register zero = make_immediate_reg_float(emit, 0.0f);

   struct tgsi_full_dst_register tmp_dst_x =
      writemask_dst(&tmp_dst, TGSI_WRITEMASK_X);
   struct tgsi_full_src_register tmp_src_xxxx =
      scalar_src(&tmp_src, TGSI_SWIZZLE_X);

   /* tmp = src[0] < 0.0 */
   emit_instruction_op2(emit, VGPU10_OPCODE_LT, &tmp_dst, &inst->Src[0], &zero);

   if (!same_swizzle_terms(&inst->Src[0])) {
      /* If the swizzle is not XXXX, YYYY, ZZZZ or WWWW we need to
       * logically OR the swizzle terms.  Most uses of this conditional
       * discard instruction only test one channel so it's good to
       * avoid these extra steps.
       */
      struct tgsi_full_src_register tmp_src_yyyy =
         scalar_src(&tmp_src, TGSI_SWIZZLE_Y);
      struct tgsi_full_src_register tmp_src_zzzz =
         scalar_src(&tmp_src, TGSI_SWIZZLE_Z);
      struct tgsi_full_src_register tmp_src_wwww =
         scalar_src(&tmp_src, TGSI_SWIZZLE_W);

      emit_instruction_op2(emit, VGPU10_OPCODE_OR, &tmp_dst_x, &tmp_src_xxxx,
                           &tmp_src_yyyy);
      emit_instruction_op2(emit, VGPU10_OPCODE_OR, &tmp_dst_x, &tmp_src_xxxx,
                           &tmp_src_zzzz);
      emit_instruction_op2(emit, VGPU10_OPCODE_OR, &tmp_dst_x, &tmp_src_xxxx,
                           &tmp_src_wwww);
   }

   begin_emit_instruction(emit);
   emit_discard_opcode(emit, true); /* discard if src0.x is non-zero */
   emit_src_register(emit, &tmp_src_xxxx);
   end_emit_instruction(emit);

   free_temp_indexes(emit);

   return true;
}


/**
 * Emit code for the unconditional discard instruction.
 */
static bool
emit_discard(struct svga_shader_emitter_v10 *emit,
             const struct tgsi_full_instruction *inst)
{
   struct tgsi_full_src_register zero = make_immediate_reg_float(emit, 0.0f);

   /* DISCARD if 0.0 is zero */
   begin_emit_instruction(emit);
   emit_discard_opcode(emit, false);
   emit_src_register(emit, &zero);
   end_emit_instruction(emit);

   return true;
}


/**
 * Emit code for TGSI_OPCODE_LG2 instruction.
 */
static bool
emit_lg2(struct svga_shader_emitter_v10 *emit,
         const struct tgsi_full_instruction *inst)
{
   /* Note that TGSI_OPCODE_LG2 computes only one value from src.x
    * while VGPU10 computes four values.
    *
    * dst = LG2(src):
    *   dst.xyzw = log2(src.x)
    */

   struct tgsi_full_src_register src_xxxx =
      swizzle_src(&inst->Src[0], TGSI_SWIZZLE_X, TGSI_SWIZZLE_X,
                  TGSI_SWIZZLE_X, TGSI_SWIZZLE_X);

   /* LOG tmp, s0.xxxx */
   emit_instruction_opn(emit, VGPU10_OPCODE_LOG,
                        &inst->Dst[0], &src_xxxx, NULL, NULL,
                        inst->Instruction.Saturate,
                        inst->Instruction.Precise);

   return true;
}


/**
 * Emit code for TGSI_OPCODE_LIT instruction.
 */
static bool
emit_lit(struct svga_shader_emitter_v10 *emit,
         const struct tgsi_full_instruction *inst)
{
   struct tgsi_full_src_register one = make_immediate_reg_float(emit, 1.0f);

   /*
    * If dst and src are the same we need to create
    * a temporary for it and insert a extra move.
    */
   unsigned tmp_move = get_temp_index(emit);
   struct tgsi_full_src_register move_src = make_src_temp_reg(tmp_move);
   struct tgsi_full_dst_register move_dst = make_dst_temp_reg(tmp_move);

   /*
    * dst.x = 1
    * dst.y = max(src.x, 0)
    * dst.z = (src.x > 0) ? max(src.y, 0)^{clamp(src.w, -128, 128))} : 0
    * dst.w = 1
    */

   /* MOV dst.x, 1.0 */
   if (inst->Dst[0].Register.WriteMask & TGSI_WRITEMASK_X) {
      struct tgsi_full_dst_register dst_x =
         writemask_dst(&move_dst, TGSI_WRITEMASK_X);
      emit_instruction_op1(emit, VGPU10_OPCODE_MOV, &dst_x, &one);
   }

   /* MOV dst.w, 1.0 */
   if (inst->Dst[0].Register.WriteMask & TGSI_WRITEMASK_W) {
      struct tgsi_full_dst_register dst_w =
         writemask_dst(&move_dst, TGSI_WRITEMASK_W);
      emit_instruction_op1(emit, VGPU10_OPCODE_MOV, &dst_w, &one);
   }

   /* MAX dst.y, src.x, 0.0 */
   if (inst->Dst[0].Register.WriteMask & TGSI_WRITEMASK_Y) {
      struct tgsi_full_dst_register dst_y =
         writemask_dst(&move_dst, TGSI_WRITEMASK_Y);
      struct tgsi_full_src_register zero =
         make_immediate_reg_float(emit, 0.0f);
      struct tgsi_full_src_register src_xxxx =
         swizzle_src(&inst->Src[0], TGSI_SWIZZLE_X, TGSI_SWIZZLE_X,
                     TGSI_SWIZZLE_X, TGSI_SWIZZLE_X);

      emit_instruction_opn(emit, VGPU10_OPCODE_MAX, &dst_y, &src_xxxx,
                           &zero, NULL, inst->Instruction.Saturate, false);
   }

   /*
    * tmp1 = clamp(src.w, -128, 128);
    *   MAX tmp1, src.w, -128
    *   MIN tmp1, tmp1, 128
    *
    * tmp2 = max(tmp2, 0);
    *   MAX tmp2, src.y, 0
    *
    * tmp1 = pow(tmp2, tmp1);
    *   LOG tmp2, tmp2
    *   MUL tmp1, tmp2, tmp1
    *   EXP tmp1, tmp1
    *
    * tmp1 = (src.w == 0) ? 1 : tmp1;
    *   EQ tmp2, 0, src.w
    *   MOVC tmp1, tmp2, 1.0, tmp1
    *
    * dst.z = (0 < src.x) ? tmp1 : 0;
    *   LT tmp2, 0, src.x
    *   MOVC dst.z, tmp2, tmp1, 0.0
    */
   if (inst->Dst[0].Register.WriteMask & TGSI_WRITEMASK_Z) {
      struct tgsi_full_dst_register dst_z =
         writemask_dst(&move_dst, TGSI_WRITEMASK_Z);

      unsigned tmp1 = get_temp_index(emit);
      struct tgsi_full_src_register tmp1_src = make_src_temp_reg(tmp1);
      struct tgsi_full_dst_register tmp1_dst = make_dst_temp_reg(tmp1);
      unsigned tmp2 = get_temp_index(emit);
      struct tgsi_full_src_register tmp2_src = make_src_temp_reg(tmp2);
      struct tgsi_full_dst_register tmp2_dst = make_dst_temp_reg(tmp2);

      struct tgsi_full_src_register src_xxxx =
         scalar_src(&inst->Src[0], TGSI_SWIZZLE_X);
      struct tgsi_full_src_register src_yyyy =
         scalar_src(&inst->Src[0], TGSI_SWIZZLE_Y);
      struct tgsi_full_src_register src_wwww =
         scalar_src(&inst->Src[0], TGSI_SWIZZLE_W);

      struct tgsi_full_src_register zero =
         make_immediate_reg_float(emit, 0.0f);
      struct tgsi_full_src_register lowerbound =
         make_immediate_reg_float(emit, -128.0f);
      struct tgsi_full_src_register upperbound =
         make_immediate_reg_float(emit, 128.0f);

      emit_instruction_op2(emit, VGPU10_OPCODE_MAX, &tmp1_dst, &src_wwww,
                           &lowerbound);
      emit_instruction_op2(emit, VGPU10_OPCODE_MIN, &tmp1_dst, &tmp1_src,
                           &upperbound);
      emit_instruction_op2(emit, VGPU10_OPCODE_MAX, &tmp2_dst, &src_yyyy,
                           &zero);

      /* POW tmp1, tmp2, tmp1 */
      /* LOG tmp2, tmp2 */
      emit_instruction_op1(emit, VGPU10_OPCODE_LOG, &tmp2_dst, &tmp2_src);

      /* MUL tmp1, tmp2, tmp1 */
      emit_instruction_op2(emit, VGPU10_OPCODE_MUL, &tmp1_dst, &tmp2_src,
                           &tmp1_src);

      /* EXP tmp1, tmp1 */
      emit_instruction_op1(emit, VGPU10_OPCODE_EXP, &tmp1_dst, &tmp1_src);

      /* EQ tmp2, 0, src.w */
      emit_instruction_op2(emit, VGPU10_OPCODE_EQ, &tmp2_dst, &zero, &src_wwww);
      /* MOVC tmp1.z, tmp2, tmp1, 1.0 */
      emit_instruction_op3(emit, VGPU10_OPCODE_MOVC, &tmp1_dst,
                           &tmp2_src, &one, &tmp1_src);

      /* LT tmp2, 0, src.x */
      emit_instruction_op2(emit, VGPU10_OPCODE_LT, &tmp2_dst, &zero, &src_xxxx);
      /* MOVC dst.z, tmp2, tmp1, 0.0 */
      emit_instruction_op3(emit, VGPU10_OPCODE_MOVC, &dst_z,
                           &tmp2_src, &tmp1_src, &zero);
   }

   emit_instruction_op1(emit, VGPU10_OPCODE_MOV, &inst->Dst[0], &move_src);
   free_temp_indexes(emit);

   return true;
}


/**
 * Emit Level Of Detail Query (LODQ) instruction.
 */
static bool
emit_lodq(struct svga_shader_emitter_v10 *emit,
          const struct tgsi_full_instruction *inst)
{
   const uint unit = inst->Src[1].Register.Index;

   assert(emit->version >= 41);

   /* LOD dst, coord, resource, sampler */
   begin_emit_instruction(emit);
   emit_opcode(emit, VGPU10_OPCODE_LOD, false);
   emit_dst_register(emit, &inst->Dst[0]);
   emit_src_register(emit, &inst->Src[0]); /* coord */
   emit_resource_register(emit, unit);
   emit_sampler_register(emit, unit);
   end_emit_instruction(emit);

   return true;
}


/**
 * Emit code for TGSI_OPCODE_LOG instruction.
 */
static bool
emit_log(struct svga_shader_emitter_v10 *emit,
         const struct tgsi_full_instruction *inst)
{
   /*
    * dst.x = floor(lg2(abs(s0.x)))
    * dst.y = abs(s0.x) / (2 ^ floor(lg2(abs(s0.x))))
    * dst.z = lg2(abs(s0.x))
    * dst.w = 1.0
    */

   struct tgsi_full_src_register src_xxxx =
      scalar_src(&inst->Src[0], TGSI_SWIZZLE_X);
   unsigned tmp = get_temp_index(emit);
   struct tgsi_full_src_register tmp_src = make_src_temp_reg(tmp);
   struct tgsi_full_dst_register tmp_dst = make_dst_temp_reg(tmp);
   struct tgsi_full_src_register abs_src_xxxx = absolute_src(&src_xxxx);

   /* only use X component of temp reg */
   tmp_dst = writemask_dst(&tmp_dst, TGSI_WRITEMASK_X);
   tmp_src = scalar_src(&tmp_src, TGSI_SWIZZLE_X);

   /* LOG tmp.x, abs(s0.x) */
   if (inst->Dst[0].Register.WriteMask & TGSI_WRITEMASK_XYZ) {
      emit_instruction_op1(emit, VGPU10_OPCODE_LOG, &tmp_dst, &abs_src_xxxx);
   }

   /* MOV dst.z, tmp.x */
   if (inst->Dst[0].Register.WriteMask & TGSI_WRITEMASK_Z) {
      struct tgsi_full_dst_register dst_z =
         writemask_dst(&inst->Dst[0], TGSI_WRITEMASK_Z);

      emit_instruction_opn(emit, VGPU10_OPCODE_MOV,
                           &dst_z, &tmp_src, NULL, NULL,
                           inst->Instruction.Saturate, false);
   }

   /* FLR tmp.x, tmp.x */
   if (inst->Dst[0].Register.WriteMask & TGSI_WRITEMASK_XY) {
      emit_instruction_op1(emit, VGPU10_OPCODE_ROUND_NI, &tmp_dst, &tmp_src);
   }

   /* MOV dst.x, tmp.x */
   if (inst->Dst[0].Register.WriteMask & TGSI_WRITEMASK_X) {
      struct tgsi_full_dst_register dst_x =
         writemask_dst(&inst->Dst[0], TGSI_WRITEMASK_X);

      emit_instruction_opn(emit, VGPU10_OPCODE_MOV,
                           &dst_x, &tmp_src, NULL, NULL,
                           inst->Instruction.Saturate, false);
   }

   /* EXP tmp.x, tmp.x */
   /* DIV dst.y, abs(s0.x), tmp.x */
   if (inst->Dst[0].Register.WriteMask & TGSI_WRITEMASK_Y) {
      struct tgsi_full_dst_register dst_y =
         writemask_dst(&inst->Dst[0], TGSI_WRITEMASK_Y);

      emit_instruction_op1(emit, VGPU10_OPCODE_EXP, &tmp_dst, &tmp_src);
      emit_instruction_opn(emit, VGPU10_OPCODE_DIV, &dst_y, &abs_src_xxxx,
                           &tmp_src, NULL, inst->Instruction.Saturate, false);
   }

   /* MOV dst.w, 1.0 */
   if (inst->Dst[0].Register.WriteMask & TGSI_WRITEMASK_W) {
      struct tgsi_full_dst_register dst_w =
         writemask_dst(&inst->Dst[0], TGSI_WRITEMASK_W);
      struct tgsi_full_src_register one =
         make_immediate_reg_float(emit, 1.0f);

      emit_instruction_op1(emit, VGPU10_OPCODE_MOV, &dst_w, &one);
   }

   free_temp_indexes(emit);

   return true;
}


/**
 * Emit code for TGSI_OPCODE_LRP instruction.
 */
static bool
emit_lrp(struct svga_shader_emitter_v10 *emit,
         const struct tgsi_full_instruction *inst)
{
   /* dst = LRP(s0, s1, s2):
    *   dst = s0 * (s1 - s2) + s2
    * Translates into:
    *   SUB tmp, s1, s2;        tmp = s1 - s2
    *   MAD dst, s0, tmp, s2;   dst = s0 * t1 + s2
    */
   unsigned tmp = get_temp_index(emit);
   struct tgsi_full_src_register src_tmp = make_src_temp_reg(tmp);
   struct tgsi_full_dst_register dst_tmp = make_dst_temp_reg(tmp);
   struct tgsi_full_src_register neg_src2 = negate_src(&inst->Src[2]);

   /* ADD tmp, s1, -s2 */
   emit_instruction_opn(emit, VGPU10_OPCODE_ADD, &dst_tmp,
                        &inst->Src[1], &neg_src2, NULL, false,
                        inst->Instruction.Precise);

   /* MAD dst, s1, tmp, s3 */
   emit_instruction_opn(emit, VGPU10_OPCODE_MAD, &inst->Dst[0],
                        &inst->Src[0], &src_tmp, &inst->Src[2],
                        inst->Instruction.Saturate,
                        inst->Instruction.Precise);

   free_temp_indexes(emit);

   return true;
}


/**
 * Emit code for TGSI_OPCODE_POW instruction.
 */
static bool
emit_pow(struct svga_shader_emitter_v10 *emit,
         const struct tgsi_full_instruction *inst)
{
   /* Note that TGSI_OPCODE_POW computes only one value from src0.x and
    * src1.x while VGPU10 computes four values.
    *
    * dst = POW(src0, src1):
    *   dst.xyzw = src0.x ^ src1.x
    */
   unsigned tmp = get_temp_index(emit);
   struct tgsi_full_src_register tmp_src = make_src_temp_reg(tmp);
   struct tgsi_full_dst_register tmp_dst = make_dst_temp_reg(tmp);
   struct tgsi_full_src_register src0_xxxx =
      swizzle_src(&inst->Src[0], TGSI_SWIZZLE_X, TGSI_SWIZZLE_X,
                  TGSI_SWIZZLE_X, TGSI_SWIZZLE_X);
   struct tgsi_full_src_register src1_xxxx =
      swizzle_src(&inst->Src[1], TGSI_SWIZZLE_X, TGSI_SWIZZLE_X,
                  TGSI_SWIZZLE_X, TGSI_SWIZZLE_X);

   /* LOG tmp, s0.xxxx */
   emit_instruction_opn(emit, VGPU10_OPCODE_LOG,
                        &tmp_dst, &src0_xxxx, NULL, NULL,
                        false, inst->Instruction.Precise);

   /* MUL tmp, tmp, s1.xxxx */
   emit_instruction_opn(emit, VGPU10_OPCODE_MUL,
                        &tmp_dst, &tmp_src, &src1_xxxx, NULL,
                        false, inst->Instruction.Precise);

   /* EXP tmp, s0.xxxx */
   emit_instruction_opn(emit, VGPU10_OPCODE_EXP,
                        &inst->Dst[0], &tmp_src, NULL, NULL,
                        inst->Instruction.Saturate,
                        inst->Instruction.Precise);

   /* free tmp */
   free_temp_indexes(emit);

   return true;
}


/**
 * Emit code for TGSI_OPCODE_RCP (reciprocal) instruction.
 */
static bool
emit_rcp(struct svga_shader_emitter_v10 *emit,
         const struct tgsi_full_instruction *inst)
{
   if (emit->version >= 50) {
      /* use new RCP instruction.  But VGPU10_OPCODE_RCP is component-wise
       * while TGSI_OPCODE_RCP computes dst.xyzw = 1.0 / src.xxxx so we need
       * to manipulate the src register's swizzle.
       */
      struct tgsi_full_src_register src = inst->Src[0];
      src.Register.SwizzleY =
      src.Register.SwizzleZ =
      src.Register.SwizzleW = src.Register.SwizzleX;

      begin_emit_instruction(emit);
      emit_opcode_precise(emit, VGPU10_OPCODE_RCP,
                          inst->Instruction.Saturate,
                          inst->Instruction.Precise);
      emit_dst_register(emit, &inst->Dst[0]);
      emit_src_register(emit, &src);
      end_emit_instruction(emit);
   }
   else {
      struct tgsi_full_src_register one = make_immediate_reg_float(emit, 1.0f);

      unsigned tmp = get_temp_index(emit);
      struct tgsi_full_src_register tmp_src = make_src_temp_reg(tmp);
      struct tgsi_full_dst_register tmp_dst = make_dst_temp_reg(tmp);

      struct tgsi_full_dst_register tmp_dst_x =
         writemask_dst(&tmp_dst, TGSI_WRITEMASK_X);
      struct tgsi_full_src_register tmp_src_xxxx =
         scalar_src(&tmp_src, TGSI_SWIZZLE_X);

      /* DIV tmp.x, 1.0, s0 */
      emit_instruction_opn(emit, VGPU10_OPCODE_DIV,
                           &tmp_dst_x, &one, &inst->Src[0], NULL,
                           false, inst->Instruction.Precise);

      /* MOV dst, tmp.xxxx */
      emit_instruction_opn(emit, VGPU10_OPCODE_MOV,
                           &inst->Dst[0], &tmp_src_xxxx, NULL, NULL,
                           inst->Instruction.Saturate,
                           inst->Instruction.Precise);

      free_temp_indexes(emit);
   }

   return true;
}


/**
 * Emit code for TGSI_OPCODE_RSQ instruction.
 */
static bool
emit_rsq(struct svga_shader_emitter_v10 *emit,
         const struct tgsi_full_instruction *inst)
{
   /* dst = RSQ(src):
    *   dst.xyzw = 1 / sqrt(src.x)
    * Translates into:
    *   RSQ tmp, src.x
    *   MOV dst, tmp.xxxx
    */

   unsigned tmp = get_temp_index(emit);
   struct tgsi_full_src_register tmp_src = make_src_temp_reg(tmp);
   struct tgsi_full_dst_register tmp_dst = make_dst_temp_reg(tmp);

   struct tgsi_full_dst_register tmp_dst_x =
      writemask_dst(&tmp_dst, TGSI_WRITEMASK_X);
   struct tgsi_full_src_register tmp_src_xxxx =
      scalar_src(&tmp_src, TGSI_SWIZZLE_X);

   /* RSQ tmp, src.x */
   emit_instruction_opn(emit, VGPU10_OPCODE_RSQ,
                        &tmp_dst_x, &inst->Src[0], NULL, NULL,
                        false, inst->Instruction.Precise);

   /* MOV dst, tmp.xxxx */
   emit_instruction_opn(emit, VGPU10_OPCODE_MOV,
                        &inst->Dst[0], &tmp_src_xxxx, NULL, NULL,
                        inst->Instruction.Saturate,
                        inst->Instruction.Precise);

   /* free tmp */
   free_temp_indexes(emit);

   return true;
}


/**
 * Emit code for TGSI_OPCODE_SEQ (Set Equal) instruction.
 */
static bool
emit_seq(struct svga_shader_emitter_v10 *emit,
         const struct tgsi_full_instruction *inst)
{
   /* dst = SEQ(s0, s1):
    *   dst = s0 == s1 ? 1.0 : 0.0  (per component)
    * Translates into:
    *   EQ tmp, s0, s1;           tmp = s0 == s1 : 0xffffffff : 0 (per comp)
    *   MOVC dst, tmp, 1.0, 0.0;  dst = tmp ? 1.0 : 0.0 (per component)
    */
   unsigned tmp = get_temp_index(emit);
   struct tgsi_full_src_register tmp_src = make_src_temp_reg(tmp);
   struct tgsi_full_dst_register tmp_dst = make_dst_temp_reg(tmp);
   struct tgsi_full_src_register zero = make_immediate_reg_float(emit, 0.0f);
   struct tgsi_full_src_register one = make_immediate_reg_float(emit, 1.0f);

   /* EQ tmp, s0, s1 */
   emit_instruction_op2(emit, VGPU10_OPCODE_EQ, &tmp_dst, &inst->Src[0],
                        &inst->Src[1]);

   /* MOVC dst, tmp, one, zero */
   emit_instruction_op3(emit, VGPU10_OPCODE_MOVC, &inst->Dst[0], &tmp_src,
                        &one, &zero);

   free_temp_indexes(emit);

   return true;
}


/**
 * Emit code for TGSI_OPCODE_SGE (Set Greater than or Equal) instruction.
 */
static bool
emit_sge(struct svga_shader_emitter_v10 *emit,
         const struct tgsi_full_instruction *inst)
{
   /* dst = SGE(s0, s1):
    *   dst = s0 >= s1 ? 1.0 : 0.0  (per component)
    * Translates into:
    *   GE tmp, s0, s1;           tmp = s0 >= s1 : 0xffffffff : 0 (per comp)
    *   MOVC dst, tmp, 1.0, 0.0;  dst = tmp ? 1.0 : 0.0 (per component)
    */
   unsigned tmp = get_temp_index(emit);
   struct tgsi_full_src_register tmp_src = make_src_temp_reg(tmp);
   struct tgsi_full_dst_register tmp_dst = make_dst_temp_reg(tmp);
   struct tgsi_full_src_register zero = make_immediate_reg_float(emit, 0.0f);
   struct tgsi_full_src_register one = make_immediate_reg_float(emit, 1.0f);

   /* GE tmp, s0, s1 */
   emit_instruction_op2(emit, VGPU10_OPCODE_GE, &tmp_dst, &inst->Src[0],
                        &inst->Src[1]);

   /* MOVC dst, tmp, one, zero */
   emit_instruction_op3(emit, VGPU10_OPCODE_MOVC, &inst->Dst[0], &tmp_src,
                        &one, &zero);

   free_temp_indexes(emit);

   return true;
}


/**
 * Emit code for TGSI_OPCODE_SGT (Set Greater than) instruction.
 */
static bool
emit_sgt(struct svga_shader_emitter_v10 *emit,
         const struct tgsi_full_instruction *inst)
{
   /* dst = SGT(s0, s1):
    *   dst = s0 > s1 ? 1.0 : 0.0  (per component)
    * Translates into:
    *   LT tmp, s1, s0;           tmp = s1 < s0 ? 0xffffffff : 0 (per comp)
    *   MOVC dst, tmp, 1.0, 0.0;  dst = tmp ? 1.0 : 0.0 (per component)
    */
   unsigned tmp = get_temp_index(emit);
   struct tgsi_full_src_register tmp_src = make_src_temp_reg(tmp);
   struct tgsi_full_dst_register tmp_dst = make_dst_temp_reg(tmp);
   struct tgsi_full_src_register zero = make_immediate_reg_float(emit, 0.0f);
   struct tgsi_full_src_register one = make_immediate_reg_float(emit, 1.0f);

   /* LT tmp, s1, s0 */
   emit_instruction_op2(emit, VGPU10_OPCODE_LT, &tmp_dst, &inst->Src[1],
                        &inst->Src[0]);

   /* MOVC dst, tmp, one, zero */
   emit_instruction_op3(emit, VGPU10_OPCODE_MOVC, &inst->Dst[0], &tmp_src,
                        &one, &zero);

   free_temp_indexes(emit);

   return true;
}


/**
 * Emit code for TGSI_OPCODE_SIN and TGSI_OPCODE_COS instructions.
 */
static bool
emit_sincos(struct svga_shader_emitter_v10 *emit,
         const struct tgsi_full_instruction *inst)
{
   unsigned tmp = get_temp_index(emit);
   struct tgsi_full_src_register tmp_src = make_src_temp_reg(tmp);
   struct tgsi_full_dst_register tmp_dst = make_dst_temp_reg(tmp);

   struct tgsi_full_src_register tmp_src_xxxx =
      scalar_src(&tmp_src, TGSI_SWIZZLE_X);
   struct tgsi_full_dst_register tmp_dst_x =
      writemask_dst(&tmp_dst, TGSI_WRITEMASK_X);

   begin_emit_instruction(emit);
   emit_opcode(emit, VGPU10_OPCODE_SINCOS, false);

   if(inst->Instruction.Opcode == TGSI_OPCODE_SIN)
   {
      emit_dst_register(emit, &tmp_dst_x);  /* first destination register */
      emit_null_dst_register(emit);  /* second destination register */
   }
   else {
      emit_null_dst_register(emit);
      emit_dst_register(emit, &tmp_dst_x);
   }

   emit_src_register(emit, &inst->Src[0]);
   end_emit_instruction(emit);

   emit_instruction_opn(emit, VGPU10_OPCODE_MOV,
                        &inst->Dst[0], &tmp_src_xxxx, NULL, NULL,
                        inst->Instruction.Saturate,
                        inst->Instruction.Precise);

   free_temp_indexes(emit);

   return true;
}


/**
 * Emit code for TGSI_OPCODE_SLE (Set Less than or Equal) instruction.
 */
static bool
emit_sle(struct svga_shader_emitter_v10 *emit,
         const struct tgsi_full_instruction *inst)
{
   /* dst = SLE(s0, s1):
    *   dst = s0 <= s1 ? 1.0 : 0.0  (per component)
    * Translates into:
    *   GE tmp, s1, s0;           tmp = s1 >= s0 : 0xffffffff : 0 (per comp)
    *   MOVC dst, tmp, 1.0, 0.0;  dst = tmp ? 1.0 : 0.0 (per component)
    */
   unsigned tmp = get_temp_index(emit);
   struct tgsi_full_src_register tmp_src = make_src_temp_reg(tmp);
   struct tgsi_full_dst_register tmp_dst = make_dst_temp_reg(tmp);
   struct tgsi_full_src_register zero = make_immediate_reg_float(emit, 0.0f);
   struct tgsi_full_src_register one = make_immediate_reg_float(emit, 1.0f);

   /* GE tmp, s1, s0 */
   emit_instruction_op2(emit, VGPU10_OPCODE_GE, &tmp_dst, &inst->Src[1],
                        &inst->Src[0]);

   /* MOVC dst, tmp, one, zero */
   emit_instruction_op3(emit, VGPU10_OPCODE_MOVC, &inst->Dst[0], &tmp_src,
                        &one, &zero);

   free_temp_indexes(emit);

   return true;
}


/**
 * Emit code for TGSI_OPCODE_SLT (Set Less than) instruction.
 */
static bool
emit_slt(struct svga_shader_emitter_v10 *emit,
         const struct tgsi_full_instruction *inst)
{
   /* dst = SLT(s0, s1):
    *   dst = s0 < s1 ? 1.0 : 0.0  (per component)
    * Translates into:
    *   LT tmp, s0, s1;           tmp = s0 < s1 ? 0xffffffff : 0 (per comp)
    *   MOVC dst, tmp, 1.0, 0.0;  dst = tmp ? 1.0 : 0.0 (per component)
    */
   unsigned tmp = get_temp_index(emit);
   struct tgsi_full_src_register tmp_src = make_src_temp_reg(tmp);
   struct tgsi_full_dst_register tmp_dst = make_dst_temp_reg(tmp);
   struct tgsi_full_src_register zero = make_immediate_reg_float(emit, 0.0f);
   struct tgsi_full_src_register one = make_immediate_reg_float(emit, 1.0f);

   /* LT tmp, s0, s1 */
   emit_instruction_op2(emit, VGPU10_OPCODE_LT, &tmp_dst, &inst->Src[0],
                        &inst->Src[1]);

   /* MOVC dst, tmp, one, zero */
   emit_instruction_op3(emit, VGPU10_OPCODE_MOVC, &inst->Dst[0], &tmp_src,
                        &one, &zero);

   free_temp_indexes(emit);

   return true;
}


/**
 * Emit code for TGSI_OPCODE_SNE (Set Not Equal) instruction.
 */
static bool
emit_sne(struct svga_shader_emitter_v10 *emit,
         const struct tgsi_full_instruction *inst)
{
   /* dst = SNE(s0, s1):
    *   dst = s0 != s1 ? 1.0 : 0.0  (per component)
    * Translates into:
    *   EQ tmp, s0, s1;           tmp = s0 == s1 : 0xffffffff : 0 (per comp)
    *   MOVC dst, tmp, 1.0, 0.0;  dst = tmp ? 1.0 : 0.0 (per component)
    */
   unsigned tmp = get_temp_index(emit);
   struct tgsi_full_src_register tmp_src = make_src_temp_reg(tmp);
   struct tgsi_full_dst_register tmp_dst = make_dst_temp_reg(tmp);
   struct tgsi_full_src_register zero = make_immediate_reg_float(emit, 0.0f);
   struct tgsi_full_src_register one = make_immediate_reg_float(emit, 1.0f);

   /* NE tmp, s0, s1 */
   emit_instruction_op2(emit, VGPU10_OPCODE_NE, &tmp_dst, &inst->Src[0],
                        &inst->Src[1]);

   /* MOVC dst, tmp, one, zero */
   emit_instruction_op3(emit, VGPU10_OPCODE_MOVC, &inst->Dst[0], &tmp_src,
                        &one, &zero);

   free_temp_indexes(emit);

   return true;
}


/**
 * Emit code for TGSI_OPCODE_SSG (Set Sign) instruction.
 */
static bool
emit_ssg(struct svga_shader_emitter_v10 *emit,
         const struct tgsi_full_instruction *inst)
{
   /* dst.x = (src.x > 0.0) ? 1.0 : (src.x < 0.0) ? -1.0 : 0.0
    * dst.y = (src.y > 0.0) ? 1.0 : (src.y < 0.0) ? -1.0 : 0.0
    * dst.z = (src.z > 0.0) ? 1.0 : (src.z < 0.0) ? -1.0 : 0.0
    * dst.w = (src.w > 0.0) ? 1.0 : (src.w < 0.0) ? -1.0 : 0.0
    * Translates into:
    *   LT tmp1, src, zero;           tmp1 = src < zero ? 0xffffffff : 0 (per comp)
    *   MOVC tmp2, tmp1, -1.0, 0.0;   tmp2 = tmp1 ? -1.0 : 0.0 (per component)
    *   LT tmp1, zero, src;           tmp1 = zero < src ? 0xffffffff : 0 (per comp)
    *   MOVC dst, tmp1, 1.0, tmp2;    dst = tmp1 ? 1.0 : tmp2 (per component)
    */
   struct tgsi_full_src_register zero =
      make_immediate_reg_float(emit, 0.0f);
   struct tgsi_full_src_register one =
      make_immediate_reg_float(emit, 1.0f);
   struct tgsi_full_src_register neg_one =
      make_immediate_reg_float(emit, -1.0f);

   unsigned tmp1 = get_temp_index(emit);
   struct tgsi_full_src_register tmp1_src = make_src_temp_reg(tmp1);
   struct tgsi_full_dst_register tmp1_dst = make_dst_temp_reg(tmp1);

   unsigned tmp2 = get_temp_index(emit);
   struct tgsi_full_src_register tmp2_src = make_src_temp_reg(tmp2);
   struct tgsi_full_dst_register tmp2_dst = make_dst_temp_reg(tmp2);

   emit_instruction_op2(emit, VGPU10_OPCODE_LT, &tmp1_dst, &inst->Src[0],
                        &zero);
   emit_instruction_op3(emit, VGPU10_OPCODE_MOVC, &tmp2_dst, &tmp1_src,
                        &neg_one, &zero);
   emit_instruction_op2(emit, VGPU10_OPCODE_LT, &tmp1_dst, &zero,
                        &inst->Src[0]);
   emit_instruction_op3(emit, VGPU10_OPCODE_MOVC, &inst->Dst[0], &tmp1_src,
                        &one, &tmp2_src);

   free_temp_indexes(emit);

   return true;
}


/**
 * Emit code for TGSI_OPCODE_ISSG (Integer Set Sign) instruction.
 */
static bool
emit_issg(struct svga_shader_emitter_v10 *emit,
          const struct tgsi_full_instruction *inst)
{
   /* dst.x = (src.x > 0) ? 1 : (src.x < 0) ? -1 : 0
    * dst.y = (src.y > 0) ? 1 : (src.y < 0) ? -1 : 0
    * dst.z = (src.z > 0) ? 1 : (src.z < 0) ? -1 : 0
    * dst.w = (src.w > 0) ? 1 : (src.w < 0) ? -1 : 0
    * Translates into:
    *   ILT tmp1, src, 0              tmp1 = src < 0 ? -1 : 0 (per component)
    *   ILT tmp2, 0, src              tmp2 = 0 < src ? -1 : 0 (per component)
    *   IADD dst, tmp1, neg(tmp2)     dst  = tmp1 - tmp2      (per component)
    */
   struct tgsi_full_src_register zero = make_immediate_reg_float(emit, 0.0f);

   unsigned tmp1 = get_temp_index(emit);
   struct tgsi_full_src_register tmp1_src = make_src_temp_reg(tmp1);
   struct tgsi_full_dst_register tmp1_dst = make_dst_temp_reg(tmp1);

   unsigned tmp2 = get_temp_index(emit);
   struct tgsi_full_src_register tmp2_src = make_src_temp_reg(tmp2);
   struct tgsi_full_dst_register tmp2_dst = make_dst_temp_reg(tmp2);

   struct tgsi_full_src_register neg_tmp2 = negate_src(&tmp2_src);

   emit_instruction_op2(emit, VGPU10_OPCODE_ILT, &tmp1_dst,
                        &inst->Src[0], &zero);
   emit_instruction_op2(emit, VGPU10_OPCODE_ILT, &tmp2_dst,
                        &zero, &inst->Src[0]);
   emit_instruction_op2(emit, VGPU10_OPCODE_IADD, &inst->Dst[0],
                        &tmp1_src, &neg_tmp2);

   free_temp_indexes(emit);

   return true;
}


/**
 * Emit a comparison instruction.  The dest register will get
 * 0 or ~0 values depending on the outcome of comparing src0 to src1.
 */
static void
emit_comparison(struct svga_shader_emitter_v10 *emit,
                SVGA3dCmpFunc func,
                const struct tgsi_full_dst_register *dst,
                const struct tgsi_full_src_register *src0,
                const struct tgsi_full_src_register *src1)
{
   struct tgsi_full_src_register immediate;
   VGPU10OpcodeToken0 opcode0;
   bool swapSrc = false;

   /* Sanity checks for svga vs. gallium enums */
   STATIC_ASSERT(SVGA3D_CMP_LESS == (PIPE_FUNC_LESS + 1));
   STATIC_ASSERT(SVGA3D_CMP_GREATEREQUAL == (PIPE_FUNC_GEQUAL + 1));

   opcode0.value = 0;

   switch (func) {
   case SVGA3D_CMP_NEVER:
      immediate = make_immediate_reg_int(emit, 0);
      /* MOV dst, {0} */
      begin_emit_instruction(emit);
      emit_dword(emit, VGPU10_OPCODE_MOV);
      emit_dst_register(emit, dst);
      emit_src_register(emit, &immediate);
      end_emit_instruction(emit);
      return;
   case SVGA3D_CMP_ALWAYS:
      immediate = make_immediate_reg_int(emit, -1);
      /* MOV dst, {-1} */
      begin_emit_instruction(emit);
      emit_dword(emit, VGPU10_OPCODE_MOV);
      emit_dst_register(emit, dst);
      emit_src_register(emit, &immediate);
      end_emit_instruction(emit);
      return;
   case SVGA3D_CMP_LESS:
      opcode0.opcodeType = VGPU10_OPCODE_LT;
      break;
   case SVGA3D_CMP_EQUAL:
      opcode0.opcodeType = VGPU10_OPCODE_EQ;
      break;
   case SVGA3D_CMP_LESSEQUAL:
      opcode0.opcodeType = VGPU10_OPCODE_GE;
      swapSrc = true;
      break;
   case SVGA3D_CMP_GREATER:
      opcode0.opcodeType = VGPU10_OPCODE_LT;
      swapSrc = true;
      break;
   case SVGA3D_CMP_NOTEQUAL:
      opcode0.opcodeType = VGPU10_OPCODE_NE;
      break;
   case SVGA3D_CMP_GREATEREQUAL:
      opcode0.opcodeType = VGPU10_OPCODE_GE;
      break;
   default:
      assert(!"Unexpected comparison mode");
      opcode0.opcodeType = VGPU10_OPCODE_EQ;
   }

   begin_emit_instruction(emit);
   emit_dword(emit, opcode0.value);
   emit_dst_register(emit, dst);
   if (swapSrc) {
      emit_src_register(emit, src1);
      emit_src_register(emit, src0);
   }
   else {
      emit_src_register(emit, src0);
      emit_src_register(emit, src1);
   }
   end_emit_instruction(emit);
}


/**
 * Get texel/address offsets for a texture instruction.
 */
static void
get_texel_offsets(const struct svga_shader_emitter_v10 *emit,
                  const struct tgsi_full_instruction *inst, int offsets[3])
{
   if (inst->Texture.NumOffsets == 1) {
      /* According to OpenGL Shader Language spec the offsets are only
       * fetched from a previously-declared immediate/literal.
       */
      const struct tgsi_texture_offset *off = inst->TexOffsets;
      const unsigned index = off[0].Index;
      const unsigned swizzleX = off[0].SwizzleX;
      const unsigned swizzleY = off[0].SwizzleY;
      const unsigned swizzleZ = off[0].SwizzleZ;
      const union tgsi_immediate_data *imm = emit->immediates[index];

      assert(inst->TexOffsets[0].File == TGSI_FILE_IMMEDIATE);

      offsets[0] = imm[swizzleX].Int;
      offsets[1] = imm[swizzleY].Int;
      offsets[2] = imm[swizzleZ].Int;
   }
   else {
      offsets[0] = offsets[1] = offsets[2] = 0;
   }
}


/**
 * Set up the coordinate register for texture sampling.
 * When we're sampling from a RECT texture we have to scale the
 * unnormalized coordinate to a normalized coordinate.
 * We do that by multiplying the coordinate by an "extra" constant.
 * An alternative would be to use the RESINFO instruction to query the
 * texture's size.
 */
static struct tgsi_full_src_register
setup_texcoord(struct svga_shader_emitter_v10 *emit,
               unsigned unit,
               const struct tgsi_full_src_register *coord)
{
   if (emit->key.tex[unit].sampler_view && emit->key.tex[unit].unnormalized) {
      unsigned scale_index = emit->texcoord_scale_index[unit];
      unsigned tmp = get_temp_index(emit);
      struct tgsi_full_src_register tmp_src = make_src_temp_reg(tmp);
      struct tgsi_full_dst_register tmp_dst = make_dst_temp_reg(tmp);
      struct tgsi_full_src_register scale_src = make_src_const_reg(scale_index);

      if (emit->key.tex[unit].texel_bias) {
         /* to fix texture coordinate rounding issue, 0.0001 offset is
          * been added. This fixes piglit test fbo-blit-scaled-linear. */
         struct tgsi_full_src_register offset =
            make_immediate_reg_float(emit, 0.0001f);

         /* ADD tmp, coord, offset */
         emit_instruction_op2(emit, VGPU10_OPCODE_ADD, &tmp_dst,
                              coord, &offset);
         /* MUL tmp, tmp, scale */
         emit_instruction_op2(emit, VGPU10_OPCODE_MUL, &tmp_dst,
                              &tmp_src, &scale_src);
      }
      else {
         /* MUL tmp, coord, const[] */
         emit_instruction_op2(emit, VGPU10_OPCODE_MUL, &tmp_dst,
                              coord, &scale_src);
      }
      return tmp_src;
   }
   else {
      /* use texcoord as-is */
      return *coord;
   }
}


/**
 * For SAMPLE_C instructions, emit the extra src register which indicates
 * the reference/comparision value.
 */
static void
emit_tex_compare_refcoord(struct svga_shader_emitter_v10 *emit,
                          enum tgsi_texture_type target,
                          const struct tgsi_full_src_register *coord)
{
   struct tgsi_full_src_register coord_src_ref;
   int component;

   assert(tgsi_is_shadow_target(target));

   component = tgsi_util_get_shadow_ref_src_index(target) % 4;
   assert(component >= 0);

   coord_src_ref = scalar_src(coord, component);

   emit_src_register(emit, &coord_src_ref);
}


/**
 * Info for implementing texture swizzles.
 * The begin_tex_swizzle(), get_tex_swizzle_dst() and end_tex_swizzle()
 * functions use this to encapsulate the extra steps needed to perform
 * a texture swizzle, or shadow/depth comparisons.
 * The shadow/depth comparison is only done here if for the cases where
 * there's no VGPU10 opcode (like texture bias lookup w/ shadow compare).
 */
struct tex_swizzle_info
{
   bool swizzled;
   bool shadow_compare;
   unsigned unit;
   enum tgsi_texture_type texture_target;  /**< TGSI_TEXTURE_x */
   struct tgsi_full_src_register tmp_src;
   struct tgsi_full_dst_register tmp_dst;
   const struct tgsi_full_dst_register *inst_dst;
   const struct tgsi_full_src_register *coord_src;
};


/**
 * Do setup for handling texture swizzles or shadow compares.
 * \param unit  the texture unit
 * \param inst  the TGSI texture instruction
 * \param shadow_compare  do shadow/depth comparison?
 * \param swz  returns the swizzle info
 */
static void
begin_tex_swizzle(struct svga_shader_emitter_v10 *emit,
                  unsigned unit,
                  const struct tgsi_full_instruction *inst,
                  bool shadow_compare,
                  struct tex_swizzle_info *swz)
{
   swz->swizzled = (emit->key.tex[unit].swizzle_r != TGSI_SWIZZLE_X ||
                    emit->key.tex[unit].swizzle_g != TGSI_SWIZZLE_Y ||
                    emit->key.tex[unit].swizzle_b != TGSI_SWIZZLE_Z ||
                    emit->key.tex[unit].swizzle_a != TGSI_SWIZZLE_W);

   swz->shadow_compare = shadow_compare;
   swz->texture_target = inst->Texture.Texture;

   if (swz->swizzled || shadow_compare) {
      /* Allocate temp register for the result of the SAMPLE instruction
       * and the source of the MOV/compare/swizzle instructions.
       */
      unsigned tmp = get_temp_index(emit);
      swz->tmp_src = make_src_temp_reg(tmp);
      swz->tmp_dst = make_dst_temp_reg(tmp);

      swz->unit = unit;
   }
   swz->inst_dst = &inst->Dst[0];
   swz->coord_src = &inst->Src[0];

   emit->shadow_compare_units |= shadow_compare << unit;
}


/**
 * Returns the register to put the SAMPLE instruction results into.
 * This will either be the original instruction dst reg (if no swizzle
 * and no shadow comparison) or a temporary reg if there is a swizzle.
 */
static const struct tgsi_full_dst_register *
get_tex_swizzle_dst(const struct tex_swizzle_info *swz)
{
   return (swz->swizzled || swz->shadow_compare)
      ? &swz->tmp_dst : swz->inst_dst;
}


/**
 * This emits the MOV instruction that actually implements a texture swizzle
 * and/or shadow comparison.
 */
static void
end_tex_swizzle(struct svga_shader_emitter_v10 *emit,
                const struct tex_swizzle_info *swz)
{
   if (swz->shadow_compare) {
      /* Emit extra instructions to compare the fetched texel value against
       * a texture coordinate component.  The result of the comparison
       * is 0.0 or 1.0.
       */
      struct tgsi_full_src_register coord_src;
      struct tgsi_full_src_register texel_src =
         scalar_src(&swz->tmp_src, TGSI_SWIZZLE_X);
      struct tgsi_full_src_register one =
         make_immediate_reg_float(emit, 1.0f);
      /* convert gallium comparison func to SVGA comparison func */
      SVGA3dCmpFunc compare_func = emit->key.tex[swz->unit].compare_func + 1;

      int component =
         tgsi_util_get_shadow_ref_src_index(swz->texture_target) % 4;
      assert(component >= 0);
      coord_src = scalar_src(swz->coord_src, component);

      /* COMPARE tmp, coord, texel */
      emit_comparison(emit, compare_func,
                      &swz->tmp_dst, &coord_src, &texel_src);

      /* AND dest, tmp, {1.0} */
      begin_emit_instruction(emit);
      emit_opcode(emit, VGPU10_OPCODE_AND, false);
      if (swz->swizzled) {
         emit_dst_register(emit, &swz->tmp_dst);
      }
      else {
         emit_dst_register(emit, swz->inst_dst);
      }
      emit_src_register(emit, &swz->tmp_src);
      emit_src_register(emit, &one);
      end_emit_instruction(emit);
   }

   if (swz->swizzled) {
      unsigned swz_r = emit->key.tex[swz->unit].swizzle_r;
      unsigned swz_g = emit->key.tex[swz->unit].swizzle_g;
      unsigned swz_b = emit->key.tex[swz->unit].swizzle_b;
      unsigned swz_a = emit->key.tex[swz->unit].swizzle_a;
      unsigned writemask_0 = 0, writemask_1 = 0;
      bool int_tex = is_integer_type(emit->sampler_return_type[swz->unit]);

      /* Swizzle w/out zero/one terms */
      struct tgsi_full_src_register src_swizzled =
         swizzle_src(&swz->tmp_src,
                     swz_r < PIPE_SWIZZLE_0 ? swz_r : PIPE_SWIZZLE_X,
                     swz_g < PIPE_SWIZZLE_0 ? swz_g : PIPE_SWIZZLE_Y,
                     swz_b < PIPE_SWIZZLE_0 ? swz_b : PIPE_SWIZZLE_Z,
                     swz_a < PIPE_SWIZZLE_0 ? swz_a : PIPE_SWIZZLE_W);

      /* MOV dst, color(tmp).<swizzle> */
      emit_instruction_op1(emit, VGPU10_OPCODE_MOV,
                           swz->inst_dst, &src_swizzled);

      /* handle swizzle zero terms */
      writemask_0 = (((swz_r == PIPE_SWIZZLE_0) << 0) |
                     ((swz_g == PIPE_SWIZZLE_0) << 1) |
                     ((swz_b == PIPE_SWIZZLE_0) << 2) |
                     ((swz_a == PIPE_SWIZZLE_0) << 3));
      writemask_0 &= swz->inst_dst->Register.WriteMask;

      if (writemask_0) {
         struct tgsi_full_src_register zero = int_tex ?
            make_immediate_reg_int(emit, 0) :
            make_immediate_reg_float(emit, 0.0f);
         struct tgsi_full_dst_register dst =
            writemask_dst(swz->inst_dst, writemask_0);

         /* MOV dst.writemask_0, {0,0,0,0} */
         emit_instruction_op1(emit, VGPU10_OPCODE_MOV, &dst, &zero);
      }

      /* handle swizzle one terms */
      writemask_1 = (((swz_r == PIPE_SWIZZLE_1) << 0) |
                     ((swz_g == PIPE_SWIZZLE_1) << 1) |
                     ((swz_b == PIPE_SWIZZLE_1) << 2) |
                     ((swz_a == PIPE_SWIZZLE_1) << 3));
      writemask_1 &= swz->inst_dst->Register.WriteMask;

      if (writemask_1) {
         struct tgsi_full_src_register one = int_tex ?
            make_immediate_reg_int(emit, 1) :
            make_immediate_reg_float(emit, 1.0f);
         struct tgsi_full_dst_register dst =
            writemask_dst(swz->inst_dst, writemask_1);

         /* MOV dst.writemask_1, {1,1,1,1} */
         emit_instruction_op1(emit, VGPU10_OPCODE_MOV, &dst, &one);
      }
   }
}


/**
 * Emit code for TGSI_OPCODE_SAMPLE instruction.
 */
static bool
emit_sample(struct svga_shader_emitter_v10 *emit,
            const struct tgsi_full_instruction *inst)
{
   const unsigned resource_unit = inst->Src[1].Register.Index;
   const unsigned sampler_unit = inst->Src[2].Register.Index;
   struct tgsi_full_src_register coord;
   int offsets[3];
   struct tex_swizzle_info swz_info;

   begin_tex_swizzle(emit, sampler_unit, inst, false, &swz_info);

   get_texel_offsets(emit, inst, offsets);

   coord = setup_texcoord(emit, resource_unit, &inst->Src[0]);

   /* SAMPLE dst, coord(s0), resource, sampler */
   begin_emit_instruction(emit);

   /* NOTE: for non-fragment shaders, we should use VGPU10_OPCODE_SAMPLE_L
    * with LOD=0.  But our virtual GPU accepts this as-is.
    */
   emit_sample_opcode(emit, VGPU10_OPCODE_SAMPLE,
                      inst->Instruction.Saturate, offsets);
   emit_dst_register(emit, get_tex_swizzle_dst(&swz_info));
   emit_src_register(emit, &coord);
   emit_resource_register(emit, resource_unit);
   emit_sampler_register(emit, sampler_unit);
   end_emit_instruction(emit);

   end_tex_swizzle(emit, &swz_info);

   free_temp_indexes(emit);

   return true;
}


/**
 * Check if a texture instruction is valid.
 * An example of an invalid texture instruction is doing shadow comparison
 * with an integer-valued texture.
 * If we detect an invalid texture instruction, we replace it with:
 *   MOV dst, {1,1,1,1};
 * \return TRUE if valid, FALSE if invalid.
 */
static bool
is_valid_tex_instruction(struct svga_shader_emitter_v10 *emit,
                         const struct tgsi_full_instruction *inst)
{
   const unsigned unit = inst->Src[1].Register.Index;
   const enum tgsi_texture_type target = inst->Texture.Texture;
   bool valid = true;

   if (tgsi_is_shadow_target(target) &&
       is_integer_type(emit->sampler_return_type[unit])) {
      debug_printf("Invalid SAMPLE_C with an integer texture!\n");
      valid = false;
   }
   /* XXX might check for other conditions in the future here */

   if (!valid) {
      /* emit a MOV dst, {1,1,1,1} instruction. */
      struct tgsi_full_src_register one = make_immediate_reg_float(emit, 1.0f);
      begin_emit_instruction(emit);
      emit_opcode(emit, VGPU10_OPCODE_MOV, false);
      emit_dst_register(emit, &inst->Dst[0]);
      emit_src_register(emit, &one);
      end_emit_instruction(emit);
   }

   return valid;
}


/**
 * Emit code for TGSI_OPCODE_TEX (simple texture lookup)
 */
static bool
emit_tex(struct svga_shader_emitter_v10 *emit,
         const struct tgsi_full_instruction *inst)
{
   const uint unit = inst->Src[1].Register.Index;
   const enum tgsi_texture_type target = inst->Texture.Texture;
   VGPU10_OPCODE_TYPE opcode;
   struct tgsi_full_src_register coord;
   int offsets[3];
   struct tex_swizzle_info swz_info;
   bool compare_in_shader;

   /* check that the sampler returns a float */
   if (!is_valid_tex_instruction(emit, inst))
      return true;

   compare_in_shader = tgsi_is_shadow_target(target) &&
                       emit->key.tex[unit].compare_in_shader;

   begin_tex_swizzle(emit, unit, inst, compare_in_shader, &swz_info);

   get_texel_offsets(emit, inst, offsets);

   coord = setup_texcoord(emit, unit, &inst->Src[0]);

   /* SAMPLE dst, coord(s0), resource, sampler */
   begin_emit_instruction(emit);

   if (tgsi_is_shadow_target(target) && !compare_in_shader)
      opcode = VGPU10_OPCODE_SAMPLE_C;
   else
      opcode = VGPU10_OPCODE_SAMPLE;

   emit_sample_opcode(emit, opcode, inst->Instruction.Saturate, offsets);
   emit_dst_register(emit, get_tex_swizzle_dst(&swz_info));
   emit_src_register(emit, &coord);
   emit_resource_register(emit, unit);
   emit_sampler_register(emit, unit);
   if (opcode == VGPU10_OPCODE_SAMPLE_C) {
      emit_tex_compare_refcoord(emit, target, &coord);
   }
   end_emit_instruction(emit);

   end_tex_swizzle(emit, &swz_info);

   free_temp_indexes(emit);

   return true;
}

/**
 * Emit code for TGSI_OPCODE_TG4 (texture lookup for texture gather)
 */
static bool
emit_tg4(struct svga_shader_emitter_v10 *emit,
         const struct tgsi_full_instruction *inst)
{
   const uint unit = inst->Src[2].Register.Index;
   struct tgsi_full_src_register src;
   struct tgsi_full_src_register offset_src, sampler, ref;
   int offsets[3];

   /* check that the sampler returns a float */
   if (!is_valid_tex_instruction(emit, inst))
      return true;

   if (emit->version >= 50) {
      unsigned target = inst->Texture.Texture;
      int index = inst->Src[1].Register.Index;
      const union tgsi_immediate_data *imm = emit->immediates[index];
      int select_comp  = imm[inst->Src[1].Register.SwizzleX].Int;
      unsigned select_swizzle = PIPE_SWIZZLE_X;

      if (!tgsi_is_shadow_target(target)) {
         switch (select_comp) {
         case 0:
            select_swizzle = emit->key.tex[unit].swizzle_r;
            break;
         case 1:
            select_swizzle = emit->key.tex[unit].swizzle_g;
            break;
         case 2:
            select_swizzle = emit->key.tex[unit].swizzle_b;
            break;
         case 3:
            select_swizzle = emit->key.tex[unit].swizzle_a;
            break;
         default:
            assert(!"Unexpected component in texture gather swizzle");
         }
      }
      else {
         select_swizzle = emit->key.tex[unit].swizzle_r;
      }

      if (select_swizzle == PIPE_SWIZZLE_1) {
         src = make_immediate_reg_float(emit, 1.0);
         emit_instruction_op1(emit, VGPU10_OPCODE_MOV, &inst->Dst[0], &src);
         return true;
      }
      else if (select_swizzle == PIPE_SWIZZLE_0) {
         src = make_immediate_reg_float(emit, 0.0);
         emit_instruction_op1(emit, VGPU10_OPCODE_MOV, &inst->Dst[0], &src);
         return true;
      }

      src = setup_texcoord(emit, unit, &inst->Src[0]);

      /* GATHER4 dst, coord, resource, sampler */
      /* GATHER4_C dst, coord, resource, sampler ref */
      /* GATHER4_PO dst, coord, offset resource, sampler */
      /* GATHER4_PO_C dst, coord, offset resource, sampler, ref */
      begin_emit_instruction(emit);
      if (inst->Texture.NumOffsets == 1) {
         if (tgsi_is_shadow_target(target)) {
            emit_opcode(emit, VGPU10_OPCODE_GATHER4_PO_C,
                        inst->Instruction.Saturate);
         }
         else {
            emit_opcode(emit, VGPU10_OPCODE_GATHER4_PO,
                        inst->Instruction.Saturate);
         }
      }
      else {
         if (tgsi_is_shadow_target(target)) {
            emit_opcode(emit, VGPU10_OPCODE_GATHER4_C,
                        inst->Instruction.Saturate);
         }
         else {
            emit_opcode(emit, VGPU10_OPCODE_GATHER4,
                        inst->Instruction.Saturate);
         }
      }

      emit_dst_register(emit, &inst->Dst[0]);
      emit_src_register(emit, &src);
      if (inst->Texture.NumOffsets == 1) {
         /* offset */
         offset_src = make_src_reg(inst->TexOffsets[0].File,
                                   inst->TexOffsets[0].Index);
         offset_src = swizzle_src(&offset_src, inst->TexOffsets[0].SwizzleX,
                                  inst->TexOffsets[0].SwizzleY,
                                  inst->TexOffsets[0].SwizzleZ,
                                  TGSI_SWIZZLE_W);
         emit_src_register(emit, &offset_src);
      }

      /* resource */
      emit_resource_register(emit, unit);

      /* sampler */
      sampler = make_src_reg(TGSI_FILE_SAMPLER,
                             emit->key.tex[unit].sampler_index);
      sampler.Register.SwizzleX =
      sampler.Register.SwizzleY =
      sampler.Register.SwizzleZ =
      sampler.Register.SwizzleW = select_swizzle;
      emit_src_register(emit, &sampler);

      if (tgsi_is_shadow_target(target)) {
         /* ref */
         if (target == TGSI_TEXTURE_SHADOWCUBE_ARRAY) {
            ref = scalar_src(&inst->Src[1], TGSI_SWIZZLE_X);
            emit_tex_compare_refcoord(emit, target, &ref);
         }
         else {
            emit_tex_compare_refcoord(emit, target, &src);
         }
      }

      end_emit_instruction(emit);
      free_temp_indexes(emit);
   }
   else {
      /* Only a single channel is supported in SM4_1 and we report
       * PIPE_CAP_MAX_TEXTURE_GATHER_COMPONENTS = 1.
       * Only the 0th component will be gathered.
       */
      switch (emit->key.tex[unit].swizzle_r) {
      case PIPE_SWIZZLE_X:
         get_texel_offsets(emit, inst, offsets);
         src = setup_texcoord(emit, unit, &inst->Src[0]);

         /* Gather dst, coord, resource, sampler */
         begin_emit_instruction(emit);
         emit_sample_opcode(emit, VGPU10_OPCODE_GATHER4,
                            inst->Instruction.Saturate, offsets);
         emit_dst_register(emit, &inst->Dst[0]);
         emit_src_register(emit, &src);
         emit_resource_register(emit, unit);

         /* sampler */
         sampler = make_src_reg(TGSI_FILE_SAMPLER,
                                emit->key.tex[unit].sampler_index);
         sampler.Register.SwizzleX =
         sampler.Register.SwizzleY =
         sampler.Register.SwizzleZ =
         sampler.Register.SwizzleW = PIPE_SWIZZLE_X;
         emit_src_register(emit, &sampler);

         end_emit_instruction(emit);
         break;
      case PIPE_SWIZZLE_W:
      case PIPE_SWIZZLE_1:
         src = make_immediate_reg_float(emit, 1.0);
         emit_instruction_op1(emit, VGPU10_OPCODE_MOV, &inst->Dst[0], &src);
         break;
      case PIPE_SWIZZLE_Y:
      case PIPE_SWIZZLE_Z:
      case PIPE_SWIZZLE_0:
      default:
         src = make_immediate_reg_float(emit, 0.0);
         emit_instruction_op1(emit, VGPU10_OPCODE_MOV, &inst->Dst[0], &src);
         break;
      }
   }

   return true;
}



/**
 * Emit code for TGSI_OPCODE_TEX2 (texture lookup for shadow cube map arrays)
 */
static bool
emit_tex2(struct svga_shader_emitter_v10 *emit,
         const struct tgsi_full_instruction *inst)
{
   const uint unit = inst->Src[2].Register.Index;
   unsigned target = inst->Texture.Texture;
   struct tgsi_full_src_register coord, ref;
   int offsets[3];
   struct tex_swizzle_info swz_info;
   VGPU10_OPCODE_TYPE opcode;
   bool compare_in_shader;

   /* check that the sampler returns a float */
   if (!is_valid_tex_instruction(emit, inst))
      return true;

   compare_in_shader = emit->key.tex[unit].compare_in_shader;
   if (compare_in_shader)
      opcode = VGPU10_OPCODE_SAMPLE;
   else
      opcode = VGPU10_OPCODE_SAMPLE_C;

   begin_tex_swizzle(emit, unit, inst, compare_in_shader, &swz_info);

   get_texel_offsets(emit, inst, offsets);

   coord = setup_texcoord(emit, unit, &inst->Src[0]);
   ref = scalar_src(&inst->Src[1], TGSI_SWIZZLE_X);

   /* SAMPLE_C dst, coord, resource, sampler, ref */
   begin_emit_instruction(emit);
   emit_sample_opcode(emit, opcode,
                      inst->Instruction.Saturate, offsets);
   emit_dst_register(emit, get_tex_swizzle_dst(&swz_info));
   emit_src_register(emit, &coord);
   emit_resource_register(emit, unit);
   emit_sampler_register(emit, unit);
   if (opcode == VGPU10_OPCODE_SAMPLE_C) {
      emit_tex_compare_refcoord(emit, target, &ref);
   }
   end_emit_instruction(emit);

   end_tex_swizzle(emit, &swz_info);

   free_temp_indexes(emit);

   return true;
}


/**
 * Emit code for TGSI_OPCODE_TXP (projective texture)
 */
static bool
emit_txp(struct svga_shader_emitter_v10 *emit,
         const struct tgsi_full_instruction *inst)
{
   const uint unit = inst->Src[1].Register.Index;
   const enum tgsi_texture_type target = inst->Texture.Texture;
   VGPU10_OPCODE_TYPE opcode;
   int offsets[3];
   unsigned tmp = get_temp_index(emit);
   struct tgsi_full_src_register tmp_src = make_src_temp_reg(tmp);
   struct tgsi_full_dst_register tmp_dst = make_dst_temp_reg(tmp);
   struct tgsi_full_src_register src0_wwww =
      scalar_src(&inst->Src[0], TGSI_SWIZZLE_W);
   struct tgsi_full_src_register coord;
   struct tex_swizzle_info swz_info;
   bool compare_in_shader;

   /* check that the sampler returns a float */
   if (!is_valid_tex_instruction(emit, inst))
      return true;

   compare_in_shader = tgsi_is_shadow_target(target) &&
                       emit->key.tex[unit].compare_in_shader;

   begin_tex_swizzle(emit, unit, inst, compare_in_shader, &swz_info);

   get_texel_offsets(emit, inst, offsets);

   coord = setup_texcoord(emit, unit, &inst->Src[0]);

   /* DIV tmp, coord, coord.wwww */
   emit_instruction_op2(emit, VGPU10_OPCODE_DIV, &tmp_dst,
                        &coord, &src0_wwww);

   /* SAMPLE dst, coord(tmp), resource, sampler */
   begin_emit_instruction(emit);

   if (tgsi_is_shadow_target(target) && !compare_in_shader)
      /* NOTE: for non-fragment shaders, we should use
       * VGPU10_OPCODE_SAMPLE_C_LZ, but our virtual GPU accepts this as-is.
       */
      opcode = VGPU10_OPCODE_SAMPLE_C;
   else
      opcode = VGPU10_OPCODE_SAMPLE;

   emit_sample_opcode(emit, opcode, inst->Instruction.Saturate, offsets);
   emit_dst_register(emit, get_tex_swizzle_dst(&swz_info));
   emit_src_register(emit, &tmp_src);  /* projected coord */
   emit_resource_register(emit, unit);
   emit_sampler_register(emit, unit);
   if (opcode == VGPU10_OPCODE_SAMPLE_C) {
      emit_tex_compare_refcoord(emit, target, &tmp_src);
   }
   end_emit_instruction(emit);

   end_tex_swizzle(emit, &swz_info);

   free_temp_indexes(emit);

   return true;
}


/**
 * Emit code for TGSI_OPCODE_TXD (explicit derivatives)
 */
static bool
emit_txd(struct svga_shader_emitter_v10 *emit,
         const struct tgsi_full_instruction *inst)
{
   const uint unit = inst->Src[3].Register.Index;
   const enum tgsi_texture_type target = inst->Texture.Texture;
   int offsets[3];
   struct tgsi_full_src_register coord;
   struct tex_swizzle_info swz_info;

   begin_tex_swizzle(emit, unit, inst, tgsi_is_shadow_target(target),
                     &swz_info);

   get_texel_offsets(emit, inst, offsets);

   coord = setup_texcoord(emit, unit, &inst->Src[0]);

   /* SAMPLE_D dst, coord(s0), resource, sampler, Xderiv(s1), Yderiv(s2) */
   begin_emit_instruction(emit);
   emit_sample_opcode(emit, VGPU10_OPCODE_SAMPLE_D,
                      inst->Instruction.Saturate, offsets);
   emit_dst_register(emit, get_tex_swizzle_dst(&swz_info));
   emit_src_register(emit, &coord);
   emit_resource_register(emit, unit);
   emit_sampler_register(emit, unit);
   emit_src_register(emit, &inst->Src[1]);  /* Xderiv */
   emit_src_register(emit, &inst->Src[2]);  /* Yderiv */
   end_emit_instruction(emit);

   end_tex_swizzle(emit, &swz_info);

   free_temp_indexes(emit);

   return true;
}


/**
 * Emit code for TGSI_OPCODE_TXF (texel fetch)
 */
static bool
emit_txf(struct svga_shader_emitter_v10 *emit,
         const struct tgsi_full_instruction *inst)
{
   const uint unit = inst->Src[1].Register.Index;
   const bool msaa = tgsi_is_msaa_target(inst->Texture.Texture)
      && emit->key.tex[unit].num_samples > 1;
   int offsets[3];
   struct tex_swizzle_info swz_info;

   begin_tex_swizzle(emit, unit, inst, false, &swz_info);

   get_texel_offsets(emit, inst, offsets);

   if (msaa) {
      assert(emit->key.tex[unit].num_samples > 1);

      /* Fetch one sample from an MSAA texture */
      struct tgsi_full_src_register sampleIndex =
         scalar_src(&inst->Src[0], TGSI_SWIZZLE_W);
      /* LD_MS dst, coord(s0), resource, sampleIndex */
      begin_emit_instruction(emit);
      emit_sample_opcode(emit, VGPU10_OPCODE_LD_MS,
                         inst->Instruction.Saturate, offsets);
      emit_dst_register(emit, get_tex_swizzle_dst(&swz_info));
      emit_src_register(emit, &inst->Src[0]);
      emit_resource_register(emit, unit);
      emit_src_register(emit, &sampleIndex);
      end_emit_instruction(emit);
   }
   else {
      /* Fetch one texel specified by integer coordinate */
      /* LD dst, coord(s0), resource */
      begin_emit_instruction(emit);
      emit_sample_opcode(emit, VGPU10_OPCODE_LD,
                         inst->Instruction.Saturate, offsets);
      emit_dst_register(emit, get_tex_swizzle_dst(&swz_info));
      emit_src_register(emit, &inst->Src[0]);
      emit_resource_register(emit, unit);
      end_emit_instruction(emit);
   }

   end_tex_swizzle(emit, &swz_info);

   free_temp_indexes(emit);

   return true;
}


/**
 * Emit code for TGSI_OPCODE_TXL (explicit LOD) or TGSI_OPCODE_TXB (LOD bias)
 * or TGSI_OPCODE_TXB2 (for cube shadow maps).
 */
static bool
emit_txl_txb(struct svga_shader_emitter_v10 *emit,
             const struct tgsi_full_instruction *inst)
{
   const enum tgsi_texture_type target = inst->Texture.Texture;
   VGPU10_OPCODE_TYPE opcode;
   unsigned unit;
   int offsets[3];
   struct tgsi_full_src_register coord, lod_bias;
   struct tex_swizzle_info swz_info;

   assert(inst->Instruction.Opcode == TGSI_OPCODE_TXL ||
          inst->Instruction.Opcode == TGSI_OPCODE_TXB ||
          inst->Instruction.Opcode == TGSI_OPCODE_TXB2);

   if (inst->Instruction.Opcode == TGSI_OPCODE_TXB2) {
      lod_bias = scalar_src(&inst->Src[1], TGSI_SWIZZLE_X);
      unit = inst->Src[2].Register.Index;
   }
   else {
      lod_bias = scalar_src(&inst->Src[0], TGSI_SWIZZLE_W);
      unit = inst->Src[1].Register.Index;
   }

   begin_tex_swizzle(emit, unit, inst, tgsi_is_shadow_target(target),
                     &swz_info);

   get_texel_offsets(emit, inst, offsets);

   coord = setup_texcoord(emit, unit, &inst->Src[0]);

   /* SAMPLE_L/B dst, coord(s0), resource, sampler, lod(s3) */
   begin_emit_instruction(emit);
   if (inst->Instruction.Opcode == TGSI_OPCODE_TXL) {
      opcode = VGPU10_OPCODE_SAMPLE_L;
   }
   else {
      opcode = VGPU10_OPCODE_SAMPLE_B;
   }
   emit_sample_opcode(emit, opcode, inst->Instruction.Saturate, offsets);
   emit_dst_register(emit, get_tex_swizzle_dst(&swz_info));
   emit_src_register(emit, &coord);
   emit_resource_register(emit, unit);
   emit_sampler_register(emit, unit);
   emit_src_register(emit, &lod_bias);
   end_emit_instruction(emit);

   end_tex_swizzle(emit, &swz_info);

   free_temp_indexes(emit);

   return true;
}


/**
 * Emit code for TGSI_OPCODE_TXL2 (explicit LOD) for cubemap array.
 */
static bool
emit_txl2(struct svga_shader_emitter_v10 *emit,
          const struct tgsi_full_instruction *inst)
{
   unsigned target = inst->Texture.Texture;
   unsigned opcode, unit;
   int offsets[3];
   struct tgsi_full_src_register coord, lod;
   struct tex_swizzle_info swz_info;

   assert(inst->Instruction.Opcode == TGSI_OPCODE_TXL2);

   lod = scalar_src(&inst->Src[1], TGSI_SWIZZLE_X);
   unit = inst->Src[2].Register.Index;

   begin_tex_swizzle(emit, unit, inst, tgsi_is_shadow_target(target),
                     &swz_info);

   get_texel_offsets(emit, inst, offsets);

   coord = setup_texcoord(emit, unit, &inst->Src[0]);

   /* SAMPLE_L dst, coord(s0), resource, sampler, lod(s3) */
   begin_emit_instruction(emit);
   opcode = VGPU10_OPCODE_SAMPLE_L;
   emit_sample_opcode(emit, opcode, inst->Instruction.Saturate, offsets);
   emit_dst_register(emit, get_tex_swizzle_dst(&swz_info));
   emit_src_register(emit, &coord);
   emit_resource_register(emit, unit);
   emit_sampler_register(emit, unit);
   emit_src_register(emit, &lod);
   end_emit_instruction(emit);

   end_tex_swizzle(emit, &swz_info);

   free_temp_indexes(emit);

   return true;
}


/**
 * Emit code for TGSI_OPCODE_TXQ (texture query) instruction.
 */
static bool
emit_txq(struct svga_shader_emitter_v10 *emit,
         const struct tgsi_full_instruction *inst)
{
   const uint unit = inst->Src[1].Register.Index;

   if (emit->key.tex[unit].target == PIPE_BUFFER) {
      /* RESINFO does not support querying texture buffers, so we instead
       * store texture buffer sizes in shader constants, then copy them to
       * implement TXQ instead of emitting RESINFO.
       * MOV dst, const[texture_buffer_size_index[unit]]
       */
      struct tgsi_full_src_register size_src =
         make_src_const_reg(emit->texture_buffer_size_index[unit]);
      emit_instruction_op1(emit, VGPU10_OPCODE_MOV, &inst->Dst[0], &size_src);
   } else {
      /* RESINFO dst, srcMipLevel, resource */
      begin_emit_instruction(emit);
      emit_opcode_resinfo(emit, VGPU10_RESINFO_RETURN_UINT);
      emit_dst_register(emit, &inst->Dst[0]);
      emit_src_register(emit, &inst->Src[0]);
      emit_resource_register(emit, unit);
      end_emit_instruction(emit);
   }

   free_temp_indexes(emit);

   return true;
}


/**
 * Does this opcode produce a double-precision result?
 * XXX perhaps move this to a TGSI utility.
 */
static bool
opcode_has_dbl_dst(unsigned opcode)
{
   switch (opcode) {
   case TGSI_OPCODE_F2D:
   case TGSI_OPCODE_DABS:
   case TGSI_OPCODE_DADD:
   case TGSI_OPCODE_DFRAC:
   case TGSI_OPCODE_DMAX:
   case TGSI_OPCODE_DMIN:
   case TGSI_OPCODE_DMUL:
   case TGSI_OPCODE_DNEG:
   case TGSI_OPCODE_I2D:
   case TGSI_OPCODE_U2D:
   case TGSI_OPCODE_DFMA:
      // XXX more TBD
      return true;
   default:
      return false;
   }
}


/**
 * Does this opcode use double-precision source registers?
 */
static bool
opcode_has_dbl_src(unsigned opcode)
{
   switch (opcode) {
   case TGSI_OPCODE_D2F:
   case TGSI_OPCODE_DABS:
   case TGSI_OPCODE_DADD:
   case TGSI_OPCODE_DFRAC:
   case TGSI_OPCODE_DMAX:
   case TGSI_OPCODE_DMIN:
   case TGSI_OPCODE_DMUL:
   case TGSI_OPCODE_DNEG:
   case TGSI_OPCODE_D2I:
   case TGSI_OPCODE_D2U:
   case TGSI_OPCODE_DFMA:
   case TGSI_OPCODE_DSLT:
   case TGSI_OPCODE_DSGE:
   case TGSI_OPCODE_DSEQ:
   case TGSI_OPCODE_DSNE:
   case TGSI_OPCODE_DRCP:
   case TGSI_OPCODE_DSQRT:
   case TGSI_OPCODE_DMAD:
   case TGSI_OPCODE_DLDEXP:
   case TGSI_OPCODE_DRSQ:
   case TGSI_OPCODE_DTRUNC:
   case TGSI_OPCODE_DCEIL:
   case TGSI_OPCODE_DFLR:
   case TGSI_OPCODE_DROUND:
   case TGSI_OPCODE_DSSG:
      return true;
   default:
      return false;
   }
}


/**
 * Check that the swizzle for reading from a double-precision register
 * is valid. If not valid, move the source to a temporary register first.
 */
static struct tgsi_full_src_register
check_double_src(struct svga_shader_emitter_v10 *emit,
                 const struct tgsi_full_src_register *reg)
{
   struct tgsi_full_src_register src;

   if (((reg->Register.SwizzleX == PIPE_SWIZZLE_X &&
         reg->Register.SwizzleY == PIPE_SWIZZLE_Y) ||
        (reg->Register.SwizzleX == PIPE_SWIZZLE_Z &&
         reg->Register.SwizzleY == PIPE_SWIZZLE_W)) &&
       ((reg->Register.SwizzleZ == PIPE_SWIZZLE_X &&
         reg->Register.SwizzleW == PIPE_SWIZZLE_Y) ||
        (reg->Register.SwizzleZ == PIPE_SWIZZLE_Z &&
         reg->Register.SwizzleW == PIPE_SWIZZLE_W))) {
      src = *reg;
   } else {
      /* move the src to a temporary to fix the swizzle */
      unsigned tmp = get_temp_index(emit);
      struct tgsi_full_src_register tmp_src = make_src_temp_reg(tmp);
      struct tgsi_full_dst_register tmp_dst = make_dst_temp_reg(tmp);
      emit_instruction_op1(emit, VGPU10_OPCODE_MOV, &tmp_dst, reg);
      src = tmp_src;

      /* The temporary index will be released in the caller */
   }
   return src;
}

/**
 * Check that the writemask for a double-precision instruction is valid.
 */
static void
check_double_dst_writemask(const struct tgsi_full_instruction *inst)
{
   ASSERTED unsigned writemask = inst->Dst[0].Register.WriteMask;

   switch (inst->Instruction.Opcode) {
   case TGSI_OPCODE_DABS:
   case TGSI_OPCODE_DADD:
   case TGSI_OPCODE_DFRAC:
   case TGSI_OPCODE_DNEG:
   case TGSI_OPCODE_DMAD:
   case TGSI_OPCODE_DMAX:
   case TGSI_OPCODE_DMIN:
   case TGSI_OPCODE_DMUL:
   case TGSI_OPCODE_DRCP:
   case TGSI_OPCODE_DSQRT:
   case TGSI_OPCODE_F2D:
   case TGSI_OPCODE_DFMA:
      assert(writemask == TGSI_WRITEMASK_XYZW ||
             writemask == TGSI_WRITEMASK_XY ||
             writemask == TGSI_WRITEMASK_ZW);
      break;
   case TGSI_OPCODE_DSEQ:
   case TGSI_OPCODE_DSGE:
   case TGSI_OPCODE_DSNE:
   case TGSI_OPCODE_DSLT:
   case TGSI_OPCODE_D2I:
   case TGSI_OPCODE_D2U:
      /* Write to 1 or 2 components only */
      assert(util_bitcount(writemask) <= 2);
      break;
   default:
      /* XXX this list may be incomplete */
      ;
   }
}


/**
 * Double-precision absolute value.
 */
static bool
emit_dabs(struct svga_shader_emitter_v10 *emit,
          const struct tgsi_full_instruction *inst)
{
   assert(emit->version >= 50);

   struct tgsi_full_src_register src = check_double_src(emit, &inst->Src[0]);
   check_double_dst_writemask(inst);

   struct tgsi_full_src_register abs_src = absolute_src(&src);

   /* DMOV dst, |src| */
   emit_instruction_op1(emit, VGPU10_OPCODE_DMOV, &inst->Dst[0], &abs_src);

   free_temp_indexes(emit);
   return true;
}


/**
 * Double-precision negation
 */
static bool
emit_dneg(struct svga_shader_emitter_v10 *emit,
          const struct tgsi_full_instruction *inst)
{
   assert(emit->version >= 50);
   struct tgsi_full_src_register src = check_double_src(emit, &inst->Src[0]);
   check_double_dst_writemask(inst);

   struct tgsi_full_src_register neg_src = negate_src(&src);

   /* DMOV dst, -src */
   emit_instruction_op1(emit, VGPU10_OPCODE_DMOV, &inst->Dst[0], &neg_src);

   free_temp_indexes(emit);
   return true;
}


/**
 * SM5 has no DMAD opcode.  Implement negation with DMUL/DADD.
 */
static bool
emit_dmad(struct svga_shader_emitter_v10 *emit,
          const struct tgsi_full_instruction *inst)
{
   assert(emit->version >= 50);
   struct tgsi_full_src_register src0 = check_double_src(emit, &inst->Src[0]);
   struct tgsi_full_src_register src1 = check_double_src(emit, &inst->Src[1]);
   struct tgsi_full_src_register src2 = check_double_src(emit, &inst->Src[2]);
   check_double_dst_writemask(inst);

   unsigned tmp = get_temp_index(emit);
   struct tgsi_full_src_register tmp_src = make_src_temp_reg(tmp);
   struct tgsi_full_dst_register tmp_dst = make_dst_temp_reg(tmp);

   /* DMUL tmp, src[0], src[1] */
   emit_instruction_opn(emit, VGPU10_OPCODE_DMUL,
                        &tmp_dst, &src0, &src1, NULL,
                        false, inst->Instruction.Precise);

   /* DADD dst, tmp, src[2] */
   emit_instruction_opn(emit, VGPU10_OPCODE_DADD,
                        &inst->Dst[0], &tmp_src, &src2, NULL,
                        inst->Instruction.Saturate, inst->Instruction.Precise);
   free_temp_indexes(emit);

   return true;
}


/**
 * Double precision reciprocal square root
 */
static bool
emit_drsq(struct svga_shader_emitter_v10 *emit,
          const struct tgsi_full_dst_register *dst,
          const struct tgsi_full_src_register *src)
{
   assert(emit->version >= 50);

   VGPU10OpcodeToken0 token0;
   struct tgsi_full_src_register dsrc = check_double_src(emit, src);

   begin_emit_instruction(emit);

   token0.value = 0;
   token0.opcodeType = VGPU10_OPCODE_VMWARE;
   token0.vmwareOpcodeType = VGPU10_VMWARE_OPCODE_DRSQ;
   emit_dword(emit, token0.value);
   emit_dst_register(emit, dst);
   emit_src_register(emit, &dsrc);
   end_emit_instruction(emit);

   free_temp_indexes(emit);

   return true;
}


/**
 * There is no SM5 opcode for double precision square root.
 * It will be implemented with DRSQ.
 * dst = src * DRSQ(src)
 */
static bool
emit_dsqrt(struct svga_shader_emitter_v10 *emit,
          const struct tgsi_full_instruction *inst)
{
   assert(emit->version >= 50);

   struct tgsi_full_src_register src = check_double_src(emit, &inst->Src[0]);

   /* temporary register to hold the source */
   unsigned tmp = get_temp_index(emit);
   struct tgsi_full_dst_register tmp_dst = make_dst_temp_reg(tmp);
   struct tgsi_full_src_register tmp_src = make_src_temp_reg(tmp);

   /* temporary register to hold the DEQ result */
   unsigned tmp_cond = get_temp_index(emit);
   struct tgsi_full_dst_register tmp_cond_dst = make_dst_temp_reg(tmp_cond);
   struct tgsi_full_dst_register tmp_cond_dst_xy =
      writemask_dst(&tmp_cond_dst, TGSI_WRITEMASK_X | TGSI_WRITEMASK_Y);
   struct tgsi_full_src_register tmp_cond_src = make_src_temp_reg(tmp_cond);
   struct tgsi_full_src_register tmp_cond_src_xy =
         swizzle_src(&tmp_cond_src,
                     PIPE_SWIZZLE_X, PIPE_SWIZZLE_Y,
                     PIPE_SWIZZLE_X, PIPE_SWIZZLE_Y);

   /* The reciprocal square root of zero yields INF.
    * So if the source is 0, we replace it with 1 in the tmp register.
    * The later multiplication of zero in the original source will yield 0
    * in the result.
    */

   /* tmp1 = (src == 0) ? 1 : src;
    *   EQ tmp1, 0, src
    *   MOVC tmp, tmp1, 1.0, src
    */
   struct tgsi_full_src_register zero =
               make_immediate_reg_double(emit, 0);

   struct tgsi_full_src_register one =
               make_immediate_reg_double(emit, 1.0);

   emit_instruction_op2(emit, VGPU10_OPCODE_DEQ, &tmp_cond_dst_xy,
                        &zero, &src);
   emit_instruction_op3(emit, VGPU10_OPCODE_DMOVC, &tmp_dst,
                        &tmp_cond_src_xy, &one, &src);

   struct tgsi_full_dst_register tmp_rsq_dst = make_dst_temp_reg(tmp);
   struct tgsi_full_src_register tmp_rsq_src = make_src_temp_reg(tmp);

   /* DRSQ tmp_rsq, tmp */
   emit_drsq(emit, &tmp_rsq_dst, &tmp_src);

   /* DMUL dst, tmp_rsq, src[0] */
   emit_instruction_op2(emit, VGPU10_OPCODE_DMUL, &inst->Dst[0],
                        &tmp_rsq_src, &src);

   free_temp_indexes(emit);

   return true;
}


/**
 * glsl-nir path does not lower DTRUNC, so we need to
 * add the translation here.
 *
 * frac = DFRAC(src)
 * tmp = src - frac
 * dst = src >= 0 ? tmp : (tmp + (frac==0 ? 0 : 1))
 */
static bool
emit_dtrunc(struct svga_shader_emitter_v10 *emit,
            const struct tgsi_full_instruction *inst)
{
   assert(emit->version >= 50);

   struct tgsi_full_src_register src = check_double_src(emit, &inst->Src[0]);

   /* frac = DFRAC(src) */
   unsigned frac_index = get_temp_index(emit);
   struct tgsi_full_dst_register frac_dst = make_dst_temp_reg(frac_index);
   struct tgsi_full_src_register frac_src = make_src_temp_reg(frac_index);

   VGPU10OpcodeToken0 token0;
   begin_emit_instruction(emit);
   token0.value = 0;
   token0.opcodeType = VGPU10_OPCODE_VMWARE;
   token0.vmwareOpcodeType = VGPU10_VMWARE_OPCODE_DFRC;
   emit_dword(emit, token0.value);
   emit_dst_register(emit, &frac_dst);
   emit_src_register(emit, &src);
   end_emit_instruction(emit);

   /* tmp = src - frac */
   unsigned tmp_index = get_temp_index(emit);
   struct tgsi_full_dst_register tmp_dst = make_dst_temp_reg(tmp_index);
   struct tgsi_full_src_register tmp_src = make_src_temp_reg(tmp_index);
   struct tgsi_full_src_register negate_frac_src = negate_src(&frac_src);
   emit_instruction_opn(emit, VGPU10_OPCODE_DADD,
                        &tmp_dst, &src, &negate_frac_src, NULL,
                        inst->Instruction.Saturate, inst->Instruction.Precise);

   /* cond = frac==0 */
   unsigned cond_index = get_temp_index(emit);
   struct tgsi_full_dst_register cond_dst = make_dst_temp_reg(cond_index);
   struct tgsi_full_src_register cond_src = make_src_temp_reg(cond_index);
   struct tgsi_full_src_register zero =
               make_immediate_reg_double(emit, 0);

   /* Only use one or two components for double opcode */
   cond_dst = writemask_dst(&cond_dst, TGSI_WRITEMASK_X | TGSI_WRITEMASK_Y);

   emit_instruction_opn(emit, VGPU10_OPCODE_DEQ,
                        &cond_dst, &frac_src, &zero, NULL,
                        inst->Instruction.Saturate, inst->Instruction.Precise);

   /* tmp2 = cond ? 0 : 1 */
   unsigned tmp2_index = get_temp_index(emit);
   struct tgsi_full_dst_register tmp2_dst = make_dst_temp_reg(tmp2_index);
   struct tgsi_full_src_register tmp2_src = make_src_temp_reg(tmp2_index);
   struct tgsi_full_src_register cond_src_xy =
      swizzle_src(&cond_src, PIPE_SWIZZLE_X, PIPE_SWIZZLE_Y,
		             PIPE_SWIZZLE_X, PIPE_SWIZZLE_Y);
   struct tgsi_full_src_register one =
               make_immediate_reg_double(emit, 1.0);

   emit_instruction_opn(emit, VGPU10_OPCODE_DMOVC,
                        &tmp2_dst, &cond_src_xy, &zero, &one,
                        inst->Instruction.Saturate, inst->Instruction.Precise);

   /* tmp2 = tmp + tmp2 */
   emit_instruction_opn(emit, VGPU10_OPCODE_DADD,
                        &tmp2_dst, &tmp_src, &tmp2_src, NULL,
                        inst->Instruction.Saturate, inst->Instruction.Precise);

   /* cond = src>=0 */
   emit_instruction_opn(emit, VGPU10_OPCODE_DGE,
                        &cond_dst, &src, &zero, NULL,
                        inst->Instruction.Saturate, inst->Instruction.Precise);

   /* dst = cond ? tmp : tmp2 */
   emit_instruction_opn(emit, VGPU10_OPCODE_DMOVC,
                        &inst->Dst[0], &cond_src_xy, &tmp_src, &tmp2_src,
                        inst->Instruction.Saturate, inst->Instruction.Precise);

   free_temp_indexes(emit);
   return true;
}


static bool
emit_interp_offset(struct svga_shader_emitter_v10 *emit,
                   const struct tgsi_full_instruction *inst)
{
   assert(emit->version >= 50);

   /* The src1.xy offset is a float with values in the range [-0.5, 0.5]
    * where (0,0) is the center of the pixel.  We need to translate that
    * into an integer offset on a 16x16 grid in the range [-8/16, 7/16].
    * Also need to flip the Y axis (I think).
    */
   unsigned tmp = get_temp_index(emit);
   struct tgsi_full_src_register tmp_src = make_src_temp_reg(tmp);
   struct tgsi_full_dst_register tmp_dst = make_dst_temp_reg(tmp);
   struct tgsi_full_dst_register tmp_dst_xy =
      writemask_dst(&tmp_dst, TGSI_WRITEMASK_X | TGSI_WRITEMASK_Y);
   struct tgsi_full_src_register const16 =
      make_immediate_reg_float4(emit, 16.0f, -16.0, 0, 0);

   /* MUL tmp.xy, src1, {16, -16, 0, 0} */
   emit_instruction_op2(emit, VGPU10_OPCODE_MUL,
                        &tmp_dst_xy, &inst->Src[1], &const16);

   /* FTOI tmp.xy, tmp */
   emit_instruction_op1(emit, VGPU10_OPCODE_FTOI, &tmp_dst_xy, &tmp_src);

   /* EVAL_SNAPPED dst, src0, tmp */
   emit_instruction_op2(emit, VGPU10_OPCODE_EVAL_SNAPPED,
                        &inst->Dst[0], &inst->Src[0], &tmp_src);

   free_temp_indexes(emit);

   return true;
}


/**
 * Emit a simple instruction (like ADD, MUL, MIN, etc).
 */
static bool
emit_simple(struct svga_shader_emitter_v10 *emit,
            const struct tgsi_full_instruction *inst)
{
   const enum tgsi_opcode opcode = inst->Instruction.Opcode;
   const struct tgsi_opcode_info *op = tgsi_get_opcode_info(opcode);
   const bool dbl_dst = opcode_has_dbl_dst(inst->Instruction.Opcode);
   const bool dbl_src = opcode_has_dbl_src(inst->Instruction.Opcode);
   unsigned i;

   struct tgsi_full_src_register src[3];

   if (inst->Instruction.Opcode == TGSI_OPCODE_BGNLOOP) {
      emit->current_loop_depth++;
   }
   else if (inst->Instruction.Opcode == TGSI_OPCODE_ENDLOOP) {
      emit->current_loop_depth--;
   }

   for (i = 0; i < op->num_src; i++) {
      if (dbl_src)
         src[i] = check_double_src(emit, &inst->Src[i]);
      else
         src[i] = inst->Src[i];
   }

   begin_emit_instruction(emit);
   emit_opcode_precise(emit, translate_opcode(inst->Instruction.Opcode),
                       inst->Instruction.Saturate,
                       inst->Instruction.Precise);
   for (i = 0; i < op->num_dst; i++) {
      if (dbl_dst) {
         check_double_dst_writemask(inst);
      }
      emit_dst_register(emit, &inst->Dst[i]);
   }
   for (i = 0; i < op->num_src; i++) {
      emit_src_register(emit, &src[i]);
   }
   end_emit_instruction(emit);

   free_temp_indexes(emit);
   return true;
}


/**
 * Emit MSB instruction (like IMSB, UMSB).
 *
 * GLSL returns the index starting from the LSB;
 * whereas in SM5, firstbit_hi/shi returns the index starting from the MSB.
 * To get correct location as per glsl from SM5 device, we should
 * return (31 - index) if returned index is not -1.
 */
static bool
emit_msb(struct svga_shader_emitter_v10 *emit,
         const struct tgsi_full_instruction *inst)
{
   const struct tgsi_full_dst_register *index_dst = &inst->Dst[0];

   assert(index_dst->Register.File != TGSI_FILE_OUTPUT);

   struct tgsi_full_src_register index_src =
      make_src_reg(index_dst->Register.File, index_dst->Register.Index);
   struct tgsi_full_src_register imm31 =
      make_immediate_reg_int(emit, 31);
   imm31 = scalar_src(&imm31, TGSI_SWIZZLE_X);
   struct tgsi_full_src_register neg_one =
      make_immediate_reg_int(emit, -1);
   neg_one = scalar_src(&neg_one, TGSI_SWIZZLE_X);
   unsigned tmp = get_temp_index(emit);
   const struct tgsi_full_dst_register tmp_dst =
      make_dst_temp_reg(tmp);
   const struct tgsi_full_dst_register tmp_dst_x =
      writemask_dst(&tmp_dst, TGSI_WRITEMASK_X);
   const struct tgsi_full_src_register tmp_src_x =
       make_src_scalar_reg(TGSI_FILE_TEMPORARY, tmp, TGSI_SWIZZLE_X);
   int writemask = TGSI_WRITEMASK_X;
   int src_swizzle = TGSI_SWIZZLE_X;
   int dst_writemask = index_dst->Register.WriteMask;

   emit_simple(emit, inst);

   /* index conversion from SM5 to GLSL */
   while (writemask & dst_writemask) {
      struct tgsi_full_src_register index_src_comp =
         scalar_src(&index_src, src_swizzle);
      struct tgsi_full_dst_register index_dst_comp =
         writemask_dst(index_dst, writemask);

      /* check if index_src_comp != -1 */
      emit_instruction_op2(emit, VGPU10_OPCODE_INE,
                           &tmp_dst_x, &index_src_comp, &neg_one);

      /* if */
      emit_if(emit, &tmp_src_x);

      index_src_comp = negate_src(&index_src_comp);
      /* SUB DST, IMM{31}, DST */
      emit_instruction_op2(emit, VGPU10_OPCODE_IADD,
                           &index_dst_comp, &imm31, &index_src_comp);

      /* endif */
      emit_instruction_op0(emit, VGPU10_OPCODE_ENDIF);

      writemask = writemask << 1;
      src_swizzle = src_swizzle + 1;
   }
   free_temp_indexes(emit);
   return true;
}


/**
 * Emit a BFE instruction (like UBFE, IBFE).
 * tgsi representation:
 * U/IBFE dst, value, offset, width
 * SM5 representation:
 * U/IBFE dst, width, offset, value
 * Note: SM5 has width & offset range (0-31);
 *      whereas GLSL has width & offset range (0-32)
 */
static bool
emit_bfe(struct svga_shader_emitter_v10 *emit,
         const struct tgsi_full_instruction *inst)
{
   const enum tgsi_opcode opcode = inst->Instruction.Opcode;
   struct tgsi_full_src_register imm32 = make_immediate_reg_int(emit, 32);
   imm32 = scalar_src(&imm32, TGSI_SWIZZLE_X);
   struct tgsi_full_src_register zero = make_immediate_reg_int(emit, 0);
   zero = scalar_src(&zero, TGSI_SWIZZLE_X);

   unsigned tmp1 = get_temp_index(emit);
   const struct tgsi_full_dst_register cond1_dst = make_dst_temp_reg(tmp1);
   const struct tgsi_full_dst_register cond1_dst_x =
      writemask_dst(&cond1_dst, TGSI_WRITEMASK_X);
   const struct tgsi_full_src_register cond1_src_x =
      make_src_scalar_reg(TGSI_FILE_TEMPORARY, tmp1, TGSI_SWIZZLE_X);

   unsigned tmp2 = get_temp_index(emit);
   const struct tgsi_full_dst_register cond2_dst = make_dst_temp_reg(tmp2);
   const struct tgsi_full_dst_register cond2_dst_x =
      writemask_dst(&cond2_dst, TGSI_WRITEMASK_X);
   const struct tgsi_full_src_register cond2_src_x =
      make_src_scalar_reg(TGSI_FILE_TEMPORARY, tmp2, TGSI_SWIZZLE_X);

   /**
    * In SM5, when width = 32  and offset = 0, it returns 0.
    * On the other hand GLSL, expects value to be copied as it is, to dst.
    */

   /* cond1 = width ! = 32 */
   emit_instruction_op2(emit, VGPU10_OPCODE_IEQ,
                        &cond1_dst_x, &inst->Src[2], &imm32);

   /* cond2 = offset ! = 0 */
   emit_instruction_op2(emit, VGPU10_OPCODE_IEQ,
                        &cond2_dst_x, &inst->Src[1], &zero);

   /* cond 2 = cond1 & cond 2 */
   emit_instruction_op2(emit, VGPU10_OPCODE_AND, &cond2_dst_x,
                        &cond2_src_x,
                        &cond1_src_x);
   /* IF */
   emit_if(emit, &cond2_src_x);

   emit_instruction_op1(emit, VGPU10_OPCODE_MOV, &inst->Dst[0],
                        &inst->Src[0]);

   /* ELSE */
   emit_instruction_op0(emit, VGPU10_OPCODE_ELSE);

   /* U/IBFE dst, width, offset, value */
   emit_instruction_op3(emit, translate_opcode(opcode), &inst->Dst[0],
                        &inst->Src[2], &inst->Src[1], &inst->Src[0]);

   /* ENDIF */
   emit_instruction_op0(emit, VGPU10_OPCODE_ENDIF);

   free_temp_indexes(emit);
   return true;
}


/**
 * Emit BFI  instruction
 * tgsi representation:
 * BFI dst, base, insert, offset, width
 * SM5 representation:
 * BFI dst, width, offset, insert, base
 * Note: SM5 has width & offset range (0-31);
 *      whereas GLSL has width & offset range (0-32)
 */
static bool
emit_bfi(struct svga_shader_emitter_v10 *emit,
         const struct tgsi_full_instruction *inst)
{
   const enum tgsi_opcode opcode = inst->Instruction.Opcode;
   struct tgsi_full_src_register imm32 = make_immediate_reg_int(emit, 32);
   imm32 = scalar_src(&imm32, TGSI_SWIZZLE_X);

   struct tgsi_full_src_register zero = make_immediate_reg_int(emit, 0);
   zero = scalar_src(&zero, TGSI_SWIZZLE_X);

   unsigned tmp1 = get_temp_index(emit);
   const struct tgsi_full_dst_register cond1_dst = make_dst_temp_reg(tmp1);
   const struct tgsi_full_dst_register cond1_dst_x =
      writemask_dst(&cond1_dst, TGSI_WRITEMASK_X);
   const struct tgsi_full_src_register cond1_src_x =
      make_src_scalar_reg(TGSI_FILE_TEMPORARY, tmp1, TGSI_SWIZZLE_X);

   unsigned tmp2 = get_temp_index(emit);
   const struct tgsi_full_dst_register cond2_dst = make_dst_temp_reg(tmp2);
   const struct tgsi_full_dst_register cond2_dst_x =
      writemask_dst(&cond2_dst, TGSI_WRITEMASK_X);
   const struct tgsi_full_src_register cond2_src_x =
      make_src_scalar_reg(TGSI_FILE_TEMPORARY, tmp2, TGSI_SWIZZLE_X);

   /**
    * In SM5, when width = 32  and offset = 0, it returns 0.
    * On the other hand GLSL, expects insert to be copied as it is, to dst.
    */

   /* cond1 = width == 32 */
   emit_instruction_op2(emit, VGPU10_OPCODE_IEQ,
                        &cond1_dst_x, &inst->Src[3], &imm32);

   /* cond1 = offset == 0 */
   emit_instruction_op2(emit, VGPU10_OPCODE_IEQ,
                        &cond2_dst_x, &inst->Src[2], &zero);

   /* cond2 = cond1 & cond2 */
   emit_instruction_op2(emit, VGPU10_OPCODE_AND,
                        &cond2_dst_x, &cond2_src_x, &cond1_src_x);

   /* if */
   emit_if(emit, &cond2_src_x);

   emit_instruction_op1(emit, VGPU10_OPCODE_MOV, &inst->Dst[0],
                        &inst->Src[1]);

   /* else */
   emit_instruction_op0(emit, VGPU10_OPCODE_ELSE);

   /* BFI dst, width, offset, insert, base */
   begin_emit_instruction(emit);
   emit_opcode(emit, translate_opcode(opcode), inst->Instruction.Saturate);
   emit_dst_register(emit, &inst->Dst[0]);
   emit_src_register(emit, &inst->Src[3]);
   emit_src_register(emit, &inst->Src[2]);
   emit_src_register(emit, &inst->Src[1]);
   emit_src_register(emit, &inst->Src[0]);
   end_emit_instruction(emit);

   /* endif */
   emit_instruction_op0(emit, VGPU10_OPCODE_ENDIF);

   free_temp_indexes(emit);
   return true;
}


/**
 * We only special case the MOV instruction to try to detect constant
 * color writes in the fragment shader.
 */
static bool
emit_mov(struct svga_shader_emitter_v10 *emit,
         const struct tgsi_full_instruction *inst)
{
   const struct tgsi_full_src_register *src = &inst->Src[0];
   const struct tgsi_full_dst_register *dst = &inst->Dst[0];

   if (emit->unit == PIPE_SHADER_FRAGMENT &&
       dst->Register.File == TGSI_FILE_OUTPUT &&
       dst->Register.Index == 0 &&
       src->Register.File == TGSI_FILE_CONSTANT &&
       !src->Register.Indirect) {
      emit->constant_color_output = true;
   }

   return emit_simple(emit, inst);
}


/**
 * Emit a simple VGPU10 instruction which writes to multiple dest registers,
 * where TGSI only uses one dest register.
 */
static bool
emit_simple_1dst(struct svga_shader_emitter_v10 *emit,
                 const struct tgsi_full_instruction *inst,
                 unsigned dst_count,
                 unsigned dst_index)
{
   const enum tgsi_opcode opcode = inst->Instruction.Opcode;
   const struct tgsi_opcode_info *op = tgsi_get_opcode_info(opcode);
   unsigned i;

   begin_emit_instruction(emit);
   emit_opcode(emit, translate_opcode(opcode), inst->Instruction.Saturate);

   for (i = 0; i < dst_count; i++) {
      if (i == dst_index) {
         emit_dst_register(emit, &inst->Dst[0]);
      } else {
         emit_null_dst_register(emit);
      }
   }

   for (i = 0; i < op->num_src; i++) {
      emit_src_register(emit, &inst->Src[i]);
   }
   end_emit_instruction(emit);

   return true;
}


/**
 * Emit a vmware specific VGPU10 instruction.
 */
static bool
emit_vmware(struct svga_shader_emitter_v10 *emit,
            const struct tgsi_full_instruction *inst,
            VGPU10_VMWARE_OPCODE_TYPE subopcode)
{
   VGPU10OpcodeToken0 token0;
   const enum tgsi_opcode opcode = inst->Instruction.Opcode;
   const struct tgsi_opcode_info *op = tgsi_get_opcode_info(opcode);
   const bool dbl_dst = opcode_has_dbl_dst(inst->Instruction.Opcode);
   const bool dbl_src = opcode_has_dbl_src(inst->Instruction.Opcode);
   unsigned i;
   struct tgsi_full_src_register src[3];

   for (i = 0; i < op->num_src; i++) {
      if (dbl_src)
         src[i] = check_double_src(emit, &inst->Src[i]);
      else
         src[i] = inst->Src[i];
   }

   begin_emit_instruction(emit);

   assert((subopcode > 0 && emit->version >= 50) || subopcode == 0);

   token0.value = 0;
   token0.opcodeType = VGPU10_OPCODE_VMWARE;
   token0.vmwareOpcodeType = subopcode;
   emit_dword(emit, token0.value);

   if (subopcode == VGPU10_VMWARE_OPCODE_IDIV) {
      /* IDIV only uses the first dest register. */
      emit_dst_register(emit, &inst->Dst[0]);
      emit_null_dst_register(emit);
   } else {
      for (i = 0; i < op->num_dst; i++) {
         if (dbl_dst) {
            check_double_dst_writemask(inst);
         }
         emit_dst_register(emit, &inst->Dst[i]);
      }
   }

   for (i = 0; i < op->num_src; i++) {
      emit_src_register(emit, &src[i]);
   }
   end_emit_instruction(emit);

   free_temp_indexes(emit);
   return true;
}

/**
 * Emit a memory register
 */

typedef enum {
   MEM_STORE = 0,
   MEM_LOAD = 1,
   MEM_ATOMIC_COUNTER
} memory_op;

static void
emit_memory_register(struct svga_shader_emitter_v10 *emit,
                     memory_op mem_op,
                     const struct tgsi_full_instruction *inst,
                     unsigned regIndex, unsigned writemask)
{
   VGPU10OperandToken0 operand0;
   unsigned resIndex = 0;

   operand0.value = 0;
   operand0.operandType = VGPU10_OPERAND_TYPE_THREAD_GROUP_SHARED_MEMORY;
   operand0.indexDimension = VGPU10_OPERAND_INDEX_1D;
   operand0.numComponents = VGPU10_OPERAND_4_COMPONENT;

   switch (mem_op) {
   case MEM_ATOMIC_COUNTER:
   {
      operand0.numComponents = VGPU10_OPERAND_0_COMPONENT;
      resIndex = inst->Src[regIndex].Register.Index;
      break;
   }
   case MEM_STORE:
   {
      const struct tgsi_full_dst_register *reg = &inst->Dst[regIndex];

      operand0.selectionMode = VGPU10_OPERAND_4_COMPONENT_MASK_MODE;
      operand0.mask = writemask;
      resIndex = reg->Register.Index;
      break;
   }
   case MEM_LOAD:
   {
      const struct tgsi_full_src_register *reg = &inst->Src[regIndex];

      operand0.selectionMode = VGPU10_OPERAND_4_COMPONENT_SWIZZLE_MODE;
      operand0.swizzleX = reg->Register.SwizzleX;
      operand0.swizzleY = reg->Register.SwizzleY;
      operand0.swizzleZ = reg->Register.SwizzleZ;
      operand0.swizzleW = reg->Register.SwizzleW;
      resIndex = reg->Register.Index;
      break;
   }
   default:
      assert(!"Unexpected memory opcode");
      break;
   }

   emit_dword(emit, operand0.value);
   emit_dword(emit, resIndex);
}


typedef enum {
   UAV_STORE = 0,
   UAV_LOAD = 1,
   UAV_ATOMIC = 2,
   UAV_RESQ = 3,
} UAV_OP;


/**
 * Emit a uav register
 * \param uav_index     index of resource register
 * \param uav_op        UAV_STORE/ UAV_LOAD/ UAV_ATOMIC depending on opcode
 * \param resourceType  resource file type
 * \param writemask     resource writemask
 */

static void
emit_uav_register(struct svga_shader_emitter_v10 *emit,
                  unsigned res_index, UAV_OP uav_op,
                  enum tgsi_file_type resourceType, unsigned writemask)
{
   VGPU10OperandToken0 operand0;
   unsigned uav_index = INVALID_INDEX;

   operand0.value = 0;
   operand0.operandType = VGPU10_OPERAND_TYPE_UAV;
   operand0.indexDimension = VGPU10_OPERAND_INDEX_1D;
   operand0.numComponents = VGPU10_OPERAND_4_COMPONENT;

   switch (resourceType) {
   case TGSI_FILE_IMAGE:
      uav_index = emit->key.images[res_index].uav_index;
      break;
   case TGSI_FILE_BUFFER:
      uav_index = emit->key.shader_buf_uav_index[res_index];
      break;
   case TGSI_FILE_HW_ATOMIC:
      uav_index = emit->key.atomic_buf_uav_index[res_index];
      break;
   default:
      assert(0);
   }

   switch (uav_op) {
   case UAV_ATOMIC:
      operand0.numComponents = VGPU10_OPERAND_0_COMPONENT;
      break;

   case UAV_STORE:
      operand0.selectionMode = VGPU10_OPERAND_4_COMPONENT_MASK_MODE;
      operand0.mask = writemask;
      break;

   case UAV_LOAD:
   case UAV_RESQ:
      operand0.selectionMode = VGPU10_OPERAND_4_COMPONENT_SWIZZLE_MODE;
      operand0.swizzleX = VGPU10_COMPONENT_X;
      operand0.swizzleY = VGPU10_COMPONENT_Y;
      operand0.swizzleZ = VGPU10_COMPONENT_Z;
      operand0.swizzleW = VGPU10_COMPONENT_W;
      break;

   default:
      break;
   }

   emit_dword(emit, operand0.value);
   emit_dword(emit, uav_index);
}


/**
 * A helper function to emit the uav address.
 * For memory, buffer, and image resource, it is set to the specified address.
 * For HW atomic counter, the address is the sum of the address offset and the
 * offset into the HW atomic buffer as specified by the register index.
 * It is also possible to specify the counter index as an indirect address.
 * And in this case, the uav address will be the sum of the address offset and the
 * counter index specified in the indirect address.
 */
static
struct tgsi_full_src_register
emit_uav_addr_offset(struct svga_shader_emitter_v10 *emit,
                     enum tgsi_file_type resourceType,
                     unsigned resourceIndex,
                     unsigned resourceIndirect,
                     unsigned resourceIndirectIndex,
                     const struct tgsi_full_src_register *addr_reg)
{
   unsigned addr_tmp;
   struct tgsi_full_dst_register addr_dst;
   struct tgsi_full_src_register addr_src;
   struct tgsi_full_src_register two = make_immediate_reg_int(emit, 2);
   struct tgsi_full_src_register zero = make_immediate_reg_int(emit, 0);

   addr_tmp = get_temp_index(emit);
   addr_dst = make_dst_temp_reg(addr_tmp);
   addr_src = make_src_temp_reg(addr_tmp);

   /* specified address offset */
   if (addr_reg)
      emit_instruction_op1(emit, VGPU10_OPCODE_MOV, &addr_dst, addr_reg);
   else
      emit_instruction_op1(emit, VGPU10_OPCODE_MOV, &addr_dst, &zero);

   /* For HW atomic counter, we need to find the index to the
    * HW atomic buffer.
    */
   if (resourceType == TGSI_FILE_HW_ATOMIC) {
      if (resourceIndirect) {

         /**
          * uav addr offset  = counter layout offset +
          *                    counter indirect index address + address offset
          */

         /* counter layout offset */
         struct tgsi_full_src_register layout_offset;
         layout_offset =
            make_immediate_reg_int(emit, resourceIndex);

         /* counter layout offset + address offset */
         emit_instruction_op2(emit, VGPU10_OPCODE_IADD, &addr_dst,
                              &addr_src, &layout_offset);

         /* counter indirect index address */
         unsigned indirect_addr =
            emit->address_reg_index[resourceIndirectIndex];

         struct tgsi_full_src_register indirect_addr_src =
            make_src_temp_reg(indirect_addr);

         indirect_addr_src = scalar_src(&indirect_addr_src, TGSI_SWIZZLE_X);

         /* counter layout offset + address offset + counter indirect address */
         emit_instruction_op2(emit, VGPU10_OPCODE_IADD, &addr_dst,
                              &addr_src, &indirect_addr_src);

      } else {
         struct tgsi_full_src_register index_src;

         index_src = make_immediate_reg_int(emit, resourceIndex);

         /* uav addr offset  = counter index address + address offset */
         emit_instruction_op2(emit, VGPU10_OPCODE_ADD, &addr_dst,
                              &addr_src, &index_src);
      }

      /* HW atomic buffer is declared as raw buffer, so the buffer address is
       * the byte offset, so we need to multiple the counter addr offset by 4.
       */
      emit_instruction_op2(emit, VGPU10_OPCODE_ISHL, &addr_dst,
                           &addr_src, &two);
   }
   else if (resourceType == TGSI_FILE_IMAGE) {
      if ((emit->key.images[resourceIndex].resource_target == PIPE_TEXTURE_3D)
             && emit->key.images[resourceIndex].is_single_layer) {

         struct tgsi_full_dst_register addr_dst_z =
            writemask_dst(&addr_dst, TGSI_WRITEMASK_Z);

         /* For non-layered 3D texture image view, we have to make sure the z
          * component of the address offset is set to 0.
          */
         emit_instruction_op1(emit, VGPU10_OPCODE_MOV, &addr_dst_z,
                              &zero);
      }
   }

   return addr_src;
}



/**
 * A helper function to expand indirect indexing to uav resource
 * by looping through the resource array, compare the indirect index and
 * emit the instruction for each resource in the array.
 */
static void
loop_instruction(unsigned index, unsigned count,
                 struct tgsi_full_src_register *addr_index,
                 void (*fb)(struct svga_shader_emitter_v10 *,
                            const struct tgsi_full_instruction *, unsigned),
                 struct svga_shader_emitter_v10 *emit,
                 const struct tgsi_full_instruction *inst)
{
   if (count == 0)
      return;

   if (index > 0) {
      /* ELSE */
      emit_instruction_op0(emit, VGPU10_OPCODE_ELSE);
   }

   struct tgsi_full_src_register index_src =
                                    make_immediate_reg_int(emit, index);

   unsigned tmp_index = get_temp_index(emit);
   struct tgsi_full_src_register tmp_src = make_src_temp_reg(tmp_index);
   struct tgsi_full_src_register tmp_src_x =
                scalar_src(&tmp_src, TGSI_SWIZZLE_X);
   struct tgsi_full_dst_register tmp_dst = make_dst_temp_reg(tmp_index);

   /* IEQ tmp, addr_tmp_index, index */
   emit_instruction_op2(emit, VGPU10_OPCODE_IEQ, &tmp_dst,
                        addr_index, &index_src);

   /* IF tmp */
   emit_if(emit, &tmp_src_x);

   free_temp_indexes(emit);

   (*fb)(emit, inst, index);

   loop_instruction(index+1, count-1, addr_index, fb, emit, inst);

   /* ENDIF */
   emit_instruction_op0(emit, VGPU10_OPCODE_ENDIF);
}


/**
 * A helper function to emit the load instruction.
 */
static void
emit_load_instruction(struct svga_shader_emitter_v10 *emit,
                      const struct tgsi_full_instruction *inst,
                      unsigned resourceIndex)
{
   VGPU10OpcodeToken0 token0;
   struct tgsi_full_src_register addr_src;
   enum tgsi_file_type resourceType = inst->Src[0].Register.File;

   /* Resolve the resource address for this resource first */
   addr_src = emit_uav_addr_offset(emit, resourceType, resourceIndex,
                                   inst->Src[0].Register.Indirect,
                                   inst->Src[0].Indirect.Index,
                                   &inst->Src[1]);

   /* LOAD resource, address, src */
   begin_emit_instruction(emit);

   token0.value = 0;

   if (resourceType == TGSI_FILE_MEMORY ||
       resourceType == TGSI_FILE_BUFFER ||
       resourceType == TGSI_FILE_HW_ATOMIC) {
      token0.opcodeType = VGPU10_OPCODE_LD_RAW;
      addr_src = scalar_src(&addr_src, TGSI_SWIZZLE_X);
   }
   else {
      token0.opcodeType = VGPU10_OPCODE_LD_UAV_TYPED;
   }

   token0.saturate = inst->Instruction.Saturate,
   emit_dword(emit, token0.value);

   emit_dst_register(emit, &inst->Dst[0]);
   emit_src_register(emit, &addr_src);

   if (resourceType == TGSI_FILE_MEMORY) {
      emit_memory_register(emit, MEM_LOAD, inst, 0, 0);
   } else if (resourceType == TGSI_FILE_HW_ATOMIC) {
      emit_uav_register(emit, inst->Src[0].Dimension.Index,
                        UAV_LOAD, inst->Src[0].Register.File, 0);
   } else if (resourceType == TGSI_FILE_BUFFER) {
      if (emit->raw_shaderbufs & (1 << resourceIndex))
         emit_resource_register(emit, resourceIndex +
                                      emit->raw_shaderbuf_srv_start_index);
      else
         emit_uav_register(emit, resourceIndex,
                           UAV_LOAD, inst->Src[0].Register.File, 0);
   } else {
      emit_uav_register(emit, resourceIndex,
                        UAV_LOAD, inst->Src[0].Register.File, 0);
   }

   end_emit_instruction(emit);

   free_temp_indexes(emit);
}


/**
 * Emit uav / memory load instruction
 */
static bool
emit_load(struct svga_shader_emitter_v10 *emit,
          const struct tgsi_full_instruction *inst)
{
   enum tgsi_file_type resourceType = inst->Src[0].Register.File;
   unsigned resourceIndex = inst->Src[0].Register.Index;

   /* If the resource register has indirect index, we will need
    * to expand it since SM5 device does not support indirect indexing
    * for uav.
    */
   if (inst->Src[0].Register.Indirect &&
       (resourceType == TGSI_FILE_BUFFER || resourceType == TGSI_FILE_IMAGE)) {

      unsigned indirect_index = inst->Src[0].Indirect.Index;
      unsigned num_resources =
         resourceType == TGSI_FILE_BUFFER ? emit->num_shader_bufs :
                                            emit->num_images;

      /* indirect index tmp register */
      unsigned indirect_addr = emit->address_reg_index[indirect_index];
      struct tgsi_full_src_register indirect_addr_src =
         make_src_temp_reg(indirect_addr);
      indirect_addr_src = scalar_src(&indirect_addr_src, TGSI_SWIZZLE_X);

      /* Add offset to the indirect index */
      if (inst->Src[0].Register.Index != 0) {
         struct tgsi_full_src_register offset =
            make_immediate_reg_int(emit, inst->Src[0].Register.Index);
         struct tgsi_full_dst_register indirect_addr_dst =
            make_dst_temp_reg(indirect_addr);
         emit_instruction_op2(emit, VGPU10_OPCODE_IADD, &indirect_addr_dst,
                              &indirect_addr_src, &offset);
      }

      /* Loop through the resource array to find which resource to use.
       */
      loop_instruction(0, num_resources, &indirect_addr_src,
                       emit_load_instruction, emit, inst);
   }
   else {
      emit_load_instruction(emit, inst, resourceIndex);
   }

   free_temp_indexes(emit);

   return true;
}


/**
 * A helper function to emit a store instruction.
 */
static void
emit_store_instruction(struct svga_shader_emitter_v10 *emit,
                       const struct tgsi_full_instruction *inst,
                       unsigned resourceIndex)
{
   VGPU10OpcodeToken0 token0;
   enum tgsi_file_type resourceType = inst->Dst[0].Register.File;
   unsigned writemask = inst->Dst[0].Register.WriteMask;
   struct tgsi_full_src_register addr_src;

   unsigned tmp_index = get_temp_index(emit);
   struct tgsi_full_src_register tmp_src = make_src_temp_reg(tmp_index);
   struct tgsi_full_dst_register tmp_dst_xyzw = make_dst_temp_reg(tmp_index);
   struct tgsi_full_dst_register tmp_dst;

   struct tgsi_full_src_register src = inst->Src[1];
   struct tgsi_full_src_register four = make_immediate_reg_int(emit, 4);

   bool needLoad = false;
   bool needPerComponentStore = false;
   unsigned swizzles = 0;

   /* Resolve the resource address for this resource first */
   addr_src = emit_uav_addr_offset(emit, resourceType,
                                   inst->Dst[0].Register.Index,
                                   inst->Dst[0].Register.Indirect,
                                   inst->Dst[0].Indirect.Index,
                                   &inst->Src[0]);

   /* First check the writemask to see if it can be supported
    * by the store instruction.
    * store_raw only allows .x, .xy, .xyz, .xyzw. For the typeless memory,
    * we can adjust the address offset, and do a per-component store.
    * store_uav_typed only allows .xyzw. In this case, we need to
    * do a load first, update the temporary and then issue the
    * store. This does have a small risk that if different threads
    * update different components of the same address, data might not be
    * in sync.
    */
   if (resourceType == TGSI_FILE_IMAGE) {
      needLoad = (writemask == TGSI_WRITEMASK_XYZW) ? false : true;
   }
   else if (resourceType == TGSI_FILE_BUFFER ||
            resourceType == TGSI_FILE_MEMORY) {
      if (!(writemask == TGSI_WRITEMASK_X || writemask == TGSI_WRITEMASK_XY ||
            writemask == TGSI_WRITEMASK_XYZ ||
            writemask == TGSI_WRITEMASK_XYZW)) {
         needPerComponentStore = true;
      }
   }

   if (needLoad) {
      assert(resourceType == TGSI_FILE_IMAGE);

      /* LOAD resource, address, src */
      begin_emit_instruction(emit);

      token0.value = 0;
      token0.opcodeType = VGPU10_OPCODE_LD_UAV_TYPED;
      token0.saturate = inst->Instruction.Saturate,
      emit_dword(emit, token0.value);

      emit_dst_register(emit, &tmp_dst_xyzw);
      emit_src_register(emit, &addr_src);
      emit_uav_register(emit, resourceIndex, UAV_LOAD, resourceType, 0);

      end_emit_instruction(emit);

      /* MOV tmp(writemask) src */
      tmp_dst = writemask_dst(&tmp_dst_xyzw, writemask);
      emit_instruction_op1(emit, VGPU10_OPCODE_MOV, &tmp_dst, &inst->Src[1]);

      /* Now set the writemask to xyzw for the store_uav_typed instruction */
      writemask = TGSI_WRITEMASK_XYZW;
   }
   else if (needPerComponentStore) {
      /* Save the src swizzles */
      swizzles = src.Register.SwizzleX |
                 src.Register.SwizzleY << 2 |
                 src.Register.SwizzleZ << 4 |
                 src.Register.SwizzleW << 6;
   }

   bool storeDone = false;
   unsigned perComponentWritemask = writemask;
   unsigned shift = 0;
   struct tgsi_full_src_register shift_src;

   while (!storeDone) {

      if (needPerComponentStore) {
         assert(perComponentWritemask);
         while (!(perComponentWritemask & TGSI_WRITEMASK_X)) {
            shift++;
            perComponentWritemask >>= 1;
         }

         /* First adjust the addr_src to the next component */
         if (shift != 0) {
            struct tgsi_full_dst_register addr_dst =
               make_dst_temp_reg(addr_src.Register.Index);
            shift_src = make_immediate_reg_int(emit, shift);
            emit_instruction_op3(emit, VGPU10_OPCODE_UMAD, &addr_dst, &four,
                                 &shift_src, &addr_src);

            /* Adjust the src swizzle as well */
            swizzles >>= (shift * 2);
         }

         /* Now the address offset is set to the next component,
          * we can set the writemask to .x and make sure to set
          * the src swizzle as well.
          */
         src.Register.SwizzleX = swizzles & 0x3;
         writemask = TGSI_WRITEMASK_X;

         /* Shift for the next component check */
         perComponentWritemask >>= 1;
         shift = 1;
      }

      /* STORE resource, address, src */
      begin_emit_instruction(emit);

      token0.value = 0;
      token0.saturate = inst->Instruction.Saturate;

      if (resourceType == TGSI_FILE_MEMORY) {
         token0.opcodeType = VGPU10_OPCODE_STORE_RAW;
         addr_src = scalar_src(&addr_src, TGSI_SWIZZLE_X);
         emit_dword(emit, token0.value);
         emit_memory_register(emit, MEM_STORE, inst, 0, writemask);
      }
      else if (resourceType == TGSI_FILE_BUFFER ||
               resourceType == TGSI_FILE_HW_ATOMIC) {
         token0.opcodeType = VGPU10_OPCODE_STORE_RAW;
         addr_src = scalar_src(&addr_src, TGSI_SWIZZLE_X);
         emit_dword(emit, token0.value);
         emit_uav_register(emit, resourceIndex, UAV_STORE,
                           resourceType, writemask);
      }
      else {
         token0.opcodeType = VGPU10_OPCODE_STORE_UAV_TYPED;
         emit_dword(emit, token0.value);
         emit_uav_register(emit, resourceIndex, UAV_STORE,
                           resourceType, writemask);
      }

      emit_src_register(emit, &addr_src);

      if (needLoad)
         emit_src_register(emit, &tmp_src);
      else
         emit_src_register(emit, &src);

      end_emit_instruction(emit);

      if (!needPerComponentStore || !perComponentWritemask)
         storeDone = true;
   }

   free_temp_indexes(emit);
}


/**
 * Emit uav / memory store instruction
 */
static bool
emit_store(struct svga_shader_emitter_v10 *emit,
           const struct tgsi_full_instruction *inst)
{
   enum tgsi_file_type resourceType = inst->Dst[0].Register.File;
   unsigned resourceIndex = inst->Dst[0].Register.Index;

   /* If the resource register has indirect index, we will need
    * to expand it since SM5 device does not support indirect indexing
    * for uav.
    */
   if (inst->Dst[0].Register.Indirect &&
       (resourceType == TGSI_FILE_BUFFER || resourceType == TGSI_FILE_IMAGE)) {

      unsigned indirect_index = inst->Dst[0].Indirect.Index;
      unsigned num_resources =
         resourceType == TGSI_FILE_BUFFER ? emit->num_shader_bufs :
                                            emit->num_images;

      /* Indirect index tmp register */
      unsigned indirect_addr = emit->address_reg_index[indirect_index];
      struct tgsi_full_src_register indirect_addr_src =
         make_src_temp_reg(indirect_addr);
      indirect_addr_src = scalar_src(&indirect_addr_src, TGSI_SWIZZLE_X);

      /* Add offset to the indirect index */
      if (inst->Dst[0].Register.Index != 0) {
         struct tgsi_full_src_register offset =
            make_immediate_reg_int(emit, inst->Dst[0].Register.Index);
         struct tgsi_full_dst_register indirect_addr_dst =
            make_dst_temp_reg(indirect_addr);
         emit_instruction_op2(emit, VGPU10_OPCODE_IADD, &indirect_addr_dst,
                              &indirect_addr_src, &offset);
      }

      /* Loop through the resource array to find which resource to use.
       */
      loop_instruction(0, num_resources, &indirect_addr_src,
                       emit_store_instruction, emit, inst);
   }
   else {
      emit_store_instruction(emit, inst, resourceIndex);
   }

   free_temp_indexes(emit);

   return true;
}


/**
 * A helper function to emit an atomic instruction.
 */

static void
emit_atomic_instruction(struct svga_shader_emitter_v10 *emit,
                        const struct tgsi_full_instruction *inst,
                        unsigned resourceIndex)
{
   VGPU10OpcodeToken0 token0;
   enum tgsi_file_type resourceType = inst->Src[0].Register.File;
   struct tgsi_full_src_register addr_src;
   VGPU10_OPCODE_TYPE opcode = emit->cur_atomic_opcode;
   const struct tgsi_full_src_register *offset;

   /* ntt does not specify offset for HWATOMIC. So just set offset to NULL. */
   offset = resourceType == TGSI_FILE_HW_ATOMIC ? NULL : &inst->Src[1];

   /* Resolve the resource address */
   addr_src = emit_uav_addr_offset(emit, resourceType,
                                   inst->Src[0].Register.Index,
                                   inst->Src[0].Register.Indirect,
                                   inst->Src[0].Indirect.Index,
                                   offset);

   /* Emit the atomic operation */
   begin_emit_instruction(emit);

   token0.value = 0;
   token0.opcodeType = opcode;
   token0.saturate = inst->Instruction.Saturate,
   emit_dword(emit, token0.value);

   emit_dst_register(emit, &inst->Dst[0]);

   if (inst->Src[0].Register.File == TGSI_FILE_MEMORY) {
      emit_memory_register(emit, MEM_ATOMIC_COUNTER, inst, 0, 0);
   } else if (inst->Src[0].Register.File == TGSI_FILE_HW_ATOMIC) {
      assert(inst->Src[0].Register.Dimension == 1);
      emit_uav_register(emit, inst->Src[0].Dimension.Index,
                        UAV_ATOMIC, inst->Src[0].Register.File, 0);
   } else {
      emit_uav_register(emit, resourceIndex,
                        UAV_ATOMIC, inst->Src[0].Register.File, 0);
   }

   /* resource address offset */
   emit_src_register(emit, &addr_src);

   struct tgsi_full_src_register src0_x =
         swizzle_src(&inst->Src[2], TGSI_SWIZZLE_X, TGSI_SWIZZLE_X,
                     TGSI_SWIZZLE_X, TGSI_SWIZZLE_X);
   emit_src_register(emit, &src0_x);

   if (opcode == VGPU10_OPCODE_IMM_ATOMIC_CMP_EXCH) {
      struct tgsi_full_src_register src1_x =
         swizzle_src(&inst->Src[3], TGSI_SWIZZLE_X, TGSI_SWIZZLE_X,
                     TGSI_SWIZZLE_X, TGSI_SWIZZLE_X);

      emit_src_register(emit, &src1_x);
   }

   end_emit_instruction(emit);

   free_temp_indexes(emit);
}


/**
 * Emit atomic instruction
 */
static bool
emit_atomic(struct svga_shader_emitter_v10 *emit,
            const struct tgsi_full_instruction *inst,
            VGPU10_OPCODE_TYPE opcode)
{
   enum tgsi_file_type resourceType = inst->Src[0].Register.File;
   unsigned resourceIndex = inst->Src[0].Register.Index;

   emit->cur_atomic_opcode = opcode;

   /* If the resource register has indirect index, we will need
    * to expand it since SM5 device does not support indirect indexing
    * for uav.
    */
   if (inst->Dst[0].Register.Indirect &&
       (resourceType == TGSI_FILE_BUFFER || resourceType == TGSI_FILE_IMAGE)) {

      unsigned indirect_index = inst->Dst[0].Indirect.Index;
      unsigned num_resources =
         resourceType == TGSI_FILE_BUFFER ? emit->num_shader_bufs :
                                            emit->num_images;

      /* indirect index tmp register */
      unsigned indirect_addr = emit->address_reg_index[indirect_index];
      struct tgsi_full_src_register indirect_addr_src =
         make_src_temp_reg(indirect_addr);
      indirect_addr_src = scalar_src(&indirect_addr_src, TGSI_SWIZZLE_X);

      /* Loop through the resource array to find which resource to use.
       */
      loop_instruction(0, num_resources, &indirect_addr_src,
                       emit_atomic_instruction, emit, inst);
   }
   else {
      emit_atomic_instruction(emit, inst, resourceIndex);
   }

   free_temp_indexes(emit);

   return true;
}


/**
 * Emit barrier instruction
 */
static bool
emit_barrier(struct svga_shader_emitter_v10 *emit,
             const struct tgsi_full_instruction *inst)
{
   VGPU10OpcodeToken0 token0;

   assert(emit->version >= 50);

   token0.value = 0;
   token0.opcodeType = VGPU10_OPCODE_SYNC;

   if (emit->unit == PIPE_SHADER_TESS_CTRL && emit->version == 50) {
      /* SM5 device doesn't support BARRIER in tcs . If barrier is used
       * in shader, don't do anything for this opcode and continue rest
       * of shader translation
       */
      util_debug_message(&emit->svga_debug_callback, INFO,
                         "barrier instruction is not supported in tessellation control shader\n");
      return true;
   }
   else if (emit->unit == PIPE_SHADER_COMPUTE) {
      if (emit->cs.shared_memory_declared)
         token0.syncThreadGroupShared = 1;

      if (emit->uav_declared)
         token0.syncUAVMemoryGroup = 1;

      token0.syncThreadsInGroup = 1;
   } else {
      token0.syncUAVMemoryGlobal = 1;
   }

   assert(token0.syncUAVMemoryGlobal || token0.syncUAVMemoryGroup ||
          token0.syncThreadGroupShared);

   begin_emit_instruction(emit);
   emit_dword(emit, token0.value);
   end_emit_instruction(emit);

   return true;
}

/**
 * Emit memory barrier instruction
 */
static bool
emit_memory_barrier(struct svga_shader_emitter_v10 *emit,
                    const struct tgsi_full_instruction *inst)
{
   unsigned index = inst->Src[0].Register.Index;
   unsigned swizzle = inst->Src[0].Register.SwizzleX;
   unsigned bartype = emit->immediates[index][swizzle].Int;
   VGPU10OpcodeToken0 token0;

   token0.value = 0;
   token0.opcodeType = VGPU10_OPCODE_SYNC;

   if (emit->unit == PIPE_SHADER_COMPUTE) {

      /* For compute shader, issue sync opcode with different options
       * depending on the memory barrier type.
       *
       * Bit 0: Shader storage buffers
       * Bit 1: Atomic buffers
       * Bit 2: Images
       * Bit 3: Shared memory
       * Bit 4: Thread group
       */

      if (bartype & (TGSI_MEMBAR_SHADER_BUFFER | TGSI_MEMBAR_ATOMIC_BUFFER |
                     TGSI_MEMBAR_SHADER_IMAGE))
         token0.syncUAVMemoryGlobal = 1;
      else if (bartype & TGSI_MEMBAR_THREAD_GROUP)
         token0.syncUAVMemoryGroup = 1;

      if (bartype & TGSI_MEMBAR_SHARED)
         token0.syncThreadGroupShared = 1;
   }
   else {
      /**
       * For graphics stages, only sync_uglobal is available.
       */
      if (bartype & (TGSI_MEMBAR_SHADER_BUFFER | TGSI_MEMBAR_ATOMIC_BUFFER |
                     TGSI_MEMBAR_SHADER_IMAGE))
         token0.syncUAVMemoryGlobal = 1;
   }

   assert(token0.syncUAVMemoryGlobal || token0.syncUAVMemoryGroup ||
          token0.syncThreadGroupShared);

   begin_emit_instruction(emit);
   emit_dword(emit, token0.value);
   end_emit_instruction(emit);

   return true;
}


/**
 * Emit code for TGSI_OPCODE_RESQ (image size) instruction.
 */
static bool
emit_resq(struct svga_shader_emitter_v10 *emit,
          const struct tgsi_full_instruction *inst)
{
   struct tgsi_full_src_register zero =
      make_immediate_reg_int(emit, 0);

   unsigned uav_resource = emit->image[inst->Src[0].Register.Index].Resource;

   if (uav_resource == TGSI_TEXTURE_CUBE_ARRAY) {
      struct tgsi_full_src_register image_src;

      image_src = make_src_const_reg(emit->image_size_index + inst->Src[0].Register.Index);

      emit_instruction_op1(emit, VGPU10_OPCODE_MOV, &inst->Dst[0], &image_src);
      return true;
   }

   begin_emit_instruction(emit);
   if (uav_resource == TGSI_TEXTURE_BUFFER) {
      emit_opcode(emit, VGPU10_OPCODE_BUFINFO, false);
      emit_dst_register(emit, &inst->Dst[0]);
   }
   else {
      emit_opcode_resinfo(emit, VGPU10_RESINFO_RETURN_UINT);
      emit_dst_register(emit, &inst->Dst[0]);
      emit_src_register(emit, &zero);
   }
   emit_uav_register(emit, inst->Src[0].Register.Index,
                     UAV_RESQ, inst->Src[0].Register.File, 0);
   end_emit_instruction(emit);

   return true;
}


static bool
emit_instruction(struct svga_shader_emitter_v10 *emit,
                 unsigned inst_number,
                 const struct tgsi_full_instruction *inst)
{
   const enum tgsi_opcode opcode = inst->Instruction.Opcode;

   switch (opcode) {
   case TGSI_OPCODE_ADD:
   case TGSI_OPCODE_AND:
   case TGSI_OPCODE_BGNLOOP:
   case TGSI_OPCODE_BRK:
   case TGSI_OPCODE_CEIL:
   case TGSI_OPCODE_CONT:
   case TGSI_OPCODE_DDX:
   case TGSI_OPCODE_DDY:
   case TGSI_OPCODE_DIV:
   case TGSI_OPCODE_DP2:
   case TGSI_OPCODE_DP3:
   case TGSI_OPCODE_DP4:
   case TGSI_OPCODE_ELSE:
   case TGSI_OPCODE_ENDIF:
   case TGSI_OPCODE_ENDLOOP:
   case TGSI_OPCODE_ENDSUB:
   case TGSI_OPCODE_F2I:
   case TGSI_OPCODE_F2U:
   case TGSI_OPCODE_FLR:
   case TGSI_OPCODE_FRC:
   case TGSI_OPCODE_FSEQ:
   case TGSI_OPCODE_FSGE:
   case TGSI_OPCODE_FSLT:
   case TGSI_OPCODE_FSNE:
   case TGSI_OPCODE_I2F:
   case TGSI_OPCODE_IMAX:
   case TGSI_OPCODE_IMIN:
   case TGSI_OPCODE_INEG:
   case TGSI_OPCODE_ISGE:
   case TGSI_OPCODE_ISHR:
   case TGSI_OPCODE_ISLT:
   case TGSI_OPCODE_MAD:
   case TGSI_OPCODE_MAX:
   case TGSI_OPCODE_MIN:
   case TGSI_OPCODE_MUL:
   case TGSI_OPCODE_NOP:
   case TGSI_OPCODE_NOT:
   case TGSI_OPCODE_OR:
   case TGSI_OPCODE_UADD:
   case TGSI_OPCODE_USEQ:
   case TGSI_OPCODE_USGE:
   case TGSI_OPCODE_USLT:
   case TGSI_OPCODE_UMIN:
   case TGSI_OPCODE_UMAD:
   case TGSI_OPCODE_UMAX:
   case TGSI_OPCODE_ROUND:
   case TGSI_OPCODE_SQRT:
   case TGSI_OPCODE_SHL:
   case TGSI_OPCODE_TRUNC:
   case TGSI_OPCODE_U2F:
   case TGSI_OPCODE_UCMP:
   case TGSI_OPCODE_USHR:
   case TGSI_OPCODE_USNE:
   case TGSI_OPCODE_XOR:
   /* Begin SM5 opcodes */
   case TGSI_OPCODE_F2D:
   case TGSI_OPCODE_D2F:
   case TGSI_OPCODE_DADD:
   case TGSI_OPCODE_DMUL:
   case TGSI_OPCODE_DMAX:
   case TGSI_OPCODE_DMIN:
   case TGSI_OPCODE_DSGE:
   case TGSI_OPCODE_DSLT:
   case TGSI_OPCODE_DSEQ:
   case TGSI_OPCODE_DSNE:
   case TGSI_OPCODE_BREV:
   case TGSI_OPCODE_POPC:
   case TGSI_OPCODE_LSB:
   case TGSI_OPCODE_INTERP_CENTROID:
   case TGSI_OPCODE_INTERP_SAMPLE:
      /* simple instructions */
      return emit_simple(emit, inst);
   case TGSI_OPCODE_RET:
      if (emit->unit == PIPE_SHADER_TESS_CTRL &&
          !emit->tcs.control_point_phase) {

         /* store the tessellation levels in the patch constant phase only */
         store_tesslevels(emit);
      }
      return emit_simple(emit, inst);

   case TGSI_OPCODE_IMSB:
   case TGSI_OPCODE_UMSB:
      return emit_msb(emit, inst);
   case TGSI_OPCODE_IBFE:
   case TGSI_OPCODE_UBFE:
      return emit_bfe(emit, inst);
   case TGSI_OPCODE_BFI:
      return emit_bfi(emit, inst);
   case TGSI_OPCODE_MOV:
      return emit_mov(emit, inst);
   case TGSI_OPCODE_EMIT:
      return emit_vertex(emit, inst);
   case TGSI_OPCODE_ENDPRIM:
      return emit_endprim(emit, inst);
   case TGSI_OPCODE_IABS:
      return emit_iabs(emit, inst);
   case TGSI_OPCODE_ARL:
      FALLTHROUGH;
   case TGSI_OPCODE_UARL:
      return emit_arl_uarl(emit, inst);
   case TGSI_OPCODE_BGNSUB:
      /* no-op */
      return true;
   case TGSI_OPCODE_CAL:
      return emit_cal(emit, inst);
   case TGSI_OPCODE_CMP:
      return emit_cmp(emit, inst);
   case TGSI_OPCODE_COS:
      return emit_sincos(emit, inst);
   case TGSI_OPCODE_DST:
      return emit_dst(emit, inst);
   case TGSI_OPCODE_EX2:
      return emit_ex2(emit, inst);
   case TGSI_OPCODE_EXP:
      return emit_exp(emit, inst);
   case TGSI_OPCODE_IF:
      return emit_if(emit, &inst->Src[0]);
   case TGSI_OPCODE_KILL:
      return emit_discard(emit, inst);
   case TGSI_OPCODE_KILL_IF:
      return emit_cond_discard(emit, inst);
   case TGSI_OPCODE_LG2:
      return emit_lg2(emit, inst);
   case TGSI_OPCODE_LIT:
      return emit_lit(emit, inst);
   case TGSI_OPCODE_LODQ:
      return emit_lodq(emit, inst);
   case TGSI_OPCODE_LOG:
      return emit_log(emit, inst);
   case TGSI_OPCODE_LRP:
      return emit_lrp(emit, inst);
   case TGSI_OPCODE_POW:
      return emit_pow(emit, inst);
   case TGSI_OPCODE_RCP:
      return emit_rcp(emit, inst);
   case TGSI_OPCODE_RSQ:
      return emit_rsq(emit, inst);
   case TGSI_OPCODE_SAMPLE:
      return emit_sample(emit, inst);
   case TGSI_OPCODE_SEQ:
      return emit_seq(emit, inst);
   case TGSI_OPCODE_SGE:
      return emit_sge(emit, inst);
   case TGSI_OPCODE_SGT:
      return emit_sgt(emit, inst);
   case TGSI_OPCODE_SIN:
      return emit_sincos(emit, inst);
   case TGSI_OPCODE_SLE:
      return emit_sle(emit, inst);
   case TGSI_OPCODE_SLT:
      return emit_slt(emit, inst);
   case TGSI_OPCODE_SNE:
      return emit_sne(emit, inst);
   case TGSI_OPCODE_SSG:
      return emit_ssg(emit, inst);
   case TGSI_OPCODE_ISSG:
      return emit_issg(emit, inst);
   case TGSI_OPCODE_TEX:
      return emit_tex(emit, inst);
   case TGSI_OPCODE_TG4:
      return emit_tg4(emit, inst);
   case TGSI_OPCODE_TEX2:
      return emit_tex2(emit, inst);
   case TGSI_OPCODE_TXP:
      return emit_txp(emit, inst);
   case TGSI_OPCODE_TXB:
   case TGSI_OPCODE_TXB2:
   case TGSI_OPCODE_TXL:
      return emit_txl_txb(emit, inst);
   case TGSI_OPCODE_TXD:
      return emit_txd(emit, inst);
   case TGSI_OPCODE_TXF:
      return emit_txf(emit, inst);
   case TGSI_OPCODE_TXL2:
      return emit_txl2(emit, inst);
   case TGSI_OPCODE_TXQ:
      return emit_txq(emit, inst);
   case TGSI_OPCODE_UIF:
      return emit_if(emit, &inst->Src[0]);
   case TGSI_OPCODE_UMUL_HI:
   case TGSI_OPCODE_IMUL_HI:
   case TGSI_OPCODE_UDIV:
      /* These cases use only the FIRST of two destination registers */
      return emit_simple_1dst(emit, inst, 2, 0);
   case TGSI_OPCODE_IDIV:
      return emit_vmware(emit, inst, VGPU10_VMWARE_OPCODE_IDIV);
   case TGSI_OPCODE_UMUL:
   case TGSI_OPCODE_UMOD:
   case TGSI_OPCODE_MOD:
      /* These cases use only the SECOND of two destination registers */
      return emit_simple_1dst(emit, inst, 2, 1);

   /* Begin SM5 opcodes */
   case TGSI_OPCODE_DABS:
      return emit_dabs(emit, inst);
   case TGSI_OPCODE_DNEG:
      return emit_dneg(emit, inst);
   case TGSI_OPCODE_DRCP:
      return emit_simple(emit, inst);
   case TGSI_OPCODE_DSQRT:
      return emit_dsqrt(emit, inst);
   case TGSI_OPCODE_DMAD:
      return emit_dmad(emit, inst);
   case TGSI_OPCODE_DFRAC:
      return emit_vmware(emit, inst, VGPU10_VMWARE_OPCODE_DFRC);
   case TGSI_OPCODE_D2I:
   case TGSI_OPCODE_D2U:
      return emit_simple(emit, inst);
   case TGSI_OPCODE_I2D:
   case TGSI_OPCODE_U2D:
      return emit_simple(emit, inst);
   case TGSI_OPCODE_DRSQ:
      return emit_drsq(emit, &inst->Dst[0], &inst->Src[0]);
   case TGSI_OPCODE_DDIV:
      return emit_simple(emit, inst);
   case TGSI_OPCODE_INTERP_OFFSET:
      return emit_interp_offset(emit, inst);
   case TGSI_OPCODE_FMA:
   case TGSI_OPCODE_DFMA:
      return emit_simple(emit, inst);

   case TGSI_OPCODE_DTRUNC:
      return emit_dtrunc(emit, inst);

   /* The following opcodes should never be seen here.  We return zero
    * for PIPE_CAP_TGSI_DROUND_SUPPORTED.
    */
   case TGSI_OPCODE_LDEXP:
   case TGSI_OPCODE_DSSG:
   case TGSI_OPCODE_DLDEXP:
   case TGSI_OPCODE_DCEIL:
   case TGSI_OPCODE_DFLR:
      debug_printf("Unexpected TGSI opcode %s.  "
                   "Should have been translated away by the GLSL compiler.\n",
                   tgsi_get_opcode_name(opcode));
      return false;

   case TGSI_OPCODE_LOAD:
      return emit_load(emit, inst);

   case TGSI_OPCODE_STORE:
      return emit_store(emit, inst);

   case TGSI_OPCODE_ATOMAND:
      return emit_atomic(emit, inst, VGPU10_OPCODE_IMM_ATOMIC_AND);

   case TGSI_OPCODE_ATOMCAS:
      return emit_atomic(emit, inst, VGPU10_OPCODE_IMM_ATOMIC_CMP_EXCH);

   case TGSI_OPCODE_ATOMIMAX:
      return emit_atomic(emit, inst, VGPU10_OPCODE_IMM_ATOMIC_IMAX);

   case TGSI_OPCODE_ATOMIMIN:
      return emit_atomic(emit, inst, VGPU10_OPCODE_IMM_ATOMIC_IMIN);

   case TGSI_OPCODE_ATOMOR:
      return emit_atomic(emit, inst, VGPU10_OPCODE_IMM_ATOMIC_OR);

   case TGSI_OPCODE_ATOMUADD:
      return emit_atomic(emit, inst, VGPU10_OPCODE_IMM_ATOMIC_IADD);

   case TGSI_OPCODE_ATOMUMAX:
      return emit_atomic(emit, inst, VGPU10_OPCODE_IMM_ATOMIC_UMAX);

   case TGSI_OPCODE_ATOMUMIN:
      return emit_atomic(emit, inst, VGPU10_OPCODE_IMM_ATOMIC_UMIN);

   case TGSI_OPCODE_ATOMXCHG:
      return emit_atomic(emit, inst, VGPU10_OPCODE_IMM_ATOMIC_EXCH);

   case TGSI_OPCODE_ATOMXOR:
      return emit_atomic(emit, inst, VGPU10_OPCODE_IMM_ATOMIC_XOR);

   case TGSI_OPCODE_BARRIER:
      return emit_barrier(emit, inst);

   case TGSI_OPCODE_MEMBAR:
      return emit_memory_barrier(emit, inst);

   case TGSI_OPCODE_RESQ:
      return emit_resq(emit, inst);

   case TGSI_OPCODE_END:
      if (!emit_post_helpers(emit))
         return false;
      return emit_simple(emit, inst);

   default:
      debug_printf("Unimplemented tgsi instruction %s\n",
                   tgsi_get_opcode_name(opcode));
      return false;
   }

   return true;
}


/**
 * Translate a single TGSI instruction to VGPU10.
 */
static bool
emit_vgpu10_instruction(struct svga_shader_emitter_v10 *emit,
                        unsigned inst_number,
                        const struct tgsi_full_instruction *inst)
{
   if (emit->skip_instruction)
      return true;

   bool ret = true;
   unsigned start_token = emit_get_num_tokens(emit);

   emit->reemit_tgsi_instruction = false;

   ret = emit_instruction(emit, inst_number, inst);

   if (emit->reemit_tgsi_instruction) {
      /**
       * Reset emit->ptr to where the translation of this tgsi instruction
       * started.
       */
      VGPU10OpcodeToken0 *tokens = (VGPU10OpcodeToken0 *) emit->buf;
      emit->ptr = (char *) (tokens + start_token);

      emit->reemit_tgsi_instruction = false;
   }
   return ret;
}


/**
 * Emit the extra instructions to adjust the vertex position.
 * There are two possible adjustments:
 * 1. Converting from Gallium to VGPU10 coordinate space by applying the
 *    "prescale" and "pretranslate" values.
 * 2. Undoing the viewport transformation when we use the swtnl/draw path.
 * \param vs_pos_tmp_index  which temporary register contains the vertex pos.
 */
static void
emit_vpos_instructions(struct svga_shader_emitter_v10 *emit)
{
   struct tgsi_full_src_register tmp_pos_src;
   struct tgsi_full_dst_register pos_dst;
   const unsigned vs_pos_tmp_index = emit->vposition.tmp_index;

   /* Don't bother to emit any extra vertex instructions if vertex position is
    * not written out
    */
   if (emit->vposition.out_index == INVALID_INDEX)
      return;

   /**
    * Reset the temporary vertex position register index
    * so that emit_dst_register() will use the real vertex position output
    */
   emit->vposition.tmp_index = INVALID_INDEX;

   tmp_pos_src = make_src_temp_reg(vs_pos_tmp_index);
   pos_dst = make_dst_output_reg(emit->vposition.out_index);

   /* If non-adjusted vertex position register index
    * is valid, copy the vertex position from the temporary
    * vertex position register before it is modified by the
    * prescale computation.
    */
   if (emit->vposition.so_index != INVALID_INDEX) {
      struct tgsi_full_dst_register pos_so_dst =
         make_dst_output_reg(emit->vposition.so_index);

      /* MOV pos_so, tmp_pos */
      emit_instruction_op1(emit, VGPU10_OPCODE_MOV, &pos_so_dst, &tmp_pos_src);
   }

   if (emit->vposition.need_prescale) {
      /* This code adjusts the vertex position to match the VGPU10 convention.
       * If p is the position computed by the shader (usually by applying the
       * modelview and projection matrices), the new position q is computed by:
       *
       * q.x = p.w * trans.x + p.x * scale.x
       * q.y = p.w * trans.y + p.y * scale.y
       * q.z = p.w * trans.z + p.z * scale.z;
       * q.w = p.w * trans.w + p.w;
       */
      struct tgsi_full_src_register tmp_pos_src_w =
         scalar_src(&tmp_pos_src, TGSI_SWIZZLE_W);
      struct tgsi_full_dst_register tmp_pos_dst =
         make_dst_temp_reg(vs_pos_tmp_index);
      struct tgsi_full_dst_register tmp_pos_dst_xyz =
         writemask_dst(&tmp_pos_dst, TGSI_WRITEMASK_XYZ);

      struct tgsi_full_src_register prescale_scale =
         make_src_temp_reg(emit->vposition.prescale_scale_index);
      struct tgsi_full_src_register prescale_trans =
         make_src_temp_reg(emit->vposition.prescale_trans_index);

      /* MUL tmp_pos.xyz, tmp_pos, prescale.scale */
      emit_instruction_op2(emit, VGPU10_OPCODE_MUL, &tmp_pos_dst_xyz,
                           &tmp_pos_src, &prescale_scale);

      /* MAD pos, tmp_pos.wwww, prescale.trans, tmp_pos */
      emit_instruction_op3(emit, VGPU10_OPCODE_MAD, &pos_dst, &tmp_pos_src_w,
                           &prescale_trans, &tmp_pos_src);
   }
   else if (emit->key.vs.undo_viewport) {
      /* This code computes the final vertex position from the temporary
       * vertex position by undoing the viewport transformation and the
       * divide-by-W operation (we convert window coords back to clip coords).
       * This is needed when we use the 'draw' module for fallbacks.
       * If p is the temp pos in window coords, then the NDC coord q is:
       *   q.x = (p.x - vp.x_trans) / vp.x_scale * p.w
       *   q.y = (p.y - vp.y_trans) / vp.y_scale * p.w
       *   q.z = p.z * p.w
       *   q.w = p.w
       * CONST[vs_viewport_index] contains:
       *   { 1/vp.x_scale, 1/vp.y_scale, -vp.x_trans, -vp.y_trans }
       */
      struct tgsi_full_dst_register tmp_pos_dst =
         make_dst_temp_reg(vs_pos_tmp_index);
      struct tgsi_full_dst_register tmp_pos_dst_xy =
         writemask_dst(&tmp_pos_dst, TGSI_WRITEMASK_XY);
      struct tgsi_full_src_register tmp_pos_src_wwww =
         scalar_src(&tmp_pos_src, TGSI_SWIZZLE_W);

      struct tgsi_full_dst_register pos_dst_xyz =
         writemask_dst(&pos_dst, TGSI_WRITEMASK_XYZ);
      struct tgsi_full_dst_register pos_dst_w =
         writemask_dst(&pos_dst, TGSI_WRITEMASK_W);

      struct tgsi_full_src_register vp_xyzw =
         make_src_const_reg(emit->vs.viewport_index);
      struct tgsi_full_src_register vp_zwww =
         swizzle_src(&vp_xyzw, TGSI_SWIZZLE_Z, TGSI_SWIZZLE_W,
                     TGSI_SWIZZLE_W, TGSI_SWIZZLE_W);

      /* ADD tmp_pos.xy, tmp_pos.xy, viewport.zwww */
      emit_instruction_op2(emit, VGPU10_OPCODE_ADD, &tmp_pos_dst_xy,
                           &tmp_pos_src, &vp_zwww);

      /* MUL tmp_pos.xy, tmp_pos.xyzw, viewport.xyzy */
      emit_instruction_op2(emit, VGPU10_OPCODE_MUL, &tmp_pos_dst_xy,
                           &tmp_pos_src, &vp_xyzw);

      /* MUL pos.xyz, tmp_pos.xyz, tmp_pos.www */
      emit_instruction_op2(emit, VGPU10_OPCODE_MUL, &pos_dst_xyz,
                           &tmp_pos_src, &tmp_pos_src_wwww);

      /* MOV pos.w, tmp_pos.w */
      emit_instruction_op1(emit, VGPU10_OPCODE_MOV, &pos_dst_w, &tmp_pos_src);
   }
   else if (vs_pos_tmp_index != INVALID_INDEX) {
      /* This code is to handle the case where the temporary vertex
       * position register is created when the vertex shader has stream
       * output and prescale is disabled because rasterization is to be
       * discarded.
       */
      struct tgsi_full_dst_register pos_dst =
         make_dst_output_reg(emit->vposition.out_index);

      /* MOV pos, tmp_pos */
      begin_emit_instruction(emit);
      emit_opcode(emit, VGPU10_OPCODE_MOV, false);
      emit_dst_register(emit, &pos_dst);
      emit_src_register(emit, &tmp_pos_src);
      end_emit_instruction(emit);
   }

   /* Restore original vposition.tmp_index value for the next GS vertex.
    * It doesn't matter for VS.
    */
   emit->vposition.tmp_index = vs_pos_tmp_index;
}

static void
emit_clipping_instructions(struct svga_shader_emitter_v10 *emit)
{
   if (emit->clip_mode == CLIP_DISTANCE) {
      /* Copy from copy distance temporary to CLIPDIST & the shadow copy */
      emit_clip_distance_instructions(emit);

   } else if (emit->clip_mode == CLIP_VERTEX &&
              emit->key.last_vertex_stage) {
      /* Convert TGSI CLIPVERTEX to CLIPDIST */
      emit_clip_vertex_instructions(emit);
   }

   /**
    * Emit vertex position and take care of legacy user planes only if
    * there is a valid vertex position register index.
    * This is to take care of the case
    * where the shader doesn't output vertex position. Then in
    * this case, don't bother to emit more vertex instructions.
    */
   if (emit->vposition.out_index == INVALID_INDEX)
      return;

   /**
    * Emit per-vertex clipping instructions for legacy user defined clip planes.
    * NOTE: we must emit the clip distance instructions before the
    * emit_vpos_instructions() call since the later function will change
    * the TEMP[vs_pos_tmp_index] value.
    */
   if (emit->clip_mode == CLIP_LEGACY && emit->key.last_vertex_stage) {
      /* Emit CLIPDIST for legacy user defined clip planes */
      emit_clip_distance_from_vpos(emit, emit->vposition.tmp_index);
   }
}


/**
 * Emit extra per-vertex instructions.  This includes clip-coordinate
 * space conversion and computing clip distances.  This is called for
 * each GS emit-vertex instruction and at the end of VS translation.
 */
static void
emit_vertex_instructions(struct svga_shader_emitter_v10 *emit)
{
   /* Emit clipping instructions based on clipping mode */
   emit_clipping_instructions(emit);

   /* Emit vertex position instructions */
   emit_vpos_instructions(emit);
}


/**
 * Translate the TGSI_OPCODE_EMIT GS instruction.
 */
static bool
emit_vertex(struct svga_shader_emitter_v10 *emit,
            const struct tgsi_full_instruction *inst)
{
   unsigned ret = true;

   assert(emit->unit == PIPE_SHADER_GEOMETRY);

   /**
    * Emit the viewport array index for the first vertex.
    */
   if (emit->gs.viewport_index_out_index != INVALID_INDEX) {
      struct tgsi_full_dst_register viewport_index_out =
         make_dst_output_reg(emit->gs.viewport_index_out_index);
      struct tgsi_full_dst_register viewport_index_out_x =
         writemask_dst(&viewport_index_out, TGSI_WRITEMASK_X);
      struct tgsi_full_src_register viewport_index_tmp =
         make_src_temp_reg(emit->gs.viewport_index_tmp_index);

      /* Set the out index to INVALID_INDEX, so it will not
       * be assigned to a temp again in emit_dst_register, and
       * the viewport index will not be assigned again in the
       * subsequent vertices.
       */
      emit->gs.viewport_index_out_index = INVALID_INDEX;
      emit_instruction_op1(emit, VGPU10_OPCODE_MOV,
                           &viewport_index_out_x, &viewport_index_tmp);
   }

   /**
    * Find the stream index associated with this emit vertex instruction.
    */
   assert(inst->Src[0].Register.File == TGSI_FILE_IMMEDIATE);
   unsigned streamIndex = find_stream_index(emit, &inst->Src[0]);

   /**
    * According to the ARB_gpu_shader5 spec, the built-in geometry shader
    * outputs are always associated with vertex stream zero.
    * So emit the extra vertex instructions for position or clip distance
    * for stream zero only.
    */
   if (streamIndex == 0) {
      /**
       * Before emitting vertex instructions, emit the temporaries for
       * the prescale constants based on the viewport index if needed.
       */
      if (emit->vposition.need_prescale && !emit->vposition.have_prescale)
         emit_temp_prescale_instructions(emit);

      emit_vertex_instructions(emit);
   }

   begin_emit_instruction(emit);
   if (emit->version >= 50) {
      if (emit->info.num_stream_output_components[streamIndex] == 0) {
         /**
          * If there is no output for this stream, discard this instruction.
          */
         emit->discard_instruction = true;
      }
      else {
         emit_opcode(emit, VGPU10_OPCODE_EMIT_STREAM, false);
         emit_stream_register(emit, streamIndex);
      }
   }
   else {
      emit_opcode(emit, VGPU10_OPCODE_EMIT, false);
   }
   end_emit_instruction(emit);

   return ret;
}


/**
 * Emit the extra code to convert from VGPU10's boolean front-face
 * register to TGSI's signed front-face register.
 *
 * TODO: Make temporary front-face register a scalar.
 */
static void
emit_frontface_instructions(struct svga_shader_emitter_v10 *emit)
{
   assert(emit->unit == PIPE_SHADER_FRAGMENT);

   if (emit->fs.face_input_index != INVALID_INDEX) {
      /* convert vgpu10 boolean face register to gallium +/-1 value */
      struct tgsi_full_dst_register tmp_dst =
         make_dst_temp_reg(emit->fs.face_tmp_index);
      struct tgsi_full_src_register one =
         make_immediate_reg_float(emit, 1.0f);
      struct tgsi_full_src_register neg_one =
         make_immediate_reg_float(emit, -1.0f);

      /* MOVC face_tmp, IS_FRONT_FACE.x, 1.0, -1.0 */
      begin_emit_instruction(emit);
      emit_opcode(emit, VGPU10_OPCODE_MOVC, false);
      emit_dst_register(emit, &tmp_dst);
      emit_face_register(emit);
      emit_src_register(emit, &one);
      emit_src_register(emit, &neg_one);
      end_emit_instruction(emit);
   }
}


/**
 * Emit the extra code to convert from VGPU10's fragcoord.w value to 1/w.
 */
static void
emit_fragcoord_instructions(struct svga_shader_emitter_v10 *emit)
{
   assert(emit->unit == PIPE_SHADER_FRAGMENT);

   if (emit->fs.fragcoord_input_index != INVALID_INDEX) {
      struct tgsi_full_dst_register tmp_dst =
         make_dst_temp_reg(emit->fs.fragcoord_tmp_index);
      struct tgsi_full_dst_register tmp_dst_xyz =
         writemask_dst(&tmp_dst, TGSI_WRITEMASK_XYZ);
      struct tgsi_full_dst_register tmp_dst_w =
         writemask_dst(&tmp_dst, TGSI_WRITEMASK_W);
      struct tgsi_full_src_register one =
         make_immediate_reg_float(emit, 1.0f);
      struct tgsi_full_src_register fragcoord =
         make_src_reg(TGSI_FILE_INPUT, emit->fs.fragcoord_input_index);

      /* save the input index */
      unsigned fragcoord_input_index = emit->fs.fragcoord_input_index;
      /* set to invalid to prevent substitution in emit_src_register() */
      emit->fs.fragcoord_input_index = INVALID_INDEX;

      /* MOV fragcoord_tmp.xyz, fragcoord.xyz */
      begin_emit_instruction(emit);
      emit_opcode(emit, VGPU10_OPCODE_MOV, false);
      emit_dst_register(emit, &tmp_dst_xyz);
      emit_src_register(emit, &fragcoord);
      end_emit_instruction(emit);

      /* DIV fragcoord_tmp.w, 1.0, fragcoord.w */
      begin_emit_instruction(emit);
      emit_opcode(emit, VGPU10_OPCODE_DIV, false);
      emit_dst_register(emit, &tmp_dst_w);
      emit_src_register(emit, &one);
      emit_src_register(emit, &fragcoord);
      end_emit_instruction(emit);

      /* restore saved value */
      emit->fs.fragcoord_input_index = fragcoord_input_index;
   }
}


/**
 * Emit the extra code to get the current sample position value and
 * put it into a temp register.
 */
static void
emit_sample_position_instructions(struct svga_shader_emitter_v10 *emit)
{
   assert(emit->unit == PIPE_SHADER_FRAGMENT);

   if (emit->fs.sample_pos_sys_index != INVALID_INDEX) {
      assert(emit->version >= 41);

      struct tgsi_full_dst_register tmp_dst =
         make_dst_temp_reg(emit->fs.sample_pos_tmp_index);
      struct tgsi_full_src_register half =
         make_immediate_reg_float4(emit, 0.5, 0.5, 0.0, 0.0);

      struct tgsi_full_src_register tmp_src =
         make_src_temp_reg(emit->fs.sample_pos_tmp_index);
      struct tgsi_full_src_register sample_index_reg =
         make_src_scalar_reg(TGSI_FILE_SYSTEM_VALUE,
                             emit->fs.sample_id_sys_index, TGSI_SWIZZLE_X);

      /* The first src register is a shader resource (if we want a
       * multisampled resource sample position) or the rasterizer register
       * (if we want the current sample position in the color buffer).  We
       * want the later.
       */

      /* SAMPLE_POS dst, RASTERIZER, sampleIndex */
      begin_emit_instruction(emit);
      emit_opcode(emit, VGPU10_OPCODE_SAMPLE_POS, false);
      emit_dst_register(emit, &tmp_dst);
      emit_rasterizer_register(emit);
      emit_src_register(emit, &sample_index_reg);
      end_emit_instruction(emit);

      /* Convert from D3D coords to GL coords by adding 0.5 bias */
      /* ADD dst, dst, half */
      begin_emit_instruction(emit);
      emit_opcode(emit, VGPU10_OPCODE_ADD, false);
      emit_dst_register(emit, &tmp_dst);
      emit_src_register(emit, &tmp_src);
      emit_src_register(emit, &half);
      end_emit_instruction(emit);
   }
}


/**
 * Emit extra instructions to adjust VS inputs/attributes.  This can
 * mean casting a vertex attribute from int to float or setting the
 * W component to 1, or both.
 */
static void
emit_vertex_attrib_instructions(struct svga_shader_emitter_v10 *emit)
{
   const unsigned save_w_1_mask = emit->key.vs.adjust_attrib_w_1;
   const unsigned save_itof_mask = emit->key.vs.adjust_attrib_itof;
   const unsigned save_utof_mask = emit->key.vs.adjust_attrib_utof;
   const unsigned save_is_bgra_mask = emit->key.vs.attrib_is_bgra;
   const unsigned save_puint_to_snorm_mask = emit->key.vs.attrib_puint_to_snorm;
   const unsigned save_puint_to_uscaled_mask = emit->key.vs.attrib_puint_to_uscaled;
   const unsigned save_puint_to_sscaled_mask = emit->key.vs.attrib_puint_to_sscaled;

   unsigned adjust_mask = (save_w_1_mask |
                           save_itof_mask |
                           save_utof_mask |
                           save_is_bgra_mask |
                           save_puint_to_snorm_mask |
                           save_puint_to_uscaled_mask |
                           save_puint_to_sscaled_mask);

   assert(emit->unit == PIPE_SHADER_VERTEX);

   if (adjust_mask) {
      struct tgsi_full_src_register one =
         make_immediate_reg_float(emit, 1.0f);

      struct tgsi_full_src_register one_int =
         make_immediate_reg_int(emit, 1);

      /* We need to turn off these bitmasks while emitting the
       * instructions below, then restore them afterward.
       */
      emit->key.vs.adjust_attrib_w_1 = 0;
      emit->key.vs.adjust_attrib_itof = 0;
      emit->key.vs.adjust_attrib_utof = 0;
      emit->key.vs.attrib_is_bgra = 0;
      emit->key.vs.attrib_puint_to_snorm = 0;
      emit->key.vs.attrib_puint_to_uscaled = 0;
      emit->key.vs.attrib_puint_to_sscaled = 0;

      while (adjust_mask) {
         unsigned index = u_bit_scan(&adjust_mask);

         /* skip the instruction if this vertex attribute is not being used */
         if (emit->info.input_usage_mask[index] == 0)
            continue;

         unsigned tmp = emit->vs.adjusted_input[index];
         struct tgsi_full_src_register input_src =
            make_src_reg(TGSI_FILE_INPUT, index);

         struct tgsi_full_dst_register tmp_dst = make_dst_temp_reg(tmp);
         struct tgsi_full_src_register tmp_src = make_src_temp_reg(tmp);
         struct tgsi_full_dst_register tmp_dst_w =
            writemask_dst(&tmp_dst, TGSI_WRITEMASK_W);

         /* ITOF/UTOF/MOV tmp, input[index] */
         if (save_itof_mask & (1 << index)) {
            emit_instruction_op1(emit, VGPU10_OPCODE_ITOF,
                                 &tmp_dst, &input_src);
         }
         else if (save_utof_mask & (1 << index)) {
            emit_instruction_op1(emit, VGPU10_OPCODE_UTOF,
                                 &tmp_dst, &input_src);
         }
         else if (save_puint_to_snorm_mask & (1 << index)) {
            emit_puint_to_snorm(emit, &tmp_dst, &input_src);
         }
         else if (save_puint_to_uscaled_mask & (1 << index)) {
            emit_puint_to_uscaled(emit, &tmp_dst, &input_src);
         }
         else if (save_puint_to_sscaled_mask & (1 << index)) {
            emit_puint_to_sscaled(emit, &tmp_dst, &input_src);
         }
         else {
            assert((save_w_1_mask | save_is_bgra_mask) & (1 << index));
            emit_instruction_op1(emit, VGPU10_OPCODE_MOV,
                                 &tmp_dst, &input_src);
         }

         if (save_is_bgra_mask & (1 << index)) {
            emit_swap_r_b(emit, &tmp_dst, &tmp_src);
         }

         if (save_w_1_mask & (1 << index)) {
            /* MOV tmp.w, 1.0 */
            if (emit->key.vs.attrib_is_pure_int & (1 << index)) {
               emit_instruction_op1(emit, VGPU10_OPCODE_MOV,
                                    &tmp_dst_w, &one_int);
            }
            else {
               emit_instruction_op1(emit, VGPU10_OPCODE_MOV,
                                    &tmp_dst_w, &one);
            }
         }
      }

      emit->key.vs.adjust_attrib_w_1 = save_w_1_mask;
      emit->key.vs.adjust_attrib_itof = save_itof_mask;
      emit->key.vs.adjust_attrib_utof = save_utof_mask;
      emit->key.vs.attrib_is_bgra = save_is_bgra_mask;
      emit->key.vs.attrib_puint_to_snorm = save_puint_to_snorm_mask;
      emit->key.vs.attrib_puint_to_uscaled = save_puint_to_uscaled_mask;
      emit->key.vs.attrib_puint_to_sscaled = save_puint_to_sscaled_mask;
   }
}


/* Find zero-value immedate for default layer index */
static void
emit_default_layer_instructions(struct svga_shader_emitter_v10 *emit)
{
   assert(emit->unit == PIPE_SHADER_FRAGMENT);

   /* immediate for default layer index 0 */
   if (emit->fs.layer_input_index != INVALID_INDEX) {
      union tgsi_immediate_data imm;
      imm.Int = 0;
      emit->fs.layer_imm_index = find_immediate(emit, imm, 0);
   }
}


static void
emit_temp_prescale_from_cbuf(struct svga_shader_emitter_v10 *emit,
                             unsigned cbuf_index,
                             struct tgsi_full_dst_register *scale,
                             struct tgsi_full_dst_register *translate)
{
   struct tgsi_full_src_register scale_cbuf = make_src_const_reg(cbuf_index);
   struct tgsi_full_src_register trans_cbuf = make_src_const_reg(cbuf_index+1);

   emit_instruction_op1(emit, VGPU10_OPCODE_MOV, scale, &scale_cbuf);
   emit_instruction_op1(emit, VGPU10_OPCODE_MOV, translate, &trans_cbuf);
}


/**
 * A recursive helper function to find the prescale from the constant buffer
 */
static void
find_prescale_from_cbuf(struct svga_shader_emitter_v10 *emit,
                        unsigned index, unsigned num_prescale,
                        struct tgsi_full_src_register *vp_index,
                        struct tgsi_full_dst_register *scale,
                        struct tgsi_full_dst_register *translate,
                        struct tgsi_full_src_register *tmp_src,
                        struct tgsi_full_dst_register *tmp_dst)
{
   if (num_prescale == 0)
      return;

   if (index > 0) {
      /* ELSE */
      emit_instruction_op0(emit, VGPU10_OPCODE_ELSE);
   }

   struct tgsi_full_src_register index_src =
	                            make_immediate_reg_int(emit, index);

   if (index == 0) {
      /* GE tmp, vp_index, index */
      emit_instruction_op2(emit, VGPU10_OPCODE_GE, tmp_dst,
                           vp_index, &index_src);
   } else {
      /* EQ tmp, vp_index, index */
      emit_instruction_op2(emit, VGPU10_OPCODE_EQ, tmp_dst,
                           vp_index, &index_src);
   }

   /* IF tmp */
   emit_if(emit, tmp_src);
   emit_temp_prescale_from_cbuf(emit,
                                emit->vposition.prescale_cbuf_index + 2 * index,
                                scale, translate);

   find_prescale_from_cbuf(emit, index+1, num_prescale-1,
                           vp_index, scale, translate,
                           tmp_src, tmp_dst);

   /* ENDIF */
   emit_instruction_op0(emit, VGPU10_OPCODE_ENDIF);
}


/**
 * This helper function emits instructions to set the prescale
 * and translate temporaries to the correct constants from the
 * constant buffer according to the designated viewport.
 */
static void
emit_temp_prescale_instructions(struct svga_shader_emitter_v10 *emit)
{
   struct tgsi_full_dst_register prescale_scale =
         make_dst_temp_reg(emit->vposition.prescale_scale_index);
   struct tgsi_full_dst_register prescale_translate =
         make_dst_temp_reg(emit->vposition.prescale_trans_index);

   unsigned prescale_cbuf_index = emit->vposition.prescale_cbuf_index;

   if (emit->vposition.num_prescale == 1) {
      emit_temp_prescale_from_cbuf(emit,
                                   prescale_cbuf_index,
                                   &prescale_scale, &prescale_translate);
   } else {
      /**
       * Since SM5 device does not support dynamic indexing, we need
       * to do the if-else to find the prescale constants for the
       * specified viewport.
       */
      struct tgsi_full_src_register vp_index_src =
         make_src_temp_reg(emit->gs.viewport_index_tmp_index);

      struct tgsi_full_src_register vp_index_src_x =
         scalar_src(&vp_index_src, TGSI_SWIZZLE_X);

      unsigned tmp = get_temp_index(emit);
      struct tgsi_full_src_register tmp_src = make_src_temp_reg(tmp);
      struct tgsi_full_src_register tmp_src_x =
                scalar_src(&tmp_src, TGSI_SWIZZLE_X);
      struct tgsi_full_dst_register tmp_dst = make_dst_temp_reg(tmp);

      find_prescale_from_cbuf(emit, 0, emit->vposition.num_prescale,
                              &vp_index_src_x,
		              &prescale_scale, &prescale_translate,
                              &tmp_src_x, &tmp_dst);
   }

   /* Mark prescale temporaries are emitted */
   emit->vposition.have_prescale = 1;
}


/**
 * A helper function to emit an instruction in a vertex shader to add a bias
 * to the VertexID system value. This patches the VertexID in the SVGA vertex
 * shader to include the base vertex of an indexed primitive or the start index
 * of a non-indexed primitive.
 */
static void
emit_vertex_id_nobase_instruction(struct svga_shader_emitter_v10 *emit)
{
   struct tgsi_full_src_register vertex_id_bias_index =
      make_src_const_reg(emit->vs.vertex_id_bias_index);
   struct tgsi_full_src_register vertex_id_sys_src =
      make_src_reg(TGSI_FILE_SYSTEM_VALUE, emit->vs.vertex_id_sys_index);
   struct tgsi_full_src_register vertex_id_sys_src_x =
      scalar_src(&vertex_id_sys_src, TGSI_SWIZZLE_X);
   struct tgsi_full_dst_register vertex_id_tmp_dst =
      make_dst_temp_reg(emit->vs.vertex_id_tmp_index);

   /* IADD vertex_id_tmp, vertex_id_sys, vertex_id_bias */
   unsigned vertex_id_tmp_index = emit->vs.vertex_id_tmp_index;
   emit->vs.vertex_id_tmp_index = INVALID_INDEX;
   emit_instruction_opn(emit, VGPU10_OPCODE_IADD, &vertex_id_tmp_dst,
                        &vertex_id_sys_src_x, &vertex_id_bias_index, NULL, false,
                        false);
   emit->vs.vertex_id_tmp_index = vertex_id_tmp_index;
}

/**
 * Hull Shader must have control point outputs. But tessellation
 * control shader can return without writing to control point output.
 * In this case, the control point output is assumed to be passthrough
 * from the control point input.
 * This helper function is to write out a control point output first in case
 * the tessellation control shader returns before writing a
 * control point output.
 */
static void
emit_tcs_default_control_point_output(struct svga_shader_emitter_v10 *emit)
{
   assert(emit->unit == PIPE_SHADER_TESS_CTRL);
   assert(emit->tcs.control_point_phase);
   assert(emit->tcs.control_point_out_index != INVALID_INDEX);
   assert(emit->tcs.invocation_id_sys_index != INVALID_INDEX);

   struct tgsi_full_dst_register output_control_point;
   output_control_point =
      make_dst_output_reg(emit->tcs.control_point_out_index);

   if (emit->tcs.control_point_input_index == INVALID_INDEX) {
      /* MOV OUTPUT 0.0f */
      struct tgsi_full_src_register zero = make_immediate_reg_float(emit, 0.0f);
      begin_emit_instruction(emit);
      emit_opcode_precise(emit, VGPU10_OPCODE_MOV, false, false);
      emit_dst_register(emit, &output_control_point);
      emit_src_register(emit, &zero);
      end_emit_instruction(emit);
   }
   else {
      /* UARL ADDR[INDEX].x INVOCATION.xxxx */

      struct tgsi_full_src_register invocation_src;
      struct tgsi_full_dst_register addr_dst;
      struct tgsi_full_dst_register addr_dst_x;
      unsigned addr_tmp;

      addr_tmp = emit->address_reg_index[emit->tcs.control_point_addr_index];
      addr_dst = make_dst_temp_reg(addr_tmp);
      addr_dst_x = writemask_dst(&addr_dst, TGSI_WRITEMASK_X);

      invocation_src = make_src_reg(TGSI_FILE_SYSTEM_VALUE,
                                    emit->tcs.invocation_id_sys_index);

      begin_emit_instruction(emit);
      emit_opcode_precise(emit, VGPU10_OPCODE_MOV, false, false);
      emit_dst_register(emit, &addr_dst_x);
      emit_src_register(emit, &invocation_src);
      end_emit_instruction(emit);


      /* MOV OUTPUT INPUT[ADDR[INDEX].x][POSITION] */

      struct tgsi_full_src_register input_control_point;
      input_control_point = make_src_reg(TGSI_FILE_INPUT,
                                         emit->tcs.control_point_input_index);
      input_control_point.Register.Dimension = 1;
      input_control_point.Dimension.Indirect = 1;
      input_control_point.DimIndirect.File = TGSI_FILE_ADDRESS;
      input_control_point.DimIndirect.Index =
         emit->tcs.control_point_addr_index;

      begin_emit_instruction(emit);
      emit_opcode_precise(emit, VGPU10_OPCODE_MOV, false, false);
      emit_dst_register(emit, &output_control_point);
      emit_src_register(emit, &input_control_point);
      end_emit_instruction(emit);
   }
}

/**
 * This functions constructs temporary tessfactor from VGPU10*_TESSFACTOR
 * values in domain shader. SM5 has tessfactors as floating point values where
 * as tgsi emit them as vector. This function allows to construct temp
 * tessfactor vector similar to TGSI_SEMANTIC_TESSINNER/OUTER filled with
 * values from VGPU10*_TESSFACTOR. Use this constructed vector whenever
 * TGSI_SEMANTIC_TESSINNER/OUTER is used in shader.
 */
static void
emit_temp_tessfactor_instructions(struct svga_shader_emitter_v10 *emit)
{
   struct tgsi_full_src_register src;
   struct tgsi_full_dst_register dst;

   if (emit->tes.inner.tgsi_index != INVALID_INDEX) {
      dst = make_dst_temp_reg(emit->tes.inner.temp_index);

      switch (emit->tes.prim_mode) {
      case MESA_PRIM_QUADS:
         src = make_src_scalar_reg(TGSI_FILE_INPUT,
                  emit->tes.inner.in_index + 1, TGSI_SWIZZLE_X);
         dst = writemask_dst(&dst, TGSI_WRITEMASK_Y);
         emit_instruction_op1(emit, VGPU10_OPCODE_MOV, &dst, &src);
         FALLTHROUGH;
      case MESA_PRIM_TRIANGLES:
         src = make_src_scalar_reg(TGSI_FILE_INPUT,
                  emit->tes.inner.in_index, TGSI_SWIZZLE_X);
         dst = writemask_dst(&dst, TGSI_WRITEMASK_X);
         emit_instruction_op1(emit, VGPU10_OPCODE_MOV, &dst, &src);
         break;
      case MESA_PRIM_LINES:
         /**
          * As per SM5 spec, InsideTessFactor for isolines are unused.
          * In fact glsl tessInnerLevel for isolines doesn't mean anything but if
          * any application try to read tessInnerLevel in TES when primitive type
          * is isolines, then instead of driver throwing segfault for accesing it,
          * return atleast vec(1.0f)
          */
         src = make_immediate_reg_float(emit, 1.0f);
         emit_instruction_op1(emit, VGPU10_OPCODE_MOV, &dst, &src);
         break;
      default:
         break;
      }
   }

   if (emit->tes.outer.tgsi_index != INVALID_INDEX) {
      dst = make_dst_temp_reg(emit->tes.outer.temp_index);

      switch (emit->tes.prim_mode) {
      case MESA_PRIM_QUADS:
         src = make_src_scalar_reg(TGSI_FILE_INPUT,
                  emit->tes.outer.in_index + 3, TGSI_SWIZZLE_X);
         dst = writemask_dst(&dst, TGSI_WRITEMASK_W);
         emit_instruction_op1(emit, VGPU10_OPCODE_MOV, &dst, &src);
         FALLTHROUGH;
      case MESA_PRIM_TRIANGLES:
         src = make_src_scalar_reg(TGSI_FILE_INPUT,
                  emit->tes.outer.in_index + 2, TGSI_SWIZZLE_X);
         dst = writemask_dst(&dst, TGSI_WRITEMASK_Z);
         emit_instruction_op1(emit, VGPU10_OPCODE_MOV, &dst, &src);
         FALLTHROUGH;
      case MESA_PRIM_LINES:
         src = make_src_scalar_reg(TGSI_FILE_INPUT,
                  emit->tes.outer.in_index + 1, TGSI_SWIZZLE_X);
         dst = writemask_dst(&dst, TGSI_WRITEMASK_Y);
         emit_instruction_op1(emit, VGPU10_OPCODE_MOV, &dst, &src);

         src = make_src_scalar_reg(TGSI_FILE_INPUT,
                  emit->tes.outer.in_index , TGSI_SWIZZLE_X);
         dst = writemask_dst(&dst, TGSI_WRITEMASK_X);
         emit_instruction_op1(emit, VGPU10_OPCODE_MOV, &dst, &src);

         break;
      default:
         break;
      }
   }
}


static void
emit_initialize_temp_instruction(struct svga_shader_emitter_v10 *emit)
{
   struct tgsi_full_src_register src;
   struct tgsi_full_dst_register dst;
   unsigned vgpu10_temp_index = remap_temp_index(emit, TGSI_FILE_TEMPORARY,
                                                 emit->initialize_temp_index);
   src = make_immediate_reg_float(emit, 0.0f);
   dst = make_dst_temp_reg(vgpu10_temp_index);
   emit_instruction_op1(emit, VGPU10_OPCODE_MOV, &dst, &src);
   emit->temp_map[emit->initialize_temp_index].initialized = true;
   emit->initialize_temp_index = INVALID_INDEX;
}


/**
 * Emit any extra/helper declarations/code that we might need between
 * the declaration section and code section.
 */
static bool
emit_pre_helpers(struct svga_shader_emitter_v10 *emit)
{
   /* Properties */
   if (emit->unit == PIPE_SHADER_GEOMETRY)
      emit_property_instructions(emit);
   else if (emit->unit == PIPE_SHADER_TESS_CTRL) {
      emit_hull_shader_declarations(emit);

      /* Save the position of the first instruction token so that we can
       * do a second pass of the instructions for the patch constant phase.
       */
      emit->tcs.instruction_token_pos = emit->cur_tgsi_token;
      emit->tcs.fork_phase_add_signature = false;

      if (!emit_hull_shader_control_point_phase(emit)) {
         emit->skip_instruction = true;
         return true;
      }

      /* Set the current tcs phase to control point phase */
      emit->tcs.control_point_phase = true;
   }
   else if (emit->unit == PIPE_SHADER_TESS_EVAL) {
      emit_domain_shader_declarations(emit);
   }
   else if (emit->unit == PIPE_SHADER_COMPUTE) {
      emit_compute_shader_declarations(emit);
   }

   /* Declare inputs */
   if (!emit_input_declarations(emit))
      return false;

   /* Declare outputs */
   if (!emit_output_declarations(emit))
      return false;

   /* Declare temporary registers */
   emit_temporaries_declaration(emit);

   /* For PIPE_SHADER_TESS_CTRL, constants, samplers, resources and immediates
    * will already be declared in hs_decls (emit_hull_shader_declarations)
    */
   if (emit->unit != PIPE_SHADER_TESS_CTRL) {

      alloc_common_immediates(emit);

      /* Declare constant registers */
      emit_constant_declaration(emit);

      /* Declare samplers and resources */
      emit_sampler_declarations(emit);
      emit_resource_declarations(emit);

      /* Declare images */
      emit_image_declarations(emit);

      /* Declare shader buffers */
      emit_shader_buf_declarations(emit);

      /* Declare atomic buffers */
      emit_atomic_buf_declarations(emit);
   }

   if (emit->unit != PIPE_SHADER_FRAGMENT &&
       emit->unit != PIPE_SHADER_COMPUTE) {
      /*
       * Declare clip distance output registers for ClipVertex or
       * user defined planes
       */
      emit_clip_distance_declarations(emit);
   }

   if (emit->unit == PIPE_SHADER_COMPUTE) {
      emit_memory_declarations(emit);

      if (emit->cs.grid_size.tgsi_index != INVALID_INDEX) {
         emit->cs.grid_size.imm_index =
            alloc_immediate_int4(emit,
                                 emit->key.cs.grid_size[0],
                                 emit->key.cs.grid_size[1],
                                 emit->key.cs.grid_size[2], 0);
      }
   }

   if (emit->unit == PIPE_SHADER_FRAGMENT &&
       emit->key.fs.alpha_func != SVGA3D_CMP_ALWAYS) {
      float alpha = emit->key.fs.alpha_ref;
      emit->fs.alpha_ref_index =
         alloc_immediate_float4(emit, alpha, alpha, alpha, alpha);
   }

   if (emit->unit != PIPE_SHADER_TESS_CTRL) {
      /**
       * For PIPE_SHADER_TESS_CTRL, immediates are already declared in
       * hs_decls
       */
      emit_vgpu10_immediates_block(emit);
   }
   else {
      emit_tcs_default_control_point_output(emit);
   }

   if (emit->unit == PIPE_SHADER_FRAGMENT) {
      emit_frontface_instructions(emit);
      emit_fragcoord_instructions(emit);
      emit_sample_position_instructions(emit);
      emit_default_layer_instructions(emit);
   }
   else if (emit->unit == PIPE_SHADER_VERTEX) {
      emit_vertex_attrib_instructions(emit);

      if (emit->info.uses_vertexid)
         emit_vertex_id_nobase_instruction(emit);
   }
   else if (emit->unit == PIPE_SHADER_TESS_EVAL) {
      emit_temp_tessfactor_instructions(emit);
   }

   /**
    * For geometry shader that writes to viewport index, the prescale
    * temporaries will be done at the first vertex emission.
    */
   if (emit->vposition.need_prescale && emit->vposition.num_prescale == 1)
      emit_temp_prescale_instructions(emit);

   return true;
}


/**
 * The device has no direct support for the pipe_blend_state::alpha_to_one
 * option so we implement it here with shader code.
 *
 * Note that this is kind of pointless, actually.  Here we're clobbering
 * the alpha value with 1.0.  So if alpha-to-coverage is enabled, we'll wind
 * up with 100% coverage.  That's almost certainly not what the user wants.
 * The work-around is to add extra shader code to compute coverage from alpha
 * and write it to the coverage output register (if the user's shader doesn't
 * do so already).  We'll probably do that in the future.
 */
static void
emit_alpha_to_one_instructions(struct svga_shader_emitter_v10 *emit,
                               unsigned fs_color_tmp_index)
{
   struct tgsi_full_src_register one = make_immediate_reg_float(emit, 1.0f);
   unsigned i;

   /* Note: it's not 100% clear from the spec if we're supposed to clobber
    * the alpha for all render targets.  But that's what NVIDIA does and
    * that's what Piglit tests.
    */
   for (i = 0; i < emit->fs.num_color_outputs; i++) {
      struct tgsi_full_dst_register color_dst;

      if (fs_color_tmp_index != INVALID_INDEX && i == 0) {
         /* write to the temp color register */
         color_dst = make_dst_temp_reg(fs_color_tmp_index);
      }
      else {
         /* write directly to the color[i] output */
         color_dst = make_dst_output_reg(emit->fs.color_out_index[i]);
      }

      color_dst = writemask_dst(&color_dst, TGSI_WRITEMASK_W);

      emit_instruction_op1(emit, VGPU10_OPCODE_MOV, &color_dst, &one);
   }
}


/**
 * Emit alpha test code.  This compares TEMP[fs_color_tmp_index].w
 * against the alpha reference value and discards the fragment if the
 * comparison fails.
 */
static void
emit_alpha_test_instructions(struct svga_shader_emitter_v10 *emit,
                             unsigned fs_color_tmp_index)
{
   /* compare output color's alpha to alpha ref and discard if comparison
    * fails.
    */
   unsigned tmp = get_temp_index(emit);
   struct tgsi_full_src_register tmp_src = make_src_temp_reg(tmp);
   struct tgsi_full_src_register tmp_src_x =
      scalar_src(&tmp_src, TGSI_SWIZZLE_X);
   struct tgsi_full_dst_register tmp_dst = make_dst_temp_reg(tmp);
   struct tgsi_full_src_register color_src =
      make_src_temp_reg(fs_color_tmp_index);
   struct tgsi_full_src_register color_src_w =
      scalar_src(&color_src, TGSI_SWIZZLE_W);
   struct tgsi_full_src_register ref_src =
      make_src_immediate_reg(emit->fs.alpha_ref_index);
   struct tgsi_full_dst_register color_dst =
      make_dst_output_reg(emit->fs.color_out_index[0]);

   assert(emit->unit == PIPE_SHADER_FRAGMENT);

   /* dst = src0 'alpha_func' src1 */
   emit_comparison(emit, emit->key.fs.alpha_func, &tmp_dst,
                   &color_src_w, &ref_src);

   /* DISCARD if dst.x == 0 */
   begin_emit_instruction(emit);
   emit_discard_opcode(emit, false);  /* discard if src0.x is zero */
   emit_src_register(emit, &tmp_src_x);
   end_emit_instruction(emit);

   /* If we don't need to broadcast the color below, emit the final color here.
    */
   if (emit->key.fs.write_color0_to_n_cbufs <= 1) {
      /* MOV output.color, tempcolor */
      emit_instruction_op1(emit, VGPU10_OPCODE_MOV, &color_dst, &color_src);
   }

   free_temp_indexes(emit);
}


/**
 * Emit instructions for writing a single color output to multiple
 * color buffers.
 * This is used when the TGSI_PROPERTY_FS_COLOR0_WRITES_ALL_CBUFS (or
 * when key.fs.white_fragments is true).
 * property is set and the number of render targets is greater than one.
 * \param fs_color_tmp_index  index of the temp register that holds the
 *                            color to broadcast.
 */
static void
emit_broadcast_color_instructions(struct svga_shader_emitter_v10 *emit,
                                 unsigned fs_color_tmp_index)
{
   const unsigned n = emit->key.fs.write_color0_to_n_cbufs;
   unsigned i;
   struct tgsi_full_src_register color_src;

   if (emit->key.fs.white_fragments) {
      /* set all color outputs to white */
      color_src = make_immediate_reg_float(emit, 1.0f);
   }
   else {
      /* set all color outputs to TEMP[fs_color_tmp_index] */
      assert(fs_color_tmp_index != INVALID_INDEX);
      color_src = make_src_temp_reg(fs_color_tmp_index);
   }

   assert(emit->unit == PIPE_SHADER_FRAGMENT);

   for (i = 0; i < n; i++) {
      unsigned output_reg = emit->fs.color_out_index[i];
      struct tgsi_full_dst_register color_dst =
         make_dst_output_reg(output_reg);

      /* Fill in this semantic here since we'll use it later in
       * emit_dst_register().
       */
      emit->info.output_semantic_name[output_reg] = TGSI_SEMANTIC_COLOR;

      /* MOV output.color[i], tempcolor */
      emit_instruction_op1(emit, VGPU10_OPCODE_MOV, &color_dst, &color_src);
   }
}


/**
 * Emit extra helper code after the original shader code, but before the
 * last END/RET instruction.
 * For vertex shaders this means emitting the extra code to apply the
 * prescale scale/translation.
 */
static bool
emit_post_helpers(struct svga_shader_emitter_v10 *emit)
{
   if (emit->unit == PIPE_SHADER_VERTEX) {
      emit_vertex_instructions(emit);
   }
   else if (emit->unit == PIPE_SHADER_FRAGMENT) {
      const unsigned fs_color_tmp_index = emit->fs.color_tmp_index;

      assert(!(emit->key.fs.white_fragments &&
               emit->key.fs.write_color0_to_n_cbufs == 0));

      /* We no longer want emit_dst_register() to substitute the
       * temporary fragment color register for the real color output.
       */
      emit->fs.color_tmp_index = INVALID_INDEX;

      if (emit->key.fs.alpha_to_one) {
         emit_alpha_to_one_instructions(emit, fs_color_tmp_index);
      }
      if (emit->key.fs.alpha_func != SVGA3D_CMP_ALWAYS) {
         emit_alpha_test_instructions(emit, fs_color_tmp_index);
      }
      if (emit->key.fs.write_color0_to_n_cbufs > 1 ||
          emit->key.fs.white_fragments) {
         emit_broadcast_color_instructions(emit, fs_color_tmp_index);
      }
   }
   else if (emit->unit == PIPE_SHADER_TESS_CTRL) {
      if (!emit->tcs.control_point_phase) {
         /* store the tessellation levels in the patch constant phase only */
         store_tesslevels(emit);
      }
      else {
         emit_clipping_instructions(emit);
      }
   }
   else if (emit->unit == PIPE_SHADER_TESS_EVAL) {
      emit_vertex_instructions(emit);
   }

   return true;
}


/**
 * Reemit rawbuf instruction
 */
static bool
emit_rawbuf_instruction(struct svga_shader_emitter_v10 *emit,
                        unsigned inst_number,
                        const struct tgsi_full_instruction *inst)
{
   bool ret;

   /* For all the rawbuf references in this instruction,
    * load the rawbuf reference and assign to the designated temporary.
    * Then reeemit the instruction.
    */
   emit->reemit_rawbuf_instruction = REEMIT_IN_PROGRESS;

   unsigned offset_tmp = get_temp_index(emit);
   struct tgsi_full_dst_register offset_dst = make_dst_temp_reg(offset_tmp);
   struct tgsi_full_src_register offset_src = make_src_temp_reg(offset_tmp);
   struct tgsi_full_src_register four = make_immediate_reg_int(emit, 4);

   for (unsigned i = 0; i < emit->raw_buf_cur_tmp_index; i++) {
      struct tgsi_full_src_register element_src;

      /* First get the element index register. */

      if (emit->raw_buf_tmp[i].indirect) {
         unsigned tmp = get_temp_index(emit);
         struct tgsi_full_dst_register element_dst = make_dst_temp_reg(tmp);
         struct tgsi_full_src_register element_index =
            make_src_temp_reg(emit->raw_buf_tmp[i].element_index);
         struct tgsi_full_src_register element_rel =
            make_immediate_reg_int(emit, emit->raw_buf_tmp[i].element_rel);

         element_src = make_src_temp_reg(tmp);
         element_src = scalar_src(&element_src, TGSI_SWIZZLE_X);
         element_dst = writemask_dst(&element_dst, TGSI_WRITEMASK_X);

         /* element index from the indirect register */
         element_index = make_src_temp_reg(emit->raw_buf_tmp[i].element_index);
         element_index = scalar_src(&element_index, TGSI_SWIZZLE_X);

         /* IADD element_src element_index element_index_relative */
         emit_instruction_op2(emit, VGPU10_OPCODE_IADD, &element_dst,
                              &element_index, &element_rel);
      }
      else {
         unsigned element_index = emit->raw_buf_tmp[i].element_index;
         union tgsi_immediate_data imm;
         imm.Int = element_index;
         int immpos = find_immediate(emit, imm, 0);
         if (immpos < 0) {
            UNUSED unsigned element_index_imm =
		                add_immediate_int(emit, element_index);
         }
         element_src = make_immediate_reg_int(emit, element_index);
      }

      /* byte offset = element index << 4 */
      emit_instruction_op2(emit, VGPU10_OPCODE_ISHL, &offset_dst,
                           &element_src, &four);

      struct tgsi_full_dst_register dst_tmp =
         make_dst_temp_reg(i + emit->raw_buf_tmp_index);

      /* LD_RAW tmp, rawbuf byte offset, rawbuf */

      begin_emit_instruction(emit);
      emit_opcode(emit, VGPU10_OPCODE_LD_RAW, false);
      emit_dst_register(emit, &dst_tmp);

      struct tgsi_full_src_register offset_x =
            scalar_src(&offset_src, TGSI_SWIZZLE_X);
      emit_src_register(emit, &offset_x);

      emit_resource_register(emit,
         emit->raw_buf_tmp[i].buffer_index + emit->raw_buf_srv_start_index);
      end_emit_instruction(emit);
   }

   emit->raw_buf_cur_tmp_index = 0;

   ret = emit_vgpu10_instruction(emit, inst_number, inst);

   /* reset raw buf state */
   emit->raw_buf_cur_tmp_index = 0;
   emit->reemit_rawbuf_instruction = REEMIT_FALSE;

   free_temp_indexes(emit);

   return ret;
}


/**
 * Translate the TGSI tokens into VGPU10 tokens.
 */
static bool
emit_vgpu10_instructions(struct svga_shader_emitter_v10 *emit,
                         const struct tgsi_token *tokens)
{
   struct tgsi_parse_context parse;
   bool ret = true;
   bool pre_helpers_emitted = false;
   unsigned inst_number = 0;

   tgsi_parse_init(&parse, tokens);

   while (!tgsi_parse_end_of_tokens(&parse)) {

      /* Save the current tgsi token starting position */
      emit->cur_tgsi_token = parse.Position;

      tgsi_parse_token(&parse);

      switch (parse.FullToken.Token.Type) {
      case TGSI_TOKEN_TYPE_IMMEDIATE:
         ret = emit_vgpu10_immediate(emit, &parse.FullToken.FullImmediate);
         if (!ret)
            goto done;
         break;

      case TGSI_TOKEN_TYPE_DECLARATION:
         ret = emit_vgpu10_declaration(emit, &parse.FullToken.FullDeclaration);
         if (!ret)
            goto done;
         break;

      case TGSI_TOKEN_TYPE_INSTRUCTION:
         if (!pre_helpers_emitted) {
            ret = emit_pre_helpers(emit);
            if (!ret)
               goto done;
            pre_helpers_emitted = true;
         }
         ret = emit_vgpu10_instruction(emit, inst_number++,
                                       &parse.FullToken.FullInstruction);

         /* Usually this applies to TCS only. If shader is reading control
          * point outputs in control point phase, we should reemit all
          * instructions which are writting into control point output in
          * control phase to store results into temporaries.
          */
         if (emit->reemit_instruction) {
            assert(emit->unit == PIPE_SHADER_TESS_CTRL);
            ret = emit_vgpu10_instruction(emit, inst_number,
                                          &parse.FullToken.FullInstruction);
         }
         else if (emit->initialize_temp_index != INVALID_INDEX) {
            emit_initialize_temp_instruction(emit);
            emit->initialize_temp_index = INVALID_INDEX;
            ret = emit_vgpu10_instruction(emit, inst_number - 1,
                                          &parse.FullToken.FullInstruction);
         }
         else if (emit->reemit_rawbuf_instruction) {
            ret = emit_rawbuf_instruction(emit, inst_number - 1,
                                          &parse.FullToken.FullInstruction);
         }

         if (!ret)
            goto done;
         break;

      case TGSI_TOKEN_TYPE_PROPERTY:
         ret = emit_vgpu10_property(emit, &parse.FullToken.FullProperty);
         if (!ret)
            goto done;
         break;

      default:
         break;
      }
   }

   if (emit->unit == PIPE_SHADER_TESS_CTRL) {
      ret = emit_hull_shader_patch_constant_phase(emit, &parse);
   }

done:
   tgsi_parse_free(&parse);
   return ret;
}


/**
 * Emit the first VGPU10 shader tokens.
 */
static bool
emit_vgpu10_header(struct svga_shader_emitter_v10 *emit)
{
   VGPU10ProgramToken ptoken;

   /* First token: VGPU10ProgramToken  (version info, program type (VS,GS,PS)) */

   /* Maximum supported shader version is 50 */
   unsigned version = MIN2(emit->version, 50);

   ptoken.value = 0; /* init whole token to zero */
   ptoken.majorVersion = version / 10;
   ptoken.minorVersion = version % 10;
   ptoken.programType = translate_shader_type(emit->unit);
   if (!emit_dword(emit, ptoken.value))
      return false;

   /* Second token: total length of shader, in tokens.  We can't fill this
    * in until we're all done.  Emit zero for now.
    */
   if (!emit_dword(emit, 0))
      return false;

   if (emit->version >= 50) {
      VGPU10OpcodeToken0 token;

      if (emit->unit == PIPE_SHADER_TESS_CTRL) {
         /* For hull shader, we need to start the declarations phase first before
          * emitting any declarations including the global flags.
          */
         token.value = 0;
         token.opcodeType = VGPU10_OPCODE_HS_DECLS;
         begin_emit_instruction(emit);
         emit_dword(emit, token.value);
         end_emit_instruction(emit);
      }

      /* Emit global flags */
      token.value = 0;    /* init whole token to zero */
      token.opcodeType = VGPU10_OPCODE_DCL_GLOBAL_FLAGS;
      token.enableDoublePrecisionFloatOps = 1;  /* set bit */
      token.instructionLength = 1;
      if (!emit_dword(emit, token.value))
         return false;
   }

   if (emit->version >= 40) {
      VGPU10OpcodeToken0 token;

      /* Reserved for global flag such as refactoringAllowed.
       * If the shader does not use the precise qualifier, we will set the
       * refactoringAllowed global flag; otherwise, we will leave the reserved
       * token to NOP.
       */
      emit->reserved_token = (emit->ptr - emit->buf) / sizeof(VGPU10OpcodeToken0);
      token.value = 0;
      token.opcodeType = VGPU10_OPCODE_NOP;
      token.instructionLength = 1;
      if (!emit_dword(emit, token.value))
         return false;
   }

   return true;
}


static bool
emit_vgpu10_tail(struct svga_shader_emitter_v10 *emit)
{
   VGPU10ProgramToken *tokens;

   /* Replace the second token with total shader length */
   tokens = (VGPU10ProgramToken *) emit->buf;
   tokens[1].value = emit_get_num_tokens(emit);

   if (emit->version >= 40 && !emit->uses_precise_qualifier) {
      /* Replace the reserved token with the RefactoringAllowed global flag */
      VGPU10OpcodeToken0 *ptoken;

      ptoken = (VGPU10OpcodeToken0 *)&tokens[emit->reserved_token];
      assert(ptoken->opcodeType == VGPU10_OPCODE_NOP);
      ptoken->opcodeType = VGPU10_OPCODE_DCL_GLOBAL_FLAGS;
      ptoken->refactoringAllowed = 1;
   }

   if (emit->version >= 50 && emit->fs.forceEarlyDepthStencil) {
      /* Replace the reserved token with the forceEarlyDepthStencil  global flag */
      VGPU10OpcodeToken0 *ptoken;

      ptoken = (VGPU10OpcodeToken0 *)&tokens[emit->reserved_token];
      ptoken->opcodeType = VGPU10_OPCODE_DCL_GLOBAL_FLAGS;
      ptoken->forceEarlyDepthStencil = 1;
   }

   return true;
}


/**
 * Modify the FS to read the BCOLORs and use the FACE register
 * to choose between the front/back colors.
 */
static const struct tgsi_token *
transform_fs_twoside(const struct tgsi_token *tokens)
{
   if (0) {
      debug_printf("Before tgsi_add_two_side ------------------\n");
      tgsi_dump(tokens,0);
   }
   tokens = tgsi_add_two_side(tokens);
   if (0) {
      debug_printf("After tgsi_add_two_side ------------------\n");
      tgsi_dump(tokens, 0);
   }
   return tokens;
}


/**
 * Modify the FS to do polygon stipple.
 */
static const struct tgsi_token *
transform_fs_pstipple(struct svga_shader_emitter_v10 *emit,
                      const struct tgsi_token *tokens)
{
   const struct tgsi_token *new_tokens;
   unsigned unit;

   if (0) {
      debug_printf("Before pstipple ------------------\n");
      tgsi_dump(tokens,0);
   }

   new_tokens = util_pstipple_create_fragment_shader(tokens, &unit, 0,
                                                     TGSI_FILE_INPUT);

   emit->fs.pstipple_sampler_unit = unit;

   /* The new sampler state is appended to the end of the samplers list */
   emit->fs.pstipple_sampler_state_index = emit->key.num_samplers++;

   /* Setup texture state for stipple */
   emit->sampler_target[unit] = TGSI_TEXTURE_2D;
   emit->key.tex[unit].swizzle_r = TGSI_SWIZZLE_X;
   emit->key.tex[unit].swizzle_g = TGSI_SWIZZLE_Y;
   emit->key.tex[unit].swizzle_b = TGSI_SWIZZLE_Z;
   emit->key.tex[unit].swizzle_a = TGSI_SWIZZLE_W;
   emit->key.tex[unit].target = PIPE_TEXTURE_2D;
   emit->key.tex[unit].sampler_index = emit->fs.pstipple_sampler_state_index;

   if (0) {
      debug_printf("After pstipple ------------------\n");
      tgsi_dump(new_tokens, 0);
   }

   return new_tokens;
}

/**
 * Modify the FS to support anti-aliasing point.
 */
static const struct tgsi_token *
transform_fs_aapoint(struct svga_context *svga,
		     const struct tgsi_token *tokens,
                     int aa_coord_index)
{
   bool need_texcoord_semantic =
      svga->pipe.screen->get_param(svga->pipe.screen, PIPE_CAP_TGSI_TEXCOORD);

   if (0) {
      debug_printf("Before tgsi_add_aa_point ------------------\n");
      tgsi_dump(tokens,0);
   }
   tokens = tgsi_add_aa_point(tokens, aa_coord_index, need_texcoord_semantic);
   if (0) {
      debug_printf("After tgsi_add_aa_point ------------------\n");
      tgsi_dump(tokens, 0);
   }
   return tokens;
}


/**
 * A helper function to determine the shader in the previous stage and
 * then call the linker function to determine the input mapping for this
 * shader to match the output indices from the shader in the previous stage.
 */
static void
compute_input_mapping(struct svga_context *svga,
                      struct svga_shader_emitter_v10 *emit,
                      enum pipe_shader_type unit)
{
   struct svga_shader *prevShader = NULL;   /* shader in the previous stage */

   if (unit == PIPE_SHADER_FRAGMENT) {
      prevShader = svga->curr.gs ?
         &svga->curr.gs->base : (svga->curr.tes ?
         &svga->curr.tes->base : &svga->curr.vs->base);
   } else if (unit == PIPE_SHADER_GEOMETRY) {
      prevShader = svga->curr.tes ? &svga->curr.tes->base : &svga->curr.vs->base;
   } else if (unit == PIPE_SHADER_TESS_EVAL) {
      assert(svga->curr.tcs);
      prevShader = &svga->curr.tcs->base;
   } else if (unit == PIPE_SHADER_TESS_CTRL) {
      assert(svga->curr.vs);
      prevShader = &svga->curr.vs->base;
   }

   if (prevShader != NULL) {
      svga_link_shaders(&prevShader->tgsi_info, &emit->info, &emit->linkage);
      emit->prevShaderInfo = &prevShader->tgsi_info;
   } 
   else {
      /**
       * Since vertex shader does not need to go through the linker to
       * establish the input map, we need to make sure the highest index
       * of input registers is set properly here.
       */
      emit->linkage.input_map_max = MAX2((int)emit->linkage.input_map_max,
                                         emit->info.file_max[TGSI_FILE_INPUT]);
   }
}


/**
 * Copies the shader signature info to the shader variant
 */
static void
copy_shader_signature(struct svga_shader_signature *sgn,
                      struct svga_shader_variant *variant)
{
   SVGA3dDXShaderSignatureHeader *header = &sgn->header;

   /* Calculate the signature length */
   variant->signatureLen = sizeof(SVGA3dDXShaderSignatureHeader) +
                           (header->numInputSignatures +
                            header->numOutputSignatures +
                            header->numPatchConstantSignatures) *
                           sizeof(SVGA3dDXShaderSignatureEntry);

   /* Allocate buffer for the signature info */
   variant->signature =
      (SVGA3dDXShaderSignatureHeader *)CALLOC(1, variant->signatureLen);

   char *sgnBuf = (char *)variant->signature;
   unsigned sgnLen;

   /* Copy the signature info to the shader variant structure */
   memcpy(sgnBuf, &sgn->header, sizeof(SVGA3dDXShaderSignatureHeader));
   sgnBuf += sizeof(SVGA3dDXShaderSignatureHeader);

   if (header->numInputSignatures) {
      sgnLen =
         header->numInputSignatures * sizeof(SVGA3dDXShaderSignatureEntry);
      memcpy(sgnBuf, &sgn->inputs[0], sgnLen);
      sgnBuf += sgnLen;
   }

   if (header->numOutputSignatures) {
      sgnLen =
         header->numOutputSignatures * sizeof(SVGA3dDXShaderSignatureEntry);
      memcpy(sgnBuf, &sgn->outputs[0], sgnLen);
      sgnBuf += sgnLen;
   }

   if (header->numPatchConstantSignatures) {
      sgnLen =
         header->numPatchConstantSignatures * sizeof(SVGA3dDXShaderSignatureEntry);
      memcpy(sgnBuf, &sgn->patchConstants[0], sgnLen);
   }
}


/**
 * This is the main entrypoint for the TGSI -> VPGU10 translator.
 */
struct svga_shader_variant *
svga_tgsi_vgpu10_translate(struct svga_context *svga,
                           const struct svga_shader *shader,
                           const struct svga_compile_key *key,
                           enum pipe_shader_type unit)
{
   struct svga_screen *svgascreen = svga_screen(svga->pipe.screen);
   struct svga_shader_variant *variant = NULL;
   struct svga_shader_emitter_v10 *emit;
   const struct tgsi_token *tokens = shader->tokens;

   (void) make_immediate_reg_double;   /* unused at this time */

   assert(unit == PIPE_SHADER_VERTEX ||
          unit == PIPE_SHADER_GEOMETRY ||
          unit == PIPE_SHADER_FRAGMENT ||
          unit == PIPE_SHADER_TESS_CTRL ||
          unit == PIPE_SHADER_TESS_EVAL ||
          unit == PIPE_SHADER_COMPUTE);

   /* These two flags cannot be used together */
   assert(key->vs.need_prescale + key->vs.undo_viewport <= 1);

   SVGA_STATS_TIME_PUSH(svga_sws(svga), SVGA_STATS_TIME_TGSIVGPU10TRANSLATE);
   /*
    * Setup the code emitter
    */
   emit = alloc_emitter();
   if (!emit)
      goto done;

   emit->unit = unit;
   if (svga_have_gl43(svga)) {
      emit->version = 51;
   } else if (svga_have_sm5(svga)) {
      emit->version = 50;
   } else if (svga_have_sm4_1(svga)) {
      emit->version = 41;
   } else {
      emit->version = 40;
   }

   emit->use_sampler_state_mapping = emit->key.sampler_state_mapping;

   emit->signature.header.headerVersion = SVGADX_SIGNATURE_HEADER_VERSION_0;

   emit->key = *key;

   emit->vposition.need_prescale = (emit->key.vs.need_prescale ||
                                    emit->key.gs.need_prescale ||
                                    emit->key.tes.need_prescale);

   /* Determine how many prescale factors in the constant buffer */
   emit->vposition.num_prescale = 1;
   if (emit->vposition.need_prescale && emit->key.gs.writes_viewport_index) {
      assert(emit->unit == PIPE_SHADER_GEOMETRY);
      emit->vposition.num_prescale = emit->key.gs.num_prescale;
   }

   emit->vposition.tmp_index = INVALID_INDEX;
   emit->vposition.so_index = INVALID_INDEX;
   emit->vposition.out_index = INVALID_INDEX;

   emit->vs.vertex_id_sys_index = INVALID_INDEX;
   emit->vs.vertex_id_tmp_index = INVALID_INDEX;
   emit->vs.vertex_id_bias_index = INVALID_INDEX;

   emit->fs.color_tmp_index = INVALID_INDEX;
   emit->fs.face_input_index = INVALID_INDEX;
   emit->fs.fragcoord_input_index = INVALID_INDEX;
   emit->fs.sample_id_sys_index = INVALID_INDEX;
   emit->fs.sample_pos_sys_index = INVALID_INDEX;
   emit->fs.sample_mask_in_sys_index = INVALID_INDEX;
   emit->fs.layer_input_index = INVALID_INDEX;
   emit->fs.layer_imm_index = INVALID_INDEX;

   emit->gs.prim_id_index = INVALID_INDEX;
   emit->gs.invocation_id_sys_index = INVALID_INDEX;
   emit->gs.viewport_index_out_index = INVALID_INDEX;
   emit->gs.viewport_index_tmp_index = INVALID_INDEX;

   emit->tcs.vertices_per_patch_index = INVALID_INDEX;
   emit->tcs.invocation_id_sys_index = INVALID_INDEX;
   emit->tcs.control_point_input_index = INVALID_INDEX;
   emit->tcs.control_point_addr_index = INVALID_INDEX;
   emit->tcs.control_point_out_index = INVALID_INDEX;
   emit->tcs.control_point_tmp_index = INVALID_INDEX;
   emit->tcs.control_point_out_count = 0;
   emit->tcs.inner.out_index = INVALID_INDEX;
   emit->tcs.inner.temp_index = INVALID_INDEX;
   emit->tcs.inner.tgsi_index = INVALID_INDEX;
   emit->tcs.outer.out_index = INVALID_INDEX;
   emit->tcs.outer.temp_index = INVALID_INDEX;
   emit->tcs.outer.tgsi_index = INVALID_INDEX;
   emit->tcs.patch_generic_out_count = 0;
   emit->tcs.patch_generic_out_index = INVALID_INDEX;
   emit->tcs.patch_generic_tmp_index = INVALID_INDEX;
   emit->tcs.prim_id_index = INVALID_INDEX;

   emit->tes.tesscoord_sys_index = INVALID_INDEX;
   emit->tes.inner.in_index = INVALID_INDEX;
   emit->tes.inner.temp_index = INVALID_INDEX;
   emit->tes.inner.tgsi_index = INVALID_INDEX;
   emit->tes.outer.in_index = INVALID_INDEX;
   emit->tes.outer.temp_index = INVALID_INDEX;
   emit->tes.outer.tgsi_index = INVALID_INDEX;
   emit->tes.prim_id_index = INVALID_INDEX;

   emit->cs.thread_id_index = INVALID_INDEX;
   emit->cs.block_id_index = INVALID_INDEX;
   emit->cs.grid_size.tgsi_index = INVALID_INDEX;
   emit->cs.grid_size.imm_index = INVALID_INDEX;
   emit->cs.block_width = 1;
   emit->cs.block_height = 1;
   emit->cs.block_depth = 1;

   emit->clip_dist_out_index = INVALID_INDEX;
   emit->clip_dist_tmp_index = INVALID_INDEX;
   emit->clip_dist_so_index = INVALID_INDEX;
   emit->clip_vertex_out_index = INVALID_INDEX;
   emit->clip_vertex_tmp_index = INVALID_INDEX;
   emit->svga_debug_callback = svga->debug.callback;

   emit->index_range.start_index = INVALID_INDEX;
   emit->index_range.count = 0;
   emit->index_range.required = false;
   emit->index_range.operandType = VGPU10_NUM_OPERANDS;
   emit->index_range.dim = 0;
   emit->index_range.size = 0;

   emit->current_loop_depth = 0;

   emit->initialize_temp_index = INVALID_INDEX;
   emit->image_size_index = INVALID_INDEX;

   emit->max_vs_inputs  = svgascreen->max_vs_inputs;
   emit->max_vs_outputs = svgascreen->max_vs_outputs;
   emit->max_gs_inputs  = svgascreen->max_gs_inputs;

   if (emit->key.fs.alpha_func == SVGA3D_CMP_INVALID) {
      emit->key.fs.alpha_func = SVGA3D_CMP_ALWAYS;
   }

   if (unit == PIPE_SHADER_FRAGMENT) {
      if (key->fs.light_twoside) {
         tokens = transform_fs_twoside(tokens);
      }
      if (key->fs.pstipple) {
         const struct tgsi_token *new_tokens =
            transform_fs_pstipple(emit, tokens);
         if (tokens != shader->tokens) {
            /* free the two-sided shader tokens */
            tgsi_free_tokens(tokens);
         }
         tokens = new_tokens;
      }
      if (key->fs.aa_point) {
         tokens = transform_fs_aapoint(svga, tokens,
			               key->fs.aa_point_coord_index);
      }
   }

   if (SVGA_DEBUG & DEBUG_TGSI) {
      debug_printf("#####################################\n");
      debug_printf("### TGSI Shader %u\n", shader->id);
      tgsi_dump(tokens, 0);
   }

   /**
    * Rescan the header if the token string is different from the one
    * included in the shader; otherwise, the header info is already up-to-date
    */
   if (tokens != shader->tokens) {
      tgsi_scan_shader(tokens, &emit->info);
   } else {
      emit->info = shader->tgsi_info;
   }

   emit->num_outputs = emit->info.num_outputs;

   /**
    * Compute input mapping to match the outputs from shader
    * in the previous stage
    */
   compute_input_mapping(svga, emit, unit);

   determine_clipping_mode(emit);

   if (unit == PIPE_SHADER_GEOMETRY || unit == PIPE_SHADER_VERTEX ||
       unit == PIPE_SHADER_TESS_CTRL || unit == PIPE_SHADER_TESS_EVAL) {
      if (shader->stream_output != NULL || emit->clip_mode == CLIP_DISTANCE) {
         /* if there is stream output declarations associated
          * with this shader or the shader writes to ClipDistance
          * then reserve extra registers for the non-adjusted vertex position
          * and the ClipDistance shadow copy.
          */
         emit->vposition.so_index = emit->num_outputs++;

         if (emit->clip_mode == CLIP_DISTANCE) {
            emit->clip_dist_so_index = emit->num_outputs++;
            if (emit->info.num_written_clipdistance > 4)
               emit->num_outputs++;
         }
      }
   }

   /* Determine if constbuf to rawbuf translation is needed */
   emit->raw_buf_srv_start_index = emit->key.srv_raw_constbuf_index;
   if (emit->info.const_buffers_declared)
      emit->raw_bufs = emit->key.raw_constbufs;

   emit->raw_shaderbuf_srv_start_index = emit->key.srv_raw_shaderbuf_index;
   if (emit->info.shader_buffers_declared)
      emit->raw_shaderbufs = emit->key.raw_shaderbufs;

   /*
    * Do actual shader translation.
    */
   if (!emit_vgpu10_header(emit)) {
      debug_printf("svga: emit VGPU10 header failed\n");
      goto cleanup;
   }

   if (!emit_vgpu10_instructions(emit, tokens)) {
      debug_printf("svga: emit VGPU10 instructions failed\n");
      goto cleanup;
   }

   if (emit->num_new_immediates > 0) {
      reemit_immediates_block(emit);
   }

   if (!emit_vgpu10_tail(emit)) {
      debug_printf("svga: emit VGPU10 tail failed\n");
      goto cleanup;
   }

   if (emit->register_overflow) {
      goto cleanup;
   }

   /*
    * Create, initialize the 'variant' object.
    */
   variant = svga_new_shader_variant(svga, unit);
   if (!variant)
      goto cleanup;

   variant->shader = shader;
   variant->nr_tokens = emit_get_num_tokens(emit);
   variant->tokens = (const unsigned *)emit->buf;

   /* Copy shader signature info to the shader variant */
   if (svga_have_sm5(svga)) {
      copy_shader_signature(&emit->signature, variant);
   }

   emit->buf = NULL;  /* buffer is no longer owed by emitter context */
   memcpy(&variant->key, key, sizeof(*key));
   variant->id = UTIL_BITMASK_INVALID_INDEX;

   /* The extra constant starting offset starts with the number of
    * shader constants declared in the shader.
    */
   variant->extra_const_start = emit->num_shader_consts[0];
   if (key->gs.wide_point) {
      /**
       * The extra constant added in the transformed shader
       * for inverse viewport scale is to be supplied by the driver.
       * So the extra constant starting offset needs to be reduced by 1.
       */
      assert(variant->extra_const_start > 0);
      variant->extra_const_start--;
   }

   if (unit == PIPE_SHADER_FRAGMENT) {
      struct svga_fs_variant *fs_variant = svga_fs_variant(variant);

      fs_variant->pstipple_sampler_unit = emit->fs.pstipple_sampler_unit;
      fs_variant->pstipple_sampler_state_index =
         emit->fs.pstipple_sampler_state_index;

      /* If there was exactly one write to a fragment shader output register
       * and it came from a constant buffer, we know all fragments will have
       * the same color (except for blending).
       */
      fs_variant->constant_color_output =
         emit->constant_color_output && emit->num_output_writes == 1;

      /** keep track in the variant if flat interpolation is used
       *  for any of the varyings.
       */
      fs_variant->uses_flat_interp = emit->uses_flat_interp;

      fs_variant->fs_shadow_compare_units = emit->shadow_compare_units;
   }
   else if (unit == PIPE_SHADER_TESS_EVAL) {
      struct svga_tes_variant *tes_variant = svga_tes_variant(variant);

      /* Keep track in the tes variant some of the layout parameters.
       * These parameters will be referenced by the tcs to emit
       * the necessary declarations for the hull shader.
       */
      tes_variant->prim_mode = emit->tes.prim_mode;
      tes_variant->spacing = emit->tes.spacing;
      tes_variant->vertices_order_cw = emit->tes.vertices_order_cw;
      tes_variant->point_mode = emit->tes.point_mode;
   }


   if (tokens != shader->tokens) {
      tgsi_free_tokens(tokens);
   }

cleanup:
   free_emitter(emit);

done:
   SVGA_STATS_TIME_POP(svga_sws(svga));
   return variant;
}
