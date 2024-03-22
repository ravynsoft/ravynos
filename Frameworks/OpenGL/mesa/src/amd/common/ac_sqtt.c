/*
 * Copyright 2020 Advanced Micro Devices, Inc.
 * Copyright 2020 Valve Corporation
 *
 * SPDX-License-Identifier: MIT
 */

#include "ac_sqtt.h"

#include "ac_gpu_info.h"
#include "util/u_math.h"
#include "util/os_time.h"

uint64_t
ac_sqtt_get_info_offset(unsigned se)
{
   return sizeof(struct ac_sqtt_data_info) * se;
}

uint64_t
ac_sqtt_get_data_offset(const struct radeon_info *rad_info, const struct ac_sqtt *data, unsigned se)
{
   unsigned max_se = rad_info->max_se;
   uint64_t data_offset;

   data_offset = align64(sizeof(struct ac_sqtt_data_info) * max_se, 1 << SQTT_BUFFER_ALIGN_SHIFT);
   data_offset += data->buffer_size * se;

   return data_offset;
}

uint64_t
ac_sqtt_get_info_va(uint64_t va, unsigned se)
{
   return va + ac_sqtt_get_info_offset(se);
}

uint64_t
ac_sqtt_get_data_va(const struct radeon_info *rad_info, const struct ac_sqtt *data, uint64_t va,
                    unsigned se)
{
   return va + ac_sqtt_get_data_offset(rad_info, data, se);
}

void
ac_sqtt_init(struct ac_sqtt *data)
{
   list_inithead(&data->rgp_pso_correlation.record);
   simple_mtx_init(&data->rgp_pso_correlation.lock, mtx_plain);

   list_inithead(&data->rgp_loader_events.record);
   simple_mtx_init(&data->rgp_loader_events.lock, mtx_plain);

   list_inithead(&data->rgp_code_object.record);
   simple_mtx_init(&data->rgp_code_object.lock, mtx_plain);

   list_inithead(&data->rgp_clock_calibration.record);
   simple_mtx_init(&data->rgp_clock_calibration.lock, mtx_plain);

   list_inithead(&data->rgp_queue_info.record);
   simple_mtx_init(&data->rgp_queue_info.lock, mtx_plain);

   list_inithead(&data->rgp_queue_event.record);
   simple_mtx_init(&data->rgp_queue_event.lock, mtx_plain);
}

void
ac_sqtt_finish(struct ac_sqtt *data)
{
   assert(data->rgp_pso_correlation.record_count == 0);
   simple_mtx_destroy(&data->rgp_pso_correlation.lock);

   assert(data->rgp_loader_events.record_count == 0);
   simple_mtx_destroy(&data->rgp_loader_events.lock);

   assert(data->rgp_code_object.record_count == 0);
   simple_mtx_destroy(&data->rgp_code_object.lock);

   assert(data->rgp_clock_calibration.record_count == 0);
   simple_mtx_destroy(&data->rgp_clock_calibration.lock);

   assert(data->rgp_queue_info.record_count == 0);
   simple_mtx_destroy(&data->rgp_queue_info.lock);

   assert(data->rgp_queue_event.record_count == 0);
   simple_mtx_destroy(&data->rgp_queue_event.lock);
}

bool
ac_is_sqtt_complete(const struct radeon_info *rad_info, const struct ac_sqtt *data,
                    const struct ac_sqtt_data_info *info)
{
   if (rad_info->gfx_level >= GFX10) {
      /* GFX10 doesn't have THREAD_TRACE_CNTR but it reports the number of
       * dropped bytes per SE via THREAD_TRACE_DROPPED_CNTR. Though, this
       * doesn't seem reliable because it might still report non-zero even if
       * the SQTT buffer isn't full.
       *
       * The solution here is to compare the number of bytes written by the hw
       * (in units of 32 bytes) to the SQTT buffer size. If it's equal, that
       * means that the buffer is full and should be resized.
       */
      return !(info->cur_offset * 32 == data->buffer_size - 32);
   }

   /* Otherwise, compare the current thread trace offset with the number
    * of written bytes.
    */
   return info->cur_offset == info->gfx9_write_counter;
}

uint32_t
ac_get_expected_buffer_size(struct radeon_info *rad_info, const struct ac_sqtt_data_info *info)
{
   if (rad_info->gfx_level >= GFX10) {
      uint32_t dropped_cntr_per_se = info->gfx10_dropped_cntr / rad_info->max_se;
      return ((info->cur_offset * 32) + dropped_cntr_per_se) / 1024;
   }

   return (info->gfx9_write_counter * 32) / 1024;
}

bool
ac_sqtt_add_pso_correlation(struct ac_sqtt *sqtt, uint64_t pipeline_hash, uint64_t api_hash)
{
   struct rgp_pso_correlation *pso_correlation = &sqtt->rgp_pso_correlation;
   struct rgp_pso_correlation_record *record;

   record = malloc(sizeof(struct rgp_pso_correlation_record));
   if (!record)
      return false;

   record->api_pso_hash = api_hash;
   record->pipeline_hash[0] = pipeline_hash;
   record->pipeline_hash[1] = pipeline_hash;
   memset(record->api_level_obj_name, 0, sizeof(record->api_level_obj_name));

   simple_mtx_lock(&pso_correlation->lock);
   list_addtail(&record->list, &pso_correlation->record);
   pso_correlation->record_count++;
   simple_mtx_unlock(&pso_correlation->lock);

   return true;
}

