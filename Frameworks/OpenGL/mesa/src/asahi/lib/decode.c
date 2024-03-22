/*
 * Copyright 2017-2019 Alyssa Rosenzweig
 * Copyright 2017-2019 Connor Abbott
 * Copyright 2019 Collabora, Ltd.
 * SPDX-License-Identifier: MIT
 */

#include <ctype.h>
#include <memory.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <agx_pack.h>

#include "util/u_hexdump.h"
#include "decode.h"
#ifdef __APPLE__
#include "agx_iokit.h"
#endif

/* Pending UAPI */
struct drm_asahi_params_global {
   int gpu_generation;
   int gpu_variant;
   int chip_id;
   int num_clusters_total;
};

struct libagxdecode_config lib_config;

UNUSED static const char *agx_alloc_types[AGX_NUM_ALLOC] = {"mem", "map",
                                                            "cmd"};

static void
agx_disassemble(void *_code, size_t maxlen, FILE *fp)
{
   /* stub */
}

FILE *agxdecode_dump_stream;

#define MAX_MAPPINGS 4096

struct agx_bo mmap_array[MAX_MAPPINGS];
unsigned mmap_count = 0;

struct agx_bo *ro_mappings[MAX_MAPPINGS];
unsigned ro_mapping_count = 0;

static struct agx_bo *
agxdecode_find_mapped_gpu_mem_containing_rw(uint64_t addr)
{
   for (unsigned i = 0; i < mmap_count; ++i) {
      if (mmap_array[i].type == AGX_ALLOC_REGULAR &&
          addr >= mmap_array[i].ptr.gpu &&
          (addr - mmap_array[i].ptr.gpu) < mmap_array[i].size)
         return mmap_array + i;
   }

   return NULL;
}

static struct agx_bo *
agxdecode_find_mapped_gpu_mem_containing(uint64_t addr)
{
   struct agx_bo *mem = agxdecode_find_mapped_gpu_mem_containing_rw(addr);

   if (mem && mem->ptr.cpu && !mem->ro) {
      mprotect(mem->ptr.cpu, mem->size, PROT_READ);
      mem->ro = true;
      ro_mappings[ro_mapping_count++] = mem;
      assert(ro_mapping_count < MAX_MAPPINGS);
   }

   if (mem && !mem->mapped) {
      fprintf(stderr,
              "[ERROR] access to memory not mapped (GPU %" PRIx64
              ", handle %u)\n",
              mem->ptr.gpu, mem->handle);
   }

   return mem;
}

static struct agx_bo *
agxdecode_find_handle(unsigned handle, unsigned type)
{
   for (unsigned i = 0; i < mmap_count; ++i) {
      if (mmap_array[i].type != type)
         continue;

      if (mmap_array[i].handle != handle)
         continue;

      return &mmap_array[i];
   }

   return NULL;
}

static void
agxdecode_mark_mapped(unsigned handle)
{
   struct agx_bo *bo = agxdecode_find_handle(handle, AGX_ALLOC_REGULAR);

   if (!bo) {
      fprintf(stderr, "ERROR - unknown BO mapped with handle %u\n", handle);
      return;
   }

   /* Mark mapped for future consumption */
   bo->mapped = true;
}

#ifdef __APPLE__

static void
agxdecode_decode_segment_list(void *segment_list)
{
   unsigned nr_handles = 0;

   /* First, mark everything unmapped */
   for (unsigned i = 0; i < mmap_count; ++i)
      mmap_array[i].mapped = false;

   /* Check the header */
   struct agx_map_header *hdr = segment_list;
   if (hdr->resource_group_count == 0) {
      fprintf(agxdecode_dump_stream, "ERROR - empty map\n");
      return;
   }

   if (hdr->segment_count != 1) {
      fprintf(agxdecode_dump_stream, "ERROR - can't handle segment count %u\n",
              hdr->segment_count);
   }

   fprintf(agxdecode_dump_stream, "Segment list:\n");
   fprintf(agxdecode_dump_stream, "  Command buffer shmem ID: %" PRIx64 "\n",
           hdr->cmdbuf_id);
   fprintf(agxdecode_dump_stream, "  Encoder ID: %" PRIx64 "\n",
           hdr->encoder_id);
   fprintf(agxdecode_dump_stream, "  Kernel commands start offset: %u\n",
           hdr->kernel_commands_start_offset);
   fprintf(agxdecode_dump_stream, "  Kernel commands end offset: %u\n",
           hdr->kernel_commands_end_offset);
   fprintf(agxdecode_dump_stream, "  Unknown: 0x%X\n", hdr->unk);

   /* Expected structure: header followed by resource groups */
   size_t length = sizeof(struct agx_map_header);
   length += sizeof(struct agx_map_entry) * hdr->resource_group_count;

   if (length != hdr->length) {
      fprintf(agxdecode_dump_stream, "ERROR: expected length %zu, got %u\n",
              length, hdr->length);
   }

   if (hdr->padding[0] || hdr->padding[1])
      fprintf(agxdecode_dump_stream, "ERROR - padding tripped\n");

   /* Check the entries */
   struct agx_map_entry *groups = ((void *)hdr) + sizeof(*hdr);
   for (unsigned i = 0; i < hdr->resource_group_count; ++i) {
      struct agx_map_entry group = groups[i];
      unsigned count = group.resource_count;

      STATIC_ASSERT(ARRAY_SIZE(group.resource_id) == 6);
      STATIC_ASSERT(ARRAY_SIZE(group.resource_unk) == 6);
      STATIC_ASSERT(ARRAY_SIZE(group.resource_flags) == 6);

      if ((count < 1) || (count > 6)) {
         fprintf(agxdecode_dump_stream, "ERROR - invalid count %u\n", count);
         continue;
      }

      for (unsigned j = 0; j < count; ++j) {
         unsigned handle = group.resource_id[j];
         unsigned unk = group.resource_unk[j];
         unsigned flags = group.resource_flags[j];

         if (!handle) {
            fprintf(agxdecode_dump_stream, "ERROR - invalid handle %u\n",
                    handle);
            continue;
         }

         agxdecode_mark_mapped(handle);
         nr_handles++;

         fprintf(agxdecode_dump_stream, "%u (0x%X, 0x%X)\n", handle, unk,
                 flags);
      }

      if (group.unka)
         fprintf(agxdecode_dump_stream, "ERROR - unknown 0x%X\n", group.unka);

      /* Visual separator for resource groups */
      fprintf(agxdecode_dump_stream, "\n");
   }

   /* Check the handle count */
   if (nr_handles != hdr->total_resources) {
      fprintf(agxdecode_dump_stream,
              "ERROR - wrong handle count, got %u, expected %u (%u entries)\n",
              nr_handles, hdr->total_resources, hdr->resource_group_count);
   }
}

