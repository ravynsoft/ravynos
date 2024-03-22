/*
************************************************************************************************************************
*
*  Copyright (C) 2007-2022 Advanced Micro Devices, Inc.  All rights reserved.
*  SPDX-License-Identifier: MIT
*
***********************************************************************************************************************/


/**
************************************************************************************************************************
* @file  addrlib2.h
* @brief Contains the Addr::V2::Lib class definition.
************************************************************************************************************************
*/

#ifndef __ADDR2_LIB2_H__
#define __ADDR2_LIB2_H__

#include "addrlib.h"

namespace Addr
{
namespace V2
{

/**
************************************************************************************************************************
* @brief Flags for SwizzleModeTable
************************************************************************************************************************
*/
union SwizzleModeFlags
{
    struct
    {
        // Swizzle mode
        UINT_32 isLinear        : 1;    // Linear

        // Block size
        UINT_32 is256b          : 1;    // Block size is 256B
        UINT_32 is4kb           : 1;    // Block size is 4KB
        UINT_32 is64kb          : 1;    // Block size is 64KB
        UINT_32 isVar           : 1;    // Block size is variable

        UINT_32 isZ             : 1;    // Z order swizzle mode
        UINT_32 isStd           : 1;    // Standard swizzle mode
        UINT_32 isDisp          : 1;    // Display swizzle mode
        UINT_32 isRot           : 1;    // Rotate swizzle mode

        // XOR mode
        UINT_32 isXor           : 1;    // XOR after swizzle if set

        UINT_32 isT             : 1;    // T mode

        // GFX10
        UINT_32 isRtOpt         : 1;    // mode opt for render target

        UINT_32 reserved        : 20;   // Reserved bits
    };

    UINT_32 u32All;
};

struct Dim2d
{
    UINT_32 w;
    UINT_32 h;
};

struct Dim3d
{
    UINT_32 w;
    UINT_32 h;
    UINT_32 d;
};

// Macro define resource block type
enum AddrBlockType
{
    AddrBlockLinear    = 0, // Resource uses linear swizzle mode
    AddrBlockMicro     = 1, // Resource uses 256B block
    AddrBlockThin4KB   = 2, // Resource uses thin 4KB block
    AddrBlockThick4KB  = 3, // Resource uses thick 4KB block
    AddrBlockThin64KB  = 4, // Resource uses thin 64KB block
    AddrBlockThick64KB = 5, // Resource uses thick 64KB block
    AddrBlockThinVar   = 6, // Resource uses thin var block
    AddrBlockThickVar  = 7, // Resource uses thick var block
    AddrBlockMaxTiledType,

    AddrBlockThin256KB  = AddrBlockThinVar,
    AddrBlockThick256KB = AddrBlockThickVar,
};

enum AddrSwSet
{
    AddrSwSetZ = 1 << ADDR_SW_Z,
    AddrSwSetS = 1 << ADDR_SW_S,
    AddrSwSetD = 1 << ADDR_SW_D,
    AddrSwSetR = 1 << ADDR_SW_R,

    AddrSwSetAll = AddrSwSetZ | AddrSwSetS | AddrSwSetD | AddrSwSetR,
};

const UINT_32 Size256 = 256u;
const UINT_32 Size4K  = 4096u;
const UINT_32 Size64K = 65536u;

const UINT_32 Log2Size256 = 8u;
const UINT_32 Log2Size4K  = 12u;
const UINT_32 Log2Size64K = 16u;

/**
************************************************************************************************************************
* @brief Bit setting for swizzle pattern
************************************************************************************************************************
*/
union ADDR_BIT_SETTING
{
    struct
    {
        UINT_16 x;
        UINT_16 y;
        UINT_16 z;
        UINT_16 s;
    };
    UINT_64 value;
};

/**
************************************************************************************************************************
* @brief Swizzle pattern information
************************************************************************************************************************
*/
// Accessed by index representing the logbase2 of (8bpp/16bpp/32bpp/64bpp/128bpp)
// contains the indices which map to 2D arrays SW_PATTERN_NIBBLE[0-9] which contain sections of an index equation. They are dependant on pipe# and bpe #
struct ADDR_SW_PATINFO
{
    UINT_8  maxItemCount;
    UINT_8  nibble01Idx;
    UINT_16 nibble2Idx;
    UINT_16 nibble3Idx;
    UINT_8  nibble4Idx;
};

/**
************************************************************************************************************************
*   InitBit
*
*   @brief
*       Initialize bit setting value via a return value
************************************************************************************************************************
*/
#define InitBit(c, index) (1ull << ((c << 4) + index))

const UINT_64 X0  = InitBit(0,  0);
const UINT_64 X1  = InitBit(0,  1);
const UINT_64 X2  = InitBit(0,  2);
const UINT_64 X3  = InitBit(0,  3);
const UINT_64 X4  = InitBit(0,  4);
const UINT_64 X5  = InitBit(0,  5);
const UINT_64 X6  = InitBit(0,  6);
const UINT_64 X7  = InitBit(0,  7);
const UINT_64 X8  = InitBit(0,  8);
const UINT_64 X9  = InitBit(0,  9);
const UINT_64 X10 = InitBit(0, 10);
const UINT_64 X11 = InitBit(0, 11);

const UINT_64 Y0  = InitBit(1,  0);
const UINT_64 Y1  = InitBit(1,  1);
const UINT_64 Y2  = InitBit(1,  2);
const UINT_64 Y3  = InitBit(1,  3);
const UINT_64 Y4  = InitBit(1,  4);
const UINT_64 Y5  = InitBit(1,  5);
const UINT_64 Y6  = InitBit(1,  6);
const UINT_64 Y7  = InitBit(1,  7);
const UINT_64 Y8  = InitBit(1,  8);
const UINT_64 Y9  = InitBit(1,  9);
const UINT_64 Y10 = InitBit(1, 10);
const UINT_64 Y11 = InitBit(1, 11);

const UINT_64 Z0  = InitBit(2,  0);
const UINT_64 Z1  = InitBit(2,  1);
const UINT_64 Z2  = InitBit(2,  2);
const UINT_64 Z3  = InitBit(2,  3);
const UINT_64 Z4  = InitBit(2,  4);
const UINT_64 Z5  = InitBit(2,  5);
const UINT_64 Z6  = InitBit(2,  6);
const UINT_64 Z7  = InitBit(2,  7);
const UINT_64 Z8  = InitBit(2,  8);

const UINT_64 S0  = InitBit(3,  0);
const UINT_64 S1  = InitBit(3,  1);
const UINT_64 S2  = InitBit(3,  2);

/**
************************************************************************************************************************
* @brief This class contains asic independent address lib functionalities
************************************************************************************************************************
*/
class Lib : public Addr::Lib
{
public:
    virtual ~Lib();

