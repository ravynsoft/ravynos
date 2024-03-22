/*
 * Copyright © 2020 Intel Corporation
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

#include "anv_private.h"

#include <math.h>

#include "util/u_debug.h"
#include "util/half_float.h"
#include "util/u_atomic.h"

#include "genxml/gen_macros.h"
#include "genxml/genX_pack.h"
#include "genxml/genX_rt_pack.h"

#include "ds/intel_tracepoints.h"

#if GFX_VERx10 == 125
#include "grl/grl_structs.h"

/* Wait for the previous dispatches to finish and flush their data port
 * writes.
 */
#define ANV_GRL_FLUSH_FLAGS (ANV_PIPE_END_OF_PIPE_SYNC_BIT | \
                             ANV_PIPE_DATA_CACHE_FLUSH_BIT | \
                             ANV_PIPE_UNTYPED_DATAPORT_CACHE_FLUSH_BIT)

static const VkAccelerationStructureGeometryKHR *
get_geometry(const VkAccelerationStructureBuildGeometryInfoKHR *pInfo,
             uint32_t index)
{
   return pInfo->pGeometries ? &pInfo->pGeometries[index] :
                               pInfo->ppGeometries[index];
}

static size_t align_transient_size(size_t bytes)
{
   return align_uintptr(bytes, 64);
}

static size_t align_private_size(size_t bytes)
{
   return align_uintptr(bytes, 64);
}

static size_t get_scheduler_size(size_t num_builds)
{
    size_t scheduler_size = sizeof(union SchedulerUnion);
    /* add more memory for qnode creation stage if needed */
    if (num_builds > QNODE_GLOBAL_ROOT_BUFFER_MIN_ENTRIES_NUM) {
        scheduler_size += (num_builds - QNODE_GLOBAL_ROOT_BUFFER_MIN_ENTRIES_NUM) * 2 *
           sizeof(struct QNodeGlobalRootBufferEntry);
    }

    return align_private_size(scheduler_size);
}

static size_t
get_batched_binnedsah_transient_mem_size(size_t num_builds)
{
   if (num_builds == 0)
      return 0;
   return num_builds * (sizeof(struct SAHBuildBuffersInfo) + sizeof(gpuva_t));
}

static size_t
get_batched_binnedsah_private_mem_size(size_t num_builds)
{
   if (num_builds == 0)
      return 0;

   size_t globals_size = align_private_size(num_builds * sizeof(struct SAHBuildGlobals));
   return globals_size + get_scheduler_size(num_builds);
}

static uint32_t
estimate_qbvh6_nodes(const uint32_t N)
{
   const uint32_t W = 6;
   const uint32_t N0 = N / 2 + N % 2; // lowest level with 2 leaves per QBVH6 node
   const uint32_t N1 = N0 / W + (N0 % W ? 1 : 0); // filled level
   const uint32_t N2 = N0 / W + (N1 % W ? 1 : 0); // filled level
   const uint32_t N3 = N0 / W + (N2 % W ? 1 : 0); // filled level
   const uint32_t N4 = N3; // overestimate remaining nodes
   return N0 + N1 + N2 + N3 + N4;
}

/* Estimates the worst case number of QBVH6 nodes for a top-down BVH
 * build that guarantees to produce subtree with N >= K primitives
 * from which a single QBVH6 node is created.
 */
static uint32_t
estimate_qbvh6_nodes_minK(const uint32_t N, uint32_t K)
{
    const uint32_t N0 = N / K + (N % K ? 1 : 0); // lowest level of nodes with K leaves minimally
    return N0 + estimate_qbvh6_nodes(N0);
}

static size_t
estimate_qbvh6_fatleafs(const size_t P)
{
   return P;
}

static size_t
estimate_qbvh6_nodes_worstcase(const size_t P)
{
   const size_t F = estimate_qbvh6_fatleafs(P);

   // worst-case each inner node having 5 fat-leaf children.
   //  number of inner nodes is F/5 and number of fat-leaves is F
   return F + ceil(F/5.0);
}

#define sizeof_PrimRef      32
#define sizeof_HwInstanceLeaf (GENX(RT_BVH_INSTANCE_LEAF_length) * 4)
#define sizeof_InternalNode   (GENX(RT_BVH_INTERNAL_NODE_length) * 4)
#define sizeof_Procedural     (GENX(RT_BVH_PROCEDURAL_LEAF_length) * 4)
#define sizeof_Quad           (GENX(RT_BVH_QUAD_LEAF_length) * 4)

