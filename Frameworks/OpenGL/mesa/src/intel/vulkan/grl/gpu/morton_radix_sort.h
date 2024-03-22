//
// Copyright (C) 2009-2021 Intel Corporation
//
// SPDX-License-Identifier: MIT
//
//

#pragma once

#include "common.h"
#include "libs/lsc_intrinsics.h"

/* ============================================================================= */
/* ============================== LSB RADIX SORT =============================== */
/* ============================================================================= */

#define RADIX_BINS 256
#define SCATTER_WG_SIZE 512
#define MORTON_LSB_SORT_NO_SHIFT_THRESHOLD 0xFFFFFFFF // turn off, because current hierarchy build requires full sort

uint2 get_thread_range( uint numItems, uint numGroups, uint taskID )
{
    uint items_per_group = (numItems / numGroups);
    uint remainder = numItems - (items_per_group * numGroups);
    uint startID = taskID * items_per_group  + min(taskID, remainder);
    uint endID   = startID + items_per_group + ((taskID < remainder) ? 1 : 0);

    return (uint2)(startID,endID);
}

GRL_INLINE void sort_morton_codes_bin_items_taskID_func(global struct Globals* globals,
                                                 global uint* global_histogram,
                                                 global uchar* input,
                                                 local uint* histogram,
                                                 uint iteration,
                                                 uint numGroups,
                                                 uint numItems,
                                                 bool shift_primID,
                                                 uint taskID,
                                                 uint startID,
                                                 uint endID)
{
    const uint shift = globals->shift;

    for (uint i = get_local_id(0); i < RADIX_BINS; i += get_local_size(0))
        histogram[i] = 0;

    barrier(CLK_LOCAL_MEM_FENCE);

    if (shift_primID)
    {
        for (uint i = startID + get_local_id(0); i < endID; i += get_local_size(0))
        {
            // Read input as ulong to make bitshift, so the bits representing primID are not being
            // taken into account during sorting, which would result in smaller sort loops for
            // cases where morton shift are bigger than 8 bits
            ulong* ptr_ul = (ulong*)&input[8 * i];
            ulong code = *ptr_ul;
            uchar* ptr = (uchar*)&code;
            code >>= shift;

            uchar bin = ptr[iteration];
            atomic_inc_local(&histogram[bin]);
        }
    }
    else
    {
        for (uint i = startID + get_local_id(0); i < endID; i += get_local_size(0))
        {
            uchar bin = input[8 * i + iteration];
            atomic_inc_local(&histogram[bin]);
        }
    }

    barrier(CLK_LOCAL_MEM_FENCE);

    for (uint i = get_local_id(0); i < RADIX_BINS; i += get_local_size(0))
        global_histogram[RADIX_BINS * taskID + i] = histogram[i];
}

GRL_INLINE void sort_morton_codes_bin_items_func(global struct Globals* globals,
    global uint* global_histogram,
    global uint* wg_flags,
    global uchar* input,
    local uint* histogram,
    uint iteration,
    uint numGroups,
    uint numItems,
    bool shift_primID,
    bool update_wg_flags)
{
    if (shift_primID)
    {
        // This check is present in other LSB sort functions as well, its purpose is
        // to skip first n iterations where n is the difference between max iterations
        // and actually needed iterations to sort without primIDs
        const uint req_iterations = globals->sort_iterations;
        if (iteration < req_iterations)
            return;

        // iteration needs to be adjusted to reflect the skipped cycles
        iteration -= req_iterations;
    }

    const uint taskID = get_group_id(0);

    if (taskID == 0 && update_wg_flags)
    {
        for (uint i = get_local_id(0); i < RADIX_BINS; i += get_local_size(0))
            wg_flags[i] = 0;
    }

    uint2 ids = get_thread_range(numItems, numGroups, taskID);
    uint startID = ids.x;
    uint endID = ids.y;

    sort_morton_codes_bin_items_taskID_func(globals, global_histogram, input, histogram, iteration, numGroups, numItems, shift_primID,
                                            taskID, startID, endID);
}

