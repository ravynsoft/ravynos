#version 320 es
precision highp float;
precision highp int;
precision highp usamplerBuffer;
precision highp usampler2D;
precision highp image2D;
precision highp uimage2D;

/* Copyright (c) 2020-2022 Hans-Kristian Arntzen
 * Copyright (c) 2022 Intel Corporation
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
 * CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#ifdef VULKAN

precision highp utextureBuffer;
precision highp utexture2DArray;
precision highp uimage2DArray;
precision highp uimage3D;
precision highp utexture3D;

#extension GL_EXT_samplerless_texture_functions : require
layout(local_size_x_id = 0, local_size_y_id = 1, local_size_z = 4) in;

layout(set = 0, binding = 0) writeonly uniform uimage2DArray OutputImage2Darray;
layout(set = 0, binding = 0) writeonly uniform uimage3D OutputImage3D;
layout(set = 0, binding = 1) uniform utexture2DArray PayloadInput2Darray;
layout(set = 0, binding = 1) uniform utexture3D PayloadInput3D;
layout(set = 0, binding = 2) uniform utextureBuffer LUTRemainingBitsToEndpointQuantizer;
layout(set = 0, binding = 3) uniform utextureBuffer LUTEndpointUnquantize;
layout(set = 0, binding = 4) uniform utextureBuffer LUTWeightQuantizer;
layout(set = 0, binding = 5) uniform utextureBuffer LUTWeightUnquantize;
layout(set = 0, binding = 6) uniform utextureBuffer LUTTritQuintDecode;
layout(set = 0, binding = 7) uniform utextureBuffer LUTPartitionTable;

layout(constant_id = 2) const bool DECODE_8BIT = false;

layout(push_constant, std430) uniform pc {
   ivec2 texel_blk_start;
   ivec2 texel_end;
   bool is_3Dimage;
};

#else /* VULKAN */

layout(local_size_x = %u, local_size_y = %u, local_size_z = 4) in;

#define utextureBuffer usamplerBuffer
#define utexture2D usampler2D

layout(binding = 0) uniform utextureBuffer LUTRemainingBitsToEndpointQuantizer;
layout(binding = 1) uniform utextureBuffer LUTEndpointUnquantize;
layout(binding = 2) uniform utextureBuffer LUTWeightQuantizer;
layout(binding = 3) uniform utextureBuffer LUTWeightUnquantize;
layout(binding = 4) uniform utextureBuffer LUTTritQuintDecode;
layout(binding = 5) uniform utexture2D LUTPartitionTable;
layout(binding = 6) uniform utexture2D PayloadInput;

layout(rgba8ui, binding = 7) writeonly uniform uimage2D OutputImage;
const bool DECODE_8BIT = true;

#endif /* VULKAN */

const int MODE_LDR = 0;
const int MODE_HDR = 1;
const int MODE_HDR_LDR_ALPHA = 2;

const uvec4 error_color = uvec4(255, 0, 255, 255);

/* bitextract.h */
int extract_bits(uvec4 payload, int offset, int bits)
{
        int last_offset = offset + bits - 1;
        int result;

        if (bits <= 0)
                result = 0;
        else if ((last_offset >> 5) == (offset >> 5))
                result = int(bitfieldExtract(payload[offset >> 5], offset & 31, bits));
        else
        {
                int first_bits = 32 - (offset & 31);
                int result_first = int(bitfieldExtract(payload[offset >> 5], offset & 31, first_bits));
                int result_second = int(bitfieldExtract(payload[(offset >> 5) + 1], 0, bits - first_bits));
                result = result_first | (result_second << first_bits);
        }
        return result;
}

/* bitextract.h */
int extract_bits_sign(uvec4 payload, int offset, int bits)
{
        int last_offset = offset + bits - 1;
        int result;

        if (bits <= 0)
                result = 0;
        else if ((last_offset >> 5) == (offset >> 5))
                result = bitfieldExtract(int(payload[offset >> 5]), offset & 31, bits);
        else
        {
                int first_bits = 32 - (offset & 31);
                int result_first = int(bitfieldExtract(payload[offset >> 5], offset & 31, first_bits));
                int result_second = bitfieldExtract(int(payload[(offset >> 5) + 1]), 0, bits - first_bits);
                result = result_first | (result_second << first_bits);
        }
        return result;
}

/* bitextract.h */
int extract_bits_reverse(uvec4 payload, int offset, int bits)
{
        int last_offset = offset + bits - 1;
        int result;

        if (bits <= 0)
                result = 0;
        else if ((last_offset >> 5) == (offset >> 5))
                result = int(bitfieldReverse(bitfieldExtract(payload[offset >> 5], offset & 31, bits)) >> (32 - bits));
        else
        {
                int first_bits = 32 - (offset & 31);
                uint result_first = bitfieldExtract(payload[offset >> 5], offset & 31, first_bits);
                uint result_second = bitfieldExtract(payload[(offset >> 5) + 1], 0, bits - first_bits);
                result = int(bitfieldReverse(result_first | (result_second << first_bits)) >> (32 - bits));
        }
        return result;
}

void swap(inout int a, inout int b)
{
    int tmp = a;
    a = b;
    b = tmp;
}

ivec4 build_coord()
{
    ivec2 payload_coord = ivec2(gl_WorkGroupID.xy) * 2;
    payload_coord.x += int(gl_LocalInvocationID.z) & 1;
    payload_coord.y += (int(gl_LocalInvocationID.z) >> 1) & 1;
#ifdef VULKAN
    payload_coord += texel_blk_start;
#endif /* VULKAN */
    ivec2 coord = payload_coord * ivec2(gl_WorkGroupSize.xy);
    coord += ivec2(gl_LocalInvocationID.xy);
    return ivec4(coord, payload_coord);
}

ivec4 interpolate_endpoint(ivec4 ep0, ivec4 ep1, ivec4 weight, int decode_mode)
{
    if (decode_mode == MODE_HDR)
    {
        ep0 <<= 4;
        ep1 <<= 4;
    }
    else if (decode_mode == MODE_HDR_LDR_ALPHA)
    {
        ep0.rgb <<= 4;
        ep1.rgb <<= 4;
        ep0.a *= 0x101;
        ep1.a *= 0x101;
    }
    else if (DECODE_8BIT)
    {
        // This isn't quite right in all cases.
        // In normal ASTC with sRGB, the alpha channel is supposed to
        // be decoded as FP16,
        // even when color components are SRGB 8-bit (?!?!?!?!).
        // This is correct if decode_unorm8 mode is used though,
        // for sanity, we're going to assume unorm8 decoding mode
        // is implied when using sRGB.
        ep0 = (ep0 << 8) | ivec4(0x80);
        ep1 = (ep1 << 8) | ivec4(0x80);
    }
    else
    {
        ep0 *= 0x101;
        ep1 *= 0x101;
    }

    ivec4 color = (ep0 * (64 - weight) + ep1 * weight + 32) >> 6;
    return color;
}

