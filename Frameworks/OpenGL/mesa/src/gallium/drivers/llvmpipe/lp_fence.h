/**************************************************************************
 *
 * Copyright 2009 VMware, Inc.
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
 * The above copyright notice and this permission notice (including the
 * next paragraph) shall be included in all copies or substantial portions
 * of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT.
 * IN NO EVENT SHALL VMWARE AND/OR ITS SUPPLIERS BE LIABLE FOR
 * ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 **************************************************************************/


#ifndef LP_FENCE_H
#define LP_FENCE_H


#include "util/u_thread.h"
#include "pipe/p_state.h"
#include "util/u_inlines.h"


struct pipe_screen;


struct lp_fence
{
   struct pipe_reference reference;
   unsigned id;

   mtx_t mutex;
   cnd_t signalled;

   bool issued;
   unsigned rank;
   unsigned count;
};


struct lp_fence *
lp_fence_create(unsigned rank);


void
lp_fence_signal(struct lp_fence *fence);

bool
lp_fence_signalled(struct lp_fence *fence);

void
lp_fence_wait(struct lp_fence *fence);

bool
lp_fence_timedwait(struct lp_fence *fence, uint64_t timeout);

void
llvmpipe_init_screen_fence_funcs(struct pipe_screen *screen);


void
lp_fence_destroy(struct lp_fence *fence);

static inline void
lp_fence_reference(struct lp_fence **ptr,
                   struct lp_fence *f)
{
   struct lp_fence *old = *ptr;

   if (pipe_reference(&old->reference, &f->reference)) {
      lp_fence_destroy(old);
   }
   
   *ptr = f;
}

static inline bool
lp_fence_issued(const struct lp_fence *fence)
{
   return fence->issued;
}


#endif /* LP_FENCE_H */
