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
 * Functions for allocating/managing framebuffers and renderbuffers.
 * Also, routines for reading/writing renderbuffer data as ubytes,
 * ushorts, uints, etc.
 */

#include <stdio.h>
#include "util/glheader.h"

#include "blend.h"
#include "buffers.h"
#include "context.h"
#include "draw_validate.h"
#include "enums.h"
#include "formats.h"
#include "macros.h"
#include "mtypes.h"
#include "fbobject.h"
#include "framebuffer.h"
#include "renderbuffer.h"
#include "texobj.h"
#include "glformats.h"
#include "state.h"
#include "util/u_memory.h"

#include "state_tracker/st_manager.h"

/**
 * Compute/set the _DepthMax field for the given framebuffer.
 * This value depends on the Z buffer resolution.
 */
static void
compute_depth_max(struct gl_framebuffer *fb)
{
   if (fb->Visual.depthBits == 0) {
      /* Special case.  Even if we don't have a depth buffer we need
       * good values for DepthMax for Z vertex transformation purposes
       * and for per-fragment fog computation.
       */
      fb->_DepthMax = (1 << 16) - 1;
   }
   else if (fb->Visual.depthBits < 32) {
      fb->_DepthMax = (1 << fb->Visual.depthBits) - 1;
   }
   else {
      /* Special case since shift values greater than or equal to the
       * number of bits in the left hand expression's type are undefined.
       */
      fb->_DepthMax = 0xffffffff;
   }
   fb->_DepthMaxF = (GLfloat) fb->_DepthMax;

   /* Minimum resolvable depth value, for polygon offset */
   fb->_MRD = (GLfloat)1.0 / fb->_DepthMaxF;
}

/**
 * Allocate a new gl_framebuffer object.
 * This is the default function for ctx->Driver.NewFramebuffer().
 * This is for allocating user-created framebuffers, not window-system
 * framebuffers!
 */
struct gl_framebuffer *
_mesa_new_framebuffer(struct gl_context *ctx, GLuint name)
{
   struct gl_framebuffer *fb;
   (void) ctx;
   assert(name != 0);
   fb = CALLOC_STRUCT(gl_framebuffer);
   if (fb) {
      _mesa_initialize_user_framebuffer(fb, name);
   }
   return fb;
}


/**
 * Initialize a gl_framebuffer object.  Typically used to initialize
 * window system-created framebuffers, not user-created framebuffers.
 * \sa _mesa_initialize_user_framebuffer
 */
void
_mesa_initialize_window_framebuffer(struct gl_framebuffer *fb,
				     const struct gl_config *visual)
{
   assert(fb);
   assert(visual);

   memset(fb, 0, sizeof(struct gl_framebuffer));

   simple_mtx_init(&fb->Mutex, mtx_plain);

   fb->RefCount = 1;

   /* save the visual */
   fb->Visual = *visual;

   /* Init read/draw renderbuffer state */
   if (visual->doubleBufferMode) {
      fb->_NumColorDrawBuffers = 1;
      fb->ColorDrawBuffer[0] = GL_BACK;
      fb->_ColorDrawBufferIndexes[0] = BUFFER_BACK_LEFT;
      fb->ColorReadBuffer = GL_BACK;
      fb->_ColorReadBufferIndex = BUFFER_BACK_LEFT;
   }
   else {
      fb->_NumColorDrawBuffers = 1;
      fb->ColorDrawBuffer[0] = GL_FRONT;
      fb->_ColorDrawBufferIndexes[0] = BUFFER_FRONT_LEFT;
      fb->ColorReadBuffer = GL_FRONT;
      fb->_ColorReadBufferIndex = BUFFER_FRONT_LEFT;
   }

   fb->Delete = _mesa_destroy_framebuffer;
   fb->_Status = GL_FRAMEBUFFER_COMPLETE_EXT;
   fb->_AllColorBuffersFixedPoint = !visual->floatMode;
   fb->_HasSNormOrFloatColorBuffer = visual->floatMode;
   fb->_HasAttachments = true;
   fb->FlipY = true;

   fb->SampleLocationTable = NULL;
   fb->ProgrammableSampleLocations = 0;
   fb->SampleLocationPixelGrid = 0;

   compute_depth_max(fb);
}


/**
 * Initialize a user-created gl_framebuffer object.
 * \sa _mesa_initialize_window_framebuffer
 */
void
_mesa_initialize_user_framebuffer(struct gl_framebuffer *fb, GLuint name)
{
   assert(fb);
   assert(name);

   memset(fb, 0, sizeof(struct gl_framebuffer));

   fb->Name = name;
   fb->RefCount = 1;
   fb->_NumColorDrawBuffers = 1;
   fb->ColorDrawBuffer[0] = GL_COLOR_ATTACHMENT0_EXT;
   fb->_ColorDrawBufferIndexes[0] = BUFFER_COLOR0;
   fb->ColorReadBuffer = GL_COLOR_ATTACHMENT0_EXT;
   fb->_ColorReadBufferIndex = BUFFER_COLOR0;
   fb->SampleLocationTable = NULL;
   fb->ProgrammableSampleLocations = 0;
   fb->SampleLocationPixelGrid = 0;
   fb->Delete = _mesa_destroy_framebuffer;
   simple_mtx_init(&fb->Mutex, mtx_plain);
}


