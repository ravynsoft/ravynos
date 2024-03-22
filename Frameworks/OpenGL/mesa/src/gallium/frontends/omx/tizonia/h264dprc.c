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

#include <tizplatform.h>
#include <tizkernel.h>
#include <tizutils.h>

#include "entrypoint.h"
#include "h264d.h"
#include "h264dprc.h"
#include "vid_omx_common.h"
#include "vid_dec_common.h"
#include "vid_dec_h264_common.h"

#include "vl/vl_video_buffer.h"
#include "vl/vl_compositor.h"
#include "util/u_hash_table.h"
#include "util/u_surface.h"

#include "dri_screen.h"
#include "egl_dri2.h"

unsigned dec_frame_delta;

static enum pipe_error hash_table_clear_item_callback(void *key, void *value, void *data)
{
   struct pipe_video_buffer *video_buffer = (struct pipe_video_buffer *)value;
   video_buffer->destroy(video_buffer);
   return PIPE_OK;
}

static void release_input_headers(vid_dec_PrivateType* priv) {
   int i;
   for (i = 0; i < priv->num_in_buffers; i++) {
      assert(!priv->in_port_disabled_);
      if (priv->in_buffers[i]->pInputPortPrivate) {
         vid_dec_FreeInputPortPrivate(priv->in_buffers[i]);
      }
      (void) tiz_krn_release_buffer (tiz_get_krn (handleOf (priv)),
                                     OMX_VID_DEC_AVC_INPUT_PORT_INDEX,
                                     priv->in_buffers[i]);
      priv->in_buffers[i] = NULL;
   }
   priv->p_inhdr_ = NULL;
   priv->num_in_buffers = 0;
}

static void release_output_header(vid_dec_PrivateType* priv) {
   if (priv->p_outhdr_) {
      assert(!priv->out_port_disabled_);
      (void) tiz_krn_release_buffer (tiz_get_krn (handleOf (priv)),
                                     OMX_VID_DEC_AVC_OUTPUT_PORT_INDEX,
                                     priv->p_outhdr_);
      priv->p_outhdr_ = NULL;
   }
}

static OMX_ERRORTYPE h264d_release_all_headers(vid_dec_PrivateType* priv)
{
   assert(priv);
   release_input_headers(priv);
   release_output_header(priv);

   return OMX_ErrorNone;
}

static void h264d_buffer_emptied(vid_dec_PrivateType* priv, OMX_BUFFERHEADERTYPE * p_hdr)
{
   assert(priv);
   assert(priv->in_buffers[0] == p_hdr);

   if (!priv->out_port_disabled_) {
      assert (p_hdr->nFilledLen == 0);
      p_hdr->nOffset = 0;

      if ((p_hdr->nFlags & OMX_BUFFERFLAG_EOS) != 0) {
         priv->eos_ = true;
      }

      (void) tiz_krn_release_buffer (tiz_get_krn (handleOf (priv)), 0, p_hdr);
      priv->p_inhdr_ = NULL;
      priv->in_buffers[0] = NULL;
   }
}

static void h264d_buffer_filled(vid_dec_PrivateType* priv, OMX_BUFFERHEADERTYPE * p_hdr)
{
   assert(priv);
   assert(p_hdr);
   assert(priv->p_outhdr_ == p_hdr);

   if (!priv->in_port_disabled_) {
      p_hdr->nOffset = 0;

      if (priv->eos_) {
         /* EOS has been received and all the input data has been consumed
          * already, so its time to propagate the EOS flag */
         priv->p_outhdr_->nFlags |= OMX_BUFFERFLAG_EOS;
         priv->eos_ = false;
      }

      (void) tiz_krn_release_buffer(tiz_get_krn (handleOf (priv)),
                                    OMX_VID_DEC_AVC_OUTPUT_PORT_INDEX,
                                    p_hdr);
      priv->p_outhdr_ = NULL;
   }
}

