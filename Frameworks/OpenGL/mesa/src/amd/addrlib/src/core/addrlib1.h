/*
************************************************************************************************************************
*
*  Copyright (C) 2007-2022 Advanced Micro Devices, Inc.  All rights reserved.
*  SPDX-License-Identifier: MIT
*
***********************************************************************************************************************/


/**
****************************************************************************************************
* @file  addrlib1.h
* @brief Contains the Addr::V1::Lib class definition.
****************************************************************************************************
*/

#ifndef __ADDR_LIB1_H__
#define __ADDR_LIB1_H__

#include "addrlib.h"

namespace Addr
{
namespace V1
{

/**
****************************************************************************************************
* @brief Neutral enums that define bank swap size
****************************************************************************************************
*/
enum SampleSplitSize
{
    ADDR_SAMPLESPLIT_1KB = 1024,
    ADDR_SAMPLESPLIT_2KB = 2048,
    ADDR_SAMPLESPLIT_4KB = 4096,
    ADDR_SAMPLESPLIT_8KB = 8192,
};

/**
****************************************************************************************************
* @brief Flags for AddrTileMode
****************************************************************************************************
*/
struct TileModeFlags
{
    UINT_32 thickness       : 4;
    UINT_32 isLinear        : 1;
    UINT_32 isMicro         : 1;
    UINT_32 isMacro         : 1;
    UINT_32 isMacro3d       : 1;
    UINT_32 isPrt           : 1;
    UINT_32 isPrtNoRotation : 1;
    UINT_32 isBankSwapped   : 1;
};

static const UINT_32 Block64K = 0x10000;
static const UINT_32 PrtTileSize = Block64K;

/**
****************************************************************************************************
* @brief This class contains asic independent address lib functionalities
****************************************************************************************************
*/
class Lib : public Addr::Lib
{
public:
    virtual ~Lib();

    static Lib* GetLib(
        ADDR_HANDLE hLib);

    /// Returns tileIndex support
    BOOL_32 UseTileIndex(INT_32 index) const
    {
        return m_configFlags.useTileIndex && (index != TileIndexInvalid);
    }

    /// Returns combined swizzle support
    BOOL_32 UseCombinedSwizzle() const
    {
        return m_configFlags.useCombinedSwizzle;
    }

    //
    // Interface stubs
    //
    ADDR_E_RETURNCODE ComputeSurfaceInfo(
        const ADDR_COMPUTE_SURFACE_INFO_INPUT* pIn,
        ADDR_COMPUTE_SURFACE_INFO_OUTPUT* pOut) const;

    ADDR_E_RETURNCODE ComputeSurfaceAddrFromCoord(
        const ADDR_COMPUTE_SURFACE_ADDRFROMCOORD_INPUT* pIn,
        ADDR_COMPUTE_SURFACE_ADDRFROMCOORD_OUTPUT* pOut) const;

    ADDR_E_RETURNCODE ComputeSurfaceCoordFromAddr(
        const ADDR_COMPUTE_SURFACE_COORDFROMADDR_INPUT*  pIn,
        ADDR_COMPUTE_SURFACE_COORDFROMADDR_OUTPUT* pOut) const;

    ADDR_E_RETURNCODE ComputeSliceTileSwizzle(
        const ADDR_COMPUTE_SLICESWIZZLE_INPUT*  pIn,
        ADDR_COMPUTE_SLICESWIZZLE_OUTPUT* pOut) const;

    ADDR_E_RETURNCODE ExtractBankPipeSwizzle(
        const ADDR_EXTRACT_BANKPIPE_SWIZZLE_INPUT* pIn,
        ADDR_EXTRACT_BANKPIPE_SWIZZLE_OUTPUT* pOut) const;

    ADDR_E_RETURNCODE CombineBankPipeSwizzle(
        const ADDR_COMBINE_BANKPIPE_SWIZZLE_INPUT*  pIn,
        ADDR_COMBINE_BANKPIPE_SWIZZLE_OUTPUT* pOut) const;

