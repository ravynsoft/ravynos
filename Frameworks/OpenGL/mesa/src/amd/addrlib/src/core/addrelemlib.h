/*
************************************************************************************************************************
*
*  Copyright (C) 2007-2022 Advanced Micro Devices, Inc.  All rights reserved.
*  SPDX-License-Identifier: MIT
*
***********************************************************************************************************************/


/**
****************************************************************************************************
* @file  addrelemlib.h
* @brief Contains the class for element/pixel related functions.
****************************************************************************************************
*/

#ifndef __ELEM_LIB_H__
#define __ELEM_LIB_H__

#include "addrinterface.h"
#include "addrobject.h"
#include "addrcommon.h"

namespace Addr
{

class Lib;

// The masks for property bits within the Properties INT_32
union ComponentFlags
{
    struct
    {
        UINT_32 byteAligned    : 1;    ///< all components are byte aligned
        UINT_32 exportNorm     : 1;    ///< components support R6xx NORM compression
        UINT_32 floatComp      : 1;    ///< there is at least one floating point component
    };

    UINT_32 value;
};

// Copy from legacy lib's NumberType
enum NumberType
{
    // The following number types have the range [-1..1]
    ADDR_NO_NUMBER,         // This component doesn't exist and has no default value
    ADDR_EPSILON,           // Force component value to integer 0x00000001
    ADDR_ZERO,              // Force component value to integer 0x00000000
    ADDR_ONE,               // Force component value to floating point 1.0
    // Above values don't have any bits per component (keep ADDR_ONE the last of these)

    ADDR_UNORM,             // Unsigned normalized (repeating fraction) full precision
    ADDR_SNORM,             // Signed normalized (repeating fraction) full precision
    ADDR_GAMMA,             // Gamma-corrected, full precision

    ADDR_UNORM_R5XXRB,      // Unsigned normalized (repeating fraction) for r5xx RB
    ADDR_SNORM_R5XXRB,      // Signed normalized (repeating fraction) for r5xx RB
    ADDR_GAMMA_R5XXRB,      // Gamma-corrected for r5xx RB (note: unnormalized value)
    ADDR_UNORM_R5XXBC,      // Unsigned normalized (repeating fraction) for r5xx BC
    ADDR_SNORM_R5XXBC,      // Signed normalized (repeating fraction) for r5xx BC
    ADDR_GAMMA_R5XXBC,      // Gamma-corrected for r5xx BC (note: unnormalized value)

    ADDR_UNORM_R6XX,        // Unsigned normalized (repeating fraction) for R6xx
    ADDR_UNORM_R6XXDB,      // Unorms for 24-bit depth: one value differs from ADDR_UNORM_R6XX
    ADDR_SNORM_R6XX,        // Signed normalized (repeating fraction) for R6xx
    ADDR_GAMMA8_R6XX,       // Gamma-corrected for r6xx
    ADDR_GAMMA8_R7XX_TP,    // Gamma-corrected for r7xx TP 12bit unorm 8.4.

    ADDR_U4FLOATC,          // Unsigned float: 4-bit exponent, bias=15, no NaN, clamp [0..1]
    ADDR_GAMMA_4SEG,        // Gamma-corrected, four segment approximation
    ADDR_U0FIXED,           // Unsigned 0.N-bit fixed point

    // The following number types have large ranges (LEAVE ADDR_USCALED first or fix Finish routine)
    ADDR_USCALED,           // Unsigned integer converted to/from floating point
    ADDR_SSCALED,           // Signed integer converted to/from floating point
    ADDR_USCALED_R5XXRB,    // Unsigned integer to/from floating point for r5xx RB
    ADDR_SSCALED_R5XXRB,    // Signed integer to/from floating point for r5xx RB
    ADDR_UINT_BITS,         // Keep in unsigned integer form, clamped to specified range
    ADDR_SINT_BITS,         // Keep in signed integer form, clamped to specified range
    ADDR_UINTBITS,          // @@ remove Keep in unsigned integer form, use modulus to reduce bits
    ADDR_SINTBITS,          // @@ remove Keep in signed integer form, use modulus to reduce bits