static bool h264d_shift_buffers_left(vid_dec_PrivateType* priv) {
   if (--priv->num_in_buffers) {
      priv->in_buffers[0] = priv->in_buffers[1];
      priv->sizes[0] = priv->sizes[1] - dec_frame_delta;
      priv->inputs[0] = priv->inputs[1] + dec_frame_delta;
      priv->timestamps[0] = priv->timestamps[1];

      return true;
   }
   return false;
}

static OMX_BUFFERHEADERTYPE * get_input_buffer(vid_dec_PrivateType* priv) {
   assert(priv);

   if (priv->in_port_disabled_) {
      return NULL;
   }

   if (priv->num_in_buffers > 1) {
      /* The input buffer wasn't cleared last time. */
      h264d_buffer_emptied(priv, priv->in_buffers[0]);
      if (priv->in_buffers[0]) {
         /* Failed to release buffer */
         return NULL;
      }
      h264d_shift_buffers_left(priv);
   }

   /* Decode_frame expects new buffers each time */
   assert(priv->p_inhdr_ || priv->first_buf_in_frame);
   tiz_krn_claim_buffer(tiz_get_krn (handleOf (priv)),
                        OMX_VID_DEC_AVC_INPUT_PORT_INDEX, 0,
                        &priv->p_inhdr_);
   return priv->p_inhdr_;
}

static struct pipe_resource * st_omx_pipe_texture_from_eglimage(EGLDisplay egldisplay,
                                                                EGLImage eglimage)
{
   _EGLDisplay *disp = egldisplay;
   struct dri2_egl_display *dri2_egl_dpy = disp->DriverData;
   __DRIscreen *_dri_screen = dri2_egl_dpy->dri_screen;
   struct dri_screen *st_dri_screen = dri_screen(_dri_screen);
   __DRIimage *_dri_image = st_dri_screen->lookup_egl_image(st_dri_screen, eglimage);

   return _dri_image->texture;
}

static void get_eglimage(vid_dec_PrivateType* priv) {
   OMX_PTR p_eglimage = NULL;
   OMX_NATIVE_WINDOWTYPE * p_egldisplay = NULL;
   const tiz_port_t * p_port = NULL;
   struct pipe_video_buffer templat = {};
   struct pipe_video_buffer *video_buffer = NULL;
   struct pipe_resource * p_res = NULL;
   struct pipe_resource *resources[VL_NUM_COMPONENTS];

   if (OMX_ErrorNone ==
      tiz_krn_claim_eglimage(tiz_get_krn (handleOf (priv)),
                             OMX_VID_DEC_AVC_OUTPUT_PORT_INDEX,
                             priv->p_outhdr_, &p_eglimage)) {
      priv->use_eglimage = true;
      p_port = tiz_krn_get_port(tiz_get_krn (handleOf (priv)),
                                OMX_VID_DEC_AVC_OUTPUT_PORT_INDEX);
      p_egldisplay = p_port->portdef_.format.video.pNativeWindow;

      if (!util_hash_table_get(priv->video_buffer_map, priv->p_outhdr_)) {
        p_res = st_omx_pipe_texture_from_eglimage(p_egldisplay, p_eglimage);

        assert(p_res);

        memset(&templat, 0, sizeof(templat));
        templat.buffer_format = p_res->format;
        templat.width = p_res->width0;
        templat.height = p_res->height0;
        templat.interlaced = 0;

        memset(resources, 0, sizeof(resources));
        pipe_resource_reference(&resources[0], p_res);

        video_buffer = vl_video_buffer_create_ex2(priv->pipe, &templat, resources);

        assert(video_buffer);
        assert(video_buffer->buffer_format == p_res->format);

        _mesa_hash_table_insert(priv->video_buffer_map, priv->p_outhdr_, video_buffer);
      }
   } else {
      (void) tiz_krn_release_buffer(tiz_get_krn (handleOf (priv)),
                                    OMX_VID_DEC_AVC_OUTPUT_PORT_INDEX,
                                    priv->p_outhdr_);
      priv->p_outhdr_ = NULL;
   }
}

