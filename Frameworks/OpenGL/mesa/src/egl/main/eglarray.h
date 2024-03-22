/**************************************************************************
 *
 * Copyright 2010 LunarG, Inc.
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
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 *
 **************************************************************************/

#ifndef EGLARRAY_INCLUDED
#define EGLARRAY_INCLUDED

#include "egltypedefs.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef EGLBoolean (*_EGLArrayForEach)(void *elem, void *foreach_data);

struct _egl_array {
   const char *Name;
   EGLint MaxSize;

   void **Elements;
   EGLint Size;
};

extern _EGLArray *
_eglCreateArray(const char *name, EGLint init_size);

extern void
_eglDestroyArray(_EGLArray *array, void (*free_cb)(void *));

extern void
_eglAppendArray(_EGLArray *array, void *elem);

extern void
_eglEraseArray(_EGLArray *array, EGLint i, void (*free_cb)(void *));

void *
_eglFindArray(_EGLArray *array, void *elem);

extern EGLint
_eglFilterArray(_EGLArray *array, void **data, EGLint size,
                _EGLArrayForEach filter, void *filter_data);

EGLint
_eglFlattenArray(_EGLArray *array, void *buffer, EGLint elem_size, EGLint size,
                 _EGLArrayForEach flatten);

static inline EGLint
_eglGetArraySize(_EGLArray *array)
{
   return (array) ? array->Size : 0;
}

#ifdef __cplusplus
}
#endif

#endif /* EGLARRAY_INCLUDED */
