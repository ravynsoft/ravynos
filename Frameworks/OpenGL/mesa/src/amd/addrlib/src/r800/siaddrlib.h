/*
************************************************************************************************************************
*
*  Copyright (C) 2007-2022 Advanced Micro Devices, Inc.  All rights reserved.
*  SPDX-License-Identifier: MIT
*
***********************************************************************************************************************/

/**
****************************************************************************************************
* @file  siaddrlib.h
* @brief Contains the R800Lib class definition.
****************************************************************************************************
*/

#ifndef __SI_ADDR_LIB_H__
#define __SI_ADDR_LIB_H__

#include "addrlib1.h"
#include "egbaddrlib.h"

namespace Addr
{
namespace V1
{

/**
****************************************************************************************************
* @brief Describes the information in tile mode table
****************************************************************************************************
*/
struct TileConfig
{
    AddrTileMode  mode;
    AddrTileType  type;
    ADDR_TILEINFO info;
};

/**
****************************************************************************************************
* @brief SI specific settings structure.
****************************************************************************************************
*/
struct SiChipSettings
{
    UINT_32 isSouthernIsland  : 1;
    UINT_32 isTahiti          : 1;
    UINT_32 isPitCairn        : 1;
    UINT_32 isCapeVerde       : 1;
    // Oland/Hainan are of GFXIP 6.0, similar with SI
    UINT_32 isOland           : 1;
    UINT_32 isHainan          : 1;

    // CI
    UINT_32 isSeaIsland       : 1;
    UINT_32 isBonaire         : 1;
    UINT_32 isKaveri          : 1;
    UINT_32 isSpectre         : 1;
    UINT_32 isSpooky          : 1;
    UINT_32 isKalindi         : 1;
    UINT_32 isHawaii          : 1;

    // VI
    UINT_32 isVolcanicIslands : 1;
    UINT_32 isIceland         : 1;
    UINT_32 isTonga           : 1;
    UINT_32 isFiji            : 1;
    UINT_32 isPolaris10       : 1;
    UINT_32 isPolaris11       : 1;
    UINT_32 isPolaris12       : 1;
    UINT_32 isVegaM           : 1;
    UINT_32 isCarrizo         : 1;
};

/**
****************************************************************************************************
* @brief This class is the SI specific address library
*        function set.
****************************************************************************************************
*/
class SiLib : public EgBasedLib
{
public:
    /// Creates SiLib object
    static Addr::Lib* CreateObj(const Client* pClient)
    {
        VOID* pMem = Object::ClientAlloc(sizeof(SiLib), pClient);
        return (pMem != NULL) ? new (pMem) SiLib(pClient) : NULL;
    }

protected:
    SiLib(const Client* pClient);
    virtual ~SiLib();

    // Hwl interface - defined in AddrLib1
    virtual ADDR_E_RETURNCODE HwlComputeSurfaceInfo(
        const ADDR_COMPUTE_SURFACE_INFO_INPUT* pIn,
        ADDR_COMPUTE_SURFACE_INFO_OUTPUT* pOut) const;

    virtual ADDR_E_RETURNCODE HwlConvertTileInfoToHW(
        const ADDR_CONVERT_TILEINFOTOHW_INPUT* pIn,
        ADDR_CONVERT_TILEINFOTOHW_OUTPUT* pOut) const;

    virtual UINT_64 HwlComputeXmaskAddrFromCoord(
        UINT_32 pitch, UINT_32 height, UINT_32 x, UINT_32 y, UINT_32 slice, UINT_32 numSlices,
        UINT_32 factor, BOOL_32 isLinear, BOOL_32 isWidth8, BOOL_32 isHeight8,
        ADDR_TILEINFO* pTileInfo, UINT_32* pBitPosition) const;

    virtual VOID HwlComputeXmaskCoordFromAddr(
        UINT_64 addr, UINT_32 bitPosition, UINT_32 pitch, UINT_32 height, UINT_32 numSlices,
        UINT_32 factor, BOOL_32 isLinear, BOOL_32 isWidth8, BOOL_32 isHeight8,
        ADDR_TILEINFO* pTileInfo, UINT_32* pX, UINT_32* pY, UINT_32* pSlice) const;

