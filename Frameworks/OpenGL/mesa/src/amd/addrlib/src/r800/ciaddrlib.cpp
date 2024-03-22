/*
************************************************************************************************************************
*
*  Copyright (C) 2007-2022 Advanced Micro Devices, Inc.  All rights reserved.
*  SPDX-License-Identifier: MIT
*
***********************************************************************************************************************/

/**
****************************************************************************************************
* @file  ciaddrlib.cpp
* @brief Contains the implementation for the CiLib class.
****************************************************************************************************
*/

#include "ciaddrlib.h"

#include "si_gb_reg.h"

#include "amdgpu_asic_addr.h"

////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////

namespace Addr
{

/**
****************************************************************************************************
*   CiHwlInit
*
*   @brief
*       Creates an CiLib object.
*
*   @return
*       Returns an CiLib object pointer.
****************************************************************************************************
*/
Lib* CiHwlInit(const Client* pClient)
{
    return V1::CiLib::CreateObj(pClient);
}

namespace V1
{

/**
****************************************************************************************************
*   Mask
*
*   @brief
*       Gets a mask of "width"
*   @return
*       Bit mask
****************************************************************************************************
*/
static UINT_64 Mask(
    UINT_32 width)  ///< Width of bits
{
    UINT_64 ret;

    if (width >= sizeof(UINT_64)*8)
    {
        ret = ~((UINT_64) 0);
    }
    else
    {
        return (((UINT_64) 1) << width) - 1;
    }
    return ret;
}

/**
****************************************************************************************************
*   GetBits
*
*   @brief
*       Gets bits within a range of [msb, lsb]
*   @return
*       Bits of this range
****************************************************************************************************
*/
static UINT_64 GetBits(
    UINT_64 bits,   ///< Source bits
    UINT_32 msb,    ///< Most signicant bit
    UINT_32 lsb)    ///< Least signicant bit
{
    UINT_64 ret = 0;

    if (msb >= lsb)
    {
        ret = (bits >> lsb) & (Mask(1 + msb - lsb));
    }
    return ret;
}

/**
****************************************************************************************************
*   RemoveBits
*
*   @brief
*       Removes bits within the range of [msb, lsb]
*   @return
*       Modified bits
****************************************************************************************************
*/
static UINT_64 RemoveBits(
    UINT_64 bits,   ///< Source bits
    UINT_32 msb,    ///< Most signicant bit
    UINT_32 lsb)    ///< Least signicant bit
{
    UINT_64 ret = bits;

    if (msb >= lsb)
    {
        ret = GetBits(bits, lsb - 1, 0) // low bits
            | (GetBits(bits, 8 * sizeof(bits) - 1, msb + 1) << lsb); //high bits
    }
    return ret;
}

/**
****************************************************************************************************
*   InsertBits
*
*   @brief
*       Inserts new bits into the range of [msb, lsb]
*   @return
*       Modified bits
****************************************************************************************************
*/
static UINT_64 InsertBits(
    UINT_64 bits,       ///< Source bits
    UINT_64 newBits,    ///< New bits to be inserted
    UINT_32 msb,        ///< Most signicant bit
    UINT_32 lsb)        ///< Least signicant bit
{
    UINT_64 ret = bits;

    if (msb >= lsb)
    {
        ret = GetBits(bits, lsb - 1, 0) // old low bitss
             | (GetBits(newBits, msb - lsb, 0) << lsb) //new bits
             | (GetBits(bits, 8 * sizeof(bits) - 1, lsb) << (msb + 1)); //old high bits
    }
    return ret;
}

/**
****************************************************************************************************
*   CiLib::CiLib
*
*   @brief
*       Constructor
*
****************************************************************************************************
*/
CiLib::CiLib(const Client* pClient)
    :
    SiLib(pClient),
    m_noOfMacroEntries(0),
    m_allowNonDispThickModes(FALSE)
{
}

/**
****************************************************************************************************
*   CiLib::~CiLib
*
*   @brief
*       Destructor
****************************************************************************************************
*/
CiLib::~CiLib()
{
}

/**
****************************************************************************************************
*   CiLib::HwlComputeDccInfo
*
*   @brief
*       Compute DCC key size, base alignment
*   @return
*       ADDR_E_RETURNCODE
****************************************************************************************************
*/
ADDR_E_RETURNCODE CiLib::HwlComputeDccInfo(
    const ADDR_COMPUTE_DCCINFO_INPUT*  pIn,
    ADDR_COMPUTE_DCCINFO_OUTPUT*       pOut) const
{
    ADDR_E_RETURNCODE returnCode = ADDR_OK;

    if (SupportDccAndTcCompatibility() && IsMacroTiled(pIn->tileMode))
    {
        UINT_64 dccFastClearSize = pIn->colorSurfSize >> 8;

        ADDR_ASSERT(0 == (pIn->colorSurfSize & 0xff));

        if (pIn->numSamples > 1)
        {
            UINT_32 tileSizePerSample = BITS_TO_BYTES(pIn->bpp * MicroTileWidth * MicroTileHeight);
            UINT_32 samplesPerSplit  = pIn->tileInfo.tileSplitBytes / tileSizePerSample;

            if (samplesPerSplit < pIn->numSamples)
            {
                UINT_32 numSplits = pIn->numSamples / samplesPerSplit;
                UINT_32 fastClearBaseAlign = HwlGetPipes(&pIn->tileInfo) * m_pipeInterleaveBytes;

                ADDR_ASSERT(IsPow2(fastClearBaseAlign));

                dccFastClearSize /= numSplits;

                if (0 != (dccFastClearSize & (fastClearBaseAlign - 1)))
                {
                    // Disable dcc fast clear
                    // if key size of fisrt sample split is not pipe*interleave aligned
                    dccFastClearSize = 0;
                }
            }
        }

        pOut->dccRamSize          = pIn->colorSurfSize >> 8;
        pOut->dccRamBaseAlign     = pIn->tileInfo.banks *
                                    HwlGetPipes(&pIn->tileInfo) *
                                    m_pipeInterleaveBytes;
        pOut->dccFastClearSize    = dccFastClearSize;
        pOut->dccRamSizeAligned   = TRUE;

        ADDR_ASSERT(IsPow2(pOut->dccRamBaseAlign));

        if (0 == (pOut->dccRamSize & (pOut->dccRamBaseAlign - 1)))
        {
            pOut->subLvlCompressible = TRUE;
        }
        else
        {
            UINT_64 dccRamSizeAlign = HwlGetPipes(&pIn->tileInfo) * m_pipeInterleaveBytes;

            if (pOut->dccRamSize == pOut->dccFastClearSize)
            {
                pOut->dccFastClearSize = PowTwoAlign(pOut->dccRamSize, dccRamSizeAlign);
            }
            if ((pOut->dccRamSize & (dccRamSizeAlign - 1)) != 0)
            {
                pOut->dccRamSizeAligned = FALSE;
            }
            pOut->dccRamSize          = PowTwoAlign(pOut->dccRamSize, dccRamSizeAlign);
            pOut->subLvlCompressible  = FALSE;
        }
    }
    else
    {
        returnCode = ADDR_NOTSUPPORTED;
    }

    return returnCode;
}

/**
****************************************************************************************************
*   CiLib::HwlComputeCmaskAddrFromCoord
*
*   @brief
*       Compute tc compatible Cmask address from fmask ram address
*
*   @return
*       ADDR_E_RETURNCODE
****************************************************************************************************
*/
ADDR_E_RETURNCODE CiLib::HwlComputeCmaskAddrFromCoord(
    const ADDR_COMPUTE_CMASK_ADDRFROMCOORD_INPUT*  pIn,  ///< [in] fmask addr/bpp/tile input
    ADDR_COMPUTE_CMASK_ADDRFROMCOORD_OUTPUT*       pOut  ///< [out] cmask address
    ) const
{
    ADDR_E_RETURNCODE returnCode = ADDR_NOTSUPPORTED;

    if ((SupportDccAndTcCompatibility() == TRUE) &&
        (pIn->flags.tcCompatible == TRUE))
    {
        UINT_32 numOfPipes   = HwlGetPipes(pIn->pTileInfo);
        UINT_32 numOfBanks   = pIn->pTileInfo->banks;
        UINT_64 fmaskAddress = pIn->fmaskAddr;
        UINT_32 elemBits     = pIn->bpp;
        UINT_32 blockByte    = 64 * elemBits / 8;
        UINT_64 metaNibbleAddress = HwlComputeMetadataNibbleAddress(fmaskAddress,
                                                                    0,
                                                                    0,
                                                                    4,   // cmask 4 bits
                                                                    elemBits,
                                                                    blockByte,
                                                                    m_pipeInterleaveBytes,
                                                                    numOfPipes,
                                                                    numOfBanks,
                                                                    1);
        pOut->addr = (metaNibbleAddress >> 1);
        pOut->bitPosition = (metaNibbleAddress % 2) ? 4 : 0;
        returnCode = ADDR_OK;
    }

    return returnCode;
}

/**
****************************************************************************************************
*   CiLib::HwlComputeHtileAddrFromCoord
*
*   @brief
*       Compute tc compatible Htile address from depth/stencil address
*
*   @return
*       ADDR_E_RETURNCODE
****************************************************************************************************
*/
ADDR_E_RETURNCODE CiLib::HwlComputeHtileAddrFromCoord(
    const ADDR_COMPUTE_HTILE_ADDRFROMCOORD_INPUT*  pIn,  ///< [in] depth/stencil addr/bpp/tile input
    ADDR_COMPUTE_HTILE_ADDRFROMCOORD_OUTPUT*       pOut  ///< [out] htile address
    ) const
{
    ADDR_E_RETURNCODE returnCode = ADDR_NOTSUPPORTED;

    if ((SupportDccAndTcCompatibility() == TRUE) &&
        (pIn->flags.tcCompatible == TRUE))
    {
        UINT_32 numOfPipes   = HwlGetPipes(pIn->pTileInfo);
        UINT_32 numOfBanks   = pIn->pTileInfo->banks;
        UINT_64 zStencilAddr = pIn->zStencilAddr;
        UINT_32 elemBits     = pIn->bpp;
        UINT_32 blockByte    = 64 * elemBits / 8;
        UINT_64 metaNibbleAddress = HwlComputeMetadataNibbleAddress(zStencilAddr,
                                                                    0,
                                                                    0,
                                                                    32,  // htile 32 bits
                                                                    elemBits,
                                                                    blockByte,
                                                                    m_pipeInterleaveBytes,
                                                                    numOfPipes,
                                                                    numOfBanks,
                                                                    1);
        pOut->addr = (metaNibbleAddress >> 1);
        pOut->bitPosition = 0;
        returnCode = ADDR_OK;
    }

    return returnCode;
}

/**
****************************************************************************************************
*   CiLib::HwlConvertChipFamily
*
*   @brief
*       Convert familyID defined in atiid.h to ChipFamily and set m_chipFamily/m_chipRevision
*   @return
*       ChipFamily
****************************************************************************************************
*/
ChipFamily CiLib::HwlConvertChipFamily(
    UINT_32 uChipFamily,        ///< [in] chip family defined in atiih.h
    UINT_32 uChipRevision)      ///< [in] chip revision defined in "asic_family"_id.h
{
    ChipFamily family = ADDR_CHIP_FAMILY_CI;

    switch (uChipFamily)
    {
        case FAMILY_CI:
            m_settings.isSeaIsland  = 1;
            m_settings.isBonaire    = ASICREV_IS_BONAIRE_M(uChipRevision);
            m_settings.isHawaii     = ASICREV_IS_HAWAII_P(uChipRevision);
            break;
        case FAMILY_KV:
            m_settings.isKaveri     = 1;
            m_settings.isSpectre    = ASICREV_IS_SPECTRE(uChipRevision);
            m_settings.isSpooky     = ASICREV_IS_SPOOKY(uChipRevision);
            m_settings.isKalindi    = ASICREV_IS_KALINDI(uChipRevision);
            break;
        case FAMILY_VI:
            m_settings.isVolcanicIslands = 1;
            m_settings.isIceland         = ASICREV_IS_ICELAND_M(uChipRevision);
            m_settings.isTonga           = ASICREV_IS_TONGA_P(uChipRevision);
            m_settings.isFiji            = ASICREV_IS_FIJI_P(uChipRevision);
            m_settings.isPolaris10       = ASICREV_IS_POLARIS10_P(uChipRevision);
            m_settings.isPolaris11       = ASICREV_IS_POLARIS11_M(uChipRevision);
            m_settings.isPolaris12       = ASICREV_IS_POLARIS12_V(uChipRevision);
            m_settings.isVegaM           = ASICREV_IS_VEGAM_P(uChipRevision);
            family = ADDR_CHIP_FAMILY_VI;
            break;
        case FAMILY_CZ:
            m_settings.isCarrizo         = 1;
            m_settings.isVolcanicIslands = 1;
            family = ADDR_CHIP_FAMILY_VI;
            break;
        default:
            ADDR_ASSERT(!"No Chip found");
            break;
    }

    return family;
}

/**
****************************************************************************************************
*   CiLib::HwlInitGlobalParams
*
*   @brief
*       Initializes global parameters
*
*   @return
*       TRUE if all settings are valid
*
****************************************************************************************************
*/
BOOL_32 CiLib::HwlInitGlobalParams(
    const ADDR_CREATE_INPUT* pCreateIn) ///< [in] create input
{
    BOOL_32  valid = TRUE;

    const ADDR_REGISTER_VALUE* pRegValue = &pCreateIn->regValue;

    valid = DecodeGbRegs(pRegValue);

    // The following assignments for m_pipes is only for fail-safe, InitTileSettingTable should
    // read the correct pipes from tile mode table
    if (m_settings.isHawaii)
    {
        m_pipes = 16;
    }
    else if (m_settings.isBonaire || m_settings.isSpectre)
    {
        m_pipes = 4;
    }
    else // Treat other KV asics to be 2-pipe
    {
        m_pipes = 2;
    }

    // @todo: VI
    // Move this to VI code path once created
    if (m_settings.isTonga || m_settings.isPolaris10)
    {
        m_pipes = 8;
    }
    else if (m_settings.isIceland)
    {
        m_pipes = 2;
    }
    else if (m_settings.isFiji)
    {
        m_pipes = 16;
    }
    else if (m_settings.isPolaris11 || m_settings.isPolaris12)
    {
        m_pipes = 4;
    }
    else if (m_settings.isVegaM)
    {
        m_pipes = 16;
    }

    if (valid)
    {
        valid = InitTileSettingTable(pRegValue->pTileConfig, pRegValue->noOfEntries);
    }
    if (valid)
    {
        valid = InitMacroTileCfgTable(pRegValue->pMacroTileConfig, pRegValue->noOfMacroEntries);
    }

    if (valid)
    {
        InitEquationTable();
    }

    return valid;
}

/**
****************************************************************************************************
*   CiLib::HwlPostCheckTileIndex
*
*   @brief
*       Map a tile setting to index if curIndex is invalid, otherwise check if curIndex matches
*       tile mode/type/info and change the index if needed
*   @return
*       Tile index.
****************************************************************************************************
*/
INT_32 CiLib::HwlPostCheckTileIndex(
    const ADDR_TILEINFO* pInfo,     ///< [in] Tile Info
    AddrTileMode         mode,      ///< [in] Tile mode
    AddrTileType         type,      ///< [in] Tile type
    INT                  curIndex   ///< [in] Current index assigned in HwlSetupTileInfo
    ) const
{
    INT_32 index = curIndex;

    if (mode == ADDR_TM_LINEAR_GENERAL)
    {
        index = TileIndexLinearGeneral;
    }
    else
    {
        BOOL_32 macroTiled = IsMacroTiled(mode);

        // We need to find a new index if either of them is true
        // 1. curIndex is invalid
        // 2. tile mode is changed
        // 3. tile info does not match for macro tiled
        if ((index == TileIndexInvalid)         ||
            (mode != m_tileTable[index].mode)   ||
            (macroTiled && pInfo->pipeConfig != m_tileTable[index].info.pipeConfig))
        {
            for (index = 0; index < static_cast<INT_32>(m_noOfEntries); index++)
            {
                if (macroTiled)
                {
                    // macro tile modes need all to match
                    if ((pInfo->pipeConfig == m_tileTable[index].info.pipeConfig) &&
                        (mode == m_tileTable[index].mode) &&
                        (type == m_tileTable[index].type))
                    {
                        // tileSplitBytes stored in m_tileTable is only valid for depth entries
                        if (type == ADDR_DEPTH_SAMPLE_ORDER)
                        {
                            if (Min(m_tileTable[index].info.tileSplitBytes,
                                    m_rowSize) == pInfo->tileSplitBytes)
                            {
                                break;
                            }
                        }
                        else // other entries are determined by other 3 fields
                        {
                            break;
                        }
                    }
                }
                else if (mode == ADDR_TM_LINEAR_ALIGNED)
                {
                    // linear mode only needs tile mode to match
                    if (mode == m_tileTable[index].mode)
                    {
                        break;
                    }
                }
                else
                {
                    // micro tile modes only need tile mode and tile type to match
                    if (mode == m_tileTable[index].mode &&
                        type == m_tileTable[index].type)
                    {
                        break;
                    }
                }
            }
        }
    }

    ADDR_ASSERT(index < static_cast<INT_32>(m_noOfEntries));

    if (index >= static_cast<INT_32>(m_noOfEntries))
    {
        index = TileIndexInvalid;
    }

    return index;
}

/**
****************************************************************************************************
*   CiLib::HwlSetupTileCfg
*
*   @brief
*       Map tile index to tile setting.
*   @return
*       ADDR_E_RETURNCODE
****************************************************************************************************
*/
ADDR_E_RETURNCODE CiLib::HwlSetupTileCfg(
    UINT_32         bpp,            ///< Bits per pixel
    INT_32          index,          ///< Tile index
    INT_32          macroModeIndex, ///< Index in macro tile mode table(CI)
    ADDR_TILEINFO*  pInfo,          ///< [out] Tile Info
    AddrTileMode*   pMode,          ///< [out] Tile mode
    AddrTileType*   pType           ///< [out] Tile type
    ) const
{
    ADDR_E_RETURNCODE returnCode = ADDR_OK;

    // Global flag to control usage of tileIndex
    if (UseTileIndex(index))
    {
        if (index == TileIndexLinearGeneral)
        {
            pInfo->banks = 2;
            pInfo->bankWidth = 1;
            pInfo->bankHeight = 1;
            pInfo->macroAspectRatio = 1;
            pInfo->tileSplitBytes = 64;
            pInfo->pipeConfig = ADDR_PIPECFG_P2;
        }
        else if (static_cast<UINT_32>(index) >= m_noOfEntries)
        {
            returnCode = ADDR_INVALIDPARAMS;
        }
        else
        {
            const TileConfig* pCfgTable = GetTileSetting(index);

            if (pInfo != NULL)
            {
                if (IsMacroTiled(pCfgTable->mode))
                {
                    ADDR_ASSERT((macroModeIndex != TileIndexInvalid) &&
                                (macroModeIndex != TileIndexNoMacroIndex));

                    UINT_32 tileSplit;

                    *pInfo = m_macroTileTable[macroModeIndex];

                    if (pCfgTable->type == ADDR_DEPTH_SAMPLE_ORDER)
                    {
                        tileSplit = pCfgTable->info.tileSplitBytes;
                    }
                    else
                    {
                        if (bpp > 0)
                        {
                            UINT_32 thickness = Thickness(pCfgTable->mode);
                            UINT_32 tileBytes1x = BITS_TO_BYTES(bpp * MicroTilePixels * thickness);
                            // Non-depth entries store a split factor
                            UINT_32 sampleSplit = m_tileTable[index].info.tileSplitBytes;
                            tileSplit = Max(256u, sampleSplit * tileBytes1x);
                        }
                        else
                        {
                            // Return tileBytes instead if not enough info
                            tileSplit = pInfo->tileSplitBytes;
                        }
                    }

                    // Clamp to row_size
                    pInfo->tileSplitBytes = Min(m_rowSize, tileSplit);

                    pInfo->pipeConfig = pCfgTable->info.pipeConfig;
                }
                else // 1D and linear modes, we return default value stored in table
                {
                    *pInfo = pCfgTable->info;
                }
            }

            if (pMode != NULL)
            {
                *pMode = pCfgTable->mode;
            }

            if (pType != NULL)
            {
                *pType = pCfgTable->type;
            }
        }
    }

    return returnCode;
}

/**
****************************************************************************************************
*   CiLib::HwlComputeSurfaceInfo
*
*   @brief
*       Entry of CI's ComputeSurfaceInfo
*   @return
*       ADDR_E_RETURNCODE
****************************************************************************************************
*/
ADDR_E_RETURNCODE CiLib::HwlComputeSurfaceInfo(
    const ADDR_COMPUTE_SURFACE_INFO_INPUT*  pIn,    ///< [in] input structure
    ADDR_COMPUTE_SURFACE_INFO_OUTPUT*       pOut    ///< [out] output structure
    ) const
{
    // If tileIndex is invalid, force macroModeIndex to be invalid, too
    if (pIn->tileIndex == TileIndexInvalid)
    {
        pOut->macroModeIndex = TileIndexInvalid;
    }

    ADDR_E_RETURNCODE retCode = SiLib::HwlComputeSurfaceInfo(pIn, pOut);

    if ((pIn->mipLevel > 0) &&
        (pOut->tcCompatible == TRUE) &&
        (pOut->tileMode != pIn->tileMode) &&
        (SupportDccAndTcCompatibility() == TRUE))
    {
        pOut->tcCompatible = CheckTcCompatibility(pOut->pTileInfo, pIn->bpp, pOut->tileMode, pOut->tileType, pOut);
    }

    if (pOut->macroModeIndex == TileIndexNoMacroIndex)
    {
        pOut->macroModeIndex = TileIndexInvalid;
    }

    if ((pIn->flags.matchStencilTileCfg == TRUE) &&
        (pIn->flags.depth == TRUE))
    {
        pOut->stencilTileIdx = TileIndexInvalid;

        if ((MinDepth2DThinIndex <= pOut->tileIndex) &&
            (MaxDepth2DThinIndex >= pOut->tileIndex))
        {
            BOOL_32 depthStencil2DTileConfigMatch = DepthStencilTileCfgMatch(pIn, pOut);

            if ((depthStencil2DTileConfigMatch == FALSE) &&
                (pOut->tcCompatible == TRUE))
            {
                pOut->macroModeIndex = TileIndexInvalid;

                ADDR_COMPUTE_SURFACE_INFO_INPUT localIn = *pIn;
                localIn.tileIndex = TileIndexInvalid;
                localIn.pTileInfo = NULL;
                localIn.flags.tcCompatible = FALSE;

                SiLib::HwlComputeSurfaceInfo(&localIn, pOut);

                ADDR_ASSERT((MinDepth2DThinIndex <= pOut->tileIndex) && (MaxDepth2DThinIndex >= pOut->tileIndex));

                depthStencil2DTileConfigMatch = DepthStencilTileCfgMatch(pIn, pOut);
            }

            if ((depthStencil2DTileConfigMatch == FALSE) &&
                (pIn->numSamples <= 1))
            {
                pOut->macroModeIndex = TileIndexInvalid;

                ADDR_COMPUTE_SURFACE_INFO_INPUT localIn = *pIn;
                localIn.tileMode = ADDR_TM_1D_TILED_THIN1;
                localIn.tileIndex = TileIndexInvalid;
                localIn.pTileInfo = NULL;

                retCode = SiLib::HwlComputeSurfaceInfo(&localIn, pOut);
            }
        }

        if (pOut->tileIndex == Depth1DThinIndex)
        {
            pOut->stencilTileIdx = Depth1DThinIndex;
        }
    }

    return retCode;
}

/**
****************************************************************************************************
*   CiLib::HwlFmaskSurfaceInfo
*   @brief
*       Entry of r800's ComputeFmaskInfo
*   @return
*       ADDR_E_RETURNCODE
****************************************************************************************************
*/
ADDR_E_RETURNCODE CiLib::HwlComputeFmaskInfo(
    const ADDR_COMPUTE_FMASK_INFO_INPUT*    pIn,   ///< [in] input structure
    ADDR_COMPUTE_FMASK_INFO_OUTPUT*         pOut   ///< [out] output structure
    )
{
    ADDR_E_RETURNCODE retCode = ADDR_OK;

    ADDR_TILEINFO tileInfo = {0};
    ADDR_COMPUTE_FMASK_INFO_INPUT fmaskIn;
    fmaskIn = *pIn;

    AddrTileMode tileMode = pIn->tileMode;

    // Use internal tile info if pOut does not have a valid pTileInfo
    if (pOut->pTileInfo == NULL)
    {
        pOut->pTileInfo = &tileInfo;
    }

    ADDR_ASSERT(tileMode == ADDR_TM_2D_TILED_THIN1     ||
                tileMode == ADDR_TM_3D_TILED_THIN1     ||
                tileMode == ADDR_TM_PRT_TILED_THIN1    ||
                tileMode == ADDR_TM_PRT_2D_TILED_THIN1 ||
                tileMode == ADDR_TM_PRT_3D_TILED_THIN1);

    ADDR_ASSERT(m_tileTable[14].mode == ADDR_TM_2D_TILED_THIN1);
    ADDR_ASSERT(m_tileTable[15].mode == ADDR_TM_3D_TILED_THIN1);

    // The only valid tile modes for fmask are 2D_THIN1 and 3D_THIN1 plus non-displayable
    INT_32 tileIndex = tileMode == ADDR_TM_2D_TILED_THIN1 ? 14 : 15;
    ADDR_SURFACE_FLAGS flags = {{0}};
    flags.fmask = 1;

    INT_32 macroModeIndex = TileIndexInvalid;

    UINT_32 numSamples = pIn->numSamples;
    UINT_32 numFrags = pIn->numFrags == 0 ? numSamples : pIn->numFrags;

    UINT_32 bpp = QLog2(numFrags);

    // EQAA needs one more bit
    if (numSamples > numFrags)
    {
        bpp++;
    }

    if (bpp == 3)
    {
        bpp = 4;
    }

    bpp = Max(8u, bpp * numSamples);

    macroModeIndex = HwlComputeMacroModeIndex(tileIndex, flags, bpp, numSamples, pOut->pTileInfo);

    fmaskIn.tileIndex = tileIndex;
    fmaskIn.pTileInfo = pOut->pTileInfo;
    pOut->macroModeIndex = macroModeIndex;
    pOut->tileIndex = tileIndex;

    retCode = DispatchComputeFmaskInfo(&fmaskIn, pOut);

    if (retCode == ADDR_OK)
    {
        pOut->tileIndex =
            HwlPostCheckTileIndex(pOut->pTileInfo, pIn->tileMode, ADDR_NON_DISPLAYABLE,
                                  pOut->tileIndex);
    }

    // Resets pTileInfo to NULL if the internal tile info is used
    if (pOut->pTileInfo == &tileInfo)
    {
        pOut->pTileInfo = NULL;
    }

    return retCode;
}

/**
****************************************************************************************************
*   CiLib::HwlFmaskPreThunkSurfInfo
*
*   @brief
*       Some preparation before thunking a ComputeSurfaceInfo call for Fmask
*   @return
*       ADDR_E_RETURNCODE
****************************************************************************************************
*/
VOID CiLib::HwlFmaskPreThunkSurfInfo(
    const ADDR_COMPUTE_FMASK_INFO_INPUT*    pFmaskIn,   ///< [in] Input of fmask info
    const ADDR_COMPUTE_FMASK_INFO_OUTPUT*   pFmaskOut,  ///< [in] Output of fmask info
    ADDR_COMPUTE_SURFACE_INFO_INPUT*        pSurfIn,    ///< [out] Input of thunked surface info
    ADDR_COMPUTE_SURFACE_INFO_OUTPUT*       pSurfOut    ///< [out] Output of thunked surface info
    ) const
{
    pSurfIn->tileIndex = pFmaskIn->tileIndex;
    pSurfOut->macroModeIndex  = pFmaskOut->macroModeIndex;
}

/**
****************************************************************************************************
*   CiLib::HwlFmaskPostThunkSurfInfo
*
*   @brief
*       Copy hwl extra field after calling thunked ComputeSurfaceInfo
*   @return
*       ADDR_E_RETURNCODE
****************************************************************************************************
*/
VOID CiLib::HwlFmaskPostThunkSurfInfo(
    const ADDR_COMPUTE_SURFACE_INFO_OUTPUT* pSurfOut,   ///< [in] Output of surface info
    ADDR_COMPUTE_FMASK_INFO_OUTPUT* pFmaskOut           ///< [out] Output of fmask info
    ) const
{
    pFmaskOut->tileIndex = pSurfOut->tileIndex;
    pFmaskOut->macroModeIndex = pSurfOut->macroModeIndex;
}

/**
****************************************************************************************************
*   CiLib::HwlDegradeThickTileMode
*
*   @brief
*       Degrades valid tile mode for thick modes if needed
*
*   @return
*       Suitable tile mode
****************************************************************************************************
*/
AddrTileMode CiLib::HwlDegradeThickTileMode(
    AddrTileMode        baseTileMode,   ///< [in] base tile mode
    UINT_32             numSlices,      ///< [in] current number of slices
    UINT_32*            pBytesPerTile   ///< [in,out] pointer to bytes per slice
    ) const
{
    return baseTileMode;
}

/**
****************************************************************************************************
*   CiLib::HwlOptimizeTileMode
*
*   @brief
*       Optimize tile mode on CI
*
*   @return
*       N/A
*
****************************************************************************************************
*/
VOID CiLib::HwlOptimizeTileMode(
    ADDR_COMPUTE_SURFACE_INFO_INPUT*    pInOut      ///< [in,out] input output structure
    ) const
{
    AddrTileMode tileMode = pInOut->tileMode;

    // Override 2D/3D macro tile mode to PRT_* tile mode if
    // client driver requests this surface is equation compatible
    if (IsMacroTiled(tileMode) == TRUE)
    {
        if ((pInOut->flags.needEquation == TRUE) &&
            (pInOut->numSamples <= 1) &&
            (IsPrtTileMode(tileMode) == FALSE))
        {
            if ((pInOut->numSlices > 1) && ((pInOut->maxBaseAlign == 0) || (pInOut->maxBaseAlign >= Block64K)))
            {
                UINT_32 thickness = Thickness(tileMode);

                if (thickness == 1)
                {
                    tileMode = ADDR_TM_PRT_TILED_THIN1;
                }
                else
                {
                    static const UINT_32 PrtTileBytes = 0x10000;
                    // First prt thick tile index in the tile mode table
                    static const UINT_32 PrtThickTileIndex = 22;
                    ADDR_TILEINFO tileInfo = {0};

                    HwlComputeMacroModeIndex(PrtThickTileIndex,
                                             pInOut->flags,
                                             pInOut->bpp,
                                             pInOut->numSamples,
                                             &tileInfo);

                    UINT_32 macroTileBytes = ((pInOut->bpp) >> 3) * 64 * pInOut->numSamples *
                                             thickness * HwlGetPipes(&tileInfo) *
                                             tileInfo.banks * tileInfo.bankWidth *
                                             tileInfo.bankHeight;

                    if (macroTileBytes <= PrtTileBytes)
                    {
                        tileMode = ADDR_TM_PRT_TILED_THICK;
                    }
                    else
                    {
                        tileMode = ADDR_TM_PRT_TILED_THIN1;
                    }
                }
            }
        }

        if (pInOut->maxBaseAlign != 0)
        {
            pInOut->flags.dccPipeWorkaround = FALSE;
        }
    }

    if (tileMode != pInOut->tileMode)
    {
        pInOut->tileMode = tileMode;
    }
}

/**
****************************************************************************************************
*   CiLib::HwlOverrideTileMode
*
*   @brief
*       Override THICK to THIN, for specific formats on CI
*
*   @return
*       N/A
*
****************************************************************************************************
*/
VOID CiLib::HwlOverrideTileMode(
    ADDR_COMPUTE_SURFACE_INFO_INPUT*    pInOut      ///< [in,out] input output structure
    ) const
{
    AddrTileMode tileMode = pInOut->tileMode;
    AddrTileType tileType = pInOut->tileType;

    // currently, all CI/VI family do not
    // support ADDR_TM_PRT_2D_TILED_THICK,ADDR_TM_PRT_3D_TILED_THICK and
    // ADDR_TM_PRT_2D_TILED_THIN1, ADDR_TM_PRT_3D_TILED_THIN1
    switch (tileMode)
    {
        case ADDR_TM_PRT_2D_TILED_THICK:
        case ADDR_TM_PRT_3D_TILED_THICK:
            tileMode = ADDR_TM_PRT_TILED_THICK;
            break;
        case ADDR_TM_PRT_2D_TILED_THIN1:
        case ADDR_TM_PRT_3D_TILED_THIN1:
            tileMode = ADDR_TM_PRT_TILED_THIN1;
            break;
        default:
            break;
    }

    // UBTS#404321, we do not need such overriding, as THICK+THICK entries removed from the tile-mode table
    if (!m_settings.isBonaire)
    {
        UINT_32 thickness = Thickness(tileMode);

        // tile_thickness = (array_mode == XTHICK) ? 8 : ((array_mode == THICK) ? 4 : 1)
        if (thickness > 1)
        {
            switch (pInOut->format)
            {
                // tcpError("Thick micro tiling is not supported for format...
                case ADDR_FMT_X24_8_32_FLOAT:
                case ADDR_FMT_32_AS_8:
                case ADDR_FMT_32_AS_8_8:
                case ADDR_FMT_32_AS_32_32_32_32:

                // packed formats
                case ADDR_FMT_GB_GR:
                case ADDR_FMT_BG_RG:
                case ADDR_FMT_1_REVERSED:
                case ADDR_FMT_1:
                case ADDR_FMT_BC1:
                case ADDR_FMT_BC2:
                case ADDR_FMT_BC3:
                case ADDR_FMT_BC4:
                case ADDR_FMT_BC5:
                case ADDR_FMT_BC6:
                case ADDR_FMT_BC7:
                    switch (tileMode)
                    {
                        case ADDR_TM_1D_TILED_THICK:
                            tileMode = ADDR_TM_1D_TILED_THIN1;
                            break;

                        case ADDR_TM_2D_TILED_XTHICK:
                        case ADDR_TM_2D_TILED_THICK:
                            tileMode = ADDR_TM_2D_TILED_THIN1;
                            break;

                        case ADDR_TM_3D_TILED_XTHICK:
                        case ADDR_TM_3D_TILED_THICK:
                            tileMode = ADDR_TM_3D_TILED_THIN1;
                            break;

                        case ADDR_TM_PRT_TILED_THICK:
                            tileMode = ADDR_TM_PRT_TILED_THIN1;
                            break;

                        case ADDR_TM_PRT_2D_TILED_THICK:
                            tileMode = ADDR_TM_PRT_2D_TILED_THIN1;
                            break;

                        case ADDR_TM_PRT_3D_TILED_THICK:
                            tileMode = ADDR_TM_PRT_3D_TILED_THIN1;
                            break;

                        default:
                            break;

                    }

                    // Switch tile type from thick to thin
                    if (tileMode != pInOut->tileMode)
                    {
                        // see tileIndex: 13-18
                        tileType = ADDR_NON_DISPLAYABLE;
                    }

                    break;
                default:
                    break;
            }
        }
    }

    if (tileMode != pInOut->tileMode)
    {
        pInOut->tileMode = tileMode;
        pInOut->tileType = tileType;
    }
}

/**
****************************************************************************************************
*   CiLib::HwlSelectTileMode
*
*   @brief
*       Select tile modes.
*
*   @return
*       N/A
*
****************************************************************************************************
*/
VOID CiLib::HwlSelectTileMode(
    ADDR_COMPUTE_SURFACE_INFO_INPUT* pInOut     ///< [in,out] input output structure
    ) const
{
    AddrTileMode tileMode;
    AddrTileType tileType;

    if (pInOut->flags.rotateDisplay)
    {
        tileMode = ADDR_TM_2D_TILED_THIN1;
        tileType = ADDR_ROTATED;
    }
    else if (pInOut->flags.volume)
    {
        BOOL_32 bThin = (m_settings.isBonaire == TRUE) ||
                        ((m_allowNonDispThickModes == TRUE) && (pInOut->flags.color == TRUE));

        if (pInOut->numSlices >= 8)
        {
            tileMode = ADDR_TM_2D_TILED_XTHICK;
            tileType = (bThin == TRUE) ? ADDR_NON_DISPLAYABLE : ADDR_THICK;
        }
        else if (pInOut->numSlices >= 4)
        {
            tileMode = ADDR_TM_2D_TILED_THICK;
            tileType = (bThin == TRUE) ? ADDR_NON_DISPLAYABLE : ADDR_THICK;
        }
        else
        {
            tileMode = ADDR_TM_2D_TILED_THIN1;
            tileType = ADDR_NON_DISPLAYABLE;
        }
    }
    else
    {
        tileMode = ADDR_TM_2D_TILED_THIN1;

        if (pInOut->flags.depth || pInOut->flags.stencil)
        {
            tileType = ADDR_DEPTH_SAMPLE_ORDER;
        }
        else if ((pInOut->bpp <= 32) ||
                 (pInOut->flags.display == TRUE) ||
                 (pInOut->flags.overlay == TRUE))
        {
            tileType = ADDR_DISPLAYABLE;
        }
        else
        {
            tileType = ADDR_NON_DISPLAYABLE;
        }
    }

    if (pInOut->flags.prt)
    {
        if (Thickness(tileMode) > 1)
        {
            tileMode = ADDR_TM_PRT_TILED_THICK;
            tileType = (m_settings.isBonaire == TRUE) ? ADDR_NON_DISPLAYABLE : ADDR_THICK;
        }
        else
        {
            tileMode = ADDR_TM_PRT_TILED_THIN1;
        }
    }

    pInOut->tileMode = tileMode;
    pInOut->tileType = tileType;

    if ((pInOut->flags.dccCompatible == FALSE) &&
        (pInOut->flags.tcCompatible == FALSE))
    {
        pInOut->flags.opt4Space = TRUE;
        pInOut->maxBaseAlign = Block64K;
    }

    // Optimize tile mode if possible
    OptimizeTileMode(pInOut);

    HwlOverrideTileMode(pInOut);
}

/**
****************************************************************************************************
*   CiLib::HwlSetPrtTileMode
*
*   @brief
*       Set PRT tile mode.
*
*   @return
*       N/A
*
****************************************************************************************************
*/
VOID CiLib::HwlSetPrtTileMode(
    ADDR_COMPUTE_SURFACE_INFO_INPUT* pInOut     ///< [in,out] input output structure
    ) const
{
    AddrTileMode tileMode = pInOut->tileMode;
    AddrTileType tileType = pInOut->tileType;

    if (Thickness(tileMode) > 1)
    {
        tileMode = ADDR_TM_PRT_TILED_THICK;
        tileType = (m_settings.isBonaire == TRUE) ? ADDR_NON_DISPLAYABLE : ADDR_THICK;
    }
    else
    {
        tileMode = ADDR_TM_PRT_TILED_THIN1;
        tileType = (tileType == ADDR_THICK) ? ADDR_NON_DISPLAYABLE : tileType;
    }

    pInOut->tileMode = tileMode;
    pInOut->tileType = tileType;
}

/**
****************************************************************************************************
*   CiLib::HwlSetupTileInfo
*
*   @brief
*       Setup default value of tile info for SI
****************************************************************************************************
*/
VOID CiLib::HwlSetupTileInfo(
    AddrTileMode                        tileMode,       ///< [in] Tile mode
    ADDR_SURFACE_FLAGS                  flags,          ///< [in] Surface type flags
    UINT_32                             bpp,            ///< [in] Bits per pixel
    UINT_32                             pitch,          ///< [in] Pitch in pixels
    UINT_32                             height,         ///< [in] Height in pixels
    UINT_32                             numSamples,     ///< [in] Number of samples
    ADDR_TILEINFO*                      pTileInfoIn,    ///< [in] Tile info input: NULL for default
    ADDR_TILEINFO*                      pTileInfoOut,   ///< [out] Tile info output
    AddrTileType                        inTileType,     ///< [in] Tile type
    ADDR_COMPUTE_SURFACE_INFO_OUTPUT*   pOut            ///< [out] Output
    ) const
{
    UINT_32 thickness = Thickness(tileMode);
    ADDR_TILEINFO* pTileInfo = pTileInfoOut;
    INT index = TileIndexInvalid;
    INT macroModeIndex = TileIndexInvalid;

    // Fail-safe code
    if (IsLinear(tileMode) == FALSE)
    {
        // Thick tile modes must use thick micro tile mode but Bonaire does not support due to
        // old derived netlists (UBTS 404321)
        if (thickness > 1)
        {
            if (m_settings.isBonaire)
            {
                inTileType = ADDR_NON_DISPLAYABLE;
            }
            else if ((m_allowNonDispThickModes == FALSE) ||
                     (inTileType != ADDR_NON_DISPLAYABLE) ||
                     // There is no PRT_THICK + THIN entry in tile mode table except Bonaire
                     (IsPrtTileMode(tileMode) == TRUE))
            {
                inTileType = ADDR_THICK;
            }
        }
        // 128 bpp tiling must be non-displayable.
        // Fmask reuse color buffer's entry but bank-height field can be from another entry
        // To simplify the logic, fmask entry should be picked from non-displayable ones
        else if (bpp == 128 || flags.fmask)
        {
            inTileType = ADDR_NON_DISPLAYABLE;
        }
        // These two modes only have non-disp entries though they can be other micro tile modes
        else if (tileMode == ADDR_TM_3D_TILED_THIN1 || tileMode == ADDR_TM_PRT_3D_TILED_THIN1)
        {
            inTileType = ADDR_NON_DISPLAYABLE;
        }

        if (flags.depth || flags.stencil)
        {
            inTileType = ADDR_DEPTH_SAMPLE_ORDER;
        }
    }

    // tcCompatible flag is only meaningful for gfx8.
    if (SupportDccAndTcCompatibility() == FALSE)
    {
        flags.tcCompatible = FALSE;
    }

    if (IsTileInfoAllZero(pTileInfo))
    {
        // See table entries 0-4
        if (flags.depth || flags.stencil)
        {
            // tileSize = thickness * bpp * numSamples * 8 * 8 / 8
            UINT_32 tileSize = thickness * bpp * numSamples * 8;

            // Turn off tc compatible if row_size is smaller than tile size (tile split occurs).
            if (m_rowSize < tileSize)
            {
                flags.tcCompatible = FALSE;
            }

            if (flags.nonSplit | flags.tcCompatible | flags.needEquation)
            {
                // Texture readable depth surface should not be split
                switch (tileSize)
                {
                    case 64:
                        index = 0;
                        break;
                    case 128:
                        index = 1;
                        break;
                    case 256:
                        index = 2;
                        break;
                    case 512:
                        index = 3;
                        break;
                    default:
                        index = 4;
                        break;
                }
            }
            else
            {
                // Depth and stencil need to use the same index, thus the pre-defined tile_split
                // can meet the requirement to choose the same macro mode index
                // uncompressed depth/stencil are not supported for now
                switch (numSamples)
                {
                    case 1:
                        index = 0;
                        break;
                    case 2:
                    case 4:
                        index = 1;
                        break;
                    case 8:
                        index = 2;
                        break;
                    default:
                        break;
                }
            }
        }

        // See table entries 5-6
        if (inTileType == ADDR_DEPTH_SAMPLE_ORDER)
        {
            switch (tileMode)
            {
                case ADDR_TM_1D_TILED_THIN1:
                    index = 5;
                    break;
                case ADDR_TM_PRT_TILED_THIN1:
                    index = 6;
                    break;
                default:
                    break;
            }
        }

        // See table entries 8-12
        if (inTileType == ADDR_DISPLAYABLE)
        {
            switch (tileMode)
            {
                case ADDR_TM_1D_TILED_THIN1:
                    index = 9;
                    break;
                case ADDR_TM_2D_TILED_THIN1:
                    index = 10;
                    break;
                case ADDR_TM_PRT_TILED_THIN1:
                    index = 11;
                    break;
                default:
                    break;
            }
        }

        // See table entries 13-18
        if (inTileType == ADDR_NON_DISPLAYABLE)
        {
            switch (tileMode)
            {
                case ADDR_TM_1D_TILED_THIN1:
                    index = 13;
                    break;
                case ADDR_TM_2D_TILED_THIN1:
                    index = 14;
                    break;
                case ADDR_TM_3D_TILED_THIN1:
                    index = 15;
                    break;
                case ADDR_TM_PRT_TILED_THIN1:
                    index = 16;
                    break;
                default:
                    break;
            }
        }

        // See table entries 19-26
        if (thickness > 1)
        {
            switch (tileMode)
            {
                case ADDR_TM_1D_TILED_THICK:
                    // special check for bonaire, for the compatablity between old KMD and new UMD
                    index = ((inTileType == ADDR_THICK) || m_settings.isBonaire) ? 19 : 18;
                    break;
                case ADDR_TM_2D_TILED_THICK:
                    // special check for bonaire, for the compatablity between old KMD and new UMD
                    index = ((inTileType == ADDR_THICK) || m_settings.isBonaire) ? 20 : 24;
                    break;
                case ADDR_TM_3D_TILED_THICK:
                    index = 21;
                    break;
                case ADDR_TM_PRT_TILED_THICK:
                    index = 22;
                    break;
                case ADDR_TM_2D_TILED_XTHICK:
                    index = 25;
                    break;
                case ADDR_TM_3D_TILED_XTHICK:
                    index = 26;
                    break;
                default:
                    break;
            }
        }

        // See table entries 27-30
        if (inTileType == ADDR_ROTATED)
        {
            switch (tileMode)
            {
                case ADDR_TM_1D_TILED_THIN1:
                    index = 27;
                    break;
                case ADDR_TM_2D_TILED_THIN1:
                    index = 28;
                    break;
                case ADDR_TM_PRT_TILED_THIN1:
                    index = 29;
                    break;
                case ADDR_TM_PRT_2D_TILED_THIN1:
                    index = 30;
                    break;
                default:
                    break;
            }
        }

        if (m_pipes >= 8)
        {
            ADDR_ASSERT((index + 1) < static_cast<INT_32>(m_noOfEntries));
            // Only do this when tile mode table is updated.
            if (((tileMode == ADDR_TM_PRT_TILED_THIN1) || (tileMode == ADDR_TM_PRT_TILED_THICK)) &&
                (m_tileTable[index + 1].mode == tileMode))
            {
                static const UINT_32 PrtTileBytes = 0x10000;
                ADDR_TILEINFO tileInfo = {0};

                HwlComputeMacroModeIndex(index, flags, bpp, numSamples, &tileInfo);

                UINT_32 macroTileBytes = (bpp >> 3) * 64 * numSamples * thickness *
                                         HwlGetPipes(&tileInfo) * tileInfo.banks *
                                         tileInfo.bankWidth * tileInfo.bankHeight;

                if (macroTileBytes != PrtTileBytes)
                {
                    // Switching to next tile mode entry to make sure macro tile size is 64KB
                    index += 1;

                    tileInfo.pipeConfig = m_tileTable[index].info.pipeConfig;

                    macroTileBytes = (bpp >> 3) * 64 * numSamples * thickness *
                                     HwlGetPipes(&tileInfo) * tileInfo.banks *
                                     tileInfo.bankWidth * tileInfo.bankHeight;

                    ADDR_ASSERT(macroTileBytes == PrtTileBytes);

                    flags.tcCompatible = FALSE;
                    pOut->dccUnsupport = TRUE;
                }
            }
        }
    }
    else
    {
        // A pre-filled tile info is ready
        index = pOut->tileIndex;
        macroModeIndex = pOut->macroModeIndex;

        // pass tile type back for post tile index compute
        pOut->tileType = inTileType;

        if (flags.depth || flags.stencil)
        {
            // tileSize = thickness * bpp * numSamples * 8 * 8 / 8
            UINT_32 tileSize = thickness * bpp * numSamples * 8;

            // Turn off tc compatible if row_size is smaller than tile size (tile split occurs).
            if (m_rowSize < tileSize)
            {
                flags.tcCompatible = FALSE;
            }
        }

        UINT_32 numPipes = GetPipePerSurf(pTileInfo->pipeConfig);

        if (m_pipes != numPipes)
        {
            pOut->dccUnsupport = TRUE;
        }
    }

    // We only need to set up tile info if there is a valid index but macroModeIndex is invalid
    if ((index != TileIndexInvalid) && (macroModeIndex == TileIndexInvalid))
    {
        macroModeIndex = HwlComputeMacroModeIndex(index, flags, bpp, numSamples, pTileInfo);

        // Copy to pOut->tileType/tileIndex/macroModeIndex
        pOut->tileIndex = index;
        pOut->tileType = m_tileTable[index].type; // Or inTileType, the samea
        pOut->macroModeIndex = macroModeIndex;
    }
    else if (tileMode == ADDR_TM_LINEAR_GENERAL)
    {
        pOut->tileIndex = TileIndexLinearGeneral;

        // Copy linear-aligned entry??
        *pTileInfo = m_tileTable[8].info;
    }
    else if (tileMode == ADDR_TM_LINEAR_ALIGNED)
    {
        pOut->tileIndex = 8;
        *pTileInfo = m_tileTable[8].info;
    }

    if (flags.tcCompatible)
    {
        flags.tcCompatible = CheckTcCompatibility(pTileInfo, bpp, tileMode, inTileType, pOut);
    }

    pOut->tcCompatible = flags.tcCompatible;
}

/**
****************************************************************************************************
*   CiLib::ReadGbTileMode
*
*   @brief
*       Convert GB_TILE_MODE HW value to ADDR_TILE_CONFIG.
****************************************************************************************************
*/
VOID CiLib::ReadGbTileMode(
    UINT_32       regValue,   ///< [in] GB_TILE_MODE register
    TileConfig*   pCfg        ///< [out] output structure
    ) const
{
    GB_TILE_MODE gbTileMode;
    gbTileMode.val = regValue;

    pCfg->type = static_cast<AddrTileType>(gbTileMode.f.micro_tile_mode_new);
    if (AltTilingEnabled() == TRUE)
    {
        pCfg->info.pipeConfig = static_cast<AddrPipeCfg>(gbTileMode.f.alt_pipe_config + 1);
    }
    else
    {
        pCfg->info.pipeConfig = static_cast<AddrPipeCfg>(gbTileMode.f.pipe_config + 1);
    }

    if (pCfg->type == ADDR_DEPTH_SAMPLE_ORDER)
    {
        pCfg->info.tileSplitBytes = 64 << gbTileMode.f.tile_split;
    }
    else
    {
        pCfg->info.tileSplitBytes = 1 << gbTileMode.f.sample_split;
    }

    UINT_32 regArrayMode = gbTileMode.f.array_mode;

    pCfg->mode = static_cast<AddrTileMode>(regArrayMode);

    switch (regArrayMode)
    {
        case 5:
            pCfg->mode = ADDR_TM_PRT_TILED_THIN1;
            break;
        case 6:
            pCfg->mode = ADDR_TM_PRT_2D_TILED_THIN1;
            break;
        case 8:
            pCfg->mode = ADDR_TM_2D_TILED_XTHICK;
            break;
        case 9:
            pCfg->mode = ADDR_TM_PRT_TILED_THICK;
            break;
        case 0xa:
            pCfg->mode = ADDR_TM_PRT_2D_TILED_THICK;
            break;
        case 0xb:
            pCfg->mode = ADDR_TM_PRT_3D_TILED_THIN1;
            break;
        case 0xe:
            pCfg->mode = ADDR_TM_3D_TILED_XTHICK;
            break;
        case 0xf:
            pCfg->mode = ADDR_TM_PRT_3D_TILED_THICK;
            break;
        default:
            break;
    }

    // Fail-safe code for these always convert tile info, as the non-macro modes
    // return the entry of tile mode table directly without looking up macro mode table
    if (!IsMacroTiled(pCfg->mode))
    {
        pCfg->info.banks = 2;
        pCfg->info.bankWidth = 1;
        pCfg->info.bankHeight = 1;
        pCfg->info.macroAspectRatio = 1;
        pCfg->info.tileSplitBytes = 64;
    }
}

/**
****************************************************************************************************
*   CiLib::InitTileSettingTable
*
*   @brief
*       Initialize the ADDR_TILE_CONFIG table.
*   @return
*       TRUE if tile table is correctly initialized
****************************************************************************************************
*/
BOOL_32 CiLib::InitTileSettingTable(
    const UINT_32*  pCfg,           ///< [in] Pointer to table of tile configs
    UINT_32         noOfEntries     ///< [in] Numbe of entries in the table above
    )
{
    BOOL_32 initOk = TRUE;

    ADDR_ASSERT(noOfEntries <= TileTableSize);

    memset(m_tileTable, 0, sizeof(m_tileTable));

    if (noOfEntries != 0)
    {
        m_noOfEntries = noOfEntries;
    }
    else
    {
        m_noOfEntries = TileTableSize;
    }

    if (pCfg) // From Client
    {
        for (UINT_32 i = 0; i < m_noOfEntries; i++)
        {
            ReadGbTileMode(*(pCfg + i), &m_tileTable[i]);
        }
    }
    else
    {
        ADDR_ASSERT_ALWAYS();
        initOk = FALSE;
    }

    if (initOk)
    {
        ADDR_ASSERT(m_tileTable[TILEINDEX_LINEAR_ALIGNED].mode == ADDR_TM_LINEAR_ALIGNED);

        if (m_settings.isBonaire == FALSE)
        {
            // Check if entry 18 is "thick+thin" combination
            if ((m_tileTable[18].mode == ADDR_TM_1D_TILED_THICK) &&
                (m_tileTable[18].type == ADDR_NON_DISPLAYABLE))
            {
                m_allowNonDispThickModes = TRUE;
                ADDR_ASSERT(m_tileTable[24].mode == ADDR_TM_2D_TILED_THICK);
            }
        }
        else
        {
            m_allowNonDispThickModes = TRUE;
        }

        // Assume the first entry is always programmed with full pipes
        m_pipes = HwlGetPipes(&m_tileTable[0].info);
    }

    return initOk;
}

/**
****************************************************************************************************
*   CiLib::ReadGbMacroTileCfg
*
*   @brief
*       Convert GB_MACRO_TILE_CFG HW value to ADDR_TILE_CONFIG.
****************************************************************************************************
*/
VOID CiLib::ReadGbMacroTileCfg(
    UINT_32             regValue,   ///< [in] GB_MACRO_TILE_MODE register
    ADDR_TILEINFO*      pCfg        ///< [out] output structure
    ) const
{
    GB_MACROTILE_MODE gbTileMode;
    gbTileMode.val = regValue;

    if (AltTilingEnabled() == TRUE)
    {
        pCfg->bankHeight       = 1 << gbTileMode.f.alt_bank_height;
        pCfg->banks            = 1 << (gbTileMode.f.alt_num_banks + 1);
        pCfg->macroAspectRatio = 1 << gbTileMode.f.alt_macro_tile_aspect;
    }
    else
    {
        pCfg->bankHeight       = 1 << gbTileMode.f.bank_height;
        pCfg->banks            = 1 << (gbTileMode.f.num_banks + 1);
        pCfg->macroAspectRatio = 1 << gbTileMode.f.macro_tile_aspect;
    }
    pCfg->bankWidth = 1 << gbTileMode.f.bank_width;
}

/**
****************************************************************************************************
*   CiLib::InitMacroTileCfgTable
*
*   @brief
*       Initialize the ADDR_MACRO_TILE_CONFIG table.
*   @return
*       TRUE if macro tile table is correctly initialized
****************************************************************************************************
*/
BOOL_32 CiLib::InitMacroTileCfgTable(
    const UINT_32*  pCfg,           ///< [in] Pointer to table of tile configs
    UINT_32         noOfMacroEntries     ///< [in] Numbe of entries in the table above
    )
{
    BOOL_32 initOk = TRUE;

    ADDR_ASSERT(noOfMacroEntries <= MacroTileTableSize);

    memset(m_macroTileTable, 0, sizeof(m_macroTileTable));

    if (noOfMacroEntries != 0)
    {
        m_noOfMacroEntries = noOfMacroEntries;
    }
    else
    {
        m_noOfMacroEntries = MacroTileTableSize;
    }

    if (pCfg) // From Client
    {
        for (UINT_32 i = 0; i < m_noOfMacroEntries; i++)
        {
            ReadGbMacroTileCfg(*(pCfg + i), &m_macroTileTable[i]);

            m_macroTileTable[i].tileSplitBytes = 64 << (i % 8);
        }
    }
    else
    {
        ADDR_ASSERT_ALWAYS();
        initOk = FALSE;
    }
    return initOk;
}

/**
****************************************************************************************************
*   CiLib::HwlComputeMacroModeIndex
*
*   @brief
*       Computes macro tile mode index
*   @return
*       TRUE if macro tile table is correctly initialized
****************************************************************************************************
*/
INT_32 CiLib::HwlComputeMacroModeIndex(
    INT_32              tileIndex,      ///< [in] Tile mode index
    ADDR_SURFACE_FLAGS  flags,          ///< [in] Surface flags
    UINT_32             bpp,            ///< [in] Bit per pixel
    UINT_32             numSamples,     ///< [in] Number of samples
    ADDR_TILEINFO*      pTileInfo,      ///< [out] Pointer to ADDR_TILEINFO
    AddrTileMode*       pTileMode,      ///< [out] Pointer to AddrTileMode
    AddrTileType*       pTileType       ///< [out] Pointer to AddrTileType
    ) const
{
    INT_32 macroModeIndex = TileIndexInvalid;

    AddrTileMode tileMode = m_tileTable[tileIndex].mode;
    AddrTileType tileType = m_tileTable[tileIndex].type;
    UINT_32 thickness = Thickness(tileMode);

    if (!IsMacroTiled(tileMode))
    {
        *pTileInfo = m_tileTable[tileIndex].info;
        macroModeIndex = TileIndexNoMacroIndex;
    }
    else
    {
        UINT_32 tileBytes1x = BITS_TO_BYTES(bpp * MicroTilePixels * thickness);
        UINT_32 tileSplit;

        if (m_tileTable[tileIndex].type == ADDR_DEPTH_SAMPLE_ORDER)
        {
            // Depth entries store real tileSplitBytes
            tileSplit = m_tileTable[tileIndex].info.tileSplitBytes;
        }
        else
        {
            // Non-depth entries store a split factor
            UINT_32 sampleSplit = m_tileTable[tileIndex].info.tileSplitBytes;
            UINT_32 colorTileSplit = Max(256u, sampleSplit * tileBytes1x);

            tileSplit = colorTileSplit;
        }

        UINT_32 tileSplitC = Min(m_rowSize, tileSplit);
        UINT_32 tileBytes;

        if (flags.fmask)
        {
            tileBytes = Min(tileSplitC, tileBytes1x);
        }
        else
        {
            tileBytes = Min(tileSplitC, numSamples * tileBytes1x);
        }

        if (tileBytes < 64)
        {
            tileBytes = 64;
        }

        macroModeIndex = Log2(tileBytes / 64);

        if (flags.prt || IsPrtTileMode(tileMode))
        {
            macroModeIndex += PrtMacroModeOffset;
            *pTileInfo = m_macroTileTable[macroModeIndex];
        }
        else
        {
            *pTileInfo = m_macroTileTable[macroModeIndex];
        }

        pTileInfo->pipeConfig = m_tileTable[tileIndex].info.pipeConfig;

        pTileInfo->tileSplitBytes = tileSplitC;
    }

    if (NULL != pTileMode)
    {
        *pTileMode = tileMode;
    }

    if (NULL != pTileType)
    {
        *pTileType = tileType;
    }

    return macroModeIndex;
}

/**
****************************************************************************************************
*   CiLib::HwlComputeTileDataWidthAndHeightLinear
*
*   @brief
*       Compute the squared cache shape for per-tile data (CMASK and HTILE) for linear layout
*
*   @note
*       MacroWidth and macroHeight are measured in pixels
****************************************************************************************************
*/
VOID CiLib::HwlComputeTileDataWidthAndHeightLinear(
    UINT_32*        pMacroWidth,     ///< [out] macro tile width
    UINT_32*        pMacroHeight,    ///< [out] macro tile height
    UINT_32         bpp,             ///< [in] bits per pixel
    ADDR_TILEINFO*  pTileInfo        ///< [in] tile info
    ) const
{
    ADDR_ASSERT(pTileInfo != NULL);

    UINT_32 numTiles;

    switch (pTileInfo->pipeConfig)
    {
        case ADDR_PIPECFG_P16_32x32_8x16:
        case ADDR_PIPECFG_P16_32x32_16x16:
        case ADDR_PIPECFG_P8_32x64_32x32:
        case ADDR_PIPECFG_P8_32x32_16x32:
        case ADDR_PIPECFG_P8_32x32_16x16:
        case ADDR_PIPECFG_P8_32x32_8x16:
        case ADDR_PIPECFG_P4_32x32:
            numTiles = 8;
            break;
        default:
            numTiles = 4;
            break;
    }

    *pMacroWidth    = numTiles * MicroTileWidth;
    *pMacroHeight   = numTiles * MicroTileHeight;
}

/**
****************************************************************************************************
*   CiLib::HwlComputeMetadataNibbleAddress
*
*   @brief
*        calculate meta data address based on input information
*
*   &parameter
*        uncompressedDataByteAddress - address of a pixel in color surface
*        dataBaseByteAddress         - base address of color surface
*        metadataBaseByteAddress     - base address of meta ram
*        metadataBitSize             - meta key size, 8 for DCC, 4 for cmask
*        elementBitSize              - element size of color surface
*        blockByteSize               - compression block size, 256 for DCC
*        pipeInterleaveBytes         - pipe interleave size
*        numOfPipes                  - number of pipes
*        numOfBanks                  - number of banks
*        numOfSamplesPerSplit        - number of samples per tile split
*   @return
*        meta data nibble address (nibble address is used to support DCC compatible cmask)
*
****************************************************************************************************
*/
UINT_64 CiLib::HwlComputeMetadataNibbleAddress(
    UINT_64 uncompressedDataByteAddress,
    UINT_64 dataBaseByteAddress,
    UINT_64 metadataBaseByteAddress,
    UINT_32 metadataBitSize,
    UINT_32 elementBitSize,
    UINT_32 blockByteSize,
    UINT_32 pipeInterleaveBytes,
    UINT_32 numOfPipes,
    UINT_32 numOfBanks,
    UINT_32 numOfSamplesPerSplit) const
{
    ///--------------------------------------------------------------------------------------------
    /// Get pipe interleave, bank and pipe bits
    ///--------------------------------------------------------------------------------------------
    UINT_32 pipeInterleaveBits  = Log2(pipeInterleaveBytes);
    UINT_32 pipeBits            = Log2(numOfPipes);
    UINT_32 bankBits            = Log2(numOfBanks);

    ///--------------------------------------------------------------------------------------------
    /// Clear pipe and bank swizzles
    ///--------------------------------------------------------------------------------------------
    UINT_32 dataMacrotileBits        = pipeInterleaveBits + pipeBits + bankBits;
    UINT_32 metadataMacrotileBits    = pipeInterleaveBits + pipeBits + bankBits;

    UINT_64 dataMacrotileClearMask     = ~((1L << dataMacrotileBits) - 1);
    UINT_64 metadataMacrotileClearMask = ~((1L << metadataMacrotileBits) - 1);

    UINT_64 dataBaseByteAddressNoSwizzle = dataBaseByteAddress & dataMacrotileClearMask;
    UINT_64 metadataBaseByteAddressNoSwizzle = metadataBaseByteAddress & metadataMacrotileClearMask;

    ///--------------------------------------------------------------------------------------------
    /// Modify metadata base before adding in so that when final address is divided by data ratio,
    /// the base address returns to where it should be
    ///--------------------------------------------------------------------------------------------
    ADDR_ASSERT((0 != metadataBitSize));
    UINT_64 metadataBaseShifted = metadataBaseByteAddressNoSwizzle * blockByteSize * 8 /
                                  metadataBitSize;
    UINT_64 offset = uncompressedDataByteAddress -
                     dataBaseByteAddressNoSwizzle +
                     metadataBaseShifted;

    ///--------------------------------------------------------------------------------------------
    /// Save bank data bits
    ///--------------------------------------------------------------------------------------------
    UINT_32 lsb = pipeBits + pipeInterleaveBits;
    UINT_32 msb = bankBits - 1 + lsb;

    UINT_64 bankDataBits = GetBits(offset, msb, lsb);

    ///--------------------------------------------------------------------------------------------
    /// Save pipe data bits
    ///--------------------------------------------------------------------------------------------
    lsb = pipeInterleaveBits;
    msb = pipeBits - 1 + lsb;

    UINT_64 pipeDataBits = GetBits(offset, msb, lsb);

    ///--------------------------------------------------------------------------------------------
    /// Remove pipe and bank bits
    ///--------------------------------------------------------------------------------------------
    lsb = pipeInterleaveBits;
    msb = dataMacrotileBits - 1;

    UINT_64 offsetWithoutPipeBankBits = RemoveBits(offset, msb, lsb);

    ADDR_ASSERT((0 != blockByteSize));
    UINT_64 blockInBankpipe = offsetWithoutPipeBankBits / blockByteSize;

    UINT_32 tileSize = 8 * 8 * elementBitSize/8 * numOfSamplesPerSplit;
    UINT_32 blocksInTile = tileSize / blockByteSize;

    if (0 == blocksInTile)
    {
        lsb = 0;
    }
    else
    {
        lsb = Log2(blocksInTile);
    }
    msb = bankBits - 1 + lsb;

    UINT_64 blockInBankpipeWithBankBits = InsertBits(blockInBankpipe, bankDataBits, msb, lsb);

    /// NOTE *2 because we are converting to Nibble address in this step
    UINT_64 metaAddressInPipe = blockInBankpipeWithBankBits * 2 * metadataBitSize / 8;


    ///--------------------------------------------------------------------------------------------
    /// Reinsert pipe bits back into the final address
    ///--------------------------------------------------------------------------------------------
    lsb = pipeInterleaveBits + 1; ///<+1 due to Nibble address now gives interleave bits extra lsb.
    msb = pipeBits - 1 + lsb;
    UINT_64 metadataAddress = InsertBits(metaAddressInPipe, pipeDataBits, msb, lsb);

    return metadataAddress;
}

/**
****************************************************************************************************
*   CiLib::HwlComputeSurfaceAlignmentsMacroTiled
*
*   @brief
*       Hardware layer function to compute alignment request for macro tile mode
*
****************************************************************************************************
*/
VOID CiLib::HwlComputeSurfaceAlignmentsMacroTiled(
    AddrTileMode                      tileMode,           ///< [in] tile mode
    UINT_32                           bpp,                ///< [in] bits per pixel
    ADDR_SURFACE_FLAGS                flags,              ///< [in] surface flags
    UINT_32                           mipLevel,           ///< [in] mip level
    UINT_32                           numSamples,         ///< [in] number of samples
    ADDR_COMPUTE_SURFACE_INFO_OUTPUT* pOut                ///< [in,out] Surface output
    ) const
{
    // This is to workaround a H/W limitation that DCC doesn't work when pipe config is switched to
    // P4. In theory, all asics that have such switching should be patched but we now only know what
    // to pad for Fiji.
    if ((m_settings.isFiji == TRUE) &&
        (flags.dccPipeWorkaround == TRUE) &&
        (flags.prt == FALSE) &&
        (mipLevel == 0) &&
        (tileMode == ADDR_TM_PRT_TILED_THIN1) &&
        (pOut->dccUnsupport == TRUE))
    {
        pOut->pitchAlign   = PowTwoAlign(pOut->pitchAlign, 256);
        // In case the client still requests DCC usage.
        pOut->dccUnsupport = FALSE;
    }
}

/**
****************************************************************************************************
*   CiLib::HwlPadDimensions
*
*   @brief
*       Helper function to pad dimensions
*
****************************************************************************************************
*/
VOID CiLib::HwlPadDimensions(
    AddrTileMode        tileMode,    ///< [in] tile mode
    UINT_32             bpp,         ///< [in] bits per pixel
    ADDR_SURFACE_FLAGS  flags,       ///< [in] surface flags
    UINT_32             numSamples,  ///< [in] number of samples
    ADDR_TILEINFO*      pTileInfo,   ///< [in] tile info
    UINT_32             mipLevel,    ///< [in] mip level
    UINT_32*            pPitch,      ///< [in,out] pitch in pixels
    UINT_32*            pPitchAlign, ///< [in,out] pitch alignment
    UINT_32             height,      ///< [in] height in pixels
    UINT_32             heightAlign  ///< [in] height alignment
    ) const
{
    if ((SupportDccAndTcCompatibility() == TRUE) &&
        (flags.dccCompatible == TRUE) &&
        (numSamples > 1) &&
        (mipLevel == 0) &&
        (IsMacroTiled(tileMode) == TRUE))
    {
        UINT_32 tileSizePerSample = BITS_TO_BYTES(bpp * MicroTileWidth * MicroTileHeight);
        UINT_32 samplesPerSplit  = pTileInfo->tileSplitBytes / tileSizePerSample;

        if (samplesPerSplit < numSamples)
        {
            UINT_32 dccFastClearByteAlign = HwlGetPipes(pTileInfo) * m_pipeInterleaveBytes * 256;
            UINT_32 bytesPerSplit = BITS_TO_BYTES((*pPitch) * height * bpp * samplesPerSplit);

            ADDR_ASSERT(IsPow2(dccFastClearByteAlign));

            if (0 != (bytesPerSplit & (dccFastClearByteAlign - 1)))
            {
                UINT_32 dccFastClearPixelAlign = dccFastClearByteAlign /
                                                BITS_TO_BYTES(bpp) /
                                                samplesPerSplit;
                UINT_32 macroTilePixelAlign = (*pPitchAlign) * heightAlign;

                if ((dccFastClearPixelAlign >= macroTilePixelAlign) &&
                    ((dccFastClearPixelAlign % macroTilePixelAlign) == 0))
                {
                    UINT_32 dccFastClearPitchAlignInMacroTile =
                        dccFastClearPixelAlign / macroTilePixelAlign;
                    UINT_32 heightInMacroTile = height / heightAlign;

                    while ((heightInMacroTile > 1) &&
                           ((heightInMacroTile % 2) == 0) &&
                           (dccFastClearPitchAlignInMacroTile > 1) &&
                           ((dccFastClearPitchAlignInMacroTile % 2) == 0))
                    {
                        heightInMacroTile >>= 1;
                        dccFastClearPitchAlignInMacroTile >>= 1;
                    }

                    UINT_32 dccFastClearPitchAlignInPixels =
                        (*pPitchAlign) * dccFastClearPitchAlignInMacroTile;

                    if (IsPow2(dccFastClearPitchAlignInPixels))
                    {
                        *pPitch = PowTwoAlign((*pPitch), dccFastClearPitchAlignInPixels);
                    }
                    else
                    {
                        *pPitch += (dccFastClearPitchAlignInPixels - 1);
                        *pPitch /= dccFastClearPitchAlignInPixels;
                        *pPitch *= dccFastClearPitchAlignInPixels;
                    }

                    *pPitchAlign = dccFastClearPitchAlignInPixels;
                }
            }
        }
    }
}

/**
****************************************************************************************************
*   CiLib::HwlComputeMaxBaseAlignments
*
*   @brief
*       Gets maximum alignments
*   @return
*       maximum alignments
****************************************************************************************************
*/
UINT_32 CiLib::HwlComputeMaxBaseAlignments() const
{
    const UINT_32 pipes = HwlGetPipes(&m_tileTable[0].info);

    // Initial size is 64 KiB for PRT.
    UINT_32 maxBaseAlign = 64 * 1024;

    for (UINT_32 i = 0; i < m_noOfMacroEntries; i++)
    {
        // The maximum tile size is 16 byte-per-pixel and either 8-sample or 8-slice.
        UINT_32 tileSize = m_macroTileTable[i].tileSplitBytes;

        UINT_32 baseAlign = tileSize * pipes * m_macroTileTable[i].banks *
                            m_macroTileTable[i].bankWidth * m_macroTileTable[i].bankHeight;

        if (baseAlign > maxBaseAlign)
        {
            maxBaseAlign = baseAlign;
        }
    }

    return maxBaseAlign;
}

/**
****************************************************************************************************
*   CiLib::HwlComputeMaxMetaBaseAlignments
*
*   @brief
*       Gets maximum alignments for metadata
*   @return
*       maximum alignments for metadata
****************************************************************************************************
*/
UINT_32 CiLib::HwlComputeMaxMetaBaseAlignments() const
{
    UINT_32 maxBank = 1;

    for (UINT_32 i = 0; i < m_noOfMacroEntries; i++)
    {
        if (SupportDccAndTcCompatibility() && IsMacroTiled(m_tileTable[i].mode))
        {
            maxBank = Max(maxBank, m_macroTileTable[i].banks);
        }
    }

    return SiLib::HwlComputeMaxMetaBaseAlignments() * maxBank;
}

/**
****************************************************************************************************
*   CiLib::DepthStencilTileCfgMatch
*
*   @brief
*       Try to find a tile index for stencil which makes its tile config parameters matches to depth
*   @return
*       TRUE if such tile index for stencil can be found
****************************************************************************************************
*/
BOOL_32 CiLib::DepthStencilTileCfgMatch(
    const ADDR_COMPUTE_SURFACE_INFO_INPUT*  pIn,    ///< [in] input structure
    ADDR_COMPUTE_SURFACE_INFO_OUTPUT*       pOut    ///< [out] output structure
    ) const
{
    BOOL_32 depthStencil2DTileConfigMatch = FALSE;

    for (INT_32 stencilTileIndex = MinDepth2DThinIndex;
         stencilTileIndex <= MaxDepth2DThinIndex;
         stencilTileIndex++)
    {
        ADDR_TILEINFO tileInfo = {0};
        INT_32 stencilMacroIndex = HwlComputeMacroModeIndex(stencilTileIndex,
                                                            pIn->flags,
                                                            8,
                                                            pIn->numSamples,
                                                            &tileInfo);

        if (stencilMacroIndex != TileIndexNoMacroIndex)
        {
            if ((m_macroTileTable[stencilMacroIndex].banks ==
                 m_macroTileTable[pOut->macroModeIndex].banks) &&
                (m_macroTileTable[stencilMacroIndex].bankWidth ==
                 m_macroTileTable[pOut->macroModeIndex].bankWidth) &&
                (m_macroTileTable[stencilMacroIndex].bankHeight ==
                 m_macroTileTable[pOut->macroModeIndex].bankHeight) &&
                (m_macroTileTable[stencilMacroIndex].macroAspectRatio ==
                 m_macroTileTable[pOut->macroModeIndex].macroAspectRatio) &&
                (m_macroTileTable[stencilMacroIndex].pipeConfig ==
                 m_macroTileTable[pOut->macroModeIndex].pipeConfig))
            {
                if ((pOut->tcCompatible == FALSE) ||
                    (tileInfo.tileSplitBytes >= MicroTileWidth * MicroTileHeight * pIn->numSamples))
                {
                    depthStencil2DTileConfigMatch = TRUE;
                    pOut->stencilTileIdx = stencilTileIndex;
                    break;
                }
            }
        }
        else
        {
            ADDR_ASSERT_ALWAYS();
        }
    }

    return depthStencil2DTileConfigMatch;
}

/**
****************************************************************************************************
*   CiLib::DepthStencilTileCfgMatch
*
*   @brief
*       Check if tc compatibility is available
*   @return
*       If tc compatibility is not available
****************************************************************************************************
*/
BOOL_32 CiLib::CheckTcCompatibility(
    const ADDR_TILEINFO*                    pTileInfo,    ///< [in] input tile info
    UINT_32                                 bpp,          ///< [in] Bits per pixel
    AddrTileMode                            tileMode,     ///< [in] input tile mode
    AddrTileType                            tileType,     ///< [in] input tile type
    const ADDR_COMPUTE_SURFACE_INFO_OUTPUT* pOut          ///< [in] output surf info
    ) const
{
    BOOL_32 tcCompatible = TRUE;

    if (IsMacroTiled(tileMode))
    {
        if (tileType != ADDR_DEPTH_SAMPLE_ORDER)
        {
            // Turn off tcCompatible for color surface if tileSplit happens. Depth/stencil
            // tileSplit case was handled at tileIndex selecting time.
            INT_32 tileIndex = pOut->tileIndex;

            if ((tileIndex == TileIndexInvalid) && (IsTileInfoAllZero(pTileInfo) == FALSE))
            {
                tileIndex = HwlPostCheckTileIndex(pTileInfo, tileMode, tileType, tileIndex);
            }

            if (tileIndex != TileIndexInvalid)
            {
                UINT_32 thickness = Thickness(tileMode);

                ADDR_ASSERT(static_cast<UINT_32>(tileIndex) < TileTableSize);
                // Non-depth entries store a split factor
                UINT_32 sampleSplit = m_tileTable[tileIndex].info.tileSplitBytes;
                UINT_32 tileBytes1x = BITS_TO_BYTES(bpp * MicroTilePixels * thickness);
                UINT_32 colorTileSplit = Max(256u, sampleSplit * tileBytes1x);

                if (m_rowSize < colorTileSplit)
                {
                    tcCompatible = FALSE;
                }
            }
        }
    }
    else
    {
        // Client should not enable tc compatible for linear and 1D tile modes.
        tcCompatible = FALSE;
    }

    return tcCompatible;
}

} // V1
} // Addr
