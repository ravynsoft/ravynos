/**************************************************************************
 *
 * Copyright 2017 Advanced Micro Devices, Inc.
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

#include "va_private.h"

void vlVaHandlePictureParameterBufferMJPEG(vlVaDriver *drv, vlVaContext *context, vlVaBuffer *buf)
{
   VAPictureParameterBufferJPEGBaseline *mjpeg = buf->data;
   unsigned sf;
   int i;

   assert(buf->size >= sizeof(VAPictureParameterBufferJPEGBaseline) && buf->num_elements == 1);

   context->desc.mjpeg.picture_parameter.picture_width = mjpeg->picture_width;
   context->desc.mjpeg.picture_parameter.picture_height = mjpeg->picture_height;

   STATIC_ASSERT(sizeof(mjpeg->components) ==
                 sizeof(context->desc.mjpeg.picture_parameter.components));
   for (i = 0; i < MIN2(mjpeg->num_components, ARRAY_SIZE(mjpeg->components)); ++i) {
      context->desc.mjpeg.picture_parameter.components[i].component_id =
         mjpeg->components[i].component_id;
      context->desc.mjpeg.picture_parameter.components[i].h_sampling_factor =
         mjpeg->components[i].h_sampling_factor;
      context->desc.mjpeg.picture_parameter.components[i].v_sampling_factor =
         mjpeg->components[i].v_sampling_factor;
      context->desc.mjpeg.picture_parameter.components[i].quantiser_table_selector =
         mjpeg->components[i].quantiser_table_selector;

      sf = mjpeg->components[i].h_sampling_factor << 4 | mjpeg->components[i].v_sampling_factor;
      context->mjpeg.sampling_factor <<= 8;
      context->mjpeg.sampling_factor |= sf;
   }

   context->desc.mjpeg.picture_parameter.num_components = mjpeg->num_components;

#if VA_CHECK_VERSION(1, 21, 0)
   context->desc.mjpeg.picture_parameter.crop_x = mjpeg->crop_rectangle.x;
   context->desc.mjpeg.picture_parameter.crop_y = mjpeg->crop_rectangle.y;
   context->desc.mjpeg.picture_parameter.crop_width = mjpeg->crop_rectangle.width;
   context->desc.mjpeg.picture_parameter.crop_height = mjpeg->crop_rectangle.height;
#else
   context->desc.mjpeg.picture_parameter.crop_x = mjpeg->va_reserved[0] & 0xffff;
   context->desc.mjpeg.picture_parameter.crop_y = (mjpeg->va_reserved[0] >> 16) & 0xffff;
   context->desc.mjpeg.picture_parameter.crop_width = mjpeg->va_reserved[1] & 0xffff;
   context->desc.mjpeg.picture_parameter.crop_height = (mjpeg->va_reserved[1] >> 16) & 0xffff;
#endif

}

void vlVaHandleIQMatrixBufferMJPEG(vlVaContext *context, vlVaBuffer *buf)
{
   VAIQMatrixBufferJPEGBaseline *mjpeg = buf->data;

   assert(buf->size >= sizeof(VAIQMatrixBufferJPEGBaseline) && buf->num_elements == 1);

   memcpy(&context->desc.mjpeg.quantization_table.load_quantiser_table, mjpeg->load_quantiser_table, 4);
   memcpy(&context->desc.mjpeg.quantization_table.quantiser_table, mjpeg->quantiser_table, 4 * 64);
}

void vlVaHandleHuffmanTableBufferType(vlVaContext *context, vlVaBuffer *buf)
{
   VAHuffmanTableBufferJPEGBaseline *mjpeg = buf->data;
   int i;

   assert(buf->size >= sizeof(VASliceParameterBufferJPEGBaseline) && buf->num_elements == 1);

   STATIC_ASSERT(sizeof(mjpeg->load_huffman_table) ==
                 sizeof(context->desc.mjpeg.huffman_table.load_huffman_table));
   for (i = 0; i < 2; ++i) {
      context->desc.mjpeg.huffman_table.load_huffman_table[i] = mjpeg->load_huffman_table[i];

      memcpy(&context->desc.mjpeg.huffman_table.table[i].num_dc_codes,
         mjpeg->huffman_table[i].num_dc_codes, 16);
      memcpy(&context->desc.mjpeg.huffman_table.table[i].dc_values,
         mjpeg->huffman_table[i].dc_values, 12);
      memcpy(&context->desc.mjpeg.huffman_table.table[i].num_ac_codes,
         mjpeg->huffman_table[i].num_ac_codes, 16);
      memcpy(&context->desc.mjpeg.huffman_table.table[i].ac_values,
         mjpeg->huffman_table[i].ac_values, 162);
      memcpy(&context->desc.mjpeg.huffman_table.table[i].pad, mjpeg->huffman_table[i].pad, 2);
   }
}

void vlVaHandleSliceParameterBufferMJPEG(vlVaContext *context, vlVaBuffer *buf)
{
   VASliceParameterBufferJPEGBaseline *mjpeg = buf->data;
   int i;

   assert(buf->size >= sizeof(VASliceParameterBufferJPEGBaseline) && buf->num_elements == 1);

   context->desc.mjpeg.slice_parameter.slice_data_size = mjpeg->slice_data_size;
   context->desc.mjpeg.slice_parameter.slice_data_offset = mjpeg->slice_data_offset;
   context->desc.mjpeg.slice_parameter.slice_data_flag = mjpeg->slice_data_flag;
   context->desc.mjpeg.slice_parameter.slice_horizontal_position = mjpeg->slice_horizontal_position;
   context->desc.mjpeg.slice_parameter.slice_vertical_position = mjpeg->slice_vertical_position;

   STATIC_ASSERT(sizeof(mjpeg->components) ==
                 sizeof(context->desc.mjpeg.slice_parameter.components));
   for (i = 0; i < MIN2(mjpeg->num_components, ARRAY_SIZE(mjpeg->components)); ++i) {
      context->desc.mjpeg.slice_parameter.components[i].component_selector =
         mjpeg->components[i].component_selector;
      context->desc.mjpeg.slice_parameter.components[i].dc_table_selector =
         mjpeg->components[i].dc_table_selector;
      context->desc.mjpeg.slice_parameter.components[i].ac_table_selector =
         mjpeg->components[i].ac_table_selector;
   }

   context->desc.mjpeg.slice_parameter.num_components = mjpeg->num_components;
   context->desc.mjpeg.slice_parameter.restart_interval = mjpeg->restart_interval;
   context->desc.mjpeg.slice_parameter.num_mcus = mjpeg->num_mcus;
}

void vlVaGetJpegSliceHeader(vlVaContext *context)
{
   int size = 0, saved_size, len_pos, i;
   uint16_t *bs;
   uint8_t *p = context->mjpeg.slice_header;

   /* SOI */
   p[size++] = 0xff;
   p[size++] = 0xd8;

   /* DQT */
   p[size++] = 0xff;
   p[size++] = 0xdb;

   len_pos = size++;
   size++;

   for (i = 0; i < 4; ++i) {
      if (context->desc.mjpeg.quantization_table.load_quantiser_table[i] == 0)
         continue;

      p[size++] = i;
      memcpy((p + size), &context->desc.mjpeg.quantization_table.quantiser_table[i], 64);
      size += 64;
   }

   bs = (uint16_t*)&p[len_pos];
   *bs = util_bswap16(size - 4);

   saved_size = size;

   /* DHT */
   p[size++] = 0xff;
   p[size++] = 0xc4;

   len_pos = size++;
   size++;

   for (i = 0; i < 2; ++i) {
      int num = 0, j;

      if (context->desc.mjpeg.huffman_table.load_huffman_table[i] == 0)
         continue;

      p[size++] = 0x00 | i;
      memcpy((p + size), &context->desc.mjpeg.huffman_table.table[i].num_dc_codes, 16);
      size += 16;
      for (j = 0; j < 16; ++j)
         num += context->desc.mjpeg.huffman_table.table[i].num_dc_codes[j];
      assert(num <= 12);
      memcpy((p + size), &context->desc.mjpeg.huffman_table.table[i].dc_values, num);
      size += num;
   }

   for (i = 0; i < 2; ++i) {
      int num = 0, j;

      if (context->desc.mjpeg.huffman_table.load_huffman_table[i] == 0)
         continue;

      p[size++] = 0x10 | i;
      memcpy((p + size), &context->desc.mjpeg.huffman_table.table[i].num_ac_codes, 16);
      size += 16;
      for (j = 0; j < 16; ++j)
         num += context->desc.mjpeg.huffman_table.table[i].num_ac_codes[j];
      assert(num <= 162);
      memcpy((p + size), &context->desc.mjpeg.huffman_table.table[i].ac_values, num);
      size += num;
   }

   bs = (uint16_t*)&p[len_pos];
   *bs = util_bswap16(size - saved_size - 2);

   saved_size = size;

   /* DRI */
   if (context->desc.mjpeg.slice_parameter.restart_interval) {
      p[size++] = 0xff;
      p[size++] = 0xdd;
      p[size++] = 0x00;
      p[size++] = 0x04;
      bs = (uint16_t*)&p[size++];
      *bs = util_bswap16(context->desc.mjpeg.slice_parameter.restart_interval);
      saved_size = ++size;
   }

   /* SOF */
   p[size++] = 0xff;
   p[size++] = 0xc0;

   len_pos = size++;
   size++;

   p[size++] = 0x08;

   bs = (uint16_t*)&p[size++];
   *bs = util_bswap16(context->desc.mjpeg.picture_parameter.picture_height);
   size++;

   bs = (uint16_t*)&p[size++];
   *bs = util_bswap16(context->desc.mjpeg.picture_parameter.picture_width);
   size++;

   p[size++] = context->desc.mjpeg.picture_parameter.num_components;

   for (i = 0; i < context->desc.mjpeg.picture_parameter.num_components; ++i) {
      p[size++] = context->desc.mjpeg.picture_parameter.components[i].component_id;
      p[size++] = context->desc.mjpeg.picture_parameter.components[i].h_sampling_factor << 4 |
                 context->desc.mjpeg.picture_parameter.components[i].v_sampling_factor;
      p[size++] = context->desc.mjpeg.picture_parameter.components[i].quantiser_table_selector;
   }

   bs = (uint16_t*)&p[len_pos];
   *bs = util_bswap16(size - saved_size - 2);

   saved_size = size;

   /* SOS */
   p[size++] = 0xff;
   p[size++] = 0xda;

   len_pos = size++;
   size++;

   p[size++] = context->desc.mjpeg.slice_parameter.num_components;

   for (i = 0; i < context->desc.mjpeg.slice_parameter.num_components; ++i) {
      p[size++] = context->desc.mjpeg.slice_parameter.components[i].component_selector;
      p[size++] = context->desc.mjpeg.slice_parameter.components[i].dc_table_selector << 4 |
                 context->desc.mjpeg.slice_parameter.components[i].ac_table_selector;
   }

   p[size++] = 0x00;
   p[size++] = 0x3f;
   p[size++] = 0x00;

   bs = (uint16_t*)&p[len_pos];
   *bs = util_bswap16(size - saved_size - 2);

   context->mjpeg.slice_header_size = size;
}