    static Lib* GetLib(
        ADDR_HANDLE hLib);

    //
    // Interface stubs
    //

    // For data surface
    ADDR_E_RETURNCODE ComputeSurfaceInfo(
        const ADDR2_COMPUTE_SURFACE_INFO_INPUT* pIn,
        ADDR2_COMPUTE_SURFACE_INFO_OUTPUT*      pOut) const;

    ADDR_E_RETURNCODE ComputeSurfaceAddrFromCoord(
        const ADDR2_COMPUTE_SURFACE_ADDRFROMCOORD_INPUT* pIn,
        ADDR2_COMPUTE_SURFACE_ADDRFROMCOORD_OUTPUT*      pOut) const;

    ADDR_E_RETURNCODE ComputeSurfaceCoordFromAddr(
        const ADDR2_COMPUTE_SURFACE_COORDFROMADDR_INPUT* pIn,
        ADDR2_COMPUTE_SURFACE_COORDFROMADDR_OUTPUT*      pOut) const;

    // For HTile
    ADDR_E_RETURNCODE ComputeHtileInfo(
        const ADDR2_COMPUTE_HTILE_INFO_INPUT* pIn,
        ADDR2_COMPUTE_HTILE_INFO_OUTPUT*      pOut) const;

    ADDR_E_RETURNCODE ComputeHtileAddrFromCoord(
        const ADDR2_COMPUTE_HTILE_ADDRFROMCOORD_INPUT* pIn,
        ADDR2_COMPUTE_HTILE_ADDRFROMCOORD_OUTPUT*      pOut);

    ADDR_E_RETURNCODE ComputeHtileCoordFromAddr(
        const ADDR2_COMPUTE_HTILE_COORDFROMADDR_INPUT* pIn,
        ADDR2_COMPUTE_HTILE_COORDFROMADDR_OUTPUT*      pOut);

    // For CMask
    ADDR_E_RETURNCODE ComputeCmaskInfo(
        const ADDR2_COMPUTE_CMASK_INFO_INPUT* pIn,
        ADDR2_COMPUTE_CMASK_INFO_OUTPUT*      pOut) const;

    ADDR_E_RETURNCODE ComputeCmaskAddrFromCoord(
        const ADDR2_COMPUTE_CMASK_ADDRFROMCOORD_INPUT* pIn,
        ADDR2_COMPUTE_CMASK_ADDRFROMCOORD_OUTPUT*      pOut);

    ADDR_E_RETURNCODE ComputeCmaskCoordFromAddr(
        const ADDR2_COMPUTE_CMASK_COORDFROMADDR_INPUT* pIn,
        ADDR2_COMPUTE_CMASK_COORDFROMADDR_OUTPUT*      pOut) const;

    // For FMask
    ADDR_E_RETURNCODE ComputeFmaskInfo(
        const ADDR2_COMPUTE_FMASK_INFO_INPUT* pIn,
        ADDR2_COMPUTE_FMASK_INFO_OUTPUT*      pOut);

    ADDR_E_RETURNCODE ComputeFmaskAddrFromCoord(
        const ADDR2_COMPUTE_FMASK_ADDRFROMCOORD_INPUT* pIn,
        ADDR2_COMPUTE_FMASK_ADDRFROMCOORD_OUTPUT*      pOut) const;

    ADDR_E_RETURNCODE ComputeFmaskCoordFromAddr(
        const ADDR2_COMPUTE_FMASK_COORDFROMADDR_INPUT* pIn,
        ADDR2_COMPUTE_FMASK_COORDFROMADDR_OUTPUT*      pOut) const;

    // For DCC key
    ADDR_E_RETURNCODE ComputeDccInfo(
        const ADDR2_COMPUTE_DCCINFO_INPUT* pIn,
        ADDR2_COMPUTE_DCCINFO_OUTPUT*      pOut) const;

