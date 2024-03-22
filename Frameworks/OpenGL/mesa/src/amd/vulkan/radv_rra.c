/*
 * Copyright © 2022 Friedrich Vock
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

#include "bvh/bvh.h"
#include "util/half_float.h"
#include "amd_family.h"
#include "radv_private.h"
#include "vk_acceleration_structure.h"
#include "vk_common_entrypoints.h"

#define RRA_MAGIC 0x204644525F444D41

struct rra_file_header {
   uint64_t magic;
   uint32_t version;
   uint32_t unused;
   uint64_t chunk_descriptions_offset;
   uint64_t chunk_descriptions_size;
};

static_assert(sizeof(struct rra_file_header) == 32, "rra_file_header does not match RRA spec");

enum rra_chunk_type {
   RADV_RRA_CHUNK_ID_ASIC_API_INFO = 0x1,
   RADV_RRA_CHUNK_ID_ACCEL_STRUCT = 0xF0005,
};

enum rra_file_api {
   RADV_RRA_API_DX9,
   RADV_RRA_API_DX11,
   RADV_RRA_API_DX12,
   RADV_RRA_API_VULKAN,
   RADV_RRA_API_OPENGL,
   RADV_RRA_API_OPENCL,
   RADV_RRA_API_MANTLE,
   RADV_RRA_API_GENERIC,
};

struct rra_file_chunk_description {
   char name[16];
   uint32_t is_zstd_compressed;
   enum rra_chunk_type type;
   uint64_t header_offset;
   uint64_t header_size;
   uint64_t data_offset;
   uint64_t data_size;
   uint64_t unused;
};

static_assert(sizeof(struct rra_file_chunk_description) == 64, "rra_file_chunk_description does not match RRA spec");

static uint64_t
node_to_addr(uint64_t node)
{
   node &= ~7ull;
   node <<= 19;
   return ((int64_t)node) >> 16;
}

static void
rra_dump_header(FILE *output, uint64_t chunk_descriptions_offset, uint64_t chunk_descriptions_size)
{
   struct rra_file_header header = {
      .magic = RRA_MAGIC,
      .version = 3,
      .chunk_descriptions_offset = chunk_descriptions_offset,
      .chunk_descriptions_size = chunk_descriptions_size,
   };
   fwrite(&header, sizeof(header), 1, output);
}

static void
rra_dump_chunk_description(uint64_t offset, uint64_t header_size, uint64_t data_size, const char *name,
                           enum rra_chunk_type type, FILE *output)
{
   struct rra_file_chunk_description chunk = {
      .type = type,
      .header_offset = offset,
      .header_size = header_size,
      .data_offset = offset + header_size,
      .data_size = data_size,
   };
   strncpy(chunk.name, name, sizeof(chunk.name) - 1);
   fwrite(&chunk, sizeof(struct rra_file_chunk_description), 1, output);
}

enum rra_memory_type {
   RRA_MEMORY_TYPE_UNKNOWN,
   RRA_MEMORY_TYPE_DDR,
   RRA_MEMORY_TYPE_DDR2,
   RRA_MEMORY_TYPE_DDR3,
   RRA_MEMORY_TYPE_DDR4,
   RRA_MEMORY_TYPE_DDR5,
   RRA_MEMORY_TYPE_GDDR3,
   RRA_MEMORY_TYPE_GDDR4,
   RRA_MEMORY_TYPE_GDDR5,
   RRA_MEMORY_TYPE_GDDR6,
   RRA_MEMORY_TYPE_HBM,
   RRA_MEMORY_TYPE_HBM2,
   RRA_MEMORY_TYPE_HBM3,
   RRA_MEMORY_TYPE_LPDDR4,
   RRA_MEMORY_TYPE_LPDDR5,
};

#define RRA_FILE_DEVICE_NAME_MAX_SIZE 256

struct rra_asic_info {
   uint64_t min_shader_clk_freq;
   uint64_t min_mem_clk_freq;
   char unused[8];
   uint64_t max_shader_clk_freq;
   uint64_t max_mem_clk_freq;
   uint32_t device_id;
   uint32_t rev_id;
   char unused2[80];
   uint64_t vram_size;
   uint32_t bus_width;
   char unused3[12];
   char device_name[RRA_FILE_DEVICE_NAME_MAX_SIZE];
   char unused4[16];
   uint32_t mem_ops_per_clk;
   uint32_t mem_type;
   char unused5[135];
   bool valid;
};

static_assert(sizeof(struct rra_asic_info) == 568, "rra_asic_info does not match RRA spec");

static uint32_t
amdgpu_vram_type_to_rra(uint32_t type)
{
   switch (type) {
   case AMD_VRAM_TYPE_UNKNOWN:
      return RRA_MEMORY_TYPE_UNKNOWN;
   case AMD_VRAM_TYPE_DDR2:
      return RRA_MEMORY_TYPE_DDR2;
   case AMD_VRAM_TYPE_DDR3:
      return RRA_MEMORY_TYPE_DDR3;
   case AMD_VRAM_TYPE_DDR4:
      return RRA_MEMORY_TYPE_DDR4;
   case AMD_VRAM_TYPE_DDR5:
      return RRA_MEMORY_TYPE_DDR5;
   case AMD_VRAM_TYPE_HBM:
      return RRA_MEMORY_TYPE_HBM;
   case AMD_VRAM_TYPE_GDDR3:
      return RRA_MEMORY_TYPE_GDDR3;
   case AMD_VRAM_TYPE_GDDR4:
      return RRA_MEMORY_TYPE_GDDR4;
   case AMD_VRAM_TYPE_GDDR5:
      return RRA_MEMORY_TYPE_GDDR5;
   case AMD_VRAM_TYPE_GDDR6:
      return RRA_MEMORY_TYPE_GDDR6;
   case AMD_VRAM_TYPE_LPDDR4:
      return RRA_MEMORY_TYPE_LPDDR4;
   case AMD_VRAM_TYPE_LPDDR5:
      return RRA_MEMORY_TYPE_LPDDR5;
   default:
      unreachable("invalid vram type");
   }
}

static void
rra_dump_asic_info(const struct radeon_info *rad_info, FILE *output)
{
   struct rra_asic_info asic_info = {
      /* All frequencies are in Hz */
      .min_shader_clk_freq = 0,
      .max_shader_clk_freq = rad_info->max_gpu_freq_mhz * 1000000,
      .min_mem_clk_freq = 0,
      .max_mem_clk_freq = rad_info->memory_freq_mhz * 1000000,

      .vram_size = (uint64_t)rad_info->vram_size_kb * 1024,

      .mem_type = amdgpu_vram_type_to_rra(rad_info->vram_type),
      .mem_ops_per_clk = ac_memory_ops_per_clock(rad_info->vram_type),
      .bus_width = rad_info->memory_bus_width,

      .device_id = rad_info->pci.dev,
      .rev_id = rad_info->pci_rev_id,
   };

   strncpy(asic_info.device_name, rad_info->marketing_name ? rad_info->marketing_name : rad_info->name,
           RRA_FILE_DEVICE_NAME_MAX_SIZE - 1);

   fwrite(&asic_info, sizeof(struct rra_asic_info), 1, output);
}

