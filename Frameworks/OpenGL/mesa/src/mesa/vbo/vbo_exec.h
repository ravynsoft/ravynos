/**************************************************************************

Copyright 2002 VMware, Inc.

All Rights Reserved.

Permission is hereby granted, free of charge, to any person obtaining a
copy of this software and associated documentation files (the "Software"),
to deal in the Software without restriction, including without limitation
on the rights to use, copy, modify, merge, publish, distribute, sub
license, and/or sell copies of the Software, and to permit persons to whom
the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice (including the next
paragraph) shall be included in all copies or substantial portions of the
Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT. IN NO EVENT SHALL
VMWARE AND/OR THEIR SUPPLIERS BE LIABLE FOR ANY CLAIM,
DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE
USE OR OTHER DEALINGS IN THE SOFTWARE.

**************************************************************************/

/*
 * Authors:
 *   Keith Whitwell <keithw@vmware.com>
 *
 */

#ifndef VBO_EXEC_H
#define VBO_EXEC_H

#include "main/dd.h"
#include "main/mesa_private.h"
#include "vbo.h"
#include "vbo_attrib.h"

void
vbo_exec_init(struct gl_context *ctx);

void
vbo_exec_destroy(struct gl_context *ctx);

void
vbo_exec_vtx_init(struct vbo_exec_context *exec);

void
vbo_exec_vtx_destroy(struct vbo_exec_context *exec);

void
vbo_exec_vtx_flush(struct vbo_exec_context *exec);

void
vbo_exec_vtx_map(struct vbo_exec_context *exec);

void
vbo_exec_eval_update(struct vbo_exec_context *exec);

void
vbo_exec_do_EvalCoord2f(struct vbo_exec_context *exec, GLfloat u, GLfloat v);

void
vbo_exec_do_EvalCoord1f(struct vbo_exec_context *exec, GLfloat u);

#endif