/**
 * Deallocate buffer and everything attached to it.
 * Typically called via the gl_framebuffer->Delete() method.
 */
void
_mesa_destroy_framebuffer(struct gl_framebuffer *fb)
{
   if (fb) {
      _mesa_free_framebuffer_data(fb);
      free(fb->Label);
      FREE(fb);
   }
}


/**
 * Free all the data hanging off the given gl_framebuffer, but don't free
 * the gl_framebuffer object itself.
 */
void
_mesa_free_framebuffer_data(struct gl_framebuffer *fb)
{
   assert(fb);
   assert(fb->RefCount == 0);

   pipe_resource_reference(&fb->resolve, NULL);

   simple_mtx_destroy(&fb->Mutex);

   for (unsigned i = 0; i < BUFFER_COUNT; i++) {
      struct gl_renderbuffer_attachment *att = &fb->Attachment[i];
      if (att->Renderbuffer) {
         _mesa_reference_renderbuffer(&att->Renderbuffer, NULL);
      }
      if (att->Texture) {
         _mesa_reference_texobj(&att->Texture, NULL);
      }
      assert(!att->Renderbuffer);
      assert(!att->Texture);
      att->Type = GL_NONE;
   }

   free(fb->SampleLocationTable);
   fb->SampleLocationTable = NULL;
}


/**
 * Set *ptr to point to fb, with refcounting and locking.
 * This is normally only called from the _mesa_reference_framebuffer() macro
 * when there's a real pointer change.
 */
void
_mesa_reference_framebuffer_(struct gl_framebuffer **ptr,
                             struct gl_framebuffer *fb)
{
   if (*ptr) {
      /* unreference old renderbuffer */
      GLboolean deleteFlag = GL_FALSE;
      struct gl_framebuffer *oldFb = *ptr;

      simple_mtx_lock(&oldFb->Mutex);
      assert(oldFb->RefCount > 0);
      oldFb->RefCount--;
      deleteFlag = (oldFb->RefCount == 0);
      simple_mtx_unlock(&oldFb->Mutex);

      if (deleteFlag)
         oldFb->Delete(oldFb);

      *ptr = NULL;
   }

   if (fb) {
      simple_mtx_lock(&fb->Mutex);
      fb->RefCount++;
      simple_mtx_unlock(&fb->Mutex);
      *ptr = fb;
   }
}


/**
 * Resize the given framebuffer's renderbuffers to the new width and height.
 * This should only be used for window-system framebuffers, not
 * user-created renderbuffers (i.e. made with GL_EXT_framebuffer_object).
 * This will typically be called directly from a device driver.
 *
 * \note it's possible for ctx to be null since a window can be resized
 * without a currently bound rendering context.
 */
void
_mesa_resize_framebuffer(struct gl_context *ctx, struct gl_framebuffer *fb,
                         GLuint width, GLuint height)
{
   /* XXX I think we could check if the size is not changing
    * and return early.
    */

   /* Can only resize win-sys framebuffer objects */
   assert(_mesa_is_winsys_fbo(fb));

   for (unsigned i = 0; i < BUFFER_COUNT; i++) {
      struct gl_renderbuffer_attachment *att = &fb->Attachment[i];
      if (att->Type == GL_RENDERBUFFER_EXT && att->Renderbuffer) {
         struct gl_renderbuffer *rb = att->Renderbuffer;
         /* only resize if size is changing */
         if (rb->Width != width || rb->Height != height) {
            if (rb->AllocStorage(ctx, rb, rb->InternalFormat, width, height)) {
               assert(rb->Width == width);
               assert(rb->Height == height);
            }
            else {
               _mesa_error(ctx, GL_OUT_OF_MEMORY, "Resizing framebuffer");
               /* no return */
            }
         }
      }
   }

   fb->Width = width;
   fb->Height = height;

   if (ctx) {
      /* update scissor / window bounds */
      _mesa_update_draw_buffer_bounds(ctx, ctx->DrawBuffer);
      /* Signal new buffer state so that swrast will update its clipping
       * info (the CLIP_BIT flag).
       */
      ctx->NewState |= _NEW_BUFFERS;
   }
}

/**
 * Given a bounding box, intersect the bounding box with the scissor of
 * a specified vieport.
 *
 * \param ctx     GL context.
 * \param idx     Index of the desired viewport
 * \param bbox    Bounding box for the scissored viewport.  Stored as xmin,
 *                xmax, ymin, ymax.
 */
