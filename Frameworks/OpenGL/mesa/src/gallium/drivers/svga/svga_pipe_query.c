/**********************************************************
 * Copyright 2008-2015 VMware, Inc.  All rights reserved.
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without
 * restriction, including without limitation the rights to use, copy,
 * modify, merge, publish, distribute, sublicense, and/or sell copies
 * of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
 * BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
 * ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 **********************************************************/

#include "pipe/p_state.h"
#include "pipe/p_context.h"

#include "util/u_bitmask.h"
#include "util/u_memory.h"

#include "svga_cmd.h"
#include "svga_context.h"
#include "svga_screen.h"
#include "svga_resource_buffer.h"
#include "svga_winsys.h"
#include "svga_debug.h"


/* Fixme: want a public base class for all pipe structs, even if there
 * isn't much in them.
 */
struct pipe_query {
   int dummy;
};

struct svga_query {
   struct pipe_query base;
   unsigned type;                  /**< PIPE_QUERY_x or SVGA_QUERY_x */
   SVGA3dQueryType svga_type;      /**< SVGA3D_QUERYTYPE_x or unused */

   unsigned id;                    /** Per-context query identifier */
   bool active;                 /** TRUE if query is active */

   struct pipe_fence_handle *fence;

   /** For PIPE_QUERY_OCCLUSION_COUNTER / SVGA3D_QUERYTYPE_OCCLUSION */

   /* For VGPU9 */
   struct svga_winsys_buffer *hwbuf;
   volatile SVGA3dQueryResult *queryResult;

   /** For VGPU10 */
   struct svga_winsys_gb_query *gb_query;
   SVGA3dDXQueryFlags flags;
   unsigned offset;                /**< offset to the gb_query memory */
   struct pipe_query *predicate;   /** The associated query that can be used for predicate */

   /** For non-GPU SVGA_QUERY_x queries */
   uint64_t begin_count, end_count;
};


/** cast wrapper */
static inline struct svga_query *
svga_query(struct pipe_query *q)
{
   return (struct svga_query *)q;
}

/**
 * VGPU9
 */

static bool
svga_get_query_result(struct pipe_context *pipe,
                      struct pipe_query *q,
                      bool wait,
                      union pipe_query_result *result);

static enum pipe_error
define_query_vgpu9(struct svga_context *svga,
                   struct svga_query *sq)
{
   struct svga_winsys_screen *sws = svga_screen(svga->pipe.screen)->sws;

   sq->hwbuf = svga_winsys_buffer_create(svga, 1,
                                         SVGA_BUFFER_USAGE_PINNED,
                                         sizeof *sq->queryResult);
   if (!sq->hwbuf)
      return PIPE_ERROR_OUT_OF_MEMORY;

   sq->queryResult = (SVGA3dQueryResult *)
                     sws->buffer_map(sws, sq->hwbuf, PIPE_MAP_WRITE);
   if (!sq->queryResult) {
      sws->buffer_destroy(sws, sq->hwbuf);
      return PIPE_ERROR_OUT_OF_MEMORY;
   }

   sq->queryResult->totalSize = sizeof *sq->queryResult;
   sq->queryResult->state = SVGA3D_QUERYSTATE_NEW;

   /* We request the buffer to be pinned and assume it is always mapped.
    * The reason is that we don't want to wait for fences when checking the
    * query status.
    */
   sws->buffer_unmap(sws, sq->hwbuf);

   return PIPE_OK;
}

static void
begin_query_vgpu9(struct svga_context *svga, struct svga_query *sq)
{
   struct svga_winsys_screen *sws = svga_screen(svga->pipe.screen)->sws;

   if (sq->queryResult->state == SVGA3D_QUERYSTATE_PENDING) {
      /* The application doesn't care for the pending query result.
       * We cannot let go of the existing buffer and just get a new one
       * because its storage may be reused for other purposes and clobbered
       * by the host when it determines the query result.  So the only
       * option here is to wait for the existing query's result -- not a
       * big deal, given that no sane application would do this.
       */
       uint64_t result;
       svga_get_query_result(&svga->pipe, &sq->base, true, (void*)&result);
       assert(sq->queryResult->state != SVGA3D_QUERYSTATE_PENDING);
   }

   sq->queryResult->state = SVGA3D_QUERYSTATE_NEW;
   sws->fence_reference(sws, &sq->fence, NULL);

   SVGA_RETRY(svga, SVGA3D_BeginQuery(svga->swc, sq->svga_type));
}

static void
end_query_vgpu9(struct svga_context *svga, struct svga_query *sq)
{
   /* Set to PENDING before sending EndQuery. */
   sq->queryResult->state = SVGA3D_QUERYSTATE_PENDING;

   SVGA_RETRY(svga, SVGA3D_EndQuery(svga->swc, sq->svga_type, sq->hwbuf));
}

static bool
get_query_result_vgpu9(struct svga_context *svga, struct svga_query *sq,
                       bool wait, uint64_t *result)
{
   struct svga_winsys_screen *sws = svga_screen(svga->pipe.screen)->sws;
   SVGA3dQueryState state;

   if (!sq->fence) {
      /* The query status won't be updated by the host unless
       * SVGA_3D_CMD_WAIT_FOR_QUERY is emitted. Unfortunately this will cause
       * a synchronous wait on the host.
       */
      SVGA_RETRY(svga, SVGA3D_WaitForQuery(svga->swc, sq->svga_type,
                                           sq->hwbuf));
      svga_context_flush(svga, &sq->fence);
      assert(sq->fence);
   }

   state = sq->queryResult->state;
   if (state == SVGA3D_QUERYSTATE_PENDING) {
      if (!wait)
         return false;
      sws->fence_finish(sws, sq->fence, OS_TIMEOUT_INFINITE,
                        SVGA_FENCE_FLAG_QUERY);
      state = sq->queryResult->state;
   }

   assert(state == SVGA3D_QUERYSTATE_SUCCEEDED ||
          state == SVGA3D_QUERYSTATE_FAILED);

   *result = (uint64_t)sq->queryResult->result32;
   return true;
}