__attribute__((reqd_work_group_size(512, 1, 1)))
void kernel
sort_morton_codes_bin_items(
    global struct Globals* globals,
    global uint* global_histogram,
    global uint* wg_flags,
    global uchar* input,
    uint iteration,
    uint numGroups,
    uint update_wg_flags
)
{
    local uint histogram[RADIX_BINS];
    const uint numItems = globals->numPrimitives;
    if(numItems < MORTON_LSB_SORT_NO_SHIFT_THRESHOLD)
        sort_morton_codes_bin_items_func(globals, global_histogram, wg_flags, input, histogram, iteration, numGroups, numItems, false, update_wg_flags);
    else
        sort_morton_codes_bin_items_func(globals, global_histogram, wg_flags, input, histogram, iteration, numGroups, numItems, true, update_wg_flags);
}


GRL_INLINE void sort_morton_codes_reduce_bins_func(global struct Globals* globals,
                                                   global uint* global_histogram,
                                                   local uint* partials,
                                                   uint numTasks,
                                                   uint iteration,
                                                   bool shift_primID)
{
    const uint localID = get_local_id(0);

    if (shift_primID)
    {
        const uint req_iterations = globals->sort_iterations;
        if (iteration < req_iterations)
            return;
    }

    uint t = 0;
    for (uint j = 0; j < numTasks; j++)
    {
        const uint count = load_uint_L1C_L3C(&global_histogram[RADIX_BINS * j + localID], 0);
        store_uint_L1WB_L3WB(&global_histogram[RADIX_BINS * j + localID], 0, t);
        t += count;
    }

    // each lane now contains the number of elements in the corresponding bin
    //     prefix sum this for use in the subsequent scattering pass.
    uint global_count = t;

    partials[get_sub_group_id()] = sub_group_reduce_add(global_count);

    barrier(CLK_LOCAL_MEM_FENCE);

    uint lane = get_sub_group_local_id();
    uint p = partials[lane];
    p = (lane < get_sub_group_id()) ? p : 0;

    global_count = sub_group_reduce_add(p) + sub_group_scan_exclusive_add(global_count);

    store_uint_L1WB_L3WB(&global_histogram[RADIX_BINS * numTasks + localID], 0, global_count);
}

GRL_ANNOTATE_IGC_DO_NOT_SPILL
__attribute__((reqd_work_group_size(256, 1, 1)))
void kernel
sort_morton_codes_reduce_bins(global struct Globals* globals,
    uint numTasks,
    global uint* global_histogram,
    uint iteration)
{
    local uint partials[RADIX_BINS];
    const uint numItems = globals->numPrimitives;
    if (numItems < MORTON_LSB_SORT_NO_SHIFT_THRESHOLD)
        sort_morton_codes_reduce_bins_func(globals, global_histogram, partials, numTasks, iteration, false);
    else
        sort_morton_codes_reduce_bins_func(globals, global_histogram, partials, numTasks, iteration, true);
}