bvec4 bvec_or(bvec4 a, bvec4 b)
{
    return bvec4(ivec4(a) | ivec4(b));
}

uint round_down_quantize_fp16(int color)
{
    // ASTC has a very peculiar way of converting the decoded result to FP16.
    // 0xffff -> 1.0, and for everything else we get roundDownQuantizeFP16(vec4(c) / vec4(0x10000)).
    int msb = findMSB(color);
    int shamt = msb;
    int m = ((color << 10) >> shamt) & 0x3ff;
    int e = msb - 1;
    uint decoded = color == 0xffff ? 0x3c00u : uint(e < 1 ? (color << 8) : (m | (e << 10)));
    return decoded;
}

uvec4 round_down_quantize_fp16(ivec4 color)
{
    // ASTC has a very peculiar way of converting the decoded result to FP16.
    // 0xffff -> 1.0, and for everything else we get roundDownQuantizeFP16(vec4(c) / vec4(0x10000)).
    ivec4 msb = findMSB(color);
    ivec4 shamt = msb;
    ivec4 m = ((color << 10) >> shamt) & 0x3ff;
    ivec4 e = msb - 1;
    uvec4 decoded = uvec4(m | (e << 10));
    uvec4 denorm_decode = uvec4(color << 8);
    decoded = mix(decoded, uvec4(denorm_decode), lessThan(e, ivec4(1)));
    decoded = mix(decoded, uvec4(0x3c00), equal(color, ivec4(0xffff)));
    return decoded;
}

uvec4 decode_fp16(ivec4 color, int decode_mode)
{
    if (decode_mode != MODE_LDR)
    {
        // Interpret the value as FP16, but with some extra fixups along the way to make the interpolation more
        // logarithmic (apparently). From spec:
        ivec4 e = color >> 11;
        ivec4 m = color & 0x7ff;
        ivec4 mt = 4 * m - 512;
        mt = mix(mt, ivec4(3 * m), lessThan(m, ivec4(512)));
        mt = mix(mt, ivec4(5 * m - 2048), greaterThanEqual(m, ivec4(1536)));

        ivec4 decoded = (e << 10) + (mt >> 3);
        // +Inf or NaN are decoded to 0x7bff (max finite value).
        decoded = mix(decoded, ivec4(0x7bff), bvec_or(greaterThan(decoded & 0x7fff, ivec4(0x7c00)), equal(decoded, ivec4(0x7c00))));

        if (decode_mode == MODE_HDR_LDR_ALPHA)
            decoded.a = int(round_down_quantize_fp16(color.a));

        return uvec4(decoded);
    }
    else
    {
        return round_down_quantize_fp16(color);
    }
}

struct BlockMode
{
    ivec2 weight_grid_size;
    int weight_mode_index;
    int num_partitions;
    int seed;
    int cem;
    int config_bits;
    int primary_config_bits;
    bool dual_plane;
    bool void_extent;
};

bool decode_error = false;

