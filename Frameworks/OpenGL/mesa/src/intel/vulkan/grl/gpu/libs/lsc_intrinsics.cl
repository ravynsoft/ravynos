//
// Copyright (C) 2009-2021 Intel Corporation
//
// SPDX-License-Identifier: MIT
//
//

// LSC Cache options
// Load message caching control
enum LSC_LDCC {
    LSC_LDCC_DEFAULT,
    LSC_LDCC_L1UC_L3UC,     // Override to L1 uncached and L3 uncached
    LSC_LDCC_L1UC_L3C,      // Override to L1 uncached and L3 cached
    LSC_LDCC_L1C_L3UC,      // Override to L1 cached and L3 uncached
    LSC_LDCC_L1C_L3C,       // Override to L1 cached and L3 cached
    LSC_LDCC_L1S_L3UC,      // Override to L1 streaming load and L3 uncached
    LSC_LDCC_L1S_L3C,       // Override to L1 streaming load and L3 cached
    LSC_LDCC_L1IAR_L3C,     // Override to L1 invalidate-after-read, and L3 cached
};

// Store message caching control (also used for atomics)
enum LSC_STCC {
    LSC_STCC_DEFAULT,
    LSC_STCC_L1UC_L3UC,     // Override to L1 uncached and L3 uncached
    LSC_STCC_L1UC_L3WB,     // Override to L1 uncached and L3 written back
    LSC_STCC_L1WT_L3UC,     // Override to L1 written through and L3 uncached
    LSC_STCC_L1WT_L3WB,     // Override to L1 written through and L3 written back
    LSC_STCC_L1S_L3UC,      // Override to L1 streaming and L3 uncached
    LSC_STCC_L1S_L3WB,      // Override to L1 streaming and L3 written back
    LSC_STCC_L1WB_L3WB,     // Override to L1 written through and L3 written back
};

// LSC Loads

// Global address space
uint    __builtin_IB_lsc_load_global_uchar_to_uint (const __global uchar  *base, int immElemOff, enum LSC_LDCC cacheOpt);     //D8U32
uint    __builtin_IB_lsc_load_global_ushort_to_uint(const __global ushort *base, int immElemOff, enum LSC_LDCC cacheOpt);   //D16U32
uint    __builtin_IB_lsc_load_global_uint  (const __global uint   *base, int immElemOff, enum LSC_LDCC cacheOpt);       //D32V1
uint2   __builtin_IB_lsc_load_global_uint2 (const __global uint2  *base, int immElemOff, enum LSC_LDCC cacheOpt);     //D32V2
uint3   __builtin_IB_lsc_load_global_uint3 (const __global uint3  *base, int immElemOff, enum LSC_LDCC cacheOpt);     //D32V3
uint4   __builtin_IB_lsc_load_global_uint4 (const __global uint4  *base, int immElemOff, enum LSC_LDCC cacheOpt);     //D32V4
uint8   __builtin_IB_lsc_load_global_uint8 (const __global uint8  *base, int immElemOff, enum LSC_LDCC cacheOpt);     //D32V8
ulong   __builtin_IB_lsc_load_global_ulong (const __global ulong  *base, int immElemOff, enum LSC_LDCC cacheOpt);    //D64V1
ulong2  __builtin_IB_lsc_load_global_ulong2(const __global ulong2 *base, int immElemOff, enum LSC_LDCC cacheOpt);  //D64V2
ulong3  __builtin_IB_lsc_load_global_ulong3(const __global ulong3 *base, int immElemOff, enum LSC_LDCC cacheOpt);  //D64V3
ulong4  __builtin_IB_lsc_load_global_ulong4(const __global ulong4 *base, int immElemOff, enum LSC_LDCC cacheOpt);  //D64V4
ulong8  __builtin_IB_lsc_load_global_ulong8(const __global ulong8 *base, int immElemOff, enum LSC_LDCC cacheOpt);  //D64V8

// Local address space
uint    __builtin_IB_lsc_load_local_uchar_to_uint( const __local  uchar *base, int immElemOff); //D8U32
uint    __builtin_IB_lsc_load_local_ushort_to_uint(const __local ushort *base, int immElemOff); //D16U32
uint    __builtin_IB_lsc_load_local_uint  (const __local uint   *base, int immElemOff);   //D32V1
uint2   __builtin_IB_lsc_load_local_uint2 (const __local uint2  *base, int immElemOff);  //D32V2
uint3   __builtin_IB_lsc_load_local_uint3 (const __local uint3  *base, int immElemOff);  //D32V3
uint4   __builtin_IB_lsc_load_local_uint4 (const __local uint4  *base, int immElemOff);  //D32V4
uint8   __builtin_IB_lsc_load_local_uint8 (const __local uint8  *base, int immElemOff);  //D32V8
ulong   __builtin_IB_lsc_load_local_ulong (const __local ulong  *base, int immElemOff);  //D64V1
ulong2  __builtin_IB_lsc_load_local_ulong2(const __local ulong2 *base, int immElemOff); //D64V2
ulong3  __builtin_IB_lsc_load_local_ulong3(const __local ulong3 *base, int immElemOff); //D64V3
ulong4  __builtin_IB_lsc_load_local_ulong4(const __local ulong4 *base, int immElemOff); //D64V4
ulong8  __builtin_IB_lsc_load_local_ulong8(const __local ulong8 *base, int immElemOff); //D64V8

// LSC Stores