#if 1
GRL_INLINE void sort_morton_codes_scatter_items_func(
    global struct Globals* globals,
    global uint* global_histogram,
    global ulong* input,
    global ulong* output,
    local uint* local_offset,
    local uint* flags,
    uint iteration,
    uint numGroups,
    uint numItems,
    bool shift_primID,
    bool update_morton_sort_in_flight)
{
    const uint gID = get_local_id(0) + get_group_id(0) * get_local_size(0);

    const uint global_shift = globals->shift;
    const uint localID = get_local_id(0);
    const uint taskID = get_group_id(0);

    if (gID == 0 && update_morton_sort_in_flight)
        globals->morton_sort_in_flight = 0;

    uint2 ids = get_thread_range(numItems, numGroups, taskID);
    uint startID = ids.x;
    uint endID = ids.y;

    if (shift_primID)
    {
        const uint req_iterations = globals->sort_iterations;
        if (iteration < req_iterations)
            return;

        iteration -= req_iterations;
    }

    const uint shift = 8 * iteration;

    // load the global bin counts, and add each bin's global prefix
    //   to the local prefix
    {
        uint global_prefix = 0, local_prefix = 0;
        if (localID < RADIX_BINS)
        {
            local_prefix = global_histogram[RADIX_BINS * taskID + localID];
            global_prefix = global_histogram[RADIX_BINS * numGroups + localID];
            local_offset[localID] = global_prefix + local_prefix;
        }

        barrier(CLK_LOCAL_MEM_FENCE);
    }


    // move elements in WG-sized chunks.   The elements need to be moved sequentially (can't use atomics)
    //   because relative order has to be preserved for LSB radix sort to work

    // For each bin, a bit vector indicating which elements are in the bin
    for (uint block_base = startID; block_base < endID; block_base += get_local_size(0))
    {
        // initialize bit vectors
        for (uint i = 4 * localID; i < RADIX_BINS * SCATTER_WG_SIZE / 32; i += 4 * get_local_size(0))
        {
            flags[i + 0] = 0;
            flags[i + 1] = 0;
            flags[i + 2] = 0;
            flags[i + 3] = 0;
        }

        barrier(CLK_LOCAL_MEM_FENCE);

        // read sort key, determine which bin it goes into, scatter into the bit vector
        //  and pre-load the local offset
        uint ID = localID + block_base;
        ulong key = 0;
        uint bin_offset = 0;
        uint bin = 0;
        uint bin_word = localID / 32;
        uint bin_bit = 1 << (localID % 32);

        if (ID < endID)
        {
            key = input[ID];

            if (shift_primID)
                bin = ((key >> global_shift) >> shift) & (RADIX_BINS - 1);
            else
                bin = (key >> shift) & (RADIX_BINS - 1);

            atomic_add_local(&flags[(SCATTER_WG_SIZE / 32) * bin + bin_word], bin_bit);
            bin_offset = local_offset[bin];
        }

        barrier(CLK_LOCAL_MEM_FENCE);

        if (ID < endID)
        {
            // each key reads the bit-vectors for its bin,
            //    - Computes local prefix sum to determine its output location
            //    - Computes number of items added to its bin (last thread adjusts bin position)
            uint prefix = 0;
            uint count = 0;
            for (uint i = 0; i < (SCATTER_WG_SIZE / 32); i++)
            {
                uint bits = flags[(SCATTER_WG_SIZE / 32) * bin + i];
                uint bc = popcount(bits);
                uint pc = popcount(bits & (bin_bit - 1));
                prefix += (i < bin_word) ? bc : 0;
                prefix += (i == bin_word) ? pc : 0;

                count += bc;
            }

            // store the key in its proper place..
            output[prefix + bin_offset] = key;

            // last item for each bin adjusts local offset for next outer loop iteration
            if (prefix == count - 1)
                local_offset[bin] += count;
        }

        barrier(CLK_LOCAL_MEM_FENCE);

    }

    /* uint local_offset[RADIX_BINS];   */
    /* uint offset_global = 0; */
    /* for (int i=0;i<RADIX_BINS;i++) */
    /*   { */
    /*     const uint count_global = global_histogram[RADIX_BINS*numTasks+i]; */
    /*     const uint offset_local  = global_histogram[RADIX_BINS*taskID+i]; */
    /*     local_offset[i] = offset_global + offset_local; */
    /*     offset_global += count_global; */
    /*   } */

    /* for (uint ID=startID;ID<endID;ID++) */
    /* { */
    /*   const uint bin = (input[ID] >> shift) & (RADIX_BINS-1); */
    /*   const uint offset = local_offset[bin]; */
    /*   output[offset] = input[ID]; */
    /*   local_offset[bin]++; */
    /* } */
}

#else

