//
// Copyright (C) 2009-2021 Intel Corporation
//
// SPDX-License-Identifier: MIT
//
//

#pragma once

#ifdef __OPENCL_VERSION__

typedef uchar  uint8_t;
typedef ushort uint16_t;
typedef uint   uint32_t;
typedef ulong  uint64_t;
typedef char   int8_t;
typedef short  int16_t;
typedef int    int32_t;
typedef long   int64_t;

#else

#include <stdint.h>

typedef uint8_t  uchar;
typedef uint16_t ushort;
typedef uint32_t uint;
typedef uint64_t ulong;

#define __constant
#define __global

typedef struct uint2
{
#ifdef __cplusplus
    uint2() {};
    uint2( uint ix, uint iy ) : x( ix ), y( iy ) {};
#endif
    uint x;
    uint y;
} uint2;

typedef struct uint3
{
#ifdef __cplusplus
    uint3() {};
    uint3( uint ix, uint iy, uint iz ) : x( ix ), y( iy ), z( iz ) {};
#endif
    uint x;
    uint y;
    uint z;
} uint3;

typedef struct int3
{
    int32_t x;
    int32_t y;
    int32_t z;

#ifdef __cplusplus
    int3() {};
    int3(int32_t ix, int32_t iy, int32_t iz) : x(ix), y(iy), z(iz) {};

    int3 operator+(const int32_t i) const { return int3(this->x + i, this->y + i, this->z + i); }
    int3 operator<<(const int32_t i) const { return int3(this->x << i, this->y << i, this->z << i); }
#endif
} int3;

typedef struct int4
{
    int32_t x;
    int32_t y;
    int32_t z;
    int32_t w;

#ifdef __cplusplus
    int4() {};
    int4(int32_t ix, int32_t iy, int32_t iz, int32_t iw) : x(ix), y(iy), z(iz), w(iw) {};

    int4 operator+(const int32_t i) const { return int4(this->x + i, this->y + i, this->z + i, this->w + i); }
    int4 operator-(const int32_t i) const { return int4(this->x - i, this->y - i, this->z - i, this->w - i); }
    int4 operator<<(const int32_t i) const { return int4(this->x << i, this->y << i, this->z << i, this->w << i); }
#endif
} int4;

typedef struct float3
{
    float x;
    float y;
    float z;

#ifdef __cplusplus
    float3(){};
    float3( float ix, float iy, float iz ) : x(ix), y(iy), z(iz){};

    float3 operator+( const float3& f3 ) { return float3( this->x + f3.x, this->y + f3.y, this->z + f3.z ); }
    float3 operator*( const float& f ) { return float3( this->x * f, this->y * f, this->z * f ); }
    float3 operator*( const float3& f3 ) const { return float3(this->x * f3.x, this->y * f3.y, this->z * f3.z); }
    float3 operator-() { return float3(-this->x, -this->y, -this->z); }
    float3 operator-( const float3& f3) { return float3(this->x - f3.x, this->y - f3.y, this->z - f3.z); }
#endif
} float3;

typedef struct float4
{
    float x;
    float y;
    float z;
    float w;

#ifdef __cplusplus
    float4() {};
    float4( float ix, float iy, float iz, float iw ) : x( ix ), y( iy ), z( iz ), w( iw ) {};

    float4 operator+(const float4& f4) const { return float4(this->x + f4.x, this->y + f4.y, this->z + f4.z, this->w + f4.w); }
    float4 operator*(const float4& f4) const { return float4(this->x * f4.x, this->y * f4.y, this->z * f4.z, this->w * f4.w); }
#endif
} float4;

#endif /* ! __OPENCL_VERSION__ */


#ifndef __cplusplus

#define GRL_NAMESPACE_BEGIN(x)
#define GRL_NAMESPACE_END(x)
#define GRL_OVERLOADABLE __attribute((overloadable))
#define GRL_INLINE __attribute__((always_inline)) inline static

#   define enum_uint8(name)   \
        typedef uint8_t name; \
        enum name##_uint32
#   define enum_uint16(name)   \
        typedef uint16_t name; \
        enum name##_uint32
#   define enum_uint32(name)   \
        typedef uint32_t name; \
        enum name##_uint32

#define OCL_BYTE_ALIGN(n) __attribute__ ((aligned (n)))
#define GRL_STATIC_ASSERT(condition,desc)

#else /* C++ */
#ifdef __OPENCL_VERSION__
#error "OpenCL C++ not supported by this header"
#endif

#define GRL_NAMESPACE_BEGIN(x) namespace x {
#define GRL_NAMESPACE_END(x) }
#define GRL_OVERLOADABLE
#define GRL_INLINE inline

#define enum_uint8(N) enum N : uint8_t
#define enum_uint16(N) enum N : uint16_t
#define enum_uint32(N) enum N : uint32_t

#define OCL_BYTE_ALIGN(n)
#define GRL_STATIC_ASSERT(condition,desc) static_assert( condition, desc )

#include <cmath>

inline float3 fmin(float3 a, float3 b)
{
    float3 o = { std::fmin(a.x, b.x), std::fmin(a.y, b.y), std::fmin(a.z, b.z) };
    return o;
}

inline float3 fmax(float3 a, float3 b)
{
    float3 o = { std::fmax(a.x, b.x), std::fmax(a.y, b.y), std::fmax(a.z, b.z) };
    return o;
}

inline float3 operator/(const float3& f3, const float& f) { return float3(f3.x / f, f3.y / f, f3.z / f); }

inline float dot(const float3& a, const float3& b) {
    return a.x * b.x + a.y * b.y + a.z * b.z;
}

inline float as_float(uint32_t i)
{
    union { float f; uint32_t i; } fi;

    fi.i = i;
    return fi.f;
}

inline float3 as_float3(int3 i3)
{
    float3 o = { as_float(i3.x), as_float(i3.y), as_float(i3.z) };
    return o;
}

inline float4 as_float4(int4 i4)
{
    float4 o = { as_float(i4.x), as_float(i4.y), as_float(i4.z), as_float(i4.w) };
    return o;
}

inline float4 convert_float4_rtn(int4 i4)
{
    return float4(static_cast<float>(i4.x), static_cast<float>(i4.y), static_cast<float>(i4.z), static_cast<float>(i4.w));
}

inline float4 convert_float4_rtp(int4 i4)
{
    return convert_float4_rtn(i4);
}

#endif
