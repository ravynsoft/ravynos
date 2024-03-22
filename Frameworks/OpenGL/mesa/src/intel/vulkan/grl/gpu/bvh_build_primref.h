//
// Copyright (C) 2009-2021 Intel Corporation
//
// SPDX-License-Identifier: MIT
//
//

#if 0
/*

Create primrefs from array of instance descriptors.

*/

void store_instance_primref(
    global struct BVHBase* top_bvh,
    global struct Globals* globals,
    global PrimRef* primrefs,
    bool alloc_primref,
    PrimRef new_primref )
{
    uint allocatePrimref = alloc_primref ? 1 : 0;
    uint index = 0;
    uint numAllocations = sub_group_reduce_add(allocatePrimref);

    if (get_sub_group_local_id() == 0)
    {
        index = atomic_add_global(&globals->numPrimitives, numAllocations);
    }

    index = sub_group_broadcast(index, 0);
    index = index + sub_group_scan_exclusive_add(allocatePrimref);

    if (allocatePrimref)
    {
        primrefs[index] = new_primref;
    }

    struct AABB centroidBounds;
    centroidBounds.lower = centroidBounds.upper = AABB_centroid2(&new_primref);
    struct AABB subgroup_bbox = AABB_sub_group_reduce(&new_primref);
    struct AABB subgroup_CentroidBounds = AABB_sub_group_reduce(&centroidBounds);

    if (get_sub_group_local_id() == 0)
    {
        AABB3f_atomic_merge_global_lu(&top_bvh->Meta.bounds, subgroup_bbox.lower.xyz, subgroup_bbox.upper.xyz);
        AABB_global_atomic_merge(&globals->centroidBounds, &subgroup_CentroidBounds);
    }
}



// Compute transformed blas AABB.  Returns false if instance is degenerate
bool create_instance_primref(
    PrimRef* ref_out,
    global __const struct GRL_RAYTRACING_INSTANCE_DESC* instance,
    global struct BVHBase* bvh,
    uint instanceMask,
    uint instanceIndex
    )
{
    struct AABB3f bbox;
    bool alloc_primref = false;
    uint rootNodeOffset = NO_NODE_OFFSET;
    if (bvh != 0)
    {
        alloc_primref = true;
        AABB3f AS_bounds = BVHBase_GetRootAABB(bvh);

        const bool valid_min = isfinite(AS_bounds.lower[0]) && isfinite(AS_bounds.lower[1]) && isfinite(AS_bounds.lower[2]);
        const bool valid_max = isfinite(AS_bounds.upper[0]) && isfinite(AS_bounds.upper[1]) && isfinite(AS_bounds.upper[2]);

        if (!valid_min || !valid_max || instanceMask == 0)
        {
            // degenerated instance case

            // TODO this should be under  if ( allocate backpointers )
            {
                // we have to allocate the primref because this instance can be updated to non-degenerated
                // take the origin of the instance as a bounding box.

                bbox.lower[0] = instance->Transform[3];
                bbox.lower[1] = instance->Transform[7];
                bbox.lower[2] = instance->Transform[11];
                bbox.upper[0] = instance->Transform[3];
                bbox.upper[1] = instance->Transform[7];
                bbox.upper[2] = instance->Transform[11];
                instanceMask = 0;
            }
        }
        else
        {
            rootNodeOffset = BVH_ROOT_NODE_OFFSET;
            float transformOverhead = 0.0f;
            bbox = compute_xfm_bbox(instance->Transform, BVHBase_GetRootNode(bvh), XFM_BOX_NOT_REFINED_TAKE_CLIPBOX, &AS_bounds, transformOverhead);
        }
    }

    *ref_out = PRIMREF_set_instance(AABB3f_load_lower(&bbox), AABB3f_load_upper(&bbox), instanceIndex, instanceMask, rootNodeOffset, 0);
    return alloc_primref;
}

GRL_INLINE void primrefs_from_instances(
    global struct Globals* globals,
    global struct BVHBase* top_bvh,
    global __const struct GRL_RAYTRACING_INSTANCE_DESC* instance,
    uint instanceIndex,
    global struct AABB* primrefs)
{
    bool alloc_primref = false;
    PrimRef new_primref;
    AABB_init(&new_primref);

    if (instance)
    {
        uint mask = GRL_get_InstanceMask(instance);
        global struct BVHBase* bvh = (global struct BVHBase*)instance->AccelerationStructure;
        alloc_primref = create_instance_primref(&new_primref, instance, bvh, mask, instanceIndex);
    }

    store_instance_primref(top_bvh, globals, primrefs, alloc_primref, new_primref);
}
#endif

