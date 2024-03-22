/**********************************************************
 * Copyright 2015-2023 VMware, Inc.  All rights reserved.
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

#include "pipebuffer/pb_bufmgr.h"
#include "util/u_memory.h"

#include "vmw_screen.h"
#include "vmw_buffer.h"
#include "vmw_query.h"



struct svga_winsys_gb_query *
vmw_svga_winsys_query_create(struct svga_winsys_screen *sws,
                             uint32 queryResultLen)
{
   struct vmw_winsys_screen *vws = vmw_winsys_screen(sws);
   struct pb_manager *provider = vws->pools.dma_base;
   struct pb_desc desc = {0};
   struct pb_buffer *pb_buf;
   struct svga_winsys_gb_query *query;

   query = CALLOC_STRUCT(svga_winsys_gb_query);
   if (!query)
      return NULL;

   /* Allocate memory to hold queries for this context */
   desc.alignment = 4096;
   pb_buf = provider->create_buffer(provider, queryResultLen, &desc);
   query->buf = vmw_svga_winsys_buffer_wrap(pb_buf);

   if (!query->buf) {
      debug_printf("Failed to allocate memory for queries\n");
      FREE(query);
      query = NULL;
   }

   return query;
}



void
vmw_svga_winsys_query_destroy(struct svga_winsys_screen *sws,
                              struct svga_winsys_gb_query *query)
{
   vmw_svga_winsys_buffer_destroy(sws, query->buf);
   FREE(query);
}



int
vmw_svga_winsys_query_init(struct svga_winsys_screen *sws,
                           struct svga_winsys_gb_query *query,
                           unsigned offset,
                           SVGA3dQueryState queryState)
{
   SVGA3dQueryState *state;

   state = (SVGA3dQueryState *) vmw_svga_winsys_buffer_map(sws,
                                       query->buf,
                                       PIPE_MAP_WRITE);
   if (!state) {
      debug_printf("Failed to map query result memory for initialization\n");
      return -1;
   }

   /* Initialize the query state for the specified query slot */
   state = (SVGA3dQueryState *)((char *)state + offset);
   *state = queryState;

   vmw_svga_winsys_buffer_unmap(sws, query->buf);

   return 0;
}



void
vmw_svga_winsys_query_get_result(struct svga_winsys_screen *sws,
                                 struct svga_winsys_gb_query *query,
                                 unsigned offset,
                                 SVGA3dQueryState *queryState,
                                 void *result, uint32 resultLen)
{
   SVGA3dQueryState *state;

   state = (SVGA3dQueryState *) vmw_svga_winsys_buffer_map(sws,
                                       query->buf,
                                       PIPE_MAP_READ);
   if (!state) {
      debug_printf("Failed to lock query result memory\n");

      if (queryState)
         *queryState = SVGA3D_QUERYSTATE_FAILED;

      return;
   }

   state = (SVGA3dQueryState *)((char *)state + offset);

   if (queryState)
      *queryState = *state;

   if (result) {
      memcpy(result, state + 1, resultLen);
   }

   vmw_svga_winsys_buffer_unmap(sws, query->buf);
}


enum pipe_error
vmw_swc_query_bind(struct svga_winsys_context *swc, 
                   struct svga_winsys_gb_query *query,
                   unsigned flags)
{
   /* no-op on Linux */
   return PIPE_OK;
}