BlockMode decode_block_mode(uvec4 payload)
{
    BlockMode mode;
    mode.void_extent = (payload.x & 0x1ffu) == 0x1fcu;
    if (mode.void_extent)
        return mode;

    mode.dual_plane = (payload.x & (1u << 10u)) != 0u;

    uint higher = (payload.x >> 2u) & 3u;
    uint lower = payload.x & 3u;

    if (lower != 0u)
    {
        mode.weight_mode_index = int((payload.x >> 4u) & 1u);
        mode.weight_mode_index |= int((payload.x << 1u) & 6u);
        mode.weight_mode_index |= int((payload.x >> 6u) & 8u);

        if (higher < 2u)
        {
            mode.weight_grid_size.x = int(bitfieldExtract(payload.x, 7, 2) + 4u + 4u * higher);
            mode.weight_grid_size.y = int(bitfieldExtract(payload.x, 5, 2) + 2u);
        }
        else if (higher == 2u)
        {
            mode.weight_grid_size.x = int(bitfieldExtract(payload.x, 5, 2) + 2u);
            mode.weight_grid_size.y = int(bitfieldExtract(payload.x, 7, 2) + 8u);
        }
        else
        {
            if ((payload.x & (1u << 8u)) != 0u)
            {
                mode.weight_grid_size.x = int(bitfieldExtract(payload.x, 7, 1) + 2u);
                mode.weight_grid_size.y = int(bitfieldExtract(payload.x, 5, 2) + 2u);
            }
            else
            {
                mode.weight_grid_size.x = int(bitfieldExtract(payload.x, 5, 2) + 2u);
                mode.weight_grid_size.y = int(bitfieldExtract(payload.x, 7, 1) + 6u);
            }
        }
    }
    else
    {
        int p3 = int(bitfieldExtract(payload.x, 9, 1));
        int hi = int(bitfieldExtract(payload.x, 7, 2));
        int lo = int(bitfieldExtract(payload.x, 5, 2));
        if (hi == 0)
        {
            mode.weight_grid_size.x = 12;
            mode.weight_grid_size.y = lo + 2;
        }
        else if (hi == 1)
        {
            mode.weight_grid_size.x = lo + 2;
            mode.weight_grid_size.y = 12;
        }
        else if (hi == 2)
        {
            mode.dual_plane = false;
            p3 = 0;
            mode.weight_grid_size.x = lo + 6;
            mode.weight_grid_size.y = int(bitfieldExtract(payload.x, 9, 2) + 6u);
        }
        else
        {
            if (lo == 0)
                mode.weight_grid_size = ivec2(6, 10);
            else if (lo == 1)
                mode.weight_grid_size = ivec2(10, 6);
            else
                decode_error = true;
        }

        int p0 = int(bitfieldExtract(payload.x, 4, 1));
        int p1 = int(bitfieldExtract(payload.x, 2, 1));
        int p2 = int(bitfieldExtract(payload.x, 3, 1));
        mode.weight_mode_index = p0 + (p1 << 1) + (p2 << 2) + (p3 << 3);
    }

    // 11 bits for block mode.
    // 2 bits for partition select
    // If partitions > 1:
    //   4 bits CEM selector
    //   If dual_plane:
    //     2 bits of CCS
    // else:
    //   10 for partition seed
    //   2 bits for CEM main selector
    //   If CEM[1:0] = 00:
    //     4 bits for CEM extra selector if all same type.
    //   else:
    //     (1 + 2) * num_partitions if different types.
    //     First 4 bits are encoded next to CEM[1:0], otherwise, packed before weights.
    //   If dual_plane:
    //     2 bits of CCS before extra CEM bits.
    const int CONFIG_BITS_BLOCK = 11;
    const int CONFIG_BITS_PARTITION_MODE = 2;
    const int CONFIG_BITS_SEED = 10;
    const int CONFIG_BITS_PRIMARY_MULTI_CEM = 2;
    const int CONFIG_BITS_CEM = 4;
    const int CONFIG_BITS_EXTRA_CEM_PER_PARTITION = 3;
    const int CONFIG_BITS_CCS = 2;

    mode.num_partitions = int(bitfieldExtract(payload.x, CONFIG_BITS_BLOCK, CONFIG_BITS_PARTITION_MODE)) + 1;

    if (mode.num_partitions > 1)
    {
        mode.seed = int(bitfieldExtract(payload.x, CONFIG_BITS_BLOCK + CONFIG_BITS_PARTITION_MODE, CONFIG_BITS_SEED));
        mode.cem = int(bitfieldExtract(payload.x, CONFIG_BITS_BLOCK + CONFIG_BITS_PARTITION_MODE + CONFIG_BITS_SEED,
                                       CONFIG_BITS_PRIMARY_MULTI_CEM + CONFIG_BITS_CEM));
    }
    else
        mode.cem = int(bitfieldExtract(payload.x, CONFIG_BITS_BLOCK + CONFIG_BITS_PARTITION_MODE, CONFIG_BITS_CEM));

    int config_bits;
    if (mode.num_partitions > 1)
    {
        bool single_cem = (mode.cem & 3) == 0;
        if (single_cem)
        {
            config_bits = CONFIG_BITS_BLOCK + CONFIG_BITS_PARTITION_MODE +
                          CONFIG_BITS_SEED + CONFIG_BITS_PRIMARY_MULTI_CEM + CONFIG_BITS_CEM;
        }
        else
        {
            config_bits = CONFIG_BITS_BLOCK + CONFIG_BITS_PARTITION_MODE +
                          CONFIG_BITS_SEED + CONFIG_BITS_PRIMARY_MULTI_CEM +
                          CONFIG_BITS_EXTRA_CEM_PER_PARTITION * mode.num_partitions;
        }
    }
    else
    {
        config_bits = CONFIG_BITS_BLOCK + CONFIG_BITS_PARTITION_MODE + CONFIG_BITS_CEM;
    }

    // Other config bits are packed before the weights.
    int primary_config_bits;
    if (mode.num_partitions > 1)
    {
        primary_config_bits = CONFIG_BITS_BLOCK + CONFIG_BITS_PARTITION_MODE + CONFIG_BITS_SEED +
                              CONFIG_BITS_PRIMARY_MULTI_CEM + CONFIG_BITS_CEM;
    }
    else
        primary_config_bits = config_bits;

    if (mode.dual_plane)
        config_bits += CONFIG_BITS_CCS;

    // This is not allowed.
    if (any(greaterThan(mode.weight_grid_size, ivec2(gl_WorkGroupSize.xy))))
        decode_error = true;
    if (mode.dual_plane && mode.num_partitions > 3)
        decode_error = true;

    mode.config_bits = config_bits;
    mode.primary_config_bits = primary_config_bits;
    return mode;
}

int idiv3_floor(int v)
{
    return (v * 0x5556) >> 16;
}

int idiv3_ceil(int v)
{
    return idiv3_floor(v + 2);
}

int idiv5_floor(int v)
{
    return (v * 0x3334) >> 16;
}

int idiv5_ceil(int v)
{
    return idiv5_floor(v + 4);
}

uvec4 build_bitmask(int bits)
{
    ivec4 num_bits = ivec4(bits, bits - 32, bits - 64, bits - 96);
    uvec4 mask = uvec4(1) << clamp(num_bits, ivec4(0), ivec4(31));
    mask--;
    mask = mix(mask, uvec4(0xffffffffu), greaterThanEqual(uvec4(bits), uvec4(32, 64, 96, 128)));
    return mask;
}

int decode_integer_sequence(uvec4 payload, int start_bit, int index, ivec3 quant)
{
    int ret;
    if (quant.y != 0)
    {
        // Trit-decoding.
        int block = idiv5_floor(index);
        int offset = index - block * 5;
        start_bit += block * (5 * quant.x + 8);

        int t0_t1_offset = start_bit + (quant.x * 1 + 0);
        int t2_t3_offset = start_bit + (quant.x * 2 + 2);
        int t4_offset    = start_bit + (quant.x * 3 + 4);
        int t5_t6_offset = start_bit + (quant.x * 4 + 5);
        int t7_offset    = start_bit + (quant.x * 5 + 7);

        int t = (extract_bits(payload, t0_t1_offset, 2) << 0) |
                (extract_bits(payload, t2_t3_offset, 2) << 2) |
                (extract_bits(payload, t4_offset, 1) << 4) |
                (extract_bits(payload, t5_t6_offset, 2) << 5) |
                (extract_bits(payload, t7_offset, 1) << 7);

        t = int(texelFetch(LUTTritQuintDecode, t).x);
        t = (t >> (3 * offset)) & 7;

        int m_offset = offset * quant.x;
        m_offset += idiv5_ceil(offset * 8);

        if (quant.x != 0)
        {
            int m = extract_bits(payload, m_offset + start_bit, quant.x);
            ret = (t << quant.x) | m;
        }
        else
            ret = t;
    }
    else if (quant.z != 0)
    {
        // Quint-decoding
        int block = idiv3_floor(index);
        int offset = index - block * 3;
        start_bit += block * (3 * quant.x + 7);

        int q0_q1_q2_offset = start_bit + (quant.x * 1 + 0);
        int q3_q4_offset    = start_bit + (quant.x * 2 + 3);
        int q5_q6_offset    = start_bit + (quant.x * 3 + 5);

        int q = (extract_bits(payload, q0_q1_q2_offset, 3) << 0) |
                (extract_bits(payload, q3_q4_offset, 2) << 3) |
                (extract_bits(payload, q5_q6_offset, 2) << 5);

        q = int(texelFetch(LUTTritQuintDecode, 256 + q).x);
        q = (q >> (3 * offset)) & 7;

        int m_offset = offset * quant.x;
        m_offset += idiv3_ceil(offset * 7);

        if (quant.x != 0)
        {
            int m = extract_bits(payload, m_offset + start_bit, quant.x);
            ret = (q << quant.x) | m;
        }
        else
            ret = q;
    }
    else
    {
        int bit = index * quant.x;
        ret = extract_bits(payload, start_bit + bit, quant.x);
    }
    return ret;
}