    virtual ADDR_E_RETURNCODE HwlGetTileIndex(
        const ADDR_GET_TILEINDEX_INPUT* pIn,
        ADDR_GET_TILEINDEX_OUTPUT*      pOut) const;

    virtual BOOL_32 HwlComputeMipLevel(
        ADDR_COMPUTE_SURFACE_INFO_INPUT* pIn) const;

    virtual ChipFamily HwlConvertChipFamily(
        UINT_32 uChipFamily, UINT_32 uChipRevision);

    virtual BOOL_32 HwlInitGlobalParams(
        const ADDR_CREATE_INPUT* pCreateIn);

    virtual ADDR_E_RETURNCODE HwlSetupTileCfg(
        UINT_32 bpp, INT_32 index, INT_32 macroModeIndex,
        ADDR_TILEINFO* pInfo, AddrTileMode* pMode = 0, AddrTileType* pType = 0) const;

    virtual VOID HwlComputeTileDataWidthAndHeightLinear(
        UINT_32* pMacroWidth, UINT_32* pMacroHeight,
        UINT_32 bpp, ADDR_TILEINFO* pTileInfo) const;

    virtual UINT_64 HwlComputeHtileBytes(
        UINT_32 pitch, UINT_32 height, UINT_32 bpp,
        BOOL_32 isLinear, UINT_32 numSlices, UINT_64* pSliceBytes, UINT_32 baseAlign) const;

    virtual ADDR_E_RETURNCODE ComputeBankEquation(
        UINT_32 log2BytesPP, UINT_32 threshX, UINT_32 threshY,
        ADDR_TILEINFO* pTileInfo, ADDR_EQUATION* pEquation) const;

    virtual ADDR_E_RETURNCODE ComputePipeEquation(
        UINT_32 log2BytesPP, UINT_32 threshX, UINT_32 threshY,
        ADDR_TILEINFO* pTileInfo, ADDR_EQUATION* pEquation) const;

    virtual UINT_32 ComputePipeFromCoord(
        UINT_32 x, UINT_32 y, UINT_32 slice,
        AddrTileMode tileMode, UINT_32 pipeSwizzle, BOOL_32 ignoreSE,
        ADDR_TILEINFO* pTileInfo) const;

    virtual UINT_32 HwlGetPipes(const ADDR_TILEINFO* pTileInfo) const;

    /// Pre-handler of 3x pitch (96 bit) adjustment
    virtual UINT_32 HwlPreHandleBaseLvl3xPitch(
        const ADDR_COMPUTE_SURFACE_INFO_INPUT* pIn, UINT_32 expPitch) const;
    /// Post-handler of 3x pitch adjustment
    virtual UINT_32 HwlPostHandleBaseLvl3xPitch(
        const ADDR_COMPUTE_SURFACE_INFO_INPUT* pIn, UINT_32 expPitch) const;

    /// Dummy function to finalize the inheritance
    virtual UINT_32 HwlComputeXmaskCoordYFrom8Pipe(
        UINT_32 pipe, UINT_32 x) const;

    // Sub-hwl interface - defined in EgBasedLib
    virtual VOID HwlSetupTileInfo(
        AddrTileMode tileMode, ADDR_SURFACE_FLAGS flags,
        UINT_32 bpp, UINT_32 pitch, UINT_32 height, UINT_32 numSamples,
        ADDR_TILEINFO* inputTileInfo, ADDR_TILEINFO* outputTileInfo,
        AddrTileType inTileType, ADDR_COMPUTE_SURFACE_INFO_OUTPUT* pOut) const;

    virtual UINT_32 HwlGetPitchAlignmentMicroTiled(
        AddrTileMode tileMode, UINT_32 bpp, ADDR_SURFACE_FLAGS flags, UINT_32 numSamples) const;

