/*
 * Copyright 2015 Philip Taylor <philip@zaynar.co.uk>
 * Copyright 2018 Advanced Micro Devices, Inc.
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
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */

/**
 * \file texcompress_astc.c
 *
 * Decompression code for GL_KHR_texture_compression_astc_ldr, which is just
 * ASTC 2D LDR.
 *
 * The ASTC 2D LDR decoder (without the sRGB part) was copied from the OASTC
 * library written by Philip Taylor. I added sRGB support and adjusted it for
 * Mesa. - Marek
 */

#include "texcompress_astc.h"
#include "macros.h"
#include "util/half_float.h"
#include <stdio.h>
#include <cstdlib>  // for abort() on windows

static bool VERBOSE_DECODE = false;
static bool VERBOSE_WRITE = false;

class decode_error
{
public:
   enum type {
      ok,
      unsupported_hdr_void_extent,
      reserved_block_mode_1,
      reserved_block_mode_2,
      dual_plane_and_too_many_partitions,
      invalid_range_in_void_extent,
      weight_grid_exceeds_block_size,
      invalid_colour_endpoints_size,
      invalid_colour_endpoints_count,
      invalid_weight_bits,
      invalid_num_weights,
   };
};


struct cem_range {
   uint8_t max;
   uint8_t t, q, b;
};

/* Based on the Color Unquantization Parameters table,
 * plus the bit-only representations, sorted by increasing size
 */
static cem_range cem_ranges[] = {
   { 5, 1, 0, 1 },
   { 7, 0, 0, 3 },
   { 9, 0, 1, 1 },
   { 11, 1, 0, 2 },
   { 15, 0, 0, 4 },
   { 19, 0, 1, 2 },
   { 23, 1, 0, 3 },
   { 31, 0, 0, 5 },
   { 39, 0, 1, 3 },
   { 47, 1, 0, 4 },
   { 63, 0, 0, 6 },
   { 79, 0, 1, 4 },
   { 95, 1, 0, 5 },
   { 127, 0, 0, 7 },
   { 159, 0, 1, 5 },
   { 191, 1, 0, 6 },
   { 255, 0, 0, 8 },
};

#define CAT_BITS_2(a, b)          ( ((a) << 1) | (b) )
#define CAT_BITS_3(a, b, c)       ( ((a) << 2) | ((b) << 1) | (c) )
#define CAT_BITS_4(a, b, c, d)    ( ((a) << 3) | ((b) << 2) | ((c) << 1) | (d) )
#define CAT_BITS_5(a, b, c, d, e) ( ((a) << 4) | ((b) << 3) | ((c) << 2) | ((d) << 1) | (e) )

/**
 * Unpack 5n+8 bits from 'in' into 5 output values.
 * If n <= 4 then T should be uint32_t, else it must be uint64_t.
 */
template <typename T>
static void unpack_trit_block(int n, T in, uint8_t *out)
{
   assert(n <= 6); /* else output will overflow uint8_t */

   uint8_t T0 = (in >> (n)) & 0x1;
   uint8_t T1 = (in >> (n+1)) & 0x1;
   uint8_t T2 = (in >> (2*n+2)) & 0x1;
   uint8_t T3 = (in >> (2*n+3)) & 0x1;
   uint8_t T4 = (in >> (3*n+4)) & 0x1;
   uint8_t T5 = (in >> (4*n+5)) & 0x1;
   uint8_t T6 = (in >> (4*n+6)) & 0x1;
   uint8_t T7 = (in >> (5*n+7)) & 0x1;
   uint8_t mmask = (1 << n) - 1;
   uint8_t m0 = (in >> (0)) & mmask;
   uint8_t m1 = (in >> (n+2)) & mmask;
   uint8_t m2 = (in >> (2*n+4)) & mmask;
   uint8_t m3 = (in >> (3*n+5)) & mmask;
   uint8_t m4 = (in >> (4*n+7)) & mmask;

   uint8_t C;
   uint8_t t4, t3, t2, t1, t0;
   if (CAT_BITS_3(T4, T3, T2) == 0x7) {
      C = CAT_BITS_5(T7, T6, T5, T1, T0);
      t4 = t3 = 2;
   } else {
      C = CAT_BITS_5(T4, T3, T2, T1, T0);
      if (CAT_BITS_2(T6, T5) == 0x3) {
         t4 = 2;
         t3 = T7;
      } else {
         t4 = T7;
         t3 = CAT_BITS_2(T6, T5);
      }
   }

   if ((C & 0x3) == 0x3) {
      t2 = 2;
      t1 = (C >> 4) & 0x1;
      uint8_t C3 = (C >> 3) & 0x1;
      uint8_t C2 = (C >> 2) & 0x1;
      t0 = (C3 << 1) | (C2 & ~C3);
   } else if (((C >> 2) & 0x3) == 0x3) {
      t2 = 2;
      t1 = 2;
      t0 = C & 0x3;
   } else {
      t2 = (C >> 4) & 0x1;
      t1 = (C >> 2) & 0x3;
      uint8_t C1 = (C >> 1) & 0x1;
      uint8_t C0 = (C >> 0) & 0x1;
      t0 = (C1 << 1) | (C0 & ~C1);
   }

   out[0] = (t0 << n) | m0;
   out[1] = (t1 << n) | m1;
   out[2] = (t2 << n) | m2;
   out[3] = (t3 << n) | m3;
   out[4] = (t4 << n) | m4;
}

/**
 * Unpack 3n+7 bits from 'in' into 3 output values
 */
static void unpack_quint_block(int n, uint32_t in, uint8_t *out)
{
   assert(n <= 5); /* else output will overflow uint8_t */

   uint8_t Q0 = (in >> (n)) & 0x1;
   uint8_t Q1 = (in >> (n+1)) & 0x1;
   uint8_t Q2 = (in >> (n+2)) & 0x1;
   uint8_t Q3 = (in >> (2*n+3)) & 0x1;
   uint8_t Q4 = (in >> (2*n+4)) & 0x1;
   uint8_t Q5 = (in >> (3*n+5)) & 0x1;
   uint8_t Q6 = (in >> (3*n+6)) & 0x1;
   uint8_t mmask = (1 << n) - 1;
   uint8_t m0 = (in >> (0)) & mmask;
   uint8_t m1 = (in >> (n+3)) & mmask;
   uint8_t m2 = (in >> (2*n+5)) & mmask;

   uint8_t C;
   uint8_t q2, q1, q0;
   if (CAT_BITS_4(Q6, Q5, Q2, Q1) == 0x3) {
      q2 = CAT_BITS_3(Q0, Q4 & ~Q0, Q3 & ~Q0);
      q1 = 4;
      q0 = 4;
   } else {
      if (CAT_BITS_2(Q2, Q1) == 0x3) {
         q2 = 4;
         C = CAT_BITS_5(Q4, Q3, 0x1 & ~Q6, 0x1 & ~Q5, Q0);
      } else {
         q2 = CAT_BITS_2(Q6, Q5);
         C = CAT_BITS_5(Q4, Q3, Q2, Q1, Q0);
      }
      if ((C & 0x7) == 0x5) {
         q1 = 4;
         q0 = (C >> 3) & 0x3;
      } else {
         q1 = (C >> 3) & 0x3;
         q0 = C & 0x7;
      }
   }
   out[0] = (q0 << n) | m0;
   out[1] = (q1 << n) | m1;
   out[2] = (q2 << n) | m2;
}


struct uint8x4_t
{
   uint8_t v[4];

   uint8x4_t() { }

   uint8x4_t(int a, int b, int c, int d)
   {
      assert(0 <= a && a <= 255);
      assert(0 <= b && b <= 255);
      assert(0 <= c && c <= 255);
      assert(0 <= d && d <= 255);
      v[0] = a;
      v[1] = b;
      v[2] = c;
      v[3] = d;
   }