static struct MKSizeEstimate
get_gpu_size_estimate(const VkAccelerationStructureBuildGeometryInfoKHR *pInfo,
                      const VkAccelerationStructureBuildRangeInfoKHR *pBuildRangeInfos,
                      const uint32_t *pMaxPrimitiveCounts)
{
   uint32_t num_triangles = 0, num_aabbs = 0, num_instances = 0;
   for (unsigned g = 0; g < pInfo->geometryCount; g++) {
      const VkAccelerationStructureGeometryKHR *pGeometry =
         get_geometry(pInfo, g);
      uint32_t prim_count = pBuildRangeInfos != NULL ?
         pBuildRangeInfos[g].primitiveCount : pMaxPrimitiveCounts[g];

      switch (pGeometry->geometryType) {
      case VK_GEOMETRY_TYPE_TRIANGLES_KHR:
         num_triangles += prim_count;
         break;
      case VK_GEOMETRY_TYPE_AABBS_KHR:
         num_aabbs += prim_count;
         break;
      case VK_GEOMETRY_TYPE_INSTANCES_KHR:
         num_instances += prim_count;
         break;
      default:
         unreachable("Unsupported geometry type");
      }
   }
   const uint32_t num_primitives = num_triangles + num_aabbs + num_instances;

   struct MKSizeEstimate est = {};

   uint64_t size = sizeof(BVHBase);
   size = align64(size, 64);

   /* Must immediately follow BVHBase because we use fixed offset to nodes. */
   est.node_data_start = size;

   switch (pInfo->type) {
   case VK_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL_KHR: {
      assert(num_triangles == 0 && num_aabbs == 0);

      est.numPrimitives = num_instances;
      est.numPrimitivesToSplit = 0;
      est.numBuildPrimitives = est.numPrimitives + est.numPrimitivesToSplit;

      est.min_primitives = est.numPrimitives;
      est.max_primitives = est.numPrimitives + est.numPrimitivesToSplit;

      unsigned int sizeInnerNodes =
         (unsigned int) estimate_qbvh6_nodes_worstcase(est.numBuildPrimitives) *
         sizeof_InternalNode;
      if (sizeInnerNodes == 0)
         sizeInnerNodes = sizeof_InternalNode;

      est.max_inner_nodes = sizeInnerNodes / sizeof_InternalNode;

      size += sizeInnerNodes;
      STATIC_ASSERT(sizeof_InternalNode % 64 == 0);

      est.leaf_data_start = size;
      size += est.numBuildPrimitives * sizeof_HwInstanceLeaf;
      STATIC_ASSERT(sizeof_HwInstanceLeaf % 64 == 0);

      est.leaf_data_size = est.numBuildPrimitives * sizeof_HwInstanceLeaf;

      break;
   }

   case VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR: {
      assert(num_instances == 0);

      /* RT: TODO */
      const float split_factor = 0.0f;
      uint32_t num_prims_to_split = 0;
      if (false)
         num_prims_to_split = num_triangles + (double)split_factor;

      const uint32_t num_build_triangles = num_triangles + num_prims_to_split;
      const uint32_t num_build_primitives = num_build_triangles + num_aabbs;

      est.numPrimitives = num_primitives;
      est.numTriangles = num_triangles;
      est.numProcedurals = num_aabbs;
      est.numMeshes = pInfo->geometryCount;
      est.numBuildPrimitives = num_build_primitives;
      est.numPrimitivesToSplit = num_prims_to_split;
      est.max_instance_leafs = 0;

      est.min_primitives = (size_t)(num_build_triangles * 0.5f + num_aabbs);
      est.max_primitives = num_build_triangles + num_aabbs;

      size_t nodeBytes = 0;
      nodeBytes += estimate_qbvh6_nodes_worstcase(num_build_triangles) * sizeof_InternalNode;
      nodeBytes += estimate_qbvh6_nodes_worstcase(num_aabbs) * sizeof_InternalNode;
      if (nodeBytes == 0) // for case with 0 primitives
         nodeBytes = sizeof_InternalNode;
      nodeBytes = MAX2(nodeBytes, 8 * (size_t)num_build_primitives); // for primref_index0/1 buffers

      est.max_inner_nodes = nodeBytes / sizeof_InternalNode;

      size += nodeBytes;
      STATIC_ASSERT(sizeof_InternalNode % 64 == 0);

      est.leaf_data_start = size;
      size += num_build_triangles * sizeof_Quad;
      STATIC_ASSERT(sizeof_Quad % 64 == 0);

      est.procedural_data_start = size;
      size += num_aabbs * sizeof_Procedural;
      STATIC_ASSERT(sizeof_Procedural % 64 == 0);

      est.leaf_data_size = num_build_triangles * sizeof_Quad +
                           num_aabbs * sizeof_Procedural;

      if (num_build_primitives == 0)
         size += MAX2(sizeof_Quad, sizeof_Procedural);
      break;
   }

   default:
      unreachable("Unsupported acceleration structure type");
   }

   size = align64(size, 64);
   est.instance_descs_start = size;
   size += sizeof(struct InstanceDesc) * num_instances;

   est.geo_meta_data_start = size;
   size += sizeof(struct GeoMetaData) * pInfo->geometryCount;
   size = align64(size, 64);

   assert(size == align64(size, 64));
   est.back_pointer_start = size;

   const bool alloc_backpointers = false; /* RT TODO */
   if (alloc_backpointers) {
      size += est.max_inner_nodes * sizeof(uint32_t);
      size = align64(size, 64);
   }

   assert(size < UINT32_MAX);
   est.sizeTotal = align64(size, 64);

   return est;
}

struct scratch_layout {
   gpuva_t base;
   uint32_t total_size;

   gpuva_t primrefs;
   gpuva_t globals;
   gpuva_t leaf_index_buffers;
   uint32_t leaf_index_buffer_stride;

   /* new_sah */
   gpuva_t qnode_buffer;
   gpuva_t bvh2_buffer;
};

static size_t
get_bvh2_size(uint32_t num_primitivies)
{
   if (num_primitivies == 0)
      return 0;
   return sizeof(struct BVH2) +
      (2 * num_primitivies - 1) * sizeof(struct BVH2Node);
}

static struct scratch_layout
get_gpu_scratch_layout(struct anv_address base,
                       struct MKSizeEstimate est,
                       enum anv_rt_bvh_build_method build_method)
{
   struct scratch_layout scratch = {
      .base = anv_address_physical(base),
   };
   gpuva_t current = anv_address_physical(base);

   scratch.globals = current;
   current += sizeof(struct Globals);

   scratch.primrefs = intel_canonical_address(current);
   current += est.numBuildPrimitives * sizeof_PrimRef;

   scratch.leaf_index_buffers = intel_canonical_address(current);
   current += est.numBuildPrimitives * sizeof(uint32_t) * 2;
   scratch.leaf_index_buffer_stride = sizeof(uint32_t);

   switch (build_method) {
   case ANV_BVH_BUILD_METHOD_TRIVIAL:
      break;

   case ANV_BVH_BUILD_METHOD_NEW_SAH: {
      size_t bvh2_size = get_bvh2_size(est.numBuildPrimitives);
      if (est.leaf_data_size < bvh2_size) {
         scratch.bvh2_buffer = intel_canonical_address(current);
         current += bvh2_size;
      }

      scratch.qnode_buffer = intel_canonical_address(current);
      current += 2 * sizeof(dword) * est.max_inner_nodes;
      break;
   }

   default:
      unreachable("invalid build");
   }

   assert((current - scratch.base) < UINT32_MAX);
   scratch.total_size = current - scratch.base;

   return scratch;
}

static void
anv_get_gpu_acceleration_structure_size(
   UNUSED struct anv_device                   *device,
   VkAccelerationStructureBuildTypeKHR         buildType,
   const VkAccelerationStructureBuildGeometryInfoKHR* pBuildInfo,
   const uint32_t*                             pMaxPrimitiveCounts,
   VkAccelerationStructureBuildSizesInfoKHR*   pSizeInfo)
{

   struct MKSizeEstimate est = get_gpu_size_estimate(pBuildInfo, NULL,
                                                     pMaxPrimitiveCounts);
   struct scratch_layout scratch = get_gpu_scratch_layout(ANV_NULL_ADDRESS, est,
                                                          device->bvh_build_method);

   pSizeInfo->accelerationStructureSize = est.sizeTotal;
   pSizeInfo->buildScratchSize = scratch.total_size;
   pSizeInfo->updateScratchSize = scratch.total_size; /* TODO */
}

