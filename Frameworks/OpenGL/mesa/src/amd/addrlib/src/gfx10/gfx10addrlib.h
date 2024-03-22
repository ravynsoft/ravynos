/*
************************************************************************************************************************
*
*  Copyright (C) 2007-2022 Advanced Micro Devices, Inc.  All rights reserved.
*  SPDX-License-Identifier: MIT
*
***********************************************************************************************************************/

/**
************************************************************************************************************************
* @file  gfx10addrlib.h
* @brief Contains the Gfx10Lib class definition.
************************************************************************************************************************
*/

#ifndef __GFX10_ADDR_LIB_H__
#define __GFX10_ADDR_LIB_H__

#include "addrlib2.h"
#include "coord.h"
#include "gfx10SwizzlePattern.h"

namespace Addr
{
namespace V2
{

/**
************************************************************************************************************************
* @brief GFX10 specific settings structure.
************************************************************************************************************************
*/
struct Gfx10ChipSettings
{
    struct
    {
        UINT_32 reserved1           : 32;

        // Misc configuration bits
        UINT_32 isDcn20             : 1; // If using DCN2.0
        UINT_32 supportRbPlus       : 1;
        UINT_32 dsMipmapHtileFix    : 1;
        UINT_32 dccUnsup3DSwDis     : 1;
        UINT_32                     : 4;
        UINT_32 reserved2           : 24;
    };
};

/**
************************************************************************************************************************
* @brief GFX10 data surface type.
************************************************************************************************************************
*/
enum Gfx10DataType
{
    Gfx10DataColor,
    Gfx10DataDepthStencil,
    Gfx10DataFmask
};

const UINT_32 Gfx10LinearSwModeMask = (1u << ADDR_SW_LINEAR);

const UINT_32 Gfx10Blk256BSwModeMask = (1u << ADDR_SW_256B_S) |
                                       (1u << ADDR_SW_256B_D);

const UINT_32 Gfx10Blk4KBSwModeMask = (1u << ADDR_SW_4KB_S)   |
                                      (1u << ADDR_SW_4KB_D)   |
                                      (1u << ADDR_SW_4KB_S_X) |
                                      (1u << ADDR_SW_4KB_D_X);

const UINT_32 Gfx10Blk64KBSwModeMask = (1u << ADDR_SW_64KB_S)   |
                                       (1u << ADDR_SW_64KB_D)   |
                                       (1u << ADDR_SW_64KB_S_T) |
                                       (1u << ADDR_SW_64KB_D_T) |
                                       (1u << ADDR_SW_64KB_Z_X) |
                                       (1u << ADDR_SW_64KB_S_X) |
                                       (1u << ADDR_SW_64KB_D_X) |
                                       (1u << ADDR_SW_64KB_R_X);

const UINT_32 Gfx10BlkVarSwModeMask = (1u << ADDR_SW_VAR_Z_X) |
                                      (1u << ADDR_SW_VAR_R_X);

const UINT_32 Gfx10ZSwModeMask = (1u << ADDR_SW_64KB_Z_X) |
                                 (1u << ADDR_SW_VAR_Z_X);

const UINT_32 Gfx10StandardSwModeMask = (1u << ADDR_SW_256B_S)   |
                                        (1u << ADDR_SW_4KB_S)    |
                                        (1u << ADDR_SW_64KB_S)   |
                                        (1u << ADDR_SW_64KB_S_T) |
                                        (1u << ADDR_SW_4KB_S_X)  |
                                        (1u << ADDR_SW_64KB_S_X);

const UINT_32 Gfx10DisplaySwModeMask = (1u << ADDR_SW_256B_D)   |
                                       (1u << ADDR_SW_4KB_D)    |
                                       (1u << ADDR_SW_64KB_D)   |
                                       (1u << ADDR_SW_64KB_D_T) |
                                       (1u << ADDR_SW_4KB_D_X)  |
                                       (1u << ADDR_SW_64KB_D_X);

const UINT_32 Gfx10RenderSwModeMask = (1u << ADDR_SW_64KB_R_X) |
                                      (1u << ADDR_SW_VAR_R_X);

const UINT_32 Gfx10XSwModeMask = (1u << ADDR_SW_4KB_S_X)  |
                                 (1u << ADDR_SW_4KB_D_X)  |
                                 (1u << ADDR_SW_64KB_Z_X) |
                                 (1u << ADDR_SW_64KB_S_X) |
                                 (1u << ADDR_SW_64KB_D_X) |
                                 (1u << ADDR_SW_64KB_R_X) |
                                 Gfx10BlkVarSwModeMask;

const UINT_32 Gfx10TSwModeMask = (1u << ADDR_SW_64KB_S_T) |
                                 (1u << ADDR_SW_64KB_D_T);

const UINT_32 Gfx10XorSwModeMask = Gfx10XSwModeMask |
                                   Gfx10TSwModeMask;

const UINT_32 Gfx10Rsrc1dSwModeMask = Gfx10LinearSwModeMask |
                                      Gfx10RenderSwModeMask |
                                      Gfx10ZSwModeMask;

const UINT_32 Gfx10Rsrc2dSwModeMask = Gfx10LinearSwModeMask  |
                                      Gfx10Blk256BSwModeMask |
                                      Gfx10Blk4KBSwModeMask  |
                                      Gfx10Blk64KBSwModeMask |
                                      Gfx10BlkVarSwModeMask;

const UINT_32 Gfx10Rsrc3dSwModeMask = (1u << ADDR_SW_LINEAR)   |
                                      (1u << ADDR_SW_4KB_S)    |
                                      (1u << ADDR_SW_64KB_S)   |
                                      (1u << ADDR_SW_64KB_S_T) |
                                      (1u << ADDR_SW_4KB_S_X)  |
                                      (1u << ADDR_SW_64KB_Z_X) |
                                      (1u << ADDR_SW_64KB_S_X) |
                                      (1u << ADDR_SW_64KB_D_X) |
                                      (1u << ADDR_SW_64KB_R_X) |
                                      Gfx10BlkVarSwModeMask;

const UINT_32 Gfx10Rsrc2dPrtSwModeMask = (Gfx10Blk4KBSwModeMask | Gfx10Blk64KBSwModeMask) & ~Gfx10XSwModeMask;

const UINT_32 Gfx10Rsrc3dPrtSwModeMask = Gfx10Rsrc2dPrtSwModeMask & ~Gfx10DisplaySwModeMask;

const UINT_32 Gfx10Rsrc3dThin64KBSwModeMask = (1u << ADDR_SW_64KB_Z_X) |
                                              (1u << ADDR_SW_64KB_R_X);


const UINT_32 Gfx10Rsrc3dThinSwModeMask = Gfx10Rsrc3dThin64KBSwModeMask |
                                          Gfx10BlkVarSwModeMask;

const UINT_32 Gfx10Rsrc3dThickSwModeMask = Gfx10Rsrc3dSwModeMask & ~(Gfx10Rsrc3dThinSwModeMask | Gfx10LinearSwModeMask);

const UINT_32 Gfx10Rsrc3dThick4KBSwModeMask = Gfx10Rsrc3dThickSwModeMask & Gfx10Blk4KBSwModeMask;

const UINT_32 Gfx10Rsrc3dThick64KBSwModeMask = Gfx10Rsrc3dThickSwModeMask & Gfx10Blk64KBSwModeMask;

const UINT_32 Gfx10MsaaSwModeMask = (Gfx10ZSwModeMask       |
                                     Gfx10RenderSwModeMask)
                                    ;

const UINT_32 Dcn20NonBpp64SwModeMask = (1u << ADDR_SW_LINEAR)   |
                                        (1u << ADDR_SW_4KB_S)    |
                                        (1u << ADDR_SW_64KB_S)   |
                                        (1u << ADDR_SW_64KB_S_T) |
                                        (1u << ADDR_SW_4KB_S_X)  |
                                        (1u << ADDR_SW_64KB_S_X) |
                                        (1u << ADDR_SW_64KB_R_X);

const UINT_32 Dcn20Bpp64SwModeMask = (1u << ADDR_SW_4KB_D)    |
                                     (1u << ADDR_SW_64KB_D)   |
                                     (1u << ADDR_SW_64KB_D_T) |
                                     (1u << ADDR_SW_4KB_D_X)  |
                                     (1u << ADDR_SW_64KB_D_X) |
                                     Dcn20NonBpp64SwModeMask;

const UINT_32 Dcn21NonBpp64SwModeMask = (1u << ADDR_SW_LINEAR)   |
                                        (1u << ADDR_SW_64KB_S)   |
                                        (1u << ADDR_SW_64KB_S_T) |
                                        (1u << ADDR_SW_64KB_S_X) |
                                        (1u << ADDR_SW_64KB_R_X);

const UINT_32 Dcn21Bpp64SwModeMask = (1u << ADDR_SW_64KB_D)   |
                                     (1u << ADDR_SW_64KB_D_T) |
                                     (1u << ADDR_SW_64KB_D_X) |
                                     Dcn21NonBpp64SwModeMask;

/**
************************************************************************************************************************
* @brief This class is the GFX10 specific address library
*        function set.
************************************************************************************************************************
*/
class Gfx10Lib : public Lib
{
public:
    /// Creates Gfx10Lib object
    static Addr::Lib* CreateObj(const Client* pClient)
    {
        VOID* pMem = Object::ClientAlloc(sizeof(Gfx10Lib), pClient);
        return (pMem != NULL) ? new (pMem) Gfx10Lib(pClient) : NULL;
    }

protected:
    Gfx10Lib(const Client* pClient);
    virtual ~Gfx10Lib();