#endif

static size_t
__agxdecode_fetch_gpu_mem(const struct agx_bo *mem, uint64_t gpu_va,
                          size_t size, void *buf, int line,
                          const char *filename)
{
   if (lib_config.read_gpu_mem)
      return lib_config.read_gpu_mem(gpu_va, size, buf);

   if (!mem)
      mem = agxdecode_find_mapped_gpu_mem_containing(gpu_va);

   if (!mem) {
      fprintf(stderr, "Access to unknown memory %" PRIx64 " in %s:%d\n", gpu_va,
              filename, line);
      fflush(agxdecode_dump_stream);
      assert(0);
   }

   assert(mem);

   if (size + (gpu_va - mem->ptr.gpu) > mem->size) {
      fprintf(stderr,
              "Overflowing to unknown memory %" PRIx64
              " of size %zu (max size %zu) in %s:%d\n",
              gpu_va, size, (size_t)(mem->size - (gpu_va - mem->ptr.gpu)),
              filename, line);
      fflush(agxdecode_dump_stream);
      assert(0);
   }

   memcpy(buf, mem->ptr.cpu + gpu_va - mem->ptr.gpu, size);

   return size;
}

#define agxdecode_fetch_gpu_mem(gpu_va, size, buf)                             \
   __agxdecode_fetch_gpu_mem(NULL, gpu_va, size, buf, __LINE__, __FILE__)

#define agxdecode_fetch_gpu_array(gpu_va, buf)                                 \
   agxdecode_fetch_gpu_mem(gpu_va, sizeof(buf), buf)

static void
agxdecode_map_read_write(void)
{
   for (unsigned i = 0; i < ro_mapping_count; ++i) {
      ro_mappings[i]->ro = false;
      mprotect(ro_mappings[i]->ptr.cpu, ro_mappings[i]->size,
               PROT_READ | PROT_WRITE);
   }

   ro_mapping_count = 0;
}

/* Helpers for parsing the cmdstream */

#define DUMP_UNPACKED(T, var, str)                                             \
   {                                                                           \
      agxdecode_log(str);                                                      \
      agx_print(agxdecode_dump_stream, T, var, (agxdecode_indent + 1) * 2);    \
   }

#define DUMP_CL(T, cl, str)                                                    \
   {                                                                           \
      agx_unpack(agxdecode_dump_stream, cl, T, temp);                          \
      DUMP_UNPACKED(T, temp, str "\n");                                        \
   }

#define agxdecode_log(str) fputs(str, agxdecode_dump_stream)
#define agxdecode_msg(str) fprintf(agxdecode_dump_stream, "// %s", str)

unsigned agxdecode_indent = 0;

typedef struct drm_asahi_params_global decoder_params;

/* Abstraction for command stream parsing */
typedef unsigned (*decode_cmd)(const uint8_t *map, uint64_t *link, bool verbose,
                               decoder_params *params, void *data);

#define STATE_DONE (0xFFFFFFFFu)
#define STATE_LINK (0xFFFFFFFEu)
#define STATE_CALL (0xFFFFFFFDu)
#define STATE_RET  (0xFFFFFFFCu)

