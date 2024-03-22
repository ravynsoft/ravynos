/*
************************************************************************************************************************
*
*  Copyright (C) 2007-2022 Advanced Micro Devices, Inc.  All rights reserved.
*  SPDX-License-Identifier: MIT
*
***********************************************************************************************************************/

/**
************************************************************************************************************************
* @file  gfx9addrlib.h
* @brief Contgfx9ns the Gfx9Lib class definition.
************************************************************************************************************************
*/

#ifndef __GFX9_ADDR_LIB_H__
#define __GFX9_ADDR_LIB_H__

#include "addrlib2.h"
#include "coord.h"

namespace Addr
{
namespace V2
{

/**
************************************************************************************************************************
* @brief GFX9 specific settings structure.
************************************************************************************************************************
*/
struct Gfx9ChipSettings
{
    struct
    {
        // Asic/Generation name
        UINT_32 isArcticIsland      : 1;
        UINT_32 isVega10            : 1;
        UINT_32 isRaven             : 1;
        UINT_32 isVega12            : 1;
        UINT_32 isVega20            : 1;
        UINT_32 reserved0           : 27;

        // Display engine IP version name
        UINT_32 isDce12             : 1;
        UINT_32 isDcn1              : 1;
        UINT_32 isDcn2              : 1;
        UINT_32 reserved1           : 29;

