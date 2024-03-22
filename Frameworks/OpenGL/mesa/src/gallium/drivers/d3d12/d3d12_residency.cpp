/*
 * Copyright © Microsoft Corporation
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
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 */

#include "d3d12_batch.h"
#include "d3d12_bufmgr.h"
#include "d3d12_residency.h"
#include "d3d12_resource.h"
#include "d3d12_screen.h"

#include "util/os_time.h"

#include <dxguids/dxguids.h>

static constexpr unsigned residency_batch_size = 128;

static void
evict_aged_allocations(struct d3d12_screen *screen, uint64_t completed_fence, int64_t time, int64_t grace_period)
{
   ID3D12Pageable *to_evict[residency_batch_size];
   unsigned num_pending_evictions = 0;

   list_for_each_entry_safe(struct d3d12_bo, bo, &screen->residency_list, residency_list_entry) {
      /* This residency list should all be base bos, not suballocated ones */
      assert(bo->res);

      if (bo->last_used_fence > completed_fence ||
          time - bo->last_used_timestamp <= grace_period) {
         /* List is LRU-sorted, this bo is still in use, so we're done */
         break;
      }

      assert(bo->residency_status == d3d12_resident);

      to_evict[num_pending_evictions++] = bo->res;
      bo->residency_status = d3d12_evicted;
      list_del(&bo->residency_list_entry);

      if (num_pending_evictions == residency_batch_size) {
         screen->dev->Evict(num_pending_evictions, to_evict);
         num_pending_evictions = 0;
      }
   }

   if (num_pending_evictions)
      screen->dev->Evict(num_pending_evictions, to_evict);
}

static void
evict_to_fence_or_budget(struct d3d12_screen *screen, uint64_t target_fence, uint64_t current_usage, uint64_t target_budget)
{
   screen->fence->SetEventOnCompletion(target_fence, nullptr);

   ID3D12Pageable *to_evict[residency_batch_size];
   unsigned num_pending_evictions = 0;

   list_for_each_entry_safe(struct d3d12_bo, bo, &screen->residency_list, residency_list_entry) {
      /* This residency list should all be base bos, not suballocated ones */
      assert(bo->res);

      if (bo->last_used_fence > target_fence || current_usage < target_budget) {
         break;
      }

      assert(bo->residency_status == d3d12_resident);

      to_evict[num_pending_evictions++] = bo->res;
      bo->residency_status = d3d12_evicted;
      list_del(&bo->residency_list_entry);

      current_usage -= bo->estimated_size;

      if (num_pending_evictions == residency_batch_size) {
         screen->dev->Evict(num_pending_evictions, to_evict);
         num_pending_evictions = 0;
      }
   }

   if (num_pending_evictions)
      screen->dev->Evict(num_pending_evictions, to_evict);
}

static constexpr int64_t eviction_grace_period_seconds_min = 1;
static constexpr int64_t eviction_grace_period_seconds_max = 60;
static constexpr int64_t microseconds_per_second = 1000000;
static constexpr int64_t eviction_grace_period_microseconds_min =
   eviction_grace_period_seconds_min * microseconds_per_second;
static constexpr int64_t eviction_grace_period_microseconds_max =
   eviction_grace_period_seconds_max * microseconds_per_second;
static constexpr double trim_percentage_usage_threshold = 0.7;

static int64_t
get_eviction_grace_period(struct d3d12_memory_info *mem_info)
{
   double pressure = double(mem_info->usage) / double(mem_info->budget);
   pressure = MIN2(pressure, 1.0);

   if (pressure > trim_percentage_usage_threshold) {
      /* Normalize pressure for the range [0, threshold] */
      pressure = (pressure - trim_percentage_usage_threshold) / (1.0 - trim_percentage_usage_threshold);
      /* Linearly interpolate between min and max period based on pressure */
      return (int64_t)((eviction_grace_period_microseconds_max - eviction_grace_period_microseconds_min) *
         (1.0 - pressure)) + eviction_grace_period_microseconds_min;
   }

   /* Unlimited grace period, essentially don't trim at all */
   return INT64_MAX;
}

static void 
gather_base_bos(struct d3d12_screen *screen, set *base_bo_set, struct d3d12_bo *bo, uint64_t &size_to_make_resident, uint64_t pending_fence_value, int64_t current_time)
{
   uint64_t offset;
   struct d3d12_bo *base_bo = d3d12_bo_get_base(bo, &offset);

   if (base_bo->residency_status == d3d12_evicted) {
      bool added = false;
      _mesa_set_search_or_add(base_bo_set, base_bo, &added);
      assert(!added);

      base_bo->residency_status = d3d12_resident;
      size_to_make_resident += base_bo->estimated_size;
      list_addtail(&base_bo->residency_list_entry, &screen->residency_list);
   } else if (base_bo->last_used_fence != pending_fence_value &&
               base_bo->residency_status == d3d12_resident) {
      /* First time seeing this already-resident base bo in this batch */
      list_del(&base_bo->residency_list_entry);
      list_addtail(&base_bo->residency_list_entry, &screen->residency_list);
   }

   base_bo->last_used_fence = pending_fence_value;
   base_bo->last_used_timestamp = current_time;
}