GRL_ANNOTATE_IGC_DO_NOT_SPILL
__attribute__((reqd_work_group_size(16, 1, 1)))
__attribute__((intel_reqd_sub_group_size(16))) void kernel
sort_morton_codes_scatter_items(
    global struct Globals* globals,
    uint shift,
    global uint* global_histogram,
    global char* input0,
    global char* input1,
    unsigned int input0_offset,
    unsigned int input1_offset,
    uint iteration)
{
    const uint numItems = globals->numPrimitives;
    const uint local_size = get_local_size(0);
    const uint taskID = get_group_id(0);
    const uint numTasks = get_num_groups(0);
    const uint localID = get_local_id(0);
    const uint globalID = get_local_id(0) + get_group_id(0) * get_local_size(0);
    const uint subgroupLocalID = get_sub_group_local_id();
    const uint subgroup_size = get_sub_group_size();

    const uint startID = (taskID + 0) * numItems / numTasks;
    const uint endID = (taskID + 1) * numItems / numTasks;

    global ulong* input = (global ulong*)((iteration % 2) == 0 ? input0 + input0_offset : input1 + input1_offset);
    global ulong* output = (global ulong*)((iteration % 2) == 0 ? input1 + input1_offset : input0 + input0_offset);

    local uint local_offset[RADIX_BINS];
    uint off = 0;
    for (int i = subgroupLocalID; i < RADIX_BINS; i += subgroup_size)
    {
        const uint count = global_histogram[RADIX_BINS * numTasks + i];
        const uint offset_task = global_histogram[RADIX_BINS * taskID + i];
        const uint sum = sub_group_reduce_add(count);
        const uint prefix_sum = sub_group_scan_exclusive_add(count);
        local_offset[i] = off + offset_task + prefix_sum;
        off += sum;
    }

    for (uint ID = startID + subgroupLocalID; ID < endID; ID += subgroup_size)
    {
        const uint bin = (input[ID] >> shift) & (RADIX_BINS - 1);
        const uint offset = atomic_add_local(&local_offset[bin], 1);
        output[offset] = input[ID];
    }

    /* uint local_offset[RADIX_BINS];   */
    /* uint offset_global = 0; */
    /* for (int i=0;i<RADIX_BINS;i++) */
    /*   { */
    /*     const uint count_global = global_histogram[RADIX_BINS*numTasks+i]; */
    /*     const uint offset_local  = global_histogram[RADIX_BINS*taskID+i]; */
    /*     local_offset[i] = offset_global + offset_local; */
    /*     offset_global += count_global; */
    /*   } */

    /* for (uint ID=startID;ID<endID;ID++) */
    /* { */
    /*   const uint bin = (input[ID] >> shift) & (RADIX_BINS-1); */
    /*   const uint offset = local_offset[bin]; */
    /*   output[offset] = input[ID]; */
    /*   local_offset[bin]++; */
    /* } */
}
#endif

#if 1
GRL_ANNOTATE_IGC_DO_NOT_SPILL
__attribute__((reqd_work_group_size(SCATTER_WG_SIZE, 1, 1)))
void kernel
sort_morton_codes_scatter_items(
    global struct Globals *globals,
    global uint *global_histogram,
    global ulong *input,
    global ulong *output,
    uint iteration,
    uint numGroups,
    uint update_morton_sort_in_flight)
{
    local uint local_offset[RADIX_BINS];
    local uint flags[RADIX_BINS*SCATTER_WG_SIZE/32];
    const uint numItems = globals->numPrimitives;
    if (numItems < MORTON_LSB_SORT_NO_SHIFT_THRESHOLD)
        sort_morton_codes_scatter_items_func(globals, global_histogram, input, output, local_offset,
                                             flags, iteration, numGroups, numItems, false, update_morton_sort_in_flight);
    else
        sort_morton_codes_scatter_items_func(globals, global_histogram, input, output, local_offset,
                                             flags, iteration, numGroups, numItems, true, update_morton_sort_in_flight);
}

#else

