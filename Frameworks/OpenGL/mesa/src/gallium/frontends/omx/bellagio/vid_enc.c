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


#include <assert.h>

#include <OMX_Video.h>

/* bellagio defines a DEBUG macro that we don't want */
#ifndef DEBUG
#include <bellagio/omxcore.h>
#undef DEBUG
#else
#include <bellagio/omxcore.h>
#endif

#include <bellagio/omx_base_video_port.h>

#include "pipe/p_screen.h"
#include "pipe/p_video_codec.h"
#include "util/u_memory.h"

#include "vl/vl_codec.h"

#include "entrypoint.h"
#include "vid_enc.h"
#include "vid_omx_common.h"
#include "vid_enc_common.h"

static OMX_ERRORTYPE vid_enc_Constructor(OMX_COMPONENTTYPE *comp, OMX_STRING name);
static OMX_ERRORTYPE vid_enc_Destructor(OMX_COMPONENTTYPE *comp);
static OMX_ERRORTYPE vid_enc_SetParameter(OMX_HANDLETYPE handle, OMX_INDEXTYPE idx, OMX_PTR param);
static OMX_ERRORTYPE vid_enc_GetParameter(OMX_HANDLETYPE handle, OMX_INDEXTYPE idx, OMX_PTR param);
static OMX_ERRORTYPE vid_enc_SetConfig(OMX_HANDLETYPE handle, OMX_INDEXTYPE idx, OMX_PTR config);
static OMX_ERRORTYPE vid_enc_GetConfig(OMX_HANDLETYPE handle, OMX_INDEXTYPE idx, OMX_PTR config);
static OMX_ERRORTYPE vid_enc_MessageHandler(OMX_COMPONENTTYPE *comp, internalRequestMessageType *msg);
static OMX_ERRORTYPE vid_enc_AllocateInBuffer(omx_base_PortType *port, OMX_INOUT OMX_BUFFERHEADERTYPE **buf,
                                              OMX_IN OMX_U32 idx, OMX_IN OMX_PTR private, OMX_IN OMX_U32 size);
static OMX_ERRORTYPE vid_enc_UseInBuffer(omx_base_PortType *port, OMX_BUFFERHEADERTYPE **buf, OMX_U32 idx,
                                         OMX_PTR private, OMX_U32 size, OMX_U8 *mem);
static OMX_ERRORTYPE vid_enc_FreeInBuffer(omx_base_PortType *port, OMX_U32 idx, OMX_BUFFERHEADERTYPE *buf);
static OMX_ERRORTYPE vid_enc_EncodeFrame(omx_base_PortType *port, OMX_BUFFERHEADERTYPE *buf);
static OMX_ERRORTYPE vid_enc_AllocateOutBuffer(omx_base_PortType *comp, OMX_INOUT OMX_BUFFERHEADERTYPE **buf,
                                               OMX_IN OMX_U32 idx, OMX_IN OMX_PTR private, OMX_IN OMX_U32 size);
static OMX_ERRORTYPE vid_enc_FreeOutBuffer(omx_base_PortType *port, OMX_U32 idx, OMX_BUFFERHEADERTYPE *buf);
static void vid_enc_BufferEncoded(OMX_COMPONENTTYPE *comp, OMX_BUFFERHEADERTYPE* input, OMX_BUFFERHEADERTYPE* output);

OMX_ERRORTYPE vid_enc_LoaderComponent(stLoaderComponentType *comp)
{
   comp->componentVersion.s.nVersionMajor = 0;
   comp->componentVersion.s.nVersionMinor = 0;
   comp->componentVersion.s.nRevision = 0;
   comp->componentVersion.s.nStep = 1;
   comp->name_specific_length = 1;
   comp->constructor = vid_enc_Constructor;

   comp->name = CALLOC(1, OMX_MAX_STRINGNAME_SIZE);
   if (!comp->name)
      return OMX_ErrorInsufficientResources;

   comp->name_specific = CALLOC(1, sizeof(char *));
   if (!comp->name_specific)
      goto error_arrays;

   comp->role_specific = CALLOC(1, sizeof(char *));
   if (!comp->role_specific)
      goto error_arrays;

   comp->name_specific[0] = CALLOC(1, OMX_MAX_STRINGNAME_SIZE);
   if (comp->name_specific[0] == NULL)
      goto error_specific;

   comp->role_specific[0] = CALLOC(1, OMX_MAX_STRINGNAME_SIZE);
   if (comp->role_specific[0] == NULL)
      goto error_specific;

   strcpy(comp->name, OMX_VID_ENC_BASE_NAME);
   strcpy(comp->name_specific[0], OMX_VID_ENC_AVC_NAME);
   strcpy(comp->role_specific[0], OMX_VID_ENC_AVC_ROLE);

   return OMX_ErrorNone;

error_specific:
   FREE(comp->role_specific[0]);
   FREE(comp->name_specific[0]);

error_arrays:
   FREE(comp->role_specific);
   FREE(comp->name_specific);

   FREE(comp->name);

   return OMX_ErrorInsufficientResources;
}