void
genX(GetAccelerationStructureBuildSizesKHR)(
    VkDevice                                    _device,
    VkAccelerationStructureBuildTypeKHR         buildType,
    const VkAccelerationStructureBuildGeometryInfoKHR* pBuildInfo,
    const uint32_t*                             pMaxPrimitiveCounts,
    VkAccelerationStructureBuildSizesInfoKHR*   pSizeInfo)
{
   ANV_FROM_HANDLE(anv_device, device, _device);
   assert(pSizeInfo->sType ==
          VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_SIZES_INFO_KHR);

   VkAccelerationStructureBuildSizesInfoKHR gpu_size_info;
   anv_get_gpu_acceleration_structure_size(device, buildType, pBuildInfo,
                                           pMaxPrimitiveCounts,
                                           &gpu_size_info);

   pSizeInfo->accelerationStructureSize =
      gpu_size_info.accelerationStructureSize;
   pSizeInfo->buildScratchSize = gpu_size_info.buildScratchSize;
   pSizeInfo->updateScratchSize = gpu_size_info.updateScratchSize;
}

void
genX(GetDeviceAccelerationStructureCompatibilityKHR)(
    VkDevice                                    _device,
    const VkAccelerationStructureVersionInfoKHR* pVersionInfo,
    VkAccelerationStructureCompatibilityKHR*    pCompatibility)
{
   ANV_FROM_HANDLE(anv_device, device, _device);

   if (memcmp(pVersionInfo->pVersionData,
              device->physical->rt_uuid,
              sizeof(device->physical->rt_uuid)) == 0) {
      *pCompatibility = VK_ACCELERATION_STRUCTURE_COMPATIBILITY_COMPATIBLE_KHR;
   } else {
      *pCompatibility = VK_ACCELERATION_STRUCTURE_COMPATIBILITY_INCOMPATIBLE_KHR;
   }
}

static inline uint8_t
vk_to_grl_GeometryFlags(VkGeometryFlagsKHR flags)
{
   uint8_t grl_flags = GEOMETRY_FLAG_NONE;
   unsigned mask = flags;
   while (mask) {
      int i = u_bit_scan(&mask);
      switch ((VkGeometryFlagBitsKHR)(1u << i)) {
      case VK_GEOMETRY_OPAQUE_BIT_KHR:
         grl_flags |= GEOMETRY_FLAG_OPAQUE;
         break;
      case VK_GEOMETRY_NO_DUPLICATE_ANY_HIT_INVOCATION_BIT_KHR:
         grl_flags |= GEOMETRY_FLAG_NO_DUPLICATE_ANYHIT_INVOCATION;
         break;
      default:
         unreachable("Unsupported acceleration structure build flag");
      }
   }
   return grl_flags;
}

static inline IndexFormat
vk_to_grl_IndexFormat(VkIndexType type)
{
   switch (type) {
   case VK_INDEX_TYPE_NONE_KHR:  return INDEX_FORMAT_NONE;
   case VK_INDEX_TYPE_UINT8_EXT: unreachable("No UINT8 support yet");
   case VK_INDEX_TYPE_UINT16:    return INDEX_FORMAT_R16_UINT;
   case VK_INDEX_TYPE_UINT32:    return INDEX_FORMAT_R32_UINT;
   default:
      unreachable("Unsupported index type");
   }
}

static inline VertexFormat
vk_to_grl_VertexFormat(VkFormat format)
{
   switch (format) {
   case VK_FORMAT_R32G32_SFLOAT:       return VERTEX_FORMAT_R32G32_FLOAT;
   case VK_FORMAT_R32G32B32_SFLOAT:    return VERTEX_FORMAT_R32G32B32_FLOAT;
   case VK_FORMAT_R16G16_SFLOAT:       return VERTEX_FORMAT_R16G16_FLOAT;
   case VK_FORMAT_R16G16B16A16_SFLOAT: return VERTEX_FORMAT_R16G16B16A16_FLOAT;
   case VK_FORMAT_R16G16_SNORM:        return VERTEX_FORMAT_R16G16_SNORM;
   case VK_FORMAT_R16G16B16A16_SNORM:  return VERTEX_FORMAT_R16G16B16A16_SNORM;
   case VK_FORMAT_R16G16B16A16_UNORM:  return VERTEX_FORMAT_R16G16B16A16_UNORM;
   case VK_FORMAT_R16G16_UNORM:        return VERTEX_FORMAT_R16G16_UNORM;
   /* case VK_FORMAT_R10G10B10A2_UNORM:   return VERTEX_FORMAT_R10G10B10A2_UNORM; */
   case VK_FORMAT_R8G8B8A8_UNORM:      return VERTEX_FORMAT_R8G8B8A8_UNORM;
   case VK_FORMAT_R8G8_UNORM:          return VERTEX_FORMAT_R8G8_UNORM;
   case VK_FORMAT_R8G8B8A8_SNORM:      return VERTEX_FORMAT_R8G8B8A8_SNORM;
   case VK_FORMAT_R8G8_SNORM:          return VERTEX_FORMAT_R8G8_SNORM;
   default:
      unreachable("Unsupported vertex format");
   }
}