    ADDR_E_RETURNCODE ComputeBaseSwizzle(
        const ADDR_COMPUTE_BASE_SWIZZLE_INPUT*  pIn,
        ADDR_COMPUTE_BASE_SWIZZLE_OUTPUT* pOut) const;

    ADDR_E_RETURNCODE ComputeFmaskInfo(
        const ADDR_COMPUTE_FMASK_INFO_INPUT*  pIn,
        ADDR_COMPUTE_FMASK_INFO_OUTPUT* pOut);

    ADDR_E_RETURNCODE ComputeFmaskAddrFromCoord(
        const ADDR_COMPUTE_FMASK_ADDRFROMCOORD_INPUT*  pIn,
        ADDR_COMPUTE_FMASK_ADDRFROMCOORD_OUTPUT* pOut) const;

    ADDR_E_RETURNCODE ComputeFmaskCoordFromAddr(
        const ADDR_COMPUTE_FMASK_COORDFROMADDR_INPUT*  pIn,
        ADDR_COMPUTE_FMASK_COORDFROMADDR_OUTPUT* pOut) const;

    ADDR_E_RETURNCODE ConvertTileInfoToHW(
        const ADDR_CONVERT_TILEINFOTOHW_INPUT* pIn,
        ADDR_CONVERT_TILEINFOTOHW_OUTPUT* pOut) const;

    ADDR_E_RETURNCODE ConvertTileIndex(
        const ADDR_CONVERT_TILEINDEX_INPUT* pIn,
        ADDR_CONVERT_TILEINDEX_OUTPUT* pOut) const;

    ADDR_E_RETURNCODE GetMacroModeIndex(
        const ADDR_GET_MACROMODEINDEX_INPUT* pIn,
        ADDR_GET_MACROMODEINDEX_OUTPUT* pOut) const;

    ADDR_E_RETURNCODE ConvertTileIndex1(
        const ADDR_CONVERT_TILEINDEX1_INPUT* pIn,
        ADDR_CONVERT_TILEINDEX_OUTPUT* pOut) const;

    ADDR_E_RETURNCODE GetTileIndex(
        const ADDR_GET_TILEINDEX_INPUT* pIn,
        ADDR_GET_TILEINDEX_OUTPUT* pOut) const;

    ADDR_E_RETURNCODE ComputeHtileInfo(
        const ADDR_COMPUTE_HTILE_INFO_INPUT* pIn,
        ADDR_COMPUTE_HTILE_INFO_OUTPUT* pOut) const;

    ADDR_E_RETURNCODE ComputeCmaskInfo(
        const ADDR_COMPUTE_CMASK_INFO_INPUT* pIn,
        ADDR_COMPUTE_CMASK_INFO_OUTPUT* pOut) const;

    ADDR_E_RETURNCODE ComputeDccInfo(
        const ADDR_COMPUTE_DCCINFO_INPUT* pIn,
        ADDR_COMPUTE_DCCINFO_OUTPUT* pOut) const;

    ADDR_E_RETURNCODE ComputeHtileAddrFromCoord(
        const ADDR_COMPUTE_HTILE_ADDRFROMCOORD_INPUT*  pIn,
        ADDR_COMPUTE_HTILE_ADDRFROMCOORD_OUTPUT* pOut) const;

    ADDR_E_RETURNCODE ComputeCmaskAddrFromCoord(
        const ADDR_COMPUTE_CMASK_ADDRFROMCOORD_INPUT*  pIn,
        ADDR_COMPUTE_CMASK_ADDRFROMCOORD_OUTPUT* pOut) const;

    ADDR_E_RETURNCODE ComputeHtileCoordFromAddr(
        const ADDR_COMPUTE_HTILE_COORDFROMADDR_INPUT*  pIn,
        ADDR_COMPUTE_HTILE_COORDFROMADDR_OUTPUT* pOut) const;

