/*
 * Copyright © 2022 Intel Corporation
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the next
 * paragraph) shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 */

#include <anv_private.h>

/* Sparse binding handling.
 *
 * There is one main structure passed around all over this file:
 *
 * - struct anv_sparse_binding_data: every resource (VkBuffer or VkImage) has
 *   a pointer to an instance of this structure. It contains the virtual
 *   memory address (VMA) used by the binding operations (which is different
 *   from the VMA used by the anv_bo it's bound to) and the VMA range size. We
 *   do not keep record of our our list of bindings (which ranges were bound
 *   to which buffers).
 */

__attribute__((format(printf, 1, 2)))
static void
sparse_debug(const char *format, ...)
{
   if (!INTEL_DEBUG(DEBUG_SPARSE))
      return;

   va_list args;
   va_start(args, format);
   vfprintf(stderr, format, args);
   va_end(args);
}

static void
dump_anv_vm_bind(struct anv_device *device,
                 const struct anv_vm_bind *bind)
{
  sparse_debug("[%s] ", bind->op == ANV_VM_BIND ? " bind " : "unbind");

   if (bind->bo)
      sparse_debug("bo:%04u ", bind->bo->gem_handle);
   else
      sparse_debug("bo:---- ");
   sparse_debug("address:%016"PRIx64" size:%08"PRIx64" "
                "mem_offset:%08"PRIx64"\n",
                bind->address, bind->size, bind->bo_offset);
}

static void
dump_vk_sparse_memory_bind(const VkSparseMemoryBind *bind)
{
   if (!INTEL_DEBUG(DEBUG_SPARSE))
      return;

   if (bind->memory != VK_NULL_HANDLE) {
      struct anv_bo *bo = anv_device_memory_from_handle(bind->memory)->bo;
      sparse_debug("bo:%04u ", bo->gem_handle);
   } else {
      sparse_debug("bo:---- ");
   }

   sparse_debug("res_offset:%08"PRIx64" size:%08"PRIx64" "
                "mem_offset:%08"PRIx64" flags:0x%08x\n",
                bind->resourceOffset, bind->size, bind->memoryOffset,
                bind->flags);
}

static void
dump_anv_image(struct anv_image *i)
{
   if (!INTEL_DEBUG(DEBUG_SPARSE))
      return;

   sparse_debug("anv_image:\n");
   sparse_debug("- format: %d\n", i->vk.format);
   sparse_debug("- extent: [%d, %d, %d]\n",
                i->vk.extent.width, i->vk.extent.height, i->vk.extent.depth);
   sparse_debug("- mip_levels: %d array_layers: %d samples: %d\n",
                i->vk.mip_levels, i->vk.array_layers, i->vk.samples);
   sparse_debug("- n_planes: %d\n", i->n_planes);
   sparse_debug("- disjoint: %d\n", i->disjoint);
}

static void
dump_isl_surf(struct isl_surf *s)
{
   if (!INTEL_DEBUG(DEBUG_SPARSE))
      return;

   sparse_debug("isl_surf:\n");

   const char *dim_s = s->dim == ISL_SURF_DIM_1D ? "1D" :
                       s->dim == ISL_SURF_DIM_2D ? "2D" :
                       s->dim == ISL_SURF_DIM_3D ? "3D" :
                       "(ERROR)";
   sparse_debug("- dim: %s\n", dim_s);
   sparse_debug("- tiling: %d (%s)\n", s->tiling,
                isl_tiling_to_name(s->tiling));
   sparse_debug("- format: %s\n", isl_format_get_short_name(s->format));
   sparse_debug("- image_alignment_el: [%d, %d, %d]\n",
                s->image_alignment_el.w, s->image_alignment_el.h,
                s->image_alignment_el.d);
   sparse_debug("- logical_level0_px: [%d, %d, %d, %d]\n",
                s->logical_level0_px.w,
                s->logical_level0_px.h,
                s->logical_level0_px.d,
                s->logical_level0_px.a);
   sparse_debug("- phys_level0_sa: [%d, %d, %d, %d]\n",
                s->phys_level0_sa.w,
                s->phys_level0_sa.h,
                s->phys_level0_sa.d,
                s->phys_level0_sa.a);
   sparse_debug("- levels: %d samples: %d\n", s->levels, s->samples);
   sparse_debug("- size_B: %"PRIu64" alignment_B: %u\n",
                s->size_B, s->alignment_B);
   sparse_debug("- row_pitch_B: %u\n", s->row_pitch_B);
   sparse_debug("- array_pitch_el_rows: %u\n", s->array_pitch_el_rows);

   const struct isl_format_layout *layout = isl_format_get_layout(s->format);
   sparse_debug("- format layout:\n");
   sparse_debug("  - format:%d bpb:%d bw:%d bh:%d bd:%d\n",
                layout->format, layout->bpb, layout->bw, layout->bh,
                layout->bd);

   struct isl_tile_info tile_info;
   isl_surf_get_tile_info(s, &tile_info);

   sparse_debug("- tile info:\n");
   sparse_debug("  - format_bpb: %d\n", tile_info.format_bpb);
   sparse_debug("  - logical_extent_el: [%d, %d, %d, %d]\n",
                tile_info.logical_extent_el.w,
                tile_info.logical_extent_el.h,
                tile_info.logical_extent_el.d,
                tile_info.logical_extent_el.a);
   sparse_debug("  - phys_extent_B: [%d, %d]\n",
                tile_info.phys_extent_B.w,
                tile_info.phys_extent_B.h);
}

static VkOffset3D
vk_offset3d_px_to_el(const VkOffset3D offset_px,
                     const struct isl_format_layout *layout)
{
   return (VkOffset3D) {
      .x = offset_px.x / layout->bw,
      .y = offset_px.y / layout->bh,
      .z = offset_px.z / layout->bd,
   };
}

static VkOffset3D
vk_offset3d_el_to_px(const VkOffset3D offset_el,
                     const struct isl_format_layout *layout)
{
   return (VkOffset3D) {
      .x = offset_el.x * layout->bw,
      .y = offset_el.y * layout->bh,
      .z = offset_el.z * layout->bd,
   };
}

static VkExtent3D
vk_extent3d_px_to_el(const VkExtent3D extent_px,
                     const struct isl_format_layout *layout)
{
   return (VkExtent3D) {
      .width = extent_px.width / layout->bw,
      .height = extent_px.height / layout->bh,
      .depth = extent_px.depth / layout->bd,
   };
}