void
_mesa_intersect_scissor_bounding_box(const struct gl_context *ctx,
                                     unsigned idx, int *bbox)
{
   if (ctx->Scissor.EnableFlags & (1u << idx)) {
      if (ctx->Scissor.ScissorArray[idx].X > bbox[0]) {
         bbox[0] = ctx->Scissor.ScissorArray[idx].X;
      }
      if (ctx->Scissor.ScissorArray[idx].Y > bbox[2]) {
         bbox[2] = ctx->Scissor.ScissorArray[idx].Y;
      }
      if (ctx->Scissor.ScissorArray[idx].X + ctx->Scissor.ScissorArray[idx].Width < bbox[1]) {
         bbox[1] = ctx->Scissor.ScissorArray[idx].X + ctx->Scissor.ScissorArray[idx].Width;
      }
      if (ctx->Scissor.ScissorArray[idx].Y + ctx->Scissor.ScissorArray[idx].Height < bbox[3]) {
         bbox[3] = ctx->Scissor.ScissorArray[idx].Y + ctx->Scissor.ScissorArray[idx].Height;
      }
      /* finally, check for empty region */
      if (bbox[0] > bbox[1]) {
         bbox[0] = bbox[1];
      }
      if (bbox[2] > bbox[3]) {
         bbox[2] = bbox[3];
      }
   }
}

/**
 * Calculate the inclusive bounding box for the scissor of a specific viewport
 *
 * \param ctx     GL context.
 * \param buffer  Framebuffer to be checked against
 * \param idx     Index of the desired viewport
 * \param bbox    Bounding box for the scissored viewport.  Stored as xmin,
 *                xmax, ymin, ymax.
 *
 * \warning This function assumes that the framebuffer dimensions are up to
 * date.
 *
 * \sa _mesa_clip_to_region
 */
static void
scissor_bounding_box(const struct gl_context *ctx,
                     const struct gl_framebuffer *buffer,
                     unsigned idx, int *bbox)
{
   bbox[0] = 0;
   bbox[2] = 0;
   bbox[1] = buffer->Width;
   bbox[3] = buffer->Height;

   _mesa_intersect_scissor_bounding_box(ctx, idx, bbox);

   assert(bbox[0] <= bbox[1]);
   assert(bbox[2] <= bbox[3]);
}

/**
 * Update the context's current drawing buffer's Xmin, Xmax, Ymin, Ymax fields.
 * These values are computed from the buffer's width and height and
 * the scissor box, if it's enabled.
 * \param ctx  the GL context.
 */
void
_mesa_update_draw_buffer_bounds(struct gl_context *ctx,
                                struct gl_framebuffer *buffer)
{
   int bbox[4];

   if (!buffer)
      return;

   /* Default to the first scissor as that's always valid */
   scissor_bounding_box(ctx, buffer, 0, bbox);
   buffer->_Xmin = bbox[0];
   buffer->_Ymin = bbox[2];
   buffer->_Xmax = bbox[1];
   buffer->_Ymax = bbox[3];
}


/**
 * The glGet queries of the framebuffer red/green/blue size, stencil size,
 * etc. are satisfied by the fields of ctx->DrawBuffer->Visual.  These can
 * change depending on the renderbuffer bindings.  This function updates
 * the given framebuffer's Visual from the current renderbuffer bindings.
 *
 * This may apply to user-created framebuffers or window system framebuffers.
 *
 * Also note: ctx->DrawBuffer->Visual.depthBits might not equal
 * ctx->DrawBuffer->Attachment[BUFFER_DEPTH].Renderbuffer.DepthBits.
 * The former one is used to convert floating point depth values into
 * integer Z values.
 */
void
_mesa_update_framebuffer_visual(struct gl_context *ctx,
				struct gl_framebuffer *fb)
{
   memset(&fb->Visual, 0, sizeof(fb->Visual));