    virtual BOOL_32 HwlIsStandardSwizzle(
        AddrResourceType resourceType,
        AddrSwizzleMode  swizzleMode) const
    {
        return m_swizzleModeTable[swizzleMode].isStd;
    }

    virtual BOOL_32 HwlIsDisplaySwizzle(
        AddrResourceType resourceType,
        AddrSwizzleMode  swizzleMode) const
    {
        return m_swizzleModeTable[swizzleMode].isDisp;
    }

    virtual BOOL_32 HwlIsThin(
        AddrResourceType resourceType,
        AddrSwizzleMode  swizzleMode) const
    {
        return ((IsTex1d(resourceType)  == TRUE) ||
                (IsTex2d(resourceType)  == TRUE) ||
                ((IsTex3d(resourceType) == TRUE)                  &&
                 (m_swizzleModeTable[swizzleMode].isStd  == FALSE) &&
                 (m_swizzleModeTable[swizzleMode].isDisp == FALSE)));
    }

    virtual BOOL_32 HwlIsThick(
        AddrResourceType resourceType,
        AddrSwizzleMode  swizzleMode) const
    {
        return ((IsTex3d(resourceType) == TRUE) &&
                (m_swizzleModeTable[swizzleMode].isStd || m_swizzleModeTable[swizzleMode].isDisp));
    }