static VkExtent3D
vk_extent3d_el_to_px(const VkExtent3D extent_el,
                     const struct isl_format_layout *layout)
{
   return (VkExtent3D) {
      .width = extent_el.width * layout->bw,
      .height = extent_el.height * layout->bh,
      .depth = extent_el.depth * layout->bd,
   };
}

static bool
isl_tiling_supports_standard_block_shapes(enum isl_tiling tiling)
{
   return tiling == ISL_TILING_64 ||
          tiling == ISL_TILING_ICL_Ys ||
          tiling == ISL_TILING_SKL_Ys;
}

static VkExtent3D
anv_sparse_get_standard_image_block_shape(enum isl_format format,
                                          VkImageType image_type,
                                          uint16_t texel_size)
{
   const struct isl_format_layout *layout = isl_format_get_layout(format);
   VkExtent3D block_shape = { .width = 0, .height = 0, .depth = 0 };

   switch (image_type) {
   case VK_IMAGE_TYPE_1D:
      /* 1D images don't have a standard block format. */
      assert(false);
      break;
   case VK_IMAGE_TYPE_2D:
      switch (texel_size) {
      case 8:
         block_shape = (VkExtent3D) { .width = 256, .height = 256, .depth = 1 };
         break;
      case 16:
         block_shape = (VkExtent3D) { .width = 256, .height = 128, .depth = 1 };
         break;
      case 32:
         block_shape = (VkExtent3D) { .width = 128, .height = 128, .depth = 1 };
         break;
      case 64:
         block_shape = (VkExtent3D) { .width = 128, .height = 64, .depth = 1 };
         break;
      case 128:
         block_shape = (VkExtent3D) { .width = 64, .height = 64, .depth = 1 };
         break;
      default:
         fprintf(stderr, "unexpected texel_size %d\n", texel_size);
         assert(false);
      }
      break;
   case VK_IMAGE_TYPE_3D:
      switch (texel_size) {
      case 8:
         block_shape = (VkExtent3D) { .width = 64, .height = 32, .depth = 32 };
         break;
      case 16:
         block_shape = (VkExtent3D) { .width = 32, .height = 32, .depth = 32 };
         break;
      case 32:
         block_shape = (VkExtent3D) { .width = 32, .height = 32, .depth = 16 };
         break;
      case 64:
         block_shape = (VkExtent3D) { .width = 32, .height = 16, .depth = 16 };
         break;
      case 128:
         block_shape = (VkExtent3D) { .width = 16, .height = 16, .depth = 16 };
         break;
      default:
         fprintf(stderr, "unexpected texel_size %d\n", texel_size);
         assert(false);
      }
      break;
   default:
      fprintf(stderr, "unexpected image_type %d\n", image_type);
      assert(false);
   }

   return vk_extent3d_el_to_px(block_shape, layout);
}

/* Adds "bind_op" to the list in "submit", while also trying to check if we
 * can just extend the last operation instead.
 */
static VkResult
anv_sparse_submission_add(struct anv_device *device,
                          struct anv_sparse_submission *submit,
                          struct anv_vm_bind *bind_op)
{
   struct anv_vm_bind *prev_bind = submit->binds_len == 0 ? NULL :
                                    &submit->binds[submit->binds_len - 1];

   if (prev_bind &&
       bind_op->op == prev_bind->op &&
       bind_op->bo == prev_bind->bo &&
       bind_op->address == prev_bind->address + prev_bind->size &&
       (bind_op->bo_offset == prev_bind->bo_offset + prev_bind->size ||
        prev_bind->bo == NULL)) {
      prev_bind->size += bind_op->size;
      return VK_SUCCESS;
   }

   if (submit->binds_len < submit->binds_capacity) {
      submit->binds[submit->binds_len++] = *bind_op;
      return VK_SUCCESS;
   }

   int new_capacity = MAX2(32, submit->binds_capacity * 2);
   struct anv_vm_bind *new_binds =
      vk_realloc(&device->vk.alloc, submit->binds,
                 new_capacity * sizeof(*new_binds), 8,
                 VK_SYSTEM_ALLOCATION_SCOPE_DEVICE);
   if (!new_binds)
      return vk_error(device, VK_ERROR_OUT_OF_HOST_MEMORY);

   new_binds[submit->binds_len] = *bind_op;

   submit->binds = new_binds;
   submit->binds_len++;
   submit->binds_capacity = new_capacity;

   return VK_SUCCESS;
}

/* We really want to try to have all the page tables on as few BOs as possible
 * to benefit from cache locality and to keep the i915.ko relocation lists
 * small. On the other hand, we don't want to waste memory on unused space.
 */
#define ANV_TRTT_PAGE_TABLE_BO_SIZE (2 * 1024 * 1024)

static VkResult
trtt_make_page_table_bo(struct anv_device *device, struct anv_bo **bo)
{
   VkResult result;
   struct anv_trtt *trtt = &device->trtt;

   result = anv_device_alloc_bo(device, "trtt-page-table",
                                ANV_TRTT_PAGE_TABLE_BO_SIZE, 0, 0, bo);
   if (result != VK_SUCCESS)
      return result;

   if (trtt->num_page_table_bos < trtt->page_table_bos_capacity) {
      trtt->page_table_bos[trtt->num_page_table_bos++] = *bo;
   } else {

      int new_capacity = MAX2(8, trtt->page_table_bos_capacity * 2);
      struct anv_bo **new_page_table_bos =
         vk_realloc(&device->vk.alloc, trtt->page_table_bos,
                    new_capacity * sizeof(*trtt->page_table_bos), 8,
                    VK_SYSTEM_ALLOCATION_SCOPE_DEVICE);
      if (!new_page_table_bos) {
         anv_device_release_bo(device, *bo);
         return vk_error(device, VK_ERROR_OUT_OF_HOST_MEMORY);
      }

      new_page_table_bos[trtt->num_page_table_bos] = *bo;

      trtt->page_table_bos = new_page_table_bos;
      trtt->page_table_bos_capacity = new_capacity;
      trtt->num_page_table_bos++;
   }

   trtt->cur_page_table_bo = *bo;
   trtt->next_page_table_bo_offset = 0;

   sparse_debug("new number of page table BOs: %d\n",
                trtt->num_page_table_bos);

   return VK_SUCCESS;
}