// Global address space
void  __builtin_IB_lsc_store_global_uchar_from_uint (__global uchar  *base, int immElemOff, uint val, enum LSC_STCC cacheOpt);     //D8U32
void  __builtin_IB_lsc_store_global_ushort_from_uint(__global ushort *base, int immElemOff, uint val, enum LSC_STCC cacheOpt);  //D16U32
void  __builtin_IB_lsc_store_global_uint  (__global uint   *base, int immElemOff, uint val, enum LSC_STCC cacheOpt);        //D32V1
void  __builtin_IB_lsc_store_global_uint2 (__global uint2  *base, int immElemOff, uint2 val, enum LSC_STCC cacheOpt);     //D32V2
void  __builtin_IB_lsc_store_global_uint3 (__global uint3  *base, int immElemOff, uint3 val, enum LSC_STCC cacheOpt);     //D32V3
void  __builtin_IB_lsc_store_global_uint4 (__global uint4  *base, int immElemOff, uint4 val, enum LSC_STCC cacheOpt);     //D32V4
void  __builtin_IB_lsc_store_global_uint8 (__global uint8  *base, int immElemOff, uint8 val, enum LSC_STCC cacheOpt);     //D32V8
void  __builtin_IB_lsc_store_global_ulong (__global ulong  *base, int immElemOff, ulong val, enum LSC_STCC cacheOpt);     //D64V1
void  __builtin_IB_lsc_store_global_ulong2(__global ulong2 *base, int immElemOff, ulong2 val, enum LSC_STCC cacheOpt);  //D64V2
void  __builtin_IB_lsc_store_global_ulong3(__global ulong3 *base, int immElemOff, ulong3 val, enum LSC_STCC cacheOpt);  //D64V3
void  __builtin_IB_lsc_store_global_ulong4(__global ulong4 *base, int immElemOff, ulong4 val, enum LSC_STCC cacheOpt);  //D64V4
void  __builtin_IB_lsc_store_global_ulong8(__global ulong8 *base, int immElemOff, ulong8 val, enum LSC_STCC cacheOpt);  //D64V8

// Local address space
void  __builtin_IB_lsc_store_local_uchar_from_uint (__local  uchar *base, int immElemOff, uint val);   //D8U32
void  __builtin_IB_lsc_store_local_ushort_from_uint(__local ushort *base, int immElemOff, uint val); //D16U32
void  __builtin_IB_lsc_store_local_uint  (__local uint   *base, int immElemOff, uint val);   //D32V1
void  __builtin_IB_lsc_store_local_uint2 (__local uint2  *base, int immElemOff, uint2 val);  //D32V2
void  __builtin_IB_lsc_store_local_uint3 (__local uint3  *base, int immElemOff, uint3 val);  //D32V3
void  __builtin_IB_lsc_store_local_uint4 (__local uint4  *base, int immElemOff, uint4 val);  //D32V4
void  __builtin_IB_lsc_store_local_uint8 (__local uint8  *base, int immElemOff, uint8 val);  //D32V8
void  __builtin_IB_lsc_store_local_ulong (__local ulong  *base, int immElemOff, ulong val);  //D64V1
void  __builtin_IB_lsc_store_local_ulong2(__local ulong2 *base, int immElemOff, ulong2 val);  //D64V2
void  __builtin_IB_lsc_store_local_ulong3(__local ulong3 *base, int immElemOff, ulong3 val);  //D64V3
void  __builtin_IB_lsc_store_local_ulong4(__local ulong4 *base, int immElemOff, ulong4 val);  //D64V4
void  __builtin_IB_lsc_store_local_ulong8(__local ulong8 *base, int immElemOff, ulong8 val);  //D64V8

// LSC prefetching

// LSC Pre-Fetch Load functions with CacheControls
// Global address space
void __builtin_IB_lsc_prefetch_global_uchar (const __global uchar  *base, int immElemOff, enum LSC_LDCC cacheOpt); //D8U32
void __builtin_IB_lsc_prefetch_global_ushort(const __global ushort *base, int immElemOff, enum LSC_LDCC cacheOpt); //D16U32
void __builtin_IB_lsc_prefetch_global_uint  (const __global uint   *base, int immElemOff, enum LSC_LDCC cacheOpt); //D32V1
void __builtin_IB_lsc_prefetch_global_uint2 (const __global uint2  *base, int immElemOff, enum LSC_LDCC cacheOpt); //D32V2
void __builtin_IB_lsc_prefetch_global_uint3 (const __global uint3  *base, int immElemOff, enum LSC_LDCC cacheOpt); //D32V3
void __builtin_IB_lsc_prefetch_global_uint4 (const __global uint4  *base, int immElemOff, enum LSC_LDCC cacheOpt); //D32V4
void __builtin_IB_lsc_prefetch_global_uint8 (const __global uint8  *base, int immElemOff, enum LSC_LDCC cacheOpt); //D32V8
void __builtin_IB_lsc_prefetch_global_ulong (const __global ulong  *base, int immElemOff, enum LSC_LDCC cacheOpt); //D64V1
void __builtin_IB_lsc_prefetch_global_ulong2(const __global ulong2 *base, int immElemOff, enum LSC_LDCC cacheOpt); //D64V2
void __builtin_IB_lsc_prefetch_global_ulong3(const __global ulong3 *base, int immElemOff, enum LSC_LDCC cacheOpt); //D64V3
void __builtin_IB_lsc_prefetch_global_ulong4(const __global ulong4 *base, int immElemOff, enum LSC_LDCC cacheOpt); //D64V4
void __builtin_IB_lsc_prefetch_global_ulong8(const __global ulong8 *base, int immElemOff, enum LSC_LDCC cacheOpt); //D64V8

// LSC Fence support

// FS - Fence Scope
enum LSC_FS {
    LSC_FS_THREAD_GROUP,
    LSC_FS_LOCAL,
    LSC_FS_TILE,
    LSC_FS_GPU,
    LSC_FS_GPUs,
    LSC_FS_SYSTEM_RELEASE,
    LSC_FS_SYSTEM_ACQUIRE
};

// FT - Fence Type
enum LSC_FT {
    LSC_FT_DEFAULT,
    LSC_FT_EVICT,
    LSC_FT_INVALIDATE,
    LSC_FT_DISCARD,
    LSC_FT_CLEAN,
    LSC_FT_L3
};

// LSC Fence functions
void  __builtin_IB_lsc_fence_global_untyped(enum LSC_FS scope, enum LSC_FT flushType);   // Mem Port - UGM
void  __builtin_IB_lsc_fence_global_untyped_cross_tile(enum LSC_FS scope, enum LSC_FT flushType);  // Mem Port - UGML
void  __builtin_IB_lsc_fence_global_typed(enum LSC_FS scope, enum LSC_FT flushType);     // Mem Port - TGM
void  __builtin_IB_lsc_fence_local();                                                    // Mem Port - SLM

// Exported functions

