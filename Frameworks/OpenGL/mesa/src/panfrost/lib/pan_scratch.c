/*
 * Copyright (C) 2019 Collabora, Ltd.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the next
 * paragraph) shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 * Authors:
 *   Alyssa Rosenzweig <alyssa.rosenzweig@collabora.com>
 */

#include "util/macros.h"
#include "util/u_math.h"
#include "pan_encoder.h"

/* Midgard has a small register file, so shaders with high register pressure
 * need to spill from the register file onto the stack. In addition to
 * spilling, it is desireable to allocate temporary arrays on the stack (for
 * instance because the register file does not support indirect access but the
 * stack does).
 *
 * The stack is located in "Thread Local Storage", sometimes abbreviated TLS in
 * the kernel source code. Thread local storage is allocated per-thread,
 * per-core, so threads executing concurrently do not interfere with each
 * other's stacks. On modern kernels, we may query
 * DRM_PANFROST_PARAM_THREAD_TLS_ALLOC for the number of threads per core we
 * must allocate for, and DRM_PANFROST_PARAM_SHADER_PRESENT for a bitmask of
 * shader cores (so take a popcount of that mask for the number of shader
 * cores). On older kernels that do not support querying these values,
 * following kbase, we may use the worst-case value of 256 threads for
 * THREAD_TLS_ALLOC, and the worst-case value of 16 cores for Midgard per the
 * "shader core count" column of the implementations table in
 * https://en.wikipedia.org/wiki/Mali_%28GPU% [citation needed]
 *
 * Within a particular thread, there is stack allocated. If it is present, its
 * size is a power-of-two, and it is at least 16 bytes. Stack is allocated
 * with the shared memory descriptor used for all shaders within a frame (note
 * that they don't execute concurrently so it's fine). So, consider the maximum
 * stack size used by any shader within a job, and then compute (where npot
 * denotes the next power of two):
 *
 *      bytes/thread = npot(max(size, 16))
 *      allocated = (# of bytes/thread) * (# of threads/core) * (# of cores)
 *
 * The size of Thread Local Storage is signaled to the GPU in the tls_size
 * field, which has a log2 modifier and is in units of 16 bytes.
 */

/* Computes log_stack_size = log2(ceil(s / 16)) */

unsigned
panfrost_get_stack_shift(unsigned stack_size)
{
   if (stack_size)
      return util_logbase2_ceil(DIV_ROUND_UP(stack_size, 16));
   else
      return 0;
}

/* Computes the aligned stack size given the shift and thread count. */

unsigned
panfrost_get_total_stack_size(unsigned thread_size, unsigned threads_per_core,
                              unsigned core_id_range)
{
   unsigned size_per_thread =
      (thread_size == 0) ? 0
                         : util_next_power_of_two(ALIGN_POT(thread_size, 16));

   return size_per_thread * threads_per_core * core_id_range;
}