   /* find first RGB renderbuffer */
   for (unsigned i = 0; i < BUFFER_COUNT; i++) {
      if (fb->Attachment[i].Renderbuffer) {
         const struct gl_renderbuffer_attachment *att = &fb->Attachment[i];
         const struct gl_renderbuffer *rb = att->Renderbuffer;
         const GLenum baseFormat = _mesa_get_format_base_format(rb->Format);
         const mesa_format fmt = rb->Format;

         /* Grab samples and sampleBuffers from any attachment point (assuming
          * the framebuffer is complete, we'll get the same answer from all
          * attachments). If using EXT_multisampled_render_to_texture, the
          * number of samples will be on fb->Attachment[i].NumSamples instead
          * of the usual rb->NumSamples, but it's still guarantted to be the
          * same for every attachment.
          *
          * From EXT_multisampled_render_to_texture:
          *
          *    Also, FBOs cannot combine attachments that have associated
          *    multisample data specified by the mechanisms described in this
          *    extension with attachments allocated using the core OpenGL ES
          *    3.1 mechanisms, such as TexStorage2DMultisample. Add to section
          *    9.4.2 "Whole Framebuffer Completeness":
          *
          *    "* If the value of RENDERBUFFER_SAMPLES is non-zero, all or
          *       none of the attached renderbuffers have been allocated
          *       using RenderbufferStorage- MultisampleEXT; if the value of
          *       TEXTURES_SAMPLES is non-zero, all or none of the attached
          *       textures have been attached using Framebuffer-
          *       Texture2DMultisampleEXT.
          *       { GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE_EXT }"
          */
         fb->Visual.samples =
            att->NumSamples ? att->NumSamples : rb->NumSamples;

         if (_mesa_is_legal_color_format(ctx, baseFormat)) {
            fb->Visual.redBits = _mesa_get_format_bits(fmt, GL_RED_BITS);
            fb->Visual.greenBits = _mesa_get_format_bits(fmt, GL_GREEN_BITS);
            fb->Visual.blueBits = _mesa_get_format_bits(fmt, GL_BLUE_BITS);
            fb->Visual.alphaBits = _mesa_get_format_bits(fmt, GL_ALPHA_BITS);
            fb->Visual.rgbBits = fb->Visual.redBits + fb->Visual.greenBits +
                                 fb->Visual.blueBits + fb->Visual.alphaBits;
            if (_mesa_is_format_srgb(fmt))
                fb->Visual.sRGBCapable = ctx->Extensions.EXT_sRGB;
            break;
         }
      }
   }

   fb->Visual.floatMode = GL_FALSE;
   for (unsigned i = 0; i < BUFFER_COUNT; i++) {
      if (i == BUFFER_DEPTH)
         continue;
      if (fb->Attachment[i].Renderbuffer) {
         const struct gl_renderbuffer *rb = fb->Attachment[i].Renderbuffer;
         const mesa_format fmt = rb->Format;

         if (_mesa_get_format_datatype(fmt) == GL_FLOAT) {
            fb->Visual.floatMode = GL_TRUE;
            break;
         }
      }
   }

   if (fb->Attachment[BUFFER_DEPTH].Renderbuffer) {
      const struct gl_renderbuffer *rb =
         fb->Attachment[BUFFER_DEPTH].Renderbuffer;
      const mesa_format fmt = rb->Format;
      fb->Visual.depthBits = _mesa_get_format_bits(fmt, GL_DEPTH_BITS);
   }

   if (fb->Attachment[BUFFER_STENCIL].Renderbuffer) {
      const struct gl_renderbuffer *rb =
         fb->Attachment[BUFFER_STENCIL].Renderbuffer;
      const mesa_format fmt = rb->Format;
      fb->Visual.stencilBits = _mesa_get_format_bits(fmt, GL_STENCIL_BITS);
   }

   if (fb->Attachment[BUFFER_ACCUM].Renderbuffer) {
      const struct gl_renderbuffer *rb =
         fb->Attachment[BUFFER_ACCUM].Renderbuffer;
      const mesa_format fmt = rb->Format;
      fb->Visual.accumRedBits = _mesa_get_format_bits(fmt, GL_RED_BITS);
      fb->Visual.accumGreenBits = _mesa_get_format_bits(fmt, GL_GREEN_BITS);
      fb->Visual.accumBlueBits = _mesa_get_format_bits(fmt, GL_BLUE_BITS);
      fb->Visual.accumAlphaBits = _mesa_get_format_bits(fmt, GL_ALPHA_BITS);
   }

   compute_depth_max(fb);
   _mesa_update_allow_draw_out_of_order(ctx);
   _mesa_update_valid_to_render_state(ctx);
}


/*
 * Example DrawBuffers scenarios:
 *
 * 1. glDrawBuffer(GL_FRONT_AND_BACK), fixed-func or shader writes to
 * "gl_FragColor" or program writes to the "result.color" register:
 *
 *   fragment color output   renderbuffer
 *   ---------------------   ---------------
 *   color[0]                Front, Back
 *
 *
 * 2. glDrawBuffers(3, [GL_FRONT, GL_AUX0, GL_AUX1]), shader writes to
 * gl_FragData[i] or program writes to result.color[i] registers:
 *
 *   fragment color output   renderbuffer
 *   ---------------------   ---------------
 *   color[0]                Front
 *   color[1]                Aux0
 *   color[3]                Aux1
 *
 *
 * 3. glDrawBuffers(3, [GL_FRONT, GL_AUX0, GL_AUX1]) and shader writes to
 * gl_FragColor, or fixed function:
 *
 *   fragment color output   renderbuffer
 *   ---------------------   ---------------
 *   color[0]                Front, Aux0, Aux1
 *
 *
 * In either case, the list of renderbuffers is stored in the
 * framebuffer->_ColorDrawBuffers[] array and
 * framebuffer->_NumColorDrawBuffers indicates the number of buffers.
 * The renderer (like swrast) has to look at the current fragment shader
 * to see if it writes to gl_FragColor vs. gl_FragData[i] to determine
 * how to map color outputs to renderbuffers.
 *
 * Note that these two calls are equivalent (for fixed function fragment
 * shading anyway):
 *   a)  glDrawBuffer(GL_FRONT_AND_BACK);  (assuming non-stereo framebuffer)
 *   b)  glDrawBuffers(2, [GL_FRONT_LEFT, GL_BACK_LEFT]);
 */




