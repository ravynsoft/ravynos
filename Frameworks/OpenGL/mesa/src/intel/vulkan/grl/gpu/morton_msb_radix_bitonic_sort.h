//
// Copyright (C) 2009-2021 Intel Corporation
//
// SPDX-License-Identifier: MIT
//
//

#pragma once

#include "common.h"
#include "morton_msb_radix_bitonic_sort_shared.h"

#include "libs/lsc_intrinsics.h"

///////////////////////////////////////////////////////////////////////////////
//
// Configuration switches
//
///////////////////////////////////////////////////////////////////////////////

#define DEBUG 0
#define MERGE_BLS_WITHIN_SG 0

///////////////////////////////////////////////////////////////////////////////


#if DEBUG
#define DEBUG_CODE(A) A
#else
#define DEBUG_CODE(A)
#endif

#define BOTTOM_LEVEL_SORT_WG_SIZE 512

// this kernel is only used to put into metakernel for debug to print that the code reached that place
GRL_ANNOTATE_IGC_DO_NOT_SPILL
__attribute__((reqd_work_group_size(1, 1, 1)))
void kernel debug_print_kernel(uint variable)
{
    if(get_local_id(0) == 0)
    printf("I'm here! %d\n", variable);
}

GRL_ANNOTATE_IGC_DO_NOT_SPILL
__attribute__((reqd_work_group_size(1, 1, 1)))
void kernel check_bls_sort(global struct Globals* globals, global ulong* input)
{
    uint prims_num = globals->numPrimitives;

    printf("in check_bls_sort kernel. Values count:: %d\n", prims_num);

    ulong left = input[0];
    ulong right;
    for (int i = 0; i < prims_num - 1; i++)
    {
        right = input[i + 1];
        printf("sorted val: %llu\n", left);
        if (left > right)
        {
            printf("element %d is bigger than %d: %llu > %llu\n", i, i+1, left, right);
        }
        left = right;
    }
}

inline uint wg_scan_inclusive_add_opt(local uint* tmp, uint val, uint SG_SIZE, uint WG_SIZE)
{
    const uint hw_thread_in_wg_id = get_local_id(0) / SG_SIZE;
    const uint sg_local_id = get_local_id(0) % SG_SIZE;
    const uint NUM_HW_THREADS_IN_WG = WG_SIZE / SG_SIZE;

    uint acc = sub_group_scan_inclusive_add(val);
    if (NUM_HW_THREADS_IN_WG == 1)
    {
        return acc;
    }
    tmp[hw_thread_in_wg_id] = sub_group_broadcast(acc, SG_SIZE - 1);
    barrier(CLK_LOCAL_MEM_FENCE);

    uint loaded_val = sg_local_id < NUM_HW_THREADS_IN_WG ? tmp[sg_local_id] : 0;
    uint wgs_acc = sub_group_scan_exclusive_add(loaded_val);
    uint acc_for_this_hw_thread = sub_group_broadcast(wgs_acc, hw_thread_in_wg_id);
    // for > 256 workitems in SIMD16 we won't fit in 16 workitems per subgroup, so we need additional iteration
    // same for > 64 workitems and more in SIMD8
    uint num_iterations = (NUM_HW_THREADS_IN_WG + SG_SIZE - 1) / SG_SIZE;
    for (int i = 1; i < num_iterations; i++)
    {
        // need to add tmp[] because of "exclusive" scan, so last element misses it
        uint prev_max_sum = sub_group_broadcast(wgs_acc, SG_SIZE - 1) + tmp[(i * SG_SIZE) - 1];
        loaded_val = (sg_local_id + i * SG_SIZE) < NUM_HW_THREADS_IN_WG ? tmp[sg_local_id] : 0;
        wgs_acc = sub_group_scan_exclusive_add(loaded_val);
        wgs_acc += prev_max_sum;
        uint new_acc_for_this_hw_thread = sub_group_broadcast(wgs_acc, hw_thread_in_wg_id % SG_SIZE);
        if (hw_thread_in_wg_id >= i * SG_SIZE)
            acc_for_this_hw_thread = new_acc_for_this_hw_thread;
    }
    return acc + acc_for_this_hw_thread;
}

struct MSBDispatchArgs
{
    global struct MSBRadixContext* context;
    uint num_of_wgs; // this is the number of workgroups that was dispatched for this context
    ulong* wg_key_start; // this is where keys to process start for current workgroup
    ulong* wg_key_end;
    uint shift_bit;
};




