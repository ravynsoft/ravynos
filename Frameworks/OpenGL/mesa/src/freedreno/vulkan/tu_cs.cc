/*
 * Copyright © 2019 Google LLC
 * SPDX-License-Identifier: MIT
 */

#include "tu_cs.h"

#include "tu_suballoc.h"

/**
 * Initialize a command stream.
 */
void
tu_cs_init(struct tu_cs *cs,
           struct tu_device *device,
           enum tu_cs_mode mode,
           uint32_t initial_size, const char *name)
{
   assert(mode != TU_CS_MODE_EXTERNAL);

   memset(cs, 0, sizeof(*cs));

   cs->device = device;
   cs->mode = mode;
   cs->next_bo_size = initial_size;
   cs->name = name;
}

/**
 * Initialize a command stream as a wrapper to an external buffer.
 */
void
tu_cs_init_external(struct tu_cs *cs, struct tu_device *device,
                    uint32_t *start, uint32_t *end, uint64_t iova,
                    bool writeable)
{
   memset(cs, 0, sizeof(*cs));

   cs->device = device;
   cs->mode = TU_CS_MODE_EXTERNAL;
   cs->start = cs->reserved_end = cs->cur = start;
   cs->end = end;
   cs->external_iova = iova;
   cs->writeable = writeable;
}

/**
 * Initialize a sub-command stream as a wrapper to an externally sub-allocated
 * buffer.
 */
void
tu_cs_init_suballoc(struct tu_cs *cs, struct tu_device *device,
                    struct tu_suballoc_bo *suballoc_bo)
{
   uint32_t *start = (uint32_t *) tu_suballoc_bo_map(suballoc_bo);
   uint32_t *end = start + (suballoc_bo->size >> 2);

   memset(cs, 0, sizeof(*cs));
   cs->device = device;
   cs->mode = TU_CS_MODE_SUB_STREAM;
   cs->start = cs->reserved_end = cs->cur = start;
   cs->end = end;
   cs->refcount_bo = tu_bo_get_ref(suballoc_bo->bo);
}

/**
 * Finish and release all resources owned by a command stream.
 */
void
tu_cs_finish(struct tu_cs *cs)
{
   for (uint32_t i = 0; i < cs->read_only.bo_count; ++i) {
      tu_bo_finish(cs->device, cs->read_only.bos[i]);
   }

   for (uint32_t i = 0; i < cs->read_write.bo_count; ++i) {
      tu_bo_finish(cs->device, cs->read_write.bos[i]);
   }

   if (cs->refcount_bo)
      tu_bo_finish(cs->device, cs->refcount_bo);

   free(cs->entries);
   free(cs->read_only.bos);
   free(cs->read_write.bos);
}

static struct tu_bo *
tu_cs_current_bo(const struct tu_cs *cs)
{
   if (cs->refcount_bo) {
      return cs->refcount_bo;
   } else {
      const struct tu_bo_array *bos = cs->writeable ? &cs->read_write : &cs->read_only;
      assert(bos->bo_count);
      return bos->bos[bos->bo_count - 1];
   }
}

/**
 * Get the offset of the command packets emitted since the last call to
 * tu_cs_add_entry.
 */
static uint32_t
tu_cs_get_offset(const struct tu_cs *cs)
{
   return cs->start - (uint32_t *) tu_cs_current_bo(cs)->map;
}

/* Get the iova for the next dword to be emitted. Useful after
 * tu_cs_reserve_space() to create a patch point that can be overwritten on
 * the GPU.
 */
uint64_t
tu_cs_get_cur_iova(const struct tu_cs *cs)
{
   if (cs->mode == TU_CS_MODE_EXTERNAL)
      return cs->external_iova + ((char *) cs->cur - (char *) cs->start);
   return tu_cs_current_bo(cs)->iova + ((char *) cs->cur - (char *) tu_cs_current_bo(cs)->map);
}

/*
 * Allocate and add a BO to a command stream.  Following command packets will
 * be emitted to the new BO.
 */
