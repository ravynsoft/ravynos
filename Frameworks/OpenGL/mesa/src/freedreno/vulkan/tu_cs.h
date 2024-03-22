/*
 * Copyright © 2019 Google LLC
 * SPDX-License-Identifier: MIT
 */

#ifndef TU_CS_H
#define TU_CS_H

#include "tu_common.h"

#include "freedreno_pm4.h"

#include "tu_knl.h"

/* For breadcrumbs we may open a network socket based on the envvar,
 * it's not something that should be enabled by default.
 */
#define TU_BREADCRUMBS_ENABLED 0

enum tu_cs_mode
{

   /*
    * A command stream in TU_CS_MODE_GROW mode grows automatically whenever it
    * is full.  tu_cs_begin must be called before command packet emission and
    * tu_cs_end must be called after.
    *
    * This mode may create multiple entries internally.  The entries must be
    * submitted together.
    */
   TU_CS_MODE_GROW,

   /*
    * A command stream in TU_CS_MODE_EXTERNAL mode wraps an external,
    * fixed-size buffer.  tu_cs_begin and tu_cs_end are optional and have no
    * effect on it.
    *
    * This mode does not create any entry or any BO.
    */
   TU_CS_MODE_EXTERNAL,

   /*
    * A command stream in TU_CS_MODE_SUB_STREAM mode does not support direct
    * command packet emission.  tu_cs_begin_sub_stream must be called to get a
    * sub-stream to emit comamnd packets to.  When done with the sub-stream,
    * tu_cs_end_sub_stream must be called.
    *
    * This mode does not create any entry internally.
    */
   TU_CS_MODE_SUB_STREAM,
};

struct tu_cs_entry
{
   /* No ownership */
   const struct tu_bo *bo;

   uint32_t size;
   uint32_t offset;
};

struct tu_cs_memory {
   uint32_t *map;
   uint64_t iova;
   bool writeable;
};

struct tu_draw_state {
   uint64_t iova;
   uint16_t size;
   bool writeable;
};

struct tu_bo_array {
   struct tu_bo **bos;
   uint32_t bo_count;
   uint32_t bo_capacity;
   uint32_t *start;
};

#define TU_COND_EXEC_STACK_SIZE 4

struct tu_cs
{
   uint32_t *start;
   uint32_t *cur;
   uint32_t *reserved_end;
   uint32_t *end;
   const char *name;

   struct tu_device *device;
   enum tu_cs_mode mode;
   bool writeable;
   uint32_t next_bo_size;

   struct tu_cs_entry *entries;
   uint32_t entry_count;
   uint32_t entry_capacity;

   struct tu_bo_array read_only, read_write;

   /* Optional BO that this CS is sub-allocated from for TU_CS_MODE_SUB_STREAM */
   struct tu_bo *refcount_bo;

   /* iova that this CS starts with in TU_CS_MODE_EXTERNAL */
   uint64_t external_iova;

   /* state for cond_exec_start/cond_exec_end */
   uint32_t cond_stack_depth;
   uint32_t cond_flags[TU_COND_EXEC_STACK_SIZE];
   uint32_t *cond_dwords[TU_COND_EXEC_STACK_SIZE];

   uint32_t breadcrumb_emit_after;
};

void
tu_breadcrumbs_init(struct tu_device *device);

void
tu_breadcrumbs_finish(struct tu_device *device);

void
tu_cs_init(struct tu_cs *cs,
           struct tu_device *device,
           enum tu_cs_mode mode,
           uint32_t initial_size, const char *name);

void
tu_cs_init_external(struct tu_cs *cs, struct tu_device *device,
                    uint32_t *start, uint32_t *end, uint64_t iova,
                    bool writeable);

void
tu_cs_init_suballoc(struct tu_cs *cs, struct tu_device *device,
                    struct tu_suballoc_bo *bo);

void
tu_cs_finish(struct tu_cs *cs);

