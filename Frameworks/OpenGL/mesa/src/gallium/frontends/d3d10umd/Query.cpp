/**************************************************************************
 *
 * Copyright 2012-2021 VMware, Inc.
 * All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sub license, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT. IN NO EVENT SHALL
 * THE COPYRIGHT HOLDERS, AUTHORS AND/OR ITS SUPPLIERS BE LIABLE FOR ANY CLAIM,
 * DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
 * OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE
 * USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 * The above copyright notice and this permission notice (including the
 * next paragraph) shall be included in all copies or substantial portions
 * of the Software.
 *
 **************************************************************************/

/*
 * Query.cpp --
 *    Functions that manipulate query resources.
 */


#include "Query.h"
#include "State.h"

#include "Debug.h"


/*
 * ----------------------------------------------------------------------
 *
 * CalcPrivateQuerySize --
 *
 *    The CalcPrivateQuerySize function determines the size of the
 *    user-mode display driver's private region of memory (that is,
 *    the size of internal driver structures, not the size of the
 *    resource video memory) for a query.
 *
 * ----------------------------------------------------------------------
 */

SIZE_T APIENTRY
CalcPrivateQuerySize(D3D10DDI_HDEVICE hDevice,                          // IN
                     __in const D3D10DDIARG_CREATEQUERY *pCreateQuery)  // IN
{
   return sizeof(Query);
}


static uint
TranslateQueryType(D3D10DDI_QUERY query)
{
   switch (query) {
   case D3D10DDI_QUERY_EVENT:
      return PIPE_QUERY_GPU_FINISHED;
   case D3D10DDI_QUERY_OCCLUSION:
      return PIPE_QUERY_OCCLUSION_COUNTER;
   case D3D10DDI_QUERY_TIMESTAMP:
      return PIPE_QUERY_TIMESTAMP;
   case D3D10DDI_QUERY_TIMESTAMPDISJOINT:
      return PIPE_QUERY_TIMESTAMP_DISJOINT;
   case D3D10DDI_QUERY_PIPELINESTATS:
      return PIPE_QUERY_PIPELINE_STATISTICS;
   case D3D10DDI_QUERY_OCCLUSIONPREDICATE:
      return PIPE_QUERY_OCCLUSION_PREDICATE;
   case D3D10DDI_QUERY_STREAMOUTPUTSTATS:
      return PIPE_QUERY_SO_STATISTICS;
   case D3D10DDI_QUERY_STREAMOVERFLOWPREDICATE:
      return PIPE_QUERY_SO_OVERFLOW_PREDICATE;
   default:
      LOG_UNSUPPORTED(true);
      return PIPE_QUERY_TYPES;
   }
}


/*
 * ----------------------------------------------------------------------
 *
 * CreateQuery --
 *
 *    The CreateQuery function creates driver-side resources for a
 *    query that the Microsoft Direct3D runtime subsequently issues
 *    for processing.
 *
 * ----------------------------------------------------------------------
 */

void APIENTRY
CreateQuery(D3D10DDI_HDEVICE hDevice,                          // IN
            __in const D3D10DDIARG_CREATEQUERY *pCreateQuery,  // IN
            D3D10DDI_HQUERY hQuery,                            // IN
            D3D10DDI_HRTQUERY hRTQuery)                        // IN
{
   LOG_ENTRYPOINT();

   Device *pDevice = CastDevice(hDevice);
   struct pipe_context *pipe = pDevice->pipe;

   Query *pQuery = CastQuery(hQuery);
   memset(pQuery, 0, sizeof *pQuery);

   pQuery->Type = pCreateQuery->Query;
   pQuery->Flags = pCreateQuery->MiscFlags;

   pQuery->pipe_type = TranslateQueryType(pCreateQuery->Query);
   if (pQuery->pipe_type < PIPE_QUERY_TYPES) {
      pQuery->handle = pipe->create_query(pipe, pQuery->pipe_type, 0);
   }
}


/*
 * ----------------------------------------------------------------------
 *
 * DestroyQuery --
 *
 *    The DestroyQuery function releases resources for a query.
 *
 * ----------------------------------------------------------------------
 */

void APIENTRY
DestroyQuery(D3D10DDI_HDEVICE hDevice, // IN
             D3D10DDI_HQUERY hQuery)   // IN
{
   LOG_ENTRYPOINT();

   struct pipe_context *pipe = CastPipeContext(hDevice);
   Query *pQuery = CastQuery(hQuery);

   if (pQuery->handle) {
      pipe->destroy_query(pipe, pQuery->handle);
   }
}


/*
 * ----------------------------------------------------------------------
 *
 * QueryBegin --
 *
 *    The QueryBegin function marks the beginning of a sequence of
 *    graphics commands for a query and transitions the query to the
 *    "building" state.
 *
 * ----------------------------------------------------------------------
 */

