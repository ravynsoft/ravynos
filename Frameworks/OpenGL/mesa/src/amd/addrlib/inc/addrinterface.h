/*
************************************************************************************************************************
*
*  Copyright (C) 2007-2022 Advanced Micro Devices, Inc.  All rights reserved.
*  SPDX-License-Identifier: MIT
*
***********************************************************************************************************************/

/**
****************************************************************************************************
* @file  addrinterface.h
* @brief Contains the addrlib interfaces declaration and parameter defines
****************************************************************************************************
*/
#ifndef __ADDR_INTERFACE_H__
#define __ADDR_INTERFACE_H__

// Includes should be before extern "C"
#include "addrtypes.h"

#if defined(__cplusplus)
extern "C"
{
#endif

#define ADDRLIB_VERSION_MAJOR 8
#define ADDRLIB_VERSION_MINOR 9
#define ADDRLIB_VERSION ((ADDRLIB_VERSION_MAJOR << 16) | ADDRLIB_VERSION_MINOR)

/// Virtually all interface functions need ADDR_HANDLE as first parameter
typedef VOID*   ADDR_HANDLE;

/// Client handle used in callbacks
typedef VOID*   ADDR_CLIENT_HANDLE;

/**
* /////////////////////////////////////////////////////////////////////////////////////////////////
* //                                  Callback functions
* /////////////////////////////////////////////////////////////////////////////////////////////////
*    typedef VOID* (ADDR_API* ADDR_ALLOCSYSMEM)(
*         const ADDR_ALLOCSYSMEM_INPUT* pInput);
*    typedef ADDR_E_RETURNCODE (ADDR_API* ADDR_FREESYSMEM)(
*         VOID* pVirtAddr);
*    typedef ADDR_E_RETURNCODE (ADDR_API* ADDR_DEBUGPRINT)(
*         const ADDR_DEBUGPRINT_INPUT* pInput);
*
* /////////////////////////////////////////////////////////////////////////////////////////////////
* //                               Create/Destroy/Config functions
* /////////////////////////////////////////////////////////////////////////////////////////////////
*     AddrCreate()
*     AddrDestroy()
*
* /////////////////////////////////////////////////////////////////////////////////////////////////
* //                                  Surface functions
* /////////////////////////////////////////////////////////////////////////////////////////////////
*     AddrComputeSurfaceInfo()
*     AddrComputeSurfaceAddrFromCoord()
*     AddrComputeSurfaceCoordFromAddr()
*
* /////////////////////////////////////////////////////////////////////////////////////////////////
* //                                   HTile functions
* /////////////////////////////////////////////////////////////////////////////////////////////////
*     AddrComputeHtileInfo()
*     AddrComputeHtileAddrFromCoord()
*     AddrComputeHtileCoordFromAddr()
*
* /////////////////////////////////////////////////////////////////////////////////////////////////
* //                                   C-mask functions
* /////////////////////////////////////////////////////////////////////////////////////////////////
*     AddrComputeCmaskInfo()
*     AddrComputeCmaskAddrFromCoord()
*     AddrComputeCmaskCoordFromAddr()
*
* /////////////////////////////////////////////////////////////////////////////////////////////////
* //                                   F-mask functions
* /////////////////////////////////////////////////////////////////////////////////////////////////
*     AddrComputeFmaskInfo()
*     AddrComputeFmaskAddrFromCoord()
*     AddrComputeFmaskCoordFromAddr()
*
* /////////////////////////////////////////////////////////////////////////////////////////////////
* //                               Element/Utility functions
* /////////////////////////////////////////////////////////////////////////////////////////////////
*     ElemFlt32ToDepthPixel()
*     ElemFlt32ToColorPixel()
*     AddrExtractBankPipeSwizzle()
*     AddrCombineBankPipeSwizzle()
*     AddrComputeSliceSwizzle()
*     AddrConvertTileInfoToHW()
*     AddrConvertTileIndex()
*     AddrConvertTileIndex1()
*     AddrGetTileIndex()
*     AddrComputeBaseSwizzle()
*     AddrUseTileIndex()
*     AddrUseCombinedSwizzle()
*
**/

////////////////////////////////////////////////////////////////////////////////////////////////////
//                                      Callback functions
////////////////////////////////////////////////////////////////////////////////////////////////////

/**
****************************************************************************************************
* @brief channel setting structure
****************************************************************************************************
*/
typedef union _ADDR_CHANNEL_SETTING
{
    struct
    {
        UINT_8 valid   : 1;    ///< Indicate whehter this channel setting is valid
        UINT_8 channel : 2;    ///< 0 for x channel, 1 for y channel, 2 for z channel, 3 for MSAA sample index
        UINT_8 index   : 5;    ///< Channel index
    };
    UINT_8 value;              ///< Value
} ADDR_CHANNEL_SETTING;

/**
****************************************************************************************************
* @brief address equation key structure
****************************************************************************************************
*/
typedef union _ADDR_EQUATION_KEY
{
    struct
    {
        UINT_32 log2ElementBytes : 3; ///< Log2 of Bytes per pixel
        UINT_32 tileMode         : 5; ///< Tile mode
        UINT_32 microTileType    : 3; ///< Micro tile type
        UINT_32 pipeConfig       : 5; ///< pipe config
        UINT_32 numBanksLog2     : 3; ///< Number of banks log2
        UINT_32 bankWidth        : 4; ///< Bank width
        UINT_32 bankHeight       : 4; ///< Bank height
        UINT_32 macroAspectRatio : 3; ///< Macro tile aspect ratio
        UINT_32 prt              : 1; ///< SI only, indicate whether this equation is for prt
        UINT_32 reserved         : 1; ///< Reserved bit
    } fields;
    UINT_32 value;
} ADDR_EQUATION_KEY;

/**
****************************************************************************************************
* @brief address equation structure
****************************************************************************************************
*/
#define ADDR_MAX_LEGACY_EQUATION_COMP 3u
#define ADDR_MAX_EQUATION_COMP        5u
#define ADDR_MAX_EQUATION_BIT         20u

// Invalid equation index
#define ADDR_INVALID_EQUATION_INDEX 0xFFFFFFFF

typedef struct _ADDR_EQUATION
{
    union
    {
        struct {
            ADDR_CHANNEL_SETTING addr[ADDR_MAX_EQUATION_BIT];  ///< addr setting
            ADDR_CHANNEL_SETTING xor1[ADDR_MAX_EQUATION_BIT];  ///< xor setting
            ADDR_CHANNEL_SETTING xor2[ADDR_MAX_EQUATION_BIT];  ///< xor2 setting
            ADDR_CHANNEL_SETTING xor3[ADDR_MAX_EQUATION_BIT];  ///< xor3 setting
            ADDR_CHANNEL_SETTING xor4[ADDR_MAX_EQUATION_BIT];  ///< xor4 setting
        };
        ///< Components showing the sources of each bit; each bit is result of addr ^ xor ^ xor2...
        ADDR_CHANNEL_SETTING comps[ADDR_MAX_EQUATION_COMP][ADDR_MAX_EQUATION_BIT];
    };
    UINT_32              numBits;                      ///< The number of bits in equation
    UINT_32              numBitComponents;             ///< The max number of channels contributing to a bit
    BOOL_32              stackedDepthSlices;           ///< TRUE if depth slices are treated as being
                                                       ///< stacked vertically prior to swizzling
} ADDR_EQUATION;


/**
****************************************************************************************************
* @brief Alloc system memory flags.
* @note These flags are reserved for future use and if flags are added will minimize the impact
*       of the client.
****************************************************************************************************
*/
typedef union _ADDR_ALLOCSYSMEM_FLAGS
{
    struct
    {
        UINT_32 reserved    : 32;  ///< Reserved for future use.
    } fields;
    UINT_32 value;

} ADDR_ALLOCSYSMEM_FLAGS;

/**
****************************************************************************************************
* @brief Alloc system memory input structure
****************************************************************************************************
*/
typedef struct _ADDR_ALLOCSYSMEM_INPUT
{
    UINT_32                 size;           ///< Size of this structure in bytes

    ADDR_ALLOCSYSMEM_FLAGS  flags;          ///< System memory flags.
    UINT_32                 sizeInBytes;    ///< System memory allocation size in bytes.
    ADDR_CLIENT_HANDLE      hClient;        ///< Client handle
} ADDR_ALLOCSYSMEM_INPUT;

/**
****************************************************************************************************
* ADDR_ALLOCSYSMEM
*   @brief
*       Allocate system memory callback function. Returns valid pointer on success.
****************************************************************************************************
*/
typedef VOID* (ADDR_API* ADDR_ALLOCSYSMEM)(
    const ADDR_ALLOCSYSMEM_INPUT* pInput);

/**
****************************************************************************************************
* @brief Free system memory input structure
****************************************************************************************************
*/
typedef struct _ADDR_FREESYSMEM_INPUT
{
    UINT_32                 size;           ///< Size of this structure in bytes

    VOID*                   pVirtAddr;      ///< Virtual address
    ADDR_CLIENT_HANDLE      hClient;        ///< Client handle
} ADDR_FREESYSMEM_INPUT;

/**
****************************************************************************************************
* ADDR_FREESYSMEM
*   @brief
*       Free system memory callback function.
*       Returns ADDR_OK on success.
****************************************************************************************************
*/
typedef ADDR_E_RETURNCODE (ADDR_API* ADDR_FREESYSMEM)(
    const ADDR_FREESYSMEM_INPUT* pInput);

/**
****************************************************************************************************
* @brief Print debug message input structure
****************************************************************************************************
*/
typedef struct _ADDR_DEBUGPRINT_INPUT
{
    UINT_32             size;           ///< Size of this structure in bytes

    CHAR*               pDebugString;   ///< Debug print string
    va_list             ap;             ///< Variable argument list
    ADDR_CLIENT_HANDLE  hClient;        ///< Client handle
} ADDR_DEBUGPRINT_INPUT;

/**
****************************************************************************************************
* ADDR_DEBUGPRINT
*   @brief
*       Print debug message callback function.
*       Returns ADDR_OK on success.
****************************************************************************************************
*/
typedef ADDR_E_RETURNCODE (ADDR_API* ADDR_DEBUGPRINT)(
    const ADDR_DEBUGPRINT_INPUT* pInput);

/**
****************************************************************************************************
* ADDR_CALLBACKS
*
*   @brief
*       Address Library needs client to provide system memory alloc/free routines.
****************************************************************************************************
*/
typedef struct _ADDR_CALLBACKS
{
    ADDR_ALLOCSYSMEM allocSysMem;   ///< Routine to allocate system memory
    ADDR_FREESYSMEM  freeSysMem;    ///< Routine to free system memory
    ADDR_DEBUGPRINT  debugPrint;    ///< Routine to print debug message
} ADDR_CALLBACKS;

////////////////////////////////////////////////////////////////////////////////////////////////////
//                               Create/Destroy functions
////////////////////////////////////////////////////////////////////////////////////////////////////

/**
****************************************************************************************************
* ADDR_CREATE_FLAGS
*
*   @brief
*       This structure is used to pass some setup in creation of AddrLib
*   @note
****************************************************************************************************
*/
typedef union _ADDR_CREATE_FLAGS
{
    struct
    {
        UINT_32 noCubeMipSlicesPad     : 1;    ///< Turn cubemap faces padding off
        UINT_32 fillSizeFields         : 1;    ///< If clients fill size fields in all input and
                                               ///  output structure
        UINT_32 useTileIndex           : 1;    ///< Make tileIndex field in input valid
        UINT_32 useCombinedSwizzle     : 1;    ///< Use combined tile swizzle
        UINT_32 checkLast2DLevel       : 1;    ///< Check the last 2D mip sub level
        UINT_32 useHtileSliceAlign     : 1;    ///< Do htile single slice alignment
        UINT_32 allowLargeThickTile    : 1;    ///< Allow 64*thickness*bytesPerPixel > rowSize
        UINT_32 forceDccAndTcCompat    : 1;    ///< Force enable DCC and TC compatibility
        UINT_32 nonPower2MemConfig     : 1;    ///< Video memory bit width is not power of 2
        UINT_32 enableAltTiling        : 1;    ///< Enable alt tile mode
        UINT_32 reserved               : 22;   ///< Reserved bits for future use
    };

    UINT_32 value;
} ADDR_CREATE_FLAGS;

/**
****************************************************************************************************
*   ADDR_REGISTER_VALUE
*
*   @brief
*       Data from registers to setup AddrLib global data, used in AddrCreate
****************************************************************************************************
*/
typedef struct _ADDR_REGISTER_VALUE
{
    UINT_32  gbAddrConfig;       ///< For R8xx, use GB_ADDR_CONFIG register value.
                                 ///  For R6xx/R7xx, use GB_TILING_CONFIG.
                                 ///  But they can be treated as the same.
                                 ///  if this value is 0, use chip to set default value
    UINT_32  backendDisables;    ///< 1 bit per backend, starting with LSB. 1=disabled,0=enabled.
                                 ///  Register value of CC_RB_BACKEND_DISABLE.BACKEND_DISABLE

                                 ///  R800 registers-----------------------------------------------
    UINT_32  noOfBanks;          ///< Number of h/w ram banks - For r800: MC_ARB_RAMCFG.NOOFBANK
                                 ///  No enums for this value in h/w header files
                                 ///  0: 4
                                 ///  1: 8
                                 ///  2: 16
    UINT_32  noOfRanks;          ///  MC_ARB_RAMCFG.NOOFRANK
                                 ///  0: 1
                                 ///  1: 2
                                 ///  SI (R1000) registers-----------------------------------------
    const UINT_32* pTileConfig;  ///< Global tile setting tables
    UINT_32  noOfEntries;        ///< Number of entries in pTileConfig

                                 ///< CI registers-------------------------------------------------
    const UINT_32* pMacroTileConfig;    ///< Global macro tile mode table
    UINT_32  noOfMacroEntries;   ///< Number of entries in pMacroTileConfig
} ADDR_REGISTER_VALUE;

/**
****************************************************************************************************
* ADDR_CREATE_INPUT
*
*   @brief
*       Parameters use to create an AddrLib Object. Caller must provide all fields.
*
****************************************************************************************************
*/
typedef struct _ADDR_CREATE_INPUT
{
    UINT_32             size;                ///< Size of this structure in bytes

    UINT_32             chipEngine;          ///< Chip Engine
    UINT_32             chipFamily;          ///< Chip Family
    UINT_32             chipRevision;        ///< Chip Revision
    ADDR_CALLBACKS      callbacks;           ///< Callbacks for sysmem alloc/free/print
    ADDR_CREATE_FLAGS   createFlags;         ///< Flags to setup AddrLib
    ADDR_REGISTER_VALUE regValue;            ///< Data from registers to setup AddrLib global data
    ADDR_CLIENT_HANDLE  hClient;             ///< Client handle
    UINT_32             minPitchAlignPixels; ///< Minimum pitch alignment in pixels
} ADDR_CREATE_INPUT;

/**
****************************************************************************************************
* ADDR_CREATEINFO_OUTPUT
*
*   @brief
*       Return AddrLib handle to client driver
*
****************************************************************************************************
*/
typedef struct _ADDR_CREATE_OUTPUT
{
    UINT_32              size;            ///< Size of this structure in bytes

    ADDR_HANDLE          hLib;            ///< Address lib handle

    UINT_32              numEquations;    ///< Number of equations in the table
    const ADDR_EQUATION* pEquationTable;  ///< Pointer to the equation table
} ADDR_CREATE_OUTPUT;

/**
****************************************************************************************************
*   AddrCreate
*
*   @brief
*       Create AddrLib object, must be called before any interface calls
*
*   @return
*       ADDR_OK if successful
****************************************************************************************************
*/
ADDR_E_RETURNCODE ADDR_API AddrCreate(
    const ADDR_CREATE_INPUT*    pAddrCreateIn,
    ADDR_CREATE_OUTPUT*         pAddrCreateOut);



/**
****************************************************************************************************
*   AddrDestroy
*
*   @brief
*       Destroy AddrLib object, must be called to free internally allocated resources.
*
*   @return
*      ADDR_OK if successful
****************************************************************************************************
*/
ADDR_E_RETURNCODE ADDR_API AddrDestroy(
    ADDR_HANDLE hLib);



////////////////////////////////////////////////////////////////////////////////////////////////////
//                                    Surface functions
////////////////////////////////////////////////////////////////////////////////////////////////////

/**
****************************************************************************************************
* @brief
*       Bank/tiling parameters. On function input, these can be set as desired or
*       left 0 for AddrLib to calculate/default. On function output, these are the actual
*       parameters used.
* @note
*       Valid bankWidth/bankHeight value:
*       1,2,4,8. They are factors instead of pixels or bytes.
*
*       The bank number remains constant across each row of the
*       macro tile as each pipe is selected, so the number of
*       tiles in the x direction with the same bank number will
*       be bank_width * num_pipes.
****************************************************************************************************
*/
typedef struct _ADDR_TILEINFO
{
    ///  Any of these parameters can be set to 0 to use the HW default.
    UINT_32     banks;              ///< Number of banks, numerical value
    UINT_32     bankWidth;          ///< Number of tiles in the X direction in the same bank
    UINT_32     bankHeight;         ///< Number of tiles in the Y direction in the same bank
    UINT_32     macroAspectRatio;   ///< Macro tile aspect ratio. 1-1:1, 2-4:1, 4-16:1, 8-64:1
    UINT_32     tileSplitBytes;     ///< Tile split size, in bytes
    AddrPipeCfg pipeConfig;         ///< Pipe Config = HW enum + 1
} ADDR_TILEINFO;

// Create a define to avoid client change. The removal of R800 is because we plan to implement SI
// within 800 HWL - An AddrPipeCfg is added in above data structure
typedef ADDR_TILEINFO ADDR_R800_TILEINFO;

/**
****************************************************************************************************
* @brief
*       Information needed by quad buffer stereo support
****************************************************************************************************
*/
typedef struct _ADDR_QBSTEREOINFO
{
    UINT_32         eyeHeight;          ///< Height (in pixel rows) to right eye
    UINT_32         rightOffset;        ///< Offset (in bytes) to right eye
    UINT_32         rightSwizzle;       ///< TileSwizzle for right eyes
} ADDR_QBSTEREOINFO;

/**
****************************************************************************************************
*   ADDR_SURFACE_FLAGS
*
*   @brief
*       Surface flags
****************************************************************************************************
*/
typedef union _ADDR_SURFACE_FLAGS
{
    struct
    {
        UINT_32 color                : 1; ///< Flag indicates this is a color buffer
        UINT_32 depth                : 1; ///< Flag indicates this is a depth/stencil buffer
        UINT_32 stencil              : 1; ///< Flag indicates this is a stencil buffer
        UINT_32 texture              : 1; ///< Flag indicates this is a texture
        UINT_32 cube                 : 1; ///< Flag indicates this is a cubemap
        UINT_32 volume               : 1; ///< Flag indicates this is a volume texture
        UINT_32 fmask                : 1; ///< Flag indicates this is an fmask
        UINT_32 cubeAsArray          : 1; ///< Flag indicates if treat cubemap as arrays
        UINT_32 compressZ            : 1; ///< Flag indicates z buffer is compressed
        UINT_32 overlay              : 1; ///< Flag indicates this is an overlay surface
        UINT_32 noStencil            : 1; ///< Flag indicates this depth has no separate stencil
        UINT_32 display              : 1; ///< Flag indicates this should match display controller req.
        UINT_32 opt4Space            : 1; ///< Flag indicates this surface should be optimized for space
                                          ///  i.e. save some memory but may lose performance
        UINT_32 prt                  : 1; ///< Flag for partially resident texture
        UINT_32 qbStereo             : 1; ///< Quad buffer stereo surface
        UINT_32 pow2Pad              : 1; ///< SI: Pad to pow2, must set for mipmap (include level0)
        UINT_32 interleaved          : 1; ///< Special flag for interleaved YUV surface padding
        UINT_32 tcCompatible         : 1; ///< Flag indicates surface needs to be shader readable
        UINT_32 dispTileType         : 1; ///< NI: force display Tiling for 128 bit shared resoruce
        UINT_32 dccCompatible        : 1; ///< VI: whether to make MSAA surface support dcc fast clear
        UINT_32 dccPipeWorkaround    : 1; ///< VI: whether to workaround the HW limit that
                                          ///  dcc can't be enabled if pipe config of tile mode
                                          ///  is different from that of ASIC, this flag
                                          ///  is address lib internal flag, client should ignore it
        UINT_32 czDispCompatible     : 1; ///< SI+: CZ family has a HW bug needs special alignment.
                                          ///  This flag indicates we need to follow the
                                          ///  alignment with CZ families or other ASICs under
                                          ///  PX configuration + CZ.
        UINT_32 nonSplit             : 1; ///< CI: depth texture should not be split
        UINT_32 disableLinearOpt     : 1; ///< Disable tile mode optimization to linear
        UINT_32 needEquation         : 1; ///< Make the surface tile setting equation compatible.
                                          ///  This flag indicates we need to override tile
                                          ///  mode to PRT_* tile mode to disable slice rotation,
                                          ///  which is needed by swizzle pattern equation.
        UINT_32 skipIndicesOutput    : 1; ///< Skipping indices in output.
        UINT_32 rotateDisplay        : 1; ///< Rotate micro tile type
        UINT_32 minimizeAlignment    : 1; ///< Minimize alignment
        UINT_32 preferEquation       : 1; ///< Return equation index without adjusting tile mode
        UINT_32 matchStencilTileCfg  : 1; ///< Select tile index of stencil as well as depth surface
                                          ///  to make sure they share same tile config parameters
        UINT_32 disallowLargeThickDegrade   : 1;    ///< Disallow large thick tile degrade
        UINT_32 reserved             : 1; ///< Reserved bits
    };

    UINT_32 value;
} ADDR_SURFACE_FLAGS;

/**
****************************************************************************************************
*   ADDR_COMPUTE_SURFACE_INFO_INPUT
*
*   @brief
*       Input structure for AddrComputeSurfaceInfo
****************************************************************************************************
*/
typedef struct _ADDR_COMPUTE_SURFACE_INFO_INPUT
{
    UINT_32             size;               ///< Size of this structure in bytes

    AddrTileMode        tileMode;           ///< Tile mode
    AddrFormat          format;             ///< If format is set to valid one, bpp/width/height
                                            ///  might be overwritten
    UINT_32             bpp;                ///< Bits per pixel
    UINT_32             numSamples;         ///< Number of samples
    UINT_32             width;              ///< Width, in pixels
    UINT_32             height;             ///< Height, in pixels
    UINT_32             numSlices;          ///< Number of surface slices or depth
    UINT_32             slice;              ///< Slice index
    UINT_32             mipLevel;           ///< Current mipmap level
    UINT_32             numMipLevels;       ///< Number of mips in mip chain
    ADDR_SURFACE_FLAGS  flags;              ///< Surface type flags
    UINT_32             numFrags;           ///< Number of fragments, leave it zero or the same as
                                            ///  number of samples for normal AA; Set it to the
                                            ///  number of fragments for EQAA
    /// r800 and later HWL parameters
    // Needed by 2D tiling, for linear and 1D tiling, just keep them 0's
    ADDR_TILEINFO*      pTileInfo;          ///< 2D tile parameters. Set to 0 to default/calculate
    AddrTileType        tileType;           ///< Micro tiling type, not needed when tileIndex != -1
    INT_32              tileIndex;          ///< Tile index, MUST be -1 if you don't want to use it
                                            ///  while the global useTileIndex is set to 1
    UINT_32             basePitch;          ///< Base level pitch in pixels, 0 means ignored, is a
                                            ///  must for mip levels from SI+.
                                            ///  Don't use pitch in blocks for compressed formats!
    UINT_32             maxBaseAlign;       ///< Max base alignment request from client
    UINT_32             pitchAlign;         ///< Pitch alignment request from client
    UINT_32             heightAlign;        ///< Height alignment request from client
} ADDR_COMPUTE_SURFACE_INFO_INPUT;

/**
****************************************************************************************************
*   ADDR_COMPUTE_SURFACE_INFO_OUTPUT
*
*   @brief
*       Output structure for AddrComputeSurfInfo
*   @note
        Element: AddrLib unit for computing. e.g. BCn: 4x4 blocks; R32B32B32: 32bit with 3x pitch
        Pixel: Original pixel
****************************************************************************************************
*/
typedef struct _ADDR_COMPUTE_SURFACE_INFO_OUTPUT
{
    UINT_32         size;           ///< Size of this structure in bytes

    UINT_32         pitch;          ///< Pitch in elements (in blocks for compressed formats)
    UINT_32         height;         ///< Height in elements (in blocks for compressed formats)
    UINT_32         depth;          ///< Number of slice/depth
    UINT_64         surfSize;       ///< Surface size in bytes
    AddrTileMode    tileMode;       ///< Actual tile mode. May differ from that in input
    UINT_32         baseAlign;      ///< Base address alignment
    UINT_32         pitchAlign;     ///< Pitch alignment, in elements
    UINT_32         heightAlign;    ///< Height alignment, in elements
    UINT_32         depthAlign;     ///< Depth alignment, aligned to thickness, for 3d texture
    UINT_32         bpp;            ///< Bits per elements (e.g. blocks for BCn, 1/3 for 96bit)
    UINT_32         pixelPitch;     ///< Pitch in original pixels
    UINT_32         pixelHeight;    ///< Height in original pixels
    UINT_32         pixelBits;      ///< Original bits per pixel, passed from input
    UINT_64         sliceSize;      ///< Size of slice specified by input's slice
                                    ///  The result is controlled by surface flags & createFlags
                                    ///  By default this value equals to surfSize for volume
    UINT_32         pitchTileMax;   ///< PITCH_TILE_MAX value for h/w register
    UINT_32         heightTileMax;  ///< HEIGHT_TILE_MAX value for h/w register
    UINT_32         sliceTileMax;   ///< SLICE_TILE_MAX value for h/w register

    UINT_32         numSamples;     ///< Pass the effective numSamples processed in this call

    /// r800 and later HWL parameters
    ADDR_TILEINFO*  pTileInfo;      ///< Tile parameters used. Filled in if 0 on input
    AddrTileType    tileType;       ///< Micro tiling type, only valid when tileIndex != -1
    INT_32          tileIndex;      ///< Tile index, MAY be "downgraded"

    INT_32          macroModeIndex; ///< Index in macro tile mode table if there is one (CI)
    /// Output flags
    struct
    {
        /// Special information to work around SI mipmap swizzle bug UBTS #317508
        UINT_32     last2DLevel  : 1;  ///< TRUE if this is the last 2D(3D) tiled
                                       ///< Only meaningful when create flag checkLast2DLevel is set
        UINT_32     tcCompatible : 1;  ///< If the surface can be shader compatible
        UINT_32     dccUnsupport : 1;  ///< If the surface can support DCC compressed rendering
        UINT_32     prtTileIndex : 1;  ///< SI only, indicate the returned tile index is for PRT
                                       ///< If address lib return true for mip 0, client should set prt flag
                                       ///< for child mips in subsequent compute surface info calls
        UINT_32     reserved     :28;  ///< Reserved bits
    };

    UINT_32         equationIndex;     ///< Equation index in the equation table;

    UINT_32         blockWidth;        ///< Width in element inside one block(1D->Micro, 2D->Macro)
    UINT_32         blockHeight;       ///< Height in element inside one block(1D->Micro, 2D->Macro)
    UINT_32         blockSlices;       ///< Slice number inside one block(1D->Micro, 2D->Macro)

    /// Stereo info
    ADDR_QBSTEREOINFO*  pStereoInfo;///< Stereo information, needed when .qbStereo flag is TRUE

    INT_32          stencilTileIdx; ///< stencil tile index output when matchStencilTileCfg was set
} ADDR_COMPUTE_SURFACE_INFO_OUTPUT;

/**
****************************************************************************************************
*   AddrComputeSurfaceInfo
*
*   @brief
*       Compute surface width/height/depth/alignments and suitable tiling mode
****************************************************************************************************
*/
ADDR_E_RETURNCODE ADDR_API AddrComputeSurfaceInfo(
    ADDR_HANDLE                             hLib,
    const ADDR_COMPUTE_SURFACE_INFO_INPUT*  pIn,
    ADDR_COMPUTE_SURFACE_INFO_OUTPUT*       pOut);



/**
****************************************************************************************************
*   ADDR_COMPUTE_SURFACE_ADDRFROMCOORD_INPUT
*
*   @brief
*       Input structure for AddrComputeSurfaceAddrFromCoord
****************************************************************************************************
*/
typedef struct _ADDR_COMPUTE_SURFACE_ADDRFROMCOORD_INPUT
{
    UINT_32         size;               ///< Size of this structure in bytes

    UINT_32         x;                  ///< X coordinate
    UINT_32         y;                  ///< Y coordinate
    UINT_32         slice;              ///< Slice index
    UINT_32         sample;             ///< Sample index, use fragment index for EQAA

    UINT_32         bpp;                ///< Bits per pixel
    UINT_32         pitch;              ///< Surface pitch, in pixels
    UINT_32         height;             ///< Surface height, in pixels
    UINT_32         numSlices;          ///< Surface depth
    UINT_32         numSamples;         ///< Number of samples

    AddrTileMode    tileMode;           ///< Tile mode
    BOOL_32         isDepth;            ///< TRUE if the surface uses depth sample ordering within
                                        ///  micro tile. Textures can also choose depth sample order
    UINT_32         tileBase;           ///< Base offset (in bits) inside micro tile which handles
                                        ///  the case that components are stored separately
    UINT_32         compBits;           ///< The component bits actually needed(for planar surface)

    UINT_32         numFrags;           ///< Number of fragments, leave it zero or the same as
                                        ///  number of samples for normal AA; Set it to the
                                        ///  number of fragments for EQAA
    /// r800 and later HWL parameters
    // Used for 1D tiling above
    AddrTileType    tileType;           ///< See defintion of AddrTileType
    struct
    {
        UINT_32     ignoreSE : 1;       ///< TRUE if shader engines are ignored. This is texture
                                        ///  only flag. Only non-RT texture can set this to TRUE
        UINT_32     reserved :31;       ///< Reserved for future use.
    };
    // 2D tiling needs following structure
    ADDR_TILEINFO*  pTileInfo;          ///< 2D tile parameters. Client must provide all data
    INT_32          tileIndex;          ///< Tile index, MUST be -1 if you don't want to use it
                                        ///  while the global useTileIndex is set to 1
    union
    {
        struct
        {
            UINT_32  bankSwizzle;       ///< Bank swizzle
            UINT_32  pipeSwizzle;       ///< Pipe swizzle
        };
        UINT_32     tileSwizzle;        ///< Combined swizzle, if useCombinedSwizzle is TRUE
    };
} ADDR_COMPUTE_SURFACE_ADDRFROMCOORD_INPUT;

/**
****************************************************************************************************
*   ADDR_COMPUTE_SURFACE_ADDRFROMCOORD_OUTPUT
*
*   @brief
*       Output structure for AddrComputeSurfaceAddrFromCoord
****************************************************************************************************
*/
typedef struct _ADDR_COMPUTE_SURFACE_ADDRFROMCOORD_OUTPUT
{
    UINT_32 size;           ///< Size of this structure in bytes

    UINT_64 addr;           ///< Byte address
    UINT_32 bitPosition;    ///< Bit position within surfaceAddr, 0-7.
                            ///  For surface bpp < 8, e.g. FMT_1.
    UINT_32 prtBlockIndex;  ///< Index of a PRT tile (64K block)
} ADDR_COMPUTE_SURFACE_ADDRFROMCOORD_OUTPUT;

/**
****************************************************************************************************
*   AddrComputeSurfaceAddrFromCoord
*
*   @brief
*       Compute surface address from a given coordinate.
****************************************************************************************************
*/
ADDR_E_RETURNCODE ADDR_API AddrComputeSurfaceAddrFromCoord(
    ADDR_HANDLE                                     hLib,
    const ADDR_COMPUTE_SURFACE_ADDRFROMCOORD_INPUT* pIn,
    ADDR_COMPUTE_SURFACE_ADDRFROMCOORD_OUTPUT*      pOut);



/**
****************************************************************************************************
*   ADDR_COMPUTE_SURFACE_COORDFROMADDR_INPUT
*
*   @brief
*       Input structure for AddrComputeSurfaceCoordFromAddr
****************************************************************************************************
*/
typedef struct _ADDR_COMPUTE_SURFACE_COORDFROMADDR_INPUT
{
    UINT_32         size;               ///< Size of this structure in bytes

    UINT_64         addr;               ///< Address in bytes
    UINT_32         bitPosition;        ///< Bit position in addr. 0-7. for surface bpp < 8,
                                        ///  e.g. FMT_1;
    UINT_32         bpp;                ///< Bits per pixel
    UINT_32         pitch;              ///< Pitch, in pixels
    UINT_32         height;             ///< Height in pixels
    UINT_32         numSlices;          ///< Surface depth
    UINT_32         numSamples;         ///< Number of samples

    AddrTileMode    tileMode;           ///< Tile mode
    BOOL_32         isDepth;            ///< Surface uses depth sample ordering within micro tile.
                                        ///  Note: Textures can choose depth sample order as well.
    UINT_32         tileBase;           ///< Base offset (in bits) inside micro tile which handles
                                        ///  the case that components are stored separately
    UINT_32         compBits;           ///< The component bits actually needed(for planar surface)

    UINT_32         numFrags;           ///< Number of fragments, leave it zero or the same as
                                        ///  number of samples for normal AA; Set it to the
                                        ///  number of fragments for EQAA
    /// r800 and later HWL parameters
    // Used for 1D tiling above
    AddrTileType    tileType;           ///< See defintion of AddrTileType
    struct
    {
        UINT_32     ignoreSE : 1;       ///< TRUE if shader engines are ignored. This is texture
                                        ///  only flag. Only non-RT texture can set this to TRUE
        UINT_32     reserved :31;       ///< Reserved for future use.
    };
    // 2D tiling needs following structure
    ADDR_TILEINFO*  pTileInfo;          ///< 2D tile parameters. Client must provide all data
    INT_32          tileIndex;          ///< Tile index, MUST be -1 if you don't want to use it
                                        ///  while the global useTileIndex is set to 1
    union
    {
        struct
        {
            UINT_32  bankSwizzle;       ///< Bank swizzle
            UINT_32  pipeSwizzle;       ///< Pipe swizzle
        };
        UINT_32     tileSwizzle;        ///< Combined swizzle, if useCombinedSwizzle is TRUE
    };
} ADDR_COMPUTE_SURFACE_COORDFROMADDR_INPUT;

/**
****************************************************************************************************
*   ADDR_COMPUTE_SURFACE_COORDFROMADDR_OUTPUT
*
*   @brief
*       Output structure for AddrComputeSurfaceCoordFromAddr
****************************************************************************************************
*/
typedef struct _ADDR_COMPUTE_SURFACE_COORDFROMADDR_OUTPUT
{
    UINT_32 size;   ///< Size of this structure in bytes

    UINT_32 x;      ///< X coordinate
    UINT_32 y;      ///< Y coordinate
    UINT_32 slice;  ///< Index of slices
    UINT_32 sample; ///< Index of samples, means fragment index for EQAA
} ADDR_COMPUTE_SURFACE_COORDFROMADDR_OUTPUT;

/**
****************************************************************************************************
*   AddrComputeSurfaceCoordFromAddr
*
*   @brief
*       Compute coordinate from a given surface address
****************************************************************************************************
*/
ADDR_E_RETURNCODE ADDR_API AddrComputeSurfaceCoordFromAddr(
    ADDR_HANDLE                                     hLib,
    const ADDR_COMPUTE_SURFACE_COORDFROMADDR_INPUT* pIn,
    ADDR_COMPUTE_SURFACE_COORDFROMADDR_OUTPUT*      pOut);

////////////////////////////////////////////////////////////////////////////////////////////////////
//                                   HTile functions
////////////////////////////////////////////////////////////////////////////////////////////////////

/**
****************************************************************************************************
*   ADDR_HTILE_FLAGS
*
*   @brief
*       HTILE flags
****************************************************************************************************
*/
typedef union _ADDR_HTILE_FLAGS
{
    struct
    {
        UINT_32 tcCompatible          : 1;  ///< Flag indicates surface needs to be shader readable
        UINT_32 skipTcCompatSizeAlign : 1;  ///< Flag indicates that addrLib will not align htile
                                            ///  size to 256xBankxPipe when computing tc-compatible
                                            ///  htile info.
        UINT_32 reserved              : 30; ///< Reserved bits
    };

    UINT_32 value;
} ADDR_HTILE_FLAGS;

/**
****************************************************************************************************
*   ADDR_COMPUTE_HTILE_INFO_INPUT
*
*   @brief
*       Input structure of AddrComputeHtileInfo
****************************************************************************************************
*/
typedef struct _ADDR_COMPUTE_HTILE_INFO_INPUT
{
    UINT_32            size;            ///< Size of this structure in bytes

    ADDR_HTILE_FLAGS   flags;           ///< HTILE flags
    UINT_32            pitch;           ///< Surface pitch, in pixels
    UINT_32            height;          ///< Surface height, in pixels
    UINT_32            numSlices;       ///< Number of slices
    BOOL_32            isLinear;        ///< Linear or tiled HTILE layout
    AddrHtileBlockSize blockWidth;      ///< 4 or 8. EG above only support 8
    AddrHtileBlockSize blockHeight;     ///< 4 or 8. EG above only support 8
    ADDR_TILEINFO*     pTileInfo;       ///< Tile info

    INT_32             tileIndex;       ///< Tile index, MUST be -1 if you don't want to use it
                                        ///  while the global useTileIndex is set to 1
    INT_32             macroModeIndex;  ///< Index in macro tile mode table if there is one (CI)
                                        ///< README: When tileIndex is not -1, this must be valid
} ADDR_COMPUTE_HTILE_INFO_INPUT;

/**
****************************************************************************************************
*   ADDR_COMPUTE_HTILE_INFO_OUTPUT
*
*   @brief
*       Output structure of AddrComputeHtileInfo
****************************************************************************************************
*/
typedef struct _ADDR_COMPUTE_HTILE_INFO_OUTPUT
{
    UINT_32 size;               ///< Size of this structure in bytes

    UINT_32 pitch;              ///< Pitch in pixels of depth buffer represented in this
                                ///  HTile buffer. This might be larger than original depth
                                ///  buffer pitch when called with an unaligned pitch.
    UINT_32 height;             ///< Height in pixels, as above
    UINT_64 htileBytes;         ///< Size of HTILE buffer, in bytes
    UINT_32 baseAlign;          ///< Base alignment
    UINT_32 bpp;                ///< Bits per pixel for HTILE is how many bits for an 8x8 block!
    UINT_32 macroWidth;         ///< Macro width in pixels, actually squared cache shape
    UINT_32 macroHeight;        ///< Macro height in pixels
    UINT_64 sliceSize;          ///< Slice size, in bytes.
    BOOL_32 sliceInterleaved;   ///< Flag to indicate if different slice's htile is interleaved
                                ///  Compute engine clear can't be used if htile is interleaved
    BOOL_32 nextMipLevelCompressible;   ///< Flag to indicate whether HTILE can be enabled in
                                        ///  next mip level, it also indicates if memory set based
                                        ///  fast clear can be used for current mip level.
} ADDR_COMPUTE_HTILE_INFO_OUTPUT;

/**
****************************************************************************************************
*   AddrComputeHtileInfo
*
*   @brief
*       Compute Htile pitch, height, base alignment and size in bytes
****************************************************************************************************
*/
ADDR_E_RETURNCODE ADDR_API AddrComputeHtileInfo(
    ADDR_HANDLE                             hLib,
    const ADDR_COMPUTE_HTILE_INFO_INPUT*    pIn,
    ADDR_COMPUTE_HTILE_INFO_OUTPUT*         pOut);



/**
****************************************************************************************************
*   ADDR_COMPUTE_HTILE_ADDRFROMCOORD_INPUT
*
*   @brief
*       Input structure for AddrComputeHtileAddrFromCoord
****************************************************************************************************
*/
typedef struct _ADDR_COMPUTE_HTILE_ADDRFROMCOORD_INPUT
{
    UINT_32            size;            ///< Size of this structure in bytes

    UINT_32            pitch;           ///< Pitch, in pixels
    UINT_32            height;          ///< Height in pixels
    UINT_32            x;               ///< X coordinate
    UINT_32            y;               ///< Y coordinate
    UINT_32            slice;           ///< Index of slice
    UINT_32            numSlices;       ///< Number of slices
    BOOL_32            isLinear;        ///< Linear or tiled HTILE layout
    ADDR_HTILE_FLAGS   flags;           ///< htile flags
    AddrHtileBlockSize blockWidth;      ///< 4 or 8. 1 means 8, 0 means 4. EG above only support 8
    AddrHtileBlockSize blockHeight;     ///< 4 or 8. 1 means 8, 0 means 4. EG above only support 8
    ADDR_TILEINFO*     pTileInfo;       ///< Tile info

    INT_32             tileIndex;       ///< Tile index, MUST be -1 if you don't want to use it
                                        ///  while the global useTileIndex is set to 1
    INT_32             macroModeIndex;  ///< Index in macro tile mode table if there is one (CI)
                                        ///< README: When tileIndex is not -1, this must be valid
    UINT_32            bpp;             ///< depth/stencil buffer bit per pixel size
    UINT_32            zStencilAddr;    ///< tcCompatible Z/Stencil surface address
} ADDR_COMPUTE_HTILE_ADDRFROMCOORD_INPUT;

/**
****************************************************************************************************
*   ADDR_COMPUTE_HTILE_ADDRFROMCOORD_OUTPUT
*
*   @brief
*       Output structure for AddrComputeHtileAddrFromCoord
****************************************************************************************************
*/
typedef struct _ADDR_COMPUTE_HTILE_ADDRFROMCOORD_OUTPUT
{
    UINT_32 size;           ///< Size of this structure in bytes

    UINT_64 addr;           ///< Address in bytes
    UINT_32 bitPosition;    ///< Bit position, 0 or 4. CMASK and HTILE shares some lib method.
                            ///  So we keep bitPosition for HTILE as well
} ADDR_COMPUTE_HTILE_ADDRFROMCOORD_OUTPUT;

/**
****************************************************************************************************
*   AddrComputeHtileAddrFromCoord
*
*   @brief
*       Compute Htile address according to coordinates (of depth buffer)
****************************************************************************************************
*/
ADDR_E_RETURNCODE ADDR_API AddrComputeHtileAddrFromCoord(
    ADDR_HANDLE                                     hLib,
    const ADDR_COMPUTE_HTILE_ADDRFROMCOORD_INPUT*   pIn,
    ADDR_COMPUTE_HTILE_ADDRFROMCOORD_OUTPUT*        pOut);



/**
****************************************************************************************************
*   ADDR_COMPUTE_HTILE_COORDFROMADDR_INPUT
*
*   @brief
*       Input structure for AddrComputeHtileCoordFromAddr
****************************************************************************************************
*/
typedef struct _ADDR_COMPUTE_HTILE_COORDFROMADDR_INPUT
{
    UINT_32            size;            ///< Size of this structure in bytes

    UINT_64            addr;            ///< Address
    UINT_32            bitPosition;     ///< Bit position 0 or 4. CMASK and HTILE share some methods
                                        ///  so we keep bitPosition for HTILE as well
    UINT_32            pitch;           ///< Pitch, in pixels
    UINT_32            height;          ///< Height, in pixels
    UINT_32            numSlices;       ///< Number of slices
    BOOL_32            isLinear;        ///< Linear or tiled HTILE layout
    AddrHtileBlockSize blockWidth;      ///< 4 or 8. 1 means 8, 0 means 4. R8xx/R9xx only support 8
    AddrHtileBlockSize blockHeight;     ///< 4 or 8. 1 means 8, 0 means 4. R8xx/R9xx only support 8
    ADDR_TILEINFO*     pTileInfo;       ///< Tile info

    INT_32             tileIndex;       ///< Tile index, MUST be -1 if you don't want to use it
                                        ///  while the global useTileIndex is set to 1
    INT_32             macroModeIndex;  ///< Index in macro tile mode table if there is one (CI)
                                        ///< README: When tileIndex is not -1, this must be valid
} ADDR_COMPUTE_HTILE_COORDFROMADDR_INPUT;

/**
****************************************************************************************************
*   ADDR_COMPUTE_HTILE_COORDFROMADDR_OUTPUT
*
*   @brief
*       Output structure for AddrComputeHtileCoordFromAddr
****************************************************************************************************
*/
typedef struct _ADDR_COMPUTE_HTILE_COORDFROMADDR_OUTPUT
{
    UINT_32 size;   ///< Size of this structure in bytes

    UINT_32 x;      ///< X coordinate
    UINT_32 y;      ///< Y coordinate
    UINT_32 slice;  ///< Slice index
} ADDR_COMPUTE_HTILE_COORDFROMADDR_OUTPUT;

/**
****************************************************************************************************
*   AddrComputeHtileCoordFromAddr
*
*   @brief
*       Compute coordinates within depth buffer (1st pixel of a micro tile) according to
*       Htile address
****************************************************************************************************
*/
ADDR_E_RETURNCODE ADDR_API AddrComputeHtileCoordFromAddr(
    ADDR_HANDLE                                     hLib,
    const ADDR_COMPUTE_HTILE_COORDFROMADDR_INPUT*   pIn,
    ADDR_COMPUTE_HTILE_COORDFROMADDR_OUTPUT*        pOut);



////////////////////////////////////////////////////////////////////////////////////////////////////
//                                     C-mask functions
////////////////////////////////////////////////////////////////////////////////////////////////////

/**
****************************************************************************************************
*   ADDR_CMASK_FLAGS
*
*   @brief
*       CMASK flags
****************************************************************************************************
*/
typedef union _ADDR_CMASK_FLAGS
{
    struct
    {
        UINT_32 tcCompatible  : 1; ///< Flag indicates surface needs to be shader readable
        UINT_32 reserved      :31; ///< Reserved bits
    };

    UINT_32 value;
} ADDR_CMASK_FLAGS;

/**
****************************************************************************************************
*   ADDR_COMPUTE_CMASK_INFO_INPUT
*
*   @brief
*       Input structure of AddrComputeCmaskInfo
****************************************************************************************************
*/
typedef struct _ADDR_COMPUTE_CMASKINFO_INPUT
{
    UINT_32             size;            ///< Size of this structure in bytes

    ADDR_CMASK_FLAGS    flags;           ///< CMASK flags
    UINT_32             pitch;           ///< Pitch, in pixels, of color buffer
    UINT_32             height;          ///< Height, in pixels, of color buffer
    UINT_32             numSlices;       ///< Number of slices, of color buffer
    BOOL_32             isLinear;        ///< Linear or tiled layout, Only SI can be linear
    ADDR_TILEINFO*      pTileInfo;       ///< Tile info

    INT_32              tileIndex;       ///< Tile index, MUST be -1 if you don't want to use it
                                         ///  while the global useTileIndex is set to 1
    INT_32              macroModeIndex;  ///< Index in macro tile mode table if there is one (CI)
                                         ///< README: When tileIndex is not -1, this must be valid
} ADDR_COMPUTE_CMASK_INFO_INPUT;

/**
****************************************************************************************************
*   ADDR_COMPUTE_CMASK_INFO_OUTPUT
*
*   @brief
*       Output structure of AddrComputeCmaskInfo
****************************************************************************************************
*/
typedef struct _ADDR_COMPUTE_CMASK_INFO_OUTPUT
{
    UINT_32 size;           ///< Size of this structure in bytes

    UINT_32 pitch;          ///< Pitch in pixels of color buffer which
                            ///  this Cmask matches. The size might be larger than
                            ///  original color buffer pitch when called with
                            ///  an unaligned pitch.
    UINT_32 height;         ///< Height in pixels, as above
    UINT_64 cmaskBytes;     ///< Size in bytes of CMask buffer
    UINT_32 baseAlign;      ///< Base alignment
    UINT_32 blockMax;       ///< Cmask block size. Need this to set CB_COLORn_MASK register
    UINT_32 macroWidth;     ///< Macro width in pixels, actually squared cache shape
    UINT_32 macroHeight;    ///< Macro height in pixels
    UINT_64 sliceSize;      ///< Slice size, in bytes.
} ADDR_COMPUTE_CMASK_INFO_OUTPUT;

/**
****************************************************************************************************
*   AddrComputeCmaskInfo
*
*   @brief
*       Compute Cmask pitch, height, base alignment and size in bytes from color buffer
*       info
****************************************************************************************************
*/
ADDR_E_RETURNCODE ADDR_API AddrComputeCmaskInfo(
    ADDR_HANDLE                             hLib,
    const ADDR_COMPUTE_CMASK_INFO_INPUT*    pIn,
    ADDR_COMPUTE_CMASK_INFO_OUTPUT*         pOut);



/**
****************************************************************************************************
*   ADDR_COMPUTE_CMASK_ADDRFROMCOORD_INPUT
*
*   @brief
*       Input structure for AddrComputeCmaskAddrFromCoord
*
****************************************************************************************************
*/
typedef struct _ADDR_COMPUTE_CMASK_ADDRFROMCOORD_INPUT
{
    UINT_32          size;           ///< Size of this structure in bytes
    UINT_32          x;              ///< X coordinate
    UINT_32          y;              ///< Y coordinate
    UINT_64          fmaskAddr;      ///< Fmask addr for tc compatible Cmask
    UINT_32          slice;          ///< Slice index
    UINT_32          pitch;          ///< Pitch in pixels, of color buffer
    UINT_32          height;         ///< Height in pixels, of color buffer
    UINT_32          numSlices;      ///< Number of slices
    UINT_32          bpp;
    BOOL_32          isLinear;       ///< Linear or tiled layout, Only SI can be linear
    ADDR_CMASK_FLAGS flags;          ///< CMASK flags
    ADDR_TILEINFO*   pTileInfo;      ///< Tile info

    INT_32           tileIndex;      ///< Tile index, MUST be -1 if you don't want to use it
                                     ///< while the global useTileIndex is set to 1
    INT_32           macroModeIndex; ///< Index in macro tile mode table if there is one (CI)
                                     ///< README: When tileIndex is not -1, this must be valid
} ADDR_COMPUTE_CMASK_ADDRFROMCOORD_INPUT;

/**
****************************************************************************************************
*   ADDR_COMPUTE_CMASK_ADDRFROMCOORD_OUTPUT
*
*   @brief
*       Output structure for AddrComputeCmaskAddrFromCoord
****************************************************************************************************
*/
typedef struct _ADDR_COMPUTE_CMASK_ADDRFROMCOORD_OUTPUT
{
    UINT_32 size;           ///< Size of this structure in bytes

    UINT_64 addr;           ///< CMASK address in bytes
    UINT_32 bitPosition;    ///< Bit position within addr, 0-7. CMASK is 4 bpp,
                            ///  so the address may be located in bit 0 (0) or 4 (4)
} ADDR_COMPUTE_CMASK_ADDRFROMCOORD_OUTPUT;

/**
****************************************************************************************************
*   AddrComputeCmaskAddrFromCoord
*
*   @brief
*       Compute Cmask address according to coordinates (of MSAA color buffer)
****************************************************************************************************
*/
ADDR_E_RETURNCODE ADDR_API AddrComputeCmaskAddrFromCoord(
    ADDR_HANDLE                                     hLib,
    const ADDR_COMPUTE_CMASK_ADDRFROMCOORD_INPUT*   pIn,
    ADDR_COMPUTE_CMASK_ADDRFROMCOORD_OUTPUT*        pOut);



/**
****************************************************************************************************
*   ADDR_COMPUTE_CMASK_COORDFROMADDR_INPUT
*
*   @brief
*       Input structure for AddrComputeCmaskCoordFromAddr
****************************************************************************************************
*/
typedef struct _ADDR_COMPUTE_CMASK_COORDFROMADDR_INPUT
{
    UINT_32        size;            ///< Size of this structure in bytes

    UINT_64        addr;            ///< CMASK address in bytes
    UINT_32        bitPosition;     ///< Bit position within addr, 0-7. CMASK is 4 bpp,
                                    ///  so the address may be located in bit 0 (0) or 4 (4)
    UINT_32        pitch;           ///< Pitch, in pixels
    UINT_32        height;          ///< Height in pixels
    UINT_32        numSlices;       ///< Number of slices
    BOOL_32        isLinear;        ///< Linear or tiled layout, Only SI can be linear
    ADDR_TILEINFO* pTileInfo;       ///< Tile info

    INT_32         tileIndex;       ///< Tile index, MUST be -1 if you don't want to use it
                                    ///  while the global useTileIndex is set to 1
    INT_32         macroModeIndex;  ///< Index in macro tile mode table if there is one (CI)
                                    ///< README: When tileIndex is not -1, this must be valid
} ADDR_COMPUTE_CMASK_COORDFROMADDR_INPUT;

/**
****************************************************************************************************
*   ADDR_COMPUTE_CMASK_COORDFROMADDR_OUTPUT
*
*   @brief
*       Output structure for AddrComputeCmaskCoordFromAddr
****************************************************************************************************
*/
typedef struct _ADDR_COMPUTE_CMASK_COORDFROMADDR_OUTPUT
{
    UINT_32 size;   ///< Size of this structure in bytes

    UINT_32 x;      ///< X coordinate
    UINT_32 y;      ///< Y coordinate
    UINT_32 slice;  ///< Slice index
} ADDR_COMPUTE_CMASK_COORDFROMADDR_OUTPUT;

/**
****************************************************************************************************
*   AddrComputeCmaskCoordFromAddr
*
*   @brief
*       Compute coordinates within color buffer (1st pixel of a micro tile) according to
*       Cmask address
****************************************************************************************************
*/
ADDR_E_RETURNCODE ADDR_API AddrComputeCmaskCoordFromAddr(
    ADDR_HANDLE                                     hLib,
    const ADDR_COMPUTE_CMASK_COORDFROMADDR_INPUT*   pIn,
    ADDR_COMPUTE_CMASK_COORDFROMADDR_OUTPUT*        pOut);



////////////////////////////////////////////////////////////////////////////////////////////////////
//                                     F-mask functions
////////////////////////////////////////////////////////////////////////////////////////////////////

/**
****************************************************************************************************
*   ADDR_COMPUTE_FMASK_INFO_INPUT
*
*   @brief
*       Input structure for AddrComputeFmaskInfo
****************************************************************************************************
*/
typedef struct _ADDR_COMPUTE_FMASK_INFO_INPUT
{
    UINT_32         size;               ///< Size of this structure in bytes

    AddrTileMode    tileMode;           ///< Tile mode
    UINT_32         pitch;              ///< Surface pitch, in pixels
    UINT_32         height;             ///< Surface height, in pixels
    UINT_32         numSlices;          ///< Number of slice/depth
    UINT_32         numSamples;         ///< Number of samples
    UINT_32         numFrags;           ///< Number of fragments, leave it zero or the same as
                                        ///  number of samples for normal AA; Set it to the
                                        ///  number of fragments for EQAA
    /// r800 and later HWL parameters
    struct
    {
        UINT_32 resolved:   1;          ///< TRUE if the surface is for resolved fmask, only used
                                        ///  by H/W clients. S/W should always set it to FALSE.
        UINT_32 reserved:  31;          ///< Reserved for future use.
    };
    ADDR_TILEINFO*  pTileInfo;          ///< 2D tiling parameters. Clients must give valid data
    INT_32          tileIndex;          ///< Tile index, MUST be -1 if you don't want to use it
                                        ///  while the global useTileIndex is set to 1
} ADDR_COMPUTE_FMASK_INFO_INPUT;

/**
****************************************************************************************************
*   ADDR_COMPUTE_FMASK_INFO_OUTPUT
*
*   @brief
*       Output structure for AddrComputeFmaskInfo
****************************************************************************************************
*/
typedef struct _ADDR_COMPUTE_FMASK_INFO_OUTPUT
{
    UINT_32         size;           ///< Size of this structure in bytes

    UINT_32         pitch;          ///< Pitch of fmask in pixels
    UINT_32         height;         ///< Height of fmask in pixels
    UINT_32         numSlices;      ///< Slices of fmask
    UINT_64         fmaskBytes;     ///< Size of fmask in bytes
    UINT_32         baseAlign;      ///< Base address alignment
    UINT_32         pitchAlign;     ///< Pitch alignment
    UINT_32         heightAlign;    ///< Height alignment
    UINT_32         bpp;            ///< Bits per pixel of FMASK is: number of bit planes
    UINT_32         numSamples;     ///< Number of samples, used for dump, export this since input
                                    ///  may be changed in 9xx and above
    /// r800 and later HWL parameters
    ADDR_TILEINFO*  pTileInfo;      ///< Tile parameters used. Fmask can have different
                                    ///  bank_height from color buffer
    INT_32          tileIndex;      ///< Tile index, MUST be -1 if you don't want to use it
                                    ///  while the global useTileIndex is set to 1
    INT_32          macroModeIndex; ///< Index in macro tile mode table if there is one (CI)
    UINT_64         sliceSize;      ///< Size of slice in bytes
} ADDR_COMPUTE_FMASK_INFO_OUTPUT;

/**
****************************************************************************************************
*   AddrComputeFmaskInfo
*
*   @brief
*       Compute Fmask pitch/height/depth/alignments and size in bytes
****************************************************************************************************
*/
ADDR_E_RETURNCODE ADDR_API AddrComputeFmaskInfo(
    ADDR_HANDLE                             hLib,
    const ADDR_COMPUTE_FMASK_INFO_INPUT*    pIn,
    ADDR_COMPUTE_FMASK_INFO_OUTPUT*         pOut);



/**
****************************************************************************************************
*   ADDR_COMPUTE_FMASK_ADDRFROMCOORD_INPUT
*
*   @brief
*       Input structure for AddrComputeFmaskAddrFromCoord
****************************************************************************************************
*/
typedef struct _ADDR_COMPUTE_FMASK_ADDRFROMCOORD_INPUT
{
    UINT_32         size;               ///< Size of this structure in bytes

    UINT_32         x;                  ///< X coordinate
    UINT_32         y;                  ///< Y coordinate
    UINT_32         slice;              ///< Slice index
    UINT_32         plane;              ///< Plane number
    UINT_32         sample;             ///< Sample index (fragment index for EQAA)

    UINT_32         pitch;              ///< Surface pitch, in pixels
    UINT_32         height;             ///< Surface height, in pixels
    UINT_32         numSamples;         ///< Number of samples
    UINT_32         numFrags;           ///< Number of fragments, leave it zero or the same as
                                        ///  number of samples for normal AA; Set it to the
                                        ///  number of fragments for EQAA

    AddrTileMode    tileMode;           ///< Tile mode
    union
    {
        struct
        {
            UINT_32  bankSwizzle;       ///< Bank swizzle
            UINT_32  pipeSwizzle;       ///< Pipe swizzle
        };
        UINT_32     tileSwizzle;        ///< Combined swizzle, if useCombinedSwizzle is TRUE
    };

    /// r800 and later HWL parameters
    struct
    {
        UINT_32 resolved:   1;          ///< TRUE if this is a resolved fmask, used by H/W clients
        UINT_32 ignoreSE:   1;          ///< TRUE if shader engines are ignored.
        UINT_32 reserved:  30;          ///< Reserved for future use.
    };
    ADDR_TILEINFO*  pTileInfo;          ///< 2D tiling parameters. Client must provide all data

} ADDR_COMPUTE_FMASK_ADDRFROMCOORD_INPUT;

/**
****************************************************************************************************
*   ADDR_COMPUTE_FMASK_ADDRFROMCOORD_OUTPUT
*
*   @brief
*       Output structure for AddrComputeFmaskAddrFromCoord
****************************************************************************************************
*/
typedef struct _ADDR_COMPUTE_FMASK_ADDRFROMCOORD_OUTPUT
{
    UINT_32 size;           ///< Size of this structure in bytes

    UINT_64 addr;           ///< Fmask address
    UINT_32 bitPosition;    ///< Bit position within fmaskAddr, 0-7.
} ADDR_COMPUTE_FMASK_ADDRFROMCOORD_OUTPUT;

/**
****************************************************************************************************
*   AddrComputeFmaskAddrFromCoord
*
*   @brief
*       Compute Fmask address according to coordinates (x,y,slice,sample,plane)
****************************************************************************************************
*/
ADDR_E_RETURNCODE ADDR_API AddrComputeFmaskAddrFromCoord(
    ADDR_HANDLE                                     hLib,
    const ADDR_COMPUTE_FMASK_ADDRFROMCOORD_INPUT*   pIn,
    ADDR_COMPUTE_FMASK_ADDRFROMCOORD_OUTPUT*        pOut);



/**
****************************************************************************************************
*   ADDR_COMPUTE_FMASK_COORDFROMADDR_INPUT
*
*   @brief
*       Input structure for AddrComputeFmaskCoordFromAddr
****************************************************************************************************
*/
typedef struct _ADDR_COMPUTE_FMASK_COORDFROMADDR_INPUT
{
    UINT_32         size;               ///< Size of this structure in bytes

    UINT_64         addr;               ///< Address
    UINT_32         bitPosition;        ///< Bit position within addr, 0-7.

    UINT_32         pitch;              ///< Pitch, in pixels
    UINT_32         height;             ///< Height in pixels
    UINT_32         numSamples;         ///< Number of samples
    UINT_32         numFrags;           ///< Number of fragments
    AddrTileMode    tileMode;           ///< Tile mode
    union
    {
        struct
        {
            UINT_32  bankSwizzle;       ///< Bank swizzle
            UINT_32  pipeSwizzle;       ///< Pipe swizzle
        };
        UINT_32     tileSwizzle;        ///< Combined swizzle, if useCombinedSwizzle is TRUE
    };

    /// r800 and later HWL parameters
    struct
    {
        UINT_32 resolved:   1;          ///< TRUE if this is a resolved fmask, used by HW components
        UINT_32 ignoreSE:   1;          ///< TRUE if shader engines are ignored.
        UINT_32 reserved:  30;          ///< Reserved for future use.
    };
    ADDR_TILEINFO*  pTileInfo;          ///< 2D tile parameters. Client must provide all data

} ADDR_COMPUTE_FMASK_COORDFROMADDR_INPUT;

/**
****************************************************************************************************
*   ADDR_COMPUTE_FMASK_COORDFROMADDR_OUTPUT
*
*   @brief
*       Output structure for AddrComputeFmaskCoordFromAddr
****************************************************************************************************
*/
typedef struct _ADDR_COMPUTE_FMASK_COORDFROMADDR_OUTPUT
{
    UINT_32 size;       ///< Size of this structure in bytes

    UINT_32 x;          ///< X coordinate
    UINT_32 y;          ///< Y coordinate
    UINT_32 slice;      ///< Slice index
    UINT_32 plane;      ///< Plane number
    UINT_32 sample;     ///< Sample index (fragment index for EQAA)
} ADDR_COMPUTE_FMASK_COORDFROMADDR_OUTPUT;

/**
****************************************************************************************************
*   AddrComputeFmaskCoordFromAddr
*
*   @brief
*       Compute FMASK coordinate from an given address
****************************************************************************************************
*/
ADDR_E_RETURNCODE ADDR_API AddrComputeFmaskCoordFromAddr(
    ADDR_HANDLE                                     hLib,
    const ADDR_COMPUTE_FMASK_COORDFROMADDR_INPUT*   pIn,
    ADDR_COMPUTE_FMASK_COORDFROMADDR_OUTPUT*        pOut);



////////////////////////////////////////////////////////////////////////////////////////////////////
//                          Element/utility functions
////////////////////////////////////////////////////////////////////////////////////////////////////

/**
****************************************************************************************************
*   AddrGetVersion
*
*   @brief
*       Get AddrLib version number
****************************************************************************************************
*/
UINT_32 ADDR_API AddrGetVersion(ADDR_HANDLE hLib);

/**
****************************************************************************************************
*   AddrUseTileIndex
*
*   @brief
*       Return TRUE if tileIndex is enabled in this address library
****************************************************************************************************
*/
BOOL_32 ADDR_API AddrUseTileIndex(ADDR_HANDLE hLib);

/**
****************************************************************************************************
*   AddrUseCombinedSwizzle
*
*   @brief
*       Return TRUE if combined swizzle is enabled in this address library
****************************************************************************************************
*/
BOOL_32 ADDR_API AddrUseCombinedSwizzle(ADDR_HANDLE hLib);

/**
****************************************************************************************************
*   ADDR_EXTRACT_BANKPIPE_SWIZZLE_INPUT
*
*   @brief
*       Input structure of AddrExtractBankPipeSwizzle
****************************************************************************************************
*/
typedef struct _ADDR_EXTRACT_BANKPIPE_SWIZZLE_INPUT
{
    UINT_32         size;           ///< Size of this structure in bytes

    UINT_32         base256b;       ///< Base256b value

    /// r800 and later HWL parameters
    ADDR_TILEINFO*  pTileInfo;      ///< 2D tile parameters. Client must provide all data

    INT_32          tileIndex;      ///< Tile index, MUST be -1 if you don't want to use it
                                    ///  while the global useTileIndex is set to 1
    INT_32          macroModeIndex; ///< Index in macro tile mode table if there is one (CI)
                                    ///< README: When tileIndex is not -1, this must be valid
} ADDR_EXTRACT_BANKPIPE_SWIZZLE_INPUT;

/**
****************************************************************************************************
*   ADDR_EXTRACT_BANKPIPE_SWIZZLE_OUTPUT
*
*   @brief
*       Output structure of AddrExtractBankPipeSwizzle
****************************************************************************************************
*/
typedef struct _ADDR_EXTRACT_BANKPIPE_SWIZZLE_OUTPUT
{
    UINT_32 size;           ///< Size of this structure in bytes

    UINT_32 bankSwizzle;    ///< Bank swizzle
    UINT_32 pipeSwizzle;    ///< Pipe swizzle
} ADDR_EXTRACT_BANKPIPE_SWIZZLE_OUTPUT;

/**
****************************************************************************************************
*   AddrExtractBankPipeSwizzle
*
*   @brief
*       Extract Bank and Pipe swizzle from base256b
*   @return
*       ADDR_OK if no error
****************************************************************************************************
*/
ADDR_E_RETURNCODE ADDR_API AddrExtractBankPipeSwizzle(
    ADDR_HANDLE                                 hLib,
    const ADDR_EXTRACT_BANKPIPE_SWIZZLE_INPUT*  pIn,
    ADDR_EXTRACT_BANKPIPE_SWIZZLE_OUTPUT*       pOut);


/**
****************************************************************************************************
*   ADDR_COMBINE_BANKPIPE_SWIZZLE_INPUT
*
*   @brief
*       Input structure of AddrCombineBankPipeSwizzle
****************************************************************************************************
*/
typedef struct _ADDR_COMBINE_BANKPIPE_SWIZZLE_INPUT
{
    UINT_32         size;           ///< Size of this structure in bytes

    UINT_32         bankSwizzle;    ///< Bank swizzle
    UINT_32         pipeSwizzle;    ///< Pipe swizzle
    UINT_64         baseAddr;       ///< Base address (leave it zero for driver clients)

    /// r800 and later HWL parameters
    ADDR_TILEINFO*  pTileInfo;      ///< 2D tile parameters. Client must provide all data

    INT_32          tileIndex;      ///< Tile index, MUST be -1 if you don't want to use it
                                    ///  while the global useTileIndex is set to 1
    INT_32          macroModeIndex; ///< Index in macro tile mode table if there is one (CI)
                                    ///< README: When tileIndex is not -1, this must be valid
} ADDR_COMBINE_BANKPIPE_SWIZZLE_INPUT;

/**
****************************************************************************************************
*   ADDR_COMBINE_BANKPIPE_SWIZZLE_OUTPUT
*
*   @brief
*       Output structure of AddrCombineBankPipeSwizzle
****************************************************************************************************
*/
typedef struct _ADDR_COMBINE_BANKPIPE_SWIZZLE_OUTPUT
{
    UINT_32 size;           ///< Size of this structure in bytes

    UINT_32 tileSwizzle;    ///< Combined swizzle
} ADDR_COMBINE_BANKPIPE_SWIZZLE_OUTPUT;

/**
****************************************************************************************************
*   AddrCombineBankPipeSwizzle
*
*   @brief
*       Combine Bank and Pipe swizzle
*   @return
*       ADDR_OK if no error
*   @note
*       baseAddr here is full MCAddress instead of base256b
****************************************************************************************************
*/
ADDR_E_RETURNCODE ADDR_API AddrCombineBankPipeSwizzle(
    ADDR_HANDLE                                 hLib,
    const ADDR_COMBINE_BANKPIPE_SWIZZLE_INPUT*  pIn,
    ADDR_COMBINE_BANKPIPE_SWIZZLE_OUTPUT*       pOut);



/**
****************************************************************************************************
*   ADDR_COMPUTE_SLICESWIZZLE_INPUT
*
*   @brief
*       Input structure of AddrComputeSliceSwizzle
****************************************************************************************************
*/
typedef struct _ADDR_COMPUTE_SLICESWIZZLE_INPUT
{
    UINT_32         size;               ///< Size of this structure in bytes

    AddrTileMode    tileMode;           ///< Tile Mode
    UINT_32         baseSwizzle;        ///< Base tile swizzle
    UINT_32         slice;              ///< Slice index
    UINT_64         baseAddr;           ///< Base address, driver should leave it 0 in most cases

    /// r800 and later HWL parameters
    ADDR_TILEINFO*  pTileInfo;          ///< 2D tile parameters. Actually banks needed here!

    INT_32          tileIndex;          ///< Tile index, MUST be -1 if you don't want to use it
                                        ///  while the global useTileIndex is set to 1
    INT_32          macroModeIndex;     ///< Index in macro tile mode table if there is one (CI)
                                        ///< README: When tileIndex is not -1, this must be valid
} ADDR_COMPUTE_SLICESWIZZLE_INPUT;



/**
****************************************************************************************************
*   ADDR_COMPUTE_SLICESWIZZLE_OUTPUT
*
*   @brief
*       Output structure of AddrComputeSliceSwizzle
****************************************************************************************************
*/
typedef struct _ADDR_COMPUTE_SLICESWIZZLE_OUTPUT
{
    UINT_32  size;           ///< Size of this structure in bytes

    UINT_32  tileSwizzle;    ///< Recalculated tileSwizzle value
} ADDR_COMPUTE_SLICESWIZZLE_OUTPUT;

/**
****************************************************************************************************
*   AddrComputeSliceSwizzle
*
*   @brief
*       Extract Bank and Pipe swizzle from base256b
*   @return
*       ADDR_OK if no error
****************************************************************************************************
*/
ADDR_E_RETURNCODE ADDR_API AddrComputeSliceSwizzle(
    ADDR_HANDLE                             hLib,
    const ADDR_COMPUTE_SLICESWIZZLE_INPUT*  pIn,
    ADDR_COMPUTE_SLICESWIZZLE_OUTPUT*       pOut);


/**
****************************************************************************************************
*   AddrSwizzleGenOption
*
*   @brief
*       Which swizzle generating options: legacy or linear
****************************************************************************************************
*/
typedef enum _AddrSwizzleGenOption
{
    ADDR_SWIZZLE_GEN_DEFAULT    = 0,    ///< As is in client driver implemention for swizzle
    ADDR_SWIZZLE_GEN_LINEAR     = 1,    ///< Using a linear increment of swizzle
} AddrSwizzleGenOption;

/**
****************************************************************************************************
*   AddrBlockType
*
*   @brief
*       Macro define resource block type
****************************************************************************************************
*/
typedef enum
{
    AddrBlockLinear = 0, // Resource uses linear swizzle mode
    AddrBlockMicro = 1, // Resource uses 256B block
    AddrBlockThin4KB = 2, // Resource uses thin 4KB block
    AddrBlockThick4KB = 3, // Resource uses thick 4KB block
    AddrBlockThin64KB = 4, // Resource uses thin 64KB block
    AddrBlockThick64KB = 5, // Resource uses thick 64KB block
    AddrBlockThinVar = 6, // Resource uses thin var block
    AddrBlockThickVar = 7, // Resource uses thick var block
    AddrBlockMaxTiledType,

    AddrBlockThin256KB = AddrBlockThinVar,
    AddrBlockThick256KB = AddrBlockThickVar,
} AddrBlockType;

/**
****************************************************************************************************
*   AddrSwizzleOption
*
*   @brief
*       Controls how swizzle is generated
****************************************************************************************************
*/
typedef union _ADDR_SWIZZLE_OPTION
{
    struct
    {
        UINT_32 genOption       : 1;    ///< The way swizzle is generated, see AddrSwizzleGenOption
        UINT_32 reduceBankBit   : 1;    ///< TRUE if we need reduce swizzle bits
        UINT_32 reserved        :30;    ///< Reserved bits
    };

    UINT_32 value;

} ADDR_SWIZZLE_OPTION;

/**
****************************************************************************************************
*   ADDR_COMPUTE_BASE_SWIZZLE_INPUT
*
*   @brief
*       Input structure of AddrComputeBaseSwizzle
****************************************************************************************************
*/
typedef struct _ADDR_COMPUTE_BASE_SWIZZLE_INPUT
{
    UINT_32             size;           ///< Size of this structure in bytes

    ADDR_SWIZZLE_OPTION option;         ///< Swizzle option
    UINT_32             surfIndex;      ///< Index of this surface type
    AddrTileMode        tileMode;       ///< Tile Mode

    /// r800 and later HWL parameters
    ADDR_TILEINFO*      pTileInfo;      ///< 2D tile parameters. Actually banks needed here!

    INT_32              tileIndex;      ///< Tile index, MUST be -1 if you don't want to use it
                                        ///  while the global useTileIndex is set to 1
    INT_32              macroModeIndex; ///< Index in macro tile mode table if there is one (CI)
                                        ///< README: When tileIndex is not -1, this must be valid
} ADDR_COMPUTE_BASE_SWIZZLE_INPUT;

/**
****************************************************************************************************
*   ADDR_COMPUTE_BASE_SWIZZLE_OUTPUT
*
*   @brief
*       Output structure of AddrComputeBaseSwizzle
****************************************************************************************************
*/
typedef struct _ADDR_COMPUTE_BASE_SWIZZLE_OUTPUT
{
    UINT_32 size;           ///< Size of this structure in bytes

    UINT_32 tileSwizzle;    ///< Combined swizzle
} ADDR_COMPUTE_BASE_SWIZZLE_OUTPUT;

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
    ADDR_COMPUTE_BASE_SWIZZLE_OUTPUT*       pOut);



