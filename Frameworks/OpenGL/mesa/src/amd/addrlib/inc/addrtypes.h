/*
************************************************************************************************************************
*
*  Copyright (C) 2007-2022 Advanced Micro Devices, Inc.  All rights reserved.
*  SPDX-License-Identifier: MIT
*
***********************************************************************************************************************/

/**
****************************************************************************************************
* @file  addrtypes.h
* @brief Contains the helper function and constants
****************************************************************************************************
*/
#ifndef __ADDR_TYPES_H__
#define __ADDR_TYPES_H__

#if defined(__APPLE__) && !defined(HAVE_TSERVER)
// External definitions header maintained by Apple driver team, but not for diag team under Mac.
// Helps address compilation issues & reduces code covered by NDA
#include "addrExtDef.h"

#else

// Windows and/or Linux
#if !defined(VOID)
typedef void           VOID;
#endif

#if !defined(FLOAT)
typedef float          FLOAT;
#endif

#if !defined(DOUBLE)
typedef double         DOUBLE;
#endif

#if !defined(CHAR)
typedef char           CHAR;
#endif

#if !defined(INT)
typedef int            INT;
#endif

#include <stdarg.h> // va_list...etc need this header

#endif // defined (__APPLE__) && !defined(HAVE_TSERVER)

/**
****************************************************************************************************
*   Calling conventions
****************************************************************************************************
*/
#ifndef ADDR_CDECL
    #if defined(__GNUC__)
        #if defined(__i386__)
            #define ADDR_CDECL __attribute__((cdecl))
        #else
            #define ADDR_CDECL
        #endif
    #else
        #define ADDR_CDECL __cdecl
    #endif
#endif

#ifndef ADDR_STDCALL
    #if defined(__GNUC__)
        #if defined(__i386__)
            #define ADDR_STDCALL __attribute__((stdcall))
        #else
            #define ADDR_STDCALL
        #endif
    #else
        #define ADDR_STDCALL __stdcall
    #endif
#endif

#ifndef ADDR_FASTCALL
    #if defined(__GNUC__)
        #if defined(__i386__) || defined(__amd64__) || defined(__x86_64__)
            #define ADDR_FASTCALL __attribute__((regparm(0)))
        #else
            #define ADDR_FASTCALL
        #endif
    #else
        #define ADDR_FASTCALL __fastcall
    #endif
#endif

#ifndef GC_CDECL
    #define GC_CDECL  ADDR_CDECL
#endif

#ifndef GC_STDCALL
    #define GC_STDCALL  ADDR_STDCALL
#endif

#ifndef GC_FASTCALL
    #define GC_FASTCALL  ADDR_FASTCALL
#endif


#if defined(__GNUC__)
    #define ADDR_INLINE static inline   // inline needs to be static to link
#else
    // win32, win64, other platforms
    #define ADDR_INLINE   __inline
#endif // #if defined(__GNUC__)

#define ADDR_API ADDR_FASTCALL //default call convention is fast call

/**
****************************************************************************************************
* Global defines used by other modules
****************************************************************************************************
*/
#if !defined(TILEINDEX_INVALID)
#define TILEINDEX_INVALID                -1
#endif

#if !defined(TILEINDEX_LINEAR_GENERAL)
#define TILEINDEX_LINEAR_GENERAL         -2
#endif

#if !defined(TILEINDEX_LINEAR_ALIGNED)
#define TILEINDEX_LINEAR_ALIGNED          8
#endif

/**
****************************************************************************************************
* Return codes
****************************************************************************************************
*/
typedef enum _ADDR_E_RETURNCODE
{
    // General Return
    ADDR_OK    = 0,
    ADDR_ERROR = 1,

    // Specific Errors
    ADDR_OUTOFMEMORY,
    ADDR_INVALIDPARAMS,
    ADDR_NOTSUPPORTED,
    ADDR_NOTIMPLEMENTED,
    ADDR_PARAMSIZEMISMATCH,
    ADDR_INVALIDGBREGVALUES,

} ADDR_E_RETURNCODE;