enum rra_bvh_type {
   RRA_BVH_TYPE_TLAS,
   RRA_BVH_TYPE_BLAS,
};

struct rra_accel_struct_chunk_header {
   /*
    * Declaring this as uint64_t would make the compiler insert padding to
    * satisfy alignment restrictions.
    */
   uint32_t virtual_address[2];
   uint32_t metadata_offset;
   uint32_t metadata_size;
   uint32_t header_offset;
   uint32_t header_size;
   enum rra_bvh_type bvh_type;
};

static_assert(sizeof(struct rra_accel_struct_chunk_header) == 28,
              "rra_accel_struct_chunk_header does not match RRA spec");

struct rra_accel_struct_post_build_info {
   uint32_t bvh_type : 1;
   uint32_t reserved1 : 5;
   uint32_t tri_compression_mode : 2;
   uint32_t fp16_interior_mode : 2;
   uint32_t reserved2 : 6;
   uint32_t build_flags : 16;
};

static_assert(sizeof(struct rra_accel_struct_post_build_info) == 4,
              "rra_accel_struct_post_build_info does not match RRA spec");

struct rra_accel_struct_header {
   struct rra_accel_struct_post_build_info post_build_info;
   /*
    * Size of the internal acceleration structure metadata in the
    * proprietary drivers. Seems to always be 128.
    */
   uint32_t metadata_size;
   uint32_t file_size;
   uint32_t primitive_count;
   uint32_t active_primitive_count;
   uint32_t unused1;
   uint32_t geometry_description_count;
   VkGeometryTypeKHR geometry_type;
   uint32_t internal_nodes_offset;
   uint32_t leaf_nodes_offset;
   uint32_t geometry_infos_offset;
   uint32_t leaf_ids_offset;
   uint32_t interior_fp32_node_count;
   uint32_t interior_fp16_node_count;
   uint32_t leaf_node_count;
   uint32_t rt_driver_interface_version;
   uint64_t unused2;
   uint32_t half_fp32_node_count;
   char unused3[44];
};

#define RRA_ROOT_NODE_OFFSET align(sizeof(struct rra_accel_struct_header), 64)

static_assert(sizeof(struct rra_accel_struct_header) == 120, "rra_accel_struct_header does not match RRA spec");

struct rra_accel_struct_metadata {
   uint64_t virtual_address;
   uint32_t byte_size;
   char unused[116];
};

static_assert(sizeof(struct rra_accel_struct_metadata) == 128, "rra_accel_struct_metadata does not match RRA spec");

struct rra_geometry_info {
   uint32_t primitive_count : 29;
   uint32_t flags : 3;
   uint32_t unknown;
   uint32_t leaf_node_list_offset;
};

static_assert(sizeof(struct rra_geometry_info) == 12, "rra_geometry_info does not match RRA spec");

static struct rra_accel_struct_header
rra_fill_accel_struct_header_common(struct radv_accel_struct_header *header, size_t parent_id_table_size,
                                    size_t leaf_node_data_size, size_t internal_node_data_size,
                                    uint64_t primitive_count)
{
   struct rra_accel_struct_header result = {
      .post_build_info =
         {
            .build_flags = header->build_flags,
            /* Seems to be no compression */
            .tri_compression_mode = 0,
         },
      .primitive_count = primitive_count,
      /* TODO: calculate active primitives */
      .active_primitive_count = primitive_count,
      .geometry_description_count = header->geometry_count,
      .interior_fp32_node_count = internal_node_data_size / sizeof(struct radv_bvh_box32_node),
      .leaf_node_count = primitive_count,
   };

   result.metadata_size = sizeof(struct rra_accel_struct_metadata) + parent_id_table_size;
   result.file_size =
      result.metadata_size + sizeof(struct rra_accel_struct_header) + internal_node_data_size + leaf_node_data_size;

   result.internal_nodes_offset = sizeof(struct rra_accel_struct_metadata);
   result.leaf_nodes_offset = result.internal_nodes_offset + internal_node_data_size;
   result.geometry_infos_offset = result.leaf_nodes_offset + leaf_node_data_size;
   result.leaf_ids_offset = result.geometry_infos_offset;
   if (!header->instance_count)
      result.leaf_ids_offset += header->geometry_count * sizeof(struct rra_geometry_info);

   return result;
}

struct rra_box32_node {
   uint32_t children[4];
   float coords[4][2][3];
   uint32_t reserved[4];
};

struct rra_box16_node {
   uint32_t children[4];
   float16_t coords[4][2][3];
};

/*
 * RRA files contain this struct in place of hardware
 * instance nodes. They're named "instance desc" internally.
 */
struct rra_instance_node {
   float wto_matrix[12];
   uint32_t custom_instance_id : 24;
   uint32_t mask : 8;
   uint32_t sbt_offset : 24;
   uint32_t instance_flags : 8;
   uint64_t blas_va : 54;
   uint64_t hw_instance_flags : 10;
   uint32_t instance_id;
   uint32_t unused1;
   uint32_t blas_metadata_size;
   uint32_t unused2;
   float otw_matrix[12];
};