GRL_ANNOTATE_IGC_DO_NOT_SPILL
__attribute__((reqd_work_group_size(16, 1, 1)))
__attribute__((intel_reqd_sub_group_size(16))) void kernel
sort_morton_codes_scatter_items(
    global struct Globals *globals,
    uint shift,
    global uint *global_histogram,
    global char *input0,
    global char *input1,
    unsigned int input0_offset,
    unsigned int input1_offset,
    uint iteration)
{
    const uint numItems = globals->numPrimitives;
    const uint local_size = get_local_size(0);
    const uint taskID = get_group_id(0);
    const uint numTasks = get_num_groups(0);
    const uint localID = get_local_id(0);
    const uint globalID = get_local_id(0) + get_group_id(0)*get_local_size(0);
    const uint subgroupLocalID = get_sub_group_local_id();
    const uint subgroup_size = get_sub_group_size();

    const uint startID = (taskID + 0) * numItems / numTasks;
    const uint endID = (taskID + 1) * numItems / numTasks;

    global ulong *input = (global ulong *)((iteration % 2) == 0 ? input0 + input0_offset : input1 + input1_offset);
    global ulong *output = (global ulong *)((iteration % 2) == 0 ? input1 + input1_offset : input0 + input0_offset);

    local uint local_offset[RADIX_BINS];
    uint off = 0;
    for (int i = subgroupLocalID; i < RADIX_BINS; i += subgroup_size)
    {
        const uint count = global_histogram[RADIX_BINS * numTasks + i];
        const uint offset_task = global_histogram[RADIX_BINS * taskID + i];
        const uint sum = sub_group_reduce_add(count);
        const uint prefix_sum = sub_group_scan_exclusive_add(count);
        local_offset[i] = off + offset_task + prefix_sum;
        off += sum;
    }

    for (uint ID = startID + subgroupLocalID; ID < endID; ID += subgroup_size)
    {
        const uint bin = (input[ID] >> shift) & (RADIX_BINS - 1);
        const uint offset = atomic_add_local(&local_offset[bin], 1);
        output[offset] = input[ID];
    }

    /* uint local_offset[RADIX_BINS];   */
    /* uint offset_global = 0; */
    /* for (int i=0;i<RADIX_BINS;i++) */
    /*   { */
    /*     const uint count_global = global_histogram[RADIX_BINS*numTasks+i]; */
    /*     const uint offset_local  = global_histogram[RADIX_BINS*taskID+i]; */
    /*     local_offset[i] = offset_global + offset_local; */
    /*     offset_global += count_global; */
    /*   } */

    /* for (uint ID=startID;ID<endID;ID++) */
    /* { */
    /*   const uint bin = (input[ID] >> shift) & (RADIX_BINS-1); */
    /*   const uint offset = local_offset[bin]; */
    /*   output[offset] = input[ID]; */
    /*   local_offset[bin]++; */
    /* } */
}
#endif

GRL_ANNOTATE_IGC_DO_NOT_SPILL
__attribute__((reqd_work_group_size(512, 1, 1)))
__attribute__((intel_reqd_sub_group_size(MAX_HW_SIMD_WIDTH)))
void kernel
sort_morton_codes_merged(
    global struct Globals* globals,
    global uint* global_histogram,
    global uchar* input,
    uint iteration,
    uint numGroups
)
{
    const uint numItems = globals->numPrimitives;
    const uint taskID = get_group_id(0);
    const uint loc_id = get_local_id(0);
    const uint lane = get_sub_group_local_id();

    uint2 ids = get_thread_range(numItems, numGroups, taskID);
    uint startID = ids.x;
    uint endID = ids.y;

    local uint histogram[RADIX_BINS];
    local uint hist_tmp[RADIX_BINS];

    if (numItems < MORTON_LSB_SORT_NO_SHIFT_THRESHOLD)
    {
        sort_morton_codes_bin_items_taskID_func(globals, global_histogram, input, histogram, iteration, numGroups, numItems, false,
            taskID, startID, endID);
    }
    else
    {
        const uint req_iterations = globals->sort_iterations;
        if (iteration < req_iterations)
            return;

        iteration -= req_iterations;

        sort_morton_codes_bin_items_taskID_func(globals, global_histogram, input, histogram, iteration, numGroups, numItems, true,
            taskID, startID, endID);
    }

    uint last_group = 0;
    if (loc_id == 0)
        last_group = atomic_inc_global(&globals->morton_sort_in_flight);

    write_mem_fence(CLK_GLOBAL_MEM_FENCE);
    barrier(CLK_LOCAL_MEM_FENCE);

    last_group = work_group_broadcast(last_group, 0);

    bool isLastGroup = (loc_id < RADIX_BINS) && (last_group == numGroups - 1);

    uint global_count = 0;

    if (isLastGroup)
    {
        for (uint j = 0; j < numGroups; j++)
        {
            const uint count = (j == taskID) ? histogram[loc_id] : load_uint_L1C_L3C(&global_histogram[RADIX_BINS * j + loc_id], 0);
            store_uint_L1WB_L3WB(&global_histogram[RADIX_BINS * j + loc_id], 0, global_count);
            global_count += count;
        }

        hist_tmp[get_sub_group_id()] = (get_sub_group_id() < MAX_HW_SIMD_WIDTH) ? sub_group_reduce_add(global_count) : 0;
    }

    barrier(CLK_LOCAL_MEM_FENCE);

    if (isLastGroup)
    {
        uint p = hist_tmp[lane];
        p = (lane < get_sub_group_id()) ? p : 0;

        global_count = sub_group_reduce_add(p) + sub_group_scan_exclusive_add(global_count);

        store_uint_L1WB_L3WB(&global_histogram[RADIX_BINS * numGroups + loc_id], 0, global_count);
    }
}