    ADDR_E_RETURNCODE ComputeCmaskCoordFromAddr(
        const ADDR_COMPUTE_CMASK_COORDFROMADDR_INPUT*  pIn,
        ADDR_COMPUTE_CMASK_COORDFROMADDR_OUTPUT* pOut) const;

    ADDR_E_RETURNCODE ComputePrtInfo(
        const ADDR_PRT_INFO_INPUT*  pIn,
        ADDR_PRT_INFO_OUTPUT*       pOut) const;
protected:
    Lib();  // Constructor is protected
    Lib(const Client* pClient);

    /// Pure Virtual function for Hwl computing surface info
    virtual ADDR_E_RETURNCODE HwlComputeSurfaceInfo(
        const ADDR_COMPUTE_SURFACE_INFO_INPUT* pIn,
        ADDR_COMPUTE_SURFACE_INFO_OUTPUT* pOut) const = 0;

    /// Pure Virtual function for Hwl computing surface address from coord
    virtual ADDR_E_RETURNCODE HwlComputeSurfaceAddrFromCoord(
        const ADDR_COMPUTE_SURFACE_ADDRFROMCOORD_INPUT* pIn,
        ADDR_COMPUTE_SURFACE_ADDRFROMCOORD_OUTPUT* pOut) const = 0;

    /// Pure Virtual function for Hwl computing surface coord from address
    virtual ADDR_E_RETURNCODE HwlComputeSurfaceCoordFromAddr(
        const ADDR_COMPUTE_SURFACE_COORDFROMADDR_INPUT* pIn,
        ADDR_COMPUTE_SURFACE_COORDFROMADDR_OUTPUT* pOut) const = 0;

    /// Pure Virtual function for Hwl computing surface tile swizzle
    virtual ADDR_E_RETURNCODE HwlComputeSliceTileSwizzle(
        const ADDR_COMPUTE_SLICESWIZZLE_INPUT* pIn,
        ADDR_COMPUTE_SLICESWIZZLE_OUTPUT* pOut) const = 0;

    /// Pure Virtual function for Hwl extracting bank/pipe swizzle from base256b
    virtual ADDR_E_RETURNCODE HwlExtractBankPipeSwizzle(
        const ADDR_EXTRACT_BANKPIPE_SWIZZLE_INPUT* pIn,
        ADDR_EXTRACT_BANKPIPE_SWIZZLE_OUTPUT* pOut) const = 0;

    /// Pure Virtual function for Hwl combining bank/pipe swizzle
    virtual ADDR_E_RETURNCODE HwlCombineBankPipeSwizzle(
        UINT_32 bankSwizzle, UINT_32 pipeSwizzle, ADDR_TILEINFO*  pTileInfo,
        UINT_64 baseAddr, UINT_32* pTileSwizzle) const = 0;

    /// Pure Virtual function for Hwl computing base swizzle
    virtual ADDR_E_RETURNCODE HwlComputeBaseSwizzle(
        const ADDR_COMPUTE_BASE_SWIZZLE_INPUT* pIn,
        ADDR_COMPUTE_BASE_SWIZZLE_OUTPUT* pOut) const = 0;

    /// Pure Virtual function for Hwl computing HTILE base align
    virtual UINT_32 HwlComputeHtileBaseAlign(
        BOOL_32 isTcCompatible, BOOL_32 isLinear, ADDR_TILEINFO* pTileInfo) const = 0;

    /// Pure Virtual function for Hwl computing HTILE bpp
    virtual UINT_32 HwlComputeHtileBpp(
        BOOL_32 isWidth8, BOOL_32 isHeight8) const = 0;

    /// Pure Virtual function for Hwl computing HTILE bytes
    virtual UINT_64 HwlComputeHtileBytes(
        UINT_32 pitch, UINT_32 height, UINT_32 bpp,
        BOOL_32 isLinear, UINT_32 numSlices, UINT_64* pSliceBytes, UINT_32 baseAlign) const = 0;