static void
agxdecode_stateful(uint64_t va, const char *label, decode_cmd decoder,
                   bool verbose, decoder_params *params, void *data)
{
   uint64_t stack[16];
   unsigned sp = 0;

   uint8_t buf[1024];
   if (!lib_config.read_gpu_mem) {
      struct agx_bo *alloc = agxdecode_find_mapped_gpu_mem_containing(va);
      assert(alloc != NULL && "nonexistent object");
      fprintf(agxdecode_dump_stream, "%s (%" PRIx64 ", handle %u)\n", label, va,
              alloc->handle);
   } else {
      fprintf(agxdecode_dump_stream, "%s (%" PRIx64 ")\n", label, va);
   }
   fflush(agxdecode_dump_stream);

   int len = agxdecode_fetch_gpu_array(va, buf);

   int left = len;
   uint8_t *map = buf;
   uint64_t link = 0;

   fflush(agxdecode_dump_stream);

   while (left) {
      if (len <= 0) {
         fprintf(agxdecode_dump_stream, "!! Failed to read GPU memory\n");
         fflush(agxdecode_dump_stream);
         return;
      }

      unsigned count = decoder(map, &link, verbose, params, data);

      /* If we fail to decode, default to a hexdump (don't hang) */
      if (count == 0) {
         u_hexdump(agxdecode_dump_stream, map, 8, false);
         count = 8;
      }

      fflush(agxdecode_dump_stream);
      if (count == STATE_DONE) {
         break;
      } else if (count == STATE_LINK) {
         fprintf(agxdecode_dump_stream, "Linking to 0x%" PRIx64 "\n\n", link);
         va = link;
         left = len = agxdecode_fetch_gpu_array(va, buf);
         map = buf;
      } else if (count == STATE_CALL) {
         fprintf(agxdecode_dump_stream,
                 "Calling 0x%" PRIx64 " (return = 0x%" PRIx64 ")\n\n", link,
                 va + 8);
         assert(sp < ARRAY_SIZE(stack));
         stack[sp++] = va + 8;
         va = link;
         left = len = agxdecode_fetch_gpu_array(va, buf);
         map = buf;
      } else if (count == STATE_RET) {
         assert(sp > 0);
         va = stack[--sp];
         fprintf(agxdecode_dump_stream, "Returning to 0x%" PRIx64 "\n\n", va);
         left = len = agxdecode_fetch_gpu_array(va, buf);
         map = buf;
      } else {
         va += count;
         map += count;
         left -= count;

         if (left < 512 && len == sizeof(buf)) {
            left = len = agxdecode_fetch_gpu_array(va, buf);
            map = buf;
         }
      }
   }
}

static unsigned
agxdecode_usc(const uint8_t *map, UNUSED uint64_t *link, UNUSED bool verbose,
              decoder_params *params, UNUSED void *data)
{
   enum agx_sampler_states *sampler_states = data;
   enum agx_usc_control type = map[0];
   uint8_t buf[8192];

   bool extended_samplers =
      (sampler_states != NULL) &&
      (((*sampler_states) == AGX_SAMPLER_STATES_8_EXTENDED) ||
       ((*sampler_states) == AGX_SAMPLER_STATES_16_EXTENDED));

#define USC_CASE(name, human)                                                  \
   case AGX_USC_CONTROL_##name: {                                              \
      DUMP_CL(USC_##name, map, human);                                         \
      return AGX_USC_##name##_LENGTH;                                          \
   }

   switch (type) {
   case AGX_USC_CONTROL_NO_PRESHADER: {
      DUMP_CL(USC_NO_PRESHADER, map, "No preshader");
      return STATE_DONE;
   }

   case AGX_USC_CONTROL_PRESHADER: {
      agx_unpack(agxdecode_dump_stream, map, USC_PRESHADER, ctrl);
      DUMP_UNPACKED(USC_PRESHADER, ctrl, "Preshader\n");

      agx_disassemble(buf, agxdecode_fetch_gpu_array(ctrl.code, buf),
                      agxdecode_dump_stream);

      return STATE_DONE;
   }

   case AGX_USC_CONTROL_SHADER: {
      agx_unpack(agxdecode_dump_stream, map, USC_SHADER, ctrl);
      DUMP_UNPACKED(USC_SHADER, ctrl, "Shader\n");

      agxdecode_log("\n");
      agx_disassemble(buf, agxdecode_fetch_gpu_array(ctrl.code, buf),
                      agxdecode_dump_stream);
      agxdecode_log("\n");

      return AGX_USC_SHADER_LENGTH;
   }

   case AGX_USC_CONTROL_SAMPLER: {
      agx_unpack(agxdecode_dump_stream, map, USC_SAMPLER, temp);
      DUMP_UNPACKED(USC_SAMPLER, temp, "Sampler state\n");

      uint8_t buf[(AGX_SAMPLER_LENGTH + AGX_BORDER_LENGTH) * temp.count];
      uint8_t *samp = buf;

      agxdecode_fetch_gpu_array(temp.buffer, buf);

      for (unsigned i = 0; i < temp.count; ++i) {
         DUMP_CL(SAMPLER, samp, "Sampler");
         samp += AGX_SAMPLER_LENGTH;

         if (extended_samplers) {
            DUMP_CL(BORDER, samp, "Border");
            samp += AGX_BORDER_LENGTH;
         }
      }

      return AGX_USC_SAMPLER_LENGTH;
   }

   case AGX_USC_CONTROL_TEXTURE: {
      agx_unpack(agxdecode_dump_stream, map, USC_TEXTURE, temp);
      DUMP_UNPACKED(USC_TEXTURE, temp, "Texture state\n");

      uint8_t buf[AGX_TEXTURE_LENGTH * temp.count];
      uint8_t *tex = buf;

      agxdecode_fetch_gpu_array(temp.buffer, buf);

      /* Note: samplers only need 8 byte alignment? */
      for (unsigned i = 0; i < temp.count; ++i) {
         agx_unpack(agxdecode_dump_stream, tex, TEXTURE, t);
         DUMP_CL(TEXTURE, tex, "Texture");
         DUMP_CL(PBE, tex, "PBE");

         tex += AGX_TEXTURE_LENGTH;
      }

      return AGX_USC_TEXTURE_LENGTH;
   }

   case AGX_USC_CONTROL_UNIFORM: {
      agx_unpack(agxdecode_dump_stream, map, USC_UNIFORM, temp);
      DUMP_UNPACKED(USC_UNIFORM, temp, "Uniform\n");

      uint8_t buf[2 * temp.size_halfs];
      agxdecode_fetch_gpu_array(temp.buffer, buf);
      u_hexdump(agxdecode_dump_stream, buf, 2 * temp.size_halfs, false);

      return AGX_USC_UNIFORM_LENGTH;
   }

      USC_CASE(FRAGMENT_PROPERTIES, "Fragment properties");
      USC_CASE(UNIFORM_HIGH, "Uniform high");
      USC_CASE(SHARED, "Shared");
      USC_CASE(REGISTERS, "Registers");

   default:
      fprintf(agxdecode_dump_stream, "Unknown USC control type: %u\n", type);
      u_hexdump(agxdecode_dump_stream, map, 8, false);
      return 8;
   }

#undef USC_CASE
}

