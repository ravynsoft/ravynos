/*
************************************************************************************************************************
*
*  Copyright (C) 2007-2022 Advanced Micro Devices, Inc.  All rights reserved.
*  SPDX-License-Identifier: MIT
*
***********************************************************************************************************************/

/**
****************************************************************************************************
* @file  addrinterface.cpp
* @brief Contains the addrlib interface functions
****************************************************************************************************
*/
#include "addrinterface.h"
#include "addrlib1.h"
#include "addrlib2.h"

#include "addrcommon.h"

using namespace Addr;

////////////////////////////////////////////////////////////////////////////////////////////////////
//                               Create/Destroy/Config functions
////////////////////////////////////////////////////////////////////////////////////////////////////

/**
****************************************************************************************************
*   AddrCreate
*
*   @brief
*       Create address lib object
*
*   @return
*       ADDR_OK if successful, otherwise an error code of ADDR_E_RETURNCODE
****************************************************************************************************
*/
ADDR_E_RETURNCODE ADDR_API AddrCreate(
    const ADDR_CREATE_INPUT*    pAddrCreateIn,  ///< [in] infomation for creating address lib object
    ADDR_CREATE_OUTPUT*         pAddrCreateOut) ///< [out] address lib handle
{
    ADDR_E_RETURNCODE returnCode = ADDR_OK;
    {
        returnCode = Lib::Create(pAddrCreateIn, pAddrCreateOut);
    }

    return returnCode;
}



/**
****************************************************************************************************
*   AddrDestroy
*
*   @brief
*       Destroy address lib object
*
*   @return
*       ADDR_OK if successful, otherwise an error code of ADDR_E_RETURNCODE
****************************************************************************************************
*/
ADDR_E_RETURNCODE ADDR_API AddrDestroy(
    ADDR_HANDLE hLib) ///< address lib handle
{
    ADDR_E_RETURNCODE returnCode = ADDR_OK;

    if (hLib)
    {
        Lib* pLib = Lib::GetLib(hLib);
        pLib->Destroy();
    }
    else
    {
        returnCode = ADDR_ERROR;
    }

    return returnCode;
}



////////////////////////////////////////////////////////////////////////////////////////////////////
//                                    Surface functions
////////////////////////////////////////////////////////////////////////////////////////////////////

/**
****************************************************************************************************
*   AddrComputeSurfaceInfo
*
*   @brief
*       Calculate surface width/height/depth/alignments and suitable tiling mode
*
*   @return
*       ADDR_OK if successful, otherwise an error code of ADDR_E_RETURNCODE
****************************************************************************************************
*/
ADDR_E_RETURNCODE ADDR_API AddrComputeSurfaceInfo(
    ADDR_HANDLE                             hLib, ///< address lib handle
    const ADDR_COMPUTE_SURFACE_INFO_INPUT*  pIn,  ///< [in] surface information
    ADDR_COMPUTE_SURFACE_INFO_OUTPUT*       pOut) ///< [out] surface parameters and alignments
{
    V1::Lib* pLib = V1::Lib::GetLib(hLib);

    ADDR_E_RETURNCODE returnCode = ADDR_OK;

    if (pLib != NULL)
    {
        returnCode = pLib->ComputeSurfaceInfo(pIn, pOut);
    }
    else
    {
        returnCode = ADDR_ERROR;
    }

    return returnCode;
}



/**
****************************************************************************************************
*   AddrComputeSurfaceAddrFromCoord
*
*   @brief
*       Compute surface address according to coordinates
*
*   @return
*       ADDR_OK if successful, otherwise an error code of ADDR_E_RETURNCODE
****************************************************************************************************
*/
ADDR_E_RETURNCODE ADDR_API AddrComputeSurfaceAddrFromCoord(
    ADDR_HANDLE                                     hLib, ///< address lib handle
    const ADDR_COMPUTE_SURFACE_ADDRFROMCOORD_INPUT* pIn,  ///< [in] surface info and coordinates
    ADDR_COMPUTE_SURFACE_ADDRFROMCOORD_OUTPUT*      pOut) ///< [out] surface address
{
    V1::Lib* pLib = V1::Lib::GetLib(hLib);

    ADDR_E_RETURNCODE returnCode = ADDR_OK;

    if (pLib != NULL)
    {
        returnCode = pLib->ComputeSurfaceAddrFromCoord(pIn, pOut);
    }
    else
    {
        returnCode = ADDR_ERROR;
    }

    return returnCode;
}

/**
****************************************************************************************************
*   AddrComputeSurfaceCoordFromAddr
*
*   @brief
*       Compute coordinates according to surface address
*
*   @return
*       ADDR_OK if successful, otherwise an error code of ADDR_E_RETURNCODE
****************************************************************************************************
*/
ADDR_E_RETURNCODE ADDR_API AddrComputeSurfaceCoordFromAddr(
    ADDR_HANDLE                                     hLib, ///< address lib handle
    const ADDR_COMPUTE_SURFACE_COORDFROMADDR_INPUT* pIn,  ///< [in] surface info and address
    ADDR_COMPUTE_SURFACE_COORDFROMADDR_OUTPUT*      pOut) ///< [out] coordinates
{
    V1::Lib* pLib = V1::Lib::GetLib(hLib);

    ADDR_E_RETURNCODE returnCode = ADDR_OK;

    if (pLib != NULL)
    {
        returnCode = pLib->ComputeSurfaceCoordFromAddr(pIn, pOut);
    }
    else
    {
        returnCode = ADDR_ERROR;
    }

    return returnCode;
}



////////////////////////////////////////////////////////////////////////////////////////////////////
//                                   HTile functions
////////////////////////////////////////////////////////////////////////////////////////////////////

/**
****************************************************************************************************
*   AddrComputeHtileInfo
*
*   @brief
*       Compute Htile pitch, height, base alignment and size in bytes
*
*   @return
*       ADDR_OK if successful, otherwise an error code of ADDR_E_RETURNCODE
****************************************************************************************************
*/
ADDR_E_RETURNCODE ADDR_API AddrComputeHtileInfo(
    ADDR_HANDLE                             hLib, ///< address lib handle
    const ADDR_COMPUTE_HTILE_INFO_INPUT*    pIn,  ///< [in] Htile information
    ADDR_COMPUTE_HTILE_INFO_OUTPUT*         pOut) ///< [out] Htile pitch, height and size in bytes
{
    V1::Lib* pLib = V1::Lib::GetLib(hLib);

    ADDR_E_RETURNCODE returnCode = ADDR_OK;

    if (pLib != NULL)
    {
        returnCode = pLib->ComputeHtileInfo(pIn, pOut);
    }
    else
    {
        returnCode = ADDR_ERROR;
    }

    return returnCode;
}

/**
****************************************************************************************************
*   AddrComputeHtileAddrFromCoord
*
*   @brief
*       Compute Htile address according to coordinates (of depth buffer)
*
*   @return
*       ADDR_OK if successful, otherwise an error code of ADDR_E_RETURNCODE
****************************************************************************************************
*/
ADDR_E_RETURNCODE ADDR_API AddrComputeHtileAddrFromCoord(
    ADDR_HANDLE                                     hLib, ///< address lib handle
    const ADDR_COMPUTE_HTILE_ADDRFROMCOORD_INPUT*   pIn,  ///< [in] Htile info and coordinates
    ADDR_COMPUTE_HTILE_ADDRFROMCOORD_OUTPUT*        pOut) ///< [out] Htile address
{
    V1::Lib* pLib = V1::Lib::GetLib(hLib);

    ADDR_E_RETURNCODE returnCode = ADDR_OK;

    if (pLib != NULL)
    {
        returnCode = pLib->ComputeHtileAddrFromCoord(pIn, pOut);
    }
    else
    {
        returnCode = ADDR_ERROR;
    }

    return returnCode;
}