/**
****************************************************************************************************
* @brief
*   Neutral enums that define tile modes for all H/W
* @note
*   R600/R800 tiling mode can be cast to hw enums directly but never cast into HW enum from
*   ADDR_TM_2D_TILED_XTHICK
*
****************************************************************************************************
*/
typedef enum _AddrTileMode
{
    ADDR_TM_LINEAR_GENERAL      = 0,    ///< Least restrictions, pitch: multiple of 8 if not buffer
    ADDR_TM_LINEAR_ALIGNED      = 1,    ///< Requests pitch or slice to be multiple of 64 pixels
    ADDR_TM_1D_TILED_THIN1      = 2,    ///< Linear array of 8x8 tiles
    ADDR_TM_1D_TILED_THICK      = 3,    ///< Linear array of 8x8x4 tiles
    ADDR_TM_2D_TILED_THIN1      = 4,    ///< A set of macro tiles consist of 8x8 tiles
    ADDR_TM_2D_TILED_THIN2      = 5,    ///< 600 HWL only, macro tile ratio is 1:4
    ADDR_TM_2D_TILED_THIN4      = 6,    ///< 600 HWL only, macro tile ratio is 1:16
    ADDR_TM_2D_TILED_THICK      = 7,    ///< A set of macro tiles consist of 8x8x4 tiles
    ADDR_TM_2B_TILED_THIN1      = 8,    ///< 600 HWL only, with bank swap
    ADDR_TM_2B_TILED_THIN2      = 9,    ///< 600 HWL only, with bank swap and ratio is 1:4
    ADDR_TM_2B_TILED_THIN4      = 10,   ///< 600 HWL only, with bank swap and ratio is 1:16
    ADDR_TM_2B_TILED_THICK      = 11,   ///< 600 HWL only, with bank swap, consists of 8x8x4 tiles
    ADDR_TM_3D_TILED_THIN1      = 12,   ///< Macro tiling w/ pipe rotation between slices
    ADDR_TM_3D_TILED_THICK      = 13,   ///< Macro tiling w/ pipe rotation bwtween slices, thick
    ADDR_TM_3B_TILED_THIN1      = 14,   ///< 600 HWL only, with bank swap
    ADDR_TM_3B_TILED_THICK      = 15,   ///< 600 HWL only, with bank swap, thick
    ADDR_TM_2D_TILED_XTHICK     = 16,   ///< Tile is 8x8x8, valid from NI
    ADDR_TM_3D_TILED_XTHICK     = 17,   ///< Tile is 8x8x8, valid from NI
    ADDR_TM_POWER_SAVE          = 18,   ///< Power save mode, only used by KMD on NI
    ADDR_TM_PRT_TILED_THIN1     = 19,   ///< No bank/pipe rotation or hashing beyond macrotile size
    ADDR_TM_PRT_2D_TILED_THIN1  = 20,   ///< Same as 2D_TILED_THIN1, PRT only
    ADDR_TM_PRT_3D_TILED_THIN1  = 21,   ///< Same as 3D_TILED_THIN1, PRT only
    ADDR_TM_PRT_TILED_THICK     = 22,   ///< No bank/pipe rotation or hashing beyond macrotile size
    ADDR_TM_PRT_2D_TILED_THICK  = 23,   ///< Same as 2D_TILED_THICK, PRT only
    ADDR_TM_PRT_3D_TILED_THICK  = 24,   ///< Same as 3D_TILED_THICK, PRT only
    ADDR_TM_UNKNOWN             = 25,   ///< Unkown tile mode, should be decided by address lib
    ADDR_TM_COUNT               = 26,   ///< Must be the value of the last tile mode
} AddrTileMode;

