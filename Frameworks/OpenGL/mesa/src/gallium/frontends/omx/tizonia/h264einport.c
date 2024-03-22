/**************************************************************************
 *
 * Copyright 2013 Advanced Micro Devices, Inc.
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
 * IN NO EVENT SHALL THE COPYRIGHT HOLDER(S) OR AUTHOR(S) BE LIABLE FOR
 * ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 **************************************************************************/

#include <assert.h>
#include <string.h>
#include <limits.h>

#include <tizplatform.h>
#include <tizkernel.h>

#include "vl/vl_winsys.h"

#include "h264e.h"
#include "h264einport.h"
#include "h264einport_decls.h"
#include "vid_enc_common.h"

static OMX_ERRORTYPE enc_AllocateBackTexture(OMX_HANDLETYPE ap_hdl,
                                             OMX_U32 idx,
                                             struct pipe_resource **resource,
                                             struct pipe_transfer **transfer,
                                             OMX_U8 **map)
{
   vid_enc_PrivateType * priv = tiz_get_prc(ap_hdl);
   tiz_port_t * port = tiz_krn_get_port(tiz_get_krn(ap_hdl), idx);
   struct pipe_resource buf_templ;
   struct pipe_box box = {};
   OMX_U8 *ptr;

   memset(&buf_templ, 0, sizeof buf_templ);
   buf_templ.target = PIPE_TEXTURE_2D;
   buf_templ.format = PIPE_FORMAT_I8_UNORM;
   buf_templ.bind = PIPE_BIND_LINEAR;
   buf_templ.usage = PIPE_USAGE_STAGING;
   buf_templ.flags = 0;
   buf_templ.width0 = port->portdef_.format.video.nFrameWidth;
   buf_templ.height0 = port->portdef_.format.video.nFrameHeight * 3 / 2;
   buf_templ.depth0 = 1;
   buf_templ.array_size = 1;

   *resource = priv->s_pipe->screen->resource_create(priv->s_pipe->screen, &buf_templ);
   if (!*resource)
      return OMX_ErrorInsufficientResources;

   box.width = (*resource)->width0;
   box.height = (*resource)->height0;
   box.depth = (*resource)->depth0;
   ptr = priv->s_pipe->texture_map(priv->s_pipe, *resource, 0, PIPE_MAP_WRITE, &box, transfer);
   if (map)
      *map = ptr;

   return OMX_ErrorNone;
}

/*
 * h264einport class
 */

static void * h264e_inport_ctor(void * ap_obj, va_list * app)
{
   return super_ctor(typeOf(ap_obj, "h264einport"), ap_obj, app);
}

static void * h264e_inport_dtor(void * ap_obj)
{
   return super_dtor(typeOf(ap_obj, "h264einport"), ap_obj);
}

/*
 * from tiz_api
 */

static OMX_ERRORTYPE h264e_inport_AllocateBuffer(const void * ap_obj, OMX_HANDLETYPE ap_hdl,
                                                 OMX_BUFFERHEADERTYPE ** buf, OMX_U32 idx,
                                                 OMX_PTR private, OMX_U32 size)
{
   struct input_buf_private *inp;
   OMX_ERRORTYPE r;

   r = super_UseBuffer(typeOf(ap_obj, "h264einport"), ap_obj, ap_hdl,
                       buf, idx, private, size, NULL);
   if (r)
     return r;

   inp = (*buf)->pInputPortPrivate = CALLOC_STRUCT(input_buf_private);
   if (!inp) {
     super_FreeBuffer(typeOf(ap_obj, "h264einport"), ap_obj, ap_hdl, idx, *buf);
     return OMX_ErrorInsufficientResources;
   }

   list_inithead(&inp->tasks);

   r = enc_AllocateBackTexture(ap_hdl, idx, &inp->resource, &inp->transfer, &(*buf)->pBuffer);

   if (r) {
     FREE(inp);
     super_FreeBuffer(typeOf(ap_obj, "h264einport"), ap_obj, ap_hdl, idx, *buf);
     return r;
   }

   return OMX_ErrorNone;
}