/**
****************************************************************************************************
*   AddrComputeHtileCoordFromAddr
*
*   @brief
*       Compute coordinates within depth buffer (1st pixel of a micro tile) according to
*       Htile address
*
*   @return
*       ADDR_OK if successful, otherwise an error code of ADDR_E_RETURNCODE
****************************************************************************************************
*/
ADDR_E_RETURNCODE ADDR_API AddrComputeHtileCoordFromAddr(
    ADDR_HANDLE                                     hLib, ///< address lib handle
    const ADDR_COMPUTE_HTILE_COORDFROMADDR_INPUT*   pIn,  ///< [in] Htile info and address
    ADDR_COMPUTE_HTILE_COORDFROMADDR_OUTPUT*        pOut) ///< [out] Htile coordinates
{
    V1::Lib* pLib = V1::Lib::GetLib(hLib);

    ADDR_E_RETURNCODE returnCode = ADDR_OK;

    if (pLib != NULL)
    {
        returnCode = pLib->ComputeHtileCoordFromAddr(pIn, pOut);
    }
    else
    {
        returnCode = ADDR_ERROR;
    }

    return returnCode;
}



////////////////////////////////////////////////////////////////////////////////////////////////////
//                                     C-mask functions
////////////////////////////////////////////////////////////////////////////////////////////////////

/**
****************************************************************************************************
*   AddrComputeCmaskInfo
*
*   @brief
*       Compute Cmask pitch, height, base alignment and size in bytes from color buffer
*       info
*
*   @return
*       ADDR_OK if successful, otherwise an error code of ADDR_E_RETURNCODE
****************************************************************************************************
*/
ADDR_E_RETURNCODE ADDR_API AddrComputeCmaskInfo(
    ADDR_HANDLE                             hLib, ///< address lib handle
    const ADDR_COMPUTE_CMASK_INFO_INPUT*    pIn,  ///< [in] Cmask pitch and height
    ADDR_COMPUTE_CMASK_INFO_OUTPUT*         pOut) ///< [out] Cmask pitch, height and size in bytes
{
    V1::Lib* pLib = V1::Lib::GetLib(hLib);

    ADDR_E_RETURNCODE returnCode = ADDR_OK;

    if (pLib != NULL)
    {
        returnCode = pLib->ComputeCmaskInfo(pIn, pOut);
    }
    else
    {
        returnCode = ADDR_ERROR;
    }

    return returnCode;
}

/**
****************************************************************************************************
*   AddrComputeCmaskAddrFromCoord
*
*   @brief
*       Compute Cmask address according to coordinates (of MSAA color buffer)
*
*   @return
*       ADDR_OK if successful, otherwise an error code of ADDR_E_RETURNCODE
****************************************************************************************************
*/
ADDR_E_RETURNCODE ADDR_API AddrComputeCmaskAddrFromCoord(
    ADDR_HANDLE                                     hLib, ///< address lib handle
    const ADDR_COMPUTE_CMASK_ADDRFROMCOORD_INPUT*   pIn,  ///< [in] Cmask info and coordinates
    ADDR_COMPUTE_CMASK_ADDRFROMCOORD_OUTPUT*        pOut) ///< [out] Cmask address
{
    V1::Lib* pLib = V1::Lib::GetLib(hLib);

    ADDR_E_RETURNCODE returnCode = ADDR_OK;

    if (pLib != NULL)
    {
        returnCode = pLib->ComputeCmaskAddrFromCoord(pIn, pOut);
    }
    else
    {
        returnCode = ADDR_ERROR;
    }

    return returnCode;
}

/**
****************************************************************************************************
*   AddrComputeCmaskCoordFromAddr
*
*   @brief
*       Compute coordinates within color buffer (1st pixel of a micro tile) according to
*       Cmask address
*
*   @return
*       ADDR_OK if successful, otherwise an error code of ADDR_E_RETURNCODE
****************************************************************************************************
*/
ADDR_E_RETURNCODE ADDR_API AddrComputeCmaskCoordFromAddr(
    ADDR_HANDLE                                     hLib, ///< address lib handle
    const ADDR_COMPUTE_CMASK_COORDFROMADDR_INPUT*   pIn,  ///< [in] Cmask info and address
    ADDR_COMPUTE_CMASK_COORDFROMADDR_OUTPUT*        pOut) ///< [out] Cmask coordinates
{
    V1::Lib* pLib = V1::Lib::GetLib(hLib);

    ADDR_E_RETURNCODE returnCode = ADDR_OK;

    if (pLib != NULL)
    {
        returnCode = pLib->ComputeCmaskCoordFromAddr(pIn, pOut);
    }
    else
    {
        returnCode = ADDR_ERROR;
    }

    return returnCode;
}



////////////////////////////////////////////////////////////////////////////////////////////////////
//                                     F-mask functions
////////////////////////////////////////////////////////////////////////////////////////////////////

/**
****************************************************************************************************
*   AddrComputeFmaskInfo
*
*   @brief
*       Compute Fmask pitch/height/depth/alignments and size in bytes
*
*   @return
*       ADDR_OK if successful, otherwise an error code of ADDR_E_RETURNCODE
****************************************************************************************************
*/
ADDR_E_RETURNCODE ADDR_API AddrComputeFmaskInfo(
    ADDR_HANDLE                             hLib, ///< address lib handle
    const ADDR_COMPUTE_FMASK_INFO_INPUT*    pIn,  ///< [in] Fmask information
    ADDR_COMPUTE_FMASK_INFO_OUTPUT*         pOut) ///< [out] Fmask pitch and height
{
    V1::Lib* pLib = V1::Lib::GetLib(hLib);

    ADDR_E_RETURNCODE returnCode = ADDR_OK;

    if (pLib != NULL)
    {
        returnCode = pLib->ComputeFmaskInfo(pIn, pOut);
    }
    else
    {
        returnCode = ADDR_ERROR;
    }

    return returnCode;
}

/**
****************************************************************************************************
*   AddrComputeFmaskAddrFromCoord
*
*   @brief
*       Compute Fmask address according to coordinates (x,y,slice,sample,plane)
*
*   @return
*       ADDR_OK if successful, otherwise an error code of ADDR_E_RETURNCODE
****************************************************************************************************
*/
ADDR_E_RETURNCODE ADDR_API AddrComputeFmaskAddrFromCoord(
    ADDR_HANDLE                                     hLib, ///< address lib handle
    const ADDR_COMPUTE_FMASK_ADDRFROMCOORD_INPUT*   pIn,  ///< [in] Fmask info and coordinates
    ADDR_COMPUTE_FMASK_ADDRFROMCOORD_OUTPUT*        pOut) ///< [out] Fmask address
{
    V1::Lib* pLib = V1::Lib::GetLib(hLib);

    ADDR_E_RETURNCODE returnCode = ADDR_OK;

    if (pLib != NULL)
    {
        returnCode = pLib->ComputeFmaskAddrFromCoord(pIn, pOut);
    }
    else
    {
        returnCode = ADDR_ERROR;
    }

    return returnCode;
}

