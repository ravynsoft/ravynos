//
// Copyright (C) 2009-2021 Intel Corporation
//
// SPDX-License-Identifier: MIT
//
//

// @file bvh_debug.cl
//
// @brief routines to do basic integrity checks
//
// Notes:
//

#include "GRLGen12.h"
#include "intrinsics.h"
#include "libs/lsc_intrinsics.h"
#include "GRLGen12IntegrityChecks.h"
#include "api_interface.h"

#define ERROR_PRINTF 0
GRL_INLINE bool commit_err(
    global uint* some_null,
    global BVHBase* bvh,
    global ERROR_INFO* err_info_slot,
    ERROR_INFO err)
{
    if (err.type != error_t_no_error) {
        uint expected = error_t_no_error;
        atomic_compare_exchange_global(&err_info_slot->type, &expected, err.type);
        if (expected == error_t_no_error)
        {
            err_info_slot->offset_in_BVH = err.offset_in_BVH;
            err_info_slot->when = err.when;
            err_info_slot->reserved = 0xAAACCAAA;
            mem_fence_evict_to_memory();
#if ERROR_PRINTF
            printf("bvh = 0x%llX, err.type = %X, err.offset_in_BVH = %d\n", bvh, err.type, err.offset_in_BVH);
#else 
			// This is to trigger PF. Note we have to write directly to memory.
            // If write would stay in L3 it won't give a PF untill this will get evicted to mem.
            store_uint_L1UC_L3UC(some_null, 0, 0x0EEE0000 + err.type);
#endif
            return true;
        }
    }
    return false;
}

__attribute__((reqd_work_group_size(16, 1, 1)))
void kernel check_tree_topology(
    global uint* some_null,
    global BVHBase* bvh,
    global ERROR_INFO* err,
    uint phase)
{
    uint globalID = get_local_id(0) + get_group_id(0) * get_local_size(0);

    if (err->type != error_t_no_error) return;

    uint dummy1, dummy2, dummy3;
    ERROR_INFO reterr =  check_tree_topology_helper(bvh, globalID, &dummy1, &dummy2, &dummy3, false);
    if (reterr.type == error_t_no_error)
    {
        reterr = check_backpointers(bvh, globalID);
    }
    if (reterr.type == error_t_no_error)
    {
        reterr = validate_atomic_update_structs(bvh, globalID);
    }
    reterr.when = phase;
    commit_err(some_null, bvh, err, reterr);
}

GRL_INLINE bool IsValid48bPtr(qword ptr)
{
    qword CANONIZED_BITS = 0xFFFFul << 48ul;
    qword canonized_part = ptr & CANONIZED_BITS;
    bool isIt = ptr != 0 && (
        canonized_part == 0 || canonized_part == CANONIZED_BITS);
    return isIt;
}