/**
 * VGPU10
 *
 * There is one query mob allocated for each context to be shared by all
 * query types. The mob is used to hold queries's state and result. Since
 * each query result type is of different length, to ease the query allocation
 * management, the mob is divided into memory blocks. Each memory block
 * will hold queries of the same type. Multiple memory blocks can be allocated
 * for a particular query type.
 *
 * Currently each memory block is of 184 bytes. We support up to 512
 * memory blocks. The query memory size is arbitrary right now.
 * Each occlusion query takes about 8 bytes. One memory block can accomodate
 * 23 occlusion queries. 512 of those blocks can support up to 11K occlusion
 * queries. That seems reasonable for now. If we think this limit is
 * not enough, we can increase the limit or try to grow the mob in runtime.
 * Note, SVGA device does not impose one mob per context for queries,
 * we could allocate multiple mobs for queries; however, wddm KMD does not
 * currently support that.
 *
 * Also note that the GL guest driver does not issue any of the
 * following commands: DXMoveQuery, DXBindAllQuery & DXReadbackAllQuery.
 */
#define SVGA_QUERY_MEM_BLOCK_SIZE    (sizeof(SVGADXQueryResultUnion) * 2)
#define SVGA_QUERY_MEM_SIZE          (512 * SVGA_QUERY_MEM_BLOCK_SIZE)

struct svga_qmem_alloc_entry
{
   unsigned start_offset;               /* start offset of the memory block */
   unsigned block_index;                /* block index of the memory block */
   unsigned query_size;                 /* query size in this memory block */
   unsigned nquery;                     /* number of queries allocated */
   struct util_bitmask *alloc_mask;     /* allocation mask */
   struct svga_qmem_alloc_entry *next;  /* next memory block */
};


/**
 * Allocate a memory block from the query object memory
 * \return NULL if out of memory, else pointer to the query memory block
 */
static struct svga_qmem_alloc_entry *
allocate_query_block(struct svga_context *svga)
{
   int index;
   unsigned offset;
   struct svga_qmem_alloc_entry *alloc_entry = NULL;

   /* Find the next available query block */
   index = util_bitmask_add(svga->gb_query_alloc_mask);

   if (index == UTIL_BITMASK_INVALID_INDEX)
      return NULL;

   offset = index * SVGA_QUERY_MEM_BLOCK_SIZE;
   if (offset >= svga->gb_query_len) {
      unsigned i;

      /* Deallocate the out-of-range index */
      util_bitmask_clear(svga->gb_query_alloc_mask, index);
      index = -1;

      /**
       * All the memory blocks are allocated, lets see if there is
       * any empty memory block around that can be freed up.
       */
      for (i = 0; i < SVGA3D_QUERYTYPE_MAX && index == -1; i++) {
         struct svga_qmem_alloc_entry *prev_alloc_entry = NULL;

         alloc_entry = svga->gb_query_map[i];
         while (alloc_entry && index == -1) {
            if (alloc_entry->nquery == 0) {
               /* This memory block is empty, it can be recycled. */
               if (prev_alloc_entry) {
                  prev_alloc_entry->next = alloc_entry->next;
               } else {
                  svga->gb_query_map[i] = alloc_entry->next;
               }
               index = alloc_entry->block_index;
            } else {
               prev_alloc_entry = alloc_entry;
               alloc_entry = alloc_entry->next;
            }
         }
      }

      if (index == -1) {
         debug_printf("Query memory object is full\n");
         return NULL;
      }
   }

   if (!alloc_entry) {
      assert(index != -1);
      alloc_entry = CALLOC_STRUCT(svga_qmem_alloc_entry);
      alloc_entry->block_index = index;
   }

   return alloc_entry;
}

/**
 * Allocate a slot in the specified memory block.
 * All slots in this memory block are of the same size.
 *
 * \return -1 if out of memory, else index of the query slot
 */
static int
allocate_query_slot(struct svga_context *svga,
                    struct svga_qmem_alloc_entry *alloc)
{
   int index;
   unsigned offset;

   /* Find the next available slot */
   index = util_bitmask_add(alloc->alloc_mask);

   if (index == UTIL_BITMASK_INVALID_INDEX)
      return -1;

   offset = index * alloc->query_size;
   if (offset >= SVGA_QUERY_MEM_BLOCK_SIZE)
      return -1;

   alloc->nquery++;

   return index;
}

/**
 * Deallocate the specified slot in the memory block.
 * If all slots are freed up, then deallocate the memory block
 * as well, so it can be allocated for other query type
 */
static void
deallocate_query_slot(struct svga_context *svga,
                      struct svga_qmem_alloc_entry *alloc,
                      unsigned index)
{
   assert(index != UTIL_BITMASK_INVALID_INDEX);

   util_bitmask_clear(alloc->alloc_mask, index);
   alloc->nquery--;

   /**
    * Don't worry about deallocating the empty memory block here.
    * The empty memory block will be recycled when no more memory block
    * can be allocated.
    */
}

static struct svga_qmem_alloc_entry *
allocate_query_block_entry(struct svga_context *svga,
                           unsigned len)
{
   struct svga_qmem_alloc_entry *alloc_entry;

   alloc_entry = allocate_query_block(svga);
   if (!alloc_entry)
      return NULL;

   assert(alloc_entry->block_index != -1);
   alloc_entry->start_offset =
      alloc_entry->block_index * SVGA_QUERY_MEM_BLOCK_SIZE;
   alloc_entry->nquery = 0;
   alloc_entry->alloc_mask = util_bitmask_create();
   alloc_entry->next = NULL;
   alloc_entry->query_size = len;

   return alloc_entry;
}

/**
 * Allocate a memory slot for a query of the specified type.
 * It will first search through the memory blocks that are allocated
 * for the query type. If no memory slot is available, it will try
 * to allocate another memory block within the query object memory for
 * this query type.
 */
static int
allocate_query(struct svga_context *svga,
               SVGA3dQueryType type,
               unsigned len)
{
   struct svga_qmem_alloc_entry *alloc_entry;
   int slot_index = -1;
   unsigned offset;

   assert(type < SVGA3D_QUERYTYPE_MAX);

   alloc_entry = svga->gb_query_map[type];

   if (!alloc_entry) {
      /**
       * No query memory block has been allocated for this query type,
       * allocate one now
       */
      alloc_entry = allocate_query_block_entry(svga, len);
      if (!alloc_entry)
         return -1;
      svga->gb_query_map[type] = alloc_entry;
   }

   /* Allocate a slot within the memory block allocated for this query type */
   slot_index = allocate_query_slot(svga, alloc_entry);

   if (slot_index == -1) {
      /* This query memory block is full, allocate another one */
      alloc_entry = allocate_query_block_entry(svga, len);
      if (!alloc_entry)
         return -1;
      alloc_entry->next = svga->gb_query_map[type];
      svga->gb_query_map[type] = alloc_entry;
      slot_index = allocate_query_slot(svga, alloc_entry);
   }

   assert(slot_index != -1);
   offset = slot_index * len + alloc_entry->start_offset;

   return offset;
}