struct MSBDispatchArgs get_msb_dispatch_args(global struct VContextScheduler* scheduler)
{
    global struct MSBDispatchQueue* queue = &scheduler->msb_queue;

    uint group = get_group_id(0);
    struct MSBDispatchRecord record;

    // TODO_OPT:  Load this entire prefix array into SLM instead of searching..
    //    Or use sub-group ops
    uint i = 0;
    while (i < queue->num_records)
    {
        uint n = queue->records[i].wgs_to_dispatch;

        if (group < n)
        {
            record = queue->records[i];
            break;
        }

        group -= n;
        i++;
    }

    uint context_id = i;
    global struct MSBRadixContext* context = &scheduler->contexts[context_id];

    // moving to ulongs to avoid uint overflow
    ulong group_id_in_dispatch = group;
    ulong start_offset = context->start_offset;
    ulong num_keys = context->num_keys;
    ulong wgs_to_dispatch = record.wgs_to_dispatch;

    struct MSBDispatchArgs args;
    args.context = context;
    args.num_of_wgs = record.wgs_to_dispatch;
    args.wg_key_start = context->keys_in + start_offset + (group_id_in_dispatch * num_keys / wgs_to_dispatch);
    args.wg_key_end = context->keys_in + start_offset + ((group_id_in_dispatch+1) * num_keys / wgs_to_dispatch);
    args.shift_bit = MSB_SHIFT_BYTE_START_OFFSET - context->iteration * MSB_BITS_PER_ITERATION;
    return args;
}




void BLSDispatchQueue_push(global struct BLSDispatchQueue* queue, struct BLSDispatchRecord* record)
{
    uint new_idx = atomic_inc_global(&queue->num_records);
    queue->records[new_idx] = *record;
    DEBUG_CODE(printf("adding bls of size: %d\n", record->count));
}




void DO_CountSort(struct BLSDispatchRecord dispatchRecord, local ulong* SLM_shared, global ulong* output)
{
    uint tid = get_local_id(0);

    global ulong* in = ((global ulong*)(dispatchRecord.keys_in)) + dispatchRecord.start_offset;

    ulong a = tid < dispatchRecord.count ? in[tid] : ULONG_MAX;

    SLM_shared[tid] = a;

    uint counter = 0;

    barrier(CLK_LOCAL_MEM_FENCE);

    ulong curr = SLM_shared[get_sub_group_local_id()];

    for (uint i = 16; i < dispatchRecord.count; i += 16)
    {
        ulong next  = SLM_shared[i + get_sub_group_local_id()];

        for (uint j = 0; j < 16; j++)
        {
            // some older drivers have bug when shuffling ulong so we process by shuffling 2x uint
            uint2 curr_as_uint2 = as_uint2(curr);
            uint2 sg_curr_as_uint2 = (uint2)(sub_group_broadcast(curr_as_uint2.x, j), sub_group_broadcast(curr_as_uint2.y, j));
            ulong c = as_ulong(sg_curr_as_uint2);
            if (c < a)
                counter++;
        }

        curr = next;
    }


    // last iter
    for (uint j = 0; j < 16; j++)
    {
        // some older drivers have bug when shuffling ulong so we process by shuffling 2x uint
        uint2 curr_as_uint2 = as_uint2(curr);
        uint2 sg_curr_as_uint2 = (uint2)(sub_group_broadcast(curr_as_uint2.x, j), sub_group_broadcast(curr_as_uint2.y, j));
        ulong c = as_ulong(sg_curr_as_uint2);
        if (c < a)
            counter++;
    }

    // save elements to its sorted positions
    if (tid < dispatchRecord.count)
        output[dispatchRecord.start_offset + counter] = a;
}