static OMX_ERRORTYPE vid_enc_Constructor(OMX_COMPONENTTYPE *comp, OMX_STRING name)
{
   vid_enc_PrivateType *priv;
   omx_base_video_PortType *port;
   struct pipe_screen *screen;
   OMX_ERRORTYPE r;
   int i;

   assert(!comp->pComponentPrivate);

   priv = comp->pComponentPrivate = CALLOC(1, sizeof(vid_enc_PrivateType));
   if (!priv)
      return OMX_ErrorInsufficientResources;

   r = omx_base_filter_Constructor(comp, name);
   if (r)
       return r;

   priv->BufferMgmtCallback = vid_enc_BufferEncoded;
   priv->messageHandler = vid_enc_MessageHandler;
   priv->destructor = vid_enc_Destructor;

   comp->SetParameter = vid_enc_SetParameter;
   comp->GetParameter = vid_enc_GetParameter;
   comp->GetConfig = vid_enc_GetConfig;
   comp->SetConfig = vid_enc_SetConfig;

   priv->screen = omx_get_screen();
   if (!priv->screen)
      return OMX_ErrorInsufficientResources;

   screen = priv->screen->pscreen;
   if (!vl_codec_supported(screen, PIPE_VIDEO_PROFILE_MPEG4_AVC_HIGH, true))
      return OMX_ErrorBadParameter;

   priv->s_pipe = pipe_create_multimedia_context(screen);
   if (!priv->s_pipe)
      return OMX_ErrorInsufficientResources;

   enc_InitCompute_common(priv);

   if (!vl_compositor_init(&priv->compositor, priv->s_pipe)) {
      priv->s_pipe->destroy(priv->s_pipe);
      priv->s_pipe = NULL;
      return OMX_ErrorInsufficientResources;
   }

   if (!vl_compositor_init_state(&priv->cstate, priv->s_pipe)) {
      vl_compositor_cleanup(&priv->compositor);
      priv->s_pipe->destroy(priv->s_pipe);
      priv->s_pipe = NULL;
      return OMX_ErrorInsufficientResources;
   }

   priv->t_pipe = pipe_create_multimedia_context(screen);
   if (!priv->t_pipe)
      return OMX_ErrorInsufficientResources;

   priv->sPortTypesParam[OMX_PortDomainVideo].nStartPortNumber = 0;
   priv->sPortTypesParam[OMX_PortDomainVideo].nPorts = 2;
   priv->ports = CALLOC(2, sizeof(omx_base_PortType *));
   if (!priv->ports)
      return OMX_ErrorInsufficientResources;

   for (i = 0; i < 2; ++i) {
      priv->ports[i] = CALLOC(1, sizeof(omx_base_video_PortType));
      if (!priv->ports[i])
         return OMX_ErrorInsufficientResources;

      base_video_port_Constructor(comp, &priv->ports[i], i, i == 0);
   }

   port = (omx_base_video_PortType *)priv->ports[OMX_BASE_FILTER_INPUTPORT_INDEX];
   port->sPortParam.format.video.nFrameWidth = 176;
   port->sPortParam.format.video.nFrameHeight = 144;
   port->sPortParam.format.video.eColorFormat = OMX_COLOR_FormatYUV420SemiPlanar;
   port->sVideoParam.eColorFormat = OMX_COLOR_FormatYUV420SemiPlanar;
   port->sPortParam.nBufferCountActual = 8;
   port->sPortParam.nBufferCountMin = 4;

   port->Port_SendBufferFunction = vid_enc_EncodeFrame;
   port->Port_AllocateBuffer = vid_enc_AllocateInBuffer;
   port->Port_UseBuffer = vid_enc_UseInBuffer;
   port->Port_FreeBuffer = vid_enc_FreeInBuffer;

   port = (omx_base_video_PortType *)priv->ports[OMX_BASE_FILTER_OUTPUTPORT_INDEX];
   strcpy(port->sPortParam.format.video.cMIMEType,"video/H264");
   port->sPortParam.format.video.nFrameWidth = 176;
   port->sPortParam.format.video.nFrameHeight = 144;
   port->sPortParam.format.video.eCompressionFormat = OMX_VIDEO_CodingAVC;
   port->sVideoParam.eCompressionFormat = OMX_VIDEO_CodingAVC;

   port->Port_AllocateBuffer = vid_enc_AllocateOutBuffer;
   port->Port_FreeBuffer = vid_enc_FreeOutBuffer;

   priv->bitrate.eControlRate = OMX_Video_ControlRateDisable;
   priv->bitrate.nTargetBitrate = 0;

   priv->quant.nQpI = OMX_VID_ENC_QUANT_I_FRAMES_DEFAULT;
   priv->quant.nQpP = OMX_VID_ENC_QUANT_P_FRAMES_DEFAULT;
   priv->quant.nQpB = OMX_VID_ENC_QUANT_B_FRAMES_DEFAULT;

   priv->profile_level.eProfile = OMX_VIDEO_AVCProfileBaseline;
   priv->profile_level.eLevel = OMX_VIDEO_AVCLevel51;

   priv->force_pic_type.IntraRefreshVOP = OMX_FALSE;
   priv->frame_num = 0;
   priv->pic_order_cnt = 0;
   priv->restricted_b_frames = debug_get_bool_option("OMX_USE_RESTRICTED_B_FRAMES", false);

   priv->scale.xWidth = OMX_VID_ENC_SCALING_WIDTH_DEFAULT;
   priv->scale.xHeight = OMX_VID_ENC_SCALING_WIDTH_DEFAULT;

   list_inithead(&priv->free_tasks);
   list_inithead(&priv->used_tasks);
   list_inithead(&priv->b_frames);
   list_inithead(&priv->stacked_tasks);

   return OMX_ErrorNone;
}