static VkResult
tu_cs_add_bo(struct tu_cs *cs, uint32_t size)
{
   /* no BO for TU_CS_MODE_EXTERNAL */
   assert(cs->mode != TU_CS_MODE_EXTERNAL);
   /* No adding more BOs if suballocating from a suballoc_bo. */
   assert(!cs->refcount_bo);

   /* no dangling command packet */
   assert(tu_cs_is_empty(cs));

   struct tu_bo_array *bos = cs->writeable ? &cs->read_write : &cs->read_only;

   /* grow cs->bos if needed */
   if (bos->bo_count == bos->bo_capacity) {
      uint32_t new_capacity = MAX2(4, 2 * bos->bo_capacity);
      struct tu_bo **new_bos = (struct tu_bo **)
         realloc(bos->bos, new_capacity * sizeof(struct tu_bo *));
      if (!new_bos)
         return VK_ERROR_OUT_OF_HOST_MEMORY;

      bos->bo_capacity = new_capacity;
      bos->bos = new_bos;
   }

   struct tu_bo *new_bo;

   VkResult result =
      tu_bo_init_new(cs->device, &new_bo, size * sizeof(uint32_t),
                     (enum tu_bo_alloc_flags)(COND(!cs->writeable,
                                                   TU_BO_ALLOC_GPU_READ_ONLY) |
                                              TU_BO_ALLOC_ALLOW_DUMP),
                     cs->name);
   if (result != VK_SUCCESS) {
      return result;
   }

   result = tu_bo_map(cs->device, new_bo);
   if (result != VK_SUCCESS) {
      tu_bo_finish(cs->device, new_bo);
      return result;
   }

   bos->bos[bos->bo_count++] = new_bo;

   cs->start = cs->cur = cs->reserved_end = (uint32_t *) new_bo->map;
   cs->end = cs->start + new_bo->size / sizeof(uint32_t);

   return VK_SUCCESS;
}

/**
 * Reserve an IB entry.
 */
static VkResult
tu_cs_reserve_entry(struct tu_cs *cs)
{
   /* entries are only for TU_CS_MODE_GROW */
   assert(cs->mode == TU_CS_MODE_GROW);

   /* grow cs->entries if needed */
   if (cs->entry_count == cs->entry_capacity) {
      uint32_t new_capacity = MAX2(4, cs->entry_capacity * 2);
      struct tu_cs_entry *new_entries = (struct tu_cs_entry *)
         realloc(cs->entries, new_capacity * sizeof(struct tu_cs_entry));
      if (!new_entries)
         return VK_ERROR_OUT_OF_HOST_MEMORY;

      cs->entry_capacity = new_capacity;
      cs->entries = new_entries;
   }

   return VK_SUCCESS;
}


/**
 * Add an IB entry for the command packets emitted since the last call to this
 * function.
 */
static void
tu_cs_add_entry(struct tu_cs *cs)
{
   /* entries are only for TU_CS_MODE_GROW */
   assert(cs->mode == TU_CS_MODE_GROW);

   /* disallow empty entry */
   assert(!tu_cs_is_empty(cs));

   /*
    * because we disallow empty entry, tu_cs_add_bo and tu_cs_reserve_entry
    * must both have been called
    */
   assert(cs->writeable ? cs->read_write.bo_count : cs->read_only.bo_count);
   assert(cs->entry_count < cs->entry_capacity);

   /* add an entry for [cs->start, cs->cur] */
   cs->entries[cs->entry_count++] = (struct tu_cs_entry) {
      .bo = tu_cs_current_bo(cs),
      .size = tu_cs_get_size(cs) * sizeof(uint32_t),
      .offset = tu_cs_get_offset(cs) * sizeof(uint32_t),
   };

   cs->start = cs->cur;
}

/**
 * same behavior as tu_cs_emit_call but without the indirect
 */