// LSC Loads
// uchar
uint load_uchar_to_uint_L1UC_L3UC(global uchar* it, int offset)
{
    return __builtin_IB_lsc_load_global_uchar_to_uint(it, offset, LSC_LDCC_L1UC_L3UC);
}

uint load_uchar_to_uint_L1UC_L3C(global uchar* it, int offset)
{
    return __builtin_IB_lsc_load_global_uchar_to_uint(it, offset, LSC_LDCC_L1UC_L3C);
}

uint load_uchar_to_uint_L1C_L3UC(global uchar* it, int offset)
{
    return __builtin_IB_lsc_load_global_uchar_to_uint(it, offset, LSC_LDCC_L1C_L3UC);
}

uint load_uchar_to_uint_L1C_L3C(global uchar* it, int offset)
{
    return __builtin_IB_lsc_load_global_uchar_to_uint(it, offset, LSC_LDCC_L1C_L3C);
}

uint load_uchar_to_uint_L1S_L3UC(global uchar* it, int offset)
{
    return __builtin_IB_lsc_load_global_uchar_to_uint(it, offset, LSC_LDCC_L1S_L3UC);
}

uint load_uchar_to_uint_L1S_L3C(global uchar* it, int offset)
{
    return __builtin_IB_lsc_load_global_uchar_to_uint(it, offset, LSC_LDCC_L1S_L3C);
}

uint load_uchar_to_uint_L1IAR_L3C(global uchar* it, int offset)
{
    return __builtin_IB_lsc_load_global_uchar_to_uint(it, offset, LSC_LDCC_L1IAR_L3C);
}

// ushort
uint load_ushort_to_uint_L1UC_L3UC(global ushort* it, int offset)
{
    return __builtin_IB_lsc_load_global_ushort_to_uint(it, offset, LSC_LDCC_L1UC_L3UC);
}

uint load_ushort_to_uint_L1UC_L3C(global ushort* it, int offset)
{
    return __builtin_IB_lsc_load_global_ushort_to_uint(it, offset, LSC_LDCC_L1UC_L3C);
}

uint load_ushort_to_uint_L1C_L3UC(global ushort* it, int offset)
{
    return __builtin_IB_lsc_load_global_ushort_to_uint(it, offset, LSC_LDCC_L1C_L3UC);
}

uint load_ushort_to_uint_L1C_L3C(global ushort* it, int offset)
{
    return __builtin_IB_lsc_load_global_ushort_to_uint(it, offset, LSC_LDCC_L1C_L3C);
}

uint load_ushort_to_uint_L1S_L3UC(global ushort* it, int offset)
{
    return __builtin_IB_lsc_load_global_ushort_to_uint(it, offset, LSC_LDCC_L1S_L3UC);
}

uint load_ushort_to_uint_L1S_L3C(global ushort* it, int offset)
{
    return __builtin_IB_lsc_load_global_ushort_to_uint(it, offset, LSC_LDCC_L1S_L3C);
}

uint load_ushort_to_uint_L1IAR_L3C(global ushort* it, int offset)
{
    return __builtin_IB_lsc_load_global_ushort_to_uint(it, offset, LSC_LDCC_L1IAR_L3C);
}

// uint
uint load_uint_L1UC_L3UC(global uint* it, int offset)
{
    return __builtin_IB_lsc_load_global_uint(it, offset, LSC_LDCC_L1UC_L3UC);
}

uint load_uint_L1UC_L3C(global uint* it, int offset)
{
    return __builtin_IB_lsc_load_global_uint(it, offset, LSC_LDCC_L1UC_L3C);
}

uint load_uint_L1C_L3UC(global uint* it, int offset)
{
    return __builtin_IB_lsc_load_global_uint(it, offset, LSC_LDCC_L1C_L3UC);
}

uint load_uint_L1C_L3C(global uint* it, int offset)
{
    return __builtin_IB_lsc_load_global_uint(it, offset, LSC_LDCC_L1C_L3C);
}

uint load_uint_L1S_L3UC(global uint* it, int offset)
{
    return __builtin_IB_lsc_load_global_uint(it, offset, LSC_LDCC_L1S_L3UC);
}

uint load_uint_L1S_L3C(global uint* it, int offset)
{
    return __builtin_IB_lsc_load_global_uint(it, offset, LSC_LDCC_L1S_L3C);
}

uint load_uint_L1IAR_L3C(global uint* it, int offset)
{
    return __builtin_IB_lsc_load_global_uint(it, offset, LSC_LDCC_L1IAR_L3C);
}

// uint2
uint2 load_uint2_L1UC_L3UC(global uint2* it, int offset)
{
    return __builtin_IB_lsc_load_global_uint2(it, offset, LSC_LDCC_L1UC_L3UC);
}

uint2 load_uint2_L1UC_L3C(global uint2* it, int offset)
{
    return __builtin_IB_lsc_load_global_uint2(it, offset, LSC_LDCC_L1UC_L3C);
}

uint2 load_uint2_L1C_L3UC(global uint2* it, int offset)
{
    return __builtin_IB_lsc_load_global_uint2(it, offset, LSC_LDCC_L1C_L3UC);
}

uint2 load_uint2_L1C_L3C(global uint2* it, int offset)
{
    return __builtin_IB_lsc_load_global_uint2(it, offset, LSC_LDCC_L1C_L3C);
}

uint2 load_uint2_L1S_L3UC(global uint2* it, int offset)
{
    return __builtin_IB_lsc_load_global_uint2(it, offset, LSC_LDCC_L1S_L3UC);
}

uint2 load_uint2_L1S_L3C(global uint2* it, int offset)
{
    return __builtin_IB_lsc_load_global_uint2(it, offset, LSC_LDCC_L1S_L3C);
}

uint2 load_uint2_L1IAR_L3C(global uint2* it, int offset)
{
    return __builtin_IB_lsc_load_global_uint2(it, offset, LSC_LDCC_L1IAR_L3C);
}

// uint3
uint3 load_uint3_L1UC_L3UC(global uint3* it, int offset)
{
    return __builtin_IB_lsc_load_global_uint3(it, offset, LSC_LDCC_L1UC_L3UC);
}

uint3 load_uint3_L1UC_L3C(global uint3* it, int offset)
{
    return __builtin_IB_lsc_load_global_uint3(it, offset, LSC_LDCC_L1UC_L3C);
}