/**
****************************************************************************************************
*   ELEM_GETEXPORTNORM_INPUT
*
*   @brief
*       Input structure for ElemGetExportNorm
*
****************************************************************************************************
*/
typedef struct _ELEM_GETEXPORTNORM_INPUT
{
    UINT_32             size;       ///< Size of this structure in bytes

    AddrColorFormat     format;     ///< Color buffer format; Client should use ColorFormat
    AddrSurfaceNumber   num;        ///< Surface number type; Client should use NumberType
    AddrSurfaceSwap     swap;       ///< Surface swap byte swap; Client should use SurfaceSwap
    UINT_32             numSamples; ///< Number of samples
} ELEM_GETEXPORTNORM_INPUT;

/**
****************************************************************************************************
*  ElemGetExportNorm
*
*   @brief
*       Helper function to check one format can be EXPORT_NUM, which is a register
*       CB_COLOR_INFO.SURFACE_FORMAT. FP16 can be reported as EXPORT_NORM for rv770 in r600
*       family
*   @note
*       The implementation is only for r600.
*       00 - EXPORT_FULL: PS exports are 4 pixels with 4 components with 32-bits-per-component. (two
*       clocks per export)
*       01 - EXPORT_NORM: PS exports are 4 pixels with 4 components with 16-bits-per-component. (one
*       clock per export)
*
****************************************************************************************************
*/
BOOL_32 ADDR_API ElemGetExportNorm(
    ADDR_HANDLE                     hLib,
    const ELEM_GETEXPORTNORM_INPUT* pIn);