static VkResult
trtt_get_page_table_bo(struct anv_device *device, struct anv_bo **bo,
                       uint64_t *bo_addr)
{
   struct anv_trtt *trtt = &device->trtt;
   VkResult result;

   if (!trtt->cur_page_table_bo) {
      result = trtt_make_page_table_bo(device, bo);
      if (result != VK_SUCCESS)
         return result;
   }

   *bo = trtt->cur_page_table_bo;
   *bo_addr = trtt->cur_page_table_bo->offset +
              trtt->next_page_table_bo_offset;

   trtt->next_page_table_bo_offset += 4096;
   if (trtt->next_page_table_bo_offset >= ANV_TRTT_PAGE_TABLE_BO_SIZE)
      trtt->cur_page_table_bo = NULL;

   return VK_SUCCESS;
}

static VkResult
anv_trtt_init_context_state(struct anv_queue *queue)
{
   struct anv_device *device = queue->device;
   struct anv_trtt *trtt = &device->trtt;

   struct drm_syncobj_create create = {
      .handle = 0,
      .flags = 0,
   };
   if (intel_ioctl(device->fd, DRM_IOCTL_SYNCOBJ_CREATE, &create))
      return vk_error(device, VK_ERROR_OUT_OF_HOST_MEMORY);
   assert(create.handle != 0);
   trtt->timeline_handle = create.handle;

   struct anv_bo *l3_bo;
   VkResult result = trtt_get_page_table_bo(device, &l3_bo, &trtt->l3_addr);
   if (result != VK_SUCCESS)
      return result;

   trtt->l3_mirror = vk_zalloc(&device->vk.alloc, 4096, 8,
                                VK_SYSTEM_ALLOCATION_SCOPE_DEVICE);
   if (!trtt->l3_mirror) {
      result = vk_error(device, VK_ERROR_OUT_OF_HOST_MEMORY);
      return result;
   }

   /* L3 has 512 entries, so we can have up to 512 L2 tables. */
   trtt->l2_mirror = vk_zalloc(&device->vk.alloc, 512 * 4096, 8,
                               VK_SYSTEM_ALLOCATION_SCOPE_DEVICE);
   if (!trtt->l2_mirror) {
      result = vk_error(device, VK_ERROR_OUT_OF_HOST_MEMORY);
      goto fail_free_l3;
   }

   result = anv_genX(device->info, init_trtt_context_state)(queue);

   return result;

fail_free_l3:
   vk_free(&device->vk.alloc, trtt->l3_mirror);
   return result;
}

static void
anv_trtt_bind_list_add_entry(struct anv_trtt_bind *binds, int *binds_len,
                             uint64_t pte_addr, uint64_t entry_addr)
{
   binds[*binds_len] = (struct anv_trtt_bind) {
      .pte_addr = pte_addr,
      .entry_addr = entry_addr,
   };
   (*binds_len)++;
}

/* For L3 and L2 pages, null and invalid entries are indicated by bits 1 and 0
 * respectively. For L1 entries, the hardware compares the addresses against
 * what we program to the GFX_TRTT_NULL and GFX_TRTT_INVAL registers.
 */
#define ANV_TRTT_L3L2_NULL_ENTRY (1 << 1)
#define ANV_TRTT_L3L2_INVALID_ENTRY (1 << 0)

/* Adds elements to the anv_trtt_bind structs passed. This doesn't write the
 * entries to the HW yet.
 */
static VkResult
anv_trtt_bind_add(struct anv_device *device,
                  uint64_t trtt_addr, uint64_t dest_addr,
                  struct anv_trtt_submission *s)
{
   VkResult result = VK_SUCCESS;
   struct anv_trtt *trtt = &device->trtt;
   bool is_null_bind = dest_addr == ANV_TRTT_L1_NULL_TILE_VAL;

   int l3_index = (trtt_addr >> 35) & 0x1FF;
   int l2_index = (trtt_addr >> 26) & 0x1FF;
   int l1_index = (trtt_addr >> 16) & 0x3FF;

   uint64_t l2_addr = trtt->l3_mirror[l3_index];
   if (l2_addr == ANV_TRTT_L3L2_NULL_ENTRY && is_null_bind) {
      return VK_SUCCESS;
   } else if (l2_addr == 0 || l2_addr == ANV_TRTT_L3L2_NULL_ENTRY) {
      if (is_null_bind) {
         trtt->l3_mirror[l3_index] = ANV_TRTT_L3L2_NULL_ENTRY;

         anv_trtt_bind_list_add_entry(s->l3l2_binds, &s->l3l2_binds_len,
               trtt->l3_addr + l3_index * sizeof(uint64_t),
               ANV_TRTT_L3L2_NULL_ENTRY);

         return VK_SUCCESS;
      }

      struct anv_bo *l2_bo;
      result = trtt_get_page_table_bo(device, &l2_bo, &l2_addr);
      if (result != VK_SUCCESS)
         return result;

      trtt->l3_mirror[l3_index] = l2_addr;

      anv_trtt_bind_list_add_entry(s->l3l2_binds, &s->l3l2_binds_len,
            trtt->l3_addr + l3_index * sizeof(uint64_t), l2_addr);
   }
   assert(l2_addr != 0 && l2_addr != ANV_TRTT_L3L2_NULL_ENTRY);

   /* The first page in the l2_mirror corresponds to l3_index=0 and so on. */
   uint64_t l1_addr = trtt->l2_mirror[l3_index * 512 + l2_index];
   if (l1_addr == ANV_TRTT_L3L2_NULL_ENTRY && is_null_bind) {
      return VK_SUCCESS;
   } else if (l1_addr == 0 || l1_addr == ANV_TRTT_L3L2_NULL_ENTRY) {
      if (is_null_bind) {
         trtt->l2_mirror[l3_index * 512 + l2_index] =
            ANV_TRTT_L3L2_NULL_ENTRY;

         anv_trtt_bind_list_add_entry(s->l3l2_binds, &s->l3l2_binds_len,
               l2_addr + l2_index * sizeof(uint64_t),
               ANV_TRTT_L3L2_NULL_ENTRY);

         return VK_SUCCESS;
      }

      struct anv_bo *l1_bo;
      result = trtt_get_page_table_bo(device, &l1_bo, &l1_addr);
      if (result != VK_SUCCESS)
         return result;

      trtt->l2_mirror[l3_index * 512 + l2_index] = l1_addr;

      anv_trtt_bind_list_add_entry(s->l3l2_binds, &s->l3l2_binds_len,
            l2_addr + l2_index * sizeof(uint64_t), l1_addr);
   }
   assert(l1_addr != 0 && l1_addr != ANV_TRTT_L3L2_NULL_ENTRY);

   anv_trtt_bind_list_add_entry(s->l1_binds, &s->l1_binds_len,
            l1_addr + l1_index * sizeof(uint32_t), dest_addr);