    /// Pure Virtual function for Hwl computing FMASK info
    virtual ADDR_E_RETURNCODE HwlComputeFmaskInfo(
        const ADDR_COMPUTE_FMASK_INFO_INPUT* pIn,
        ADDR_COMPUTE_FMASK_INFO_OUTPUT* pOut) = 0;

    /// Pure Virtual function for Hwl FMASK address from coord
    virtual ADDR_E_RETURNCODE HwlComputeFmaskAddrFromCoord(
        const ADDR_COMPUTE_FMASK_ADDRFROMCOORD_INPUT* pIn,
        ADDR_COMPUTE_FMASK_ADDRFROMCOORD_OUTPUT* pOut) const = 0;

    /// Pure Virtual function for Hwl FMASK coord from address
    virtual ADDR_E_RETURNCODE HwlComputeFmaskCoordFromAddr(
        const ADDR_COMPUTE_FMASK_COORDFROMADDR_INPUT* pIn,
        ADDR_COMPUTE_FMASK_COORDFROMADDR_OUTPUT* pOut) const = 0;

    /// Pure Virtual function for Hwl convert tile info from real value to HW value
    virtual ADDR_E_RETURNCODE HwlConvertTileInfoToHW(
        const ADDR_CONVERT_TILEINFOTOHW_INPUT* pIn,
        ADDR_CONVERT_TILEINFOTOHW_OUTPUT* pOut) const = 0;

    /// Pure Virtual function for Hwl compute mipmap info
    virtual BOOL_32 HwlComputeMipLevel(
        ADDR_COMPUTE_SURFACE_INFO_INPUT* pIn) const = 0;

    /// Pure Virtual function for Hwl compute max cmask blockMax value
    virtual BOOL_32 HwlGetMaxCmaskBlockMax() const = 0;

    /// Pure Virtual function for Hwl compute fmask bits
    virtual UINT_32 HwlComputeFmaskBits(
        const ADDR_COMPUTE_FMASK_INFO_INPUT* pIn,
        UINT_32* pNumSamples) const = 0;

    /// Virtual function to get index (not pure then no need to implement this in all hwls
    virtual ADDR_E_RETURNCODE HwlGetTileIndex(
        const ADDR_GET_TILEINDEX_INPUT* pIn,
        ADDR_GET_TILEINDEX_OUTPUT*      pOut) const
    {
        return ADDR_NOTSUPPORTED;
    }

    /// Virtual function for Hwl to compute Dcc info
    virtual ADDR_E_RETURNCODE HwlComputeDccInfo(
        const ADDR_COMPUTE_DCCINFO_INPUT* pIn,
        ADDR_COMPUTE_DCCINFO_OUTPUT* pOut) const
    {
        return ADDR_NOTSUPPORTED;
    }

    /// Virtual function to get cmask address for tc compatible cmask
    virtual ADDR_E_RETURNCODE HwlComputeCmaskAddrFromCoord(
        const ADDR_COMPUTE_CMASK_ADDRFROMCOORD_INPUT* pIn,
        ADDR_COMPUTE_CMASK_ADDRFROMCOORD_OUTPUT* pOut) const
    {
        return ADDR_NOTSUPPORTED;
    }

    /// Virtual function to get htile address for tc compatible htile
    virtual ADDR_E_RETURNCODE HwlComputeHtileAddrFromCoord(
        const ADDR_COMPUTE_HTILE_ADDRFROMCOORD_INPUT* pIn,
        ADDR_COMPUTE_HTILE_ADDRFROMCOORD_OUTPUT* pOut) const
    {
        return ADDR_NOTSUPPORTED;
    }

    // Compute attributes

    // HTILE
    UINT_32    ComputeHtileInfo(
        ADDR_HTILE_FLAGS flags,
        UINT_32 pitchIn, UINT_32 heightIn, UINT_32 numSlices,
        BOOL_32 isLinear, BOOL_32 isWidth8, BOOL_32 isHeight8,
        ADDR_TILEINFO*  pTileInfo,
        UINT_32* pPitchOut, UINT_32* pHeightOut, UINT_64* pHtileBytes,
        UINT_32* pMacroWidth = NULL, UINT_32* pMacroHeight = NULL,
        UINT_64* pSliceSize = NULL, UINT_32* pBaseAlign = NULL) const;

