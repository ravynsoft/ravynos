//
// Copyright (C) 2009-2022 Intel Corporation
//
// SPDX-License-Identifier: MIT
//
//

#include "libs/lsc_intrinsics.h"
#include "morton/morton_common.h"

// caution rec.local_parent_index__numItems needs to have high 16bits filled afterwards;
BuildRecordLocalMortonFlattener TranslateToLocalRecord(struct BinaryMortonCodeHierarchy srcRec)
{
    BuildRecordLocalMortonFlattener rec;
    rec.leftChild  = srcRec.leftChild;
    rec.rightChild = srcRec.rightChild;
    rec.rangeStart = srcRec.range.start;
    rec.local_parent_index__numItems = (srcRec.range.end - srcRec.range.start) + 1;
    return rec;
}

GRL_INLINE BuildRecordLocalMortonFlattener MortonFlattenedBoxlessNode_reinterpret_as_BR(MortonFlattenedBoxlessNode boxless)
{
    BuildRecordLocalMortonFlattener rec;
    rec.leftChild = boxless.binary_hierarchy_index;
    rec.rightChild = boxless.childOffset_type;
    rec.rangeStart = boxless.backPointer;
    rec.local_parent_index__numItems = 0;
    return rec;
}

GRL_INLINE void SUBGROUP_create_boxless_node_phase1(
    uniform global struct Globals* globals,
    uniform global struct BinaryMortonCodeHierarchy* bnodes,
    uniform global char* bvh_mem,
    uniform BuildRecordLocalMortonFlattener currentRecord,
    uniform uint  currQnodeLocalId, //local index for flattened qnoode, don't mix this with nodeIndex that is in morton build record
    uniform local uint* local_numRecords,
    uniform uint tictoc,
    uniform uint* sg_bu_startpoint_arr,
    uniform uint* sg_bu_startpoint_cnt,
    uniform uint parentOfRoot,
    uniform bool processRoot,
    uniform UPerNodeData* nodeData)
{
    varying ushort lane = get_sub_group_local_id();

    /* initialize child array */
    uniform uint numChildren = 2;
    varying struct BuildRecordLocalMortonFlattener sg_children;
    sg_children.local_parent_index__numItems = 0;

    uint binary_hierarchy_child_idx = (lane == 0) ? currentRecord.leftChild : currentRecord.rightChild;
    if (lane >= numChildren) binary_hierarchy_child_idx = 1 << 31;

    sg_children = TranslateToLocalRecord(BinaryMortonCodeHierarchy_getEntry(bnodes, binary_hierarchy_child_idx));

    /* fill QBVH6 node with up to 6 children */
    while (numChildren < BVH_NODE_N6)
    {
        // we dont have to do "local_parent_index__numItems & 0xFFFF" because local_parent_index part is 0 here at this point
        uint childNumItems = sg_children.local_parent_index__numItems;
        varying bool sg_is_leaf = childNumItems <= cfg_minLeafSize;
        if (sub_group_all(sg_is_leaf)) { break; }

        uniform uint   bestItems = sub_group_reduce_max_N6(childNumItems);
        uniform ushort bestChild = ctz(intel_sub_group_ballot(childNumItems == bestItems));
        varying uint   leftOfBest = sg_children.leftChild; // val important only for (lane == bestChild), not valid for other lanes
        uniform uint   rightOfBest = sub_group_broadcast(sg_children.rightChild, bestChild);

        varying uint nodeID = (lane == bestChild) ? leftOfBest : rightOfBest;

        if (lane == numChildren || lane == bestChild)
        {
            sg_children = TranslateToLocalRecord(BinaryMortonCodeHierarchy_getEntry(bnodes, nodeID));
        }

        numChildren++;
    }

    uniform uint global_offset;
    uniform uint child_node_index;

    bool isFatleafChild = (sg_children.local_parent_index__numItems <= cfg_minLeafSize) && (lane < numChildren);
    uint numFatleafChildren = popcount(intel_sub_group_ballot(isFatleafChild));

    if (lane <= numChildren) {
        uint           writeIDX = 0;

        if (lane == numChildren)
    {
        /* create nodes in local structure, to be used later in the bottom up to create nodes in actual bvh */
        MortonFlattenedBoxlessNode flattened_node;
            uint parentIDX;

            if (processRoot)
            {
                *local_numRecords = numChildren + 1;
                child_node_index = 1;
                writeIDX = 0;
        flattened_node.binary_hierarchy_index = 0xFFFFFFFF;
                flattened_node.childOffset_type = (1 << 6) | BVH_INTERNAL_NODE;
                parentIDX = parentOfRoot;
            }
            else
            {
                uint shift = (16 * tictoc);
                uint mask = 0xFFFF;
                uint atomicAddVal = numChildren << shift;
                child_node_index = atomic_add_local(local_numRecords, atomicAddVal);
                sub_group_barrier(0);
                writeIDX = currQnodeLocalId;
                parentIDX = currentRecord.local_parent_index__numItems >> 16;
                flattened_node.binary_hierarchy_index = 0xFFFFFFFF;
                sub_group_barrier(0);
                child_node_index = (child_node_index >> 16) + (child_node_index & mask);
        flattened_node.childOffset_type = ((child_node_index - currQnodeLocalId) << 6) | BVH_INTERNAL_NODE;
            }

#if MORTON_VERBOSE_LOG
            printf("wg %d: SUBGROUP_create_boxless_node_phase1: writeIDX %d, child_node_index %d - %d\n", get_group_id(0), writeIDX, child_node_index, child_node_index + numChildren);
#endif
            flattened_node.backPointer = (parentIDX << 6) | (numChildren << 3) | numFatleafChildren;
            sg_children = MortonFlattenedBoxlessNode_reinterpret_as_BR(flattened_node);
    }

        child_node_index = sub_group_broadcast(child_node_index, numChildren);

        if (lane != numChildren)
    {
            writeIDX = child_node_index + lane;
            sg_children.local_parent_index__numItems |= currQnodeLocalId << 16;
    }

        nodeData[writeIDX].buildRecord = sg_children;
    }

    if (numFatleafChildren == numChildren) {
        uint arridx = *sg_bu_startpoint_cnt;
        // GRL_INLINE void set_2xSG_arr_first_write(uint index, uint* arr, ushort val, short lane)
        set_2xSG_arr_first_write(arridx, sg_bu_startpoint_arr, (ushort)currQnodeLocalId, lane);
        *sg_bu_startpoint_cnt = arridx + 1;
    }
}