   return VK_SUCCESS;
}

static VkResult
anv_sparse_bind_trtt(struct anv_device *device,
                     struct anv_sparse_submission *sparse_submit)
{
   struct anv_trtt *trtt = &device->trtt;
   VkResult result;

   /* TR-TT submission needs a queue even when the API entry point doesn't
    * give one, such as resource creation. */
   if (!sparse_submit->queue)
      sparse_submit->queue = trtt->queue;

   /* These capacities are conservative estimations. For L1 binds the
    * number will match exactly unless we skip NULL binds due to L2 already
    * being NULL. For L3/L2 things are harder to estimate, but the resulting
    * numbers are so small that a little overestimation won't hurt.
    *
    * We have assertions below to catch estimation errors.
    */
   int l3l2_binds_capacity = 1;
   int l1_binds_capacity = 0;
   for (int b = 0; b < sparse_submit->binds_len; b++) {
      assert(sparse_submit->binds[b].size % (64 * 1024) == 0);
      int pages = sparse_submit->binds[b].size / (64 * 1024);
      l1_binds_capacity += pages;
      l3l2_binds_capacity += (pages / 1024 + 1) * 2;
   }

   STACK_ARRAY(struct anv_trtt_bind, l3l2_binds, l3l2_binds_capacity);
   STACK_ARRAY(struct anv_trtt_bind, l1_binds, l1_binds_capacity);
   struct anv_trtt_submission trtt_submit = {
      .sparse = sparse_submit,
      .l3l2_binds = l3l2_binds,
      .l1_binds = l1_binds,
      .l3l2_binds_len = 0,
      .l1_binds_len = 0,
   };

   pthread_mutex_lock(&trtt->mutex);

   if (!trtt->l3_addr)
      anv_trtt_init_context_state(sparse_submit->queue);

   assert(trtt->l3_addr);

   for (int b = 0; b < sparse_submit->binds_len; b++) {
      struct anv_vm_bind *vm_bind = &sparse_submit->binds[b];
      for (size_t i = 0; i < vm_bind->size; i += 64 * 1024) {
         uint64_t trtt_addr = vm_bind->address + i;
         uint64_t dest_addr =
            (vm_bind->op == ANV_VM_BIND && vm_bind->bo) ?
               vm_bind->bo->offset + vm_bind->bo_offset + i :
               ANV_TRTT_L1_NULL_TILE_VAL;

         result = anv_trtt_bind_add(device, trtt_addr, dest_addr,
                                    &trtt_submit);
         if (result != VK_SUCCESS)
            goto out;
      }
   }

   assert(trtt_submit.l3l2_binds_len <= l3l2_binds_capacity);
   assert(trtt_submit.l1_binds_len <= l1_binds_capacity);

   sparse_debug("trtt_binds: num_vm_binds:%02d l3l2:%04d l1:%04d\n",
                sparse_submit->binds_len, trtt_submit.l3l2_binds_len,
                trtt_submit.l1_binds_len);

   if (trtt_submit.l3l2_binds_len || trtt_submit.l1_binds_len)
      result = anv_genX(device->info, write_trtt_entries)(&trtt_submit);

out:
   pthread_mutex_unlock(&trtt->mutex);
   STACK_ARRAY_FINISH(l1_binds);
   STACK_ARRAY_FINISH(l3l2_binds);
   return result;
}

static VkResult
anv_sparse_bind_vm_bind(struct anv_device *device,
                        struct anv_sparse_submission *submit)
{
   struct anv_queue *queue = submit->queue;
   VkResult result;

   if (!queue)
      assert(submit->wait_count == 0 && submit->signal_count == 0);

   /* TODO: make both the syncs and signals be passed as part of the vm_bind
    * ioctl so they can be waited asynchronously. For now this doesn't matter
    * as we're doing synchronous vm_bind, but later when we make it async this
    * will make a difference.
    */
   result = vk_sync_wait_many(&device->vk, submit->wait_count, submit->waits,
                              VK_SYNC_WAIT_COMPLETE, INT64_MAX);
   if (result != VK_SUCCESS)
      return vk_queue_set_lost(&queue->vk, "vk_sync_wait failed");

   /* FIXME: here we were supposed to issue a single vm_bind ioctl by calling
    * vm_bind(device, num_binds, binds), but for an unknown reason some
    * shader-related tests fail when we do that, so work around it for now.
    * See: https://gitlab.freedesktop.org/drm/xe/kernel/-/issues/746
    */
   for (int b = 0; b < submit->binds_len; b++) {
      struct anv_sparse_submission s = {
         .queue = submit->queue,
         .binds = &submit->binds[b],
         .binds_len = 1,
         .binds_capacity = 1,
         .wait_count = 0,
         .signal_count = 0,
      };
      int rc = device->kmd_backend->vm_bind(device, &s);
      if (rc)
         return vk_error(device, VK_ERROR_OUT_OF_DEVICE_MEMORY);
   }

   for (uint32_t i = 0; i < submit->signal_count; i++) {
      struct vk_sync_signal *s = &submit->signals[i];
      result = vk_sync_signal(&device->vk, s->sync, s->signal_value);
      if (result != VK_SUCCESS)
         return vk_queue_set_lost(&queue->vk, "vk_sync_signal failed");
   }

   return VK_SUCCESS;
}

VkResult
anv_sparse_bind(struct anv_device *device,
                struct anv_sparse_submission *submit)
{
   if (INTEL_DEBUG(DEBUG_SPARSE)) {
      for (int b = 0; b < submit->binds_len; b++)
         dump_anv_vm_bind(device, &submit->binds[b]);
   }

   return device->physical->sparse_uses_trtt ?
            anv_sparse_bind_trtt(device, submit) :
            anv_sparse_bind_vm_bind(device, submit);
}

VkResult
anv_init_sparse_bindings(struct anv_device *device,
                         uint64_t size_,
                         struct anv_sparse_binding_data *sparse,
                         enum anv_bo_alloc_flags alloc_flags,
                         uint64_t client_address,
                         struct anv_address *out_address)
{
   uint64_t size = align64(size_, ANV_SPARSE_BLOCK_SIZE);

   if (device->physical->sparse_uses_trtt)
      alloc_flags |= ANV_BO_ALLOC_TRTT;

   sparse->address = anv_vma_alloc(device, size, ANV_SPARSE_BLOCK_SIZE,
                                   alloc_flags,
                                   intel_48b_address(client_address),
                                   &sparse->vma_heap);
   sparse->size = size;