/**
 * Update the (derived) list of color drawing renderbuffer pointers.
 * Later, when we're rendering we'll loop from 0 to _NumColorDrawBuffers
 * writing colors.
 */
static void
update_color_draw_buffers(struct gl_framebuffer *fb)
{
   GLuint output;

   /* set 0th buffer to NULL now in case _NumColorDrawBuffers is zero */
   fb->_ColorDrawBuffers[0] = NULL;

   for (output = 0; output < fb->_NumColorDrawBuffers; output++) {
      gl_buffer_index buf = fb->_ColorDrawBufferIndexes[output];
      if (buf != BUFFER_NONE) {
         fb->_ColorDrawBuffers[output] = fb->Attachment[buf].Renderbuffer;
      }
      else {
         fb->_ColorDrawBuffers[output] = NULL;
      }
   }
}


/**
 * Update the (derived) color read renderbuffer pointer.
 * Unlike the DrawBuffer, we can only read from one (or zero) color buffers.
 */
static void
update_color_read_buffer(struct gl_framebuffer *fb)
{
   if (fb->_ColorReadBufferIndex == BUFFER_NONE ||
       fb->DeletePending ||
       fb->Width == 0 ||
       fb->Height == 0) {
      fb->_ColorReadBuffer = NULL; /* legal! */
   }
   else {
      assert(fb->_ColorReadBufferIndex >= 0);
      assert(fb->_ColorReadBufferIndex < BUFFER_COUNT);
      fb->_ColorReadBuffer
         = fb->Attachment[fb->_ColorReadBufferIndex].Renderbuffer;
   }
}

/**
 * Called via glDrawBuffer.  We only provide this driver function so that we
 * can check if we need to allocate a new renderbuffer.  Specifically, we
 * don't usually allocate a front color buffer when using a double-buffered
 * visual.  But if the app calls glDrawBuffer(GL_FRONT) we need to allocate
 * that buffer.  Note, this is only for window system buffers, not user-
 * created FBOs.
 */
void
_mesa_draw_buffer_allocate(struct gl_context *ctx)
{
   struct gl_framebuffer *fb = ctx->DrawBuffer;
   assert(_mesa_is_winsys_fbo(fb));
   GLuint i;
   /* add the renderbuffers on demand */
   for (i = 0; i < fb->_NumColorDrawBuffers; i++) {
      gl_buffer_index idx = fb->_ColorDrawBufferIndexes[i];

      if (idx != BUFFER_NONE) {
         st_manager_add_color_renderbuffer(ctx, fb, idx);
      }
   }
}

/**
 * Update a gl_framebuffer's derived state.
 *
 * Specifically, update these framebuffer fields:
 *    _ColorDrawBuffers
 *    _NumColorDrawBuffers
 *    _ColorReadBuffer
 *
 * If the framebuffer is user-created, make sure it's complete.
 *
 * The following functions (at least) can effect framebuffer state:
 * glReadBuffer, glDrawBuffer, glDrawBuffersARB, glFramebufferRenderbufferEXT,
 * glRenderbufferStorageEXT.
 */
static void
update_framebuffer(struct gl_context *ctx, struct gl_framebuffer *fb)
{
   if (_mesa_is_winsys_fbo(fb)) {
      /* This is a window-system framebuffer */
      /* Need to update the FB's GL_DRAW_BUFFER state to match the
       * context state (GL_READ_BUFFER too).
       */
      if (fb->ColorDrawBuffer[0] != ctx->Color.DrawBuffer[0]) {
         _mesa_drawbuffers(ctx, fb, ctx->Const.MaxDrawBuffers,
                           ctx->Color.DrawBuffer, NULL);
      }

      /* Call device driver function if fb is the bound draw buffer. */
      if (fb == ctx->DrawBuffer) {
         _mesa_draw_buffer_allocate(ctx);
      }
   }
   else {
      /* This is a user-created framebuffer.
       * Completeness only matters for user-created framebuffers.
       */
      if (fb->_Status != GL_FRAMEBUFFER_COMPLETE) {
         _mesa_test_framebuffer_completeness(ctx, fb);
      }
   }

   /* Strictly speaking, we don't need to update the draw-state
    * if this FB is bound as ctx->ReadBuffer (and conversely, the
    * read-state if this FB is bound as ctx->DrawBuffer), but no
    * harm.
    */
   update_color_draw_buffers(fb);
   update_color_read_buffer(fb);

   compute_depth_max(fb);
}


/**
 * Update state related to the draw/read framebuffers.
 */
