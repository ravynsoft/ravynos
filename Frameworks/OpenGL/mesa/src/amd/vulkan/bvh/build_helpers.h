/*
 * Copyright © 2022 Konstantin Seurer
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

#ifndef BVH_BUILD_HELPERS_H
#define BVH_BUILD_HELPERS_H

#include "bvh.h"

#define VK_FORMAT_UNDEFINED                  0
#define VK_FORMAT_R4G4_UNORM_PACK8           1
#define VK_FORMAT_R4G4B4A4_UNORM_PACK16      2
#define VK_FORMAT_B4G4R4A4_UNORM_PACK16      3
#define VK_FORMAT_R5G6B5_UNORM_PACK16        4
#define VK_FORMAT_B5G6R5_UNORM_PACK16        5
#define VK_FORMAT_R5G5B5A1_UNORM_PACK16      6
#define VK_FORMAT_B5G5R5A1_UNORM_PACK16      7
#define VK_FORMAT_A1R5G5B5_UNORM_PACK16      8
#define VK_FORMAT_R8_UNORM                   9
#define VK_FORMAT_R8_SNORM                   10
#define VK_FORMAT_R8_USCALED                 11
#define VK_FORMAT_R8_SSCALED                 12
#define VK_FORMAT_R8_UINT                    13
#define VK_FORMAT_R8_SINT                    14
#define VK_FORMAT_R8_SRGB                    15
#define VK_FORMAT_R8G8_UNORM                 16
#define VK_FORMAT_R8G8_SNORM                 17
#define VK_FORMAT_R8G8_USCALED               18
#define VK_FORMAT_R8G8_SSCALED               19
#define VK_FORMAT_R8G8_UINT                  20
#define VK_FORMAT_R8G8_SINT                  21
#define VK_FORMAT_R8G8_SRGB                  22
#define VK_FORMAT_R8G8B8_UNORM               23
#define VK_FORMAT_R8G8B8_SNORM               24
#define VK_FORMAT_R8G8B8_USCALED             25
#define VK_FORMAT_R8G8B8_SSCALED             26
#define VK_FORMAT_R8G8B8_UINT                27
#define VK_FORMAT_R8G8B8_SINT                28
#define VK_FORMAT_R8G8B8_SRGB                29
#define VK_FORMAT_B8G8R8_UNORM               30
#define VK_FORMAT_B8G8R8_SNORM               31
#define VK_FORMAT_B8G8R8_USCALED             32
#define VK_FORMAT_B8G8R8_SSCALED             33
#define VK_FORMAT_B8G8R8_UINT                34
#define VK_FORMAT_B8G8R8_SINT                35
#define VK_FORMAT_B8G8R8_SRGB                36
#define VK_FORMAT_R8G8B8A8_UNORM             37
#define VK_FORMAT_R8G8B8A8_SNORM             38
#define VK_FORMAT_R8G8B8A8_USCALED           39
#define VK_FORMAT_R8G8B8A8_SSCALED           40
#define VK_FORMAT_R8G8B8A8_UINT              41
#define VK_FORMAT_R8G8B8A8_SINT              42
#define VK_FORMAT_R8G8B8A8_SRGB              43
#define VK_FORMAT_B8G8R8A8_UNORM             44
#define VK_FORMAT_B8G8R8A8_SNORM             45
#define VK_FORMAT_B8G8R8A8_USCALED           46
#define VK_FORMAT_B8G8R8A8_SSCALED           47
#define VK_FORMAT_B8G8R8A8_UINT              48
#define VK_FORMAT_B8G8R8A8_SINT              49
#define VK_FORMAT_B8G8R8A8_SRGB              50
#define VK_FORMAT_A8B8G8R8_UNORM_PACK32      51
#define VK_FORMAT_A8B8G8R8_SNORM_PACK32      52
#define VK_FORMAT_A8B8G8R8_USCALED_PACK32    53
#define VK_FORMAT_A8B8G8R8_SSCALED_PACK32    54
#define VK_FORMAT_A8B8G8R8_UINT_PACK32       55
#define VK_FORMAT_A8B8G8R8_SINT_PACK32       56
#define VK_FORMAT_A8B8G8R8_SRGB_PACK32       57
#define VK_FORMAT_A2R10G10B10_UNORM_PACK32   58
#define VK_FORMAT_A2R10G10B10_SNORM_PACK32   59
#define VK_FORMAT_A2R10G10B10_USCALED_PACK32 60
#define VK_FORMAT_A2R10G10B10_SSCALED_PACK32 61
#define VK_FORMAT_A2R10G10B10_UINT_PACK32    62
#define VK_FORMAT_A2R10G10B10_SINT_PACK32    63
#define VK_FORMAT_A2B10G10R10_UNORM_PACK32   64
#define VK_FORMAT_A2B10G10R10_SNORM_PACK32   65
#define VK_FORMAT_A2B10G10R10_USCALED_PACK32 66
#define VK_FORMAT_A2B10G10R10_SSCALED_PACK32 67
#define VK_FORMAT_A2B10G10R10_UINT_PACK32    68
#define VK_FORMAT_A2B10G10R10_SINT_PACK32    69
#define VK_FORMAT_R16_UNORM                  70
#define VK_FORMAT_R16_SNORM                  71
#define VK_FORMAT_R16_USCALED                72
#define VK_FORMAT_R16_SSCALED                73
#define VK_FORMAT_R16_UINT                   74
#define VK_FORMAT_R16_SINT                   75
#define VK_FORMAT_R16_SFLOAT                 76
#define VK_FORMAT_R16G16_UNORM               77
#define VK_FORMAT_R16G16_SNORM               78
#define VK_FORMAT_R16G16_USCALED             79
#define VK_FORMAT_R16G16_SSCALED             80
#define VK_FORMAT_R16G16_UINT                81
#define VK_FORMAT_R16G16_SINT                82
#define VK_FORMAT_R16G16_SFLOAT              83
#define VK_FORMAT_R16G16B16_UNORM            84
#define VK_FORMAT_R16G16B16_SNORM            85
#define VK_FORMAT_R16G16B16_USCALED          86
#define VK_FORMAT_R16G16B16_SSCALED          87
#define VK_FORMAT_R16G16B16_UINT             88
#define VK_FORMAT_R16G16B16_SINT             89
#define VK_FORMAT_R16G16B16_SFLOAT           90
#define VK_FORMAT_R16G16B16A16_UNORM         91
#define VK_FORMAT_R16G16B16A16_SNORM         92
#define VK_FORMAT_R16G16B16A16_USCALED       93
#define VK_FORMAT_R16G16B16A16_SSCALED       94
#define VK_FORMAT_R16G16B16A16_UINT          95
#define VK_FORMAT_R16G16B16A16_SINT          96
#define VK_FORMAT_R16G16B16A16_SFLOAT        97
#define VK_FORMAT_R32_UINT                   98
#define VK_FORMAT_R32_SINT                   99
#define VK_FORMAT_R32_SFLOAT                 100
#define VK_FORMAT_R32G32_UINT                101
#define VK_FORMAT_R32G32_SINT                102
#define VK_FORMAT_R32G32_SFLOAT              103
#define VK_FORMAT_R32G32B32_UINT             104
#define VK_FORMAT_R32G32B32_SINT             105
#define VK_FORMAT_R32G32B32_SFLOAT           106
#define VK_FORMAT_R32G32B32A32_UINT          107
#define VK_FORMAT_R32G32B32A32_SINT          108
#define VK_FORMAT_R32G32B32A32_SFLOAT        109
#define VK_FORMAT_R64_UINT                   110
#define VK_FORMAT_R64_SINT                   111
#define VK_FORMAT_R64_SFLOAT                 112
#define VK_FORMAT_R64G64_UINT                113
#define VK_FORMAT_R64G64_SINT                114
#define VK_FORMAT_R64G64_SFLOAT              115
#define VK_FORMAT_R64G64B64_UINT             116
#define VK_FORMAT_R64G64B64_SINT             117
#define VK_FORMAT_R64G64B64_SFLOAT           118
#define VK_FORMAT_R64G64B64A64_UINT          119
#define VK_FORMAT_R64G64B64A64_SINT          120
#define VK_FORMAT_R64G64B64A64_SFLOAT        121

#define VK_INDEX_TYPE_UINT16    0
#define VK_INDEX_TYPE_UINT32    1
#define VK_INDEX_TYPE_NONE_KHR  1000165000
#define VK_INDEX_TYPE_UINT8_EXT 1000265000

#define VK_GEOMETRY_TYPE_TRIANGLES_KHR 0
#define VK_GEOMETRY_TYPE_AABBS_KHR     1
#define VK_GEOMETRY_TYPE_INSTANCES_KHR 2

#define VK_GEOMETRY_INSTANCE_TRIANGLE_FACING_CULL_DISABLE_BIT_KHR 1
#define VK_GEOMETRY_INSTANCE_TRIANGLE_FLIP_FACING_BIT_KHR         2
#define VK_GEOMETRY_INSTANCE_FORCE_OPAQUE_BIT_KHR                 4
#define VK_GEOMETRY_INSTANCE_FORCE_NO_OPAQUE_BIT_KHR              8

#define TYPE(type, align)                                                                                              \
   layout(buffer_reference, buffer_reference_align = align, scalar) buffer type##_ref                                  \
   {                                                                                                                   \
      type value;                                                                                                      \
   };

#define REF(type)  type##_ref
#define VOID_REF   uint64_t
#define NULL       0
#define DEREF(var) var.value

#define SIZEOF(type) uint32_t(uint64_t(REF(type)(uint64_t(0)) + 1))

#define OFFSET(ptr, offset) (uint64_t(ptr) + offset)

#define INFINITY (1.0 / 0.0)
#define NAN      (0.0 / 0.0)

#define INDEX(type, ptr, index) REF(type)(OFFSET(ptr, (index)*SIZEOF(type)))

TYPE(int8_t, 1);
TYPE(uint8_t, 1);
TYPE(int16_t, 2);
TYPE(uint16_t, 2);
TYPE(int32_t, 4);
TYPE(uint32_t, 4);
TYPE(int64_t, 8);
TYPE(uint64_t, 8);

TYPE(float, 4);

TYPE(vec2, 4);
TYPE(vec3, 4);
TYPE(vec4, 4);

TYPE(uvec4, 16);

TYPE(VOID_REF, 8);

/* copied from u_math.h */
uint32_t
align(uint32_t value, uint32_t alignment)
{
   return (value + alignment - 1) & ~(alignment - 1);
}