static_assert(sizeof(struct rra_instance_node) == 128, "rra_instance_node does not match RRA spec!");

/*
 * Format RRA uses for aabb nodes
 */
struct rra_aabb_node {
   float aabb[2][3];
   uint32_t unused1[6];
   uint32_t geometry_id : 28;
   uint32_t flags : 4;
   uint32_t primitive_id;
   uint32_t unused[2];
};

static_assert(sizeof(struct rra_aabb_node) == 64, "rra_aabb_node does not match RRA spec!");

struct rra_triangle_node {
   float coords[3][3];
   uint32_t reserved[3];
   uint32_t geometry_id : 28;
   uint32_t flags : 4;
   uint32_t triangle_id;
   uint32_t reserved2;
   uint32_t id;
};

static_assert(sizeof(struct rra_triangle_node) == 64, "rra_triangle_node does not match RRA spec!");

static void
rra_dump_tlas_header(struct radv_accel_struct_header *header, size_t parent_id_table_size, size_t leaf_node_data_size,
                     size_t internal_node_data_size, uint64_t primitive_count, FILE *output)
{
   struct rra_accel_struct_header file_header = rra_fill_accel_struct_header_common(
      header, parent_id_table_size, leaf_node_data_size, internal_node_data_size, primitive_count);
   file_header.post_build_info.bvh_type = RRA_BVH_TYPE_TLAS;
   file_header.geometry_type = VK_GEOMETRY_TYPE_INSTANCES_KHR;

   fwrite(&file_header, sizeof(struct rra_accel_struct_header), 1, output);
}

static void
rra_dump_blas_header(struct radv_accel_struct_header *header, size_t parent_id_table_size,
                     struct radv_accel_struct_geometry_info *geometry_infos, size_t leaf_node_data_size,
                     size_t internal_node_data_size, uint64_t primitive_count, FILE *output)
{
   struct rra_accel_struct_header file_header = rra_fill_accel_struct_header_common(
      header, parent_id_table_size, leaf_node_data_size, internal_node_data_size, primitive_count);
   file_header.post_build_info.bvh_type = RRA_BVH_TYPE_BLAS;
   file_header.geometry_type = header->geometry_count ? geometry_infos->type : VK_GEOMETRY_TYPE_TRIANGLES_KHR;

   fwrite(&file_header, sizeof(struct rra_accel_struct_header), 1, output);
}

static uint32_t
rra_parent_table_index_from_offset(uint32_t offset, uint32_t parent_table_size)
{
   uint32_t max_parent_table_index = parent_table_size / sizeof(uint32_t) - 1;
   return max_parent_table_index - (offset - RRA_ROOT_NODE_OFFSET) / 64;
}

struct rra_validation_context {
   bool failed;
   char location[31];
};

static void PRINTFLIKE(2, 3) rra_validation_fail(struct rra_validation_context *ctx, const char *message, ...)
{
   if (!ctx->failed) {
      fprintf(stderr, "radv: rra: Validation failed at %s:\n", ctx->location);
      ctx->failed = true;
   }

   fprintf(stderr, "   ");

   va_list list;
   va_start(list, message);
   vfprintf(stderr, message, list);
   va_end(list);

   fprintf(stderr, "\n");
}

static bool
rra_validate_header(struct radv_rra_accel_struct_data *accel_struct, const struct radv_accel_struct_header *header)
{
   struct rra_validation_context ctx = {
      .location = "header",
   };

   if (accel_struct->type == VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR && header->instance_count > 0)
      rra_validation_fail(&ctx, "BLAS contains instances");

   if (header->bvh_offset >= accel_struct->size)
      rra_validation_fail(&ctx, "Invalid BVH offset %u", header->bvh_offset);

   if (header->instance_count * sizeof(struct radv_bvh_instance_node) >= accel_struct->size)
      rra_validation_fail(&ctx, "Too many instances");

   return ctx.failed;
}

static bool
is_internal_node(uint32_t type)
{
   return type == radv_bvh_node_box16 || type == radv_bvh_node_box32;
}

static const char *node_type_names[8] = {
   [radv_bvh_node_triangle + 0] = "triangle0",
   [radv_bvh_node_triangle + 1] = "triangle1",
   [radv_bvh_node_triangle + 2] = "triangle2",
   [radv_bvh_node_triangle + 3] = "triangle3",
   [radv_bvh_node_box16] = "box16",
   [radv_bvh_node_box32] = "box32",
   [radv_bvh_node_instance] = "instance",
   [radv_bvh_node_aabb] = "aabb",
};