uint3 load_uint3_L1C_L3UC(global uint3* it, int offset)
{
    return __builtin_IB_lsc_load_global_uint3(it, offset, LSC_LDCC_L1C_L3UC);
}

uint3 load_uint3_L1C_L3C(global uint3* it, int offset)
{
    return __builtin_IB_lsc_load_global_uint3(it, offset, LSC_LDCC_L1C_L3C);
}

uint3 load_uint3_L1S_L3UC(global uint3* it, int offset)
{
    return __builtin_IB_lsc_load_global_uint3(it, offset, LSC_LDCC_L1S_L3UC);
}

uint3 load_uint3_L1S_L3C(global uint3* it, int offset)
{
    return __builtin_IB_lsc_load_global_uint3(it, offset, LSC_LDCC_L1S_L3C);
}

uint3 load_uint3_L1IAR_L3C(global uint3* it, int offset)
{
    return __builtin_IB_lsc_load_global_uint3(it, offset, LSC_LDCC_L1IAR_L3C);
}

// uint4
uint4 load_uint4_L1UC_L3UC(global uint4* it, int offset)
{
    return __builtin_IB_lsc_load_global_uint4(it, offset, LSC_LDCC_L1UC_L3UC);
}

uint4 load_uint4_L1UC_L3C(global uint4* it, int offset)
{
    return __builtin_IB_lsc_load_global_uint4(it, offset, LSC_LDCC_L1UC_L3C);
}

uint4 load_uint4_L1C_L3UC(global uint4* it, int offset)
{
    return __builtin_IB_lsc_load_global_uint4(it, offset, LSC_LDCC_L1C_L3UC);
}

uint4 load_uint4_L1C_L3C(global uint4* it, int offset)
{
    return __builtin_IB_lsc_load_global_uint4(it, offset, LSC_LDCC_L1C_L3C);
}

uint4 load_uint4_L1S_L3UC(global uint4* it, int offset)
{
    return __builtin_IB_lsc_load_global_uint4(it, offset, LSC_LDCC_L1S_L3UC);
}

uint4 load_uint4_L1S_L3C(global uint4* it, int offset)
{
    return __builtin_IB_lsc_load_global_uint4(it, offset, LSC_LDCC_L1S_L3C);
}

uint4 load_uint4_L1IAR_L3C(global uint4* it, int offset)
{
    return __builtin_IB_lsc_load_global_uint4(it, offset, LSC_LDCC_L1IAR_L3C);
}

// uint8
uint8 load_uint8_L1UC_L3UC(global uint8* it, int offset)
{
    return __builtin_IB_lsc_load_global_uint8(it, offset, LSC_LDCC_L1UC_L3UC);
}

uint8 load_uint8_L1UC_L3C(global uint8* it, int offset)
{
    return __builtin_IB_lsc_load_global_uint8(it, offset, LSC_LDCC_L1UC_L3C);
}

uint8 load_uint8_L1C_L3UC(global uint8* it, int offset)
{
    return __builtin_IB_lsc_load_global_uint8(it, offset, LSC_LDCC_L1C_L3UC);
}

uint8 load_uint8_L1C_L3C(global uint8* it, int offset)
{
    return __builtin_IB_lsc_load_global_uint8(it, offset, LSC_LDCC_L1C_L3C);
}

uint8 load_uint8_L1S_L3UC(global uint8* it, int offset)
{
    return __builtin_IB_lsc_load_global_uint8(it, offset, LSC_LDCC_L1S_L3UC);
}

uint8 load_uint8_L1S_L3C(global uint8* it, int offset)
{
    return __builtin_IB_lsc_load_global_uint8(it, offset, LSC_LDCC_L1S_L3C);
}

uint8 load_uint8_L1IAR_L3C(global uint8* it, int offset)
{
    return __builtin_IB_lsc_load_global_uint8(it, offset, LSC_LDCC_L1IAR_L3C);
}

// ulong
ulong load_ulong_L1UC_L3UC(global ulong* it, int offset)
{
    return __builtin_IB_lsc_load_global_ulong(it, offset, LSC_LDCC_L1UC_L3UC);
}

ulong load_ulong_L1UC_L3C(global ulong* it, int offset)
{
    return __builtin_IB_lsc_load_global_ulong(it, offset, LSC_LDCC_L1UC_L3C);
}

ulong load_ulong_L1C_L3UC(global ulong* it, int offset)
{
    return __builtin_IB_lsc_load_global_ulong(it, offset, LSC_LDCC_L1C_L3UC);
}

ulong load_ulong_L1C_L3C(global ulong* it, int offset)
{
    return __builtin_IB_lsc_load_global_ulong(it, offset, LSC_LDCC_L1C_L3C);
}

ulong load_ulong_L1S_L3UC(global ulong* it, int offset)
{
    return __builtin_IB_lsc_load_global_ulong(it, offset, LSC_LDCC_L1S_L3UC);
}

ulong load_ulong_L1S_L3C(global ulong* it, int offset)
{
    return __builtin_IB_lsc_load_global_ulong(it, offset, LSC_LDCC_L1S_L3C);
}

ulong load_ulong_L1IAR_L3C(global ulong* it, int offset)
{
    return __builtin_IB_lsc_load_global_ulong(it, offset, LSC_LDCC_L1IAR_L3C);
}

// ulong2
ulong2 load_ulong2_L1UC_L3UC(global ulong2* it, int offset)
{
    return __builtin_IB_lsc_load_global_ulong2(it, offset, LSC_LDCC_L1UC_L3UC);
}

ulong2 load_ulong2_L1UC_L3C(global ulong2* it, int offset)
{
    return __builtin_IB_lsc_load_global_ulong2(it, offset, LSC_LDCC_L1UC_L3C);
}

ulong2 load_ulong2_L1C_L3UC(global ulong2* it, int offset)
{
    return __builtin_IB_lsc_load_global_ulong2(it, offset, LSC_LDCC_L1C_L3UC);
}

ulong2 load_ulong2_L1C_L3C(global ulong2* it, int offset)
{
    return __builtin_IB_lsc_load_global_ulong2(it, offset, LSC_LDCC_L1C_L3C);
}

