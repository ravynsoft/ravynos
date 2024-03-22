//
// Copyright (C) 2009-2021 Intel Corporation
//
// SPDX-License-Identifier: MIT
//
//

#pragma once

// TODO: AABB_work_group_reduce is super slow, remove !!!

#pragma cl_intel_subgroups : enable
#pragma cl_khr_fp16        : enable
#pragma OPENCL EXTENSION cl_khr_fp16 : enable


uint intel_sub_group_ballot(bool valid);

// atom_min
float __attribute__((overloadable)) atom_min(volatile __global float *p, float val);
float __attribute__((overloadable)) atom_min(volatile __local float *p, float val);
float __attribute__((overloadable)) atomic_min(volatile __global float *p, float val);
float __attribute__((overloadable)) atomic_min(volatile __local float *p, float val);
// atom_max
float __attribute__((overloadable)) atom_max(volatile __global float *p, float val);
float __attribute__((overloadable)) atom_max(volatile __local float *p, float val);
float __attribute__((overloadable)) atomic_max(volatile __global float *p, float val);
float __attribute__((overloadable)) atomic_max(volatile __local float *p, float val);
// atom_cmpxchg
float __attribute__((overloadable)) atom_cmpxchg(volatile __global float *p, float cmp, float val);
float __attribute__((overloadable)) atom_cmpxchg(volatile __local float *p, float cmp, float val);
float __attribute__((overloadable)) atomic_cmpxchg(volatile __global float *p, float cmp, float val);
float __attribute__((overloadable)) atomic_cmpxchg(volatile __local float *p, float cmp, float val);



inline uint subgroup_single_atomic_add(global uint *p, uint val)
{
    const uint subgroupLocalID = get_sub_group_local_id();
    const int v = subgroupLocalID == 0 ? atomic_add(p, val) : 0;
    return sub_group_broadcast(v, 0);
}

inline float halfarea(const float3 d)
{
    return fma(d.x, (d.y + d.z), d.y * d.z);
}

inline float area(const float3 d)
{
    return halfarea(d) * 2.0f;
}

inline uint maxDim(const float3 a)
{
    const float3 b = fabs(a);
    const bool b_x_y = b.x > b.y;
    const float cur_max = b_x_y ? b.x : b.y;
    const uint cur_idx = b_x_y ? 0 : 1;
    const bool b_x_y_z = b.z > cur_max;
    return b_x_y_z ? 2 : cur_idx;
}

inline uint3 sortByMaxDim(const float3 a)
{
    const uint kz = maxDim(a);
    const uint _kx = (kz + 1) % 3;
    const uint _ky = (_kx + 1) % 3;
    const bool kz_pos = a[kz] >= 0.0f;
    const uint kx = kz_pos ? _ky : _kx;
    const uint ky = kz_pos ? _kx : _ky;
    return (uint3)(kx, ky, kz);
}

inline uint4 sort4_ascending(const uint4 dist)
{
    const uint a0 = dist.s0;
    const uint a1 = dist.s1;
    const uint a2 = dist.s2;
    const uint a3 = dist.s3;
    const uint b0 = min(a0, a2);
    const uint b1 = min(a1, a3);
    const uint b2 = max(a0, a2);
    const uint b3 = max(a1, a3);
    const uint c0 = min(b0, b1);
    const uint c1 = max(b0, b1);
    const uint c2 = min(b2, b3);
    const uint c3 = max(b2, b3);
    const uint d0 = c0;
    const uint d1 = min(c1, c2);
    const uint d2 = max(c1, c2);
    const uint d3 = c3;
    return (uint4)(d0, d1, d2, d3);
}

__constant const uint shuffleA[8] = {1, 0, 3, 2, 5, 4, 7, 6};
__constant const uint shuffleB[8] = {2, 3, 0, 1, 7, 6, 5, 4};
__constant const uint shuffleC[8] = {1, 0, 3, 2, 5, 4, 7, 6};
__constant const uint shuffleD[8] = {7, 6, 5, 4, 3, 2, 1, 0};
__constant const uint shuffleE[8] = {2, 3, 0, 1, 6, 7, 4, 5};
__constant const uint shuffleF[8] = {1, 0, 3, 2, 5, 4, 7, 6};
__constant const uint shuffleG[8] = {0, 2, 1, 3, 5, 4, 7, 6};

