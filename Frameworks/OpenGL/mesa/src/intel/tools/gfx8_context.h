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

#ifndef GFX8_CONTEXT_H
#define GFX8_CONTEXT_H

static inline void gfx8_render_context_init(const struct intel_context_parameters *params,
                                            uint32_t *data, uint32_t *size)
{
   *size = CONTEXT_RENDER_SIZE;
   if (!data)
      return;

   *data++ = 0 /* MI_NOOP */;
   MI_LOAD_REGISTER_IMM_vals(data, MI_LRI_FORCE_POSTED,
                             0x2244 /* CONTEXT_CONTROL */,
                             0x90009 /* Inhibit Synchronous Context Switch | Engine Context Restore Inhibit */,
                             0x2034 /* RING_HEAD */, 0,
                             0x2030 /* RING_TAIL */, 0,
                             0x2038 /* RING_BUFFER_START */,       params->ring_addr,
                             0x203C /* RING_BUFFER_CONTROL */,     (params->ring_addr - 4096) | 1 /* Buffer Length | Ring Buffer Enable */,
                             0x2168 /* BB_HEAD_U */,               0,
                             0x2140 /* BB_HEAD_L */,               0,
                             0x2110 /* BB_STATE */,                0,
                             0x211C /* SECOND_BB_HEAD_U */,        0,
                             0x2114 /* SECOND_BB_HEAD_L */,        0,
                             0x2118 /* SECOND_BB_STATE */,         0,
                             0x21C0 /* BB_PER_CTX_PTR */,          0,
                             0x21C4 /* RCS_INDIRECT_CTX */,        0,
                             0x21C8 /* RCS_INDIRECT_CTX_OFFSET */, 0);
   /* MI_NOOP */
   *data++ = 0;
   *data++ = 0;

   *data++ = 0; /* MI_NOOP */
   MI_LOAD_REGISTER_IMM_vals(data, MI_LRI_FORCE_POSTED,
                             0x23A8 /* CTX_TIMESTAMP */, 0,
                             0x228C /* PDP3_UDW */,      0,
                             0x2288 /* PDP3_LDW */,      0,
                             0x2284 /* PDP2_UDW */,      0,
                             0x2280 /* PDP2_LDW */,      0,
                             0x227C /* PDP1_UDW */,      0,
                             0x2278 /* PDP1_LDW */,      0,
                             0x2274 /* PDP0_UDW */,      params->pml4_addr >> 32,
                             0x2270 /* PDP0_LDW */,      params->pml4_addr & 0xffffffff);
   /* MI_NOOP */
   for (int i = 0; i < 12; i++)
      *data++ = 0 /* MI_NOOP */;

   *data++ = 0 /* MI_NOOP */;
   MI_LOAD_REGISTER_IMM_vals(data, 0,
                             0x20C8 /* R_PWR_CLK_STATE */, 0x7FFFFFFF);
   *data++ = MI_BATCH_BUFFER_END;
}

static inline void gfx8_blitter_context_init(const struct intel_context_parameters *params,
                                             uint32_t *data, uint32_t *size)
{
   *size = CONTEXT_OTHER_SIZE;
   if (!data)
      return;

   *data++ = 0 /* MI_NOOP */;
   MI_LOAD_REGISTER_IMM_vals(data, MI_LRI_FORCE_POSTED,
                             0x22244 /* CONTEXT_CONTROL */,     0x90009 /* Inhibit Synchronous Context Switch | Engine Context Restore Inhibit */,
                             0x22034 /* RING_HEAD */,           0,
                             0x22030 /* RING_TAIL */,           0,
                             0x22038 /* RING_BUFFER_START */,   params->ring_addr,
                             0x2203C /* RING_BUFFER_CONTROL */, (params->ring_size - 4096) | 1 /* Buffer Length | Ring Buffer Enable */,
                             0x22168 /* BB_HEAD_U */,           0,
                             0x22140 /* BB_HEAD_L */,           0,
                             0x22110 /* BB_STATE */,            0,
                             0x2211C /* SECOND_BB_HEAD_U */,    0,
                             0x22114 /* SECOND_BB_HEAD_L */,    0,
                             0x22118 /* SECOND_BB_STATE */,     0);

   for (int i = 0; i < 8; i++)
      *data++ = 0 /* MI_NOOP */;

   *data = 0 /* MI_NOOP */;
   MI_LOAD_REGISTER_IMM_vals(data, MI_LRI_FORCE_POSTED,
                             0x223A8 /* CTX_TIMESTAMP */, 0,
                             0x2228C /* PDP3_UDW */,      0,
                             0x22288 /* PDP3_LDW */,      0,
                             0x22284 /* PDP2_UDW */,      0,
                             0x22280 /* PDP2_LDW */,      0,
                             0x2227C /* PDP1_UDW */,      0,
                             0x22278 /* PDP1_LDW */,      0,
                             0x22274 /* PDP0_UDW */,      params->pml4_addr >> 32,
                             0x22270 /* PDP0_LDW */,      params->pml4_addr & 0xffffffff);

   for (int i = 0; i < 12; i++)
      *data++ = 0 /* MI_NOOP */;

   *data++ = MI_BATCH_BUFFER_END;
}

static inline void gfx8_video_context_init(const struct intel_context_parameters *params,
                                           uint32_t *data, uint32_t *size)
{
   *size = CONTEXT_OTHER_SIZE;
   if (!data)
      return;

   *data++ = 0 /* MI_NOOP */;
   MI_LOAD_REGISTER_IMM_vals(data, MI_LRI_FORCE_POSTED,
                             0x1C244 /* CONTEXT_CONTROL */,     0x90009 /* Inhibit Synchronous Context Switch | Engine Context Restore Inhibit */,
                             0x1C034 /* RING_HEAD */,           0,
                             0x1C030 /* RING_TAIL */,           0,
                             0x1C038 /* RING_BUFFER_START */,   params->ring_addr,
                             0x1C03C /* RING_BUFFER_CONTROL */, (params->ring_size - 4096) | 1 /* Buffer Length | Ring Buffer Enable */,
                             0x1C168 /* BB_HEAD_U */,           0,
                             0x1C140 /* BB_HEAD_L */,           0,
                             0x1C110 /* BB_STATE */,            0,
                             0x1C11C /* SECOND_BB_HEAD_U */,    0,
                             0x1C114 /* SECOND_BB_HEAD_L */,    0,
                             0x1C118 /* SECOND_BB_STATE */,     0);
   for (int i = 0; i < 8; i++)
      *data++ = 0 /* MI_NOOP */;

   *data++ = 0 /* MI_NOOP */;
   MI_LOAD_REGISTER_IMM_vals(data, MI_LRI_FORCE_POSTED,
                             0x1C3A8 /* CTX_TIMESTAMP */, 0,
                             0x1C28C /* PDP3_UDW */,      0,
                             0x1C288 /* PDP3_LDW */,      0,
                             0x1C284 /* PDP2_UDW */,      0,
                             0x1C280 /* PDP2_LDW */,      0,
                             0x1C27C /* PDP1_UDW */,      0,
                             0x1C278 /* PDP1_LDW */,      0,
                             0x1C274 /* PDP0_UDW */,      params->pml4_addr >> 32,
                             0x1C270 /* PDP0_LDW */,      params->pml4_addr & 0xffffffff);
   for (int i = 0; i < 12; i++)
      *data++ = 0 /* MI_NOOP */;

   *data++ = MI_BATCH_BUFFER_END;
}

#endif /* GFX8_CONTEXT_H */
