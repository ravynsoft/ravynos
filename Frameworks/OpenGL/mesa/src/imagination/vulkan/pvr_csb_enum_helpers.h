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

#ifndef PVR_CSB_ENUM_HELPERS_H
#define PVR_CSB_ENUM_HELPERS_H

#include <stdint.h>
#include <vulkan/vulkan_core.h>

#include "pvr_csb.h"
#include "rogue/rogue.h"
#include "util/macros.h"

static const char *
pvr_cmd_stream_type_to_str(const enum pvr_cmd_stream_type stream_type)
{
   switch (stream_type) {
   case PVR_CMD_STREAM_TYPE_INVALID:
      return "INVALID";
   case PVR_CMD_STREAM_TYPE_GRAPHICS:
      return "GRAPHICS";
   case PVR_CMD_STREAM_TYPE_COMPUTE:
      return "COMPUTE";
   default:
      return NULL;
   }
}

/******************************************************************************
   CR
 ******************************************************************************/

/* TODO: Use VkSampleCountFlagBits as param type? */
/* clang-format off */
static inline enum PVRX(CR_ISP_AA_MODE_TYPE)
pvr_cr_isp_aa_mode_type(uint32_t samples)
/* clang-format on */
{
   switch (samples) {
   case 1:
      return PVRX(CR_ISP_AA_MODE_TYPE_AA_NONE);
   case 2:
      return PVRX(CR_ISP_AA_MODE_TYPE_AA_2X);
   case 4:
      return PVRX(CR_ISP_AA_MODE_TYPE_AA_4X);
   case 8:
      return PVRX(CR_ISP_AA_MODE_TYPE_AA_8X);
   default:
      unreachable("Unsupported number of samples");
   }
}

/* clang-format off */
static inline bool
pvr_zls_format_type_is_packed(enum PVRX(CR_ZLS_FORMAT_TYPE) type)
/* clang-format on */
{
   switch (type) {
   case PVRX(CR_ZLS_FORMAT_TYPE_24BITINT):
   case PVRX(CR_ZLS_FORMAT_TYPE_F64Z):
      return true;

   case PVRX(CR_ZLS_FORMAT_TYPE_F32Z):
   case PVRX(CR_ZLS_FORMAT_TYPE_16BITINT):
      return false;

   default:
      unreachable("Invalid ZLS format type");
   }
}

/* clang-format off */
static inline bool
pvr_zls_format_type_is_int(enum PVRX(CR_ZLS_FORMAT_TYPE) type)
/* clang-format on */
{
   switch (type) {
   case PVRX(CR_ZLS_FORMAT_TYPE_24BITINT):
   case PVRX(CR_ZLS_FORMAT_TYPE_16BITINT):
      return true;

   case PVRX(CR_ZLS_FORMAT_TYPE_F32Z):
   case PVRX(CR_ZLS_FORMAT_TYPE_F64Z):
      return false;

   default:
      unreachable("Invalid ZLS format type");
   }
}

/******************************************************************************
   PDS
 ******************************************************************************/

/* clang-format off */
static inline enum PVRX(PDSINST_DOUTU_SAMPLE_RATE)
pvr_pdsinst_doutu_sample_rate_from_rogue(enum rogue_msaa_mode msaa_mode)
/* clang-format on */
{
   switch (msaa_mode) {
   case ROGUE_MSAA_MODE_PIXEL:
      return PVRX(PDSINST_DOUTU_SAMPLE_RATE_INSTANCE);
   case ROGUE_MSAA_MODE_SELECTIVE:
      return PVRX(PDSINST_DOUTU_SAMPLE_RATE_SELECTIVE);
   case ROGUE_MSAA_MODE_FULL:
      return PVRX(PDSINST_DOUTU_SAMPLE_RATE_FULL);
   default:
      unreachable("Undefined MSAA mode.");
   }
}

/******************************************************************************
   PBESTATE
 ******************************************************************************/