    // The following number types and ADDR_U4FLOATC have exponents
    // (LEAVE ADDR_S8FLOAT first or fix Finish routine)
    ADDR_S8FLOAT,           // Signed floating point with 8-bit exponent, bias=127
    ADDR_S8FLOAT32,         // 32-bit IEEE float, passes through NaN values
    ADDR_S5FLOAT,           // Signed floating point with 5-bit exponent, bias=15
    ADDR_S5FLOATM,          // Signed floating point with 5-bit exponent, bias=15, no NaN/Inf
    ADDR_U5FLOAT,           // Signed floating point with 5-bit exponent, bias=15
    ADDR_U3FLOATM,          // Unsigned floating point with 3-bit exponent, bias=3

    ADDR_S5FIXED,           // Signed 5.N-bit fixed point, with rounding

    ADDR_END_NUMBER         // Used for range comparisons
};

// Copy from legacy lib's AddrElement
enum ElemMode
{
    // These formats allow both packing an unpacking
    ADDR_ROUND_BY_HALF,      // add 1/2 and truncate when packing this element
    ADDR_ROUND_TRUNCATE,     // truncate toward 0 for sign/mag, else toward neg
    ADDR_ROUND_DITHER,       // Pack by dithering -- requires (x,y) position

    // These formats only allow unpacking, no packing
    ADDR_UNCOMPRESSED,       // Elements are not compressed: one data element per pixel/texel
    ADDR_EXPANDED,           // Elements are split up and stored in multiple data elements
    ADDR_PACKED_STD,         // Elements are compressed into ExpandX by ExpandY data elements
    ADDR_PACKED_REV,         // Like ADDR_PACKED, but X order of pixels is reverved
    ADDR_PACKED_GBGR,        // Elements are compressed 4:2:2 in G1B_G0R order (high to low)
    ADDR_PACKED_BGRG,        // Elements are compressed 4:2:2 in BG1_RG0 order (high to low)
    ADDR_PACKED_BC1,         // Each data element is uncompressed to a 4x4 pixel/texel array
    ADDR_PACKED_BC2,         // Each data element is uncompressed to a 4x4 pixel/texel array
    ADDR_PACKED_BC3,         // Each data element is uncompressed to a 4x4 pixel/texel array
    ADDR_PACKED_BC4,         // Each data element is uncompressed to a 4x4 pixel/texel array
    ADDR_PACKED_BC5,         // Each data element is uncompressed to a 4x4 pixel/texel array
    ADDR_PACKED_ETC2_64BPP,  // ETC2 formats that use 64bpp to represent each 4x4 block
    ADDR_PACKED_ETC2_128BPP, // ETC2 formats that use 128bpp to represent each 4x4 block
    ADDR_PACKED_ASTC,        // Various ASTC formats, all are 128bpp with varying block sizes

    // These formats provide various kinds of compression
    ADDR_ZPLANE_R5XX,        // Compressed Zplane using r5xx architecture format
    ADDR_ZPLANE_R6XX,        // Compressed Zplane using r6xx architecture format
    //@@ Fill in the compression modes

    ADDR_END_ELEMENT         // Used for range comparisons
};

enum DepthPlanarType
{
    ADDR_DEPTH_PLANAR_NONE = 0, // No plane z/stencl
    ADDR_DEPTH_PLANAR_R600 = 1, // R600 z and stencil planes are store within a tile
    ADDR_DEPTH_PLANAR_R800 = 2, // R800 has separate z and stencil planes
};

/**
****************************************************************************************************
*   PixelFormatInfo
*
*   @brief
*       Per component info
*
****************************************************************************************************
*/
struct PixelFormatInfo
{
    UINT_32             compBit[4];
    NumberType          numType[4];
    UINT_32             compStart[4];
    ElemMode            elemMode;
    UINT_32             comps;          ///< Number of components
};

/**
****************************************************************************************************
* @brief This class contains asic indepentent element related attributes and operations
****************************************************************************************************
*/
class ElemLib : public Object
{
protected:
    ElemLib(Lib* pAddrLib);

public:

    /// Makes this class virtual
    virtual ~ElemLib();