VkResult
tu_cs_add_entries(struct tu_cs *cs, struct tu_cs *target)
{
   VkResult result;

   assert(cs->mode == TU_CS_MODE_GROW);
   assert(target->mode == TU_CS_MODE_GROW);

   if (!tu_cs_is_empty(cs))
      tu_cs_add_entry(cs);

   for (unsigned i = 0; i < target->entry_count; i++) {
      result = tu_cs_reserve_entry(cs);
      if (result != VK_SUCCESS)
         return result;
      cs->entries[cs->entry_count++] = target->entries[i];
   }

   return VK_SUCCESS;
}

/**
 * Begin (or continue) command packet emission.  This does nothing but sanity
 * checks currently.  \a cs must not be in TU_CS_MODE_SUB_STREAM mode.
 */
void
tu_cs_begin(struct tu_cs *cs)
{
   assert(cs->mode != TU_CS_MODE_SUB_STREAM);
   assert(tu_cs_is_empty(cs));
}

/**
 * End command packet emission.  This adds an IB entry when \a cs is in
 * TU_CS_MODE_GROW mode.
 */
void
tu_cs_end(struct tu_cs *cs)
{
   assert(cs->mode != TU_CS_MODE_SUB_STREAM);

   if (cs->mode == TU_CS_MODE_GROW && !tu_cs_is_empty(cs))
      tu_cs_add_entry(cs);
}

void
tu_cs_set_writeable(struct tu_cs *cs, bool writeable)
{
   assert(cs->mode == TU_CS_MODE_GROW || cs->mode == TU_CS_MODE_SUB_STREAM);

   if (cs->writeable != writeable) {
      if (cs->mode == TU_CS_MODE_GROW && !tu_cs_is_empty(cs))
         tu_cs_add_entry(cs);
      struct tu_bo_array *old_bos = cs->writeable ? &cs->read_write : &cs->read_only;
      struct tu_bo_array *new_bos = writeable ? &cs->read_write : &cs->read_only;

      old_bos->start = cs->start;
      cs->start = cs->cur = cs->reserved_end = new_bos->start;
      if (new_bos->bo_count) {
         struct tu_bo *bo = new_bos->bos[new_bos->bo_count - 1];
         cs->end = (uint32_t *)bo->map + bo->size / sizeof(uint32_t);
      } else {
         cs->end = NULL;
      }

      cs->writeable = writeable;
   }
}

/**
 * Begin command packet emission to a sub-stream.  \a cs must be in
 * TU_CS_MODE_SUB_STREAM mode.
 *
 * Return \a sub_cs which is in TU_CS_MODE_EXTERNAL mode.  tu_cs_begin and
 * tu_cs_reserve_space are implied and \a sub_cs is ready for command packet
 * emission.
 */
VkResult
tu_cs_begin_sub_stream(struct tu_cs *cs, uint32_t size, struct tu_cs *sub_cs)
{
   assert(cs->mode == TU_CS_MODE_SUB_STREAM);
   assert(size);

   VkResult result = tu_cs_reserve_space(cs, size);
   if (result != VK_SUCCESS)
      return result;

   tu_cs_init_external(sub_cs, cs->device, cs->cur, cs->reserved_end,
                       tu_cs_get_cur_iova(cs), cs->writeable);
   tu_cs_begin(sub_cs);
   result = tu_cs_reserve_space(sub_cs, size);
   assert(result == VK_SUCCESS);

   return VK_SUCCESS;
}

/**
 * Allocate count*size dwords, aligned to size dwords.
 * \a cs must be in TU_CS_MODE_SUB_STREAM mode.
 *
 */
VkResult
tu_cs_alloc(struct tu_cs *cs,
            uint32_t count,
            uint32_t size,
            struct tu_cs_memory *memory)
{
   assert(cs->mode == TU_CS_MODE_SUB_STREAM);
   assert(size && size <= 1024);

   if (!count) {
      /* If you allocated no memory, you'd better not use the iova for anything
       * (but it's left aligned for sanity).
       */
      memory->map = NULL;
      memory->iova = 0xdead0000;
      return VK_SUCCESS;
   }