__constant const uint selAA[8] = {0, 1, 0, 1, 0, 1, 0, 1};
__constant const uint selCC[8] = {0, 0, 1, 1, 0, 0, 1, 1};
__constant const uint selF0[8] = {0, 0, 0, 0, 1, 1, 1, 1};

__constant const uint selGG[8] = {0, 0, 1, 0, 1, 1, 1, 1};

inline uint compare_exchange_descending(const uint a0, const uint shuffleMask, const uint selectMask)
{
    const uint a1 = intel_sub_group_shuffle(a0, shuffleMask);
    const uint a_min = min(a0, a1);
    const uint a_max = max(a0, a1);
    return select(a_max, a_min, selectMask);
}

inline uint compare_exchange_ascending(const uint a0, const uint shuffleMask, const uint selectMask)
{
    const uint a1 = intel_sub_group_shuffle(a0, shuffleMask);
    const uint a_min = min(a0, a1);
    const uint a_max = max(a0, a1);
    return select(a_min, a_max, selectMask);
}

inline uint sort8_descending(const uint aa)
{
    const unsigned int slotID = get_sub_group_local_id() % 8;
    const uint bb = compare_exchange_descending(aa, shuffleA[slotID], selAA[slotID]);
    const uint cc = compare_exchange_descending(bb, shuffleB[slotID], selCC[slotID]);
    const uint dd = compare_exchange_descending(cc, shuffleC[slotID], selAA[slotID]);
    const uint ee = compare_exchange_descending(dd, shuffleD[slotID], selF0[slotID]);
    const uint ff = compare_exchange_descending(ee, shuffleE[slotID], selCC[slotID]);
    const uint gg = compare_exchange_descending(ff, shuffleF[slotID], selAA[slotID]);
    return gg;
}

inline uint sort8_ascending(const uint aa)
{
    const unsigned int slotID = get_sub_group_local_id() % 8;
    const uint bb = compare_exchange_ascending(aa, shuffleA[slotID], selAA[slotID]);
    const uint cc = compare_exchange_ascending(bb, shuffleB[slotID], selCC[slotID]);
    const uint dd = compare_exchange_ascending(cc, shuffleC[slotID], selAA[slotID]);
    const uint ee = compare_exchange_ascending(dd, shuffleD[slotID], selF0[slotID]);
    const uint ff = compare_exchange_ascending(ee, shuffleE[slotID], selCC[slotID]);
    const uint gg = compare_exchange_ascending(ff, shuffleF[slotID], selAA[slotID]);
    return gg;
}

inline uint sort4_descending(const uint aa)
{
    const unsigned int slotID = get_sub_group_local_id() % 8;
    const uint bb = compare_exchange_descending(aa, shuffleA[slotID], selAA[slotID]);
    const uint cc = compare_exchange_descending(bb, shuffleB[slotID], selCC[slotID]);
    const uint dd = compare_exchange_descending(cc, shuffleG[slotID], selGG[slotID]);
    return dd;
}

inline ulong compare_exchange_descending_ulong(const ulong a0, const uint shuffleMask, const uint selectMask)
{
    const ulong a1 = intel_sub_group_shuffle(a0, shuffleMask);
    const ulong a_min = min(a0, a1);
    const ulong a_max = max(a0, a1);
    return select(a_max, a_min, (ulong)selectMask);
}

inline ulong compare_exchange_ascending_ulong(const ulong a0, const uint shuffleMask, const uint selectMask)
{
    const ulong a1 = intel_sub_group_shuffle(a0, shuffleMask);
    const ulong a_min = min(a0, a1);
    const ulong a_max = max(a0, a1);
    return select(a_min, a_max, (ulong)selectMask);
}

