/**************************************************************************
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

#include <tizport_decls.h>

#include "vid_dec_h264_common.h"

#include "h264dprc.h"
#include "h264d.h"
#include "names.h"

static OMX_VERSIONTYPE h264_decoder_version = {{0, 0, 0, 1}};

OMX_PTR instantiate_h264d_input_port(OMX_HANDLETYPE ap_hdl)
{
   OMX_VIDEO_PORTDEFINITIONTYPE portdef;
   OMX_VIDEO_PARAM_AVCTYPE avctype;
   OMX_VIDEO_CODINGTYPE encodings[] = {
      OMX_VIDEO_CodingAVC,
      OMX_VIDEO_CodingMax
   };
   OMX_COLOR_FORMATTYPE formats[] = {
      OMX_COLOR_FormatUnused,
      OMX_COLOR_FormatMax
   };
   tiz_port_options_t avc_port_opts = {
      OMX_PortDomainVideo,
      OMX_DirInput,
      OMX_VID_DEC_AVC_INPUT_PORT_MIN_BUF_COUNT,
      OMX_VID_DEC_AVC_PORT_MIN_INPUT_BUF_SIZE,
      OMX_VID_DEC_AVC_PORT_NONCONTIGUOUS,
      OMX_VID_DEC_AVC_PORT_ALIGNMENT,
      OMX_VID_DEC_AVC_PORT_SUPPLIERPREF,
      {OMX_VID_DEC_AVC_INPUT_PORT_INDEX, NULL, NULL, NULL},
      1                     /* slave port */
   };
   OMX_VIDEO_AVCLEVELTYPE levels[] = {
      OMX_VIDEO_AVCLevel1,
      OMX_VIDEO_AVCLevel1b,
      OMX_VIDEO_AVCLevel11,
      OMX_VIDEO_AVCLevel12,
      OMX_VIDEO_AVCLevel13,
      OMX_VIDEO_AVCLevel2,
      OMX_VIDEO_AVCLevel21,
      OMX_VIDEO_AVCLevel22,
      OMX_VIDEO_AVCLevel3,
      OMX_VIDEO_AVCLevel31,
      OMX_VIDEO_AVCLevel32,
      OMX_VIDEO_AVCLevel4,
      OMX_VIDEO_AVCLevel41,
      OMX_VIDEO_AVCLevel42,
      OMX_VIDEO_AVCLevel5,
      OMX_VIDEO_AVCLevel51,
      OMX_VIDEO_AVCLevelMax
   };

   portdef.pNativeRender = NULL;
   portdef.nFrameWidth = OMX_VID_DEC_AVC_DEFAULT_FRAME_WIDTH;
   portdef.nFrameHeight = OMX_VID_DEC_AVC_DEFAULT_FRAME_HEIGHT;
   portdef.nStride = 0;
   portdef.nSliceHeight = 0;
   portdef.nBitrate = 64000;
   portdef.xFramerate = OMX_VID_DEC_AVC_DEFAULT_FRAME_RATE;
   portdef.bFlagErrorConcealment = OMX_FALSE;
   portdef.eCompressionFormat = OMX_VIDEO_CodingAVC;
   portdef.eColorFormat = OMX_COLOR_FormatUnused;
   portdef.pNativeWindow = NULL;

   avctype.nSize = sizeof (OMX_VIDEO_PARAM_AVCTYPE);
   avctype.nVersion.nVersion = OMX_VERSION;
   avctype.nPortIndex = OMX_VID_DEC_AVC_INPUT_PORT_INDEX;
   avctype.eProfile = OMX_VIDEO_AVCProfileHigh;
    /* Encoder related, decide if need to initialise these */
   avctype.nSliceHeaderSpacing = 0;
   avctype.nPFrames = 0;
   avctype.nBFrames = 0;
   avctype.bUseHadamard = OMX_FALSE;
   avctype.nRefFrames = 1;
   avctype.nRefIdx10ActiveMinus1 = 1;
   avctype.nRefIdx11ActiveMinus1 = 0;
   avctype.bEnableUEP = OMX_FALSE;
   avctype.bEnableFMO = OMX_FALSE;
   avctype.bEnableASO = OMX_FALSE;
   avctype.bEnableRS = OMX_FALSE;
   avctype.eLevel = OMX_VIDEO_AVCLevel51;
   avctype.nAllowedPictureTypes = 2;
   avctype.bFrameMBsOnly = OMX_FALSE;
   avctype.bMBAFF = OMX_FALSE;
   avctype.bEntropyCodingCABAC = OMX_FALSE;
   avctype.bWeightedPPrediction = OMX_FALSE;
   avctype.nWeightedBipredicitonMode = 0;
   avctype.bconstIpred = OMX_FALSE;
   avctype.bDirect8x8Inference = OMX_FALSE;
   avctype.bDirectSpatialTemporal = OMX_FALSE;
   avctype.nCabacInitIdc = 0;
   avctype.eLoopFilterMode = OMX_VIDEO_AVCLoopFilterEnable;

   return factory_new (tiz_get_type(ap_hdl, "h264dinport"),
                       &avc_port_opts, &portdef,
                       &encodings, &formats, &avctype, &levels,
                       NULL  /* OMX_VIDEO_PARAM_BITRATETYPE */,
                       NULL  /* OMX_VIDEO_PARAM_QUANTIZATIONTYPE */);
}

