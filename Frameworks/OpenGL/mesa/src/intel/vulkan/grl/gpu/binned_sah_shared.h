//
// Copyright (C) 2009-2021 Intel Corporation
//
// SPDX-License-Identifier: MIT
//
//

//
//   This file contains structure definitions shared by GRL OCL kernels and host code
//

#include "GRLGen12.h"
#pragma once

#define BFS_NUM_BINS        16
#define BFS_NUM_VCONTEXTS   256
#define BFS_MAX_DEPTH 32

#define TRIVIAL_BUILD_THRESHOLD   6
#define SINGLE_WG_BUILD_THRESHOLD 256

#define QNODE_GLOBAL_ROOT_BUFFER_MIN_ENTRIES_NUM 16384


typedef uchar vcontext_id_t;

GRL_NAMESPACE_BEGIN(GRL)
GRL_NAMESPACE_BEGIN(RTAS)
GRL_NAMESPACE_BEGIN(GPUBVHBuilder)

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

enum_uint8(SAHBuildFlags)
{
    SAH_FLAG_NEED_BACKPOINTERS = 1,        // identifies a mixed internal node where each child can have a different type
    SAH_FLAG_NEED_MASKS        = 2
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


GRL_NAMESPACE_END(GPUBVHBuilder)
GRL_NAMESPACE_END(RTAS)
GRL_NAMESPACE_END(GRL)