// TODO_OPT:  Consider having phase 0 bucket the build records by number of primitives, and dispatch different variants
//    of this kernel with different WG sizes.   There are many records produced that generate only 1 or 2 subtrees, so 8 SGs is
//     probably often wasted
GRL_INLINE void phase1_process_fatleaf(
    uint   globalBaseForInternalNodes,    // for root node this is indexOfRoot
    uint   globalParent          ,        // for root this should be parentOfRoot
    bool   isInstancePrimLeafType,        //
    uint   leafPrimType,                  //
    uint   leafStride,                    //
    global struct QBVHNodeN* nodeData,    // per group
    uint nodeDataStart,                   //
    struct AABB* primref,                 //
    BackPointers* backPointers,           //
    global struct MortonCodePrimitive* mc,//
    uint nodesToLeafsGap,                 //
    local union UPerNodeData* perNodeData,//
    bool processRoot,                               //
    short localNodeId,                              //
    BuildRecordLocalMortonFlattener fatleafRecord,  // per node
    uint primID )                                   //
{
    uint lane = get_sub_group_local_id();
    uint numChildren = (fatleafRecord.local_parent_index__numItems & 0xFFFF);
    uniform uint mcID = fatleafRecord.rangeStart;
    uint pseudolane = lane < numChildren ? lane : 0;
    varying struct AABB sg_bounds = primref[primID];

    uint local_parent_idx = (fatleafRecord.local_parent_index__numItems >> 16);
    uint globalNodeId = globalBaseForInternalNodes + localNodeId;
    uniform global struct QBVHNodeN* qnode = nodeData + globalNodeId;

    uint children_offset = (mcID * leafStride + nodesToLeafsGap) - globalNodeId;

    {
        /* For all primitives in a fat leaf we store a back
         * pointer. This way we can modify the fat leaf node at leaf construction time. */
        uint back_pointer = globalNodeId + nodeDataStart;
        /* Store back pointer and primID inside morton code array to
         * be later used by leaf creation. */
        mc[mcID + pseudolane].index_code = ((ulong)back_pointer) << 32 | (ulong)primID;
    }

    struct AABB reduce_bounds = AABB_sub_group_reduce_N6(&sg_bounds);
    reduce_bounds = AABB_sub_group_shuffle( &reduce_bounds, 0 );

    uint8_t instMask;
    if (isInstancePrimLeafType)
    {
        instMask = lane < numChildren ? PRIMREF_instanceMask(&sg_bounds) : 0;
        subgroup_setInstanceQBVHNodeN(children_offset, &sg_bounds, numChildren, qnode, instMask);
        instMask = sub_group_reduce_or_N6(instMask);
    }
    else
    {
        instMask = 0xFF;
        subgroup_setQBVHNodeN_setFields_reduced_bounds(children_offset, leafPrimType, &sg_bounds, numChildren, instMask, qnode, false, reduce_bounds);
    }

    reduce_bounds.lower.w = as_float((uint)instMask);
    uint reduce_bounds_lane = AABB_sub_group_shuffle_coordPerLane(&reduce_bounds, 0);
    local uint* boxUint = (local uint*)(perNodeData + localNodeId);
    if (get_sub_group_size() == 8 || lane < 8)
    {
        boxUint[lane] = reduce_bounds_lane;
        uint globalParentIdx;
        if (processRoot) {
            // for root, treeletRootGlobalIndex is index of rootsParent in global space
            globalParentIdx = globalParent;
        }
        else {
            // for non root, raw_parent_idx is in local space
            globalParentIdx = (local_parent_idx > 0) ? (globalBaseForInternalNodes + local_parent_idx) : globalParent;
        }
        if (lane == 0) {
            *InnerNode_GetBackPointer(backPointers, globalNodeId) = (globalParentIdx << 6) | (numChildren << 3);
        }
    }
}