void
d3d12_process_batch_residency(struct d3d12_screen *screen, struct d3d12_batch *batch)
{
   d3d12_memory_info mem_info;
   screen->get_memory_info(screen, &mem_info);

   uint64_t completed_fence_value = screen->fence->GetCompletedValue();
   uint64_t pending_fence_value = screen->fence_value + 1;
   int64_t current_time = os_time_get();
   int64_t grace_period = get_eviction_grace_period(&mem_info);

   /* Gather base bos for the batch */
   uint64_t size_to_make_resident = 0;
   set *base_bo_set = _mesa_pointer_set_create(nullptr);

   util_dynarray_foreach(&batch->local_bos, d3d12_bo*, bo)
      gather_base_bos(screen, base_bo_set, *bo, size_to_make_resident, pending_fence_value, current_time);
   hash_table_foreach(batch->bos, entry) 
      gather_base_bos(screen, base_bo_set, (struct d3d12_bo *)entry->key, size_to_make_resident, pending_fence_value, current_time);

   /* Now that bos referenced by this batch are moved to the end of the LRU, trim it */
   evict_aged_allocations(screen, completed_fence_value, current_time, grace_period);

   /* If there's nothing needing to be made newly resident, we're done once we've trimmed */
   if (base_bo_set->entries == 0) {
      _mesa_set_destroy(base_bo_set, nullptr);
      return;
   }

   uint64_t residency_fence_value_snapshot = screen->residency_fence_value;

   struct set_entry *entry = _mesa_set_next_entry(base_bo_set, nullptr);
   uint64_t batch_memory_size = 0;
   unsigned batch_count = 0;
   ID3D12Pageable *to_make_resident[residency_batch_size];
   while (true) {
      /* Refresh memory stats */
      screen->get_memory_info(screen, &mem_info);

      int64_t available_memory = (int64_t)mem_info.budget - (int64_t)mem_info.usage;

      assert(!list_is_empty(&screen->residency_list));
      struct d3d12_bo *oldest_resident_bo =
         list_first_entry(&screen->residency_list, struct d3d12_bo, residency_list_entry);
      bool anything_to_wait_for = oldest_resident_bo->last_used_fence < pending_fence_value;

      /* We've got some room, or we can't free up any more room, make some resources resident */
      HRESULT hr = S_OK;
      if ((available_memory || !anything_to_wait_for) && batch_count < residency_batch_size) {
         for (; entry; entry = _mesa_set_next_entry(base_bo_set, entry)) {
            struct d3d12_bo *bo = (struct d3d12_bo *)entry->key;
            if (anything_to_wait_for &&
                (int64_t)(batch_memory_size + bo->estimated_size) > available_memory)
               break;

            batch_memory_size += bo->estimated_size;
            to_make_resident[batch_count++] = bo->res;
            if (batch_count == residency_batch_size)
               break;
         }

         if (batch_count) {
            hr = screen->dev->EnqueueMakeResident(D3D12_RESIDENCY_FLAG_NONE, batch_count, to_make_resident,
               screen->residency_fence, screen->residency_fence_value + 1);
            if (SUCCEEDED(hr))
               ++screen->residency_fence_value;
         }

         if (SUCCEEDED(hr) && batch_count == residency_batch_size) {
            batch_count = 0;
            size_to_make_resident -= batch_memory_size;
            continue;
         }
      }

      /* We need to free up some space, either we broke early from the resource loop,
       * or the MakeResident call itself failed.
       */
      if (FAILED(hr) || entry) {
         if (!anything_to_wait_for) {
            assert(false);
            break;
         }

         evict_to_fence_or_budget(screen, oldest_resident_bo->last_used_fence, mem_info.usage + size_to_make_resident, mem_info.budget);
         continue;
      }

      /* Made it to the end without explicitly needing to loop, so we're done */
      break;
   }
   _mesa_set_destroy(base_bo_set, nullptr);

   /* The GPU needs to wait for these resources to be made resident */
   if (residency_fence_value_snapshot != screen->residency_fence_value)
      screen->cmdqueue->Wait(screen->residency_fence, screen->residency_fence_value);
}

bool
d3d12_init_residency(struct d3d12_screen *screen)
{
   list_inithead(&screen->residency_list);
   if (FAILED(screen->dev->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&screen->residency_fence))))
      return false;

   return true;
}

void
d3d12_deinit_residency(struct d3d12_screen *screen)
{
   if (screen->residency_fence) {
      screen->residency_fence->Release();
      screen->residency_fence = nullptr;
   }
}

void
d3d12_promote_to_permanent_residency(struct d3d12_screen *screen, struct d3d12_resource* resource)
{
   mtx_lock(&screen->submit_mutex);
   uint64_t offset;
   struct d3d12_bo *base_bo = d3d12_bo_get_base(resource->bo, &offset);

   /* Promote non-permanent resident resources to permanent residency*/
   if(base_bo->residency_status != d3d12_permanently_resident) {

      /* Mark as permanently resident*/
      base_bo->residency_status = d3d12_permanently_resident;

      /* If it wasn't made resident before, make it*/
      bool was_made_resident = (base_bo->residency_status == d3d12_resident);
      if(!was_made_resident) {
         ID3D12Pageable *pageable = base_bo->res;
         ASSERTED HRESULT hr = screen->dev->MakeResident(1, &pageable);
         assert(SUCCEEDED(hr));
      }
   }
   mtx_unlock(&screen->submit_mutex);
}