    virtual UINT_64 HwlGetSizeAdjustmentMicroTiled(
        UINT_32 thickness, UINT_32 bpp, ADDR_SURFACE_FLAGS flags, UINT_32 numSamples,
        UINT_32 baseAlign, UINT_32 pitchAlign,
        UINT_32 *pPitch, UINT_32 *pHeight) const;

    virtual VOID HwlCheckLastMacroTiledLvl(
        const ADDR_COMPUTE_SURFACE_INFO_INPUT* pIn, ADDR_COMPUTE_SURFACE_INFO_OUTPUT* pOut) const;

    virtual BOOL_32 HwlTileInfoEqual(
        const ADDR_TILEINFO* pLeft, const ADDR_TILEINFO* pRight) const;

    virtual AddrTileMode HwlDegradeThickTileMode(
        AddrTileMode baseTileMode, UINT_32 numSlices, UINT_32* pBytesPerTile) const;

    virtual VOID HwlOverrideTileMode(ADDR_COMPUTE_SURFACE_INFO_INPUT* pInOut) const;

    virtual VOID HwlOptimizeTileMode(ADDR_COMPUTE_SURFACE_INFO_INPUT* pInOut) const;

    virtual VOID HwlSelectTileMode(ADDR_COMPUTE_SURFACE_INFO_INPUT* pInOut) const;

    /// Overwrite tile setting to PRT
    virtual VOID HwlSetPrtTileMode(ADDR_COMPUTE_SURFACE_INFO_INPUT* pInOut) const;

    virtual BOOL_32 HwlSanityCheckMacroTiled(
        ADDR_TILEINFO* pTileInfo) const
    {
        return TRUE;
    }

    virtual UINT_32 HwlGetPitchAlignmentLinear(UINT_32 bpp, ADDR_SURFACE_FLAGS flags) const;

    virtual UINT_64 HwlGetSizeAdjustmentLinear(
        AddrTileMode tileMode,
        UINT_32 bpp, UINT_32 numSamples, UINT_32 baseAlign, UINT_32 pitchAlign,
        UINT_32 *pPitch, UINT_32 *pHeight, UINT_32 *pHeightAlign) const;

    virtual VOID HwlComputeSurfaceCoord2DFromBankPipe(
        AddrTileMode tileMode, UINT_32* pX, UINT_32* pY, UINT_32 slice,
        UINT_32 bank, UINT_32 pipe,
        UINT_32 bankSwizzle, UINT_32 pipeSwizzle, UINT_32 tileSlices,
        BOOL_32 ignoreSE,
        ADDR_TILEINFO* pTileInfo) const;

    virtual UINT_32 HwlPreAdjustBank(
        UINT_32 tileX, UINT_32 bank, ADDR_TILEINFO* pTileInfo) const;

    virtual INT_32 HwlPostCheckTileIndex(
        const ADDR_TILEINFO* pInfo, AddrTileMode mode, AddrTileType type,
        INT curIndex = TileIndexInvalid) const;

    virtual VOID HwlFmaskPreThunkSurfInfo(
        const ADDR_COMPUTE_FMASK_INFO_INPUT* pFmaskIn,
        const ADDR_COMPUTE_FMASK_INFO_OUTPUT* pFmaskOut,
        ADDR_COMPUTE_SURFACE_INFO_INPUT* pSurfIn,
        ADDR_COMPUTE_SURFACE_INFO_OUTPUT* pSurfOut) const;

    virtual VOID HwlFmaskPostThunkSurfInfo(
        const ADDR_COMPUTE_SURFACE_INFO_OUTPUT* pSurfOut,
        ADDR_COMPUTE_FMASK_INFO_OUTPUT* pFmaskOut) const;

    virtual UINT_32 HwlComputeFmaskBits(
        const ADDR_COMPUTE_FMASK_INFO_INPUT* pIn,
        UINT_32* pNumSamples) const;

