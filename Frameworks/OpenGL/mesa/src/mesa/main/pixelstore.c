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

/**
 * \file pixelstore.c
 * glPixelStore functions.
 */


#include "util/glheader.h"
#include "bufferobj.h"
#include "context.h"
#include "pixelstore.h"
#include "mtypes.h"
#include "util/rounding.h"
#include "api_exec_decl.h"


static ALWAYS_INLINE void
pixel_storei(GLenum pname, GLint param, bool no_error)
{
   /* NOTE: this call can't be compiled into the display list */
   GET_CURRENT_CONTEXT(ctx);

   switch (pname) {
      case GL_PACK_SWAP_BYTES:
         if (!no_error && !_mesa_is_desktop_gl(ctx))
            goto invalid_enum_error;
         ctx->Pack.SwapBytes = param ? GL_TRUE : GL_FALSE;
         break;
      case GL_PACK_LSB_FIRST:
         if (!no_error && !_mesa_is_desktop_gl(ctx))
            goto invalid_enum_error;
         ctx->Pack.LsbFirst = param ? GL_TRUE : GL_FALSE;
         break;
      case GL_PACK_ROW_LENGTH:
         if (!no_error && _mesa_is_gles1(ctx))
            goto invalid_enum_error;
         if (!no_error && param<0)
            goto invalid_value_error;
         ctx->Pack.RowLength = param;
         break;
      case GL_PACK_IMAGE_HEIGHT:
         if (!no_error && !_mesa_is_desktop_gl(ctx) && !_mesa_is_gles3(ctx))
            goto invalid_enum_error;
         if (!no_error && param<0)
            goto invalid_value_error;
         ctx->Pack.ImageHeight = param;
         break;
      case GL_PACK_SKIP_PIXELS:
         if (!no_error && _mesa_is_gles1(ctx))
            goto invalid_enum_error;
         if (!no_error && param<0)
            goto invalid_value_error;
         ctx->Pack.SkipPixels = param;
         break;
      case GL_PACK_SKIP_ROWS:
         if (!no_error && _mesa_is_gles1(ctx))
            goto invalid_enum_error;
         if (!no_error && param<0)
            goto invalid_value_error;
         ctx->Pack.SkipRows = param;
         break;
      case GL_PACK_SKIP_IMAGES:
         if (!no_error && !_mesa_is_desktop_gl(ctx) && !_mesa_is_gles3(ctx))
            goto invalid_enum_error;
         if (!no_error && param<0)
            goto invalid_value_error;
         ctx->Pack.SkipImages = param;
         break;
      case GL_PACK_ALIGNMENT:
         if (!no_error && param!=1 && param!=2 && param!=4 && param!=8)
            goto invalid_value_error;
         ctx->Pack.Alignment = param;
         break;
      case GL_PACK_INVERT_MESA:
         if (!no_error && !_mesa_has_MESA_pack_invert(ctx))
            goto invalid_enum_error;
         ctx->Pack.Invert = param;
         break;
      case GL_PACK_REVERSE_ROW_ORDER_ANGLE:
         if (!no_error && !_mesa_has_ANGLE_pack_reverse_row_order(ctx))
            goto invalid_enum_error;
         ctx->Pack.Invert = param;
         break;
      case GL_PACK_COMPRESSED_BLOCK_WIDTH:
         if (!no_error && !_mesa_is_desktop_gl(ctx))
            goto invalid_enum_error;
         if (!no_error && param<0)
            goto invalid_value_error;
         ctx->Pack.CompressedBlockWidth = param;
         break;
      case GL_PACK_COMPRESSED_BLOCK_HEIGHT:
         if (!no_error && !_mesa_is_desktop_gl(ctx))
            goto invalid_enum_error;
         if (!no_error && param<0)
            goto invalid_value_error;
         ctx->Pack.CompressedBlockHeight = param;
         break;
      case GL_PACK_COMPRESSED_BLOCK_DEPTH:
         if (!no_error && !_mesa_is_desktop_gl(ctx))
            goto invalid_enum_error;
         if (!no_error && param<0)
            goto invalid_value_error;
         ctx->Pack.CompressedBlockDepth = param;
         break;
      case GL_PACK_COMPRESSED_BLOCK_SIZE:
         if (!no_error && !_mesa_is_desktop_gl(ctx))
            goto invalid_enum_error;
         if (!no_error && param<0)
            goto invalid_value_error;
         ctx->Pack.CompressedBlockSize = param;
         break;

      case GL_UNPACK_SWAP_BYTES:
         if (!no_error && !_mesa_is_desktop_gl(ctx))
            goto invalid_enum_error;
         ctx->Unpack.SwapBytes = param ? GL_TRUE : GL_FALSE;
         break;
      case GL_UNPACK_LSB_FIRST:
         if (!no_error && !_mesa_is_desktop_gl(ctx))
            goto invalid_enum_error;
         ctx->Unpack.LsbFirst = param ? GL_TRUE : GL_FALSE;
         break;
      case GL_UNPACK_ROW_LENGTH:
         if (!no_error && _mesa_is_gles1(ctx))
            goto invalid_enum_error;
         if (!no_error && param<0)
            goto invalid_value_error;
         ctx->Unpack.RowLength = param;
         break;
      case GL_UNPACK_IMAGE_HEIGHT:
         if (!no_error && !_mesa_is_desktop_gl(ctx) && !_mesa_is_gles3(ctx))
            goto invalid_enum_error;
         if (!no_error && param<0)
            goto invalid_value_error;
         ctx->Unpack.ImageHeight = param;
         break;
      case GL_UNPACK_SKIP_PIXELS:
         if (!no_error && _mesa_is_gles1(ctx))
            goto invalid_enum_error;
         if (!no_error && param<0)
            goto invalid_value_error;
         ctx->Unpack.SkipPixels = param;
         break;
      case GL_UNPACK_SKIP_ROWS:
         if (!no_error && _mesa_is_gles1(ctx))
            goto invalid_enum_error;
         if (!no_error && param<0)
            goto invalid_value_error;
         ctx->Unpack.SkipRows = param;
         break;
      case GL_UNPACK_SKIP_IMAGES:
         if (!no_error && !_mesa_is_desktop_gl(ctx) && !_mesa_is_gles3(ctx))
            goto invalid_enum_error;
         if (!no_error && param < 0)
            goto invalid_value_error;
         ctx->Unpack.SkipImages = param;
         break;
      case GL_UNPACK_ALIGNMENT:
         if (!no_error && param!=1 && param!=2 && param!=4 && param!=8)
            goto invalid_value_error;
         ctx->Unpack.Alignment = param;
         break;
      case GL_UNPACK_COMPRESSED_BLOCK_WIDTH:
         if (!no_error && !_mesa_is_desktop_gl(ctx))
            goto invalid_enum_error;
         if (!no_error && param<0)
            goto invalid_value_error;
         ctx->Unpack.CompressedBlockWidth = param;
         break;
      case GL_UNPACK_COMPRESSED_BLOCK_HEIGHT:
         if (!no_error && !_mesa_is_desktop_gl(ctx))
            goto invalid_enum_error;
         if (!no_error && param<0)
            goto invalid_value_error;
         ctx->Unpack.CompressedBlockHeight = param;
         break;
      case GL_UNPACK_COMPRESSED_BLOCK_DEPTH:
         if (!no_error && !_mesa_is_desktop_gl(ctx))
            goto invalid_enum_error;
         if (!no_error && param<0)
            goto invalid_value_error;
         ctx->Unpack.CompressedBlockDepth = param;
         break;
      case GL_UNPACK_COMPRESSED_BLOCK_SIZE:
         if (!no_error && !_mesa_is_desktop_gl(ctx))
            goto invalid_enum_error;
         if (!no_error && param<0)
            goto invalid_value_error;
         ctx->Unpack.CompressedBlockSize = param;
         break;
      default:
         if (!no_error)
            goto invalid_enum_error;
         else
            unreachable("invalid pixel store enum");
   }

   return;

invalid_enum_error:
   _mesa_error(ctx, GL_INVALID_ENUM, "glPixelStore");
   return;

invalid_value_error:
   _mesa_error(ctx, GL_INVALID_VALUE, "glPixelStore(param)");
   return;
}


