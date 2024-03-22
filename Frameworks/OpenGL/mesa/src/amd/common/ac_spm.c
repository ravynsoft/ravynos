/*
 * Copyright 2021 Valve Corporation
 *
 * SPDX-License-Identifier: MIT
 */

#include "ac_spm.h"

#include "util/bitscan.h"
#include "util/u_memory.h"
#include "ac_perfcounter.h"

/* SPM counters definition. */
/* GFX10+ */
static struct ac_spm_counter_descr gfx10_num_l2_hits = {TCP, 0x9};
static struct ac_spm_counter_descr gfx10_num_l2_misses = {TCP, 0x12};
static struct ac_spm_counter_descr gfx10_num_scache_hits = {SQ, 0x14f};
static struct ac_spm_counter_descr gfx10_num_scache_misses = {SQ, 0x150};
static struct ac_spm_counter_descr gfx10_num_scache_misses_dup = {SQ, 0x151};
static struct ac_spm_counter_descr gfx10_num_icache_hits = {SQ, 0x12c};
static struct ac_spm_counter_descr gfx10_num_icache_misses = {SQ, 0x12d};
static struct ac_spm_counter_descr gfx10_num_icache_misses_dup = {SQ, 0x12e};
static struct ac_spm_counter_descr gfx10_num_gl1c_hits = {GL1C, 0xe};
static struct ac_spm_counter_descr gfx10_num_gl1c_misses = {GL1C, 0x12};
static struct ac_spm_counter_descr gfx10_num_gl2c_hits = {GL2C, 0x3};
static struct ac_spm_counter_descr gfx10_num_gl2c_misses = {GL2C, 0x23};

static struct ac_spm_counter_create_info gfx10_spm_counters[] = {
   {&gfx10_num_l2_hits},
   {&gfx10_num_l2_misses},
   {&gfx10_num_scache_hits},
   {&gfx10_num_scache_misses},
   {&gfx10_num_scache_misses_dup},
   {&gfx10_num_icache_hits},
   {&gfx10_num_icache_misses},
   {&gfx10_num_icache_misses_dup},
   {&gfx10_num_gl1c_hits},
   {&gfx10_num_gl1c_misses},
   {&gfx10_num_gl2c_hits},
   {&gfx10_num_gl2c_misses},
};

/* GFX10.3+ */
static struct ac_spm_counter_descr gfx103_num_gl2c_misses = {GL2C, 0x2b};

static struct ac_spm_counter_create_info gfx103_spm_counters[] = {
   {&gfx10_num_l2_hits},
   {&gfx10_num_l2_misses},
   {&gfx10_num_scache_hits},
   {&gfx10_num_scache_misses},
   {&gfx10_num_scache_misses_dup},
   {&gfx10_num_icache_hits},
   {&gfx10_num_icache_misses},
   {&gfx10_num_icache_misses_dup},
   {&gfx10_num_gl1c_hits},
   {&gfx10_num_gl1c_misses},
   {&gfx10_num_gl2c_hits},
   {&gfx103_num_gl2c_misses},
};

/* GFX11+ */
static struct ac_spm_counter_descr gfx11_num_l2_misses = {TCP, 0x11};
static struct ac_spm_counter_descr gfx11_num_scache_hits = {SQ_WGP, 0x126};
static struct ac_spm_counter_descr gfx11_num_scache_misses = {SQ_WGP, 0x127};
static struct ac_spm_counter_descr gfx11_num_scache_misses_dup = {SQ_WGP, 0x128};
static struct ac_spm_counter_descr gfx11_num_icache_hits = {SQ_WGP, 0x10e};
static struct ac_spm_counter_descr gfx11_num_icache_misses = {SQ_WGP, 0x10f};
static struct ac_spm_counter_descr gfx11_num_icache_misses_dup = {SQ_WGP, 0x110};

static struct ac_spm_counter_create_info gfx11_spm_counters[] = {
   {&gfx10_num_l2_hits},
   {&gfx11_num_l2_misses},
   {&gfx11_num_scache_hits},
   {&gfx11_num_scache_misses},
   {&gfx11_num_scache_misses_dup},
   {&gfx11_num_icache_hits},
   {&gfx11_num_icache_misses},
   {&gfx11_num_icache_misses_dup},
   {&gfx10_num_gl1c_hits},
   {&gfx10_num_gl1c_misses},
   {&gfx10_num_gl2c_hits},
   {&gfx103_num_gl2c_misses},
};