   out_address->bo = NULL;
   out_address->offset = sparse->address;

   struct anv_vm_bind bind = {
      .bo = NULL, /* That's a NULL binding. */
      .address = sparse->address,
      .bo_offset = 0,
      .size = size,
      .op = ANV_VM_BIND,
   };
   struct anv_sparse_submission submit = {
      .queue = NULL,
      .binds = &bind,
      .binds_len = 1,
      .binds_capacity = 1,
      .wait_count = 0,
      .signal_count = 0,
   };
   VkResult res = anv_sparse_bind(device, &submit);
   if (res != VK_SUCCESS) {
      anv_vma_free(device, sparse->vma_heap, sparse->address, sparse->size);
      return res;
   }

   return VK_SUCCESS;
}

VkResult
anv_free_sparse_bindings(struct anv_device *device,
                         struct anv_sparse_binding_data *sparse)
{
   if (!sparse->address)
      return VK_SUCCESS;

   sparse_debug("%s: address:0x%016"PRIx64" size:0x%08"PRIx64"\n",
                __func__, sparse->address, sparse->size);

   struct anv_vm_bind unbind = {
      .bo = 0,
      .address = sparse->address,
      .bo_offset = 0,
      .size = sparse->size,
      .op = ANV_VM_UNBIND,
   };
   struct anv_sparse_submission submit = {
      .queue = NULL,
      .binds = &unbind,
      .binds_len = 1,
      .binds_capacity = 1,
      .wait_count = 0,
      .signal_count = 0,
   };
   VkResult res = anv_sparse_bind(device, &submit);
   if (res != VK_SUCCESS)
      return res;

   anv_vma_free(device, sparse->vma_heap, sparse->address, sparse->size);

   return VK_SUCCESS;
}

static VkExtent3D
anv_sparse_calc_block_shape(struct anv_physical_device *pdevice,
                            struct isl_surf *surf)
{
   const struct isl_format_layout *layout =
      isl_format_get_layout(surf->format);
   const int Bpb = layout->bpb / 8;

   struct isl_tile_info tile_info;
   isl_surf_get_tile_info(surf, &tile_info);

   VkExtent3D block_shape_el = {
      .width = tile_info.logical_extent_el.width,
      .height = tile_info.logical_extent_el.height,
      .depth = tile_info.logical_extent_el.depth,
   };
   VkExtent3D block_shape_px = vk_extent3d_el_to_px(block_shape_el, layout);

   if (surf->tiling == ISL_TILING_LINEAR) {
      uint32_t elements_per_row = surf->row_pitch_B /
                                  (block_shape_el.width * Bpb);
      uint32_t rows_per_tile = ANV_SPARSE_BLOCK_SIZE /
                               (elements_per_row * Bpb);
      assert(rows_per_tile * elements_per_row * Bpb == ANV_SPARSE_BLOCK_SIZE);

      block_shape_px = (VkExtent3D) {
         .width = elements_per_row * layout->bw,
         .height = rows_per_tile * layout->bh,
         .depth = layout->bd,
      };
   }

   return block_shape_px;
}

VkSparseImageFormatProperties
anv_sparse_calc_image_format_properties(struct anv_physical_device *pdevice,
                                        VkImageAspectFlags aspect,
                                        VkImageType vk_image_type,
                                        struct isl_surf *surf)
{
   const struct isl_format_layout *isl_layout =
      isl_format_get_layout(surf->format);
   const int bpb = isl_layout->bpb;
   assert(bpb == 8 || bpb == 16 || bpb == 32 || bpb == 64 ||bpb == 128);
   const int Bpb = bpb / 8;

   VkExtent3D granularity = anv_sparse_calc_block_shape(pdevice, surf);
   bool is_standard = false;
   bool is_known_nonstandard_format = false;

   if (vk_image_type != VK_IMAGE_TYPE_1D) {
      VkExtent3D std_shape =
         anv_sparse_get_standard_image_block_shape(surf->format, vk_image_type,
                                                   bpb);
      /* YUV formats don't work with Tile64, which is required if we want to
       * claim standard block shapes. The spec requires us to support all
       * non-compressed color formats that non-sparse supports, so we can't
       * just say YUV formats are not supported by Sparse. So we end
       * supporting this format and anv_sparse_calc_miptail_properties() will
       * say that everything is part of the miptail.
       *
       * For more details on the hardware restriction, please check
       * isl_gfx125_filter_tiling().
       */
      if (pdevice->info.verx10 >= 125 && isl_format_is_yuv(surf->format))
         is_known_nonstandard_format = true;

      /* The standard block shapes (and by extension, the tiling formats they
       * require) are simply incompatible with getting a 2D view of a 3D
       * image.
       */
      if (surf->usage & ISL_SURF_USAGE_2D_3D_COMPATIBLE_BIT)
         is_known_nonstandard_format = true;

      is_standard = granularity.width == std_shape.width &&
                    granularity.height == std_shape.height &&
                    granularity.depth == std_shape.depth;

      /* TODO: dEQP seems to care about the block shapes being standard even
       * for the cases where is_known_nonstandard_format is true. Luckily as
       * of today all of those cases are NotSupported but sooner or later we
       * may end up getting a failure.
       * Notice that in practice we report these cases as having the mip tail
       * starting on mip level 0, so the reported block shapes are irrelevant
       * since non-opaque binds are not supported. Still, dEQP seems to care.
       */
      assert(is_standard || is_known_nonstandard_format);
   }

   uint32_t block_size = granularity.width * granularity.height *
                         granularity.depth * Bpb;
   bool wrong_block_size = block_size != ANV_SPARSE_BLOCK_SIZE;

   return (VkSparseImageFormatProperties) {
      .aspectMask = aspect,
      .imageGranularity = granularity,
      .flags = ((is_standard || is_known_nonstandard_format) ? 0 :
                  VK_SPARSE_IMAGE_FORMAT_NONSTANDARD_BLOCK_SIZE_BIT) |
               (wrong_block_size ? VK_SPARSE_IMAGE_FORMAT_SINGLE_MIPTAIL_BIT :
                  0),
   };
}