/**
****************************************************************************************************
*   AddrComputeFmaskCoordFromAddr
*
*   @brief
*       Compute coordinates (x,y,slice,sample,plane) according to Fmask address
*
*   @return
*       ADDR_OK if successful, otherwise an error code of ADDR_E_RETURNCODE
****************************************************************************************************
*/
ADDR_E_RETURNCODE ADDR_API AddrComputeFmaskCoordFromAddr(
    ADDR_HANDLE                                     hLib, ///< address lib handle
    const ADDR_COMPUTE_FMASK_COORDFROMADDR_INPUT*   pIn,  ///< [in] Fmask info and address
    ADDR_COMPUTE_FMASK_COORDFROMADDR_OUTPUT*        pOut) ///< [out] Fmask coordinates
{
    V1::Lib* pLib = V1::Lib::GetLib(hLib);

    ADDR_E_RETURNCODE returnCode = ADDR_OK;

    if (pLib != NULL)
    {
        returnCode = pLib->ComputeFmaskCoordFromAddr(pIn, pOut);
    }
    else
    {
        returnCode = ADDR_ERROR;
    }

    return returnCode;
}



////////////////////////////////////////////////////////////////////////////////////////////////////
//                                     DCC key functions
////////////////////////////////////////////////////////////////////////////////////////////////////

/**
****************************************************************************************************
*   AddrComputeDccInfo
*
*   @brief
*       Compute DCC key size, base alignment based on color surface size, tile info or tile index
*
****************************************************************************************************
*/
ADDR_E_RETURNCODE ADDR_API AddrComputeDccInfo(
    ADDR_HANDLE                             hLib,   ///< handle of addrlib
    const ADDR_COMPUTE_DCCINFO_INPUT*       pIn,    ///< [in] input
    ADDR_COMPUTE_DCCINFO_OUTPUT*            pOut)   ///< [out] output
{
    ADDR_E_RETURNCODE returnCode;

    V1::Lib* pLib = V1::Lib::GetLib(hLib);

    if (pLib != NULL)
    {
        returnCode = pLib->ComputeDccInfo(pIn, pOut);
    }
    else
    {
        returnCode = ADDR_ERROR;
    }

    return returnCode;
}



///////////////////////////////////////////////////////////////////////////////
// Below functions are element related or helper functions
///////////////////////////////////////////////////////////////////////////////

/**
****************************************************************************************************
*   AddrGetVersion
*
*   @brief
*       Get AddrLib version number. Client may check this return value against ADDRLIB_VERSION
*       defined in addrinterface.h to see if there is a mismatch.
****************************************************************************************************
*/
UINT_32 ADDR_API AddrGetVersion(ADDR_HANDLE hLib)
{
    UINT_32 version = 0;

    Addr::Lib* pLib = Lib::GetLib(hLib);

    ADDR_ASSERT(pLib != NULL);

    if (pLib)
    {
        version = pLib->GetVersion();
    }

    return version;
}

/**
****************************************************************************************************
*   AddrUseTileIndex
*
*   @brief
*       Return TRUE if tileIndex is enabled in this address library
****************************************************************************************************
*/
BOOL_32 ADDR_API AddrUseTileIndex(ADDR_HANDLE hLib)
{
    BOOL_32 useTileIndex = FALSE;

    V1::Lib* pLib = V1::Lib::GetLib(hLib);

    ADDR_ASSERT(pLib != NULL);

    if (pLib)
    {
        useTileIndex = pLib->UseTileIndex(0);
    }

    return useTileIndex;
}

/**
****************************************************************************************************
*   AddrUseCombinedSwizzle
*
*   @brief
*       Return TRUE if combined swizzle is enabled in this address library
****************************************************************************************************
*/
BOOL_32 ADDR_API AddrUseCombinedSwizzle(ADDR_HANDLE hLib)
{
    BOOL_32 useCombinedSwizzle = FALSE;

    V1::Lib* pLib = V1::Lib::GetLib(hLib);

    ADDR_ASSERT(pLib != NULL);

    if (pLib)
    {
        useCombinedSwizzle = pLib->UseCombinedSwizzle();
    }

    return useCombinedSwizzle;
}

/**
****************************************************************************************************
*   AddrExtractBankPipeSwizzle
*
*   @brief
*       Extract Bank and Pipe swizzle from base256b
*   @return
*       ADDR_OK if successful, otherwise an error code of ADDR_E_RETURNCODE
****************************************************************************************************
*/
ADDR_E_RETURNCODE ADDR_API AddrExtractBankPipeSwizzle(
    ADDR_HANDLE                                 hLib,     ///< addrlib handle
    const ADDR_EXTRACT_BANKPIPE_SWIZZLE_INPUT*  pIn,      ///< [in] input structure
    ADDR_EXTRACT_BANKPIPE_SWIZZLE_OUTPUT*       pOut)     ///< [out] output structure
{
    ADDR_E_RETURNCODE returnCode = ADDR_OK;

    V1::Lib* pLib = V1::Lib::GetLib(hLib);

    if (pLib != NULL)
    {
        returnCode = pLib->ExtractBankPipeSwizzle(pIn, pOut);
    }
    else
    {
        returnCode = ADDR_ERROR;
    }

    return returnCode;
}

/**
****************************************************************************************************
*   AddrCombineBankPipeSwizzle
*
*   @brief
*       Combine Bank and Pipe swizzle
*   @return
*       ADDR_E_RETURNCODE
****************************************************************************************************
*/
ADDR_E_RETURNCODE ADDR_API AddrCombineBankPipeSwizzle(
    ADDR_HANDLE                                 hLib,
    const ADDR_COMBINE_BANKPIPE_SWIZZLE_INPUT*  pIn,
    ADDR_COMBINE_BANKPIPE_SWIZZLE_OUTPUT*       pOut)
{
    ADDR_E_RETURNCODE returnCode = ADDR_OK;

    V1::Lib* pLib = V1::Lib::GetLib(hLib);

    if (pLib != NULL)
    {
        returnCode = pLib->CombineBankPipeSwizzle(pIn, pOut);
    }
    else
    {
        returnCode = ADDR_ERROR;
    }

    return returnCode;
}

/**
****************************************************************************************************
*   AddrComputeSliceSwizzle
*
*   @brief
*       Compute a swizzle for slice from a base swizzle
*   @return
*       ADDR_OK if no error
****************************************************************************************************
*/
ADDR_E_RETURNCODE ADDR_API AddrComputeSliceSwizzle(
    ADDR_HANDLE                                 hLib,
    const ADDR_COMPUTE_SLICESWIZZLE_INPUT*      pIn,
    ADDR_COMPUTE_SLICESWIZZLE_OUTPUT*           pOut)
{
    ADDR_E_RETURNCODE returnCode = ADDR_OK;

    V1::Lib* pLib = V1::Lib::GetLib(hLib);

    if (pLib != NULL)
    {
        returnCode = pLib->ComputeSliceTileSwizzle(pIn, pOut);
    }
    else
    {
        returnCode = ADDR_ERROR;
    }

    return returnCode;
}

/**
****************************************************************************************************
*   AddrComputeBaseSwizzle
*
*   @brief
*       Return a Combined Bank and Pipe swizzle base on surface based on surface type/index
*   @return
*       ADDR_OK if no error
****************************************************************************************************
*/
ADDR_E_RETURNCODE ADDR_API AddrComputeBaseSwizzle(
    ADDR_HANDLE                             hLib,
    const ADDR_COMPUTE_BASE_SWIZZLE_INPUT*  pIn,
    ADDR_COMPUTE_BASE_SWIZZLE_OUTPUT*       pOut)
{
    ADDR_E_RETURNCODE returnCode = ADDR_OK;

    V1::Lib* pLib = V1::Lib::GetLib(hLib);

    if (pLib != NULL)
    {
        returnCode = pLib->ComputeBaseSwizzle(pIn, pOut);
    }
    else
    {
        returnCode = ADDR_ERROR;
    }

    return returnCode;
}

