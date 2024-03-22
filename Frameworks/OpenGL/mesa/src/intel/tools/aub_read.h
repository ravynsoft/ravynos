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

#ifndef INTEL_AUB_READ
#define INTEL_AUB_READ

#include <stdint.h>

#include "dev/intel_device_info.h"

#ifdef __cplusplus
extern "C" {
#endif

struct aub_read {
   /* Caller's data */
   void *user_data;

   void (*error)(void *user_data, const void *aub_data, const char *msg);

   void (*info)(void *user_data, int pci_id, const char *app_name);

   void (*comment)(void *user_data, const char *msg);

   void (*local_write)(void *user_data, uint64_t phys_addr, const void *data, uint32_t data_len);
   void (*phys_write)(void *user_data, uint64_t phys_addr, const void *data, uint32_t data_len);
   void (*ggtt_write)(void *user_data, uint64_t phys_addr, const void *data, uint32_t data_len);
   void (*ggtt_entry_write)(void *user_data, uint64_t phys_addr,
                            const void *data, uint32_t data_len);

   void (*reg_write)(void *user_data, uint32_t reg_offset, uint32_t reg_value);

   void (*ring_write)(void *user_data, enum intel_engine_class engine,
                      const void *data, uint32_t data_len);
   void (*execlist_write)(void *user_data, enum intel_engine_class engine,
                          uint64_t context_descriptor);

   /* Reader's data */
   uint32_t render_elsp[4];
   int render_elsp_index;
   uint32_t video_elsp[4];
   int video_elsp_index;
   uint32_t blitter_elsp[4];
   int blitter_elsp_index;

   struct intel_device_info devinfo;
};

int aub_read_command(struct aub_read *read, const void *data, uint32_t data_len);

#ifdef __cplusplus
}
#endif

#endif /* INTEL_AUB_READ */