static bool
rra_validate_node(struct hash_table_u64 *accel_struct_vas, uint8_t *data, void *node, uint32_t geometry_count,
                  uint32_t size, bool is_bottom_level)
{
   struct rra_validation_context ctx = {0};

   uint32_t cur_offset = (uint8_t *)node - data;
   snprintf(ctx.location, sizeof(ctx.location), "internal node (offset=%u)", cur_offset);

   /* The child ids are located at offset=0 for both box16 and box32 nodes. */
   uint32_t *children = node;
   for (uint32_t i = 0; i < 4; ++i) {
      if (children[i] == 0xFFFFFFFF)
         continue;

      uint32_t type = children[i] & 7;
      uint32_t offset = (children[i] & (~7u)) << 3;

      if (!is_internal_node(type) && is_bottom_level == (type == radv_bvh_node_instance))
         rra_validation_fail(&ctx,
                             is_bottom_level ? "%s node in BLAS (child index %u)" : "%s node in TLAS (child index %u)",
                             node_type_names[type], i);

      if (offset > size) {
         rra_validation_fail(&ctx, "Invalid child offset (child index %u)", i);
         continue;
      }

      struct rra_validation_context child_ctx = {0};
      snprintf(child_ctx.location, sizeof(child_ctx.location), "%s node (offset=%u)", node_type_names[type], offset);

      if (is_internal_node(type)) {
         ctx.failed |= rra_validate_node(accel_struct_vas, data, data + offset, geometry_count, size, is_bottom_level);
      } else if (type == radv_bvh_node_instance) {
         struct radv_bvh_instance_node *src = (struct radv_bvh_instance_node *)(data + offset);
         uint64_t blas_va = node_to_addr(src->bvh_ptr) - src->bvh_offset;
         if (!_mesa_hash_table_u64_search(accel_struct_vas, blas_va))
            rra_validation_fail(&child_ctx, "Invalid instance node pointer 0x%llx (offset: 0x%x)",
                                (unsigned long long)src->bvh_ptr, src->bvh_offset);
      } else if (type == radv_bvh_node_aabb) {
         struct radv_bvh_aabb_node *src = (struct radv_bvh_aabb_node *)(data + offset);
         if ((src->geometry_id_and_flags & 0xFFFFFFF) >= geometry_count)
            rra_validation_fail(&ctx, "geometry_id >= geometry_count");
      } else {
         struct radv_bvh_triangle_node *src = (struct radv_bvh_triangle_node *)(data + offset);
         if ((src->geometry_id_and_flags & 0xFFFFFFF) >= geometry_count)
            rra_validation_fail(&ctx, "geometry_id >= geometry_count");
      }

      ctx.failed |= child_ctx.failed;
   }
   return ctx.failed;
}

struct rra_transcoding_context {
   const uint8_t *src;
   uint8_t *dst;
   uint32_t dst_leaf_offset;
   uint32_t dst_internal_offset;
   uint32_t *parent_id_table;
   uint32_t parent_id_table_size;
   uint32_t *leaf_node_ids;
   uint32_t *leaf_indices;
};

static void
rra_transcode_triangle_node(struct rra_transcoding_context *ctx, const struct radv_bvh_triangle_node *src)
{
   struct rra_triangle_node *dst = (struct rra_triangle_node *)(ctx->dst + ctx->dst_leaf_offset);
   ctx->dst_leaf_offset += sizeof(struct rra_triangle_node);

   for (int i = 0; i < 3; ++i)
      for (int j = 0; j < 3; ++j)
         dst->coords[i][j] = src->coords[i][j];
   dst->triangle_id = src->triangle_id;
   dst->geometry_id = src->geometry_id_and_flags & 0xfffffff;
   dst->flags = src->geometry_id_and_flags >> 28;
   dst->id = src->id;
}

static void
rra_transcode_aabb_node(struct rra_transcoding_context *ctx, const struct radv_bvh_aabb_node *src, radv_aabb bounds)
{
   struct rra_aabb_node *dst = (struct rra_aabb_node *)(ctx->dst + ctx->dst_leaf_offset);
   ctx->dst_leaf_offset += sizeof(struct rra_aabb_node);

   dst->aabb[0][0] = bounds.min.x;
   dst->aabb[0][1] = bounds.min.y;
   dst->aabb[0][2] = bounds.min.z;
   dst->aabb[1][0] = bounds.max.x;
   dst->aabb[1][1] = bounds.max.y;
   dst->aabb[1][2] = bounds.max.z;

   dst->geometry_id = src->geometry_id_and_flags & 0xfffffff;
   dst->flags = src->geometry_id_and_flags >> 28;
   dst->primitive_id = src->primitive_id;
}

static void
rra_transcode_instance_node(struct rra_transcoding_context *ctx, const struct radv_bvh_instance_node *src)
{
   uint64_t blas_va = node_to_addr(src->bvh_ptr) - src->bvh_offset;

   struct rra_instance_node *dst = (struct rra_instance_node *)(ctx->dst + ctx->dst_leaf_offset);
   ctx->dst_leaf_offset += sizeof(struct rra_instance_node);

   dst->custom_instance_id = src->custom_instance_and_mask & 0xffffff;
   dst->mask = src->custom_instance_and_mask >> 24;
   dst->sbt_offset = src->sbt_offset_and_flags & 0xffffff;
   dst->instance_flags = src->sbt_offset_and_flags >> 24;
   dst->blas_va = (blas_va + sizeof(struct rra_accel_struct_metadata)) >> 3;
   dst->instance_id = src->instance_id;
   dst->blas_metadata_size = sizeof(struct rra_accel_struct_metadata);

   memcpy(dst->wto_matrix, src->wto_matrix.values, sizeof(dst->wto_matrix));
   memcpy(dst->otw_matrix, src->otw_matrix.values, sizeof(dst->otw_matrix));
}

static uint32_t rra_transcode_node(struct rra_transcoding_context *ctx, uint32_t parent_id, uint32_t src_id,
                                   radv_aabb bounds);

static void
rra_transcode_box16_node(struct rra_transcoding_context *ctx, const struct radv_bvh_box16_node *src)
{
   uint32_t dst_offset = ctx->dst_internal_offset;
   ctx->dst_internal_offset += sizeof(struct rra_box16_node);
   struct rra_box16_node *dst = (struct rra_box16_node *)(ctx->dst + dst_offset);

   memcpy(dst->coords, src->coords, sizeof(dst->coords));

   for (uint32_t i = 0; i < 4; ++i) {
      if (src->children[i] == 0xffffffff) {
         dst->children[i] = 0xffffffff;
         continue;
      }

      radv_aabb bounds = {
         .min =
            {
               _mesa_half_to_float(src->coords[i][0][0]),
               _mesa_half_to_float(src->coords[i][0][1]),
               _mesa_half_to_float(src->coords[i][0][2]),
            },
         .max =
            {
               _mesa_half_to_float(src->coords[i][1][0]),
               _mesa_half_to_float(src->coords[i][1][1]),
               _mesa_half_to_float(src->coords[i][1][2]),
            },
      };

      dst->children[i] = rra_transcode_node(ctx, radv_bvh_node_box16 | (dst_offset >> 3), src->children[i], bounds);
   }
}