static OMX_ERRORTYPE vid_enc_Destructor(OMX_COMPONENTTYPE *comp)
{
   vid_enc_PrivateType* priv = comp->pComponentPrivate;
   int i;

   enc_ReleaseTasks(&priv->free_tasks);
   enc_ReleaseTasks(&priv->used_tasks);
   enc_ReleaseTasks(&priv->b_frames);
   enc_ReleaseTasks(&priv->stacked_tasks);

   if (priv->ports) {
      for (i = 0; i < priv->sPortTypesParam[OMX_PortDomainVideo].nPorts; ++i) {
         if(priv->ports[i])
            priv->ports[i]->PortDestructor(priv->ports[i]);
      }
      FREE(priv->ports);
      priv->ports=NULL;
   }

   for (i = 0; i < OMX_VID_ENC_NUM_SCALING_BUFFERS; ++i)
      if (priv->scale_buffer[i])
         priv->scale_buffer[i]->destroy(priv->scale_buffer[i]);

   if (priv->s_pipe) {
      vl_compositor_cleanup_state(&priv->cstate);
      vl_compositor_cleanup(&priv->compositor);
      enc_ReleaseCompute_common(priv);
      priv->s_pipe->destroy(priv->s_pipe);
   }

   if (priv->t_pipe)
      priv->t_pipe->destroy(priv->t_pipe);

   if (priv->screen)
      omx_put_screen();

   return omx_workaround_Destructor(comp);
}