GRL_INLINE void perform_phase1(global struct Globals* globals,
    global struct MortonCodePrimitive* mc,
    global struct AABB* primref,
    global struct BinaryMortonCodeHierarchy* bnodes,
    global char* bvh_mem,
    local union UPerNodeData* perNodeData,
    local uint* local_records_head,
    local uint* local_globalOffsetForNodes,
    BuildRecordLocalMortonFlattener rootRecord,
    uint treeletRootGlobalIndex,
    uint parentOfRootIndex,
    const uint leafPrimType,
    bool isInstancePrimLeafType)
{
    global struct BVHBase* bvh = (global struct BVHBase*)bvh_mem;
    varying ushort lane = get_sub_group_local_id();

    // array that will keep 2x8 shorts indices
    varying uint    sg_fatleaf_array = 0x0;
    uniform uint8_t sg_fatleaf_cnt = 0;
    /* terminate when all subtrees are leaves */

    uint subgroupId = get_sub_group_id();
    uint ID = subgroupId;

    uint sg_bu_startpoints = 0;
    uniform uint sg_bu_startpoints_cnt = 0;
    const uint shift_mask = globals->shift_mask;

    const uint nodeDataStart  = BVH_ROOT_NODE_OFFSET / 64;
    BackPointers* backPointers = BVHBase_GetBackPointers(bvh);
    global struct QBVHNodeN* nodeData = BVHBase_nodeData(bvh);

    uint* pLeafStart = (!isInstancePrimLeafType) ? &bvh->quadLeafStart : &bvh->instanceLeafStart;
    uint  leafStart = *pLeafStart;
    uint  leafStride = (!isInstancePrimLeafType) ? 1 : (sizeof(struct HwInstanceLeaf) / sizeof(struct InternalNode));
    uint  nodesToLeafsGap = leafStart - nodeDataStart;

    if (ID == 0)
    {
        BuildRecordLocalMortonFlattener current = rootRecord;

        if ((current.local_parent_index__numItems & 0xFFFF) <= BVH_NODE_N6)
        {
             *local_records_head = 1;
#if MORTON_DEBUG_CHECKS
                if (sg_fatleaf_cnt > 32) printf("parallel_build_phase1_Indirect_SG sg_fatleaf_array: one subgroup has more than 32 items remembered\n");
#endif
            BuildRecordLocalMortonFlattener fatleafRecord = current;
            uint numChildren = (fatleafRecord.local_parent_index__numItems & 0xFFFF);
            uint pseudolane = lane < numChildren ? lane : 0;
            uniform const uint mcID = fatleafRecord.rangeStart;
            varying uint primID = (uint)(mc[mcID + pseudolane].index_code & shift_mask);

            phase1_process_fatleaf(
                treeletRootGlobalIndex, parentOfRootIndex, isInstancePrimLeafType, leafPrimType, leafStride,
                nodeData, nodeDataStart, primref, backPointers, mc, nodesToLeafsGap, perNodeData,
                true, 0, fatleafRecord, primID);
        }
        else
        {
#if MORTON_VERBOSE_LOG
            if (get_local_id(0) == 0) { printf("wg %d perform_phase1: starting collapsing subtree with root at node %d \n", get_group_id(0), rootIndex); }
#endif
            //printf("local_records_head = %d\n", *local_records_head);
            SUBGROUP_create_boxless_node_phase1(globals, bnodes, bvh_mem, current, ID, local_records_head, 0, &sg_bu_startpoints, &sg_bu_startpoints_cnt, parentOfRootIndex, true, perNodeData);
            *local_globalOffsetForNodes = treeletRootGlobalIndex;
        }

        ID += get_num_sub_groups();
    }

    uniform uint priv_records_tail = 1;

    /* wait for all work items to have updated local_records array */
    work_group_barrier(CLK_LOCAL_MEM_FENCE);

    uniform uint priv_records_head = *local_records_head & 0xFFFF;
    treeletRootGlobalIndex = *local_globalOffsetForNodes; // propagated from subgroup 1
    uniform uint priv_records_tail_prev = priv_records_tail;
    uniform uint other_records_head = priv_records_head;

    uint ticToc = 1;

    if (priv_records_head == priv_records_tail)
    {
        return;
    }
    else
    {
        do
        {
            for (; ID < priv_records_head; ID += get_num_sub_groups())
            {
                BuildRecordLocalMortonFlattener current = (perNodeData[ID].buildRecord);

                if ((current.local_parent_index__numItems & 0xFFFF) <= BVH_NODE_N6)
                {
                    set_2xSG_arr_first_write(sg_fatleaf_cnt++, &sg_fatleaf_array, ID, lane);
#if MORTON_VERBOSE_LOG
                    if (lane == 0)printf("wg %d, sg %d, perform_phase1: node ID %d is fatleaf \n", get_group_id(0), get_sub_group_id(), ID);
#endif
#if MORTON_DEBUG_CHECKS
                    if (sg_fatleaf_cnt > 32) printf("parallel_build_phase1_Indirect_SG sg_fatleaf_array: one subgroup has more than 32 items remembered\n");
#endif
                }
                else
                {
                    SUBGROUP_create_boxless_node_phase1(globals, bnodes, bvh_mem, current, ID, local_records_head, ticToc, &sg_bu_startpoints, &sg_bu_startpoints_cnt, 0, 0, perNodeData);
                }
            }

            priv_records_tail = priv_records_head;
            /* wait for all work items to have updated local_records array */
            work_group_barrier(CLK_LOCAL_MEM_FENCE);
            {
                uint records_as_in_mem = *local_records_head;
                priv_records_head = (records_as_in_mem >> (16 * ticToc)) & 0xFFFF;
                uint other_records_head_temp = priv_records_head;
                priv_records_head += other_records_head;
                other_records_head = other_records_head_temp;
                ticToc = ticToc ^ 1;
#if MORTON_VERBOSE_LOG
                if(get_local_id(0) == 0)printf("wg %d, perform_phase1: priv_records_tail %d, priv_records_head %d, records_as_in_mem %x\n", get_group_id(0), get_sub_group_id(), priv_records_tail, priv_records_head, records_as_in_mem);
#endif
            }
        } while (priv_records_tail != priv_records_head); // get out of the loop if the tail reached the head
    }

    bool atomicNodeAllocation = treeletRootGlobalIndex > 0;
    bool atomicNodeAllocationProduce = (get_sub_group_id() + lane == 0) && atomicNodeAllocation;
    uint singleTreeletBumpBVHnodeCnt = (!atomicNodeAllocation && (get_sub_group_id() + lane == 0)) ? nodeDataStart + priv_records_tail : 0;

    uniform uint globalBaseForInternalNodes = 0;

    // we distinguish multi treelet from single treelets here by looking on our treeletRootGlobalIndex
    // if treelets root is whole tree root (treeletRootGlobalIndex==0) then we are the only treelet so
    // there's no need to synchronize multiple treelets nodes allocations with atomics.
    if (atomicNodeAllocationProduce)
    {
        *local_globalOffsetForNodes = allocate_inner_nodes(bvh, priv_records_tail - 1);
    }

    // because, root is allocated elsewhere, and first node placed in global mem is node with local index 1
            // mapping local to global:
            // local space                           global space
            // [0]             - treelet root        [treeletRootGlobalIndex]
            //                                       ... possibly very long distance ...
            // [1]             - first non root      [globalBaseForInternalNodes + 1] - this index is returned by atomic allocator above
            // [2]             - first               [globalBaseForInternalNodes + 2]
            // ...
            // [numToAllocate] - last node           [globalBaseForInternalNodes + 3]
    if (atomicNodeAllocation)
    {
        work_group_barrier(CLK_LOCAL_MEM_FENCE);
        globalBaseForInternalNodes = *local_globalOffsetForNodes -(nodeDataStart+1);
    }

#if MORTON_VERBOSE_LOG
    if (get_local_id(0) == 0) { printf("wg %d perform_phase1: globalBaseForInternalNodes %d, num local nodes %d\n", get_group_id(0), globalBaseForInternalNodes, priv_records_tail - 1); }
#endif

    if (sg_fatleaf_cnt)
    {
        short localNodeId = get_from_2xSG_arr(sg_fatleaf_cnt - 1, sg_fatleaf_array, lane);
        //if (localNodeId >= MORTON_BUILDER_SUBTREE_THRESHOLD * 2) continue;
        //if(local_startpoints_cnt > 1) return;
        BuildRecordLocalMortonFlattener fatleafRecord = perNodeData[localNodeId].buildRecord;

        varying uint primID;
        {
            uint numChildren = (fatleafRecord.local_parent_index__numItems & 0xFFFF);
            uint pseudolane = lane < numChildren ? lane : 0;
                uniform const uint mcID = fatleafRecord.rangeStart;
                primID = (uint)(mc[mcID + pseudolane].index_code & shift_mask);
        }

        // process fatleafs, and store their boxes to SLM
        // also put startpoints for bottom up
        //uint fatleaf_cnt = *local_startpoints_cnt;
        while (sg_fatleaf_cnt-- > 1)
        {
            short                           nextLocalNodeId   = get_from_2xSG_arr(sg_fatleaf_cnt-1, sg_fatleaf_array, lane);
            BuildRecordLocalMortonFlattener nextfatleafRecord = perNodeData[nextLocalNodeId].buildRecord;
            varying uint                    nextPrimId;

            {
                uint numChildren = (nextfatleafRecord.local_parent_index__numItems & 0xFFFF);
                uint pseudolane = lane < numChildren ? lane : 0;
                uniform const uint mcID = nextfatleafRecord.rangeStart;
                nextPrimId = (uint)(mc[mcID + pseudolane].index_code & shift_mask);
            }

            phase1_process_fatleaf(
                globalBaseForInternalNodes, treeletRootGlobalIndex, isInstancePrimLeafType, leafPrimType, leafStride,
                nodeData, nodeDataStart, primref, backPointers, mc, nodesToLeafsGap, perNodeData,
                false, localNodeId, fatleafRecord, primID);

            fatleafRecord = nextfatleafRecord;
            localNodeId   = nextLocalNodeId;
            primID        = nextPrimId;
        }

        phase1_process_fatleaf(
            globalBaseForInternalNodes, treeletRootGlobalIndex, isInstancePrimLeafType, leafPrimType, leafStride,
            nodeData, nodeDataStart, primref, backPointers, mc, nodesToLeafsGap, perNodeData,
            false, localNodeId, fatleafRecord, primID);
        }

#if 0
    // put collected bottom-up startpoints to wg shared array to later distribute the work evenly accross the groups.
        {
            ushort myStartpointWriteSite = 0;

            if (lane == 0)
            {
                myStartpointWriteSite = atomic_add_local((local uint*)local_startpoints_cnt, (ushort)sg_bu_startpoints_cnt);
            }
            myStartpointWriteSite = sub_group_broadcast(myStartpointWriteSite, 0);

            unpack_from_2xSG_arr(sg_bu_startpoints_cnt, sg_bu_startpoints, lane, local_startpoints_arr + myStartpointWriteSite);
        }
#endif

        work_group_barrier(CLK_LOCAL_MEM_FENCE);

        // distribute bottom-up startpoints
#if 0
        {
            short sp_count_to_divide = (*local_startpoints_cnt);

            //calculate the chunk for each sg.
            sg_bu_startpoints_cnt = sp_count_to_divide / get_num_sub_groups();
            uint sg_bu_startpoints_cnt_reminder = sp_count_to_divide % get_num_sub_groups();

            uint myReadSite = get_sub_group_id() * sg_bu_startpoints_cnt;
            if (get_sub_group_id() < sg_bu_startpoints_cnt_reminder) {
                //from the reminder elements if sg idx is < sg_bu_startpoints_cnt_reminder then sg gets one extra idx
                // and all sgs before it also have one extra
                myReadSite += get_sub_group_id();
                sg_bu_startpoints_cnt++;
        }
        else
        {
            // all reminder elements are consummed by previous sgs
            myReadSite += sg_bu_startpoints_cnt_reminder;
        }

        pack_from_2xSG_arr(local_startpoints_arr + myReadSite, sg_bu_startpoints_cnt, &sg_bu_startpoints, lane);
    }
#endif

    SUBGROUP_refit_bottom_up_local(nodeData, backPointers, treeletRootGlobalIndex, globalBaseForInternalNodes, lane, perNodeData, sg_bu_startpoints, sg_bu_startpoints_cnt);

    if (singleTreeletBumpBVHnodeCnt)
    {
        bvh->nodeDataCur = singleTreeletBumpBVHnodeCnt;
    }
}