void DO_Bitonic(struct BLSDispatchRecord dispatchRecord, local ulong* SLM_shared, global ulong* output)
{
    uint lid = get_local_id(0);
    uint elements_to_sort = BOTTOM_LEVEL_SORT_THRESHOLD;
    while ((elements_to_sort >> 1) >= dispatchRecord.count && elements_to_sort >> 1 >= BOTTOM_LEVEL_SORT_WG_SIZE)
    {
        elements_to_sort >>= 1;
    }

    for (int i = 0; i < elements_to_sort / BOTTOM_LEVEL_SORT_WG_SIZE; i++)
    {
        uint tid = lid + i * BOTTOM_LEVEL_SORT_WG_SIZE;

        if (tid >= dispatchRecord.count)
            SLM_shared[tid] = ULONG_MAX;
        else
            SLM_shared[tid] = ((global ulong*)(dispatchRecord.keys_in))[dispatchRecord.start_offset + tid];
    }

    barrier(CLK_LOCAL_MEM_FENCE);

    uint k_iterations = elements_to_sort;
    while(k_iterations >> 1 >= dispatchRecord.count && k_iterations != 0)
    {
        k_iterations >>= 1;
    }

    for (unsigned int k = 2; k <= k_iterations; k *= 2)
    {
        for (unsigned int j = k / 2; j > 0; j /= 2)
        {
            // this loop is needed when we can't create big enough workgroup so we need to process multiple times
            for (uint i = 0; i < elements_to_sort / BOTTOM_LEVEL_SORT_WG_SIZE; i++)
            {
                uint tid = lid + i * BOTTOM_LEVEL_SORT_WG_SIZE;
                unsigned int ixj = tid ^ j;
                if (ixj > tid)
                {
                    if ((tid & k) == 0)
                    {
                        if (SLM_shared[tid] > SLM_shared[ixj])
                        {
                            ulong tmp = SLM_shared[tid];
                            SLM_shared[tid] = SLM_shared[ixj];
                            SLM_shared[ixj] = tmp;
                        }
                    }
                    else
                    {
                        if (SLM_shared[tid] < SLM_shared[ixj])
                        {
                            ulong tmp = SLM_shared[tid];
                            SLM_shared[tid] = SLM_shared[ixj];
                            SLM_shared[ixj] = tmp;
                        }
                    }
                }
            }

            barrier(CLK_LOCAL_MEM_FENCE);
        }
    }

    for (int i = 0; i < elements_to_sort / BOTTOM_LEVEL_SORT_WG_SIZE; i++)
    {
        uint tid = lid + i * BOTTOM_LEVEL_SORT_WG_SIZE;

        if (tid < dispatchRecord.count)
            output[dispatchRecord.start_offset + tid] = SLM_shared[tid];
    }
}




void DO_Create_Separate_BLS_Work(global struct VContextScheduler* scheduler, global struct MSBRadixContext* context, global ulong* input)
{
    uint lid = get_local_id(0);

    uint start = context->start[lid];
    uint count = context->count[lid];
    uint start_offset = context->start_offset + start;

    struct BLSDispatchRecord record;
    record.start_offset = start_offset;
    record.count = count;
    record.keys_in = context->keys_out;

    if (count == 0) // we don't have elements so don't do anything
    {
    }
    else if (count == 1) // single element so just write it out
    {
        input[start_offset] = ((global ulong*)record.keys_in)[start_offset];
    }
    else if (count <= BOTTOM_LEVEL_SORT_THRESHOLD)
    {
        BLSDispatchQueue_push((global struct BLSDispatchQueue*)scheduler->next_bls_queue, &record);
    }
}




// We try to merge small BLS into larger one within the sub_group
void DO_Create_SG_Merged_BLS_Work_Parallel(global struct VContextScheduler* scheduler, global struct MSBRadixContext* context, global ulong* input)
{
    uint lid = get_local_id(0);
    uint sid = get_sub_group_local_id();

    uint create_msb_work = context->count[lid] > BOTTOM_LEVEL_SORT_THRESHOLD ? 1 : 0;

    uint start = context->start[lid];
    uint count = context->count[lid];
    uint ctx_start_offset = context->start_offset;

    if (sid == 0 || create_msb_work) // these SIMD lanes are the begining of merged BLS
    {
        struct BLSDispatchRecord record;
        if (create_msb_work)
        {
            record.start_offset = ctx_start_offset + start + count;
            record.count = 0;
        }
        else // SIMD lane 0 case
        {
            record.start_offset = ctx_start_offset + start; 
            record.count = count;
        }

        record.keys_in = context->keys_out;

        uint loop_idx = 1;
        while (sid + loop_idx < 16) // loop over subgroup
        {
            uint _create_msb_work = intel_sub_group_shuffle_down(create_msb_work, 0u, loop_idx);
            uint _count = intel_sub_group_shuffle_down(count, 0u, loop_idx);
            uint _start = intel_sub_group_shuffle_down(start, 0u, loop_idx);

            if (_create_msb_work) // found out next MSB work, so range of merges ends
                break;

            // need to push record since nothing more will fit
            if (record.count + _count > BOTTOM_LEVEL_SORT_MERGING_THRESHOLD)
            {
                if (record.count == 1)
                {
                    input[record.start_offset] = record.keys_in[record.start_offset];
                }
                else if (record.count > 1)
                {
                    BLSDispatchQueue_push((global struct BLSDispatchQueue*)scheduler->next_bls_queue, &record);
                }
                record.start_offset = ctx_start_offset + _start;
                record.count = _count;
            }
            else
            {
                record.count += _count;
            }
            loop_idx++;
        }
        // if we have any elements left, then schedule them
        if (record.count == 1) // only one element, so just write it out
        {
            input[record.start_offset] = record.keys_in[record.start_offset];
        }
        else if (record.count > 1)
        {
            BLSDispatchQueue_push((global struct BLSDispatchQueue*)scheduler->next_bls_queue, &record);
        }
    }
}




