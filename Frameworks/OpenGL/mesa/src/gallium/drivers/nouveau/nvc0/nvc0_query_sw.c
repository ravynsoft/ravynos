/*
 * Copyright 2011 Christoph Bumiller
 * Copyright 2015 Samuel Pitoiset
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR
 * OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 */

#include "nvc0/nvc0_context.h"

#include "nvc0_query_sw.h"

/* === DRIVER STATISTICS === */

#ifdef NOUVEAU_ENABLE_DRIVER_STATISTICS

static const char *nvc0_sw_query_drv_stat_names[] =
{
   "drv-tex_obj_current_count",
   "drv-tex_obj_current_bytes",
   "drv-buf_obj_current_count",
   "drv-buf_obj_current_bytes_vid",
   "drv-buf_obj_current_bytes_sys",
   "drv-tex_transfers_rd",
   "drv-tex_transfers_wr",
   "drv-tex_copy_count",
   "drv-tex_blit_count",
   "drv-tex_cache_flush_count",
   "drv-buf_transfers_rd",
   "drv-buf_transfers_wr",
   "drv-buf_read_bytes_staging_vid",
   "drv-buf_write_bytes_direct",
   "drv-buf_write_bytes_staging_vid",
   "drv-buf_write_bytes_staging_sys",
   "drv-buf_copy_bytes",
   "drv-buf_non_kernel_fence_sync_count",
   "drv-any_non_kernel_fence_sync_count",
   "drv-query_sync_count",
   "drv-gpu_serialize_count",
   "drv-draw_calls_array",
   "drv-draw_calls_indexed",
   "drv-draw_calls_fallback_count",
   "drv-user_buffer_upload_bytes",
   "drv-constbuf_upload_count",
   "drv-constbuf_upload_bytes",
   "drv-pushbuf_count",
   "drv-resource_validate_count"
};

#endif /* NOUVEAU_ENABLE_DRIVER_STATISTICS */

static void
nvc0_sw_destroy_query(struct nvc0_context *nvc0, struct nvc0_query *q)
{
   struct nvc0_sw_query *sq = nvc0_sw_query(q);
   FREE(sq);
}

static bool
nvc0_sw_begin_query(struct nvc0_context *nvc0, struct nvc0_query *q)
{
#ifdef NOUVEAU_ENABLE_DRIVER_STATISTICS
   struct nvc0_sw_query *sq = nvc0_sw_query(q);

   if (q->index >= 5) {
      sq->value = nvc0->screen->base.stats.v[q->index];
   } else {
      sq->value = 0;
   }
#endif
   return true;
}

static void
nvc0_sw_end_query(struct nvc0_context *nvc0, struct nvc0_query *q)
{
#ifdef NOUVEAU_ENABLE_DRIVER_STATISTICS
   struct nvc0_sw_query *sq = nvc0_sw_query(q);
   sq->value = nvc0->screen->base.stats.v[q->index] - sq->value;
#endif
}

static bool
nvc0_sw_get_query_result(struct nvc0_context *nvc0, struct nvc0_query *q,
                         bool wait, union pipe_query_result *result)
{
#ifdef NOUVEAU_ENABLE_DRIVER_STATISTICS
   struct nvc0_sw_query *sq = nvc0_sw_query(q);
   uint64_t *res64 = (uint64_t *)result;

   res64[0] = sq->value;
#endif
   return true;
}

static const struct nvc0_query_funcs sw_query_funcs = {
   .destroy_query = nvc0_sw_destroy_query,
   .begin_query = nvc0_sw_begin_query,
   .end_query = nvc0_sw_end_query,
   .get_query_result = nvc0_sw_get_query_result,
};

struct nvc0_query *
nvc0_sw_create_query(struct nvc0_context *nvcO, unsigned type, unsigned index)
{
   struct nvc0_sw_query *sq;
   struct nvc0_query *q;

   if (type < NVC0_SW_QUERY_DRV_STAT(0) || type > NVC0_SW_QUERY_DRV_STAT_LAST)
      return NULL;

   sq = CALLOC_STRUCT(nvc0_sw_query);
   if (!sq)
      return NULL;

   q = &sq->base;
   q->funcs = &sw_query_funcs;
   q->type = type;
   q->index = type - NVC0_SW_QUERY_DRV_STAT(0);

   return q;
}

int
nvc0_sw_get_driver_query_info(struct nvc0_screen *screen, unsigned id,
                              struct pipe_driver_query_info *info)
{
   int count = 0;

   count += NVC0_SW_QUERY_DRV_STAT_COUNT;
   if (!info)
      return count;

#ifdef NOUVEAU_ENABLE_DRIVER_STATISTICS
   if (id < count) {
      info->name = nvc0_sw_query_drv_stat_names[id];
      info->query_type = NVC0_SW_QUERY_DRV_STAT(id);
      info->type = PIPE_DRIVER_QUERY_TYPE_UINT64;
      info->max_value.u64 = 0;
      if (strstr(info->name, "bytes"))
         info->type = PIPE_DRIVER_QUERY_TYPE_BYTES;
      info->group_id = NVC0_SW_QUERY_DRV_STAT_GROUP;
      return 1;
   }
#endif
   return 0;
}