    ADDR_E_RETURNCODE ComputeDccAddrFromCoord(
        const ADDR2_COMPUTE_DCC_ADDRFROMCOORD_INPUT* pIn,
        ADDR2_COMPUTE_DCC_ADDRFROMCOORD_OUTPUT*      pOut);

    // Misc
    ADDR_E_RETURNCODE ComputePipeBankXor(
        const ADDR2_COMPUTE_PIPEBANKXOR_INPUT* pIn,
        ADDR2_COMPUTE_PIPEBANKXOR_OUTPUT*      pOut);

    ADDR_E_RETURNCODE ComputeSlicePipeBankXor(
        const ADDR2_COMPUTE_SLICE_PIPEBANKXOR_INPUT* pIn,
        ADDR2_COMPUTE_SLICE_PIPEBANKXOR_OUTPUT*      pOut);

    ADDR_E_RETURNCODE ComputeSubResourceOffsetForSwizzlePattern(
        const ADDR2_COMPUTE_SUBRESOURCE_OFFSET_FORSWIZZLEPATTERN_INPUT* pIn,
        ADDR2_COMPUTE_SUBRESOURCE_OFFSET_FORSWIZZLEPATTERN_OUTPUT*      pOut);

    ADDR_E_RETURNCODE ComputeNonBlockCompressedView(
        const ADDR2_COMPUTE_NONBLOCKCOMPRESSEDVIEW_INPUT* pIn,
        ADDR2_COMPUTE_NONBLOCKCOMPRESSEDVIEW_OUTPUT*      pOut);

    ADDR_E_RETURNCODE Addr2GetPreferredSurfaceSetting(
        const ADDR2_GET_PREFERRED_SURF_SETTING_INPUT* pIn,
        ADDR2_GET_PREFERRED_SURF_SETTING_OUTPUT*      pOut) const;

    ADDR_E_RETURNCODE GetPossibleSwizzleModes(
        const ADDR2_GET_PREFERRED_SURF_SETTING_INPUT* pIn,
        ADDR2_GET_PREFERRED_SURF_SETTING_OUTPUT* pOut) const;

    virtual BOOL_32 IsValidDisplaySwizzleMode(
        const ADDR2_COMPUTE_SURFACE_INFO_INPUT* pIn) const
    {
        ADDR_NOT_IMPLEMENTED();
        return ADDR_NOTIMPLEMENTED;
    }

    ADDR_E_RETURNCODE GetAllowedBlockSet(
        ADDR2_SWMODE_SET allowedSwModeSet,
        AddrResourceType rsrcType,
        ADDR2_BLOCK_SET* pAllowedBlockSet) const;

    ADDR_E_RETURNCODE GetAllowedSwSet(
        ADDR2_SWMODE_SET  allowedSwModeSet,
        ADDR2_SWTYPE_SET* pAllowedSwSet) const;

protected:
    Lib();  // Constructor is protected
    Lib(const Client* pClient);

    static const UINT_32 MaxNumOfBpp = 5;
    static const UINT_32 MaxNumOfBppCMask = 4;
    static const UINT_32 MaxNumOfAA  = 4;

    static const Dim2d Block256_2d[MaxNumOfBpp];
    static const Dim3d Block1K_3d[MaxNumOfBpp];

    static const UINT_32 PrtAlignment = 64 * 1024;
    static const UINT_32 MaxMacroBits = 20;

    static const UINT_32 MaxMipLevels = 16;

    BOOL_32 IsValidSwMode(AddrSwizzleMode swizzleMode) const
    {
        return (m_swizzleModeTable[swizzleMode].u32All != 0);
    }

    // Checking block size
    BOOL_32 IsBlock256b(AddrSwizzleMode swizzleMode) const
    {
        return m_swizzleModeTable[swizzleMode].is256b;
    }

    BOOL_32 IsBlock4kb(AddrSwizzleMode swizzleMode) const
    {
        return m_swizzleModeTable[swizzleMode].is4kb;
    }

    BOOL_32 IsBlock64kb(AddrSwizzleMode swizzleMode) const
    {
        return m_swizzleModeTable[swizzleMode].is64kb;
    }

    BOOL_32 IsBlockVariable(AddrSwizzleMode swizzleMode) const
    {
        return m_swizzleModeTable[swizzleMode].isVar;
    }

    // Checking swizzle mode
    BOOL_32 IsLinear(AddrSwizzleMode swizzleMode) const
    {
        return m_swizzleModeTable[swizzleMode].isLinear;
    }

    BOOL_32 IsRtOptSwizzle(AddrSwizzleMode swizzleMode) const
    {
        return m_swizzleModeTable[swizzleMode].isRtOpt;
    }

    BOOL_32 IsZOrderSwizzle(AddrSwizzleMode swizzleMode) const
    {
        return m_swizzleModeTable[swizzleMode].isZ;
    }

    BOOL_32 IsStandardSwizzle(AddrSwizzleMode swizzleMode) const
    {
        return m_swizzleModeTable[swizzleMode].isStd;
    }

    BOOL_32 IsDisplaySwizzle(AddrSwizzleMode swizzleMode) const
    {
        return m_swizzleModeTable[swizzleMode].isDisp;
    }