   static uint8x4_t clamped(int a, int b, int c, int d)
   {
      uint8x4_t r;
      r.v[0] = MAX2(0, MIN2(255, a));
      r.v[1] = MAX2(0, MIN2(255, b));
      r.v[2] = MAX2(0, MIN2(255, c));
      r.v[3] = MAX2(0, MIN2(255, d));
      return r;
   }
};

static uint8x4_t blue_contract(int r, int g, int b, int a)
{
   return uint8x4_t((r+b) >> 1, (g+b) >> 1, b, a);
}

static uint8x4_t blue_contract_clamped(int r, int g, int b, int a)
{
   return uint8x4_t::clamped((r+b) >> 1, (g+b) >> 1, b, a);
}

static void bit_transfer_signed(int &a, int &b)
{
   b >>= 1;
   b |= a & 0x80;
   a >>= 1;
   a &= 0x3f;
   if (a & 0x20)
      a -= 0x40;
}

static uint32_t hash52(uint32_t p)
{
   p ^= p >> 15;
   p -= p << 17;
   p += p << 7;
   p += p << 4;
   p ^= p >> 5;
   p += p << 16;
   p ^= p >> 7;
   p ^= p >> 3;
   p ^= p << 6;
   p ^= p >> 17;
   return p;
}

static int select_partition(int seed, int x, int y, int z, int partitioncount,
                            int small_block)
{
   if (small_block) {
      x <<= 1;
      y <<= 1;
      z <<= 1;
   }
   seed += (partitioncount - 1) * 1024;
   uint32_t rnum = hash52(seed);
   uint8_t seed1 = rnum & 0xF;
   uint8_t seed2 = (rnum >> 4) & 0xF;
   uint8_t seed3 = (rnum >> 8) & 0xF;
   uint8_t seed4 = (rnum >> 12) & 0xF;
   uint8_t seed5 = (rnum >> 16) & 0xF;
   uint8_t seed6 = (rnum >> 20) & 0xF;
   uint8_t seed7 = (rnum >> 24) & 0xF;
   uint8_t seed8 = (rnum >> 28) & 0xF;
   uint8_t seed9 = (rnum >> 18) & 0xF;
   uint8_t seed10 = (rnum >> 22) & 0xF;
   uint8_t seed11 = (rnum >> 26) & 0xF;
   uint8_t seed12 = ((rnum >> 30) | (rnum << 2)) & 0xF;

   seed1 *= seed1;
   seed2 *= seed2;
   seed3 *= seed3;
   seed4 *= seed4;
   seed5 *= seed5;
   seed6 *= seed6;
   seed7 *= seed7;
   seed8 *= seed8;
   seed9 *= seed9;
   seed10 *= seed10;
   seed11 *= seed11;
   seed12 *= seed12;

   int sh1, sh2, sh3;
   if (seed & 1) {
      sh1 = (seed & 2 ? 4 : 5);
      sh2 = (partitioncount == 3 ? 6 : 5);
   } else {
      sh1 = (partitioncount == 3 ? 6 : 5);
      sh2 = (seed & 2 ? 4 : 5);
   }
   sh3 = (seed & 0x10) ? sh1 : sh2;

   seed1 >>= sh1;
   seed2 >>= sh2;
   seed3 >>= sh1;
   seed4 >>= sh2;
   seed5 >>= sh1;
   seed6 >>= sh2;
   seed7 >>= sh1;
   seed8 >>= sh2;
   seed9 >>= sh3;
   seed10 >>= sh3;
   seed11 >>= sh3;
   seed12 >>= sh3;

   int a = seed1 * x + seed2 * y + seed11 * z + (rnum >> 14);
   int b = seed3 * x + seed4 * y + seed12 * z + (rnum >> 10);
   int c = seed5 * x + seed6 * y + seed9 * z + (rnum >> 6);
   int d = seed7 * x + seed8 * y + seed10 * z + (rnum >> 2);

   a &= 0x3F;
   b &= 0x3F;
   c &= 0x3F;
   d &= 0x3F;

   if (partitioncount < 4)
      d = 0;
   if (partitioncount < 3)
      c = 0;

   if (a >= b && a >= c && a >= d)
      return 0;
   else if (b >= c && b >= d)
      return 1;
   else if (c >= d)
      return 2;
   else
      return 3;
}


struct InputBitVector
{
   uint32_t data[4];

   void printf_bits(int offset, int count, const char *fmt = "", ...)
   {
      char out[129];
      memset(out, '.', 128);
      out[128] = '\0';
      int idx = offset;
      for (int i = 0; i < count; ++i) {
         out[127 - idx] = ((data[idx >> 5] >> (idx & 31)) & 1) ? '1' : '0';
         ++idx;
      }
      printf("%s ", out);
      va_list ap;
      va_start(ap, fmt);
      vprintf(fmt, ap);
      va_end(ap);
      printf("\n");
   }

   uint32_t get_bits(int offset, int count)
   {
      assert(count >= 0 && count < 32);

      uint32_t out = 0;
      if (offset < 32)
         out |= data[0] >> offset;

      if (0 < offset && offset <= 32)
         out |= data[1] << (32 - offset);
      if (32 < offset && offset < 64)
         out |= data[1] >> (offset - 32);

      if (32 < offset && offset <= 64)
         out |= data[2] << (64 - offset);
      if (64 < offset && offset < 96)
         out |= data[2] >> (offset - 64);

      if (64 < offset && offset <= 96)
         out |= data[3] << (96 - offset);
      if (96 < offset && offset < 128)
         out |= data[3] >> (offset - 96);

      out &= (1 << count) - 1;
      return out;
   }

   uint64_t get_bits64(int offset, int count)
   {
      assert(count >= 0 && count < 64);

      uint64_t out = 0;
      if (offset < 32)
         out |= data[0] >> offset;

      if (offset <= 32)
         out |= (uint64_t)data[1] << (32 - offset);
      if (32 < offset && offset < 64)
         out |= data[1] >> (offset - 32);

      if (0 < offset && offset <= 64)
         out |= (uint64_t)data[2] << (64 - offset);
      if (64 < offset && offset < 96)
         out |= data[2] >> (offset - 64);

      if (32 < offset && offset <= 96)
         out |= (uint64_t)data[3] << (96 - offset);
      if (96 < offset && offset < 128)
         out |= data[3] >> (offset - 96);

      out &= ((uint64_t)1 << count) - 1;
      return out;
   }

   uint32_t get_bits_rev(int offset, int count)
   {
      assert(offset >= count);
      uint32_t tmp = get_bits(offset - count, count);
      uint32_t out = 0;
      for (int i = 0; i < count; ++i)
         out |= ((tmp >> i) & 1) << (count - 1 - i);
      return out;
   }
};

struct OutputBitVector
{
   uint32_t data[4];
   int offset;

   OutputBitVector()
      : offset(0)
   {
      memset(data, 0, sizeof(data));
   }

   void append(uint32_t value, int size)
   {
      if (VERBOSE_WRITE)
         printf("append offset=%d size=%d values=0x%x\n", offset, size, value);

      assert(offset + size <= 128);

      assert(size <= 32);
      if (size < 32)
         assert((value >> size) == 0);

      while (size) {
         int c = MIN2(size, 32 - (offset & 31));
         data[offset >> 5] |= (value << (offset & 31));
         offset += c;
         size -= c;
         value >>= c;
      }
   }

   void append64(uint64_t value, int size)
   {
      if (VERBOSE_WRITE)
         printf("append offset=%d size=%d values=0x%llx\n", offset, size, (unsigned long long)value);

      assert(offset + size <= 128);

      assert(size <= 64);
      if (size < 64)
         assert((value >> size) == 0);

      while (size) {
         int c = MIN2(size, 32 - (offset & 31));
         data[offset >> 5] |= (value << (offset & 31));
         offset += c;
         size -= c;
         value >>= c;
      }
   }

