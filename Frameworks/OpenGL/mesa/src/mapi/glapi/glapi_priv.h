/*
 * Mesa 3-D graphics library
 *
 * Copyright (C) 1999-2008  Brian Paul   All Rights Reserved.
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
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR
 * OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 */


#ifndef _GLAPI_PRIV_H
#define _GLAPI_PRIV_H

#include "glapi/glapi.h"
#include "util/glheader.h"


#ifdef __cplusplus
extern "C" {
#endif

/* entrypoint */

extern void
init_glapi_relocs_once(void);


extern _glapi_proc
get_entrypoint_address(unsigned int functionOffset);


/**
 * Size (in bytes) of dispatch function (entrypoint).
 */
#if defined(USE_X86_ASM)
#define DISPATCH_FUNCTION_SIZE  16
#endif

#if defined(USE_X64_64_ASM)
#define DISPATCH_FUNCTION_SIZE  16
#endif


#ifdef __cplusplus
}
#endif

#endif