#if 0
GRL_ANNOTATE_IGC_DO_NOT_SPILL
__attribute__((reqd_work_group_size(16, 1, 1)))
__attribute__((intel_reqd_sub_group_size(16))) void kernel
sort_morton_codes_bin_items(
    global struct Globals* globals,
    uint shift,
    global uint* global_histogram,
    global char* input0,
    global char* input1,
    unsigned int input0_offset,
    unsigned int input1_offset,
    uint iteration)
{
    const uint numItems = globals->numPrimitives;
    const uint local_size = get_local_size(0);
    const uint taskID = get_group_id(0);
    const uint numTasks = get_num_groups(0);
    const uint localID = get_local_id(0);
    const uint globalID = get_local_id(0) + get_group_id(0) * get_local_size(0);
    const uint subgroupLocalID = get_sub_group_local_id();
    const uint subgroup_size = get_sub_group_size();

    const uint startID = (taskID + 0) * numItems / numTasks;
    const uint endID = (taskID + 1) * numItems / numTasks;

    global ulong* input = (global ulong*)((iteration % 2) == 0 ? input0 + input0_offset : input1 + input1_offset);

#if 1
    local uint histogram[RADIX_BINS];
    for (uint i = subgroupLocalID; i < RADIX_BINS; i += subgroup_size)
        histogram[i] = 0;

    for (uint ID = startID + subgroupLocalID; ID < endID; ID += subgroup_size)
    {
        const uint bin = ((uint)(input[ID] >> (ulong)shift)) & (RADIX_BINS - 1);
        atomic_add(&histogram[bin], 1);
    }

    for (uint i = subgroupLocalID; i < RADIX_BINS; i += subgroup_size)
        global_histogram[RADIX_BINS * taskID + i] = histogram[i];

#else
    uint histogram[RADIX_BINS];
    for (int i = 0; i < RADIX_BINS; i++)
        histogram[i] = 0;

    for (uint ID = startID + subgroupLocalID; ID < endID; ID += subgroup_size)
    {
        const uint bin = ((uint)(input[ID] >> (ulong)shift)) & (RADIX_BINS - 1);
        histogram[bin]++;
    }

    for (uint i = 0; i < RADIX_BINS; i++)
    {
        const uint reduced_counter = sub_group_reduce_add(histogram[i]);
        global_histogram[RADIX_BINS * taskID + i] = reduced_counter;
    }
#endif
}

#endif

#define WG_SIZE_WIDE 256
#define SG_SIZE_SCAN 16

// Fast implementation of work_group_scan_exclusive using SLM for WG size 256 and SG size 16
GRL_INLINE uint work_group_scan_exclusive_add_opt(local uint* tmp, uint val)
{
    const uint hw_thread_in_wg_id = get_local_id(0) / SG_SIZE_SCAN;
    const uint sg_local_id = get_local_id(0) % SG_SIZE_SCAN;
    const uint NUM_HW_THREADS_IN_WG = WG_SIZE_WIDE / SG_SIZE_SCAN;

    uint acc = sub_group_scan_exclusive_add(val);
    uint acc2 = acc + val;

    tmp[hw_thread_in_wg_id] = sub_group_broadcast(acc2, SG_SIZE_SCAN - 1);
    barrier(CLK_LOCAL_MEM_FENCE);
    uint loaded_val = tmp[sg_local_id];
    uint wgs_acc = sub_group_scan_exclusive_add(loaded_val);
    uint acc_for_this_hw_thread = sub_group_broadcast(wgs_acc, hw_thread_in_wg_id);
    return acc + acc_for_this_hw_thread;
}

