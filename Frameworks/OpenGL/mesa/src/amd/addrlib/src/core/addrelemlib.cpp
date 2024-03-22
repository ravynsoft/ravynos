/*
************************************************************************************************************************
*
*  Copyright (C) 2007-2022 Advanced Micro Devices, Inc.  All rights reserved.
*  SPDX-License-Identifier: MIT
*
***********************************************************************************************************************/

/**
****************************************************************************************************
* @file  addrelemlib.cpp
* @brief Contains the class implementation for element/pixel related functions.
****************************************************************************************************
*/

#include "addrelemlib.h"
#include "addrlib.h"

namespace Addr
{

/**
****************************************************************************************************
*   ElemLib::ElemLib
*
*   @brief
*       constructor
*
*   @return
*       N/A
****************************************************************************************************
*/
ElemLib::ElemLib(
    Lib* pAddrLib)  ///< [in] Parent addrlib instance pointer
    :
    Object(pAddrLib->GetClient()),
    m_pAddrLib(pAddrLib)
{
    switch (m_pAddrLib->GetChipFamily())
    {
        case ADDR_CHIP_FAMILY_R6XX:
            m_depthPlanarType = ADDR_DEPTH_PLANAR_R600;
            m_fp16ExportNorm = 0;
            break;
        case ADDR_CHIP_FAMILY_R7XX:
            m_depthPlanarType = ADDR_DEPTH_PLANAR_R600;
            m_fp16ExportNorm = 1;
            break;
        case ADDR_CHIP_FAMILY_R8XX:
        case ADDR_CHIP_FAMILY_NI: // Same as 8xx
            m_depthPlanarType = ADDR_DEPTH_PLANAR_R800;
            m_fp16ExportNorm = 1;
            break;
        default:
            m_fp16ExportNorm = 1;
            m_depthPlanarType = ADDR_DEPTH_PLANAR_R800;
            break;
    }

    m_configFlags.value = 0;
}

/**
****************************************************************************************************
*   ElemLib::~ElemLib
*
*   @brief
*       destructor
*
*   @return
*       N/A
****************************************************************************************************
*/
ElemLib::~ElemLib()
{
}

/**
****************************************************************************************************
*   ElemLib::Create
*
*   @brief
*       Creates and initializes AddrLib object.
*
*   @return
*       Returns point to ADDR_CREATEINFO if successful.
****************************************************************************************************
*/
ElemLib* ElemLib::Create(
    const Lib* pAddrLib)   ///< [in] Pointer of parent AddrLib instance
{
    ElemLib* pElemLib = NULL;

    if (pAddrLib)
    {
        VOID* pObj = Object::ClientAlloc(sizeof(ElemLib), pAddrLib->GetClient());
        if (pObj)
        {
            pElemLib = new(pObj) ElemLib(const_cast<Lib* const>(pAddrLib));
        }
    }

    return pElemLib;
}

/**************************************************************************************************
*   ElemLib::Flt32sToInt32s
*
*   @brief
*       Convert a ADDR_FLT_32 value to Int32 value
*
*   @return
*       N/A
****************************************************************************************************
*/
VOID ElemLib::Flt32sToInt32s(
    ADDR_FLT_32     value,      ///< [in] ADDR_FLT_32 value
    UINT_32         bits,       ///< [in] nubmer of bits in value
    NumberType      numberType, ///< [in] the type of number
    UINT_32*        pResult)    ///< [out] Int32 value
{
    UINT_8 round = 128;    //ADDR_ROUND_BY_HALF
    UINT_32 uscale;
    UINT_32 sign;

    //convert each component to an INT_32
    switch ( numberType )
    {
        case ADDR_NO_NUMBER:    //fall through
        case ADDR_ZERO:         //fall through
        case ADDR_ONE:          //fall through
        case ADDR_EPSILON:      //fall through
            return;        // these are zero-bit components, so don't set result

        case ADDR_UINT_BITS:            // unsigned integer bit field, clamped to range
            uscale = (1<<bits) - 1;
            if (bits == 32)               // special case unsigned 32-bit int
            {
                *pResult = value.i;
            }
            else
            {
                if ((value.i < 0) || (value.u > uscale))
                {
                    *pResult = uscale;
                }
                else
                {
                    *pResult = value.i;
                }
                return;
            }

        // The algorithm used in the DB and TX differs at one value for 24-bit unorms
        case ADDR_UNORM_R6XXDB:        // unsigned repeating fraction
            if ((bits==24) && (value.i == 0x33000000))
            {
                *pResult = 1;
                return;
            }              // Else treat like ADDR_UNORM_R6XX

        case ADDR_UNORM_R6XX:            // unsigned repeating fraction
            if (value.f <= 0)
            {
                *pResult = 0;            // first clamp to [0..1]
            }
            else
            {
                if (value.f >= 1)
                {
                     *pResult = (1<<bits) - 1;
                }
                else
                {
                    if ((value.i | 0x87FFFFFF) == 0xFFFFFFFF)
                    {
                        *pResult = 0;                        // NaN, so force to 0
                    }

                    #if 0 // floating point version for documentation
                    else
                    {
                        FLOAT f = value.f * ((1<<bits) - 1);
                        *pResult = static_cast<INT_32>(f + (round/256.0f));
                    }
                    #endif
                    else
                    {
                        ADDR_FLT_32 scaled;
                        ADDR_FLT_32 shifted;
                        UINT_64 truncated, rounded;
                        UINT_32 altShift;
                        UINT_32 mask = (1 << bits) - 1;
                        UINT_32 half = 1 << (bits - 1);
                        UINT_32 mant24 = (value.i & 0x7FFFFF) + 0x800000;
                        UINT_64 temp = mant24 - (mant24>>bits) -
                            static_cast<INT_32>((mant24 & mask) > half);
                        UINT_32 exp8 = value.i >> 23;
                        UINT_32 shift = 126 - exp8 + 24 - bits;
                        UINT_64 final;

                        if (shift >= 32) // This is zero, even with maximum dither add
                        {
                            final = 0;
                        }
                        else
                        {
                            final = ((temp<<8) + (static_cast<UINT_64>(round)<<shift)) >> (shift+8);
                        }
                        //ADDR_EXIT( *pResult == final,
                        //    ("Float %x converted to %d-bit Unorm %x != bitwise %x",
                        //     value.u, bits, (UINT_32)*pResult, (UINT_32)final) );
                        if (final > mask)
                        {
                            final = mask;
                        }

                        scaled.f  = value.f * ((1<<bits) - 1);
                        shifted.f = (scaled.f * 256);
                        truncated = ((shifted.i&0x7FFFFF) + (INT_64)0x800000) << 8;
                        altShift  = 126 + 24 + 8 - ((shifted.i>>23)&0xFF);
                        truncated = (altShift > 60) ? 0 : truncated >> altShift;
                        rounded   = static_cast<INT_32>((round + truncated) >> 8);
                        //if (rounded > ((1<<bits) - 1))
                        //    rounded = ((1<<bits) - 1);
                        *pResult = static_cast<INT_32>(rounded); //(INT_32)final;
                    }
                }
            }

            return;

        case ADDR_S8FLOAT32:    // 32-bit IEEE float, passes through NaN values
            *pResult = value.i;
            return;

        // @@ FIX ROUNDING in this code, fix the denorm case
        case ADDR_U4FLOATC:         // Unsigned float, 4-bit exponent. bias 15, clamped [0..1]
            sign = (value.i >> 31) & 1;
            if ((value.i&0x7F800000) == 0x7F800000)    // If NaN or INF:
            {
                if ((value.i&0x007FFFFF) != 0)             // then if NaN
                {
                    *pResult = 0;                       // return 0
                }
                else
                {
                    *pResult = (sign)?0:0xF00000;           // else +INF->+1, -INF->0
                }
                return;
            }
            if (value.f <= 0)
            {
                *pResult = 0;
            }
            else
            {
                if (value.f>=1)
                {
                    *pResult = 0xF << (bits-4);
                }
                else
                {
                    if ((value.i>>23) > 112 )
                    {
                        // 24-bit float: normalized
                        // value.i += 1 << (22-bits+4);
                        // round the IEEE mantissa to mantissa size
                        // @@ NOTE: add code to support rounding
                        value.u &= 0x7FFFFFF;             // mask off high 4 exponent bits
                        *pResult = value.i >> (23-bits+4);// shift off unused mantissa bits
                    }
                    else
                    {
                        // 24-bit float: denormalized
                        value.f = value.f / (1<<28) / (1<<28);
                        value.f = value.f / (1<<28) / (1<<28);    // convert to IEEE denorm
                        // value.i += 1 << (22-bits+4);
                        // round the IEEE mantissa to mantissa size
                        // @@ NOTE: add code to support rounding
                        *pResult = value.i >> (23-bits+4);    // shift off unused mantissa bits
                    }
                }
            }

            return;

        default:                    // invalid number mode
            //ADDR_EXIT(0, ("Invalid AddrNumber %d", numberType) );
            break;

    }
}

/**
****************************************************************************************************
*   ElemLib::Int32sToPixel
*
*   @brief
*       Pack 32-bit integer values into an uncompressed pixel,
*       in the proper order
*
*   @return
*       N/A
*
*   @note
*       This entry point packes four 32-bit integer values into
*       an uncompressed pixel. The pixel values are specifies in
*       standard order, e.g. depth/stencil. This routine asserts
*       if called on compressed pixel.
****************************************************************************************************
*/
VOID ElemLib::Int32sToPixel(
    UINT_32              numComps,      ///< [in] number of components
    UINT_32*             pComps,        ///< [in] compnents
    UINT_32*             pCompBits,     ///< [in] total bits in each component
    UINT_32*             pCompStart,    ///< [in] the first bit position of each component
    ComponentFlags       properties,    ///< [in] properties about byteAligned, exportNorm
    UINT_32              resultBits,    ///< [in] result bits: total bpp after decompression
    UINT_8*              pPixel)        ///< [out] a depth/stencil pixel value
{
    UINT_32 i;
    UINT_32 j;
    UINT_32 start;
    UINT_32 size;
    UINT_32 byte;
    UINT_32 value = 0;
    UINT_32 compMask;
    UINT_32 elemMask=0;
    UINT_32 elementXor = 0;  // address xor when reading bytes from elements


    // @@ NOTE: assert if called on a compressed format!

    if (properties.byteAligned)    // Components are all byte-sized
    {
        for (i = 0; i < numComps; i++)        // Then for each component
        {
            // Copy the bytes of the component into the element
            start = pCompStart[i] / 8;
            size  = pCompBits[i]  / 8;
            for (j = 0; j < size; j++)
            {
                pPixel[(j+start)^elementXor] = static_cast<UINT_8>(pComps[i] >> (8*j));
            }
        }
    }
    else                        // Element is 32-bits or less, components are bit fields
    {
        // First, extract each component in turn and combine it into a 32-bit value
        for (i = 0; i < numComps; i++)
        {
            compMask = (1 << pCompBits[i]) - 1;
            elemMask |= compMask << pCompStart[i];
            value |= (pComps[i] & compMask) << pCompStart[i];
        }

        // Mext, copy the masked value into the element
        size = (resultBits + 7) / 8;
        for (i = 0; i < size; i++)
        {
            byte = pPixel[i^elementXor] & ~(elemMask >> (8*i));
            pPixel[i^elementXor] = static_cast<UINT_8>(byte | ((elemMask & value) >> (8*i)));
        }
    }
}

/**
****************************************************************************************************
*   Flt32ToDepthPixel
*
*   @brief
*       Convert a FLT_32 value to a depth/stencil pixel value
*
*   @return
*       N/A
****************************************************************************************************
*/
VOID ElemLib::Flt32ToDepthPixel(
    AddrDepthFormat     format,     ///< [in] Depth format
    const ADDR_FLT_32   comps[2],   ///< [in] two components of depth
    UINT_8*             pPixel      ///< [out] depth pixel value
    ) const
{
    UINT_32 i;
    UINT_32 values[2];
    ComponentFlags properties;  // byteAligned, exportNorm
    UINT_32 resultBits = 0;     // result bits: total bits per pixel after decompression

    PixelFormatInfo fmt;

    // get type for each component
    PixGetDepthCompInfo(format, &fmt);

    //initialize properties
    properties.byteAligned = TRUE;
    properties.exportNorm  = TRUE;
    properties.floatComp   = FALSE;

    //set properties and result bits
    for (i = 0; i < 2; i++)
    {
        if ((fmt.compBit[i] & 7) || (fmt.compStart[i] & 7))
        {
            properties.byteAligned = FALSE;
        }

        if (resultBits < fmt.compStart[i] + fmt.compBit[i])
        {
            resultBits = fmt.compStart[i] + fmt.compBit[i];
        }

        // Clear ADDR_EXPORT_NORM if can't be represented as 11-bit or smaller [-1..+1] format
        if (fmt.compBit[i] > 11 || fmt.numType[i] >= ADDR_USCALED)
        {
            properties.exportNorm = FALSE;
        }

        // Mark if there are any floating point components
        if ((fmt.numType[i] == ADDR_U4FLOATC) || (fmt.numType[i] >= ADDR_S8FLOAT) )
        {
            properties.floatComp = TRUE;
        }
    }

    // Convert the two input floats to integer values
    for (i = 0; i < 2; i++)
    {
        Flt32sToInt32s(comps[i], fmt.compBit[i], fmt.numType[i], &values[i]);
    }

    // Then pack the two integer components, in the proper order
    Int32sToPixel(2, values, fmt.compBit, fmt.compStart, properties, resultBits, pPixel );

}

/**
****************************************************************************************************
*   Flt32ToColorPixel
*
*   @brief
*       Convert a FLT_32 value to a red/green/blue/alpha pixel value
*
*   @return
*       N/A
****************************************************************************************************
*/
VOID ElemLib::Flt32ToColorPixel(
    AddrColorFormat     format,     ///< [in] Color format
    AddrSurfaceNumber   surfNum,    ///< [in] Surface number
    AddrSurfaceSwap     surfSwap,   ///< [in] Surface swap
    const ADDR_FLT_32   comps[4],   ///< [in] four components of color
    UINT_8*             pPixel      ///< [out] a red/green/blue/alpha pixel value
    ) const
{
    PixelFormatInfo pixelInfo;

    UINT_32 i;
    UINT_32 values[4];
    ComponentFlags properties;    // byteAligned, exportNorm
    UINT_32 resultBits = 0;       // result bits: total bits per pixel after decompression

    memset(&pixelInfo, 0, sizeof(PixelFormatInfo));

    PixGetColorCompInfo(format, surfNum, surfSwap, &pixelInfo);

    //initialize properties
    properties.byteAligned = TRUE;
    properties.exportNorm  = TRUE;
    properties.floatComp   = FALSE;

    //set properties and result bits
    for (i = 0; i < 4; i++)
    {
        if ( (pixelInfo.compBit[i] & 7) || (pixelInfo.compStart[i] & 7) )
        {
            properties.byteAligned = FALSE;
        }

        if (resultBits < pixelInfo.compStart[i] + pixelInfo.compBit[i])
        {
            resultBits = pixelInfo.compStart[i] + pixelInfo.compBit[i];
        }

        if (m_fp16ExportNorm)
        {
            // Clear ADDR_EXPORT_NORM if can't be represented as 11-bit or smaller [-1..+1] format
            // or if it's not FP and <=16 bits
            if (((pixelInfo.compBit[i] > 11) || (pixelInfo.numType[i] >= ADDR_USCALED))
                && (pixelInfo.numType[i] !=ADDR_U4FLOATC))
            {
                properties.exportNorm = FALSE;
            }
        }
        else
        {
            // Clear ADDR_EXPORT_NORM if can't be represented as 11-bit or smaller [-1..+1] format
            if (pixelInfo.compBit[i] > 11 || pixelInfo.numType[i] >= ADDR_USCALED)
            {
                properties.exportNorm = FALSE;
            }
        }

        // Mark if there are any floating point components
        if ( (pixelInfo.numType[i] == ADDR_U4FLOATC) ||
             (pixelInfo.numType[i] >= ADDR_S8FLOAT) )
        {
            properties.floatComp = TRUE;
        }
    }

    // Convert the four input floats to integer values
    for (i = 0; i < 4; i++)
    {
        Flt32sToInt32s(comps[i], pixelInfo.compBit[i], pixelInfo.numType[i], &values[i]);
    }

    // Then pack the four integer components, in the proper order
    Int32sToPixel(4, values, &pixelInfo.compBit[0], &pixelInfo.compStart[0],
                  properties, resultBits, pPixel);
}

/**
****************************************************************************************************
*   ElemLib::GetCompType
*
*   @brief
*       Fill per component info
*
*   @return
*       N/A
*
****************************************************************************************************
*/
VOID ElemLib::GetCompType(
    AddrColorFormat   format,     ///< [in] surface format
    AddrSurfaceNumber numType,  ///< [in] number type
    PixelFormatInfo*  pInfo)       ///< [in][out] per component info out
{
    BOOL_32 handled = FALSE;

    // Floating point formats override the number format
    switch (format)
    {
        case ADDR_COLOR_16_FLOAT:            // fall through for all pure floating point format
        case ADDR_COLOR_16_16_FLOAT:
        case ADDR_COLOR_16_16_16_16_FLOAT:
        case ADDR_COLOR_32_FLOAT:
        case ADDR_COLOR_32_32_FLOAT:
        case ADDR_COLOR_32_32_32_32_FLOAT:
        case ADDR_COLOR_10_11_11_FLOAT:
        case ADDR_COLOR_11_11_10_FLOAT:
            numType = ADDR_NUMBER_FLOAT;
            break;
            // Special handling for the depth formats
        case ADDR_COLOR_8_24:                // fall through for these 2 similar format
        case ADDR_COLOR_24_8:
            for (UINT_32 c = 0; c < 4; c++)
            {
                if (pInfo->compBit[c] == 8)
                {
                    pInfo->numType[c] = ADDR_UINT_BITS;
                }
                else if (pInfo->compBit[c]  == 24)
                {
                    pInfo->numType[c] = ADDR_UNORM_R6XX;
                }
                else
                {
                    pInfo->numType[c] = ADDR_NO_NUMBER;
                }
            }
            handled = TRUE;
            break;
        case ADDR_COLOR_8_24_FLOAT:          // fall through for these 3 similar format
        case ADDR_COLOR_24_8_FLOAT:
        case ADDR_COLOR_X24_8_32_FLOAT:
            for (UINT_32 c = 0; c < 4; c++)
            {
                if (pInfo->compBit[c] == 8)
                {
                    pInfo->numType[c] = ADDR_UINT_BITS;
                }
                else if (pInfo->compBit[c] == 24)
                {
                    pInfo->numType[c] = ADDR_U4FLOATC;
                }
                else if (pInfo->compBit[c] == 32)
                {
                    pInfo->numType[c] = ADDR_S8FLOAT32;
                }
                else
                {
                    pInfo->numType[c] = ADDR_NO_NUMBER;
                }
            }
            handled = TRUE;
            break;
        default:
            break;
    }

    if (!handled)
    {
        for (UINT_32 c = 0; c < 4; c++)
        {
            // Assign a number type for each component
            AddrSurfaceNumber cnum;

            // First handle default component values
            if (pInfo->compBit[c] == 0)
            {
                if (c < 3)
                {
                    pInfo->numType[c] = ADDR_ZERO;      // Default is zero for RGB
                }
                else if (numType == ADDR_NUMBER_UINT || numType == ADDR_NUMBER_SINT)
                {
                    pInfo->numType[c] = ADDR_EPSILON;   // Alpha INT_32 bits default is 0x01
                }
                else
                {
                    pInfo->numType[c] = ADDR_ONE;       // Alpha normal default is float 1.0
                }
                continue;
            }
            // Now handle small components
            else if (pInfo->compBit[c] == 1)
            {
                if (numType == ADDR_NUMBER_UINT || numType == ADDR_NUMBER_SINT)
                {
                    cnum = ADDR_NUMBER_UINT;
                }
                else
                {
                    cnum = ADDR_NUMBER_UNORM;
                }
            }
            else
            {
                cnum = numType;
            }

            // If no default, set the number type fom num, compbits, and architecture
            switch (cnum)
            {
                case ADDR_NUMBER_SRGB:
                    pInfo->numType[c] = (c < 3) ? ADDR_GAMMA8_R6XX : ADDR_UNORM_R6XX;
                    break;
                case ADDR_NUMBER_UNORM:
                    pInfo->numType[c] = ADDR_UNORM_R6XX;
                    break;
                case ADDR_NUMBER_SNORM:
                    pInfo->numType[c] = ADDR_SNORM_R6XX;
                    break;
                case ADDR_NUMBER_USCALED:
                    pInfo->numType[c] = ADDR_USCALED;  // @@ Do we need separate Pele routine?
                    break;
                case ADDR_NUMBER_SSCALED:
                    pInfo->numType[c] = ADDR_SSCALED;  // @@ Do we need separate Pele routine?
                    break;
                case ADDR_NUMBER_FLOAT:
                    if (pInfo->compBit[c] == 32)
                    {
                        pInfo->numType[c] = ADDR_S8FLOAT32;
                    }
                    else if (pInfo->compBit[c] == 16)
                    {
                        pInfo->numType[c] = ADDR_S5FLOAT;
                    }
                    else if (pInfo->compBit[c] >= 10)
                    {
                        pInfo->numType[c] = ADDR_U5FLOAT;
                    }
                    else
                    {
                        ADDR_ASSERT_ALWAYS();
                    }
                    break;
                case ADDR_NUMBER_SINT:
                    pInfo->numType[c] = ADDR_SINT_BITS;
                    break;
                case ADDR_NUMBER_UINT:
                    pInfo->numType[c] = ADDR_UINT_BITS;
                    break;

                default:
                    ADDR_ASSERT(!"Invalid number type");
                    pInfo->numType[c] = ADDR_NO_NUMBER;
                    break;
             }
        }
    }
}

/**
****************************************************************************************************
*   ElemLib::GetCompSwap
*
*   @brief
*       Get components swapped for color surface
*
*   @return
*       N/A
*
****************************************************************************************************
*/
VOID ElemLib::GetCompSwap(
    AddrSurfaceSwap  swap,   ///< [in] swap mode
    PixelFormatInfo* pInfo)  ///< [in,out] output per component info
{
    switch (pInfo->comps)
    {
        case 4:
            switch (swap)
            {
                case ADDR_SWAP_ALT:
                    SwapComps( 0, 2, pInfo );
                    break;    // BGRA
                case ADDR_SWAP_STD_REV:
                    SwapComps( 0, 3, pInfo );
                    SwapComps( 1, 2, pInfo );
                    break;    // ABGR
                case ADDR_SWAP_ALT_REV:
                    SwapComps( 0, 3, pInfo );
                    SwapComps( 0, 2, pInfo );
                    SwapComps( 0, 1, pInfo );
                    break;    // ARGB
                default:
                    break;
            }
            break;
        case 3:
            switch (swap)
            {
                case ADDR_SWAP_ALT_REV:
                    SwapComps( 0, 3, pInfo );
                    SwapComps( 0, 2, pInfo );
                    break;    // AGR
                case ADDR_SWAP_STD_REV:
                    SwapComps( 0, 2, pInfo );
                    break;    // BGR
                case ADDR_SWAP_ALT:
                    SwapComps( 2, 3, pInfo );
                    break;    // RGA
                default:
                    break;    // RGB
            }
            break;
        case 2:
            switch (swap)
            {
                case ADDR_SWAP_ALT_REV:
                    SwapComps( 0, 1, pInfo );
                    SwapComps( 1, 3, pInfo );
                    break;    // AR
                case ADDR_SWAP_STD_REV:
                    SwapComps( 0, 1, pInfo );
                    break;    // GR
                case ADDR_SWAP_ALT:
                    SwapComps( 1, 3, pInfo );
                    break;    // RA
                default:
                    break;    // RG
            }
            break;
        case 1:
            switch (swap)
            {
                case ADDR_SWAP_ALT_REV:
                    SwapComps( 0, 3, pInfo );
                    break;    // A
                case ADDR_SWAP_STD_REV:
                    SwapComps( 0, 2, pInfo );
                    break;    // B
                case ADDR_SWAP_ALT:
                    SwapComps( 0, 1, pInfo );
                    break;    // G
                default:
                    break;    // R
            }
            break;
    }
}

/**
****************************************************************************************************
*   ElemLib::GetCompSwap
*
*   @brief
*       Get components swapped for color surface
*
*   @return
*       N/A
*
****************************************************************************************************
*/
VOID ElemLib::SwapComps(
    UINT_32          c0,     ///< [in] component index 0
    UINT_32          c1,     ///< [in] component index 1
    PixelFormatInfo* pInfo)  ///< [in,out] output per component info
{
    UINT_32 start;
    UINT_32 bits;

    start = pInfo->compStart[c0];
    pInfo->compStart[c0] = pInfo->compStart[c1];
    pInfo->compStart[c1] = start;

    bits  = pInfo->compBit[c0];
    pInfo->compBit[c0] = pInfo->compBit[c1];
    pInfo->compBit[c1] = bits;
}

/**
****************************************************************************************************
*   ElemLib::PixGetColorCompInfo
*
*   @brief
*       Get per component info for color surface
*
*   @return
*       N/A
*
****************************************************************************************************
*/
VOID ElemLib::PixGetColorCompInfo(
    AddrColorFormat   format, ///< [in] surface format, read from register
    AddrSurfaceNumber number, ///< [in] pixel number type
    AddrSurfaceSwap   swap,   ///< [in] component swap mode
    PixelFormatInfo*  pInfo   ///< [out] output per component info
    ) const
{
    // 1. Get componet bits
    switch (format)
    {
        case ADDR_COLOR_8:
            GetCompBits(8, 0, 0, 0, pInfo);
            break;
        case ADDR_COLOR_1_5_5_5:
            GetCompBits(5, 5, 5, 1, pInfo);
            break;
        case ADDR_COLOR_5_6_5:
            GetCompBits(8, 6, 5, 0, pInfo);
            break;
        case ADDR_COLOR_6_5_5:
            GetCompBits(5, 5, 6, 0, pInfo);
            break;
        case ADDR_COLOR_8_8:
            GetCompBits(8, 8, 0, 0, pInfo);
            break;
        case ADDR_COLOR_4_4_4_4:
            GetCompBits(4, 4, 4, 4, pInfo);
            break;
        case ADDR_COLOR_16:
            GetCompBits(16, 0, 0, 0, pInfo);
            break;
        case ADDR_COLOR_8_8_8_8:
            GetCompBits(8, 8, 8, 8, pInfo);
            break;
        case ADDR_COLOR_2_10_10_10:
            GetCompBits(10, 10, 10, 2, pInfo);
            break;
        case ADDR_COLOR_10_11_11:
            GetCompBits(11, 11, 10, 0, pInfo);
            break;
        case ADDR_COLOR_11_11_10:
            GetCompBits(10, 11, 11, 0, pInfo);
            break;
        case ADDR_COLOR_16_16:
            GetCompBits(16, 16, 0, 0, pInfo);
            break;
        case ADDR_COLOR_16_16_16_16:
            GetCompBits(16, 16, 16, 16, pInfo);
            break;
        case ADDR_COLOR_16_FLOAT:
            GetCompBits(16, 0, 0, 0, pInfo);
            break;
        case ADDR_COLOR_16_16_FLOAT:
            GetCompBits(16, 16, 0, 0, pInfo);
            break;
        case ADDR_COLOR_32_FLOAT:
            GetCompBits(32, 0, 0, 0, pInfo);
            break;
        case ADDR_COLOR_32_32_FLOAT:
            GetCompBits(32, 32, 0, 0, pInfo);
            break;
        case ADDR_COLOR_16_16_16_16_FLOAT:
            GetCompBits(16, 16, 16, 16, pInfo);
            break;
        case ADDR_COLOR_32_32_32_32_FLOAT:
            GetCompBits(32, 32, 32, 32, pInfo);
            break;

        case ADDR_COLOR_32:
            GetCompBits(32, 0, 0, 0, pInfo);
            break;
        case ADDR_COLOR_32_32:
            GetCompBits(32, 32, 0, 0, pInfo);
            break;
        case ADDR_COLOR_32_32_32_32:
            GetCompBits(32, 32, 32, 32, pInfo);
            break;
        case ADDR_COLOR_10_10_10_2:
            GetCompBits(2, 10, 10, 10, pInfo);
            break;
        case ADDR_COLOR_10_11_11_FLOAT:
            GetCompBits(11, 11, 10, 0, pInfo);
            break;
        case ADDR_COLOR_11_11_10_FLOAT:
            GetCompBits(10, 11, 11, 0, pInfo);
            break;
        case ADDR_COLOR_5_5_5_1:
            GetCompBits(1, 5, 5, 5, pInfo);
            break;
        case ADDR_COLOR_3_3_2:
            GetCompBits(2, 3, 3, 0, pInfo);
            break;
        case ADDR_COLOR_4_4:
            GetCompBits(4, 4, 0, 0, pInfo);
            break;
        case ADDR_COLOR_8_24:
        case ADDR_COLOR_8_24_FLOAT:  // same bit count, fall through
            GetCompBits(24, 8, 0, 0, pInfo);
            break;
        case ADDR_COLOR_24_8:
        case ADDR_COLOR_24_8_FLOAT:  // same bit count, fall through
            GetCompBits(8, 24, 0, 0, pInfo);
            break;
        case ADDR_COLOR_X24_8_32_FLOAT:
            GetCompBits(32, 8, 0, 0, pInfo);
            break;

        case ADDR_COLOR_INVALID:
            GetCompBits(0, 0, 0, 0, pInfo);
            break;
        default:
            ADDR_ASSERT(0);
            GetCompBits(0, 0, 0, 0, pInfo);
            break;
    }

    // 2. Get component number type

    GetCompType(format, number, pInfo);

    // 3. Swap components if needed

    GetCompSwap(swap, pInfo);
}

/**
****************************************************************************************************
*   ElemLib::PixGetDepthCompInfo
*
*   @brief
*       Get per component info for depth surface
*
*   @return
*       N/A
*
****************************************************************************************************
*/
VOID ElemLib::PixGetDepthCompInfo(
    AddrDepthFormat  format,     ///< [in] surface format, read from register
    PixelFormatInfo* pInfo       ///< [out] output per component bits and type
    ) const
{
    if (m_depthPlanarType == ADDR_DEPTH_PLANAR_R800)
    {
        if (format == ADDR_DEPTH_8_24_FLOAT)
        {
            format = ADDR_DEPTH_X24_8_32_FLOAT; // Use this format to represent R800's D24FS8
        }

        if (format == ADDR_DEPTH_X8_24_FLOAT)
        {
            format = ADDR_DEPTH_32_FLOAT;
        }
    }

    switch (format)
    {
        case ADDR_DEPTH_16:
            GetCompBits(16, 0, 0, 0, pInfo);
            break;
        case ADDR_DEPTH_8_24:
        case ADDR_DEPTH_8_24_FLOAT:      // similar format, fall through
            GetCompBits(24, 8, 0, 0, pInfo);
            break;
        case ADDR_DEPTH_X8_24:
        case ADDR_DEPTH_X8_24_FLOAT:     // similar format, fall through
            GetCompBits(24, 0, 0, 0, pInfo);
            break;
        case ADDR_DEPTH_32_FLOAT:
            GetCompBits(32, 0, 0, 0, pInfo);
            break;
        case ADDR_DEPTH_X24_8_32_FLOAT:
            GetCompBits(32, 8, 0, 0, pInfo);
            break;
        case ADDR_DEPTH_INVALID:
            GetCompBits(0, 0, 0, 0, pInfo);
            break;
        default:
            ADDR_ASSERT(0);
            GetCompBits(0, 0, 0, 0, pInfo);
            break;
    }

    switch (format)
    {
        case ADDR_DEPTH_16:
            pInfo->numType [0] = ADDR_UNORM_R6XX;
            pInfo->numType [1] = ADDR_ZERO;
            break;
        case ADDR_DEPTH_8_24:
            pInfo->numType [0] = ADDR_UNORM_R6XXDB;
            pInfo->numType [1] = ADDR_UINT_BITS;
            break;
        case ADDR_DEPTH_8_24_FLOAT:
            pInfo->numType [0] = ADDR_U4FLOATC;
            pInfo->numType [1] = ADDR_UINT_BITS;
            break;
        case ADDR_DEPTH_X8_24:
            pInfo->numType [0] = ADDR_UNORM_R6XXDB;
            pInfo->numType [1] = ADDR_ZERO;
            break;
        case ADDR_DEPTH_X8_24_FLOAT:
            pInfo->numType [0] = ADDR_U4FLOATC;
            pInfo->numType [1] = ADDR_ZERO;
            break;
        case ADDR_DEPTH_32_FLOAT:
            pInfo->numType [0] = ADDR_S8FLOAT32;
            pInfo->numType [1] = ADDR_ZERO;
            break;
        case ADDR_DEPTH_X24_8_32_FLOAT:
            pInfo->numType [0] = ADDR_S8FLOAT32;
            pInfo->numType [1] = ADDR_UINT_BITS;
            break;
        default:
            pInfo->numType [0] = ADDR_NO_NUMBER;
            pInfo->numType [1] = ADDR_NO_NUMBER;
            break;
    }

    pInfo->numType [2] = ADDR_NO_NUMBER;
    pInfo->numType [3] = ADDR_NO_NUMBER;
}

/**
****************************************************************************************************
*   ElemLib::PixGetExportNorm
*
*   @brief
*       Check if fp16 export norm can be enabled.
*
*   @return
*       TRUE if this can be enabled.
*
****************************************************************************************************
*/
BOOL_32 ElemLib::PixGetExportNorm(
    AddrColorFormat     colorFmt,       ///< [in] surface format, read from register
    AddrSurfaceNumber   numberFmt,      ///< [in] pixel number type
    AddrSurfaceSwap     swap            ///< [in] components swap type
    ) const
{
    BOOL_32 enabled = TRUE;

    PixelFormatInfo formatInfo;

    PixGetColorCompInfo(colorFmt, numberFmt, swap, &formatInfo);

    for (UINT_32 c = 0; c < 4; c++)
    {
        if (m_fp16ExportNorm)
        {
            if (((formatInfo.compBit[c] > 11) || (formatInfo.numType[c] > ADDR_USCALED)) &&
                (formatInfo.numType[c] != ADDR_U4FLOATC)    &&
                (formatInfo.numType[c] != ADDR_S5FLOAT)     &&
                (formatInfo.numType[c] != ADDR_S5FLOATM)    &&
                (formatInfo.numType[c] != ADDR_U5FLOAT)     &&
                (formatInfo.numType[c] != ADDR_U3FLOATM))
            {
                enabled = FALSE;
                break;
            }
        }
        else
        {
            if ((formatInfo.compBit[c] > 11) || (formatInfo.numType[c] > ADDR_USCALED))
            {
                enabled = FALSE;
                break;
            }
        }
    }

    return enabled;
}

/**
****************************************************************************************************
*   ElemLib::AdjustSurfaceInfo
*
*   @brief
*       Adjust bpp/base pitch/width/height according to elemMode and expandX/Y
*
*   @return
*       N/A
****************************************************************************************************
*/
VOID ElemLib::AdjustSurfaceInfo(
    ElemMode        elemMode,       ///< [in] element mode
    UINT_32         expandX,        ///< [in] decompression expansion factor in X
    UINT_32         expandY,        ///< [in] decompression expansion factor in Y
    UINT_32*        pBpp,           ///< [in,out] bpp
    UINT_32*        pBasePitch,     ///< [in,out] base pitch
    UINT_32*        pWidth,         ///< [in,out] width
    UINT_32*        pHeight)        ///< [in,out] height
{
    UINT_32 packedBits;
    UINT_32 basePitch;
    UINT_32 width;
    UINT_32 height;
    UINT_32 bpp;
    BOOL_32 bBCnFormat = FALSE;

    ADDR_ASSERT(pBpp != NULL);
    ADDR_ASSERT(pWidth != NULL && pHeight != NULL && pBasePitch != NULL);

    if (pBpp)
    {
        bpp = *pBpp;

        switch (elemMode)
        {
            case ADDR_EXPANDED:
                packedBits = bpp / expandX / expandY;
                break;
            case ADDR_PACKED_STD: // Different bit order
            case ADDR_PACKED_REV:
                packedBits = bpp * expandX * expandY;
                break;
            case ADDR_PACKED_GBGR:
            case ADDR_PACKED_BGRG:
                packedBits = bpp; // 32-bit packed ==> 2 32-bit result
                break;
            case ADDR_PACKED_BC1: // Fall through
            case ADDR_PACKED_BC4:
                packedBits = 64;
                bBCnFormat = TRUE;
                break;
            case ADDR_PACKED_BC2: // Fall through
            case ADDR_PACKED_BC3: // Fall through
            case ADDR_PACKED_BC5: // Fall through
                bBCnFormat = TRUE;
                // fall through
            case ADDR_PACKED_ASTC:
            case ADDR_PACKED_ETC2_128BPP:
                packedBits = 128;
                break;
            case ADDR_PACKED_ETC2_64BPP:
                packedBits = 64;
                break;
            case ADDR_ROUND_BY_HALF:  // Fall through
            case ADDR_ROUND_TRUNCATE: // Fall through
            case ADDR_ROUND_DITHER:   // Fall through
            case ADDR_UNCOMPRESSED:
                packedBits = bpp;
                break;
            default:
                packedBits = bpp;
                ADDR_ASSERT_ALWAYS();
                break;
        }

        *pBpp = packedBits;
    }

    if (pWidth && pHeight && pBasePitch)
    {
        basePitch = *pBasePitch;
        width     = *pWidth;
        height    = *pHeight;

        if ((expandX > 1) || (expandY > 1))
        {
            if (elemMode == ADDR_EXPANDED)
            {
                basePitch *= expandX;
                width     *= expandX;
                height    *= expandY;
            }
            else
            {
                // Evergreen family workaround
                if (bBCnFormat && (m_pAddrLib->GetChipFamily() == ADDR_CHIP_FAMILY_R8XX))
                {
                    // For BCn we now pad it to POW2 at the beginning so it is safe to
                    // divide by 4 directly
                    basePitch = basePitch / expandX;
                    width     = width  / expandX;
                    height    = height / expandY;
#if DEBUG
                    width     = (width == 0) ? 1 : width;
                    height    = (height == 0) ? 1 : height;

                    if ((*pWidth > PowTwoAlign(width, 8) * expandX) ||
                        (*pHeight > PowTwoAlign(height, 8) * expandY)) // 8 is 1D tiling alignment
                    {
                        // if this assertion is hit we may have issues if app samples
                        // rightmost/bottommost pixels
                        ADDR_ASSERT_ALWAYS();
                    }
#endif
                }
                else // Not BCn format we still keep old way (FMT_1? No real test yet)
                {
                    basePitch = (basePitch + expandX - 1) / expandX;
                    width     = (width + expandX - 1) / expandX;
                    height    = (height + expandY - 1) / expandY;
                }
            }

            *pBasePitch = basePitch; // 0 is legal value for base pitch.
            *pWidth     = (width == 0) ? 1 : width;
            *pHeight    = (height == 0) ? 1 : height;
        } //if (pWidth && pHeight && pBasePitch)
    }
}

/**
****************************************************************************************************
*   ElemLib::RestoreSurfaceInfo
*
*   @brief
*       Reverse operation of AdjustSurfaceInfo
*
*   @return
*       N/A
****************************************************************************************************
*/
VOID ElemLib::RestoreSurfaceInfo(
    ElemMode        elemMode,       ///< [in] element mode
    UINT_32         expandX,        ///< [in] decompression expansion factor in X
    UINT_32         expandY,        ///< [out] decompression expansion factor in Y
    UINT_32*        pBpp,           ///< [in,out] bpp
    UINT_32*        pWidth,         ///< [in,out] width
    UINT_32*        pHeight)        ///< [in,out] height
{
    UINT_32 originalBits;
    UINT_32 width;
    UINT_32 height;
    UINT_32 bpp;

    BOOL_32 bBCnFormat = FALSE;

    ADDR_ASSERT(pBpp != NULL);
    ADDR_ASSERT(pWidth != NULL && pHeight != NULL);

    if (pBpp)
    {
        bpp = *pBpp;

        switch (elemMode)
        {
        case ADDR_EXPANDED:
            originalBits = bpp * expandX * expandY;
            break;
        case ADDR_PACKED_STD: // Different bit order
        case ADDR_PACKED_REV:
            originalBits = bpp / expandX / expandY;
            break;
        case ADDR_PACKED_GBGR:
        case ADDR_PACKED_BGRG:
            originalBits = bpp; // 32-bit packed ==> 2 32-bit result
            break;
        case ADDR_PACKED_BC1: // Fall through
        case ADDR_PACKED_BC4:
            originalBits = 64;
            bBCnFormat = TRUE;
            break;
        case ADDR_PACKED_BC2: // Fall through
        case ADDR_PACKED_BC3: // Fall through
        case ADDR_PACKED_BC5:
            bBCnFormat = TRUE;
            // fall through
        case ADDR_PACKED_ASTC:
        case ADDR_PACKED_ETC2_128BPP:
            originalBits = 128;
            break;
        case ADDR_PACKED_ETC2_64BPP:
            originalBits = 64;
            break;
        case ADDR_ROUND_BY_HALF:  // Fall through
        case ADDR_ROUND_TRUNCATE: // Fall through
        case ADDR_ROUND_DITHER:   // Fall through
        case ADDR_UNCOMPRESSED:
            originalBits = bpp;
            break;
        default:
            originalBits = bpp;
            ADDR_ASSERT_ALWAYS();
            break;
        }

        *pBpp = originalBits;
    }

    if (pWidth && pHeight)
    {
        width    = *pWidth;
        height   = *pHeight;

        if ((expandX > 1) || (expandY > 1))
        {
            if (elemMode == ADDR_EXPANDED)
            {
                width /= expandX;
                height /= expandY;
            }
            else
            {
                width *= expandX;
                height *= expandY;
            }
        }

        *pWidth  = (width == 0) ? 1 : width;
        *pHeight = (height == 0) ? 1 : height;
    }
}

/**
****************************************************************************************************
*   ElemLib::GetBitsPerPixel
*
*   @brief
*       Compute the total bits per element according to a format
*       code. For compressed formats, this is not the same as
*       the number of bits per decompressed element.
*
*   @return
*       Bits per pixel
****************************************************************************************************
*/
UINT_32 ElemLib::GetBitsPerPixel(
    AddrFormat          format,         ///< [in] surface format code
    ElemMode*           pElemMode,      ///< [out] element mode
    UINT_32*            pExpandX,       ///< [out] decompression expansion factor in X
    UINT_32*            pExpandY,       ///< [out] decompression expansion factor in Y
    UINT_32*            pUnusedBits)    ///< [out] bits unused
{
    UINT_32 bpp;
    UINT_32 expandX = 1;
    UINT_32 expandY = 1;
    UINT_32 bitUnused = 0;
    ElemMode elemMode = ADDR_UNCOMPRESSED; // default value

    switch (format)
    {
        case ADDR_FMT_8:
            bpp = 8;
            break;
        case ADDR_FMT_1_5_5_5:
        case ADDR_FMT_5_6_5:
        case ADDR_FMT_6_5_5:
        case ADDR_FMT_8_8:
        case ADDR_FMT_4_4_4_4:
        case ADDR_FMT_16:
            bpp = 16;
            break;
        case ADDR_FMT_GB_GR:
            elemMode = ADDR_PACKED_GBGR;
            bpp      = m_configFlags.use32bppFor422Fmt ? 32 : 16;
            expandX  = m_configFlags.use32bppFor422Fmt ? 2 : 1;
            break;
        case ADDR_FMT_BG_RG:
            elemMode = ADDR_PACKED_BGRG;
            bpp      = m_configFlags.use32bppFor422Fmt ? 32 : 16;
            expandX  = m_configFlags.use32bppFor422Fmt ? 2 : 1;
            break;
        case ADDR_FMT_8_8_8_8:
        case ADDR_FMT_2_10_10_10:
        case ADDR_FMT_10_11_11:
        case ADDR_FMT_11_11_10:
        case ADDR_FMT_16_16:
        case ADDR_FMT_32:
        case ADDR_FMT_24_8:
            bpp = 32;
            break;
        case ADDR_FMT_BG_RG_16_16_16_16:
            elemMode = ADDR_PACKED_BGRG;
            bpp = 32;
            break;
        case ADDR_FMT_16_16_16_16:
        case ADDR_FMT_32_32:
        case ADDR_FMT_CTX1:
            bpp = 64;
            break;
        case ADDR_FMT_32_32_32_32:
            bpp = 128;
            break;
        case ADDR_FMT_INVALID:
            bpp = 0;
            break;
        case ADDR_FMT_1_REVERSED:
            elemMode = ADDR_PACKED_REV;
            expandX = 8;
            bpp = 1;
            break;
        case ADDR_FMT_1:
            elemMode = ADDR_PACKED_STD;
            expandX = 8;
            bpp = 1;
            break;
        case ADDR_FMT_4_4:
        case ADDR_FMT_3_3_2:
            bpp = 8;
            break;
        case ADDR_FMT_5_5_5_1:
            bpp = 16;
            break;
        case ADDR_FMT_32_AS_8:
        case ADDR_FMT_32_AS_8_8:
        case ADDR_FMT_8_24:
        case ADDR_FMT_10_10_10_2:
        case ADDR_FMT_5_9_9_9_SHAREDEXP:
            bpp = 32;
            break;
        case ADDR_FMT_X24_8_32_FLOAT:
            bpp = 64;
            bitUnused = 24;
            break;
        case ADDR_FMT_8_8_8:
            elemMode = ADDR_EXPANDED;
            bpp = 24;//@@ 8;      // read 3 elements per pixel
            expandX = 3;
            break;
        case ADDR_FMT_16_16_16:
            elemMode = ADDR_EXPANDED;
            bpp = 48;//@@ 16;      // read 3 elements per pixel
            expandX = 3;
            break;
        case ADDR_FMT_32_32_32:
            elemMode = ADDR_EXPANDED;
            expandX = 3;
            bpp = 96;//@@ 32;      // read 3 elements per pixel
            break;
        case ADDR_FMT_BC1:
            elemMode = ADDR_PACKED_BC1;
            expandX = 4;
            expandY = 4;
            bpp = 64;
            break;
        case ADDR_FMT_BC4:
            elemMode = ADDR_PACKED_BC4;
            expandX = 4;
            expandY = 4;
            bpp = 64;
            break;
        case ADDR_FMT_BC2:
            elemMode = ADDR_PACKED_BC2;
            expandX = 4;
            expandY = 4;
            bpp = 128;
            break;
        case ADDR_FMT_BC3:
            elemMode = ADDR_PACKED_BC3;
            expandX = 4;
            expandY = 4;
            bpp = 128;
            break;
        case ADDR_FMT_BC5:
        case ADDR_FMT_BC6: // reuse ADDR_PACKED_BC5
        case ADDR_FMT_BC7: // reuse ADDR_PACKED_BC5
            elemMode = ADDR_PACKED_BC5;
            expandX = 4;
            expandY = 4;
            bpp = 128;
            break;

        case ADDR_FMT_ETC2_64BPP:
            elemMode = ADDR_PACKED_ETC2_64BPP;
            expandX  = 4;
            expandY  = 4;
            bpp      = 64;
            break;

        case ADDR_FMT_ETC2_128BPP:
            elemMode = ADDR_PACKED_ETC2_128BPP;
            expandX  = 4;
            expandY  = 4;
            bpp      = 128;
            break;

        case ADDR_FMT_ASTC_4x4:
            elemMode = ADDR_PACKED_ASTC;
            expandX  = 4;
            expandY  = 4;
            bpp      = 128;
            break;

        case ADDR_FMT_ASTC_5x4:
            elemMode = ADDR_PACKED_ASTC;
            expandX  = 5;
            expandY  = 4;
            bpp      = 128;
            break;

        case ADDR_FMT_ASTC_5x5:
            elemMode = ADDR_PACKED_ASTC;
            expandX  = 5;
            expandY  = 5;
            bpp      = 128;
            break;

        case ADDR_FMT_ASTC_6x5:
            elemMode = ADDR_PACKED_ASTC;
            expandX  = 6;
            expandY  = 5;
            bpp      = 128;
            break;

        case ADDR_FMT_ASTC_6x6:
            elemMode = ADDR_PACKED_ASTC;
            expandX  = 6;
            expandY  = 6;
            bpp      = 128;
            break;

        case ADDR_FMT_ASTC_8x5:
            elemMode = ADDR_PACKED_ASTC;
            expandX  = 8;
            expandY  = 5;
            bpp      = 128;
            break;

        case ADDR_FMT_ASTC_8x6:
            elemMode = ADDR_PACKED_ASTC;
            expandX  = 8;
            expandY  = 6;
            bpp      = 128;
            break;

        case ADDR_FMT_ASTC_8x8:
            elemMode = ADDR_PACKED_ASTC;
            expandX  = 8;
            expandY  = 8;
            bpp      = 128;
            break;

        case ADDR_FMT_ASTC_10x5:
            elemMode = ADDR_PACKED_ASTC;
            expandX  = 10;
            expandY  = 5;
            bpp      = 128;
            break;

        case ADDR_FMT_ASTC_10x6:
            elemMode = ADDR_PACKED_ASTC;
            expandX  = 10;
            expandY  = 6;
            bpp      = 128;
            break;

        case ADDR_FMT_ASTC_10x8:
            elemMode = ADDR_PACKED_ASTC;
            expandX  = 10;
            expandY  = 8;
            bpp      = 128;
            break;

        case ADDR_FMT_ASTC_10x10:
            elemMode = ADDR_PACKED_ASTC;
            expandX  = 10;
            expandY  = 10;
            bpp      = 128;
            break;

        case ADDR_FMT_ASTC_12x10:
            elemMode = ADDR_PACKED_ASTC;
            expandX  = 12;
            expandY  = 10;
            bpp      = 128;
            break;

        case ADDR_FMT_ASTC_12x12:
            elemMode = ADDR_PACKED_ASTC;
            expandX  = 12;
            expandY  = 12;
            bpp      = 128;
            break;

        default:
            bpp = 0;
            ADDR_ASSERT_ALWAYS();
            break;
            // @@ or should this be an error?
    }

    SafeAssign(pExpandX, expandX);
    SafeAssign(pExpandY, expandY);
    SafeAssign(pUnusedBits, bitUnused);
    SafeAssign(reinterpret_cast<UINT_32*>(pElemMode), elemMode);

    return bpp;
}

/**
****************************************************************************************************
*   ElemLib::GetCompBits
*
*   @brief
*       Set each component's bit size and bit start. And set element mode and number type
*
*   @return
*       N/A
****************************************************************************************************
*/
VOID ElemLib::GetCompBits(
    UINT_32          c0,        ///< [in] bits of component 0
    UINT_32          c1,        ///< [in] bits of component 1
    UINT_32          c2,        ///< [in] bits of component 2
    UINT_32          c3,        ///< [in] bits of component 3
    PixelFormatInfo* pInfo,     ///< [out] per component info out
    ElemMode         elemMode)  ///< [in] element mode
{
    pInfo->comps = 0;

    pInfo->compBit[0] = c0;
    pInfo->compBit[1] = c1;
    pInfo->compBit[2] = c2;
    pInfo->compBit[3] = c3;

    pInfo->compStart[0] = 0;
    pInfo->compStart[1] = c0;
    pInfo->compStart[2] = c0+c1;
    pInfo->compStart[3] = c0+c1+c2;

    pInfo->elemMode = elemMode;
    // still needed since component swap may depend on number of components
    for (INT i=0; i<4; i++)
    {
        if (pInfo->compBit[i] == 0)
        {
            pInfo->compStart[i]  = 0;       // all null components start at bit 0
            pInfo->numType[i] = ADDR_NO_NUMBER; // and have no number type
        }
        else
        {
            pInfo->comps++;
        }
    }
}

/**
****************************************************************************************************
*   ElemLib::GetCompBits
*
*   @brief
*       Set the clear color (or clear depth/stencil) for a surface
*
*   @note
*       If clearColor is zero, a default clear value is used in place of comps[4].
*       If float32 is set, full precision is used, else the mantissa is reduced to 12-bits
*
*   @return
*       N/A
****************************************************************************************************
*/
VOID ElemLib::SetClearComps(
    ADDR_FLT_32 comps[4],   ///< [in,out] components
    BOOL_32 clearColor,     ///< [in] TRUE if clear color is set (CLEAR_COLOR)
    BOOL_32 float32)        ///< [in] TRUE if float32 component (BLEND_FLOAT32)
{
    INT_32 i;

    // Use default clearvalues if clearColor is disabled
    if (clearColor == FALSE)
    {
        for (i=0; i<3; i++)
        {
            comps[i].f = 0.0;
        }
        comps[3].f = 1.0;
    }

    // Otherwise use the (modified) clear value
    else
    {
        for (i=0; i<4; i++)
        {   // If full precision, use clear value unchanged
            if (float32)
            {
                // Do nothing
                //comps[i] = comps[i];
            }
            // Else if it is a NaN, use the standard NaN value
            else if ((comps[i].u & 0x7FFFFFFF) > 0x7F800000)
            {
                comps[i].u = 0xFFC00000;
            }
            // Else reduce the mantissa precision
            else
            {
                comps[i].u = comps[i].u & 0xFFFFF000;
            }
        }
    }
}

/**
****************************************************************************************************
*   ElemLib::IsBlockCompressed
*
*   @brief
*       TRUE if this is block compressed format
*
*   @note
*
*   @return
*       BOOL_32
****************************************************************************************************
*/
BOOL_32 ElemLib::IsBlockCompressed(
    AddrFormat format)  ///< [in] Format
{
    return (((format >= ADDR_FMT_BC1) && (format <= ADDR_FMT_BC7)) ||
            ((format >= ADDR_FMT_ASTC_4x4) && (format <= ADDR_FMT_ETC2_128BPP)));
}


/**
****************************************************************************************************
*   ElemLib::IsCompressed
*
*   @brief
*       TRUE if this is block compressed format or 1 bit format
*
*   @note
*
*   @return
*       BOOL_32
****************************************************************************************************
*/
BOOL_32 ElemLib::IsCompressed(
    AddrFormat format)  ///< [in] Format
{
    return IsBlockCompressed(format) || format == ADDR_FMT_BC1 || format == ADDR_FMT_BC7;
}

/**
****************************************************************************************************
*   ElemLib::IsExpand3x
*
*   @brief
*       TRUE if this is 3x expand format
*
*   @note
*
*   @return
*       BOOL_32
****************************************************************************************************
*/
BOOL_32 ElemLib::IsExpand3x(
    AddrFormat format)  ///< [in] Format
{
    BOOL_32 is3x = FALSE;

    switch (format)
    {
        case ADDR_FMT_8_8_8:
        case ADDR_FMT_16_16_16:
        case ADDR_FMT_32_32_32:
            is3x = TRUE;
            break;
        default:
            break;
    }

    return is3x;
}

/**
****************************************************************************************************
*   ElemLib::IsMacroPixelPacked
*
*   @brief
*       TRUE if this is a macro-pixel-packed format.
*
*   @note
*
*   @return
*       BOOL_32
****************************************************************************************************
*/
BOOL_32 ElemLib::IsMacroPixelPacked(
    AddrFormat format)  ///< [in] Format
{
    BOOL_32 isMacroPixelPacked = FALSE;

    switch (format)
    {
        case ADDR_FMT_BG_RG:
        case ADDR_FMT_GB_GR:
        case ADDR_FMT_BG_RG_16_16_16_16:
            isMacroPixelPacked = TRUE;
            break;
        default:
            break;
    }

    return isMacroPixelPacked;
}

}