/**
 * Deallocate memory slot allocated for the specified query
 */
static void
deallocate_query(struct svga_context *svga,
                 struct svga_query *sq)
{
   struct svga_qmem_alloc_entry *alloc_entry;
   unsigned slot_index;
   unsigned offset = sq->offset;

   alloc_entry = svga->gb_query_map[sq->svga_type];

   while (alloc_entry) {
      if (offset >= alloc_entry->start_offset &&
          offset < alloc_entry->start_offset + SVGA_QUERY_MEM_BLOCK_SIZE) {

         /* The slot belongs to this memory block, deallocate it */
         slot_index = (offset - alloc_entry->start_offset) /
                      alloc_entry->query_size;
         deallocate_query_slot(svga, alloc_entry, slot_index);
         alloc_entry = NULL;
      } else {
         alloc_entry = alloc_entry->next;
      }
   }
}


/**
 * Destroy the gb query object and all the related query structures
 */
static void
destroy_gb_query_obj(struct svga_context *svga)
{
   struct svga_winsys_screen *sws = svga_screen(svga->pipe.screen)->sws;
   unsigned i;

   for (i = 0; i < SVGA3D_QUERYTYPE_MAX; i++) {
      struct svga_qmem_alloc_entry *alloc_entry, *next;
      alloc_entry = svga->gb_query_map[i];
      while (alloc_entry) {
         next = alloc_entry->next;
         util_bitmask_destroy(alloc_entry->alloc_mask);
         FREE(alloc_entry);
         alloc_entry = next;
      }
      svga->gb_query_map[i] = NULL;
   }

   if (svga->gb_query)
      sws->query_destroy(sws, svga->gb_query);
   svga->gb_query = NULL;

   util_bitmask_destroy(svga->gb_query_alloc_mask);
}

/**
 * Define query and create the gb query object if it is not already created.
 * There is only one gb query object per context which will be shared by
 * queries of all types.
 */
static enum pipe_error
define_query_vgpu10(struct svga_context *svga,
                    struct svga_query *sq, int resultLen)
{
   struct svga_winsys_screen *sws = svga_screen(svga->pipe.screen)->sws;
   int qlen;
   enum pipe_error ret = PIPE_OK;

   SVGA_DBG(DEBUG_QUERY, "%s\n", __func__);

   if (svga->gb_query == NULL) {
      /* Create a gb query object */
      svga->gb_query = sws->query_create(sws, SVGA_QUERY_MEM_SIZE);
      if (!svga->gb_query)
         return PIPE_ERROR_OUT_OF_MEMORY;
      svga->gb_query_len = SVGA_QUERY_MEM_SIZE;
      memset (svga->gb_query_map, 0, sizeof(svga->gb_query_map));
      svga->gb_query_alloc_mask = util_bitmask_create();

      /* Bind the query object to the context */
      SVGA_RETRY(svga, svga->swc->query_bind(svga->swc, svga->gb_query,
                                             SVGA_QUERY_FLAG_SET));
   }

   sq->gb_query = svga->gb_query;

   /* Make sure query length is in multiples of 8 bytes */
   qlen = align(resultLen + sizeof(SVGA3dQueryState), 8);

   /* Find a slot for this query in the gb object */
   sq->offset = allocate_query(svga, sq->svga_type, qlen);
   if (sq->offset == -1)
      return PIPE_ERROR_OUT_OF_MEMORY;

   assert((sq->offset & 7) == 0);

   SVGA_DBG(DEBUG_QUERY, "   query type=%d qid=0x%x offset=%d\n",
            sq->svga_type, sq->id, sq->offset);

   /**
    * Send SVGA3D commands to define the query
    */
   SVGA_RETRY_OOM(svga, ret, SVGA3D_vgpu10_DefineQuery(svga->swc, sq->id,
                                                       sq->svga_type,
                                                       sq->flags));
   if (ret != PIPE_OK)
      return PIPE_ERROR_OUT_OF_MEMORY;

   SVGA_RETRY(svga, SVGA3D_vgpu10_BindQuery(svga->swc, sq->gb_query, sq->id));
   SVGA_RETRY(svga, SVGA3D_vgpu10_SetQueryOffset(svga->swc, sq->id,
                                                 sq->offset));

   return PIPE_OK;
}

static void
destroy_query_vgpu10(struct svga_context *svga, struct svga_query *sq)
{
   SVGA_RETRY(svga, SVGA3D_vgpu10_DestroyQuery(svga->swc, sq->id));

   /* Deallocate the memory slot allocated for this query */
   deallocate_query(svga, sq);
}


/**
 * Rebind queryies to the context.
 */
static void
rebind_vgpu10_query(struct svga_context *svga)
{
   SVGA_RETRY(svga, svga->swc->query_bind(svga->swc, svga->gb_query,
                                          SVGA_QUERY_FLAG_REF));
   svga->rebind.flags.query = false;
}


static enum pipe_error
begin_query_vgpu10(struct svga_context *svga, struct svga_query *sq)
{
   struct svga_winsys_screen *sws = svga_screen(svga->pipe.screen)->sws;
   int status = 0;

   sws->fence_reference(sws, &sq->fence, NULL);

   /* Initialize the query state to NEW */
   status = sws->query_init(sws, sq->gb_query, sq->offset, SVGA3D_QUERYSTATE_NEW);
   if (status)
      return PIPE_ERROR;

   if (svga->rebind.flags.query) {
      rebind_vgpu10_query(svga);
   }

   /* Send the BeginQuery command to the device */
   SVGA_RETRY(svga, SVGA3D_vgpu10_BeginQuery(svga->swc, sq->id));
   return PIPE_OK;
}

static void
end_query_vgpu10(struct svga_context *svga, struct svga_query *sq)
{
   if (svga->rebind.flags.query) {
      rebind_vgpu10_query(svga);
   }

   SVGA_RETRY(svga, SVGA3D_vgpu10_EndQuery(svga->swc, sq->id));
}