inline ulong sort8_ascending_ulong(const ulong aa)
{
    const unsigned int slotID = get_sub_group_local_id() % 8;
    const ulong bb = compare_exchange_ascending_ulong(aa, shuffleA[slotID], selAA[slotID]);
    const ulong cc = compare_exchange_ascending_ulong(bb, shuffleB[slotID], selCC[slotID]);
    const ulong dd = compare_exchange_ascending_ulong(cc, shuffleC[slotID], selAA[slotID]);
    const ulong ee = compare_exchange_ascending_ulong(dd, shuffleD[slotID], selF0[slotID]);
    const ulong ff = compare_exchange_ascending_ulong(ee, shuffleE[slotID], selCC[slotID]);
    const ulong gg = compare_exchange_ascending_ulong(ff, shuffleF[slotID], selAA[slotID]);
    return gg;
}

inline uint bitInterleave3D(const uint4 in)
{
    uint x = in.x, y = in.y, z = in.z;
    x = (x | (x << 16)) & 0x030000FF;
    x = (x | (x << 8)) & 0x0300F00F;
    x = (x | (x << 4)) & 0x030C30C3;
    x = (x | (x << 2)) & 0x09249249;

    y = (y | (y << 16)) & 0x030000FF;
    y = (y | (y << 8)) & 0x0300F00F;
    y = (y | (y << 4)) & 0x030C30C3;
    y = (y | (y << 2)) & 0x09249249;

    z = (z | (z << 16)) & 0x030000FF;
    z = (z | (z << 8)) & 0x0300F00F;
    z = (z | (z << 4)) & 0x030C30C3;
    z = (z | (z << 2)) & 0x09249249;

    return x | (y << 1) | (z << 2);
}

inline uint bitInterleave4D(const uint4 in)
{
    uint x = in.x, y = in.y, z = in.z, w = in.w;

    x = x & 0x000000ff;
    x = (x ^ (x << 16)) & 0x00c0003f;
    x = (x ^ (x << 8)) & 0x00c03807;
    x = (x ^ (x << 4)) & 0x08530853;
    x = (x ^ (x << 2)) & 0x09090909;
    x = (x ^ (x << 1)) & 0x11111111;

    y = y & 0x000000ff;
    y = (y ^ (y << 16)) & 0x00c0003f;
    y = (y ^ (y << 8)) & 0x00c03807;
    y = (y ^ (y << 4)) & 0x08530853;
    y = (y ^ (y << 2)) & 0x09090909;
    y = (y ^ (y << 1)) & 0x11111111;

    z = z & 0x000000ff;
    z = (z ^ (z << 16)) & 0x00c0003f;
    z = (z ^ (z << 8)) & 0x00c03807;
    z = (z ^ (z << 4)) & 0x08530853;
    z = (z ^ (z << 2)) & 0x09090909;
    z = (z ^ (z << 1)) & 0x11111111;

    w = w & 0x000000ff;
    w = (w ^ (w << 16)) & 0x00c0003f;
    w = (w ^ (w << 8)) & 0x00c03807;
    w = (w ^ (w << 4)) & 0x08530853;
    w = (w ^ (w << 2)) & 0x09090909;
    w = (w ^ (w << 1)) & 0x11111111;

    return (x | (y << 1) | (z << 2) | (w << 3));
}

inline ulong ulong_bitInterleave4D(const uint4 in)
{
    ulong x = in.x, y = in.y, z = in.z, w = in.w;

    x = x & 0x0000ffff;
    x = (x ^ (x << 32)) & 0x0000f800000007ff;
    x = (x ^ (x << 16)) & 0x0000f80007c0003f;
    x = (x ^ (x << 8)) & 0x00c0380700c03807;
    x = (x ^ (x << 4)) & 0x0843084308430843;
    x = (x ^ (x << 2)) & 0x0909090909090909;
    x = (x ^ (x << 1)) & 0x1111111111111111;

    y = y & 0x0000ffff;
    y = (y ^ (y << 32)) & 0x0000f800000007ff;
    y = (y ^ (y << 16)) & 0x0000f80007c0003f;
    y = (y ^ (y << 8)) & 0x00c0380700c03807;
    y = (y ^ (y << 4)) & 0x0843084308430843;
    y = (y ^ (y << 2)) & 0x0909090909090909;
    y = (y ^ (y << 1)) & 0x1111111111111111;

    z = z & 0x0000ffff;
    z = (z ^ (z << 32)) & 0x0000f800000007ff;
    z = (z ^ (z << 16)) & 0x0000f80007c0003f;
    z = (z ^ (z << 8)) & 0x00c0380700c03807;
    z = (z ^ (z << 4)) & 0x0843084308430843;
    z = (z ^ (z << 2)) & 0x0909090909090909;
    z = (z ^ (z << 1)) & 0x1111111111111111;

    w = w & 0x0000ffff;
    w = (w ^ (w << 32)) & 0x0000f800000007ff;
    w = (w ^ (w << 16)) & 0x0000f80007c0003f;
    w = (w ^ (w << 8)) & 0x00c0380700c03807;
    w = (w ^ (w << 4)) & 0x0843084308430843;
    w = (w ^ (w << 2)) & 0x0909090909090909;
    w = (w ^ (w << 1)) & 0x1111111111111111;

    return (x | (y << 1) | (z << 2) | (w << 3));
}

