/*
************************************************************************************************************************
*
*  Copyright (C) 2007-2022 Advanced Micro Devices, Inc.  All rights reserved.
*  SPDX-License-Identifier: MIT
*
***********************************************************************************************************************/

/**
****************************************************************************************************
* @file  addrlib.h
* @brief Contains the Addr::Lib base class definition.
****************************************************************************************************
*/

#ifndef __ADDR_LIB_H__
#define __ADDR_LIB_H__

#include "addrinterface.h"
#include "addrtypes.h"
#include "addrobject.h"
#include "addrelemlib.h"

#include "amdgpu_asic_addr.h"

#ifndef CIASICIDGFXENGINE_R600
#define CIASICIDGFXENGINE_R600 0x00000006
#endif

#ifndef CIASICIDGFXENGINE_R800
#define CIASICIDGFXENGINE_R800 0x00000008
#endif

#ifndef CIASICIDGFXENGINE_SOUTHERNISLAND
#define CIASICIDGFXENGINE_SOUTHERNISLAND 0x0000000A
#endif

#ifndef CIASICIDGFXENGINE_ARCTICISLAND
#define CIASICIDGFXENGINE_ARCTICISLAND 0x0000000D
#endif

namespace Addr
{

/**
****************************************************************************************************
* @brief Neutral enums that define pipeinterleave
****************************************************************************************************
*/
enum PipeInterleave
{
    ADDR_PIPEINTERLEAVE_256B = 256,
    ADDR_PIPEINTERLEAVE_512B = 512,
    ADDR_PIPEINTERLEAVE_1KB  = 1024,
    ADDR_PIPEINTERLEAVE_2KB  = 2048,
};

/**
****************************************************************************************************
* @brief Neutral enums that define DRAM row size
****************************************************************************************************
*/
enum RowSize
{
    ADDR_ROWSIZE_1KB = 1024,
    ADDR_ROWSIZE_2KB = 2048,
    ADDR_ROWSIZE_4KB = 4096,
    ADDR_ROWSIZE_8KB = 8192,
};

/**
****************************************************************************************************
* @brief Neutral enums that define bank interleave
****************************************************************************************************
*/
enum BankInterleave
{
    ADDR_BANKINTERLEAVE_1 = 1,
    ADDR_BANKINTERLEAVE_2 = 2,
    ADDR_BANKINTERLEAVE_4 = 4,
    ADDR_BANKINTERLEAVE_8 = 8,
};

/**
****************************************************************************************************
* @brief Neutral enums that define shader engine tile size
****************************************************************************************************
*/
enum ShaderEngineTileSize
{
    ADDR_SE_TILESIZE_16 = 16,
    ADDR_SE_TILESIZE_32 = 32,
};

/**
****************************************************************************************************
* @brief Neutral enums that define bank swap size
****************************************************************************************************
*/
enum BankSwapSize
{
    ADDR_BANKSWAP_128B = 128,
    ADDR_BANKSWAP_256B = 256,
    ADDR_BANKSWAP_512B = 512,
    ADDR_BANKSWAP_1KB = 1024,
};

/**
****************************************************************************************************
* @brief Enums that define max compressed fragments config
****************************************************************************************************
*/
enum NumMaxCompressedFragmentsConfig
{
    ADDR_CONFIG_1_MAX_COMPRESSED_FRAGMENTS   = 0x00000000,
    ADDR_CONFIG_2_MAX_COMPRESSED_FRAGMENTS   = 0x00000001,
    ADDR_CONFIG_4_MAX_COMPRESSED_FRAGMENTS   = 0x00000002,
    ADDR_CONFIG_8_MAX_COMPRESSED_FRAGMENTS   = 0x00000003,
};

/**
****************************************************************************************************
* @brief Enums that define num pipes config
****************************************************************************************************
*/
enum NumPipesConfig
{
    ADDR_CONFIG_1_PIPE                       = 0x00000000,
    ADDR_CONFIG_2_PIPE                       = 0x00000001,
    ADDR_CONFIG_4_PIPE                       = 0x00000002,
    ADDR_CONFIG_8_PIPE                       = 0x00000003,
    ADDR_CONFIG_16_PIPE                      = 0x00000004,
    ADDR_CONFIG_32_PIPE                      = 0x00000005,
    ADDR_CONFIG_64_PIPE                      = 0x00000006,
};

/**
****************************************************************************************************
* @brief Enums that define num banks config
****************************************************************************************************
*/
enum NumBanksConfig
{
    ADDR_CONFIG_1_BANK                       = 0x00000000,
    ADDR_CONFIG_2_BANK                       = 0x00000001,
    ADDR_CONFIG_4_BANK                       = 0x00000002,
    ADDR_CONFIG_8_BANK                       = 0x00000003,
    ADDR_CONFIG_16_BANK                      = 0x00000004,
};

/**
****************************************************************************************************
* @brief Enums that define num rb per shader engine config
****************************************************************************************************
*/
enum NumRbPerShaderEngineConfig
{
    ADDR_CONFIG_1_RB_PER_SHADER_ENGINE       = 0x00000000,
    ADDR_CONFIG_2_RB_PER_SHADER_ENGINE       = 0x00000001,
    ADDR_CONFIG_4_RB_PER_SHADER_ENGINE       = 0x00000002,
};

/**
****************************************************************************************************
* @brief Enums that define num shader engines config
****************************************************************************************************
*/
enum NumShaderEnginesConfig
{
    ADDR_CONFIG_1_SHADER_ENGINE              = 0x00000000,
    ADDR_CONFIG_2_SHADER_ENGINE              = 0x00000001,
    ADDR_CONFIG_4_SHADER_ENGINE              = 0x00000002,
    ADDR_CONFIG_8_SHADER_ENGINE              = 0x00000003,
};

/**
****************************************************************************************************
* @brief Enums that define pipe interleave size config
****************************************************************************************************
*/
enum PipeInterleaveSizeConfig
{
    ADDR_CONFIG_PIPE_INTERLEAVE_256B         = 0x00000000,
    ADDR_CONFIG_PIPE_INTERLEAVE_512B         = 0x00000001,
    ADDR_CONFIG_PIPE_INTERLEAVE_1KB          = 0x00000002,
    ADDR_CONFIG_PIPE_INTERLEAVE_2KB          = 0x00000003,
};

/**
****************************************************************************************************
* @brief Enums that define row size config
****************************************************************************************************
*/
enum RowSizeConfig
{
    ADDR_CONFIG_1KB_ROW                      = 0x00000000,
    ADDR_CONFIG_2KB_ROW                      = 0x00000001,
    ADDR_CONFIG_4KB_ROW                      = 0x00000002,
};

/**
****************************************************************************************************
* @brief Enums that define bank interleave size config
****************************************************************************************************
*/
enum BankInterleaveSizeConfig
{
    ADDR_CONFIG_BANK_INTERLEAVE_1            = 0x00000000,
    ADDR_CONFIG_BANK_INTERLEAVE_2            = 0x00000001,
    ADDR_CONFIG_BANK_INTERLEAVE_4            = 0x00000002,
    ADDR_CONFIG_BANK_INTERLEAVE_8            = 0x00000003,
};

/**
****************************************************************************************************
* @brief Enums that define engine tile size config
****************************************************************************************************
*/
enum ShaderEngineTileSizeConfig
{
    ADDR_CONFIG_SE_TILE_16                   = 0x00000000,
    ADDR_CONFIG_SE_TILE_32                   = 0x00000001,
};

/**
****************************************************************************************************
* @brief This class contains asic independent address lib functionalities
****************************************************************************************************
*/
class Lib : public Object
{
public:
    virtual ~Lib();