void
_mesa_update_framebuffer(struct gl_context *ctx,
                         struct gl_framebuffer *readFb,
                         struct gl_framebuffer *drawFb)
{
   assert(ctx);

   update_framebuffer(ctx, drawFb);
   if (readFb != drawFb)
      update_framebuffer(ctx, readFb);

   _mesa_update_clamp_vertex_color(ctx, drawFb);
   _mesa_update_clamp_fragment_color(ctx, drawFb);
}


/**
 * Check if the renderbuffer for a read/draw operation exists.
 * \param format  a basic image format such as GL_RGB, GL_RGBA, GL_ALPHA,
 *                GL_DEPTH_COMPONENT, etc. or GL_COLOR, GL_DEPTH, GL_STENCIL.
 * \param reading  if TRUE, we're going to read from the buffer,
                   if FALSE, we're going to write to the buffer.
 * \return GL_TRUE if buffer exists, GL_FALSE otherwise
 */
static GLboolean
renderbuffer_exists(struct gl_context *ctx,
                    struct gl_framebuffer *fb,
                    GLenum format,
                    GLboolean reading)
{
   const struct gl_renderbuffer_attachment *att = fb->Attachment;

   /* If we don't know the framebuffer status, update it now */
   if (fb->_Status == 0) {
      _mesa_test_framebuffer_completeness(ctx, fb);
   }

   if (fb->_Status != GL_FRAMEBUFFER_COMPLETE_EXT) {
      return GL_FALSE;
   }

   switch (format) {
   case GL_COLOR:
   case GL_RED:
   case GL_GREEN:
   case GL_BLUE:
   case GL_ALPHA:
   case GL_LUMINANCE:
   case GL_LUMINANCE_ALPHA:
   case GL_INTENSITY:
   case GL_RG:
   case GL_RGB:
   case GL_BGR:
   case GL_RGBA:
   case GL_BGRA:
   case GL_ABGR_EXT:
   case GL_RED_INTEGER_EXT:
   case GL_RG_INTEGER:
   case GL_GREEN_INTEGER_EXT:
   case GL_BLUE_INTEGER_EXT:
   case GL_ALPHA_INTEGER_EXT:
   case GL_RGB_INTEGER_EXT:
   case GL_RGBA_INTEGER_EXT:
   case GL_BGR_INTEGER_EXT:
   case GL_BGRA_INTEGER_EXT:
   case GL_LUMINANCE_INTEGER_EXT:
   case GL_LUMINANCE_ALPHA_INTEGER_EXT:
      if (reading) {
         /* about to read from a color buffer */
         const struct gl_renderbuffer *readBuf = fb->_ColorReadBuffer;
         if (!readBuf) {
            return GL_FALSE;
         }
         assert(_mesa_get_format_bits(readBuf->Format, GL_RED_BITS) > 0 ||
                _mesa_get_format_bits(readBuf->Format, GL_ALPHA_BITS) > 0 ||
                _mesa_get_format_bits(readBuf->Format, GL_TEXTURE_LUMINANCE_SIZE) > 0 ||
                _mesa_get_format_bits(readBuf->Format, GL_TEXTURE_INTENSITY_SIZE) > 0 ||
                _mesa_get_format_bits(readBuf->Format, GL_INDEX_BITS) > 0);
      }
      else {
         /* about to draw to zero or more color buffers (none is OK) */
         return GL_TRUE;
      }
      break;
   case GL_DEPTH:
   case GL_DEPTH_COMPONENT:
      if (att[BUFFER_DEPTH].Type == GL_NONE) {
         return GL_FALSE;
      }
      break;
   case GL_STENCIL:
   case GL_STENCIL_INDEX:
      if (att[BUFFER_STENCIL].Type == GL_NONE) {
         return GL_FALSE;
      }
      break;
   case GL_DEPTH_STENCIL_EXT:
      if (att[BUFFER_DEPTH].Type == GL_NONE ||
          att[BUFFER_STENCIL].Type == GL_NONE) {
         return GL_FALSE;
      }
      break;
   case GL_DEPTH_STENCIL_TO_RGBA_NV:
   case GL_DEPTH_STENCIL_TO_BGRA_NV:
      if (att[BUFFER_DEPTH].Type == GL_NONE ||
          att[BUFFER_STENCIL].Type == GL_NONE) {
         return GL_FALSE;
      }
      break;
   default:
      _mesa_problem(ctx,
                    "Unexpected format 0x%x in renderbuffer_exists",
                    format);
      return GL_FALSE;
   }

   /* OK */
   return GL_TRUE;
}


/**
 * Check if the renderbuffer for a read operation (glReadPixels, glCopyPixels,
 * glCopyTex[Sub]Image, etc) exists.
 * \param format  a basic image format such as GL_RGB, GL_RGBA, GL_ALPHA,
 *                GL_DEPTH_COMPONENT, etc. or GL_COLOR, GL_DEPTH, GL_STENCIL.
 * \return GL_TRUE if buffer exists, GL_FALSE otherwise
 */