static struct ac_spm_block_select *
ac_spm_get_block_select(struct ac_spm *spm, const struct ac_pc_block *block)
{
   struct ac_spm_block_select *block_sel, *new_block_sel;
   uint32_t num_block_sel;

   for (uint32_t i = 0; i < spm->num_block_sel; i++) {
      if (spm->block_sel[i].b->b->b->gpu_block == block->b->b->gpu_block)
         return &spm->block_sel[i];
   }

   /* Allocate a new select block if it doesn't already exist. */
   num_block_sel = spm->num_block_sel + 1;
   block_sel = realloc(spm->block_sel, num_block_sel * sizeof(*block_sel));
   if (!block_sel)
      return NULL;

   spm->num_block_sel = num_block_sel;
   spm->block_sel = block_sel;

   /* Initialize the new select block. */
   new_block_sel = &spm->block_sel[spm->num_block_sel - 1];
   memset(new_block_sel, 0, sizeof(*new_block_sel));

   new_block_sel->b = block;
   new_block_sel->instances =
      calloc(block->num_global_instances, sizeof(*new_block_sel->instances));
   if (!new_block_sel->instances)
      return NULL;
   new_block_sel->num_instances = block->num_global_instances;

   for (unsigned i = 0; i < new_block_sel->num_instances; i++)
      new_block_sel->instances[i].num_counters = block->b->b->num_spm_counters;

   return new_block_sel;
}

struct ac_spm_instance_mapping {
   uint32_t se_index;         /* SE index or 0 if global */
   uint32_t sa_index;         /* SA index or 0 if global or per-SE */
   uint32_t instance_index;
};

static bool
ac_spm_init_instance_mapping(const struct radeon_info *info,
                             const struct ac_pc_block *block,
                             const struct ac_spm_counter_info *counter,
                             struct ac_spm_instance_mapping *mapping)
{
   uint32_t instance_index = 0, se_index = 0, sa_index = 0;

   if (block->b->b->flags & AC_PC_BLOCK_SE) {
      if (block->b->b->gpu_block == SQ) {
         /* Per-SE blocks. */
         se_index = counter->instance / block->num_instances;
         instance_index = counter->instance % block->num_instances;
      } else {
         /* Per-SA blocks. */
         assert(block->b->b->gpu_block == GL1C ||
                block->b->b->gpu_block == TCP ||
                block->b->b->gpu_block == SQ_WGP);
         se_index = (counter->instance / block->num_instances) / info->max_sa_per_se;
         sa_index = (counter->instance / block->num_instances) % info->max_sa_per_se;
         instance_index = counter->instance % block->num_instances;
      }
   } else {
      /* Global blocks. */
      assert(block->b->b->gpu_block == GL2C);
      instance_index = counter->instance;
   }

   if (se_index >= info->num_se ||
       sa_index >= info->max_sa_per_se ||
       instance_index >= block->num_instances)
      return false;

   mapping->se_index = se_index;
   mapping->sa_index = sa_index;
   mapping->instance_index = instance_index;

   return true;
}

static void
ac_spm_init_muxsel(const struct radeon_info *info,
                   const struct ac_pc_block *block,
                   const struct ac_spm_instance_mapping *mapping,
                   struct ac_spm_counter_info *counter,
                   uint32_t spm_wire)
{
   const uint16_t counter_idx = 2 * spm_wire + (counter->is_even ? 0 : 1);
   union ac_spm_muxsel *muxsel = &counter->muxsel;

   if (info->gfx_level >= GFX11) {
      muxsel->gfx11.counter = counter_idx;
      muxsel->gfx11.block = block->b->b->spm_block_select;
      muxsel->gfx11.shader_array = mapping->sa_index;
      muxsel->gfx11.instance = mapping->instance_index;
   } else {
      muxsel->gfx10.counter = counter_idx;
      muxsel->gfx10.block = block->b->b->spm_block_select;
      muxsel->gfx10.shader_array = mapping->sa_index;
      muxsel->gfx10.instance = mapping->instance_index;
   }
}

