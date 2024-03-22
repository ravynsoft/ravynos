//
// Copyright (C) 2009-2021 Intel Corporation
//
// SPDX-License-Identifier: MIT
//
//

#include "api_interface.h"
#include "common.h"

#define GRID_SIZE 1024

/*
  This presplit item contains for each primitive a number of splits to
  perform (priority) and the primref index.
 */

struct PresplitItem
{
    unsigned int index;
    float priority;
};

/*

  This function splits a line v0->v1 at position pos in dimension dim
  and merges the bounds for the left and right line segments into
  lbounds and rbounds.

 */

GRL_INLINE void splitLine(const uint dim,
                      const float pos,
                      const float4 v0,
                      const float4 v1,
                      struct AABB *lbounds,
                      struct AABB *rbounds)
{
    const float v0d = v0[dim];
    const float v1d = v1[dim];

    /* this point is on left side */
    if (v0d <= pos)
        AABB_extend_point(lbounds, v0);

    /* this point is on right side */
    if (v0d >= pos)
        AABB_extend_point(rbounds, v0);

    /* the edge crosses the splitting location */
    if ((v0d < pos && pos < v1d) || (v1d < pos && pos < v0d))
    {
        const float f = (pos - v0d) / (v1d - v0d);
        const float4 c = f * (v1 - v0) + v0;
        AABB_extend_point(lbounds, c);
        AABB_extend_point(rbounds, c);
    }
}

/*

  This function splits a clipped triangle v0,v1,v2 with bounds prim at
  position pos in dimension dim and merges the bounds for the left and
  right clipped triangle fragments into lbounds and rbounds.

 */

GRL_INLINE void splitTriangle(struct AABB *prim,
                          const uint dim,
                          const float pos,
                          const float4 v0,
                          const float4 v1,
                          const float4 v2,
                          struct AABB *lbounds,
                          struct AABB *rbounds)
{
    /* clip each triangle edge */
    splitLine(dim, pos, v0, v1, lbounds, rbounds);
    splitLine(dim, pos, v1, v2, lbounds, rbounds);
    splitLine(dim, pos, v2, v0, lbounds, rbounds);

    /* the triangle itself was clipped already, thus clip against triangle bounds */
    AABB_intersect(lbounds, prim);
    AABB_intersect(rbounds, prim);
}

float calculate_priority(struct AABB *prim, global GRL_RAYTRACING_GEOMETRY_DESC *geom)
{
    /* calculate projected area of first triangles */
    const uint primID0 = PRIMREF_primID0(prim);
    const uint3 tri0 = GRL_load_triangle(geom, primID0);
    const float4 av0 = GRL_load_vertex(geom, tri0.x);
    const float4 av1 = GRL_load_vertex(geom, tri0.y);
    const float4 av2 = GRL_load_vertex(geom, tri0.z);
    const float area_tri0 = areaProjectedTriangle(av0, av1, av2);

    /* calculate projected area of second triangle */
    const uint primID1 = PRIMREF_primID1(prim);
    const uint3 tri1 = GRL_load_triangle(geom, primID1);
    const float4 bv0 = GRL_load_vertex(geom, tri1.x);
    const float4 bv1 = GRL_load_vertex(geom, tri1.y);
    const float4 bv2 = GRL_load_vertex(geom, tri1.z);
    const float area_tri1 = areaProjectedTriangle(bv0, bv1, bv2);

    /* as priority we use the AABB area */
    const float area_aabb = AABB_halfArea(prim);
    float priority = area_aabb;

    /* prefer triangles with a large potential SAH gain. */
    const float area_tris = area_tri0 + area_tri1;
    const float area_ratio = min(4.0f, area_aabb / max(1E-12f, area_tris));
    priority *= area_ratio;

    /* ignore too small primitives */
    //const float4 size = AABB_size(prim);
    //const float max_size = max(size.x,max(size.y,size.z));
    //if (max_size < 0.5f*max_scene_size/GRID_SIZE)
    //  priority = 0.0f;

    return priority;
}

/*

  This kernel calculates for each primitive an estimated splitting priority.

 */

 GRL_ANNOTATE_IGC_DO_NOT_SPILL