    virtual ADDR_E_RETURNCODE HwlComputeHtileInfo(
        const ADDR2_COMPUTE_HTILE_INFO_INPUT* pIn,
        ADDR2_COMPUTE_HTILE_INFO_OUTPUT*      pOut) const;

    virtual ADDR_E_RETURNCODE HwlComputeCmaskInfo(
        const ADDR2_COMPUTE_CMASK_INFO_INPUT* pIn,
        ADDR2_COMPUTE_CMASK_INFO_OUTPUT*      pOut) const;

    virtual ADDR_E_RETURNCODE HwlComputeDccInfo(
        const ADDR2_COMPUTE_DCCINFO_INPUT* pIn,
        ADDR2_COMPUTE_DCCINFO_OUTPUT*      pOut) const;

    virtual ADDR_E_RETURNCODE HwlComputeCmaskAddrFromCoord(
        const ADDR2_COMPUTE_CMASK_ADDRFROMCOORD_INPUT* pIn,
        ADDR2_COMPUTE_CMASK_ADDRFROMCOORD_OUTPUT*      pOut);

    virtual ADDR_E_RETURNCODE HwlComputeHtileAddrFromCoord(
        const ADDR2_COMPUTE_HTILE_ADDRFROMCOORD_INPUT* pIn,
        ADDR2_COMPUTE_HTILE_ADDRFROMCOORD_OUTPUT*      pOut);

