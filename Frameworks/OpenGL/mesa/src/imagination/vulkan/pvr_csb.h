/*
 * Copyright © 2022 Imagination Technologies Ltd.
 *
 * based in part on anv driver which is:
 * Copyright © 2015 Intel Corporation
 *
 * based in part on v3dv_cl.h which is:
 * Copyright © 2019 Raspberry Pi
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

#ifndef PVR_CSB_H
#define PVR_CSB_H

#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <vulkan/vulkan.h>

#include "pvr_bo.h"
#include "pvr_types.h"
#include "pvr_winsys.h"
#include "util/list.h"
#include "util/macros.h"
#include "util/u_dynarray.h"

#define __pvr_address_type pvr_dev_addr_t
#define __pvr_get_address(pvr_dev_addr) (pvr_dev_addr).addr
/* clang-format off */
#define __pvr_make_address(addr_u64) PVR_DEV_ADDR(addr_u64)
/* clang-format on */

#include "csbgen/rogue_hwdefs.h"

/**
 * \brief Size of the individual csb buffer object.
 */
#define PVR_CMD_BUFFER_CSB_BO_SIZE 4096

struct pvr_device;

enum pvr_cmd_stream_type {
   PVR_CMD_STREAM_TYPE_INVALID = 0, /* explicitly treat 0 as invalid */
   PVR_CMD_STREAM_TYPE_GRAPHICS,
   PVR_CMD_STREAM_TYPE_GRAPHICS_DEFERRED,
   PVR_CMD_STREAM_TYPE_COMPUTE,
};

struct pvr_csb {
   struct pvr_device *device;

   /* Pointer to current csb buffer object */
   struct pvr_bo *pvr_bo;

   /* pointers to current bo memory */
   void *start;
   void *end;
   void *next;

   /* When extending the control stream we can't break state updates across bos.
    * This indicates where the current state update starts, so that it can be
    * be relocated into the new bo without breaking the update.
    */
   void *relocation_mark;
#if defined(DEBUG)
   /* Used to track the state of the `relocation_mark` and to catch cases where
    * the driver might have emitted to the cs without using the
    * `relocation_mark`. Doing so is mostly harmless but will waste memory in
    * case the cs is extended while an untracked state update is emitted, as
    * we'll have to relocate the cs contents from the last tracked state update
    * instead of just the one currently being emitted.
    */
   enum pvr_csb_relocation_mark_status {
      PVR_CSB_RELOCATION_MARK_UNINITIALIZED,
      PVR_CSB_RELOCATION_MARK_SET,
      PVR_CSB_RELOCATION_MARK_SET_AND_CONSUMED,
      PVR_CSB_RELOCATION_MARK_CLEARED,
   } relocation_mark_status;
#endif

   /* List of csb buffer objects */
   struct list_head pvr_bo_list;

   struct util_dynarray deferred_cs_mem;

   enum pvr_cmd_stream_type stream_type;

   /* Current error status of the command buffer. Used to track inconsistent
    * or incomplete command buffer states that are the consequence of run-time
    * errors such as out of memory scenarios. We want to track this in the
    * csb because the command buffer object is not visible to some parts
    * of the driver.
    */
   VkResult status;
};

/**
 * \brief Gets the status of the csb.
 *
 * \param[in] csb Control Stream Builder object.
 * \return VK_SUCCESS if the csb hasn't encountered any error or error code
 *         otherwise.
 */
static inline VkResult pvr_csb_get_status(const struct pvr_csb *csb)
{
   return csb->status;
}

/**
 * \brief Checks if the control stream is empty or not.
 *
 * \param[in] csb Control Stream Builder object.
 * \return true if csb is empty false otherwise.
 */
static inline bool pvr_csb_is_empty(const struct pvr_csb *csb)
{
   return list_is_empty(&csb->pvr_bo_list);
}

static inline pvr_dev_addr_t
pvr_csb_get_start_address(const struct pvr_csb *csb)
{
   if (!pvr_csb_is_empty(csb)) {
      struct pvr_bo *pvr_bo =
         list_first_entry(&csb->pvr_bo_list, struct pvr_bo, link);

      return pvr_bo->vma->dev_addr;
   }

   return PVR_DEV_ADDR_INVALID;
}