static bool
get_query_result_vgpu10(struct svga_context *svga, struct svga_query *sq,
                        bool wait, void *result, int resultLen)
{
   struct svga_winsys_screen *sws = svga_screen(svga->pipe.screen)->sws;
   SVGA3dQueryState queryState;

   if (svga->rebind.flags.query) {
      rebind_vgpu10_query(svga);
   }

   sws->query_get_result(sws, sq->gb_query, sq->offset, &queryState, result, resultLen);

   if (queryState != SVGA3D_QUERYSTATE_SUCCEEDED && !sq->fence) {
      /* We don't have the query result yet, and the query hasn't been
       * submitted.  We need to submit it now since the GL spec says
       * "Querying the state for a given occlusion query forces that
       * occlusion query to complete within a finite amount of time."
       */
      svga_context_flush(svga, &sq->fence);
   }

   if (queryState == SVGA3D_QUERYSTATE_PENDING ||
       queryState == SVGA3D_QUERYSTATE_NEW) {
      if (!wait)
         return false;
      sws->fence_finish(sws, sq->fence, OS_TIMEOUT_INFINITE,
                        SVGA_FENCE_FLAG_QUERY);
      sws->query_get_result(sws, sq->gb_query, sq->offset, &queryState, result, resultLen);
   }

   assert(queryState == SVGA3D_QUERYSTATE_SUCCEEDED ||
          queryState == SVGA3D_QUERYSTATE_FAILED);

   return true;
}

static struct pipe_query *
svga_create_query(struct pipe_context *pipe,
                  unsigned query_type,
                  unsigned index)
{
   struct svga_context *svga = svga_context(pipe);
   struct svga_query *sq;
   enum pipe_error ret;

   assert(query_type < SVGA_QUERY_MAX);

   sq = CALLOC_STRUCT(svga_query);
   if (!sq)
      goto fail;

   /* Allocate an integer ID for the query */
   sq->id = util_bitmask_add(svga->query_id_bm);
   if (sq->id == UTIL_BITMASK_INVALID_INDEX)
      goto fail;

   SVGA_DBG(DEBUG_QUERY, "%s type=%d sq=0x%x id=%d\n", __func__,
            query_type, sq, sq->id);

   switch (query_type) {
   case PIPE_QUERY_OCCLUSION_COUNTER:
      sq->svga_type = SVGA3D_QUERYTYPE_OCCLUSION;
      if (svga_have_vgpu10(svga)) {
         ret = define_query_vgpu10(svga, sq,
                                   sizeof(SVGADXOcclusionQueryResult));
         if (ret != PIPE_OK)
            goto fail;

         /**
          * In OpenGL, occlusion counter query can be used in conditional
          * rendering; however, in DX10, only OCCLUSION_PREDICATE query can
          * be used for predication. Hence, we need to create an occlusion
          * predicate query along with the occlusion counter query. So when
          * the occlusion counter query is used for predication, the associated
          * query of occlusion predicate type will be used
          * in the SetPredication command.
          */
         sq->predicate = svga_create_query(pipe, PIPE_QUERY_OCCLUSION_PREDICATE, index);

      } else {
         ret = define_query_vgpu9(svga, sq);
         if (ret != PIPE_OK)
            goto fail;
      }
      break;
   case PIPE_QUERY_OCCLUSION_PREDICATE:
   case PIPE_QUERY_OCCLUSION_PREDICATE_CONSERVATIVE:
      if (svga_have_vgpu10(svga)) {
         sq->svga_type = SVGA3D_QUERYTYPE_OCCLUSIONPREDICATE;
         ret = define_query_vgpu10(svga, sq,
                                   sizeof(SVGADXOcclusionPredicateQueryResult));
         if (ret != PIPE_OK)
            goto fail;
      } else {
         sq->svga_type = SVGA3D_QUERYTYPE_OCCLUSION;
         ret = define_query_vgpu9(svga, sq);
         if (ret != PIPE_OK)
            goto fail;
      }
      break;
   case PIPE_QUERY_PRIMITIVES_GENERATED:
   case PIPE_QUERY_PRIMITIVES_EMITTED:
   case PIPE_QUERY_SO_STATISTICS:
      assert(svga_have_vgpu10(svga));

      /* Until the device supports the new query type for multiple streams,
       * we will use the single stream query type for stream 0.
       */
      if (svga_have_sm5(svga) && index > 0) {
         assert(index < 4);

         sq->svga_type = SVGA3D_QUERYTYPE_SOSTATS_STREAM0 + index;
      }
      else {
         assert(index == 0);
         sq->svga_type = SVGA3D_QUERYTYPE_STREAMOUTPUTSTATS;
      }
      ret = define_query_vgpu10(svga, sq,
                                sizeof(SVGADXStreamOutStatisticsQueryResult));
      if (ret != PIPE_OK)
         goto fail;
      break;
   case PIPE_QUERY_TIMESTAMP:
      assert(svga_have_vgpu10(svga));
      sq->svga_type = SVGA3D_QUERYTYPE_TIMESTAMP;
      ret = define_query_vgpu10(svga, sq,
                                sizeof(SVGADXTimestampQueryResult));
      if (ret != PIPE_OK)
         goto fail;
      break;
   case SVGA_QUERY_NUM_DRAW_CALLS:
   case SVGA_QUERY_NUM_FALLBACKS:
   case SVGA_QUERY_NUM_FLUSHES:
   case SVGA_QUERY_NUM_VALIDATIONS:
   case SVGA_QUERY_NUM_BUFFERS_MAPPED:
   case SVGA_QUERY_NUM_TEXTURES_MAPPED:
   case SVGA_QUERY_NUM_BYTES_UPLOADED:
   case SVGA_QUERY_NUM_COMMAND_BUFFERS:
   case SVGA_QUERY_COMMAND_BUFFER_SIZE:
   case SVGA_QUERY_SURFACE_WRITE_FLUSHES:
   case SVGA_QUERY_MEMORY_USED:
   case SVGA_QUERY_NUM_SHADERS:
   case SVGA_QUERY_NUM_RESOURCES:
   case SVGA_QUERY_NUM_STATE_OBJECTS:
   case SVGA_QUERY_NUM_SURFACE_VIEWS:
   case SVGA_QUERY_NUM_GENERATE_MIPMAP:
   case SVGA_QUERY_NUM_READBACKS:
   case SVGA_QUERY_NUM_RESOURCE_UPDATES:
   case SVGA_QUERY_NUM_BUFFER_UPLOADS:
   case SVGA_QUERY_NUM_CONST_BUF_UPDATES:
   case SVGA_QUERY_NUM_CONST_UPDATES:
   case SVGA_QUERY_NUM_FAILED_ALLOCATIONS:
   case SVGA_QUERY_NUM_COMMANDS_PER_DRAW:
   case SVGA_QUERY_NUM_SHADER_RELOCATIONS:
   case SVGA_QUERY_NUM_SURFACE_RELOCATIONS:
   case SVGA_QUERY_SHADER_MEM_USED:
      break;
   case SVGA_QUERY_FLUSH_TIME:
   case SVGA_QUERY_MAP_BUFFER_TIME:
      /* These queries need os_time_get() */
      svga->hud.uses_time = true;
      break;

   default:
      assert(!"unexpected query type in svga_create_query()");
   }

   sq->type = query_type;

   return &sq->base;

fail:
   FREE(sq);
   return NULL;
}