    BOOL_32 IsRotateSwizzle(AddrSwizzleMode swizzleMode) const
    {
        return m_swizzleModeTable[swizzleMode].isRot;
    }

    BOOL_32 IsStandardSwizzle(AddrResourceType resourceType, AddrSwizzleMode swizzleMode) const
    {
        return HwlIsStandardSwizzle(resourceType, swizzleMode);
    }

    BOOL_32 IsDisplaySwizzle(AddrResourceType resourceType, AddrSwizzleMode swizzleMode) const
    {
        return HwlIsDisplaySwizzle(resourceType, swizzleMode);
    }

    BOOL_32 IsXor(AddrSwizzleMode swizzleMode) const
    {
        return m_swizzleModeTable[swizzleMode].isXor;
    }

    BOOL_32 IsPrt(AddrSwizzleMode swizzleMode) const
    {
        return m_swizzleModeTable[swizzleMode].isT;
    }

    BOOL_32 IsNonPrtXor(AddrSwizzleMode swizzleMode) const
    {
        return (IsXor(swizzleMode) && (IsPrt(swizzleMode) == FALSE));
    }

    // Checking resource type
    static BOOL_32 IsTex1d(AddrResourceType resourceType)
    {
        return (resourceType == ADDR_RSRC_TEX_1D);
    }

    static BOOL_32 IsTex2d(AddrResourceType resourceType)
    {
        return (resourceType == ADDR_RSRC_TEX_2D);
    }

    static BOOL_32 IsTex3d(AddrResourceType resourceType)
    {
        return (resourceType == ADDR_RSRC_TEX_3D);
    }

    BOOL_32 IsThick(AddrResourceType resourceType, AddrSwizzleMode swizzleMode) const
    {
        return HwlIsThick(resourceType, swizzleMode);
    }

    BOOL_32 IsThin(AddrResourceType resourceType, AddrSwizzleMode swizzleMode) const
    {
        return HwlIsThin(resourceType, swizzleMode);
    }

    UINT_32 GetBlockSizeLog2(AddrSwizzleMode swizzleMode) const
    {
        UINT_32 blockSizeLog2 = 0;

        if (IsBlock256b(swizzleMode) || IsLinear(swizzleMode))
        {
            blockSizeLog2 = 8;
        }
        else if (IsBlock4kb(swizzleMode))
        {
            blockSizeLog2 = 12;
        }
        else if (IsBlock64kb(swizzleMode))
        {
            blockSizeLog2 = 16;
        }
        else if (IsBlockVariable(swizzleMode) && (m_blockVarSizeLog2 != 0))
        {
            blockSizeLog2 = m_blockVarSizeLog2;
        }
        else
        {
            ADDR_ASSERT_ALWAYS();
        }

        return blockSizeLog2;
    }

    UINT_32 GetBlockSize(AddrSwizzleMode swizzleMode) const
    {
        return (1 << GetBlockSizeLog2(swizzleMode));
    }

    static UINT_32 GetFmaskBpp(UINT_32 sample, UINT_32 frag)
    {
        sample = (sample == 0) ? 1 : sample;
        frag   = (frag   == 0) ? sample : frag;

        UINT_32 fmaskBpp = QLog2(frag);

        if (sample > frag)
        {
            fmaskBpp++;
        }

        if (fmaskBpp == 3)
        {
            fmaskBpp = 4;
        }

        fmaskBpp = Max(8u, fmaskBpp * sample);

        return fmaskBpp;
    }

    virtual BOOL_32 HwlIsStandardSwizzle(
        AddrResourceType resourceType,
        AddrSwizzleMode  swizzleMode) const
    {
        ADDR_NOT_IMPLEMENTED();
        return FALSE;
    }

    virtual BOOL_32 HwlIsDisplaySwizzle(
        AddrResourceType resourceType,
        AddrSwizzleMode  swizzleMode) const
    {
        ADDR_NOT_IMPLEMENTED();
        return FALSE;
    }

    virtual BOOL_32 HwlIsThin(
        AddrResourceType resourceType,
        AddrSwizzleMode  swizzleMode) const
    {
        ADDR_NOT_IMPLEMENTED();
        return FALSE;
    }

    virtual BOOL_32 HwlIsThick(
        AddrResourceType resourceType,
        AddrSwizzleMode  swizzleMode) const
    {
        ADDR_NOT_IMPLEMENTED();
        return FALSE;
    }

    virtual ADDR_E_RETURNCODE HwlComputeHtileInfo(
        const ADDR2_COMPUTE_HTILE_INFO_INPUT*    pIn,
        ADDR2_COMPUTE_HTILE_INFO_OUTPUT*         pOut) const
    {
        ADDR_NOT_IMPLEMENTED();
        return ADDR_NOTSUPPORTED;
    }

    virtual ADDR_E_RETURNCODE HwlComputeCmaskInfo(
        const ADDR2_COMPUTE_CMASK_INFO_INPUT*    pIn,
        ADDR2_COMPUTE_CMASK_INFO_OUTPUT*         pOut) const
    {
        ADDR_NOT_IMPLEMENTED();
        return ADDR_NOTSUPPORTED;
    }