static struct Geo
vk_to_grl_Geo(const VkAccelerationStructureGeometryKHR *pGeometry,
              uint32_t prim_count,
              uint32_t transform_offset,
              uint32_t primitive_offset,
              uint32_t first_vertex)
{
   struct Geo geo = {
      .Flags = vk_to_grl_GeometryFlags(pGeometry->flags),
   };

   switch (pGeometry->geometryType) {
   case VK_GEOMETRY_TYPE_TRIANGLES_KHR: {
      const VkAccelerationStructureGeometryTrianglesDataKHR *vk_tri =
         &pGeometry->geometry.triangles;

      geo.Type = GEOMETRY_TYPE_TRIANGLES;

      geo.Desc.Triangles.pTransformBuffer =
         vk_tri->transformData.deviceAddress;
      geo.Desc.Triangles.pIndexBuffer =
         vk_tri->indexData.deviceAddress;
      geo.Desc.Triangles.pVertexBuffer =
         vk_tri->vertexData.deviceAddress;
      geo.Desc.Triangles.VertexBufferByteStride = vk_tri->vertexStride;

      if (geo.Desc.Triangles.pTransformBuffer)
         geo.Desc.Triangles.pTransformBuffer += transform_offset;

      if (vk_tri->indexType == VK_INDEX_TYPE_NONE_KHR) {
         geo.Desc.Triangles.IndexCount = 0;
         geo.Desc.Triangles.VertexCount = prim_count * 3;
         geo.Desc.Triangles.IndexFormat = INDEX_FORMAT_NONE;
         geo.Desc.Triangles.pVertexBuffer += primitive_offset;
      } else {
         geo.Desc.Triangles.IndexCount = prim_count * 3;
         geo.Desc.Triangles.VertexCount = vk_tri->maxVertex;
         geo.Desc.Triangles.IndexFormat =
            vk_to_grl_IndexFormat(vk_tri->indexType);
         geo.Desc.Triangles.pIndexBuffer += primitive_offset;
      }

      geo.Desc.Triangles.VertexFormat =
         vk_to_grl_VertexFormat(vk_tri->vertexFormat);
      geo.Desc.Triangles.pVertexBuffer += vk_tri->vertexStride * first_vertex;
      break;
   }

   case VK_GEOMETRY_TYPE_AABBS_KHR: {
      const VkAccelerationStructureGeometryAabbsDataKHR *vk_aabbs =
         &pGeometry->geometry.aabbs;
      geo.Type = GEOMETRY_TYPE_PROCEDURAL;
      geo.Desc.Procedural.pAABBs_GPUVA =
         vk_aabbs->data.deviceAddress + primitive_offset;
      geo.Desc.Procedural.AABBByteStride = vk_aabbs->stride;
      geo.Desc.Procedural.AABBCount = prim_count;
      break;
   }

   default:
      unreachable("Invalid geometry type");
   }

   return geo;
}

#include "grl/grl_metakernel_copy.h"
#include "grl/grl_metakernel_misc.h"
#include "grl/grl_metakernel_build_primref.h"
#include "grl/grl_metakernel_new_sah_builder.h"
#include "grl/grl_metakernel_build_leaf.h"

struct build_state {
   enum anv_rt_bvh_build_method build_method;

   struct MKSizeEstimate estimate;
   struct scratch_layout scratch;
   struct MKBuilderState state;

   struct anv_address bvh_addr;

   size_t geom_size_prefix_sum_buffer;
   size_t transient_size;

   uint32_t leaf_type;
   uint32_t leaf_size;

   uint32_t num_geometries;
   uint32_t num_instances;

   uint64_t instances_addr;
   bool array_of_instances_ptr;

   const VkAccelerationStructureGeometryKHR *vk_geoms;
};

static void
get_binnedsah_scratch_buffers(struct build_state *bs,
                              uint64_t *p_qnode_buffer,
                              uint64_t *p_primref_indices,
                              uint64_t *p_bvh2)
{
    if (bs->estimate.numBuildPrimitives == 0)
    {
        *p_bvh2 = 0;
	*p_qnode_buffer = 0;
        *p_primref_indices = 0;
        return;
    }

    size_t bvh2_size = get_bvh2_size(bs->estimate.numBuildPrimitives);
    if (bs->estimate.leaf_data_size < bvh2_size) {
       assert(bs->scratch.bvh2_buffer != 0);
       *p_bvh2 = bs->scratch.bvh2_buffer;
    } else {
       *p_bvh2 = intel_canonical_address(bs->state.bvh_buffer +
                                         bs->estimate.leaf_data_start);
    }

    assert(bs->scratch.qnode_buffer != 0);
    *p_qnode_buffer = bs->scratch.qnode_buffer;

    assert(bs->scratch.leaf_index_buffers != 0);
    *p_primref_indices = bs->scratch.leaf_index_buffers;
}

static void
write_memory(struct anv_cmd_alloc alloc, size_t offset, const void *data, size_t data_len)
{
   assert((offset + data_len) < alloc.size);
   memcpy(alloc.map + offset, data, data_len);
}