__attribute__((reqd_work_group_size(MAX_HW_SIMD_WIDTH, 1, 1))) void kernel compute_num_presplits(global struct Globals *globals,
                                                                                                 global struct BVHBase* bvh_base,
                                                                                                 global struct AABB *primref,
                                                                                                 global struct PresplitItem *presplit,
                                                                                                 global GRL_RAYTRACING_GEOMETRY_DESC *geomDesc)
{
    //assert(sizeof(PresplitItem) == sizeof_PresplitItem);

    /* calculate the range of primitives each work group should process */
    const uint numPrimitives = globals->numPrimitives;
    const uint startID = (get_group_id(0) + 0) * numPrimitives / get_num_groups(0);
    const uint endID = (get_group_id(0) + 1) * numPrimitives / get_num_groups(0);

    /* get scene bounding box size */
    const float3 scene_size = AABB3f_size(&bvh_base->Meta.bounds);
    const float max_scene_size = max(scene_size.x, max(scene_size.y, scene_size.z));

    /* each work group iterates over its range of primitives */
    for (uint i = startID + get_local_id(0); i < endID; i += get_local_size(0))
    {
        const uint geomID = PRIMREF_geomID(&primref[i]);

        /* splitting heuristic for triangles */
        if (GRL_is_triangle(&geomDesc[geomID]))
        {
            presplit[i].index = i;
            presplit[i].priority = calculate_priority(&primref[i], &geomDesc[geomID]);
        }

        /* splitting of procedurals is not supported */
        else if (GRL_is_procedural(&geomDesc[geomID]))
        {
            presplit[i].index = i;
            presplit[i].priority = 0.0f;
        }

        else
        {
            //assert(false);
        }
    }

    if (get_local_id(0) + get_group_id(0)*get_local_size(0) == 0)
        globals->numOriginalPrimitives = globals->numPrimitives;
}

/*

  This kernel computes the sum of all priorities.

 */

 GRL_ANNOTATE_IGC_DO_NOT_SPILL
__attribute__((reqd_work_group_size(MAX_WORKGROUP_SIZE, 1, 1)))
__attribute__((intel_reqd_sub_group_size(16))) void kernel
priority_sum(global struct Globals *globals,
             global struct PresplitItem *presplit,
             uint numPrimitivesToSplit)
{
    const uint N = globals->numPrimitives;
    const uint j = get_local_id(0);
    const uint J = get_local_size(0);
    const uint BLOCKSIZE = (N + J - 1) / J;
    const uint start = min((j + 0) * BLOCKSIZE, N);
    const uint end = min((j + 1) * BLOCKSIZE, N);

    float prioritySum = 0;
    for (uint i = start; i < end; i++)
        prioritySum += presplit[i].priority;

    prioritySum = work_group_reduce_add(prioritySum);
    globals->presplitPrioritySum = prioritySum;

#if 0
  work_group_barrier(CLK_LOCAL_MEM_FENCE | CLK_GLOBAL_MEM_FENCE);

  float scale = 1.0f;
  for (uint i = 0; i < 10; i++)
  {
    //if (j == 0)
    //printf("prioritySum = %f\n",scale*prioritySum);

    uint numSplits = 0;
    for (uint i = start; i < end; i++)
      numSplits += presplit[i].priority / (scale*prioritySum)*numPrimitivesToSplit;

    numSplits = work_group_reduce_add(numSplits);

    if (numSplits > numPrimitivesToSplit)
      break;

    //if (j == 0)
    //  printf("numSplits = %i (%i)\n",numSplits,numPrimitivesToSplit);

    globals->presplitPrioritySum = scale * prioritySum;
    scale -= 0.05f;
  }
#endif
}

GRL_INLINE void heapify_down(struct AABB *array, uint size)
{
    /* we start at the root */
    uint cur_node_id = 0;
    struct AABB *cur_node = array;

    while (true)
    {
        int larger_node_id = cur_node_id;
        struct AABB *larger_node = cur_node;

        /* check if left child is largest */
        const int left_node_id = 2 * cur_node_id + 1;
        struct AABB *left_node = &array[left_node_id];
        if (left_node_id < size && AABB_halfArea(left_node) > AABB_halfArea(larger_node))
        {
            larger_node_id = left_node_id;
            larger_node = left_node;
        }

        /* check if right child is largest */
        const int right_node_id = 2 * cur_node_id + 2;
        struct AABB *right_node = &array[right_node_id];
        if (right_node_id < size && AABB_halfArea(right_node) > AABB_halfArea(larger_node))
        {
            larger_node_id = right_node_id;
            larger_node = right_node;
        }

        /* if current node is largest heap property is fulfilled and we are done */
        if (larger_node_id == cur_node_id)
            break;

        /* otherwise we swap cur and largest */
        struct AABB tmp = *cur_node;
        *cur_node = *larger_node;
        *larger_node = tmp;

        /* we continue downwards with the largest node */
        cur_node_id = larger_node_id;
        cur_node = larger_node;
    }
}

GRL_INLINE void heapify_up(struct AABB *array, uint cur_node_id)
{
    /* stop if we start at the root */
    if (cur_node_id == 0)
        return;

    struct AABB *cur_node = &array[cur_node_id];

    /* we loop until we reach the root node */
    while (cur_node_id)
    {
        /* get parent node */
        uint parent_node_id = (cur_node_id - 1) / 2;
        struct AABB *parent_node = &array[parent_node_id];

        /* if parent is larger then current we fulfill the heap property and can terminate */
        if (AABB_halfArea(parent_node) > AABB_halfArea(cur_node))
            break;

        /* otherwise we swap cur and parent */
        struct AABB tmp = *cur_node;
        *cur_node = *parent_node;
        *parent_node = tmp;

        /* and continue upwards */
        cur_node_id = parent_node_id;
        cur_node = parent_node;
    }
}