/**
****************************************************************************************************
* @brief
*   Neutral enums that define swizzle modes for Gfx9+ ASIC
* @note
*
*   ADDR_SW_LINEAR linear aligned addressing mode, for 1D/2D/3D resource
*   ADDR_SW_256B_* addressing block aligned size is 256B, for 2D resource
*   ADDR_SW_4KB_*  addressing block aligned size is 4KB, for 2D/3D resource
*   ADDR_SW_64KB_* addressing block aligned size is 64KB, for 1D/2D/3D resource
*   ADDR_SW_VAR_*  addressing block aligned size is ASIC specific
*
*   ADDR_SW_*_Z    For GFX9:
                   - for 2D resource, represents Z-order swizzle mode for depth/stencil/FMask
                   - for 3D resource, represents a swizzle mode similar to legacy thick tile mode
                   For GFX10:
                   - represents Z-order swizzle mode for depth/stencil/FMask
*   ADDR_SW_*_S    For GFX9+:
                   - represents standard swizzle mode defined by MS
*   ADDR_SW_*_D    For GFX9:
                   - for 2D resource, represents a swizzle mode for displayable resource
*                  - for 3D resource, represents a swizzle mode which places each slice in order & pixel
                   For GFX10:
                   - for 2D resource, represents a swizzle mode for displayable resource
                   - for 3D resource, represents a swizzle mode similar to legacy thick tile mode
                   within slice is placed as 2D ADDR_SW_*_S. Don't use this combination if possible!
*   ADDR_SW_*_R    For GFX9:
                   - 2D resource only, represents a swizzle mode for rotated displayable resource
                   For GFX10:
                   - represents a swizzle mode for render target resource
*
****************************************************************************************************
*/
typedef enum _AddrSwizzleMode
{
    ADDR_SW_LINEAR          = 0,
    ADDR_SW_256B_S          = 1,
    ADDR_SW_256B_D          = 2,
    ADDR_SW_256B_R          = 3,
    ADDR_SW_4KB_Z           = 4,
    ADDR_SW_4KB_S           = 5,
    ADDR_SW_4KB_D           = 6,
    ADDR_SW_4KB_R           = 7,
    ADDR_SW_64KB_Z          = 8,
    ADDR_SW_64KB_S          = 9,
    ADDR_SW_64KB_D          = 10,
    ADDR_SW_64KB_R          = 11,
    ADDR_SW_MISCDEF12       = 12,
    ADDR_SW_MISCDEF13       = 13,
    ADDR_SW_MISCDEF14       = 14,
    ADDR_SW_MISCDEF15       = 15,
    ADDR_SW_64KB_Z_T        = 16,
    ADDR_SW_64KB_S_T        = 17,
    ADDR_SW_64KB_D_T        = 18,
    ADDR_SW_64KB_R_T        = 19,
    ADDR_SW_4KB_Z_X         = 20,
    ADDR_SW_4KB_S_X         = 21,
    ADDR_SW_4KB_D_X         = 22,
    ADDR_SW_4KB_R_X         = 23,
    ADDR_SW_64KB_Z_X        = 24,
    ADDR_SW_64KB_S_X        = 25,
    ADDR_SW_64KB_D_X        = 26,
    ADDR_SW_64KB_R_X        = 27,
    ADDR_SW_MISCDEF28       = 28,
    ADDR_SW_MISCDEF29       = 29,
    ADDR_SW_MISCDEF30       = 30,
    ADDR_SW_MISCDEF31       = 31,
    ADDR_SW_LINEAR_GENERAL  = 32,
    ADDR_SW_MAX_TYPE        = 33,

    ADDR_SW_RESERVED0       = ADDR_SW_MISCDEF12,
    ADDR_SW_RESERVED1       = ADDR_SW_MISCDEF13,
    ADDR_SW_RESERVED2       = ADDR_SW_MISCDEF14,
    ADDR_SW_RESERVED3       = ADDR_SW_MISCDEF15,
    ADDR_SW_RESERVED4       = ADDR_SW_MISCDEF29,
    ADDR_SW_RESERVED5       = ADDR_SW_MISCDEF30,

    ADDR_SW_VAR_Z_X         = ADDR_SW_MISCDEF28,
    ADDR_SW_VAR_R_X         = ADDR_SW_MISCDEF31,

    ADDR_SW_256KB_Z_X       = ADDR_SW_MISCDEF28,
    ADDR_SW_256KB_S_X       = ADDR_SW_MISCDEF29,
    ADDR_SW_256KB_D_X       = ADDR_SW_MISCDEF30,
    ADDR_SW_256KB_R_X       = ADDR_SW_MISCDEF31,
} AddrSwizzleMode;