int32_t
to_emulated_float(float f)
{
   int32_t bits = floatBitsToInt(f);
   return f < 0 ? -2147483648 - bits : bits;
}

float
from_emulated_float(int32_t bits)
{
   return intBitsToFloat(bits < 0 ? -2147483648 - bits : bits);
}

TYPE(radv_aabb, 4);

struct key_id_pair {
   uint32_t id;
   uint32_t key;
};
TYPE(key_id_pair, 4);

TYPE(radv_accel_struct_serialization_header, 8);
TYPE(radv_accel_struct_header, 8);
TYPE(radv_bvh_triangle_node, 4);
TYPE(radv_bvh_aabb_node, 4);
TYPE(radv_bvh_instance_node, 8);
TYPE(radv_bvh_box16_node, 4);
TYPE(radv_bvh_box32_node, 4);

TYPE(radv_ir_header, 4);
TYPE(radv_ir_node, 4);
TYPE(radv_ir_box_node, 4);

TYPE(radv_global_sync_data, 4);

uint32_t
id_to_offset(uint32_t id)
{
   return (id & (~7u)) << 3;
}

uint32_t
id_to_type(uint32_t id)
{
   return id & 7u;
}

uint32_t
pack_node_id(uint32_t offset, uint32_t type)
{
   return (offset >> 3) | type;
}