/* splits a quad primref */
GRL_INLINE void splitQuadPrimRef(global GRL_RAYTRACING_GEOMETRY_DESC *geom,
                      struct AABB *cur, uint dim, float fsplit,
                      struct AABB *left, struct AABB *right)
{
    /* left and right bounds to compute */
    AABB_init(left);
    AABB_init(right);

    /* load first triangle and split it */
    const uint primID0 = PRIMREF_primID0(cur);
    const uint3 tri0 = GRL_load_triangle(geom, primID0);
    const float4 av0 = GRL_load_vertex(geom, tri0.x);
    const float4 av1 = GRL_load_vertex(geom, tri0.y);
    const float4 av2 = GRL_load_vertex(geom, tri0.z);
    splitTriangle(cur, dim, fsplit, av0, av1, av2, left, right);

    /* load second triangle and split it */
    const uint primID1 = PRIMREF_primID1(cur);
    const uint3 tri1 = GRL_load_triangle(geom, primID1);
    const float4 bv0 = GRL_load_vertex(geom, tri1.x);
    const float4 bv1 = GRL_load_vertex(geom, tri1.y);
    const float4 bv2 = GRL_load_vertex(geom, tri1.z);
    splitTriangle(cur, dim, fsplit, bv0, bv1, bv2, left, right);

    /* copy the PrimRef payload into left and right */
    left->lower.w = cur->lower.w;
    left->upper.w = cur->upper.w;
    right->lower.w = cur->lower.w;
    right->upper.w = cur->upper.w;
}

/*

  This kernel performs the actual pre-splitting. It selects split
  locations based on an implicit octree over the scene.

 */

#define USE_HEAP 0
#define HEAP_SIZE 32u