/**
****************************************************************************************************
*   ElemFlt32ToDepthPixel
*
*   @brief
*       Convert a FLT_32 value to a depth/stencil pixel value
*
*   @return
*       ADDR_OK if successful, otherwise an error code of ADDR_E_RETURNCODE
*
****************************************************************************************************
*/
ADDR_E_RETURNCODE ADDR_API ElemFlt32ToDepthPixel(
    ADDR_HANDLE                         hLib,    ///< addrlib handle
    const ELEM_FLT32TODEPTHPIXEL_INPUT* pIn,     ///< [in] per-component value
    ELEM_FLT32TODEPTHPIXEL_OUTPUT*      pOut)    ///< [out] final pixel value
{
    ADDR_E_RETURNCODE returnCode = ADDR_OK;

    Lib* pLib = Lib::GetLib(hLib);

    if (pLib != NULL)
    {
        pLib->Flt32ToDepthPixel(pIn, pOut);
    }
    else
    {
        returnCode = ADDR_ERROR;
    }

    return returnCode;
}

/**
****************************************************************************************************
*   ElemFlt32ToColorPixel
*
*   @brief
*       Convert a FLT_32 value to a red/green/blue/alpha pixel value
*
*   @return
*       ADDR_OK if successful, otherwise an error code of ADDR_E_RETURNCODE
*
****************************************************************************************************
*/
ADDR_E_RETURNCODE ADDR_API ElemFlt32ToColorPixel(
    ADDR_HANDLE                         hLib,    ///< addrlib handle
    const ELEM_FLT32TOCOLORPIXEL_INPUT* pIn,     ///< [in] format, surface number and swap value
    ELEM_FLT32TOCOLORPIXEL_OUTPUT*      pOut)    ///< [out] final pixel value
{
    ADDR_E_RETURNCODE returnCode = ADDR_OK;

    Lib* pLib = Lib::GetLib(hLib);

    if (pLib != NULL)
    {
        pLib->Flt32ToColorPixel(pIn, pOut);
    }
    else
    {
        returnCode = ADDR_ERROR;
    }

    return returnCode;
}

/**
****************************************************************************************************
*   ElemGetExportNorm
*
*   @brief
*       Helper function to check one format can be EXPORT_NUM,
*       which is a register CB_COLOR_INFO.SURFACE_FORMAT.
*       FP16 can be reported as EXPORT_NORM for rv770 in r600
*       family
*
****************************************************************************************************
*/
BOOL_32 ADDR_API ElemGetExportNorm(
    ADDR_HANDLE                     hLib, ///< addrlib handle
    const ELEM_GETEXPORTNORM_INPUT* pIn)  ///< [in] input structure
{
    Addr::Lib* pLib = Lib::GetLib(hLib);
    BOOL_32 enabled = FALSE;

    ADDR_E_RETURNCODE returnCode = ADDR_OK;

    if (pLib != NULL)
    {
        enabled = pLib->GetExportNorm(pIn);
    }
    else
    {
        returnCode = ADDR_ERROR;
    }

    ADDR_ASSERT(returnCode == ADDR_OK);

    return enabled;
}

/**
****************************************************************************************************
*   ElemSize
*
*   @brief
*       Get bits-per-element for specified format
*
*   @return
*       Bits-per-element of specified format
*
****************************************************************************************************
*/
UINT_32 ADDR_API ElemSize(
    ADDR_HANDLE hLib,
    AddrFormat  format)
{
    UINT_32 bpe = 0;

    Addr::Lib* pLib = Lib::GetLib(hLib);

    if (pLib != NULL)
    {
        bpe = pLib->GetBpe(format);
    }

    return bpe;
}

/**
****************************************************************************************************
*   AddrConvertTileInfoToHW
*
*   @brief
*       Convert tile info from real value to hardware register value
*
*   @return
*       ADDR_OK if successful, otherwise an error code of ADDR_E_RETURNCODE
****************************************************************************************************
*/
ADDR_E_RETURNCODE ADDR_API AddrConvertTileInfoToHW(
    ADDR_HANDLE                             hLib, ///< address lib handle
    const ADDR_CONVERT_TILEINFOTOHW_INPUT*  pIn,  ///< [in] tile info with real value
    ADDR_CONVERT_TILEINFOTOHW_OUTPUT*       pOut) ///< [out] tile info with HW register value
{
    V1::Lib* pLib = V1::Lib::GetLib(hLib);

    ADDR_E_RETURNCODE returnCode = ADDR_OK;

    if (pLib != NULL)
    {
        returnCode = pLib->ConvertTileInfoToHW(pIn, pOut);
    }
    else
    {
        returnCode = ADDR_ERROR;
    }

    return returnCode;
}

/**
****************************************************************************************************
*   AddrConvertTileIndex
*
*   @brief
*       Convert tile index to tile mode/type/info
*
*   @return
*       ADDR_OK if successful, otherwise an error code of ADDR_E_RETURNCODE
****************************************************************************************************
*/
ADDR_E_RETURNCODE ADDR_API AddrConvertTileIndex(
    ADDR_HANDLE                          hLib, ///< address lib handle
    const ADDR_CONVERT_TILEINDEX_INPUT*  pIn,  ///< [in] input - tile index
    ADDR_CONVERT_TILEINDEX_OUTPUT*       pOut) ///< [out] tile mode/type/info
{
    V1::Lib* pLib = V1::Lib::GetLib(hLib);

    ADDR_E_RETURNCODE returnCode = ADDR_OK;

    if (pLib != NULL)
    {
        returnCode = pLib->ConvertTileIndex(pIn, pOut);
    }
    else
    {
        returnCode = ADDR_ERROR;
    }

    return returnCode;
}

/**
****************************************************************************************************
*   AddrGetMacroModeIndex
*
*   @brief
*       Get macro mode index based on input parameters
*
*   @return
*       ADDR_OK if successful, otherwise an error code of ADDR_E_RETURNCODE
****************************************************************************************************
*/
ADDR_E_RETURNCODE ADDR_API AddrGetMacroModeIndex(
    ADDR_HANDLE                          hLib, ///< address lib handle
    const ADDR_GET_MACROMODEINDEX_INPUT* pIn,  ///< [in] input
    ADDR_GET_MACROMODEINDEX_OUTPUT*      pOut) ///< [out] macro mode index
{
    V1::Lib* pLib = V1::Lib::GetLib(hLib);

    ADDR_E_RETURNCODE returnCode;

    if (pLib != NULL)
    {
        returnCode = pLib->GetMacroModeIndex(pIn, pOut);
    }
    else
    {
        returnCode = ADDR_ERROR;
    }

    return returnCode;
}

/**
****************************************************************************************************
*   AddrConvertTileIndex1
*
*   @brief
*       Convert tile index to tile mode/type/info
*
*   @return
*       ADDR_OK if successful, otherwise an error code of ADDR_E_RETURNCODE
****************************************************************************************************
*/
ADDR_E_RETURNCODE ADDR_API AddrConvertTileIndex1(
    ADDR_HANDLE                          hLib, ///< address lib handle
    const ADDR_CONVERT_TILEINDEX1_INPUT* pIn,  ///< [in] input - tile index
    ADDR_CONVERT_TILEINDEX_OUTPUT*       pOut) ///< [out] tile mode/type/info
{
    V1::Lib* pLib = V1::Lib::GetLib(hLib);

    ADDR_E_RETURNCODE returnCode = ADDR_OK;

    if (pLib != NULL)
    {
        returnCode = pLib->ConvertTileIndex1(pIn, pOut);
    }
    else
    {
        returnCode = ADDR_ERROR;
    }

    return returnCode;
}

