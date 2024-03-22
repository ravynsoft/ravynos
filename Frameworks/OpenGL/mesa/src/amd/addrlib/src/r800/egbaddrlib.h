/*
************************************************************************************************************************
*
*  Copyright (C) 2007-2022 Advanced Micro Devices, Inc.  All rights reserved.
*  SPDX-License-Identifier: MIT
*
***********************************************************************************************************************/

/**
****************************************************************************************************
* @file  egbaddrlib.h
* @brief Contains the EgBasedLib class definition.
****************************************************************************************************
*/

#ifndef __EG_BASED_ADDR_LIB_H__
#define __EG_BASED_ADDR_LIB_H__

#include "addrlib1.h"

namespace Addr
{
namespace V1
{
/// Structures for functions
struct CoordFromBankPipe
{
    UINT_32 xBits : 3;
    UINT_32 yBits : 4;

    UINT_32 xBit3 : 1;
    UINT_32 xBit4 : 1;
    UINT_32 xBit5 : 1;
    UINT_32 yBit3 : 1;
    UINT_32 yBit4 : 1;
    UINT_32 yBit5 : 1;
    UINT_32 yBit6 : 1;
};

/**
****************************************************************************************************
* @brief This class is the Evergreen based address library
* @note  Abstract class
****************************************************************************************************
*/
class EgBasedLib : public Lib
{
protected:
    EgBasedLib(const Client* pClient);
    virtual ~EgBasedLib();

public:

    /// Surface info functions

    // NOTE: DispatchComputeSurfaceInfo using TileInfo takes both an input and an output.
    //       On input:
    //       One or more fields may be 0 to be calculated/defaulted - pre-SI h/w.
    //       H/W using tile mode index only accepts none or all 0's - SI and newer h/w.
    //       It then returns the actual tiling configuration used.
    //       Other methods' TileInfo must be valid on entry
    BOOL_32 DispatchComputeSurfaceInfo(
        const ADDR_COMPUTE_SURFACE_INFO_INPUT* pIn,
        ADDR_COMPUTE_SURFACE_INFO_OUTPUT* pOut) const;

    ADDR_E_RETURNCODE DispatchComputeFmaskInfo(
        const ADDR_COMPUTE_FMASK_INFO_INPUT* pIn,
        ADDR_COMPUTE_FMASK_INFO_OUTPUT* pOut);

protected:
    // Hwl interface
    virtual ADDR_E_RETURNCODE HwlComputeSurfaceInfo(
        const ADDR_COMPUTE_SURFACE_INFO_INPUT* pIn,
        ADDR_COMPUTE_SURFACE_INFO_OUTPUT* pOut) const;

    virtual ADDR_E_RETURNCODE HwlComputeSurfaceAddrFromCoord(
        const ADDR_COMPUTE_SURFACE_ADDRFROMCOORD_INPUT* pIn,
        ADDR_COMPUTE_SURFACE_ADDRFROMCOORD_OUTPUT* pOut) const;

    virtual ADDR_E_RETURNCODE HwlComputeSurfaceCoordFromAddr(
        const ADDR_COMPUTE_SURFACE_COORDFROMADDR_INPUT* pIn,
        ADDR_COMPUTE_SURFACE_COORDFROMADDR_OUTPUT* pOut) const;

    virtual ADDR_E_RETURNCODE HwlComputeSliceTileSwizzle(
        const ADDR_COMPUTE_SLICESWIZZLE_INPUT* pIn,
        ADDR_COMPUTE_SLICESWIZZLE_OUTPUT* pOut) const;

    virtual ADDR_E_RETURNCODE HwlExtractBankPipeSwizzle(
        const ADDR_EXTRACT_BANKPIPE_SWIZZLE_INPUT* pIn,
        ADDR_EXTRACT_BANKPIPE_SWIZZLE_OUTPUT* pOut) const;

    virtual ADDR_E_RETURNCODE HwlCombineBankPipeSwizzle(
        UINT_32 bankSwizzle, UINT_32 pipeSwizzle, ADDR_TILEINFO*  pTileInfo,
        UINT_64 baseAddr, UINT_32* pTileSwizzle) const;

    virtual ADDR_E_RETURNCODE HwlComputeBaseSwizzle(
        const ADDR_COMPUTE_BASE_SWIZZLE_INPUT* pIn,
        ADDR_COMPUTE_BASE_SWIZZLE_OUTPUT* pOut) const;