   void append(OutputBitVector &v, int size)
   {
      if (VERBOSE_WRITE)
         printf("append vector offset=%d size=%d\n", offset, size);

      assert(offset + size <= 128);
      int i = 0;
      while (size >= 32) {
         append(v.data[i++], 32);
         size -= 32;
      }
      if (size > 0)
         append(v.data[i] & ((1 << size) - 1), size);
   }

   void append_end(OutputBitVector &v, int size)
   {
      for (int i = 0; i < size; ++i)
         data[(127 - i) >> 5] |= ((v.data[i >> 5] >> (i & 31)) & 1) << ((127 - i) & 31);
   }

   /* Insert the given number of '1' bits. (We could use 0s instead, but 1s are
    * more likely to flush out bugs where we accidentally read undefined bits.)
    */
   void skip(int size)
   {
      if (VERBOSE_WRITE)
         printf("skip offset=%d size=%d\n", offset, size);

      assert(offset + size <= 128);
      while (size >= 32) {
         append(0xffffffff, 32);
         size -= 32;
      }
      if (size > 0)
         append(0xffffffff >> (32 - size), size);
   }
};


class Decoder
{
public:
   Decoder(int block_w, int block_h, int block_d, bool srgb, bool output_unorm8)
      : block_w(block_w), block_h(block_h), block_d(block_d), srgb(srgb),
        output_unorm8(output_unorm8) {}

   decode_error::type decode(const uint8_t *in, uint16_t *output) const;

   int block_w, block_h, block_d;
   bool srgb, output_unorm8;
};

struct Block
{
   bool is_error;
   bool bogus_colour_endpoints;
   bool bogus_weights;

   int high_prec;
   int dual_plane;
   int colour_component_selector;
   int wt_range;
   int wt_w, wt_h, wt_d;
   int num_parts;
   int partition_index;

   bool is_void_extent;
   int void_extent_d;
   int void_extent_min_s;
   int void_extent_max_s;
   int void_extent_min_t;
   int void_extent_max_t;
   uint16_t void_extent_colour_r;
   uint16_t void_extent_colour_g;
   uint16_t void_extent_colour_b;
   uint16_t void_extent_colour_a;

   bool is_multi_cem;
   int num_extra_cem_bits;
   int colour_endpoint_data_offset;
   int extra_cem_bits;
   int cem_base_class;
   int cems[4];

   int num_cem_values;

   /* Calculated by unpack_weights(): */
   uint8_t weights_quant[64 + 4]; /* max 64 values, plus padding for overflows in trit parsing */

   /* Calculated by unquantise_weights(): */
   uint8_t weights[64 + 18]; /* max 64 values, plus padding for the infill interpolation */

   /* Calculated by unpack_colour_endpoints(): */
   uint8_t colour_endpoints_quant[18 + 4]; /* max 18 values, plus padding for overflows in trit parsing */

   /* Calculated by unquantise_colour_endpoints(): */
   uint8_t colour_endpoints[18];

   /* Calculated by calculate_from_weights(): */
   int wt_trits;
   int wt_quints;
   int wt_bits;
   int wt_max;
   int num_weights;
   int weight_bits;

   /* Calculated by calculate_remaining_bits(): */
   int remaining_bits;

   /* Calculated by calculate_colour_endpoints_size(): */
   int colour_endpoint_bits;
   int ce_max;
   int ce_trits;
   int ce_quints;
   int ce_bits;

   /* Calculated by compute_infill_weights(); */
   uint8_t infill_weights[2][216]; /* large enough for 6x6x6 */

   /* Calculated by decode_colour_endpoints(); */
   uint8x4_t endpoints_decoded[2][4];

   void calculate_from_weights();
   void calculate_remaining_bits();
   decode_error::type calculate_colour_endpoints_size();

   void unquantise_weights();
   void unquantise_colour_endpoints();

   decode_error::type decode(const Decoder &decoder, InputBitVector in);

   decode_error::type decode_block_mode(InputBitVector in);
   decode_error::type decode_void_extent(InputBitVector in);
   void decode_cem(InputBitVector in);
   void unpack_colour_endpoints(InputBitVector in);
   void decode_colour_endpoints();
   void unpack_weights(InputBitVector in);
   void compute_infill_weights(int block_w, int block_h, int block_d);

   void write_decoded(const Decoder &decoder, uint16_t *output);
};


decode_error::type Decoder::decode(const uint8_t *in, uint16_t *output) const
{
   Block blk;
   InputBitVector in_vec;
   memcpy(&in_vec.data, in, 16);
   decode_error::type err = blk.decode(*this, in_vec);
   if (err == decode_error::ok) {
      blk.write_decoded(*this, output);
   } else {
      /* Fill output with the error colour */
      for (int i = 0; i < block_w * block_h * block_d; ++i) {
         if (output_unorm8) {
            output[i*4+0] = 0xff;
            output[i*4+1] = 0;
            output[i*4+2] = 0xff;
            output[i*4+3] = 0xff;
         } else {
            assert(!srgb); /* srgb must use unorm8 */

            output[i*4+0] = FP16_ONE;
            output[i*4+1] = FP16_ZERO;
            output[i*4+2] = FP16_ONE;
            output[i*4+3] = FP16_ONE;
         }
      }
   }
   return err;
}


decode_error::type Block::decode_void_extent(InputBitVector block)
{
   /* TODO: 3D */

   is_void_extent = true;
   void_extent_d = block.get_bits(9, 1);
   void_extent_min_s = block.get_bits(12, 13);
   void_extent_max_s = block.get_bits(25, 13);
   void_extent_min_t = block.get_bits(38, 13);
   void_extent_max_t = block.get_bits(51, 13);
   void_extent_colour_r = block.get_bits(64, 16);
   void_extent_colour_g = block.get_bits(80, 16);
   void_extent_colour_b = block.get_bits(96, 16);
   void_extent_colour_a = block.get_bits(112, 16);

   /* TODO: maybe we should do something useful with the extent coordinates? */

   if (void_extent_d) {
      return decode_error::unsupported_hdr_void_extent;
   }

   if (void_extent_min_s == 0x1fff && void_extent_max_s == 0x1fff
       && void_extent_min_t == 0x1fff && void_extent_max_t == 0x1fff) {

      /* No extents */

   } else {

      /* Check for illegal encoding */
      if (void_extent_min_s >= void_extent_max_s || void_extent_min_t >= void_extent_max_t) {
         return decode_error::invalid_range_in_void_extent;
      }
   }

   return decode_error::ok;
}

