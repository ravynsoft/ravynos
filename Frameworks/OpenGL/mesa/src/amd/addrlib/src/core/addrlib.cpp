/*
************************************************************************************************************************
*
*  Copyright (C) 2007-2022 Advanced Micro Devices, Inc.  All rights reserved.
*  SPDX-License-Identifier: MIT
*
***********************************************************************************************************************/

/**
****************************************************************************************************
* @file  addrlib.cpp
* @brief Contains the implementation for the Addr::Lib class.
****************************************************************************************************
*/

#include "addrinterface.h"
#include "addrlib.h"
#include "addrcommon.h"

#if defined(__APPLE__)

UINT_32 div64_32(UINT_64 n, UINT_32 base)
{
    UINT_64 rem = n;
    UINT_64 b = base;
    UINT_64 res, d = 1;
    UINT_32 high = rem >> 32;

    res = 0;
    if (high >= base)
    {
        high /= base;
        res = (UINT_64) high << 32;
        rem -= (UINT_64) (high * base) << 32;
    }

    while (((INT_64)b > 0) && (b < rem))
    {
        b = b + b;
        d = d + d;
    }

    do
    {
        if (rem >= b)
        {
            rem -= b;
            res += d;
        }
        b >>= 1;
        d >>= 1;
    } while (d);

    n = res;
    return rem;
}

extern "C"
UINT_32 __umoddi3(UINT_64 n, UINT_32 base)
{
    return div64_32(n, base);
}

#endif // __APPLE__

