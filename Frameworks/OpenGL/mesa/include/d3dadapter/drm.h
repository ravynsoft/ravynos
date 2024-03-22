/*
 * Copyright 2011 Joakim Sindholt <opensource@zhasha.com>
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
 * USE OR OTHER DEALINGS IN THE SOFTWARE. */

#ifndef _D3DADAPTER9_DRM_H_
#define _D3DADAPTER9_DRM_H_

#include "d3dadapter9.h"

/* query driver support name */
#define D3DADAPTER9DRM_NAME "drm"
/* current version */
#define D3DADAPTER9DRM_MAJOR 0
#define D3DADAPTER9DRM_MINOR 2

/* version 0.0: Initial release
 *         0.1: All IDirect3D objects can be assumed to have a pointer to the
 *              internal vtable in second position of the structure
 *         0.2: IDirect3DDevice9_SetCursorPosition always calls
 *              ID3DPresent_SetCursorPos for hardware cursors
 */

struct D3DAdapter9DRM
{
    unsigned major_version; /* ABI break */
    unsigned minor_version; /* backwards compatible feature additions */

    /* NOTE: upon passing an fd to this function, it's now owned by this
        function. If this function fails, the fd will be closed here as well */
    HRESULT (WINAPI *create_adapter)(int fd, ID3DAdapter9 **ppAdapter);
};

#endif /* _D3DADAPTER9_DRM_H_ */