/* The miptail is supposed to be this region where the tiniest mip levels
 * are squished together in one single page, which should save us some memory.
 * It's a hardware feature which our hardware supports on certain tiling
 * formats - the ones we always want to use for sparse resources.
 *
 * For sparse, the main feature of the miptail is that it only supports opaque
 * binds, so you either bind the whole miptail or you bind nothing at all,
 * there are no subresources inside it to separately bind. While the idea is
 * that the miptail as reported by sparse should match what our hardware does,
 * in practice we can say in our sparse functions that certain mip levels are
 * part of the miptail while from the point of view of our hardwared they
 * aren't.
 *
 * If we detect we're using the sparse-friendly tiling formats and ISL
 * supports miptails for them, we can just trust the miptail level set by ISL
 * and things can proceed as The Spec intended.
 *
 * However, if that's not the case, we have to go on a best-effort policy. We
 * could simply declare that every mip level is part of the miptail and be
 * done, but since that kinda defeats the purpose of Sparse we try to find
 * what level we really should be reporting as the first miptail level based
 * on the alignments of the surface subresources.
 */
void
anv_sparse_calc_miptail_properties(struct anv_device *device,
                                   struct anv_image *image,
                                   VkImageAspectFlags vk_aspect,
                                   uint32_t *imageMipTailFirstLod,
                                   VkDeviceSize *imageMipTailSize,
                                   VkDeviceSize *imageMipTailOffset,
                                   VkDeviceSize *imageMipTailStride)
{
   assert(__builtin_popcount(vk_aspect) == 1);
   const uint32_t plane = anv_image_aspect_to_plane(image, vk_aspect);
   struct isl_surf *surf = &image->planes[plane].primary_surface.isl;
   uint64_t binding_plane_offset =
      image->planes[plane].primary_surface.memory_range.offset;
   const struct isl_format_layout *isl_layout =
      isl_format_get_layout(surf->format);
   const int Bpb = isl_layout->bpb / 8;
   struct isl_tile_info tile_info;
   isl_surf_get_tile_info(surf, &tile_info);
   uint32_t tile_size = tile_info.logical_extent_el.width * Bpb *
                        tile_info.logical_extent_el.height *
                        tile_info.logical_extent_el.depth;

   uint64_t layer1_offset;
   uint32_t x_off, y_off;

   /* Treat the whole thing as a single miptail. We should have already
    * reported this image as VK_SPARSE_IMAGE_FORMAT_SINGLE_MIPTAIL_BIT.
    *
    * In theory we could try to make ISL massage the alignments so that we
    * could at least claim mip level 0 to be not part of the miptail, but
    * that could end up wasting a lot of memory, so it's better to do
    * nothing and focus our efforts into making things use the appropriate
    * tiling formats that give us the standard block shapes.
    */
   if (tile_size != ANV_SPARSE_BLOCK_SIZE)
      goto out_everything_is_miptail;

   assert(surf->tiling != ISL_TILING_LINEAR);

   if (image->vk.array_layers == 1) {
      layer1_offset = surf->size_B;
   } else {
      isl_surf_get_image_offset_B_tile_sa(surf, 0, 1, 0, &layer1_offset,
                                          &x_off, &y_off);
      if (x_off || y_off)
         goto out_everything_is_miptail;
   }
   assert(layer1_offset % tile_size == 0);

   /* We could try to do better here, but there's not really any point since
    * we should be supporting the appropriate tiling formats everywhere.
    */
   if (!isl_tiling_supports_standard_block_shapes(surf->tiling))
      goto out_everything_is_miptail;

   int miptail_first_level = surf->miptail_start_level;
   if (miptail_first_level >= image->vk.mip_levels)
      goto out_no_miptail;

   uint64_t miptail_offset = 0;
   isl_surf_get_image_offset_B_tile_sa(surf, miptail_first_level, 0, 0,
                                       &miptail_offset,
                                       &x_off, &y_off);
   assert(x_off == 0 && y_off == 0);
   assert(miptail_offset % tile_size == 0);

   *imageMipTailFirstLod = miptail_first_level;
   *imageMipTailSize = tile_size;
   *imageMipTailOffset = binding_plane_offset + miptail_offset;
   *imageMipTailStride = layer1_offset;
   goto out_debug;

out_no_miptail:
   *imageMipTailFirstLod = image->vk.mip_levels;
   *imageMipTailSize = 0;
   *imageMipTailOffset = 0;
   *imageMipTailStride = 0;
   goto out_debug;

out_everything_is_miptail:
   *imageMipTailFirstLod = 0;
   *imageMipTailSize = surf->size_B;
   *imageMipTailOffset = binding_plane_offset;
   *imageMipTailStride = 0;

out_debug:
   sparse_debug("miptail first_lod:%d size:%"PRIu64" offset:%"PRIu64" "
                "stride:%"PRIu64"\n",
                *imageMipTailFirstLod, *imageMipTailSize,
                *imageMipTailOffset, *imageMipTailStride);
}

static struct anv_vm_bind
vk_bind_to_anv_vm_bind(struct anv_sparse_binding_data *sparse,
                       const struct VkSparseMemoryBind *vk_bind)
{
   struct anv_vm_bind anv_bind = {
      .bo = NULL,
      .address = sparse->address + vk_bind->resourceOffset,
      .bo_offset = 0,
      .size = vk_bind->size,
      .op = ANV_VM_BIND,
   };

   assert(vk_bind->size);
   assert(vk_bind->resourceOffset + vk_bind->size <= sparse->size);

   if (vk_bind->memory != VK_NULL_HANDLE) {
      anv_bind.bo = anv_device_memory_from_handle(vk_bind->memory)->bo;
      anv_bind.bo_offset = vk_bind->memoryOffset,
      assert(vk_bind->memoryOffset + vk_bind->size <= anv_bind.bo->size);
   }

   return anv_bind;
}

VkResult
anv_sparse_bind_resource_memory(struct anv_device *device,
                                struct anv_sparse_binding_data *sparse,
                                const VkSparseMemoryBind *vk_bind,
                                struct anv_sparse_submission *submit)
{
   struct anv_vm_bind bind = vk_bind_to_anv_vm_bind(sparse, vk_bind);

   if (vk_bind->size % ANV_SPARSE_BLOCK_SIZE != 0)
      return vk_error(device, VK_ERROR_VALIDATION_FAILED_EXT);

   return anv_sparse_submission_add(device, submit, &bind);
}

VkResult
anv_sparse_bind_image_memory(struct anv_queue *queue,
                             struct anv_image *image,
                             const VkSparseImageMemoryBind *bind,
                             struct anv_sparse_submission *submit)
{
   struct anv_device *device = queue->device;
   VkImageAspectFlags aspect = bind->subresource.aspectMask;
   uint32_t mip_level = bind->subresource.mipLevel;
   uint32_t array_layer = bind->subresource.arrayLayer;

