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

/**
 * This file contains a redefinition of structures defined in the GRL library.
 * We need to have those structures defined to allocate & prepare data for
 * the OpenCL kernels building acceleration structures. Unfortunately because
 * of C++ & OpenCL assumptions in GRL, it's no possible to just include GRL
 * header files directly so we have to redefine stuff here.
 */

#ifndef GRL_STRUCTS_H
#define GRL_STRUCTS_H

#include "GRLStructs.h"
#include "GRLRTASCommon.h"

struct MKBuilderState {
   qword geomDesc_buffer;
   qword build_primref_buffer;
   qword build_globals;
   qword bvh_buffer;
   dword leaf_type;
   dword leaf_size;
};

#define PREFIX_MK_STATE(prefix, obj) \
   (struct prefix##_MKBuilderState) { \
      .geomDesc_buffer = (obj).geomDesc_buffer, \
      .build_primref_buffer = (obj).build_primref_buffer, \
      .build_globals = (obj).build_globals, \
      .bvh_buffer = (obj).bvh_buffer, \
      .leaf_type = (obj).leaf_type, \
      .leaf_size = (obj).leaf_size, \
   }

struct MKSizeEstimate {
   dword numTriangles;
   dword numProcedurals;
   dword numPrimitives;
   dword numMeshes;
   dword numBuildPrimitives;
   dword numPrimitivesToSplit;
   dword instance_descs_start;
   dword geo_meta_data_start;
   dword node_data_start;
   dword leaf_data_start;
   dword procedural_data_start;
   dword back_pointer_start;
   dword sizeTotal;
   dword updateScratchSizeTotal;
   dword fatleaf_table_start;
   dword innernode_table_start;
   dword max_fatleaves;

   size_t max_instance_leafs;
   size_t max_inner_nodes;
   size_t leaf_data_size;
   size_t min_primitives;
   size_t max_primitives;
};

#define PREFIX_MK_SIZE(prefix, obj) \
   (struct prefix##_MKSizeEstimate) { \
      .numTriangles = (obj).numTriangles, \
      .numProcedurals = (obj).numProcedurals, \
      .numPrimitives = (obj).numPrimitives, \
      .numMeshes = (obj).numMeshes, \
      .numBuildPrimitives = (obj).numBuildPrimitives, \
      .numPrimitivesToSplit = (obj).numPrimitivesToSplit, \
      .instance_descs_start = (obj).instance_descs_start, \
      .geo_meta_data_start = (obj).geo_meta_data_start, \
      .node_data_start = (obj).node_data_start, \
      .leaf_data_start = (obj).leaf_data_start, \
      .procedural_data_start = (obj).procedural_data_start, \
      .back_pointer_start = (obj).back_pointer_start, \
      .sizeTotal = (obj).sizeTotal, \
      .updateScratchSizeTotal = (obj).updateScratchSizeTotal, \
      .fatleaf_table_start = (obj).fatleaf_table_start, \
      .innernode_table_start = (obj).innernode_table_start, \
      .max_fatleaves = (obj).max_fatleaves, \
   }

typedef struct AABB {
   float lower[4];
   float upper[4];
} AABB;

struct Globals
{
   struct AABB centroidBounds;

   unsigned int build_record_start;
   unsigned int numPrimitives;
   unsigned int leafPrimType;
   unsigned int leafSize;

   unsigned int numSplittedPrimitives;
   unsigned int numBuildRecords;

   // spatial split sate
   unsigned int numOriginalPrimitives;
   float presplitPrioritySum;
   float probThreshold;

   // binned-sah bfs state
   unsigned int counter;
   unsigned int numBuildRecords_extended;

   // sync variable used for global-sync on work groups
   unsigned int sync;


   /* morton code builder state */
   unsigned int shift;      // used by adaptive mc-builder
   unsigned int shift_mask; // used by adaptive mc-builder
   unsigned int binary_hierarchy_root;
   unsigned int p0_allocated_num;
   unsigned int p0_created_num;
   unsigned int morton_sort_in_flight;
   unsigned int sort_iterations;

   gpuva_t binary_hierarchy_buffer; // pointer to the binary morton code hierarchy.  Stashed here as a debug aid
};