/**
****************************************************************************************************
*   ELEM_FLT32TODEPTHPIXEL_INPUT
*
*   @brief
*       Input structure for addrFlt32ToDepthPixel
*
****************************************************************************************************
*/
typedef struct _ELEM_FLT32TODEPTHPIXEL_INPUT
{
    UINT_32         size;           ///< Size of this structure in bytes

    AddrDepthFormat format;         ///< Depth buffer format
    ADDR_FLT_32     comps[2];       ///< Component values (Z/stencil)
} ELEM_FLT32TODEPTHPIXEL_INPUT;

/**
****************************************************************************************************
*   ELEM_FLT32TODEPTHPIXEL_INPUT
*
*   @brief
*       Output structure for ElemFlt32ToDepthPixel
*
****************************************************************************************************
*/
typedef struct _ELEM_FLT32TODEPTHPIXEL_OUTPUT
{
    UINT_32 size;           ///< Size of this structure in bytes

    UINT_8* pPixel;         ///< Real depth value. Same data type as depth buffer.
                            ///  Client must provide enough storage for this type.
    UINT_32 depthBase;      ///< Tile base in bits for depth bits
    UINT_32 stencilBase;    ///< Tile base in bits for stencil bits
    UINT_32 depthBits;      ///< Bits for depth
    UINT_32 stencilBits;    ///< Bits for stencil
} ELEM_FLT32TODEPTHPIXEL_OUTPUT;