    virtual ADDR_E_RETURNCODE HwlConvertTileInfoToHW(
        const ADDR_CONVERT_TILEINFOTOHW_INPUT* pIn,
        ADDR_CONVERT_TILEINFOTOHW_OUTPUT* pOut) const;

    virtual UINT_32 HwlComputeHtileBpp(
        BOOL_32 isWidth8, BOOL_32 isHeight8) const;

    virtual UINT_32 HwlComputeHtileBaseAlign(
        BOOL_32 isTcCompatible, BOOL_32 isLinear, ADDR_TILEINFO* pTileInfo) const;

    virtual ADDR_E_RETURNCODE HwlComputeFmaskInfo(
        const ADDR_COMPUTE_FMASK_INFO_INPUT* pIn,
        ADDR_COMPUTE_FMASK_INFO_OUTPUT* pOut);

    virtual ADDR_E_RETURNCODE HwlComputeFmaskAddrFromCoord(
        const ADDR_COMPUTE_FMASK_ADDRFROMCOORD_INPUT* pIn,
        ADDR_COMPUTE_FMASK_ADDRFROMCOORD_OUTPUT* pOut) const;

    virtual ADDR_E_RETURNCODE HwlComputeFmaskCoordFromAddr(
        const ADDR_COMPUTE_FMASK_COORDFROMADDR_INPUT* pIn,
        ADDR_COMPUTE_FMASK_COORDFROMADDR_OUTPUT* pOut) const;

    virtual BOOL_32 HwlGetAlignmentInfoMacroTiled(
        const ADDR_COMPUTE_SURFACE_INFO_INPUT* pIn,
        UINT_32* pPitchAlign, UINT_32* pHeightAlign, UINT_32* pSizeAlign) const;

    virtual UINT_32 HwlComputeQbStereoRightSwizzle(
        ADDR_COMPUTE_SURFACE_INFO_OUTPUT* pInfo) const;

    virtual VOID HwlComputePixelCoordFromOffset(
        UINT_32 offset, UINT_32 bpp, UINT_32 numSamples,
        AddrTileMode tileMode, UINT_32 tileBase, UINT_32 compBits,
        UINT_32* pX, UINT_32* pY, UINT_32* pSlice, UINT_32* pSample,
        AddrTileType microTileType, BOOL_32 isDepthSampleOrder) const;

    /// Return Cmask block max
    virtual BOOL_32 HwlGetMaxCmaskBlockMax() const
    {
        return 0x3FFF; // 14 bits, 0n16383
    }

    // Sub-hwl interface
    /// Pure virtual function to setup tile info (indices) if client requests to do so
    virtual VOID HwlSetupTileInfo(
        AddrTileMode tileMode, ADDR_SURFACE_FLAGS flags,
        UINT_32 bpp, UINT_32 pitch, UINT_32 height, UINT_32 numSamples,
        ADDR_TILEINFO* inputTileInfo, ADDR_TILEINFO* outputTileInfo,
        AddrTileType inTileType, ADDR_COMPUTE_SURFACE_INFO_OUTPUT* pOut) const = 0;

    /// Pure virtual function to get pitch alignment for linear modes
    virtual UINT_32 HwlGetPitchAlignmentLinear(UINT_32 bpp, ADDR_SURFACE_FLAGS flags) const = 0;

    /// Pure virtual function to get size adjustment for linear modes
    virtual UINT_64 HwlGetSizeAdjustmentLinear(
        AddrTileMode tileMode,
        UINT_32 bpp, UINT_32 numSamples, UINT_32 baseAlign, UINT_32 pitchAlign,
        UINT_32 *pPitch, UINT_32 *pHeight, UINT_32 *pHeightAlign) const = 0;

    virtual UINT_32 HwlGetPitchAlignmentMicroTiled(
        AddrTileMode tileMode, UINT_32 bpp, ADDR_SURFACE_FLAGS flags, UINT_32 numSamples) const;

    virtual UINT_64 HwlGetSizeAdjustmentMicroTiled(
        UINT_32 thickness, UINT_32 bpp, ADDR_SURFACE_FLAGS flags, UINT_32 numSamples,
        UINT_32 baseAlign, UINT_32 pitchAlign,
        UINT_32 *pPitch, UINT_32 *pHeight) const;

        /// Pure virtual function to do extra sanity check
    virtual BOOL_32 HwlSanityCheckMacroTiled(
        ADDR_TILEINFO* pTileInfo) const = 0;

    /// Pure virtual function to check current level to be the last macro tiled one
    virtual VOID HwlCheckLastMacroTiledLvl(
        const ADDR_COMPUTE_SURFACE_INFO_INPUT* pIn,
        ADDR_COMPUTE_SURFACE_INFO_OUTPUT* pOut) const = 0;

