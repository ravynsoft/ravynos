/*
************************************************************************************************************************
*
*  Copyright (C) 2007-2022 Advanced Micro Devices, Inc.  All rights reserved.
*  SPDX-License-Identifier: MIT
*
***********************************************************************************************************************/

/**
************************************************************************************************************************
* @file  gfx10SwizzlePattern.h
* @brief swizzle pattern for gfx10.
************************************************************************************************************************
*/

#ifndef __GFX10_SWIZZLE_PATTERN_H__
#define __GFX10_SWIZZLE_PATTERN_H__


namespace Addr
{
namespace V2
{
const ADDR_SW_PATINFO GFX10_SW_256_S_PATINFO[] =
{
    {   1,    0,    0,    0,    0, } , // 1 pipes 1 bpe @ SW_256_S @ Navi1x
    {   1,    1,    0,    0,    0, } , // 1 pipes 2 bpe @ SW_256_S @ Navi1x
    {   1,    2,    0,    0,    0, } , // 1 pipes 4 bpe @ SW_256_S @ Navi1x
    {   1,    3,    0,    0,    0, } , // 1 pipes 8 bpe @ SW_256_S @ Navi1x
    {   1,    4,    0,    0,    0, } , // 1 pipes 16 bpe @ SW_256_S @ Navi1x
    {   1,    0,    0,    0,    0, } , // 2 pipes 1 bpe @ SW_256_S @ Navi1x
    {   1,    1,    0,    0,    0, } , // 2 pipes 2 bpe @ SW_256_S @ Navi1x
    {   1,    2,    0,    0,    0, } , // 2 pipes 4 bpe @ SW_256_S @ Navi1x
    {   1,    3,    0,    0,    0, } , // 2 pipes 8 bpe @ SW_256_S @ Navi1x
    {   1,    4,    0,    0,    0, } , // 2 pipes 16 bpe @ SW_256_S @ Navi1x
    {   1,    0,    0,    0,    0, } , // 4 pipes 1 bpe @ SW_256_S @ Navi1x
    {   1,    1,    0,    0,    0, } , // 4 pipes 2 bpe @ SW_256_S @ Navi1x
    {   1,    2,    0,    0,    0, } , // 4 pipes 4 bpe @ SW_256_S @ Navi1x
    {   1,    3,    0,    0,    0, } , // 4 pipes 8 bpe @ SW_256_S @ Navi1x
    {   1,    4,    0,    0,    0, } , // 4 pipes 16 bpe @ SW_256_S @ Navi1x
    {   1,    0,    0,    0,    0, } , // 8 pipes 1 bpe @ SW_256_S @ Navi1x
    {   1,    1,    0,    0,    0, } , // 8 pipes 2 bpe @ SW_256_S @ Navi1x
    {   1,    2,    0,    0,    0, } , // 8 pipes 4 bpe @ SW_256_S @ Navi1x
    {   1,    3,    0,    0,    0, } , // 8 pipes 8 bpe @ SW_256_S @ Navi1x
    {   1,    4,    0,    0,    0, } , // 8 pipes 16 bpe @ SW_256_S @ Navi1x
    {   1,    0,    0,    0,    0, } , // 16 pipes 1 bpe @ SW_256_S @ Navi1x
    {   1,    1,    0,    0,    0, } , // 16 pipes 2 bpe @ SW_256_S @ Navi1x
    {   1,    2,    0,    0,    0, } , // 16 pipes 4 bpe @ SW_256_S @ Navi1x
    {   1,    3,    0,    0,    0, } , // 16 pipes 8 bpe @ SW_256_S @ Navi1x
    {   1,    4,    0,    0,    0, } , // 16 pipes 16 bpe @ SW_256_S @ Navi1x
    {   1,    0,    0,    0,    0, } , // 32 pipes 1 bpe @ SW_256_S @ Navi1x
    {   1,    1,    0,    0,    0, } , // 32 pipes 2 bpe @ SW_256_S @ Navi1x
    {   1,    2,    0,    0,    0, } , // 32 pipes 4 bpe @ SW_256_S @ Navi1x
    {   1,    3,    0,    0,    0, } , // 32 pipes 8 bpe @ SW_256_S @ Navi1x
    {   1,    4,    0,    0,    0, } , // 32 pipes 16 bpe @ SW_256_S @ Navi1x
    {   1,    0,    0,    0,    0, } , // 64 pipes 1 bpe @ SW_256_S @ Navi1x
    {   1,    1,    0,    0,    0, } , // 64 pipes 2 bpe @ SW_256_S @ Navi1x
    {   1,    2,    0,    0,    0, } , // 64 pipes 4 bpe @ SW_256_S @ Navi1x
    {   1,    3,    0,    0,    0, } , // 64 pipes 8 bpe @ SW_256_S @ Navi1x
    {   1,    4,    0,    0,    0, } , // 64 pipes 16 bpe @ SW_256_S @ Navi1x
};

const ADDR_SW_PATINFO GFX10_SW_256_D_PATINFO[] =
{
    {   1,    5,    0,    0,    0, } , // 1 pipes 1 bpe @ SW_256_D @ Navi1x
    {   1,    1,    0,    0,    0, } , // 1 pipes 2 bpe @ SW_256_D @ Navi1x
    {   1,    2,    0,    0,    0, } , // 1 pipes 4 bpe @ SW_256_D @ Navi1x
    {   1,    6,    0,    0,    0, } , // 1 pipes 8 bpe @ SW_256_D @ Navi1x
    {   1,    7,    0,    0,    0, } , // 1 pipes 16 bpe @ SW_256_D @ Navi1x
    {   1,    5,    0,    0,    0, } , // 2 pipes 1 bpe @ SW_256_D @ Navi1x
    {   1,    1,    0,    0,    0, } , // 2 pipes 2 bpe @ SW_256_D @ Navi1x
    {   1,    2,    0,    0,    0, } , // 2 pipes 4 bpe @ SW_256_D @ Navi1x
    {   1,    6,    0,    0,    0, } , // 2 pipes 8 bpe @ SW_256_D @ Navi1x
    {   1,    7,    0,    0,    0, } , // 2 pipes 16 bpe @ SW_256_D @ Navi1x
    {   1,    5,    0,    0,    0, } , // 4 pipes 1 bpe @ SW_256_D @ Navi1x
    {   1,    1,    0,    0,    0, } , // 4 pipes 2 bpe @ SW_256_D @ Navi1x
    {   1,    2,    0,    0,    0, } , // 4 pipes 4 bpe @ SW_256_D @ Navi1x
    {   1,    6,    0,    0,    0, } , // 4 pipes 8 bpe @ SW_256_D @ Navi1x
    {   1,    7,    0,    0,    0, } , // 4 pipes 16 bpe @ SW_256_D @ Navi1x
    {   1,    5,    0,    0,    0, } , // 8 pipes 1 bpe @ SW_256_D @ Navi1x
    {   1,    1,    0,    0,    0, } , // 8 pipes 2 bpe @ SW_256_D @ Navi1x
    {   1,    2,    0,    0,    0, } , // 8 pipes 4 bpe @ SW_256_D @ Navi1x
    {   1,    6,    0,    0,    0, } , // 8 pipes 8 bpe @ SW_256_D @ Navi1x
    {   1,    7,    0,    0,    0, } , // 8 pipes 16 bpe @ SW_256_D @ Navi1x
    {   1,    5,    0,    0,    0, } , // 16 pipes 1 bpe @ SW_256_D @ Navi1x
    {   1,    1,    0,    0,    0, } , // 16 pipes 2 bpe @ SW_256_D @ Navi1x
    {   1,    2,    0,    0,    0, } , // 16 pipes 4 bpe @ SW_256_D @ Navi1x
    {   1,    6,    0,    0,    0, } , // 16 pipes 8 bpe @ SW_256_D @ Navi1x
    {   1,    7,    0,    0,    0, } , // 16 pipes 16 bpe @ SW_256_D @ Navi1x
    {   1,    5,    0,    0,    0, } , // 32 pipes 1 bpe @ SW_256_D @ Navi1x
    {   1,    1,    0,    0,    0, } , // 32 pipes 2 bpe @ SW_256_D @ Navi1x
    {   1,    2,    0,    0,    0, } , // 32 pipes 4 bpe @ SW_256_D @ Navi1x
    {   1,    6,    0,    0,    0, } , // 32 pipes 8 bpe @ SW_256_D @ Navi1x
    {   1,    7,    0,    0,    0, } , // 32 pipes 16 bpe @ SW_256_D @ Navi1x
    {   1,    5,    0,    0,    0, } , // 64 pipes 1 bpe @ SW_256_D @ Navi1x
    {   1,    1,    0,    0,    0, } , // 64 pipes 2 bpe @ SW_256_D @ Navi1x
    {   1,    2,    0,    0,    0, } , // 64 pipes 4 bpe @ SW_256_D @ Navi1x
    {   1,    6,    0,    0,    0, } , // 64 pipes 8 bpe @ SW_256_D @ Navi1x
    {   1,    7,    0,    0,    0, } , // 64 pipes 16 bpe @ SW_256_D @ Navi1x
};

const ADDR_SW_PATINFO GFX10_SW_4K_S_PATINFO[] =
{
    {   1,    0,    1,    0,    0, } , // 1 pipes 1 bpe @ SW_4K_S @ Navi1x
    {   1,    1,    2,    0,    0, } , // 1 pipes 2 bpe @ SW_4K_S @ Navi1x
    {   1,    2,    3,    0,    0, } , // 1 pipes 4 bpe @ SW_4K_S @ Navi1x
    {   1,    3,    4,    0,    0, } , // 1 pipes 8 bpe @ SW_4K_S @ Navi1x
    {   1,    4,    5,    0,    0, } , // 1 pipes 16 bpe @ SW_4K_S @ Navi1x
    {   1,    0,    1,    0,    0, } , // 2 pipes 1 bpe @ SW_4K_S @ Navi1x
    {   1,    1,    2,    0,    0, } , // 2 pipes 2 bpe @ SW_4K_S @ Navi1x
    {   1,    2,    3,    0,    0, } , // 2 pipes 4 bpe @ SW_4K_S @ Navi1x
    {   1,    3,    4,    0,    0, } , // 2 pipes 8 bpe @ SW_4K_S @ Navi1x
    {   1,    4,    5,    0,    0, } , // 2 pipes 16 bpe @ SW_4K_S @ Navi1x
    {   1,    0,    1,    0,    0, } , // 4 pipes 1 bpe @ SW_4K_S @ Navi1x
    {   1,    1,    2,    0,    0, } , // 4 pipes 2 bpe @ SW_4K_S @ Navi1x
    {   1,    2,    3,    0,    0, } , // 4 pipes 4 bpe @ SW_4K_S @ Navi1x
    {   1,    3,    4,    0,    0, } , // 4 pipes 8 bpe @ SW_4K_S @ Navi1x
    {   1,    4,    5,    0,    0, } , // 4 pipes 16 bpe @ SW_4K_S @ Navi1x
    {   1,    0,    1,    0,    0, } , // 8 pipes 1 bpe @ SW_4K_S @ Navi1x
    {   1,    1,    2,    0,    0, } , // 8 pipes 2 bpe @ SW_4K_S @ Navi1x
    {   1,    2,    3,    0,    0, } , // 8 pipes 4 bpe @ SW_4K_S @ Navi1x
    {   1,    3,    4,    0,    0, } , // 8 pipes 8 bpe @ SW_4K_S @ Navi1x
    {   1,    4,    5,    0,    0, } , // 8 pipes 16 bpe @ SW_4K_S @ Navi1x
    {   1,    0,    1,    0,    0, } , // 16 pipes 1 bpe @ SW_4K_S @ Navi1x
    {   1,    1,    2,    0,    0, } , // 16 pipes 2 bpe @ SW_4K_S @ Navi1x
    {   1,    2,    3,    0,    0, } , // 16 pipes 4 bpe @ SW_4K_S @ Navi1x
    {   1,    3,    4,    0,    0, } , // 16 pipes 8 bpe @ SW_4K_S @ Navi1x
    {   1,    4,    5,    0,    0, } , // 16 pipes 16 bpe @ SW_4K_S @ Navi1x
    {   1,    0,    1,    0,    0, } , // 32 pipes 1 bpe @ SW_4K_S @ Navi1x
    {   1,    1,    2,    0,    0, } , // 32 pipes 2 bpe @ SW_4K_S @ Navi1x
    {   1,    2,    3,    0,    0, } , // 32 pipes 4 bpe @ SW_4K_S @ Navi1x
    {   1,    3,    4,    0,    0, } , // 32 pipes 8 bpe @ SW_4K_S @ Navi1x
    {   1,    4,    5,    0,    0, } , // 32 pipes 16 bpe @ SW_4K_S @ Navi1x
    {   1,    0,    1,    0,    0, } , // 64 pipes 1 bpe @ SW_4K_S @ Navi1x
    {   1,    1,    2,    0,    0, } , // 64 pipes 2 bpe @ SW_4K_S @ Navi1x
    {   1,    2,    3,    0,    0, } , // 64 pipes 4 bpe @ SW_4K_S @ Navi1x
    {   1,    3,    4,    0,    0, } , // 64 pipes 8 bpe @ SW_4K_S @ Navi1x
    {   1,    4,    5,    0,    0, } , // 64 pipes 16 bpe @ SW_4K_S @ Navi1x
};

const ADDR_SW_PATINFO GFX10_SW_4K_D_PATINFO[] =
{
    {   1,    5,    1,    0,    0, } , // 1 pipes 1 bpe @ SW_4K_D @ Navi1x
    {   1,    1,    2,    0,    0, } , // 1 pipes 2 bpe @ SW_4K_D @ Navi1x
    {   1,    2,    3,    0,    0, } , // 1 pipes 4 bpe @ SW_4K_D @ Navi1x
    {   1,    6,    4,    0,    0, } , // 1 pipes 8 bpe @ SW_4K_D @ Navi1x
    {   1,    7,    5,    0,    0, } , // 1 pipes 16 bpe @ SW_4K_D @ Navi1x
    {   1,    5,    1,    0,    0, } , // 2 pipes 1 bpe @ SW_4K_D @ Navi1x
    {   1,    1,    2,    0,    0, } , // 2 pipes 2 bpe @ SW_4K_D @ Navi1x
    {   1,    2,    3,    0,    0, } , // 2 pipes 4 bpe @ SW_4K_D @ Navi1x
    {   1,    6,    4,    0,    0, } , // 2 pipes 8 bpe @ SW_4K_D @ Navi1x
    {   1,    7,    5,    0,    0, } , // 2 pipes 16 bpe @ SW_4K_D @ Navi1x
    {   1,    5,    1,    0,    0, } , // 4 pipes 1 bpe @ SW_4K_D @ Navi1x
    {   1,    1,    2,    0,    0, } , // 4 pipes 2 bpe @ SW_4K_D @ Navi1x
    {   1,    2,    3,    0,    0, } , // 4 pipes 4 bpe @ SW_4K_D @ Navi1x
    {   1,    6,    4,    0,    0, } , // 4 pipes 8 bpe @ SW_4K_D @ Navi1x
    {   1,    7,    5,    0,    0, } , // 4 pipes 16 bpe @ SW_4K_D @ Navi1x
    {   1,    5,    1,    0,    0, } , // 8 pipes 1 bpe @ SW_4K_D @ Navi1x
    {   1,    1,    2,    0,    0, } , // 8 pipes 2 bpe @ SW_4K_D @ Navi1x
    {   1,    2,    3,    0,    0, } , // 8 pipes 4 bpe @ SW_4K_D @ Navi1x
    {   1,    6,    4,    0,    0, } , // 8 pipes 8 bpe @ SW_4K_D @ Navi1x
    {   1,    7,    5,    0,    0, } , // 8 pipes 16 bpe @ SW_4K_D @ Navi1x
    {   1,    5,    1,    0,    0, } , // 16 pipes 1 bpe @ SW_4K_D @ Navi1x
    {   1,    1,    2,    0,    0, } , // 16 pipes 2 bpe @ SW_4K_D @ Navi1x
    {   1,    2,    3,    0,    0, } , // 16 pipes 4 bpe @ SW_4K_D @ Navi1x
    {   1,    6,    4,    0,    0, } , // 16 pipes 8 bpe @ SW_4K_D @ Navi1x
    {   1,    7,    5,    0,    0, } , // 16 pipes 16 bpe @ SW_4K_D @ Navi1x
    {   1,    5,    1,    0,    0, } , // 32 pipes 1 bpe @ SW_4K_D @ Navi1x
    {   1,    1,    2,    0,    0, } , // 32 pipes 2 bpe @ SW_4K_D @ Navi1x
    {   1,    2,    3,    0,    0, } , // 32 pipes 4 bpe @ SW_4K_D @ Navi1x
    {   1,    6,    4,    0,    0, } , // 32 pipes 8 bpe @ SW_4K_D @ Navi1x
    {   1,    7,    5,    0,    0, } , // 32 pipes 16 bpe @ SW_4K_D @ Navi1x
    {   1,    5,    1,    0,    0, } , // 64 pipes 1 bpe @ SW_4K_D @ Navi1x
    {   1,    1,    2,    0,    0, } , // 64 pipes 2 bpe @ SW_4K_D @ Navi1x
    {   1,    2,    3,    0,    0, } , // 64 pipes 4 bpe @ SW_4K_D @ Navi1x
    {   1,    6,    4,    0,    0, } , // 64 pipes 8 bpe @ SW_4K_D @ Navi1x
    {   1,    7,    5,    0,    0, } , // 64 pipes 16 bpe @ SW_4K_D @ Navi1x
};

const ADDR_SW_PATINFO GFX10_SW_4K_S_X_PATINFO[] =
{
    {   1,    0,    1,    0,    0, } , // 1 pipes 1 bpe @ SW_4K_S_X @ Navi1x
    {   1,    1,    2,    0,    0, } , // 1 pipes 2 bpe @ SW_4K_S_X @ Navi1x
    {   1,    2,    3,    0,    0, } , // 1 pipes 4 bpe @ SW_4K_S_X @ Navi1x
    {   1,    3,    4,    0,    0, } , // 1 pipes 8 bpe @ SW_4K_S_X @ Navi1x
    {   1,    4,    5,    0,    0, } , // 1 pipes 16 bpe @ SW_4K_S_X @ Navi1x
    {   3,    0,    6,    0,    0, } , // 2 pipes 1 bpe @ SW_4K_S_X @ Navi1x
    {   3,    1,    7,    0,    0, } , // 2 pipes 2 bpe @ SW_4K_S_X @ Navi1x
    {   3,    2,    8,    0,    0, } , // 2 pipes 4 bpe @ SW_4K_S_X @ Navi1x
    {   3,    3,    9,    0,    0, } , // 2 pipes 8 bpe @ SW_4K_S_X @ Navi1x
    {   3,    4,   10,    0,    0, } , // 2 pipes 16 bpe @ SW_4K_S_X @ Navi1x
    {   3,    0,   11,    0,    0, } , // 4 pipes 1 bpe @ SW_4K_S_X @ Navi1x
    {   3,    1,   12,    0,    0, } , // 4 pipes 2 bpe @ SW_4K_S_X @ Navi1x
    {   3,    2,   13,    0,    0, } , // 4 pipes 4 bpe @ SW_4K_S_X @ Navi1x
    {   3,    3,   14,    0,    0, } , // 4 pipes 8 bpe @ SW_4K_S_X @ Navi1x
    {   3,    4,   15,    0,    0, } , // 4 pipes 16 bpe @ SW_4K_S_X @ Navi1x
    {   3,    0,   16,    0,    0, } , // 8 pipes 1 bpe @ SW_4K_S_X @ Navi1x
    {   3,    1,   17,    0,    0, } , // 8 pipes 2 bpe @ SW_4K_S_X @ Navi1x
    {   3,    2,   18,    0,    0, } , // 8 pipes 4 bpe @ SW_4K_S_X @ Navi1x
    {   3,    3,   19,    0,    0, } , // 8 pipes 8 bpe @ SW_4K_S_X @ Navi1x
    {   3,    4,   20,    0,    0, } , // 8 pipes 16 bpe @ SW_4K_S_X @ Navi1x
    {   3,    0,   21,    0,    0, } , // 16 pipes 1 bpe @ SW_4K_S_X @ Navi1x
    {   3,    1,   22,    0,    0, } , // 16 pipes 2 bpe @ SW_4K_S_X @ Navi1x
    {   3,    2,   23,    0,    0, } , // 16 pipes 4 bpe @ SW_4K_S_X @ Navi1x
    {   3,    3,   24,    0,    0, } , // 16 pipes 8 bpe @ SW_4K_S_X @ Navi1x
    {   3,    4,   25,    0,    0, } , // 16 pipes 16 bpe @ SW_4K_S_X @ Navi1x
    {   3,    0,   21,    0,    0, } , // 32 pipes 1 bpe @ SW_4K_S_X @ Navi1x
    {   3,    1,   22,    0,    0, } , // 32 pipes 2 bpe @ SW_4K_S_X @ Navi1x
    {   3,    2,   23,    0,    0, } , // 32 pipes 4 bpe @ SW_4K_S_X @ Navi1x
    {   3,    3,   24,    0,    0, } , // 32 pipes 8 bpe @ SW_4K_S_X @ Navi1x
    {   3,    4,   25,    0,    0, } , // 32 pipes 16 bpe @ SW_4K_S_X @ Navi1x
    {   3,    0,   21,    0,    0, } , // 64 pipes 1 bpe @ SW_4K_S_X @ Navi1x
    {   3,    1,   22,    0,    0, } , // 64 pipes 2 bpe @ SW_4K_S_X @ Navi1x
    {   3,    2,   23,    0,    0, } , // 64 pipes 4 bpe @ SW_4K_S_X @ Navi1x
    {   3,    3,   24,    0,    0, } , // 64 pipes 8 bpe @ SW_4K_S_X @ Navi1x
    {   3,    4,   25,    0,    0, } , // 64 pipes 16 bpe @ SW_4K_S_X @ Navi1x
};

const ADDR_SW_PATINFO GFX10_SW_4K_D_X_PATINFO[] =
{
    {   1,    5,    1,    0,    0, } , // 1 pipes 1 bpe @ SW_4K_D_X @ Navi1x
    {   1,    1,    2,    0,    0, } , // 1 pipes 2 bpe @ SW_4K_D_X @ Navi1x
    {   1,    2,    3,    0,    0, } , // 1 pipes 4 bpe @ SW_4K_D_X @ Navi1x
    {   1,    6,    4,    0,    0, } , // 1 pipes 8 bpe @ SW_4K_D_X @ Navi1x
    {   1,    7,    5,    0,    0, } , // 1 pipes 16 bpe @ SW_4K_D_X @ Navi1x
    {   3,    5,    6,    0,    0, } , // 2 pipes 1 bpe @ SW_4K_D_X @ Navi1x
    {   3,    1,    7,    0,    0, } , // 2 pipes 2 bpe @ SW_4K_D_X @ Navi1x
    {   3,    2,    8,    0,    0, } , // 2 pipes 4 bpe @ SW_4K_D_X @ Navi1x
    {   3,    6,    9,    0,    0, } , // 2 pipes 8 bpe @ SW_4K_D_X @ Navi1x
    {   3,    7,   10,    0,    0, } , // 2 pipes 16 bpe @ SW_4K_D_X @ Navi1x
    {   3,    5,   11,    0,    0, } , // 4 pipes 1 bpe @ SW_4K_D_X @ Navi1x
    {   3,    1,   12,    0,    0, } , // 4 pipes 2 bpe @ SW_4K_D_X @ Navi1x
    {   3,    2,   13,    0,    0, } , // 4 pipes 4 bpe @ SW_4K_D_X @ Navi1x
    {   3,    6,   14,    0,    0, } , // 4 pipes 8 bpe @ SW_4K_D_X @ Navi1x
    {   3,    7,   15,    0,    0, } , // 4 pipes 16 bpe @ SW_4K_D_X @ Navi1x
    {   3,    5,   16,    0,    0, } , // 8 pipes 1 bpe @ SW_4K_D_X @ Navi1x
    {   3,    1,   17,    0,    0, } , // 8 pipes 2 bpe @ SW_4K_D_X @ Navi1x
    {   3,    2,   18,    0,    0, } , // 8 pipes 4 bpe @ SW_4K_D_X @ Navi1x
    {   3,    6,   19,    0,    0, } , // 8 pipes 8 bpe @ SW_4K_D_X @ Navi1x
    {   3,    7,   20,    0,    0, } , // 8 pipes 16 bpe @ SW_4K_D_X @ Navi1x
    {   3,    5,   21,    0,    0, } , // 16 pipes 1 bpe @ SW_4K_D_X @ Navi1x
    {   3,    1,   22,    0,    0, } , // 16 pipes 2 bpe @ SW_4K_D_X @ Navi1x
    {   3,    2,   23,    0,    0, } , // 16 pipes 4 bpe @ SW_4K_D_X @ Navi1x
    {   3,    6,   24,    0,    0, } , // 16 pipes 8 bpe @ SW_4K_D_X @ Navi1x
    {   3,    7,   25,    0,    0, } , // 16 pipes 16 bpe @ SW_4K_D_X @ Navi1x
    {   3,    5,   21,    0,    0, } , // 32 pipes 1 bpe @ SW_4K_D_X @ Navi1x
    {   3,    1,   22,    0,    0, } , // 32 pipes 2 bpe @ SW_4K_D_X @ Navi1x
    {   3,    2,   23,    0,    0, } , // 32 pipes 4 bpe @ SW_4K_D_X @ Navi1x
    {   3,    6,   24,    0,    0, } , // 32 pipes 8 bpe @ SW_4K_D_X @ Navi1x
    {   3,    7,   25,    0,    0, } , // 32 pipes 16 bpe @ SW_4K_D_X @ Navi1x
    {   3,    5,   21,    0,    0, } , // 64 pipes 1 bpe @ SW_4K_D_X @ Navi1x
    {   3,    1,   22,    0,    0, } , // 64 pipes 2 bpe @ SW_4K_D_X @ Navi1x
    {   3,    2,   23,    0,    0, } , // 64 pipes 4 bpe @ SW_4K_D_X @ Navi1x
    {   3,    6,   24,    0,    0, } , // 64 pipes 8 bpe @ SW_4K_D_X @ Navi1x
    {   3,    7,   25,    0,    0, } , // 64 pipes 16 bpe @ SW_4K_D_X @ Navi1x
};

const ADDR_SW_PATINFO GFX10_SW_4K_S3_PATINFO[] =
{
    {   1,   29,  131,    0,    0, } , // 1 pipes 1 bpe @ SW_4K_S3 @ Navi1x
    {   1,   30,  132,    0,    0, } , // 1 pipes 2 bpe @ SW_4K_S3 @ Navi1x
    {   1,   31,  133,    0,    0, } , // 1 pipes 4 bpe @ SW_4K_S3 @ Navi1x
    {   1,   32,  134,    0,    0, } , // 1 pipes 8 bpe @ SW_4K_S3 @ Navi1x
    {   1,   33,  135,    0,    0, } , // 1 pipes 16 bpe @ SW_4K_S3 @ Navi1x
    {   1,   29,  131,    0,    0, } , // 2 pipes 1 bpe @ SW_4K_S3 @ Navi1x
    {   1,   30,  132,    0,    0, } , // 2 pipes 2 bpe @ SW_4K_S3 @ Navi1x
    {   1,   31,  133,    0,    0, } , // 2 pipes 4 bpe @ SW_4K_S3 @ Navi1x
    {   1,   32,  134,    0,    0, } , // 2 pipes 8 bpe @ SW_4K_S3 @ Navi1x
    {   1,   33,  135,    0,    0, } , // 2 pipes 16 bpe @ SW_4K_S3 @ Navi1x
    {   1,   29,  131,    0,    0, } , // 4 pipes 1 bpe @ SW_4K_S3 @ Navi1x
    {   1,   30,  132,    0,    0, } , // 4 pipes 2 bpe @ SW_4K_S3 @ Navi1x
    {   1,   31,  133,    0,    0, } , // 4 pipes 4 bpe @ SW_4K_S3 @ Navi1x
    {   1,   32,  134,    0,    0, } , // 4 pipes 8 bpe @ SW_4K_S3 @ Navi1x
    {   1,   33,  135,    0,    0, } , // 4 pipes 16 bpe @ SW_4K_S3 @ Navi1x
    {   1,   29,  131,    0,    0, } , // 8 pipes 1 bpe @ SW_4K_S3 @ Navi1x
    {   1,   30,  132,    0,    0, } , // 8 pipes 2 bpe @ SW_4K_S3 @ Navi1x
    {   1,   31,  133,    0,    0, } , // 8 pipes 4 bpe @ SW_4K_S3 @ Navi1x
    {   1,   32,  134,    0,    0, } , // 8 pipes 8 bpe @ SW_4K_S3 @ Navi1x
    {   1,   33,  135,    0,    0, } , // 8 pipes 16 bpe @ SW_4K_S3 @ Navi1x
    {   1,   29,  131,    0,    0, } , // 16 pipes 1 bpe @ SW_4K_S3 @ Navi1x
    {   1,   30,  132,    0,    0, } , // 16 pipes 2 bpe @ SW_4K_S3 @ Navi1x
    {   1,   31,  133,    0,    0, } , // 16 pipes 4 bpe @ SW_4K_S3 @ Navi1x
    {   1,   32,  134,    0,    0, } , // 16 pipes 8 bpe @ SW_4K_S3 @ Navi1x
    {   1,   33,  135,    0,    0, } , // 16 pipes 16 bpe @ SW_4K_S3 @ Navi1x
    {   1,   29,  131,    0,    0, } , // 32 pipes 1 bpe @ SW_4K_S3 @ Navi1x
    {   1,   30,  132,    0,    0, } , // 32 pipes 2 bpe @ SW_4K_S3 @ Navi1x
    {   1,   31,  133,    0,    0, } , // 32 pipes 4 bpe @ SW_4K_S3 @ Navi1x
    {   1,   32,  134,    0,    0, } , // 32 pipes 8 bpe @ SW_4K_S3 @ Navi1x
    {   1,   33,  135,    0,    0, } , // 32 pipes 16 bpe @ SW_4K_S3 @ Navi1x
    {   1,   29,  131,    0,    0, } , // 64 pipes 1 bpe @ SW_4K_S3 @ Navi1x
    {   1,   30,  132,    0,    0, } , // 64 pipes 2 bpe @ SW_4K_S3 @ Navi1x
    {   1,   31,  133,    0,    0, } , // 64 pipes 4 bpe @ SW_4K_S3 @ Navi1x
    {   1,   32,  134,    0,    0, } , // 64 pipes 8 bpe @ SW_4K_S3 @ Navi1x
    {   1,   33,  135,    0,    0, } , // 64 pipes 16 bpe @ SW_4K_S3 @ Navi1x
};

const ADDR_SW_PATINFO GFX10_SW_4K_S3_X_PATINFO[] =
{
    {   1,   29,  131,    0,    0, } , // 1 pipes 1 bpe @ SW_4K_S3_X @ Navi1x
    {   1,   30,  132,    0,    0, } , // 1 pipes 2 bpe @ SW_4K_S3_X @ Navi1x
    {   1,   31,  133,    0,    0, } , // 1 pipes 4 bpe @ SW_4K_S3_X @ Navi1x
    {   1,   32,  134,    0,    0, } , // 1 pipes 8 bpe @ SW_4K_S3_X @ Navi1x
    {   1,   33,  135,    0,    0, } , // 1 pipes 16 bpe @ SW_4K_S3_X @ Navi1x
    {   3,   29,  136,    0,    0, } , // 2 pipes 1 bpe @ SW_4K_S3_X @ Navi1x
    {   3,   30,  137,    0,    0, } , // 2 pipes 2 bpe @ SW_4K_S3_X @ Navi1x
    {   3,   31,  138,    0,    0, } , // 2 pipes 4 bpe @ SW_4K_S3_X @ Navi1x
    {   3,   32,  139,    0,    0, } , // 2 pipes 8 bpe @ SW_4K_S3_X @ Navi1x
    {   3,   33,  140,    0,    0, } , // 2 pipes 16 bpe @ SW_4K_S3_X @ Navi1x
    {   3,   29,  141,    0,    0, } , // 4 pipes 1 bpe @ SW_4K_S3_X @ Navi1x
    {   3,   30,  142,    0,    0, } , // 4 pipes 2 bpe @ SW_4K_S3_X @ Navi1x
    {   3,   31,  143,    0,    0, } , // 4 pipes 4 bpe @ SW_4K_S3_X @ Navi1x
    {   3,   32,  144,    0,    0, } , // 4 pipes 8 bpe @ SW_4K_S3_X @ Navi1x
    {   3,   33,  145,    0,    0, } , // 4 pipes 16 bpe @ SW_4K_S3_X @ Navi1x
    {   3,   29,  146,    0,    0, } , // 8 pipes 1 bpe @ SW_4K_S3_X @ Navi1x
    {   3,   30,  147,    0,    0, } , // 8 pipes 2 bpe @ SW_4K_S3_X @ Navi1x
    {   3,   31,  148,    0,    0, } , // 8 pipes 4 bpe @ SW_4K_S3_X @ Navi1x
    {   3,   32,  149,    0,    0, } , // 8 pipes 8 bpe @ SW_4K_S3_X @ Navi1x
    {   3,   33,  150,    0,    0, } , // 8 pipes 16 bpe @ SW_4K_S3_X @ Navi1x
    {   3,   29,  151,    0,    0, } , // 16 pipes 1 bpe @ SW_4K_S3_X @ Navi1x
    {   3,   30,  152,    0,    0, } , // 16 pipes 2 bpe @ SW_4K_S3_X @ Navi1x
    {   3,   31,  153,    0,    0, } , // 16 pipes 4 bpe @ SW_4K_S3_X @ Navi1x
    {   3,   32,  154,    0,    0, } , // 16 pipes 8 bpe @ SW_4K_S3_X @ Navi1x
    {   3,   33,  155,    0,    0, } , // 16 pipes 16 bpe @ SW_4K_S3_X @ Navi1x
    {   3,   29,  151,    0,    0, } , // 32 pipes 1 bpe @ SW_4K_S3_X @ Navi1x
    {   3,   30,  152,    0,    0, } , // 32 pipes 2 bpe @ SW_4K_S3_X @ Navi1x
    {   3,   31,  153,    0,    0, } , // 32 pipes 4 bpe @ SW_4K_S3_X @ Navi1x
    {   3,   32,  154,    0,    0, } , // 32 pipes 8 bpe @ SW_4K_S3_X @ Navi1x
    {   3,   33,  155,    0,    0, } , // 32 pipes 16 bpe @ SW_4K_S3_X @ Navi1x
    {   3,   29,  151,    0,    0, } , // 64 pipes 1 bpe @ SW_4K_S3_X @ Navi1x
    {   3,   30,  152,    0,    0, } , // 64 pipes 2 bpe @ SW_4K_S3_X @ Navi1x
    {   3,   31,  153,    0,    0, } , // 64 pipes 4 bpe @ SW_4K_S3_X @ Navi1x
    {   3,   32,  154,    0,    0, } , // 64 pipes 8 bpe @ SW_4K_S3_X @ Navi1x
    {   3,   33,  155,    0,    0, } , // 64 pipes 16 bpe @ SW_4K_S3_X @ Navi1x
};

const ADDR_SW_PATINFO GFX10_SW_64K_S_PATINFO[] =
{
    {   1,    0,    1,    1,    0, } , // 1 pipes 1 bpe @ SW_64K_S @ Navi1x
    {   1,    1,    2,    2,    0, } , // 1 pipes 2 bpe @ SW_64K_S @ Navi1x
    {   1,    2,    3,    3,    0, } , // 1 pipes 4 bpe @ SW_64K_S @ Navi1x
    {   1,    3,    4,    4,    0, } , // 1 pipes 8 bpe @ SW_64K_S @ Navi1x
    {   1,    4,    5,    5,    0, } , // 1 pipes 16 bpe @ SW_64K_S @ Navi1x
    {   1,    0,    1,    1,    0, } , // 2 pipes 1 bpe @ SW_64K_S @ Navi1x
    {   1,    1,    2,    2,    0, } , // 2 pipes 2 bpe @ SW_64K_S @ Navi1x
    {   1,    2,    3,    3,    0, } , // 2 pipes 4 bpe @ SW_64K_S @ Navi1x
    {   1,    3,    4,    4,    0, } , // 2 pipes 8 bpe @ SW_64K_S @ Navi1x
    {   1,    4,    5,    5,    0, } , // 2 pipes 16 bpe @ SW_64K_S @ Navi1x
    {   1,    0,    1,    1,    0, } , // 4 pipes 1 bpe @ SW_64K_S @ Navi1x
    {   1,    1,    2,    2,    0, } , // 4 pipes 2 bpe @ SW_64K_S @ Navi1x
    {   1,    2,    3,    3,    0, } , // 4 pipes 4 bpe @ SW_64K_S @ Navi1x
    {   1,    3,    4,    4,    0, } , // 4 pipes 8 bpe @ SW_64K_S @ Navi1x
    {   1,    4,    5,    5,    0, } , // 4 pipes 16 bpe @ SW_64K_S @ Navi1x
    {   1,    0,    1,    1,    0, } , // 8 pipes 1 bpe @ SW_64K_S @ Navi1x
    {   1,    1,    2,    2,    0, } , // 8 pipes 2 bpe @ SW_64K_S @ Navi1x
    {   1,    2,    3,    3,    0, } , // 8 pipes 4 bpe @ SW_64K_S @ Navi1x
    {   1,    3,    4,    4,    0, } , // 8 pipes 8 bpe @ SW_64K_S @ Navi1x
    {   1,    4,    5,    5,    0, } , // 8 pipes 16 bpe @ SW_64K_S @ Navi1x
    {   1,    0,    1,    1,    0, } , // 16 pipes 1 bpe @ SW_64K_S @ Navi1x
    {   1,    1,    2,    2,    0, } , // 16 pipes 2 bpe @ SW_64K_S @ Navi1x
    {   1,    2,    3,    3,    0, } , // 16 pipes 4 bpe @ SW_64K_S @ Navi1x
    {   1,    3,    4,    4,    0, } , // 16 pipes 8 bpe @ SW_64K_S @ Navi1x
    {   1,    4,    5,    5,    0, } , // 16 pipes 16 bpe @ SW_64K_S @ Navi1x
    {   1,    0,    1,    1,    0, } , // 32 pipes 1 bpe @ SW_64K_S @ Navi1x
    {   1,    1,    2,    2,    0, } , // 32 pipes 2 bpe @ SW_64K_S @ Navi1x
    {   1,    2,    3,    3,    0, } , // 32 pipes 4 bpe @ SW_64K_S @ Navi1x
    {   1,    3,    4,    4,    0, } , // 32 pipes 8 bpe @ SW_64K_S @ Navi1x
    {   1,    4,    5,    5,    0, } , // 32 pipes 16 bpe @ SW_64K_S @ Navi1x
    {   1,    0,    1,    1,    0, } , // 64 pipes 1 bpe @ SW_64K_S @ Navi1x
    {   1,    1,    2,    2,    0, } , // 64 pipes 2 bpe @ SW_64K_S @ Navi1x
    {   1,    2,    3,    3,    0, } , // 64 pipes 4 bpe @ SW_64K_S @ Navi1x
    {   1,    3,    4,    4,    0, } , // 64 pipes 8 bpe @ SW_64K_S @ Navi1x
    {   1,    4,    5,    5,    0, } , // 64 pipes 16 bpe @ SW_64K_S @ Navi1x
};

const ADDR_SW_PATINFO GFX10_SW_64K_D_PATINFO[] =
{
    {   1,    5,    1,    1,    0, } , // 1 pipes 1 bpe @ SW_64K_D @ Navi1x
    {   1,    1,    2,    2,    0, } , // 1 pipes 2 bpe @ SW_64K_D @ Navi1x
    {   1,    2,    3,    3,    0, } , // 1 pipes 4 bpe @ SW_64K_D @ Navi1x
    {   1,    6,    4,    4,    0, } , // 1 pipes 8 bpe @ SW_64K_D @ Navi1x
    {   1,    7,    5,    5,    0, } , // 1 pipes 16 bpe @ SW_64K_D @ Navi1x
    {   1,    5,    1,    1,    0, } , // 2 pipes 1 bpe @ SW_64K_D @ Navi1x
    {   1,    1,    2,    2,    0, } , // 2 pipes 2 bpe @ SW_64K_D @ Navi1x
    {   1,    2,    3,    3,    0, } , // 2 pipes 4 bpe @ SW_64K_D @ Navi1x
    {   1,    6,    4,    4,    0, } , // 2 pipes 8 bpe @ SW_64K_D @ Navi1x
    {   1,    7,    5,    5,    0, } , // 2 pipes 16 bpe @ SW_64K_D @ Navi1x
    {   1,    5,    1,    1,    0, } , // 4 pipes 1 bpe @ SW_64K_D @ Navi1x
    {   1,    1,    2,    2,    0, } , // 4 pipes 2 bpe @ SW_64K_D @ Navi1x
    {   1,    2,    3,    3,    0, } , // 4 pipes 4 bpe @ SW_64K_D @ Navi1x
    {   1,    6,    4,    4,    0, } , // 4 pipes 8 bpe @ SW_64K_D @ Navi1x
    {   1,    7,    5,    5,    0, } , // 4 pipes 16 bpe @ SW_64K_D @ Navi1x
    {   1,    5,    1,    1,    0, } , // 8 pipes 1 bpe @ SW_64K_D @ Navi1x
    {   1,    1,    2,    2,    0, } , // 8 pipes 2 bpe @ SW_64K_D @ Navi1x
    {   1,    2,    3,    3,    0, } , // 8 pipes 4 bpe @ SW_64K_D @ Navi1x
    {   1,    6,    4,    4,    0, } , // 8 pipes 8 bpe @ SW_64K_D @ Navi1x
    {   1,    7,    5,    5,    0, } , // 8 pipes 16 bpe @ SW_64K_D @ Navi1x
    {   1,    5,    1,    1,    0, } , // 16 pipes 1 bpe @ SW_64K_D @ Navi1x
    {   1,    1,    2,    2,    0, } , // 16 pipes 2 bpe @ SW_64K_D @ Navi1x
    {   1,    2,    3,    3,    0, } , // 16 pipes 4 bpe @ SW_64K_D @ Navi1x
    {   1,    6,    4,    4,    0, } , // 16 pipes 8 bpe @ SW_64K_D @ Navi1x
    {   1,    7,    5,    5,    0, } , // 16 pipes 16 bpe @ SW_64K_D @ Navi1x
    {   1,    5,    1,    1,    0, } , // 32 pipes 1 bpe @ SW_64K_D @ Navi1x
    {   1,    1,    2,    2,    0, } , // 32 pipes 2 bpe @ SW_64K_D @ Navi1x
    {   1,    2,    3,    3,    0, } , // 32 pipes 4 bpe @ SW_64K_D @ Navi1x
    {   1,    6,    4,    4,    0, } , // 32 pipes 8 bpe @ SW_64K_D @ Navi1x
    {   1,    7,    5,    5,    0, } , // 32 pipes 16 bpe @ SW_64K_D @ Navi1x
    {   1,    5,    1,    1,    0, } , // 64 pipes 1 bpe @ SW_64K_D @ Navi1x
    {   1,    1,    2,    2,    0, } , // 64 pipes 2 bpe @ SW_64K_D @ Navi1x
    {   1,    2,    3,    3,    0, } , // 64 pipes 4 bpe @ SW_64K_D @ Navi1x
    {   1,    6,    4,    4,    0, } , // 64 pipes 8 bpe @ SW_64K_D @ Navi1x
    {   1,    7,    5,    5,    0, } , // 64 pipes 16 bpe @ SW_64K_D @ Navi1x
};

const ADDR_SW_PATINFO GFX10_SW_64K_S_T_PATINFO[] =
{
    {   1,    0,    1,    1,    0, } , // 1 pipes 1 bpe @ SW_64K_S_T @ Navi1x
    {   1,    1,    2,    2,    0, } , // 1 pipes 2 bpe @ SW_64K_S_T @ Navi1x
    {   1,    2,    3,    3,    0, } , // 1 pipes 4 bpe @ SW_64K_S_T @ Navi1x
    {   1,    3,    4,    4,    0, } , // 1 pipes 8 bpe @ SW_64K_S_T @ Navi1x
    {   1,    4,    5,    5,    0, } , // 1 pipes 16 bpe @ SW_64K_S_T @ Navi1x
    {   2,    0,   36,    1,    0, } , // 2 pipes 1 bpe @ SW_64K_S_T @ Navi1x
    {   2,    1,   37,    2,    0, } , // 2 pipes 2 bpe @ SW_64K_S_T @ Navi1x
    {   2,    2,   38,    3,    0, } , // 2 pipes 4 bpe @ SW_64K_S_T @ Navi1x
    {   2,    3,   39,    4,    0, } , // 2 pipes 8 bpe @ SW_64K_S_T @ Navi1x
    {   2,    4,   40,    5,    0, } , // 2 pipes 16 bpe @ SW_64K_S_T @ Navi1x
    {   2,    0,   41,    1,    0, } , // 4 pipes 1 bpe @ SW_64K_S_T @ Navi1x
    {   2,    1,   42,    2,    0, } , // 4 pipes 2 bpe @ SW_64K_S_T @ Navi1x
    {   2,    2,   43,    3,    0, } , // 4 pipes 4 bpe @ SW_64K_S_T @ Navi1x
    {   2,    3,   44,    4,    0, } , // 4 pipes 8 bpe @ SW_64K_S_T @ Navi1x
    {   2,    4,   45,    5,    0, } , // 4 pipes 16 bpe @ SW_64K_S_T @ Navi1x
    {   2,    0,   46,    1,    0, } , // 8 pipes 1 bpe @ SW_64K_S_T @ Navi1x
    {   2,    1,   47,    2,    0, } , // 8 pipes 2 bpe @ SW_64K_S_T @ Navi1x
    {   2,    2,   48,    3,    0, } , // 8 pipes 4 bpe @ SW_64K_S_T @ Navi1x
    {   2,    3,   49,    4,    0, } , // 8 pipes 8 bpe @ SW_64K_S_T @ Navi1x
    {   2,    4,   50,    5,    0, } , // 8 pipes 16 bpe @ SW_64K_S_T @ Navi1x
    {   2,    0,   51,    1,    0, } , // 16 pipes 1 bpe @ SW_64K_S_T @ Navi1x
    {   2,    1,   52,    2,    0, } , // 16 pipes 2 bpe @ SW_64K_S_T @ Navi1x
    {   2,    2,   53,    3,    0, } , // 16 pipes 4 bpe @ SW_64K_S_T @ Navi1x
    {   2,    3,   54,    4,    0, } , // 16 pipes 8 bpe @ SW_64K_S_T @ Navi1x
    {   2,    4,   55,    5,    0, } , // 16 pipes 16 bpe @ SW_64K_S_T @ Navi1x
    {   2,    0,   56,   16,    0, } , // 32 pipes 1 bpe @ SW_64K_S_T @ Navi1x
    {   2,    1,   57,   17,    0, } , // 32 pipes 2 bpe @ SW_64K_S_T @ Navi1x
    {   2,    2,   58,   18,    0, } , // 32 pipes 4 bpe @ SW_64K_S_T @ Navi1x
    {   2,    3,   59,   19,    0, } , // 32 pipes 8 bpe @ SW_64K_S_T @ Navi1x
    {   2,    4,   60,   20,    0, } , // 32 pipes 16 bpe @ SW_64K_S_T @ Navi1x
    {   2,    0,    1,   21,    0, } , // 64 pipes 1 bpe @ SW_64K_S_T @ Navi1x
    {   2,    1,    2,   22,    0, } , // 64 pipes 2 bpe @ SW_64K_S_T @ Navi1x
    {   2,    2,    3,   23,    0, } , // 64 pipes 4 bpe @ SW_64K_S_T @ Navi1x
    {   2,    3,    4,   24,    0, } , // 64 pipes 8 bpe @ SW_64K_S_T @ Navi1x
    {   2,    4,    5,   25,    0, } , // 64 pipes 16 bpe @ SW_64K_S_T @ Navi1x
};

const ADDR_SW_PATINFO GFX10_SW_64K_D_T_PATINFO[] =
{
    {   1,    5,    1,    1,    0, } , // 1 pipes 1 bpe @ SW_64K_D_T @ Navi1x
    {   1,    1,    2,    2,    0, } , // 1 pipes 2 bpe @ SW_64K_D_T @ Navi1x
    {   1,    2,    3,    3,    0, } , // 1 pipes 4 bpe @ SW_64K_D_T @ Navi1x
    {   1,    6,    4,    4,    0, } , // 1 pipes 8 bpe @ SW_64K_D_T @ Navi1x
    {   1,    7,    5,    5,    0, } , // 1 pipes 16 bpe @ SW_64K_D_T @ Navi1x
    {   2,    5,   36,    1,    0, } , // 2 pipes 1 bpe @ SW_64K_D_T @ Navi1x
    {   2,    1,   37,    2,    0, } , // 2 pipes 2 bpe @ SW_64K_D_T @ Navi1x
    {   2,    2,   38,    3,    0, } , // 2 pipes 4 bpe @ SW_64K_D_T @ Navi1x
    {   2,    6,   39,    4,    0, } , // 2 pipes 8 bpe @ SW_64K_D_T @ Navi1x
    {   2,    7,   40,    5,    0, } , // 2 pipes 16 bpe @ SW_64K_D_T @ Navi1x
    {   2,    5,   41,    1,    0, } , // 4 pipes 1 bpe @ SW_64K_D_T @ Navi1x
    {   2,    1,   42,    2,    0, } , // 4 pipes 2 bpe @ SW_64K_D_T @ Navi1x
    {   2,    2,   43,    3,    0, } , // 4 pipes 4 bpe @ SW_64K_D_T @ Navi1x
    {   2,    6,   44,    4,    0, } , // 4 pipes 8 bpe @ SW_64K_D_T @ Navi1x
    {   2,    7,   45,    5,    0, } , // 4 pipes 16 bpe @ SW_64K_D_T @ Navi1x
    {   2,    5,   46,    1,    0, } , // 8 pipes 1 bpe @ SW_64K_D_T @ Navi1x
    {   2,    1,   47,    2,    0, } , // 8 pipes 2 bpe @ SW_64K_D_T @ Navi1x
    {   2,    2,   48,    3,    0, } , // 8 pipes 4 bpe @ SW_64K_D_T @ Navi1x
    {   2,    6,   49,    4,    0, } , // 8 pipes 8 bpe @ SW_64K_D_T @ Navi1x
    {   2,    7,   50,    5,    0, } , // 8 pipes 16 bpe @ SW_64K_D_T @ Navi1x
    {   2,    5,   51,    1,    0, } , // 16 pipes 1 bpe @ SW_64K_D_T @ Navi1x
    {   2,    1,   52,    2,    0, } , // 16 pipes 2 bpe @ SW_64K_D_T @ Navi1x
    {   2,    2,   53,    3,    0, } , // 16 pipes 4 bpe @ SW_64K_D_T @ Navi1x
    {   2,    6,   54,    4,    0, } , // 16 pipes 8 bpe @ SW_64K_D_T @ Navi1x
    {   2,    7,   55,    5,    0, } , // 16 pipes 16 bpe @ SW_64K_D_T @ Navi1x
    {   2,    5,   56,   16,    0, } , // 32 pipes 1 bpe @ SW_64K_D_T @ Navi1x
    {   2,    1,   57,   17,    0, } , // 32 pipes 2 bpe @ SW_64K_D_T @ Navi1x
    {   2,    2,   58,   18,    0, } , // 32 pipes 4 bpe @ SW_64K_D_T @ Navi1x
    {   2,    6,   59,   19,    0, } , // 32 pipes 8 bpe @ SW_64K_D_T @ Navi1x
    {   2,    7,   60,   20,    0, } , // 32 pipes 16 bpe @ SW_64K_D_T @ Navi1x
    {   2,    5,    1,   21,    0, } , // 64 pipes 1 bpe @ SW_64K_D_T @ Navi1x
    {   2,    1,    2,   22,    0, } , // 64 pipes 2 bpe @ SW_64K_D_T @ Navi1x
    {   2,    2,    3,   23,    0, } , // 64 pipes 4 bpe @ SW_64K_D_T @ Navi1x
    {   2,    6,    4,   24,    0, } , // 64 pipes 8 bpe @ SW_64K_D_T @ Navi1x
    {   2,    7,    5,   25,    0, } , // 64 pipes 16 bpe @ SW_64K_D_T @ Navi1x
};

const ADDR_SW_PATINFO GFX10_SW_64K_S_X_PATINFO[] =
{
    {   1,    0,    1,    1,    0, } , // 1 pipes 1 bpe @ SW_64K_S_X @ Navi1x
    {   1,    1,    2,    2,    0, } , // 1 pipes 2 bpe @ SW_64K_S_X @ Navi1x
    {   1,    2,    3,    3,    0, } , // 1 pipes 4 bpe @ SW_64K_S_X @ Navi1x
    {   1,    3,    4,    4,    0, } , // 1 pipes 8 bpe @ SW_64K_S_X @ Navi1x
    {   1,    4,    5,    5,    0, } , // 1 pipes 16 bpe @ SW_64K_S_X @ Navi1x
    {   3,    0,    6,    1,    0, } , // 2 pipes 1 bpe @ SW_64K_S_X @ Navi1x
    {   3,    1,    7,    2,    0, } , // 2 pipes 2 bpe @ SW_64K_S_X @ Navi1x
    {   3,    2,    8,    3,    0, } , // 2 pipes 4 bpe @ SW_64K_S_X @ Navi1x
    {   3,    3,    9,    4,    0, } , // 2 pipes 8 bpe @ SW_64K_S_X @ Navi1x
    {   3,    4,   10,    5,    0, } , // 2 pipes 16 bpe @ SW_64K_S_X @ Navi1x
    {   3,    0,   11,    1,    0, } , // 4 pipes 1 bpe @ SW_64K_S_X @ Navi1x
    {   3,    1,   12,    2,    0, } , // 4 pipes 2 bpe @ SW_64K_S_X @ Navi1x
    {   3,    2,   13,    3,    0, } , // 4 pipes 4 bpe @ SW_64K_S_X @ Navi1x
    {   3,    3,   14,    4,    0, } , // 4 pipes 8 bpe @ SW_64K_S_X @ Navi1x
    {   3,    4,   15,    5,    0, } , // 4 pipes 16 bpe @ SW_64K_S_X @ Navi1x
    {   3,    0,   16,    1,    0, } , // 8 pipes 1 bpe @ SW_64K_S_X @ Navi1x
    {   3,    1,   17,    2,    0, } , // 8 pipes 2 bpe @ SW_64K_S_X @ Navi1x
    {   3,    2,   18,    3,    0, } , // 8 pipes 4 bpe @ SW_64K_S_X @ Navi1x
    {   3,    3,   19,    4,    0, } , // 8 pipes 8 bpe @ SW_64K_S_X @ Navi1x
    {   3,    4,   20,    5,    0, } , // 8 pipes 16 bpe @ SW_64K_S_X @ Navi1x
    {   3,    0,   21,    1,    0, } , // 16 pipes 1 bpe @ SW_64K_S_X @ Navi1x
    {   3,    1,   22,    2,    0, } , // 16 pipes 2 bpe @ SW_64K_S_X @ Navi1x
    {   3,    2,   23,    3,    0, } , // 16 pipes 4 bpe @ SW_64K_S_X @ Navi1x
    {   3,    3,   24,    4,    0, } , // 16 pipes 8 bpe @ SW_64K_S_X @ Navi1x
    {   3,    4,   25,    5,    0, } , // 16 pipes 16 bpe @ SW_64K_S_X @ Navi1x
    {   3,    0,   26,    6,    0, } , // 32 pipes 1 bpe @ SW_64K_S_X @ Navi1x
    {   3,    1,   27,    7,    0, } , // 32 pipes 2 bpe @ SW_64K_S_X @ Navi1x
    {   3,    2,   28,    8,    0, } , // 32 pipes 4 bpe @ SW_64K_S_X @ Navi1x
    {   3,    3,   29,    9,    0, } , // 32 pipes 8 bpe @ SW_64K_S_X @ Navi1x
    {   3,    4,   30,   10,    0, } , // 32 pipes 16 bpe @ SW_64K_S_X @ Navi1x
    {   3,    0,   31,   11,    0, } , // 64 pipes 1 bpe @ SW_64K_S_X @ Navi1x
    {   3,    1,   32,   12,    0, } , // 64 pipes 2 bpe @ SW_64K_S_X @ Navi1x
    {   3,    2,   33,   13,    0, } , // 64 pipes 4 bpe @ SW_64K_S_X @ Navi1x
    {   3,    3,   34,   14,    0, } , // 64 pipes 8 bpe @ SW_64K_S_X @ Navi1x
    {   3,    4,   35,   15,    0, } , // 64 pipes 16 bpe @ SW_64K_S_X @ Navi1x
};

const ADDR_SW_PATINFO GFX10_SW_64K_D_X_PATINFO[] =
{
    {   1,    5,    1,    1,    0, } , // 1 pipes 1 bpe @ SW_64K_D_X @ Navi1x
    {   1,    1,    2,    2,    0, } , // 1 pipes 2 bpe @ SW_64K_D_X @ Navi1x
    {   1,    2,    3,    3,    0, } , // 1 pipes 4 bpe @ SW_64K_D_X @ Navi1x
    {   1,    6,    4,    4,    0, } , // 1 pipes 8 bpe @ SW_64K_D_X @ Navi1x
    {   1,    7,    5,    5,    0, } , // 1 pipes 16 bpe @ SW_64K_D_X @ Navi1x
    {   3,    5,    6,    1,    0, } , // 2 pipes 1 bpe @ SW_64K_D_X @ Navi1x
    {   3,    1,    7,    2,    0, } , // 2 pipes 2 bpe @ SW_64K_D_X @ Navi1x
    {   3,    2,    8,    3,    0, } , // 2 pipes 4 bpe @ SW_64K_D_X @ Navi1x
    {   3,    6,    9,    4,    0, } , // 2 pipes 8 bpe @ SW_64K_D_X @ Navi1x
    {   3,    7,   10,    5,    0, } , // 2 pipes 16 bpe @ SW_64K_D_X @ Navi1x
    {   3,    5,   11,    1,    0, } , // 4 pipes 1 bpe @ SW_64K_D_X @ Navi1x
    {   3,    1,   12,    2,    0, } , // 4 pipes 2 bpe @ SW_64K_D_X @ Navi1x
    {   3,    2,   13,    3,    0, } , // 4 pipes 4 bpe @ SW_64K_D_X @ Navi1x
    {   3,    6,   14,    4,    0, } , // 4 pipes 8 bpe @ SW_64K_D_X @ Navi1x
    {   3,    7,   15,    5,    0, } , // 4 pipes 16 bpe @ SW_64K_D_X @ Navi1x
    {   3,    5,   16,    1,    0, } , // 8 pipes 1 bpe @ SW_64K_D_X @ Navi1x
    {   3,    1,   17,    2,    0, } , // 8 pipes 2 bpe @ SW_64K_D_X @ Navi1x
    {   3,    2,   18,    3,    0, } , // 8 pipes 4 bpe @ SW_64K_D_X @ Navi1x
    {   3,    6,   19,    4,    0, } , // 8 pipes 8 bpe @ SW_64K_D_X @ Navi1x
    {   3,    7,   20,    5,    0, } , // 8 pipes 16 bpe @ SW_64K_D_X @ Navi1x
    {   3,    5,   21,    1,    0, } , // 16 pipes 1 bpe @ SW_64K_D_X @ Navi1x
    {   3,    1,   22,    2,    0, } , // 16 pipes 2 bpe @ SW_64K_D_X @ Navi1x
    {   3,    2,   23,    3,    0, } , // 16 pipes 4 bpe @ SW_64K_D_X @ Navi1x
    {   3,    6,   24,    4,    0, } , // 16 pipes 8 bpe @ SW_64K_D_X @ Navi1x
    {   3,    7,   25,    5,    0, } , // 16 pipes 16 bpe @ SW_64K_D_X @ Navi1x
    {   3,    5,   26,    6,    0, } , // 32 pipes 1 bpe @ SW_64K_D_X @ Navi1x
    {   3,    1,   27,    7,    0, } , // 32 pipes 2 bpe @ SW_64K_D_X @ Navi1x
    {   3,    2,   28,    8,    0, } , // 32 pipes 4 bpe @ SW_64K_D_X @ Navi1x
    {   3,    6,   29,    9,    0, } , // 32 pipes 8 bpe @ SW_64K_D_X @ Navi1x
    {   3,    7,   30,   10,    0, } , // 32 pipes 16 bpe @ SW_64K_D_X @ Navi1x
    {   3,    5,   31,   11,    0, } , // 64 pipes 1 bpe @ SW_64K_D_X @ Navi1x
    {   3,    1,   32,   12,    0, } , // 64 pipes 2 bpe @ SW_64K_D_X @ Navi1x
    {   3,    2,   33,   13,    0, } , // 64 pipes 4 bpe @ SW_64K_D_X @ Navi1x
    {   3,    6,   34,   14,    0, } , // 64 pipes 8 bpe @ SW_64K_D_X @ Navi1x
    {   3,    7,   35,   15,    0, } , // 64 pipes 16 bpe @ SW_64K_D_X @ Navi1x
};

const ADDR_SW_PATINFO GFX10_SW_64K_R_X_1xaa_PATINFO[] =
{
    {   1,    5,    1,    1,    0, } , // 1 pipes 1 bpe @ SW_64K_R_X 1xaa @ Navi1x
    {   1,    1,    2,    2,    0, } , // 1 pipes 2 bpe @ SW_64K_R_X 1xaa @ Navi1x
    {   1,    2,    3,    3,    0, } , // 1 pipes 4 bpe @ SW_64K_R_X 1xaa @ Navi1x
    {   1,    6,    4,    4,    0, } , // 1 pipes 8 bpe @ SW_64K_R_X 1xaa @ Navi1x
    {   1,    7,    5,    5,    0, } , // 1 pipes 16 bpe @ SW_64K_R_X 1xaa @ Navi1x
    {   3,   28,   61,    1,    0, } , // 2 pipes 1 bpe @ SW_64K_R_X 1xaa @ Navi1x
    {   3,    1,   62,    2,    0, } , // 2 pipes 2 bpe @ SW_64K_R_X 1xaa @ Navi1x
    {   3,    2,    8,    3,    0, } , // 2 pipes 4 bpe @ SW_64K_R_X 1xaa @ Navi1x
    {   3,    6,   63,    4,    0, } , // 2 pipes 8 bpe @ SW_64K_R_X 1xaa @ Navi1x
    {   3,    7,   64,    5,    0, } , // 2 pipes 16 bpe @ SW_64K_R_X 1xaa @ Navi1x
    {   3,   28,   65,    1,    0, } , // 4 pipes 1 bpe @ SW_64K_R_X 1xaa @ Navi1x
    {   3,    1,   66,    2,    0, } , // 4 pipes 2 bpe @ SW_64K_R_X 1xaa @ Navi1x
    {   3,    2,   67,    3,    0, } , // 4 pipes 4 bpe @ SW_64K_R_X 1xaa @ Navi1x
    {   3,    6,   68,    4,    0, } , // 4 pipes 8 bpe @ SW_64K_R_X 1xaa @ Navi1x
    {   3,    7,   69,   26,    0, } , // 4 pipes 16 bpe @ SW_64K_R_X 1xaa @ Navi1x
    {   3,   28,   70,    1,    0, } , // 8 pipes 1 bpe @ SW_64K_R_X 1xaa @ Navi1x
    {   3,    1,   71,    2,    0, } , // 8 pipes 2 bpe @ SW_64K_R_X 1xaa @ Navi1x
    {   3,    2,   72,   27,    0, } , // 8 pipes 4 bpe @ SW_64K_R_X 1xaa @ Navi1x
    {   3,    6,   72,   28,    0, } , // 8 pipes 8 bpe @ SW_64K_R_X 1xaa @ Navi1x
    {   3,    7,   73,   29,    0, } , // 8 pipes 16 bpe @ SW_64K_R_X 1xaa @ Navi1x
    {   3,   28,   74,    1,    0, } , // 16 pipes 1 bpe @ SW_64K_R_X 1xaa @ Navi1x
    {   3,    1,   74,   30,    0, } , // 16 pipes 2 bpe @ SW_64K_R_X 1xaa @ Navi1x
    {   3,    2,   74,   31,    0, } , // 16 pipes 4 bpe @ SW_64K_R_X 1xaa @ Navi1x
    {   3,    6,   74,   32,    0, } , // 16 pipes 8 bpe @ SW_64K_R_X 1xaa @ Navi1x
    {   3,    7,   74,   33,    0, } , // 16 pipes 16 bpe @ SW_64K_R_X 1xaa @ Navi1x
    {   3,   28,   75,    6,    0, } , // 32 pipes 1 bpe @ SW_64K_R_X 1xaa @ Navi1x
    {   3,    1,   75,   34,    0, } , // 32 pipes 2 bpe @ SW_64K_R_X 1xaa @ Navi1x
    {   3,    2,   75,   35,    0, } , // 32 pipes 4 bpe @ SW_64K_R_X 1xaa @ Navi1x
    {   3,    6,   75,   36,    0, } , // 32 pipes 8 bpe @ SW_64K_R_X 1xaa @ Navi1x
    {   3,    7,   76,   37,    0, } , // 32 pipes 16 bpe @ SW_64K_R_X 1xaa @ Navi1x
    {   3,   28,   77,   11,    0, } , // 64 pipes 1 bpe @ SW_64K_R_X 1xaa @ Navi1x
    {   3,    1,   77,   38,    0, } , // 64 pipes 2 bpe @ SW_64K_R_X 1xaa @ Navi1x
    {   3,    2,   77,   39,    0, } , // 64 pipes 4 bpe @ SW_64K_R_X 1xaa @ Navi1x
    {   3,    6,   78,   40,    0, } , // 64 pipes 8 bpe @ SW_64K_R_X 1xaa @ Navi1x
    {   3,    7,   79,   41,    0, } , // 64 pipes 16 bpe @ SW_64K_R_X 1xaa @ Navi1x
};

const ADDR_SW_PATINFO GFX10_SW_64K_R_X_2xaa_PATINFO[] =
{
    {   2,    5,    1,   99,    0, } , // 1 pipes 1 bpe @ SW_64K_R_X 2xaa @ Navi1x
    {   2,    1,    2,  100,    0, } , // 1 pipes 2 bpe @ SW_64K_R_X 2xaa @ Navi1x
    {   2,    2,    3,  101,    0, } , // 1 pipes 4 bpe @ SW_64K_R_X 2xaa @ Navi1x
    {   2,    6,    4,  102,    0, } , // 1 pipes 8 bpe @ SW_64K_R_X 2xaa @ Navi1x
    {   2,    7,    5,  103,    0, } , // 1 pipes 16 bpe @ SW_64K_R_X 2xaa @ Navi1x
    {   3,   28,   61,   99,    0, } , // 2 pipes 1 bpe @ SW_64K_R_X 2xaa @ Navi1x
    {   3,    1,   62,  100,    0, } , // 2 pipes 2 bpe @ SW_64K_R_X 2xaa @ Navi1x
    {   3,    2,    8,  101,    0, } , // 2 pipes 4 bpe @ SW_64K_R_X 2xaa @ Navi1x
    {   3,    6,   63,  102,    0, } , // 2 pipes 8 bpe @ SW_64K_R_X 2xaa @ Navi1x
    {   3,    7,   64,  103,    0, } , // 2 pipes 16 bpe @ SW_64K_R_X 2xaa @ Navi1x
    {   3,   28,   65,   99,    0, } , // 4 pipes 1 bpe @ SW_64K_R_X 2xaa @ Navi1x
    {   3,    1,   66,  100,    0, } , // 4 pipes 2 bpe @ SW_64K_R_X 2xaa @ Navi1x
    {   3,    2,   67,  101,    0, } , // 4 pipes 4 bpe @ SW_64K_R_X 2xaa @ Navi1x
    {   3,    6,   68,  102,    0, } , // 4 pipes 8 bpe @ SW_64K_R_X 2xaa @ Navi1x
    {   3,    7,   69,  104,    0, } , // 4 pipes 16 bpe @ SW_64K_R_X 2xaa @ Navi1x
    {   3,   28,   70,   99,    0, } , // 8 pipes 1 bpe @ SW_64K_R_X 2xaa @ Navi1x
    {   3,    1,   71,  100,    0, } , // 8 pipes 2 bpe @ SW_64K_R_X 2xaa @ Navi1x
    {   3,    2,   72,  105,    0, } , // 8 pipes 4 bpe @ SW_64K_R_X 2xaa @ Navi1x
    {   3,    6,   72,  106,    0, } , // 8 pipes 8 bpe @ SW_64K_R_X 2xaa @ Navi1x
    {   3,    7,   73,  107,    0, } , // 8 pipes 16 bpe @ SW_64K_R_X 2xaa @ Navi1x
    {   3,   28,   74,   99,    0, } , // 16 pipes 1 bpe @ SW_64K_R_X 2xaa @ Navi1x
    {   3,    1,   74,  108,    0, } , // 16 pipes 2 bpe @ SW_64K_R_X 2xaa @ Navi1x
    {   3,    2,   74,  109,    0, } , // 16 pipes 4 bpe @ SW_64K_R_X 2xaa @ Navi1x
    {   3,    6,   74,  107,    0, } , // 16 pipes 8 bpe @ SW_64K_R_X 2xaa @ Navi1x
    {   3,    7,  113,   33,    0, } , // 16 pipes 16 bpe @ SW_64K_R_X 2xaa @ Navi1x
    {   3,   28,   75,  110,    0, } , // 32 pipes 1 bpe @ SW_64K_R_X 2xaa @ Navi1x
    {   3,    1,   75,  111,    0, } , // 32 pipes 2 bpe @ SW_64K_R_X 2xaa @ Navi1x
    {   3,    2,   75,  112,    0, } , // 32 pipes 4 bpe @ SW_64K_R_X 2xaa @ Navi1x
    {   3,    6,   76,  113,    0, } , // 32 pipes 8 bpe @ SW_64K_R_X 2xaa @ Navi1x
    {   3,    7,  114,   37,    0, } , // 32 pipes 16 bpe @ SW_64K_R_X 2xaa @ Navi1x
    {   3,   28,   78,  114,    0, } , // 64 pipes 1 bpe @ SW_64K_R_X 2xaa @ Navi1x
    {   3,    1,   78,  115,    0, } , // 64 pipes 2 bpe @ SW_64K_R_X 2xaa @ Navi1x
    {   3,    2,   78,  116,    0, } , // 64 pipes 4 bpe @ SW_64K_R_X 2xaa @ Navi1x
    {   3,    6,   79,  117,    0, } , // 64 pipes 8 bpe @ SW_64K_R_X 2xaa @ Navi1x
    {   3,    7,  115,   41,    0, } , // 64 pipes 16 bpe @ SW_64K_R_X 2xaa @ Navi1x
};

const ADDR_SW_PATINFO GFX10_SW_64K_R_X_4xaa_PATINFO[] =
{
    {   2,    5,    1,  118,    0, } , // 1 pipes 1 bpe @ SW_64K_R_X 4xaa @ Navi1x
    {   2,    1,    2,  119,    0, } , // 1 pipes 2 bpe @ SW_64K_R_X 4xaa @ Navi1x
    {   2,    2,    3,  120,    0, } , // 1 pipes 4 bpe @ SW_64K_R_X 4xaa @ Navi1x
    {   2,    6,    4,  121,    0, } , // 1 pipes 8 bpe @ SW_64K_R_X 4xaa @ Navi1x
    {   2,    7,    5,  122,    0, } , // 1 pipes 16 bpe @ SW_64K_R_X 4xaa @ Navi1x
    {   3,   28,   61,  118,    0, } , // 2 pipes 1 bpe @ SW_64K_R_X 4xaa @ Navi1x
    {   3,    1,   62,  119,    0, } , // 2 pipes 2 bpe @ SW_64K_R_X 4xaa @ Navi1x
    {   3,    2,    8,  120,    0, } , // 2 pipes 4 bpe @ SW_64K_R_X 4xaa @ Navi1x
    {   3,    6,   63,  121,    0, } , // 2 pipes 8 bpe @ SW_64K_R_X 4xaa @ Navi1x
    {   3,    7,   64,  122,    0, } , // 2 pipes 16 bpe @ SW_64K_R_X 4xaa @ Navi1x
    {   3,   28,   65,  118,    0, } , // 4 pipes 1 bpe @ SW_64K_R_X 4xaa @ Navi1x
    {   3,    1,   66,  119,    0, } , // 4 pipes 2 bpe @ SW_64K_R_X 4xaa @ Navi1x
    {   3,    2,   67,  120,    0, } , // 4 pipes 4 bpe @ SW_64K_R_X 4xaa @ Navi1x
    {   3,    6,   68,  121,    0, } , // 4 pipes 8 bpe @ SW_64K_R_X 4xaa @ Navi1x
    {   3,    7,   69,  123,    0, } , // 4 pipes 16 bpe @ SW_64K_R_X 4xaa @ Navi1x
    {   3,   28,   70,  118,    0, } , // 8 pipes 1 bpe @ SW_64K_R_X 4xaa @ Navi1x
    {   3,    1,   71,  119,    0, } , // 8 pipes 2 bpe @ SW_64K_R_X 4xaa @ Navi1x
    {   3,    2,   72,  124,    0, } , // 8 pipes 4 bpe @ SW_64K_R_X 4xaa @ Navi1x
    {   3,    6,   93,  125,    0, } , // 8 pipes 8 bpe @ SW_64K_R_X 4xaa @ Navi1x
    {   3,    7,  116,  107,    0, } , // 8 pipes 16 bpe @ SW_64K_R_X 4xaa @ Navi1x
    {   3,   28,   74,  118,    0, } , // 16 pipes 1 bpe @ SW_64K_R_X 4xaa @ Navi1x
    {   3,    1,   74,  126,    0, } , // 16 pipes 2 bpe @ SW_64K_R_X 4xaa @ Navi1x
    {   3,    2,   74,  127,    0, } , // 16 pipes 4 bpe @ SW_64K_R_X 4xaa @ Navi1x
    {   3,    6,  117,  107,    0, } , // 16 pipes 8 bpe @ SW_64K_R_X 4xaa @ Navi1x
    {   3,    7,  118,   33,    0, } , // 16 pipes 16 bpe @ SW_64K_R_X 4xaa @ Navi1x
    {   3,   28,   76,  128,    0, } , // 32 pipes 1 bpe @ SW_64K_R_X 4xaa @ Navi1x
    {   3,    1,   76,  129,    0, } , // 32 pipes 2 bpe @ SW_64K_R_X 4xaa @ Navi1x
    {   3,    2,   76,  130,    0, } , // 32 pipes 4 bpe @ SW_64K_R_X 4xaa @ Navi1x
    {   3,    6,  119,  113,    0, } , // 32 pipes 8 bpe @ SW_64K_R_X 4xaa @ Navi1x
    {   3,    7,  120,   37,    0, } , // 32 pipes 16 bpe @ SW_64K_R_X 4xaa @ Navi1x
    {   3,   28,   79,  131,    0, } , // 64 pipes 1 bpe @ SW_64K_R_X 4xaa @ Navi1x
    {   3,    1,   79,  132,    0, } , // 64 pipes 2 bpe @ SW_64K_R_X 4xaa @ Navi1x
    {   3,    2,   79,  133,    0, } , // 64 pipes 4 bpe @ SW_64K_R_X 4xaa @ Navi1x
    {   3,    6,  121,  117,    0, } , // 64 pipes 8 bpe @ SW_64K_R_X 4xaa @ Navi1x
    {   3,    7,  122,   41,    0, } , // 64 pipes 16 bpe @ SW_64K_R_X 4xaa @ Navi1x
};

const ADDR_SW_PATINFO GFX10_SW_64K_R_X_8xaa_PATINFO[] =
{
    {   2,    5,    1,  134,    0, } , // 1 pipes 1 bpe @ SW_64K_R_X 8xaa @ Navi1x
    {   2,    1,    2,  135,    0, } , // 1 pipes 2 bpe @ SW_64K_R_X 8xaa @ Navi1x
    {   2,    2,    3,  135,    0, } , // 1 pipes 4 bpe @ SW_64K_R_X 8xaa @ Navi1x
    {   2,    6,    4,  136,    0, } , // 1 pipes 8 bpe @ SW_64K_R_X 8xaa @ Navi1x
    {   2,    7,    5,  136,    0, } , // 1 pipes 16 bpe @ SW_64K_R_X 8xaa @ Navi1x
    {   3,   28,   61,  134,    0, } , // 2 pipes 1 bpe @ SW_64K_R_X 8xaa @ Navi1x
    {   3,    1,   62,  135,    0, } , // 2 pipes 2 bpe @ SW_64K_R_X 8xaa @ Navi1x
    {   3,    2,    8,  135,    0, } , // 2 pipes 4 bpe @ SW_64K_R_X 8xaa @ Navi1x
    {   3,    6,   63,  136,    0, } , // 2 pipes 8 bpe @ SW_64K_R_X 8xaa @ Navi1x
    {   3,    7,   64,  136,    0, } , // 2 pipes 16 bpe @ SW_64K_R_X 8xaa @ Navi1x
    {   3,   28,   65,  134,    0, } , // 4 pipes 1 bpe @ SW_64K_R_X 8xaa @ Navi1x
    {   3,    1,   66,  135,    0, } , // 4 pipes 2 bpe @ SW_64K_R_X 8xaa @ Navi1x
    {   3,    2,   67,  135,    0, } , // 4 pipes 4 bpe @ SW_64K_R_X 8xaa @ Navi1x
    {   3,    6,   68,  136,    0, } , // 4 pipes 8 bpe @ SW_64K_R_X 8xaa @ Navi1x
    {   3,    7,  102,  137,    0, } , // 4 pipes 16 bpe @ SW_64K_R_X 8xaa @ Navi1x
    {   3,   28,   70,  134,    0, } , // 8 pipes 1 bpe @ SW_64K_R_X 8xaa @ Navi1x
    {   3,    1,   71,  135,    0, } , // 8 pipes 2 bpe @ SW_64K_R_X 8xaa @ Navi1x
    {   3,    2,   72,  138,    0, } , // 8 pipes 4 bpe @ SW_64K_R_X 8xaa @ Navi1x
    {   3,    6,  123,  139,    0, } , // 8 pipes 8 bpe @ SW_64K_R_X 8xaa @ Navi1x
    {   3,    7,  124,  140,    0, } , // 8 pipes 16 bpe @ SW_64K_R_X 8xaa @ Navi1x
    {   3,   28,  105,  134,    0, } , // 16 pipes 1 bpe @ SW_64K_R_X 8xaa @ Navi1x
    {   3,    1,  105,  138,    0, } , // 16 pipes 2 bpe @ SW_64K_R_X 8xaa @ Navi1x
    {   3,    2,  125,  127,    0, } , // 16 pipes 4 bpe @ SW_64K_R_X 8xaa @ Navi1x
    {   3,    6,  126,  107,    0, } , // 16 pipes 8 bpe @ SW_64K_R_X 8xaa @ Navi1x
    {   3,    7,  126,  141,    0, } , // 16 pipes 16 bpe @ SW_64K_R_X 8xaa @ Navi1x
    {   3,   28,  107,  142,    0, } , // 32 pipes 1 bpe @ SW_64K_R_X 8xaa @ Navi1x
    {   3,    1,  108,  143,    0, } , // 32 pipes 2 bpe @ SW_64K_R_X 8xaa @ Navi1x
    {   3,    2,  127,  130,    0, } , // 32 pipes 4 bpe @ SW_64K_R_X 8xaa @ Navi1x
    {   3,    6,  128,  113,    0, } , // 32 pipes 8 bpe @ SW_64K_R_X 8xaa @ Navi1x
    {   3,    7,  128,  144,    0, } , // 32 pipes 16 bpe @ SW_64K_R_X 8xaa @ Navi1x
    {   3,   28,  110,  145,    0, } , // 64 pipes 1 bpe @ SW_64K_R_X 8xaa @ Navi1x
    {   3,    1,  111,  146,    0, } , // 64 pipes 2 bpe @ SW_64K_R_X 8xaa @ Navi1x
    {   3,    2,  129,  133,    0, } , // 64 pipes 4 bpe @ SW_64K_R_X 8xaa @ Navi1x
    {   3,    6,  130,  117,    0, } , // 64 pipes 8 bpe @ SW_64K_R_X 8xaa @ Navi1x
    {   3,    7,  130,  147,    0, } , // 64 pipes 16 bpe @ SW_64K_R_X 8xaa @ Navi1x
};

const ADDR_SW_PATINFO GFX10_SW_64K_Z_X_1xaa_PATINFO[] =
{
    {   1,    8,    1,    1,    0, } , // 1 pipes 1 bpe @ SW_64K_Z_X 1xaa @ Navi1x
    {   1,    9,    2,    2,    0, } , // 1 pipes 2 bpe @ SW_64K_Z_X 1xaa @ Navi1x
    {   1,   10,    3,    3,    0, } , // 1 pipes 4 bpe @ SW_64K_Z_X 1xaa @ Navi1x
    {   1,   11,    4,    4,    0, } , // 1 pipes 8 bpe @ SW_64K_Z_X 1xaa @ Navi1x
    {   1,    7,    5,    5,    0, } , // 1 pipes 16 bpe @ SW_64K_Z_X 1xaa @ Navi1x
    {   3,   12,   61,    1,    0, } , // 2 pipes 1 bpe @ SW_64K_Z_X 1xaa @ Navi1x
    {   3,    9,   62,    2,    0, } , // 2 pipes 2 bpe @ SW_64K_Z_X 1xaa @ Navi1x
    {   3,   10,    8,    3,    0, } , // 2 pipes 4 bpe @ SW_64K_Z_X 1xaa @ Navi1x
    {   3,   11,   63,    4,    0, } , // 2 pipes 8 bpe @ SW_64K_Z_X 1xaa @ Navi1x
    {   3,    7,   64,    5,    0, } , // 2 pipes 16 bpe @ SW_64K_Z_X 1xaa @ Navi1x
    {   3,   12,   65,    1,    0, } , // 4 pipes 1 bpe @ SW_64K_Z_X 1xaa @ Navi1x
    {   3,    9,   66,    2,    0, } , // 4 pipes 2 bpe @ SW_64K_Z_X 1xaa @ Navi1x
    {   3,   10,   67,    3,    0, } , // 4 pipes 4 bpe @ SW_64K_Z_X 1xaa @ Navi1x
    {   3,   11,   68,    4,    0, } , // 4 pipes 8 bpe @ SW_64K_Z_X 1xaa @ Navi1x
    {   3,    7,   69,   26,    0, } , // 4 pipes 16 bpe @ SW_64K_Z_X 1xaa @ Navi1x
    {   3,   12,   70,    1,    0, } , // 8 pipes 1 bpe @ SW_64K_Z_X 1xaa @ Navi1x
    {   3,    9,   71,    2,    0, } , // 8 pipes 2 bpe @ SW_64K_Z_X 1xaa @ Navi1x
    {   3,   10,   72,   27,    0, } , // 8 pipes 4 bpe @ SW_64K_Z_X 1xaa @ Navi1x
    {   3,   11,   72,   28,    0, } , // 8 pipes 8 bpe @ SW_64K_Z_X 1xaa @ Navi1x
    {   3,    7,   73,   29,    0, } , // 8 pipes 16 bpe @ SW_64K_Z_X 1xaa @ Navi1x
    {   3,   12,   74,    1,    0, } , // 16 pipes 1 bpe @ SW_64K_Z_X 1xaa @ Navi1x
    {   3,    9,   74,   30,    0, } , // 16 pipes 2 bpe @ SW_64K_Z_X 1xaa @ Navi1x
    {   3,   10,   74,   31,    0, } , // 16 pipes 4 bpe @ SW_64K_Z_X 1xaa @ Navi1x
    {   3,   11,   74,   32,    0, } , // 16 pipes 8 bpe @ SW_64K_Z_X 1xaa @ Navi1x
    {   3,    7,   74,   33,    0, } , // 16 pipes 16 bpe @ SW_64K_Z_X 1xaa @ Navi1x
    {   3,   12,   75,    6,    0, } , // 32 pipes 1 bpe @ SW_64K_Z_X 1xaa @ Navi1x
    {   3,    9,   75,   34,    0, } , // 32 pipes 2 bpe @ SW_64K_Z_X 1xaa @ Navi1x
    {   3,   10,   75,   35,    0, } , // 32 pipes 4 bpe @ SW_64K_Z_X 1xaa @ Navi1x
    {   3,   11,   75,   36,    0, } , // 32 pipes 8 bpe @ SW_64K_Z_X 1xaa @ Navi1x
    {   3,    7,   76,   37,    0, } , // 32 pipes 16 bpe @ SW_64K_Z_X 1xaa @ Navi1x
    {   3,   12,   77,   11,    0, } , // 64 pipes 1 bpe @ SW_64K_Z_X 1xaa @ Navi1x
    {   3,    9,   77,   38,    0, } , // 64 pipes 2 bpe @ SW_64K_Z_X 1xaa @ Navi1x
    {   3,   10,   77,   39,    0, } , // 64 pipes 4 bpe @ SW_64K_Z_X 1xaa @ Navi1x
    {   3,   11,   78,   40,    0, } , // 64 pipes 8 bpe @ SW_64K_Z_X 1xaa @ Navi1x
    {   3,    7,   79,   41,    0, } , // 64 pipes 16 bpe @ SW_64K_Z_X 1xaa @ Navi1x
};

const ADDR_SW_PATINFO GFX10_SW_64K_Z_X_2xaa_PATINFO[] =
{
    {   1,   13,   80,   42,    0, } , // 1 pipes 1 bpe @ SW_64K_Z_X 2xaa @ Navi1x
    {   1,   14,    3,    3,    0, } , // 1 pipes 2 bpe @ SW_64K_Z_X 2xaa @ Navi1x
    {   2,   15,    3,   43,    0, } , // 1 pipes 4 bpe @ SW_64K_Z_X 2xaa @ Navi1x
    {   2,   16,   81,   44,    0, } , // 1 pipes 8 bpe @ SW_64K_Z_X 2xaa @ Navi1x
    {   2,   17,    5,   45,    0, } , // 1 pipes 16 bpe @ SW_64K_Z_X 2xaa @ Navi1x
    {   3,   13,   82,   42,    0, } , // 2 pipes 1 bpe @ SW_64K_Z_X 2xaa @ Navi1x
    {   3,   14,    8,    3,    0, } , // 2 pipes 2 bpe @ SW_64K_Z_X 2xaa @ Navi1x
    {   3,   15,    8,   43,    0, } , // 2 pipes 4 bpe @ SW_64K_Z_X 2xaa @ Navi1x
    {   3,   16,   83,   44,    0, } , // 2 pipes 8 bpe @ SW_64K_Z_X 2xaa @ Navi1x
    {   3,   17,   64,   45,    0, } , // 2 pipes 16 bpe @ SW_64K_Z_X 2xaa @ Navi1x
    {   3,   13,   84,   42,    0, } , // 4 pipes 1 bpe @ SW_64K_Z_X 2xaa @ Navi1x
    {   3,   14,   67,    3,    0, } , // 4 pipes 2 bpe @ SW_64K_Z_X 2xaa @ Navi1x
    {   3,   15,   67,   43,    0, } , // 4 pipes 4 bpe @ SW_64K_Z_X 2xaa @ Navi1x
    {   3,   16,   85,   44,    0, } , // 4 pipes 8 bpe @ SW_64K_Z_X 2xaa @ Navi1x
    {   3,   17,   69,   46,    0, } , // 4 pipes 16 bpe @ SW_64K_Z_X 2xaa @ Navi1x
    {   3,   13,   86,   42,    0, } , // 8 pipes 1 bpe @ SW_64K_Z_X 2xaa @ Navi1x
    {   3,   14,   72,   27,    0, } , // 8 pipes 2 bpe @ SW_64K_Z_X 2xaa @ Navi1x
    {   3,   15,   72,   47,    0, } , // 8 pipes 4 bpe @ SW_64K_Z_X 2xaa @ Navi1x
    {   3,   16,   73,   48,    0, } , // 8 pipes 8 bpe @ SW_64K_Z_X 2xaa @ Navi1x
    {   3,   17,   73,   49,    0, } , // 8 pipes 16 bpe @ SW_64K_Z_X 2xaa @ Navi1x
    {   3,   13,   74,   50,    0, } , // 16 pipes 1 bpe @ SW_64K_Z_X 2xaa @ Navi1x
    {   3,   14,   74,   31,    0, } , // 16 pipes 2 bpe @ SW_64K_Z_X 2xaa @ Navi1x
    {   3,   15,   74,   51,    0, } , // 16 pipes 4 bpe @ SW_64K_Z_X 2xaa @ Navi1x
    {   3,   16,   74,   52,    0, } , // 16 pipes 8 bpe @ SW_64K_Z_X 2xaa @ Navi1x
    {   3,   17,   87,   53,    0, } , // 16 pipes 16 bpe @ SW_64K_Z_X 2xaa @ Navi1x
    {   3,   13,   75,   54,    0, } , // 32 pipes 1 bpe @ SW_64K_Z_X 2xaa @ Navi1x
    {   3,   14,   75,   35,    0, } , // 32 pipes 2 bpe @ SW_64K_Z_X 2xaa @ Navi1x
    {   3,   15,   75,   55,    0, } , // 32 pipes 4 bpe @ SW_64K_Z_X 2xaa @ Navi1x
    {   3,   16,   76,   56,    0, } , // 32 pipes 8 bpe @ SW_64K_Z_X 2xaa @ Navi1x
    {   3,   17,   88,   57,    0, } , // 32 pipes 16 bpe @ SW_64K_Z_X 2xaa @ Navi1x
    {   3,   13,   78,   58,    0, } , // 64 pipes 1 bpe @ SW_64K_Z_X 2xaa @ Navi1x
    {   3,   14,   78,   59,    0, } , // 64 pipes 2 bpe @ SW_64K_Z_X 2xaa @ Navi1x
    {   3,   15,   78,   60,    0, } , // 64 pipes 4 bpe @ SW_64K_Z_X 2xaa @ Navi1x
    {   3,   16,   79,   41,    0, } , // 64 pipes 8 bpe @ SW_64K_Z_X 2xaa @ Navi1x
    {   3,   17,   89,   61,    0, } , // 64 pipes 16 bpe @ SW_64K_Z_X 2xaa @ Navi1x
};

const ADDR_SW_PATINFO GFX10_SW_64K_Z_X_4xaa_PATINFO[] =
{
    {   1,   18,    3,    3,    0, } , // 1 pipes 1 bpe @ SW_64K_Z_X 4xaa @ Navi1x
    {   2,   19,   90,   62,    0, } , // 1 pipes 2 bpe @ SW_64K_Z_X 4xaa @ Navi1x
    {   2,   20,    3,   63,    0, } , // 1 pipes 4 bpe @ SW_64K_Z_X 4xaa @ Navi1x
    {   2,   21,    4,   64,    0, } , // 1 pipes 8 bpe @ SW_64K_Z_X 4xaa @ Navi1x
    {   2,   22,    5,   65,    0, } , // 1 pipes 16 bpe @ SW_64K_Z_X 4xaa @ Navi1x
    {   3,   18,    8,    3,    0, } , // 2 pipes 1 bpe @ SW_64K_Z_X 4xaa @ Navi1x
    {   3,   19,   91,   62,    0, } , // 2 pipes 2 bpe @ SW_64K_Z_X 4xaa @ Navi1x
    {   3,   20,    8,   66,    0, } , // 2 pipes 4 bpe @ SW_64K_Z_X 4xaa @ Navi1x
    {   3,   21,   63,   67,    0, } , // 2 pipes 8 bpe @ SW_64K_Z_X 4xaa @ Navi1x
    {   3,   22,   64,   68,    0, } , // 2 pipes 16 bpe @ SW_64K_Z_X 4xaa @ Navi1x
    {   3,   18,   67,    3,    0, } , // 4 pipes 1 bpe @ SW_64K_Z_X 4xaa @ Navi1x
    {   3,   19,   92,   62,    0, } , // 4 pipes 2 bpe @ SW_64K_Z_X 4xaa @ Navi1x
    {   3,   20,   67,   63,    0, } , // 4 pipes 4 bpe @ SW_64K_Z_X 4xaa @ Navi1x
    {   3,   21,   68,   64,    0, } , // 4 pipes 8 bpe @ SW_64K_Z_X 4xaa @ Navi1x
    {   3,   22,   69,   69,    0, } , // 4 pipes 16 bpe @ SW_64K_Z_X 4xaa @ Navi1x
    {   3,   18,   72,   27,    0, } , // 8 pipes 1 bpe @ SW_64K_Z_X 4xaa @ Navi1x
    {   3,   19,   72,   70,    0, } , // 8 pipes 2 bpe @ SW_64K_Z_X 4xaa @ Navi1x
    {   3,   20,   72,   71,    0, } , // 8 pipes 4 bpe @ SW_64K_Z_X 4xaa @ Navi1x
    {   3,   21,   93,   72,    0, } , // 8 pipes 8 bpe @ SW_64K_Z_X 4xaa @ Navi1x
    {   3,   22,   94,   73,    0, } , // 8 pipes 16 bpe @ SW_64K_Z_X 4xaa @ Navi1x
    {   3,   18,   74,   31,    0, } , // 16 pipes 1 bpe @ SW_64K_Z_X 4xaa @ Navi1x
    {   3,   19,   74,   74,    0, } , // 16 pipes 2 bpe @ SW_64K_Z_X 4xaa @ Navi1x
    {   3,   20,   74,   75,    0, } , // 16 pipes 4 bpe @ SW_64K_Z_X 4xaa @ Navi1x
    {   3,   21,   95,   76,    0, } , // 16 pipes 8 bpe @ SW_64K_Z_X 4xaa @ Navi1x
    {   3,   22,   96,   76,    0, } , // 16 pipes 16 bpe @ SW_64K_Z_X 4xaa @ Navi1x
    {   3,   18,   76,   77,    0, } , // 32 pipes 1 bpe @ SW_64K_Z_X 4xaa @ Navi1x
    {   3,   19,   76,   78,    0, } , // 32 pipes 2 bpe @ SW_64K_Z_X 4xaa @ Navi1x
    {   3,   20,   76,   56,    0, } , // 32 pipes 4 bpe @ SW_64K_Z_X 4xaa @ Navi1x
    {   3,   21,   97,   79,    0, } , // 32 pipes 8 bpe @ SW_64K_Z_X 4xaa @ Navi1x
    {   3,   22,   98,   79,    0, } , // 32 pipes 16 bpe @ SW_64K_Z_X 4xaa @ Navi1x
    {   3,   18,   79,   80,    0, } , // 64 pipes 1 bpe @ SW_64K_Z_X 4xaa @ Navi1x
    {   3,   19,   79,   81,    0, } , // 64 pipes 2 bpe @ SW_64K_Z_X 4xaa @ Navi1x
    {   3,   20,   79,   41,    0, } , // 64 pipes 4 bpe @ SW_64K_Z_X 4xaa @ Navi1x
    {   3,   21,   99,   82,    0, } , // 64 pipes 8 bpe @ SW_64K_Z_X 4xaa @ Navi1x
    {   3,   22,  100,   82,    0, } , // 64 pipes 16 bpe @ SW_64K_Z_X 4xaa @ Navi1x
};

const ADDR_SW_PATINFO GFX10_SW_64K_Z_X_8xaa_PATINFO[] =
{
    {   2,   23,    3,   43,    0, } , // 1 pipes 1 bpe @ SW_64K_Z_X 8xaa @ Navi1x
    {   2,   24,    3,   63,    0, } , // 1 pipes 2 bpe @ SW_64K_Z_X 8xaa @ Navi1x
    {   2,   25,    3,   83,    0, } , // 1 pipes 4 bpe @ SW_64K_Z_X 8xaa @ Navi1x
    {   2,   26,   81,   84,    0, } , // 1 pipes 8 bpe @ SW_64K_Z_X 8xaa @ Navi1x
    {   2,   27,    5,   85,    0, } , // 1 pipes 16 bpe @ SW_64K_Z_X 8xaa @ Navi1x
    {   3,   23,    8,   43,    0, } , // 2 pipes 1 bpe @ SW_64K_Z_X 8xaa @ Navi1x
    {   3,   24,    8,   66,    0, } , // 2 pipes 2 bpe @ SW_64K_Z_X 8xaa @ Navi1x
    {   3,   25,    8,   86,    0, } , // 2 pipes 4 bpe @ SW_64K_Z_X 8xaa @ Navi1x
    {   3,   26,  101,   87,    0, } , // 2 pipes 8 bpe @ SW_64K_Z_X 8xaa @ Navi1x
    {   3,   27,   64,   88,    0, } , // 2 pipes 16 bpe @ SW_64K_Z_X 8xaa @ Navi1x
    {   3,   23,   67,   43,    0, } , // 4 pipes 1 bpe @ SW_64K_Z_X 8xaa @ Navi1x
    {   3,   24,   67,   63,    0, } , // 4 pipes 2 bpe @ SW_64K_Z_X 8xaa @ Navi1x
    {   3,   25,   67,   83,    0, } , // 4 pipes 4 bpe @ SW_64K_Z_X 8xaa @ Navi1x
    {   3,   26,   85,   84,    0, } , // 4 pipes 8 bpe @ SW_64K_Z_X 8xaa @ Navi1x
    {   3,   27,  102,   89,    0, } , // 4 pipes 16 bpe @ SW_64K_Z_X 8xaa @ Navi1x
    {   3,   23,   72,   47,    0, } , // 8 pipes 1 bpe @ SW_64K_Z_X 8xaa @ Navi1x
    {   3,   24,   72,   71,    0, } , // 8 pipes 2 bpe @ SW_64K_Z_X 8xaa @ Navi1x
    {   3,   25,   72,   90,    0, } , // 8 pipes 4 bpe @ SW_64K_Z_X 8xaa @ Navi1x
    {   3,   26,  103,   91,    0, } , // 8 pipes 8 bpe @ SW_64K_Z_X 8xaa @ Navi1x
    {   3,   27,  104,   92,    0, } , // 8 pipes 16 bpe @ SW_64K_Z_X 8xaa @ Navi1x
    {   3,   23,  105,   51,    0, } , // 16 pipes 1 bpe @ SW_64K_Z_X 8xaa @ Navi1x
    {   3,   24,  105,   75,    0, } , // 16 pipes 2 bpe @ SW_64K_Z_X 8xaa @ Navi1x
    {   3,   25,   87,   93,    0, } , // 16 pipes 4 bpe @ SW_64K_Z_X 8xaa @ Navi1x
    {   3,   26,   96,   76,    0, } , // 16 pipes 8 bpe @ SW_64K_Z_X 8xaa @ Navi1x
    {   3,   27,  106,   94,    0, } , // 16 pipes 16 bpe @ SW_64K_Z_X 8xaa @ Navi1x
    {   3,   23,  107,   95,    0, } , // 32 pipes 1 bpe @ SW_64K_Z_X 8xaa @ Navi1x
    {   3,   24,  108,   56,    0, } , // 32 pipes 2 bpe @ SW_64K_Z_X 8xaa @ Navi1x
    {   3,   25,   88,   57,    0, } , // 32 pipes 4 bpe @ SW_64K_Z_X 8xaa @ Navi1x
    {   3,   26,   98,   79,    0, } , // 32 pipes 8 bpe @ SW_64K_Z_X 8xaa @ Navi1x
    {   3,   27,  109,   96,    0, } , // 32 pipes 16 bpe @ SW_64K_Z_X 8xaa @ Navi1x
    {   3,   23,  110,   97,    0, } , // 64 pipes 1 bpe @ SW_64K_Z_X 8xaa @ Navi1x
    {   3,   24,  111,   41,    0, } , // 64 pipes 2 bpe @ SW_64K_Z_X 8xaa @ Navi1x
    {   3,   25,   89,   61,    0, } , // 64 pipes 4 bpe @ SW_64K_Z_X 8xaa @ Navi1x
    {   3,   26,  100,   82,    0, } , // 64 pipes 8 bpe @ SW_64K_Z_X 8xaa @ Navi1x
    {   3,   27,  112,   98,    0, } , // 64 pipes 16 bpe @ SW_64K_Z_X 8xaa @ Navi1x
};

const ADDR_SW_PATINFO GFX10_SW_64K_S3_PATINFO[] =
{
    {   1,   29,  131,  148,    0, } , // 1 pipes 1 bpe @ SW_64K_S3 @ Navi1x
    {   1,   30,  132,  149,    0, } , // 1 pipes 2 bpe @ SW_64K_S3 @ Navi1x
    {   1,   31,  133,  150,    0, } , // 1 pipes 4 bpe @ SW_64K_S3 @ Navi1x
    {   1,   32,  134,  151,    0, } , // 1 pipes 8 bpe @ SW_64K_S3 @ Navi1x
    {   1,   33,  135,  152,    0, } , // 1 pipes 16 bpe @ SW_64K_S3 @ Navi1x
    {   1,   29,  131,  148,    0, } , // 2 pipes 1 bpe @ SW_64K_S3 @ Navi1x
    {   1,   30,  132,  149,    0, } , // 2 pipes 2 bpe @ SW_64K_S3 @ Navi1x
    {   1,   31,  133,  150,    0, } , // 2 pipes 4 bpe @ SW_64K_S3 @ Navi1x
    {   1,   32,  134,  151,    0, } , // 2 pipes 8 bpe @ SW_64K_S3 @ Navi1x
    {   1,   33,  135,  152,    0, } , // 2 pipes 16 bpe @ SW_64K_S3 @ Navi1x
    {   1,   29,  131,  148,    0, } , // 4 pipes 1 bpe @ SW_64K_S3 @ Navi1x
    {   1,   30,  132,  149,    0, } , // 4 pipes 2 bpe @ SW_64K_S3 @ Navi1x
    {   1,   31,  133,  150,    0, } , // 4 pipes 4 bpe @ SW_64K_S3 @ Navi1x
    {   1,   32,  134,  151,    0, } , // 4 pipes 8 bpe @ SW_64K_S3 @ Navi1x
    {   1,   33,  135,  152,    0, } , // 4 pipes 16 bpe @ SW_64K_S3 @ Navi1x
    {   1,   29,  131,  148,    0, } , // 8 pipes 1 bpe @ SW_64K_S3 @ Navi1x
    {   1,   30,  132,  149,    0, } , // 8 pipes 2 bpe @ SW_64K_S3 @ Navi1x
    {   1,   31,  133,  150,    0, } , // 8 pipes 4 bpe @ SW_64K_S3 @ Navi1x
    {   1,   32,  134,  151,    0, } , // 8 pipes 8 bpe @ SW_64K_S3 @ Navi1x
    {   1,   33,  135,  152,    0, } , // 8 pipes 16 bpe @ SW_64K_S3 @ Navi1x
    {   1,   29,  131,  148,    0, } , // 16 pipes 1 bpe @ SW_64K_S3 @ Navi1x
    {   1,   30,  132,  149,    0, } , // 16 pipes 2 bpe @ SW_64K_S3 @ Navi1x
    {   1,   31,  133,  150,    0, } , // 16 pipes 4 bpe @ SW_64K_S3 @ Navi1x
    {   1,   32,  134,  151,    0, } , // 16 pipes 8 bpe @ SW_64K_S3 @ Navi1x
    {   1,   33,  135,  152,    0, } , // 16 pipes 16 bpe @ SW_64K_S3 @ Navi1x
    {   1,   29,  131,  148,    0, } , // 32 pipes 1 bpe @ SW_64K_S3 @ Navi1x
    {   1,   30,  132,  149,    0, } , // 32 pipes 2 bpe @ SW_64K_S3 @ Navi1x
    {   1,   31,  133,  150,    0, } , // 32 pipes 4 bpe @ SW_64K_S3 @ Navi1x
    {   1,   32,  134,  151,    0, } , // 32 pipes 8 bpe @ SW_64K_S3 @ Navi1x
    {   1,   33,  135,  152,    0, } , // 32 pipes 16 bpe @ SW_64K_S3 @ Navi1x
    {   1,   29,  131,  148,    0, } , // 64 pipes 1 bpe @ SW_64K_S3 @ Navi1x
    {   1,   30,  132,  149,    0, } , // 64 pipes 2 bpe @ SW_64K_S3 @ Navi1x
    {   1,   31,  133,  150,    0, } , // 64 pipes 4 bpe @ SW_64K_S3 @ Navi1x
    {   1,   32,  134,  151,    0, } , // 64 pipes 8 bpe @ SW_64K_S3 @ Navi1x
    {   1,   33,  135,  152,    0, } , // 64 pipes 16 bpe @ SW_64K_S3 @ Navi1x
};

const ADDR_SW_PATINFO GFX10_SW_64K_S3_X_PATINFO[] =
{
    {   1,   29,  131,  148,    0, } , // 1 pipes 1 bpe @ SW_64K_S3_X @ Navi1x
    {   1,   30,  132,  149,    0, } , // 1 pipes 2 bpe @ SW_64K_S3_X @ Navi1x
    {   1,   31,  133,  150,    0, } , // 1 pipes 4 bpe @ SW_64K_S3_X @ Navi1x
    {   1,   32,  134,  151,    0, } , // 1 pipes 8 bpe @ SW_64K_S3_X @ Navi1x
    {   1,   33,  135,  152,    0, } , // 1 pipes 16 bpe @ SW_64K_S3_X @ Navi1x
    {   3,   29,  136,  148,    0, } , // 2 pipes 1 bpe @ SW_64K_S3_X @ Navi1x
    {   3,   30,  137,  149,    0, } , // 2 pipes 2 bpe @ SW_64K_S3_X @ Navi1x
    {   3,   31,  138,  150,    0, } , // 2 pipes 4 bpe @ SW_64K_S3_X @ Navi1x
    {   3,   32,  139,  151,    0, } , // 2 pipes 8 bpe @ SW_64K_S3_X @ Navi1x
    {   3,   33,  140,  152,    0, } , // 2 pipes 16 bpe @ SW_64K_S3_X @ Navi1x
    {   3,   29,  141,  148,    0, } , // 4 pipes 1 bpe @ SW_64K_S3_X @ Navi1x
    {   3,   30,  142,  149,    0, } , // 4 pipes 2 bpe @ SW_64K_S3_X @ Navi1x
    {   3,   31,  143,  150,    0, } , // 4 pipes 4 bpe @ SW_64K_S3_X @ Navi1x
    {   3,   32,  144,  151,    0, } , // 4 pipes 8 bpe @ SW_64K_S3_X @ Navi1x
    {   3,   33,  145,  152,    0, } , // 4 pipes 16 bpe @ SW_64K_S3_X @ Navi1x
    {   3,   29,  146,  148,    0, } , // 8 pipes 1 bpe @ SW_64K_S3_X @ Navi1x
    {   3,   30,  147,  149,    0, } , // 8 pipes 2 bpe @ SW_64K_S3_X @ Navi1x
    {   3,   31,  148,  150,    0, } , // 8 pipes 4 bpe @ SW_64K_S3_X @ Navi1x
    {   3,   32,  149,  151,    0, } , // 8 pipes 8 bpe @ SW_64K_S3_X @ Navi1x
    {   3,   33,  150,  152,    0, } , // 8 pipes 16 bpe @ SW_64K_S3_X @ Navi1x
    {   3,   29,  151,  148,    0, } , // 16 pipes 1 bpe @ SW_64K_S3_X @ Navi1x
    {   3,   30,  152,  149,    0, } , // 16 pipes 2 bpe @ SW_64K_S3_X @ Navi1x
    {   3,   31,  153,  150,    0, } , // 16 pipes 4 bpe @ SW_64K_S3_X @ Navi1x
    {   3,   32,  154,  151,    0, } , // 16 pipes 8 bpe @ SW_64K_S3_X @ Navi1x
    {   3,   33,  155,  152,    0, } , // 16 pipes 16 bpe @ SW_64K_S3_X @ Navi1x
    {   3,   29,  156,  153,    0, } , // 32 pipes 1 bpe @ SW_64K_S3_X @ Navi1x
    {   3,   30,  157,  154,    0, } , // 32 pipes 2 bpe @ SW_64K_S3_X @ Navi1x
    {   3,   31,  158,  155,    0, } , // 32 pipes 4 bpe @ SW_64K_S3_X @ Navi1x
    {   3,   32,  159,  156,    0, } , // 32 pipes 8 bpe @ SW_64K_S3_X @ Navi1x
    {   3,   33,  160,  157,    0, } , // 32 pipes 16 bpe @ SW_64K_S3_X @ Navi1x
    {   3,   29,  161,  158,    0, } , // 64 pipes 1 bpe @ SW_64K_S3_X @ Navi1x
    {   3,   30,  162,  159,    0, } , // 64 pipes 2 bpe @ SW_64K_S3_X @ Navi1x
    {   3,   31,  163,  160,    0, } , // 64 pipes 4 bpe @ SW_64K_S3_X @ Navi1x
    {   3,   32,  164,  161,    0, } , // 64 pipes 8 bpe @ SW_64K_S3_X @ Navi1x
    {   3,   33,  165,  162,    0, } , // 64 pipes 16 bpe @ SW_64K_S3_X @ Navi1x
};

const ADDR_SW_PATINFO GFX10_SW_64K_S3_T_PATINFO[] =
{
    {   1,   29,  131,  148,    0, } , // 1 pipes 1 bpe @ SW_64K_S3_T @ Navi1x
    {   1,   30,  132,  149,    0, } , // 1 pipes 2 bpe @ SW_64K_S3_T @ Navi1x
    {   1,   31,  133,  150,    0, } , // 1 pipes 4 bpe @ SW_64K_S3_T @ Navi1x
    {   1,   32,  134,  151,    0, } , // 1 pipes 8 bpe @ SW_64K_S3_T @ Navi1x
    {   1,   33,  135,  152,    0, } , // 1 pipes 16 bpe @ SW_64K_S3_T @ Navi1x
    {   3,   29,  136,  148,    0, } , // 2 pipes 1 bpe @ SW_64K_S3_T @ Navi1x
    {   3,   30,  137,  149,    0, } , // 2 pipes 2 bpe @ SW_64K_S3_T @ Navi1x
    {   3,   31,  138,  150,    0, } , // 2 pipes 4 bpe @ SW_64K_S3_T @ Navi1x
    {   3,   32,  139,  151,    0, } , // 2 pipes 8 bpe @ SW_64K_S3_T @ Navi1x
    {   3,   33,  140,  152,    0, } , // 2 pipes 16 bpe @ SW_64K_S3_T @ Navi1x
    {   3,   29,  141,  148,    0, } , // 4 pipes 1 bpe @ SW_64K_S3_T @ Navi1x
    {   3,   30,  142,  149,    0, } , // 4 pipes 2 bpe @ SW_64K_S3_T @ Navi1x
    {   3,   31,  143,  150,    0, } , // 4 pipes 4 bpe @ SW_64K_S3_T @ Navi1x
    {   3,   32,  144,  151,    0, } , // 4 pipes 8 bpe @ SW_64K_S3_T @ Navi1x
    {   3,   33,  145,  152,    0, } , // 4 pipes 16 bpe @ SW_64K_S3_T @ Navi1x
    {   3,   29,  166,  148,    0, } , // 8 pipes 1 bpe @ SW_64K_S3_T @ Navi1x
    {   3,   30,  167,  149,    0, } , // 8 pipes 2 bpe @ SW_64K_S3_T @ Navi1x
    {   3,   31,  168,  150,    0, } , // 8 pipes 4 bpe @ SW_64K_S3_T @ Navi1x
    {   3,   32,  169,  151,    0, } , // 8 pipes 8 bpe @ SW_64K_S3_T @ Navi1x
    {   3,   33,  170,  152,    0, } , // 8 pipes 16 bpe @ SW_64K_S3_T @ Navi1x
    {   3,   29,  171,  148,    0, } , // 16 pipes 1 bpe @ SW_64K_S3_T @ Navi1x
    {   3,   30,  172,  149,    0, } , // 16 pipes 2 bpe @ SW_64K_S3_T @ Navi1x
    {   3,   31,  173,  150,    0, } , // 16 pipes 4 bpe @ SW_64K_S3_T @ Navi1x
    {   3,   32,  174,  151,    0, } , // 16 pipes 8 bpe @ SW_64K_S3_T @ Navi1x
    {   3,   33,  175,  152,    0, } , // 16 pipes 16 bpe @ SW_64K_S3_T @ Navi1x
    {   3,   29,  176,  153,    0, } , // 32 pipes 1 bpe @ SW_64K_S3_T @ Navi1x
    {   3,   30,  177,  154,    0, } , // 32 pipes 2 bpe @ SW_64K_S3_T @ Navi1x
    {   3,   31,  178,  155,    0, } , // 32 pipes 4 bpe @ SW_64K_S3_T @ Navi1x
    {   3,   32,  179,  156,    0, } , // 32 pipes 8 bpe @ SW_64K_S3_T @ Navi1x
    {   3,   33,  180,  157,    0, } , // 32 pipes 16 bpe @ SW_64K_S3_T @ Navi1x
    {   3,   29,  131,  163,    0, } , // 64 pipes 1 bpe @ SW_64K_S3_T @ Navi1x
    {   3,   30,  132,  164,    0, } , // 64 pipes 2 bpe @ SW_64K_S3_T @ Navi1x
    {   3,   31,  133,  165,    0, } , // 64 pipes 4 bpe @ SW_64K_S3_T @ Navi1x
    {   3,   32,  134,  166,    0, } , // 64 pipes 8 bpe @ SW_64K_S3_T @ Navi1x
    {   3,   33,  135,  167,    0, } , // 64 pipes 16 bpe @ SW_64K_S3_T @ Navi1x
};

const ADDR_SW_PATINFO GFX10_SW_64K_D3_X_PATINFO[] =
{
    {   1,   34,  131,  148,    0, } , // 1 pipes 1 bpe @ SW_64K_D3_X @ Navi1x
    {   1,   35,  132,  149,    0, } , // 1 pipes 2 bpe @ SW_64K_D3_X @ Navi1x
    {   1,   36,  133,  150,    0, } , // 1 pipes 4 bpe @ SW_64K_D3_X @ Navi1x
    {   1,   37,  134,  151,    0, } , // 1 pipes 8 bpe @ SW_64K_D3_X @ Navi1x
    {   1,   38,  135,  152,    0, } , // 1 pipes 16 bpe @ SW_64K_D3_X @ Navi1x
    {   2,   34,  181,  148,    0, } , // 2 pipes 1 bpe @ SW_64K_D3_X @ Navi1x
    {   2,   35,  182,  149,    0, } , // 2 pipes 2 bpe @ SW_64K_D3_X @ Navi1x
    {   2,   36,  183,  150,    0, } , // 2 pipes 4 bpe @ SW_64K_D3_X @ Navi1x
    {   2,   37,  184,  168,    0, } , // 2 pipes 8 bpe @ SW_64K_D3_X @ Navi1x
    {   2,   38,  185,  169,    0, } , // 2 pipes 16 bpe @ SW_64K_D3_X @ Navi1x
    {   2,   34,  186,  170,    0, } , // 4 pipes 1 bpe @ SW_64K_D3_X @ Navi1x
    {   2,   35,  186,  171,    0, } , // 4 pipes 2 bpe @ SW_64K_D3_X @ Navi1x
    {   2,   36,  187,  172,    0, } , // 4 pipes 4 bpe @ SW_64K_D3_X @ Navi1x
    {   2,   37,  188,  169,    0, } , // 4 pipes 8 bpe @ SW_64K_D3_X @ Navi1x
    {   3,   38,  189,  169,    0, } , // 4 pipes 16 bpe @ SW_64K_D3_X @ Navi1x
    {   2,   34,  190,  173,    0, } , // 8 pipes 1 bpe @ SW_64K_D3_X @ Navi1x
    {   3,   35,  191,  171,    0, } , // 8 pipes 2 bpe @ SW_64K_D3_X @ Navi1x
    {   3,   36,  192,  172,    0, } , // 8 pipes 4 bpe @ SW_64K_D3_X @ Navi1x
    {   3,   37,  193,  169,    0, } , // 8 pipes 8 bpe @ SW_64K_D3_X @ Navi1x
    {   3,   38,  194,  169,    0, } , // 8 pipes 16 bpe @ SW_64K_D3_X @ Navi1x
    {   3,   34,  195,  174,    0, } , // 16 pipes 1 bpe @ SW_64K_D3_X @ Navi1x
    {   3,   35,  196,  171,    0, } , // 16 pipes 2 bpe @ SW_64K_D3_X @ Navi1x
    {   3,   36,  197,  172,    0, } , // 16 pipes 4 bpe @ SW_64K_D3_X @ Navi1x
    {   3,   37,  198,  169,    0, } , // 16 pipes 8 bpe @ SW_64K_D3_X @ Navi1x
    {   3,   38,  199,  169,    0, } , // 16 pipes 16 bpe @ SW_64K_D3_X @ Navi1x
    {   3,   34,  200,  175,    0, } , // 32 pipes 1 bpe @ SW_64K_D3_X @ Navi1x
    {   3,   35,  201,  176,    0, } , // 32 pipes 2 bpe @ SW_64K_D3_X @ Navi1x
    {   3,   36,  202,  177,    0, } , // 32 pipes 4 bpe @ SW_64K_D3_X @ Navi1x
    {   3,   37,  203,  178,    0, } , // 32 pipes 8 bpe @ SW_64K_D3_X @ Navi1x
    {   3,   38,  204,  178,    0, } , // 32 pipes 16 bpe @ SW_64K_D3_X @ Navi1x
    {   3,   34,  205,  179,    0, } , // 64 pipes 1 bpe @ SW_64K_D3_X @ Navi1x
    {   3,   35,  206,  180,    0, } , // 64 pipes 2 bpe @ SW_64K_D3_X @ Navi1x
    {   3,   36,  207,  181,    0, } , // 64 pipes 4 bpe @ SW_64K_D3_X @ Navi1x
    {   3,   37,  208,  182,    0, } , // 64 pipes 8 bpe @ SW_64K_D3_X @ Navi1x
    {   3,   38,  209,  182,    0, } , // 64 pipes 16 bpe @ SW_64K_D3_X @ Navi1x
};

const ADDR_SW_PATINFO GFX10_SW_256_S_RBPLUS_PATINFO[] =
{
    {   1,    0,    0,    0,    0, } , // 1 pipes (1 PKRs) 1 bpe @ SW_256_S @ RbPlus
    {   1,    1,    0,    0,    0, } , // 1 pipes (1 PKRs) 2 bpe @ SW_256_S @ RbPlus
    {   1,    2,    0,    0,    0, } , // 1 pipes (1 PKRs) 4 bpe @ SW_256_S @ RbPlus
    {   1,    3,    0,    0,    0, } , // 1 pipes (1 PKRs) 8 bpe @ SW_256_S @ RbPlus
    {   1,    4,    0,    0,    0, } , // 1 pipes (1 PKRs) 16 bpe @ SW_256_S @ RbPlus
    {   1,    0,    0,    0,    0, } , // 2 pipes (1-2 PKRs) 1 bpe @ SW_256_S @ RbPlus
    {   1,    1,    0,    0,    0, } , // 2 pipes (1-2 PKRs) 2 bpe @ SW_256_S @ RbPlus
    {   1,    2,    0,    0,    0, } , // 2 pipes (1-2 PKRs) 4 bpe @ SW_256_S @ RbPlus
    {   1,    3,    0,    0,    0, } , // 2 pipes (1-2 PKRs) 8 bpe @ SW_256_S @ RbPlus
    {   1,    4,    0,    0,    0, } , // 2 pipes (1-2 PKRs) 16 bpe @ SW_256_S @ RbPlus
    {   1,    0,    0,    0,    0, } , // 4 pipes (1-2 PKRs) 1 bpe @ SW_256_S @ RbPlus
    {   1,    1,    0,    0,    0, } , // 4 pipes (1-2 PKRs) 2 bpe @ SW_256_S @ RbPlus
    {   1,    2,    0,    0,    0, } , // 4 pipes (1-2 PKRs) 4 bpe @ SW_256_S @ RbPlus
    {   1,    3,    0,    0,    0, } , // 4 pipes (1-2 PKRs) 8 bpe @ SW_256_S @ RbPlus
    {   1,    4,    0,    0,    0, } , // 4 pipes (1-2 PKRs) 16 bpe @ SW_256_S @ RbPlus
    {   1,    0,    0,    0,    0, } , // 8 pipes (2 PKRs) 1 bpe @ SW_256_S @ RbPlus
    {   1,    1,    0,    0,    0, } , // 8 pipes (2 PKRs) 2 bpe @ SW_256_S @ RbPlus
    {   1,    2,    0,    0,    0, } , // 8 pipes (2 PKRs) 4 bpe @ SW_256_S @ RbPlus
    {   1,    3,    0,    0,    0, } , // 8 pipes (2 PKRs) 8 bpe @ SW_256_S @ RbPlus
    {   1,    4,    0,    0,    0, } , // 8 pipes (2 PKRs) 16 bpe @ SW_256_S @ RbPlus
    {   1,    0,    0,    0,    0, } , // 4 pipes (4 PKRs) 1 bpe @ SW_256_S @ RbPlus
    {   1,    1,    0,    0,    0, } , // 4 pipes (4 PKRs) 2 bpe @ SW_256_S @ RbPlus
    {   1,    2,    0,    0,    0, } , // 4 pipes (4 PKRs) 4 bpe @ SW_256_S @ RbPlus
    {   1,    3,    0,    0,    0, } , // 4 pipes (4 PKRs) 8 bpe @ SW_256_S @ RbPlus
    {   1,    4,    0,    0,    0, } , // 4 pipes (4 PKRs) 16 bpe @ SW_256_S @ RbPlus
    {   1,    0,    0,    0,    0, } , // 8 pipes (4 PKRs) 1 bpe @ SW_256_S @ RbPlus
    {   1,    1,    0,    0,    0, } , // 8 pipes (4 PKRs) 2 bpe @ SW_256_S @ RbPlus
    {   1,    2,    0,    0,    0, } , // 8 pipes (4 PKRs) 4 bpe @ SW_256_S @ RbPlus
    {   1,    3,    0,    0,    0, } , // 8 pipes (4 PKRs) 8 bpe @ SW_256_S @ RbPlus
    {   1,    4,    0,    0,    0, } , // 8 pipes (4 PKRs) 16 bpe @ SW_256_S @ RbPlus
    {   1,    0,    0,    0,    0, } , // 16 pipes (4 PKRs) 1 bpe @ SW_256_S @ RbPlus
    {   1,    1,    0,    0,    0, } , // 16 pipes (4 PKRs) 2 bpe @ SW_256_S @ RbPlus
    {   1,    2,    0,    0,    0, } , // 16 pipes (4 PKRs) 4 bpe @ SW_256_S @ RbPlus
    {   1,    3,    0,    0,    0, } , // 16 pipes (4 PKRs) 8 bpe @ SW_256_S @ RbPlus
    {   1,    4,    0,    0,    0, } , // 16 pipes (4 PKRs) 16 bpe @ SW_256_S @ RbPlus
    {   1,    0,    0,    0,    0, } , // 8 pipes (8 PKRs) 1 bpe @ SW_256_S @ RbPlus
    {   1,    1,    0,    0,    0, } , // 8 pipes (8 PKRs) 2 bpe @ SW_256_S @ RbPlus
    {   1,    2,    0,    0,    0, } , // 8 pipes (8 PKRs) 4 bpe @ SW_256_S @ RbPlus
    {   1,    3,    0,    0,    0, } , // 8 pipes (8 PKRs) 8 bpe @ SW_256_S @ RbPlus
    {   1,    4,    0,    0,    0, } , // 8 pipes (8 PKRs) 16 bpe @ SW_256_S @ RbPlus
    {   1,    0,    0,    0,    0, } , // 16 pipes (8 PKRs) 1 bpe @ SW_256_S @ RbPlus
    {   1,    1,    0,    0,    0, } , // 16 pipes (8 PKRs) 2 bpe @ SW_256_S @ RbPlus
    {   1,    2,    0,    0,    0, } , // 16 pipes (8 PKRs) 4 bpe @ SW_256_S @ RbPlus
    {   1,    3,    0,    0,    0, } , // 16 pipes (8 PKRs) 8 bpe @ SW_256_S @ RbPlus
    {   1,    4,    0,    0,    0, } , // 16 pipes (8 PKRs) 16 bpe @ SW_256_S @ RbPlus
    {   1,    0,    0,    0,    0, } , // 32 pipes (8 PKRs) 1 bpe @ SW_256_S @ RbPlus
    {   1,    1,    0,    0,    0, } , // 32 pipes (8 PKRs) 2 bpe @ SW_256_S @ RbPlus
    {   1,    2,    0,    0,    0, } , // 32 pipes (8 PKRs) 4 bpe @ SW_256_S @ RbPlus
    {   1,    3,    0,    0,    0, } , // 32 pipes (8 PKRs) 8 bpe @ SW_256_S @ RbPlus
    {   1,    4,    0,    0,    0, } , // 32 pipes (8 PKRs) 16 bpe @ SW_256_S @ RbPlus
    {   1,    0,    0,    0,    0, } , // 16 pipes (16 PKRs) 1 bpe @ SW_256_S @ RbPlus
    {   1,    1,    0,    0,    0, } , // 16 pipes (16 PKRs) 2 bpe @ SW_256_S @ RbPlus
    {   1,    2,    0,    0,    0, } , // 16 pipes (16 PKRs) 4 bpe @ SW_256_S @ RbPlus
    {   1,    3,    0,    0,    0, } , // 16 pipes (16 PKRs) 8 bpe @ SW_256_S @ RbPlus
    {   1,    4,    0,    0,    0, } , // 16 pipes (16 PKRs) 16 bpe @ SW_256_S @ RbPlus
    {   1,    0,    0,    0,    0, } , // 32 pipes (16 PKRs) 1 bpe @ SW_256_S @ RbPlus
    {   1,    1,    0,    0,    0, } , // 32 pipes (16 PKRs) 2 bpe @ SW_256_S @ RbPlus
    {   1,    2,    0,    0,    0, } , // 32 pipes (16 PKRs) 4 bpe @ SW_256_S @ RbPlus
    {   1,    3,    0,    0,    0, } , // 32 pipes (16 PKRs) 8 bpe @ SW_256_S @ RbPlus
    {   1,    4,    0,    0,    0, } , // 32 pipes (16 PKRs) 16 bpe @ SW_256_S @ RbPlus
    {   1,    0,    0,    0,    0, } , // 64 pipes (16 PKRs) 1 bpe @ SW_256_S @ RbPlus
    {   1,    1,    0,    0,    0, } , // 64 pipes (16 PKRs) 2 bpe @ SW_256_S @ RbPlus
    {   1,    2,    0,    0,    0, } , // 64 pipes (16 PKRs) 4 bpe @ SW_256_S @ RbPlus
    {   1,    3,    0,    0,    0, } , // 64 pipes (16 PKRs) 8 bpe @ SW_256_S @ RbPlus
    {   1,    4,    0,    0,    0, } , // 64 pipes (16 PKRs) 16 bpe @ SW_256_S @ RbPlus
    {   1,    0,    0,    0,    0, } , // 32 pipes (32 PKRs) 1 bpe @ SW_256_S @ RbPlus
    {   1,    1,    0,    0,    0, } , // 32 pipes (32 PKRs) 2 bpe @ SW_256_S @ RbPlus
    {   1,    2,    0,    0,    0, } , // 32 pipes (32 PKRs) 4 bpe @ SW_256_S @ RbPlus
    {   1,    3,    0,    0,    0, } , // 32 pipes (32 PKRs) 8 bpe @ SW_256_S @ RbPlus
    {   1,    4,    0,    0,    0, } , // 32 pipes (32 PKRs) 16 bpe @ SW_256_S @ RbPlus
    {   1,    0,    0,    0,    0, } , // 64 pipes (32 PKRs) 1 bpe @ SW_256_S @ RbPlus
    {   1,    1,    0,    0,    0, } , // 64 pipes (32 PKRs) 2 bpe @ SW_256_S @ RbPlus
    {   1,    2,    0,    0,    0, } , // 64 pipes (32 PKRs) 4 bpe @ SW_256_S @ RbPlus
    {   1,    3,    0,    0,    0, } , // 64 pipes (32 PKRs) 8 bpe @ SW_256_S @ RbPlus
    {   1,    4,    0,    0,    0, } , // 64 pipes (32 PKRs) 16 bpe @ SW_256_S @ RbPlus
};

const ADDR_SW_PATINFO GFX10_SW_256_D_RBPLUS_PATINFO[] =
{
    {   1,    5,    0,    0,    0, } , // 1 pipes (1 PKRs) 1 bpe @ SW_256_D @ RbPlus
    {   1,    1,    0,    0,    0, } , // 1 pipes (1 PKRs) 2 bpe @ SW_256_D @ RbPlus
    {   1,   39,    0,    0,    0, } , // 1 pipes (1 PKRs) 4 bpe @ SW_256_D @ RbPlus
    {   1,    6,    0,    0,    0, } , // 1 pipes (1 PKRs) 8 bpe @ SW_256_D @ RbPlus
    {   1,    7,    0,    0,    0, } , // 1 pipes (1 PKRs) 16 bpe @ SW_256_D @ RbPlus
    {   1,    5,    0,    0,    0, } , // 2 pipes (1-2 PKRs) 1 bpe @ SW_256_D @ RbPlus
    {   1,    1,    0,    0,    0, } , // 2 pipes (1-2 PKRs) 2 bpe @ SW_256_D @ RbPlus
    {   1,   39,    0,    0,    0, } , // 2 pipes (1-2 PKRs) 4 bpe @ SW_256_D @ RbPlus
    {   1,    6,    0,    0,    0, } , // 2 pipes (1-2 PKRs) 8 bpe @ SW_256_D @ RbPlus
    {   1,    7,    0,    0,    0, } , // 2 pipes (1-2 PKRs) 16 bpe @ SW_256_D @ RbPlus
    {   1,    5,    0,    0,    0, } , // 4 pipes (1-2 PKRs) 1 bpe @ SW_256_D @ RbPlus
    {   1,    1,    0,    0,    0, } , // 4 pipes (1-2 PKRs) 2 bpe @ SW_256_D @ RbPlus
    {   1,   39,    0,    0,    0, } , // 4 pipes (1-2 PKRs) 4 bpe @ SW_256_D @ RbPlus
    {   1,    6,    0,    0,    0, } , // 4 pipes (1-2 PKRs) 8 bpe @ SW_256_D @ RbPlus
    {   1,    7,    0,    0,    0, } , // 4 pipes (1-2 PKRs) 16 bpe @ SW_256_D @ RbPlus
    {   1,    5,    0,    0,    0, } , // 8 pipes (2 PKRs) 1 bpe @ SW_256_D @ RbPlus
    {   1,    1,    0,    0,    0, } , // 8 pipes (2 PKRs) 2 bpe @ SW_256_D @ RbPlus
    {   1,   39,    0,    0,    0, } , // 8 pipes (2 PKRs) 4 bpe @ SW_256_D @ RbPlus
    {   1,    6,    0,    0,    0, } , // 8 pipes (2 PKRs) 8 bpe @ SW_256_D @ RbPlus
    {   1,    7,    0,    0,    0, } , // 8 pipes (2 PKRs) 16 bpe @ SW_256_D @ RbPlus
    {   1,    5,    0,    0,    0, } , // 4 pipes (4 PKRs) 1 bpe @ SW_256_D @ RbPlus
    {   1,    1,    0,    0,    0, } , // 4 pipes (4 PKRs) 2 bpe @ SW_256_D @ RbPlus
    {   1,   39,    0,    0,    0, } , // 4 pipes (4 PKRs) 4 bpe @ SW_256_D @ RbPlus
    {   1,    6,    0,    0,    0, } , // 4 pipes (4 PKRs) 8 bpe @ SW_256_D @ RbPlus
    {   1,    7,    0,    0,    0, } , // 4 pipes (4 PKRs) 16 bpe @ SW_256_D @ RbPlus
    {   1,    5,    0,    0,    0, } , // 8 pipes (4 PKRs) 1 bpe @ SW_256_D @ RbPlus
    {   1,    1,    0,    0,    0, } , // 8 pipes (4 PKRs) 2 bpe @ SW_256_D @ RbPlus
    {   1,   39,    0,    0,    0, } , // 8 pipes (4 PKRs) 4 bpe @ SW_256_D @ RbPlus
    {   1,    6,    0,    0,    0, } , // 8 pipes (4 PKRs) 8 bpe @ SW_256_D @ RbPlus
    {   1,    7,    0,    0,    0, } , // 8 pipes (4 PKRs) 16 bpe @ SW_256_D @ RbPlus
    {   1,    5,    0,    0,    0, } , // 16 pipes (4 PKRs) 1 bpe @ SW_256_D @ RbPlus
    {   1,    1,    0,    0,    0, } , // 16 pipes (4 PKRs) 2 bpe @ SW_256_D @ RbPlus
    {   1,   39,    0,    0,    0, } , // 16 pipes (4 PKRs) 4 bpe @ SW_256_D @ RbPlus
    {   1,    6,    0,    0,    0, } , // 16 pipes (4 PKRs) 8 bpe @ SW_256_D @ RbPlus
    {   1,    7,    0,    0,    0, } , // 16 pipes (4 PKRs) 16 bpe @ SW_256_D @ RbPlus
    {   1,    5,    0,    0,    0, } , // 8 pipes (8 PKRs) 1 bpe @ SW_256_D @ RbPlus
    {   1,    1,    0,    0,    0, } , // 8 pipes (8 PKRs) 2 bpe @ SW_256_D @ RbPlus
    {   1,   39,    0,    0,    0, } , // 8 pipes (8 PKRs) 4 bpe @ SW_256_D @ RbPlus
    {   1,    6,    0,    0,    0, } , // 8 pipes (8 PKRs) 8 bpe @ SW_256_D @ RbPlus
    {   1,    7,    0,    0,    0, } , // 8 pipes (8 PKRs) 16 bpe @ SW_256_D @ RbPlus
    {   1,    5,    0,    0,    0, } , // 16 pipes (8 PKRs) 1 bpe @ SW_256_D @ RbPlus
    {   1,    1,    0,    0,    0, } , // 16 pipes (8 PKRs) 2 bpe @ SW_256_D @ RbPlus
    {   1,   39,    0,    0,    0, } , // 16 pipes (8 PKRs) 4 bpe @ SW_256_D @ RbPlus
    {   1,    6,    0,    0,    0, } , // 16 pipes (8 PKRs) 8 bpe @ SW_256_D @ RbPlus
    {   1,    7,    0,    0,    0, } , // 16 pipes (8 PKRs) 16 bpe @ SW_256_D @ RbPlus
    {   1,    5,    0,    0,    0, } , // 32 pipes (8 PKRs) 1 bpe @ SW_256_D @ RbPlus
    {   1,    1,    0,    0,    0, } , // 32 pipes (8 PKRs) 2 bpe @ SW_256_D @ RbPlus
    {   1,   39,    0,    0,    0, } , // 32 pipes (8 PKRs) 4 bpe @ SW_256_D @ RbPlus
    {   1,    6,    0,    0,    0, } , // 32 pipes (8 PKRs) 8 bpe @ SW_256_D @ RbPlus
    {   1,    7,    0,    0,    0, } , // 32 pipes (8 PKRs) 16 bpe @ SW_256_D @ RbPlus
    {   1,    5,    0,    0,    0, } , // 16 pipes (16 PKRs) 1 bpe @ SW_256_D @ RbPlus
    {   1,    1,    0,    0,    0, } , // 16 pipes (16 PKRs) 2 bpe @ SW_256_D @ RbPlus
    {   1,   39,    0,    0,    0, } , // 16 pipes (16 PKRs) 4 bpe @ SW_256_D @ RbPlus
    {   1,    6,    0,    0,    0, } , // 16 pipes (16 PKRs) 8 bpe @ SW_256_D @ RbPlus
    {   1,    7,    0,    0,    0, } , // 16 pipes (16 PKRs) 16 bpe @ SW_256_D @ RbPlus
    {   1,    5,    0,    0,    0, } , // 32 pipes (16 PKRs) 1 bpe @ SW_256_D @ RbPlus
    {   1,    1,    0,    0,    0, } , // 32 pipes (16 PKRs) 2 bpe @ SW_256_D @ RbPlus
    {   1,   39,    0,    0,    0, } , // 32 pipes (16 PKRs) 4 bpe @ SW_256_D @ RbPlus
    {   1,    6,    0,    0,    0, } , // 32 pipes (16 PKRs) 8 bpe @ SW_256_D @ RbPlus
    {   1,    7,    0,    0,    0, } , // 32 pipes (16 PKRs) 16 bpe @ SW_256_D @ RbPlus
    {   1,    5,    0,    0,    0, } , // 64 pipes (16 PKRs) 1 bpe @ SW_256_D @ RbPlus
    {   1,    1,    0,    0,    0, } , // 64 pipes (16 PKRs) 2 bpe @ SW_256_D @ RbPlus
    {   1,   39,    0,    0,    0, } , // 64 pipes (16 PKRs) 4 bpe @ SW_256_D @ RbPlus
    {   1,    6,    0,    0,    0, } , // 64 pipes (16 PKRs) 8 bpe @ SW_256_D @ RbPlus
    {   1,    7,    0,    0,    0, } , // 64 pipes (16 PKRs) 16 bpe @ SW_256_D @ RbPlus
    {   1,    5,    0,    0,    0, } , // 32 pipes (32 PKRs) 1 bpe @ SW_256_D @ RbPlus
    {   1,    1,    0,    0,    0, } , // 32 pipes (32 PKRs) 2 bpe @ SW_256_D @ RbPlus
    {   1,   39,    0,    0,    0, } , // 32 pipes (32 PKRs) 4 bpe @ SW_256_D @ RbPlus
    {   1,    6,    0,    0,    0, } , // 32 pipes (32 PKRs) 8 bpe @ SW_256_D @ RbPlus
    {   1,    7,    0,    0,    0, } , // 32 pipes (32 PKRs) 16 bpe @ SW_256_D @ RbPlus
    {   1,    5,    0,    0,    0, } , // 64 pipes (32 PKRs) 1 bpe @ SW_256_D @ RbPlus
    {   1,    1,    0,    0,    0, } , // 64 pipes (32 PKRs) 2 bpe @ SW_256_D @ RbPlus
    {   1,   39,    0,    0,    0, } , // 64 pipes (32 PKRs) 4 bpe @ SW_256_D @ RbPlus
    {   1,    6,    0,    0,    0, } , // 64 pipes (32 PKRs) 8 bpe @ SW_256_D @ RbPlus
    {   1,    7,    0,    0,    0, } , // 64 pipes (32 PKRs) 16 bpe @ SW_256_D @ RbPlus
};

const ADDR_SW_PATINFO GFX10_SW_4K_S_RBPLUS_PATINFO[] =
{
    {   1,    0,    1,    0,    0, } , // 1 pipes (1 PKRs) 1 bpe @ SW_4K_S @ RbPlus
    {   1,    1,    2,    0,    0, } , // 1 pipes (1 PKRs) 2 bpe @ SW_4K_S @ RbPlus
    {   1,    2,    3,    0,    0, } , // 1 pipes (1 PKRs) 4 bpe @ SW_4K_S @ RbPlus
    {   1,    3,    4,    0,    0, } , // 1 pipes (1 PKRs) 8 bpe @ SW_4K_S @ RbPlus
    {   1,    4,    5,    0,    0, } , // 1 pipes (1 PKRs) 16 bpe @ SW_4K_S @ RbPlus
    {   1,    0,    1,    0,    0, } , // 2 pipes (1-2 PKRs) 1 bpe @ SW_4K_S @ RbPlus
    {   1,    1,    2,    0,    0, } , // 2 pipes (1-2 PKRs) 2 bpe @ SW_4K_S @ RbPlus
    {   1,    2,    3,    0,    0, } , // 2 pipes (1-2 PKRs) 4 bpe @ SW_4K_S @ RbPlus
    {   1,    3,    4,    0,    0, } , // 2 pipes (1-2 PKRs) 8 bpe @ SW_4K_S @ RbPlus
    {   1,    4,    5,    0,    0, } , // 2 pipes (1-2 PKRs) 16 bpe @ SW_4K_S @ RbPlus
    {   1,    0,    1,    0,    0, } , // 4 pipes (1-2 PKRs) 1 bpe @ SW_4K_S @ RbPlus
    {   1,    1,    2,    0,    0, } , // 4 pipes (1-2 PKRs) 2 bpe @ SW_4K_S @ RbPlus
    {   1,    2,    3,    0,    0, } , // 4 pipes (1-2 PKRs) 4 bpe @ SW_4K_S @ RbPlus
    {   1,    3,    4,    0,    0, } , // 4 pipes (1-2 PKRs) 8 bpe @ SW_4K_S @ RbPlus
    {   1,    4,    5,    0,    0, } , // 4 pipes (1-2 PKRs) 16 bpe @ SW_4K_S @ RbPlus
    {   1,    0,    1,    0,    0, } , // 8 pipes (2 PKRs) 1 bpe @ SW_4K_S @ RbPlus
    {   1,    1,    2,    0,    0, } , // 8 pipes (2 PKRs) 2 bpe @ SW_4K_S @ RbPlus
    {   1,    2,    3,    0,    0, } , // 8 pipes (2 PKRs) 4 bpe @ SW_4K_S @ RbPlus
    {   1,    3,    4,    0,    0, } , // 8 pipes (2 PKRs) 8 bpe @ SW_4K_S @ RbPlus
    {   1,    4,    5,    0,    0, } , // 8 pipes (2 PKRs) 16 bpe @ SW_4K_S @ RbPlus
    {   1,    0,    1,    0,    0, } , // 4 pipes (4 PKRs) 1 bpe @ SW_4K_S @ RbPlus
    {   1,    1,    2,    0,    0, } , // 4 pipes (4 PKRs) 2 bpe @ SW_4K_S @ RbPlus
    {   1,    2,    3,    0,    0, } , // 4 pipes (4 PKRs) 4 bpe @ SW_4K_S @ RbPlus
    {   1,    3,    4,    0,    0, } , // 4 pipes (4 PKRs) 8 bpe @ SW_4K_S @ RbPlus
    {   1,    4,    5,    0,    0, } , // 4 pipes (4 PKRs) 16 bpe @ SW_4K_S @ RbPlus
    {   1,    0,    1,    0,    0, } , // 8 pipes (4 PKRs) 1 bpe @ SW_4K_S @ RbPlus
    {   1,    1,    2,    0,    0, } , // 8 pipes (4 PKRs) 2 bpe @ SW_4K_S @ RbPlus
    {   1,    2,    3,    0,    0, } , // 8 pipes (4 PKRs) 4 bpe @ SW_4K_S @ RbPlus
    {   1,    3,    4,    0,    0, } , // 8 pipes (4 PKRs) 8 bpe @ SW_4K_S @ RbPlus
    {   1,    4,    5,    0,    0, } , // 8 pipes (4 PKRs) 16 bpe @ SW_4K_S @ RbPlus
    {   1,    0,    1,    0,    0, } , // 16 pipes (4 PKRs) 1 bpe @ SW_4K_S @ RbPlus
    {   1,    1,    2,    0,    0, } , // 16 pipes (4 PKRs) 2 bpe @ SW_4K_S @ RbPlus
    {   1,    2,    3,    0,    0, } , // 16 pipes (4 PKRs) 4 bpe @ SW_4K_S @ RbPlus
    {   1,    3,    4,    0,    0, } , // 16 pipes (4 PKRs) 8 bpe @ SW_4K_S @ RbPlus
    {   1,    4,    5,    0,    0, } , // 16 pipes (4 PKRs) 16 bpe @ SW_4K_S @ RbPlus
    {   1,    0,    1,    0,    0, } , // 8 pipes (8 PKRs) 1 bpe @ SW_4K_S @ RbPlus
    {   1,    1,    2,    0,    0, } , // 8 pipes (8 PKRs) 2 bpe @ SW_4K_S @ RbPlus
    {   1,    2,    3,    0,    0, } , // 8 pipes (8 PKRs) 4 bpe @ SW_4K_S @ RbPlus
    {   1,    3,    4,    0,    0, } , // 8 pipes (8 PKRs) 8 bpe @ SW_4K_S @ RbPlus
    {   1,    4,    5,    0,    0, } , // 8 pipes (8 PKRs) 16 bpe @ SW_4K_S @ RbPlus
    {   1,    0,    1,    0,    0, } , // 16 pipes (8 PKRs) 1 bpe @ SW_4K_S @ RbPlus
    {   1,    1,    2,    0,    0, } , // 16 pipes (8 PKRs) 2 bpe @ SW_4K_S @ RbPlus
    {   1,    2,    3,    0,    0, } , // 16 pipes (8 PKRs) 4 bpe @ SW_4K_S @ RbPlus
    {   1,    3,    4,    0,    0, } , // 16 pipes (8 PKRs) 8 bpe @ SW_4K_S @ RbPlus
    {   1,    4,    5,    0,    0, } , // 16 pipes (8 PKRs) 16 bpe @ SW_4K_S @ RbPlus
    {   1,    0,    1,    0,    0, } , // 32 pipes (8 PKRs) 1 bpe @ SW_4K_S @ RbPlus
    {   1,    1,    2,    0,    0, } , // 32 pipes (8 PKRs) 2 bpe @ SW_4K_S @ RbPlus
    {   1,    2,    3,    0,    0, } , // 32 pipes (8 PKRs) 4 bpe @ SW_4K_S @ RbPlus
    {   1,    3,    4,    0,    0, } , // 32 pipes (8 PKRs) 8 bpe @ SW_4K_S @ RbPlus
    {   1,    4,    5,    0,    0, } , // 32 pipes (8 PKRs) 16 bpe @ SW_4K_S @ RbPlus
    {   1,    0,    1,    0,    0, } , // 16 pipes (16 PKRs) 1 bpe @ SW_4K_S @ RbPlus
    {   1,    1,    2,    0,    0, } , // 16 pipes (16 PKRs) 2 bpe @ SW_4K_S @ RbPlus
    {   1,    2,    3,    0,    0, } , // 16 pipes (16 PKRs) 4 bpe @ SW_4K_S @ RbPlus
    {   1,    3,    4,    0,    0, } , // 16 pipes (16 PKRs) 8 bpe @ SW_4K_S @ RbPlus
    {   1,    4,    5,    0,    0, } , // 16 pipes (16 PKRs) 16 bpe @ SW_4K_S @ RbPlus
    {   1,    0,    1,    0,    0, } , // 32 pipes (16 PKRs) 1 bpe @ SW_4K_S @ RbPlus
    {   1,    1,    2,    0,    0, } , // 32 pipes (16 PKRs) 2 bpe @ SW_4K_S @ RbPlus
    {   1,    2,    3,    0,    0, } , // 32 pipes (16 PKRs) 4 bpe @ SW_4K_S @ RbPlus
    {   1,    3,    4,    0,    0, } , // 32 pipes (16 PKRs) 8 bpe @ SW_4K_S @ RbPlus
    {   1,    4,    5,    0,    0, } , // 32 pipes (16 PKRs) 16 bpe @ SW_4K_S @ RbPlus
    {   1,    0,    1,    0,    0, } , // 64 pipes (16 PKRs) 1 bpe @ SW_4K_S @ RbPlus
    {   1,    1,    2,    0,    0, } , // 64 pipes (16 PKRs) 2 bpe @ SW_4K_S @ RbPlus
    {   1,    2,    3,    0,    0, } , // 64 pipes (16 PKRs) 4 bpe @ SW_4K_S @ RbPlus
    {   1,    3,    4,    0,    0, } , // 64 pipes (16 PKRs) 8 bpe @ SW_4K_S @ RbPlus
    {   1,    4,    5,    0,    0, } , // 64 pipes (16 PKRs) 16 bpe @ SW_4K_S @ RbPlus
    {   1,    0,    1,    0,    0, } , // 32 pipes (32 PKRs) 1 bpe @ SW_4K_S @ RbPlus
    {   1,    1,    2,    0,    0, } , // 32 pipes (32 PKRs) 2 bpe @ SW_4K_S @ RbPlus
    {   1,    2,    3,    0,    0, } , // 32 pipes (32 PKRs) 4 bpe @ SW_4K_S @ RbPlus
    {   1,    3,    4,    0,    0, } , // 32 pipes (32 PKRs) 8 bpe @ SW_4K_S @ RbPlus
    {   1,    4,    5,    0,    0, } , // 32 pipes (32 PKRs) 16 bpe @ SW_4K_S @ RbPlus
    {   1,    0,    1,    0,    0, } , // 64 pipes (32 PKRs) 1 bpe @ SW_4K_S @ RbPlus
    {   1,    1,    2,    0,    0, } , // 64 pipes (32 PKRs) 2 bpe @ SW_4K_S @ RbPlus
    {   1,    2,    3,    0,    0, } , // 64 pipes (32 PKRs) 4 bpe @ SW_4K_S @ RbPlus
    {   1,    3,    4,    0,    0, } , // 64 pipes (32 PKRs) 8 bpe @ SW_4K_S @ RbPlus
    {   1,    4,    5,    0,    0, } , // 64 pipes (32 PKRs) 16 bpe @ SW_4K_S @ RbPlus
};

const ADDR_SW_PATINFO GFX10_SW_4K_D_RBPLUS_PATINFO[] =
{
    {   1,    5,    1,    0,    0, } , // 1 pipes (1 PKRs) 1 bpe @ SW_4K_D @ RbPlus
    {   1,    1,    2,    0,    0, } , // 1 pipes (1 PKRs) 2 bpe @ SW_4K_D @ RbPlus
    {   1,   39,    3,    0,    0, } , // 1 pipes (1 PKRs) 4 bpe @ SW_4K_D @ RbPlus
    {   1,    6,    4,    0,    0, } , // 1 pipes (1 PKRs) 8 bpe @ SW_4K_D @ RbPlus
    {   1,    7,    5,    0,    0, } , // 1 pipes (1 PKRs) 16 bpe @ SW_4K_D @ RbPlus
    {   1,    5,    1,    0,    0, } , // 2 pipes (1-2 PKRs) 1 bpe @ SW_4K_D @ RbPlus
    {   1,    1,    2,    0,    0, } , // 2 pipes (1-2 PKRs) 2 bpe @ SW_4K_D @ RbPlus
    {   1,   39,    3,    0,    0, } , // 2 pipes (1-2 PKRs) 4 bpe @ SW_4K_D @ RbPlus
    {   1,    6,    4,    0,    0, } , // 2 pipes (1-2 PKRs) 8 bpe @ SW_4K_D @ RbPlus
    {   1,    7,    5,    0,    0, } , // 2 pipes (1-2 PKRs) 16 bpe @ SW_4K_D @ RbPlus
    {   1,    5,    1,    0,    0, } , // 4 pipes (1-2 PKRs) 1 bpe @ SW_4K_D @ RbPlus
    {   1,    1,    2,    0,    0, } , // 4 pipes (1-2 PKRs) 2 bpe @ SW_4K_D @ RbPlus
    {   1,   39,    3,    0,    0, } , // 4 pipes (1-2 PKRs) 4 bpe @ SW_4K_D @ RbPlus
    {   1,    6,    4,    0,    0, } , // 4 pipes (1-2 PKRs) 8 bpe @ SW_4K_D @ RbPlus
    {   1,    7,    5,    0,    0, } , // 4 pipes (1-2 PKRs) 16 bpe @ SW_4K_D @ RbPlus
    {   1,    5,    1,    0,    0, } , // 8 pipes (2 PKRs) 1 bpe @ SW_4K_D @ RbPlus
    {   1,    1,    2,    0,    0, } , // 8 pipes (2 PKRs) 2 bpe @ SW_4K_D @ RbPlus
    {   1,   39,    3,    0,    0, } , // 8 pipes (2 PKRs) 4 bpe @ SW_4K_D @ RbPlus
    {   1,    6,    4,    0,    0, } , // 8 pipes (2 PKRs) 8 bpe @ SW_4K_D @ RbPlus
    {   1,    7,    5,    0,    0, } , // 8 pipes (2 PKRs) 16 bpe @ SW_4K_D @ RbPlus
    {   1,    5,    1,    0,    0, } , // 4 pipes (4 PKRs) 1 bpe @ SW_4K_D @ RbPlus
    {   1,    1,    2,    0,    0, } , // 4 pipes (4 PKRs) 2 bpe @ SW_4K_D @ RbPlus
    {   1,   39,    3,    0,    0, } , // 4 pipes (4 PKRs) 4 bpe @ SW_4K_D @ RbPlus
    {   1,    6,    4,    0,    0, } , // 4 pipes (4 PKRs) 8 bpe @ SW_4K_D @ RbPlus
    {   1,    7,    5,    0,    0, } , // 4 pipes (4 PKRs) 16 bpe @ SW_4K_D @ RbPlus
    {   1,    5,    1,    0,    0, } , // 8 pipes (4 PKRs) 1 bpe @ SW_4K_D @ RbPlus
    {   1,    1,    2,    0,    0, } , // 8 pipes (4 PKRs) 2 bpe @ SW_4K_D @ RbPlus
    {   1,   39,    3,    0,    0, } , // 8 pipes (4 PKRs) 4 bpe @ SW_4K_D @ RbPlus
    {   1,    6,    4,    0,    0, } , // 8 pipes (4 PKRs) 8 bpe @ SW_4K_D @ RbPlus
    {   1,    7,    5,    0,    0, } , // 8 pipes (4 PKRs) 16 bpe @ SW_4K_D @ RbPlus
    {   1,    5,    1,    0,    0, } , // 16 pipes (4 PKRs) 1 bpe @ SW_4K_D @ RbPlus
    {   1,    1,    2,    0,    0, } , // 16 pipes (4 PKRs) 2 bpe @ SW_4K_D @ RbPlus
    {   1,   39,    3,    0,    0, } , // 16 pipes (4 PKRs) 4 bpe @ SW_4K_D @ RbPlus
    {   1,    6,    4,    0,    0, } , // 16 pipes (4 PKRs) 8 bpe @ SW_4K_D @ RbPlus
    {   1,    7,    5,    0,    0, } , // 16 pipes (4 PKRs) 16 bpe @ SW_4K_D @ RbPlus
    {   1,    5,    1,    0,    0, } , // 8 pipes (8 PKRs) 1 bpe @ SW_4K_D @ RbPlus
    {   1,    1,    2,    0,    0, } , // 8 pipes (8 PKRs) 2 bpe @ SW_4K_D @ RbPlus
    {   1,   39,    3,    0,    0, } , // 8 pipes (8 PKRs) 4 bpe @ SW_4K_D @ RbPlus
    {   1,    6,    4,    0,    0, } , // 8 pipes (8 PKRs) 8 bpe @ SW_4K_D @ RbPlus
    {   1,    7,    5,    0,    0, } , // 8 pipes (8 PKRs) 16 bpe @ SW_4K_D @ RbPlus
    {   1,    5,    1,    0,    0, } , // 16 pipes (8 PKRs) 1 bpe @ SW_4K_D @ RbPlus
    {   1,    1,    2,    0,    0, } , // 16 pipes (8 PKRs) 2 bpe @ SW_4K_D @ RbPlus
    {   1,   39,    3,    0,    0, } , // 16 pipes (8 PKRs) 4 bpe @ SW_4K_D @ RbPlus
    {   1,    6,    4,    0,    0, } , // 16 pipes (8 PKRs) 8 bpe @ SW_4K_D @ RbPlus
    {   1,    7,    5,    0,    0, } , // 16 pipes (8 PKRs) 16 bpe @ SW_4K_D @ RbPlus
    {   1,    5,    1,    0,    0, } , // 32 pipes (8 PKRs) 1 bpe @ SW_4K_D @ RbPlus
    {   1,    1,    2,    0,    0, } , // 32 pipes (8 PKRs) 2 bpe @ SW_4K_D @ RbPlus
    {   1,   39,    3,    0,    0, } , // 32 pipes (8 PKRs) 4 bpe @ SW_4K_D @ RbPlus
    {   1,    6,    4,    0,    0, } , // 32 pipes (8 PKRs) 8 bpe @ SW_4K_D @ RbPlus
    {   1,    7,    5,    0,    0, } , // 32 pipes (8 PKRs) 16 bpe @ SW_4K_D @ RbPlus
    {   1,    5,    1,    0,    0, } , // 16 pipes (16 PKRs) 1 bpe @ SW_4K_D @ RbPlus
    {   1,    1,    2,    0,    0, } , // 16 pipes (16 PKRs) 2 bpe @ SW_4K_D @ RbPlus
    {   1,   39,    3,    0,    0, } , // 16 pipes (16 PKRs) 4 bpe @ SW_4K_D @ RbPlus
    {   1,    6,    4,    0,    0, } , // 16 pipes (16 PKRs) 8 bpe @ SW_4K_D @ RbPlus
    {   1,    7,    5,    0,    0, } , // 16 pipes (16 PKRs) 16 bpe @ SW_4K_D @ RbPlus
    {   1,    5,    1,    0,    0, } , // 32 pipes (16 PKRs) 1 bpe @ SW_4K_D @ RbPlus
    {   1,    1,    2,    0,    0, } , // 32 pipes (16 PKRs) 2 bpe @ SW_4K_D @ RbPlus
    {   1,   39,    3,    0,    0, } , // 32 pipes (16 PKRs) 4 bpe @ SW_4K_D @ RbPlus
    {   1,    6,    4,    0,    0, } , // 32 pipes (16 PKRs) 8 bpe @ SW_4K_D @ RbPlus
    {   1,    7,    5,    0,    0, } , // 32 pipes (16 PKRs) 16 bpe @ SW_4K_D @ RbPlus
    {   1,    5,    1,    0,    0, } , // 64 pipes (16 PKRs) 1 bpe @ SW_4K_D @ RbPlus
    {   1,    1,    2,    0,    0, } , // 64 pipes (16 PKRs) 2 bpe @ SW_4K_D @ RbPlus
    {   1,   39,    3,    0,    0, } , // 64 pipes (16 PKRs) 4 bpe @ SW_4K_D @ RbPlus
    {   1,    6,    4,    0,    0, } , // 64 pipes (16 PKRs) 8 bpe @ SW_4K_D @ RbPlus
    {   1,    7,    5,    0,    0, } , // 64 pipes (16 PKRs) 16 bpe @ SW_4K_D @ RbPlus
    {   1,    5,    1,    0,    0, } , // 32 pipes (32 PKRs) 1 bpe @ SW_4K_D @ RbPlus
    {   1,    1,    2,    0,    0, } , // 32 pipes (32 PKRs) 2 bpe @ SW_4K_D @ RbPlus
    {   1,   39,    3,    0,    0, } , // 32 pipes (32 PKRs) 4 bpe @ SW_4K_D @ RbPlus
    {   1,    6,    4,    0,    0, } , // 32 pipes (32 PKRs) 8 bpe @ SW_4K_D @ RbPlus
    {   1,    7,    5,    0,    0, } , // 32 pipes (32 PKRs) 16 bpe @ SW_4K_D @ RbPlus
    {   1,    5,    1,    0,    0, } , // 64 pipes (32 PKRs) 1 bpe @ SW_4K_D @ RbPlus
    {   1,    1,    2,    0,    0, } , // 64 pipes (32 PKRs) 2 bpe @ SW_4K_D @ RbPlus
    {   1,   39,    3,    0,    0, } , // 64 pipes (32 PKRs) 4 bpe @ SW_4K_D @ RbPlus
    {   1,    6,    4,    0,    0, } , // 64 pipes (32 PKRs) 8 bpe @ SW_4K_D @ RbPlus
    {   1,    7,    5,    0,    0, } , // 64 pipes (32 PKRs) 16 bpe @ SW_4K_D @ RbPlus
};

const ADDR_SW_PATINFO GFX10_SW_4K_S_X_RBPLUS_PATINFO[] =
{
    {   1,    0,    1,    0,    0, } , // 1 pipes (1 PKRs) 1 bpe @ SW_4K_S_X @ RbPlus
    {   1,    1,    2,    0,    0, } , // 1 pipes (1 PKRs) 2 bpe @ SW_4K_S_X @ RbPlus
    {   1,    2,    3,    0,    0, } , // 1 pipes (1 PKRs) 4 bpe @ SW_4K_S_X @ RbPlus
    {   1,    3,    4,    0,    0, } , // 1 pipes (1 PKRs) 8 bpe @ SW_4K_S_X @ RbPlus
    {   1,    4,    5,    0,    0, } , // 1 pipes (1 PKRs) 16 bpe @ SW_4K_S_X @ RbPlus
    {   3,    0,    6,    0,    0, } , // 2 pipes (1-2 PKRs) 1 bpe @ SW_4K_S_X @ RbPlus
    {   3,    1,    7,    0,    0, } , // 2 pipes (1-2 PKRs) 2 bpe @ SW_4K_S_X @ RbPlus
    {   3,    2,    8,    0,    0, } , // 2 pipes (1-2 PKRs) 4 bpe @ SW_4K_S_X @ RbPlus
    {   3,    3,    9,    0,    0, } , // 2 pipes (1-2 PKRs) 8 bpe @ SW_4K_S_X @ RbPlus
    {   3,    4,   10,    0,    0, } , // 2 pipes (1-2 PKRs) 16 bpe @ SW_4K_S_X @ RbPlus
    {   3,    0,  210,    0,    0, } , // 4 pipes (1-2 PKRs) 1 bpe @ SW_4K_S_X @ RbPlus
    {   3,    1,  211,    0,    0, } , // 4 pipes (1-2 PKRs) 2 bpe @ SW_4K_S_X @ RbPlus
    {   3,    2,  212,    0,    0, } , // 4 pipes (1-2 PKRs) 4 bpe @ SW_4K_S_X @ RbPlus
    {   3,    3,  213,    0,    0, } , // 4 pipes (1-2 PKRs) 8 bpe @ SW_4K_S_X @ RbPlus
    {   3,    4,  214,    0,    0, } , // 4 pipes (1-2 PKRs) 16 bpe @ SW_4K_S_X @ RbPlus
    {   3,    0,  215,    0,    0, } , // 8 pipes (2 PKRs) 1 bpe @ SW_4K_S_X @ RbPlus
    {   3,    1,  216,    0,    0, } , // 8 pipes (2 PKRs) 2 bpe @ SW_4K_S_X @ RbPlus
    {   3,    2,  217,    0,    0, } , // 8 pipes (2 PKRs) 4 bpe @ SW_4K_S_X @ RbPlus
    {   3,    3,  218,    0,    0, } , // 8 pipes (2 PKRs) 8 bpe @ SW_4K_S_X @ RbPlus
    {   3,    4,  219,    0,    0, } , // 8 pipes (2 PKRs) 16 bpe @ SW_4K_S_X @ RbPlus
    {   3,    0,   11,    0,    0, } , // 4 pipes (4 PKRs) 1 bpe @ SW_4K_S_X @ RbPlus
    {   3,    1,   12,    0,    0, } , // 4 pipes (4 PKRs) 2 bpe @ SW_4K_S_X @ RbPlus
    {   3,    2,   13,    0,    0, } , // 4 pipes (4 PKRs) 4 bpe @ SW_4K_S_X @ RbPlus
    {   3,    3,   14,    0,    0, } , // 4 pipes (4 PKRs) 8 bpe @ SW_4K_S_X @ RbPlus
    {   3,    4,   15,    0,    0, } , // 4 pipes (4 PKRs) 16 bpe @ SW_4K_S_X @ RbPlus
    {   3,    0,  220,    0,    0, } , // 8 pipes (4 PKRs) 1 bpe @ SW_4K_S_X @ RbPlus
    {   3,    1,  221,    0,    0, } , // 8 pipes (4 PKRs) 2 bpe @ SW_4K_S_X @ RbPlus
    {   3,    2,  222,    0,    0, } , // 8 pipes (4 PKRs) 4 bpe @ SW_4K_S_X @ RbPlus
    {   3,    3,  223,    0,    0, } , // 8 pipes (4 PKRs) 8 bpe @ SW_4K_S_X @ RbPlus
    {   3,    4,  224,    0,    0, } , // 8 pipes (4 PKRs) 16 bpe @ SW_4K_S_X @ RbPlus
    {   3,    0,  225,    0,    0, } , // 16 pipes (4 PKRs) 1 bpe @ SW_4K_S_X @ RbPlus
    {   3,    1,  226,    0,    0, } , // 16 pipes (4 PKRs) 2 bpe @ SW_4K_S_X @ RbPlus
    {   3,    2,  227,    0,    0, } , // 16 pipes (4 PKRs) 4 bpe @ SW_4K_S_X @ RbPlus
    {   3,    3,  228,    0,    0, } , // 16 pipes (4 PKRs) 8 bpe @ SW_4K_S_X @ RbPlus
    {   3,    4,  229,    0,    0, } , // 16 pipes (4 PKRs) 16 bpe @ SW_4K_S_X @ RbPlus
    {   3,    0,   16,    0,    0, } , // 8 pipes (8 PKRs) 1 bpe @ SW_4K_S_X @ RbPlus
    {   3,    1,   17,    0,    0, } , // 8 pipes (8 PKRs) 2 bpe @ SW_4K_S_X @ RbPlus
    {   3,    2,   18,    0,    0, } , // 8 pipes (8 PKRs) 4 bpe @ SW_4K_S_X @ RbPlus
    {   3,    3,   19,    0,    0, } , // 8 pipes (8 PKRs) 8 bpe @ SW_4K_S_X @ RbPlus
    {   3,    4,   20,    0,    0, } , // 8 pipes (8 PKRs) 16 bpe @ SW_4K_S_X @ RbPlus
    {   3,    0,  230,    0,    0, } , // 16 pipes (8 PKRs) 1 bpe @ SW_4K_S_X @ RbPlus
    {   3,    1,  231,    0,    0, } , // 16 pipes (8 PKRs) 2 bpe @ SW_4K_S_X @ RbPlus
    {   3,    2,  232,    0,    0, } , // 16 pipes (8 PKRs) 4 bpe @ SW_4K_S_X @ RbPlus
    {   3,    3,  233,    0,    0, } , // 16 pipes (8 PKRs) 8 bpe @ SW_4K_S_X @ RbPlus
    {   3,    4,  234,    0,    0, } , // 16 pipes (8 PKRs) 16 bpe @ SW_4K_S_X @ RbPlus
    {   3,    0,  235,    0,    0, } , // 32 pipes (8 PKRs) 1 bpe @ SW_4K_S_X @ RbPlus
    {   3,    1,  236,    0,    0, } , // 32 pipes (8 PKRs) 2 bpe @ SW_4K_S_X @ RbPlus
    {   3,    2,  237,    0,    0, } , // 32 pipes (8 PKRs) 4 bpe @ SW_4K_S_X @ RbPlus
    {   3,    3,  238,    0,    0, } , // 32 pipes (8 PKRs) 8 bpe @ SW_4K_S_X @ RbPlus
    {   3,    4,  239,    0,    0, } , // 32 pipes (8 PKRs) 16 bpe @ SW_4K_S_X @ RbPlus
    {   3,    0,   21,    0,    0, } , // 16 pipes (16 PKRs) 1 bpe @ SW_4K_S_X @ RbPlus
    {   3,    1,   22,    0,    0, } , // 16 pipes (16 PKRs) 2 bpe @ SW_4K_S_X @ RbPlus
    {   3,    2,   23,    0,    0, } , // 16 pipes (16 PKRs) 4 bpe @ SW_4K_S_X @ RbPlus
    {   3,    3,   24,    0,    0, } , // 16 pipes (16 PKRs) 8 bpe @ SW_4K_S_X @ RbPlus
    {   3,    4,   25,    0,    0, } , // 16 pipes (16 PKRs) 16 bpe @ SW_4K_S_X @ RbPlus
    {   3,    0,  240,    0,    0, } , // 32 pipes (16 PKRs) 1 bpe @ SW_4K_S_X @ RbPlus
    {   3,    1,  241,    0,    0, } , // 32 pipes (16 PKRs) 2 bpe @ SW_4K_S_X @ RbPlus
    {   3,    2,  242,    0,    0, } , // 32 pipes (16 PKRs) 4 bpe @ SW_4K_S_X @ RbPlus
    {   3,    3,  243,    0,    0, } , // 32 pipes (16 PKRs) 8 bpe @ SW_4K_S_X @ RbPlus
    {   3,    4,  244,    0,    0, } , // 32 pipes (16 PKRs) 16 bpe @ SW_4K_S_X @ RbPlus
    {   3,    0,  245,    0,    0, } , // 64 pipes (16 PKRs) 1 bpe @ SW_4K_S_X @ RbPlus
    {   3,    1,  246,    0,    0, } , // 64 pipes (16 PKRs) 2 bpe @ SW_4K_S_X @ RbPlus
    {   3,    2,  247,    0,    0, } , // 64 pipes (16 PKRs) 4 bpe @ SW_4K_S_X @ RbPlus
    {   3,    3,  248,    0,    0, } , // 64 pipes (16 PKRs) 8 bpe @ SW_4K_S_X @ RbPlus
    {   3,    4,  249,    0,    0, } , // 64 pipes (16 PKRs) 16 bpe @ SW_4K_S_X @ RbPlus
    {   3,    0,   21,    0,    0, } , // 32 pipes (32 PKRs) 1 bpe @ SW_4K_S_X @ RbPlus
    {   3,    1,   22,    0,    0, } , // 32 pipes (32 PKRs) 2 bpe @ SW_4K_S_X @ RbPlus
    {   3,    2,   23,    0,    0, } , // 32 pipes (32 PKRs) 4 bpe @ SW_4K_S_X @ RbPlus
    {   3,    3,   24,    0,    0, } , // 32 pipes (32 PKRs) 8 bpe @ SW_4K_S_X @ RbPlus
    {   3,    4,   25,    0,    0, } , // 32 pipes (32 PKRs) 16 bpe @ SW_4K_S_X @ RbPlus
    {   3,    0,  240,    0,    0, } , // 64 pipes (32 PKRs) 1 bpe @ SW_4K_S_X @ RbPlus
    {   3,    1,  241,    0,    0, } , // 64 pipes (32 PKRs) 2 bpe @ SW_4K_S_X @ RbPlus
    {   3,    2,  242,    0,    0, } , // 64 pipes (32 PKRs) 4 bpe @ SW_4K_S_X @ RbPlus
    {   3,    3,  243,    0,    0, } , // 64 pipes (32 PKRs) 8 bpe @ SW_4K_S_X @ RbPlus
    {   3,    4,  244,    0,    0, } , // 64 pipes (32 PKRs) 16 bpe @ SW_4K_S_X @ RbPlus
};

const ADDR_SW_PATINFO GFX10_SW_4K_D_X_RBPLUS_PATINFO[] =
{
    {   1,    5,    1,    0,    0, } , // 1 pipes (1 PKRs) 1 bpe @ SW_4K_D_X @ RbPlus
    {   1,    1,    2,    0,    0, } , // 1 pipes (1 PKRs) 2 bpe @ SW_4K_D_X @ RbPlus
    {   1,   39,    3,    0,    0, } , // 1 pipes (1 PKRs) 4 bpe @ SW_4K_D_X @ RbPlus
    {   1,    6,    4,    0,    0, } , // 1 pipes (1 PKRs) 8 bpe @ SW_4K_D_X @ RbPlus
    {   1,    7,    5,    0,    0, } , // 1 pipes (1 PKRs) 16 bpe @ SW_4K_D_X @ RbPlus
    {   3,    5,    6,    0,    0, } , // 2 pipes (1-2 PKRs) 1 bpe @ SW_4K_D_X @ RbPlus
    {   3,    1,    7,    0,    0, } , // 2 pipes (1-2 PKRs) 2 bpe @ SW_4K_D_X @ RbPlus
    {   3,   39,    8,    0,    0, } , // 2 pipes (1-2 PKRs) 4 bpe @ SW_4K_D_X @ RbPlus
    {   3,    6,    9,    0,    0, } , // 2 pipes (1-2 PKRs) 8 bpe @ SW_4K_D_X @ RbPlus
    {   3,    7,   10,    0,    0, } , // 2 pipes (1-2 PKRs) 16 bpe @ SW_4K_D_X @ RbPlus
    {   3,    5,  210,    0,    0, } , // 4 pipes (1-2 PKRs) 1 bpe @ SW_4K_D_X @ RbPlus
    {   3,    1,  211,    0,    0, } , // 4 pipes (1-2 PKRs) 2 bpe @ SW_4K_D_X @ RbPlus
    {   3,   39,  212,    0,    0, } , // 4 pipes (1-2 PKRs) 4 bpe @ SW_4K_D_X @ RbPlus
    {   3,    6,  213,    0,    0, } , // 4 pipes (1-2 PKRs) 8 bpe @ SW_4K_D_X @ RbPlus
    {   3,    7,  214,    0,    0, } , // 4 pipes (1-2 PKRs) 16 bpe @ SW_4K_D_X @ RbPlus
    {   3,    5,  215,    0,    0, } , // 8 pipes (2 PKRs) 1 bpe @ SW_4K_D_X @ RbPlus
    {   3,    1,  216,    0,    0, } , // 8 pipes (2 PKRs) 2 bpe @ SW_4K_D_X @ RbPlus
    {   3,   39,  217,    0,    0, } , // 8 pipes (2 PKRs) 4 bpe @ SW_4K_D_X @ RbPlus
    {   3,    6,  218,    0,    0, } , // 8 pipes (2 PKRs) 8 bpe @ SW_4K_D_X @ RbPlus
    {   3,    7,  219,    0,    0, } , // 8 pipes (2 PKRs) 16 bpe @ SW_4K_D_X @ RbPlus
    {   3,    5,   11,    0,    0, } , // 4 pipes (4 PKRs) 1 bpe @ SW_4K_D_X @ RbPlus
    {   3,    1,   12,    0,    0, } , // 4 pipes (4 PKRs) 2 bpe @ SW_4K_D_X @ RbPlus
    {   3,   39,   13,    0,    0, } , // 4 pipes (4 PKRs) 4 bpe @ SW_4K_D_X @ RbPlus
    {   3,    6,   14,    0,    0, } , // 4 pipes (4 PKRs) 8 bpe @ SW_4K_D_X @ RbPlus
    {   3,    7,   15,    0,    0, } , // 4 pipes (4 PKRs) 16 bpe @ SW_4K_D_X @ RbPlus
    {   3,    5,  220,    0,    0, } , // 8 pipes (4 PKRs) 1 bpe @ SW_4K_D_X @ RbPlus
    {   3,    1,  221,    0,    0, } , // 8 pipes (4 PKRs) 2 bpe @ SW_4K_D_X @ RbPlus
    {   3,   39,  222,    0,    0, } , // 8 pipes (4 PKRs) 4 bpe @ SW_4K_D_X @ RbPlus
    {   3,    6,  223,    0,    0, } , // 8 pipes (4 PKRs) 8 bpe @ SW_4K_D_X @ RbPlus
    {   3,    7,  224,    0,    0, } , // 8 pipes (4 PKRs) 16 bpe @ SW_4K_D_X @ RbPlus
    {   3,    5,  225,    0,    0, } , // 16 pipes (4 PKRs) 1 bpe @ SW_4K_D_X @ RbPlus
    {   3,    1,  226,    0,    0, } , // 16 pipes (4 PKRs) 2 bpe @ SW_4K_D_X @ RbPlus
    {   3,   39,  227,    0,    0, } , // 16 pipes (4 PKRs) 4 bpe @ SW_4K_D_X @ RbPlus
    {   3,    6,  228,    0,    0, } , // 16 pipes (4 PKRs) 8 bpe @ SW_4K_D_X @ RbPlus
    {   3,    7,  229,    0,    0, } , // 16 pipes (4 PKRs) 16 bpe @ SW_4K_D_X @ RbPlus
    {   3,    5,   16,    0,    0, } , // 8 pipes (8 PKRs) 1 bpe @ SW_4K_D_X @ RbPlus
    {   3,    1,   17,    0,    0, } , // 8 pipes (8 PKRs) 2 bpe @ SW_4K_D_X @ RbPlus
    {   3,   39,   18,    0,    0, } , // 8 pipes (8 PKRs) 4 bpe @ SW_4K_D_X @ RbPlus
    {   3,    6,   19,    0,    0, } , // 8 pipes (8 PKRs) 8 bpe @ SW_4K_D_X @ RbPlus
    {   3,    7,   20,    0,    0, } , // 8 pipes (8 PKRs) 16 bpe @ SW_4K_D_X @ RbPlus
    {   3,    5,  230,    0,    0, } , // 16 pipes (8 PKRs) 1 bpe @ SW_4K_D_X @ RbPlus
    {   3,    1,  231,    0,    0, } , // 16 pipes (8 PKRs) 2 bpe @ SW_4K_D_X @ RbPlus
    {   3,   39,  232,    0,    0, } , // 16 pipes (8 PKRs) 4 bpe @ SW_4K_D_X @ RbPlus
    {   3,    6,  233,    0,    0, } , // 16 pipes (8 PKRs) 8 bpe @ SW_4K_D_X @ RbPlus
    {   3,    7,  234,    0,    0, } , // 16 pipes (8 PKRs) 16 bpe @ SW_4K_D_X @ RbPlus
    {   3,    5,  235,    0,    0, } , // 32 pipes (8 PKRs) 1 bpe @ SW_4K_D_X @ RbPlus
    {   3,    1,  236,    0,    0, } , // 32 pipes (8 PKRs) 2 bpe @ SW_4K_D_X @ RbPlus
    {   3,   39,  237,    0,    0, } , // 32 pipes (8 PKRs) 4 bpe @ SW_4K_D_X @ RbPlus
    {   3,    6,  238,    0,    0, } , // 32 pipes (8 PKRs) 8 bpe @ SW_4K_D_X @ RbPlus
    {   3,    7,  239,    0,    0, } , // 32 pipes (8 PKRs) 16 bpe @ SW_4K_D_X @ RbPlus
    {   3,    5,   21,    0,    0, } , // 16 pipes (16 PKRs) 1 bpe @ SW_4K_D_X @ RbPlus
    {   3,    1,   22,    0,    0, } , // 16 pipes (16 PKRs) 2 bpe @ SW_4K_D_X @ RbPlus
    {   3,   39,   23,    0,    0, } , // 16 pipes (16 PKRs) 4 bpe @ SW_4K_D_X @ RbPlus
    {   3,    6,   24,    0,    0, } , // 16 pipes (16 PKRs) 8 bpe @ SW_4K_D_X @ RbPlus
    {   3,    7,   25,    0,    0, } , // 16 pipes (16 PKRs) 16 bpe @ SW_4K_D_X @ RbPlus
    {   3,    5,  240,    0,    0, } , // 32 pipes (16 PKRs) 1 bpe @ SW_4K_D_X @ RbPlus
    {   3,    1,  241,    0,    0, } , // 32 pipes (16 PKRs) 2 bpe @ SW_4K_D_X @ RbPlus
    {   3,   39,  242,    0,    0, } , // 32 pipes (16 PKRs) 4 bpe @ SW_4K_D_X @ RbPlus
    {   3,    6,  243,    0,    0, } , // 32 pipes (16 PKRs) 8 bpe @ SW_4K_D_X @ RbPlus
    {   3,    7,  244,    0,    0, } , // 32 pipes (16 PKRs) 16 bpe @ SW_4K_D_X @ RbPlus
    {   3,    5,  245,    0,    0, } , // 64 pipes (16 PKRs) 1 bpe @ SW_4K_D_X @ RbPlus
    {   3,    1,  246,    0,    0, } , // 64 pipes (16 PKRs) 2 bpe @ SW_4K_D_X @ RbPlus
    {   3,   39,  247,    0,    0, } , // 64 pipes (16 PKRs) 4 bpe @ SW_4K_D_X @ RbPlus
    {   3,    6,  248,    0,    0, } , // 64 pipes (16 PKRs) 8 bpe @ SW_4K_D_X @ RbPlus
    {   3,    7,  249,    0,    0, } , // 64 pipes (16 PKRs) 16 bpe @ SW_4K_D_X @ RbPlus
    {   3,    5,   21,    0,    0, } , // 32 pipes (32 PKRs) 1 bpe @ SW_4K_D_X @ RbPlus
    {   3,    1,   22,    0,    0, } , // 32 pipes (32 PKRs) 2 bpe @ SW_4K_D_X @ RbPlus
    {   3,   39,   23,    0,    0, } , // 32 pipes (32 PKRs) 4 bpe @ SW_4K_D_X @ RbPlus
    {   3,    6,   24,    0,    0, } , // 32 pipes (32 PKRs) 8 bpe @ SW_4K_D_X @ RbPlus
    {   3,    7,   25,    0,    0, } , // 32 pipes (32 PKRs) 16 bpe @ SW_4K_D_X @ RbPlus
    {   3,    5,  240,    0,    0, } , // 64 pipes (32 PKRs) 1 bpe @ SW_4K_D_X @ RbPlus
    {   3,    1,  241,    0,    0, } , // 64 pipes (32 PKRs) 2 bpe @ SW_4K_D_X @ RbPlus
    {   3,   39,  242,    0,    0, } , // 64 pipes (32 PKRs) 4 bpe @ SW_4K_D_X @ RbPlus
    {   3,    6,  243,    0,    0, } , // 64 pipes (32 PKRs) 8 bpe @ SW_4K_D_X @ RbPlus
    {   3,    7,  244,    0,    0, } , // 64 pipes (32 PKRs) 16 bpe @ SW_4K_D_X @ RbPlus
};

const ADDR_SW_PATINFO GFX10_SW_4K_S3_RBPLUS_PATINFO[] =
{
    {   1,   29,  131,    0,    0, } , // 1 pipes (1 PKRs) 1 bpe @ SW_4K_S3 @ RbPlus
    {   1,   30,  132,    0,    0, } , // 1 pipes (1 PKRs) 2 bpe @ SW_4K_S3 @ RbPlus
    {   1,   31,  133,    0,    0, } , // 1 pipes (1 PKRs) 4 bpe @ SW_4K_S3 @ RbPlus
    {   1,   32,  134,    0,    0, } , // 1 pipes (1 PKRs) 8 bpe @ SW_4K_S3 @ RbPlus
    {   1,   33,  135,    0,    0, } , // 1 pipes (1 PKRs) 16 bpe @ SW_4K_S3 @ RbPlus
    {   1,   29,  131,    0,    0, } , // 2 pipes (1-2 PKRs) 1 bpe @ SW_4K_S3 @ RbPlus
    {   1,   30,  132,    0,    0, } , // 2 pipes (1-2 PKRs) 2 bpe @ SW_4K_S3 @ RbPlus
    {   1,   31,  133,    0,    0, } , // 2 pipes (1-2 PKRs) 4 bpe @ SW_4K_S3 @ RbPlus
    {   1,   32,  134,    0,    0, } , // 2 pipes (1-2 PKRs) 8 bpe @ SW_4K_S3 @ RbPlus
    {   1,   33,  135,    0,    0, } , // 2 pipes (1-2 PKRs) 16 bpe @ SW_4K_S3 @ RbPlus
    {   1,   29,  131,    0,    0, } , // 4 pipes (1-2 PKRs) 1 bpe @ SW_4K_S3 @ RbPlus
    {   1,   30,  132,    0,    0, } , // 4 pipes (1-2 PKRs) 2 bpe @ SW_4K_S3 @ RbPlus
    {   1,   31,  133,    0,    0, } , // 4 pipes (1-2 PKRs) 4 bpe @ SW_4K_S3 @ RbPlus
    {   1,   32,  134,    0,    0, } , // 4 pipes (1-2 PKRs) 8 bpe @ SW_4K_S3 @ RbPlus
    {   1,   33,  135,    0,    0, } , // 4 pipes (1-2 PKRs) 16 bpe @ SW_4K_S3 @ RbPlus
    {   1,   29,  131,    0,    0, } , // 8 pipes (2 PKRs) 1 bpe @ SW_4K_S3 @ RbPlus
    {   1,   30,  132,    0,    0, } , // 8 pipes (2 PKRs) 2 bpe @ SW_4K_S3 @ RbPlus
    {   1,   31,  133,    0,    0, } , // 8 pipes (2 PKRs) 4 bpe @ SW_4K_S3 @ RbPlus
    {   1,   32,  134,    0,    0, } , // 8 pipes (2 PKRs) 8 bpe @ SW_4K_S3 @ RbPlus
    {   1,   33,  135,    0,    0, } , // 8 pipes (2 PKRs) 16 bpe @ SW_4K_S3 @ RbPlus
    {   1,   29,  131,    0,    0, } , // 4 pipes (4 PKRs) 1 bpe @ SW_4K_S3 @ RbPlus
    {   1,   30,  132,    0,    0, } , // 4 pipes (4 PKRs) 2 bpe @ SW_4K_S3 @ RbPlus
    {   1,   31,  133,    0,    0, } , // 4 pipes (4 PKRs) 4 bpe @ SW_4K_S3 @ RbPlus
    {   1,   32,  134,    0,    0, } , // 4 pipes (4 PKRs) 8 bpe @ SW_4K_S3 @ RbPlus
    {   1,   33,  135,    0,    0, } , // 4 pipes (4 PKRs) 16 bpe @ SW_4K_S3 @ RbPlus
    {   1,   29,  131,    0,    0, } , // 8 pipes (4 PKRs) 1 bpe @ SW_4K_S3 @ RbPlus
    {   1,   30,  132,    0,    0, } , // 8 pipes (4 PKRs) 2 bpe @ SW_4K_S3 @ RbPlus
    {   1,   31,  133,    0,    0, } , // 8 pipes (4 PKRs) 4 bpe @ SW_4K_S3 @ RbPlus
    {   1,   32,  134,    0,    0, } , // 8 pipes (4 PKRs) 8 bpe @ SW_4K_S3 @ RbPlus
    {   1,   33,  135,    0,    0, } , // 8 pipes (4 PKRs) 16 bpe @ SW_4K_S3 @ RbPlus
    {   1,   29,  131,    0,    0, } , // 16 pipes (4 PKRs) 1 bpe @ SW_4K_S3 @ RbPlus
    {   1,   30,  132,    0,    0, } , // 16 pipes (4 PKRs) 2 bpe @ SW_4K_S3 @ RbPlus
    {   1,   31,  133,    0,    0, } , // 16 pipes (4 PKRs) 4 bpe @ SW_4K_S3 @ RbPlus
    {   1,   32,  134,    0,    0, } , // 16 pipes (4 PKRs) 8 bpe @ SW_4K_S3 @ RbPlus
    {   1,   33,  135,    0,    0, } , // 16 pipes (4 PKRs) 16 bpe @ SW_4K_S3 @ RbPlus
    {   1,   29,  131,    0,    0, } , // 8 pipes (8 PKRs) 1 bpe @ SW_4K_S3 @ RbPlus
    {   1,   30,  132,    0,    0, } , // 8 pipes (8 PKRs) 2 bpe @ SW_4K_S3 @ RbPlus
    {   1,   31,  133,    0,    0, } , // 8 pipes (8 PKRs) 4 bpe @ SW_4K_S3 @ RbPlus
    {   1,   32,  134,    0,    0, } , // 8 pipes (8 PKRs) 8 bpe @ SW_4K_S3 @ RbPlus
    {   1,   33,  135,    0,    0, } , // 8 pipes (8 PKRs) 16 bpe @ SW_4K_S3 @ RbPlus
    {   1,   29,  131,    0,    0, } , // 16 pipes (8 PKRs) 1 bpe @ SW_4K_S3 @ RbPlus
    {   1,   30,  132,    0,    0, } , // 16 pipes (8 PKRs) 2 bpe @ SW_4K_S3 @ RbPlus
    {   1,   31,  133,    0,    0, } , // 16 pipes (8 PKRs) 4 bpe @ SW_4K_S3 @ RbPlus
    {   1,   32,  134,    0,    0, } , // 16 pipes (8 PKRs) 8 bpe @ SW_4K_S3 @ RbPlus
    {   1,   33,  135,    0,    0, } , // 16 pipes (8 PKRs) 16 bpe @ SW_4K_S3 @ RbPlus
    {   1,   29,  131,    0,    0, } , // 32 pipes (8 PKRs) 1 bpe @ SW_4K_S3 @ RbPlus
    {   1,   30,  132,    0,    0, } , // 32 pipes (8 PKRs) 2 bpe @ SW_4K_S3 @ RbPlus
    {   1,   31,  133,    0,    0, } , // 32 pipes (8 PKRs) 4 bpe @ SW_4K_S3 @ RbPlus
    {   1,   32,  134,    0,    0, } , // 32 pipes (8 PKRs) 8 bpe @ SW_4K_S3 @ RbPlus
    {   1,   33,  135,    0,    0, } , // 32 pipes (8 PKRs) 16 bpe @ SW_4K_S3 @ RbPlus
    {   1,   29,  131,    0,    0, } , // 16 pipes (16 PKRs) 1 bpe @ SW_4K_S3 @ RbPlus
    {   1,   30,  132,    0,    0, } , // 16 pipes (16 PKRs) 2 bpe @ SW_4K_S3 @ RbPlus
    {   1,   31,  133,    0,    0, } , // 16 pipes (16 PKRs) 4 bpe @ SW_4K_S3 @ RbPlus
    {   1,   32,  134,    0,    0, } , // 16 pipes (16 PKRs) 8 bpe @ SW_4K_S3 @ RbPlus
    {   1,   33,  135,    0,    0, } , // 16 pipes (16 PKRs) 16 bpe @ SW_4K_S3 @ RbPlus
    {   1,   29,  131,    0,    0, } , // 32 pipes (16 PKRs) 1 bpe @ SW_4K_S3 @ RbPlus
    {   1,   30,  132,    0,    0, } , // 32 pipes (16 PKRs) 2 bpe @ SW_4K_S3 @ RbPlus
    {   1,   31,  133,    0,    0, } , // 32 pipes (16 PKRs) 4 bpe @ SW_4K_S3 @ RbPlus
    {   1,   32,  134,    0,    0, } , // 32 pipes (16 PKRs) 8 bpe @ SW_4K_S3 @ RbPlus
    {   1,   33,  135,    0,    0, } , // 32 pipes (16 PKRs) 16 bpe @ SW_4K_S3 @ RbPlus
    {   1,   29,  131,    0,    0, } , // 64 pipes (16 PKRs) 1 bpe @ SW_4K_S3 @ RbPlus
    {   1,   30,  132,    0,    0, } , // 64 pipes (16 PKRs) 2 bpe @ SW_4K_S3 @ RbPlus
    {   1,   31,  133,    0,    0, } , // 64 pipes (16 PKRs) 4 bpe @ SW_4K_S3 @ RbPlus
    {   1,   32,  134,    0,    0, } , // 64 pipes (16 PKRs) 8 bpe @ SW_4K_S3 @ RbPlus
    {   1,   33,  135,    0,    0, } , // 64 pipes (16 PKRs) 16 bpe @ SW_4K_S3 @ RbPlus
    {   1,   29,  131,    0,    0, } , // 32 pipes (32 PKRs) 1 bpe @ SW_4K_S3 @ RbPlus
    {   1,   30,  132,    0,    0, } , // 32 pipes (32 PKRs) 2 bpe @ SW_4K_S3 @ RbPlus
    {   1,   31,  133,    0,    0, } , // 32 pipes (32 PKRs) 4 bpe @ SW_4K_S3 @ RbPlus
    {   1,   32,  134,    0,    0, } , // 32 pipes (32 PKRs) 8 bpe @ SW_4K_S3 @ RbPlus
    {   1,   33,  135,    0,    0, } , // 32 pipes (32 PKRs) 16 bpe @ SW_4K_S3 @ RbPlus
    {   1,   29,  131,    0,    0, } , // 64 pipes (32 PKRs) 1 bpe @ SW_4K_S3 @ RbPlus
    {   1,   30,  132,    0,    0, } , // 64 pipes (32 PKRs) 2 bpe @ SW_4K_S3 @ RbPlus
    {   1,   31,  133,    0,    0, } , // 64 pipes (32 PKRs) 4 bpe @ SW_4K_S3 @ RbPlus
    {   1,   32,  134,    0,    0, } , // 64 pipes (32 PKRs) 8 bpe @ SW_4K_S3 @ RbPlus
    {   1,   33,  135,    0,    0, } , // 64 pipes (32 PKRs) 16 bpe @ SW_4K_S3 @ RbPlus
};

const ADDR_SW_PATINFO GFX10_SW_4K_S3_X_RBPLUS_PATINFO[] =
{
    {   1,   29,  131,    0,    0, } , // 1 pipes (1 PKRs) 1 bpe @ SW_4K_S3_X @ RbPlus
    {   1,   30,  132,    0,    0, } , // 1 pipes (1 PKRs) 2 bpe @ SW_4K_S3_X @ RbPlus
    {   1,   31,  133,    0,    0, } , // 1 pipes (1 PKRs) 4 bpe @ SW_4K_S3_X @ RbPlus
    {   1,   32,  134,    0,    0, } , // 1 pipes (1 PKRs) 8 bpe @ SW_4K_S3_X @ RbPlus
    {   1,   33,  135,    0,    0, } , // 1 pipes (1 PKRs) 16 bpe @ SW_4K_S3_X @ RbPlus
    {   3,   29,  136,    0,    0, } , // 2 pipes (1-2 PKRs) 1 bpe @ SW_4K_S3_X @ RbPlus
    {   3,   30,  137,    0,    0, } , // 2 pipes (1-2 PKRs) 2 bpe @ SW_4K_S3_X @ RbPlus
    {   3,   31,  138,    0,    0, } , // 2 pipes (1-2 PKRs) 4 bpe @ SW_4K_S3_X @ RbPlus
    {   3,   32,  139,    0,    0, } , // 2 pipes (1-2 PKRs) 8 bpe @ SW_4K_S3_X @ RbPlus
    {   3,   33,  140,    0,    0, } , // 2 pipes (1-2 PKRs) 16 bpe @ SW_4K_S3_X @ RbPlus
    {   3,   29,  141,    0,    0, } , // 4 pipes (1-2 PKRs) 1 bpe @ SW_4K_S3_X @ RbPlus
    {   3,   30,  142,    0,    0, } , // 4 pipes (1-2 PKRs) 2 bpe @ SW_4K_S3_X @ RbPlus
    {   3,   31,  143,    0,    0, } , // 4 pipes (1-2 PKRs) 4 bpe @ SW_4K_S3_X @ RbPlus
    {   3,   32,  144,    0,    0, } , // 4 pipes (1-2 PKRs) 8 bpe @ SW_4K_S3_X @ RbPlus
    {   3,   33,  145,    0,    0, } , // 4 pipes (1-2 PKRs) 16 bpe @ SW_4K_S3_X @ RbPlus
    {   3,   29,  146,    0,    0, } , // 8 pipes (2 PKRs) 1 bpe @ SW_4K_S3_X @ RbPlus
    {   3,   30,  147,    0,    0, } , // 8 pipes (2 PKRs) 2 bpe @ SW_4K_S3_X @ RbPlus
    {   3,   31,  148,    0,    0, } , // 8 pipes (2 PKRs) 4 bpe @ SW_4K_S3_X @ RbPlus
    {   3,   32,  149,    0,    0, } , // 8 pipes (2 PKRs) 8 bpe @ SW_4K_S3_X @ RbPlus
    {   3,   33,  150,    0,    0, } , // 8 pipes (2 PKRs) 16 bpe @ SW_4K_S3_X @ RbPlus
    {   3,   29,  141,    0,    0, } , // 4 pipes (4 PKRs) 1 bpe @ SW_4K_S3_X @ RbPlus
    {   3,   30,  142,    0,    0, } , // 4 pipes (4 PKRs) 2 bpe @ SW_4K_S3_X @ RbPlus
    {   3,   31,  143,    0,    0, } , // 4 pipes (4 PKRs) 4 bpe @ SW_4K_S3_X @ RbPlus
    {   3,   32,  144,    0,    0, } , // 4 pipes (4 PKRs) 8 bpe @ SW_4K_S3_X @ RbPlus
    {   3,   33,  145,    0,    0, } , // 4 pipes (4 PKRs) 16 bpe @ SW_4K_S3_X @ RbPlus
    {   3,   29,  146,    0,    0, } , // 8 pipes (4 PKRs) 1 bpe @ SW_4K_S3_X @ RbPlus
    {   3,   30,  147,    0,    0, } , // 8 pipes (4 PKRs) 2 bpe @ SW_4K_S3_X @ RbPlus
    {   3,   31,  148,    0,    0, } , // 8 pipes (4 PKRs) 4 bpe @ SW_4K_S3_X @ RbPlus
    {   3,   32,  149,    0,    0, } , // 8 pipes (4 PKRs) 8 bpe @ SW_4K_S3_X @ RbPlus
    {   3,   33,  150,    0,    0, } , // 8 pipes (4 PKRs) 16 bpe @ SW_4K_S3_X @ RbPlus
    {   3,   29,  151,    0,    0, } , // 16 pipes (4 PKRs) 1 bpe @ SW_4K_S3_X @ RbPlus
    {   3,   30,  152,    0,    0, } , // 16 pipes (4 PKRs) 2 bpe @ SW_4K_S3_X @ RbPlus
    {   3,   31,  153,    0,    0, } , // 16 pipes (4 PKRs) 4 bpe @ SW_4K_S3_X @ RbPlus
    {   3,   32,  154,    0,    0, } , // 16 pipes (4 PKRs) 8 bpe @ SW_4K_S3_X @ RbPlus
    {   3,   33,  155,    0,    0, } , // 16 pipes (4 PKRs) 16 bpe @ SW_4K_S3_X @ RbPlus
    {   3,   29,  146,    0,    0, } , // 8 pipes (8 PKRs) 1 bpe @ SW_4K_S3_X @ RbPlus
    {   3,   30,  147,    0,    0, } , // 8 pipes (8 PKRs) 2 bpe @ SW_4K_S3_X @ RbPlus
    {   3,   31,  148,    0,    0, } , // 8 pipes (8 PKRs) 4 bpe @ SW_4K_S3_X @ RbPlus
    {   3,   32,  149,    0,    0, } , // 8 pipes (8 PKRs) 8 bpe @ SW_4K_S3_X @ RbPlus
    {   3,   33,  150,    0,    0, } , // 8 pipes (8 PKRs) 16 bpe @ SW_4K_S3_X @ RbPlus
    {   3,   29,  151,    0,    0, } , // 16 pipes (8 PKRs) 1 bpe @ SW_4K_S3_X @ RbPlus
    {   3,   30,  152,    0,    0, } , // 16 pipes (8 PKRs) 2 bpe @ SW_4K_S3_X @ RbPlus
    {   3,   31,  153,    0,    0, } , // 16 pipes (8 PKRs) 4 bpe @ SW_4K_S3_X @ RbPlus
    {   3,   32,  154,    0,    0, } , // 16 pipes (8 PKRs) 8 bpe @ SW_4K_S3_X @ RbPlus
    {   3,   33,  155,    0,    0, } , // 16 pipes (8 PKRs) 16 bpe @ SW_4K_S3_X @ RbPlus
    {   3,   29,  151,    0,    0, } , // 32 pipes (8 PKRs) 1 bpe @ SW_4K_S3_X @ RbPlus
    {   3,   30,  152,    0,    0, } , // 32 pipes (8 PKRs) 2 bpe @ SW_4K_S3_X @ RbPlus
    {   3,   31,  153,    0,    0, } , // 32 pipes (8 PKRs) 4 bpe @ SW_4K_S3_X @ RbPlus
    {   3,   32,  154,    0,    0, } , // 32 pipes (8 PKRs) 8 bpe @ SW_4K_S3_X @ RbPlus
    {   3,   33,  155,    0,    0, } , // 32 pipes (8 PKRs) 16 bpe @ SW_4K_S3_X @ RbPlus
    {   3,   29,  151,    0,    0, } , // 16 pipes (16 PKRs) 1 bpe @ SW_4K_S3_X @ RbPlus
    {   3,   30,  152,    0,    0, } , // 16 pipes (16 PKRs) 2 bpe @ SW_4K_S3_X @ RbPlus
    {   3,   31,  153,    0,    0, } , // 16 pipes (16 PKRs) 4 bpe @ SW_4K_S3_X @ RbPlus
    {   3,   32,  154,    0,    0, } , // 16 pipes (16 PKRs) 8 bpe @ SW_4K_S3_X @ RbPlus
    {   3,   33,  155,    0,    0, } , // 16 pipes (16 PKRs) 16 bpe @ SW_4K_S3_X @ RbPlus
    {   3,   29,  151,    0,    0, } , // 32 pipes (16 PKRs) 1 bpe @ SW_4K_S3_X @ RbPlus
    {   3,   30,  152,    0,    0, } , // 32 pipes (16 PKRs) 2 bpe @ SW_4K_S3_X @ RbPlus
    {   3,   31,  153,    0,    0, } , // 32 pipes (16 PKRs) 4 bpe @ SW_4K_S3_X @ RbPlus
    {   3,   32,  154,    0,    0, } , // 32 pipes (16 PKRs) 8 bpe @ SW_4K_S3_X @ RbPlus
    {   3,   33,  155,    0,    0, } , // 32 pipes (16 PKRs) 16 bpe @ SW_4K_S3_X @ RbPlus
    {   3,   29,  151,    0,    0, } , // 64 pipes (16 PKRs) 1 bpe @ SW_4K_S3_X @ RbPlus
    {   3,   30,  152,    0,    0, } , // 64 pipes (16 PKRs) 2 bpe @ SW_4K_S3_X @ RbPlus
    {   3,   31,  153,    0,    0, } , // 64 pipes (16 PKRs) 4 bpe @ SW_4K_S3_X @ RbPlus
    {   3,   32,  154,    0,    0, } , // 64 pipes (16 PKRs) 8 bpe @ SW_4K_S3_X @ RbPlus
    {   3,   33,  155,    0,    0, } , // 64 pipes (16 PKRs) 16 bpe @ SW_4K_S3_X @ RbPlus
    {   3,   29,  151,    0,    0, } , // 32 pipes (32 PKRs) 1 bpe @ SW_4K_S3_X @ RbPlus
    {   3,   30,  152,    0,    0, } , // 32 pipes (32 PKRs) 2 bpe @ SW_4K_S3_X @ RbPlus
    {   3,   31,  153,    0,    0, } , // 32 pipes (32 PKRs) 4 bpe @ SW_4K_S3_X @ RbPlus
    {   3,   32,  154,    0,    0, } , // 32 pipes (32 PKRs) 8 bpe @ SW_4K_S3_X @ RbPlus
    {   3,   33,  155,    0,    0, } , // 32 pipes (32 PKRs) 16 bpe @ SW_4K_S3_X @ RbPlus
    {   3,   29,  151,    0,    0, } , // 64 pipes (32 PKRs) 1 bpe @ SW_4K_S3_X @ RbPlus
    {   3,   30,  152,    0,    0, } , // 64 pipes (32 PKRs) 2 bpe @ SW_4K_S3_X @ RbPlus
    {   3,   31,  153,    0,    0, } , // 64 pipes (32 PKRs) 4 bpe @ SW_4K_S3_X @ RbPlus
    {   3,   32,  154,    0,    0, } , // 64 pipes (32 PKRs) 8 bpe @ SW_4K_S3_X @ RbPlus
    {   3,   33,  155,    0,    0, } , // 64 pipes (32 PKRs) 16 bpe @ SW_4K_S3_X @ RbPlus
};

const ADDR_SW_PATINFO GFX10_SW_64K_S_RBPLUS_PATINFO[] =
{
    {   1,    0,    1,    1,    0, } , // 1 pipes (1 PKRs) 1 bpe @ SW_64K_S @ RbPlus
    {   1,    1,    2,    2,    0, } , // 1 pipes (1 PKRs) 2 bpe @ SW_64K_S @ RbPlus
    {   1,    2,    3,    3,    0, } , // 1 pipes (1 PKRs) 4 bpe @ SW_64K_S @ RbPlus
    {   1,    3,    4,    4,    0, } , // 1 pipes (1 PKRs) 8 bpe @ SW_64K_S @ RbPlus
    {   1,    4,    5,    5,    0, } , // 1 pipes (1 PKRs) 16 bpe @ SW_64K_S @ RbPlus
    {   1,    0,    1,    1,    0, } , // 2 pipes (1-2 PKRs) 1 bpe @ SW_64K_S @ RbPlus
    {   1,    1,    2,    2,    0, } , // 2 pipes (1-2 PKRs) 2 bpe @ SW_64K_S @ RbPlus
    {   1,    2,    3,    3,    0, } , // 2 pipes (1-2 PKRs) 4 bpe @ SW_64K_S @ RbPlus
    {   1,    3,    4,    4,    0, } , // 2 pipes (1-2 PKRs) 8 bpe @ SW_64K_S @ RbPlus
    {   1,    4,    5,    5,    0, } , // 2 pipes (1-2 PKRs) 16 bpe @ SW_64K_S @ RbPlus
    {   1,    0,    1,    1,    0, } , // 4 pipes (1-2 PKRs) 1 bpe @ SW_64K_S @ RbPlus
    {   1,    1,    2,    2,    0, } , // 4 pipes (1-2 PKRs) 2 bpe @ SW_64K_S @ RbPlus
    {   1,    2,    3,    3,    0, } , // 4 pipes (1-2 PKRs) 4 bpe @ SW_64K_S @ RbPlus
    {   1,    3,    4,    4,    0, } , // 4 pipes (1-2 PKRs) 8 bpe @ SW_64K_S @ RbPlus
    {   1,    4,    5,    5,    0, } , // 4 pipes (1-2 PKRs) 16 bpe @ SW_64K_S @ RbPlus
    {   1,    0,    1,    1,    0, } , // 8 pipes (2 PKRs) 1 bpe @ SW_64K_S @ RbPlus
    {   1,    1,    2,    2,    0, } , // 8 pipes (2 PKRs) 2 bpe @ SW_64K_S @ RbPlus
    {   1,    2,    3,    3,    0, } , // 8 pipes (2 PKRs) 4 bpe @ SW_64K_S @ RbPlus
    {   1,    3,    4,    4,    0, } , // 8 pipes (2 PKRs) 8 bpe @ SW_64K_S @ RbPlus
    {   1,    4,    5,    5,    0, } , // 8 pipes (2 PKRs) 16 bpe @ SW_64K_S @ RbPlus
    {   1,    0,    1,    1,    0, } , // 4 pipes (4 PKRs) 1 bpe @ SW_64K_S @ RbPlus
    {   1,    1,    2,    2,    0, } , // 4 pipes (4 PKRs) 2 bpe @ SW_64K_S @ RbPlus
    {   1,    2,    3,    3,    0, } , // 4 pipes (4 PKRs) 4 bpe @ SW_64K_S @ RbPlus
    {   1,    3,    4,    4,    0, } , // 4 pipes (4 PKRs) 8 bpe @ SW_64K_S @ RbPlus
    {   1,    4,    5,    5,    0, } , // 4 pipes (4 PKRs) 16 bpe @ SW_64K_S @ RbPlus
    {   1,    0,    1,    1,    0, } , // 8 pipes (4 PKRs) 1 bpe @ SW_64K_S @ RbPlus
    {   1,    1,    2,    2,    0, } , // 8 pipes (4 PKRs) 2 bpe @ SW_64K_S @ RbPlus
    {   1,    2,    3,    3,    0, } , // 8 pipes (4 PKRs) 4 bpe @ SW_64K_S @ RbPlus
    {   1,    3,    4,    4,    0, } , // 8 pipes (4 PKRs) 8 bpe @ SW_64K_S @ RbPlus
    {   1,    4,    5,    5,    0, } , // 8 pipes (4 PKRs) 16 bpe @ SW_64K_S @ RbPlus
    {   1,    0,    1,    1,    0, } , // 16 pipes (4 PKRs) 1 bpe @ SW_64K_S @ RbPlus
    {   1,    1,    2,    2,    0, } , // 16 pipes (4 PKRs) 2 bpe @ SW_64K_S @ RbPlus
    {   1,    2,    3,    3,    0, } , // 16 pipes (4 PKRs) 4 bpe @ SW_64K_S @ RbPlus
    {   1,    3,    4,    4,    0, } , // 16 pipes (4 PKRs) 8 bpe @ SW_64K_S @ RbPlus
    {   1,    4,    5,    5,    0, } , // 16 pipes (4 PKRs) 16 bpe @ SW_64K_S @ RbPlus
    {   1,    0,    1,    1,    0, } , // 8 pipes (8 PKRs) 1 bpe @ SW_64K_S @ RbPlus
    {   1,    1,    2,    2,    0, } , // 8 pipes (8 PKRs) 2 bpe @ SW_64K_S @ RbPlus
    {   1,    2,    3,    3,    0, } , // 8 pipes (8 PKRs) 4 bpe @ SW_64K_S @ RbPlus
    {   1,    3,    4,    4,    0, } , // 8 pipes (8 PKRs) 8 bpe @ SW_64K_S @ RbPlus
    {   1,    4,    5,    5,    0, } , // 8 pipes (8 PKRs) 16 bpe @ SW_64K_S @ RbPlus
    {   1,    0,    1,    1,    0, } , // 16 pipes (8 PKRs) 1 bpe @ SW_64K_S @ RbPlus
    {   1,    1,    2,    2,    0, } , // 16 pipes (8 PKRs) 2 bpe @ SW_64K_S @ RbPlus
    {   1,    2,    3,    3,    0, } , // 16 pipes (8 PKRs) 4 bpe @ SW_64K_S @ RbPlus
    {   1,    3,    4,    4,    0, } , // 16 pipes (8 PKRs) 8 bpe @ SW_64K_S @ RbPlus
    {   1,    4,    5,    5,    0, } , // 16 pipes (8 PKRs) 16 bpe @ SW_64K_S @ RbPlus
    {   1,    0,    1,    1,    0, } , // 32 pipes (8 PKRs) 1 bpe @ SW_64K_S @ RbPlus
    {   1,    1,    2,    2,    0, } , // 32 pipes (8 PKRs) 2 bpe @ SW_64K_S @ RbPlus
    {   1,    2,    3,    3,    0, } , // 32 pipes (8 PKRs) 4 bpe @ SW_64K_S @ RbPlus
    {   1,    3,    4,    4,    0, } , // 32 pipes (8 PKRs) 8 bpe @ SW_64K_S @ RbPlus
    {   1,    4,    5,    5,    0, } , // 32 pipes (8 PKRs) 16 bpe @ SW_64K_S @ RbPlus
    {   1,    0,    1,    1,    0, } , // 16 pipes (16 PKRs) 1 bpe @ SW_64K_S @ RbPlus
    {   1,    1,    2,    2,    0, } , // 16 pipes (16 PKRs) 2 bpe @ SW_64K_S @ RbPlus
    {   1,    2,    3,    3,    0, } , // 16 pipes (16 PKRs) 4 bpe @ SW_64K_S @ RbPlus
    {   1,    3,    4,    4,    0, } , // 16 pipes (16 PKRs) 8 bpe @ SW_64K_S @ RbPlus
    {   1,    4,    5,    5,    0, } , // 16 pipes (16 PKRs) 16 bpe @ SW_64K_S @ RbPlus
    {   1,    0,    1,    1,    0, } , // 32 pipes (16 PKRs) 1 bpe @ SW_64K_S @ RbPlus
    {   1,    1,    2,    2,    0, } , // 32 pipes (16 PKRs) 2 bpe @ SW_64K_S @ RbPlus
    {   1,    2,    3,    3,    0, } , // 32 pipes (16 PKRs) 4 bpe @ SW_64K_S @ RbPlus
    {   1,    3,    4,    4,    0, } , // 32 pipes (16 PKRs) 8 bpe @ SW_64K_S @ RbPlus
    {   1,    4,    5,    5,    0, } , // 32 pipes (16 PKRs) 16 bpe @ SW_64K_S @ RbPlus
    {   1,    0,    1,    1,    0, } , // 64 pipes (16 PKRs) 1 bpe @ SW_64K_S @ RbPlus
    {   1,    1,    2,    2,    0, } , // 64 pipes (16 PKRs) 2 bpe @ SW_64K_S @ RbPlus
    {   1,    2,    3,    3,    0, } , // 64 pipes (16 PKRs) 4 bpe @ SW_64K_S @ RbPlus
    {   1,    3,    4,    4,    0, } , // 64 pipes (16 PKRs) 8 bpe @ SW_64K_S @ RbPlus
    {   1,    4,    5,    5,    0, } , // 64 pipes (16 PKRs) 16 bpe @ SW_64K_S @ RbPlus
    {   1,    0,    1,    1,    0, } , // 32 pipes (32 PKRs) 1 bpe @ SW_64K_S @ RbPlus
    {   1,    1,    2,    2,    0, } , // 32 pipes (32 PKRs) 2 bpe @ SW_64K_S @ RbPlus
    {   1,    2,    3,    3,    0, } , // 32 pipes (32 PKRs) 4 bpe @ SW_64K_S @ RbPlus
    {   1,    3,    4,    4,    0, } , // 32 pipes (32 PKRs) 8 bpe @ SW_64K_S @ RbPlus
    {   1,    4,    5,    5,    0, } , // 32 pipes (32 PKRs) 16 bpe @ SW_64K_S @ RbPlus
    {   1,    0,    1,    1,    0, } , // 64 pipes (32 PKRs) 1 bpe @ SW_64K_S @ RbPlus
    {   1,    1,    2,    2,    0, } , // 64 pipes (32 PKRs) 2 bpe @ SW_64K_S @ RbPlus
    {   1,    2,    3,    3,    0, } , // 64 pipes (32 PKRs) 4 bpe @ SW_64K_S @ RbPlus
    {   1,    3,    4,    4,    0, } , // 64 pipes (32 PKRs) 8 bpe @ SW_64K_S @ RbPlus
    {   1,    4,    5,    5,    0, } , // 64 pipes (32 PKRs) 16 bpe @ SW_64K_S @ RbPlus
};

const ADDR_SW_PATINFO GFX10_SW_64K_D_RBPLUS_PATINFO[] =
{
    {   1,    5,    1,    1,    0, } , // 1 pipes (1 PKRs) 1 bpe @ SW_64K_D @ RbPlus
    {   1,    1,    2,    2,    0, } , // 1 pipes (1 PKRs) 2 bpe @ SW_64K_D @ RbPlus
    {   1,   39,    3,    3,    0, } , // 1 pipes (1 PKRs) 4 bpe @ SW_64K_D @ RbPlus
    {   1,    6,    4,    4,    0, } , // 1 pipes (1 PKRs) 8 bpe @ SW_64K_D @ RbPlus
    {   1,    7,    5,    5,    0, } , // 1 pipes (1 PKRs) 16 bpe @ SW_64K_D @ RbPlus
    {   1,    5,    1,    1,    0, } , // 2 pipes (1-2 PKRs) 1 bpe @ SW_64K_D @ RbPlus
    {   1,    1,    2,    2,    0, } , // 2 pipes (1-2 PKRs) 2 bpe @ SW_64K_D @ RbPlus
    {   1,   39,    3,    3,    0, } , // 2 pipes (1-2 PKRs) 4 bpe @ SW_64K_D @ RbPlus
    {   1,    6,    4,    4,    0, } , // 2 pipes (1-2 PKRs) 8 bpe @ SW_64K_D @ RbPlus
    {   1,    7,    5,    5,    0, } , // 2 pipes (1-2 PKRs) 16 bpe @ SW_64K_D @ RbPlus
    {   1,    5,    1,    1,    0, } , // 4 pipes (1-2 PKRs) 1 bpe @ SW_64K_D @ RbPlus
    {   1,    1,    2,    2,    0, } , // 4 pipes (1-2 PKRs) 2 bpe @ SW_64K_D @ RbPlus
    {   1,   39,    3,    3,    0, } , // 4 pipes (1-2 PKRs) 4 bpe @ SW_64K_D @ RbPlus
    {   1,    6,    4,    4,    0, } , // 4 pipes (1-2 PKRs) 8 bpe @ SW_64K_D @ RbPlus
    {   1,    7,    5,    5,    0, } , // 4 pipes (1-2 PKRs) 16 bpe @ SW_64K_D @ RbPlus
    {   1,    5,    1,    1,    0, } , // 8 pipes (2 PKRs) 1 bpe @ SW_64K_D @ RbPlus
    {   1,    1,    2,    2,    0, } , // 8 pipes (2 PKRs) 2 bpe @ SW_64K_D @ RbPlus
    {   1,   39,    3,    3,    0, } , // 8 pipes (2 PKRs) 4 bpe @ SW_64K_D @ RbPlus
    {   1,    6,    4,    4,    0, } , // 8 pipes (2 PKRs) 8 bpe @ SW_64K_D @ RbPlus
    {   1,    7,    5,    5,    0, } , // 8 pipes (2 PKRs) 16 bpe @ SW_64K_D @ RbPlus
    {   1,    5,    1,    1,    0, } , // 4 pipes (4 PKRs) 1 bpe @ SW_64K_D @ RbPlus
    {   1,    1,    2,    2,    0, } , // 4 pipes (4 PKRs) 2 bpe @ SW_64K_D @ RbPlus
    {   1,   39,    3,    3,    0, } , // 4 pipes (4 PKRs) 4 bpe @ SW_64K_D @ RbPlus
    {   1,    6,    4,    4,    0, } , // 4 pipes (4 PKRs) 8 bpe @ SW_64K_D @ RbPlus
    {   1,    7,    5,    5,    0, } , // 4 pipes (4 PKRs) 16 bpe @ SW_64K_D @ RbPlus
    {   1,    5,    1,    1,    0, } , // 8 pipes (4 PKRs) 1 bpe @ SW_64K_D @ RbPlus
    {   1,    1,    2,    2,    0, } , // 8 pipes (4 PKRs) 2 bpe @ SW_64K_D @ RbPlus
    {   1,   39,    3,    3,    0, } , // 8 pipes (4 PKRs) 4 bpe @ SW_64K_D @ RbPlus
    {   1,    6,    4,    4,    0, } , // 8 pipes (4 PKRs) 8 bpe @ SW_64K_D @ RbPlus
    {   1,    7,    5,    5,    0, } , // 8 pipes (4 PKRs) 16 bpe @ SW_64K_D @ RbPlus
    {   1,    5,    1,    1,    0, } , // 16 pipes (4 PKRs) 1 bpe @ SW_64K_D @ RbPlus
    {   1,    1,    2,    2,    0, } , // 16 pipes (4 PKRs) 2 bpe @ SW_64K_D @ RbPlus
    {   1,   39,    3,    3,    0, } , // 16 pipes (4 PKRs) 4 bpe @ SW_64K_D @ RbPlus
    {   1,    6,    4,    4,    0, } , // 16 pipes (4 PKRs) 8 bpe @ SW_64K_D @ RbPlus
    {   1,    7,    5,    5,    0, } , // 16 pipes (4 PKRs) 16 bpe @ SW_64K_D @ RbPlus
    {   1,    5,    1,    1,    0, } , // 8 pipes (8 PKRs) 1 bpe @ SW_64K_D @ RbPlus
    {   1,    1,    2,    2,    0, } , // 8 pipes (8 PKRs) 2 bpe @ SW_64K_D @ RbPlus
    {   1,   39,    3,    3,    0, } , // 8 pipes (8 PKRs) 4 bpe @ SW_64K_D @ RbPlus
    {   1,    6,    4,    4,    0, } , // 8 pipes (8 PKRs) 8 bpe @ SW_64K_D @ RbPlus
    {   1,    7,    5,    5,    0, } , // 8 pipes (8 PKRs) 16 bpe @ SW_64K_D @ RbPlus
    {   1,    5,    1,    1,    0, } , // 16 pipes (8 PKRs) 1 bpe @ SW_64K_D @ RbPlus
    {   1,    1,    2,    2,    0, } , // 16 pipes (8 PKRs) 2 bpe @ SW_64K_D @ RbPlus
    {   1,   39,    3,    3,    0, } , // 16 pipes (8 PKRs) 4 bpe @ SW_64K_D @ RbPlus
    {   1,    6,    4,    4,    0, } , // 16 pipes (8 PKRs) 8 bpe @ SW_64K_D @ RbPlus
    {   1,    7,    5,    5,    0, } , // 16 pipes (8 PKRs) 16 bpe @ SW_64K_D @ RbPlus
    {   1,    5,    1,    1,    0, } , // 32 pipes (8 PKRs) 1 bpe @ SW_64K_D @ RbPlus
    {   1,    1,    2,    2,    0, } , // 32 pipes (8 PKRs) 2 bpe @ SW_64K_D @ RbPlus
    {   1,   39,    3,    3,    0, } , // 32 pipes (8 PKRs) 4 bpe @ SW_64K_D @ RbPlus
    {   1,    6,    4,    4,    0, } , // 32 pipes (8 PKRs) 8 bpe @ SW_64K_D @ RbPlus
    {   1,    7,    5,    5,    0, } , // 32 pipes (8 PKRs) 16 bpe @ SW_64K_D @ RbPlus
    {   1,    5,    1,    1,    0, } , // 16 pipes (16 PKRs) 1 bpe @ SW_64K_D @ RbPlus
    {   1,    1,    2,    2,    0, } , // 16 pipes (16 PKRs) 2 bpe @ SW_64K_D @ RbPlus
    {   1,   39,    3,    3,    0, } , // 16 pipes (16 PKRs) 4 bpe @ SW_64K_D @ RbPlus
    {   1,    6,    4,    4,    0, } , // 16 pipes (16 PKRs) 8 bpe @ SW_64K_D @ RbPlus
    {   1,    7,    5,    5,    0, } , // 16 pipes (16 PKRs) 16 bpe @ SW_64K_D @ RbPlus
    {   1,    5,    1,    1,    0, } , // 32 pipes (16 PKRs) 1 bpe @ SW_64K_D @ RbPlus
    {   1,    1,    2,    2,    0, } , // 32 pipes (16 PKRs) 2 bpe @ SW_64K_D @ RbPlus
    {   1,   39,    3,    3,    0, } , // 32 pipes (16 PKRs) 4 bpe @ SW_64K_D @ RbPlus
    {   1,    6,    4,    4,    0, } , // 32 pipes (16 PKRs) 8 bpe @ SW_64K_D @ RbPlus
    {   1,    7,    5,    5,    0, } , // 32 pipes (16 PKRs) 16 bpe @ SW_64K_D @ RbPlus
    {   1,    5,    1,    1,    0, } , // 64 pipes (16 PKRs) 1 bpe @ SW_64K_D @ RbPlus
    {   1,    1,    2,    2,    0, } , // 64 pipes (16 PKRs) 2 bpe @ SW_64K_D @ RbPlus
    {   1,   39,    3,    3,    0, } , // 64 pipes (16 PKRs) 4 bpe @ SW_64K_D @ RbPlus
    {   1,    6,    4,    4,    0, } , // 64 pipes (16 PKRs) 8 bpe @ SW_64K_D @ RbPlus
    {   1,    7,    5,    5,    0, } , // 64 pipes (16 PKRs) 16 bpe @ SW_64K_D @ RbPlus
    {   1,    5,    1,    1,    0, } , // 32 pipes (32 PKRs) 1 bpe @ SW_64K_D @ RbPlus
    {   1,    1,    2,    2,    0, } , // 32 pipes (32 PKRs) 2 bpe @ SW_64K_D @ RbPlus
    {   1,   39,    3,    3,    0, } , // 32 pipes (32 PKRs) 4 bpe @ SW_64K_D @ RbPlus
    {   1,    6,    4,    4,    0, } , // 32 pipes (32 PKRs) 8 bpe @ SW_64K_D @ RbPlus
    {   1,    7,    5,    5,    0, } , // 32 pipes (32 PKRs) 16 bpe @ SW_64K_D @ RbPlus
    {   1,    5,    1,    1,    0, } , // 64 pipes (32 PKRs) 1 bpe @ SW_64K_D @ RbPlus
    {   1,    1,    2,    2,    0, } , // 64 pipes (32 PKRs) 2 bpe @ SW_64K_D @ RbPlus
    {   1,   39,    3,    3,    0, } , // 64 pipes (32 PKRs) 4 bpe @ SW_64K_D @ RbPlus
    {   1,    6,    4,    4,    0, } , // 64 pipes (32 PKRs) 8 bpe @ SW_64K_D @ RbPlus
    {   1,    7,    5,    5,    0, } , // 64 pipes (32 PKRs) 16 bpe @ SW_64K_D @ RbPlus
};

const ADDR_SW_PATINFO GFX10_SW_64K_S_T_RBPLUS_PATINFO[] =
{
    {   1,    0,    1,    1,    0, } , // 1 pipes (1 PKRs) 1 bpe @ SW_64K_S_T @ RbPlus
    {   1,    1,    2,    2,    0, } , // 1 pipes (1 PKRs) 2 bpe @ SW_64K_S_T @ RbPlus
    {   1,    2,    3,    3,    0, } , // 1 pipes (1 PKRs) 4 bpe @ SW_64K_S_T @ RbPlus
    {   1,    3,    4,    4,    0, } , // 1 pipes (1 PKRs) 8 bpe @ SW_64K_S_T @ RbPlus
    {   1,    4,    5,    5,    0, } , // 1 pipes (1 PKRs) 16 bpe @ SW_64K_S_T @ RbPlus
    {   2,    0,   36,    1,    0, } , // 2 pipes (1-2 PKRs) 1 bpe @ SW_64K_S_T @ RbPlus
    {   2,    1,   37,    2,    0, } , // 2 pipes (1-2 PKRs) 2 bpe @ SW_64K_S_T @ RbPlus
    {   2,    2,   38,    3,    0, } , // 2 pipes (1-2 PKRs) 4 bpe @ SW_64K_S_T @ RbPlus
    {   2,    3,   39,    4,    0, } , // 2 pipes (1-2 PKRs) 8 bpe @ SW_64K_S_T @ RbPlus
    {   2,    4,   40,    5,    0, } , // 2 pipes (1-2 PKRs) 16 bpe @ SW_64K_S_T @ RbPlus
    {   2,    0,   41,    1,    0, } , // 4 pipes (1-2 PKRs) 1 bpe @ SW_64K_S_T @ RbPlus
    {   2,    1,   42,    2,    0, } , // 4 pipes (1-2 PKRs) 2 bpe @ SW_64K_S_T @ RbPlus
    {   2,    2,   43,    3,    0, } , // 4 pipes (1-2 PKRs) 4 bpe @ SW_64K_S_T @ RbPlus
    {   2,    3,   44,    4,    0, } , // 4 pipes (1-2 PKRs) 8 bpe @ SW_64K_S_T @ RbPlus
    {   2,    4,   45,    5,    0, } , // 4 pipes (1-2 PKRs) 16 bpe @ SW_64K_S_T @ RbPlus
    {   2,    0,   46,    1,    0, } , // 8 pipes (2 PKRs) 1 bpe @ SW_64K_S_T @ RbPlus
    {   2,    1,   47,    2,    0, } , // 8 pipes (2 PKRs) 2 bpe @ SW_64K_S_T @ RbPlus
    {   2,    2,   48,    3,    0, } , // 8 pipes (2 PKRs) 4 bpe @ SW_64K_S_T @ RbPlus
    {   2,    3,   49,    4,    0, } , // 8 pipes (2 PKRs) 8 bpe @ SW_64K_S_T @ RbPlus
    {   2,    4,   50,    5,    0, } , // 8 pipes (2 PKRs) 16 bpe @ SW_64K_S_T @ RbPlus
    {   2,    0,   41,    1,    0, } , // 4 pipes (4 PKRs) 1 bpe @ SW_64K_S_T @ RbPlus
    {   2,    1,   42,    2,    0, } , // 4 pipes (4 PKRs) 2 bpe @ SW_64K_S_T @ RbPlus
    {   2,    2,   43,    3,    0, } , // 4 pipes (4 PKRs) 4 bpe @ SW_64K_S_T @ RbPlus
    {   2,    3,   44,    4,    0, } , // 4 pipes (4 PKRs) 8 bpe @ SW_64K_S_T @ RbPlus
    {   2,    4,   45,    5,    0, } , // 4 pipes (4 PKRs) 16 bpe @ SW_64K_S_T @ RbPlus
    {   2,    0,   46,    1,    0, } , // 8 pipes (4 PKRs) 1 bpe @ SW_64K_S_T @ RbPlus
    {   2,    1,   47,    2,    0, } , // 8 pipes (4 PKRs) 2 bpe @ SW_64K_S_T @ RbPlus
    {   2,    2,   48,    3,    0, } , // 8 pipes (4 PKRs) 4 bpe @ SW_64K_S_T @ RbPlus
    {   2,    3,   49,    4,    0, } , // 8 pipes (4 PKRs) 8 bpe @ SW_64K_S_T @ RbPlus
    {   2,    4,   50,    5,    0, } , // 8 pipes (4 PKRs) 16 bpe @ SW_64K_S_T @ RbPlus
    {   2,    0,   51,    1,    0, } , // 16 pipes (4 PKRs) 1 bpe @ SW_64K_S_T @ RbPlus
    {   2,    1,   52,    2,    0, } , // 16 pipes (4 PKRs) 2 bpe @ SW_64K_S_T @ RbPlus
    {   2,    2,   53,    3,    0, } , // 16 pipes (4 PKRs) 4 bpe @ SW_64K_S_T @ RbPlus
    {   2,    3,   54,    4,    0, } , // 16 pipes (4 PKRs) 8 bpe @ SW_64K_S_T @ RbPlus
    {   2,    4,   55,    5,    0, } , // 16 pipes (4 PKRs) 16 bpe @ SW_64K_S_T @ RbPlus
    {   2,    0,   46,    1,    0, } , // 8 pipes (8 PKRs) 1 bpe @ SW_64K_S_T @ RbPlus
    {   2,    1,   47,    2,    0, } , // 8 pipes (8 PKRs) 2 bpe @ SW_64K_S_T @ RbPlus
    {   2,    2,   48,    3,    0, } , // 8 pipes (8 PKRs) 4 bpe @ SW_64K_S_T @ RbPlus
    {   2,    3,   49,    4,    0, } , // 8 pipes (8 PKRs) 8 bpe @ SW_64K_S_T @ RbPlus
    {   2,    4,   50,    5,    0, } , // 8 pipes (8 PKRs) 16 bpe @ SW_64K_S_T @ RbPlus
    {   2,    0,   51,    1,    0, } , // 16 pipes (8 PKRs) 1 bpe @ SW_64K_S_T @ RbPlus
    {   2,    1,   52,    2,    0, } , // 16 pipes (8 PKRs) 2 bpe @ SW_64K_S_T @ RbPlus
    {   2,    2,   53,    3,    0, } , // 16 pipes (8 PKRs) 4 bpe @ SW_64K_S_T @ RbPlus
    {   2,    3,   54,    4,    0, } , // 16 pipes (8 PKRs) 8 bpe @ SW_64K_S_T @ RbPlus
    {   2,    4,   55,    5,    0, } , // 16 pipes (8 PKRs) 16 bpe @ SW_64K_S_T @ RbPlus
    {   2,    0,   56,   16,    0, } , // 32 pipes (8 PKRs) 1 bpe @ SW_64K_S_T @ RbPlus
    {   2,    1,   57,   17,    0, } , // 32 pipes (8 PKRs) 2 bpe @ SW_64K_S_T @ RbPlus
    {   2,    2,   58,   18,    0, } , // 32 pipes (8 PKRs) 4 bpe @ SW_64K_S_T @ RbPlus
    {   2,    3,   59,   19,    0, } , // 32 pipes (8 PKRs) 8 bpe @ SW_64K_S_T @ RbPlus
    {   2,    4,   60,   20,    0, } , // 32 pipes (8 PKRs) 16 bpe @ SW_64K_S_T @ RbPlus
    {   2,    0,   51,    1,    0, } , // 16 pipes (16 PKRs) 1 bpe @ SW_64K_S_T @ RbPlus
    {   2,    1,   52,    2,    0, } , // 16 pipes (16 PKRs) 2 bpe @ SW_64K_S_T @ RbPlus
    {   2,    2,   53,    3,    0, } , // 16 pipes (16 PKRs) 4 bpe @ SW_64K_S_T @ RbPlus
    {   2,    3,   54,    4,    0, } , // 16 pipes (16 PKRs) 8 bpe @ SW_64K_S_T @ RbPlus
    {   2,    4,   55,    5,    0, } , // 16 pipes (16 PKRs) 16 bpe @ SW_64K_S_T @ RbPlus
    {   2,    0,   56,   16,    0, } , // 32 pipes (16 PKRs) 1 bpe @ SW_64K_S_T @ RbPlus
    {   2,    1,   57,   17,    0, } , // 32 pipes (16 PKRs) 2 bpe @ SW_64K_S_T @ RbPlus
    {   2,    2,   58,   18,    0, } , // 32 pipes (16 PKRs) 4 bpe @ SW_64K_S_T @ RbPlus
    {   2,    3,   59,   19,    0, } , // 32 pipes (16 PKRs) 8 bpe @ SW_64K_S_T @ RbPlus
    {   2,    4,   60,   20,    0, } , // 32 pipes (16 PKRs) 16 bpe @ SW_64K_S_T @ RbPlus
    {   2,    0,    1,   21,    0, } , // 64 pipes (16 PKRs) 1 bpe @ SW_64K_S_T @ RbPlus
    {   2,    1,    2,   22,    0, } , // 64 pipes (16 PKRs) 2 bpe @ SW_64K_S_T @ RbPlus
    {   2,    2,    3,   23,    0, } , // 64 pipes (16 PKRs) 4 bpe @ SW_64K_S_T @ RbPlus
    {   2,    3,    4,   24,    0, } , // 64 pipes (16 PKRs) 8 bpe @ SW_64K_S_T @ RbPlus
    {   2,    4,    5,   25,    0, } , // 64 pipes (16 PKRs) 16 bpe @ SW_64K_S_T @ RbPlus
    {   2,    0,   56,   16,    0, } , // 32 pipes (32 PKRs) 1 bpe @ SW_64K_S_T @ RbPlus
    {   2,    1,   57,   17,    0, } , // 32 pipes (32 PKRs) 2 bpe @ SW_64K_S_T @ RbPlus
    {   2,    2,   58,   18,    0, } , // 32 pipes (32 PKRs) 4 bpe @ SW_64K_S_T @ RbPlus
    {   2,    3,   59,   19,    0, } , // 32 pipes (32 PKRs) 8 bpe @ SW_64K_S_T @ RbPlus
    {   2,    4,   60,   20,    0, } , // 32 pipes (32 PKRs) 16 bpe @ SW_64K_S_T @ RbPlus
    {   2,    0,    1,   21,    0, } , // 64 pipes (32 PKRs) 1 bpe @ SW_64K_S_T @ RbPlus
    {   2,    1,    2,   22,    0, } , // 64 pipes (32 PKRs) 2 bpe @ SW_64K_S_T @ RbPlus
    {   2,    2,    3,   23,    0, } , // 64 pipes (32 PKRs) 4 bpe @ SW_64K_S_T @ RbPlus
    {   2,    3,    4,   24,    0, } , // 64 pipes (32 PKRs) 8 bpe @ SW_64K_S_T @ RbPlus
    {   2,    4,    5,   25,    0, } , // 64 pipes (32 PKRs) 16 bpe @ SW_64K_S_T @ RbPlus
};

const ADDR_SW_PATINFO GFX10_SW_64K_D_T_RBPLUS_PATINFO[] =
{
    {   1,    5,    1,    1,    0, } , // 1 pipes (1 PKRs) 1 bpe @ SW_64K_D_T @ RbPlus
    {   1,    1,    2,    2,    0, } , // 1 pipes (1 PKRs) 2 bpe @ SW_64K_D_T @ RbPlus
    {   1,   39,    3,    3,    0, } , // 1 pipes (1 PKRs) 4 bpe @ SW_64K_D_T @ RbPlus
    {   1,    6,    4,    4,    0, } , // 1 pipes (1 PKRs) 8 bpe @ SW_64K_D_T @ RbPlus
    {   1,    7,    5,    5,    0, } , // 1 pipes (1 PKRs) 16 bpe @ SW_64K_D_T @ RbPlus
    {   2,    5,   36,    1,    0, } , // 2 pipes (1-2 PKRs) 1 bpe @ SW_64K_D_T @ RbPlus
    {   2,    1,   37,    2,    0, } , // 2 pipes (1-2 PKRs) 2 bpe @ SW_64K_D_T @ RbPlus
    {   2,   39,   38,    3,    0, } , // 2 pipes (1-2 PKRs) 4 bpe @ SW_64K_D_T @ RbPlus
    {   2,    6,   39,    4,    0, } , // 2 pipes (1-2 PKRs) 8 bpe @ SW_64K_D_T @ RbPlus
    {   2,    7,   40,    5,    0, } , // 2 pipes (1-2 PKRs) 16 bpe @ SW_64K_D_T @ RbPlus
    {   2,    5,   41,    1,    0, } , // 4 pipes (1-2 PKRs) 1 bpe @ SW_64K_D_T @ RbPlus
    {   2,    1,   42,    2,    0, } , // 4 pipes (1-2 PKRs) 2 bpe @ SW_64K_D_T @ RbPlus
    {   2,   39,   43,    3,    0, } , // 4 pipes (1-2 PKRs) 4 bpe @ SW_64K_D_T @ RbPlus
    {   2,    6,   44,    4,    0, } , // 4 pipes (1-2 PKRs) 8 bpe @ SW_64K_D_T @ RbPlus
    {   2,    7,   45,    5,    0, } , // 4 pipes (1-2 PKRs) 16 bpe @ SW_64K_D_T @ RbPlus
    {   2,    5,   46,    1,    0, } , // 8 pipes (2 PKRs) 1 bpe @ SW_64K_D_T @ RbPlus
    {   2,    1,   47,    2,    0, } , // 8 pipes (2 PKRs) 2 bpe @ SW_64K_D_T @ RbPlus
    {   2,   39,   48,    3,    0, } , // 8 pipes (2 PKRs) 4 bpe @ SW_64K_D_T @ RbPlus
    {   2,    6,   49,    4,    0, } , // 8 pipes (2 PKRs) 8 bpe @ SW_64K_D_T @ RbPlus
    {   2,    7,   50,    5,    0, } , // 8 pipes (2 PKRs) 16 bpe @ SW_64K_D_T @ RbPlus
    {   2,    5,   41,    1,    0, } , // 4 pipes (4 PKRs) 1 bpe @ SW_64K_D_T @ RbPlus
    {   2,    1,   42,    2,    0, } , // 4 pipes (4 PKRs) 2 bpe @ SW_64K_D_T @ RbPlus
    {   2,   39,   43,    3,    0, } , // 4 pipes (4 PKRs) 4 bpe @ SW_64K_D_T @ RbPlus
    {   2,    6,   44,    4,    0, } , // 4 pipes (4 PKRs) 8 bpe @ SW_64K_D_T @ RbPlus
    {   2,    7,   45,    5,    0, } , // 4 pipes (4 PKRs) 16 bpe @ SW_64K_D_T @ RbPlus
    {   2,    5,   46,    1,    0, } , // 8 pipes (4 PKRs) 1 bpe @ SW_64K_D_T @ RbPlus
    {   2,    1,   47,    2,    0, } , // 8 pipes (4 PKRs) 2 bpe @ SW_64K_D_T @ RbPlus
    {   2,   39,   48,    3,    0, } , // 8 pipes (4 PKRs) 4 bpe @ SW_64K_D_T @ RbPlus
    {   2,    6,   49,    4,    0, } , // 8 pipes (4 PKRs) 8 bpe @ SW_64K_D_T @ RbPlus
    {   2,    7,   50,    5,    0, } , // 8 pipes (4 PKRs) 16 bpe @ SW_64K_D_T @ RbPlus
    {   2,    5,   51,    1,    0, } , // 16 pipes (4 PKRs) 1 bpe @ SW_64K_D_T @ RbPlus
    {   2,    1,   52,    2,    0, } , // 16 pipes (4 PKRs) 2 bpe @ SW_64K_D_T @ RbPlus
    {   2,   39,   53,    3,    0, } , // 16 pipes (4 PKRs) 4 bpe @ SW_64K_D_T @ RbPlus
    {   2,    6,   54,    4,    0, } , // 16 pipes (4 PKRs) 8 bpe @ SW_64K_D_T @ RbPlus
    {   2,    7,   55,    5,    0, } , // 16 pipes (4 PKRs) 16 bpe @ SW_64K_D_T @ RbPlus
    {   2,    5,   46,    1,    0, } , // 8 pipes (8 PKRs) 1 bpe @ SW_64K_D_T @ RbPlus
    {   2,    1,   47,    2,    0, } , // 8 pipes (8 PKRs) 2 bpe @ SW_64K_D_T @ RbPlus
    {   2,   39,   48,    3,    0, } , // 8 pipes (8 PKRs) 4 bpe @ SW_64K_D_T @ RbPlus
    {   2,    6,   49,    4,    0, } , // 8 pipes (8 PKRs) 8 bpe @ SW_64K_D_T @ RbPlus
    {   2,    7,   50,    5,    0, } , // 8 pipes (8 PKRs) 16 bpe @ SW_64K_D_T @ RbPlus
    {   2,    5,   51,    1,    0, } , // 16 pipes (8 PKRs) 1 bpe @ SW_64K_D_T @ RbPlus
    {   2,    1,   52,    2,    0, } , // 16 pipes (8 PKRs) 2 bpe @ SW_64K_D_T @ RbPlus
    {   2,   39,   53,    3,    0, } , // 16 pipes (8 PKRs) 4 bpe @ SW_64K_D_T @ RbPlus
    {   2,    6,   54,    4,    0, } , // 16 pipes (8 PKRs) 8 bpe @ SW_64K_D_T @ RbPlus
    {   2,    7,   55,    5,    0, } , // 16 pipes (8 PKRs) 16 bpe @ SW_64K_D_T @ RbPlus
    {   2,    5,   56,   16,    0, } , // 32 pipes (8 PKRs) 1 bpe @ SW_64K_D_T @ RbPlus
    {   2,    1,   57,   17,    0, } , // 32 pipes (8 PKRs) 2 bpe @ SW_64K_D_T @ RbPlus
    {   2,   39,   58,   18,    0, } , // 32 pipes (8 PKRs) 4 bpe @ SW_64K_D_T @ RbPlus
    {   2,    6,   59,   19,    0, } , // 32 pipes (8 PKRs) 8 bpe @ SW_64K_D_T @ RbPlus
    {   2,    7,   60,   20,    0, } , // 32 pipes (8 PKRs) 16 bpe @ SW_64K_D_T @ RbPlus
    {   2,    5,   51,    1,    0, } , // 16 pipes (16 PKRs) 1 bpe @ SW_64K_D_T @ RbPlus
    {   2,    1,   52,    2,    0, } , // 16 pipes (16 PKRs) 2 bpe @ SW_64K_D_T @ RbPlus
    {   2,   39,   53,    3,    0, } , // 16 pipes (16 PKRs) 4 bpe @ SW_64K_D_T @ RbPlus
    {   2,    6,   54,    4,    0, } , // 16 pipes (16 PKRs) 8 bpe @ SW_64K_D_T @ RbPlus
    {   2,    7,   55,    5,    0, } , // 16 pipes (16 PKRs) 16 bpe @ SW_64K_D_T @ RbPlus
    {   2,    5,   56,   16,    0, } , // 32 pipes (16 PKRs) 1 bpe @ SW_64K_D_T @ RbPlus
    {   2,    1,   57,   17,    0, } , // 32 pipes (16 PKRs) 2 bpe @ SW_64K_D_T @ RbPlus
    {   2,   39,   58,   18,    0, } , // 32 pipes (16 PKRs) 4 bpe @ SW_64K_D_T @ RbPlus
    {   2,    6,   59,   19,    0, } , // 32 pipes (16 PKRs) 8 bpe @ SW_64K_D_T @ RbPlus
    {   2,    7,   60,   20,    0, } , // 32 pipes (16 PKRs) 16 bpe @ SW_64K_D_T @ RbPlus
    {   2,    5,    1,   21,    0, } , // 64 pipes (16 PKRs) 1 bpe @ SW_64K_D_T @ RbPlus
    {   2,    1,    2,   22,    0, } , // 64 pipes (16 PKRs) 2 bpe @ SW_64K_D_T @ RbPlus
    {   2,   39,    3,   23,    0, } , // 64 pipes (16 PKRs) 4 bpe @ SW_64K_D_T @ RbPlus
    {   2,    6,    4,   24,    0, } , // 64 pipes (16 PKRs) 8 bpe @ SW_64K_D_T @ RbPlus
    {   2,    7,    5,   25,    0, } , // 64 pipes (16 PKRs) 16 bpe @ SW_64K_D_T @ RbPlus
    {   2,    5,   56,   16,    0, } , // 32 pipes (32 PKRs) 1 bpe @ SW_64K_D_T @ RbPlus
    {   2,    1,   57,   17,    0, } , // 32 pipes (32 PKRs) 2 bpe @ SW_64K_D_T @ RbPlus
    {   2,   39,   58,   18,    0, } , // 32 pipes (32 PKRs) 4 bpe @ SW_64K_D_T @ RbPlus
    {   2,    6,   59,   19,    0, } , // 32 pipes (32 PKRs) 8 bpe @ SW_64K_D_T @ RbPlus
    {   2,    7,   60,   20,    0, } , // 32 pipes (32 PKRs) 16 bpe @ SW_64K_D_T @ RbPlus
    {   2,    5,    1,   21,    0, } , // 64 pipes (32 PKRs) 1 bpe @ SW_64K_D_T @ RbPlus
    {   2,    1,    2,   22,    0, } , // 64 pipes (32 PKRs) 2 bpe @ SW_64K_D_T @ RbPlus
    {   2,   39,    3,   23,    0, } , // 64 pipes (32 PKRs) 4 bpe @ SW_64K_D_T @ RbPlus
    {   2,    6,    4,   24,    0, } , // 64 pipes (32 PKRs) 8 bpe @ SW_64K_D_T @ RbPlus
    {   2,    7,    5,   25,    0, } , // 64 pipes (32 PKRs) 16 bpe @ SW_64K_D_T @ RbPlus
};

const ADDR_SW_PATINFO GFX10_SW_64K_S_X_RBPLUS_PATINFO[] =
{
    {   1,    0,    1,    1,    0, } , // 1 pipes (1 PKRs) 1 bpe @ SW_64K_S_X @ RbPlus
    {   1,    1,    2,    2,    0, } , // 1 pipes (1 PKRs) 2 bpe @ SW_64K_S_X @ RbPlus
    {   1,    2,    3,    3,    0, } , // 1 pipes (1 PKRs) 4 bpe @ SW_64K_S_X @ RbPlus
    {   1,    3,    4,    4,    0, } , // 1 pipes (1 PKRs) 8 bpe @ SW_64K_S_X @ RbPlus
    {   1,    4,    5,    5,    0, } , // 1 pipes (1 PKRs) 16 bpe @ SW_64K_S_X @ RbPlus
    {   3,    0,    6,    1,    0, } , // 2 pipes (1-2 PKRs) 1 bpe @ SW_64K_S_X @ RbPlus
    {   3,    1,    7,    2,    0, } , // 2 pipes (1-2 PKRs) 2 bpe @ SW_64K_S_X @ RbPlus
    {   3,    2,    8,    3,    0, } , // 2 pipes (1-2 PKRs) 4 bpe @ SW_64K_S_X @ RbPlus
    {   3,    3,    9,    4,    0, } , // 2 pipes (1-2 PKRs) 8 bpe @ SW_64K_S_X @ RbPlus
    {   3,    4,   10,    5,    0, } , // 2 pipes (1-2 PKRs) 16 bpe @ SW_64K_S_X @ RbPlus
    {   3,    0,  210,    1,    0, } , // 4 pipes (1-2 PKRs) 1 bpe @ SW_64K_S_X @ RbPlus
    {   3,    1,  211,    2,    0, } , // 4 pipes (1-2 PKRs) 2 bpe @ SW_64K_S_X @ RbPlus
    {   3,    2,  212,    3,    0, } , // 4 pipes (1-2 PKRs) 4 bpe @ SW_64K_S_X @ RbPlus
    {   3,    3,  213,    4,    0, } , // 4 pipes (1-2 PKRs) 8 bpe @ SW_64K_S_X @ RbPlus
    {   3,    4,  214,    5,    0, } , // 4 pipes (1-2 PKRs) 16 bpe @ SW_64K_S_X @ RbPlus
    {   3,    0,  215,    1,    0, } , // 8 pipes (2 PKRs) 1 bpe @ SW_64K_S_X @ RbPlus
    {   3,    1,  216,    2,    0, } , // 8 pipes (2 PKRs) 2 bpe @ SW_64K_S_X @ RbPlus
    {   3,    2,  217,    3,    0, } , // 8 pipes (2 PKRs) 4 bpe @ SW_64K_S_X @ RbPlus
    {   3,    3,  218,    4,    0, } , // 8 pipes (2 PKRs) 8 bpe @ SW_64K_S_X @ RbPlus
    {   3,    4,  219,    5,    0, } , // 8 pipes (2 PKRs) 16 bpe @ SW_64K_S_X @ RbPlus
    {   3,    0,   11,    1,    0, } , // 4 pipes (4 PKRs) 1 bpe @ SW_64K_S_X @ RbPlus
    {   3,    1,   12,    2,    0, } , // 4 pipes (4 PKRs) 2 bpe @ SW_64K_S_X @ RbPlus
    {   3,    2,   13,    3,    0, } , // 4 pipes (4 PKRs) 4 bpe @ SW_64K_S_X @ RbPlus
    {   3,    3,   14,    4,    0, } , // 4 pipes (4 PKRs) 8 bpe @ SW_64K_S_X @ RbPlus
    {   3,    4,   15,    5,    0, } , // 4 pipes (4 PKRs) 16 bpe @ SW_64K_S_X @ RbPlus
    {   3,    0,  220,    1,    0, } , // 8 pipes (4 PKRs) 1 bpe @ SW_64K_S_X @ RbPlus
    {   3,    1,  221,    2,    0, } , // 8 pipes (4 PKRs) 2 bpe @ SW_64K_S_X @ RbPlus
    {   3,    2,  222,    3,    0, } , // 8 pipes (4 PKRs) 4 bpe @ SW_64K_S_X @ RbPlus
    {   3,    3,  223,    4,    0, } , // 8 pipes (4 PKRs) 8 bpe @ SW_64K_S_X @ RbPlus
    {   3,    4,  224,    5,    0, } , // 8 pipes (4 PKRs) 16 bpe @ SW_64K_S_X @ RbPlus
    {   3,    0,  225,    1,    0, } , // 16 pipes (4 PKRs) 1 bpe @ SW_64K_S_X @ RbPlus
    {   3,    1,  226,    2,    0, } , // 16 pipes (4 PKRs) 2 bpe @ SW_64K_S_X @ RbPlus
    {   3,    2,  227,    3,    0, } , // 16 pipes (4 PKRs) 4 bpe @ SW_64K_S_X @ RbPlus
    {   3,    3,  228,    4,    0, } , // 16 pipes (4 PKRs) 8 bpe @ SW_64K_S_X @ RbPlus
    {   3,    4,  229,    5,    0, } , // 16 pipes (4 PKRs) 16 bpe @ SW_64K_S_X @ RbPlus
    {   3,    0,   16,    1,    0, } , // 8 pipes (8 PKRs) 1 bpe @ SW_64K_S_X @ RbPlus
    {   3,    1,   17,    2,    0, } , // 8 pipes (8 PKRs) 2 bpe @ SW_64K_S_X @ RbPlus
    {   3,    2,   18,    3,    0, } , // 8 pipes (8 PKRs) 4 bpe @ SW_64K_S_X @ RbPlus
    {   3,    3,   19,    4,    0, } , // 8 pipes (8 PKRs) 8 bpe @ SW_64K_S_X @ RbPlus
    {   3,    4,   20,    5,    0, } , // 8 pipes (8 PKRs) 16 bpe @ SW_64K_S_X @ RbPlus
    {   3,    0,  230,    1,    0, } , // 16 pipes (8 PKRs) 1 bpe @ SW_64K_S_X @ RbPlus
    {   3,    1,  231,    2,    0, } , // 16 pipes (8 PKRs) 2 bpe @ SW_64K_S_X @ RbPlus
    {   3,    2,  232,    3,    0, } , // 16 pipes (8 PKRs) 4 bpe @ SW_64K_S_X @ RbPlus
    {   3,    3,  233,    4,    0, } , // 16 pipes (8 PKRs) 8 bpe @ SW_64K_S_X @ RbPlus
    {   3,    4,  234,    5,    0, } , // 16 pipes (8 PKRs) 16 bpe @ SW_64K_S_X @ RbPlus
    {   3,    0,  250,    6,    0, } , // 32 pipes (8 PKRs) 1 bpe @ SW_64K_S_X @ RbPlus
    {   3,    1,  251,    7,    0, } , // 32 pipes (8 PKRs) 2 bpe @ SW_64K_S_X @ RbPlus
    {   3,    2,  252,    8,    0, } , // 32 pipes (8 PKRs) 4 bpe @ SW_64K_S_X @ RbPlus
    {   3,    3,  253,    9,    0, } , // 32 pipes (8 PKRs) 8 bpe @ SW_64K_S_X @ RbPlus
    {   3,    4,  254,   10,    0, } , // 32 pipes (8 PKRs) 16 bpe @ SW_64K_S_X @ RbPlus
    {   3,    0,   21,    1,    0, } , // 16 pipes (16 PKRs) 1 bpe @ SW_64K_S_X @ RbPlus
    {   3,    1,   22,    2,    0, } , // 16 pipes (16 PKRs) 2 bpe @ SW_64K_S_X @ RbPlus
    {   3,    2,   23,    3,    0, } , // 16 pipes (16 PKRs) 4 bpe @ SW_64K_S_X @ RbPlus
    {   3,    3,   24,    4,    0, } , // 16 pipes (16 PKRs) 8 bpe @ SW_64K_S_X @ RbPlus
    {   3,    4,   25,    5,    0, } , // 16 pipes (16 PKRs) 16 bpe @ SW_64K_S_X @ RbPlus
    {   3,    0,  255,    6,    0, } , // 32 pipes (16 PKRs) 1 bpe @ SW_64K_S_X @ RbPlus
    {   3,    1,  256,    7,    0, } , // 32 pipes (16 PKRs) 2 bpe @ SW_64K_S_X @ RbPlus
    {   3,    2,  257,    8,    0, } , // 32 pipes (16 PKRs) 4 bpe @ SW_64K_S_X @ RbPlus
    {   3,    3,  258,    9,    0, } , // 32 pipes (16 PKRs) 8 bpe @ SW_64K_S_X @ RbPlus
    {   3,    4,  259,   10,    0, } , // 32 pipes (16 PKRs) 16 bpe @ SW_64K_S_X @ RbPlus
    {   3,    0,  260,   11,    0, } , // 64 pipes (16 PKRs) 1 bpe @ SW_64K_S_X @ RbPlus
    {   3,    1,  261,   12,    0, } , // 64 pipes (16 PKRs) 2 bpe @ SW_64K_S_X @ RbPlus
    {   3,    2,  262,   13,    0, } , // 64 pipes (16 PKRs) 4 bpe @ SW_64K_S_X @ RbPlus
    {   3,    3,  263,   14,    0, } , // 64 pipes (16 PKRs) 8 bpe @ SW_64K_S_X @ RbPlus
    {   3,    4,  264,   15,    0, } , // 64 pipes (16 PKRs) 16 bpe @ SW_64K_S_X @ RbPlus
    {   3,    0,   26,    6,    0, } , // 32 pipes (32 PKRs) 1 bpe @ SW_64K_S_X @ RbPlus
    {   3,    1,   27,    7,    0, } , // 32 pipes (32 PKRs) 2 bpe @ SW_64K_S_X @ RbPlus
    {   3,    2,   28,    8,    0, } , // 32 pipes (32 PKRs) 4 bpe @ SW_64K_S_X @ RbPlus
    {   3,    3,   29,    9,    0, } , // 32 pipes (32 PKRs) 8 bpe @ SW_64K_S_X @ RbPlus
    {   3,    4,   30,   10,    0, } , // 32 pipes (32 PKRs) 16 bpe @ SW_64K_S_X @ RbPlus
    {   3,    0,  265,   11,    0, } , // 64 pipes (32 PKRs) 1 bpe @ SW_64K_S_X @ RbPlus
    {   3,    1,  266,   12,    0, } , // 64 pipes (32 PKRs) 2 bpe @ SW_64K_S_X @ RbPlus
    {   3,    2,  267,   13,    0, } , // 64 pipes (32 PKRs) 4 bpe @ SW_64K_S_X @ RbPlus
    {   3,    3,  268,   14,    0, } , // 64 pipes (32 PKRs) 8 bpe @ SW_64K_S_X @ RbPlus
    {   3,    4,  269,   15,    0, } , // 64 pipes (32 PKRs) 16 bpe @ SW_64K_S_X @ RbPlus
};

const ADDR_SW_PATINFO GFX10_SW_64K_D_X_RBPLUS_PATINFO[] =
{
    {   1,    5,    1,    1,    0, } , // 1 pipes (1 PKRs) 1 bpe @ SW_64K_D_X @ RbPlus
    {   1,    1,    2,    2,    0, } , // 1 pipes (1 PKRs) 2 bpe @ SW_64K_D_X @ RbPlus
    {   1,   39,    3,    3,    0, } , // 1 pipes (1 PKRs) 4 bpe @ SW_64K_D_X @ RbPlus
    {   1,    6,    4,    4,    0, } , // 1 pipes (1 PKRs) 8 bpe @ SW_64K_D_X @ RbPlus
    {   1,    7,    5,    5,    0, } , // 1 pipes (1 PKRs) 16 bpe @ SW_64K_D_X @ RbPlus
    {   3,    5,    6,    1,    0, } , // 2 pipes (1-2 PKRs) 1 bpe @ SW_64K_D_X @ RbPlus
    {   3,    1,    7,    2,    0, } , // 2 pipes (1-2 PKRs) 2 bpe @ SW_64K_D_X @ RbPlus
    {   3,   39,    8,    3,    0, } , // 2 pipes (1-2 PKRs) 4 bpe @ SW_64K_D_X @ RbPlus
    {   3,    6,    9,    4,    0, } , // 2 pipes (1-2 PKRs) 8 bpe @ SW_64K_D_X @ RbPlus
    {   3,    7,   10,    5,    0, } , // 2 pipes (1-2 PKRs) 16 bpe @ SW_64K_D_X @ RbPlus
    {   3,    5,  210,    1,    0, } , // 4 pipes (1-2 PKRs) 1 bpe @ SW_64K_D_X @ RbPlus
    {   3,    1,  211,    2,    0, } , // 4 pipes (1-2 PKRs) 2 bpe @ SW_64K_D_X @ RbPlus
    {   3,   39,  212,    3,    0, } , // 4 pipes (1-2 PKRs) 4 bpe @ SW_64K_D_X @ RbPlus
    {   3,    6,  213,    4,    0, } , // 4 pipes (1-2 PKRs) 8 bpe @ SW_64K_D_X @ RbPlus
    {   3,    7,  214,    5,    0, } , // 4 pipes (1-2 PKRs) 16 bpe @ SW_64K_D_X @ RbPlus
    {   3,    5,  215,    1,    0, } , // 8 pipes (2 PKRs) 1 bpe @ SW_64K_D_X @ RbPlus
    {   3,    1,  216,    2,    0, } , // 8 pipes (2 PKRs) 2 bpe @ SW_64K_D_X @ RbPlus
    {   3,   39,  217,    3,    0, } , // 8 pipes (2 PKRs) 4 bpe @ SW_64K_D_X @ RbPlus
    {   3,    6,  218,    4,    0, } , // 8 pipes (2 PKRs) 8 bpe @ SW_64K_D_X @ RbPlus
    {   3,    7,  219,    5,    0, } , // 8 pipes (2 PKRs) 16 bpe @ SW_64K_D_X @ RbPlus
    {   3,    5,   11,    1,    0, } , // 4 pipes (4 PKRs) 1 bpe @ SW_64K_D_X @ RbPlus
    {   3,    1,   12,    2,    0, } , // 4 pipes (4 PKRs) 2 bpe @ SW_64K_D_X @ RbPlus
    {   3,   39,   13,    3,    0, } , // 4 pipes (4 PKRs) 4 bpe @ SW_64K_D_X @ RbPlus
    {   3,    6,   14,    4,    0, } , // 4 pipes (4 PKRs) 8 bpe @ SW_64K_D_X @ RbPlus
    {   3,    7,   15,    5,    0, } , // 4 pipes (4 PKRs) 16 bpe @ SW_64K_D_X @ RbPlus
    {   3,    5,  220,    1,    0, } , // 8 pipes (4 PKRs) 1 bpe @ SW_64K_D_X @ RbPlus
    {   3,    1,  221,    2,    0, } , // 8 pipes (4 PKRs) 2 bpe @ SW_64K_D_X @ RbPlus
    {   3,   39,  222,    3,    0, } , // 8 pipes (4 PKRs) 4 bpe @ SW_64K_D_X @ RbPlus
    {   3,    6,  223,    4,    0, } , // 8 pipes (4 PKRs) 8 bpe @ SW_64K_D_X @ RbPlus
    {   3,    7,  224,    5,    0, } , // 8 pipes (4 PKRs) 16 bpe @ SW_64K_D_X @ RbPlus
    {   3,    5,  225,    1,    0, } , // 16 pipes (4 PKRs) 1 bpe @ SW_64K_D_X @ RbPlus
    {   3,    1,  226,    2,    0, } , // 16 pipes (4 PKRs) 2 bpe @ SW_64K_D_X @ RbPlus
    {   3,   39,  227,    3,    0, } , // 16 pipes (4 PKRs) 4 bpe @ SW_64K_D_X @ RbPlus
    {   3,    6,  228,    4,    0, } , // 16 pipes (4 PKRs) 8 bpe @ SW_64K_D_X @ RbPlus
    {   3,    7,  229,    5,    0, } , // 16 pipes (4 PKRs) 16 bpe @ SW_64K_D_X @ RbPlus
    {   3,    5,   16,    1,    0, } , // 8 pipes (8 PKRs) 1 bpe @ SW_64K_D_X @ RbPlus
    {   3,    1,   17,    2,    0, } , // 8 pipes (8 PKRs) 2 bpe @ SW_64K_D_X @ RbPlus
    {   3,   39,   18,    3,    0, } , // 8 pipes (8 PKRs) 4 bpe @ SW_64K_D_X @ RbPlus
    {   3,    6,   19,    4,    0, } , // 8 pipes (8 PKRs) 8 bpe @ SW_64K_D_X @ RbPlus
    {   3,    7,   20,    5,    0, } , // 8 pipes (8 PKRs) 16 bpe @ SW_64K_D_X @ RbPlus
    {   3,    5,  230,    1,    0, } , // 16 pipes (8 PKRs) 1 bpe @ SW_64K_D_X @ RbPlus
    {   3,    1,  231,    2,    0, } , // 16 pipes (8 PKRs) 2 bpe @ SW_64K_D_X @ RbPlus
    {   3,   39,  232,    3,    0, } , // 16 pipes (8 PKRs) 4 bpe @ SW_64K_D_X @ RbPlus
    {   3,    6,  233,    4,    0, } , // 16 pipes (8 PKRs) 8 bpe @ SW_64K_D_X @ RbPlus
    {   3,    7,  234,    5,    0, } , // 16 pipes (8 PKRs) 16 bpe @ SW_64K_D_X @ RbPlus
    {   3,    5,  250,    6,    0, } , // 32 pipes (8 PKRs) 1 bpe @ SW_64K_D_X @ RbPlus
    {   3,    1,  251,    7,    0, } , // 32 pipes (8 PKRs) 2 bpe @ SW_64K_D_X @ RbPlus
    {   3,   39,  252,    8,    0, } , // 32 pipes (8 PKRs) 4 bpe @ SW_64K_D_X @ RbPlus
    {   3,    6,  253,    9,    0, } , // 32 pipes (8 PKRs) 8 bpe @ SW_64K_D_X @ RbPlus
    {   3,    7,  254,   10,    0, } , // 32 pipes (8 PKRs) 16 bpe @ SW_64K_D_X @ RbPlus
    {   3,    5,   21,    1,    0, } , // 16 pipes (16 PKRs) 1 bpe @ SW_64K_D_X @ RbPlus
    {   3,    1,   22,    2,    0, } , // 16 pipes (16 PKRs) 2 bpe @ SW_64K_D_X @ RbPlus
    {   3,   39,   23,    3,    0, } , // 16 pipes (16 PKRs) 4 bpe @ SW_64K_D_X @ RbPlus
    {   3,    6,   24,    4,    0, } , // 16 pipes (16 PKRs) 8 bpe @ SW_64K_D_X @ RbPlus
    {   3,    7,   25,    5,    0, } , // 16 pipes (16 PKRs) 16 bpe @ SW_64K_D_X @ RbPlus
    {   3,    5,  255,    6,    0, } , // 32 pipes (16 PKRs) 1 bpe @ SW_64K_D_X @ RbPlus
    {   3,    1,  256,    7,    0, } , // 32 pipes (16 PKRs) 2 bpe @ SW_64K_D_X @ RbPlus
    {   3,   39,  257,    8,    0, } , // 32 pipes (16 PKRs) 4 bpe @ SW_64K_D_X @ RbPlus
    {   3,    6,  258,    9,    0, } , // 32 pipes (16 PKRs) 8 bpe @ SW_64K_D_X @ RbPlus
    {   3,    7,  259,   10,    0, } , // 32 pipes (16 PKRs) 16 bpe @ SW_64K_D_X @ RbPlus
    {   3,    5,  260,   11,    0, } , // 64 pipes (16 PKRs) 1 bpe @ SW_64K_D_X @ RbPlus
    {   3,    1,  261,   12,    0, } , // 64 pipes (16 PKRs) 2 bpe @ SW_64K_D_X @ RbPlus
    {   3,   39,  262,   13,    0, } , // 64 pipes (16 PKRs) 4 bpe @ SW_64K_D_X @ RbPlus
    {   3,    6,  263,   14,    0, } , // 64 pipes (16 PKRs) 8 bpe @ SW_64K_D_X @ RbPlus
    {   3,    7,  264,   15,    0, } , // 64 pipes (16 PKRs) 16 bpe @ SW_64K_D_X @ RbPlus
    {   3,    5,   26,    6,    0, } , // 32 pipes (32 PKRs) 1 bpe @ SW_64K_D_X @ RbPlus
    {   3,    1,   27,    7,    0, } , // 32 pipes (32 PKRs) 2 bpe @ SW_64K_D_X @ RbPlus
    {   3,   39,   28,    8,    0, } , // 32 pipes (32 PKRs) 4 bpe @ SW_64K_D_X @ RbPlus
    {   3,    6,   29,    9,    0, } , // 32 pipes (32 PKRs) 8 bpe @ SW_64K_D_X @ RbPlus
    {   3,    7,   30,   10,    0, } , // 32 pipes (32 PKRs) 16 bpe @ SW_64K_D_X @ RbPlus
    {   3,    5,  265,   11,    0, } , // 64 pipes (32 PKRs) 1 bpe @ SW_64K_D_X @ RbPlus
    {   3,    1,  266,   12,    0, } , // 64 pipes (32 PKRs) 2 bpe @ SW_64K_D_X @ RbPlus
    {   3,   39,  267,   13,    0, } , // 64 pipes (32 PKRs) 4 bpe @ SW_64K_D_X @ RbPlus
    {   3,    6,  268,   14,    0, } , // 64 pipes (32 PKRs) 8 bpe @ SW_64K_D_X @ RbPlus
    {   3,    7,  269,   15,    0, } , // 64 pipes (32 PKRs) 16 bpe @ SW_64K_D_X @ RbPlus
};

const ADDR_SW_PATINFO GFX10_SW_64K_R_X_1xaa_RBPLUS_PATINFO[] =
{
    {   2,    0,  347,  193,    0, } , // 1 pipes (1 PKRs) 1 bpe @ SW_64K_R_X 1xaa @ RbPlus
    {   2,    1,  348,  366,    0, } , // 1 pipes (1 PKRs) 2 bpe @ SW_64K_R_X 1xaa @ RbPlus
    {   2,   39,  349,  195,    0, } , // 1 pipes (1 PKRs) 4 bpe @ SW_64K_R_X 1xaa @ RbPlus
    {   2,    6,  350,  367,    0, } , // 1 pipes (1 PKRs) 8 bpe @ SW_64K_R_X 1xaa @ RbPlus
    {   2,    7,  351,  368,    0, } , // 1 pipes (1 PKRs) 16 bpe @ SW_64K_R_X 1xaa @ RbPlus
    {   3,    0,  352,  193,    0, } , // 2 pipes (1-2 PKRs) 1 bpe @ SW_64K_R_X 1xaa @ RbPlus
    {   3,    1,  353,  194,    0, } , // 2 pipes (1-2 PKRs) 2 bpe @ SW_64K_R_X 1xaa @ RbPlus
    {   3,   39,  354,  195,    0, } , // 2 pipes (1-2 PKRs) 4 bpe @ SW_64K_R_X 1xaa @ RbPlus
    {   3,    6,  355,  369,    0, } , // 2 pipes (1-2 PKRs) 8 bpe @ SW_64K_R_X 1xaa @ RbPlus
    {   3,    7,  356,  370,    0, } , // 2 pipes (1-2 PKRs) 16 bpe @ SW_64K_R_X 1xaa @ RbPlus
    {   3,    0,  280,  193,    0, } , // 4 pipes (1-2 PKRs) 1 bpe @ SW_64K_R_X 1xaa @ RbPlus
    {   3,    1,  281,  194,    0, } , // 4 pipes (1-2 PKRs) 2 bpe @ SW_64K_R_X 1xaa @ RbPlus
    {   3,   39,  282,  195,    0, } , // 4 pipes (1-2 PKRs) 4 bpe @ SW_64K_R_X 1xaa @ RbPlus
    {   3,    6,  283,  196,    0, } , // 4 pipes (1-2 PKRs) 8 bpe @ SW_64K_R_X 1xaa @ RbPlus
    {   3,    7,  284,  197,    0, } , // 4 pipes (1-2 PKRs) 16 bpe @ SW_64K_R_X 1xaa @ RbPlus
    {   3,    0,  394,  219,    0, } , // 8 pipes (2 PKRs) 1 bpe @ SW_64K_R_X 1xaa @ RbPlus
    {   3,    1,  395,  371,    0, } , // 8 pipes (2 PKRs) 2 bpe @ SW_64K_R_X 1xaa @ RbPlus
    {   3,   39,  396,  372,    0, } , // 8 pipes (2 PKRs) 4 bpe @ SW_64K_R_X 1xaa @ RbPlus
    {   3,    6,  397,  373,    0, } , // 8 pipes (2 PKRs) 8 bpe @ SW_64K_R_X 1xaa @ RbPlus
    {   3,    7,  398,  374,    0, } , // 8 pipes (2 PKRs) 16 bpe @ SW_64K_R_X 1xaa @ RbPlus
    {   3,    0,  290,  203,    0, } , // 4 pipes (4 PKRs) 1 bpe @ SW_64K_R_X 1xaa @ RbPlus
    {   3,    1,  291,  204,    0, } , // 4 pipes (4 PKRs) 2 bpe @ SW_64K_R_X 1xaa @ RbPlus
    {   3,   39,  292,  205,    0, } , // 4 pipes (4 PKRs) 4 bpe @ SW_64K_R_X 1xaa @ RbPlus
    {   3,    6,  293,  206,    0, } , // 4 pipes (4 PKRs) 8 bpe @ SW_64K_R_X 1xaa @ RbPlus
    {   3,    7,  294,  207,    0, } , // 4 pipes (4 PKRs) 16 bpe @ SW_64K_R_X 1xaa @ RbPlus
    {   3,    0,  295,  219,    0, } , // 8 pipes (4 PKRs) 1 bpe @ SW_64K_R_X 1xaa @ RbPlus
    {   3,    1,  296,  375,    0, } , // 8 pipes (4 PKRs) 2 bpe @ SW_64K_R_X 1xaa @ RbPlus
    {   3,   39,  297,  376,    0, } , // 8 pipes (4 PKRs) 4 bpe @ SW_64K_R_X 1xaa @ RbPlus
    {   3,    6,  298,  377,    0, } , // 8 pipes (4 PKRs) 8 bpe @ SW_64K_R_X 1xaa @ RbPlus
    {   3,    7,  299,  378,    0, } , // 8 pipes (4 PKRs) 16 bpe @ SW_64K_R_X 1xaa @ RbPlus
    {   3,    0,  399,  379,    0, } , // 16 pipes (4 PKRs) 1 bpe @ SW_64K_R_X 1xaa @ RbPlus
    {   3,    1,  399,  380,    0, } , // 16 pipes (4 PKRs) 2 bpe @ SW_64K_R_X 1xaa @ RbPlus
    {   3,   39,  399,  381,    0, } , // 16 pipes (4 PKRs) 4 bpe @ SW_64K_R_X 1xaa @ RbPlus
    {   3,    6,  399,  382,    0, } , // 16 pipes (4 PKRs) 8 bpe @ SW_64K_R_X 1xaa @ RbPlus
    {   3,    7,  399,  383,    0, } , // 16 pipes (4 PKRs) 16 bpe @ SW_64K_R_X 1xaa @ RbPlus
    {   3,    0,  400,  669,    0, } , // 8 pipes (8 PKRs) 1 bpe @ SW_64K_R_X 1xaa @ RbPlus
    {   3,    1,  401,  670,    0, } , // 8 pipes (8 PKRs) 2 bpe @ SW_64K_R_X 1xaa @ RbPlus
    {   3,   39,  402,  671,    0, } , // 8 pipes (8 PKRs) 4 bpe @ SW_64K_R_X 1xaa @ RbPlus
    {   3,    6,  304,  387,    0, } , // 8 pipes (8 PKRs) 8 bpe @ SW_64K_R_X 1xaa @ RbPlus
    {   3,    7,  305,  388,    0, } , // 8 pipes (8 PKRs) 16 bpe @ SW_64K_R_X 1xaa @ RbPlus
    {   3,    0,  307,  379,    0, } , // 16 pipes (8 PKRs) 1 bpe @ SW_64K_R_X 1xaa @ RbPlus
    {   3,    1,  307,  389,    0, } , // 16 pipes (8 PKRs) 2 bpe @ SW_64K_R_X 1xaa @ RbPlus
    {   3,   39,  307,  381,    0, } , // 16 pipes (8 PKRs) 4 bpe @ SW_64K_R_X 1xaa @ RbPlus
    {   3,    6,  307,  382,    0, } , // 16 pipes (8 PKRs) 8 bpe @ SW_64K_R_X 1xaa @ RbPlus
    {   3,    7,  307,  390,    0, } , // 16 pipes (8 PKRs) 16 bpe @ SW_64K_R_X 1xaa @ RbPlus
    {   3,    0,  307,  672,    0, } , // 32 pipes (8 PKRs) 1 bpe @ SW_64K_R_X 1xaa @ RbPlus
    {   3,    1,  307,  673,    0, } , // 32 pipes (8 PKRs) 2 bpe @ SW_64K_R_X 1xaa @ RbPlus
    {   3,   39,  307,  674,    0, } , // 32 pipes (8 PKRs) 4 bpe @ SW_64K_R_X 1xaa @ RbPlus
    {   3,    6,  307,  675,    0, } , // 32 pipes (8 PKRs) 8 bpe @ SW_64K_R_X 1xaa @ RbPlus
    {   3,    7,  307,  676,    0, } , // 32 pipes (8 PKRs) 16 bpe @ SW_64K_R_X 1xaa @ RbPlus
    {   3,    0,  309,  677,    0, } , // 16 pipes (16 PKRs) 1 bpe @ SW_64K_R_X 1xaa @ RbPlus
    {   3,    1,  309,  678,    0, } , // 16 pipes (16 PKRs) 2 bpe @ SW_64K_R_X 1xaa @ RbPlus
    {   3,   39,  309,  679,    0, } , // 16 pipes (16 PKRs) 4 bpe @ SW_64K_R_X 1xaa @ RbPlus
    {   3,    6,  309,  399,    0, } , // 16 pipes (16 PKRs) 8 bpe @ SW_64K_R_X 1xaa @ RbPlus
    {   3,    7,  323,  400,    0, } , // 16 pipes (16 PKRs) 16 bpe @ SW_64K_R_X 1xaa @ RbPlus
    {   3,    0,  309,  680,    0, } , // 32 pipes (16 PKRs) 1 bpe @ SW_64K_R_X 1xaa @ RbPlus
    {   3,    1,  309,  681,    0, } , // 32 pipes (16 PKRs) 2 bpe @ SW_64K_R_X 1xaa @ RbPlus
    {   3,   39,  309,  682,    0, } , // 32 pipes (16 PKRs) 4 bpe @ SW_64K_R_X 1xaa @ RbPlus
    {   3,    6,  309,  404,    0, } , // 32 pipes (16 PKRs) 8 bpe @ SW_64K_R_X 1xaa @ RbPlus
    {   3,    7,  323,  405,    0, } , // 32 pipes (16 PKRs) 16 bpe @ SW_64K_R_X 1xaa @ RbPlus
    {   3,    0,  309,  505,    0, } , // 64 pipes (16 PKRs) 1 bpe @ SW_64K_R_X 1xaa @ RbPlus
    {   3,    1,  309,  506,    0, } , // 64 pipes (16 PKRs) 2 bpe @ SW_64K_R_X 1xaa @ RbPlus
    {   3,   39,  309,  507,    0, } , // 64 pipes (16 PKRs) 4 bpe @ SW_64K_R_X 1xaa @ RbPlus
    {   3,    6,  309,  683,    0, } , // 64 pipes (16 PKRs) 8 bpe @ SW_64K_R_X 1xaa @ RbPlus
    {   3,    7,  323,  684,    0, } , // 64 pipes (16 PKRs) 16 bpe @ SW_64K_R_X 1xaa @ RbPlus
    {   3,    0,  311,  685,    0, } , // 32 pipes (32 PKRs) 1 bpe @ SW_64K_R_X 1xaa @ RbPlus
    {   3,    1,  311,  686,    0, } , // 32 pipes (32 PKRs) 2 bpe @ SW_64K_R_X 1xaa @ RbPlus
    {   3,   39,  311,  687,    0, } , // 32 pipes (32 PKRs) 4 bpe @ SW_64K_R_X 1xaa @ RbPlus
    {   3,    6,  318,  411,    0, } , // 32 pipes (32 PKRs) 8 bpe @ SW_64K_R_X 1xaa @ RbPlus
    {   3,    7,  324,  412,    0, } , // 32 pipes (32 PKRs) 16 bpe @ SW_64K_R_X 1xaa @ RbPlus
    {   3,    0,  311,  513,    0, } , // 64 pipes (32 PKRs) 1 bpe @ SW_64K_R_X 1xaa @ RbPlus
    {   3,    1,  311,  514,    0, } , // 64 pipes (32 PKRs) 2 bpe @ SW_64K_R_X 1xaa @ RbPlus
    {   3,   39,  311,  515,    0, } , // 64 pipes (32 PKRs) 4 bpe @ SW_64K_R_X 1xaa @ RbPlus
    {   3,    6,  318,  413,    0, } , // 64 pipes (32 PKRs) 8 bpe @ SW_64K_R_X 1xaa @ RbPlus
    {   3,    7,  324,  414,    0, } , // 64 pipes (32 PKRs) 16 bpe @ SW_64K_R_X 1xaa @ RbPlus
};

const ADDR_SW_PATINFO GFX10_SW_64K_R_X_2xaa_RBPLUS_PATINFO[] =
{
    {   3,    0,  424,  526,    0, } , // 1 pipes (1 PKRs) 1 bpe @ SW_64K_R_X 2xaa @ RbPlus
    {   3,    1,  348,  527,    0, } , // 1 pipes (1 PKRs) 2 bpe @ SW_64K_R_X 2xaa @ RbPlus
    {   3,   39,  358,  528,    0, } , // 1 pipes (1 PKRs) 4 bpe @ SW_64K_R_X 2xaa @ RbPlus
    {   3,    6,  350,  688,    0, } , // 1 pipes (1 PKRs) 8 bpe @ SW_64K_R_X 2xaa @ RbPlus
    {   3,    7,  359,  689,    0, } , // 1 pipes (1 PKRs) 16 bpe @ SW_64K_R_X 2xaa @ RbPlus
    {   3,    0,  352,  526,    0, } , // 2 pipes (1-2 PKRs) 1 bpe @ SW_64K_R_X 2xaa @ RbPlus
    {   3,    1,  353,  527,    0, } , // 2 pipes (1-2 PKRs) 2 bpe @ SW_64K_R_X 2xaa @ RbPlus
    {   3,   39,  354,  528,    0, } , // 2 pipes (1-2 PKRs) 4 bpe @ SW_64K_R_X 2xaa @ RbPlus
    {   3,    6,  355,  688,    0, } , // 2 pipes (1-2 PKRs) 8 bpe @ SW_64K_R_X 2xaa @ RbPlus
    {   3,    7,  356,  690,    0, } , // 2 pipes (1-2 PKRs) 16 bpe @ SW_64K_R_X 2xaa @ RbPlus
    {   3,    0,  280,  526,    0, } , // 4 pipes (1-2 PKRs) 1 bpe @ SW_64K_R_X 2xaa @ RbPlus
    {   3,    1,  281,  527,    0, } , // 4 pipes (1-2 PKRs) 2 bpe @ SW_64K_R_X 2xaa @ RbPlus
    {   3,   39,  282,  528,    0, } , // 4 pipes (1-2 PKRs) 4 bpe @ SW_64K_R_X 2xaa @ RbPlus
    {   3,    6,  283,  529,    0, } , // 4 pipes (1-2 PKRs) 8 bpe @ SW_64K_R_X 2xaa @ RbPlus
    {   3,    7,  284,  530,    0, } , // 4 pipes (1-2 PKRs) 16 bpe @ SW_64K_R_X 2xaa @ RbPlus
    {   3,    0,  394,  691,    0, } , // 8 pipes (2 PKRs) 1 bpe @ SW_64K_R_X 2xaa @ RbPlus
    {   3,    1,  395,  692,    0, } , // 8 pipes (2 PKRs) 2 bpe @ SW_64K_R_X 2xaa @ RbPlus
    {   3,   39,  396,  693,    0, } , // 8 pipes (2 PKRs) 4 bpe @ SW_64K_R_X 2xaa @ RbPlus
    {   3,    6,  397,  694,    0, } , // 8 pipes (2 PKRs) 8 bpe @ SW_64K_R_X 2xaa @ RbPlus
    {   3,    7,  425,  695,    0, } , // 8 pipes (2 PKRs) 16 bpe @ SW_64K_R_X 2xaa @ RbPlus
    {   3,    0,  290,  534,    0, } , // 4 pipes (4 PKRs) 1 bpe @ SW_64K_R_X 2xaa @ RbPlus
    {   3,    1,  291,  535,    0, } , // 4 pipes (4 PKRs) 2 bpe @ SW_64K_R_X 2xaa @ RbPlus
    {   3,   39,  292,  536,    0, } , // 4 pipes (4 PKRs) 4 bpe @ SW_64K_R_X 2xaa @ RbPlus
    {   3,    6,  293,  537,    0, } , // 4 pipes (4 PKRs) 8 bpe @ SW_64K_R_X 2xaa @ RbPlus
    {   3,    7,  294,  538,    0, } , // 4 pipes (4 PKRs) 16 bpe @ SW_64K_R_X 2xaa @ RbPlus
    {   3,    0,  295,  691,    0, } , // 8 pipes (4 PKRs) 1 bpe @ SW_64K_R_X 2xaa @ RbPlus
    {   3,    1,  296,  696,    0, } , // 8 pipes (4 PKRs) 2 bpe @ SW_64K_R_X 2xaa @ RbPlus
    {   3,   39,  297,  697,    0, } , // 8 pipes (4 PKRs) 4 bpe @ SW_64K_R_X 2xaa @ RbPlus
    {   3,    6,  298,  698,    0, } , // 8 pipes (4 PKRs) 8 bpe @ SW_64K_R_X 2xaa @ RbPlus
    {   3,    7,  299,  699,    0, } , // 8 pipes (4 PKRs) 16 bpe @ SW_64K_R_X 2xaa @ RbPlus
    {   3,    0,  399,  700,    0, } , // 16 pipes (4 PKRs) 1 bpe @ SW_64K_R_X 2xaa @ RbPlus
    {   3,    1,  399,  701,    0, } , // 16 pipes (4 PKRs) 2 bpe @ SW_64K_R_X 2xaa @ RbPlus
    {   3,   39,  399,  702,    0, } , // 16 pipes (4 PKRs) 4 bpe @ SW_64K_R_X 2xaa @ RbPlus
    {   3,    6,  399,  703,    0, } , // 16 pipes (4 PKRs) 8 bpe @ SW_64K_R_X 2xaa @ RbPlus
    {   3,    7,  426,  429,    0, } , // 16 pipes (4 PKRs) 16 bpe @ SW_64K_R_X 2xaa @ RbPlus
    {   3,    0,  400,  704,    0, } , // 8 pipes (8 PKRs) 1 bpe @ SW_64K_R_X 2xaa @ RbPlus
    {   3,    1,  401,  705,    0, } , // 8 pipes (8 PKRs) 2 bpe @ SW_64K_R_X 2xaa @ RbPlus
    {   3,   39,  402,  706,    0, } , // 8 pipes (8 PKRs) 4 bpe @ SW_64K_R_X 2xaa @ RbPlus
    {   3,    6,  304,  707,    0, } , // 8 pipes (8 PKRs) 8 bpe @ SW_64K_R_X 2xaa @ RbPlus
    {   3,    7,  364,  708,    0, } , // 8 pipes (8 PKRs) 16 bpe @ SW_64K_R_X 2xaa @ RbPlus
    {   3,    0,  307,  700,    0, } , // 16 pipes (8 PKRs) 1 bpe @ SW_64K_R_X 2xaa @ RbPlus
    {   3,    1,  307,  701,    0, } , // 16 pipes (8 PKRs) 2 bpe @ SW_64K_R_X 2xaa @ RbPlus
    {   3,   39,  307,  702,    0, } , // 16 pipes (8 PKRs) 4 bpe @ SW_64K_R_X 2xaa @ RbPlus
    {   3,    6,  307,  703,    0, } , // 16 pipes (8 PKRs) 8 bpe @ SW_64K_R_X 2xaa @ RbPlus
    {   3,    7,  427,  390,    0, } , // 16 pipes (8 PKRs) 16 bpe @ SW_64K_R_X 2xaa @ RbPlus
    {   3,    0,  307,  709,    0, } , // 32 pipes (8 PKRs) 1 bpe @ SW_64K_R_X 2xaa @ RbPlus
    {   3,    1,  307,  710,    0, } , // 32 pipes (8 PKRs) 2 bpe @ SW_64K_R_X 2xaa @ RbPlus
    {   3,   39,  307,  711,    0, } , // 32 pipes (8 PKRs) 4 bpe @ SW_64K_R_X 2xaa @ RbPlus
    {   3,    6,  307,  712,    0, } , // 32 pipes (8 PKRs) 8 bpe @ SW_64K_R_X 2xaa @ RbPlus
    {   3,    7,  427,  676,    0, } , // 32 pipes (8 PKRs) 16 bpe @ SW_64K_R_X 2xaa @ RbPlus
    {   3,    0,  309,  713,    0, } , // 16 pipes (16 PKRs) 1 bpe @ SW_64K_R_X 2xaa @ RbPlus
    {   3,    1,  309,  714,    0, } , // 16 pipes (16 PKRs) 2 bpe @ SW_64K_R_X 2xaa @ RbPlus
    {   3,   39,  309,  715,    0, } , // 16 pipes (16 PKRs) 4 bpe @ SW_64K_R_X 2xaa @ RbPlus
    {   3,    6,  323,  716,    0, } , // 16 pipes (16 PKRs) 8 bpe @ SW_64K_R_X 2xaa @ RbPlus
    {   3,    7,  428,  400,    0, } , // 16 pipes (16 PKRs) 16 bpe @ SW_64K_R_X 2xaa @ RbPlus
    {   3,    0,  309,  717,    0, } , // 32 pipes (16 PKRs) 1 bpe @ SW_64K_R_X 2xaa @ RbPlus
    {   3,    1,  309,  718,    0, } , // 32 pipes (16 PKRs) 2 bpe @ SW_64K_R_X 2xaa @ RbPlus
    {   3,   39,  309,  719,    0, } , // 32 pipes (16 PKRs) 4 bpe @ SW_64K_R_X 2xaa @ RbPlus
    {   3,    6,  323,  720,    0, } , // 32 pipes (16 PKRs) 8 bpe @ SW_64K_R_X 2xaa @ RbPlus
    {   3,    7,  428,  405,    0, } , // 32 pipes (16 PKRs) 16 bpe @ SW_64K_R_X 2xaa @ RbPlus
    {   3,    0,  309,  721,    0, } , // 64 pipes (16 PKRs) 1 bpe @ SW_64K_R_X 2xaa @ RbPlus
    {   3,    1,  309,  722,    0, } , // 64 pipes (16 PKRs) 2 bpe @ SW_64K_R_X 2xaa @ RbPlus
    {   3,   39,  309,  723,    0, } , // 64 pipes (16 PKRs) 4 bpe @ SW_64K_R_X 2xaa @ RbPlus
    {   3,    6,  323,  724,    0, } , // 64 pipes (16 PKRs) 8 bpe @ SW_64K_R_X 2xaa @ RbPlus
    {   3,    7,  428,  684,    0, } , // 64 pipes (16 PKRs) 16 bpe @ SW_64K_R_X 2xaa @ RbPlus
    {   3,    0,  318,  725,    0, } , // 32 pipes (32 PKRs) 1 bpe @ SW_64K_R_X 2xaa @ RbPlus
    {   3,    1,  318,  726,    0, } , // 32 pipes (32 PKRs) 2 bpe @ SW_64K_R_X 2xaa @ RbPlus
    {   3,   39,  318,  727,    0, } , // 32 pipes (32 PKRs) 4 bpe @ SW_64K_R_X 2xaa @ RbPlus
    {   3,    6,  324,  728,    0, } , // 32 pipes (32 PKRs) 8 bpe @ SW_64K_R_X 2xaa @ RbPlus
    {   3,    7,  429,  412,    0, } , // 32 pipes (32 PKRs) 16 bpe @ SW_64K_R_X 2xaa @ RbPlus
    {   3,    0,  318,  729,    0, } , // 64 pipes (32 PKRs) 1 bpe @ SW_64K_R_X 2xaa @ RbPlus
    {   3,    1,  318,  730,    0, } , // 64 pipes (32 PKRs) 2 bpe @ SW_64K_R_X 2xaa @ RbPlus
    {   3,   39,  318,  731,    0, } , // 64 pipes (32 PKRs) 4 bpe @ SW_64K_R_X 2xaa @ RbPlus
    {   3,    6,  324,  732,    0, } , // 64 pipes (32 PKRs) 8 bpe @ SW_64K_R_X 2xaa @ RbPlus
    {   3,    7,  429,  414,    0, } , // 64 pipes (32 PKRs) 16 bpe @ SW_64K_R_X 2xaa @ RbPlus
};

const ADDR_SW_PATINFO GFX10_SW_64K_R_X_4xaa_RBPLUS_PATINFO[] =
{
    {   3,    0,  347,  566,    0, } , // 1 pipes (1 PKRs) 1 bpe @ SW_64K_R_X 4xaa @ RbPlus
    {   3,    1,  348,  733,    0, } , // 1 pipes (1 PKRs) 2 bpe @ SW_64K_R_X 4xaa @ RbPlus
    {   3,   39,  349,  568,    0, } , // 1 pipes (1 PKRs) 4 bpe @ SW_64K_R_X 4xaa @ RbPlus
    {   3,    6,  350,  734,    0, } , // 1 pipes (1 PKRs) 8 bpe @ SW_64K_R_X 4xaa @ RbPlus
    {   3,    7,  351,  735,    0, } , // 1 pipes (1 PKRs) 16 bpe @ SW_64K_R_X 4xaa @ RbPlus
    {   3,    0,  352,  566,    0, } , // 2 pipes (1-2 PKRs) 1 bpe @ SW_64K_R_X 4xaa @ RbPlus
    {   3,    1,  353,  567,    0, } , // 2 pipes (1-2 PKRs) 2 bpe @ SW_64K_R_X 4xaa @ RbPlus
    {   3,   39,  354,  568,    0, } , // 2 pipes (1-2 PKRs) 4 bpe @ SW_64K_R_X 4xaa @ RbPlus
    {   3,    6,  355,  736,    0, } , // 2 pipes (1-2 PKRs) 8 bpe @ SW_64K_R_X 4xaa @ RbPlus
    {   3,    7,  356,  737,    0, } , // 2 pipes (1-2 PKRs) 16 bpe @ SW_64K_R_X 4xaa @ RbPlus
    {   3,    0,  280,  566,    0, } , // 4 pipes (1-2 PKRs) 1 bpe @ SW_64K_R_X 4xaa @ RbPlus
    {   3,    1,  281,  567,    0, } , // 4 pipes (1-2 PKRs) 2 bpe @ SW_64K_R_X 4xaa @ RbPlus
    {   3,   39,  282,  568,    0, } , // 4 pipes (1-2 PKRs) 4 bpe @ SW_64K_R_X 4xaa @ RbPlus
    {   3,    6,  283,  569,    0, } , // 4 pipes (1-2 PKRs) 8 bpe @ SW_64K_R_X 4xaa @ RbPlus
    {   3,    7,  284,  570,    0, } , // 4 pipes (1-2 PKRs) 16 bpe @ SW_64K_R_X 4xaa @ RbPlus
    {   3,    0,  394,  587,    0, } , // 8 pipes (2 PKRs) 1 bpe @ SW_64K_R_X 4xaa @ RbPlus
    {   3,    1,  395,  738,    0, } , // 8 pipes (2 PKRs) 2 bpe @ SW_64K_R_X 4xaa @ RbPlus
    {   3,   39,  396,  739,    0, } , // 8 pipes (2 PKRs) 4 bpe @ SW_64K_R_X 4xaa @ RbPlus
    {   3,    6,  397,  740,    0, } , // 8 pipes (2 PKRs) 8 bpe @ SW_64K_R_X 4xaa @ RbPlus
    {   3,    7,  430,  741,    0, } , // 8 pipes (2 PKRs) 16 bpe @ SW_64K_R_X 4xaa @ RbPlus
    {   3,    0,  290,  576,    0, } , // 4 pipes (4 PKRs) 1 bpe @ SW_64K_R_X 4xaa @ RbPlus
    {   3,    1,  291,  577,    0, } , // 4 pipes (4 PKRs) 2 bpe @ SW_64K_R_X 4xaa @ RbPlus
    {   3,   39,  292,  578,    0, } , // 4 pipes (4 PKRs) 4 bpe @ SW_64K_R_X 4xaa @ RbPlus
    {   3,    6,  293,  579,    0, } , // 4 pipes (4 PKRs) 8 bpe @ SW_64K_R_X 4xaa @ RbPlus
    {   3,    7,  405,  580,    0, } , // 4 pipes (4 PKRs) 16 bpe @ SW_64K_R_X 4xaa @ RbPlus
    {   3,    0,  295,  587,    0, } , // 8 pipes (4 PKRs) 1 bpe @ SW_64K_R_X 4xaa @ RbPlus
    {   3,    1,  296,  742,    0, } , // 8 pipes (4 PKRs) 2 bpe @ SW_64K_R_X 4xaa @ RbPlus
    {   3,   39,  297,  743,    0, } , // 8 pipes (4 PKRs) 4 bpe @ SW_64K_R_X 4xaa @ RbPlus
    {   3,    6,  298,  740,    0, } , // 8 pipes (4 PKRs) 8 bpe @ SW_64K_R_X 4xaa @ RbPlus
    {   3,    7,  431,  699,    0, } , // 8 pipes (4 PKRs) 16 bpe @ SW_64K_R_X 4xaa @ RbPlus
    {   3,    0,  399,  744,    0, } , // 16 pipes (4 PKRs) 1 bpe @ SW_64K_R_X 4xaa @ RbPlus
    {   3,    1,  399,  745,    0, } , // 16 pipes (4 PKRs) 2 bpe @ SW_64K_R_X 4xaa @ RbPlus
    {   3,   39,  399,  746,    0, } , // 16 pipes (4 PKRs) 4 bpe @ SW_64K_R_X 4xaa @ RbPlus
    {   3,    6,  432,  747,    0, } , // 16 pipes (4 PKRs) 8 bpe @ SW_64K_R_X 4xaa @ RbPlus
    {   3,    7,  433,  429,    0, } , // 16 pipes (4 PKRs) 16 bpe @ SW_64K_R_X 4xaa @ RbPlus
    {   3,    0,  400,  748,    0, } , // 8 pipes (8 PKRs) 1 bpe @ SW_64K_R_X 4xaa @ RbPlus
    {   3,    1,  401,  749,    0, } , // 8 pipes (8 PKRs) 2 bpe @ SW_64K_R_X 4xaa @ RbPlus
    {   3,   39,  402,  750,    0, } , // 8 pipes (8 PKRs) 4 bpe @ SW_64K_R_X 4xaa @ RbPlus
    {   3,    6,  434,  707,    0, } , // 8 pipes (8 PKRs) 8 bpe @ SW_64K_R_X 4xaa @ RbPlus
    {   3,    7,  435,  708,    0, } , // 8 pipes (8 PKRs) 16 bpe @ SW_64K_R_X 4xaa @ RbPlus
    {   3,    0,  307,  744,    0, } , // 16 pipes (8 PKRs) 1 bpe @ SW_64K_R_X 4xaa @ RbPlus
    {   3,    1,  307,  751,    0, } , // 16 pipes (8 PKRs) 2 bpe @ SW_64K_R_X 4xaa @ RbPlus
    {   3,   39,  307,  746,    0, } , // 16 pipes (8 PKRs) 4 bpe @ SW_64K_R_X 4xaa @ RbPlus
    {   3,    6,  436,  703,    0, } , // 16 pipes (8 PKRs) 8 bpe @ SW_64K_R_X 4xaa @ RbPlus
    {   3,    7,  437,  390,    0, } , // 16 pipes (8 PKRs) 16 bpe @ SW_64K_R_X 4xaa @ RbPlus
    {   3,    0,  307,  752,    0, } , // 32 pipes (8 PKRs) 1 bpe @ SW_64K_R_X 4xaa @ RbPlus
    {   3,    1,  307,  753,    0, } , // 32 pipes (8 PKRs) 2 bpe @ SW_64K_R_X 4xaa @ RbPlus
    {   3,   39,  307,  754,    0, } , // 32 pipes (8 PKRs) 4 bpe @ SW_64K_R_X 4xaa @ RbPlus
    {   3,    6,  436,  712,    0, } , // 32 pipes (8 PKRs) 8 bpe @ SW_64K_R_X 4xaa @ RbPlus
    {   3,    7,  437,  676,    0, } , // 32 pipes (8 PKRs) 16 bpe @ SW_64K_R_X 4xaa @ RbPlus
    {   3,    0,  323,  755,    0, } , // 16 pipes (16 PKRs) 1 bpe @ SW_64K_R_X 4xaa @ RbPlus
    {   3,    1,  323,  756,    0, } , // 16 pipes (16 PKRs) 2 bpe @ SW_64K_R_X 4xaa @ RbPlus
    {   3,   39,  323,  757,    0, } , // 16 pipes (16 PKRs) 4 bpe @ SW_64K_R_X 4xaa @ RbPlus
    {   3,    6,  438,  716,    0, } , // 16 pipes (16 PKRs) 8 bpe @ SW_64K_R_X 4xaa @ RbPlus
    {   3,    7,  439,  400,    0, } , // 16 pipes (16 PKRs) 16 bpe @ SW_64K_R_X 4xaa @ RbPlus
    {   3,    0,  323,  758,    0, } , // 32 pipes (16 PKRs) 1 bpe @ SW_64K_R_X 4xaa @ RbPlus
    {   3,    1,  323,  759,    0, } , // 32 pipes (16 PKRs) 2 bpe @ SW_64K_R_X 4xaa @ RbPlus
    {   3,   39,  323,  760,    0, } , // 32 pipes (16 PKRs) 4 bpe @ SW_64K_R_X 4xaa @ RbPlus
    {   3,    6,  438,  720,    0, } , // 32 pipes (16 PKRs) 8 bpe @ SW_64K_R_X 4xaa @ RbPlus
    {   3,    7,  439,  405,    0, } , // 32 pipes (16 PKRs) 16 bpe @ SW_64K_R_X 4xaa @ RbPlus
    {   3,    0,  323,  761,    0, } , // 64 pipes (16 PKRs) 1 bpe @ SW_64K_R_X 4xaa @ RbPlus
    {   3,    1,  323,  762,    0, } , // 64 pipes (16 PKRs) 2 bpe @ SW_64K_R_X 4xaa @ RbPlus
    {   3,   39,  323,  763,    0, } , // 64 pipes (16 PKRs) 4 bpe @ SW_64K_R_X 4xaa @ RbPlus
    {   3,    6,  438,  724,    0, } , // 64 pipes (16 PKRs) 8 bpe @ SW_64K_R_X 4xaa @ RbPlus
    {   3,    7,  439,  684,    0, } , // 64 pipes (16 PKRs) 16 bpe @ SW_64K_R_X 4xaa @ RbPlus
    {   3,    0,  324,  764,    0, } , // 32 pipes (32 PKRs) 1 bpe @ SW_64K_R_X 4xaa @ RbPlus
    {   3,    1,  324,  765,    0, } , // 32 pipes (32 PKRs) 2 bpe @ SW_64K_R_X 4xaa @ RbPlus
    {   3,   39,  324,  766,    0, } , // 32 pipes (32 PKRs) 4 bpe @ SW_64K_R_X 4xaa @ RbPlus
    {   3,    6,  440,  728,    0, } , // 32 pipes (32 PKRs) 8 bpe @ SW_64K_R_X 4xaa @ RbPlus
    {   3,    7,  441,  412,    0, } , // 32 pipes (32 PKRs) 16 bpe @ SW_64K_R_X 4xaa @ RbPlus
    {   3,    0,  324,  767,    0, } , // 64 pipes (32 PKRs) 1 bpe @ SW_64K_R_X 4xaa @ RbPlus
    {   3,    1,  324,  768,    0, } , // 64 pipes (32 PKRs) 2 bpe @ SW_64K_R_X 4xaa @ RbPlus
    {   3,   39,  324,  769,    0, } , // 64 pipes (32 PKRs) 4 bpe @ SW_64K_R_X 4xaa @ RbPlus
    {   3,    6,  440,  732,    0, } , // 64 pipes (32 PKRs) 8 bpe @ SW_64K_R_X 4xaa @ RbPlus
    {   3,    7,  441,  414,    0, } , // 64 pipes (32 PKRs) 16 bpe @ SW_64K_R_X 4xaa @ RbPlus
};

const ADDR_SW_PATINFO GFX10_SW_64K_R_X_8xaa_RBPLUS_PATINFO[] =
{
    {   3,    0,  424,  619,    0, } , // 1 pipes (1 PKRs) 1 bpe @ SW_64K_R_X 8xaa @ RbPlus
    {   3,    1,  348,  620,    0, } , // 1 pipes (1 PKRs) 2 bpe @ SW_64K_R_X 8xaa @ RbPlus
    {   3,   39,  358,  621,    0, } , // 1 pipes (1 PKRs) 4 bpe @ SW_64K_R_X 8xaa @ RbPlus
    {   3,    6,  350,  770,    0, } , // 1 pipes (1 PKRs) 8 bpe @ SW_64K_R_X 8xaa @ RbPlus
    {   3,    7,  359,  771,    0, } , // 1 pipes (1 PKRs) 16 bpe @ SW_64K_R_X 8xaa @ RbPlus
    {   3,    0,  352,  619,    0, } , // 2 pipes (1-2 PKRs) 1 bpe @ SW_64K_R_X 8xaa @ RbPlus
    {   3,    1,  353,  620,    0, } , // 2 pipes (1-2 PKRs) 2 bpe @ SW_64K_R_X 8xaa @ RbPlus
    {   3,   39,  354,  621,    0, } , // 2 pipes (1-2 PKRs) 4 bpe @ SW_64K_R_X 8xaa @ RbPlus
    {   3,    6,  355,  770,    0, } , // 2 pipes (1-2 PKRs) 8 bpe @ SW_64K_R_X 8xaa @ RbPlus
    {   3,    7,  378,  772,    0, } , // 2 pipes (1-2 PKRs) 16 bpe @ SW_64K_R_X 8xaa @ RbPlus
    {   3,    0,  280,  619,    0, } , // 4 pipes (1-2 PKRs) 1 bpe @ SW_64K_R_X 8xaa @ RbPlus
    {   3,    1,  281,  620,    0, } , // 4 pipes (1-2 PKRs) 2 bpe @ SW_64K_R_X 8xaa @ RbPlus
    {   3,   39,  282,  621,    0, } , // 4 pipes (1-2 PKRs) 4 bpe @ SW_64K_R_X 8xaa @ RbPlus
    {   3,    6,  283,  622,    0, } , // 4 pipes (1-2 PKRs) 8 bpe @ SW_64K_R_X 8xaa @ RbPlus
    {   3,    7,  413,  623,    0, } , // 4 pipes (1-2 PKRs) 16 bpe @ SW_64K_R_X 8xaa @ RbPlus
    {   3,    0,  394,  773,    0, } , // 8 pipes (2 PKRs) 1 bpe @ SW_64K_R_X 8xaa @ RbPlus
    {   3,    1,  395,  774,    0, } , // 8 pipes (2 PKRs) 2 bpe @ SW_64K_R_X 8xaa @ RbPlus
    {   3,   39,  442,  775,    0, } , // 8 pipes (2 PKRs) 4 bpe @ SW_64K_R_X 8xaa @ RbPlus
    {   3,    6,  443,  776,    0, } , // 8 pipes (2 PKRs) 8 bpe @ SW_64K_R_X 8xaa @ RbPlus
    {   3,    7,  444,  777,    0, } , // 8 pipes (2 PKRs) 16 bpe @ SW_64K_R_X 8xaa @ RbPlus
    {   3,    0,  415,  629,    0, } , // 4 pipes (4 PKRs) 1 bpe @ SW_64K_R_X 8xaa @ RbPlus
    {   3,    1,  291,  630,    0, } , // 4 pipes (4 PKRs) 2 bpe @ SW_64K_R_X 8xaa @ RbPlus
    {   3,   39,  292,  631,    0, } , // 4 pipes (4 PKRs) 4 bpe @ SW_64K_R_X 8xaa @ RbPlus
    {   3,    6,  416,  632,    0, } , // 4 pipes (4 PKRs) 8 bpe @ SW_64K_R_X 8xaa @ RbPlus
    {   3,    7,  417,  580,    0, } , // 4 pipes (4 PKRs) 16 bpe @ SW_64K_R_X 8xaa @ RbPlus
    {   3,    0,  295,  773,    0, } , // 8 pipes (4 PKRs) 1 bpe @ SW_64K_R_X 8xaa @ RbPlus
    {   3,    1,  296,  778,    0, } , // 8 pipes (4 PKRs) 2 bpe @ SW_64K_R_X 8xaa @ RbPlus
    {   3,   39,  297,  779,    0, } , // 8 pipes (4 PKRs) 4 bpe @ SW_64K_R_X 8xaa @ RbPlus
    {   3,    6,  445,  780,    0, } , // 8 pipes (4 PKRs) 8 bpe @ SW_64K_R_X 8xaa @ RbPlus
    {   3,    7,  446,  699,    0, } , // 8 pipes (4 PKRs) 16 bpe @ SW_64K_R_X 8xaa @ RbPlus
    {   3,    0,  399,  781,    0, } , // 16 pipes (4 PKRs) 1 bpe @ SW_64K_R_X 8xaa @ RbPlus
    {   3,    1,  399,  782,    0, } , // 16 pipes (4 PKRs) 2 bpe @ SW_64K_R_X 8xaa @ RbPlus
    {   3,   39,  447,  783,    0, } , // 16 pipes (4 PKRs) 4 bpe @ SW_64K_R_X 8xaa @ RbPlus
    {   3,    6,  448,  784,    0, } , // 16 pipes (4 PKRs) 8 bpe @ SW_64K_R_X 8xaa @ RbPlus
    {   3,    7,  449,  429,    0, } , // 16 pipes (4 PKRs) 16 bpe @ SW_64K_R_X 8xaa @ RbPlus
    {   3,    0,  450,  785,    0, } , // 8 pipes (8 PKRs) 1 bpe @ SW_64K_R_X 8xaa @ RbPlus
    {   3,    1,  302,  786,    0, } , // 8 pipes (8 PKRs) 2 bpe @ SW_64K_R_X 8xaa @ RbPlus
    {   3,   39,  303,  787,    0, } , // 8 pipes (8 PKRs) 4 bpe @ SW_64K_R_X 8xaa @ RbPlus
    {   3,    6,  420,  788,    0, } , // 8 pipes (8 PKRs) 8 bpe @ SW_64K_R_X 8xaa @ RbPlus
    {   3,    7,  451,  708,    0, } , // 8 pipes (8 PKRs) 16 bpe @ SW_64K_R_X 8xaa @ RbPlus
    {   3,    0,  339,  781,    0, } , // 16 pipes (8 PKRs) 1 bpe @ SW_64K_R_X 8xaa @ RbPlus
    {   3,    1,  339,  782,    0, } , // 16 pipes (8 PKRs) 2 bpe @ SW_64K_R_X 8xaa @ RbPlus
    {   3,   39,  422,  746,    0, } , // 16 pipes (8 PKRs) 4 bpe @ SW_64K_R_X 8xaa @ RbPlus
    {   3,    6,  452,  703,    0, } , // 16 pipes (8 PKRs) 8 bpe @ SW_64K_R_X 8xaa @ RbPlus
    {   3,    7,  453,  390,    0, } , // 16 pipes (8 PKRs) 16 bpe @ SW_64K_R_X 8xaa @ RbPlus
    {   3,    0,  339,  789,    0, } , // 32 pipes (8 PKRs) 1 bpe @ SW_64K_R_X 8xaa @ RbPlus
    {   3,    1,  339,  790,    0, } , // 32 pipes (8 PKRs) 2 bpe @ SW_64K_R_X 8xaa @ RbPlus
    {   3,   39,  422,  754,    0, } , // 32 pipes (8 PKRs) 4 bpe @ SW_64K_R_X 8xaa @ RbPlus
    {   3,    6,  452,  712,    0, } , // 32 pipes (8 PKRs) 8 bpe @ SW_64K_R_X 8xaa @ RbPlus
    {   3,    7,  453,  676,    0, } , // 32 pipes (8 PKRs) 16 bpe @ SW_64K_R_X 8xaa @ RbPlus
    {   3,    0,  343,  791,    0, } , // 16 pipes (16 PKRs) 1 bpe @ SW_64K_R_X 8xaa @ RbPlus
    {   3,    1,  341,  792,    0, } , // 16 pipes (16 PKRs) 2 bpe @ SW_64K_R_X 8xaa @ RbPlus
    {   3,   39,  423,  757,    0, } , // 16 pipes (16 PKRs) 4 bpe @ SW_64K_R_X 8xaa @ RbPlus
    {   3,    6,  454,  716,    0, } , // 16 pipes (16 PKRs) 8 bpe @ SW_64K_R_X 8xaa @ RbPlus
    {   3,    7,  455,  400,    0, } , // 16 pipes (16 PKRs) 16 bpe @ SW_64K_R_X 8xaa @ RbPlus
    {   3,    0,  343,  793,    0, } , // 32 pipes (16 PKRs) 1 bpe @ SW_64K_R_X 8xaa @ RbPlus
    {   3,    1,  341,  794,    0, } , // 32 pipes (16 PKRs) 2 bpe @ SW_64K_R_X 8xaa @ RbPlus
    {   3,   39,  423,  760,    0, } , // 32 pipes (16 PKRs) 4 bpe @ SW_64K_R_X 8xaa @ RbPlus
    {   3,    6,  454,  720,    0, } , // 32 pipes (16 PKRs) 8 bpe @ SW_64K_R_X 8xaa @ RbPlus
    {   3,    7,  455,  405,    0, } , // 32 pipes (16 PKRs) 16 bpe @ SW_64K_R_X 8xaa @ RbPlus
    {   3,    0,  343,  795,    0, } , // 64 pipes (16 PKRs) 1 bpe @ SW_64K_R_X 8xaa @ RbPlus
    {   3,    1,  341,  796,    0, } , // 64 pipes (16 PKRs) 2 bpe @ SW_64K_R_X 8xaa @ RbPlus
    {   3,   39,  423,  763,    0, } , // 64 pipes (16 PKRs) 4 bpe @ SW_64K_R_X 8xaa @ RbPlus
    {   3,    6,  454,  724,    0, } , // 64 pipes (16 PKRs) 8 bpe @ SW_64K_R_X 8xaa @ RbPlus
    {   3,    7,  455,  684,    0, } , // 64 pipes (16 PKRs) 16 bpe @ SW_64K_R_X 8xaa @ RbPlus
    {   3,    0,  344,  797,    0, } , // 32 pipes (32 PKRs) 1 bpe @ SW_64K_R_X 8xaa @ RbPlus
    {   3,    1,  345,  798,    0, } , // 32 pipes (32 PKRs) 2 bpe @ SW_64K_R_X 8xaa @ RbPlus
    {   3,   39,  456,  766,    0, } , // 32 pipes (32 PKRs) 4 bpe @ SW_64K_R_X 8xaa @ RbPlus
    {   3,    6,  457,  728,    0, } , // 32 pipes (32 PKRs) 8 bpe @ SW_64K_R_X 8xaa @ RbPlus
    {   3,    7,  458,  412,    0, } , // 32 pipes (32 PKRs) 16 bpe @ SW_64K_R_X 8xaa @ RbPlus
    {   3,    0,  344,  799,    0, } , // 64 pipes (32 PKRs) 1 bpe @ SW_64K_R_X 8xaa @ RbPlus
    {   3,    1,  345,  800,    0, } , // 64 pipes (32 PKRs) 2 bpe @ SW_64K_R_X 8xaa @ RbPlus
    {   3,   39,  456,  769,    0, } , // 64 pipes (32 PKRs) 4 bpe @ SW_64K_R_X 8xaa @ RbPlus
    {   3,    6,  457,  732,    0, } , // 64 pipes (32 PKRs) 8 bpe @ SW_64K_R_X 8xaa @ RbPlus
    {   3,    7,  458,  414,    0, } , // 64 pipes (32 PKRs) 16 bpe @ SW_64K_R_X 8xaa @ RbPlus
};

const ADDR_SW_PATINFO GFX10_SW_64K_Z_X_1xaa_RBPLUS_PATINFO[] =
{
    {   2,    8,  347,  193,    0, } , // 1 pipes (1 PKRs) 1 bpe @ SW_64K_Z_X 1xaa @ RbPlus
    {   2,    9,  348,  366,    0, } , // 1 pipes (1 PKRs) 2 bpe @ SW_64K_Z_X 1xaa @ RbPlus
    {   2,   10,  349,  195,    0, } , // 1 pipes (1 PKRs) 4 bpe @ SW_64K_Z_X 1xaa @ RbPlus
    {   2,   11,  350,  367,    0, } , // 1 pipes (1 PKRs) 8 bpe @ SW_64K_Z_X 1xaa @ RbPlus
    {   2,    7,  351,  368,    0, } , // 1 pipes (1 PKRs) 16 bpe @ SW_64K_Z_X 1xaa @ RbPlus
    {   3,    8,  352,  193,    0, } , // 2 pipes (1-2 PKRs) 1 bpe @ SW_64K_Z_X 1xaa @ RbPlus
    {   3,    9,  353,  194,    0, } , // 2 pipes (1-2 PKRs) 2 bpe @ SW_64K_Z_X 1xaa @ RbPlus
    {   3,   10,  354,  195,    0, } , // 2 pipes (1-2 PKRs) 4 bpe @ SW_64K_Z_X 1xaa @ RbPlus
    {   3,   11,  355,  369,    0, } , // 2 pipes (1-2 PKRs) 8 bpe @ SW_64K_Z_X 1xaa @ RbPlus
    {   3,    7,  356,  370,    0, } , // 2 pipes (1-2 PKRs) 16 bpe @ SW_64K_Z_X 1xaa @ RbPlus
    {   3,    8,  280,  193,    0, } , // 4 pipes (1-2 PKRs) 1 bpe @ SW_64K_Z_X 1xaa @ RbPlus
    {   3,    9,  281,  194,    0, } , // 4 pipes (1-2 PKRs) 2 bpe @ SW_64K_Z_X 1xaa @ RbPlus
    {   3,   10,  282,  195,    0, } , // 4 pipes (1-2 PKRs) 4 bpe @ SW_64K_Z_X 1xaa @ RbPlus
    {   3,   11,  283,  196,    0, } , // 4 pipes (1-2 PKRs) 8 bpe @ SW_64K_Z_X 1xaa @ RbPlus
    {   3,    7,  284,  197,    0, } , // 4 pipes (1-2 PKRs) 16 bpe @ SW_64K_Z_X 1xaa @ RbPlus
    {   3,    8,  285,  219,    0, } , // 8 pipes (2 PKRs) 1 bpe @ SW_64K_Z_X 1xaa @ RbPlus
    {   3,    9,  286,  371,    0, } , // 8 pipes (2 PKRs) 2 bpe @ SW_64K_Z_X 1xaa @ RbPlus
    {   3,   10,  287,  372,    0, } , // 8 pipes (2 PKRs) 4 bpe @ SW_64K_Z_X 1xaa @ RbPlus
    {   3,   11,  288,  373,    0, } , // 8 pipes (2 PKRs) 8 bpe @ SW_64K_Z_X 1xaa @ RbPlus
    {   3,    7,  289,  374,    0, } , // 8 pipes (2 PKRs) 16 bpe @ SW_64K_Z_X 1xaa @ RbPlus
    {   3,    8,  290,  203,    0, } , // 4 pipes (4 PKRs) 1 bpe @ SW_64K_Z_X 1xaa @ RbPlus
    {   3,    9,  291,  204,    0, } , // 4 pipes (4 PKRs) 2 bpe @ SW_64K_Z_X 1xaa @ RbPlus
    {   3,   10,  292,  205,    0, } , // 4 pipes (4 PKRs) 4 bpe @ SW_64K_Z_X 1xaa @ RbPlus
    {   3,   11,  293,  206,    0, } , // 4 pipes (4 PKRs) 8 bpe @ SW_64K_Z_X 1xaa @ RbPlus
    {   3,    7,  294,  207,    0, } , // 4 pipes (4 PKRs) 16 bpe @ SW_64K_Z_X 1xaa @ RbPlus
    {   3,    8,  295,  219,    0, } , // 8 pipes (4 PKRs) 1 bpe @ SW_64K_Z_X 1xaa @ RbPlus
    {   3,    9,  296,  375,    0, } , // 8 pipes (4 PKRs) 2 bpe @ SW_64K_Z_X 1xaa @ RbPlus
    {   3,   10,  297,  376,    0, } , // 8 pipes (4 PKRs) 4 bpe @ SW_64K_Z_X 1xaa @ RbPlus
    {   3,   11,  298,  377,    0, } , // 8 pipes (4 PKRs) 8 bpe @ SW_64K_Z_X 1xaa @ RbPlus
    {   3,    7,  299,  378,    0, } , // 8 pipes (4 PKRs) 16 bpe @ SW_64K_Z_X 1xaa @ RbPlus
    {   3,    8,  300,  379,    0, } , // 16 pipes (4 PKRs) 1 bpe @ SW_64K_Z_X 1xaa @ RbPlus
    {   3,    9,  300,  380,    0, } , // 16 pipes (4 PKRs) 2 bpe @ SW_64K_Z_X 1xaa @ RbPlus
    {   3,   10,  300,  381,    0, } , // 16 pipes (4 PKRs) 4 bpe @ SW_64K_Z_X 1xaa @ RbPlus
    {   3,   11,  300,  382,    0, } , // 16 pipes (4 PKRs) 8 bpe @ SW_64K_Z_X 1xaa @ RbPlus
    {   3,    7,  300,  383,    0, } , // 16 pipes (4 PKRs) 16 bpe @ SW_64K_Z_X 1xaa @ RbPlus
    {   3,    8,  301,  384,    0, } , // 8 pipes (8 PKRs) 1 bpe @ SW_64K_Z_X 1xaa @ RbPlus
    {   3,    9,  302,  385,    0, } , // 8 pipes (8 PKRs) 2 bpe @ SW_64K_Z_X 1xaa @ RbPlus
    {   3,   10,  303,  386,    0, } , // 8 pipes (8 PKRs) 4 bpe @ SW_64K_Z_X 1xaa @ RbPlus
    {   3,   11,  304,  387,    0, } , // 8 pipes (8 PKRs) 8 bpe @ SW_64K_Z_X 1xaa @ RbPlus
    {   3,    7,  305,  388,    0, } , // 8 pipes (8 PKRs) 16 bpe @ SW_64K_Z_X 1xaa @ RbPlus
    {   3,    8,  306,  379,    0, } , // 16 pipes (8 PKRs) 1 bpe @ SW_64K_Z_X 1xaa @ RbPlus
    {   3,    9,  306,  389,    0, } , // 16 pipes (8 PKRs) 2 bpe @ SW_64K_Z_X 1xaa @ RbPlus
    {   3,   10,  306,  381,    0, } , // 16 pipes (8 PKRs) 4 bpe @ SW_64K_Z_X 1xaa @ RbPlus
    {   3,   11,  307,  382,    0, } , // 16 pipes (8 PKRs) 8 bpe @ SW_64K_Z_X 1xaa @ RbPlus
    {   3,    7,  307,  390,    0, } , // 16 pipes (8 PKRs) 16 bpe @ SW_64K_Z_X 1xaa @ RbPlus
    {   3,    8,  306,  391,    0, } , // 32 pipes (8 PKRs) 1 bpe @ SW_64K_Z_X 1xaa @ RbPlus
    {   3,    9,  306,  392,    0, } , // 32 pipes (8 PKRs) 2 bpe @ SW_64K_Z_X 1xaa @ RbPlus
    {   3,   10,  306,  393,    0, } , // 32 pipes (8 PKRs) 4 bpe @ SW_64K_Z_X 1xaa @ RbPlus
    {   3,   11,  307,  394,    0, } , // 32 pipes (8 PKRs) 8 bpe @ SW_64K_Z_X 1xaa @ RbPlus
    {   3,    7,  307,  395,    0, } , // 32 pipes (8 PKRs) 16 bpe @ SW_64K_Z_X 1xaa @ RbPlus
    {   3,    8,  308,  396,    0, } , // 16 pipes (16 PKRs) 1 bpe @ SW_64K_Z_X 1xaa @ RbPlus
    {   3,    9,  308,  397,    0, } , // 16 pipes (16 PKRs) 2 bpe @ SW_64K_Z_X 1xaa @ RbPlus
    {   3,   10,  308,  398,    0, } , // 16 pipes (16 PKRs) 4 bpe @ SW_64K_Z_X 1xaa @ RbPlus
    {   3,   11,  309,  399,    0, } , // 16 pipes (16 PKRs) 8 bpe @ SW_64K_Z_X 1xaa @ RbPlus
    {   3,    7,  323,  400,    0, } , // 16 pipes (16 PKRs) 16 bpe @ SW_64K_Z_X 1xaa @ RbPlus
    {   3,    8,  308,  401,    0, } , // 32 pipes (16 PKRs) 1 bpe @ SW_64K_Z_X 1xaa @ RbPlus
    {   3,    9,  308,  402,    0, } , // 32 pipes (16 PKRs) 2 bpe @ SW_64K_Z_X 1xaa @ RbPlus
    {   3,   10,  308,  403,    0, } , // 32 pipes (16 PKRs) 4 bpe @ SW_64K_Z_X 1xaa @ RbPlus
    {   3,   11,  309,  404,    0, } , // 32 pipes (16 PKRs) 8 bpe @ SW_64K_Z_X 1xaa @ RbPlus
    {   3,    7,  323,  405,    0, } , // 32 pipes (16 PKRs) 16 bpe @ SW_64K_Z_X 1xaa @ RbPlus
    {   3,    8,  308,  240,    0, } , // 64 pipes (16 PKRs) 1 bpe @ SW_64K_Z_X 1xaa @ RbPlus
    {   3,    9,  308,  241,    0, } , // 64 pipes (16 PKRs) 2 bpe @ SW_64K_Z_X 1xaa @ RbPlus
    {   3,   10,  308,  242,    0, } , // 64 pipes (16 PKRs) 4 bpe @ SW_64K_Z_X 1xaa @ RbPlus
    {   3,   11,  309,  406,    0, } , // 64 pipes (16 PKRs) 8 bpe @ SW_64K_Z_X 1xaa @ RbPlus
    {   3,    7,  323,  407,    0, } , // 64 pipes (16 PKRs) 16 bpe @ SW_64K_Z_X 1xaa @ RbPlus
    {   3,    8,  310,  408,    0, } , // 32 pipes (32 PKRs) 1 bpe @ SW_64K_Z_X 1xaa @ RbPlus
    {   3,    9,  310,  409,    0, } , // 32 pipes (32 PKRs) 2 bpe @ SW_64K_Z_X 1xaa @ RbPlus
    {   3,   10,  310,  410,    0, } , // 32 pipes (32 PKRs) 4 bpe @ SW_64K_Z_X 1xaa @ RbPlus
    {   3,   11,  318,  411,    0, } , // 32 pipes (32 PKRs) 8 bpe @ SW_64K_Z_X 1xaa @ RbPlus
    {   3,    7,  324,  412,    0, } , // 32 pipes (32 PKRs) 16 bpe @ SW_64K_Z_X 1xaa @ RbPlus
    {   3,    8,  310,  250,    0, } , // 64 pipes (32 PKRs) 1 bpe @ SW_64K_Z_X 1xaa @ RbPlus
    {   3,    9,  310,  251,    0, } , // 64 pipes (32 PKRs) 2 bpe @ SW_64K_Z_X 1xaa @ RbPlus
    {   3,   10,  310,  252,    0, } , // 64 pipes (32 PKRs) 4 bpe @ SW_64K_Z_X 1xaa @ RbPlus
    {   3,   11,  318,  413,    0, } , // 64 pipes (32 PKRs) 8 bpe @ SW_64K_Z_X 1xaa @ RbPlus
    {   3,    7,  324,  414,    0, } , // 64 pipes (32 PKRs) 16 bpe @ SW_64K_Z_X 1xaa @ RbPlus
};

const ADDR_SW_PATINFO GFX10_SW_64K_Z_X_2xaa_RBPLUS_PATINFO[] =
{
    {   2,   13,  357,  415,    0, } , // 1 pipes (1 PKRs) 1 bpe @ SW_64K_Z_X 2xaa @ RbPlus
    {   2,   14,  349,  195,    0, } , // 1 pipes (1 PKRs) 2 bpe @ SW_64K_Z_X 2xaa @ RbPlus
    {   3,   15,  358,  263,    0, } , // 1 pipes (1 PKRs) 4 bpe @ SW_64K_Z_X 2xaa @ RbPlus
    {   3,   16,  350,  416,    0, } , // 1 pipes (1 PKRs) 8 bpe @ SW_64K_Z_X 2xaa @ RbPlus
    {   3,   17,  359,  417,    0, } , // 1 pipes (1 PKRs) 16 bpe @ SW_64K_Z_X 2xaa @ RbPlus
    {   3,   13,  360,  415,    0, } , // 2 pipes (1-2 PKRs) 1 bpe @ SW_64K_Z_X 2xaa @ RbPlus
    {   3,   14,  354,  195,    0, } , // 2 pipes (1-2 PKRs) 2 bpe @ SW_64K_Z_X 2xaa @ RbPlus
    {   3,   15,  354,  263,    0, } , // 2 pipes (1-2 PKRs) 4 bpe @ SW_64K_Z_X 2xaa @ RbPlus
    {   3,   16,  361,  418,    0, } , // 2 pipes (1-2 PKRs) 8 bpe @ SW_64K_Z_X 2xaa @ RbPlus
    {   3,   17,  356,  419,    0, } , // 2 pipes (1-2 PKRs) 16 bpe @ SW_64K_Z_X 2xaa @ RbPlus
    {   3,   13,  281,  262,    0, } , // 4 pipes (1-2 PKRs) 1 bpe @ SW_64K_Z_X 2xaa @ RbPlus
    {   3,   14,  282,  195,    0, } , // 4 pipes (1-2 PKRs) 2 bpe @ SW_64K_Z_X 2xaa @ RbPlus
    {   3,   15,  282,  263,    0, } , // 4 pipes (1-2 PKRs) 4 bpe @ SW_64K_Z_X 2xaa @ RbPlus
    {   3,   16,  317,  264,    0, } , // 4 pipes (1-2 PKRs) 8 bpe @ SW_64K_Z_X 2xaa @ RbPlus
    {   3,   17,  284,  265,    0, } , // 4 pipes (1-2 PKRs) 16 bpe @ SW_64K_Z_X 2xaa @ RbPlus
    {   3,   13,  286,  420,    0, } , // 8 pipes (2 PKRs) 1 bpe @ SW_64K_Z_X 2xaa @ RbPlus
    {   3,   14,  287,  376,    0, } , // 8 pipes (2 PKRs) 2 bpe @ SW_64K_Z_X 2xaa @ RbPlus
    {   3,   15,  287,  421,    0, } , // 8 pipes (2 PKRs) 4 bpe @ SW_64K_Z_X 2xaa @ RbPlus
    {   3,   16,  289,  422,    0, } , // 8 pipes (2 PKRs) 8 bpe @ SW_64K_Z_X 2xaa @ RbPlus
    {   3,   17,  289,  423,    0, } , // 8 pipes (2 PKRs) 16 bpe @ SW_64K_Z_X 2xaa @ RbPlus
    {   3,   13,  291,  268,    0, } , // 4 pipes (4 PKRs) 1 bpe @ SW_64K_Z_X 2xaa @ RbPlus
    {   3,   14,  292,  205,    0, } , // 4 pipes (4 PKRs) 2 bpe @ SW_64K_Z_X 2xaa @ RbPlus
    {   3,   15,  292,  269,    0, } , // 4 pipes (4 PKRs) 4 bpe @ SW_64K_Z_X 2xaa @ RbPlus
    {   3,   16,  293,  270,    0, } , // 4 pipes (4 PKRs) 8 bpe @ SW_64K_Z_X 2xaa @ RbPlus
    {   3,   17,  294,  271,    0, } , // 4 pipes (4 PKRs) 16 bpe @ SW_64K_Z_X 2xaa @ RbPlus
    {   3,   13,  296,  420,    0, } , // 8 pipes (4 PKRs) 1 bpe @ SW_64K_Z_X 2xaa @ RbPlus
    {   3,   14,  297,  376,    0, } , // 8 pipes (4 PKRs) 2 bpe @ SW_64K_Z_X 2xaa @ RbPlus
    {   3,   15,  297,  421,    0, } , // 8 pipes (4 PKRs) 4 bpe @ SW_64K_Z_X 2xaa @ RbPlus
    {   3,   16,  298,  424,    0, } , // 8 pipes (4 PKRs) 8 bpe @ SW_64K_Z_X 2xaa @ RbPlus
    {   3,   17,  299,  423,    0, } , // 8 pipes (4 PKRs) 16 bpe @ SW_64K_Z_X 2xaa @ RbPlus
    {   3,   13,  300,  425,    0, } , // 16 pipes (4 PKRs) 1 bpe @ SW_64K_Z_X 2xaa @ RbPlus
    {   3,   14,  300,  426,    0, } , // 16 pipes (4 PKRs) 2 bpe @ SW_64K_Z_X 2xaa @ RbPlus
    {   3,   15,  300,  427,    0, } , // 16 pipes (4 PKRs) 4 bpe @ SW_64K_Z_X 2xaa @ RbPlus
    {   3,   16,  362,  428,    0, } , // 16 pipes (4 PKRs) 8 bpe @ SW_64K_Z_X 2xaa @ RbPlus
    {   3,   17,  363,  429,    0, } , // 16 pipes (4 PKRs) 16 bpe @ SW_64K_Z_X 2xaa @ RbPlus
    {   3,   13,  302,  430,    0, } , // 8 pipes (8 PKRs) 1 bpe @ SW_64K_Z_X 2xaa @ RbPlus
    {   3,   14,  303,  386,    0, } , // 8 pipes (8 PKRs) 2 bpe @ SW_64K_Z_X 2xaa @ RbPlus
    {   3,   15,  303,  431,    0, } , // 8 pipes (8 PKRs) 4 bpe @ SW_64K_Z_X 2xaa @ RbPlus
    {   3,   16,  305,  432,    0, } , // 8 pipes (8 PKRs) 8 bpe @ SW_64K_Z_X 2xaa @ RbPlus
    {   3,   17,  364,  433,    0, } , // 8 pipes (8 PKRs) 16 bpe @ SW_64K_Z_X 2xaa @ RbPlus
    {   3,   13,  306,  380,    0, } , // 16 pipes (8 PKRs) 1 bpe @ SW_64K_Z_X 2xaa @ RbPlus
    {   3,   14,  306,  381,    0, } , // 16 pipes (8 PKRs) 2 bpe @ SW_64K_Z_X 2xaa @ RbPlus
    {   3,   15,  306,  434,    0, } , // 16 pipes (8 PKRs) 4 bpe @ SW_64K_Z_X 2xaa @ RbPlus
    {   3,   16,  307,  435,    0, } , // 16 pipes (8 PKRs) 8 bpe @ SW_64K_Z_X 2xaa @ RbPlus
    {   3,   17,  365,  435,    0, } , // 16 pipes (8 PKRs) 16 bpe @ SW_64K_Z_X 2xaa @ RbPlus
    {   3,   13,  306,  402,    0, } , // 32 pipes (8 PKRs) 1 bpe @ SW_64K_Z_X 2xaa @ RbPlus
    {   3,   14,  306,  403,    0, } , // 32 pipes (8 PKRs) 2 bpe @ SW_64K_Z_X 2xaa @ RbPlus
    {   3,   15,  306,  436,    0, } , // 32 pipes (8 PKRs) 4 bpe @ SW_64K_Z_X 2xaa @ RbPlus
    {   3,   16,  307,  405,    0, } , // 32 pipes (8 PKRs) 8 bpe @ SW_64K_Z_X 2xaa @ RbPlus
    {   3,   17,  365,  405,    0, } , // 32 pipes (8 PKRs) 16 bpe @ SW_64K_Z_X 2xaa @ RbPlus
    {   3,   13,  308,  397,    0, } , // 16 pipes (16 PKRs) 1 bpe @ SW_64K_Z_X 2xaa @ RbPlus
    {   3,   14,  308,  398,    0, } , // 16 pipes (16 PKRs) 2 bpe @ SW_64K_Z_X 2xaa @ RbPlus
    {   3,   15,  308,  437,    0, } , // 16 pipes (16 PKRs) 4 bpe @ SW_64K_Z_X 2xaa @ RbPlus
    {   3,   16,  323,  438,    0, } , // 16 pipes (16 PKRs) 8 bpe @ SW_64K_Z_X 2xaa @ RbPlus
    {   3,   17,  366,  438,    0, } , // 16 pipes (16 PKRs) 16 bpe @ SW_64K_Z_X 2xaa @ RbPlus
    {   3,   13,  308,  402,    0, } , // 32 pipes (16 PKRs) 1 bpe @ SW_64K_Z_X 2xaa @ RbPlus
    {   3,   14,  308,  403,    0, } , // 32 pipes (16 PKRs) 2 bpe @ SW_64K_Z_X 2xaa @ RbPlus
    {   3,   15,  308,  436,    0, } , // 32 pipes (16 PKRs) 4 bpe @ SW_64K_Z_X 2xaa @ RbPlus
    {   3,   16,  323,  439,    0, } , // 32 pipes (16 PKRs) 8 bpe @ SW_64K_Z_X 2xaa @ RbPlus
    {   3,   17,  366,  439,    0, } , // 32 pipes (16 PKRs) 16 bpe @ SW_64K_Z_X 2xaa @ RbPlus
    {   3,   13,  308,  440,    0, } , // 64 pipes (16 PKRs) 1 bpe @ SW_64K_Z_X 2xaa @ RbPlus
    {   3,   14,  308,  242,    0, } , // 64 pipes (16 PKRs) 2 bpe @ SW_64K_Z_X 2xaa @ RbPlus
    {   3,   15,  308,  441,    0, } , // 64 pipes (16 PKRs) 4 bpe @ SW_64K_Z_X 2xaa @ RbPlus
    {   3,   16,  323,  442,    0, } , // 64 pipes (16 PKRs) 8 bpe @ SW_64K_Z_X 2xaa @ RbPlus
    {   3,   17,  366,  442,    0, } , // 64 pipes (16 PKRs) 16 bpe @ SW_64K_Z_X 2xaa @ RbPlus
    {   3,   13,  310,  443,    0, } , // 32 pipes (32 PKRs) 1 bpe @ SW_64K_Z_X 2xaa @ RbPlus
    {   3,   14,  310,  410,    0, } , // 32 pipes (32 PKRs) 2 bpe @ SW_64K_Z_X 2xaa @ RbPlus
    {   3,   15,  310,  444,    0, } , // 32 pipes (32 PKRs) 4 bpe @ SW_64K_Z_X 2xaa @ RbPlus
    {   3,   16,  324,  412,    0, } , // 32 pipes (32 PKRs) 8 bpe @ SW_64K_Z_X 2xaa @ RbPlus
    {   3,   17,  367,  412,    0, } , // 32 pipes (32 PKRs) 16 bpe @ SW_64K_Z_X 2xaa @ RbPlus
    {   3,   13,  310,  445,    0, } , // 64 pipes (32 PKRs) 1 bpe @ SW_64K_Z_X 2xaa @ RbPlus
    {   3,   14,  310,  252,    0, } , // 64 pipes (32 PKRs) 2 bpe @ SW_64K_Z_X 2xaa @ RbPlus
    {   3,   15,  310,  446,    0, } , // 64 pipes (32 PKRs) 4 bpe @ SW_64K_Z_X 2xaa @ RbPlus
    {   3,   16,  324,  414,    0, } , // 64 pipes (32 PKRs) 8 bpe @ SW_64K_Z_X 2xaa @ RbPlus
    {   3,   17,  367,  414,    0, } , // 64 pipes (32 PKRs) 16 bpe @ SW_64K_Z_X 2xaa @ RbPlus
};

const ADDR_SW_PATINFO GFX10_SW_64K_Z_X_4xaa_RBPLUS_PATINFO[] =
{
    {   2,   18,  349,  195,    0, } , // 1 pipes (1 PKRs) 1 bpe @ SW_64K_Z_X 4xaa @ RbPlus
    {   3,   19,  349,  447,    0, } , // 1 pipes (1 PKRs) 2 bpe @ SW_64K_Z_X 4xaa @ RbPlus
    {   3,   20,  349,  448,    0, } , // 1 pipes (1 PKRs) 4 bpe @ SW_64K_Z_X 4xaa @ RbPlus
    {   3,   21,  350,  449,    0, } , // 1 pipes (1 PKRs) 8 bpe @ SW_64K_Z_X 4xaa @ RbPlus
    {   3,   22,  351,  450,    0, } , // 1 pipes (1 PKRs) 16 bpe @ SW_64K_Z_X 4xaa @ RbPlus
    {   3,   18,  354,  195,    0, } , // 2 pipes (1-2 PKRs) 1 bpe @ SW_64K_Z_X 4xaa @ RbPlus
    {   3,   19,  368,  451,    0, } , // 2 pipes (1-2 PKRs) 2 bpe @ SW_64K_Z_X 4xaa @ RbPlus
    {   3,   20,  354,  299,    0, } , // 2 pipes (1-2 PKRs) 4 bpe @ SW_64K_Z_X 4xaa @ RbPlus
    {   3,   21,  355,  452,    0, } , // 2 pipes (1-2 PKRs) 8 bpe @ SW_64K_Z_X 4xaa @ RbPlus
    {   3,   22,  356,  453,    0, } , // 2 pipes (1-2 PKRs) 16 bpe @ SW_64K_Z_X 4xaa @ RbPlus
    {   3,   18,  282,  195,    0, } , // 4 pipes (1-2 PKRs) 1 bpe @ SW_64K_Z_X 4xaa @ RbPlus
    {   3,   19,  282,  298,    0, } , // 4 pipes (1-2 PKRs) 2 bpe @ SW_64K_Z_X 4xaa @ RbPlus
    {   3,   20,  282,  299,    0, } , // 4 pipes (1-2 PKRs) 4 bpe @ SW_64K_Z_X 4xaa @ RbPlus
    {   3,   21,  283,  300,    0, } , // 4 pipes (1-2 PKRs) 8 bpe @ SW_64K_Z_X 4xaa @ RbPlus
    {   3,   22,  284,  301,    0, } , // 4 pipes (1-2 PKRs) 16 bpe @ SW_64K_Z_X 4xaa @ RbPlus
    {   3,   18,  287,  372,    0, } , // 8 pipes (2 PKRs) 1 bpe @ SW_64K_Z_X 4xaa @ RbPlus
    {   3,   19,  287,  454,    0, } , // 8 pipes (2 PKRs) 2 bpe @ SW_64K_Z_X 4xaa @ RbPlus
    {   3,   20,  287,  455,    0, } , // 8 pipes (2 PKRs) 4 bpe @ SW_64K_Z_X 4xaa @ RbPlus
    {   3,   21,  288,  456,    0, } , // 8 pipes (2 PKRs) 8 bpe @ SW_64K_Z_X 4xaa @ RbPlus
    {   3,   22,  331,  457,    0, } , // 8 pipes (2 PKRs) 16 bpe @ SW_64K_Z_X 4xaa @ RbPlus
    {   3,   18,  292,  205,    0, } , // 4 pipes (4 PKRs) 1 bpe @ SW_64K_Z_X 4xaa @ RbPlus
    {   3,   19,  292,  306,    0, } , // 4 pipes (4 PKRs) 2 bpe @ SW_64K_Z_X 4xaa @ RbPlus
    {   3,   20,  292,  307,    0, } , // 4 pipes (4 PKRs) 4 bpe @ SW_64K_Z_X 4xaa @ RbPlus
    {   3,   21,  320,  308,    0, } , // 4 pipes (4 PKRs) 8 bpe @ SW_64K_Z_X 4xaa @ RbPlus
    {   3,   22,  321,  309,    0, } , // 4 pipes (4 PKRs) 16 bpe @ SW_64K_Z_X 4xaa @ RbPlus
    {   3,   18,  297,  376,    0, } , // 8 pipes (4 PKRs) 1 bpe @ SW_64K_Z_X 4xaa @ RbPlus
    {   3,   19,  297,  458,    0, } , // 8 pipes (4 PKRs) 2 bpe @ SW_64K_Z_X 4xaa @ RbPlus
    {   3,   20,  297,  459,    0, } , // 8 pipes (4 PKRs) 4 bpe @ SW_64K_Z_X 4xaa @ RbPlus
    {   3,   21,  299,  460,    0, } , // 8 pipes (4 PKRs) 8 bpe @ SW_64K_Z_X 4xaa @ RbPlus
    {   3,   22,  369,  461,    0, } , // 8 pipes (4 PKRs) 16 bpe @ SW_64K_Z_X 4xaa @ RbPlus
    {   3,   18,  300,  381,    0, } , // 16 pipes (4 PKRs) 1 bpe @ SW_64K_Z_X 4xaa @ RbPlus
    {   3,   19,  300,  462,    0, } , // 16 pipes (4 PKRs) 2 bpe @ SW_64K_Z_X 4xaa @ RbPlus
    {   3,   20,  300,  463,    0, } , // 16 pipes (4 PKRs) 4 bpe @ SW_64K_Z_X 4xaa @ RbPlus
    {   3,   21,  363,  464,    0, } , // 16 pipes (4 PKRs) 8 bpe @ SW_64K_Z_X 4xaa @ RbPlus
    {   3,   22,  370,  465,    0, } , // 16 pipes (4 PKRs) 16 bpe @ SW_64K_Z_X 4xaa @ RbPlus
    {   3,   18,  303,  386,    0, } , // 8 pipes (8 PKRs) 1 bpe @ SW_64K_Z_X 4xaa @ RbPlus
    {   3,   19,  303,  466,    0, } , // 8 pipes (8 PKRs) 2 bpe @ SW_64K_Z_X 4xaa @ RbPlus
    {   3,   20,  303,  467,    0, } , // 8 pipes (8 PKRs) 4 bpe @ SW_64K_Z_X 4xaa @ RbPlus
    {   3,   21,  371,  468,    0, } , // 8 pipes (8 PKRs) 8 bpe @ SW_64K_Z_X 4xaa @ RbPlus
    {   3,   22,  337,  469,    0, } , // 8 pipes (8 PKRs) 16 bpe @ SW_64K_Z_X 4xaa @ RbPlus
    {   3,   18,  306,  381,    0, } , // 16 pipes (8 PKRs) 1 bpe @ SW_64K_Z_X 4xaa @ RbPlus
    {   3,   19,  306,  462,    0, } , // 16 pipes (8 PKRs) 2 bpe @ SW_64K_Z_X 4xaa @ RbPlus
    {   3,   20,  306,  470,    0, } , // 16 pipes (8 PKRs) 4 bpe @ SW_64K_Z_X 4xaa @ RbPlus
    {   3,   21,  372,  470,    0, } , // 16 pipes (8 PKRs) 8 bpe @ SW_64K_Z_X 4xaa @ RbPlus
    {   3,   22,  373,  470,    0, } , // 16 pipes (8 PKRs) 16 bpe @ SW_64K_Z_X 4xaa @ RbPlus
    {   3,   18,  306,  393,    0, } , // 32 pipes (8 PKRs) 1 bpe @ SW_64K_Z_X 4xaa @ RbPlus
    {   3,   19,  306,  471,    0, } , // 32 pipes (8 PKRs) 2 bpe @ SW_64K_Z_X 4xaa @ RbPlus
    {   3,   20,  306,  472,    0, } , // 32 pipes (8 PKRs) 4 bpe @ SW_64K_Z_X 4xaa @ RbPlus
    {   3,   21,  372,  472,    0, } , // 32 pipes (8 PKRs) 8 bpe @ SW_64K_Z_X 4xaa @ RbPlus
    {   3,   22,  373,  472,    0, } , // 32 pipes (8 PKRs) 16 bpe @ SW_64K_Z_X 4xaa @ RbPlus
    {   3,   18,  308,  398,    0, } , // 16 pipes (16 PKRs) 1 bpe @ SW_64K_Z_X 4xaa @ RbPlus
    {   3,   19,  308,  473,    0, } , // 16 pipes (16 PKRs) 2 bpe @ SW_64K_Z_X 4xaa @ RbPlus
    {   3,   20,  308,  438,    0, } , // 16 pipes (16 PKRs) 4 bpe @ SW_64K_Z_X 4xaa @ RbPlus
    {   3,   21,  374,  438,    0, } , // 16 pipes (16 PKRs) 8 bpe @ SW_64K_Z_X 4xaa @ RbPlus
    {   3,   22,  375,  438,    0, } , // 16 pipes (16 PKRs) 16 bpe @ SW_64K_Z_X 4xaa @ RbPlus
    {   3,   18,  308,  403,    0, } , // 32 pipes (16 PKRs) 1 bpe @ SW_64K_Z_X 4xaa @ RbPlus
    {   3,   19,  308,  471,    0, } , // 32 pipes (16 PKRs) 2 bpe @ SW_64K_Z_X 4xaa @ RbPlus
    {   3,   20,  308,  439,    0, } , // 32 pipes (16 PKRs) 4 bpe @ SW_64K_Z_X 4xaa @ RbPlus
    {   3,   21,  374,  439,    0, } , // 32 pipes (16 PKRs) 8 bpe @ SW_64K_Z_X 4xaa @ RbPlus
    {   3,   22,  375,  439,    0, } , // 32 pipes (16 PKRs) 16 bpe @ SW_64K_Z_X 4xaa @ RbPlus
    {   3,   18,  308,  242,    0, } , // 64 pipes (16 PKRs) 1 bpe @ SW_64K_Z_X 4xaa @ RbPlus
    {   3,   19,  308,  441,    0, } , // 64 pipes (16 PKRs) 2 bpe @ SW_64K_Z_X 4xaa @ RbPlus
    {   3,   20,  308,  442,    0, } , // 64 pipes (16 PKRs) 4 bpe @ SW_64K_Z_X 4xaa @ RbPlus
    {   3,   21,  374,  442,    0, } , // 64 pipes (16 PKRs) 8 bpe @ SW_64K_Z_X 4xaa @ RbPlus
    {   3,   22,  375,  442,    0, } , // 64 pipes (16 PKRs) 16 bpe @ SW_64K_Z_X 4xaa @ RbPlus
    {   3,   18,  310,  410,    0, } , // 32 pipes (32 PKRs) 1 bpe @ SW_64K_Z_X 4xaa @ RbPlus
    {   3,   19,  310,  474,    0, } , // 32 pipes (32 PKRs) 2 bpe @ SW_64K_Z_X 4xaa @ RbPlus
    {   3,   20,  310,  412,    0, } , // 32 pipes (32 PKRs) 4 bpe @ SW_64K_Z_X 4xaa @ RbPlus
    {   3,   21,  376,  412,    0, } , // 32 pipes (32 PKRs) 8 bpe @ SW_64K_Z_X 4xaa @ RbPlus
    {   3,   22,  377,  412,    0, } , // 32 pipes (32 PKRs) 16 bpe @ SW_64K_Z_X 4xaa @ RbPlus
    {   3,   18,  310,  252,    0, } , // 64 pipes (32 PKRs) 1 bpe @ SW_64K_Z_X 4xaa @ RbPlus
    {   3,   19,  310,  475,    0, } , // 64 pipes (32 PKRs) 2 bpe @ SW_64K_Z_X 4xaa @ RbPlus
    {   3,   20,  310,  414,    0, } , // 64 pipes (32 PKRs) 4 bpe @ SW_64K_Z_X 4xaa @ RbPlus
    {   3,   21,  376,  414,    0, } , // 64 pipes (32 PKRs) 8 bpe @ SW_64K_Z_X 4xaa @ RbPlus
    {   3,   22,  377,  414,    0, } , // 64 pipes (32 PKRs) 16 bpe @ SW_64K_Z_X 4xaa @ RbPlus
};

const ADDR_SW_PATINFO GFX10_SW_64K_Z_X_8xaa_RBPLUS_PATINFO[] =
{
    {   3,   23,  358,  263,    0, } , // 1 pipes (1 PKRs) 1 bpe @ SW_64K_Z_X 8xaa @ RbPlus
    {   3,   24,  349,  448,    0, } , // 1 pipes (1 PKRs) 2 bpe @ SW_64K_Z_X 8xaa @ RbPlus
    {   3,   25,  358,  332,    0, } , // 1 pipes (1 PKRs) 4 bpe @ SW_64K_Z_X 8xaa @ RbPlus
    {   3,   26,  350,  476,    0, } , // 1 pipes (1 PKRs) 8 bpe @ SW_64K_Z_X 8xaa @ RbPlus
    {   3,   27,  359,  477,    0, } , // 1 pipes (1 PKRs) 16 bpe @ SW_64K_Z_X 8xaa @ RbPlus
    {   3,   23,  354,  263,    0, } , // 2 pipes (1-2 PKRs) 1 bpe @ SW_64K_Z_X 8xaa @ RbPlus
    {   3,   24,  354,  299,    0, } , // 2 pipes (1-2 PKRs) 2 bpe @ SW_64K_Z_X 8xaa @ RbPlus
    {   3,   25,  354,  332,    0, } , // 2 pipes (1-2 PKRs) 4 bpe @ SW_64K_Z_X 8xaa @ RbPlus
    {   3,   26,  361,  478,    0, } , // 2 pipes (1-2 PKRs) 8 bpe @ SW_64K_Z_X 8xaa @ RbPlus
    {   3,   27,  378,  479,    0, } , // 2 pipes (1-2 PKRs) 16 bpe @ SW_64K_Z_X 8xaa @ RbPlus
    {   3,   23,  282,  263,    0, } , // 4 pipes (1-2 PKRs) 1 bpe @ SW_64K_Z_X 8xaa @ RbPlus
    {   3,   24,  282,  299,    0, } , // 4 pipes (1-2 PKRs) 2 bpe @ SW_64K_Z_X 8xaa @ RbPlus
    {   3,   25,  282,  332,    0, } , // 4 pipes (1-2 PKRs) 4 bpe @ SW_64K_Z_X 8xaa @ RbPlus
    {   3,   26,  317,  333,    0, } , // 4 pipes (1-2 PKRs) 8 bpe @ SW_64K_Z_X 8xaa @ RbPlus
    {   3,   27,  329,  334,    0, } , // 4 pipes (1-2 PKRs) 16 bpe @ SW_64K_Z_X 8xaa @ RbPlus
    {   3,   23,  287,  421,    0, } , // 8 pipes (2 PKRs) 1 bpe @ SW_64K_Z_X 8xaa @ RbPlus
    {   3,   24,  287,  480,    0, } , // 8 pipes (2 PKRs) 2 bpe @ SW_64K_Z_X 8xaa @ RbPlus
    {   3,   25,  287,  481,    0, } , // 8 pipes (2 PKRs) 4 bpe @ SW_64K_Z_X 8xaa @ RbPlus
    {   3,   26,  379,  482,    0, } , // 8 pipes (2 PKRs) 8 bpe @ SW_64K_Z_X 8xaa @ RbPlus
    {   3,   27,  380,  483,    0, } , // 8 pipes (2 PKRs) 16 bpe @ SW_64K_Z_X 8xaa @ RbPlus
    {   3,   23,  292,  269,    0, } , // 4 pipes (4 PKRs) 1 bpe @ SW_64K_Z_X 8xaa @ RbPlus
    {   3,   24,  292,  307,    0, } , // 4 pipes (4 PKRs) 2 bpe @ SW_64K_Z_X 8xaa @ RbPlus
    {   3,   25,  292,  339,    0, } , // 4 pipes (4 PKRs) 4 bpe @ SW_64K_Z_X 8xaa @ RbPlus
    {   3,   26,  332,  340,    0, } , // 4 pipes (4 PKRs) 8 bpe @ SW_64K_Z_X 8xaa @ RbPlus
    {   3,   27,  333,  341,    0, } , // 4 pipes (4 PKRs) 16 bpe @ SW_64K_Z_X 8xaa @ RbPlus
    {   3,   23,  297,  421,    0, } , // 8 pipes (4 PKRs) 1 bpe @ SW_64K_Z_X 8xaa @ RbPlus
    {   3,   24,  297,  459,    0, } , // 8 pipes (4 PKRs) 2 bpe @ SW_64K_Z_X 8xaa @ RbPlus
    {   3,   25,  297,  481,    0, } , // 8 pipes (4 PKRs) 4 bpe @ SW_64K_Z_X 8xaa @ RbPlus
    {   3,   26,  381,  484,    0, } , // 8 pipes (4 PKRs) 8 bpe @ SW_64K_Z_X 8xaa @ RbPlus
    {   3,   27,  382,  485,    0, } , // 8 pipes (4 PKRs) 16 bpe @ SW_64K_Z_X 8xaa @ RbPlus
    {   3,   23,  300,  434,    0, } , // 16 pipes (4 PKRs) 1 bpe @ SW_64K_Z_X 8xaa @ RbPlus
    {   3,   24,  300,  463,    0, } , // 16 pipes (4 PKRs) 2 bpe @ SW_64K_Z_X 8xaa @ RbPlus
    {   3,   25,  383,  486,    0, } , // 16 pipes (4 PKRs) 4 bpe @ SW_64K_Z_X 8xaa @ RbPlus
    {   3,   26,  384,  487,    0, } , // 16 pipes (4 PKRs) 8 bpe @ SW_64K_Z_X 8xaa @ RbPlus
    {   3,   27,  385,  488,    0, } , // 16 pipes (4 PKRs) 16 bpe @ SW_64K_Z_X 8xaa @ RbPlus
    {   3,   23,  303,  431,    0, } , // 8 pipes (8 PKRs) 1 bpe @ SW_64K_Z_X 8xaa @ RbPlus
    {   3,   24,  303,  467,    0, } , // 8 pipes (8 PKRs) 2 bpe @ SW_64K_Z_X 8xaa @ RbPlus
    {   3,   25,  303,  489,    0, } , // 8 pipes (8 PKRs) 4 bpe @ SW_64K_Z_X 8xaa @ RbPlus
    {   3,   26,  337,  469,    0, } , // 8 pipes (8 PKRs) 8 bpe @ SW_64K_Z_X 8xaa @ RbPlus
    {   3,   27,  386,  469,    0, } , // 8 pipes (8 PKRs) 16 bpe @ SW_64K_Z_X 8xaa @ RbPlus
    {   3,   23,  306,  434,    0, } , // 16 pipes (8 PKRs) 1 bpe @ SW_64K_Z_X 8xaa @ RbPlus
    {   3,   24,  306,  470,    0, } , // 16 pipes (8 PKRs) 2 bpe @ SW_64K_Z_X 8xaa @ RbPlus
    {   3,   25,  387,  490,    0, } , // 16 pipes (8 PKRs) 4 bpe @ SW_64K_Z_X 8xaa @ RbPlus
    {   3,   26,  373,  470,    0, } , // 16 pipes (8 PKRs) 8 bpe @ SW_64K_Z_X 8xaa @ RbPlus
    {   3,   27,  388,  470,    0, } , // 16 pipes (8 PKRs) 16 bpe @ SW_64K_Z_X 8xaa @ RbPlus
    {   3,   23,  306,  436,    0, } , // 32 pipes (8 PKRs) 1 bpe @ SW_64K_Z_X 8xaa @ RbPlus
    {   3,   24,  306,  472,    0, } , // 32 pipes (8 PKRs) 2 bpe @ SW_64K_Z_X 8xaa @ RbPlus
    {   3,   25,  387,  491,    0, } , // 32 pipes (8 PKRs) 4 bpe @ SW_64K_Z_X 8xaa @ RbPlus
    {   3,   26,  373,  472,    0, } , // 32 pipes (8 PKRs) 8 bpe @ SW_64K_Z_X 8xaa @ RbPlus
    {   3,   27,  388,  492,    0, } , // 32 pipes (8 PKRs) 16 bpe @ SW_64K_Z_X 8xaa @ RbPlus
    {   3,   23,  308,  437,    0, } , // 16 pipes (16 PKRs) 1 bpe @ SW_64K_Z_X 8xaa @ RbPlus
    {   3,   24,  308,  438,    0, } , // 16 pipes (16 PKRs) 2 bpe @ SW_64K_Z_X 8xaa @ RbPlus
    {   3,   25,  389,  493,    0, } , // 16 pipes (16 PKRs) 4 bpe @ SW_64K_Z_X 8xaa @ RbPlus
    {   3,   26,  375,  438,    0, } , // 16 pipes (16 PKRs) 8 bpe @ SW_64K_Z_X 8xaa @ RbPlus
    {   3,   27,  390,  438,    0, } , // 16 pipes (16 PKRs) 16 bpe @ SW_64K_Z_X 8xaa @ RbPlus
    {   3,   23,  308,  436,    0, } , // 32 pipes (16 PKRs) 1 bpe @ SW_64K_Z_X 8xaa @ RbPlus
    {   3,   24,  308,  439,    0, } , // 32 pipes (16 PKRs) 2 bpe @ SW_64K_Z_X 8xaa @ RbPlus
    {   3,   25,  391,  494,    0, } , // 32 pipes (16 PKRs) 4 bpe @ SW_64K_Z_X 8xaa @ RbPlus
    {   3,   26,  375,  439,    0, } , // 32 pipes (16 PKRs) 8 bpe @ SW_64K_Z_X 8xaa @ RbPlus
    {   3,   27,  390,  439,    0, } , // 32 pipes (16 PKRs) 16 bpe @ SW_64K_Z_X 8xaa @ RbPlus
    {   3,   23,  308,  441,    0, } , // 64 pipes (16 PKRs) 1 bpe @ SW_64K_Z_X 8xaa @ RbPlus
    {   3,   24,  308,  442,    0, } , // 64 pipes (16 PKRs) 2 bpe @ SW_64K_Z_X 8xaa @ RbPlus
    {   3,   25,  391,  495,    0, } , // 64 pipes (16 PKRs) 4 bpe @ SW_64K_Z_X 8xaa @ RbPlus
    {   3,   26,  375,  442,    0, } , // 64 pipes (16 PKRs) 8 bpe @ SW_64K_Z_X 8xaa @ RbPlus
    {   3,   27,  390,  442,    0, } , // 64 pipes (16 PKRs) 16 bpe @ SW_64K_Z_X 8xaa @ RbPlus
    {   3,   23,  310,  444,    0, } , // 32 pipes (32 PKRs) 1 bpe @ SW_64K_Z_X 8xaa @ RbPlus
    {   3,   24,  310,  412,    0, } , // 32 pipes (32 PKRs) 2 bpe @ SW_64K_Z_X 8xaa @ RbPlus
    {   3,   25,  392,  496,    0, } , // 32 pipes (32 PKRs) 4 bpe @ SW_64K_Z_X 8xaa @ RbPlus
    {   3,   26,  377,  412,    0, } , // 32 pipes (32 PKRs) 8 bpe @ SW_64K_Z_X 8xaa @ RbPlus
    {   3,   27,  393,  412,    0, } , // 32 pipes (32 PKRs) 16 bpe @ SW_64K_Z_X 8xaa @ RbPlus
    {   3,   23,  310,  446,    0, } , // 64 pipes (32 PKRs) 1 bpe @ SW_64K_Z_X 8xaa @ RbPlus
    {   3,   24,  310,  414,    0, } , // 64 pipes (32 PKRs) 2 bpe @ SW_64K_Z_X 8xaa @ RbPlus
    {   3,   25,  367,  414,    0, } , // 64 pipes (32 PKRs) 4 bpe @ SW_64K_Z_X 8xaa @ RbPlus
    {   3,   26,  377,  414,    0, } , // 64 pipes (32 PKRs) 8 bpe @ SW_64K_Z_X 8xaa @ RbPlus
    {   3,   27,  393,  414,    0, } , // 64 pipes (32 PKRs) 16 bpe @ SW_64K_Z_X 8xaa @ RbPlus
};

const ADDR_SW_PATINFO GFX10_SW_64K_S3_RBPLUS_PATINFO[] =
{
    {   1,   29,  131,  148,    0, } , // 1 pipes (1 PKRs) 1 bpe @ SW_64K_S3 @ RbPlus
    {   1,   30,  132,  149,    0, } , // 1 pipes (1 PKRs) 2 bpe @ SW_64K_S3 @ RbPlus
    {   1,   31,  133,  150,    0, } , // 1 pipes (1 PKRs) 4 bpe @ SW_64K_S3 @ RbPlus
    {   1,   32,  134,  151,    0, } , // 1 pipes (1 PKRs) 8 bpe @ SW_64K_S3 @ RbPlus
    {   1,   33,  135,  152,    0, } , // 1 pipes (1 PKRs) 16 bpe @ SW_64K_S3 @ RbPlus
    {   1,   29,  131,  148,    0, } , // 2 pipes (1-2 PKRs) 1 bpe @ SW_64K_S3 @ RbPlus
    {   1,   30,  132,  149,    0, } , // 2 pipes (1-2 PKRs) 2 bpe @ SW_64K_S3 @ RbPlus
    {   1,   31,  133,  150,    0, } , // 2 pipes (1-2 PKRs) 4 bpe @ SW_64K_S3 @ RbPlus
    {   1,   32,  134,  151,    0, } , // 2 pipes (1-2 PKRs) 8 bpe @ SW_64K_S3 @ RbPlus
    {   1,   33,  135,  152,    0, } , // 2 pipes (1-2 PKRs) 16 bpe @ SW_64K_S3 @ RbPlus
    {   1,   29,  131,  148,    0, } , // 4 pipes (1-2 PKRs) 1 bpe @ SW_64K_S3 @ RbPlus
    {   1,   30,  132,  149,    0, } , // 4 pipes (1-2 PKRs) 2 bpe @ SW_64K_S3 @ RbPlus
    {   1,   31,  133,  150,    0, } , // 4 pipes (1-2 PKRs) 4 bpe @ SW_64K_S3 @ RbPlus
    {   1,   32,  134,  151,    0, } , // 4 pipes (1-2 PKRs) 8 bpe @ SW_64K_S3 @ RbPlus
    {   1,   33,  135,  152,    0, } , // 4 pipes (1-2 PKRs) 16 bpe @ SW_64K_S3 @ RbPlus
    {   1,   29,  131,  148,    0, } , // 8 pipes (2 PKRs) 1 bpe @ SW_64K_S3 @ RbPlus
    {   1,   30,  132,  149,    0, } , // 8 pipes (2 PKRs) 2 bpe @ SW_64K_S3 @ RbPlus
    {   1,   31,  133,  150,    0, } , // 8 pipes (2 PKRs) 4 bpe @ SW_64K_S3 @ RbPlus
    {   1,   32,  134,  151,    0, } , // 8 pipes (2 PKRs) 8 bpe @ SW_64K_S3 @ RbPlus
    {   1,   33,  135,  152,    0, } , // 8 pipes (2 PKRs) 16 bpe @ SW_64K_S3 @ RbPlus
    {   1,   29,  131,  148,    0, } , // 4 pipes (4 PKRs) 1 bpe @ SW_64K_S3 @ RbPlus
    {   1,   30,  132,  149,    0, } , // 4 pipes (4 PKRs) 2 bpe @ SW_64K_S3 @ RbPlus
    {   1,   31,  133,  150,    0, } , // 4 pipes (4 PKRs) 4 bpe @ SW_64K_S3 @ RbPlus
    {   1,   32,  134,  151,    0, } , // 4 pipes (4 PKRs) 8 bpe @ SW_64K_S3 @ RbPlus
    {   1,   33,  135,  152,    0, } , // 4 pipes (4 PKRs) 16 bpe @ SW_64K_S3 @ RbPlus
    {   1,   29,  131,  148,    0, } , // 8 pipes (4 PKRs) 1 bpe @ SW_64K_S3 @ RbPlus
    {   1,   30,  132,  149,    0, } , // 8 pipes (4 PKRs) 2 bpe @ SW_64K_S3 @ RbPlus
    {   1,   31,  133,  150,    0, } , // 8 pipes (4 PKRs) 4 bpe @ SW_64K_S3 @ RbPlus
    {   1,   32,  134,  151,    0, } , // 8 pipes (4 PKRs) 8 bpe @ SW_64K_S3 @ RbPlus
    {   1,   33,  135,  152,    0, } , // 8 pipes (4 PKRs) 16 bpe @ SW_64K_S3 @ RbPlus
    {   1,   29,  131,  148,    0, } , // 16 pipes (4 PKRs) 1 bpe @ SW_64K_S3 @ RbPlus
    {   1,   30,  132,  149,    0, } , // 16 pipes (4 PKRs) 2 bpe @ SW_64K_S3 @ RbPlus
    {   1,   31,  133,  150,    0, } , // 16 pipes (4 PKRs) 4 bpe @ SW_64K_S3 @ RbPlus
    {   1,   32,  134,  151,    0, } , // 16 pipes (4 PKRs) 8 bpe @ SW_64K_S3 @ RbPlus
    {   1,   33,  135,  152,    0, } , // 16 pipes (4 PKRs) 16 bpe @ SW_64K_S3 @ RbPlus
    {   1,   29,  131,  148,    0, } , // 8 pipes (8 PKRs) 1 bpe @ SW_64K_S3 @ RbPlus
    {   1,   30,  132,  149,    0, } , // 8 pipes (8 PKRs) 2 bpe @ SW_64K_S3 @ RbPlus
    {   1,   31,  133,  150,    0, } , // 8 pipes (8 PKRs) 4 bpe @ SW_64K_S3 @ RbPlus
    {   1,   32,  134,  151,    0, } , // 8 pipes (8 PKRs) 8 bpe @ SW_64K_S3 @ RbPlus
    {   1,   33,  135,  152,    0, } , // 8 pipes (8 PKRs) 16 bpe @ SW_64K_S3 @ RbPlus
    {   1,   29,  131,  148,    0, } , // 16 pipes (8 PKRs) 1 bpe @ SW_64K_S3 @ RbPlus
    {   1,   30,  132,  149,    0, } , // 16 pipes (8 PKRs) 2 bpe @ SW_64K_S3 @ RbPlus
    {   1,   31,  133,  150,    0, } , // 16 pipes (8 PKRs) 4 bpe @ SW_64K_S3 @ RbPlus
    {   1,   32,  134,  151,    0, } , // 16 pipes (8 PKRs) 8 bpe @ SW_64K_S3 @ RbPlus
    {   1,   33,  135,  152,    0, } , // 16 pipes (8 PKRs) 16 bpe @ SW_64K_S3 @ RbPlus
    {   1,   29,  131,  148,    0, } , // 32 pipes (8 PKRs) 1 bpe @ SW_64K_S3 @ RbPlus
    {   1,   30,  132,  149,    0, } , // 32 pipes (8 PKRs) 2 bpe @ SW_64K_S3 @ RbPlus
    {   1,   31,  133,  150,    0, } , // 32 pipes (8 PKRs) 4 bpe @ SW_64K_S3 @ RbPlus
    {   1,   32,  134,  151,    0, } , // 32 pipes (8 PKRs) 8 bpe @ SW_64K_S3 @ RbPlus
    {   1,   33,  135,  152,    0, } , // 32 pipes (8 PKRs) 16 bpe @ SW_64K_S3 @ RbPlus
    {   1,   29,  131,  148,    0, } , // 16 pipes (16 PKRs) 1 bpe @ SW_64K_S3 @ RbPlus
    {   1,   30,  132,  149,    0, } , // 16 pipes (16 PKRs) 2 bpe @ SW_64K_S3 @ RbPlus
    {   1,   31,  133,  150,    0, } , // 16 pipes (16 PKRs) 4 bpe @ SW_64K_S3 @ RbPlus
    {   1,   32,  134,  151,    0, } , // 16 pipes (16 PKRs) 8 bpe @ SW_64K_S3 @ RbPlus
    {   1,   33,  135,  152,    0, } , // 16 pipes (16 PKRs) 16 bpe @ SW_64K_S3 @ RbPlus
    {   1,   29,  131,  148,    0, } , // 32 pipes (16 PKRs) 1 bpe @ SW_64K_S3 @ RbPlus
    {   1,   30,  132,  149,    0, } , // 32 pipes (16 PKRs) 2 bpe @ SW_64K_S3 @ RbPlus
    {   1,   31,  133,  150,    0, } , // 32 pipes (16 PKRs) 4 bpe @ SW_64K_S3 @ RbPlus
    {   1,   32,  134,  151,    0, } , // 32 pipes (16 PKRs) 8 bpe @ SW_64K_S3 @ RbPlus
    {   1,   33,  135,  152,    0, } , // 32 pipes (16 PKRs) 16 bpe @ SW_64K_S3 @ RbPlus
    {   1,   29,  131,  148,    0, } , // 64 pipes (16 PKRs) 1 bpe @ SW_64K_S3 @ RbPlus
    {   1,   30,  132,  149,    0, } , // 64 pipes (16 PKRs) 2 bpe @ SW_64K_S3 @ RbPlus
    {   1,   31,  133,  150,    0, } , // 64 pipes (16 PKRs) 4 bpe @ SW_64K_S3 @ RbPlus
    {   1,   32,  134,  151,    0, } , // 64 pipes (16 PKRs) 8 bpe @ SW_64K_S3 @ RbPlus
    {   1,   33,  135,  152,    0, } , // 64 pipes (16 PKRs) 16 bpe @ SW_64K_S3 @ RbPlus
    {   1,   29,  131,  148,    0, } , // 32 pipes (32 PKRs) 1 bpe @ SW_64K_S3 @ RbPlus
    {   1,   30,  132,  149,    0, } , // 32 pipes (32 PKRs) 2 bpe @ SW_64K_S3 @ RbPlus
    {   1,   31,  133,  150,    0, } , // 32 pipes (32 PKRs) 4 bpe @ SW_64K_S3 @ RbPlus
    {   1,   32,  134,  151,    0, } , // 32 pipes (32 PKRs) 8 bpe @ SW_64K_S3 @ RbPlus
    {   1,   33,  135,  152,    0, } , // 32 pipes (32 PKRs) 16 bpe @ SW_64K_S3 @ RbPlus
    {   1,   29,  131,  148,    0, } , // 64 pipes (32 PKRs) 1 bpe @ SW_64K_S3 @ RbPlus
    {   1,   30,  132,  149,    0, } , // 64 pipes (32 PKRs) 2 bpe @ SW_64K_S3 @ RbPlus
    {   1,   31,  133,  150,    0, } , // 64 pipes (32 PKRs) 4 bpe @ SW_64K_S3 @ RbPlus
    {   1,   32,  134,  151,    0, } , // 64 pipes (32 PKRs) 8 bpe @ SW_64K_S3 @ RbPlus
    {   1,   33,  135,  152,    0, } , // 64 pipes (32 PKRs) 16 bpe @ SW_64K_S3 @ RbPlus
};

const ADDR_SW_PATINFO GFX10_SW_64K_S3_X_RBPLUS_PATINFO[] =
{
    {   1,   29,  131,  148,    0, } , // 1 pipes (1 PKRs) 1 bpe @ SW_64K_S3_X @ RbPlus
    {   1,   30,  132,  149,    0, } , // 1 pipes (1 PKRs) 2 bpe @ SW_64K_S3_X @ RbPlus
    {   1,   31,  133,  150,    0, } , // 1 pipes (1 PKRs) 4 bpe @ SW_64K_S3_X @ RbPlus
    {   1,   32,  134,  151,    0, } , // 1 pipes (1 PKRs) 8 bpe @ SW_64K_S3_X @ RbPlus
    {   1,   33,  135,  152,    0, } , // 1 pipes (1 PKRs) 16 bpe @ SW_64K_S3_X @ RbPlus
    {   3,   29,  136,  148,    0, } , // 2 pipes (1-2 PKRs) 1 bpe @ SW_64K_S3_X @ RbPlus
    {   3,   30,  137,  149,    0, } , // 2 pipes (1-2 PKRs) 2 bpe @ SW_64K_S3_X @ RbPlus
    {   3,   31,  138,  150,    0, } , // 2 pipes (1-2 PKRs) 4 bpe @ SW_64K_S3_X @ RbPlus
    {   3,   32,  139,  151,    0, } , // 2 pipes (1-2 PKRs) 8 bpe @ SW_64K_S3_X @ RbPlus
    {   3,   33,  140,  152,    0, } , // 2 pipes (1-2 PKRs) 16 bpe @ SW_64K_S3_X @ RbPlus
    {   3,   29,  141,  148,    0, } , // 4 pipes (1-2 PKRs) 1 bpe @ SW_64K_S3_X @ RbPlus
    {   3,   30,  142,  149,    0, } , // 4 pipes (1-2 PKRs) 2 bpe @ SW_64K_S3_X @ RbPlus
    {   3,   31,  143,  150,    0, } , // 4 pipes (1-2 PKRs) 4 bpe @ SW_64K_S3_X @ RbPlus
    {   3,   32,  144,  151,    0, } , // 4 pipes (1-2 PKRs) 8 bpe @ SW_64K_S3_X @ RbPlus
    {   3,   33,  145,  152,    0, } , // 4 pipes (1-2 PKRs) 16 bpe @ SW_64K_S3_X @ RbPlus
    {   3,   29,  146,  148,    0, } , // 8 pipes (2 PKRs) 1 bpe @ SW_64K_S3_X @ RbPlus
    {   3,   30,  147,  149,    0, } , // 8 pipes (2 PKRs) 2 bpe @ SW_64K_S3_X @ RbPlus
    {   3,   31,  148,  150,    0, } , // 8 pipes (2 PKRs) 4 bpe @ SW_64K_S3_X @ RbPlus
    {   3,   32,  149,  151,    0, } , // 8 pipes (2 PKRs) 8 bpe @ SW_64K_S3_X @ RbPlus
    {   3,   33,  150,  152,    0, } , // 8 pipes (2 PKRs) 16 bpe @ SW_64K_S3_X @ RbPlus
    {   3,   29,  141,  148,    0, } , // 4 pipes (4 PKRs) 1 bpe @ SW_64K_S3_X @ RbPlus
    {   3,   30,  142,  149,    0, } , // 4 pipes (4 PKRs) 2 bpe @ SW_64K_S3_X @ RbPlus
    {   3,   31,  143,  150,    0, } , // 4 pipes (4 PKRs) 4 bpe @ SW_64K_S3_X @ RbPlus
    {   3,   32,  144,  151,    0, } , // 4 pipes (4 PKRs) 8 bpe @ SW_64K_S3_X @ RbPlus
    {   3,   33,  145,  152,    0, } , // 4 pipes (4 PKRs) 16 bpe @ SW_64K_S3_X @ RbPlus
    {   3,   29,  146,  148,    0, } , // 8 pipes (4 PKRs) 1 bpe @ SW_64K_S3_X @ RbPlus
    {   3,   30,  147,  149,    0, } , // 8 pipes (4 PKRs) 2 bpe @ SW_64K_S3_X @ RbPlus
    {   3,   31,  148,  150,    0, } , // 8 pipes (4 PKRs) 4 bpe @ SW_64K_S3_X @ RbPlus
    {   3,   32,  149,  151,    0, } , // 8 pipes (4 PKRs) 8 bpe @ SW_64K_S3_X @ RbPlus
    {   3,   33,  150,  152,    0, } , // 8 pipes (4 PKRs) 16 bpe @ SW_64K_S3_X @ RbPlus
    {   3,   29,  151,  148,    0, } , // 16 pipes (4 PKRs) 1 bpe @ SW_64K_S3_X @ RbPlus
    {   3,   30,  152,  149,    0, } , // 16 pipes (4 PKRs) 2 bpe @ SW_64K_S3_X @ RbPlus
    {   3,   31,  153,  150,    0, } , // 16 pipes (4 PKRs) 4 bpe @ SW_64K_S3_X @ RbPlus
    {   3,   32,  154,  151,    0, } , // 16 pipes (4 PKRs) 8 bpe @ SW_64K_S3_X @ RbPlus
    {   3,   33,  155,  152,    0, } , // 16 pipes (4 PKRs) 16 bpe @ SW_64K_S3_X @ RbPlus
    {   3,   29,  146,  148,    0, } , // 8 pipes (8 PKRs) 1 bpe @ SW_64K_S3_X @ RbPlus
    {   3,   30,  147,  149,    0, } , // 8 pipes (8 PKRs) 2 bpe @ SW_64K_S3_X @ RbPlus
    {   3,   31,  148,  150,    0, } , // 8 pipes (8 PKRs) 4 bpe @ SW_64K_S3_X @ RbPlus
    {   3,   32,  149,  151,    0, } , // 8 pipes (8 PKRs) 8 bpe @ SW_64K_S3_X @ RbPlus
    {   3,   33,  150,  152,    0, } , // 8 pipes (8 PKRs) 16 bpe @ SW_64K_S3_X @ RbPlus
    {   3,   29,  151,  148,    0, } , // 16 pipes (8 PKRs) 1 bpe @ SW_64K_S3_X @ RbPlus
    {   3,   30,  152,  149,    0, } , // 16 pipes (8 PKRs) 2 bpe @ SW_64K_S3_X @ RbPlus
    {   3,   31,  153,  150,    0, } , // 16 pipes (8 PKRs) 4 bpe @ SW_64K_S3_X @ RbPlus
    {   3,   32,  154,  151,    0, } , // 16 pipes (8 PKRs) 8 bpe @ SW_64K_S3_X @ RbPlus
    {   3,   33,  155,  152,    0, } , // 16 pipes (8 PKRs) 16 bpe @ SW_64K_S3_X @ RbPlus
    {   3,   29,  156,  153,    0, } , // 32 pipes (8 PKRs) 1 bpe @ SW_64K_S3_X @ RbPlus
    {   3,   30,  157,  154,    0, } , // 32 pipes (8 PKRs) 2 bpe @ SW_64K_S3_X @ RbPlus
    {   3,   31,  158,  155,    0, } , // 32 pipes (8 PKRs) 4 bpe @ SW_64K_S3_X @ RbPlus
    {   3,   32,  159,  156,    0, } , // 32 pipes (8 PKRs) 8 bpe @ SW_64K_S3_X @ RbPlus
    {   3,   33,  160,  157,    0, } , // 32 pipes (8 PKRs) 16 bpe @ SW_64K_S3_X @ RbPlus
    {   3,   29,  151,  148,    0, } , // 16 pipes (16 PKRs) 1 bpe @ SW_64K_S3_X @ RbPlus
    {   3,   30,  152,  149,    0, } , // 16 pipes (16 PKRs) 2 bpe @ SW_64K_S3_X @ RbPlus
    {   3,   31,  153,  150,    0, } , // 16 pipes (16 PKRs) 4 bpe @ SW_64K_S3_X @ RbPlus
    {   3,   32,  154,  151,    0, } , // 16 pipes (16 PKRs) 8 bpe @ SW_64K_S3_X @ RbPlus
    {   3,   33,  155,  152,    0, } , // 16 pipes (16 PKRs) 16 bpe @ SW_64K_S3_X @ RbPlus
    {   3,   29,  156,  153,    0, } , // 32 pipes (16 PKRs) 1 bpe @ SW_64K_S3_X @ RbPlus
    {   3,   30,  157,  154,    0, } , // 32 pipes (16 PKRs) 2 bpe @ SW_64K_S3_X @ RbPlus
    {   3,   31,  158,  155,    0, } , // 32 pipes (16 PKRs) 4 bpe @ SW_64K_S3_X @ RbPlus
    {   3,   32,  159,  156,    0, } , // 32 pipes (16 PKRs) 8 bpe @ SW_64K_S3_X @ RbPlus
    {   3,   33,  160,  157,    0, } , // 32 pipes (16 PKRs) 16 bpe @ SW_64K_S3_X @ RbPlus
    {   3,   29,  161,  158,    0, } , // 64 pipes (16 PKRs) 1 bpe @ SW_64K_S3_X @ RbPlus
    {   3,   30,  162,  159,    0, } , // 64 pipes (16 PKRs) 2 bpe @ SW_64K_S3_X @ RbPlus
    {   3,   31,  163,  160,    0, } , // 64 pipes (16 PKRs) 4 bpe @ SW_64K_S3_X @ RbPlus
    {   3,   32,  164,  161,    0, } , // 64 pipes (16 PKRs) 8 bpe @ SW_64K_S3_X @ RbPlus
    {   3,   33,  165,  162,    0, } , // 64 pipes (16 PKRs) 16 bpe @ SW_64K_S3_X @ RbPlus
    {   3,   29,  156,  153,    0, } , // 32 pipes (32 PKRs) 1 bpe @ SW_64K_S3_X @ RbPlus
    {   3,   30,  157,  154,    0, } , // 32 pipes (32 PKRs) 2 bpe @ SW_64K_S3_X @ RbPlus
    {   3,   31,  158,  155,    0, } , // 32 pipes (32 PKRs) 4 bpe @ SW_64K_S3_X @ RbPlus
    {   3,   32,  159,  156,    0, } , // 32 pipes (32 PKRs) 8 bpe @ SW_64K_S3_X @ RbPlus
    {   3,   33,  160,  157,    0, } , // 32 pipes (32 PKRs) 16 bpe @ SW_64K_S3_X @ RbPlus
    {   3,   29,  161,  158,    0, } , // 64 pipes (32 PKRs) 1 bpe @ SW_64K_S3_X @ RbPlus
    {   3,   30,  162,  159,    0, } , // 64 pipes (32 PKRs) 2 bpe @ SW_64K_S3_X @ RbPlus
    {   3,   31,  163,  160,    0, } , // 64 pipes (32 PKRs) 4 bpe @ SW_64K_S3_X @ RbPlus
    {   3,   32,  164,  161,    0, } , // 64 pipes (32 PKRs) 8 bpe @ SW_64K_S3_X @ RbPlus
    {   3,   33,  165,  162,    0, } , // 64 pipes (32 PKRs) 16 bpe @ SW_64K_S3_X @ RbPlus
};

const ADDR_SW_PATINFO GFX10_SW_64K_S3_T_RBPLUS_PATINFO[] =
{
    {   1,   29,  131,  148,    0, } , // 1 pipes (1 PKRs) 1 bpe @ SW_64K_S3_T @ RbPlus
    {   1,   30,  132,  149,    0, } , // 1 pipes (1 PKRs) 2 bpe @ SW_64K_S3_T @ RbPlus
    {   1,   31,  133,  150,    0, } , // 1 pipes (1 PKRs) 4 bpe @ SW_64K_S3_T @ RbPlus
    {   1,   32,  134,  151,    0, } , // 1 pipes (1 PKRs) 8 bpe @ SW_64K_S3_T @ RbPlus
    {   1,   33,  135,  152,    0, } , // 1 pipes (1 PKRs) 16 bpe @ SW_64K_S3_T @ RbPlus
    {   3,   29,  136,  148,    0, } , // 2 pipes (1-2 PKRs) 1 bpe @ SW_64K_S3_T @ RbPlus
    {   3,   30,  137,  149,    0, } , // 2 pipes (1-2 PKRs) 2 bpe @ SW_64K_S3_T @ RbPlus
    {   3,   31,  138,  150,    0, } , // 2 pipes (1-2 PKRs) 4 bpe @ SW_64K_S3_T @ RbPlus
    {   3,   32,  139,  151,    0, } , // 2 pipes (1-2 PKRs) 8 bpe @ SW_64K_S3_T @ RbPlus
    {   3,   33,  140,  152,    0, } , // 2 pipes (1-2 PKRs) 16 bpe @ SW_64K_S3_T @ RbPlus
    {   3,   29,  141,  148,    0, } , // 4 pipes (1-2 PKRs) 1 bpe @ SW_64K_S3_T @ RbPlus
    {   3,   30,  142,  149,    0, } , // 4 pipes (1-2 PKRs) 2 bpe @ SW_64K_S3_T @ RbPlus
    {   3,   31,  143,  150,    0, } , // 4 pipes (1-2 PKRs) 4 bpe @ SW_64K_S3_T @ RbPlus
    {   3,   32,  144,  151,    0, } , // 4 pipes (1-2 PKRs) 8 bpe @ SW_64K_S3_T @ RbPlus
    {   3,   33,  145,  152,    0, } , // 4 pipes (1-2 PKRs) 16 bpe @ SW_64K_S3_T @ RbPlus
    {   3,   29,  166,  148,    0, } , // 8 pipes (2 PKRs) 1 bpe @ SW_64K_S3_T @ RbPlus
    {   3,   30,  167,  149,    0, } , // 8 pipes (2 PKRs) 2 bpe @ SW_64K_S3_T @ RbPlus
    {   3,   31,  168,  150,    0, } , // 8 pipes (2 PKRs) 4 bpe @ SW_64K_S3_T @ RbPlus
    {   3,   32,  169,  151,    0, } , // 8 pipes (2 PKRs) 8 bpe @ SW_64K_S3_T @ RbPlus
    {   3,   33,  170,  152,    0, } , // 8 pipes (2 PKRs) 16 bpe @ SW_64K_S3_T @ RbPlus
    {   3,   29,  141,  148,    0, } , // 4 pipes (4 PKRs) 1 bpe @ SW_64K_S3_T @ RbPlus
    {   3,   30,  142,  149,    0, } , // 4 pipes (4 PKRs) 2 bpe @ SW_64K_S3_T @ RbPlus
    {   3,   31,  143,  150,    0, } , // 4 pipes (4 PKRs) 4 bpe @ SW_64K_S3_T @ RbPlus
    {   3,   32,  144,  151,    0, } , // 4 pipes (4 PKRs) 8 bpe @ SW_64K_S3_T @ RbPlus
    {   3,   33,  145,  152,    0, } , // 4 pipes (4 PKRs) 16 bpe @ SW_64K_S3_T @ RbPlus
    {   3,   29,  166,  148,    0, } , // 8 pipes (4 PKRs) 1 bpe @ SW_64K_S3_T @ RbPlus
    {   3,   30,  167,  149,    0, } , // 8 pipes (4 PKRs) 2 bpe @ SW_64K_S3_T @ RbPlus
    {   3,   31,  168,  150,    0, } , // 8 pipes (4 PKRs) 4 bpe @ SW_64K_S3_T @ RbPlus
    {   3,   32,  169,  151,    0, } , // 8 pipes (4 PKRs) 8 bpe @ SW_64K_S3_T @ RbPlus
    {   3,   33,  170,  152,    0, } , // 8 pipes (4 PKRs) 16 bpe @ SW_64K_S3_T @ RbPlus
    {   3,   29,  171,  148,    0, } , // 16 pipes (4 PKRs) 1 bpe @ SW_64K_S3_T @ RbPlus
    {   3,   30,  172,  149,    0, } , // 16 pipes (4 PKRs) 2 bpe @ SW_64K_S3_T @ RbPlus
    {   3,   31,  173,  150,    0, } , // 16 pipes (4 PKRs) 4 bpe @ SW_64K_S3_T @ RbPlus
    {   3,   32,  174,  151,    0, } , // 16 pipes (4 PKRs) 8 bpe @ SW_64K_S3_T @ RbPlus
    {   3,   33,  175,  152,    0, } , // 16 pipes (4 PKRs) 16 bpe @ SW_64K_S3_T @ RbPlus
    {   3,   29,  166,  148,    0, } , // 8 pipes (8 PKRs) 1 bpe @ SW_64K_S3_T @ RbPlus
    {   3,   30,  167,  149,    0, } , // 8 pipes (8 PKRs) 2 bpe @ SW_64K_S3_T @ RbPlus
    {   3,   31,  168,  150,    0, } , // 8 pipes (8 PKRs) 4 bpe @ SW_64K_S3_T @ RbPlus
    {   3,   32,  169,  151,    0, } , // 8 pipes (8 PKRs) 8 bpe @ SW_64K_S3_T @ RbPlus
    {   3,   33,  170,  152,    0, } , // 8 pipes (8 PKRs) 16 bpe @ SW_64K_S3_T @ RbPlus
    {   3,   29,  171,  148,    0, } , // 16 pipes (8 PKRs) 1 bpe @ SW_64K_S3_T @ RbPlus
    {   3,   30,  172,  149,    0, } , // 16 pipes (8 PKRs) 2 bpe @ SW_64K_S3_T @ RbPlus
    {   3,   31,  173,  150,    0, } , // 16 pipes (8 PKRs) 4 bpe @ SW_64K_S3_T @ RbPlus
    {   3,   32,  174,  151,    0, } , // 16 pipes (8 PKRs) 8 bpe @ SW_64K_S3_T @ RbPlus
    {   3,   33,  175,  152,    0, } , // 16 pipes (8 PKRs) 16 bpe @ SW_64K_S3_T @ RbPlus
    {   3,   29,  176,  153,    0, } , // 32 pipes (8 PKRs) 1 bpe @ SW_64K_S3_T @ RbPlus
    {   3,   30,  177,  154,    0, } , // 32 pipes (8 PKRs) 2 bpe @ SW_64K_S3_T @ RbPlus
    {   3,   31,  178,  155,    0, } , // 32 pipes (8 PKRs) 4 bpe @ SW_64K_S3_T @ RbPlus
    {   3,   32,  179,  156,    0, } , // 32 pipes (8 PKRs) 8 bpe @ SW_64K_S3_T @ RbPlus
    {   3,   33,  180,  157,    0, } , // 32 pipes (8 PKRs) 16 bpe @ SW_64K_S3_T @ RbPlus
    {   3,   29,  171,  148,    0, } , // 16 pipes (16 PKRs) 1 bpe @ SW_64K_S3_T @ RbPlus
    {   3,   30,  172,  149,    0, } , // 16 pipes (16 PKRs) 2 bpe @ SW_64K_S3_T @ RbPlus
    {   3,   31,  173,  150,    0, } , // 16 pipes (16 PKRs) 4 bpe @ SW_64K_S3_T @ RbPlus
    {   3,   32,  174,  151,    0, } , // 16 pipes (16 PKRs) 8 bpe @ SW_64K_S3_T @ RbPlus
    {   3,   33,  175,  152,    0, } , // 16 pipes (16 PKRs) 16 bpe @ SW_64K_S3_T @ RbPlus
    {   3,   29,  176,  153,    0, } , // 32 pipes (16 PKRs) 1 bpe @ SW_64K_S3_T @ RbPlus
    {   3,   30,  177,  154,    0, } , // 32 pipes (16 PKRs) 2 bpe @ SW_64K_S3_T @ RbPlus
    {   3,   31,  178,  155,    0, } , // 32 pipes (16 PKRs) 4 bpe @ SW_64K_S3_T @ RbPlus
    {   3,   32,  179,  156,    0, } , // 32 pipes (16 PKRs) 8 bpe @ SW_64K_S3_T @ RbPlus
    {   3,   33,  180,  157,    0, } , // 32 pipes (16 PKRs) 16 bpe @ SW_64K_S3_T @ RbPlus
    {   3,   29,  131,  163,    0, } , // 64 pipes (16 PKRs) 1 bpe @ SW_64K_S3_T @ RbPlus
    {   3,   30,  132,  164,    0, } , // 64 pipes (16 PKRs) 2 bpe @ SW_64K_S3_T @ RbPlus
    {   3,   31,  133,  165,    0, } , // 64 pipes (16 PKRs) 4 bpe @ SW_64K_S3_T @ RbPlus
    {   3,   32,  134,  166,    0, } , // 64 pipes (16 PKRs) 8 bpe @ SW_64K_S3_T @ RbPlus
    {   3,   33,  135,  167,    0, } , // 64 pipes (16 PKRs) 16 bpe @ SW_64K_S3_T @ RbPlus
    {   3,   29,  176,  153,    0, } , // 32 pipes (32 PKRs) 1 bpe @ SW_64K_S3_T @ RbPlus
    {   3,   30,  177,  154,    0, } , // 32 pipes (32 PKRs) 2 bpe @ SW_64K_S3_T @ RbPlus
    {   3,   31,  178,  155,    0, } , // 32 pipes (32 PKRs) 4 bpe @ SW_64K_S3_T @ RbPlus
    {   3,   32,  179,  156,    0, } , // 32 pipes (32 PKRs) 8 bpe @ SW_64K_S3_T @ RbPlus
    {   3,   33,  180,  157,    0, } , // 32 pipes (32 PKRs) 16 bpe @ SW_64K_S3_T @ RbPlus
    {   3,   29,  131,  163,    0, } , // 64 pipes (32 PKRs) 1 bpe @ SW_64K_S3_T @ RbPlus
    {   3,   30,  132,  164,    0, } , // 64 pipes (32 PKRs) 2 bpe @ SW_64K_S3_T @ RbPlus
    {   3,   31,  133,  165,    0, } , // 64 pipes (32 PKRs) 4 bpe @ SW_64K_S3_T @ RbPlus
    {   3,   32,  134,  166,    0, } , // 64 pipes (32 PKRs) 8 bpe @ SW_64K_S3_T @ RbPlus
    {   3,   33,  135,  167,    0, } , // 64 pipes (32 PKRs) 16 bpe @ SW_64K_S3_T @ RbPlus
};

const ADDR_SW_PATINFO GFX10_SW_64K_D3_X_RBPLUS_PATINFO[] =
{
    {   1,   34,  131,  148,    0, } , // 1 pipes (1 PKRs) 1 bpe @ SW_64K_D3_X @ RbPlus
    {   1,   35,  132,  149,    0, } , // 1 pipes (1 PKRs) 2 bpe @ SW_64K_D3_X @ RbPlus
    {   1,   36,  133,  150,    0, } , // 1 pipes (1 PKRs) 4 bpe @ SW_64K_D3_X @ RbPlus
    {   1,   37,  134,  151,    0, } , // 1 pipes (1 PKRs) 8 bpe @ SW_64K_D3_X @ RbPlus
    {   1,   38,  135,  152,    0, } , // 1 pipes (1 PKRs) 16 bpe @ SW_64K_D3_X @ RbPlus
    {   2,   34,  459,  170,    0, } , // 2 pipes (1-2 PKRs) 1 bpe @ SW_64K_D3_X @ RbPlus
    {   2,   35,  459,  801,    0, } , // 2 pipes (1-2 PKRs) 2 bpe @ SW_64K_D3_X @ RbPlus
    {   2,   36,  460,  802,    0, } , // 2 pipes (1-2 PKRs) 4 bpe @ SW_64K_D3_X @ RbPlus
    {   2,   37,  461,  152,    0, } , // 2 pipes (1-2 PKRs) 8 bpe @ SW_64K_D3_X @ RbPlus
    {   3,   38,  462,  152,    0, } , // 2 pipes (1-2 PKRs) 16 bpe @ SW_64K_D3_X @ RbPlus
    {   3,   34,  463,  803,    0, } , // 4 pipes (1-2 PKRs) 1 bpe @ SW_64K_D3_X @ RbPlus
    {   3,   35,  463,  804,    0, } , // 4 pipes (1-2 PKRs) 2 bpe @ SW_64K_D3_X @ RbPlus
    {   3,   36,  464,  805,    0, } , // 4 pipes (1-2 PKRs) 4 bpe @ SW_64K_D3_X @ RbPlus
    {   4,   37,  465,  806,    0, } , // 4 pipes (1-2 PKRs) 8 bpe @ SW_64K_D3_X @ RbPlus
    {   4,   38,  466,  806,    0, } , // 4 pipes (1-2 PKRs) 16 bpe @ SW_64K_D3_X @ RbPlus
    {   3,   34,  467,  803,    0, } , // 8 pipes (2 PKRs) 1 bpe @ SW_64K_D3_X @ RbPlus
    {   3,   35,  467,  804,    0, } , // 8 pipes (2 PKRs) 2 bpe @ SW_64K_D3_X @ RbPlus
    {   3,   36,  468,  805,    0, } , // 8 pipes (2 PKRs) 4 bpe @ SW_64K_D3_X @ RbPlus
    {   4,   37,  469,  806,    0, } , // 8 pipes (2 PKRs) 8 bpe @ SW_64K_D3_X @ RbPlus
    {   4,   38,  470,  806,    0, } , // 8 pipes (2 PKRs) 16 bpe @ SW_64K_D3_X @ RbPlus
    {   3,   34,  471,  807,    0, } , // 4 pipes (4 PKRs) 1 bpe @ SW_64K_D3_X @ RbPlus
    {   3,   35,  472,  808,    0, } , // 4 pipes (4 PKRs) 2 bpe @ SW_64K_D3_X @ RbPlus
    {   3,   36,  473,  809,    0, } , // 4 pipes (4 PKRs) 4 bpe @ SW_64K_D3_X @ RbPlus
    {   4,   37,  474,  810,    0, } , // 4 pipes (4 PKRs) 8 bpe @ SW_64K_D3_X @ RbPlus
    {   4,   38,  475,  811,    0, } , // 4 pipes (4 PKRs) 16 bpe @ SW_64K_D3_X @ RbPlus
    {   3,   34,  476,  812,    0, } , // 8 pipes (4 PKRs) 1 bpe @ SW_64K_D3_X @ RbPlus
    {   3,   35,  477,  804,    0, } , // 8 pipes (4 PKRs) 2 bpe @ SW_64K_D3_X @ RbPlus
    {   3,   36,  478,  805,    0, } , // 8 pipes (4 PKRs) 4 bpe @ SW_64K_D3_X @ RbPlus
    {   4,   37,  479,  806,    0, } , // 8 pipes (4 PKRs) 8 bpe @ SW_64K_D3_X @ RbPlus
    {   4,   38,  480,  806,    0, } , // 8 pipes (4 PKRs) 16 bpe @ SW_64K_D3_X @ RbPlus
    {   3,   34,  481,  813,    0, } , // 16 pipes (4 PKRs) 1 bpe @ SW_64K_D3_X @ RbPlus
    {   3,   35,  482,  804,    0, } , // 16 pipes (4 PKRs) 2 bpe @ SW_64K_D3_X @ RbPlus
    {   3,   36,  483,  805,    0, } , // 16 pipes (4 PKRs) 4 bpe @ SW_64K_D3_X @ RbPlus
    {   4,   37,  484,  806,    0, } , // 16 pipes (4 PKRs) 8 bpe @ SW_64K_D3_X @ RbPlus
    {   4,   38,  485,  806,    0, } , // 16 pipes (4 PKRs) 16 bpe @ SW_64K_D3_X @ RbPlus
    {   3,   34,  486,  814,    0, } , // 8 pipes (8 PKRs) 1 bpe @ SW_64K_D3_X @ RbPlus
    {   3,   35,  486,  815,    0, } , // 8 pipes (8 PKRs) 2 bpe @ SW_64K_D3_X @ RbPlus
    {   3,   36,  486,  816,    0, } , // 8 pipes (8 PKRs) 4 bpe @ SW_64K_D3_X @ RbPlus
    {   4,   37,  487,  817,    0, } , // 8 pipes (8 PKRs) 8 bpe @ SW_64K_D3_X @ RbPlus
    {   4,   38,  488,  817,    0, } , // 8 pipes (8 PKRs) 16 bpe @ SW_64K_D3_X @ RbPlus
    {   3,   34,  489,  812,    0, } , // 16 pipes (8 PKRs) 1 bpe @ SW_64K_D3_X @ RbPlus
    {   3,   35,  490,  804,    0, } , // 16 pipes (8 PKRs) 2 bpe @ SW_64K_D3_X @ RbPlus
    {   3,   36,  491,  805,    0, } , // 16 pipes (8 PKRs) 4 bpe @ SW_64K_D3_X @ RbPlus
    {   4,   37,  492,  806,    0, } , // 16 pipes (8 PKRs) 8 bpe @ SW_64K_D3_X @ RbPlus
    {   4,   38,  493,  806,    0, } , // 16 pipes (8 PKRs) 16 bpe @ SW_64K_D3_X @ RbPlus
    {   3,   34,  489,  818,    0, } , // 32 pipes (8 PKRs) 1 bpe @ SW_64K_D3_X @ RbPlus
    {   3,   35,  494,  819,    0, } , // 32 pipes (8 PKRs) 2 bpe @ SW_64K_D3_X @ RbPlus
    {   3,   36,  494,  820,    0, } , // 32 pipes (8 PKRs) 4 bpe @ SW_64K_D3_X @ RbPlus
    {   4,   37,  495,  821,    0, } , // 32 pipes (8 PKRs) 8 bpe @ SW_64K_D3_X @ RbPlus
    {   4,   38,  496,  821,    0, } , // 32 pipes (8 PKRs) 16 bpe @ SW_64K_D3_X @ RbPlus
    {   3,   34,  497,  822,    0, } , // 16 pipes (16 PKRs) 1 bpe @ SW_64K_D3_X @ RbPlus
    {   3,   35,  498,  823,    0, } , // 16 pipes (16 PKRs) 2 bpe @ SW_64K_D3_X @ RbPlus
    {   3,   36,  499,  824,    0, } , // 16 pipes (16 PKRs) 4 bpe @ SW_64K_D3_X @ RbPlus
    {   4,   37,  500,  825,    0, } , // 16 pipes (16 PKRs) 8 bpe @ SW_64K_D3_X @ RbPlus
    {   4,   38,  501,  825,    0, } , // 16 pipes (16 PKRs) 16 bpe @ SW_64K_D3_X @ RbPlus
    {   3,   34,  497,  826,    0, } , // 32 pipes (16 PKRs) 1 bpe @ SW_64K_D3_X @ RbPlus
    {   3,   35,  498,  827,    0, } , // 32 pipes (16 PKRs) 2 bpe @ SW_64K_D3_X @ RbPlus
    {   3,   36,  499,  828,    0, } , // 32 pipes (16 PKRs) 4 bpe @ SW_64K_D3_X @ RbPlus
    {   4,   37,  500,  829,    0, } , // 32 pipes (16 PKRs) 8 bpe @ SW_64K_D3_X @ RbPlus
    {   4,   38,  501,  829,    0, } , // 32 pipes (16 PKRs) 16 bpe @ SW_64K_D3_X @ RbPlus
    {   3,   34,  497,  830,    0, } , // 64 pipes (16 PKRs) 1 bpe @ SW_64K_D3_X @ RbPlus
    {   3,   35,  502,  831,    0, } , // 64 pipes (16 PKRs) 2 bpe @ SW_64K_D3_X @ RbPlus
    {   3,   36,  502,  832,    0, } , // 64 pipes (16 PKRs) 4 bpe @ SW_64K_D3_X @ RbPlus
    {   4,   37,  503,  833,    0, } , // 64 pipes (16 PKRs) 8 bpe @ SW_64K_D3_X @ RbPlus
    {   4,   38,  504,  833,    0, } , // 64 pipes (16 PKRs) 16 bpe @ SW_64K_D3_X @ RbPlus
    {   3,   34,  505,  834,    0, } , // 32 pipes (32 PKRs) 1 bpe @ SW_64K_D3_X @ RbPlus
    {   3,   35,  506,  835,    0, } , // 32 pipes (32 PKRs) 2 bpe @ SW_64K_D3_X @ RbPlus
    {   3,   36,  507,  836,    0, } , // 32 pipes (32 PKRs) 4 bpe @ SW_64K_D3_X @ RbPlus
    {   4,   37,  508,  837,    0, } , // 32 pipes (32 PKRs) 8 bpe @ SW_64K_D3_X @ RbPlus
    {   4,   38,  509,  837,    0, } , // 32 pipes (32 PKRs) 16 bpe @ SW_64K_D3_X @ RbPlus
    {   3,   34,  505,  838,    0, } , // 64 pipes (32 PKRs) 1 bpe @ SW_64K_D3_X @ RbPlus
    {   3,   35,  506,  839,    0, } , // 64 pipes (32 PKRs) 2 bpe @ SW_64K_D3_X @ RbPlus
    {   3,   36,  507,  840,    0, } , // 64 pipes (32 PKRs) 4 bpe @ SW_64K_D3_X @ RbPlus
    {   4,   37,  508,  841,    0, } , // 64 pipes (32 PKRs) 8 bpe @ SW_64K_D3_X @ RbPlus
    {   4,   38,  509,  841,    0, } , // 64 pipes (32 PKRs) 16 bpe @ SW_64K_D3_X @ RbPlus
};

const ADDR_SW_PATINFO GFX10_SW_VAR_R_X_1xaa_RBPLUS_PATINFO[] =
{
    {   2,    0,  270,  183,    0, } , // 1 pipes (1 PKRs) 1 bpe @ SW_VAR_R_X 1xaa @ RbPlus
    {   2,    1,  271,  184,    0, } , // 1 pipes (1 PKRs) 2 bpe @ SW_VAR_R_X 1xaa @ RbPlus
    {   2,   39,  272,  185,    0, } , // 1 pipes (1 PKRs) 4 bpe @ SW_VAR_R_X 1xaa @ RbPlus
    {   2,    6,  273,  186,    0, } , // 1 pipes (1 PKRs) 8 bpe @ SW_VAR_R_X 1xaa @ RbPlus
    {   2,    7,  274,  187,    0, } , // 1 pipes (1 PKRs) 16 bpe @ SW_VAR_R_X 1xaa @ RbPlus
    {   3,    0,  275,  188,    0, } , // 2 pipes (1-2 PKRs) 1 bpe @ SW_VAR_R_X 1xaa @ RbPlus
    {   3,    1,  276,  189,    0, } , // 2 pipes (1-2 PKRs) 2 bpe @ SW_VAR_R_X 1xaa @ RbPlus
    {   3,   39,  277,  190,    0, } , // 2 pipes (1-2 PKRs) 4 bpe @ SW_VAR_R_X 1xaa @ RbPlus
    {   3,    6,  278,  191,    0, } , // 2 pipes (1-2 PKRs) 8 bpe @ SW_VAR_R_X 1xaa @ RbPlus
    {   3,    7,  279,  192,    0, } , // 2 pipes (1-2 PKRs) 16 bpe @ SW_VAR_R_X 1xaa @ RbPlus
    {   3,    0,  280,  193,    0, } , // 4 pipes (1-2 PKRs) 1 bpe @ SW_VAR_R_X 1xaa @ RbPlus
    {   3,    1,  281,  194,    0, } , // 4 pipes (1-2 PKRs) 2 bpe @ SW_VAR_R_X 1xaa @ RbPlus
    {   3,   39,  282,  195,    0, } , // 4 pipes (1-2 PKRs) 4 bpe @ SW_VAR_R_X 1xaa @ RbPlus
    {   3,    6,  283,  196,    0, } , // 4 pipes (1-2 PKRs) 8 bpe @ SW_VAR_R_X 1xaa @ RbPlus
    {   3,    7,  284,  197,    0, } , // 4 pipes (1-2 PKRs) 16 bpe @ SW_VAR_R_X 1xaa @ RbPlus
    {   3,    0,  394,  198,    1, } , // 8 pipes (2 PKRs) 1 bpe @ SW_VAR_R_X 1xaa @ RbPlus
    {   3,    1,  395,  199,    2, } , // 8 pipes (2 PKRs) 2 bpe @ SW_VAR_R_X 1xaa @ RbPlus
    {   3,   39,  396,  200,    3, } , // 8 pipes (2 PKRs) 4 bpe @ SW_VAR_R_X 1xaa @ RbPlus
    {   3,    6,  397,  201,    4, } , // 8 pipes (2 PKRs) 8 bpe @ SW_VAR_R_X 1xaa @ RbPlus
    {   3,    7,  398,  202,    5, } , // 8 pipes (2 PKRs) 16 bpe @ SW_VAR_R_X 1xaa @ RbPlus
    {   3,    0,  290,  203,    0, } , // 4 pipes (4 PKRs) 1 bpe @ SW_VAR_R_X 1xaa @ RbPlus
    {   3,    1,  291,  204,    0, } , // 4 pipes (4 PKRs) 2 bpe @ SW_VAR_R_X 1xaa @ RbPlus
    {   3,   39,  292,  205,    0, } , // 4 pipes (4 PKRs) 4 bpe @ SW_VAR_R_X 1xaa @ RbPlus
    {   3,    6,  293,  206,    0, } , // 4 pipes (4 PKRs) 8 bpe @ SW_VAR_R_X 1xaa @ RbPlus
    {   3,    7,  294,  207,    0, } , // 4 pipes (4 PKRs) 16 bpe @ SW_VAR_R_X 1xaa @ RbPlus
    {   3,    0,  295,  208,    6, } , // 8 pipes (4 PKRs) 1 bpe @ SW_VAR_R_X 1xaa @ RbPlus
    {   3,    1,  296,  209,    2, } , // 8 pipes (4 PKRs) 2 bpe @ SW_VAR_R_X 1xaa @ RbPlus
    {   3,   39,  297,  210,    7, } , // 8 pipes (4 PKRs) 4 bpe @ SW_VAR_R_X 1xaa @ RbPlus
    {   3,    6,  298,  211,    4, } , // 8 pipes (4 PKRs) 8 bpe @ SW_VAR_R_X 1xaa @ RbPlus
    {   3,    7,  299,  212,    8, } , // 8 pipes (4 PKRs) 16 bpe @ SW_VAR_R_X 1xaa @ RbPlus
    {   3,    0,  399,  213,    9, } , // 16 pipes (4 PKRs) 1 bpe @ SW_VAR_R_X 1xaa @ RbPlus
    {   3,    1,  399,  214,   10, } , // 16 pipes (4 PKRs) 2 bpe @ SW_VAR_R_X 1xaa @ RbPlus
    {   3,   39,  399,  215,   11, } , // 16 pipes (4 PKRs) 4 bpe @ SW_VAR_R_X 1xaa @ RbPlus
    {   3,    6,  399,  216,   12, } , // 16 pipes (4 PKRs) 8 bpe @ SW_VAR_R_X 1xaa @ RbPlus
    {   3,    7,  399,  217,   13, } , // 16 pipes (4 PKRs) 16 bpe @ SW_VAR_R_X 1xaa @ RbPlus
    {   3,    0,  400,  218,   15, } , // 8 pipes (8 PKRs) 1 bpe @ SW_VAR_R_X 1xaa @ RbPlus
    {   3,    1,  401,  219,   15, } , // 8 pipes (8 PKRs) 2 bpe @ SW_VAR_R_X 1xaa @ RbPlus
    {   3,   39,  402,  220,   15, } , // 8 pipes (8 PKRs) 4 bpe @ SW_VAR_R_X 1xaa @ RbPlus
    {   3,    6,  304,  221,   15, } , // 8 pipes (8 PKRs) 8 bpe @ SW_VAR_R_X 1xaa @ RbPlus
    {   3,    7,  305,  222,   15, } , // 8 pipes (8 PKRs) 16 bpe @ SW_VAR_R_X 1xaa @ RbPlus
    {   3,    0,  307,  213,    9, } , // 16 pipes (8 PKRs) 1 bpe @ SW_VAR_R_X 1xaa @ RbPlus
    {   3,    1,  307,  223,   16, } , // 16 pipes (8 PKRs) 2 bpe @ SW_VAR_R_X 1xaa @ RbPlus
    {   3,   39,  307,  215,   11, } , // 16 pipes (8 PKRs) 4 bpe @ SW_VAR_R_X 1xaa @ RbPlus
    {   3,    6,  307,  216,   17, } , // 16 pipes (8 PKRs) 8 bpe @ SW_VAR_R_X 1xaa @ RbPlus
    {   3,    7,  307,  224,   13, } , // 16 pipes (8 PKRs) 16 bpe @ SW_VAR_R_X 1xaa @ RbPlus
    {   3,    0,  307,  497,   18, } , // 32 pipes (8 PKRs) 1 bpe @ SW_VAR_R_X 1xaa @ RbPlus
    {   3,    1,  307,  498,   19, } , // 32 pipes (8 PKRs) 2 bpe @ SW_VAR_R_X 1xaa @ RbPlus
    {   3,   39,  307,  499,   20, } , // 32 pipes (8 PKRs) 4 bpe @ SW_VAR_R_X 1xaa @ RbPlus
    {   3,    6,  307,  500,   21, } , // 32 pipes (8 PKRs) 8 bpe @ SW_VAR_R_X 1xaa @ RbPlus
    {   3,    7,  307,  501,   22, } , // 32 pipes (8 PKRs) 16 bpe @ SW_VAR_R_X 1xaa @ RbPlus
    {   3,    0,  309,  230,  125, } , // 16 pipes (16 PKRs) 1 bpe @ SW_VAR_R_X 1xaa @ RbPlus
    {   3,    1,  309,  231,  126, } , // 16 pipes (16 PKRs) 2 bpe @ SW_VAR_R_X 1xaa @ RbPlus
    {   3,   39,  309,  232,  127, } , // 16 pipes (16 PKRs) 4 bpe @ SW_VAR_R_X 1xaa @ RbPlus
    {   3,    6,  309,  233,   26, } , // 16 pipes (16 PKRs) 8 bpe @ SW_VAR_R_X 1xaa @ RbPlus
    {   3,    7,  309,  234,   27, } , // 16 pipes (16 PKRs) 16 bpe @ SW_VAR_R_X 1xaa @ RbPlus
    {   3,    0,  309,  502,   28, } , // 32 pipes (16 PKRs) 1 bpe @ SW_VAR_R_X 1xaa @ RbPlus
    {   3,    1,  309,  503,   19, } , // 32 pipes (16 PKRs) 2 bpe @ SW_VAR_R_X 1xaa @ RbPlus
    {   3,   39,  309,  504,   29, } , // 32 pipes (16 PKRs) 4 bpe @ SW_VAR_R_X 1xaa @ RbPlus
    {   3,    6,  309,  238,   30, } , // 32 pipes (16 PKRs) 8 bpe @ SW_VAR_R_X 1xaa @ RbPlus
    {   3,    7,  309,  239,   31, } , // 32 pipes (16 PKRs) 16 bpe @ SW_VAR_R_X 1xaa @ RbPlus
    {   3,    0,  309,  505,   32, } , // 64 pipes (16 PKRs) 1 bpe @ SW_VAR_R_X 1xaa @ RbPlus
    {   3,    1,  309,  506,   33, } , // 64 pipes (16 PKRs) 2 bpe @ SW_VAR_R_X 1xaa @ RbPlus
    {   3,   39,  309,  507,   34, } , // 64 pipes (16 PKRs) 4 bpe @ SW_VAR_R_X 1xaa @ RbPlus
    {   3,    6,  309,  508,   35, } , // 64 pipes (16 PKRs) 8 bpe @ SW_VAR_R_X 1xaa @ RbPlus
    {   3,    7,  309,  509,   36, } , // 64 pipes (16 PKRs) 16 bpe @ SW_VAR_R_X 1xaa @ RbPlus
    {   3,    0,  311,  510,  128, } , // 32 pipes (32 PKRs) 1 bpe @ SW_VAR_R_X 1xaa @ RbPlus
    {   3,    1,  311,  511,  129, } , // 32 pipes (32 PKRs) 2 bpe @ SW_VAR_R_X 1xaa @ RbPlus
    {   3,   39,  311,  512,  130, } , // 32 pipes (32 PKRs) 4 bpe @ SW_VAR_R_X 1xaa @ RbPlus
    {   3,    6,  311,  248,   40, } , // 32 pipes (32 PKRs) 8 bpe @ SW_VAR_R_X 1xaa @ RbPlus
    {   3,    7,  311,  249,   41, } , // 32 pipes (32 PKRs) 16 bpe @ SW_VAR_R_X 1xaa @ RbPlus
    {   3,    0,  311,  513,   32, } , // 64 pipes (32 PKRs) 1 bpe @ SW_VAR_R_X 1xaa @ RbPlus
    {   3,    1,  311,  514,   42, } , // 64 pipes (32 PKRs) 2 bpe @ SW_VAR_R_X 1xaa @ RbPlus
    {   3,   39,  311,  515,   34, } , // 64 pipes (32 PKRs) 4 bpe @ SW_VAR_R_X 1xaa @ RbPlus
    {   3,    6,  311,  253,   43, } , // 64 pipes (32 PKRs) 8 bpe @ SW_VAR_R_X 1xaa @ RbPlus
    {   3,    7,  311,  254,   44, } , // 64 pipes (32 PKRs) 16 bpe @ SW_VAR_R_X 1xaa @ RbPlus
};

const ADDR_SW_PATINFO GFX10_SW_VAR_R_X_2xaa_RBPLUS_PATINFO[] =
{
    {   3,    0,  403,  516,    0, } , // 1 pipes (1 PKRs) 1 bpe @ SW_VAR_R_X 2xaa @ RbPlus
    {   3,    1,  271,  517,    0, } , // 1 pipes (1 PKRs) 2 bpe @ SW_VAR_R_X 2xaa @ RbPlus
    {   3,   39,  313,  518,    0, } , // 1 pipes (1 PKRs) 4 bpe @ SW_VAR_R_X 2xaa @ RbPlus
    {   3,    6,  273,  519,    0, } , // 1 pipes (1 PKRs) 8 bpe @ SW_VAR_R_X 2xaa @ RbPlus
    {   3,    7,  314,  520,    0, } , // 1 pipes (1 PKRs) 16 bpe @ SW_VAR_R_X 2xaa @ RbPlus
    {   3,    0,  404,  521,    0, } , // 2 pipes (1-2 PKRs) 1 bpe @ SW_VAR_R_X 2xaa @ RbPlus
    {   3,    1,  276,  522,    0, } , // 2 pipes (1-2 PKRs) 2 bpe @ SW_VAR_R_X 2xaa @ RbPlus
    {   3,   39,  315,  523,    0, } , // 2 pipes (1-2 PKRs) 4 bpe @ SW_VAR_R_X 2xaa @ RbPlus
    {   3,    6,  278,  524,    0, } , // 2 pipes (1-2 PKRs) 8 bpe @ SW_VAR_R_X 2xaa @ RbPlus
    {   3,    7,  316,  525,    0, } , // 2 pipes (1-2 PKRs) 16 bpe @ SW_VAR_R_X 2xaa @ RbPlus
    {   3,    0,  280,  526,    0, } , // 4 pipes (1-2 PKRs) 1 bpe @ SW_VAR_R_X 2xaa @ RbPlus
    {   3,    1,  281,  527,    0, } , // 4 pipes (1-2 PKRs) 2 bpe @ SW_VAR_R_X 2xaa @ RbPlus
    {   3,   39,  282,  528,    0, } , // 4 pipes (1-2 PKRs) 4 bpe @ SW_VAR_R_X 2xaa @ RbPlus
    {   3,    6,  283,  529,    0, } , // 4 pipes (1-2 PKRs) 8 bpe @ SW_VAR_R_X 2xaa @ RbPlus
    {   3,    7,  284,  530,    0, } , // 4 pipes (1-2 PKRs) 16 bpe @ SW_VAR_R_X 2xaa @ RbPlus
    {   3,    0,  394,  208,  131, } , // 8 pipes (2 PKRs) 1 bpe @ SW_VAR_R_X 2xaa @ RbPlus
    {   3,    1,  395,  531,  132, } , // 8 pipes (2 PKRs) 2 bpe @ SW_VAR_R_X 2xaa @ RbPlus
    {   3,   39,  396,  302,  133, } , // 8 pipes (2 PKRs) 4 bpe @ SW_VAR_R_X 2xaa @ RbPlus
    {   3,    6,  397,  532,  134, } , // 8 pipes (2 PKRs) 8 bpe @ SW_VAR_R_X 2xaa @ RbPlus
    {   3,    7,  398,  533,  135, } , // 8 pipes (2 PKRs) 16 bpe @ SW_VAR_R_X 2xaa @ RbPlus
    {   3,    0,  290,  534,    0, } , // 4 pipes (4 PKRs) 1 bpe @ SW_VAR_R_X 2xaa @ RbPlus
    {   3,    1,  291,  535,    0, } , // 4 pipes (4 PKRs) 2 bpe @ SW_VAR_R_X 2xaa @ RbPlus
    {   3,   39,  292,  536,    0, } , // 4 pipes (4 PKRs) 4 bpe @ SW_VAR_R_X 2xaa @ RbPlus
    {   3,    6,  293,  537,    0, } , // 4 pipes (4 PKRs) 8 bpe @ SW_VAR_R_X 2xaa @ RbPlus
    {   3,    7,  294,  538,    0, } , // 4 pipes (4 PKRs) 16 bpe @ SW_VAR_R_X 2xaa @ RbPlus
    {   3,    0,  295,  208,  131, } , // 8 pipes (4 PKRs) 1 bpe @ SW_VAR_R_X 2xaa @ RbPlus
    {   3,    1,  296,  209,  132, } , // 8 pipes (4 PKRs) 2 bpe @ SW_VAR_R_X 2xaa @ RbPlus
    {   3,   39,  297,  210,  133, } , // 8 pipes (4 PKRs) 4 bpe @ SW_VAR_R_X 2xaa @ RbPlus
    {   3,    6,  298,  211,  134, } , // 8 pipes (4 PKRs) 8 bpe @ SW_VAR_R_X 2xaa @ RbPlus
    {   3,    7,  299,  212,  135, } , // 8 pipes (4 PKRs) 16 bpe @ SW_VAR_R_X 2xaa @ RbPlus
    {   3,    0,  399,  539,  136, } , // 16 pipes (4 PKRs) 1 bpe @ SW_VAR_R_X 2xaa @ RbPlus
    {   3,    1,  399,  214,  137, } , // 16 pipes (4 PKRs) 2 bpe @ SW_VAR_R_X 2xaa @ RbPlus
    {   3,   39,  399,  280,  138, } , // 16 pipes (4 PKRs) 4 bpe @ SW_VAR_R_X 2xaa @ RbPlus
    {   3,    6,  399,  216,  139, } , // 16 pipes (4 PKRs) 8 bpe @ SW_VAR_R_X 2xaa @ RbPlus
    {   3,    7,  399,  224,  140, } , // 16 pipes (4 PKRs) 16 bpe @ SW_VAR_R_X 2xaa @ RbPlus
    {   3,    0,  400,  540,   15, } , // 8 pipes (8 PKRs) 1 bpe @ SW_VAR_R_X 2xaa @ RbPlus
    {   3,    1,  401,  541,   15, } , // 8 pipes (8 PKRs) 2 bpe @ SW_VAR_R_X 2xaa @ RbPlus
    {   3,   39,  402,  542,   15, } , // 8 pipes (8 PKRs) 4 bpe @ SW_VAR_R_X 2xaa @ RbPlus
    {   3,    6,  304,  543,   15, } , // 8 pipes (8 PKRs) 8 bpe @ SW_VAR_R_X 2xaa @ RbPlus
    {   3,    7,  305,  544,   15, } , // 8 pipes (8 PKRs) 16 bpe @ SW_VAR_R_X 2xaa @ RbPlus
    {   3,    0,  307,  539,  136, } , // 16 pipes (8 PKRs) 1 bpe @ SW_VAR_R_X 2xaa @ RbPlus
    {   3,    1,  307,  214,  137, } , // 16 pipes (8 PKRs) 2 bpe @ SW_VAR_R_X 2xaa @ RbPlus
    {   3,   39,  307,  280,  138, } , // 16 pipes (8 PKRs) 4 bpe @ SW_VAR_R_X 2xaa @ RbPlus
    {   3,    6,  307,  216,  139, } , // 16 pipes (8 PKRs) 8 bpe @ SW_VAR_R_X 2xaa @ RbPlus
    {   3,    7,  307,  224,  140, } , // 16 pipes (8 PKRs) 16 bpe @ SW_VAR_R_X 2xaa @ RbPlus
    {   3,    0,  307,  545,  141, } , // 32 pipes (8 PKRs) 1 bpe @ SW_VAR_R_X 2xaa @ RbPlus
    {   3,    1,  307,  498,  142, } , // 32 pipes (8 PKRs) 2 bpe @ SW_VAR_R_X 2xaa @ RbPlus
    {   3,   39,  307,  546,  143, } , // 32 pipes (8 PKRs) 4 bpe @ SW_VAR_R_X 2xaa @ RbPlus
    {   3,    6,  307,  500,  144, } , // 32 pipes (8 PKRs) 8 bpe @ SW_VAR_R_X 2xaa @ RbPlus
    {   3,    7,  307,  547,  145, } , // 32 pipes (8 PKRs) 16 bpe @ SW_VAR_R_X 2xaa @ RbPlus
    {   3,    0,  309,  548,  146, } , // 16 pipes (16 PKRs) 1 bpe @ SW_VAR_R_X 2xaa @ RbPlus
    {   3,    1,  309,  231,  147, } , // 16 pipes (16 PKRs) 2 bpe @ SW_VAR_R_X 2xaa @ RbPlus
    {   3,   39,  309,  285,  148, } , // 16 pipes (16 PKRs) 4 bpe @ SW_VAR_R_X 2xaa @ RbPlus
    {   3,    6,  309,  233,  149, } , // 16 pipes (16 PKRs) 8 bpe @ SW_VAR_R_X 2xaa @ RbPlus
    {   3,    7,  309,  286,  150, } , // 16 pipes (16 PKRs) 16 bpe @ SW_VAR_R_X 2xaa @ RbPlus
    {   3,    0,  309,  502,  141, } , // 32 pipes (16 PKRs) 1 bpe @ SW_VAR_R_X 2xaa @ RbPlus
    {   3,    1,  309,  503,  151, } , // 32 pipes (16 PKRs) 2 bpe @ SW_VAR_R_X 2xaa @ RbPlus
    {   3,   39,  309,  504,  143, } , // 32 pipes (16 PKRs) 4 bpe @ SW_VAR_R_X 2xaa @ RbPlus
    {   3,    6,  309,  238,  152, } , // 32 pipes (16 PKRs) 8 bpe @ SW_VAR_R_X 2xaa @ RbPlus
    {   3,    7,  309,  239,  153, } , // 32 pipes (16 PKRs) 16 bpe @ SW_VAR_R_X 2xaa @ RbPlus
    {   3,    0,  309,  505,  154, } , // 64 pipes (16 PKRs) 1 bpe @ SW_VAR_R_X 2xaa @ RbPlus
    {   3,    1,  309,  506,  155, } , // 64 pipes (16 PKRs) 2 bpe @ SW_VAR_R_X 2xaa @ RbPlus
    {   3,   39,  309,  507,  156, } , // 64 pipes (16 PKRs) 4 bpe @ SW_VAR_R_X 2xaa @ RbPlus
    {   3,    6,  309,  508,  157, } , // 64 pipes (16 PKRs) 8 bpe @ SW_VAR_R_X 2xaa @ RbPlus
    {   3,    7,  309,  509,  158, } , // 64 pipes (16 PKRs) 16 bpe @ SW_VAR_R_X 2xaa @ RbPlus
    {   3,    0,  318,  549,  159, } , // 32 pipes (32 PKRs) 1 bpe @ SW_VAR_R_X 2xaa @ RbPlus
    {   3,    1,  318,  550,  160, } , // 32 pipes (32 PKRs) 2 bpe @ SW_VAR_R_X 2xaa @ RbPlus
    {   3,   39,  318,  551,  161, } , // 32 pipes (32 PKRs) 4 bpe @ SW_VAR_R_X 2xaa @ RbPlus
    {   3,    6,  318,  287,  162, } , // 32 pipes (32 PKRs) 8 bpe @ SW_VAR_R_X 2xaa @ RbPlus
    {   3,    7,  318,  288,  163, } , // 32 pipes (32 PKRs) 16 bpe @ SW_VAR_R_X 2xaa @ RbPlus
    {   3,    0,  318,  552,  154, } , // 64 pipes (32 PKRs) 1 bpe @ SW_VAR_R_X 2xaa @ RbPlus
    {   3,    1,  318,  553,  155, } , // 64 pipes (32 PKRs) 2 bpe @ SW_VAR_R_X 2xaa @ RbPlus
    {   3,   39,  318,  554,  156, } , // 64 pipes (32 PKRs) 4 bpe @ SW_VAR_R_X 2xaa @ RbPlus
    {   3,    6,  318,  555,  157, } , // 64 pipes (32 PKRs) 8 bpe @ SW_VAR_R_X 2xaa @ RbPlus
    {   3,    7,  318,  290,  158, } , // 64 pipes (32 PKRs) 16 bpe @ SW_VAR_R_X 2xaa @ RbPlus
};

const ADDR_SW_PATINFO GFX10_SW_VAR_R_X_4xaa_RBPLUS_PATINFO[] =
{
    {   3,    0,  270,  556,    0, } , // 1 pipes (1 PKRs) 1 bpe @ SW_VAR_R_X 4xaa @ RbPlus
    {   3,    1,  271,  557,    0, } , // 1 pipes (1 PKRs) 2 bpe @ SW_VAR_R_X 4xaa @ RbPlus
    {   3,   39,  272,  558,    0, } , // 1 pipes (1 PKRs) 4 bpe @ SW_VAR_R_X 4xaa @ RbPlus
    {   3,    6,  273,  559,    0, } , // 1 pipes (1 PKRs) 8 bpe @ SW_VAR_R_X 4xaa @ RbPlus
    {   3,    7,  274,  560,    0, } , // 1 pipes (1 PKRs) 16 bpe @ SW_VAR_R_X 4xaa @ RbPlus
    {   3,    0,  275,  561,    0, } , // 2 pipes (1-2 PKRs) 1 bpe @ SW_VAR_R_X 4xaa @ RbPlus
    {   3,    1,  276,  562,    0, } , // 2 pipes (1-2 PKRs) 2 bpe @ SW_VAR_R_X 4xaa @ RbPlus
    {   3,   39,  277,  563,    0, } , // 2 pipes (1-2 PKRs) 4 bpe @ SW_VAR_R_X 4xaa @ RbPlus
    {   3,    6,  278,  564,    0, } , // 2 pipes (1-2 PKRs) 8 bpe @ SW_VAR_R_X 4xaa @ RbPlus
    {   3,    7,  279,  565,    0, } , // 2 pipes (1-2 PKRs) 16 bpe @ SW_VAR_R_X 4xaa @ RbPlus
    {   3,    0,  280,  566,    0, } , // 4 pipes (1-2 PKRs) 1 bpe @ SW_VAR_R_X 4xaa @ RbPlus
    {   3,    1,  281,  567,    0, } , // 4 pipes (1-2 PKRs) 2 bpe @ SW_VAR_R_X 4xaa @ RbPlus
    {   3,   39,  282,  568,    0, } , // 4 pipes (1-2 PKRs) 4 bpe @ SW_VAR_R_X 4xaa @ RbPlus
    {   3,    6,  283,  569,    0, } , // 4 pipes (1-2 PKRs) 8 bpe @ SW_VAR_R_X 4xaa @ RbPlus
    {   3,    7,  284,  570,    0, } , // 4 pipes (1-2 PKRs) 16 bpe @ SW_VAR_R_X 4xaa @ RbPlus
    {   3,    0,  394,  571,  164, } , // 8 pipes (2 PKRs) 1 bpe @ SW_VAR_R_X 4xaa @ RbPlus
    {   3,    1,  395,  572,  165, } , // 8 pipes (2 PKRs) 2 bpe @ SW_VAR_R_X 4xaa @ RbPlus
    {   3,   39,  396,  573,  166, } , // 8 pipes (2 PKRs) 4 bpe @ SW_VAR_R_X 4xaa @ RbPlus
    {   3,    6,  397,  574,  167, } , // 8 pipes (2 PKRs) 8 bpe @ SW_VAR_R_X 4xaa @ RbPlus
    {   3,    7,  398,  575,  168, } , // 8 pipes (2 PKRs) 16 bpe @ SW_VAR_R_X 4xaa @ RbPlus
    {   3,    0,  290,  576,    0, } , // 4 pipes (4 PKRs) 1 bpe @ SW_VAR_R_X 4xaa @ RbPlus
    {   3,    1,  291,  577,    0, } , // 4 pipes (4 PKRs) 2 bpe @ SW_VAR_R_X 4xaa @ RbPlus
    {   3,   39,  292,  578,    0, } , // 4 pipes (4 PKRs) 4 bpe @ SW_VAR_R_X 4xaa @ RbPlus
    {   3,    6,  293,  579,    0, } , // 4 pipes (4 PKRs) 8 bpe @ SW_VAR_R_X 4xaa @ RbPlus
    {   3,    7,  405,  580,    0, } , // 4 pipes (4 PKRs) 16 bpe @ SW_VAR_R_X 4xaa @ RbPlus
    {   3,    0,  295,  581,  169, } , // 8 pipes (4 PKRs) 1 bpe @ SW_VAR_R_X 4xaa @ RbPlus
    {   3,    1,  296,  582,  165, } , // 8 pipes (4 PKRs) 2 bpe @ SW_VAR_R_X 4xaa @ RbPlus
    {   3,   39,  297,  583,  170, } , // 8 pipes (4 PKRs) 4 bpe @ SW_VAR_R_X 4xaa @ RbPlus
    {   3,    6,  298,  584,  167, } , // 8 pipes (4 PKRs) 8 bpe @ SW_VAR_R_X 4xaa @ RbPlus
    {   3,    7,  299,  585,  168, } , // 8 pipes (4 PKRs) 16 bpe @ SW_VAR_R_X 4xaa @ RbPlus
    {   3,    0,  399,  213,  171, } , // 16 pipes (4 PKRs) 1 bpe @ SW_VAR_R_X 4xaa @ RbPlus
    {   3,    1,  399,  214,  172, } , // 16 pipes (4 PKRs) 2 bpe @ SW_VAR_R_X 4xaa @ RbPlus
    {   3,   39,  399,  215,  173, } , // 16 pipes (4 PKRs) 4 bpe @ SW_VAR_R_X 4xaa @ RbPlus
    {   3,    6,  399,  216,  174, } , // 16 pipes (4 PKRs) 8 bpe @ SW_VAR_R_X 4xaa @ RbPlus
    {   3,    7,  399,  217,  175, } , // 16 pipes (4 PKRs) 16 bpe @ SW_VAR_R_X 4xaa @ RbPlus
    {   3,    0,  400,  586,   15, } , // 8 pipes (8 PKRs) 1 bpe @ SW_VAR_R_X 4xaa @ RbPlus
    {   3,    1,  401,  587,   15, } , // 8 pipes (8 PKRs) 2 bpe @ SW_VAR_R_X 4xaa @ RbPlus
    {   3,   39,  402,  588,   15, } , // 8 pipes (8 PKRs) 4 bpe @ SW_VAR_R_X 4xaa @ RbPlus
    {   3,    6,  304,  589,   15, } , // 8 pipes (8 PKRs) 8 bpe @ SW_VAR_R_X 4xaa @ RbPlus
    {   3,    7,  406,  544,   15, } , // 8 pipes (8 PKRs) 16 bpe @ SW_VAR_R_X 4xaa @ RbPlus
    {   3,    0,  307,  213,  171, } , // 16 pipes (8 PKRs) 1 bpe @ SW_VAR_R_X 4xaa @ RbPlus
    {   3,    1,  307,  223,  176, } , // 16 pipes (8 PKRs) 2 bpe @ SW_VAR_R_X 4xaa @ RbPlus
    {   3,   39,  307,  215,  173, } , // 16 pipes (8 PKRs) 4 bpe @ SW_VAR_R_X 4xaa @ RbPlus
    {   3,    6,  307,  216,  177, } , // 16 pipes (8 PKRs) 8 bpe @ SW_VAR_R_X 4xaa @ RbPlus
    {   3,    7,  307,  224,  175, } , // 16 pipes (8 PKRs) 16 bpe @ SW_VAR_R_X 4xaa @ RbPlus
    {   3,    0,  307,  497,  178, } , // 32 pipes (8 PKRs) 1 bpe @ SW_VAR_R_X 4xaa @ RbPlus
    {   3,    1,  307,  498,  179, } , // 32 pipes (8 PKRs) 2 bpe @ SW_VAR_R_X 4xaa @ RbPlus
    {   3,   39,  307,  499,  180, } , // 32 pipes (8 PKRs) 4 bpe @ SW_VAR_R_X 4xaa @ RbPlus
    {   3,    6,  307,  500,  181, } , // 32 pipes (8 PKRs) 8 bpe @ SW_VAR_R_X 4xaa @ RbPlus
    {   3,    7,  307,  501,  182, } , // 32 pipes (8 PKRs) 16 bpe @ SW_VAR_R_X 4xaa @ RbPlus
    {   3,    0,  323,  590,  183, } , // 16 pipes (16 PKRs) 1 bpe @ SW_VAR_R_X 4xaa @ RbPlus
    {   3,    1,  323,  591,  184, } , // 16 pipes (16 PKRs) 2 bpe @ SW_VAR_R_X 4xaa @ RbPlus
    {   3,   39,  323,  592,  185, } , // 16 pipes (16 PKRs) 4 bpe @ SW_VAR_R_X 4xaa @ RbPlus
    {   3,    6,  323,  593,  186, } , // 16 pipes (16 PKRs) 8 bpe @ SW_VAR_R_X 4xaa @ RbPlus
    {   3,    7,  323,  286,  187, } , // 16 pipes (16 PKRs) 16 bpe @ SW_VAR_R_X 4xaa @ RbPlus
    {   3,    0,  323,  594,  188, } , // 32 pipes (16 PKRs) 1 bpe @ SW_VAR_R_X 4xaa @ RbPlus
    {   3,    1,  323,  595,  179, } , // 32 pipes (16 PKRs) 2 bpe @ SW_VAR_R_X 4xaa @ RbPlus
    {   3,   39,  323,  596,  189, } , // 32 pipes (16 PKRs) 4 bpe @ SW_VAR_R_X 4xaa @ RbPlus
    {   3,    6,  323,  321,  190, } , // 32 pipes (16 PKRs) 8 bpe @ SW_VAR_R_X 4xaa @ RbPlus
    {   3,    7,  323,  322,  191, } , // 32 pipes (16 PKRs) 16 bpe @ SW_VAR_R_X 4xaa @ RbPlus
    {   3,    0,  323,  597,  192, } , // 64 pipes (16 PKRs) 1 bpe @ SW_VAR_R_X 4xaa @ RbPlus
    {   3,    1,  323,  598,  193, } , // 64 pipes (16 PKRs) 2 bpe @ SW_VAR_R_X 4xaa @ RbPlus
    {   3,   39,  323,  599,  194, } , // 64 pipes (16 PKRs) 4 bpe @ SW_VAR_R_X 4xaa @ RbPlus
    {   3,    6,  323,  600,  195, } , // 64 pipes (16 PKRs) 8 bpe @ SW_VAR_R_X 4xaa @ RbPlus
    {   3,    7,  323,  601,  196, } , // 64 pipes (16 PKRs) 16 bpe @ SW_VAR_R_X 4xaa @ RbPlus
    {   3,    0,  324,  602,  197, } , // 32 pipes (32 PKRs) 1 bpe @ SW_VAR_R_X 4xaa @ RbPlus
    {   3,    1,  324,  603,  198, } , // 32 pipes (32 PKRs) 2 bpe @ SW_VAR_R_X 4xaa @ RbPlus
    {   3,   39,  324,  604,  199, } , // 32 pipes (32 PKRs) 4 bpe @ SW_VAR_R_X 4xaa @ RbPlus
    {   3,    6,  324,  605,  200, } , // 32 pipes (32 PKRs) 8 bpe @ SW_VAR_R_X 4xaa @ RbPlus
    {   3,    7,  324,  606,  201, } , // 32 pipes (32 PKRs) 16 bpe @ SW_VAR_R_X 4xaa @ RbPlus
    {   3,    0,  324,  607,  192, } , // 64 pipes (32 PKRs) 1 bpe @ SW_VAR_R_X 4xaa @ RbPlus
    {   3,    1,  324,  608,  202, } , // 64 pipes (32 PKRs) 2 bpe @ SW_VAR_R_X 4xaa @ RbPlus
    {   3,   39,  324,  609,  194, } , // 64 pipes (32 PKRs) 4 bpe @ SW_VAR_R_X 4xaa @ RbPlus
    {   3,    6,  324,  327,  203, } , // 64 pipes (32 PKRs) 8 bpe @ SW_VAR_R_X 4xaa @ RbPlus
    {   3,    7,  324,  328,  204, } , // 64 pipes (32 PKRs) 16 bpe @ SW_VAR_R_X 4xaa @ RbPlus
};

const ADDR_SW_PATINFO GFX10_SW_VAR_R_X_8xaa_RBPLUS_PATINFO[] =
{
    {   3,    0,  407,  610,    0, } , // 1 pipes (1 PKRs) 1 bpe @ SW_VAR_R_X 8xaa @ RbPlus
    {   3,    1,  408,  611,    0, } , // 1 pipes (1 PKRs) 2 bpe @ SW_VAR_R_X 8xaa @ RbPlus
    {   3,   39,  409,  612,    0, } , // 1 pipes (1 PKRs) 4 bpe @ SW_VAR_R_X 8xaa @ RbPlus
    {   3,    6,  410,  613,    0, } , // 1 pipes (1 PKRs) 8 bpe @ SW_VAR_R_X 8xaa @ RbPlus
    {   3,    7,  411,  614,    0, } , // 1 pipes (1 PKRs) 16 bpe @ SW_VAR_R_X 8xaa @ RbPlus
    {   3,    0,  404,  615,    0, } , // 2 pipes (1-2 PKRs) 1 bpe @ SW_VAR_R_X 8xaa @ RbPlus
    {   3,    1,  276,  616,    0, } , // 2 pipes (1-2 PKRs) 2 bpe @ SW_VAR_R_X 8xaa @ RbPlus
    {   3,   39,  315,  617,    0, } , // 2 pipes (1-2 PKRs) 4 bpe @ SW_VAR_R_X 8xaa @ RbPlus
    {   3,    6,  278,  618,    0, } , // 2 pipes (1-2 PKRs) 8 bpe @ SW_VAR_R_X 8xaa @ RbPlus
    {   3,    7,  412,  565,    0, } , // 2 pipes (1-2 PKRs) 16 bpe @ SW_VAR_R_X 8xaa @ RbPlus
    {   3,    0,  280,  619,    0, } , // 4 pipes (1-2 PKRs) 1 bpe @ SW_VAR_R_X 8xaa @ RbPlus
    {   3,    1,  281,  620,    0, } , // 4 pipes (1-2 PKRs) 2 bpe @ SW_VAR_R_X 8xaa @ RbPlus
    {   3,   39,  282,  621,    0, } , // 4 pipes (1-2 PKRs) 4 bpe @ SW_VAR_R_X 8xaa @ RbPlus
    {   3,    6,  283,  622,    0, } , // 4 pipes (1-2 PKRs) 8 bpe @ SW_VAR_R_X 8xaa @ RbPlus
    {   3,    7,  413,  623,    0, } , // 4 pipes (1-2 PKRs) 16 bpe @ SW_VAR_R_X 8xaa @ RbPlus
    {   3,    0,  394,  624,  205, } , // 8 pipes (2 PKRs) 1 bpe @ SW_VAR_R_X 8xaa @ RbPlus
    {   3,    1,  395,  625,  206, } , // 8 pipes (2 PKRs) 2 bpe @ SW_VAR_R_X 8xaa @ RbPlus
    {   3,   39,  396,  626,  207, } , // 8 pipes (2 PKRs) 4 bpe @ SW_VAR_R_X 8xaa @ RbPlus
    {   3,    6,  397,  627,  208, } , // 8 pipes (2 PKRs) 8 bpe @ SW_VAR_R_X 8xaa @ RbPlus
    {   3,    7,  414,  628,  209, } , // 8 pipes (2 PKRs) 16 bpe @ SW_VAR_R_X 8xaa @ RbPlus
    {   3,    0,  415,  629,    0, } , // 4 pipes (4 PKRs) 1 bpe @ SW_VAR_R_X 8xaa @ RbPlus
    {   3,    1,  291,  630,    0, } , // 4 pipes (4 PKRs) 2 bpe @ SW_VAR_R_X 8xaa @ RbPlus
    {   3,   39,  292,  631,    0, } , // 4 pipes (4 PKRs) 4 bpe @ SW_VAR_R_X 8xaa @ RbPlus
    {   3,    6,  416,  632,    0, } , // 4 pipes (4 PKRs) 8 bpe @ SW_VAR_R_X 8xaa @ RbPlus
    {   3,    7,  417,  580,    0, } , // 4 pipes (4 PKRs) 16 bpe @ SW_VAR_R_X 8xaa @ RbPlus
    {   3,    0,  295,  624,  205, } , // 8 pipes (4 PKRs) 1 bpe @ SW_VAR_R_X 8xaa @ RbPlus
    {   3,    1,  296,  633,  206, } , // 8 pipes (4 PKRs) 2 bpe @ SW_VAR_R_X 8xaa @ RbPlus
    {   3,   39,  297,  634,  207, } , // 8 pipes (4 PKRs) 4 bpe @ SW_VAR_R_X 8xaa @ RbPlus
    {   3,    6,  298,  627,  208, } , // 8 pipes (4 PKRs) 8 bpe @ SW_VAR_R_X 8xaa @ RbPlus
    {   3,    7,  418,  635,  210, } , // 8 pipes (4 PKRs) 16 bpe @ SW_VAR_R_X 8xaa @ RbPlus
    {   3,    0,  399,  636,  211, } , // 16 pipes (4 PKRs) 1 bpe @ SW_VAR_R_X 8xaa @ RbPlus
    {   3,    1,  399,  637,  212, } , // 16 pipes (4 PKRs) 2 bpe @ SW_VAR_R_X 8xaa @ RbPlus
    {   3,   39,  399,  638,  213, } , // 16 pipes (4 PKRs) 4 bpe @ SW_VAR_R_X 8xaa @ RbPlus
    {   3,    6,  399,  639,  214, } , // 16 pipes (4 PKRs) 8 bpe @ SW_VAR_R_X 8xaa @ RbPlus
    {   3,    7,  419,  640,  215, } , // 16 pipes (4 PKRs) 16 bpe @ SW_VAR_R_X 8xaa @ RbPlus
    {   3,    0,  301,  641,  216, } , // 8 pipes (8 PKRs) 1 bpe @ SW_VAR_R_X 8xaa @ RbPlus
    {   3,    1,  302,  642,  216, } , // 8 pipes (8 PKRs) 2 bpe @ SW_VAR_R_X 8xaa @ RbPlus
    {   3,   39,  303,  643,  216, } , // 8 pipes (8 PKRs) 4 bpe @ SW_VAR_R_X 8xaa @ RbPlus
    {   3,    6,  420,  589,  105, } , // 8 pipes (8 PKRs) 8 bpe @ SW_VAR_R_X 8xaa @ RbPlus
    {   3,    7,  421,  544,  217, } , // 8 pipes (8 PKRs) 16 bpe @ SW_VAR_R_X 8xaa @ RbPlus
    {   3,    0,  339,  636,  211, } , // 16 pipes (8 PKRs) 1 bpe @ SW_VAR_R_X 8xaa @ RbPlus
    {   3,    1,  339,  637,  212, } , // 16 pipes (8 PKRs) 2 bpe @ SW_VAR_R_X 8xaa @ RbPlus
    {   3,   39,  339,  638,  213, } , // 16 pipes (8 PKRs) 4 bpe @ SW_VAR_R_X 8xaa @ RbPlus
    {   3,    6,  339,  639,  214, } , // 16 pipes (8 PKRs) 8 bpe @ SW_VAR_R_X 8xaa @ RbPlus
    {   3,    7,  422,  224,  175, } , // 16 pipes (8 PKRs) 16 bpe @ SW_VAR_R_X 8xaa @ RbPlus
    {   3,    0,  339,  545,  218, } , // 32 pipes (8 PKRs) 1 bpe @ SW_VAR_R_X 8xaa @ RbPlus
    {   3,    1,  339,  498,  219, } , // 32 pipes (8 PKRs) 2 bpe @ SW_VAR_R_X 8xaa @ RbPlus
    {   3,   39,  339,  546,  220, } , // 32 pipes (8 PKRs) 4 bpe @ SW_VAR_R_X 8xaa @ RbPlus
    {   3,    6,  339,  500,  221, } , // 32 pipes (8 PKRs) 8 bpe @ SW_VAR_R_X 8xaa @ RbPlus
    {   3,    7,  339,  644,  222, } , // 32 pipes (8 PKRs) 16 bpe @ SW_VAR_R_X 8xaa @ RbPlus
    {   3,    0,  343,  645,  223, } , // 16 pipes (16 PKRs) 1 bpe @ SW_VAR_R_X 8xaa @ RbPlus
    {   3,    1,  343,  646,  224, } , // 16 pipes (16 PKRs) 2 bpe @ SW_VAR_R_X 8xaa @ RbPlus
    {   3,   39,  343,  647,  225, } , // 16 pipes (16 PKRs) 4 bpe @ SW_VAR_R_X 8xaa @ RbPlus
    {   3,    6,  341,  648,  226, } , // 16 pipes (16 PKRs) 8 bpe @ SW_VAR_R_X 8xaa @ RbPlus
    {   3,    7,  423,  286,  187, } , // 16 pipes (16 PKRs) 16 bpe @ SW_VAR_R_X 8xaa @ RbPlus
    {   3,    0,  343,  649,  218, } , // 32 pipes (16 PKRs) 1 bpe @ SW_VAR_R_X 8xaa @ RbPlus
    {   3,    1,  343,  650,  227, } , // 32 pipes (16 PKRs) 2 bpe @ SW_VAR_R_X 8xaa @ RbPlus
    {   3,   39,  343,  651,  220, } , // 32 pipes (16 PKRs) 4 bpe @ SW_VAR_R_X 8xaa @ RbPlus
    {   3,    6,  343,  652,  221, } , // 32 pipes (16 PKRs) 8 bpe @ SW_VAR_R_X 8xaa @ RbPlus
    {   3,    7,  341,  653,  228, } , // 32 pipes (16 PKRs) 16 bpe @ SW_VAR_R_X 8xaa @ RbPlus
    {   3,    0,  343,  654,  229, } , // 64 pipes (16 PKRs) 1 bpe @ SW_VAR_R_X 8xaa @ RbPlus
    {   3,    1,  343,  655,  230, } , // 64 pipes (16 PKRs) 2 bpe @ SW_VAR_R_X 8xaa @ RbPlus
    {   3,   39,  343,  656,  231, } , // 64 pipes (16 PKRs) 4 bpe @ SW_VAR_R_X 8xaa @ RbPlus
    {   3,    6,  343,  657,  232, } , // 64 pipes (16 PKRs) 8 bpe @ SW_VAR_R_X 8xaa @ RbPlus
    {   3,    7,  343,  658,  233, } , // 64 pipes (16 PKRs) 16 bpe @ SW_VAR_R_X 8xaa @ RbPlus
    {   3,    0,  346,  659,  234, } , // 32 pipes (32 PKRs) 1 bpe @ SW_VAR_R_X 8xaa @ RbPlus
    {   3,    1,  346,  660,  235, } , // 32 pipes (32 PKRs) 2 bpe @ SW_VAR_R_X 8xaa @ RbPlus
    {   3,   39,  346,  661,  236, } , // 32 pipes (32 PKRs) 4 bpe @ SW_VAR_R_X 8xaa @ RbPlus
    {   3,    6,  344,  662,  237, } , // 32 pipes (32 PKRs) 8 bpe @ SW_VAR_R_X 8xaa @ RbPlus
    {   3,    7,  345,  663,  238, } , // 32 pipes (32 PKRs) 16 bpe @ SW_VAR_R_X 8xaa @ RbPlus
    {   3,    0,  346,  664,  229, } , // 64 pipes (32 PKRs) 1 bpe @ SW_VAR_R_X 8xaa @ RbPlus
    {   3,    1,  346,  665,  230, } , // 64 pipes (32 PKRs) 2 bpe @ SW_VAR_R_X 8xaa @ RbPlus
    {   3,   39,  346,  666,  231, } , // 64 pipes (32 PKRs) 4 bpe @ SW_VAR_R_X 8xaa @ RbPlus
    {   3,    6,  346,  667,  232, } , // 64 pipes (32 PKRs) 8 bpe @ SW_VAR_R_X 8xaa @ RbPlus
    {   3,    7,  344,  668,  204, } , // 64 pipes (32 PKRs) 16 bpe @ SW_VAR_R_X 8xaa @ RbPlus
};

const ADDR_SW_PATINFO GFX10_SW_VAR_Z_X_1xaa_RBPLUS_PATINFO[] =
{
    {   2,    8,  270,  183,    0, } , // 1 pipes (1 PKRs) 1 bpe @ SW_VAR_Z_X 1xaa @ RbPlus
    {   2,    9,  271,  184,    0, } , // 1 pipes (1 PKRs) 2 bpe @ SW_VAR_Z_X 1xaa @ RbPlus
    {   2,   10,  272,  185,    0, } , // 1 pipes (1 PKRs) 4 bpe @ SW_VAR_Z_X 1xaa @ RbPlus
    {   2,   11,  273,  186,    0, } , // 1 pipes (1 PKRs) 8 bpe @ SW_VAR_Z_X 1xaa @ RbPlus
    {   2,    7,  274,  187,    0, } , // 1 pipes (1 PKRs) 16 bpe @ SW_VAR_Z_X 1xaa @ RbPlus
    {   3,    8,  275,  188,    0, } , // 2 pipes (1-2 PKRs) 1 bpe @ SW_VAR_Z_X 1xaa @ RbPlus
    {   3,    9,  276,  189,    0, } , // 2 pipes (1-2 PKRs) 2 bpe @ SW_VAR_Z_X 1xaa @ RbPlus
    {   3,   10,  277,  190,    0, } , // 2 pipes (1-2 PKRs) 4 bpe @ SW_VAR_Z_X 1xaa @ RbPlus
    {   3,   11,  278,  191,    0, } , // 2 pipes (1-2 PKRs) 8 bpe @ SW_VAR_Z_X 1xaa @ RbPlus
    {   3,    7,  279,  192,    0, } , // 2 pipes (1-2 PKRs) 16 bpe @ SW_VAR_Z_X 1xaa @ RbPlus
    {   3,    8,  280,  193,    0, } , // 4 pipes (1-2 PKRs) 1 bpe @ SW_VAR_Z_X 1xaa @ RbPlus
    {   3,    9,  281,  194,    0, } , // 4 pipes (1-2 PKRs) 2 bpe @ SW_VAR_Z_X 1xaa @ RbPlus
    {   3,   10,  282,  195,    0, } , // 4 pipes (1-2 PKRs) 4 bpe @ SW_VAR_Z_X 1xaa @ RbPlus
    {   3,   11,  283,  196,    0, } , // 4 pipes (1-2 PKRs) 8 bpe @ SW_VAR_Z_X 1xaa @ RbPlus
    {   3,    7,  284,  197,    0, } , // 4 pipes (1-2 PKRs) 16 bpe @ SW_VAR_Z_X 1xaa @ RbPlus
    {   3,    8,  285,  198,    1, } , // 8 pipes (2 PKRs) 1 bpe @ SW_VAR_Z_X 1xaa @ RbPlus
    {   3,    9,  286,  199,    2, } , // 8 pipes (2 PKRs) 2 bpe @ SW_VAR_Z_X 1xaa @ RbPlus
    {   3,   10,  287,  200,    3, } , // 8 pipes (2 PKRs) 4 bpe @ SW_VAR_Z_X 1xaa @ RbPlus
    {   3,   11,  288,  201,    4, } , // 8 pipes (2 PKRs) 8 bpe @ SW_VAR_Z_X 1xaa @ RbPlus
    {   3,    7,  289,  202,    5, } , // 8 pipes (2 PKRs) 16 bpe @ SW_VAR_Z_X 1xaa @ RbPlus
    {   3,    8,  290,  203,    0, } , // 4 pipes (4 PKRs) 1 bpe @ SW_VAR_Z_X 1xaa @ RbPlus
    {   3,    9,  291,  204,    0, } , // 4 pipes (4 PKRs) 2 bpe @ SW_VAR_Z_X 1xaa @ RbPlus
    {   3,   10,  292,  205,    0, } , // 4 pipes (4 PKRs) 4 bpe @ SW_VAR_Z_X 1xaa @ RbPlus
    {   3,   11,  293,  206,    0, } , // 4 pipes (4 PKRs) 8 bpe @ SW_VAR_Z_X 1xaa @ RbPlus
    {   3,    7,  294,  207,    0, } , // 4 pipes (4 PKRs) 16 bpe @ SW_VAR_Z_X 1xaa @ RbPlus
    {   3,    8,  295,  208,    6, } , // 8 pipes (4 PKRs) 1 bpe @ SW_VAR_Z_X 1xaa @ RbPlus
    {   3,    9,  296,  209,    2, } , // 8 pipes (4 PKRs) 2 bpe @ SW_VAR_Z_X 1xaa @ RbPlus
    {   3,   10,  297,  210,    7, } , // 8 pipes (4 PKRs) 4 bpe @ SW_VAR_Z_X 1xaa @ RbPlus
    {   3,   11,  298,  211,    4, } , // 8 pipes (4 PKRs) 8 bpe @ SW_VAR_Z_X 1xaa @ RbPlus
    {   3,    7,  299,  212,    8, } , // 8 pipes (4 PKRs) 16 bpe @ SW_VAR_Z_X 1xaa @ RbPlus
    {   3,    8,  300,  213,    9, } , // 16 pipes (4 PKRs) 1 bpe @ SW_VAR_Z_X 1xaa @ RbPlus
    {   3,    9,  300,  214,   10, } , // 16 pipes (4 PKRs) 2 bpe @ SW_VAR_Z_X 1xaa @ RbPlus
    {   3,   10,  300,  215,   11, } , // 16 pipes (4 PKRs) 4 bpe @ SW_VAR_Z_X 1xaa @ RbPlus
    {   3,   11,  300,  216,   12, } , // 16 pipes (4 PKRs) 8 bpe @ SW_VAR_Z_X 1xaa @ RbPlus
    {   3,    7,  300,  217,   13, } , // 16 pipes (4 PKRs) 16 bpe @ SW_VAR_Z_X 1xaa @ RbPlus
    {   3,    8,  301,  218,   14, } , // 8 pipes (8 PKRs) 1 bpe @ SW_VAR_Z_X 1xaa @ RbPlus
    {   3,    9,  302,  219,   14, } , // 8 pipes (8 PKRs) 2 bpe @ SW_VAR_Z_X 1xaa @ RbPlus
    {   3,   10,  303,  220,   14, } , // 8 pipes (8 PKRs) 4 bpe @ SW_VAR_Z_X 1xaa @ RbPlus
    {   3,   11,  304,  221,   15, } , // 8 pipes (8 PKRs) 8 bpe @ SW_VAR_Z_X 1xaa @ RbPlus
    {   3,    7,  305,  222,   15, } , // 8 pipes (8 PKRs) 16 bpe @ SW_VAR_Z_X 1xaa @ RbPlus
    {   3,    8,  306,  213,    9, } , // 16 pipes (8 PKRs) 1 bpe @ SW_VAR_Z_X 1xaa @ RbPlus
    {   3,    9,  306,  223,   16, } , // 16 pipes (8 PKRs) 2 bpe @ SW_VAR_Z_X 1xaa @ RbPlus
    {   3,   10,  306,  215,   11, } , // 16 pipes (8 PKRs) 4 bpe @ SW_VAR_Z_X 1xaa @ RbPlus
    {   3,   11,  307,  216,   17, } , // 16 pipes (8 PKRs) 8 bpe @ SW_VAR_Z_X 1xaa @ RbPlus
    {   3,    7,  307,  224,   13, } , // 16 pipes (8 PKRs) 16 bpe @ SW_VAR_Z_X 1xaa @ RbPlus
    {   3,    8,  306,  225,   18, } , // 32 pipes (8 PKRs) 1 bpe @ SW_VAR_Z_X 1xaa @ RbPlus
    {   3,    9,  306,  226,   19, } , // 32 pipes (8 PKRs) 2 bpe @ SW_VAR_Z_X 1xaa @ RbPlus
    {   3,   10,  306,  227,   20, } , // 32 pipes (8 PKRs) 4 bpe @ SW_VAR_Z_X 1xaa @ RbPlus
    {   3,   11,  307,  228,   21, } , // 32 pipes (8 PKRs) 8 bpe @ SW_VAR_Z_X 1xaa @ RbPlus
    {   3,    7,  307,  229,   22, } , // 32 pipes (8 PKRs) 16 bpe @ SW_VAR_Z_X 1xaa @ RbPlus
    {   3,    8,  308,  230,   23, } , // 16 pipes (16 PKRs) 1 bpe @ SW_VAR_Z_X 1xaa @ RbPlus
    {   3,    9,  308,  231,   24, } , // 16 pipes (16 PKRs) 2 bpe @ SW_VAR_Z_X 1xaa @ RbPlus
    {   3,   10,  308,  232,   25, } , // 16 pipes (16 PKRs) 4 bpe @ SW_VAR_Z_X 1xaa @ RbPlus
    {   3,   11,  309,  233,   26, } , // 16 pipes (16 PKRs) 8 bpe @ SW_VAR_Z_X 1xaa @ RbPlus
    {   3,    7,  309,  234,   27, } , // 16 pipes (16 PKRs) 16 bpe @ SW_VAR_Z_X 1xaa @ RbPlus
    {   3,    8,  308,  235,   28, } , // 32 pipes (16 PKRs) 1 bpe @ SW_VAR_Z_X 1xaa @ RbPlus
    {   3,    9,  308,  236,   19, } , // 32 pipes (16 PKRs) 2 bpe @ SW_VAR_Z_X 1xaa @ RbPlus
    {   3,   10,  308,  237,   29, } , // 32 pipes (16 PKRs) 4 bpe @ SW_VAR_Z_X 1xaa @ RbPlus
    {   3,   11,  309,  238,   30, } , // 32 pipes (16 PKRs) 8 bpe @ SW_VAR_Z_X 1xaa @ RbPlus
    {   3,    7,  309,  239,   31, } , // 32 pipes (16 PKRs) 16 bpe @ SW_VAR_Z_X 1xaa @ RbPlus
    {   3,    8,  308,  240,   32, } , // 64 pipes (16 PKRs) 1 bpe @ SW_VAR_Z_X 1xaa @ RbPlus
    {   3,    9,  308,  241,   33, } , // 64 pipes (16 PKRs) 2 bpe @ SW_VAR_Z_X 1xaa @ RbPlus
    {   3,   10,  308,  242,   34, } , // 64 pipes (16 PKRs) 4 bpe @ SW_VAR_Z_X 1xaa @ RbPlus
    {   3,   11,  309,  243,   35, } , // 64 pipes (16 PKRs) 8 bpe @ SW_VAR_Z_X 1xaa @ RbPlus
    {   3,    7,  309,  244,   36, } , // 64 pipes (16 PKRs) 16 bpe @ SW_VAR_Z_X 1xaa @ RbPlus
    {   3,    8,  310,  245,   37, } , // 32 pipes (32 PKRs) 1 bpe @ SW_VAR_Z_X 1xaa @ RbPlus
    {   3,    9,  310,  246,   38, } , // 32 pipes (32 PKRs) 2 bpe @ SW_VAR_Z_X 1xaa @ RbPlus
    {   3,   10,  310,  247,   39, } , // 32 pipes (32 PKRs) 4 bpe @ SW_VAR_Z_X 1xaa @ RbPlus
    {   3,   11,  311,  248,   40, } , // 32 pipes (32 PKRs) 8 bpe @ SW_VAR_Z_X 1xaa @ RbPlus
    {   3,    7,  311,  249,   41, } , // 32 pipes (32 PKRs) 16 bpe @ SW_VAR_Z_X 1xaa @ RbPlus
    {   3,    8,  310,  250,   32, } , // 64 pipes (32 PKRs) 1 bpe @ SW_VAR_Z_X 1xaa @ RbPlus
    {   3,    9,  310,  251,   42, } , // 64 pipes (32 PKRs) 2 bpe @ SW_VAR_Z_X 1xaa @ RbPlus
    {   3,   10,  310,  252,   34, } , // 64 pipes (32 PKRs) 4 bpe @ SW_VAR_Z_X 1xaa @ RbPlus
    {   3,   11,  311,  253,   43, } , // 64 pipes (32 PKRs) 8 bpe @ SW_VAR_Z_X 1xaa @ RbPlus
    {   3,    7,  311,  254,   44, } , // 64 pipes (32 PKRs) 16 bpe @ SW_VAR_Z_X 1xaa @ RbPlus
};

const ADDR_SW_PATINFO GFX10_SW_VAR_Z_X_2xaa_RBPLUS_PATINFO[] =
{
    {   2,   13,  312,  255,    0, } , // 1 pipes (1 PKRs) 1 bpe @ SW_VAR_Z_X 2xaa @ RbPlus
    {   2,   14,  272,  185,    0, } , // 1 pipes (1 PKRs) 2 bpe @ SW_VAR_Z_X 2xaa @ RbPlus
    {   3,   15,  313,  256,    0, } , // 1 pipes (1 PKRs) 4 bpe @ SW_VAR_Z_X 2xaa @ RbPlus
    {   3,   16,  273,  257,    0, } , // 1 pipes (1 PKRs) 8 bpe @ SW_VAR_Z_X 2xaa @ RbPlus
    {   3,   17,  314,  258,    0, } , // 1 pipes (1 PKRs) 16 bpe @ SW_VAR_Z_X 2xaa @ RbPlus
    {   3,   13,  276,  189,    0, } , // 2 pipes (1-2 PKRs) 1 bpe @ SW_VAR_Z_X 2xaa @ RbPlus
    {   3,   14,  277,  190,    0, } , // 2 pipes (1-2 PKRs) 2 bpe @ SW_VAR_Z_X 2xaa @ RbPlus
    {   3,   15,  315,  259,    0, } , // 2 pipes (1-2 PKRs) 4 bpe @ SW_VAR_Z_X 2xaa @ RbPlus
    {   3,   16,  278,  260,    0, } , // 2 pipes (1-2 PKRs) 8 bpe @ SW_VAR_Z_X 2xaa @ RbPlus
    {   3,   17,  316,  261,    0, } , // 2 pipes (1-2 PKRs) 16 bpe @ SW_VAR_Z_X 2xaa @ RbPlus
    {   3,   13,  281,  262,    0, } , // 4 pipes (1-2 PKRs) 1 bpe @ SW_VAR_Z_X 2xaa @ RbPlus
    {   3,   14,  282,  195,    0, } , // 4 pipes (1-2 PKRs) 2 bpe @ SW_VAR_Z_X 2xaa @ RbPlus
    {   3,   15,  282,  263,    0, } , // 4 pipes (1-2 PKRs) 4 bpe @ SW_VAR_Z_X 2xaa @ RbPlus
    {   3,   16,  317,  264,    0, } , // 4 pipes (1-2 PKRs) 8 bpe @ SW_VAR_Z_X 2xaa @ RbPlus
    {   3,   17,  284,  265,    0, } , // 4 pipes (1-2 PKRs) 16 bpe @ SW_VAR_Z_X 2xaa @ RbPlus
    {   3,   13,  286,  209,    2, } , // 8 pipes (2 PKRs) 1 bpe @ SW_VAR_Z_X 2xaa @ RbPlus
    {   3,   14,  287,  266,    3, } , // 8 pipes (2 PKRs) 2 bpe @ SW_VAR_Z_X 2xaa @ RbPlus
    {   3,   15,  287,  210,   45, } , // 8 pipes (2 PKRs) 4 bpe @ SW_VAR_Z_X 2xaa @ RbPlus
    {   3,   16,  288,  211,   46, } , // 8 pipes (2 PKRs) 8 bpe @ SW_VAR_Z_X 2xaa @ RbPlus
    {   3,   17,  289,  267,   47, } , // 8 pipes (2 PKRs) 16 bpe @ SW_VAR_Z_X 2xaa @ RbPlus
    {   3,   13,  291,  268,    0, } , // 4 pipes (4 PKRs) 1 bpe @ SW_VAR_Z_X 2xaa @ RbPlus
    {   3,   14,  292,  205,    0, } , // 4 pipes (4 PKRs) 2 bpe @ SW_VAR_Z_X 2xaa @ RbPlus
    {   3,   15,  292,  269,    0, } , // 4 pipes (4 PKRs) 4 bpe @ SW_VAR_Z_X 2xaa @ RbPlus
    {   3,   16,  293,  270,    0, } , // 4 pipes (4 PKRs) 8 bpe @ SW_VAR_Z_X 2xaa @ RbPlus
    {   3,   17,  294,  271,    0, } , // 4 pipes (4 PKRs) 16 bpe @ SW_VAR_Z_X 2xaa @ RbPlus
    {   3,   13,  296,  209,    2, } , // 8 pipes (4 PKRs) 1 bpe @ SW_VAR_Z_X 2xaa @ RbPlus
    {   3,   14,  297,  210,    7, } , // 8 pipes (4 PKRs) 2 bpe @ SW_VAR_Z_X 2xaa @ RbPlus
    {   3,   15,  297,  210,   45, } , // 8 pipes (4 PKRs) 4 bpe @ SW_VAR_Z_X 2xaa @ RbPlus
    {   3,   16,  298,  211,   46, } , // 8 pipes (4 PKRs) 8 bpe @ SW_VAR_Z_X 2xaa @ RbPlus
    {   3,   17,  299,  212,   47, } , // 8 pipes (4 PKRs) 16 bpe @ SW_VAR_Z_X 2xaa @ RbPlus
    {   3,   13,  300,  272,   48, } , // 16 pipes (4 PKRs) 1 bpe @ SW_VAR_Z_X 2xaa @ RbPlus
    {   3,   14,  300,  273,   11, } , // 16 pipes (4 PKRs) 2 bpe @ SW_VAR_Z_X 2xaa @ RbPlus
    {   3,   15,  300,  273,   49, } , // 16 pipes (4 PKRs) 4 bpe @ SW_VAR_Z_X 2xaa @ RbPlus
    {   3,   16,  300,  274,   50, } , // 16 pipes (4 PKRs) 8 bpe @ SW_VAR_Z_X 2xaa @ RbPlus
    {   3,   17,  300,  275,   51, } , // 16 pipes (4 PKRs) 16 bpe @ SW_VAR_Z_X 2xaa @ RbPlus
    {   3,   13,  302,  219,   14, } , // 8 pipes (8 PKRs) 1 bpe @ SW_VAR_Z_X 2xaa @ RbPlus
    {   3,   14,  303,  220,   14, } , // 8 pipes (8 PKRs) 2 bpe @ SW_VAR_Z_X 2xaa @ RbPlus
    {   3,   15,  303,  276,   14, } , // 8 pipes (8 PKRs) 4 bpe @ SW_VAR_Z_X 2xaa @ RbPlus
    {   3,   16,  304,  277,   15, } , // 8 pipes (8 PKRs) 8 bpe @ SW_VAR_Z_X 2xaa @ RbPlus
    {   3,   17,  305,  278,   15, } , // 8 pipes (8 PKRs) 16 bpe @ SW_VAR_Z_X 2xaa @ RbPlus
    {   3,   13,  306,  279,   48, } , // 16 pipes (8 PKRs) 1 bpe @ SW_VAR_Z_X 2xaa @ RbPlus
    {   3,   14,  306,  215,   11, } , // 16 pipes (8 PKRs) 2 bpe @ SW_VAR_Z_X 2xaa @ RbPlus
    {   3,   15,  306,  280,   49, } , // 16 pipes (8 PKRs) 4 bpe @ SW_VAR_Z_X 2xaa @ RbPlus
    {   3,   16,  307,  281,   52, } , // 16 pipes (8 PKRs) 8 bpe @ SW_VAR_Z_X 2xaa @ RbPlus
    {   3,   17,  307,  224,   53, } , // 16 pipes (8 PKRs) 16 bpe @ SW_VAR_Z_X 2xaa @ RbPlus
    {   3,   13,  306,  236,   19, } , // 32 pipes (8 PKRs) 1 bpe @ SW_VAR_Z_X 2xaa @ RbPlus
    {   3,   14,  306,  237,   54, } , // 32 pipes (8 PKRs) 2 bpe @ SW_VAR_Z_X 2xaa @ RbPlus
    {   3,   15,  306,  237,   55, } , // 32 pipes (8 PKRs) 4 bpe @ SW_VAR_Z_X 2xaa @ RbPlus
    {   3,   16,  307,  282,   56, } , // 32 pipes (8 PKRs) 8 bpe @ SW_VAR_Z_X 2xaa @ RbPlus
    {   3,   17,  307,  283,   57, } , // 32 pipes (8 PKRs) 16 bpe @ SW_VAR_Z_X 2xaa @ RbPlus
    {   3,   13,  308,  284,   24, } , // 16 pipes (16 PKRs) 1 bpe @ SW_VAR_Z_X 2xaa @ RbPlus
    {   3,   14,  308,  232,   25, } , // 16 pipes (16 PKRs) 2 bpe @ SW_VAR_Z_X 2xaa @ RbPlus
    {   3,   15,  308,  285,   58, } , // 16 pipes (16 PKRs) 4 bpe @ SW_VAR_Z_X 2xaa @ RbPlus
    {   3,   16,  309,  233,   59, } , // 16 pipes (16 PKRs) 8 bpe @ SW_VAR_Z_X 2xaa @ RbPlus
    {   3,   17,  309,  286,   60, } , // 16 pipes (16 PKRs) 16 bpe @ SW_VAR_Z_X 2xaa @ RbPlus
    {   3,   13,  308,  236,   19, } , // 32 pipes (16 PKRs) 1 bpe @ SW_VAR_Z_X 2xaa @ RbPlus
    {   3,   14,  308,  237,   29, } , // 32 pipes (16 PKRs) 2 bpe @ SW_VAR_Z_X 2xaa @ RbPlus
    {   3,   15,  308,  237,   55, } , // 32 pipes (16 PKRs) 4 bpe @ SW_VAR_Z_X 2xaa @ RbPlus
    {   3,   16,  309,  238,   56, } , // 32 pipes (16 PKRs) 8 bpe @ SW_VAR_Z_X 2xaa @ RbPlus
    {   3,   17,  309,  239,   61, } , // 32 pipes (16 PKRs) 16 bpe @ SW_VAR_Z_X 2xaa @ RbPlus
    {   3,   13,  308,  241,   62, } , // 64 pipes (16 PKRs) 1 bpe @ SW_VAR_Z_X 2xaa @ RbPlus
    {   3,   14,  308,  242,   34, } , // 64 pipes (16 PKRs) 2 bpe @ SW_VAR_Z_X 2xaa @ RbPlus
    {   3,   15,  308,  242,   63, } , // 64 pipes (16 PKRs) 4 bpe @ SW_VAR_Z_X 2xaa @ RbPlus
    {   3,   16,  309,  243,   64, } , // 64 pipes (16 PKRs) 8 bpe @ SW_VAR_Z_X 2xaa @ RbPlus
    {   3,   17,  309,  244,   65, } , // 64 pipes (16 PKRs) 16 bpe @ SW_VAR_Z_X 2xaa @ RbPlus
    {   3,   13,  310,  246,   38, } , // 32 pipes (32 PKRs) 1 bpe @ SW_VAR_Z_X 2xaa @ RbPlus
    {   3,   14,  310,  247,   39, } , // 32 pipes (32 PKRs) 2 bpe @ SW_VAR_Z_X 2xaa @ RbPlus
    {   3,   15,  310,  247,   66, } , // 32 pipes (32 PKRs) 4 bpe @ SW_VAR_Z_X 2xaa @ RbPlus
    {   3,   16,  318,  287,   67, } , // 32 pipes (32 PKRs) 8 bpe @ SW_VAR_Z_X 2xaa @ RbPlus
    {   3,   17,  318,  288,   68, } , // 32 pipes (32 PKRs) 16 bpe @ SW_VAR_Z_X 2xaa @ RbPlus
    {   3,   13,  310,  251,   62, } , // 64 pipes (32 PKRs) 1 bpe @ SW_VAR_Z_X 2xaa @ RbPlus
    {   3,   14,  310,  252,   34, } , // 64 pipes (32 PKRs) 2 bpe @ SW_VAR_Z_X 2xaa @ RbPlus
    {   3,   15,  310,  252,   63, } , // 64 pipes (32 PKRs) 4 bpe @ SW_VAR_Z_X 2xaa @ RbPlus
    {   3,   16,  318,  289,   69, } , // 64 pipes (32 PKRs) 8 bpe @ SW_VAR_Z_X 2xaa @ RbPlus
    {   3,   17,  318,  290,   65, } , // 64 pipes (32 PKRs) 16 bpe @ SW_VAR_Z_X 2xaa @ RbPlus
};

const ADDR_SW_PATINFO GFX10_SW_VAR_Z_X_4xaa_RBPLUS_PATINFO[] =
{
    {   2,   18,  272,  185,    0, } , // 1 pipes (1 PKRs) 1 bpe @ SW_VAR_Z_X 4xaa @ RbPlus
    {   3,   19,  272,  291,    0, } , // 1 pipes (1 PKRs) 2 bpe @ SW_VAR_Z_X 4xaa @ RbPlus
    {   3,   20,  272,  292,    0, } , // 1 pipes (1 PKRs) 4 bpe @ SW_VAR_Z_X 4xaa @ RbPlus
    {   3,   21,  273,  293,    0, } , // 1 pipes (1 PKRs) 8 bpe @ SW_VAR_Z_X 4xaa @ RbPlus
    {   3,   22,  274,  294,    0, } , // 1 pipes (1 PKRs) 16 bpe @ SW_VAR_Z_X 4xaa @ RbPlus
    {   3,   18,  277,  190,    0, } , // 2 pipes (1-2 PKRs) 1 bpe @ SW_VAR_Z_X 4xaa @ RbPlus
    {   3,   19,  315,  259,    0, } , // 2 pipes (1-2 PKRs) 2 bpe @ SW_VAR_Z_X 4xaa @ RbPlus
    {   3,   20,  277,  295,    0, } , // 2 pipes (1-2 PKRs) 4 bpe @ SW_VAR_Z_X 4xaa @ RbPlus
    {   3,   21,  319,  296,    0, } , // 2 pipes (1-2 PKRs) 8 bpe @ SW_VAR_Z_X 4xaa @ RbPlus
    {   3,   22,  279,  297,    0, } , // 2 pipes (1-2 PKRs) 16 bpe @ SW_VAR_Z_X 4xaa @ RbPlus
    {   3,   18,  282,  195,    0, } , // 4 pipes (1-2 PKRs) 1 bpe @ SW_VAR_Z_X 4xaa @ RbPlus
    {   3,   19,  282,  298,    0, } , // 4 pipes (1-2 PKRs) 2 bpe @ SW_VAR_Z_X 4xaa @ RbPlus
    {   3,   20,  282,  299,    0, } , // 4 pipes (1-2 PKRs) 4 bpe @ SW_VAR_Z_X 4xaa @ RbPlus
    {   3,   21,  283,  300,    0, } , // 4 pipes (1-2 PKRs) 8 bpe @ SW_VAR_Z_X 4xaa @ RbPlus
    {   3,   22,  284,  301,    0, } , // 4 pipes (1-2 PKRs) 16 bpe @ SW_VAR_Z_X 4xaa @ RbPlus
    {   3,   18,  287,  200,    3, } , // 8 pipes (2 PKRs) 1 bpe @ SW_VAR_Z_X 4xaa @ RbPlus
    {   3,   19,  287,  302,   45, } , // 8 pipes (2 PKRs) 2 bpe @ SW_VAR_Z_X 4xaa @ RbPlus
    {   3,   20,  287,  303,   70, } , // 8 pipes (2 PKRs) 4 bpe @ SW_VAR_Z_X 4xaa @ RbPlus
    {   3,   21,  289,  304,   71, } , // 8 pipes (2 PKRs) 8 bpe @ SW_VAR_Z_X 4xaa @ RbPlus
    {   3,   22,  289,  305,   72, } , // 8 pipes (2 PKRs) 16 bpe @ SW_VAR_Z_X 4xaa @ RbPlus
    {   3,   18,  292,  205,    0, } , // 4 pipes (4 PKRs) 1 bpe @ SW_VAR_Z_X 4xaa @ RbPlus
    {   3,   19,  292,  306,    0, } , // 4 pipes (4 PKRs) 2 bpe @ SW_VAR_Z_X 4xaa @ RbPlus
    {   3,   20,  292,  307,    0, } , // 4 pipes (4 PKRs) 4 bpe @ SW_VAR_Z_X 4xaa @ RbPlus
    {   3,   21,  320,  308,    0, } , // 4 pipes (4 PKRs) 8 bpe @ SW_VAR_Z_X 4xaa @ RbPlus
    {   3,   22,  321,  309,    0, } , // 4 pipes (4 PKRs) 16 bpe @ SW_VAR_Z_X 4xaa @ RbPlus
    {   3,   18,  297,  210,    7, } , // 8 pipes (4 PKRs) 1 bpe @ SW_VAR_Z_X 4xaa @ RbPlus
    {   3,   19,  297,  210,   45, } , // 8 pipes (4 PKRs) 2 bpe @ SW_VAR_Z_X 4xaa @ RbPlus
    {   3,   20,  297,  310,   45, } , // 8 pipes (4 PKRs) 4 bpe @ SW_VAR_Z_X 4xaa @ RbPlus
    {   3,   21,  298,  311,   71, } , // 8 pipes (4 PKRs) 8 bpe @ SW_VAR_Z_X 4xaa @ RbPlus
    {   3,   22,  299,  312,   47, } , // 8 pipes (4 PKRs) 16 bpe @ SW_VAR_Z_X 4xaa @ RbPlus
    {   3,   18,  300,  215,   11, } , // 16 pipes (4 PKRs) 1 bpe @ SW_VAR_Z_X 4xaa @ RbPlus
    {   3,   19,  300,  215,   73, } , // 16 pipes (4 PKRs) 2 bpe @ SW_VAR_Z_X 4xaa @ RbPlus
    {   3,   20,  300,  215,   74, } , // 16 pipes (4 PKRs) 4 bpe @ SW_VAR_Z_X 4xaa @ RbPlus
    {   3,   21,  300,  216,   75, } , // 16 pipes (4 PKRs) 8 bpe @ SW_VAR_Z_X 4xaa @ RbPlus
    {   3,   22,  300,  217,   76, } , // 16 pipes (4 PKRs) 16 bpe @ SW_VAR_Z_X 4xaa @ RbPlus
    {   3,   18,  303,  220,   14, } , // 8 pipes (8 PKRs) 1 bpe @ SW_VAR_Z_X 4xaa @ RbPlus
    {   3,   19,  303,  276,   14, } , // 8 pipes (8 PKRs) 2 bpe @ SW_VAR_Z_X 4xaa @ RbPlus
    {   3,   20,  303,  313,   14, } , // 8 pipes (8 PKRs) 4 bpe @ SW_VAR_Z_X 4xaa @ RbPlus
    {   3,   21,  305,  314,   15, } , // 8 pipes (8 PKRs) 8 bpe @ SW_VAR_Z_X 4xaa @ RbPlus
    {   3,   22,  322,  315,   15, } , // 8 pipes (8 PKRs) 16 bpe @ SW_VAR_Z_X 4xaa @ RbPlus
    {   3,   18,  306,  215,   11, } , // 16 pipes (8 PKRs) 1 bpe @ SW_VAR_Z_X 4xaa @ RbPlus
    {   3,   19,  306,  232,   77, } , // 16 pipes (8 PKRs) 2 bpe @ SW_VAR_Z_X 4xaa @ RbPlus
    {   3,   20,  306,  215,   78, } , // 16 pipes (8 PKRs) 4 bpe @ SW_VAR_Z_X 4xaa @ RbPlus
    {   3,   21,  307,  216,   79, } , // 16 pipes (8 PKRs) 8 bpe @ SW_VAR_Z_X 4xaa @ RbPlus
    {   3,   22,  307,  224,   80, } , // 16 pipes (8 PKRs) 16 bpe @ SW_VAR_Z_X 4xaa @ RbPlus
    {   3,   18,  306,  227,   20, } , // 32 pipes (8 PKRs) 1 bpe @ SW_VAR_Z_X 4xaa @ RbPlus
    {   3,   19,  306,  316,   55, } , // 32 pipes (8 PKRs) 2 bpe @ SW_VAR_Z_X 4xaa @ RbPlus
    {   3,   20,  306,  227,   81, } , // 32 pipes (8 PKRs) 4 bpe @ SW_VAR_Z_X 4xaa @ RbPlus
    {   3,   21,  307,  317,   82, } , // 32 pipes (8 PKRs) 8 bpe @ SW_VAR_Z_X 4xaa @ RbPlus
    {   3,   22,  307,  229,   83, } , // 32 pipes (8 PKRs) 16 bpe @ SW_VAR_Z_X 4xaa @ RbPlus
    {   3,   18,  308,  232,   25, } , // 16 pipes (16 PKRs) 1 bpe @ SW_VAR_Z_X 4xaa @ RbPlus
    {   3,   19,  308,  232,   84, } , // 16 pipes (16 PKRs) 2 bpe @ SW_VAR_Z_X 4xaa @ RbPlus
    {   3,   20,  308,  318,   84, } , // 16 pipes (16 PKRs) 4 bpe @ SW_VAR_Z_X 4xaa @ RbPlus
    {   3,   21,  323,  319,   85, } , // 16 pipes (16 PKRs) 8 bpe @ SW_VAR_Z_X 4xaa @ RbPlus
    {   3,   22,  323,  320,   86, } , // 16 pipes (16 PKRs) 16 bpe @ SW_VAR_Z_X 4xaa @ RbPlus
    {   3,   18,  308,  237,   29, } , // 32 pipes (16 PKRs) 1 bpe @ SW_VAR_Z_X 4xaa @ RbPlus
    {   3,   19,  308,  237,   55, } , // 32 pipes (16 PKRs) 2 bpe @ SW_VAR_Z_X 4xaa @ RbPlus
    {   3,   20,  308,  237,   87, } , // 32 pipes (16 PKRs) 4 bpe @ SW_VAR_Z_X 4xaa @ RbPlus
    {   3,   21,  323,  321,   88, } , // 32 pipes (16 PKRs) 8 bpe @ SW_VAR_Z_X 4xaa @ RbPlus
    {   3,   22,  323,  322,   89, } , // 32 pipes (16 PKRs) 16 bpe @ SW_VAR_Z_X 4xaa @ RbPlus
    {   3,   18,  308,  242,   34, } , // 64 pipes (16 PKRs) 1 bpe @ SW_VAR_Z_X 4xaa @ RbPlus
    {   3,   19,  308,  242,   90, } , // 64 pipes (16 PKRs) 2 bpe @ SW_VAR_Z_X 4xaa @ RbPlus
    {   3,   20,  308,  242,   91, } , // 64 pipes (16 PKRs) 4 bpe @ SW_VAR_Z_X 4xaa @ RbPlus
    {   3,   21,  323,  323,   92, } , // 64 pipes (16 PKRs) 8 bpe @ SW_VAR_Z_X 4xaa @ RbPlus
    {   3,   22,  323,  324,   93, } , // 64 pipes (16 PKRs) 16 bpe @ SW_VAR_Z_X 4xaa @ RbPlus
    {   3,   18,  310,  247,   39, } , // 32 pipes (32 PKRs) 1 bpe @ SW_VAR_Z_X 4xaa @ RbPlus
    {   3,   19,  310,  247,   66, } , // 32 pipes (32 PKRs) 2 bpe @ SW_VAR_Z_X 4xaa @ RbPlus
    {   3,   20,  310,  247,   94, } , // 32 pipes (32 PKRs) 4 bpe @ SW_VAR_Z_X 4xaa @ RbPlus
    {   3,   21,  324,  325,   95, } , // 32 pipes (32 PKRs) 8 bpe @ SW_VAR_Z_X 4xaa @ RbPlus
    {   3,   22,  324,  326,   96, } , // 32 pipes (32 PKRs) 16 bpe @ SW_VAR_Z_X 4xaa @ RbPlus
    {   3,   18,  310,  252,   34, } , // 64 pipes (32 PKRs) 1 bpe @ SW_VAR_Z_X 4xaa @ RbPlus
    {   3,   19,  310,  252,   97, } , // 64 pipes (32 PKRs) 2 bpe @ SW_VAR_Z_X 4xaa @ RbPlus
    {   3,   20,  310,  252,   98, } , // 64 pipes (32 PKRs) 4 bpe @ SW_VAR_Z_X 4xaa @ RbPlus
    {   3,   21,  324,  327,   99, } , // 64 pipes (32 PKRs) 8 bpe @ SW_VAR_Z_X 4xaa @ RbPlus
    {   3,   22,  324,  328,  100, } , // 64 pipes (32 PKRs) 16 bpe @ SW_VAR_Z_X 4xaa @ RbPlus
};

const ADDR_SW_PATINFO GFX10_SW_VAR_Z_X_8xaa_RBPLUS_PATINFO[] =
{
    {   3,   23,  313,  256,    0, } , // 1 pipes (1 PKRs) 1 bpe @ SW_VAR_Z_X 8xaa @ RbPlus
    {   3,   24,  272,  292,    0, } , // 1 pipes (1 PKRs) 2 bpe @ SW_VAR_Z_X 8xaa @ RbPlus
    {   3,   25,  325,  292,    0, } , // 1 pipes (1 PKRs) 4 bpe @ SW_VAR_Z_X 8xaa @ RbPlus
    {   3,   26,  326,  329,    0, } , // 1 pipes (1 PKRs) 8 bpe @ SW_VAR_Z_X 8xaa @ RbPlus
    {   3,   27,  327,  294,    0, } , // 1 pipes (1 PKRs) 16 bpe @ SW_VAR_Z_X 8xaa @ RbPlus
    {   3,   23,  315,  259,    0, } , // 2 pipes (1-2 PKRs) 1 bpe @ SW_VAR_Z_X 8xaa @ RbPlus
    {   3,   24,  277,  295,    0, } , // 2 pipes (1-2 PKRs) 2 bpe @ SW_VAR_Z_X 8xaa @ RbPlus
    {   3,   25,  315,  330,    0, } , // 2 pipes (1-2 PKRs) 4 bpe @ SW_VAR_Z_X 8xaa @ RbPlus
    {   3,   26,  278,  331,    0, } , // 2 pipes (1-2 PKRs) 8 bpe @ SW_VAR_Z_X 8xaa @ RbPlus
    {   3,   27,  328,  331,    0, } , // 2 pipes (1-2 PKRs) 16 bpe @ SW_VAR_Z_X 8xaa @ RbPlus
    {   3,   23,  282,  263,    0, } , // 4 pipes (1-2 PKRs) 1 bpe @ SW_VAR_Z_X 8xaa @ RbPlus
    {   3,   24,  282,  299,    0, } , // 4 pipes (1-2 PKRs) 2 bpe @ SW_VAR_Z_X 8xaa @ RbPlus
    {   3,   25,  282,  332,    0, } , // 4 pipes (1-2 PKRs) 4 bpe @ SW_VAR_Z_X 8xaa @ RbPlus
    {   3,   26,  317,  333,    0, } , // 4 pipes (1-2 PKRs) 8 bpe @ SW_VAR_Z_X 8xaa @ RbPlus
    {   3,   27,  329,  334,    0, } , // 4 pipes (1-2 PKRs) 16 bpe @ SW_VAR_Z_X 8xaa @ RbPlus
    {   3,   23,  287,  210,   45, } , // 8 pipes (2 PKRs) 1 bpe @ SW_VAR_Z_X 8xaa @ RbPlus
    {   3,   24,  287,  335,   70, } , // 8 pipes (2 PKRs) 2 bpe @ SW_VAR_Z_X 8xaa @ RbPlus
    {   3,   25,  287,  336,   70, } , // 8 pipes (2 PKRs) 4 bpe @ SW_VAR_Z_X 8xaa @ RbPlus
    {   3,   26,  330,  337,   72, } , // 8 pipes (2 PKRs) 8 bpe @ SW_VAR_Z_X 8xaa @ RbPlus
    {   3,   27,  331,  338,  101, } , // 8 pipes (2 PKRs) 16 bpe @ SW_VAR_Z_X 8xaa @ RbPlus
    {   3,   23,  292,  269,    0, } , // 4 pipes (4 PKRs) 1 bpe @ SW_VAR_Z_X 8xaa @ RbPlus
    {   3,   24,  292,  307,    0, } , // 4 pipes (4 PKRs) 2 bpe @ SW_VAR_Z_X 8xaa @ RbPlus
    {   3,   25,  292,  339,    0, } , // 4 pipes (4 PKRs) 4 bpe @ SW_VAR_Z_X 8xaa @ RbPlus
    {   3,   26,  332,  340,    0, } , // 4 pipes (4 PKRs) 8 bpe @ SW_VAR_Z_X 8xaa @ RbPlus
    {   3,   27,  333,  341,    0, } , // 4 pipes (4 PKRs) 16 bpe @ SW_VAR_Z_X 8xaa @ RbPlus
    {   3,   23,  297,  210,   45, } , // 8 pipes (4 PKRs) 1 bpe @ SW_VAR_Z_X 8xaa @ RbPlus
    {   3,   24,  297,  310,   45, } , // 8 pipes (4 PKRs) 2 bpe @ SW_VAR_Z_X 8xaa @ RbPlus
    {   3,   25,  297,  342,   45, } , // 8 pipes (4 PKRs) 4 bpe @ SW_VAR_Z_X 8xaa @ RbPlus
    {   3,   26,  299,  343,  102, } , // 8 pipes (4 PKRs) 8 bpe @ SW_VAR_Z_X 8xaa @ RbPlus
    {   3,   27,  334,  344,  103, } , // 8 pipes (4 PKRs) 16 bpe @ SW_VAR_Z_X 8xaa @ RbPlus
    {   3,   23,  300,  273,   49, } , // 16 pipes (4 PKRs) 1 bpe @ SW_VAR_Z_X 8xaa @ RbPlus
    {   3,   24,  300,  273,   74, } , // 16 pipes (4 PKRs) 2 bpe @ SW_VAR_Z_X 8xaa @ RbPlus
    {   3,   25,  300,  345,   74, } , // 16 pipes (4 PKRs) 4 bpe @ SW_VAR_Z_X 8xaa @ RbPlus
    {   3,   26,  335,  346,   76, } , // 16 pipes (4 PKRs) 8 bpe @ SW_VAR_Z_X 8xaa @ RbPlus
    {   3,   27,  336,  286,  104, } , // 16 pipes (4 PKRs) 16 bpe @ SW_VAR_Z_X 8xaa @ RbPlus
    {   3,   23,  303,  276,   14, } , // 8 pipes (8 PKRs) 1 bpe @ SW_VAR_Z_X 8xaa @ RbPlus
    {   3,   24,  303,  313,   14, } , // 8 pipes (8 PKRs) 2 bpe @ SW_VAR_Z_X 8xaa @ RbPlus
    {   3,   25,  303,  347,   14, } , // 8 pipes (8 PKRs) 4 bpe @ SW_VAR_Z_X 8xaa @ RbPlus
    {   3,   26,  337,  348,  105, } , // 8 pipes (8 PKRs) 8 bpe @ SW_VAR_Z_X 8xaa @ RbPlus
    {   3,   27,  338,  349,  106, } , // 8 pipes (8 PKRs) 16 bpe @ SW_VAR_Z_X 8xaa @ RbPlus
    {   3,   23,  306,  280,   49, } , // 16 pipes (8 PKRs) 1 bpe @ SW_VAR_Z_X 8xaa @ RbPlus
    {   3,   24,  306,  215,   78, } , // 16 pipes (8 PKRs) 2 bpe @ SW_VAR_Z_X 8xaa @ RbPlus
    {   3,   25,  306,  350,   74, } , // 16 pipes (8 PKRs) 4 bpe @ SW_VAR_Z_X 8xaa @ RbPlus
    {   3,   26,  339,  351,  107, } , // 16 pipes (8 PKRs) 8 bpe @ SW_VAR_Z_X 8xaa @ RbPlus
    {   3,   27,  340,  351,  108, } , // 16 pipes (8 PKRs) 16 bpe @ SW_VAR_Z_X 8xaa @ RbPlus
    {   3,   23,  306,  237,   55, } , // 32 pipes (8 PKRs) 1 bpe @ SW_VAR_Z_X 8xaa @ RbPlus
    {   3,   24,  306,  237,  109, } , // 32 pipes (8 PKRs) 2 bpe @ SW_VAR_Z_X 8xaa @ RbPlus
    {   3,   25,  306,  237,  110, } , // 32 pipes (8 PKRs) 4 bpe @ SW_VAR_Z_X 8xaa @ RbPlus
    {   3,   26,  339,  352,  111, } , // 32 pipes (8 PKRs) 8 bpe @ SW_VAR_Z_X 8xaa @ RbPlus
    {   3,   27,  339,  353,  112, } , // 32 pipes (8 PKRs) 16 bpe @ SW_VAR_Z_X 8xaa @ RbPlus
    {   3,   23,  308,  285,   58, } , // 16 pipes (16 PKRs) 1 bpe @ SW_VAR_Z_X 8xaa @ RbPlus
    {   3,   24,  308,  318,   84, } , // 16 pipes (16 PKRs) 2 bpe @ SW_VAR_Z_X 8xaa @ RbPlus
    {   3,   25,  308,  354,   84, } , // 16 pipes (16 PKRs) 4 bpe @ SW_VAR_Z_X 8xaa @ RbPlus
    {   3,   26,  341,  355,  113, } , // 16 pipes (16 PKRs) 8 bpe @ SW_VAR_Z_X 8xaa @ RbPlus
    {   3,   27,  342,  356,  114, } , // 16 pipes (16 PKRs) 16 bpe @ SW_VAR_Z_X 8xaa @ RbPlus
    {   3,   23,  308,  237,   55, } , // 32 pipes (16 PKRs) 1 bpe @ SW_VAR_Z_X 8xaa @ RbPlus
    {   3,   24,  308,  237,   87, } , // 32 pipes (16 PKRs) 2 bpe @ SW_VAR_Z_X 8xaa @ RbPlus
    {   3,   25,  308,  237,  115, } , // 32 pipes (16 PKRs) 4 bpe @ SW_VAR_Z_X 8xaa @ RbPlus
    {   3,   26,  343,  357,  116, } , // 32 pipes (16 PKRs) 8 bpe @ SW_VAR_Z_X 8xaa @ RbPlus
    {   3,   27,  341,  358,  117, } , // 32 pipes (16 PKRs) 16 bpe @ SW_VAR_Z_X 8xaa @ RbPlus
    {   3,   23,  308,  242,   63, } , // 64 pipes (16 PKRs) 1 bpe @ SW_VAR_Z_X 8xaa @ RbPlus
    {   3,   24,  308,  242,   91, } , // 64 pipes (16 PKRs) 2 bpe @ SW_VAR_Z_X 8xaa @ RbPlus
    {   3,   25,  308,  242,  118, } , // 64 pipes (16 PKRs) 4 bpe @ SW_VAR_Z_X 8xaa @ RbPlus
    {   3,   26,  343,  359,  119, } , // 64 pipes (16 PKRs) 8 bpe @ SW_VAR_Z_X 8xaa @ RbPlus
    {   3,   27,  343,  360,  120, } , // 64 pipes (16 PKRs) 16 bpe @ SW_VAR_Z_X 8xaa @ RbPlus
    {   3,   23,  310,  247,   66, } , // 32 pipes (32 PKRs) 1 bpe @ SW_VAR_Z_X 8xaa @ RbPlus
    {   3,   24,  310,  247,   94, } , // 32 pipes (32 PKRs) 2 bpe @ SW_VAR_Z_X 8xaa @ RbPlus
    {   3,   25,  310,  361,   94, } , // 32 pipes (32 PKRs) 4 bpe @ SW_VAR_Z_X 8xaa @ RbPlus
    {   3,   26,  344,  362,  121, } , // 32 pipes (32 PKRs) 8 bpe @ SW_VAR_Z_X 8xaa @ RbPlus
    {   3,   27,  345,  363,  122, } , // 32 pipes (32 PKRs) 16 bpe @ SW_VAR_Z_X 8xaa @ RbPlus
    {   3,   23,  310,  252,   63, } , // 64 pipes (32 PKRs) 1 bpe @ SW_VAR_Z_X 8xaa @ RbPlus
    {   3,   24,  310,  252,   98, } , // 64 pipes (32 PKRs) 2 bpe @ SW_VAR_Z_X 8xaa @ RbPlus
    {   3,   25,  310,  252,  118, } , // 64 pipes (32 PKRs) 4 bpe @ SW_VAR_Z_X 8xaa @ RbPlus
    {   3,   26,  346,  364,  123, } , // 64 pipes (32 PKRs) 8 bpe @ SW_VAR_Z_X 8xaa @ RbPlus
    {   3,   27,  344,  365,  124, } , // 64 pipes (32 PKRs) 16 bpe @ SW_VAR_Z_X 8xaa @ RbPlus
};

const UINT_64 GFX10_SW_PATTERN_NIBBLE01[][8] =
{
    {X0,            X1,            X2,            X3,            Y0,            Y1,            Y2,            Y3,            }, // 0
    {0,             X0,            X1,            X2,            Y0,            Y1,            Y2,            X3,            }, // 1
    {0,             0,             X0,            X1,            Y0,            Y1,            Y2,            X2,            }, // 2
    {0,             0,             0,             X0,            Y0,            Y1,            X1,            X2,            }, // 3
    {0,             0,             0,             0,             Y0,            Y1,            X0,            X1,            }, // 4
    {X0,            X1,            X2,            Y1,            Y0,            Y2,            X3,            Y3,            }, // 5
    {0,             0,             0,             X0,            Y0,            X1,            X2,            Y1,            }, // 6
    {0,             0,             0,             0,             X0,            Y0,            X1,            Y1,            }, // 7
    {X0,            Y0,            X1,            Y1,            X2,            Y2,            X3,            Y3,            }, // 8
    {0,             X0,            Y0,            X1,            Y1,            X2,            Y2,            X3,            }, // 9
    {0,             0,             X0,            Y0,            X1,            Y1,            X2,            Y2,            }, // 10
    {0,             0,             0,             X0,            Y0,            X1,            Y1,            X2,            }, // 11
    {X0,            Y0,            X1,            Y1,            X2,            Y2,            X3,            Y4,            }, // 12
    {S0,            X0,            Y0,            X1,            Y1,            X2,            Y2,            X3,            }, // 13
    {0,             S0,            X0,            Y0,            X1,            Y1,            X2,            Y2,            }, // 14
    {0,             0,             S0,            X0,            Y0,            X1,            Y1,            X2,            }, // 15
    {0,             0,             0,             S0,            X0,            Y0,            X1,            Y1,            }, // 16
    {0,             0,             0,             0,             S0,            X0,            Y0,            X1,            }, // 17
    {S0,            S1,            X0,            Y0,            X1,            Y1,            X2,            Y2,            }, // 18
    {0,             S0,            S1,            X0,            Y0,            X1,            Y1,            X2,            }, // 19
    {0,             0,             S0,            S1,            X0,            Y0,            X1,            Y1,            }, // 20
    {0,             0,             0,             S0,            S1,            X0,            Y0,            X1,            }, // 21
    {0,             0,             0,             0,             S0,            S1,            X0,            Y0,            }, // 22
    {S0,            S1,            S2,            X0,            Y0,            X1,            Y1,            X2,            }, // 23
    {0,             S0,            S1,            S2,            X0,            Y0,            X1,            Y1,            }, // 24
    {0,             0,             S0,            S1,            S2,            X0,            Y0,            X1,            }, // 25
    {0,             0,             0,             S0,            S1,            S2,            X0,            Y0,            }, // 26
    {0,             0,             0,             0,             S0,            S1,            S2,            X0,            }, // 27
    {X0,            X1,            X2,            Y1,            Y0,            Y2,            X3,            Y4,            }, // 28
    {X0,            X1,            Z0,            Y0,            Z1,            Y1,            X2,            Z2,            }, // 29
    {0,             X0,            Z0,            Y0,            Z1,            Y1,            X1,            Z2,            }, // 30
    {0,             0,             X0,            Y0,            Z0,            Y1,            X1,            Z1,            }, // 31
    {0,             0,             0,             X0,            Z0,            Y0,            X1,            Z1,            }, // 32
    {0,             0,             0,             0,             Z0,            Y0,            X0,            Z1,            }, // 33
    {X0,            X1,            Z0,            Y0,            Y1,            Z1,            X2,            Z2,            }, // 34
    {0,             X0,            Z0,            Y0,            X1,            Z1,            Y1,            Z2,            }, // 35
    {0,             0,             X0,            Y0,            X1,            Z0,            Y1,            Z1,            }, // 36
    {0,             0,             0,             X0,            Y0,            Z0,            X1,            Z1,            }, // 37
    {0,             0,             0,             0,             X0,            Z0,            Y0,            Z1,            }, // 38
    {0,             0,             X0,            X1,            Y0,            Y1,            X2,            Y2,            }, // 39
};

const UINT_64 GFX10_SW_PATTERN_NIBBLE2[][4] =
{
    {0,             0,             0,             0,             }, // 0
    {Y4,            X4,            Y5,            X5,            }, // 1
    {Y3,            X4,            Y4,            X5,            }, // 2
    {Y3,            X3,            Y4,            X4,            }, // 3
    {Y2,            X3,            Y3,            X4,            }, // 4
    {Y2,            X2,            Y3,            X3,            }, // 5
    {Z0^X4^Y4,      X4,            Y5,            X5,            }, // 6
    {Z0^Y3^X4,      X4,            Y4,            X5,            }, // 7
    {Z0^X3^Y3,      X3,            Y4,            X4,            }, // 8
    {Z0^Y2^X3,      X3,            Y3,            X4,            }, // 9
    {Z0^X2^Y2,      X2,            Y3,            X3,            }, // 10
    {Z1^Y4^X5,      Z0^X4^Y5,      Y5,            X5,            }, // 11
    {Z1^Y3^X5,      Z0^X4^Y4,      Y4,            X5,            }, // 12
    {Z1^Y3^X4,      Z0^X3^Y4,      Y4,            X4,            }, // 13
    {Z1^Y2^X4,      Z0^X3^Y3,      Y3,            X4,            }, // 14
    {Z1^Y2^X3,      Z0^X2^Y3,      Y3,            X3,            }, // 15
    {Z2^Y4^X6,      Z1^X4^Y6,      Z0^X5^Y5,      X5,            }, // 16
    {Z2^Y3^X6,      Z1^X4^Y5,      Z0^Y4^X5,      X5,            }, // 17
    {Z2^Y3^X5,      Z1^X3^Y5,      Z0^X4^Y4,      X4,            }, // 18
    {Y2^Z2^X5,      Z1^X3^Y4,      Z0^Y3^X4,      X4,            }, // 19
    {Y2^Z2^X4,      Z1^X2^Y4,      Z0^X3^Y3,      X3,            }, // 20
    {Z3^Y4^X7,      Z2^X4^Y7,      Z1^Y5^X6,      Z0^X5^Y6,      }, // 21
    {Y3^Z3^X7,      Z2^X4^Y6,      Z1^Y4^X6,      Z0^X5^Y5,      }, // 22
    {Y3^Z3^X6,      Z2^X3^Y6,      Z1^Y4^X5,      Z0^X4^Y5,      }, // 23
    {Y2^Z3^X6,      Z2^X3^Y5,      Z1^Y3^X5,      Z0^X4^Y4,      }, // 24
    {Y2^Z3^X5,      X2^Z2^Y5,      Z1^Y3^X4,      Z0^X3^Y4,      }, // 25
    {Y4^Z4^X8,      Z3^X4^Y8,      Z2^Y5^X7,      Z1^X5^Y7,      }, // 26
    {Y3^Z4^X8,      Z3^X4^Y7,      Z2^Y4^X7,      Z1^X5^Y6,      }, // 27
    {Y3^Z4^X7,      X3^Z3^Y7,      Z2^Y4^X6,      Z1^X4^Y6,      }, // 28
    {Y2^Z4^X7,      X3^Z3^Y6,      Z2^Y3^X6,      Z1^X4^Y5,      }, // 29
    {Y2^Z4^X6,      X2^Z3^Y6,      Z2^Y3^X5,      Z1^X3^Y5,      }, // 30
    {Y4^Z5^X9,      X4^Z4^Y9,      Z3^Y5^X8,      Z2^X5^Y8,      }, // 31
    {Y3^Z5^X9,      X4^Z4^Y8,      Z3^Y4^X8,      Z2^X5^Y7,      }, // 32
    {Y3^Z5^X8,      X3^Z4^Y8,      Z3^Y4^X7,      Z2^X4^Y7,      }, // 33
    {Y2^Z5^X8,      X3^Z4^Y7,      Y3^Z3^X7,      Z2^X4^Y6,      }, // 34
    {Y2^Z5^X7,      X2^Z4^Y7,      Y3^Z3^X6,      Z2^X3^Y6,      }, // 35
    {X4^Y4,         X4,            Y5,            X5,            }, // 36
    {Y3^X4,         X4,            Y4,            X5,            }, // 37
    {X3^Y3,         X3,            Y4,            X4,            }, // 38
    {Y2^X3,         X3,            Y3,            X4,            }, // 39
    {X2^Y2,         X2,            Y3,            X3,            }, // 40
    {Y4^X5,         X4^Y5,         Y5,            X5,            }, // 41
    {Y3^X5,         X4^Y4,         Y4,            X5,            }, // 42
    {Y3^X4,         X3^Y4,         Y4,            X4,            }, // 43
    {Y2^X4,         X3^Y3,         Y3,            X4,            }, // 44
    {Y2^X3,         X2^Y3,         Y3,            X3,            }, // 45
    {Y4^X6,         X4^Y6,         X5^Y5,         X5,            }, // 46
    {Y3^X6,         X4^Y5,         Y4^X5,         X5,            }, // 47
    {Y3^X5,         X3^Y5,         X4^Y4,         X4,            }, // 48
    {Y2^X5,         X3^Y4,         Y3^X4,         X4,            }, // 49
    {Y2^X4,         X2^Y4,         X3^Y3,         X3,            }, // 50
    {Y4^X7,         X4^Y7,         Y5^X6,         X5^Y6,         }, // 51
    {Y3^X7,         X4^Y6,         Y4^X6,         X5^Y5,         }, // 52
    {Y3^X6,         X3^Y6,         Y4^X5,         X4^Y5,         }, // 53
    {Y2^X6,         X3^Y5,         Y3^X5,         X4^Y4,         }, // 54
    {Y2^X5,         X2^Y5,         Y3^X4,         X3^Y4,         }, // 55
    {Y4,            X4,            Y5^X7,         X5^Y7,         }, // 56
    {Y3,            X4,            Y4^X7,         X5^Y6,         }, // 57
    {Y3,            X3,            Y4^X6,         X4^Y6,         }, // 58
    {Y2,            X3,            Y3^X6,         X4^Y5,         }, // 59
    {Y2,            X2,            Y3^X5,         X3^Y5,         }, // 60
    {Z0^X3^Y3,      X4,            Y5,            X5,            }, // 61
    {Z0^X3^Y3,      X4,            Y4,            X5,            }, // 62
    {Z0^X3^Y3,      X3,            Y2,            X4,            }, // 63
    {Z0^X3^Y3,      X2,            Y2,            X3,            }, // 64
    {Z1^X3^Y3,      Z0^X4^Y4,      Y5,            X5,            }, // 65
    {Z1^X3^Y3,      Z0^X4^Y4,      Y4,            X5,            }, // 66
    {Z1^X3^Y3,      Z0^X4^Y4,      Y3,            X4,            }, // 67
    {Z1^X3^Y3,      Z0^X4^Y4,      Y2,            X3,            }, // 68
    {Z1^X3^Y3,      Z0^X4^Y4,      Y2,            X2,            }, // 69
    {Z2^X3^Y3,      Z1^X4^Y4,      Z0^X5^Y5,      X5,            }, // 70
    {Z2^X3^Y3,      Z1^X4^Y4,      Z0^X5^Y5,      X4,            }, // 71
    {Z2^X3^Y3,      Z1^X4^Y4,      Z0^X5^Y5,      X3,            }, // 72
    {Z2^X3^Y3,      Z1^X4^Y4,      Z0^X5^Y5,      X2,            }, // 73
    {X3^Y3^Z3,      Z2^X4^Y4,      Z1^Y5^X6,      Z0^X5^Y6,      }, // 74
    {X3^Y3^Z4,      Z3^X4^Y4,      Z2^Y5^X7,      Z1^X5^Y7,      }, // 75
    {X3^Y3^Z3,      Z2^X4^Y4,      Z1^Y5^X7,      Z0^X5^Y7,      }, // 76
    {X3^Y3^Z5,      X4^Y4^Z4,      Z3^Y5^X8,      Z2^X5^Y8,      }, // 77
    {X3^Y3^Z4,      Z3^X4^Y4,      Z2^Y5^X8,      Z1^X5^Y8,      }, // 78
    {X3^Y3^Z3,      Z2^X4^Y4,      Z1^Y5^X8,      Z0^X5^Y8,      }, // 79
    {Y3,            Y4,            X4,            Y5,            }, // 80
    {X2,            Y3,            X3,            Y4,            }, // 81
    {Z0^X3^Y3,      Y4,            X4,            Y5,            }, // 82
    {Z0^X3^Y3,      X2,            X3,            Y4,            }, // 83
    {Z1^X3^Y3,      Z0^X4^Y4,      Y4,            Y5,            }, // 84
    {Z1^X3^Y3,      Z0^X4^Y4,      X2,            Y3,            }, // 85
    {Z2^X3^Y3,      Z1^X4^Y4,      Z0^X5^Y5,      Y4,            }, // 86
    {Z2^X3^Y3,      Z1^X4^Y4,      Z0^Y5^X6,      Y2^X5^Y6,      }, // 87
    {Z2^X3^Y3,      Z1^X4^Y4,      Z0^Y5^X7,      Y2^X5^Y7,      }, // 88
    {Z2^X3^Y3,      Z1^X4^Y4,      Z0^Y5^X8,      Y2^X5^Y8,      }, // 89
    {X3,            Y3,            X4,            Y4,            }, // 90
    {Z0^X3^Y3,      X3,            X4,            Y4,            }, // 91
    {Z1^X3^Y3,      Z0^X4^Y4,      X3,            Y4,            }, // 92
    {Z2^X3^Y3,      Z1^X4^Y4,      Z0^X5^Y5,      Y2,            }, // 93
    {Z1^X3^Y3,      Z0^X4^Y4,      Y2^X5^Y5,      X2,            }, // 94
    {Z2^X3^Y3,      Z1^X4^Y4,      Y2^Y5^X6,      Z0^X5^Y6,      }, // 95
    {Z1^X3^Y3,      Z0^X4^Y4,      Y2^Y5^X6,      X1^X5^Y6,      }, // 96
    {Z2^X3^Y3,      Z1^X4^Y4,      Y2^Y5^X7,      Z0^X5^Y7,      }, // 97
    {Z1^X3^Y3,      Z0^X4^Y4,      Y2^Y5^X7,      X1^X5^Y7,      }, // 98
    {Z2^X3^Y3,      Z1^X4^Y4,      Y2^Y5^X8,      Z0^X5^Y8,      }, // 99
    {Z1^X3^Y3,      Z0^X4^Y4,      Y2^Y5^X8,      X1^X5^Y8,      }, // 100
    {Z0^X3^Y3,      Y2,            X3,            Y4,            }, // 101
    {Z1^X3^Y3,      Z0^X4^Y4,      X2,            Y2,            }, // 102
    {Z1^X3^Y3,      Z0^X4^Y4,      Y2^X5^Y5,      Y3,            }, // 103
    {Z1^X3^Y3,      Z0^X4^Y4,      Y0^X5^Y5,      Y2,            }, // 104
    {Z2^X3^Y3,      Z1^X4^Y4,      Z0^Y5^X6,      Z3^X5^Y6,      }, // 105
    {Z1^X3^Y3,      Z0^X4^Y4,      Y0^Y5^X6,      X1^X5^Y6,      }, // 106
    {Z2^X3^Y3,      Z1^X4^Y4,      Z0^Y5^X7,      Z4^X5^Y7,      }, // 107
    {Z2^X3^Y3,      Z1^X4^Y4,      Z0^Y5^X7,      Z3^X5^Y7,      }, // 108
    {Z1^X3^Y3,      Z0^X4^Y4,      Y0^Y5^X7,      X1^X5^Y7,      }, // 109
    {Z2^X3^Y3,      Z1^X4^Y4,      Z0^Y5^X8,      Z4^X5^Y8,      }, // 110
    {Z2^X3^Y3,      Z1^X4^Y4,      Z0^Y5^X8,      Z3^X5^Y8,      }, // 111
    {Z1^X3^Y3,      Z0^X4^Y4,      Y0^Y5^X8,      X1^X5^Y8,      }, // 112
    {Z2^X3^Y3,      Z1^X4^Y4,      Z0^Y5^X6,      S0^X5^Y6,      }, // 113
    {Z2^X3^Y3,      Z1^X4^Y4,      Z0^Y5^X7,      S0^X5^Y7,      }, // 114
    {Z2^X3^Y3,      Z1^X4^Y4,      Z0^Y5^X8,      S0^X5^Y8,      }, // 115
    {Z1^X3^Y3,      Z0^X4^Y4,      S1^X5^Y5,      X2,            }, // 116
    {Z2^X3^Y3,      Z1^X4^Y4,      S1^Y5^X6,      Z0^X5^Y6,      }, // 117
    {Z1^X3^Y3,      Z0^X4^Y4,      S1^Y5^X6,      S0^X5^Y6,      }, // 118
    {Z2^X3^Y3,      Z1^X4^Y4,      S1^Y5^X7,      Z0^X5^Y7,      }, // 119
    {Z1^X3^Y3,      Z0^X4^Y4,      S1^Y5^X7,      S0^X5^Y7,      }, // 120
    {Z2^X3^Y3,      Z1^X4^Y4,      S1^Y5^X8,      Z0^X5^Y8,      }, // 121
    {Z1^X3^Y3,      Z0^X4^Y4,      S1^Y5^X8,      S0^X5^Y8,      }, // 122
    {Z1^X3^Y3,      Z0^X4^Y4,      S2^X5^Y5,      Y2,            }, // 123
    {Z1^X3^Y3,      Z0^X4^Y4,      S2^X5^Y5,      X2,            }, // 124
    {Z2^X3^Y3,      Z1^X4^Y4,      Z0^Y5^X6,      S2^X5^Y6,      }, // 125
    {Z1^X3^Y3,      Z0^X4^Y4,      S2^Y5^X6,      S1^X5^Y6,      }, // 126
    {Z2^X3^Y3,      Z1^X4^Y4,      Z0^Y5^X7,      S2^X5^Y7,      }, // 127
    {Z1^X3^Y3,      Z0^X4^Y4,      S2^Y5^X7,      S1^X5^Y7,      }, // 128
    {Z2^X3^Y3,      Z1^X4^Y4,      Z0^Y5^X8,      S2^X5^Y8,      }, // 129
    {Z1^X3^Y3,      Z0^X4^Y4,      S2^Y5^X8,      S1^X5^Y8,      }, // 130
    {Y2,            X3,            Z3,            Y3,            }, // 131
    {Y2,            X2,            Z3,            Y3,            }, // 132
    {Y2,            X2,            Z2,            Y3,            }, // 133
    {Y1,            X2,            Z2,            Y2,            }, // 134
    {Y1,            X1,            Z2,            Y2,            }, // 135
    {Y2^X3^Z3,      X3,            Z3,            Y3,            }, // 136
    {X2^Y2^Z3,      X2,            Z3,            Y3,            }, // 137
    {X2^Y2^Z2,      X2,            Z2,            Y3,            }, // 138
    {Y1^X2^Z2,      X2,            Z2,            Y2,            }, // 139
    {X1^Y1^Z2,      X1,            Z2,            Y2,            }, // 140
    {Y2^X4^Z4,      X3^Y3^Z3,      Z3,            Y3,            }, // 141
    {Y2^X3^Z4,      X2^Y3^Z3,      Z3,            Y3,            }, // 142
    {Y2^X3^Z3,      X2^Z2^Y3,      Z2,            Y3,            }, // 143
    {Y1^X3^Z3,      X2^Y2^Z2,      Z2,            Y2,            }, // 144
    {Y1^X2^Z3,      X1^Y2^Z2,      Z2,            Y2,            }, // 145
    {Y2^X5^Z5,      X3^Y4^Z4,      Y3^Z3^X4,      Y3,            }, // 146
    {Y2^X4^Z5,      X2^Y4^Z4,      X3^Y3^Z3,      Y3,            }, // 147
    {Y2^X4^Z4,      X2^Z3^Y4,      Z2^X3^Y3,      Y3,            }, // 148
    {Y1^X4^Z4,      X2^Y3^Z3,      Y2^Z2^X3,      Y2,            }, // 149
    {Y1^X3^Z4,      X1^Y3^Z3,      X2^Y2^Z2,      Y2,            }, // 150
    {Y2^X6^Z6,      X3^Y5^Z5,      Z3^Y4^X5,      Y3^X4^Z4,      }, // 151
    {Y2^X5^Z6,      X2^Y5^Z5,      Z3^X4^Y4,      X3^Y3^Z4,      }, // 152
    {Y2^X5^Z5,      X2^Z4^Y5,      Z2^X4^Y4,      X3^Y3^Z3,      }, // 153
    {Y1^X5^Z5,      X2^Y4^Z4,      Z2^Y3^X4,      Y2^X3^Z3,      }, // 154
    {Y1^X4^Z5,      X1^Y4^Z4,      Z2^X3^Y3,      X2^Y2^Z3,      }, // 155
    {Y2^X7^Z7,      X3^Y6^Z6,      Z3^Y5^X6,      Y3^X5^Z5,      }, // 156
    {Y2^X6^Z7,      X2^Y6^Z6,      Z3^X5^Y5,      Y3^X4^Z5,      }, // 157
    {Y2^X6^Z6,      X2^Z5^Y6,      Z2^X5^Y5,      Y3^X4^Z4,      }, // 158
    {Y1^X6^Z6,      X2^Y5^Z5,      Z2^Y4^X5,      Y2^X4^Z4,      }, // 159
    {Y1^X5^Z6,      X1^Y5^Z5,      Z2^X4^Y4,      Y2^X3^Z4,      }, // 160
    {Y2^X8^Z8,      X3^Y7^Z7,      Z3^Y6^X7,      Y3^X6^Z6,      }, // 161
    {Y2^X7^Z8,      X2^Y7^Z7,      Z3^X6^Y6,      Y3^X5^Z6,      }, // 162
    {Y2^X7^Z7,      X2^Z6^Y7,      Z2^X6^Y6,      Y3^X5^Z5,      }, // 163
    {Y1^X7^Z7,      X2^Y6^Z6,      Z2^Y5^X6,      Y2^X5^Z5,      }, // 164
    {Y1^X6^Z7,      X1^Y6^Z6,      Z2^X5^Y5,      Y2^X4^Z5,      }, // 165
    {Y2^X5,         X3^Y4^Z4,      Y3^Z3^X4,      Y3,            }, // 166
    {Y2^X4,         X2^Y4^Z4,      X3^Y3^Z3,      Y3,            }, // 167
    {Y2^X4,         X2^Z3^Y4,      Z2^X3^Y3,      Y3,            }, // 168
    {Y1^X4,         X2^Y3^Z3,      Y2^Z2^X3,      Y2,            }, // 169
    {Y1^X3,         X1^Y3^Z3,      X2^Y2^Z2,      Y2,            }, // 170
    {Y2,            X3,            Z3^Y4^X5,      Y3^X4^Z4,      }, // 171
    {Y2,            X2,            Z3^X4^Y4,      X3^Y3^Z4,      }, // 172
    {Y2,            X2,            Z2^X4^Y4,      X3^Y3^Z3,      }, // 173
    {Y1,            X2,            Z2^Y3^X4,      Y2^X3^Z3,      }, // 174
    {Y1,            X1,            Z2^X3^Y3,      X2^Y2^Z3,      }, // 175
    {Y2,            X3,            Z3,            Y3^X5,         }, // 176
    {Y2,            X2,            Z3,            Y3^X4,         }, // 177
    {Y2,            X2,            Z2,            Y3^X4,         }, // 178
    {Y1,            X2,            Z2,            Y2^X4,         }, // 179
    {Y1,            X1,            Z2,            Y2^X3,         }, // 180
    {X3^Y3,         X3,            Z3,            Y2,            }, // 181
    {X3^Y3,         X2,            Z3,            Y2,            }, // 182
    {X3^Y3,         X2,            Z2,            Y2,            }, // 183
    {X3^Y3,         X2,            Z2,            Y1,            }, // 184
    {X3^Y3,         X1,            Z2,            Y1,            }, // 185
    {X3^Y3,         X4^Y4,         Z3,            Y2,            }, // 186
    {X3^Y3,         X4^Y4,         Z2,            Y2,            }, // 187
    {X3^Y3,         X4^Y4,         Z2,            Y1,            }, // 188
    {X3^Y3,         X1^X4^Y4,      Z2,            Y1,            }, // 189
    {X3^Y3,         X4^Y4,         X5^Y5,         Z3,            }, // 190
    {X3^Y3,         X4^Y4,         Z3^X5^Y5,      Y2,            }, // 191
    {X3^Y3,         X4^Y4,         Z2^X5^Y5,      Y2,            }, // 192
    {X3^Y3,         X4^Y4,         Z2^X5^Y5,      Y1,            }, // 193
    {X3^Y3,         X1^X4^Y4,      Z2^X5^Y5,      Y1,            }, // 194
    {X3^Y3,         X4^Y4,         Y2^Y5^X6,      X5^Y6,         }, // 195
    {X3^Y3,         X4^Y4,         Z3^Y5^X6,      Y2^X5^Y6,      }, // 196
    {X3^Y3,         X4^Y4,         Z2^Y5^X6,      Y2^X5^Y6,      }, // 197
    {X3^Y3,         X4^Y4,         Z2^Y5^X6,      Y1^X5^Y6,      }, // 198
    {X3^Y3,         X1^X4^Y4,      Z2^Y5^X6,      Y1^X5^Y6,      }, // 199
    {X3^Y3,         X4^Y4,         Y2^Y5^X7,      X5^Y7,         }, // 200
    {X3^Y3,         X4^Y4,         Z3^Y5^X7,      Y2^X5^Y7,      }, // 201
    {X3^Y3,         X4^Y4,         Z2^Y5^X7,      Y2^X5^Y7,      }, // 202
    {X3^Y3,         X4^Y4,         Z2^Y5^X7,      Y1^X5^Y7,      }, // 203
    {X3^Y3,         X1^X4^Y4,      Z2^Y5^X7,      Y1^X5^Y7,      }, // 204
    {X3^Y3,         X4^Y4,         Y2^Y5^X8,      X5^Y8,         }, // 205
    {X3^Y3,         X4^Y4,         Z3^Y5^X8,      Y2^X5^Y8,      }, // 206
    {X3^Y3,         X4^Y4,         Z2^Y5^X8,      Y2^X5^Y8,      }, // 207
    {X3^Y3,         X4^Y4,         Z2^Y5^X8,      Y1^X5^Y8,      }, // 208
    {X3^Y3,         X1^X4^Y4,      Z2^Y5^X8,      Y1^X5^Y8,      }, // 209
    {Y4^X5,         Z0^X4^Y5,      Y5,            X5,            }, // 210
    {Y3^X5,         Z0^X4^Y4,      Y4,            X5,            }, // 211
    {Y3^X4,         Z0^X3^Y4,      Y4,            X4,            }, // 212
    {Y2^X4,         Z0^X3^Y3,      Y3,            X4,            }, // 213
    {Y2^X3,         Z0^X2^Y3,      Y3,            X3,            }, // 214
    {Y4^X6,         X4^Y6,         Z0^X5^Y5,      X5,            }, // 215
    {Y3^X6,         X4^Y5,         Z0^Y4^X5,      X5,            }, // 216
    {Y3^X5,         X3^Y5,         Z0^X4^Y4,      X4,            }, // 217
    {Y2^X5,         X3^Y4,         Z0^Y3^X4,      X4,            }, // 218
    {Y2^X4,         X2^Y4,         Z0^X3^Y3,      X3,            }, // 219
    {Y4^X6,         Z1^X4^Y6,      Z0^X5^Y5,      X5,            }, // 220
    {Y3^X6,         Z1^X4^Y5,      Z0^Y4^X5,      X5,            }, // 221
    {Y3^X5,         Z1^X3^Y5,      Z0^X4^Y4,      X4,            }, // 222
    {Y2^X5,         Z1^X3^Y4,      Z0^Y3^X4,      X4,            }, // 223
    {Y2^X4,         Z1^X2^Y4,      Z0^X3^Y3,      X3,            }, // 224
    {Y4^X7,         X4^Y7,         Z1^Y5^X6,      Z0^X5^Y6,      }, // 225
    {Y3^X7,         X4^Y6,         Z1^Y4^X6,      Z0^X5^Y5,      }, // 226
    {Y3^X6,         X3^Y6,         Z1^Y4^X5,      Z0^X4^Y5,      }, // 227
    {Y2^X6,         X3^Y5,         Z1^Y3^X5,      Z0^X4^Y4,      }, // 228
    {Y2^X5,         X2^Y5,         Z1^Y3^X4,      Z0^X3^Y4,      }, // 229
    {Y4^X7,         Z2^X4^Y7,      Z1^Y5^X6,      Z0^X5^Y6,      }, // 230
    {Y3^X7,         Z2^X4^Y6,      Z1^Y4^X6,      Z0^X5^Y5,      }, // 231
    {Y3^X6,         Z2^X3^Y6,      Z1^Y4^X5,      Z0^X4^Y5,      }, // 232
    {Y2^X6,         Z2^X3^Y5,      Z1^Y3^X5,      Z0^X4^Y4,      }, // 233
    {Y2^X5,         X2^Z2^Y5,      Z1^Y3^X4,      Z0^X3^Y4,      }, // 234
    {Y4^X7,         X4^Y7,         Z2^Y5^X6,      Z1^X5^Y6,      }, // 235
    {Y3^X7,         X4^Y6,         Z2^Y4^X6,      Z1^X5^Y5,      }, // 236
    {Y3^X6,         X3^Y6,         Z2^Y4^X5,      Z1^X4^Y5,      }, // 237
    {Y2^X6,         X3^Y5,         Z2^Y3^X5,      Z1^X4^Y4,      }, // 238
    {Y2^X5,         X2^Y5,         Z2^Y3^X4,      Z1^X3^Y4,      }, // 239
    {Y4^X7,         Z3^X4^Y7,      Z2^Y5^X6,      Z1^X5^Y6,      }, // 240
    {Y3^X7,         Z3^X4^Y6,      Z2^Y4^X6,      Z1^X5^Y5,      }, // 241
    {Y3^X6,         X3^Z3^Y6,      Z2^Y4^X5,      Z1^X4^Y5,      }, // 242
    {Y2^X6,         X3^Z3^Y5,      Z2^Y3^X5,      Z1^X4^Y4,      }, // 243
    {Y2^X5,         X2^Z3^Y5,      Z2^Y3^X4,      Z1^X3^Y4,      }, // 244
    {Y4^X7,         X4^Y7,         Z3^Y5^X6,      Z2^X5^Y6,      }, // 245
    {Y3^X7,         X4^Y6,         Z3^Y4^X6,      Z2^X5^Y5,      }, // 246
    {Y3^X6,         X3^Y6,         Z3^Y4^X5,      Z2^X4^Y5,      }, // 247
    {Y2^X6,         X3^Y5,         Y3^Z3^X5,      Z2^X4^Y4,      }, // 248
    {Y2^X5,         X2^Y5,         Y3^Z3^X4,      Z2^X3^Y4,      }, // 249
    {Y4^X8,         X4^Y8,         Z2^Y5^X7,      Z1^X5^Y7,      }, // 250
    {Y3^X8,         X4^Y7,         Z2^Y4^X7,      Z1^X5^Y6,      }, // 251
    {Y3^X7,         X3^Y7,         Z2^Y4^X6,      Z1^X4^Y6,      }, // 252
    {Y2^X7,         X3^Y6,         Z2^Y3^X6,      Z1^X4^Y5,      }, // 253
    {Y2^X6,         X2^Y6,         Z2^Y3^X5,      Z1^X3^Y5,      }, // 254
    {Y4^X8,         Z3^X4^Y8,      Z2^Y5^X7,      Z1^X5^Y7,      }, // 255
    {Y3^X8,         Z3^X4^Y7,      Z2^Y4^X7,      Z1^X5^Y6,      }, // 256
    {Y3^X7,         X3^Z3^Y7,      Z2^Y4^X6,      Z1^X4^Y6,      }, // 257
    {Y2^X7,         X3^Z3^Y6,      Z2^Y3^X6,      Z1^X4^Y5,      }, // 258
    {Y2^X6,         X2^Z3^Y6,      Z2^Y3^X5,      Z1^X3^Y5,      }, // 259
    {Y4^X9,         X4^Y9,         Z3^Y5^X8,      Z2^X5^Y8,      }, // 260
    {Y3^X9,         X4^Y8,         Z3^Y4^X8,      Z2^X5^Y7,      }, // 261
    {Y3^X8,         X3^Y8,         Z3^Y4^X7,      Z2^X4^Y7,      }, // 262
    {Y2^X8,         X3^Y7,         Y3^Z3^X7,      Z2^X4^Y6,      }, // 263
    {Y2^X7,         X2^Y7,         Y3^Z3^X6,      Z2^X3^Y6,      }, // 264
    {Y4^X9,         X4^Z4^Y9,      Z3^Y5^X8,      Z2^X5^Y8,      }, // 265
    {Y3^X9,         X4^Z4^Y8,      Z3^Y4^X8,      Z2^X5^Y7,      }, // 266
    {Y3^X8,         X3^Z4^Y8,      Z3^Y4^X7,      Z2^X4^Y7,      }, // 267
    {Y2^X8,         X3^Z4^Y7,      Y3^Z3^X7,      Z2^X4^Y6,      }, // 268
    {Y2^X7,         X2^Z4^Y7,      Y3^Z3^X6,      Z2^X3^Y6,      }, // 269
    {X4,            Y4,            X5^Y8,         Y5^X8,         }, // 270
    {Y3,            X4,            Y4^X8,         X5^Y7,         }, // 271
    {X3,            Y3,            X4^Y7,         Y4^X7,         }, // 272
    {Y2,            X3,            Y3^X7,         X4^Y6,         }, // 273
    {X2,            Y2,            X3^Y6,         Y3^X6,         }, // 274
    {Z0^X4^Y4,      Y4,            X5,            X6^Y8,         }, // 275
    {Z0^X4^Y4,      Y3,            Y4,            X5^Y8,         }, // 276
    {Z0^X4^Y4,      X3,            Y3,            X5^Y7,         }, // 277
    {Z0^X4^Y4,      Y2,            X3,            Y3^X8,         }, // 278
    {Z0^X4^Y4,      X2,            Y2,            X3^Y6,         }, // 279
    {Y4^X5^Y5,      Z0^X4^Y4,      X5,            Y5,            }, // 280
    {Y4^X5^Y5,      Z0^X4^Y4,      Y3,            X5,            }, // 281
    {Y4^X5^Y5,      Z0^X4^Y4,      X3,            Y3,            }, // 282
    {Y4^X5^Y5,      Z0^X4^Y4,      Y2,            X3,            }, // 283
    {Y4^X5^Y5,      Z0^X4^Y4,      X2,            Y2,            }, // 284
    {Y4^X5^Y5,      Z0^X4^Y4,      X5^Y5,         Y5,            }, // 285
    {Y4^X5^Y5,      Z0^X4^Y4,      X5^Y5,         Y3,            }, // 286
    {Y4^X5^Y5,      Z0^X4^Y4,      X5^Y5,         X3,            }, // 287
    {Y4^X5^Y5,      Z0^X4^Y4,      X5^Y5,         Y2,            }, // 288
    {Y4^X5^Y5,      Z0^X4^Y4,      X5^Y5,         X2,            }, // 289
    {Y4^X6^Y6,      Z1^X4^Y4,      X5,            X6,            }, // 290
    {Y4^X6^Y6,      Z1^X4^Y4,      Y3,            X5,            }, // 291
    {Y4^X6^Y6,      Z1^X4^Y4,      X3,            Y3,            }, // 292
    {Y4^X6^Y6,      Z1^X4^Y4,      Y2,            X3,            }, // 293
    {Y4^X6^Y6,      Z1^X4^Y4,      X2,            Y2,            }, // 294
    {Y4^X6^Y6,      Z1^X4^Y4,      Z0^X5^Y5,      X5,            }, // 295
    {Y4^X6^Y6,      Z1^X4^Y4,      Z0^X5^Y5,      Y3,            }, // 296
    {Y4^X6^Y6,      Z1^X4^Y4,      Z0^X5^Y5,      X3,            }, // 297
    {Y4^X6^Y6,      Z1^X4^Y4,      Z0^X5^Y5,      Y2,            }, // 298
    {Y4^X6^Y6,      Z1^X4^Y4,      Z0^X5^Y5,      X2,            }, // 299
    {Y4^X6^Y6,      Z1^X4^Y4,      Z0^X5^Y5,      X5^Y6,         }, // 300
    {Y4^X7^Y7,      Z1^X4^Y4,      Z0^Y5^X6,      X6,            }, // 301
    {Y4^X7^Y7,      Z1^X4^Y4,      Z0^Y5^X6,      Y3,            }, // 302
    {Y4^X7^Y7,      Z1^X4^Y4,      Z0^Y5^X6,      X3,            }, // 303
    {Y4^X7^Y7,      Z2^X4^Y4,      Z1^Y5^X6,      Y2,            }, // 304
    {Y4^X7^Y7,      Z2^X4^Y4,      Z1^Y5^X6,      X2,            }, // 305
    {Y4^X7^Y7,      Z1^X4^Y4,      Z0^Y5^X6,      X5^Y6,         }, // 306
    {Y4^X7^Y7,      Z2^X4^Y4,      Z1^Y5^X6,      Z0^X5^Y6,      }, // 307
    {Y4^X8^Y8,      Z1^X4^Y4,      Z0^Y5^X7,      X5^Y7,         }, // 308
    {Y4^X8^Y8,      Z3^X4^Y4,      Z2^Y5^X7,      Z1^X5^Y7,      }, // 309
    {Y4^X9^Y9,      Z1^X4^Y4,      Z0^Y5^X8,      X5^Y8,         }, // 310
    {Y4^X9^Y9,      X4^Y4^Z4,      Z3^Y5^X8,      Z2^X5^Y8,      }, // 311
    {Y3,            X4,            Y4^X8,         Y5^X7,         }, // 312
    {X3,            Y3,            Y4^X7,         X4^Y7,         }, // 313
    {X2,            Y2,            Y3^X6,         X3^Y6,         }, // 314
    {Z0^X4^Y4,      X3,            Y3,            Y4^X8,         }, // 315
    {Z0^X4^Y4,      X2,            Y2,            Y3^X7,         }, // 316
    {Y4^X5^Y5,      Z0^X4^Y4,      X2,            X3,            }, // 317
    {Y4^X9^Y9,      Z3^X4^Y4,      Z2^Y5^X8,      Z1^X5^Y8,      }, // 318
    {Z0^X4^Y4,      X2,            X3,            Y3^X8,         }, // 319
    {Y4^X6^Y6,      Z1^X4^Y4,      X2,            X3,            }, // 320
    {Y4^X6^Y6,      Z0^X4^Y4,      X2,            X3,            }, // 321
    {Y4^X7^Y7,      Z1^X4^Y4,      Y1^Y5^X6,      X2,            }, // 322
    {Y4^X8^Y8,      Z2^X4^Y4,      Z1^Y5^X7,      Z0^X5^Y7,      }, // 323
    {Y4^X9^Y9,      Z2^X4^Y4,      Z1^Y5^X8,      Z0^X5^Y8,      }, // 324
    {X3,            Y3,            Y4^X7,         Y1^X4^Y7,      }, // 325
    {Y2,            X3,            Y3^X7,         X1^X4^Y6,      }, // 326
    {X2,            Y2,            Y3^X6,         Y0^X3^Y6,      }, // 327
    {Y0^X4^Y4,      Y2,            X3,            Y3^X8,         }, // 328
    {Y4^X5^Y5,      Y0^X4^Y4,      X2,            X3,            }, // 329
    {Y4^X5^Y5,      Z0^X4^Y4,      X2^X5^Y5,      Y2,            }, // 330
    {Y4^X5^Y5,      Z0^X4^Y4,      Y1^X5^Y5,      X2,            }, // 331
    {Y4^X6^Y6,      Z0^X4^Y4,      X3,            Y3,            }, // 332
    {Y4^X6^Y6,      Y0^X4^Y4,      X3,            Y3,            }, // 333
    {Y4^X6^Y6,      Z0^X4^Y4,      Y0^X5^Y5,      X2,            }, // 334
    {Y4^X6^Y6,      Z1^X4^Y4,      Z0^X5^Y5,      X2^X5^Y5,      }, // 335
    {Y4^X6^Y6,      Z1^X4^Y4,      Z0^X5^Y5,      Y1^X5^Y5,      }, // 336
    {Y4^X7^Y7,      Z0^X4^Y4,      Y1^Y5^X6,      X3,            }, // 337
    {Y4^X7^Y7,      Z0^X4^Y4,      Y0^Y5^X6,      X3,            }, // 338
    {Y4^X7^Y7,      Z1^X4^Y4,      Z0^Y5^X6,      Z2^X5^Y6,      }, // 339
    {Y4^X7^Y7,      Z1^X4^Y4,      Z0^Y5^X6,      Y0^X5^Y6,      }, // 340
    {Y4^X8^Y8,      Z1^X4^Y4,      Z0^Y5^X7,      Z2^X5^Y7,      }, // 341
    {Y4^X8^Y8,      Z1^X4^Y4,      Z0^Y5^X7,      Y0^X5^Y7,      }, // 342
    {Y4^X8^Y8,      Z1^X4^Y4,      Z0^Y5^X7,      Z3^X5^Y7,      }, // 343
    {Y4^X9^Y9,      Z1^X4^Y4,      Z0^Y5^X8,      Z3^X5^Y8,      }, // 344
    {Y4^X9^Y9,      Z1^X4^Y4,      Z0^Y5^X8,      Z2^X5^Y8,      }, // 345
    {Y4^X9^Y9,      Z1^X4^Y4,      Z0^Y5^X8,      Z4^X5^Y8,      }, // 346
    {X4,            Y4,            X5^Y10,        Y5^X10,        }, // 347
    {Y3,            X4,            Y4^X10,        X5^Y9,         }, // 348
    {X3,            Y3,            X4^Y9,         Y4^X9,         }, // 349
    {Y2,            X3,            Y3^X9,         X4^Y8,         }, // 350
    {X2,            Y2,            X3^Y8,         Y3^X8,         }, // 351
    {Z0^X4^Y4,      Y4,            X5,            Y5^X10,        }, // 352
    {Z0^X4^Y4,      Y3,            Y4,            X5^Y9,         }, // 353
    {Z0^X4^Y4,      X3,            Y3,            Y4^X9,         }, // 354
    {Z0^X4^Y4,      Y2,            X3,            Y3^X9,         }, // 355
    {Z0^X4^Y4,      X2,            Y2,            Y3^X8,         }, // 356
    {Y3,            X4,            Y4^X10,        Y5^X9,         }, // 357
    {X3,            Y3,            Y4^X9,         X4^Y9,         }, // 358
    {X2,            Y2,            Y3^X8,         X3^Y8,         }, // 359
    {Z0^X4^Y4,      Y3,            Y4,            Y5^X9,         }, // 360
    {Z0^X4^Y4,      X2,            X3,            Y3^X9,         }, // 361
    {Y4^X6^Y6,      Z1^X4^Y4,      Z0^X5^Y5,      X2^X5^Y6,      }, // 362
    {Y4^X6^Y6,      Z1^X4^Y4,      Z0^X5^Y5,      Y1^X5^Y6,      }, // 363
    {Y4^X7^Y7,      Z1^X4^Y4,      Z0^Y5^X6,      X2,            }, // 364
    {Y4^X7^Y7,      Z1^X4^Y4,      Z0^Y5^X6,      Y1^X5^Y6,      }, // 365
    {Y4^X8^Y8,      Z1^X4^Y4,      Z0^Y5^X7,      Y1^X5^Y7,      }, // 366
    {Y4^X9^Y9,      Z1^X4^Y4,      Z0^Y5^X8,      Y1^X5^Y8,      }, // 367
    {Z0^X4^Y4,      X3,            Y3,            X5^Y8,         }, // 368
    {Y4^X6^Y6,      Z0^X4^Y4,      Y1^X5^Y5,      X2,            }, // 369
    {Y4^X6^Y6,      Z0^X4^Y4,      Y1^X5^Y5,      X1^X5^Y6,      }, // 370
    {Y4^X7^Y7,      Z1^X4^Y4,      Y1^Y5^X6,      X3,            }, // 371
    {Y4^X7^Y7,      Z1^X4^Y4,      Y1^Y5^X6,      Z0^X5^Y6,      }, // 372
    {Y4^X7^Y7,      Z0^X4^Y4,      Y1^Y5^X6,      X1^X5^Y6,      }, // 373
    {Y4^X8^Y8,      Z1^X4^Y4,      Y1^Y5^X7,      Z0^X5^Y7,      }, // 374
    {Y4^X8^Y8,      Z0^X4^Y4,      Y1^Y5^X7,      X1^X5^Y7,      }, // 375
    {Y4^X9^Y9,      Z1^X4^Y4,      Y1^Y5^X8,      Z0^X5^Y8,      }, // 376
    {Y4^X9^Y9,      Z0^X4^Y4,      Y1^Y5^X8,      X1^X5^Y8,      }, // 377
    {Z0^X4^Y4,      X2,            Y2,            X3^Y7,         }, // 378
    {Y4^X5^Y5,      Z0^X4^Y4,      Y2^X5^Y5,      X2,            }, // 379
    {Y4^X5^Y5,      Y0^X4^Y4,      X1^X5^Y5,      X2,            }, // 380
    {Y4^X6^Y6,      Z0^X4^Y4,      Y1^X5^Y5,      X3,            }, // 381
    {Y4^X6^Y6,      Y0^X4^Y4,      Y1^X5^Y5,      X3,            }, // 382
    {Y4^X6^Y6,      Z1^X4^Y4,      Z0^X5^Y5,      Y2^X5^Y6,      }, // 383
    {Y4^X6^Y6,      Z0^X4^Y4,      Y1^X5^Y5,      X2^X5^Y6,      }, // 384
    {Y4^X6^Y6,      Y0^X4^Y4,      Y1^X5^Y5,      Y2^X5^Y6,      }, // 385
    {Y4^X7^Y7,      Y0^X4^Y4,      Y1^Y5^X6,      X3,            }, // 386
    {Y4^X7^Y7,      Z1^X4^Y4,      Z0^Y5^X6,      Y2^X5^Y6,      }, // 387
    {Y4^X7^Y7,      Y0^X4^Y4,      Y1^Y5^X6,      X1^X5^Y6,      }, // 388
    {Y4^X8^Y8,      Z1^X4^Y4,      Z0^Y5^X7,      Y2^X5^Y7,      }, // 389
    {Y4^X8^Y8,      Y0^X4^Y4,      Y1^Y5^X7,      X1^X5^Y7,      }, // 390
    {Y4^X8^Y8,      Z1^X4^Y4,      Z0^Y5^X7,      X2^X5^Y7,      }, // 391
    {Y4^X9^Y9,      Z1^X4^Y4,      Z0^Y5^X8,      X2^X5^Y8,      }, // 392
    {Y4^X9^Y9,      Y0^X4^Y4,      Y1^Y5^X8,      X1^X5^Y8,      }, // 393
    {Y4^X5^Y5,      Z0^X4^Y4,      X5^X6^Y6,      Y5,            }, // 394
    {Y4^X5^Y5,      Z0^X4^Y4,      X5^X6^Y6,      Y3,            }, // 395
    {Y4^X5^Y5,      Z0^X4^Y4,      X5^X6^Y6,      X3,            }, // 396
    {Y4^X5^Y5,      Z0^X4^Y4,      X5^X6^Y6,      Y2,            }, // 397
    {Y4^X5^Y5,      Z0^X4^Y4,      X5^X6^Y6,      X2,            }, // 398
    {Y4^X6^Y6,      Z1^X4^Y4,      Z0^X5^Y5,      X5^X7^Y7,      }, // 399
    {Y4^X7^Y7,      Z2^X4^Y4,      Z1^Y5^X6,      X6,            }, // 400
    {Y4^X7^Y7,      Z2^X4^Y4,      Z1^Y5^X6,      Y3,            }, // 401
    {Y4^X7^Y7,      Z2^X4^Y4,      Z1^Y5^X6,      X3,            }, // 402
    {X4,            Y4,            Y5^X8,         X5^Y8,         }, // 403
    {Z0^X4^Y4,      Y4,            X5,            Y5^X9,         }, // 404
    {Y4^X6^Y6,      Z0^X4^Y4,      X2,            Y2,            }, // 405
    {Y4^X7^Y7,      Z1^X4^Y4,      S1^Y5^X6,      X2,            }, // 406
    {X4,            Y4,            Y5^X8,         S0^X5^Y8,      }, // 407
    {Y3,            X4,            Y4^X8,         S0^X5^Y7,      }, // 408
    {X3,            Y3,            Y4^X7,         S0^X4^Y7,      }, // 409
    {Y2,            X3,            Y3^X7,         S0^X4^Y6,      }, // 410
    {X2,            Y2,            Y3^X6,         S0^X3^Y6,      }, // 411
    {S2^X4^Y4,      X2,            Y2,            X3^Y6,         }, // 412
    {Y4^X5^Y5,      S2^X4^Y4,      X2,            Y2,            }, // 413
    {Y4^X5^Y5,      Z0^X4^Y4,      X3^X6^Y6,      X2,            }, // 414
    {Y4^X6^Y6,      Z1^X4^Y4,      X5,            Y6,            }, // 415
    {Y4^X6^Y6,      Z0^X4^Y4,      Y2,            X3,            }, // 416
    {Y4^X6^Y6,      S2^X4^Y4,      X2,            Y2,            }, // 417
    {Y4^X6^Y6,      Z0^X4^Y4,      S2^X5^Y5,      X2,            }, // 418
    {Y4^X6^Y6,      Z1^X4^Y4,      Z0^X5^Y5,      X3^X7^Y7,      }, // 419
    {Y4^X7^Y7,      Z0^X4^Y4,      S2^Y5^X6,      Y2,            }, // 420
    {Y4^X7^Y7,      Z0^X4^Y4,      S2^Y5^X6,      X2,            }, // 421
    {Y4^X7^Y7,      Z1^X4^Y4,      Z0^Y5^X6,      S2^X5^Y6,      }, // 422
    {Y4^X8^Y8,      Z1^X4^Y4,      Z0^Y5^X7,      S2^X5^Y7,      }, // 423
    {X4,            Y4,            Y5^X10,        X5^Y10,        }, // 424
    {Y4^X5^Y5,      Z0^X4^Y4,      S0^X6^Y6,      X2,            }, // 425
    {Y4^X6^Y6,      Z1^X4^Y4,      Z0^X5^Y5,      S0^X7^Y7,      }, // 426
    {Y4^X7^Y7,      Z1^X4^Y4,      Z0^Y5^X6,      S0^X5^Y6,      }, // 427
    {Y4^X8^Y8,      Z1^X4^Y4,      Z0^Y5^X7,      S0^X5^Y7,      }, // 428
    {Y4^X9^Y9,      Z1^X4^Y4,      Z0^Y5^X8,      S0^X5^Y8,      }, // 429
    {Y4^X5^Y5,      Z0^X4^Y4,      S1^X6^Y6,      X2,            }, // 430
    {Y4^X6^Y6,      Z0^X4^Y4,      S1^X5^Y5,      X2,            }, // 431
    {Y4^X6^Y6,      Z1^X4^Y4,      Z0^X5^Y5,      S1^X7^Y7,      }, // 432
    {Y4^X6^Y6,      Z0^X4^Y4,      S1^X5^Y5,      S0^X7^Y7,      }, // 433
    {Y4^X7^Y7,      Z1^X4^Y4,      S1^Y5^X6,      Y2,            }, // 434
    {Y4^X7^Y7,      Z0^X4^Y4,      S1^Y5^X6,      X2,            }, // 435
    {Y4^X7^Y7,      Z1^X4^Y4,      S1^Y5^X6,      Z0^X5^Y6,      }, // 436
    {Y4^X7^Y7,      Z0^X4^Y4,      S1^Y5^X6,      S0^X5^Y6,      }, // 437
    {Y4^X8^Y8,      Z1^X4^Y4,      S1^Y5^X7,      Z0^X5^Y7,      }, // 438
    {Y4^X8^Y8,      Z0^X4^Y4,      S1^Y5^X7,      S0^X5^Y7,      }, // 439
    {Y4^X9^Y9,      Z1^X4^Y4,      S1^Y5^X8,      Z0^X5^Y8,      }, // 440
    {Y4^X9^Y9,      Z0^X4^Y4,      S1^Y5^X8,      S0^X5^Y8,      }, // 441
    {Y4^X5^Y5,      Z0^X4^Y4,      S2^X6^Y6,      X3,            }, // 442
    {Y4^X5^Y5,      Z0^X4^Y4,      S2^X6^Y6,      Y2,            }, // 443
    {Y4^X5^Y5,      S2^X4^Y4,      S1^X6^Y6,      X2,            }, // 444
    {Y4^X6^Y6,      Z0^X4^Y4,      S2^X5^Y5,      Y2,            }, // 445
    {Y4^X6^Y6,      S2^X4^Y4,      S1^X5^Y5,      X2,            }, // 446
    {Y4^X6^Y6,      Z1^X4^Y4,      Z0^X5^Y5,      S2^X7^Y7,      }, // 447
    {Y4^X6^Y6,      Z0^X4^Y4,      S2^X5^Y5,      S1^X7^Y7,      }, // 448
    {Y4^X6^Y6,      S2^X4^Y4,      S1^X5^Y5,      S0^X7^Y7,      }, // 449
    {Y4^X7^Y7,      Z1^X4^Y4,      Z0^Y5^X6,      Y6,            }, // 450
    {Y4^X7^Y7,      S2^X4^Y4,      S1^Y5^X6,      X2,            }, // 451
    {Y4^X7^Y7,      Z0^X4^Y4,      S2^Y5^X6,      S1^X5^Y6,      }, // 452
    {Y4^X7^Y7,      S2^X4^Y4,      S1^Y5^X6,      S0^X5^Y6,      }, // 453
    {Y4^X8^Y8,      Z0^X4^Y4,      S2^Y5^X7,      S1^X5^Y7,      }, // 454
    {Y4^X8^Y8,      S2^X4^Y4,      S1^Y5^X7,      S0^X5^Y7,      }, // 455
    {Y4^X9^Y9,      Z1^X4^Y4,      Z0^Y5^X8,      S2^X5^Y8,      }, // 456
    {Y4^X9^Y9,      Z0^X4^Y4,      S2^Y5^X8,      S1^X5^Y8,      }, // 457
    {Y4^X9^Y9,      S2^X4^Y4,      S1^Y5^X8,      S0^X5^Y8,      }, // 458
    {X4^Y4,         Y2,            Z3,            Y3,            }, // 459
    {X4^Y4,         Y2,            Z2,            Y3,            }, // 460
    {X4^Y4,         Y1,            Z2,            Y2,            }, // 461
    {Y1^X4^Y4,      X1,            Z2,            Y2,            }, // 462
    {Y4^X5^Y5,      X4^Y4,         Y2,            Z3,            }, // 463
    {Y4^X5^Y5,      X4^Y4,         Y2,            Z2,            }, // 464
    {Z3^Y4^X5^Y5,   X4^Y4,         Y1,            Z2,            }, // 465
    {Z3^Y4^X5^Y5,   Y1^X4^Y4,      X1,            Z2,            }, // 466
    {Y4^X5^Y5,      X4^Y4,         Z3^X5,         Y2,            }, // 467
    {Y4^X5^Y5,      X4^Y4,         Z2^X5,         Y2,            }, // 468
    {Z3^Y4^X5^Y5,   X4^Y4,         Z2^X5,         Y1,            }, // 469
    {Z3^Y4^X5^Y5,   Y1^X4^Y4,      Z2^X5,         X1,            }, // 470
    {Y4^X6^Y6,      X4^Y4,         Y2,            Y3,            }, // 471
    {Y4^X6^Y6,      X4^Y4,         Z3,            Y3,            }, // 472
    {Y4^X6^Y6,      X4^Y4,         Z2,            Y3,            }, // 473
    {Z3^Y4^X6^Y6,   X4^Y4,         Z2,            Y2,            }, // 474
    {Z3^Y4^X6^Y6,   Y1^X4^Y4,      Z2,            Y2,            }, // 475
    {Y4^X6^Y6,      X4^Y4,         X5^Y5,         Y2,            }, // 476
    {Y4^X6^Y6,      X4^Y4,         Y2^X5^Y5,      Z3,            }, // 477
    {Y4^X6^Y6,      X4^Y4,         Y2^X5^Y5,      Z2,            }, // 478
    {Z3^Y4^X6^Y6,   X4^Y4,         Y1^X5^Y5,      Z2,            }, // 479
    {Z3^Y4^X6^Y6,   Y1^X4^Y4,      X1^X5^Y5,      Z2,            }, // 480
    {Y4^X6^Y6,      X4^Y4,         X5^Y5,         Z3^X6,         }, // 481
    {Y4^X6^Y6,      X4^Y4,         Y2^X5^Y5,      Z3^X6,         }, // 482
    {Y4^X6^Y6,      X4^Y4,         Y2^X5^Y5,      Z2^X6,         }, // 483
    {Z3^Y4^X6^Y6,   X4^Y4,         Y1^X5^Y5,      Z2^X6,         }, // 484
    {Z3^Y4^X6^Y6,   Y1^X4^Y4,      X1^X5^Y5,      Z2^X6,         }, // 485
    {Y4^X7^Y7,      X4^Y4,         Y2^Y5^X6,      Y3,            }, // 486
    {Z3^Y4^X7^Y7,   X4^Y4,         Y1^Y5^X6,      Y2,            }, // 487
    {Z3^Y4^X7^Y7,   Y1^X4^Y4,      X1^Y5^X6,      Y2,            }, // 488
    {Y4^X7^Y7,      X4^Y4,         Y2^Y5^X6,      X5^Y6,         }, // 489
    {Y4^X7^Y7,      X4^Y4,         Y2^Y5^X6,      Z3^X5^Y6,      }, // 490
    {Y4^X7^Y7,      X4^Y4,         Y2^Y5^X6,      Z2^X5^Y6,      }, // 491
    {Z3^Y4^X7^Y7,   X4^Y4,         Y1^Y5^X6,      Z2^X5^Y6,      }, // 492
    {Z3^Y4^X7^Y7,   Y1^X4^Y4,      X1^Y5^X6,      Z2^X5^Y6,      }, // 493
    {Y4^X7^Y7,      X4^Y4,         Y2^Y5^X6,      Y3^X5^Y6,      }, // 494
    {Z3^Y4^X7^Y7,   X4^Y4,         Y1^Y5^X6,      Y2^X5^Y6,      }, // 495
    {Z3^Y4^X7^Y7,   Y1^X4^Y4,      X1^Y5^X6,      Y2^X5^Y6,      }, // 496
    {Y4^X8^Y8,      X4^Y4,         Y2^Y5^X7,      X5^Y7,         }, // 497
    {Y4^X8^Y8,      X4^Y4,         Y2^Y5^X7,      Z3^X5^Y7,      }, // 498
    {Y4^X8^Y8,      X4^Y4,         Y2^Y5^X7,      Z2^X5^Y7,      }, // 499
    {Z3^Y4^X8^Y8,   X4^Y4,         Y1^Y5^X7,      Z2^X5^Y7,      }, // 500
    {Z3^Y4^X8^Y8,   Y1^X4^Y4,      X1^Y5^X7,      Z2^X5^Y7,      }, // 501
    {Y4^X8^Y8,      X4^Y4,         Y2^Y5^X7,      Y3^X5^Y7,      }, // 502
    {Z3^Y4^X8^Y8,   X4^Y4,         Y1^Y5^X7,      Y2^X5^Y7,      }, // 503
    {Z3^Y4^X8^Y8,   Y1^X4^Y4,      X1^Y5^X7,      Y2^X5^Y7,      }, // 504
    {Y4^X9^Y9,      X4^Y4,         Y2^Y5^X8,      X5^Y8,         }, // 505
    {Y4^X9^Y9,      X4^Y4,         Y2^Y5^X8,      Z3^X5^Y8,      }, // 506
    {Y4^X9^Y9,      X4^Y4,         Y2^Y5^X8,      Z2^X5^Y8,      }, // 507
    {Z3^Y4^X9^Y9,   X4^Y4,         Y1^Y5^X8,      Z2^X5^Y8,      }, // 508
    {Z3^Y4^X9^Y9,   Y1^X4^Y4,      X1^Y5^X8,      Z2^X5^Y8,      }, // 509
};

const UINT_64 GFX10_SW_PATTERN_NIBBLE3[][4] =
{
    {0,             0,             0,             0,             }, // 0
    {Y6,            X6,            Y7,            X7,            }, // 1
    {Y5,            X6,            Y6,            X7,            }, // 2
    {Y5,            X5,            Y6,            X6,            }, // 3
    {Y4,            X5,            Y5,            X6,            }, // 4
    {Y4,            X4,            Y5,            X5,            }, // 5
    {Z0^X6^Y6,      X6,            Y7,            X7,            }, // 6
    {Z0^Y5^X6,      X6,            Y6,            X7,            }, // 7
    {Z0^X5^Y5,      X5,            Y6,            X6,            }, // 8
    {Z0^Y4^X5,      X5,            Y5,            X6,            }, // 9
    {Z0^X4^Y4,      X4,            Y5,            X5,            }, // 10
    {Z1^Y6^X7,      Z0^X6^Y7,      Y7,            X7,            }, // 11
    {Z1^Y5^X7,      Z0^X6^Y6,      Y6,            X7,            }, // 12
    {Z1^Y5^X6,      Z0^X5^Y6,      Y6,            X6,            }, // 13
    {Z1^Y4^X6,      Z0^X5^Y5,      Y5,            X6,            }, // 14
    {Z1^Y4^X5,      Z0^X4^Y5,      Y5,            X5,            }, // 15
    {X6^Y6,         X6,            Y7,            X7,            }, // 16
    {Y5^X6,         X6,            Y6,            X7,            }, // 17
    {X5^Y5,         X5,            Y6,            X6,            }, // 18
    {Y4^X5,         X5,            Y5,            X6,            }, // 19
    {X4^Y4,         X4,            Y5,            X5,            }, // 20
    {Y6^X7,         X6^Y7,         Y7,            X7,            }, // 21
    {Y5^X7,         X6^Y6,         Y6,            X7,            }, // 22
    {Y5^X6,         X5^Y6,         Y6,            X6,            }, // 23
    {Y4^X6,         X5^Y5,         Y5,            X6,            }, // 24
    {Y4^X5,         X4^Y5,         Y5,            X5,            }, // 25
    {Y3,            X4,            Y5,            X5,            }, // 26
    {Y4,            X5,            Y6,            X6,            }, // 27
    {Y2,            X4,            Y5,            X6,            }, // 28
    {Y2,            X3,            Y4,            X5,            }, // 29
    {Y4,            X6,            Y6,            X7,            }, // 30
    {Y3,            X4,            Y6,            X6,            }, // 31
    {Y2,            X3,            Y4,            X6,            }, // 32
    {Y2,            X2,            Y3,            X4,            }, // 33
    {Z0^X6^Y6,      X4,            Y6,            X7,            }, // 34
    {Z0^X6^Y6,      X3,            Y4,            X6,            }, // 35
    {Z0^X6^Y6,      Y2,            X3,            Y4,            }, // 36
    {Y2^X6^Y6,      X2,            Y3,            X4,            }, // 37
    {Z1^Y6^X7,      Z0^X6^Y7,      Y4,            X7,            }, // 38
    {Z1^Y6^X7,      Z0^X6^Y7,      Y3,            X4,            }, // 39
    {Y2^Y6^X7,      Z0^X6^Y7,      Y3,            X4,            }, // 40
    {Y2^Y6^X7,      X2^X6^Y7,      Y3,            X4,            }, // 41
    {X5,            Y6,            X6,            Y7,            }, // 42
    {Y5,            X5,            Y6,            Y2^Y7,         }, // 43
    {X4,            Y5,            X5,            Y2^Y6,         }, // 44
    {Y4,            X4,            Y5,            Y1^Y6,         }, // 45
    {Y3,            X4,            Y5,            Y1^Y6,         }, // 46
    {Y4,            X5,            Y6,            Y2^Y7,         }, // 47
    {X3,            Y4,            X5,            Y2^Y6,         }, // 48
    {Y2,            X3,            Y4,            Y1^Y6,         }, // 49
    {Y4,            Y6,            X6,            Y7,            }, // 50
    {Y3,            X4,            Y6,            Y2^Y7,         }, // 51
    {X2,            Y3,            X4,            Y2^Y6,         }, // 52
    {Y1,            X3,            Y4,            X2^Y6,         }, // 53
    {Z0^X6^Y6,      Y4,            X6,            Y7,            }, // 54
    {Z0^X6^Y6,      X3,            Y4,            Y2^Y7,         }, // 55
    {Y2^X6^Y6,      Y3,            X4,            X2^Y7,         }, // 56
    {X2^X6^Y6,      X3,            Y4,            Y1^Y7,         }, // 57
    {Z0^Y6^X7,      Z5^X6^Y7,      Y4,            Y7,            }, // 58
    {Z0^Y6^X7,      Z5^X6^Y7,      Y3,            X4,            }, // 59
    {Z0^Y6^X7,      Y2^X6^Y7,      X3,            Y4,            }, // 60
    {X2^Y6^X7,      Y1^X6^Y7,      X3,            Y4,            }, // 61
    {X5,            Y5,            X6,            Y2^Y6,         }, // 62
    {Y5,            X5,            Y2^Y6,         X2^Y7,         }, // 63
    {Y4,            X5,            Y1^Y5,         X2^Y6,         }, // 64
    {Y4,            X4,            Y1^Y5,         X1^Y6,         }, // 65
    {Y5,            X5,            X2^Y6,         Y2^Y7,         }, // 66
    {Y4,            X5,            X2^Y5,         Y1^Y6,         }, // 67
    {Y4,            X4,            X1^Y5,         Y1^Y6,         }, // 68
    {Y3,            X4,            Y1^Y5,         X1^Y6,         }, // 69
    {X4,            Y5,            X6,            Y2^Y6,         }, // 70
    {Y4,            X5,            X2^Y6,         Y2^Y7,         }, // 71
    {X3,            Y4,            Y1^Y5,         X2^Y6,         }, // 72
    {Y3,            X4,            X1^Y6,         Y1^Y7,         }, // 73
    {X3,            Y4,            X6,            Y2^Y6,         }, // 74
    {Y3,            X4,            Y2^Y6,         X2^Y7,         }, // 75
    {Y3,            X4,            Y1^Y6,         X2^Y7,         }, // 76
    {Z4^X6^Y6,      X3,            Y4,            X6,            }, // 77
    {Z4^X6^Y6,      X3,            Y4,            Y2^Y6,         }, // 78
    {Y1^X6^Y6,      Y3,            X4,            X2^Y7,         }, // 79
    {Z5^Y6^X7,      Z4^X6^Y7,      Y3,            X4,            }, // 80
    {Y2^Y6^X7,      Z4^X6^Y7,      Y3,            X4,            }, // 81
    {Y1^Y6^X7,      X2^X6^Y7,      Y3,            X4,            }, // 82
    {Y5,            Y1^Y6,         Y2^Y7,         X2^Y8,         }, // 83
    {X4,            Y1^Y5,         X1^Y6,         Y2^Y7,         }, // 84
    {Y4,            Y0^Y5,         Y1^Y6,         X1^Y7,         }, // 85
    {Y5,            Y1^Y6,         X2^Y7,         Y2^Y8,         }, // 86
    {X4,            X1^Y5,         Y1^Y6,         X2^Y7,         }, // 87
    {Y4,            Y0^Y5,         X1^Y6,         Y1^Y7,         }, // 88
    {X3,            Y0^Y5,         X1^Y6,         Y1^Y7,         }, // 89
    {Y4,            Y1^Y6,         X2^Y7,         Y2^Y8,         }, // 90
    {X4,            X1^Y6,         Y1^Y7,         X2^Y8,         }, // 91
    {X3,            X1^Y6,         Y1^Y7,         X2^Y8,         }, // 92
    {X3,            Y4,            X2^Y6,         Y1^Y7,         }, // 93
    {X3,            Y1^Y6,         X2^Y7,         Y2^Y8,         }, // 94
    {Z3^X6^Y6,      X3,            Y4,            Y2^Y7,         }, // 95
    {Y2^X6^Y6,      X3,            X2^Y7,         Y1^Y8,         }, // 96
    {Z3^Y6^X7,      Y2^X6^Y7,      X3,            Y4,            }, // 97
    {Y2^Y6^X7,      X2^X6^Y7,      X3,            Y1^Y7,         }, // 98
    {Y6,            X6,            Y7,            S0^Y8,         }, // 99
    {Y5,            X6,            Y6,            S0^Y7,         }, // 100
    {Y5,            X5,            Y6,            S0^Y7,         }, // 101
    {Y4,            X5,            Y5,            S0^Y6,         }, // 102
    {Y4,            X4,            Y5,            S0^Y6,         }, // 103
    {Y3,            X4,            Y5,            S0^Y6,         }, // 104
    {Y4,            X5,            Y6,            S0^Y7,         }, // 105
    {Y2,            X4,            Y5,            S0^Y6,         }, // 106
    {Y2,            X3,            Y4,            S0^Y6,         }, // 107
    {Y4,            X6,            Y6,            S0^Y7,         }, // 108
    {Y3,            X4,            Y6,            S0^Y7,         }, // 109
    {Z0^X6^Y6,      X6,            Y7,            S0^Y8,         }, // 110
    {Z0^X6^Y6,      X4,            Y6,            S0^Y7,         }, // 111
    {Z0^X6^Y6,      X3,            Y4,            S0^Y7,         }, // 112
    {S0^X6^Y6,      Y2,            X3,            Y4,            }, // 113
    {Z0^Y6^X7,      Z5^X6^Y7,      Y7,            S0^Y8,         }, // 114
    {Z0^Y6^X7,      Z5^X6^Y7,      Y4,            S0^Y7,         }, // 115
    {Z0^Y6^X7,      S0^X6^Y7,      Y3,            X4,            }, // 116
    {S0^Y6^X7,      Y2^X6^Y7,      X3,            Y4,            }, // 117
    {Y6,            X6,            S0^Y7,         S1^Y8,         }, // 118
    {Y5,            X6,            S0^Y6,         S1^Y7,         }, // 119
    {Y5,            X5,            S0^Y6,         S1^Y7,         }, // 120
    {Y4,            X5,            S0^Y5,         S1^Y6,         }, // 121
    {Y4,            X4,            S0^Y5,         S1^Y6,         }, // 122
    {Y3,            X4,            S0^Y5,         S1^Y6,         }, // 123
    {Y4,            X5,            S0^Y6,         S1^Y7,         }, // 124
    {X3,            Y4,            S0^Y5,         S1^Y6,         }, // 125
    {Y4,            X6,            S0^Y6,         S1^Y7,         }, // 126
    {Y3,            X4,            S0^Y6,         S1^Y7,         }, // 127
    {Z4^X6^Y6,      X6,            S0^Y7,         S1^Y8,         }, // 128
    {Z4^X6^Y6,      Y4,            S0^Y6,         S1^Y7,         }, // 129
    {S1^X6^Y6,      X3,            Y4,            S0^Y7,         }, // 130
    {Z5^Y6^X7,      Z4^X6^Y7,      S0^Y7,         S1^Y8,         }, // 131
    {S1^Y6^X7,      Z4^X6^Y7,      Y4,            S0^Y7,         }, // 132
    {S1^Y6^X7,      S0^X6^Y7,      Y3,            X4,            }, // 133
    {Y6,            S0^Y7,         S1^Y8,         S2^Y9,         }, // 134
    {Y5,            S0^Y6,         S1^Y7,         S2^Y8,         }, // 135
    {Y4,            S0^Y5,         S1^Y6,         S2^Y7,         }, // 136
    {X3,            S0^Y5,         S1^Y6,         S2^Y7,         }, // 137
    {Y4,            S0^Y6,         S1^Y7,         S2^Y8,         }, // 138
    {X3,            Y4,            S0^Y6,         S1^Y7,         }, // 139
    {Y2,            X3,            S0^Y6,         S1^Y7,         }, // 140
    {X2,            Y2,            X3,            S0^Y6,         }, // 141
    {Z3^X6^Y6,      S0^Y7,         S1^Y8,         S2^Y9,         }, // 142
    {S2^X6^Y6,      Y4,            S0^Y7,         S1^Y8,         }, // 143
    {S0^X6^Y6,      X2,            Y2,            X3,            }, // 144
    {Z3^Y6^X7,      S2^X6^Y7,      S0^Y7,         S1^Y8,         }, // 145
    {S2^Y6^X7,      S1^X6^Y7,      Y4,            S0^Y7,         }, // 146
    {S0^Y6^X7,      X2^X6^Y7,      Y2,            X3,            }, // 147
    {X4,            Z4,            Y4,            X5,            }, // 148
    {X3,            Z4,            Y4,            X4,            }, // 149
    {X3,            Z3,            Y4,            X4,            }, // 150
    {X3,            Z3,            Y3,            X4,            }, // 151
    {X2,            Z3,            Y3,            X3,            }, // 152
    {X4^Y4^Z4,      Z4,            Y4,            X5,            }, // 153
    {X3^Y4^Z4,      Z4,            Y4,            X4,            }, // 154
    {X3^Z3^Y4,      Z3,            Y4,            X4,            }, // 155
    {X3^Y3^Z3,      Z3,            Y3,            X4,            }, // 156
    {X2^Y3^Z3,      Z3,            Y3,            X3,            }, // 157
    {X4^Y5^Z5,      Y4^Z4^X5,      Y4,            X5,            }, // 158
    {X3^Y5^Z5,      X4^Y4^Z4,      Y4,            X4,            }, // 159
    {X3^Z4^Y5,      Z3^X4^Y4,      Y4,            X4,            }, // 160
    {X3^Y4^Z4,      Y3^Z3^X4,      Y3,            X4,            }, // 161
    {X2^Y4^Z4,      X3^Y3^Z3,      Y3,            X3,            }, // 162
    {X4,            Y4^Z4^X5,      Y4,            X5,            }, // 163
    {X3,            X4^Y4^Z4,      Y4,            X4,            }, // 164
    {X3,            Z3^X4^Y4,      Y4,            X4,            }, // 165
    {X3,            Y3^Z3^X4,      Y3,            X4,            }, // 166
    {X2,            X3^Y3^Z3,      Y3,            X3,            }, // 167
    {X3,            Z3,            Y2,            X4,            }, // 168
    {X2,            Z3,            Y2,            X3,            }, // 169
    {X3,            Z4,            Y4,            X5,            }, // 170
    {X2,            Z4,            Y3,            X4,            }, // 171
    {X2,            Z3,            Y3,            X4,            }, // 172
    {Y2,            X3,            Z4,            Y4,            }, // 173
    {Z3,            Y3,            X4,            Z4,            }, // 174
    {Z3^X6^Y6,      Y3,            X4,            Z4,            }, // 175
    {X2^X6^Y6,      Z4,            Y3,            X4,            }, // 176
    {X2^X6^Y6,      Z3,            Y3,            X4,            }, // 177
    {X2^X6^Y6,      Z3,            Y2,            X3,            }, // 178
    {Z3^Y6^X7,      Z4^X6^Y7,      Y3,            X4,            }, // 179
    {X2^Y6^X7,      Z4^X6^Y7,      Y3,            X4,            }, // 180
    {X2^Y6^X7,      Z3^X6^Y7,      Y3,            X4,            }, // 181
    {X2^Y6^X7,      Z3^X6^Y7,      Y2,            X3,            }, // 182
    {X6^Y7,         Y6^X7,         0,             0,             }, // 183
    {Y5^X7,         X6^Y6,         0,             0,             }, // 184
    {X5^Y6,         Y5^X6,         0,             0,             }, // 185
    {Y4^X6,         X5^Y5,         0,             0,             }, // 186
    {X4^Y5,         Y4^X5,         0,             0,             }, // 187
    {Y5^X9,         X7^Y7,         Y6^X8,         0,             }, // 188
    {Y5^X8,         X6^Y7,         Y6^X7,         0,             }, // 189
    {Y4^X8,         X6^Y6,         Y5^X7,         0,             }, // 190
    {Y4^X7,         X5^Y6,         Y5^X6,         0,             }, // 191
    {Y3^X7,         X5^Y5,         Y4^X6,         0,             }, // 192
    {X6^Y9,         Y6^X9,         X7^Y8,         Y7^X8,         }, // 193
    {X6^Y8,         Y5^X9,         X7^Y7,         Y6^X8,         }, // 194
    {X5^Y8,         Y5^X8,         X6^Y7,         Y6^X7,         }, // 195
    {Y3^X8,         X5^Y7,         X6^Y6,         Y5^X7,         }, // 196
    {Y3^X7,         X3^Y7,         X5^Y6,         Y5^X6,         }, // 197
    {X6,            X7^Y9,         Y6^X10,        X8^Y8,         }, // 198
    {Y5,            X6^Y9,         Y6^X9,         X7^Y8,         }, // 199
    {Y3,            X6^Y8,         Y5^X9,         X7^Y7,         }, // 200
    {X3,            Y3^X9,         Y5^X8,         X6^Y7,         }, // 201
    {Y2,            X3^Y7,         Y3^X8,         X6^Y6,         }, // 202
    {Y6^X9,         X7^Y8,         Y7^X8,         Z0^X5^Y5,      }, // 203
    {X6^Y8,         Y6^X8,         X7^Y7,         Z0^X5^Y5,      }, // 204
    {X5^Y8,         X6^Y7,         Y6^X7,         Z0^X5^Y5,      }, // 205
    {Y3^X7,         X5^Y7,         X6^Y6,         Z0^X5^Y5,      }, // 206
    {X3^Y7,         Y3^X6,         X5^Y6,         Z0^X5^Y5,      }, // 207
    {X6,            Y6^X10,        X7^Y9,         Y7^X9,         }, // 208
    {X5,            X6^Y9,         Y6^X9,         X7^Y8,         }, // 209
    {Y3,            X5^Y9,         X6^Y8,         Y6^X8,         }, // 210
    {X3,            Y3^X8,         X5^Y8,         X6^Y7,         }, // 211
    {Y2,            X3^Y8,         Y3^X7,         X5^Y7,         }, // 212
    {X6,            Y6,            X7^Y10,        Y7^X10,        }, // 213
    {Y3,            X6,            Y6^X10,        X7^Y9,         }, // 214
    {X3,            Y3,            X6^Y9,         Y6^X9,         }, // 215
    {Y2,            X3,            Y3^X9,         X6^Y8,         }, // 216
    {X2,            Y2,            X3^Y8,         Y3^X8,         }, // 217
    {Y6,            X7^Y9,         X8^Y8,         Y7^X9,         }, // 218
    {X6,            Y6^X9,         X7^Y8,         Y7^X8,         }, // 219
    {Y3,            X6^Y8,         X7^Y7,         Y6^X8,         }, // 220
    {X3,            Y3^X8,         X6^Y7,         Y6^X7,         }, // 221
    {Y2,            X3^Y7,         Y3^X7,         X6^Y6,         }, // 222
    {Y3,            X6,            X7^Y9,         Y6^X10,        }, // 223
    {X2,            Y2,            Y3^X8,         X3^Y8,         }, // 224
    {X6^Y6,         Y6,            X7,            X8^Y10,        }, // 225
    {X6^Y6,         Y3,            Y6,            X7^Y10,        }, // 226
    {X6^Y6,         X3,            Y3,            X7^Y9,         }, // 227
    {X6^Y6,         Y2,            X3,            Y3^X10,        }, // 228
    {X6^Y6,         X2,            Y2,            X3^Y8,         }, // 229
    {X6,            X7,            Y7^X10,        X8^Y9,         }, // 230
    {Y3,            X6,            X7^Y9,         Y7^X9,         }, // 231
    {X3,            Y3,            X6^Y9,         X7^Y8,         }, // 232
    {Y2,            X3,            Y3^X8,         X6^Y8,         }, // 233
    {X2,            Y2,            X3^Y8,         Y3^X7,         }, // 234
    {X6^Y6,         X6,            X7,            Y7^X11,        }, // 235
    {X6^Y6,         Y3,            X6,            X7^Y10,        }, // 236
    {X6^Y6,         X3,            Y3,            X6^Y10,        }, // 237
    {Z0^X6^Y6,      Y2,            X3,            Y3^X9,         }, // 238
    {Z0^X6^Y6,      X2,            Y2,            X3^Y9,         }, // 239
    {X6^Y6,         X6^Y8,         X7,            Y7,            }, // 240
    {X6^Y6,         X6^Y8,         Y3,            X7,            }, // 241
    {X6^Y6,         X6^Y8,         X3,            Y3,            }, // 242
    {Z0^X6^Y6,      X6^Y8,         Y2,            X3,            }, // 243
    {Z0^X6^Y6,      X6^Y8,         X2,            Y2,            }, // 244
    {Y6^X7,         X7,            Y7,            X8^Y10,        }, // 245
    {Y6^X7,         Y3,            X7,            Y7^X10,        }, // 246
    {Y6^X7,         X3,            Y3,            X7^Y9,         }, // 247
    {Z1^Y6^X7,      Y2,            X3,            Y3^X9,         }, // 248
    {Z1^Y6^X7,      X2,            Y2,            X3^Y8,         }, // 249
    {Y6^X7,         X6^Y7,         X7,            Y7,            }, // 250
    {Y6^X7,         X6^Y7,         Y3,            X7,            }, // 251
    {Y6^X7,         X6^Y7,         X3,            Y3,            }, // 252
    {Z1^Y6^X7,      Z0^X6^Y7,      Y2,            X3,            }, // 253
    {Z1^Y6^X7,      Z0^X6^Y7,      X2,            Y2,            }, // 254
    {X5^Y7,         X6^Y6,         0,             0,             }, // 255
    {Y5^X6,         Y2^X5^Y6,      0,             0,             }, // 256
    {Y4^X6,         X2^X5^Y5,      0,             0,             }, // 257
    {Y4^X5,         Y1^X4^Y5,      0,             0,             }, // 258
    {X5^Y7,         Y5^X7,         Y2^X6^Y6,      0,             }, // 259
    {X5^Y6,         Y4^X7,         X2^Y5^X6,      0,             }, // 260
    {X3^Y6,         Y4^X6,         Y1^X5^Y5,      0,             }, // 261
    {Y5^X9,         Y6^X8,         X6^Y8,         X7^Y7,         }, // 262
    {Y5^X8,         X5^Y8,         Y6^X7,         Y2^X6^Y7,      }, // 263
    {Y3^X8,         X5^Y7,         Y5^X7,         Y2^X6^Y6,      }, // 264
    {Y3^X7,         X3^Y7,         Y5^X6,         Y1^X5^Y6,      }, // 265
    {Y3,            X5^Y9,         X6^Y8,         X7^Y7,         }, // 266
    {Y2,            Y3^X7,         X3^Y8,         X5^Y7,         }, // 267
    {Y6^X8,         X6^Y8,         X7^Y7,         Z0^X5^Y5,      }, // 268
    {X5^Y8,         Y6^X7,         Y2^X6^Y7,      Z0^X5^Y5,      }, // 269
    {Y3^X7,         X5^Y7,         X2^X6^Y6,      Z0^X5^Y5,      }, // 270
    {Y3^X6,         X3^Y7,         Y1^X5^Y6,      Z0^X5^Y5,      }, // 271
    {Y3,            X5,            X6^Y10,        Y7^X9,         }, // 272
    {X3,            Y3,            X5^Y10,        X6^Y9,         }, // 273
    {Y2,            X3,            Y3^X8,         X5^Y9,         }, // 274
    {X2,            Y2,            Y3^X7,         X3^Y9,         }, // 275
    {Y3,            X6^Y8,         Y6^X8,         Y2^X7^Y7,      }, // 276
    {X3,            Y3^X8,         X6^Y7,         X2^Y6^X7,      }, // 277
    {Y2,            Y3^X7,         X3^Y7,         Y1^X6^Y6,      }, // 278
    {Y3,            X6,            Y6^X10,        Y7^X9,         }, // 279
    {X3,            Y3,            Y6^X9,         X6^Y9,         }, // 280
    {X2,            X3,            Y3^X9,         X6^Y8,         }, // 281
    {X6^Y6,         Y2,            X3,            Y3^X9,         }, // 282
    {X6^Y6,         X2,            Y2,            Y3^X8,         }, // 283
    {Y3,            X6,            Y7^X9,         X7^Y9,         }, // 284
    {X3,            Y3,            X6^Y9,         Y7^X8,         }, // 285
    {X2,            Y2,            Y3^X7,         X3^Y8,         }, // 286
    {Z0^Y6^X7,      Y2,            X3,            Y3^X9,         }, // 287
    {Z0^Y6^X7,      X2,            Y2,            Y3^X8,         }, // 288
    {Z0^Y6^X7,      Z4^X6^Y7,      X2,            X3,            }, // 289
    {Z0^Y6^X7,      Z4^X6^Y7,      X2,            Y2,            }, // 290
    {X5^Y6,         Y2^Y5^X6,      0,             0,             }, // 291
    {X2^X5^Y6,      Y2^Y5^X6,      0,             0,             }, // 292
    {X2^X5^Y5,      Y1^Y4^X6,      0,             0,             }, // 293
    {X1^X4^Y5,      Y1^Y4^X5,      0,             0,             }, // 294
    {Y4^X8,         X2^X6^Y6,      Y2^Y5^X7,      0,             }, // 295
    {Y4^X7,         Y2^Y5^X6,      Y1^X5^Y6,      0,             }, // 296
    {Y3^X7,         X1^X5^Y5,      Y1^Y4^X6,      0,             }, // 297
    {X5^Y8,         X6^Y7,         Y5^X8,         Y2^Y6^X7,      }, // 298
    {X5^Y8,         Y5^X8,         X2^Y6^X7,      Y2^X6^Y7,      }, // 299
    {Y3^X8,         X5^Y7,         X2^Y5^X7,      Y1^X6^Y6,      }, // 300
    {Y3^X7,         X3^Y7,         X1^Y5^X6,      Y1^X5^Y6,      }, // 301
    {Y3,            Y5^X9,         X6^Y8,         Y6^X8,         }, // 302
    {Y3,            X6^Y8,         Y5^X9,         X2^X7^Y7,      }, // 303
    {X3,            Y3^X9,         Y5^X8,         Y2^Y6^X7,      }, // 304
    {Y2,            X3^Y7,         Y3^X8,         X1^X6^Y6,      }, // 305
    {X5^Y8,         X6^Y7,         Y2^Y6^X7,      Z0^X5^Y5,      }, // 306
    {X5^Y8,         X2^X6^Y7,      Y2^Y6^X7,      Z0^X5^Y5,      }, // 307
    {Y3^X8,         Y2^Y5^X7,      Y1^X6^Y6,      Z0^X5^Y5,      }, // 308
    {Y3^X7,         Y2^X6^Y6,      X1^X5^Y7,      Y1^X5^Y5,      }, // 309
    {Y3,            X5^Y9,         X6^Y8,         X2^Y6^X8,      }, // 310
    {X3,            Y3^X8,         X5^Y8,         X2^Y6^X7,      }, // 311
    {Y2,            Y3^X8,         X3^Y7,         X1^Y5^X7,      }, // 312
    {Y3,            X6^Y8,         X2^X7^Y7,      Y2^Y6^X8,      }, // 313
    {X3,            Y3^X8,         Y2^Y6^X7,      Y1^X6^Y7,      }, // 314
    {X3,            Y3^X8,         Y2^Y6^X7,      X1^X6^Y7,      }, // 315
    {X6^Y6,         X3,            Y3,            Y6^X10,        }, // 316
    {X6^Y6,         X2,            X3,            Y3^X10,        }, // 317
    {X3,            Y3,            X6^Y9,         X2^X7^Y8,      }, // 318
    {X2,            X3,            Y3^X9,         Y2^Y6^X8,      }, // 319
    {X2,            X3,            Y3^X8,         Y2^X7^Y7,      }, // 320
    {Z3^X6^Y6,      Y2,            X3,            Y3^X9,         }, // 321
    {Z3^X6^Y6,      X2,            Y2,            Y3^X9,         }, // 322
    {Z3^X6^Y6,      X6^Y8,         Y2,            X3,            }, // 323
    {Z3^X6^Y6,      X6^Y8,         X2,            Y2,            }, // 324
    {Z4^Y6^X7,      X2,            X3,            Y3^X9,         }, // 325
    {Y1^Y6^X7,      X2,            X3,            Y3^X9,         }, // 326
    {Z4^Y6^X7,      Z3^X6^Y7,      Y2,            X3,            }, // 327
    {Z4^Y6^X7,      Z3^X6^Y7,      X2,            Y2,            }, // 328
    {Y1^Y4^X6,      X2^X5^Y5,      0,             0,             }, // 329
    {Y1^X5^Y7,      X2^X6^Y6,      Y2^Y5^X7,      0,             }, // 330
    {X1^X5^Y6,      Y1^Y4^X7,      X2^Y5^X6,      0,             }, // 331
    {Y5^X8,         Y1^X5^Y8,      X2^X6^Y7,      Y2^Y6^X7,      }, // 332
    {Y3^X8,         Y1^X5^Y7,      X1^Y5^X7,      Y2^X6^Y6,      }, // 333
    {Y3^X7,         Y1^X4^Y7,      Y2^X5^Y6,      X1^Y5^X6,      }, // 334
    {Y3,            X5^Y9,         X6^Y8,         X2^X7^Y7,      }, // 335
    {Y3,            X5^Y9,         Y1^X6^Y8,      X2^X7^Y7,      }, // 336
    {X3,            Y3^X8,         X5^Y7,         X1^X6^Y6,      }, // 337
    {Y2,            Y3^X7,         X3^Y7,         Y0^X5^Y6,      }, // 338
    {Y1^X5^Y8,      X2^X6^Y7,      Y2^Y6^X7,      Z0^X5^Y5,      }, // 339
    {X1^X5^Y8,      Y2^Y6^X7,      X2^X6^Y7,      Y1^X5^Y5,      }, // 340
    {X1^X5^Y8,      X2^X6^Y7,      Y2^Y6^X7,      Y1^X5^Y5,      }, // 341
    {Y3,            X5^Y9,         Y1^X6^Y8,      X2^Y6^X8,      }, // 342
    {X3,            Y3^X9,         Y1^X6^Y7,      X1^Y5^X8,      }, // 343
    {X3,            Y3^X8,         Y1^X5^Y8,      Y2^X6^Y7,      }, // 344
    {X3,            Y3,            X5^Y10,        Y1^X6^Y9,      }, // 345
    {Y2,            X3,            Y3^X8,         X5^Y8,         }, // 346
    {Y3,            Y1^X6^Y8,      X2^X7^Y7,      Y2^Y6^X8,      }, // 347
    {Y3,            X1^X6^Y8,      Y2^Y6^X8,      X2^X7^Y7,      }, // 348
    {Y3,            X1^X6^Y8,      X2^X7^Y7,      Y2^Y6^X8,      }, // 349
    {X3,            Y3,            Y6^X9,         Y1^X6^Y9,      }, // 350
    {X2,            X3,            Y3^X9,         Y1^X6^Y8,      }, // 351
    {X2^X6^Y6,      Y2,            X3,            Y3^X9,         }, // 352
    {Y1^X6^Y6,      X2,            Y2,            Y3^X8,         }, // 353
    {X3,            Y3,            Y1^X6^Y9,      X2^X7^Y8,      }, // 354
    {X3,            Y3,            X1^X6^Y9,      Y2^Y7^X8,      }, // 355
    {X3,            Y3,            X1^X6^Y9,      X2^X7^Y8,      }, // 356
    {Z2^X6^Y6,      X2,            X3,            Y3^X10,        }, // 357
    {Y0^X6^Y6,      X2,            X3,            Y3^X9,         }, // 358
    {Z2^X6^Y6,      X6^Y8,         Y2,            X3,            }, // 359
    {Z2^X6^Y6,      Y1^X6^Y8,      X2,            Y2,            }, // 360
    {Y6^X7,         X3,            Y3,            Y1^X7^Y9,      }, // 361
    {Y1^Y6^X7,      X3,            Y3,            X1^X7^Y9,      }, // 362
    {Y0^Y6^X7,      X3,            Y3,            X1^X7^Y9,      }, // 363
    {Z3^Y6^X7,      Z2^X6^Y7,      X2,            X3,            }, // 364
    {Z2^Y6^X7,      Y0^X6^Y7,      X2,            X3,            }, // 365
    {Y5^X9,         X6^Y8,         Y6^X8,         X7^Y7,         }, // 366
    {Y4^X8,         X5^Y7,         Y5^X7,         X6^Y6,         }, // 367
    {X4^Y7,         Y4^X7,         X5^Y6,         Y5^X6,         }, // 368
    {X5^Y7,         Y4^X8,         X6^Y6,         Y5^X7,         }, // 369
    {X3^Y7,         Y4^X7,         X5^Y6,         Y5^X6,         }, // 370
    {Y5,            X6^Y8,         X7^Y7,         Y6^X8,         }, // 371
    {Y3,            Y5^X8,         X6^Y7,         Y6^X7,         }, // 372
    {X3,            Y3^X8,         X6^Y6,         Y5^X7,         }, // 373
    {Y2,            Y3^X7,         X3^Y6,         Y5^X6,         }, // 374
    {X5,            X6^Y8,         Y6^X8,         X7^Y7,         }, // 375
    {Y3,            X5^Y8,         X6^Y7,         Y6^X7,         }, // 376
    {X3,            Y3^X7,         X5^Y7,         X6^Y6,         }, // 377
    {Y2,            X3^Y7,         Y3^X6,         X5^Y6,         }, // 378
    {X6,            Y6,            X7^Y8,         Y7^X8,         }, // 379
    {Y3,            X6,            Y6^X8,         X7^Y7,         }, // 380
    {X3,            Y3,            X6^Y7,         Y6^X7,         }, // 381
    {Y2,            X3,            Y3^X7,         X6^Y6,         }, // 382
    {X2,            Y2,            X3^Y6,         Y3^X6,         }, // 383
    {Y6,            X7^Y8,         Y7^X8,         X5^Y6,         }, // 384
    {X6,            X7^Y7,         Y6^X8,         X5^Y6,         }, // 385
    {Y3,            X6^Y7,         Y6^X7,         X5^Y6,         }, // 386
    {X3,            Y3^X7,         X6^Y6,         Z0^X5^Y6,      }, // 387
    {Y2,            Y3^X6,         X3^Y6,         Z0^X5^Y6,      }, // 388
    {Y3,            X6,            X7^Y7,         Y6^X8,         }, // 389
    {X2,            Y2,            Y3^X6,         X3^Y6,         }, // 390
    {X6^Y6,         Y6,            X7,            Y7^X8,         }, // 391
    {X6^Y6,         Y3,            Y6,            X7^Y7,         }, // 392
    {X6^Y6,         X3,            Y3,            Y6^X7,         }, // 393
    {X6^Y6,         Y2,            X3,            Y3^X7,         }, // 394
    {X3^Y6,         X2,            Y2,            Y3^X6,         }, // 395
    {X6,            X7,            Y7^X8,         X6^Y6,         }, // 396
    {Y3,            X6,            X7^Y7,         X6^Y6,         }, // 397
    {X3,            Y3,            X6^Y7,         X6^Y6,         }, // 398
    {Y2,            X3,            Y3^X7,         Z0^X6^Y6,      }, // 399
    {X2,            X3,            Y3^X6,         Y2^X6^Y6,      }, // 400
    {X6^Y6,         X6,            X7,            Y7^X8,         }, // 401
    {X6^Y6,         Y3,            X6,            X7^Y7,         }, // 402
    {X6^Y6,         X3,            Y3,            X6^Y7,         }, // 403
    {Z0^X6^Y6,      Y2,            X3,            Y3^X7,         }, // 404
    {Y2^X6^Y6,      X2,            X3,            Y3^X6,         }, // 405
    {Z0^X6^Y6,      X3^Y8,         Y2,            Y3,            }, // 406
    {Y2^X6^Y6,      X3^Y8,         X2,            Y3,            }, // 407
    {Y6^X7,         X7,            Y7,            X6^Y7,         }, // 408
    {Y6^X7,         Y3,            X7,            X6^Y7,         }, // 409
    {Y6^X7,         X3,            Y3,            X6^Y7,         }, // 410
    {Y2^Y6^X7,      X3,            Y3,            Z0^X6^Y7,      }, // 411
    {Y2^Y6^X7,      X3,            Y3,            X2^X6^Y7,      }, // 412
    {Y2^Y6^X7,      Z0^X6^Y7,      X3,            Y3,            }, // 413
    {Y2^Y6^X7,      X2^X6^Y7,      X3,            Y3,            }, // 414
    {X5^Y9,         Y6^X8,         X6^Y8,         X7^Y7,         }, // 415
    {Y4^X8,         X5^Y7,         Y5^X7,         X2^X6^Y6,      }, // 416
    {Y4^X7,         X4^Y7,         Y5^X6,         Y1^X5^Y6,      }, // 417
    {Y4^X8,         X5^Y7,         Y5^X7,         Y2^X6^Y6,      }, // 418
    {Y4^X7,         X3^Y7,         Y5^X6,         Y1^X5^Y6,      }, // 419
    {X5,            Y6^X8,         X6^Y8,         X7^Y7,         }, // 420
    {Y3,            X5^Y8,         Y6^X7,         Y2^X6^Y7,      }, // 421
    {X3,            Y3^X7,         X5^Y7,         Y2^X6^Y6,      }, // 422
    {Y2,            Y3^X6,         X3^Y7,         Y1^X5^Y6,      }, // 423
    {X3,            Y3^X7,         X5^Y7,         X2^X6^Y6,      }, // 424
    {Y3,            X5,            X6^Y8,         X7^Y7,         }, // 425
    {X3,            Y3,            X5^Y8,         X6^Y7,         }, // 426
    {X3,            Y3,            X5^Y8,         Y2^X6^Y7,      }, // 427
    {Y2,            X3,            Y3^X6,         X5^Y6,         }, // 428
    {X2,            Y2,            Y3^X5,         X3^Y6,         }, // 429
    {X6,            Y6^X8,         X7^Y7,         X5^Y6,         }, // 430
    {Y3,            Y6^X7,         Y2^X6^Y7,      X5^Y6,         }, // 431
    {X3,            Y3^X7,         Y2^X6^Y6,      Z0^X5^Y6,      }, // 432
    {X3,            Y3^X7,         Y2^X6^Y6,      Y1^X5^Y6,      }, // 433
    {X3,            Y3,            Y6^X7,         Y2^X6^Y7,      }, // 434
    {X2,            X3,            Y3^X7,         Y2^X6^Y6,      }, // 435
    {X6^Y6,         X3,            Y3,            Y2^X6^Y7,      }, // 436
    {X3,            Y3,            Y2^X6^Y7,      X6^Y6,         }, // 437
    {X3,            Y3,            X2^X6^Y7,      Y2^X6^Y6,      }, // 438
    {Y2^X6^Y6,      X3,            Y3,            X2^X6^Y7,      }, // 439
    {X6^Y6,         X6^Y8,         Y3,            Y7,            }, // 440
    {X6^Y6,         Y2^X6^Y8,      X3,            Y3,            }, // 441
    {Y2^X6^Y6,      X2^X6^Y8,      X3,            Y3,            }, // 442
    {Y6^X7,         Y3,            Y7,            X6^Y7,         }, // 443
    {Y6^X7,         X3,            Y3,            Y2^X6^Y7,      }, // 444
    {Y6^X7,         X6^Y7,         Y3,            Y7,            }, // 445
    {Y6^X7,         Y2^X6^Y7,      X3,            Y3,            }, // 446
    {X5^Y8,         Y5^X8,         X6^Y7,         Y2^Y6^X7,      }, // 447
    {X5^Y8,         Y5^X8,         X2^X6^Y7,      Y2^Y6^X7,      }, // 448
    {Y4^X8,         X5^Y7,         X2^X6^Y6,      Y1^Y5^X7,      }, // 449
    {X4^Y7,         Y4^X7,         X1^X5^Y6,      Y1^Y5^X6,      }, // 450
    {Y4^X9,         X6^Y7,         Y5^X8,         Y2^Y6^X7,      }, // 451
    {X5^Y7,         Y4^X8,         X2^Y5^X7,      Y1^X6^Y6,      }, // 452
    {X3^Y7,         Y4^X7,         X1^Y5^X6,      Y1^X5^Y6,      }, // 453
    {Y3,            X6^Y7,         Y5^X8,         Y2^Y6^X7,      }, // 454
    {Y3,            Y5^X8,         X2^Y6^X7,      Y2^X6^Y7,      }, // 455
    {X3,            Y3^X8,         X2^Y5^X7,      Y1^X6^Y6,      }, // 456
    {Y2,            Y3^X6,         X3^Y6,         X1^X5^Y5,      }, // 457
    {Y3,            X5^Y8,         X6^Y7,         Y2^Y6^X7,      }, // 458
    {Y3,            X5^Y8,         X2^X6^Y7,      Y2^Y6^X7,      }, // 459
    {X3,            Y3^X8,         Y2^Y5^X7,      Y1^X6^Y6,      }, // 460
    {X3,            Y3^X7,         Y2^X6^Y6,      X1^X5^Y7,      }, // 461
    {X3,            Y3,            X6^Y7,         Y2^Y6^X7,      }, // 462
    {X3,            Y3,            X2^X6^Y7,      Y2^Y6^X7,      }, // 463
    {X2,            X3,            Y3^X7,         Y2^Y5^X6,      }, // 464
    {X2,            X3,            Y3^X6,         Y2^X5^Y6,      }, // 465
    {Y3,            X6^Y7,         Y2^Y6^X7,      X5^Y6,         }, // 466
    {Y3,            X2^Y6^X7,      Y2^X6^Y7,      X5^Y6,         }, // 467
    {Y3,            X2^Y6^X7,      Y2^X6^Y7,      Z0^X5^Y6,      }, // 468
    {Y3,            X2^Y6^X7,      Y2^X6^Y7,      X1^X5^Y6,      }, // 469
    {X3,            Y3,            X2^Y6^X7,      Y2^X6^Y7,      }, // 470
    {X6^Y6,         X3,            Y3,            Y2^Y6^X7,      }, // 471
    {Y2^X6^Y6,      X3,            Y3,            X2^X6^Y6,      }, // 472
    {X3,            Y3,            Y2^Y6^X7,      X6^Y6,         }, // 473
    {Y2^Y6^X7,      X3,            Y3,            X6^Y7,         }, // 474
    {Y2^Y6^X7,      X6^Y7,         X3,            Y3,            }, // 475
    {Y4^X8,         X1^X5^Y7,      Y1^Y5^X7,      X2^X6^Y6,      }, // 476
    {Y4^X7,         Y0^X4^Y7,      X1^X5^Y6,      Y1^Y5^X6,      }, // 477
    {Y4^X8,         Y1^X5^Y7,      X1^Y5^X7,      Y2^X6^Y6,      }, // 478
    {Y3^X7,         Y0^X4^Y6,      X1^Y4^X6,      Y1^X5^Y5,      }, // 479
    {Y3,            X5^Y8,         X2^Y6^X7,      Y2^X6^Y7,      }, // 480
    {Y3,            Y1^X5^Y8,      X2^X6^Y7,      Y2^Y6^X7,      }, // 481
    {X3,            Y3^X7,         Y1^X5^Y6,      X1^Y5^X6,      }, // 482
    {X3,            Y3^X6,         Y1^X4^Y6,      Y2^X5^Y5,      }, // 483
    {Y3,            X1^X5^Y8,      Y2^Y6^X7,      X2^X6^Y7,      }, // 484
    {Y3,            X1^X5^Y8,      X2^X6^Y7,      Y2^Y6^X7,      }, // 485
    {X3,            Y3,            Y1^X5^Y7,      X2^X6^Y6,      }, // 486
    {X3,            Y3,            X1^X5^Y7,      Y2^X6^Y6,      }, // 487
    {X3,            Y3,            X1^X5^Y7,      X2^X6^Y6,      }, // 488
    {Y3,            X2^Y6^X7,      Y1^X6^Y7,      Y2^X5^Y6,      }, // 489
    {X3,            Y3,            X2^Y6^X7,      Y1^X6^Y7,      }, // 490
    {X2^X6^Y6,      X3,            Y3,            Y1^X6^Y6,      }, // 491
    {X2^X6^Y6,      X3,            Y3,            Y2^X6^Y6,      }, // 492
    {X3,            Y3,            Y1^X6^Y7,      X2^X6^Y6,      }, // 493
    {Y2^X6^Y6,      X3,            Y3,            Y1^X6^Y7,      }, // 494
    {Y2^X6^Y6,      Y1^X6^Y8,      X3,            Y3,            }, // 495
    {Y2^Y6^X7,      X3,            Y3,            Y1^X6^Y7,      }, // 496
    {X6^X8^Y8,      Y6,            X7,            X8^Y10,        }, // 497
    {X6^X8^Y8,      Y3,            Y6,            X7^Y10,        }, // 498
    {X6^X8^Y8,      X3,            Y3,            X7^Y9,         }, // 499
    {X6^X8^Y8,      Y2,            X3,            Y3^X10,        }, // 500
    {X6^X8^Y8,      X2,            Y2,            X3^Y8,         }, // 501
    {Z0^X6^Y6,      X6,            X7,            Y7^X11,        }, // 502
    {Z0^X6^Y6,      Y3,            X6,            X7^Y10,        }, // 503
    {Z0^X6^Y6,      X3,            Y3,            X6^Y10,        }, // 504
    {Z0^X6^Y6,      X6^X9^Y9,      X7,            Y7,            }, // 505
    {Z0^X6^Y6,      X6^X9^Y9,      Y3,            X7,            }, // 506
    {Z0^X6^Y6,      X6^X9^Y9,      X3,            Y3,            }, // 507
    {Z0^X6^Y6,      X6^X9^Y9,      Y2,            X3,            }, // 508
    {Z0^X6^Y6,      X6^X9^Y9,      X2,            Y2,            }, // 509
    {Z1^Y6^X7,      X7,            Y7,            X8^Y10,        }, // 510
    {Z1^Y6^X7,      Y3,            X7,            Y7^X10,        }, // 511
    {Z1^Y6^X7,      X3,            Y3,            X7^Y9,         }, // 512
    {Z1^Y6^X7,      Z0^X6^Y7,      X7,            Y7,            }, // 513
    {Z1^Y6^X7,      Z0^X6^Y7,      Y3,            X7,            }, // 514
    {Z1^Y6^X7,      Z0^X6^Y7,      X3,            Y3,            }, // 515
    {Y6^X7,         S0^X6^Y7,      0,             0,             }, // 516
    {Y5^X7,         S0^X6^Y6,      0,             0,             }, // 517
    {Y5^X6,         S0^X5^Y6,      0,             0,             }, // 518
    {Y4^X6,         S0^X5^Y5,      0,             0,             }, // 519
    {Y4^X5,         S0^X4^Y5,      0,             0,             }, // 520
    {X6^Y8,         Y6^X8,         S0^X7^Y7,      0,             }, // 521
    {X6^Y7,         Y5^X8,         S0^Y6^X7,      0,             }, // 522
    {X5^Y7,         Y5^X7,         S0^X6^Y6,      0,             }, // 523
    {X5^Y6,         Y4^X7,         S0^Y5^X6,      0,             }, // 524
    {X3^Y6,         Y4^X6,         S0^X5^Y5,      0,             }, // 525
    {Y6^X9,         X6^Y9,         Y7^X8,         S0^X7^Y8,      }, // 526
    {Y5^X9,         X6^Y8,         Y6^X8,         S0^X7^Y7,      }, // 527
    {Y5^X8,         X5^Y8,         Y6^X7,         S0^X6^Y7,      }, // 528
    {Y3^X8,         X5^Y7,         Y5^X7,         S0^X6^Y6,      }, // 529
    {Y3^X7,         X3^Y7,         Y5^X6,         S0^X5^Y6,      }, // 530
    {Y5,            X6^Y9,         X7^Y8,         Y6^X9,         }, // 531
    {X3,            Y3^X9,         X6^Y7,         Y5^X8,         }, // 532
    {Y2,            Y3^X8,         X3^Y7,         Y5^X7,         }, // 533
    {Y6^X9,         Y7^X8,         S0^X7^Y8,      Z0^X5^Y5,      }, // 534
    {X6^Y8,         Y6^X8,         S0^X7^Y7,      Z0^X5^Y5,      }, // 535
    {X5^Y8,         Y6^X7,         S0^X6^Y7,      Z0^X5^Y5,      }, // 536
    {Y3^X7,         X5^Y7,         S0^X6^Y6,      Z0^X5^Y5,      }, // 537
    {Y3^X6,         X3^Y7,         S0^X5^Y6,      Z0^X5^Y5,      }, // 538
    {X6,            Y6,            Y7^X10,        X7^Y10,        }, // 539
    {Y6,            X7^Y9,         Y7^X9,         S0^X8^Y8,      }, // 540
    {X6,            X7^Y8,         Y6^X9,         S0^Y7^X8,      }, // 541
    {Y3,            X6^Y8,         Y6^X8,         S0^X7^Y7,      }, // 542
    {X3,            Y3^X8,         X6^Y7,         S0^Y6^X7,      }, // 543
    {Y2,            Y3^X7,         X3^Y7,         S0^X6^Y6,      }, // 544
    {X6^X8^Y8,      Y6,            X7,            Y7^X11,        }, // 545
    {X6^X8^Y8,      X3,            Y3,            Y6^X10,        }, // 546
    {X6^X8^Y8,      X2,            Y2,            Y3^X9,         }, // 547
    {X6,            X7,            Y7^X10,        Y8^X9,         }, // 548
    {Z0^Y6^X7,      X7,            Y7,            X8^Y10,        }, // 549
    {Z0^Y6^X7,      Y3,            X7,            X8^Y9,         }, // 550
    {Z0^Y6^X7,      X3,            Y3,            X7^Y9,         }, // 551
    {Z0^Y6^X7,      Z4^X6^Y7,      X7,            Y7,            }, // 552
    {Z0^Y6^X7,      Z4^X6^Y7,      Y3,            X7,            }, // 553
    {Z0^Y6^X7,      Z4^X6^Y7,      X3,            Y3,            }, // 554
    {Z0^Y6^X7,      Z4^X6^Y7,      Y2,            X3,            }, // 555
    {S0^X6^Y7,      S1^Y6^X7,      0,             0,             }, // 556
    {S0^Y5^X7,      S1^X6^Y6,      0,             0,             }, // 557
    {S0^X5^Y6,      S1^Y5^X6,      0,             0,             }, // 558
    {S0^Y4^X6,      S1^X5^Y5,      0,             0,             }, // 559
    {S0^X4^Y5,      S1^Y4^X5,      0,             0,             }, // 560
    {Y5^X9,         S0^X7^Y7,      S1^Y6^X8,      0,             }, // 561
    {Y5^X8,         S0^X6^Y7,      S1^Y6^X7,      0,             }, // 562
    {Y4^X8,         S0^X6^Y6,      S1^Y5^X7,      0,             }, // 563
    {Y4^X7,         S0^X5^Y6,      S1^Y5^X6,      0,             }, // 564
    {Y3^X7,         S0^X5^Y5,      S1^Y4^X6,      0,             }, // 565
    {X6^Y9,         Y6^X9,         S0^X7^Y8,      S1^Y7^X8,      }, // 566
    {X6^Y8,         Y5^X9,         S0^X7^Y7,      S1^Y6^X8,      }, // 567
    {X5^Y8,         Y5^X8,         S0^X6^Y7,      S1^Y6^X7,      }, // 568
    {Y3^X8,         X5^Y7,         S0^X6^Y6,      S1^Y5^X7,      }, // 569
    {Y3^X7,         X3^Y7,         S0^X5^Y6,      S1^Y5^X6,      }, // 570
    {X6,            X7^Y9,         Y6^X10,        S0^X8^Y8,      }, // 571
    {Y5,            X6^Y9,         Y6^X9,         S0^X7^Y8,      }, // 572
    {Y3,            X6^Y8,         Y5^X9,         S0^X7^Y7,      }, // 573
    {X3,            Y3^X9,         Y5^X8,         S0^X6^Y7,      }, // 574
    {Y2,            X3^Y7,         Y3^X8,         S0^X6^Y6,      }, // 575
    {Y6^X9,         S0^X7^Y8,      S1^Y7^X8,      Z0^X5^Y5,      }, // 576
    {X6^Y8,         S0^Y6^X8,      S1^X7^Y7,      Z0^X5^Y5,      }, // 577
    {X5^Y8,         S0^X6^Y7,      S1^Y6^X7,      Z0^X5^Y5,      }, // 578
    {Y3^X8,         S0^X6^Y6,      S1^Y5^X7,      Z0^X5^Y5,      }, // 579
    {Y3^X6,         X3^Y7,         S0^X5^Y6,      S1^X5^Y5,      }, // 580
    {X6,            Y6^X10,        X7^Y9,         S0^Y7^X9,      }, // 581
    {X5,            X6^Y9,         Y6^X9,         S0^X7^Y8,      }, // 582
    {Y3,            X5^Y9,         X6^Y8,         S0^Y6^X8,      }, // 583
    {X3,            Y3^X8,         X5^Y8,         S0^X6^Y7,      }, // 584
    {Y2,            Y3^X8,         X3^Y7,         S0^X6^Y6,      }, // 585
    {Y6,            X7^Y9,         S0^X8^Y8,      S1^Y7^X9,      }, // 586
    {X6,            Y6^X9,         S0^X7^Y8,      S1^Y7^X8,      }, // 587
    {Y3,            X6^Y8,         S0^X7^Y7,      S1^Y6^X8,      }, // 588
    {X3,            Y3^X8,         S0^X6^Y7,      S1^Y6^X7,      }, // 589
    {X6,            X7,            Y7^X10,        S0^X8^Y9,      }, // 590
    {Y3,            X6,            X7^Y9,         S0^Y7^X9,      }, // 591
    {X3,            Y3,            X6^Y9,         S0^X7^Y8,      }, // 592
    {Y2,            X3,            Y3^X9,         S0^X7^Y7,      }, // 593
    {Z3^X6^Y6,      X6,            X7,            Y7^X11,        }, // 594
    {Z3^X6^Y6,      Y3,            X6,            X7^Y10,        }, // 595
    {Z3^X6^Y6,      X3,            Y3,            X6^Y10,        }, // 596
    {Z3^X6^Y6,      X6^X9^Y9,      X7,            Y7,            }, // 597
    {Z3^X6^Y6,      X6^X9^Y9,      Y3,            X7,            }, // 598
    {Z3^X6^Y6,      X6^X9^Y9,      X3,            Y3,            }, // 599
    {Z3^X6^Y6,      X6^X9^Y9,      Y2,            X3,            }, // 600
    {Z3^X6^Y6,      X6^X9^Y9,      X2,            Y2,            }, // 601
    {Z4^Y6^X7,      X7,            Y7,            X8^Y10,        }, // 602
    {Z4^Y6^X7,      Y3,            X7,            Y7^X10,        }, // 603
    {Z4^Y6^X7,      X3,            Y3,            X7^Y9,         }, // 604
    {Z4^Y6^X7,      Y2,            X3,            Y3^X9,         }, // 605
    {S1^Y6^X7,      X2,            Y2,            Y3^X8,         }, // 606
    {Z4^Y6^X7,      Z3^X6^Y7,      X7,            Y7,            }, // 607
    {Z4^Y6^X7,      Z3^X6^Y7,      Y3,            X7,            }, // 608
    {Z4^Y6^X7,      Z3^X6^Y7,      X3,            Y3,            }, // 609
    {S1^Y6^X7,      S2^X6^Y7,      0,             0,             }, // 610
    {S1^Y5^X7,      S2^X6^Y6,      0,             0,             }, // 611
    {S1^Y5^X6,      S2^X5^Y6,      0,             0,             }, // 612
    {S1^Y4^X6,      S2^X5^Y5,      0,             0,             }, // 613
    {S1^Y4^X5,      S2^X4^Y5,      0,             0,             }, // 614
    {S0^X6^Y8,      S1^Y6^X8,      S2^X7^Y7,      0,             }, // 615
    {S0^X6^Y7,      S1^Y5^X8,      S2^Y6^X7,      0,             }, // 616
    {S0^X5^Y7,      S1^Y5^X7,      S2^X6^Y6,      0,             }, // 617
    {S0^X5^Y6,      S1^Y4^X7,      S2^Y5^X6,      0,             }, // 618
    {Y6^X9,         S0^X6^Y9,      S1^Y7^X8,      S2^X7^Y8,      }, // 619
    {Y5^X9,         S0^X6^Y8,      S1^Y6^X8,      S2^X7^Y7,      }, // 620
    {Y5^X8,         S0^X5^Y8,      S1^Y6^X7,      S2^X6^Y7,      }, // 621
    {Y3^X8,         S0^X5^Y7,      S1^Y5^X7,      S2^X6^Y6,      }, // 622
    {Y3^X6,         X3^Y7,         S0^X4^Y6,      S1^X5^Y5,      }, // 623
    {X6,            Y6^X10,        S0^X7^Y9,      S1^Y7^X9,      }, // 624
    {Y5,            X6^Y9,         S0^X7^Y8,      S1^Y6^X9,      }, // 625
    {Y3,            Y5^X9,         S0^X6^Y8,      S1^Y6^X8,      }, // 626
    {X3,            Y3^X9,         S0^X6^Y7,      S1^Y5^X8,      }, // 627
    {Y2,            Y3^X8,         S0^X5^Y7,      S1^Y5^X7,      }, // 628
    {S0^X6^Y9,      S1^Y7^X8,      S2^X7^Y8,      Z0^X5^Y5,      }, // 629
    {S0^X6^Y8,      S1^Y6^X8,      S2^X7^Y7,      Z0^X5^Y5,      }, // 630
    {S0^X5^Y8,      S1^Y6^X7,      S2^X6^Y7,      Z0^X5^Y5,      }, // 631
    {Y3^X7,         S0^X5^Y7,      S1^X6^Y6,      S2^X5^Y5,      }, // 632
    {X5,            X6^Y9,         S0^Y6^X9,      S1^X7^Y8,      }, // 633
    {Y3,            X5^Y9,         S0^X6^Y8,      S1^Y6^X8,      }, // 634
    {Y2,            Y3^X7,         X3^Y8,         S0^X5^Y7,      }, // 635
    {X6,            Y6,            Y7^X10,        S0^X7^Y10,     }, // 636
    {Y3,            X6,            Y6^X10,        S0^X7^Y9,      }, // 637
    {X3,            Y3,            Y6^X9,         S0^X6^Y9,      }, // 638
    {Y2,            X3,            Y3^X9,         S0^X6^Y8,      }, // 639
    {X2,            Y2,            Y3^X8,         S0^X5^Y8,      }, // 640
    {Y6,            S0^X7^Y9,      S1^Y7^X9,      S2^X8^Y8,      }, // 641
    {X6,            S0^X7^Y8,      S1^Y6^X9,      S2^Y7^X8,      }, // 642
    {Y3,            S0^X6^Y8,      S1^Y6^X8,      S2^X7^Y7,      }, // 643
    {X3^X8^Y8,      X2,            Y2,            Y3^X9,         }, // 644
    {X6,            Y7,            S0^X7^Y10,     S1^Y8^X9,      }, // 645
    {Y3,            X6,            S0^X7^Y9,      S1^Y7^X9,      }, // 646
    {X3,            Y3,            S0^X6^Y9,      S1^Y7^X8,      }, // 647
    {Y2,            X3,            Y3^X8,         S0^X6^Y8,      }, // 648
    {Z2^X6^Y6,      X6,            X7,            Y7^X11,        }, // 649
    {Z2^X6^Y6,      Y3,            X6,            X7^Y10,        }, // 650
    {Z2^X6^Y6,      X3,            Y3,            X6^Y10,        }, // 651
    {Z2^X6^Y6,      Y2,            X3,            Y3^X10,        }, // 652
    {S2^X6^Y6,      X2,            Y2,            Y3^X8,         }, // 653
    {Z2^X6^Y6,      X6^X9^Y9,      X7,            Y7,            }, // 654
    {Z2^X6^Y6,      X6^X9^Y9,      Y3,            X7,            }, // 655
    {Z2^X6^Y6,      X6^X9^Y9,      X3,            Y3,            }, // 656
    {Z2^X6^Y6,      X6^X9^Y9,      Y2,            X3,            }, // 657
    {Z2^X6^Y6,      X3^X9^Y9,      X2,            Y2,            }, // 658
    {Z3^Y6^X7,      X7,            Y7,            S0^X8^Y10,     }, // 659
    {Z3^Y6^X7,      Y3,            X7,            S0^X8^Y9,      }, // 660
    {Z3^Y6^X7,      X3,            Y3,            S0^X7^Y9,      }, // 661
    {S2^Y6^X7,      Y2,            X3,            Y3^X9,         }, // 662
    {S2^Y6^X7,      X2,            Y2,            Y3^X8,         }, // 663
    {Z3^Y6^X7,      Z2^X6^Y7,      X7,            Y7,            }, // 664
    {Z3^Y6^X7,      Z2^X6^Y7,      Y3,            X7,            }, // 665
    {Z3^Y6^X7,      Z2^X6^Y7,      X3,            Y3,            }, // 666
    {Z3^Y6^X7,      Z2^X6^Y7,      Y2,            X3,            }, // 667
    {Z2^Y6^X7,      S2^X6^Y7,      X2,            Y2,            }, // 668
    {Y6,            X7^Y8,         Y7^X8,         Z0^X5^Y6,      }, // 669
    {X6,            X7^Y7,         Y6^X8,         Z0^X5^Y6,      }, // 670
    {Y3,            X6^Y7,         Y6^X7,         Z0^X5^Y6,      }, // 671
    {X6^X8^Y8,      Y6,            X7,            Y7^X8,         }, // 672
    {X6^X8^Y8,      Y3,            Y6,            X7^Y7,         }, // 673
    {X6^X8^Y8,      X3,            Y3,            Y6^X7,         }, // 674
    {X6^X8^Y8,      Y2,            X3,            Y3^X7,         }, // 675
    {X3^X8^Y8,      X2,            Y2,            Y3^X6,         }, // 676
    {X6,            X7,            Y7^X8,         Z0^X6^Y6,      }, // 677
    {Y3,            X6,            X7^Y7,         Z0^X6^Y6,      }, // 678
    {X3,            Y3,            X6^Y7,         Z0^X6^Y6,      }, // 679
    {Z0^X6^Y6,      X6,            X7,            Y7^X8,         }, // 680
    {Z0^X6^Y6,      Y3,            X6,            X7^Y7,         }, // 681
    {Z0^X6^Y6,      X3,            Y3,            X6^Y7,         }, // 682
    {Z0^X6^Y6,      X3^X9^Y9,      Y2,            Y3,            }, // 683
    {Y2^X6^Y6,      X3^X9^Y9,      X2,            Y3,            }, // 684
    {Z1^Y6^X7,      X7,            Y7,            Z0^X6^Y7,      }, // 685
    {Z1^Y6^X7,      Y3,            X7,            Z0^X6^Y7,      }, // 686
    {Z1^Y6^X7,      X3,            Y3,            Z0^X6^Y7,      }, // 687
    {Y4^X8,         X5^Y7,         Y5^X7,         S0^X6^Y6,      }, // 688
    {Y4^X7,         X4^Y7,         Y5^X6,         S0^X5^Y6,      }, // 689
    {Y4^X7,         X3^Y7,         Y5^X6,         S0^X5^Y6,      }, // 690
    {X6,            Y6^X9,         Y7^X8,         S0^X7^Y8,      }, // 691
    {Y5,            X6^Y8,         Y6^X8,         S0^X7^Y7,      }, // 692
    {Y3,            Y5^X8,         Y6^X7,         S0^X6^Y7,      }, // 693
    {X3,            Y3^X8,         Y5^X7,         S0^X6^Y6,      }, // 694
    {Y2,            Y3^X6,         X3^Y6,         X5^Y5,         }, // 695
    {X5,            X6^Y8,         Y6^X8,         S0^X7^Y7,      }, // 696
    {Y3,            X5^Y8,         Y6^X7,         S0^X6^Y7,      }, // 697
    {X3,            Y3^X7,         X5^Y7,         S0^X6^Y6,      }, // 698
    {Y2,            Y3^X6,         X3^Y7,         S0^X5^Y6,      }, // 699
    {X6,            Y6,            Y7^X8,         S0^X7^Y8,      }, // 700
    {Y3,            X6,            Y6^X8,         S0^X7^Y7,      }, // 701
    {X3,            Y3,            Y6^X7,         S0^X6^Y7,      }, // 702
    {Y2,            X3,            Y3^X7,         S0^X6^Y6,      }, // 703
    {Y6,            Y7^X8,         S0^X7^Y8,      Z0^X5^Y6,      }, // 704
    {X6,            Y6^X8,         S0^X7^Y7,      Z0^X5^Y6,      }, // 705
    {Y3,            Y6^X7,         S0^X6^Y7,      Z0^X5^Y6,      }, // 706
    {X3,            Y3^X7,         S0^X6^Y6,      Z0^X5^Y6,      }, // 707
    {Y2,            Y3^X6,         X3^Y6,         S0^X5^Y6,      }, // 708
    {X6^X8^Y8,      Y6,            Y7,            S0^X7^Y8,      }, // 709
    {X6^X8^Y8,      Y3,            Y6,            S0^X7^Y7,      }, // 710
    {S0^X8^Y8,      X3,            Y3,            X6^Y6,         }, // 711
    {S0^X8^Y8,      Y2,            X3,            Y3^X6,         }, // 712
    {X6,            Y7,            S0^X7^Y8,      Z0^X6^Y6,      }, // 713
    {Y3,            X6,            S0^X7^Y7,      Z0^X6^Y6,      }, // 714
    {X3,            Y3,            S0^X6^Y7,      Z0^X6^Y6,      }, // 715
    {Y2,            X3,            Y3^X6,         S0^X6^Y6,      }, // 716
    {Z0^X6^Y6,      X6,            Y7,            S0^X7^Y8,      }, // 717
    {Z0^X6^Y6,      Y3,            X6,            S0^X7^Y7,      }, // 718
    {Z0^X6^Y6,      X3,            Y3,            S0^X6^Y7,      }, // 719
    {S0^X6^Y6,      Y2,            X3,            Y3^X6,         }, // 720
    {Z0^X6^Y6,      X6^X9^Y9,      Y7,            S0^X7,         }, // 721
    {Z0^X6^Y6,      X6^X9^Y9,      Y3,            S0^X7,         }, // 722
    {Z0^X6^Y6,      S0^X9^Y9,      X3,            Y3,            }, // 723
    {S0^X6^Y6,      X3^X9^Y9,      Y2,            Y3,            }, // 724
    {Z0^Y6^X7,      Y7,            S0^X7,         Z4^X6^Y7,      }, // 725
    {Z0^Y6^X7,      Y3,            S0^X7,         Z4^X6^Y7,      }, // 726
    {Z0^Y6^X7,      X3,            Y3,            S0^X6^Y7,      }, // 727
    {S0^Y6^X7,      X3,            Y3,            Y2^X6^Y7,      }, // 728
    {Z0^Y6^X7,      Z4^X6^Y7,      Y7,            S0^X7,         }, // 729
    {Z0^Y6^X7,      Z4^X6^Y7,      Y3,            S0^X7,         }, // 730
    {Z0^Y6^X7,      S0^X6^Y7,      X3,            Y3,            }, // 731
    {S0^Y6^X7,      Y2^X6^Y7,      X3,            Y3,            }, // 732
    {Y5^X9,         X6^Y8,         S0^Y6^X8,      S1^X7^Y7,      }, // 733
    {Y4^X8,         X5^Y7,         S0^Y5^X7,      S1^X6^Y6,      }, // 734
    {X4^Y7,         Y4^X7,         S0^X5^Y6,      S1^Y5^X6,      }, // 735
    {X5^Y7,         Y4^X8,         S0^X6^Y6,      S1^Y5^X7,      }, // 736
    {X3^Y7,         Y4^X7,         S0^X5^Y6,      S1^Y5^X6,      }, // 737
    {Y5,            X6^Y8,         S0^X7^Y7,      S1^Y6^X8,      }, // 738
    {Y3,            Y5^X8,         S0^X6^Y7,      S1^Y6^X7,      }, // 739
    {X3,            Y3^X8,         S0^X6^Y6,      S1^Y5^X7,      }, // 740
    {Y2,            Y3^X6,         X3^Y6,         S0^X5^Y5,      }, // 741
    {X5,            X6^Y8,         S0^Y6^X8,      S1^X7^Y7,      }, // 742
    {Y3,            X5^Y8,         S0^X6^Y7,      S1^Y6^X7,      }, // 743
    {X6,            Y6,            S0^X7^Y8,      S1^Y7^X8,      }, // 744
    {Y3,            X6,            S0^Y6^X8,      S1^X7^Y7,      }, // 745
    {X3,            Y3,            S0^X6^Y7,      S1^Y6^X7,      }, // 746
    {Y2,            X3,            Y3^X7,         S0^Y5^X6,      }, // 747
    {Y6,            S0^X7^Y8,      S1^Y7^X8,      Z0^X5^Y6,      }, // 748
    {X6,            S0^X7^Y7,      S1^Y6^X8,      Z0^X5^Y6,      }, // 749
    {Y3,            S0^X6^Y7,      S1^Y6^X7,      Z0^X5^Y6,      }, // 750
    {Y3,            X6,            S0^X7^Y7,      S1^Y6^X8,      }, // 751
    {X6^X8^Y8,      Y6,            S0^X7,         S1^Y7^X8,      }, // 752
    {X6^X8^Y8,      Y3,            S0^X7,         S1^Y6^X8,      }, // 753
    {S1^X8^Y8,      X3,            Y3,            S0^X6^Y6,      }, // 754
    {X6,            S0^X7,         S1^Y7^X8,      Z3^X6^Y6,      }, // 755
    {Y3,            S0^X7,         S1^Y6^X8,      Z3^X6^Y6,      }, // 756
    {X3,            Y3,            S0^X6^Y7,      S1^X6^Y6,      }, // 757
    {Z3^X6^Y6,      X6,            S0^X7,         S1^Y7^X8,      }, // 758
    {Z3^X6^Y6,      Y3,            S0^X7,         S1^Y6^X8,      }, // 759
    {S1^X6^Y6,      X3,            Y3,            S0^X6^Y7,      }, // 760
    {Z3^X6^Y6,      X6^X9^Y9,      S0^X7,         S1^Y7,         }, // 761
    {Z3^X6^Y6,      S1^X9^Y9,      Y3,            S0^X7,         }, // 762
    {S1^X6^Y6,      S0^X9^Y9,      X3,            Y3,            }, // 763
    {Z4^Y6^X7,      S0^X7,         S1^Y7,         Z3^X6^Y7,      }, // 764
    {S1^Y6^X7,      Y3,            S0^X7,         Z3^X6^Y7,      }, // 765
    {S1^Y6^X7,      X3,            Y3,            S0^X6^Y7,      }, // 766
    {Z4^Y6^X7,      Z3^X6^Y7,      S0^X7,         S1^Y7,         }, // 767
    {S1^Y6^X7,      Z3^X6^Y7,      Y3,            S0^X7,         }, // 768
    {S1^Y6^X7,      S0^X6^Y7,      X3,            Y3,            }, // 769
    {Y4^X8,         S0^X5^Y7,      S1^Y5^X7,      S2^X6^Y6,      }, // 770
    {Y4^X7,         S0^X4^Y7,      S1^Y5^X6,      S2^X5^Y6,      }, // 771
    {Y3^X7,         S0^X4^Y6,      S1^Y4^X6,      S2^X5^Y5,      }, // 772
    {Y6,            S0^X6^Y9,      S1^Y7^X8,      S2^X7^Y8,      }, // 773
    {Y5,            S0^X6^Y8,      S1^Y6^X8,      S2^X7^Y7,      }, // 774
    {Y3,            Y5^X7,         S0^X5^Y7,      S1^X6^Y6,      }, // 775
    {X3,            Y3^X7,         S0^X5^Y6,      S1^Y5^X6,      }, // 776
    {Y2,            Y3^X5,         X3^Y6,         S0^X4^Y5,      }, // 777
    {X5,            S0^X6^Y8,      S1^Y6^X8,      S2^X7^Y7,      }, // 778
    {Y3,            S0^X5^Y8,      S1^Y6^X7,      S2^X6^Y7,      }, // 779
    {X3,            Y3^X7,         S0^X5^Y7,      S1^X6^Y6,      }, // 780
    {Y6,            S0^X6,         S1^Y7^X8,      S2^X7^Y8,      }, // 781
    {Y3,            S0^X6,         S1^Y6^X8,      S2^X7^Y7,      }, // 782
    {X3,            Y3,            S0^X5^Y7,      S1^X6^Y6,      }, // 783
    {Y2,            X3,            Y3^X6,         S0^X5^Y6,      }, // 784
    {S0^X6,         S1^Y7^X8,      S2^X7^Y8,      Z2^X5^Y6,      }, // 785
    {S0^X6,         S1^Y6^X8,      S2^X7^Y7,      Z2^X5^Y6,      }, // 786
    {Y3,            S0^X6^Y7,      S1^Y6^X7,      S2^X5^Y6,      }, // 787
    {X3,            Y3^X7,         S0^X6^Y6,      S1^X5^Y6,      }, // 788
    {S2^X8^Y8,      Y6,            S0^X6,         S1^X7^Y7,      }, // 789
    {S2^X8^Y8,      Y3,            S0^X6,         S1^Y6^X7,      }, // 790
    {S0^X6,         S1^Y7,         S2^X7^Y8,      Z2^X6^Y6,      }, // 791
    {Y3,            S0^X6,         S1^X7^Y7,      S2^X6^Y6,      }, // 792
    {Z2^X6^Y6,      S0^X6,         S1^Y7,         S2^X7^Y8,      }, // 793
    {S2^X6^Y6,      Y3,            S0^X6,         S1^X7^Y7,      }, // 794
    {Z2^X6^Y6,      S2^X9^Y9,      S0^X6,         S1^Y7,         }, // 795
    {S2^X6^Y6,      S1^X9^Y9,      Y3,            S0^X6,         }, // 796
    {Z2^Y6^X7,      S0^X7,         S1^Y7,         S2^X6^Y7,      }, // 797
    {S2^Y6^X7,      Y3,            S0^X7,         S1^X6^Y7,      }, // 798
    {Z2^Y6^X7,      S2^X6^Y7,      S0^X7,         S1^Y7,         }, // 799
    {S2^Y6^X7,      S1^X6^Y7,      Y3,            S0^X7,         }, // 800
    {X2,            Z4,            Y4,            X3,            }, // 801
    {X2,            Z3,            Y4,            X3,            }, // 802
    {Y3,            X3,            Z4,            X5,            }, // 803
    {Y3,            X2,            Z4,            X3,            }, // 804
    {Y3,            X2,            Z3,            X3,            }, // 805
    {Y2,            X2,            Y3,            X3,            }, // 806
    {Z3,            X3,            Z4,            X5^Y5,         }, // 807
    {X2,            Z4,            X3,            Y2^X5^Y5,      }, // 808
    {X2,            Z3,            X3,            Y2^X5^Y5,      }, // 809
    {X2,            Y3,            X3,            Y1^X5^Y5,      }, // 810
    {X2,            Y3,            X3,            X1^X5^Y5,      }, // 811
    {Y3,            Z3,            X3,            Z4,            }, // 812
    {Y2,            Y3,            X3,            Z4,            }, // 813
    {Z3,            X3,            Z4,            X5^Y6,         }, // 814
    {X2,            Z4,            X3,            Z3^X5^Y6,      }, // 815
    {X2,            Z3,            X3,            Z2^X5^Y6,      }, // 816
    {X2,            Y3,            X3,            Z2^X5^Y6,      }, // 817
    {Z3^X7,         Y3,            X3,            Z4,            }, // 818
    {Z3^X7,         X2,            Z4,            X3,            }, // 819
    {Z2^X7,         X2,            Z3,            X3,            }, // 820
    {Z2^X7,         X2,            Y3,            X3,            }, // 821
    {Z3,            X3,            Z4,            Y3^X6^Y6,      }, // 822
    {X2,            Z4,            X3,            Y3^X6^Y6,      }, // 823
    {X2,            Z3,            X3,            Y3^X6^Y6,      }, // 824
    {X2,            Y3,            X3,            Y2^X6^Y6,      }, // 825
    {Y3^X6^Y6,      Z3,            X3,            Z4,            }, // 826
    {Y3^X6^Y6,      X2,            Z4,            X3,            }, // 827
    {Y3^X6^Y6,      X2,            Z3,            X3,            }, // 828
    {Y2^X6^Y6,      X2,            Y3,            X3,            }, // 829
    {Y3^X6^Y6,      Z3^X8,         X3,            Z4,            }, // 830
    {X2^X6^Y6,      Z3^X8,         Z4,            X3,            }, // 831
    {X2^X6^Y6,      Z2^X8,         Z3,            X3,            }, // 832
    {X2^X6^Y6,      Z2^X8,         Y3,            X3,            }, // 833
    {Y3^Y6^X7,      X3,            Z4,            Z3^X6^Y7,      }, // 834
    {Y3^Y6^X7,      Z4,            X3,            X2^X6^Y7,      }, // 835
    {Y3^Y6^X7,      Z3,            X3,            X2^X6^Y7,      }, // 836
    {Y2^Y6^X7,      Y3,            X3,            X2^X6^Y7,      }, // 837
    {Y3^Y6^X7,      Z3^X6^Y7,      X3,            Z4,            }, // 838
    {Y3^Y6^X7,      X2^X6^Y7,      Z4,            X3,            }, // 839
    {Y3^Y6^X7,      X2^X6^Y7,      Z3,            X3,            }, // 840
    {Y2^Y6^X7,      X2^X6^Y7,      Y3,            X3,            }, // 841
};

const UINT_64 GFX10_SW_PATTERN_NIBBLE4[][4] =
{
    {0,             0,             0,             0,             }, // 0
    {Y7^X9,         0,             0,             0,             }, // 1
    {Y7^X8,         0,             0,             0,             }, // 2
    {Y6^X8,         0,             0,             0,             }, // 3
    {Y6^X7,         0,             0,             0,             }, // 4
    {Y5^X7,         0,             0,             0,             }, // 5
    {X8^Y8,         0,             0,             0,             }, // 6
    {X7^Y7,         0,             0,             0,             }, // 7
    {X6^Y6,         0,             0,             0,             }, // 8
    {X8^Y9,         Y8^X9,         0,             0,             }, // 9
    {Y7^X9,         X8^Y8,         0,             0,             }, // 10
    {X7^Y8,         Y7^X8,         0,             0,             }, // 11
    {Y6^X8,         X7^Y7,         0,             0,             }, // 12
    {X6^Y7,         Y6^X7,         0,             0,             }, // 13
    {X5^Y6,         0,             0,             0,             }, // 14
    {Z0^X5^Y6,      0,             0,             0,             }, // 15
    {X8^Y8,         Y7^X9,         0,             0,             }, // 16
    {X7^Y7,         Y6^X8,         0,             0,             }, // 17
    {Y7^X11,        X9^Y9,         Y8^X10,        0,             }, // 18
    {Y7^X10,        X8^Y9,         Y8^X9,         0,             }, // 19
    {Y6^X10,        X8^Y8,         Y7^X9,         0,             }, // 20
    {Y6^X9,         X7^Y8,         Y7^X8,         0,             }, // 21
    {Y3^X9,         X7^Y7,         Y6^X8,         0,             }, // 22
    {Y8^X9,         X6^Y6,         0,             0,             }, // 23
    {X8^Y8,         X6^Y6,         0,             0,             }, // 24
    {Y7^X8,         X6^Y6,         0,             0,             }, // 25
    {X7^Y7,         Z0^X6^Y6,      0,             0,             }, // 26
    {X6^Y7,         Z0^X6^Y6,      0,             0,             }, // 27
    {X8^Y10,        Y8^X10,        X9^Y9,         0,             }, // 28
    {X7^Y9,         Y7^X9,         X8^Y8,         0,             }, // 29
    {X6^Y9,         X7^Y8,         Y7^X8,         0,             }, // 30
    {Y3^X8,         X6^Y8,         X7^Y7,         0,             }, // 31
    {X8^Y11,        Y8^X11,        X9^Y10,        Y9^X10,        }, // 32
    {Y7^X11,        X8^Y10,        Y8^X10,        X9^Y9,         }, // 33
    {X7^Y10,        Y7^X10,        X8^Y9,         Y8^X9,         }, // 34
    {Y3^X10,        X7^Y9,         Y7^X9,         X8^Y8,         }, // 35
    {X3^Y9,         Y3^X9,         X7^Y8,         Y7^X8,         }, // 36
    {X9^Y9,         Y8^X10,        X6^Y7,         0,             }, // 37
    {X8^Y9,         Y8^X9,         X6^Y7,         0,             }, // 38
    {X8^Y8,         Y7^X9,         X6^Y7,         0,             }, // 39
    {X7^Y8,         Y7^X8,         Z0^X6^Y7,      0,             }, // 40
    {Y3^X8,         X7^Y7,         Z0^X6^Y7,      0,             }, // 41
    {X8^Y10,        Y7^X11,        X9^Y9,         Y8^X10,        }, // 42
    {Y3^X10,        X7^Y9,         X8^Y8,         Y7^X9,         }, // 43
    {Y3^X9,         X3^Y9,         X7^Y8,         Y7^X8,         }, // 44
    {Y2^X7^Y7,      0,             0,             0,             }, // 45
    {X2^Y6^X7,      0,             0,             0,             }, // 46
    {Y1^X6^Y6,      0,             0,             0,             }, // 47
    {X7^Y9,         X8^Y8,         0,             0,             }, // 48
    {Y7^X8,         Y2^X7^Y8,      0,             0,             }, // 49
    {X6^Y8,         X2^X7^Y7,      0,             0,             }, // 50
    {X5^Y8,         Y1^X6^Y7,      0,             0,             }, // 51
    {Y6^X8,         Y2^X7^Y7,      0,             0,             }, // 52
    {Y6^X7,         Y1^X6^Y7,      0,             0,             }, // 53
    {X7^Y9,         X8^Y8,         Y7^X9,         0,             }, // 54
    {X7^Y9,         Y7^X9,         Y2^X8^Y8,      0,             }, // 55
    {X6^Y9,         X7^Y8,         X2^Y7^X8,      0,             }, // 56
    {X3^Y9,         X6^Y8,         Y1^X7^Y7,      0,             }, // 57
    {Y2^X7^Y8,      X6^Y6,         0,             0,             }, // 58
    {X2^X7^Y7,      Z0^X6^Y6,      0,             0,             }, // 59
    {Y1^X6^Y7,      Z0^X6^Y6,      0,             0,             }, // 60
    {Y3^X8,         X6^Y8,         Y1^X7^Y7,      0,             }, // 61
    {Y7^X11,        Y8^X10,        X8^Y10,        X9^Y9,         }, // 62
    {Y7^X10,        X7^Y10,        Y8^X9,         Y2^X8^Y9,      }, // 63
    {Y3^X10,        X7^Y9,         Y7^X9,         X2^X8^Y8,      }, // 64
    {Y3^X9,         X3^Y9,         Y7^X8,         Y1^X7^Y8,      }, // 65
    {Y7^X9,         Y2^X8^Y8,      X6^Y7,         0,             }, // 66
    {X7^Y8,         X2^Y7^X8,      Z4^X6^Y7,      0,             }, // 67
    {X3^Y8,         Y1^X7^Y7,      Z4^X6^Y7,      0,             }, // 68
    {Y3^X10,        X7^Y9,         Y7^X9,         Y2^X8^Y8,      }, // 69
    {Y2^Y6^X8,      0,             0,             0,             }, // 70
    {Y1^X6^Y7,      0,             0,             0,             }, // 71
    {Y1^Y5^X7,      0,             0,             0,             }, // 72
    {X7^Y8,         Y2^Y7^X8,      0,             0,             }, // 73
    {X2^X7^Y8,      Y2^Y7^X8,      0,             0,             }, // 74
    {X2^X7^Y7,      Y1^Y6^X8,      0,             0,             }, // 75
    {X1^X6^Y7,      Y1^Y6^X7,      0,             0,             }, // 76
    {Y6^X9,         Y2^Y7^X8,      0,             0,             }, // 77
    {X2^Y7^X8,      Y2^X7^Y8,      0,             0,             }, // 78
    {X2^Y6^X8,      Y1^X7^Y7,      0,             0,             }, // 79
    {X1^Y6^X7,      Y1^X6^Y7,      0,             0,             }, // 80
    {Y6^X10,        X2^X8^Y8,      Y2^Y7^X9,      0,             }, // 81
    {Y6^X9,         Y2^Y7^X8,      Y1^X7^Y8,      0,             }, // 82
    {Y3^X9,         X1^X7^Y7,      Y1^Y6^X8,      0,             }, // 83
    {Y2^Y7^X8,      X6^Y6,         0,             0,             }, // 84
    {Y1^X7^Y7,      Z3^X6^Y6,      0,             0,             }, // 85
    {X1^X6^Y8,      Y1^X6^Y6,      0,             0,             }, // 86
    {X7^Y9,         X2^Y7^X9,      Y2^X8^Y8,      0,             }, // 87
    {X6^Y9,         X2^Y7^X8,      Y1^X7^Y8,      0,             }, // 88
    {X3^Y8,         X1^Y6^X8,      Y1^X7^Y7,      0,             }, // 89
    {X7^Y10,        Y7^X10,        X8^Y9,         Y2^Y8^X9,      }, // 90
    {X7^Y10,        Y7^X10,        X2^X8^Y9,      Y2^Y8^X9,      }, // 91
    {Y3^X10,        X7^Y9,         X2^X8^Y8,      Y1^Y7^X9,      }, // 92
    {X3^Y9,         Y3^X9,         X1^X7^Y8,      Y1^Y7^X8,      }, // 93
    {X2^X8^Y8,      Y2^Y7^X9,      X6^Y7,         0,             }, // 94
    {Y2^Y7^X8,      Y1^X7^Y8,      Z3^X6^Y7,      0,             }, // 95
    {Y2^Y7^X8,      X1^X7^Y8,      Z3^X6^Y7,      0,             }, // 96
    {X7^Y10,        X8^Y9,         Y7^X10,        Y2^Y8^X9,      }, // 97
    {X7^Y10,        Y7^X10,        X2^Y8^X9,      Y2^X8^Y9,      }, // 98
    {Y3^X10,        X7^Y9,         X2^Y7^X9,      Y1^X8^Y8,      }, // 99
    {Y3^X9,         X3^Y9,         X1^Y7^X8,      Y1^X7^Y8,      }, // 100
    {X1^Y5^X6,      0,             0,             0,             }, // 101
    {Y2^Y6^X7,      0,             0,             0,             }, // 102
    {X1^Y6^X7,      0,             0,             0,             }, // 103
    {Y0^X5^Y7,      X1^X6^Y6,      0,             0,             }, // 104
    {Z1^X5^Y6,      0,             0,             0,             }, // 105
    {Y1^X5^Y6,      0,             0,             0,             }, // 106
    {X1^Y6^X8,      Y2^X7^Y7,      0,             0,             }, // 107
    {Y2^X7^Y7,      X1^Y6^X8,      0,             0,             }, // 108
    {X7^Y9,         X2^X8^Y8,      Y2^Y7^X9,      0,             }, // 109
    {Y1^X7^Y9,      X2^X8^Y8,      Y2^Y7^X9,      0,             }, // 110
    {X6^Y8,         X1^X7^Y7,      Y1^Y6^X8,      0,             }, // 111
    {X3^Y8,         Y0^X6^Y7,      X1^Y6^X7,      0,             }, // 112
    {X2^X7^Y8,      Y1^X6^Y6,      0,             0,             }, // 113
    {Y2^Y7^X8,      Y1^X6^Y6,      0,             0,             }, // 114
    {Y1^X7^Y9,      X2^Y7^X9,      Y2^X8^Y8,      0,             }, // 115
    {Y1^X7^Y8,      X1^Y6^X9,      Y2^Y7^X8,      0,             }, // 116
    {Y1^X6^Y9,      Y2^X7^Y8,      X1^Y7^X8,      0,             }, // 117
    {Y7^X10,        Y1^X7^Y10,     X2^X8^Y9,      Y2^Y8^X9,      }, // 118
    {Y3^X10,        X1^X7^Y9,      Y1^Y7^X9,      X2^X8^Y8,      }, // 119
    {Y3^X8,         X3^Y9,         Y0^X6^Y8,      X1^X7^Y7,      }, // 120
    {Y2^Y7^X9,      X2^X8^Y8,      Z2^X6^Y7,      0,             }, // 121
    {X2^X8^Y8,      Y2^Y7^X9,      Y1^X6^Y7,      0,             }, // 122
    {Y3^X10,        Y1^X7^Y9,      X1^Y7^X9,      Y2^X8^Y8,      }, // 123
    {Y3^X10,        Y1^X7^Y9,      Y2^X8^Y8,      X1^Y7^X9,      }, // 124
    {Y8^X9,         Z0^X6^Y6,      0,             0,             }, // 125
    {X8^Y8,         Z0^X6^Y6,      0,             0,             }, // 126
    {Y7^X8,         Z0^X6^Y6,      0,             0,             }, // 127
    {X9^Y9,         Y8^X10,        Z0^X6^Y7,      0,             }, // 128
    {X8^Y9,         Y8^X9,         Z0^X6^Y7,      0,             }, // 129
    {X8^Y8,         Y7^X9,         Z0^X6^Y7,      0,             }, // 130
    {S0^X8^Y8,      0,             0,             0,             }, // 131
    {S0^Y7^X8,      0,             0,             0,             }, // 132
    {S0^X7^Y7,      0,             0,             0,             }, // 133
    {S0^Y6^X7,      0,             0,             0,             }, // 134
    {S0^X6^Y6,      0,             0,             0,             }, // 135
    {Y8^X9,         S0^X8^Y9,      0,             0,             }, // 136
    {Y7^X9,         S0^X8^Y8,      0,             0,             }, // 137
    {Y7^X8,         S0^X7^Y8,      0,             0,             }, // 138
    {Y6^X8,         S0^X7^Y7,      0,             0,             }, // 139
    {Y6^X7,         S0^X6^Y7,      0,             0,             }, // 140
    {X8^Y10,        Y8^X10,        S0^X9^Y9,      0,             }, // 141
    {X8^Y9,         Y7^X10,        S0^Y8^X9,      0,             }, // 142
    {X7^Y9,         Y7^X9,         S0^X8^Y8,      0,             }, // 143
    {X7^Y8,         Y6^X9,         S0^Y7^X8,      0,             }, // 144
    {X3^Y8,         Y6^X8,         S0^X7^Y7,      0,             }, // 145
    {S0^X8^Y9,      Z0^X6^Y6,      0,             0,             }, // 146
    {S0^X8^Y8,      Z0^X6^Y6,      0,             0,             }, // 147
    {S0^X7^Y8,      Z0^X6^Y6,      0,             0,             }, // 148
    {S0^X7^Y7,      Z0^X6^Y6,      0,             0,             }, // 149
    {S0^X6^Y7,      Z0^X6^Y6,      0,             0,             }, // 150
    {Y7^X10,        X8^Y9,         S0^Y8^X9,      0,             }, // 151
    {X6^Y9,         X7^Y8,         S0^Y7^X8,      0,             }, // 152
    {Y3^X8,         X6^Y8,         S0^X7^Y7,      0,             }, // 153
    {Y8^X11,        X8^Y11,        Y9^X10,        S0^X9^Y10,     }, // 154
    {Y7^X11,        X8^Y10,        Y8^X10,        S0^X9^Y9,      }, // 155
    {Y7^X10,        X7^Y10,        Y8^X9,         S0^X8^Y9,      }, // 156
    {Y3^X10,        X7^Y9,         Y7^X9,         S0^X8^Y8,      }, // 157
    {Y3^X9,         X3^Y9,         Y7^X8,         S0^X7^Y8,      }, // 158
    {Y8^X10,        S0^X9^Y9,      Z4^X6^Y7,      0,             }, // 159
    {Y7^X10,        S0^Y8^X9,      Z4^X6^Y7,      0,             }, // 160
    {Y7^X9,         S0^X8^Y8,      Z4^X6^Y7,      0,             }, // 161
    {X7^Y8,         S0^Y7^X8,      Z4^X6^Y7,      0,             }, // 162
    {X3^Y8,         S0^X7^Y7,      Z4^X6^Y7,      0,             }, // 163
    {S1^Y7^X9,      0,             0,             0,             }, // 164
    {S1^Y7^X8,      0,             0,             0,             }, // 165
    {S1^Y6^X8,      0,             0,             0,             }, // 166
    {S1^Y6^X7,      0,             0,             0,             }, // 167
    {S1^Y5^X7,      0,             0,             0,             }, // 168
    {S1^X8^Y8,      0,             0,             0,             }, // 169
    {S1^X7^Y7,      0,             0,             0,             }, // 170
    {S0^X8^Y9,      S1^Y8^X9,      0,             0,             }, // 171
    {S0^Y7^X9,      S1^X8^Y8,      0,             0,             }, // 172
    {S0^X7^Y8,      S1^Y7^X8,      0,             0,             }, // 173
    {S0^Y6^X8,      S1^X7^Y7,      0,             0,             }, // 174
    {S0^X6^Y7,      S1^Y6^X7,      0,             0,             }, // 175
    {S0^X8^Y8,      S1^Y7^X9,      0,             0,             }, // 176
    {S0^X7^Y7,      S1^Y6^X8,      0,             0,             }, // 177
    {Y7^X11,        S0^X9^Y9,      S1^Y8^X10,     0,             }, // 178
    {Y7^X10,        S0^X8^Y9,      S1^Y8^X9,      0,             }, // 179
    {Y6^X10,        S0^X8^Y8,      S1^Y7^X9,      0,             }, // 180
    {Y6^X9,         S0^X7^Y8,      S1^Y7^X8,      0,             }, // 181
    {Y3^X9,         S0^X7^Y7,      S1^Y6^X8,      0,             }, // 182
    {S1^Y8^X9,      Z3^X6^Y6,      0,             0,             }, // 183
    {S1^X8^Y8,      Z3^X6^Y6,      0,             0,             }, // 184
    {S1^Y7^X8,      Z3^X6^Y6,      0,             0,             }, // 185
    {S1^Y6^X8,      Z3^X6^Y6,      0,             0,             }, // 186
    {S0^X6^Y7,      S1^X6^Y6,      0,             0,             }, // 187
    {X8^Y10,        S0^Y8^X10,     S1^X9^Y9,      0,             }, // 188
    {X7^Y9,         S0^Y7^X9,      S1^X8^Y8,      0,             }, // 189
    {X6^Y9,         S0^X7^Y8,      S1^Y7^X8,      0,             }, // 190
    {X3^Y8,         S0^X7^Y7,      S1^Y6^X8,      0,             }, // 191
    {X8^Y11,        Y8^X11,        S0^X9^Y10,     S1^Y9^X10,     }, // 192
    {Y7^X11,        X8^Y10,        S0^Y8^X10,     S1^X9^Y9,      }, // 193
    {X7^Y10,        Y7^X10,        S0^X8^Y9,      S1^Y8^X9,      }, // 194
    {Y3^X10,        X7^Y9,         S0^Y7^X9,      S1^X8^Y8,      }, // 195
    {X3^Y9,         Y3^X9,         S0^X7^Y8,      S1^Y7^X8,      }, // 196
    {S0^X9^Y9,      S1^Y8^X10,     Z3^X6^Y7,      0,             }, // 197
    {S0^X8^Y9,      S1^Y8^X9,      Z3^X6^Y7,      0,             }, // 198
    {S0^X8^Y8,      S1^Y7^X9,      Z3^X6^Y7,      0,             }, // 199
    {S0^X7^Y8,      S1^Y7^X8,      Z3^X6^Y7,      0,             }, // 200
    {X3^Y8,         S0^X7^Y7,      Z3^X6^Y7,      0,             }, // 201
    {X8^Y10,        Y7^X11,        S0^X9^Y9,      S1^Y8^X10,     }, // 202
    {Y3^X10,        X7^Y9,         S0^X8^Y8,      S1^Y7^X9,      }, // 203
    {Y3^X9,         X3^Y9,         S0^X7^Y8,      S1^Y7^X8,      }, // 204
    {S2^X8^Y8,      0,             0,             0,             }, // 205
    {S2^Y7^X8,      0,             0,             0,             }, // 206
    {S2^X7^Y7,      0,             0,             0,             }, // 207
    {S2^Y6^X7,      0,             0,             0,             }, // 208
    {S2^X6^Y6,      0,             0,             0,             }, // 209
    {S1^X6^Y6,      0,             0,             0,             }, // 210
    {S1^Y8^X9,      S2^X8^Y9,      0,             0,             }, // 211
    {S1^Y7^X9,      S2^X8^Y8,      0,             0,             }, // 212
    {S1^Y7^X8,      S2^X7^Y8,      0,             0,             }, // 213
    {S1^Y6^X8,      S2^X7^Y7,      0,             0,             }, // 214
    {S1^Y6^X7,      S2^X6^Y7,      0,             0,             }, // 215
    {Z2^X5^Y6,      0,             0,             0,             }, // 216
    {S1^X5^Y6,      0,             0,             0,             }, // 217
    {S0^X8^Y10,     S1^Y8^X10,     S2^X9^Y9,      0,             }, // 218
    {S0^X8^Y9,      S1^Y7^X10,     S2^Y8^X9,      0,             }, // 219
    {S0^X7^Y9,      S1^Y7^X9,      S2^X8^Y8,      0,             }, // 220
    {S0^X7^Y8,      S1^Y6^X9,      S2^Y7^X8,      0,             }, // 221
    {S0^X6^Y8,      S1^Y6^X8,      S2^X7^Y7,      0,             }, // 222
    {S2^X8^Y9,      Z2^X6^Y6,      0,             0,             }, // 223
    {S2^X8^Y8,      Z2^X6^Y6,      0,             0,             }, // 224
    {S2^X7^Y8,      Z2^X6^Y6,      0,             0,             }, // 225
    {S1^X7^Y7,      S2^X6^Y6,      0,             0,             }, // 226
    {S0^Y7^X10,     S1^X8^Y9,      S2^Y8^X9,      0,             }, // 227
    {X3^Y9,         S0^X6^Y8,      S1^X7^Y7,      0,             }, // 228
    {Y8^X11,        S0^X8^Y11,     S1^Y9^X10,     S2^X9^Y10,     }, // 229
    {Y7^X11,        S0^X8^Y10,     S1^Y8^X10,     S2^X9^Y9,      }, // 230
    {Y7^X10,        S0^X7^Y10,     S1^Y8^X9,      S2^X8^Y9,      }, // 231
    {Y3^X10,        S0^X7^Y9,      S1^Y7^X9,      S2^X8^Y8,      }, // 232
    {Y3^X9,         S0^X6^Y9,      S1^Y7^X8,      S2^X7^Y8,      }, // 233
    {S1^Y8^X10,     S2^X9^Y9,      Z2^X6^Y7,      0,             }, // 234
    {S1^Y7^X10,     S2^Y8^X9,      Z2^X6^Y7,      0,             }, // 235
    {S1^Y7^X9,      S2^X8^Y8,      Z2^X6^Y7,      0,             }, // 236
    {S0^X7^Y8,      S1^Y7^X8,      Z2^X6^Y7,      0,             }, // 237
    {X3^Y8,         S0^X7^Y7,      S1^X6^Y7,      0,             }, // 238
};

const UINT_8 GFX10_DCC_64K_R_X_PATIDX[] =
{
       0, // 1 pipes 1 bpe ua @ SW_64K_R_X 1xaa @ Navi1x
       1, // 1 pipes 2 bpe ua @ SW_64K_R_X 1xaa @ Navi1x
       2, // 1 pipes 4 bpe ua @ SW_64K_R_X 1xaa @ Navi1x
       3, // 1 pipes 8 bpe ua @ SW_64K_R_X 1xaa @ Navi1x
       4, // 1 pipes 16 bpe ua @ SW_64K_R_X 1xaa @ Navi1x
       5, // 2 pipes 1 bpe ua @ SW_64K_R_X 1xaa @ Navi1x
       6, // 2 pipes 2 bpe ua @ SW_64K_R_X 1xaa @ Navi1x
       2, // 2 pipes 4 bpe ua @ SW_64K_R_X 1xaa @ Navi1x
       3, // 2 pipes 8 bpe ua @ SW_64K_R_X 1xaa @ Navi1x
       4, // 2 pipes 16 bpe ua @ SW_64K_R_X 1xaa @ Navi1x
       7, // 4+ pipes 1 bpe ua @ SW_64K_R_X 1xaa @ Navi1x
       6, // 4+ pipes 2 bpe ua @ SW_64K_R_X 1xaa @ Navi1x
       2, // 4+ pipes 4 bpe ua @ SW_64K_R_X 1xaa @ Navi1x
       3, // 4+ pipes 8 bpe ua @ SW_64K_R_X 1xaa @ Navi1x
       4, // 4+ pipes 16 bpe ua @ SW_64K_R_X 1xaa @ Navi1x
       0, // 1 pipes 1 bpe pa @ SW_64K_R_X 1xaa @ Navi1x
       1, // 1 pipes 2 bpe pa @ SW_64K_R_X 1xaa @ Navi1x
       2, // 1 pipes 4 bpe pa @ SW_64K_R_X 1xaa @ Navi1x
       3, // 1 pipes 8 bpe pa @ SW_64K_R_X 1xaa @ Navi1x
       4, // 1 pipes 16 bpe pa @ SW_64K_R_X 1xaa @ Navi1x
       8, // 2 pipes 1 bpe pa @ SW_64K_R_X 1xaa @ Navi1x
       9, // 2 pipes 2 bpe pa @ SW_64K_R_X 1xaa @ Navi1x
      10, // 2 pipes 4 bpe pa @ SW_64K_R_X 1xaa @ Navi1x
      11, // 2 pipes 8 bpe pa @ SW_64K_R_X 1xaa @ Navi1x
      12, // 2 pipes 16 bpe pa @ SW_64K_R_X 1xaa @ Navi1x
      13, // 4 pipes 1 bpe pa @ SW_64K_R_X 1xaa @ Navi1x
      14, // 4 pipes 2 bpe pa @ SW_64K_R_X 1xaa @ Navi1x
      15, // 4 pipes 4 bpe pa @ SW_64K_R_X 1xaa @ Navi1x
      16, // 4 pipes 8 bpe pa @ SW_64K_R_X 1xaa @ Navi1x
      17, // 4 pipes 16 bpe pa @ SW_64K_R_X 1xaa @ Navi1x
      18, // 8 pipes 1 bpe pa @ SW_64K_R_X 1xaa @ Navi1x
      19, // 8 pipes 2 bpe pa @ SW_64K_R_X 1xaa @ Navi1x
      20, // 8 pipes 4 bpe pa @ SW_64K_R_X 1xaa @ Navi1x
      21, // 8 pipes 8 bpe pa @ SW_64K_R_X 1xaa @ Navi1x
      22, // 8 pipes 16 bpe pa @ SW_64K_R_X 1xaa @ Navi1x
      23, // 16 pipes 1 bpe pa @ SW_64K_R_X 1xaa @ Navi1x
      24, // 16 pipes 2 bpe pa @ SW_64K_R_X 1xaa @ Navi1x
      25, // 16 pipes 4 bpe pa @ SW_64K_R_X 1xaa @ Navi1x
      26, // 16 pipes 8 bpe pa @ SW_64K_R_X 1xaa @ Navi1x
      27, // 16 pipes 16 bpe pa @ SW_64K_R_X 1xaa @ Navi1x
      28, // 32 pipes 1 bpe pa @ SW_64K_R_X 1xaa @ Navi1x
      29, // 32 pipes 2 bpe pa @ SW_64K_R_X 1xaa @ Navi1x
      30, // 32 pipes 4 bpe pa @ SW_64K_R_X 1xaa @ Navi1x
      31, // 32 pipes 8 bpe pa @ SW_64K_R_X 1xaa @ Navi1x
      32, // 32 pipes 16 bpe pa @ SW_64K_R_X 1xaa @ Navi1x
      33, // 64 pipes 1 bpe pa @ SW_64K_R_X 1xaa @ Navi1x
      34, // 64 pipes 2 bpe pa @ SW_64K_R_X 1xaa @ Navi1x
      35, // 64 pipes 4 bpe pa @ SW_64K_R_X 1xaa @ Navi1x
      36, // 64 pipes 8 bpe pa @ SW_64K_R_X 1xaa @ Navi1x
      37, // 64 pipes 16 bpe pa @ SW_64K_R_X 1xaa @ Navi1x
};

const UINT_8 GFX10_HTILE_PATIDX[] =
{
       0, // 1xaa ua @ HTILE_64K @ Navi1x
       0, // 2xaa ua @ HTILE_64K @ Navi1x
       0, // 4xaa ua @ HTILE_64K @ Navi1x
       0, // 8xaa ua @ HTILE_64K @ Navi1x
       0, // 1 pipes 1xaa pa @ HTILE_64K @ Navi1x
       0, // 1 pipes 2xaa pa @ HTILE_64K @ Navi1x
       0, // 1 pipes 4xaa pa @ HTILE_64K @ Navi1x
       0, // 1 pipes 8xaa pa @ HTILE_64K @ Navi1x
       1, // 2 pipes 1xaa pa @ HTILE_64K @ Navi1x
       1, // 2 pipes 2xaa pa @ HTILE_64K @ Navi1x
       1, // 2 pipes 4xaa pa @ HTILE_64K @ Navi1x
       1, // 2 pipes 8xaa pa @ HTILE_64K @ Navi1x
       2, // 4 pipes 1xaa pa @ HTILE_64K @ Navi1x
       2, // 4 pipes 2xaa pa @ HTILE_64K @ Navi1x
       2, // 4 pipes 4xaa pa @ HTILE_64K @ Navi1x
       2, // 4 pipes 8xaa pa @ HTILE_64K @ Navi1x
       3, // 8 pipes 1xaa pa @ HTILE_64K @ Navi1x
       3, // 8 pipes 2xaa pa @ HTILE_64K @ Navi1x
       3, // 8 pipes 4xaa pa @ HTILE_64K @ Navi1x
       3, // 8 pipes 8xaa pa @ HTILE_64K @ Navi1x
       4, // 16 pipes 1xaa pa @ HTILE_64K @ Navi1x
       4, // 16 pipes 2xaa pa @ HTILE_64K @ Navi1x
       4, // 16 pipes 4xaa pa @ HTILE_64K @ Navi1x
       5, // 16 pipes 8xaa pa @ HTILE_64K @ Navi1x
       6, // 32 pipes 1xaa pa @ HTILE_64K @ Navi1x
       6, // 32 pipes 2xaa pa @ HTILE_64K @ Navi1x
       7, // 32 pipes 4xaa pa @ HTILE_64K @ Navi1x
       8, // 32 pipes 8xaa pa @ HTILE_64K @ Navi1x
       9, // 64 pipes 1xaa pa @ HTILE_64K @ Navi1x
      10, // 64 pipes 2xaa pa @ HTILE_64K @ Navi1x
      11, // 64 pipes 4xaa pa @ HTILE_64K @ Navi1x
      12, // 64 pipes 8xaa pa @ HTILE_64K @ Navi1x
};

const UINT_8 GFX10_CMASK_64K_PATIDX[] =
{
       0, // 1 bpe ua @ CMASK_64K @ Navi1x
       0, // 2 bpe ua @ CMASK_64K @ Navi1x
       0, // 4 bpe ua @ CMASK_64K @ Navi1x
       0, // 8 bpe ua @ CMASK_64K @ Navi1x
       0, // 1 pipes 1 bpe pa @ CMASK_64K @ Navi1x
       0, // 1 pipes 2 bpe pa @ CMASK_64K @ Navi1x
       0, // 1 pipes 4 bpe pa @ CMASK_64K @ Navi1x
       0, // 1 pipes 8 bpe pa @ CMASK_64K @ Navi1x
       1, // 2 pipes 1 bpe pa @ CMASK_64K @ Navi1x
       1, // 2 pipes 2 bpe pa @ CMASK_64K @ Navi1x
       1, // 2 pipes 4 bpe pa @ CMASK_64K @ Navi1x
       1, // 2 pipes 8 bpe pa @ CMASK_64K @ Navi1x
       2, // 4 pipes 1 bpe pa @ CMASK_64K @ Navi1x
       2, // 4 pipes 2 bpe pa @ CMASK_64K @ Navi1x
       2, // 4 pipes 4 bpe pa @ CMASK_64K @ Navi1x
       2, // 4 pipes 8 bpe pa @ CMASK_64K @ Navi1x
       3, // 8 pipes 1 bpe pa @ CMASK_64K @ Navi1x
       3, // 8 pipes 2 bpe pa @ CMASK_64K @ Navi1x
       3, // 8 pipes 4 bpe pa @ CMASK_64K @ Navi1x
       3, // 8 pipes 8 bpe pa @ CMASK_64K @ Navi1x
       4, // 16 pipes 1 bpe pa @ CMASK_64K @ Navi1x
       4, // 16 pipes 2 bpe pa @ CMASK_64K @ Navi1x
       4, // 16 pipes 4 bpe pa @ CMASK_64K @ Navi1x
       4, // 16 pipes 8 bpe pa @ CMASK_64K @ Navi1x
       5, // 32 pipes 1 bpe pa @ CMASK_64K @ Navi1x
       5, // 32 pipes 2 bpe pa @ CMASK_64K @ Navi1x
       5, // 32 pipes 4 bpe pa @ CMASK_64K @ Navi1x
       5, // 32 pipes 8 bpe pa @ CMASK_64K @ Navi1x
       6, // 64 pipes 1 bpe pa @ CMASK_64K @ Navi1x
       6, // 64 pipes 2 bpe pa @ CMASK_64K @ Navi1x
       6, // 64 pipes 4 bpe pa @ CMASK_64K @ Navi1x
       7, // 64 pipes 8 bpe pa @ CMASK_64K @ Navi1x
};

const UINT_8 GFX10_DCC_64K_R_X_RBPLUS_PATIDX[] =
{
       0, // 1 bpe ua @ SW_64K_R_X 1xaa @ RbPlus
       1, // 2 bpe ua @ SW_64K_R_X 1xaa @ RbPlus
       2, // 4 bpe ua @ SW_64K_R_X 1xaa @ RbPlus
       3, // 8 bpe ua @ SW_64K_R_X 1xaa @ RbPlus
       4, // 16 bpe ua @ SW_64K_R_X 1xaa @ RbPlus
       0, // 1 pipes (1 PKRs) 1 bpe pa @ SW_64K_R_X 1xaa @ RbPlus
       1, // 1 pipes (1 PKRs) 2 bpe pa @ SW_64K_R_X 1xaa @ RbPlus
       2, // 1 pipes (1 PKRs) 4 bpe pa @ SW_64K_R_X 1xaa @ RbPlus
       3, // 1 pipes (1 PKRs) 8 bpe pa @ SW_64K_R_X 1xaa @ RbPlus
       4, // 1 pipes (1 PKRs) 16 bpe pa @ SW_64K_R_X 1xaa @ RbPlus
      38, // 2 pipes (1-2 PKRs) 1 bpe pa @ SW_64K_R_X 1xaa @ RbPlus
      39, // 2 pipes (1-2 PKRs) 2 bpe pa @ SW_64K_R_X 1xaa @ RbPlus
      40, // 2 pipes (1-2 PKRs) 4 bpe pa @ SW_64K_R_X 1xaa @ RbPlus
      41, // 2 pipes (1-2 PKRs) 8 bpe pa @ SW_64K_R_X 1xaa @ RbPlus
      42, // 2 pipes (1-2 PKRs) 16 bpe pa @ SW_64K_R_X 1xaa @ RbPlus
      43, // 4 pipes (1-2 PKRs) 1 bpe pa @ SW_64K_R_X 1xaa @ RbPlus
      44, // 4 pipes (1-2 PKRs) 2 bpe pa @ SW_64K_R_X 1xaa @ RbPlus
      45, // 4 pipes (1-2 PKRs) 4 bpe pa @ SW_64K_R_X 1xaa @ RbPlus
      46, // 4 pipes (1-2 PKRs) 8 bpe pa @ SW_64K_R_X 1xaa @ RbPlus
      47, // 4 pipes (1-2 PKRs) 16 bpe pa @ SW_64K_R_X 1xaa @ RbPlus
      48, // 8 pipes (2 PKRs) 1 bpe pa @ SW_64K_R_X 1xaa @ RbPlus
      49, // 8 pipes (2 PKRs) 2 bpe pa @ SW_64K_R_X 1xaa @ RbPlus
      50, // 8 pipes (2 PKRs) 4 bpe pa @ SW_64K_R_X 1xaa @ RbPlus
      51, // 8 pipes (2 PKRs) 8 bpe pa @ SW_64K_R_X 1xaa @ RbPlus
      52, // 8 pipes (2 PKRs) 16 bpe pa @ SW_64K_R_X 1xaa @ RbPlus
      53, // 4 pipes (4 PKRs) 1 bpe pa @ SW_64K_R_X 1xaa @ RbPlus
      54, // 4 pipes (4 PKRs) 2 bpe pa @ SW_64K_R_X 1xaa @ RbPlus
      55, // 4 pipes (4 PKRs) 4 bpe pa @ SW_64K_R_X 1xaa @ RbPlus
      56, // 4 pipes (4 PKRs) 8 bpe pa @ SW_64K_R_X 1xaa @ RbPlus
      57, // 4 pipes (4 PKRs) 16 bpe pa @ SW_64K_R_X 1xaa @ RbPlus
      58, // 8 pipes (4 PKRs) 1 bpe pa @ SW_64K_R_X 1xaa @ RbPlus
      59, // 8 pipes (4 PKRs) 2 bpe pa @ SW_64K_R_X 1xaa @ RbPlus
      60, // 8 pipes (4 PKRs) 4 bpe pa @ SW_64K_R_X 1xaa @ RbPlus
      61, // 8 pipes (4 PKRs) 8 bpe pa @ SW_64K_R_X 1xaa @ RbPlus
      62, // 8 pipes (4 PKRs) 16 bpe pa @ SW_64K_R_X 1xaa @ RbPlus
      63, // 16 pipes (4 PKRs) 1 bpe pa @ SW_64K_R_X 1xaa @ RbPlus
      64, // 16 pipes (4 PKRs) 2 bpe pa @ SW_64K_R_X 1xaa @ RbPlus
      65, // 16 pipes (4 PKRs) 4 bpe pa @ SW_64K_R_X 1xaa @ RbPlus
      66, // 16 pipes (4 PKRs) 8 bpe pa @ SW_64K_R_X 1xaa @ RbPlus
      67, // 16 pipes (4 PKRs) 16 bpe pa @ SW_64K_R_X 1xaa @ RbPlus
      68, // 8 pipes (8 PKRs) 1 bpe pa @ SW_64K_R_X 1xaa @ RbPlus
      69, // 8 pipes (8 PKRs) 2 bpe pa @ SW_64K_R_X 1xaa @ RbPlus
      70, // 8 pipes (8 PKRs) 4 bpe pa @ SW_64K_R_X 1xaa @ RbPlus
      71, // 8 pipes (8 PKRs) 8 bpe pa @ SW_64K_R_X 1xaa @ RbPlus
      72, // 8 pipes (8 PKRs) 16 bpe pa @ SW_64K_R_X 1xaa @ RbPlus
      73, // 16 pipes (8 PKRs) 1 bpe pa @ SW_64K_R_X 1xaa @ RbPlus
      74, // 16 pipes (8 PKRs) 2 bpe pa @ SW_64K_R_X 1xaa @ RbPlus
      75, // 16 pipes (8 PKRs) 4 bpe pa @ SW_64K_R_X 1xaa @ RbPlus
      76, // 16 pipes (8 PKRs) 8 bpe pa @ SW_64K_R_X 1xaa @ RbPlus
      77, // 16 pipes (8 PKRs) 16 bpe pa @ SW_64K_R_X 1xaa @ RbPlus
      78, // 32 pipes (8 PKRs) 1 bpe pa @ SW_64K_R_X 1xaa @ RbPlus
      79, // 32 pipes (8 PKRs) 2 bpe pa @ SW_64K_R_X 1xaa @ RbPlus
      80, // 32 pipes (8 PKRs) 4 bpe pa @ SW_64K_R_X 1xaa @ RbPlus
      81, // 32 pipes (8 PKRs) 8 bpe pa @ SW_64K_R_X 1xaa @ RbPlus
      82, // 32 pipes (8 PKRs) 16 bpe pa @ SW_64K_R_X 1xaa @ RbPlus
      83, // 16 pipes (16 PKRs) 1 bpe pa @ SW_64K_R_X 1xaa @ RbPlus
      84, // 16 pipes (16 PKRs) 2 bpe pa @ SW_64K_R_X 1xaa @ RbPlus
      85, // 16 pipes (16 PKRs) 4 bpe pa @ SW_64K_R_X 1xaa @ RbPlus
      86, // 16 pipes (16 PKRs) 8 bpe pa @ SW_64K_R_X 1xaa @ RbPlus
      87, // 16 pipes (16 PKRs) 16 bpe pa @ SW_64K_R_X 1xaa @ RbPlus
      88, // 32 pipes (16 PKRs) 1 bpe pa @ SW_64K_R_X 1xaa @ RbPlus
      89, // 32 pipes (16 PKRs) 2 bpe pa @ SW_64K_R_X 1xaa @ RbPlus
      90, // 32 pipes (16 PKRs) 4 bpe pa @ SW_64K_R_X 1xaa @ RbPlus
      91, // 32 pipes (16 PKRs) 8 bpe pa @ SW_64K_R_X 1xaa @ RbPlus
      92, // 32 pipes (16 PKRs) 16 bpe pa @ SW_64K_R_X 1xaa @ RbPlus
      93, // 64 pipes (16 PKRs) 1 bpe pa @ SW_64K_R_X 1xaa @ RbPlus
      94, // 64 pipes (16 PKRs) 2 bpe pa @ SW_64K_R_X 1xaa @ RbPlus
      95, // 64 pipes (16 PKRs) 4 bpe pa @ SW_64K_R_X 1xaa @ RbPlus
      96, // 64 pipes (16 PKRs) 8 bpe pa @ SW_64K_R_X 1xaa @ RbPlus
      97, // 64 pipes (16 PKRs) 16 bpe pa @ SW_64K_R_X 1xaa @ RbPlus
      98, // 32 pipes (32 PKRs) 1 bpe pa @ SW_64K_R_X 1xaa @ RbPlus
      99, // 32 pipes (32 PKRs) 2 bpe pa @ SW_64K_R_X 1xaa @ RbPlus
     100, // 32 pipes (32 PKRs) 4 bpe pa @ SW_64K_R_X 1xaa @ RbPlus
     101, // 32 pipes (32 PKRs) 8 bpe pa @ SW_64K_R_X 1xaa @ RbPlus
     102, // 32 pipes (32 PKRs) 16 bpe pa @ SW_64K_R_X 1xaa @ RbPlus
     103, // 64 pipes (32 PKRs) 1 bpe pa @ SW_64K_R_X 1xaa @ RbPlus
     104, // 64 pipes (32 PKRs) 2 bpe pa @ SW_64K_R_X 1xaa @ RbPlus
     105, // 64 pipes (32 PKRs) 4 bpe pa @ SW_64K_R_X 1xaa @ RbPlus
     106, // 64 pipes (32 PKRs) 8 bpe pa @ SW_64K_R_X 1xaa @ RbPlus
     107, // 64 pipes (32 PKRs) 16 bpe pa @ SW_64K_R_X 1xaa @ RbPlus
};

const UINT_8 GFX10_HTILE_RBPLUS_PATIDX[] =
{
       0, // 1xaa ua @ HTILE_64K @ RbPlus
       0, // 2xaa ua @ HTILE_64K @ RbPlus
       0, // 4xaa ua @ HTILE_64K @ RbPlus
       0, // 8xaa ua @ HTILE_64K @ RbPlus
       0, // 1 pipes (1-2 PKRs) 1xaa pa @ HTILE_64K @ RbPlus
       0, // 1 pipes (1-2 PKRs) 2xaa pa @ HTILE_64K @ RbPlus
       0, // 1 pipes (1-2 PKRs) 4xaa pa @ HTILE_64K @ RbPlus
       0, // 1 pipes (1-2 PKRs) 8xaa pa @ HTILE_64K @ RbPlus
      13, // 2 pipes (1-2 PKRs) 1xaa pa @ HTILE_64K @ RbPlus
      13, // 2 pipes (1-2 PKRs) 2xaa pa @ HTILE_64K @ RbPlus
      13, // 2 pipes (1-2 PKRs) 4xaa pa @ HTILE_64K @ RbPlus
      13, // 2 pipes (1-2 PKRs) 8xaa pa @ HTILE_64K @ RbPlus
      14, // 4 pipes (1-2 PKRs) 1xaa pa @ HTILE_64K @ RbPlus
      14, // 4 pipes (1-2 PKRs) 2xaa pa @ HTILE_64K @ RbPlus
      14, // 4 pipes (1-2 PKRs) 4xaa pa @ HTILE_64K @ RbPlus
      14, // 4 pipes (1-2 PKRs) 8xaa pa @ HTILE_64K @ RbPlus
      15, // 8 pipes (1-2 PKRs) 1xaa pa @ HTILE_64K @ RbPlus
      15, // 8 pipes (1-2 PKRs) 2xaa pa @ HTILE_64K @ RbPlus
      15, // 8 pipes (1-2 PKRs) 4xaa pa @ HTILE_64K @ RbPlus
      15, // 8 pipes (1-2 PKRs) 8xaa pa @ HTILE_64K @ RbPlus
      13, // 2 pipes (4 PKRs) 1xaa pa @ HTILE_64K @ RbPlus
      13, // 2 pipes (4 PKRs) 2xaa pa @ HTILE_64K @ RbPlus
      13, // 2 pipes (4 PKRs) 4xaa pa @ HTILE_64K @ RbPlus
      13, // 2 pipes (4 PKRs) 8xaa pa @ HTILE_64K @ RbPlus
      16, // 4 pipes (4 PKRs) 1xaa pa @ HTILE_64K @ RbPlus
      16, // 4 pipes (4 PKRs) 2xaa pa @ HTILE_64K @ RbPlus
      16, // 4 pipes (4 PKRs) 4xaa pa @ HTILE_64K @ RbPlus
      16, // 4 pipes (4 PKRs) 8xaa pa @ HTILE_64K @ RbPlus
      17, // 8 pipes (4 PKRs) 1xaa pa @ HTILE_64K @ RbPlus
      17, // 8 pipes (4 PKRs) 2xaa pa @ HTILE_64K @ RbPlus
      17, // 8 pipes (4 PKRs) 4xaa pa @ HTILE_64K @ RbPlus
      17, // 8 pipes (4 PKRs) 8xaa pa @ HTILE_64K @ RbPlus
      18, // 16 pipes (4 PKRs) 1xaa pa @ HTILE_64K @ RbPlus
      18, // 16 pipes (4 PKRs) 2xaa pa @ HTILE_64K @ RbPlus
      18, // 16 pipes (4 PKRs) 4xaa pa @ HTILE_64K @ RbPlus
      18, // 16 pipes (4 PKRs) 8xaa pa @ HTILE_64K @ RbPlus
      19, // 4 pipes (8 PKRs) 1xaa pa @ HTILE_64K @ RbPlus
      19, // 4 pipes (8 PKRs) 2xaa pa @ HTILE_64K @ RbPlus
      19, // 4 pipes (8 PKRs) 4xaa pa @ HTILE_64K @ RbPlus
      19, // 4 pipes (8 PKRs) 8xaa pa @ HTILE_64K @ RbPlus
      20, // 8 pipes (8 PKRs) 1xaa pa @ HTILE_64K @ RbPlus
      20, // 8 pipes (8 PKRs) 2xaa pa @ HTILE_64K @ RbPlus
      20, // 8 pipes (8 PKRs) 4xaa pa @ HTILE_64K @ RbPlus
      20, // 8 pipes (8 PKRs) 8xaa pa @ HTILE_64K @ RbPlus
      21, // 16 pipes (8 PKRs) 1xaa pa @ HTILE_64K @ RbPlus
      21, // 16 pipes (8 PKRs) 2xaa pa @ HTILE_64K @ RbPlus
      21, // 16 pipes (8 PKRs) 4xaa pa @ HTILE_64K @ RbPlus
      21, // 16 pipes (8 PKRs) 8xaa pa @ HTILE_64K @ RbPlus
      22, // 32 pipes (8 PKRs) 1xaa pa @ HTILE_64K @ RbPlus
      22, // 32 pipes (8 PKRs) 2xaa pa @ HTILE_64K @ RbPlus
      22, // 32 pipes (8 PKRs) 4xaa pa @ HTILE_64K @ RbPlus
      22, // 32 pipes (8 PKRs) 8xaa pa @ HTILE_64K @ RbPlus
      23, // 8 pipes (16 PKRs) 1xaa pa @ HTILE_64K @ RbPlus
      23, // 8 pipes (16 PKRs) 2xaa pa @ HTILE_64K @ RbPlus
      23, // 8 pipes (16 PKRs) 4xaa pa @ HTILE_64K @ RbPlus
      23, // 8 pipes (16 PKRs) 8xaa pa @ HTILE_64K @ RbPlus
      24, // 16 pipes (16 PKRs) 1xaa pa @ HTILE_64K @ RbPlus
      24, // 16 pipes (16 PKRs) 2xaa pa @ HTILE_64K @ RbPlus
      24, // 16 pipes (16 PKRs) 4xaa pa @ HTILE_64K @ RbPlus
      24, // 16 pipes (16 PKRs) 8xaa pa @ HTILE_64K @ RbPlus
      25, // 32 pipes (16 PKRs) 1xaa pa @ HTILE_64K @ RbPlus
      25, // 32 pipes (16 PKRs) 2xaa pa @ HTILE_64K @ RbPlus
      25, // 32 pipes (16 PKRs) 4xaa pa @ HTILE_64K @ RbPlus
      25, // 32 pipes (16 PKRs) 8xaa pa @ HTILE_64K @ RbPlus
      26, // 64 pipes (16 PKRs) 1xaa pa @ HTILE_64K @ RbPlus
      26, // 64 pipes (16 PKRs) 2xaa pa @ HTILE_64K @ RbPlus
      26, // 64 pipes (16 PKRs) 4xaa pa @ HTILE_64K @ RbPlus
      26, // 64 pipes (16 PKRs) 8xaa pa @ HTILE_64K @ RbPlus
      27, // 16 pipes (32 PKRs) 1xaa pa @ HTILE_64K @ RbPlus
      27, // 16 pipes (32 PKRs) 2xaa pa @ HTILE_64K @ RbPlus
      27, // 16 pipes (32 PKRs) 4xaa pa @ HTILE_64K @ RbPlus
      27, // 16 pipes (32 PKRs) 8xaa pa @ HTILE_64K @ RbPlus
      28, // 32 pipes (32 PKRs) 1xaa pa @ HTILE_64K @ RbPlus
      28, // 32 pipes (32 PKRs) 2xaa pa @ HTILE_64K @ RbPlus
      28, // 32 pipes (32 PKRs) 4xaa pa @ HTILE_64K @ RbPlus
      28, // 32 pipes (32 PKRs) 8xaa pa @ HTILE_64K @ RbPlus
      29, // 64 pipes (32 PKRs) 1xaa pa @ HTILE_64K @ RbPlus
      29, // 64 pipes (32 PKRs) 2xaa pa @ HTILE_64K @ RbPlus
      29, // 64 pipes (32 PKRs) 4xaa pa @ HTILE_64K @ RbPlus
      29, // 64 pipes (32 PKRs) 8xaa pa @ HTILE_64K @ RbPlus
};

const UINT_8 GFX10_CMASK_64K_RBPLUS_PATIDX[] =
{
       0, // 1 bpe ua @ CMASK_64K @ RbPlus
       0, // 2 bpe ua @ CMASK_64K @ RbPlus
       0, // 4 bpe ua @ CMASK_64K @ RbPlus
       0, // 8 bpe ua @ CMASK_64K @ RbPlus
       0, // 1 pipes (1-2 PKRs) 1 bpe pa @ CMASK_64K @ RbPlus
       0, // 1 pipes (1-2 PKRs) 2 bpe pa @ CMASK_64K @ RbPlus
       0, // 1 pipes (1-2 PKRs) 4 bpe pa @ CMASK_64K @ RbPlus
       0, // 1 pipes (1-2 PKRs) 8 bpe pa @ CMASK_64K @ RbPlus
       8, // 2 pipes (1-2 PKRs) 1 bpe pa @ CMASK_64K @ RbPlus
       8, // 2 pipes (1-2 PKRs) 2 bpe pa @ CMASK_64K @ RbPlus
       8, // 2 pipes (1-2 PKRs) 4 bpe pa @ CMASK_64K @ RbPlus
       8, // 2 pipes (1-2 PKRs) 8 bpe pa @ CMASK_64K @ RbPlus
       9, // 4 pipes (1-2 PKRs) 1 bpe pa @ CMASK_64K @ RbPlus
       9, // 4 pipes (1-2 PKRs) 2 bpe pa @ CMASK_64K @ RbPlus
       9, // 4 pipes (1-2 PKRs) 4 bpe pa @ CMASK_64K @ RbPlus
       9, // 4 pipes (1-2 PKRs) 8 bpe pa @ CMASK_64K @ RbPlus
      10, // 8 pipes (1-2 PKRs) 1 bpe pa @ CMASK_64K @ RbPlus
      10, // 8 pipes (1-2 PKRs) 2 bpe pa @ CMASK_64K @ RbPlus
      10, // 8 pipes (1-2 PKRs) 4 bpe pa @ CMASK_64K @ RbPlus
      10, // 8 pipes (1-2 PKRs) 8 bpe pa @ CMASK_64K @ RbPlus
       8, // 2 pipes (4 PKRs) 1 bpe pa @ CMASK_64K @ RbPlus
       8, // 2 pipes (4 PKRs) 2 bpe pa @ CMASK_64K @ RbPlus
       8, // 2 pipes (4 PKRs) 4 bpe pa @ CMASK_64K @ RbPlus
       8, // 2 pipes (4 PKRs) 8 bpe pa @ CMASK_64K @ RbPlus
      11, // 4 pipes (4 PKRs) 1 bpe pa @ CMASK_64K @ RbPlus
      11, // 4 pipes (4 PKRs) 2 bpe pa @ CMASK_64K @ RbPlus
      11, // 4 pipes (4 PKRs) 4 bpe pa @ CMASK_64K @ RbPlus
      11, // 4 pipes (4 PKRs) 8 bpe pa @ CMASK_64K @ RbPlus
      12, // 8 pipes (4 PKRs) 1 bpe pa @ CMASK_64K @ RbPlus
      12, // 8 pipes (4 PKRs) 2 bpe pa @ CMASK_64K @ RbPlus
      12, // 8 pipes (4 PKRs) 4 bpe pa @ CMASK_64K @ RbPlus
      12, // 8 pipes (4 PKRs) 8 bpe pa @ CMASK_64K @ RbPlus
      13, // 16 pipes (4 PKRs) 1 bpe pa @ CMASK_64K @ RbPlus
      13, // 16 pipes (4 PKRs) 2 bpe pa @ CMASK_64K @ RbPlus
      13, // 16 pipes (4 PKRs) 4 bpe pa @ CMASK_64K @ RbPlus
      13, // 16 pipes (4 PKRs) 8 bpe pa @ CMASK_64K @ RbPlus
      14, // 4 pipes (8 PKRs) 1 bpe pa @ CMASK_64K @ RbPlus
      14, // 4 pipes (8 PKRs) 2 bpe pa @ CMASK_64K @ RbPlus
      14, // 4 pipes (8 PKRs) 4 bpe pa @ CMASK_64K @ RbPlus
      14, // 4 pipes (8 PKRs) 8 bpe pa @ CMASK_64K @ RbPlus
      15, // 8 pipes (8 PKRs) 1 bpe pa @ CMASK_64K @ RbPlus
      15, // 8 pipes (8 PKRs) 2 bpe pa @ CMASK_64K @ RbPlus
      15, // 8 pipes (8 PKRs) 4 bpe pa @ CMASK_64K @ RbPlus
      16, // 8 pipes (8 PKRs) 8 bpe pa @ CMASK_64K @ RbPlus
      15, // 16 pipes (8 PKRs) 1 bpe pa @ CMASK_64K @ RbPlus
      15, // 16 pipes (8 PKRs) 2 bpe pa @ CMASK_64K @ RbPlus
      15, // 16 pipes (8 PKRs) 4 bpe pa @ CMASK_64K @ RbPlus
      17, // 16 pipes (8 PKRs) 8 bpe pa @ CMASK_64K @ RbPlus
      18, // 32 pipes (8 PKRs) 1 bpe pa @ CMASK_64K @ RbPlus
      18, // 32 pipes (8 PKRs) 2 bpe pa @ CMASK_64K @ RbPlus
      18, // 32 pipes (8 PKRs) 4 bpe pa @ CMASK_64K @ RbPlus
      19, // 32 pipes (8 PKRs) 8 bpe pa @ CMASK_64K @ RbPlus
      20, // 8 pipes (16 PKRs) 1 bpe pa @ CMASK_64K @ RbPlus
      20, // 8 pipes (16 PKRs) 2 bpe pa @ CMASK_64K @ RbPlus
      20, // 8 pipes (16 PKRs) 4 bpe pa @ CMASK_64K @ RbPlus
      21, // 8 pipes (16 PKRs) 8 bpe pa @ CMASK_64K @ RbPlus
      22, // 16 pipes (16 PKRs) 1 bpe pa @ CMASK_64K @ RbPlus
      22, // 16 pipes (16 PKRs) 2 bpe pa @ CMASK_64K @ RbPlus
      22, // 16 pipes (16 PKRs) 4 bpe pa @ CMASK_64K @ RbPlus
      23, // 16 pipes (16 PKRs) 8 bpe pa @ CMASK_64K @ RbPlus
      22, // 32 pipes (16 PKRs) 1 bpe pa @ CMASK_64K @ RbPlus
      22, // 32 pipes (16 PKRs) 2 bpe pa @ CMASK_64K @ RbPlus
      22, // 32 pipes (16 PKRs) 4 bpe pa @ CMASK_64K @ RbPlus
      24, // 32 pipes (16 PKRs) 8 bpe pa @ CMASK_64K @ RbPlus
      25, // 64 pipes (16 PKRs) 1 bpe pa @ CMASK_64K @ RbPlus
      25, // 64 pipes (16 PKRs) 2 bpe pa @ CMASK_64K @ RbPlus
      25, // 64 pipes (16 PKRs) 4 bpe pa @ CMASK_64K @ RbPlus
      32, // 64 pipes (16 PKRs) 8 bpe pa @ CMASK_64K @ RbPlus
      27, // 16 pipes (32 PKRs) 1 bpe pa @ CMASK_64K @ RbPlus
      27, // 16 pipes (32 PKRs) 2 bpe pa @ CMASK_64K @ RbPlus
      27, // 16 pipes (32 PKRs) 4 bpe pa @ CMASK_64K @ RbPlus
      28, // 16 pipes (32 PKRs) 8 bpe pa @ CMASK_64K @ RbPlus
      29, // 32 pipes (32 PKRs) 1 bpe pa @ CMASK_64K @ RbPlus
      29, // 32 pipes (32 PKRs) 2 bpe pa @ CMASK_64K @ RbPlus
      29, // 32 pipes (32 PKRs) 4 bpe pa @ CMASK_64K @ RbPlus
      33, // 32 pipes (32 PKRs) 8 bpe pa @ CMASK_64K @ RbPlus
      29, // 64 pipes (32 PKRs) 1 bpe pa @ CMASK_64K @ RbPlus
      29, // 64 pipes (32 PKRs) 2 bpe pa @ CMASK_64K @ RbPlus
      29, // 64 pipes (32 PKRs) 4 bpe pa @ CMASK_64K @ RbPlus
      34, // 64 pipes (32 PKRs) 8 bpe pa @ CMASK_64K @ RbPlus
};

const UINT_8 GFX10_CMASK_VAR_RBPLUS_PATIDX[] =
{
       0, // 1 bpe ua @ CMASK_VAR @ RbPlus
       0, // 2 bpe ua @ CMASK_VAR @ RbPlus
       0, // 4 bpe ua @ CMASK_VAR @ RbPlus
       0, // 8 bpe ua @ CMASK_VAR @ RbPlus
       0, // 1 pipes (1-2 PKRs) 1 bpe pa @ CMASK_VAR @ RbPlus
       0, // 1 pipes (1-2 PKRs) 2 bpe pa @ CMASK_VAR @ RbPlus
       0, // 1 pipes (1-2 PKRs) 4 bpe pa @ CMASK_VAR @ RbPlus
       0, // 1 pipes (1-2 PKRs) 8 bpe pa @ CMASK_VAR @ RbPlus
       8, // 2 pipes (1-2 PKRs) 1 bpe pa @ CMASK_VAR @ RbPlus
       8, // 2 pipes (1-2 PKRs) 2 bpe pa @ CMASK_VAR @ RbPlus
       8, // 2 pipes (1-2 PKRs) 4 bpe pa @ CMASK_VAR @ RbPlus
       8, // 2 pipes (1-2 PKRs) 8 bpe pa @ CMASK_VAR @ RbPlus
       9, // 4 pipes (1-2 PKRs) 1 bpe pa @ CMASK_VAR @ RbPlus
       9, // 4 pipes (1-2 PKRs) 2 bpe pa @ CMASK_VAR @ RbPlus
       9, // 4 pipes (1-2 PKRs) 4 bpe pa @ CMASK_VAR @ RbPlus
       9, // 4 pipes (1-2 PKRs) 8 bpe pa @ CMASK_VAR @ RbPlus
      10, // 8 pipes (1-2 PKRs) 1 bpe pa @ CMASK_VAR @ RbPlus
      10, // 8 pipes (1-2 PKRs) 2 bpe pa @ CMASK_VAR @ RbPlus
      10, // 8 pipes (1-2 PKRs) 4 bpe pa @ CMASK_VAR @ RbPlus
      10, // 8 pipes (1-2 PKRs) 8 bpe pa @ CMASK_VAR @ RbPlus
       8, // 2 pipes (4 PKRs) 1 bpe pa @ CMASK_VAR @ RbPlus
       8, // 2 pipes (4 PKRs) 2 bpe pa @ CMASK_VAR @ RbPlus
       8, // 2 pipes (4 PKRs) 4 bpe pa @ CMASK_VAR @ RbPlus
       8, // 2 pipes (4 PKRs) 8 bpe pa @ CMASK_VAR @ RbPlus
      11, // 4 pipes (4 PKRs) 1 bpe pa @ CMASK_VAR @ RbPlus
      11, // 4 pipes (4 PKRs) 2 bpe pa @ CMASK_VAR @ RbPlus
      11, // 4 pipes (4 PKRs) 4 bpe pa @ CMASK_VAR @ RbPlus
      11, // 4 pipes (4 PKRs) 8 bpe pa @ CMASK_VAR @ RbPlus
      12, // 8 pipes (4 PKRs) 1 bpe pa @ CMASK_VAR @ RbPlus
      12, // 8 pipes (4 PKRs) 2 bpe pa @ CMASK_VAR @ RbPlus
      12, // 8 pipes (4 PKRs) 4 bpe pa @ CMASK_VAR @ RbPlus
      12, // 8 pipes (4 PKRs) 8 bpe pa @ CMASK_VAR @ RbPlus
      13, // 16 pipes (4 PKRs) 1 bpe pa @ CMASK_VAR @ RbPlus
      13, // 16 pipes (4 PKRs) 2 bpe pa @ CMASK_VAR @ RbPlus
      13, // 16 pipes (4 PKRs) 4 bpe pa @ CMASK_VAR @ RbPlus
      13, // 16 pipes (4 PKRs) 8 bpe pa @ CMASK_VAR @ RbPlus
      14, // 4 pipes (8 PKRs) 1 bpe pa @ CMASK_VAR @ RbPlus
      14, // 4 pipes (8 PKRs) 2 bpe pa @ CMASK_VAR @ RbPlus
      14, // 4 pipes (8 PKRs) 4 bpe pa @ CMASK_VAR @ RbPlus
      14, // 4 pipes (8 PKRs) 8 bpe pa @ CMASK_VAR @ RbPlus
      15, // 8 pipes (8 PKRs) 1 bpe pa @ CMASK_VAR @ RbPlus
      15, // 8 pipes (8 PKRs) 2 bpe pa @ CMASK_VAR @ RbPlus
      15, // 8 pipes (8 PKRs) 4 bpe pa @ CMASK_VAR @ RbPlus
      16, // 8 pipes (8 PKRs) 8 bpe pa @ CMASK_VAR @ RbPlus
      15, // 16 pipes (8 PKRs) 1 bpe pa @ CMASK_VAR @ RbPlus
      15, // 16 pipes (8 PKRs) 2 bpe pa @ CMASK_VAR @ RbPlus
      15, // 16 pipes (8 PKRs) 4 bpe pa @ CMASK_VAR @ RbPlus
      17, // 16 pipes (8 PKRs) 8 bpe pa @ CMASK_VAR @ RbPlus
      18, // 32 pipes (8 PKRs) 1 bpe pa @ CMASK_VAR @ RbPlus
      18, // 32 pipes (8 PKRs) 2 bpe pa @ CMASK_VAR @ RbPlus
      18, // 32 pipes (8 PKRs) 4 bpe pa @ CMASK_VAR @ RbPlus
      19, // 32 pipes (8 PKRs) 8 bpe pa @ CMASK_VAR @ RbPlus
      20, // 8 pipes (16 PKRs) 1 bpe pa @ CMASK_VAR @ RbPlus
      20, // 8 pipes (16 PKRs) 2 bpe pa @ CMASK_VAR @ RbPlus
      20, // 8 pipes (16 PKRs) 4 bpe pa @ CMASK_VAR @ RbPlus
      21, // 8 pipes (16 PKRs) 8 bpe pa @ CMASK_VAR @ RbPlus
      22, // 16 pipes (16 PKRs) 1 bpe pa @ CMASK_VAR @ RbPlus
      22, // 16 pipes (16 PKRs) 2 bpe pa @ CMASK_VAR @ RbPlus
      22, // 16 pipes (16 PKRs) 4 bpe pa @ CMASK_VAR @ RbPlus
      23, // 16 pipes (16 PKRs) 8 bpe pa @ CMASK_VAR @ RbPlus
      22, // 32 pipes (16 PKRs) 1 bpe pa @ CMASK_VAR @ RbPlus
      22, // 32 pipes (16 PKRs) 2 bpe pa @ CMASK_VAR @ RbPlus
      22, // 32 pipes (16 PKRs) 4 bpe pa @ CMASK_VAR @ RbPlus
      24, // 32 pipes (16 PKRs) 8 bpe pa @ CMASK_VAR @ RbPlus
      25, // 64 pipes (16 PKRs) 1 bpe pa @ CMASK_VAR @ RbPlus
      25, // 64 pipes (16 PKRs) 2 bpe pa @ CMASK_VAR @ RbPlus
      25, // 64 pipes (16 PKRs) 4 bpe pa @ CMASK_VAR @ RbPlus
      26, // 64 pipes (16 PKRs) 8 bpe pa @ CMASK_VAR @ RbPlus
      27, // 16 pipes (32 PKRs) 1 bpe pa @ CMASK_VAR @ RbPlus
      27, // 16 pipes (32 PKRs) 2 bpe pa @ CMASK_VAR @ RbPlus
      27, // 16 pipes (32 PKRs) 4 bpe pa @ CMASK_VAR @ RbPlus
      28, // 16 pipes (32 PKRs) 8 bpe pa @ CMASK_VAR @ RbPlus
      29, // 32 pipes (32 PKRs) 1 bpe pa @ CMASK_VAR @ RbPlus
      29, // 32 pipes (32 PKRs) 2 bpe pa @ CMASK_VAR @ RbPlus
      29, // 32 pipes (32 PKRs) 4 bpe pa @ CMASK_VAR @ RbPlus
      30, // 32 pipes (32 PKRs) 8 bpe pa @ CMASK_VAR @ RbPlus
      29, // 64 pipes (32 PKRs) 1 bpe pa @ CMASK_VAR @ RbPlus
      29, // 64 pipes (32 PKRs) 2 bpe pa @ CMASK_VAR @ RbPlus
      29, // 64 pipes (32 PKRs) 4 bpe pa @ CMASK_VAR @ RbPlus
      31, // 64 pipes (32 PKRs) 8 bpe pa @ CMASK_VAR @ RbPlus
};

const UINT_64 GFX10_DCC_64K_R_X_SW_PATTERN[][17] =
{
    {0,             X4,            Y4,            X5,            Y5,            X6,            Y6,            X7,            Y7,            X8,            Y8,            X9,            Y9,            0,             0,             0,             0,             }, //0
    {0,             Y3,            X4,            Y4,            X5,            Y5,            X6,            Y6,            X7,            Y7,            X8,            Y8,            X9,            0,             0,             0,             0,             }, //1
    {0,             X3,            Y3,            X4,            Y4,            X5,            Y5,            X6,            Y6,            X7,            Y7,            X8,            Y8,            0,             0,             0,             0,             }, //2
    {0,             Y2,            X3,            Y3,            X4,            Y4,            X5,            Y5,            X6,            Y6,            X7,            Y7,            X8,            0,             0,             0,             0,             }, //3
    {0,             X2,            Y2,            X3,            Y3,            X4,            Y4,            X5,            Y5,            X6,            Y6,            X7,            Y7,            0,             0,             0,             0,             }, //4
    {0,             X3^Y3,         X4,            X5,            Y5,            X6,            Y6,            X7,            Y7,            X8,            Y8,            X9,            Y9,            0,             0,             0,             0,             }, //5
    {0,             X3^Y3,         X4,            Y4,            X5,            Y5,            X6,            Y6,            X7,            Y7,            X8,            Y8,            X9,            0,             0,             0,             0,             }, //6
    {0,             X3^Y3,         X4^Y4,         X5,            Y5,            X6,            Y6,            X7,            Y7,            X8,            Y8,            X9,            Y9,            0,             0,             0,             0,             }, //7
    {0,             X4,            X5,            Y5,            X6,            Y6,            X7,            Y7,            X8,            Z0^X3^Y3,      Y8,            X9,            Y9,            0,             0,             0,             0,             }, //8
    {0,             Y4,            X4,            X5,            Y5,            X6,            Y6,            X7,            Y7,            Z0^X3^Y3,      X8,            Y8,            X9,            0,             0,             0,             0,             }, //9
    {0,             X3,            Y4,            X4,            X5,            Y5,            X6,            Y6,            X7,            Z0^X3^Y3,      Y7,            X8,            Y8,            0,             0,             0,             0,             }, //10
    {0,             Y2,            X3,            Y4,            X4,            X5,            Y5,            X6,            Y6,            Z0^X3^Y3,      X7,            Y7,            X8,            0,             0,             0,             0,             }, //11
    {0,             X2,            Y2,            X3,            Y4,            X4,            X5,            Y5,            X6,            Z0^X3^Y3,      Y6,            X7,            Y7,            0,             0,             0,             0,             }, //12
    {0,             X5,            Y5,            X6,            Y6,            X7,            Y7,            X8,            Y8,            Z1^X3^Y3,      Z0^X4^Y4,      X9,            Y9,            0,             0,             0,             0,             }, //13
    {0,             Y4,            X5,            Y5,            X6,            Y6,            X7,            Y7,            X8,            Z1^X3^Y3,      Z0^X4^Y4,      Y8,            X9,            0,             0,             0,             0,             }, //14
    {0,             X3,            Y4,            X5,            Y5,            X6,            Y6,            X7,            Y7,            Z1^X3^Y3,      Z0^X4^Y4,      X8,            Y8,            0,             0,             0,             0,             }, //15
    {0,             Y2,            X3,            Y4,            X5,            Y5,            X6,            Y6,            X7,            Z1^X3^Y3,      Z0^X4^Y4,      Y7,            X8,            0,             0,             0,             0,             }, //16
    {0,             X2,            Y2,            X3,            Y4,            X5,            Y5,            X6,            Y6,            Z1^X3^Y3,      Z0^X4^Y4,      X7,            Y7,            0,             0,             0,             0,             }, //17
    {0,             Y5,            X6,            Y6,            X7,            Y7,            X8,            Y8,            X9,            Z2^X3^Y3,      Z1^X4^Y4,      Z0^X5^Y5,      Y9,            0,             0,             0,             0,             }, //18
    {0,             Y4,            Y5,            X6,            Y6,            X7,            Y7,            X8,            Y8,            Z2^X3^Y3,      Z1^X4^Y4,      Z0^X5^Y5,      X9,            0,             0,             0,             0,             }, //19
    {0,             X3,            Y4,            Y5,            X6,            Y6,            X7,            Y7,            X8,            Z2^X3^Y3,      Z1^X4^Y4,      Z0^X5^Y5,      Y8,            0,             0,             0,             0,             }, //20
    {0,             Y2,            X3,            Y4,            Y5,            X6,            Y6,            X7,            Y7,            Z2^X3^Y3,      Z1^X4^Y4,      Z0^X5^Y5,      X8,            0,             0,             0,             0,             }, //21
    {0,             X2,            Y2,            X3,            Y4,            Y5,            X6,            Y6,            X7,            Z2^X3^Y3,      Z1^X4^Y4,      Z0^X5^Y5,      Y7,            0,             0,             0,             0,             }, //22
    {0,             X6,            Y6,            X7,            Y7,            X8,            Y8,            X9,            Y9,            X3^Y3^Z3,      Z2^X4^Y4,      Z1^Y5^X6,      Z0^X5^Y6,      0,             0,             0,             0,             }, //23
    {0,             Y4,            X6,            Y6,            X7,            Y7,            X8,            Y8,            X9,            X3^Y3^Z3,      Z2^X4^Y4,      Z1^Y5^X6,      Z0^X5^Y6,      0,             0,             0,             0,             }, //24
    {0,             X3,            Y4,            X6,            Y6,            X7,            Y7,            X8,            Y8,            X3^Y3^Z3,      Z2^X4^Y4,      Z1^Y5^X6,      Z0^X5^Y6,      0,             0,             0,             0,             }, //25
    {0,             Y2,            X3,            Y4,            X6,            Y6,            X7,            Y7,            X8,            X3^Y3^Z3,      Z2^X4^Y4,      Z1^Y5^X6,      Z0^X5^Y6,      0,             0,             0,             0,             }, //26
    {0,             X2,            Y2,            X3,            Y4,            X6,            Y6,            X7,            Y7,            X3^Y3^Z3,      Z2^X4^Y4,      Z1^Y5^X6,      Z0^X5^Y6,      0,             0,             0,             0,             }, //27
    {0,             Y6,            X7,            Y7,            X8,            Y8,            X9,            Y9,            X10,           X3^Y3^Z4,      Z3^X4^Y4,      Z2^Y5^X7,      Z1^X5^Y7,      Z0^X6^Y6,      0,             0,             0,             }, //28
    {0,             Y4,            Y6,            X7,            Y7,            X8,            Y8,            X9,            Y9,            X3^Y3^Z4,      Z3^X4^Y4,      Z2^Y5^X7,      Z1^X5^Y7,      Z0^X6^Y6,      0,             0,             0,             }, //29
    {0,             X3,            Y4,            Y6,            X7,            Y7,            X8,            Y8,            X9,            X3^Y3^Z4,      Z3^X4^Y4,      Z2^Y5^X7,      Z1^X5^Y7,      Z0^X6^Y6,      0,             0,             0,             }, //30
    {0,             Y2,            X3,            Y4,            Y6,            X7,            Y7,            X8,            Y8,            X3^Y3^Z4,      Z3^X4^Y4,      Z2^Y5^X7,      Z1^X5^Y7,      Z0^X6^Y6,      0,             0,             0,             }, //31
    {0,             X2,            X3,            Y4,            Y6,            X7,            Y7,            Y2,            X8,            X3^Y3^Z3,      Z2^X4^Y4,      Z1^Y5^X7,      Z0^X5^Y7,      Y2^X6^Y6,      0,             0,             0,             }, //32
    {0,             X7,            Y7,            X8,            Y8,            X9,            Y9,            X10,           Y10,           X3^Y3^Z5,      X4^Y4^Z4,      Z3^Y5^X8,      Z2^X5^Y8,      Z1^Y6^X7,      Z0^X6^Y7,      0,             0,             }, //33
    {0,             Y4,            X7,            Y7,            X8,            Y8,            X9,            Y9,            X10,           X3^Y3^Z5,      X4^Y4^Z4,      Z3^Y5^X8,      Z2^X5^Y8,      Z1^Y6^X7,      Z0^X6^Y7,      0,             0,             }, //34
    {0,             X3,            Y4,            X7,            Y7,            X8,            Y8,            X9,            Y9,            X3^Y3^Z5,      X4^Y4^Z4,      Z3^Y5^X8,      Z2^X5^Y8,      Z1^Y6^X7,      Z0^X6^Y7,      0,             0,             }, //35
    {0,             X3,            Y4,            X7,            Y7,            X8,            Y8,            Y2,            X9,            X3^Y3^Z4,      Z3^X4^Y4,      Z2^Y5^X8,      Z1^X5^Y8,      Y2^Y6^X7,      Z0^X6^Y7,      0,             0,             }, //36
    {0,             X3,            Y4,            X7,            Y7,            X8,            Y8,            X2,            Y2,            X3^Y3^Z3,      Z2^X4^Y4,      Z1^Y5^X8,      Z0^X5^Y8,      Y2^Y6^X7,      X2^X6^Y7,      0,             0,             }, //37
    {0,             Y4,            X5,            Y5,            X6,            Y6,            X7,            Y7,            X8,            Z0^X4^Y4,      Y8,            X9,            Y9,            0,             0,             0,             0,             }, //38
    {0,             Y3,            Y4,            X5,            Y5,            X6,            Y6,            X7,            Y7,            Z0^X4^Y4,      X8,            Y8,            X9,            0,             0,             0,             0,             }, //39
    {0,             X3,            Y3,            Y4,            X5,            Y5,            X6,            Y6,            X7,            Z0^X4^Y4,      Y7,            X8,            Y8,            0,             0,             0,             0,             }, //40
    {0,             Y2,            X3,            Y3,            Y4,            X5,            Y5,            X6,            Y6,            Z0^X4^Y4,      X7,            Y7,            X8,            0,             0,             0,             0,             }, //41
    {0,             X2,            Y2,            X3,            Y3,            Y4,            X5,            Y5,            X6,            Z0^X4^Y4,      Y6,            X7,            Y7,            0,             0,             0,             0,             }, //42
    {0,             X5,            Y5,            X6,            Y6,            X7,            Y7,            X8,            Y8,            Y4^X5^Y5,      Z0^X4^Y4,      X9,            Y9,            0,             0,             0,             0,             }, //43
    {0,             Y3,            X5,            Y5,            X6,            Y6,            X7,            Y7,            X8,            Y4^X5^Y5,      Z0^X4^Y4,      Y8,            X9,            0,             0,             0,             0,             }, //44
    {0,             X3,            Y3,            X5,            Y5,            X6,            Y6,            X7,            Y7,            Y4^X5^Y5,      Z0^X4^Y4,      X8,            Y8,            0,             0,             0,             0,             }, //45
    {0,             Y2,            X3,            Y3,            X5,            Y5,            X6,            Y6,            X7,            Y4^X5^Y5,      Z0^X4^Y4,      Y7,            X8,            0,             0,             0,             0,             }, //46
    {0,             X2,            Y2,            X3,            Y3,            X5,            Y5,            X6,            Y6,            Y4^X5^Y5,      Z0^X4^Y4,      X7,            Y7,            0,             0,             0,             0,             }, //47
    {0,             Y5,            X6,            Y6,            X7,            Y7,            X8,            Y8,            X9,            Y4^X5^Y5,      Z0^X4^Y4,      X5^X6^Y6,      Y9,            0,             0,             0,             0,             }, //48
    {0,             Y3,            Y5,            X6,            Y6,            X7,            Y7,            X8,            Y8,            Y4^X5^Y5,      Z0^X4^Y4,      X5^X6^Y6,      X9,            0,             0,             0,             0,             }, //49
    {0,             X3,            Y3,            Y5,            X6,            Y6,            X7,            Y7,            X8,            Y4^X5^Y5,      Z0^X4^Y4,      X5^X6^Y6,      Y8,            0,             0,             0,             0,             }, //50
    {0,             Y2,            X3,            Y3,            Y5,            X6,            Y6,            X7,            Y7,            Y4^X5^Y5,      Z0^X4^Y4,      X5^X6^Y6,      X8,            0,             0,             0,             0,             }, //51
    {0,             X2,            Y2,            X3,            Y3,            Y5,            X6,            Y6,            X7,            Y4^X5^Y5,      Z0^X4^Y4,      X5^X6^Y6,      Y7,            0,             0,             0,             0,             }, //52
    {0,             X5,            X6,            Y6,            X7,            Y7,            X8,            Y8,            X9,            Y4^X6^Y6,      Z1^X4^Y4,      X5^Y5,         Y9,            0,             0,             0,             0,             }, //53
    {0,             Y3,            X5,            X6,            Y6,            X7,            Y7,            X8,            Y8,            Y4^X6^Y6,      Z1^X4^Y4,      X5^Y5,         X9,            0,             0,             0,             0,             }, //54
    {0,             X3,            Y3,            X5,            X6,            Y6,            X7,            Y7,            X8,            Y4^X6^Y6,      Z1^X4^Y4,      X5^Y5,         Y8,            0,             0,             0,             0,             }, //55
    {0,             Y2,            X3,            Y3,            X5,            X6,            Y6,            X7,            Y7,            Y4^X6^Y6,      Z1^X4^Y4,      X5^Y5,         X8,            0,             0,             0,             0,             }, //56
    {0,             X2,            Y2,            X3,            Y3,            X5,            X6,            Y6,            X7,            Y4^X6^Y6,      Z1^X4^Y4,      X5^Y5,         Y7,            0,             0,             0,             0,             }, //57
    {0,             X5,            X6,            Y6,            X7,            Y7,            X8,            Y8,            X9,            Y4^X6^Y6,      Z1^X4^Y4,      Z0^X5^Y5,      Y9,            0,             0,             0,             0,             }, //58
    {0,             Y3,            X5,            X6,            Y6,            X7,            Y7,            X8,            Y8,            Y4^X6^Y6,      Z1^X4^Y4,      Z0^X5^Y5,      X9,            0,             0,             0,             0,             }, //59
    {0,             X3,            Y3,            X5,            X6,            Y6,            X7,            Y7,            X8,            Y4^X6^Y6,      Z1^X4^Y4,      Z0^X5^Y5,      Y8,            0,             0,             0,             0,             }, //60
    {0,             Y2,            X3,            Y3,            X5,            X6,            Y6,            X7,            Y7,            Y4^X6^Y6,      Z1^X4^Y4,      Z0^X5^Y5,      X8,            0,             0,             0,             0,             }, //61
    {0,             X2,            Y2,            X3,            Y3,            X5,            X6,            Y6,            X7,            Y4^X6^Y6,      Z1^X4^Y4,      Z0^X5^Y5,      Y7,            0,             0,             0,             0,             }, //62
    {0,             X6,            Y6,            X7,            Y7,            X8,            Y8,            X9,            Y9,            Y4^X6^Y6,      Z1^X4^Y4,      Z0^X5^Y5,      X5^X7^Y7,      0,             0,             0,             0,             }, //63
    {0,             Y3,            X6,            Y6,            X7,            Y7,            X8,            Y8,            X9,            Y4^X6^Y6,      Z1^X4^Y4,      Z0^X5^Y5,      X5^X7^Y7,      0,             0,             0,             0,             }, //64
    {0,             X3,            Y3,            X6,            Y6,            X7,            Y7,            X8,            Y8,            Y4^X6^Y6,      Z1^X4^Y4,      Z0^X5^Y5,      X5^X7^Y7,      0,             0,             0,             0,             }, //65
    {0,             Y2,            X3,            Y3,            X6,            Y6,            X7,            Y7,            X8,            Y4^X6^Y6,      Z1^X4^Y4,      Z0^X5^Y5,      X5^X7^Y7,      0,             0,             0,             0,             }, //66
    {0,             X2,            Y2,            X3,            Y3,            X6,            Y6,            X7,            Y7,            Y4^X6^Y6,      Z1^X4^Y4,      Z0^X5^Y5,      X5^X7^Y7,      0,             0,             0,             0,             }, //67
    {0,             X6,            Y6,            X7,            Y7,            X8,            Y8,            X9,            Y9,            Y4^X7^Y7,      Z2^X4^Y4,      Z1^Y5^X6,      X5^Y6,         0,             0,             0,             0,             }, //68
    {0,             Y3,            X6,            Y6,            X7,            Y7,            X8,            Y8,            X9,            Y4^X7^Y7,      Z2^X4^Y4,      Z1^Y5^X6,      X5^Y6,         0,             0,             0,             0,             }, //69
    {0,             X3,            Y3,            X6,            Y6,            X7,            Y7,            X8,            Y8,            Y4^X7^Y7,      Z2^X4^Y4,      Z1^Y5^X6,      X5^Y6,         0,             0,             0,             0,             }, //70
    {0,             Y2,            X3,            Y3,            X6,            Y6,            X7,            Y7,            X8,            Y4^X7^Y7,      Z2^X4^Y4,      Z1^Y5^X6,      X5^Y6,         0,             0,             0,             0,             }, //71
    {0,             X2,            Y2,            X3,            Y3,            X6,            Y6,            X7,            Y7,            Y4^X7^Y7,      Z2^X4^Y4,      Z1^Y5^X6,      X5^Y6,         0,             0,             0,             0,             }, //72
    {0,             X6,            Y6,            X7,            Y7,            X8,            Y8,            X9,            Y9,            Y4^X7^Y7,      Z2^X4^Y4,      Z1^Y5^X6,      Z0^X5^Y6,      0,             0,             0,             0,             }, //73
    {0,             Y3,            X6,            Y6,            X7,            Y7,            X8,            Y8,            X9,            Y4^X7^Y7,      Z2^X4^Y4,      Z1^Y5^X6,      Z0^X5^Y6,      0,             0,             0,             0,             }, //74
    {0,             X3,            Y3,            X6,            Y6,            X7,            Y7,            X8,            Y8,            Y4^X7^Y7,      Z2^X4^Y4,      Z1^Y5^X6,      Z0^X5^Y6,      0,             0,             0,             0,             }, //75
    {0,             Y2,            X3,            Y3,            X6,            Y6,            X7,            Y7,            X8,            Y4^X7^Y7,      Z2^X4^Y4,      Z1^Y5^X6,      Z0^X5^Y6,      0,             0,             0,             0,             }, //76
    {0,             X2,            Y2,            X3,            Y3,            X6,            Y6,            X7,            Y7,            Y4^X7^Y7,      Z2^X4^Y4,      Z1^Y5^X6,      Z0^X5^Y6,      0,             0,             0,             0,             }, //77
    {0,             Y6,            X7,            Y7,            X8,            Y8,            X9,            Y9,            X10,           Y4^X7^Y7,      Z2^X4^Y4,      Z1^Y5^X6,      Z0^X5^Y6,      X6^X8^Y8,      0,             0,             0,             }, //78
    {0,             Y3,            Y6,            X7,            Y7,            X8,            Y8,            X9,            Y9,            Y4^X7^Y7,      Z2^X4^Y4,      Z1^Y5^X6,      Z0^X5^Y6,      X6^X8^Y8,      0,             0,             0,             }, //79
    {0,             X3,            Y3,            Y6,            X7,            Y7,            X8,            Y8,            X9,            Y4^X7^Y7,      Z2^X4^Y4,      Z1^Y5^X6,      Z0^X5^Y6,      X6^X8^Y8,      0,             0,             0,             }, //80
    {0,             Y2,            X3,            Y3,            Y6,            X7,            Y7,            X8,            Y8,            Y4^X7^Y7,      Z2^X4^Y4,      Z1^Y5^X6,      Z0^X5^Y6,      X6^X8^Y8,      0,             0,             0,             }, //81
    {0,             X2,            Y2,            Y3,            X6,            Y6,            X7,            Y7,            X8,            Y4^X7^Y7,      Z2^X4^Y4,      Z1^Y5^X6,      Z0^X5^Y6,      X3^X8^Y8,      0,             0,             0,             }, //82
    {0,             X6,            X7,            Y7,            X8,            Y8,            X9,            Y9,            X10,           Y4^X8^Y8,      Z3^X4^Y4,      Z2^Y5^X7,      Z1^X5^Y7,      X6^Y6,         0,             0,             0,             }, //83
    {0,             Y3,            X6,            X7,            Y7,            X8,            Y8,            X9,            Y9,            Y4^X8^Y8,      Z3^X4^Y4,      Z2^Y5^X7,      Z1^X5^Y7,      X6^Y6,         0,             0,             0,             }, //84
    {0,             X3,            Y3,            X6,            X7,            Y7,            X8,            Y8,            X9,            Y4^X8^Y8,      Z3^X4^Y4,      Z2^Y5^X7,      Z1^X5^Y7,      X6^Y6,         0,             0,             0,             }, //85
    {0,             Y2,            X3,            Y3,            X6,            X7,            Y7,            X8,            Y8,            Y4^X8^Y8,      Z3^X4^Y4,      Z2^Y5^X7,      Z1^X5^Y7,      X6^Y6,         0,             0,             0,             }, //86
    {0,             X2,            X3,            Y3,            X6,            X7,            Y7,            Y2,            X8,            Y4^X8^Y8,      Z2^X4^Y4,      Z1^Y5^X7,      Z0^X5^Y7,      X6^Y6,         0,             0,             0,             }, //87
    {0,             X6,            X7,            Y7,            X8,            Y8,            X9,            Y9,            X10,           Y4^X8^Y8,      Z3^X4^Y4,      Z2^Y5^X7,      Z1^X5^Y7,      Z0^X6^Y6,      0,             0,             0,             }, //88
    {0,             Y3,            X6,            X7,            Y7,            X8,            Y8,            X9,            Y9,            Y4^X8^Y8,      Z3^X4^Y4,      Z2^Y5^X7,      Z1^X5^Y7,      Z0^X6^Y6,      0,             0,             0,             }, //89
    {0,             X3,            Y3,            X6,            X7,            Y7,            X8,            Y8,            X9,            Y4^X8^Y8,      Z3^X4^Y4,      Z2^Y5^X7,      Z1^X5^Y7,      Z0^X6^Y6,      0,             0,             0,             }, //90
    {0,             Y2,            X3,            Y3,            X6,            X7,            Y7,            X8,            Y8,            Y4^X8^Y8,      Z3^X4^Y4,      Z2^Y5^X7,      Z1^X5^Y7,      Z0^X6^Y6,      0,             0,             0,             }, //91
    {0,             X2,            X3,            Y3,            X6,            X7,            Y7,            Y2,            X8,            Y4^X8^Y8,      Z2^X4^Y4,      Z1^Y5^X7,      Z0^X5^Y7,      Y2^X6^Y6,      0,             0,             0,             }, //92
    {0,             X7,            Y7,            X8,            Y8,            X9,            Y9,            X10,           Y10,           Y4^X8^Y8,      Z3^X4^Y4,      Z2^Y5^X7,      Z1^X5^Y7,      Z0^X6^Y6,      X6^X9^Y9,      0,             0,             }, //93
    {0,             Y3,            X7,            Y7,            X8,            Y8,            X9,            Y9,            X10,           Y4^X8^Y8,      Z3^X4^Y4,      Z2^Y5^X7,      Z1^X5^Y7,      Z0^X6^Y6,      X6^X9^Y9,      0,             0,             }, //94
    {0,             X3,            Y3,            X7,            Y7,            X8,            Y8,            X9,            Y9,            Y4^X8^Y8,      Z3^X4^Y4,      Z2^Y5^X7,      Z1^X5^Y7,      Z0^X6^Y6,      X6^X9^Y9,      0,             0,             }, //95
    {0,             Y2,            Y3,            X6,            X7,            Y7,            X8,            Y8,            X9,            Y4^X8^Y8,      Z3^X4^Y4,      Z2^Y5^X7,      Z1^X5^Y7,      Z0^X6^Y6,      X3^X9^Y9,      0,             0,             }, //96
    {0,             X2,            Y3,            X6,            X7,            Y7,            X8,            Y2,            Y8,            Y4^X8^Y8,      Z2^X4^Y4,      Z1^Y5^X7,      Z0^X5^Y7,      Y2^X6^Y6,      X3^X9^Y9,      0,             0,             }, //97
    {0,             X7,            Y7,            X8,            Y8,            X9,            Y9,            X10,           Y10,           Y4^X9^Y9,      X4^Y4^Z4,      Z3^Y5^X8,      Z2^X5^Y8,      Z1^Y6^X7,      X6^Y7,         0,             0,             }, //98
    {0,             Y3,            X7,            Y7,            X8,            Y8,            X9,            Y9,            X10,           Y4^X9^Y9,      X4^Y4^Z4,      Z3^Y5^X8,      Z2^X5^Y8,      Z1^Y6^X7,      X6^Y7,         0,             0,             }, //99
    {0,             X3,            Y3,            X7,            Y7,            X8,            Y8,            X9,            Y9,            Y4^X9^Y9,      X4^Y4^Z4,      Z3^Y5^X8,      Z2^X5^Y8,      Z1^Y6^X7,      X6^Y7,         0,             0,             }, //100
    {0,             X3,            Y3,            X7,            Y7,            X8,            Y8,            Y2,            X9,            Y4^X9^Y9,      Z3^X4^Y4,      Z2^Y5^X8,      Z1^X5^Y8,      Y2^Y6^X7,      X6^Y7,         0,             0,             }, //101
    {0,             X3,            Y3,            X7,            Y7,            X8,            Y8,            X2,            Y2,            Y4^X9^Y9,      Z2^X4^Y4,      Z1^Y5^X8,      Z0^X5^Y8,      Y2^Y6^X7,      X6^Y7,         0,             0,             }, //102
    {0,             X7,            Y7,            X8,            Y8,            X9,            Y9,            X10,           Y10,           Y4^X9^Y9,      X4^Y4^Z4,      Z3^Y5^X8,      Z2^X5^Y8,      Z1^Y6^X7,      Z0^X6^Y7,      0,             0,             }, //103
    {0,             Y3,            X7,            Y7,            X8,            Y8,            X9,            Y9,            X10,           Y4^X9^Y9,      X4^Y4^Z4,      Z3^Y5^X8,      Z2^X5^Y8,      Z1^Y6^X7,      Z0^X6^Y7,      0,             0,             }, //104
    {0,             X3,            Y3,            X7,            Y7,            X8,            Y8,            X9,            Y9,            Y4^X9^Y9,      X4^Y4^Z4,      Z3^Y5^X8,      Z2^X5^Y8,      Z1^Y6^X7,      Z0^X6^Y7,      0,             0,             }, //105
    {0,             X3,            Y3,            X7,            Y7,            X8,            Y8,            Y2,            X9,            Y4^X9^Y9,      Z3^X4^Y4,      Z2^Y5^X8,      Z1^X5^Y8,      Y2^Y6^X7,      Z0^X6^Y7,      0,             0,             }, //106
    {0,             X3,            Y3,            X7,            Y7,            X8,            Y8,            X2,            Y2,            Y4^X9^Y9,      Z2^X4^Y4,      Z1^Y5^X8,      Z0^X5^Y8,      Y2^Y6^X7,      X2^X6^Y7,      0,             0,             }, //107
};

const UINT_64 GFX10_HTILE_SW_PATTERN[][18] =
{
    {0,             0,             0,             X3,            Y3,            X4,            Y4,            X5,            Y5,            X6,            Y6,            X7,            Y7,            0,             0,             0,             0,             0,             }, //0
    {0,             0,             0,             X3,            Y4,            X4,            X5,            Y5,            X6,            Z0^X3^Y3,      Y6,            X7,            Y7,            0,             0,             0,             0,             0,             }, //1
    {0,             0,             0,             X3,            Y4,            X5,            Y5,            X6,            Y6,            Z1^X3^Y3,      Z0^X4^Y4,      X7,            Y7,            X8,            0,             0,             0,             0,             }, //2
    {0,             0,             0,             X3,            Y4,            Y5,            X6,            Y6,            X7,            Z2^X3^Y3,      Z1^X4^Y4,      Z0^X5^Y5,      Y7,            X8,            Y8,            0,             0,             0,             }, //3
    {0,             0,             0,             X3,            Y4,            X6,            Y6,            X7,            Y7,            X3^Y3^Z3,      Z2^X4^Y4,      Z1^Y5^X6,      Z0^X5^Y6,      X8,            Y8,            X9,            0,             0,             }, //4
    {0,             0,             0,             X3,            Y4,            X6,            Y6,            X7,            Y7,            Z2^X3^Y3,      Z1^X4^Y4,      Z0^Y5^X6,      X5^Y6,         X8,            Y8,            X9,            0,             0,             }, //5
    {0,             0,             0,             X3,            Y4,            Y6,            X7,            Y7,            X8,            X3^Y3^Z4,      Z3^X4^Y4,      Z2^Y5^X7,      Z1^X5^Y7,      Z0^X6^Y6,      Y8,            X9,            Y9,            0,             }, //6
    {0,             0,             0,             X3,            Y4,            Y6,            X7,            Y7,            X8,            X3^Y3^Z3,      Z2^X4^Y4,      Z1^Y5^X7,      Z0^X5^Y7,      X6^Y6,         Y8,            X9,            Y9,            0,             }, //7
    {0,             0,             0,             X3,            Y4,            Y6,            X7,            Y7,            X8,            Z2^X3^Y3,      Z1^X4^Y4,      Z0^Y5^X7,      X5^Y7,         X6^Y6,         Y8,            X9,            Y9,            0,             }, //8
    {0,             0,             0,             X3,            Y4,            X7,            Y7,            X8,            Y8,            X3^Y3^Z5,      X4^Y4^Z4,      Z3^Y5^X8,      Z2^X5^Y8,      Z1^Y6^X7,      Z0^X6^Y7,      X9,            Y9,            X10,           }, //9
    {0,             0,             0,             X3,            Y4,            X7,            Y7,            X8,            Y8,            X3^Y3^Z4,      Z3^X4^Y4,      Z2^Y5^X8,      Z1^X5^Y8,      Z0^Y6^X7,      X6^Y7,         X9,            Y9,            X10,           }, //10
    {0,             0,             0,             X3,            Y4,            X7,            Y7,            X8,            Y8,            X3^Y3^Z3,      Z2^X4^Y4,      Z1^Y5^X8,      Z0^X5^Y8,      Y6^X7,         X6^Y7,         X9,            Y9,            X10,           }, //11
    {0,             0,             0,             X3,            Y4,            X7,            Y7,            X8,            Y8,            Z2^X3^Y3,      Z1^X4^Y4,      Z0^Y5^X8,      X5^Y8,         Y6^X7,         X6^Y7,         X9,            Y9,            X10,           }, //12
    {0,             0,             0,             X3,            Y3,            Y4,            X5,            Y5,            X6,            Z0^X4^Y4,      Y6,            X7,            Y7,            0,             0,             0,             0,             0,             }, //13
    {0,             0,             0,             X3,            Y3,            X5,            Y5,            X6,            Y6,            Y4^X5^Y5,      Z0^X4^Y4,      X7,            Y7,            X8,            0,             0,             0,             0,             }, //14
    {0,             0,             0,             X3,            Y3,            Y5,            X6,            Y6,            X7,            Y4^X5^Y5,      Z0^X4^Y4,      X5^Y5,         Y7,            X8,            Y8,            0,             0,             0,             }, //15
    {0,             0,             0,             X3,            Y3,            X5,            X6,            Y6,            X7,            Y4^X6^Y6,      Z1^X4^Y4,      Y7,            X8,            Y8,            X5^Y5,         0,             0,             0,             }, //16
    {0,             0,             0,             X3,            Y3,            X5,            X6,            Y6,            X7,            Y4^X6^Y6,      Z1^X4^Y4,      Z0^X5^Y5,      Y7,            X8,            Y8,            0,             0,             0,             }, //17
    {0,             0,             0,             X3,            Y3,            X6,            Y6,            X7,            Y7,            Y4^X6^Y6,      Z1^X4^Y4,      Z0^X5^Y5,      X5^Y6,         X8,            Y8,            X9,            0,             0,             }, //18
    {0,             0,             0,             X3,            Y3,            Y4,            X5,            X6,            Y6,            Z1^X4^Y4,      Z0^X5^Y5,      X7,            Y7,            X8,            0,             0,             0,             0,             }, //19
    {0,             0,             0,             X3,            Y3,            X6,            Y6,            X7,            Y7,            Y4^X7^Y7,      Z1^X4^Y4,      Z0^Y5^X6,      X8,            Y8,            X9,            X5^Y6,         0,             0,             }, //20
    {0,             0,             0,             X3,            Y3,            X6,            Y6,            X7,            Y7,            Y4^X7^Y7,      Z1^X4^Y4,      Z0^Y5^X6,      X5^Y6,         X8,            Y8,            X9,            0,             0,             }, //21
    {0,             0,             0,             X3,            Y3,            Y6,            X7,            Y7,            X8,            Y4^X7^Y7,      Z1^X4^Y4,      Z0^Y5^X6,      X5^Y6,         X6^Y6,         Y8,            X9,            Y9,            0,             }, //22
    {0,             0,             0,             X3,            Y3,            Y4,            X6,            Y6,            X7,            Z1^X4^Y4,      Z0^Y5^X6,      X5^Y6,         Y7,            X8,            Y8,            0,             0,             0,             }, //23
    {0,             0,             0,             X3,            Y3,            X6,            X7,            Y7,            X8,            Y4^X8^Y8,      Z1^X4^Y4,      Z0^Y5^X7,      X5^Y7,         Y8,            X9,            Y9,            X6^Y6,         0,             }, //24
    {0,             0,             0,             X3,            Y3,            X6,            X7,            Y7,            X8,            Y4^X8^Y8,      Z1^X4^Y4,      Z0^Y5^X7,      X5^Y7,         X6^Y6,         Y8,            X9,            Y9,            0,             }, //25
    {0,             0,             0,             X3,            Y3,            X7,            Y7,            X8,            Y8,            Y4^X8^Y8,      Z1^X4^Y4,      Z0^Y5^X7,      X5^Y7,         X6^Y6,         X6^Y8,         X9,            Y9,            X10,           }, //26
    {0,             0,             0,             X3,            Y3,            Y4,            X6,            X7,            Y7,            Z1^X4^Y4,      Z0^Y5^X7,      X5^Y7,         X6^Y6,         X8,            Y8,            X9,            0,             0,             }, //27
    {0,             0,             0,             X3,            Y3,            X7,            Y7,            X8,            Y8,            Y4^X9^Y9,      Z1^X4^Y4,      Z0^Y5^X8,      X5^Y8,         Y6^X7,         X9,            Y9,            X10,           X6^Y7,         }, //28
    {0,             0,             0,             X3,            Y3,            X7,            Y7,            X8,            Y8,            Y4^X9^Y9,      Z1^X4^Y4,      Z0^Y5^X8,      X5^Y8,         Y6^X7,         X6^Y7,         X9,            Y9,            X10,           }, //29
};

const UINT_64 GFX10_CMASK_SW_PATTERN[][17] =
{
    {X3,            Y3,            X4,            Y4,            X5,            Y5,            X6,            Y6,            X7,            Y7,            X8,            Y8,            X9,            0,             0,             0,             0,             }, //0
    {X3,            Y4,            X4,            X5,            Y5,            X6,            Y6,            X7,            Y7,            Z0^X3^Y3,      X8,            Y8,            X9,            0,             0,             0,             0,             }, //1
    {X3,            Y4,            X5,            Y5,            X6,            Y6,            X7,            Y7,            X8,            Z1^X3^Y3,      Z0^X4^Y4,      Y8,            X9,            0,             0,             0,             0,             }, //2
    {X3,            Y4,            Y5,            X6,            Y6,            X7,            Y7,            X8,            Y8,            Z2^X3^Y3,      Z1^X4^Y4,      Z0^X5^Y5,      X9,            0,             0,             0,             0,             }, //3
    {X3,            Y4,            X6,            Y6,            X7,            Y7,            X8,            Y8,            X9,            X3^Y3^Z3,      Z2^X4^Y4,      Z1^Y5^X6,      Z0^X5^Y6,      0,             0,             0,             0,             }, //4
    {X3,            Y4,            Y6,            X7,            Y7,            X8,            Y8,            X9,            Y9,            X3^Y3^Z4,      Z3^X4^Y4,      Z2^Y5^X7,      Z1^X5^Y7,      Z0^X6^Y6,      0,             0,             0,             }, //5
    {X3,            Y4,            X7,            Y7,            X8,            Y8,            X9,            Y9,            X10,           X3^Y3^Z5,      X4^Y4^Z4,      Z3^Y5^X8,      Z2^X5^Y8,      Z1^Y6^X7,      Z0^X6^Y7,      0,             0,             }, //6
    {X3,            Y4,            X7,            Y7,            X8,            Y8,            X9,            Y9,            X10,           X3^Y3^Z4,      Z3^X4^Y4,      Z2^Y5^X8,      Z1^X5^Y8,      Y6^X7,         Z0^X6^Y7,      0,             0,             }, //7
    {X3,            Y3,            Y4,            X5,            Y5,            X6,            Y6,            X7,            Y7,            Z0^X4^Y4,      X8,            Y8,            X9,            0,             0,             0,             0,             }, //8
    {X3,            Y3,            X5,            Y5,            X6,            Y6,            X7,            Y7,            X8,            Y4^X5^Y5,      Z0^X4^Y4,      Y8,            X9,            0,             0,             0,             0,             }, //9
    {X3,            Y3,            Y5,            X6,            Y6,            X7,            Y7,            X8,            Y8,            Y4^X5^Y5,      Z0^X4^Y4,      X5^Y5,         X9,            0,             0,             0,             0,             }, //10
    {X3,            Y3,            X5,            X6,            Y6,            X7,            Y7,            X8,            Y8,            Y4^X6^Y6,      Z1^X4^Y4,      X5^Y5,         X9,            0,             0,             0,             0,             }, //11
    {X3,            Y3,            X5,            X6,            Y6,            X7,            Y7,            X8,            Y8,            Y4^X6^Y6,      Z1^X4^Y4,      Z0^X5^Y5,      X9,            0,             0,             0,             0,             }, //12
    {X3,            Y3,            X6,            Y6,            X7,            Y7,            X8,            Y8,            X9,            Y4^X6^Y6,      Z1^X4^Y4,      Z0^X5^Y5,      X5^Y6,         0,             0,             0,             0,             }, //13
    {X3,            Y3,            Y4,            X5,            X6,            Y6,            X7,            Y7,            X8,            Z1^X4^Y4,      Z0^X5^Y5,      Y8,            X9,            0,             0,             0,             0,             }, //14
    {X3,            Y3,            X6,            Y6,            X7,            Y7,            X8,            Y8,            X9,            Y4^X7^Y7,      Z1^X4^Y4,      Z0^Y5^X6,      X5^Y6,         0,             0,             0,             0,             }, //15
    {X3,            Y3,            X6,            Y6,            X7,            Y7,            X8,            Y8,            X9,            Y4^X7^Y7,      Z2^X4^Y4,      Z1^Y5^X6,      X5^Y6,         0,             0,             0,             0,             }, //16
    {X3,            Y3,            X6,            Y6,            X7,            Y7,            X8,            Y8,            X9,            Y4^X7^Y7,      Z2^X4^Y4,      Z1^Y5^X6,      Z0^X5^Y6,      0,             0,             0,             0,             }, //17
    {X3,            Y3,            Y6,            X7,            Y7,            X8,            Y8,            X9,            Y9,            Y4^X7^Y7,      Z1^X4^Y4,      Z0^Y5^X6,      X5^Y6,         X6^Y6,         0,             0,             0,             }, //18
    {X3,            Y3,            Y6,            X7,            Y7,            X8,            Y8,            X9,            Y9,            Y4^X7^Y7,      Z2^X4^Y4,      Z1^Y5^X6,      Z0^X5^Y6,      X6^Y6,         0,             0,             0,             }, //19
    {X3,            Y3,            Y4,            X6,            Y6,            X7,            Y7,            X8,            Y8,            Z1^X4^Y4,      Z0^Y5^X6,      X5^Y6,         X9,            0,             0,             0,             0,             }, //20
    {X3,            Y3,            Y4,            X6,            Y6,            X7,            Y7,            X8,            Y8,            Z2^X4^Y4,      Z1^Y5^X6,      Z0^X5^Y6,      X9,            0,             0,             0,             0,             }, //21
    {X3,            Y3,            X6,            X7,            Y7,            X8,            Y8,            X9,            Y9,            Y4^X8^Y8,      Z1^X4^Y4,      Z0^Y5^X7,      X5^Y7,         X6^Y6,         0,             0,             0,             }, //22
    {X3,            Y3,            X6,            X7,            Y7,            X8,            Y8,            X9,            Y9,            Y4^X8^Y8,      Z3^X4^Y4,      Z2^Y5^X7,      Z1^X5^Y7,      X6^Y6,         0,             0,             0,             }, //23
    {X3,            Y3,            X6,            X7,            Y7,            X8,            Y8,            X9,            Y9,            Y4^X8^Y8,      Z3^X4^Y4,      Z2^Y5^X7,      Z1^X5^Y7,      Z0^X6^Y6,      0,             0,             0,             }, //24
    {X3,            Y3,            X7,            Y7,            X8,            Y8,            X9,            Y9,            X10,           Y4^X8^Y8,      Z1^X4^Y4,      Z0^Y5^X7,      X5^Y7,         X6^Y6,         X6^Y8,         0,             0,             }, //25
    {X3,            Y3,            X7,            Y7,            X8,            Y8,            X9,            Y9,            X10,           Y4^X8^Y8,      Z3^X4^Y4,      Z2^Y5^X7,      Z1^X5^Y7,      Z0^X6^Y6,      X6^Y8,         0,             0,             }, //26
    {X3,            Y3,            Y4,            X6,            X7,            Y7,            X8,            Y8,            X9,            Z1^X4^Y4,      Z0^Y5^X7,      X5^Y7,         X6^Y6,         0,             0,             0,             0,             }, //27
    {X3,            Y3,            Y4,            X6,            X7,            Y7,            X8,            Y8,            X9,            Z3^X4^Y4,      Z2^Y5^X7,      Z1^X5^Y7,      Z0^X6^Y6,      0,             0,             0,             0,             }, //28
    {X3,            Y3,            X7,            Y7,            X8,            Y8,            X9,            Y9,            X10,           Y4^X9^Y9,      Z1^X4^Y4,      Z0^Y5^X8,      X5^Y8,         Y6^X7,         X6^Y7,         0,             0,             }, //29
    {X3,            Y3,            X7,            Y7,            X8,            Y8,            X9,            Y9,            X10,           Y4^X9^Y9,      X4^Y4^Z4,      Z3^Y5^X8,      Z2^X5^Y8,      Z1^Y6^X7,      X6^Y7,         0,             0,             }, //30
    {X3,            Y3,            X7,            Y7,            X8,            Y8,            X9,            Y9,            X10,           Y4^X9^Y9,      X4^Y4^Z4,      Z3^Y5^X8,      Z2^X5^Y8,      Z1^Y6^X7,      Z0^X6^Y7,      0,             0,             }, //31
    {X3,            Y3,            X6,            X7,            Y7,            X8,            X9,            Y9,            X10,           Y4^X8^Y8,      Z3^X4^Y4,      Z2^Y5^X7,      Z1^X5^Y7,      Z0^X6^Y6,      X3^Y8,         0,             0,             }, //32
    {X3,            Y3,            X7,            Y7,            X8,            Y8,            X9,            Y9,            X10,           Y4^X9^Y9,      Z3^X4^Y4,      Z2^Y5^X8,      Z1^X5^Y8,      Y6^X7,         X6^Y7,         0,             0,             }, //33
    {X3,            Y3,            X7,            Y7,            X8,            Y8,            X9,            Y9,            X10,           Y4^X9^Y9,      Z3^X4^Y4,      Z2^Y5^X8,      Z1^X5^Y8,      Y6^X7,         Z0^X6^Y7,      0,             0,             }, //34
};

}// V2
} // Addr

#endif