   /* TODO: smarter way to deal with alignment? */

   VkResult result = tu_cs_reserve_space(cs, count * size + (size-1));
   if (result != VK_SUCCESS)
      return result;

   struct tu_bo *bo = tu_cs_current_bo(cs);
   size_t offset = align(tu_cs_get_offset(cs), size);

   memory->map = (uint32_t *) bo->map + offset;
   memory->iova = bo->iova + offset * sizeof(uint32_t);
   memory->writeable = cs->writeable;

   cs->start = cs->cur = (uint32_t*) bo->map + offset + count * size;

   return VK_SUCCESS;
}

/**
 * End command packet emission to a sub-stream.  \a sub_cs becomes invalid
 * after this call.
 *
 * Return an IB entry for the sub-stream.  The entry has the same lifetime as
 * \a cs.
 */
struct tu_cs_entry
tu_cs_end_sub_stream(struct tu_cs *cs, struct tu_cs *sub_cs)
{
   assert(cs->mode == TU_CS_MODE_SUB_STREAM);
   assert(sub_cs->start == cs->cur && sub_cs->end == cs->reserved_end);
   tu_cs_sanity_check(sub_cs);

   tu_cs_end(sub_cs);

   cs->cur = sub_cs->cur;

   struct tu_cs_entry entry = {
      .bo = tu_cs_current_bo(cs),
      .size = tu_cs_get_size(cs) * sizeof(uint32_t),
      .offset = tu_cs_get_offset(cs) * sizeof(uint32_t),
   };

   cs->start = cs->cur;

   return entry;
}

/**
 * Reserve space from a command stream for \a reserved_size uint32_t values.
 * This never fails when \a cs has mode TU_CS_MODE_EXTERNAL.
 */
VkResult
tu_cs_reserve_space(struct tu_cs *cs, uint32_t reserved_size)
{
   if (tu_cs_get_space(cs) < reserved_size) {
      if (cs->mode == TU_CS_MODE_EXTERNAL) {
         unreachable("cannot grow external buffer");
         return VK_ERROR_OUT_OF_HOST_MEMORY;
      }

      /* add an entry for the exiting command packets */
      if (!tu_cs_is_empty(cs)) {
         /* no direct command packet for TU_CS_MODE_SUB_STREAM */
         assert(cs->mode != TU_CS_MODE_SUB_STREAM);

         tu_cs_add_entry(cs);
      }

      for (uint32_t i = 0; i < cs->cond_stack_depth; i++) {
         /* Subtract one here to account for the DWORD field itself. */
         *cs->cond_dwords[i] = cs->cur - cs->cond_dwords[i] - 1;

         /* space for CP_COND_REG_EXEC in next bo */
         reserved_size += 3;
      }

      /* switch to a new BO */
      uint32_t new_size = MAX2(cs->next_bo_size, reserved_size);
      VkResult result = tu_cs_add_bo(cs, new_size);
      if (result != VK_SUCCESS)
         return result;

      if (cs->cond_stack_depth) {
         cs->reserved_end = cs->cur + reserved_size;
      }

      /* Re-emit CP_COND_REG_EXECs */
      for (uint32_t i = 0; i < cs->cond_stack_depth; i++) {
         tu_cs_emit_pkt7(cs, CP_COND_REG_EXEC, 2);
         tu_cs_emit(cs, cs->cond_flags[i]);

         cs->cond_dwords[i] = cs->cur;

         /* Emit dummy DWORD field here */
         tu_cs_emit(cs, RENDER_MODE_CP_COND_REG_EXEC_1_DWORDS(0));
      }

      /* double the size for the next bo, also there is an upper
       * bound on IB size, which appears to be 0x0fffff
       */
      new_size = MIN2(new_size << 1, 0x0fffff);
      if (cs->next_bo_size < new_size)
         cs->next_bo_size = new_size;
   }

   assert(tu_cs_get_space(cs) >= reserved_size);
   cs->reserved_end = cs->cur + reserved_size;

   if (cs->mode == TU_CS_MODE_GROW) {
      /* reserve an entry for the next call to this function or tu_cs_end */
      return tu_cs_reserve_entry(cs);
   }

   return VK_SUCCESS;
}

