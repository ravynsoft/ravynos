/*
 * Copyright © 2019 Raspberry Pi Ltd
 * Copyright © 2014-2017 Broadcom
 * Copyright (C) 2012 Rob Clark <robclark@freedesktop.org>
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

#ifndef V3D_SIMULATOR_H
#define V3D_SIMULATOR_H

#ifdef V3D_VERSION
#include "broadcom/common/v3d_macros.h"
#endif

#include <stdint.h>

struct v3d_simulator_file;

struct v3d_simulator_file* v3d_simulator_init(int fd);
void v3d_simulator_destroy(struct v3d_simulator_file *sim_file);
uint32_t v3d_simulator_get_spill(uint32_t spill_size);
int v3d_simulator_ioctl(int fd, unsigned long request, void *arg);
void v3d_simulator_open_from_handle(int fd, int handle, uint32_t size);
uint32_t v3d_simulator_get_mem_size(void);
uint32_t v3d_simulator_get_mem_free(void);

#ifdef v3dX
#  include "v3dx_simulator.h"
#else
#  define v3dX(x) v3d42_##x
#  include "v3dx_simulator.h"
#  undef v3dX

#  define v3dX(x) v3d71_##x
#  include "v3dx_simulator.h"
#  undef v3dX

#endif

/* Helper to call simulator ver specific functions */
#define v3d_X_simulator(thing) ({                     \
   __typeof(&v3d42_simulator_##thing) v3d_X_sim_thing;\
   switch (sim_state.ver) {                           \
   case 42:                                           \
      v3d_X_sim_thing = &v3d42_simulator_##thing;     \
      break;                                          \
   case 71:                                           \
      v3d_X_sim_thing = &v3d71_simulator_##thing;     \
      break;                                          \
   default:                                           \
      unreachable("Unsupported hardware generation"); \
   }                                                  \
   v3d_X_sim_thing;                                   \
})

#endif