/**
****************************************************************************************************
*   ElemFlt32ToDepthPixel
*
*   @brief
*       Convert a FLT_32 value to a depth/stencil pixel value
*
*   @return
*       Return code
*
****************************************************************************************************
*/
ADDR_E_RETURNCODE ADDR_API ElemFlt32ToDepthPixel(
    ADDR_HANDLE                         hLib,
    const ELEM_FLT32TODEPTHPIXEL_INPUT* pIn,
    ELEM_FLT32TODEPTHPIXEL_OUTPUT*      pOut);



/**
****************************************************************************************************
*   ELEM_FLT32TOCOLORPIXEL_INPUT
*
*   @brief
*       Input structure for addrFlt32ToColorPixel
*
****************************************************************************************************
*/
typedef struct _ELEM_FLT32TOCOLORPIXEL_INPUT
{
    UINT_32            size;           ///< Size of this structure in bytes

    AddrColorFormat    format;         ///< Color buffer format
    AddrSurfaceNumber  surfNum;        ///< Surface number
    AddrSurfaceSwap    surfSwap;       ///< Surface swap
    ADDR_FLT_32        comps[4];       ///< Component values (r/g/b/a)
} ELEM_FLT32TOCOLORPIXEL_INPUT;

/**
****************************************************************************************************
*   ELEM_FLT32TOCOLORPIXEL_INPUT
*
*   @brief
*       Output structure for ElemFlt32ToColorPixel
*
****************************************************************************************************
*/
typedef struct _ELEM_FLT32TOCOLORPIXEL_OUTPUT
{
    UINT_32 size;       ///< Size of this structure in bytes

    UINT_8* pPixel;     ///< Real color value. Same data type as color buffer.
                        ///  Client must provide enough storage for this type.
} ELEM_FLT32TOCOLORPIXEL_OUTPUT;

/**
****************************************************************************************************
*   ElemFlt32ToColorPixel
*
*   @brief
*       Convert a FLT_32 value to a red/green/blue/alpha pixel value
*
*   @return
*       Return code
*
****************************************************************************************************
*/
ADDR_E_RETURNCODE ADDR_API ElemFlt32ToColorPixel(
    ADDR_HANDLE                         hLib,
    const ELEM_FLT32TOCOLORPIXEL_INPUT* pIn,
    ELEM_FLT32TOCOLORPIXEL_OUTPUT*      pOut);

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
    AddrFormat  format);

/**
****************************************************************************************************
*   ADDR_CONVERT_TILEINFOTOHW_INPUT
*
*   @brief
*       Input structure for AddrConvertTileInfoToHW
*   @note
*       When reverse is TRUE, indices are igonred
****************************************************************************************************
*/
typedef struct _ADDR_CONVERT_TILEINFOTOHW_INPUT
{
    UINT_32         size;               ///< Size of this structure in bytes
    BOOL_32         reverse;            ///< Convert control flag.
                                        ///  FALSE: convert from real value to HW value;
                                        ///  TRUE: convert from HW value to real value.

    /// r800 and later HWL parameters
    ADDR_TILEINFO*  pTileInfo;          ///< Tile parameters with real value

    INT_32          tileIndex;          ///< Tile index, MUST be -1 if you don't want to use it
                                        ///  while the global useTileIndex is set to 1
    INT_32          macroModeIndex;     ///< Index in macro tile mode table if there is one (CI)
                                        ///< README: When tileIndex is not -1, this must be valid
    UINT_32         bpp;                ///< Bits per pixel
} ADDR_CONVERT_TILEINFOTOHW_INPUT;