/**
****************************************************************************************************
* @brief
*   Neutral enums that define image type
* @note
*   this is new for address library interface version 2
*
****************************************************************************************************
*/
typedef enum _AddrResourceType
{
    ADDR_RSRC_TEX_1D = 0,
    ADDR_RSRC_TEX_2D = 1,
    ADDR_RSRC_TEX_3D = 2,
    ADDR_RSRC_MAX_TYPE = 3,
} AddrResourceType;

/**
****************************************************************************************************
* @brief
*   Neutral enums that define resource heap location
* @note
*   this is new for address library interface version 2
*
****************************************************************************************************
*/
typedef enum _AddrResrouceLocation
{
    ADDR_RSRC_LOC_UNDEF  = 0,   // Resource heap is undefined/unknown
    ADDR_RSRC_LOC_LOCAL  = 1,   // CPU visable and CPU invisable local heap
    ADDR_RSRC_LOC_USWC   = 2,   // CPU write-combined non-cached nonlocal heap
    ADDR_RSRC_LOC_CACHED = 3,   // CPU cached nonlocal heap
    ADDR_RSRC_LOC_INVIS  = 4,   // CPU invisable local heap only
    ADDR_RSRC_LOC_MAX_TYPE = 5,
} AddrResrouceLocation;

/**
****************************************************************************************************
* @brief
*   Neutral enums that define resource basic swizzle mode
* @note
*   this is new for address library interface version 2
*
****************************************************************************************************
*/
typedef enum _AddrSwType
{
    ADDR_SW_Z  = 0,   // Resource basic swizzle mode is ZOrder
    ADDR_SW_S  = 1,   // Resource basic swizzle mode is Standard
    ADDR_SW_D  = 2,   // Resource basic swizzle mode is Display
    ADDR_SW_R  = 3,   // Resource basic swizzle mode is Rotated/Render optimized
    ADDR_SW_L  = 4,   // Resource basic swizzle mode is Linear
    ADDR_SW_MAX_SWTYPE
} AddrSwType;

/**
****************************************************************************************************
* @brief
*   Neutral enums that define mipmap major mode
* @note
*   this is new for address library interface version 2
*
****************************************************************************************************
*/
typedef enum _AddrMajorMode
{
    ADDR_MAJOR_X = 0,
    ADDR_MAJOR_Y = 1,
    ADDR_MAJOR_Z = 2,
    ADDR_MAJOR_MAX_TYPE = 3,
} AddrMajorMode;