static void
rra_transcode_box32_node(struct rra_transcoding_context *ctx, const struct radv_bvh_box32_node *src)
{
   uint32_t dst_offset = ctx->dst_internal_offset;
   ctx->dst_internal_offset += sizeof(struct rra_box32_node);
   struct rra_box32_node *dst = (struct rra_box32_node *)(ctx->dst + dst_offset);

   memcpy(dst->coords, src->coords, sizeof(dst->coords));

   for (uint32_t i = 0; i < 4; ++i) {
      if (isnan(src->coords[i].min.x)) {
         dst->children[i] = 0xffffffff;
         continue;
      }

      dst->children[i] =
         rra_transcode_node(ctx, radv_bvh_node_box32 | (dst_offset >> 3), src->children[i], src->coords[i]);
   }
}

static uint32_t
get_geometry_id(const void *node, uint32_t node_type)
{
   if (node_type == radv_bvh_node_triangle) {
      const struct radv_bvh_triangle_node *triangle = node;
      return triangle->geometry_id_and_flags & 0xFFFFFFF;
   }

   if (node_type == radv_bvh_node_aabb) {
      const struct radv_bvh_aabb_node *aabb = node;
      return aabb->geometry_id_and_flags & 0xFFFFFFF;
   }

   return 0;
}

static uint32_t
rra_transcode_node(struct rra_transcoding_context *ctx, uint32_t parent_id, uint32_t src_id, radv_aabb bounds)
{
   uint32_t node_type = src_id & 7;
   uint32_t src_offset = (src_id & (~7u)) << 3;

   uint32_t dst_offset;

   const void *src_child_node = ctx->src + src_offset;
   if (is_internal_node(node_type)) {
      dst_offset = ctx->dst_internal_offset;
      if (node_type == radv_bvh_node_box32)
         rra_transcode_box32_node(ctx, src_child_node);
      else
         rra_transcode_box16_node(ctx, src_child_node);
   } else {
      dst_offset = ctx->dst_leaf_offset;

      if (node_type == radv_bvh_node_triangle)
         rra_transcode_triangle_node(ctx, src_child_node);
      else if (node_type == radv_bvh_node_aabb)
         rra_transcode_aabb_node(ctx, src_child_node, bounds);
      else if (node_type == radv_bvh_node_instance)
         rra_transcode_instance_node(ctx, src_child_node);
   }

   uint32_t parent_id_index = rra_parent_table_index_from_offset(dst_offset, ctx->parent_id_table_size);
   ctx->parent_id_table[parent_id_index] = parent_id;

   uint32_t dst_id = node_type | (dst_offset >> 3);
   if (!is_internal_node(node_type))
      ctx->leaf_node_ids[ctx->leaf_indices[get_geometry_id(src_child_node, node_type)]++] = dst_id;

   return dst_id;
}

struct rra_bvh_info {
   uint32_t leaf_nodes_size;
   uint32_t internal_nodes_size;
   struct rra_geometry_info *geometry_infos;
};

static void
rra_gather_bvh_info(const uint8_t *bvh, uint32_t node_id, struct rra_bvh_info *dst)
{
   uint32_t node_type = node_id & 7;

   switch (node_type) {
   case radv_bvh_node_box16:
      dst->internal_nodes_size += sizeof(struct rra_box16_node);
      break;
   case radv_bvh_node_box32:
      dst->internal_nodes_size += sizeof(struct rra_box32_node);
      break;
   case radv_bvh_node_instance:
      dst->leaf_nodes_size += sizeof(struct rra_instance_node);
      break;
   case radv_bvh_node_triangle:
      dst->leaf_nodes_size += sizeof(struct rra_triangle_node);
      break;
   case radv_bvh_node_aabb:
      dst->leaf_nodes_size += sizeof(struct rra_aabb_node);
      break;
   default:
      break;
   }

   const void *node = bvh + ((node_id & (~7u)) << 3);
   if (is_internal_node(node_type)) {
      /* The child ids are located at offset=0 for both box16 and box32 nodes. */
      const uint32_t *children = node;
      for (uint32_t i = 0; i < 4; i++)
         if (children[i] != 0xffffffff)
            rra_gather_bvh_info(bvh, children[i], dst);
   } else {
      dst->geometry_infos[get_geometry_id(node, node_type)].primitive_count++;
   }
}

static VkResult
rra_dump_acceleration_structure(struct radv_rra_accel_struct_data *accel_struct, uint8_t *data,
                                struct hash_table_u64 *accel_struct_vas, bool should_validate, FILE *output)
{
   struct radv_accel_struct_header *header = (struct radv_accel_struct_header *)data;

   bool is_tlas = header->instance_count > 0;

   uint64_t geometry_infos_offset = sizeof(struct radv_accel_struct_header);

   /* convert root node id to offset */
   uint32_t src_root_offset = (RADV_BVH_ROOT_NODE & ~7) << 3;

   if (should_validate) {
      if (rra_validate_header(accel_struct, header)) {
         return VK_ERROR_VALIDATION_FAILED_EXT;
      }
      if (rra_validate_node(accel_struct_vas, data + header->bvh_offset, data + header->bvh_offset + src_root_offset,
                            header->geometry_count, accel_struct->size, !is_tlas)) {
         return VK_ERROR_VALIDATION_FAILED_EXT;
      }
   }

   VkResult result = VK_SUCCESS;

   struct rra_geometry_info *rra_geometry_infos = NULL;
   uint32_t *leaf_indices = NULL;
   uint32_t *node_parent_table = NULL;
   uint32_t *leaf_node_ids = NULL;
   uint8_t *dst_structure_data = NULL;

   rra_geometry_infos = calloc(header->geometry_count, sizeof(struct rra_geometry_info));
   if (!rra_geometry_infos) {
      result = VK_ERROR_OUT_OF_HOST_MEMORY;
      goto exit;
   }

   struct rra_bvh_info bvh_info = {
      .geometry_infos = rra_geometry_infos,
   };
   rra_gather_bvh_info(data + header->bvh_offset, RADV_BVH_ROOT_NODE, &bvh_info);