/**
****************************************************************************************************
*   AddrGetTileIndex
*
*   @brief
*       Get tile index from tile mode/type/info
*
*   @return
*       ADDR_OK if successful, otherwise an error code of ADDR_E_RETURNCODE
*
*   @note
*       Only meaningful for SI (and above)
****************************************************************************************************
*/
ADDR_E_RETURNCODE ADDR_API AddrGetTileIndex(
    ADDR_HANDLE                     hLib,
    const ADDR_GET_TILEINDEX_INPUT* pIn,
    ADDR_GET_TILEINDEX_OUTPUT*      pOut)
{
    V1::Lib* pLib = V1::Lib::GetLib(hLib);

    ADDR_E_RETURNCODE returnCode = ADDR_OK;

    if (pLib != NULL)
    {
        returnCode = pLib->GetTileIndex(pIn, pOut);
    }
    else
    {
        returnCode = ADDR_ERROR;
    }

    return returnCode;
}

/**
****************************************************************************************************
*   AddrComputePrtInfo
*
*   @brief
*       Interface function for ComputePrtInfo
*
****************************************************************************************************
*/
ADDR_E_RETURNCODE ADDR_API AddrComputePrtInfo(
    ADDR_HANDLE                 hLib,
    const ADDR_PRT_INFO_INPUT*  pIn,
    ADDR_PRT_INFO_OUTPUT*       pOut)
{
    ADDR_E_RETURNCODE returnCode = ADDR_OK;

    V1::Lib* pLib = V1::Lib::GetLib(hLib);

    if (pLib != NULL)
    {
        returnCode = pLib->ComputePrtInfo(pIn, pOut);
    }
    else
    {
        returnCode = ADDR_ERROR;
    }

    return returnCode;
}

/**
****************************************************************************************************
*   AddrGetMaxAlignments
*
*   @brief
*       Convert maximum alignments
*
*   @return
*       ADDR_OK if successful, otherwise an error code of ADDR_E_RETURNCODE
****************************************************************************************************
*/
ADDR_E_RETURNCODE ADDR_API AddrGetMaxAlignments(
    ADDR_HANDLE                     hLib, ///< address lib handle
    ADDR_GET_MAX_ALIGNMENTS_OUTPUT* pOut) ///< [out] output structure
{
    Addr::Lib* pLib = Lib::GetLib(hLib);

    ADDR_E_RETURNCODE returnCode = ADDR_OK;

    if (pLib != NULL)
    {
        returnCode = pLib->GetMaxAlignments(pOut);
    }
    else
    {
        returnCode = ADDR_ERROR;
    }

    return returnCode;
}

/**
****************************************************************************************************
*   AddrGetMaxMetaAlignments
*
*   @brief
*       Convert maximum alignments for metadata
*
*   @return
*       ADDR_OK if successful, otherwise an error code of ADDR_E_RETURNCODE
****************************************************************************************************
*/
ADDR_E_RETURNCODE ADDR_API AddrGetMaxMetaAlignments(
    ADDR_HANDLE                     hLib, ///< address lib handle
    ADDR_GET_MAX_ALIGNMENTS_OUTPUT* pOut) ///< [out] output structure
{
    Addr::Lib* pLib = Lib::GetLib(hLib);

    ADDR_E_RETURNCODE returnCode = ADDR_OK;

    if (pLib != NULL)
    {
        returnCode = pLib->GetMaxMetaAlignments(pOut);
    }
    else
    {
        returnCode = ADDR_ERROR;
    }

    return returnCode;
}


////////////////////////////////////////////////////////////////////////////////////////////////////
//                                    Surface functions for Addr2
////////////////////////////////////////////////////////////////////////////////////////////////////

/**
****************************************************************************************************
*   Addr2ComputeSurfaceInfo
*
*   @brief
*       Calculate surface width/height/depth/alignments and suitable tiling mode
*
*   @return
*       ADDR_OK if successful, otherwise an error code of ADDR_E_RETURNCODE
****************************************************************************************************
*/
ADDR_E_RETURNCODE ADDR_API Addr2ComputeSurfaceInfo(
    ADDR_HANDLE                                hLib, ///< address lib handle
    const ADDR2_COMPUTE_SURFACE_INFO_INPUT*    pIn,  ///< [in] surface information
    ADDR2_COMPUTE_SURFACE_INFO_OUTPUT*         pOut) ///< [out] surface parameters and alignments
{
    V2::Lib* pLib = V2::Lib::GetLib(hLib);

    ADDR_E_RETURNCODE returnCode = ADDR_OK;

    if (pLib != NULL)
    {
        returnCode = pLib->ComputeSurfaceInfo(pIn, pOut);
    }
    else
    {
        returnCode = ADDR_ERROR;
    }

    return returnCode;
}


/**
****************************************************************************************************
*   Addr2ComputeSurfaceAddrFromCoord
*
*   @brief
*       Compute surface address according to coordinates
*
*   @return
*       ADDR_OK if successful, otherwise an error code of ADDR_E_RETURNCODE
****************************************************************************************************
*/
ADDR_E_RETURNCODE ADDR_API Addr2ComputeSurfaceAddrFromCoord(
    ADDR_HANDLE                                         hLib, ///< address lib handle
    const ADDR2_COMPUTE_SURFACE_ADDRFROMCOORD_INPUT*    pIn,  ///< [in] surface info and coordinates
    ADDR2_COMPUTE_SURFACE_ADDRFROMCOORD_OUTPUT*         pOut) ///< [out] surface address
{
    V2::Lib* pLib = V2::Lib::GetLib(hLib);

    ADDR_E_RETURNCODE returnCode = ADDR_OK;

    if (pLib != NULL)
    {
        returnCode = pLib->ComputeSurfaceAddrFromCoord(pIn, pOut);
    }
    else
    {
        returnCode = ADDR_ERROR;
    }

    return returnCode;
}


/**
****************************************************************************************************
*   Addr2ComputeSurfaceCoordFromAddr
*
*   @brief
*       Compute coordinates according to surface address
*
*   @return
*       ADDR_OK if successful, otherwise an error code of ADDR_E_RETURNCODE
****************************************************************************************************
*/
ADDR_E_RETURNCODE ADDR_API Addr2ComputeSurfaceCoordFromAddr(
    ADDR_HANDLE                                         hLib, ///< address lib handle
    const ADDR2_COMPUTE_SURFACE_COORDFROMADDR_INPUT*    pIn,  ///< [in] surface info and address
    ADDR2_COMPUTE_SURFACE_COORDFROMADDR_OUTPUT*         pOut) ///< [out] coordinates
{
    V2::Lib* pLib = V2::Lib::GetLib(hLib);

    ADDR_E_RETURNCODE returnCode = ADDR_OK;

    if (pLib != NULL)
    {
        returnCode = pLib->ComputeSurfaceCoordFromAddr(pIn, pOut);
    }
    else
    {
        returnCode = ADDR_ERROR;
    }

    return returnCode;
}



////////////////////////////////////////////////////////////////////////////////////////////////////
//                                   HTile functions for Addr2
////////////////////////////////////////////////////////////////////////////////////////////////////

/**
****************************************************************************************************
*   Addr2ComputeHtileInfo
*
*   @brief
*       Compute Htile pitch, height, base alignment and size in bytes
*
*   @return
*       ADDR_OK if successful, otherwise an error code of ADDR_E_RETURNCODE
****************************************************************************************************
*/
ADDR_E_RETURNCODE ADDR_API Addr2ComputeHtileInfo(
    ADDR_HANDLE                              hLib, ///< address lib handle
    const ADDR2_COMPUTE_HTILE_INFO_INPUT*    pIn,  ///< [in] Htile information
    ADDR2_COMPUTE_HTILE_INFO_OUTPUT*         pOut) ///< [out] Htile pitch, height and size in bytes
{
    V2::Lib* pLib = V2::Lib::GetLib(hLib);

    ADDR_E_RETURNCODE returnCode = ADDR_OK;

    if (pLib != NULL)
    {
        returnCode = pLib->ComputeHtileInfo(pIn, pOut);
    }
    else
    {
        returnCode = ADDR_ERROR;
    }

    return returnCode;
}