    /// Adjusts bank before bank is modified by rotation
    virtual UINT_32 HwlPreAdjustBank(
        UINT_32 tileX, UINT_32 bank, ADDR_TILEINFO*  pTileInfo) const = 0;

    virtual VOID HwlComputeSurfaceCoord2DFromBankPipe(
        AddrTileMode tileMode, UINT_32* pX, UINT_32* pY, UINT_32 slice,
        UINT_32 bank, UINT_32 pipe,
        UINT_32 bankSwizzle, UINT_32 pipeSwizzle, UINT_32 tileSlices,
        BOOL_32 ignoreSE,
        ADDR_TILEINFO* pTileInfo) const = 0;

    virtual BOOL_32 HwlTileInfoEqual(
        const ADDR_TILEINFO* pLeft, const ADDR_TILEINFO* pRight) const;

    virtual AddrTileMode HwlDegradeThickTileMode(
        AddrTileMode baseTileMode, UINT_32 numSlices, UINT_32* pBytesPerTile) const;

    virtual INT_32 HwlPostCheckTileIndex(
        const ADDR_TILEINFO* pInfo, AddrTileMode mode, AddrTileType type,
        INT curIndex = TileIndexInvalid) const
    {
        return TileIndexInvalid;
    }

    virtual VOID HwlFmaskPreThunkSurfInfo(
        const ADDR_COMPUTE_FMASK_INFO_INPUT* pFmaskIn,
        const ADDR_COMPUTE_FMASK_INFO_OUTPUT* pFmaskOut,
        ADDR_COMPUTE_SURFACE_INFO_INPUT* pSurfIn,
        ADDR_COMPUTE_SURFACE_INFO_OUTPUT* pSurfOut) const
    {
    }

    virtual VOID HwlFmaskPostThunkSurfInfo(
        const ADDR_COMPUTE_SURFACE_INFO_OUTPUT* pSurfOut,
        ADDR_COMPUTE_FMASK_INFO_OUTPUT* pFmaskOut) const
    {
    }

    virtual UINT_32 HwlStereoCheckRightOffsetPadding(ADDR_TILEINFO* pTileInfo) const;

    virtual BOOL_32 HwlReduceBankWidthHeight(
        UINT_32 tileSize, UINT_32 bpp, ADDR_SURFACE_FLAGS flags, UINT_32 numSamples,
        UINT_32 bankHeightAlign, UINT_32 pipes,
        ADDR_TILEINFO* pTileInfo) const;

    // Protected non-virtual functions

    /// Mip level functions
    AddrTileMode ComputeSurfaceMipLevelTileMode(
        AddrTileMode baseTileMode, UINT_32 bpp,
        UINT_32 pitch, UINT_32 height, UINT_32 numSlices, UINT_32 numSamples,
        UINT_32 pitchAlign, UINT_32 heightAlign,
        ADDR_TILEINFO* pTileInfo) const;

    /// Swizzle functions
    VOID ExtractBankPipeSwizzle(
        UINT_32 base256b, ADDR_TILEINFO* pTileInfo,
        UINT_32* pBankSwizzle, UINT_32* pPipeSwizzle) const;

    UINT_32 GetBankPipeSwizzle(
        UINT_32 bankSwizzle, UINT_32 pipeSwizzle,
        UINT_64 baseAddr, ADDR_TILEINFO*  pTileInfo) const;

    UINT_32 ComputeSliceTileSwizzle(
        AddrTileMode tileMode, UINT_32 baseSwizzle, UINT_32 slice, UINT_64 baseAddr,
        ADDR_TILEINFO* pTileInfo) const;

    /// Addressing functions
    virtual ADDR_E_RETURNCODE ComputeBankEquation(
        UINT_32 log2BytesPP, UINT_32 threshX, UINT_32 threshY,
        ADDR_TILEINFO* pTileInfo, ADDR_EQUATION* pEquation) const
    {
        return ADDR_NOTSUPPORTED;
    }

    UINT_32 ComputeBankFromCoord(
        UINT_32 x, UINT_32 y, UINT_32 slice,
        AddrTileMode tileMode, UINT_32 bankSwizzle, UINT_32 tileSpitSlice,
        ADDR_TILEINFO* pTileInfo) const;

    UINT_32 ComputeBankFromAddr(
        UINT_64 addr, UINT_32 numBanks, UINT_32 numPipes) const;

    UINT_32 ComputePipeRotation(
        AddrTileMode tileMode, UINT_32 numPipes) const;