static void
svga_destroy_query(struct pipe_context *pipe, struct pipe_query *q)
{
   struct svga_context *svga = svga_context(pipe);
   struct svga_winsys_screen *sws = svga_screen(svga->pipe.screen)->sws;
   struct svga_query *sq;

   if (!q) {
      destroy_gb_query_obj(svga);
      return;
   }

   sq = svga_query(q);

   SVGA_DBG(DEBUG_QUERY, "%s sq=0x%x id=%d\n", __func__,
            sq, sq->id);

   switch (sq->type) {
   case PIPE_QUERY_OCCLUSION_COUNTER:
   case PIPE_QUERY_OCCLUSION_PREDICATE:
   case PIPE_QUERY_OCCLUSION_PREDICATE_CONSERVATIVE:
      if (svga_have_vgpu10(svga)) {
         /* make sure to also destroy any associated predicate query */
         if (sq->predicate)
            svga_destroy_query(pipe, sq->predicate);
         destroy_query_vgpu10(svga, sq);
      } else {
         sws->buffer_destroy(sws, sq->hwbuf);
      }
      sws->fence_reference(sws, &sq->fence, NULL);
      break;
   case PIPE_QUERY_PRIMITIVES_GENERATED:
   case PIPE_QUERY_PRIMITIVES_EMITTED:
   case PIPE_QUERY_SO_STATISTICS:
   case PIPE_QUERY_TIMESTAMP:
      assert(svga_have_vgpu10(svga));
      destroy_query_vgpu10(svga, sq);
      sws->fence_reference(sws, &sq->fence, NULL);
      break;
   case SVGA_QUERY_NUM_DRAW_CALLS:
   case SVGA_QUERY_NUM_FALLBACKS:
   case SVGA_QUERY_NUM_FLUSHES:
   case SVGA_QUERY_NUM_VALIDATIONS:
   case SVGA_QUERY_MAP_BUFFER_TIME:
   case SVGA_QUERY_NUM_BUFFERS_MAPPED:
   case SVGA_QUERY_NUM_TEXTURES_MAPPED:
   case SVGA_QUERY_NUM_BYTES_UPLOADED:
   case SVGA_QUERY_NUM_COMMAND_BUFFERS:
   case SVGA_QUERY_COMMAND_BUFFER_SIZE:
   case SVGA_QUERY_FLUSH_TIME:
   case SVGA_QUERY_SURFACE_WRITE_FLUSHES:
   case SVGA_QUERY_MEMORY_USED:
   case SVGA_QUERY_NUM_SHADERS:
   case SVGA_QUERY_NUM_RESOURCES:
   case SVGA_QUERY_NUM_STATE_OBJECTS:
   case SVGA_QUERY_NUM_SURFACE_VIEWS:
   case SVGA_QUERY_NUM_GENERATE_MIPMAP:
   case SVGA_QUERY_NUM_READBACKS:
   case SVGA_QUERY_NUM_RESOURCE_UPDATES:
   case SVGA_QUERY_NUM_BUFFER_UPLOADS:
   case SVGA_QUERY_NUM_CONST_BUF_UPDATES:
   case SVGA_QUERY_NUM_CONST_UPDATES:
   case SVGA_QUERY_NUM_FAILED_ALLOCATIONS:
   case SVGA_QUERY_NUM_COMMANDS_PER_DRAW:
   case SVGA_QUERY_NUM_SHADER_RELOCATIONS:
   case SVGA_QUERY_NUM_SURFACE_RELOCATIONS:
   case SVGA_QUERY_SHADER_MEM_USED:
      /* nothing */
      break;
   default:
      assert(!"svga: unexpected query type in svga_destroy_query()");
   }

   /* Free the query id */
   util_bitmask_clear(svga->query_id_bm, sq->id);

   FREE(sq);
}