inline uint bitCompact(uint x)
{
    x &= 0x09249249;
    x = (x ^ (x >> 2)) & 0x030c30c3;
    x = (x ^ (x >> 4)) & 0x0300f00f;
    x = (x ^ (x >> 8)) & 0xff0000ff;
    x = (x ^ (x >> 16)) & 0x000003ff;
    return x;
}

inline uint3 bitCompact3D(const uint in)
{
    const uint x = bitCompact(x >> 0);
    const uint y = bitCompact(y >> 1);
    const uint z = bitCompact(z >> 2);
    return (uint3)(x, y, z);
}

inline uint convertToPushIndices8(uint ID)
{
    const unsigned int slotID = get_sub_group_local_id();
    uint index = 0;
    for (uint i = 0; i < 8; i++)
    {
        const uint mask = intel_sub_group_ballot(ID == i);
        const uint new_index = ctz(mask);
        index = i == slotID ? new_index : index;
    }
    return index;
}

inline uint convertToPushIndices16(uint ID)
{
    const unsigned int slotID = get_sub_group_local_id();
    uint index = 0;
    for (uint i = 0; i < 16; i++)
    {
        const uint mask = intel_sub_group_ballot(ID == i);
        const uint new_index = ctz(mask);
        index = i == slotID ? new_index : index;
    }
    return index;
}

#define FLOAT_EXPONENT_MASK     (0x7F800000)  // used to be EXPONENT_MASK
#define FLOAT_MANTISSA_MASK     (0x007FFFFF)  // used to be MANTISSA_MASK
#define FLOAT_NEG_ONE_EXP_MASK  (0x3F000000)
#define FLOAT_BIAS              (127)
#define FLOAT_MANTISSA_BITS     (23)

inline float3 frexp_vec3(float3 len, int3* exp)
{
    float3 mant = as_float3((int3)((as_int3(len) & (int3)FLOAT_MANTISSA_MASK) + (int3)FLOAT_NEG_ONE_EXP_MASK));
    mant = select(mant, (float3)(0.5f), (int3)(mant == (float3)(1.0f)));
    mant = copysign(mant, len);
    *exp = ((as_int3(len) & (int3)FLOAT_EXPONENT_MASK) >> (int3)FLOAT_MANTISSA_BITS) - ((int3)FLOAT_BIAS - (int3)(1));
    return mant;
}


#ifndef uniform
#define uniform
#endif

#ifndef varying
#define varying
#endif

uint get_sub_group_global_id()
{
    return get_sub_group_id() + get_num_sub_groups() * get_group_id( 0 );
}

// each lane contains the number of 1 bits below the corresponding position in 'mask'
uint subgroup_bit_prefix_exclusive(uniform uint mask)
{
    varying ushort lane = get_sub_group_local_id();
    varying uint lane_mask = (1 << lane) - 1;
    varying uint m = mask & lane_mask;
    return popcount(m);
}

uint bit_prefix_exclusive(uniform uint mask, varying uint lane_idx )
{
    varying uint lane_mask = (1 << lane_idx) - 1;
    varying uint m = mask & lane_mask;
    return popcount(m);
}


uint3 sub_group_broadcast_uint3(uint3 v, uniform ushort idx)
{
    return (uint3)(sub_group_broadcast(v.x,idx),
                   sub_group_broadcast(v.y,idx),
                   sub_group_broadcast(v.z,idx));
}