    static ADDR_E_RETURNCODE Create(
        const ADDR_CREATE_INPUT* pCreateInfo, ADDR_CREATE_OUTPUT* pCreateOut);

    /// Pair of Create
    VOID Destroy()
    {
        delete this;
    }

    static Lib* GetLib(ADDR_HANDLE hLib);

    /// Returns AddrLib version (from compiled binary instead include file)
    UINT_32 GetVersion()
    {
        return m_version;
    }

    /// Returns asic chip family name defined by AddrLib
    ChipFamily GetChipFamily() const
    {
        return m_chipFamily;
    }

    ADDR_E_RETURNCODE Flt32ToDepthPixel(
        const ELEM_FLT32TODEPTHPIXEL_INPUT* pIn,
        ELEM_FLT32TODEPTHPIXEL_OUTPUT* pOut) const;

    ADDR_E_RETURNCODE Flt32ToColorPixel(
        const ELEM_FLT32TOCOLORPIXEL_INPUT* pIn,
        ELEM_FLT32TOCOLORPIXEL_OUTPUT* pOut) const;

    BOOL_32 GetExportNorm(const ELEM_GETEXPORTNORM_INPUT* pIn) const;

    ADDR_E_RETURNCODE GetMaxAlignments(ADDR_GET_MAX_ALIGNMENTS_OUTPUT* pOut) const;