   leaf_indices = calloc(header->geometry_count, sizeof(struct rra_geometry_info));
   if (!leaf_indices) {
      result = VK_ERROR_OUT_OF_HOST_MEMORY;
      goto exit;
   }

   uint64_t primitive_count = 0;

   struct radv_accel_struct_geometry_info *geometry_infos =
      (struct radv_accel_struct_geometry_info *)(data + geometry_infos_offset);

   for (uint32_t i = 0; i < header->geometry_count; ++i) {
      rra_geometry_infos[i].flags = geometry_infos[i].flags;
      rra_geometry_infos[i].leaf_node_list_offset = primitive_count * sizeof(uint32_t);
      leaf_indices[i] = primitive_count;
      primitive_count += rra_geometry_infos[i].primitive_count;
   }

   uint32_t node_parent_table_size =
      ((bvh_info.leaf_nodes_size + bvh_info.internal_nodes_size) / 64) * sizeof(uint32_t);

   node_parent_table = calloc(node_parent_table_size, 1);
   if (!node_parent_table) {
      result = VK_ERROR_OUT_OF_HOST_MEMORY;
      goto exit;
   }

   leaf_node_ids = calloc(primitive_count, sizeof(uint32_t));
   if (!leaf_node_ids) {
      result = VK_ERROR_OUT_OF_HOST_MEMORY;
      goto exit;
   }
   dst_structure_data = calloc(RRA_ROOT_NODE_OFFSET + bvh_info.internal_nodes_size + bvh_info.leaf_nodes_size, 1);
   if (!dst_structure_data) {
      result = VK_ERROR_OUT_OF_HOST_MEMORY;
      goto exit;
   }

   struct rra_transcoding_context ctx = {
      .src = data + header->bvh_offset,
      .dst = dst_structure_data,
      .dst_leaf_offset = RRA_ROOT_NODE_OFFSET + bvh_info.internal_nodes_size,
      .dst_internal_offset = RRA_ROOT_NODE_OFFSET,
      .parent_id_table = node_parent_table,
      .parent_id_table_size = node_parent_table_size,
      .leaf_node_ids = leaf_node_ids,
      .leaf_indices = leaf_indices,
   };

   rra_transcode_node(&ctx, 0xFFFFFFFF, RADV_BVH_ROOT_NODE, header->aabb);

   struct rra_accel_struct_chunk_header chunk_header = {
      .metadata_offset = 0,
      /*
       * RRA loads the part of the metadata that is used into a struct.
       * If the size is larger than just the "used" part, the loading
       * operation overwrites internal pointers with data from the file,
       * likely causing a crash.
       */
      .metadata_size = offsetof(struct rra_accel_struct_metadata, unused),
      .header_offset = sizeof(struct rra_accel_struct_metadata) + node_parent_table_size,
      .header_size = sizeof(struct rra_accel_struct_header),
      .bvh_type = is_tlas ? RRA_BVH_TYPE_TLAS : RRA_BVH_TYPE_BLAS,
   };

   /*
    * When associating TLASes with BLASes, acceleration structure VAs are
    * looked up in a hashmap. But due to the way BLAS VAs are stored for
    * each instance in the RRA file format (divided by 8, and limited to 54 bits),
    * the top bits are masked away.
    * In order to make sure BLASes can be found in the hashmap, we have
    * to replicate that mask here.
    */
   uint64_t va = accel_struct->va & 0x1FFFFFFFFFFFFFF;
   memcpy(chunk_header.virtual_address, &va, sizeof(uint64_t));

   struct rra_accel_struct_metadata rra_metadata = {
      .virtual_address = va,
      .byte_size = bvh_info.leaf_nodes_size + bvh_info.internal_nodes_size + sizeof(struct rra_accel_struct_header),
   };

   fwrite(&chunk_header, sizeof(struct rra_accel_struct_chunk_header), 1, output);
   fwrite(&rra_metadata, sizeof(struct rra_accel_struct_metadata), 1, output);

   /* Write node parent id data */
   fwrite(node_parent_table, 1, node_parent_table_size, output);

   if (is_tlas)
      rra_dump_tlas_header(header, node_parent_table_size, bvh_info.leaf_nodes_size, bvh_info.internal_nodes_size,
                           primitive_count, output);
   else
      rra_dump_blas_header(header, node_parent_table_size, geometry_infos, bvh_info.leaf_nodes_size,
                           bvh_info.internal_nodes_size, primitive_count, output);

   /* Write acceleration structure data  */
   fwrite(dst_structure_data + RRA_ROOT_NODE_OFFSET, 1, bvh_info.internal_nodes_size + bvh_info.leaf_nodes_size,
          output);

   if (!is_tlas)
      fwrite(rra_geometry_infos, sizeof(struct rra_geometry_info), header->geometry_count, output);

   /* Write leaf node ids */
   uint32_t leaf_node_list_size = primitive_count * sizeof(uint32_t);
   fwrite(leaf_node_ids, 1, leaf_node_list_size, output);

exit:
   free(rra_geometry_infos);
   free(leaf_indices);
   free(dst_structure_data);
   free(node_parent_table);
   free(leaf_node_ids);

   return result;
}

void
radv_rra_trace_init(struct radv_device *device)
{
   device->rra_trace.validate_as = debug_get_bool_option("RADV_RRA_TRACE_VALIDATE", false);
   device->rra_trace.copy_after_build = debug_get_bool_option("RADV_RRA_TRACE_COPY_AFTER_BUILD", false);
   device->rra_trace.accel_structs = _mesa_pointer_hash_table_create(NULL);
   device->rra_trace.accel_struct_vas = _mesa_hash_table_u64_create(NULL);
   simple_mtx_init(&device->rra_trace.data_mtx, mtx_plain);

   device->rra_trace.copy_memory_index = radv_find_memory_index(
      device->physical_device,
      VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT | VK_MEMORY_PROPERTY_HOST_CACHED_BIT);
}