    UINT_32 ComputeBankRotation(
        AddrTileMode tileMode, UINT_32 numBanks,
        UINT_32 numPipes) const;

    VOID ComputeSurfaceCoord2DFromBankPipe(
        AddrTileMode tileMode, UINT_32 x, UINT_32 y, UINT_32 slice,
        UINT_32 bank, UINT_32 pipe,
        UINT_32 bankSwizzle, UINT_32 pipeSwizzle, UINT_32 tileSlices,
        ADDR_TILEINFO* pTileInfo,
        CoordFromBankPipe *pOutput) const;

    /// Htile/Cmask functions
    UINT_64 ComputeHtileBytes(
        UINT_32 pitch, UINT_32 height, UINT_32 bpp,
        BOOL_32 isLinear, UINT_32 numSlices, UINT_64* sliceBytes, UINT_32 baseAlign) const;

    ADDR_E_RETURNCODE ComputeMacroTileEquation(
        UINT_32 log2BytesPP, AddrTileMode tileMode, AddrTileType microTileType,
        ADDR_TILEINFO* pTileInfo, ADDR_EQUATION* pEquation) const;

    // Static functions
    static BOOL_32 IsTileInfoAllZero(const ADDR_TILEINFO* pTileInfo);
    static UINT_32 ComputeFmaskNumPlanesFromNumSamples(UINT_32 numSamples);
    static UINT_32 ComputeFmaskResolvedBppFromNumSamples(UINT_32 numSamples);

    virtual VOID HwlComputeSurfaceAlignmentsMacroTiled(
        AddrTileMode tileMode, UINT_32 bpp, ADDR_SURFACE_FLAGS flags,
        UINT_32 mipLevel, UINT_32 numSamples, ADDR_COMPUTE_SURFACE_INFO_OUTPUT* pOut) const
    {
    }

private:

    BOOL_32 ComputeSurfaceInfoLinear(
        const ADDR_COMPUTE_SURFACE_INFO_INPUT* pIn,
        ADDR_COMPUTE_SURFACE_INFO_OUTPUT* pOut,
        UINT_32 padDims) const;

    BOOL_32 ComputeSurfaceInfoMicroTiled(
        const ADDR_COMPUTE_SURFACE_INFO_INPUT* pIn,
        ADDR_COMPUTE_SURFACE_INFO_OUTPUT* pOut,
        UINT_32 padDims,
        AddrTileMode expTileMode) const;

    BOOL_32 ComputeSurfaceInfoMacroTiled(
        const ADDR_COMPUTE_SURFACE_INFO_INPUT* pIn,
        ADDR_COMPUTE_SURFACE_INFO_OUTPUT* pOut,
        UINT_32 padDims,
        AddrTileMode expTileMode) const;

    BOOL_32 ComputeSurfaceAlignmentsLinear(
        AddrTileMode tileMode, UINT_32 bpp, ADDR_SURFACE_FLAGS flags,
        UINT_32* pBaseAlign, UINT_32* pPitchAlign, UINT_32* pHeightAlign) const;

    BOOL_32 ComputeSurfaceAlignmentsMicroTiled(
        AddrTileMode tileMode, UINT_32 bpp, ADDR_SURFACE_FLAGS flags,
        UINT_32 mipLevel, UINT_32 numSamples,
        UINT_32* pBaseAlign, UINT_32* pPitchAlign, UINT_32* pHeightAlign) const;

    BOOL_32 ComputeSurfaceAlignmentsMacroTiled(
        AddrTileMode tileMode, UINT_32 bpp, ADDR_SURFACE_FLAGS flags,
        UINT_32 mipLevel, UINT_32 numSamples,
        ADDR_COMPUTE_SURFACE_INFO_OUTPUT* pOut) const;

    /// Surface addressing functions
    UINT_64 DispatchComputeSurfaceAddrFromCoord(
        const ADDR_COMPUTE_SURFACE_ADDRFROMCOORD_INPUT* pIn,
        ADDR_COMPUTE_SURFACE_ADDRFROMCOORD_OUTPUT* pOut) const;

    VOID DispatchComputeSurfaceCoordFromAddr(
        const ADDR_COMPUTE_SURFACE_COORDFROMADDR_INPUT* pIn,
        ADDR_COMPUTE_SURFACE_COORDFROMADDR_OUTPUT* pOut) const;