    virtual ADDR_E_RETURNCODE HwlComputeHtileCoordFromAddr(
        const ADDR2_COMPUTE_HTILE_COORDFROMADDR_INPUT* pIn,
        ADDR2_COMPUTE_HTILE_COORDFROMADDR_OUTPUT*      pOut);

    virtual ADDR_E_RETURNCODE HwlSupportComputeDccAddrFromCoord(
        const ADDR2_COMPUTE_DCC_ADDRFROMCOORD_INPUT* pIn);

    virtual VOID HwlComputeDccAddrFromCoord(
        const ADDR2_COMPUTE_DCC_ADDRFROMCOORD_INPUT* pIn,
        ADDR2_COMPUTE_DCC_ADDRFROMCOORD_OUTPUT*      pOut);

    virtual UINT_32 HwlGetEquationIndex(
        const ADDR2_COMPUTE_SURFACE_INFO_INPUT* pIn,
        ADDR2_COMPUTE_SURFACE_INFO_OUTPUT*      pOut) const;

    virtual UINT_32 HwlGetEquationTableInfo(const ADDR_EQUATION** ppEquationTable) const
    {
        *ppEquationTable = m_equationTable;

        return m_numEquations;
    }

    virtual ADDR_E_RETURNCODE HwlComputePipeBankXor(
        const ADDR2_COMPUTE_PIPEBANKXOR_INPUT* pIn,
        ADDR2_COMPUTE_PIPEBANKXOR_OUTPUT*      pOut) const;

    virtual ADDR_E_RETURNCODE HwlComputeSlicePipeBankXor(
        const ADDR2_COMPUTE_SLICE_PIPEBANKXOR_INPUT* pIn,
        ADDR2_COMPUTE_SLICE_PIPEBANKXOR_OUTPUT*      pOut) const;

    virtual ADDR_E_RETURNCODE HwlComputeSubResourceOffsetForSwizzlePattern(
        const ADDR2_COMPUTE_SUBRESOURCE_OFFSET_FORSWIZZLEPATTERN_INPUT* pIn,
        ADDR2_COMPUTE_SUBRESOURCE_OFFSET_FORSWIZZLEPATTERN_OUTPUT*      pOut) const;

    virtual ADDR_E_RETURNCODE HwlComputeNonBlockCompressedView(
        const ADDR2_COMPUTE_NONBLOCKCOMPRESSEDVIEW_INPUT* pIn,
        ADDR2_COMPUTE_NONBLOCKCOMPRESSEDVIEW_OUTPUT*      pOut) const;

    virtual ADDR_E_RETURNCODE HwlGetPreferredSurfaceSetting(
        const ADDR2_GET_PREFERRED_SURF_SETTING_INPUT* pIn,
        ADDR2_GET_PREFERRED_SURF_SETTING_OUTPUT*      pOut) const;

    virtual ADDR_E_RETURNCODE HwlComputeSurfaceInfoSanityCheck(
        const ADDR2_COMPUTE_SURFACE_INFO_INPUT* pIn) const;

    virtual ADDR_E_RETURNCODE HwlComputeSurfaceInfoTiled(
         const ADDR2_COMPUTE_SURFACE_INFO_INPUT* pIn,
         ADDR2_COMPUTE_SURFACE_INFO_OUTPUT*      pOut) const;

    virtual ADDR_E_RETURNCODE HwlComputeSurfaceInfoLinear(
         const ADDR2_COMPUTE_SURFACE_INFO_INPUT* pIn,
         ADDR2_COMPUTE_SURFACE_INFO_OUTPUT*      pOut) const;

    virtual ADDR_E_RETURNCODE HwlComputeSurfaceAddrFromCoordTiled(
        const ADDR2_COMPUTE_SURFACE_ADDRFROMCOORD_INPUT* pIn,
        ADDR2_COMPUTE_SURFACE_ADDRFROMCOORD_OUTPUT*      pOut) const;