static OMX_ERRORTYPE h264e_inport_UseBuffer(const void * ap_obj, OMX_HANDLETYPE ap_hdl,
                                            OMX_BUFFERHEADERTYPE **buf, OMX_U32 idx,
                                            OMX_PTR private, OMX_U32 size, OMX_U8 *mem)
{
   struct input_buf_private *inp;
   OMX_ERRORTYPE r;

   r = super_UseBuffer(typeOf(ap_obj, "h264einport"), ap_obj, ap_hdl,
                       buf, idx, private, size, mem);
   if (r)
     return r;

   inp = (*buf)->pInputPortPrivate = CALLOC_STRUCT(input_buf_private);
   if (!inp) {
     super_FreeBuffer(typeOf(ap_obj, "h264einport"), ap_obj, ap_hdl, idx, *buf);
     return OMX_ErrorInsufficientResources;
   }

   list_inithead(&inp->tasks);

   return OMX_ErrorNone;
}

static OMX_ERRORTYPE h264e_inport_FreeBuffer(const void * ap_obj, OMX_HANDLETYPE ap_hdl,
                                             OMX_U32 idx, OMX_BUFFERHEADERTYPE *buf)
{
   vid_enc_PrivateType *priv = tiz_get_prc(ap_hdl);
   struct input_buf_private *inp = buf->pInputPortPrivate;

   if (inp) {
     enc_ReleaseTasks(&inp->tasks);
     if (inp->transfer)
       pipe_texture_unmap(priv->s_pipe, inp->transfer);
     pipe_resource_reference(&inp->resource, NULL);
     FREE(inp);
   }

   return super_FreeBuffer(typeOf(ap_obj, "h264einport"), ap_obj, ap_hdl, idx, buf);
}

/*
 * h264einport_class
 */

static void * h264e_inport_class_ctor(void * ap_obj, va_list * app)
{
   /* NOTE: Class methods might be added in the future. None for now. */
   return super_ctor (typeOf (ap_obj, "h264einport_class"), ap_obj, app);
}

/*
 * initialization
 */

void * h264e_inport_class_init(void * ap_tos, void * ap_hdl)
{
   void * tizvideoport = tiz_get_type(ap_hdl, "tizvideoport");
   void * h264einport_class
     = factory_new(classOf(tizvideoport), "h264einport_class",
                   classOf(tizvideoport), sizeof(h264e_inport_class_t),
                   ap_tos, ap_hdl, ctor, h264e_inport_class_ctor, 0);
   return h264einport_class;
}

void * h264e_inport_init(void * ap_tos, void * ap_hdl)
{
   void * tizvideoport = tiz_get_type (ap_hdl, "tizvideoport");
   void * h264einport_class = tiz_get_type (ap_hdl, "h264einport_class");
   void * h264einport = factory_new
     /* TIZ_CLASS_COMMENT: class type, class name, parent, size */
     (h264einport_class, "h264einport", tizvideoport,
      sizeof (h264e_inport_t),
      /* TIZ_CLASS_COMMENT: class constructor */
      ap_tos, ap_hdl,
      /* TIZ_CLASS_COMMENT: class constructor */
      ctor, h264e_inport_ctor,
      /* TIZ_CLASS_COMMENT: class destructor */
      dtor, h264e_inport_dtor,
      /* TIZ_CLASS_COMMENT: */
      tiz_api_AllocateBuffer, h264e_inport_AllocateBuffer,
      /* TIZ_CLASS_COMMENT: */
      tiz_api_UseBuffer, h264e_inport_UseBuffer,
      /* TIZ_CLASS_COMMENT: */
      tiz_api_FreeBuffer, h264e_inport_FreeBuffer,
      /* TIZ_CLASS_COMMENT: stop value*/
      0);

   return h264einport;
}