static OMX_BUFFERHEADERTYPE * get_output_buffer(vid_dec_PrivateType* priv) {
   assert (priv);

   if (priv->out_port_disabled_) {
      return NULL;
   }

   if (!priv->p_outhdr_) {
      if (OMX_ErrorNone
          == tiz_krn_claim_buffer(tiz_get_krn (handleOf (priv)),
                                  OMX_VID_DEC_AVC_OUTPUT_PORT_INDEX, 0,
                                  &priv->p_outhdr_)) {
         if (priv->p_outhdr_) {
            /* Check pBuffer nullity to know if an eglimage has been registered. */
            if (!priv->p_outhdr_->pBuffer) {
               get_eglimage(priv);
            }
         }
      }
   }
   return priv->p_outhdr_;
}

static void reset_stream_parameters(vid_dec_PrivateType* apriv)
{
   assert(apriv);
   TIZ_INIT_OMX_PORT_STRUCT(apriv->out_port_def_,
                            OMX_VID_DEC_AVC_OUTPUT_PORT_INDEX);

   tiz_api_GetParameter (tiz_get_krn (handleOf (apriv)), handleOf (apriv),
                          OMX_IndexParamPortDefinition, &(apriv->out_port_def_));

   apriv->p_inhdr_ = 0;
   apriv->num_in_buffers = 0;
   apriv->first_buf_in_frame = true;
   apriv->eos_ = false;
   apriv->frame_finished = false;
   apriv->frame_started = false;
   apriv->picture.h264.field_order_cnt[0] = apriv->picture.h264.field_order_cnt[1] = INT_MAX;
   apriv->slice = NULL;
}

/* Replacement for bellagio's omx_base_filter_BufferMgmtFunction */
static void h264d_manage_buffers(vid_dec_PrivateType* priv) {
   bool next_is_eos = priv->num_in_buffers == 2 ? !!(priv->in_buffers[1]->nFlags & OMX_BUFFERFLAG_EOS) : false;
   vid_dec_FrameDecoded_common(priv, priv->in_buffers[0], priv->p_outhdr_);

   priv->p_outhdr_->nTimeStamp = priv->in_buffers[0]->nTimeStamp;

   /* Realase output buffer if filled or eos
      Keep if two input buffers are being decoded */
   if ((!next_is_eos) && ((priv->p_outhdr_->nFilledLen > 0) || priv->use_eglimage  || priv->eos_)) {
      h264d_buffer_filled(priv, priv->p_outhdr_);
   }

   /* Release input buffer if possible */
   if (priv->in_buffers[0]->nFilledLen == 0) {
      h264d_buffer_emptied(priv, priv->in_buffers[0]);
   }
}