uint64_t
node_to_addr(uint64_t node)
{
   node &= ~7ul;
   node <<= 19;
   return int64_t(node) >> 16;
}

uint64_t
addr_to_node(uint64_t addr)
{
   return (addr >> 3) & ((1ul << 45) - 1);
}

uint32_t
ir_id_to_offset(uint32_t id)
{
   return id & (~3u);
}

uint32_t
ir_id_to_type(uint32_t id)
{
   return id & 3u;
}

uint32_t
pack_ir_node_id(uint32_t offset, uint32_t type)
{
   return offset | type;
}

uint32_t
ir_type_to_bvh_type(uint32_t type)
{
   switch (type) {
   case radv_ir_node_triangle:
      return radv_bvh_node_triangle;
   case radv_ir_node_internal:
      return radv_bvh_node_box32;
   case radv_ir_node_instance:
      return radv_bvh_node_instance;
   case radv_ir_node_aabb:
      return radv_bvh_node_aabb;
   }
   /* unreachable in valid nodes */
   return RADV_BVH_INVALID_NODE;
}

float
aabb_surface_area(radv_aabb aabb)
{
   vec3 diagonal = aabb.max - aabb.min;
   return 2 * diagonal.x * diagonal.y + 2 * diagonal.y * diagonal.z + 2 * diagonal.x * diagonal.z;
}

