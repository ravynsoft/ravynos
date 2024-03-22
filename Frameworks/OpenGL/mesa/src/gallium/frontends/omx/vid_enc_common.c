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

#include "vid_enc_common.h"

#include "vl/vl_video_buffer.h"
#include "tgsi/tgsi_text.h"

void enc_ReleaseTasks(struct list_head *head)
{
   struct encode_task *i, *next;

   if (!head || !list_is_linked(head))
      return;

   LIST_FOR_EACH_ENTRY_SAFE(i, next, head, list) {
      pipe_resource_reference(&i->bitstream, NULL);
      i->buf->destroy(i->buf);
      FREE(i);
   }
}

void enc_MoveTasks(struct list_head *from, struct list_head *to)
{
   to->prev->next = from->next;
   from->next->prev = to->prev;
   from->prev->next = to;
   to->prev = from->prev;
   list_inithead(from);
}

static void enc_GetPictureParamPreset(struct pipe_h264_enc_picture_desc *picture)
{
   picture->motion_est.enc_disable_sub_mode = 0x000000fe;
   picture->motion_est.enc_ime2_search_range_x = 0x00000001;
   picture->motion_est.enc_ime2_search_range_y = 0x00000001;
   picture->seq.enc_constraint_set_flags = 0x00000040;
}

enum pipe_video_profile enc_TranslateOMXProfileToPipe(unsigned omx_profile)
{
   switch (omx_profile) {
   case OMX_VIDEO_AVCProfileBaseline:
      return PIPE_VIDEO_PROFILE_MPEG4_AVC_BASELINE;
   case OMX_VIDEO_AVCProfileMain:
      return PIPE_VIDEO_PROFILE_MPEG4_AVC_MAIN;
   case OMX_VIDEO_AVCProfileExtended:
      return PIPE_VIDEO_PROFILE_MPEG4_AVC_EXTENDED;
   case OMX_VIDEO_AVCProfileHigh:
      return PIPE_VIDEO_PROFILE_MPEG4_AVC_HIGH;
   case OMX_VIDEO_AVCProfileHigh10:
      return PIPE_VIDEO_PROFILE_MPEG4_AVC_HIGH10;
   case OMX_VIDEO_AVCProfileHigh422:
      return PIPE_VIDEO_PROFILE_MPEG4_AVC_HIGH422;
   case OMX_VIDEO_AVCProfileHigh444:
      return PIPE_VIDEO_PROFILE_MPEG4_AVC_HIGH444;
   default:
      return PIPE_VIDEO_PROFILE_UNKNOWN;
   }
}

unsigned enc_TranslateOMXLevelToPipe(unsigned omx_level)
{
   switch (omx_level) {
   case OMX_VIDEO_AVCLevel1:
   case OMX_VIDEO_AVCLevel1b:
      return 10;
   case OMX_VIDEO_AVCLevel11:
      return 11;
   case OMX_VIDEO_AVCLevel12:
      return 12;
   case OMX_VIDEO_AVCLevel13:
      return 13;
   case OMX_VIDEO_AVCLevel2:
      return 20;
   case OMX_VIDEO_AVCLevel21:
      return 21;
   case OMX_VIDEO_AVCLevel22:
      return 22;
   case OMX_VIDEO_AVCLevel3:
      return 30;
   case OMX_VIDEO_AVCLevel31:
      return 31;
   case OMX_VIDEO_AVCLevel32:
      return 32;
   case OMX_VIDEO_AVCLevel4:
      return 40;
   case OMX_VIDEO_AVCLevel41:
      return 41;
   default:
   case OMX_VIDEO_AVCLevel42:
      return 42;
   case OMX_VIDEO_AVCLevel5:
      return 50;
   case OMX_VIDEO_AVCLevel51:
      return 51;
   }
}