/**
****************************************************************************************************
*   ADDR_CONVERT_TILEINFOTOHW_OUTPUT
*
*   @brief
*       Output structure for AddrConvertTileInfoToHW
****************************************************************************************************
*/
typedef struct _ADDR_CONVERT_TILEINFOTOHW_OUTPUT
{
    UINT_32             size;               ///< Size of this structure in bytes

    /// r800 and later HWL parameters
    ADDR_TILEINFO*      pTileInfo;          ///< Tile parameters with hardware register value

} ADDR_CONVERT_TILEINFOTOHW_OUTPUT;

/**
****************************************************************************************************
*   AddrConvertTileInfoToHW
*
*   @brief
*       Convert tile info from real value to hardware register value
****************************************************************************************************
*/
ADDR_E_RETURNCODE ADDR_API AddrConvertTileInfoToHW(
    ADDR_HANDLE                             hLib,
    const ADDR_CONVERT_TILEINFOTOHW_INPUT*  pIn,
    ADDR_CONVERT_TILEINFOTOHW_OUTPUT*       pOut);



/**
****************************************************************************************************
*   ADDR_CONVERT_TILEINDEX_INPUT
*
*   @brief
*       Input structure for AddrConvertTileIndex
****************************************************************************************************
*/
typedef struct _ADDR_CONVERT_TILEINDEX_INPUT
{
    UINT_32         size;               ///< Size of this structure in bytes

    INT_32          tileIndex;          ///< Tile index
    INT_32          macroModeIndex;     ///< Index in macro tile mode table if there is one (CI)
    UINT_32         bpp;                ///< Bits per pixel
    BOOL_32         tileInfoHw;         ///< Set to TRUE if client wants HW enum, otherwise actual
} ADDR_CONVERT_TILEINDEX_INPUT;

/**
****************************************************************************************************
*   ADDR_CONVERT_TILEINDEX_OUTPUT
*
*   @brief
*       Output structure for AddrConvertTileIndex
****************************************************************************************************
*/
typedef struct _ADDR_CONVERT_TILEINDEX_OUTPUT
{
    UINT_32             size;           ///< Size of this structure in bytes

    AddrTileMode        tileMode;       ///< Tile mode
    AddrTileType        tileType;       ///< Tile type
    ADDR_TILEINFO*      pTileInfo;      ///< Tile info

} ADDR_CONVERT_TILEINDEX_OUTPUT;

/**
****************************************************************************************************
*   AddrConvertTileIndex
*
*   @brief
*       Convert tile index to tile mode/type/info
****************************************************************************************************
*/
ADDR_E_RETURNCODE ADDR_API AddrConvertTileIndex(
    ADDR_HANDLE                         hLib,
    const ADDR_CONVERT_TILEINDEX_INPUT* pIn,
    ADDR_CONVERT_TILEINDEX_OUTPUT*      pOut);

/**
****************************************************************************************************
*   ADDR_GET_MACROMODEINDEX_INPUT
*
*   @brief
*       Input structure for AddrGetMacroModeIndex
****************************************************************************************************
*/
typedef struct _ADDR_GET_MACROMODEINDEX_INPUT
{
    UINT_32             size;               ///< Size of this structure in bytes
    ADDR_SURFACE_FLAGS  flags;              ///< Surface flag
    INT_32              tileIndex;          ///< Tile index
    UINT_32             bpp;                ///< Bits per pixel
    UINT_32             numFrags;           ///< Number of color fragments
} ADDR_GET_MACROMODEINDEX_INPUT;

/**
****************************************************************************************************
*   ADDR_GET_MACROMODEINDEX_OUTPUT
*
*   @brief
*       Output structure for AddrGetMacroModeIndex
****************************************************************************************************
*/
typedef struct _ADDR_GET_MACROMODEINDEX_OUTPUT
{
    UINT_32             size;            ///< Size of this structure in bytes
    INT_32              macroModeIndex;  ///< Index in macro tile mode table if there is one (CI)
} ADDR_GET_MACROMODEINDEX_OUTPUT;

/**
****************************************************************************************************
*   AddrGetMacroModeIndex
*
*   @brief
*       Get macro mode index based on input parameters
****************************************************************************************************
*/
ADDR_E_RETURNCODE ADDR_API AddrGetMacroModeIndex(
    ADDR_HANDLE                          hLib,
    const ADDR_GET_MACROMODEINDEX_INPUT* pIn,
    ADDR_GET_MACROMODEINDEX_OUTPUT*      pOut);

/**
****************************************************************************************************
*   ADDR_CONVERT_TILEINDEX1_INPUT
*
*   @brief
*       Input structure for AddrConvertTileIndex1 (without macro mode index)
****************************************************************************************************
*/
typedef struct _ADDR_CONVERT_TILEINDEX1_INPUT
{
    UINT_32         size;               ///< Size of this structure in bytes

    INT_32          tileIndex;          ///< Tile index
    UINT_32         bpp;                ///< Bits per pixel
    UINT_32         numSamples;         ///< Number of samples
    BOOL_32         tileInfoHw;         ///< Set to TRUE if client wants HW enum, otherwise actual
} ADDR_CONVERT_TILEINDEX1_INPUT;

/**
****************************************************************************************************
*   AddrConvertTileIndex1
*
*   @brief
*       Convert tile index to tile mode/type/info
****************************************************************************************************
*/
ADDR_E_RETURNCODE ADDR_API AddrConvertTileIndex1(
    ADDR_HANDLE                             hLib,
    const ADDR_CONVERT_TILEINDEX1_INPUT*    pIn,
    ADDR_CONVERT_TILEINDEX_OUTPUT*          pOut);



/**
****************************************************************************************************
*   ADDR_GET_TILEINDEX_INPUT
*
*   @brief
*       Input structure for AddrGetTileIndex
****************************************************************************************************
*/
typedef struct _ADDR_GET_TILEINDEX_INPUT
{
    UINT_32         size;           ///< Size of this structure in bytes

    AddrTileMode    tileMode;       ///< Tile mode
    AddrTileType    tileType;       ///< Tile-type: disp/non-disp/...
    ADDR_TILEINFO*  pTileInfo;      ///< Pointer to tile-info structure, can be NULL for linear/1D
} ADDR_GET_TILEINDEX_INPUT;

/**
****************************************************************************************************
*   ADDR_GET_TILEINDEX_OUTPUT
*
*   @brief
*       Output structure for AddrGetTileIndex
****************************************************************************************************
*/
typedef struct _ADDR_GET_TILEINDEX_OUTPUT
{
    UINT_32         size;           ///< Size of this structure in bytes

    INT_32          index;          ///< index in table
} ADDR_GET_TILEINDEX_OUTPUT;

/**
****************************************************************************************************
*   AddrGetTileIndex
*
*   @brief
*       Get the tiling mode index in table
****************************************************************************************************
*/
ADDR_E_RETURNCODE ADDR_API AddrGetTileIndex(
    ADDR_HANDLE                     hLib,
    const ADDR_GET_TILEINDEX_INPUT* pIn,
    ADDR_GET_TILEINDEX_OUTPUT*      pOut);



/**
****************************************************************************************************
*   ADDR_PRT_INFO_INPUT
*
*   @brief
*       Input structure for AddrComputePrtInfo
****************************************************************************************************
*/
typedef struct _ADDR_PRT_INFO_INPUT
{
    AddrFormat          format;        ///< Surface format
    UINT_32             baseMipWidth;  ///< Base mipmap width
    UINT_32             baseMipHeight; ///< Base mipmap height
    UINT_32             baseMipDepth;  ///< Base mipmap depth
    UINT_32             numFrags;      ///< Number of fragments,
} ADDR_PRT_INFO_INPUT;

/**
****************************************************************************************************
*   ADDR_PRT_INFO_OUTPUT
*
*   @brief
*       Input structure for AddrComputePrtInfo
****************************************************************************************************
*/
typedef struct _ADDR_PRT_INFO_OUTPUT
{
    UINT_32             prtTileWidth;
    UINT_32             prtTileHeight;
} ADDR_PRT_INFO_OUTPUT;

/**
****************************************************************************************************
*   AddrComputePrtInfo
*
*   @brief
*       Compute prt surface related information
****************************************************************************************************
*/
ADDR_E_RETURNCODE ADDR_API AddrComputePrtInfo(
    ADDR_HANDLE                 hLib,
    const ADDR_PRT_INFO_INPUT*  pIn,
    ADDR_PRT_INFO_OUTPUT*       pOut);



////////////////////////////////////////////////////////////////////////////////////////////////////
//                                     DCC key functions
////////////////////////////////////////////////////////////////////////////////////////////////////

/**
****************************************************************************************************
*   _ADDR_COMPUTE_DCCINFO_INPUT
*
*   @brief
*       Input structure of AddrComputeDccInfo
****************************************************************************************************
*/
typedef struct _ADDR_COMPUTE_DCCINFO_INPUT
{
    UINT_32             size;            ///< Size of this structure in bytes
    UINT_32             bpp;             ///< BitPP of color surface
    UINT_32             numSamples;      ///< Sample number of color surface
    UINT_64             colorSurfSize;   ///< Size of color surface to which dcc key is bound
    AddrTileMode        tileMode;        ///< Tile mode of color surface
    ADDR_TILEINFO       tileInfo;        ///< Tile info of color surface
    UINT_32             tileSwizzle;     ///< Tile swizzle
    INT_32              tileIndex;       ///< Tile index of color surface,
                                         ///< MUST be -1 if you don't want to use it
                                         ///< while the global useTileIndex is set to 1
    INT_32              macroModeIndex;  ///< Index in macro tile mode table if there is one (CI)
                                         ///< README: When tileIndex is not -1, this must be valid
} ADDR_COMPUTE_DCCINFO_INPUT;

/**
****************************************************************************************************
*   ADDR_COMPUTE_DCCINFO_OUTPUT
*
*   @brief
*       Output structure of AddrComputeDccInfo
****************************************************************************************************
*/
typedef struct _ADDR_COMPUTE_DCCINFO_OUTPUT
{
    UINT_32 size;                 ///< Size of this structure in bytes
    UINT_32 dccRamBaseAlign;      ///< Base alignment of dcc key
    UINT_64 dccRamSize;           ///< Size of dcc key
    UINT_64 dccFastClearSize;     ///< Size of dcc key portion that can be fast cleared
    BOOL_32 subLvlCompressible;   ///< Whether sub resource is compressiable
    BOOL_32 dccRamSizeAligned;    ///< Whether the dcc key size is aligned
} ADDR_COMPUTE_DCCINFO_OUTPUT;

/**
****************************************************************************************************
*   AddrComputeDccInfo
*
*   @brief
*       Compute DCC key size, base alignment
*       info
****************************************************************************************************
*/
ADDR_E_RETURNCODE ADDR_API AddrComputeDccInfo(
    ADDR_HANDLE                             hLib,
    const ADDR_COMPUTE_DCCINFO_INPUT*       pIn,
    ADDR_COMPUTE_DCCINFO_OUTPUT*            pOut);



/**
****************************************************************************************************
*   ADDR_GET_MAX_ALIGNMENTS_OUTPUT
*
*   @brief
*       Output structure of AddrGetMaxAlignments
****************************************************************************************************
*/
typedef struct ADDR_GET_MAX_ALINGMENTS_OUTPUT
{
    UINT_32 size;                   ///< Size of this structure in bytes
    UINT_32 baseAlign;              ///< Maximum base alignment in bytes
} ADDR_GET_MAX_ALIGNMENTS_OUTPUT;

/**
****************************************************************************************************
*   AddrGetMaxAlignments
*
*   @brief
*       Gets maximnum alignments
****************************************************************************************************
*/
ADDR_E_RETURNCODE ADDR_API AddrGetMaxAlignments(
    ADDR_HANDLE                     hLib,
    ADDR_GET_MAX_ALIGNMENTS_OUTPUT* pOut);

/**
****************************************************************************************************
*   AddrGetMaxMetaAlignments
*
*   @brief
*       Gets maximnum alignments for metadata
****************************************************************************************************
*/
ADDR_E_RETURNCODE ADDR_API AddrGetMaxMetaAlignments(
    ADDR_HANDLE                     hLib,
    ADDR_GET_MAX_ALIGNMENTS_OUTPUT* pOut);

/**
****************************************************************************************************
*                                Address library interface version 2
*                                    available from Gfx9 hardware
****************************************************************************************************
*     Addr2ComputeSurfaceInfo()
*     Addr2ComputeSurfaceAddrFromCoord()
*     Addr2ComputeSurfaceCoordFromAddr()

*     Addr2ComputeHtileInfo()
*     Addr2ComputeHtileAddrFromCoord()
*     Addr2ComputeHtileCoordFromAddr()
*
*     Addr2ComputeCmaskInfo()
*     Addr2ComputeCmaskAddrFromCoord()
*     Addr2ComputeCmaskCoordFromAddr()
*
*     Addr2ComputeFmaskInfo()
*     Addr2ComputeFmaskAddrFromCoord()
*     Addr2ComputeFmaskCoordFromAddr()
*
*     Addr2ComputeDccInfo()
*
**/


////////////////////////////////////////////////////////////////////////////////////////////////////
//                                    Surface functions for Gfx9
////////////////////////////////////////////////////////////////////////////////////////////////////

/**
****************************************************************************************************
*   ADDR2_SURFACE_FLAGS
*
*   @brief
*       Surface flags
****************************************************************************************************
*/
typedef union _ADDR2_SURFACE_FLAGS
{
    struct
    {
        UINT_32 color             :  1; ///< This resource is a color buffer, can be used with RTV
        UINT_32 depth             :  1; ///< Thie resource is a depth buffer, can be used with DSV
        UINT_32 stencil           :  1; ///< Thie resource is a stencil buffer, can be used with DSV
        UINT_32 fmask             :  1; ///< This is an fmask surface
        UINT_32 overlay           :  1; ///< This is an overlay surface
        UINT_32 display           :  1; ///< This resource is displable, can be used with DRV
        UINT_32 prt               :  1; ///< This is a partially resident texture
        UINT_32 qbStereo          :  1; ///< This is a quad buffer stereo surface
        UINT_32 interleaved       :  1; ///< Special flag for interleaved YUV surface padding
        UINT_32 texture           :  1; ///< This resource can be used with SRV
        UINT_32 unordered         :  1; ///< This resource can be used with UAV
        UINT_32 rotated           :  1; ///< This resource is rotated and displable
        UINT_32 needEquation      :  1; ///< This resource needs equation to be generated if possible
        UINT_32 opt4space         :  1; ///< This resource should be optimized for space
        UINT_32 minimizeAlign     :  1; ///< This resource should use minimum alignment
        UINT_32 noMetadata        :  1; ///< This resource has no metadata
        UINT_32 metaRbUnaligned   :  1; ///< This resource has rb unaligned metadata
        UINT_32 metaPipeUnaligned :  1; ///< This resource has pipe unaligned metadata
        UINT_32 view3dAs2dArray   :  1; ///< This resource is a 3D resource viewed as 2D array
        UINT_32 allowExtEquation  :  1; ///< If unset, only legacy DX eqs are allowed (2 XORs)
        UINT_32 reserved          : 12; ///< Reserved bits
    };

    UINT_32 value;
} ADDR2_SURFACE_FLAGS;

/**
****************************************************************************************************
*   ADDR2_COMPUTE_SURFACE_INFO_INPUT
*
*   @brief
*       Input structure for Addr2ComputeSurfaceInfo
****************************************************************************************************
*/
typedef struct _ADDR2_COMPUTE_SURFACE_INFO_INPUT
{
    UINT_32               size;              ///< Size of this structure in bytes

    ADDR2_SURFACE_FLAGS   flags;             ///< Surface flags
    AddrSwizzleMode       swizzleMode;       ///< Swizzle Mode for Gfx9
    AddrResourceType      resourceType;      ///< Surface type
    AddrFormat            format;            ///< Surface format
    UINT_32               bpp;               ///< bits per pixel
    UINT_32               width;             ///< Width (of mip0), in pixels
    UINT_32               height;            ///< Height (of mip0), in pixels
    UINT_32               numSlices;         ///< Number surface slice/depth (of mip0),
    UINT_32               numMipLevels;      ///< Total mipmap levels.
    UINT_32               numSamples;        ///< Number of samples
    UINT_32               numFrags;          ///< Number of fragments, leave it zero or the same as
                                             ///  number of samples for normal AA; Set it to the
                                             ///  number of fragments for EQAA
    UINT_32               pitchInElement;    ///< Pitch in elements (blocks for compressed formats)
    UINT_32               sliceAlign;        ///< Required slice size in bytes
} ADDR2_COMPUTE_SURFACE_INFO_INPUT;

/**
****************************************************************************************************
*   ADDR2_MIP_INFO
*
*   @brief
*       Structure that contains information for mip level
*
****************************************************************************************************
*/
typedef struct _ADDR2_MIP_INFO
{
    UINT_32             pitch;              ///< Pitch in elements
    UINT_32             height;             ///< Padded height in elements
    UINT_32             depth;              ///< Padded depth
    UINT_32             pixelPitch;         ///< Pitch in pixels
    UINT_32             pixelHeight;        ///< Padded height in pixels
    UINT_32             equationIndex;      ///< Equation index in the equation table
    UINT_64             offset;             ///< Offset in bytes from mip base, should only be used
                                            ///< to setup vam surface descriptor, can't be used
                                            ///< to setup swizzle pattern
    UINT_64             macroBlockOffset;   ///< macro block offset in bytes from mip base
    UINT_32             mipTailOffset;      ///< mip tail offset in bytes
    UINT_32             mipTailCoordX;      ///< mip tail coord x
    UINT_32             mipTailCoordY;      ///< mip tail coord y
    UINT_32             mipTailCoordZ;      ///< mip tail coord z
} ADDR2_MIP_INFO;

/**
****************************************************************************************************
*   ADDR2_COMPUTE_SURFACE_INFO_OUTPUT
*
*   @brief
*       Output structure for Addr2ComputeSurfInfo
*   @note
        Element: AddrLib unit for computing. e.g. BCn: 4x4 blocks; R32B32B32: 32bit with 3x pitch
        Pixel: Original pixel
****************************************************************************************************
*/
typedef struct _ADDR2_COMPUTE_SURFACE_INFO_OUTPUT
{
    UINT_32             size;                 ///< Size of this structure in bytes

    UINT_32             pitch;                ///< Pitch in elements (blocks for compressed formats)
    UINT_32             height;               ///< Padded height (of mip0) in elements
    UINT_32             numSlices;            ///< Padded depth for 3d resource
                                              ///< or padded number of slices for 2d array resource
    UINT_32             mipChainPitch;        ///< Pitch (of total mip chain) in elements
    UINT_32             mipChainHeight;       ///< Padded height (of total mip chain) in elements
    UINT_32             mipChainSlice;        ///< Padded depth (of total mip chain)
    UINT_64             sliceSize;            ///< Slice (total mip chain) size in bytes
    UINT_64             surfSize;             ///< Surface (total mip chain) size in bytes
    UINT_32             baseAlign;            ///< Base address alignment
    UINT_32             bpp;                  ///< Bits per elements
                                              ///  (e.g. blocks for BCn, 1/3 for 96bit)
    UINT_32             pixelMipChainPitch;   ///< Mip chain pitch in original pixels
    UINT_32             pixelMipChainHeight;  ///< Mip chain height in original pixels
    UINT_32             pixelPitch;           ///< Pitch in original pixels
    UINT_32             pixelHeight;          ///< Height in original pixels
    UINT_32             pixelBits;            ///< Original bits per pixel, passed from input

    UINT_32             blockWidth;           ///< Width in element inside one block
    UINT_32             blockHeight;          ///< Height in element inside one block
    UINT_32             blockSlices;          ///< Slice number inside one block
                                              ///< Prt tile is one block, its width/height/slice
                                              ///< equals to blcok width/height/slice

    BOOL_32             epitchIsHeight;       ///< Whether to use height to program epitch register
    /// Stereo info
    ADDR_QBSTEREOINFO*  pStereoInfo;          ///< Stereo info, needed if qbStereo flag is TRUE
    /// Mip info
    ADDR2_MIP_INFO*     pMipInfo;             ///< Pointer to mip information array
                                              ///  if it is not NULL, the array is assumed to
                                              ///  contain numMipLevels entries

    UINT_32             equationIndex;        ///< Equation index in the equation table of mip0
    BOOL_32             mipChainInTail;       ///< If whole mipchain falls into mip tail block
    UINT_32             firstMipIdInTail;     ///< The id of first mip in tail, if there is no mip
                                              ///  in tail, it will be set to number of mip levels
} ADDR2_COMPUTE_SURFACE_INFO_OUTPUT;

