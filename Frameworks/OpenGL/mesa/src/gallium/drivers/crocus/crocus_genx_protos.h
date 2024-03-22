/*
 * Copyright © 2019 Intel Corporation
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */

/* GenX-specific function declarations.
 *
 * Don't include this directly, it will be included by crocus_context.h.
 *
 * NOTE: This header can be included multiple times, from the same file.
 */

/* crocus_state.c */
void genX(crocus_init_state)(struct crocus_context *ice);
void genX(crocus_init_screen_state)(struct crocus_screen *screen);
void genX(crocus_upload_urb)(struct crocus_batch *batch,
                      unsigned vs_size,
                      bool gs_present,
                      unsigned gs_size);
void genX(crocus_update_pma_fix)(struct crocus_context *ice,
                                 struct crocus_batch *batch,
                                 bool enable);
/* crocus_blorp.c */
void genX(crocus_init_blorp)(struct crocus_context *ice);

/* crocus_query.c */
void genX(crocus_init_query)(struct crocus_context *ice);
void genX(crocus_init_screen_query)(struct crocus_screen *screen);

/* crocus_blt.c */
void genX(crocus_init_blt)(struct crocus_screen *screen);