/**
****************************************************************************************************
*   AddrFormat
*
*   @brief
*       Neutral enum for SurfaceFormat
*
****************************************************************************************************
*/
typedef enum _AddrFormat {
    ADDR_FMT_INVALID                              = 0x00000000,
    ADDR_FMT_8                                    = 0x00000001,
    ADDR_FMT_4_4                                  = 0x00000002,
    ADDR_FMT_3_3_2                                = 0x00000003,
    ADDR_FMT_RESERVED_4                           = 0x00000004,
    ADDR_FMT_16                                   = 0x00000005,
    ADDR_FMT_16_FLOAT                             = ADDR_FMT_16,
    ADDR_FMT_8_8                                  = 0x00000007,
    ADDR_FMT_5_6_5                                = 0x00000008,
    ADDR_FMT_6_5_5                                = 0x00000009,
    ADDR_FMT_1_5_5_5                              = 0x0000000a,
    ADDR_FMT_4_4_4_4                              = 0x0000000b,
    ADDR_FMT_5_5_5_1                              = 0x0000000c,
    ADDR_FMT_32                                   = 0x0000000d,
    ADDR_FMT_32_FLOAT                             = ADDR_FMT_32,
    ADDR_FMT_16_16                                = 0x0000000f,
    ADDR_FMT_16_16_FLOAT                          = ADDR_FMT_16_16,
    ADDR_FMT_8_24                                 = 0x00000011,
    ADDR_FMT_8_24_FLOAT                           = ADDR_FMT_8_24,
    ADDR_FMT_24_8                                 = 0x00000013,
    ADDR_FMT_24_8_FLOAT                           = ADDR_FMT_24_8,
    ADDR_FMT_10_11_11                             = 0x00000015,
    ADDR_FMT_10_11_11_FLOAT                       = ADDR_FMT_10_11_11,
    ADDR_FMT_11_11_10                             = 0x00000017,
    ADDR_FMT_11_11_10_FLOAT                       = ADDR_FMT_11_11_10,
    ADDR_FMT_2_10_10_10                           = 0x00000019,
    ADDR_FMT_8_8_8_8                              = 0x0000001a,
    ADDR_FMT_10_10_10_2                           = 0x0000001b,
    ADDR_FMT_X24_8_32_FLOAT                       = 0x0000001c,
    ADDR_FMT_32_32                                = 0x0000001d,
    ADDR_FMT_32_32_FLOAT                          = ADDR_FMT_32_32,
    ADDR_FMT_16_16_16_16                          = 0x0000001f,
    ADDR_FMT_16_16_16_16_FLOAT                    = ADDR_FMT_16_16_16_16,
    ADDR_FMT_RESERVED_33                          = 0x00000021,
    ADDR_FMT_32_32_32_32                          = 0x00000022,
    ADDR_FMT_32_32_32_32_FLOAT                    = ADDR_FMT_32_32_32_32,
    ADDR_FMT_RESERVED_36                          = 0x00000024,
    ADDR_FMT_1                                    = 0x00000025,
    ADDR_FMT_1_REVERSED                           = 0x00000026,
    ADDR_FMT_GB_GR                                = 0x00000027,
    ADDR_FMT_BG_RG                                = 0x00000028,
    ADDR_FMT_32_AS_8                              = 0x00000029,
    ADDR_FMT_32_AS_8_8                            = 0x0000002a,
    ADDR_FMT_5_9_9_9_SHAREDEXP                    = 0x0000002b,
    ADDR_FMT_8_8_8                                = 0x0000002c,
    ADDR_FMT_16_16_16                             = 0x0000002d,
    ADDR_FMT_16_16_16_FLOAT                       = ADDR_FMT_16_16_16,
    ADDR_FMT_32_32_32                             = 0x0000002f,
    ADDR_FMT_32_32_32_FLOAT                       = ADDR_FMT_32_32_32,
    ADDR_FMT_BC1                                  = 0x00000031,
    ADDR_FMT_BC2                                  = 0x00000032,
    ADDR_FMT_BC3                                  = 0x00000033,
    ADDR_FMT_BC4                                  = 0x00000034,
    ADDR_FMT_BC5                                  = 0x00000035,
    ADDR_FMT_BC6                                  = 0x00000036,
    ADDR_FMT_BC7                                  = 0x00000037,
    ADDR_FMT_32_AS_32_32_32_32                    = 0x00000038,
    ADDR_FMT_APC3                                 = 0x00000039,
    ADDR_FMT_APC4                                 = 0x0000003a,
    ADDR_FMT_APC5                                 = 0x0000003b,
    ADDR_FMT_APC6                                 = 0x0000003c,
    ADDR_FMT_APC7                                 = 0x0000003d,
    ADDR_FMT_CTX1                                 = 0x0000003e,
    ADDR_FMT_RESERVED_63                          = 0x0000003f,
    ADDR_FMT_ASTC_4x4                             = 0x00000040,
    ADDR_FMT_ASTC_5x4                             = 0x00000041,
    ADDR_FMT_ASTC_5x5                             = 0x00000042,
    ADDR_FMT_ASTC_6x5                             = 0x00000043,
    ADDR_FMT_ASTC_6x6                             = 0x00000044,
    ADDR_FMT_ASTC_8x5                             = 0x00000045,
    ADDR_FMT_ASTC_8x6                             = 0x00000046,
    ADDR_FMT_ASTC_8x8                             = 0x00000047,
    ADDR_FMT_ASTC_10x5                            = 0x00000048,
    ADDR_FMT_ASTC_10x6                            = 0x00000049,
    ADDR_FMT_ASTC_10x8                            = 0x0000004a,
    ADDR_FMT_ASTC_10x10                           = 0x0000004b,
    ADDR_FMT_ASTC_12x10                           = 0x0000004c,
    ADDR_FMT_ASTC_12x12                           = 0x0000004d,
    ADDR_FMT_ETC2_64BPP                           = 0x0000004e,
    ADDR_FMT_ETC2_128BPP                          = 0x0000004f,
    ADDR_FMT_BG_RG_16_16_16_16                    = 0x00000050,
} AddrFormat;