ivec2 normalize_coord(ivec2 pixel_coord)
{
    ivec2 D = ivec2((vec2((1024 + ivec2(gl_WorkGroupSize.xy >> 1u))) + 0.5) / vec2(gl_WorkGroupSize.xy - 1u));
    ivec2 c = D * pixel_coord;
    return c;
}

int decode_weight(uvec4 payload, int weight_index, ivec4 quant)
{
    int primary_weight = decode_integer_sequence(payload, 0, weight_index, quant.xyz);
    primary_weight = int(texelFetch(LUTWeightUnquantize, primary_weight + quant.w).x);
    return primary_weight;
}

int decode_weight_bilinear(uvec4 payload, ivec2 coord, int weight_resolution,
                           int stride, int offset, ivec2 fractional, ivec4 quant)
{
    int index = coord.y * weight_resolution + coord.x;
    int p00 = decode_weight(payload, stride * index + offset, quant);
    int p10, p01, p11;

    if (fractional.x != 0)
        p10 = decode_weight(payload, stride * (index + 1) + offset, quant);
    else
        p10 = p00;

    if (fractional.y != 0)
    {
        p01 = decode_weight(payload, stride * (index + weight_resolution) + offset, quant);
        if (fractional.x != 0)
            p11 = decode_weight(payload, stride * (index + weight_resolution + 1) + offset, quant);
        else
            p11 = p01;
    }
    else
    {
        p01 = p00;
        p11 = p10;
    }

    int w11 = (fractional.x * fractional.y + 8) >> 4;
    int w10 = fractional.x - w11;
    int w01 = fractional.y - w11;
    int w00 = 16 - fractional.x - fractional.y + w11;
    return (p00 * w00 + p10 * w10 + p01 * w01 + p11 * w11 + 8) >> 4;
}

ivec4 decode_weights(uvec4 payload, BlockMode mode, ivec2 normalized_pixel, out int weight_cost_bits)
{
    ivec4 quant = ivec4(texelFetch(LUTWeightQuantizer, mode.weight_mode_index));
    int num_weights = mode.weight_grid_size.x * mode.weight_grid_size.y;
    num_weights <<= int(mode.dual_plane);
    weight_cost_bits =
        quant.x * num_weights +
        idiv5_ceil(num_weights * 8 * quant.y) +
        idiv3_ceil(num_weights * 7 * quant.z);

    // Decoders must deal with error conditions and return the correct error color.
    if (weight_cost_bits < 24 || weight_cost_bits > 96 || num_weights > 64)
    {
        decode_error = true;
        return ivec4(0);
    }

    int ccs;
    if (mode.dual_plane)
    {
        int extra_cem_bits = 0;
        if ((mode.cem & 3) != 0)
            extra_cem_bits = max(mode.num_partitions * 3 - 4, 0);
        ccs = extract_bits(payload, 126 - weight_cost_bits - extra_cem_bits, 2);
    }

    payload = bitfieldReverse(payload);
    payload = payload.wzyx;
    payload &= build_bitmask(weight_cost_bits);

    // Scale the normalized coordinate to weight grid.
    ivec2 weight_pixel_fixed_point = (normalized_pixel * (mode.weight_grid_size - 1) + 32) >> 6;
    ivec2 weight_pixel = weight_pixel_fixed_point >> 4;
    ivec2 weight_pixel_fractional = weight_pixel_fixed_point & 0xf;

    ivec4 ret;
    int primary_weight = decode_weight_bilinear(payload, weight_pixel, mode.weight_grid_size.x,
                                                1 << int(mode.dual_plane), 0,
                                                weight_pixel_fractional, quant);
    if (mode.dual_plane)
    {
        int secondary_weight = decode_weight_bilinear(payload, weight_pixel, mode.weight_grid_size.x,
                                                      2, 1,
                                                      weight_pixel_fractional, quant);
        ret = mix(ivec4(primary_weight), ivec4(secondary_weight), equal(ivec4(ccs), ivec4(0, 1, 2, 3)));
    }
    else
        ret = ivec4(primary_weight);

    return ret;
}

void decode_endpoint_ldr_luma_direct(out ivec4 ep0, out ivec4 ep1,
        int v0, int v1)
{
    ep0 = ivec4(ivec3(v0), 0xff);
    ep1 = ivec4(ivec3(v1), 0xff);
}

void decode_endpoint_hdr_luma_direct(out ivec4 ep0, out ivec4 ep1,
        int v0, int v1)
{
    int y0, y1;
    if (v1 >= v0)
    {
        y0 = v0 << 4;
        y1 = v1 << 4;
    }
    else
    {
        y0 = (v1 << 4) + 8;
        y1 = (v0 << 4) - 8;
    }

    ep0 = ivec4(ivec3(y0), 0x780);
    ep1 = ivec4(ivec3(y1), 0x780);
}

void decode_endpoint_hdr_luma_direct_small_range(out ivec4 ep0, out ivec4 ep1,
        int v0, int v1)
{
    int y0, y1, d;

    if ((v0 & 0x80) != 0)
    {
        y0 = ((v1 & 0xe0) << 4) | ((v0 & 0x7f) << 2);
        d = (v1 & 0x1f) << 2;
    }
    else
    {
        y0 = ((v1 & 0xf0) << 4) | ((v0 & 0x7f) << 1);
        d = (v1 & 0x0f)  << 1;
    }

    y1 = min(y0 + d, 0xfff);

    ep0 = ivec4(ivec3(y0), 0x780);
    ep1 = ivec4(ivec3(y1), 0x780);
}

void decode_endpoint_ldr_luma_base_offset(out ivec4 ep0, out ivec4 ep1,
        int v0, int v1)
{
    int l0 = (v0 >> 2) | (v1 & 0xc0);
    int l1 = l0 + (v1 & 0x3f);
    l1 = min(l1, 0xff);
    ep0 = ivec4(ivec3(l0), 0xff);
    ep1 = ivec4(ivec3(l1), 0xff);
}