typedef struct BVHBase
{
   // TODO:  Implement the "copy-first-node" trick... duplicate root node here

   uint64_t rootNodeOffset;

   uint32_t reserved;

   uint32_t nodeDataCur; // nodeDataStart is sizeof(BVHBase) / 64 = BVH_ROOT_NODE_OFFSET / 64
   uint32_t quadLeafStart;
   uint32_t quadLeafCur;
   uint32_t proceduralDataStart;
   uint32_t proceduralDataCur;
   uint32_t instanceLeafStart;
   uint32_t instanceLeafEnd;
   uint32_t backPointerDataStart;     //
   uint32_t refitTreeletsDataStart;   // refit structs
   uint32_t refitStartPointDataStart; //
   uint32_t BVHDataEnd;

   // number of bottom treelets
   // if 1, then the bottom treelet is also tip treelet
   uint32_t refitTreeletCnt;
   uint32_t refitTreeletCnt2; // always 0, used for atomic updates
   // data layout:
   // @backPointerDataStart
   //  'backpointer' - a dword per inner node.
   //  The bits are used as follows:
   //     2:0  --> Used as a refit counter during BVH refitting.  MBZ
   //     5:3  --> Number of children
   //     31:6 --> Index of the parent node in the internal node array
   //    The root node has a parent index of all ones
   // @refitTreeletsDataStart
   //  RefitTreelet[], the last treelet is for top treelet all previous are for bottom
   // @refitStartPointDataStart
   //  for each treelet T there is [T.startpoint_offset, T.numStartpoints) interval of startpoints here in that space
   // @backPointerDataEnd

   uint32_t fatLeafCount;  // number of internal nodes which are "fat-leaves"
   uint32_t innerCount;    // number of internal nodes which are true inner nodes (all internalNode children)
   uint32_t fatLeafTableStart;
   uint32_t innerTableStart;

   uint32_t _pad[12];

   struct RTASMetaData Meta;
} BVHBase;


struct BatchedInitGlobalsData
{
   qword p_build_globals;
   qword p_bvh_buffer;
   dword numPrimitives;
   dword numGeometries;
   dword numInstances;
   dword instance_descs_start;
   dword geo_meta_data_start;
   dword node_data_start;
   dword leaf_data_start;
   dword procedural_data_start;
   dword back_pointer_start;
   dword sizeTotal;
   dword leafType;
   dword leafSize;
   dword fatleaf_table_start;
   dword innernode_table_start;
};


#define BFS_NUM_BINS        16
#define BFS_NUM_VCONTEXTS   256
#define BFS_MAX_DEPTH 32

#define QNODE_GLOBAL_ROOT_BUFFER_MIN_ENTRIES_NUM 16384

struct BFS_Split
{
   float sah;
   int dim;
   int pos;
};

struct BFS_BinInfo
{
   float min_max[18 * BFS_NUM_BINS]; //  layout: bins[axis][num_bins][6]
   //          The 6 are lower(xyz) and -upper(xyz)
   // bins use negated-max so that we can use vectorized mins instead of min/max pairs
   uint counts[3 * BFS_NUM_BINS];
};

struct SAHBuildGlobals
{
   qword   p_primref_index_buffers;
   qword   p_primrefs_buffer;
   qword   p_bvh2;
   qword   p_globals;     // TODO: deprecate this
   qword   p_bvh_base;
   gpuva_t p_qnode_root_buffer;

   dword flags; // bit 1 is 'alloc_backpointers'.  bit 2 is 'need_masks'
   dword num_primrefs;
   dword leaf_size;
   dword leaf_type;

   dword root_buffer_num_produced;
   dword root_buffer_num_produced_hi;
   dword root_buffer_num_consumed;
   dword root_buffer_num_consumed_hi;
   dword root_buffer_num_to_consume;
   dword root_buffer_num_to_consume_hi;
};

typedef union LRBounds
{
   struct
   {
      struct AABB3f left_centroid_bounds;
      struct AABB3f left_geom_bounds;
      struct AABB3f right_centroid_bounds;
      struct AABB3f right_geom_bounds;
   } boxes;
   struct
   {
      float Array[24];
   } scalars;
} LRBounds;


struct VContext
{
   uint dispatch_primref_begin;    // range of primrefs for this task
   uint dispatch_primref_end;
   uint bvh2_root;                 // BVH2 root node for this task
   uint tree_depth;                // depth of this node in the tree
   uint num_left;          // primref counts
   uint num_right;
   uint lr_mask;      // lower 8b : left mask.  upper 8b : right mask
   uint batch_index;

   // pass1 global working state and output
   struct BFS_Split split;
   struct BFS_BinInfo global_bin_info;

   // pass2 global working state and output
   LRBounds lr_bounds;
};



struct BFSDispatchRecord
{
   ushort batch_index;
   ushort context_id;
};


struct BFSDispatchQueue
{
   uint num_dispatches;
   uint wg_count[BFS_NUM_VCONTEXTS];
   struct BFSDispatchRecord records[BFS_NUM_VCONTEXTS];
};

struct BFS1SpillStackEntry
{
   uint primref_begin;
   uint primref_end;
   uint bvh2_root;
   ushort tree_depth;
   ushort batch_index;
};

struct BFS1SpillStack
{
   uint size;
   struct BFS1SpillStackEntry entries[BFS_NUM_VCONTEXTS * BFS_MAX_DEPTH];
};

struct QNodeGlobalRootBufferEntry
{
   uint bvh2_node;
   uint qnode;
   uint build_idx;
   uint _pad;
};