/**
****************************************************************************************************
*   Addr2ComputeSurfaceInfo
*
*   @brief
*       Compute surface width/height/slices/alignments and suitable tiling mode
****************************************************************************************************
*/
ADDR_E_RETURNCODE ADDR_API Addr2ComputeSurfaceInfo(
    ADDR_HANDLE                                hLib,
    const ADDR2_COMPUTE_SURFACE_INFO_INPUT*    pIn,
    ADDR2_COMPUTE_SURFACE_INFO_OUTPUT*         pOut);



/**
****************************************************************************************************
*   ADDR2_COMPUTE_SURFACE_ADDRFROMCOORD_INPUT
*
*   @brief
*       Input structure for Addr2ComputeSurfaceAddrFromCoord
****************************************************************************************************
*/
typedef struct _ADDR2_COMPUTE_SURFACE_ADDRFROMCOORD_INPUT
{
    UINT_32             size;            ///< Size of this structure in bytes

    UINT_32             x;               ///< X coordinate
    UINT_32             y;               ///< Y coordinate
    UINT_32             slice;           ///< Slice index
    UINT_32             sample;          ///< Sample index, use fragment index for EQAA
    UINT_32             mipId;           ///< the mip ID in mip chain

    AddrSwizzleMode     swizzleMode;     ///< Swizzle mode for Gfx9
    ADDR2_SURFACE_FLAGS flags;           ///< Surface flags
    AddrResourceType    resourceType;    ///< Surface type
    UINT_32             bpp;             ///< Bits per pixel
    UINT_32             unalignedWidth;  ///< Surface original width (of mip0)
    UINT_32             unalignedHeight; ///< Surface original height (of mip0)
    UINT_32             numSlices;       ///< Surface original slices (of mip0)
    UINT_32             numMipLevels;    ///< Total mipmap levels
    UINT_32             numSamples;      ///< Number of samples
    UINT_32             numFrags;        ///< Number of fragments, leave it zero or the same as
                                         ///  number of samples for normal AA; Set it to the
                                         ///  number of fragments for EQAA

    UINT_32             pipeBankXor;     ///< Combined swizzle used to do bank/pipe rotation
    UINT_32             pitchInElement;  ///< Pitch in elements (blocks for compressed formats)
} ADDR2_COMPUTE_SURFACE_ADDRFROMCOORD_INPUT;

/**
****************************************************************************************************
*   ADDR2_COMPUTE_SURFACE_ADDRFROMCOORD_OUTPUT
*
*   @brief
*       Output structure for Addr2ComputeSurfaceAddrFromCoord
****************************************************************************************************
*/
typedef struct _ADDR2_COMPUTE_SURFACE_ADDRFROMCOORD_OUTPUT
{
    UINT_32    size;             ///< Size of this structure in bytes

    UINT_64    addr;             ///< Byte offset from the image starting address
    UINT_32    bitPosition;      ///< Bit position within surfaceAddr, 0-7.
                                 ///  For surface bpp < 8, e.g. FMT_1.
    UINT_32    prtBlockIndex;    ///< Index of a PRT tile (64K block)
} ADDR2_COMPUTE_SURFACE_ADDRFROMCOORD_OUTPUT;

/**
****************************************************************************************************
*   Addr2ComputeSurfaceAddrFromCoord
*
*   @brief
*       Compute surface address from a given coordinate.
****************************************************************************************************
*/
ADDR_E_RETURNCODE ADDR_API Addr2ComputeSurfaceAddrFromCoord(
    ADDR_HANDLE                                         hLib,
    const ADDR2_COMPUTE_SURFACE_ADDRFROMCOORD_INPUT*    pIn,
    ADDR2_COMPUTE_SURFACE_ADDRFROMCOORD_OUTPUT*         pOut);



/**
****************************************************************************************************
*   ADDR2_COMPUTE_SURFACE_COORDFROMADDR_INPUT
*
*   @brief
*       Input structure for Addr2ComputeSurfaceCoordFromAddr
****************************************************************************************************
*/
typedef struct _ADDR2_COMPUTE_SURFACE_COORDFROMADDR_INPUT
{
    UINT_32             size;            ///< Size of this structure in bytes

    UINT_64             addr;            ///< Address in bytes
    UINT_32             bitPosition;     ///< Bit position in addr. 0-7. for surface bpp < 8,
                                         ///  e.g. FMT_1;

    AddrSwizzleMode     swizzleMode;     ///< Swizzle mode for Gfx9
    ADDR2_SURFACE_FLAGS flags;           ///< Surface flags
    AddrResourceType    resourceType;    ///< Surface type
    UINT_32             bpp;             ///< Bits per pixel
    UINT_32             unalignedWidth;  ///< Surface original width (of mip0)
    UINT_32             unalignedHeight; ///< Surface original height (of mip0)
    UINT_32             numSlices;       ///< Surface original slices (of mip0)
    UINT_32             numMipLevels;    ///< Total mipmap levels.
    UINT_32             numSamples;      ///< Number of samples
    UINT_32             numFrags;        ///< Number of fragments, leave it zero or the same as
                                         ///  number of samples for normal AA; Set it to the
                                         ///  number of fragments for EQAA

    UINT_32             pipeBankXor;     ///< Combined swizzle used to do bank/pipe rotation
    UINT_32             pitchInElement;  ///< Pitch in elements (blocks for compressed formats)
} ADDR2_COMPUTE_SURFACE_COORDFROMADDR_INPUT;

/**
****************************************************************************************************
*   ADDR2_COMPUTE_SURFACE_COORDFROMADDR_OUTPUT
*
*   @brief
*       Output structure for Addr2ComputeSurfaceCoordFromAddr
****************************************************************************************************
*/
typedef struct _ADDR2_COMPUTE_SURFACE_COORDFROMADDR_OUTPUT
{
    UINT_32    size;       ///< Size of this structure in bytes

    UINT_32    x;          ///< X coordinate
    UINT_32    y;          ///< Y coordinate
    UINT_32    slice;      ///< Index of slices
    UINT_32    sample;     ///< Index of samples, means fragment index for EQAA
    UINT_32    mipId;      ///< mipmap level id
} ADDR2_COMPUTE_SURFACE_COORDFROMADDR_OUTPUT;

/**
****************************************************************************************************
*   Addr2ComputeSurfaceCoordFromAddr
*
*   @brief
*       Compute coordinate from a given surface address
****************************************************************************************************
*/
ADDR_E_RETURNCODE ADDR_API Addr2ComputeSurfaceCoordFromAddr(
    ADDR_HANDLE                                         hLib,
    const ADDR2_COMPUTE_SURFACE_COORDFROMADDR_INPUT*    pIn,
    ADDR2_COMPUTE_SURFACE_COORDFROMADDR_OUTPUT*         pOut);



////////////////////////////////////////////////////////////////////////////////////////////////////
//                                   HTile functions for Gfx9
////////////////////////////////////////////////////////////////////////////////////////////////////

/**
****************************************************************************************************
*   ADDR2_META_FLAGS
*
*   @brief
*       Metadata flags
****************************************************************************************************
*/
typedef union _ADDR2_META_FLAGS
{
    struct
    {
        UINT_32 pipeAligned :  1;    ///< if Metadata being pipe aligned
        UINT_32 rbAligned   :  1;    ///< if Metadata being RB aligned
        UINT_32 linear      :  1;    ///< if Metadata linear, GFX9 does not suppord this!
        UINT_32 reserved    : 29;    ///< Reserved bits
    };

    UINT_32 value;
} ADDR2_META_FLAGS;

/**
****************************************************************************************************
*   ADDR2_META_MIP_INFO
*
*   @brief
*       Structure to store per mip metadata information
****************************************************************************************************
*/
typedef struct _ADDR2_META_MIP_INFO
{
    BOOL_32    inMiptail;
    union
    {
        struct
        {
            UINT_32    startX;
            UINT_32    startY;
            UINT_32    startZ;
            UINT_32    width;
            UINT_32    height;
            UINT_32    depth;
        };

        // GFX10
        struct
        {
            UINT_32    offset;      ///< Metadata offset within one slice,
                                    ///  the thickness of a slice is meta block depth.
            UINT_32    sliceSize;   ///< Metadata size within one slice,
                                    ///  the thickness of a slice is meta block depth.
        };
    };
} ADDR2_META_MIP_INFO;

/**
****************************************************************************************************
*   ADDR2_COMPUTE_HTILE_INFO_INPUT
*
*   @brief
*       Input structure of Addr2ComputeHtileInfo
****************************************************************************************************
*/
typedef struct _ADDR2_COMPUTE_HTILE_INFO_INPUT
{
    UINT_32             size;               ///< Size of this structure in bytes

    ADDR2_META_FLAGS    hTileFlags;         ///< HTILE flags
    ADDR2_SURFACE_FLAGS depthFlags;         ///< Depth surface flags
    AddrSwizzleMode     swizzleMode;        ///< Depth surface swizzle mode
    UINT_32             unalignedWidth;     ///< Depth surface original width (of mip0)
    UINT_32             unalignedHeight;    ///< Depth surface original height (of mip0)
    UINT_32             numSlices;          ///< Number of slices of depth surface (of mip0)
    UINT_32             numMipLevels;       ///< Total mipmap levels of color surface
    UINT_32             firstMipIdInTail;   ///  Id of the first mip in tail,
                                            ///  if no mip is in tail, it should be set to
                                            ///  number of mip levels
                                            ///  Only for GFX10
} ADDR2_COMPUTE_HTILE_INFO_INPUT;

/**
****************************************************************************************************
*   ADDR2_COMPUTE_HTILE_INFO_OUTPUT
*
*   @brief
*       Output structure of Addr2ComputeHtileInfo
****************************************************************************************************
*/
typedef struct _ADDR2_COMPUTE_HTILE_INFO_OUTPUT
{
    UINT_32    size;                ///< Size of this structure in bytes

    UINT_32    pitch;               ///< Pitch in pixels of depth buffer represented in this
                                    ///  HTile buffer. This might be larger than original depth
                                    ///  buffer pitch when called with an unaligned pitch.
    UINT_32    height;              ///< Height in pixels, as above
    UINT_32    baseAlign;           ///< Base alignment
    UINT_32    sliceSize;           ///< Slice size, in bytes.
    UINT_32    htileBytes;          ///< Size of HTILE buffer, in bytes
    UINT_32    metaBlkWidth;        ///< Meta block width
    UINT_32    metaBlkHeight;       ///< Meta block height
    UINT_32    metaBlkNumPerSlice;  ///< Number of metablock within one slice

    ADDR2_META_MIP_INFO* pMipInfo;  ///< HTILE mip information

    struct {
      UINT_16* gfx10_bits; /* 72 2-byte elements */
   } equation;
} ADDR2_COMPUTE_HTILE_INFO_OUTPUT;

/**
****************************************************************************************************
*   Addr2ComputeHtileInfo
*
*   @brief
*       Compute Htile pitch, height, base alignment and size in bytes
****************************************************************************************************
*/
ADDR_E_RETURNCODE ADDR_API Addr2ComputeHtileInfo(
    ADDR_HANDLE                              hLib,
    const ADDR2_COMPUTE_HTILE_INFO_INPUT*    pIn,
    ADDR2_COMPUTE_HTILE_INFO_OUTPUT*         pOut);



/**
****************************************************************************************************
*   ADDR2_COMPUTE_HTILE_ADDRFROMCOORD_INPUT
*
*   @brief
*       Input structure for Addr2ComputeHtileAddrFromCoord
****************************************************************************************************
*/
typedef struct _ADDR2_COMPUTE_HTILE_ADDRFROMCOORD_INPUT
{
    UINT_32             size;                ///< Size of this structure in bytes

    UINT_32             x;                   ///< X coordinate
    UINT_32             y;                   ///< Y coordinate
    UINT_32             slice;               ///< Index of slices
    UINT_32             mipId;               ///< mipmap level id

    ADDR2_META_FLAGS    hTileFlags;          ///< HTILE flags
    ADDR2_SURFACE_FLAGS depthflags;          ///< Depth surface flags
    AddrSwizzleMode     swizzleMode;         ///< Depth surface swizzle mode
    UINT_32             bpp;                 ///< Depth surface bits per pixel
    UINT_32             unalignedWidth;      ///< Depth surface original width (of mip0)
    UINT_32             unalignedHeight;     ///< Depth surface original height (of mip0)
    UINT_32             numSlices;           ///< Depth surface original depth (of mip0)
    UINT_32             numMipLevels;        ///< Depth surface total mipmap levels
    UINT_32             numSamples;          ///< Depth surface number of samples
    UINT_32             pipeXor;             ///< Pipe xor setting
} ADDR2_COMPUTE_HTILE_ADDRFROMCOORD_INPUT;

/**
****************************************************************************************************
*   ADDR2_COMPUTE_HTILE_ADDRFROMCOORD_OUTPUT
*
*   @brief
*       Output structure for Addr2ComputeHtileAddrFromCoord
****************************************************************************************************
*/
typedef struct _ADDR2_COMPUTE_HTILE_ADDRFROMCOORD_OUTPUT
{
    UINT_32    size;    ///< Size of this structure in bytes

    UINT_64    addr;    ///< Address in bytes
} ADDR2_COMPUTE_HTILE_ADDRFROMCOORD_OUTPUT;

/**
****************************************************************************************************
*   Addr2ComputeHtileAddrFromCoord
*
*   @brief
*       Compute Htile address according to coordinates (of depth buffer)
****************************************************************************************************
*/
ADDR_E_RETURNCODE ADDR_API Addr2ComputeHtileAddrFromCoord(
    ADDR_HANDLE                                       hLib,
    const ADDR2_COMPUTE_HTILE_ADDRFROMCOORD_INPUT*    pIn,
    ADDR2_COMPUTE_HTILE_ADDRFROMCOORD_OUTPUT*         pOut);



/**
****************************************************************************************************
*   ADDR2_COMPUTE_HTILE_COORDFROMADDR_INPUT
*
*   @brief
*       Input structure for Addr2ComputeHtileCoordFromAddr
****************************************************************************************************
*/
typedef struct _ADDR2_COMPUTE_HTILE_COORDFROMADDR_INPUT
{
    UINT_32             size;                ///< Size of this structure in bytes

    UINT_64             addr;                ///< Address

    ADDR2_META_FLAGS    hTileFlags;          ///< HTILE flags
    ADDR2_SURFACE_FLAGS depthFlags;          ///< Depth surface flags
    AddrSwizzleMode     swizzleMode;         ///< Depth surface swizzle mode
    UINT_32             bpp;                 ///< Depth surface bits per pixel
    UINT_32             unalignedWidth;      ///< Depth surface original width (of mip0)
    UINT_32             unalignedHeight;     ///< Depth surface original height (of mip0)
    UINT_32             numSlices;           ///< Depth surface original depth (of mip0)
    UINT_32             numMipLevels;        ///< Depth surface total mipmap levels
    UINT_32             numSamples;          ///< Depth surface number of samples
    UINT_32             pipeXor;             ///< Pipe xor setting
} ADDR2_COMPUTE_HTILE_COORDFROMADDR_INPUT;

/**
****************************************************************************************************
*   ADDR2_COMPUTE_HTILE_COORDFROMADDR_OUTPUT
*
*   @brief
*       Output structure for Addr2ComputeHtileCoordFromAddr
****************************************************************************************************
*/
typedef struct _ADDR2_COMPUTE_HTILE_COORDFROMADDR_OUTPUT
{
    UINT_32    size;        ///< Size of this structure in bytes

    UINT_32    x;           ///< X coordinate
    UINT_32    y;           ///< Y coordinate
    UINT_32    slice;       ///< Index of slices
    UINT_32    mipId;       ///< mipmap level id
} ADDR2_COMPUTE_HTILE_COORDFROMADDR_OUTPUT;

/**
****************************************************************************************************
*   Addr2ComputeHtileCoordFromAddr
*
*   @brief
*       Compute coordinates within depth buffer (1st pixel of a micro tile) according to
*       Htile address
****************************************************************************************************
*/
ADDR_E_RETURNCODE ADDR_API Addr2ComputeHtileCoordFromAddr(
    ADDR_HANDLE                                       hLib,
    const ADDR2_COMPUTE_HTILE_COORDFROMADDR_INPUT*    pIn,
    ADDR2_COMPUTE_HTILE_COORDFROMADDR_OUTPUT*         pOut);



////////////////////////////////////////////////////////////////////////////////////////////////////
//                                     C-mask functions for Gfx9
////////////////////////////////////////////////////////////////////////////////////////////////////

/**
****************************************************************************************************
*   ADDR2_COMPUTE_CMASK_INFO_INPUT
*
*   @brief
*       Input structure of Addr2ComputeCmaskInfo
****************************************************************************************************
*/
typedef struct _ADDR2_COMPUTE_CMASKINFO_INPUT
{
    UINT_32             size;               ///< Size of this structure in bytes

    ADDR2_META_FLAGS    cMaskFlags;         ///< CMASK flags
    ADDR2_SURFACE_FLAGS colorFlags;         ///< Color surface flags
    AddrResourceType    resourceType;       ///< Color surface type
    AddrSwizzleMode     swizzleMode;        ///< FMask surface swizzle mode
    UINT_32             unalignedWidth;     ///< Color surface original width
    UINT_32             unalignedHeight;    ///< Color surface original height
    UINT_32             numSlices;          ///< Number of slices of color buffer
    UINT_32             numMipLevels;       ///< Number of mip levels
    UINT_32             firstMipIdInTail;   ///< The id of first mip in tail, if no mip is in tail,
                                            ///  it should be number of mip levels
                                            ///  Only for GFX10
} ADDR2_COMPUTE_CMASK_INFO_INPUT;

/* DCC addr meta equation for GFX9. */
struct gfx9_addr_meta_equation {
   UINT_8 num_bits;

   struct {
      struct {
         UINT_8 dim; /* 0..4 as index, 5 means invalid */
         UINT_8 ord; /* 0..31 */
      } coord[8]; /* 0..num_coords */
   } bit[32]; /* 0..num_bits */

   UINT_8 numPipeBits;
};

/**
****************************************************************************************************
*   ADDR2_COMPUTE_CMASK_INFO_OUTPUT
*
*   @brief
*       Output structure of Addr2ComputeCmaskInfo
****************************************************************************************************
*/
typedef struct _ADDR2_COMPUTE_CMASK_INFO_OUTPUT
{
    UINT_32    size;          ///< Size of this structure in bytes

    UINT_32    pitch;         ///< Pitch in pixels of color buffer which
                              ///  this Cmask matches. The size might be larger than
                              ///  original color buffer pitch when called with
                              ///  an unaligned pitch.
    UINT_32    height;        ///< Height in pixels, as above
    UINT_32    baseAlign;     ///< Base alignment
    UINT_32    sliceSize;     ///< Slice size, in bytes.
    UINT_32    cmaskBytes;    ///< Size in bytes of CMask buffer
    UINT_32    metaBlkWidth;  ///< Meta block width
    UINT_32    metaBlkHeight; ///< Meta block height

    UINT_32    metaBlkNumPerSlice;  ///< Number of metablock within one slice

    ADDR2_META_MIP_INFO* pMipInfo;  ///< CMASK mip information

    /* The equation for doing CMASK address computations in shaders. */
    union {
       /* This is chip-specific, and it varies with:
        * - resource type
        * - swizzle_mode
        * - bpp
        * - pipe_aligned
        * - rb_aligned
        */
       struct gfx9_addr_meta_equation gfx9;

       /* This is chip-specific, it requires 64KB_Z_X. */
       UINT_16 *gfx10_bits; /* 68 2-byte elements */
    } equation;
} ADDR2_COMPUTE_CMASK_INFO_OUTPUT;

/**
****************************************************************************************************
*   Addr2ComputeCmaskInfo
*
*   @brief
*       Compute Cmask pitch, height, base alignment and size in bytes from color buffer
*       info
****************************************************************************************************
*/
ADDR_E_RETURNCODE ADDR_API Addr2ComputeCmaskInfo(
    ADDR_HANDLE                              hLib,
    const ADDR2_COMPUTE_CMASK_INFO_INPUT*    pIn,
    ADDR2_COMPUTE_CMASK_INFO_OUTPUT*         pOut);



