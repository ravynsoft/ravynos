//
// Copyright (C) 2009-2021 Intel Corporation
//
// SPDX-License-Identifier: MIT
//
//

//
// This file is to contain structure definitions related to the Gen12 QBVH6 acceleration structures
//
//

//********************************************************************************************
//   WARNING!!!!!
// This file is shared by OpenCL and C++ source code and must be compatible.
//  There should only be C structure definitions and trivial GRL_INLINE functions here
//
//********************************************************************************************

#pragma once

#include "GRLRTASCommon.h"
#include "GRLUtilities.h"

GRL_NAMESPACE_BEGIN(GRL)
GRL_NAMESPACE_BEGIN(RTAS)
GRL_NAMESPACE_BEGIN(GEN12)

    enum_uint8(NodeType)
    {
        NODE_TYPE_MIXED = 0x0,        // identifies a mixed internal node where each child can have a different type
        NODE_TYPE_INTERNAL = 0x0,     // internal BVH node with 6 children
        NODE_TYPE_INSTANCE = 0x1,     // instance leaf
        NODE_TYPE_PROCEDURAL = 0x3,   // procedural leaf
        NODE_TYPE_QUAD = 0x4,         // quad leaf
        NODE_TYPE_INVALID = 0x7       // indicates invalid node
    };


    typedef enum PrimLeafType
    {
        TYPE_NONE = 0,

        TYPE_QUAD = 0,

        /* For a node type of NODE_TYPE_PROCEDURAL we support enabling
        * and disabling the opaque/non_opaque culling. */

        TYPE_OPACITY_CULLING_ENABLED = 0,
        TYPE_OPACITY_CULLING_DISABLED = 1
    } PrimLeafType;

    #define BVH_MAGIC_MACRO     "GEN12_RTAS_005"    //  If serialization-breaking or algorithm-breaking changes are made, increment the digits at the end
    static const char BVH_MAGIC[16] = BVH_MAGIC_MACRO;

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

        uint32_t quadLeftoversCountNewAtomicUpdate; // number of quad leftovers for new atomic update
        uint32_t quadTableSizeNewAtomicUpdate; // size of quad Table including leftovers, padded to 256
        uint32_t quadIndicesDataStart;

        uint32_t _pad[9];

        struct RTASMetaData Meta;

    } BVHBase;

    GRL_INLINE struct GeoMetaData* BVHBase_GetGeoMetaData(BVHBase* base)
    {
        return (struct GeoMetaData*)(((char*)base) + base->Meta.geoDescsStart);
    }

#ifdef __OPENCL_VERSION__
#define BVH_ROOT_NODE_OFFSET sizeof(BVHBase)
#else
#define BVH_ROOT_NODE_OFFSET sizeof(GRL::RTAS::GEN12::BVHBase)
#endif

