/*
************************************************************************************************************************
*
*  Copyright (C) 2007-2022 Advanced Micro Devices, Inc.  All rights reserved.
*  SPDX-License-Identifier: MIT
*
***********************************************************************************************************************/

/**
************************************************************************************************************************
* @file  gfx11addrlib.cpp
* @brief Contain the implementation for the Gfx11Lib class.
************************************************************************************************************************
*/

#include "gfx11addrlib.h"
#include "gfx11_gb_reg.h"

#include "amdgpu_asic_addr.h"

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace Addr
{
/**
************************************************************************************************************************
*   Gfx11HwlInit
*
*   @brief
*       Creates an Gfx11Lib object.
*
*   @return
*       Returns an Gfx11Lib object pointer.
************************************************************************************************************************
*/
Addr::Lib* Gfx11HwlInit(const Client* pClient)
{
    return V2::Gfx11Lib::CreateObj(pClient);
}

namespace V2
{

////////////////////////////////////////////////////////////////////////////////////////////////////
//                               Static Const Member
////////////////////////////////////////////////////////////////////////////////////////////////////

const SwizzleModeFlags Gfx11Lib::SwizzleModeTable[ADDR_SW_MAX_TYPE] =
{//Linear 256B  4KB  64KB  256KB   Z    Std   Disp  Rot   XOR    T     RtOpt Reserved
    {{1,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0}}, // ADDR_SW_LINEAR
    {{0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0}}, // Reserved
    {{0,    1,    0,    0,    0,    0,    0,    1,    0,    0,    0,    0,    0}}, // ADDR_SW_256B_D
    {{0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0}}, // Reserved

    {{0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0}}, // Reserved
    {{0,    0,    1,    0,    0,    0,    1,    0,    0,    0,    0,    0,    0}}, // ADDR_SW_4KB_S
    {{0,    0,    1,    0,    0,    0,    0,    1,    0,    0,    0,    0,    0}}, // ADDR_SW_4KB_D
    {{0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0}}, // Reserved

    {{0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0}}, // Reserved
    {{0,    0,    0,    1,    0,    0,    1,    0,    0,    0,    0,    0,    0}}, // ADDR_SW_64KB_S
    {{0,    0,    0,    1,    0,    0,    0,    1,    0,    0,    0,    0,    0}}, // ADDR_SW_64KB_D
    {{0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0}}, // Reserved

    {{0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0}}, // Reserved
    {{0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0}}, // Reserved
    {{0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0}}, // Reserved
    {{0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0}}, // Reserved

    {{0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0}}, // Reserved
    {{0,    0,    0,    1,    0,    0,    1,    0,    0,    1,    1,    0,    0}}, // ADDR_SW_64KB_S_T
    {{0,    0,    0,    1,    0,    0,    0,    1,    0,    1,    1,    0,    0}}, // ADDR_SW_64KB_D_T
    {{0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0}}, // Reserved

    {{0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0}}, // Reserved
    {{0,    0,    1,    0,    0,    0,    1,    0,    0,    1,    0,    0,    0}}, // ADDR_SW_4KB_S_X
    {{0,    0,    1,    0,    0,    0,    0,    1,    0,    1,    0,    0,    0}}, // ADDR_SW_4KB_D_X
    {{0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0}}, // Reserved

    {{0,    0,    0,    1,    0,    1,    0,    0,    0,    1,    0,    0,    0}}, // ADDR_SW_64KB_Z_X
    {{0,    0,    0,    1,    0,    0,    1,    0,    0,    1,    0,    0,    0}}, // ADDR_SW_64KB_S_X
    {{0,    0,    0,    1,    0,    0,    0,    1,    0,    1,    0,    0,    0}}, // ADDR_SW_64KB_D_X
    {{0,    0,    0,    1,    0,    0,    0,    0,    0,    1,    0,    1,    0}}, // ADDR_SW_64KB_R_X

    {{0,    0,    0,    0,    1,    1,    0,    0,    0,    1,    0,    0,    0}}, // ADDR_SW_256KB_Z_X
    {{0,    0,    0,    0,    1,    0,    1,    0,    0,    1,    0,    0,    0}}, // ADDR_SW_256KB_S_X
    {{0,    0,    0,    0,    1,    0,    0,    1,    0,    1,    0,    0,    0}}, // ADDR_SW_256KB_D_X
    {{0,    0,    0,    0,    1,    0,    0,    0,    0,    1,    0,    1,    0}}, // ADDR_SW_256KB_R_X
    {{1,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0}}, // ADDR_SW_LINEAR_GENERAL
};

const Dim3d Gfx11Lib::Block256_3d[] = {{8, 4, 8}, {4, 4, 8}, {4, 4, 4}, {4, 2, 4}, {2, 2, 4}};

const Dim3d Gfx11Lib::Block256K_Log2_3d[] = {{6, 6, 6}, {5, 6, 6}, {5, 6, 5}, {5, 5, 5}, {4, 5, 5}};
const Dim3d Gfx11Lib::Block64K_Log2_3d[]  = {{6, 5, 5}, {5, 5, 5}, {5, 5, 4}, {5, 4, 4}, {4, 4, 4}};
const Dim3d Gfx11Lib::Block4K_Log2_3d[]   = {{4, 4, 4}, {3, 4, 4}, {3, 4, 3}, {3, 3, 3}, {2, 3, 3}};

/**
************************************************************************************************************************
*   Gfx11Lib::Gfx11Lib
*
*   @brief
*       Constructor
*
************************************************************************************************************************
*/
Gfx11Lib::Gfx11Lib(const Client* pClient)
    :
    Lib(pClient),
    m_numPkrLog2(0),
    m_numSaLog2(0),
    m_colorBaseIndex(0),
    m_htileBaseIndex(0),
    m_dccBaseIndex(0)
{
    memset(&m_settings, 0, sizeof(m_settings));
    memcpy(m_swizzleModeTable, SwizzleModeTable, sizeof(SwizzleModeTable));
}

/**
************************************************************************************************************************
*   Gfx11Lib::~Gfx11Lib
*
*   @brief
*       Destructor
************************************************************************************************************************
*/
Gfx11Lib::~Gfx11Lib()
{
}

/**
************************************************************************************************************************
*   Gfx11Lib::HwlComputeHtileInfo
*
*   @brief
*       Interface function stub of AddrComputeHtilenfo
*
*   @return
*       ADDR_E_RETURNCODE
************************************************************************************************************************
*/
ADDR_E_RETURNCODE Gfx11Lib::HwlComputeHtileInfo(
    const ADDR2_COMPUTE_HTILE_INFO_INPUT* pIn,    ///< [in] input structure
    ADDR2_COMPUTE_HTILE_INFO_OUTPUT*      pOut    ///< [out] output structure
    ) const
{
    ADDR_E_RETURNCODE ret = ADDR_OK;

    if ((pIn->swizzleMode != ADDR_SW_64KB_Z_X)  &&
        (pIn->swizzleMode != ADDR_SW_256KB_Z_X) &&
        (pIn->hTileFlags.pipeAligned != TRUE))
    {
        ret = ADDR_INVALIDPARAMS;
    }
    else
    {
        Dim3d         metaBlk     = {};
        const UINT_32 metaBlkSize = GetMetaBlkSize(Gfx11DataDepthStencil,
                                                   ADDR_RSRC_TEX_2D,
                                                   pIn->swizzleMode,
                                                   0,
                                                   0,
                                                   TRUE,
                                                   &metaBlk);

        pOut->pitch         = PowTwoAlign(pIn->unalignedWidth,  metaBlk.w);
        pOut->height        = PowTwoAlign(pIn->unalignedHeight, metaBlk.h);
        pOut->baseAlign     = Max(metaBlkSize, 1u << (m_pipesLog2 + 11u));
        pOut->metaBlkWidth  = metaBlk.w;
        pOut->metaBlkHeight = metaBlk.h;

        if (pIn->numMipLevels > 1)
        {
            ADDR_ASSERT(pIn->firstMipIdInTail <= pIn->numMipLevels);

            UINT_32 offset = (pIn->firstMipIdInTail == pIn->numMipLevels) ? 0 : metaBlkSize;

            for (INT_32 i = static_cast<INT_32>(pIn->firstMipIdInTail) - 1; i >=0; i--)
            {
                UINT_32 mipWidth, mipHeight;

                GetMipSize(pIn->unalignedWidth, pIn->unalignedHeight, 1, i, &mipWidth, &mipHeight);

                mipWidth  = PowTwoAlign(mipWidth,  metaBlk.w);
                mipHeight = PowTwoAlign(mipHeight, metaBlk.h);

                const UINT_32 pitchInM     = mipWidth  / metaBlk.w;
                const UINT_32 heightInM    = mipHeight / metaBlk.h;
                const UINT_32 mipSliceSize = pitchInM * heightInM * metaBlkSize;

                if (pOut->pMipInfo != NULL)
                {
                    pOut->pMipInfo[i].inMiptail = FALSE;
                    pOut->pMipInfo[i].offset    = offset;
                    pOut->pMipInfo[i].sliceSize = mipSliceSize;
                }

                offset += mipSliceSize;
            }

            pOut->sliceSize          = offset;
            pOut->metaBlkNumPerSlice = offset / metaBlkSize;
            pOut->htileBytes         = pOut->sliceSize * pIn->numSlices;

            if (pOut->pMipInfo != NULL)
            {
                for (UINT_32 i = pIn->firstMipIdInTail; i < pIn->numMipLevels; i++)
                {
                    pOut->pMipInfo[i].inMiptail = TRUE;
                    pOut->pMipInfo[i].offset    = 0;
                    pOut->pMipInfo[i].sliceSize = 0;
                }

                if (pIn->firstMipIdInTail != pIn->numMipLevels)
                {
                    pOut->pMipInfo[pIn->firstMipIdInTail].sliceSize = metaBlkSize;
                }
            }
        }
        else
        {
            const UINT_32 pitchInM  = pOut->pitch  / metaBlk.w;
            const UINT_32 heightInM = pOut->height / metaBlk.h;

            pOut->metaBlkNumPerSlice    = pitchInM * heightInM;
            pOut->sliceSize             = pOut->metaBlkNumPerSlice * metaBlkSize;
            pOut->htileBytes            = pOut->sliceSize * pIn->numSlices;

            if (pOut->pMipInfo != NULL)
            {
                pOut->pMipInfo[0].inMiptail = FALSE;
                pOut->pMipInfo[0].offset    = 0;
                pOut->pMipInfo[0].sliceSize = pOut->sliceSize;
            }
        }

        // Get the HTILE address equation (copied from HtileAddrFromCoord).
        // HTILE addressing depends on the number of samples, but this code doesn't support it yet.
        const UINT_32  index         = m_htileBaseIndex;
        const UINT_8* patIdxTable = GFX11_HTILE_PATIDX;

        ADDR_C_ASSERT(sizeof(GFX11_HTILE_SW_PATTERN[patIdxTable[index]]) == 72 * 2);
        pOut->equation.gfx10_bits = (UINT_16 *)GFX11_HTILE_SW_PATTERN[patIdxTable[index]];
    }

    return ret;
}

/**
************************************************************************************************************************
*   Gfx11Lib::HwlComputeDccInfo
*
*   @brief
*       Interface function to compute DCC key info
*
*   @return
*       ADDR_E_RETURNCODE
************************************************************************************************************************
*/
ADDR_E_RETURNCODE Gfx11Lib::HwlComputeDccInfo(
    const ADDR2_COMPUTE_DCCINFO_INPUT* pIn,    ///< [in] input structure
    ADDR2_COMPUTE_DCCINFO_OUTPUT*      pOut    ///< [out] output structure
    ) const
{
    ADDR_E_RETURNCODE ret = ADDR_OK;

    if (IsLinear(pIn->swizzleMode) || IsBlock256b(pIn->swizzleMode))
    {
        ret = ADDR_INVALIDPARAMS;
    }
    else
    {
        const UINT_32 elemLog2    = Log2(pIn->bpp >> 3);
        const UINT_32 numFragLog2 = Log2(Max(pIn->numFrags, 1u));
        Dim3d         compBlock   = {};

        GetCompressedBlockSizeLog2(Gfx11DataColor,
                                   pIn->resourceType,
                                   pIn->swizzleMode,
                                   elemLog2,
                                   numFragLog2,
                                   &compBlock);
        pOut->compressBlkWidth  = 1 << compBlock.w;
        pOut->compressBlkHeight = 1 << compBlock.h;
        pOut->compressBlkDepth  = 1 << compBlock.d;

        if (ret == ADDR_OK)
        {
            Dim3d         metaBlk     = {};
            const UINT_32 metaBlkSize = GetMetaBlkSize(Gfx11DataColor,
                                                       pIn->resourceType,
                                                       pIn->swizzleMode,
                                                       elemLog2,
                                                       numFragLog2,
                                                       pIn->dccKeyFlags.pipeAligned,
                                                       &metaBlk);

            pOut->dccRamBaseAlign   = metaBlkSize;
            pOut->metaBlkWidth      = metaBlk.w;
            pOut->metaBlkHeight     = metaBlk.h;
            pOut->metaBlkDepth      = metaBlk.d;
            pOut->metaBlkSize       = metaBlkSize;

            pOut->pitch             = PowTwoAlign(pIn->unalignedWidth,     metaBlk.w);
            pOut->height            = PowTwoAlign(pIn->unalignedHeight,    metaBlk.h);
            pOut->depth             = PowTwoAlign(Max(pIn->numSlices, 1u), metaBlk.d);

            if (pIn->numMipLevels > 1)
            {
                ADDR_ASSERT(pIn->firstMipIdInTail <= pIn->numMipLevels);

                UINT_32 offset = (pIn->firstMipIdInTail == pIn->numMipLevels) ? 0 : metaBlkSize;

                for (INT_32 i = static_cast<INT_32>(pIn->firstMipIdInTail) - 1; i >= 0; i--)
                {
                    UINT_32 mipWidth, mipHeight;

                    GetMipSize(pIn->unalignedWidth, pIn->unalignedHeight, 1, i, &mipWidth, &mipHeight);

                    mipWidth  = PowTwoAlign(mipWidth,  metaBlk.w);
                    mipHeight = PowTwoAlign(mipHeight, metaBlk.h);

                    const UINT_32 pitchInM     = mipWidth  / metaBlk.w;
                    const UINT_32 heightInM    = mipHeight / metaBlk.h;
                    const UINT_32 mipSliceSize = pitchInM * heightInM * metaBlkSize;

                    if (pOut->pMipInfo != NULL)
                    {
                        pOut->pMipInfo[i].inMiptail = FALSE;
                        pOut->pMipInfo[i].offset    = offset;
                        pOut->pMipInfo[i].sliceSize = mipSliceSize;
                    }

                    offset += mipSliceSize;
                }

                pOut->dccRamSliceSize    = offset;
                pOut->metaBlkNumPerSlice = offset / metaBlkSize;
                pOut->dccRamSize         = pOut->dccRamSliceSize * (pOut->depth  / metaBlk.d);

                if (pOut->pMipInfo != NULL)
                {
                    for (UINT_32 i = pIn->firstMipIdInTail; i < pIn->numMipLevels; i++)
                    {
                        pOut->pMipInfo[i].inMiptail = TRUE;
                        pOut->pMipInfo[i].offset    = 0;
                        pOut->pMipInfo[i].sliceSize = 0;
                    }

                    if (pIn->firstMipIdInTail != pIn->numMipLevels)
                    {
                        pOut->pMipInfo[pIn->firstMipIdInTail].sliceSize = metaBlkSize;
                    }
                }
            }
            else
            {
                const UINT_32 pitchInM  = pOut->pitch  / metaBlk.w;
                const UINT_32 heightInM = pOut->height / metaBlk.h;

                pOut->metaBlkNumPerSlice = pitchInM * heightInM;
                pOut->dccRamSliceSize    = pOut->metaBlkNumPerSlice * metaBlkSize;
                pOut->dccRamSize         = pOut->dccRamSliceSize * (pOut->depth  / metaBlk.d);

                if (pOut->pMipInfo != NULL)
                {
                    pOut->pMipInfo[0].inMiptail = FALSE;
                    pOut->pMipInfo[0].offset    = 0;
                    pOut->pMipInfo[0].sliceSize = pOut->dccRamSliceSize;
                }
            }

            // Get the DCC address equation (copied from DccAddrFromCoord)
            const UINT_32 elemLog2    = Log2(pIn->bpp >> 3);
            const UINT_32 numPipeLog2 = m_pipesLog2;
            UINT_32       index       = m_dccBaseIndex + elemLog2;
            const UINT_8* patIdxTable = (pIn->swizzleMode == ADDR_SW_64KB_R_X) ?
                                        GFX11_DCC_64K_R_X_PATIDX : GFX11_DCC_256K_R_X_PATIDX;

            if (pIn->dccKeyFlags.pipeAligned)
            {
                index += MaxNumOfBpp;

                if (m_numPkrLog2 < 2)
                {
                    index += m_pipesLog2 * MaxNumOfBpp;
                }
                else
                {
                    // 4 groups for "m_numPkrLog2 < 2" case
                    index += 4 * MaxNumOfBpp;

                    const UINT_32 dccPipePerPkr = 3;

                    index += (m_numPkrLog2 - 2) * dccPipePerPkr * MaxNumOfBpp +
                             (m_pipesLog2 - m_numPkrLog2) * MaxNumOfBpp;
                }
            }

            ADDR_C_ASSERT(sizeof(GFX11_DCC_R_X_SW_PATTERN[patIdxTable[index]]) == 68 * 2);
            pOut->equation.gfx10_bits = (UINT_16*)GFX11_DCC_R_X_SW_PATTERN[patIdxTable[index]];
        }
    }

    return ret;
}

/**
************************************************************************************************************************
*   Gfx11Lib::HwlComputeHtileAddrFromCoord
*
*   @brief
*       Interface function stub of AddrComputeHtileAddrFromCoord
*
*   @return
*       ADDR_E_RETURNCODE
************************************************************************************************************************
*/
ADDR_E_RETURNCODE Gfx11Lib::HwlComputeHtileAddrFromCoord(
    const ADDR2_COMPUTE_HTILE_ADDRFROMCOORD_INPUT* pIn,    ///< [in] input structure
    ADDR2_COMPUTE_HTILE_ADDRFROMCOORD_OUTPUT*      pOut)   ///< [out] output structure
{
    ADDR_E_RETURNCODE returnCode = ADDR_OK;

    if (pIn->numMipLevels > 1)
    {
        returnCode = ADDR_NOTIMPLEMENTED;
    }
    else
    {
        ADDR2_COMPUTE_HTILE_INFO_INPUT input = {};
        input.size            = sizeof(input);
        input.hTileFlags      = pIn->hTileFlags;
        input.depthFlags      = pIn->depthflags;
        input.swizzleMode     = pIn->swizzleMode;
        input.unalignedWidth  = Max(pIn->unalignedWidth,  1u);
        input.unalignedHeight = Max(pIn->unalignedHeight, 1u);
        input.numSlices       = Max(pIn->numSlices,       1u);
        input.numMipLevels    = 1;

        ADDR2_COMPUTE_HTILE_INFO_OUTPUT output = {};
        output.size = sizeof(output);

        returnCode = ComputeHtileInfo(&input, &output);

        if (returnCode == ADDR_OK)
        {
            const UINT_32  numSampleLog2 = Log2(pIn->numSamples);
            const UINT_32  pipeMask      = (1 << m_pipesLog2) - 1;
            const UINT_32  index         = m_htileBaseIndex + numSampleLog2;
            const UINT_8*  patIdxTable   = GFX11_HTILE_PATIDX;
            const UINT_32  blkSizeLog2   = Log2(output.metaBlkWidth) + Log2(output.metaBlkHeight) - 4;
            const UINT_32  blkMask       = (1 << blkSizeLog2) - 1;
            const UINT_32  blkOffset     = ComputeOffsetFromSwizzlePattern(GFX11_HTILE_SW_PATTERN[patIdxTable[index]],
                                                                           blkSizeLog2 + 1, // +1 for nibble offset
                                                                           pIn->x,
                                                                           pIn->y,
                                                                           pIn->slice,
                                                                           0);
            const UINT_32 xb       = pIn->x / output.metaBlkWidth;
            const UINT_32 yb       = pIn->y / output.metaBlkHeight;
            const UINT_32 pb       = output.pitch / output.metaBlkWidth;
            const UINT_32 blkIndex = (yb * pb) + xb;
            const UINT_32 pipeXor  = ((pIn->pipeXor & pipeMask) << m_pipeInterleaveLog2) & blkMask;

            pOut->addr = (static_cast<UINT_64>(output.sliceSize) * pIn->slice) +
                         (blkIndex * (1 << blkSizeLog2)) +
                         ((blkOffset >> 1) ^ pipeXor);
        }
    }

    return returnCode;
}

/**
************************************************************************************************************************
*   Gfx11Lib::HwlComputeHtileCoordFromAddr
*
*   @brief
*       Interface function stub of AddrComputeHtileCoordFromAddr
*
*   @return
*       ADDR_E_RETURNCODE
************************************************************************************************************************
*/
ADDR_E_RETURNCODE Gfx11Lib::HwlComputeHtileCoordFromAddr(
    const ADDR2_COMPUTE_HTILE_COORDFROMADDR_INPUT* pIn,    ///< [in] input structure
    ADDR2_COMPUTE_HTILE_COORDFROMADDR_OUTPUT*      pOut)   ///< [out] output structure
{
    ADDR_NOT_IMPLEMENTED();

    return ADDR_OK;
}

/**
************************************************************************************************************************
*   Gfx11Lib::HwlSupportComputeDccAddrFromCoord
*
*   @brief
*       Check whether HwlComputeDccAddrFromCoord() can be done for the input parameter
*
*   @return
*       ADDR_E_RETURNCODE
************************************************************************************************************************
*/
ADDR_E_RETURNCODE Gfx11Lib::HwlSupportComputeDccAddrFromCoord(
    const ADDR2_COMPUTE_DCC_ADDRFROMCOORD_INPUT* pIn)
{
    ADDR_E_RETURNCODE returnCode = ADDR_OK;

    if ((pIn->resourceType != ADDR_RSRC_TEX_2D) ||
        ((pIn->swizzleMode != ADDR_SW_64KB_R_X) &&
         (pIn->swizzleMode != ADDR_SW_256KB_R_X)) ||
        (pIn->dccKeyFlags.linear == TRUE) ||
        (pIn->numFrags > 1) ||
        (pIn->numMipLevels > 1) ||
        (pIn->mipId > 0))
    {
        returnCode = ADDR_NOTSUPPORTED;
    }
    else if ((pIn->pitch == 0)         ||
             (pIn->metaBlkWidth == 0)  ||
             (pIn->metaBlkHeight == 0) ||
             (pIn->slice > 0 && pIn->dccRamSliceSize == 0))
    {
        returnCode = ADDR_NOTSUPPORTED;
    }

    return returnCode;
}

/**
************************************************************************************************************************
*   Gfx11Lib::HwlComputeDccAddrFromCoord
*
*   @brief
*       Interface function stub of AddrComputeDccAddrFromCoord
*
*   @return
*       N/A
************************************************************************************************************************
*/
VOID Gfx11Lib::HwlComputeDccAddrFromCoord(
    const ADDR2_COMPUTE_DCC_ADDRFROMCOORD_INPUT* pIn,  ///< [in] input structure
    ADDR2_COMPUTE_DCC_ADDRFROMCOORD_OUTPUT*      pOut) ///< [out] output structure
{
    const UINT_32 elemLog2    = Log2(pIn->bpp >> 3);
    const UINT_32 numPipeLog2 = m_pipesLog2;
    const UINT_32 pipeMask    = (1 << numPipeLog2) - 1;
    UINT_32       index       = m_dccBaseIndex + elemLog2;
    const UINT_8* patIdxTable = (pIn->swizzleMode == ADDR_SW_64KB_R_X) ?
                                GFX11_DCC_64K_R_X_PATIDX : GFX11_DCC_256K_R_X_PATIDX;

    if (pIn->dccKeyFlags.pipeAligned)
    {
        index += MaxNumOfBpp;

        if (m_numPkrLog2 < 2)
        {
            index += m_pipesLog2 * MaxNumOfBpp;
        }
        else
        {
            // 4 groups for "m_numPkrLog2 < 2" case
            index += 4 * MaxNumOfBpp;

            const UINT_32 dccPipePerPkr = 3;

            index += (m_numPkrLog2 - 2) * dccPipePerPkr * MaxNumOfBpp +
                     (m_pipesLog2 - m_numPkrLog2) * MaxNumOfBpp;
        }
    }

    const UINT_32  blkSizeLog2 = Log2(pIn->metaBlkWidth) + Log2(pIn->metaBlkHeight) + elemLog2 - 8;
    const UINT_32  blkMask     = (1 << blkSizeLog2) - 1;
    const UINT_32  blkOffset   = ComputeOffsetFromSwizzlePattern(GFX11_DCC_R_X_SW_PATTERN[patIdxTable[index]],
                                                                 blkSizeLog2 + 1, // +1 for nibble offset
                                                                 pIn->x,
                                                                 pIn->y,
                                                                 pIn->slice,
                                                                 0);
    const UINT_32 xb       = pIn->x / pIn->metaBlkWidth;
    const UINT_32 yb       = pIn->y / pIn->metaBlkHeight;
    const UINT_32 pb       = pIn->pitch / pIn->metaBlkWidth;
    const UINT_32 blkIndex = (yb * pb) + xb;
    const UINT_32 pipeXor  = ((pIn->pipeXor & pipeMask) << m_pipeInterleaveLog2) & blkMask;

    pOut->addr = (static_cast<UINT_64>(pIn->dccRamSliceSize) * pIn->slice) +
                 (blkIndex * (1 << blkSizeLog2)) +
                 ((blkOffset >> 1) ^ pipeXor);
}

/**
************************************************************************************************************************
*   Gfx11Lib::HwlInitGlobalParams
*
*   @brief
*       Initializes global parameters
*
*   @return
*       TRUE if all settings are valid
*
************************************************************************************************************************
*/
BOOL_32 Gfx11Lib::HwlInitGlobalParams(
    const ADDR_CREATE_INPUT* pCreateIn) ///< [in] create input
{
    BOOL_32              valid = TRUE;
    GB_ADDR_CONFIG_GFX11 gbAddrConfig;

    gbAddrConfig.u32All = pCreateIn->regValue.gbAddrConfig;

    switch (gbAddrConfig.bits.NUM_PIPES)
    {
        case ADDR_CONFIG_1_PIPE:
            m_pipes     = 1;
            m_pipesLog2 = 0;
            break;
        case ADDR_CONFIG_2_PIPE:
            m_pipes     = 2;
            m_pipesLog2 = 1;
            break;
        case ADDR_CONFIG_4_PIPE:
            m_pipes     = 4;
            m_pipesLog2 = 2;
            break;
        case ADDR_CONFIG_8_PIPE:
            m_pipes     = 8;
            m_pipesLog2 = 3;
            break;
        case ADDR_CONFIG_16_PIPE:
            m_pipes     = 16;
            m_pipesLog2 = 4;
            break;
        case ADDR_CONFIG_32_PIPE:
            m_pipes     = 32;
            m_pipesLog2 = 5;
            break;
        case ADDR_CONFIG_64_PIPE:
            m_pipes     = 64;
            m_pipesLog2 = 6;
            break;
        default:
            ADDR_ASSERT_ALWAYS();
            valid = FALSE;
            break;
    }

    switch (gbAddrConfig.bits.PIPE_INTERLEAVE_SIZE)
    {
        case ADDR_CONFIG_PIPE_INTERLEAVE_256B:
            m_pipeInterleaveBytes = ADDR_PIPEINTERLEAVE_256B;
            m_pipeInterleaveLog2  = 8;
            break;
        case ADDR_CONFIG_PIPE_INTERLEAVE_512B:
            m_pipeInterleaveBytes = ADDR_PIPEINTERLEAVE_512B;
            m_pipeInterleaveLog2  = 9;
            break;
        case ADDR_CONFIG_PIPE_INTERLEAVE_1KB:
            m_pipeInterleaveBytes = ADDR_PIPEINTERLEAVE_1KB;
            m_pipeInterleaveLog2  = 10;
            break;
        case ADDR_CONFIG_PIPE_INTERLEAVE_2KB:
            m_pipeInterleaveBytes = ADDR_PIPEINTERLEAVE_2KB;
            m_pipeInterleaveLog2  = 11;
            break;
        default:
            ADDR_ASSERT_ALWAYS();
            valid = FALSE;
            break;
    }

    // Addr::V2::Lib::ComputePipeBankXor()/ComputeSlicePipeBankXor() requires pipe interleave to be exactly 8 bits, and
    // any larger value requires a post-process (left shift) on the output pipeBankXor bits.
    // And more importantly, SW AddrLib doesn't support sw equation/pattern for PI != 256 case.
    ADDR_ASSERT(m_pipeInterleaveBytes == ADDR_PIPEINTERLEAVE_256B);

    // These fields are deprecated on GFX11; they do nothing on HW.
    m_maxCompFrag     = 1;
    m_maxCompFragLog2 = 0;

    // Skip unaligned case
    m_htileBaseIndex += MaxNumOfAA;

    m_htileBaseIndex += m_pipesLog2 * MaxNumOfAA;
    m_colorBaseIndex += m_pipesLog2 * MaxNumOfBpp;

    m_numPkrLog2 = gbAddrConfig.bits.NUM_PKRS;
    m_numSaLog2  = (m_numPkrLog2 > 0) ? (m_numPkrLog2 - 1) : 0;

    ADDR_ASSERT((m_numPkrLog2 <= m_pipesLog2) && ((m_pipesLog2 - m_numPkrLog2) <= 2));

    if (m_numPkrLog2 >= 2)
    {
        m_colorBaseIndex += (2 * m_numPkrLog2 - 2) * MaxNumOfBpp;
        m_htileBaseIndex += (m_numPkrLog2 - 1) * 3 * MaxNumOfAA;
    }

    // There is no so-called VAR swizzle mode on GFX11 and instead there are 4 256KB swizzle modes. Here we treat 256KB
    // swizzle mode as "VAR" swizzle mode for reusing exising facilities (e.g GetBlockSizeLog2()) provided by base class
    m_blockVarSizeLog2 = 18;

    if (valid)
    {
        InitEquationTable();
    }

    return valid;
}

/**
************************************************************************************************************************
*   Gfx11Lib::HwlConvertChipFamily
*
*   @brief
*       Convert familyID defined in atiid.h to ChipFamily and set m_chipFamily/m_chipRevision
*   @return
*       ChipFamily
************************************************************************************************************************
*/
ChipFamily Gfx11Lib::HwlConvertChipFamily(
    UINT_32 chipFamily,        ///< [in] chip family defined in atiih.h
    UINT_32 chipRevision)      ///< [in] chip revision defined in "asic_family"_id.h
{
    ChipFamily family = ADDR_CHIP_FAMILY_NAVI;

    switch (chipFamily)
    {
        case FAMILY_NV3:
            if (ASICREV_IS_NAVI31_P(chipRevision))
            {
            }
            if (ASICREV_IS_NAVI32_P(chipRevision))
            {
            }
            if (ASICREV_IS_NAVI33_P(chipRevision))
            {
            }
            break;
        case FAMILY_GFX1150:
            if (ASICREV_IS_GFX1150(chipRevision))
            {
                m_settings.isGfx1150 = 1;
            }
            break;
        case FAMILY_GFX1103:
            m_settings.isGfx1103 = 1;
            break;
        default:
            ADDR_ASSERT(!"Unknown chip family");
            break;
    }

    m_configFlags.use32bppFor422Fmt = TRUE;

    return family;
}

/**
************************************************************************************************************************
*   Gfx11Lib::GetBlk256SizeLog2
*
*   @brief
*       Get block 256 size
*
*   @return
*       N/A
************************************************************************************************************************
*/
void Gfx11Lib::GetBlk256SizeLog2(
    AddrResourceType resourceType,      ///< [in] Resource type
    AddrSwizzleMode  swizzleMode,       ///< [in] Swizzle mode
    UINT_32          elemLog2,          ///< [in] element size log2
    UINT_32          numSamplesLog2,    ///< [in] number of samples
    Dim3d*           pBlock             ///< [out] block size
    ) const
{
    if (IsThin(resourceType, swizzleMode))
    {
        UINT_32 blockBits = 8 - elemLog2;

        // On GFX11, Z and R modes are the same thing.
        if (IsZOrderSwizzle(swizzleMode) || IsRtOptSwizzle(swizzleMode))
        {
            blockBits -= numSamplesLog2;
        }

        pBlock->w = (blockBits >> 1) + (blockBits & 1);
        pBlock->h = (blockBits >> 1);
        pBlock->d = 0;
    }
    else
    {
        ADDR_ASSERT(IsThick(resourceType, swizzleMode));

        UINT_32 blockBits = 8 - elemLog2;

        pBlock->d = (blockBits / 3) + (((blockBits % 3) > 0) ? 1 : 0);
        pBlock->w = (blockBits / 3) + (((blockBits % 3) > 1) ? 1 : 0);
        pBlock->h = (blockBits / 3);
    }
}

/**
************************************************************************************************************************
*   Gfx11Lib::GetCompressedBlockSizeLog2
*
*   @brief
*       Get compress block size
*
*   @return
*       N/A
************************************************************************************************************************
*/
void Gfx11Lib::GetCompressedBlockSizeLog2(
    Gfx11DataType    dataType,          ///< [in] Data type
    AddrResourceType resourceType,      ///< [in] Resource type
    AddrSwizzleMode  swizzleMode,       ///< [in] Swizzle mode
    UINT_32          elemLog2,          ///< [in] element size log2
    UINT_32          numSamplesLog2,    ///< [in] number of samples
    Dim3d*           pBlock             ///< [out] block size
    ) const
{
    if (dataType == Gfx11DataColor)
    {
        GetBlk256SizeLog2(resourceType, swizzleMode, elemLog2, numSamplesLog2, pBlock);
    }
    else
    {
        ADDR_ASSERT(dataType == Gfx11DataDepthStencil);
        pBlock->w = 3;
        pBlock->h = 3;
        pBlock->d = 0;
    }
}

/**
************************************************************************************************************************
*   Gfx11Lib::GetMetaOverlapLog2
*
*   @brief
*       Get meta block overlap
*
*   @return
*       N/A
************************************************************************************************************************
*/
INT_32 Gfx11Lib::GetMetaOverlapLog2(
    Gfx11DataType    dataType,          ///< [in] Data type
    AddrResourceType resourceType,      ///< [in] Resource type
    AddrSwizzleMode  swizzleMode,       ///< [in] Swizzle mode
    UINT_32          elemLog2,          ///< [in] element size log2
    UINT_32          numSamplesLog2     ///< [in] number of samples
    ) const
{
    Dim3d compBlock;
    Dim3d microBlock;

    GetCompressedBlockSizeLog2(dataType, resourceType, swizzleMode, elemLog2, numSamplesLog2, &compBlock);
    GetBlk256SizeLog2(resourceType, swizzleMode, elemLog2, numSamplesLog2, &microBlock);

    const INT_32 compSizeLog2   = compBlock.w  + compBlock.h  + compBlock.d;
    const INT_32 blk256SizeLog2 = microBlock.w + microBlock.h + microBlock.d;
    const INT_32 maxSizeLog2    = Max(compSizeLog2, blk256SizeLog2);
    const INT_32 numPipesLog2   = GetEffectiveNumPipes();
    INT_32       overlap        = numPipesLog2 - maxSizeLog2;

    if (numPipesLog2 > 1)
    {
        overlap++;
    }

    // In 16Bpp 8xaa, we lose 1 overlap bit because the block size reduction eats into a pipe anchor bit (y4)
    if ((elemLog2 == 4) && (numSamplesLog2 == 3))
    {
        overlap--;
    }
    overlap = Max(overlap, 0);
    return overlap;
}

/**
************************************************************************************************************************
*   Gfx11Lib::Get3DMetaOverlapLog2
*
*   @brief
*       Get 3d meta block overlap
*
*   @return
*       N/A
************************************************************************************************************************
*/
INT_32 Gfx11Lib::Get3DMetaOverlapLog2(
    AddrResourceType resourceType,      ///< [in] Resource type
    AddrSwizzleMode  swizzleMode,       ///< [in] Swizzle mode
    UINT_32          elemLog2           ///< [in] element size log2
    ) const
{
    Dim3d microBlock;
    GetBlk256SizeLog2(resourceType, swizzleMode, elemLog2, 0, &microBlock);

    INT_32 overlap = GetEffectiveNumPipes() - static_cast<INT_32>(microBlock.w);

    overlap++;

    if ((overlap < 0) || (IsStandardSwizzle(resourceType, swizzleMode) == TRUE))
    {
        overlap = 0;
    }
    return overlap;
}

/**
************************************************************************************************************************
*   Gfx11Lib::GetPipeRotateAmount
*
*   @brief
*       Get pipe rotate amount
*
*   @return
*       Pipe rotate amount
************************************************************************************************************************
*/

INT_32 Gfx11Lib::GetPipeRotateAmount(
    AddrResourceType resourceType,      ///< [in] Resource type
    AddrSwizzleMode  swizzleMode        ///< [in] Swizzle mode
    ) const
{
    INT_32 amount = 0;

    if ((m_pipesLog2 >= (m_numSaLog2 + 1)) && (m_pipesLog2 > 1))
    {
        amount = ((m_pipesLog2 == (m_numSaLog2 + 1)) && IsRbAligned(resourceType, swizzleMode)) ?
                 1 : m_pipesLog2 - (m_numSaLog2 + 1);
    }

    return amount;
}

/**
************************************************************************************************************************
*   Gfx11Lib::GetMetaBlkSize
*
*   @brief
*       Get metadata block size
*
*   @return
*       Meta block size
************************************************************************************************************************
*/
UINT_32 Gfx11Lib::GetMetaBlkSize(
    Gfx11DataType    dataType,          ///< [in] Data type
    AddrResourceType resourceType,      ///< [in] Resource type
    AddrSwizzleMode  swizzleMode,       ///< [in] Swizzle mode
    UINT_32          elemLog2,          ///< [in] element size log2
    UINT_32          numSamplesLog2,    ///< [in] number of samples
    BOOL_32          pipeAlign,         ///< [in] pipe align
    Dim3d*           pBlock             ///< [out] block size
    ) const
{
    INT_32 metablkSizeLog2;

    const INT_32 metaElemSizeLog2   = GetMetaElementSizeLog2(dataType);
    const INT_32 metaCacheSizeLog2  = GetMetaCacheSizeLog2(dataType);
    const INT_32 compBlkSizeLog2    = (dataType == Gfx11DataColor) ? 8 : 6 + numSamplesLog2 + elemLog2;
    const INT_32 metaBlkSamplesLog2 = numSamplesLog2;
    const INT_32 dataBlkSizeLog2    = GetBlockSizeLog2(swizzleMode);
    INT_32       numPipesLog2       = m_pipesLog2;

    if (IsThin(resourceType, swizzleMode))
    {
        if ((pipeAlign == FALSE) ||
            (IsStandardSwizzle(resourceType, swizzleMode) == TRUE) ||
            (IsDisplaySwizzle(resourceType, swizzleMode)  == TRUE))
        {
            if (pipeAlign)
            {
                metablkSizeLog2 = Max(static_cast<INT_32>(m_pipeInterleaveLog2) + numPipesLog2, 12);
                metablkSizeLog2 = Min(metablkSizeLog2, dataBlkSizeLog2);
            }
            else
            {
                metablkSizeLog2 = Min(dataBlkSizeLog2, 12);
            }
        }
        else
        {
            if ((m_pipesLog2 == m_numSaLog2 + 1) && (m_pipesLog2 > 1))
            {
                numPipesLog2++;
            }

            INT_32 pipeRotateLog2 = GetPipeRotateAmount(resourceType, swizzleMode);

            if (numPipesLog2 >= 4)
            {
                INT_32 overlapLog2 = GetMetaOverlapLog2(dataType, resourceType, swizzleMode, elemLog2, numSamplesLog2);

                // In 16Bpe 8xaa, we have an extra overlap bit
                if ((pipeRotateLog2 > 0)  &&
                    (elemLog2 == 4)       &&
                    (numSamplesLog2 == 3) &&
                    (IsZOrderSwizzle(swizzleMode) || (GetEffectiveNumPipes() > 3)))
                {
                    overlapLog2++;
                }

                metablkSizeLog2 = metaCacheSizeLog2 + overlapLog2 + numPipesLog2;
                metablkSizeLog2 = Max(metablkSizeLog2, static_cast<INT_32>(m_pipeInterleaveLog2) + numPipesLog2);
            }
            else
            {
                metablkSizeLog2 = Max(static_cast<INT_32>(m_pipeInterleaveLog2) + numPipesLog2, 12);
            }

            if (dataType == Gfx11DataDepthStencil)
            {
                // For htile surfaces, pad meta block size to 2K * num_pipes
                metablkSizeLog2 = Max(metablkSizeLog2, 11 + numPipesLog2);
            }

            const INT_32 compFragLog2 = numSamplesLog2;

            if  (IsRtOptSwizzle(swizzleMode) && (compFragLog2 > 1) && (pipeRotateLog2 >= 1))
            {
                const INT_32 tmp = 8 + m_pipesLog2 + Max(pipeRotateLog2, compFragLog2 - 1);

                metablkSizeLog2 = Max(metablkSizeLog2, tmp);
            }
        }

        const INT_32 metablkBitsLog2 =
            metablkSizeLog2 + compBlkSizeLog2 - elemLog2 - metaBlkSamplesLog2 - metaElemSizeLog2;
        pBlock->w = 1 << ((metablkBitsLog2 >> 1) + (metablkBitsLog2 & 1));
        pBlock->h = 1 << (metablkBitsLog2 >> 1);
        pBlock->d = 1;
    }
    else
    {
        ADDR_ASSERT(IsThick(resourceType, swizzleMode));

        if (pipeAlign)
        {
            if ((m_pipesLog2 == m_numSaLog2 + 1) &&
                (m_pipesLog2 > 1)                &&
                IsRbAligned(resourceType, swizzleMode))
            {
                numPipesLog2++;
            }

            const INT_32 overlapLog2 = Get3DMetaOverlapLog2(resourceType, swizzleMode, elemLog2);

            metablkSizeLog2 = metaCacheSizeLog2 + overlapLog2 + numPipesLog2;
            metablkSizeLog2 = Max(metablkSizeLog2, static_cast<INT_32>(m_pipeInterleaveLog2) + numPipesLog2);
            metablkSizeLog2 = Max(metablkSizeLog2, 12);
        }
        else
        {
            metablkSizeLog2 = 12;
        }

        const INT_32 metablkBitsLog2 =
            metablkSizeLog2 + compBlkSizeLog2 - elemLog2 - metaBlkSamplesLog2 - metaElemSizeLog2;
        pBlock->w = 1 << ((metablkBitsLog2 / 3) + (((metablkBitsLog2 % 3) > 0) ? 1 : 0));
        pBlock->h = 1 << ((metablkBitsLog2 / 3) + (((metablkBitsLog2 % 3) > 1) ? 1 : 0));
        pBlock->d = 1 << (metablkBitsLog2 / 3);
    }

    return (1 << static_cast<UINT_32>(metablkSizeLog2));
}

/**
************************************************************************************************************************
*   Gfx11Lib::ConvertSwizzlePatternToEquation
*
*   @brief
*       Convert swizzle pattern to equation.
*
*   @return
*       N/A
************************************************************************************************************************
*/
VOID Gfx11Lib::ConvertSwizzlePatternToEquation(
    UINT_32                elemLog2,  ///< [in] element bytes log2
    AddrResourceType       rsrcType,  ///< [in] resource type
    AddrSwizzleMode        swMode,    ///< [in] swizzle mode
    const ADDR_SW_PATINFO* pPatInfo,  ///< [in] swizzle pattern infor
    ADDR_EQUATION*         pEquation) ///< [out] equation converted from swizzle pattern
    const
{
    ADDR_BIT_SETTING fullSwizzlePattern[ADDR_MAX_EQUATION_BIT];
    GetSwizzlePatternFromPatternInfo(pPatInfo, fullSwizzlePattern);

    const ADDR_BIT_SETTING* pSwizzle      = fullSwizzlePattern;
    const UINT_32           blockSizeLog2 = GetBlockSizeLog2(swMode);
    memset(pEquation, 0, sizeof(ADDR_EQUATION));
    pEquation->numBits            = blockSizeLog2;
    pEquation->numBitComponents   = pPatInfo->maxItemCount;
    pEquation->stackedDepthSlices = FALSE;

    for (UINT_32 i = 0; i < elemLog2; i++)
    {
        pEquation->addr[i].channel = 0;
        pEquation->addr[i].valid   = 1;
        pEquation->addr[i].index   = i;
    }

    if (IsXor(swMode) == FALSE)
    {
        // Use simplified logic when we only have one bit-component
        for (UINT_32 i = elemLog2; i < blockSizeLog2; i++)
        {
            ADDR_ASSERT(IsPow2(pSwizzle[i].value));

            if (pSwizzle[i].x != 0)
            {
                ADDR_ASSERT(IsPow2(static_cast<UINT_32>(pSwizzle[i].x)));

                pEquation->addr[i].channel = 0;
                pEquation->addr[i].valid   = 1;
                pEquation->addr[i].index   = Log2(pSwizzle[i].x) + elemLog2;
            }
            else if (pSwizzle[i].y != 0)
            {
                ADDR_ASSERT(IsPow2(static_cast<UINT_32>(pSwizzle[i].y)));

                pEquation->addr[i].channel = 1;
                pEquation->addr[i].valid   = 1;
                pEquation->addr[i].index   = Log2(pSwizzle[i].y);
            }
            else
            {
                ADDR_ASSERT(pSwizzle[i].z != 0);
                ADDR_ASSERT(IsPow2(static_cast<UINT_32>(pSwizzle[i].z)));

                pEquation->addr[i].channel = 2;
                pEquation->addr[i].valid   = 1;
                pEquation->addr[i].index   = Log2(pSwizzle[i].z);
            }
        }
    }
    else
    {
        Dim3d dim;
        ComputeBlockDimension(&dim.w, &dim.h, &dim.d, 8u << elemLog2, rsrcType, swMode);

        const UINT_32 blkXLog2 = Log2(dim.w);
        const UINT_32 blkYLog2 = Log2(dim.h);
        const UINT_32 blkZLog2 = Log2(dim.d);
        const UINT_32 blkXMask = dim.w - 1;
        const UINT_32 blkYMask = dim.h - 1;
        const UINT_32 blkZMask = dim.d - 1;

        ADDR_BIT_SETTING swizzle[ADDR_MAX_EQUATION_BIT] = {};
        memcpy(&swizzle, pSwizzle, sizeof(swizzle));
        UINT_32          xMask = 0;
        UINT_32          yMask = 0;
        UINT_32          zMask = 0;

        for (UINT_32 i = elemLog2; i < blockSizeLog2; i++)
        {
            for (UINT_32 bitComp = 0; bitComp < ADDR_MAX_EQUATION_COMP; bitComp++)
            {
                if (swizzle[i].value == 0)
                {
                    ADDR_ASSERT(bitComp != 0); // Bits above element size must have at least one addr-bit
                    ADDR_ASSERT(bitComp <= pPatInfo->maxItemCount);
                    break;
                }

                if (swizzle[i].x != 0)
                {
                    const UINT_32 xLog2 = BitScanForward(swizzle[i].x);
                    swizzle[i].x = UnsetLeastBit(swizzle[i].x);
                    xMask |= (1 << xLog2);

                    pEquation->comps[bitComp][i].channel = 0;
                    pEquation->comps[bitComp][i].valid   = 1;
                    pEquation->comps[bitComp][i].index   = xLog2 + elemLog2;
                }
                else if (swizzle[i].y != 0)
                {
                    const UINT_32 yLog2 = BitScanForward(swizzle[i].y);
                    swizzle[i].y = UnsetLeastBit(swizzle[i].y);
                    yMask |= (1 << yLog2);

                    pEquation->comps[bitComp][i].channel = 1;
                    pEquation->comps[bitComp][i].valid   = 1;
                    pEquation->comps[bitComp][i].index   = yLog2;
                }
                else if (swizzle[i].z != 0)
                {
                    const UINT_32 zLog2 = BitScanForward(swizzle[i].z);
                    swizzle[i].z = UnsetLeastBit(swizzle[i].z);
                    zMask |= (1 << zLog2);

                    pEquation->comps[bitComp][i].channel = 2;
                    pEquation->comps[bitComp][i].valid   = 1;
                    pEquation->comps[bitComp][i].index   = zLog2;
                }
                else
                {
                    // This function doesn't handle MSAA (must update block dims, here, and consumers)
                    ADDR_ASSERT_ALWAYS();
                }
            }
            ADDR_ASSERT(swizzle[i].value == 0); // We missed an xor? Are there too many?
        }

        // We missed an address bit for coords inside the block?
        // That means two coords will land on the same addr, which is bad.
        ADDR_ASSERT(((xMask & blkXMask) == blkXMask) &&
                    ((yMask & blkYMask) == blkYMask) &&
                    ((zMask & blkZMask) == blkZMask));
        // We're sourcing from outside our block? That won't fly for PRTs, which need to be movable.
        // Non-xor modes can also be used for 2D PRTs but they're handled in the simplified logic above.
        ADDR_ASSERT((IsPrt(swMode) == false) ||
                    ((xMask == blkXMask) &&
                     (yMask == blkYMask) &&
                     (zMask == blkZMask)));
    }
}

/**
************************************************************************************************************************
*   Gfx11Lib::InitEquationTable
*
*   @brief
*       Initialize Equation table.
*
*   @return
*       N/A
************************************************************************************************************************
*/
VOID Gfx11Lib::InitEquationTable()
{
    memset(m_equationTable, 0, sizeof(m_equationTable));

    for (UINT_32 rsrcTypeIdx = 0; rsrcTypeIdx < MaxRsrcType; rsrcTypeIdx++)
    {
        const AddrResourceType rsrcType = static_cast<AddrResourceType>(rsrcTypeIdx + ADDR_RSRC_TEX_2D);

        for (UINT_32 swModeIdx = 0; swModeIdx < MaxSwModeType; swModeIdx++)
        {
            const AddrSwizzleMode swMode = static_cast<AddrSwizzleMode>(swModeIdx);

            for (UINT_32 elemLog2 = 0; elemLog2 < MaxElementBytesLog2; elemLog2++)
            {
                UINT_32                equationIndex = ADDR_INVALID_EQUATION_INDEX;
                const ADDR_SW_PATINFO* pPatInfo      = GetSwizzlePatternInfo(swMode, rsrcType, elemLog2, 1);

                if (pPatInfo != NULL)
                {
                    ADDR_ASSERT(IsValidSwMode(swMode));
                    ADDR_EQUATION equation = {};

                    ConvertSwizzlePatternToEquation(elemLog2, rsrcType, swMode, pPatInfo, &equation);

                    equationIndex = m_numEquations;
                    ADDR_ASSERT(equationIndex < EquationTableSize);

                    m_equationTable[equationIndex] = equation;

                    m_numEquations++;
                }

                m_equationLookupTable[rsrcTypeIdx][swModeIdx][elemLog2] = equationIndex;
            }
        }
    }
}

/**
************************************************************************************************************************
*   Gfx11Lib::HwlGetEquationIndex
*
*   @brief
*       Interface function stub of GetEquationIndex
*
*   @return
*       ADDR_E_RETURNCODE
************************************************************************************************************************
*/
UINT_32 Gfx11Lib::HwlGetEquationIndex(
    const ADDR2_COMPUTE_SURFACE_INFO_INPUT* pIn,    ///< [in] input structure
    ADDR2_COMPUTE_SURFACE_INFO_OUTPUT*      pOut    ///< [out] output structure
    ) const
{
    UINT_32 equationIdx = ADDR_INVALID_EQUATION_INDEX;

    if ((pIn->resourceType == ADDR_RSRC_TEX_2D) ||
        (pIn->resourceType == ADDR_RSRC_TEX_3D))
    {
        const UINT_32 rsrcTypeIdx = static_cast<UINT_32>(pIn->resourceType) - 1;
        const UINT_32 swModeIdx   = static_cast<UINT_32>(pIn->swizzleMode);
        const UINT_32 elemLog2    = Log2(pIn->bpp >> 3);

        equationIdx = m_equationLookupTable[rsrcTypeIdx][swModeIdx][elemLog2];
    }

    if (pOut->pMipInfo != NULL)
    {
        for (UINT_32 i = 0; i < pIn->numMipLevels; i++)
        {
            pOut->pMipInfo[i].equationIndex = equationIdx;
        }
    }

    return equationIdx;
}

/**
************************************************************************************************************************
*   Gfx11Lib::GetValidDisplaySwizzleModes
*
*   @brief
*       Get valid swizzle modes mask for displayable surface
*
*   @return
*       Valid swizzle modes mask for displayable surface
************************************************************************************************************************
*/
UINT_32 Gfx11Lib::GetValidDisplaySwizzleModes(
    UINT_32 bpp
    ) const
{
    UINT_32 swModeMask = 0;

    if (bpp <= 64)
    {
        const ChipFamily  family = GetChipFamily();

        swModeMask = Dcn32SwModeMask;

        if (false
            || (m_settings.isGfx1103)
            || (m_settings.isGfx1150)
           )
        {
            // Not all GPUs support displaying with 256kB swizzle modes.
            swModeMask &= ~((1u << ADDR_SW_256KB_D_X) |
                            (1u << ADDR_SW_256KB_R_X));
        }
    }

    return swModeMask;
}

/**
************************************************************************************************************************
*   Gfx11Lib::IsValidDisplaySwizzleMode
*
*   @brief
*       Check if a swizzle mode is supported by display engine
*
*   @return
*       TRUE is swizzle mode is supported by display engine
************************************************************************************************************************
*/
BOOL_32 Gfx11Lib::IsValidDisplaySwizzleMode(
    const ADDR2_COMPUTE_SURFACE_INFO_INPUT* pIn     ///< [in] input structure
    ) const
{
    ADDR_ASSERT(pIn->resourceType == ADDR_RSRC_TEX_2D);

    return (GetValidDisplaySwizzleModes(pIn->bpp) & (1 << pIn->swizzleMode)) ? TRUE : FALSE;
}

/**
************************************************************************************************************************
*   Gfx11Lib::GetMaxNumMipsInTail
*
*   @brief
*       Return max number of mips in tails
*
*   @return
*       Max number of mips in tails
************************************************************************************************************************
*/
UINT_32 Gfx11Lib::GetMaxNumMipsInTail(
    UINT_32 blockSizeLog2,     ///< block size log2
    BOOL_32 isThin             ///< is thin or thick
    ) const
{
    UINT_32 effectiveLog2 = blockSizeLog2;

    if (isThin == FALSE)
    {
        effectiveLog2 -= (blockSizeLog2 - 8) / 3;
    }

    return (effectiveLog2 <= 11) ? (1 + (1 << (effectiveLog2 - 9))) : (effectiveLog2 - 4);
}

/**
************************************************************************************************************************
*   Gfx11Lib::HwlComputePipeBankXor
*
*   @brief
*       Generate a PipeBankXor value to be ORed into bits above pipeInterleaveBits of address
*
*   @return
*       PipeBankXor value
************************************************************************************************************************
*/
ADDR_E_RETURNCODE Gfx11Lib::HwlComputePipeBankXor(
    const ADDR2_COMPUTE_PIPEBANKXOR_INPUT* pIn,     ///< [in] input structure
    ADDR2_COMPUTE_PIPEBANKXOR_OUTPUT*      pOut     ///< [out] output structure
    ) const
{
    if (IsNonPrtXor(pIn->swizzleMode))
    {
        pOut->pipeBankXor = 0;
    }
    else
    {
        pOut->pipeBankXor = 0;
    }

    return ADDR_OK;
}

/**
************************************************************************************************************************
*   Gfx11Lib::HwlComputeSlicePipeBankXor
*
*   @brief
*       Generate slice PipeBankXor value based on base PipeBankXor value and slice id
*
*   @return
*       PipeBankXor value
************************************************************************************************************************
*/
ADDR_E_RETURNCODE Gfx11Lib::HwlComputeSlicePipeBankXor(
    const ADDR2_COMPUTE_SLICE_PIPEBANKXOR_INPUT* pIn,   ///< [in] input structure
    ADDR2_COMPUTE_SLICE_PIPEBANKXOR_OUTPUT*      pOut   ///< [out] output structure
    ) const
{
    ADDR_E_RETURNCODE returnCode = ADDR_OK;

    if (IsNonPrtXor(pIn->swizzleMode))
    {
        if (pIn->bpe == 0)
        {
            ADDR_ASSERT_ALWAYS();

            // Require a valid bytes-per-element value passed from client...
            returnCode = ADDR_INVALIDPARAMS;
        }
        else
        {
            const ADDR_SW_PATINFO* pPatInfo = GetSwizzlePatternInfo(pIn->swizzleMode,
                                                                    pIn->resourceType,
                                                                    Log2(pIn->bpe >> 3),
                                                                    1);

            if (pPatInfo != NULL)
            {
                ADDR_BIT_SETTING fullSwizzlePattern[20];
                GetSwizzlePatternFromPatternInfo(pPatInfo, fullSwizzlePattern);

                const UINT_32 pipeBankXorOffset =
                    ComputeOffsetFromSwizzlePattern(reinterpret_cast<const UINT_64*>(fullSwizzlePattern),
                                                    GetBlockSizeLog2(pIn->swizzleMode),
                                                    0,
                                                    0,
                                                    pIn->slice,
                                                    0);

                const UINT_32 pipeBankXor = pipeBankXorOffset >> m_pipeInterleaveLog2;

                // Should have no bit set under pipe interleave
                ADDR_ASSERT((pipeBankXor << m_pipeInterleaveLog2) == pipeBankXorOffset);

                pOut->pipeBankXor = pIn->basePipeBankXor ^ pipeBankXor;
            }
            else
            {
                // Should never come here...
                ADDR_NOT_IMPLEMENTED();

                returnCode = ADDR_NOTSUPPORTED;
            }
        }
    }
    else
    {
        pOut->pipeBankXor = 0;
    }

    return returnCode;
}

/**
************************************************************************************************************************
*   Gfx11Lib::HwlComputeSubResourceOffsetForSwizzlePattern
*
*   @brief
*       Compute sub resource offset to support swizzle pattern
*
*   @return
*       Offset
************************************************************************************************************************
*/
ADDR_E_RETURNCODE Gfx11Lib::HwlComputeSubResourceOffsetForSwizzlePattern(
    const ADDR2_COMPUTE_SUBRESOURCE_OFFSET_FORSWIZZLEPATTERN_INPUT* pIn,    ///< [in] input structure
    ADDR2_COMPUTE_SUBRESOURCE_OFFSET_FORSWIZZLEPATTERN_OUTPUT*      pOut    ///< [out] output structure
    ) const
{
    ADDR_ASSERT(IsThin(pIn->resourceType, pIn->swizzleMode));

    pOut->offset = pIn->slice * pIn->sliceSize + pIn->macroBlockOffset;

    return ADDR_OK;
}

/**
************************************************************************************************************************
*   Gfx11Lib::HwlComputeNonBlockCompressedView
*
*   @brief
*       Compute non-block-compressed view for a given mipmap level/slice.
*
*   @return
*       ADDR_E_RETURNCODE
************************************************************************************************************************
*/
ADDR_E_RETURNCODE Gfx11Lib::HwlComputeNonBlockCompressedView(
    const ADDR2_COMPUTE_NONBLOCKCOMPRESSEDVIEW_INPUT* pIn,    ///< [in] input structure
    ADDR2_COMPUTE_NONBLOCKCOMPRESSEDVIEW_OUTPUT*      pOut    ///< [out] output structure
    ) const
{
    ADDR_E_RETURNCODE returnCode = ADDR_OK;

    if (IsThin(pIn->resourceType, pIn->swizzleMode) == FALSE)
    {
        // Only thin swizzle mode can have a NonBC view...
        returnCode = ADDR_INVALIDPARAMS;
    }
    else if (((pIn->format < ADDR_FMT_ASTC_4x4) || (pIn->format > ADDR_FMT_ETC2_128BPP)) &&
             ((pIn->format < ADDR_FMT_BC1) || (pIn->format > ADDR_FMT_BC7)))
    {
        // Only support BC1~BC7, ASTC, or ETC2 for now...
        returnCode = ADDR_NOTSUPPORTED;
    }
    else
    {
        UINT_32 bcWidth, bcHeight;
        UINT_32 bpp = GetElemLib()->GetBitsPerPixel(pIn->format, NULL, &bcWidth, &bcHeight);

        ADDR2_COMPUTE_SURFACE_INFO_INPUT infoIn = {};
        infoIn.flags        = pIn->flags;
        infoIn.swizzleMode  = pIn->swizzleMode;
        infoIn.resourceType = pIn->resourceType;
        infoIn.bpp          = bpp;
        infoIn.width        = RoundUpQuotient(pIn->width, bcWidth);
        infoIn.height       = RoundUpQuotient(pIn->height, bcHeight);
        infoIn.numSlices    = pIn->numSlices;
        infoIn.numMipLevels = pIn->numMipLevels;
        infoIn.numSamples   = 1;
        infoIn.numFrags     = 1;

        ADDR2_MIP_INFO mipInfo[MaxMipLevels] = {};

        ADDR2_COMPUTE_SURFACE_INFO_OUTPUT infoOut = {};
        infoOut.pMipInfo = mipInfo;

        const BOOL_32 tiled = (pIn->swizzleMode != ADDR_SW_LINEAR) ? TRUE : FALSE;

        if (tiled)
        {
            returnCode = HwlComputeSurfaceInfoTiled(&infoIn, &infoOut);
        }
        else
        {
            returnCode = HwlComputeSurfaceInfoLinear(&infoIn, &infoOut);
        }

        if (returnCode == ADDR_OK)
        {
            ADDR2_COMPUTE_SUBRESOURCE_OFFSET_FORSWIZZLEPATTERN_INPUT subOffIn = {};
            subOffIn.swizzleMode      = infoIn.swizzleMode;
            subOffIn.resourceType     = infoIn.resourceType;
            subOffIn.slice            = pIn->slice;
            subOffIn.sliceSize        = infoOut.sliceSize;
            subOffIn.macroBlockOffset = mipInfo[pIn->mipId].macroBlockOffset;
            subOffIn.mipTailOffset    = mipInfo[pIn->mipId].mipTailOffset;

            ADDR2_COMPUTE_SUBRESOURCE_OFFSET_FORSWIZZLEPATTERN_OUTPUT subOffOut = {};

            // For any mipmap level, move nonBc view base address by offset
            HwlComputeSubResourceOffsetForSwizzlePattern(&subOffIn, &subOffOut);
            pOut->offset = subOffOut.offset;

            ADDR2_COMPUTE_SLICE_PIPEBANKXOR_INPUT slicePbXorIn = {};
            slicePbXorIn.bpe             = infoIn.bpp;
            slicePbXorIn.swizzleMode     = infoIn.swizzleMode;
            slicePbXorIn.resourceType    = infoIn.resourceType;
            slicePbXorIn.basePipeBankXor = pIn->pipeBankXor;
            slicePbXorIn.slice           = pIn->slice;

            ADDR2_COMPUTE_SLICE_PIPEBANKXOR_OUTPUT slicePbXorOut = {};

            // For any mipmap level, nonBc view should use computed pbXor
            HwlComputeSlicePipeBankXor(&slicePbXorIn, &slicePbXorOut);
            pOut->pipeBankXor = slicePbXorOut.pipeBankXor;

            const BOOL_32 inTail           = tiled && (pIn->mipId >= infoOut.firstMipIdInTail) ? TRUE : FALSE;
            const UINT_32 requestMipWidth  = RoundUpQuotient(Max(pIn->width >> pIn->mipId, 1u), bcWidth);
            const UINT_32 requestMipHeight = RoundUpQuotient(Max(pIn->height >> pIn->mipId, 1u), bcHeight);

            if (inTail)
            {
                // For mipmap level that is in mip tail block, hack a lot of things...
                // Basically all mipmap levels in tail block will be viewed as a small mipmap chain that all levels
                // are fit in tail block:

                // - mipId = relative mip id (which is counted from first mip ID in tail in original mip chain)
                pOut->mipId = pIn->mipId - infoOut.firstMipIdInTail;

                // - at least 2 mipmap levels (since only 1 mipmap level will not be viewed as mipmap!)
                pOut->numMipLevels = Max(infoIn.numMipLevels - infoOut.firstMipIdInTail, 2u);

                // - (mip0) width = requestMipWidth << mipId, the value can't exceed mip tail dimension threshold
                pOut->unalignedWidth = Min(requestMipWidth << pOut->mipId, infoOut.blockWidth / 2);

                // - (mip0) height = requestMipHeight << mipId, the value can't exceed mip tail dimension threshold
                pOut->unalignedHeight = Min(requestMipHeight << pOut->mipId, infoOut.blockHeight);
            }
            // This check should cover at least mipId == 0
            else if (requestMipWidth << pIn->mipId == infoIn.width)
            {
                // For mipmap level [N] that is not in mip tail block and downgraded without losing element:
                // - only one mipmap level and mipId = 0
                pOut->mipId        = 0;
                pOut->numMipLevels = 1;

                // (mip0) width = requestMipWidth
                pOut->unalignedWidth = requestMipWidth;

                // (mip0) height = requestMipHeight
                pOut->unalignedHeight = requestMipHeight;
            }
            else
            {
                // For mipmap level [N] that is not in mip tail block and downgraded with element losing,
                // We have to make it a multiple mipmap view (2 levels view here), add one extra element if needed,
                // because single mip view may have different pitch value than original (multiple) mip view...
                // A simple case would be:
                // - 64KB block swizzle mode, 8 Bytes-Per-Element. Block dim = [0x80, 0x40]
                // - 2 mipmap levels with API mip0 width = 0x401/mip1 width = 0x200 and non-BC view
                //   mip0 width = 0x101/mip1 width = 0x80
                // By multiple mip view, the pitch for mip level 1 would be 0x100 bytes, due to rounding up logic in
                // GetMipSize(), and by single mip level view the pitch will only be 0x80 bytes.

                // - 2 levels and mipId = 1
                pOut->mipId        = 1;
                pOut->numMipLevels = 2;

                const UINT_32 upperMipWidth  = RoundUpQuotient(Max(pIn->width >> (pIn->mipId - 1), 1u), bcWidth);
                const UINT_32 upperMipHeight = RoundUpQuotient(Max(pIn->height >> (pIn->mipId - 1), 1u), bcHeight);

                const BOOL_32 needToAvoidInTail =
                    tiled && (requestMipWidth <= infoOut.blockWidth / 2) && (requestMipHeight <= infoOut.blockHeight) ?
                    TRUE : FALSE;

                const UINT_32 hwMipWidth  = PowTwoAlign(ShiftCeil(infoIn.width, pIn->mipId), infoOut.blockWidth);
                const UINT_32 hwMipHeight = PowTwoAlign(ShiftCeil(infoIn.height, pIn->mipId), infoOut.blockHeight);

                const BOOL_32 needExtraWidth =
                    ((upperMipWidth < requestMipWidth * 2) ||
                     ((upperMipWidth == requestMipWidth * 2) &&
                      ((needToAvoidInTail == TRUE) ||
                       (hwMipWidth > PowTwoAlign(requestMipWidth, infoOut.blockWidth))))) ? TRUE : FALSE;

                const BOOL_32 needExtraHeight =
                    ((upperMipHeight < requestMipHeight * 2) ||
                     ((upperMipHeight == requestMipHeight * 2) &&
                      ((needToAvoidInTail == TRUE) ||
                       (hwMipHeight > PowTwoAlign(requestMipHeight, infoOut.blockHeight))))) ? TRUE : FALSE;

                // (mip0) width = requestLastMipLevelWidth
                pOut->unalignedWidth  = upperMipWidth + (needExtraWidth ? 1: 0);

                // (mip0) height = requestLastMipLevelHeight
                pOut->unalignedHeight = upperMipHeight + (needExtraHeight ? 1: 0);
            }

            // Assert the downgrading from this mip[0] width would still generate correct mip[N] width
            ADDR_ASSERT(ShiftRight(pOut->unalignedWidth, pOut->mipId) == requestMipWidth);
            // Assert the downgrading from this mip[0] height would still generate correct mip[N] height
            ADDR_ASSERT(ShiftRight(pOut->unalignedHeight, pOut->mipId) == requestMipHeight);
        }
    }

    return returnCode;
}

/**
************************************************************************************************************************
*   Gfx11Lib::ValidateNonSwModeParams
*
*   @brief
*       Validate compute surface info params except swizzle mode
*
*   @return
*       TRUE if parameters are valid, FALSE otherwise
************************************************************************************************************************
*/
BOOL_32 Gfx11Lib::ValidateNonSwModeParams(
    const ADDR2_COMPUTE_SURFACE_INFO_INPUT* pIn) const
{
    BOOL_32 valid = TRUE;

    if ((pIn->bpp == 0) || (pIn->bpp > 128) || (pIn->width == 0) || (pIn->numFrags > 8))
    {
        ADDR_ASSERT_ALWAYS();
        valid = FALSE;
    }
    else if (pIn->flags.fmask == 1)
    {
        // There is no FMASK for GFX11 ASICs
        ADDR_ASSERT_ALWAYS();
        valid = FALSE;
    }
    else if (pIn->numSamples > 8)
    {
        // There is no EQAA support for GFX11 ASICs, so the max number of sample is 8
        ADDR_ASSERT_ALWAYS();
        valid = FALSE;
    }
    else if ((pIn->numFrags != 0) && (pIn->numSamples != pIn->numFrags))
    {
        // There is no EQAA support for GFX11 ASICs, so the number of sample has to be same as number of fragment
        ADDR_ASSERT_ALWAYS();
        valid = FALSE;
    }

    const ADDR2_SURFACE_FLAGS flags    = pIn->flags;
    const AddrResourceType    rsrcType = pIn->resourceType;
    const BOOL_32             mipmap   = (pIn->numMipLevels > 1);
    const BOOL_32             msaa     = (pIn->numSamples > 1);
    const BOOL_32             display  = flags.display;
    const BOOL_32             tex3d    = IsTex3d(rsrcType);
    const BOOL_32             tex2d    = IsTex2d(rsrcType);
    const BOOL_32             tex1d    = IsTex1d(rsrcType);
    const BOOL_32             stereo   = flags.qbStereo;

    // Resource type check
    if (tex1d)
    {
        if (msaa || display || stereo)
        {
            ADDR_ASSERT_ALWAYS();
            valid = FALSE;
        }
    }
    else if (tex2d)
    {
        if ((msaa && mipmap) || (stereo && msaa) || (stereo && mipmap))
        {
            ADDR_ASSERT_ALWAYS();
            valid = FALSE;
        }
    }
    else if (tex3d)
    {
        if (msaa || display || stereo)
        {
            ADDR_ASSERT_ALWAYS();
            valid = FALSE;
        }
    }
    else
    {
        ADDR_ASSERT_ALWAYS();
        valid = FALSE;
    }

    return valid;
}

/**
************************************************************************************************************************
*   Gfx11Lib::ValidateSwModeParams
*
*   @brief
*       Validate compute surface info related to swizzle mode
*
*   @return
*       TRUE if parameters are valid, FALSE otherwise
************************************************************************************************************************
*/
BOOL_32 Gfx11Lib::ValidateSwModeParams(
    const ADDR2_COMPUTE_SURFACE_INFO_INPUT* pIn) const
{
    BOOL_32 valid = TRUE;

    if (pIn->swizzleMode >= ADDR_SW_MAX_TYPE)
    {
        ADDR_ASSERT_ALWAYS();
        valid = FALSE;
    }
    else if (IsValidSwMode(pIn->swizzleMode) == FALSE)
    {
        ADDR_ASSERT_ALWAYS();
        valid = FALSE;
    }

    const ADDR2_SURFACE_FLAGS flags       = pIn->flags;
    const AddrResourceType    rsrcType    = pIn->resourceType;
    const AddrSwizzleMode     swizzle     = pIn->swizzleMode;
    const BOOL_32             msaa        = (pIn->numSamples > 1);
    const BOOL_32             zbuffer     = flags.depth || flags.stencil;
    const BOOL_32             color       = flags.color;
    const BOOL_32             display     = flags.display;
    const BOOL_32             tex3d       = IsTex3d(rsrcType);
    const BOOL_32             tex2d       = IsTex2d(rsrcType);
    const BOOL_32             tex1d       = IsTex1d(rsrcType);
    const BOOL_32             thin3d      = flags.view3dAs2dArray;
    const BOOL_32             linear      = IsLinear(swizzle);
    const BOOL_32             blk256B     = IsBlock256b(swizzle);
    const BOOL_32             isNonPrtXor = IsNonPrtXor(swizzle);
    const BOOL_32             prt         = flags.prt;

    // Misc check
    if (msaa && (GetBlockSize(swizzle) < (m_pipeInterleaveBytes * pIn->numSamples)))
    {
        // MSAA surface must have blk_bytes/pipe_interleave >= num_samples
        ADDR_ASSERT_ALWAYS();
        valid = FALSE;
    }

    if (display && (IsValidDisplaySwizzleMode(pIn) == FALSE))
    {
        ADDR_ASSERT_ALWAYS();
        valid = FALSE;
    }

    if ((pIn->bpp == 96) && (linear == FALSE))
    {
        ADDR_ASSERT_ALWAYS();
        valid = FALSE;
    }

    const UINT_32 swizzleMask = 1 << swizzle;

    // Resource type check
    if (tex1d)
    {
        if ((swizzleMask & Gfx11Rsrc1dSwModeMask) == 0)
        {
            ADDR_ASSERT_ALWAYS();
            valid = FALSE;
        }
    }
    else if (tex2d)
    {
        if ((swizzleMask & Gfx11Rsrc2dSwModeMask) == 0)
        {
            ADDR_ASSERT_ALWAYS();
            valid = FALSE;
        }
        else if (prt && ((swizzleMask & Gfx11Rsrc2dPrtSwModeMask) == 0))
        {
            ADDR_ASSERT_ALWAYS();
            valid = FALSE;
        }
    }
    else if (tex3d)
    {
        if (((swizzleMask & Gfx11Rsrc3dSwModeMask) == 0) ||
            (prt && ((swizzleMask & Gfx11Rsrc3dPrtSwModeMask) == 0)) ||
            (thin3d && ((swizzleMask & Gfx11Rsrc3dThinSwModeMask) == 0)))
        {
            ADDR_ASSERT_ALWAYS();
            valid = FALSE;
        }
    }

    // Swizzle type check
    if (linear)
    {
        if (zbuffer || msaa || (pIn->bpp == 0) || ((pIn->bpp % 8) != 0))
        {
            ADDR_ASSERT_ALWAYS();
            valid = FALSE;
        }
    }
    else if (IsZOrderSwizzle(swizzle))
    {
        if ((pIn->bpp > 64)                         ||
            (msaa && (color || (pIn->bpp > 32)))    ||
            ElemLib::IsBlockCompressed(pIn->format) ||
            ElemLib::IsMacroPixelPacked(pIn->format))
        {
            ADDR_ASSERT_ALWAYS();
            valid = FALSE;
        }
    }
    else if (IsStandardSwizzle(rsrcType, swizzle))
    {
        if (zbuffer || msaa)
        {
            ADDR_ASSERT_ALWAYS();
            valid = FALSE;
        }
    }
    else if (IsDisplaySwizzle(rsrcType, swizzle))
    {
        if (zbuffer || msaa)
        {
            ADDR_ASSERT_ALWAYS();
            valid = FALSE;
        }
    }
    else if (IsRtOptSwizzle(swizzle))
    {
        if (zbuffer)
        {
            ADDR_ASSERT_ALWAYS();
            valid = FALSE;
        }
    }
    else
    {
        ADDR_ASSERT_ALWAYS();
        valid = FALSE;
    }

    // Block type check
    if (blk256B)
    {
        if (zbuffer || tex3d || msaa)
        {
            ADDR_ASSERT_ALWAYS();
            valid = FALSE;
        }
    }

    return valid;
}

/**
************************************************************************************************************************
*   Gfx11Lib::HwlComputeSurfaceInfoSanityCheck
*
*   @brief
*       Compute surface info sanity check
*
*   @return
*       Offset
************************************************************************************************************************
*/
ADDR_E_RETURNCODE Gfx11Lib::HwlComputeSurfaceInfoSanityCheck(
    const ADDR2_COMPUTE_SURFACE_INFO_INPUT* pIn     ///< [in] input structure
    ) const
{
    return ValidateNonSwModeParams(pIn) && ValidateSwModeParams(pIn) ? ADDR_OK : ADDR_INVALIDPARAMS;
}

/**
************************************************************************************************************************
*   Gfx11Lib::HwlGetPreferredSurfaceSetting
*
*   @brief
*       Internal function to get suggested surface information for cliet to use
*
*   @return
*       ADDR_E_RETURNCODE
************************************************************************************************************************
*/
ADDR_E_RETURNCODE Gfx11Lib::HwlGetPreferredSurfaceSetting(
    const ADDR2_GET_PREFERRED_SURF_SETTING_INPUT* pIn,  ///< [in] input structure
    ADDR2_GET_PREFERRED_SURF_SETTING_OUTPUT*      pOut  ///< [out] output structure
    ) const
{
    ADDR_E_RETURNCODE returnCode = ADDR_OK;

    if (pIn->flags.fmask)
    {
        // There is no FMASK for GFX11 ASICs.
        ADDR_ASSERT_ALWAYS();

        returnCode = ADDR_INVALIDPARAMS;
    }
    else
    {
        UINT_32 bpp    = pIn->bpp;
        UINT_32 width  = Max(pIn->width, 1u);
        UINT_32 height = Max(pIn->height, 1u);

        // Set format to INVALID will skip this conversion
        if (pIn->format != ADDR_FMT_INVALID)
        {
            ElemMode elemMode = ADDR_UNCOMPRESSED;
            UINT_32 expandX, expandY;

            // Get compression/expansion factors and element mode which indicates compression/expansion
            bpp = GetElemLib()->GetBitsPerPixel(pIn->format,
                                                &elemMode,
                                                &expandX,
                                                &expandY);

            UINT_32 basePitch = 0;
            GetElemLib()->AdjustSurfaceInfo(elemMode,
                                            expandX,
                                            expandY,
                                            &bpp,
                                            &basePitch,
                                            &width,
                                            &height);
        }

        const UINT_32 numSlices    = Max(pIn->numSlices,    1u);
        const UINT_32 numMipLevels = Max(pIn->numMipLevels, 1u);
        const UINT_32 numSamples   = Max(pIn->numSamples,   1u);
        const BOOL_32 msaa         = numSamples > 1;

        // Pre sanity check on non swizzle mode parameters
        ADDR2_COMPUTE_SURFACE_INFO_INPUT localIn = {};
        localIn.flags        = pIn->flags;
        localIn.resourceType = pIn->resourceType;
        localIn.format       = pIn->format;
        localIn.bpp          = bpp;
        localIn.width        = width;
        localIn.height       = height;
        localIn.numSlices    = numSlices;
        localIn.numMipLevels = numMipLevels;
        localIn.numSamples   = numSamples;
        localIn.numFrags     = numSamples;

        if (ValidateNonSwModeParams(&localIn))
        {
            // Forbid swizzle mode(s) by client setting
            ADDR2_SWMODE_SET allowedSwModeSet = {};
            allowedSwModeSet.value |= pIn->forbiddenBlock.linear ? 0 : Gfx11LinearSwModeMask;
            allowedSwModeSet.value |= pIn->forbiddenBlock.micro  ? 0 : Gfx11Blk256BSwModeMask;
            allowedSwModeSet.value |=
                pIn->forbiddenBlock.macroThin4KB ? 0 :
                ((pIn->resourceType == ADDR_RSRC_TEX_3D) ? 0 : Gfx11Blk4KBSwModeMask);
            allowedSwModeSet.value |=
                pIn->forbiddenBlock.macroThick4KB ? 0 :
                ((pIn->resourceType == ADDR_RSRC_TEX_3D) ? Gfx11Rsrc3dThick4KBSwModeMask : 0);
            allowedSwModeSet.value |=
                pIn->forbiddenBlock.macroThin64KB ? 0 :
                ((pIn->resourceType == ADDR_RSRC_TEX_3D) ? Gfx11Rsrc3dThin64KBSwModeMask : Gfx11Blk64KBSwModeMask);
            allowedSwModeSet.value |=
                pIn->forbiddenBlock.macroThick64KB ? 0 :
                ((pIn->resourceType == ADDR_RSRC_TEX_3D) ? Gfx11Rsrc3dThick64KBSwModeMask : 0);
            allowedSwModeSet.value |=
                pIn->forbiddenBlock.gfx11.thin256KB ? 0 :
                ((pIn->resourceType == ADDR_RSRC_TEX_3D) ? Gfx11Rsrc3dThin256KBSwModeMask : Gfx11Blk256KBSwModeMask);
            allowedSwModeSet.value |=
                pIn->forbiddenBlock.gfx11.thick256KB ? 0 :
                ((pIn->resourceType == ADDR_RSRC_TEX_3D) ? Gfx11Rsrc3dThick256KBSwModeMask : 0);

            if (pIn->preferredSwSet.value != 0)
            {
                allowedSwModeSet.value &= pIn->preferredSwSet.sw_Z ? ~0 : ~Gfx11ZSwModeMask;
                allowedSwModeSet.value &= pIn->preferredSwSet.sw_S ? ~0 : ~Gfx11StandardSwModeMask;
                allowedSwModeSet.value &= pIn->preferredSwSet.sw_D ? ~0 : ~Gfx11DisplaySwModeMask;
                allowedSwModeSet.value &= pIn->preferredSwSet.sw_R ? ~0 : ~Gfx11RenderSwModeMask;
            }

            if (pIn->noXor)
            {
                allowedSwModeSet.value &= ~Gfx11XorSwModeMask;
            }

            if (pIn->maxAlign > 0)
            {
                if (pIn->maxAlign < Size256K)
                {
                    allowedSwModeSet.value &= ~Gfx11Blk256KBSwModeMask;
                }

                if (pIn->maxAlign < Size64K)
                {
                    allowedSwModeSet.value &= ~Gfx11Blk64KBSwModeMask;
                }

                if (pIn->maxAlign < Size4K)
                {
                    allowedSwModeSet.value &= ~Gfx11Blk4KBSwModeMask;
                }

                if (pIn->maxAlign < Size256)
                {
                    allowedSwModeSet.value &= ~Gfx11Blk256BSwModeMask;
                }
            }

            // Filter out invalid swizzle mode(s) by image attributes and HW restrictions
            switch (pIn->resourceType)
            {
                case ADDR_RSRC_TEX_1D:
                    allowedSwModeSet.value &= Gfx11Rsrc1dSwModeMask;
                    break;

                case ADDR_RSRC_TEX_2D:
                    allowedSwModeSet.value &= pIn->flags.prt ? Gfx11Rsrc2dPrtSwModeMask : Gfx11Rsrc2dSwModeMask;
                    break;

                case ADDR_RSRC_TEX_3D:
                    allowedSwModeSet.value &= pIn->flags.prt ? Gfx11Rsrc3dPrtSwModeMask : Gfx11Rsrc3dSwModeMask;

                    if (pIn->flags.view3dAs2dArray)
                    {
                        allowedSwModeSet.value &= Gfx11Rsrc3dThinSwModeMask;
                    }
                    break;

                default:
                    ADDR_ASSERT_ALWAYS();
                    allowedSwModeSet.value = 0;
                    break;
            }

            if (ElemLib::IsBlockCompressed(pIn->format)  ||
                ElemLib::IsMacroPixelPacked(pIn->format) ||
                (bpp > 64)                               ||
                (msaa && ((bpp > 32) || pIn->flags.color || pIn->flags.unordered)))
            {
                allowedSwModeSet.value &= ~Gfx11ZSwModeMask;
            }

            if (pIn->format == ADDR_FMT_32_32_32)
            {
                allowedSwModeSet.value &= Gfx11LinearSwModeMask;
            }

            if (msaa)
            {
                allowedSwModeSet.value &= Gfx11MsaaSwModeMask;
            }

            if (pIn->flags.depth || pIn->flags.stencil)
            {
                allowedSwModeSet.value &= Gfx11ZSwModeMask;
            }

            if (pIn->flags.display)
            {
                allowedSwModeSet.value &= GetValidDisplaySwizzleModes(bpp);
            }

            if (allowedSwModeSet.value != 0)
            {
#if DEBUG
                // Post sanity check, at least AddrLib should accept the output generated by its own
                UINT_32 validateSwModeSet = allowedSwModeSet.value;

                for (UINT_32 i = 0; validateSwModeSet != 0; i++)
                {
                    if (validateSwModeSet & 1)
                    {
                        localIn.swizzleMode = static_cast<AddrSwizzleMode>(i);
                        ADDR_ASSERT(ValidateSwModeParams(&localIn));
                    }

                    validateSwModeSet >>= 1;
                }
#endif

                pOut->resourceType   = pIn->resourceType;
                pOut->validSwModeSet = allowedSwModeSet;
                pOut->canXor         = (allowedSwModeSet.value & Gfx11XorSwModeMask) ? TRUE : FALSE;

                GetAllowedBlockSet(allowedSwModeSet, pOut->resourceType, &(pOut->validBlockSet));
                GetAllowedSwSet(allowedSwModeSet, &(pOut->validSwTypeSet));

                pOut->clientPreferredSwSet = pIn->preferredSwSet;

                if (pOut->clientPreferredSwSet.value == 0)
                {
                    pOut->clientPreferredSwSet.value = AddrSwSetAll;
                }

                // Apply optional restrictions
                if (pIn->flags.needEquation)
                {
                    UINT_32 components = pIn->flags.allowExtEquation ?  ADDR_MAX_EQUATION_COMP :
                                                                        ADDR_MAX_LEGACY_EQUATION_COMP;
                    FilterInvalidEqSwizzleMode(allowedSwModeSet, pIn->resourceType, Log2(bpp >> 3), components);
                }

                if (allowedSwModeSet.value == Gfx11LinearSwModeMask)
                {
                    pOut->swizzleMode = ADDR_SW_LINEAR;
                }
                else
                {
                    const BOOL_32 computeMinSize = (pIn->flags.minimizeAlign == 1) || (pIn->memoryBudget >= 1.0);

                    if ((height > 1) && (computeMinSize == FALSE))
                    {
                        // Always ignore linear swizzle mode if:
                        // 1. This is a (2D/3D) resource with height > 1
                        // 2. Client doesn't require computing minimize size
                        allowedSwModeSet.swLinear = 0;
                    }

                    ADDR2_BLOCK_SET allowedBlockSet = {};
                    GetAllowedBlockSet(allowedSwModeSet, pOut->resourceType, &allowedBlockSet);

                    // Determine block size if there are 2 or more block type candidates
                    if (IsPow2(allowedBlockSet.value) == FALSE)
                    {
                        AddrSwizzleMode swMode[AddrBlockMaxTiledType] = {};

                        swMode[AddrBlockLinear] = ADDR_SW_LINEAR;

                        if (pOut->resourceType == ADDR_RSRC_TEX_3D)
                        {
                            swMode[AddrBlockThick4KB]   = ADDR_SW_4KB_S_X;
                            swMode[AddrBlockThin64KB]   = ADDR_SW_64KB_R_X;
                            swMode[AddrBlockThick64KB]  = ADDR_SW_64KB_S_X;
                            swMode[AddrBlockThin256KB]  = ADDR_SW_256KB_R_X;
                            swMode[AddrBlockThick256KB] = ADDR_SW_256KB_S_X;
                        }
                        else
                        {
                            swMode[AddrBlockMicro]     = ADDR_SW_256B_D;
                            swMode[AddrBlockThin4KB]   = ADDR_SW_4KB_D_X;
                            swMode[AddrBlockThin64KB]  = ADDR_SW_64KB_D_X;
                            swMode[AddrBlockThin256KB] = ADDR_SW_256KB_D_X;
                        }

                        UINT_64 padSize[AddrBlockMaxTiledType] = {};

                        const UINT_32 ratioLow           = computeMinSize ? 1 : (pIn->flags.opt4space ? 3 : 2);
                        const UINT_32 ratioHi            = computeMinSize ? 1 : (pIn->flags.opt4space ? 2 : 1);
                        const UINT_64 sizeAlignInElement = Max(NextPow2(pIn->minSizeAlign) / (bpp >> 3), 1u);
                        UINT_32       minSizeBlk         = AddrBlockMicro;
                        UINT_64       minSize            = 0;

                        ADDR2_COMPUTE_SURFACE_INFO_OUTPUT localOut = {};

                        for (UINT_32 i = AddrBlockLinear; i < AddrBlockMaxTiledType; i++)
                        {
                            if (Addr2IsBlockTypeAvailable(allowedBlockSet, static_cast<::AddrBlockType>(i)))
                            {
                                localIn.swizzleMode = swMode[i];

                                if (localIn.swizzleMode == ADDR_SW_LINEAR)
                                {
                                    returnCode = HwlComputeSurfaceInfoLinear(&localIn, &localOut);
                                }
                                else
                                {
                                    returnCode = HwlComputeSurfaceInfoTiled(&localIn, &localOut);
                                }

                                if (returnCode == ADDR_OK)
                                {
                                    padSize[i] = localOut.surfSize;

                                    if ((minSize == 0) ||
                                        Addr2BlockTypeWithinMemoryBudget(minSize, padSize[i], ratioLow, ratioHi))
                                    {
                                        minSize    = padSize[i];
                                        minSizeBlk = i;
                                    }
                                }
                                else
                                {
                                    ADDR_ASSERT_ALWAYS();
                                    break;
                                }
                            }
                        }

                        if (pIn->memoryBudget > 1.0)
                        {
                            // If minimum size is given by swizzle mode with bigger-block type, then don't ever check
                            // smaller-block type again in coming loop
                            switch (minSizeBlk)
                            {
                                case AddrBlockThick256KB:
                                    allowedBlockSet.gfx11.thin256KB = 0;
                                case AddrBlockThin256KB:
                                    allowedBlockSet.macroThick64KB = 0;
                                case AddrBlockThick64KB:
                                    allowedBlockSet.macroThin64KB = 0;
                                case AddrBlockThin64KB:
                                    allowedBlockSet.macroThick4KB = 0;
                                case AddrBlockThick4KB:
                                    allowedBlockSet.macroThin4KB = 0;
                                case AddrBlockThin4KB:
                                    allowedBlockSet.micro  = 0;
                                case AddrBlockMicro:
                                    allowedBlockSet.linear = 0;
                                case AddrBlockLinear:
                                    break;

                                default:
                                    ADDR_ASSERT_ALWAYS();
                                    break;
                            }

                            for (UINT_32 i = AddrBlockMicro; i < AddrBlockMaxTiledType; i++)
                            {
                                if ((i != minSizeBlk) &&
                                    Addr2IsBlockTypeAvailable(allowedBlockSet, static_cast<::AddrBlockType>(i)))
                                {
                                    if (Addr2BlockTypeWithinMemoryBudget(minSize, padSize[i], 0, 0, pIn->memoryBudget) == FALSE)
                                    {
                                        // Clear the block type if the memory waste is unacceptable
                                        allowedBlockSet.value &= ~(1u << (i - 1));
                                    }
                                }
                            }

                            // Remove linear block type if 2 or more block types are allowed
                            if (IsPow2(allowedBlockSet.value) == FALSE)
                            {
                                allowedBlockSet.linear = 0;
                            }

                            // Select the biggest allowed block type
                            minSizeBlk = Log2NonPow2(allowedBlockSet.value) + 1;

                            if (minSizeBlk == static_cast<UINT_32>(AddrBlockMaxTiledType))
                            {
                                minSizeBlk = AddrBlockLinear;
                            }
                        }

                        switch (minSizeBlk)
                        {
                            case AddrBlockLinear:
                                allowedSwModeSet.value &= Gfx11LinearSwModeMask;
                                break;

                            case AddrBlockMicro:
                                ADDR_ASSERT(pOut->resourceType != ADDR_RSRC_TEX_3D);
                                allowedSwModeSet.value &= Gfx11Blk256BSwModeMask;
                                break;

                            case AddrBlockThin4KB:
                                ADDR_ASSERT(pOut->resourceType != ADDR_RSRC_TEX_3D);
                                allowedSwModeSet.value &= Gfx11Blk4KBSwModeMask;
                                break;

                            case AddrBlockThick4KB:
                                ADDR_ASSERT(pOut->resourceType == ADDR_RSRC_TEX_3D);
                                allowedSwModeSet.value &= Gfx11Rsrc3dThick4KBSwModeMask;
                                break;

                            case AddrBlockThin64KB:
                                allowedSwModeSet.value &= (pOut->resourceType == ADDR_RSRC_TEX_3D) ?
                                                          Gfx11Rsrc3dThin64KBSwModeMask : Gfx11Blk64KBSwModeMask;
                                break;

                            case AddrBlockThick64KB:
                                ADDR_ASSERT(pOut->resourceType == ADDR_RSRC_TEX_3D);
                                allowedSwModeSet.value &= Gfx11Rsrc3dThick64KBSwModeMask;
                                break;

                            case AddrBlockThin256KB:
                                allowedSwModeSet.value &= (pOut->resourceType == ADDR_RSRC_TEX_3D) ?
                                                          Gfx11Rsrc3dThin256KBSwModeMask : Gfx11Blk256KBSwModeMask;
                                break;

                            case AddrBlockThick256KB:
                                ADDR_ASSERT(pOut->resourceType == ADDR_RSRC_TEX_3D);
                                allowedSwModeSet.value &= Gfx11Rsrc3dThick256KBSwModeMask;
                                break;

                            default:
                                ADDR_ASSERT_ALWAYS();
                                allowedSwModeSet.value = 0;
                                break;
                        }
                    }

                    // Block type should be determined.
                    GetAllowedBlockSet(allowedSwModeSet, pOut->resourceType, &allowedBlockSet);
                    ADDR_ASSERT(IsPow2(allowedBlockSet.value));

                    ADDR2_SWTYPE_SET allowedSwSet = {};
                    GetAllowedSwSet(allowedSwModeSet, &allowedSwSet);

                    // Determine swizzle type if there are 2 or more swizzle type candidates
                    if ((allowedSwSet.value != 0) && (IsPow2(allowedSwSet.value) == FALSE))
                    {
                        if (ElemLib::IsBlockCompressed(pIn->format))
                        {
                            if (allowedSwSet.sw_D)
                            {
                                allowedSwModeSet.value &= Gfx11DisplaySwModeMask;
                            }
                            else if (allowedSwSet.sw_S)
                            {
                                allowedSwModeSet.value &= Gfx11StandardSwModeMask;
                            }
                            else
                            {
                                ADDR_ASSERT(allowedSwSet.sw_R);
                                allowedSwModeSet.value &= Gfx11RenderSwModeMask;
                            }
                        }
                        else if (ElemLib::IsMacroPixelPacked(pIn->format))
                        {
                            if (allowedSwSet.sw_S)
                            {
                                allowedSwModeSet.value &= Gfx11StandardSwModeMask;
                            }
                            else if (allowedSwSet.sw_D)
                            {
                                allowedSwModeSet.value &= Gfx11DisplaySwModeMask;
                            }
                            else
                            {
                                ADDR_ASSERT(allowedSwSet.sw_R);
                                allowedSwModeSet.value &= Gfx11RenderSwModeMask;
                            }
                        }
                        else if (pIn->resourceType == ADDR_RSRC_TEX_3D)
                        {
                            if (pIn->flags.color && allowedSwSet.sw_R)
                            {
                                allowedSwModeSet.value &= Gfx11RenderSwModeMask;
                            }
                            else if (allowedSwSet.sw_S)
                            {
                                allowedSwModeSet.value &= Gfx11StandardSwModeMask;
                            }
                            else if (allowedSwSet.sw_D)
                            {
                                allowedSwModeSet.value &= Gfx11DisplaySwModeMask;
                            }
                            else
                            {
                                ADDR_ASSERT(allowedSwSet.sw_Z);
                                allowedSwModeSet.value &= Gfx11ZSwModeMask;
                            }
                        }
                        else
                        {
                            if (allowedSwSet.sw_R)
                            {
                                allowedSwModeSet.value &= Gfx11RenderSwModeMask;
                            }
                            else if (allowedSwSet.sw_D)
                            {
                                allowedSwModeSet.value &= Gfx11DisplaySwModeMask;
                            }
                            else if (allowedSwSet.sw_Z)
                            {
                                allowedSwModeSet.value &= Gfx11ZSwModeMask;
                            }
                            else
                            {
                                ADDR_ASSERT_ALWAYS();
                            }
                        }

                        // Swizzle type should be determined.
                        GetAllowedSwSet(allowedSwModeSet, &allowedSwSet);
                        ADDR_ASSERT(IsPow2(allowedSwSet.value));
                    }

                    // Determine swizzle mode now. Always select the "largest" swizzle mode for a given block type +
                    // swizzle type combination. E.g, for AddrBlockThin64KB + ADDR_SW_S, select SW_64KB_S_X(25) if it's
                    // available, or otherwise select SW_64KB_S_T(17) if it's available, or otherwise select SW_64KB_S(9).
                    pOut->swizzleMode = static_cast<AddrSwizzleMode>(Log2NonPow2(allowedSwModeSet.value));
                }
            }
            else
            {
                // Invalid combination...
                ADDR_ASSERT_ALWAYS();
                returnCode = ADDR_INVALIDPARAMS;
            }
        }
        else
        {
            // Invalid combination...
            ADDR_ASSERT_ALWAYS();
            returnCode = ADDR_INVALIDPARAMS;
        }
    }

    return returnCode;
}

/**
************************************************************************************************************************
*   Gfx11Lib::HwlGetPossibleSwizzleModes
*
*   @brief
*       Returns a list of swizzle modes that are valid from the hardware's perspective for the client to choose from
*
*   @return
*       ADDR_E_RETURNCODE
************************************************************************************************************************
*/
ADDR_E_RETURNCODE Gfx11Lib::HwlGetPossibleSwizzleModes(
    const ADDR2_GET_PREFERRED_SURF_SETTING_INPUT* pIn,  ///< [in] input structure
    ADDR2_GET_PREFERRED_SURF_SETTING_OUTPUT*      pOut  ///< [out] output structure
    ) const
{
    ADDR_E_RETURNCODE returnCode = ADDR_OK;

    if (pIn->flags.fmask)
    {
        // There is no FMASK for GFX11 ASICs.
        ADDR_ASSERT_ALWAYS();

        returnCode = ADDR_INVALIDPARAMS;
    }
    else
    {
        UINT_32 bpp    = pIn->bpp;
        UINT_32 width  = Max(pIn->width, 1u);
        UINT_32 height = Max(pIn->height, 1u);

        // Set format to INVALID will skip this conversion
        if (pIn->format != ADDR_FMT_INVALID)
        {
            ElemMode elemMode = ADDR_UNCOMPRESSED;
            UINT_32 expandX, expandY;

            // Get compression/expansion factors and element mode which indicates compression/expansion
            bpp = GetElemLib()->GetBitsPerPixel(pIn->format,
                &elemMode,
                &expandX,
                &expandY);

            UINT_32 basePitch = 0;
            GetElemLib()->AdjustSurfaceInfo(elemMode,
                expandX,
                expandY,
                &bpp,
                &basePitch,
                &width,
                &height);
        }

        const UINT_32 numSlices    = Max(pIn->numSlices, 1u);
        const UINT_32 numMipLevels = Max(pIn->numMipLevels, 1u);
        const UINT_32 numSamples   = Max(pIn->numSamples, 1u);
        const BOOL_32 msaa         = numSamples > 1;

        // Pre sanity check on non swizzle mode parameters
        ADDR2_COMPUTE_SURFACE_INFO_INPUT localIn = {};
        localIn.flags = pIn->flags;
        localIn.resourceType = pIn->resourceType;
        localIn.format = pIn->format;
        localIn.bpp = bpp;
        localIn.width = width;
        localIn.height = height;
        localIn.numSlices = numSlices;
        localIn.numMipLevels = numMipLevels;
        localIn.numSamples = numSamples;
        localIn.numFrags = numSamples;

        if (ValidateNonSwModeParams(&localIn))
        {
            // Allow appropriate swizzle modes by default
            ADDR2_SWMODE_SET allowedSwModeSet = {};
            allowedSwModeSet.value |= Gfx11LinearSwModeMask | Gfx11Blk256BSwModeMask;
            if (pIn->resourceType == ADDR_RSRC_TEX_3D)
            {
                allowedSwModeSet.value |= Gfx11Rsrc3dThick4KBSwModeMask  |
                                          Gfx11Rsrc3dThin64KBSwModeMask  |
                                          Gfx11Rsrc3dThick64KBSwModeMask |
                                          Gfx11Rsrc3dThin256KBSwModeMask |
                                          Gfx11Rsrc3dThick256KBSwModeMask;
            }
            else
            {
                allowedSwModeSet.value |= Gfx11Blk4KBSwModeMask | Gfx11Blk64KBSwModeMask | Gfx11Blk256KBSwModeMask;
            }

            // Filter out invalid swizzle mode(s) by image attributes and HW restrictions
            switch (pIn->resourceType)
            {
            case ADDR_RSRC_TEX_1D:
                allowedSwModeSet.value &= Gfx11Rsrc1dSwModeMask;
                break;

            case ADDR_RSRC_TEX_2D:
                allowedSwModeSet.value &= pIn->flags.prt ? Gfx11Rsrc2dPrtSwModeMask : Gfx11Rsrc2dSwModeMask;
                break;

            case ADDR_RSRC_TEX_3D:
                allowedSwModeSet.value &= pIn->flags.prt ? Gfx11Rsrc3dPrtSwModeMask : Gfx11Rsrc3dSwModeMask;

                if (pIn->flags.view3dAs2dArray)
                {
                    allowedSwModeSet.value &= Gfx11Rsrc3dThinSwModeMask;
                }
                break;

            default:
                ADDR_ASSERT_ALWAYS();
                allowedSwModeSet.value = 0;
                break;
            }

            // TODO: figure out if following restrictions are correct on GFX11...
            if (ElemLib::IsBlockCompressed(pIn->format) ||
                ElemLib::IsMacroPixelPacked(pIn->format) ||
                (bpp > 64) ||
                (msaa && ((bpp > 32) || pIn->flags.color || pIn->flags.unordered)))
            {
                allowedSwModeSet.value &= ~Gfx11ZSwModeMask;
            }

            if (pIn->format == ADDR_FMT_32_32_32)
            {
                allowedSwModeSet.value &= Gfx11LinearSwModeMask;
            }

            if (msaa)
            {
                allowedSwModeSet.value &= Gfx11MsaaSwModeMask;
            }

            if (pIn->flags.depth || pIn->flags.stencil)
            {
                allowedSwModeSet.value &= Gfx11ZSwModeMask;
            }

            if (pIn->flags.display)
            {
                allowedSwModeSet.value &= GetValidDisplaySwizzleModes(bpp);
            }

            if (allowedSwModeSet.value != 0)
            {
#if DEBUG
                // Post sanity check, at least AddrLib should accept the output generated by its own
                UINT_32 validateSwModeSet = allowedSwModeSet.value;

                for (UINT_32 i = 0; validateSwModeSet != 0; i++)
                {
                    if (validateSwModeSet & 1)
                    {
                        localIn.swizzleMode = static_cast<AddrSwizzleMode>(i);
                        ADDR_ASSERT(ValidateSwModeParams(&localIn));
                    }

                    validateSwModeSet >>= 1;
                }
#endif

                pOut->resourceType = pIn->resourceType;
                pOut->clientPreferredSwSet = pIn->preferredSwSet;

                if (pOut->clientPreferredSwSet.value == 0)
                {
                    pOut->clientPreferredSwSet.value = AddrSwSetAll;
                }

                if (pIn->flags.needEquation)
                {
                    UINT_32 components = pIn->flags.allowExtEquation ?  ADDR_MAX_EQUATION_COMP :
                                                                        ADDR_MAX_LEGACY_EQUATION_COMP;
                    FilterInvalidEqSwizzleMode(allowedSwModeSet, pIn->resourceType, Log2(bpp >> 3), components);
                }

                pOut->validSwModeSet = allowedSwModeSet;
                pOut->canXor = (allowedSwModeSet.value & Gfx11XorSwModeMask) ? TRUE : FALSE;
            }
            else
            {
                // Invalid combination...
                ADDR_ASSERT_ALWAYS();
                returnCode = ADDR_INVALIDPARAMS;
            }
        }
        else
        {
            // Invalid combination...
            ADDR_ASSERT_ALWAYS();
            returnCode = ADDR_INVALIDPARAMS;
        }
    }

    return returnCode;
}

/**
************************************************************************************************************************
*   Gfx11Lib::HwlGetAllowedBlockSet
*
*   @brief
*       Returns the set of allowed block sizes given the allowed swizzle modes and resource type
*
*   @return
*       ADDR_E_RETURNCODE
************************************************************************************************************************
*/
ADDR_E_RETURNCODE Gfx11Lib::HwlGetAllowedBlockSet(
    ADDR2_SWMODE_SET allowedSwModeSet,  ///< [in] allowed swizzle modes
    AddrResourceType rsrcType,          ///< [in] resource type
    ADDR2_BLOCK_SET* pAllowedBlockSet   ///< [out] allowed block sizes
    ) const
{
    ADDR2_BLOCK_SET allowedBlockSet = {};

    allowedBlockSet.micro  = (allowedSwModeSet.value & Gfx11Blk256BSwModeMask) ? TRUE : FALSE;
    allowedBlockSet.linear = (allowedSwModeSet.value & Gfx11LinearSwModeMask)  ? TRUE : FALSE;

    if (rsrcType == ADDR_RSRC_TEX_3D)
    {
        allowedBlockSet.macroThick4KB    = (allowedSwModeSet.value & Gfx11Rsrc3dThick4KBSwModeMask)   ? TRUE : FALSE;
        allowedBlockSet.macroThin64KB    = (allowedSwModeSet.value & Gfx11Rsrc3dThin64KBSwModeMask)   ? TRUE : FALSE;
        allowedBlockSet.macroThick64KB   = (allowedSwModeSet.value & Gfx11Rsrc3dThick64KBSwModeMask)  ? TRUE : FALSE;
        allowedBlockSet.gfx11.thin256KB  = (allowedSwModeSet.value & Gfx11Rsrc3dThin256KBSwModeMask)  ? TRUE : FALSE;
        allowedBlockSet.gfx11.thick256KB = (allowedSwModeSet.value & Gfx11Rsrc3dThick256KBSwModeMask) ? TRUE : FALSE;
    }
    else
    {
        allowedBlockSet.macroThin4KB    = (allowedSwModeSet.value & Gfx11Blk4KBSwModeMask)   ? TRUE : FALSE;
        allowedBlockSet.macroThin64KB   = (allowedSwModeSet.value & Gfx11Blk64KBSwModeMask)  ? TRUE : FALSE;
        allowedBlockSet.gfx11.thin256KB = (allowedSwModeSet.value & Gfx11Blk256KBSwModeMask) ? TRUE : FALSE;
    }

    *pAllowedBlockSet = allowedBlockSet;
    return ADDR_OK;
}

/**
************************************************************************************************************************
*   Gfx11Lib::HwlGetAllowedSwSet
*
*   @brief
*       Returns the set of allowed swizzle types given the allowed swizzle modes
*   @return
*       ADDR_E_RETURNCODE
************************************************************************************************************************
*/
ADDR_E_RETURNCODE Gfx11Lib::HwlGetAllowedSwSet(
    ADDR2_SWMODE_SET  allowedSwModeSet, ///< [in] allowed swizzle modes
    ADDR2_SWTYPE_SET* pAllowedSwSet     ///< [out] allowed swizzle types
    ) const
{
    ADDR2_SWTYPE_SET allowedSwSet = {};

    allowedSwSet.sw_Z = (allowedSwModeSet.value & Gfx11ZSwModeMask)        ? TRUE : FALSE;
    allowedSwSet.sw_S = (allowedSwModeSet.value & Gfx11StandardSwModeMask) ? TRUE : FALSE;
    allowedSwSet.sw_D = (allowedSwModeSet.value & Gfx11DisplaySwModeMask)  ? TRUE : FALSE;
    allowedSwSet.sw_R = (allowedSwModeSet.value & Gfx11RenderSwModeMask)   ? TRUE : FALSE;

    *pAllowedSwSet = allowedSwSet;
    return ADDR_OK;
}

/**
************************************************************************************************************************
*   Gfx11Lib::ComputeStereoInfo
*
*   @brief
*       Compute height alignment and right eye pipeBankXor for stereo surface
*
*   @return
*       Error code
*
************************************************************************************************************************
*/
ADDR_E_RETURNCODE Gfx11Lib::ComputeStereoInfo(
    const ADDR2_COMPUTE_SURFACE_INFO_INPUT* pIn,        ///< Compute surface info
    UINT_32*                                pAlignY,    ///< Stereo requested additional alignment in Y
    UINT_32*                                pRightXor   ///< Right eye xor
    ) const
{
    ADDR_E_RETURNCODE ret = ADDR_OK;

    *pRightXor = 0;

    if (IsNonPrtXor(pIn->swizzleMode))
    {
        const UINT_32 blkSizeLog2 = GetBlockSizeLog2(pIn->swizzleMode);
        const UINT_32 elemLog2    = Log2(pIn->bpp >> 3);
        const UINT_32 rsrcType    = static_cast<UINT_32>(pIn->resourceType) - 1;
        const UINT_32 swMode      = static_cast<UINT_32>(pIn->swizzleMode);
        const UINT_32 eqIndex     = m_equationLookupTable[rsrcType][swMode][elemLog2];

        if (eqIndex != ADDR_INVALID_EQUATION_INDEX)
        {
            UINT_32 yMax     = 0;
            UINT_32 yPosMask = 0;

            // First get "max y bit"
            for (UINT_32 i = m_pipeInterleaveLog2; i < blkSizeLog2; i++)
            {
                ADDR_ASSERT(m_equationTable[eqIndex].addr[i].valid == 1);

                if ((m_equationTable[eqIndex].addr[i].channel == 1) &&
                    (m_equationTable[eqIndex].addr[i].index > yMax))
                {
                    yMax = m_equationTable[eqIndex].addr[i].index;
                }

                if ((m_equationTable[eqIndex].xor1[i].valid == 1) &&
                    (m_equationTable[eqIndex].xor1[i].channel == 1) &&
                    (m_equationTable[eqIndex].xor1[i].index > yMax))
                {
                    yMax = m_equationTable[eqIndex].xor1[i].index;
                }

                if ((m_equationTable[eqIndex].xor2[i].valid == 1) &&
                    (m_equationTable[eqIndex].xor2[i].channel == 1) &&
                    (m_equationTable[eqIndex].xor2[i].index > yMax))
                {
                    yMax = m_equationTable[eqIndex].xor2[i].index;
                }
            }

            // Then loop again for populating a position mask of "max Y bit"
            for (UINT_32 i = m_pipeInterleaveLog2; i < blkSizeLog2; i++)
            {
                if ((m_equationTable[eqIndex].addr[i].channel == 1) &&
                    (m_equationTable[eqIndex].addr[i].index == yMax))
                {
                    yPosMask |= 1u << i;
                }
                else if ((m_equationTable[eqIndex].xor1[i].valid == 1) &&
                         (m_equationTable[eqIndex].xor1[i].channel == 1) &&
                         (m_equationTable[eqIndex].xor1[i].index == yMax))
                {
                    yPosMask |= 1u << i;
                }
                else if ((m_equationTable[eqIndex].xor2[i].valid == 1) &&
                         (m_equationTable[eqIndex].xor2[i].channel == 1) &&
                         (m_equationTable[eqIndex].xor2[i].index == yMax))
                {
                    yPosMask |= 1u << i;
                }
            }

            const UINT_32 additionalAlign = 1 << yMax;

            if (additionalAlign >= *pAlignY)
            {
                *pAlignY = additionalAlign;

                const UINT_32 alignedHeight = PowTwoAlign(pIn->height, additionalAlign);

                if ((alignedHeight >> yMax) & 1)
                {
                    *pRightXor = yPosMask >> m_pipeInterleaveLog2;
                }
            }
        }
        else
        {
            ret = ADDR_INVALIDPARAMS;
        }
    }

    return ret;
}

/**
************************************************************************************************************************
*   Gfx11Lib::HwlComputeSurfaceInfoTiled
*
*   @brief
*       Internal function to calculate alignment for tiled surface
*
*   @return
*       ADDR_E_RETURNCODE
************************************************************************************************************************
*/
ADDR_E_RETURNCODE Gfx11Lib::HwlComputeSurfaceInfoTiled(
     const ADDR2_COMPUTE_SURFACE_INFO_INPUT* pIn,    ///< [in] input structure
     ADDR2_COMPUTE_SURFACE_INFO_OUTPUT*      pOut    ///< [out] output structure
     ) const
{
    ADDR_E_RETURNCODE ret;

    // Mip chain dimesion and epitch has no meaning in GFX11, set to default value
    pOut->mipChainPitch    = 0;
    pOut->mipChainHeight   = 0;
    pOut->mipChainSlice    = 0;
    pOut->epitchIsHeight   = FALSE;

    // Following information will be provided in ComputeSurfaceInfoMacroTiled() if necessary
    pOut->mipChainInTail   = FALSE;
    pOut->firstMipIdInTail = pIn->numMipLevels;

    if (IsBlock256b(pIn->swizzleMode))
    {
        ret = ComputeSurfaceInfoMicroTiled(pIn, pOut);
    }
    else
    {
        ret = ComputeSurfaceInfoMacroTiled(pIn, pOut);
    }

    return ret;
}

/**
************************************************************************************************************************
*   Gfx11Lib::ComputeSurfaceInfoMicroTiled
*
*   @brief
*       Internal function to calculate alignment for micro tiled surface
*
*   @return
*       ADDR_E_RETURNCODE
************************************************************************************************************************
*/
ADDR_E_RETURNCODE Gfx11Lib::ComputeSurfaceInfoMicroTiled(
     const ADDR2_COMPUTE_SURFACE_INFO_INPUT* pIn,    ///< [in] input structure
     ADDR2_COMPUTE_SURFACE_INFO_OUTPUT*      pOut    ///< [out] output structure
     ) const
{
    ADDR_E_RETURNCODE ret = ComputeBlockDimensionForSurf(&pOut->blockWidth,
                                                         &pOut->blockHeight,
                                                         &pOut->blockSlices,
                                                         pIn->bpp,
                                                         pIn->numSamples,
                                                         pIn->resourceType,
                                                         pIn->swizzleMode);

    if (ret == ADDR_OK)
    {
        const UINT_32 blockSize = GetBlockSize(pIn->swizzleMode);

        pOut->pitch     = PowTwoAlign(pIn->width,  pOut->blockWidth);
        pOut->height    = PowTwoAlign(pIn->height, pOut->blockHeight);
        pOut->numSlices = pIn->numSlices;
        pOut->baseAlign = blockSize;

        if (pIn->numMipLevels > 1)
        {
            const UINT_32 mip0Width    = pIn->width;
            const UINT_32 mip0Height   = pIn->height;
            UINT_64       mipSliceSize = 0;

            for (INT_32 i = static_cast<INT_32>(pIn->numMipLevels) - 1; i >= 0; i--)
            {
                UINT_32 mipWidth, mipHeight;

                GetMipSize(mip0Width, mip0Height, 1, i, &mipWidth, &mipHeight);

                const UINT_32 mipActualWidth  = PowTwoAlign(mipWidth,  pOut->blockWidth);
                const UINT_32 mipActualHeight = PowTwoAlign(mipHeight, pOut->blockHeight);

                if (pOut->pMipInfo != NULL)
                {
                    pOut->pMipInfo[i].pitch            = mipActualWidth;
                    pOut->pMipInfo[i].height           = mipActualHeight;
                    pOut->pMipInfo[i].depth            = 1;
                    pOut->pMipInfo[i].offset           = mipSliceSize;
                    pOut->pMipInfo[i].mipTailOffset    = 0;
                    pOut->pMipInfo[i].macroBlockOffset = mipSliceSize;
                }

                mipSliceSize += mipActualWidth * mipActualHeight * (pIn->bpp >> 3);
            }

            pOut->sliceSize = mipSliceSize;
            pOut->surfSize  = mipSliceSize * pOut->numSlices;
        }
        else
        {
            pOut->sliceSize = static_cast<UINT_64>(pOut->pitch) * pOut->height * (pIn->bpp >> 3);
            pOut->surfSize  = pOut->sliceSize * pOut->numSlices;

            if (pOut->pMipInfo != NULL)
            {
                pOut->pMipInfo[0].pitch            = pOut->pitch;
                pOut->pMipInfo[0].height           = pOut->height;
                pOut->pMipInfo[0].depth            = 1;
                pOut->pMipInfo[0].offset           = 0;
                pOut->pMipInfo[0].mipTailOffset    = 0;
                pOut->pMipInfo[0].macroBlockOffset = 0;
            }
        }

    }

    return ret;
}

/**
************************************************************************************************************************
*   Gfx11Lib::ComputeSurfaceInfoMacroTiled
*
*   @brief
*       Internal function to calculate alignment for macro tiled surface
*
*   @return
*       ADDR_E_RETURNCODE
************************************************************************************************************************
*/
ADDR_E_RETURNCODE Gfx11Lib::ComputeSurfaceInfoMacroTiled(
     const ADDR2_COMPUTE_SURFACE_INFO_INPUT* pIn,    ///< [in] input structure
     ADDR2_COMPUTE_SURFACE_INFO_OUTPUT*      pOut    ///< [out] output structure
     ) const
{
    ADDR_E_RETURNCODE returnCode = ComputeBlockDimensionForSurf(&pOut->blockWidth,
                                                                &pOut->blockHeight,
                                                                &pOut->blockSlices,
                                                                pIn->bpp,
                                                                pIn->numSamples,
                                                                pIn->resourceType,
                                                                pIn->swizzleMode);

    if (returnCode == ADDR_OK)
    {
        UINT_32 heightAlign = pOut->blockHeight;

        if (pIn->flags.qbStereo)
        {
            UINT_32 rightXor = 0;

            returnCode = ComputeStereoInfo(pIn, &heightAlign, &rightXor);

            if (returnCode == ADDR_OK)
            {
                pOut->pStereoInfo->rightSwizzle = rightXor;
            }
        }

        if (returnCode == ADDR_OK)
        {
            const UINT_32 blockSizeLog2 = GetBlockSizeLog2(pIn->swizzleMode);
            const UINT_32 blockSize     = 1 << blockSizeLog2;

            pOut->pitch     = PowTwoAlign(pIn->width,     pOut->blockWidth);
            pOut->height    = PowTwoAlign(pIn->height,    heightAlign);
            pOut->numSlices = PowTwoAlign(pIn->numSlices, pOut->blockSlices);
            pOut->baseAlign = blockSize;

            if (pIn->numMipLevels > 1)
            {
                const Dim3d  tailMaxDim         = GetMipTailDim(pIn->resourceType,
                                                                pIn->swizzleMode,
                                                                pOut->blockWidth,
                                                                pOut->blockHeight,
                                                                pOut->blockSlices);
                const UINT_32 mip0Width         = pIn->width;
                const UINT_32 mip0Height        = pIn->height;
                const BOOL_32 isThin            = IsThin(pIn->resourceType, pIn->swizzleMode);
                const UINT_32 mip0Depth         = isThin ? 1 : pIn->numSlices;
                const UINT_32 maxMipsInTail     = GetMaxNumMipsInTail(blockSizeLog2, isThin);
                const UINT_32 index             = Log2(pIn->bpp >> 3);
                UINT_32       firstMipInTail    = pIn->numMipLevels;
                UINT_64       mipChainSliceSize = 0;
                UINT_64       mipSize[MaxMipLevels];
                UINT_64       mipSliceSize[MaxMipLevels];

                // For htile, we need to make z16 and stencil enter the mip tail at the same time as z32 would
                Dim3d fixedTailMaxDim = tailMaxDim;
                if (IsZOrderSwizzle(pIn->swizzleMode) && (index <= 1))
                {
                    fixedTailMaxDim.w /= Block256_2d[index].w / Block256_2d[2].w;
                    fixedTailMaxDim.h /= Block256_2d[index].w / Block256_2d[2].w;
                }

                for (UINT_32 i = 0; i < pIn->numMipLevels; i++)
                {
                    UINT_32 mipWidth, mipHeight, mipDepth;

                    GetMipSize(mip0Width, mip0Height, mip0Depth, i, &mipWidth, &mipHeight, &mipDepth);

                    if (IsInMipTail(fixedTailMaxDim, maxMipsInTail, mipWidth, mipHeight, pIn->numMipLevels - i))
                    {
                        firstMipInTail     = i;
                        mipChainSliceSize += blockSize / pOut->blockSlices;
                        break;
                    }
                    else
                    {
                        const UINT_32 pitch     = PowTwoAlign(mipWidth,  pOut->blockWidth);
                        const UINT_32 height    = PowTwoAlign(mipHeight, pOut->blockHeight);
                        const UINT_32 depth     = PowTwoAlign(mipDepth,  pOut->blockSlices);
                        const UINT_64 sliceSize = static_cast<UINT_64>(pitch) * height * (pIn->bpp >> 3);

                        mipSize[i]         = sliceSize * depth;
                        mipSliceSize[i]    = sliceSize * pOut->blockSlices;
                        mipChainSliceSize += sliceSize;

                        if (pOut->pMipInfo != NULL)
                        {
                            pOut->pMipInfo[i].pitch  = pitch;
                            pOut->pMipInfo[i].height = height;
                            pOut->pMipInfo[i].depth  = depth;
                        }
                    }
                }

                pOut->sliceSize        = mipChainSliceSize;
                pOut->surfSize         = mipChainSliceSize * pOut->numSlices;
                pOut->mipChainInTail   = (firstMipInTail == 0) ? TRUE : FALSE;
                pOut->firstMipIdInTail = firstMipInTail;

                if (pOut->pMipInfo != NULL)
                {
                    UINT_64 offset         = 0;
                    UINT_64 macroBlkOffset = 0;
                    UINT_32 tailMaxDepth   = 0;

                    if (firstMipInTail != pIn->numMipLevels)
                    {
                        UINT_32 mipWidth, mipHeight;

                        GetMipSize(mip0Width, mip0Height, mip0Depth, firstMipInTail,
                                   &mipWidth, &mipHeight, &tailMaxDepth);

                        offset         = blockSize * PowTwoAlign(tailMaxDepth, pOut->blockSlices) / pOut->blockSlices;
                        macroBlkOffset = blockSize;
                    }

                    for (INT_32 i = firstMipInTail - 1; i >= 0; i--)
                    {
                        pOut->pMipInfo[i].offset           = offset;
                        pOut->pMipInfo[i].macroBlockOffset = macroBlkOffset;
                        pOut->pMipInfo[i].mipTailOffset    = 0;

                        offset         += mipSize[i];
                        macroBlkOffset += mipSliceSize[i];
                    }

                    UINT_32 pitch  = tailMaxDim.w;
                    UINT_32 height = tailMaxDim.h;
                    UINT_32 depth  = isThin ? 1 : PowTwoAlign(tailMaxDepth, Block256_3d[index].d);

                    tailMaxDepth = isThin ? 1 : (depth / Block256_3d[index].d);

                    for (UINT_32 i = firstMipInTail; i < pIn->numMipLevels; i++)
                    {
                        const UINT_32 m         = maxMipsInTail - 1 - (i - firstMipInTail);
                        const UINT_32 mipOffset = (m > 6) ? (16 << m) : (m << 8);

                        pOut->pMipInfo[i].offset           = mipOffset * tailMaxDepth;
                        pOut->pMipInfo[i].mipTailOffset    = mipOffset;
                        pOut->pMipInfo[i].macroBlockOffset = 0;

                        pOut->pMipInfo[i].pitch  = pitch;
                        pOut->pMipInfo[i].height = height;
                        pOut->pMipInfo[i].depth  = depth;

                        UINT_32 mipX = ((mipOffset >> 9)  & 1)  |
                                       ((mipOffset >> 10) & 2)  |
                                       ((mipOffset >> 11) & 4)  |
                                       ((mipOffset >> 12) & 8)  |
                                       ((mipOffset >> 13) & 16) |
                                       ((mipOffset >> 14) & 32);
                        UINT_32 mipY = ((mipOffset >> 8)  & 1)  |
                                       ((mipOffset >> 9)  & 2)  |
                                       ((mipOffset >> 10) & 4)  |
                                       ((mipOffset >> 11) & 8)  |
                                       ((mipOffset >> 12) & 16) |
                                       ((mipOffset >> 13) & 32);

                        if (blockSizeLog2 & 1)
                        {
                            const UINT_32 temp = mipX;
                            mipX = mipY;
                            mipY = temp;

                            if (index & 1)
                            {
                                mipY = (mipY << 1) | (mipX & 1);
                                mipX = mipX >> 1;
                            }
                        }

                        if (isThin)
                        {
                            pOut->pMipInfo[i].mipTailCoordX = mipX * Block256_2d[index].w;
                            pOut->pMipInfo[i].mipTailCoordY = mipY * Block256_2d[index].h;
                            pOut->pMipInfo[i].mipTailCoordZ = 0;

                            pitch  = Max(pitch  >> 1, Block256_2d[index].w);
                            height = Max(height >> 1, Block256_2d[index].h);
                            depth  = 1;
                        }
                        else
                        {
                            pOut->pMipInfo[i].mipTailCoordX = mipX * Block256_3d[index].w;
                            pOut->pMipInfo[i].mipTailCoordY = mipY * Block256_3d[index].h;
                            pOut->pMipInfo[i].mipTailCoordZ = 0;

                            pitch  = Max(pitch  >> 1, Block256_3d[index].w);
                            height = Max(height >> 1, Block256_3d[index].h);
                            depth  = PowTwoAlign(Max(depth  >> 1, 1u), Block256_3d[index].d);
                        }
                    }
                }
            }
            else
            {
                pOut->sliceSize = static_cast<UINT_64>(pOut->pitch) * pOut->height * (pIn->bpp >> 3) * pIn->numSamples;
                pOut->surfSize  = pOut->sliceSize * pOut->numSlices;

                if (pOut->pMipInfo != NULL)
                {
                    pOut->pMipInfo[0].pitch            = pOut->pitch;
                    pOut->pMipInfo[0].height           = pOut->height;
                    pOut->pMipInfo[0].depth            = IsTex3d(pIn->resourceType)? pOut->numSlices : 1;
                    pOut->pMipInfo[0].offset           = 0;
                    pOut->pMipInfo[0].mipTailOffset    = 0;
                    pOut->pMipInfo[0].macroBlockOffset = 0;
                    pOut->pMipInfo[0].mipTailCoordX    = 0;
                    pOut->pMipInfo[0].mipTailCoordY    = 0;
                    pOut->pMipInfo[0].mipTailCoordZ    = 0;
                }
            }
        }
    }

    return returnCode;
}

/**
************************************************************************************************************************
*   Gfx11Lib::HwlComputeSurfaceAddrFromCoordTiled
*
*   @brief
*       Internal function to calculate address from coord for tiled swizzle surface
*
*   @return
*       ADDR_E_RETURNCODE
************************************************************************************************************************
*/
ADDR_E_RETURNCODE Gfx11Lib::HwlComputeSurfaceAddrFromCoordTiled(
     const ADDR2_COMPUTE_SURFACE_ADDRFROMCOORD_INPUT* pIn,    ///< [in] input structure
     ADDR2_COMPUTE_SURFACE_ADDRFROMCOORD_OUTPUT*      pOut    ///< [out] output structure
     ) const
{
    ADDR_E_RETURNCODE ret;

    if (IsBlock256b(pIn->swizzleMode))
    {
        ret = ComputeSurfaceAddrFromCoordMicroTiled(pIn, pOut);
    }
    else
    {
        ret = ComputeSurfaceAddrFromCoordMacroTiled(pIn, pOut);
    }

    return ret;
}

/**
************************************************************************************************************************
*   Gfx11Lib::ComputeOffsetFromEquation
*
*   @brief
*       Compute offset from equation
*
*   @return
*       Offset
************************************************************************************************************************
*/
UINT_32 Gfx11Lib::ComputeOffsetFromEquation(
    const ADDR_EQUATION* pEq,   ///< Equation
    UINT_32              x,     ///< x coord in bytes
    UINT_32              y,     ///< y coord in pixel
    UINT_32              z      ///< z coord in slice
    ) const
{
    UINT_32 offset = 0;

    for (UINT_32 i = 0; i < pEq->numBits; i++)
    {
        UINT_32 v = 0;

        for (UINT_32 c = 0; c < pEq->numBitComponents; c++)
        {
            if (pEq->comps[c][i].valid)
            {
                if (pEq->comps[c][i].channel == 0)
                {
                    v ^= (x >> pEq->comps[c][i].index) & 1;
                }
                else if (pEq->comps[c][i].channel == 1)
                {
                    v ^= (y >> pEq->comps[c][i].index) & 1;
                }
                else
                {
                    ADDR_ASSERT(pEq->comps[c][i].channel == 2);
                    v ^= (z >> pEq->comps[c][i].index) & 1;
                }
            }
        }

        offset |= (v << i);
    }

    return offset;
}

/**
************************************************************************************************************************
*   Gfx11Lib::ComputeOffsetFromSwizzlePattern
*
*   @brief
*       Compute offset from swizzle pattern
*
*   @return
*       Offset
************************************************************************************************************************
*/
UINT_32 Gfx11Lib::ComputeOffsetFromSwizzlePattern(
    const UINT_64* pPattern,    ///< Swizzle pattern
    UINT_32        numBits,     ///< Number of bits in pattern
    UINT_32        x,           ///< x coord in pixel
    UINT_32        y,           ///< y coord in pixel
    UINT_32        z,           ///< z coord in slice
    UINT_32        s            ///< sample id
    ) const
{
    UINT_32                 offset          = 0;
    const ADDR_BIT_SETTING* pSwizzlePattern = reinterpret_cast<const ADDR_BIT_SETTING*>(pPattern);

    for (UINT_32 i = 0; i < numBits; i++)
    {
        UINT_32 v = 0;

        if (pSwizzlePattern[i].x != 0)
        {
            UINT_16 mask  = pSwizzlePattern[i].x;
            UINT_32 xBits = x;

            while (mask != 0)
            {
                if (mask & 1)
                {
                    v ^= xBits & 1;
                }

                xBits >>= 1;
                mask  >>= 1;
            }
        }

        if (pSwizzlePattern[i].y != 0)
        {
            UINT_16 mask  = pSwizzlePattern[i].y;
            UINT_32 yBits = y;

            while (mask != 0)
            {
                if (mask & 1)
                {
                    v ^= yBits & 1;
                }

                yBits >>= 1;
                mask  >>= 1;
            }
        }

        if (pSwizzlePattern[i].z != 0)
        {
            UINT_16 mask  = pSwizzlePattern[i].z;
            UINT_32 zBits = z;

            while (mask != 0)
            {
                if (mask & 1)
                {
                    v ^= zBits & 1;
                }

                zBits >>= 1;
                mask  >>= 1;
            }
        }

        if (pSwizzlePattern[i].s != 0)
        {
            UINT_16 mask  = pSwizzlePattern[i].s;
            UINT_32 sBits = s;

            while (mask != 0)
            {
                if (mask & 1)
                {
                    v ^= sBits & 1;
                }

                sBits >>= 1;
                mask  >>= 1;
            }
        }

        offset |= (v << i);
    }

    return offset;
}

/**
************************************************************************************************************************
*   Gfx11Lib::GetSwizzlePatternInfo
*
*   @brief
*       Get swizzle pattern
*
*   @return
*       Swizzle pattern information
************************************************************************************************************************
*/
const ADDR_SW_PATINFO* Gfx11Lib::GetSwizzlePatternInfo(
    AddrSwizzleMode  swizzleMode,       ///< Swizzle mode
    AddrResourceType resourceType,      ///< Resource type
    UINT_32          elemLog2,          ///< Element size in bytes log2
    UINT_32          numFrag            ///< Number of fragment
    ) const
{
    const UINT_32          index       = IsXor(swizzleMode) ? (m_colorBaseIndex + elemLog2) : elemLog2;
    const ADDR_SW_PATINFO* patInfo     = NULL;
    const UINT_32          swizzleMask = 1 << swizzleMode;
    const BOOL_32          isBlock256k = IsBlock256kb(swizzleMode);
    const BOOL_32          isBlock64K  = IsBlock64kb(swizzleMode);

    if (IsLinear(swizzleMode) == FALSE)
    {
        if (resourceType == ADDR_RSRC_TEX_3D)
        {
            ADDR_ASSERT(numFrag == 1);

            if ((swizzleMask & Gfx11Rsrc3dSwModeMask) != 0)
            {
                if (IsZOrderSwizzle(swizzleMode) || IsRtOptSwizzle(swizzleMode))
                {
                    if (isBlock256k)
                    {
                        ADDR_ASSERT((swizzleMode == ADDR_SW_256KB_Z_X) || (swizzleMode == ADDR_SW_256KB_R_X));
                        patInfo = GFX11_SW_256K_ZR_X_1xaa_PATINFO;
                    }
                    else if (isBlock64K)
                    {
                        ADDR_ASSERT((swizzleMode == ADDR_SW_64KB_Z_X) || (swizzleMode == ADDR_SW_64KB_R_X));
                        patInfo = GFX11_SW_64K_ZR_X_1xaa_PATINFO;
                    }
                    else
                    {
                        ADDR_ASSERT_ALWAYS();
                    }
                }
                else if (IsDisplaySwizzle(resourceType, swizzleMode))
                {
                    if (isBlock256k)
                    {
                        ADDR_ASSERT(swizzleMode == ADDR_SW_256KB_D_X);
                        // patInfo = GFX11_SW_256K_D3_X_PATINFO;
                    }
                    else if (isBlock64K)
                    {
                        ADDR_ASSERT(swizzleMode == ADDR_SW_64KB_D_X);
                        patInfo = GFX11_SW_64K_D3_X_PATINFO;
                    }
                    else
                    {
                        ADDR_ASSERT_ALWAYS();
                    }
                }
                else
                {
                    ADDR_ASSERT(IsStandardSwizzle(resourceType, swizzleMode));

                    if (isBlock256k)
                    {
                        ADDR_ASSERT(swizzleMode == ADDR_SW_256KB_S_X);
                        patInfo = GFX11_SW_256K_S3_X_PATINFO;
                    }
                    else if (isBlock64K)
                    {
                        if (swizzleMode == ADDR_SW_64KB_S)
                        {
                            patInfo = GFX11_SW_64K_S3_PATINFO;
                        }
                        else if (swizzleMode == ADDR_SW_64KB_S_X)
                        {
                            patInfo = GFX11_SW_64K_S3_X_PATINFO;
                        }
                        else if (swizzleMode == ADDR_SW_64KB_S_T)
                        {
                            patInfo = GFX11_SW_64K_S3_T_PATINFO;
                        }
                        else
                        {
                            ADDR_ASSERT_ALWAYS();
                        }
                    }
                    else if (IsBlock4kb(swizzleMode))
                    {
                        if (swizzleMode == ADDR_SW_4KB_S)
                        {
                            patInfo = GFX11_SW_4K_S3_PATINFO;
                        }
                        else if (swizzleMode == ADDR_SW_4KB_S_X)
                        {
                            patInfo = GFX11_SW_4K_S3_X_PATINFO;
                        }
                        else
                        {
                            ADDR_ASSERT_ALWAYS();
                        }
                    }
                    else
                    {
                        ADDR_ASSERT_ALWAYS();
                    }
                }
            }
        }
        else
        {
            if ((swizzleMask & Gfx11Rsrc2dSwModeMask) != 0)
            {
                if (IsBlock256b(swizzleMode))
                {
                    ADDR_ASSERT(swizzleMode == ADDR_SW_256B_D);
                    patInfo = GFX11_SW_256_D_PATINFO;
                }
                else if (IsBlock4kb(swizzleMode))
                {
                    if (swizzleMode == ADDR_SW_4KB_D)
                    {
                        patInfo = GFX11_SW_4K_D_PATINFO;
                    }
                    else if (swizzleMode == ADDR_SW_4KB_D_X)
                    {
                        patInfo = GFX11_SW_4K_D_X_PATINFO;
                    }
                    else
                    {
                        ADDR_ASSERT_ALWAYS();
                    }
                }
                else if (isBlock64K)
                {
                    if (IsZOrderSwizzle(swizzleMode) || IsRtOptSwizzle(swizzleMode))
                    {
                        if (numFrag == 1)
                        {
                            patInfo = GFX11_SW_64K_ZR_X_1xaa_PATINFO;
                        }
                        else if (numFrag == 2)
                        {
                            patInfo = GFX11_SW_64K_ZR_X_2xaa_PATINFO;
                        }
                        else if (numFrag == 4)
                        {
                            patInfo = GFX11_SW_64K_ZR_X_4xaa_PATINFO;
                        }
                        else if (numFrag == 8)
                        {
                            patInfo = GFX11_SW_64K_ZR_X_8xaa_PATINFO;
                        }
                        else
                        {
                            ADDR_ASSERT_ALWAYS();
                        }
                    }
                    else if (IsDisplaySwizzle(resourceType, swizzleMode))
                    {
                        if (swizzleMode == ADDR_SW_64KB_D)
                        {
                            patInfo = GFX11_SW_64K_D_PATINFO;
                        }
                        else if (swizzleMode == ADDR_SW_64KB_D_X)
                        {
                            patInfo = GFX11_SW_64K_D_X_PATINFO;
                        }
                        else if (swizzleMode == ADDR_SW_64KB_D_T)
                        {
                            patInfo = GFX11_SW_64K_D_T_PATINFO;
                        }
                        else
                        {
                            ADDR_ASSERT_ALWAYS();
                        }
                    }
                    else
                    {
                        ADDR_ASSERT_ALWAYS();
                    }
                }
                else if (isBlock256k)
                {
                    if (IsZOrderSwizzle(swizzleMode) || IsRtOptSwizzle(swizzleMode))
                    {
                        if (numFrag == 1)
                        {
                            patInfo = GFX11_SW_256K_ZR_X_1xaa_PATINFO;
                        }
                        else if (numFrag == 2)
                        {
                            patInfo = GFX11_SW_256K_ZR_X_2xaa_PATINFO;
                        }
                        else if (numFrag == 4)
                        {
                            patInfo = GFX11_SW_256K_ZR_X_4xaa_PATINFO;
                        }
                        else if (numFrag == 8)
                        {
                            patInfo = GFX11_SW_256K_ZR_X_8xaa_PATINFO;
                        }
                        else
                        {
                            ADDR_ASSERT_ALWAYS();
                        }
                    }
                    else if (IsDisplaySwizzle(resourceType, swizzleMode))
                    {
                        ADDR_ASSERT(swizzleMode == ADDR_SW_256KB_D_X);
                        patInfo = GFX11_SW_256K_D_X_PATINFO;
                    }
                    else
                    {
                        ADDR_ASSERT_ALWAYS();
                    }
                }
                else
                {
                    ADDR_ASSERT_ALWAYS();
                }
            }
        }
    }

    return (patInfo != NULL) ? &patInfo[index] : NULL;
}

/**
************************************************************************************************************************
*   Gfx11Lib::ComputeSurfaceAddrFromCoordMicroTiled
*
*   @brief
*       Internal function to calculate address from coord for micro tiled swizzle surface
*
*   @return
*       ADDR_E_RETURNCODE
************************************************************************************************************************
*/
ADDR_E_RETURNCODE Gfx11Lib::ComputeSurfaceAddrFromCoordMicroTiled(
     const ADDR2_COMPUTE_SURFACE_ADDRFROMCOORD_INPUT* pIn,    ///< [in] input structure
     ADDR2_COMPUTE_SURFACE_ADDRFROMCOORD_OUTPUT*      pOut    ///< [out] output structure
     ) const
{
    ADDR2_COMPUTE_SURFACE_INFO_INPUT  localIn  = {};
    ADDR2_COMPUTE_SURFACE_INFO_OUTPUT localOut = {};
    ADDR2_MIP_INFO                    mipInfo[MaxMipLevels];

    localIn.swizzleMode  = pIn->swizzleMode;
    localIn.flags        = pIn->flags;
    localIn.resourceType = pIn->resourceType;
    localIn.bpp          = pIn->bpp;
    localIn.width        = Max(pIn->unalignedWidth,  1u);
    localIn.height       = Max(pIn->unalignedHeight, 1u);
    localIn.numSlices    = Max(pIn->numSlices,       1u);
    localIn.numMipLevels = Max(pIn->numMipLevels,    1u);
    localIn.numSamples   = Max(pIn->numSamples,      1u);
    localIn.numFrags     = localIn.numSamples;
    localOut.pMipInfo    = mipInfo;

    ADDR_E_RETURNCODE ret = ComputeSurfaceInfoMicroTiled(&localIn, &localOut);

    if (ret == ADDR_OK)
    {
        const UINT_32 elemLog2 = Log2(pIn->bpp >> 3);
        const UINT_32 rsrcType = static_cast<UINT_32>(pIn->resourceType) - 1;
        const UINT_32 swMode   = static_cast<UINT_32>(pIn->swizzleMode);
        const UINT_32 eqIndex  = m_equationLookupTable[rsrcType][swMode][elemLog2];

        if (eqIndex != ADDR_INVALID_EQUATION_INDEX)
        {
            const UINT_32 pb           = mipInfo[pIn->mipId].pitch / localOut.blockWidth;
            const UINT_32 yb           = pIn->y / localOut.blockHeight;
            const UINT_32 xb           = pIn->x / localOut.blockWidth;
            const UINT_32 blockIndex   = yb * pb + xb;
            const UINT_32 blockSize    = 256;
            const UINT_32 blk256Offset = ComputeOffsetFromEquation(&m_equationTable[eqIndex],
                                                                   pIn->x << elemLog2,
                                                                   pIn->y,
                                                                   0);
            pOut->addr = localOut.sliceSize * pIn->slice +
                         mipInfo[pIn->mipId].macroBlockOffset +
                         (blockIndex * blockSize) +
                         blk256Offset;
        }
        else
        {
            ret = ADDR_INVALIDPARAMS;
        }
    }

    return ret;
}

/**
************************************************************************************************************************
*   Gfx11Lib::ComputeSurfaceAddrFromCoordMacroTiled
*
*   @brief
*       Internal function to calculate address from coord for macro tiled swizzle surface
*
*   @return
*       ADDR_E_RETURNCODE
************************************************************************************************************************
*/
ADDR_E_RETURNCODE Gfx11Lib::ComputeSurfaceAddrFromCoordMacroTiled(
     const ADDR2_COMPUTE_SURFACE_ADDRFROMCOORD_INPUT* pIn,    ///< [in] input structure
     ADDR2_COMPUTE_SURFACE_ADDRFROMCOORD_OUTPUT*      pOut    ///< [out] output structure
     ) const
{
    ADDR2_COMPUTE_SURFACE_INFO_INPUT  localIn  = {};
    ADDR2_COMPUTE_SURFACE_INFO_OUTPUT localOut = {};
    ADDR2_MIP_INFO                    mipInfo[MaxMipLevels];

    localIn.swizzleMode  = pIn->swizzleMode;
    localIn.flags        = pIn->flags;
    localIn.resourceType = pIn->resourceType;
    localIn.bpp          = pIn->bpp;
    localIn.width        = Max(pIn->unalignedWidth,  1u);
    localIn.height       = Max(pIn->unalignedHeight, 1u);
    localIn.numSlices    = Max(pIn->numSlices,       1u);
    localIn.numMipLevels = Max(pIn->numMipLevels,    1u);
    localIn.numSamples   = Max(pIn->numSamples,      1u);
    localIn.numFrags     = localIn.numSamples;
    localOut.pMipInfo    = mipInfo;

    ADDR_E_RETURNCODE ret = ComputeSurfaceInfoMacroTiled(&localIn, &localOut);

    if (ret == ADDR_OK)
    {
        const UINT_32 elemLog2    = Log2(pIn->bpp >> 3);
        const UINT_32 blkSizeLog2 = GetBlockSizeLog2(pIn->swizzleMode);
        const UINT_32 blkMask     = (1 << blkSizeLog2) - 1;
        const UINT_32 pipeMask    = (1 << m_pipesLog2) - 1;
        const UINT_32 bankMask    = ((1 << GetBankXorBits(blkSizeLog2)) - 1) << (m_pipesLog2 + ColumnBits);
        const UINT_32 pipeBankXor = IsXor(pIn->swizzleMode) ?
                                    (((pIn->pipeBankXor & (pipeMask | bankMask)) << m_pipeInterleaveLog2) & blkMask) : 0;

        if (localIn.numSamples > 1)
        {
            const ADDR_SW_PATINFO* pPatInfo = GetSwizzlePatternInfo(pIn->swizzleMode,
                                                                    pIn->resourceType,
                                                                    elemLog2,
                                                                    localIn.numSamples);

            if (pPatInfo != NULL)
            {
                const UINT_32 pb     = localOut.pitch / localOut.blockWidth;
                const UINT_32 yb     = pIn->y / localOut.blockHeight;
                const UINT_32 xb     = pIn->x / localOut.blockWidth;
                const UINT_64 blkIdx = yb * pb + xb;

                ADDR_BIT_SETTING fullSwizzlePattern[20];
                GetSwizzlePatternFromPatternInfo(pPatInfo, fullSwizzlePattern);

                const UINT_32 blkOffset =
                    ComputeOffsetFromSwizzlePattern(reinterpret_cast<const UINT_64*>(fullSwizzlePattern),
                                                    blkSizeLog2,
                                                    pIn->x,
                                                    pIn->y,
                                                    pIn->slice,
                                                    pIn->sample);

                pOut->addr = (localOut.sliceSize * pIn->slice) +
                             (blkIdx << blkSizeLog2) +
                             (blkOffset ^ pipeBankXor);
            }
            else
            {
                ret = ADDR_INVALIDPARAMS;
            }
        }
        else
        {
            const UINT_32 rsrcIdx = (pIn->resourceType == ADDR_RSRC_TEX_3D) ? 1 : 0;
            const UINT_32 swMode  = static_cast<UINT_32>(pIn->swizzleMode);
            const UINT_32 eqIndex = m_equationLookupTable[rsrcIdx][swMode][elemLog2];

            if (eqIndex != ADDR_INVALID_EQUATION_INDEX)
            {
                const BOOL_32 inTail    = (mipInfo[pIn->mipId].mipTailOffset != 0) ? TRUE : FALSE;
                const BOOL_32 isThin    = IsThin(pIn->resourceType, pIn->swizzleMode);
                const UINT_64 sliceSize = isThin ? localOut.sliceSize : (localOut.sliceSize * localOut.blockSlices);
                const UINT_32 sliceId   = isThin ? pIn->slice : (pIn->slice / localOut.blockSlices);
                const UINT_32 x         = inTail ? (pIn->x     + mipInfo[pIn->mipId].mipTailCoordX) : pIn->x;
                const UINT_32 y         = inTail ? (pIn->y     + mipInfo[pIn->mipId].mipTailCoordY) : pIn->y;
                const UINT_32 z         = inTail ? (pIn->slice + mipInfo[pIn->mipId].mipTailCoordZ) : pIn->slice;
                const UINT_32 pb        = mipInfo[pIn->mipId].pitch / localOut.blockWidth;
                const UINT_32 yb        = pIn->y / localOut.blockHeight;
                const UINT_32 xb        = pIn->x / localOut.blockWidth;
                const UINT_64 blkIdx    = yb * pb + xb;
                const UINT_32 blkOffset = ComputeOffsetFromEquation(&m_equationTable[eqIndex],
                                                                    x << elemLog2,
                                                                    y,
                                                                    z);
                pOut->addr = sliceSize * sliceId +
                             mipInfo[pIn->mipId].macroBlockOffset +
                             (blkIdx << blkSizeLog2) +
                             (blkOffset ^ pipeBankXor);
            }
            else
            {
                ret = ADDR_INVALIDPARAMS;
            }
        }
    }

    return ret;
}

/**
************************************************************************************************************************
*   Gfx11Lib::HwlComputeMaxBaseAlignments
*
*   @brief
*       Gets maximum alignments
*   @return
*       maximum alignments
************************************************************************************************************************
*/
UINT_32 Gfx11Lib::HwlComputeMaxBaseAlignments() const
{
    return Size256K;
}

/**
************************************************************************************************************************
*   Gfx11Lib::HwlComputeMaxMetaBaseAlignments
*
*   @brief
*       Gets maximum alignments for metadata
*   @return
*       maximum alignments for metadata
************************************************************************************************************************
*/
UINT_32 Gfx11Lib::HwlComputeMaxMetaBaseAlignments() const
{
    Dim3d metaBlk;

    // Max base alignment for Htile
    const AddrSwizzleMode ValidSwizzleModeForHtile[] =
    {
        ADDR_SW_64KB_Z_X,
        ADDR_SW_256KB_Z_X,
    };

    UINT_32 maxBaseAlignHtile = 0;

    for (UINT_32 swIdx = 0; swIdx < sizeof(ValidSwizzleModeForHtile) / sizeof(ValidSwizzleModeForHtile[0]); swIdx++)
    {
        for (UINT_32 bppLog2 = 0; bppLog2 < 3; bppLog2++)
        {
            for (UINT_32 numFragLog2 = 0; numFragLog2 < 4; numFragLog2++)
            {
                const UINT_32 metaBlkSizeHtile = GetMetaBlkSize(Gfx11DataDepthStencil,
                                                                ADDR_RSRC_TEX_2D,
                                                                ValidSwizzleModeForHtile[swIdx],
                                                                bppLog2,
                                                                numFragLog2,
                                                                TRUE,
                                                                &metaBlk);

                maxBaseAlignHtile = Max(maxBaseAlignHtile, metaBlkSizeHtile);
            }
        }
    }

    // Max base alignment for 2D Dcc
    // swizzle mode support DCC...
    const AddrSwizzleMode ValidSwizzleModeForDcc2D[] =
    {
        ADDR_SW_64KB_R_X,
        ADDR_SW_256KB_R_X,
    };

    UINT_32 maxBaseAlignDcc2D = 0;

    for (UINT_32 swIdx = 0; swIdx < sizeof(ValidSwizzleModeForDcc2D) / sizeof(ValidSwizzleModeForDcc2D[0]); swIdx++)
    {
        for (UINT_32 bppLog2 = 0; bppLog2 < MaxNumOfBpp; bppLog2++)
        {
            for (UINT_32 numFragLog2 = 0; numFragLog2 < 4; numFragLog2++)
            {
                const UINT_32 metaBlkSize2D = GetMetaBlkSize(Gfx11DataColor,
                                                             ADDR_RSRC_TEX_2D,
                                                             ValidSwizzleModeForDcc2D[swIdx],
                                                             bppLog2,
                                                             numFragLog2,
                                                             TRUE,
                                                             &metaBlk);

                maxBaseAlignDcc2D = Max(maxBaseAlignDcc2D, metaBlkSize2D);
            }
        }
    }

    // Max base alignment for 3D Dcc
    const AddrSwizzleMode ValidSwizzleModeForDcc3D[] =
    {
        ADDR_SW_64KB_S_X,
        ADDR_SW_64KB_D_X,
        ADDR_SW_64KB_R_X,
        ADDR_SW_256KB_S_X,
        ADDR_SW_256KB_D_X,
        ADDR_SW_256KB_R_X,
    };

    UINT_32 maxBaseAlignDcc3D = 0;

    for (UINT_32 swIdx = 0; swIdx < sizeof(ValidSwizzleModeForDcc3D) / sizeof(ValidSwizzleModeForDcc3D[0]); swIdx++)
    {
        for (UINT_32 bppLog2 = 0; bppLog2 < MaxNumOfBpp; bppLog2++)
        {
            const UINT_32 metaBlkSize3D = GetMetaBlkSize(Gfx11DataColor,
                                                         ADDR_RSRC_TEX_3D,
                                                         ValidSwizzleModeForDcc3D[swIdx],
                                                         bppLog2,
                                                         0,
                                                         TRUE,
                                                         &metaBlk);

            maxBaseAlignDcc3D = Max(maxBaseAlignDcc3D, metaBlkSize3D);
        }
    }

    return Max(maxBaseAlignHtile, Max(maxBaseAlignDcc2D, maxBaseAlignDcc3D));
}

/**
************************************************************************************************************************
*   Gfx11Lib::GetMetaElementSizeLog2
*
*   @brief
*       Gets meta data element size log2
*   @return
*       Meta data element size log2
************************************************************************************************************************
*/
INT_32 Gfx11Lib::GetMetaElementSizeLog2(
    Gfx11DataType dataType) ///< Data surface type
{
    INT_32 elemSizeLog2 = 0;

    if (dataType == Gfx11DataColor)
    {
        elemSizeLog2 = 0;
    }
    else
    {
        ADDR_ASSERT(dataType == Gfx11DataDepthStencil);
        elemSizeLog2 = 2;
    }

    return elemSizeLog2;
}

/**
************************************************************************************************************************
*   Gfx11Lib::GetMetaCacheSizeLog2
*
*   @brief
*       Gets meta data cache line size log2
*   @return
*       Meta data cache line size log2
************************************************************************************************************************
*/
INT_32 Gfx11Lib::GetMetaCacheSizeLog2(
    Gfx11DataType dataType) ///< Data surface type
{
    INT_32 cacheSizeLog2 = 0;

    if (dataType == Gfx11DataColor)
    {
        cacheSizeLog2 = 6;
    }
    else
    {
        ADDR_ASSERT(dataType == Gfx11DataDepthStencil);
        cacheSizeLog2 = 8;
    }

    return cacheSizeLog2;
}

/**
************************************************************************************************************************
*   Gfx11Lib::HwlComputeSurfaceInfoLinear
*
*   @brief
*       Internal function to calculate alignment for linear surface
*
*   @return
*       ADDR_E_RETURNCODE
************************************************************************************************************************
*/
ADDR_E_RETURNCODE Gfx11Lib::HwlComputeSurfaceInfoLinear(
     const ADDR2_COMPUTE_SURFACE_INFO_INPUT* pIn,    ///< [in] input structure
     ADDR2_COMPUTE_SURFACE_INFO_OUTPUT*      pOut    ///< [out] output structure
     ) const
{
    ADDR_E_RETURNCODE returnCode = ADDR_OK;

    if (IsTex1d(pIn->resourceType) && (pIn->height > 1))
    {
        returnCode = ADDR_INVALIDPARAMS;
    }
    else
    {
        const UINT_32 elementBytes = pIn->bpp >> 3;
        const UINT_32 pitchAlign   = (pIn->swizzleMode == ADDR_SW_LINEAR_GENERAL) ? 1 : (256 / elementBytes);
        const UINT_32 mipDepth     = (pIn->resourceType == ADDR_RSRC_TEX_3D) ? pIn->numSlices : 1;
        UINT_32       pitch        = PowTwoAlign(pIn->width, pitchAlign);
        UINT_32       actualHeight = pIn->height;
        UINT_64       sliceSize    = 0;

        if (pIn->numMipLevels > 1)
        {
            for (INT_32 i = static_cast<INT_32>(pIn->numMipLevels) - 1; i >= 0; i--)
            {
                UINT_32 mipWidth, mipHeight;

                GetMipSize(pIn->width, pIn->height, 1, i, &mipWidth, &mipHeight);

                const UINT_32 mipActualWidth = PowTwoAlign(mipWidth, pitchAlign);

                if (pOut->pMipInfo != NULL)
                {
                    pOut->pMipInfo[i].pitch            = mipActualWidth;
                    pOut->pMipInfo[i].height           = mipHeight;
                    pOut->pMipInfo[i].depth            = mipDepth;
                    pOut->pMipInfo[i].offset           = sliceSize;
                    pOut->pMipInfo[i].mipTailOffset    = 0;
                    pOut->pMipInfo[i].macroBlockOffset = sliceSize;
                }

                sliceSize += static_cast<UINT_64>(mipActualWidth) * mipHeight * elementBytes;
            }
        }
        else
        {
            returnCode = ApplyCustomizedPitchHeight(pIn, elementBytes, pitchAlign, &pitch, &actualHeight);

            if (returnCode == ADDR_OK)
            {
                sliceSize = static_cast<UINT_64>(pitch) * actualHeight * elementBytes;

                if (pOut->pMipInfo != NULL)
                {
                    pOut->pMipInfo[0].pitch            = pitch;
                    pOut->pMipInfo[0].height           = actualHeight;
                    pOut->pMipInfo[0].depth            = mipDepth;
                    pOut->pMipInfo[0].offset           = 0;
                    pOut->pMipInfo[0].mipTailOffset    = 0;
                    pOut->pMipInfo[0].macroBlockOffset = 0;
                }
            }
        }

        if (returnCode == ADDR_OK)
        {
            pOut->pitch          = pitch;
            pOut->height         = actualHeight;
            pOut->numSlices      = pIn->numSlices;
            pOut->sliceSize      = sliceSize;
            pOut->surfSize       = sliceSize * pOut->numSlices;
            pOut->baseAlign      = (pIn->swizzleMode == ADDR_SW_LINEAR_GENERAL) ? elementBytes : 256;
            pOut->blockWidth     = pitchAlign;
            pOut->blockHeight    = 1;
            pOut->blockSlices    = 1;

            // Following members are useless on GFX11
            pOut->mipChainPitch  = 0;
            pOut->mipChainHeight = 0;
            pOut->mipChainSlice  = 0;
            pOut->epitchIsHeight = FALSE;

            // Post calculation validate
            ADDR_ASSERT(pOut->sliceSize > 0);
        }
    }

    return returnCode;
}

} // V2
} // Addr