ulong2 load_ulong2_L1S_L3UC(global ulong2* it, int offset)
{
    return __builtin_IB_lsc_load_global_ulong2(it, offset, LSC_LDCC_L1S_L3UC);
}

ulong2 load_ulong2_L1S_L3C(global ulong2* it, int offset)
{
    return __builtin_IB_lsc_load_global_ulong2(it, offset, LSC_LDCC_L1S_L3C);
}

ulong2 load_ulong2_L1IAR_L3C(global ulong2* it, int offset)
{
    return __builtin_IB_lsc_load_global_ulong2(it, offset, LSC_LDCC_L1IAR_L3C);
}

// ulong3
ulong3 load_ulong3_L1UC_L3UC(global ulong3* it, int offset)
{
    return __builtin_IB_lsc_load_global_ulong3(it, offset, LSC_LDCC_L1UC_L3UC);
}

ulong3 load_ulong3_L1UC_L3C(global ulong3* it, int offset)
{
    return __builtin_IB_lsc_load_global_ulong3(it, offset, LSC_LDCC_L1UC_L3C);
}

ulong3 load_ulong3_L1C_L3UC(global ulong3* it, int offset)
{
    return __builtin_IB_lsc_load_global_ulong3(it, offset, LSC_LDCC_L1C_L3UC);
}

ulong3 load_ulong3_L1C_L3C(global ulong3* it, int offset)
{
    return __builtin_IB_lsc_load_global_ulong3(it, offset, LSC_LDCC_L1C_L3C);
}

ulong3 load_ulong3_L1S_L3UC(global ulong3* it, int offset)
{
    return __builtin_IB_lsc_load_global_ulong3(it, offset, LSC_LDCC_L1S_L3UC);
}

ulong3 load_ulong3_L1S_L3C(global ulong3* it, int offset)
{
    return __builtin_IB_lsc_load_global_ulong3(it, offset, LSC_LDCC_L1S_L3C);
}

ulong3 load_ulong3_L1IAR_L3C(global ulong3* it, int offset)
{
    return __builtin_IB_lsc_load_global_ulong3(it, offset, LSC_LDCC_L1IAR_L3C);
}

// ulong4
ulong4 load_ulong4_L1UC_L3UC(global ulong4* it, int offset)
{
    return __builtin_IB_lsc_load_global_ulong4(it, offset, LSC_LDCC_L1UC_L3UC);
}

ulong4 load_ulong4_L1UC_L3C(global ulong4* it, int offset)
{
    return __builtin_IB_lsc_load_global_ulong4(it, offset, LSC_LDCC_L1UC_L3C);
}

ulong4 load_ulong4_L1C_L3UC(global ulong4* it, int offset)
{
    return __builtin_IB_lsc_load_global_ulong4(it, offset, LSC_LDCC_L1C_L3UC);
}

ulong4 load_ulong4_L1C_L3C(global ulong4* it, int offset)
{
    return __builtin_IB_lsc_load_global_ulong4(it, offset, LSC_LDCC_L1C_L3C);
}

ulong4 load_ulong4_L1S_L3UC(global ulong4* it, int offset)
{
    return __builtin_IB_lsc_load_global_ulong4(it, offset, LSC_LDCC_L1S_L3UC);
}

ulong4 load_ulong4_L1S_L3C(global ulong4* it, int offset)
{
    return __builtin_IB_lsc_load_global_ulong4(it, offset, LSC_LDCC_L1S_L3C);
}

ulong4 load_ulong4_L1IAR_L3C(global ulong4* it, int offset)
{
    return __builtin_IB_lsc_load_global_ulong4(it, offset, LSC_LDCC_L1IAR_L3C);
}

// ulong8
ulong8 load_ulong8_L1UC_L3UC(global ulong8* it, int offset)
{
    return __builtin_IB_lsc_load_global_ulong8(it, offset, LSC_LDCC_L1UC_L3UC);
}

ulong8 load_ulong8_L1UC_L3C(global ulong8* it, int offset)
{
    return __builtin_IB_lsc_load_global_ulong8(it, offset, LSC_LDCC_L1UC_L3C);
}

ulong8 load_ulong8_L1C_L3UC(global ulong8* it, int offset)
{
    return __builtin_IB_lsc_load_global_ulong8(it, offset, LSC_LDCC_L1C_L3UC);
}

ulong8 load_ulong8_L1C_L3C(global ulong8* it, int offset)
{
    return __builtin_IB_lsc_load_global_ulong8(it, offset, LSC_LDCC_L1C_L3C);
}

ulong8 load_ulong8_L1S_L3UC(global ulong8* it, int offset)
{
    return __builtin_IB_lsc_load_global_ulong8(it, offset, LSC_LDCC_L1S_L3UC);
}

ulong8 load_ulong8_L1S_L3C(global ulong8* it, int offset)
{
    return __builtin_IB_lsc_load_global_ulong8(it, offset, LSC_LDCC_L1S_L3C);
}

ulong8 load_ulong8_L1IAR_L3C(global ulong8* it, int offset)
{
    return __builtin_IB_lsc_load_global_ulong8(it, offset, LSC_LDCC_L1IAR_L3C);
}

// LSC Stores
// uchar
void store_uchar_from_uint_L1UC_L3UC(global uchar* it, int offset, uint value)
{
    __builtin_IB_lsc_store_global_uchar_from_uint(it, offset, value, LSC_STCC_L1UC_L3UC);
}

void store_uchar_from_uint_L1UC_L3WB(global uchar* it, int offset, uint value)
{
    __builtin_IB_lsc_store_global_uchar_from_uint(it, offset, value, LSC_STCC_L1UC_L3WB);
}

void store_uchar_from_uint_L1WT_L3UC(global uchar* it, int offset, uint value)
{
    __builtin_IB_lsc_store_global_uchar_from_uint(it, offset, value, LSC_STCC_L1WT_L3UC);
}

void store_uchar_from_uint_L1WT_L3WB(global uchar* it, int offset, uint value)
{
    __builtin_IB_lsc_store_global_uchar_from_uint(it, offset, value, LSC_STCC_L1WT_L3WB);
}