__attribute__((reqd_work_group_size(16, 1, 1)))
void kernel check_geos_before_quad_update(
    global BVHBase* bvh, //dest bvh
    global GRL_RAYTRACING_GEOMETRY_DESC* geomDesc,
    global uint* some_null,
    global ERROR_INFO* err,
    uint phase,
    uint numGeos,
    uint numThreads)
{
    uint globalID = get_local_id(0) + get_group_id(0) * get_local_size(0);

    if (err->type != error_t_no_error) return;

    // first check sanity of geos
    ERROR_INFO geo_insanity_error = { error_t_input_geo_insane, 0 };  

    for (uint ID = globalID; ID < numGeos; ID += numThreads * get_sub_group_size())
    {
        bool IsSane = IsValid48bPtr((qword)(qword)geomDesc);

        if (IsSane) {
            GRL_RAYTRACING_GEOMETRY_DESC geo = geomDesc[globalID];
            IsSane = geo.Type < NUM_GEOMETRY_TYPES;
            if (IsSane) {
                if (geo.Type == GEOMETRY_TYPE_TRIANGLES) {
                    if (geo.Desc.Triangles.IndexFormat >= INDEX_FORMAT_END) {
                        IsSane = false;
                    }
                    else
                    {
                        if (geo.Desc.Triangles.IndexFormat != INDEX_FORMAT_NONE && geo.Desc.Triangles.IndexCount > 2)
                        {
                            IsSane = (geo.Desc.Triangles.VertexFormat < VERTEX_FORMAT_END) &&
                                IsValid48bPtr((qword)geo.Desc.Triangles.pVertexBuffer) &&
                                IsValid48bPtr((qword)geo.Desc.Triangles.pIndexBuffer);
                        }   
                        else if (geo.Desc.Triangles.VertexCount > 2)
                        {
                            IsSane =
                                geo.Desc.Triangles.VertexFormat < VERTEX_FORMAT_END&&
                                IsValid48bPtr((qword)geo.Desc.Triangles.pVertexBuffer) != 0;
                        }
                    }
                }
            }
        }

        geo_insanity_error.offset_in_BVH = ID;
        geo_insanity_error.when = phase;
        if (!IsSane) {
            commit_err(some_null, bvh, err, geo_insanity_error);
        }
        return;
    }
}

__attribute__((reqd_work_group_size(16, 1, 1)))
void kernel check_geos_vs_quads(
    global BVHBase* bvh,
    global GRL_RAYTRACING_GEOMETRY_DESC* geomDesc,
    global uint* some_null,
    global ERROR_INFO* err,
    uint phase,
    uint numGeos,
    uint numThreads)
{
    uint numQuads = BVHBase_GetNumQuads(bvh);

    QuadLeaf* quads = BVHBase_GetQuadLeaves(bvh);

    uint globalID = get_local_id(0) + get_group_id(0) * get_local_size(0);
    uint qoffset = bvh->quadLeafStart;

    if (err->type != error_t_no_error) return;
    
    ERROR_INFO theErr = { error_t_no_error, 0 };
    
    for (uint ID = globalID; ID < numQuads; ID += numThreads * get_sub_group_size())
    {
        ERROR_INFO quadErr = { error_t_quad_leaf_broken, qoffset + ID, phase };
        
        QuadLeaf quad = quads[ID];

        uint geoIdx = PrimLeaf_GetGeoIndex(&quad.leafDesc);
        
        if (geoIdx > numGeos) { commit_err(some_null, bvh, err, quadErr); return; }

        uint numPrimsInGeo = geomDesc[geoIdx].Desc.Triangles.IndexFormat != INDEX_FORMAT_NONE ?
            geomDesc[geoIdx].Desc.Triangles.IndexCount  / 3 :
            geomDesc[geoIdx].Desc.Triangles.VertexCount / 3;

        if(quad.primIndex0 >= numPrimsInGeo) { 
            commit_err(some_null, bvh, err, quadErr);
            return; 
        }
        
        if(!QuadLeaf_IsSingleTriangle(&quad) && 
           (quad.primIndex0 + QuadLeaf_GetPrimIndexDelta(&quad) >= numPrimsInGeo))
        {
            commit_err(some_null, bvh, err, quadErr);
            return; 
        }
    }
}

__attribute__((reqd_work_group_size(16, 1, 1)))
void kernel check_instances_linked_bvhs(
    global uint* some_null,
    global BVHBase* bvh,
    global ERROR_INFO* err,
    uint phase)
{
    if (err->type != error_t_no_error) return;

    uint instanceLeafStart = bvh->instanceLeafStart;
    uint instanceLeafEnd = bvh->instanceLeafEnd;
    uint numInstances = (instanceLeafEnd - instanceLeafStart) / 2;

    uint globalID = get_local_id(0) + get_group_id(0) * get_local_size(0);

    ERROR_INFO reterr = check_instances_linked_bvhs_helper(bvh, globalID, /*touchBlas*/true);
    reterr.when = phase;
    commit_err(some_null, bvh, err, reterr);
}