/**
****************************************************************************************************
*   Addr2ComputeHtileAddrFromCoord
*
*   @brief
*       Compute Htile address according to coordinates (of depth buffer)
*
*   @return
*       ADDR_OK if successful, otherwise an error code of ADDR_E_RETURNCODE
****************************************************************************************************
*/
ADDR_E_RETURNCODE ADDR_API Addr2ComputeHtileAddrFromCoord(
    ADDR_HANDLE                                       hLib, ///< address lib handle
    const ADDR2_COMPUTE_HTILE_ADDRFROMCOORD_INPUT*    pIn,  ///< [in] Htile info and coordinates
    ADDR2_COMPUTE_HTILE_ADDRFROMCOORD_OUTPUT*         pOut) ///< [out] Htile address
{
    V2::Lib* pLib = V2::Lib::GetLib(hLib);

    ADDR_E_RETURNCODE returnCode = ADDR_OK;

    if (pLib != NULL)
    {
        returnCode = pLib->ComputeHtileAddrFromCoord(pIn, pOut);
    }
    else
    {
        returnCode = ADDR_ERROR;
    }

    return returnCode;
}


/**
****************************************************************************************************
*   Addr2ComputeHtileCoordFromAddr
*
*   @brief
*       Compute coordinates within depth buffer (1st pixel of a micro tile) according to
*       Htile address
*
*   @return
*       ADDR_OK if successful, otherwise an error code of ADDR_E_RETURNCODE
****************************************************************************************************
*/
ADDR_E_RETURNCODE ADDR_API Addr2ComputeHtileCoordFromAddr(
    ADDR_HANDLE                                       hLib, ///< address lib handle
    const ADDR2_COMPUTE_HTILE_COORDFROMADDR_INPUT*    pIn,  ///< [in] Htile info and address
    ADDR2_COMPUTE_HTILE_COORDFROMADDR_OUTPUT*         pOut) ///< [out] Htile coordinates
{
    V2::Lib* pLib = V2::Lib::GetLib(hLib);

    ADDR_E_RETURNCODE returnCode = ADDR_OK;

    if (pLib != NULL)
    {
        returnCode = pLib->ComputeHtileCoordFromAddr(pIn, pOut);
    }
    else
    {
        returnCode = ADDR_ERROR;
    }

    return returnCode;
}



////////////////////////////////////////////////////////////////////////////////////////////////////
//                                     C-mask functions for Addr2
////////////////////////////////////////////////////////////////////////////////////////////////////

/**
****************************************************************************************************
*   Addr2ComputeCmaskInfo
*
*   @brief
*       Compute Cmask pitch, height, base alignment and size in bytes from color buffer
*       info
*
*   @return
*       ADDR_OK if successful, otherwise an error code of ADDR_E_RETURNCODE
****************************************************************************************************
*/
ADDR_E_RETURNCODE ADDR_API Addr2ComputeCmaskInfo(
    ADDR_HANDLE                              hLib, ///< address lib handle
    const ADDR2_COMPUTE_CMASK_INFO_INPUT*    pIn,  ///< [in] Cmask pitch and height
    ADDR2_COMPUTE_CMASK_INFO_OUTPUT*         pOut) ///< [out] Cmask pitch, height and size in bytes
{
    V2::Lib* pLib = V2::Lib::GetLib(hLib);

    ADDR_E_RETURNCODE returnCode = ADDR_OK;

    if (pLib != NULL)
    {
        returnCode = pLib->ComputeCmaskInfo(pIn, pOut);
    }
    else
    {
        returnCode = ADDR_ERROR;
    }

    return returnCode;
}


/**
****************************************************************************************************
*   Addr2ComputeCmaskAddrFromCoord
*
*   @brief
*       Compute Cmask address according to coordinates (of MSAA color buffer)
*
*   @return
*       ADDR_OK if successful, otherwise an error code of ADDR_E_RETURNCODE
****************************************************************************************************
*/
ADDR_E_RETURNCODE ADDR_API Addr2ComputeCmaskAddrFromCoord(
    ADDR_HANDLE                                       hLib, ///< address lib handle
    const ADDR2_COMPUTE_CMASK_ADDRFROMCOORD_INPUT*    pIn,  ///< [in] Cmask info and coordinates
    ADDR2_COMPUTE_CMASK_ADDRFROMCOORD_OUTPUT*         pOut) ///< [out] Cmask address
{
    V2::Lib* pLib = V2::Lib::GetLib(hLib);

    ADDR_E_RETURNCODE returnCode = ADDR_OK;

    if (pLib != NULL)
    {
        returnCode = pLib->ComputeCmaskAddrFromCoord(pIn, pOut);
    }
    else
    {
        returnCode = ADDR_ERROR;
    }

    return returnCode;
}


/**
****************************************************************************************************
*   Addr2ComputeCmaskCoordFromAddr
*
*   @brief
*       Compute coordinates within color buffer (1st pixel of a micro tile) according to
*       Cmask address
*
*   @return
*       ADDR_OK if successful, otherwise an error code of ADDR_E_RETURNCODE
****************************************************************************************************
*/
ADDR_E_RETURNCODE ADDR_API Addr2ComputeCmaskCoordFromAddr(
    ADDR_HANDLE                                       hLib, ///< address lib handle
    const ADDR2_COMPUTE_CMASK_COORDFROMADDR_INPUT*    pIn,  ///< [in] Cmask info and address
    ADDR2_COMPUTE_CMASK_COORDFROMADDR_OUTPUT*         pOut) ///< [out] Cmask coordinates
{
    V2::Lib* pLib = V2::Lib::GetLib(hLib);

    ADDR_E_RETURNCODE returnCode = ADDR_OK;

    if (pLib != NULL)
    {
        returnCode = pLib->ComputeCmaskCoordFromAddr(pIn, pOut);
    }
    else
    {
        returnCode = ADDR_ERROR;
    }

    return returnCode;
}



////////////////////////////////////////////////////////////////////////////////////////////////////
//                                     F-mask functions for Addr2
////////////////////////////////////////////////////////////////////////////////////////////////////

/**
****************************************************************************************************
*   Addr2ComputeFmaskInfo
*
*   @brief
*       Compute Fmask pitch/height/depth/alignments and size in bytes
*
*   @return
*       ADDR_OK if successful, otherwise an error code of ADDR_E_RETURNCODE
****************************************************************************************************
*/
ADDR_E_RETURNCODE ADDR_API Addr2ComputeFmaskInfo(
    ADDR_HANDLE                              hLib, ///< address lib handle
    const ADDR2_COMPUTE_FMASK_INFO_INPUT*    pIn,  ///< [in] Fmask information
    ADDR2_COMPUTE_FMASK_INFO_OUTPUT*         pOut) ///< [out] Fmask pitch and height
{
    V2::Lib* pLib = V2::Lib::GetLib(hLib);

    ADDR_E_RETURNCODE returnCode = ADDR_OK;

    if (pLib != NULL)
    {
        returnCode = pLib->ComputeFmaskInfo(pIn, pOut);
    }
    else
    {
        returnCode = ADDR_ERROR;
    }

    return returnCode;
}


