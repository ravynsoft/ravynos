/*
************************************************************************************************************************
*
*  Copyright (C) 2007-2022 Advanced Micro Devices, Inc.  All rights reserved.
*  SPDX-License-Identifier: MIT
*
***********************************************************************************************************************/

/**
************************************************************************************************************************
* @file  gfx10addrlib.cpp
* @brief Contain the implementation for the Gfx10Lib class.
************************************************************************************************************************
*/

#include "gfx10addrlib.h"
#include "addrcommon.h"
#include "gfx10_gb_reg.h"

#include "amdgpu_asic_addr.h"

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace Addr
{
/**
************************************************************************************************************************
*   Gfx10HwlInit
*
*   @brief
*       Creates an Gfx10Lib object.
*
*   @return
*       Returns an Gfx10Lib object pointer.
************************************************************************************************************************
*/
Addr::Lib* Gfx10HwlInit(const Client* pClient)
{
    return V2::Gfx10Lib::CreateObj(pClient);
}

namespace V2
{

////////////////////////////////////////////////////////////////////////////////////////////////////
//                               Static Const Member
////////////////////////////////////////////////////////////////////////////////////////////////////

const SwizzleModeFlags Gfx10Lib::SwizzleModeTable[ADDR_SW_MAX_TYPE] =
{//Linear 256B  4KB  64KB   Var    Z    Std   Disp  Rot   XOR    T     RtOpt Reserved
    {{1,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0}}, // ADDR_SW_LINEAR
    {{0,    1,    0,    0,    0,    0,    1,    0,    0,    0,    0,    0,    0}}, // ADDR_SW_256B_S
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
    {{0,    0,    1,    0,    0,    0,    0,    0,    0,    1,    0,    1,    0}}, // ADDR_SW_4KB_R_X

    {{0,    0,    0,    1,    0,    1,    0,    0,    0,    1,    0,    0,    0}}, // ADDR_SW_64KB_Z_X
    {{0,    0,    0,    1,    0,    0,    1,    0,    0,    1,    0,    0,    0}}, // ADDR_SW_64KB_S_X
    {{0,    0,    0,    1,    0,    0,    0,    1,    0,    1,    0,    0,    0}}, // ADDR_SW_64KB_D_X
    {{0,    0,    0,    1,    0,    0,    0,    0,    0,    1,    0,    1,    0}}, // ADDR_SW_64KB_R_X

    {{0,    0,    0,    0,    1,    1,    0,    0,    0,    1,    0,    0,    0}}, // ADDR_SW_VAR_Z_X
    {{0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0}}, // Reserved
    {{0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0}}, // Reserved
    {{0,    0,    0,    0,    1,    0,    0,    0,    0,    1,    0,    1,    0}}, // ADDR_SW_VAR_R_X
    {{1,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0}}, // ADDR_SW_LINEAR_GENERAL
};

const Dim3d Gfx10Lib::Block256_3d[] = {{8, 4, 8}, {4, 4, 8}, {4, 4, 4}, {4, 2, 4}, {2, 2, 4}};

const Dim3d Gfx10Lib::Block64K_Log2_3d[] = {{6, 5, 5}, {5, 5, 5}, {5, 5, 4}, {5, 4, 4}, {4, 4, 4}};
const Dim3d Gfx10Lib::Block4K_Log2_3d[]  = {{4, 4, 4}, {3, 4, 4}, {3, 4, 3}, {3, 3, 3}, {2, 3, 3}};

/**
************************************************************************************************************************
*   Gfx10Lib::Gfx10Lib
*
*   @brief
*       Constructor
*
************************************************************************************************************************
*/
Gfx10Lib::Gfx10Lib(const Client* pClient)
    :
    Lib(pClient),
    m_numPkrLog2(0),
    m_numSaLog2(0),
    m_colorBaseIndex(0),
    m_xmaskBaseIndex(0),
    m_htileBaseIndex(0),
    m_dccBaseIndex(0)
{
    memset(&m_settings, 0, sizeof(m_settings));
    memcpy(m_swizzleModeTable, SwizzleModeTable, sizeof(SwizzleModeTable));
}

/**
************************************************************************************************************************
*   Gfx10Lib::~Gfx10Lib
*
*   @brief
*       Destructor
************************************************************************************************************************
*/
Gfx10Lib::~Gfx10Lib()
{
}

/**
************************************************************************************************************************
*   Gfx10Lib::HwlComputeHtileInfo
*
*   @brief
*       Interface function stub of AddrComputeHtilenfo
*
*   @return
*       ADDR_E_RETURNCODE
************************************************************************************************************************
*/
ADDR_E_RETURNCODE Gfx10Lib::HwlComputeHtileInfo(
    const ADDR2_COMPUTE_HTILE_INFO_INPUT* pIn,    ///< [in] input structure
    ADDR2_COMPUTE_HTILE_INFO_OUTPUT*      pOut    ///< [out] output structure
    ) const
{
    ADDR_E_RETURNCODE ret = ADDR_OK;

    if (((pIn->swizzleMode != ADDR_SW_64KB_Z_X) &&
         ((pIn->swizzleMode != ADDR_SW_VAR_Z_X) || (m_blockVarSizeLog2 == 0))) ||
        (pIn->hTileFlags.pipeAligned != TRUE))
    {
        ret = ADDR_INVALIDPARAMS;
    }
    else
    {
        Dim3d         metaBlk     = {};
        const UINT_32 metaBlkSize = GetMetaBlkSize(Gfx10DataDepthStencil,
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
        const UINT_32 index = m_xmaskBaseIndex;
        const UINT_8* patIdxTable = m_settings.supportRbPlus ? GFX10_HTILE_RBPLUS_PATIDX : GFX10_HTILE_PATIDX;

        ADDR_C_ASSERT(sizeof(GFX10_HTILE_SW_PATTERN[patIdxTable[index]]) == 72 * 2);
        pOut->equation.gfx10_bits = (UINT_16 *)GFX10_HTILE_SW_PATTERN[patIdxTable[index]];
    }

    return ret;
}

/**
************************************************************************************************************************
*   Gfx10Lib::HwlComputeCmaskInfo
*
*   @brief
*       Interface function stub of AddrComputeCmaskInfo
*
*   @return
*       ADDR_E_RETURNCODE
************************************************************************************************************************
*/
ADDR_E_RETURNCODE Gfx10Lib::HwlComputeCmaskInfo(
    const ADDR2_COMPUTE_CMASK_INFO_INPUT* pIn,    ///< [in] input structure
    ADDR2_COMPUTE_CMASK_INFO_OUTPUT*      pOut    ///< [out] output structure
    ) const
{
    ADDR_E_RETURNCODE ret = ADDR_OK;

    if ((pIn->resourceType != ADDR_RSRC_TEX_2D) ||
        (pIn->cMaskFlags.pipeAligned != TRUE)   ||
        ((pIn->swizzleMode != ADDR_SW_64KB_Z_X) &&
         ((pIn->swizzleMode != ADDR_SW_VAR_Z_X) || (m_blockVarSizeLog2 == 0))))
    {
        ret = ADDR_INVALIDPARAMS;
    }
    else
    {
        Dim3d         metaBlk     = {};
        const UINT_32 metaBlkSize = GetMetaBlkSize(Gfx10DataFmask,
                                                   ADDR_RSRC_TEX_2D,
                                                   pIn->swizzleMode,
                                                   0,
                                                   0,
                                                   TRUE,
                                                   &metaBlk);

        pOut->pitch         = PowTwoAlign(pIn->unalignedWidth,  metaBlk.w);
        pOut->height        = PowTwoAlign(pIn->unalignedHeight, metaBlk.h);
        pOut->baseAlign     = metaBlkSize;
        pOut->metaBlkWidth  = metaBlk.w;
        pOut->metaBlkHeight = metaBlk.h;

        if (pIn->numMipLevels > 1)
        {
            ADDR_ASSERT(pIn->firstMipIdInTail <= pIn->numMipLevels);

            UINT_32 metaBlkPerSlice = (pIn->firstMipIdInTail == pIn->numMipLevels) ? 0 : 1;

            for (INT_32 i = static_cast<INT_32>(pIn->firstMipIdInTail) - 1; i >= 0; i--)
            {
                UINT_32 mipWidth, mipHeight;

                GetMipSize(pIn->unalignedWidth, pIn->unalignedHeight, 1, i, &mipWidth, &mipHeight);

                mipWidth  = PowTwoAlign(mipWidth,  metaBlk.w);
                mipHeight = PowTwoAlign(mipHeight, metaBlk.h);

                const UINT_32 pitchInM  = mipWidth  / metaBlk.w;
                const UINT_32 heightInM = mipHeight / metaBlk.h;

                if (pOut->pMipInfo != NULL)
                {
                    pOut->pMipInfo[i].inMiptail = FALSE;
                    pOut->pMipInfo[i].offset    = metaBlkPerSlice * metaBlkSize;
                    pOut->pMipInfo[i].sliceSize = pitchInM * heightInM * metaBlkSize;
                }

                metaBlkPerSlice += pitchInM * heightInM;
            }

            pOut->metaBlkNumPerSlice = metaBlkPerSlice;

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

            if (pOut->pMipInfo != NULL)
            {
                pOut->pMipInfo[0].inMiptail = FALSE;
                pOut->pMipInfo[0].offset    = 0;
                pOut->pMipInfo[0].sliceSize = pOut->metaBlkNumPerSlice * metaBlkSize;
            }
        }

        pOut->sliceSize  = pOut->metaBlkNumPerSlice * metaBlkSize;
        pOut->cmaskBytes = pOut->sliceSize * pIn->numSlices;

        // Get the CMASK address equation (copied from CmaskAddrFromCoord)
        const UINT_32  fmaskBpp      = GetFmaskBpp(1, 1);
        const UINT_32  fmaskElemLog2 = Log2(fmaskBpp >> 3);
        const UINT_32  index         = m_xmaskBaseIndex + fmaskElemLog2;
        const UINT_8*  patIdxTable   =
            (pIn->swizzleMode == ADDR_SW_VAR_Z_X) ? GFX10_CMASK_VAR_RBPLUS_PATIDX :
            (m_settings.supportRbPlus ? GFX10_CMASK_64K_RBPLUS_PATIDX : GFX10_CMASK_64K_PATIDX);

        ADDR_C_ASSERT(sizeof(GFX10_CMASK_SW_PATTERN[patIdxTable[index]]) == 68 * 2);
        pOut->equation.gfx10_bits = (UINT_16*)GFX10_CMASK_SW_PATTERN[patIdxTable[index]];
    }

    return ret;
}

/**
************************************************************************************************************************
*   Gfx10Lib::HwlComputeDccInfo
*
*   @brief
*       Interface function to compute DCC key info
*
*   @return
*       ADDR_E_RETURNCODE
************************************************************************************************************************
*/
ADDR_E_RETURNCODE Gfx10Lib::HwlComputeDccInfo(
    const ADDR2_COMPUTE_DCCINFO_INPUT* pIn,    ///< [in] input structure
    ADDR2_COMPUTE_DCCINFO_OUTPUT*      pOut    ///< [out] output structure
    ) const
{
    ADDR_E_RETURNCODE ret = ADDR_OK;

    if (IsLinear(pIn->swizzleMode) || IsBlock256b(pIn->swizzleMode))
    {
        // Hardware support dcc for 256 swizzle mode, but address lib will not support it because we only
        // select 256 swizzle mode for small surface, and it's not helpful to enable dcc for small surface.
        ret = ADDR_INVALIDPARAMS;
    }
    else if (m_settings.dccUnsup3DSwDis && IsTex3d(pIn->resourceType) && IsDisplaySwizzle(pIn->swizzleMode))
    {
        // DCC is not supported on 3D Display surfaces for GFX10.0 and GFX10.1
        ret = ADDR_INVALIDPARAMS;
    }
    else
    {
        const UINT_32 elemLog2 = Log2(pIn->bpp >> 3);

        {
            // only SW_*_R_X surfaces may be DCC compressed when attached to the CB
            ADDR_ASSERT(IsRtOptSwizzle(pIn->swizzleMode));

            const BOOL_32 isThick = IsThick(pIn->resourceType, pIn->swizzleMode);

            pOut->compressBlkWidth  = isThick ? Block256_3d[elemLog2].w : Block256_2d[elemLog2].w;
            pOut->compressBlkHeight = isThick ? Block256_3d[elemLog2].h : Block256_2d[elemLog2].h;
            pOut->compressBlkDepth  = isThick ? Block256_3d[elemLog2].d : 1;
        }

        if (ret == ADDR_OK)
        {
            Dim3d         metaBlk     = {};
            const UINT_32 numFragLog2 = Log2(Max(pIn->numFrags, 1u));
            const UINT_32 metaBlkSize = GetMetaBlkSize(Gfx10DataColor,
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
            const UINT_8* patIdxTable;

            if (m_settings.supportRbPlus)
            {
                patIdxTable = GFX10_DCC_64K_R_X_RBPLUS_PATIDX;

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
            }
            else
            {
                patIdxTable = GFX10_DCC_64K_R_X_PATIDX;

                if (pIn->dccKeyFlags.pipeAligned)
                {
                    index += (numPipeLog2 + UnalignedDccType) * MaxNumOfBpp;
                }
                else
                {
                    index += Min(numPipeLog2, UnalignedDccType - 1) * MaxNumOfBpp;
                }
            }

            ADDR_C_ASSERT(sizeof(GFX10_DCC_64K_R_X_SW_PATTERN[patIdxTable[index]]) == 68 * 2);
            pOut->equation.gfx10_bits = (UINT_16*)GFX10_DCC_64K_R_X_SW_PATTERN[patIdxTable[index]];
        }
    }

    return ret;
}

/**
************************************************************************************************************************
*   Gfx10Lib::HwlComputeCmaskAddrFromCoord
*
*   @brief
*       Interface function stub of AddrComputeCmaskAddrFromCoord
*
*   @return
*       ADDR_E_RETURNCODE
************************************************************************************************************************
*/
ADDR_E_RETURNCODE Gfx10Lib::HwlComputeCmaskAddrFromCoord(
    const ADDR2_COMPUTE_CMASK_ADDRFROMCOORD_INPUT* pIn,    ///< [in] input structure
    ADDR2_COMPUTE_CMASK_ADDRFROMCOORD_OUTPUT*      pOut)   ///< [out] output structure
{
    // Only support pipe aligned CMask
    ADDR_ASSERT(pIn->cMaskFlags.pipeAligned == TRUE);

    ADDR2_COMPUTE_CMASK_INFO_INPUT input = {};
    input.size            = sizeof(input);
    input.cMaskFlags      = pIn->cMaskFlags;
    input.colorFlags      = pIn->colorFlags;
    input.unalignedWidth  = Max(pIn->unalignedWidth,  1u);
    input.unalignedHeight = Max(pIn->unalignedHeight, 1u);
    input.numSlices       = Max(pIn->numSlices,       1u);
    input.swizzleMode     = pIn->swizzleMode;
    input.resourceType    = pIn->resourceType;

    ADDR2_COMPUTE_CMASK_INFO_OUTPUT output = {};
    output.size = sizeof(output);

    ADDR_E_RETURNCODE returnCode = ComputeCmaskInfo(&input, &output);

    if (returnCode == ADDR_OK)
    {
        const UINT_32  fmaskBpp      = GetFmaskBpp(pIn->numSamples, pIn->numFrags);
        const UINT_32  fmaskElemLog2 = Log2(fmaskBpp >> 3);
        const UINT_32  pipeMask      = (1 << m_pipesLog2) - 1;
        const UINT_32  index         = m_xmaskBaseIndex + fmaskElemLog2;
        const UINT_8*  patIdxTable   =
            (pIn->swizzleMode == ADDR_SW_VAR_Z_X) ? GFX10_CMASK_VAR_RBPLUS_PATIDX :
            (m_settings.supportRbPlus ? GFX10_CMASK_64K_RBPLUS_PATIDX : GFX10_CMASK_64K_PATIDX);

        const UINT_32  blkSizeLog2  = Log2(output.metaBlkWidth) + Log2(output.metaBlkHeight) - 7;
        const UINT_32  blkMask      = (1 << blkSizeLog2) - 1;
        const UINT_32  blkOffset    = ComputeOffsetFromSwizzlePattern(GFX10_CMASK_SW_PATTERN[patIdxTable[index]],
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

        pOut->addr = (output.sliceSize * pIn->slice) +
                     (blkIndex * (1 << blkSizeLog2)) +
                     ((blkOffset >> 1) ^ pipeXor);
        pOut->bitPosition = (blkOffset & 1) << 2;
    }

    return returnCode;
}

/**
************************************************************************************************************************
*   Gfx10Lib::HwlComputeHtileAddrFromCoord
*
*   @brief
*       Interface function stub of AddrComputeHtileAddrFromCoord
*
*   @return
*       ADDR_E_RETURNCODE
************************************************************************************************************************
*/
ADDR_E_RETURNCODE Gfx10Lib::HwlComputeHtileAddrFromCoord(
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
            const UINT_8*  patIdxTable   = m_settings.supportRbPlus ? GFX10_HTILE_RBPLUS_PATIDX : GFX10_HTILE_PATIDX;

            const UINT_32  blkSizeLog2   = Log2(output.metaBlkWidth) + Log2(output.metaBlkHeight) - 4;
            const UINT_32  blkMask       = (1 << blkSizeLog2) - 1;
            const UINT_32  blkOffset     = ComputeOffsetFromSwizzlePattern(GFX10_HTILE_SW_PATTERN[patIdxTable[index]],
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
*   Gfx10Lib::HwlComputeHtileCoordFromAddr
*
*   @brief
*       Interface function stub of AddrComputeHtileCoordFromAddr
*
*   @return
*       ADDR_E_RETURNCODE
************************************************************************************************************************
*/
ADDR_E_RETURNCODE Gfx10Lib::HwlComputeHtileCoordFromAddr(
    const ADDR2_COMPUTE_HTILE_COORDFROMADDR_INPUT* pIn,    ///< [in] input structure
    ADDR2_COMPUTE_HTILE_COORDFROMADDR_OUTPUT*      pOut)   ///< [out] output structure
{
    ADDR_NOT_IMPLEMENTED();

    return ADDR_OK;
}

/**
************************************************************************************************************************
*   Gfx10Lib::HwlSupportComputeDccAddrFromCoord
*
*   @brief
*       Check whether HwlComputeDccAddrFromCoord() can be done for the input parameter
*
*   @return
*       ADDR_E_RETURNCODE
************************************************************************************************************************
*/
ADDR_E_RETURNCODE Gfx10Lib::HwlSupportComputeDccAddrFromCoord(
    const ADDR2_COMPUTE_DCC_ADDRFROMCOORD_INPUT* pIn)
{
    ADDR_E_RETURNCODE returnCode = ADDR_OK;

    if ((pIn->resourceType       != ADDR_RSRC_TEX_2D) ||
        (pIn->swizzleMode        != ADDR_SW_64KB_R_X) ||
        (pIn->dccKeyFlags.linear == TRUE)             ||
        (pIn->numFrags           >  1)                ||
        (pIn->numMipLevels       >  1)                ||
        (pIn->mipId              >  0))
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
*   Gfx10Lib::HwlComputeDccAddrFromCoord
*
*   @brief
*       Interface function stub of AddrComputeDccAddrFromCoord
*
*   @return
*       N/A
************************************************************************************************************************
*/
VOID Gfx10Lib::HwlComputeDccAddrFromCoord(
    const ADDR2_COMPUTE_DCC_ADDRFROMCOORD_INPUT* pIn,  ///< [in] input structure
    ADDR2_COMPUTE_DCC_ADDRFROMCOORD_OUTPUT*      pOut) ///< [out] output structure
{
    const UINT_32 elemLog2    = Log2(pIn->bpp >> 3);
    const UINT_32 numPipeLog2 = m_pipesLog2;
    const UINT_32 pipeMask    = (1 << numPipeLog2) - 1;
    UINT_32       index       = m_dccBaseIndex + elemLog2;
    const UINT_8* patIdxTable;

    if (m_settings.supportRbPlus)
    {
        patIdxTable = GFX10_DCC_64K_R_X_RBPLUS_PATIDX;

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
    }
    else
    {
        patIdxTable = GFX10_DCC_64K_R_X_PATIDX;

        if (pIn->dccKeyFlags.pipeAligned)
        {
            index += (numPipeLog2 + UnalignedDccType) * MaxNumOfBpp;
        }
        else
        {
            index += Min(numPipeLog2, UnalignedDccType - 1) * MaxNumOfBpp;
        }
    }

    const UINT_32  blkSizeLog2 = Log2(pIn->metaBlkWidth) + Log2(pIn->metaBlkHeight) + elemLog2 - 8;
    const UINT_32  blkMask     = (1 << blkSizeLog2) - 1;
    const UINT_32  blkOffset   =
        ComputeOffsetFromSwizzlePattern(GFX10_DCC_64K_R_X_SW_PATTERN[patIdxTable[index]],
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
*   Gfx10Lib::HwlInitGlobalParams
*
*   @brief
*       Initializes global parameters
*
*   @return
*       TRUE if all settings are valid
*
************************************************************************************************************************
*/
BOOL_32 Gfx10Lib::HwlInitGlobalParams(
    const ADDR_CREATE_INPUT* pCreateIn) ///< [in] create input
{
    BOOL_32              valid = TRUE;
    GB_ADDR_CONFIG_GFX10 gbAddrConfig;

    gbAddrConfig.u32All = pCreateIn->regValue.gbAddrConfig;

    // These values are copied from CModel code
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

    switch (gbAddrConfig.bits.MAX_COMPRESSED_FRAGS)
    {
        case ADDR_CONFIG_1_MAX_COMPRESSED_FRAGMENTS:
            m_maxCompFrag     = 1;
            m_maxCompFragLog2 = 0;
            break;
        case ADDR_CONFIG_2_MAX_COMPRESSED_FRAGMENTS:
            m_maxCompFrag     = 2;
            m_maxCompFragLog2 = 1;
            break;
        case ADDR_CONFIG_4_MAX_COMPRESSED_FRAGMENTS:
            m_maxCompFrag     = 4;
            m_maxCompFragLog2 = 2;
            break;
        case ADDR_CONFIG_8_MAX_COMPRESSED_FRAGMENTS:
            m_maxCompFrag     = 8;
            m_maxCompFragLog2 = 3;
            break;
        default:
            ADDR_ASSERT_ALWAYS();
            valid = FALSE;
            break;
    }

    {
        // Skip unaligned case
        m_xmaskBaseIndex += MaxNumOfBppCMask;
        m_htileBaseIndex += MaxNumOfAA;

        m_xmaskBaseIndex += m_pipesLog2 * MaxNumOfBppCMask;
        m_htileBaseIndex += m_pipesLog2 * MaxNumOfAA;
        m_colorBaseIndex += m_pipesLog2 * MaxNumOfBpp;

        if (m_settings.supportRbPlus)
        {
            m_numPkrLog2 = gbAddrConfig.bits.NUM_PKRS;
            m_numSaLog2  = (m_numPkrLog2 > 0) ? (m_numPkrLog2 - 1) : 0;

            ADDR_ASSERT((m_numPkrLog2 <= m_pipesLog2) && ((m_pipesLog2 - m_numPkrLog2) <= 2));

            ADDR_C_ASSERT(sizeof(GFX10_HTILE_RBPLUS_PATIDX) / sizeof(GFX10_HTILE_RBPLUS_PATIDX[0]) ==
                          sizeof(GFX10_CMASK_64K_RBPLUS_PATIDX) / sizeof(GFX10_CMASK_64K_RBPLUS_PATIDX[0]));

            if (m_numPkrLog2 >= 2)
            {
                m_colorBaseIndex += (2 * m_numPkrLog2 - 2) * MaxNumOfBpp;
                m_xmaskBaseIndex += (m_numPkrLog2 - 1) * 3 * MaxNumOfBppCMask;
                m_htileBaseIndex += (m_numPkrLog2 - 1) * 3 * MaxNumOfAA;
            }
        }
        else
        {
            const UINT_32 numPipeType = static_cast<UINT_32>(ADDR_CONFIG_64_PIPE) -
                                        static_cast<UINT_32>(ADDR_CONFIG_1_PIPE)  +
                                        1;

            ADDR_C_ASSERT(sizeof(GFX10_HTILE_PATIDX) / sizeof(GFX10_HTILE_PATIDX[0]) == (numPipeType + 1) * MaxNumOfAA);
            ADDR_C_ASSERT(sizeof(GFX10_CMASK_64K_PATIDX) / sizeof(GFX10_CMASK_64K_PATIDX[0]) ==
                          (numPipeType + 1) * MaxNumOfBppCMask);
        }
    }

    if (m_settings.supportRbPlus)
    {
        // VAR block size = 16K * num_pipes. For 4 pipe configuration, SW_VAR_* mode swizzle patterns are same as the
        // corresponding SW_64KB_* mode
        m_blockVarSizeLog2 = m_pipesLog2 + 14;
    }

    if (valid)
    {
        InitEquationTable();
    }

    return valid;
}

/**
************************************************************************************************************************
*   Gfx10Lib::HwlConvertChipFamily
*
*   @brief
*       Convert familyID defined in atiid.h to ChipFamily and set m_chipFamily/m_chipRevision
*   @return
*       ChipFamily
************************************************************************************************************************
*/
ChipFamily Gfx10Lib::HwlConvertChipFamily(
    UINT_32 chipFamily,        ///< [in] chip family defined in atiih.h
    UINT_32 chipRevision)      ///< [in] chip revision defined in "asic_family"_id.h
{
    ChipFamily family = ADDR_CHIP_FAMILY_NAVI;

    m_settings.dccUnsup3DSwDis  = 1;
    m_settings.dsMipmapHtileFix = 1;

    switch (chipFamily)
    {
        case FAMILY_NV:
            if (ASICREV_IS_NAVI10_P(chipRevision))
            {
                m_settings.dsMipmapHtileFix = 0;
                m_settings.isDcn20          = 1;
            }

            if (ASICREV_IS_NAVI12_P(chipRevision))
            {
                m_settings.isDcn20 = 1;
            }

            if (ASICREV_IS_NAVI14_M(chipRevision))
            {
                m_settings.isDcn20 = 1;
            }

            if (ASICREV_IS_NAVI21_M(chipRevision))
            {
                m_settings.supportRbPlus   = 1;
                m_settings.dccUnsup3DSwDis = 0;
            }

            if (ASICREV_IS_NAVI22_P(chipRevision))
            {
                m_settings.supportRbPlus   = 1;
                m_settings.dccUnsup3DSwDis = 0;
            }

            if (ASICREV_IS_NAVI23_P(chipRevision))
            {
                m_settings.supportRbPlus   = 1;
                m_settings.dccUnsup3DSwDis = 0;
            }

            if (ASICREV_IS_NAVI24_P(chipRevision))
            {
                m_settings.supportRbPlus   = 1;
                m_settings.dccUnsup3DSwDis = 0;
            }
            break;

        case FAMILY_VGH:
            if (ASICREV_IS_VANGOGH(chipRevision))
            {
                m_settings.supportRbPlus   = 1;
                m_settings.dccUnsup3DSwDis = 0;
            }
            else
            {
                ADDR_ASSERT(!"Unknown chip revision");
            }
            break;

        case FAMILY_RMB:
            if (ASICREV_IS_REMBRANDT(chipRevision))
            {
                m_settings.supportRbPlus   = 1;
                m_settings.dccUnsup3DSwDis = 0;
            }
            else
            {
                ADDR_ASSERT(!"Unknown chip revision");
            }
            break;
        case FAMILY_RPL:
            if (ASICREV_IS_RAPHAEL(chipRevision))
            {
                m_settings.supportRbPlus   = 1;
                m_settings.dccUnsup3DSwDis = 0;
            }
            break;
        case FAMILY_MDN:
            if (ASICREV_IS_MENDOCINO(chipRevision))
            {
                m_settings.supportRbPlus   = 1;
                m_settings.dccUnsup3DSwDis = 0;
            }
            else
            {
                ADDR_ASSERT(!"Unknown chip revision");
            }
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
*   Gfx10Lib::GetBlk256SizeLog2
*
*   @brief
*       Get block 256 size
*
*   @return
*       N/A
************************************************************************************************************************
*/
void Gfx10Lib::GetBlk256SizeLog2(
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

        if (IsZOrderSwizzle(swizzleMode))
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
*   Gfx10Lib::GetCompressedBlockSizeLog2
*
*   @brief
*       Get compress block size
*
*   @return
*       N/A
************************************************************************************************************************
*/
void Gfx10Lib::GetCompressedBlockSizeLog2(
    Gfx10DataType    dataType,          ///< [in] Data type
    AddrResourceType resourceType,      ///< [in] Resource type
    AddrSwizzleMode  swizzleMode,       ///< [in] Swizzle mode
    UINT_32          elemLog2,          ///< [in] element size log2
    UINT_32          numSamplesLog2,    ///< [in] number of samples
    Dim3d*           pBlock             ///< [out] block size
    ) const
{
    if (dataType == Gfx10DataColor)
    {
        GetBlk256SizeLog2(resourceType, swizzleMode, elemLog2, numSamplesLog2, pBlock);
    }
    else
    {
        ADDR_ASSERT((dataType == Gfx10DataDepthStencil) || (dataType == Gfx10DataFmask));
        pBlock->w = 3;
        pBlock->h = 3;
        pBlock->d = 0;
    }
}

/**
************************************************************************************************************************
*   Gfx10Lib::GetMetaOverlapLog2
*
*   @brief
*       Get meta block overlap
*
*   @return
*       N/A
************************************************************************************************************************
*/
INT_32 Gfx10Lib::GetMetaOverlapLog2(
    Gfx10DataType    dataType,          ///< [in] Data type
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

    if ((numPipesLog2 > 1) && m_settings.supportRbPlus)
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
*   Gfx10Lib::Get3DMetaOverlapLog2
*
*   @brief
*       Get 3d meta block overlap
*
*   @return
*       N/A
************************************************************************************************************************
*/
INT_32 Gfx10Lib::Get3DMetaOverlapLog2(
    AddrResourceType resourceType,      ///< [in] Resource type
    AddrSwizzleMode  swizzleMode,       ///< [in] Swizzle mode
    UINT_32          elemLog2           ///< [in] element size log2
    ) const
{
    Dim3d microBlock;
    GetBlk256SizeLog2(resourceType, swizzleMode, elemLog2, 0, &microBlock);

    INT_32 overlap = GetEffectiveNumPipes() - static_cast<INT_32>(microBlock.w);

    if (m_settings.supportRbPlus)
    {
        overlap++;
    }

    if ((overlap < 0) || (IsStandardSwizzle(resourceType, swizzleMode) == TRUE))
    {
        overlap = 0;
    }
    return overlap;
}

/**
************************************************************************************************************************
*   Gfx10Lib::GetPipeRotateAmount
*
*   @brief
*       Get pipe rotate amount
*
*   @return
*       Pipe rotate amount
************************************************************************************************************************
*/

INT_32 Gfx10Lib::GetPipeRotateAmount(
    AddrResourceType resourceType,      ///< [in] Resource type
    AddrSwizzleMode  swizzleMode        ///< [in] Swizzle mode
    ) const
{
    INT_32 amount = 0;

    if (m_settings.supportRbPlus && (m_pipesLog2 >= (m_numSaLog2 + 1)) && (m_pipesLog2 > 1))
    {
        amount = ((m_pipesLog2 == (m_numSaLog2 + 1)) && IsRbAligned(resourceType, swizzleMode)) ?
                 1 : m_pipesLog2 - (m_numSaLog2 + 1);
    }

    return amount;
}

/**
************************************************************************************************************************
*   Gfx10Lib::GetMetaBlkSize
*
*   @brief
*       Get metadata block size
*
*   @return
*       Meta block size
************************************************************************************************************************
*/
UINT_32 Gfx10Lib::GetMetaBlkSize(
    Gfx10DataType    dataType,          ///< [in] Data type
    AddrResourceType resourceType,      ///< [in] Resource type
    AddrSwizzleMode  swizzleMode,       ///< [in] Swizzle mode
    UINT_32          elemLog2,          ///< [in] element size log2
    UINT_32          numSamplesLog2,    ///< [in] number of samples
    BOOL_32          pipeAlign,         ///< [in] pipe align
    Dim3d*           pBlock             ///< [out] block size
    ) const
{
    INT_32 metablkSizeLog2;

    {
        const INT_32 metaElemSizeLog2   = GetMetaElementSizeLog2(dataType);
        const INT_32 metaCacheSizeLog2  = GetMetaCacheSizeLog2(dataType);
        const INT_32 compBlkSizeLog2    = (dataType == Gfx10DataColor) ? 8 : 6 + numSamplesLog2 + elemLog2;
        const INT_32 metaBlkSamplesLog2 = (dataType == Gfx10DataDepthStencil) ?
                                          numSamplesLog2 : Min(numSamplesLog2, m_maxCompFragLog2);
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
                if (m_settings.supportRbPlus && (m_pipesLog2 == m_numSaLog2 + 1) && (m_pipesLog2 > 1))
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

                    if (m_settings.supportRbPlus    &&
                        IsRtOptSwizzle(swizzleMode) &&
                        (numPipesLog2 == 6)         &&
                        (numSamplesLog2 == 3)       &&
                        (m_maxCompFragLog2 == 3)    &&
                        (metablkSizeLog2 < 15))
                    {
                        metablkSizeLog2 = 15;
                    }
                }
                else
                {
                    metablkSizeLog2 = Max(static_cast<INT_32>(m_pipeInterleaveLog2) + numPipesLog2, 12);
                }

                if (dataType == Gfx10DataDepthStencil)
                {
                    // For htile surfaces, pad meta block size to 2K * num_pipes
                    metablkSizeLog2 = Max(metablkSizeLog2, 11 + numPipesLog2);
                }

                const INT_32 compFragLog2 = Min(m_maxCompFragLog2, numSamplesLog2);

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
                if (m_settings.supportRbPlus         &&
                    (m_pipesLog2 == m_numSaLog2 + 1) &&
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
    }

    return (1 << static_cast<UINT_32>(metablkSizeLog2));
}

/**
************************************************************************************************************************
*   Gfx10Lib::ConvertSwizzlePatternToEquation
*
*   @brief
*       Convert swizzle pattern to equation.
*
*   @return
*       N/A
************************************************************************************************************************
*/
VOID Gfx10Lib::ConvertSwizzlePatternToEquation(
    UINT_32                elemLog2,  ///< [in] element bytes log2
    AddrResourceType       rsrcType,  ///< [in] resource type
    AddrSwizzleMode        swMode,    ///< [in] swizzle mode
    const ADDR_SW_PATINFO* pPatInfo,  ///< [in] swizzle pattern infor
    ADDR_EQUATION*         pEquation) ///< [out] equation converted from swizzle pattern
    const
{
    // Get full swizzle pattern and store it as an ADDR_BIT_SETTING list
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
*   Gfx10Lib::InitEquationTable
*
*   @brief
*       Initialize Equation table.
*
*   @return
*       N/A
************************************************************************************************************************
*/
VOID Gfx10Lib::InitEquationTable()
{
    memset(m_equationTable, 0, sizeof(m_equationTable));

    // Iterate through resourceTypes, up to MaxRsrcType where a "resourceType" refers to AddrResourceType (1D/2D/3D)
    // resources. This starts with rsrcTypeIdx = 0, however there is an offset added that will start us off at
    // computing 2D resources.
    for (UINT_32 rsrcTypeIdx = 0; rsrcTypeIdx < MaxRsrcType; rsrcTypeIdx++)
    {
        // Add offset. Start iterating from ADDR_RSRC_TEX_2D
        const AddrResourceType rsrcType = static_cast<AddrResourceType>(rsrcTypeIdx + ADDR_RSRC_TEX_2D);

        // Iterate through the maximum number of swizzlemodes a type can hold
        for (UINT_32 swModeIdx = 0; swModeIdx < MaxSwModeType; swModeIdx++)
        {
            const AddrSwizzleMode swMode = static_cast<AddrSwizzleMode>(swModeIdx);

            // Iterate through the different bits-per-pixel settings (8bpp/16bpp/32bpp/64bpp/128bpp)
            for (UINT_32 elemLog2 = 0; elemLog2 < MaxElementBytesLog2; elemLog2++)
            {
                UINT_32                equationIndex = ADDR_INVALID_EQUATION_INDEX;
                // May or may not return a ADDR_SW_PATINFO for a completely different swizzle mode, essentially
                // overwriting the choice.
                const ADDR_SW_PATINFO* pPatInfo      = GetSwizzlePatternInfo(swMode, rsrcType, elemLog2, 1);

                if (pPatInfo != NULL)
                {
                    ADDR_ASSERT(IsValidSwMode(swMode));
                    ADDR_EQUATION equation = {};

                    // Passing in pPatInfo to get the addr equation
                    ConvertSwizzlePatternToEquation(elemLog2, rsrcType, swMode, pPatInfo, &equation);

                    equationIndex = m_numEquations;
                    ADDR_ASSERT(equationIndex < EquationTableSize);
                    // Updates m_equationTable[m_numEquations] to be the addr equation for this PatInfo
                    m_equationTable[equationIndex] = equation;
                    // Increment m_numEquations
                    m_numEquations++;
                }
                // equationIndex, which is used to look up equations in m_equationTable, will be cached for every
                // iteration in this nested for-loop
                m_equationLookupTable[rsrcTypeIdx][swModeIdx][elemLog2] = equationIndex;
            }
        }
    }
}

/**
************************************************************************************************************************
*   Gfx10Lib::HwlGetEquationIndex
*
*   @brief
*       Interface function stub of GetEquationIndex
*
*   @return
*       ADDR_E_RETURNCODE
************************************************************************************************************************
*/
UINT_32 Gfx10Lib::HwlGetEquationIndex(
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
*   Gfx10Lib::GetValidDisplaySwizzleModes
*
*   @brief
*       Get valid swizzle modes mask for displayable surface
*
*   @return
*       Valid swizzle modes mask for displayable surface
************************************************************************************************************************
*/
UINT_32 Gfx10Lib::GetValidDisplaySwizzleModes(
    UINT_32 bpp
    ) const
{
    UINT_32 swModeMask = 0;

    if (bpp <= 64)
    {
        if (m_settings.isDcn20)
        {
            swModeMask = (bpp == 64) ? Dcn20Bpp64SwModeMask : Dcn20NonBpp64SwModeMask;
        }
        else
        {
            swModeMask = (bpp == 64) ? Dcn21Bpp64SwModeMask : Dcn21NonBpp64SwModeMask;
        }
    }

    return swModeMask;
}

/**
************************************************************************************************************************
*   Gfx10Lib::IsValidDisplaySwizzleMode
*
*   @brief
*       Check if a swizzle mode is supported by display engine
*
*   @return
*       TRUE is swizzle mode is supported by display engine
************************************************************************************************************************
*/
BOOL_32 Gfx10Lib::IsValidDisplaySwizzleMode(
    const ADDR2_COMPUTE_SURFACE_INFO_INPUT* pIn     ///< [in] input structure
    ) const
{
    ADDR_ASSERT(pIn->resourceType == ADDR_RSRC_TEX_2D);

    return (GetValidDisplaySwizzleModes(pIn->bpp) & (1 << pIn->swizzleMode)) ? TRUE : FALSE;
}

/**
************************************************************************************************************************
*   Gfx10Lib::GetMaxNumMipsInTail
*
*   @brief
*       Return max number of mips in tails
*
*   @return
*       Max number of mips in tails
************************************************************************************************************************
*/
UINT_32 Gfx10Lib::GetMaxNumMipsInTail(
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
*   Gfx10Lib::HwlComputePipeBankXor
*
*   @brief
*       Generate a PipeBankXor value to be ORed into bits above pipeInterleaveBits of address
*
*   @return
*       PipeBankXor value
************************************************************************************************************************
*/
ADDR_E_RETURNCODE Gfx10Lib::HwlComputePipeBankXor(
    const ADDR2_COMPUTE_PIPEBANKXOR_INPUT* pIn,     ///< [in] input structure
    ADDR2_COMPUTE_PIPEBANKXOR_OUTPUT*      pOut     ///< [out] output structure
    ) const
{
    if (IsNonPrtXor(pIn->swizzleMode))
    {
        const UINT_32 bankBits = GetBankXorBits(GetBlockSizeLog2(pIn->swizzleMode));

        // No pipe xor...
        const UINT_32 pipeXor = 0;
        UINT_32       bankXor = 0;

        const UINT_32         XorPatternLen = 8;
        static const UINT_32  XorBankRot1b[XorPatternLen] = {0,  1,  0,  1,  0,  1,  0,  1};
        static const UINT_32  XorBankRot2b[XorPatternLen] = {0,  2,  1,  3,  2,  0,  3,  1};
        static const UINT_32  XorBankRot3b[XorPatternLen] = {0,  4,  2,  6,  1,  5,  3,  7};
        static const UINT_32  XorBankRot4b[XorPatternLen] = {0,  8,  4, 12,  2, 10,  6, 14};
        static const UINT_32* XorBankRotPat[] = {XorBankRot1b, XorBankRot2b, XorBankRot3b, XorBankRot4b};

        switch (bankBits)
        {
            case 1:
            case 2:
            case 3:
            case 4:
                bankXor = XorBankRotPat[bankBits - 1][pIn->surfIndex % XorPatternLen] << (m_pipesLog2 + ColumnBits);
                break;
            default:
                // valid bank bits should be 0~4
                ADDR_ASSERT_ALWAYS();
            case 0:
                break;
        }

        pOut->pipeBankXor = bankXor | pipeXor;
    }
    else
    {
        pOut->pipeBankXor = 0;
    }

    return ADDR_OK;
}

/**
************************************************************************************************************************
*   Gfx10Lib::HwlComputeSlicePipeBankXor
*
*   @brief
*       Generate slice PipeBankXor value based on base PipeBankXor value and slice id
*
*   @return
*       PipeBankXor value
************************************************************************************************************************
*/
ADDR_E_RETURNCODE Gfx10Lib::HwlComputeSlicePipeBankXor(
    const ADDR2_COMPUTE_SLICE_PIPEBANKXOR_INPUT* pIn,   ///< [in] input structure
    ADDR2_COMPUTE_SLICE_PIPEBANKXOR_OUTPUT*      pOut   ///< [out] output structure
    ) const
{
    if (IsNonPrtXor(pIn->swizzleMode))
    {
        const UINT_32 blockBits = GetBlockSizeLog2(pIn->swizzleMode);
        const UINT_32 pipeBits  = GetPipeXorBits(blockBits);
        const UINT_32 pipeXor   = ReverseBitVector(pIn->slice, pipeBits);

        pOut->pipeBankXor = pIn->basePipeBankXor ^ pipeXor;

        if (pIn->bpe != 0)
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
                                                    blockBits,
                                                    0,
                                                    0,
                                                    pIn->slice,
                                                    0);

                const UINT_32 pipeBankXor = pipeBankXorOffset >> m_pipeInterleaveLog2;

                // Should have no bit set under pipe interleave
                ADDR_ASSERT((pipeBankXor << m_pipeInterleaveLog2) == pipeBankXorOffset);

                // This assertion firing means old approach doesn't calculate a correct sliceXor value...
                ADDR_ASSERT(pipeBankXor == pipeXor);

                pOut->pipeBankXor = pIn->basePipeBankXor ^ pipeBankXor;
            }
        }
    }
    else
    {
        pOut->pipeBankXor = 0;
    }

    return ADDR_OK;
}

/**
************************************************************************************************************************
*   Gfx10Lib::HwlComputeSubResourceOffsetForSwizzlePattern
*
*   @brief
*       Compute sub resource offset to support swizzle pattern
*
*   @return
*       Offset
************************************************************************************************************************
*/
ADDR_E_RETURNCODE Gfx10Lib::HwlComputeSubResourceOffsetForSwizzlePattern(
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
*   Gfx10Lib::HwlComputeNonBlockCompressedView
*
*   @brief
*       Compute non-block-compressed view for a given mipmap level/slice.
*
*   @return
*       ADDR_E_RETURNCODE
************************************************************************************************************************
*/
ADDR_E_RETURNCODE Gfx10Lib::HwlComputeNonBlockCompressedView(
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
        ADDR_ASSERT(pIn->numMipLevels <= MaxMipLevels);

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
*   Gfx10Lib::ValidateNonSwModeParams
*
*   @brief
*       Validate compute surface info params except swizzle mode
*
*   @return
*       TRUE if parameters are valid, FALSE otherwise
************************************************************************************************************************
*/
BOOL_32 Gfx10Lib::ValidateNonSwModeParams(
    const ADDR2_COMPUTE_SURFACE_INFO_INPUT* pIn) const
{
    BOOL_32 valid = TRUE;

    if ((pIn->bpp == 0) || (pIn->bpp > 128) || (pIn->width == 0) || (pIn->numFrags > 8) || (pIn->numSamples > 16))
    {
        ADDR_ASSERT_ALWAYS();
        valid = FALSE;
    }

    if (pIn->resourceType >= ADDR_RSRC_MAX_TYPE)
    {
        ADDR_ASSERT_ALWAYS();
        valid = FALSE;
    }

    const ADDR2_SURFACE_FLAGS flags    = pIn->flags;
    const AddrResourceType    rsrcType = pIn->resourceType;
    const BOOL_32             mipmap   = (pIn->numMipLevels > 1);
    const BOOL_32             msaa     = (pIn->numFrags > 1);
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
*   Gfx10Lib::ValidateSwModeParams
*
*   @brief
*       Validate compute surface info related to swizzle mode
*
*   @return
*       TRUE if parameters are valid, FALSE otherwise
************************************************************************************************************************
*/
BOOL_32 Gfx10Lib::ValidateSwModeParams(
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
        {
            ADDR_ASSERT_ALWAYS();
            valid = FALSE;
        }
    }

    const ADDR2_SURFACE_FLAGS flags       = pIn->flags;
    const AddrResourceType    rsrcType    = pIn->resourceType;
    const AddrSwizzleMode     swizzle     = pIn->swizzleMode;
    const BOOL_32             msaa        = (pIn->numFrags > 1);
    const BOOL_32             zbuffer     = flags.depth || flags.stencil;
    const BOOL_32             color       = flags.color;
    const BOOL_32             display     = flags.display;
    const BOOL_32             tex3d       = IsTex3d(rsrcType);
    const BOOL_32             tex2d       = IsTex2d(rsrcType);
    const BOOL_32             tex1d       = IsTex1d(rsrcType);
    const BOOL_32             thin3d      = flags.view3dAs2dArray;
    const BOOL_32             linear      = IsLinear(swizzle);
    const BOOL_32             blk256B     = IsBlock256b(swizzle);
    const BOOL_32             blkVar      = IsBlockVariable(swizzle);
    const BOOL_32             isNonPrtXor = IsNonPrtXor(swizzle);
    const BOOL_32             prt         = flags.prt;
    const BOOL_32             fmask       = flags.fmask;

    // Misc check
    if ((pIn->numFrags > 1) &&
        (GetBlockSize(swizzle) < (m_pipeInterleaveBytes * pIn->numFrags)))
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
        if ((swizzleMask & Gfx10Rsrc1dSwModeMask) == 0)
        {
            ADDR_ASSERT_ALWAYS();
            valid = FALSE;
        }
    }
    else if (tex2d)
    {
        if ((swizzleMask & Gfx10Rsrc2dSwModeMask) == 0)
        {
            {
                ADDR_ASSERT_ALWAYS();
                valid = FALSE;
            }
        }
        else if ((prt && ((swizzleMask & Gfx10Rsrc2dPrtSwModeMask) == 0)) ||
                 (fmask && ((swizzleMask & Gfx10ZSwModeMask) == 0)))
        {
            ADDR_ASSERT_ALWAYS();
            valid = FALSE;
        }
    }
    else if (tex3d)
    {
        if (((swizzleMask & Gfx10Rsrc3dSwModeMask) == 0) ||
            (prt && ((swizzleMask & Gfx10Rsrc3dPrtSwModeMask) == 0)) ||
            (thin3d && ((swizzleMask & Gfx10Rsrc3dThinSwModeMask) == 0)))
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
        {
            ADDR_ASSERT_ALWAYS();
            valid = FALSE;
        }
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
    else if (blkVar)
    {
        if (m_blockVarSizeLog2 == 0)
        {
            ADDR_ASSERT_ALWAYS();
            valid = FALSE;
        }
    }

    return valid;
}

/**
************************************************************************************************************************
*   Gfx10Lib::HwlComputeSurfaceInfoSanityCheck
*
*   @brief
*       Compute surface info sanity check
*
*   @return
*       Offset
************************************************************************************************************************
*/
ADDR_E_RETURNCODE Gfx10Lib::HwlComputeSurfaceInfoSanityCheck(
    const ADDR2_COMPUTE_SURFACE_INFO_INPUT* pIn     ///< [in] input structure
    ) const
{
    return ValidateNonSwModeParams(pIn) && ValidateSwModeParams(pIn) ? ADDR_OK : ADDR_INVALIDPARAMS;
}

/**
************************************************************************************************************************
*   Gfx10Lib::HwlGetPreferredSurfaceSetting
*
*   @brief
*       Internal function to get suggested surface information for client to use
*
*   @return
*       ADDR_E_RETURNCODE
************************************************************************************************************************
*/
ADDR_E_RETURNCODE Gfx10Lib::HwlGetPreferredSurfaceSetting(
    const ADDR2_GET_PREFERRED_SURF_SETTING_INPUT* pIn,  ///< [in] input structure
    ADDR2_GET_PREFERRED_SURF_SETTING_OUTPUT*      pOut  ///< [out] output structure
    ) const
{
    ADDR_E_RETURNCODE returnCode = ADDR_OK;

    if (pIn->flags.fmask)
    {
        const BOOL_32 forbid64KbBlockType = pIn->forbiddenBlock.macroThin64KB ? TRUE : FALSE;
        const BOOL_32 forbidVarBlockType  = ((m_blockVarSizeLog2 == 0) || (pIn->forbiddenBlock.var != 0));

        if (forbid64KbBlockType && forbidVarBlockType)
        {
            // Invalid combination...
            ADDR_ASSERT_ALWAYS();
            returnCode = ADDR_INVALIDPARAMS;
        }
        else
        {
            pOut->resourceType                   = ADDR_RSRC_TEX_2D;
            pOut->validBlockSet.value            = 0;
            pOut->validBlockSet.macroThin64KB    = forbid64KbBlockType ? 0 : 1;
            pOut->validBlockSet.var              = forbidVarBlockType  ? 0 : 1;
            pOut->validSwModeSet.value           = 0;
            pOut->validSwModeSet.sw64KB_Z_X      = forbid64KbBlockType ? 0 : 1;
            pOut->validSwModeSet.gfx10.swVar_Z_X = forbidVarBlockType  ? 0 : 1;
            pOut->canXor                         = TRUE;
            pOut->validSwTypeSet.value           = AddrSwSetZ;
            pOut->clientPreferredSwSet           = pOut->validSwTypeSet;

            BOOL_32 use64KbBlockType = (forbid64KbBlockType == FALSE);

            if ((forbid64KbBlockType == FALSE) && (forbidVarBlockType == FALSE))
            {
                const UINT_8  maxFmaskSwizzleModeType = 2;
                const UINT_32 ratioLow                = pIn->flags.minimizeAlign ? 1 : (pIn->flags.opt4space ? 3 : 2);
                const UINT_32 ratioHi                 = pIn->flags.minimizeAlign ? 1 : (pIn->flags.opt4space ? 2 : 1);
                const UINT_32 fmaskBpp                = GetFmaskBpp(pIn->numSamples, pIn->numFrags);
                const UINT_32 numSlices               = Max(pIn->numSlices, 1u);
                const UINT_32 width                   = Max(pIn->width, 1u);
                const UINT_32 height                  = Max(pIn->height, 1u);
                const UINT_64 sizeAlignInElement      = Max(NextPow2(pIn->minSizeAlign) / (fmaskBpp >> 3), 1u);

                AddrSwizzleMode swMode[maxFmaskSwizzleModeType]  = {ADDR_SW_64KB_Z_X, ADDR_SW_VAR_Z_X};
                Dim3d           blkDim[maxFmaskSwizzleModeType]  = {{}, {}};
                Dim3d           padDim[maxFmaskSwizzleModeType]  = {{}, {}};
                UINT_64         padSize[maxFmaskSwizzleModeType] = {};

                for (UINT_8 i = 0; i < maxFmaskSwizzleModeType; i++)
                {
                    ComputeBlockDimensionForSurf(&blkDim[i].w,
                                                 &blkDim[i].h,
                                                 &blkDim[i].d,
                                                 fmaskBpp,
                                                 1,
                                                 pOut->resourceType,
                                                 swMode[i]);

                    padSize[i] = ComputePadSize(&blkDim[i], width, height, numSlices, &padDim[i]);
                    padSize[i] = PowTwoAlign(padSize[i], sizeAlignInElement);
                }

                if (Addr2BlockTypeWithinMemoryBudget(padSize[0],
                                                padSize[1],
                                                ratioLow,
                                                ratioHi,
                                                pIn->memoryBudget,
                                                GetBlockSizeLog2(swMode[1]) >= GetBlockSizeLog2(swMode[0])))
                {
                    use64KbBlockType = FALSE;
                }
            }
            else if (forbidVarBlockType)
            {
                use64KbBlockType = TRUE;
            }

            if (use64KbBlockType)
            {
                pOut->swizzleMode = ADDR_SW_64KB_Z_X;
            }
            else
            {
                pOut->swizzleMode = ADDR_SW_VAR_Z_X;
            }
        }
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
        const UINT_32 numFrags     = (pIn->numFrags == 0) ? numSamples : pIn->numFrags;
        const BOOL_32 msaa         = (numFrags > 1) || (numSamples > 1);

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
        localIn.numFrags     = numFrags;

        if (ValidateNonSwModeParams(&localIn))
        {
            // Forbid swizzle mode(s) by client setting
            ADDR2_SWMODE_SET allowedSwModeSet = {};
            allowedSwModeSet.value |= pIn->forbiddenBlock.linear ? 0 : Gfx10LinearSwModeMask;
            allowedSwModeSet.value |= pIn->forbiddenBlock.micro  ? 0 : Gfx10Blk256BSwModeMask;
            allowedSwModeSet.value |=
                pIn->forbiddenBlock.macroThin4KB ? 0 :
                ((pIn->resourceType == ADDR_RSRC_TEX_3D) ? 0 : Gfx10Blk4KBSwModeMask);
            allowedSwModeSet.value |=
                pIn->forbiddenBlock.macroThick4KB ? 0 :
                ((pIn->resourceType == ADDR_RSRC_TEX_3D) ? Gfx10Rsrc3dThick4KBSwModeMask : 0);
            allowedSwModeSet.value |=
                pIn->forbiddenBlock.macroThin64KB ? 0 :
                ((pIn->resourceType == ADDR_RSRC_TEX_3D) ? Gfx10Rsrc3dThin64KBSwModeMask : Gfx10Blk64KBSwModeMask);
            allowedSwModeSet.value |=
                pIn->forbiddenBlock.macroThick64KB ? 0 :
                ((pIn->resourceType == ADDR_RSRC_TEX_3D) ? Gfx10Rsrc3dThick64KBSwModeMask : 0);
            allowedSwModeSet.value |=
                pIn->forbiddenBlock.var ? 0 : (m_blockVarSizeLog2 ? Gfx10BlkVarSwModeMask : 0);

            if (pIn->preferredSwSet.value != 0)
            {
                allowedSwModeSet.value &= pIn->preferredSwSet.sw_Z ? ~0 : ~Gfx10ZSwModeMask;
                allowedSwModeSet.value &= pIn->preferredSwSet.sw_S ? ~0 : ~Gfx10StandardSwModeMask;
                allowedSwModeSet.value &= pIn->preferredSwSet.sw_D ? ~0 : ~Gfx10DisplaySwModeMask;
                allowedSwModeSet.value &= pIn->preferredSwSet.sw_R ? ~0 : ~Gfx10RenderSwModeMask;
            }

            if (pIn->noXor)
            {
                allowedSwModeSet.value &= ~Gfx10XorSwModeMask;
            }

            if (pIn->maxAlign > 0)
            {
                if (pIn->maxAlign < (1u << m_blockVarSizeLog2))
                {
                    allowedSwModeSet.value &= ~Gfx10BlkVarSwModeMask;
                }

                if (pIn->maxAlign < Size64K)
                {
                    allowedSwModeSet.value &= ~Gfx10Blk64KBSwModeMask;
                }

                if (pIn->maxAlign < Size4K)
                {
                    allowedSwModeSet.value &= ~Gfx10Blk4KBSwModeMask;
                }

                if (pIn->maxAlign < Size256)
                {
                    allowedSwModeSet.value &= ~Gfx10Blk256BSwModeMask;
                }
            }

            // Filter out invalid swizzle mode(s) by image attributes and HW restrictions
            switch (pIn->resourceType)
            {
                case ADDR_RSRC_TEX_1D:
                    allowedSwModeSet.value &= Gfx10Rsrc1dSwModeMask;
                    break;

                case ADDR_RSRC_TEX_2D:
                    allowedSwModeSet.value &= pIn->flags.prt ? Gfx10Rsrc2dPrtSwModeMask : Gfx10Rsrc2dSwModeMask;
                    break;

                case ADDR_RSRC_TEX_3D:
                    allowedSwModeSet.value &= pIn->flags.prt ? Gfx10Rsrc3dPrtSwModeMask : Gfx10Rsrc3dSwModeMask;

                    if (pIn->flags.view3dAs2dArray)
                    {
                        allowedSwModeSet.value &= Gfx10Rsrc3dThinSwModeMask;
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
                allowedSwModeSet.value &= ~Gfx10ZSwModeMask;
            }

            if (pIn->format == ADDR_FMT_32_32_32)
            {
                allowedSwModeSet.value &= Gfx10LinearSwModeMask;
            }

            if (msaa)
            {
                allowedSwModeSet.value &= Gfx10MsaaSwModeMask;
            }

            if (pIn->flags.depth || pIn->flags.stencil)
            {
                allowedSwModeSet.value &= Gfx10ZSwModeMask;
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
                pOut->canXor         = (allowedSwModeSet.value & Gfx10XorSwModeMask) ? TRUE : FALSE;
                pOut->validBlockSet  = GetAllowedBlockSet(allowedSwModeSet, pOut->resourceType);
                pOut->validSwTypeSet = GetAllowedSwSet(allowedSwModeSet);

                pOut->clientPreferredSwSet = pIn->preferredSwSet;

                if (pOut->clientPreferredSwSet.value == 0)
                {
                    pOut->clientPreferredSwSet.value = AddrSwSetAll;
                }

                // Apply optional restrictions
                if ((pIn->flags.depth || pIn->flags.stencil) && msaa && m_configFlags.nonPower2MemConfig)
                {
                    if ((allowedSwModeSet.value &= ~Gfx10BlkVarSwModeMask) != 0)
                    {
                        // MSAA depth in non power of 2 memory configs would suffer from non-local channel accesses from
                        // the GL2 in VAR mode, so it should be avoided.
                        allowedSwModeSet.value &= ~Gfx10BlkVarSwModeMask;
                    }
                    else
                    {
                        // We should still be able to use VAR for non power of 2 memory configs with MSAA z/stencil.
                        // But we have to suffer from low performance because there is no other choice...
                        ADDR_ASSERT_ALWAYS();
                    }
                }

                if (pIn->flags.needEquation)
                {
                    UINT_32 components = pIn->flags.allowExtEquation ?  ADDR_MAX_EQUATION_COMP :
                                                                        ADDR_MAX_LEGACY_EQUATION_COMP;
                    FilterInvalidEqSwizzleMode(allowedSwModeSet, pIn->resourceType, Log2(bpp >> 3), components);
                }

                if (allowedSwModeSet.value == Gfx10LinearSwModeMask)
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

                    // A bitfield where each bit represents a block type. Each swizzle mode maps to a block.
                    ADDR2_BLOCK_SET allowedBlockSet = GetAllowedBlockSet(allowedSwModeSet, pOut->resourceType);

                    // Determine block size if there are 2 or more block type candidates
                    if (IsPow2(allowedBlockSet.value) == FALSE)
                    {
                        // Tracks a valid SwizzleMode for each valid block type
                        AddrSwizzleMode swMode[AddrBlockMaxTiledType] = {};

                        swMode[AddrBlockLinear] = ADDR_SW_LINEAR;

                        if (m_blockVarSizeLog2 != 0)
                        {
                            swMode[AddrBlockThinVar] = ADDR_SW_VAR_R_X;
                        }

                        if (pOut->resourceType == ADDR_RSRC_TEX_3D)
                        {
                            swMode[AddrBlockThick4KB]  = ADDR_SW_4KB_S;
                            swMode[AddrBlockThin64KB]  = ADDR_SW_64KB_R_X;
                            swMode[AddrBlockThick64KB] = ADDR_SW_64KB_S;
                        }
                        else
                        {
                            swMode[AddrBlockMicro]    = ADDR_SW_256B_S;
                            swMode[AddrBlockThin4KB]  = ADDR_SW_4KB_S;
                            swMode[AddrBlockThin64KB] = ADDR_SW_64KB_S;
                        }

                        // Tracks the size of each valid swizzle mode's surface in bytes
                        UINT_64 padSize[AddrBlockMaxTiledType] = {};

                        const UINT_32 ratioLow           = computeMinSize ? 1 : (pIn->flags.opt4space ? 3 : 2);
                        const UINT_32 ratioHi            = computeMinSize ? 1 : (pIn->flags.opt4space ? 2 : 1);
                        const UINT_64 sizeAlignInElement = Max(NextPow2(pIn->minSizeAlign) / (bpp >> 3), 1u);
                        UINT_32       minSizeBlk         = AddrBlockMicro; // Tracks the most optimal block to use
                        UINT_64       minSize            = 0;              // Tracks the minimum acceptable block type

                        ADDR2_COMPUTE_SURFACE_INFO_OUTPUT localOut = {};

                        // Iterate through all block types
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

                                    if (minSize == 0)
                                    {
                                        minSize    = padSize[i];
                                        minSizeBlk = i;
                                    }
                                    else
                                    {
                                        // Checks if the block type is within the memory budget but favors larger blocks
                                        if (Addr2BlockTypeWithinMemoryBudget(
                                                minSize,
                                                padSize[i],
                                                ratioLow,
                                                ratioHi,
                                                0.0,
                                                GetBlockSizeLog2(swMode[i]) >= GetBlockSizeLog2(swMode[minSizeBlk])))
                                        {
                                            minSize    = padSize[i];
                                            minSizeBlk = i;
                                        }
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
                                case AddrBlockThick64KB:
                                    allowedBlockSet.macroThin64KB = 0;
                                case AddrBlockThinVar:
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
                                    if (Addr2BlockTypeWithinMemoryBudget(
                                            minSize,
                                            padSize[i],
                                            0,
                                            0,
                                            pIn->memoryBudget,
                                            GetBlockSizeLog2(swMode[i]) >= GetBlockSizeLog2(swMode[minSizeBlk])) == FALSE)
                                    {
                                        // Clear the block type if the memory waste is unacceptable
                                        allowedBlockSet.value &= ~(1u << (i - 1));
                                    }
                                }
                            }

                            // Remove VAR block type if bigger block type is allowed
                            if (GetBlockSizeLog2(swMode[AddrBlockThinVar]) < GetBlockSizeLog2(ADDR_SW_64KB_R_X))
                            {
                                if (allowedBlockSet.macroThick64KB || allowedBlockSet.macroThin64KB)
                                {
                                    allowedBlockSet.var = 0;
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
                                allowedSwModeSet.value &= Gfx10LinearSwModeMask;
                                break;

                            case AddrBlockMicro:
                                ADDR_ASSERT(pOut->resourceType != ADDR_RSRC_TEX_3D);
                                allowedSwModeSet.value &= Gfx10Blk256BSwModeMask;
                                break;

                            case AddrBlockThin4KB:
                                ADDR_ASSERT(pOut->resourceType != ADDR_RSRC_TEX_3D);
                                allowedSwModeSet.value &= Gfx10Blk4KBSwModeMask;
                                break;

                            case AddrBlockThick4KB:
                                ADDR_ASSERT(pOut->resourceType == ADDR_RSRC_TEX_3D);
                                allowedSwModeSet.value &= Gfx10Rsrc3dThick4KBSwModeMask;
                                break;

                            case AddrBlockThin64KB:
                                allowedSwModeSet.value &= (pOut->resourceType == ADDR_RSRC_TEX_3D) ?
                                                          Gfx10Rsrc3dThin64KBSwModeMask : Gfx10Blk64KBSwModeMask;
                                break;

                            case AddrBlockThick64KB:
                                ADDR_ASSERT(pOut->resourceType == ADDR_RSRC_TEX_3D);
                                allowedSwModeSet.value &= Gfx10Rsrc3dThick64KBSwModeMask;
                                break;

                            case AddrBlockThinVar:
                                allowedSwModeSet.value &= Gfx10BlkVarSwModeMask;
                                break;

                            default:
                                ADDR_ASSERT_ALWAYS();
                                allowedSwModeSet.value = 0;
                                break;
                        }
                    }

                    // Block type should be determined.
                    ADDR_ASSERT(IsPow2(GetAllowedBlockSet(allowedSwModeSet, pOut->resourceType).value));

                    ADDR2_SWTYPE_SET allowedSwSet = GetAllowedSwSet(allowedSwModeSet);

                    // Determine swizzle type if there are 2 or more swizzle type candidates
                    if ((allowedSwSet.value != 0) && (IsPow2(allowedSwSet.value) == FALSE))
                    {
                        if (ElemLib::IsBlockCompressed(pIn->format))
                        {
                            if (allowedSwSet.sw_D)
                            {
                                allowedSwModeSet.value &= Gfx10DisplaySwModeMask;
                            }
                            else if (allowedSwSet.sw_S)
                            {
                                allowedSwModeSet.value &= Gfx10StandardSwModeMask;
                            }
                            else
                            {
                                ADDR_ASSERT(allowedSwSet.sw_R);
                                allowedSwModeSet.value &= Gfx10RenderSwModeMask;
                            }
                        }
                        else if (ElemLib::IsMacroPixelPacked(pIn->format))
                        {
                            if (allowedSwSet.sw_S)
                            {
                                allowedSwModeSet.value &= Gfx10StandardSwModeMask;
                            }
                            else if (allowedSwSet.sw_D)
                            {
                                allowedSwModeSet.value &= Gfx10DisplaySwModeMask;
                            }
                            else
                            {
                                ADDR_ASSERT(allowedSwSet.sw_R);
                                allowedSwModeSet.value &= Gfx10RenderSwModeMask;
                            }
                        }
                        else if (pIn->resourceType == ADDR_RSRC_TEX_3D)
                        {
                            if (pIn->flags.color &&
                                GetAllowedBlockSet(allowedSwModeSet, pOut->resourceType).macroThick64KB &&
                                allowedSwSet.sw_D)
                            {
                                allowedSwModeSet.value &= Gfx10DisplaySwModeMask;
                            }
                            else if (allowedSwSet.sw_S)
                            {
                                allowedSwModeSet.value &= Gfx10StandardSwModeMask;
                            }
                            else if (allowedSwSet.sw_R)
                            {
                                allowedSwModeSet.value &= Gfx10RenderSwModeMask;
                            }
                            else
                            {
                                ADDR_ASSERT(allowedSwSet.sw_Z);
                                allowedSwModeSet.value &= Gfx10ZSwModeMask;
                            }
                        }
                        else
                        {
                            if (allowedSwSet.sw_R)
                            {
                                allowedSwModeSet.value &= Gfx10RenderSwModeMask;
                            }
                            else if (allowedSwSet.sw_D)
                            {
                                allowedSwModeSet.value &= Gfx10DisplaySwModeMask;
                            }
                            else if (allowedSwSet.sw_S)
                            {
                                allowedSwModeSet.value &= Gfx10StandardSwModeMask;
                            }
                            else
                            {
                                ADDR_ASSERT(allowedSwSet.sw_Z);
                                allowedSwModeSet.value &= Gfx10ZSwModeMask;
                            }
                        }

                        // Swizzle type should be determined.
                        ADDR_ASSERT(IsPow2(GetAllowedSwSet(allowedSwModeSet).value));
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
*   Gfx10Lib::ComputeStereoInfo
*
*   @brief
*       Compute height alignment and right eye pipeBankXor for stereo surface
*
*   @return
*       Error code
*
************************************************************************************************************************
*/
ADDR_E_RETURNCODE Gfx10Lib::ComputeStereoInfo(
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
*   Gfx10Lib::HwlComputeSurfaceInfoTiled
*
*   @brief
*       Internal function to calculate alignment for tiled surface
*
*   @return
*       ADDR_E_RETURNCODE
************************************************************************************************************************
*/
ADDR_E_RETURNCODE Gfx10Lib::HwlComputeSurfaceInfoTiled(
     const ADDR2_COMPUTE_SURFACE_INFO_INPUT* pIn,    ///< [in] input structure
     ADDR2_COMPUTE_SURFACE_INFO_OUTPUT*      pOut    ///< [out] output structure
     ) const
{
    ADDR_E_RETURNCODE ret;

    // Mip chain dimesion and epitch has no meaning in GFX10, set to default value
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
*   Gfx10Lib::ComputeSurfaceInfoMicroTiled
*
*   @brief
*       Internal function to calculate alignment for micro tiled surface
*
*   @return
*       ADDR_E_RETURNCODE
************************************************************************************************************************
*/
ADDR_E_RETURNCODE Gfx10Lib::ComputeSurfaceInfoMicroTiled(
     const ADDR2_COMPUTE_SURFACE_INFO_INPUT* pIn,    ///< [in] input structure
     ADDR2_COMPUTE_SURFACE_INFO_OUTPUT*      pOut    ///< [out] output structure
     ) const
{
    ADDR_E_RETURNCODE ret = ComputeBlockDimensionForSurf(&pOut->blockWidth,
                                                         &pOut->blockHeight,
                                                         &pOut->blockSlices,
                                                         pIn->bpp,
                                                         pIn->numFrags,
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
*   Gfx10Lib::ComputeSurfaceInfoMacroTiled
*
*   @brief
*       Internal function to calculate alignment for macro tiled surface
*
*   @return
*       ADDR_E_RETURNCODE
************************************************************************************************************************
*/
ADDR_E_RETURNCODE Gfx10Lib::ComputeSurfaceInfoMacroTiled(
     const ADDR2_COMPUTE_SURFACE_INFO_INPUT* pIn,    ///< [in] input structure
     ADDR2_COMPUTE_SURFACE_INFO_OUTPUT*      pOut    ///< [out] output structure
     ) const
{
    ADDR_E_RETURNCODE returnCode = ComputeBlockDimensionForSurf(&pOut->blockWidth,
                                                                &pOut->blockHeight,
                                                                &pOut->blockSlices,
                                                                pIn->bpp,
                                                                pIn->numFrags,
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

                ADDR_ASSERT(pIn->numMipLevels <= MaxMipLevels);
                Dim3d fixedTailMaxDim = tailMaxDim;

                if (m_settings.dsMipmapHtileFix && IsZOrderSwizzle(pIn->swizzleMode) && (index <= 1))
                {
                    fixedTailMaxDim.w /= Block256_2d[index].w / Block256_2d[2].w;
                    fixedTailMaxDim.h /= Block256_2d[index].h / Block256_2d[2].h;
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
                            pOut->pMipInfo[i].depth  = IsTex3d(pIn->resourceType) ? pOut->numSlices : 1;
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
                        pOut->pMipInfo[i].depth  = IsTex3d(pIn->resourceType) ? pOut->numSlices : 1;

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
                        }
                        else
                        {
                            pOut->pMipInfo[i].mipTailCoordX = mipX * Block256_3d[index].w;
                            pOut->pMipInfo[i].mipTailCoordY = mipY * Block256_3d[index].h;
                            pOut->pMipInfo[i].mipTailCoordZ = 0;

                            pitch  = Max(pitch  >> 1, Block256_3d[index].w);
                            height = Max(height >> 1, Block256_3d[index].h);
                        }
                    }
                }
            }
            else
            {
                pOut->sliceSize = static_cast<UINT_64>(pOut->pitch) * pOut->height * (pIn->bpp >> 3) * pIn->numFrags;
                pOut->surfSize  = pOut->sliceSize * pOut->numSlices;

                if (pOut->pMipInfo != NULL)
                {
                    pOut->pMipInfo[0].pitch            = pOut->pitch;
                    pOut->pMipInfo[0].height           = pOut->height;
                    pOut->pMipInfo[0].depth            = IsTex3d(pIn->resourceType) ? pOut->numSlices : 1;
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
*   Gfx10Lib::HwlComputeSurfaceAddrFromCoordTiled
*
*   @brief
*       Internal function to calculate address from coord for tiled swizzle surface
*
*   @return
*       ADDR_E_RETURNCODE
************************************************************************************************************************
*/
ADDR_E_RETURNCODE Gfx10Lib::HwlComputeSurfaceAddrFromCoordTiled(
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
*   Gfx10Lib::ComputeOffsetFromEquation
*
*   @brief
*       Compute offset from equation
*
*   @return
*       Offset
************************************************************************************************************************
*/
UINT_32 Gfx10Lib::ComputeOffsetFromEquation(
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
*   Gfx10Lib::ComputeOffsetFromSwizzlePattern
*
*   @brief
*       Compute offset from swizzle pattern
*
*   @return
*       Offset
************************************************************************************************************************
*/
UINT_32 Gfx10Lib::ComputeOffsetFromSwizzlePattern(
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
*   Gfx10Lib::GetSwizzlePatternInfo
*
*   @brief
*       Get swizzle pattern
*
*   @return
*       Swizzle pattern information
************************************************************************************************************************
*/
const ADDR_SW_PATINFO* Gfx10Lib::GetSwizzlePatternInfo(
    AddrSwizzleMode  swizzleMode,       ///< Swizzle mode
    AddrResourceType resourceType,      ///< Resource type
    UINT_32          elemLog2,          ///< Element size in bytes log2
    UINT_32          numFrag            ///< Number of fragment
    ) const
{
    // Now elemLog2 is going to be used to access the correct index insode of the pPatInfo array so we will start from
    // the right location
    const UINT_32          index       = IsXor(swizzleMode) ? (m_colorBaseIndex + elemLog2) : elemLog2;
    const ADDR_SW_PATINFO* patInfo     = NULL;
    const UINT_32          swizzleMask = 1 << swizzleMode;

    if (IsBlockVariable(swizzleMode))
    {
        if (m_blockVarSizeLog2 != 0)
        {
            ADDR_ASSERT(m_settings.supportRbPlus);

            if (IsRtOptSwizzle(swizzleMode))
            {
                if (numFrag == 1)
                {
                    patInfo = GFX10_SW_VAR_R_X_1xaa_RBPLUS_PATINFO;
                }
                else if (numFrag == 2)
                {
                    patInfo = GFX10_SW_VAR_R_X_2xaa_RBPLUS_PATINFO;
                }
                else if (numFrag == 4)
                {
                    patInfo = GFX10_SW_VAR_R_X_4xaa_RBPLUS_PATINFO;
                }
                else
                {
                    ADDR_ASSERT(numFrag == 8);
                    patInfo = GFX10_SW_VAR_R_X_8xaa_RBPLUS_PATINFO;
                }
            }
            else if (IsZOrderSwizzle(swizzleMode))
            {
                if (numFrag == 1)
                {
                    patInfo = GFX10_SW_VAR_Z_X_1xaa_RBPLUS_PATINFO;
                }
                else if (numFrag == 2)
                {
                    patInfo = GFX10_SW_VAR_Z_X_2xaa_RBPLUS_PATINFO;
                }
                else if (numFrag == 4)
                {
                    patInfo = GFX10_SW_VAR_Z_X_4xaa_RBPLUS_PATINFO;
                }
                else
                {
                    ADDR_ASSERT(numFrag == 8);
                    patInfo = GFX10_SW_VAR_Z_X_8xaa_RBPLUS_PATINFO;
                }
            }
        }
    }
    else if (IsLinear(swizzleMode) == FALSE)
    {
        if (resourceType == ADDR_RSRC_TEX_3D)
        {
            ADDR_ASSERT(numFrag == 1);

            if ((swizzleMask & Gfx10Rsrc3dSwModeMask) != 0)
            {
                if (IsRtOptSwizzle(swizzleMode))
                {
                    if (swizzleMode == ADDR_SW_4KB_R_X)
                    {
                        patInfo = NULL;
                    }
                    else
                    {
                        patInfo = m_settings.supportRbPlus ?
                                  GFX10_SW_64K_R_X_1xaa_RBPLUS_PATINFO : GFX10_SW_64K_R_X_1xaa_PATINFO;
                    }
                }
                else if (IsZOrderSwizzle(swizzleMode))
                {
                    patInfo = m_settings.supportRbPlus ?
                              GFX10_SW_64K_Z_X_1xaa_RBPLUS_PATINFO : GFX10_SW_64K_Z_X_1xaa_PATINFO;
                }
                else if (IsDisplaySwizzle(resourceType, swizzleMode))
                {
                    ADDR_ASSERT(swizzleMode == ADDR_SW_64KB_D_X);
                    patInfo = m_settings.supportRbPlus ?
                              GFX10_SW_64K_D3_X_RBPLUS_PATINFO : GFX10_SW_64K_D3_X_PATINFO;
                }
                else
                {
                    ADDR_ASSERT(IsStandardSwizzle(resourceType, swizzleMode));

                    if (IsBlock4kb(swizzleMode))
                    {
                        if (swizzleMode == ADDR_SW_4KB_S)
                        {
                            patInfo = m_settings.supportRbPlus ?
                                      GFX10_SW_4K_S3_RBPLUS_PATINFO : GFX10_SW_4K_S3_PATINFO;
                        }
                        else
                        {
                            ADDR_ASSERT(swizzleMode == ADDR_SW_4KB_S_X);
                            patInfo = m_settings.supportRbPlus ?
                                      GFX10_SW_4K_S3_X_RBPLUS_PATINFO : GFX10_SW_4K_S3_X_PATINFO;
                        }
                    }
                    else
                    {
                        if (swizzleMode == ADDR_SW_64KB_S)
                        {
                            patInfo = m_settings.supportRbPlus ?
                                      GFX10_SW_64K_S3_RBPLUS_PATINFO : GFX10_SW_64K_S3_PATINFO;
                        }
                        else if (swizzleMode == ADDR_SW_64KB_S_X)
                        {
                            patInfo = m_settings.supportRbPlus ?
                                      GFX10_SW_64K_S3_X_RBPLUS_PATINFO : GFX10_SW_64K_S3_X_PATINFO;
                        }
                        else
                        {
                            ADDR_ASSERT(swizzleMode == ADDR_SW_64KB_S_T);
                            patInfo = m_settings.supportRbPlus ?
                                      GFX10_SW_64K_S3_T_RBPLUS_PATINFO : GFX10_SW_64K_S3_T_PATINFO;
                        }
                    }
                }
            }
        }
        else
        {
            if ((swizzleMask & Gfx10Rsrc2dSwModeMask) != 0)
            {
                if (IsBlock256b(swizzleMode))
                {
                    if (swizzleMode == ADDR_SW_256B_S)
                    {
                        patInfo = m_settings.supportRbPlus ?
                                  GFX10_SW_256_S_RBPLUS_PATINFO : GFX10_SW_256_S_PATINFO;
                    }
                    else
                    {
                        ADDR_ASSERT(swizzleMode == ADDR_SW_256B_D);
                        patInfo = m_settings.supportRbPlus ?
                                  GFX10_SW_256_D_RBPLUS_PATINFO : GFX10_SW_256_D_PATINFO;
                    }
                }
                else if (IsBlock4kb(swizzleMode))
                {
                    if (IsStandardSwizzle(resourceType, swizzleMode))
                    {
                        if (swizzleMode == ADDR_SW_4KB_S)
                        {
                            patInfo = m_settings.supportRbPlus ?
                                      GFX10_SW_4K_S_RBPLUS_PATINFO : GFX10_SW_4K_S_PATINFO;
                        }
                        else
                        {
                            ADDR_ASSERT(swizzleMode == ADDR_SW_4KB_S_X);
                            patInfo = m_settings.supportRbPlus ?
                                      GFX10_SW_4K_S_X_RBPLUS_PATINFO : GFX10_SW_4K_S_X_PATINFO;
                        }
                    }
                    else
                    {
                        if (swizzleMode == ADDR_SW_4KB_D)
                        {
                            patInfo = m_settings.supportRbPlus ?
                                      GFX10_SW_4K_D_RBPLUS_PATINFO : GFX10_SW_4K_D_PATINFO;
                        }
                        else if (swizzleMode == ADDR_SW_4KB_R_X)
                        {
                            patInfo = NULL;
                        }
                        else
                        {
                            ADDR_ASSERT(swizzleMode == ADDR_SW_4KB_D_X);
                            patInfo = m_settings.supportRbPlus ?
                                      GFX10_SW_4K_D_X_RBPLUS_PATINFO : GFX10_SW_4K_D_X_PATINFO;
                        }
                    }
                }
                else
                {
                    if (IsRtOptSwizzle(swizzleMode))
                    {
                        if (numFrag == 1)
                        {
                            patInfo = m_settings.supportRbPlus ?
                                      GFX10_SW_64K_R_X_1xaa_RBPLUS_PATINFO : GFX10_SW_64K_R_X_1xaa_PATINFO;
                        }
                        else if (numFrag == 2)
                        {
                            patInfo = m_settings.supportRbPlus ?
                                      GFX10_SW_64K_R_X_2xaa_RBPLUS_PATINFO : GFX10_SW_64K_R_X_2xaa_PATINFO;
                        }
                        else if (numFrag == 4)
                        {
                            patInfo = m_settings.supportRbPlus ?
                                      GFX10_SW_64K_R_X_4xaa_RBPLUS_PATINFO : GFX10_SW_64K_R_X_4xaa_PATINFO;
                        }
                        else
                        {
                            ADDR_ASSERT(numFrag == 8);
                            patInfo = m_settings.supportRbPlus ?
                                      GFX10_SW_64K_R_X_8xaa_RBPLUS_PATINFO : GFX10_SW_64K_R_X_8xaa_PATINFO;
                        }
                    }
                    else if (IsZOrderSwizzle(swizzleMode))
                    {
                        if (numFrag == 1)
                        {
                            patInfo = m_settings.supportRbPlus ?
                                      GFX10_SW_64K_Z_X_1xaa_RBPLUS_PATINFO : GFX10_SW_64K_Z_X_1xaa_PATINFO;
                        }
                        else if (numFrag == 2)
                        {
                            patInfo = m_settings.supportRbPlus ?
                                      GFX10_SW_64K_Z_X_2xaa_RBPLUS_PATINFO : GFX10_SW_64K_Z_X_2xaa_PATINFO;
                        }
                        else if (numFrag == 4)
                        {
                            patInfo = m_settings.supportRbPlus ?
                                      GFX10_SW_64K_Z_X_4xaa_RBPLUS_PATINFO : GFX10_SW_64K_Z_X_4xaa_PATINFO;
                        }
                        else
                        {
                            ADDR_ASSERT(numFrag == 8);
                            patInfo = m_settings.supportRbPlus ?
                                      GFX10_SW_64K_Z_X_8xaa_RBPLUS_PATINFO : GFX10_SW_64K_Z_X_8xaa_PATINFO;
                        }
                    }
                    else if (IsDisplaySwizzle(resourceType, swizzleMode))
                    {
                        if (swizzleMode == ADDR_SW_64KB_D)
                        {
                            patInfo = m_settings.supportRbPlus ?
                                      GFX10_SW_64K_D_RBPLUS_PATINFO : GFX10_SW_64K_D_PATINFO;
                        }
                        else if (swizzleMode == ADDR_SW_64KB_D_X)
                        {
                            patInfo = m_settings.supportRbPlus ?
                                      GFX10_SW_64K_D_X_RBPLUS_PATINFO : GFX10_SW_64K_D_X_PATINFO;
                        }
                        else
                        {
                            ADDR_ASSERT(swizzleMode == ADDR_SW_64KB_D_T);
                            patInfo = m_settings.supportRbPlus ?
                                      GFX10_SW_64K_D_T_RBPLUS_PATINFO : GFX10_SW_64K_D_T_PATINFO;
                        }
                    }
                    else
                    {
                        if (swizzleMode == ADDR_SW_64KB_S)
                        {
                            patInfo = m_settings.supportRbPlus ?
                                      GFX10_SW_64K_S_RBPLUS_PATINFO : GFX10_SW_64K_S_PATINFO;
                        }
                        else if (swizzleMode == ADDR_SW_64KB_S_X)
                        {
                            patInfo = m_settings.supportRbPlus ?
                                      GFX10_SW_64K_S_X_RBPLUS_PATINFO : GFX10_SW_64K_S_X_PATINFO;
                        }
                        else
                        {
                            ADDR_ASSERT(swizzleMode == ADDR_SW_64KB_S_T);
                            patInfo = m_settings.supportRbPlus ?
                                      GFX10_SW_64K_S_T_RBPLUS_PATINFO : GFX10_SW_64K_S_T_PATINFO;
                        }
                    }
                }
            }
        }
    }

    return (patInfo != NULL) ? &patInfo[index] : NULL;
}

/**
************************************************************************************************************************
*   Gfx10Lib::ComputeSurfaceAddrFromCoordMicroTiled
*
*   @brief
*       Internal function to calculate address from coord for micro tiled swizzle surface
*
*   @return
*       ADDR_E_RETURNCODE
************************************************************************************************************************
*/
ADDR_E_RETURNCODE Gfx10Lib::ComputeSurfaceAddrFromCoordMicroTiled(
     const ADDR2_COMPUTE_SURFACE_ADDRFROMCOORD_INPUT* pIn,    ///< [in] input structure
     ADDR2_COMPUTE_SURFACE_ADDRFROMCOORD_OUTPUT*      pOut    ///< [out] output structure
     ) const
{
    ADDR2_COMPUTE_SURFACE_INFO_INPUT  localIn  = {};
    ADDR2_COMPUTE_SURFACE_INFO_OUTPUT localOut = {};
    ADDR2_MIP_INFO                    mipInfo[MaxMipLevels];
    ADDR_ASSERT(pIn->numMipLevels <= MaxMipLevels);

    localIn.swizzleMode  = pIn->swizzleMode;
    localIn.flags        = pIn->flags;
    localIn.resourceType = pIn->resourceType;
    localIn.bpp          = pIn->bpp;
    localIn.width        = Max(pIn->unalignedWidth,  1u);
    localIn.height       = Max(pIn->unalignedHeight, 1u);
    localIn.numSlices    = Max(pIn->numSlices,       1u);
    localIn.numMipLevels = Max(pIn->numMipLevels,    1u);
    localIn.numSamples   = Max(pIn->numSamples,      1u);
    localIn.numFrags     = Max(pIn->numFrags,        1u);
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
*   Gfx10Lib::ComputeSurfaceAddrFromCoordMacroTiled
*
*   @brief
*       Internal function to calculate address from coord for macro tiled swizzle surface
*
*   @return
*       ADDR_E_RETURNCODE
************************************************************************************************************************
*/
ADDR_E_RETURNCODE Gfx10Lib::ComputeSurfaceAddrFromCoordMacroTiled(
     const ADDR2_COMPUTE_SURFACE_ADDRFROMCOORD_INPUT* pIn,    ///< [in] input structure
     ADDR2_COMPUTE_SURFACE_ADDRFROMCOORD_OUTPUT*      pOut    ///< [out] output structure
     ) const
{
    ADDR2_COMPUTE_SURFACE_INFO_INPUT  localIn  = {};
    ADDR2_COMPUTE_SURFACE_INFO_OUTPUT localOut = {};
    ADDR2_MIP_INFO                    mipInfo[MaxMipLevels];
    ADDR_ASSERT(pIn->numMipLevels <= MaxMipLevels);

    localIn.swizzleMode  = pIn->swizzleMode;
    localIn.flags        = pIn->flags;
    localIn.resourceType = pIn->resourceType;
    localIn.bpp          = pIn->bpp;
    localIn.width        = Max(pIn->unalignedWidth,  1u);
    localIn.height       = Max(pIn->unalignedHeight, 1u);
    localIn.numSlices    = Max(pIn->numSlices,       1u);
    localIn.numMipLevels = Max(pIn->numMipLevels,    1u);
    localIn.numSamples   = Max(pIn->numSamples,      1u);
    localIn.numFrags     = Max(pIn->numFrags,        1u);
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

        if (localIn.numFrags > 1)
        {
            const ADDR_SW_PATINFO* pPatInfo = GetSwizzlePatternInfo(pIn->swizzleMode,
                                                                    pIn->resourceType,
                                                                    elemLog2,
                                                                    localIn.numFrags);

            if (pPatInfo != NULL)
            {
                const UINT_32 pb        = localOut.pitch / localOut.blockWidth;
                const UINT_32 yb        = pIn->y / localOut.blockHeight;
                const UINT_32 xb        = pIn->x / localOut.blockWidth;
                const UINT_64 blkIdx    = yb * pb + xb;

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
*   Gfx10Lib::HwlComputeMaxBaseAlignments
*
*   @brief
*       Gets maximum alignments
*   @return
*       maximum alignments
************************************************************************************************************************
*/
UINT_32 Gfx10Lib::HwlComputeMaxBaseAlignments() const
{
    return m_blockVarSizeLog2 ? Max(Size64K, 1u << m_blockVarSizeLog2) : Size64K;
}

/**
************************************************************************************************************************
*   Gfx10Lib::HwlComputeMaxMetaBaseAlignments
*
*   @brief
*       Gets maximum alignments for metadata
*   @return
*       maximum alignments for metadata
************************************************************************************************************************
*/
UINT_32 Gfx10Lib::HwlComputeMaxMetaBaseAlignments() const
{
    Dim3d metaBlk;

    const AddrSwizzleMode ValidSwizzleModeForXmask[] =
    {
        ADDR_SW_64KB_Z_X,
        m_blockVarSizeLog2 ? ADDR_SW_VAR_Z_X : ADDR_SW_64KB_Z_X,
    };

    UINT_32 maxBaseAlignHtile = 0;
    UINT_32 maxBaseAlignCmask = 0;

    for (UINT_32 swIdx = 0; swIdx < sizeof(ValidSwizzleModeForXmask) / sizeof(ValidSwizzleModeForXmask[0]); swIdx++)
    {
        for (UINT_32 bppLog2 = 0; bppLog2 < 3; bppLog2++)
        {
            for (UINT_32 numFragLog2 = 0; numFragLog2 < 4; numFragLog2++)
            {
                // Max base alignment for Htile
                const UINT_32 metaBlkSizeHtile = GetMetaBlkSize(Gfx10DataDepthStencil,
                                                                ADDR_RSRC_TEX_2D,
                                                                ValidSwizzleModeForXmask[swIdx],
                                                                bppLog2,
                                                                numFragLog2,
                                                                TRUE,
                                                                &metaBlk);

                maxBaseAlignHtile = Max(maxBaseAlignHtile, metaBlkSizeHtile);
            }
        }

        // Max base alignment for Cmask
        const UINT_32 metaBlkSizeCmask = GetMetaBlkSize(Gfx10DataFmask,
                                                        ADDR_RSRC_TEX_2D,
                                                        ValidSwizzleModeForXmask[swIdx],
                                                        0,
                                                        0,
                                                        TRUE,
                                                        &metaBlk);

        maxBaseAlignCmask = Max(maxBaseAlignCmask, metaBlkSizeCmask);
    }

    // Max base alignment for 2D Dcc
    const AddrSwizzleMode ValidSwizzleModeForDcc2D[] =
    {
        ADDR_SW_64KB_S_X,
        ADDR_SW_64KB_D_X,
        ADDR_SW_64KB_R_X,
        m_blockVarSizeLog2 ? ADDR_SW_VAR_R_X : ADDR_SW_64KB_R_X,
    };

    UINT_32 maxBaseAlignDcc2D = 0;

    for (UINT_32 swIdx = 0; swIdx < sizeof(ValidSwizzleModeForDcc2D) / sizeof(ValidSwizzleModeForDcc2D[0]); swIdx++)
    {
        for (UINT_32 bppLog2 = 0; bppLog2 < MaxNumOfBpp; bppLog2++)
        {
            for (UINT_32 numFragLog2 = 0; numFragLog2 < 4; numFragLog2++)
            {
                const UINT_32 metaBlkSize2D = GetMetaBlkSize(Gfx10DataColor,
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
        ADDR_SW_64KB_Z_X,
        ADDR_SW_64KB_S_X,
        ADDR_SW_64KB_D_X,
        ADDR_SW_64KB_R_X,
        m_blockVarSizeLog2 ? ADDR_SW_VAR_R_X : ADDR_SW_64KB_R_X,
    };

    UINT_32 maxBaseAlignDcc3D = 0;

    for (UINT_32 swIdx = 0; swIdx < sizeof(ValidSwizzleModeForDcc3D) / sizeof(ValidSwizzleModeForDcc3D[0]); swIdx++)
    {
        for (UINT_32 bppLog2 = 0; bppLog2 < MaxNumOfBpp; bppLog2++)
        {
            const UINT_32 metaBlkSize3D = GetMetaBlkSize(Gfx10DataColor,
                                                         ADDR_RSRC_TEX_3D,
                                                         ValidSwizzleModeForDcc3D[swIdx],
                                                         bppLog2,
                                                         0,
                                                         TRUE,
                                                         &metaBlk);

            maxBaseAlignDcc3D = Max(maxBaseAlignDcc3D, metaBlkSize3D);
        }
    }

    return Max(Max(maxBaseAlignHtile, maxBaseAlignCmask), Max(maxBaseAlignDcc2D, maxBaseAlignDcc3D));
}

/**
************************************************************************************************************************
*   Gfx10Lib::GetMetaElementSizeLog2
*
*   @brief
*       Gets meta data element size log2
*   @return
*       Meta data element size log2
************************************************************************************************************************
*/
INT_32 Gfx10Lib::GetMetaElementSizeLog2(
    Gfx10DataType dataType) ///< Data surface type
{
    INT_32 elemSizeLog2 = 0;

    if (dataType == Gfx10DataColor)
    {
        elemSizeLog2 = 0;
    }
    else if (dataType == Gfx10DataDepthStencil)
    {
        elemSizeLog2 = 2;
    }
    else
    {
        ADDR_ASSERT(dataType == Gfx10DataFmask);
        elemSizeLog2 = -1;
    }

    return elemSizeLog2;
}

/**
************************************************************************************************************************
*   Gfx10Lib::GetMetaCacheSizeLog2
*
*   @brief
*       Gets meta data cache line size log2
*   @return
*       Meta data cache line size log2
************************************************************************************************************************
*/
INT_32 Gfx10Lib::GetMetaCacheSizeLog2(
    Gfx10DataType dataType) ///< Data surface type
{
    INT_32 cacheSizeLog2 = 0;

    if (dataType == Gfx10DataColor)
    {
        cacheSizeLog2 = 6;
    }
    else if (dataType == Gfx10DataDepthStencil)
    {
        cacheSizeLog2 = 8;
    }
    else
    {
        ADDR_ASSERT(dataType == Gfx10DataFmask);
        cacheSizeLog2 = 8;
    }
    return cacheSizeLog2;
}

/**
************************************************************************************************************************
*   Gfx10Lib::HwlComputeSurfaceInfoLinear
*
*   @brief
*       Internal function to calculate alignment for linear surface
*
*   @return
*       ADDR_E_RETURNCODE
************************************************************************************************************************
*/
ADDR_E_RETURNCODE Gfx10Lib::HwlComputeSurfaceInfoLinear(
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

            // Following members are useless on GFX10
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