static bool
svga_begin_query(struct pipe_context *pipe, struct pipe_query *q)
{
   struct svga_context *svga = svga_context(pipe);
   struct svga_query *sq = svga_query(q);
   enum pipe_error ret = PIPE_OK;

   assert(sq);
   assert(sq->type < SVGA_QUERY_MAX);

   /* Need to flush out buffered drawing commands so that they don't
    * get counted in the query results.
    */
   svga_hwtnl_flush_retry(svga);

   switch (sq->type) {
   case PIPE_QUERY_OCCLUSION_COUNTER:
   case PIPE_QUERY_OCCLUSION_PREDICATE:
   case PIPE_QUERY_OCCLUSION_PREDICATE_CONSERVATIVE:
      if (svga_have_vgpu10(svga)) {
         ret = begin_query_vgpu10(svga, sq);
         /* also need to start the associated occlusion predicate query */
         if (sq->predicate) {
            enum pipe_error status;
            status = begin_query_vgpu10(svga, svga_query(sq->predicate));
            assert(status == PIPE_OK);
            (void) status;
         }
      } else {
         begin_query_vgpu9(svga, sq);
      }
      assert(ret == PIPE_OK);
      (void) ret;
      break;
   case PIPE_QUERY_PRIMITIVES_GENERATED:
   case PIPE_QUERY_PRIMITIVES_EMITTED:
   case PIPE_QUERY_SO_STATISTICS:
   case PIPE_QUERY_TIMESTAMP:
      assert(svga_have_vgpu10(svga));
      ret = begin_query_vgpu10(svga, sq);
      assert(ret == PIPE_OK);
      break;
   case SVGA_QUERY_NUM_DRAW_CALLS:
      sq->begin_count = svga->hud.num_draw_calls;
      break;
   case SVGA_QUERY_NUM_FALLBACKS:
      sq->begin_count = svga->hud.num_fallbacks;
      break;
   case SVGA_QUERY_NUM_FLUSHES:
      sq->begin_count = svga->hud.num_flushes;
      break;
   case SVGA_QUERY_NUM_VALIDATIONS:
      sq->begin_count = svga->hud.num_validations;
      break;
   case SVGA_QUERY_MAP_BUFFER_TIME:
      sq->begin_count = svga->hud.map_buffer_time;
      break;
   case SVGA_QUERY_NUM_BUFFERS_MAPPED:
      sq->begin_count = svga->hud.num_buffers_mapped;
      break;
   case SVGA_QUERY_NUM_TEXTURES_MAPPED:
      sq->begin_count = svga->hud.num_textures_mapped;
      break;
   case SVGA_QUERY_NUM_BYTES_UPLOADED:
      sq->begin_count = svga->hud.num_bytes_uploaded;
      break;
   case SVGA_QUERY_NUM_COMMAND_BUFFERS:
      sq->begin_count = svga->swc->num_command_buffers;
      break;
   case SVGA_QUERY_COMMAND_BUFFER_SIZE:
      sq->begin_count = svga->hud.command_buffer_size;
      break;
   case SVGA_QUERY_FLUSH_TIME:
      sq->begin_count = svga->hud.flush_time;
      break;
   case SVGA_QUERY_SURFACE_WRITE_FLUSHES:
      sq->begin_count = svga->hud.surface_write_flushes;
      break;
   case SVGA_QUERY_NUM_READBACKS:
      sq->begin_count = svga->hud.num_readbacks;
      break;
   case SVGA_QUERY_NUM_RESOURCE_UPDATES:
      sq->begin_count = svga->hud.num_resource_updates;
      break;
   case SVGA_QUERY_NUM_BUFFER_UPLOADS:
      sq->begin_count = svga->hud.num_buffer_uploads;
      break;
   case SVGA_QUERY_NUM_CONST_BUF_UPDATES:
      sq->begin_count = svga->hud.num_const_buf_updates;
      break;
   case SVGA_QUERY_NUM_CONST_UPDATES:
      sq->begin_count = svga->hud.num_const_updates;
      break;
   case SVGA_QUERY_NUM_SHADER_RELOCATIONS:
      sq->begin_count = svga->swc->num_shader_reloc;
      break;
   case SVGA_QUERY_NUM_SURFACE_RELOCATIONS:
      sq->begin_count = svga->swc->num_surf_reloc;
      break;
   case SVGA_QUERY_MEMORY_USED:
   case SVGA_QUERY_NUM_SHADERS:
   case SVGA_QUERY_NUM_RESOURCES:
   case SVGA_QUERY_NUM_STATE_OBJECTS:
   case SVGA_QUERY_NUM_SURFACE_VIEWS:
   case SVGA_QUERY_NUM_GENERATE_MIPMAP:
   case SVGA_QUERY_NUM_FAILED_ALLOCATIONS:
   case SVGA_QUERY_NUM_COMMANDS_PER_DRAW:
   case SVGA_QUERY_SHADER_MEM_USED:
      /* nothing */
      break;
   default:
      assert(!"unexpected query type in svga_begin_query()");
   }

   SVGA_DBG(DEBUG_QUERY, "%s sq=0x%x id=%d type=%d svga_type=%d\n",
            __func__, sq, sq->id, sq->type, sq->svga_type);

   sq->active = true;

   return true;
}


static bool
svga_end_query(struct pipe_context *pipe, struct pipe_query *q)
{
   struct svga_context *svga = svga_context(pipe);
   struct svga_query *sq = svga_query(q);

   assert(sq);
   assert(sq->type < SVGA_QUERY_MAX);

   SVGA_DBG(DEBUG_QUERY, "%s sq=0x%x type=%d\n",
            __func__, sq, sq->type);

   if (sq->type == PIPE_QUERY_TIMESTAMP && !sq->active)
      svga_begin_query(pipe, q);

   SVGA_DBG(DEBUG_QUERY, "%s sq=0x%x id=%d type=%d svga_type=%d\n",
            __func__, sq, sq->id, sq->type, sq->svga_type);

   svga_hwtnl_flush_retry(svga);

   assert(sq->active);

   switch (sq->type) {
   case PIPE_QUERY_OCCLUSION_COUNTER:
   case PIPE_QUERY_OCCLUSION_PREDICATE:
   case PIPE_QUERY_OCCLUSION_PREDICATE_CONSERVATIVE:
      if (svga_have_vgpu10(svga)) {
         end_query_vgpu10(svga, sq);
         /* also need to end the associated occlusion predicate query */
         if (sq->predicate) {
            end_query_vgpu10(svga, svga_query(sq->predicate));
         }
      } else {
         end_query_vgpu9(svga, sq);
      }
      break;
   case PIPE_QUERY_PRIMITIVES_GENERATED:
   case PIPE_QUERY_PRIMITIVES_EMITTED:
   case PIPE_QUERY_SO_STATISTICS:
   case PIPE_QUERY_TIMESTAMP:
      assert(svga_have_vgpu10(svga));
      end_query_vgpu10(svga, sq);
      break;
   case SVGA_QUERY_NUM_DRAW_CALLS:
      sq->end_count = svga->hud.num_draw_calls;
      break;
   case SVGA_QUERY_NUM_FALLBACKS:
      sq->end_count = svga->hud.num_fallbacks;
      break;
   case SVGA_QUERY_NUM_FLUSHES:
      sq->end_count = svga->hud.num_flushes;
      break;
   case SVGA_QUERY_NUM_VALIDATIONS:
      sq->end_count = svga->hud.num_validations;
      break;
   case SVGA_QUERY_MAP_BUFFER_TIME:
      sq->end_count = svga->hud.map_buffer_time;
      break;
   case SVGA_QUERY_NUM_BUFFERS_MAPPED:
      sq->end_count = svga->hud.num_buffers_mapped;
      break;
   case SVGA_QUERY_NUM_TEXTURES_MAPPED:
      sq->end_count = svga->hud.num_textures_mapped;
      break;
   case SVGA_QUERY_NUM_BYTES_UPLOADED:
      sq->end_count = svga->hud.num_bytes_uploaded;
      break;
   case SVGA_QUERY_NUM_COMMAND_BUFFERS:
      sq->end_count = svga->swc->num_command_buffers;
      break;
   case SVGA_QUERY_COMMAND_BUFFER_SIZE:
      sq->end_count = svga->hud.command_buffer_size;
      break;
   case SVGA_QUERY_FLUSH_TIME:
      sq->end_count = svga->hud.flush_time;
      break;
   case SVGA_QUERY_SURFACE_WRITE_FLUSHES:
      sq->end_count = svga->hud.surface_write_flushes;
      break;
   case SVGA_QUERY_NUM_READBACKS:
      sq->end_count = svga->hud.num_readbacks;
      break;
   case SVGA_QUERY_NUM_RESOURCE_UPDATES:
      sq->end_count = svga->hud.num_resource_updates;
      break;
   case SVGA_QUERY_NUM_BUFFER_UPLOADS:
      sq->end_count = svga->hud.num_buffer_uploads;
      break;
   case SVGA_QUERY_NUM_CONST_BUF_UPDATES:
      sq->end_count = svga->hud.num_const_buf_updates;
      break;
   case SVGA_QUERY_NUM_CONST_UPDATES:
      sq->end_count = svga->hud.num_const_updates;
      break;
   case SVGA_QUERY_NUM_SHADER_RELOCATIONS:
      sq->end_count = svga->swc->num_shader_reloc;
      break;
   case SVGA_QUERY_NUM_SURFACE_RELOCATIONS:
      sq->end_count = svga->swc->num_surf_reloc;
      break;
   case SVGA_QUERY_MEMORY_USED:
   case SVGA_QUERY_NUM_SHADERS:
   case SVGA_QUERY_NUM_RESOURCES:
   case SVGA_QUERY_NUM_STATE_OBJECTS:
   case SVGA_QUERY_NUM_SURFACE_VIEWS:
   case SVGA_QUERY_NUM_GENERATE_MIPMAP:
   case SVGA_QUERY_NUM_FAILED_ALLOCATIONS:
   case SVGA_QUERY_NUM_COMMANDS_PER_DRAW:
   case SVGA_QUERY_SHADER_MEM_USED:
      /* nothing */
      break;
   default:
      assert(!"unexpected query type in svga_end_query()");
   }
   sq->active = false;
   return true;
}