void decode_endpoint_ldr_luma_alpha_direct(out ivec4 ep0, out ivec4 ep1,
    int v0, int v1, int v2, int v3)
{
    ep0 = ivec4(ivec3(v0), v2);
    ep1 = ivec4(ivec3(v1), v3);
}

ivec4 blue_contract(int r, int g, int b, int a)
{
    ivec4 ret;
    ret.r = (r + b) >> 1;
    ret.g = (g + b) >> 1;
    ret.b = b;
    ret.a = a;
    return ret;
}

void bit_transfer_signed(inout int a, inout int b)
{
    b >>= 1;
    b |= a & 0x80;
    a >>= 1;
    a &= 0x3f;
    a = bitfieldExtract(a, 0, 6);
}

void decode_endpoint_ldr_luma_alpha_base_offset(out ivec4 ep0, out ivec4 ep1,
    int v0, int v1, int v2, int v3)
{
    bit_transfer_signed(v1, v0);
    bit_transfer_signed(v3, v2);
    int v0_v1 = clamp(v0 + v1, 0, 0xff);
    int v2_v3 = clamp(v2 + v3, 0, 0xff);
    v0 = clamp(v0, 0, 0xff);
    v2 = clamp(v2, 0, 0xff);
    ep0 = ivec4(ivec3(v0), v2);
    ep1 = ivec4(ivec3(v0_v1), v2_v3);
}

void decode_endpoint_ldr_rgb_base_scale(out ivec4 ep0, out ivec4 ep1,
        int v0, int v1, int v2, int v3)
{
    ep0 = ivec4((ivec3(v0, v1, v2) * v3) >> 8, 0xff);
    ep1 = ivec4(v0, v1, v2, 0xff);
}

void decode_endpoint_ldr_rgb_base_scale_two_a(out ivec4 ep0, out ivec4 ep1,
        int v0, int v1, int v2, int v3, int v4, int v5)
{
    ep0 = ivec4((ivec3(v0, v1, v2) * v3) >> 8, v4);
    ep1 = ivec4(v0, v1, v2, v5);
}

void decode_endpoint_ldr_rgb_direct(out ivec4 ep0, out ivec4 ep1,
        int v0, int v1, int v2, int v3, int v4, int v5)
{
    int s0 = v0 + v2 + v4;
    int s1 = v1 + v3 + v5;
    if (s1 >= s0)
    {
        ep0 = ivec4(v0, v2, v4, 0xff);
        ep1 = ivec4(v1, v3, v5, 0xff);
    }
    else
    {
        ep0 = blue_contract(v1, v3, v5, 0xff);
        ep1 = blue_contract(v0, v2, v4, 0xff);
    }
}

void decode_endpoint_hdr_rgb_scale(out ivec4 ep0, out ivec4 ep1,
    int v0, int v1, int v2, int v3)
{
    // Mind-numbing weird format, just copy from spec ...
    int mode_value = ((v0 & 0xc0) >> 6) | ((v1 & 0x80) >> 5) | ((v2 & 0x80) >> 4);
    int major_component;
    int mode;

    if ((mode_value & 0xc) != 0xc)
    {
        major_component = mode_value >> 2;
        mode = mode_value & 3;
    }
    else if (mode_value != 0xf)
    {
        major_component = mode_value & 3;
        mode = 4;
    }
    else
    {
        major_component = 0;
        mode = 5;
    }

    int red = v0 & 0x3f;
    int green = v1 & 0x1f;
    int blue = v2 & 0x1f;
    int scale = v3 & 0x1f;

    int x0 = (v1 >> 6) & 1;
    int x1 = (v1 >> 5) & 1;
    int x2 = (v2 >> 6) & 1;
    int x3 = (v2 >> 5) & 1;
    int x4 = (v3 >> 7) & 1;
    int x5 = (v3 >> 6) & 1;
    int x6 = (v3 >> 5) & 1;

    int ohm = 1 << mode;
    if ((ohm & 0x30) != 0) green |= x0 << 6;
    if ((ohm & 0x3a) != 0) green |= x1 << 5;
    if ((ohm & 0x30) != 0) blue |= x2 << 6;
    if ((ohm & 0x3a) != 0) blue |= x3 << 5;
    if ((ohm & 0x3d) != 0) scale |= x6 << 5;
    if ((ohm & 0x2d) != 0) scale |= x5 << 6;
    if ((ohm & 0x04) != 0) scale |= x4 << 7;
    if ((ohm & 0x3b) != 0) red |= x4 << 6;
    if ((ohm & 0x04) != 0) red |= x3 << 6;
    if ((ohm & 0x10) != 0) red |= x5 << 7;
    if ((ohm & 0x0f) != 0) red |= x2 << 7;
    if ((ohm & 0x05) != 0) red |= x1 << 8;
    if ((ohm & 0x0a) != 0) red |= x0 << 8;
    if ((ohm & 0x05) != 0) red |= x0 << 9;
    if ((ohm & 0x02) != 0) red |= x6 << 9;
    if ((ohm & 0x01) != 0) red |= x3 << 10;
    if ((ohm & 0x02) != 0) red |= x5 << 10;

    int shamt = max(mode, 1);
    red <<= shamt;
    green <<= shamt;
    blue <<= shamt;
    scale <<= shamt;

    if (mode != 5)
    {
        green = red - green;
        blue = red - blue;
    }

    if (major_component == 1)
        swap(red, green);
    else if (major_component == 2)
        swap(red, blue);

    ep1 = ivec4(clamp(ivec3(red, green, blue), ivec3(0), ivec3(0xfff)), 0x780);
    ep0 = ivec4(clamp(ivec3(red, green, blue) - scale, ivec3(0), ivec3(0xfff)), 0x780);
}