static uint32_t
ac_spm_init_grbm_gfx_index(const struct ac_pc_block *block,
                           const struct ac_spm_instance_mapping *mapping)
{
   uint32_t instance = mapping->instance_index;
   uint32_t grbm_gfx_index = 0;

   grbm_gfx_index |= S_030800_SE_INDEX(mapping->se_index) |
                     S_030800_SH_INDEX(mapping->sa_index);

   switch (block->b->b->gpu_block) {
   case GL2C:
      /* Global blocks. */
      grbm_gfx_index |= S_030800_SE_BROADCAST_WRITES(1);
      break;
   case SQ:
      /* Per-SE blocks. */
      grbm_gfx_index |= S_030800_SH_BROADCAST_WRITES(1);
      break;
   default:
      /* Other blocks shouldn't broadcast. */
      break;
   }

   if (block->b->b->gpu_block == SQ_WGP) {
      union {
         struct {
            uint32_t block_index : 2; /* Block index withing WGP */
            uint32_t wgp_index : 3;
            uint32_t is_below_spi : 1; /* 0: lower WGP numbers, 1: higher WGP numbers */
            uint32_t reserved : 26;
         };

         uint32_t value;
      } instance_index = {0};

      const uint32_t num_wgp_above_spi = 4;
      const bool is_below_spi = mapping->instance_index >= num_wgp_above_spi;

      instance_index.wgp_index =
         is_below_spi ? (mapping->instance_index - num_wgp_above_spi) : mapping->instance_index;
      instance_index.is_below_spi = is_below_spi;

      instance = instance_index.value;
   }

   grbm_gfx_index |= S_030800_INSTANCE_INDEX(instance);

   return grbm_gfx_index;
}

static bool
ac_spm_map_counter(struct ac_spm *spm, struct ac_spm_block_select *block_sel,
                   struct ac_spm_counter_info *counter,
                   const struct ac_spm_instance_mapping *mapping,
                   uint32_t *spm_wire)
{
   uint32_t instance = counter->instance;

   if (block_sel->b->b->b->gpu_block == SQ_WGP) {
      if (!spm->sq_wgp[instance].grbm_gfx_index) {
         spm->sq_wgp[instance].grbm_gfx_index =
            ac_spm_init_grbm_gfx_index(block_sel->b, mapping);
      }

      for (unsigned i = 0; i < ARRAY_SIZE(spm->sq_wgp[instance].counters); i++) {
         struct ac_spm_counter_select *cntr_sel = &spm->sq_wgp[instance].counters[i];

         if (i < spm->sq_wgp[instance].num_counters)
            continue;

         cntr_sel->sel0 |= S_036700_PERF_SEL(counter->event_id) |
                           S_036700_SPM_MODE(1) | /* 16-bit clamp */
                           S_036700_PERF_MODE(0);

         /* Each SQ_WQP modules (GFX11+) share one 32-bit accumulator/wire
          * per pair of selects.
          */
         cntr_sel->active |= 1 << (i % 2);
         *spm_wire = i / 2;

         if (cntr_sel->active & 0x1)
            counter->is_even = true;

         spm->sq_wgp[instance].num_counters++;
         return true;
      }
   } else if (block_sel->b->b->b->gpu_block == SQ) {
      for (unsigned i = 0; i < ARRAY_SIZE(spm->sqg[instance].counters); i++) {
         struct ac_spm_counter_select *cntr_sel = &spm->sqg[instance].counters[i];

         if (i < spm->sqg[instance].num_counters)
            continue;

         /* SQ doesn't support 16-bit counters. */
         cntr_sel->sel0 |= S_036700_PERF_SEL(counter->event_id) |
                           S_036700_SPM_MODE(3) | /* 32-bit clamp */
                           S_036700_PERF_MODE(0);
         cntr_sel->active |= 0x3;

         /* 32-bits counter are always even. */
         counter->is_even = true;

         /* One wire per SQ module. */
         *spm_wire = i;

         spm->sqg[instance].num_counters++;
         return true;
      }
   } else {
      /* Generic blocks. */
      struct ac_spm_block_instance *block_instance =
         &block_sel->instances[instance];

      if (!block_instance->grbm_gfx_index) {
         block_instance->grbm_gfx_index =
            ac_spm_init_grbm_gfx_index(block_sel->b, mapping);
      }

      for (unsigned i = 0; i < block_instance->num_counters; i++) {
         struct ac_spm_counter_select *cntr_sel = &block_instance->counters[i];
         int index = ffs(~cntr_sel->active) - 1;

         switch (index) {
         case 0: /* use S_037004_PERF_SEL */
            cntr_sel->sel0 |= S_037004_PERF_SEL(counter->event_id) |
                              S_037004_CNTR_MODE(1) | /* 16-bit clamp */
                              S_037004_PERF_MODE(0); /* accum */
            break;
         case 1: /* use S_037004_PERF_SEL1 */
            cntr_sel->sel0 |= S_037004_PERF_SEL1(counter->event_id) |
                              S_037004_PERF_MODE1(0);
            break;
         case 2: /* use S_037004_PERF_SEL2 */
            cntr_sel->sel1 |= S_037008_PERF_SEL2(counter->event_id) |
                              S_037008_PERF_MODE2(0);
            break;
         case 3: /* use S_037004_PERF_SEL3 */
            cntr_sel->sel1 |= S_037008_PERF_SEL3(counter->event_id) |
                              S_037008_PERF_MODE3(0);
            break;
         default:
            return false;
         }

         /* Mark this 16-bit counter as used. */
         cntr_sel->active |= 1 << index;

         /* Determine if the counter is even or odd. */
         counter->is_even = !(index % 2);

         /* Determine the SPM wire (one wire holds two 16-bit counters). */
         *spm_wire = !!(index >= 2);

         return true;
      }
   }

   return false;
}