// We try to merge small BLS into larger one within the sub_group
void DO_Create_SG_Merged_BLS_Work(global struct VContextScheduler* scheduler, global struct MSBRadixContext* context, global ulong* input)
{
    uint lid = get_local_id(0);
    uint sid = get_sub_group_local_id();

    uint create_msb_work = context->count[lid] > BOTTOM_LEVEL_SORT_THRESHOLD ? 1 : 0;

    uint start = context->start[lid];
    uint count = context->count[lid];
    uint ctx_start_offset = context->start_offset;

    if (sid == 0)
    {
        struct BLSDispatchRecord record;
        record.start_offset = ctx_start_offset + start;
        record.count = 0;
        record.keys_in = context->keys_out;

        for (int i = 0; i < 16; i++)
        {
            uint _create_msb_work = sub_group_broadcast(create_msb_work, i);
            uint _count = sub_group_broadcast(count, i);
            uint _start = sub_group_broadcast(start, i);
            if (_create_msb_work)
            {
                if (record.count == 1) // only one element, so just write it out
                {
                    input[record.start_offset] = record.keys_in[record.start_offset];
                }
                else if (record.count > 1)
                {
                    BLSDispatchQueue_push((global struct BLSDispatchQueue*)scheduler->next_bls_queue, &record);
                }
                record.start_offset = ctx_start_offset + _start + _count;
                record.count = 0;
                continue;
            }
            // need to push record since nothing more will fit
            if (record.count + _count > BOTTOM_LEVEL_SORT_MERGING_THRESHOLD)
            {
                BLSDispatchQueue_push((global struct BLSDispatchQueue*)scheduler->next_bls_queue, &record);
                record.start_offset = ctx_start_offset + _start;
                record.count = _count;
            }
            else
            {
                record.count += _count;
            }
        }
        // if we have any elements left, then schedule them
        if (record.count == 1) // only one element, so just write it out
        {
            input[record.start_offset] = record.keys_in[record.start_offset];
        }
        else if (record.count > 1)
        {
            BLSDispatchQueue_push((global struct BLSDispatchQueue*)scheduler->next_bls_queue, &record);
        }
    }
}




void DO_Create_Work(global struct VContextScheduler* scheduler, global struct MSBRadixContext* context, global ulong* input, local uint* slm_for_wg_scan, uint sg_size, uint wg_size)
{
    uint lid = get_local_id(0);

    uint iteration = context->iteration + 1;
    uint start = context->start[lid];
    uint count = context->count[lid];
    uint start_offset = context->start_offset + start;

    uint create_msb_work = count > BOTTOM_LEVEL_SORT_THRESHOLD ? 1 : 0;

#if MERGE_BLS_WITHIN_SG
    DO_Create_SG_Merged_BLS_Work_Parallel(scheduler, context, input);
#else
    DO_Create_Separate_BLS_Work(scheduler, context, input);
#endif

    uint new_entry_id = wg_scan_inclusive_add_opt(slm_for_wg_scan, create_msb_work, sg_size, wg_size);//work_group_scan_inclusive_add(create_msb_work);
    uint stack_begin_entry;
    // last workitem in wg contains number of all new entries
    if (lid == (MSB_RADIX_NUM_BINS - 1))
    {
        stack_begin_entry = atomic_add_global(&scheduler->msb_stack.num_entries, new_entry_id);
    }
    stack_begin_entry = work_group_broadcast(stack_begin_entry, (MSB_RADIX_NUM_BINS - 1));
    new_entry_id += stack_begin_entry -1;
    

    if (create_msb_work)
    {
        scheduler->msb_stack.entries[new_entry_id].start_offset = start_offset;
        scheduler->msb_stack.entries[new_entry_id].count = count;
        scheduler->msb_stack.entries[new_entry_id].iteration = iteration;
    }

    if (lid == 0) {
        DEBUG_CODE(printf("num of new bls: %d\n", scheduler->next_bls_queue->num_records));
    }
}