/* Just a wrapper for 3 uints. */
struct triangle_indices {
   uint32_t index[3];
};

triangle_indices
load_indices(VOID_REF indices, uint32_t index_format, uint32_t global_id)
{
   triangle_indices result;

   uint32_t index_base = global_id * 3;

   switch (index_format) {
   case VK_INDEX_TYPE_UINT16: {
      result.index[0] = DEREF(INDEX(uint16_t, indices, index_base + 0));
      result.index[1] = DEREF(INDEX(uint16_t, indices, index_base + 1));
      result.index[2] = DEREF(INDEX(uint16_t, indices, index_base + 2));
      break;
   }
   case VK_INDEX_TYPE_UINT32: {
      result.index[0] = DEREF(INDEX(uint32_t, indices, index_base + 0));
      result.index[1] = DEREF(INDEX(uint32_t, indices, index_base + 1));
      result.index[2] = DEREF(INDEX(uint32_t, indices, index_base + 2));
      break;
   }
   case VK_INDEX_TYPE_NONE_KHR: {
      result.index[0] = index_base + 0;
      result.index[1] = index_base + 1;
      result.index[2] = index_base + 2;
      break;
   }
   case VK_INDEX_TYPE_UINT8_EXT: {
      result.index[0] = DEREF(INDEX(uint8_t, indices, index_base + 0));
      result.index[1] = DEREF(INDEX(uint8_t, indices, index_base + 1));
      result.index[2] = DEREF(INDEX(uint8_t, indices, index_base + 2));
      break;
   }
   }

   return result;
}

/* Just a wrapper for 3 vec4s. */
struct triangle_vertices {
   vec4 vertex[3];
};

TYPE(float16_t, 2);

