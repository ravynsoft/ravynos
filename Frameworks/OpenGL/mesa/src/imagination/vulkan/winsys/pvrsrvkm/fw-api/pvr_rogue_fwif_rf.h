/*
 * Copyright © 2022 Imagination Technologies Ltd.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the next
 * paragraph) shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#ifndef PVR_ROGUE_FWIF_RF_H
#define PVR_ROGUE_FWIF_RF_H

#include <stdint.h>

#include "pvr_rogue_fwif_shared.h"
#include "util/macros.h"

struct rogue_fwif_rf_regs {
   union {
      uint64_t cdm_cb_base;
      uint64_t cdm_ctrl_stream_base;
   };

   uint64_t cdm_cb_queue;
   uint64_t cdm_cb;
};

struct rogue_fwif_rf_cmd {
   /* THIS MUST BE THE LAST MEMBER OF THE CONTAINING STRUCTURE */
   alignas(8) struct rogue_fwif_rf_regs regs;
};

#endif /* PVR_ROGUE_FWIF_RF_H */