static bool
svga_get_query_result(struct pipe_context *pipe,
                      struct pipe_query *q,
                      bool wait,
                      union pipe_query_result *vresult)
{
   struct svga_screen *svgascreen = svga_screen(pipe->screen);
   struct svga_context *svga = svga_context(pipe);
   struct svga_query *sq = svga_query(q);
   uint64_t *result = (uint64_t *)vresult;
   bool ret = true;

   assert(sq);

   SVGA_DBG(DEBUG_QUERY, "%s sq=0x%x id=%d wait: %d\n",
            __func__, sq, sq->id, wait);

   switch (sq->type) {
   case PIPE_QUERY_OCCLUSION_COUNTER:
      if (svga_have_vgpu10(svga)) {
         SVGADXOcclusionQueryResult occResult;
         ret = get_query_result_vgpu10(svga, sq, wait,
                                       (void *)&occResult, sizeof(occResult));
         *result = (uint64_t)occResult.samplesRendered;
      } else {
         ret = get_query_result_vgpu9(svga, sq, wait, result);
      }
      break;
   case PIPE_QUERY_OCCLUSION_PREDICATE:
   case PIPE_QUERY_OCCLUSION_PREDICATE_CONSERVATIVE: {
      if (svga_have_vgpu10(svga)) {
         SVGADXOcclusionPredicateQueryResult occResult;
         ret = get_query_result_vgpu10(svga, sq, wait,
                                       (void *)&occResult, sizeof(occResult));
         vresult->b = occResult.anySamplesRendered != 0;
      } else {
         uint64_t count = 0;
         ret = get_query_result_vgpu9(svga, sq, wait, &count);
         vresult->b = count != 0;
      }
      break;
   }
   case PIPE_QUERY_SO_STATISTICS: {
      SVGADXStreamOutStatisticsQueryResult sResult;
      struct pipe_query_data_so_statistics *pResult =
         (struct pipe_query_data_so_statistics *)vresult;

      assert(svga_have_vgpu10(svga));
      ret = get_query_result_vgpu10(svga, sq, wait,
                                    (void *)&sResult, sizeof(sResult));
      pResult->num_primitives_written = sResult.numPrimitivesWritten;
      pResult->primitives_storage_needed = sResult.numPrimitivesRequired;
      break;
   }
   case PIPE_QUERY_TIMESTAMP: {
      SVGADXTimestampQueryResult sResult;

      assert(svga_have_vgpu10(svga));
      ret = get_query_result_vgpu10(svga, sq, wait,
                                    (void *)&sResult, sizeof(sResult));
      *result = (uint64_t)sResult.timestamp;
      break;
   }
   case PIPE_QUERY_PRIMITIVES_GENERATED: {
      SVGADXStreamOutStatisticsQueryResult sResult;

      assert(svga_have_vgpu10(svga));
      ret = get_query_result_vgpu10(svga, sq, wait,
                                    (void *)&sResult, sizeof sResult);
      *result = (uint64_t)sResult.numPrimitivesRequired;
      break;
   }
   case PIPE_QUERY_PRIMITIVES_EMITTED: {
      SVGADXStreamOutStatisticsQueryResult sResult;

      assert(svga_have_vgpu10(svga));
      ret = get_query_result_vgpu10(svga, sq, wait,
                                    (void *)&sResult, sizeof sResult);
      *result = (uint64_t)sResult.numPrimitivesWritten;
      break;
   }
   /* These are per-frame counters */
   case SVGA_QUERY_NUM_DRAW_CALLS:
   case SVGA_QUERY_NUM_FALLBACKS:
   case SVGA_QUERY_NUM_FLUSHES:
   case SVGA_QUERY_NUM_VALIDATIONS:
   case SVGA_QUERY_MAP_BUFFER_TIME:
   case SVGA_QUERY_NUM_BUFFERS_MAPPED:
   case SVGA_QUERY_NUM_TEXTURES_MAPPED:
   case SVGA_QUERY_NUM_BYTES_UPLOADED:
   case SVGA_QUERY_NUM_COMMAND_BUFFERS:
   case SVGA_QUERY_COMMAND_BUFFER_SIZE:
   case SVGA_QUERY_FLUSH_TIME:
   case SVGA_QUERY_SURFACE_WRITE_FLUSHES:
   case SVGA_QUERY_NUM_READBACKS:
   case SVGA_QUERY_NUM_RESOURCE_UPDATES:
   case SVGA_QUERY_NUM_BUFFER_UPLOADS:
   case SVGA_QUERY_NUM_CONST_BUF_UPDATES:
   case SVGA_QUERY_NUM_CONST_UPDATES:
   case SVGA_QUERY_NUM_SHADER_RELOCATIONS:
   case SVGA_QUERY_NUM_SURFACE_RELOCATIONS:
      vresult->u64 = sq->end_count - sq->begin_count;
      break;
   /* These are running total counters */
   case SVGA_QUERY_MEMORY_USED:
      vresult->u64 = svgascreen->hud.total_resource_bytes;
      break;
   case SVGA_QUERY_NUM_SHADERS:
      vresult->u64 = svga->hud.num_shaders;
      break;
   case SVGA_QUERY_NUM_RESOURCES:
      vresult->u64 = svgascreen->hud.num_resources;
      break;
   case SVGA_QUERY_NUM_STATE_OBJECTS:
      vresult->u64 = (svga->hud.num_blend_objects +
                      svga->hud.num_depthstencil_objects +
                      svga->hud.num_rasterizer_objects +
                      svga->hud.num_sampler_objects +
                      svga->hud.num_samplerview_objects +
                      svga->hud.num_vertexelement_objects);
      break;
   case SVGA_QUERY_NUM_SURFACE_VIEWS:
      vresult->u64 = svga->hud.num_surface_views;
      break;
   case SVGA_QUERY_NUM_GENERATE_MIPMAP:
      vresult->u64 = svga->hud.num_generate_mipmap;
      break;
   case SVGA_QUERY_NUM_FAILED_ALLOCATIONS:
      vresult->u64 = svgascreen->hud.num_failed_allocations;
      break;
   case SVGA_QUERY_NUM_COMMANDS_PER_DRAW:
      vresult->f = (float) svga->swc->num_commands
         / (float) svga->swc->num_draw_commands;
      break;
   case SVGA_QUERY_SHADER_MEM_USED:
      vresult->u64 = svga->hud.shader_mem_used;
      break;
   default:
      assert(!"unexpected query type in svga_get_query_result");
   }