    virtual ADDR_E_RETURNCODE HwlComputeDccInfo(
        const ADDR2_COMPUTE_DCCINFO_INPUT*    pIn,
        ADDR2_COMPUTE_DCCINFO_OUTPUT*         pOut) const
    {
        ADDR_NOT_IMPLEMENTED();
        return ADDR_NOTSUPPORTED;
    }

    virtual ADDR_E_RETURNCODE HwlSupportComputeDccAddrFromCoord(
        const ADDR2_COMPUTE_DCC_ADDRFROMCOORD_INPUT* pIn)
    {
        ADDR_NOT_IMPLEMENTED();
        return ADDR_NOTSUPPORTED;
    }

    virtual VOID HwlComputeDccAddrFromCoord(
        const ADDR2_COMPUTE_DCC_ADDRFROMCOORD_INPUT* pIn,
        ADDR2_COMPUTE_DCC_ADDRFROMCOORD_OUTPUT*      pOut)
    {
        ADDR_NOT_IMPLEMENTED();
    }

    virtual ADDR_E_RETURNCODE HwlComputeCmaskAddrFromCoord(
        const ADDR2_COMPUTE_CMASK_ADDRFROMCOORD_INPUT* pIn,
        ADDR2_COMPUTE_CMASK_ADDRFROMCOORD_OUTPUT*      pOut)
    {
        ADDR_NOT_IMPLEMENTED();
        return ADDR_NOTSUPPORTED;
    }

    virtual ADDR_E_RETURNCODE HwlComputeHtileAddrFromCoord(
        const ADDR2_COMPUTE_HTILE_ADDRFROMCOORD_INPUT* pIn,
        ADDR2_COMPUTE_HTILE_ADDRFROMCOORD_OUTPUT*      pOut)
    {
        ADDR_NOT_IMPLEMENTED();
        return ADDR_NOTSUPPORTED;
    }

    virtual ADDR_E_RETURNCODE HwlComputeHtileCoordFromAddr(
        const ADDR2_COMPUTE_HTILE_COORDFROMADDR_INPUT* pIn,
        ADDR2_COMPUTE_HTILE_COORDFROMADDR_OUTPUT*      pOut)
    {
        ADDR_NOT_IMPLEMENTED();
        return ADDR_NOTSUPPORTED;
    }

    virtual ADDR_E_RETURNCODE HwlComputeBlock256Equation(
        AddrResourceType rsrcType,
        AddrSwizzleMode swMode,
        UINT_32 elementBytesLog2,
        ADDR_EQUATION* pEquation) const
    {
        ADDR_NOT_IMPLEMENTED();
        return ADDR_NOTSUPPORTED;
    }

    virtual ADDR_E_RETURNCODE HwlComputeThinEquation(
        AddrResourceType rsrcType,
        AddrSwizzleMode swMode,
        UINT_32 elementBytesLog2,
        ADDR_EQUATION* pEquation) const
    {
        ADDR_NOT_IMPLEMENTED();
        return ADDR_NOTSUPPORTED;
    }

    virtual ADDR_E_RETURNCODE HwlComputeThickEquation(
        AddrResourceType rsrcType,
        AddrSwizzleMode swMode,
        UINT_32 elementBytesLog2,
        ADDR_EQUATION* pEquation) const
    {
        ADDR_NOT_IMPLEMENTED();
        return ADDR_NOTSUPPORTED;
    }

    virtual UINT_32 HwlGetEquationIndex(
        const ADDR2_COMPUTE_SURFACE_INFO_INPUT* pIn,
        ADDR2_COMPUTE_SURFACE_INFO_OUTPUT*      pOut) const
    {
        ADDR_NOT_IMPLEMENTED();
        return ADDR_INVALID_EQUATION_INDEX;
    }

    UINT_32 GetEquationIndex(
        const ADDR2_COMPUTE_SURFACE_INFO_INPUT* pIn,
        ADDR2_COMPUTE_SURFACE_INFO_OUTPUT*      pOut) const
    {
        return HwlGetEquationIndex(pIn, pOut);
    }

    virtual ADDR_E_RETURNCODE HwlComputePipeBankXor(
        const ADDR2_COMPUTE_PIPEBANKXOR_INPUT* pIn,
        ADDR2_COMPUTE_PIPEBANKXOR_OUTPUT*      pOut) const
    {
        ADDR_NOT_IMPLEMENTED();
        return ADDR_NOTSUPPORTED;
    }

    virtual ADDR_E_RETURNCODE HwlComputeSlicePipeBankXor(
        const ADDR2_COMPUTE_SLICE_PIPEBANKXOR_INPUT* pIn,
        ADDR2_COMPUTE_SLICE_PIPEBANKXOR_OUTPUT*      pOut) const
    {
        ADDR_NOT_IMPLEMENTED();
        return ADDR_NOTSUPPORTED;
    }

    virtual ADDR_E_RETURNCODE HwlComputeSubResourceOffsetForSwizzlePattern(
        const ADDR2_COMPUTE_SUBRESOURCE_OFFSET_FORSWIZZLEPATTERN_INPUT* pIn,
        ADDR2_COMPUTE_SUBRESOURCE_OFFSET_FORSWIZZLEPATTERN_OUTPUT*      pOut) const
    {
        ADDR_NOT_IMPLEMENTED();
        return ADDR_NOTSUPPORTED;
    }