/**
****************************************************************************************************
*   AddrDepthFormat
*
*   @brief
*       Neutral enum for addrFlt32ToDepthPixel
*
****************************************************************************************************
*/
typedef enum _AddrDepthFormat
{
    ADDR_DEPTH_INVALID                            = 0x00000000,
    ADDR_DEPTH_16                                 = 0x00000001,
    ADDR_DEPTH_X8_24                              = 0x00000002,
    ADDR_DEPTH_8_24                               = 0x00000003,
    ADDR_DEPTH_X8_24_FLOAT                        = 0x00000004,
    ADDR_DEPTH_8_24_FLOAT                         = 0x00000005,
    ADDR_DEPTH_32_FLOAT                           = 0x00000006,
    ADDR_DEPTH_X24_8_32_FLOAT                     = 0x00000007,

} AddrDepthFormat;

/**
****************************************************************************************************
*   AddrColorFormat
*
*   @brief
*       Neutral enum for ColorFormat
*
****************************************************************************************************
*/
typedef enum _AddrColorFormat
{
    ADDR_COLOR_INVALID                            = 0x00000000,
    ADDR_COLOR_8                                  = 0x00000001,
    ADDR_COLOR_4_4                                = 0x00000002,
    ADDR_COLOR_3_3_2                              = 0x00000003,
    ADDR_COLOR_RESERVED_4                         = 0x00000004,
    ADDR_COLOR_16                                 = 0x00000005,
    ADDR_COLOR_16_FLOAT                           = 0x00000006,
    ADDR_COLOR_8_8                                = 0x00000007,
    ADDR_COLOR_5_6_5                              = 0x00000008,
    ADDR_COLOR_6_5_5                              = 0x00000009,
    ADDR_COLOR_1_5_5_5                            = 0x0000000a,
    ADDR_COLOR_4_4_4_4                            = 0x0000000b,
    ADDR_COLOR_5_5_5_1                            = 0x0000000c,
    ADDR_COLOR_32                                 = 0x0000000d,
    ADDR_COLOR_32_FLOAT                           = 0x0000000e,
    ADDR_COLOR_16_16                              = 0x0000000f,
    ADDR_COLOR_16_16_FLOAT                        = 0x00000010,
    ADDR_COLOR_8_24                               = 0x00000011,
    ADDR_COLOR_8_24_FLOAT                         = 0x00000012,
    ADDR_COLOR_24_8                               = 0x00000013,
    ADDR_COLOR_24_8_FLOAT                         = 0x00000014,
    ADDR_COLOR_10_11_11                           = 0x00000015,
    ADDR_COLOR_10_11_11_FLOAT                     = 0x00000016,
    ADDR_COLOR_11_11_10                           = 0x00000017,
    ADDR_COLOR_11_11_10_FLOAT                     = 0x00000018,
    ADDR_COLOR_2_10_10_10                         = 0x00000019,
    ADDR_COLOR_8_8_8_8                            = 0x0000001a,
    ADDR_COLOR_10_10_10_2                         = 0x0000001b,
    ADDR_COLOR_X24_8_32_FLOAT                     = 0x0000001c,
    ADDR_COLOR_32_32                              = 0x0000001d,
    ADDR_COLOR_32_32_FLOAT                        = 0x0000001e,
    ADDR_COLOR_16_16_16_16                        = 0x0000001f,
    ADDR_COLOR_16_16_16_16_FLOAT                  = 0x00000020,
    ADDR_COLOR_RESERVED_33                        = 0x00000021,
    ADDR_COLOR_32_32_32_32                        = 0x00000022,
    ADDR_COLOR_32_32_32_32_FLOAT                  = 0x00000023,
} AddrColorFormat;