decode_error::type Block::decode_block_mode(InputBitVector in)
{
   dual_plane = in.get_bits(10, 1);
   high_prec = in.get_bits(9, 1);

   if (in.get_bits(0, 2) != 0x0) {
      wt_range = (in.get_bits(0, 2) << 1) | in.get_bits(4, 1);
      int a = in.get_bits(5, 2);
      int b = in.get_bits(7, 2);
      switch (in.get_bits(2, 2)) {
      case 0x0:
         if (VERBOSE_DECODE)
            in.printf_bits(0, 11, "DHBBAAR00RR");
         wt_w = b + 4;
         wt_h = a + 2;
         break;
      case 0x1:
         if (VERBOSE_DECODE)
            in.printf_bits(0, 11, "DHBBAAR01RR");
         wt_w = b + 8;
         wt_h = a + 2;
         break;
      case 0x2:
         if (VERBOSE_DECODE)
            in.printf_bits(0, 11, "DHBBAAR10RR");
         wt_w = a + 2;
         wt_h = b + 8;
         break;
      case 0x3:
         if ((b & 0x2) == 0) {
            if (VERBOSE_DECODE)
               in.printf_bits(0, 11, "DH0BAAR11RR");
            wt_w = a + 2;
            wt_h = b + 6;
         } else {
            if (VERBOSE_DECODE)
               in.printf_bits(0, 11, "DH1BAAR11RR");
            wt_w = (b & 0x1) + 2;
            wt_h = a + 2;
         }
         break;
      }
   } else {
      if (in.get_bits(6, 3) == 0x7) {
         if (in.get_bits(0, 9) == 0x1fc) {
            if (VERBOSE_DECODE)
               in.printf_bits(0, 11, "xx111111100 (void extent)");
            return decode_void_extent(in);
         } else {
            if (VERBOSE_DECODE)
               in.printf_bits(0, 11, "xx111xxxx00");
            return decode_error::reserved_block_mode_1;
         }
      }
      if (in.get_bits(0, 4) == 0x0) {
         if (VERBOSE_DECODE)
            in.printf_bits(0, 11, "xxxxxxx0000");
         return decode_error::reserved_block_mode_2;
      }

      wt_range = in.get_bits(1, 3) | in.get_bits(4, 1);
      int a = in.get_bits(5, 2);
      int b;

      switch (in.get_bits(7, 2)) {
      case 0x0:
         if (VERBOSE_DECODE)
            in.printf_bits(0, 11, "DH00AARRR00");
         wt_w = 12;
         wt_h = a + 2;
         break;
      case 0x1:
         if (VERBOSE_DECODE)
            in.printf_bits(0, 11, "DH01AARRR00");
         wt_w = a + 2;
         wt_h = 12;
         break;
      case 0x3:
         if (in.get_bits(5, 1) == 0) {
            if (VERBOSE_DECODE)
               in.printf_bits(0, 11, "DH1100RRR00");
            wt_w = 6;
            wt_h = 10;
         } else {
            if (VERBOSE_DECODE)
               in.printf_bits(0, 11, "DH1101RRR00");
            wt_w = 10;
            wt_h = 6;
         }
         break;
      case 0x2:
         if (VERBOSE_DECODE)
            in.printf_bits(0, 11, "BB10AARRR00");
         b = in.get_bits(9, 2);
         wt_w = a + 6;
         wt_h = b + 6;
         dual_plane = 0;
         high_prec = 0;
         break;
      }
   }
   return decode_error::ok;
}

void Block::decode_cem(InputBitVector in)
{
   cems[0] = cems[1] = cems[2] = cems[3] = -1;

   num_extra_cem_bits = 0;
   extra_cem_bits = 0;

   if (num_parts > 1) {

      partition_index = in.get_bits(13, 10);
      if (VERBOSE_DECODE)
         in.printf_bits(13, 10, "partition ID (%d)", partition_index);

      uint32_t cem = in.get_bits(23, 6);

      if ((cem & 0x3) == 0x0) {
         cem >>= 2;
         cem_base_class = cem >> 2;
         is_multi_cem = false;

         for (int i = 0; i < num_parts; ++i)
            cems[i] = cem;

         if (VERBOSE_DECODE)
            in.printf_bits(23, 6, "CEM (single, %d)", cem);
      } else {

         cem_base_class = (cem & 0x3) - 1;
         is_multi_cem = true;

         if (VERBOSE_DECODE)
            in.printf_bits(23, 6, "CEM (multi, base class %d)", cem_base_class);

         int offset = 128 - weight_bits;

         if (num_parts == 2) {
            if (VERBOSE_DECODE) {
               in.printf_bits(25, 4, "M0M0 C1 C0");
               in.printf_bits(offset - 2, 2, "M1M1");
            }

            uint32_t c0 = in.get_bits(25, 1);
            uint32_t c1 = in.get_bits(26, 1);

            extra_cem_bits = c0 + c1;

            num_extra_cem_bits = 2;

            uint32_t m0 = in.get_bits(27, 2);
            uint32_t m1 = in.get_bits(offset - 2, 2);

            cems[0] = ((cem_base_class + c0) << 2) | m0;
            cems[1] = ((cem_base_class + c1) << 2) | m1;

         } else if (num_parts == 3) {
            if (VERBOSE_DECODE) {
               in.printf_bits(25, 4, "M0 C2 C1 C0");
               in.printf_bits(offset - 5, 5, "M2M2 M1M1 M0");
            }

            uint32_t c0 = in.get_bits(25, 1);
            uint32_t c1 = in.get_bits(26, 1);
            uint32_t c2 = in.get_bits(27, 1);

            extra_cem_bits = c0 + c1 + c2;

            num_extra_cem_bits = 5;

            uint32_t m0 = in.get_bits(28, 1) | (in.get_bits(128 - weight_bits - 5, 1) << 1);
            uint32_t m1 = in.get_bits(offset - 4, 2);
            uint32_t m2 = in.get_bits(offset - 2, 2);

            cems[0] = ((cem_base_class + c0) << 2) | m0;
            cems[1] = ((cem_base_class + c1) << 2) | m1;
            cems[2] = ((cem_base_class + c2) << 2) | m2;

         } else if (num_parts == 4) {
            if (VERBOSE_DECODE) {
               in.printf_bits(25, 4, "C3 C2 C1 C0");
               in.printf_bits(offset - 8, 8, "M3M3 M2M2 M1M1 M0M0");
            }

            uint32_t c0 = in.get_bits(25, 1);
            uint32_t c1 = in.get_bits(26, 1);
            uint32_t c2 = in.get_bits(27, 1);
            uint32_t c3 = in.get_bits(28, 1);

            extra_cem_bits = c0 + c1 + c2 + c3;

            num_extra_cem_bits = 8;

            uint32_t m0 = in.get_bits(offset - 8, 2);
            uint32_t m1 = in.get_bits(offset - 6, 2);
            uint32_t m2 = in.get_bits(offset - 4, 2);
            uint32_t m3 = in.get_bits(offset - 2, 2);

            cems[0] = ((cem_base_class + c0) << 2) | m0;
            cems[1] = ((cem_base_class + c1) << 2) | m1;
            cems[2] = ((cem_base_class + c2) << 2) | m2;
            cems[3] = ((cem_base_class + c3) << 2) | m3;
         } else {
            unreachable("");
         }
      }

      colour_endpoint_data_offset = 29;

   } else {
      uint32_t cem = in.get_bits(13, 4);

      cem_base_class = cem >> 2;
      is_multi_cem = false;

      cems[0] = cem;

      partition_index = -1;

      if (VERBOSE_DECODE)
         in.printf_bits(13, 4, "CEM = %d (class %d)", cem, cem_base_class);

      colour_endpoint_data_offset = 17;
   }
}

void Block::unpack_colour_endpoints(InputBitVector in)
{
   if (ce_trits) {
      int offset = colour_endpoint_data_offset;
      int bits_left = colour_endpoint_bits;
      for (int i = 0; i < num_cem_values; i += 5) {
         int bits_to_read = MIN2(bits_left, 8 + ce_bits * 5);
         /* If ce_trits then ce_bits <= 6, so bits_to_read <= 38 and we have to use uint64_t */
         uint64_t raw = in.get_bits64(offset, bits_to_read);
         unpack_trit_block(ce_bits, raw, &colour_endpoints_quant[i]);

         if (VERBOSE_DECODE)
            in.printf_bits(offset, bits_to_read,
                           "trits [%d,%d,%d,%d,%d]",
                           colour_endpoints_quant[i+0], colour_endpoints_quant[i+1],
                  colour_endpoints_quant[i+2], colour_endpoints_quant[i+3],
                  colour_endpoints_quant[i+4]);

         offset += 8 + ce_bits * 5;
         bits_left -= 8 + ce_bits * 5;
      }
   } else if (ce_quints) {
      int offset = colour_endpoint_data_offset;
      int bits_left = colour_endpoint_bits;
      for (int i = 0; i < num_cem_values; i += 3) {
         int bits_to_read = MIN2(bits_left, 7 + ce_bits * 3);
         /* If ce_quints then ce_bits <= 5, so bits_to_read <= 22 and we can use uint32_t */
         uint32_t raw = in.get_bits(offset, bits_to_read);
         unpack_quint_block(ce_bits, raw, &colour_endpoints_quant[i]);

         if (VERBOSE_DECODE)
            in.printf_bits(offset, bits_to_read,
                           "quints [%d,%d,%d]",
                           colour_endpoints_quant[i], colour_endpoints_quant[i+1], colour_endpoints_quant[i+2]);

         offset += 7 + ce_bits * 3;
         bits_left -= 7 + ce_bits * 3;
      }
   } else {
      assert((colour_endpoint_bits % ce_bits) == 0);
      int offset = colour_endpoint_data_offset;
      for (int i = 0; i < num_cem_values; i++) {
         colour_endpoints_quant[i] = in.get_bits(offset, ce_bits);

         if (VERBOSE_DECODE)
            in.printf_bits(offset, ce_bits, "bits [%d]", colour_endpoints_quant[i]);

         offset += ce_bits;
      }
   }
}