    virtual ADDR_E_RETURNCODE HwlComputeNonBlockCompressedView(
        const ADDR2_COMPUTE_NONBLOCKCOMPRESSEDVIEW_INPUT* pIn,
        ADDR2_COMPUTE_NONBLOCKCOMPRESSEDVIEW_OUTPUT*      pOut) const
    {
        ADDR_NOT_IMPLEMENTED();
        return ADDR_NOTSUPPORTED;
    }

    virtual ADDR_E_RETURNCODE HwlGetPreferredSurfaceSetting(
        const ADDR2_GET_PREFERRED_SURF_SETTING_INPUT* pIn,
        ADDR2_GET_PREFERRED_SURF_SETTING_OUTPUT*      pOut) const
    {
        ADDR_NOT_IMPLEMENTED();
        return ADDR_NOTSUPPORTED;
    }

    virtual ADDR_E_RETURNCODE HwlGetPossibleSwizzleModes(
        const ADDR2_GET_PREFERRED_SURF_SETTING_INPUT* pIn,
        ADDR2_GET_PREFERRED_SURF_SETTING_OUTPUT*      pOut) const
    {
        ADDR_NOT_IMPLEMENTED();
        return ADDR_NOTSUPPORTED;
    }

    virtual ADDR_E_RETURNCODE HwlGetAllowedBlockSet(
        ADDR2_SWMODE_SET allowedSwModeSet,
        AddrResourceType rsrcType,
        ADDR2_BLOCK_SET* pAllowedBlockSet) const
    {
        ADDR_NOT_IMPLEMENTED();
        return ADDR_NOTIMPLEMENTED;
    }

    virtual ADDR_E_RETURNCODE HwlGetAllowedSwSet(
        ADDR2_SWMODE_SET  allowedSwModeSet,
        ADDR2_SWTYPE_SET* pAllowedSwSet) const
    {
        ADDR_NOT_IMPLEMENTED();
        return ADDR_NOTIMPLEMENTED;
    }

    virtual ADDR_E_RETURNCODE HwlComputeSurfaceInfoSanityCheck(
        const ADDR2_COMPUTE_SURFACE_INFO_INPUT* pIn) const
    {
        ADDR_NOT_IMPLEMENTED();
        return ADDR_NOTSUPPORTED;
    }

    virtual ADDR_E_RETURNCODE HwlComputeSurfaceInfoTiled(
         const ADDR2_COMPUTE_SURFACE_INFO_INPUT* pIn,
         ADDR2_COMPUTE_SURFACE_INFO_OUTPUT*      pOut) const
    {
        ADDR_NOT_IMPLEMENTED();
        return ADDR_NOTIMPLEMENTED;
    }

    virtual ADDR_E_RETURNCODE HwlComputeSurfaceInfoLinear(
         const ADDR2_COMPUTE_SURFACE_INFO_INPUT* pIn,
         ADDR2_COMPUTE_SURFACE_INFO_OUTPUT*      pOut) const
    {
        ADDR_NOT_IMPLEMENTED();
        return ADDR_NOTIMPLEMENTED;
    }

    virtual ADDR_E_RETURNCODE HwlComputeSurfaceAddrFromCoordTiled(
        const ADDR2_COMPUTE_SURFACE_ADDRFROMCOORD_INPUT* pIn,
        ADDR2_COMPUTE_SURFACE_ADDRFROMCOORD_OUTPUT*      pOut) const
    {
        ADDR_NOT_IMPLEMENTED();
        return ADDR_NOTIMPLEMENTED;
    }

    ADDR_E_RETURNCODE ComputeBlock256Equation(
        AddrResourceType rsrcType,
        AddrSwizzleMode swMode,
        UINT_32 elementBytesLog2,
        ADDR_EQUATION* pEquation) const;

    ADDR_E_RETURNCODE ComputeThinEquation(
        AddrResourceType rsrcType,
        AddrSwizzleMode swMode,
        UINT_32 elementBytesLog2,
        ADDR_EQUATION* pEquation) const;

    ADDR_E_RETURNCODE ComputeThickEquation(
        AddrResourceType rsrcType,
        AddrSwizzleMode swMode,
        UINT_32 elementBytesLog2,
        ADDR_EQUATION* pEquation) const;

    ADDR_E_RETURNCODE ComputeSurfaceInfoSanityCheck(
        const ADDR2_COMPUTE_SURFACE_INFO_INPUT* pIn) const;

    ADDR_E_RETURNCODE ComputeSurfaceInfoLinear(
        const ADDR2_COMPUTE_SURFACE_INFO_INPUT* pIn,
        ADDR2_COMPUTE_SURFACE_INFO_OUTPUT*      pOut) const;

    ADDR_E_RETURNCODE ComputeSurfaceInfoTiled(
        const ADDR2_COMPUTE_SURFACE_INFO_INPUT* pIn,
        ADDR2_COMPUTE_SURFACE_INFO_OUTPUT*      pOut) const;

    ADDR_E_RETURNCODE ComputeSurfaceAddrFromCoordLinear(
        const ADDR2_COMPUTE_SURFACE_ADDRFROMCOORD_INPUT* pIn,
        ADDR2_COMPUTE_SURFACE_ADDRFROMCOORD_OUTPUT*      pOut) const;

