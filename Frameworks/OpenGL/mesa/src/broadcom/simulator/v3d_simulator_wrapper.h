/*
 * Copyright © 2017 Broadcom
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

#include <stdint.h>
#include <stdlib.h>

struct v3d_hw;

#ifdef __cplusplus
extern "C" {
#endif

struct v3d_hw *v3d_hw_auto_new(void *params);
uint32_t v3d_hw_get_mem(const struct v3d_hw *hw, uint32_t *size, void **p);
bool v3d_hw_alloc_mem(struct v3d_hw *hw, size_t min_size);
uint32_t v3d_hw_read_reg(struct v3d_hw *hw, uint32_t reg);
void v3d_hw_write_reg(struct v3d_hw *hw, uint32_t reg, uint32_t val);
void v3d_hw_tick(struct v3d_hw *hw);
int v3d_hw_get_version(struct v3d_hw *hw);
void v3d_hw_set_isr(struct v3d_hw *hw, void (*isr)(uint32_t status));
uint32_t v3d_hw_get_hub_core();

#ifdef __cplusplus
}
#endif