void decode_endpoint_hdr_rgb_direct(out ivec4 ep0, out ivec4 ep1,
        int v0, int v1, int v2, int v3, int v4, int v5)
{
    int major_component = ((v4 & 0x80) >> 7) | ((v5 & 0x80) >> 6);

    if (major_component == 3)
    {
        ep0 = ivec4(v0 << 4, v2 << 4, (v4 & 0x7f) << 5, 0x780);
        ep1 = ivec4(v1 << 4, v3 << 4, (v5 & 0x7f) << 5, 0x780);
        return;
    }

    int mode = ((v1 & 0x80) >> 7) | ((v2 & 0x80) >> 6) | ((v3 & 0x80) >> 5);
    int va = v0 | ((v1 & 0x40) << 2);
    int vb0 = v2 & 0x3f;
    int vb1 =  v3 & 0x3f;
    int vc = v1 & 0x3f;
    int vd0 = v4 & 0x7f;
    int vd1 = v5 & 0x7f;

    int d_bits = 7 - (mode & 1);
    if ((mode & 5) == 4)
        d_bits -= 2;

    vd0 = bitfieldExtract(vd0, 0, d_bits);
    vd1 = bitfieldExtract(vd1, 0, d_bits);

    int x0 = (v2 >> 6) & 1;
    int x1 = (v3 >> 6) & 1;
    int x2 = (v4 >> 6) & 1;
    int x3 = (v5 >> 6) & 1;
    int x4 = (v4 >> 5) & 1;
    int x5 = (v5 >> 5) & 1;

    int ohm = 1 << mode;
    if ((ohm & 0xa4) != 0) va |= x0 << 9;
    if ((ohm & 0x08) != 0) va |= x2 << 9;
    if ((ohm & 0x50) != 0) va |= x4 << 9;
    if ((ohm & 0x50) != 0) va |= x5 << 10;
    if ((ohm & 0xa0) != 0) va |= x1 << 10;
    if ((ohm & 0xc0) != 0) va |= x2 << 11;

    if ((ohm & 0x04) != 0) vc |= x1 << 6;
    if ((ohm & 0xe8) != 0) vc |= x3 << 6;
    if ((ohm & 0x20) != 0) vc |= x2 << 7;

    if ((ohm & 0x5b) != 0) vb0 |= x0 << 6;
    if ((ohm & 0x5b) != 0) vb1 |= x1 << 6;
    if ((ohm & 0x12) != 0) vb0 |= x2 << 7;
    if ((ohm & 0x12) != 0) vb1 |= x3 << 7;

    int shamt = (mode >> 1) ^ 3;
    va <<= shamt;
    vb0 <<= shamt;
    vb1 <<= shamt;
    vc <<= shamt;
    vd0 <<= shamt;
    vd1 <<= shamt;

    ep1 = ivec4(clamp(ivec3(va, va - vb0, va - vb1), ivec3(0), ivec3(0xfff)), 0x780);
    ep0 = ivec4(clamp(ivec3(va - vc, va - vb0 - vc - vd0, va - vb1 - vc - vd1), ivec3(0), ivec3(0xfff)), 0x780);

    if (major_component == 1)
    {
        swap(ep0.r, ep0.g);
        swap(ep1.r, ep1.g);
    }
    else if (major_component == 2)
    {
        swap(ep0.r, ep0.b);
        swap(ep1.r, ep1.b);
    }
}

void decode_endpoint_ldr_rgb_base_offset(out ivec4 ep0, out ivec4 ep1,
        int v0, int v1, int v2, int v3, int v4, int v5)
{
    bit_transfer_signed(v1, v0);
    bit_transfer_signed(v3, v2);
    bit_transfer_signed(v5, v4);
    if (v1 + v3 + v5 >= 0)
    {
        ep0 = ivec4(v0, v2, v4, 0xff);
        ep1 = ivec4(v0 + v1, v2 + v3, v4 + v5, 0xff);
    }
    else
    {
        ep0 = blue_contract(v0 + v1, v2 + v3, v4 + v5, 0xff);
        ep1 = blue_contract(v0, v2, v4, 0xff);
    }

    ep0.rgb = clamp(ep0.rgb, ivec3(0), ivec3(0xff));
    ep1.rgb = clamp(ep1.rgb, ivec3(0), ivec3(0xff));
}

void decode_endpoint_ldr_rgba_direct(out ivec4 ep0, out ivec4 ep1,
        int v0, int v1, int v2, int v3,
        int v4, int v5, int v6, int v7)
{
    int s0 = v0 + v2 + v4;
    int s1 = v1 + v3 + v5;
    if (s1 >= s0)
    {
        ep0 = ivec4(v0, v2, v4, v6);
        ep1 = ivec4(v1, v3, v5, v7);
    }
    else
    {
        ep0 = blue_contract(v1, v3, v5, v7);
        ep1 = blue_contract(v0, v2, v4, v6);
    }
}

void decode_endpoint_ldr_rgba_base_offset(out ivec4 ep0, out ivec4 ep1,
        int v0, int v1, int v2, int v3, int v4, int v5, int v6, int v7)
{
    bit_transfer_signed(v1, v0);
    bit_transfer_signed(v3, v2);
    bit_transfer_signed(v5, v4);
    bit_transfer_signed(v7, v6);

    if (v1 + v3 + v5 >= 0)
    {
        ep0 = ivec4(v0, v2, v4, v6);
        ep1 = ivec4(v0 + v1, v2 + v3, v4 + v5, v6 + v7);
    }
    else
    {
        ep0 = blue_contract(v0 + v1, v2 + v3, v4 + v5, v6 + v7);
        ep1 = blue_contract(v0, v2, v4, v6);
    }

    ep0 = clamp(ep0, ivec4(0), ivec4(0xff));
    ep1 = clamp(ep1, ivec4(0), ivec4(0xff));
}

void decode_endpoint_hdr_alpha(out int ep0, out int ep1, int v6, int v7)
{
    int mode = ((v6 >> 7) & 1) | ((v7 >> 6) & 2);
    v6 &= 0x7f;
    v7 &= 0x7f;

    if (mode == 3)
    {
        ep0 = v6 << 5;
        ep1 = v7 << 5;
    }
    else
    {
        v6 |= (v7 << (mode + 1)) & 0x780;
        v7 &= 0x3f >> mode;
        v7 ^= 0x20 >> mode;
        v7 -= 0x20 >> mode;
        v6 <<= 4 - mode;
        v7 <<= 4 - mode;
        v7 += v6;
        v7 = clamp(v7, 0, 0xfff);
        ep0 = v6;
        ep1 = v7;
    }
}

