/*
 * Copyright © 2021 Red Hat
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the next
 * paragraph) shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 */

#include "vk_video.h"
#include "vk_util.h"
#include "vk_log.h"
#include "vk_alloc.h"
#include "vk_device.h"
#include "util/vl_rbsp.h"
#include "util/vl_bitstream.h"

VkResult
vk_video_session_init(struct vk_device *device,
                      struct vk_video_session *vid,
                      const VkVideoSessionCreateInfoKHR *create_info)
{
   vk_object_base_init(device, &vid->base, VK_OBJECT_TYPE_VIDEO_SESSION_KHR);

   vid->flags = create_info->flags;
   vid->op = create_info->pVideoProfile->videoCodecOperation;
   vid->max_coded = create_info->maxCodedExtent;
   vid->picture_format = create_info->pictureFormat;
   vid->ref_format = create_info->referencePictureFormat;
   vid->max_dpb_slots = create_info->maxDpbSlots;
   vid->max_active_ref_pics = create_info->maxActiveReferencePictures;

   switch (vid->op) {
   case VK_VIDEO_CODEC_OPERATION_DECODE_H264_BIT_KHR: {
      const struct VkVideoDecodeH264ProfileInfoKHR *h264_profile =
         vk_find_struct_const(create_info->pVideoProfile->pNext,
                              VIDEO_DECODE_H264_PROFILE_INFO_KHR);
      vid->h264.profile_idc = h264_profile->stdProfileIdc;
      break;
   }
   case VK_VIDEO_CODEC_OPERATION_DECODE_H265_BIT_KHR: {
      const struct VkVideoDecodeH265ProfileInfoKHR *h265_profile =
         vk_find_struct_const(create_info->pVideoProfile->pNext,
                              VIDEO_DECODE_H265_PROFILE_INFO_KHR);
      vid->h265.profile_idc = h265_profile->stdProfileIdc;
      break;
   }
   case VK_VIDEO_CODEC_OPERATION_ENCODE_H264_BIT_KHR: {
      const struct VkVideoEncodeH264ProfileInfoKHR *h264_profile =
         vk_find_struct_const(create_info->pVideoProfile->pNext, VIDEO_ENCODE_H264_PROFILE_INFO_KHR);
      vid->h264.profile_idc = h264_profile->stdProfileIdc;
      break;
   }
   case VK_VIDEO_CODEC_OPERATION_ENCODE_H265_BIT_KHR: {
      const struct VkVideoEncodeH265ProfileInfoKHR *h265_profile =
         vk_find_struct_const(create_info->pVideoProfile->pNext, VIDEO_ENCODE_H265_PROFILE_INFO_KHR);
      vid->h265.profile_idc = h265_profile->stdProfileIdc;
      break;
   }
   default:
      return VK_ERROR_FEATURE_NOT_PRESENT;
   }

   if (vid->op == VK_VIDEO_CODEC_OPERATION_ENCODE_H264_BIT_KHR ||
       vid->op == VK_VIDEO_CODEC_OPERATION_ENCODE_H265_BIT_KHR) {
      const struct VkVideoEncodeUsageInfoKHR *encode_usage_profile =
         vk_find_struct_const(create_info->pVideoProfile->pNext, VIDEO_ENCODE_USAGE_INFO_KHR);
      if (encode_usage_profile) {
         vid->enc_usage.video_usage_hints = encode_usage_profile->videoUsageHints;
         vid->enc_usage.video_content_hints = encode_usage_profile->videoContentHints;
         vid->enc_usage.tuning_mode = encode_usage_profile->tuningMode;
      } else {
         vid->enc_usage.video_usage_hints = VK_VIDEO_ENCODE_USAGE_DEFAULT_KHR;
         vid->enc_usage.video_content_hints = VK_VIDEO_ENCODE_CONTENT_DEFAULT_KHR;
         vid->enc_usage.tuning_mode = VK_VIDEO_ENCODE_TUNING_MODE_DEFAULT_KHR;
      }
   }

   return VK_SUCCESS;
}