void vid_enc_BufferEncoded_common(vid_enc_PrivateType * priv, OMX_BUFFERHEADERTYPE* input, OMX_BUFFERHEADERTYPE* output)
{
   struct output_buf_private *outp = output->pOutputPortPrivate;
   struct input_buf_private *inp = input->pInputPortPrivate;
   struct encode_task *task;
   struct pipe_box box = {};
   unsigned size;

#if ENABLE_ST_OMX_BELLAGIO
   if (!inp || list_is_empty(&inp->tasks)) {
      input->nFilledLen = 0; /* mark buffer as empty */
      enc_MoveTasks(&priv->used_tasks, &inp->tasks);
      return;
   }
#endif

   task = list_entry(inp->tasks.next, struct encode_task, list);
   list_del(&task->list);
   list_addtail(&task->list, &priv->used_tasks);

   if (!task->bitstream)
      return;

   /* ------------- map result buffer ----------------- */

   if (outp->transfer)
      pipe_buffer_unmap(priv->t_pipe, outp->transfer);

   pipe_resource_reference(&outp->bitstream, task->bitstream);
   pipe_resource_reference(&task->bitstream, NULL);

   box.width = outp->bitstream->width0;
   box.height = outp->bitstream->height0;
   box.depth = outp->bitstream->depth0;

   output->pBuffer = priv->t_pipe->buffer_map(priv->t_pipe, outp->bitstream, 0,
                                                PIPE_MAP_READ_WRITE,
                                                &box, &outp->transfer);

   /* ------------- get size of result ----------------- */

   priv->codec->get_feedback(priv->codec, task->feedback, &size, NULL);

   output->nOffset = 0;
   output->nFilledLen = size; /* mark buffer as full */

   /* all output buffers contain exactly one frame */
   output->nFlags = OMX_BUFFERFLAG_ENDOFFRAME;

#if ENABLE_ST_OMX_TIZONIA
   input->nFilledLen = 0; /* mark buffer as empty */
   enc_MoveTasks(&priv->used_tasks, &inp->tasks);
#endif
}


struct encode_task *enc_NeedTask_common(vid_enc_PrivateType * priv, OMX_VIDEO_PORTDEFINITIONTYPE *def)
{
   struct pipe_video_buffer templat = {};
   struct encode_task *task;

   if (!list_is_empty(&priv->free_tasks)) {
      task = list_entry(priv->free_tasks.next, struct encode_task, list);
      list_del(&task->list);
      return task;
   }

   /* allocate a new one */
   task = CALLOC_STRUCT(encode_task);
   if (!task)
      return NULL;

   templat.buffer_format = PIPE_FORMAT_NV12;
   templat.width = def->nFrameWidth;
   templat.height = def->nFrameHeight;
   templat.interlaced = false;

   task->buf = priv->s_pipe->create_video_buffer(priv->s_pipe, &templat);
   if (!task->buf) {
      FREE(task);
      return NULL;
   }

   return task;
}

void enc_ScaleInput_common(vid_enc_PrivateType * priv, OMX_VIDEO_PORTDEFINITIONTYPE *def,
                                  struct pipe_video_buffer **vbuf, unsigned *size)
{
   struct pipe_video_buffer *src_buf = *vbuf;
   struct vl_compositor *compositor = &priv->compositor;
   struct vl_compositor_state *s = &priv->cstate;
   struct pipe_sampler_view **views;
   struct pipe_surface **dst_surface;
   unsigned i;

   if (!priv->scale_buffer[priv->current_scale_buffer])
      return;

   views = src_buf->get_sampler_view_planes(src_buf);
   dst_surface = priv->scale_buffer[priv->current_scale_buffer]->get_surfaces
                 (priv->scale_buffer[priv->current_scale_buffer]);
   vl_compositor_clear_layers(s);

   for (i = 0; i < VL_MAX_SURFACES; ++i) {
      struct u_rect src_rect;
      if (!views[i] || !dst_surface[i])
         continue;
      src_rect.x0 = 0;
      src_rect.y0 = 0;
      src_rect.x1 = def->nFrameWidth;
      src_rect.y1 = def->nFrameHeight;
      if (i > 0) {
         src_rect.x1 /= 2;
         src_rect.y1 /= 2;
      }
      vl_compositor_set_rgba_layer(s, compositor, 0, views[i], &src_rect, NULL, NULL);
      vl_compositor_render(s, compositor, dst_surface[i], NULL, false);
   }
   *size  = priv->scale.xWidth * priv->scale.xHeight * 2;
   *vbuf = priv->scale_buffer[priv->current_scale_buffer++];
   priv->current_scale_buffer %= OMX_VID_ENC_NUM_SCALING_BUFFERS;
}