void decode_endpoint(out ivec4 ep0, out ivec4 ep1, out int decode_mode,
                     uvec4 payload, int bit_offset, ivec4 quant, int ep_mode,
                     int base_endpoint_index, int num_endpoint_bits)
{
    num_endpoint_bits += bit_offset;
    payload &= build_bitmask(num_endpoint_bits);

    // Could of course use an array, but that doesn't lower nicely to indexed registers on all GPUs.
    int v0, v1, v2, v3, v4, v5, v6, v7;
    int num_values = 2 * ((ep_mode >> 2) + 1);

#define DECODE_EP(i) \
    int(texelFetch(LUTEndpointUnquantize, quant.w + decode_integer_sequence(payload, bit_offset, i + base_endpoint_index, quant.xyz)).x)

    int hi_bits = ep_mode >> 2;
    v0 = DECODE_EP(0);
    v1 = DECODE_EP(1);

    if (hi_bits >= 1)
    {
        v2 = DECODE_EP(2);
        v3 = DECODE_EP(3);
    }

    if (hi_bits >= 2)
    {
        v4 = DECODE_EP(4);
        v5 = DECODE_EP(5);
    }

    if (hi_bits >= 3)
    {
        v6 = DECODE_EP(6);
        v7 = DECODE_EP(7);
    }

    switch (ep_mode)
    {
    case 0:
        decode_endpoint_ldr_luma_direct(ep0, ep1,
            v0, v1);
        decode_mode = MODE_LDR;
        break;

    case 1:
        decode_endpoint_ldr_luma_base_offset(ep0, ep1,
            v0, v1);
        decode_mode = MODE_LDR;
        break;

    case 2:
        decode_endpoint_hdr_luma_direct(ep0, ep1,
            v0, v1);
        decode_mode = MODE_HDR;
        break;

    case 3:
        decode_endpoint_hdr_luma_direct_small_range(ep0, ep1,
            v0, v1);
        decode_mode = MODE_HDR;
        break;

    case 4:
        decode_endpoint_ldr_luma_alpha_direct(ep0, ep1,
            v0, v1, v2, v3);
        decode_mode = MODE_LDR;
        break;

    case 5:
        decode_endpoint_ldr_luma_alpha_base_offset(ep0, ep1,
            v0, v1, v2, v3);
        decode_mode = MODE_LDR;
        break;

    case 6:
        decode_endpoint_ldr_rgb_base_scale(ep0, ep1,
            v0, v1, v2, v3);
        decode_mode = MODE_LDR;
        break;

    case 7:
        decode_endpoint_hdr_rgb_scale(ep0, ep1,
            v0, v1, v2, v3);
        decode_mode = MODE_HDR;
        break;

    case 8:
        decode_endpoint_ldr_rgb_direct(ep0, ep1,
            v0, v1, v2, v3, v4, v5);
        decode_mode = MODE_LDR;
        break;

    case 9:
        decode_endpoint_ldr_rgb_base_offset(ep0, ep1,
            v0, v1, v2, v3, v4, v5);
        decode_mode = MODE_LDR;
        break;

    case 10:
        decode_endpoint_ldr_rgb_base_scale_two_a(ep0, ep1,
            v0, v1, v2, v3, v4, v5);
        decode_mode = MODE_LDR;
        break;

    case 11:
    case 14:
    case 15:
        decode_endpoint_hdr_rgb_direct(ep0, ep1,
            v0, v1, v2, v3, v4, v5);
        if (ep_mode == 14)
        {
            ep0.a = v6;
            ep1.a = v7;
            decode_mode = MODE_HDR_LDR_ALPHA;
        }
        else if (ep_mode == 15)
        {
            decode_endpoint_hdr_alpha(ep0.a, ep1.a, v6, v7);
            decode_mode = MODE_HDR;
        }
        else
            decode_mode = MODE_HDR;
        break;

    case 12:
        decode_endpoint_ldr_rgba_direct(ep0, ep1,
            v0, v1, v2, v3, v4, v5, v6, v7);
        decode_mode = MODE_LDR;
        break;

    case 13:
        decode_endpoint_ldr_rgba_base_offset(ep0, ep1,
            v0, v1, v2, v3, v4, v5, v6, v7);
        decode_mode = MODE_LDR;
        break;
    }

    if (DECODE_8BIT && decode_mode != MODE_LDR)
        decode_error = true;
}

#define CHECK_DECODE_ERROR() do { \
    if (decode_error) \
    { \
        emit_decode_error(coord.xy); \
        return; \
    } \
} while(false)

void emit_decode_error(ivec2 coord)
{
#ifdef VULKAN
    if (is_3Dimage)
        imageStore(OutputImage3D, ivec3(coord, gl_WorkGroupID.z), error_color);
    else
        imageStore(OutputImage2Darray, ivec3(coord, gl_WorkGroupID.z), error_color);
#else /* VULKAN */
    imageStore(OutputImage, coord, error_color);
#endif /* VULKAN */
}

int compute_num_endpoint_pairs(int num_partitions, int cem)
{
    int ret;
    if (num_partitions > 1)
    {
        bool single_cem = (cem & 3) == 0;
        if (single_cem)
            ret = ((cem >> 4) + 1) * num_partitions;
        else
            ret = (cem & 3) * num_partitions + bitCount(bitfieldExtract(uint(cem), 2, num_partitions));
    }
    else
    {
        ret = (cem >> 2) + 1;
    }
    return ret;
}

void decode_cem_base_endpoint(uvec4 payload, int weight_cost_bits, inout int cem, out int base_endpoint_index,
    int num_partitions, int partition_index)
{
    if (num_partitions > 1)
    {
        bool single_cem = (cem & 3) == 0;
        if (single_cem)
        {
            cem >>= 2;
            base_endpoint_index = ((cem >> 2) + 1) * partition_index;
        }
        else
        {
            if (partition_index != 0)
                base_endpoint_index = (cem & 3) * partition_index + bitCount(bitfieldExtract(uint(cem), 2, partition_index));
            else
                base_endpoint_index = 0;

            int base_class = (cem & 3) - 1;
            int extra_cem_bits = num_partitions * 3 - 4;
            int extra_bits = extract_bits(payload, 128 - weight_cost_bits - extra_cem_bits, extra_cem_bits);
            cem = (extra_bits << 4) | (cem >> 2);

            int class_offset_bit = (cem >> partition_index) & 1;
            int ep_bits = (cem >> (num_partitions + 2 * partition_index)) & 3;

            cem = 4 * (base_class + class_offset_bit) + ep_bits;
        }
        base_endpoint_index *= 2;
    }
    else
    {
        base_endpoint_index = 0;
    }
}