/** \defgroup CSB relocation marking.
 * Functions and macros related to relocation marking for control stream words.
 *
 * When there is no more space left in the current bo, csb needs has to extend
 * the control stream by allocating a new bo and emitting a link to it. State
 * updates have to be contiguous so cannot be broken by a link. Thus csb copies
 * the current, in construction, state update into the new bo and emits a link
 * in its place in the old bo. To do so however, it needs a hint from the driver
 * to determine where the current state update started from, so a relocation
 * mark is used.
 *
 * List of words demarking the beginning of state updates (i.e. state update
 * headers):
 *  - ROGUE_VDMCTRL_PPP_STATE0
 *  - ROGUE_VDMCTRL_PDS_STATE0
 *  - ROGUE_VDMCTRL_VDM_STATE0
 *  - ROGUE_VDMCTRL_INDEX_LIST0
 *  - ROGUE_VDMCTRL_STREAM_LINK0
 *  - ROGUE_VDMCTRL_STREAM_RETURN
 *  - ROGUE_VDMCTRL_STREAM_TERMINATE
 *
 *  - ROGUE_CDMCTRL_KERNEL0
 *  - ROGUE_CDMCTRL_STREAM_LINK0
 *  - ROGUE_CDMCTRL_STREAM_TERMINATE
 *
 * The driver should set the relocation mark whenever a new state update is
 * started. And clear it when the state update is fully formed.
 *
 * PVR_CSB_RELOCATION_MARK state machine:
 *
 *    UNINITIALIZED
 *         ↓
 * ┌─── → SET ─────────┐
 * │       ↓           │
 * │ SET_AND_CONSUMED  │
 * │       ↓           │
 * │    CLEARED ← ─────┘
 * └───────┘
 *
 * @{
 */
/* TODO: Add in the IPF transfer control stream state updates to the list once
 * csb gets used for it
 */

/**
 * \brief Set the relocation mark.
 *
 * Indicates to csb that on cs extension it should relocate all words, starting
 * from now, into the new bo.
 */
static inline void pvr_csb_set_relocation_mark(struct pvr_csb *csb)
{
#if defined(DEBUG)
   assert(csb->relocation_mark_status ==
             PVR_CSB_RELOCATION_MARK_UNINITIALIZED ||
          csb->relocation_mark_status == PVR_CSB_RELOCATION_MARK_CLEARED);

   csb->relocation_mark_status = PVR_CSB_RELOCATION_MARK_SET;
#endif

   csb->relocation_mark = csb->next;
}

/**
 * \brief Clear the relocation mark.
 *
 * Indicate to csb that the state update is fully formed so it doesn't need to
 * relocate it in case of cs extension.
 */
static inline void pvr_csb_clear_relocation_mark(UNUSED struct pvr_csb *csb)
{
#if defined(DEBUG)
   assert(csb->relocation_mark_status == PVR_CSB_RELOCATION_MARK_SET ||
          csb->relocation_mark_status ==
             PVR_CSB_RELOCATION_MARK_SET_AND_CONSUMED);

   csb->relocation_mark_status = PVR_CSB_RELOCATION_MARK_CLEARED;
#endif
}

/** @} */
/* End of \defgroup CSB relocation marking. */

void pvr_csb_init(struct pvr_device *device,
                  enum pvr_cmd_stream_type stream_type,
                  struct pvr_csb *csb);
void pvr_csb_finish(struct pvr_csb *csb);
VkResult pvr_csb_bake(struct pvr_csb *csb, struct list_head *bo_list_out);
void *pvr_csb_alloc_dwords(struct pvr_csb *csb, uint32_t num_dwords);
VkResult pvr_csb_copy(struct pvr_csb *csb_dst, struct pvr_csb *csb_src);
void pvr_csb_emit_link(struct pvr_csb *csb, pvr_dev_addr_t addr, bool ret);
VkResult pvr_csb_emit_return(struct pvr_csb *csb);
VkResult pvr_csb_emit_terminate(struct pvr_csb *csb);

void pvr_csb_dump(const struct pvr_csb *csb,
                  uint32_t frame_num,
                  uint32_t job_num);

#define PVRX(x) ROGUE_##x
#define pvr_cmd_length(x) PVRX(x##_length)
#define pvr_cmd_header(x) PVRX(x##_header)
#define pvr_cmd_pack(x) PVRX(x##_pack)
#define pvr_cmd_unpack(x) PVRX(x##_unpack)
#define pvr_cmd_enum_to_str(x) PVRX(x##_to_str)

/**
 * \brief Merges dwords0 and dwords1 arrays and stores the result into the
 * control stream pointed by the csb object.
 *
 * \param[in] csb     Control Stream Builder object.
 * \param[in] dwords0 Dwords0 array.
 * \param[in] dwords1 Dwords1 array.
 */
#define pvr_csb_emit_merge(csb, dwords0, dwords1)                \
   do {                                                          \
      uint32_t *dw;                                              \
      STATIC_ASSERT(ARRAY_SIZE(dwords0) == ARRAY_SIZE(dwords1)); \
      dw = pvr_csb_alloc_dwords(csb, ARRAY_SIZE(dwords0));       \
      if (!dw)                                                   \
         break;                                                  \
      for (uint32_t i = 0; i < ARRAY_SIZE(dwords0); i++)         \
         dw[i] = (dwords0)[i] | (dwords1)[i];                    \
   } while (0)