   assert(__builtin_popcount(aspect) == 1);
   assert(!(bind->flags & VK_SPARSE_MEMORY_BIND_METADATA_BIT));

   struct anv_image_binding *img_binding = image->disjoint ?
      anv_image_aspect_to_binding(image, aspect) :
      &image->bindings[ANV_IMAGE_MEMORY_BINDING_MAIN];
   struct anv_sparse_binding_data *sparse_data = &img_binding->sparse_data;

   const uint32_t plane = anv_image_aspect_to_plane(image, aspect);
   struct isl_surf *surf = &image->planes[plane].primary_surface.isl;
   uint64_t binding_plane_offset =
      image->planes[plane].primary_surface.memory_range.offset;
   const struct isl_format_layout *layout =
      isl_format_get_layout(surf->format);
   struct isl_tile_info tile_info;
   isl_surf_get_tile_info(surf, &tile_info);

   sparse_debug("\n=== [%s:%d] [%s] BEGIN\n", __FILE__, __LINE__, __func__);
   sparse_debug("--> mip_level:%d array_layer:%d\n",
                mip_level, array_layer);
   sparse_debug("aspect:0x%x plane:%d\n", aspect, plane);
   sparse_debug("binding offset: [%d, %d, %d] extent: [%d, %d, %d]\n",
                bind->offset.x, bind->offset.y, bind->offset.z,
                bind->extent.width, bind->extent.height, bind->extent.depth);
   dump_anv_image(image);
   dump_isl_surf(surf);

   VkExtent3D block_shape_px =
      anv_sparse_calc_block_shape(device->physical, surf);
   VkExtent3D block_shape_el = vk_extent3d_px_to_el(block_shape_px, layout);

   /* Both bind->offset and bind->extent are in pixel units. */
   VkOffset3D bind_offset_el = vk_offset3d_px_to_el(bind->offset, layout);

   /* The spec says we only really need to align if for a given coordinate
    * offset + extent equals the corresponding dimensions of the image
    * subresource, but all the other non-aligned usage is invalid, so just
    * align everything.
    */
   VkExtent3D bind_extent_px = {
      .width = ALIGN_NPOT(bind->extent.width, block_shape_px.width),
      .height = ALIGN_NPOT(bind->extent.height, block_shape_px.height),
      .depth = ALIGN_NPOT(bind->extent.depth, block_shape_px.depth),
   };
   VkExtent3D bind_extent_el = vk_extent3d_px_to_el(bind_extent_px, layout);

   /* A sparse block should correspond to our tile size, so this has to be
    * either 4k or 64k depending on the tiling format. */
   const uint64_t block_size_B = block_shape_el.width * (layout->bpb / 8) *
                                 block_shape_el.height *
                                 block_shape_el.depth;
   /* How many blocks are necessary to form a whole line on this image? */
   const uint32_t blocks_per_line = surf->row_pitch_B / (layout->bpb / 8) /
                                    block_shape_el.width;
   /* The loop below will try to bind a whole line of blocks at a time as
    * they're guaranteed to be contiguous, so we calculate how many blocks
    * that is and how big is each block to figure the bind size of a whole
    * line.
    */
   uint64_t line_bind_size_in_blocks = bind_extent_el.width /
                                       block_shape_el.width;
   uint64_t line_bind_size = line_bind_size_in_blocks * block_size_B;
   assert(line_bind_size_in_blocks != 0);
   assert(line_bind_size != 0);

   uint64_t memory_offset = bind->memoryOffset;
   for (uint32_t z = bind_offset_el.z;
        z < bind_offset_el.z + bind_extent_el.depth;
        z += block_shape_el.depth) {
      uint64_t subresource_offset_B;
      uint32_t subresource_x_offset, subresource_y_offset;
      isl_surf_get_image_offset_B_tile_sa(surf, mip_level, array_layer, z,
                                          &subresource_offset_B,
                                          &subresource_x_offset,
                                          &subresource_y_offset);
      assert(subresource_x_offset == 0 && subresource_y_offset == 0);
      assert(subresource_offset_B % block_size_B == 0);

      for (uint32_t y = bind_offset_el.y;
           y < bind_offset_el.y + bind_extent_el.height;
           y+= block_shape_el.height) {
         uint32_t line_block_offset = y / block_shape_el.height *
                                      blocks_per_line;
         uint64_t line_start_B = subresource_offset_B +
                                 line_block_offset * block_size_B;
         uint64_t bind_offset_B = line_start_B +
                                  (bind_offset_el.x / block_shape_el.width) *
                                  block_size_B;

         VkSparseMemoryBind opaque_bind = {
            .resourceOffset = binding_plane_offset + bind_offset_B,
            .size = line_bind_size,
            .memory = bind->memory,
            .memoryOffset = memory_offset,
            .flags = bind->flags,
         };

         memory_offset += line_bind_size;

         assert(line_start_B % block_size_B == 0);
         assert(opaque_bind.resourceOffset % block_size_B == 0);
         assert(opaque_bind.size % block_size_B == 0);

         struct anv_vm_bind anv_bind = vk_bind_to_anv_vm_bind(sparse_data,
                                                              &opaque_bind);
         VkResult result = anv_sparse_submission_add(device, submit,
                                                     &anv_bind);
         if (result != VK_SUCCESS)
            return result;
      }
   }

   sparse_debug("\n=== [%s:%d] [%s] END\n", __FILE__, __LINE__, __func__);
   return VK_SUCCESS;
}