static void
cmd_build_acceleration_structures(
   struct anv_cmd_buffer *cmd_buffer,
   uint32_t infoCount,
   const VkAccelerationStructureBuildGeometryInfoKHR *pInfos,
   const VkAccelerationStructureBuildRangeInfoKHR *const *ppBuildRangeInfos,
   const VkDeviceAddress *pIndirectDeviceAddresses,
   const uint32_t *pIndirectStrides,
   const uint32_t *const *ppMaxPrimitiveCounts)
{
   struct anv_device *device = cmd_buffer->device;
   VK_MULTIALLOC(ma);

   struct build_state *builds;
   vk_multialloc_add(&ma, &builds, struct build_state, infoCount);

   if (!vk_multialloc_zalloc(&ma,
                             &cmd_buffer->device->vk.alloc,
                             VK_SYSTEM_ALLOCATION_SCOPE_COMMAND)) {
      anv_batch_set_error(&cmd_buffer->batch, VK_ERROR_OUT_OF_HOST_MEMORY);
      return;
   }

   trace_intel_begin_as_build(&cmd_buffer->trace);

   /* TODO: Indirect */
   assert(ppBuildRangeInfos != NULL);

   size_t transient_mem_init_globals_size = 0;
   size_t transient_mem_init_globals_offset = 0;

   size_t transient_total     = 0;

    size_t private_mem_total = 0;

    size_t num_trivial_builds = 0;
    size_t num_new_sah_builds = 0;

   /* Prepare a bunch of data for the kernels we have to run. */
   for (uint32_t i = 0; i < infoCount; i++) {
      struct build_state *bs = &builds[i];

      const VkAccelerationStructureBuildGeometryInfoKHR *pInfo = &pInfos[i];
      struct anv_address scratch_addr =
         anv_address_from_u64(pInfo->scratchData.deviceAddress);

      const VkAccelerationStructureBuildRangeInfoKHR *pBuildRangeInfos =
         ppBuildRangeInfos ? ppBuildRangeInfos[i] : NULL;
      const uint32_t *pMaxPrimitiveCounts =
         ppMaxPrimitiveCounts ? ppMaxPrimitiveCounts[i] : NULL;

      ANV_FROM_HANDLE(vk_acceleration_structure, dst_accel,
                      pInfo->dstAccelerationStructure);

      bs->build_method = device->bvh_build_method;

      bs->bvh_addr = anv_address_from_u64(vk_acceleration_structure_get_va(dst_accel));

      bs->estimate = get_gpu_size_estimate(pInfo, pBuildRangeInfos,
                                           pMaxPrimitiveCounts);
      bs->scratch = get_gpu_scratch_layout(scratch_addr, bs->estimate,
                                           bs->build_method);

      uint32_t leaf_size, leaf_type;

      switch (pInfo->type) {
      case VK_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL_KHR: {
         assert(pInfo->geometryCount == 1);

         const VkAccelerationStructureGeometryKHR *pGeometry =
            get_geometry(pInfo, 0);
         assert(pGeometry->geometryType == VK_GEOMETRY_TYPE_INSTANCES_KHR);

         const VkAccelerationStructureGeometryInstancesDataKHR *instances =
            &pGeometry->geometry.instances;

         bs->num_instances = pBuildRangeInfos[0].primitiveCount;
         bs->instances_addr = instances->data.deviceAddress;
         bs->array_of_instances_ptr = instances->arrayOfPointers;
         leaf_type = NODE_TYPE_INSTANCE;
         leaf_size = GENX(RT_BVH_INSTANCE_LEAF_length) * 4;
         break;
      }

      case VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR: {
         bs->num_geometries = pInfo->geometryCount;
         leaf_type = NODE_TYPE_QUAD;
         leaf_size = GENX(RT_BVH_QUAD_LEAF_length) * 4;
         break;
      }

      default:
         unreachable("Unsupported acceleration structure type");
      }

      size_t geom_struct_size = bs->num_geometries * sizeof(struct Geo);
      size_t geom_prefix_sum_size = align_uintptr(sizeof(uint32_t) * (bs->num_geometries + 1), 64);

      bs->transient_size = geom_prefix_sum_size + geom_struct_size;

      bs->geom_size_prefix_sum_buffer = transient_total + 0;

      bs->state = (struct MKBuilderState) {
         .geomDesc_buffer = bs->geom_size_prefix_sum_buffer +
                            geom_prefix_sum_size,
         .build_primref_buffer = bs->scratch.primrefs,
         .build_globals = bs->scratch.globals,
         .bvh_buffer = anv_address_physical(bs->bvh_addr),
         .leaf_type = leaf_type,
         .leaf_size = leaf_size,
      };

      transient_total += bs->transient_size;

      switch (device->bvh_build_method) {
      case ANV_BVH_BUILD_METHOD_TRIVIAL:
         num_trivial_builds++;
         break;
      case ANV_BVH_BUILD_METHOD_NEW_SAH:
         num_new_sah_builds++;
         break;
      default:
         unreachable("invalid BVH build method");
      }

      transient_mem_init_globals_size += sizeof(struct BatchedInitGlobalsData);
   }

   transient_total = align_transient_size(transient_total);
   transient_mem_init_globals_offset = transient_total;
   transient_total += align_transient_size(transient_mem_init_globals_size);

   size_t transient_mem_binnedsah_size = 0;
   size_t transient_mem_binnedsah_offset = 0;
   size_t private_mem_binnedsah_size = 0;
   size_t private_mem_binnedsah_offset = 0;

   transient_mem_binnedsah_size = get_batched_binnedsah_transient_mem_size(num_new_sah_builds);
   transient_mem_binnedsah_offset = transient_total;
   transient_total += align_transient_size(transient_mem_binnedsah_size);

   private_mem_binnedsah_size = get_batched_binnedsah_private_mem_size(num_new_sah_builds);
   private_mem_binnedsah_offset = private_mem_total;
   private_mem_total += align_private_size(private_mem_binnedsah_size);

   /* Allocate required memory, unless we already have a suiteable buffer */
   struct anv_cmd_alloc private_mem_alloc;
   if (private_mem_total > cmd_buffer->state.rt.build_priv_mem_size) {
      private_mem_alloc =
         anv_cmd_buffer_alloc_space(cmd_buffer, private_mem_total, 64,
                                    false /* mapped */);
      if (anv_cmd_alloc_is_empty(private_mem_alloc)) {
         anv_batch_set_error(&cmd_buffer->batch, VK_ERROR_OUT_OF_DEVICE_MEMORY);
         goto error;
      }

      cmd_buffer->state.rt.build_priv_mem_addr = private_mem_alloc.address;
      cmd_buffer->state.rt.build_priv_mem_size = private_mem_alloc.size;
   } else {
      private_mem_alloc = (struct anv_cmd_alloc) {
         .address = cmd_buffer->state.rt.build_priv_mem_addr,
         .map     = anv_address_map(cmd_buffer->state.rt.build_priv_mem_addr),
         .size    = cmd_buffer->state.rt.build_priv_mem_size,
      };
   }

   struct anv_cmd_alloc transient_mem_alloc =
      anv_cmd_buffer_alloc_space(cmd_buffer, transient_total, 64,
                                 true /* mapped */);
   if (transient_total > 0 && anv_cmd_alloc_is_empty(transient_mem_alloc)) {
      anv_batch_set_error(&cmd_buffer->batch, VK_ERROR_OUT_OF_DEVICE_MEMORY);
      goto error;
   }

   uint64_t private_base = anv_address_physical(private_mem_alloc.address);
   uint64_t transient_base = anv_address_physical(transient_mem_alloc.address);

   /* Prepare transient memory */
   for (uint32_t i = 0; i < infoCount; i++) {
      struct build_state *bs = &builds[i];

      const VkAccelerationStructureBuildGeometryInfoKHR *pInfo = &pInfos[i];

      const VkAccelerationStructureBuildRangeInfoKHR *pBuildRangeInfos =
         ppBuildRangeInfos ? ppBuildRangeInfos[i] : NULL;

      struct Geo *geos = transient_mem_alloc.map + bs->state.geomDesc_buffer;
      uint32_t *prefixes = transient_mem_alloc.map + bs->geom_size_prefix_sum_buffer;
      uint32_t prefix_sum = 0;
      for (unsigned g = 0; g < bs->num_geometries; g++) {
         const VkAccelerationStructureGeometryKHR *pGeometry = get_geometry(pInfo, g);
         uint32_t prim_count = pBuildRangeInfos[g].primitiveCount;
         geos[g] = vk_to_grl_Geo(pGeometry, prim_count,
                                 pBuildRangeInfos[g].transformOffset,
                                 pBuildRangeInfos[g].primitiveOffset,
                                 pBuildRangeInfos[g].firstVertex);

         prefixes[g] = prefix_sum;
         prefix_sum += prim_count;
      }

      prefixes[bs->num_geometries] = prefix_sum;

      bs->geom_size_prefix_sum_buffer =
         intel_canonical_address(bs->geom_size_prefix_sum_buffer +
                                 transient_base);
      bs->state.geomDesc_buffer =
         intel_canonical_address(bs->state.geomDesc_buffer +
                                 transient_base);

      struct BatchedInitGlobalsData data = {
         .p_build_globals = bs->scratch.globals,
         .p_bvh_buffer = anv_address_physical(bs->bvh_addr),

         .numPrimitives = 0,
         .numGeometries = bs->num_geometries,
         .numInstances = bs->num_instances,

         .instance_descs_start = bs->estimate.instance_descs_start,
         .geo_meta_data_start = bs->estimate.geo_meta_data_start,
         .node_data_start = bs->estimate.node_data_start,
         .leaf_data_start = bs->estimate.leaf_data_start,
         .procedural_data_start = bs->estimate.procedural_data_start,
         .back_pointer_start = bs->estimate.back_pointer_start,
         .sizeTotal = bs->estimate.sizeTotal,

         .leafType = bs->state.leaf_type,
         .leafSize = bs->state.leaf_size,
      };

      write_memory(transient_mem_alloc,
                   transient_mem_init_globals_offset + i * sizeof(data),
                   &data, sizeof(data));
   }

   if (anv_cmd_buffer_is_render_queue(cmd_buffer))
      genX(flush_pipeline_select_gpgpu)(cmd_buffer);

   /* Due to the nature of GRL and its heavy use of jumps/predication, we
    * cannot tell exactly in what order the CFE_STATE we insert are going to
    * be executed. So always use the largest possible size.
    */
   genX(cmd_buffer_ensure_cfe_state)(
      cmd_buffer,
      cmd_buffer->device->physical->max_grl_scratch_size);

   /* Round 1 : init_globals kernel */
   genX(grl_misc_batched_init_globals)(
      cmd_buffer,
      intel_canonical_address(transient_base +
                              transient_mem_init_globals_offset),
      infoCount);

   anv_add_pending_pipe_bits(cmd_buffer,
                             ANV_GRL_FLUSH_FLAGS,
                             "building accel struct");

   /* Round 2 : Copy instance/geometry data from the application provided
    *           buffers into the acceleration structures.
    */
   for (uint32_t i = 0; i < infoCount; i++) {
      struct build_state *bs = &builds[i];

      /* Metadata */
      if (bs->num_instances) {
         assert(bs->num_geometries == 0);

         const uint64_t copy_size = bs->num_instances * sizeof(InstanceDesc);
         /* This must be calculated in same way as
          * groupCountForGeoMetaDataCopySize
          */
         const uint32_t num_threads = (copy_size >> 8) + 3;

         if (bs->array_of_instances_ptr) {
            genX(grl_misc_copy_instance_ptrs)(
               cmd_buffer,
               anv_address_physical(anv_address_add(bs->bvh_addr,
                                                    bs->estimate.instance_descs_start)),
               bs->instances_addr,
               copy_size, num_threads);
         } else {
            genX(grl_misc_copy_instances)(
               cmd_buffer,
               anv_address_physical(anv_address_add(bs->bvh_addr,
                                                    bs->estimate.instance_descs_start)),
               bs->instances_addr,
               copy_size, num_threads);
         }
      }

      if (bs->num_geometries) {
         assert(bs->num_instances == 0);
         const uint64_t copy_size = bs->num_geometries * sizeof(struct GeoMetaData);

         /* This must be calculated in same way as
          * groupCountForGeoMetaDataCopySize
          */
         const uint32_t num_threads = (copy_size >> 6) + 1;

         genX(grl_misc_copy_geo_meta_data)(
            cmd_buffer,
            anv_address_physical(anv_address_add(bs->bvh_addr,
                                                 bs->estimate.geo_meta_data_start)),
            bs->state.geomDesc_buffer,
            copy_size,
            num_threads);
      }

      /* Primrefs */
      if (bs->num_instances) {
         if (bs->array_of_instances_ptr) {
            genX(grl_build_primref_buildPrimirefsFromInstancesArrOfPtrs)(
               cmd_buffer,
               bs->instances_addr,
               PREFIX_MK_SIZE(grl_build_primref, bs->estimate),
               PREFIX_MK_STATE(grl_build_primref, bs->state),
               false /* allowUpdate */);
         } else {
            genX(grl_build_primref_buildPrimirefsFromInstances)(
               cmd_buffer,
               bs->instances_addr,
               PREFIX_MK_SIZE(grl_build_primref, bs->estimate),
               PREFIX_MK_STATE(grl_build_primref, bs->state),
               false /* allowUpdate */);
         }
      }

      if (bs->num_geometries) {
         const VkAccelerationStructureBuildGeometryInfoKHR *pInfo = &pInfos[i];
         const VkAccelerationStructureBuildRangeInfoKHR *pBuildRangeInfos =
            ppBuildRangeInfos ? ppBuildRangeInfos[i] : NULL;

         assert(pInfo->geometryCount == bs->num_geometries);
         for (unsigned g = 0; g < pInfo->geometryCount; g++) {
            const VkAccelerationStructureGeometryKHR *pGeometry =
               get_geometry(pInfo, g);

            switch (pGeometry->geometryType) {
            case VK_GEOMETRY_TYPE_TRIANGLES_KHR:
               genX(grl_build_primref_primrefs_from_tris)(
                  cmd_buffer,
                  PREFIX_MK_STATE(grl_build_primref, bs->state),
                  PREFIX_MK_SIZE(grl_build_primref, bs->estimate),
                  bs->state.geomDesc_buffer + g * sizeof(struct Geo),
                  g,
                  vk_to_grl_GeometryFlags(pGeometry->flags),
                  /* TODO: Indirect */
                  pBuildRangeInfos[g].primitiveCount);
               break;

            case VK_GEOMETRY_TYPE_AABBS_KHR:
               genX(grl_build_primref_primrefs_from_proc)(
                  cmd_buffer,
                  PREFIX_MK_STATE(grl_build_primref, bs->state),
                  PREFIX_MK_SIZE(grl_build_primref, bs->estimate),
                  bs->state.geomDesc_buffer + g * sizeof(struct Geo),
                  g,
                  vk_to_grl_GeometryFlags(pGeometry->flags),
                  /* TODO: Indirect */
                  pBuildRangeInfos[g].primitiveCount);
               break;

            default:
               unreachable("Invalid geometry type");
            }
         }
      }
   }

   anv_add_pending_pipe_bits(cmd_buffer,
                             ANV_GRL_FLUSH_FLAGS,
                             "building accel struct");

   /* Dispatch trivial builds */
   if (num_trivial_builds) {
      for (uint32_t i = 0; i < infoCount; i++) {
         struct build_state *bs = &builds[i];

         if (bs->build_method != ANV_BVH_BUILD_METHOD_TRIVIAL)
            continue;

         genX(grl_new_sah_builder_single_pass_binsah)(
            cmd_buffer,
            bs->scratch.globals,
            bs->state.bvh_buffer,
            bs->state.build_primref_buffer,
            bs->scratch.leaf_index_buffers,
            false /* alloc_backpointers */);
      }
   }

   /* Dispatch new SAH builds */
   if (num_new_sah_builds) {
      size_t global_ptrs_offset  = transient_mem_binnedsah_offset;
      size_t buffers_info_offset = transient_mem_binnedsah_offset + sizeof(gpuva_t) * num_new_sah_builds;

      size_t scheduler_offset   = private_mem_binnedsah_offset;
      size_t sah_globals_offset = private_mem_binnedsah_offset + get_scheduler_size(num_new_sah_builds);

      struct SAHBuildArgsBatchable args = {
         .num_builds                               = infoCount,
         .p_globals_ptrs                           = intel_canonical_address(transient_base + global_ptrs_offset),
         .p_buffers_info                           = intel_canonical_address(transient_base + buffers_info_offset),
         .p_scheduler                              = intel_canonical_address(private_base + scheduler_offset),
         .p_sah_globals                            = intel_canonical_address(private_base + sah_globals_offset),
         .num_max_qnode_global_root_buffer_entries = MAX2(num_new_sah_builds, QNODE_GLOBAL_ROOT_BUFFER_MIN_ENTRIES_NUM),
      };

      for (uint32_t i = 0; i < infoCount; i++) {
         struct build_state *bs = &builds[i];

         if (bs->build_method != ANV_BVH_BUILD_METHOD_NEW_SAH)
            continue;

         uint64_t p_build_primref_index_buffers;
         uint64_t p_bvh2;
         uint64_t p_qnode_child_buffer;

         get_binnedsah_scratch_buffers(bs,
                                       &p_qnode_child_buffer,
                                       &p_build_primref_index_buffers,
                                       &p_bvh2);

         struct SAHBuildBuffersInfo buffers = {
            .p_primref_index_buffers  = bs->scratch.leaf_index_buffers,
            .p_bvh_base               = bs->state.bvh_buffer,
            .p_primrefs_buffer        = bs->state.build_primref_buffer,
            .p_bvh2                   = p_bvh2,
            .p_qnode_root_buffer      = p_qnode_child_buffer,
            .sah_globals_flags        = 0,
         };

         write_memory(transient_mem_alloc, buffers_info_offset, &buffers, sizeof(buffers));
         buffers_info_offset += sizeof(buffers);

         write_memory(transient_mem_alloc, global_ptrs_offset, &bs->state.build_globals,
                      sizeof(bs->state.build_globals));
         global_ptrs_offset += sizeof(bs->state.build_globals);
      }

      genX(grl_new_sah_builder_new_sah_build_batchable)(
         cmd_buffer, PREFIX_MK_SAH_BUILD_ARGS_BATCHABLE(grl_new_sah_builder, args));
   }

   if (num_new_sah_builds == 0)
      anv_add_pending_pipe_bits(cmd_buffer,
                              ANV_GRL_FLUSH_FLAGS,
                             "building accel struct");

   /* Finally write the leaves. */
   for (uint32_t i = 0; i < infoCount; i++) {
      struct build_state *bs = &builds[i];

      if (bs->num_instances) {
         assert(bs->num_geometries == 0);
         if (bs->array_of_instances_ptr) {
            genX(grl_leaf_builder_buildLeafDXR_instances_pointers)(cmd_buffer,
               PREFIX_MK_STATE(grl_leaf_builder, bs->state),
               bs->scratch.leaf_index_buffers,
               bs->instances_addr,
               bs->scratch.leaf_index_buffer_stride,
               0 /* offset */,
               bs->estimate.numBuildPrimitives);
         } else {
            genX(grl_leaf_builder_buildLeafDXR_instances)(cmd_buffer,
               PREFIX_MK_STATE(grl_leaf_builder, bs->state),
               bs->scratch.leaf_index_buffers,
               bs->instances_addr,
               bs->scratch.leaf_index_buffer_stride,
               0 /* offset */,
               bs->estimate.numBuildPrimitives);
         }
      }

      if (bs->num_geometries) {
         assert(bs->num_instances == 0);
         const uint64_t p_numPrimitives =
            bs->state.build_globals + offsetof(struct Globals, numPrimitives);

         assert(bs->estimate.numProcedurals == 0 ||
                bs->estimate.numTriangles == 0);
         if (bs->estimate.numProcedurals) {
            genX(grl_leaf_builder_buildLeafDXR_procedurals)(
               cmd_buffer,
               PREFIX_MK_STATE(grl_leaf_builder, bs->state),
               bs->scratch.leaf_index_buffers,
               bs->scratch.leaf_index_buffer_stride,
               0 /* offset */,
               p_numPrimitives);
         } else {
            genX(grl_leaf_builder_buildLeafDXR_quads)(
               cmd_buffer,
               PREFIX_MK_STATE(grl_leaf_builder, bs->state),
               bs->scratch.leaf_index_buffers,
               bs->scratch.leaf_index_buffer_stride,
               0 /* offset */,
               p_numPrimitives,
               false /* allow_updates */);
         }
      }
   }

   anv_add_pending_pipe_bits(cmd_buffer,
                             ANV_GRL_FLUSH_FLAGS,
                             "building accel struct");

   trace_intel_end_as_build(&cmd_buffer->trace);

 error:
   vk_free(&cmd_buffer->device->vk.alloc, builds);
}