GRL_STATIC_ASSERT( sizeof(BVHBase) == BVH_ROOT_NODE_OFFSET, "Wrong size!");
GRL_STATIC_ASSERT( (sizeof(BVHBase) % 64) == 0 , "Misaligned size!");

    typedef struct BackPointers {
    } BackPointers;

    // threshold for size of bottom treelets, note usually treelets will be 2-3x smaller than that number
    // means that no bottom treelet has more paths than this number
    #define TREELET_NUM_STARTPOINTS 1536

    // threshold under which only one treelet will be created
    #define SINGLE_TREELET_THRESHOLD 3072
    
    typedef struct LeafTableEntry {

        uint backpointer;
        uint inner_node_index;
        uint leaf_index;
    } LeafTableEntry;

    typedef struct InnerNodeTableEntry {

        uint node_index_and_numchildren; // numchildren in 3 lsbs
        uint first_child;

    } InnerNodeTableEntry;

    typedef struct QuadDataIndices
    {
        uint header_data[4];
        uint vert_idx[4];
    } QuadDataIndices;

    typedef struct RefitTreelet {
        uint32_t startpoint_offset;
        uint32_t numStartpoints;
        uint32_t numNonTrivialStartpoints;
        uint8_t  maxDepth;
        uint8_t  depthLess64; // depth from bottom at which there are less 64  paths
        uint8_t  depthLess128;// depth from bottom at which there are less 128 paths
        uint8_t  depthLess256;// depth from bottom at which there are less 256 paths
    } RefitTreelet;

    // if RefitTreelet has number of startpoints == 1
    // it should be reinterpreted as:
    typedef struct RefitTreeletTrivial {
        uint32_t theOnlyNodeIndex;
        uint32_t numStartpoints; // have to be 1 or 0
        int32_t  childrenOffsetOfTheNode; // 0th node based
        uint8_t  maxDepth;
        uint8_t  numChildrenOfTheNode;
    } RefitTreeletTrivial;

    // 5:0  - depth after you die
    // 31:6 - Index of the inner node
    typedef uint32_t StartPoint;

    struct HwInstanceLeaf;
    struct QuadLeaf;
    struct ProceduralLeaf;
    struct InternalNode;

    typedef struct HwInstanceLeaf HwInstanceLeaf;
    typedef struct InternalNode InternalNode;
    typedef struct QuadLeaf QuadLeaf;
    typedef struct ProceduralLeaf ProceduralLeaf;

    GRL_INLINE uint32_t BackPointer_GetParentIndex( uint32_t bp )
    {
        return bp >> 6;
    }
    GRL_INLINE uint32_t BackPointer_GetNumChildren( uint32_t bp )
    {
        return (bp >> 3) & (7);
    }
    GRL_INLINE uint32_t BackPointer_GetRefitCount( uint32_t bp )
    {
        return bp & 7;
    }
    GRL_INLINE bool BackPointer_IsRoot( uint32_t bp )
    {
        return (bp >> 6) == 0x03FFFFFF;
    }

    GRL_INLINE InternalNode* BVHBase_GetRootNode( const BVHBase* p )
    {
        return (InternalNode*)( ((char*)p) + BVH_ROOT_NODE_OFFSET);
    }

    GRL_INLINE AABB3f BVHBase_GetRootAABB(const BVHBase* p)
    {
        return p->Meta.bounds;
    }

    GRL_INLINE InternalNode* BVHBase_GetInternalNodes(const BVHBase* p)
    {
        return (InternalNode*)(((char*)p) + BVH_ROOT_NODE_OFFSET);
    }
    GRL_INLINE InternalNode* BVHBase_GetInternalNodesEnd(const BVHBase* p)
    {
        return (InternalNode*)(((char*)p) + (size_t)(64u * p->nodeDataCur));
    }
    GRL_INLINE uint32_t BVHBase_GetNumInternalNodes(const BVHBase* p)
    {
        return p->nodeDataCur - BVH_ROOT_NODE_OFFSET / 64;
    }


    GRL_INLINE QuadLeaf* BVHBase_GetQuadLeaves(const BVHBase* p)
    {
        return (QuadLeaf*)(((char*)p) + (size_t)(64u * p->quadLeafStart));
    }
    GRL_INLINE const QuadLeaf* BVHBase_GetQuadLeaves_End(const BVHBase* p)
    {
        return (QuadLeaf*)(((char*)p) + (size_t)(64u * p->quadLeafCur));
    }

    GRL_INLINE const ProceduralLeaf* BVHBase_GetProceduralLeaves_End(const BVHBase* p)
    {
        return (ProceduralLeaf*)(((char*)p) + (size_t)(64u * p->proceduralDataCur));
    }

    GRL_INLINE ProceduralLeaf* BVHBase_GetProceduralLeaves(const BVHBase* p)
    {
        return (ProceduralLeaf*)(((char*)p) + (size_t)(64u * p->proceduralDataStart));
    }

    GRL_INLINE HwInstanceLeaf* BVHBase_GetHWInstanceLeaves(const BVHBase* p )
    {
        char* pRTASBits = (char*)p;
        return (HwInstanceLeaf*)(pRTASBits + (size_t)(64u * p->instanceLeafStart));
    }

    GRL_INLINE HwInstanceLeaf* BVHBase_GetHWInstanceLeaves_End(const BVHBase* p )
    {
        char* pRTASBits = (char*) p;
        return (HwInstanceLeaf*)(pRTASBits + (size_t)(64u * p->instanceLeafEnd));
    }

    GRL_INLINE uint BVHBase_GetNumHWInstanceLeaves( const BVHBase* p )
    {
        return (p->instanceLeafEnd - p->instanceLeafStart) / 2;
    }

    GRL_INLINE uint* BVHBase_GetRefitStartPoints(const BVHBase* p)
    {
        return (uint32_t*)(((char*)p) + (size_t)(64u * p->refitStartPointDataStart));
    }

    GRL_INLINE uint BVHBase_GetRefitStartPointsSize(const BVHBase* p)
    {
        return 64u * (p->fatLeafTableStart - p->refitStartPointDataStart);
    }

    GRL_INLINE uint StartPoint_GetDepth(StartPoint s)
    {
        return s & ((1 << 6) - 1);
    }

    GRL_INLINE uint StartPoint_GetNodeIdx(StartPoint s)
    {
        return s >> 6;
    }

    GRL_INLINE RefitTreelet* BVHBase_GetRefitTreeletDescs(const BVHBase* p)
    {
        return (RefitTreelet*)(((char*)p) + (size_t)(64u * p->refitTreeletsDataStart));
    }

    // this is treelet count as should be executed, ie. num of bottom treelets if there are top and bottoms.
    // to get real number of all treelets including tip, the formula is 
    //    actualNumTreelets = refitTreeletCnt > 1 ? refitTreeletCnt + 1 : 1;
    GRL_INLINE uint32_t* BVHBase_GetRefitTreeletCntPtr(BVHBase* p)
    {
        return &p->refitTreeletCnt;
    }

    GRL_INLINE uint32_t BVHBase_GetRefitTreeletCnt(const BVHBase* p)
    {
        return p->refitTreeletCnt;
    }

    GRL_INLINE uint32_t BVHBase_IsSingleTreelet(const BVHBase* p)
    {
        return p->refitTreeletCnt == 1;
    }

    GRL_INLINE BackPointers* BVHBase_GetBackPointers(const BVHBase* p)
    {
        return (BackPointers*)(((char*)p) + (size_t)(64u * p->backPointerDataStart));
    }


    GRL_INLINE LeafTableEntry* BVHBase_GetFatLeafTable(const BVHBase* p)
    {
        return (LeafTableEntry*)(((char*)p) + (size_t)(64u * p->fatLeafTableStart));
    }
    GRL_INLINE InnerNodeTableEntry* BVHBase_GetInnerNodeTable(const BVHBase* p)
    {
        return (InnerNodeTableEntry*)(((char*)p) + (size_t)(64u * p->innerTableStart));
    }
    GRL_INLINE QuadDataIndices* BVHBase_GetQuadDataIndicesTable(const BVHBase* p)
    {
        return (QuadDataIndices*)(((char*)p) + (size_t)(64u * p->quadIndicesDataStart));
    }

    GRL_INLINE unsigned* InnerNode_GetBackPointer(
        BackPointers* backpointersStruct,
        uint32_t inodeOffset /*in 64B units, from the earliest Inner node*/)
    {
        uint* backpointersArray = (uint*)backpointersStruct;
        // BACKPOINTER_LAYOUT
        uint new_index = inodeOffset;                                                                              //<-layout canonical
        //uint new_index = inodeOffset*16;                                                                           //<-layout scattered
        // uint new_index = (inodeOffset & (~0xFFFF)) | (((inodeOffset & 0xFF) << 8) | ((inodeOffset & 0xFF00) >> 8));     //<-layout hashed

        return backpointersArray + new_index;
    }

    GRL_INLINE uint32_t BVHBase_GetRefitStructsDataSize(const BVHBase* p)
    {
        return 64u * (p->BVHDataEnd - p->backPointerDataStart);
    }

    GRL_INLINE uint32_t BVHBase_GetBackpointersDataSize(const BVHBase* p)
    {
        return 64u * (p->refitTreeletsDataStart - p->backPointerDataStart);
    }

    GRL_INLINE uint32_t* BVHBase_GetBVHDataEnd( const BVHBase* p )
    {
        return (uint32_t*)(((char*)p) + (size_t)(64u * p->BVHDataEnd));
    }

    GRL_INLINE bool BVHBase_HasBackPointers( const BVHBase* p )
    {
        return p->refitTreeletsDataStart > p->backPointerDataStart;
    }

    GRL_INLINE const size_t BVHBase_GetNumQuads(const BVHBase* p)
    {
        return p->quadLeafCur - p->quadLeafStart;
    }

    GRL_INLINE const size_t BVHBase_GetNumProcedurals(const BVHBase* p)
    {
        return p->proceduralDataCur - p->proceduralDataStart;
    }

    GRL_INLINE const size_t BVHBase_GetNumInstances(const BVHBase* p)
    {
        return (p->instanceLeafEnd - p->instanceLeafStart) / 2;
    }

    GRL_INLINE const size_t BVHBase_totalBytes(const BVHBase* p)
    {
        return p->BVHDataEnd * 64u;
    }



    struct HwInstanceLeaf
    {
        /* first 64 bytes accessed during traversal */
        struct Part0
        {
            //uint32_t shaderIndex : 24;
            //uint32_t geomMask : 8;
            uint32_t DW0;

            // uint32_t instanceContributionToHitGroupIndex : 24;
            // uint32_t pad0 : 8
            //
            // NOTE:  Traversal shaders are implemented by aliasing instance leaves as procedural and sending them through the procedural path
            //    For a procedural instance, bit 29 should be set to 1, to disable "opaque culling"
            //      and bits 30 and 31 must be zero.  See also the definition of the 'PrimLeafDesc' structure
            uint32_t DW1;

            //      uint64_t rootNodePtr : 48;
            //      uint64_t instFlags : 8;
            //      uint64_t pad1 : 8;
            uint64_t DW2_DW3;

            // Vec3f world2obj_vx;   // 1st row of Worl2Obj transform
            float    world2obj_vx_x;
            float    world2obj_vx_y;
            float    world2obj_vx_z;

            // Vec3f world2obj_vy;   // 2nd row of Worl2Obj transform
            float    world2obj_vy_x;
            float    world2obj_vy_y;
            float    world2obj_vy_z;

            // Vec3f world2obj_vz;   // 3rd row of Worl2Obj transform
            float    world2obj_vz_x;
            float    world2obj_vz_y;
            float    world2obj_vz_z;

            // Vec3f obj2world_p;    // translation of Obj2World transform (on purpose in fist 64 bytes)
            float    obj2world_p_x;
            float    obj2world_p_y;
            float    obj2world_p_z;
        } part0;

        /* second 64 bytes accessed during shading */
        // NOTE: Everything in this block is under SW control
        struct Part1
        {
            //      uint64_t bvhPtr : 48;
            //      uint64_t pad : 16;
            uint64_t DW0_DW1;

            uint32_t instanceID;
            uint32_t instanceIndex;

            // Vec3f world2obj_vx;   // 1st row of Worl2Obj transform
            float    obj2world_vx_x;
            float    obj2world_vx_y;
            float    obj2world_vx_z;

            // Vec3f world2obj_vy;   // 2nd row of Worl2Obj transform
            float    obj2world_vy_x;
            float    obj2world_vy_y;
            float    obj2world_vy_z;

            // Vec3f world2obj_vz;   // 3rd row of Worl2Obj transform
            float    obj2world_vz_x;
            float    obj2world_vz_y;
            float    obj2world_vz_z;

            // Vec3f obj2world_p;    // translation of Obj2World transform (on purpose in fist 64 bytes)
            float    world2obj_p_x;
            float    world2obj_p_y;
            float    world2obj_p_z;
        } part1;
    };

    __constant const uint64_t c_one = 1ul;

    GRL_INLINE uint32_t HwInstanceLeaf_GetInstanceMask( const HwInstanceLeaf* p )
    {
        return p->part0.DW0 >> 24;
    }

    GRL_INLINE uint32_t HwInstanceLeaf_GetInstanceContributionToHitGroupIndex( const HwInstanceLeaf* p )
    {
        return p->part0.DW1 & 0x00ffffff;
    }

    GRL_INLINE uint32_t HwInstanceLeaf_GetInstanceFlags( const HwInstanceLeaf* p )
    {
        return (p->part0.DW2_DW3 >> 48) & 0xff;
    }
    GRL_INLINE uint32_t HwInstanceLeaf_GetInstanceID( const HwInstanceLeaf* p )
    {
        return p->part1.instanceID;
    }

    GRL_INLINE gpuva_t HwInstanceLeaf_GetBVH( const HwInstanceLeaf* p )           { return p->part1.DW0_DW1 & ((c_one << 48) - 1); }
    GRL_INLINE gpuva_t HwInstanceLeaf_GetStartNode( const HwInstanceLeaf* p )     { return p->part0.DW2_DW3 & ((c_one << 48) - 1); }
    GRL_INLINE uint32_t HwInstanceLeaf_GetInstanceIndex( const HwInstanceLeaf* p ) { return p->part1.instanceIndex; }

    GRL_INLINE void HwInstanceLeaf_GetTransform(struct HwInstanceLeaf* p, float* transform)
    {
        transform[0]  = p->part1.obj2world_vx_x;
        transform[1]  = p->part1.obj2world_vy_x;
        transform[2]  = p->part1.obj2world_vz_x;
        transform[3]  = p->part0.obj2world_p_x;
        transform[4]  = p->part1.obj2world_vx_y;
        transform[5]  = p->part1.obj2world_vy_y;
        transform[6]  = p->part1.obj2world_vz_y;
        transform[7]  = p->part0.obj2world_p_y;
        transform[8]  = p->part1.obj2world_vx_z;
        transform[9]  = p->part1.obj2world_vy_z;
        transform[10] = p->part1.obj2world_vz_z;
        transform[11] = p->part0.obj2world_p_z;
    }

    GRL_INLINE void HwInstanceLeaf_SetBVH( HwInstanceLeaf* p, gpuva_t b ) {
        uint64_t mask = ((c_one << 48) - 1);
        uint64_t v = p->part1.DW0_DW1;
        v = (b & mask) | (v & ~mask);
        p->part1.DW0_DW1 = v;
    }
    GRL_INLINE void HwInstanceLeaf_SetStartNode( HwInstanceLeaf* p, gpuva_t b ) {
        uint64_t mask = ((c_one << 48) - 1);
        uint64_t v = p->part0.DW2_DW3;
        v = (b & mask) | (v & ~mask);
        p->part0.DW2_DW3 = v;
    }
    GRL_INLINE void HwInstanceLeaf_SetStartNodeAndInstanceFlags( HwInstanceLeaf* p,
                                                             gpuva_t root,
                                                             uint8_t flags ) {
        uint64_t mask = ((1ull << 48) - 1);
        uint64_t v = (root & mask) | ((uint64_t)(flags)<<48);
        p->part1.DW0_DW1 = v;
    }

    struct InternalNode
    {
        float lower[3];       // world space origin of quantization grid
        int32_t childOffset;  // offset to all children in 64B multiples

        uint8_t nodeType;     // the type of the node
        uint8_t pad;          // unused byte

        int8_t exp_x;         // 2^exp_x is the size of the grid in x dimension
        int8_t exp_y;         // 2^exp_y is the size of the grid in y dimension
        int8_t exp_z;         // 2^exp_z is the size of the grid in z dimension
        uint8_t nodeMask;     // mask used for ray filtering

        struct ChildData
        {
            //uint8_t blockIncr : 2; // size of child in 64 byte blocks.   Must be ==2 for instance leaves, <=2 for quad leaves.
            //uint8_t startPrim : 4; // start primitive in fat leaf mode or child type in mixed mode
            //uint8_t pad : 2; // unused bits
            uint8_t bits;
        } childData[6];

        uint8_t lower_x[6];  // the quantized lower bounds in x-dimension
        uint8_t upper_x[6];  // the quantized upper bounds in x-dimension
        uint8_t lower_y[6];  // the quantized lower bounds in y-dimension
        uint8_t upper_y[6];  // the quantized upper bounds in y-dimension
        uint8_t lower_z[6];  // the quantized lower bounds in z-dimension
        uint8_t upper_z[6];  // the quantized upper bounds in z-dimension
    };

    GRL_INLINE uint InternalNode_GetChildBlockIncr( const InternalNode* p, uint idx )
    {
        return p->childData[idx].bits & 3;
    }
    GRL_INLINE uint InternalNode_GetChildStartPrim( const InternalNode* p, uint idx )
    {
        return (p->childData[idx].bits>>2) & 0xf;
    }

    GRL_INLINE uint8_t InternalNode_GetChildType( const InternalNode* p, uint idx )
    {
        return (p->childData[idx].bits >> 2) & 0xF;
    }

    GRL_INLINE void InternalNode_SetChildType( InternalNode* p, uint idx, uint type )
    {
        uint bits = p->childData[idx].bits;
        const uint mask = (0xF << 2);
        bits = ((type << 2) & mask) | (bits & ~mask);
        p->childData[idx].bits = (uint8_t)bits;
    }

    GRL_INLINE bool InternalNode_IsChildValid( const InternalNode* p, size_t child )
    {
        bool lower = p->lower_x[child] & 0x80; // invalid nodes are indicated by setting lower_msb = 1 and upper_msb=0
        bool upper = p->upper_x[child] & 0x80;
        return !lower || upper;
    }

    GRL_INLINE AABB3f InternalNode_GetChildAABB(const InternalNode* node, size_t i)
    {
        float4 lower, upper;
        const float4 base = { node->lower[0], node->lower[1], node->lower[2], 0.0f };
        const int4 lower_i = { node->lower_x[i], node->lower_y[i], node->lower_z[i], 0 };
        const int4 upper_i = { node->upper_x[i], node->upper_y[i], node->upper_z[i], 0 };
        const int4 exp_i = { node->exp_x, node->exp_y, node->exp_z, 0 };
        lower = base + bitShiftLdexp4(convert_float4_rtn(lower_i), exp_i - 8);
        upper = base + bitShiftLdexp4(convert_float4_rtp(upper_i), exp_i - 8);
        AABB3f aabb3f = {
            { lower.x, lower.y, lower.z },
            { upper.x, upper.y, upper.z } };
        return aabb3f;
    }

    GRL_INLINE void* InternalNode_GetChildren( InternalNode* node)
    {
        return (void*)(((char*)node) + node->childOffset * 64);
    }

    typedef struct PrimLeafDesc
    {
        //uint32_t shaderIndex : 24;    // shader index used for shader record calculations
        //uint32_t geomMask : 8;        // geometry mask used for ray masking
        uint32_t shaderIndex_geomMask;

        //uint32_t geomIndex : 29;      // the geometry index specifies the n'th geometry of the scene
        //PrimLeafType type : 1;        // see above
        //GeometryFlags geomFlags : 2;  // geometry flags of this geometry
        uint32_t geomIndex_flags;
    } PrimLeafDesc;

    GRL_INLINE uint32_t PrimLeaf_GetShaderIndex( const PrimLeafDesc* p )
    {
        return p->shaderIndex_geomMask & ((1 << 24) - 1);
    }
    GRL_INLINE uint32_t PrimLeaf_GetGeoIndex( const PrimLeafDesc* p )
    {
        return p->geomIndex_flags & ((1<<29)-1);
    }
    GRL_INLINE uint32_t PrimLeaf_GetGeomFlags( const PrimLeafDesc* p )
    {
        return (p->geomIndex_flags >> 30);
    }
    GRL_INLINE uint32_t PrimLeaf_GetType(const PrimLeafDesc* p)
    {
        return (p->geomIndex_flags >> 29) & 1;
    }

    struct QuadLeaf
    {
        PrimLeafDesc leafDesc;

        uint32_t primIndex0;

        //uint32_t primIndex1Delta : 16;
        //uint32_t j0 : 2;
        //uint32_t j1 : 2;
        //uint32_t j2 : 2;
        //uint32_t last : 1; // last quad in list
        //uint32_t pad : 9;
        uint32_t DW1;

        float v[4][3];
    };

    GRL_INLINE uint32_t QuadLeaf_GetPrimIndexDelta( const QuadLeaf* p )
    {
        return p->DW1 & 0x0000ffff;
    }
    GRL_INLINE uint32_t QuadLeaf_GetPrimIndex0( const QuadLeaf* p )
    {
        return p->primIndex0;
    }
    GRL_INLINE uint32_t QuadLeaf_GetPrimIndex1( const QuadLeaf* p )
    {
        return p->primIndex0 + QuadLeaf_GetPrimIndexDelta(p);
    }
    GRL_INLINE bool QuadLeaf_IsSingleTriangle( const QuadLeaf* p )
    {
        return QuadLeaf_GetPrimIndexDelta(p) == 0;
    }
    GRL_INLINE uint32_t QuadLeaf_GetSecondTriangleIndices( const QuadLeaf* p )
    {
        return (p->DW1>>16) & 0x3f;
    }

    GRL_INLINE void QuadLeaf_SetVertices( QuadLeaf* quad, float3 v0, float3 v1, float3 v2, float3 v3 )
    {
        quad->v[0][0] = v0.x;
        quad->v[0][1] = v0.y;
        quad->v[0][2] = v0.z;
        quad->v[1][0] = v1.x;
        quad->v[1][1] = v1.y;
        quad->v[1][2] = v1.z;
        quad->v[2][0] = v2.x;
        quad->v[2][1] = v2.y;
        quad->v[2][2] = v2.z;
        quad->v[3][0] = v3.x;
        quad->v[3][1] = v3.y;
        quad->v[3][2] = v3.z;
    }


    struct ProceduralLeaf {
        PrimLeafDesc leafDesc;

        // Number of primitives + "last" bits.
        // The meaning of this section is SW-defined and flexible
        uint32_t DW1 ;
        uint32_t _primIndex[13];
    } ;

GRL_NAMESPACE_END(Gen12)
GRL_NAMESPACE_END(RTAS)
GRL_NAMESPACE_END(GRL)
