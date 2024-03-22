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

#include "h264d.h"
#include "h264dinport.h"
#include "h264dinport_decls.h"
#include "vid_dec_common.h"

/*
 * h264dinport class
 */

static void * h264d_inport_ctor(void * ap_obj, va_list * app)
{
   return super_ctor(typeOf(ap_obj, "h264dinport"), ap_obj, app);
}

static void * h264d_inport_dtor(void * ap_obj)
{
   return super_dtor(typeOf(ap_obj, "h264dinport"), ap_obj);
}

/*
 * from tiz_api
 */

static OMX_ERRORTYPE h264d_inport_SetParameter(const void * ap_obj, OMX_HANDLETYPE ap_hdl,
                                               OMX_INDEXTYPE a_index, OMX_PTR ap_struct)
{
   OMX_ERRORTYPE err = OMX_ErrorNone;

   assert(ap_obj);
   assert(ap_hdl);
   assert(ap_struct);

   if (a_index == OMX_IndexParamPortDefinition) {
      vid_dec_PrivateType * p_prc = tiz_get_prc(ap_hdl);
      OMX_VIDEO_PORTDEFINITIONTYPE * p_def = &(p_prc->out_port_def_.format.video);
      OMX_PARAM_PORTDEFINITIONTYPE * i_def = (OMX_PARAM_PORTDEFINITIONTYPE *) ap_struct;

      /* Make changes only if there is a resolution change */
      if ((p_def->nFrameWidth == i_def->format.video.nFrameWidth) &&
          (p_def->nFrameHeight == i_def->format.video.nFrameHeight) &&
          (p_def->eCompressionFormat == i_def->format.video.eCompressionFormat))
         return err;

      /* Set some default values if not set */
      if (i_def->format.video.nStride == 0)
         i_def->format.video.nStride = i_def->format.video.nFrameWidth;
      if (i_def->format.video.nSliceHeight == 0)
         i_def->format.video.nSliceHeight = i_def->format.video.nFrameHeight;

      err = super_SetParameter(typeOf (ap_obj, "h264dinport"), ap_obj,
                               ap_hdl, a_index, ap_struct);
      if (err == OMX_ErrorNone) {
         tiz_port_t * p_obj = (tiz_port_t *) ap_obj;

         /* Set desired buffer size that will be used when allocating input buffers */
         p_obj->portdef_.nBufferSize = i_def->format.video.nFrameWidth * i_def->format.video.nFrameHeight * 512 / (16*16);

         /* Get a locally copy of port def. Useful for the early return above */
         tiz_check_omx(tiz_api_GetParameter(tiz_get_krn(handleOf(p_prc)), handleOf(p_prc),
                       OMX_IndexParamPortDefinition, &(p_prc->out_port_def_)));
      }
   }

   return err;
}

/*
 * h264dinport_class
 */

static void * h264d_inport_class_ctor(void * ap_obj, va_list * app)
{
   /* NOTE: Class methods might be added in the future. None for now. */
   return super_ctor (typeOf (ap_obj, "h264dinport_class"), ap_obj, app);
}

/*
 * initialization
 */

void * h264d_inport_class_init(void * ap_tos, void * ap_hdl)
{
   void * tizavcport = tiz_get_type(ap_hdl, "tizavcport");
   void * h264dinport_class
     = factory_new(classOf(tizavcport), "h264dinport_class",
                   classOf(tizavcport), sizeof(h264d_inport_class_t),
                   ap_tos, ap_hdl, ctor, h264d_inport_class_ctor, 0);
   return h264dinport_class;
}

void * h264d_inport_init(void * ap_tos, void * ap_hdl)
{
   void * tizavcport = tiz_get_type (ap_hdl, "tizavcport");
   void * h264dinport_class = tiz_get_type (ap_hdl, "h264dinport_class");
   void * h264dinport = factory_new
     /* TIZ_CLASS_COMMENT: class type, class name, parent, size */
     (h264dinport_class, "h264dinport", tizavcport,
      sizeof (h264d_inport_t),
      /* TIZ_CLASS_COMMENT: class constructor */
      ap_tos, ap_hdl,
      /* TIZ_CLASS_COMMENT: class constructor */
      ctor, h264d_inport_ctor,
      /* TIZ_CLASS_COMMENT: class destructor */
      dtor, h264d_inport_dtor,
      /* TIZ_CLASS_COMMENT: */
      tiz_api_SetParameter, h264d_inport_SetParameter,
      /* TIZ_CLASS_COMMENT: stop value*/
      0);

   return h264dinport;
}