triangle_vertices
load_vertices(VOID_REF vertices, triangle_indices indices, uint32_t vertex_format, uint32_t stride)
{
   triangle_vertices result;

   for (uint32_t i = 0; i < 3; i++) {
      VOID_REF vertex_ptr = OFFSET(vertices, indices.index[i] * stride);
      vec4 vertex = vec4(0.0, 0.0, 0.0, 1.0);

      switch (vertex_format) {
      case VK_FORMAT_R32G32_SFLOAT:
         vertex.x = DEREF(INDEX(float, vertex_ptr, 0));
         vertex.y = DEREF(INDEX(float, vertex_ptr, 1));
         break;
      case VK_FORMAT_R32G32B32_SFLOAT:
      case VK_FORMAT_R32G32B32A32_SFLOAT:
         vertex.x = DEREF(INDEX(float, vertex_ptr, 0));
         vertex.y = DEREF(INDEX(float, vertex_ptr, 1));
         vertex.z = DEREF(INDEX(float, vertex_ptr, 2));
         break;
      case VK_FORMAT_R16G16_SFLOAT:
         vertex.x = DEREF(INDEX(float16_t, vertex_ptr, 0));
         vertex.y = DEREF(INDEX(float16_t, vertex_ptr, 1));
         break;
      case VK_FORMAT_R16G16B16_SFLOAT:
      case VK_FORMAT_R16G16B16A16_SFLOAT:
         vertex.x = DEREF(INDEX(float16_t, vertex_ptr, 0));
         vertex.y = DEREF(INDEX(float16_t, vertex_ptr, 1));
         vertex.z = DEREF(INDEX(float16_t, vertex_ptr, 2));
         break;
      case VK_FORMAT_R16G16_SNORM:
         vertex.x = max(-1.0, DEREF(INDEX(int16_t, vertex_ptr, 0)) / float(0x7FFF));
         vertex.y = max(-1.0, DEREF(INDEX(int16_t, vertex_ptr, 1)) / float(0x7FFF));
         break;
      case VK_FORMAT_R16G16B16A16_SNORM:
         vertex.x = max(-1.0, DEREF(INDEX(int16_t, vertex_ptr, 0)) / float(0x7FFF));
         vertex.y = max(-1.0, DEREF(INDEX(int16_t, vertex_ptr, 1)) / float(0x7FFF));
         vertex.z = max(-1.0, DEREF(INDEX(int16_t, vertex_ptr, 2)) / float(0x7FFF));
         break;
      case VK_FORMAT_R8G8_SNORM:
         vertex.x = max(-1.0, DEREF(INDEX(int8_t, vertex_ptr, 0)) / float(0x7F));
         vertex.y = max(-1.0, DEREF(INDEX(int8_t, vertex_ptr, 1)) / float(0x7F));
         break;
      case VK_FORMAT_R8G8B8A8_SNORM:
         vertex.x = max(-1.0, DEREF(INDEX(int8_t, vertex_ptr, 0)) / float(0x7F));
         vertex.y = max(-1.0, DEREF(INDEX(int8_t, vertex_ptr, 1)) / float(0x7F));
         vertex.z = max(-1.0, DEREF(INDEX(int8_t, vertex_ptr, 2)) / float(0x7F));
         break;
      case VK_FORMAT_R16G16_UNORM:
         vertex.x = DEREF(INDEX(uint16_t, vertex_ptr, 0)) / float(0xFFFF);
         vertex.y = DEREF(INDEX(uint16_t, vertex_ptr, 1)) / float(0xFFFF);
         break;
      case VK_FORMAT_R16G16B16A16_UNORM:
         vertex.x = DEREF(INDEX(uint16_t, vertex_ptr, 0)) / float(0xFFFF);
         vertex.y = DEREF(INDEX(uint16_t, vertex_ptr, 1)) / float(0xFFFF);
         vertex.z = DEREF(INDEX(uint16_t, vertex_ptr, 2)) / float(0xFFFF);
         break;
      case VK_FORMAT_R8G8_UNORM:
         vertex.x = DEREF(INDEX(uint8_t, vertex_ptr, 0)) / float(0xFF);
         vertex.y = DEREF(INDEX(uint8_t, vertex_ptr, 1)) / float(0xFF);
         break;
      case VK_FORMAT_R8G8B8A8_UNORM:
         vertex.x = DEREF(INDEX(uint8_t, vertex_ptr, 0)) / float(0xFF);
         vertex.y = DEREF(INDEX(uint8_t, vertex_ptr, 1)) / float(0xFF);
         vertex.z = DEREF(INDEX(uint8_t, vertex_ptr, 2)) / float(0xFF);
         break;
      case VK_FORMAT_A2B10G10R10_UNORM_PACK32: {
         uint32_t data = DEREF(REF(uint32_t)(vertex_ptr));
         vertex.x = float(data & 0x3FF) / 0x3FF;
         vertex.y = float((data >> 10) & 0x3FF) / 0x3FF;
         vertex.z = float((data >> 20) & 0x3FF) / 0x3FF;
         break;
      }
      }

      result.vertex[i] = vertex;
   }

   return result;
}

/* A GLSL-adapted copy of VkAccelerationStructureInstanceKHR. */
struct AccelerationStructureInstance {
   mat3x4 transform;
   uint32_t custom_instance_and_mask;
   uint32_t sbt_offset_and_flags;
   uint64_t accelerationStructureReference;
};
TYPE(AccelerationStructureInstance, 8);