void Block::decode_colour_endpoints()
{
   int cem_values_idx = 0;
   for (int part = 0; part < num_parts; ++part) {
      uint8_t *v = &colour_endpoints[cem_values_idx];
      int v0 = v[0];
      int v1 = v[1];
      int v2 = v[2];
      int v3 = v[3];
      int v4 = v[4];
      int v5 = v[5];
      int v6 = v[6];
      int v7 = v[7];
      cem_values_idx += ((cems[part] >> 2) + 1) * 2;

      uint8x4_t e0, e1;
      int s0, s1, L0, L1;

      switch (cems[part])
      {
      case 0:
         e0 = uint8x4_t(v0, v0, v0, 0xff);
         e1 = uint8x4_t(v1, v1, v1, 0xff);
         break;
      case 1:
         L0 = (v0 >> 2) | (v1 & 0xc0);
         L1 = L0 + (v1 & 0x3f);
         if (L1 > 0xff)
            L1 = 0xff;
         e0 = uint8x4_t(L0, L0, L0, 0xff);
         e1 = uint8x4_t(L1, L1, L1, 0xff);
         break;
      case 4:
         e0 = uint8x4_t(v0, v0, v0, v2);
         e1 = uint8x4_t(v1, v1, v1, v3);
         break;
      case 5:
         bit_transfer_signed(v1, v0);
         bit_transfer_signed(v3, v2);
         e0 = uint8x4_t(v0, v0, v0, v2);
         e1 = uint8x4_t::clamped(v0+v1, v0+v1, v0+v1, v2+v3);
         break;
      case 6:
         e0 = uint8x4_t(v0*v3 >> 8, v1*v3 >> 8, v2*v3 >> 8, 0xff);
         e1 = uint8x4_t(v0, v1, v2, 0xff);
         break;
      case 8:
         s0 = v0 + v2 + v4;
         s1 = v1 + v3 + v5;
         if (s1 >= s0) {
            e0 = uint8x4_t(v0, v2, v4, 0xff);
            e1 = uint8x4_t(v1, v3, v5, 0xff);
         } else {
            e0 = blue_contract(v1, v3, v5, 0xff);
            e1 = blue_contract(v0, v2, v4, 0xff);
         }
         break;
      case 9:
         bit_transfer_signed(v1, v0);
         bit_transfer_signed(v3, v2);
         bit_transfer_signed(v5, v4);
         if (v1 + v3 + v5 >= 0) {
            e0 = uint8x4_t(v0, v2, v4, 0xff);
            e1 = uint8x4_t::clamped(v0+v1, v2+v3, v4+v5, 0xff);
         } else {
            e0 = blue_contract_clamped(v0+v1, v2+v3, v4+v5, 0xff);
            e1 = blue_contract(v0, v2, v4, 0xff);
         }
         break;
      case 10:
         e0 = uint8x4_t(v0*v3 >> 8, v1*v3 >> 8, v2*v3 >> 8, v4);
         e1 = uint8x4_t(v0, v1, v2, v5);
         break;
      case 12:
         s0 = v0 + v2 + v4;
         s1 = v1 + v3 + v5;
         if (s1 >= s0) {
            e0 = uint8x4_t(v0, v2, v4, v6);
            e1 = uint8x4_t(v1, v3, v5, v7);
         } else {
            e0 = blue_contract(v1, v3, v5, v7);
            e1 = blue_contract(v0, v2, v4, v6);
         }
         break;
      case 13:
         bit_transfer_signed(v1, v0);
         bit_transfer_signed(v3, v2);
         bit_transfer_signed(v5, v4);
         bit_transfer_signed(v7, v6);
         if (v1 + v3 + v5 >= 0) {
            e0 = uint8x4_t(v0, v2, v4, v6);
            e1 = uint8x4_t::clamped(v0+v1, v2+v3, v4+v5, v6+v7);
         } else {
            e0 = blue_contract_clamped(v0+v1, v2+v3, v4+v5, v6+v7);
            e1 = blue_contract(v0, v2, v4, v6);
         }
         break;
      default:
         /* HDR endpoints not supported; return error colour */
         e0 = uint8x4_t(255, 0, 255, 255);
         e1 = uint8x4_t(255, 0, 255, 255);
         break;
      }

      endpoints_decoded[0][part] = e0;
      endpoints_decoded[1][part] = e1;

      if (VERBOSE_DECODE) {
         printf("cems[%d]=%d v=[", part, cems[part]);
         for (int i = 0; i < (cems[part] >> 2) + 1; ++i) {
            if (i)
               printf(", ");
            printf("%3d", v[i]);
         }
         printf("] e0=[%3d,%4d,%4d,%4d] e1=[%3d,%4d,%4d,%4d]\n",
                e0.v[0], e0.v[1], e0.v[2], e0.v[3],
               e1.v[0], e1.v[1], e1.v[2], e1.v[3]);
      }
   }
}

void Block::unpack_weights(InputBitVector in)
{
   if (wt_trits) {
      int offset = 128;
      int bits_left = weight_bits;
      for (int i = 0; i < num_weights; i += 5) {
         int bits_to_read = MIN2(bits_left, 8 + 5*wt_bits);
         /* If wt_trits then wt_bits <= 3, so bits_to_read <= 23 and we can use uint32_t */
         uint32_t raw = in.get_bits_rev(offset, bits_to_read);
         unpack_trit_block(wt_bits, raw, &weights_quant[i]);

         if (VERBOSE_DECODE)
            in.printf_bits(offset - bits_to_read, bits_to_read, "weight trits [%d,%d,%d,%d,%d]",
                           weights_quant[i+0], weights_quant[i+1],
                  weights_quant[i+2], weights_quant[i+3],
                  weights_quant[i+4]);

         offset -= 8 + wt_bits * 5;
         bits_left -= 8 + wt_bits * 5;
      }

   } else if (wt_quints) {

      int offset = 128;
      int bits_left = weight_bits;
      for (int i = 0; i < num_weights; i += 3) {
         int bits_to_read = MIN2(bits_left, 7 + 3*wt_bits);
         /* If wt_quints then wt_bits <= 2, so bits_to_read <= 13 and we can use uint32_t */
         uint32_t raw = in.get_bits_rev(offset, bits_to_read);
         unpack_quint_block(wt_bits, raw, &weights_quant[i]);

         if (VERBOSE_DECODE)
            in.printf_bits(offset - bits_to_read, bits_to_read, "weight quints [%d,%d,%d]",
                           weights_quant[i], weights_quant[i+1], weights_quant[i+2]);

         offset -= 7 + wt_bits * 3;
         bits_left -= 7 + wt_bits * 3;
      }

   } else {
      int offset = 128;
      assert((weight_bits % wt_bits) == 0);
      for (int i = 0; i < num_weights; ++i) {
         weights_quant[i] = in.get_bits_rev(offset, wt_bits);

         if (VERBOSE_DECODE)
            in.printf_bits(offset - wt_bits, wt_bits, "weight bits [%d]", weights_quant[i]);

         offset -= wt_bits;
      }
   }
}