void
genX(CmdBuildAccelerationStructuresKHR)(
    VkCommandBuffer                             commandBuffer,
    uint32_t                                    infoCount,
    const VkAccelerationStructureBuildGeometryInfoKHR* pInfos,
    const VkAccelerationStructureBuildRangeInfoKHR* const* ppBuildRangeInfos)
{
   ANV_FROM_HANDLE(anv_cmd_buffer, cmd_buffer, commandBuffer);

   if (anv_batch_has_error(&cmd_buffer->batch))
      return;

   cmd_build_acceleration_structures(cmd_buffer, infoCount, pInfos,
                                     ppBuildRangeInfos, NULL, NULL, NULL);
}

void
genX(CmdBuildAccelerationStructuresIndirectKHR)(
    VkCommandBuffer                             commandBuffer,
    uint32_t                                    infoCount,
    const VkAccelerationStructureBuildGeometryInfoKHR* pInfos,
    const VkDeviceAddress*                      pIndirectDeviceAddresses,
    const uint32_t*                             pIndirectStrides,
    const uint32_t* const*                      ppMaxPrimitiveCounts)
{
   unreachable("Unimplemented");
}

void
genX(CmdCopyAccelerationStructureKHR)(
    VkCommandBuffer                             commandBuffer,
    const VkCopyAccelerationStructureInfoKHR*   pInfo)
{
   ANV_FROM_HANDLE(anv_cmd_buffer, cmd_buffer, commandBuffer);
   ANV_FROM_HANDLE(vk_acceleration_structure, src_accel, pInfo->src);
   ANV_FROM_HANDLE(vk_acceleration_structure, dst_accel, pInfo->dst);

   assert(pInfo->mode == VK_COPY_ACCELERATION_STRUCTURE_MODE_COMPACT_KHR ||
          pInfo->mode == VK_COPY_ACCELERATION_STRUCTURE_MODE_CLONE_KHR);

   if (pInfo->mode == VK_COPY_ACCELERATION_STRUCTURE_MODE_CLONE_KHR) {
      uint64_t src_size_addr =
         vk_acceleration_structure_get_va(src_accel) +
         offsetof(struct BVHBase, Meta.allocationSize);
      genX(grl_copy_clone_indirect)(
         cmd_buffer,
         vk_acceleration_structure_get_va(dst_accel),
         vk_acceleration_structure_get_va(src_accel),
         src_size_addr);
   } else {
      genX(grl_copy_compact)(
         cmd_buffer,
         vk_acceleration_structure_get_va(dst_accel),
         vk_acceleration_structure_get_va(src_accel));
   }

   anv_add_pending_pipe_bits(cmd_buffer,
                             ANV_PIPE_END_OF_PIPE_SYNC_BIT,
                             "after copy acceleration struct");
}

