/*
 * Copyright 2019 Red Hat
 * All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * on the rights to use, copy, modify, merge, publish, distribute, sub
 * license, and/or sell copies of the Software, and to permit persons to whom
 * the Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the next
 * paragraph) shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT. IN NO EVENT SHALL
 * THE AUTHOR(S) AND/OR THEIR SUPPLIERS BE LIABLE FOR ANY CLAIM,
 * DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
 * OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE
 * USE OR OTHER DEALINGS IN THE SOFTWARE.
 */
#ifndef _NIR_TO_TGSI_INFO_H_
#define _NIR_TO_TGSI_INFO_H_

#include <stdbool.h>
struct nir_shader;
struct tgsi_shader_info;

/* only llvmpipe uses this path, so handle draw not using llvm */
#if DRAW_LLVM_AVAILABLE
void nir_tgsi_scan_shader(const struct nir_shader *nir,
                          struct tgsi_shader_info *info,
                          bool need_texcoord);
#else
static inline void nir_tgsi_scan_shader(const struct nir_shader *nir,
                                        struct tgsi_shader_info *info,
                                        bool need_texcoord) {}
#endif

#endif