GLboolean
_mesa_source_buffer_exists(struct gl_context *ctx, GLenum format)
{
   return renderbuffer_exists(ctx, ctx->ReadBuffer, format, GL_TRUE);
}


/**
 * As above, but for drawing operations.
 */
GLboolean
_mesa_dest_buffer_exists(struct gl_context *ctx, GLenum format)
{
   return renderbuffer_exists(ctx, ctx->DrawBuffer, format, GL_FALSE);
}

extern bool
_mesa_has_rtt_samples(const struct gl_framebuffer *fb)
{
   /* If there are multiple attachments, all of them are guaranteed
    * to have the same sample count. */
   if (fb->_ColorReadBufferIndex) {
      assert(fb->Attachment[fb->_ColorReadBufferIndex].Type != GL_NONE);
      return fb->Attachment[fb->_ColorReadBufferIndex].NumSamples > 0;
   } else if (fb->Attachment[BUFFER_DEPTH].Type != GL_NONE) {
      return fb->Attachment[BUFFER_DEPTH].NumSamples > 0;
   } else if (fb->Attachment[BUFFER_STENCIL].Type != GL_NONE) {
      return fb->Attachment[BUFFER_STENCIL].NumSamples > 0;
   }

   return true;
}

/**
 * Used to answer the GL_IMPLEMENTATION_COLOR_READ_FORMAT_OES queries (using
 * GetIntegerv, GetFramebufferParameteriv, etc)
 *
 * If @fb is NULL, the method returns the value for the current bound
 * framebuffer.
 */
GLenum
_mesa_get_color_read_format(struct gl_context *ctx,
                            struct gl_framebuffer *fb,
                            const char *caller)
{
   if (ctx->NewState)
      _mesa_update_state(ctx);

   if (fb == NULL)
      fb = ctx->ReadBuffer;

   if (!fb || !fb->_ColorReadBuffer) {
      /*
       * From OpenGL 4.5 spec, section 18.2.2 "ReadPixels":
       *
       *    "An INVALID_OPERATION error is generated by GetIntegerv if pname
       *     is IMPLEMENTATION_COLOR_READ_FORMAT or IMPLEMENTATION_COLOR_-
       *     READ_TYPE and any of:
       *      * the read framebuffer is not framebuffer complete.
       *      * the read framebuffer is a framebuffer object, and the selected
       *        read buffer (see section 18.2.1) has no image attached.
       *      * the selected read buffer is NONE."
       *
       * There is not equivalent quote for GetFramebufferParameteriv or
       * GetNamedFramebufferParameteriv, but from section 9.2.3 "Framebuffer
       * Object Queries":
       *
       *    "Values of framebuffer-dependent state are identical to those that
       *     would be obtained were the framebuffer object bound and queried
       *     using the simple state queries in that table."
       *
       * Where "using the simple state queries" refer to use GetIntegerv. So
       * we will assume that on that situation the same error should be
       * triggered too.
       */
      _mesa_error(ctx, GL_INVALID_OPERATION,
                  "%s(GL_IMPLEMENTATION_COLOR_READ_FORMAT: no GL_READ_BUFFER)",
                  caller);
      return GL_NONE;
   }
   else {
      const mesa_format format = fb->_ColorReadBuffer->Format;

      switch (format) {
      case MESA_FORMAT_RGBA_UINT8:
         return GL_RGBA_INTEGER;
      case MESA_FORMAT_B8G8R8A8_UNORM:
         return GL_BGRA;
      case MESA_FORMAT_B5G6R5_UNORM:
      case MESA_FORMAT_R11G11B10_FLOAT:
         return GL_RGB;
      case MESA_FORMAT_RG_FLOAT32:
      case MESA_FORMAT_RG_FLOAT16:
      case MESA_FORMAT_RG_UNORM8:
         return GL_RG;
      case MESA_FORMAT_RG_SINT32:
      case MESA_FORMAT_RG_UINT32:
      case MESA_FORMAT_RG_SINT16:
      case MESA_FORMAT_RG_UINT16:
      case MESA_FORMAT_RG_SINT8:
      case MESA_FORMAT_RG_UINT8:
         return GL_RG_INTEGER;
      case MESA_FORMAT_R_FLOAT32:
      case MESA_FORMAT_R_FLOAT16:
      case MESA_FORMAT_R_UNORM16:
      case MESA_FORMAT_R_UNORM8:
      case MESA_FORMAT_R_SNORM16:
      case MESA_FORMAT_R_SNORM8:
         return GL_RED;
      case MESA_FORMAT_R_SINT32:
      case MESA_FORMAT_R_UINT32:
      case MESA_FORMAT_R_SINT16:
      case MESA_FORMAT_R_UINT16:
      case MESA_FORMAT_R_SINT8:
      case MESA_FORMAT_R_UINT8:
         return GL_RED_INTEGER;
      default:
         break;
      }

      if (_mesa_is_format_integer(format))
         return GL_RGBA_INTEGER;
      else
         return GL_RGBA;
   }
}