    ADDR_E_RETURNCODE ComputeSurfaceAddrFromCoordTiled(
        const ADDR2_COMPUTE_SURFACE_ADDRFROMCOORD_INPUT* pIn,
        ADDR2_COMPUTE_SURFACE_ADDRFROMCOORD_OUTPUT*      pOut) const;

    ADDR_E_RETURNCODE ComputeSurfaceCoordFromAddrLinear(
        const ADDR2_COMPUTE_SURFACE_COORDFROMADDR_INPUT* pIn,
        ADDR2_COMPUTE_SURFACE_COORDFROMADDR_OUTPUT*      pOut) const;

    ADDR_E_RETURNCODE ComputeSurfaceCoordFromAddrTiled(
        const ADDR2_COMPUTE_SURFACE_COORDFROMADDR_INPUT* pIn,
        ADDR2_COMPUTE_SURFACE_COORDFROMADDR_OUTPUT*      pOut) const;

    UINT_32 ComputeSurface2DMicroBlockOffset(
        const _ADDR2_COMPUTE_SURFACE_ADDRFROMCOORD_INPUT* pIn) const;

    UINT_32 ComputeSurface3DMicroBlockOffset(
        const _ADDR2_COMPUTE_SURFACE_ADDRFROMCOORD_INPUT* pIn) const;

    // Misc
    ADDR_E_RETURNCODE ComputeBlockDimensionForSurf(
        UINT_32*         pWidth,
        UINT_32*         pHeight,
        UINT_32*         pDepth,
        UINT_32          bpp,
        UINT_32          numSamples,
        AddrResourceType resourceType,
        AddrSwizzleMode  swizzleMode) const;

    ADDR_E_RETURNCODE ComputeBlockDimension(
        UINT_32*         pWidth,
        UINT_32*         pHeight,
        UINT_32*         pDepth,
        UINT_32          bpp,
        AddrResourceType resourceType,
        AddrSwizzleMode  swizzleMode) const;

    virtual VOID ComputeThinBlockDimension(
        UINT_32*         pWidth,
        UINT_32*         pHeight,
        UINT_32*         pDepth,
        UINT_32          bpp,
        UINT_32          numSamples,
        AddrResourceType resourceType,
        AddrSwizzleMode  swizzleMode) const;

    VOID ComputeThickBlockDimension(
        UINT_32*         pWidth,
        UINT_32*         pHeight,
        UINT_32*         pDepth,
        UINT_32          bpp,
        AddrResourceType resourceType,
        AddrSwizzleMode  swizzleMode) const;

    static UINT_64 ComputePadSize(
        const Dim3d*      pBlkDim,
        UINT_32           width,
        UINT_32           height,
        UINT_32           numSlices,
        Dim3d*            pPadDim)
    {
        pPadDim->w = PowTwoAlign(width ,pBlkDim->w);
        pPadDim->h = PowTwoAlign(height ,pBlkDim->h);
        pPadDim->d = PowTwoAlign(numSlices, pBlkDim->d);
        return static_cast<UINT_64>(pPadDim->w) * pPadDim->h * pPadDim->d;
    }

    static ADDR_E_RETURNCODE ExtractPipeBankXor(
        UINT_32  pipeBankXor,
        UINT_32  bankBits,
        UINT_32  pipeBits,
        UINT_32* pBankX,
        UINT_32* pPipeX);

    static BOOL_32 Valid3DMipSliceIdConstraint(
        UINT_32 numSlices,
        UINT_32 mipId,
        UINT_32 slice)
    {
        return (Max((numSlices >> mipId), 1u) > slice);
    }

    Dim3d GetMipTailDim(
        AddrResourceType  resourceType,
        AddrSwizzleMode   swizzleMode,
        UINT_32           blockWidth,
        UINT_32           blockHeight,
        UINT_32           blockDepth) const;

    static BOOL_32 IsLocalHeap(AddrResrouceLocation resourceType)
    {
        return ((resourceType == ADDR_RSRC_LOC_LOCAL) ||
                (resourceType == ADDR_RSRC_LOC_INVIS));
    }

    static BOOL_32 IsInvisibleHeap(AddrResrouceLocation resourceType)
    {
        return (resourceType == ADDR_RSRC_LOC_INVIS);
    }

    static BOOL_32 IsNonlocalHeap(AddrResrouceLocation resourceType)
    {
        return ((resourceType == ADDR_RSRC_LOC_USWC) ||
                (resourceType == ADDR_RSRC_LOC_CACHED));
    }

    UINT_32 GetPipeLog2ForMetaAddressing(BOOL_32 pipeAligned, AddrSwizzleMode swizzleMode) const
    {
        UINT_32 numPipeLog2 = pipeAligned ? Min(m_pipesLog2 + m_seLog2, 5u) : 0;

        if (IsXor(swizzleMode))
        {
            UINT_32 maxPipeLog2 = GetBlockSizeLog2(swizzleMode) - m_pipeInterleaveLog2;

            numPipeLog2 = Min(numPipeLog2, maxPipeLog2);
        }

        return numPipeLog2;
    }

    UINT_32 GetPipeNumForMetaAddressing(BOOL_32 pipeAligned, AddrSwizzleMode swizzleMode) const
    {
        return (1 << GetPipeLog2ForMetaAddressing(pipeAligned, swizzleMode));
    }