struct BatchedBLSDispatchEntry
{
    /////////////////////////////////////////////////////////////
    //  State data used for communication with command streamer
    //  NOTE: This part must match definition in 'msb_radix_bitonic_sort.grl'
    /////////////////////////////////////////////////////////////
    qword p_data_buffer;
    qword num_elements; // number of elements in p_data_buffer
};


GRL_ANNOTATE_IGC_DO_NOT_SPILL
__attribute__((reqd_work_group_size(BOTTOM_LEVEL_SORT_WG_SIZE, 1, 1)))
__attribute__((intel_reqd_sub_group_size(16)))
void kernel sort_morton_codes_batched_BLS_dispatch(global struct BatchedBLSDispatchEntry* bls_dispatches)
{
    uint dispatch_id = get_group_id(0);
    uint lid = get_local_id(0);

    local ulong SLM_shared[BOTTOM_LEVEL_SORT_THRESHOLD];

    struct BatchedBLSDispatchEntry dispatchArgs = bls_dispatches[dispatch_id];
    struct BLSDispatchRecord dispatchRecord;
    dispatchRecord.start_offset = 0;
    dispatchRecord.count = dispatchArgs.num_elements;
    dispatchRecord.keys_in = (ulong*)dispatchArgs.p_data_buffer;

    DEBUG_CODE(if (lid == 0) printf("running sort_morton_codes_bottom_level_single_wg for %d elements\n", dispatchRecord.count));

    if(dispatchRecord.count > 1)
        DO_Bitonic(dispatchRecord, SLM_shared, (global ulong*)dispatchRecord.keys_in);
}




GRL_ANNOTATE_IGC_DO_NOT_SPILL
__attribute__((reqd_work_group_size(BOTTOM_LEVEL_SORT_WG_SIZE, 1, 1)))
__attribute__((intel_reqd_sub_group_size(16)))
void kernel sort_morton_codes_bottom_level_single_wg(global struct Globals* globals, global ulong* input, global ulong* output)
{
    uint lid = get_local_id(0);

    DEBUG_CODE(if (lid == 0) printf("running sort_morton_codes_bottom_level_single_wg for %d elements\n", globals->numPrimitives));

    local ulong SLM_shared[BOTTOM_LEVEL_SORT_THRESHOLD];
    
    struct BLSDispatchRecord dispatchRecord;
    dispatchRecord.start_offset = 0;
    dispatchRecord.count = globals->numPrimitives;
    dispatchRecord.keys_in = (ulong*)input;

    //TODO: count or bitonic here?
    //DO_Bitonic(dispatchRecord, SLM_shared, output);
    DO_CountSort(dispatchRecord, SLM_shared, output);
}




// This kernel initializes first context to start up the whole execution
GRL_ANNOTATE_IGC_DO_NOT_SPILL
__attribute__((reqd_work_group_size(MSB_RADIX_NUM_BINS, 1, 1)))
__attribute__((intel_reqd_sub_group_size(16)))
void kernel sort_morton_codes_msb_begin(
    global struct Globals* globals,
    global struct VContextScheduler* scheduler,
    global ulong* buf0,
    global ulong* buf1)
{
    uint lid = get_local_id(0);
    uint gid = get_group_id(0);

    DEBUG_CODE(if (lid == 0)printf("running sort_morton_codes_msb_begin\n"));

    scheduler->contexts[gid].count[lid] = 0;

    if (gid == 0 && lid == 0)
    {
        global struct MSBRadixContext* context = &scheduler->contexts[lid];
        const uint num_prims = globals->numPrimitives;

        scheduler->bls_queue0.num_records = 0;
        scheduler->bls_queue1.num_records = 0;

        scheduler->curr_bls_queue = &scheduler->bls_queue1;
        scheduler->next_bls_queue = &scheduler->bls_queue0;

        context->start_offset = 0;
        context->num_wgs_in_flight = 0;
        context->num_keys = num_prims;
        context->iteration = 0;
        context->keys_in = buf0;
        context->keys_out = buf1;

        uint msb_wgs_to_dispatch = (num_prims + MSB_WG_SORT_ELEMENTS_THRESHOLD - 1) / MSB_WG_SORT_ELEMENTS_THRESHOLD;
        scheduler->msb_queue.records[0].wgs_to_dispatch = msb_wgs_to_dispatch;

        scheduler->num_wgs_msb = msb_wgs_to_dispatch;
        scheduler->num_wgs_bls = 0;
        scheduler->msb_stack.num_entries = 0;
        scheduler->msb_queue.num_records = 1;
    }
}




