/*
 * Mesa 3-D graphics library
 *
 * Copyright (C) 2016 Ilia Mirkin.  All Rights Reserved.
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


/**
 * \file bbox.c
 * glPrimitiveBoundingBox function
 */

#include "bbox.h"
#include "context.h"
#include "api_exec_decl.h"

void GLAPIENTRY
_mesa_PrimitiveBoundingBox(
      GLfloat minX, GLfloat minY, GLfloat minZ, GLfloat minW,
      GLfloat maxX, GLfloat maxY, GLfloat maxZ, GLfloat maxW)
{
   GET_CURRENT_CONTEXT(ctx);

   ctx->PrimitiveBoundingBox[0] = minX;
   ctx->PrimitiveBoundingBox[1] = minY;
   ctx->PrimitiveBoundingBox[2] = minZ;
   ctx->PrimitiveBoundingBox[3] = minW;
   ctx->PrimitiveBoundingBox[4] = maxX;
   ctx->PrimitiveBoundingBox[5] = maxY;
   ctx->PrimitiveBoundingBox[6] = maxZ;
   ctx->PrimitiveBoundingBox[7] = maxW;
}

void
_mesa_init_bbox(struct gl_context *ctx)
{
   ctx->PrimitiveBoundingBox[0] =
   ctx->PrimitiveBoundingBox[1] =
   ctx->PrimitiveBoundingBox[2] = -1.0f;

   ctx->PrimitiveBoundingBox[3] =
   ctx->PrimitiveBoundingBox[4] =
   ctx->PrimitiveBoundingBox[5] =
   ctx->PrimitiveBoundingBox[6] =
   ctx->PrimitiveBoundingBox[7] = 1.0f;
}