// Wide reduce algorithm is divided into 2 kernels:
// 1. First, partial exclusive add scans are made within each work group using SLM.
//    Then, The last work group for each histogram bin perform exclusive add scan along the bins using separate histgram_partials buffer.
//    Last work group is determined using global atomics on wg_flags buffer.
// 2. Second kernel globally adds the values from histgram_partials to the histogram buffer where partial sums are.
//    Then, last work group performs one more work_group scan and add so the histogram buffer values are adjusted with the global ones.
GRL_INLINE void sort_morton_codes_reduce_bins_wide_partial_sum_func(
    global struct Globals* globals,
    global uint* global_histogram,
    global uint* global_histogram_partials,
    global uint* wg_flags,
    local uint* exclusive_scan_tmp,
    uint numTasks,
    uint numGroups,
    uint iteration,
    bool shift_primID)
{
    if (shift_primID)
    {
        const uint req_iterations = globals->sort_iterations;
        if (iteration < req_iterations)
            return;

        iteration -= req_iterations;
    }

    const uint groupID = get_group_id(0) % RADIX_BINS;
    const uint scanGroupID = get_group_id(0) / RADIX_BINS;
    uint localID = get_local_id(0);
    uint globalID = localID + (scanGroupID * WG_SIZE_WIDE);
    const uint lastGroup = (numGroups / WG_SIZE_WIDE);
    const uint endID = min(numTasks, (uint)(scanGroupID * WG_SIZE_WIDE + WG_SIZE_WIDE)) - 1;

    uint temp = 0;
    uint last_count = 0;
    if (globalID < numTasks)
    {
        temp = global_histogram[RADIX_BINS * globalID + groupID];

        // Store the last value of the work group, it is either last element of histogram or last item in work group
        if (globalID == endID)
            last_count = temp;
    }

    uint val = work_group_scan_exclusive_add_opt(exclusive_scan_tmp, temp);

    if (globalID <= numTasks)
    {
        global_histogram[RADIX_BINS * globalID + groupID] = val;

        // Store the block sum value to separate buffer
        if (globalID == endID)
            global_histogram_partials[scanGroupID * WG_SIZE_WIDE + groupID] = val + last_count;
    }

    // Make sure that global_histogram_partials is updated in all work groups
    write_mem_fence(CLK_GLOBAL_MEM_FENCE);
    barrier(0);

    // Now, wait for the last group for each histogram bin, so we know that
    // all work groups already updated the global_histogram_partials buffer
    uint last_group = 0;
    if (localID == 0)
        last_group = atomic_inc_global(&wg_flags[groupID]);

    last_group = work_group_broadcast(last_group, 0);
    bool isLastGroup = (last_group == lastGroup - 1);

    // Each of the last groups computes the scan exclusive add for each partial sum we have
    if (isLastGroup)
    {
        uint temp1 = 0;
        if (localID < lastGroup)
            temp1 = global_histogram_partials[localID * WG_SIZE_WIDE + groupID];

        uint val2 = work_group_scan_exclusive_add_opt(exclusive_scan_tmp, temp1);

        if (localID < lastGroup)
            global_histogram_partials[localID * WG_SIZE_WIDE + groupID] = val2;
    }
}