bool
build_triangle(inout radv_aabb bounds, VOID_REF dst_ptr, radv_bvh_geometry_data geom_data, uint32_t global_id)
{
   bool is_valid = true;
   triangle_indices indices = load_indices(geom_data.indices, geom_data.index_format, global_id);

   triangle_vertices vertices = load_vertices(geom_data.data, indices, geom_data.vertex_format, geom_data.stride);

   /* An inactive triangle is one for which the first (X) component of any vertex is NaN. If any
    * other vertex component is NaN, and the first is not, the behavior is undefined. If the vertex
    * format does not have a NaN representation, then all triangles are considered active.
    */
   if (isnan(vertices.vertex[0].x) || isnan(vertices.vertex[1].x) || isnan(vertices.vertex[2].x))
#if ALWAYS_ACTIVE
      is_valid = false;
#else
      return false;
#endif

   if (geom_data.transform != NULL) {
      mat4 transform = mat4(1.0);

      for (uint32_t col = 0; col < 4; col++)
         for (uint32_t row = 0; row < 3; row++)
            transform[col][row] = DEREF(INDEX(float, geom_data.transform, col + row * 4));

      for (uint32_t i = 0; i < 3; i++)
         vertices.vertex[i] = transform * vertices.vertex[i];
   }

   REF(radv_bvh_triangle_node) node = REF(radv_bvh_triangle_node)(dst_ptr);

   bounds.min = vec3(INFINITY);
   bounds.max = vec3(-INFINITY);

   for (uint32_t coord = 0; coord < 3; coord++)
      for (uint32_t comp = 0; comp < 3; comp++) {
         DEREF(node).coords[coord][comp] = vertices.vertex[coord][comp];
         bounds.min[comp] = min(bounds.min[comp], vertices.vertex[coord][comp]);
         bounds.max[comp] = max(bounds.max[comp], vertices.vertex[coord][comp]);
      }

   DEREF(node).triangle_id = global_id;
   DEREF(node).geometry_id_and_flags = geom_data.geometry_id;
   DEREF(node).id = 9;

   return is_valid;
}

bool
build_aabb(inout radv_aabb bounds, VOID_REF src_ptr, VOID_REF dst_ptr, uint32_t geometry_id, uint32_t global_id)
{
   bool is_valid = true;
   REF(radv_bvh_aabb_node) node = REF(radv_bvh_aabb_node)(dst_ptr);

   for (uint32_t vec = 0; vec < 2; vec++)
      for (uint32_t comp = 0; comp < 3; comp++) {
         float coord = DEREF(INDEX(float, src_ptr, comp + vec * 3));

         if (vec == 0)
            bounds.min[comp] = coord;
         else
            bounds.max[comp] = coord;
      }

   /* An inactive AABB is one for which the minimum X coordinate is NaN. If any other component is
    * NaN, and the first is not, the behavior is undefined.
    */
   if (isnan(bounds.min.x))
#if ALWAYS_ACTIVE
      is_valid = false;
#else
      return false;
#endif

   DEREF(node).primitive_id = global_id;
   DEREF(node).geometry_id_and_flags = geometry_id;

   return is_valid;
}

radv_aabb
calculate_instance_node_bounds(radv_accel_struct_header header, mat3x4 otw_matrix)
{
   radv_aabb aabb;
   for (uint32_t comp = 0; comp < 3; ++comp) {
      aabb.min[comp] = otw_matrix[comp][3];
      aabb.max[comp] = otw_matrix[comp][3];
      for (uint32_t col = 0; col < 3; ++col) {
         aabb.min[comp] +=
            min(otw_matrix[comp][col] * header.aabb.min[col], otw_matrix[comp][col] * header.aabb.max[col]);
         aabb.max[comp] +=
            max(otw_matrix[comp][col] * header.aabb.min[col], otw_matrix[comp][col] * header.aabb.max[col]);
      }
   }
   return aabb;
}