void GLAPIENTRY
_mesa_PixelStorei(GLenum pname, GLint param)
{
   pixel_storei(pname, param, false);
}


void GLAPIENTRY
_mesa_PixelStoref(GLenum pname, GLfloat param)
{
   _mesa_PixelStorei(pname, lroundf(param));
}


void GLAPIENTRY
_mesa_PixelStorei_no_error(GLenum pname, GLint param)
{
   pixel_storei(pname, param, true);
}


void GLAPIENTRY
_mesa_PixelStoref_no_error(GLenum pname, GLfloat param)
{
   _mesa_PixelStorei_no_error(pname, lroundf(param));
}


/**
 * Initialize the context's pixel store state.
 */
void
_mesa_init_pixelstore(struct gl_context *ctx)
{
   /* Pixel transfer */
   ctx->Pack.Alignment = 4;
   ctx->Pack.RowLength = 0;
   ctx->Pack.ImageHeight = 0;
   ctx->Pack.SkipPixels = 0;
   ctx->Pack.SkipRows = 0;
   ctx->Pack.SkipImages = 0;
   ctx->Pack.SwapBytes = GL_FALSE;
   ctx->Pack.LsbFirst = GL_FALSE;
   ctx->Pack.Invert = GL_FALSE;
   ctx->Pack.CompressedBlockWidth = 0;
   ctx->Pack.CompressedBlockHeight = 0;
   ctx->Pack.CompressedBlockDepth = 0;
   ctx->Pack.CompressedBlockSize = 0;
   _mesa_reference_buffer_object(ctx, &ctx->Pack.BufferObj, NULL);
   ctx->Unpack.Alignment = 4;
   ctx->Unpack.RowLength = 0;
   ctx->Unpack.ImageHeight = 0;
   ctx->Unpack.SkipPixels = 0;
   ctx->Unpack.SkipRows = 0;
   ctx->Unpack.SkipImages = 0;
   ctx->Unpack.SwapBytes = GL_FALSE;
   ctx->Unpack.LsbFirst = GL_FALSE;
   ctx->Unpack.Invert = GL_FALSE;
   ctx->Unpack.CompressedBlockWidth = 0;
   ctx->Unpack.CompressedBlockHeight = 0;
   ctx->Unpack.CompressedBlockDepth = 0;
   ctx->Unpack.CompressedBlockSize = 0;
   _mesa_reference_buffer_object(ctx, &ctx->Unpack.BufferObj, NULL);

   /*
    * _mesa_unpack_image() returns image data in this format.  When we
    * execute image commands (glDrawPixels(), glTexImage(), etc) from
    * within display lists we have to be sure to set the current
    * unpacking parameters to these values!
    */
   ctx->DefaultPacking.Alignment = 1;
   ctx->DefaultPacking.RowLength = 0;
   ctx->DefaultPacking.SkipPixels = 0;
   ctx->DefaultPacking.SkipRows = 0;
   ctx->DefaultPacking.ImageHeight = 0;
   ctx->DefaultPacking.SkipImages = 0;
   ctx->DefaultPacking.SwapBytes = GL_FALSE;
   ctx->DefaultPacking.LsbFirst = GL_FALSE;
   ctx->DefaultPacking.Invert = GL_FALSE;
   _mesa_reference_buffer_object(ctx, &ctx->DefaultPacking.BufferObj, NULL);
}