float3 sub_group_broadcast_float3(float3 v, uniform ushort idx)
{
    return (float3)(sub_group_broadcast(v.x, idx),
                    sub_group_broadcast(v.y, idx),
                    sub_group_broadcast(v.z, idx));
}

float3 sub_group_reduce_min_float3(float3 v)
{
    return (float3)(sub_group_reduce_min(v.x),
                    sub_group_reduce_min(v.y),
                    sub_group_reduce_min(v.z) );
}
float3 sub_group_reduce_max_float3(float3 v)
{
    return (float3)(sub_group_reduce_max(v.x),
                    sub_group_reduce_max(v.y),
                    sub_group_reduce_max(v.z));
}

float3 sub_group_shuffle_float3(float3 v, uniform ushort idx)
{
    return (float3)(intel_sub_group_shuffle(v.x, idx),
                    intel_sub_group_shuffle(v.y, idx),
                    intel_sub_group_shuffle(v.z, idx));
}
uint3 sub_group_shuffle_uint3(uint3 v, uniform ushort idx)
{
    return (uint3)( intel_sub_group_shuffle(v.x, idx),
                    intel_sub_group_shuffle(v.y, idx),
                    intel_sub_group_shuffle(v.z, idx));
}


inline uchar sub_group_reduce_or_N6(uchar val)
{
    val = val | intel_sub_group_shuffle_down(val, val, 4);
    val = val | intel_sub_group_shuffle_down(val, val, 2);
    val = val | intel_sub_group_shuffle_down(val, val, 1);
    return sub_group_broadcast(val, 0);
}

inline uchar sub_group_reduce_or_N6_2xSIMD8_in_SIMD16(uchar val)
{
    uint SIMD8_id = get_sub_group_local_id() / 8;
    val = val | intel_sub_group_shuffle_down(val, val, 4);
    val = val | intel_sub_group_shuffle_down(val, val, 2);
    val = val | intel_sub_group_shuffle_down(val, val, 1);

    return intel_sub_group_shuffle(val, SIMD8_id * 8);
}


inline __attribute__((overloadable)) uint atomic_inc_local( local uint* p )
{
    return atomic_fetch_add_explicit( (volatile local atomic_uint*) p, (uint)1, memory_order_relaxed, memory_scope_work_group );
}

inline __attribute__((overloadable)) int atomic_inc_local(local int* p)
{
    return atomic_fetch_add_explicit( (volatile local atomic_int*) p, (int)1, memory_order_relaxed, memory_scope_work_group);
}

inline __attribute__((overloadable)) uint atomic_dec_local(local uint* p)
{
    return atomic_fetch_sub_explicit((volatile local atomic_uint*) p, (uint)1, memory_order_relaxed, memory_scope_work_group);
}

inline __attribute__((overloadable)) int atomic_dec_local(local int* p)
{
    return atomic_fetch_sub_explicit((volatile local atomic_int*) p, (int)1, memory_order_relaxed, memory_scope_work_group);
}

inline __attribute__((overloadable)) uint atomic_sub_local(local uint* p, uint n)
{
    return atomic_fetch_sub_explicit((volatile local atomic_uint*) p, n, memory_order_relaxed, memory_scope_work_group);
}

inline __attribute__((overloadable)) int atomic_sub_local(local int* p, int n )
{
    return atomic_fetch_sub_explicit( (volatile local atomic_int*) p, n, memory_order_relaxed, memory_scope_work_group);
}

inline uint atomic_add_local( local uint* p, uint n )
{
    return atomic_fetch_add_explicit((volatile local atomic_uint*) p, n, memory_order_relaxed, memory_scope_work_group);
}

inline uint atomic_xor_local(local uint* p, uint n)
{
    return atomic_fetch_xor_explicit((volatile local atomic_uint*) p, n, memory_order_relaxed, memory_scope_work_group);
}

inline uint atomic_or_local(local uint* p, uint n)
{
    return atomic_fetch_or_explicit((volatile local atomic_uint*) p, n, memory_order_relaxed, memory_scope_work_group);
}