void store_uchar_from_uint_L1S_L3UC(global uchar* it, int offset, uint value)
{
    __builtin_IB_lsc_store_global_uchar_from_uint(it, offset, value, LSC_STCC_L1S_L3UC);
}

void store_uchar_from_uint_L1S_L3WB(global uchar* it, int offset, uint value)
{
    __builtin_IB_lsc_store_global_uchar_from_uint(it, offset, value, LSC_STCC_L1S_L3WB);
}

void store_uchar_from_uint_L1WB_L3WB(global uchar* it, int offset, uint value)
{
    __builtin_IB_lsc_store_global_uchar_from_uint(it, offset, value, LSC_STCC_L1WB_L3WB);
}

// ushort
void store_ushort_from_uint_L1UC_L3UC(global ushort* it, int offset, uint value)
{
    __builtin_IB_lsc_store_global_ushort_from_uint(it, offset, value, LSC_STCC_L1UC_L3UC);
}

void store_ushort_from_uint_L1UC_L3WB(global ushort* it, int offset, uint value)
{
    __builtin_IB_lsc_store_global_ushort_from_uint(it, offset, value, LSC_STCC_L1UC_L3WB);
}

void store_ushort_from_uint_L1WT_L3UC(global ushort* it, int offset, uint value)
{
    __builtin_IB_lsc_store_global_ushort_from_uint(it, offset, value, LSC_STCC_L1WT_L3UC);
}

void store_ushort_from_uint_L1WT_L3WB(global ushort* it, int offset, uint value)
{
    __builtin_IB_lsc_store_global_ushort_from_uint(it, offset, value, LSC_STCC_L1WT_L3WB);
}

void store_ushort_from_uint_L1S_L3UC(global ushort* it, int offset, uint value)
{
    __builtin_IB_lsc_store_global_ushort_from_uint(it, offset, value, LSC_STCC_L1S_L3UC);
}

void store_ushort_from_uint_L1S_L3WB(global ushort* it, int offset, uint value)
{
    __builtin_IB_lsc_store_global_ushort_from_uint(it, offset, value, LSC_STCC_L1S_L3WB);
}

void store_ushort_from_uint_L1WB_L3WB(global ushort* it, int offset, uint value)
{
    __builtin_IB_lsc_store_global_ushort_from_uint(it, offset, value, LSC_STCC_L1WB_L3WB);
}

// uint
void store_uint_L1UC_L3UC(global uint* it, int offset, uint value)
{
    __builtin_IB_lsc_store_global_uint(it, offset, value, LSC_STCC_L1UC_L3UC);
}

void store_uint_L1UC_L3WB(global uint* it, int offset, uint value)
{
    __builtin_IB_lsc_store_global_uint(it, offset, value, LSC_STCC_L1UC_L3WB);
}

void store_uint_L1WT_L3UC(global uint* it, int offset, uint value)
{
    __builtin_IB_lsc_store_global_uint(it, offset, value, LSC_STCC_L1WT_L3UC);
}

void store_uint_L1WT_L3WB(global uint* it, int offset, uint value)
{
    __builtin_IB_lsc_store_global_uint(it, offset, value, LSC_STCC_L1WT_L3WB);
}

void store_uint_L1S_L3UC(global uint* it, int offset, uint value)
{
    __builtin_IB_lsc_store_global_uint(it, offset, value, LSC_STCC_L1S_L3UC);
}

void store_uint_L1S_L3WB(global uint* it, int offset, uint value)
{
    __builtin_IB_lsc_store_global_uint(it, offset, value, LSC_STCC_L1S_L3WB);
}

void store_uint_L1WB_L3WB(global uint* it, int offset, uint value)
{
    __builtin_IB_lsc_store_global_uint(it, offset, value, LSC_STCC_L1WB_L3WB);
}

// uint2
void store_uint2_L1UC_L3UC(global uint2* it, int offset, uint2 value)
{
    __builtin_IB_lsc_store_global_uint2(it, offset, value, LSC_STCC_L1UC_L3UC);
}

void store_uint2_L1UC_L3WB(global uint2* it, int offset, uint2 value)
{
    __builtin_IB_lsc_store_global_uint2(it, offset, value, LSC_STCC_L1UC_L3WB);
}

void store_uint2_L1WT_L3UC(global uint2* it, int offset, uint2 value)
{
    __builtin_IB_lsc_store_global_uint2(it, offset, value, LSC_STCC_L1WT_L3UC);
}

void store_uint2_L1WT_L3WB(global uint2* it, int offset, uint2 value)
{
    __builtin_IB_lsc_store_global_uint2(it, offset, value, LSC_STCC_L1WT_L3WB);
}

void store_uint2_L1S_L3UC(global uint2* it, int offset, uint2 value)
{
    __builtin_IB_lsc_store_global_uint2(it, offset, value, LSC_STCC_L1S_L3UC);
}

void store_uint2_L1S_L3WB(global uint2* it, int offset, uint2 value)
{
    __builtin_IB_lsc_store_global_uint2(it, offset, value, LSC_STCC_L1S_L3WB);
}

void store_uint2_L1WB_L3WB(global uint2* it, int offset, uint2 value)
{
    __builtin_IB_lsc_store_global_uint2(it, offset, value, LSC_STCC_L1WB_L3WB);
}

// uint3
void store_uint3_L1UC_L3UC(global uint3* it, int offset, uint3 value)
{
    __builtin_IB_lsc_store_global_uint3(it, offset, value, LSC_STCC_L1UC_L3UC);
}

void store_uint3_L1UC_L3WB(global uint3* it, int offset, uint3 value)
{
    __builtin_IB_lsc_store_global_uint3(it, offset, value, LSC_STCC_L1UC_L3WB);
}

void store_uint3_L1WT_L3UC(global uint3* it, int offset, uint3 value)
{
    __builtin_IB_lsc_store_global_uint3(it, offset, value, LSC_STCC_L1WT_L3UC);
}

void store_uint3_L1WT_L3WB(global uint3* it, int offset, uint3 value)
{
    __builtin_IB_lsc_store_global_uint3(it, offset, value, LSC_STCC_L1WT_L3WB);
}

void store_uint3_L1S_L3UC(global uint3* it, int offset, uint3 value)
{
    __builtin_IB_lsc_store_global_uint3(it, offset, value, LSC_STCC_L1S_L3UC);
}