static bool
ac_spm_add_counter(const struct radeon_info *info,
                   const struct ac_perfcounters *pc,
                   struct ac_spm *spm,
                   const struct ac_spm_counter_create_info *counter_info)
{
   struct ac_spm_instance_mapping instance_mapping = {0};
   struct ac_spm_counter_info *counter;
   struct ac_spm_block_select *block_sel;
   struct ac_pc_block *block;
   uint32_t spm_wire;

   /* Check if the GPU block is valid. */
   block = ac_pc_get_block(pc, counter_info->b->gpu_block);
   if (!block) {
      fprintf(stderr, "ac/spm: Invalid GPU block.\n");
      return false;
   }

   /* Check if the number of instances is valid. */
   if (counter_info->instance > block->num_global_instances - 1) {
      fprintf(stderr, "ac/spm: Invalid instance ID.\n");
      return false;
   }

   /* Check if the event ID is valid. */
   if (counter_info->b->event_id > block->b->selectors) {
      fprintf(stderr, "ac/spm: Invalid event ID.\n");
      return false;
   }

   counter = &spm->counters[spm->num_counters];
   spm->num_counters++;

   counter->gpu_block = counter_info->b->gpu_block;
   counter->event_id = counter_info->b->event_id;
   counter->instance = counter_info->instance;

   /* Get the select block used to configure the counter. */
   block_sel = ac_spm_get_block_select(spm, block);
   if (!block_sel)
      return false;

   /* Initialize instance mapping for the counter. */
   if (!ac_spm_init_instance_mapping(info, block, counter, &instance_mapping)) {
      fprintf(stderr, "ac/spm: Failed to initialize instance mapping.\n");
      return false;
   }

   /* Map the counter to the select block. */
   if (!ac_spm_map_counter(spm, block_sel, counter, &instance_mapping, &spm_wire)) {
      fprintf(stderr, "ac/spm: No free slots available!\n");
      return false;
   }

   /* Determine the counter segment type. */
   if (block->b->b->flags & AC_PC_BLOCK_SE) {
      counter->segment_type = instance_mapping.se_index;
   } else {
      counter->segment_type = AC_SPM_SEGMENT_TYPE_GLOBAL;
   }

   /* Configure the muxsel for SPM. */
   ac_spm_init_muxsel(info, block, &instance_mapping, counter, spm_wire);

   return true;
}

