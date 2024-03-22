/**********************************************************
 * Copyright 2015 VMware, Inc.  All rights reserved.
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

#ifndef VMW_DRM_QUERY_H
#define VMW_DRM_QUERY_H

#include "svga3d_reg.h"



/** Guest-backed query */
struct svga_winsys_gb_query
{
   struct svga_winsys_buffer *buf;
};


struct svga_winsys_gb_query *
vmw_svga_winsys_query_create(struct svga_winsys_screen *sws,
                             uint32 queryResultLen);

void
vmw_svga_winsys_query_destroy(struct svga_winsys_screen *sws,
                              struct svga_winsys_gb_query *query);

int
vmw_svga_winsys_query_init(struct svga_winsys_screen *sws,
                           struct svga_winsys_gb_query *query,
                           unsigned offset,
                           SVGA3dQueryState queryState);

void
vmw_svga_winsys_query_get_result(struct svga_winsys_screen *sws,
                       struct svga_winsys_gb_query *query,
                       unsigned offset,
                       SVGA3dQueryState *queryState,
                       void *result, uint32 resultLen);

enum pipe_error
vmw_swc_query_bind(struct svga_winsys_context *swc, 
                   struct svga_winsys_gb_query *query,
                   unsigned flags);

#endif /* VMW_DRM_QUERY_H */