GRL_INLINE void sort_morton_codes_reduce_bins_wide_add_reduce_func(
    global struct Globals* globals,
    global uint* global_histogram,
    global uint* global_histogram_partials,
    local uint* partials,
    uint numTasks,
    uint numGroups,
    uint iteration,
    bool shift_primID)
{
    if (shift_primID)
    {
        const uint req_iterations = globals->sort_iterations;
        if (iteration < req_iterations)
            return;

        iteration -= req_iterations;
    }

    const uint groupID = get_group_id(0) % RADIX_BINS;
    const uint scanGroupID = get_group_id(0) / RADIX_BINS;
    const uint lastGroup = (numGroups / WG_SIZE_WIDE);
    uint localID = get_local_id(0);
    uint globalID = localID + (scanGroupID * WG_SIZE_WIDE);
    const uint endID = min(numTasks, (uint)(scanGroupID * WG_SIZE_WIDE + WG_SIZE_WIDE)) - 1;

    // Add the global sums to the partials, skip the firsy scanGroupID as the first add
    // value is 0 in case of exclusive add scans
    if (scanGroupID > 0 && globalID <= numTasks)
    {
        uint add_val = global_histogram_partials[scanGroupID * RADIX_BINS + groupID];
        atomic_add_global(&global_histogram[globalID * RADIX_BINS + groupID], add_val);
    }

    // Wait for the last group
    uint last_group = 0;
    if (localID == 0)
        last_group = atomic_inc_global(&globals->morton_sort_in_flight);

    last_group = work_group_broadcast(last_group, 0);
    bool isLastGroup = (last_group == numGroups - 1);

    // Do the exclusive scan within all bins with global data now
    if (isLastGroup)
    {
        mem_fence_gpu_invalidate();

        uint global_count = global_histogram[numTasks * RADIX_BINS + localID];

        partials[get_sub_group_id()] = sub_group_reduce_add(global_count);

        barrier(CLK_LOCAL_MEM_FENCE);

        uint lane = get_sub_group_local_id();
        uint p = partials[lane];
        p = (lane < get_sub_group_id()) ? p : 0;

        global_count = sub_group_reduce_add(p) + sub_group_scan_exclusive_add(global_count);

        store_uint_L1WB_L3WB(&global_histogram[numTasks * RADIX_BINS + localID], 0, global_count);
    }
}


GRL_ANNOTATE_IGC_DO_NOT_SPILL
__attribute__((reqd_work_group_size(WG_SIZE_WIDE, 1, 1)))
__attribute__((intel_reqd_sub_group_size(SG_SIZE_SCAN)))
void kernel
sort_morton_codes_reduce_bins_wide_partial_sum(
    global struct Globals* globals,
    uint numTasks,
    uint numGroups,
    global uint* global_histogram,
    global uint* global_histogram_partials,
    global uint* wg_flags,
    uint iteration)
{
    local uint exclusive_scan_tmp[WG_SIZE_WIDE / SG_SIZE_SCAN];

    const uint numItems = globals->numPrimitives;
    if (numItems < MORTON_LSB_SORT_NO_SHIFT_THRESHOLD)
        sort_morton_codes_reduce_bins_wide_partial_sum_func(globals, global_histogram, global_histogram_partials, wg_flags, exclusive_scan_tmp, numTasks, numGroups, iteration, false);
    else
        sort_morton_codes_reduce_bins_wide_partial_sum_func(globals, global_histogram, global_histogram_partials, wg_flags, exclusive_scan_tmp, numTasks, numGroups, iteration, true);
}

GRL_ANNOTATE_IGC_DO_NOT_SPILL
__attribute__((reqd_work_group_size(WG_SIZE_WIDE, 1, 1)))
__attribute__((intel_reqd_sub_group_size(SG_SIZE_SCAN)))
void kernel
sort_morton_codes_reduce_bins_wide_add_reduce(
    global struct Globals* globals,
    uint numTasks,
    uint numGroups,
    global uint* global_histogram,
    global uint* global_histogram_partials,
    uint iteration)
{
    local uint partials[RADIX_BINS];

    const uint numItems = globals->numPrimitives;
    if (numItems < MORTON_LSB_SORT_NO_SHIFT_THRESHOLD)
        sort_morton_codes_reduce_bins_wide_add_reduce_func(globals, global_histogram, global_histogram_partials, partials, numTasks, numGroups, iteration, false);
    else
        sort_morton_codes_reduce_bins_wide_add_reduce_func(globals, global_histogram, global_histogram_partials, partials, numTasks, numGroups, iteration, true);
}