        // Misc configuration bits
        UINT_32 metaBaseAlignFix    : 1;
        UINT_32 depthPipeXorDisable : 1;
        UINT_32 htileAlignFix       : 1;
        UINT_32 applyAliasFix       : 1;
        UINT_32 htileCacheRbConflict: 1;
        UINT_32 reserved2           : 27;
    };
};

/**
************************************************************************************************************************
* @brief GFX9 data surface type.
************************************************************************************************************************
*/
enum Gfx9DataType
{
    Gfx9DataColor,
    Gfx9DataDepthStencil,
    Gfx9DataFmask
};

const UINT_32 Gfx9LinearSwModeMask = (1u << ADDR_SW_LINEAR);

const UINT_32 Gfx9Blk256BSwModeMask = (1u << ADDR_SW_256B_S) |
                                      (1u << ADDR_SW_256B_D) |
                                      (1u << ADDR_SW_256B_R);

const UINT_32 Gfx9Blk4KBSwModeMask = (1u << ADDR_SW_4KB_Z)   |
                                     (1u << ADDR_SW_4KB_S)   |
                                     (1u << ADDR_SW_4KB_D)   |
                                     (1u << ADDR_SW_4KB_R)   |
                                     (1u << ADDR_SW_4KB_Z_X) |
                                     (1u << ADDR_SW_4KB_S_X) |
                                     (1u << ADDR_SW_4KB_D_X) |
                                     (1u << ADDR_SW_4KB_R_X);

const UINT_32 Gfx9Blk64KBSwModeMask = (1u << ADDR_SW_64KB_Z)   |
                                      (1u << ADDR_SW_64KB_S)   |
                                      (1u << ADDR_SW_64KB_D)   |
                                      (1u << ADDR_SW_64KB_R)   |
                                      (1u << ADDR_SW_64KB_Z_T) |
                                      (1u << ADDR_SW_64KB_S_T) |
                                      (1u << ADDR_SW_64KB_D_T) |
                                      (1u << ADDR_SW_64KB_R_T) |
                                      (1u << ADDR_SW_64KB_Z_X) |
                                      (1u << ADDR_SW_64KB_S_X) |
                                      (1u << ADDR_SW_64KB_D_X) |
                                      (1u << ADDR_SW_64KB_R_X);

const UINT_32 Gfx9ZSwModeMask = (1u << ADDR_SW_4KB_Z)    |
                                (1u << ADDR_SW_64KB_Z)   |
                                (1u << ADDR_SW_64KB_Z_T) |
                                (1u << ADDR_SW_4KB_Z_X)  |
                                (1u << ADDR_SW_64KB_Z_X);

const UINT_32 Gfx9StandardSwModeMask = (1u << ADDR_SW_256B_S)   |
                                       (1u << ADDR_SW_4KB_S)    |
                                       (1u << ADDR_SW_64KB_S)   |
                                       (1u << ADDR_SW_64KB_S_T) |
                                       (1u << ADDR_SW_4KB_S_X)  |
                                       (1u << ADDR_SW_64KB_S_X);

const UINT_32 Gfx9DisplaySwModeMask = (1u << ADDR_SW_256B_D)   |
                                      (1u << ADDR_SW_4KB_D)    |
                                      (1u << ADDR_SW_64KB_D)   |
                                      (1u << ADDR_SW_64KB_D_T) |
                                      (1u << ADDR_SW_4KB_D_X)  |
                                      (1u << ADDR_SW_64KB_D_X);

const UINT_32 Gfx9RotateSwModeMask = (1u << ADDR_SW_256B_R)   |
                                     (1u << ADDR_SW_4KB_R)    |
                                     (1u << ADDR_SW_64KB_R)   |
                                     (1u << ADDR_SW_64KB_R_T) |
                                     (1u << ADDR_SW_4KB_R_X)  |
                                     (1u << ADDR_SW_64KB_R_X);

const UINT_32 Gfx9XSwModeMask = (1u << ADDR_SW_4KB_Z_X)  |
                                (1u << ADDR_SW_4KB_S_X)  |
                                (1u << ADDR_SW_4KB_D_X)  |
                                (1u << ADDR_SW_4KB_R_X)  |
                                (1u << ADDR_SW_64KB_Z_X) |
                                (1u << ADDR_SW_64KB_S_X) |
                                (1u << ADDR_SW_64KB_D_X) |
                                (1u << ADDR_SW_64KB_R_X);

const UINT_32 Gfx9TSwModeMask = (1u << ADDR_SW_64KB_Z_T) |
                                (1u << ADDR_SW_64KB_S_T) |
                                (1u << ADDR_SW_64KB_D_T) |
                                (1u << ADDR_SW_64KB_R_T);

const UINT_32 Gfx9XorSwModeMask = Gfx9XSwModeMask |
                                  Gfx9TSwModeMask;

const UINT_32 Gfx9AllSwModeMask = Gfx9LinearSwModeMask   |
                                  Gfx9ZSwModeMask        |
                                  Gfx9StandardSwModeMask |
                                  Gfx9DisplaySwModeMask  |
                                  Gfx9RotateSwModeMask;

const UINT_32 Gfx9Rsrc1dSwModeMask = Gfx9LinearSwModeMask;

const UINT_32 Gfx9Rsrc2dSwModeMask = Gfx9AllSwModeMask;

const UINT_32 Gfx9Rsrc3dSwModeMask = Gfx9AllSwModeMask & ~Gfx9Blk256BSwModeMask & ~Gfx9RotateSwModeMask;

const UINT_32 Gfx9Rsrc2dPrtSwModeMask = (Gfx9Blk4KBSwModeMask | Gfx9Blk64KBSwModeMask) & ~Gfx9XSwModeMask;

const UINT_32 Gfx9Rsrc3dPrtSwModeMask = Gfx9Rsrc2dPrtSwModeMask & ~Gfx9RotateSwModeMask & ~Gfx9DisplaySwModeMask;

const UINT_32 Gfx9Rsrc3dThinSwModeMask = Gfx9DisplaySwModeMask & ~Gfx9Blk256BSwModeMask;

const UINT_32 Gfx9Rsrc3dThin4KBSwModeMask = Gfx9Rsrc3dThinSwModeMask & Gfx9Blk4KBSwModeMask;

const UINT_32 Gfx9Rsrc3dThin64KBSwModeMask = Gfx9Rsrc3dThinSwModeMask & Gfx9Blk64KBSwModeMask;

const UINT_32 Gfx9Rsrc3dThickSwModeMask = Gfx9Rsrc3dSwModeMask & ~(Gfx9Rsrc3dThinSwModeMask | Gfx9LinearSwModeMask);

const UINT_32 Gfx9Rsrc3dThick4KBSwModeMask = Gfx9Rsrc3dThickSwModeMask & Gfx9Blk4KBSwModeMask;

const UINT_32 Gfx9Rsrc3dThick64KBSwModeMask = Gfx9Rsrc3dThickSwModeMask & Gfx9Blk64KBSwModeMask;

const UINT_32 Gfx9MsaaSwModeMask = Gfx9AllSwModeMask & ~Gfx9Blk256BSwModeMask & ~Gfx9LinearSwModeMask;

const UINT_32 Dce12NonBpp32SwModeMask = (1u << ADDR_SW_LINEAR)   |
                                        (1u << ADDR_SW_4KB_D)    |
                                        (1u << ADDR_SW_4KB_R)    |
                                        (1u << ADDR_SW_64KB_D)   |
                                        (1u << ADDR_SW_64KB_R)   |
                                        (1u << ADDR_SW_4KB_D_X)  |
                                        (1u << ADDR_SW_4KB_R_X)  |
                                        (1u << ADDR_SW_64KB_D_X) |
                                        (1u << ADDR_SW_64KB_R_X);

const UINT_32 Dce12Bpp32SwModeMask = (1u << ADDR_SW_256B_D) |
                                     (1u << ADDR_SW_256B_R) |
                                     Dce12NonBpp32SwModeMask;

const UINT_32 Dcn1NonBpp64SwModeMask = (1u << ADDR_SW_LINEAR)   |
                                       (1u << ADDR_SW_4KB_S)    |
                                       (1u << ADDR_SW_64KB_S)   |
                                       (1u << ADDR_SW_64KB_S_T) |
                                       (1u << ADDR_SW_4KB_S_X)  |
                                       (1u << ADDR_SW_64KB_S_X);
const UINT_32 Dcn1Bpp64SwModeMask = (1u << ADDR_SW_4KB_D)    |
                                    (1u << ADDR_SW_64KB_D)   |
                                    (1u << ADDR_SW_64KB_D_T) |
                                    (1u << ADDR_SW_4KB_D_X)  |
                                    (1u << ADDR_SW_64KB_D_X) |
                                    Dcn1NonBpp64SwModeMask;

const UINT_32 Dcn2NonBpp64SwModeMask = (1u << ADDR_SW_LINEAR)   |
                                       (1u << ADDR_SW_64KB_S)   |
                                       (1u << ADDR_SW_64KB_S_T) |
                                       (1u << ADDR_SW_64KB_S_X);

const UINT_32 Dcn2Bpp64SwModeMask = (1u << ADDR_SW_64KB_D)   |
                                    (1u << ADDR_SW_64KB_D_T) |
                                    (1u << ADDR_SW_64KB_D_X) |
                                    Dcn2NonBpp64SwModeMask;

/**
************************************************************************************************************************
* @brief GFX9 meta equation parameters
************************************************************************************************************************
*/
struct MetaEqParams
{
    UINT_32          maxMip;
    UINT_32          elementBytesLog2;
    UINT_32          numSamplesLog2;
    ADDR2_META_FLAGS metaFlag;
    Gfx9DataType     dataSurfaceType;
    AddrSwizzleMode  swizzleMode;
    AddrResourceType resourceType;
    UINT_32          metaBlkWidthLog2;
    UINT_32          metaBlkHeightLog2;
    UINT_32          metaBlkDepthLog2;
    UINT_32          compBlkWidthLog2;
    UINT_32          compBlkHeightLog2;
    UINT_32          compBlkDepthLog2;
};

/**
************************************************************************************************************************
* @brief This class is the GFX9 specific address library
*        function set.
************************************************************************************************************************
*/
class Gfx9Lib : public Lib
{
public:
    /// Creates Gfx9Lib object
    static Addr::Lib* CreateObj(const Client* pClient)
    {
        VOID* pMem = Object::ClientAlloc(sizeof(Gfx9Lib), pClient);
        return (pMem != NULL) ? new (pMem) Gfx9Lib(pClient) : NULL;
    }

protected:
    Gfx9Lib(const Client* pClient);
    virtual ~Gfx9Lib();