namespace Addr
{

////////////////////////////////////////////////////////////////////////////////////////////////////
//                               Constructor/Destructor
////////////////////////////////////////////////////////////////////////////////////////////////////

/**
****************************************************************************************************
*   Lib::Lib
*
*   @brief
*       Constructor for the AddrLib class
*
****************************************************************************************************
*/
Lib::Lib() :
    m_chipFamily(ADDR_CHIP_FAMILY_IVLD),
    m_chipRevision(0),
    m_version(ADDRLIB_VERSION),
    m_pipes(0),
    m_banks(0),
    m_pipeInterleaveBytes(0),
    m_rowSize(0),
    m_minPitchAlignPixels(1),
    m_maxSamples(8),
    m_maxBaseAlign(0),
    m_maxMetaBaseAlign(0),
    m_pElemLib(NULL)
{
    m_configFlags.value = 0;
}

/**
****************************************************************************************************
*   Lib::Lib
*
*   @brief
*       Constructor for the AddrLib class with hClient as parameter
*
****************************************************************************************************
*/
Lib::Lib(const Client* pClient) :
    Object(pClient),
    m_chipFamily(ADDR_CHIP_FAMILY_IVLD),
    m_chipRevision(0),
    m_version(ADDRLIB_VERSION),
    m_pipes(0),
    m_banks(0),
    m_pipeInterleaveBytes(0),
    m_rowSize(0),
    m_minPitchAlignPixels(1),
    m_maxSamples(8),
    m_maxBaseAlign(0),
    m_maxMetaBaseAlign(0),
    m_pElemLib(NULL)
{
    m_configFlags.value = 0;
}

/**
****************************************************************************************************
*   Lib::~AddrLib
*
*   @brief
*       Destructor for the AddrLib class
*
****************************************************************************************************
*/
Lib::~Lib()
{
    if (m_pElemLib)
    {
        delete m_pElemLib;
        m_pElemLib = NULL;
    }
}


////////////////////////////////////////////////////////////////////////////////////////////////////
//                               Initialization/Helper
////////////////////////////////////////////////////////////////////////////////////////////////////

/**
****************************************************************************************************
*   Lib::Create
*
*   @brief
*       Creates and initializes AddrLib object.
*
*   @return
*       ADDR_E_RETURNCODE
****************************************************************************************************
*/
ADDR_E_RETURNCODE Lib::Create(
    const ADDR_CREATE_INPUT* pCreateIn,     ///< [in] pointer to ADDR_CREATE_INPUT
    ADDR_CREATE_OUTPUT*      pCreateOut)    ///< [out] pointer to ADDR_CREATE_OUTPUT
{
    Lib* pLib = NULL;
    ADDR_E_RETURNCODE returnCode = ADDR_OK;

    if (pCreateIn->createFlags.fillSizeFields == TRUE)
    {
        if ((pCreateIn->size != sizeof(ADDR_CREATE_INPUT)) ||
            (pCreateOut->size != sizeof(ADDR_CREATE_OUTPUT)))
        {
            returnCode = ADDR_PARAMSIZEMISMATCH;
        }
    }

    if ((returnCode == ADDR_OK)                    &&
        (pCreateIn->callbacks.allocSysMem != NULL) &&
        (pCreateIn->callbacks.freeSysMem != NULL))
    {
        Client client = {
            pCreateIn->hClient,
            pCreateIn->callbacks
        };

        switch (pCreateIn->chipEngine)
        {
            case CIASICIDGFXENGINE_SOUTHERNISLAND:
                switch (pCreateIn->chipFamily)
                {
                    case FAMILY_SI:
                        pLib = SiHwlInit(&client);
                        break;
                    case FAMILY_VI:
                    case FAMILY_CZ: // VI based fusion
                    case FAMILY_CI:
                    case FAMILY_KV: // CI based fusion
                        pLib = CiHwlInit(&client);
                        break;
                    default:
                        ADDR_ASSERT_ALWAYS();
                        break;
                }
                break;
            case CIASICIDGFXENGINE_ARCTICISLAND:
                switch (pCreateIn->chipFamily)
                {
                    case FAMILY_AI:
                    case FAMILY_RV:
                        pLib = Gfx9HwlInit(&client);
                        break;
                    case FAMILY_NV:
                    case FAMILY_VGH:
                    case FAMILY_RMB:
                    case FAMILY_RPL:
                    case FAMILY_MDN:
                        pLib = Gfx10HwlInit(&client);
                        break;
                    case FAMILY_NV3:
                    case FAMILY_GFX1103:
                    case FAMILY_GFX1150:
                        pLib = Gfx11HwlInit(&client);
                        break;
                    default:
                        ADDR_ASSERT_ALWAYS();
                        break;
                }
                break;
            default:
                ADDR_ASSERT_ALWAYS();
                break;
        }
    }

    if (pLib != NULL)
    {
        BOOL_32 initValid;

        // Pass createFlags to configFlags first since these flags may be overwritten
        pLib->m_configFlags.noCubeMipSlicesPad  = pCreateIn->createFlags.noCubeMipSlicesPad;
        pLib->m_configFlags.fillSizeFields      = pCreateIn->createFlags.fillSizeFields;
        pLib->m_configFlags.useTileIndex        = pCreateIn->createFlags.useTileIndex;
        pLib->m_configFlags.useCombinedSwizzle  = pCreateIn->createFlags.useCombinedSwizzle;
        pLib->m_configFlags.checkLast2DLevel    = pCreateIn->createFlags.checkLast2DLevel;
        pLib->m_configFlags.useHtileSliceAlign  = pCreateIn->createFlags.useHtileSliceAlign;
        pLib->m_configFlags.allowLargeThickTile = pCreateIn->createFlags.allowLargeThickTile;
        pLib->m_configFlags.forceDccAndTcCompat = pCreateIn->createFlags.forceDccAndTcCompat;
        pLib->m_configFlags.nonPower2MemConfig  = pCreateIn->createFlags.nonPower2MemConfig;
        pLib->m_configFlags.enableAltTiling     = pCreateIn->createFlags.enableAltTiling;
        pLib->m_configFlags.disableLinearOpt    = FALSE;

        pLib->SetChipFamily(pCreateIn->chipFamily, pCreateIn->chipRevision);

        pLib->SetMinPitchAlignPixels(pCreateIn->minPitchAlignPixels);

        // Global parameters initialized and remaining configFlags bits are set as well
        initValid = pLib->HwlInitGlobalParams(pCreateIn);

        if (initValid)
        {
            pLib->m_pElemLib = ElemLib::Create(pLib);
        }
        else
        {
            pLib->m_pElemLib = NULL; // Don't go on allocating element lib
            returnCode = ADDR_INVALIDGBREGVALUES;
        }

        if (pLib->m_pElemLib == NULL)
        {
            delete pLib;
            pLib = NULL;
            ADDR_ASSERT_ALWAYS();
        }
        else
        {
            pLib->m_pElemLib->SetConfigFlags(pLib->m_configFlags);
        }
    }

    pCreateOut->hLib = pLib;

    if ((pLib != NULL) &&
        (returnCode == ADDR_OK))
    {
        pCreateOut->numEquations =
            pLib->HwlGetEquationTableInfo(&pCreateOut->pEquationTable);

        pLib->SetMaxAlignments();

    }
    else if ((pLib == NULL) &&
             (returnCode == ADDR_OK))
    {
        // Unknown failures, we return the general error code
        returnCode = ADDR_ERROR;
    }

    return returnCode;
}

/**
****************************************************************************************************
*   Lib::SetChipFamily
*
*   @brief
*       Convert familyID defined in atiid.h to ChipFamily and set m_chipFamily/m_chipRevision
*   @return
*      N/A
****************************************************************************************************
*/
VOID Lib::SetChipFamily(
    UINT_32 uChipFamily,        ///< [in] chip family defined in atiih.h
    UINT_32 uChipRevision)      ///< [in] chip revision defined in "asic_family"_id.h
{
    ChipFamily family = HwlConvertChipFamily(uChipFamily, uChipRevision);

    ADDR_ASSERT(family != ADDR_CHIP_FAMILY_IVLD);

    m_chipFamily   = family;
    m_chipRevision = uChipRevision;
}

/**
****************************************************************************************************
*   Lib::SetMinPitchAlignPixels
*
*   @brief
*       Set m_minPitchAlignPixels with input param
*
*   @return
*      N/A
****************************************************************************************************
*/
VOID Lib::SetMinPitchAlignPixels(
    UINT_32 minPitchAlignPixels)    ///< [in] minmum pitch alignment in pixels
{
    m_minPitchAlignPixels = (minPitchAlignPixels == 0) ? 1 : minPitchAlignPixels;
}

/**
****************************************************************************************************
*   Lib::SetMaxAlignments
*
*   @brief
*       Set max alignments
*
*   @return
*      N/A
****************************************************************************************************
*/
VOID Lib::SetMaxAlignments()
{
    m_maxBaseAlign     = HwlComputeMaxBaseAlignments();
    m_maxMetaBaseAlign = HwlComputeMaxMetaBaseAlignments();
}

/**
****************************************************************************************************
*   Lib::GetLib
*
*   @brief
*       Get AddrLib pointer
*
*   @return
*      An AddrLib class pointer
****************************************************************************************************
*/
Lib* Lib::GetLib(
    ADDR_HANDLE hLib)   ///< [in] handle of ADDR_HANDLE
{
    return static_cast<Addr::Lib*>(hLib);
}

/**
****************************************************************************************************
*   Lib::GetMaxAlignments
*
*   @brief
*       Gets maximum alignments for data surface (include FMask)
*
*   @return
*       ADDR_E_RETURNCODE
****************************************************************************************************
*/
ADDR_E_RETURNCODE Lib::GetMaxAlignments(
    ADDR_GET_MAX_ALIGNMENTS_OUTPUT* pOut    ///< [out] output structure
    ) const
{
    ADDR_E_RETURNCODE returnCode = ADDR_OK;

    if (GetFillSizeFieldsFlags() == TRUE)
    {
        if (pOut->size != sizeof(ADDR_GET_MAX_ALIGNMENTS_OUTPUT))
        {
            returnCode = ADDR_PARAMSIZEMISMATCH;
        }
    }

    if (returnCode == ADDR_OK)
    {
        if (m_maxBaseAlign != 0)
        {
            pOut->baseAlign = m_maxBaseAlign;
        }
        else
        {
            returnCode = ADDR_NOTIMPLEMENTED;
        }
    }

    return returnCode;
}

/**
****************************************************************************************************
*   Lib::GetMaxMetaAlignments
*
*   @brief
*       Gets maximum alignments for metadata (CMask, DCC and HTile)
*
*   @return
*       ADDR_E_RETURNCODE
****************************************************************************************************
*/
ADDR_E_RETURNCODE Lib::GetMaxMetaAlignments(
    ADDR_GET_MAX_ALIGNMENTS_OUTPUT* pOut    ///< [out] output structure
    ) const
{
    ADDR_E_RETURNCODE returnCode = ADDR_OK;

    if (GetFillSizeFieldsFlags() == TRUE)
    {
        if (pOut->size != sizeof(ADDR_GET_MAX_ALIGNMENTS_OUTPUT))
        {
            returnCode = ADDR_PARAMSIZEMISMATCH;
        }
    }

    if (returnCode == ADDR_OK)
    {
        if (m_maxMetaBaseAlign != 0)
        {
            pOut->baseAlign = m_maxMetaBaseAlign;
        }
        else
        {
            returnCode = ADDR_NOTIMPLEMENTED;
        }
    }

    return returnCode;
}

/**
****************************************************************************************************
*   Lib::Bits2Number
*
*   @brief
*       Cat a array of binary bit to a number
*
*   @return
*       The number combined with the array of bits
****************************************************************************************************
*/
UINT_32 Lib::Bits2Number(
    UINT_32 bitNum,     ///< [in] how many bits
    ...)                ///< [in] varaible bits value starting from MSB
{
    UINT_32 number = 0;
    UINT_32 i;
    va_list bits_ptr;

    va_start(bits_ptr, bitNum);

    for(i = 0; i < bitNum; i++)
    {
        number |= va_arg(bits_ptr, UINT_32);
        number <<= 1;
    }

    number >>= 1;

    va_end(bits_ptr);

    return number;
}


////////////////////////////////////////////////////////////////////////////////////////////////////
//                               Element lib
////////////////////////////////////////////////////////////////////////////////////////////////////


/**
****************************************************************************************************
*   Lib::Flt32ToColorPixel
*
*   @brief
*       Convert a FLT_32 value to a depth/stencil pixel value
*   @return
*       ADDR_E_RETURNCODE
****************************************************************************************************
*/
ADDR_E_RETURNCODE Lib::Flt32ToDepthPixel(
    const ELEM_FLT32TODEPTHPIXEL_INPUT* pIn,
    ELEM_FLT32TODEPTHPIXEL_OUTPUT* pOut) const
{
    ADDR_E_RETURNCODE returnCode = ADDR_OK;

    if (GetFillSizeFieldsFlags() == TRUE)
    {
        if ((pIn->size != sizeof(ELEM_FLT32TODEPTHPIXEL_INPUT)) ||
            (pOut->size != sizeof(ELEM_FLT32TODEPTHPIXEL_OUTPUT)))
        {
            returnCode = ADDR_PARAMSIZEMISMATCH;
        }
    }

    if (returnCode == ADDR_OK)
    {
        GetElemLib()->Flt32ToDepthPixel(pIn->format, pIn->comps, pOut->pPixel);

        UINT_32 depthBase = 0;
        UINT_32 stencilBase = 0;
        UINT_32 depthBits = 0;
        UINT_32 stencilBits = 0;

        switch (pIn->format)
        {
            case ADDR_DEPTH_16:
                depthBits = 16;
                break;
            case ADDR_DEPTH_X8_24:
            case ADDR_DEPTH_8_24:
            case ADDR_DEPTH_X8_24_FLOAT:
            case ADDR_DEPTH_8_24_FLOAT:
                depthBase = 8;
                depthBits = 24;
                stencilBits = 8;
                break;
            case ADDR_DEPTH_32_FLOAT:
                depthBits = 32;
                break;
            case ADDR_DEPTH_X24_8_32_FLOAT:
                depthBase = 8;
                depthBits = 32;
                stencilBits = 8;
                break;
            default:
                break;
        }

        // Overwrite base since R800 has no "tileBase"
        if (GetElemLib()->IsDepthStencilTilePlanar() == FALSE)
        {
            depthBase = 0;
            stencilBase = 0;
        }

        depthBase *= 64;
        stencilBase *= 64;

        pOut->stencilBase = stencilBase;
        pOut->depthBase = depthBase;
        pOut->depthBits = depthBits;
        pOut->stencilBits = stencilBits;
    }

    return returnCode;
}

/**
****************************************************************************************************
*   Lib::Flt32ToColorPixel
*
*   @brief
*       Convert a FLT_32 value to a red/green/blue/alpha pixel value
*   @return
*       ADDR_E_RETURNCODE
****************************************************************************************************
*/
ADDR_E_RETURNCODE Lib::Flt32ToColorPixel(
    const ELEM_FLT32TOCOLORPIXEL_INPUT* pIn,
    ELEM_FLT32TOCOLORPIXEL_OUTPUT* pOut) const
{
    ADDR_E_RETURNCODE returnCode = ADDR_OK;

    if (GetFillSizeFieldsFlags() == TRUE)
    {
        if ((pIn->size != sizeof(ELEM_FLT32TOCOLORPIXEL_INPUT)) ||
            (pOut->size != sizeof(ELEM_FLT32TOCOLORPIXEL_OUTPUT)))
        {
            returnCode = ADDR_PARAMSIZEMISMATCH;
        }
    }

    if (returnCode == ADDR_OK)
    {
        GetElemLib()->Flt32ToColorPixel(pIn->format,
                                        pIn->surfNum,
                                        pIn->surfSwap,
                                        pIn->comps,
                                        pOut->pPixel);
    }

    return returnCode;
}


/**
****************************************************************************************************
*   Lib::GetExportNorm
*
*   @brief
*       Check one format can be EXPORT_NUM
*   @return
*       TRUE if EXPORT_NORM can be used
****************************************************************************************************
*/
BOOL_32 Lib::GetExportNorm(
    const ELEM_GETEXPORTNORM_INPUT* pIn) const
{
    ADDR_E_RETURNCODE returnCode = ADDR_OK;

    BOOL_32 enabled = FALSE;

    if (GetFillSizeFieldsFlags() == TRUE)
    {
        if (pIn->size != sizeof(ELEM_GETEXPORTNORM_INPUT))
        {
            returnCode = ADDR_PARAMSIZEMISMATCH;
        }
    }

    if (returnCode == ADDR_OK)
    {
        enabled = GetElemLib()->PixGetExportNorm(pIn->format, pIn->num, pIn->swap);
    }

    return enabled;
}

/**
****************************************************************************************************
*   Lib::GetBpe
*
*   @brief
*       Get bits-per-element for specified format
*   @return
*       bits-per-element of specified format
****************************************************************************************************
*/
UINT_32 Lib::GetBpe(AddrFormat format) const
{
    return GetElemLib()->GetBitsPerPixel(format);
}

} // Addr