static OMX_ERRORTYPE decode_frame(vid_dec_PrivateType*priv,
                                  OMX_BUFFERHEADERTYPE *in_buf)
{
   unsigned i = priv->num_in_buffers++;
   priv->in_buffers[i] = in_buf;
   priv->sizes[i] = in_buf->nFilledLen;
   priv->inputs[i] = in_buf->pBuffer;
   priv->timestamps[i] = in_buf->nTimeStamp;

   while (priv->num_in_buffers > (!!(in_buf->nFlags & OMX_BUFFERFLAG_EOS) ? 0 : 1)) {
      priv->eos_ = !!(priv->in_buffers[0]->nFlags & OMX_BUFFERFLAG_EOS);
      unsigned min_bits_left = priv->eos_ ? 32 : MAX2(in_buf->nFilledLen * 8, 32);
      struct vl_vlc vlc;

      vl_vlc_init(&vlc, priv->num_in_buffers, priv->inputs, priv->sizes);

      if (priv->slice)
         priv->bytes_left = vl_vlc_bits_left(&vlc) / 8;

      while (vl_vlc_bits_left (&vlc) > min_bits_left) {
         vid_dec_h264_Decode(priv, &vlc, min_bits_left);
         vl_vlc_fillbits(&vlc);
      }

      if (priv->slice) {
         unsigned bytes = priv->bytes_left - vl_vlc_bits_left(&vlc) / 8;

         priv->codec->decode_bitstream(priv->codec, priv->target, &priv->picture.base,
                                 1, &priv->slice, &bytes);

         if (priv->num_in_buffers)
            priv->slice = priv->inputs[1];
         else
            priv->slice = NULL;
      }

      if (priv->eos_ && priv->frame_started)
         vid_dec_h264_EndFrame(priv);

      if (priv->frame_finished) {
         priv->frame_finished = false;
         h264d_manage_buffers(priv);
      } else if (priv->eos_) {
         vid_dec_FreeInputPortPrivate(priv->in_buffers[0]);
         h264d_manage_buffers(priv);
      } else {
         priv->in_buffers[0]->nFilledLen = 0;
         h264d_buffer_emptied(priv, priv->in_buffers[0]);
      }

      if (priv->out_port_disabled_) {
         /* In case out port is disabled, h264d_buffer_emptied will fail to release input port.
          * We need to wait before shifting the buffers in that case and check in
          * get_input_buffer when out port is enabled to release and shift the buffers.
          * Infinite looping occurs if buffer is not released */
         if (priv->num_in_buffers == 2) {
            /* Set the delta value for use in get_input_buffer before exiting */
            dec_frame_delta = MIN2((min_bits_left - vl_vlc_bits_left(&vlc)) / 8, priv->sizes[1]);
         }
         break;
      }

      h264d_shift_buffers_left(priv);
   }

   return OMX_ErrorNone;
}

/*
 * h264dprc
 */

static void * h264d_prc_ctor(void *ap_obj, va_list * app)
{
   vid_dec_PrivateType*priv = super_ctor(typeOf (ap_obj, "h264dprc"), ap_obj, app);
   assert(priv);
   priv->p_inhdr_ = 0;
   priv->p_outhdr_ = 0;
   priv->first_buf_in_frame = true;
   priv->eos_ = false;
   priv->in_port_disabled_   = false;
   priv->out_port_disabled_   = false;
   priv->picture.base.profile = PIPE_VIDEO_PROFILE_MPEG4_AVC_HIGH;
   priv->profile = PIPE_VIDEO_PROFILE_MPEG4_AVC_HIGH;
   reset_stream_parameters(priv);

   return priv;
}

static void * h264d_prc_dtor(void *ap_obj)
{
   return super_dtor(typeOf(ap_obj, "h264dprc"), ap_obj);
}

static OMX_ERRORTYPE h264d_prc_allocate_resources(void *ap_obj, OMX_U32 a_pid)
{
   vid_dec_PrivateType*priv = ap_obj;
   struct pipe_screen *screen;
   vl_csc_matrix csc;

   assert (priv);

   priv->screen = omx_get_screen();
   if (!priv->screen)
      return OMX_ErrorInsufficientResources;

   screen = priv->screen->pscreen;
   priv->pipe = pipe_create_multimedia_context(screen);
   if (!priv->pipe)
      return OMX_ErrorInsufficientResources;

   if (!vl_compositor_init(&priv->compositor, priv->pipe)) {
      priv->pipe->destroy(priv->pipe);
      priv->pipe = NULL;
      return OMX_ErrorInsufficientResources;
   }

   if (!vl_compositor_init_state(&priv->cstate, priv->pipe)) {
      vl_compositor_cleanup(&priv->compositor);
      priv->pipe->destroy(priv->pipe);
      priv->pipe = NULL;
      return OMX_ErrorInsufficientResources;
   }

   vl_csc_get_matrix(VL_CSC_COLOR_STANDARD_BT_601, NULL, true, &csc);
   if (!vl_compositor_set_csc_matrix(&priv->cstate, (const vl_csc_matrix *)&csc, 1.0f, 0.0f)) {
      vl_compositor_cleanup(&priv->compositor);
      priv->pipe->destroy(priv->pipe);
      priv->pipe = NULL;
      return OMX_ErrorInsufficientResources;
   }

   list_inithead(&priv->codec_data.h264.dpb_list);

   priv->video_buffer_map = util_hash_table_create_ptr_keys();

   return OMX_ErrorNone;
}