void store_uint3_L1S_L3WB(global uint3* it, int offset, uint3 value)
{
    __builtin_IB_lsc_store_global_uint3(it, offset, value, LSC_STCC_L1S_L3WB);
}

void store_uint3_L1WB_L3WB(global uint3* it, int offset, uint3 value)
{
    __builtin_IB_lsc_store_global_uint3(it, offset, value, LSC_STCC_L1WB_L3WB);
}

// uint4
void store_uint4_L1UC_L3UC(global uint4* it, int offset, uint4 value)
{
    __builtin_IB_lsc_store_global_uint4(it, offset, value, LSC_STCC_L1UC_L3UC);
}

void store_uint4_L1UC_L3WB(global uint4* it, int offset, uint4 value)
{
    __builtin_IB_lsc_store_global_uint4(it, offset, value, LSC_STCC_L1UC_L3WB);
}

void store_uint4_L1WT_L3UC(global uint4* it, int offset, uint4 value)
{
    __builtin_IB_lsc_store_global_uint4(it, offset, value, LSC_STCC_L1WT_L3UC);
}

void store_uint4_L1WT_L3WB(global uint4* it, int offset, uint4 value)
{
    __builtin_IB_lsc_store_global_uint4(it, offset, value, LSC_STCC_L1WT_L3WB);
}

void store_uint4_L1S_L3UC(global uint4* it, int offset, uint4 value)
{
    __builtin_IB_lsc_store_global_uint4(it, offset, value, LSC_STCC_L1S_L3UC);
}

void store_uint4_L1S_L3WB(global uint4* it, int offset, uint4 value)
{
    __builtin_IB_lsc_store_global_uint4(it, offset, value, LSC_STCC_L1S_L3WB);
}

void store_uint4_L1WB_L3WB(global uint4* it, int offset, uint4 value)
{
    __builtin_IB_lsc_store_global_uint4(it, offset, value, LSC_STCC_L1WB_L3WB);
}

// uint8
void store_uint8_L1UC_L3UC(global uint8* it, int offset, uint8 value)
{
    __builtin_IB_lsc_store_global_uint8(it, offset, value, LSC_STCC_L1UC_L3UC);
}

void store_uint8_L1UC_L3WB(global uint8* it, int offset, uint8 value)
{
    __builtin_IB_lsc_store_global_uint8(it, offset, value, LSC_STCC_L1UC_L3WB);
}

void store_uint8_L1WT_L3UC(global uint8* it, int offset, uint8 value)
{
    __builtin_IB_lsc_store_global_uint8(it, offset, value, LSC_STCC_L1WT_L3UC);
}

void store_uint8_L1WT_L3WB(global uint8* it, int offset, uint8 value)
{
    __builtin_IB_lsc_store_global_uint8(it, offset, value, LSC_STCC_L1WT_L3WB);
}

void store_uint8_L1S_L3UC(global uint8* it, int offset, uint8 value)
{
    __builtin_IB_lsc_store_global_uint8(it, offset, value, LSC_STCC_L1S_L3UC);
}

void store_uint8_L1S_L3WB(global uint8* it, int offset, uint8 value)
{
    __builtin_IB_lsc_store_global_uint8(it, offset, value, LSC_STCC_L1S_L3WB);
}

void store_uint8_L1WB_L3WB(global uint8* it, int offset, uint8 value)
{
    __builtin_IB_lsc_store_global_uint8(it, offset, value, LSC_STCC_L1WB_L3WB);
}

// ulong
void store_ulong_L1UC_L3UC(global ulong* it, int offset, ulong value)
{
    __builtin_IB_lsc_store_global_ulong(it, offset, value, LSC_STCC_L1UC_L3UC);
}

void store_ulong_L1UC_L3WB(global ulong* it, int offset, ulong value)
{
    __builtin_IB_lsc_store_global_ulong(it, offset, value, LSC_STCC_L1UC_L3WB);
}

void store_ulong_L1WT_L3UC(global ulong* it, int offset, ulong value)
{
    __builtin_IB_lsc_store_global_ulong(it, offset, value, LSC_STCC_L1WT_L3UC);
}

void store_ulong_L1WT_L3WB(global ulong* it, int offset, ulong value)
{
    __builtin_IB_lsc_store_global_ulong(it, offset, value, LSC_STCC_L1WT_L3WB);
}

void store_ulong_L1S_L3UC(global ulong* it, int offset, ulong value)
{
    __builtin_IB_lsc_store_global_ulong(it, offset, value, LSC_STCC_L1S_L3UC);
}

void store_ulong_L1S_L3WB(global ulong* it, int offset, ulong value)
{
    __builtin_IB_lsc_store_global_ulong(it, offset, value, LSC_STCC_L1S_L3WB);
}

void store_ulong_L1WB_L3WB(global ulong* it, int offset, ulong value)
{
    __builtin_IB_lsc_store_global_ulong(it, offset, value, LSC_STCC_L1WB_L3WB);
}

// ulong2
void store_ulong2_L1UC_L3UC(global ulong2* it, int offset, ulong2 value)
{
    __builtin_IB_lsc_store_global_ulong2(it, offset, value, LSC_STCC_L1UC_L3UC);
}

void store_ulong2_L1UC_L3WB(global ulong2* it, int offset, ulong2 value)
{
    __builtin_IB_lsc_store_global_ulong2(it, offset, value, LSC_STCC_L1UC_L3WB);
}

void store_ulong2_L1WT_L3UC(global ulong2* it, int offset, ulong2 value)
{
    __builtin_IB_lsc_store_global_ulong2(it, offset, value, LSC_STCC_L1WT_L3UC);
}

void store_ulong2_L1WT_L3WB(global ulong2* it, int offset, ulong2 value)
{
    __builtin_IB_lsc_store_global_ulong2(it, offset, value, LSC_STCC_L1WT_L3WB);
}

void store_ulong2_L1S_L3UC(global ulong2* it, int offset, ulong2 value)
{
    __builtin_IB_lsc_store_global_ulong2(it, offset, value, LSC_STCC_L1S_L3UC);
}