    ADDR_E_RETURNCODE GetMaxMetaAlignments(ADDR_GET_MAX_ALIGNMENTS_OUTPUT* pOut) const;

    UINT_32 GetBpe(AddrFormat format) const;

protected:
    Lib();  // Constructor is protected
    Lib(const Client* pClient);

    /// Pure virtual function to get max base alignments
    virtual UINT_32 HwlComputeMaxBaseAlignments() const = 0;

    /// Gets maximum alignements for metadata
    virtual UINT_32 HwlComputeMaxMetaBaseAlignments() const
    {
        ADDR_NOT_IMPLEMENTED();

        return 0;
    }

    VOID ValidBaseAlignments(UINT_32 alignment) const
    {
#if DEBUG
        ADDR_ASSERT(alignment <= m_maxBaseAlign);
#endif
    }

    VOID ValidMetaBaseAlignments(UINT_32 metaAlignment) const
    {
#if DEBUG
        ADDR_ASSERT(metaAlignment <= m_maxMetaBaseAlign);
#endif
    }

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

    //
    // Initialization
    //
    /// Pure Virtual function for Hwl computing internal global parameters from h/w registers
    virtual BOOL_32 HwlInitGlobalParams(const ADDR_CREATE_INPUT* pCreateIn) = 0;

    /// Pure Virtual function for Hwl converting chip family
    virtual ChipFamily HwlConvertChipFamily(UINT_32 uChipFamily, UINT_32 uChipRevision) = 0;

    /// Get equation table pointer and number of equations
    virtual UINT_32 HwlGetEquationTableInfo(const ADDR_EQUATION** ppEquationTable) const
    {
        *ppEquationTable = NULL;

        return 0;
    }

    //
    // Misc helper
    //
    static UINT_32 Bits2Number(UINT_32 bitNum, ...);

    static UINT_32 GetNumFragments(UINT_32 numSamples, UINT_32 numFrags)
    {
        return (numFrags != 0) ? numFrags : Max(1u, numSamples);
    }

    /// Returns pointer of ElemLib
    ElemLib* GetElemLib() const
    {
        return m_pElemLib;
    }

    /// Returns fillSizeFields flag
    UINT_32 GetFillSizeFieldsFlags() const
    {
        return m_configFlags.fillSizeFields;
    }

private:
    // Disallow the copy constructor
    Lib(const Lib& a);

    // Disallow the assignment operator
    Lib& operator=(const Lib& a);

    VOID SetChipFamily(UINT_32 uChipFamily, UINT_32 uChipRevision);

    VOID SetMinPitchAlignPixels(UINT_32 minPitchAlignPixels);

    VOID SetMaxAlignments();

protected:
    ChipFamily  m_chipFamily;   ///< Chip family translated from the one in atiid.h

    UINT_32     m_chipRevision; ///< Revision id from xxx_id.h

    UINT_32     m_version;      ///< Current version

    //
    // Global parameters
    //
    ConfigFlags m_configFlags;          ///< Global configuration flags. Note this is setup by
                                        ///  AddrLib instead of Client except forceLinearAligned

    UINT_32     m_pipes;                ///< Number of pipes
    UINT_32     m_banks;                ///< Number of banks
                                        ///  For r800 this is MC_ARB_RAMCFG.NOOFBANK
                                        ///  Keep it here to do default parameter calculation

    UINT_32     m_pipeInterleaveBytes;
                                        ///< Specifies the size of contiguous address space
                                        ///  within each tiling pipe when making linear
                                        ///  accesses. (Formerly Group Size)

    UINT_32     m_rowSize;              ///< DRAM row size, in bytes

    UINT_32     m_minPitchAlignPixels;  ///< Minimum pitch alignment in pixels
    UINT_32     m_maxSamples;           ///< Max numSamples

    UINT_32     m_maxBaseAlign;         ///< Max base alignment for data surface
    UINT_32     m_maxMetaBaseAlign;     ///< Max base alignment for metadata

private:
    ElemLib*    m_pElemLib;             ///< Element Lib pointer
};

Lib* SiHwlInit   (const Client* pClient);
Lib* CiHwlInit   (const Client* pClient);
Lib* Gfx9HwlInit (const Client* pClient);
Lib* Gfx10HwlInit(const Client* pClient);
Lib* Gfx11HwlInit(const Client* pClient);
} // Addr

#endif
