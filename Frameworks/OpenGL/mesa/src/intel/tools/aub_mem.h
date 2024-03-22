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
 *
 */

#ifndef INTEL_AUB_MEM
#define INTEL_AUB_MEM

#include <stdint.h>

#include "util/list.h"
#include "util/rb_tree.h"

#include "dev/intel_device_info.h"
#include "common/intel_decoder.h"

#ifdef __cplusplus
extern "C" {
#endif

struct aub_mem {
   uint64_t pml4;

   int mem_fd;
   off_t mem_fd_len;

   struct list_head maps;
   struct rb_tree ggtt;
   struct rb_tree mem;
};

bool aub_mem_init(struct aub_mem *mem);
void aub_mem_fini(struct aub_mem *mem);

void aub_mem_clear_bo_maps(struct aub_mem *mem);

void aub_mem_phys_write(void *mem, uint64_t virt_address,
                        const void *data, uint32_t size);
void aub_mem_ggtt_write(void *mem, uint64_t virt_address,
                        const void *data, uint32_t size);
void aub_mem_ggtt_entry_write(void *mem, uint64_t virt_address,
                              const void *data, uint32_t size);
void aub_mem_local_write(void *mem, uint64_t virt_address,
                         const void *data, uint32_t size);

struct intel_batch_decode_bo aub_mem_get_ggtt_bo(void *mem, uint64_t address);
struct intel_batch_decode_bo aub_mem_get_ppgtt_bo(void *mem, uint64_t address);

struct intel_batch_decode_bo aub_mem_get_phys_addr_data(struct aub_mem *mem, uint64_t phys_addr);
struct intel_batch_decode_bo aub_mem_get_ppgtt_addr_data(struct aub_mem *mem, uint64_t virt_addr);

struct intel_batch_decode_bo aub_mem_get_ppgtt_addr_aub_data(struct aub_mem *mem, uint64_t virt_addr);


#ifdef __cplusplus
}
#endif

#endif /* INTEL_AUB_MEM */
