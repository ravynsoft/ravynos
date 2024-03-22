/**************************************************************************
 *
 * Copyright 2011 Lauri Kasanen
 * All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sub license, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice (including the
 * next paragraph) shall be included in all copies or substantial portions
 * of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR
 * ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 **************************************************************************/

#ifndef PP_EXTERNAL_FILTERS_H
#define PP_EXTERNAL_FILTERS_H

#include "postprocess/postprocess.h"

#define PP_FILTERS 6            /* Increment this if you add filters */
#define PP_MAX_PASSES 6


typedef bool (*pp_init_func) (struct pp_queue_t *, unsigned int,
                              unsigned int);
typedef void (*pp_free_func) (struct pp_queue_t *, unsigned int);

struct pp_filter_t
{
   const char *name;            /* Config name */
   unsigned int inner_tmps;     /* Request how many inner temps */
   unsigned int shaders;        /* Request how many shaders */
   unsigned int verts;          /* How many are vertex shaders */
   pp_init_func init;           /* Init function */
   pp_func main;                /* Run function */
   pp_free_func free;           /* Free function */
};

extern const struct pp_filter_t pp_filters[PP_FILTERS];
#endif