void
tu_cs_begin(struct tu_cs *cs);

void
tu_cs_end(struct tu_cs *cs);

void
tu_cs_set_writeable(struct tu_cs *cs, bool writeable);

VkResult
tu_cs_begin_sub_stream(struct tu_cs *cs, uint32_t size, struct tu_cs *sub_cs);

VkResult
tu_cs_alloc(struct tu_cs *cs,
            uint32_t count,
            uint32_t size,
            struct tu_cs_memory *memory);

struct tu_cs_entry
tu_cs_end_sub_stream(struct tu_cs *cs, struct tu_cs *sub_cs);

static inline struct tu_draw_state
tu_cs_end_draw_state(struct tu_cs *cs, struct tu_cs *sub_cs)
{
   struct tu_cs_entry entry = tu_cs_end_sub_stream(cs, sub_cs);
   return (struct tu_draw_state) {
      .iova = entry.bo->iova + entry.offset,
      .size = entry.size / sizeof(uint32_t),
      .writeable = sub_cs->writeable,
   };
}

VkResult
tu_cs_reserve_space(struct tu_cs *cs, uint32_t reserved_size);

uint64_t
tu_cs_get_cur_iova(const struct tu_cs *cs);

static inline struct tu_draw_state
tu_cs_draw_state(struct tu_cs *sub_cs, struct tu_cs *cs, uint32_t size)
{
   struct tu_cs_memory memory;

   /* TODO: clean this up */
   tu_cs_alloc(sub_cs, size, 1, &memory);
   tu_cs_init_external(cs, sub_cs->device, memory.map, memory.map + size,
                       memory.iova, memory.writeable);
   tu_cs_begin(cs);
   tu_cs_reserve_space(cs, size);

   return (struct tu_draw_state) {
      .iova = memory.iova,
      .size = size,
      .writeable = sub_cs->writeable,
   };
}

void
tu_cs_reset(struct tu_cs *cs);

VkResult
tu_cs_add_entries(struct tu_cs *cs, struct tu_cs *target);

/**
 * Get the size of the command packets emitted since the last call to
 * tu_cs_add_entry.
 */
static inline uint32_t
tu_cs_get_size(const struct tu_cs *cs)
{
   return cs->cur - cs->start;
}

/**
 * Return true if there is no command packet emitted since the last call to
 * tu_cs_add_entry.
 */
static inline uint32_t
tu_cs_is_empty(const struct tu_cs *cs)
{
   return tu_cs_get_size(cs) == 0;
}

/**
 * Discard all entries.  This allows \a cs to be reused while keeping the
 * existing BOs and command packets intact.
 */
static inline void
tu_cs_discard_entries(struct tu_cs *cs)
{
   assert(cs->mode == TU_CS_MODE_GROW);
   cs->entry_count = 0;
}

/**
 * Get the size needed for tu_cs_emit_call.
 */
static inline uint32_t
tu_cs_get_call_size(const struct tu_cs *cs)
{
   assert(cs->mode == TU_CS_MODE_GROW);
   /* each CP_INDIRECT_BUFFER needs 4 dwords */
   return cs->entry_count * 4;
}

/**
 * Assert that we did not exceed the reserved space.
 */
static inline void
tu_cs_sanity_check(const struct tu_cs *cs)
{
   assert(cs->start <= cs->cur);
   assert(cs->cur <= cs->reserved_end);
   assert(cs->reserved_end <= cs->end);
}

void
tu_cs_emit_sync_breadcrumb(struct tu_cs *cs, uint8_t opcode, uint16_t cnt);

/**
 * Emit a uint32_t value into a command stream, without boundary checking.
 */
static inline void
tu_cs_emit(struct tu_cs *cs, uint32_t value)
{
   assert(cs->cur < cs->reserved_end);
   *cs->cur = value;
   ++cs->cur;

#if TU_BREADCRUMBS_ENABLED
   cs->breadcrumb_emit_after--;
   if (cs->breadcrumb_emit_after == 0)
      tu_cs_emit_sync_breadcrumb(cs, -1, 0);
#endif
}