/**
 * Check if the given compressed pixel storage parameters are legal.
 * Record a GL error if illegal.
 * \return  true if legal, false if illegal
 */
bool
_mesa_compressed_pixel_storage_error_check(
   struct gl_context *ctx,
   GLint dimensions,
   const struct gl_pixelstore_attrib *packing,
   const char *caller)
{
   if (!_mesa_is_desktop_gl(ctx) || !packing->CompressedBlockSize)
      return true;

   if (packing->CompressedBlockWidth &&
       packing->SkipPixels % packing->CompressedBlockWidth) {
      _mesa_error(ctx, GL_INVALID_OPERATION,
                  "%s(skip-pixels %% block-width)", caller);
      return false;
   }

   if (dimensions > 1 &&
       packing->CompressedBlockHeight &&
       packing->SkipRows % packing->CompressedBlockHeight) {
      _mesa_error(ctx, GL_INVALID_OPERATION,
                  "%s(skip-rows %% block-height)", caller);
      return false;
   }

   if (dimensions > 2 &&
       packing->CompressedBlockDepth &&
       packing->SkipImages % packing->CompressedBlockDepth) {
      _mesa_error(ctx, GL_INVALID_OPERATION,
                  "%s(skip-images %% block-depth)", caller);
      return false;
   }

   return true;
}