    UINT_64 ComputeSurfaceAddrFromCoordMicroTiled(
        UINT_32 x, UINT_32 y, UINT_32 slice, UINT_32 sample,
        UINT_32 bpp, UINT_32 pitch, UINT_32 height, UINT_32 numSamples,
        AddrTileMode tileMode,
        AddrTileType microTileType, BOOL_32 isDepthSampleOrder,
        UINT_32* pBitPosition) const;

    UINT_64 ComputeSurfaceAddrFromCoordMacroTiled(
        UINT_32 x, UINT_32 y, UINT_32 slice, UINT_32 sample,
        UINT_32 bpp, UINT_32 pitch, UINT_32 height, UINT_32 numSamples,
        AddrTileMode tileMode,
        AddrTileType microTileType, BOOL_32 ignoreSE, BOOL_32 isDepthSampleOrder,
        UINT_32 pipeSwizzle, UINT_32 bankSwizzle,
        ADDR_TILEINFO* pTileInfo,
        UINT_32* pBitPosition) const;

    VOID ComputeSurfaceCoordFromAddrMacroTiled(
        UINT_64 addr, UINT_32 bitPosition,
        UINT_32 bpp, UINT_32 pitch, UINT_32 height, UINT_32 numSamples,
        AddrTileMode tileMode, UINT_32 tileBase, UINT_32 compBits,
        AddrTileType microTileType, BOOL_32 ignoreSE, BOOL_32 isDepthSampleOrder,
        UINT_32 pipeSwizzle, UINT_32 bankSwizzle,
        ADDR_TILEINFO* pTileInfo,
        UINT_32* pX, UINT_32* pY, UINT_32* pSlice, UINT_32* pSample) const;

    /// Fmask functions
    UINT_64 DispatchComputeFmaskAddrFromCoord(
        const ADDR_COMPUTE_FMASK_ADDRFROMCOORD_INPUT* pIn,
        ADDR_COMPUTE_FMASK_ADDRFROMCOORD_OUTPUT* pOut) const;

    VOID DispatchComputeFmaskCoordFromAddr(
        const ADDR_COMPUTE_FMASK_COORDFROMADDR_INPUT* pIn,
        ADDR_COMPUTE_FMASK_COORDFROMADDR_OUTPUT* pOut) const;

    // FMASK related methods - private
    UINT_64 ComputeFmaskAddrFromCoordMicroTiled(
        UINT_32 x, UINT_32 y, UINT_32 slice, UINT_32 sample, UINT_32 plane,
        UINT_32 pitch, UINT_32 height, UINT_32 numSamples, AddrTileMode tileMode,
        BOOL_32 resolved, UINT_32* pBitPosition) const;

    VOID ComputeFmaskCoordFromAddrMicroTiled(
        UINT_64 addr, UINT_32 bitPosition,
        UINT_32 pitch, UINT_32 height, UINT_32 numSamples,
        AddrTileMode tileMode, BOOL_32 resolved,
        UINT_32* pX, UINT_32* pY, UINT_32* pSlice, UINT_32* pSample, UINT_32* pPlane) const;

    VOID ComputeFmaskCoordFromAddrMacroTiled(
        UINT_64 addr, UINT_32 bitPosition,
        UINT_32 pitch, UINT_32 height, UINT_32 numSamples, AddrTileMode tileMode,
        UINT_32 pipeSwizzle, UINT_32 bankSwizzle,
        BOOL_32 ignoreSE,
        ADDR_TILEINFO* pTileInfo,
        BOOL_32 resolved,
        UINT_32* pX, UINT_32* pY, UINT_32* pSlice, UINT_32* pSample, UINT_32* pPlane) const;

    UINT_64 ComputeFmaskAddrFromCoordMacroTiled(
        UINT_32 x, UINT_32 y, UINT_32 slice, UINT_32 sample, UINT_32 plane,
        UINT_32 pitch, UINT_32 height, UINT_32 numSamples,
        AddrTileMode tileMode, UINT_32 pipeSwizzle, UINT_32 bankSwizzle,
        BOOL_32 ignoreSE,
        ADDR_TILEINFO* pTileInfo,
        BOOL_32 resolved,
        UINT_32* pBitPosition) const;

    /// Sanity check functions
    BOOL_32 SanityCheckMacroTiled(
        ADDR_TILEINFO* pTileInfo) const;

protected:
    UINT_32 m_ranks;                ///< Number of ranks - MC_ARB_RAMCFG.NOOFRANK
    UINT_32 m_logicalBanks;         ///< Logical banks = m_banks * m_ranks if m_banks != 16
    UINT_32 m_bankInterleave;       ///< Bank interleave, as a multiple of pipe interleave size
};

} // V1
} // Addr

#endif