    static ElemLib* Create(
        const Lib* pAddrLib);

    /// The implementation is only for R6xx/R7xx, so make it virtual in case we need for R8xx
    BOOL_32 PixGetExportNorm(
        AddrColorFormat colorFmt,
        AddrSurfaceNumber numberFmt, AddrSurfaceSwap swap) const;

    /// Below method are asic independent, so make them just static.
    /// Remove static if we need different operation in hwl.

    VOID    Flt32ToDepthPixel(
        AddrDepthFormat format, const ADDR_FLT_32 comps[2], UINT_8 *pPixel) const;

    VOID    Flt32ToColorPixel(
        AddrColorFormat format, AddrSurfaceNumber surfNum, AddrSurfaceSwap surfSwap,
        const ADDR_FLT_32 comps[4], UINT_8 *pPixel) const;

    static VOID    Flt32sToInt32s(
        ADDR_FLT_32 value, UINT_32 bits, NumberType numberType, UINT_32* pResult);

    static VOID    Int32sToPixel(
        UINT_32 numComps, UINT_32* pComps, UINT_32* pCompBits, UINT_32* pCompStart,
        ComponentFlags properties, UINT_32 resultBits, UINT_8* pPixel);

    VOID    PixGetColorCompInfo(
        AddrColorFormat format, AddrSurfaceNumber number, AddrSurfaceSwap swap,
        PixelFormatInfo* pInfo) const;

    VOID    PixGetDepthCompInfo(
        AddrDepthFormat format, PixelFormatInfo* pInfo) const;

    UINT_32 GetBitsPerPixel(
        AddrFormat format, ElemMode* pElemMode = NULL,
        UINT_32* pExpandX = NULL, UINT_32* pExpandY = NULL, UINT_32* pBitsUnused = NULL);

    static VOID    SetClearComps(
        ADDR_FLT_32 comps[4], BOOL_32 clearColor, BOOL_32 float32);

    VOID    AdjustSurfaceInfo(
        ElemMode elemMode, UINT_32 expandX, UINT_32 expandY,
        UINT_32* pBpp, UINT_32* pBasePitch, UINT_32* pWidth, UINT_32* pHeight);

    VOID    RestoreSurfaceInfo(
        ElemMode elemMode, UINT_32 expandX, UINT_32 expandY,
        UINT_32* pBpp, UINT_32* pWidth, UINT_32* pHeight);

    /// Checks if depth and stencil are planar inside a tile
    BOOL_32 IsDepthStencilTilePlanar()
    {
        return (m_depthPlanarType == ADDR_DEPTH_PLANAR_R600) ? TRUE : FALSE;
    }

    /// Sets m_configFlags, copied from AddrLib
    VOID    SetConfigFlags(ConfigFlags flags)
    {
        m_configFlags = flags;
    }

    static BOOL_32 IsCompressed(AddrFormat format);
    static BOOL_32 IsBlockCompressed(AddrFormat format);
    static BOOL_32 IsExpand3x(AddrFormat format);
    static BOOL_32 IsMacroPixelPacked(AddrFormat format);

protected:

    static VOID    GetCompBits(
        UINT_32 c0, UINT_32 c1, UINT_32 c2, UINT_32 c3,
        PixelFormatInfo* pInfo,
        ElemMode elemMode = ADDR_ROUND_BY_HALF);

    static VOID    GetCompType(
        AddrColorFormat format, AddrSurfaceNumber numType,
        PixelFormatInfo* pInfo);

    static VOID    GetCompSwap(
        AddrSurfaceSwap swap, PixelFormatInfo* pInfo);

    static VOID    SwapComps(
        UINT_32 c0, UINT_32 c1, PixelFormatInfo* pInfo);

private:

    UINT_32             m_fp16ExportNorm;   ///< If allow FP16 to be reported as EXPORT_NORM
    DepthPlanarType     m_depthPlanarType;

    ConfigFlags         m_configFlags;      ///< Copy of AddrLib's configFlags
    Addr::Lib* const    m_pAddrLib;         ///< Pointer to parent addrlib instance
};

} //Addr

#endif