    VOID VerifyMipLevelInfo(const ADDR2_COMPUTE_SURFACE_INFO_INPUT* pIn) const
    {
#if DEBUG
        if (pIn->numMipLevels > 1)
        {
            UINT_32 actualMipLevels = 1;
            switch (pIn->resourceType)
            {
                case ADDR_RSRC_TEX_3D:
                    // Fall through to share 2D case
                    actualMipLevels = Max(actualMipLevels, Log2NonPow2(pIn->numSlices) + 1);
                case ADDR_RSRC_TEX_2D:
                    // Fall through to share 1D case
                    actualMipLevels = Max(actualMipLevels, Log2NonPow2(pIn->height) + 1);
                case ADDR_RSRC_TEX_1D:
                    // Base 1D case
                    actualMipLevels = Max(actualMipLevels, Log2NonPow2(pIn->width) + 1);
                    break;
                default:
                    ADDR_ASSERT_ALWAYS();
                    break;
            }
            // Client pass wrong number of MipLevels to addrlib and result will be bad.
            // Not sure if we should fail this calling instead of putting an assertion here.
            ADDR_ASSERT(actualMipLevels >= pIn->numMipLevels);
        }
#endif
    }

    ADDR_E_RETURNCODE ApplyCustomerPipeBankXor(
        AddrSwizzleMode swizzleMode,
        UINT_32         pipeBankXor,
        UINT_32         bankBits,
        UINT_32         pipeBits,
        UINT_32*        pBlockOffset) const
    {
        ADDR_E_RETURNCODE returnCode = ADDR_OK;

        if (IsXor(swizzleMode))
        {
            // Apply driver set bankPipeXor
            UINT_32 bankX = 0;
            UINT_32 pipeX = 0;
            returnCode = ExtractPipeBankXor(pipeBankXor, bankBits, pipeBits, &bankX, &pipeX);
            *pBlockOffset ^= (pipeX << m_pipeInterleaveLog2);
            *pBlockOffset ^= (bankX << (m_pipeInterleaveLog2 + pipeBits));
        }

        return returnCode;
    }

    UINT_32 GetPipeXorBits(UINT_32 macroBlockBits) const;

    ADDR_E_RETURNCODE ApplyCustomizedPitchHeight(
        const ADDR2_COMPUTE_SURFACE_INFO_INPUT* pIn,
        UINT_32                                 elementBytes,
        UINT_32                                 pitchAlignInElement,
        UINT_32*                                pPitch,
        UINT_32*                                pHeight) const;

    VOID ComputeQbStereoInfo(ADDR2_COMPUTE_SURFACE_INFO_OUTPUT* pOut) const;

    VOID FilterInvalidEqSwizzleMode(
        ADDR2_SWMODE_SET& allowedSwModeSet,
        AddrResourceType  resourceType,
        UINT_32           elemLog2,
        UINT_32           maxComponents) const;

#if DEBUG
    VOID ValidateStereoInfo(
        const ADDR2_COMPUTE_SURFACE_INFO_INPUT* pIn,
        const ADDR2_COMPUTE_SURFACE_INFO_OUTPUT* pOut) const;
#endif

    UINT_32 m_se;                       ///< Number of shader engine
    UINT_32 m_rbPerSe;                  ///< Number of render backend per shader engine
    UINT_32 m_maxCompFrag;              ///< Number of max compressed fragment

    UINT_32 m_banksLog2;                ///< Number of bank Log2
    UINT_32 m_pipesLog2;                ///< Number of pipe per shader engine Log2
    UINT_32 m_seLog2;                   ///< Number of shader engine Log2
    UINT_32 m_rbPerSeLog2;              ///< Number of render backend per shader engine Log2
    UINT_32 m_maxCompFragLog2;          ///< Number of max compressed fragment Log2

    UINT_32 m_pipeInterleaveLog2;       ///< Log2 of pipe interleave bytes

    UINT_32 m_blockVarSizeLog2;         ///< Log2 of block var size

    SwizzleModeFlags m_swizzleModeTable[ADDR_SW_MAX_TYPE];  ///< Swizzle mode table

    // Max number of swizzle mode supported for equation
    static const UINT_32    MaxSwModeType = 32;
    // Max number of resource type (2D/3D) supported for equation
    static const UINT_32    MaxRsrcType = 2;
    // Max number of bpp (8bpp/16bpp/32bpp/64bpp/128bpp)
    static const UINT_32    MaxElementBytesLog2  = 5;
    // Almost all swizzle mode + resource type support equation
    static const UINT_32    EquationTableSize = MaxElementBytesLog2 * MaxSwModeType * MaxRsrcType;
    // Equation table
    ADDR_EQUATION           m_equationTable[EquationTableSize];

    // Number of equation entries in the table
    UINT_32                 m_numEquations;
    // Equation lookup table according to bpp and tile index
    UINT_32                 m_equationLookupTable[MaxRsrcType][MaxSwModeType][MaxElementBytesLog2];

private:
    // Disallow the copy constructor
    Lib(const Lib& a);

    // Disallow the assignment operator
    Lib& operator=(const Lib& a);
};

} // V2
} // Addr

#endif

