/*
************************************************************************************************************************
*
*  Copyright (C) 2007-2022 Advanced Micro Devices, Inc.  All rights reserved.
*  SPDX-License-Identifier: MIT
*
***********************************************************************************************************************/

/**
****************************************************************************************************
* @file  ciaddrlib.h
* @brief Contains the CiLib class definition.
****************************************************************************************************
*/

#ifndef __CI_ADDR_LIB_H__
#define __CI_ADDR_LIB_H__

#include "addrlib1.h"
#include "siaddrlib.h"

namespace Addr
{
namespace V1
{

/**
****************************************************************************************************
* @brief This class is the CI specific address library
*        function set.
****************************************************************************************************
*/
class CiLib : public SiLib
{
public:
    /// Creates CiLib object
    static Addr::Lib* CreateObj(const Client* pClient)
    {
        VOID* pMem = Object::ClientAlloc(sizeof(CiLib), pClient);
        return (pMem != NULL) ? new (pMem) CiLib(pClient) : NULL;
    }

private:
    CiLib(const Client* pClient);
    virtual ~CiLib();

protected:

    // Hwl interface - defined in AddrLib1
    virtual ADDR_E_RETURNCODE HwlComputeSurfaceInfo(
        const ADDR_COMPUTE_SURFACE_INFO_INPUT* pIn,
        ADDR_COMPUTE_SURFACE_INFO_OUTPUT* pOut) const;

    virtual ADDR_E_RETURNCODE HwlComputeFmaskInfo(
        const ADDR_COMPUTE_FMASK_INFO_INPUT* pIn,
        ADDR_COMPUTE_FMASK_INFO_OUTPUT* pOut);

    virtual ChipFamily HwlConvertChipFamily(
        UINT_32 uChipFamily, UINT_32 uChipRevision);

    virtual BOOL_32 HwlInitGlobalParams(
        const ADDR_CREATE_INPUT* pCreateIn);

    virtual ADDR_E_RETURNCODE HwlSetupTileCfg(
        UINT_32 bpp, INT_32 index, INT_32 macroModeIndex, ADDR_TILEINFO* pInfo,
        AddrTileMode* pMode = 0, AddrTileType* pType = 0) const;

    virtual VOID HwlComputeTileDataWidthAndHeightLinear(
        UINT_32* pMacroWidth, UINT_32* pMacroHeight,
        UINT_32 bpp, ADDR_TILEINFO* pTileInfo) const;

    virtual INT_32 HwlComputeMacroModeIndex(
        INT_32 tileIndex, ADDR_SURFACE_FLAGS flags, UINT_32 bpp, UINT_32 numSamples,
        ADDR_TILEINFO* pTileInfo, AddrTileMode* pTileMode = NULL, AddrTileType* pTileType = NULL
        ) const;

    // Sub-hwl interface - defined in EgBasedLib
    virtual VOID HwlSetupTileInfo(
        AddrTileMode tileMode, ADDR_SURFACE_FLAGS flags,
        UINT_32 bpp, UINT_32 pitch, UINT_32 height, UINT_32 numSamples,
        ADDR_TILEINFO* inputTileInfo, ADDR_TILEINFO* outputTileInfo,
        AddrTileType inTileType, ADDR_COMPUTE_SURFACE_INFO_OUTPUT* pOut) const;

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

    virtual AddrTileMode HwlDegradeThickTileMode(
        AddrTileMode baseTileMode, UINT_32 numSlices, UINT_32* pBytesPerTile) const;

    virtual VOID HwlOverrideTileMode(ADDR_COMPUTE_SURFACE_INFO_INPUT* pInOut) const;

    virtual VOID HwlOptimizeTileMode(ADDR_COMPUTE_SURFACE_INFO_INPUT* pInOut) const;

    virtual VOID HwlSelectTileMode(ADDR_COMPUTE_SURFACE_INFO_INPUT* pInOut) const;

    /// Overwrite tile setting to PRT
    virtual VOID HwlSetPrtTileMode(ADDR_COMPUTE_SURFACE_INFO_INPUT* pInOut) const;