uint32_t
encode_sbt_offset_and_flags(uint32_t src)
{
   uint32_t flags = src >> 24;
   uint32_t ret = src & 0xffffffu;
   if ((flags & VK_GEOMETRY_INSTANCE_FORCE_OPAQUE_BIT_KHR) != 0)
      ret |= RADV_INSTANCE_FORCE_OPAQUE;
   if ((flags & VK_GEOMETRY_INSTANCE_FORCE_NO_OPAQUE_BIT_KHR) == 0)
      ret |= RADV_INSTANCE_NO_FORCE_NOT_OPAQUE;
   if ((flags & VK_GEOMETRY_INSTANCE_TRIANGLE_FACING_CULL_DISABLE_BIT_KHR) != 0)
      ret |= RADV_INSTANCE_TRIANGLE_FACING_CULL_DISABLE;
   if ((flags & VK_GEOMETRY_INSTANCE_TRIANGLE_FLIP_FACING_BIT_KHR) != 0)
      ret |= RADV_INSTANCE_TRIANGLE_FLIP_FACING;
   return ret;
}

bool
build_instance(inout radv_aabb bounds, VOID_REF src_ptr, VOID_REF dst_ptr, uint32_t global_id)
{
   REF(radv_bvh_instance_node) node = REF(radv_bvh_instance_node)(dst_ptr);

   AccelerationStructureInstance instance = DEREF(REF(AccelerationStructureInstance)(src_ptr));

   /* An inactive instance is one whose acceleration structure handle is VK_NULL_HANDLE. Since the active terminology is
    * only relevant for BVH updates, which we do not implement, we can also skip instances with mask == 0.
    */
   if (instance.accelerationStructureReference == 0 || instance.custom_instance_and_mask < (1u << 24u))
      return false;

   radv_accel_struct_header instance_header =
      DEREF(REF(radv_accel_struct_header)(instance.accelerationStructureReference));

   DEREF(node).bvh_ptr = addr_to_node(instance.accelerationStructureReference + instance_header.bvh_offset);
   DEREF(node).bvh_offset = instance_header.bvh_offset;

   mat4 transform = mat4(instance.transform);
   mat4 inv_transform = transpose(inverse(transpose(transform)));
   DEREF(node).wto_matrix = mat3x4(inv_transform);
   DEREF(node).otw_matrix = mat3x4(transform);

   bounds = calculate_instance_node_bounds(instance_header, mat3x4(transform));

   DEREF(node).custom_instance_and_mask = instance.custom_instance_and_mask;
   DEREF(node).sbt_offset_and_flags = encode_sbt_offset_and_flags(instance.sbt_offset_and_flags);
   DEREF(node).instance_id = global_id;

   return true;
}

/** Compute ceiling of integer quotient of A divided by B.
    From macros.h */
#define DIV_ROUND_UP(A, B) (((A) + (B)-1) / (B))

#ifdef USE_GLOBAL_SYNC

/* There might be more invocations available than tasks to do.
 * In that case, the fetched task index is greater than the
 * counter offset for the next phase. To avoid out-of-bounds
 * accessing, phases will be skipped until the task index is
 * is in-bounds again. */
uint32_t num_tasks_to_skip = 0;
uint32_t phase_index = 0;
bool should_skip = false;
shared uint32_t global_task_index;

shared uint32_t shared_phase_index;

uint32_t
task_count(REF(radv_ir_header) header)
{
   uint32_t phase_index = DEREF(header).sync_data.phase_index;
   return DEREF(header).sync_data.task_counts[phase_index & 1];
}

/* Sets the task count for the next phase. */
void
set_next_task_count(REF(radv_ir_header) header, uint32_t new_count)
{
   uint32_t phase_index = DEREF(header).sync_data.phase_index;
   DEREF(header).sync_data.task_counts[(phase_index + 1) & 1] = new_count;
}

/*
 * This function has two main objectives:
 * Firstly, it partitions pending work among free invocations.
 * Secondly, it guarantees global synchronization between different phases.
 *
 * After every call to fetch_task, a new task index is returned.
 * fetch_task will also set num_tasks_to_skip. Use should_execute_phase
 * to determine if the current phase should be executed or skipped.
 *
 * Since tasks are assigned per-workgroup, there is a possibility of the task index being
 * greater than the total task count.
 */