void
genX(CmdCopyAccelerationStructureToMemoryKHR)(
    VkCommandBuffer                             commandBuffer,
    const VkCopyAccelerationStructureToMemoryInfoKHR* pInfo)
{
   ANV_FROM_HANDLE(anv_cmd_buffer, cmd_buffer, commandBuffer);
   ANV_FROM_HANDLE(vk_acceleration_structure, src_accel, pInfo->src);
   struct anv_device *device = cmd_buffer->device;
   uint64_t src_size_addr =
      vk_acceleration_structure_get_va(src_accel) +
      offsetof(struct BVHBase, Meta.allocationSize);

   assert(pInfo->mode == VK_COPY_ACCELERATION_STRUCTURE_MODE_SERIALIZE_KHR);

   genX(grl_copy_serialize_indirect)(
      cmd_buffer,
      pInfo->dst.deviceAddress,
      vk_acceleration_structure_get_va(src_accel),
      anv_address_physical(device->rt_uuid_addr),
      src_size_addr);

   anv_add_pending_pipe_bits(cmd_buffer,
                             ANV_PIPE_END_OF_PIPE_SYNC_BIT,
                             "after copy acceleration struct");
}

void
genX(CmdCopyMemoryToAccelerationStructureKHR)(
    VkCommandBuffer                             commandBuffer,
    const VkCopyMemoryToAccelerationStructureInfoKHR* pInfo)
{
   ANV_FROM_HANDLE(anv_cmd_buffer, cmd_buffer, commandBuffer);
   ANV_FROM_HANDLE(vk_acceleration_structure, dst_accel, pInfo->dst);

   assert(pInfo->mode == VK_COPY_ACCELERATION_STRUCTURE_MODE_DESERIALIZE_KHR);

   uint64_t src_size_addr = pInfo->src.deviceAddress +
      offsetof(struct SerializationHeader, DeserializedSizeInBytes);
   genX(grl_copy_deserialize_indirect)(
      cmd_buffer,
      vk_acceleration_structure_get_va(dst_accel),
      pInfo->src.deviceAddress,
      src_size_addr);

   anv_add_pending_pipe_bits(cmd_buffer,
                             ANV_PIPE_END_OF_PIPE_SYNC_BIT,
                             "after copy acceleration struct");
}

