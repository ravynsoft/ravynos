/*
 * Copyright © Microsoft Corporation
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
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 */

#ifndef D3D12_CMD_SIGNATURE_H
#define D3D12_CMD_SIGNATURE_H

#include "d3d12_context.h"

struct d3d12_cmd_signature_key {
   uint8_t compute:1;
   uint8_t indexed:1;
   uint8_t draw_or_dispatch_params:1;
   /* 5 bits padding */
   uint8_t params_root_const_param;
   uint8_t params_root_const_offset;
   /* 8 bits padding */

   unsigned multi_draw_stride;

   /* Will be zero if no constant/resource updates in the arg buffer */
   ID3D12RootSignature *root_sig;
};

void
d3d12_cmd_signature_cache_init(struct d3d12_context *ctx);

void
d3d12_cmd_signature_cache_destroy(struct d3d12_context *ctx);

ID3D12CommandSignature *
d3d12_get_cmd_signature(struct d3d12_context *ctx,
                        const struct d3d12_cmd_signature_key *key);

#endif