/**
****************************************************************************************************
*   Addr2ComputeFmaskAddrFromCoord
*
*   @brief
*       Compute Fmask address according to coordinates (x,y,slice,sample,plane)
*
*   @return
*       ADDR_OK if successful, otherwise an error code of ADDR_E_RETURNCODE
****************************************************************************************************
*/
ADDR_E_RETURNCODE ADDR_API Addr2ComputeFmaskAddrFromCoord(
    ADDR_HANDLE                                       hLib, ///< address lib handle
    const ADDR2_COMPUTE_FMASK_ADDRFROMCOORD_INPUT*    pIn,  ///< [in] Fmask info and coordinates
    ADDR2_COMPUTE_FMASK_ADDRFROMCOORD_OUTPUT*         pOut) ///< [out] Fmask address
{
    V2::Lib* pLib = V2::Lib::GetLib(hLib);

    ADDR_E_RETURNCODE returnCode = ADDR_OK;

    if (pLib != NULL)
    {
        returnCode = pLib->ComputeFmaskAddrFromCoord(pIn, pOut);
    }
    else
    {
        returnCode = ADDR_ERROR;
    }

    return returnCode;
}


/**
****************************************************************************************************
*   Addr2ComputeFmaskCoordFromAddr
*
*   @brief
*       Compute coordinates (x,y,slice,sample,plane) according to Fmask address
*
*   @return
*       ADDR_OK if successful, otherwise an error code of ADDR_E_RETURNCODE
****************************************************************************************************
*/
ADDR_E_RETURNCODE ADDR_API Addr2ComputeFmaskCoordFromAddr(
    ADDR_HANDLE                                       hLib, ///< address lib handle
    const ADDR2_COMPUTE_FMASK_COORDFROMADDR_INPUT*    pIn,  ///< [in] Fmask info and address
    ADDR2_COMPUTE_FMASK_COORDFROMADDR_OUTPUT*         pOut) ///< [out] Fmask coordinates
{
    V2::Lib* pLib = V2::Lib::GetLib(hLib);

    ADDR_E_RETURNCODE returnCode = ADDR_OK;

    if (pLib != NULL)
    {
        returnCode = pLib->ComputeFmaskCoordFromAddr(pIn, pOut);
    }
    else
    {
        returnCode = ADDR_ERROR;
    }

    return returnCode;
}



////////////////////////////////////////////////////////////////////////////////////////////////////
//                                     DCC key functions for Addr2
////////////////////////////////////////////////////////////////////////////////////////////////////

/**
****************************************************************************************************
*   Addr2ComputeDccInfo
*
*   @brief
*       Compute DCC key size, base alignment based on color surface size, tile info or tile index
*
****************************************************************************************************
*/
ADDR_E_RETURNCODE ADDR_API Addr2ComputeDccInfo(
    ADDR_HANDLE                           hLib,   ///< handle of addrlib
    const ADDR2_COMPUTE_DCCINFO_INPUT*    pIn,    ///< [in] input
    ADDR2_COMPUTE_DCCINFO_OUTPUT*         pOut)   ///< [out] output
{
    ADDR_E_RETURNCODE returnCode;

    V2::Lib* pLib = V2::Lib::GetLib(hLib);

    if (pLib != NULL)
    {
        returnCode = pLib->ComputeDccInfo(pIn, pOut);
    }
    else
    {
        returnCode = ADDR_ERROR;
    }

    return returnCode;
}

/**
****************************************************************************************************
*   Addr2ComputeDccAddrFromCoord
*
*   @brief
*       Compute DCC key address according to coordinates
*
*   @return
*       ADDR_OK if successful, otherwise an error code of ADDR_E_RETURNCODE
****************************************************************************************************
*/
ADDR_E_RETURNCODE ADDR_API Addr2ComputeDccAddrFromCoord(
    ADDR_HANDLE                                     hLib, ///< address lib handle
    const ADDR2_COMPUTE_DCC_ADDRFROMCOORD_INPUT*    pIn,  ///< [in] Dcc info and coordinates
    ADDR2_COMPUTE_DCC_ADDRFROMCOORD_OUTPUT*         pOut) ///< [out] Dcc address
{
    V2::Lib* pLib = V2::Lib::GetLib(hLib);

    ADDR_E_RETURNCODE returnCode = ADDR_OK;

    if (pLib != NULL)
    {
        returnCode = pLib->ComputeDccAddrFromCoord(pIn, pOut);
    }
    else
    {
        returnCode = ADDR_ERROR;
    }

    return returnCode;
}

/**
****************************************************************************************************
*   Addr2ComputePipeBankXor
*
*   @brief
*       Calculate a valid bank pipe xor value for client to use.
****************************************************************************************************
*/
ADDR_E_RETURNCODE ADDR_API Addr2ComputePipeBankXor(
    ADDR_HANDLE                            hLib, ///< handle of addrlib
    const ADDR2_COMPUTE_PIPEBANKXOR_INPUT* pIn,  ///< [in] input
    ADDR2_COMPUTE_PIPEBANKXOR_OUTPUT*      pOut) ///< [out] output
{
    ADDR_E_RETURNCODE returnCode;

    V2::Lib* pLib = V2::Lib::GetLib(hLib);

    if (pLib != NULL)
    {
        returnCode = pLib->ComputePipeBankXor(pIn, pOut);
    }
    else
    {
        returnCode = ADDR_ERROR;
    }

    return returnCode;
}

/**
****************************************************************************************************
*   Addr2ComputeSlicePipeBankXor
*
*   @brief
*       Calculate slice pipe bank xor value based on base pipe bank xor and slice id.
****************************************************************************************************
*/
ADDR_E_RETURNCODE ADDR_API Addr2ComputeSlicePipeBankXor(
    ADDR_HANDLE                                  hLib, ///< handle of addrlib
    const ADDR2_COMPUTE_SLICE_PIPEBANKXOR_INPUT* pIn,  ///< [in] input
    ADDR2_COMPUTE_SLICE_PIPEBANKXOR_OUTPUT*      pOut) ///< [out] output
{
    ADDR_E_RETURNCODE returnCode;

    V2::Lib* pLib = V2::Lib::GetLib(hLib);

    if (pLib != NULL)
    {
        returnCode = pLib->ComputeSlicePipeBankXor(pIn, pOut);
    }
    else
    {
        returnCode = ADDR_ERROR;
    }

    return returnCode;
}

/**
****************************************************************************************************
*   Addr2ComputeSubResourceOffsetForSwizzlePattern
*
*   @brief
*       Calculate sub resource offset for swizzle pattern.
****************************************************************************************************
*/
ADDR_E_RETURNCODE ADDR_API Addr2ComputeSubResourceOffsetForSwizzlePattern(
    ADDR_HANDLE                                                     hLib, ///< handle of addrlib
    const ADDR2_COMPUTE_SUBRESOURCE_OFFSET_FORSWIZZLEPATTERN_INPUT* pIn,  ///< [in] input
    ADDR2_COMPUTE_SUBRESOURCE_OFFSET_FORSWIZZLEPATTERN_OUTPUT*      pOut) ///< [out] output
{
    ADDR_E_RETURNCODE returnCode;

    V2::Lib* pLib = V2::Lib::GetLib(hLib);

    if (pLib != NULL)
    {
        returnCode = pLib->ComputeSubResourceOffsetForSwizzlePattern(pIn, pOut);
    }
    else
    {
        returnCode = ADDR_ERROR;
    }

    return returnCode;
}

