//
// Copyright (C) 2009-2021 Intel Corporation
//
// SPDX-License-Identifier: MIT
//
//

#pragma once

#include "shared.h"
#include "affinespace.h"
#include "api_interface.h"
#include "qbvh6.h"
#include "libs/lsc_intrinsics.h"

GRL_INLINE uint32_t HwInstanceLeafPart1_getInstanceIndex(struct HwInstanceLeaf *I)
{
    return I->part1.instanceIndex;
}

GRL_INLINE void encodeDW0_HwInstanceLeafPart0(
    uint32_t shaderIndex,
    uint32_t geomMask,
    uint4 *dst)
{
    (*dst).x = (shaderIndex & ((1 << 24) - 1)) |
             (geomMask << 24);
}

GRL_INLINE void encodeDW1_HwInstanceLeafPart0(
    uint32_t instanceContributionToHitGroupIndex,
    uint32_t notProcedural,
    uint32_t geomFlags,
    uint4* dst)
{
    (*dst).y = (instanceContributionToHitGroupIndex & ((1 << 24) - 1)) |
        ((notProcedural & 1) << (24 + 5)) |
        ((geomFlags & 3) << (24 + 5 + 1));
}

GRL_INLINE void encodeDW2DW3_HwInstanceLeafPart0(
    uint64_t rootNodePtr,
    uint32_t instFlags,
    uint4* dst)
{
    uint64_t flags = instFlags;
    uint DW2 = (uint)rootNodePtr;
    uint DW3 = ((uint)(rootNodePtr >> 32ul) & 0xffff);
    DW3 |= flags << 16ull;
    (*dst).z = DW2;
    (*dst).w = DW3;
}

GRL_INLINE void HwInstanceLeafPart0_setDW0(struct HwInstanceLeaf *I,
                                       uint32_t shaderIndex,
                                       uint32_t geomMask)
{
    I->part0.DW0 =
        (shaderIndex & ((1 << 24) - 1)) |
        (geomMask << 24);
}

GRL_INLINE void HwInstanceLeafPart0_setDW1(struct HwInstanceLeaf *I,
                                       uint32_t instanceContributionToHitGroupIndex,
                                       uint32_t notProcedural,
                                       uint32_t geomFlags)
{
    I->part0.DW1 =
        (instanceContributionToHitGroupIndex & ((1 << 24) - 1)) |
        ((notProcedural & 1) << (24 + 5)) |
        ((geomFlags & 3) << (24 + 5 + 1));
}

GRL_INLINE void HwInstanceLeafPart1_setDW0DW1(struct HwInstanceLeaf *I,
                                          global char *pBvhPtr)
{
    I->part1.DW0_DW1 = ((uint64_t)pBvhPtr) & (((uint64_t)1 << 48) - 1);
}

GRL_INLINE void HwInstanceLeafPart0_setDW2DW3(struct HwInstanceLeaf *I,
                                          uint64_t rootNodePtr,
                                          uint32_t instFlags)
{
    uint64_t flags = instFlags;
    flags = flags << 48ull;
    uint64_t ptr = rootNodePtr & 0x0000ffffffffffff;
    I->part0.DW2_DW3 = ptr + flags;
}

GRL_INLINE void HwInstanceLeaf_Constructor(global struct HwInstanceLeaf* leaf,
    global const struct GRL_RAYTRACING_INSTANCE_DESC* instDesc,
    uint instanceIndex,
    uint rootNodeByteOffset,
    uint instanceMask)
{
    global uint4* InstanceLeaf_4DWparts = (global uint4*) (leaf);

    struct AffineSpace3f obj2world = AffineSpace3f_load_row_major(instDesc->Transform);

    qword accStructPtr = (qword)instDesc->AccelerationStructure;
    uint4 p1_DW0_3 = (uint4)(
        (uint)accStructPtr,
        (uint)(accStructPtr >> (uint64_t)32),
        GRL_get_instanceID(instDesc),
        instanceIndex);

    struct AffineSpace3f world2obj = AffineSpace3f_invert(obj2world);

    store_uint4_L1S_L3WB(InstanceLeaf_4DWparts, 4 + 0 /*part1 + 0DW*/, p1_DW0_3);

    uint4 p1_DW4_7 = (uint4)(
        as_uint(obj2world.l.vx.x),
        as_uint(obj2world.l.vx.y),
        as_uint(obj2world.l.vx.z),
        as_uint(obj2world.l.vy.x));

    store_uint4_L1S_L3WB(InstanceLeaf_4DWparts, 4 + 1 /*part1 + 4DW*/, p1_DW4_7);

    uint4 p1_DW8_11 = (uint4)(
        as_uint(obj2world.l.vy.y),
        as_uint(obj2world.l.vy.z),
        as_uint(obj2world.l.vz.x),
        as_uint(obj2world.l.vz.y));

    store_uint4_L1S_L3WB(InstanceLeaf_4DWparts, 4 + 2 /*part1 + 8DW*/, p1_DW8_11);


    uint4 p1_DW12_15 = (uint4)(
        as_uint(obj2world.l.vz.z),
        as_uint(world2obj.p.x),
        as_uint(world2obj.p.y),
        as_uint(world2obj.p.z));

    store_uint4_L1S_L3WB(InstanceLeaf_4DWparts, 4 + 3 /*part1 + 12DW*/, p1_DW12_15);

    
    uint hit_group_index = GRL_get_InstanceContributionToHitGroupIndex(instDesc);
    global struct BVHBase* bvh = (global struct BVHBase*)instDesc->AccelerationStructure;

    uint4 p0_DW0_3;

    encodeDW0_HwInstanceLeafPart0(
        hit_group_index,
        instanceMask,
        &p0_DW0_3);

    encodeDW1_HwInstanceLeafPart0(
        hit_group_index, // for HW instance leaf, this field is used to offset the hit-group index
        1,  // disable opaque culling.. Necessary for SW instancing.. don't-care for HW instancing
        0,
        &p0_DW0_3);

    encodeDW2DW3_HwInstanceLeafPart0(
        rootNodeByteOffset == NO_NODE_OFFSET ? 0 : ((uint64_t)bvh) + rootNodeByteOffset, // offset NO_NODE_OFFSET is for degenerated instance, put null as root pointer
        GRL_get_InstanceFlags(instDesc),
        &p0_DW0_3);

    store_uint4_L1S_L3WB(InstanceLeaf_4DWparts, 0 /*part0 + 0DW*/, p0_DW0_3);

    uint4 p0_DW4_7 = (uint4)(
        as_uint(world2obj.l.vx.x),
        as_uint(world2obj.l.vx.y),
        as_uint(world2obj.l.vx.z),
        as_uint(world2obj.l.vy.x));

    store_uint4_L1S_L3WB(InstanceLeaf_4DWparts, 1 /*part0 + 4DW*/, p0_DW4_7);

    uint4 p0_DW8_11 = (uint4)(
        as_uint(world2obj.l.vy.y),
        as_uint(world2obj.l.vy.z),
        as_uint(world2obj.l.vz.x),
        as_uint(world2obj.l.vz.y));

    store_uint4_L1S_L3WB(InstanceLeaf_4DWparts, 2 /*part0 + 8DW*/, p0_DW8_11);

    uint4 p0_DW12_15 = (uint4)(
        as_uint(world2obj.l.vz.z),
        as_uint(obj2world.p.x),
        as_uint(obj2world.p.y),
        as_uint(obj2world.p.z));

    store_uint4_L1S_L3WB(InstanceLeaf_4DWparts, 3 /*part0 + 12DW*/, p0_DW12_15);
}