void APIENTRY
QueryBegin(D3D10DDI_HDEVICE hDevice,   // IN
           D3D10DDI_HQUERY hQuery)     // IN
{
   LOG_ENTRYPOINT();

   Device *pDevice = CastDevice(hDevice);
   struct pipe_context *pipe = pDevice->pipe;

   Query *pQuery = CastQuery(hQuery);
   struct pipe_query *state = CastPipeQuery(hQuery);

   if (state) {
      assert(pQuery->pipe_type < PIPE_QUERY_TYPES);
      pipe->begin_query(pipe, state);
   }
}


/*
 * ----------------------------------------------------------------------
 *
 * QueryEnd --
 *
 *    The QueryEnd function marks the end of a sequence of graphics
 *    commands for a query and transitions the query to the
 *    "issued" state.
 *
 * ----------------------------------------------------------------------
 */

void APIENTRY
QueryEnd(D3D10DDI_HDEVICE hDevice,  // IN
         D3D10DDI_HQUERY hQuery)    // IN
{
   LOG_ENTRYPOINT();

   Device *pDevice = CastDevice(hDevice);
   struct pipe_context *pipe = pDevice->pipe;
   Query *pQuery = CastQuery(hQuery);
   struct pipe_query *state = pQuery->handle;

   pQuery->SeqNo = ++pDevice->LastEmittedQuerySeqNo;
   pQuery->GetDataCount = 0;

   if (state) {
      pipe->end_query(pipe, state);
   }
}


/*
 * ----------------------------------------------------------------------
 *
 * QueryGetData --
 *
 *    The QueryGetData function polls for the state of a query operation.
 *
 * ----------------------------------------------------------------------
 */

void APIENTRY
QueryGetData(D3D10DDI_HDEVICE hDevice,                      // IN
             D3D10DDI_HQUERY hQuery,                        // IN
             __out_bcount_full_opt (DataSize) void *pData,  // OUT
             UINT DataSize,                                 // IN
             UINT Flags)                                    // IN
{
   LOG_ENTRYPOINT();

   Device *pDevice = CastDevice(hDevice);
   struct pipe_context *pipe = pDevice->pipe;
   Query *pQuery = CastQuery(hQuery);
   struct pipe_query *state = pQuery->handle;

   /*
    * Never return data for recently emitted queries immediately, to make
    * wgfasync happy.
    */
   if (DataSize == 0 &&
       (pQuery->SeqNo - pDevice->LastFinishedQuerySeqNo) > 0 &&
       (pQuery->GetDataCount++) == 0) {
      SetError(hDevice, DXGI_DDI_ERR_WASSTILLDRAWING);
      return;
   }

   bool wait = !!(Flags & D3D10_DDI_GET_DATA_DO_NOT_FLUSH);
   union pipe_query_result result;

   memset(&result, 0, sizeof result);

   bool ret;

   if (state) {
      ret = pipe->get_query_result(pipe, state, wait, &result);
   } else {
      LOG_UNSUPPORTED(true);
      ret = true;
   }

   if (!ret) {
      SetError(hDevice, DXGI_DDI_ERR_WASSTILLDRAWING);
      return;
   }

   if (pData) {
      switch (pQuery->Type) {
      case D3D10DDI_QUERY_EVENT:
      case D3D10DDI_QUERY_OCCLUSIONPREDICATE:
      case D3D10DDI_QUERY_STREAMOVERFLOWPREDICATE:
         *(BOOL *)pData = result.b;
         break;
      case D3D10DDI_QUERY_OCCLUSION:
      case D3D10DDI_QUERY_TIMESTAMP:
         *(UINT64 *)pData = result.u64;
         break;
      case D3D10DDI_QUERY_TIMESTAMPDISJOINT:
         {
            D3D10_DDI_QUERY_DATA_TIMESTAMP_DISJOINT *pResult =
              (D3D10_DDI_QUERY_DATA_TIMESTAMP_DISJOINT *)pData;
            pResult->Frequency = result.timestamp_disjoint.frequency;
            pResult->Disjoint = result.timestamp_disjoint.disjoint;
         }
         break;
      case D3D10DDI_QUERY_PIPELINESTATS:
         {
            D3D10_DDI_QUERY_DATA_PIPELINE_STATISTICS *pResult =
              (D3D10_DDI_QUERY_DATA_PIPELINE_STATISTICS *)pData;
            pResult->IAVertices = result.pipeline_statistics.ia_vertices;
            pResult->IAPrimitives = result.pipeline_statistics.ia_primitives;
            pResult->VSInvocations = result.pipeline_statistics.vs_invocations;
            pResult->GSInvocations = result.pipeline_statistics.gs_invocations;
            pResult->GSPrimitives = result.pipeline_statistics.gs_primitives;
            pResult->CInvocations = result.pipeline_statistics.c_invocations;
            pResult->CPrimitives = result.pipeline_statistics.c_primitives;
            pResult->PSInvocations = result.pipeline_statistics.ps_invocations;
            //pResult->HSInvocations = result.pipeline_statistics.hs_invocations;
            //pResult->DSInvocations = result.pipeline_statistics.ds_invocations;
            //pResult->CSInvocations = result.pipeline_statistics.cs_invocations;
         }
         break;
      case D3D10DDI_QUERY_STREAMOUTPUTSTATS:
         {
            D3D10_DDI_QUERY_DATA_SO_STATISTICS *pResult =
              (D3D10_DDI_QUERY_DATA_SO_STATISTICS *)pData;
            pResult->NumPrimitivesWritten = result.so_statistics.num_primitives_written;
            pResult->PrimitivesStorageNeeded = result.so_statistics.primitives_storage_needed;
         }
         break;
      default:
         assert(0);
         break;
      }
   }

   /*
    * Keep track of the last finished query, as wgfasync checks that queries
    * are completed in order.
    */
   if ((pQuery->SeqNo - pDevice->LastFinishedQuerySeqNo) > 0) {
      pDevice->LastFinishedQuerySeqNo = pQuery->SeqNo;
   }
   pQuery->GetDataCount = 0x80000000;
}