    virtual UINT_32 HwlComputeMaxBaseAlignments() const;

    virtual UINT_32 HwlComputeMaxMetaBaseAlignments() const;

    virtual BOOL_32 HwlInitGlobalParams(const ADDR_CREATE_INPUT* pCreateIn);

    virtual ChipFamily HwlConvertChipFamily(UINT_32 uChipFamily, UINT_32 uChipRevision);

private:
    // Initialize equation table
    VOID InitEquationTable();

    ADDR_E_RETURNCODE ComputeSurfaceInfoMacroTiled(
         const ADDR2_COMPUTE_SURFACE_INFO_INPUT* pIn,
         ADDR2_COMPUTE_SURFACE_INFO_OUTPUT*      pOut) const;

    ADDR_E_RETURNCODE ComputeSurfaceInfoMicroTiled(
         const ADDR2_COMPUTE_SURFACE_INFO_INPUT* pIn,
         ADDR2_COMPUTE_SURFACE_INFO_OUTPUT*      pOut) const;

    ADDR_E_RETURNCODE ComputeSurfaceAddrFromCoordMacroTiled(
        const ADDR2_COMPUTE_SURFACE_ADDRFROMCOORD_INPUT* pIn,
        ADDR2_COMPUTE_SURFACE_ADDRFROMCOORD_OUTPUT*      pOut) const;

    ADDR_E_RETURNCODE ComputeSurfaceAddrFromCoordMicroTiled(
        const ADDR2_COMPUTE_SURFACE_ADDRFROMCOORD_INPUT* pIn,
        ADDR2_COMPUTE_SURFACE_ADDRFROMCOORD_OUTPUT*      pOut) const;

    UINT_32 ComputeOffsetFromSwizzlePattern(
        const UINT_64* pPattern,
        UINT_32        numBits,
        UINT_32        x,
        UINT_32        y,
        UINT_32        z,
        UINT_32        s) const;

    UINT_32 ComputeOffsetFromEquation(
        const ADDR_EQUATION* pEq,
        UINT_32              x,
        UINT_32              y,
        UINT_32              z) const;

    ADDR_E_RETURNCODE ComputeStereoInfo(
        const ADDR2_COMPUTE_SURFACE_INFO_INPUT* pIn,
        UINT_32*                                pAlignY,
        UINT_32*                                pRightXor) const;

    static void GetMipSize(
        UINT_32  mip0Width,
        UINT_32  mip0Height,
        UINT_32  mip0Depth,
        UINT_32  mipId,
        UINT_32* pMipWidth,
        UINT_32* pMipHeight,
        UINT_32* pMipDepth = NULL)
    {
        *pMipWidth  = ShiftCeil(Max(mip0Width, 1u),  mipId);
        *pMipHeight = ShiftCeil(Max(mip0Height, 1u), mipId);

        if (pMipDepth != NULL)
        {
            *pMipDepth = ShiftCeil(Max(mip0Depth, 1u),  mipId);
        }
    }

    const ADDR_SW_PATINFO* GetSwizzlePatternInfo(
        AddrSwizzleMode  swizzleMode,
        AddrResourceType resourceType,
        UINT_32          log2Elem,
        UINT_32          numFrag) const;

    /**
     * Will use the indices, "nibbles", to build an index equation inside pSwizzle
     *
     * @param pPatInfo Pointer to a patInfo. Contains indices mapping to the 2D nibble arrays which will be used to build an index equation.
     * @param pSwizzle Array to write the index equation to.
     */
    VOID GetSwizzlePatternFromPatternInfo(
        const ADDR_SW_PATINFO* pPatInfo,
        ADDR_BIT_SETTING       (&pSwizzle)[20]) const
    {
        memcpy(pSwizzle,
               GFX10_SW_PATTERN_NIBBLE01[pPatInfo->nibble01Idx],
               sizeof(GFX10_SW_PATTERN_NIBBLE01[pPatInfo->nibble01Idx]));

        memcpy(&pSwizzle[8],
               GFX10_SW_PATTERN_NIBBLE2[pPatInfo->nibble2Idx],
               sizeof(GFX10_SW_PATTERN_NIBBLE2[pPatInfo->nibble2Idx]));

        memcpy(&pSwizzle[12],
               GFX10_SW_PATTERN_NIBBLE3[pPatInfo->nibble3Idx],
               sizeof(GFX10_SW_PATTERN_NIBBLE3[pPatInfo->nibble3Idx]));

        memcpy(&pSwizzle[16],
               GFX10_SW_PATTERN_NIBBLE4[pPatInfo->nibble4Idx],
               sizeof(GFX10_SW_PATTERN_NIBBLE4[pPatInfo->nibble4Idx]));
    }