static OMX_ERRORTYPE h264d_prc_deallocate_resources(void *ap_obj)
{
   vid_dec_PrivateType*priv = ap_obj;
   assert(priv);

   /* Clear hash table */
   util_hash_table_foreach(priv->video_buffer_map,
                            &hash_table_clear_item_callback,
                            NULL);
   _mesa_hash_table_destroy(priv->video_buffer_map, NULL);

   if (priv->pipe) {
      vl_compositor_cleanup_state(&priv->cstate);
      vl_compositor_cleanup(&priv->compositor);
      priv->pipe->destroy(priv->pipe);
   }

   if (priv->screen)
      omx_put_screen();

   return OMX_ErrorNone;
}

static OMX_ERRORTYPE h264d_prc_prepare_to_transfer(void *ap_obj, OMX_U32 a_pid)
{
   vid_dec_PrivateType*priv = ap_obj;
   assert(priv);

   TIZ_INIT_OMX_PORT_STRUCT(priv->out_port_def_,
                            OMX_VID_DEC_AVC_OUTPUT_PORT_INDEX);
   tiz_check_omx(
      tiz_api_GetParameter(tiz_get_krn(handleOf(priv)), handleOf(priv),
                           OMX_IndexParamPortDefinition, &(priv->out_port_def_)));

   priv->first_buf_in_frame = true;
   priv->eos_ = false;
   return OMX_ErrorNone;
}

static OMX_ERRORTYPE h264d_prc_transfer_and_process(void *ap_obj, OMX_U32 a_pid)
{
   return OMX_ErrorNone;
}

static OMX_ERRORTYPE h264d_prc_stop_and_return(void *ap_obj)
{
   vid_dec_PrivateType*priv = (vid_dec_PrivateType*) ap_obj;
   return h264d_release_all_headers (priv);
}

static OMX_ERRORTYPE h264d_prc_buffers_ready(const void *ap_obj)
{
   vid_dec_PrivateType*priv = (vid_dec_PrivateType*) ap_obj;
   OMX_BUFFERHEADERTYPE *in_buf = NULL;
   OMX_BUFFERHEADERTYPE *out_buf = NULL;

   assert(priv);

   /* Set parameters if start of stream */
   if (!priv->eos_ && priv->first_buf_in_frame && (in_buf = get_input_buffer(priv))) {
      decode_frame(priv, in_buf);
   }

   /* Don't get input buffer if output buffer not found */
   while (!priv->eos_ && (out_buf = get_output_buffer(priv)) && (in_buf = get_input_buffer(priv))) {
      if (!priv->out_port_disabled_) {
         decode_frame(priv, in_buf);
      }
   }

   return OMX_ErrorNone;
}

static OMX_ERRORTYPE h264d_prc_port_flush(const void *ap_obj, OMX_U32 a_pid)
{
   vid_dec_PrivateType*priv = (vid_dec_PrivateType*) ap_obj;
   if (OMX_ALL == a_pid || OMX_VID_DEC_AVC_INPUT_PORT_INDEX == a_pid) {
      release_input_headers(priv);
      reset_stream_parameters(priv);
   }
   if (OMX_ALL == a_pid || OMX_VID_DEC_AVC_OUTPUT_PORT_INDEX == a_pid) {
      release_output_header(priv);
   }
   return OMX_ErrorNone;
}

static OMX_ERRORTYPE h264d_prc_port_disable(const void *ap_obj, OMX_U32 a_pid)
{
   vid_dec_PrivateType*priv = (vid_dec_PrivateType*) ap_obj;
   assert(priv);
   if (OMX_ALL == a_pid || OMX_VID_DEC_AVC_INPUT_PORT_INDEX == a_pid) {
      /* Release all buffers */
      h264d_release_all_headers(priv);
      reset_stream_parameters(priv);
      priv->in_port_disabled_ = true;
   }
   if (OMX_ALL == a_pid || OMX_VID_DEC_AVC_OUTPUT_PORT_INDEX == a_pid) {
      release_output_header(priv);
      priv->out_port_disabled_ = true;
   }
   return OMX_ErrorNone;
}