__attribute__((reqd_work_group_size(MSB_RADIX_NUM_VCONTEXTS, 1, 1)))
kernel void
scheduler(global struct VContextScheduler* scheduler, global ulong* buf0, global ulong* buf1)
{
    uint lid = get_local_id(0);
    
    DEBUG_CODE(if (lid == 0) printf("running sort_morton_codes_scheduler\n"));

    uint context_idx = lid;

    const uint num_of_stack_entries = scheduler->msb_stack.num_entries;

    uint msb_wgs_to_dispatch = 0;
    if (lid < num_of_stack_entries)
    {
        struct MSBStackEntry entry = scheduler->msb_stack.entries[(num_of_stack_entries-1) - lid];
        global struct MSBRadixContext* context = &scheduler->contexts[lid];
        context->start_offset = entry.start_offset;
        context->num_wgs_in_flight = 0;
        context->num_keys = entry.count;
        context->iteration = entry.iteration;
        context->keys_in = entry.iteration % 2 == 0 ? buf0 : buf1;
        context->keys_out = entry.iteration % 2 == 0 ? buf1 : buf0;

        msb_wgs_to_dispatch = (entry.count + MSB_WG_SORT_ELEMENTS_THRESHOLD - 1) / MSB_WG_SORT_ELEMENTS_THRESHOLD;
        scheduler->msb_queue.records[lid].wgs_to_dispatch = msb_wgs_to_dispatch;
    }

    msb_wgs_to_dispatch = work_group_reduce_add(msb_wgs_to_dispatch);// TODO: if compiler implementation is slow, then consider to manually write it

    if (lid == 0)
    {
        // swap queue for next iteration
        struct BLSDispatchQueue* tmp = scheduler->curr_bls_queue;
        scheduler->curr_bls_queue = scheduler->next_bls_queue;
        scheduler->next_bls_queue = tmp;

        scheduler->next_bls_queue->num_records = 0;

        scheduler->num_wgs_bls = scheduler->curr_bls_queue->num_records;
        scheduler->num_wgs_msb = msb_wgs_to_dispatch;

        if (num_of_stack_entries < MSB_RADIX_NUM_VCONTEXTS)
        {
            scheduler->msb_queue.num_records = num_of_stack_entries;
            scheduler->msb_stack.num_entries = 0;
        }
        else
        {
            scheduler->msb_queue.num_records = MSB_RADIX_NUM_VCONTEXTS;
            scheduler->msb_stack.num_entries -= MSB_RADIX_NUM_VCONTEXTS;
        }
    }

    DEBUG_CODE(if (lid == 0) printf("running sort_morton_codes_scheduler finished, to spawn %d MSB wgs in %d contexts and %d BLS wgs, MSB records on stack %d\n",
        scheduler->num_wgs_msb, scheduler->msb_queue.num_records, scheduler->num_wgs_bls, scheduler->msb_stack.num_entries));
}




// this is the lowest sub-task, which should end return sorted codes
GRL_ANNOTATE_IGC_DO_NOT_SPILL
__attribute__((reqd_work_group_size(BOTTOM_LEVEL_SORT_WG_SIZE, 1, 1)))
__attribute__((intel_reqd_sub_group_size(16)))
void kernel sort_morton_codes_bottom_level( global struct VContextScheduler* scheduler, global ulong* output)
{
    uint lid = get_local_id(0);

    DEBUG_CODE(if (get_group_id(0) == 0 && lid == 0) printf("running sort_morton_codes_bottom_level\n"));

    local struct BLSDispatchRecord l_dispatchRecord;
    if (lid == 0)
    {
        uint record_idx = get_group_id(0);
        l_dispatchRecord = scheduler->curr_bls_queue->records[record_idx];
        //l_dispatchRecord = BLSDispatchQueue_pop((global struct BLSDispatchQueue*)scheduler->curr_bls_queue);
        atomic_dec_global(&scheduler->num_wgs_bls);
    }

    barrier(CLK_LOCAL_MEM_FENCE);

    struct BLSDispatchRecord dispatchRecord = l_dispatchRecord;

    local ulong SLM_shared[BOTTOM_LEVEL_SORT_THRESHOLD];

    // right now use only bitonic sort
    // TODO: maybe implement something else
    if (1)
    {
        //DO_Bitonic(dispatchRecord, SLM_shared, output);
        DO_CountSort(dispatchRecord, SLM_shared, output);
    }
}