/**
 * Used to answer the GL_IMPLEMENTATION_COLOR_READ_TYPE_OES queries (using
 * GetIntegerv, GetFramebufferParameteriv, etc)
 *
 * If @fb is NULL, the method returns the value for the current bound
 * framebuffer.
 */
GLenum
_mesa_get_color_read_type(struct gl_context *ctx,
                          struct gl_framebuffer *fb,
                          const char *caller)
{
   if (ctx->NewState)
      _mesa_update_state(ctx);

   if (fb == NULL)
      fb = ctx->ReadBuffer;

   if (!fb || !fb->_ColorReadBuffer) {
      /*
       * See comment on _mesa_get_color_read_format
       */
      _mesa_error(ctx, GL_INVALID_OPERATION,
                  "%s(GL_IMPLEMENTATION_COLOR_READ_TYPE: no GL_READ_BUFFER)",
                  caller);
      return GL_NONE;
   }
   else {
      const mesa_format format = fb->_ColorReadBuffer->Format;
      GLenum data_type;
      GLuint comps;

      _mesa_uncompressed_format_to_type_and_comps(format, &data_type, &comps);

      return data_type;
   }
}


/**
 * Returns the read renderbuffer for the specified format.
 */
struct gl_renderbuffer *
_mesa_get_read_renderbuffer_for_format(const struct gl_context *ctx,
                                       GLenum format)
{
   const struct gl_framebuffer *rfb = ctx->ReadBuffer;

   if (_mesa_is_color_format(format)) {
      return rfb->Attachment[rfb->_ColorReadBufferIndex].Renderbuffer;
   } else if (_mesa_is_depth_format(format) ||
              _mesa_is_depthstencil_format(format)) {
      return rfb->Attachment[BUFFER_DEPTH].Renderbuffer;
   } else {
      return rfb->Attachment[BUFFER_STENCIL].Renderbuffer;
   }
}


/**
 * Print framebuffer info to stderr, for debugging.
 */
void
_mesa_print_framebuffer(const struct gl_framebuffer *fb)
{
   fprintf(stderr, "Mesa Framebuffer %u at %p\n", fb->Name, (void *) fb);
   fprintf(stderr, "  Size: %u x %u  Status: %s\n", fb->Width, fb->Height,
           _mesa_enum_to_string(fb->_Status));
   fprintf(stderr, "  Attachments:\n");

   for (unsigned i = 0; i < BUFFER_COUNT; i++) {
      const struct gl_renderbuffer_attachment *att = &fb->Attachment[i];
      if (att->Type == GL_TEXTURE) {
         const struct gl_texture_image *texImage = att->Renderbuffer->TexImage;
         fprintf(stderr,
                 "  %2d: Texture %u, level %u, face %u, slice %u, complete %d\n",
                 i, att->Texture->Name, att->TextureLevel, att->CubeMapFace,
                 att->Zoffset, att->Complete);
         fprintf(stderr, "       Size: %u x %u x %u  Format %s\n",
                 texImage->Width, texImage->Height, texImage->Depth,
                 _mesa_get_format_name(texImage->TexFormat));
      }
      else if (att->Type == GL_RENDERBUFFER) {
         fprintf(stderr, "  %2d: Renderbuffer %u, complete %d\n",
                 i, att->Renderbuffer->Name, att->Complete);
         fprintf(stderr, "       Size: %u x %u  Format %s\n",
                 att->Renderbuffer->Width, att->Renderbuffer->Height,
                 _mesa_get_format_name(att->Renderbuffer->Format));
      }
      else {
         fprintf(stderr, "  %2d: none\n", i);
      }
   }
}

static inline GLuint
_mesa_geometric_nonvalidated_samples(const struct gl_framebuffer *buffer)
{
   return buffer->_HasAttachments ?
      buffer->Visual.samples :
      buffer->DefaultGeometry.NumSamples;
}

bool
_mesa_is_multisample_enabled(const struct gl_context *ctx)
{
   /* The sample count may not be validated by the driver, but when it is set,
    * we know that is in a valid range and no driver should ever validate a
    * multisampled framebuffer to non-multisampled and vice-versa.
    */
   return ctx->Multisample.Enabled &&
          ctx->DrawBuffer &&
          _mesa_geometric_nonvalidated_samples(ctx->DrawBuffer) >= 1;
}

/**
 * Is alpha testing enabled and applicable to the currently bound
 * framebuffer?
 */
bool
_mesa_is_alpha_test_enabled(const struct gl_context *ctx)
{
   bool buffer0_is_integer = ctx->DrawBuffer->_IntegerBuffers & 0x1;
   return (ctx->Color.AlphaEnabled && !buffer0_is_integer);
}