   SVGA_DBG(DEBUG_QUERY, "%s result %d\n", __func__, *((uint64_t *)vresult));

   return ret;
}

static void
svga_render_condition(struct pipe_context *pipe, struct pipe_query *q,
                      bool condition, enum pipe_render_cond_flag mode)
{
   struct svga_context *svga = svga_context(pipe);
   struct svga_winsys_screen *sws = svga_screen(svga->pipe.screen)->sws;
   struct svga_query *sq = svga_query(q);
   SVGA3dQueryId queryId;

   SVGA_DBG(DEBUG_QUERY, "%s\n", __func__);

   assert(svga_have_vgpu10(svga));
   if (sq == NULL) {
      queryId = SVGA3D_INVALID_ID;
   }
   else {
      assert(sq->svga_type == SVGA3D_QUERYTYPE_OCCLUSION ||
             sq->svga_type == SVGA3D_QUERYTYPE_OCCLUSIONPREDICATE);

      if (sq->svga_type == SVGA3D_QUERYTYPE_OCCLUSION) {
         assert(sq->predicate);
         /**
          * For conditional rendering, make sure to use the associated
          * predicate query.
          */
         sq = svga_query(sq->predicate);
      }
      queryId = sq->id;

      if ((mode == PIPE_RENDER_COND_WAIT ||
           mode == PIPE_RENDER_COND_BY_REGION_WAIT) && sq->fence) {
         sws->fence_finish(sws, sq->fence, OS_TIMEOUT_INFINITE,
                           SVGA_FENCE_FLAG_QUERY);
      }
   }
   /*
    * if the kernel module doesn't support the predication command,
    * we'll just render unconditionally.
    * This is probably acceptable for the typical case of occlusion culling.
    */
   if (sws->have_set_predication_cmd) {
      SVGA_RETRY(svga, SVGA3D_vgpu10_SetPredication(svga->swc, queryId,
                                                    (uint32) condition));
      svga->pred.query_id = queryId;
      svga->pred.cond = condition;
   }

   svga->render_condition = (sq != NULL);
}


/*
 * This function is a workaround because we lack the ability to query
 * renderer's time synchronously.
 */
static uint64_t
svga_get_timestamp(struct pipe_context *pipe)
{
   struct pipe_query *q = svga_create_query(pipe, PIPE_QUERY_TIMESTAMP, 0);
   union pipe_query_result result;

   util_query_clear_result(&result, PIPE_QUERY_TIMESTAMP);
   svga_begin_query(pipe, q);
   svga_end_query(pipe,q);
   svga_get_query_result(pipe, q, true, &result);
   svga_destroy_query(pipe, q);

   return result.u64;
}


static void
svga_set_active_query_state(struct pipe_context *pipe, bool enable)
{
}


/**
 * \brief Toggle conditional rendering if already enabled
 *
 * \param svga[in]  The svga context
 * \param render_condition_enabled[in]  Whether to ignore requests to turn
 * conditional rendering off
 * \param on[in]  Whether to turn conditional rendering on or off
 */
void
svga_toggle_render_condition(struct svga_context *svga,
                             bool render_condition_enabled,
                             bool on)
{
   SVGA3dQueryId query_id;

   if (render_condition_enabled ||
       svga->pred.query_id == SVGA3D_INVALID_ID) {
      return;
   }

   /*
    * If we get here, it means that the system supports
    * conditional rendering since svga->pred.query_id has already been
    * modified for this context and thus support has already been
    * verified.
    */
   query_id = on ? svga->pred.query_id : SVGA3D_INVALID_ID;

   SVGA_RETRY(svga, SVGA3D_vgpu10_SetPredication(svga->swc, query_id,
                                                 (uint32) svga->pred.cond));
}


void
svga_init_query_functions(struct svga_context *svga)
{
   svga->pipe.create_query = svga_create_query;
   svga->pipe.destroy_query = svga_destroy_query;
   svga->pipe.begin_query = svga_begin_query;
   svga->pipe.end_query = svga_end_query;
   svga->pipe.get_query_result = svga_get_query_result;
   svga->pipe.set_active_query_state = svga_set_active_query_state;
   svga->pipe.render_condition = svga_render_condition;
   svga->pipe.get_timestamp = svga_get_timestamp;
}
