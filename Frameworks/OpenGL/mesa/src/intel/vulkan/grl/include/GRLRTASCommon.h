//
// Copyright (C) 2009-2021 Intel Corporation
//
// SPDX-License-Identifier: MIT
//
//

//
// This file is to contain structure definitions for RTAS-related meta-deta.
//   The structures here should be generic enough to apply to any acceleration structure.
//   If we ever move to KD-Trees or Octrees, this file should not need to change.
//

//********************************************************************************************
//   WARNING!!!!!
//
// This file is shared by OpenCL and C++ source code and must be a pure C header
//  There should only be C structure definitions and trivial inline functions here
//
//********************************************************************************************


#pragma once
#include "GRLIntTypes.h"

GRL_NAMESPACE_BEGIN(GRL)
GRL_NAMESPACE_BEGIN(RTAS)

    typedef struct SerializationIdentifier
    {
        uint8_t Bytes[16];
    } SerializationIdentifier;

    GRL_STATIC_ASSERT(sizeof(SerializationIdentifier) == 16, "Wrong size!");


    // Header structure for RTAS serialization.
    //    This structure is binary-compatible with the DXR and Vulkan API definitions
    typedef struct SerializationHeader
    {
        SerializationIdentifier DriverID;   // DXR 'DriverOpaqueGUID'.            Vulkan: 'driverUUID'
        SerializationIdentifier GRLID;      // DXR 'DriverOpaqueVersioningData'.  Vulkan: 'accelerationStructureUUID'

        uint64_t SerializedSizeInBytesIncludingHeader;
        uint64_t DeserializedSizeInBytes;
        uint64_t InstanceHandleCount;
    } SerializationHeader;

    GRL_STATIC_ASSERT(sizeof(SerializationHeader) == 56, "Wrong size!");

    // This structure is binary-compatible with DXR and Vulkan 'InstanceDesc' structures
    typedef struct InstanceDesc {
        float    Transform[3][4];
        uint32_t InstanceIDAndMask; // mask in 8 msbs
        uint32_t InstanceContributionToHitGroupIndexAndFlags; // flags in 8 msbs
        gpuva_t  AccelerationStructureGPUVA; // NOTE:  In GRL this is always a VA.  Vulkan CPU builds use handles here, and these may need to be translated
    } InstanceDesc;
    GRL_STATIC_ASSERT(sizeof(InstanceDesc) == 64, "Wrong size!");

    typedef struct GeoMetaData{
        uint32_t PrimitiveCount;
        uint16_t Type;
        uint16_t Flags;
    } GeoMetaData;
    GRL_STATIC_ASSERT(sizeof(GeoMetaData) == 8, "Wrong size!");

    typedef struct AABB3f {
        float lower[3];
        float upper[3];
    } AABB3f;
    GRL_STATIC_ASSERT(sizeof(AABB3f) == 24, "Wrong size!");

    enum_uint32(error_t_) {
        error_t_no_error = 0x0,
        error_t_internal_node_child_OOB = 0x1,
        error_t_leaf_node_child_OOB = 0x2,
        error_t_unrecognised_node_t = 0x4,
        error_t_mixed_node_unsupported = 0x8,
        error_t_instance_pointers_inconsistent = 0x10,
        error_t_instance_pointed_root_not_internal = 0x20,
        error_t_leaf_node_instance_child_missed_by_64B = 0x40,
        error_t_internal_node_child_cycle = 0x80,
        error_t_input_geo_insane = 0x100,
        error_t_quad_leaf_broken = 0x200,
        error_t_backpointer_not_reset = 0x400,
        error_t_backpointer_wrong_children_num = 0x500,
        error_t_backpointer_inconsitent_parent_child = 0x600,
        error_t_backpointer_root_not_root_error = 0x700,
        error_t_backpointer_OOB = 0x800,
        error_t_backpointers_buffer_too_small = 0x900,
        error_t_atomic_update_struct_fatleaf_count_oob = 0x1000,            // for this and following:
        error_t_atomic_update_struct_fatleaf_node_idx_oob = 0x2000,         // offset_in_BVH is just index in fatleaf or inner node arrays
        error_t_atomic_update_struct_fatleaf_backpointer_mismatch = 0x3000,
        error_t_atomic_update_struct_fatleaf_num_children_error = 0x4000,
        error_t_atomic_update_struct_fatleaf_children_non_leaf = 0x5000,
        error_t_atomic_update_struct_inner_count_oob = 0x6000,
        error_t_atomic_update_struct_inner_node_idx_oob = 0x7000,
        error_t_atomic_update_struct_inner_node_child_idx_error = 0x8000,
        error_t_atomic_update_struct_inner_num_children_error = 0x9000,
        error_t_atomic_update_struct_inner_children_non_internal = 0xA000,
        error_t_unknown = 1u << 31,
    };

    enum_uint32(error_phase_t) {
        error_phase_t_unknown = 0,
        error_phase_t_post_build_Morton  = 1,
        error_phase_t_post_build_Trivial = 2,
        error_phase_t_post_build_NewSAH  = 3,
        error_phase_t_post_update        = 4,
        error_phase_t_pre_update         = 5,
        error_phase_t_post_copy_op       = 6,
    };

    typedef struct ERROR_INFO {
        error_t_ type;
        uint    offset_in_BVH; //in 64B units
        error_phase_t when;
        uint reserved;
    } ERROR_INFO;

    // Meta-data common to all acceleration structures, which is needed to implement required functionality
    //  All RTAS structures must contain a struct of this type named 'Meta'
    typedef struct RTASMetaData {
        struct AABB3f bounds;

        uint32_t instanceDescsStart;  // byte offset to array of original instance_descs used for build.  Required for DXR visualization and serialization
        uint32_t instanceCount;

        uint32_t geoDescsStart;     // byte offset to array of 'GeoMetaData' matching input geos.  Required for DXR visualization
        uint32_t geoCount;

        uint64_t allocationSize;  // Size of the memory allocation containing this RTAS
                                  //  This is the size given to the app in the prebuild info when the RTAS was first created
                                  //  If RTAS was compacted, this will be the compacted size

        ERROR_INFO errors;        // only used in debug mode
    } RTASMetaData;

    GRL_STATIC_ASSERT( sizeof(RTASMetaData) == 64, "Wrong size!");

GRL_NAMESPACE_END(RTAS)
GRL_NAMESPACE_END(GRL)