ivec4 void_extent_color(uvec4 payload, out int decode_mode)
{
    int min_s = extract_bits(payload, 12, 13);
    int max_s = extract_bits(payload, 12 + 13, 13);
    int min_t = extract_bits(payload, 12 + 2 * 13, 13);
    int max_t = extract_bits(payload, 12 + 3 * 13, 13);

    int reserved = extract_bits(payload, 10, 2);
    if (reserved != 3)
    {
        decode_error = true;
        return ivec4(0);
    }

    if (!all(equal(ivec4(min_s, max_s, min_t, max_t), ivec4((1 << 13) - 1))))
    {
        if (any(greaterThanEqual(ivec2(min_s, min_t), ivec2(max_s, max_t))))
        {
            decode_error = true;
            return ivec4(0);
        }
    }

    decode_mode = (payload.x & (1u << 9)) != 0u ? MODE_HDR : MODE_LDR;

    int r = extract_bits(payload, 64, 16);
    int g = extract_bits(payload, 64 + 16, 16);
    int b = extract_bits(payload, 64 + 32, 16);
    int a = extract_bits(payload, 64 + 48, 16);

    return ivec4(r, g, b, a);
}

void main()
{
    ivec4 coord = build_coord();
#ifdef VULKAN
    if (any(greaterThanEqual(coord.xy, texel_end.xy)))
        return;
#else /* VULKAN */
    if (any(greaterThanEqual(coord.xy, imageSize(OutputImage))))
        return;
#endif /* VULKAN */

    ivec2 pixel_coord = ivec2(gl_LocalInvocationID.xy);
    int linear_pixel = int(gl_WorkGroupSize.x) * pixel_coord.y + pixel_coord.x;
    uvec4 payload;
#ifdef VULKAN
    if (is_3Dimage)
        payload = texelFetch(PayloadInput3D, ivec3(coord.zw, gl_WorkGroupID.z), 0);
    else
        payload = texelFetch(PayloadInput2Darray,ivec3(coord.zw, gl_WorkGroupID.z), 0);
#else /* VULKAN */
    payload = texelFetch(PayloadInput, coord.zw, 0);
#endif /* VULKAN */

    BlockMode block_mode = decode_block_mode(payload);
    CHECK_DECODE_ERROR();

    ivec4 final_color;
    int decode_mode;
    if (block_mode.void_extent)
    {
        final_color = void_extent_color(payload, decode_mode);
        CHECK_DECODE_ERROR();
    }
    else
    {
        int weight_cost_bits;
        ivec4 weights = decode_weights(payload, block_mode, normalize_coord(pixel_coord), weight_cost_bits);

        int partition_index = 0;
        if (block_mode.num_partitions > 1)
        {
            int lut_x = pixel_coord.x + int(gl_WorkGroupSize.x) * (block_mode.seed & 31);
            int lut_y = pixel_coord.y + int(gl_WorkGroupSize.y) * (block_mode.seed >> 5);
#ifdef VULKAN
            int lut_width = int(gl_WorkGroupSize.x) * 32;
            partition_index = int(texelFetch(LUTPartitionTable, lut_y * lut_width + lut_x).x);
#else /* VULKAN */
            partition_index = int(texelFetch(LUTPartitionTable, ivec2(lut_x, lut_y), 0).x);
#endif /* VULKAN */
            partition_index = (partition_index >> (2 * block_mode.num_partitions - 4)) & 3;
        }

        int available_endpoint_bits = max(128 - block_mode.config_bits - weight_cost_bits, 0);

        // In multi-partition mode, the 6-bit CEM field is encoded as
        // First two bits tell if all CEM field are the same, if not we specify a class offset, and N bits
        // after that will offset the class by 1.
        int num_endpoint_pairs = compute_num_endpoint_pairs(block_mode.num_partitions, block_mode.cem);

        // Error color must be emitted if we need more than 18 integer sequence encoded values of color.
        if (num_endpoint_pairs > 9)
        {
            decode_error = true;
            emit_decode_error(coord.xy);
            return;
        }

        ivec4 endpoint_quant = ivec4(texelFetch(LUTRemainingBitsToEndpointQuantizer,
                128 * (num_endpoint_pairs - 1) + available_endpoint_bits));

        // Only read the bits we need for endpoints.
        int num_endpoint_values = num_endpoint_pairs * 2;
        available_endpoint_bits =
            endpoint_quant.x * num_endpoint_values +
            idiv5_ceil(endpoint_quant.y * 8 * num_endpoint_values) +
            idiv3_ceil(endpoint_quant.z * 7 * num_endpoint_values);

        // No space left for color endpoints.
        if (all(equal(endpoint_quant.xyz, ivec3(0))))
        {
            decode_error = true;
            emit_decode_error(coord.xy);
            return;
        }

        int endpoint_bit_offset = block_mode.primary_config_bits;
        ivec4 ep0, ep1;

        // Decode CEM for multi-partition schemes.
        int cem = block_mode.cem;
        int base_endpoint_index;
        decode_cem_base_endpoint(payload, weight_cost_bits, cem, base_endpoint_index,
                                 block_mode.num_partitions, partition_index);

        decode_endpoint(ep0, ep1, decode_mode, payload, endpoint_bit_offset, endpoint_quant,
                        cem, base_endpoint_index, available_endpoint_bits);
        CHECK_DECODE_ERROR();

        final_color = interpolate_endpoint(ep0, ep1, weights, decode_mode);
    }

    if (DECODE_8BIT)
    {
#ifdef VULKAN
        if (is_3Dimage)
            imageStore(OutputImage3D, ivec3(coord.xy, gl_WorkGroupID.z), uvec4(final_color >> 8));
        else
            imageStore(OutputImage2Darray, ivec3(coord.xy, gl_WorkGroupID.z), uvec4(final_color >> 8));
#else /* VULKAN */
        imageStore(OutputImage, coord.xy, uvec4(final_color >> 8));
#endif /* VULKAN */
    }
    else
    {
        uvec4 encoded;
        if (block_mode.void_extent && decode_mode == MODE_HDR)
            encoded = uvec4(final_color);
        else
            encoded = decode_fp16(final_color, decode_mode);
#ifdef VULKAN
        if (is_3Dimage)
            imageStore(OutputImage3D, ivec3(coord.xy, gl_WorkGroupID.z), encoded);
        else
            imageStore(OutputImage2Darray, ivec3(coord.xy, gl_WorkGroupID.z), encoded);
#else /* VULKAN */
        imageStore(OutputImage, coord.xy, encoded);
#endif /* VULKAN */
    }
}