/* TODO: Host commands */

VkResult
genX(BuildAccelerationStructuresKHR)(
    VkDevice                                    _device,
    VkDeferredOperationKHR                      deferredOperation,
    uint32_t                                    infoCount,
    const VkAccelerationStructureBuildGeometryInfoKHR* pInfos,
    const VkAccelerationStructureBuildRangeInfoKHR* const* ppBuildRangeInfos)
{
   ANV_FROM_HANDLE(anv_device, device, _device);
   unreachable("Unimplemented");
   return vk_error(device, VK_ERROR_FEATURE_NOT_PRESENT);
}

VkResult
genX(CopyAccelerationStructureKHR)(
    VkDevice                                    _device,
    VkDeferredOperationKHR                      deferredOperation,
    const VkCopyAccelerationStructureInfoKHR*   pInfo)
{
   ANV_FROM_HANDLE(anv_device, device, _device);
   unreachable("Unimplemented");
   return vk_error(device, VK_ERROR_FEATURE_NOT_PRESENT);
}

VkResult
genX(CopyAccelerationStructureToMemoryKHR)(
    VkDevice                                    _device,
    VkDeferredOperationKHR                      deferredOperation,
    const VkCopyAccelerationStructureToMemoryInfoKHR* pInfo)
{
   ANV_FROM_HANDLE(anv_device, device, _device);
   unreachable("Unimplemented");
   return vk_error(device, VK_ERROR_FEATURE_NOT_PRESENT);
}

VkResult
genX(CopyMemoryToAccelerationStructureKHR)(
    VkDevice                                    _device,
    VkDeferredOperationKHR                      deferredOperation,
    const VkCopyMemoryToAccelerationStructureInfoKHR* pInfo)
{
   ANV_FROM_HANDLE(anv_device, device, _device);
   unreachable("Unimplemented");
   return vk_error(device, VK_ERROR_FEATURE_NOT_PRESENT);
}

VkResult
genX(WriteAccelerationStructuresPropertiesKHR)(
    VkDevice                                    _device,
    uint32_t                                    accelerationStructureCount,
    const VkAccelerationStructureKHR*           pAccelerationStructures,
    VkQueryType                                 queryType,
    size_t                                      dataSize,
    void*                                       pData,
    size_t                                      stride)
{
   ANV_FROM_HANDLE(anv_device, device, _device);
   unreachable("Unimplemented");
   return vk_error(device, VK_ERROR_FEATURE_NOT_PRESENT);
}

#endif /* GFX_VERx10 >= 125 */