static OMX_ERRORTYPE enc_AllocateBackTexture(omx_base_PortType *port,
                                             struct pipe_resource **resource,
                                             struct pipe_transfer **transfer,
                                             OMX_U8 **map)
{
   OMX_COMPONENTTYPE* comp = port->standCompContainer;
   vid_enc_PrivateType *priv = comp->pComponentPrivate;
   struct pipe_resource buf_templ;
   struct pipe_box box = {};
   OMX_U8 *ptr;

   memset(&buf_templ, 0, sizeof buf_templ);
   buf_templ.target = PIPE_TEXTURE_2D;
   buf_templ.format = PIPE_FORMAT_I8_UNORM;
   buf_templ.bind = PIPE_BIND_LINEAR;
   buf_templ.usage = PIPE_USAGE_STAGING;
   buf_templ.flags = 0;
   buf_templ.width0 = port->sPortParam.format.video.nFrameWidth;
   buf_templ.height0 = port->sPortParam.format.video.nFrameHeight * 3 / 2;
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

static OMX_ERRORTYPE vid_enc_SetParameter(OMX_HANDLETYPE handle, OMX_INDEXTYPE idx, OMX_PTR param)
{
   OMX_COMPONENTTYPE *comp = handle;
   vid_enc_PrivateType *priv = comp->pComponentPrivate;
   OMX_ERRORTYPE r;

   if (!param)
      return OMX_ErrorBadParameter;

   switch(idx) {
   case OMX_IndexParamPortDefinition: {
      OMX_PARAM_PORTDEFINITIONTYPE *def = param;

      r = omx_base_component_SetParameter(handle, idx, param);
      if (r)
         return r;

      if (def->nPortIndex == OMX_BASE_FILTER_INPUTPORT_INDEX) {
         omx_base_video_PortType *port;
         unsigned framesize;
         struct pipe_resource *resource;
         struct pipe_transfer *transfer;

         port = (omx_base_video_PortType *)priv->ports[OMX_BASE_FILTER_INPUTPORT_INDEX];
         enc_AllocateBackTexture(priv->ports[OMX_BASE_FILTER_INPUTPORT_INDEX],
                                 &resource, &transfer, NULL);
         port->sPortParam.format.video.nStride = transfer->stride;
         pipe_texture_unmap(priv->s_pipe, transfer);
         pipe_resource_reference(&resource, NULL);

         framesize = port->sPortParam.format.video.nStride *
                     port->sPortParam.format.video.nFrameHeight;
         port->sPortParam.format.video.nSliceHeight = port->sPortParam.format.video.nFrameHeight;
         port->sPortParam.nBufferSize = framesize * 3 / 2;

         port = (omx_base_video_PortType *)priv->ports[OMX_BASE_FILTER_OUTPUTPORT_INDEX];
         port->sPortParam.nBufferSize = framesize * 512 / (16*16);

         priv->frame_rate = def->format.video.xFramerate;

         priv->callbacks->EventHandler(comp, priv->callbackData, OMX_EventPortSettingsChanged,
                                       OMX_BASE_FILTER_OUTPUTPORT_INDEX, 0, NULL);
      }
      break;
   }
   case OMX_IndexParamStandardComponentRole: {
      OMX_PARAM_COMPONENTROLETYPE *role = param;

      r = checkHeader(param, sizeof(OMX_PARAM_COMPONENTROLETYPE));
      if (r)
         return r;

      if (strcmp((char *)role->cRole, OMX_VID_ENC_AVC_ROLE)) {
         return OMX_ErrorBadParameter;
      }

      break;
   }
   case OMX_IndexParamVideoBitrate: {
      OMX_VIDEO_PARAM_BITRATETYPE *bitrate = param;

      r = checkHeader(param, sizeof(OMX_VIDEO_PARAM_BITRATETYPE));
      if (r)
         return r;

      priv->bitrate = *bitrate;

      break;
   }
   case OMX_IndexParamVideoQuantization: {
      OMX_VIDEO_PARAM_QUANTIZATIONTYPE *quant = param;

      r = checkHeader(param, sizeof(OMX_VIDEO_PARAM_QUANTIZATIONTYPE));
      if (r)
         return r;

      priv->quant = *quant;

      break;
   }
   case OMX_IndexParamVideoProfileLevelCurrent: {
      OMX_VIDEO_PARAM_PROFILELEVELTYPE *profile_level = param;

      r = checkHeader(param, sizeof(OMX_VIDEO_PARAM_PROFILELEVELTYPE));
      if (r)
         return r;

      priv->profile_level = *profile_level;

      break;
   }
   default:
      return omx_base_component_SetParameter(handle, idx, param);
   }
   return OMX_ErrorNone;
}

static OMX_ERRORTYPE vid_enc_GetParameter(OMX_HANDLETYPE handle, OMX_INDEXTYPE idx, OMX_PTR param)
{
   OMX_COMPONENTTYPE *comp = handle;
   vid_enc_PrivateType *priv = comp->pComponentPrivate;
   OMX_ERRORTYPE r;

   if (!param)
      return OMX_ErrorBadParameter;

   switch(idx) {
   case OMX_IndexParamStandardComponentRole: {
      OMX_PARAM_COMPONENTROLETYPE *role = param;

      r = checkHeader(param, sizeof(OMX_PARAM_COMPONENTROLETYPE));
      if (r)
         return r;

      strcpy((char *)role->cRole, OMX_VID_ENC_AVC_ROLE);
      break;
   }
   case OMX_IndexParamVideoInit:
      r = checkHeader(param, sizeof(OMX_PORT_PARAM_TYPE));
      if (r)
         return r;

      memcpy(param, &priv->sPortTypesParam[OMX_PortDomainVideo], sizeof(OMX_PORT_PARAM_TYPE));
      break;

   case OMX_IndexParamVideoPortFormat: {
      OMX_VIDEO_PARAM_PORTFORMATTYPE *format = param;
      omx_base_video_PortType *port;

      r = checkHeader(param, sizeof(OMX_VIDEO_PARAM_PORTFORMATTYPE));
      if (r)
         return r;

      if (format->nPortIndex > 1)
         return OMX_ErrorBadPortIndex;
      if (format->nIndex >= 1)
         return OMX_ErrorNoMore;

      port = (omx_base_video_PortType *)priv->ports[format->nPortIndex];
      memcpy(format, &port->sVideoParam, sizeof(OMX_VIDEO_PARAM_PORTFORMATTYPE));
      break;
   }
   case OMX_IndexParamVideoBitrate: {
      OMX_VIDEO_PARAM_BITRATETYPE *bitrate = param;

      r = checkHeader(param, sizeof(OMX_VIDEO_PARAM_BITRATETYPE));
      if (r)
         return r;

      bitrate->eControlRate = priv->bitrate.eControlRate;
      bitrate->nTargetBitrate = priv->bitrate.nTargetBitrate;

      break;
   }
   case OMX_IndexParamVideoQuantization: {
      OMX_VIDEO_PARAM_QUANTIZATIONTYPE *quant = param;

      r = checkHeader(param, sizeof(OMX_VIDEO_PARAM_QUANTIZATIONTYPE));
      if (r)
         return r;

      quant->nQpI = priv->quant.nQpI;
      quant->nQpP = priv->quant.nQpP;
      quant->nQpB = priv->quant.nQpB;

      break;
   }
   case OMX_IndexParamVideoProfileLevelCurrent: {
      OMX_VIDEO_PARAM_PROFILELEVELTYPE *profile_level = param;

      r = checkHeader(param, sizeof(OMX_VIDEO_PARAM_PROFILELEVELTYPE));
      if (r)
         return r;

      profile_level->eProfile = priv->profile_level.eProfile;
      profile_level->eLevel = priv->profile_level.eLevel;

      break;
   }
   default:
      return omx_base_component_GetParameter(handle, idx, param);
   }
   return OMX_ErrorNone;
}

static OMX_ERRORTYPE vid_enc_SetConfig(OMX_HANDLETYPE handle, OMX_INDEXTYPE idx, OMX_PTR config)
{
   OMX_COMPONENTTYPE *comp = handle;
   vid_enc_PrivateType *priv = comp->pComponentPrivate;
   OMX_ERRORTYPE r;
   int i;

   if (!config)
      return OMX_ErrorBadParameter;

   switch(idx) {
   case OMX_IndexConfigVideoIntraVOPRefresh: {
      OMX_CONFIG_INTRAREFRESHVOPTYPE *type = config;

      r = checkHeader(config, sizeof(OMX_CONFIG_INTRAREFRESHVOPTYPE));
      if (r)
         return r;

      priv->force_pic_type = *type;

      break;
   }
   case OMX_IndexConfigCommonScale: {
      OMX_CONFIG_SCALEFACTORTYPE *scale = config;

      r = checkHeader(config, sizeof(OMX_CONFIG_SCALEFACTORTYPE));
      if (r)
         return r;

      if (scale->xWidth < 176 || scale->xHeight < 144)
         return OMX_ErrorBadParameter;

      for (i = 0; i < OMX_VID_ENC_NUM_SCALING_BUFFERS; ++i) {
         if (priv->scale_buffer[i]) {
            priv->scale_buffer[i]->destroy(priv->scale_buffer[i]);
            priv->scale_buffer[i] = NULL;
         }
      }

      priv->scale = *scale;
      if (priv->scale.xWidth != 0xffffffff && priv->scale.xHeight != 0xffffffff) {
         struct pipe_video_buffer templat = {};

         templat.buffer_format = PIPE_FORMAT_NV12;
         templat.width = priv->scale.xWidth;
         templat.height = priv->scale.xHeight;
         templat.interlaced = false;
         for (i = 0; i < OMX_VID_ENC_NUM_SCALING_BUFFERS; ++i) {
            priv->scale_buffer[i] = priv->s_pipe->create_video_buffer(priv->s_pipe, &templat);
            if (!priv->scale_buffer[i])
               return OMX_ErrorInsufficientResources;
         }
      }

      break;
   }
   default:
      return omx_base_component_SetConfig(handle, idx, config);
   }

   return OMX_ErrorNone;
}

static OMX_ERRORTYPE vid_enc_GetConfig(OMX_HANDLETYPE handle, OMX_INDEXTYPE idx, OMX_PTR config)
{
   OMX_COMPONENTTYPE *comp = handle;
   vid_enc_PrivateType *priv = comp->pComponentPrivate;
   OMX_ERRORTYPE r;

   if (!config)
      return OMX_ErrorBadParameter;

   switch(idx) {
   case OMX_IndexConfigCommonScale: {
      OMX_CONFIG_SCALEFACTORTYPE *scale = config;

      r = checkHeader(config, sizeof(OMX_CONFIG_SCALEFACTORTYPE));
      if (r)
         return r;

      scale->xWidth = priv->scale.xWidth;
      scale->xHeight = priv->scale.xHeight;

      break;
   }
   default:
      return omx_base_component_GetConfig(handle, idx, config);
   }

   return OMX_ErrorNone;
}

static OMX_ERRORTYPE vid_enc_MessageHandler(OMX_COMPONENTTYPE* comp, internalRequestMessageType *msg)
{
   vid_enc_PrivateType* priv = comp->pComponentPrivate;

   if (msg->messageType == OMX_CommandStateSet) {
      if ((msg->messageParam == OMX_StateIdle ) && (priv->state == OMX_StateLoaded)) {

         struct pipe_video_codec templat = {};
         omx_base_video_PortType *port;

         port = (omx_base_video_PortType *)priv->ports[OMX_BASE_FILTER_INPUTPORT_INDEX];

         templat.profile = enc_TranslateOMXProfileToPipe(priv->profile_level.eProfile);
         templat.level = enc_TranslateOMXLevelToPipe(priv->profile_level.eLevel);
         templat.entrypoint = PIPE_VIDEO_ENTRYPOINT_ENCODE;
         templat.chroma_format = PIPE_VIDEO_CHROMA_FORMAT_420;
         templat.width = priv->scale_buffer[priv->current_scale_buffer] ?
                            priv->scale.xWidth : port->sPortParam.format.video.nFrameWidth;
         templat.height = priv->scale_buffer[priv->current_scale_buffer] ?
                            priv->scale.xHeight : port->sPortParam.format.video.nFrameHeight;

         if (templat.profile == PIPE_VIDEO_PROFILE_MPEG4_AVC_BASELINE) {
            struct pipe_screen *screen = priv->screen->pscreen;
            templat.max_references = 1;
            priv->stacked_frames_num =
               screen->get_video_param(screen,
                                       PIPE_VIDEO_PROFILE_MPEG4_AVC_HIGH,
                                       PIPE_VIDEO_ENTRYPOINT_ENCODE,
                                       PIPE_VIDEO_CAP_STACKED_FRAMES);
         } else {
            templat.max_references = OMX_VID_ENC_P_PERIOD_DEFAULT;
            priv->stacked_frames_num = 1;
         }
         priv->codec = priv->s_pipe->create_video_codec(priv->s_pipe, &templat);

      } else if ((msg->messageParam == OMX_StateLoaded) && (priv->state == OMX_StateIdle)) {
         if (priv->codec) {
            priv->codec->destroy(priv->codec);
            priv->codec = NULL;
         }
      }
   }

   return omx_base_component_MessageHandler(comp, msg);
}

static OMX_ERRORTYPE vid_enc_AllocateInBuffer(omx_base_PortType *port, OMX_INOUT OMX_BUFFERHEADERTYPE **buf,
                                              OMX_IN OMX_U32 idx, OMX_IN OMX_PTR private, OMX_IN OMX_U32 size)
{
   struct input_buf_private *inp;
   OMX_ERRORTYPE r;

   r = base_port_AllocateBuffer(port, buf, idx, private, size);
   if (r)
      return r;

   inp = (*buf)->pInputPortPrivate = CALLOC_STRUCT(input_buf_private);
   if (!inp) {
      base_port_FreeBuffer(port, idx, *buf);
      return OMX_ErrorInsufficientResources;
   }

   list_inithead(&inp->tasks);

   FREE((*buf)->pBuffer);
   r = enc_AllocateBackTexture(port, &inp->resource, &inp->transfer, &(*buf)->pBuffer);
   if (r) {
      FREE(inp);
      base_port_FreeBuffer(port, idx, *buf);
      return r;
   }

   return OMX_ErrorNone;
}

static OMX_ERRORTYPE vid_enc_UseInBuffer(omx_base_PortType *port, OMX_BUFFERHEADERTYPE **buf, OMX_U32 idx,
                                         OMX_PTR private, OMX_U32 size, OMX_U8 *mem)
{
   struct input_buf_private *inp;
   OMX_ERRORTYPE r;

   r = base_port_UseBuffer(port, buf, idx, private, size, mem);
   if (r)
      return r;

   inp = (*buf)->pInputPortPrivate = CALLOC_STRUCT(input_buf_private);
   if (!inp) {
      base_port_FreeBuffer(port, idx, *buf);
      return OMX_ErrorInsufficientResources;
   }

   list_inithead(&inp->tasks);

   return OMX_ErrorNone;
}

static OMX_ERRORTYPE vid_enc_FreeInBuffer(omx_base_PortType *port, OMX_U32 idx, OMX_BUFFERHEADERTYPE *buf)
{
   OMX_COMPONENTTYPE* comp = port->standCompContainer;
   vid_enc_PrivateType *priv = comp->pComponentPrivate;
   struct input_buf_private *inp = buf->pInputPortPrivate;

   if (inp) {
      enc_ReleaseTasks(&inp->tasks);
      if (inp->transfer)
         pipe_texture_unmap(priv->s_pipe, inp->transfer);
      pipe_resource_reference(&inp->resource, NULL);
      FREE(inp);
   }
   buf->pBuffer = NULL;

   return base_port_FreeBuffer(port, idx, buf);
}

static OMX_ERRORTYPE vid_enc_AllocateOutBuffer(omx_base_PortType *port, OMX_INOUT OMX_BUFFERHEADERTYPE **buf,
                                               OMX_IN OMX_U32 idx, OMX_IN OMX_PTR private, OMX_IN OMX_U32 size)
{
   OMX_ERRORTYPE r;

   r = base_port_AllocateBuffer(port, buf, idx, private, size);
   if (r)
      return r;

   FREE((*buf)->pBuffer);
   (*buf)->pBuffer = NULL;
   (*buf)->pOutputPortPrivate = CALLOC(1, sizeof(struct output_buf_private));
   if (!(*buf)->pOutputPortPrivate) {
      base_port_FreeBuffer(port, idx, *buf);
      return OMX_ErrorInsufficientResources;
   }

   return OMX_ErrorNone;
}

static OMX_ERRORTYPE vid_enc_FreeOutBuffer(omx_base_PortType *port, OMX_U32 idx, OMX_BUFFERHEADERTYPE *buf)
{
   OMX_COMPONENTTYPE* comp = port->standCompContainer;
   vid_enc_PrivateType *priv = comp->pComponentPrivate;

   if (buf->pOutputPortPrivate) {
      struct output_buf_private *outp = buf->pOutputPortPrivate;
      if (outp->transfer)
         pipe_buffer_unmap(priv->t_pipe, outp->transfer);
      pipe_resource_reference(&outp->bitstream, NULL);
      FREE(outp);
      buf->pOutputPortPrivate = NULL;
   }
   buf->pBuffer = NULL;

   return base_port_FreeBuffer(port, idx, buf);
}

static struct encode_task *enc_NeedTask(omx_base_PortType *port)
{
   OMX_VIDEO_PORTDEFINITIONTYPE *def = &port->sPortParam.format.video;
   OMX_COMPONENTTYPE* comp = port->standCompContainer;
   vid_enc_PrivateType *priv = comp->pComponentPrivate;

   return enc_NeedTask_common(priv, def);
}

static OMX_ERRORTYPE enc_LoadImage(omx_base_PortType *port, OMX_BUFFERHEADERTYPE *buf,
                                   struct pipe_video_buffer *vbuf)
{
   OMX_COMPONENTTYPE* comp = port->standCompContainer;
   vid_enc_PrivateType *priv = comp->pComponentPrivate;
   OMX_VIDEO_PORTDEFINITIONTYPE *def = &port->sPortParam.format.video;
   return enc_LoadImage_common(priv, def, buf, vbuf);
}

static void enc_ScaleInput(omx_base_PortType *port, struct pipe_video_buffer **vbuf, unsigned *size)
{
   OMX_COMPONENTTYPE* comp = port->standCompContainer;
   vid_enc_PrivateType *priv = comp->pComponentPrivate;
   OMX_VIDEO_PORTDEFINITIONTYPE *def = &port->sPortParam.format.video;
   enc_ScaleInput_common(priv, def, vbuf, size);
}

static void enc_ControlPicture(omx_base_PortType *port, struct pipe_h264_enc_picture_desc *picture)
{
   OMX_COMPONENTTYPE* comp = port->standCompContainer;
   vid_enc_PrivateType *priv = comp->pComponentPrivate;
   enc_ControlPicture_common(priv, picture);
}

static void enc_HandleTask(omx_base_PortType *port, struct encode_task *task,
                           enum pipe_h2645_enc_picture_type picture_type)
{
   OMX_COMPONENTTYPE* comp = port->standCompContainer;
   vid_enc_PrivateType *priv = comp->pComponentPrivate;
   unsigned size = priv->ports[OMX_BASE_FILTER_OUTPUTPORT_INDEX]->sPortParam.nBufferSize;
   struct pipe_video_buffer *vbuf = task->buf;
   struct pipe_h264_enc_picture_desc picture = {};

   /* -------------- scale input image --------- */
   enc_ScaleInput(port, &vbuf, &size);
   priv->s_pipe->flush(priv->s_pipe, NULL, 0);

   /* -------------- allocate output buffer --------- */
   task->bitstream = pipe_buffer_create(priv->s_pipe->screen,
                                        PIPE_BIND_VERTEX_BUFFER,
                                        PIPE_USAGE_STAGING, /* map for read */
                                        size);

   picture.picture_type = picture_type;
   picture.pic_order_cnt = task->pic_order_cnt;
   picture.base.profile = enc_TranslateOMXProfileToPipe(priv->profile_level.eProfile);
   picture.base.entry_point = PIPE_VIDEO_ENTRYPOINT_ENCODE;
   if (priv->restricted_b_frames && picture_type == PIPE_H2645_ENC_PICTURE_TYPE_B)
      picture.not_referenced = true;
   enc_ControlPicture(port, &picture);

   /* -------------- encode frame --------- */
   priv->codec->begin_frame(priv->codec, vbuf, &picture.base);
   priv->codec->encode_bitstream(priv->codec, vbuf, task->bitstream, &task->feedback);
   priv->codec->end_frame(priv->codec, vbuf, &picture.base);
}

static void enc_ClearBframes(omx_base_PortType *port, struct input_buf_private *inp)
{
   OMX_COMPONENTTYPE* comp = port->standCompContainer;
   vid_enc_PrivateType *priv = comp->pComponentPrivate;
   struct encode_task *task;

   if (list_is_empty(&priv->b_frames))
      return;

   task = list_entry(priv->b_frames.prev, struct encode_task, list);
   list_del(&task->list);

   /* promote last from to P frame */
   priv->ref_idx_l0 = priv->ref_idx_l1;
   enc_HandleTask(port, task, PIPE_H2645_ENC_PICTURE_TYPE_P);
   list_addtail(&task->list, &inp->tasks);
   priv->ref_idx_l1 = priv->frame_num++;

   /* handle B frames */
   LIST_FOR_EACH_ENTRY(task, &priv->b_frames, list) {
      enc_HandleTask(port, task, PIPE_H2645_ENC_PICTURE_TYPE_B);
      if (!priv->restricted_b_frames)
         priv->ref_idx_l0 = priv->frame_num;
      priv->frame_num++;
   }

   enc_MoveTasks(&priv->b_frames, &inp->tasks);
}

static OMX_ERRORTYPE vid_enc_EncodeFrame(omx_base_PortType *port, OMX_BUFFERHEADERTYPE *buf)
{
   OMX_COMPONENTTYPE* comp = port->standCompContainer;
   vid_enc_PrivateType *priv = comp->pComponentPrivate;
   struct input_buf_private *inp = buf->pInputPortPrivate;
   enum pipe_h2645_enc_picture_type picture_type;
   struct encode_task *task;
   unsigned stacked_num = 0;
   OMX_ERRORTYPE err;

   enc_MoveTasks(&inp->tasks, &priv->free_tasks);
   task = enc_NeedTask(port);
   if (!task)
      return OMX_ErrorInsufficientResources;

   if (buf->nFilledLen == 0) {
      if (buf->nFlags & OMX_BUFFERFLAG_EOS) {
         buf->nFilledLen = buf->nAllocLen;
         enc_ClearBframes(port, inp);
         enc_MoveTasks(&priv->stacked_tasks, &inp->tasks);
         priv->codec->flush(priv->codec);
      }
      return base_port_SendBufferFunction(port, buf);
   }

   if (buf->pOutputPortPrivate) {
      struct pipe_video_buffer *vbuf = buf->pOutputPortPrivate;
      buf->pOutputPortPrivate = task->buf;
      task->buf = vbuf;
   } else {
      /* ------- load input image into video buffer ---- */
      err = enc_LoadImage(port, buf, task->buf);
      if (err != OMX_ErrorNone) {
         FREE(task);
         return err;
      }
   }

   /* -------------- determine picture type --------- */
   if (!(priv->pic_order_cnt % OMX_VID_ENC_IDR_PERIOD_DEFAULT) ||
       priv->force_pic_type.IntraRefreshVOP) {
      enc_ClearBframes(port, inp);
      picture_type = PIPE_H2645_ENC_PICTURE_TYPE_IDR;
      priv->force_pic_type.IntraRefreshVOP = OMX_FALSE;
      priv->frame_num = 0;
      priv->pic_order_cnt = 0;
   } else if (priv->codec->profile == PIPE_VIDEO_PROFILE_MPEG4_AVC_BASELINE ||
              !(priv->pic_order_cnt % OMX_VID_ENC_P_PERIOD_DEFAULT) ||
              (buf->nFlags & OMX_BUFFERFLAG_EOS)) {
      picture_type = PIPE_H2645_ENC_PICTURE_TYPE_P;
   } else {
      picture_type = PIPE_H2645_ENC_PICTURE_TYPE_B;
   }

   task->pic_order_cnt = priv->pic_order_cnt++;

   if (picture_type == PIPE_H2645_ENC_PICTURE_TYPE_B) {
      /* put frame at the tail of the queue */
      list_addtail(&task->list, &priv->b_frames);
   } else {
      /* handle I or P frame */
      priv->ref_idx_l0 = priv->ref_idx_l1;
      enc_HandleTask(port, task, picture_type);
      list_addtail(&task->list, &priv->stacked_tasks);
      LIST_FOR_EACH_ENTRY(task, &priv->stacked_tasks, list) {
         ++stacked_num;
      }
      if (stacked_num == priv->stacked_frames_num) {
         struct encode_task *t;
         t = list_entry(priv->stacked_tasks.next, struct encode_task, list);
         list_del(&t->list);
         list_addtail(&t->list, &inp->tasks);
      }
      priv->ref_idx_l1 = priv->frame_num++;

      /* handle B frames */
      LIST_FOR_EACH_ENTRY(task, &priv->b_frames, list) {
         enc_HandleTask(port, task, PIPE_H2645_ENC_PICTURE_TYPE_B);
         if (!priv->restricted_b_frames)
            priv->ref_idx_l0 = priv->frame_num;
         priv->frame_num++;
      }

      enc_MoveTasks(&priv->b_frames, &inp->tasks);
   }

   if (list_is_empty(&inp->tasks))
      return port->ReturnBufferFunction(port, buf);
   else
      return base_port_SendBufferFunction(port, buf);
}

static void vid_enc_BufferEncoded(OMX_COMPONENTTYPE *comp, OMX_BUFFERHEADERTYPE* input, OMX_BUFFERHEADERTYPE* output)
{
   vid_enc_PrivateType *priv = comp->pComponentPrivate;
   vid_enc_BufferEncoded_common(priv, input, output);
}