/**
 * Reset a command stream to its initial state.  This discards all comand
 * packets in \a cs, but does not necessarily release all resources.
 */
void
tu_cs_reset(struct tu_cs *cs)
{
   if (cs->mode == TU_CS_MODE_EXTERNAL) {
      assert(!cs->read_only.bo_count && !cs->read_write.bo_count &&
             !cs->refcount_bo && !cs->entry_count);
      cs->reserved_end = cs->cur = cs->start;
      return;
   }

   for (uint32_t i = 0; i + 1 < cs->read_only.bo_count; ++i) {
      tu_bo_finish(cs->device, cs->read_only.bos[i]);
   }

   for (uint32_t i = 0; i + 1 < cs->read_write.bo_count; ++i) {
      tu_bo_finish(cs->device, cs->read_write.bos[i]);
   }

   cs->writeable = false;

   if (cs->read_only.bo_count) {
      cs->read_only.bos[0] = cs->read_only.bos[cs->read_only.bo_count - 1];
      cs->read_only.bo_count = 1;

      cs->start = cs->cur = cs->reserved_end = (uint32_t *) cs->read_only.bos[0]->map;
      cs->end = cs->start + cs->read_only.bos[0]->size / sizeof(uint32_t);
   }

   if (cs->read_write.bo_count) {
      cs->read_write.bos[0] = cs->read_write.bos[cs->read_write.bo_count - 1];
      cs->read_write.bo_count = 1;
   }

   cs->entry_count = 0;
}

void
tu_cs_emit_debug_string(struct tu_cs *cs, const char *string, int len)
{
   assert(cs->mode == TU_CS_MODE_GROW);

   /* max packet size is 0x3fff dwords */
   len = MIN2(len, 0x3fff * 4);

   tu_cs_emit_pkt7(cs, CP_NOP, align(len, 4) / 4);
   const uint32_t *buf = (const uint32_t *) string;

   tu_cs_emit_array(cs, buf, len / 4);
   buf += len / 4;
   len = len % 4;

   /* copy remainder bytes without reading past end of input string */
   if (len > 0) {
      uint32_t w = 0;
      memcpy(&w, buf, len);
      tu_cs_emit(cs, w);
   }
}

void
tu_cs_emit_debug_magic_strv(struct tu_cs *cs,
                            uint32_t magic,
                            const char *fmt,
                            va_list args)
{
   int fmt_len = vsnprintf(NULL, 0, fmt, args);
   int len = 4 + fmt_len + 1;
   char *string = (char *) malloc(len);

   /* format: <magic><formatted string>\0 */
   *(uint32_t *) string = magic;
   vsnprintf(string + 4, fmt_len + 1, fmt, args);

   tu_cs_emit_debug_string(cs, string, len);
   free(string);
}

__attribute__((format(printf, 2, 3))) void
tu_cs_emit_debug_msg(struct tu_cs *cs, const char *fmt, ...)
{
   va_list args;
   va_start(args, fmt);
   tu_cs_emit_debug_magic_strv(cs, CP_NOP_MESG, fmt, args);
   va_end(args);
}

void
tu_cs_trace_start(struct u_trace_context *utctx,
                  void *cs,
                  const char *fmt,
                  ...)
{
   va_list args;
   va_start(args, fmt);
   tu_cs_emit_debug_magic_strv((struct tu_cs *) cs, CP_NOP_BEGN, fmt, args);
   va_end(args);
}

void
tu_cs_trace_end(struct u_trace_context *utctx, void *cs, const char *fmt, ...)
{
   va_list args;
   va_start(args, fmt);
   tu_cs_emit_debug_magic_strv((struct tu_cs *) cs, CP_NOP_END, fmt, args);
   va_end(args);
}