void
radv_rra_trace_finish(VkDevice vk_device, struct radv_rra_trace_data *data)
{
   if (data->accel_structs)
      hash_table_foreach (data->accel_structs, entry)
         radv_destroy_rra_accel_struct_data(vk_device, entry->data);

   simple_mtx_destroy(&data->data_mtx);
   _mesa_hash_table_destroy(data->accel_structs, NULL);
   _mesa_hash_table_u64_destroy(data->accel_struct_vas);
}

void
radv_destroy_rra_accel_struct_data(VkDevice device, struct radv_rra_accel_struct_data *data)
{
   radv_DestroyEvent(device, data->build_event, NULL);
   radv_DestroyBuffer(device, data->buffer, NULL);
   radv_FreeMemory(device, data->memory, NULL);
   free(data);
}

static int
accel_struct_entry_cmp(const void *a, const void *b)
{
   struct hash_entry *entry_a = *(struct hash_entry *const *)a;
   struct hash_entry *entry_b = *(struct hash_entry *const *)b;
   const struct radv_rra_accel_struct_data *s_a = entry_a->data;
   const struct radv_rra_accel_struct_data *s_b = entry_b->data;

   return s_a->va > s_b->va ? 1 : s_a->va < s_b->va ? -1 : 0;
}

struct rra_copy_context {
   VkDevice device;
   VkQueue queue;

   VkCommandPool pool;
   VkCommandBuffer cmd_buffer;
   uint32_t family_index;

   VkDeviceMemory memory;
   VkBuffer buffer;
   void *mapped_data;

   struct hash_entry **entries;
};

static VkResult
rra_copy_context_init(struct rra_copy_context *ctx)
{
   RADV_FROM_HANDLE(radv_device, device, ctx->device);
   if (device->rra_trace.copy_after_build)
      return VK_SUCCESS;

   uint32_t max_size = 0;
   uint32_t accel_struct_count = _mesa_hash_table_num_entries(device->rra_trace.accel_structs);
   for (unsigned i = 0; i < accel_struct_count; i++) {
      struct radv_rra_accel_struct_data *data = ctx->entries[i]->data;
      max_size = MAX2(max_size, data->size);
   }

   VkCommandPoolCreateInfo pool_info = {
      .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
      .queueFamilyIndex = ctx->family_index,
   };

   VkResult result = vk_common_CreateCommandPool(ctx->device, &pool_info, NULL, &ctx->pool);
   if (result != VK_SUCCESS)
      return result;

   VkCommandBufferAllocateInfo cmdbuf_alloc_info = {
      .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
      .commandPool = ctx->pool,
      .commandBufferCount = 1,
   };

   result = vk_common_AllocateCommandBuffers(ctx->device, &cmdbuf_alloc_info, &ctx->cmd_buffer);
   if (result != VK_SUCCESS)
      goto fail_pool;

   VkBufferCreateInfo buffer_create_info = {
      .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
      .pNext =
         &(VkBufferUsageFlags2CreateInfoKHR){
            .sType = VK_STRUCTURE_TYPE_BUFFER_USAGE_FLAGS_2_CREATE_INFO_KHR,
            .usage = VK_BUFFER_USAGE_2_TRANSFER_DST_BIT_KHR,
         },
      .size = max_size,
   };

   result = radv_CreateBuffer(ctx->device, &buffer_create_info, NULL, &ctx->buffer);
   if (result != VK_SUCCESS)
      goto fail_pool;

   VkMemoryRequirements requirements;
   vk_common_GetBufferMemoryRequirements(ctx->device, ctx->buffer, &requirements);

   VkMemoryAllocateInfo alloc_info = {
      .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
      .allocationSize = requirements.size,
      .memoryTypeIndex = device->rra_trace.copy_memory_index,
   };

   result = radv_AllocateMemory(ctx->device, &alloc_info, NULL, &ctx->memory);
   if (result != VK_SUCCESS)
      goto fail_buffer;

   result = vk_common_MapMemory(ctx->device, ctx->memory, 0, VK_WHOLE_SIZE, 0, (void **)&ctx->mapped_data);
   if (result != VK_SUCCESS)
      goto fail_memory;

   result = vk_common_BindBufferMemory(ctx->device, ctx->buffer, ctx->memory, 0);
   if (result != VK_SUCCESS)
      goto fail_memory;

   return result;
fail_memory:
   radv_FreeMemory(ctx->device, ctx->memory, NULL);
fail_buffer:
   radv_DestroyBuffer(ctx->device, ctx->buffer, NULL);
fail_pool:
   vk_common_DestroyCommandPool(ctx->device, ctx->pool, NULL);
   return result;
}

static void
rra_copy_context_finish(struct rra_copy_context *ctx)
{
   RADV_FROM_HANDLE(radv_device, device, ctx->device);
   if (device->rra_trace.copy_after_build)
      return;

   vk_common_DestroyCommandPool(ctx->device, ctx->pool, NULL);
   radv_DestroyBuffer(ctx->device, ctx->buffer, NULL);
   vk_common_UnmapMemory(ctx->device, ctx->memory);
   radv_FreeMemory(ctx->device, ctx->memory, NULL);
}

static void *
rra_map_accel_struct_data(struct rra_copy_context *ctx, uint32_t i)
{
   struct radv_rra_accel_struct_data *data = ctx->entries[i]->data;
   if (radv_GetEventStatus(ctx->device, data->build_event) != VK_EVENT_SET)
      return NULL;

   if (data->memory) {
      void *mapped_data;
      vk_common_MapMemory(ctx->device, data->memory, 0, VK_WHOLE_SIZE, 0, &mapped_data);
      return mapped_data;
   }

   const struct vk_acceleration_structure *accel_struct = ctx->entries[i]->key;
   VkResult result;

   VkCommandBufferBeginInfo begin_info = {
      .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
   };
   result = radv_BeginCommandBuffer(ctx->cmd_buffer, &begin_info);
   if (result != VK_SUCCESS)
      return NULL;

   VkBufferCopy2 copy = {
      .sType = VK_STRUCTURE_TYPE_BUFFER_COPY_2,
      .srcOffset = accel_struct->offset,
      .size = accel_struct->size,
   };

   VkCopyBufferInfo2 copy_info = {
      .sType = VK_STRUCTURE_TYPE_COPY_BUFFER_INFO_2,
      .srcBuffer = accel_struct->buffer,
      .dstBuffer = ctx->buffer,
      .regionCount = 1,
      .pRegions = &copy,
   };

   radv_CmdCopyBuffer2(ctx->cmd_buffer, &copy_info);

   result = radv_EndCommandBuffer(ctx->cmd_buffer);
   if (result != VK_SUCCESS)
      return NULL;

   VkSubmitInfo submit_info = {
      .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
      .commandBufferCount = 1,
      .pCommandBuffers = &ctx->cmd_buffer,
   };

   result = vk_common_QueueSubmit(ctx->queue, 1, &submit_info, VK_NULL_HANDLE);
   if (result != VK_SUCCESS)
      return NULL;

   result = vk_common_QueueWaitIdle(ctx->queue);
   if (result != VK_SUCCESS)
      return NULL;

   return ctx->mapped_data;
}