GRL_INLINE void update_empty_blas(global struct BVHBase* bvh, uint leafPrimType)
{
    if (get_sub_group_id() == 0 )
    {
        global struct QBVHNodeN* qnode = BVHBase_nodeData(bvh);
        BackPointers* backPointers = BVHBase_GetBackPointers(bvh);

        //set required fields to mark that blas is empty
        uint k = (get_sub_group_local_id() < BVH_NODE_N6) ? get_sub_group_local_id() : 0;
        qnode->type = leafPrimType;
        qnode->instMask = 0;
        qnode->qbounds.lower_x[k] = 0x80;
        qnode->qbounds.upper_x[k] = 0;

        *InnerNode_GetBackPointer(backPointers, 0) = (((uint)-1) << 6);
    }
}

/*

  POSTSORT PHASE1:
  Two kernels here, selected by MORTON_BUILDER_SUBTREE_THRESHOLD.
  1. parallel_build_phase1_Indirect_SG - record[0] is set to the subtree tip
  2. parallel_build_phase1_Indirect_global_root - record[0] is set to the bvh root (no phase2 needed afterwards)

*/

__attribute__( (reqd_work_group_size( 512, 1, 1 )) )
__attribute__((intel_reqd_sub_group_size(16))) void kernel
parallel_build_phase1_Indirect_SG( global struct Globals* globals,
    global struct MortonCodePrimitive* mc,
    global struct AABB* primref,
    global struct BinaryMortonCodeHierarchy* bnodes,
    global char* bvh_mem)
{
    global struct BVHBase* bvh = (global struct BVHBase*)bvh_mem;
    const uint leafPrimType = globals->leafPrimType;

    //special case for empty blas
    if(globals->numPrimitives == 0)
    {
        bvh->nodeDataCur = BVH_ROOT_NODE_OFFSET / 64 + 1;
        update_empty_blas(bvh, leafPrimType);
        return;
    }

    local union UPerNodeData perNodeData[(MORTON_BUILDER_SUBTREE_THRESHOLD * 2) -1];
    local uint local_records_head;
    // Two separate SLM variables for local_globalOffsetForNodes to remove one of the barriers
    local uint local_globalOffsetForNodes, local_globalOffsetForNodes2;

    uint rootIndex = 0;
    uint parentOfRoot = 0;
    BuildRecordLocalMortonFlattener  rootBuildRecord;

    /* add start build record to local stack */
    if (get_sub_group_id() == 0 )
    {
        global struct BuildRecordMorton* records = (global struct BuildRecordMorton*)(bvh_mem + 64 * bvh->quadLeafStart);
        uint recordID = get_group_id(0);
        struct BuildRecordMorton mortonGlobalRecord = records[recordID];

        rootBuildRecord = TranslateToLocalRecord(BinaryMortonCodeHierarchy_getEntry(bnodes, mortonGlobalRecord.nodeID));

        parentOfRoot = mortonGlobalRecord.parent_index;
        rootIndex = mortonGlobalRecord.current_index;

#if MORTON_VERBOSE_LOG
        printf("P1_STARTPOINTS: current_index: %d, buildRecord.numItems: %d, buildRecord.binary_hierarchy_index: %d, buildRecord.local_parent_index: %d\n",
               local_globalOffsetForNodes, buildRecord.numItems, buildRecord.binary_hierarchy_index, buildRecord.local_parent_index);
#endif
    }

    if (leafPrimType == NODE_TYPE_INSTANCE)
    {
        perform_phase1(globals, mc, primref, bnodes, bvh_mem, perNodeData,
            &local_records_head, &local_globalOffsetForNodes,
            rootBuildRecord, rootIndex, parentOfRoot, NODE_TYPE_INSTANCE, true);
    }
    else
    {
        perform_phase1(globals, mc, primref, bnodes, bvh_mem, perNodeData,
            &local_records_head, &local_globalOffsetForNodes,
            rootBuildRecord, rootIndex, parentOfRoot, leafPrimType, false);
    }

}

