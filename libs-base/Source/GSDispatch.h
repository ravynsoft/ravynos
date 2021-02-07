/* Support header for conditionally enabling use of libdispatch.
   Copyright (C) 2012 Free Software Foundation, Inc.

   Written by:  Niels Grewe <niels.grewe@halbordnung.de>
   Date: March 2012

   This file is part of the GNUstep Base Library.

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with this library; if not, write to the Free
   Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110 USA.
   */

#import "GNUstepBase/GSBlocks.h"

#if HAVE_DISPATCH_H
#include <dispatch.h>
#elif HAVE_DISPATCH_DISPATCH_H
#include <dispatch/dispatch.h>
#endif


/*
 * If gnustep-base is built with libdispatch support, these macros will expand
 * to code for creating and cleaning up after libdispach queues to which blocks
 * can be submitted. If libdispatch is not available, setup and teardown will
 * be no-ops, and the block will simply be executed on the calling thread.
 */
#if __has_feature(blocks) && (GS_USE_LIBDISPATCH == 1)

/*
 * Older versions of libdispatch do not support concurrent queues.
 * We define away the attributes in this case.
 */
#ifndef DISPATCH_QUEUE_SERIAL
#define DISPATCH_QUEUE_SERIAL NULL
#endif
#ifndef DISPATCH_QUEUE_CONCURRENT
#define DISPATCH_QUEUE_CONCURRENT NULL
#endif

#define GS_DISPATCH_GET_DEFAULT_CONCURRENT_QUEUE() dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_DEFAULT, 0)
/**
 * This macro creates a dispatch queue using the attributes
 */
#define GS_DISPATCH_QUEUE_CREATE(attr) dispatch_queue_create(NULL, attr)

/**
 * Create a dispatch group
 */
#define GS_DISPATCH_GROUP_CREATE() dispatch_group_create()

/**
 * Wait for the dispatch group to finish
 */
#define GS_DISPATCH_GROUP_FINISH(group) dispatch_group_wait(group, DISPATCH_TIME_FOREVER)

/**
 * Release an dispatch object.
 */
#define GS_DISPATCH_RELEASE(x) dispatch_release(x)

/**
 * Allows an arbitrary block to be submitted to the queue (if available) or run
 * in place. Since dispatch blocks return nothing and take no arguments, the
 * caller can use the before and after arguments to guard calling the block as
 * required.
 */
#define GS_DISPATCH_SUBMIT_BLOCK(group, queue, before, after, block, args, ...) \
  if (queue != NULL) {\
    dispatch_group_async(group, queue, ^(void){before block(args, ## __VA_ARGS__); after});\
  } else {\
    before\
    block(args, ## __VA_ARGS__);\
    after\
  }

/**
 * Submits a block without special provisions.
 */
#define GS_DISPATCH_SUBMIT_BLOCK_NO_ARGS(group, queue, block) dispatch_group_async(group, queue, block)


/**
 * Convenience macro to create concurrent dispatch queues for the various
 * -enumerateUsingBlock: methods. Non-concurrent will be run in place.
 */
#define GS_DISPATCH_CREATE_QUEUE_AND_GROUP_FOR_ENUMERATION(queue, opts)\
  dispatch_queue_t queue = NULL;\
  dispatch_group_t queue ## Group = NULL;\
  if (opts & NSEnumerationConcurrent)\
  {\
    queue = GS_DISPATCH_GET_DEFAULT_CONCURRENT_QUEUE();\
    queue ## Group = GS_DISPATCH_GROUP_CREATE();\
  }

/**
 * Convenience macro to destroy concurrent dispatch queues for the various
 * -enumerateUsingBlock: methods.
 */
#define GS_DISPATCH_TEARDOWN_QUEUE_AND_GROUP_FOR_ENUMERATION(queue, opts)\
  if (queue != NULL) { \
    GS_DISPATCH_GROUP_FINISH(queue ## Group);\
    GS_DISPATCH_RELEASE(queue ## Group);\
    if (NO == (opts & NSEnumerationConcurrent))\
    {\
      GS_DISPATCH_RELEASE(queue);\
    }\
  }


#else

/*
 * No-Op defintions if libdispatch is not supposed to be used.
 */
#define DISPATCH_QUEUE_SERIAL 0
#define DISPATCH_QUEUE_CONCURRENT 0
#define dispatch_queue_attr_t int
#define dispatch_queue_t int
#define dispatch_group_t int
#define GS_DISPATCH_GET_DEFAULT_CONCURRENT_QUEUE() 0
#define GS_DISPATCH_QUEUE_CREATE(attr) 0
#define GS_DISPATCH_GROUP_CREATE() 0
#define GS_DISPATCH_GROUP_FINISH(group)
#define GS_DISPATCH_RELEASE(x)
#define GS_DISPATCH_SUBMIT_BLOCK(group, queue, before, after, block, args...) CALL_BLOCK(block, args)
#define GS_DISPATCH_SUBMIT_BLOCK_NO_ARGS(group, queue, block) CALL_BLOCK_NO_ARGS(block)
#define GS_DISPATCH_CREATE_QUEUE_AND_GROUP_FOR_ENUMERATION(queue, opts)
#define GS_DISPATCH_TEARDOWN_QUEUE_AND_GROUP_FOR_ENUMERATION(queue, opts)
#endif