#define MSB_COUNT_WG_SIZE MSB_RADIX_NUM_BINS
#define MSB_COUNT_SG_SIZE 16

// count how many elements per buckets we have
GRL_ANNOTATE_IGC_DO_NOT_SPILL
__attribute__((reqd_work_group_size(MSB_COUNT_WG_SIZE, 1, 1)))
__attribute__((intel_reqd_sub_group_size(MSB_COUNT_SG_SIZE)))
void kernel sort_morton_codes_msb_count_items( global struct VContextScheduler* scheduler)
{
    uint lid = get_local_id(0);
    uint lsz = MSB_RADIX_NUM_BINS;

    DEBUG_CODE(if (lid == 0) printf("running sort_morton_codes_msb_count_items\n"));

    local uint bucket_count[MSB_RADIX_NUM_BINS];
    local uint finish_count;
    bucket_count[lid] = 0;
    if (lid == 0)
    {
        finish_count = 0;
    }

    struct MSBDispatchArgs dispatchArgs = get_msb_dispatch_args(scheduler);

    global struct MSBRadixContext* context = dispatchArgs.context;

    global ulong* key_start = (global ulong*)dispatchArgs.wg_key_start + lid;
    global ulong* key_end = (global ulong*)dispatchArgs.wg_key_end;
    uint shift_bit = dispatchArgs.shift_bit;
    uchar shift_byte = shift_bit / 8; // so we count how many uchars to shift
    barrier(CLK_LOCAL_MEM_FENCE);

    global uchar* ks = (global uchar*)key_start;
    ks += shift_byte;
    global uchar* ke = (global uchar*)key_end;
    ke += shift_byte;

    // double buffering on value loading
    if (ks < ke)
    {
        uchar bucket_id = *ks;
        ks += lsz * sizeof(ulong);

        for (global uchar* k = ks; k < ke; k += lsz * sizeof(ulong))
        {
            uchar next_bucket_id = *k;
            atomic_inc_local(&bucket_count[bucket_id]);
            bucket_id = next_bucket_id;
        }

        atomic_inc_local(&bucket_count[bucket_id]);

    }

    barrier(CLK_LOCAL_MEM_FENCE);

    //update global counters for context
    uint count = bucket_count[lid];
    if (count > 0)
        atomic_add_global(&context->count[lid], bucket_count[lid]);

    mem_fence_gpu_invalidate();
    work_group_barrier(0);

    bool final_wg = true;
    // count WGs which have reached the end
    if (dispatchArgs.num_of_wgs > 1)
    {
        if (lid == 0)
            finish_count = atomic_inc_global(&context->num_wgs_in_flight) + 1;

        barrier(CLK_LOCAL_MEM_FENCE);

        final_wg = finish_count == dispatchArgs.num_of_wgs;
    }

    local uint partial_dispatches[MSB_COUNT_WG_SIZE / MSB_COUNT_SG_SIZE];
    // if this is last wg for current dispatch, update context
    if (final_wg)
    {
        // code below does work_group_scan_exclusive_add(context->count[lid]);
        {
            uint lane_val = context->count[lid];
            uint sg_result = sub_group_scan_inclusive_add(lane_val);

            partial_dispatches[get_sub_group_id()] = sub_group_broadcast(sg_result, MSB_COUNT_SG_SIZE - 1);
            barrier(CLK_LOCAL_MEM_FENCE);

            uint slm_result = sub_group_scan_exclusive_add(partial_dispatches[get_sub_group_local_id()]);
            slm_result = sub_group_broadcast(slm_result, get_sub_group_id());
            uint result = slm_result + sg_result - lane_val;
            context->start[lid] = result;//work_group_scan_exclusive_add(context->count[lid]);
        }

        context->count[lid] = 0;
        if(lid == 0)
            context->num_wgs_in_flight = 0;
    }
}