    // CMASK
    ADDR_E_RETURNCODE ComputeCmaskInfo(
        ADDR_CMASK_FLAGS flags,
        UINT_32 pitchIn, UINT_32 heightIn, UINT_32 numSlices, BOOL_32 isLinear,
        ADDR_TILEINFO* pTileInfo, UINT_32* pPitchOut, UINT_32* pHeightOut, UINT_64* pCmaskBytes,
        UINT_32* pMacroWidth, UINT_32* pMacroHeight, UINT_64* pSliceSize = NULL,
        UINT_32* pBaseAlign = NULL, UINT_32* pBlockMax = NULL) const;

    virtual VOID HwlComputeTileDataWidthAndHeightLinear(
        UINT_32* pMacroWidth, UINT_32* pMacroHeight,
        UINT_32 bpp, ADDR_TILEINFO* pTileInfo) const;

    // CMASK & HTILE addressing
    virtual UINT_64 HwlComputeXmaskAddrFromCoord(
        UINT_32 pitch, UINT_32 height, UINT_32 x, UINT_32 y, UINT_32 slice,
        UINT_32 numSlices, UINT_32 factor, BOOL_32 isLinear, BOOL_32 isWidth8,
        BOOL_32 isHeight8, ADDR_TILEINFO* pTileInfo,
        UINT_32* bitPosition) const;

    virtual VOID HwlComputeXmaskCoordFromAddr(
        UINT_64 addr, UINT_32 bitPosition, UINT_32 pitch, UINT_32 height, UINT_32 numSlices,
        UINT_32 factor, BOOL_32 isLinear, BOOL_32 isWidth8, BOOL_32 isHeight8,
        ADDR_TILEINFO* pTileInfo, UINT_32* pX, UINT_32* pY, UINT_32* pSlice) const;

    // Surface mipmap
    VOID    ComputeMipLevel(
        ADDR_COMPUTE_SURFACE_INFO_INPUT* pIn) const;

    /// Pure Virtual function for Hwl to get macro tiled alignment info
    virtual BOOL_32 HwlGetAlignmentInfoMacroTiled(
        const ADDR_COMPUTE_SURFACE_INFO_INPUT* pIn,
        UINT_32* pPitchAlign, UINT_32* pHeightAlign, UINT_32* pSizeAlign) const = 0;


    virtual VOID HwlOverrideTileMode(ADDR_COMPUTE_SURFACE_INFO_INPUT* pInOut) const
    {
        // not supported in hwl layer
    }

    virtual VOID HwlOptimizeTileMode(ADDR_COMPUTE_SURFACE_INFO_INPUT* pInOut) const
    {
        // not supported in hwl layer
    }

    virtual VOID HwlSelectTileMode(ADDR_COMPUTE_SURFACE_INFO_INPUT* pInOut) const
    {
        // not supported in hwl layer
    }

    AddrTileMode DegradeLargeThickTile(AddrTileMode tileMode, UINT_32 bpp) const;

    VOID PadDimensions(
        AddrTileMode tileMode, UINT_32 bpp, ADDR_SURFACE_FLAGS flags,
        UINT_32 numSamples, ADDR_TILEINFO* pTileInfo, UINT_32 padDims, UINT_32 mipLevel,
        UINT_32* pPitch, UINT_32* pPitchAlign, UINT_32* pHeight, UINT_32 heightAlign,
        UINT_32* pSlices, UINT_32 sliceAlign) const;

    virtual VOID HwlPadDimensions(
        AddrTileMode tileMode, UINT_32 bpp, ADDR_SURFACE_FLAGS flags,
        UINT_32 numSamples, ADDR_TILEINFO* pTileInfo, UINT_32 mipLevel,
        UINT_32* pPitch, UINT_32* pPitchAlign, UINT_32 height, UINT_32 heightAlign) const
    {
    }