bool
ac_sqtt_add_code_object_loader_event(struct ac_sqtt *sqtt, uint64_t pipeline_hash,
                                     uint64_t base_address)
{
   struct rgp_loader_events *loader_events = &sqtt->rgp_loader_events;
   struct rgp_loader_events_record *record;

   record = malloc(sizeof(struct rgp_loader_events_record));
   if (!record)
      return false;

   record->loader_event_type = RGP_LOAD_TO_GPU_MEMORY;
   record->reserved = 0;
   record->base_address = base_address & 0xffffffffffff;
   record->code_object_hash[0] = pipeline_hash;
   record->code_object_hash[1] = pipeline_hash;
   record->time_stamp = os_time_get_nano();

   simple_mtx_lock(&loader_events->lock);
   list_addtail(&record->list, &loader_events->record);
   loader_events->record_count++;
   simple_mtx_unlock(&loader_events->lock);

   return true;
}

bool
ac_sqtt_add_clock_calibration(struct ac_sqtt *sqtt, uint64_t cpu_timestamp, uint64_t gpu_timestamp)
{
   struct rgp_clock_calibration *clock_calibration = &sqtt->rgp_clock_calibration;
   struct rgp_clock_calibration_record *record;

   record = malloc(sizeof(struct rgp_clock_calibration_record));
   if (!record)
      return false;

   record->cpu_timestamp = cpu_timestamp;
   record->gpu_timestamp = gpu_timestamp;

   simple_mtx_lock(&clock_calibration->lock);
   list_addtail(&record->list, &clock_calibration->record);
   clock_calibration->record_count++;
   simple_mtx_unlock(&clock_calibration->lock);

   return true;
}

/* See https://gitlab.freedesktop.org/mesa/mesa/-/issues/5260
 * On some HW SQTT can hang if we're not in one of the profiling pstates. */
bool
ac_check_profile_state(const struct radeon_info *info)
{
   char path[128];
   char data[128];
   int n;

   if (!info->pci.valid)
      return false; /* Unknown but optimistic. */

   snprintf(path, sizeof(path),
            "/sys/bus/pci/devices/%04x:%02x:%02x.%x/power_dpm_force_performance_level",
            info->pci.domain, info->pci.bus, info->pci.dev, info->pci.func);

   FILE *f = fopen(path, "r");
   if (!f)
      return false; /* Unknown but optimistic. */
   n = fread(data, 1, sizeof(data) - 1, f);
   fclose(f);
   data[n] = 0;
   return strstr(data, "profile") == NULL;
}

union rgp_sqtt_marker_cb_id
ac_sqtt_get_next_cmdbuf_id(struct ac_sqtt *data, enum amd_ip_type ip_type)
{
   union rgp_sqtt_marker_cb_id cb_id = {0};

   cb_id.global_cb_id.cb_index =
      p_atomic_inc_return(&data->cmdbuf_ids_per_queue[ip_type]);

   return cb_id;
}

bool
ac_sqtt_se_is_disabled(const struct radeon_info *info, unsigned se)
{
   /* No active CU on the SE means it is disabled. */
   return info->cu_mask[se][0] == 0;
}

uint32_t
ac_sqtt_get_active_cu(const struct radeon_info *info, unsigned se)
{
   uint32_t cu_index;

   if (info->gfx_level >= GFX11) {
      /* GFX11 seems to operate on the last active CU. */
      cu_index = util_last_bit(info->cu_mask[se][0]) - 1;
   } else {
      /* Default to the first active CU. */
      cu_index = ffs(info->cu_mask[se][0]);
   }

   return cu_index;
}

bool
ac_sqtt_get_trace(struct ac_sqtt *data, const struct radeon_info *info,
                  struct ac_sqtt_trace *sqtt_trace)
{
   unsigned max_se = info->max_se;
   void *ptr = data->ptr;

   memset(sqtt_trace, 0, sizeof(*sqtt_trace));

   for (unsigned se = 0; se < max_se; se++) {
      uint64_t info_offset = ac_sqtt_get_info_offset(se);
      uint64_t data_offset = ac_sqtt_get_data_offset(info, data, se);
      void *info_ptr = (uint8_t *)ptr + info_offset;
      void *data_ptr = (uint8_t *)ptr + data_offset;
      struct ac_sqtt_data_info *trace_info = (struct ac_sqtt_data_info *)info_ptr;
      struct ac_sqtt_data_se data_se = {0};
      int active_cu = ac_sqtt_get_active_cu(info, se);

      if (ac_sqtt_se_is_disabled(info, se))
         continue;

      if (!ac_is_sqtt_complete(info, data, trace_info))
         return false;

      data_se.data_ptr = data_ptr;
      data_se.info = *trace_info;
      data_se.shader_engine = se;

      /* RGP seems to expect units of WGP on GFX10+. */
      data_se.compute_unit = info->gfx_level >= GFX10 ? (active_cu / 2) : active_cu;

      sqtt_trace->traces[sqtt_trace->num_traces] = data_se;
      sqtt_trace->num_traces++;
   }

   sqtt_trace->rgp_code_object = &data->rgp_code_object;
   sqtt_trace->rgp_loader_events = &data->rgp_loader_events;
   sqtt_trace->rgp_pso_correlation = &data->rgp_pso_correlation;
   sqtt_trace->rgp_queue_info = &data->rgp_queue_info;
   sqtt_trace->rgp_queue_event = &data->rgp_queue_event;
   sqtt_trace->rgp_clock_calibration = &data->rgp_clock_calibration;

   return true;
}

uint32_t
ac_sqtt_get_shader_mask(const struct radeon_info *info)
{
   unsigned shader_mask = 0x7f; /* all shader stages */

   if (info->gfx_level >= GFX11) {
      /* Disable unsupported hw shader stages */
      shader_mask &= ~(0x02 /* VS */ | 0x08 /* ES */ | 0x20 /* LS */);
   }

   return shader_mask;
}