/**
****************************************************************************************************
*   ADDR2_COMPUTE_CMASK_ADDRFROMCOORD_INPUT
*
*   @brief
*       Input structure for Addr2ComputeCmaskAddrFromCoord
*
****************************************************************************************************
*/
typedef struct _ADDR2_COMPUTE_CMASK_ADDRFROMCOORD_INPUT
{
    UINT_32             size;                ///< Size of this structure in bytes

    UINT_32             x;                   ///< X coordinate
    UINT_32             y;                   ///< Y coordinate
    UINT_32             slice;               ///< Index of slices

    ADDR2_META_FLAGS    cMaskFlags;          ///< CMASK flags
    ADDR2_SURFACE_FLAGS colorFlags;          ///< Color surface flags
    AddrResourceType    resourceType;        ///< Color surface type
    AddrSwizzleMode     swizzleMode;         ///< FMask surface swizzle mode

    UINT_32             unalignedWidth;      ///< Color surface original width (of mip0)
    UINT_32             unalignedHeight;     ///< Color surface original height (of mip0)
    UINT_32             numSlices;           ///< Color surface original slices (of mip0)

    UINT_32             numSamples;          ///< Color surfae sample number
    UINT_32             numFrags;            ///< Color surface fragment number

    UINT_32             pipeXor;             ///< pipe Xor setting
} ADDR2_COMPUTE_CMASK_ADDRFROMCOORD_INPUT;

/**
****************************************************************************************************
*   ADDR2_COMPUTE_CMASK_ADDRFROMCOORD_OUTPUT
*
*   @brief
*       Output structure for Addr2ComputeCmaskAddrFromCoord
****************************************************************************************************
*/
typedef struct _ADDR2_COMPUTE_CMASK_ADDRFROMCOORD_OUTPUT
{
    UINT_32    size;           ///< Size of this structure in bytes

    UINT_64    addr;           ///< CMASK address in bytes
    UINT_32    bitPosition;    ///< Bit position within addr, 0 or 4
} ADDR2_COMPUTE_CMASK_ADDRFROMCOORD_OUTPUT;

/**
****************************************************************************************************
*   Addr2ComputeCmaskAddrFromCoord
*
*   @brief
*       Compute Cmask address according to coordinates (of MSAA color buffer)
****************************************************************************************************
*/
ADDR_E_RETURNCODE ADDR_API Addr2ComputeCmaskAddrFromCoord(
    ADDR_HANDLE                                      hLib,
    const ADDR2_COMPUTE_CMASK_ADDRFROMCOORD_INPUT*   pIn,
    ADDR2_COMPUTE_CMASK_ADDRFROMCOORD_OUTPUT*        pOut);



/**
****************************************************************************************************
*   ADDR2_COMPUTE_CMASK_COORDFROMADDR_INPUT
*
*   @brief
*       Input structure for Addr2ComputeCmaskCoordFromAddr
****************************************************************************************************
*/
typedef struct _ADDR2_COMPUTE_CMASK_COORDFROMADDR_INPUT
{
    UINT_32             size;                ///< Size of this structure in bytes

    UINT_64             addr;                ///< CMASK address in bytes
    UINT_32             bitPosition;         ///< Bit position within addr, 0 or 4

    ADDR2_META_FLAGS    cMaskFlags;          ///< CMASK flags
    ADDR2_SURFACE_FLAGS colorFlags;          ///< Color surface flags
    AddrResourceType    resourceType;        ///< Color surface type
    AddrSwizzleMode     swizzleMode;         ///< FMask surface swizzle mode

    UINT_32             unalignedWidth;      ///< Color surface original width (of mip0)
    UINT_32             unalignedHeight;     ///< Color surface original height (of mip0)
    UINT_32             numSlices;           ///< Color surface original slices (of mip0)
    UINT_32             numMipLevels;        ///< Color surface total mipmap levels.
} ADDR2_COMPUTE_CMASK_COORDFROMADDR_INPUT;

/**
****************************************************************************************************
*   ADDR2_COMPUTE_CMASK_COORDFROMADDR_OUTPUT
*
*   @brief
*       Output structure for Addr2ComputeCmaskCoordFromAddr
****************************************************************************************************
*/
typedef struct _ADDR2_COMPUTE_CMASK_COORDFROMADDR_OUTPUT
{
    UINT_32    size;        ///< Size of this structure in bytes

    UINT_32    x;           ///< X coordinate
    UINT_32    y;           ///< Y coordinate
    UINT_32    slice;       ///< Index of slices
    UINT_32    mipId;       ///< mipmap level id
} ADDR2_COMPUTE_CMASK_COORDFROMADDR_OUTPUT;

/**
****************************************************************************************************
*   Addr2ComputeCmaskCoordFromAddr
*
*   @brief
*       Compute coordinates within color buffer (1st pixel of a micro tile) according to
*       Cmask address
****************************************************************************************************
*/
ADDR_E_RETURNCODE ADDR_API Addr2ComputeCmaskCoordFromAddr(
    ADDR_HANDLE                                       hLib,
    const ADDR2_COMPUTE_CMASK_COORDFROMADDR_INPUT*    pIn,
    ADDR2_COMPUTE_CMASK_COORDFROMADDR_OUTPUT*         pOut);



////////////////////////////////////////////////////////////////////////////////////////////////////
//                                     F-mask functions for Gfx9
////////////////////////////////////////////////////////////////////////////////////////////////////

/**
****************************************************************************************************
*   ADDR2_FMASK_FLAGS
*
*   @brief
*       FMASK flags
****************************************************************************************************
*/
typedef union _ADDR2_FMASK_FLAGS
{
    struct
    {
        UINT_32 resolved :  1;    ///< TRUE if this is a resolved fmask, used by H/W clients
                                  ///  by H/W clients. S/W should always set it to FALSE.
        UINT_32 reserved : 31;    ///< Reserved for future use.
    };

    UINT_32 value;
} ADDR2_FMASK_FLAGS;

/**
****************************************************************************************************
*   ADDR2_COMPUTE_FMASK_INFO_INPUT
*
*   @brief
*       Input structure for Addr2ComputeFmaskInfo
****************************************************************************************************
*/
typedef struct _ADDR2_COMPUTE_FMASK_INFO_INPUT
{
    UINT_32             size;               ///< Size of this structure in bytes

    AddrSwizzleMode     swizzleMode;        ///< FMask surface swizzle mode
    UINT_32             unalignedWidth;     ///< Color surface original width
    UINT_32             unalignedHeight;    ///< Color surface original height
    UINT_32             numSlices;          ///< Number of slices/depth
    UINT_32             numSamples;         ///< Number of samples
    UINT_32             numFrags;           ///< Number of fragments, leave it zero or the same as
                                            ///  number of samples for normal AA; Set it to the
                                            ///  number of fragments for EQAA
    ADDR2_FMASK_FLAGS   fMaskFlags;         ///< FMASK flags
} ADDR2_COMPUTE_FMASK_INFO_INPUT;

/**
****************************************************************************************************
*   ADDR2_COMPUTE_FMASK_INFO_OUTPUT
*
*   @brief
*       Output structure for Addr2ComputeFmaskInfo
****************************************************************************************************
*/
typedef struct _ADDR2_COMPUTE_FMASK_INFO_OUTPUT
{
    UINT_32    size;           ///< Size of this structure in bytes

    UINT_32    pitch;          ///< Pitch of fmask in pixels
    UINT_32    height;         ///< Height of fmask in pixels
    UINT_32    baseAlign;      ///< Base alignment
    UINT_32    numSlices;      ///< Slices of fmask
    UINT_32    fmaskBytes;     ///< Size of fmask in bytes
    UINT_32    bpp;            ///< Bits per pixel of FMASK is: number of bit planes
    UINT_32    numSamples;     ///< Number of samples
    UINT_32    sliceSize;      ///< Size of slice in bytes
} ADDR2_COMPUTE_FMASK_INFO_OUTPUT;

/**
****************************************************************************************************
*   Addr2ComputeFmaskInfo
*
*   @brief
*       Compute Fmask pitch/height/slices/alignments and size in bytes
****************************************************************************************************
*/
ADDR_E_RETURNCODE ADDR_API Addr2ComputeFmaskInfo(
    ADDR_HANDLE                              hLib,
    const ADDR2_COMPUTE_FMASK_INFO_INPUT*    pIn,
    ADDR2_COMPUTE_FMASK_INFO_OUTPUT*         pOut);



/**
****************************************************************************************************
*   ADDR2_COMPUTE_FMASK_ADDRFROMCOORD_INPUT
*
*   @brief
*       Input structure for Addr2ComputeFmaskAddrFromCoord
****************************************************************************************************
*/
typedef struct _ADDR2_COMPUTE_FMASK_ADDRFROMCOORD_INPUT
{
    UINT_32            size;               ///< Size of this structure in bytes

    AddrSwizzleMode    swizzleMode;        ///< FMask surface swizzle mode
    UINT_32            x;                  ///< X coordinate
    UINT_32            y;                  ///< Y coordinate
    UINT_32            slice;              ///< Slice index
    UINT_32            sample;             ///< Sample index (fragment index for EQAA)
    UINT_32            plane;              ///< Plane number

    UINT_32            unalignedWidth;     ///< Color surface original width
    UINT_32            unalignedHeight;    ///< Color surface original height
    UINT_32            numSamples;         ///< Number of samples
    UINT_32            numFrags;           ///< Number of fragments, leave it zero or the same as
                                           ///  number of samples for normal AA; Set it to the
                                           ///  number of fragments for EQAA
    UINT_32            tileSwizzle;        ///< Combined swizzle used to do bank/pipe rotation

    ADDR2_FMASK_FLAGS  fMaskFlags; ///< FMASK flags
} ADDR2_COMPUTE_FMASK_ADDRFROMCOORD_INPUT;

/**
****************************************************************************************************
*   ADDR2_COMPUTE_FMASK_ADDRFROMCOORD_OUTPUT
*
*   @brief
*       Output structure for Addr2ComputeFmaskAddrFromCoord
****************************************************************************************************
*/
typedef struct _ADDR2_COMPUTE_FMASK_ADDRFROMCOORD_OUTPUT
{
    UINT_32    size;           ///< Size of this structure in bytes

    UINT_64    addr;           ///< Fmask address
    UINT_32    bitPosition;    ///< Bit position within fmaskAddr, 0-7.
} ADDR2_COMPUTE_FMASK_ADDRFROMCOORD_OUTPUT;

/**
****************************************************************************************************
*   Addr2ComputeFmaskAddrFromCoord
*
*   @brief
*       Compute Fmask address according to coordinates (x,y,slice,sample,plane)
****************************************************************************************************
*/
ADDR_E_RETURNCODE ADDR_API Addr2ComputeFmaskAddrFromCoord(
    ADDR_HANDLE                                       hLib,
    const ADDR2_COMPUTE_FMASK_ADDRFROMCOORD_INPUT*    pIn,
    ADDR2_COMPUTE_FMASK_ADDRFROMCOORD_OUTPUT*         pOut);



/**
****************************************************************************************************
*   ADDR2_COMPUTE_FMASK_COORDFROMADDR_INPUT
*
*   @brief
*       Input structure for Addr2ComputeFmaskCoordFromAddr
****************************************************************************************************
*/
typedef struct _ADDR2_COMPUTE_FMASK_COORDFROMADDR_INPUT
{
    UINT_32            size;               ///< Size of this structure in bytes

    UINT_64            addr;               ///< Address
    UINT_32            bitPosition;        ///< Bit position within addr, 0-7.
    AddrSwizzleMode    swizzleMode;        ///< FMask surface swizzle mode

    UINT_32            unalignedWidth;     ///< Color surface original width
    UINT_32            unalignedHeight;    ///< Color surface original height
    UINT_32            numSamples;         ///< Number of samples
    UINT_32            numFrags;           ///< Number of fragments

    UINT_32            tileSwizzle;        ///< Combined swizzle used to do bank/pipe rotation

    ADDR2_FMASK_FLAGS  fMaskFlags; ///< FMASK flags
} ADDR2_COMPUTE_FMASK_COORDFROMADDR_INPUT;

/**
****************************************************************************************************
*   ADDR2_COMPUTE_FMASK_COORDFROMADDR_OUTPUT
*
*   @brief
*       Output structure for Addr2ComputeFmaskCoordFromAddr
****************************************************************************************************
*/
typedef struct _ADDR2_COMPUTE_FMASK_COORDFROMADDR_OUTPUT
{
    UINT_32    size;      ///< Size of this structure in bytes

    UINT_32    x;         ///< X coordinate
    UINT_32    y;         ///< Y coordinate
    UINT_32    slice;     ///< Slice index
    UINT_32    sample;    ///< Sample index (fragment index for EQAA)
    UINT_32    plane;     ///< Plane number
} ADDR2_COMPUTE_FMASK_COORDFROMADDR_OUTPUT;

/**
****************************************************************************************************
*   Addr2ComputeFmaskCoordFromAddr
*
*   @brief
*       Compute FMASK coordinate from an given address
****************************************************************************************************
*/
ADDR_E_RETURNCODE ADDR_API Addr2ComputeFmaskCoordFromAddr(
    ADDR_HANDLE                                       hLib,
    const ADDR2_COMPUTE_FMASK_COORDFROMADDR_INPUT*    pIn,
    ADDR2_COMPUTE_FMASK_COORDFROMADDR_OUTPUT*         pOut);



////////////////////////////////////////////////////////////////////////////////////////////////////
//                                     DCC key functions for Gfx9
////////////////////////////////////////////////////////////////////////////////////////////////////

/**
****************************************************************************************************
*   _ADDR2_COMPUTE_DCCINFO_INPUT
*
*   @brief
*       Input structure of Addr2ComputeDccInfo
****************************************************************************************************
*/
typedef struct _ADDR2_COMPUTE_DCCINFO_INPUT
{
    UINT_32             size;               ///< Size of this structure in bytes

    ADDR2_META_FLAGS    dccKeyFlags;        ///< DCC key flags
    ADDR2_SURFACE_FLAGS colorFlags;         ///< Color surface flags
    AddrResourceType    resourceType;       ///< Color surface type
    AddrSwizzleMode     swizzleMode;        ///< Color surface swizzle mode
    UINT_32             bpp;                ///< bits per pixel
    UINT_32             unalignedWidth;     ///< Color surface original width (of mip0)
    UINT_32             unalignedHeight;    ///< Color surface original height (of mip0)
    UINT_32             numSlices;          ///< Number of slices, of color surface (of mip0)
    UINT_32             numFrags;           ///< Fragment number of color surface
    UINT_32             numMipLevels;       ///< Total mipmap levels of color surface
    UINT_32             dataSurfaceSize;    ///< The padded size of all slices and mip levels
                                            ///< useful in meta linear case
    UINT_32             firstMipIdInTail;   ///< The id of first mip in tail, if no mip is in tail,
                                            ///  it should be number of mip levels
                                            ///  Only for GFX10
} ADDR2_COMPUTE_DCCINFO_INPUT;

/**
****************************************************************************************************
*   ADDR2_COMPUTE_DCCINFO_OUTPUT
*
*   @brief
*       Output structure of Addr2ComputeDccInfo
****************************************************************************************************
*/
typedef struct _ADDR2_COMPUTE_DCCINFO_OUTPUT
{
    UINT_32    size;               ///< Size of this structure in bytes

    UINT_32    dccRamBaseAlign;    ///< Base alignment of dcc key
    UINT_32    dccRamSize;         ///< Size of dcc key

    UINT_32    pitch;              ///< DCC surface mip chain pitch
    UINT_32    height;             ///< DCC surface mip chain height
    UINT_32    depth;              ///< DCC surface mip chain depth

    UINT_32    compressBlkWidth;   ///< DCC compress block width
    UINT_32    compressBlkHeight;  ///< DCC compress block height
    UINT_32    compressBlkDepth;   ///< DCC compress block depth

    UINT_32    metaBlkWidth;       ///< DCC meta block width
    UINT_32    metaBlkHeight;      ///< DCC meta block height
    UINT_32    metaBlkDepth;       ///< DCC meta block depth
    UINT_32    metaBlkSize;        ///< DCC meta block size in bytes
    UINT_32    metaBlkNumPerSlice; ///< Number of metablock within one slice

    union
    {
        UINT_32 fastClearSizePerSlice;  ///< Size of DCC within a slice should be fast cleared
        UINT_32 dccRamSliceSize;        ///< DCC ram size per slice. For mipmap, it's
                                        ///  the slize size of a mip chain, the thickness of a
                                        ///  a slice is meta block depth
                                        ///  Only for GFX10
    };

    ADDR2_META_MIP_INFO* pMipInfo;      ///< DCC mip information

    /* The equation for doing DCC address computations in shaders. */
    union {
       /* This is chip-specific, and it varies with:
        * - resource type
        * - swizzle_mode
        * - bpp
        * - number of fragments
        * - pipe_aligned
        * - rb_aligned
        */
       struct gfx9_addr_meta_equation gfx9;

       /* This is chip-specific, it requires 64KB_R_X, and it varies with:
        * - bpp
        * - pipe_aligned
        */
       UINT_16 *gfx10_bits; /* 68 2-byte elements */
    } equation;
} ADDR2_COMPUTE_DCCINFO_OUTPUT;

/**
****************************************************************************************************
*   Addr2ComputeDccInfo
*
*   @brief
*       Compute DCC key size, base alignment
*       info
****************************************************************************************************
*/
ADDR_E_RETURNCODE ADDR_API Addr2ComputeDccInfo(
    ADDR_HANDLE                           hLib,
    const ADDR2_COMPUTE_DCCINFO_INPUT*    pIn,
    ADDR2_COMPUTE_DCCINFO_OUTPUT*         pOut);


/**
****************************************************************************************************
*   ADDR2_COMPUTE_DCC_ADDRFROMCOORD_INPUT
*
*   @brief
*       Input structure for Addr2ComputeDccAddrFromCoord
*
****************************************************************************************************
*/
typedef struct _ADDR2_COMPUTE_DCC_ADDRFROMCOORD_INPUT
{
    UINT_32             size;                ///< Size of this structure in bytes

    UINT_32             x;                   ///< X coordinate
    UINT_32             y;                   ///< Y coordinate
    UINT_32             slice;               ///< Index of slices
    UINT_32             sample;              ///< Index of samples, means fragment index for EQAA
    UINT_32             mipId;               ///< mipmap level id

    ADDR2_META_FLAGS    dccKeyFlags;         ///< DCC flags
    ADDR2_SURFACE_FLAGS colorFlags;          ///< Color surface flags
    AddrResourceType    resourceType;        ///< Color surface type
    AddrSwizzleMode     swizzleMode;         ///< Color surface swizzle mode
    UINT_32             bpp;                 ///< Color surface bits per pixel
    UINT_32             unalignedWidth;      ///< Color surface original width (of mip0)
    UINT_32             unalignedHeight;     ///< Color surface original height (of mip0)
    UINT_32             numSlices;           ///< Color surface original slices (of mip0)
    UINT_32             numMipLevels;        ///< Color surface mipmap levels
    UINT_32             numFrags;            ///< Color surface fragment number

    UINT_32             pipeXor;             ///< pipe Xor setting
    UINT_32             pitch;               ///< ADDR2_COMPUTE_DCC_INFO_OUTPUT::pitch
    UINT_32             height;              ///< ADDR2_COMPUTE_DCC_INFO_OUTPUT::height
    UINT_32             compressBlkWidth;    ///< ADDR2_COMPUTE_DCC_INFO_OUTPUT::compressBlkWidth
    UINT_32             compressBlkHeight;   ///< ADDR2_COMPUTE_DCC_INFO_OUTPUT::compressBlkHeight
    UINT_32             compressBlkDepth;    ///< ADDR2_COMPUTE_DCC_INFO_OUTPUT::compressBlkDepth
    UINT_32             metaBlkWidth;        ///< ADDR2_COMPUTE_DCC_INFO_OUTPUT::metaBlkWidth
    UINT_32             metaBlkHeight;       ///< ADDR2_COMPUTE_DCC_INFO_OUTPUT::metaBlkHeight
    UINT_32             metaBlkDepth;        ///< ADDR2_COMPUTE_DCC_INFO_OUTPUT::metaBlkDepth
    UINT_32             dccRamSliceSize;     ///< ADDR2_COMPUTE_DCC_INFO_OUTPUT::dccRamSliceSize
} ADDR2_COMPUTE_DCC_ADDRFROMCOORD_INPUT;