GRL_ANNOTATE_IGC_DO_NOT_SPILL
__attribute__((reqd_work_group_size(MAX_HW_SIMD_WIDTH, 1, 1)))
//__attribute__((intel_reqd_sub_group_size(16)))
void kernel
perform_presplits(global struct Globals *globals,
                  global struct BVHBase* bvh_base,
                  global struct AABB *primref,
                  global struct PresplitItem *presplit,
                  global char *bvh_mem,
                  global GRL_RAYTRACING_GEOMETRY_DESC *geomDesc,
                  uint numPrimitivesToSplit)
{
    /* calculate the range of primitives each work group should process */
    const uint numPrimitives = globals->numPrimitives;
    int pstart = globals->numOriginalPrimitives - numPrimitivesToSplit;
    pstart = max(0, pstart);
    const uint numPrimitivesToProcess = globals->numPrimitives - pstart;
    const uint startID = (get_group_id(0) + 0) * numPrimitivesToProcess / get_num_groups(0);
    const uint endID = (get_group_id(0) + 1) * numPrimitivesToProcess / get_num_groups(0);

    /* calculates the 3D grid */
    float4 grid_base;
    grid_base.xyz = AABB3f_load_lower( &bvh_base->Meta.bounds );
    grid_base.w = 0;

    float4 grid_extend;
    grid_extend.xyz = AABB3f_size(&bvh_base->Meta.bounds);
    grid_extend.w=0;

    grid_extend = max(grid_extend.x, max(grid_extend.y, grid_extend.z));
    const float4 grid_scale = select(GRID_SIZE / grid_extend, 0.0f, grid_extend == 0.0f);
    const float inv_grid_size = 1.0f / GRID_SIZE;

    /* we have to update centroid bounds */
    struct AABB centroidBounds;
    AABB_init(&centroidBounds);

    /* initialize heap */
    struct AABB heap[HEAP_SIZE];
    uint heap_size = 0;

    /* each work group iterates over its range of primitives */
    for (uint j = startID + get_local_id(0); j < endID; j += get_local_size(0))
    {
        /* array is in ascending order */
        //const uint ID = numPrimitives-1-j;
        const uint ID = pstart + j;
        const float prob = presplit[ID].priority;
        const uint i = presplit[ID].index;
        const uint geomID = PRIMREF_geomID(&primref[i]);

        /* do not split primitives with low splitting priority */
        if (prob <= 0.0f)
            continue;

        /* we support splitting only for triangles */
        if (!GRL_is_triangle(&geomDesc[geomID]))
            continue;

        /* compute number of split primitives to produce */
        uint numSplitPrims = prob / globals->presplitPrioritySum * numPrimitivesToSplit;
        numSplitPrims = min(HEAP_SIZE, numSplitPrims);

        /* stop if not splits have to get performed */
        if (numSplitPrims <= 1)
            continue;

        /* add primref to heap */
        heap[0] = primref[i];
        heap_size = 1;
        uint heap_pos = 0;

        /* iterate until all splits are done */
        uint prims = 1;
        uint last_heap_size = heap_size;
        while (prims < numSplitPrims)
        {
            /* map the primitive bounds to the grid */
            const float4 lower = heap[heap_pos].lower;
            const float4 upper = heap[heap_pos].upper;
            const float4 glower = (lower - grid_base) * grid_scale + 0.2f;
            const float4 gupper = (upper - grid_base) * grid_scale - 0.2f;
            uint4 ilower = convert_uint4_rtz(glower);
            uint4 iupper = convert_uint4_rtz(gupper);

            /* this ignores dimensions that are empty */
            if (glower.x >= gupper.x)
                iupper.x = ilower.x;
            if (glower.y >= gupper.y)
                iupper.y = ilower.y;
            if (glower.z >= gupper.z)
                iupper.z = ilower.z;

            /* Now we compute a morton code for the lower and upper grid
       * coordinates. */
            const uint lower_code = bitInterleave3D(ilower);
            const uint upper_code = bitInterleave3D(iupper);

            /* if all bits are equal then we cannot split */
            if (lower_code == upper_code)
            {
#if !USE_HEAP
                prims++; // !!!!!!!

                heap_pos++;
                if (heap_pos == last_heap_size)
                {
                    heap_pos = 0;
                    last_heap_size = heap_size;
                }
                continue;
#else
                if (heap_size == 1)
                    break;

                const uint offset = numPrimitives + atomic_add(&globals->numSplittedPrimitives, 1);
                primref[offset] = heap[heap_pos];

                presplit[offset].index = offset;
                presplit[offset].priority = calculate_priority(&heap[heap_pos], &geomDesc[geomID]);

                heap[0] = heap[--heap_size];
                heapify_down(heap, heap_size);
                continue;
#endif
            }

            /* We find the bit position of the first differing bit from the
       * top down. This bit indicates a split position inside an
       * implicit octree. */
            const uint diff = 31 - clz(lower_code ^ upper_code);

            /* compute octree level and dimension to perform the split in */
            const uint level = diff / 3;
            const uint dim = diff % 3;

            /* now we compute the grid position of the split */
            const uint isplit = iupper[dim] & ~((1 << level) - 1);

            /* compute world space position of split */
            const float fsplit = grid_base[dim] + isplit * inv_grid_size * grid_extend[dim];

            /* split primref into left and right part */
            struct AABB left, right;
            splitQuadPrimRef(&geomDesc[geomID], &heap[heap_pos], dim, fsplit, &left, &right);
            prims++;

            /* update centroid bounds */
            AABB_extend_point(&centroidBounds, AABB_centroid2(&left));
            AABB_extend_point(&centroidBounds, AABB_centroid2(&right));

#if !USE_HEAP

            heap[heap_pos] = left;
            heap[heap_size] = right;
            heap_size++;

            heap_pos++;
            if (heap_pos == last_heap_size)
            {
                heap_pos = 0;
                last_heap_size = heap_size;
            }
#else

            /* insert left element into heap */
            heap[0] = left;
            heapify_down(heap, heap_size);

            /* insert right element into heap */
            heap[heap_size] = right;
            heapify_up(heap, heap_size);

            heap_size++;
#endif
        }

        /* copy primities to primref array */
        primref[i] = heap[0];

        presplit[ID].index = i;
        presplit[ID].priority = calculate_priority(&heap[0], &geomDesc[geomID]);

        for (uint k = 1; k < heap_size; k++)
        {
            const uint offset = numPrimitives + atomic_add(&globals->numSplittedPrimitives, 1);
            primref[offset] = heap[k];

            presplit[offset].index = offset;
            presplit[offset].priority = calculate_priority(&heap[k], &geomDesc[geomID]);
        }
    }

    /* merge centroid bounds into global bounds */
    centroidBounds = AABB_sub_group_reduce(&centroidBounds);
    if (get_sub_group_local_id() == 0)
        AABB_global_atomic_merge(&globals->centroidBounds, &centroidBounds);

    work_group_barrier(CLK_LOCAL_MEM_FENCE | CLK_GLOBAL_MEM_FENCE);

    /* update number of primitives on finish */
    if (Globals_OnFinish(globals))
    {
        globals->numPrimitives = globals->numPrimitives + globals->numSplittedPrimitives;
        globals->numSplittedPrimitives = 0;

        /* update first build record */ // FIXME: should be done in builder itself
        global struct BuildRecord *record = (global struct BuildRecord *)(bvh_mem + bvh_base->quadLeafStart*64);
        record->end = globals->numPrimitives;
    }
}