#define FIND(PARAMSET, SS, SET, ID)                                     \
   static PARAMSET *find_##SS##_##SET(const struct vk_video_session_parameters *params, uint32_t id) { \
      for (unsigned i = 0; i < params->SS.SET##_count; i++) {           \
         if (params->SS.SET[i].ID == id)                                \
            return &params->SS.SET[i];                                  \
      }                                                                 \
      return NULL;                                                      \
   }                                                                    \
                                                                        \
   static void add_##SS##_##SET(struct vk_video_session_parameters *params, \
                                const PARAMSET *new_set, bool noreplace) {  \
      PARAMSET *set = find_##SS##_##SET(params, new_set->ID);           \
      if (set) {                                                        \
	 if (noreplace)                                                 \
            return;                                                     \
         *set = *new_set;                                               \
      } else                                                            \
         params->SS.SET[params->SS.SET##_count++] = *new_set;           \
   }                                                                    \
                                                                        \
   static VkResult update_##SS##_##SET(struct vk_video_session_parameters *params, \
                                       uint32_t count, const PARAMSET *updates) { \
      if (params->SS.SET##_count + count >= params->SS.max_##SET##_count) \
         return VK_ERROR_TOO_MANY_OBJECTS;                              \
      typed_memcpy(&params->SS.SET[params->SS.SET##_count], updates, count); \
      params->SS.SET##_count += count;                                  \
      return VK_SUCCESS;                                                \
   }

FIND(StdVideoH264SequenceParameterSet, h264_dec, std_sps, seq_parameter_set_id)
FIND(StdVideoH264PictureParameterSet, h264_dec, std_pps, pic_parameter_set_id)
FIND(StdVideoH265VideoParameterSet, h265_dec, std_vps, vps_video_parameter_set_id)
FIND(StdVideoH265SequenceParameterSet, h265_dec, std_sps, sps_seq_parameter_set_id)
FIND(StdVideoH265PictureParameterSet, h265_dec, std_pps, pps_pic_parameter_set_id)

FIND(StdVideoH264SequenceParameterSet, h264_enc, std_sps, seq_parameter_set_id)
FIND(StdVideoH264PictureParameterSet, h264_enc, std_pps, pic_parameter_set_id)

FIND(StdVideoH265VideoParameterSet, h265_enc, std_vps, vps_video_parameter_set_id)
FIND(StdVideoH265SequenceParameterSet, h265_enc, std_sps, sps_seq_parameter_set_id)
FIND(StdVideoH265PictureParameterSet, h265_enc, std_pps, pps_pic_parameter_set_id)

static void
init_add_h264_dec_session_parameters(struct vk_video_session_parameters *params,
                                     const struct VkVideoDecodeH264SessionParametersAddInfoKHR *h264_add,
                                     const struct vk_video_session_parameters *templ)
{
   unsigned i;

   if (h264_add) {
      for (i = 0; i < h264_add->stdSPSCount; i++) {
         add_h264_dec_std_sps(params, &h264_add->pStdSPSs[i], false);
      }
   }
   if (templ) {
      for (i = 0; i < templ->h264_dec.std_sps_count; i++) {
         add_h264_dec_std_sps(params, &templ->h264_dec.std_sps[i], true);
      }
   }

   if (h264_add) {
      for (i = 0; i < h264_add->stdPPSCount; i++) {
         add_h264_dec_std_pps(params, &h264_add->pStdPPSs[i], false);
      }
   }
   if (templ) {
      for (i = 0; i < templ->h264_dec.std_pps_count; i++) {
         add_h264_dec_std_pps(params, &templ->h264_dec.std_pps[i], true);
      }
   }
}

static void
init_add_h264_enc_session_parameters(struct vk_video_session_parameters *params,
                                     const struct VkVideoEncodeH264SessionParametersAddInfoKHR *h264_add,
                                     const struct vk_video_session_parameters *templ)
{
   unsigned i;
   if (h264_add) {
      for (i = 0; i < h264_add->stdSPSCount; i++) {
         add_h264_enc_std_sps(params, &h264_add->pStdSPSs[i], false);
      }
   }
   if (templ) {
      for (i = 0; i < templ->h264_dec.std_sps_count; i++) {
         add_h264_enc_std_sps(params, &templ->h264_enc.std_sps[i], true);
      }
   }

   if (h264_add) {
      for (i = 0; i < h264_add->stdPPSCount; i++) {
         add_h264_enc_std_pps(params, &h264_add->pStdPPSs[i], false);
      }
   }
   if (templ) {
      for (i = 0; i < templ->h264_enc.std_pps_count; i++) {
         add_h264_enc_std_pps(params, &templ->h264_enc.std_pps[i], true);
      }
   }
}

static void
init_add_h265_dec_session_parameters(struct vk_video_session_parameters *params,
                                 const struct VkVideoDecodeH265SessionParametersAddInfoKHR *h265_add,
                                 const struct vk_video_session_parameters *templ)
{
   unsigned i;

   if (h265_add) {
      for (i = 0; i < h265_add->stdVPSCount; i++) {
         add_h265_dec_std_vps(params, &h265_add->pStdVPSs[i], false);
      }
   }
   if (templ) {
      for (i = 0; i < templ->h265_dec.std_vps_count; i++) {
         add_h265_dec_std_vps(params, &templ->h265_dec.std_vps[i], true);
      }
   }
   if (h265_add) {
      for (i = 0; i < h265_add->stdSPSCount; i++) {
         add_h265_dec_std_sps(params, &h265_add->pStdSPSs[i], false);
      }
   }
   if (templ) {
      for (i = 0; i < templ->h265_dec.std_sps_count; i++) {
         add_h265_dec_std_sps(params, &templ->h265_dec.std_sps[i], true);
      }
   }

   if (h265_add) {
      for (i = 0; i < h265_add->stdPPSCount; i++) {
         add_h265_dec_std_pps(params, &h265_add->pStdPPSs[i], false);
      }
   }
   if (templ) {
      for (i = 0; i < templ->h265_dec.std_pps_count; i++) {
         add_h265_dec_std_pps(params, &templ->h265_dec.std_pps[i], true);
      }
   }
}

static void
init_add_h265_enc_session_parameters(struct vk_video_session_parameters *params,
                                     const struct VkVideoEncodeH265SessionParametersAddInfoKHR *h265_add,
                                     const struct vk_video_session_parameters *templ)
{
   unsigned i;

   if (h265_add) {
      for (i = 0; i < h265_add->stdVPSCount; i++) {
         add_h265_enc_std_vps(params, &h265_add->pStdVPSs[i], false);
      }
   }
   if (templ) {
      for (i = 0; i < templ->h265_enc.std_vps_count; i++) {
         add_h265_enc_std_vps(params, &templ->h265_enc.std_vps[i], true);
      }
   }
   if (h265_add) {
      for (i = 0; i < h265_add->stdSPSCount; i++) {
         add_h265_enc_std_sps(params, &h265_add->pStdSPSs[i], false);
      }
   }
   if (templ) {
      for (i = 0; i < templ->h265_enc.std_sps_count; i++) {
         add_h265_enc_std_sps(params, &templ->h265_enc.std_sps[i], true);
      }
   }

   if (h265_add) {
      for (i = 0; i < h265_add->stdPPSCount; i++) {
         add_h265_enc_std_pps(params, &h265_add->pStdPPSs[i], false);
      }
   }
   if (templ) {
      for (i = 0; i < templ->h265_enc.std_pps_count; i++) {
         add_h265_enc_std_pps(params, &templ->h265_enc.std_pps[i], true);
      }
   }
}

VkResult
vk_video_session_parameters_init(struct vk_device *device,
                                 struct vk_video_session_parameters *params,
                                 const struct vk_video_session *vid,
                                 const struct vk_video_session_parameters *templ,
                                 const VkVideoSessionParametersCreateInfoKHR *create_info)
{
   memset(params, 0, sizeof(*params));
   vk_object_base_init(device, &params->base, VK_OBJECT_TYPE_VIDEO_SESSION_PARAMETERS_KHR);

   params->op = vid->op;

   switch (vid->op) {
   case VK_VIDEO_CODEC_OPERATION_DECODE_H264_BIT_KHR: {
      const struct VkVideoDecodeH264SessionParametersCreateInfoKHR *h264_create =
         vk_find_struct_const(create_info->pNext, VIDEO_DECODE_H264_SESSION_PARAMETERS_CREATE_INFO_KHR);

      params->h264_dec.max_std_sps_count = h264_create->maxStdSPSCount;
      params->h264_dec.max_std_pps_count = h264_create->maxStdPPSCount;

      uint32_t sps_size = params->h264_dec.max_std_sps_count * sizeof(StdVideoH264SequenceParameterSet);
      uint32_t pps_size = params->h264_dec.max_std_pps_count * sizeof(StdVideoH264PictureParameterSet);

      params->h264_dec.std_sps = vk_alloc(&device->alloc, sps_size, 8, VK_SYSTEM_ALLOCATION_SCOPE_OBJECT);
      params->h264_dec.std_pps = vk_alloc(&device->alloc, pps_size, 8, VK_SYSTEM_ALLOCATION_SCOPE_OBJECT);
      if (!params->h264_dec.std_sps || !params->h264_dec.std_pps) {
         vk_free(&device->alloc, params->h264_dec.std_sps);
         vk_free(&device->alloc, params->h264_dec.std_pps);
         return vk_error(device, VK_ERROR_OUT_OF_HOST_MEMORY);
      }

      init_add_h264_dec_session_parameters(params, h264_create->pParametersAddInfo, templ);
      break;
   }
   case VK_VIDEO_CODEC_OPERATION_DECODE_H265_BIT_KHR: {
      const struct VkVideoDecodeH265SessionParametersCreateInfoKHR *h265_create =
         vk_find_struct_const(create_info->pNext, VIDEO_DECODE_H265_SESSION_PARAMETERS_CREATE_INFO_KHR);

      params->h265_dec.max_std_vps_count = h265_create->maxStdVPSCount;
      params->h265_dec.max_std_sps_count = h265_create->maxStdSPSCount;
      params->h265_dec.max_std_pps_count = h265_create->maxStdPPSCount;

      uint32_t vps_size = params->h265_dec.max_std_vps_count * sizeof(StdVideoH265VideoParameterSet);
      uint32_t sps_size = params->h265_dec.max_std_sps_count * sizeof(StdVideoH265SequenceParameterSet);
      uint32_t pps_size = params->h265_dec.max_std_pps_count * sizeof(StdVideoH265PictureParameterSet);

      params->h265_dec.std_vps = vk_alloc(&device->alloc, vps_size, 8, VK_SYSTEM_ALLOCATION_SCOPE_OBJECT);
      params->h265_dec.std_sps = vk_alloc(&device->alloc, sps_size, 8, VK_SYSTEM_ALLOCATION_SCOPE_OBJECT);
      params->h265_dec.std_pps = vk_alloc(&device->alloc, pps_size, 8, VK_SYSTEM_ALLOCATION_SCOPE_OBJECT);
      if (!params->h265_dec.std_sps || !params->h265_dec.std_pps || !params->h265_dec.std_vps) {
         vk_free(&device->alloc, params->h265_dec.std_vps);
         vk_free(&device->alloc, params->h265_dec.std_sps);
         vk_free(&device->alloc, params->h265_dec.std_pps);
         return vk_error(device, VK_ERROR_OUT_OF_HOST_MEMORY);
      }

      init_add_h265_dec_session_parameters(params, h265_create->pParametersAddInfo, templ);
      break;
   }
   case VK_VIDEO_CODEC_OPERATION_ENCODE_H264_BIT_KHR: {
      const struct VkVideoEncodeH264SessionParametersCreateInfoKHR *h264_create =
         vk_find_struct_const(create_info->pNext, VIDEO_ENCODE_H264_SESSION_PARAMETERS_CREATE_INFO_KHR);

      params->h264_enc.max_std_sps_count = h264_create->maxStdSPSCount;
      params->h264_enc.max_std_pps_count = h264_create->maxStdPPSCount;

      uint32_t sps_size = params->h264_enc.max_std_sps_count * sizeof(StdVideoH264SequenceParameterSet);
      uint32_t pps_size = params->h264_enc.max_std_pps_count * sizeof(StdVideoH264PictureParameterSet);

      params->h264_enc.std_sps = vk_alloc(&device->alloc, sps_size, 8, VK_SYSTEM_ALLOCATION_SCOPE_OBJECT);
      params->h264_enc.std_pps = vk_alloc(&device->alloc, pps_size, 8, VK_SYSTEM_ALLOCATION_SCOPE_OBJECT);
      if (!params->h264_enc.std_sps || !params->h264_enc.std_pps) {
         vk_free(&device->alloc, params->h264_enc.std_sps);
         vk_free(&device->alloc, params->h264_enc.std_pps);
         return vk_error(device, VK_ERROR_OUT_OF_HOST_MEMORY);
      }

      init_add_h264_enc_session_parameters(params, h264_create->pParametersAddInfo, templ);
      break;
   }
   case VK_VIDEO_CODEC_OPERATION_ENCODE_H265_BIT_KHR: {
      const struct VkVideoEncodeH265SessionParametersCreateInfoKHR *h265_create =
         vk_find_struct_const(create_info->pNext, VIDEO_ENCODE_H265_SESSION_PARAMETERS_CREATE_INFO_KHR);

      params->h265_enc.max_std_vps_count = h265_create->maxStdVPSCount;
      params->h265_enc.max_std_sps_count = h265_create->maxStdSPSCount;
      params->h265_enc.max_std_pps_count = h265_create->maxStdPPSCount;

      uint32_t vps_size = params->h265_enc.max_std_vps_count * sizeof(StdVideoH265VideoParameterSet);
      uint32_t sps_size = params->h265_enc.max_std_sps_count * sizeof(StdVideoH265SequenceParameterSet);
      uint32_t pps_size = params->h265_enc.max_std_pps_count * sizeof(StdVideoH265PictureParameterSet);

      params->h265_enc.std_vps = vk_alloc(&device->alloc, vps_size, 8, VK_SYSTEM_ALLOCATION_SCOPE_OBJECT);
      params->h265_enc.std_sps = vk_alloc(&device->alloc, sps_size, 8, VK_SYSTEM_ALLOCATION_SCOPE_OBJECT);
      params->h265_enc.std_pps = vk_alloc(&device->alloc, pps_size, 8, VK_SYSTEM_ALLOCATION_SCOPE_OBJECT);
      if (!params->h265_enc.std_sps || !params->h265_enc.std_pps || !params->h265_enc.std_vps) {
         vk_free(&device->alloc, params->h265_enc.std_vps);
         vk_free(&device->alloc, params->h265_enc.std_sps);
         vk_free(&device->alloc, params->h265_enc.std_pps);
         return vk_error(device, VK_ERROR_OUT_OF_HOST_MEMORY);
      }

      init_add_h265_enc_session_parameters(params, h265_create->pParametersAddInfo, templ);
      break;
   }
   default:
      unreachable("Unsupported video codec operation");
      break;
   }
   return VK_SUCCESS;
}

void
vk_video_session_parameters_finish(struct vk_device *device,
                                   struct vk_video_session_parameters *params)
{
   switch (params->op) {
   case VK_VIDEO_CODEC_OPERATION_DECODE_H264_BIT_KHR:
      vk_free(&device->alloc, params->h264_dec.std_sps);
      vk_free(&device->alloc, params->h264_dec.std_pps);
      break;
   case VK_VIDEO_CODEC_OPERATION_DECODE_H265_BIT_KHR:
      vk_free(&device->alloc, params->h265_dec.std_vps);
      vk_free(&device->alloc, params->h265_dec.std_sps);
      vk_free(&device->alloc, params->h265_dec.std_pps);
      break;
   case VK_VIDEO_CODEC_OPERATION_ENCODE_H264_BIT_KHR:
      vk_free(&device->alloc, params->h264_enc.std_sps);
      vk_free(&device->alloc, params->h264_enc.std_pps);
      break;
   case VK_VIDEO_CODEC_OPERATION_ENCODE_H265_BIT_KHR:
      vk_free(&device->alloc, params->h265_enc.std_vps);
      vk_free(&device->alloc, params->h265_enc.std_sps);
      vk_free(&device->alloc, params->h265_enc.std_pps);
      break;
   default:
      break;
   }
   vk_object_base_finish(&params->base);
}

static VkResult
update_sps(struct vk_video_session_parameters *params,
           uint32_t count, const StdVideoH264SequenceParameterSet *adds)
{
    if (params->h264_dec.std_sps_count + count >= params->h264_dec.max_std_sps_count)
      return VK_ERROR_TOO_MANY_OBJECTS;

   typed_memcpy(&params->h264_dec.std_sps[params->h264_dec.std_sps_count], adds, count);
   params->h264_dec.std_sps_count += count;
   return VK_SUCCESS;
}

static VkResult
update_h264_dec_session_parameters(struct vk_video_session_parameters *params,
                                   const struct VkVideoDecodeH264SessionParametersAddInfoKHR *h264_add)
{
   VkResult result = VK_SUCCESS;

   result = update_h264_dec_std_sps(params, h264_add->stdSPSCount, h264_add->pStdSPSs);
   if (result != VK_SUCCESS)
      return result;

   result = update_h264_dec_std_pps(params, h264_add->stdPPSCount, h264_add->pStdPPSs);
   return result;
}

static VkResult
update_h264_enc_session_parameters(struct vk_video_session_parameters *params,
                                  const struct VkVideoEncodeH264SessionParametersAddInfoKHR *h264_add)
{
   VkResult result = VK_SUCCESS;
   result = update_h264_enc_std_sps(params, h264_add->stdSPSCount, h264_add->pStdSPSs);
   if (result != VK_SUCCESS)
      return result;

   result = update_h264_enc_std_pps(params, h264_add->stdPPSCount, h264_add->pStdPPSs);
   return result;
}

static VkResult
update_h265_enc_session_parameters(struct vk_video_session_parameters *params,
                                   const struct VkVideoEncodeH265SessionParametersAddInfoKHR *h265_add)
{
   VkResult result = VK_SUCCESS;

   result = update_h265_enc_std_vps(params, h265_add->stdVPSCount, h265_add->pStdVPSs);
   if (result != VK_SUCCESS)
      return result;

   result = update_h265_enc_std_sps(params, h265_add->stdSPSCount, h265_add->pStdSPSs);
   if (result != VK_SUCCESS)
      return result;

   result = update_h265_enc_std_pps(params, h265_add->stdPPSCount, h265_add->pStdPPSs);
   return result;
}

static VkResult
update_h265_session_parameters(struct vk_video_session_parameters *params,
                               const struct VkVideoDecodeH265SessionParametersAddInfoKHR *h265_add)
{
   VkResult result = VK_SUCCESS;
   result = update_h265_dec_std_vps(params, h265_add->stdVPSCount, h265_add->pStdVPSs);
   if (result != VK_SUCCESS)
      return result;

   result = update_h265_dec_std_sps(params, h265_add->stdSPSCount, h265_add->pStdSPSs);
   if (result != VK_SUCCESS)
      return result;

   result = update_h265_dec_std_pps(params, h265_add->stdPPSCount, h265_add->pStdPPSs);
   return result;
}

VkResult
vk_video_session_parameters_update(struct vk_video_session_parameters *params,
                                   const VkVideoSessionParametersUpdateInfoKHR *update)
{
   /* 39.6.5. Decoder Parameter Sets -
    * "The provided H.264 SPS/PPS parameters must be within the limits specified during decoder
    * creation for the decoder specified in VkVideoSessionParametersCreateInfoKHR."
    */

   /*
    * There is no need to deduplicate here.
    * videoSessionParameters must not already contain a StdVideoH264PictureParameterSet entry with
    * both seq_parameter_set_id and pic_parameter_set_id matching any of the elements of
    * VkVideoDecodeH264SessionParametersAddInfoKHR::pStdPPS
    */
   VkResult result = VK_SUCCESS;

   switch (params->op) {
   case VK_VIDEO_CODEC_OPERATION_DECODE_H264_BIT_KHR: {
      const struct VkVideoDecodeH264SessionParametersAddInfoKHR *h264_add =
         vk_find_struct_const(update->pNext, VIDEO_DECODE_H264_SESSION_PARAMETERS_ADD_INFO_KHR);
      return update_h264_dec_session_parameters(params, h264_add);
   }
   case VK_VIDEO_CODEC_OPERATION_DECODE_H265_BIT_KHR: {
      const struct VkVideoDecodeH265SessionParametersAddInfoKHR *h265_add =
         vk_find_struct_const(update->pNext, VIDEO_DECODE_H265_SESSION_PARAMETERS_ADD_INFO_KHR);

      return update_h265_session_parameters(params, h265_add);
   }
   case VK_VIDEO_CODEC_OPERATION_ENCODE_H264_BIT_KHR: {
      const struct VkVideoEncodeH264SessionParametersAddInfoKHR *h264_add =
        vk_find_struct_const(update->pNext, VIDEO_ENCODE_H264_SESSION_PARAMETERS_ADD_INFO_KHR);
      return update_h264_enc_session_parameters(params, h264_add);
   }
   case VK_VIDEO_CODEC_OPERATION_ENCODE_H265_BIT_KHR: {
      const struct VkVideoEncodeH265SessionParametersAddInfoKHR *h265_add =
        vk_find_struct_const(update->pNext, VIDEO_ENCODE_H265_SESSION_PARAMETERS_ADD_INFO_KHR);
      return update_h265_enc_session_parameters(params, h265_add);
   }
   default:
      unreachable("Unknown codec\n");
   }
   return result;
}

const uint8_t h264_scaling_list_default_4x4_intra[] =
{
   /* Table 7-3 - Default_4x4_Intra */
   6, 13, 13, 20, 20, 20, 28, 28, 28, 28, 32, 32, 32, 37, 37, 42
};

const uint8_t h264_scaling_list_default_4x4_inter[] =
{
   /* Table 7-3 - Default_4x4_Inter */
   10, 14, 14, 20, 20, 20, 24, 24, 24, 24, 27, 27, 27, 30, 30, 34
};

const uint8_t h264_scaling_list_default_8x8_intra[] =
{
   /* Table 7-4 - Default_8x8_Intra */
   6,  10, 10, 13, 11, 13, 16, 16, 16, 16, 18, 18, 18, 18, 18, 23,
   23, 23, 23, 23, 23, 25, 25, 25, 25, 25, 25, 25, 27, 27, 27, 27,
   27, 27, 27, 27, 29, 29, 29, 29, 29, 29, 29, 31, 31, 31, 31, 31,
   31, 33, 33, 33, 33, 33, 36, 36, 36, 36, 38, 38, 38, 40, 40, 42,
};

const uint8_t h264_scaling_list_default_8x8_inter[] =
{
   /* Table 7-4 - Default_8x8_Inter */
   9 , 13, 13, 15, 13, 15, 17, 17, 17, 17, 19, 19, 19, 19, 19, 21,
   21, 21, 21, 21, 21, 22, 22, 22, 22, 22, 22, 22, 24, 24, 24, 24,
   24, 24, 24, 24, 25, 25, 25, 25, 25, 25, 25, 27, 27, 27, 27, 27,
   27, 28, 28, 28, 28, 28, 30, 30, 30, 30, 32, 32, 32, 33, 33, 35,
};

void
vk_video_derive_h264_scaling_list(const StdVideoH264SequenceParameterSet *sps,
                                  const StdVideoH264PictureParameterSet *pps,
                                  StdVideoH264ScalingLists *list)
{
   StdVideoH264ScalingLists temp;

   /* derive SPS scaling list first, because PPS may depend on it in fall-back
    * rule B */
   if (sps->flags.seq_scaling_matrix_present_flag)
   {
      for (int i = 0; i < STD_VIDEO_H264_SCALING_LIST_4X4_NUM_LISTS; i++)
      {
         if (sps->pScalingLists->scaling_list_present_mask & (1 << i))
            memcpy(temp.ScalingList4x4[i],
                   pps->pScalingLists->ScalingList4x4[i],
                   STD_VIDEO_H264_SCALING_LIST_4X4_NUM_ELEMENTS);
         else /* fall-back rule A */
         {
            if (i == 0)
               memcpy(temp.ScalingList4x4[i],
                      h264_scaling_list_default_4x4_intra,
                      STD_VIDEO_H264_SCALING_LIST_4X4_NUM_ELEMENTS);
            else if (i == 3)
               memcpy(temp.ScalingList4x4[i],
                      h264_scaling_list_default_4x4_inter,
                      STD_VIDEO_H264_SCALING_LIST_4X4_NUM_ELEMENTS);
            else
               memcpy(temp.ScalingList4x4[i],
                      temp.ScalingList4x4[i - 1],
                      STD_VIDEO_H264_SCALING_LIST_4X4_NUM_ELEMENTS);
         }
      }

      for (int j = 0; j < STD_VIDEO_H264_SCALING_LIST_8X8_NUM_LISTS; j++)
      {
         int i = j + STD_VIDEO_H264_SCALING_LIST_4X4_NUM_LISTS;
         if (sps->pScalingLists->scaling_list_present_mask & (1 << i))
            memcpy(temp.ScalingList8x8[j], pps->pScalingLists->ScalingList8x8[j],
                   STD_VIDEO_H264_SCALING_LIST_8X8_NUM_ELEMENTS);
         else /* fall-back rule A */
         {
            if (i == 6)
               memcpy(temp.ScalingList8x8[j],
                      h264_scaling_list_default_8x8_intra,
                      STD_VIDEO_H264_SCALING_LIST_8X8_NUM_ELEMENTS);
            else if (i == 7)
               memcpy(temp.ScalingList8x8[j],
                      h264_scaling_list_default_8x8_inter,
                      STD_VIDEO_H264_SCALING_LIST_8X8_NUM_ELEMENTS);
            else
               memcpy(temp.ScalingList8x8[j], temp.ScalingList8x8[j - 2],
                      STD_VIDEO_H264_SCALING_LIST_8X8_NUM_ELEMENTS);
         }
      }
   }
   else
   {
      memset(temp.ScalingList4x4, 0x10,
             STD_VIDEO_H264_SCALING_LIST_4X4_NUM_LISTS *
             STD_VIDEO_H264_SCALING_LIST_4X4_NUM_ELEMENTS);
      memset(temp.ScalingList8x8, 0x10,
             STD_VIDEO_H264_SCALING_LIST_8X8_NUM_LISTS *
             STD_VIDEO_H264_SCALING_LIST_8X8_NUM_ELEMENTS);
   }

   if (pps->flags.pic_scaling_matrix_present_flag)
   {
      for (int i = 0; i < STD_VIDEO_H264_SCALING_LIST_4X4_NUM_LISTS; i++)
      {
         if (pps->pScalingLists->scaling_list_present_mask & (1 << i))
            memcpy(list->ScalingList4x4[i], pps->pScalingLists->ScalingList4x4[i],
                   STD_VIDEO_H264_SCALING_LIST_4X4_NUM_ELEMENTS);
         else if (sps->flags.seq_scaling_matrix_present_flag) /* fall-back rule B */
         {
            if (i == 0 || i == 3)
               memcpy(list->ScalingList4x4[i], temp.ScalingList4x4[i],
                      STD_VIDEO_H264_SCALING_LIST_4X4_NUM_ELEMENTS);
            else
               memcpy(list->ScalingList4x4[i], list->ScalingList4x4[i - 1],
                      STD_VIDEO_H264_SCALING_LIST_4X4_NUM_ELEMENTS);
         }
         else /* fall-back rule A */
         {
            if (i == 0)
               memcpy(list->ScalingList4x4[i],
                      h264_scaling_list_default_4x4_intra,
                      STD_VIDEO_H264_SCALING_LIST_4X4_NUM_ELEMENTS);
            else if (i == 3)
               memcpy(list->ScalingList4x4[i],
                      h264_scaling_list_default_4x4_inter,
                      STD_VIDEO_H264_SCALING_LIST_4X4_NUM_ELEMENTS);
            else
               memcpy(list->ScalingList4x4[i],
                      list->ScalingList4x4[i - 1],
                      STD_VIDEO_H264_SCALING_LIST_4X4_NUM_ELEMENTS);
         }
      }

      for (int j = 0; j < STD_VIDEO_H264_SCALING_LIST_8X8_NUM_LISTS; j++)
      {
         int i = j + STD_VIDEO_H264_SCALING_LIST_4X4_NUM_LISTS;
         if (pps->pScalingLists->scaling_list_present_mask & (1 << i))
            memcpy(list->ScalingList8x8[j], pps->pScalingLists->ScalingList8x8[j],
                   STD_VIDEO_H264_SCALING_LIST_8X8_NUM_ELEMENTS);
         else if (sps->flags.seq_scaling_matrix_present_flag) /* fall-back rule B */
         {
            if (i == 6 || i == 7)
               memcpy(list->ScalingList8x8[j], temp.ScalingList8x8[j],
                      STD_VIDEO_H264_SCALING_LIST_8X8_NUM_ELEMENTS);
            else
               memcpy(list->ScalingList8x8[j], list->ScalingList8x8[j - 2],
                      STD_VIDEO_H264_SCALING_LIST_8X8_NUM_ELEMENTS);
         }
         else /* fall-back rule A */
         {
            if (i == 6)
               memcpy(list->ScalingList8x8[j],
                      h264_scaling_list_default_8x8_intra,
                      STD_VIDEO_H264_SCALING_LIST_8X8_NUM_ELEMENTS);
            else if (i == 7)
               memcpy(list->ScalingList8x8[j],
                      h264_scaling_list_default_8x8_inter,
                      STD_VIDEO_H264_SCALING_LIST_8X8_NUM_ELEMENTS);
            else
               memcpy(list->ScalingList8x8[j], list->ScalingList8x8[j - 2],
                      STD_VIDEO_H264_SCALING_LIST_8X8_NUM_ELEMENTS);
         }
      }
   }
   else
   {
      memcpy(list->ScalingList4x4, temp.ScalingList4x4,
            STD_VIDEO_H264_SCALING_LIST_4X4_NUM_LISTS *
            STD_VIDEO_H264_SCALING_LIST_4X4_NUM_ELEMENTS);
      memcpy(list->ScalingList8x8, temp.ScalingList8x8,
            STD_VIDEO_H264_SCALING_LIST_8X8_NUM_LISTS *
            STD_VIDEO_H264_SCALING_LIST_8X8_NUM_ELEMENTS);
   }
}

const StdVideoH264SequenceParameterSet *
vk_video_find_h264_dec_std_sps(const struct vk_video_session_parameters *params,
                               uint32_t id)
{
   return find_h264_dec_std_sps(params, id);
}

const StdVideoH264PictureParameterSet *
vk_video_find_h264_dec_std_pps(const struct vk_video_session_parameters *params,
                               uint32_t id)
{
   return find_h264_dec_std_pps(params, id);
}

const StdVideoH265VideoParameterSet *
vk_video_find_h265_dec_std_vps(const struct vk_video_session_parameters *params,
                               uint32_t id)
{
   return find_h265_dec_std_vps(params, id);
}

const StdVideoH265SequenceParameterSet *
vk_video_find_h265_dec_std_sps(const struct vk_video_session_parameters *params,
                               uint32_t id)
{
   return find_h265_dec_std_sps(params, id);
}

const StdVideoH265PictureParameterSet *
vk_video_find_h265_dec_std_pps(const struct vk_video_session_parameters *params,
                               uint32_t id)
{
   return find_h265_dec_std_pps(params, id);
}

int
vk_video_h265_poc_by_slot(const struct VkVideoDecodeInfoKHR *frame_info, int slot)
{
   for (unsigned i = 0; i < frame_info->referenceSlotCount; i++) {
      const VkVideoDecodeH265DpbSlotInfoKHR *dpb_slot_info =
         vk_find_struct_const(frame_info->pReferenceSlots[i].pNext, VIDEO_DECODE_H265_DPB_SLOT_INFO_KHR);
      if (frame_info->pReferenceSlots[i].slotIndex == slot)
         return dpb_slot_info->pStdReferenceInfo->PicOrderCntVal;
   }

   assert(0);

   return 0;
}

void
vk_fill_video_h265_reference_info(const VkVideoDecodeInfoKHR *frame_info,
                                  const struct VkVideoDecodeH265PictureInfoKHR *pic,
                                  const struct vk_video_h265_slice_params *slice_params,
                                  struct vk_video_h265_reference ref_slots[][8])
{
   uint8_t list_cnt = slice_params->slice_type == STD_VIDEO_H265_SLICE_TYPE_B ? 2 : 1;
   uint8_t list_idx;
   int i, j;

   for (list_idx = 0; list_idx < list_cnt; list_idx++) {
      /* The order is
       *  L0: Short term current before set - Short term current after set - long term current
       *  L1: Short term current after set - short term current before set - long term current
       */
      const uint8_t *rps[3] = {
         list_idx ? pic->pStdPictureInfo->RefPicSetStCurrAfter : pic->pStdPictureInfo->RefPicSetStCurrBefore,
         list_idx ? pic->pStdPictureInfo->RefPicSetStCurrBefore : pic->pStdPictureInfo->RefPicSetStCurrAfter,
         pic->pStdPictureInfo->RefPicSetLtCurr
      };

      uint8_t ref_idx = 0;
      for (i = 0; i < 3; i++) {
         const uint8_t *cur_rps = rps[i];

         for (j = 0; (cur_rps[j] != 0xff) && ((j + ref_idx) < 8); j++) {
            ref_slots[list_idx][j + ref_idx].slot_index = cur_rps[j];
            ref_slots[list_idx][j + ref_idx].pic_order_cnt = vk_video_h265_poc_by_slot(frame_info, cur_rps[j]);
         }
         ref_idx += j;
      }

      /* TODO: should handle cases where rpl_modification_flag is true. */
      assert(!slice_params->rpl_modification_flag[0] && !slice_params->rpl_modification_flag[1]);
   }
}

static void
h265_pred_weight_table(struct vk_video_h265_slice_params *params,
                       struct vl_rbsp *rbsp,
                       const StdVideoH265SequenceParameterSet *sps,
                       StdVideoH265SliceType slice_type)
{
   unsigned chroma_array_type = sps->flags.separate_colour_plane_flag ? 0 : sps->chroma_format_idc;
   unsigned i, j;

   params->luma_log2_weight_denom = vl_rbsp_ue(rbsp);

   assert(params->luma_log2_weight_denom >= 0 && params->luma_log2_weight_denom < 8);

   if (chroma_array_type != 0) {
      params->chroma_log2_weight_denom = params->luma_log2_weight_denom + vl_rbsp_se(rbsp);
      assert(params->chroma_log2_weight_denom >= 0 && params->chroma_log2_weight_denom < 8);
   }

   for (i = 0; i < params->num_ref_idx_l0_active; ++i) {
      params->luma_weight_l0_flag[i] = vl_rbsp_u(rbsp, 1);
      if (!params->luma_weight_l0_flag[i]) {
         params->luma_weight_l0[i] = 1 << params->luma_log2_weight_denom;
         params->luma_offset_l0[i] = 0;
      }
   }

   for (i = 0; i < params->num_ref_idx_l0_active; ++i) {
      if (chroma_array_type == 0) {
         params->chroma_weight_l0_flag[i] = 0;
      } else {
         params->chroma_weight_l0_flag[i] = vl_rbsp_u(rbsp, 1);
      }
   }

   for (i = 0; i < params->num_ref_idx_l0_active; ++i) {
      if (params->luma_weight_l0_flag[i]) {
         params->delta_luma_weight_l0[i] = vl_rbsp_se(rbsp);
         params->luma_weight_l0[i] = (1 << params->luma_log2_weight_denom) + params->delta_luma_weight_l0[i];
         params->luma_offset_l0[i] = vl_rbsp_se(rbsp);
      }

      if (params->chroma_weight_l0_flag[i]) {
         for (j = 0; j < 2; j++) {
            params->delta_chroma_weight_l0[i][j] = vl_rbsp_se(rbsp);
            params->delta_chroma_offset_l0[i][j] = vl_rbsp_se(rbsp);

            params->chroma_weight_l0[i][j] =
               (1 << params->chroma_log2_weight_denom) + params->delta_chroma_weight_l0[i][j];
            params->chroma_offset_l0[i][j] = CLAMP(params->delta_chroma_offset_l0[i][j] -
               ((128 * params->chroma_weight_l0[i][j]) >> params->chroma_log2_weight_denom) + 128, -128, 127);
         }
      } else {
         for (j = 0; j < 2; j++) {
            params->chroma_weight_l0[i][j] = 1 << params->chroma_log2_weight_denom;
            params->chroma_offset_l0[i][j] = 0;
         }
      }
   }

   if (slice_type == STD_VIDEO_H265_SLICE_TYPE_B) {
      for (i = 0; i < params->num_ref_idx_l1_active; ++i) {
         params->luma_weight_l1_flag[i] = vl_rbsp_u(rbsp, 1);
         if (!params->luma_weight_l1_flag[i]) {
            params->luma_weight_l1[i] = 1 << params->luma_log2_weight_denom;
            params->luma_offset_l1[i] = 0;
         }
      }

      for (i = 0; i < params->num_ref_idx_l1_active; ++i) {
         if (chroma_array_type == 0) {
            params->chroma_weight_l1_flag[i] = 0;
         } else {
            params->chroma_weight_l1_flag[i] = vl_rbsp_u(rbsp, 1);
         }
      }

      for (i = 0; i < params->num_ref_idx_l1_active; ++i) {
         if (params->luma_weight_l1_flag[i]) {
            params->delta_luma_weight_l1[i] = vl_rbsp_se(rbsp);
            params->luma_weight_l1[i] =
               (1 << params->luma_log2_weight_denom) + params->delta_luma_weight_l1[i];
            params->luma_offset_l1[i] = vl_rbsp_se(rbsp);
         }

         if (params->chroma_weight_l1_flag[i]) {
            for (j = 0; j < 2; j++) {
               params->delta_chroma_weight_l1[i][j] = vl_rbsp_se(rbsp);
               params->delta_chroma_offset_l1[i][j] = vl_rbsp_se(rbsp);

               params->chroma_weight_l1[i][j] =
                  (1 << params->chroma_log2_weight_denom) + params->delta_chroma_weight_l1[i][j];
               params->chroma_offset_l1[i][j] = CLAMP(params->delta_chroma_offset_l1[i][j] -
                  ((128 * params->chroma_weight_l1[i][j]) >> params->chroma_log2_weight_denom) + 128, -128, 127);
            }
         } else {
            for (j = 0; j < 2; j++) {
               params->chroma_weight_l1[i][j] = 1 << params->chroma_log2_weight_denom;
               params->chroma_offset_l1[i][j] = 0;
            }
         }
      }
   }
}

void
vk_video_parse_h265_slice_header(const struct VkVideoDecodeInfoKHR *frame_info,
                                 const VkVideoDecodeH265PictureInfoKHR *pic_info,
                                 const StdVideoH265SequenceParameterSet *sps,
                                 const StdVideoH265PictureParameterSet *pps,
                                 void *slice_data,
                                 uint32_t slice_size,
                                 struct vk_video_h265_slice_params *params)
{
   struct vl_vlc vlc;
   const void *slice_headers[1] = { slice_data };
   vl_vlc_init(&vlc, 1, slice_headers, &slice_size);

   assert(vl_vlc_peekbits(&vlc, 24) == 0x000001);

   vl_vlc_eatbits(&vlc, 24);

   /* forbidden_zero_bit */
   vl_vlc_eatbits(&vlc, 1);

   if (vl_vlc_valid_bits(&vlc) < 15)
      vl_vlc_fillbits(&vlc);

   vl_vlc_get_uimsbf(&vlc, 6); /* nal_unit_type */
   vl_vlc_get_uimsbf(&vlc, 6); /* nuh_layer_id */
   vl_vlc_get_uimsbf(&vlc, 3); /* nuh_temporal_id_plus1 */

   struct vl_rbsp rbsp;
   vl_rbsp_init(&rbsp, &vlc, 128, /* emulation_bytes */ true);

   memset(params, 0, sizeof(*params));

   params->slice_size = slice_size;
   params->first_slice_segment_in_pic_flag = vl_rbsp_u(&rbsp, 1);

   /* no_output_of_prior_pics_flag */
   if (pic_info->pStdPictureInfo->flags.IrapPicFlag)
      vl_rbsp_u(&rbsp, 1);

   /* pps id */
   vl_rbsp_ue(&rbsp);

   if (!params->first_slice_segment_in_pic_flag) {
      int size, num;
      int bits_slice_segment_address = 0;

      if (pps->flags.dependent_slice_segments_enabled_flag)
         params->dependent_slice_segment = vl_rbsp_u(&rbsp, 1);

      size = 1 << (sps->log2_min_luma_coding_block_size_minus3 + 3 +
                   sps->log2_diff_max_min_luma_coding_block_size);

      num = ((sps->pic_width_in_luma_samples + size - 1) / size) *
            ((sps->pic_height_in_luma_samples + size - 1) / size);

      while (num > (1 << bits_slice_segment_address))
         bits_slice_segment_address++;

      /* slice_segment_address */
      params->slice_segment_address = vl_rbsp_u(&rbsp, bits_slice_segment_address);
   }

   if (params->dependent_slice_segment)
      return;

   for (unsigned i = 0; i < pps->num_extra_slice_header_bits; ++i)
      /* slice_reserved_flag */
      vl_rbsp_u(&rbsp, 1);

   /* slice_type */
   params->slice_type = vl_rbsp_ue(&rbsp);

   if (pps->flags.output_flag_present_flag)
      /* pic output flag */
      vl_rbsp_u(&rbsp, 1);

   if (sps->flags.separate_colour_plane_flag)
      /* colour_plane_id */
      vl_rbsp_u(&rbsp, 2);

   if (!pic_info->pStdPictureInfo->flags.IdrPicFlag) {
      /* slice_pic_order_cnt_lsb */
      params->pic_order_cnt_lsb =
         vl_rbsp_u(&rbsp, sps->log2_max_pic_order_cnt_lsb_minus4 + 4);

      /* short_term_ref_pic_set_sps_flag */
      if (!vl_rbsp_u(&rbsp, 1)) {
         uint8_t rps_predict = 0;

         if (sps->num_short_term_ref_pic_sets)
            rps_predict = vl_rbsp_u(&rbsp, 1);

         if (rps_predict) {
            /* delta_idx */
            vl_rbsp_ue(&rbsp);
            /* delta_rps_sign */
            vl_rbsp_u(&rbsp, 1);
            /* abs_delta_rps */
            vl_rbsp_ue(&rbsp);

            for (int i = 0 ; i <= pic_info->pStdPictureInfo->NumDeltaPocsOfRefRpsIdx; i++) {
               uint8_t used = vl_rbsp_u(&rbsp, 1);
               if (!used)
                  vl_rbsp_u(&rbsp, 1);
            }
         } else {
            /* num_negative_pics */
            unsigned num_neg_pics = vl_rbsp_ue(&rbsp);
            /* num_positive_pics */
            unsigned num_pos_pics = vl_rbsp_ue(&rbsp);

            for(unsigned i = 0 ; i < num_neg_pics; ++i) {
               /* delta_poc_s0_minus1 */
               vl_rbsp_ue(&rbsp);
               /* used_by_curr_pic_s0_flag */
               vl_rbsp_u(&rbsp, 1);
            }

            for(unsigned i = 0; i < num_pos_pics; ++i) {
               /* delta_poc_s1_minus1 */
               vl_rbsp_ue(&rbsp);
               /* used_by_curr_pic_s0_flag */
               vl_rbsp_u(&rbsp, 1);
            }
         }

      } else {
         unsigned num_st_rps = sps->num_short_term_ref_pic_sets;

         int numbits = util_logbase2_ceil(num_st_rps);
         if (numbits > 0)
            /* short_term_ref_pic_set_idx */
            vl_rbsp_u(&rbsp, numbits);
      }

      if (sps->flags.long_term_ref_pics_present_flag) {
         unsigned num_lt_sps = 0;

         if (sps->num_long_term_ref_pics_sps > 0)
            num_lt_sps = vl_rbsp_ue(&rbsp);

         unsigned num_lt_pics = vl_rbsp_ue(&rbsp);
         unsigned num_refs = num_lt_pics + num_lt_sps;

         for (unsigned i = 0; i < num_refs; i++) {
            if (i < num_lt_sps) {
               if (sps->num_long_term_ref_pics_sps > 1)
                  /* lt_idx_sps */
                  vl_rbsp_u(&rbsp,
                        util_logbase2_ceil(sps->num_long_term_ref_pics_sps));
            } else {
               /* poc_lsb_lt */
               vl_rbsp_u(&rbsp, sps->log2_max_pic_order_cnt_lsb_minus4 + 4);
               /* used_by_curr_pic_lt_flag */
               vl_rbsp_u(&rbsp, 1);
            }

            /* poc_msb_present */
            if (vl_rbsp_u(&rbsp, 1)) {
               /* delta_poc_msb_cycle_lt */
               vl_rbsp_ue(&rbsp);
            }
         }
      }

      if (sps->flags.sps_temporal_mvp_enabled_flag)
         params->temporal_mvp_enable = vl_rbsp_u(&rbsp, 1);
   }

   if (sps->flags.sample_adaptive_offset_enabled_flag) {
      params->sao_luma_flag = vl_rbsp_u(&rbsp, 1);
      if (sps->chroma_format_idc)
         params->sao_chroma_flag = vl_rbsp_u(&rbsp, 1);
   }

   params->max_num_merge_cand = 5;

   if (params->slice_type != STD_VIDEO_H265_SLICE_TYPE_I) {

      params->num_ref_idx_l0_active = pps->num_ref_idx_l0_default_active_minus1 + 1;

      if (params->slice_type == STD_VIDEO_H265_SLICE_TYPE_B)
         params->num_ref_idx_l1_active = pps->num_ref_idx_l1_default_active_minus1 + 1;
      else
         params->num_ref_idx_l1_active = 0;

      /* num_ref_idx_active_override_flag */
      if (vl_rbsp_u(&rbsp, 1)) {
         params->num_ref_idx_l0_active = vl_rbsp_ue(&rbsp) + 1;
         if (params->slice_type == STD_VIDEO_H265_SLICE_TYPE_B)
            params->num_ref_idx_l1_active = vl_rbsp_ue(&rbsp) + 1;
      }

      if (pps->flags.lists_modification_present_flag) {
         params->rpl_modification_flag[0] = vl_rbsp_u(&rbsp, 1);
         if (params->rpl_modification_flag[0]) {
            for (int i = 0; i < params->num_ref_idx_l0_active; i++) {
               /* list_entry_l0 */
               vl_rbsp_u(&rbsp,
                     util_logbase2_ceil(params->num_ref_idx_l0_active + params->num_ref_idx_l1_active));
            }
         }

         if (params->slice_type == STD_VIDEO_H265_SLICE_TYPE_B) {
            params->rpl_modification_flag[1] = vl_rbsp_u(&rbsp, 1);
            if (params->rpl_modification_flag[1]) {
               for (int i = 0; i < params->num_ref_idx_l1_active; i++) {
                  /* list_entry_l1 */
                  vl_rbsp_u(&rbsp,
                        util_logbase2_ceil(params->num_ref_idx_l0_active + params->num_ref_idx_l1_active));
               }
            }
         }
      }

      if (params->slice_type == STD_VIDEO_H265_SLICE_TYPE_B)
         params->mvd_l1_zero_flag = vl_rbsp_u(&rbsp, 1);

      if (pps->flags.cabac_init_present_flag)
         /* cabac_init_flag */
         params->cabac_init_idc = vl_rbsp_u(&rbsp, 1);

      if (params->temporal_mvp_enable) {
         if (params->slice_type == STD_VIDEO_H265_SLICE_TYPE_B)
            params->collocated_list = !vl_rbsp_u(&rbsp, 1);

         if (params->collocated_list == 0) {
            if (params->num_ref_idx_l0_active > 1)
               params->collocated_ref_idx = vl_rbsp_ue(&rbsp);
         }  else if (params->collocated_list == 1) {
            if (params->num_ref_idx_l1_active > 1)
               params->collocated_ref_idx = vl_rbsp_ue(&rbsp);
         }
      }

      if ((pps->flags.weighted_pred_flag && params->slice_type == STD_VIDEO_H265_SLICE_TYPE_P) ||
            (pps->flags.weighted_bipred_flag && params->slice_type == STD_VIDEO_H265_SLICE_TYPE_B)) {
         h265_pred_weight_table(params, &rbsp, sps, params->slice_type);
      }

      params->max_num_merge_cand -= vl_rbsp_ue(&rbsp);
   }

   params->slice_qp_delta = vl_rbsp_se(&rbsp);

   if (pps->flags.pps_slice_chroma_qp_offsets_present_flag) {
      params->slice_cb_qp_offset = vl_rbsp_se(&rbsp);
      params->slice_cr_qp_offset = vl_rbsp_se(&rbsp);
   }

   if (pps->flags.chroma_qp_offset_list_enabled_flag)
      /* cu_chroma_qp_offset_enabled_flag */
      vl_rbsp_u(&rbsp, 1);

   if (pps->flags.deblocking_filter_control_present_flag) {
      if (pps->flags.deblocking_filter_override_enabled_flag) {
         /* deblocking_filter_override_flag */
         if (vl_rbsp_u(&rbsp, 1)) {
            params->disable_deblocking_filter_idc = vl_rbsp_u(&rbsp, 1);

            if (!params->disable_deblocking_filter_idc) {
               params->beta_offset_div2 = vl_rbsp_se(&rbsp);
               params->tc_offset_div2 = vl_rbsp_se(&rbsp);
            }
         } else {
            params->disable_deblocking_filter_idc =
               pps->flags.pps_deblocking_filter_disabled_flag;
         }
      }
   }

   if (pps->flags.pps_loop_filter_across_slices_enabled_flag &&
         (params->sao_luma_flag || params->sao_chroma_flag ||
          !params->disable_deblocking_filter_idc))
      params->loop_filter_across_slices_enable = vl_rbsp_u(&rbsp, 1);

   if (pps->flags.tiles_enabled_flag || pps->flags.entropy_coding_sync_enabled_flag) {
      unsigned num_entry_points_offsets = vl_rbsp_ue(&rbsp);

      if (num_entry_points_offsets > 0) {
         unsigned offset_len = vl_rbsp_ue(&rbsp) + 1;
         for (unsigned i = 0; i < num_entry_points_offsets; i++) {
            /* entry_point_offset_minus1 */
            vl_rbsp_u(&rbsp, offset_len);
         }
      }
   }

   if (pps->flags.pps_extension_present_flag) {
      unsigned length = vl_rbsp_ue(&rbsp);
      for (unsigned i = 0; i < length; i++)
         /* slice_reserved_undetermined_flag */
         vl_rbsp_u(&rbsp, 1);
   }

   unsigned header_bits =
      (slice_size * 8 - 24 /* start code */) - vl_vlc_bits_left(&rbsp.nal) - rbsp.removed;
   params->slice_data_bytes_offset = (header_bits + 8) / 8;
}

void
vk_video_get_profile_alignments(const VkVideoProfileListInfoKHR *profile_list,
                                uint32_t *width_align_out, uint32_t *height_align_out)
{
   uint32_t width_align = 1, height_align = 1;
   for (unsigned i = 0; i < profile_list->profileCount; i++) {
      if (profile_list->pProfiles[i].videoCodecOperation == VK_VIDEO_CODEC_OPERATION_DECODE_H264_BIT_KHR ||
          profile_list->pProfiles[i].videoCodecOperation == VK_VIDEO_CODEC_OPERATION_ENCODE_H264_BIT_KHR
         ) {
         width_align = MAX2(width_align, VK_VIDEO_H264_MACROBLOCK_WIDTH);
         height_align = MAX2(height_align, VK_VIDEO_H264_MACROBLOCK_HEIGHT);
      }
      if (profile_list->pProfiles[i].videoCodecOperation == VK_VIDEO_CODEC_OPERATION_DECODE_H265_BIT_KHR ||
          profile_list->pProfiles[i].videoCodecOperation == VK_VIDEO_CODEC_OPERATION_ENCODE_H265_BIT_KHR
         ) {
         width_align = MAX2(width_align, VK_VIDEO_H265_CTU_MAX_WIDTH);
         height_align = MAX2(height_align, VK_VIDEO_H265_CTU_MAX_HEIGHT);
      }
   }
   *width_align_out = width_align;
   *height_align_out = height_align;
}

static const uint8_t vk_video_h264_levels[] = {10, 11, 12, 13, 20, 21, 22, 30, 31, 32, 40, 41, 42, 50, 51, 52, 60, 61, 62};
uint8_t
vk_video_get_h264_level(StdVideoH264LevelIdc level)
{
   assert(level <= STD_VIDEO_H264_LEVEL_IDC_6_2);
   return vk_video_h264_levels[level];
}

const StdVideoH264SequenceParameterSet *
vk_video_find_h264_enc_std_sps(const struct vk_video_session_parameters *params,
                               uint32_t id)
{
   return find_h264_enc_std_sps(params, id);
}

const StdVideoH264PictureParameterSet *
vk_video_find_h264_enc_std_pps(const struct vk_video_session_parameters *params,
                               uint32_t id)
{
   return find_h264_enc_std_pps(params, id);
}

const StdVideoH265VideoParameterSet *
vk_video_find_h265_enc_std_vps(const struct vk_video_session_parameters *params,
                               uint32_t id)
{
   return find_h265_enc_std_vps(params, id);
}

const StdVideoH265SequenceParameterSet *
vk_video_find_h265_enc_std_sps(const struct vk_video_session_parameters *params,
                               uint32_t id)
{
   return find_h265_enc_std_sps(params, id);
}

const StdVideoH265PictureParameterSet *
vk_video_find_h265_enc_std_pps(const struct vk_video_session_parameters *params,
                               uint32_t id)
{
   return find_h265_enc_std_pps(params, id);
}

enum H264NALUType
{
   H264_NAL_UNSPECIFIED           = 0,
   H264_NAL_SLICE                 = 1,
   H264_NAL_SLICEDATA_A           = 2,
   H264_NAL_SLICEDATA_B           = 3,
   H264_NAL_SLICEDATA_C           = 4,
   H264_NAL_IDR                   = 5,
   H264_NAL_SEI                   = 6,
   H264_NAL_SPS                   = 7,
   H264_NAL_PPS                   = 8,
   H264_NAL_ACCESS_UNIT_DEMILITER = 9,
   H264_NAL_END_OF_SEQUENCE       = 10,
   H264_NAL_END_OF_STREAM         = 11,
   H264_NAL_FILLER_DATA           = 12,
   H264_NAL_SPS_EXTENSION         = 13,
   H264_NAL_PREFIX                = 14,
   /* 15...18 RESERVED */
   H264_NAL_AUXILIARY_SLICE = 19,
   /* 20...23 RESERVED */
   /* 24...31 UNSPECIFIED */
};

enum HEVCNALUnitType {
   HEVC_NAL_TRAIL_N        = 0,
   HEVC_NAL_TRAIL_R        = 1,
   HEVC_NAL_TSA_N          = 2,
   HEVC_NAL_TSA_R          = 3,
   HEVC_NAL_STSA_N         = 4,
   HEVC_NAL_STSA_R         = 5,
   HEVC_NAL_RADL_N         = 6,
   HEVC_NAL_RADL_R         = 7,
   HEVC_NAL_RASL_N         = 8,
   HEVC_NAL_RASL_R         = 9,
   HEVC_NAL_VCL_N10        = 10,
   HEVC_NAL_VCL_R11        = 11,
   HEVC_NAL_VCL_N12        = 12,
   HEVC_NAL_VCL_R13        = 13,
   HEVC_NAL_VCL_N14        = 14,
   HEVC_NAL_VCL_R15        = 15,
   HEVC_NAL_BLA_W_LP       = 16,
   HEVC_NAL_BLA_W_RADL     = 17,
   HEVC_NAL_BLA_N_LP       = 18,
   HEVC_NAL_IDR_W_RADL     = 19,
   HEVC_NAL_IDR_N_LP       = 20,
   HEVC_NAL_CRA_NUT        = 21,
   HEVC_NAL_VPS_NUT        = 32,
   HEVC_NAL_SPS_NUT        = 33,
   HEVC_NAL_PPS_NUT        = 34,
};

unsigned
vk_video_get_h265_nal_unit(StdVideoH265PictureType pic_type, bool irap_pic_flag)
{
   switch (pic_type) {
   case STD_VIDEO_H265_PICTURE_TYPE_IDR:
      return HEVC_NAL_IDR_W_RADL;
   case STD_VIDEO_H265_PICTURE_TYPE_I:
      return HEVC_NAL_CRA_NUT;
   case STD_VIDEO_H265_PICTURE_TYPE_P:
      return HEVC_NAL_TRAIL_R;
   case STD_VIDEO_H265_PICTURE_TYPE_B:
      if (irap_pic_flag)
         return HEVC_NAL_RASL_R;
      else
         return HEVC_NAL_TRAIL_R;
      break;
   default:
      assert(0);
      break;
   }
   return 0;
}

static const uint8_t vk_video_h265_levels[] = {10, 20, 21, 30, 31, 40, 41, 50, 51, 52, 60, 61, 62};

static uint8_t
vk_video_get_h265_level(StdVideoH265LevelIdc level)
{
   assert(level <= STD_VIDEO_H265_LEVEL_IDC_6_2);
   return vk_video_h265_levels[level];
}

static void
emit_nalu_header(struct vl_bitstream_encoder *enc,
                 int nal_ref, int nal_unit)
{
   enc->prevent_start_code = false;

   vl_bitstream_put_bits(enc, 24, 0);
   vl_bitstream_put_bits(enc, 8, 1);
   vl_bitstream_put_bits(enc, 1, 0);
   vl_bitstream_put_bits(enc, 2, nal_ref); /* SPS NAL REF */
   vl_bitstream_put_bits(enc, 5, nal_unit); /* SPS NAL UNIT */
   vl_bitstream_flush(enc);

   enc->prevent_start_code = true;
}

static void
encode_hrd_params(struct vl_bitstream_encoder *enc,
                  const StdVideoH264HrdParameters *hrd)
{
   vl_bitstream_exp_golomb_ue(enc, hrd->cpb_cnt_minus1);
   vl_bitstream_put_bits(enc, 4, hrd->bit_rate_scale);
   vl_bitstream_put_bits(enc, 4, hrd->cpb_size_scale);
   for (int sched_sel_idx = 0; sched_sel_idx <= hrd->cpb_cnt_minus1; sched_sel_idx++) {
      vl_bitstream_exp_golomb_ue(enc, hrd->bit_rate_value_minus1[sched_sel_idx]);
      vl_bitstream_exp_golomb_ue(enc, hrd->cpb_size_value_minus1[sched_sel_idx]);
      vl_bitstream_put_bits(enc, 1, hrd->cbr_flag[sched_sel_idx]);
   }
   vl_bitstream_put_bits(enc, 5, hrd->initial_cpb_removal_delay_length_minus1);
   vl_bitstream_put_bits(enc, 5, hrd->cpb_removal_delay_length_minus1);
   vl_bitstream_put_bits(enc, 5, hrd->dpb_output_delay_length_minus1);
   vl_bitstream_put_bits(enc, 5, hrd->time_offset_length);
}

void
vk_video_encode_h264_sps(StdVideoH264SequenceParameterSet *sps,
                         size_t size_limit,
                         size_t *data_size_ptr,
                         void *data_ptr)
{
   struct vl_bitstream_encoder enc;
   uint32_t data_size = *data_size_ptr;

   vl_bitstream_encoder_clear(&enc, data_ptr, data_size, size_limit);

   emit_nalu_header(&enc, 3, H264_NAL_SPS);

   vl_bitstream_put_bits(&enc, 8, sps->profile_idc);
   vl_bitstream_put_bits(&enc, 1, sps->flags.constraint_set0_flag);
   vl_bitstream_put_bits(&enc, 1, sps->flags.constraint_set1_flag);
   vl_bitstream_put_bits(&enc, 1, sps->flags.constraint_set2_flag);
   vl_bitstream_put_bits(&enc, 1, sps->flags.constraint_set3_flag);
   vl_bitstream_put_bits(&enc, 1, sps->flags.constraint_set4_flag);
   vl_bitstream_put_bits(&enc, 1, sps->flags.constraint_set5_flag);
   vl_bitstream_put_bits(&enc, 2, 0);
   vl_bitstream_put_bits(&enc, 8, vk_video_get_h264_level(sps->level_idc));
   vl_bitstream_exp_golomb_ue(&enc, sps->seq_parameter_set_id);

   if (sps->profile_idc == STD_VIDEO_H264_PROFILE_IDC_HIGH /* high10 as well */) {
      vl_bitstream_exp_golomb_ue(&enc, sps->chroma_format_idc);
      vl_bitstream_exp_golomb_ue(&enc, sps->bit_depth_luma_minus8);
      vl_bitstream_exp_golomb_ue(&enc, sps->bit_depth_chroma_minus8);
      vl_bitstream_put_bits(&enc, 1, sps->flags.qpprime_y_zero_transform_bypass_flag);
      vl_bitstream_put_bits(&enc, 1, sps->flags.seq_scaling_matrix_present_flag);
   }

   vl_bitstream_exp_golomb_ue(&enc, sps->log2_max_frame_num_minus4);

   vl_bitstream_exp_golomb_ue(&enc, sps->pic_order_cnt_type);
   if (sps->pic_order_cnt_type == 0)
      vl_bitstream_exp_golomb_ue(&enc, sps->log2_max_pic_order_cnt_lsb_minus4);

   vl_bitstream_exp_golomb_ue(&enc, sps->max_num_ref_frames);
   vl_bitstream_put_bits(&enc, 1, sps->flags.gaps_in_frame_num_value_allowed_flag);
   vl_bitstream_exp_golomb_ue(&enc, sps->pic_width_in_mbs_minus1);
   vl_bitstream_exp_golomb_ue(&enc, sps->pic_height_in_map_units_minus1);

   vl_bitstream_put_bits(&enc, 1, sps->flags.frame_mbs_only_flag);
   vl_bitstream_put_bits(&enc, 1, sps->flags.direct_8x8_inference_flag);

   vl_bitstream_put_bits(&enc, 1, sps->flags.frame_cropping_flag);
   if (sps->flags.frame_cropping_flag) {
      vl_bitstream_exp_golomb_ue(&enc, sps->frame_crop_left_offset);
      vl_bitstream_exp_golomb_ue(&enc, sps->frame_crop_right_offset);
      vl_bitstream_exp_golomb_ue(&enc, sps->frame_crop_top_offset);
      vl_bitstream_exp_golomb_ue(&enc, sps->frame_crop_bottom_offset);
   }

   vl_bitstream_put_bits(&enc, 1, sps->flags.vui_parameters_present_flag); /* vui parameters present flag */
   if (sps->flags.vui_parameters_present_flag) {
      const StdVideoH264SequenceParameterSetVui *vui = sps->pSequenceParameterSetVui;
      vl_bitstream_put_bits(&enc, 1, vui->flags.aspect_ratio_info_present_flag);

      if (vui->flags.aspect_ratio_info_present_flag) {
         vl_bitstream_put_bits(&enc, 8, vui->aspect_ratio_idc);
         if (vui->aspect_ratio_idc == STD_VIDEO_H264_ASPECT_RATIO_IDC_EXTENDED_SAR) {
            vl_bitstream_put_bits(&enc, 16, vui->sar_width);
            vl_bitstream_put_bits(&enc, 16, vui->sar_height);
         }
      }

      vl_bitstream_put_bits(&enc, 1, vui->flags.overscan_info_present_flag);
      if (vui->flags.overscan_info_present_flag)
         vl_bitstream_put_bits(&enc, 1, vui->flags.overscan_appropriate_flag);
      vl_bitstream_put_bits(&enc, 1, vui->flags.video_signal_type_present_flag);
      if (vui->flags.video_signal_type_present_flag) {
         vl_bitstream_put_bits(&enc, 3, vui->video_format);
         vl_bitstream_put_bits(&enc, 1, vui->flags.video_full_range_flag);
         vl_bitstream_put_bits(&enc, 1, vui->flags.color_description_present_flag);
         if (vui->flags.color_description_present_flag) {
            vl_bitstream_put_bits(&enc, 8, vui->colour_primaries);
            vl_bitstream_put_bits(&enc, 8, vui->transfer_characteristics);
            vl_bitstream_put_bits(&enc, 8, vui->matrix_coefficients);
         }
      }

      vl_bitstream_put_bits(&enc, 1, vui->flags.chroma_loc_info_present_flag);
      if (vui->flags.chroma_loc_info_present_flag) {
         vl_bitstream_exp_golomb_ue(&enc, vui->chroma_sample_loc_type_top_field);
         vl_bitstream_exp_golomb_ue(&enc, vui->chroma_sample_loc_type_bottom_field);
      }
      vl_bitstream_put_bits(&enc, 1, vui->flags.timing_info_present_flag);
      if (vui->flags.timing_info_present_flag) {
         vl_bitstream_put_bits(&enc, 32, vui->num_units_in_tick);
         vl_bitstream_put_bits(&enc, 32, vui->time_scale);
         vl_bitstream_put_bits(&enc, 32, vui->flags.fixed_frame_rate_flag);
      }
      vl_bitstream_put_bits(&enc, 1, vui->flags.nal_hrd_parameters_present_flag);
      if (vui->flags.nal_hrd_parameters_present_flag)
         encode_hrd_params(&enc, vui->pHrdParameters);
      vl_bitstream_put_bits(&enc, 1, vui->flags.vcl_hrd_parameters_present_flag);
      if (vui->flags.vcl_hrd_parameters_present_flag)
         encode_hrd_params(&enc, vui->pHrdParameters);
      if (vui->flags.nal_hrd_parameters_present_flag || vui->flags.vcl_hrd_parameters_present_flag)
         vl_bitstream_put_bits(&enc, 1, 0);
      vl_bitstream_put_bits(&enc, 1, 0);
      vl_bitstream_put_bits(&enc, 1, vui->flags.bitstream_restriction_flag);
      if (vui->flags.bitstream_restriction_flag) {
         vl_bitstream_put_bits(&enc, 1, 0);
         vl_bitstream_exp_golomb_ue(&enc, 0);
         vl_bitstream_exp_golomb_ue(&enc, 0);
         vl_bitstream_exp_golomb_ue(&enc, 0);
         vl_bitstream_exp_golomb_ue(&enc, 0);
         vl_bitstream_exp_golomb_ue(&enc, vui->max_num_reorder_frames);
         vl_bitstream_exp_golomb_ue(&enc, vui->max_dec_frame_buffering);
      }
   }

   vl_bitstream_rbsp_trailing(&enc);

   vl_bitstream_flush(&enc);
   *data_size_ptr += vl_bitstream_get_byte_count(&enc);
   vl_bitstream_encoder_free(&enc);
}

void
vk_video_encode_h264_pps(StdVideoH264PictureParameterSet *pps,
                         bool high_profile,
                         size_t size_limit,
                         size_t *data_size_ptr,
                         void *data_ptr)
{
   struct vl_bitstream_encoder enc;
   uint32_t data_size = *data_size_ptr;

   vl_bitstream_encoder_clear(&enc, data_ptr, data_size, size_limit);

   emit_nalu_header(&enc, 3, H264_NAL_PPS);

   vl_bitstream_exp_golomb_ue(&enc, pps->pic_parameter_set_id);
   vl_bitstream_exp_golomb_ue(&enc, pps->seq_parameter_set_id);
   vl_bitstream_put_bits(&enc, 1, pps->flags.entropy_coding_mode_flag);
   vl_bitstream_put_bits(&enc, 1, pps->flags.bottom_field_pic_order_in_frame_present_flag);
   vl_bitstream_exp_golomb_ue(&enc, 0); /* num_slice_groups_minus1 */

   vl_bitstream_exp_golomb_ue(&enc, pps->num_ref_idx_l0_default_active_minus1);
   vl_bitstream_exp_golomb_ue(&enc, pps->num_ref_idx_l1_default_active_minus1);
   vl_bitstream_put_bits(&enc, 1, pps->flags.weighted_pred_flag);
   vl_bitstream_put_bits(&enc, 2, pps->weighted_bipred_idc);
   vl_bitstream_exp_golomb_se(&enc, pps->pic_init_qp_minus26);
   vl_bitstream_exp_golomb_se(&enc, pps->pic_init_qs_minus26);
   vl_bitstream_exp_golomb_se(&enc, pps->chroma_qp_index_offset);
   vl_bitstream_put_bits(&enc, 1, pps->flags.deblocking_filter_control_present_flag);
   vl_bitstream_put_bits(&enc, 1, pps->flags.constrained_intra_pred_flag);
   vl_bitstream_put_bits(&enc, 1, pps->flags.redundant_pic_cnt_present_flag);

   /* high profile */
   if (high_profile) {
      vl_bitstream_put_bits(&enc, 1, pps->flags.transform_8x8_mode_flag);
      vl_bitstream_put_bits(&enc, 1, pps->flags.pic_scaling_matrix_present_flag);
      vl_bitstream_exp_golomb_se(&enc, pps->second_chroma_qp_index_offset);
   }
   vl_bitstream_rbsp_trailing(&enc);

   vl_bitstream_flush(&enc);
   *data_size_ptr += vl_bitstream_get_byte_count(&enc);
   vl_bitstream_encoder_free(&enc);
}

static void
emit_nalu_h265_header(struct vl_bitstream_encoder *enc,
                      int nal_unit_type)
{
   enc->prevent_start_code = false;

   vl_bitstream_put_bits(enc, 24, 0);
   vl_bitstream_put_bits(enc, 8, 1);
   vl_bitstream_put_bits(enc, 1, 0);
   vl_bitstream_put_bits(enc, 6, nal_unit_type); /* SPS NAL REF */
   vl_bitstream_put_bits(enc, 6, 0);//nuh_layer_id
   vl_bitstream_put_bits(enc, 3, 1);//nuh_temporal_id_plus1;
   vl_bitstream_flush(enc);

   enc->prevent_start_code = true;
}

static void
encode_h265_profile_tier_level(struct vl_bitstream_encoder *enc,
                               const StdVideoH265ProfileTierLevel *ptl)
{
   vl_bitstream_put_bits(enc, 2, 0);
   vl_bitstream_put_bits(enc, 1, ptl->flags.general_tier_flag);
   vl_bitstream_put_bits(enc, 5, ptl->general_profile_idc);

   for (int j = 0; j < 32; j++)
      vl_bitstream_put_bits(enc, 1, j == ptl->general_profile_idc);

   vl_bitstream_put_bits(enc, 1, ptl->flags.general_progressive_source_flag);
   vl_bitstream_put_bits(enc, 1, ptl->flags.general_interlaced_source_flag);
   vl_bitstream_put_bits(enc, 1, ptl->flags.general_non_packed_constraint_flag);
   vl_bitstream_put_bits(enc, 1, ptl->flags.general_frame_only_constraint_flag);
   vl_bitstream_put_bits(enc, 31, 0);
   vl_bitstream_put_bits(enc, 13, 0);
   vl_bitstream_put_bits(enc, 8, vk_video_get_h265_level(ptl->general_level_idc));
}

void
vk_video_encode_h265_vps(StdVideoH265VideoParameterSet *vps,
                         size_t size_limit,
                         size_t *data_size_ptr,
                         void *data_ptr)
{
   struct vl_bitstream_encoder enc;
   uint32_t data_size = *data_size_ptr;

   vl_bitstream_encoder_clear(&enc, data_ptr, data_size, size_limit);

   emit_nalu_h265_header(&enc, HEVC_NAL_VPS_NUT);

   vl_bitstream_put_bits(&enc, 4, vps->vps_video_parameter_set_id);
   vl_bitstream_put_bits(&enc, 2, 3);
   vl_bitstream_put_bits(&enc, 6, 0);//vps->vps_max_layers_minus1);
   vl_bitstream_put_bits(&enc, 3, vps->vps_max_sub_layers_minus1);
   vl_bitstream_put_bits(&enc, 1, vps->flags.vps_temporal_id_nesting_flag);
   vl_bitstream_put_bits(&enc, 16, 0xffff);

   encode_h265_profile_tier_level(&enc, vps->pProfileTierLevel);

   vl_bitstream_put_bits(&enc, 1, vps->flags.vps_sub_layer_ordering_info_present_flag);

   for (int i = 0; i <= vps->vps_max_sub_layers_minus1; i++) {
      vl_bitstream_exp_golomb_ue(&enc, vps->pDecPicBufMgr->max_dec_pic_buffering_minus1[i]);
      vl_bitstream_exp_golomb_ue(&enc, vps->pDecPicBufMgr->max_num_reorder_pics[i]);
      vl_bitstream_exp_golomb_ue(&enc, vps->pDecPicBufMgr->max_latency_increase_plus1[i]);
   }


   vl_bitstream_put_bits(&enc, 6, 0);//vps->vps_max_layer_id);
   vl_bitstream_exp_golomb_ue(&enc, 0);//vps->vps_num_layer_sets_minus1);
   vl_bitstream_put_bits(&enc, 1, vps->flags.vps_timing_info_present_flag);

   if (vps->flags.vps_timing_info_present_flag) {
      vl_bitstream_put_bits(&enc, 32, vps->vps_num_units_in_tick);
      vl_bitstream_put_bits(&enc, 32, vps->vps_time_scale);
      vl_bitstream_put_bits(&enc, 1, vps->flags.vps_poc_proportional_to_timing_flag);
      if (vps->flags.vps_poc_proportional_to_timing_flag)
         vl_bitstream_exp_golomb_ue(&enc, vps->vps_num_ticks_poc_diff_one_minus1);
      vl_bitstream_exp_golomb_ue(&enc, 0);
   }

   vl_bitstream_put_bits(&enc, 1, 0);   /* vps extension flag */
   vl_bitstream_rbsp_trailing(&enc);

   vl_bitstream_flush(&enc);
   *data_size_ptr += vl_bitstream_get_byte_count(&enc);
   vl_bitstream_encoder_free(&enc);
}

static void
encode_rps(struct vl_bitstream_encoder *enc,
           const StdVideoH265SequenceParameterSet *sps,
           int st_rps_idx)
{
   const StdVideoH265ShortTermRefPicSet *rps = &sps->pShortTermRefPicSet[st_rps_idx];
   if (st_rps_idx != 0)
      vl_bitstream_put_bits(enc, 1, rps->flags.inter_ref_pic_set_prediction_flag);

   if (rps->flags.inter_ref_pic_set_prediction_flag) {
      int ref_rps_idx = st_rps_idx - (rps->delta_idx_minus1 + 1);
      vl_bitstream_put_bits(enc, 1, rps->flags.delta_rps_sign);
      vl_bitstream_exp_golomb_ue(enc, rps->abs_delta_rps_minus1);

      const StdVideoH265ShortTermRefPicSet *rps_ref = &sps->pShortTermRefPicSet[ref_rps_idx];
      int num_delta_pocs = rps_ref->num_negative_pics + rps_ref->num_positive_pics;

      for (int j = 0; j < num_delta_pocs; j++) {
         vl_bitstream_put_bits(enc, 1, !!(rps->used_by_curr_pic_flag & (1 << j)));
         if (!(rps->used_by_curr_pic_flag & (1 << j))) {
            vl_bitstream_put_bits(enc, 1, !!(rps->use_delta_flag & (1 << j)));
         }
      }
   } else {
      vl_bitstream_exp_golomb_ue(enc, rps->num_negative_pics);
      vl_bitstream_exp_golomb_ue(enc, rps->num_positive_pics);

      for (int i = 0; i < rps->num_negative_pics; i++) {
         vl_bitstream_exp_golomb_ue(enc, rps->delta_poc_s0_minus1[i]);
         vl_bitstream_put_bits(enc, 1, !!(rps->used_by_curr_pic_s0_flag & (1 << i)));
      }
      for (int i = 0; i < rps->num_positive_pics; i++) {
         vl_bitstream_exp_golomb_ue(enc, rps->delta_poc_s1_minus1[i]);
         vl_bitstream_put_bits(enc, 1, !!(rps->used_by_curr_pic_s1_flag & (1 << i)));
      }
   }
}

void
vk_video_encode_h265_sps(StdVideoH265SequenceParameterSet *sps,
                         size_t size_limit,
                         size_t *data_size_ptr,
                         void *data_ptr)
{
   struct vl_bitstream_encoder enc;
   uint32_t data_size = *data_size_ptr;

   vl_bitstream_encoder_clear(&enc, data_ptr, data_size, size_limit);

   emit_nalu_h265_header(&enc, HEVC_NAL_SPS_NUT);

   vl_bitstream_put_bits(&enc, 4, sps->sps_video_parameter_set_id);
   vl_bitstream_put_bits(&enc, 3, sps->sps_max_sub_layers_minus1);
   vl_bitstream_put_bits(&enc, 1, sps->flags.sps_temporal_id_nesting_flag);

   encode_h265_profile_tier_level(&enc, sps->pProfileTierLevel);

   vl_bitstream_exp_golomb_ue(&enc, sps->sps_seq_parameter_set_id);
   vl_bitstream_exp_golomb_ue(&enc, sps->chroma_format_idc);

   vl_bitstream_exp_golomb_ue(&enc, sps->pic_width_in_luma_samples);
   vl_bitstream_exp_golomb_ue(&enc, sps->pic_height_in_luma_samples);

   vl_bitstream_put_bits(&enc, 1, sps->flags.conformance_window_flag);

   if (sps->flags.conformance_window_flag) {
      vl_bitstream_exp_golomb_ue(&enc, sps->conf_win_left_offset);
      vl_bitstream_exp_golomb_ue(&enc, sps->conf_win_right_offset);
      vl_bitstream_exp_golomb_ue(&enc, sps->conf_win_top_offset);
      vl_bitstream_exp_golomb_ue(&enc, sps->conf_win_bottom_offset);
   }

   vl_bitstream_exp_golomb_ue(&enc, sps->bit_depth_luma_minus8);
   vl_bitstream_exp_golomb_ue(&enc, sps->bit_depth_chroma_minus8);

   vl_bitstream_exp_golomb_ue(&enc, sps->log2_max_pic_order_cnt_lsb_minus4);
   vl_bitstream_put_bits(&enc, 1, sps->flags.sps_sub_layer_ordering_info_present_flag);

   for (int i = 0; i <= sps->sps_max_sub_layers_minus1; i++) {
      vl_bitstream_exp_golomb_ue(&enc, sps->pDecPicBufMgr->max_dec_pic_buffering_minus1[i]);
      vl_bitstream_exp_golomb_ue(&enc, sps->pDecPicBufMgr->max_num_reorder_pics[i]);
      vl_bitstream_exp_golomb_ue(&enc, sps->pDecPicBufMgr->max_latency_increase_plus1[i]);
   }

   vl_bitstream_exp_golomb_ue(&enc, sps->log2_min_luma_coding_block_size_minus3);
   vl_bitstream_exp_golomb_ue(&enc, sps->log2_diff_max_min_luma_coding_block_size);
   vl_bitstream_exp_golomb_ue(&enc, sps->log2_min_luma_transform_block_size_minus2);
   vl_bitstream_exp_golomb_ue(&enc, sps->log2_diff_max_min_luma_transform_block_size);

   vl_bitstream_exp_golomb_ue(&enc, sps->max_transform_hierarchy_depth_inter);
   vl_bitstream_exp_golomb_ue(&enc, sps->max_transform_hierarchy_depth_intra);

   vl_bitstream_put_bits(&enc, 1, sps->flags.scaling_list_enabled_flag);

   vl_bitstream_put_bits(&enc, 1, sps->flags.amp_enabled_flag);
   vl_bitstream_put_bits(&enc, 1, sps->flags.sample_adaptive_offset_enabled_flag);

   vl_bitstream_put_bits(&enc, 1, sps->flags.pcm_enabled_flag);

   if (sps->flags.pcm_enabled_flag) {
      vl_bitstream_put_bits(&enc, 4, sps->bit_depth_luma_minus8 + 7);
      vl_bitstream_put_bits(&enc, 4, sps->bit_depth_chroma_minus8 + 7);
      vl_bitstream_exp_golomb_ue(&enc, sps->log2_min_luma_coding_block_size_minus3);
      vl_bitstream_exp_golomb_ue(&enc, sps->log2_diff_max_min_luma_coding_block_size);
      vl_bitstream_put_bits(&enc, 1, sps->flags.pcm_loop_filter_disabled_flag);
   }

   vl_bitstream_exp_golomb_ue(&enc, sps->num_short_term_ref_pic_sets);
   for (int i = 0; i < sps->num_short_term_ref_pic_sets; i++)
      encode_rps(&enc, sps, i);

   vl_bitstream_put_bits(&enc, 1, sps->flags.long_term_ref_pics_present_flag);
   if (sps->flags.long_term_ref_pics_present_flag) {
      vl_bitstream_exp_golomb_ue(&enc, sps->num_long_term_ref_pics_sps);
      for (int i = 0; i < sps->num_long_term_ref_pics_sps; i++) {
         vl_bitstream_put_bits(&enc, sps->log2_max_pic_order_cnt_lsb_minus4 + 4, sps->pLongTermRefPicsSps->lt_ref_pic_poc_lsb_sps[i]);
         vl_bitstream_put_bits(&enc, 1, sps->pLongTermRefPicsSps->used_by_curr_pic_lt_sps_flag);
      }
   }

   vl_bitstream_put_bits(&enc, 1, sps->flags.sps_temporal_mvp_enabled_flag);
   vl_bitstream_put_bits(&enc, 1, sps->flags.strong_intra_smoothing_enabled_flag);
   vl_bitstream_put_bits(&enc, 1, sps->flags.vui_parameters_present_flag);

   if (sps->flags.vui_parameters_present_flag) {
      const StdVideoH265SequenceParameterSetVui *vui = sps->pSequenceParameterSetVui;
      vl_bitstream_put_bits(&enc, 1, vui->flags.aspect_ratio_info_present_flag);
      if (vui->flags.aspect_ratio_info_present_flag) {
         vl_bitstream_put_bits(&enc, 8, vui->aspect_ratio_idc);
         if (vui->aspect_ratio_idc == STD_VIDEO_H265_ASPECT_RATIO_IDC_EXTENDED_SAR) {
            vl_bitstream_put_bits(&enc, 16, vui->sar_width);
            vl_bitstream_put_bits(&enc, 16, vui->sar_height);
         }
      }
      vl_bitstream_put_bits(&enc, 1, vui->flags.overscan_info_present_flag);
      if (vui->flags.overscan_info_present_flag)
         vl_bitstream_put_bits(&enc, 1, vui->flags.overscan_appropriate_flag);
      vl_bitstream_put_bits(&enc, 1, vui->flags.video_signal_type_present_flag);
      if (vui->flags.video_signal_type_present_flag) {
         vl_bitstream_put_bits(&enc, 3, vui->video_format);
         vl_bitstream_put_bits(&enc, 1, vui->flags.video_full_range_flag);
         vl_bitstream_put_bits(&enc, 1, vui->flags.colour_description_present_flag);
         if (vui->flags.colour_description_present_flag) {
            vl_bitstream_put_bits(&enc, 8, vui->colour_primaries);
            vl_bitstream_put_bits(&enc, 8, vui->transfer_characteristics);
            vl_bitstream_put_bits(&enc, 8, vui->matrix_coeffs);
         }
      }
      vl_bitstream_put_bits(&enc, 1, vui->flags.chroma_loc_info_present_flag);
      if (vui->flags.chroma_loc_info_present_flag) {
         vl_bitstream_exp_golomb_ue(&enc, vui->chroma_sample_loc_type_top_field);
         vl_bitstream_exp_golomb_ue(&enc, vui->chroma_sample_loc_type_bottom_field);
      }
      vl_bitstream_put_bits(&enc, 1, vui->flags.neutral_chroma_indication_flag);
      vl_bitstream_put_bits(&enc, 1, vui->flags.field_seq_flag);
      vl_bitstream_put_bits(&enc, 1, vui->flags.frame_field_info_present_flag);
      vl_bitstream_put_bits(&enc, 1, vui->flags.default_display_window_flag);
      if (vui->flags.default_display_window_flag) {
         vl_bitstream_exp_golomb_ue(&enc, vui->def_disp_win_left_offset);
         vl_bitstream_exp_golomb_ue(&enc, vui->def_disp_win_right_offset);
         vl_bitstream_exp_golomb_ue(&enc, vui->def_disp_win_top_offset);
         vl_bitstream_exp_golomb_ue(&enc, vui->def_disp_win_bottom_offset);
      }
      vl_bitstream_put_bits(&enc, 1, vui->flags.vui_timing_info_present_flag);
      if (vui->flags.vui_timing_info_present_flag) {
         vl_bitstream_put_bits(&enc, 32, vui->vui_num_units_in_tick);
         vl_bitstream_put_bits(&enc, 32, vui->vui_time_scale);
         vl_bitstream_put_bits(&enc, 1, vui->flags.vui_poc_proportional_to_timing_flag);
         if (vui->flags.vui_poc_proportional_to_timing_flag)
            vl_bitstream_exp_golomb_ue(&enc, vui->vui_num_ticks_poc_diff_one_minus1);
         vl_bitstream_put_bits(&enc, 1, 0);//vui->flags.vui_hrd_parameters_present_flag);
         // HRD
      }

      vl_bitstream_put_bits(&enc, 1, vui->flags.bitstream_restriction_flag);
      if (vui->flags.bitstream_restriction_flag) {
         vl_bitstream_put_bits(&enc, 1, vui->flags.tiles_fixed_structure_flag);
         vl_bitstream_put_bits(&enc, 1, vui->flags.motion_vectors_over_pic_boundaries_flag);
         vl_bitstream_put_bits(&enc, 1, vui->flags.restricted_ref_pic_lists_flag);
         vl_bitstream_exp_golomb_ue(&enc, vui->min_spatial_segmentation_idc);
         vl_bitstream_exp_golomb_ue(&enc, vui->max_bytes_per_pic_denom);
         vl_bitstream_exp_golomb_ue(&enc, vui->max_bits_per_min_cu_denom);
         vl_bitstream_exp_golomb_ue(&enc, vui->log2_max_mv_length_horizontal);
            vl_bitstream_exp_golomb_ue(&enc, vui->log2_max_mv_length_vertical);
      }
   }

   vl_bitstream_put_bits(&enc, 1, 0);   /* sps extension flg */
   vl_bitstream_rbsp_trailing(&enc);

   vl_bitstream_flush(&enc);
   *data_size_ptr += vl_bitstream_get_byte_count(&enc);
   vl_bitstream_encoder_free(&enc);
}

void
vk_video_encode_h265_pps(StdVideoH265PictureParameterSet *pps,
                         size_t size_limit,
                         size_t *data_size_ptr,
                         void *data_ptr)
{
   struct vl_bitstream_encoder enc;
   uint32_t data_size = *data_size_ptr;

   vl_bitstream_encoder_clear(&enc, data_ptr, data_size, size_limit);

   emit_nalu_h265_header(&enc, HEVC_NAL_PPS_NUT);
   vl_bitstream_exp_golomb_ue(&enc, pps->pps_pic_parameter_set_id);
   vl_bitstream_exp_golomb_ue(&enc, pps->pps_seq_parameter_set_id);

   vl_bitstream_put_bits(&enc, 1, pps->flags.dependent_slice_segments_enabled_flag);

   vl_bitstream_put_bits(&enc, 1, pps->flags.output_flag_present_flag);
   vl_bitstream_put_bits(&enc, 3, pps->num_extra_slice_header_bits);

   vl_bitstream_put_bits(&enc, 1, pps->flags.sign_data_hiding_enabled_flag);
   vl_bitstream_put_bits(&enc, 1, pps->flags.cabac_init_present_flag);

   vl_bitstream_exp_golomb_ue(&enc, pps->num_ref_idx_l0_default_active_minus1);
   vl_bitstream_exp_golomb_ue(&enc, pps->num_ref_idx_l1_default_active_minus1);

   vl_bitstream_exp_golomb_se(&enc, pps->init_qp_minus26);

   vl_bitstream_put_bits(&enc, 1, pps->flags.constrained_intra_pred_flag);
   vl_bitstream_put_bits(&enc, 1, pps->flags.transform_skip_enabled_flag);
   vl_bitstream_put_bits(&enc, 1, pps->flags.cu_qp_delta_enabled_flag);

   if (pps->flags.cu_qp_delta_enabled_flag)
      vl_bitstream_exp_golomb_ue(&enc, pps->diff_cu_qp_delta_depth);

   vl_bitstream_exp_golomb_se(&enc, pps->pps_cb_qp_offset);
   vl_bitstream_exp_golomb_se(&enc, pps->pps_cr_qp_offset);

   vl_bitstream_put_bits(&enc, 1, pps->flags.pps_slice_chroma_qp_offsets_present_flag);
   vl_bitstream_put_bits(&enc, 1, pps->flags.weighted_pred_flag);
   vl_bitstream_put_bits(&enc, 1, pps->flags.weighted_bipred_flag);
   vl_bitstream_put_bits(&enc, 1, pps->flags.transquant_bypass_enabled_flag);

   vl_bitstream_put_bits(&enc, 1, pps->flags.tiles_enabled_flag);
   vl_bitstream_put_bits(&enc, 1, pps->flags.entropy_coding_sync_enabled_flag);

   assert (!pps->flags.tiles_enabled_flag);

   vl_bitstream_put_bits(&enc, 1, pps->flags.pps_loop_filter_across_slices_enabled_flag);
   vl_bitstream_put_bits(&enc, 1, pps->flags.deblocking_filter_control_present_flag);

   if (pps->flags.deblocking_filter_control_present_flag) {
      vl_bitstream_put_bits(&enc, 1, pps->flags.deblocking_filter_override_enabled_flag);
      vl_bitstream_put_bits(&enc, 1, pps->flags.pps_deblocking_filter_disabled_flag);
      if (!pps->flags.pps_deblocking_filter_disabled_flag) {
         vl_bitstream_exp_golomb_se(&enc, pps->pps_beta_offset_div2);
         vl_bitstream_exp_golomb_se(&enc, pps->pps_tc_offset_div2);
      }
   }

   vl_bitstream_put_bits(&enc, 1, pps->flags.pps_scaling_list_data_present_flag);
   assert (!pps->flags.pps_scaling_list_data_present_flag);

   vl_bitstream_put_bits(&enc, 1, pps->flags.lists_modification_present_flag);
   vl_bitstream_exp_golomb_ue(&enc, pps->log2_parallel_merge_level_minus2);
   vl_bitstream_put_bits(&enc, 1, pps->flags.slice_segment_header_extension_present_flag);

   vl_bitstream_put_bits(&enc, 1, 0); /* pps extension flag */
   vl_bitstream_rbsp_trailing(&enc);

   vl_bitstream_flush(&enc);
   *data_size_ptr += vl_bitstream_get_byte_count(&enc);
   vl_bitstream_encoder_free(&enc);
}