void Block::unquantise_weights()
{
   assert(num_weights <= (int)ARRAY_SIZE(weights_quant));
   assert(num_weights <= (int)ARRAY_SIZE(weights));

   memset(weights, 0, sizeof(weights));

   for (int i = 0; i < num_weights; ++i) {

      uint8_t v = weights_quant[i];
      uint8_t w;

      if (wt_trits) {

         if (wt_bits == 0) {
            w = v * 32;
         } else {
            uint8_t A, B, C, D;
            A = (v & 0x1) ? 0x7F : 0x00;
            switch (wt_bits) {
            case 1:
               B = 0;
               C = 50;
               D = v >> 1;
               break;
            case 2:
               B = (v & 0x2) ? 0x45 : 0x00;
               C = 23;
               D = v >> 2;
               break;
            case 3:
               B = ((v & 0x6) >> 1) | ((v & 0x6) << 4);
               C = 11;
               D = v >> 3;
               break;
            default:
               unreachable("");
            }
            uint16_t T = D * C + B;
            T = T ^ A;
            T = (A & 0x20) | (T >> 2);
            assert(T < 64);
            if (T > 32)
               T++;
            w = T;
         }

      } else if (wt_quints) {

         if (wt_bits == 0) {
            w = v * 16;
         } else {
            uint8_t A, B, C, D;
            A = (v & 0x1) ? 0x7F : 0x00;
            switch (wt_bits) {
            case 1:
               B = 0;
               C = 28;
               D = v >> 1;
               break;
            case 2:
               B = (v & 0x2) ? 0x42 : 0x00;
               C = 13;
               D = v >> 2;
               break;
            default:
               unreachable("");
            }
            uint16_t T = D * C + B;
            T = T ^ A;
            T = (A & 0x20) | (T >> 2);
            assert(T < 64);
            if (T > 32)
               T++;
            w = T;
         }
         weights[i] = w;

      } else {

         switch (wt_bits) {
         case 1: w = v ? 0x3F : 0x00; break;
         case 2: w = v | (v << 2) | (v << 4); break;
         case 3: w = v | (v << 3); break;
         case 4: w = (v >> 2) | (v << 2); break;
         case 5: w = (v >> 4) | (v << 1); break;
         default: unreachable("");
         }
         assert(w < 64);
         if (w > 32)
            w++;
      }
      weights[i] = w;
   }
}

void Block::compute_infill_weights(int block_w, int block_h, int block_d)
{
   int Ds = block_w <= 1 ? 0 : (1024 + block_w / 2) / (block_w - 1);
   int Dt = block_h <= 1 ? 0 : (1024 + block_h / 2) / (block_h - 1);
   int Dr = block_d <= 1 ? 0 : (1024 + block_d / 2) / (block_d - 1);
   for (int r = 0; r < block_d; ++r) {
      for (int t = 0; t < block_h; ++t) {
         for (int s = 0; s < block_w; ++s) {
            int cs = Ds * s;
            int ct = Dt * t;
            int cr = Dr * r;
            int gs = (cs * (wt_w - 1) + 32) >> 6;
            int gt = (ct * (wt_h - 1) + 32) >> 6;
            int gr = (cr * (wt_d - 1) + 32) >> 6;
            assert(gs >= 0 && gs <= 176);
            assert(gt >= 0 && gt <= 176);
            assert(gr >= 0 && gr <= 176);
            int js = gs >> 4;
            int fs = gs & 0xf;
            int jt = gt >> 4;
            int ft = gt & 0xf;
            int jr = gr >> 4;
            int fr = gr & 0xf;

            /* TODO: 3D */
            (void)jr;
            (void)fr;

            int w11 = (fs * ft + 8) >> 4;
            int w10 = ft - w11;
            int w01 = fs - w11;
            int w00 = 16 - fs - ft + w11;

            if (dual_plane) {
               int p00, p01, p10, p11, i0, i1;
               int v0 = js + jt * wt_w;
               p00 = weights[(v0) * 2];
               p01 = weights[(v0 + 1) * 2];
               p10 = weights[(v0 + wt_w) * 2];
               p11 = weights[(v0 + wt_w + 1) * 2];
               i0 = (p00*w00 + p01*w01 + p10*w10 + p11*w11 + 8) >> 4;
               p00 = weights[(v0) * 2 + 1];
               p01 = weights[(v0 + 1) * 2 + 1];
               p10 = weights[(v0 + wt_w) * 2 + 1];
               p11 = weights[(v0 + wt_w + 1) * 2 + 1];
               assert((v0 + wt_w + 1) * 2 + 1 < (int)ARRAY_SIZE(weights));
               i1 = (p00*w00 + p01*w01 + p10*w10 + p11*w11 + 8) >> 4;
               assert(0 <= i0 && i0 <= 64);
               infill_weights[0][s + t*block_w + r*block_w*block_h] = i0;
               infill_weights[1][s + t*block_w + r*block_w*block_h] = i1;
            } else {
               int p00, p01, p10, p11, i;
               int v0 = js + jt * wt_w;
               p00 = weights[v0];
               p01 = weights[v0 + 1];
               p10 = weights[v0 + wt_w];
               p11 = weights[v0 + wt_w + 1];
               assert(v0 + wt_w + 1 < (int)ARRAY_SIZE(weights));
               i = (p00*w00 + p01*w01 + p10*w10 + p11*w11 + 8) >> 4;
               assert(0 <= i && i <= 64);
               infill_weights[0][s + t*block_w + r*block_w*block_h] = i;
            }
         }
      }
   }
}

void Block::unquantise_colour_endpoints()
{
   assert(num_cem_values <= (int)ARRAY_SIZE(colour_endpoints_quant));
   assert(num_cem_values <= (int)ARRAY_SIZE(colour_endpoints));

   for (int i = 0; i < num_cem_values; ++i) {
      uint8_t v = colour_endpoints_quant[i];

      if (ce_trits) {
         uint16_t A, B, C, D;
         uint16_t t;
         A = (v & 0x1) ? 0x1FF : 0x000;
         switch (ce_bits) {
         case 1:
            B = 0;
            C = 204;
            D = v >> 1;
            break;
         case 2:
            B = (v & 0x2) ? 0x116 : 0x000;
            C = 93;
            D = v >> 2;
            break;
         case 3:
            t = ((v >> 1) & 0x3);
            B = t | (t << 2) | (t << 7);
            C = 44;
            D = v >> 3;
            break;
         case 4:
            t = ((v >> 1) & 0x7);
            B = t | (t << 6);
            C = 22;
            D = v >> 4;
            break;
         case 5:
            t = ((v >> 1) & 0xF);
            B = (t >> 2) | (t << 5);
            C = 11;
            D = v >> 5;
            break;
         case 6:
            B = ((v & 0x3E) << 3) | ((v >> 5) & 0x1);
            C = 5;
            D = v >> 6;
            break;
         default:
            unreachable("");
         }
         uint16_t T = D * C + B;
         T = T ^ A;
         T = (A & 0x80) | (T >> 2);
         assert(T < 256);
         colour_endpoints[i] = T;
      } else if (ce_quints) {
         uint16_t A, B, C, D;
         uint16_t t;
         A = (v & 0x1) ? 0x1FF : 0x000;
         switch (ce_bits) {
         case 1:
            B = 0;
            C = 113;
            D = v >> 1;
            break;
         case 2:
            B = (v & 0x2) ? 0x10C : 0x000;
            C = 54;
            D = v >> 2;
            break;
         case 3:
            t = ((v >> 1) & 0x3);
            B = (t >> 1) | (t << 1) | (t << 7);
            C = 26;
            D = v >> 3;
            break;
         case 4:
            t = ((v >> 1) & 0x7);
            B = (t >> 1) | (t << 6);
            C = 13;
            D = v >> 4;
            break;
         case 5:
            t = ((v >> 1) & 0xF);
            B = (t >> 4) | (t << 5);
            C = 6;
            D = v >> 5;
            break;
         default:
            unreachable("");
         }
         uint16_t T = D * C + B;
         T = T ^ A;
         T = (A & 0x80) | (T >> 2);
         assert(T < 256);
         colour_endpoints[i] = T;
      } else {
         switch (ce_bits) {
         case 1: v = v ? 0xFF : 0x00; break;
         case 2: v = (v << 6) | (v << 4) | (v << 2) | v; break;
         case 3: v = (v << 5) | (v << 2) | (v >> 1); break;
         case 4: v = (v << 4) | v; break;
         case 5: v = (v << 3) | (v >> 2); break;
         case 6: v = (v << 2) | (v >> 4); break;
         case 7: v = (v << 1) | (v >> 6); break;
         case 8: break;
         default: unreachable("");
         }
         colour_endpoints[i] = v;
      }
   }
}