enum pvr_pbe_source_start_pos {
   PVR_PBE_STARTPOS_BIT0,
   PVR_PBE_STARTPOS_BIT32,
   PVR_PBE_STARTPOS_BIT64,
   PVR_PBE_STARTPOS_BIT96,
   /* The below values are available if has_eight_output_registers feature is
    * enabled.
    */
   PVR_PBE_STARTPOS_BIT128,
   PVR_PBE_STARTPOS_BIT160,
   PVR_PBE_STARTPOS_BIT192,
   PVR_PBE_STARTPOS_BIT224,
};

static inline enum ROGUE_PBESTATE_SOURCE_POS
pvr_pbestate_source_pos(enum pvr_pbe_source_start_pos pos)
{
   switch (pos) {
   case PVR_PBE_STARTPOS_BIT0:
   case PVR_PBE_STARTPOS_BIT128:
      return ROGUE_PBESTATE_SOURCE_POS_START_BIT0;

   case PVR_PBE_STARTPOS_BIT32:
   case PVR_PBE_STARTPOS_BIT160:
      return ROGUE_PBESTATE_SOURCE_POS_START_BIT32;

   case PVR_PBE_STARTPOS_BIT64:
   case PVR_PBE_STARTPOS_BIT192:
      return ROGUE_PBESTATE_SOURCE_POS_START_BIT64;

   case PVR_PBE_STARTPOS_BIT96:
   case PVR_PBE_STARTPOS_BIT224:
      return ROGUE_PBESTATE_SOURCE_POS_START_BIT96;

   default:
      unreachable("Undefined PBE source pos.");
   }
}

/******************************************************************************
   TA
 ******************************************************************************/

static inline enum PVRX(TA_CMPMODE) pvr_ta_cmpmode(VkCompareOp op)
{
   /* enum values are identical, so we can just cast the input directly. */
   return (enum PVRX(TA_CMPMODE))op;
}

static inline enum PVRX(TA_ISPB_STENCILOP) pvr_ta_stencilop(VkStencilOp op)
{
   /* enum values are identical, so we can just cast the input directly. */
   return (enum PVRX(TA_ISPB_STENCILOP))op;
}

/* clang-format off */
static inline enum PVRX(TA_OBJTYPE)
pvr_ta_objtype(VkPrimitiveTopology topology)
/* clang-format on */
{
   switch (topology) {
   case VK_PRIMITIVE_TOPOLOGY_POINT_LIST:
      return PVRX(TA_OBJTYPE_SPRITE_01UV);

   case VK_PRIMITIVE_TOPOLOGY_LINE_LIST:
   case VK_PRIMITIVE_TOPOLOGY_LINE_STRIP:
   case VK_PRIMITIVE_TOPOLOGY_LINE_LIST_WITH_ADJACENCY:
   case VK_PRIMITIVE_TOPOLOGY_LINE_STRIP_WITH_ADJACENCY:
      return PVRX(TA_OBJTYPE_LINE);

   case VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST:
   case VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP:
   case VK_PRIMITIVE_TOPOLOGY_TRIANGLE_FAN:
   case VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST_WITH_ADJACENCY:
   case VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP_WITH_ADJACENCY:
      return PVRX(TA_OBJTYPE_TRIANGLE);

   default:
      unreachable("Invalid topology.");
      return 0;
   }
}

/******************************************************************************
   TEXSTATE
 ******************************************************************************/

static inline enum PVRX(TEXSTATE_CMP_MODE) pvr_texstate_cmpmode(VkCompareOp op)
{
   /* enum values are identical, so we can just cast the input directly. */
   return (enum PVRX(TEXSTATE_CMP_MODE))op;
}

/******************************************************************************
   VDMCTRL
 ******************************************************************************/

/* clang-format off */
static inline uint32_t
pvr_vdmctrl_index_size_nr_bytes(enum PVRX(VDMCTRL_INDEX_SIZE) index_size)
/* clang-format on */
{
   switch (index_size) {
   case PVRX(VDMCTRL_INDEX_SIZE_B8):
      return 1;
   case PVRX(VDMCTRL_INDEX_SIZE_B16):
      return 2;
   case PVRX(VDMCTRL_INDEX_SIZE_B32):
      return 4;
   default:
      return 0;
   }
}

#endif /* PVR_CSB_ENUM_HELPERS_H */