void enc_ControlPicture_common(vid_enc_PrivateType * priv, struct pipe_h264_enc_picture_desc *picture)
{
   struct pipe_h264_enc_rate_control *rate_ctrl = &picture->rate_ctrl[0];

   /* Get bitrate from port */
   switch (priv->bitrate.eControlRate) {
   case OMX_Video_ControlRateVariable:
      rate_ctrl->rate_ctrl_method = PIPE_H2645_ENC_RATE_CONTROL_METHOD_VARIABLE;
      break;
   case OMX_Video_ControlRateConstant:
      rate_ctrl->rate_ctrl_method = PIPE_H2645_ENC_RATE_CONTROL_METHOD_CONSTANT;
      break;
   case OMX_Video_ControlRateVariableSkipFrames:
      rate_ctrl->rate_ctrl_method = PIPE_H2645_ENC_RATE_CONTROL_METHOD_VARIABLE_SKIP;
      break;
   case OMX_Video_ControlRateConstantSkipFrames:
      rate_ctrl->rate_ctrl_method = PIPE_H2645_ENC_RATE_CONTROL_METHOD_CONSTANT_SKIP;
      break;
   default:
      rate_ctrl->rate_ctrl_method = PIPE_H2645_ENC_RATE_CONTROL_METHOD_DISABLE;
      break;
   }

   rate_ctrl->frame_rate_den = OMX_VID_ENC_CONTROL_FRAME_RATE_DEN_DEFAULT;
   rate_ctrl->frame_rate_num = ((priv->frame_rate) >> 16) * rate_ctrl->frame_rate_den;

   if (rate_ctrl->rate_ctrl_method != PIPE_H2645_ENC_RATE_CONTROL_METHOD_DISABLE) {
      if (priv->bitrate.nTargetBitrate < OMX_VID_ENC_BITRATE_MIN)
         rate_ctrl->target_bitrate = OMX_VID_ENC_BITRATE_MIN;
      else if (priv->bitrate.nTargetBitrate < OMX_VID_ENC_BITRATE_MAX)
         rate_ctrl->target_bitrate = priv->bitrate.nTargetBitrate;
      else
         rate_ctrl->target_bitrate = OMX_VID_ENC_BITRATE_MAX;
      rate_ctrl->peak_bitrate = rate_ctrl->target_bitrate;
      if (rate_ctrl->target_bitrate < OMX_VID_ENC_BITRATE_MEDIAN)
         rate_ctrl->vbv_buffer_size = MIN2((rate_ctrl->target_bitrate * 2.75), OMX_VID_ENC_BITRATE_MEDIAN);
      else
         rate_ctrl->vbv_buffer_size = rate_ctrl->target_bitrate;

      if (rate_ctrl->frame_rate_num) {
         unsigned long long t = rate_ctrl->target_bitrate;
         t *= rate_ctrl->frame_rate_den;
         rate_ctrl->target_bits_picture = t / rate_ctrl->frame_rate_num;
      } else {
         rate_ctrl->target_bits_picture = rate_ctrl->target_bitrate;
      }
      rate_ctrl->peak_bits_picture_integer = rate_ctrl->target_bits_picture;
      rate_ctrl->peak_bits_picture_fraction = 0;
   }

   picture->quant_i_frames = priv->quant.nQpI;
   picture->quant_p_frames = priv->quant.nQpP;
   picture->quant_b_frames = priv->quant.nQpB;

   picture->frame_num = priv->frame_num;
   picture->num_ref_idx_l0_active_minus1 = 0;
   picture->ref_idx_l0_list[0] = priv->ref_idx_l0;
   picture->num_ref_idx_l1_active_minus1 = 0;
   picture->ref_idx_l1_list[0] = priv->ref_idx_l1;
   picture->enable_vui = (picture->rate_ctrl[0].frame_rate_num != 0);
   enc_GetPictureParamPreset(picture);
}

static void *create_compute_state(struct pipe_context *pipe,
                                  const char *source)
{
   struct tgsi_token tokens[1024];
   struct pipe_compute_state state = {0};

   if (!tgsi_text_translate(source, tokens, ARRAY_SIZE(tokens))) {
           assert(false);
           return NULL;
   }

   state.ir_type = PIPE_SHADER_IR_TGSI;
   state.prog = tokens;

   return pipe->create_compute_state(pipe, &state);
}