/**
 * \brief Packs a command/state into one or more dwords and stores them into
 * the control stream pointed by the csb object.
 *
 * \param[in] csb      Control Stream Builder object.
 * \param[in] cmd      Command/state type.
 * \param[in,out] name Name to give to the command/state structure variable,
 *                     which contains the information to be packed. This can be
 *                     used by the caller to modify the command or state
 *                     information before it's packed.
 */
#define pvr_csb_emit(csb, cmd, name)                               \
   for (struct PVRX(cmd)                                           \
           name = { pvr_cmd_header(cmd) },                         \
           *_dst = pvr_csb_alloc_dwords(csb, pvr_cmd_length(cmd)); \
        __builtin_expect(_dst != NULL, 1);                         \
        ({                                                         \
           pvr_cmd_pack(cmd)(_dst, &name);                         \
           _dst = NULL;                                            \
        }))

/**
 * \brief Stores dword into the control stream pointed by the csb object.
 *
 * \param[in] csb   Control Stream Builder object.
 * \param[in] dword Dword to store into control stream.
 */
#define pvr_csb_emit_dword(csb, dword)                  \
   do {                                                 \
      uint32_t *dw;                                     \
      STATIC_ASSERT(sizeof(dword) == sizeof(uint32_t)); \
      dw = pvr_csb_alloc_dwords(csb, 1U);               \
      if (!dw)                                          \
         break;                                         \
      *dw = dword;                                      \
   } while (0)

/**
 * \name Raw command/state buffer helpers.
 * These provide functionality to read or write control/state words from/to a
 * raw buffer, accessed through a pointer, with some extra checks.
 *
 * The raw buffer doesn't have to be related to a control stream builder object
 * so these can be used with any cpu accessible buffer.
 */
/**@{*/

/**
 * \brief Packs a command/state into one or more dwords and stores them in the
 * memory pointed to by _dst.
 *
 * \param[out] _dst    Pointer to store the packed command/state.
 * \param[in] cmd      Command/state type.
 * \param[in,out] name Name to give to the command/state structure variable,
 *                     which contains the information to be packed and emitted.
 *                     This can be used by the caller to modify the command or
 *                     state information before it's packed.
 */
#define pvr_csb_pack(_dst, cmd, name)                           \
   for (struct PVRX(cmd) name = { pvr_cmd_header(cmd) },        \
                         *_loop_terminate = &name;              \
        __builtin_expect(_loop_terminate != NULL, 1);           \
        ({                                                      \
           STATIC_ASSERT(sizeof(*(_dst)) ==                     \
                         PVR_DW_TO_BYTES(pvr_cmd_length(cmd))); \
           pvr_cmd_pack(cmd)((_dst), &name);                    \
           _loop_terminate = NULL;                              \
        }))

/**
 * \brief Unpacks one or more dwords into a command/state struct.
 *
 * Unlike pvr_csb_pack, this returns the stack-allocated struct directly
 * since it is not needed afterwards.
 *
 * \param[in] _src     Pointer to read the packed command/state from.
 * \param[in] cmd      Command/state type.
 */
#define pvr_csb_unpack(_src, cmd)                                             \
   ({                                                                         \
      struct PVRX(cmd) _name;                                                 \
      STATIC_ASSERT(sizeof(*(_src)) == PVR_DW_TO_BYTES(pvr_cmd_length(cmd))); \
      pvr_cmd_unpack(cmd)((_src), &_name);                                    \
      _name;                                                                  \
   })

/**
 * \brief Writes a command/state word value into a raw buffer and advance.
 *
 * The buffer pointer is incremented appropriately based on the control stream
 * word length.
 *
 * \param[in,out] dst Raw buffer pointer for writing.
 * \param[in]     cmd Command/state type.
 * \param[in]     val Pre-packed value to write.
 */
#define pvr_csb_write_value(dst, cmd, val)                                  \
   do {                                                                     \
      static_assert(sizeof(*(dst)) == PVR_DW_TO_BYTES(pvr_cmd_length(cmd)), \
                    "Size mismatch");                                       \
      static_assert(sizeof(*(dst)) == sizeof(val), "Size mismatch");        \
      *(dst) = (val);                                                       \
      (dst)++;                                                              \
   } while (0)

/**
 * \brief Packs a command/state word struct and writes the value into a raw
 * buffer and advance.
 *
 * The buffer pointer is incremented appropriately based on the control stream
 * word length.
 *
 * \param[in,out] dst Raw buffer pointer for writing.
 * \param[in]     cmd Command/state type.
 * \param[in]     val Command/state struct to pack and write.
 */
#define pvr_csb_write_struct(dst, cmd, val)                                 \
   do {                                                                     \
      static_assert(sizeof(*(dst)) == PVR_DW_TO_BYTES(pvr_cmd_length(cmd)), \
                    "Size mismatch");                                       \
      pvr_cmd_pack(cmd)((dst), (val));                                      \
      (dst)++;                                                              \
   } while (0)

/**@}*/
/* End of \name Raw command/state buffer helpers. */

#endif /* PVR_CSB_H */