/**
****************************************************************************************************
*   AddrSurfaceNumber
*
*   @brief
*       Neutral enum for SurfaceNumber
*
****************************************************************************************************
*/
typedef enum _AddrSurfaceNumber {
    ADDR_NUMBER_UNORM                             = 0x00000000,
    ADDR_NUMBER_SNORM                             = 0x00000001,
    ADDR_NUMBER_USCALED                           = 0x00000002,
    ADDR_NUMBER_SSCALED                           = 0x00000003,
    ADDR_NUMBER_UINT                              = 0x00000004,
    ADDR_NUMBER_SINT                              = 0x00000005,
    ADDR_NUMBER_SRGB                              = 0x00000006,
    ADDR_NUMBER_FLOAT                             = 0x00000007,
} AddrSurfaceNumber;

/**
****************************************************************************************************
*   AddrSurfaceSwap
*
*   @brief
*       Neutral enum for SurfaceSwap
*
****************************************************************************************************
*/
typedef enum _AddrSurfaceSwap {
    ADDR_SWAP_STD                                 = 0x00000000,
    ADDR_SWAP_ALT                                 = 0x00000001,
    ADDR_SWAP_STD_REV                             = 0x00000002,
    ADDR_SWAP_ALT_REV                             = 0x00000003,
} AddrSurfaceSwap;

/**
****************************************************************************************************
*   AddrHtileBlockSize
*
*   @brief
*       Size of HTILE blocks, valid values are 4 or 8 for now
****************************************************************************************************
*/
typedef enum _AddrHtileBlockSize
{
    ADDR_HTILE_BLOCKSIZE_4 = 4,
    ADDR_HTILE_BLOCKSIZE_8 = 8,
} AddrHtileBlockSize;


/**
****************************************************************************************************
*   AddrPipeCfg
*
*   @brief
*       The pipe configuration field specifies both the number of pipes and
*       how pipes are interleaved on the surface.
*       The expression of number of pipes, the shader engine tile size, and packer tile size
*       is encoded in a PIPE_CONFIG register field.
*       In general the number of pipes usually matches the number of memory channels of the
*       hardware configuration.
*       For hw configurations w/ non-pow2 memory number of memory channels, it usually matches
*       the number of ROP units(? TODO: which registers??)
*       The enum value = hw enum + 1 which is to reserve 0 for requesting default.
****************************************************************************************************
*/
typedef enum _AddrPipeCfg
{
    ADDR_PIPECFG_INVALID              = 0,
    ADDR_PIPECFG_P2                   = 1, /// 2 pipes,
    ADDR_PIPECFG_P4_8x16              = 5, /// 4 pipes,
    ADDR_PIPECFG_P4_16x16             = 6,
    ADDR_PIPECFG_P4_16x32             = 7,
    ADDR_PIPECFG_P4_32x32             = 8,
    ADDR_PIPECFG_P8_16x16_8x16        = 9, /// 8 pipes
    ADDR_PIPECFG_P8_16x32_8x16        = 10,
    ADDR_PIPECFG_P8_32x32_8x16        = 11,
    ADDR_PIPECFG_P8_16x32_16x16       = 12,
    ADDR_PIPECFG_P8_32x32_16x16       = 13,
    ADDR_PIPECFG_P8_32x32_16x32       = 14,
    ADDR_PIPECFG_P8_32x64_32x32       = 15,
    ADDR_PIPECFG_P16_32x32_8x16       = 17, /// 16 pipes
    ADDR_PIPECFG_P16_32x32_16x16      = 18,
    ADDR_PIPECFG_UNUSED               = 19,
    ADDR_PIPECFG_MAX                  = 20,
} AddrPipeCfg;

/**
****************************************************************************************************
* AddrTileType
*
*   @brief
*       Neutral enums that specifies micro tile type (MICRO_TILE_MODE)
****************************************************************************************************
*/
typedef enum _AddrTileType
{
    ADDR_DISPLAYABLE        = 0,    ///< Displayable tiling
    ADDR_NON_DISPLAYABLE    = 1,    ///< Non-displayable tiling, a.k.a thin micro tiling
    ADDR_DEPTH_SAMPLE_ORDER = 2,    ///< Same as non-displayable plus depth-sample-order
    ADDR_ROTATED            = 3,    ///< Rotated displayable tiling
    ADDR_THICK              = 4,    ///< Thick micro-tiling, only valid for THICK and XTHICK
} AddrTileType;