/**
****************************************************************************************************
*   ADDR2_COMPUTE_DCC_ADDRFROMCOORD_OUTPUT
*
*   @brief
*       Output structure for Addr2ComputeDccAddrFromCoord
****************************************************************************************************
*/
typedef struct _ADDR2_COMPUTE_DCC_ADDRFROMCOORD_OUTPUT
{
    UINT_32    size;           ///< Size of this structure in bytes

    UINT_64    addr;           ///< DCC address in bytes
} ADDR2_COMPUTE_DCC_ADDRFROMCOORD_OUTPUT;

/**
****************************************************************************************************
*   Addr2ComputeDccAddrFromCoord
*
*   @brief
*       Compute DCC address according to coordinates (of MSAA color buffer)
****************************************************************************************************
*/
ADDR_E_RETURNCODE ADDR_API Addr2ComputeDccAddrFromCoord(
    ADDR_HANDLE                                    hLib,
    const ADDR2_COMPUTE_DCC_ADDRFROMCOORD_INPUT*   pIn,
    ADDR2_COMPUTE_DCC_ADDRFROMCOORD_OUTPUT*        pOut);

////////////////////////////////////////////////////////////////////////////////////////////////////
//                                     Misc functions for Gfx9
////////////////////////////////////////////////////////////////////////////////////////////////////

/**
****************************************************************************************************
*   ADDR2_COMPUTE_PIPEBANKXOR_INPUT
*
*   @brief
*       Input structure of Addr2ComputePipebankXor
****************************************************************************************************
*/
typedef struct _ADDR2_COMPUTE_PIPEBANKXOR_INPUT
{
    UINT_32             size;               ///< Size of this structure in bytes
    UINT_32             surfIndex;          ///< Input surface index
    ADDR2_SURFACE_FLAGS flags;              ///< Surface flag
    AddrSwizzleMode     swizzleMode;        ///< Surface swizzle mode
    AddrResourceType    resourceType;       ///< Surface resource type
    AddrFormat          format;             ///< Surface format
    UINT_32             numSamples;         ///< Number of samples
    UINT_32             numFrags;           ///< Number of fragments, leave it zero or the same as
                                            ///  number of samples for normal AA; Set it to the
                                            ///  number of fragments for EQAA
} ADDR2_COMPUTE_PIPEBANKXOR_INPUT;

/**
****************************************************************************************************
*   ADDR2_COMPUTE_PIPEBANKXOR_OUTPUT
*
*   @brief
*       Output structure of Addr2ComputePipebankXor
****************************************************************************************************
*/
typedef struct _ADDR2_COMPUTE_PIPEBANKXOR_OUTPUT
{
    UINT_32             size;               ///< Size of this structure in bytes
    UINT_32             pipeBankXor;        ///< Pipe bank xor
} ADDR2_COMPUTE_PIPEBANKXOR_OUTPUT;

/**
****************************************************************************************************
*   Addr2ComputePipeBankXor
*
*   @brief
*       Calculate a valid bank pipe xor value for client to use.
****************************************************************************************************
*/
ADDR_E_RETURNCODE ADDR_API Addr2ComputePipeBankXor(
    ADDR_HANDLE                            hLib,
    const ADDR2_COMPUTE_PIPEBANKXOR_INPUT* pIn,
    ADDR2_COMPUTE_PIPEBANKXOR_OUTPUT*      pOut);

/**
****************************************************************************************************
*   ADDR2_COMPUTE_SLICE_PIPEBANKXOR_INPUT
*
*   @brief
*       Input structure of Addr2ComputeSlicePipeBankXor
****************************************************************************************************
*/
typedef struct _ADDR2_COMPUTE_SLICE_PIPEBANKXOR_INPUT
{
    UINT_32             size;               ///< Size of this structure in bytes
    AddrSwizzleMode     swizzleMode;        ///< Surface swizzle mode
    AddrResourceType    resourceType;       ///< Surface resource type
    UINT_32             bpe;                ///< bits per element (e.g. block size for BCn format)
    UINT_32             basePipeBankXor;    ///< Base pipe bank xor
    UINT_32             slice;              ///< Slice id
    UINT_32             numSamples;         ///< Number of samples
} ADDR2_COMPUTE_SLICE_PIPEBANKXOR_INPUT;

/**
****************************************************************************************************
*   ADDR2_COMPUTE_SLICE_PIPEBANKXOR_OUTPUT
*
*   @brief
*       Output structure of Addr2ComputeSlicePipeBankXor
****************************************************************************************************
*/
typedef struct _ADDR2_COMPUTE_SLICE_PIPEBANKXOR_OUTPUT
{
    UINT_32             size;               ///< Size of this structure in bytes
    UINT_32             pipeBankXor;        ///< Pipe bank xor
} ADDR2_COMPUTE_SLICE_PIPEBANKXOR_OUTPUT;

/**
****************************************************************************************************
*   Addr2ComputeSlicePipeBankXor
*
*   @brief
*       Calculate slice pipe bank xor value based on base pipe bank xor and slice id.
****************************************************************************************************
*/
ADDR_E_RETURNCODE ADDR_API Addr2ComputeSlicePipeBankXor(
    ADDR_HANDLE                                  hLib,
    const ADDR2_COMPUTE_SLICE_PIPEBANKXOR_INPUT* pIn,
    ADDR2_COMPUTE_SLICE_PIPEBANKXOR_OUTPUT*      pOut);

/**
****************************************************************************************************
*   ADDR2_COMPUTE_SUBRESOURCE_OFFSET_FORSWIZZLEPATTERN_INPUT
*
*   @brief
*       Input structure of Addr2ComputeSubResourceOffsetForSwizzlePattern
****************************************************************************************************
*/
typedef struct _ADDR2_COMPUTE_SUBRESOURCE_OFFSET_FORSWIZZLEPATTERN_INPUT
{
    UINT_32             size;               ///< Size of this structure in bytes
    AddrSwizzleMode     swizzleMode;        ///< Surface swizzle mode
    AddrResourceType    resourceType;       ///< Surface resource type
    UINT_32             pipeBankXor;        ///< Per resource xor
    UINT_32             slice;              ///< Slice id
    UINT_64             sliceSize;          ///< Slice size of a mip chain
    UINT_64             macroBlockOffset;   ///< Macro block offset, returned in ADDR2_MIP_INFO
    UINT_32             mipTailOffset;      ///< Mip tail offset, returned in ADDR2_MIP_INFO
} ADDR2_COMPUTE_SUBRESOURCE_OFFSET_FORSWIZZLEPATTERN_INPUT;

/**
****************************************************************************************************
*   ADDR2_COMPUTE_SUBRESOURCE_OFFSET_FORSWIZZLEPATTERN_OUTPUT
*
*   @brief
*       Output structure of Addr2ComputeSubResourceOffsetForSwizzlePattern
****************************************************************************************************
*/
typedef struct _ADDR2_COMPUTE_SUBRESOURCE_OFFSET_FORSWIZZLEPATTERN_OUTPUT
{
    UINT_32             size;               ///< Size of this structure in bytes
    UINT_64             offset;             ///< offset
} ADDR2_COMPUTE_SUBRESOURCE_OFFSET_FORSWIZZLEPATTERN_OUTPUT;

/**
****************************************************************************************************
*   Addr2ComputeSubResourceOffsetForSwizzlePattern
*
*   @brief
*       Calculate sub resource offset to support swizzle pattern.
****************************************************************************************************
*/
ADDR_E_RETURNCODE ADDR_API Addr2ComputeSubResourceOffsetForSwizzlePattern(
    ADDR_HANDLE                                                     hLib,
    const ADDR2_COMPUTE_SUBRESOURCE_OFFSET_FORSWIZZLEPATTERN_INPUT* pIn,
    ADDR2_COMPUTE_SUBRESOURCE_OFFSET_FORSWIZZLEPATTERN_OUTPUT*      pOut);

/**
****************************************************************************************************
*   ADDR2_COMPUTE_NONBLOCKCOMPRESSEDVIEW_INPUT
*
*   @brief
*       Input structure of Addr2ComputeNonBlockCompressedView
****************************************************************************************************
*/
typedef struct _ADDR2_COMPUTE_NONBLOCKCOMPRESSEDVIEW_INPUT
{
    UINT_32               size;              ///< Size of this structure in bytes
    ADDR2_SURFACE_FLAGS   flags;             ///< Surface flags
    AddrSwizzleMode       swizzleMode;       ///< Swizzle Mode for Gfx9
    AddrResourceType      resourceType;      ///< Surface type
    AddrFormat            format;            ///< Surface format
    UINT_32               width;             ///< Width of mip0 in texels (not in compressed block)
    UINT_32               height;            ///< Height of mip0 in texels (not in compressed block) 
    UINT_32               numSlices;         ///< Number surface slice/depth of mip0
    UINT_32               numMipLevels;      ///< Total mipmap levels.
    UINT_32               pipeBankXor;       ///< Combined swizzle used to do bank/pipe rotation
    UINT_32               slice;             ///< Index of slice to view
    UINT_32               mipId;             ///< Id of mip to view
} ADDR2_COMPUTE_NONBLOCKCOMPRESSEDVIEW_INPUT;

/**
****************************************************************************************************
*   ADDR2_COMPUTE_NONBLOCKCOMPRESSEDVIEW_OUTPUT
*
*   @brief
*       Output structure of Addr2ComputeNonBlockCompressedView
****************************************************************************************************
*/
typedef struct _ADDR2_COMPUTE_NONBLOCKCOMPRESSEDVIEW_OUTPUT
{
    UINT_32             size;               ///< Size of this structure in bytes
    UINT_64             offset;             ///< Offset shifted from resource base for the view
    UINT_32             pipeBankXor;        ///< Pipe bank xor for the view
    UINT_32             unalignedWidth;     ///< Mip0 width (in element) for the view
    UINT_32             unalignedHeight;    ///< Mip0 height (in element) for the view
    UINT_32             numMipLevels;       ///< Total mipmap levels for the view
    UINT_32             mipId;              ///< Mip ID for the view
} ADDR2_COMPUTE_NONBLOCKCOMPRESSEDVIEW_OUTPUT;

/**
****************************************************************************************************
*   Addr2ComputeNonBlockCompressedView
*
*   @brief
*       Compute non-block-compressed view for a given mipmap level/slice
****************************************************************************************************
*/
ADDR_E_RETURNCODE ADDR_API Addr2ComputeNonBlockCompressedView(
    ADDR_HANDLE                                       hLib,
    const ADDR2_COMPUTE_NONBLOCKCOMPRESSEDVIEW_INPUT* pIn,
    ADDR2_COMPUTE_NONBLOCKCOMPRESSEDVIEW_OUTPUT*      pOut);

/**
****************************************************************************************************
*   ADDR2_BLOCK_SET
*
*   @brief
*       Bit field that defines block type
****************************************************************************************************
*/
typedef union _ADDR2_BLOCK_SET
{
    struct
    {
        UINT_32 micro          : 1;   // 256B block for 2D resource
        UINT_32 macroThin4KB   : 1;   // Thin 4KB for 2D/3D resource
        UINT_32 macroThick4KB  : 1;   // Thick 4KB for 3D resource
        UINT_32 macroThin64KB  : 1;   // Thin 64KB for 2D/3D resource
        UINT_32 macroThick64KB : 1;   // Thick 64KB for 3D resource
        UINT_32 var            : 1;   // VAR block
        UINT_32                : 1;
        UINT_32 linear         : 1;   // Linear block
        UINT_32 reserved       : 24;
    };

    struct
    {
        UINT_32                : 5;
        UINT_32 thin256KB      : 1;   // Thin 256KB block
        UINT_32 thick256KB     : 1;   // Thick 256KB block
        UINT_32                : 25;
    } gfx11;

    UINT_32 value;
} ADDR2_BLOCK_SET;

/**
****************************************************************************************************
*   ADDR2_SWTYPE_SET
*
*   @brief
*       Bit field that defines swizzle type
****************************************************************************************************
*/
typedef union _ADDR2_SWTYPE_SET
{
    struct
    {
        UINT_32 sw_Z     : 1;   // SW_*_Z_*
        UINT_32 sw_S     : 1;   // SW_*_S_*
        UINT_32 sw_D     : 1;   // SW_*_D_*
        UINT_32 sw_R     : 1;   // SW_*_R_*
        UINT_32 reserved : 28;
    };

    UINT_32 value;
} ADDR2_SWTYPE_SET;

/**
****************************************************************************************************
*   ADDR2_SWMODE_SET
*
*   @brief
*       Bit field that defines swizzle type
****************************************************************************************************
*/
typedef union _ADDR2_SWMODE_SET
{
    struct
    {
        UINT_32 swLinear    : 1;
        UINT_32 sw256B_S    : 1;
        UINT_32 sw256B_D    : 1;
        UINT_32 sw256B_R    : 1;
        UINT_32 sw4KB_Z     : 1;
        UINT_32 sw4KB_S     : 1;
        UINT_32 sw4KB_D     : 1;
        UINT_32 sw4KB_R     : 1;
        UINT_32 sw64KB_Z    : 1;
        UINT_32 sw64KB_S    : 1;
        UINT_32 sw64KB_D    : 1;
        UINT_32 sw64KB_R    : 1;
        UINT_32 swMiscDef12 : 1;
        UINT_32 swMiscDef13 : 1;
        UINT_32 swMiscDef14 : 1;
        UINT_32 swMiscDef15 : 1;
        UINT_32 sw64KB_Z_T  : 1;
        UINT_32 sw64KB_S_T  : 1;
        UINT_32 sw64KB_D_T  : 1;
        UINT_32 sw64KB_R_T  : 1;
        UINT_32 sw4KB_Z_X   : 1;
        UINT_32 sw4KB_S_X   : 1;
        UINT_32 sw4KB_D_X   : 1;
        UINT_32 sw4KB_R_X   : 1;
        UINT_32 sw64KB_Z_X  : 1;
        UINT_32 sw64KB_S_X  : 1;
        UINT_32 sw64KB_D_X  : 1;
        UINT_32 sw64KB_R_X  : 1;
        UINT_32 swMiscDef28 : 1;
        UINT_32 swMiscDef29 : 1;
        UINT_32 swMiscDef30 : 1;
        UINT_32 swMiscDef31 : 1;
    };

    struct
    {
        UINT_32             : 28;
        UINT_32 swVar_Z_X   : 1;
        UINT_32             : 2;
        UINT_32 swVar_R_X   : 1;
    } gfx10;

    struct
    {
        UINT_32             : 28;
        UINT_32 sw256KB_Z_X : 1;
        UINT_32 sw256KB_S_X : 1;
        UINT_32 sw256KB_D_X : 1;
        UINT_32 sw256KB_R_X : 1;
    } gfx11;

    UINT_32 value;
} ADDR2_SWMODE_SET;

/**
****************************************************************************************************
*   ADDR2_GET_PREFERRED_SURF_SETTING_INPUT
*
*   @brief
*       Input structure of Addr2GetPreferredSurfaceSetting
****************************************************************************************************
*/
typedef struct _ADDR2_GET_PREFERRED_SURF_SETTING_INPUT
{
    UINT_32               size;              ///< Size of this structure in bytes

    ADDR2_SURFACE_FLAGS   flags;             ///< Surface flags
    AddrResourceType      resourceType;      ///< Surface type
    AddrFormat            format;            ///< Surface format
    AddrResrouceLocation  resourceLoction;   ///< Surface heap choice
    ADDR2_BLOCK_SET       forbiddenBlock;    ///< Client can use it to disable some block setting
                                             ///< such as linear for DXTn, tiled for YUV
    ADDR2_SWTYPE_SET      preferredSwSet;    ///< Client can use it to specify sw type(s) wanted
    BOOL_32               noXor;             ///< Do not use xor mode for this resource
    UINT_32               bpp;               ///< bits per pixel
    UINT_32               width;             ///< Width (of mip0), in pixels
    UINT_32               height;            ///< Height (of mip0), in pixels
    UINT_32               numSlices;         ///< Number surface slice/depth (of mip0),
    UINT_32               numMipLevels;      ///< Total mipmap levels.
    UINT_32               numSamples;        ///< Number of samples
    UINT_32               numFrags;          ///< Number of fragments, leave it zero or the same as
                                             ///  number of samples for normal AA; Set it to the
                                             ///  number of fragments for EQAA
    UINT_32               maxAlign;          ///< maximum base/size alignment requested by client
    UINT_32               minSizeAlign;      ///< memory allocated for surface in client driver will
                                             ///  be padded to multiple of this value (in bytes)
    DOUBLE                memoryBudget;      ///< Memory consumption ratio based on minimum possible
                                             ///  size.
} ADDR2_GET_PREFERRED_SURF_SETTING_INPUT;

/**
****************************************************************************************************
*   ADDR2_GET_PREFERRED_SURF_SETTING_OUTPUT
*
*   @brief
*       Output structure of Addr2GetPreferredSurfaceSetting
****************************************************************************************************
*/
typedef struct _ADDR2_GET_PREFERRED_SURF_SETTING_OUTPUT
{
    UINT_32               size;                 ///< Size of this structure in bytes

    AddrSwizzleMode       swizzleMode;          ///< Suggested swizzle mode to be used
    AddrResourceType      resourceType;         ///< Suggested resource type to program HW
    ADDR2_BLOCK_SET       validBlockSet;        ///< Valid block type bit conbination
    BOOL_32               canXor;               ///< If client can use xor on a valid macro block
                                                ///  type
    ADDR2_SWTYPE_SET      validSwTypeSet;       ///< Valid swizzle type bit combination
    ADDR2_SWTYPE_SET      clientPreferredSwSet; ///< Client-preferred swizzle type bit combination
    ADDR2_SWMODE_SET      validSwModeSet;       ///< Valid swizzle mode bit combination
} ADDR2_GET_PREFERRED_SURF_SETTING_OUTPUT;

/**
****************************************************************************************************
*   Addr2GetPreferredSurfaceSetting
*
*   @brief
*       Suggest a preferred setting for client driver to program HW register
****************************************************************************************************
*/
ADDR_E_RETURNCODE ADDR_API Addr2GetPreferredSurfaceSetting(
    ADDR_HANDLE                                   hLib,
    const ADDR2_GET_PREFERRED_SURF_SETTING_INPUT* pIn,
    ADDR2_GET_PREFERRED_SURF_SETTING_OUTPUT*      pOut);

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
    ADDR_HANDLE                                   hLib,
    const ADDR2_GET_PREFERRED_SURF_SETTING_INPUT* pIn,
    ADDR2_GET_PREFERRED_SURF_SETTING_OUTPUT*      pOut);

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
    BOOL_32         *pResult);

/**
****************************************************************************************************
*   Addr2GetAllowedBlockSet
*
*   @brief
*       Returns the set of allowed block sizes given the allowed swizzle modes and resource type
****************************************************************************************************
*/
ADDR_E_RETURNCODE ADDR_API Addr2GetAllowedBlockSet(
    ADDR_HANDLE      hLib,
    ADDR2_SWMODE_SET allowedSwModeSet,
    AddrResourceType rsrcType,
    ADDR2_BLOCK_SET* pAllowedBlockSet);

/**
****************************************************************************************************
*   Addr2GetAllowedSwSet
*
*   @brief
*       Returns the set of allowed swizzle types given the allowed swizzle modes
****************************************************************************************************
*/
ADDR_E_RETURNCODE ADDR_API Addr2GetAllowedSwSet(
    ADDR_HANDLE       hLib,
    ADDR2_SWMODE_SET  allowedSwModeSet,
    ADDR2_SWTYPE_SET* pAllowedSwSet);

/**
****************************************************************************************************
*   Addr2IsBlockTypeAvailable
*
*   @brief
*       Determine whether a block type is allowed in a given blockSet
****************************************************************************************************
*/
BOOL_32 Addr2IsBlockTypeAvailable(ADDR2_BLOCK_SET blockSet, AddrBlockType blockType);

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
#if defined(__cplusplus)
    DOUBLE  memoryBudget = 0.0f,
    BOOL_32 newBlockTypeBigger = TRUE);
#else
    DOUBLE  memoryBudget,
    BOOL_32 newBlockTypeBigger);
#endif

#if defined(__cplusplus)
}
#endif

#endif // __ADDR_INTERFACE_H__