    //
    // Addressing shared for linear/1D tiling
    //
    UINT_64 ComputeSurfaceAddrFromCoordLinear(
        UINT_32 x, UINT_32 y, UINT_32 slice, UINT_32 sample,
        UINT_32 bpp, UINT_32 pitch, UINT_32 height, UINT_32 numSlices,
        UINT_32* pBitPosition) const;

    VOID    ComputeSurfaceCoordFromAddrLinear(
        UINT_64 addr, UINT_32 bitPosition, UINT_32 bpp,
        UINT_32 pitch, UINT_32 height, UINT_32 numSlices,
        UINT_32* pX, UINT_32* pY, UINT_32* pSlice, UINT_32* pSample) const;

    VOID    ComputeSurfaceCoordFromAddrMicroTiled(
        UINT_64 addr, UINT_32 bitPosition,
        UINT_32 bpp, UINT_32 pitch, UINT_32 height, UINT_32 numSamples,
        AddrTileMode tileMode, UINT_32 tileBase, UINT_32 compBits,
        UINT_32* pX, UINT_32* pY, UINT_32* pSlice, UINT_32* pSample,
        AddrTileType microTileType, BOOL_32 isDepthSampleOrder) const;

    ADDR_E_RETURNCODE ComputeMicroTileEquation(
        UINT_32 bpp, AddrTileMode tileMode,
        AddrTileType microTileType, ADDR_EQUATION* pEquation) const;

    UINT_32 ComputePixelIndexWithinMicroTile(
        UINT_32 x, UINT_32 y, UINT_32 z,
        UINT_32 bpp, AddrTileMode tileMode, AddrTileType microTileType) const;

    /// Pure Virtual function for Hwl computing coord from offset inside micro tile
    virtual VOID HwlComputePixelCoordFromOffset(
        UINT_32 offset, UINT_32 bpp, UINT_32 numSamples,
        AddrTileMode tileMode, UINT_32 tileBase, UINT_32 compBits,
        UINT_32* pX, UINT_32* pY, UINT_32* pSlice, UINT_32* pSample,
        AddrTileType microTileType, BOOL_32 isDepthSampleOrder) const = 0;

    //
    // Addressing shared by all
    //
    virtual UINT_32 HwlGetPipes(
        const ADDR_TILEINFO* pTileInfo) const;

    UINT_32 ComputePipeFromAddr(
        UINT_64 addr, UINT_32 numPipes) const;

    virtual ADDR_E_RETURNCODE ComputePipeEquation(
        UINT_32 log2BytesPP, UINT_32 threshX, UINT_32 threshY, ADDR_TILEINFO* pTileInfo, ADDR_EQUATION* pEquation) const
    {
        return ADDR_NOTSUPPORTED;
    }

    /// Pure Virtual function for Hwl computing pipe from coord
    virtual UINT_32 ComputePipeFromCoord(
        UINT_32 x, UINT_32 y, UINT_32 slice, AddrTileMode tileMode,
        UINT_32 pipeSwizzle, BOOL_32 flags, ADDR_TILEINFO* pTileInfo) const = 0;

    /// Pure Virtual function for Hwl computing coord Y for 8 pipe cmask/htile
    virtual UINT_32 HwlComputeXmaskCoordYFrom8Pipe(
        UINT_32 pipe, UINT_32 x) const = 0;

    //
    // Misc helper
    //
    static const TileModeFlags ModeFlags[ADDR_TM_COUNT];

    static UINT_32 Thickness(
        AddrTileMode tileMode);

    // Checking tile mode
    static BOOL_32 IsMacroTiled(AddrTileMode tileMode);
    static BOOL_32 IsMacro3dTiled(AddrTileMode tileMode);
    static BOOL_32 IsLinear(AddrTileMode tileMode);
    static BOOL_32 IsMicroTiled(AddrTileMode tileMode);
    static BOOL_32 IsPrtTileMode(AddrTileMode tileMode);
    static BOOL_32 IsPrtNoRotationTileMode(AddrTileMode tileMode);