/**
 * Emit an array of uint32_t into a command stream, without boundary checking.
 */
static inline void
tu_cs_emit_array(struct tu_cs *cs, const uint32_t *values, uint32_t length)
{
   assert(cs->cur + length <= cs->reserved_end);
   memcpy(cs->cur, values, sizeof(uint32_t) * length);
   cs->cur += length;
}

/**
 * Get the size of the remaining space in the current BO.
 */
static inline uint32_t
tu_cs_get_space(const struct tu_cs *cs)
{
   return cs->end - cs->cur;
}

static inline void
tu_cs_reserve(struct tu_cs *cs, uint32_t reserved_size)
{
   if (cs->mode != TU_CS_MODE_GROW) {
      assert(tu_cs_get_space(cs) >= reserved_size);
      assert(cs->reserved_end == cs->end);
      return;
   }

   if (tu_cs_get_space(cs) >= reserved_size &&
       cs->entry_count < cs->entry_capacity) {
      cs->reserved_end = cs->cur + reserved_size;
      return;
   }

   ASSERTED VkResult result = tu_cs_reserve_space(cs, reserved_size);
   /* TODO: set this error in tu_cs and use it */
   assert(result == VK_SUCCESS);
}

/**
 * Emit a type-4 command packet header into a command stream.
 */
static inline void
tu_cs_emit_pkt4(struct tu_cs *cs, uint16_t regindx, uint16_t cnt)
{
   tu_cs_reserve(cs, cnt + 1);
   tu_cs_emit(cs, pm4_pkt4_hdr(regindx, cnt));
}

/**
 * Emit a type-7 command packet header into a command stream.
 */
static inline void
tu_cs_emit_pkt7(struct tu_cs *cs, uint8_t opcode, uint16_t cnt)
{
#if TU_BREADCRUMBS_ENABLED
   tu_cs_emit_sync_breadcrumb(cs, opcode, cnt + 1);
#endif

   tu_cs_reserve(cs, cnt + 1);
   tu_cs_emit(cs, pm4_pkt7_hdr(opcode, cnt));
}

static inline void
tu_cs_emit_wfi(struct tu_cs *cs)
{
   tu_cs_emit_pkt7(cs, CP_WAIT_FOR_IDLE, 0);
}

static inline void
tu_cs_emit_qw(struct tu_cs *cs, uint64_t value)
{
   tu_cs_emit(cs, (uint32_t) value);
   tu_cs_emit(cs, (uint32_t) (value >> 32));
}

static inline void
tu_cs_emit_write_reg(struct tu_cs *cs, uint16_t reg, uint32_t value)
{
   tu_cs_emit_pkt4(cs, reg, 1);
   tu_cs_emit(cs, value);
}

/**
 * Emit a CP_INDIRECT_BUFFER command packet.
 */
static inline void
tu_cs_emit_ib(struct tu_cs *cs, const struct tu_cs_entry *entry)
{
   assert(entry->bo);
   assert(entry->size && entry->offset + entry->size <= entry->bo->size);
   assert(entry->size % sizeof(uint32_t) == 0);
   assert(entry->offset % sizeof(uint32_t) == 0);

   tu_cs_emit_pkt7(cs, CP_INDIRECT_BUFFER, 3);
   tu_cs_emit_qw(cs, entry->bo->iova + entry->offset);
   tu_cs_emit(cs, entry->size / sizeof(uint32_t));
}

/* for compute which isn't using SET_DRAW_STATE */
static inline void
tu_cs_emit_state_ib(struct tu_cs *cs, struct tu_draw_state state)
{
   if (state.size) {
      tu_cs_emit_pkt7(cs, CP_INDIRECT_BUFFER, 3);
      tu_cs_emit_qw(cs, state.iova);
      tu_cs_emit(cs, state.size);
   }
}