struct QNodeGlobalRootBuffer
{
   uint curr_entries_offset; // we use "entries" as two buffers, so offset is either 0 or QNODE_GLOBAL_ROOT_BUFFER_MIN_ENTRIES_NUM
   struct QNodeGlobalRootBufferEntry entries[QNODE_GLOBAL_ROOT_BUFFER_MIN_ENTRIES_NUM * 2];
};

struct DFSDispatchRecord
{
   uint primref_base;
   uint bvh2_base;
   uint batch_index;
   ushort num_primrefs;
   ushort tree_depth;
};


struct DFSDispatchQueue
{
   struct DFSDispatchRecord records[BFS_NUM_VCONTEXTS * 2];
};

#define VCONTEXT_STATE_EXECUTING   0
#define VCONTEXT_STATE_UNALLOCATED 1

union SchedulerUnion
{
   struct VContextScheduler
   {
      /////////////////////////////////////////////////////////////
      //  State data used for communication with command streamer
      //   NOTE: This part must match definition in 'new_sah_builder.grl'
      /////////////////////////////////////////////////////////////

      dword num_bfs_wgs;
      dword num_dfs_wgs;

      dword scheduler_postsync;
      dword _pad1;

      dword num_trivial_builds; // number of trivial builds (#primrefs < leaf_size).
      dword num_single_builds;  // number of single-wg builds (#primrefs < threshold)

      dword batched_build_wg_count;  // number of wgs to dispatch for initial BFS pass
      dword batched_build_loop_mask; // value is 0 if  #builds <= #contexts.  else 1  command streamer uses this as a loop condition

      /////////////////////////////////////////////////////////////

      dword batched_build_count;  // number of batched builds in the SAHBuildGlobals buffer
      dword batched_build_offset; // location of the first batched-build in the SAHBuildGlobals buffer

      dword vcontext_state[BFS_NUM_VCONTEXTS];

      struct BFSDispatchQueue bfs_queue;
      struct DFSDispatchQueue dfs_queue;

      struct VContext contexts[BFS_NUM_VCONTEXTS];

      struct BFS1SpillStack bfs2_spill_stack;
   } vContextScheduler;

   struct QnodeScheduler
   {
      dword num_qnode_grb_curr_entries;
      dword num_qnode_grb_new_entries;

      dword scheduler_postsync;
      dword _pad1;

      dword num_trivial_builds; // number of trivial builds (#primrefs < leaf_size).
      dword num_single_builds;  // number of single-wg builds (#primrefs < threshold)

      dword batched_builds_to_process;
      dword num_max_qnode_global_root_buffer_entries; // number of maximum entries to global root buffer

      /////////////////////////////////////////////////////////////

      dword batched_build_count;  // number of batched builds in the SAHBuildGlobals buffer
      dword batched_build_offset; // location of the first batched-build in the SAHBuildGlobals buffer

      struct QNodeGlobalRootBuffer qnode_global_root_buffer;
   } qnodeScheduler;
};


struct BVH2Node
{
   struct AABB3f box;
   uint  meta_u;   // leaf:  primref start.  inner: offset from node to its first child
   uint  meta_ss;
   //ushort meta_s;   // leaf: primref count.  inner: offset from first to second child, in nodes
   //uchar is_inner; //  1 if inner, 0 if leaf
   //uchar mask;
};

struct BVH2
{
   uint num_nodes;
   uint _pad[7];  // align to 32B
};

struct BatchedBLSDispatchEntry
{
   /////////////////////////////////////////////////////////////
   //  State data used for communication with command streamer
   //  NOTE: This part must match definition in 'msb_radix_bitonic_sort.grl'
   /////////////////////////////////////////////////////////////
   qword p_data_buffer;
   qword num_elements; // number of elements in p_data_buffer
};

struct SAHBuildArgsBatchable
{
   qword p_globals_ptrs;
   qword p_scheduler;
   qword p_buffers_info;
   qword p_sah_globals;

   dword num_max_qnode_global_root_buffer_entries;
   dword num_builds;
};

#define PREFIX_MK_SAH_BUILD_ARGS_BATCHABLE(prefix, obj) \
   (struct prefix##_SAHBuildArgsBatchable) { \
      .p_globals_ptrs = (obj).p_globals_ptrs, \
      .p_scheduler = (obj).p_scheduler, \
      .p_buffers_info = (obj).p_buffers_info, \
      .p_sah_globals = (obj).p_sah_globals, \
      .num_max_qnode_global_root_buffer_entries = \
      (obj).num_max_qnode_global_root_buffer_entries, \
      .num_builds = (obj).num_builds, \
   }


struct SAHBuildBuffersInfo
{
   gpuva_t p_globals;
   gpuva_t p_primref_index_buffers;
   gpuva_t p_primrefs_buffer;
   gpuva_t p_bvh2;
   gpuva_t p_bvh_base;
   gpuva_t p_qnode_root_buffer;
   dword   sah_globals_flags;
   dword   _pad;
   gpuva_t _pad2;
};

#endif /* GRL_STRUCTS_H */