static void
ac_spm_fill_muxsel_ram(const struct radeon_info *info,
                       struct ac_spm *spm,
                       enum ac_spm_segment_type segment_type,
                       uint32_t offset)
{
   struct ac_spm_muxsel_line *mappings = spm->muxsel_lines[segment_type];
   uint32_t even_counter_idx = 0, even_line_idx = 0;
   uint32_t odd_counter_idx = 0, odd_line_idx = 1;

   /* Add the global timestamps first. */
   if (segment_type == AC_SPM_SEGMENT_TYPE_GLOBAL) {
      if (info->gfx_level >= GFX11) {
         mappings[even_line_idx].muxsel[even_counter_idx++].value = 0xf840;
         mappings[even_line_idx].muxsel[even_counter_idx++].value = 0xf841;
         mappings[even_line_idx].muxsel[even_counter_idx++].value = 0xf842;
         mappings[even_line_idx].muxsel[even_counter_idx++].value = 0xf843;
      } else {
         for (unsigned i = 0; i < 4; i++) {
            mappings[even_line_idx].muxsel[even_counter_idx++].value = 0xf0f0;
         }
      }
   }

   for (unsigned i = 0; i < spm->num_counters; i++) {
      struct ac_spm_counter_info *counter = &spm->counters[i];

      if (counter->segment_type != segment_type)
         continue;

      if (counter->is_even) {
         counter->offset =
            (offset + even_line_idx) * AC_SPM_NUM_COUNTER_PER_MUXSEL + even_counter_idx;

         mappings[even_line_idx].muxsel[even_counter_idx] = spm->counters[i].muxsel;
         if (++even_counter_idx == AC_SPM_NUM_COUNTER_PER_MUXSEL) {
            even_counter_idx = 0;
            even_line_idx += 2;
         }
      } else {
         counter->offset =
            (offset + odd_line_idx) * AC_SPM_NUM_COUNTER_PER_MUXSEL + odd_counter_idx;

         mappings[odd_line_idx].muxsel[odd_counter_idx] = spm->counters[i].muxsel;
         if (++odd_counter_idx == AC_SPM_NUM_COUNTER_PER_MUXSEL) {
            odd_counter_idx = 0;
            odd_line_idx += 2;
         }
      }
   }
}

bool ac_init_spm(const struct radeon_info *info,
                 const struct ac_perfcounters *pc,
                 struct ac_spm *spm)
{
   const struct ac_spm_counter_create_info *create_info;
   unsigned create_info_count;
   unsigned num_counters = 0;

   switch (info->gfx_level) {
   case GFX10:
      create_info_count = ARRAY_SIZE(gfx10_spm_counters);
      create_info = gfx10_spm_counters;
      break;
   case GFX10_3:
      create_info_count = ARRAY_SIZE(gfx103_spm_counters);
      create_info = gfx103_spm_counters;
      break;
   case GFX11:
   case GFX11_5:
      create_info_count = ARRAY_SIZE(gfx11_spm_counters);
      create_info = gfx11_spm_counters;
      break;
   default:
      return false; /* not implemented */
   }

   /* Count the total number of counters. */
   for (unsigned i = 0; i < create_info_count; i++) {
      const struct ac_pc_block *block = ac_pc_get_block(pc, create_info[i].b->gpu_block);

      if (!block)
         return false;

      num_counters += block->num_global_instances;
   }

   spm->counters = CALLOC(num_counters, sizeof(*spm->counters));
   if (!spm->counters)
      return false;

   for (unsigned i = 0; i < create_info_count; i++) {
      const struct ac_pc_block *block = ac_pc_get_block(pc, create_info[i].b->gpu_block);
      struct ac_spm_counter_create_info counter = create_info[i];

      for (unsigned j = 0; j < block->num_global_instances; j++) {
         counter.instance = j;

         if (!ac_spm_add_counter(info, pc, spm, &counter)) {
            fprintf(stderr, "ac/spm: Failed to add SPM counter (%d).\n", i);
            return false;
         }
      }
   }

   /* Determine the segment size and create a muxsel ram for every segment. */
   for (unsigned s = 0; s < AC_SPM_SEGMENT_TYPE_COUNT; s++) {
      unsigned num_even_counters = 0, num_odd_counters = 0;

      if (s == AC_SPM_SEGMENT_TYPE_GLOBAL) {
         /* The global segment always start with a 64-bit timestamp. */
         num_even_counters += AC_SPM_GLOBAL_TIMESTAMP_COUNTERS;
      }

      /* Count the number of even/odd counters for this segment. */
      for (unsigned c = 0; c < spm->num_counters; c++) {
         struct ac_spm_counter_info *counter = &spm->counters[c];

         if (counter->segment_type != s)
            continue;

         if (counter->is_even) {
            num_even_counters++;
         } else {
            num_odd_counters++;
         }
      }

      /* Compute the number of lines. */
      unsigned even_lines =
         DIV_ROUND_UP(num_even_counters, AC_SPM_NUM_COUNTER_PER_MUXSEL);
      unsigned odd_lines =
         DIV_ROUND_UP(num_odd_counters, AC_SPM_NUM_COUNTER_PER_MUXSEL);
      unsigned num_lines = (even_lines > odd_lines) ? (2 * even_lines - 1) : (2 * odd_lines);

      spm->muxsel_lines[s] = CALLOC(num_lines, sizeof(*spm->muxsel_lines[s]));
      if (!spm->muxsel_lines[s])
         return false;
      spm->num_muxsel_lines[s] = num_lines;
   }

