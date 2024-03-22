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

/*
 * Authors:
 *      Christian König <christian.koenig@amd.com>
 *
 */

#include "entrypoint.h"
#include "vid_dec.h"
#include "vid_enc.h"

int omx_component_library_Setup(stLoaderComponentType **stComponents)
{
   OMX_ERRORTYPE r;

   if (stComponents == NULL)
      return 2;

   /* component 0 - video decoder */
   r = vid_dec_LoaderComponent(stComponents[0]);
   if (r != OMX_ErrorNone)
      return OMX_ErrorInsufficientResources;

   /* component 1 - video encoder */
   r = vid_enc_LoaderComponent(stComponents[1]);
   if (r != OMX_ErrorNone)
      return OMX_ErrorInsufficientResources;

   return 2;
}

OMX_ERRORTYPE omx_workaround_Destructor(OMX_COMPONENTTYPE *comp)
{
   omx_base_component_PrivateType* priv = (omx_base_component_PrivateType*)comp->pComponentPrivate;

   priv->state = OMX_StateInvalid;
   tsem_up(priv->messageSem);

   /* wait for thread to exit */
   pthread_join(priv->messageHandlerThread, NULL);

   return omx_base_component_Destructor(comp);
}