uint32_t
fetch_task(REF(radv_ir_header) header, bool did_work)
{
   /* Perform a memory + control barrier for all buffer writes for the entire workgroup.
    * This guarantees that once the workgroup leaves the PHASE loop, all invocations have finished
    * and their results are written to memory. */
   controlBarrier(gl_ScopeWorkgroup, gl_ScopeDevice, gl_StorageSemanticsBuffer,
                  gl_SemanticsAcquireRelease | gl_SemanticsMakeAvailable | gl_SemanticsMakeVisible);
   if (gl_LocalInvocationIndex == 0) {
      if (did_work)
         atomicAdd(DEREF(header).sync_data.task_done_counter, 1);
      global_task_index = atomicAdd(DEREF(header).sync_data.task_started_counter, 1);

      do {
         /* Perform a memory barrier to refresh the current phase's end counter, in case
          * another workgroup changed it. */
         memoryBarrier(gl_ScopeDevice, gl_StorageSemanticsBuffer,
                       gl_SemanticsAcquireRelease | gl_SemanticsMakeAvailable | gl_SemanticsMakeVisible);

         /* The first invocation of the first workgroup in a new phase is responsible to initiate the
          * switch to a new phase. It is only possible to switch to a new phase if all tasks of the
          * previous phase have been completed. Switching to a new phase and incrementing the phase
          * end counter in turn notifies all invocations for that phase that it is safe to execute.
          */
         if (global_task_index == DEREF(header).sync_data.current_phase_end_counter &&
             DEREF(header).sync_data.task_done_counter == DEREF(header).sync_data.current_phase_end_counter) {
            if (DEREF(header).sync_data.next_phase_exit_flag != 0) {
               DEREF(header).sync_data.phase_index = TASK_INDEX_INVALID;
               memoryBarrier(gl_ScopeDevice, gl_StorageSemanticsBuffer,
                             gl_SemanticsAcquireRelease | gl_SemanticsMakeAvailable | gl_SemanticsMakeVisible);
            } else {
               atomicAdd(DEREF(header).sync_data.phase_index, 1);
               DEREF(header).sync_data.current_phase_start_counter = DEREF(header).sync_data.current_phase_end_counter;
               /* Ensure the changes to the phase index and start/end counter are visible for other
                * workgroup waiting in the loop. */
               memoryBarrier(gl_ScopeDevice, gl_StorageSemanticsBuffer,
                             gl_SemanticsAcquireRelease | gl_SemanticsMakeAvailable | gl_SemanticsMakeVisible);
               atomicAdd(DEREF(header).sync_data.current_phase_end_counter,
                         DIV_ROUND_UP(task_count(header), gl_WorkGroupSize.x));
            }
            break;
         }

         /* If other invocations have finished all nodes, break out; there is no work to do */
         if (DEREF(header).sync_data.phase_index == TASK_INDEX_INVALID) {
            break;
         }
      } while (global_task_index >= DEREF(header).sync_data.current_phase_end_counter);

      shared_phase_index = DEREF(header).sync_data.phase_index;
   }

   barrier();
   if (DEREF(header).sync_data.phase_index == TASK_INDEX_INVALID)
      return TASK_INDEX_INVALID;

   num_tasks_to_skip = shared_phase_index - phase_index;

   uint32_t local_task_index = global_task_index - DEREF(header).sync_data.current_phase_start_counter;
   return local_task_index * gl_WorkGroupSize.x + gl_LocalInvocationID.x;
}

bool
should_execute_phase()
{
   if (num_tasks_to_skip > 0) {
      /* Skip to next phase. */
      ++phase_index;
      --num_tasks_to_skip;
      return false;
   }
   return true;
}

#define PHASE(header)                                                                                                  \
   for (; task_index != TASK_INDEX_INVALID && should_execute_phase(); task_index = fetch_task(header, true))
#endif

#endif