   /* Compute the maximum number of muxsel lines among all SEs. On GFX11,
    * there is only one SE segment size value and the highest value is used.
    */
   for (unsigned s = 0; s < AC_SPM_SEGMENT_TYPE_GLOBAL; s++) {
      spm->max_se_muxsel_lines =
         MAX2(spm->num_muxsel_lines[s], spm->max_se_muxsel_lines);
   }

   /* RLC uses the following order: Global, SE0, SE1, SE2, SE3, SE4, SE5. */
   ac_spm_fill_muxsel_ram(info, spm, AC_SPM_SEGMENT_TYPE_GLOBAL, 0);

   const uint32_t num_global_lines = spm->num_muxsel_lines[AC_SPM_SEGMENT_TYPE_GLOBAL];

   if (info->gfx_level >= GFX11) {
      /* On GFX11, RLC uses one segment size for every single SE. */
      for (unsigned i = 0; i < info->num_se; i++) {
         assert(i < AC_SPM_SEGMENT_TYPE_GLOBAL);
         uint32_t offset = num_global_lines + i * spm->max_se_muxsel_lines;

         ac_spm_fill_muxsel_ram(info, spm, i, offset);
      }
   } else {
      uint32_t offset = num_global_lines;

      for (unsigned i = 0; i < info->num_se; i++) {
         assert(i < AC_SPM_SEGMENT_TYPE_GLOBAL);

         ac_spm_fill_muxsel_ram(info, spm, i, offset);

         offset += spm->num_muxsel_lines[i];
      }
   }

   /* On GFX11, the data size written by the hw is in units of segment. */
   spm->ptr_granularity = info->gfx_level >= GFX11 ? 32 : 1;

   return true;
}

void ac_destroy_spm(struct ac_spm *spm)
{
   for (unsigned s = 0; s < AC_SPM_SEGMENT_TYPE_COUNT; s++) {
      FREE(spm->muxsel_lines[s]);
   }

   for (unsigned i = 0; i < spm->num_block_sel; i++) {
      FREE(spm->block_sel[i].instances);
   }

   FREE(spm->block_sel);
   FREE(spm->counters);
}

static uint32_t ac_spm_get_sample_size(const struct ac_spm *spm)
{
   uint32_t sample_size = 0; /* in bytes */

   for (unsigned s = 0; s < AC_SPM_SEGMENT_TYPE_COUNT; s++) {
      sample_size += spm->num_muxsel_lines[s] * AC_SPM_MUXSEL_LINE_SIZE * 4;
   }

   return sample_size;
}

static uint32_t ac_spm_get_num_samples(const struct ac_spm *spm)
{
   uint32_t sample_size = ac_spm_get_sample_size(spm);
   uint32_t *ptr = (uint32_t *)spm->ptr;
   uint32_t data_size, num_lines_written;
   uint32_t num_samples = 0;

   /* Get the data size (in bytes) written by the hw to the ring buffer. */
   data_size = ptr[0] * spm->ptr_granularity;

   /* Compute the number of 256 bits (16 * 16-bits counters) lines written. */
   num_lines_written = data_size / (2 * AC_SPM_NUM_COUNTER_PER_MUXSEL);

   /* Check for overflow. */
   if (num_lines_written % (sample_size / 32)) {
      abort();
   } else {
      num_samples = num_lines_written / (sample_size / 32);
   }

   return num_samples;
}

void ac_spm_get_trace(const struct ac_spm *spm, struct ac_spm_trace *trace)
{
   memset(trace, 0, sizeof(*trace));

   trace->ptr = spm->ptr;
   trace->sample_interval = spm->sample_interval;
   trace->num_counters = spm->num_counters;
   trace->counters = spm->counters;
   trace->sample_size_in_bytes = ac_spm_get_sample_size(spm);
   trace->num_samples = ac_spm_get_num_samples(spm);
}