#define PPP_PRINT(map, header_name, struct_name, human)                        \
   if (hdr.header_name) {                                                      \
      if (((map + AGX_##struct_name##_LENGTH) > (base + size))) {              \
         fprintf(agxdecode_dump_stream, "Buffer overrun in PPP update\n");     \
         return;                                                               \
      }                                                                        \
      DUMP_CL(struct_name, map, human);                                        \
      map += AGX_##struct_name##_LENGTH;                                       \
      fflush(agxdecode_dump_stream);                                           \
   }

static void
agxdecode_record(uint64_t va, size_t size, bool verbose, decoder_params *params)
{
   uint8_t buf[size];
   uint8_t *base = buf;
   uint8_t *map = base;

   agxdecode_fetch_gpu_array(va, buf);

   agx_unpack(agxdecode_dump_stream, map, PPP_HEADER, hdr);
   map += AGX_PPP_HEADER_LENGTH;

   PPP_PRINT(map, fragment_control, FRAGMENT_CONTROL, "Fragment control");
   PPP_PRINT(map, fragment_control_2, FRAGMENT_CONTROL, "Fragment control 2");
   PPP_PRINT(map, fragment_front_face, FRAGMENT_FACE, "Front face");
   PPP_PRINT(map, fragment_front_face_2, FRAGMENT_FACE_2, "Front face 2");
   PPP_PRINT(map, fragment_front_stencil, FRAGMENT_STENCIL, "Front stencil");
   PPP_PRINT(map, fragment_back_face, FRAGMENT_FACE, "Back face");
   PPP_PRINT(map, fragment_back_face_2, FRAGMENT_FACE_2, "Back face 2");
   PPP_PRINT(map, fragment_back_stencil, FRAGMENT_STENCIL, "Back stencil");
   PPP_PRINT(map, depth_bias_scissor, DEPTH_BIAS_SCISSOR, "Depth bias/scissor");

   if (hdr.region_clip) {
      if (((map + (AGX_REGION_CLIP_LENGTH * hdr.viewport_count)) >
           (base + size))) {
         fprintf(agxdecode_dump_stream, "Buffer overrun in PPP update\n");
         return;
      }

      for (unsigned i = 0; i < hdr.viewport_count; ++i) {
         DUMP_CL(REGION_CLIP, map, "Region clip");
         map += AGX_REGION_CLIP_LENGTH;
         fflush(agxdecode_dump_stream);
      }
   }

   if (hdr.viewport) {
      if (((map + AGX_VIEWPORT_CONTROL_LENGTH +
            (AGX_VIEWPORT_LENGTH * hdr.viewport_count)) > (base + size))) {
         fprintf(agxdecode_dump_stream, "Buffer overrun in PPP update\n");
         return;
      }

      DUMP_CL(VIEWPORT_CONTROL, map, "Viewport control");
      map += AGX_VIEWPORT_CONTROL_LENGTH;

      for (unsigned i = 0; i < hdr.viewport_count; ++i) {
         DUMP_CL(VIEWPORT, map, "Viewport");
         map += AGX_VIEWPORT_LENGTH;
         fflush(agxdecode_dump_stream);
      }
   }

   PPP_PRINT(map, w_clamp, W_CLAMP, "W clamp");
   PPP_PRINT(map, output_select, OUTPUT_SELECT, "Output select");
   PPP_PRINT(map, varying_counts_32, VARYING_COUNTS, "Varying counts 32");
   PPP_PRINT(map, varying_counts_16, VARYING_COUNTS, "Varying counts 16");
   PPP_PRINT(map, cull, CULL, "Cull");
   PPP_PRINT(map, cull_2, CULL_2, "Cull 2");

   if (hdr.fragment_shader) {
      agx_unpack(agxdecode_dump_stream, map, FRAGMENT_SHADER, frag);
      agxdecode_stateful(frag.pipeline, "Fragment pipeline", agxdecode_usc,
                         verbose, params, &frag.sampler_state_register_count);

      if (frag.cf_bindings) {
         uint8_t buf[128];
         uint8_t *cf = buf;

         agxdecode_fetch_gpu_array(frag.cf_bindings, buf);
         u_hexdump(agxdecode_dump_stream, cf, 128, false);

         DUMP_CL(CF_BINDING_HEADER, cf, "Coefficient binding header:");
         cf += AGX_CF_BINDING_HEADER_LENGTH;

         for (unsigned i = 0; i < frag.cf_binding_count; ++i) {
            DUMP_CL(CF_BINDING, cf, "Coefficient binding:");
            cf += AGX_CF_BINDING_LENGTH;
         }
      }

      DUMP_UNPACKED(FRAGMENT_SHADER, frag, "Fragment shader\n");
      map += AGX_FRAGMENT_SHADER_LENGTH;
   }

   PPP_PRINT(map, occlusion_query, FRAGMENT_OCCLUSION_QUERY, "Occlusion query");
   PPP_PRINT(map, occlusion_query_2, FRAGMENT_OCCLUSION_QUERY_2,
             "Occlusion query 2");
   PPP_PRINT(map, output_unknown, OUTPUT_UNKNOWN, "Output unknown");
   PPP_PRINT(map, output_size, OUTPUT_SIZE, "Output size");
   PPP_PRINT(map, varying_word_2, VARYING_2, "Varying word 2");

   /* PPP print checks we don't read too much, now check we read enough */
   assert(map == (base + size) && "invalid size of PPP update");
}

static unsigned
agxdecode_cdm(const uint8_t *map, uint64_t *link, bool verbose,
              decoder_params *params, UNUSED void *data)
{
   /* Bits 29-31 contain the block type */
   enum agx_cdm_block_type block_type = (map[3] >> 5);

   switch (block_type) {
   case AGX_CDM_BLOCK_TYPE_LAUNCH: {
      size_t length = AGX_CDM_LAUNCH_LENGTH;

#define CDM_PRINT(STRUCT_NAME, human)                                          \
   do {                                                                        \
      DUMP_CL(CDM_##STRUCT_NAME, map, human);                                  \
      map += AGX_CDM_##STRUCT_NAME##_LENGTH;                                   \
      length += AGX_CDM_##STRUCT_NAME##_LENGTH;                                \
   } while (0);

      agx_unpack(agxdecode_dump_stream, map, CDM_LAUNCH, hdr);
      agxdecode_stateful(hdr.pipeline, "Pipeline", agxdecode_usc, verbose,
                         params, &hdr.sampler_state_register_count);
      DUMP_UNPACKED(CDM_LAUNCH, hdr, "Compute\n");
      map += AGX_CDM_LAUNCH_LENGTH;

      /* Added in G14X */
      if (params->gpu_generation >= 14 && params->num_clusters_total > 1)
         CDM_PRINT(UNK_G14X, "Unknown G14X");

      switch (hdr.mode) {
      case AGX_CDM_MODE_DIRECT:
         CDM_PRINT(GLOBAL_SIZE, "Global size");
         CDM_PRINT(LOCAL_SIZE, "Local size");
         break;
      case AGX_CDM_MODE_INDIRECT_GLOBAL:
         CDM_PRINT(INDIRECT, "Indirect buffer");
         CDM_PRINT(LOCAL_SIZE, "Local size");
         break;
      case AGX_CDM_MODE_INDIRECT_LOCAL:
         CDM_PRINT(INDIRECT, "Indirect buffer");
         break;
      default:
         fprintf(agxdecode_dump_stream, "Unknown CDM mode: %u\n", hdr.mode);
         break;
      }

      return length;
   }

   case AGX_CDM_BLOCK_TYPE_STREAM_LINK: {
      agx_unpack(agxdecode_dump_stream, map, CDM_STREAM_LINK, hdr);
      DUMP_UNPACKED(CDM_STREAM_LINK, hdr, "Stream Link\n");
      *link = hdr.target_lo | (((uint64_t)hdr.target_hi) << 32);
      return STATE_LINK;
   }

   case AGX_CDM_BLOCK_TYPE_STREAM_TERMINATE: {
      DUMP_CL(CDM_STREAM_TERMINATE, map, "Stream Terminate");
      return STATE_DONE;
   }

   case AGX_CDM_BLOCK_TYPE_BARRIER: {
      DUMP_CL(CDM_BARRIER, map, "Barrier");
      return AGX_CDM_BARRIER_LENGTH;
   }

   default:
      fprintf(agxdecode_dump_stream, "Unknown CDM block type: %u\n",
              block_type);
      u_hexdump(agxdecode_dump_stream, map, 8, false);
      return 8;
   }
}

static unsigned
agxdecode_vdm(const uint8_t *map, uint64_t *link, bool verbose,
              decoder_params *params, UNUSED void *data)
{
   /* Bits 29-31 contain the block type */
   enum agx_vdm_block_type block_type = (map[3] >> 5);

   switch (block_type) {
   case AGX_VDM_BLOCK_TYPE_BARRIER: {
      agx_unpack(agxdecode_dump_stream, map, VDM_BARRIER, hdr);
      DUMP_UNPACKED(VDM_BARRIER, hdr, "Barrier\n");
      return hdr.returns ? STATE_RET : AGX_VDM_BARRIER_LENGTH;
   }

   case AGX_VDM_BLOCK_TYPE_PPP_STATE_UPDATE: {
      agx_unpack(agxdecode_dump_stream, map, PPP_STATE, cmd);

      uint64_t address = (((uint64_t)cmd.pointer_hi) << 32) | cmd.pointer_lo;

      if (!lib_config.read_gpu_mem) {
         struct agx_bo *mem = agxdecode_find_mapped_gpu_mem_containing(address);

         if (!mem) {
            DUMP_UNPACKED(PPP_STATE, cmd, "Non-existent record (XXX)\n");
            return AGX_PPP_STATE_LENGTH;
         }
      }

      agxdecode_record(address, cmd.size_words * 4, verbose, params);
      return AGX_PPP_STATE_LENGTH;
   }

   case AGX_VDM_BLOCK_TYPE_VDM_STATE_UPDATE: {
      size_t length = AGX_VDM_STATE_LENGTH;
      agx_unpack(agxdecode_dump_stream, map, VDM_STATE, hdr);
      map += AGX_VDM_STATE_LENGTH;

#define VDM_PRINT(header_name, STRUCT_NAME, human)                             \
   if (hdr.header_name##_present) {                                            \
      DUMP_CL(VDM_STATE_##STRUCT_NAME, map, human);                            \
      map += AGX_VDM_STATE_##STRUCT_NAME##_LENGTH;                             \
      length += AGX_VDM_STATE_##STRUCT_NAME##_LENGTH;                          \
   }

      VDM_PRINT(restart_index, RESTART_INDEX, "Restart index");

      /* If word 1 is present but word 0 is not, fallback to compact samplers */
      enum agx_sampler_states sampler_states = 0;

      if (hdr.vertex_shader_word_0_present) {
         agx_unpack(agxdecode_dump_stream, map, VDM_STATE_VERTEX_SHADER_WORD_0,
                    word_0);
         sampler_states = word_0.sampler_state_register_count;
      }

      VDM_PRINT(vertex_shader_word_0, VERTEX_SHADER_WORD_0,
                "Vertex shader word 0");

      if (hdr.vertex_shader_word_1_present) {
         agx_unpack(agxdecode_dump_stream, map, VDM_STATE_VERTEX_SHADER_WORD_1,
                    word_1);
         fprintf(agxdecode_dump_stream, "Pipeline %X\n",
                 (uint32_t)word_1.pipeline);
         agxdecode_stateful(word_1.pipeline, "Pipeline", agxdecode_usc, verbose,
                            params, &sampler_states);
      }

      VDM_PRINT(vertex_shader_word_1, VERTEX_SHADER_WORD_1,
                "Vertex shader word 1");
      VDM_PRINT(vertex_outputs, VERTEX_OUTPUTS, "Vertex outputs");
      VDM_PRINT(tessellation, TESSELLATION, "Tessellation");
      VDM_PRINT(vertex_unknown, VERTEX_UNKNOWN, "Vertex unknown");
      VDM_PRINT(tessellation_scale, TESSELLATION_SCALE, "Tessellation scale");

#undef VDM_PRINT
      return hdr.tessellation_scale_present ? length : ALIGN_POT(length, 8);
   }

   case AGX_VDM_BLOCK_TYPE_INDEX_LIST: {
      size_t length = AGX_INDEX_LIST_LENGTH;
      agx_unpack(agxdecode_dump_stream, map, INDEX_LIST, hdr);
      DUMP_UNPACKED(INDEX_LIST, hdr, "Index List\n");
      map += AGX_INDEX_LIST_LENGTH;

#define IDX_PRINT(header_name, STRUCT_NAME, human)                             \
   if (hdr.header_name##_present) {                                            \
      DUMP_CL(INDEX_LIST_##STRUCT_NAME, map, human);                           \
      map += AGX_INDEX_LIST_##STRUCT_NAME##_LENGTH;                            \
      length += AGX_INDEX_LIST_##STRUCT_NAME##_LENGTH;                         \
   }

      IDX_PRINT(index_buffer, BUFFER_LO, "Index buffer");
      IDX_PRINT(index_count, COUNT, "Index count");
      IDX_PRINT(instance_count, INSTANCES, "Instance count");
      IDX_PRINT(start, START, "Start");
      IDX_PRINT(indirect_buffer, INDIRECT_BUFFER, "Indirect buffer");
      IDX_PRINT(index_buffer_size, BUFFER_SIZE, "Index buffer size");

#undef IDX_PRINT
      return length;
   }

   case AGX_VDM_BLOCK_TYPE_STREAM_LINK: {
      agx_unpack(agxdecode_dump_stream, map, VDM_STREAM_LINK, hdr);
      DUMP_UNPACKED(VDM_STREAM_LINK, hdr, "Stream Link\n");
      *link = hdr.target_lo | (((uint64_t)hdr.target_hi) << 32);
      return hdr.with_return ? STATE_CALL : STATE_LINK;
   }

   case AGX_VDM_BLOCK_TYPE_STREAM_TERMINATE: {
      DUMP_CL(VDM_STREAM_TERMINATE, map, "Stream Terminate");
      return STATE_DONE;
   }

   case AGX_VDM_BLOCK_TYPE_TESSELLATE: {
      size_t length = AGX_VDM_TESSELLATE_LENGTH;
      agx_unpack(agxdecode_dump_stream, map, VDM_TESSELLATE, hdr);
      DUMP_UNPACKED(VDM_TESSELLATE, hdr, "Tessellate List\n");
      map += AGX_VDM_TESSELLATE_LENGTH;

#define TESS_PRINT(header_name, STRUCT_NAME, human)                            \
   if (hdr.header_name##_present) {                                            \
      DUMP_CL(VDM_TESSELLATE_##STRUCT_NAME, map, human);                       \
      map += AGX_VDM_TESSELLATE_##STRUCT_NAME##_LENGTH;                        \
      length += AGX_VDM_TESSELLATE_##STRUCT_NAME##_LENGTH;                     \
   }

      TESS_PRINT(factor_buffer, FACTOR_BUFFER, "Factor buffer");
      TESS_PRINT(patch_count, PATCH_COUNT, "Patch");
      TESS_PRINT(instance_count, INSTANCE_COUNT, "Instance count");
      TESS_PRINT(base_patch, BASE_PATCH, "Base patch");
      TESS_PRINT(base_instance, BASE_INSTANCE, "Base instance");
      TESS_PRINT(instance_stride, INSTANCE_STRIDE, "Instance stride");
      TESS_PRINT(indirect, INDIRECT, "Indirect");
      TESS_PRINT(unknown, UNKNOWN, "Unknown");

#undef TESS_PRINT
      return length;
   }

   default:
      fprintf(agxdecode_dump_stream, "Unknown VDM block type: %u\n",
              block_type);
      u_hexdump(agxdecode_dump_stream, map, 8, false);
      return 8;
   }
}

static void
agxdecode_cs(uint32_t *cmdbuf, uint64_t encoder, bool verbose,
             decoder_params *params)
{
   agx_unpack(agxdecode_dump_stream, cmdbuf + 16, IOGPU_COMPUTE, cs);
   DUMP_UNPACKED(IOGPU_COMPUTE, cs, "Compute\n");

   agxdecode_stateful(encoder, "Encoder", agxdecode_cdm, verbose, params, NULL);

   fprintf(agxdecode_dump_stream, "Context switch program:\n");
   uint8_t buf[1024];
   agx_disassemble(buf,
                   agxdecode_fetch_gpu_array(cs.context_switch_program, buf),
                   agxdecode_dump_stream);
}

static void
agxdecode_gfx(uint32_t *cmdbuf, uint64_t encoder, bool verbose,
              decoder_params *params)
{
   agx_unpack(agxdecode_dump_stream, cmdbuf + 16, IOGPU_GRAPHICS, gfx);
   DUMP_UNPACKED(IOGPU_GRAPHICS, gfx, "Graphics\n");

   agxdecode_stateful(encoder, "Encoder", agxdecode_vdm, verbose, params, NULL);

   if (gfx.clear_pipeline_unk) {
      fprintf(agxdecode_dump_stream, "Unk: %X\n", gfx.clear_pipeline_unk);
      agxdecode_stateful(gfx.clear_pipeline, "Clear pipeline", agxdecode_usc,
                         verbose, params, NULL);
   }

   if (gfx.store_pipeline_unk) {
      assert(gfx.store_pipeline_unk == 0x4);
      agxdecode_stateful(gfx.store_pipeline, "Store pipeline", agxdecode_usc,
                         verbose, params, NULL);
   }

   assert((gfx.partial_reload_pipeline_unk & 0xF) == 0x4);
   if (gfx.partial_reload_pipeline) {
      agxdecode_stateful(gfx.partial_reload_pipeline, "Partial reload pipeline",
                         agxdecode_usc, verbose, params, NULL);
   }

   if (gfx.partial_store_pipeline) {
      agxdecode_stateful(gfx.partial_store_pipeline, "Partial store pipeline",
                         agxdecode_usc, verbose, params, NULL);
   }
}

static void
chip_id_to_params(decoder_params *params, uint32_t chip_id)
{
   switch (chip_id) {
   case 0x6000 ... 0x6002:
      *params = (decoder_params){
         .gpu_generation = 13,
         .gpu_variant = "SCD"[chip_id & 15],
         .chip_id = chip_id,
         .num_clusters_total = 2 << (chip_id & 15),
      };
      break;
   case 0x6020 ... 0x6022:
      *params = (decoder_params){
         .gpu_generation = 14,
         .gpu_variant = "SCD"[chip_id & 15],
         .chip_id = chip_id,
         .num_clusters_total = 2 << (chip_id & 15),
      };
      break;
   case 0x8112:
      *params = (decoder_params){
         .gpu_generation = 14,
         .gpu_variant = 'G',
         .chip_id = chip_id,
         .num_clusters_total = 1,
      };
      break;
   case 0x8103:
   default:
      *params = (decoder_params){
         .gpu_generation = 13,
         .gpu_variant = 'G',
         .chip_id = chip_id,
         .num_clusters_total = 1,
      };
      break;
   }
}

#ifdef __APPLE__

void
agxdecode_cmdstream(unsigned cmdbuf_handle, unsigned map_handle, bool verbose)
{
   agxdecode_dump_file_open();

   struct agx_bo *cmdbuf =
      agxdecode_find_handle(cmdbuf_handle, AGX_ALLOC_CMDBUF);
   struct agx_bo *map = agxdecode_find_handle(map_handle, AGX_ALLOC_MEMMAP);
   assert(cmdbuf != NULL && "nonexistent command buffer");
   assert(map != NULL && "nonexistent mapping");

   /* Before decoding anything, validate the map. Set bo->mapped fields */
   agxdecode_decode_segment_list(map->ptr.cpu);

   /* Print the IOGPU stuff */
   agx_unpack(agxdecode_dump_stream, cmdbuf->ptr.cpu, IOGPU_HEADER, cmd);
   DUMP_UNPACKED(IOGPU_HEADER, cmd, "IOGPU Header\n");

   DUMP_CL(IOGPU_ATTACHMENT_COUNT,
           ((uint8_t *)cmdbuf->ptr.cpu + cmd.attachment_offset),
           "Attachment count");

   uint32_t *attachments =
      (uint32_t *)((uint8_t *)cmdbuf->ptr.cpu + cmd.attachment_offset);
   unsigned attachment_count = attachments[3];
   for (unsigned i = 0; i < attachment_count; ++i) {
      uint32_t *ptr = attachments + 4 + (i * AGX_IOGPU_ATTACHMENT_LENGTH / 4);
      DUMP_CL(IOGPU_ATTACHMENT, ptr, "Attachment");
   }

   struct drm_asahi_params_global params;

   chip_id_to_params(&params, 0x8103);

   if (cmd.unk_5 == 3)
      agxdecode_cs((uint32_t *)cmdbuf->ptr.cpu, cmd.encoder, verbose, &params);
   else
      agxdecode_gfx((uint32_t *)cmdbuf->ptr.cpu, cmd.encoder, verbose, &params);

   agxdecode_map_read_write();
}

void
agxdecode_dump_mappings(unsigned map_handle)
{
   agxdecode_dump_file_open();

   struct agx_bo *map = agxdecode_find_handle(map_handle, AGX_ALLOC_MEMMAP);
   assert(map != NULL && "nonexistent mapping");
   agxdecode_decode_segment_list(map->ptr.cpu);

   for (unsigned i = 0; i < mmap_count; ++i) {
      if (!mmap_array[i].ptr.cpu || !mmap_array[i].size ||
          !mmap_array[i].mapped)
         continue;

      assert(mmap_array[i].type < AGX_NUM_ALLOC);

      fprintf(agxdecode_dump_stream,
              "Buffer: type %s, gpu %" PRIx64 ", handle %u.bin:\n\n",
              agx_alloc_types[mmap_array[i].type], mmap_array[i].ptr.gpu,
              mmap_array[i].handle);

      u_hexdump(agxdecode_dump_stream, mmap_array[i].ptr.cpu,
                mmap_array[i].size, false);
      fprintf(agxdecode_dump_stream, "\n");
   }
}

#endif

void
agxdecode_track_alloc(struct agx_bo *alloc)
{
   assert((mmap_count + 1) < MAX_MAPPINGS);

   for (unsigned i = 0; i < mmap_count; ++i) {
      struct agx_bo *bo = &mmap_array[i];
      bool match = (bo->handle == alloc->handle && bo->type == alloc->type);
      assert(!match && "tried to alloc already allocated BO");
   }

   mmap_array[mmap_count++] = *alloc;
}

void
agxdecode_track_free(struct agx_bo *bo)
{
   bool found = false;

   for (unsigned i = 0; i < mmap_count; ++i) {
      if (mmap_array[i].handle == bo->handle &&
          (mmap_array[i].type == AGX_ALLOC_REGULAR) ==
             (bo->type == AGX_ALLOC_REGULAR)) {
         assert(!found && "mapped multiple times!");
         found = true;

         memset(&mmap_array[i], 0, sizeof(mmap_array[i]));
      }
   }

   assert(found && "freed unmapped memory");
}

static int agxdecode_dump_frame_count = 0;

void
agxdecode_dump_file_open(void)
{
   if (agxdecode_dump_stream)
      return;

   /* This does a getenv every frame, so it is possible to use
    * setenv to change the base at runtime.
    */
   const char *dump_file_base =
      getenv("AGXDECODE_DUMP_FILE") ?: "agxdecode.dump";
   if (!strcmp(dump_file_base, "stderr"))
      agxdecode_dump_stream = stderr;
   else {
      char buffer[1024];
      snprintf(buffer, sizeof(buffer), "%s.%04d", dump_file_base,
               agxdecode_dump_frame_count);
      printf("agxdecode: dump command stream to file %s\n", buffer);
      agxdecode_dump_stream = fopen(buffer, "w");
      if (!agxdecode_dump_stream) {
         fprintf(stderr,
                 "agxdecode: failed to open command stream log file %s\n",
                 buffer);
      }
   }
}

static void
agxdecode_dump_file_close(void)
{
   if (agxdecode_dump_stream && agxdecode_dump_stream != stderr) {
      fclose(agxdecode_dump_stream);
      agxdecode_dump_stream = NULL;
   }
}

void
agxdecode_next_frame(void)
{
   agxdecode_dump_file_close();
   agxdecode_dump_frame_count++;
}

void
agxdecode_close(void)
{
   agxdecode_dump_file_close();
}

static ssize_t
libagxdecode_writer(void *cookie, const char *buffer, size_t size)
{
   return lib_config.stream_write(buffer, size);
}

#ifdef _GNU_SOURCE
static cookie_io_functions_t funcs = {.write = libagxdecode_writer};
#endif

static decoder_params lib_params;

void
libagxdecode_init(struct libagxdecode_config *config)
{
#ifdef _GNU_SOURCE
   lib_config = *config;
   agxdecode_dump_stream = fopencookie(NULL, "w", funcs);

   chip_id_to_params(&lib_params, config->chip_id);
#else
   /* fopencookie is a glibc extension */
   unreachable("libagxdecode only available with glibc");
#endif
}

void
libagxdecode_vdm(uint64_t addr, const char *label, bool verbose)
{
   agxdecode_stateful(addr, label, agxdecode_vdm, verbose, &lib_params, NULL);
}

void
libagxdecode_cdm(uint64_t addr, const char *label, bool verbose)
{
   agxdecode_stateful(addr, label, agxdecode_cdm, verbose, &lib_params, NULL);
}
void
libagxdecode_usc(uint64_t addr, const char *label, bool verbose)
{
   agxdecode_stateful(addr, label, agxdecode_usc, verbose, &lib_params, NULL);
}
void
libagxdecode_shutdown(void)
{
   agxdecode_dump_file_close();
}