    virtual BOOL_32 HwlReduceBankWidthHeight(
        UINT_32 tileSize, UINT_32 bpp, ADDR_SURFACE_FLAGS flags, UINT_32 numSamples,
        UINT_32 bankHeightAlign, UINT_32 pipes,
        ADDR_TILEINFO* pTileInfo) const
    {
        return TRUE;
    }

    virtual UINT_32 HwlComputeMaxBaseAlignments() const;

    virtual UINT_32 HwlComputeMaxMetaBaseAlignments() const;

    virtual VOID HwlComputeSurfaceAlignmentsMacroTiled(
        AddrTileMode tileMode, UINT_32 bpp, ADDR_SURFACE_FLAGS flags,
        UINT_32 mipLevel, UINT_32 numSamples, ADDR_COMPUTE_SURFACE_INFO_OUTPUT* pOut) const;

    // Get equation table pointer and number of equations
    virtual UINT_32 HwlGetEquationTableInfo(const ADDR_EQUATION** ppEquationTable) const
    {
        *ppEquationTable = m_equationTable;

        return m_numEquations;
    }

    // Check if it is supported for given bpp and tile config to generate an equation
    BOOL_32 IsEquationSupported(
        UINT_32 bpp, TileConfig tileConfig, INT_32 tileIndex, UINT_32 elementBytesLog2) const;

    // Protected non-virtual functions
    VOID ComputeTileCoordFromPipeAndElemIdx(
        UINT_32 elemIdx, UINT_32 pipe, AddrPipeCfg pipeCfg, UINT_32 pitchInMacroTile,
        UINT_32 x, UINT_32 y, UINT_32* pX, UINT_32* pY) const;

    UINT_32 TileCoordToMaskElementIndex(
        UINT_32 tx, UINT_32 ty, AddrPipeCfg  pipeConfig,
        UINT_32 *macroShift, UINT_32 *elemIdxBits) const;

    BOOL_32 DecodeGbRegs(
        const ADDR_REGISTER_VALUE* pRegValue);

    const TileConfig* GetTileSetting(
        UINT_32 index) const;

    // Initialize equation table
    VOID InitEquationTable();

    UINT_32 GetPipePerSurf(AddrPipeCfg pipeConfig) const;

    static const UINT_32    TileTableSize = 32;
    TileConfig              m_tileTable[TileTableSize];
    UINT_32                 m_noOfEntries;

    // Max number of bpp (8bpp/16bpp/32bpp/64bpp/128bpp)
    static const UINT_32    MaxNumElementBytes  = 5;

    static const BOOL_32    m_EquationSupport[TileTableSize][MaxNumElementBytes];

    // Prt tile mode index mask
    static const UINT_32    SiPrtTileIndexMask = ((1 << 3)  | (1 << 5)  | (1 << 6)  | (1 << 7)  |
                                                  (1 << 21) | (1 << 22) | (1 << 23) | (1 << 24) |
                                                  (1 << 25) | (1 << 30));

    // More than half slots in tile mode table can't support equation
    static const UINT_32    EquationTableSize   = (MaxNumElementBytes * TileTableSize) / 2;
    // Equation table
    ADDR_EQUATION           m_equationTable[EquationTableSize];
    UINT_32                 m_numMacroBits[EquationTableSize];
    UINT_32                 m_blockWidth[EquationTableSize];
    UINT_32                 m_blockHeight[EquationTableSize];
    UINT_32                 m_blockSlices[EquationTableSize];
    // Number of equation entries in the table
    UINT_32                 m_numEquations;
    // Equation lookup table according to bpp and tile index
    UINT_32                 m_equationLookupTable[MaxNumElementBytes][TileTableSize];

    UINT_32                 m_uncompressDepthEqIndex;

    SiChipSettings          m_settings;

private:

    VOID ReadGbTileMode(UINT_32 regValue, TileConfig* pCfg) const;
    BOOL_32 InitTileSettingTable(const UINT_32 *pSetting, UINT_32 noOfEntries);
};

} // V1
} // Addr

#endif