inline uint atomic_min_local(local uint* p, uint n)
{
    return atomic_fetch_min_explicit((volatile local atomic_uint*) p, n, memory_order_relaxed, memory_scope_work_group);
}

inline uint atomic_max_local(local uint* p, uint n)
{
    return atomic_fetch_max_explicit((volatile local atomic_uint*) p, n, memory_order_relaxed, memory_scope_work_group);
}




inline uint atomic_inc_global( global uint* p )
{
    return atomic_fetch_add_explicit((volatile global atomic_uint*) p, (uint)1, memory_order_relaxed, memory_scope_device);
}

inline uint atomic_dec_global(global uint* p)
{
    return atomic_fetch_sub_explicit( (volatile global atomic_uint*) p, (uint)1, memory_order_relaxed, memory_scope_device);
}

inline bool atomic_compare_exchange_global(global uint* p, uint* expected, uint desired)
{
    return atomic_compare_exchange_strong_explicit((volatile global atomic_uint*) p, expected, desired, memory_order_relaxed, memory_order_relaxed, memory_scope_device);
}

inline uint atomic_add_global( global uint* p, uint n )
{
    return atomic_fetch_add_explicit( (volatile global atomic_uint*) p, n, memory_order_relaxed, memory_scope_device);
}

inline uint atomic_sub_global(global uint* p, uint n)
{
    return atomic_fetch_sub_explicit((volatile global atomic_uint*) p, n, memory_order_relaxed, memory_scope_device);
}

inline uint atomic_or_global(global uint* p, uint n)
{
    return atomic_fetch_or_explicit((volatile global atomic_uint*) p, n, memory_order_relaxed, memory_scope_device);
}


inline uint atomic_inc_global_acquire(global uint* p)
{
    return atomic_fetch_add_explicit((volatile global atomic_uint*) p, (uint)1, memory_order_acquire, memory_scope_device);
}


inline uint atomic_inc_global_release(global uint* p)
{
    return atomic_fetch_add_explicit((volatile global atomic_uint*) p, (uint)1, memory_order_release, memory_scope_device);
}
inline uint atomic_dec_global_release(global uint* p)
{
    return atomic_fetch_sub_explicit((volatile global atomic_uint*) p, (uint)1, memory_order_release, memory_scope_device);
}

inline uint generic_atomic_add(uint* p, uint val)
{
    if (to_global(p) != NULL)
        return atomic_add_global(to_global(p), val);
    if (to_local(p) != NULL)
        return atomic_add_local(to_local(p), val);
    return 0;
}

inline __attribute__((overloadable)) uint sub_group_reduce_max_N6( uint n )
{
    n = max( n, intel_sub_group_shuffle_down( n, n, 4 ) );
    n = max( n, intel_sub_group_shuffle_down( n, n, 2 ) );
    n = max( n, intel_sub_group_shuffle_down( n, n, 1 ) );
    return sub_group_broadcast( n, 0 );
}

inline __attribute__((overloadable)) float sub_group_reduce_max_N6( float n )
{
    n = max( n, intel_sub_group_shuffle_down( n, n, 4 ) );
    n = max( n, intel_sub_group_shuffle_down( n, n, 2 ) );
    n = max( n, intel_sub_group_shuffle_down( n, n, 1 ) );
    return sub_group_broadcast( n, 0 );
}

inline __attribute__((overloadable)) float sub_group_reduce_max_N6_2xSIMD8_in_SIMD16(float n)
{
    n = max(n, intel_sub_group_shuffle_down(n, n, 4));
    n = max(n, intel_sub_group_shuffle_down(n, n, 2));
    n = max(n, intel_sub_group_shuffle_down(n, n, 1));
    return intel_sub_group_shuffle(n, (get_sub_group_local_id() / 8) * 8);//sub_group_broadcast(n, 0);
}

inline uint generic_atomic_inc(uint* p)
{
    if (to_global(p) != NULL)
        return atomic_inc_global(to_global(p));
    if (to_local(p) != NULL)
        return atomic_inc(to_local(p));
    return 0;
}


// Built-in GRL function which, if called in a kernel body, will force the kernel
//  to be compiled to the minimum SIMD width supported by the platform
void GRL_UseMinimumSIMDWidth();