#if 1
GRL_INLINE void primrefs_from_instances(
    global struct Globals* globals,
    global struct BVHBase* top_bvh,
    global __const struct GRL_RAYTRACING_INSTANCE_DESC* instance,
    uint instanceIndex,
    global struct AABB* primrefs,
    global GRL_RAYTRACING_AABB* procedural_aabb,
    uint allowUpdate
    )
{
    struct AABB3f bbox;
    uint allocatePrimref = 0;

    uint rootNodeOffset = NO_NODE_OFFSET;
    uint instanceMask = 0;

    bool is_procedural = (procedural_aabb != 0);

    if( instance )
    {
        instanceMask = GRL_get_InstanceMask(instance) ;
        if ( is_procedural )
        {
            // procedural instance primref
            allocatePrimref = 1;

            float3 lower = (float3)(procedural_aabb->MinX, procedural_aabb->MinY, procedural_aabb->MinZ);
            float3 upper = (float3)(procedural_aabb->MaxX, procedural_aabb->MaxY, procedural_aabb->MaxZ);

            if (instanceMask == 0 || any(lower > upper))
            {
                bbox.lower[0] = instance->Transform[3];
                bbox.lower[1] = instance->Transform[7];
                bbox.lower[2] = instance->Transform[11];
                bbox.upper[0] = instance->Transform[3];
                bbox.upper[1] = instance->Transform[7];
                bbox.upper[2] = instance->Transform[11];
                instanceMask = 0;
            }
            else
            {
                bbox = transform_aabb(lower, upper, instance->Transform);
            }
        }
        else
        {
            // HW-instance primref

            global struct BVHBase* bvh = instance ?
                (global struct BVHBase*)instance->AccelerationStructure :
                0;

            if (bvh != 0)
            {
                AABB3f AS_bounds = BVHBase_GetRootAABB(bvh);

                const bool valid_min = isfinite(AS_bounds.lower[0]) && isfinite(AS_bounds.lower[1]) && isfinite(AS_bounds.lower[2]);
                const bool valid_max = isfinite(AS_bounds.upper[0]) && isfinite(AS_bounds.upper[1]) && isfinite(AS_bounds.upper[2]);


                if (valid_min && valid_max && instanceMask != 0)
                {
                    allocatePrimref = 1;
                    rootNodeOffset = BVH_ROOT_NODE_OFFSET;
                    float transformOverhead = 0.0f;
                    bbox = compute_xfm_bbox(instance->Transform, BVHBase_GetRootNode(bvh), XFM_BOX_NOT_REFINED_TAKE_CLIPBOX, &AS_bounds, transformOverhead);
                }
                else if (allowUpdate)
                {
                    // degenerated instance case
                    // we have to allocate the primref because this instance can be updated to non-degenerated
                    // take the origin of the instance as a bounding box.
                    allocatePrimref = 1;
                    bbox.lower[0] = instance->Transform[3];
                    bbox.lower[1] = instance->Transform[7];
                    bbox.lower[2] = instance->Transform[11];
                    bbox.upper[0] = instance->Transform[3];
                    bbox.upper[1] = instance->Transform[7];
                    bbox.upper[2] = instance->Transform[11];
                    instanceMask = 0;
                }
            }
        }
    }

    uint index = 0;
    uint numAllocations = sub_group_reduce_add(allocatePrimref);

    if (get_sub_group_local_id() == 0)
    {
        index = atomic_add_global(&globals->numPrimitives, numAllocations);
    }

    index = sub_group_broadcast(index, 0);
    index = index + sub_group_scan_exclusive_add(allocatePrimref);

    struct AABB new_primref;
    struct AABB centroidBounds;
    if (allocatePrimref)
    {
        new_primref = PRIMREF_set_instance(AABB3f_load_lower(&bbox), AABB3f_load_upper(&bbox), instanceIndex, instanceMask, rootNodeOffset, is_procedural);
        primrefs[index] = new_primref;
        centroidBounds.lower = centroidBounds.upper = AABB_centroid2(&new_primref);
    }
    else
    {
        AABB_init(&new_primref);
        AABB_init(&centroidBounds);
    }


    struct AABB subgroup_bbox = AABB_sub_group_reduce(&new_primref);
    struct AABB subgroup_CentroidBounds = AABB_sub_group_reduce(&centroidBounds);

    if (get_sub_group_local_id() == 0)
    {
        AABB3f_atomic_merge_global_lu(&top_bvh->Meta.bounds, subgroup_bbox.lower.xyz, subgroup_bbox.upper.xyz);
        AABB_global_atomic_merge(&globals->centroidBounds, &subgroup_CentroidBounds);
    }
}
#endif