void enc_InitCompute_common(vid_enc_PrivateType *priv)
{
   struct pipe_context *pipe = priv->s_pipe;
   struct pipe_screen *screen = pipe->screen;

   /* We need the partial last block support. */
   if (!screen->get_param(screen, PIPE_CAP_COMPUTE_GRID_INFO_LAST_BLOCK))
      return;

   static const char *copy_y =
         "COMP\n"
         "PROPERTY CS_FIXED_BLOCK_WIDTH 64\n"
         "PROPERTY CS_FIXED_BLOCK_HEIGHT 1\n"
         "PROPERTY CS_FIXED_BLOCK_DEPTH 1\n"
         "DCL SV[0], THREAD_ID\n"
         "DCL SV[1], BLOCK_ID\n"
         "DCL IMAGE[0], 2D, PIPE_FORMAT_R8_UINT\n"
         "DCL IMAGE[1], 2D, PIPE_FORMAT_R8_UINT, WR\n"
         "DCL TEMP[0..1]\n"
         "IMM[0] UINT32 {64, 0, 0, 0}\n"

         "UMAD TEMP[0].x, SV[1], IMM[0], SV[0]\n"
         "MOV TEMP[0].y, SV[1]\n"
         "LOAD TEMP[1].x, IMAGE[0], TEMP[0], 2D, PIPE_FORMAT_R8_UINT\n"
         "STORE IMAGE[1].x, TEMP[0], TEMP[1], 2D, PIPE_FORMAT_R8_UINT\n"
         "END\n";

   static const char *copy_uv =
         "COMP\n"
         "PROPERTY CS_FIXED_BLOCK_WIDTH 64\n"
         "PROPERTY CS_FIXED_BLOCK_HEIGHT 1\n"
         "PROPERTY CS_FIXED_BLOCK_DEPTH 1\n"
         "DCL SV[0], THREAD_ID\n"
         "DCL SV[1], BLOCK_ID\n"
         "DCL IMAGE[0], 2D, PIPE_FORMAT_R8_UINT\n"
         "DCL IMAGE[2], 2D, PIPE_FORMAT_R8G8_UINT, WR\n"
         "DCL CONST[0][0]\n" /* .x = offset of the UV portion in the y direction */
         "DCL TEMP[0..4]\n"
         "IMM[0] UINT32 {64, 0, 2, 1}\n"
         /* Destination R8G8 coordinates */
         "UMAD TEMP[0].x, SV[1], IMM[0], SV[0]\n"
         "MOV TEMP[0].y, SV[1]\n"
         /* Source R8 coordinates of U */
         "UMUL TEMP[1].x, TEMP[0], IMM[0].zzzz\n"
         "UADD TEMP[1].y, TEMP[0], CONST[0].xxxx\n"
         /* Source R8 coordinates of V */
         "UADD TEMP[2].x, TEMP[1], IMM[0].wwww\n"
         "MOV TEMP[2].y, TEMP[1]\n"

         "LOAD TEMP[3].x, IMAGE[0], TEMP[1], 2D, PIPE_FORMAT_R8_UINT\n"
         "LOAD TEMP[4].x, IMAGE[0], TEMP[2], 2D, PIPE_FORMAT_R8_UINT\n"
         "MOV TEMP[3].y, TEMP[4].xxxx\n"
         "STORE IMAGE[2], TEMP[0], TEMP[3], 2D, PIPE_FORMAT_R8G8_UINT\n"
         "END\n";

   priv->copy_y_shader = create_compute_state(pipe, copy_y);
   priv->copy_uv_shader = create_compute_state(pipe, copy_uv);
}

void enc_ReleaseCompute_common(vid_enc_PrivateType *priv)
{
   struct pipe_context *pipe = priv->s_pipe;

   if (priv->copy_y_shader)
      pipe->delete_compute_state(pipe, priv->copy_y_shader);
   if (priv->copy_uv_shader)
      pipe->delete_compute_state(pipe, priv->copy_uv_shader);
}

OMX_ERRORTYPE enc_LoadImage_common(vid_enc_PrivateType * priv, OMX_VIDEO_PORTDEFINITIONTYPE *def,
                                   OMX_BUFFERHEADERTYPE *buf,
                                   struct pipe_video_buffer *vbuf)
{
   struct pipe_context *pipe = priv->s_pipe;
   struct pipe_box box = {};
   struct input_buf_private *inp = buf->pInputPortPrivate;