OMX_PTR instantiate_h264d_output_port(OMX_HANDLETYPE ap_hdl)
{
   OMX_VIDEO_PORTDEFINITIONTYPE portdef;
   OMX_VIDEO_CODINGTYPE encodings[] = {
      OMX_VIDEO_CodingUnused,
      OMX_VIDEO_CodingMax
   };
   OMX_COLOR_FORMATTYPE formats[] = {
      OMX_COLOR_FormatYUV420SemiPlanar,
      OMX_COLOR_FormatMax
   };
   tiz_port_options_t rawvideo_port_opts = {
      OMX_PortDomainVideo,
      OMX_DirOutput,
      OMX_VID_DEC_AVC_OUTPUT_PORT_MIN_BUF_COUNT,
      OMX_VID_DEC_AVC_PORT_MIN_OUTPUT_BUF_SIZE,
      OMX_VID_DEC_AVC_PORT_NONCONTIGUOUS,
      OMX_VID_DEC_AVC_PORT_ALIGNMENT,
      OMX_VID_DEC_AVC_PORT_SUPPLIERPREF,
      {OMX_VID_DEC_AVC_OUTPUT_PORT_INDEX, NULL, NULL, NULL},
      0                     /* Master port */
   };

   portdef.pNativeRender = NULL;
   portdef.nFrameWidth = OMX_VID_DEC_AVC_DEFAULT_FRAME_WIDTH;
   portdef.nFrameHeight = OMX_VID_DEC_AVC_DEFAULT_FRAME_HEIGHT;
   portdef.nStride = 0;
   portdef.nSliceHeight = 0;
   portdef.nBitrate = 64000;
   portdef.xFramerate = OMX_VID_DEC_AVC_DEFAULT_FRAME_RATE;
   portdef.bFlagErrorConcealment = OMX_FALSE;
   portdef.eCompressionFormat = OMX_VIDEO_CodingUnused;
   portdef.eColorFormat = OMX_COLOR_FormatYUV420SemiPlanar;
   portdef.pNativeWindow = NULL;

   return factory_new(tiz_get_type(ap_hdl, "tizvideoport"),
                      &rawvideo_port_opts, &portdef,
                      &encodings, &formats);
}

OMX_PTR instantiate_h264d_config_port(OMX_HANDLETYPE ap_hdl)
{
   return factory_new(tiz_get_type(ap_hdl, "tizconfigport"),
                      NULL, /* this port does not take options */
                      OMX_VID_COMP_NAME, h264_decoder_version);
}

OMX_PTR instantiate_h264d_processor(OMX_HANDLETYPE ap_hdl)
{
   return factory_new(tiz_get_type(ap_hdl, "h264dprc"));
}
