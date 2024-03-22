/*
 * Copyright © 2018 Intel Corporation
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

#ifndef INTEL_GEM_H
#define INTEL_GEM_H

#ifdef __cplusplus
extern "C" {
#endif

#include <assert.h>
#include <time.h>
#include <errno.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/ioctl.h>

#include "intel_engine.h"
#include "util/macros.h"

#define RCS_TIMESTAMP 0x2358

static inline uint64_t
intel_canonical_address(uint64_t v)
{
   /* From the Broadwell PRM Vol. 2a, MI_LOAD_REGISTER_MEM::MemoryAddress:
    *
    *    "This field specifies the address of the memory location where the
    *    register value specified in the DWord above will read from. The
    *    address specifies the DWord location of the data. Range =
    *    GraphicsVirtualAddress[63:2] for a DWord register GraphicsAddress
    *    [63:48] are ignored by the HW and assumed to be in correct
    *    canonical form [63:48] == [47]."
    */
   const int shift = 63 - 47;
   return (int64_t)(v << shift) >> shift;
}

/**
 * This returns a 48-bit address with the high 16 bits zeroed.
 *
 * It's the opposite of intel_canonicalize_address.
 */
static inline uint64_t
intel_48b_address(uint64_t v)
{
   const int shift = 63 - 47;
   return (uint64_t)(v << shift) >> shift;
}

/**
 * Call ioctl, restarting if it is interrupted
 */
static inline int
intel_ioctl(int fd, unsigned long request, void *arg)
{
    int ret;

    do {
        ret = ioctl(fd, request, arg);
    } while (ret == -1 && (errno == EINTR || errno == EAGAIN));
    return ret;
}

bool intel_gem_supports_syncobj_wait(int fd);

bool
intel_gem_read_render_timestamp(int fd, enum intel_kmd_type kmd_type,
                                uint64_t *value);
bool
intel_gem_read_correlate_cpu_gpu_timestamp(int fd,
                                           enum intel_kmd_type kmd_type,
                                           enum intel_engine_class engine_class,
                                           uint16_t engine_instance,
                                           clockid_t cpu_clock_id,
                                           uint64_t *cpu_timestamp,
                                           uint64_t *gpu_timestamp,
                                           uint64_t *cpu_delta);
bool intel_gem_can_render_on_fd(int fd, enum intel_kmd_type kmd_type);

/* Functions only used by i915 */
enum intel_gem_create_context_flags {
   INTEL_GEM_CREATE_CONTEXT_EXT_RECOVERABLE_FLAG = BITFIELD_BIT(0),
   INTEL_GEM_CREATE_CONTEXT_EXT_PROTECTED_FLAG   = BITFIELD_BIT(1),
};

bool intel_gem_create_context(int fd, uint32_t *context_id);
bool intel_gem_destroy_context(int fd, uint32_t context_id);
bool
intel_gem_create_context_engines(int fd,
                                 enum intel_gem_create_context_flags flags,
                                 const struct intel_query_engine_info *info,
                                 int num_engines, enum intel_engine_class *engine_classes,
                                 uint32_t vm_id,
                                 uint32_t *context_id);
bool
intel_gem_set_context_param(int fd, uint32_t context, uint32_t param,
                            uint64_t value);
bool
intel_gem_get_context_param(int fd, uint32_t context, uint32_t param,
                            uint64_t *value);
bool intel_gem_get_param(int fd, uint32_t param, int *value);
bool intel_gem_wait_on_get_param(int fd, uint32_t param, int target_val,
                                 uint32_t timeout_ms);

bool intel_gem_create_context_ext(int fd, enum intel_gem_create_context_flags flags,
                                  uint32_t *ctx_id);
bool intel_gem_supports_protected_context(int fd,
                                          enum intel_kmd_type kmd_type);

#ifdef __cplusplus
}
#endif

#endif /* INTEL_GEM_H */