////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  Type definitions: short system-independent names for address library types
//
////////////////////////////////////////////////////////////////////////////////////////////////////

#if !defined(__APPLE__) || defined(HAVE_TSERVER)

#ifndef BOOL_32        // no bool type in C
/// @brief Boolean type, since none is defined in C
/// @ingroup type
#define BOOL_32 int
#endif

#ifndef INT_32
#define INT_32  int
#endif

#ifndef UINT_32
#define UINT_32 unsigned int
#endif

#ifndef INT_16
#define INT_16  short
#endif

#ifndef UINT_16
#define UINT_16 unsigned short
#endif

#ifndef INT_8
#define INT_8   signed char // signed must be used because of aarch64
#endif

#ifndef UINT_8
#define UINT_8  unsigned char
#endif

#ifndef NULL
#define NULL 0
#endif

#ifndef TRUE
#define TRUE 1
#endif

#ifndef FALSE
#define FALSE 0
#endif

//
//  64-bit integer types depend on the compiler
//
#if defined( __GNUC__ ) || defined( __WATCOMC__ )
#define INT_64   long long
#define UINT_64  unsigned long long

#elif defined( _WIN32 )
#define INT_64   __int64
#define UINT_64  unsigned __int64

#else
#error Unsupported compiler and/or operating system for 64-bit integers

/// @brief 64-bit signed integer type (compiler dependent)
/// @ingroup type
///
/// The addrlib defines a 64-bit signed integer type for either
/// Gnu/Watcom compilers (which use the first syntax) or for
/// the Windows VCC compiler (which uses the second syntax).
#define INT_64  long long OR __int64

/// @brief 64-bit unsigned integer type (compiler dependent)
/// @ingroup type
///
/// The addrlib defines a 64-bit unsigned integer type for either
/// Gnu/Watcom compilers (which use the first syntax) or for
/// the Windows VCC compiler (which uses the second syntax).
///
#define UINT_64  unsigned long long OR unsigned __int64
#endif

#endif // #if !defined(__APPLE__) || defined(HAVE_TSERVER)

//  ADDR64X is used to print addresses in hex form on both Windows and Linux
//
#if defined( __GNUC__ ) || defined( __WATCOMC__ )
#define ADDR64X "llx"
#define ADDR64D "lld"

#elif defined( _WIN32 )
#define ADDR64X "I64x"
#define ADDR64D "I64d"

#else
#error Unsupported compiler and/or operating system for 64-bit integers

/// @brief Addrlib device address 64-bit printf tag  (compiler dependent)
/// @ingroup type
///
/// This allows printf to display an ADDR_64 for either the Windows VCC compiler
/// (which used this value) or the Gnu/Watcom compilers (which use "llx".
/// An example of use is printf("addr 0x%"ADDR64X"\n", address);
///
#define ADDR64X "llx" OR "I64x"
#define ADDR64D "lld" OR "I64d"
#endif


/// @brief Union for storing a 32-bit float or 32-bit integer
/// @ingroup type
///
/// This union provides a simple way to convert between a 32-bit float
/// and a 32-bit integer. It also prevents the compiler from producing
/// code that alters NaN values when assiging or coying floats.
/// Therefore, all address library routines that pass or return 32-bit
/// floating point data do so by passing or returning a FLT_32.
///
typedef union {
    INT_32   i;
    UINT_32  u;
    float    f;
} ADDR_FLT_32;


////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  Macros for controlling linking and building on multiple systems
//
////////////////////////////////////////////////////////////////////////////////////////////////////
#if defined(_MSC_VER)
#if defined(va_copy)
#undef va_copy  //redefine va_copy to support VC2013
#endif
#endif

#if !defined(va_copy)
#define va_copy(dst, src) \
    ((void) memcpy(&(dst), &(src), sizeof(va_list)))
#endif

#endif // __ADDR_TYPES_H__