/**
 * Emit a CP_INDIRECT_BUFFER command packet for each entry in the target
 * command stream.
 */
static inline void
tu_cs_emit_call(struct tu_cs *cs, const struct tu_cs *target)
{
   assert(target->mode == TU_CS_MODE_GROW);
   for (uint32_t i = 0; i < target->entry_count; i++)
      tu_cs_emit_ib(cs, target->entries + i);
}

/**
 * Emit a CP_NOP with a string tail into the command stream.
 */
void
tu_cs_emit_debug_string(struct tu_cs *cs, const char *string, int len);

void
tu_cs_emit_debug_magic_strv(struct tu_cs *cs,
                            uint32_t magic,
                            const char *fmt,
                            va_list args);

__attribute__((format(printf, 2, 3))) void
tu_cs_emit_debug_msg(struct tu_cs *cs, const char *fmt, ...);

/**
 * Emit a single message into the CS that denote the calling function and any
 * optional printf-style parameters when utrace markers are enabled.
 */
#define TU_CS_DEBUG_MSG(CS, FORMAT_STRING, ...)                              \
   do {                                                                      \
      if (unlikely(u_trace_markers_enabled(&(CS)->device->trace_context)))   \
         tu_cs_emit_debug_msg(CS, "%s(" FORMAT_STRING ")", __func__,         \
                              ## __VA_ARGS__);                               \
   } while (0)

typedef struct tu_cs *tu_debug_scope;

__attribute__((format(printf, 3, 4))) void
tu_cs_trace_start(struct u_trace_context *utctx,
                  void *cs,
                  const char *fmt,
                  ...);

__attribute__((format(printf, 3, 4))) void
tu_cs_trace_end(struct u_trace_context *utctx, void *cs, const char *fmt, ...);

/* Helpers for bracketing a large sequence of commands of unknown size inside
 * a CP_COND_REG_EXEC packet.
 */
static inline void
tu_cond_exec_start(struct tu_cs *cs, uint32_t cond_flags)
{
   assert(cs->mode == TU_CS_MODE_GROW);
   assert(cs->cond_stack_depth < TU_COND_EXEC_STACK_SIZE);

   ASSERTED enum compare_mode mode =
      (enum compare_mode)((cond_flags & CP_COND_REG_EXEC_0_MODE__MASK) >>
                          CP_COND_REG_EXEC_0_MODE__SHIFT);
   assert(mode == PRED_TEST || mode == RENDER_MODE || mode == THREAD_MODE);

   tu_cs_emit_pkt7(cs, CP_COND_REG_EXEC, 2);
   tu_cs_emit(cs, cond_flags);

   cs->cond_flags[cs->cond_stack_depth] = cond_flags;
   cs->cond_dwords[cs->cond_stack_depth] = cs->cur;

   /* Emit dummy DWORD field here */
   tu_cs_emit(cs, RENDER_MODE_CP_COND_REG_EXEC_1_DWORDS(0));

   cs->cond_stack_depth++;
}
#define CP_COND_EXEC_0_RENDER_MODE_GMEM \
   (CP_COND_REG_EXEC_0_MODE(RENDER_MODE) | CP_COND_REG_EXEC_0_GMEM)
#define CP_COND_EXEC_0_RENDER_MODE_SYSMEM \
   (CP_COND_REG_EXEC_0_MODE(RENDER_MODE) | CP_COND_REG_EXEC_0_SYSMEM)

static inline void
tu_cond_exec_end(struct tu_cs *cs)
{
   assert(cs->cond_stack_depth > 0);
   cs->cond_stack_depth--;

   cs->cond_flags[cs->cond_stack_depth] = 0;
   /* Subtract one here to account for the DWORD field itself. */
   uint32_t cond_len = cs->cur - cs->cond_dwords[cs->cond_stack_depth] - 1;
   if (cond_len) {
      *cs->cond_dwords[cs->cond_stack_depth] = cond_len;
   } else {
      /* rewind the CS to drop the empty cond reg packet. */
      cs->cur = cs->cur - 3;
   }
}

/* Temporary struct for tracking a register state to be written, used by
 * a6xx-pack.h and tu_cs_emit_regs()
 */
struct tu_reg_value {
   uint32_t reg;
   uint64_t value;
   struct tu_bo *bo;
   bool is_address;
   bool bo_write;
   uint32_t bo_offset;
   uint32_t bo_shift;
   uint32_t bo_low;
};

#define fd_reg_pair tu_reg_value
#define __bo_type struct tu_bo *

#include "a6xx-pack.xml.h"
#include "adreno-pm4-pack.xml.h"

#define __assert_eq(a, b)                                               \
   do {                                                                 \
      if ((a) != (b)) {                                                 \
         fprintf(stderr, "assert failed: " #a " (0x%x) != " #b " (0x%x)\n", a, b); \
         assert((a) == (b));                                            \
      }                                                                 \
   } while (0)

#define __ONE_REG(i, regs)                                      \
   do {                                                         \
      if (i < ARRAY_SIZE(regs) && regs[i].reg > 0) {            \
         __assert_eq(regs[0].reg + i, regs[i].reg);             \
         if (regs[i].bo) {                                      \
            uint64_t v = regs[i].bo->iova + regs[i].bo_offset;  \
            v >>= regs[i].bo_shift;                             \
            v <<= regs[i].bo_low;                               \
            v |= regs[i].value;                                 \
                                                                \
            *p++ = v;                                           \
            *p++ = v >> 32;                                     \
         } else {                                               \
            *p++ = regs[i].value;                               \
            if (regs[i].is_address)                             \
               *p++ = regs[i].value >> 32;                      \
         }                                                      \
      }                                                         \
   } while (0)

/* Emits a sequence of register writes in order using a pkt4.  This will check
 * (at runtime on a !NDEBUG build) that the registers were actually set up in
 * order in the code.
 *
 * Note that references to buffers aren't automatically added to the CS,
 * unlike in freedreno.  We are clever in various places to avoid duplicating
 * the reference add work.
 *
 * Also, 64-bit address registers don't have a way (currently) to set a 64-bit
 * address without having a reference to a BO, since the .dword field in the
 * register's struct is only 32-bit wide.  We should fix this in the pack
 * codegen later.
 */
#define tu_cs_emit_regs(cs, ...) do {                   \
   const struct fd_reg_pair regs[] = { __VA_ARGS__ };   \
   unsigned count = ARRAY_SIZE(regs);                   \
                                                        \
   STATIC_ASSERT(ARRAY_SIZE(regs) > 0);                 \
   STATIC_ASSERT(ARRAY_SIZE(regs) <= 16);               \
                                                        \
   tu_cs_emit_pkt4((cs), regs[0].reg, count);             \
   uint32_t *p = (cs)->cur;                               \
   __ONE_REG( 0, regs);                                 \
   __ONE_REG( 1, regs);                                 \
   __ONE_REG( 2, regs);                                 \
   __ONE_REG( 3, regs);                                 \
   __ONE_REG( 4, regs);                                 \
   __ONE_REG( 5, regs);                                 \
   __ONE_REG( 6, regs);                                 \
   __ONE_REG( 7, regs);                                 \
   __ONE_REG( 8, regs);                                 \
   __ONE_REG( 9, regs);                                 \
   __ONE_REG(10, regs);                                 \
   __ONE_REG(11, regs);                                 \
   __ONE_REG(12, regs);                                 \
   __ONE_REG(13, regs);                                 \
   __ONE_REG(14, regs);                                 \
   __ONE_REG(15, regs);                                 \
   (cs)->cur = p;                                         \
   } while (0)

#endif /* TU_CS_H */