decode_error::type Block::decode(const Decoder &decoder, InputBitVector in)
{
   decode_error::type err;

   is_error = false;
   bogus_colour_endpoints = false;
   bogus_weights = false;
   is_void_extent = false;

   wt_d = 1;
   /* TODO: 3D */

   /* TODO: test for all the illegal encodings */

   if (VERBOSE_DECODE)
      in.printf_bits(0, 128);

   err = decode_block_mode(in);
   if (err != decode_error::ok)
      return err;

   if (is_void_extent)
      return decode_error::ok;

   /* TODO: 3D */

   calculate_from_weights();

   if (VERBOSE_DECODE)
      printf("weights_grid=%dx%dx%d dual_plane=%d num_weights=%d high_prec=%d r=%d range=0..%d (%dt %dq %db) weight_bits=%d\n",
             wt_w, wt_h, wt_d, dual_plane, num_weights, high_prec, wt_range, wt_max, wt_trits, wt_quints, wt_bits, weight_bits);

   if (wt_w > decoder.block_w || wt_h > decoder.block_h || wt_d > decoder.block_d)
      return decode_error::weight_grid_exceeds_block_size;

   num_parts = in.get_bits(11, 2) + 1;

   if (VERBOSE_DECODE)
      in.printf_bits(11, 2, "partitions = %d", num_parts);

   if (dual_plane && num_parts > 3)
      return decode_error::dual_plane_and_too_many_partitions;

   decode_cem(in);

   if (VERBOSE_DECODE)
      printf("cem=[%d,%d,%d,%d] base_cem_class=%d\n", cems[0], cems[1], cems[2], cems[3], cem_base_class);

   int num_cem_pairs = (cem_base_class + 1) * num_parts + extra_cem_bits;
   num_cem_values = num_cem_pairs * 2;

   calculate_remaining_bits();
   err = calculate_colour_endpoints_size();
   if (err != decode_error::ok)
      return err;

   if (VERBOSE_DECODE)
      in.printf_bits(colour_endpoint_data_offset, colour_endpoint_bits,
                     "endpoint data (%d bits, %d vals, %dt %dq %db)",
                     colour_endpoint_bits, num_cem_values, ce_trits, ce_quints, ce_bits);

   unpack_colour_endpoints(in);

   if (VERBOSE_DECODE) {
      printf("cem values raw =[");
      for (int i = 0; i < num_cem_values; i++) {
         if (i)
            printf(", ");
         printf("%3d", colour_endpoints_quant[i]);
      }
      printf("]\n");
   }

   if (num_cem_values > 18)
      return decode_error::invalid_colour_endpoints_count;

   unquantise_colour_endpoints();

   if (VERBOSE_DECODE) {
      printf("cem values norm=[");
      for (int i = 0; i < num_cem_values; i++) {
         if (i)
            printf(", ");
         printf("%3d", colour_endpoints[i]);
      }
      printf("]\n");
   }

   decode_colour_endpoints();

   if (dual_plane) {
      int ccs_offset = 128 - weight_bits - num_extra_cem_bits - 2;
      colour_component_selector = in.get_bits(ccs_offset, 2);

      if (VERBOSE_DECODE)
         in.printf_bits(ccs_offset, 2, "colour component selector = %d", colour_component_selector);
   } else {
      colour_component_selector = 0;
   }


   if (VERBOSE_DECODE)
      in.printf_bits(128 - weight_bits, weight_bits, "weights (%d bits)", weight_bits);

   if (num_weights > 64)
      return decode_error::invalid_num_weights;

   if (weight_bits < 24 || weight_bits > 96)
      return decode_error::invalid_weight_bits;

   unpack_weights(in);

   unquantise_weights();

   if (VERBOSE_DECODE) {
      printf("weights=[");
      for (int i = 0; i < num_weights; ++i) {
         if (i)
            printf(", ");
         printf("%d", weights[i]);
      }
      printf("]\n");

      for (int plane = 0; plane <= dual_plane; ++plane) {
         printf("weights (plane %d):\n", plane);
         int i = 0;
         (void)i;

         for (int r = 0; r < wt_d; ++r) {
            for (int t = 0; t < wt_h; ++t) {
               for (int s = 0; s < wt_w; ++s) {
                  printf("%3d", weights[i++ * (1 + dual_plane) + plane]);
               }
               printf("\n");
            }
            if (r < wt_d - 1)
               printf("\n");
         }
      }
   }

   compute_infill_weights(decoder.block_w, decoder.block_h, decoder.block_d);

   if (VERBOSE_DECODE) {
      for (int plane = 0; plane <= dual_plane; ++plane) {
         printf("infilled weights (plane %d):\n", plane);
         int i = 0;
         (void)i;

         for (int r = 0; r < decoder.block_d; ++r) {
            for (int t = 0; t < decoder.block_h; ++t) {
               for (int s = 0; s < decoder.block_w; ++s) {
                  printf("%3d", infill_weights[plane][i++]);
               }
               printf("\n");
            }
            if (r < decoder.block_d - 1)
               printf("\n");
         }
      }
   }
   if (VERBOSE_DECODE)
      printf("\n");

   return decode_error::ok;
}

