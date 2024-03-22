/*
* Copyright 2009, VMware, Inc.
* Copyright (C) 2010 LunarG Inc.
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

#ifndef ST_INTEROP_H
#define ST_INTEROP_H

#include "GL/mesa_glinterop.h"
#include "st_context.h"

int
st_interop_query_device_info(struct st_context *st,
                             struct mesa_glinterop_device_info *out);

int
st_interop_export_object(struct st_context *st,
                         struct mesa_glinterop_export_in *in,
                         struct mesa_glinterop_export_out *out);

int
st_interop_flush_objects(struct st_context *st,
                         unsigned count, struct mesa_glinterop_export_in *objects,
                         struct mesa_glinterop_flush_out *out);

#endif /* ST_INTEROP_H */