static void
rra_unmap_accel_struct_data(struct rra_copy_context *ctx, uint32_t i)
{
   struct radv_rra_accel_struct_data *data = ctx->entries[i]->data;

   if (data->memory)
      vk_common_UnmapMemory(ctx->device, data->memory);
}

VkResult
radv_rra_dump_trace(VkQueue vk_queue, char *filename)
{
   RADV_FROM_HANDLE(radv_queue, queue, vk_queue);
   struct radv_device *device = queue->device;
   VkDevice vk_device = radv_device_to_handle(device);

   VkResult result = vk_common_DeviceWaitIdle(vk_device);
   if (result != VK_SUCCESS)
      return result;

   uint32_t accel_struct_count = _mesa_hash_table_num_entries(device->rra_trace.accel_structs);

   uint64_t *accel_struct_offsets = calloc(accel_struct_count, sizeof(uint64_t));
   if (!accel_struct_offsets)
      return VK_ERROR_OUT_OF_HOST_MEMORY;

   uint32_t struct_count = _mesa_hash_table_num_entries(device->rra_trace.accel_structs);
   struct hash_entry **hash_entries = malloc(sizeof(*hash_entries) * struct_count);
   if (!hash_entries) {
      free(accel_struct_offsets);
      return VK_ERROR_OUT_OF_HOST_MEMORY;
   }

   FILE *file = fopen(filename, "w");
   if (!file) {
      free(accel_struct_offsets);
      free(hash_entries);
      return VK_ERROR_OUT_OF_HOST_MEMORY;
   }

   /*
    * The header contents can only be determined after all acceleration
    * structures have been dumped. An empty struct is written instead
    * to keep offsets intact.
    */
   struct rra_file_header header = {0};
   fwrite(&header, sizeof(struct rra_file_header), 1, file);

   uint64_t api_info_offset = (uint64_t)ftell(file);
   uint64_t api = RADV_RRA_API_VULKAN;
   fwrite(&api, sizeof(uint64_t), 1, file);

   uint64_t asic_info_offset = (uint64_t)ftell(file);
   rra_dump_asic_info(&device->physical_device->rad_info, file);

   uint64_t written_accel_struct_count = 0;

   struct hash_entry *last_entry = NULL;
   for (unsigned i = 0; (last_entry = _mesa_hash_table_next_entry(device->rra_trace.accel_structs, last_entry)); ++i)
      hash_entries[i] = last_entry;

   qsort(hash_entries, struct_count, sizeof(*hash_entries), accel_struct_entry_cmp);

   struct rra_copy_context copy_ctx = {
      .device = vk_device,
      .queue = vk_queue,
      .entries = hash_entries,
      .family_index = queue->vk.queue_family_index,
   };

   result = rra_copy_context_init(&copy_ctx);
   if (result != VK_SUCCESS) {
      free(accel_struct_offsets);
      free(hash_entries);
      fclose(file);
      return result;
   }

   for (unsigned i = 0; i < struct_count; i++) {
      struct radv_rra_accel_struct_data *data = hash_entries[i]->data;
      void *mapped_data = rra_map_accel_struct_data(&copy_ctx, i);
      if (!mapped_data)
         continue;

      accel_struct_offsets[written_accel_struct_count] = (uint64_t)ftell(file);
      result = rra_dump_acceleration_structure(data, mapped_data, device->rra_trace.accel_struct_vas,
                                               device->rra_trace.validate_as, file);

      rra_unmap_accel_struct_data(&copy_ctx, i);

      if (result == VK_SUCCESS)
         written_accel_struct_count++;
   }

   rra_copy_context_finish(&copy_ctx);

   uint64_t chunk_info_offset = (uint64_t)ftell(file);
   rra_dump_chunk_description(api_info_offset, 0, 8, "ApiInfo", RADV_RRA_CHUNK_ID_ASIC_API_INFO, file);
   rra_dump_chunk_description(asic_info_offset, 0, sizeof(struct rra_asic_info), "AsicInfo",
                              RADV_RRA_CHUNK_ID_ASIC_API_INFO, file);

   for (uint32_t i = 0; i < written_accel_struct_count; ++i) {
      uint64_t accel_struct_size;
      if (i == written_accel_struct_count - 1)
         accel_struct_size = (uint64_t)(chunk_info_offset - accel_struct_offsets[i]);
      else
         accel_struct_size = (uint64_t)(accel_struct_offsets[i + 1] - accel_struct_offsets[i]);

      rra_dump_chunk_description(accel_struct_offsets[i], sizeof(struct rra_accel_struct_chunk_header),
                                 accel_struct_size, "RawAccelStruct", RADV_RRA_CHUNK_ID_ACCEL_STRUCT, file);
   }

   uint64_t file_end = (uint64_t)ftell(file);

   /* All info is available, dump header now */
   fseek(file, 0, SEEK_SET);
   rra_dump_header(file, chunk_info_offset, file_end - chunk_info_offset);
   fclose(file);
   free(hash_entries);
   free(accel_struct_offsets);
   return VK_SUCCESS;
}
