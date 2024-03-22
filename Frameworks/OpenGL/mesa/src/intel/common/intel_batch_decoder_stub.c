/*
 * Copyright © 2021 Intel Corporation
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
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 */

#include "common/intel_decoder.h"
#include "util/log.h"

void
intel_batch_decode_ctx_init(struct intel_batch_decode_ctx *ctx,
                            const struct brw_isa_info *isa,
                            const struct intel_device_info *devinfo,
                            FILE *fp, enum intel_batch_decode_flags flags,
                            const char *xml_path,
                            struct intel_batch_decode_bo (*get_bo)(void *,
                                                                   bool,
                                                                   uint64_t),
                            unsigned (*get_state_size)(void *, uint64_t,
                                                       uint64_t),
                            void *user_data)
{
   /* Clear ctx to play safe. */
   memset(ctx, 0, sizeof(*ctx));
}

void
intel_batch_decode_ctx_finish(struct intel_batch_decode_ctx *ctx)
{
}

void
intel_print_batch(struct intel_batch_decode_ctx *ctx,
                  const uint32_t *batch, uint32_t batch_size,
                  uint64_t batch_addr, bool from_ring)
{
   mesa_logw("Batch logging not supported on Android.");
}

void
intel_batch_stats_reset(struct intel_batch_decode_ctx *ctx)
{
}

void
intel_batch_stats(struct intel_batch_decode_ctx *ctx,
                  const uint32_t *batch, uint32_t batch_size,
                  uint64_t batch_addr, bool from_ring)
{
}

void
intel_batch_print_stats(struct intel_batch_decode_ctx *ctx)
{
   mesa_logw("Batch logging not supported on Android.");
}