__attribute__( (reqd_work_group_size( 512, 1, 1 )) )
__attribute__((intel_reqd_sub_group_size(16))) void kernel
parallel_build_phase1_Indirect_global_root( global struct Globals* globals,
    global struct MortonCodePrimitive* mc,
    global struct AABB* primref,
    global struct BinaryMortonCodeHierarchy* bnodes,
    global char* bvh_mem)
{
    global struct BVHBase* bvh = (global struct BVHBase*)bvh_mem;
    const uint leafPrimType = globals->leafPrimType;
    const uint nodeDataStart = BVH_ROOT_NODE_OFFSET / 64;

    bvh->nodeDataCur = nodeDataStart + 1;

    //special case for empty blas
    if(globals->numPrimitives == 0)
    {
        update_empty_blas(bvh, leafPrimType);
        return;
    }

    local union UPerNodeData perNodeData[MORTON_BUILDER_SUBTREE_THRESHOLD * 2 - 1];
    local uint local_records_head;
    local uint local_globalOffsetForNodes;

    BuildRecordLocalMortonFlattener rootBuildRecord;

    if (get_sub_group_id() == 0 )
    {
        struct BinaryMortonCodeHierarchy binaryNode = BinaryMortonCodeHierarchy_getEntry(bnodes, globals->binary_hierarchy_root);

        rootBuildRecord = TranslateToLocalRecord(binaryNode);

        local_globalOffsetForNodes = 0;
    }

    if (leafPrimType == NODE_TYPE_INSTANCE)
    {
        perform_phase1(globals, mc, primref, bnodes, bvh_mem, perNodeData,
            &local_records_head, &local_globalOffsetForNodes, rootBuildRecord, 0, (uint)-1, NODE_TYPE_INSTANCE, true);
    }
    else
    {
        perform_phase1(globals, mc, primref, bnodes, bvh_mem, perNodeData,
            &local_records_head, &local_globalOffsetForNodes, rootBuildRecord, 0, (uint)-1, leafPrimType, false);

    }
}