    virtual ADDR_E_RETURNCODE HwlComputeDccInfo(
        const ADDR_COMPUTE_DCCINFO_INPUT* pIn,
        ADDR_COMPUTE_DCCINFO_OUTPUT* pOut) const;

    virtual ADDR_E_RETURNCODE HwlComputeCmaskAddrFromCoord(
        const ADDR_COMPUTE_CMASK_ADDRFROMCOORD_INPUT* pIn,
        ADDR_COMPUTE_CMASK_ADDRFROMCOORD_OUTPUT* pOut) const;

    virtual ADDR_E_RETURNCODE HwlComputeHtileAddrFromCoord(
        const ADDR_COMPUTE_HTILE_ADDRFROMCOORD_INPUT*  pIn,
        ADDR_COMPUTE_HTILE_ADDRFROMCOORD_OUTPUT*       pOut) const;

    virtual UINT_32 HwlComputeMaxBaseAlignments() const;

    virtual UINT_32 HwlComputeMaxMetaBaseAlignments() const;

    virtual VOID HwlPadDimensions(
        AddrTileMode tileMode, UINT_32 bpp, ADDR_SURFACE_FLAGS flags,
        UINT_32 numSamples, ADDR_TILEINFO* pTileInfo, UINT_32 mipLevel,
        UINT_32* pPitch, UINT_32 *PitchAlign, UINT_32 height, UINT_32 heightAlign) const;

    virtual VOID HwlComputeSurfaceAlignmentsMacroTiled(
        AddrTileMode tileMode, UINT_32 bpp, ADDR_SURFACE_FLAGS flags,
        UINT_32 mipLevel, UINT_32 numSamples, ADDR_COMPUTE_SURFACE_INFO_OUTPUT* pOut) const;

private:
    VOID ReadGbTileMode(
        UINT_32 regValue, TileConfig* pCfg) const;

    VOID ReadGbMacroTileCfg(
        UINT_32 regValue, ADDR_TILEINFO* pCfg) const;

private:
    BOOL_32 InitTileSettingTable(
        const UINT_32 *pSetting, UINT_32 noOfEntries);

    BOOL_32 InitMacroTileCfgTable(
        const UINT_32 *pSetting, UINT_32 noOfEntries);

    UINT_64 HwlComputeMetadataNibbleAddress(
        UINT_64 uncompressedDataByteAddress,
        UINT_64 dataBaseByteAddress,
        UINT_64 metadataBaseByteAddress,
        UINT_32 metadataBitSize,
        UINT_32 elementBitSize,
        UINT_32 blockByteSize,
        UINT_32 pipeInterleaveBytes,
        UINT_32 numOfPipes,
        UINT_32 numOfBanks,
        UINT_32 numOfSamplesPerSplit) const;

    BOOL_32 DepthStencilTileCfgMatch(
        const ADDR_COMPUTE_SURFACE_INFO_INPUT*  pIn,
        ADDR_COMPUTE_SURFACE_INFO_OUTPUT*       pOut) const;

    BOOL_32 CheckTcCompatibility(const ADDR_TILEINFO* pTileInfo, UINT_32 bpp, AddrTileMode tileMode,
                                 AddrTileType tileType, const ADDR_COMPUTE_SURFACE_INFO_OUTPUT* pOut) const;

    BOOL_32 SupportDccAndTcCompatibility() const
    {
        return ((m_settings.isVolcanicIslands == TRUE) || (m_configFlags.forceDccAndTcCompat == TRUE));
    }

    BOOL_32 AltTilingEnabled() const
    {
        return (m_configFlags.enableAltTiling == TRUE);
    }

    static const UINT_32    MacroTileTableSize = 16;
    static const UINT_32    PrtMacroModeOffset = MacroTileTableSize / 2;
    static const INT_32     MinDepth2DThinIndex = 0;
    static const INT_32     MaxDepth2DThinIndex = 4;
    static const INT_32     Depth1DThinIndex = 5;

    ADDR_TILEINFO           m_macroTileTable[MacroTileTableSize];
    UINT_32                 m_noOfMacroEntries;
    BOOL_32                 m_allowNonDispThickModes;
};

} // V1
} // Addr

#endif