static OMX_ERRORTYPE h264d_prc_port_enable(const void *ap_obj, OMX_U32 a_pid)
{
   vid_dec_PrivateType* priv = (vid_dec_PrivateType*) ap_obj;
   assert(priv);
   if (OMX_ALL == a_pid || OMX_VID_DEC_AVC_INPUT_PORT_INDEX == a_pid) {
      if (priv->in_port_disabled_) {
         reset_stream_parameters(priv);
         priv->in_port_disabled_ = false;
      }
   }
   if (OMX_ALL == a_pid || OMX_VID_DEC_AVC_OUTPUT_PORT_INDEX == a_pid) {
      priv->out_port_disabled_ = false;
   }
   return OMX_ErrorNone;
}

/*
 * h264d_prc_class
 */

static void * h264d_prc_class_ctor(void *ap_obj, va_list * app)
{
   /* NOTE: Class methods might be added in the future. None for now. */
   return super_ctor(typeOf(ap_obj, "h264dprc_class"), ap_obj, app);
}

/*
 * initialization
 */

void * h264d_prc_class_init(void * ap_tos, void * ap_hdl)
{
   void * tizprc = tiz_get_type(ap_hdl, "tizprc");
   void * h264dprc_class = factory_new
      /* TIZ_CLASS_COMMENT: class type, class name, parent, size */
      (classOf(tizprc), "h264dprc_class", classOf(tizprc),
       sizeof(h264d_prc_class_t),
       /* TIZ_CLASS_COMMENT: */
       ap_tos, ap_hdl,
       /* TIZ_CLASS_COMMENT: class constructor */
       ctor, h264d_prc_class_ctor,
       /* TIZ_CLASS_COMMENT: stop value*/
       0);
   return h264dprc_class;
}

void * h264d_prc_init(void * ap_tos, void * ap_hdl)
{
   void * tizprc = tiz_get_type(ap_hdl, "tizprc");
   void * h264dprc_class = tiz_get_type(ap_hdl, "h264dprc_class");
   TIZ_LOG_CLASS (h264dprc_class);
   void * h264dprc = factory_new
     /* TIZ_CLASS_COMMENT: class type, class name, parent, size */
     (h264dprc_class, "h264dprc", tizprc, sizeof(vid_dec_PrivateType),
      /* TIZ_CLASS_COMMENT: */
      ap_tos, ap_hdl,
      /* TIZ_CLASS_COMMENT: class constructor */
      ctor, h264d_prc_ctor,
      /* TIZ_CLASS_COMMENT: class destructor */
      dtor, h264d_prc_dtor,
      /* TIZ_CLASS_COMMENT: */
      tiz_srv_allocate_resources, h264d_prc_allocate_resources,
      /* TIZ_CLASS_COMMENT: */
      tiz_srv_deallocate_resources, h264d_prc_deallocate_resources,
      /* TIZ_CLASS_COMMENT: */
      tiz_srv_prepare_to_transfer, h264d_prc_prepare_to_transfer,
      /* TIZ_CLASS_COMMENT: */
      tiz_srv_transfer_and_process, h264d_prc_transfer_and_process,
      /* TIZ_CLASS_COMMENT: */
      tiz_srv_stop_and_return, h264d_prc_stop_and_return,
      /* TIZ_CLASS_COMMENT: */
      tiz_prc_buffers_ready, h264d_prc_buffers_ready,
      /* TIZ_CLASS_COMMENT: */
      tiz_prc_port_flush, h264d_prc_port_flush,
      /* TIZ_CLASS_COMMENT: */
      tiz_prc_port_disable, h264d_prc_port_disable,
      /* TIZ_CLASS_COMMENT: */
      tiz_prc_port_enable, h264d_prc_port_enable,
      /* TIZ_CLASS_COMMENT: stop value*/
      0);

   return h264dprc;
}