/*
 * ----------------------------------------------------------------------
 *
 * SetPredication --
 *
 *    The SetPredication function specifies whether rendering and
 *    resource-manipulation commands that follow are actually performed.
 *
 * ----------------------------------------------------------------------
 */

void APIENTRY
SetPredication(D3D10DDI_HDEVICE hDevice,  // IN
               D3D10DDI_HQUERY hQuery,    // IN
               BOOL PredicateValue)       // IN
{
   LOG_ENTRYPOINT();

   Device *pDevice = CastDevice(hDevice);
   struct pipe_context *pipe = pDevice->pipe;
   Query *pQuery = CastQuery(hQuery);
   struct pipe_query *state = CastPipeQuery(hQuery);
   enum pipe_render_cond_flag wait;

   wait = (pQuery && pQuery->Flags & D3D10DDI_QUERY_MISCFLAG_PREDICATEHINT) ?
             PIPE_RENDER_COND_NO_WAIT : PIPE_RENDER_COND_WAIT;

   pipe->render_condition(pipe, state, PredicateValue, wait);

   pDevice->pPredicate = pQuery;
   pDevice->PredicateValue = PredicateValue;
}


/*
 * ----------------------------------------------------------------------
 *
 * CheckPredicate --
 *
 *    Check predicate value and whether to draw or not.
 *
 * ----------------------------------------------------------------------
 */

BOOL
CheckPredicate(Device *pDevice)
{
   Query *pQuery = pDevice->pPredicate;
   if (!pQuery) {
      return true;
   }

   assert(pQuery->Type == D3D10DDI_QUERY_OCCLUSIONPREDICATE ||
          pQuery->Type == D3D10DDI_QUERY_STREAMOVERFLOWPREDICATE);

   struct pipe_context *pipe = pDevice->pipe;
   struct pipe_query *query = pQuery->handle;
   assert(query);

   union pipe_query_result result;
   memset(&result, 0, sizeof result);

   bool ret;
   ret = pipe->get_query_result(pipe, query, true, &result);
   assert(ret == true);
   if (!ret) {
      return true;
   }

   if (!!result.b == !!pDevice->PredicateValue) {
      return false;
   }

   return true;
}


/*
 * ----------------------------------------------------------------------
 *
 * CheckCounterInfo --
 *
 *    The CheckCounterInfo function determines global information that
 *    is related to manipulating counters.
 *
 * ----------------------------------------------------------------------
 */

void APIENTRY
CheckCounterInfo(D3D10DDI_HDEVICE hDevice,                  // IN
                 __out D3D10DDI_COUNTER_INFO *pCounterInfo) // OUT
{
   //LOG_ENTRYPOINT();

   pCounterInfo->LastDeviceDependentCounter = (D3D10DDI_QUERY)0;
   pCounterInfo->NumSimultaneousCounters = 0;
   pCounterInfo->NumDetectableParallelUnits = 0;
}


/*
 * ----------------------------------------------------------------------
 *
 * CheckCounter --
 *
 *    The CheckCounter function retrieves information that
 *    describes a counter.
 *
 * ----------------------------------------------------------------------
 */

void APIENTRY
CheckCounter(
   D3D10DDI_HDEVICE hDevice,                                                                // IN
   D3D10DDI_QUERY Query,                                                                    // IN
   __out D3D10DDI_COUNTER_TYPE *pCounterType,                                               // OUT
   __out UINT *pActiveCounters,                                                             // OUT
   __out_ecount_part_z_opt (*pNameLength, *pNameLength) LPSTR pName,                        // OUT
   __inout_opt UINT *pNameLength,                                                           // OUT
   __out_ecount_part_z_opt (*pUnitsLength, *pUnitsLength) LPSTR pUnits,                     // OUT
   __inout_opt UINT *pUnitsLength,                                                          // OUT
   __out_ecount_part_z_opt (*pDescriptionLength, *pDescriptionLength) LPSTR pDescription,   // OUT
   __inout_opt UINT* pDescriptionLength)                                                    // OUT
{
   LOG_ENTRYPOINT();

   SetError(hDevice, DXGI_DDI_ERR_UNSUPPORTED);
}