/**
****************************************************************************************************
*   Addr2ComputeNonBlockCompressedView
*
*   @brief
*       Compute non-block-compressed view for a given mipmap level/slice.
****************************************************************************************************
*/
ADDR_E_RETURNCODE ADDR_API Addr2ComputeNonBlockCompressedView(
    ADDR_HANDLE                                       hLib, ///< handle of addrlib
    const ADDR2_COMPUTE_NONBLOCKCOMPRESSEDVIEW_INPUT* pIn,  ///< [in] input
    ADDR2_COMPUTE_NONBLOCKCOMPRESSEDVIEW_OUTPUT*      pOut) ///< [out] output
{
    ADDR_E_RETURNCODE returnCode;

    V2::Lib* pLib = V2::Lib::GetLib(hLib);

    if (pLib != NULL)
    {
        returnCode = pLib->ComputeNonBlockCompressedView(pIn, pOut);
    }
    else
    {
        returnCode = ADDR_ERROR;
    }

    return returnCode;
}

/**
****************************************************************************************************
*   Addr2GetPreferredSurfaceSetting
*
*   @brief
*       Suggest a preferred setting for client driver to program HW register
****************************************************************************************************
*/
ADDR_E_RETURNCODE ADDR_API Addr2GetPreferredSurfaceSetting(
    ADDR_HANDLE                                   hLib, ///< handle of addrlib
    const ADDR2_GET_PREFERRED_SURF_SETTING_INPUT* pIn,  ///< [in] input
    ADDR2_GET_PREFERRED_SURF_SETTING_OUTPUT*      pOut) ///< [out] output
{
    ADDR_E_RETURNCODE returnCode;

    V2::Lib* pLib = V2::Lib::GetLib(hLib);

    if (pLib != NULL)
    {
        returnCode = pLib->Addr2GetPreferredSurfaceSetting(pIn, pOut);
    }
    else
    {
        returnCode = ADDR_ERROR;
    }

    return returnCode;
}

/**
****************************************************************************************************
*   Addr2IsValidDisplaySwizzleMode
*
*   @brief
*       Return whether the swizzle mode is supported by display engine
****************************************************************************************************
*/
ADDR_E_RETURNCODE ADDR_API Addr2IsValidDisplaySwizzleMode(
    ADDR_HANDLE     hLib,
    AddrSwizzleMode swizzleMode,
    UINT_32         bpp,
    BOOL_32         *pResult)
{
    ADDR_E_RETURNCODE returnCode;

    V2::Lib* pLib = V2::Lib::GetLib(hLib);

    if (pLib != NULL)
    {
        ADDR2_COMPUTE_SURFACE_INFO_INPUT in = {};
        in.resourceType = ADDR_RSRC_TEX_2D;
        in.swizzleMode  = swizzleMode;
        in.bpp          = bpp;

        *pResult   = pLib->IsValidDisplaySwizzleMode(&in);
        returnCode = ADDR_OK;
    }
    else
    {
        returnCode = ADDR_ERROR;
    }

    return returnCode;
}

/**
****************************************************************************************************
*   Addr2GetPossibleSwizzleModes
*
*   @brief
*       Returns a list of swizzle modes that are valid from the hardware's perspective for the
*       client to choose from
****************************************************************************************************
*/
ADDR_E_RETURNCODE ADDR_API Addr2GetPossibleSwizzleModes(
    ADDR_HANDLE                                   hLib, ///< handle of addrlib
    const ADDR2_GET_PREFERRED_SURF_SETTING_INPUT* pIn,  ///< [in] input
    ADDR2_GET_PREFERRED_SURF_SETTING_OUTPUT*      pOut) ///< [out] output
{
    ADDR_E_RETURNCODE returnCode;

    V2::Lib* pLib = V2::Lib::GetLib(hLib);

    if (pLib != NULL)
    {
        returnCode = pLib->GetPossibleSwizzleModes(pIn, pOut);
    }
    else
    {
        returnCode = ADDR_ERROR;
    }

    return returnCode;
}
/**
****************************************************************************************************
*   Addr2GetAllowedBlockSet
*
*   @brief
*       Returns the set of allowed block sizes given the allowed swizzle modes and resource type
****************************************************************************************************
*/
ADDR_E_RETURNCODE ADDR_API Addr2GetAllowedBlockSet(
    ADDR_HANDLE      hLib,              ///< handle of addrlib
    ADDR2_SWMODE_SET allowedSwModeSet,  ///< [in] allowed swizzle modes
    AddrResourceType rsrcType,          ///< [in] resource type
    ADDR2_BLOCK_SET* pAllowedBlockSet)  ///< [out] allowed block sizes
{
    ADDR_E_RETURNCODE returnCode;

    V2::Lib* pLib = V2::Lib::GetLib(hLib);

    if (pLib != NULL)
    {
        returnCode = pLib->GetAllowedBlockSet(allowedSwModeSet, rsrcType, pAllowedBlockSet);
    }
    else
    {
        returnCode = ADDR_ERROR;
    }

    return returnCode;
}

/**
****************************************************************************************************
*   Addr2GetAllowedSwSet
*
*   @brief
*       Returns the set of allowed swizzle types given the allowed swizzle modes
****************************************************************************************************
*/
ADDR_E_RETURNCODE ADDR_API Addr2GetAllowedSwSet(
    ADDR_HANDLE       hLib,              ///< handle of addrlib
    ADDR2_SWMODE_SET  allowedSwModeSet,  ///< [in] allowed swizzle modes
    ADDR2_SWTYPE_SET* pAllowedSwSet)     ///< [out] allowed swizzle types
{
    ADDR_E_RETURNCODE returnCode;

    V2::Lib* pLib = V2::Lib::GetLib(hLib);

    if (pLib != NULL)
    {
        returnCode = pLib->GetAllowedSwSet(allowedSwModeSet, pAllowedSwSet);
    }
    else
    {
        returnCode = ADDR_ERROR;
    }

    return returnCode;
}

/**
****************************************************************************************************
*   Addr2IsBlockTypeAvailable
*
*   @brief
*       Determine whether a block type is allowed in a given blockSet
****************************************************************************************************
*/
BOOL_32 Addr2IsBlockTypeAvailable(
    ADDR2_BLOCK_SET blockSet,
    AddrBlockType   blockType)
{
    BOOL_32 avail;

    if (blockType == AddrBlockLinear)
    {
        avail = blockSet.linear ? TRUE : FALSE;
    }
    else
    {
        avail = blockSet.value & (1 << (static_cast<UINT_32>(blockType) - 1)) ? TRUE : FALSE;
    }

    return avail;
}

/**
****************************************************************************************************
*   Addr2BlockTypeWithinMemoryBudget
*
*   @brief
*       Determine whether a new block type is acceptable based on memory waste ratio. Will favor
*       larger block types.
****************************************************************************************************
*/
BOOL_32 Addr2BlockTypeWithinMemoryBudget(
    UINT_64 minSize,
    UINT_64 newBlockTypeSize,
    UINT_32 ratioLow,
    UINT_32 ratioHi,
    DOUBLE  memoryBudget,
    BOOL_32 newBlockTypeBigger)
{
    BOOL_32 accept = FALSE;

    if (memoryBudget >= 1.0)
    {
        if (newBlockTypeBigger)
        {
            if ((static_cast<DOUBLE>(newBlockTypeSize) / minSize) <= memoryBudget)
            {
                accept = TRUE;
            }
        }
        else
        {
            if ((static_cast<DOUBLE>(minSize) / newBlockTypeSize) > memoryBudget)
            {
                accept = TRUE;
            }
        }
    }
    else
    {
        if (newBlockTypeBigger)
        {
            if ((newBlockTypeSize * ratioHi) <= (minSize * ratioLow))
            {
                accept = TRUE;
            }
        }
        else
        {
            if ((newBlockTypeSize * ratioLow) < (minSize * ratioHi))
            {
                accept = TRUE;
            }
        }
    }

    return accept;
}
