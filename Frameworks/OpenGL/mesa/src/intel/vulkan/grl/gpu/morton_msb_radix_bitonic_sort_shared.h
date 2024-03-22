//
// Copyright (C) 2009-2021 Intel Corporation
//
// SPDX-License-Identifier: MIT
//
//

//
//   This file contains structure definitions shared by GRL OCL kernels and host code
//

#pragma once

#include "GRLGen12.h"

// NOTE:
// MSB(Most significant byte) - here I refer to it as a part of sorting that does MSB Radix sort, which can spawn additional work
// BLS(Bottom level sort) - here I refer to it as a last part of sorting a particular range(currently Bitonic), which cannot spawn additional work
//

#define MSB_RADIX_NUM_BINS    256
#define MSB_BITS_PER_ITERATION 8 // how many bits are sorted per iteration
#define MSB_SHIFT_BYTE_START_OFFSET 56 // start offset for byte shifting, first iteration will start from here

#define MSB_RADIX_NUM_VCONTEXTS 8 // NOTE: mkulikow: maybe expand/shrink? More means more MSB processed in parallel but more memory used

#define MSB_STACK_ENTRIES_NUM (MSB_RADIX_NUM_VCONTEXTS * MSB_RADIX_NUM_BINS * 7) // first level doesn't get spawned, so 7 iterations must fit here,
// since at max one algorithm iteration can spawn MSB_RADIX_NUM_VCONTEXTS * MSB_RADIX_NUM_BINS we need 7 of these

#define MSB_DISPATCH_QUEUE_NUM_RECORDS (MSB_RADIX_NUM_VCONTEXTS) // one per context

#define BLS_DISPATCH_QUEUE_NUM_RECORDS (MSB_RADIX_NUM_VCONTEXTS * MSB_RADIX_NUM_BINS) // each context can spawn MSB_RADIX_NUM_BINS,
// so at max one algorithm iteration can spawn MSB_RADIX_NUM_VCONTEXTS * MSB_RADIX_NUM_BINS

#define MSB_WG_SORT_ELEMENTS_THRESHOLD 256 // This tells us how many elements at max we can process in a single workgroup.
                                           // If a single MSB entry needs more, then it will spawn more WGs
                                           // after updating this also needs to update msb_radix_bitonic_sort.grl's computation of initial workgroups num

#define BOTTOM_LEVEL_SORT_THRESHOLD 512 // TODO: is 4096 best value? ON skl gives best performance
// Right now we use 256 workitems in simd16 which give us 16 hw threads, assuming 2KB per thread, we have 32KB SLM to play with.
// Since we use ulong(8bytes) we can store 4096 elements
// This also tells us that if number of elements to sort is less than this, we don't need to allocate scheduler
// Need to keep in sync with the GRL const BOTTOM_LEVEL_SORT_THRESHOLD

#define BOTTOM_LEVEL_SORT_MERGING_THRESHOLD 512 // This is the amount till which we'll merge small BLS'es produced by MSB into a single bigger BLS

GRL_NAMESPACE_BEGIN(GRL)




GRL_NAMESPACE_BEGIN(RTAS)
GRL_NAMESPACE_BEGIN(MORTON_MSB_RADIX_BITONIC_SORT)

struct MSBStackEntry
{
    uint start_offset;
    uint count;
    uint iteration;
};

struct MSBStack
{
    dword num_entries;
    struct MSBStackEntry entries[MSB_STACK_ENTRIES_NUM];
};

struct MSBRadixContext
{
    uint start[MSB_RADIX_NUM_BINS];
    uint count[MSB_RADIX_NUM_BINS];
    uint num_wgs_in_flight; // this is used to identify which msb wg is last
    uint num_keys; // number of keys to process
    uint iteration;
    ulong* keys_in;
    ulong* keys_out;

    uint start_offset; //offset from the beginning of the buffer
};

struct MSBDispatchRecord
{
    uint wgs_to_dispatch; // amount of workgroups to dispatch for this current record
};

struct MSBDispatchQueue
{
    dword num_records;
    struct MSBDispatchRecord records[MSB_RADIX_NUM_VCONTEXTS]; // each context have its own record
};

// BLS(Bottom Level Sort) - last stage of sorting which will not spawn any new tasks
struct BLSDispatchRecord
{
    uint start_offset; // offset from the beginning of the buffer
    uint count;
    ulong* keys_in; // we don't need keys_out since we will write always to the same output buffer 
};

struct BLSDispatchQueue
{
    dword num_records;
    struct BLSDispatchRecord records[BLS_DISPATCH_QUEUE_NUM_RECORDS];
};

struct VContextScheduler
{
    /////////////////////////////////////////////////////////////
    //  State data used for communication with command streamer
    //  NOTE: This part must match definition in 'msb_radix_bitonic_sort.grl'
    /////////////////////////////////////////////////////////////

    dword num_wgs_msb; // number of MSB workgroups being processed by current iteration
    dword num_wgs_bls; // number of BLS workgroups being processed by current iteration

    dword scheduler_postsync;
    dword _pad1;

    /////////////////////////////////////////////////////////////

    struct MSBDispatchQueue msb_queue;
    struct BLSDispatchQueue bls_queue0;
    struct BLSDispatchQueue bls_queue1;

    struct BLSDispatchQueue* curr_bls_queue;
    struct BLSDispatchQueue* next_bls_queue;

    struct MSBStack msb_stack;

    struct MSBRadixContext contexts[MSB_RADIX_NUM_VCONTEXTS];
};

GRL_NAMESPACE_END(MORTON_MSB_RADIX_BITONIC_SORT)
GRL_NAMESPACE_END(RTAS)
GRL_NAMESPACE_END(GRL)