    VOID ConvertSwizzlePatternToEquation(
        UINT_32                elemLog2,
        AddrResourceType       rsrcType,
        AddrSwizzleMode        swMode,
        const ADDR_SW_PATINFO* pPatInfo,
        ADDR_EQUATION*         pEquation) const;

    static INT_32 GetMetaElementSizeLog2(Gfx10DataType dataType);

    static INT_32 GetMetaCacheSizeLog2(Gfx10DataType dataType);

    void GetBlk256SizeLog2(
        AddrResourceType resourceType,
        AddrSwizzleMode  swizzleMode,
        UINT_32          elemLog2,
        UINT_32          numSamplesLog2,
        Dim3d*           pBlock) const;

    void GetCompressedBlockSizeLog2(
        Gfx10DataType    dataType,
        AddrResourceType resourceType,
        AddrSwizzleMode  swizzleMode,
        UINT_32          elemLog2,
        UINT_32          numSamplesLog2,
        Dim3d*           pBlock) const;

    INT_32 GetMetaOverlapLog2(
        Gfx10DataType    dataType,
        AddrResourceType resourceType,
        AddrSwizzleMode  swizzleMode,
        UINT_32          elemLog2,
        UINT_32          numSamplesLog2) const;

    INT_32 Get3DMetaOverlapLog2(
        AddrResourceType resourceType,
        AddrSwizzleMode  swizzleMode,
        UINT_32          elemLog2) const;

    UINT_32 GetMetaBlkSize(
        Gfx10DataType    dataType,
        AddrResourceType resourceType,
        AddrSwizzleMode  swizzleMode,
        UINT_32          elemLog2,
        UINT_32          numSamplesLog2,
        BOOL_32          pipeAlign,
        Dim3d*           pBlock) const;

    INT_32 GetPipeRotateAmount(
        AddrResourceType resourceType,
        AddrSwizzleMode  swizzleMode) const;

    INT_32 GetEffectiveNumPipes() const
    {
        return ((m_settings.supportRbPlus == FALSE) ||
                ((m_numSaLog2 + 1) >= m_pipesLog2)) ? m_pipesLog2 : m_numSaLog2 + 1;
    }

    BOOL_32 IsRbAligned(
        AddrResourceType resourceType,
        AddrSwizzleMode  swizzleMode) const
    {
        const BOOL_32 isRtopt   = IsRtOptSwizzle(swizzleMode);
        const BOOL_32 isZ       = IsZOrderSwizzle(swizzleMode);
        const BOOL_32 isDisplay = IsDisplaySwizzle(swizzleMode);

        return (IsTex2d(resourceType) && (isRtopt || isZ)) ||
               (IsTex3d(resourceType) && isDisplay);

    }

    UINT_32 GetValidDisplaySwizzleModes(UINT_32 bpp) const;

    BOOL_32 IsValidDisplaySwizzleMode(const ADDR2_COMPUTE_SURFACE_INFO_INPUT* pIn) const;

    UINT_32 GetMaxNumMipsInTail(UINT_32 blockSizeLog2, BOOL_32 isThin) const;