VkResult
anv_sparse_image_check_support(struct anv_physical_device *pdevice,
                               VkImageCreateFlags flags,
                               VkImageTiling tiling,
                               VkSampleCountFlagBits samples,
                               VkImageType type,
                               VkFormat vk_format)
{
   assert(flags & VK_IMAGE_CREATE_SPARSE_BINDING_BIT);

   /* The spec says:
    *   "A sparse image created using VK_IMAGE_CREATE_SPARSE_BINDING_BIT (but
    *    not VK_IMAGE_CREATE_SPARSE_RESIDENCY_BIT) supports all formats that
    *    non-sparse usage supports, and supports both VK_IMAGE_TILING_OPTIMAL
    *    and VK_IMAGE_TILING_LINEAR tiling."
    */
   if (!(flags & VK_IMAGE_CREATE_SPARSE_RESIDENCY_BIT))
      return VK_SUCCESS;

   /* From here on, these are the rules:
    *   "A sparse image created using VK_IMAGE_CREATE_SPARSE_RESIDENCY_BIT
    *    supports all non-compressed color formats with power-of-two element
    *    size that non-sparse usage supports. Additional formats may also be
    *    supported and can be queried via
    *    vkGetPhysicalDeviceSparseImageFormatProperties.
    *    VK_IMAGE_TILING_LINEAR tiling is not supported."
    */

   /* We choose not to support sparse residency on emulated compressed
    * formats due to the additional image plane. It would make the
    * implementation extremely complicated.
    */
   if (anv_is_format_emulated(pdevice, vk_format))
      return VK_ERROR_FORMAT_NOT_SUPPORTED;

   /* While the spec itself says linear is not supported (see above), deqp-vk
    * tries anyway to create linear sparse images, so we have to check for it.
    * This is also said in VUID-VkImageCreateInfo-tiling-04121:
    *   "If tiling is VK_IMAGE_TILING_LINEAR, flags must not contain
    *    VK_IMAGE_CREATE_SPARSE_RESIDENCY_BIT"
    */
   if (tiling == VK_IMAGE_TILING_LINEAR)
      return VK_ERROR_FORMAT_NOT_SUPPORTED;

   /* TODO: not supported yet. */
   if (samples != VK_SAMPLE_COUNT_1_BIT)
      return VK_ERROR_FEATURE_NOT_PRESENT;

   /* While the Vulkan spec allows us to support depth/stencil sparse images
    * everywhere, sometimes we're not able to have them with the tiling
    * formats that give us the standard block shapes. Having standard block
    * shapes is higher priority than supporting depth/stencil sparse images.
    *
    * Please see ISL's filter_tiling() functions for accurate explanations on
    * why depth/stencil images are not always supported with the tiling
    * formats we want. But in short: depth/stencil support in our HW is
    * limited to 2D and we can't build a 2D view of a 3D image with these
    * tiling formats due to the address swizzling being different.
    */
   VkImageAspectFlags aspects = vk_format_aspects(vk_format);
   if (aspects & (VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT)) {
      /* For 125+, isl_gfx125_filter_tiling() claims 3D is not supported.
       * For the previous platforms, isl_gfx6_filter_tiling() says only 2D is
       * supported.
       */
      if (pdevice->info.verx10 >= 125) {
         if (type == VK_IMAGE_TYPE_3D)
            return VK_ERROR_FORMAT_NOT_SUPPORTED;
      } else {
         if (type != VK_IMAGE_TYPE_2D)
            return VK_ERROR_FORMAT_NOT_SUPPORTED;
      }
   }

   const struct anv_format *anv_format = anv_get_format(vk_format);
   if (!anv_format)
      return VK_ERROR_FORMAT_NOT_SUPPORTED;

   for (int p = 0; p < anv_format->n_planes; p++) {
      enum isl_format isl_format = anv_format->planes[p].isl_format;

      if (isl_format == ISL_FORMAT_UNSUPPORTED)
         return VK_ERROR_FORMAT_NOT_SUPPORTED;

      const struct isl_format_layout *isl_layout =
         isl_format_get_layout(isl_format);

      /* As quoted above, we only need to support the power-of-two formats.
       * The problem with the non-power-of-two formats is that we need an
       * integer number of pixels to fit into a sparse block, so we'd need the
       * sparse block sizes to be, for example, 192k for 24bpp.
       *
       * TODO: add support for these formats.
       */
      if (isl_layout->bpb != 8 && isl_layout->bpb != 16 &&
          isl_layout->bpb != 32 && isl_layout->bpb != 64 &&
          isl_layout->bpb != 128)
         return VK_ERROR_FORMAT_NOT_SUPPORTED;
   }

   /* These YUV formats are considered by Vulkan to be compressed 2x1 blocks.
    * We don't need to support them since they're compressed. On Gfx12 we
    * can't even have Tile64 for them. Once we do support these formats we'll
    * have to report the correct block shapes because dEQP cares about them,
    * and we'll have to adjust for the fact that ISL treats these as 16bpp 1x1
    * blocks instead of 32bpp 2x1 compressed blocks (as block shapes are
    * reported in units of compressed blocks).
    */
   if (vk_format == VK_FORMAT_G8B8G8R8_422_UNORM ||
       vk_format == VK_FORMAT_B8G8R8G8_422_UNORM)
      return VK_ERROR_FORMAT_NOT_SUPPORTED;

   return VK_SUCCESS;
}

static VkResult
anv_trtt_garbage_collect_batches(struct anv_device *device)
{
   struct anv_trtt *trtt = &device->trtt;

   if (trtt->timeline_val % 8 != 7)
      return VK_SUCCESS;

   uint64_t cur_timeline_val = 0;
   struct drm_syncobj_timeline_array array = {
      .handles = (uintptr_t)&trtt->timeline_handle,
      .points = (uintptr_t)&cur_timeline_val,
      .count_handles = 1,
      .flags = 0,
   };
   if (intel_ioctl(device->fd, DRM_IOCTL_SYNCOBJ_QUERY, &array))
      return vk_error(device, VK_ERROR_UNKNOWN);

   list_for_each_entry_safe(struct anv_trtt_batch_bo, trtt_bbo,
                            &trtt->in_flight_batches, link) {
      if (trtt_bbo->timeline_val > cur_timeline_val)
         return VK_SUCCESS;

      anv_trtt_batch_bo_free(device, trtt_bbo);
   }

   return VK_SUCCESS;
}

VkResult
anv_trtt_batch_bo_new(struct anv_device *device, uint32_t batch_size,
                      struct anv_trtt_batch_bo **out_trtt_bbo)
{
   struct anv_trtt *trtt = &device->trtt;
   VkResult result;

   anv_trtt_garbage_collect_batches(device);

   struct anv_trtt_batch_bo *trtt_bbo =
      vk_alloc(&device->vk.alloc, sizeof(*trtt_bbo), 8,
               VK_SYSTEM_ALLOCATION_SCOPE_DEVICE);
   if (!trtt_bbo)
      return vk_error(device, VK_ERROR_OUT_OF_HOST_MEMORY);

   result = anv_bo_pool_alloc(&device->batch_bo_pool, batch_size,
                              &trtt_bbo->bo);
   if (result != VK_SUCCESS)
      goto out;

   trtt_bbo->size = batch_size;
   trtt_bbo->timeline_val = ++trtt->timeline_val;

   list_addtail(&trtt_bbo->link, &trtt->in_flight_batches);

   *out_trtt_bbo = trtt_bbo;

   return VK_SUCCESS;
out:
   vk_free(&device->vk.alloc, trtt_bbo);
   return result;
}