// sort elements into appropriate buckets
GRL_ANNOTATE_IGC_DO_NOT_SPILL
__attribute__((reqd_work_group_size(MSB_RADIX_NUM_BINS, 1, 1)))
__attribute__((intel_reqd_sub_group_size(16)))
void kernel sort_morton_codes_msb_bin_items(
    global struct VContextScheduler* scheduler, global ulong* input)
{
    uint lid = get_local_id(0);
    uint lsz = get_local_size(0);

    DEBUG_CODE(if (lid == 0) printf("running sort_morton_codes_msb_bin_items\n"));

    local uint finish_count;
    if (lid == 0)
    {
        finish_count = 0;
    }

    struct MSBDispatchArgs dispatchArgs = get_msb_dispatch_args(scheduler);
    global struct MSBRadixContext* context = dispatchArgs.context;

    global ulong* key_start = (global ulong*)dispatchArgs.wg_key_start + lid;
    global ulong* key_end = (global ulong*)dispatchArgs.wg_key_end;
    uint shift_bit = dispatchArgs.shift_bit;

    barrier(CLK_LOCAL_MEM_FENCE);

    global ulong* sorted_keys = (global ulong*)context->keys_out + context->start_offset;
    
#if MSB_RADIX_NUM_BINS == MSB_WG_SORT_ELEMENTS_THRESHOLD // special case meaning that we process exactly 1 element per workitem
    // here we'll do local counting, then move to global

    local uint slm_counters[MSB_RADIX_NUM_BINS];
    slm_counters[lid] = 0;

    barrier(CLK_LOCAL_MEM_FENCE);

    uint place_in_slm_bucket;
    uint bucket_id;
    ulong val;

    bool active_lane = key_start < key_end;

    if (active_lane)
    {
        val = *key_start;

        bucket_id = (val >> (ulong)shift_bit) & (MSB_RADIX_NUM_BINS - 1);
        place_in_slm_bucket = atomic_inc_local(&slm_counters[bucket_id]);
    }

    barrier(CLK_LOCAL_MEM_FENCE);

    // override slm_counters with global counters - we don't need to override counters with 0 elements since we won't use them anyway
    if (slm_counters[lid])
        slm_counters[lid] = atomic_add_global(&context->count[lid], slm_counters[lid]);

    barrier(CLK_LOCAL_MEM_FENCE);

    uint id_in_bucket = slm_counters[bucket_id] + place_in_slm_bucket;//atomic_inc_global(&context->count[bucket_id]);

    if (active_lane)
        sorted_keys[context->start[bucket_id] + id_in_bucket] = val;
#else
    // double buffering on value loading
    if (key_start < key_end)
    {
        ulong val = *key_start;
        key_start += lsz;

        for (global ulong* k = key_start; k < key_end; k += lsz)
        {
            ulong next_val = *k;
            uint bucket_id = (val >> (ulong)shift_bit) & (MSB_RADIX_NUM_BINS - 1);
            uint id_in_bucket = atomic_inc_global(&context->count[bucket_id]);

            //printf("dec: %llu, val: %llX bucket_id: %X", *k, *k, bucket_id);
            sorted_keys[context->start[bucket_id] + id_in_bucket] = val;

            val = next_val;
        }

        uint bucket_id = (val >> (ulong)shift_bit) & (MSB_RADIX_NUM_BINS - 1);
        uint id_in_bucket = atomic_inc_global(&context->count[bucket_id]);

        sorted_keys[context->start[bucket_id] + id_in_bucket] = val;
    }
#endif

    // make sure all groups's "counters" and "starts" are visible to final workgroup
    mem_fence_gpu_invalidate();
    work_group_barrier(0);

    bool final_wg = true;
    // count WGs which have reached the end
    if (dispatchArgs.num_of_wgs > 1)
    {
        if (lid == 0)
            finish_count = atomic_inc_global(&context->num_wgs_in_flight) + 1;

        barrier(CLK_LOCAL_MEM_FENCE);

        final_wg = finish_count == dispatchArgs.num_of_wgs;
    }

    local uint slm_for_wg_funcs[MSB_COUNT_WG_SIZE / MSB_COUNT_SG_SIZE];
    // if this is last wg for current dispatch, then prepare sub-tasks
    if (final_wg)
    {
        DO_Create_Work(scheduler, context, input, slm_for_wg_funcs, 16, MSB_RADIX_NUM_BINS);

        // clear context's counters for future execution
        context->count[lid] = 0;
    }

}