   if (!inp->resource) {
      struct pipe_sampler_view **views;
      void *ptr;

      views = vbuf->get_sampler_view_planes(vbuf);
      if (!views)
         return OMX_ErrorInsufficientResources;

      ptr = buf->pBuffer;
      box.width = def->nFrameWidth;
      box.height = def->nFrameHeight;
      box.depth = 1;
      pipe->texture_subdata(pipe, views[0]->texture, 0,
                            PIPE_MAP_WRITE, &box,
                            ptr, def->nStride, 0);
      ptr = ((uint8_t*)buf->pBuffer) + (def->nStride * box.height);
      box.width = def->nFrameWidth / 2;
      box.height = def->nFrameHeight / 2;
      box.depth = 1;
      pipe->texture_subdata(pipe, views[1]->texture, 0,
                            PIPE_MAP_WRITE, &box,
                            ptr, def->nStride, 0);
   } else {
      struct vl_video_buffer *dst_buf = (struct vl_video_buffer *)vbuf;

      pipe_texture_unmap(pipe, inp->transfer);

      /* inp->resource uses PIPE_FORMAT_I8 and the layout looks like this:
       *
       * def->nFrameWidth = 4, def->nFrameHeight = 4:
       * |----|
       * |YYYY|
       * |YYYY|
       * |YYYY|
       * |YYYY|
       * |UVUV|
       * |UVUV|
       * |----|
       *
       * The copy has 2 steps:
       * - Copy Y to dst_buf->resources[0] as R8.
       * - Copy UV to dst_buf->resources[1] as R8G8.
       */
      if (priv->copy_y_shader && priv->copy_uv_shader) {
         /* Compute path */
         /* Set shader images for both copies. */
         struct pipe_image_view image[3] = {0};
         image[0].resource = inp->resource;
         image[0].shader_access = image[0].access = PIPE_IMAGE_ACCESS_READ;
         image[0].format = PIPE_FORMAT_R8_UINT;

         image[1].resource = dst_buf->resources[0];
         image[1].shader_access = image[1].access = PIPE_IMAGE_ACCESS_WRITE;
         image[1].format = PIPE_FORMAT_R8_UINT;

         image[2].resource = dst_buf->resources[1];
         image[2].shader_access = image[1].access = PIPE_IMAGE_ACCESS_WRITE;
         image[2].format = PIPE_FORMAT_R8G8_UINT;

         pipe->set_shader_images(pipe, PIPE_SHADER_COMPUTE, 0, 3, 0, image);

         /* Set the constant buffer. */
         uint32_t constants[4] = {def->nFrameHeight};
         struct pipe_constant_buffer cb = {};

         cb.buffer_size = sizeof(constants);
         cb.user_buffer = constants;
         pipe->set_constant_buffer(pipe, PIPE_SHADER_COMPUTE, 0, false, &cb);

         /* Use the optimal block size for the linear image layout. */
         struct pipe_grid_info info = {};
         info.block[0] = 64;
         info.block[1] = 1;
         info.block[2] = 1;
         info.grid[2] = 1;

         /* Copy Y */
         pipe->bind_compute_state(pipe, priv->copy_y_shader);

         info.grid[0] = DIV_ROUND_UP(def->nFrameWidth, 64);
         info.grid[1] = def->nFrameHeight;
         info.last_block[0] = def->nFrameWidth % 64;
         pipe->launch_grid(pipe, &info);

         /* Copy UV */
         pipe->bind_compute_state(pipe, priv->copy_uv_shader);

         info.grid[0] = DIV_ROUND_UP(def->nFrameWidth / 2, 64);
         info.grid[1] = def->nFrameHeight / 2;
         info.last_block[0] = (def->nFrameWidth / 2) % 64;
         pipe->launch_grid(pipe, &info);

         /* Make the result visible to all clients. */
         pipe->memory_barrier(pipe, PIPE_BARRIER_ALL);

         /* Unbind. */
         pipe->set_shader_images(pipe, PIPE_SHADER_COMPUTE, 0, 0, 3, NULL);
         pipe->set_constant_buffer(pipe, PIPE_SHADER_COMPUTE, 0, false, NULL);
         pipe->bind_compute_state(pipe, NULL);
      } else {
         /* Graphics path */
         struct pipe_blit_info blit;

         box.width = def->nFrameWidth;
         box.height = def->nFrameHeight;
         box.depth = 1;

         /* Copy Y */
         pipe->resource_copy_region(pipe,
                                    dst_buf->resources[0],
                                    0, 0, 0, 0, inp->resource, 0, &box);

         /* Copy U */
         memset(&blit, 0, sizeof(blit));
         blit.src.resource = inp->resource;
         blit.src.format = inp->resource->format;

         blit.src.box.x = -1;
         blit.src.box.y = def->nFrameHeight;
         blit.src.box.width = def->nFrameWidth;
         blit.src.box.height = def->nFrameHeight / 2 ;
         blit.src.box.depth = 1;

         blit.dst.resource = dst_buf->resources[1];
         blit.dst.format = blit.dst.resource->format;

         blit.dst.box.width = def->nFrameWidth / 2;
         blit.dst.box.height = def->nFrameHeight / 2;
         blit.dst.box.depth = 1;
         blit.filter = PIPE_TEX_FILTER_NEAREST;

         blit.mask = PIPE_MASK_R;
         pipe->blit(pipe, &blit);

         /* Copy V */
         blit.src.box.x = 0;
         blit.mask = PIPE_MASK_G;
         pipe->blit(pipe, &blit);
      }

      pipe->flush(pipe, NULL, 0);

      box.width = inp->resource->width0;
      box.height = inp->resource->height0;
      box.depth = inp->resource->depth0;
      buf->pBuffer = pipe->texture_map(pipe, inp->resource, 0,
                                        PIPE_MAP_WRITE, &box,
                                        &inp->transfer);
   }

   return OMX_ErrorNone;
}
