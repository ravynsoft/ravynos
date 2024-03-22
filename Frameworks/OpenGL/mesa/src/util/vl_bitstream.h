/*
 * Copyright 2023 Red Hat
 * SPDX-License-Identifier: MIT
 *
 * This is a C rewrite of pieces of the d3d12 frontend which is:
 * Copyright © Microsoft Corporation
 */
#ifndef VL_BITSTREAM_H
#define VL_BITSTREAM_H

struct vl_bitstream_encoder {
   uint8_t *bits;
   uint32_t bits_buffer_size;
   uint32_t offset;

   uint32_t enc_buffer;

   int32_t bits_to_go;
   bool prevent_start_code;

   bool internal_buffer;
   bool overflow;
};

#define VL_BITSTREAM_MAX_BUFFER 256

static inline void
vl_bitstream_encoder_clear(struct vl_bitstream_encoder *enc,
                           void *buffer_base,
                           size_t buffer_offset,
                           size_t buffer_limit)
{
   memset(enc, 0, sizeof(*enc));
   enc->bits_to_go = 32;

   if (!buffer_base) {
      enc->bits = malloc(VL_BITSTREAM_MAX_BUFFER);
      enc->bits_buffer_size = VL_BITSTREAM_MAX_BUFFER;
      enc->internal_buffer = true;
   } else {
      enc->bits = (uint8_t *)buffer_base + buffer_offset;
      enc->bits_buffer_size = buffer_limit;
   }
}

static inline void
vl_bitstream_encoder_free(struct vl_bitstream_encoder *enc)
{
   if (enc->internal_buffer)
      free(enc->bits);
}

static inline int
vl_bitstream_get_num_bits_for_byte_align(struct vl_bitstream_encoder *enc)
{
   return enc->bits_to_go & 7;
}

static inline int
vl_bitstream_get_byte_count(struct vl_bitstream_encoder *enc)
{
   return enc->offset + ((32 - enc->bits_to_go) >> 3);
}

static inline bool
vl_bitstream_is_byte_aligned(struct vl_bitstream_encoder *enc)
{
   if (enc->overflow)
      enc->bits_to_go = 32;
   return !(enc->bits_to_go & 7);
}

static inline void
vl_bitstream_write_byte_start_code(struct vl_bitstream_encoder *enc, uint8_t val)
{
   int offset = enc->offset;
   uint8_t *buffer = enc->bits + enc->offset;
   if (enc->prevent_start_code && enc->offset > 1) {
      if (((val & 0xfc) | buffer[-2] | buffer[-1]) == 0) {
         *buffer++ = 3;
         offset++;
      }
   }

   *buffer = val;
   offset++;
   enc->offset = offset;
}

static inline bool
vl_bitstream_verify_buffer(struct vl_bitstream_encoder *enc, uint32_t bytes_to_write)
{
   if (enc->overflow)
      return false;

   if (enc->offset + bytes_to_write > enc->bits_buffer_size) {
      enc->overflow = true;
      return false;
   }
   return true;
}

static inline void
vl_bitstream_flush(struct vl_bitstream_encoder *enc)
{
   ASSERTED bool is_aligned = vl_bitstream_is_byte_aligned(enc);
   assert (is_aligned);

   uint32_t temp = (uint32_t)(32 - enc->bits_to_go);

   if (!vl_bitstream_verify_buffer(enc, temp >> 3)) {
      return;
   }

   while (temp > 0) {
      vl_bitstream_write_byte_start_code(enc, (uint8_t)(enc->enc_buffer >> 24));
      enc->enc_buffer <<= 8;
      temp -= 8;
   }

   enc->bits_to_go = 32;
   enc->enc_buffer = 0;
}

static inline void
vl_bitstream_put_bits(struct vl_bitstream_encoder *enc, int bits_count, uint32_t bits_val)
{
   if (bits_count < enc->bits_to_go) {
      enc->enc_buffer |= (bits_val << (enc->bits_to_go - bits_count));
      enc->bits_to_go -= bits_count;
   } else if (vl_bitstream_verify_buffer(enc, 4)) {
      int left_over_bits = bits_count - enc->bits_to_go;
      enc->enc_buffer |= (bits_val >> left_over_bits);

      {
         uint8_t *temp = (uint8_t *)&enc->enc_buffer;
         vl_bitstream_write_byte_start_code(enc, *(temp + 3));
         vl_bitstream_write_byte_start_code(enc, *(temp + 2));
         vl_bitstream_write_byte_start_code(enc, *(temp + 1));
         vl_bitstream_write_byte_start_code(enc, *temp);
      }

      enc->enc_buffer = 0;
      enc->bits_to_go = 32 - left_over_bits;

      if (left_over_bits > 0)
         enc->enc_buffer = (bits_val << (32 - left_over_bits));
   }
}

static inline int
vl_bitstream_get_exp_golomb0_code_len(uint32_t val)
{
   int len = 0;
   val++;

   if (val >= 0x10000) {
      val >>= 16;
      len += 16;
   }
   if (val >= 0x100) {
      val >>= 8;
      len += 8;
   }
   assert(val < 256);

   return len + util_logbase2(val);
}

static inline void
vl_bitstream_exp_golomb_ue(struct vl_bitstream_encoder *enc, uint32_t val)
{
   if (val != UINT32_MAX) {
      int len = vl_bitstream_get_exp_golomb0_code_len(val);
      vl_bitstream_put_bits(enc, (len << 1) + 1, val + 1);
   } else {
      vl_bitstream_put_bits(enc, 32, 0);
      vl_bitstream_put_bits(enc, 1, 1);
      vl_bitstream_put_bits(enc, 32, 1);
   }
}

static inline void
vl_bitstream_exp_golomb_se(struct vl_bitstream_encoder *enc, uint32_t val)
{
   if (val > 0)
      vl_bitstream_exp_golomb_ue(enc, (val << 1) - 1);
   else
      vl_bitstream_exp_golomb_ue(enc, ((-val) << 1) - (val == INT_MIN));
}

static inline void
vl_bitstream_rbsp_trailing(struct vl_bitstream_encoder *enc)
{
   vl_bitstream_put_bits(enc, 1, 1);
   int left = vl_bitstream_get_num_bits_for_byte_align(enc);

   if (left)
      vl_bitstream_put_bits(enc, left, 0);

   ASSERTED bool is_aligned = vl_bitstream_is_byte_aligned(enc);
   assert(is_aligned);
}
#endif