    /// Return TRUE if tile info is needed
    BOOL_32 UseTileInfo() const
    {
        return !m_configFlags.ignoreTileInfo;
    }

    /// Adjusts pitch alignment for flipping surface
    VOID    AdjustPitchAlignment(
        ADDR_SURFACE_FLAGS flags, UINT_32* pPitchAlign) const;

    /// Overwrite tile config according to tile index
    virtual ADDR_E_RETURNCODE HwlSetupTileCfg(
        UINT_32 bpp, INT_32 index, INT_32 macroModeIndex,
        ADDR_TILEINFO* pInfo, AddrTileMode* mode = NULL, AddrTileType* type = NULL) const;

    /// Overwrite macro tile config according to tile index
    virtual INT_32 HwlComputeMacroModeIndex(
        INT_32 index, ADDR_SURFACE_FLAGS flags, UINT_32 bpp, UINT_32 numSamples,
        ADDR_TILEINFO* pTileInfo, AddrTileMode *pTileMode = NULL, AddrTileType *pTileType = NULL
        ) const
    {
        return TileIndexNoMacroIndex;
    }

    /// Pre-handler of 3x pitch (96 bit) adjustment
    virtual UINT_32 HwlPreHandleBaseLvl3xPitch(
        const ADDR_COMPUTE_SURFACE_INFO_INPUT* pIn, UINT_32 expPitch) const;
    /// Post-handler of 3x pitch adjustment
    virtual UINT_32 HwlPostHandleBaseLvl3xPitch(
        const ADDR_COMPUTE_SURFACE_INFO_INPUT* pIn, UINT_32 expPitch) const;
    /// Check miplevel after surface adjustment
    ADDR_E_RETURNCODE PostComputeMipLevel(
        ADDR_COMPUTE_SURFACE_INFO_INPUT* pIn,
        ADDR_COMPUTE_SURFACE_INFO_OUTPUT* pOut) const;

    /// Quad buffer stereo support, has its implementation in ind. layer
    VOID ComputeQbStereoInfo(
        ADDR_COMPUTE_SURFACE_INFO_OUTPUT* pOut) const;

    /// Pure virutual function to compute stereo bank swizzle for right eye
    virtual UINT_32 HwlComputeQbStereoRightSwizzle(
        ADDR_COMPUTE_SURFACE_INFO_OUTPUT* pOut) const = 0;

    VOID OptimizeTileMode(ADDR_COMPUTE_SURFACE_INFO_INPUT* pInOut) const;

    /// Overwrite tile setting to PRT
    virtual VOID HwlSetPrtTileMode(ADDR_COMPUTE_SURFACE_INFO_INPUT* pInOut) const
    {
    }

    static BOOL_32 DegradeTo1D(
        UINT_32 width, UINT_32 height,
        UINT_32 macroTilePitchAlign, UINT_32 macroTileHeightAlign);

private:
    // Disallow the copy constructor
    Lib(const Lib& a);

    // Disallow the assignment operator
    Lib& operator=(const Lib& a);

    UINT_32 ComputeCmaskBaseAlign(
        ADDR_CMASK_FLAGS flags, ADDR_TILEINFO*  pTileInfo) const;

    UINT_64 ComputeCmaskBytes(
        UINT_32 pitch, UINT_32 height, UINT_32 numSlices) const;

    //
    // CMASK/HTILE shared methods
    //
    VOID    ComputeTileDataWidthAndHeight(
        UINT_32 bpp, UINT_32 cacheBits, ADDR_TILEINFO* pTileInfo,
        UINT_32* pMacroWidth, UINT_32* pMacroHeight) const;

    UINT_32 ComputeXmaskCoordYFromPipe(
        UINT_32 pipe, UINT_32 x) const;
};

} // V1
} // Addr

#endif