    virtual BOOL_32 HwlIsStandardSwizzle(
        AddrResourceType resourceType,
        AddrSwizzleMode  swizzleMode) const
    {
        return m_swizzleModeTable[swizzleMode].isStd ||
               (IsTex3d(resourceType) && m_swizzleModeTable[swizzleMode].isDisp);
    }

    virtual BOOL_32 HwlIsDisplaySwizzle(
        AddrResourceType resourceType,
        AddrSwizzleMode  swizzleMode) const
    {
        return IsTex2d(resourceType) && m_swizzleModeTable[swizzleMode].isDisp;
    }

    virtual BOOL_32 HwlIsThin(
        AddrResourceType resourceType,
        AddrSwizzleMode  swizzleMode) const
    {
        return ((IsTex2d(resourceType)  == TRUE) ||
                ((IsTex3d(resourceType) == TRUE)                  &&
                 (m_swizzleModeTable[swizzleMode].isZ   == FALSE) &&
                 (m_swizzleModeTable[swizzleMode].isStd == FALSE)));
    }

    virtual BOOL_32 HwlIsThick(
        AddrResourceType resourceType,
        AddrSwizzleMode  swizzleMode) const
    {
        return (IsTex3d(resourceType) &&
                (m_swizzleModeTable[swizzleMode].isZ || m_swizzleModeTable[swizzleMode].isStd));
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

    virtual ADDR_E_RETURNCODE HwlComputeBlock256Equation(
        AddrResourceType rsrcType,
        AddrSwizzleMode swMode,
        UINT_32 elementBytesLog2,
        ADDR_EQUATION* pEquation) const;

    virtual ADDR_E_RETURNCODE HwlComputeThinEquation(
        AddrResourceType rsrcType,
        AddrSwizzleMode swMode,
        UINT_32 elementBytesLog2,
        ADDR_EQUATION* pEquation) const;

    virtual ADDR_E_RETURNCODE HwlComputeThickEquation(
        AddrResourceType rsrcType,
        AddrSwizzleMode swMode,
        UINT_32 elementBytesLog2,
        ADDR_EQUATION* pEquation) const;

    // Get equation table pointer and number of equations
    virtual UINT_32 HwlGetEquationTableInfo(const ADDR_EQUATION** ppEquationTable) const
    {
        *ppEquationTable = m_equationTable;

        return m_numEquations;
    }

    virtual BOOL_32 IsEquationSupported(
        AddrResourceType rsrcType,
        AddrSwizzleMode swMode,
        UINT_32 elementBytesLog2) const;

    virtual ADDR_E_RETURNCODE HwlComputePipeBankXor(
        const ADDR2_COMPUTE_PIPEBANKXOR_INPUT* pIn,
        ADDR2_COMPUTE_PIPEBANKXOR_OUTPUT*      pOut) const;

    virtual ADDR_E_RETURNCODE HwlComputeSlicePipeBankXor(
        const ADDR2_COMPUTE_SLICE_PIPEBANKXOR_INPUT* pIn,
        ADDR2_COMPUTE_SLICE_PIPEBANKXOR_OUTPUT*      pOut) const;

    virtual ADDR_E_RETURNCODE HwlComputeSubResourceOffsetForSwizzlePattern(
        const ADDR2_COMPUTE_SUBRESOURCE_OFFSET_FORSWIZZLEPATTERN_INPUT* pIn,
        ADDR2_COMPUTE_SUBRESOURCE_OFFSET_FORSWIZZLEPATTERN_OUTPUT*      pOut) const;

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

    virtual VOID ComputeThinBlockDimension(
        UINT_32*         pWidth,
        UINT_32*         pHeight,
        UINT_32*         pDepth,
        UINT_32          bpp,
        UINT_32          numSamples,
        AddrResourceType resourceType,
        AddrSwizzleMode  swizzleMode) const;

private:
    VOID GetRbEquation(CoordEq* pRbEq, UINT_32 rbPerSeLog2, UINT_32 seLog2) const;

    VOID GetDataEquation(CoordEq* pDataEq, Gfx9DataType dataSurfaceType,
                         AddrSwizzleMode swizzleMode, AddrResourceType resourceType,
                         UINT_32 elementBytesLog2, UINT_32 numSamplesLog2) const;

    VOID GetPipeEquation(CoordEq* pPipeEq, CoordEq* pDataEq,
                         UINT_32 pipeInterleaveLog2, UINT_32 numPipesLog2,
                         UINT_32 numSamplesLog2, Gfx9DataType dataSurfaceType,
                         AddrSwizzleMode swizzleMode, AddrResourceType resourceType) const;

    VOID GenMetaEquation(CoordEq* pMetaEq, UINT_32 maxMip,
                         UINT_32 elementBytesLog2, UINT_32 numSamplesLog2,
                         ADDR2_META_FLAGS metaFlag, Gfx9DataType dataSurfaceType,
                         AddrSwizzleMode swizzleMode, AddrResourceType resourceType,
                         UINT_32 metaBlkWidthLog2, UINT_32 metaBlkHeightLog2,
                         UINT_32 metaBlkDepthLog2, UINT_32 compBlkWidthLog2,
                         UINT_32 compBlkHeightLog2, UINT_32 compBlkDepthLog2) const;

    const CoordEq* GetMetaEquation(const MetaEqParams& metaEqParams);

    VOID GetMetaMipInfo(UINT_32 numMipLevels, Dim3d* pMetaBlkDim,
                        BOOL_32 dataThick, ADDR2_META_MIP_INFO* pInfo,
                        UINT_32 mip0Width, UINT_32 mip0Height, UINT_32 mip0Depth,
                        UINT_32* pNumMetaBlkX, UINT_32* pNumMetaBlkY, UINT_32* pNumMetaBlkZ) const;

    BOOL_32 IsValidDisplaySwizzleMode(const ADDR2_COMPUTE_SURFACE_INFO_INPUT* pIn) const;

    ADDR_E_RETURNCODE ComputeSurfaceLinearPadding(
        const ADDR2_COMPUTE_SURFACE_INFO_INPUT* pIn,
        UINT_32*                                pMipmap0PaddedWidth,
        UINT_32*                                pSlice0PaddedHeight,
        ADDR2_MIP_INFO*                         pMipInfo = NULL) const;

    static ADDR2_BLOCK_SET GetAllowedBlockSet(ADDR2_SWMODE_SET allowedSwModeSet, AddrResourceType rsrcType)
    {
        ADDR2_BLOCK_SET allowedBlockSet = {};

        allowedBlockSet.micro  = (allowedSwModeSet.value & Gfx9Blk256BSwModeMask) ? TRUE : FALSE;
        allowedBlockSet.linear = (allowedSwModeSet.value & Gfx9LinearSwModeMask)  ? TRUE : FALSE;

        if (rsrcType == ADDR_RSRC_TEX_3D)
        {
            allowedBlockSet.macroThin4KB   = (allowedSwModeSet.value & Gfx9Rsrc3dThin4KBSwModeMask)   ? TRUE : FALSE;
            allowedBlockSet.macroThick4KB  = (allowedSwModeSet.value & Gfx9Rsrc3dThick4KBSwModeMask)  ? TRUE : FALSE;
            allowedBlockSet.macroThin64KB  = (allowedSwModeSet.value & Gfx9Rsrc3dThin64KBSwModeMask)  ? TRUE : FALSE;
            allowedBlockSet.macroThick64KB = (allowedSwModeSet.value & Gfx9Rsrc3dThick64KBSwModeMask) ? TRUE : FALSE;
        }
        else
        {
            allowedBlockSet.macroThin4KB  = (allowedSwModeSet.value & Gfx9Blk4KBSwModeMask)  ? TRUE : FALSE;
            allowedBlockSet.macroThin64KB = (allowedSwModeSet.value & Gfx9Blk64KBSwModeMask) ? TRUE : FALSE;
        }

        return allowedBlockSet;
    }

    static ADDR2_SWTYPE_SET GetAllowedSwSet(ADDR2_SWMODE_SET allowedSwModeSet)
    {
        ADDR2_SWTYPE_SET allowedSwSet = {};

        allowedSwSet.sw_Z = (allowedSwModeSet.value & Gfx9ZSwModeMask)        ? TRUE : FALSE;
        allowedSwSet.sw_S = (allowedSwModeSet.value & Gfx9StandardSwModeMask) ? TRUE : FALSE;
        allowedSwSet.sw_D = (allowedSwModeSet.value & Gfx9DisplaySwModeMask)  ? TRUE : FALSE;
        allowedSwSet.sw_R = (allowedSwModeSet.value & Gfx9RotateSwModeMask)   ? TRUE : FALSE;

        return allowedSwSet;
    }

    BOOL_32 IsInMipTail(
        AddrResourceType  resourceType,
        AddrSwizzleMode   swizzleMode,
        Dim3d             mipTailDim,
        UINT_32           width,
        UINT_32           height,
        UINT_32           depth) const
    {
        BOOL_32 inTail = ((width <= mipTailDim.w) &&
                          (height <= mipTailDim.h) &&
                          (IsThin(resourceType, swizzleMode) || (depth <= mipTailDim.d)));

        return inTail;
    }

    BOOL_32 ValidateNonSwModeParams(const ADDR2_COMPUTE_SURFACE_INFO_INPUT* pIn) const;
    BOOL_32 ValidateSwModeParams(const ADDR2_COMPUTE_SURFACE_INFO_INPUT* pIn) const;

    UINT_32 GetBankXorBits(UINT_32 macroBlockBits) const
    {
        UINT_32 pipeBits = GetPipeXorBits(macroBlockBits);

        // Bank xor bits
        UINT_32 bankBits = Min(macroBlockBits - pipeBits - m_pipeInterleaveLog2, m_banksLog2);

        return bankBits;
    }

    UINT_32 ComputeSurfaceBaseAlignTiled(AddrSwizzleMode swizzleMode) const
    {
        UINT_32 baseAlign;

        if (IsXor(swizzleMode))
        {
            baseAlign = GetBlockSize(swizzleMode);
        }
        else
        {
            baseAlign = 256;
        }

        return baseAlign;
    }

    // Initialize equation table
    VOID InitEquationTable();

    ADDR_E_RETURNCODE ComputeStereoInfo(
        const ADDR2_COMPUTE_SURFACE_INFO_INPUT* pIn,
        ADDR2_COMPUTE_SURFACE_INFO_OUTPUT*      pOut,
        UINT_32*                                pHeightAlign) const;

    UINT_32 GetMipChainInfo(
        AddrResourceType  resourceType,
        AddrSwizzleMode   swizzleMode,
        UINT_32           bpp,
        UINT_32           mip0Width,
        UINT_32           mip0Height,
        UINT_32           mip0Depth,
        UINT_32           blockWidth,
        UINT_32           blockHeight,
        UINT_32           blockDepth,
        UINT_32           numMipLevel,
        ADDR2_MIP_INFO*   pMipInfo) const;

    VOID GetMetaMiptailInfo(
        ADDR2_META_MIP_INFO*    pInfo,
        Dim3d                   mipCoord,
        UINT_32                 numMipInTail,
        Dim3d*                  pMetaBlkDim) const;

    Dim3d GetMipStartPos(
        AddrResourceType  resourceType,
        AddrSwizzleMode   swizzleMode,
        UINT_32           width,
        UINT_32           height,
        UINT_32           depth,
        UINT_32           blockWidth,
        UINT_32           blockHeight,
        UINT_32           blockDepth,
        UINT_32           mipId,
        UINT_32           log2ElementBytes,
        UINT_32*          pMipTailBytesOffset) const;

    AddrMajorMode GetMajorMode(
        AddrResourceType resourceType,
        AddrSwizzleMode  swizzleMode,
        UINT_32          mip0WidthInBlk,
        UINT_32          mip0HeightInBlk,
        UINT_32          mip0DepthInBlk) const
    {
        BOOL_32 yMajor = (mip0WidthInBlk < mip0HeightInBlk);
        BOOL_32 xMajor = (yMajor == FALSE);

        if (IsThick(resourceType, swizzleMode))
        {
            yMajor = yMajor && (mip0HeightInBlk >= mip0DepthInBlk);
            xMajor = xMajor && (mip0WidthInBlk >= mip0DepthInBlk);
        }

        AddrMajorMode majorMode;
        if (xMajor)
        {
            majorMode = ADDR_MAJOR_X;
        }
        else if (yMajor)
        {
            majorMode = ADDR_MAJOR_Y;
        }
        else
        {
            majorMode = ADDR_MAJOR_Z;
        }

        return majorMode;
    }

    Dim3d GetDccCompressBlk(
        AddrResourceType resourceType,
        AddrSwizzleMode  swizzleMode,
        UINT_32          bpp) const
    {
        UINT_32 index = Log2(bpp >> 3);
        Dim3d   compressBlkDim;

        if (IsThin(resourceType, swizzleMode))
        {
            compressBlkDim.w = Block256_2d[index].w;
            compressBlkDim.h = Block256_2d[index].h;
            compressBlkDim.d = 1;
        }
        else if (IsStandardSwizzle(resourceType, swizzleMode))
        {
            compressBlkDim = Block256_3dS[index];
        }
        else
        {
            compressBlkDim = Block256_3dZ[index];
        }

        return compressBlkDim;
    }

    static const UINT_32 MaxSeLog2      = 3;
    static const UINT_32 MaxRbPerSeLog2 = 2;

    static const Dim3d   Block256_3dS[MaxNumOfBpp];
    static const Dim3d   Block256_3dZ[MaxNumOfBpp];

    static const UINT_32 MipTailOffset256B[];

    static const SwizzleModeFlags SwizzleModeTable[ADDR_SW_MAX_TYPE];

    static const UINT_32 MaxCachedMetaEq = 2;

    Gfx9ChipSettings m_settings;

    CoordEq      m_cachedMetaEq[MaxCachedMetaEq];
    MetaEqParams m_cachedMetaEqKey[MaxCachedMetaEq];
    UINT_32      m_metaEqOverrideIndex;
};

} // V2
} // Addr

#endif