void Block::write_decoded(const Decoder &decoder, uint16_t *output)
{
   /* sRGB can only be stored as unorm8. */
   assert(!decoder.srgb || decoder.output_unorm8);

   if (is_void_extent) {
      for (int idx = 0; idx < decoder.block_w*decoder.block_h*decoder.block_d; ++idx) {
         if (decoder.output_unorm8) {
            output[idx*4+0] = void_extent_colour_r >> 8;
            output[idx*4+1] = void_extent_colour_g >> 8;
            output[idx*4+2] = void_extent_colour_b >> 8;
            output[idx*4+3] = void_extent_colour_a >> 8;
         } else {
            /* Store the color as FP16. */
            output[idx*4+0] = _mesa_uint16_div_64k_to_half(void_extent_colour_r);
            output[idx*4+1] = _mesa_uint16_div_64k_to_half(void_extent_colour_g);
            output[idx*4+2] = _mesa_uint16_div_64k_to_half(void_extent_colour_b);
            output[idx*4+3] = _mesa_uint16_div_64k_to_half(void_extent_colour_a);
         }
      }
      return;
   }

   int small_block = (decoder.block_w * decoder.block_h * decoder.block_d) < 31;

   int idx = 0;
   for (int z = 0; z < decoder.block_d; ++z) {
      for (int y = 0; y < decoder.block_h; ++y) {
         for (int x = 0; x < decoder.block_w; ++x) {

            int partition;
            if (num_parts > 1) {
               partition = select_partition(partition_index, x, y, z, num_parts, small_block);
               assert(partition < num_parts);
            } else {
               partition = 0;
            }

            /* TODO: HDR */

            uint8x4_t e0 = endpoints_decoded[0][partition];
            uint8x4_t e1 = endpoints_decoded[1][partition];
            uint16_t c0[4], c1[4];

            /* Expand to 16 bits. */
            if (decoder.srgb) {
               c0[0] = (uint16_t)((e0.v[0] << 8) | 0x80);
               c0[1] = (uint16_t)((e0.v[1] << 8) | 0x80);
               c0[2] = (uint16_t)((e0.v[2] << 8) | 0x80);
               c0[3] = (uint16_t)((e0.v[3] << 8) | 0x80);

               c1[0] = (uint16_t)((e1.v[0] << 8) | 0x80);
               c1[1] = (uint16_t)((e1.v[1] << 8) | 0x80);
               c1[2] = (uint16_t)((e1.v[2] << 8) | 0x80);
               c1[3] = (uint16_t)((e1.v[3] << 8) | 0x80);
            } else {
               c0[0] = (uint16_t)((e0.v[0] << 8) | e0.v[0]);
               c0[1] = (uint16_t)((e0.v[1] << 8) | e0.v[1]);
               c0[2] = (uint16_t)((e0.v[2] << 8) | e0.v[2]);
               c0[3] = (uint16_t)((e0.v[3] << 8) | e0.v[3]);

               c1[0] = (uint16_t)((e1.v[0] << 8) | e1.v[0]);
               c1[1] = (uint16_t)((e1.v[1] << 8) | e1.v[1]);
               c1[2] = (uint16_t)((e1.v[2] << 8) | e1.v[2]);
               c1[3] = (uint16_t)((e1.v[3] << 8) | e1.v[3]);
            }

            int w[4];
            if (dual_plane) {
               int w0 = infill_weights[0][idx];
               int w1 = infill_weights[1][idx];
               w[0] = w[1] = w[2] = w[3] = w0;
               w[colour_component_selector] = w1;
            } else {
               int w0 = infill_weights[0][idx];
               w[0] = w[1] = w[2] = w[3] = w0;
            }

            /* Interpolate to produce UNORM16, applying weights. */
            uint16_t c[4] = {
               (uint16_t)((c0[0] * (64 - w[0]) + c1[0] * w[0] + 32) >> 6),
               (uint16_t)((c0[1] * (64 - w[1]) + c1[1] * w[1] + 32) >> 6),
               (uint16_t)((c0[2] * (64 - w[2]) + c1[2] * w[2] + 32) >> 6),
               (uint16_t)((c0[3] * (64 - w[3]) + c1[3] * w[3] + 32) >> 6),
            };

            if (decoder.output_unorm8) {
               output[idx*4+0] = c[0] >> 8;
               output[idx*4+1] = c[1] >> 8;
               output[idx*4+2] = c[2] >> 8;
               output[idx*4+3] = c[3] >> 8;
            } else {
               /* Store the color as FP16. */
               output[idx*4+0] = c[0] == 65535 ? FP16_ONE : _mesa_uint16_div_64k_to_half(c[0]);
               output[idx*4+1] = c[1] == 65535 ? FP16_ONE : _mesa_uint16_div_64k_to_half(c[1]);
               output[idx*4+2] = c[2] == 65535 ? FP16_ONE : _mesa_uint16_div_64k_to_half(c[2]);
               output[idx*4+3] = c[3] == 65535 ? FP16_ONE : _mesa_uint16_div_64k_to_half(c[3]);
            }

            idx++;
         }
      }
   }
}

void Block::calculate_from_weights()
{
   wt_trits = 0;
   wt_quints = 0;
   wt_bits = 0;
   switch (high_prec) {
   case 0:
      switch (wt_range) {
      case 0x2: wt_max = 1; wt_bits = 1; break;
      case 0x3: wt_max = 2; wt_trits = 1; break;
      case 0x4: wt_max = 3; wt_bits = 2; break;
      case 0x5: wt_max = 4; wt_quints = 1; break;
      case 0x6: wt_max = 5; wt_trits = 1; wt_bits = 1; break;
      case 0x7: wt_max = 7; wt_bits = 3; break;
      default: abort();
      }
      break;
   case 1:
      switch (wt_range) {
      case 0x2: wt_max = 9; wt_quints = 1; wt_bits = 1; break;
      case 0x3: wt_max = 11; wt_trits = 1; wt_bits = 2; break;
      case 0x4: wt_max = 15; wt_bits = 4; break;
      case 0x5: wt_max = 19; wt_quints = 1; wt_bits = 2; break;
      case 0x6: wt_max = 23; wt_trits = 1; wt_bits = 3; break;
      case 0x7: wt_max = 31; wt_bits = 5; break;
      default: abort();
      }
      break;
   }

   assert(wt_trits || wt_quints || wt_bits);

   num_weights = wt_w * wt_h * wt_d;

   if (dual_plane)
      num_weights *= 2;

   weight_bits =
         (num_weights * 8 * wt_trits + 4) / 5
         + (num_weights * 7 * wt_quints + 2) / 3
         +  num_weights * wt_bits;
}

void Block::calculate_remaining_bits()
{
   int config_bits;
   if (num_parts > 1) {
      if (!is_multi_cem)
         config_bits = 29;
      else
         config_bits = 25 + 3 * num_parts;
   } else {
      config_bits = 17;
   }

   if (dual_plane)
      config_bits += 2;

   remaining_bits = 128 - config_bits - weight_bits;
}

decode_error::type Block::calculate_colour_endpoints_size()
{
   /* Specified as illegal */
   if (remaining_bits < (13 * num_cem_values + 4) / 5) {
      colour_endpoint_bits = ce_max = ce_trits = ce_quints = ce_bits = 0;
      return decode_error::invalid_colour_endpoints_size;
   }

   /* Find the largest cem_ranges that fits within remaining_bits */
   for (int i = ARRAY_SIZE(cem_ranges)-1; i >= 0; --i) {
      int cem_bits;
      cem_bits = (num_cem_values * 8 * cem_ranges[i].t + 4) / 5
                 + (num_cem_values * 7 * cem_ranges[i].q + 2) / 3
                 +  num_cem_values * cem_ranges[i].b;

      if (cem_bits <= remaining_bits)
      {
         colour_endpoint_bits = cem_bits;
         ce_max = cem_ranges[i].max;
         ce_trits = cem_ranges[i].t;
         ce_quints = cem_ranges[i].q;
         ce_bits = cem_ranges[i].b;
         return decode_error::ok;
      }
   }

   assert(0);
   return decode_error::invalid_colour_endpoints_size;
}

/**
 * Decode ASTC 2D LDR texture data.
 *
 * \param src_width in pixels
 * \param src_height in pixels
 * \param dst_stride in bytes
 */
extern "C" void
_mesa_unpack_astc_2d_ldr(uint8_t *dst_row,
                         unsigned dst_stride,
                         const uint8_t *src_row,
                         unsigned src_stride,
                         unsigned src_width,
                         unsigned src_height,
                         mesa_format format)
{
   assert(_mesa_is_format_astc_2d(format));
   bool srgb = _mesa_is_format_srgb(format);

   unsigned blk_w, blk_h;
   _mesa_get_format_block_size(format, &blk_w, &blk_h);

   const unsigned block_size = 16;
   unsigned x_blocks = (src_width + blk_w - 1) / blk_w;
   unsigned y_blocks = (src_height + blk_h - 1) / blk_h;

   Decoder dec(blk_w, blk_h, 1, srgb, true);

   for (unsigned y = 0; y < y_blocks; ++y) {
      for (unsigned x = 0; x < x_blocks; ++x) {
         /* Same size as the largest block. */
         uint16_t block_out[12 * 12 * 4];

         dec.decode(src_row + x * block_size, block_out);

         /* This can be smaller with NPOT dimensions. */
         unsigned dst_blk_w = MIN2(blk_w, src_width  - x*blk_w);
         unsigned dst_blk_h = MIN2(blk_h, src_height - y*blk_h);

         for (unsigned sub_y = 0; sub_y < dst_blk_h; ++sub_y) {
            for (unsigned sub_x = 0; sub_x < dst_blk_w; ++sub_x) {
               uint8_t *dst = dst_row + sub_y * dst_stride +
                              (x * blk_w + sub_x) * 4;
               const uint16_t *src = &block_out[(sub_y * blk_w + sub_x) * 4];

               dst[0] = src[0];
               dst[1] = src[1];
               dst[2] = src[2];
               dst[3] = src[3];
            }
         }
      }
      src_row += src_stride;
      dst_row += dst_stride * blk_h;
   }
}