#if 0
GRL_INLINE void
DO_OLD_PARALLEL_BUILD_PHASE1( global struct Globals* globals,
    global struct MortonCodePrimitive* mc,
    global struct AABB* primref,
    global struct BinaryMortonCodeHierarchy* bnodes,
    global char* bvh_mem,
    uint startID, uint endID,
    local uint* local_numRecords,
    local uint* local_numRecordsOld,
    local struct BuildRecordMorton* local_records
)
{
    global struct BVHBase* bvh = (global struct BVHBase*)bvh_mem;
    global struct BuildRecordMorton* records = (global struct BuildRecordMorton*)(bvh_mem + bvh->quadLeafStart*64);

    /* iterate over all subtrees this workgroup should build */
    for ( uint recordID = startID; recordID < endID; recordID++ )
    {
        /* add start build record to local stack */
        if ( get_local_id( 0 ) == 0 )
        {
            local_records[0] = records[recordID];
            *local_numRecords = 1;
            *local_numRecordsOld = 0;
        }
        work_group_barrier( CLK_LOCAL_MEM_FENCE );

        /* terminate when all subtrees are leaves */
        while ( *local_numRecords != *local_numRecordsOld )
        {
            /* remember the old number of build records to detect later
       * whether we are done */
            if ( get_local_id( 0 ) == 0 )
            {
                *local_numRecordsOld = *local_numRecords;
            }
            work_group_barrier( CLK_LOCAL_MEM_FENCE );

            /* all work items in the sub group pick a subtree to build */
            for ( uint ID = get_local_id( 0 ); ID < *local_numRecordsOld; ID += get_local_size( 0 ) )
            {
                /* ignore small subtrees */
                if ( local_records[ID].items <= BVH_NODE_N6 )
                    continue;

                /* create QBVH node */
                create_node( globals, bnodes, bvh_mem, ID, local_numRecords, local_records, &local_records[ID] );
            }

            /* wait for all work items to have updated local_records array */
            work_group_barrier( CLK_LOCAL_MEM_FENCE );
        }

        const uint shift_mask = globals->shift_mask;
        const uint leafPrimType = globals->leafPrimType;
        const uint rootNodeOffset = BVH_ROOT_NODE_OFFSET;
        BackPointers* backPointers = BVHBase_GetBackPointers( bvh );
        global struct QBVHNodeN* nodeData = BVHBase_nodeData( bvh );

        /* create all fat leaf nodes and initiate refit */
        for ( uint ID = get_local_id( 0 ); ID < *local_numRecords; ID += get_local_size( 0 ) )
        {
            struct BuildRecordMorton current = local_records[ID];
            const uint primrefID = BinaryMortonCodeHierarchy_getRangeStart( bnodes, current.nodeID );

            global struct QBVHNodeN* qnode = nodeData + current.current_index;

            /* get bounds of all children of the fat leaf node */
            struct AABB bounds[BVH_NODE_N6];
            for ( uint i = 0; i < current.items; i++ )
            {
                /* get primID and bounds of primitive */
                const uint primID = (uint)(mc[primrefID + i].index_code & shift_mask);
                bounds[i] = primref[primID];

                /* For all primitives in a fat leaf we store a back
                 * pointer. This way we can modify the fat leaf node at leaf construction time. */
                const uint back_pointer = qnode - (struct QBVHNodeN*)bvh_mem;

                /* Store back pointer and primID inside morton code array to
                 * be later used by leaf creation. */
                mc[primrefID + i].index_code = ((ulong)back_pointer) << 32 | (ulong)primID;
            }

            /* update fat leaf node */
            QBVHNodeN_setType( qnode, leafPrimType );
            global void* offset;
            if ( leafPrimType != BVH_INSTANCE_NODE )
            {
                offset = bvh_mem + 64*bvh->quadLeafStart + primrefID * sizeof( struct Quad );
                QBVHNodeN_setChildIncr1( qnode );
            }
            else
            {
                offset = bvh_mem + 64*bvh->instanceLeafStart + primrefID * sizeof( struct HwInstanceLeaf );
                QBVHNodeN_setChildIncr2( qnode );
            }
            QBVH6Node_set_offset( qnode, offset );
            QBVHNodeN_setBounds( qnode, bounds, current.items );

            /* set back pointers for fat leaf nodes */
            *InnerNode_GetBackPointer(backPointers, current.current_index) = (current.parent_index << 6) | (current.items << 3);

            /* bottom up refit */
            refit_bottom_up( qnode, bvh, bounds, current.items );
        }
    }
}