void store_ulong2_L1S_L3WB(global ulong2* it, int offset, ulong2 value)
{
    __builtin_IB_lsc_store_global_ulong2(it, offset, value, LSC_STCC_L1S_L3WB);
}

void store_ulong2_L1WB_L3WB(global ulong2* it, int offset, ulong2 value)
{
    __builtin_IB_lsc_store_global_ulong2(it, offset, value, LSC_STCC_L1WB_L3WB);
}

// ulong3
void store_ulong3_L1UC_L3UC(global ulong3* it, int offset, ulong3 value)
{
    __builtin_IB_lsc_store_global_ulong3(it, offset, value, LSC_STCC_L1UC_L3UC);
}

void store_ulong3_L1UC_L3WB(global ulong3* it, int offset, ulong3 value)
{
    __builtin_IB_lsc_store_global_ulong3(it, offset, value, LSC_STCC_L1UC_L3WB);
}

void store_ulong3_L1WT_L3UC(global ulong3* it, int offset, ulong3 value)
{
    __builtin_IB_lsc_store_global_ulong3(it, offset, value, LSC_STCC_L1WT_L3UC);
}

void store_ulong3_L1WT_L3WB(global ulong3* it, int offset, ulong3 value)
{
    __builtin_IB_lsc_store_global_ulong3(it, offset, value, LSC_STCC_L1WT_L3WB);
}

void store_ulong3_L1S_L3UC(global ulong3* it, int offset, ulong3 value)
{
    __builtin_IB_lsc_store_global_ulong3(it, offset, value, LSC_STCC_L1S_L3UC);
}

void store_ulong3_L1S_L3WB(global ulong3* it, int offset, ulong3 value)
{
    __builtin_IB_lsc_store_global_ulong3(it, offset, value, LSC_STCC_L1S_L3WB);
}

void store_ulong3_L1WB_L3WB(global ulong3* it, int offset, ulong3 value)
{
    __builtin_IB_lsc_store_global_ulong3(it, offset, value, LSC_STCC_L1WB_L3WB);
}

// ulong4
void store_ulong4_L1UC_L3UC(global ulong4* it, int offset, ulong4 value)
{
    __builtin_IB_lsc_store_global_ulong4(it, offset, value, LSC_STCC_L1UC_L3UC);
}

void store_ulong4_L1UC_L3WB(global ulong4* it, int offset, ulong4 value)
{
    __builtin_IB_lsc_store_global_ulong4(it, offset, value, LSC_STCC_L1UC_L3WB);
}

void store_ulong4_L1WT_L3UC(global ulong4* it, int offset, ulong4 value)
{
    __builtin_IB_lsc_store_global_ulong4(it, offset, value, LSC_STCC_L1WT_L3UC);
}

void store_ulong4_L1WT_L3WB(global ulong4* it, int offset, ulong4 value)
{
    __builtin_IB_lsc_store_global_ulong4(it, offset, value, LSC_STCC_L1WT_L3WB);
}

void store_ulong4_L1S_L3UC(global ulong4* it, int offset, ulong4 value)
{
    __builtin_IB_lsc_store_global_ulong4(it, offset, value, LSC_STCC_L1S_L3UC);
}

void store_ulong4_L1S_L3WB(global ulong4* it, int offset, ulong4 value)
{
    __builtin_IB_lsc_store_global_ulong4(it, offset, value, LSC_STCC_L1S_L3WB);
}

void store_ulong4_L1WB_L3WB(global ulong4* it, int offset, ulong4 value)
{
    __builtin_IB_lsc_store_global_ulong4(it, offset, value, LSC_STCC_L1WB_L3WB);
}

// ulong8
void store_ulong8_L1UC_L3UC(global ulong8* it, int offset, ulong8 value)
{
    __builtin_IB_lsc_store_global_ulong8(it, offset, value, LSC_STCC_L1UC_L3UC);
}

void store_ulong8_L1UC_L3WB(global ulong8* it, int offset, ulong8 value)
{
    __builtin_IB_lsc_store_global_ulong8(it, offset, value, LSC_STCC_L1UC_L3WB);
}

void store_ulong8_L1WT_L3UC(global ulong8* it, int offset, ulong8 value)
{
    __builtin_IB_lsc_store_global_ulong8(it, offset, value, LSC_STCC_L1WT_L3UC);
}

void store_ulong8_L1WT_L3WB(global ulong8* it, int offset, ulong8 value)
{
    __builtin_IB_lsc_store_global_ulong8(it, offset, value, LSC_STCC_L1WT_L3WB);
}

void store_ulong8_L1S_L3UC(global ulong8* it, int offset, ulong8 value)
{
    __builtin_IB_lsc_store_global_ulong8(it, offset, value, LSC_STCC_L1S_L3UC);
}

void store_ulong8_L1S_L3WB(global ulong8* it, int offset, ulong8 value)
{
    __builtin_IB_lsc_store_global_ulong8(it, offset, value, LSC_STCC_L1S_L3WB);
}

void store_ulong8_L1WB_L3WB(global ulong8* it, int offset, ulong8 value)
{
    __builtin_IB_lsc_store_global_ulong8(it, offset, value, LSC_STCC_L1WB_L3WB);
}

// LSC Fence support
void mem_fence_gpu_default()
{
    __builtin_IB_lsc_fence_global_untyped(LSC_FS_TILE, LSC_FT_DEFAULT);
}

void mem_fence_workgroup_default()
{
    __builtin_IB_lsc_fence_global_untyped(LSC_FS_THREAD_GROUP, LSC_FT_DEFAULT);
}

void mem_fence_gpu_invalidate()
{
    // NOTE: 'FS_TILE' is used here to avoid DG2 HW bug where L3 is needlessly flushed on a 'GPU' scope fence
    __builtin_IB_lsc_fence_global_untyped(LSC_FS_TILE, LSC_FT_INVALIDATE);
}

void mem_fence_gpu_evict()
{
    __builtin_IB_lsc_fence_global_untyped(LSC_FS_TILE, LSC_FT_EVICT);
}

void mem_fence_evict_to_memory()
{
    __builtin_IB_lsc_fence_global_untyped(LSC_FS_GPU, LSC_FT_EVICT);
    __builtin_IB_lsc_fence_global_untyped(LSC_FS_GPU, LSC_FT_L3);
}