    static ADDR2_BLOCK_SET GetAllowedBlockSet(ADDR2_SWMODE_SET allowedSwModeSet, AddrResourceType rsrcType)
    {
        ADDR2_BLOCK_SET allowedBlockSet = {};

        allowedBlockSet.micro  = (allowedSwModeSet.value & Gfx10Blk256BSwModeMask) ? TRUE : FALSE;
        allowedBlockSet.linear = (allowedSwModeSet.value & Gfx10LinearSwModeMask)  ? TRUE : FALSE;
        allowedBlockSet.var    = (allowedSwModeSet.value & Gfx10BlkVarSwModeMask)  ? TRUE : FALSE;

        if (rsrcType == ADDR_RSRC_TEX_3D)
        {
            allowedBlockSet.macroThick4KB  = (allowedSwModeSet.value & Gfx10Rsrc3dThick4KBSwModeMask)  ? TRUE : FALSE;
            allowedBlockSet.macroThin64KB  = (allowedSwModeSet.value & Gfx10Rsrc3dThin64KBSwModeMask)  ? TRUE : FALSE;
            allowedBlockSet.macroThick64KB = (allowedSwModeSet.value & Gfx10Rsrc3dThick64KBSwModeMask) ? TRUE : FALSE;
        }
        else
        {
            allowedBlockSet.macroThin4KB  = (allowedSwModeSet.value & Gfx10Blk4KBSwModeMask)  ? TRUE : FALSE;
            allowedBlockSet.macroThin64KB = (allowedSwModeSet.value & Gfx10Blk64KBSwModeMask) ? TRUE : FALSE;
        }

        return allowedBlockSet;
    }

    static ADDR2_SWTYPE_SET GetAllowedSwSet(ADDR2_SWMODE_SET allowedSwModeSet)
    {
        ADDR2_SWTYPE_SET allowedSwSet = {};

        allowedSwSet.sw_Z = (allowedSwModeSet.value & Gfx10ZSwModeMask)        ? TRUE : FALSE;
        allowedSwSet.sw_S = (allowedSwModeSet.value & Gfx10StandardSwModeMask) ? TRUE : FALSE;
        allowedSwSet.sw_D = (allowedSwModeSet.value & Gfx10DisplaySwModeMask)  ? TRUE : FALSE;
        allowedSwSet.sw_R = (allowedSwModeSet.value & Gfx10RenderSwModeMask)   ? TRUE : FALSE;

        return allowedSwSet;
    }

    BOOL_32 IsInMipTail(
        Dim3d   mipTailDim,
        UINT_32 maxNumMipsInTail,
        UINT_32 mipWidth,
        UINT_32 mipHeight,
        UINT_32 numMipsToTheEnd) const
    {
        BOOL_32 inTail = ((mipWidth <= mipTailDim.w) &&
                          (mipHeight <= mipTailDim.h) &&
                          (numMipsToTheEnd <= maxNumMipsInTail));

        return inTail;
    }

    UINT_32 GetBankXorBits(UINT_32 blockBits) const
    {
        return (blockBits > m_pipeInterleaveLog2 + m_pipesLog2 + ColumnBits) ?
               Min(blockBits - m_pipeInterleaveLog2 - m_pipesLog2 - ColumnBits, BankBits) : 0;
    }

    BOOL_32 ValidateNonSwModeParams(const ADDR2_COMPUTE_SURFACE_INFO_INPUT* pIn) const;
    BOOL_32 ValidateSwModeParams(const ADDR2_COMPUTE_SURFACE_INFO_INPUT* pIn) const;

    static const UINT_32 ColumnBits       = 2;
    static const UINT_32 BankBits         = 4;
    static const UINT_32 UnalignedDccType = 3;

    static const Dim3d Block256_3d[MaxNumOfBpp];
    static const Dim3d Block64K_Log2_3d[MaxNumOfBpp];
    static const Dim3d Block4K_Log2_3d[MaxNumOfBpp];

    static const SwizzleModeFlags SwizzleModeTable[ADDR_SW_MAX_TYPE];

    // Number of packers log2
    UINT_32 m_numPkrLog2;
    // Number of shader array log2
    UINT_32 m_numSaLog2;

    Gfx10ChipSettings m_settings;

    UINT_32 m_colorBaseIndex;
    UINT_32 m_xmaskBaseIndex;
    UINT_32 m_htileBaseIndex;
    UINT_32 m_dccBaseIndex;
};

} // V2
} // Addr

#endif