/*

  This phase takes the build records calculated in phase0 as input and
  finished the BVH construction for all these subtrees.

*/
__attribute__((reqd_work_group_size(8, 1, 1)))
old_parallel_build_phase1(global struct Globals *globals,
                      global struct MortonCodePrimitive *mc,
                      global struct AABB *primref,
                      global struct BinaryMortonCodeHierarchy *bnodes,
                      global char *bvh_mem)
{
    global struct BVHBase *bvh = (global struct BVHBase *)bvh_mem;
    global struct BuildRecordMorton *records = (global struct BuildRecordMorton *)(bvh_mem + 64*bvh->quadLeafStart);

    /* a queue of build records */
    local struct BuildRecordMorton local_records[MORTON_BUILDER_SUBTREE_THRESHOLD];
    local uint local_numRecords;
    local uint local_numRecordsOld;

    /* construct range of build records that each sub group will process */
    const uint numRecords = globals->numBuildRecords;
    const uint startID = (get_group_id(0) + 0) * numRecords / get_num_groups(0);
    const uint endID = (get_group_id(0) + 1) * numRecords / get_num_groups(0);

    DO_OLD_PARALLEL_BUILD_PHASE1( globals, mc, primref, bnodes, bvh_mem, startID, endID, &local_numRecords, &local_numRecordsOld, local_records );

}

__attribute__( (reqd_work_group_size( 8, 1, 1 )) )
old_parallel_build_phase1_Indirect( global struct Globals* globals,
    global struct MortonCodePrimitive* mc,
    global struct AABB* primref,
    global struct BinaryMortonCodeHierarchy* bnodes,
    global char* bvh_mem )
{
    global struct BVHBase* bvh = (global struct BVHBase*)bvh_mem;
    global struct BuildRecordMorton* records = (global struct BuildRecordMorton*)(bvh_mem + 64*bvh->quadLeafStart);

    /* a queue of build records */
    local struct BuildRecordMorton local_records[MORTON_BUILDER_SUBTREE_THRESHOLD];
    local uint local_numRecords;
    local uint local_numRecordsOld;

    /* construct range of build records that each sub group will process */
    const uint numRecords = globals->numBuildRecords;
    uint startID = get_group_id( 0 );
    uint endID   = startID + 1;

    DO_OLD_PARALLEL_BUILD_PHASE1( globals, mc, primref, bnodes, bvh_mem, startID, endID, &local_numRecords, &local_numRecordsOld, local_records );

}
#endif
