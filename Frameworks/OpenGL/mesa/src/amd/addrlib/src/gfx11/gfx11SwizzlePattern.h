/*
************************************************************************************************************************
*
*  Copyright (C) 2007-2022 Advanced Micro Devices, Inc.  All rights reserved.
*  SPDX-License-Identifier: MIT
*
***********************************************************************************************************************/

/**
************************************************************************************************************************
* @file  gfx11SwizzlePattern.h
* @brief swizzle pattern for gfx11.
************************************************************************************************************************
*/

#ifndef __GFX11_SWIZZLE_PATTERN_H__
#define __GFX11_SWIZZLE_PATTERN_H__


namespace Addr
{
namespace V2
{
const ADDR_SW_PATINFO GFX11_SW_256_D_PATINFO[] =
{
    {   1,    0,    0,    0,    0, } , // 1 pipes (1 PKRs) 1 bpe @ SW_256_D
    {   1,    1,    0,    0,    0, } , // 1 pipes (1 PKRs) 2 bpe @ SW_256_D
    {   1,    2,    0,    0,    0, } , // 1 pipes (1 PKRs) 4 bpe @ SW_256_D
    {   1,    3,    0,    0,    0, } , // 1 pipes (1 PKRs) 8 bpe @ SW_256_D
    {   1,    4,    0,    0,    0, } , // 1 pipes (1 PKRs) 16 bpe @ SW_256_D
    {   1,    0,    0,    0,    0, } , // 2 pipes (1-2 PKRs) 1 bpe @ SW_256_D
    {   1,    1,    0,    0,    0, } , // 2 pipes (1-2 PKRs) 2 bpe @ SW_256_D
    {   1,    2,    0,    0,    0, } , // 2 pipes (1-2 PKRs) 4 bpe @ SW_256_D
    {   1,    3,    0,    0,    0, } , // 2 pipes (1-2 PKRs) 8 bpe @ SW_256_D
    {   1,    4,    0,    0,    0, } , // 2 pipes (1-2 PKRs) 16 bpe @ SW_256_D
    {   1,    0,    0,    0,    0, } , // 4 pipes (1-2 PKRs) 1 bpe @ SW_256_D
    {   1,    1,    0,    0,    0, } , // 4 pipes (1-2 PKRs) 2 bpe @ SW_256_D
    {   1,    2,    0,    0,    0, } , // 4 pipes (1-2 PKRs) 4 bpe @ SW_256_D
    {   1,    3,    0,    0,    0, } , // 4 pipes (1-2 PKRs) 8 bpe @ SW_256_D
    {   1,    4,    0,    0,    0, } , // 4 pipes (1-2 PKRs) 16 bpe @ SW_256_D
    {   1,    0,    0,    0,    0, } , // 8 pipes (2 PKRs) 1 bpe @ SW_256_D
    {   1,    1,    0,    0,    0, } , // 8 pipes (2 PKRs) 2 bpe @ SW_256_D
    {   1,    2,    0,    0,    0, } , // 8 pipes (2 PKRs) 4 bpe @ SW_256_D
    {   1,    3,    0,    0,    0, } , // 8 pipes (2 PKRs) 8 bpe @ SW_256_D
    {   1,    4,    0,    0,    0, } , // 8 pipes (2 PKRs) 16 bpe @ SW_256_D
    {   1,    0,    0,    0,    0, } , // 4 pipes (4 PKRs) 1 bpe @ SW_256_D
    {   1,    1,    0,    0,    0, } , // 4 pipes (4 PKRs) 2 bpe @ SW_256_D
    {   1,    2,    0,    0,    0, } , // 4 pipes (4 PKRs) 4 bpe @ SW_256_D
    {   1,    3,    0,    0,    0, } , // 4 pipes (4 PKRs) 8 bpe @ SW_256_D
    {   1,    4,    0,    0,    0, } , // 4 pipes (4 PKRs) 16 bpe @ SW_256_D
    {   1,    0,    0,    0,    0, } , // 8 pipes (4 PKRs) 1 bpe @ SW_256_D
    {   1,    1,    0,    0,    0, } , // 8 pipes (4 PKRs) 2 bpe @ SW_256_D
    {   1,    2,    0,    0,    0, } , // 8 pipes (4 PKRs) 4 bpe @ SW_256_D
    {   1,    3,    0,    0,    0, } , // 8 pipes (4 PKRs) 8 bpe @ SW_256_D
    {   1,    4,    0,    0,    0, } , // 8 pipes (4 PKRs) 16 bpe @ SW_256_D
    {   1,    0,    0,    0,    0, } , // 16 pipes (4 PKRs) 1 bpe @ SW_256_D
    {   1,    1,    0,    0,    0, } , // 16 pipes (4 PKRs) 2 bpe @ SW_256_D
    {   1,    2,    0,    0,    0, } , // 16 pipes (4 PKRs) 4 bpe @ SW_256_D
    {   1,    3,    0,    0,    0, } , // 16 pipes (4 PKRs) 8 bpe @ SW_256_D
    {   1,    4,    0,    0,    0, } , // 16 pipes (4 PKRs) 16 bpe @ SW_256_D
    {   1,    0,    0,    0,    0, } , // 8 pipes (8 PKRs) 1 bpe @ SW_256_D
    {   1,    1,    0,    0,    0, } , // 8 pipes (8 PKRs) 2 bpe @ SW_256_D
    {   1,    2,    0,    0,    0, } , // 8 pipes (8 PKRs) 4 bpe @ SW_256_D
    {   1,    3,    0,    0,    0, } , // 8 pipes (8 PKRs) 8 bpe @ SW_256_D
    {   1,    4,    0,    0,    0, } , // 8 pipes (8 PKRs) 16 bpe @ SW_256_D
    {   1,    0,    0,    0,    0, } , // 16 pipes (8 PKRs) 1 bpe @ SW_256_D
    {   1,    1,    0,    0,    0, } , // 16 pipes (8 PKRs) 2 bpe @ SW_256_D
    {   1,    2,    0,    0,    0, } , // 16 pipes (8 PKRs) 4 bpe @ SW_256_D
    {   1,    3,    0,    0,    0, } , // 16 pipes (8 PKRs) 8 bpe @ SW_256_D
    {   1,    4,    0,    0,    0, } , // 16 pipes (8 PKRs) 16 bpe @ SW_256_D
    {   1,    0,    0,    0,    0, } , // 32 pipes (8 PKRs) 1 bpe @ SW_256_D
    {   1,    1,    0,    0,    0, } , // 32 pipes (8 PKRs) 2 bpe @ SW_256_D
    {   1,    2,    0,    0,    0, } , // 32 pipes (8 PKRs) 4 bpe @ SW_256_D
    {   1,    3,    0,    0,    0, } , // 32 pipes (8 PKRs) 8 bpe @ SW_256_D
    {   1,    4,    0,    0,    0, } , // 32 pipes (8 PKRs) 16 bpe @ SW_256_D
    {   1,    0,    0,    0,    0, } , // 16 pipes (16 PKRs) 1 bpe @ SW_256_D
    {   1,    1,    0,    0,    0, } , // 16 pipes (16 PKRs) 2 bpe @ SW_256_D
    {   1,    2,    0,    0,    0, } , // 16 pipes (16 PKRs) 4 bpe @ SW_256_D
    {   1,    3,    0,    0,    0, } , // 16 pipes (16 PKRs) 8 bpe @ SW_256_D
    {   1,    4,    0,    0,    0, } , // 16 pipes (16 PKRs) 16 bpe @ SW_256_D
    {   1,    0,    0,    0,    0, } , // 32 pipes (16 PKRs) 1 bpe @ SW_256_D
    {   1,    1,    0,    0,    0, } , // 32 pipes (16 PKRs) 2 bpe @ SW_256_D
    {   1,    2,    0,    0,    0, } , // 32 pipes (16 PKRs) 4 bpe @ SW_256_D
    {   1,    3,    0,    0,    0, } , // 32 pipes (16 PKRs) 8 bpe @ SW_256_D
    {   1,    4,    0,    0,    0, } , // 32 pipes (16 PKRs) 16 bpe @ SW_256_D
    {   1,    0,    0,    0,    0, } , // 64 pipes (16 PKRs) 1 bpe @ SW_256_D
    {   1,    1,    0,    0,    0, } , // 64 pipes (16 PKRs) 2 bpe @ SW_256_D
    {   1,    2,    0,    0,    0, } , // 64 pipes (16 PKRs) 4 bpe @ SW_256_D
    {   1,    3,    0,    0,    0, } , // 64 pipes (16 PKRs) 8 bpe @ SW_256_D
    {   1,    4,    0,    0,    0, } , // 64 pipes (16 PKRs) 16 bpe @ SW_256_D
    {   1,    0,    0,    0,    0, } , // 32 pipes (32 PKRs) 1 bpe @ SW_256_D
    {   1,    1,    0,    0,    0, } , // 32 pipes (32 PKRs) 2 bpe @ SW_256_D
    {   1,    2,    0,    0,    0, } , // 32 pipes (32 PKRs) 4 bpe @ SW_256_D
    {   1,    3,    0,    0,    0, } , // 32 pipes (32 PKRs) 8 bpe @ SW_256_D
    {   1,    4,    0,    0,    0, } , // 32 pipes (32 PKRs) 16 bpe @ SW_256_D
    {   1,    0,    0,    0,    0, } , // 64 pipes (32 PKRs) 1 bpe @ SW_256_D
    {   1,    1,    0,    0,    0, } , // 64 pipes (32 PKRs) 2 bpe @ SW_256_D
    {   1,    2,    0,    0,    0, } , // 64 pipes (32 PKRs) 4 bpe @ SW_256_D
    {   1,    3,    0,    0,    0, } , // 64 pipes (32 PKRs) 8 bpe @ SW_256_D
    {   1,    4,    0,    0,    0, } , // 64 pipes (32 PKRs) 16 bpe @ SW_256_D
};

const ADDR_SW_PATINFO GFX11_SW_4K_D_PATINFO[] =
{
    {   1,    0,    1,    0,    0, } , // 1 pipes (1 PKRs) 1 bpe @ SW_4K_D
    {   1,    1,    2,    0,    0, } , // 1 pipes (1 PKRs) 2 bpe @ SW_4K_D
    {   1,    2,    3,    0,    0, } , // 1 pipes (1 PKRs) 4 bpe @ SW_4K_D
    {   1,    3,    4,    0,    0, } , // 1 pipes (1 PKRs) 8 bpe @ SW_4K_D
    {   1,    4,    5,    0,    0, } , // 1 pipes (1 PKRs) 16 bpe @ SW_4K_D
    {   1,    0,    1,    0,    0, } , // 2 pipes (1-2 PKRs) 1 bpe @ SW_4K_D
    {   1,    1,    2,    0,    0, } , // 2 pipes (1-2 PKRs) 2 bpe @ SW_4K_D
    {   1,    2,    3,    0,    0, } , // 2 pipes (1-2 PKRs) 4 bpe @ SW_4K_D
    {   1,    3,    4,    0,    0, } , // 2 pipes (1-2 PKRs) 8 bpe @ SW_4K_D
    {   1,    4,    5,    0,    0, } , // 2 pipes (1-2 PKRs) 16 bpe @ SW_4K_D
    {   1,    0,    1,    0,    0, } , // 4 pipes (1-2 PKRs) 1 bpe @ SW_4K_D
    {   1,    1,    2,    0,    0, } , // 4 pipes (1-2 PKRs) 2 bpe @ SW_4K_D
    {   1,    2,    3,    0,    0, } , // 4 pipes (1-2 PKRs) 4 bpe @ SW_4K_D
    {   1,    3,    4,    0,    0, } , // 4 pipes (1-2 PKRs) 8 bpe @ SW_4K_D
    {   1,    4,    5,    0,    0, } , // 4 pipes (1-2 PKRs) 16 bpe @ SW_4K_D
    {   1,    0,    1,    0,    0, } , // 8 pipes (2 PKRs) 1 bpe @ SW_4K_D
    {   1,    1,    2,    0,    0, } , // 8 pipes (2 PKRs) 2 bpe @ SW_4K_D
    {   1,    2,    3,    0,    0, } , // 8 pipes (2 PKRs) 4 bpe @ SW_4K_D
    {   1,    3,    4,    0,    0, } , // 8 pipes (2 PKRs) 8 bpe @ SW_4K_D
    {   1,    4,    5,    0,    0, } , // 8 pipes (2 PKRs) 16 bpe @ SW_4K_D
    {   1,    0,    1,    0,    0, } , // 4 pipes (4 PKRs) 1 bpe @ SW_4K_D
    {   1,    1,    2,    0,    0, } , // 4 pipes (4 PKRs) 2 bpe @ SW_4K_D
    {   1,    2,    3,    0,    0, } , // 4 pipes (4 PKRs) 4 bpe @ SW_4K_D
    {   1,    3,    4,    0,    0, } , // 4 pipes (4 PKRs) 8 bpe @ SW_4K_D
    {   1,    4,    5,    0,    0, } , // 4 pipes (4 PKRs) 16 bpe @ SW_4K_D
    {   1,    0,    1,    0,    0, } , // 8 pipes (4 PKRs) 1 bpe @ SW_4K_D
    {   1,    1,    2,    0,    0, } , // 8 pipes (4 PKRs) 2 bpe @ SW_4K_D
    {   1,    2,    3,    0,    0, } , // 8 pipes (4 PKRs) 4 bpe @ SW_4K_D
    {   1,    3,    4,    0,    0, } , // 8 pipes (4 PKRs) 8 bpe @ SW_4K_D
    {   1,    4,    5,    0,    0, } , // 8 pipes (4 PKRs) 16 bpe @ SW_4K_D
    {   1,    0,    1,    0,    0, } , // 16 pipes (4 PKRs) 1 bpe @ SW_4K_D
    {   1,    1,    2,    0,    0, } , // 16 pipes (4 PKRs) 2 bpe @ SW_4K_D
    {   1,    2,    3,    0,    0, } , // 16 pipes (4 PKRs) 4 bpe @ SW_4K_D
    {   1,    3,    4,    0,    0, } , // 16 pipes (4 PKRs) 8 bpe @ SW_4K_D
    {   1,    4,    5,    0,    0, } , // 16 pipes (4 PKRs) 16 bpe @ SW_4K_D
    {   1,    0,    1,    0,    0, } , // 8 pipes (8 PKRs) 1 bpe @ SW_4K_D
    {   1,    1,    2,    0,    0, } , // 8 pipes (8 PKRs) 2 bpe @ SW_4K_D
    {   1,    2,    3,    0,    0, } , // 8 pipes (8 PKRs) 4 bpe @ SW_4K_D
    {   1,    3,    4,    0,    0, } , // 8 pipes (8 PKRs) 8 bpe @ SW_4K_D
    {   1,    4,    5,    0,    0, } , // 8 pipes (8 PKRs) 16 bpe @ SW_4K_D
    {   1,    0,    1,    0,    0, } , // 16 pipes (8 PKRs) 1 bpe @ SW_4K_D
    {   1,    1,    2,    0,    0, } , // 16 pipes (8 PKRs) 2 bpe @ SW_4K_D
    {   1,    2,    3,    0,    0, } , // 16 pipes (8 PKRs) 4 bpe @ SW_4K_D
    {   1,    3,    4,    0,    0, } , // 16 pipes (8 PKRs) 8 bpe @ SW_4K_D
    {   1,    4,    5,    0,    0, } , // 16 pipes (8 PKRs) 16 bpe @ SW_4K_D
    {   1,    0,    1,    0,    0, } , // 32 pipes (8 PKRs) 1 bpe @ SW_4K_D
    {   1,    1,    2,    0,    0, } , // 32 pipes (8 PKRs) 2 bpe @ SW_4K_D
    {   1,    2,    3,    0,    0, } , // 32 pipes (8 PKRs) 4 bpe @ SW_4K_D
    {   1,    3,    4,    0,    0, } , // 32 pipes (8 PKRs) 8 bpe @ SW_4K_D
    {   1,    4,    5,    0,    0, } , // 32 pipes (8 PKRs) 16 bpe @ SW_4K_D
    {   1,    0,    1,    0,    0, } , // 16 pipes (16 PKRs) 1 bpe @ SW_4K_D
    {   1,    1,    2,    0,    0, } , // 16 pipes (16 PKRs) 2 bpe @ SW_4K_D
    {   1,    2,    3,    0,    0, } , // 16 pipes (16 PKRs) 4 bpe @ SW_4K_D
    {   1,    3,    4,    0,    0, } , // 16 pipes (16 PKRs) 8 bpe @ SW_4K_D
    {   1,    4,    5,    0,    0, } , // 16 pipes (16 PKRs) 16 bpe @ SW_4K_D
    {   1,    0,    1,    0,    0, } , // 32 pipes (16 PKRs) 1 bpe @ SW_4K_D
    {   1,    1,    2,    0,    0, } , // 32 pipes (16 PKRs) 2 bpe @ SW_4K_D
    {   1,    2,    3,    0,    0, } , // 32 pipes (16 PKRs) 4 bpe @ SW_4K_D
    {   1,    3,    4,    0,    0, } , // 32 pipes (16 PKRs) 8 bpe @ SW_4K_D
    {   1,    4,    5,    0,    0, } , // 32 pipes (16 PKRs) 16 bpe @ SW_4K_D
    {   1,    0,    1,    0,    0, } , // 64 pipes (16 PKRs) 1 bpe @ SW_4K_D
    {   1,    1,    2,    0,    0, } , // 64 pipes (16 PKRs) 2 bpe @ SW_4K_D
    {   1,    2,    3,    0,    0, } , // 64 pipes (16 PKRs) 4 bpe @ SW_4K_D
    {   1,    3,    4,    0,    0, } , // 64 pipes (16 PKRs) 8 bpe @ SW_4K_D
    {   1,    4,    5,    0,    0, } , // 64 pipes (16 PKRs) 16 bpe @ SW_4K_D
    {   1,    0,    1,    0,    0, } , // 32 pipes (32 PKRs) 1 bpe @ SW_4K_D
    {   1,    1,    2,    0,    0, } , // 32 pipes (32 PKRs) 2 bpe @ SW_4K_D
    {   1,    2,    3,    0,    0, } , // 32 pipes (32 PKRs) 4 bpe @ SW_4K_D
    {   1,    3,    4,    0,    0, } , // 32 pipes (32 PKRs) 8 bpe @ SW_4K_D
    {   1,    4,    5,    0,    0, } , // 32 pipes (32 PKRs) 16 bpe @ SW_4K_D
    {   1,    0,    1,    0,    0, } , // 64 pipes (32 PKRs) 1 bpe @ SW_4K_D
    {   1,    1,    2,    0,    0, } , // 64 pipes (32 PKRs) 2 bpe @ SW_4K_D
    {   1,    2,    3,    0,    0, } , // 64 pipes (32 PKRs) 4 bpe @ SW_4K_D
    {   1,    3,    4,    0,    0, } , // 64 pipes (32 PKRs) 8 bpe @ SW_4K_D
    {   1,    4,    5,    0,    0, } , // 64 pipes (32 PKRs) 16 bpe @ SW_4K_D
};

const ADDR_SW_PATINFO GFX11_SW_4K_D_X_PATINFO[] =
{
    {   1,    0,    1,    0,    0, } , // 1 pipes (1 PKRs) 1 bpe @ SW_4K_D_X
    {   1,    1,    2,    0,    0, } , // 1 pipes (1 PKRs) 2 bpe @ SW_4K_D_X
    {   1,    2,    3,    0,    0, } , // 1 pipes (1 PKRs) 4 bpe @ SW_4K_D_X
    {   1,    3,    4,    0,    0, } , // 1 pipes (1 PKRs) 8 bpe @ SW_4K_D_X
    {   1,    4,    5,    0,    0, } , // 1 pipes (1 PKRs) 16 bpe @ SW_4K_D_X
    {   3,    0,    6,    0,    0, } , // 2 pipes (1-2 PKRs) 1 bpe @ SW_4K_D_X
    {   3,    1,    7,    0,    0, } , // 2 pipes (1-2 PKRs) 2 bpe @ SW_4K_D_X
    {   3,    2,    8,    0,    0, } , // 2 pipes (1-2 PKRs) 4 bpe @ SW_4K_D_X
    {   3,    3,    9,    0,    0, } , // 2 pipes (1-2 PKRs) 8 bpe @ SW_4K_D_X
    {   3,    4,   10,    0,    0, } , // 2 pipes (1-2 PKRs) 16 bpe @ SW_4K_D_X
    {   3,    0,   11,    0,    0, } , // 4 pipes (1-2 PKRs) 1 bpe @ SW_4K_D_X
    {   3,    1,   12,    0,    0, } , // 4 pipes (1-2 PKRs) 2 bpe @ SW_4K_D_X
    {   3,    2,   13,    0,    0, } , // 4 pipes (1-2 PKRs) 4 bpe @ SW_4K_D_X
    {   3,    3,   14,    0,    0, } , // 4 pipes (1-2 PKRs) 8 bpe @ SW_4K_D_X
    {   3,    4,   15,    0,    0, } , // 4 pipes (1-2 PKRs) 16 bpe @ SW_4K_D_X
    {   3,    0,   16,    0,    0, } , // 8 pipes (2 PKRs) 1 bpe @ SW_4K_D_X
    {   3,    1,   17,    0,    0, } , // 8 pipes (2 PKRs) 2 bpe @ SW_4K_D_X
    {   3,    2,   18,    0,    0, } , // 8 pipes (2 PKRs) 4 bpe @ SW_4K_D_X
    {   3,    3,   19,    0,    0, } , // 8 pipes (2 PKRs) 8 bpe @ SW_4K_D_X
    {   3,    4,   20,    0,    0, } , // 8 pipes (2 PKRs) 16 bpe @ SW_4K_D_X
    {   3,    0,   21,    0,    0, } , // 4 pipes (4 PKRs) 1 bpe @ SW_4K_D_X
    {   3,    1,   22,    0,    0, } , // 4 pipes (4 PKRs) 2 bpe @ SW_4K_D_X
    {   3,    2,   23,    0,    0, } , // 4 pipes (4 PKRs) 4 bpe @ SW_4K_D_X
    {   3,    3,   24,    0,    0, } , // 4 pipes (4 PKRs) 8 bpe @ SW_4K_D_X
    {   3,    4,   25,    0,    0, } , // 4 pipes (4 PKRs) 16 bpe @ SW_4K_D_X
    {   3,    0,   26,    0,    0, } , // 8 pipes (4 PKRs) 1 bpe @ SW_4K_D_X
    {   3,    1,   27,    0,    0, } , // 8 pipes (4 PKRs) 2 bpe @ SW_4K_D_X
    {   3,    2,   28,    0,    0, } , // 8 pipes (4 PKRs) 4 bpe @ SW_4K_D_X
    {   3,    3,   29,    0,    0, } , // 8 pipes (4 PKRs) 8 bpe @ SW_4K_D_X
    {   3,    4,   30,    0,    0, } , // 8 pipes (4 PKRs) 16 bpe @ SW_4K_D_X
    {   3,    0,   31,    0,    0, } , // 16 pipes (4 PKRs) 1 bpe @ SW_4K_D_X
    {   3,    1,   32,    0,    0, } , // 16 pipes (4 PKRs) 2 bpe @ SW_4K_D_X
    {   3,    2,   33,    0,    0, } , // 16 pipes (4 PKRs) 4 bpe @ SW_4K_D_X
    {   3,    3,   34,    0,    0, } , // 16 pipes (4 PKRs) 8 bpe @ SW_4K_D_X
    {   3,    4,   35,    0,    0, } , // 16 pipes (4 PKRs) 16 bpe @ SW_4K_D_X
    {   3,    0,   36,    0,    0, } , // 8 pipes (8 PKRs) 1 bpe @ SW_4K_D_X
    {   3,    1,   37,    0,    0, } , // 8 pipes (8 PKRs) 2 bpe @ SW_4K_D_X
    {   3,    2,   38,    0,    0, } , // 8 pipes (8 PKRs) 4 bpe @ SW_4K_D_X
    {   3,    3,   39,    0,    0, } , // 8 pipes (8 PKRs) 8 bpe @ SW_4K_D_X
    {   3,    4,   40,    0,    0, } , // 8 pipes (8 PKRs) 16 bpe @ SW_4K_D_X
    {   3,    0,   41,    0,    0, } , // 16 pipes (8 PKRs) 1 bpe @ SW_4K_D_X
    {   3,    1,   42,    0,    0, } , // 16 pipes (8 PKRs) 2 bpe @ SW_4K_D_X
    {   3,    2,   43,    0,    0, } , // 16 pipes (8 PKRs) 4 bpe @ SW_4K_D_X
    {   3,    3,   44,    0,    0, } , // 16 pipes (8 PKRs) 8 bpe @ SW_4K_D_X
    {   3,    4,   45,    0,    0, } , // 16 pipes (8 PKRs) 16 bpe @ SW_4K_D_X
    {   3,    0,   46,    0,    0, } , // 32 pipes (8 PKRs) 1 bpe @ SW_4K_D_X
    {   3,    1,   47,    0,    0, } , // 32 pipes (8 PKRs) 2 bpe @ SW_4K_D_X
    {   3,    2,   48,    0,    0, } , // 32 pipes (8 PKRs) 4 bpe @ SW_4K_D_X
    {   3,    3,   49,    0,    0, } , // 32 pipes (8 PKRs) 8 bpe @ SW_4K_D_X
    {   3,    4,   50,    0,    0, } , // 32 pipes (8 PKRs) 16 bpe @ SW_4K_D_X
    {   3,    0,   51,    0,    0, } , // 16 pipes (16 PKRs) 1 bpe @ SW_4K_D_X
    {   3,    1,   52,    0,    0, } , // 16 pipes (16 PKRs) 2 bpe @ SW_4K_D_X
    {   3,    2,   53,    0,    0, } , // 16 pipes (16 PKRs) 4 bpe @ SW_4K_D_X
    {   3,    3,   54,    0,    0, } , // 16 pipes (16 PKRs) 8 bpe @ SW_4K_D_X
    {   3,    4,   55,    0,    0, } , // 16 pipes (16 PKRs) 16 bpe @ SW_4K_D_X
    {   3,    0,   56,    0,    0, } , // 32 pipes (16 PKRs) 1 bpe @ SW_4K_D_X
    {   3,    1,   57,    0,    0, } , // 32 pipes (16 PKRs) 2 bpe @ SW_4K_D_X
    {   3,    2,   58,    0,    0, } , // 32 pipes (16 PKRs) 4 bpe @ SW_4K_D_X
    {   3,    3,   59,    0,    0, } , // 32 pipes (16 PKRs) 8 bpe @ SW_4K_D_X
    {   3,    4,   60,    0,    0, } , // 32 pipes (16 PKRs) 16 bpe @ SW_4K_D_X
    {   3,    0,   61,    0,    0, } , // 64 pipes (16 PKRs) 1 bpe @ SW_4K_D_X
    {   3,    1,   62,    0,    0, } , // 64 pipes (16 PKRs) 2 bpe @ SW_4K_D_X
    {   3,    2,   63,    0,    0, } , // 64 pipes (16 PKRs) 4 bpe @ SW_4K_D_X
    {   3,    3,   64,    0,    0, } , // 64 pipes (16 PKRs) 8 bpe @ SW_4K_D_X
    {   3,    4,   65,    0,    0, } , // 64 pipes (16 PKRs) 16 bpe @ SW_4K_D_X
    {   3,    0,   51,    0,    0, } , // 32 pipes (32 PKRs) 1 bpe @ SW_4K_D_X
    {   3,    1,   52,    0,    0, } , // 32 pipes (32 PKRs) 2 bpe @ SW_4K_D_X
    {   3,    2,   53,    0,    0, } , // 32 pipes (32 PKRs) 4 bpe @ SW_4K_D_X
    {   3,    3,   54,    0,    0, } , // 32 pipes (32 PKRs) 8 bpe @ SW_4K_D_X
    {   3,    4,   55,    0,    0, } , // 32 pipes (32 PKRs) 16 bpe @ SW_4K_D_X
    {   3,    0,   56,    0,    0, } , // 64 pipes (32 PKRs) 1 bpe @ SW_4K_D_X
    {   3,    1,   57,    0,    0, } , // 64 pipes (32 PKRs) 2 bpe @ SW_4K_D_X
    {   3,    2,   58,    0,    0, } , // 64 pipes (32 PKRs) 4 bpe @ SW_4K_D_X
    {   3,    3,   59,    0,    0, } , // 64 pipes (32 PKRs) 8 bpe @ SW_4K_D_X
    {   3,    4,   60,    0,    0, } , // 64 pipes (32 PKRs) 16 bpe @ SW_4K_D_X
};

const ADDR_SW_PATINFO GFX11_SW_64K_D_PATINFO[] =
{
    {   1,    0,    1,    1,    0, } , // 1 pipes (1 PKRs) 1 bpe @ SW_64K_D
    {   1,    1,    2,    2,    0, } , // 1 pipes (1 PKRs) 2 bpe @ SW_64K_D
    {   1,    2,    3,    3,    0, } , // 1 pipes (1 PKRs) 4 bpe @ SW_64K_D
    {   1,    3,    4,    4,    0, } , // 1 pipes (1 PKRs) 8 bpe @ SW_64K_D
    {   1,    4,    5,    5,    0, } , // 1 pipes (1 PKRs) 16 bpe @ SW_64K_D
    {   1,    0,    1,    1,    0, } , // 2 pipes (1-2 PKRs) 1 bpe @ SW_64K_D
    {   1,    1,    2,    2,    0, } , // 2 pipes (1-2 PKRs) 2 bpe @ SW_64K_D
    {   1,    2,    3,    3,    0, } , // 2 pipes (1-2 PKRs) 4 bpe @ SW_64K_D
    {   1,    3,    4,    4,    0, } , // 2 pipes (1-2 PKRs) 8 bpe @ SW_64K_D
    {   1,    4,    5,    5,    0, } , // 2 pipes (1-2 PKRs) 16 bpe @ SW_64K_D
    {   1,    0,    1,    1,    0, } , // 4 pipes (1-2 PKRs) 1 bpe @ SW_64K_D
    {   1,    1,    2,    2,    0, } , // 4 pipes (1-2 PKRs) 2 bpe @ SW_64K_D
    {   1,    2,    3,    3,    0, } , // 4 pipes (1-2 PKRs) 4 bpe @ SW_64K_D
    {   1,    3,    4,    4,    0, } , // 4 pipes (1-2 PKRs) 8 bpe @ SW_64K_D
    {   1,    4,    5,    5,    0, } , // 4 pipes (1-2 PKRs) 16 bpe @ SW_64K_D
    {   1,    0,    1,    1,    0, } , // 8 pipes (2 PKRs) 1 bpe @ SW_64K_D
    {   1,    1,    2,    2,    0, } , // 8 pipes (2 PKRs) 2 bpe @ SW_64K_D
    {   1,    2,    3,    3,    0, } , // 8 pipes (2 PKRs) 4 bpe @ SW_64K_D
    {   1,    3,    4,    4,    0, } , // 8 pipes (2 PKRs) 8 bpe @ SW_64K_D
    {   1,    4,    5,    5,    0, } , // 8 pipes (2 PKRs) 16 bpe @ SW_64K_D
    {   1,    0,    1,    1,    0, } , // 4 pipes (4 PKRs) 1 bpe @ SW_64K_D
    {   1,    1,    2,    2,    0, } , // 4 pipes (4 PKRs) 2 bpe @ SW_64K_D
    {   1,    2,    3,    3,    0, } , // 4 pipes (4 PKRs) 4 bpe @ SW_64K_D
    {   1,    3,    4,    4,    0, } , // 4 pipes (4 PKRs) 8 bpe @ SW_64K_D
    {   1,    4,    5,    5,    0, } , // 4 pipes (4 PKRs) 16 bpe @ SW_64K_D
    {   1,    0,    1,    1,    0, } , // 8 pipes (4 PKRs) 1 bpe @ SW_64K_D
    {   1,    1,    2,    2,    0, } , // 8 pipes (4 PKRs) 2 bpe @ SW_64K_D
    {   1,    2,    3,    3,    0, } , // 8 pipes (4 PKRs) 4 bpe @ SW_64K_D
    {   1,    3,    4,    4,    0, } , // 8 pipes (4 PKRs) 8 bpe @ SW_64K_D
    {   1,    4,    5,    5,    0, } , // 8 pipes (4 PKRs) 16 bpe @ SW_64K_D
    {   1,    0,    1,    1,    0, } , // 16 pipes (4 PKRs) 1 bpe @ SW_64K_D
    {   1,    1,    2,    2,    0, } , // 16 pipes (4 PKRs) 2 bpe @ SW_64K_D
    {   1,    2,    3,    3,    0, } , // 16 pipes (4 PKRs) 4 bpe @ SW_64K_D
    {   1,    3,    4,    4,    0, } , // 16 pipes (4 PKRs) 8 bpe @ SW_64K_D
    {   1,    4,    5,    5,    0, } , // 16 pipes (4 PKRs) 16 bpe @ SW_64K_D
    {   1,    0,    1,    1,    0, } , // 8 pipes (8 PKRs) 1 bpe @ SW_64K_D
    {   1,    1,    2,    2,    0, } , // 8 pipes (8 PKRs) 2 bpe @ SW_64K_D
    {   1,    2,    3,    3,    0, } , // 8 pipes (8 PKRs) 4 bpe @ SW_64K_D
    {   1,    3,    4,    4,    0, } , // 8 pipes (8 PKRs) 8 bpe @ SW_64K_D
    {   1,    4,    5,    5,    0, } , // 8 pipes (8 PKRs) 16 bpe @ SW_64K_D
    {   1,    0,    1,    1,    0, } , // 16 pipes (8 PKRs) 1 bpe @ SW_64K_D
    {   1,    1,    2,    2,    0, } , // 16 pipes (8 PKRs) 2 bpe @ SW_64K_D
    {   1,    2,    3,    3,    0, } , // 16 pipes (8 PKRs) 4 bpe @ SW_64K_D
    {   1,    3,    4,    4,    0, } , // 16 pipes (8 PKRs) 8 bpe @ SW_64K_D
    {   1,    4,    5,    5,    0, } , // 16 pipes (8 PKRs) 16 bpe @ SW_64K_D
    {   1,    0,    1,    1,    0, } , // 32 pipes (8 PKRs) 1 bpe @ SW_64K_D
    {   1,    1,    2,    2,    0, } , // 32 pipes (8 PKRs) 2 bpe @ SW_64K_D
    {   1,    2,    3,    3,    0, } , // 32 pipes (8 PKRs) 4 bpe @ SW_64K_D
    {   1,    3,    4,    4,    0, } , // 32 pipes (8 PKRs) 8 bpe @ SW_64K_D
    {   1,    4,    5,    5,    0, } , // 32 pipes (8 PKRs) 16 bpe @ SW_64K_D
    {   1,    0,    1,    1,    0, } , // 16 pipes (16 PKRs) 1 bpe @ SW_64K_D
    {   1,    1,    2,    2,    0, } , // 16 pipes (16 PKRs) 2 bpe @ SW_64K_D
    {   1,    2,    3,    3,    0, } , // 16 pipes (16 PKRs) 4 bpe @ SW_64K_D
    {   1,    3,    4,    4,    0, } , // 16 pipes (16 PKRs) 8 bpe @ SW_64K_D
    {   1,    4,    5,    5,    0, } , // 16 pipes (16 PKRs) 16 bpe @ SW_64K_D
    {   1,    0,    1,    1,    0, } , // 32 pipes (16 PKRs) 1 bpe @ SW_64K_D
    {   1,    1,    2,    2,    0, } , // 32 pipes (16 PKRs) 2 bpe @ SW_64K_D
    {   1,    2,    3,    3,    0, } , // 32 pipes (16 PKRs) 4 bpe @ SW_64K_D
    {   1,    3,    4,    4,    0, } , // 32 pipes (16 PKRs) 8 bpe @ SW_64K_D
    {   1,    4,    5,    5,    0, } , // 32 pipes (16 PKRs) 16 bpe @ SW_64K_D
    {   1,    0,    1,    1,    0, } , // 64 pipes (16 PKRs) 1 bpe @ SW_64K_D
    {   1,    1,    2,    2,    0, } , // 64 pipes (16 PKRs) 2 bpe @ SW_64K_D
    {   1,    2,    3,    3,    0, } , // 64 pipes (16 PKRs) 4 bpe @ SW_64K_D
    {   1,    3,    4,    4,    0, } , // 64 pipes (16 PKRs) 8 bpe @ SW_64K_D
    {   1,    4,    5,    5,    0, } , // 64 pipes (16 PKRs) 16 bpe @ SW_64K_D
    {   1,    0,    1,    1,    0, } , // 32 pipes (32 PKRs) 1 bpe @ SW_64K_D
    {   1,    1,    2,    2,    0, } , // 32 pipes (32 PKRs) 2 bpe @ SW_64K_D
    {   1,    2,    3,    3,    0, } , // 32 pipes (32 PKRs) 4 bpe @ SW_64K_D
    {   1,    3,    4,    4,    0, } , // 32 pipes (32 PKRs) 8 bpe @ SW_64K_D
    {   1,    4,    5,    5,    0, } , // 32 pipes (32 PKRs) 16 bpe @ SW_64K_D
    {   1,    0,    1,    1,    0, } , // 64 pipes (32 PKRs) 1 bpe @ SW_64K_D
    {   1,    1,    2,    2,    0, } , // 64 pipes (32 PKRs) 2 bpe @ SW_64K_D
    {   1,    2,    3,    3,    0, } , // 64 pipes (32 PKRs) 4 bpe @ SW_64K_D
    {   1,    3,    4,    4,    0, } , // 64 pipes (32 PKRs) 8 bpe @ SW_64K_D
    {   1,    4,    5,    5,    0, } , // 64 pipes (32 PKRs) 16 bpe @ SW_64K_D
};

const ADDR_SW_PATINFO GFX11_SW_64K_D_X_PATINFO[] =
{
    {   1,    0,    1,    1,    0, } , // 1 pipes (1 PKRs) 1 bpe @ SW_64K_D_X
    {   1,    1,    2,    2,    0, } , // 1 pipes (1 PKRs) 2 bpe @ SW_64K_D_X
    {   1,    2,    3,    3,    0, } , // 1 pipes (1 PKRs) 4 bpe @ SW_64K_D_X
    {   1,    3,    4,    4,    0, } , // 1 pipes (1 PKRs) 8 bpe @ SW_64K_D_X
    {   1,    4,    5,    5,    0, } , // 1 pipes (1 PKRs) 16 bpe @ SW_64K_D_X
    {   3,    0,    6,    1,    0, } , // 2 pipes (1-2 PKRs) 1 bpe @ SW_64K_D_X
    {   3,    1,    7,    2,    0, } , // 2 pipes (1-2 PKRs) 2 bpe @ SW_64K_D_X
    {   3,    2,    8,    3,    0, } , // 2 pipes (1-2 PKRs) 4 bpe @ SW_64K_D_X
    {   3,    3,    9,    4,    0, } , // 2 pipes (1-2 PKRs) 8 bpe @ SW_64K_D_X
    {   3,    4,   10,    5,    0, } , // 2 pipes (1-2 PKRs) 16 bpe @ SW_64K_D_X
    {   3,    0,   11,    1,    0, } , // 4 pipes (1-2 PKRs) 1 bpe @ SW_64K_D_X
    {   3,    1,   12,    2,    0, } , // 4 pipes (1-2 PKRs) 2 bpe @ SW_64K_D_X
    {   3,    2,   13,    3,    0, } , // 4 pipes (1-2 PKRs) 4 bpe @ SW_64K_D_X
    {   3,    3,   14,    4,    0, } , // 4 pipes (1-2 PKRs) 8 bpe @ SW_64K_D_X
    {   3,    4,   15,    5,    0, } , // 4 pipes (1-2 PKRs) 16 bpe @ SW_64K_D_X
    {   3,    0,   16,    1,    0, } , // 8 pipes (2 PKRs) 1 bpe @ SW_64K_D_X
    {   3,    1,   17,    2,    0, } , // 8 pipes (2 PKRs) 2 bpe @ SW_64K_D_X
    {   3,    2,   18,    3,    0, } , // 8 pipes (2 PKRs) 4 bpe @ SW_64K_D_X
    {   3,    3,   19,    4,    0, } , // 8 pipes (2 PKRs) 8 bpe @ SW_64K_D_X
    {   3,    4,   20,    5,    0, } , // 8 pipes (2 PKRs) 16 bpe @ SW_64K_D_X
    {   3,    0,   21,    1,    0, } , // 4 pipes (4 PKRs) 1 bpe @ SW_64K_D_X
    {   3,    1,   22,    2,    0, } , // 4 pipes (4 PKRs) 2 bpe @ SW_64K_D_X
    {   3,    2,   23,    3,    0, } , // 4 pipes (4 PKRs) 4 bpe @ SW_64K_D_X
    {   3,    3,   24,    4,    0, } , // 4 pipes (4 PKRs) 8 bpe @ SW_64K_D_X
    {   3,    4,   25,    5,    0, } , // 4 pipes (4 PKRs) 16 bpe @ SW_64K_D_X
    {   3,    0,   26,    1,    0, } , // 8 pipes (4 PKRs) 1 bpe @ SW_64K_D_X
    {   3,    1,   27,    2,    0, } , // 8 pipes (4 PKRs) 2 bpe @ SW_64K_D_X
    {   3,    2,   28,    3,    0, } , // 8 pipes (4 PKRs) 4 bpe @ SW_64K_D_X
    {   3,    3,   29,    4,    0, } , // 8 pipes (4 PKRs) 8 bpe @ SW_64K_D_X
    {   3,    4,   30,    5,    0, } , // 8 pipes (4 PKRs) 16 bpe @ SW_64K_D_X
    {   3,    0,   31,    1,    0, } , // 16 pipes (4 PKRs) 1 bpe @ SW_64K_D_X
    {   3,    1,   32,    2,    0, } , // 16 pipes (4 PKRs) 2 bpe @ SW_64K_D_X
    {   3,    2,   33,    3,    0, } , // 16 pipes (4 PKRs) 4 bpe @ SW_64K_D_X
    {   3,    3,   34,    4,    0, } , // 16 pipes (4 PKRs) 8 bpe @ SW_64K_D_X
    {   3,    4,   35,    5,    0, } , // 16 pipes (4 PKRs) 16 bpe @ SW_64K_D_X
    {   3,    0,   36,    1,    0, } , // 8 pipes (8 PKRs) 1 bpe @ SW_64K_D_X
    {   3,    1,   37,    2,    0, } , // 8 pipes (8 PKRs) 2 bpe @ SW_64K_D_X
    {   3,    2,   38,    3,    0, } , // 8 pipes (8 PKRs) 4 bpe @ SW_64K_D_X
    {   3,    3,   39,    4,    0, } , // 8 pipes (8 PKRs) 8 bpe @ SW_64K_D_X
    {   3,    4,   40,    5,    0, } , // 8 pipes (8 PKRs) 16 bpe @ SW_64K_D_X
    {   3,    0,   41,    1,    0, } , // 16 pipes (8 PKRs) 1 bpe @ SW_64K_D_X
    {   3,    1,   42,    2,    0, } , // 16 pipes (8 PKRs) 2 bpe @ SW_64K_D_X
    {   3,    2,   43,    3,    0, } , // 16 pipes (8 PKRs) 4 bpe @ SW_64K_D_X
    {   3,    3,   44,    4,    0, } , // 16 pipes (8 PKRs) 8 bpe @ SW_64K_D_X
    {   3,    4,   45,    5,    0, } , // 16 pipes (8 PKRs) 16 bpe @ SW_64K_D_X
    {   3,    0,   66,    6,    0, } , // 32 pipes (8 PKRs) 1 bpe @ SW_64K_D_X
    {   3,    1,   67,    7,    0, } , // 32 pipes (8 PKRs) 2 bpe @ SW_64K_D_X
    {   3,    2,   68,    8,    0, } , // 32 pipes (8 PKRs) 4 bpe @ SW_64K_D_X
    {   3,    3,   69,    9,    0, } , // 32 pipes (8 PKRs) 8 bpe @ SW_64K_D_X
    {   3,    4,   70,   10,    0, } , // 32 pipes (8 PKRs) 16 bpe @ SW_64K_D_X
    {   3,    0,   51,    1,    0, } , // 16 pipes (16 PKRs) 1 bpe @ SW_64K_D_X
    {   3,    1,   52,    2,    0, } , // 16 pipes (16 PKRs) 2 bpe @ SW_64K_D_X
    {   3,    2,   53,    3,    0, } , // 16 pipes (16 PKRs) 4 bpe @ SW_64K_D_X
    {   3,    3,   54,    4,    0, } , // 16 pipes (16 PKRs) 8 bpe @ SW_64K_D_X
    {   3,    4,   55,    5,    0, } , // 16 pipes (16 PKRs) 16 bpe @ SW_64K_D_X
    {   3,    0,   71,    6,    0, } , // 32 pipes (16 PKRs) 1 bpe @ SW_64K_D_X
    {   3,    1,   72,    7,    0, } , // 32 pipes (16 PKRs) 2 bpe @ SW_64K_D_X
    {   3,    2,   73,    8,    0, } , // 32 pipes (16 PKRs) 4 bpe @ SW_64K_D_X
    {   3,    3,   74,    9,    0, } , // 32 pipes (16 PKRs) 8 bpe @ SW_64K_D_X
    {   3,    4,   75,   10,    0, } , // 32 pipes (16 PKRs) 16 bpe @ SW_64K_D_X
    {   3,    0,   76,   11,    0, } , // 64 pipes (16 PKRs) 1 bpe @ SW_64K_D_X
    {   3,    1,   77,   12,    0, } , // 64 pipes (16 PKRs) 2 bpe @ SW_64K_D_X
    {   3,    2,   78,   13,    0, } , // 64 pipes (16 PKRs) 4 bpe @ SW_64K_D_X
    {   3,    3,   79,   14,    0, } , // 64 pipes (16 PKRs) 8 bpe @ SW_64K_D_X
    {   3,    4,   80,   15,    0, } , // 64 pipes (16 PKRs) 16 bpe @ SW_64K_D_X
    {   3,    0,   81,    6,    0, } , // 32 pipes (32 PKRs) 1 bpe @ SW_64K_D_X
    {   3,    1,   82,    7,    0, } , // 32 pipes (32 PKRs) 2 bpe @ SW_64K_D_X
    {   3,    2,   83,    8,    0, } , // 32 pipes (32 PKRs) 4 bpe @ SW_64K_D_X
    {   3,    3,   84,    9,    0, } , // 32 pipes (32 PKRs) 8 bpe @ SW_64K_D_X
    {   3,    4,   85,   10,    0, } , // 32 pipes (32 PKRs) 16 bpe @ SW_64K_D_X
    {   3,    0,   86,   11,    0, } , // 64 pipes (32 PKRs) 1 bpe @ SW_64K_D_X
    {   3,    1,   87,   12,    0, } , // 64 pipes (32 PKRs) 2 bpe @ SW_64K_D_X
    {   3,    2,   88,   13,    0, } , // 64 pipes (32 PKRs) 4 bpe @ SW_64K_D_X
    {   3,    3,   89,   14,    0, } , // 64 pipes (32 PKRs) 8 bpe @ SW_64K_D_X
    {   3,    4,   90,   15,    0, } , // 64 pipes (32 PKRs) 16 bpe @ SW_64K_D_X
};

const ADDR_SW_PATINFO GFX11_SW_64K_D_T_PATINFO[] =
{
    {   1,    0,    1,    1,    0, } , // 1 pipes (1 PKRs) 1 bpe @ SW_64K_D_T
    {   1,    1,    2,    2,    0, } , // 1 pipes (1 PKRs) 2 bpe @ SW_64K_D_T
    {   1,    2,    3,    3,    0, } , // 1 pipes (1 PKRs) 4 bpe @ SW_64K_D_T
    {   1,    3,    4,    4,    0, } , // 1 pipes (1 PKRs) 8 bpe @ SW_64K_D_T
    {   1,    4,    5,    5,    0, } , // 1 pipes (1 PKRs) 16 bpe @ SW_64K_D_T
    {   2,    0,   91,    1,    0, } , // 2 pipes (1-2 PKRs) 1 bpe @ SW_64K_D_T
    {   2,    1,   92,    2,    0, } , // 2 pipes (1-2 PKRs) 2 bpe @ SW_64K_D_T
    {   2,    2,   93,    3,    0, } , // 2 pipes (1-2 PKRs) 4 bpe @ SW_64K_D_T
    {   2,    3,   94,    4,    0, } , // 2 pipes (1-2 PKRs) 8 bpe @ SW_64K_D_T
    {   2,    4,   95,    5,    0, } , // 2 pipes (1-2 PKRs) 16 bpe @ SW_64K_D_T
    {   2,    0,   96,    1,    0, } , // 4 pipes (1-2 PKRs) 1 bpe @ SW_64K_D_T
    {   2,    1,   97,    2,    0, } , // 4 pipes (1-2 PKRs) 2 bpe @ SW_64K_D_T
    {   2,    2,   98,    3,    0, } , // 4 pipes (1-2 PKRs) 4 bpe @ SW_64K_D_T
    {   2,    3,   99,    4,    0, } , // 4 pipes (1-2 PKRs) 8 bpe @ SW_64K_D_T
    {   2,    4,  100,    5,    0, } , // 4 pipes (1-2 PKRs) 16 bpe @ SW_64K_D_T
    {   2,    0,  101,    1,    0, } , // 8 pipes (2 PKRs) 1 bpe @ SW_64K_D_T
    {   2,    1,  102,    2,    0, } , // 8 pipes (2 PKRs) 2 bpe @ SW_64K_D_T
    {   2,    2,  103,    3,    0, } , // 8 pipes (2 PKRs) 4 bpe @ SW_64K_D_T
    {   2,    3,  104,    4,    0, } , // 8 pipes (2 PKRs) 8 bpe @ SW_64K_D_T
    {   2,    4,  105,    5,    0, } , // 8 pipes (2 PKRs) 16 bpe @ SW_64K_D_T
    {   2,    0,   96,    1,    0, } , // 4 pipes (4 PKRs) 1 bpe @ SW_64K_D_T
    {   2,    1,   97,    2,    0, } , // 4 pipes (4 PKRs) 2 bpe @ SW_64K_D_T
    {   2,    2,   98,    3,    0, } , // 4 pipes (4 PKRs) 4 bpe @ SW_64K_D_T
    {   2,    3,   99,    4,    0, } , // 4 pipes (4 PKRs) 8 bpe @ SW_64K_D_T
    {   2,    4,  100,    5,    0, } , // 4 pipes (4 PKRs) 16 bpe @ SW_64K_D_T
    {   2,    0,  101,    1,    0, } , // 8 pipes (4 PKRs) 1 bpe @ SW_64K_D_T
    {   2,    1,  102,    2,    0, } , // 8 pipes (4 PKRs) 2 bpe @ SW_64K_D_T
    {   2,    2,  103,    3,    0, } , // 8 pipes (4 PKRs) 4 bpe @ SW_64K_D_T
    {   2,    3,  104,    4,    0, } , // 8 pipes (4 PKRs) 8 bpe @ SW_64K_D_T
    {   2,    4,  105,    5,    0, } , // 8 pipes (4 PKRs) 16 bpe @ SW_64K_D_T
    {   2,    0,  106,    1,    0, } , // 16 pipes (4 PKRs) 1 bpe @ SW_64K_D_T
    {   2,    1,  107,    2,    0, } , // 16 pipes (4 PKRs) 2 bpe @ SW_64K_D_T
    {   2,    2,  108,    3,    0, } , // 16 pipes (4 PKRs) 4 bpe @ SW_64K_D_T
    {   2,    3,  109,    4,    0, } , // 16 pipes (4 PKRs) 8 bpe @ SW_64K_D_T
    {   2,    4,  110,    5,    0, } , // 16 pipes (4 PKRs) 16 bpe @ SW_64K_D_T
    {   2,    0,  101,    1,    0, } , // 8 pipes (8 PKRs) 1 bpe @ SW_64K_D_T
    {   2,    1,  102,    2,    0, } , // 8 pipes (8 PKRs) 2 bpe @ SW_64K_D_T
    {   2,    2,  103,    3,    0, } , // 8 pipes (8 PKRs) 4 bpe @ SW_64K_D_T
    {   2,    3,  104,    4,    0, } , // 8 pipes (8 PKRs) 8 bpe @ SW_64K_D_T
    {   2,    4,  105,    5,    0, } , // 8 pipes (8 PKRs) 16 bpe @ SW_64K_D_T
    {   2,    0,  106,    1,    0, } , // 16 pipes (8 PKRs) 1 bpe @ SW_64K_D_T
    {   2,    1,  107,    2,    0, } , // 16 pipes (8 PKRs) 2 bpe @ SW_64K_D_T
    {   2,    2,  108,    3,    0, } , // 16 pipes (8 PKRs) 4 bpe @ SW_64K_D_T
    {   2,    3,  109,    4,    0, } , // 16 pipes (8 PKRs) 8 bpe @ SW_64K_D_T
    {   2,    4,  110,    5,    0, } , // 16 pipes (8 PKRs) 16 bpe @ SW_64K_D_T
    {   2,    0,  111,   16,    0, } , // 32 pipes (8 PKRs) 1 bpe @ SW_64K_D_T
    {   2,    1,  112,   17,    0, } , // 32 pipes (8 PKRs) 2 bpe @ SW_64K_D_T
    {   2,    2,  113,   18,    0, } , // 32 pipes (8 PKRs) 4 bpe @ SW_64K_D_T
    {   2,    3,  114,   19,    0, } , // 32 pipes (8 PKRs) 8 bpe @ SW_64K_D_T
    {   2,    4,  115,   20,    0, } , // 32 pipes (8 PKRs) 16 bpe @ SW_64K_D_T
    {   2,    0,  106,    1,    0, } , // 16 pipes (16 PKRs) 1 bpe @ SW_64K_D_T
    {   2,    1,  107,    2,    0, } , // 16 pipes (16 PKRs) 2 bpe @ SW_64K_D_T
    {   2,    2,  108,    3,    0, } , // 16 pipes (16 PKRs) 4 bpe @ SW_64K_D_T
    {   2,    3,  109,    4,    0, } , // 16 pipes (16 PKRs) 8 bpe @ SW_64K_D_T
    {   2,    4,  110,    5,    0, } , // 16 pipes (16 PKRs) 16 bpe @ SW_64K_D_T
    {   2,    0,  111,   16,    0, } , // 32 pipes (16 PKRs) 1 bpe @ SW_64K_D_T
    {   2,    1,  112,   17,    0, } , // 32 pipes (16 PKRs) 2 bpe @ SW_64K_D_T
    {   2,    2,  113,   18,    0, } , // 32 pipes (16 PKRs) 4 bpe @ SW_64K_D_T
    {   2,    3,  114,   19,    0, } , // 32 pipes (16 PKRs) 8 bpe @ SW_64K_D_T
    {   2,    4,  115,   20,    0, } , // 32 pipes (16 PKRs) 16 bpe @ SW_64K_D_T
    {   2,    0,    1,   21,    0, } , // 64 pipes (16 PKRs) 1 bpe @ SW_64K_D_T
    {   2,    1,    2,   22,    0, } , // 64 pipes (16 PKRs) 2 bpe @ SW_64K_D_T
    {   2,    2,    3,   23,    0, } , // 64 pipes (16 PKRs) 4 bpe @ SW_64K_D_T
    {   2,    3,    4,   24,    0, } , // 64 pipes (16 PKRs) 8 bpe @ SW_64K_D_T
    {   2,    4,    5,   25,    0, } , // 64 pipes (16 PKRs) 16 bpe @ SW_64K_D_T
    {   2,    0,  111,   16,    0, } , // 32 pipes (32 PKRs) 1 bpe @ SW_64K_D_T
    {   2,    1,  112,   17,    0, } , // 32 pipes (32 PKRs) 2 bpe @ SW_64K_D_T
    {   2,    2,  113,   18,    0, } , // 32 pipes (32 PKRs) 4 bpe @ SW_64K_D_T
    {   2,    3,  114,   19,    0, } , // 32 pipes (32 PKRs) 8 bpe @ SW_64K_D_T
    {   2,    4,  115,   20,    0, } , // 32 pipes (32 PKRs) 16 bpe @ SW_64K_D_T
    {   2,    0,    1,   21,    0, } , // 64 pipes (32 PKRs) 1 bpe @ SW_64K_D_T
    {   2,    1,    2,   22,    0, } , // 64 pipes (32 PKRs) 2 bpe @ SW_64K_D_T
    {   2,    2,    3,   23,    0, } , // 64 pipes (32 PKRs) 4 bpe @ SW_64K_D_T
    {   2,    3,    4,   24,    0, } , // 64 pipes (32 PKRs) 8 bpe @ SW_64K_D_T
    {   2,    4,    5,   25,    0, } , // 64 pipes (32 PKRs) 16 bpe @ SW_64K_D_T
};

const ADDR_SW_PATINFO GFX11_SW_256K_D_X_PATINFO[] =
{
    {   1,    0,    1,    1,    1, } , // 1 pipes (1 PKRs) 1 bpe @ SW_256K_D_X
    {   1,    1,    2,    2,    2, } , // 1 pipes (1 PKRs) 2 bpe @ SW_256K_D_X
    {   1,    2,    3,    3,    3, } , // 1 pipes (1 PKRs) 4 bpe @ SW_256K_D_X
    {   1,    3,    4,    4,    4, } , // 1 pipes (1 PKRs) 8 bpe @ SW_256K_D_X
    {   1,    4,    5,    5,    5, } , // 1 pipes (1 PKRs) 16 bpe @ SW_256K_D_X
    {   3,    0,    6,    1,    1, } , // 2 pipes (1-2 PKRs) 1 bpe @ SW_256K_D_X
    {   3,    1,    7,    2,    2, } , // 2 pipes (1-2 PKRs) 2 bpe @ SW_256K_D_X
    {   3,    2,    8,    3,    3, } , // 2 pipes (1-2 PKRs) 4 bpe @ SW_256K_D_X
    {   3,    3,    9,    4,    4, } , // 2 pipes (1-2 PKRs) 8 bpe @ SW_256K_D_X
    {   3,    4,   10,    5,    5, } , // 2 pipes (1-2 PKRs) 16 bpe @ SW_256K_D_X
    {   3,    0,   11,    1,    1, } , // 4 pipes (1-2 PKRs) 1 bpe @ SW_256K_D_X
    {   3,    1,   12,    2,    2, } , // 4 pipes (1-2 PKRs) 2 bpe @ SW_256K_D_X
    {   3,    2,   13,    3,    3, } , // 4 pipes (1-2 PKRs) 4 bpe @ SW_256K_D_X
    {   3,    3,   14,    4,    4, } , // 4 pipes (1-2 PKRs) 8 bpe @ SW_256K_D_X
    {   3,    4,   15,    5,    5, } , // 4 pipes (1-2 PKRs) 16 bpe @ SW_256K_D_X
    {   3,    0,   16,    1,    1, } , // 8 pipes (2 PKRs) 1 bpe @ SW_256K_D_X
    {   3,    1,   17,    2,    2, } , // 8 pipes (2 PKRs) 2 bpe @ SW_256K_D_X
    {   3,    2,   18,    3,    3, } , // 8 pipes (2 PKRs) 4 bpe @ SW_256K_D_X
    {   3,    3,   19,    4,    4, } , // 8 pipes (2 PKRs) 8 bpe @ SW_256K_D_X
    {   3,    4,   20,    5,    5, } , // 8 pipes (2 PKRs) 16 bpe @ SW_256K_D_X
    {   3,    0,   21,    1,    1, } , // 4 pipes (4 PKRs) 1 bpe @ SW_256K_D_X
    {   3,    1,   22,    2,    2, } , // 4 pipes (4 PKRs) 2 bpe @ SW_256K_D_X
    {   3,    2,   23,    3,    3, } , // 4 pipes (4 PKRs) 4 bpe @ SW_256K_D_X
    {   3,    3,   24,    4,    4, } , // 4 pipes (4 PKRs) 8 bpe @ SW_256K_D_X
    {   3,    4,   25,    5,    5, } , // 4 pipes (4 PKRs) 16 bpe @ SW_256K_D_X
    {   3,    0,   26,    1,    1, } , // 8 pipes (4 PKRs) 1 bpe @ SW_256K_D_X
    {   3,    1,   27,    2,    2, } , // 8 pipes (4 PKRs) 2 bpe @ SW_256K_D_X
    {   3,    2,   28,    3,    3, } , // 8 pipes (4 PKRs) 4 bpe @ SW_256K_D_X
    {   3,    3,   29,    4,    4, } , // 8 pipes (4 PKRs) 8 bpe @ SW_256K_D_X
    {   3,    4,   30,    5,    5, } , // 8 pipes (4 PKRs) 16 bpe @ SW_256K_D_X
    {   3,    0,   31,    1,    1, } , // 16 pipes (4 PKRs) 1 bpe @ SW_256K_D_X
    {   3,    1,   32,    2,    2, } , // 16 pipes (4 PKRs) 2 bpe @ SW_256K_D_X
    {   3,    2,   33,    3,    3, } , // 16 pipes (4 PKRs) 4 bpe @ SW_256K_D_X
    {   3,    3,   34,    4,    4, } , // 16 pipes (4 PKRs) 8 bpe @ SW_256K_D_X
    {   3,    4,   35,    5,    5, } , // 16 pipes (4 PKRs) 16 bpe @ SW_256K_D_X
    {   3,    0,   36,    1,    1, } , // 8 pipes (8 PKRs) 1 bpe @ SW_256K_D_X
    {   3,    1,   37,    2,    2, } , // 8 pipes (8 PKRs) 2 bpe @ SW_256K_D_X
    {   3,    2,   38,    3,    3, } , // 8 pipes (8 PKRs) 4 bpe @ SW_256K_D_X
    {   3,    3,   39,    4,    4, } , // 8 pipes (8 PKRs) 8 bpe @ SW_256K_D_X
    {   3,    4,   40,    5,    5, } , // 8 pipes (8 PKRs) 16 bpe @ SW_256K_D_X
    {   3,    0,   41,    1,    1, } , // 16 pipes (8 PKRs) 1 bpe @ SW_256K_D_X
    {   3,    1,   42,    2,    2, } , // 16 pipes (8 PKRs) 2 bpe @ SW_256K_D_X
    {   3,    2,   43,    3,    3, } , // 16 pipes (8 PKRs) 4 bpe @ SW_256K_D_X
    {   3,    3,   44,    4,    4, } , // 16 pipes (8 PKRs) 8 bpe @ SW_256K_D_X
    {   3,    4,   45,    5,    5, } , // 16 pipes (8 PKRs) 16 bpe @ SW_256K_D_X
    {   3,    0,   66,    6,    1, } , // 32 pipes (8 PKRs) 1 bpe @ SW_256K_D_X
    {   3,    1,   67,    7,    2, } , // 32 pipes (8 PKRs) 2 bpe @ SW_256K_D_X
    {   3,    2,   68,    8,    3, } , // 32 pipes (8 PKRs) 4 bpe @ SW_256K_D_X
    {   3,    3,   69,    9,    4, } , // 32 pipes (8 PKRs) 8 bpe @ SW_256K_D_X
    {   3,    4,   70,   10,    5, } , // 32 pipes (8 PKRs) 16 bpe @ SW_256K_D_X
    {   3,    0,   51,    1,    1, } , // 16 pipes (16 PKRs) 1 bpe @ SW_256K_D_X
    {   3,    1,   52,    2,    2, } , // 16 pipes (16 PKRs) 2 bpe @ SW_256K_D_X
    {   3,    2,   53,    3,    3, } , // 16 pipes (16 PKRs) 4 bpe @ SW_256K_D_X
    {   3,    3,   54,    4,    4, } , // 16 pipes (16 PKRs) 8 bpe @ SW_256K_D_X
    {   3,    4,   55,    5,    5, } , // 16 pipes (16 PKRs) 16 bpe @ SW_256K_D_X
    {   3,    0,   71,    6,    1, } , // 32 pipes (16 PKRs) 1 bpe @ SW_256K_D_X
    {   3,    1,   72,    7,    2, } , // 32 pipes (16 PKRs) 2 bpe @ SW_256K_D_X
    {   3,    2,   73,    8,    3, } , // 32 pipes (16 PKRs) 4 bpe @ SW_256K_D_X
    {   3,    3,   74,    9,    4, } , // 32 pipes (16 PKRs) 8 bpe @ SW_256K_D_X
    {   3,    4,   75,   10,    5, } , // 32 pipes (16 PKRs) 16 bpe @ SW_256K_D_X
    {   3,    0,   76,   11,    1, } , // 64 pipes (16 PKRs) 1 bpe @ SW_256K_D_X
    {   3,    1,   77,   12,    2, } , // 64 pipes (16 PKRs) 2 bpe @ SW_256K_D_X
    {   3,    2,   78,   13,    3, } , // 64 pipes (16 PKRs) 4 bpe @ SW_256K_D_X
    {   3,    3,   79,   14,    4, } , // 64 pipes (16 PKRs) 8 bpe @ SW_256K_D_X
    {   3,    4,   80,   15,    5, } , // 64 pipes (16 PKRs) 16 bpe @ SW_256K_D_X
    {   3,    0,   81,    6,    1, } , // 32 pipes (32 PKRs) 1 bpe @ SW_256K_D_X
    {   3,    1,   82,    7,    2, } , // 32 pipes (32 PKRs) 2 bpe @ SW_256K_D_X
    {   3,    2,   83,    8,    3, } , // 32 pipes (32 PKRs) 4 bpe @ SW_256K_D_X
    {   3,    3,   84,    9,    4, } , // 32 pipes (32 PKRs) 8 bpe @ SW_256K_D_X
    {   3,    4,   85,   10,    5, } , // 32 pipes (32 PKRs) 16 bpe @ SW_256K_D_X
    {   3,    0,   86,   11,    1, } , // 64 pipes (32 PKRs) 1 bpe @ SW_256K_D_X
    {   3,    1,   87,   12,    2, } , // 64 pipes (32 PKRs) 2 bpe @ SW_256K_D_X
    {   3,    2,   88,   13,    3, } , // 64 pipes (32 PKRs) 4 bpe @ SW_256K_D_X
    {   3,    3,   89,   14,    4, } , // 64 pipes (32 PKRs) 8 bpe @ SW_256K_D_X
    {   3,    4,   90,   15,    5, } , // 64 pipes (32 PKRs) 16 bpe @ SW_256K_D_X
};

const ADDR_SW_PATINFO GFX11_SW_64K_ZR_X_1xaa_PATINFO[] =
{
    {   2,    0,  116,   26,    0, } , // 1 pipes (1 PKRs) 1 bpe @ SW_64K_{Z,R}_X 1xaa
    {   2,    1,  117,   22,    0, } , // 1 pipes (1 PKRs) 2 bpe @ SW_64K_{Z,R}_X 1xaa
    {   2,    2,  118,   27,    0, } , // 1 pipes (1 PKRs) 4 bpe @ SW_64K_{Z,R}_X 1xaa
    {   2,    3,  119,   28,    0, } , // 1 pipes (1 PKRs) 8 bpe @ SW_64K_{Z,R}_X 1xaa
    {   2,    4,  120,   29,    0, } , // 1 pipes (1 PKRs) 16 bpe @ SW_64K_{Z,R}_X 1xaa
    {   3,    0,  121,   30,    0, } , // 2 pipes (1-2 PKRs) 1 bpe @ SW_64K_{Z,R}_X 1xaa
    {   3,    1,  122,   31,    0, } , // 2 pipes (1-2 PKRs) 2 bpe @ SW_64K_{Z,R}_X 1xaa
    {   3,    2,  123,   32,    0, } , // 2 pipes (1-2 PKRs) 4 bpe @ SW_64K_{Z,R}_X 1xaa
    {   3,    3,  124,   33,    0, } , // 2 pipes (1-2 PKRs) 8 bpe @ SW_64K_{Z,R}_X 1xaa
    {   3,    4,  125,   34,    0, } , // 2 pipes (1-2 PKRs) 16 bpe @ SW_64K_{Z,R}_X 1xaa
    {   3,    0,  126,   35,    0, } , // 4 pipes (1-2 PKRs) 1 bpe @ SW_64K_{Z,R}_X 1xaa
    {   3,    1,  127,   36,    0, } , // 4 pipes (1-2 PKRs) 2 bpe @ SW_64K_{Z,R}_X 1xaa
    {   3,    2,  128,   37,    0, } , // 4 pipes (1-2 PKRs) 4 bpe @ SW_64K_{Z,R}_X 1xaa
    {   3,    3,  129,   38,    0, } , // 4 pipes (1-2 PKRs) 8 bpe @ SW_64K_{Z,R}_X 1xaa
    {   3,    4,  130,   39,    0, } , // 4 pipes (1-2 PKRs) 16 bpe @ SW_64K_{Z,R}_X 1xaa
    {   3,    0,  131,   40,    0, } , // 8 pipes (2 PKRs) 1 bpe @ SW_64K_{Z,R}_X 1xaa
    {   3,    1,  132,   41,    0, } , // 8 pipes (2 PKRs) 2 bpe @ SW_64K_{Z,R}_X 1xaa
    {   3,    2,  133,   42,    0, } , // 8 pipes (2 PKRs) 4 bpe @ SW_64K_{Z,R}_X 1xaa
    {   3,    3,  134,   43,    0, } , // 8 pipes (2 PKRs) 8 bpe @ SW_64K_{Z,R}_X 1xaa
    {   3,    4,  135,   44,    0, } , // 8 pipes (2 PKRs) 16 bpe @ SW_64K_{Z,R}_X 1xaa
    {   3,    0,  136,   45,    0, } , // 4 pipes (4 PKRs) 1 bpe @ SW_64K_{Z,R}_X 1xaa
    {   3,    1,  137,   46,    0, } , // 4 pipes (4 PKRs) 2 bpe @ SW_64K_{Z,R}_X 1xaa
    {   3,    2,  138,   47,    0, } , // 4 pipes (4 PKRs) 4 bpe @ SW_64K_{Z,R}_X 1xaa
    {   3,    3,  139,   48,    0, } , // 4 pipes (4 PKRs) 8 bpe @ SW_64K_{Z,R}_X 1xaa
    {   3,    4,  140,   49,    0, } , // 4 pipes (4 PKRs) 16 bpe @ SW_64K_{Z,R}_X 1xaa
    {   3,    0,  141,   40,    0, } , // 8 pipes (4 PKRs) 1 bpe @ SW_64K_{Z,R}_X 1xaa
    {   3,    1,  142,   50,    0, } , // 8 pipes (4 PKRs) 2 bpe @ SW_64K_{Z,R}_X 1xaa
    {   3,    2,  143,   51,    0, } , // 8 pipes (4 PKRs) 4 bpe @ SW_64K_{Z,R}_X 1xaa
    {   3,    3,  144,   52,    0, } , // 8 pipes (4 PKRs) 8 bpe @ SW_64K_{Z,R}_X 1xaa
    {   3,    4,  145,   53,    0, } , // 8 pipes (4 PKRs) 16 bpe @ SW_64K_{Z,R}_X 1xaa
    {   3,    0,  146,   54,    0, } , // 16 pipes (4 PKRs) 1 bpe @ SW_64K_{Z,R}_X 1xaa
    {   3,    1,  146,   55,    0, } , // 16 pipes (4 PKRs) 2 bpe @ SW_64K_{Z,R}_X 1xaa
    {   3,    2,  146,   56,    0, } , // 16 pipes (4 PKRs) 4 bpe @ SW_64K_{Z,R}_X 1xaa
    {   3,    3,  146,   57,    0, } , // 16 pipes (4 PKRs) 8 bpe @ SW_64K_{Z,R}_X 1xaa
    {   3,    4,  146,   58,    0, } , // 16 pipes (4 PKRs) 16 bpe @ SW_64K_{Z,R}_X 1xaa
    {   3,    0,  147,   59,    0, } , // 8 pipes (8 PKRs) 1 bpe @ SW_64K_{Z,R}_X 1xaa
    {   3,    1,  148,   60,    0, } , // 8 pipes (8 PKRs) 2 bpe @ SW_64K_{Z,R}_X 1xaa
    {   3,    2,  149,   61,    0, } , // 8 pipes (8 PKRs) 4 bpe @ SW_64K_{Z,R}_X 1xaa
    {   3,    3,  150,   62,    0, } , // 8 pipes (8 PKRs) 8 bpe @ SW_64K_{Z,R}_X 1xaa
    {   3,    4,  151,   63,    0, } , // 8 pipes (8 PKRs) 16 bpe @ SW_64K_{Z,R}_X 1xaa
    {   3,    0,  152,   54,    0, } , // 16 pipes (8 PKRs) 1 bpe @ SW_64K_{Z,R}_X 1xaa
    {   3,    1,  152,   64,    0, } , // 16 pipes (8 PKRs) 2 bpe @ SW_64K_{Z,R}_X 1xaa
    {   3,    2,  152,   56,    0, } , // 16 pipes (8 PKRs) 4 bpe @ SW_64K_{Z,R}_X 1xaa
    {   3,    3,  153,   57,    0, } , // 16 pipes (8 PKRs) 8 bpe @ SW_64K_{Z,R}_X 1xaa
    {   3,    4,  153,   65,    0, } , // 16 pipes (8 PKRs) 16 bpe @ SW_64K_{Z,R}_X 1xaa
    {   3,    0,  152,   66,    0, } , // 32 pipes (8 PKRs) 1 bpe @ SW_64K_{Z,R}_X 1xaa
    {   3,    1,  152,   67,    0, } , // 32 pipes (8 PKRs) 2 bpe @ SW_64K_{Z,R}_X 1xaa
    {   3,    2,  152,   68,    0, } , // 32 pipes (8 PKRs) 4 bpe @ SW_64K_{Z,R}_X 1xaa
    {   3,    3,  153,   69,    0, } , // 32 pipes (8 PKRs) 8 bpe @ SW_64K_{Z,R}_X 1xaa
    {   3,    4,  153,   70,    0, } , // 32 pipes (8 PKRs) 16 bpe @ SW_64K_{Z,R}_X 1xaa
    {   3,    0,  154,   71,    0, } , // 16 pipes (16 PKRs) 1 bpe @ SW_64K_{Z,R}_X 1xaa
    {   3,    1,  154,   72,    0, } , // 16 pipes (16 PKRs) 2 bpe @ SW_64K_{Z,R}_X 1xaa
    {   3,    2,  154,   73,    0, } , // 16 pipes (16 PKRs) 4 bpe @ SW_64K_{Z,R}_X 1xaa
    {   3,    3,  155,   74,    0, } , // 16 pipes (16 PKRs) 8 bpe @ SW_64K_{Z,R}_X 1xaa
    {   3,    4,  156,   75,    0, } , // 16 pipes (16 PKRs) 16 bpe @ SW_64K_{Z,R}_X 1xaa
    {   3,    0,  154,   76,    0, } , // 32 pipes (16 PKRs) 1 bpe @ SW_64K_{Z,R}_X 1xaa
    {   3,    1,  154,   77,    0, } , // 32 pipes (16 PKRs) 2 bpe @ SW_64K_{Z,R}_X 1xaa
    {   3,    2,  154,   78,    0, } , // 32 pipes (16 PKRs) 4 bpe @ SW_64K_{Z,R}_X 1xaa
    {   3,    3,  155,   79,    0, } , // 32 pipes (16 PKRs) 8 bpe @ SW_64K_{Z,R}_X 1xaa
    {   3,    4,  156,   80,    0, } , // 32 pipes (16 PKRs) 16 bpe @ SW_64K_{Z,R}_X 1xaa
    {   3,    0,  154,   81,    0, } , // 64 pipes (16 PKRs) 1 bpe @ SW_64K_{Z,R}_X 1xaa
    {   3,    1,  154,   82,    0, } , // 64 pipes (16 PKRs) 2 bpe @ SW_64K_{Z,R}_X 1xaa
    {   3,    2,  154,   83,    0, } , // 64 pipes (16 PKRs) 4 bpe @ SW_64K_{Z,R}_X 1xaa
    {   3,    3,  155,   84,    0, } , // 64 pipes (16 PKRs) 8 bpe @ SW_64K_{Z,R}_X 1xaa
    {   3,    4,  156,   85,    0, } , // 64 pipes (16 PKRs) 16 bpe @ SW_64K_{Z,R}_X 1xaa
    {   3,    0,  157,   86,    0, } , // 32 pipes (32 PKRs) 1 bpe @ SW_64K_{Z,R}_X 1xaa
    {   3,    1,  157,   87,    0, } , // 32 pipes (32 PKRs) 2 bpe @ SW_64K_{Z,R}_X 1xaa
    {   3,    2,  157,   88,    0, } , // 32 pipes (32 PKRs) 4 bpe @ SW_64K_{Z,R}_X 1xaa
    {   3,    3,  158,   89,    0, } , // 32 pipes (32 PKRs) 8 bpe @ SW_64K_{Z,R}_X 1xaa
    {   3,    4,  159,   90,    0, } , // 32 pipes (32 PKRs) 16 bpe @ SW_64K_{Z,R}_X 1xaa
    {   3,    0,  157,   91,    0, } , // 64 pipes (32 PKRs) 1 bpe @ SW_64K_{Z,R}_X 1xaa
    {   3,    1,  157,   92,    0, } , // 64 pipes (32 PKRs) 2 bpe @ SW_64K_{Z,R}_X 1xaa
    {   3,    2,  157,   93,    0, } , // 64 pipes (32 PKRs) 4 bpe @ SW_64K_{Z,R}_X 1xaa
    {   3,    3,  158,   94,    0, } , // 64 pipes (32 PKRs) 8 bpe @ SW_64K_{Z,R}_X 1xaa
    {   3,    4,  159,   95,    0, } , // 64 pipes (32 PKRs) 16 bpe @ SW_64K_{Z,R}_X 1xaa
};

const ADDR_SW_PATINFO GFX11_SW_64K_ZR_X_2xaa_PATINFO[] =
{
    {   2,    5,  160,   96,    0, } , // 1 pipes (1 PKRs) 1 bpe @ SW_64K_{Z,R}_X 2xaa
    {   2,    6,  118,   27,    0, } , // 1 pipes (1 PKRs) 2 bpe @ SW_64K_{Z,R}_X 2xaa
    {   2,    7,  161,   97,    0, } , // 1 pipes (1 PKRs) 4 bpe @ SW_64K_{Z,R}_X 2xaa
    {   2,    8,  119,   98,    0, } , // 1 pipes (1 PKRs) 8 bpe @ SW_64K_{Z,R}_X 2xaa
    {   2,    9,  162,   99,    0, } , // 1 pipes (1 PKRs) 16 bpe @ SW_64K_{Z,R}_X 2xaa
    {   3,    5,  163,  100,    0, } , // 2 pipes (1-2 PKRs) 1 bpe @ SW_64K_{Z,R}_X 2xaa
    {   3,    6,  123,   32,    0, } , // 2 pipes (1-2 PKRs) 2 bpe @ SW_64K_{Z,R}_X 2xaa
    {   3,    7,  123,  101,    0, } , // 2 pipes (1-2 PKRs) 4 bpe @ SW_64K_{Z,R}_X 2xaa
    {   3,    8,  164,  102,    0, } , // 2 pipes (1-2 PKRs) 8 bpe @ SW_64K_{Z,R}_X 2xaa
    {   3,    9,  125,  103,    0, } , // 2 pipes (1-2 PKRs) 16 bpe @ SW_64K_{Z,R}_X 2xaa
    {   3,    5,  127,  104,    0, } , // 4 pipes (1-2 PKRs) 1 bpe @ SW_64K_{Z,R}_X 2xaa
    {   3,    6,  128,   37,    0, } , // 4 pipes (1-2 PKRs) 2 bpe @ SW_64K_{Z,R}_X 2xaa
    {   3,    7,  128,  105,    0, } , // 4 pipes (1-2 PKRs) 4 bpe @ SW_64K_{Z,R}_X 2xaa
    {   3,    8,  165,  106,    0, } , // 4 pipes (1-2 PKRs) 8 bpe @ SW_64K_{Z,R}_X 2xaa
    {   3,    9,  130,  107,    0, } , // 4 pipes (1-2 PKRs) 16 bpe @ SW_64K_{Z,R}_X 2xaa
    {   3,    5,  132,  108,    0, } , // 8 pipes (2 PKRs) 1 bpe @ SW_64K_{Z,R}_X 2xaa
    {   3,    6,  133,   51,    0, } , // 8 pipes (2 PKRs) 2 bpe @ SW_64K_{Z,R}_X 2xaa
    {   3,    7,  133,  109,    0, } , // 8 pipes (2 PKRs) 4 bpe @ SW_64K_{Z,R}_X 2xaa
    {   3,    8,  135,  110,    0, } , // 8 pipes (2 PKRs) 8 bpe @ SW_64K_{Z,R}_X 2xaa
    {   3,    9,  135,  111,    0, } , // 8 pipes (2 PKRs) 16 bpe @ SW_64K_{Z,R}_X 2xaa
    {   3,    5,  137,  112,    0, } , // 4 pipes (4 PKRs) 1 bpe @ SW_64K_{Z,R}_X 2xaa
    {   3,    6,  138,   47,    0, } , // 4 pipes (4 PKRs) 2 bpe @ SW_64K_{Z,R}_X 2xaa
    {   3,    7,  138,  113,    0, } , // 4 pipes (4 PKRs) 4 bpe @ SW_64K_{Z,R}_X 2xaa
    {   3,    8,  139,  114,    0, } , // 4 pipes (4 PKRs) 8 bpe @ SW_64K_{Z,R}_X 2xaa
    {   3,    9,  140,  115,    0, } , // 4 pipes (4 PKRs) 16 bpe @ SW_64K_{Z,R}_X 2xaa
    {   3,    5,  142,  108,    0, } , // 8 pipes (4 PKRs) 1 bpe @ SW_64K_{Z,R}_X 2xaa
    {   3,    6,  143,   51,    0, } , // 8 pipes (4 PKRs) 2 bpe @ SW_64K_{Z,R}_X 2xaa
    {   3,    7,  143,  109,    0, } , // 8 pipes (4 PKRs) 4 bpe @ SW_64K_{Z,R}_X 2xaa
    {   3,    8,  144,  116,    0, } , // 8 pipes (4 PKRs) 8 bpe @ SW_64K_{Z,R}_X 2xaa
    {   3,    9,  145,  111,    0, } , // 8 pipes (4 PKRs) 16 bpe @ SW_64K_{Z,R}_X 2xaa
    {   3,    5,  146,  117,    0, } , // 16 pipes (4 PKRs) 1 bpe @ SW_64K_{Z,R}_X 2xaa
    {   3,    6,  146,  118,    0, } , // 16 pipes (4 PKRs) 2 bpe @ SW_64K_{Z,R}_X 2xaa
    {   3,    7,  146,  119,    0, } , // 16 pipes (4 PKRs) 4 bpe @ SW_64K_{Z,R}_X 2xaa
    {   3,    8,  166,  120,    0, } , // 16 pipes (4 PKRs) 8 bpe @ SW_64K_{Z,R}_X 2xaa
    {   3,    9,  167,  121,    0, } , // 16 pipes (4 PKRs) 16 bpe @ SW_64K_{Z,R}_X 2xaa
    {   3,    5,  148,  122,    0, } , // 8 pipes (8 PKRs) 1 bpe @ SW_64K_{Z,R}_X 2xaa
    {   3,    6,  149,   61,    0, } , // 8 pipes (8 PKRs) 2 bpe @ SW_64K_{Z,R}_X 2xaa
    {   3,    7,  149,  123,    0, } , // 8 pipes (8 PKRs) 4 bpe @ SW_64K_{Z,R}_X 2xaa
    {   3,    8,  151,  124,    0, } , // 8 pipes (8 PKRs) 8 bpe @ SW_64K_{Z,R}_X 2xaa
    {   3,    9,  168,  125,    0, } , // 8 pipes (8 PKRs) 16 bpe @ SW_64K_{Z,R}_X 2xaa
    {   3,    5,  152,   55,    0, } , // 16 pipes (8 PKRs) 1 bpe @ SW_64K_{Z,R}_X 2xaa
    {   3,    6,  152,   56,    0, } , // 16 pipes (8 PKRs) 2 bpe @ SW_64K_{Z,R}_X 2xaa
    {   3,    7,  152,  126,    0, } , // 16 pipes (8 PKRs) 4 bpe @ SW_64K_{Z,R}_X 2xaa
    {   3,    8,  153,  127,    0, } , // 16 pipes (8 PKRs) 8 bpe @ SW_64K_{Z,R}_X 2xaa
    {   3,    9,  169,  127,    0, } , // 16 pipes (8 PKRs) 16 bpe @ SW_64K_{Z,R}_X 2xaa
    {   3,    5,  152,   77,    0, } , // 32 pipes (8 PKRs) 1 bpe @ SW_64K_{Z,R}_X 2xaa
    {   3,    6,  152,   78,    0, } , // 32 pipes (8 PKRs) 2 bpe @ SW_64K_{Z,R}_X 2xaa
    {   3,    7,  152,  128,    0, } , // 32 pipes (8 PKRs) 4 bpe @ SW_64K_{Z,R}_X 2xaa
    {   3,    8,  153,   80,    0, } , // 32 pipes (8 PKRs) 8 bpe @ SW_64K_{Z,R}_X 2xaa
    {   3,    9,  169,   80,    0, } , // 32 pipes (8 PKRs) 16 bpe @ SW_64K_{Z,R}_X 2xaa
    {   3,    5,  154,   72,    0, } , // 16 pipes (16 PKRs) 1 bpe @ SW_64K_{Z,R}_X 2xaa
    {   3,    6,  154,   73,    0, } , // 16 pipes (16 PKRs) 2 bpe @ SW_64K_{Z,R}_X 2xaa
    {   3,    7,  154,  129,    0, } , // 16 pipes (16 PKRs) 4 bpe @ SW_64K_{Z,R}_X 2xaa
    {   3,    8,  156,  130,    0, } , // 16 pipes (16 PKRs) 8 bpe @ SW_64K_{Z,R}_X 2xaa
    {   3,    9,  170,  130,    0, } , // 16 pipes (16 PKRs) 16 bpe @ SW_64K_{Z,R}_X 2xaa
    {   3,    5,  154,   77,    0, } , // 32 pipes (16 PKRs) 1 bpe @ SW_64K_{Z,R}_X 2xaa
    {   3,    6,  154,   78,    0, } , // 32 pipes (16 PKRs) 2 bpe @ SW_64K_{Z,R}_X 2xaa
    {   3,    7,  154,  128,    0, } , // 32 pipes (16 PKRs) 4 bpe @ SW_64K_{Z,R}_X 2xaa
    {   3,    8,  156,  131,    0, } , // 32 pipes (16 PKRs) 8 bpe @ SW_64K_{Z,R}_X 2xaa
    {   3,    9,  170,  131,    0, } , // 32 pipes (16 PKRs) 16 bpe @ SW_64K_{Z,R}_X 2xaa
    {   3,    5,  154,  132,    0, } , // 64 pipes (16 PKRs) 1 bpe @ SW_64K_{Z,R}_X 2xaa
    {   3,    6,  154,   83,    0, } , // 64 pipes (16 PKRs) 2 bpe @ SW_64K_{Z,R}_X 2xaa
    {   3,    7,  154,  133,    0, } , // 64 pipes (16 PKRs) 4 bpe @ SW_64K_{Z,R}_X 2xaa
    {   3,    8,  156,  134,    0, } , // 64 pipes (16 PKRs) 8 bpe @ SW_64K_{Z,R}_X 2xaa
    {   3,    9,  170,  134,    0, } , // 64 pipes (16 PKRs) 16 bpe @ SW_64K_{Z,R}_X 2xaa
    {   3,    5,  157,  135,    0, } , // 32 pipes (32 PKRs) 1 bpe @ SW_64K_{Z,R}_X 2xaa
    {   3,    6,  157,   88,    0, } , // 32 pipes (32 PKRs) 2 bpe @ SW_64K_{Z,R}_X 2xaa
    {   3,    7,  157,  136,    0, } , // 32 pipes (32 PKRs) 4 bpe @ SW_64K_{Z,R}_X 2xaa
    {   3,    8,  159,   90,    0, } , // 32 pipes (32 PKRs) 8 bpe @ SW_64K_{Z,R}_X 2xaa
    {   3,    9,  171,   90,    0, } , // 32 pipes (32 PKRs) 16 bpe @ SW_64K_{Z,R}_X 2xaa
    {   3,    5,  157,  137,    0, } , // 64 pipes (32 PKRs) 1 bpe @ SW_64K_{Z,R}_X 2xaa
    {   3,    6,  157,   93,    0, } , // 64 pipes (32 PKRs) 2 bpe @ SW_64K_{Z,R}_X 2xaa
    {   3,    7,  157,  138,    0, } , // 64 pipes (32 PKRs) 4 bpe @ SW_64K_{Z,R}_X 2xaa
    {   3,    8,  159,   95,    0, } , // 64 pipes (32 PKRs) 8 bpe @ SW_64K_{Z,R}_X 2xaa
    {   3,    9,  171,   95,    0, } , // 64 pipes (32 PKRs) 16 bpe @ SW_64K_{Z,R}_X 2xaa
};

const ADDR_SW_PATINFO GFX11_SW_64K_ZR_X_4xaa_PATINFO[] =
{
    {   2,   10,  118,   27,    0, } , // 1 pipes (1 PKRs) 1 bpe @ SW_64K_{Z,R}_X 4xaa
    {   2,   11,  118,  139,    0, } , // 1 pipes (1 PKRs) 2 bpe @ SW_64K_{Z,R}_X 4xaa
    {   2,   12,  118,  140,    0, } , // 1 pipes (1 PKRs) 4 bpe @ SW_64K_{Z,R}_X 4xaa
    {   2,   13,  119,  141,    0, } , // 1 pipes (1 PKRs) 8 bpe @ SW_64K_{Z,R}_X 4xaa
    {   2,   14,  120,  142,    0, } , // 1 pipes (1 PKRs) 16 bpe @ SW_64K_{Z,R}_X 4xaa
    {   3,   10,  123,   32,    0, } , // 2 pipes (1-2 PKRs) 1 bpe @ SW_64K_{Z,R}_X 4xaa
    {   3,   11,  172,  143,    0, } , // 2 pipes (1-2 PKRs) 2 bpe @ SW_64K_{Z,R}_X 4xaa
    {   3,   12,  123,  144,    0, } , // 2 pipes (1-2 PKRs) 4 bpe @ SW_64K_{Z,R}_X 4xaa
    {   3,   13,  124,  145,    0, } , // 2 pipes (1-2 PKRs) 8 bpe @ SW_64K_{Z,R}_X 4xaa
    {   3,   14,  125,  146,    0, } , // 2 pipes (1-2 PKRs) 16 bpe @ SW_64K_{Z,R}_X 4xaa
    {   3,   10,  128,   37,    0, } , // 4 pipes (1-2 PKRs) 1 bpe @ SW_64K_{Z,R}_X 4xaa
    {   3,   11,  128,  147,    0, } , // 4 pipes (1-2 PKRs) 2 bpe @ SW_64K_{Z,R}_X 4xaa
    {   3,   12,  128,  148,    0, } , // 4 pipes (1-2 PKRs) 4 bpe @ SW_64K_{Z,R}_X 4xaa
    {   3,   13,  129,  149,    0, } , // 4 pipes (1-2 PKRs) 8 bpe @ SW_64K_{Z,R}_X 4xaa
    {   3,   14,  130,  150,    0, } , // 4 pipes (1-2 PKRs) 16 bpe @ SW_64K_{Z,R}_X 4xaa
    {   3,   10,  133,   42,    0, } , // 8 pipes (2 PKRs) 1 bpe @ SW_64K_{Z,R}_X 4xaa
    {   3,   11,  133,  151,    0, } , // 8 pipes (2 PKRs) 2 bpe @ SW_64K_{Z,R}_X 4xaa
    {   3,   12,  133,  152,    0, } , // 8 pipes (2 PKRs) 4 bpe @ SW_64K_{Z,R}_X 4xaa
    {   3,   13,  134,  153,    0, } , // 8 pipes (2 PKRs) 8 bpe @ SW_64K_{Z,R}_X 4xaa
    {   3,   14,  173,  154,    0, } , // 8 pipes (2 PKRs) 16 bpe @ SW_64K_{Z,R}_X 4xaa
    {   3,   10,  138,   47,    0, } , // 4 pipes (4 PKRs) 1 bpe @ SW_64K_{Z,R}_X 4xaa
    {   3,   11,  138,  155,    0, } , // 4 pipes (4 PKRs) 2 bpe @ SW_64K_{Z,R}_X 4xaa
    {   3,   12,  138,  156,    0, } , // 4 pipes (4 PKRs) 4 bpe @ SW_64K_{Z,R}_X 4xaa
    {   3,   13,  174,  157,    0, } , // 4 pipes (4 PKRs) 8 bpe @ SW_64K_{Z,R}_X 4xaa
    {   3,   14,  175,  158,    0, } , // 4 pipes (4 PKRs) 16 bpe @ SW_64K_{Z,R}_X 4xaa
    {   3,   10,  143,   51,    0, } , // 8 pipes (4 PKRs) 1 bpe @ SW_64K_{Z,R}_X 4xaa
    {   3,   11,  143,  159,    0, } , // 8 pipes (4 PKRs) 2 bpe @ SW_64K_{Z,R}_X 4xaa
    {   3,   12,  143,  160,    0, } , // 8 pipes (4 PKRs) 4 bpe @ SW_64K_{Z,R}_X 4xaa
    {   3,   13,  145,  161,    0, } , // 8 pipes (4 PKRs) 8 bpe @ SW_64K_{Z,R}_X 4xaa
    {   3,   14,  176,  162,    0, } , // 8 pipes (4 PKRs) 16 bpe @ SW_64K_{Z,R}_X 4xaa
    {   3,   10,  146,   56,    0, } , // 16 pipes (4 PKRs) 1 bpe @ SW_64K_{Z,R}_X 4xaa
    {   3,   11,  146,  163,    0, } , // 16 pipes (4 PKRs) 2 bpe @ SW_64K_{Z,R}_X 4xaa
    {   3,   12,  146,  164,    0, } , // 16 pipes (4 PKRs) 4 bpe @ SW_64K_{Z,R}_X 4xaa
    {   3,   13,  167,  165,    0, } , // 16 pipes (4 PKRs) 8 bpe @ SW_64K_{Z,R}_X 4xaa
    {   3,   14,  177,  166,    0, } , // 16 pipes (4 PKRs) 16 bpe @ SW_64K_{Z,R}_X 4xaa
    {   3,   10,  149,   61,    0, } , // 8 pipes (8 PKRs) 1 bpe @ SW_64K_{Z,R}_X 4xaa
    {   3,   11,  149,  167,    0, } , // 8 pipes (8 PKRs) 2 bpe @ SW_64K_{Z,R}_X 4xaa
    {   3,   12,  149,  168,    0, } , // 8 pipes (8 PKRs) 4 bpe @ SW_64K_{Z,R}_X 4xaa
    {   3,   13,  178,  169,    0, } , // 8 pipes (8 PKRs) 8 bpe @ SW_64K_{Z,R}_X 4xaa
    {   3,   14,  179,  170,    0, } , // 8 pipes (8 PKRs) 16 bpe @ SW_64K_{Z,R}_X 4xaa
    {   3,   10,  152,   56,    0, } , // 16 pipes (8 PKRs) 1 bpe @ SW_64K_{Z,R}_X 4xaa
    {   3,   11,  152,  163,    0, } , // 16 pipes (8 PKRs) 2 bpe @ SW_64K_{Z,R}_X 4xaa
    {   3,   12,  152,  171,    0, } , // 16 pipes (8 PKRs) 4 bpe @ SW_64K_{Z,R}_X 4xaa
    {   3,   13,  180,  171,    0, } , // 16 pipes (8 PKRs) 8 bpe @ SW_64K_{Z,R}_X 4xaa
    {   3,   14,  181,  171,    0, } , // 16 pipes (8 PKRs) 16 bpe @ SW_64K_{Z,R}_X 4xaa
    {   3,   10,  152,   68,    0, } , // 32 pipes (8 PKRs) 1 bpe @ SW_64K_{Z,R}_X 4xaa
    {   3,   11,  152,  172,    0, } , // 32 pipes (8 PKRs) 2 bpe @ SW_64K_{Z,R}_X 4xaa
    {   3,   12,  152,  173,    0, } , // 32 pipes (8 PKRs) 4 bpe @ SW_64K_{Z,R}_X 4xaa
    {   3,   13,  180,  173,    0, } , // 32 pipes (8 PKRs) 8 bpe @ SW_64K_{Z,R}_X 4xaa
    {   3,   14,  181,  173,    0, } , // 32 pipes (8 PKRs) 16 bpe @ SW_64K_{Z,R}_X 4xaa
    {   3,   10,  154,   73,    0, } , // 16 pipes (16 PKRs) 1 bpe @ SW_64K_{Z,R}_X 4xaa
    {   3,   11,  154,  174,    0, } , // 16 pipes (16 PKRs) 2 bpe @ SW_64K_{Z,R}_X 4xaa
    {   3,   12,  154,  130,    0, } , // 16 pipes (16 PKRs) 4 bpe @ SW_64K_{Z,R}_X 4xaa
    {   3,   13,  182,  130,    0, } , // 16 pipes (16 PKRs) 8 bpe @ SW_64K_{Z,R}_X 4xaa
    {   3,   14,  183,  130,    0, } , // 16 pipes (16 PKRs) 16 bpe @ SW_64K_{Z,R}_X 4xaa
    {   3,   10,  154,   78,    0, } , // 32 pipes (16 PKRs) 1 bpe @ SW_64K_{Z,R}_X 4xaa
    {   3,   11,  154,  172,    0, } , // 32 pipes (16 PKRs) 2 bpe @ SW_64K_{Z,R}_X 4xaa
    {   3,   12,  154,  131,    0, } , // 32 pipes (16 PKRs) 4 bpe @ SW_64K_{Z,R}_X 4xaa
    {   3,   13,  182,  131,    0, } , // 32 pipes (16 PKRs) 8 bpe @ SW_64K_{Z,R}_X 4xaa
    {   3,   14,  183,  131,    0, } , // 32 pipes (16 PKRs) 16 bpe @ SW_64K_{Z,R}_X 4xaa
    {   3,   10,  154,   83,    0, } , // 64 pipes (16 PKRs) 1 bpe @ SW_64K_{Z,R}_X 4xaa
    {   3,   11,  154,  133,    0, } , // 64 pipes (16 PKRs) 2 bpe @ SW_64K_{Z,R}_X 4xaa
    {   3,   12,  154,  134,    0, } , // 64 pipes (16 PKRs) 4 bpe @ SW_64K_{Z,R}_X 4xaa
    {   3,   13,  182,  134,    0, } , // 64 pipes (16 PKRs) 8 bpe @ SW_64K_{Z,R}_X 4xaa
    {   3,   14,  183,  134,    0, } , // 64 pipes (16 PKRs) 16 bpe @ SW_64K_{Z,R}_X 4xaa
    {   3,   10,  157,   88,    0, } , // 32 pipes (32 PKRs) 1 bpe @ SW_64K_{Z,R}_X 4xaa
    {   3,   11,  157,  175,    0, } , // 32 pipes (32 PKRs) 2 bpe @ SW_64K_{Z,R}_X 4xaa
    {   3,   12,  157,   90,    0, } , // 32 pipes (32 PKRs) 4 bpe @ SW_64K_{Z,R}_X 4xaa
    {   3,   13,  184,   90,    0, } , // 32 pipes (32 PKRs) 8 bpe @ SW_64K_{Z,R}_X 4xaa
    {   3,   14,  185,   90,    0, } , // 32 pipes (32 PKRs) 16 bpe @ SW_64K_{Z,R}_X 4xaa
    {   3,   10,  157,   93,    0, } , // 64 pipes (32 PKRs) 1 bpe @ SW_64K_{Z,R}_X 4xaa
    {   3,   11,  157,  176,    0, } , // 64 pipes (32 PKRs) 2 bpe @ SW_64K_{Z,R}_X 4xaa
    {   3,   12,  157,   95,    0, } , // 64 pipes (32 PKRs) 4 bpe @ SW_64K_{Z,R}_X 4xaa
    {   3,   13,  184,   95,    0, } , // 64 pipes (32 PKRs) 8 bpe @ SW_64K_{Z,R}_X 4xaa
    {   3,   14,  185,   95,    0, } , // 64 pipes (32 PKRs) 16 bpe @ SW_64K_{Z,R}_X 4xaa
};

const ADDR_SW_PATINFO GFX11_SW_64K_ZR_X_8xaa_PATINFO[] =
{
    {   2,   15,  161,   97,    0, } , // 1 pipes (1 PKRs) 1 bpe @ SW_64K_{Z,R}_X 8xaa
    {   2,   16,  118,  140,    0, } , // 1 pipes (1 PKRs) 2 bpe @ SW_64K_{Z,R}_X 8xaa
    {   3,   17,  186,  177,    0, } , // 1 pipes (1 PKRs) 4 bpe @ SW_64K_{Z,R}_X 8xaa
    {   3,   18,  187,  178,    0, } , // 1 pipes (1 PKRs) 8 bpe @ SW_64K_{Z,R}_X 8xaa
    {   3,   19,  162,  179,    0, } , // 1 pipes (1 PKRs) 16 bpe @ SW_64K_{Z,R}_X 8xaa
    {   3,   15,  123,  101,    0, } , // 2 pipes (1-2 PKRs) 1 bpe @ SW_64K_{Z,R}_X 8xaa
    {   3,   16,  123,  144,    0, } , // 2 pipes (1-2 PKRs) 2 bpe @ SW_64K_{Z,R}_X 8xaa
    {   3,   17,  188,  180,    0, } , // 2 pipes (1-2 PKRs) 4 bpe @ SW_64K_{Z,R}_X 8xaa
    {   3,   18,  189,  181,    0, } , // 2 pipes (1-2 PKRs) 8 bpe @ SW_64K_{Z,R}_X 8xaa
    {   3,   19,  190,  182,    0, } , // 2 pipes (1-2 PKRs) 16 bpe @ SW_64K_{Z,R}_X 8xaa
    {   3,   15,  128,  105,    0, } , // 4 pipes (1-2 PKRs) 1 bpe @ SW_64K_{Z,R}_X 8xaa
    {   3,   16,  128,  148,    0, } , // 4 pipes (1-2 PKRs) 2 bpe @ SW_64K_{Z,R}_X 8xaa
    {   3,   17,  128,  183,    0, } , // 4 pipes (1-2 PKRs) 4 bpe @ SW_64K_{Z,R}_X 8xaa
    {   3,   18,  165,  184,    0, } , // 4 pipes (1-2 PKRs) 8 bpe @ SW_64K_{Z,R}_X 8xaa
    {   3,   19,  191,  185,    0, } , // 4 pipes (1-2 PKRs) 16 bpe @ SW_64K_{Z,R}_X 8xaa
    {   3,   15,  133,  109,    0, } , // 8 pipes (2 PKRs) 1 bpe @ SW_64K_{Z,R}_X 8xaa
    {   3,   16,  133,  186,    0, } , // 8 pipes (2 PKRs) 2 bpe @ SW_64K_{Z,R}_X 8xaa
    {   3,   17,  133,  187,    0, } , // 8 pipes (2 PKRs) 4 bpe @ SW_64K_{Z,R}_X 8xaa
    {   3,   18,  192,  188,    0, } , // 8 pipes (2 PKRs) 8 bpe @ SW_64K_{Z,R}_X 8xaa
    {   3,   19,  193,  189,    0, } , // 8 pipes (2 PKRs) 16 bpe @ SW_64K_{Z,R}_X 8xaa
    {   3,   15,  138,  113,    0, } , // 4 pipes (4 PKRs) 1 bpe @ SW_64K_{Z,R}_X 8xaa
    {   3,   16,  138,  156,    0, } , // 4 pipes (4 PKRs) 2 bpe @ SW_64K_{Z,R}_X 8xaa
    {   3,   17,  138,  190,    0, } , // 4 pipes (4 PKRs) 4 bpe @ SW_64K_{Z,R}_X 8xaa
    {   3,   18,  194,  191,    0, } , // 4 pipes (4 PKRs) 8 bpe @ SW_64K_{Z,R}_X 8xaa
    {   3,   19,  195,  192,    0, } , // 4 pipes (4 PKRs) 16 bpe @ SW_64K_{Z,R}_X 8xaa
    {   3,   15,  143,  109,    0, } , // 8 pipes (4 PKRs) 1 bpe @ SW_64K_{Z,R}_X 8xaa
    {   3,   16,  143,  160,    0, } , // 8 pipes (4 PKRs) 2 bpe @ SW_64K_{Z,R}_X 8xaa
    {   3,   17,  143,  187,    0, } , // 8 pipes (4 PKRs) 4 bpe @ SW_64K_{Z,R}_X 8xaa
    {   3,   18,  196,  193,    0, } , // 8 pipes (4 PKRs) 8 bpe @ SW_64K_{Z,R}_X 8xaa
    {   3,   19,  197,  194,    0, } , // 8 pipes (4 PKRs) 16 bpe @ SW_64K_{Z,R}_X 8xaa
    {   3,   15,  146,  126,    0, } , // 16 pipes (4 PKRs) 1 bpe @ SW_64K_{Z,R}_X 8xaa
    {   3,   16,  146,  164,    0, } , // 16 pipes (4 PKRs) 2 bpe @ SW_64K_{Z,R}_X 8xaa
    {   3,   17,  198,  195,    0, } , // 16 pipes (4 PKRs) 4 bpe @ SW_64K_{Z,R}_X 8xaa
    {   3,   18,  199,  196,    0, } , // 16 pipes (4 PKRs) 8 bpe @ SW_64K_{Z,R}_X 8xaa
    {   3,   19,  200,  197,    0, } , // 16 pipes (4 PKRs) 16 bpe @ SW_64K_{Z,R}_X 8xaa
    {   3,   15,  149,  123,    0, } , // 8 pipes (8 PKRs) 1 bpe @ SW_64K_{Z,R}_X 8xaa
    {   3,   16,  149,  168,    0, } , // 8 pipes (8 PKRs) 2 bpe @ SW_64K_{Z,R}_X 8xaa
    {   3,   17,  149,  198,    0, } , // 8 pipes (8 PKRs) 4 bpe @ SW_64K_{Z,R}_X 8xaa
    {   3,   18,  179,  170,    0, } , // 8 pipes (8 PKRs) 8 bpe @ SW_64K_{Z,R}_X 8xaa
    {   3,   19,  201,  170,    0, } , // 8 pipes (8 PKRs) 16 bpe @ SW_64K_{Z,R}_X 8xaa
    {   3,   15,  152,  126,    0, } , // 16 pipes (8 PKRs) 1 bpe @ SW_64K_{Z,R}_X 8xaa
    {   3,   16,  152,  171,    0, } , // 16 pipes (8 PKRs) 2 bpe @ SW_64K_{Z,R}_X 8xaa
    {   3,   17,  202,  199,    0, } , // 16 pipes (8 PKRs) 4 bpe @ SW_64K_{Z,R}_X 8xaa
    {   3,   18,  181,  171,    0, } , // 16 pipes (8 PKRs) 8 bpe @ SW_64K_{Z,R}_X 8xaa
    {   3,   19,  203,  171,    0, } , // 16 pipes (8 PKRs) 16 bpe @ SW_64K_{Z,R}_X 8xaa
    {   3,   15,  152,  128,    0, } , // 32 pipes (8 PKRs) 1 bpe @ SW_64K_{Z,R}_X 8xaa
    {   3,   16,  152,  173,    0, } , // 32 pipes (8 PKRs) 2 bpe @ SW_64K_{Z,R}_X 8xaa
    {   3,   17,  202,  200,    0, } , // 32 pipes (8 PKRs) 4 bpe @ SW_64K_{Z,R}_X 8xaa
    {   3,   18,  181,  173,    0, } , // 32 pipes (8 PKRs) 8 bpe @ SW_64K_{Z,R}_X 8xaa
    {   3,   19,  203,  201,    0, } , // 32 pipes (8 PKRs) 16 bpe @ SW_64K_{Z,R}_X 8xaa
    {   3,   15,  154,  129,    0, } , // 16 pipes (16 PKRs) 1 bpe @ SW_64K_{Z,R}_X 8xaa
    {   3,   16,  154,  130,    0, } , // 16 pipes (16 PKRs) 2 bpe @ SW_64K_{Z,R}_X 8xaa
    {   3,   17,  204,  202,    0, } , // 16 pipes (16 PKRs) 4 bpe @ SW_64K_{Z,R}_X 8xaa
    {   3,   18,  183,  130,    0, } , // 16 pipes (16 PKRs) 8 bpe @ SW_64K_{Z,R}_X 8xaa
    {   3,   19,  205,  130,    0, } , // 16 pipes (16 PKRs) 16 bpe @ SW_64K_{Z,R}_X 8xaa
    {   3,   15,  154,  128,    0, } , // 32 pipes (16 PKRs) 1 bpe @ SW_64K_{Z,R}_X 8xaa
    {   3,   16,  154,  131,    0, } , // 32 pipes (16 PKRs) 2 bpe @ SW_64K_{Z,R}_X 8xaa
    {   3,   17,  206,  203,    0, } , // 32 pipes (16 PKRs) 4 bpe @ SW_64K_{Z,R}_X 8xaa
    {   3,   18,  183,  131,    0, } , // 32 pipes (16 PKRs) 8 bpe @ SW_64K_{Z,R}_X 8xaa
    {   3,   19,  205,  131,    0, } , // 32 pipes (16 PKRs) 16 bpe @ SW_64K_{Z,R}_X 8xaa
    {   3,   15,  154,  133,    0, } , // 64 pipes (16 PKRs) 1 bpe @ SW_64K_{Z,R}_X 8xaa
    {   3,   16,  154,  134,    0, } , // 64 pipes (16 PKRs) 2 bpe @ SW_64K_{Z,R}_X 8xaa
    {   3,   17,  206,  204,    0, } , // 64 pipes (16 PKRs) 4 bpe @ SW_64K_{Z,R}_X 8xaa
    {   3,   18,  183,  134,    0, } , // 64 pipes (16 PKRs) 8 bpe @ SW_64K_{Z,R}_X 8xaa
    {   3,   19,  205,  134,    0, } , // 64 pipes (16 PKRs) 16 bpe @ SW_64K_{Z,R}_X 8xaa
    {   3,   15,  157,  136,    0, } , // 32 pipes (32 PKRs) 1 bpe @ SW_64K_{Z,R}_X 8xaa
    {   3,   16,  157,   90,    0, } , // 32 pipes (32 PKRs) 2 bpe @ SW_64K_{Z,R}_X 8xaa
    {   3,   17,  207,  205,    0, } , // 32 pipes (32 PKRs) 4 bpe @ SW_64K_{Z,R}_X 8xaa
    {   3,   18,  185,   90,    0, } , // 32 pipes (32 PKRs) 8 bpe @ SW_64K_{Z,R}_X 8xaa
    {   3,   19,  208,   90,    0, } , // 32 pipes (32 PKRs) 16 bpe @ SW_64K_{Z,R}_X 8xaa
    {   3,   15,  157,  138,    0, } , // 64 pipes (32 PKRs) 1 bpe @ SW_64K_{Z,R}_X 8xaa
    {   3,   16,  157,   95,    0, } , // 64 pipes (32 PKRs) 2 bpe @ SW_64K_{Z,R}_X 8xaa
    {   3,   17,  171,   95,    0, } , // 64 pipes (32 PKRs) 4 bpe @ SW_64K_{Z,R}_X 8xaa
    {   3,   18,  185,   95,    0, } , // 64 pipes (32 PKRs) 8 bpe @ SW_64K_{Z,R}_X 8xaa
    {   3,   19,  208,   95,    0, } , // 64 pipes (32 PKRs) 16 bpe @ SW_64K_{Z,R}_X 8xaa
};

const ADDR_SW_PATINFO GFX11_SW_256K_ZR_X_1xaa_PATINFO[] =
{
    {   2,    0,  116,   26,    6, } , // 1 pipes (1 PKRs) 1 bpe @ SW_256K_{Z,R}_X 1xaa
    {   2,    1,  117,   22,    2, } , // 1 pipes (1 PKRs) 2 bpe @ SW_256K_{Z,R}_X 1xaa
    {   2,    2,  118,   27,    7, } , // 1 pipes (1 PKRs) 4 bpe @ SW_256K_{Z,R}_X 1xaa
    {   2,    3,  119,   28,    4, } , // 1 pipes (1 PKRs) 8 bpe @ SW_256K_{Z,R}_X 1xaa
    {   2,    4,  120,   29,    8, } , // 1 pipes (1 PKRs) 16 bpe @ SW_256K_{Z,R}_X 1xaa
    {   3,    0,  121,   30,    6, } , // 2 pipes (1-2 PKRs) 1 bpe @ SW_256K_{Z,R}_X 1xaa
    {   3,    1,  122,   31,    9, } , // 2 pipes (1-2 PKRs) 2 bpe @ SW_256K_{Z,R}_X 1xaa
    {   3,    2,  123,   32,    7, } , // 2 pipes (1-2 PKRs) 4 bpe @ SW_256K_{Z,R}_X 1xaa
    {   3,    3,  124,   33,   10, } , // 2 pipes (1-2 PKRs) 8 bpe @ SW_256K_{Z,R}_X 1xaa
    {   3,    4,  125,   34,    8, } , // 2 pipes (1-2 PKRs) 16 bpe @ SW_256K_{Z,R}_X 1xaa
    {   3,    0,  126,   35,    6, } , // 4 pipes (1-2 PKRs) 1 bpe @ SW_256K_{Z,R}_X 1xaa
    {   3,    1,  127,   36,    9, } , // 4 pipes (1-2 PKRs) 2 bpe @ SW_256K_{Z,R}_X 1xaa
    {   3,    2,  128,   37,    7, } , // 4 pipes (1-2 PKRs) 4 bpe @ SW_256K_{Z,R}_X 1xaa
    {   3,    3,  129,   38,   10, } , // 4 pipes (1-2 PKRs) 8 bpe @ SW_256K_{Z,R}_X 1xaa
    {   3,    4,  130,   39,    8, } , // 4 pipes (1-2 PKRs) 16 bpe @ SW_256K_{Z,R}_X 1xaa
    {   3,    0,  131,  206,   11, } , // 8 pipes (2 PKRs) 1 bpe @ SW_256K_{Z,R}_X 1xaa
    {   3,    1,  132,  207,   12, } , // 8 pipes (2 PKRs) 2 bpe @ SW_256K_{Z,R}_X 1xaa
    {   3,    2,  133,  208,   13, } , // 8 pipes (2 PKRs) 4 bpe @ SW_256K_{Z,R}_X 1xaa
    {   3,    3,  134,  209,   14, } , // 8 pipes (2 PKRs) 8 bpe @ SW_256K_{Z,R}_X 1xaa
    {   3,    4,  135,  210,   15, } , // 8 pipes (2 PKRs) 16 bpe @ SW_256K_{Z,R}_X 1xaa
    {   3,    0,  136,  211,   16, } , // 4 pipes (4 PKRs) 1 bpe @ SW_256K_{Z,R}_X 1xaa
    {   3,    1,  137,   35,   17, } , // 4 pipes (4 PKRs) 2 bpe @ SW_256K_{Z,R}_X 1xaa
    {   3,    2,  138,  212,   18, } , // 4 pipes (4 PKRs) 4 bpe @ SW_256K_{Z,R}_X 1xaa
    {   3,    3,  139,  213,   19, } , // 4 pipes (4 PKRs) 8 bpe @ SW_256K_{Z,R}_X 1xaa
    {   3,    4,  140,  214,   20, } , // 4 pipes (4 PKRs) 16 bpe @ SW_256K_{Z,R}_X 1xaa
    {   3,    0,  141,  206,   11, } , // 8 pipes (4 PKRs) 1 bpe @ SW_256K_{Z,R}_X 1xaa
    {   3,    1,  142,  215,   21, } , // 8 pipes (4 PKRs) 2 bpe @ SW_256K_{Z,R}_X 1xaa
    {   3,    2,  143,  216,   13, } , // 8 pipes (4 PKRs) 4 bpe @ SW_256K_{Z,R}_X 1xaa
    {   3,    3,  144,  217,   22, } , // 8 pipes (4 PKRs) 8 bpe @ SW_256K_{Z,R}_X 1xaa
    {   3,    4,  145,  218,   15, } , // 8 pipes (4 PKRs) 16 bpe @ SW_256K_{Z,R}_X 1xaa
    {   3,    0,  146,  219,   23, } , // 16 pipes (4 PKRs) 1 bpe @ SW_256K_{Z,R}_X 1xaa
    {   3,    1,  146,  220,   24, } , // 16 pipes (4 PKRs) 2 bpe @ SW_256K_{Z,R}_X 1xaa
    {   3,    2,  146,  221,   25, } , // 16 pipes (4 PKRs) 4 bpe @ SW_256K_{Z,R}_X 1xaa
    {   3,    3,  146,  222,   26, } , // 16 pipes (4 PKRs) 8 bpe @ SW_256K_{Z,R}_X 1xaa
    {   3,    4,  146,  223,   27, } , // 16 pipes (4 PKRs) 16 bpe @ SW_256K_{Z,R}_X 1xaa
    {   3,    0,  147,  224,   28, } , // 8 pipes (8 PKRs) 1 bpe @ SW_256K_{Z,R}_X 1xaa
    {   3,    1,  148,  225,   29, } , // 8 pipes (8 PKRs) 2 bpe @ SW_256K_{Z,R}_X 1xaa
    {   3,    2,  149,  226,   30, } , // 8 pipes (8 PKRs) 4 bpe @ SW_256K_{Z,R}_X 1xaa
    {   3,    3,  150,  227,   31, } , // 8 pipes (8 PKRs) 8 bpe @ SW_256K_{Z,R}_X 1xaa
    {   3,    4,  151,  228,   32, } , // 8 pipes (8 PKRs) 16 bpe @ SW_256K_{Z,R}_X 1xaa
    {   3,    0,  152,  219,   23, } , // 16 pipes (8 PKRs) 1 bpe @ SW_256K_{Z,R}_X 1xaa
    {   3,    1,  152,  229,   33, } , // 16 pipes (8 PKRs) 2 bpe @ SW_256K_{Z,R}_X 1xaa
    {   3,    2,  152,  221,   25, } , // 16 pipes (8 PKRs) 4 bpe @ SW_256K_{Z,R}_X 1xaa
    {   3,    3,  153,  222,   34, } , // 16 pipes (8 PKRs) 8 bpe @ SW_256K_{Z,R}_X 1xaa
    {   3,    4,  153,  230,   27, } , // 16 pipes (8 PKRs) 16 bpe @ SW_256K_{Z,R}_X 1xaa
    {   3,    0,  152,  231,   23, } , // 32 pipes (8 PKRs) 1 bpe @ SW_256K_{Z,R}_X 1xaa
    {   3,    1,  152,  232,   33, } , // 32 pipes (8 PKRs) 2 bpe @ SW_256K_{Z,R}_X 1xaa
    {   3,    2,  152,  233,   25, } , // 32 pipes (8 PKRs) 4 bpe @ SW_256K_{Z,R}_X 1xaa
    {   3,    3,  153,  234,   34, } , // 32 pipes (8 PKRs) 8 bpe @ SW_256K_{Z,R}_X 1xaa
    {   3,    4,  153,  235,   35, } , // 32 pipes (8 PKRs) 16 bpe @ SW_256K_{Z,R}_X 1xaa
    {   3,    0,  154,  236,   36, } , // 16 pipes (16 PKRs) 1 bpe @ SW_256K_{Z,R}_X 1xaa
    {   3,    1,  154,  237,   37, } , // 16 pipes (16 PKRs) 2 bpe @ SW_256K_{Z,R}_X 1xaa
    {   3,    2,  154,  238,   38, } , // 16 pipes (16 PKRs) 4 bpe @ SW_256K_{Z,R}_X 1xaa
    {   3,    3,  155,  239,   39, } , // 16 pipes (16 PKRs) 8 bpe @ SW_256K_{Z,R}_X 1xaa
    {   3,    4,  155,  240,   40, } , // 16 pipes (16 PKRs) 16 bpe @ SW_256K_{Z,R}_X 1xaa
    {   3,    0,  154,  241,   23, } , // 32 pipes (16 PKRs) 1 bpe @ SW_256K_{Z,R}_X 1xaa
    {   3,    1,  154,  242,   24, } , // 32 pipes (16 PKRs) 2 bpe @ SW_256K_{Z,R}_X 1xaa
    {   3,    2,  154,  243,   25, } , // 32 pipes (16 PKRs) 4 bpe @ SW_256K_{Z,R}_X 1xaa
    {   3,    3,  155,  244,   41, } , // 32 pipes (16 PKRs) 8 bpe @ SW_256K_{Z,R}_X 1xaa
    {   3,    4,  155,  245,   42, } , // 32 pipes (16 PKRs) 16 bpe @ SW_256K_{Z,R}_X 1xaa
    {   3,    0,  154,   81,   23, } , // 64 pipes (16 PKRs) 1 bpe @ SW_256K_{Z,R}_X 1xaa
    {   3,    1,  154,   82,   24, } , // 64 pipes (16 PKRs) 2 bpe @ SW_256K_{Z,R}_X 1xaa
    {   3,    2,  154,   83,   25, } , // 64 pipes (16 PKRs) 4 bpe @ SW_256K_{Z,R}_X 1xaa
    {   3,    3,  155,  246,   43, } , // 64 pipes (16 PKRs) 8 bpe @ SW_256K_{Z,R}_X 1xaa
    {   3,    4,  155,  247,   44, } , // 64 pipes (16 PKRs) 16 bpe @ SW_256K_{Z,R}_X 1xaa
    {   3,    0,  157,  248,   45, } , // 32 pipes (32 PKRs) 1 bpe @ SW_256K_{Z,R}_X 1xaa
    {   3,    1,  157,  249,   46, } , // 32 pipes (32 PKRs) 2 bpe @ SW_256K_{Z,R}_X 1xaa
    {   3,    2,  157,  250,   47, } , // 32 pipes (32 PKRs) 4 bpe @ SW_256K_{Z,R}_X 1xaa
    {   3,    3,  209,  251,   48, } , // 32 pipes (32 PKRs) 8 bpe @ SW_256K_{Z,R}_X 1xaa
    {   3,    4,  209,  252,   49, } , // 32 pipes (32 PKRs) 16 bpe @ SW_256K_{Z,R}_X 1xaa
    {   3,    0,  157,   91,   23, } , // 64 pipes (32 PKRs) 1 bpe @ SW_256K_{Z,R}_X 1xaa
    {   3,    1,  157,   92,   33, } , // 64 pipes (32 PKRs) 2 bpe @ SW_256K_{Z,R}_X 1xaa
    {   3,    2,  157,   93,   25, } , // 64 pipes (32 PKRs) 4 bpe @ SW_256K_{Z,R}_X 1xaa
    {   3,    3,  209,  253,   43, } , // 64 pipes (32 PKRs) 8 bpe @ SW_256K_{Z,R}_X 1xaa
    {   3,    4,  209,  254,   50, } , // 64 pipes (32 PKRs) 16 bpe @ SW_256K_{Z,R}_X 1xaa
};

const ADDR_SW_PATINFO GFX11_SW_256K_ZR_X_2xaa_PATINFO[] =
{
    {   2,    5,  160,   96,   51, } , // 1 pipes (1 PKRs) 1 bpe @ SW_256K_{Z,R}_X 2xaa
    {   2,    6,  118,   27,    7, } , // 1 pipes (1 PKRs) 2 bpe @ SW_256K_{Z,R}_X 2xaa
    {   2,    7,  210,  255,   52, } , // 1 pipes (1 PKRs) 4 bpe @ SW_256K_{Z,R}_X 2xaa
    {   2,    8,  120,   29,    8, } , // 1 pipes (1 PKRs) 8 bpe @ SW_256K_{Z,R}_X 2xaa
    {   2,    9,  211,  256,   53, } , // 1 pipes (1 PKRs) 16 bpe @ SW_256K_{Z,R}_X 2xaa
    {   3,    5,  163,  100,   51, } , // 2 pipes (1-2 PKRs) 1 bpe @ SW_256K_{Z,R}_X 2xaa
    {   3,    6,  123,   32,    7, } , // 2 pipes (1-2 PKRs) 2 bpe @ SW_256K_{Z,R}_X 2xaa
    {   3,    7,  212,  257,   52, } , // 2 pipes (1-2 PKRs) 4 bpe @ SW_256K_{Z,R}_X 2xaa
    {   3,    8,  125,   34,    8, } , // 2 pipes (1-2 PKRs) 8 bpe @ SW_256K_{Z,R}_X 2xaa
    {   3,    9,  213,  258,   53, } , // 2 pipes (1-2 PKRs) 16 bpe @ SW_256K_{Z,R}_X 2xaa
    {   3,    5,  127,  104,   51, } , // 4 pipes (1-2 PKRs) 1 bpe @ SW_256K_{Z,R}_X 2xaa
    {   3,    6,  128,   37,    7, } , // 4 pipes (1-2 PKRs) 2 bpe @ SW_256K_{Z,R}_X 2xaa
    {   3,    7,  129,  259,   52, } , // 4 pipes (1-2 PKRs) 4 bpe @ SW_256K_{Z,R}_X 2xaa
    {   3,    8,  130,   39,    8, } , // 4 pipes (1-2 PKRs) 8 bpe @ SW_256K_{Z,R}_X 2xaa
    {   3,    9,  214,  260,   53, } , // 4 pipes (1-2 PKRs) 16 bpe @ SW_256K_{Z,R}_X 2xaa
    {   3,    5,  132,  261,   54, } , // 8 pipes (2 PKRs) 1 bpe @ SW_256K_{Z,R}_X 2xaa
    {   3,    6,  133,  216,   13, } , // 8 pipes (2 PKRs) 2 bpe @ SW_256K_{Z,R}_X 2xaa
    {   3,    7,  134,  262,   55, } , // 8 pipes (2 PKRs) 4 bpe @ SW_256K_{Z,R}_X 2xaa
    {   3,    8,  135,  263,   15, } , // 8 pipes (2 PKRs) 8 bpe @ SW_256K_{Z,R}_X 2xaa
    {   3,    9,  215,  264,   56, } , // 8 pipes (2 PKRs) 16 bpe @ SW_256K_{Z,R}_X 2xaa
    {   3,    5,  137,  265,   16, } , // 4 pipes (4 PKRs) 1 bpe @ SW_256K_{Z,R}_X 2xaa
    {   3,    6,  138,  212,   18, } , // 4 pipes (4 PKRs) 2 bpe @ SW_256K_{Z,R}_X 2xaa
    {   3,    7,  139,  266,   18, } , // 4 pipes (4 PKRs) 4 bpe @ SW_256K_{Z,R}_X 2xaa
    {   3,    8,  140,  214,   20, } , // 4 pipes (4 PKRs) 8 bpe @ SW_256K_{Z,R}_X 2xaa
    {   3,    9,  216,  267,   20, } , // 4 pipes (4 PKRs) 16 bpe @ SW_256K_{Z,R}_X 2xaa
    {   3,    5,  142,  261,   54, } , // 8 pipes (4 PKRs) 1 bpe @ SW_256K_{Z,R}_X 2xaa
    {   3,    6,  143,  216,   13, } , // 8 pipes (4 PKRs) 2 bpe @ SW_256K_{Z,R}_X 2xaa
    {   3,    7,  144,  262,   55, } , // 8 pipes (4 PKRs) 4 bpe @ SW_256K_{Z,R}_X 2xaa
    {   3,    8,  145,  218,   15, } , // 8 pipes (4 PKRs) 8 bpe @ SW_256K_{Z,R}_X 2xaa
    {   3,    9,  217,  268,   56, } , // 8 pipes (4 PKRs) 16 bpe @ SW_256K_{Z,R}_X 2xaa
    {   3,    5,  146,  269,   57, } , // 16 pipes (4 PKRs) 1 bpe @ SW_256K_{Z,R}_X 2xaa
    {   3,    6,  146,  270,   25, } , // 16 pipes (4 PKRs) 2 bpe @ SW_256K_{Z,R}_X 2xaa
    {   3,    7,  146,  271,   41, } , // 16 pipes (4 PKRs) 4 bpe @ SW_256K_{Z,R}_X 2xaa
    {   3,    8,  146,  272,   58, } , // 16 pipes (4 PKRs) 8 bpe @ SW_256K_{Z,R}_X 2xaa
    {   3,    9,  146,  273,   59, } , // 16 pipes (4 PKRs) 16 bpe @ SW_256K_{Z,R}_X 2xaa
    {   3,    5,  148,  274,   60, } , // 8 pipes (8 PKRs) 1 bpe @ SW_256K_{Z,R}_X 2xaa
    {   3,    6,  149,  226,   30, } , // 8 pipes (8 PKRs) 2 bpe @ SW_256K_{Z,R}_X 2xaa
    {   3,    7,  218,  275,   61, } , // 8 pipes (8 PKRs) 4 bpe @ SW_256K_{Z,R}_X 2xaa
    {   3,    8,  151,  228,   32, } , // 8 pipes (8 PKRs) 8 bpe @ SW_256K_{Z,R}_X 2xaa
    {   3,    9,  219,  276,   62, } , // 8 pipes (8 PKRs) 16 bpe @ SW_256K_{Z,R}_X 2xaa
    {   3,    5,  152,  277,   57, } , // 16 pipes (8 PKRs) 1 bpe @ SW_256K_{Z,R}_X 2xaa
    {   3,    6,  152,  221,   25, } , // 16 pipes (8 PKRs) 2 bpe @ SW_256K_{Z,R}_X 2xaa
    {   3,    7,  152,  278,   41, } , // 16 pipes (8 PKRs) 4 bpe @ SW_256K_{Z,R}_X 2xaa
    {   3,    8,  153,  230,   27, } , // 16 pipes (8 PKRs) 8 bpe @ SW_256K_{Z,R}_X 2xaa
    {   3,    9,  153,  279,   63, } , // 16 pipes (8 PKRs) 16 bpe @ SW_256K_{Z,R}_X 2xaa
    {   3,    5,  152,  280,   57, } , // 32 pipes (8 PKRs) 1 bpe @ SW_256K_{Z,R}_X 2xaa
    {   3,    6,  152,  243,   25, } , // 32 pipes (8 PKRs) 2 bpe @ SW_256K_{Z,R}_X 2xaa
    {   3,    7,  152,  281,   41, } , // 32 pipes (8 PKRs) 4 bpe @ SW_256K_{Z,R}_X 2xaa
    {   3,    8,  153,  282,   64, } , // 32 pipes (8 PKRs) 8 bpe @ SW_256K_{Z,R}_X 2xaa
    {   3,    9,  153,  283,   65, } , // 32 pipes (8 PKRs) 16 bpe @ SW_256K_{Z,R}_X 2xaa
    {   3,    5,  154,  284,   37, } , // 16 pipes (16 PKRs) 1 bpe @ SW_256K_{Z,R}_X 2xaa
    {   3,    6,  154,  238,   38, } , // 16 pipes (16 PKRs) 2 bpe @ SW_256K_{Z,R}_X 2xaa
    {   3,    7,  154,  239,   66, } , // 16 pipes (16 PKRs) 4 bpe @ SW_256K_{Z,R}_X 2xaa
    {   3,    8,  155,  240,   40, } , // 16 pipes (16 PKRs) 8 bpe @ SW_256K_{Z,R}_X 2xaa
    {   3,    9,  155,  273,   67, } , // 16 pipes (16 PKRs) 16 bpe @ SW_256K_{Z,R}_X 2xaa
    {   3,    5,  154,  280,   57, } , // 32 pipes (16 PKRs) 1 bpe @ SW_256K_{Z,R}_X 2xaa
    {   3,    6,  154,  243,   25, } , // 32 pipes (16 PKRs) 2 bpe @ SW_256K_{Z,R}_X 2xaa
    {   3,    7,  154,  281,   41, } , // 32 pipes (16 PKRs) 4 bpe @ SW_256K_{Z,R}_X 2xaa
    {   3,    8,  155,  245,   42, } , // 32 pipes (16 PKRs) 8 bpe @ SW_256K_{Z,R}_X 2xaa
    {   3,    9,  155,  285,   68, } , // 32 pipes (16 PKRs) 16 bpe @ SW_256K_{Z,R}_X 2xaa
    {   3,    5,  154,   82,   24, } , // 64 pipes (16 PKRs) 1 bpe @ SW_256K_{Z,R}_X 2xaa
    {   3,    6,  154,   83,   25, } , // 64 pipes (16 PKRs) 2 bpe @ SW_256K_{Z,R}_X 2xaa
    {   3,    7,  154,  286,   43, } , // 64 pipes (16 PKRs) 4 bpe @ SW_256K_{Z,R}_X 2xaa
    {   3,    8,  155,  247,   44, } , // 64 pipes (16 PKRs) 8 bpe @ SW_256K_{Z,R}_X 2xaa
    {   3,    9,  155,  287,   69, } , // 64 pipes (16 PKRs) 16 bpe @ SW_256K_{Z,R}_X 2xaa
    {   3,    5,  157,  288,   70, } , // 32 pipes (32 PKRs) 1 bpe @ SW_256K_{Z,R}_X 2xaa
    {   3,    6,  157,  250,   47, } , // 32 pipes (32 PKRs) 2 bpe @ SW_256K_{Z,R}_X 2xaa
    {   3,    7,  157,  289,   71, } , // 32 pipes (32 PKRs) 4 bpe @ SW_256K_{Z,R}_X 2xaa
    {   3,    8,  158,  290,   72, } , // 32 pipes (32 PKRs) 8 bpe @ SW_256K_{Z,R}_X 2xaa
    {   3,    9,  158,  291,   73, } , // 32 pipes (32 PKRs) 16 bpe @ SW_256K_{Z,R}_X 2xaa
    {   3,    5,  157,   92,   24, } , // 64 pipes (32 PKRs) 1 bpe @ SW_256K_{Z,R}_X 2xaa
    {   3,    6,  157,   93,   25, } , // 64 pipes (32 PKRs) 2 bpe @ SW_256K_{Z,R}_X 2xaa
    {   3,    7,  157,  292,   43, } , // 64 pipes (32 PKRs) 4 bpe @ SW_256K_{Z,R}_X 2xaa
    {   3,    8,  158,  293,   50, } , // 64 pipes (32 PKRs) 8 bpe @ SW_256K_{Z,R}_X 2xaa
    {   3,    9,  158,  294,   74, } , // 64 pipes (32 PKRs) 16 bpe @ SW_256K_{Z,R}_X 2xaa
};

const ADDR_SW_PATINFO GFX11_SW_256K_ZR_X_4xaa_PATINFO[] =
{
    {   2,   10,  118,   27,    7, } , // 1 pipes (1 PKRs) 1 bpe @ SW_256K_{Z,R}_X 4xaa
    {   2,   11,  119,   28,    4, } , // 1 pipes (1 PKRs) 2 bpe @ SW_256K_{Z,R}_X 4xaa
    {   2,   12,  120,   29,    8, } , // 1 pipes (1 PKRs) 4 bpe @ SW_256K_{Z,R}_X 4xaa
    {   2,   13,  220,  295,   75, } , // 1 pipes (1 PKRs) 8 bpe @ SW_256K_{Z,R}_X 4xaa
    {   2,   14,  221,  296,   76, } , // 1 pipes (1 PKRs) 16 bpe @ SW_256K_{Z,R}_X 4xaa
    {   3,   10,  123,   32,    7, } , // 2 pipes (1-2 PKRs) 1 bpe @ SW_256K_{Z,R}_X 4xaa
    {   3,   11,  124,   33,   10, } , // 2 pipes (1-2 PKRs) 2 bpe @ SW_256K_{Z,R}_X 4xaa
    {   3,   12,  125,   34,    8, } , // 2 pipes (1-2 PKRs) 4 bpe @ SW_256K_{Z,R}_X 4xaa
    {   3,   13,  222,  297,   77, } , // 2 pipes (1-2 PKRs) 8 bpe @ SW_256K_{Z,R}_X 4xaa
    {   3,   14,  223,  298,   76, } , // 2 pipes (1-2 PKRs) 16 bpe @ SW_256K_{Z,R}_X 4xaa
    {   3,   10,  128,   37,    7, } , // 4 pipes (1-2 PKRs) 1 bpe @ SW_256K_{Z,R}_X 4xaa
    {   3,   11,  129,   38,   10, } , // 4 pipes (1-2 PKRs) 2 bpe @ SW_256K_{Z,R}_X 4xaa
    {   3,   12,  130,   39,    8, } , // 4 pipes (1-2 PKRs) 4 bpe @ SW_256K_{Z,R}_X 4xaa
    {   3,   13,  224,  299,   77, } , // 4 pipes (1-2 PKRs) 8 bpe @ SW_256K_{Z,R}_X 4xaa
    {   3,   14,  225,  300,   76, } , // 4 pipes (1-2 PKRs) 16 bpe @ SW_256K_{Z,R}_X 4xaa
    {   3,   10,  133,  208,   13, } , // 8 pipes (2 PKRs) 1 bpe @ SW_256K_{Z,R}_X 4xaa
    {   3,   11,  134,  209,   14, } , // 8 pipes (2 PKRs) 2 bpe @ SW_256K_{Z,R}_X 4xaa
    {   3,   12,  135,  210,   15, } , // 8 pipes (2 PKRs) 4 bpe @ SW_256K_{Z,R}_X 4xaa
    {   3,   13,  215,  301,   78, } , // 8 pipes (2 PKRs) 8 bpe @ SW_256K_{Z,R}_X 4xaa
    {   3,   14,  226,  302,   79, } , // 8 pipes (2 PKRs) 16 bpe @ SW_256K_{Z,R}_X 4xaa
    {   3,   10,  138,  212,   18, } , // 4 pipes (4 PKRs) 1 bpe @ SW_256K_{Z,R}_X 4xaa
    {   3,   11,  139,  213,   19, } , // 4 pipes (4 PKRs) 2 bpe @ SW_256K_{Z,R}_X 4xaa
    {   3,   12,  140,  214,   20, } , // 4 pipes (4 PKRs) 4 bpe @ SW_256K_{Z,R}_X 4xaa
    {   3,   13,  216,  299,   80, } , // 4 pipes (4 PKRs) 8 bpe @ SW_256K_{Z,R}_X 4xaa
    {   3,   14,  227,  303,   81, } , // 4 pipes (4 PKRs) 16 bpe @ SW_256K_{Z,R}_X 4xaa
    {   3,   10,  143,  216,   13, } , // 8 pipes (4 PKRs) 1 bpe @ SW_256K_{Z,R}_X 4xaa
    {   3,   11,  144,  217,   22, } , // 8 pipes (4 PKRs) 2 bpe @ SW_256K_{Z,R}_X 4xaa
    {   3,   12,  145,  218,   15, } , // 8 pipes (4 PKRs) 4 bpe @ SW_256K_{Z,R}_X 4xaa
    {   3,   13,  217,  304,   82, } , // 8 pipes (4 PKRs) 8 bpe @ SW_256K_{Z,R}_X 4xaa
    {   3,   14,  228,  305,   83, } , // 8 pipes (4 PKRs) 16 bpe @ SW_256K_{Z,R}_X 4xaa
    {   3,   10,  146,  221,   25, } , // 16 pipes (4 PKRs) 1 bpe @ SW_256K_{Z,R}_X 4xaa
    {   3,   11,  146,  222,   26, } , // 16 pipes (4 PKRs) 2 bpe @ SW_256K_{Z,R}_X 4xaa
    {   3,   12,  146,  223,   27, } , // 16 pipes (4 PKRs) 4 bpe @ SW_256K_{Z,R}_X 4xaa
    {   3,   13,  146,  306,   84, } , // 16 pipes (4 PKRs) 8 bpe @ SW_256K_{Z,R}_X 4xaa
    {   3,   14,  146,  307,   85, } , // 16 pipes (4 PKRs) 16 bpe @ SW_256K_{Z,R}_X 4xaa
    {   3,   10,  149,  226,   30, } , // 8 pipes (8 PKRs) 1 bpe @ SW_256K_{Z,R}_X 4xaa
    {   3,   11,  218,  227,   86, } , // 8 pipes (8 PKRs) 2 bpe @ SW_256K_{Z,R}_X 4xaa
    {   3,   12,  168,  228,   87, } , // 8 pipes (8 PKRs) 4 bpe @ SW_256K_{Z,R}_X 4xaa
    {   3,   13,  219,  301,   62, } , // 8 pipes (8 PKRs) 8 bpe @ SW_256K_{Z,R}_X 4xaa
    {   3,   14,  229,  308,   88, } , // 8 pipes (8 PKRs) 16 bpe @ SW_256K_{Z,R}_X 4xaa
    {   3,   10,  152,  221,   25, } , // 16 pipes (8 PKRs) 1 bpe @ SW_256K_{Z,R}_X 4xaa
    {   3,   11,  152,  222,   34, } , // 16 pipes (8 PKRs) 2 bpe @ SW_256K_{Z,R}_X 4xaa
    {   3,   12,  152,  230,   27, } , // 16 pipes (8 PKRs) 4 bpe @ SW_256K_{Z,R}_X 4xaa
    {   3,   13,  153,  306,   84, } , // 16 pipes (8 PKRs) 8 bpe @ SW_256K_{Z,R}_X 4xaa
    {   3,   14,  153,  309,   89, } , // 16 pipes (8 PKRs) 16 bpe @ SW_256K_{Z,R}_X 4xaa
    {   3,   10,  152,  233,   25, } , // 32 pipes (8 PKRs) 1 bpe @ SW_256K_{Z,R}_X 4xaa
    {   3,   11,  152,  234,   34, } , // 32 pipes (8 PKRs) 2 bpe @ SW_256K_{Z,R}_X 4xaa
    {   3,   12,  152,  235,   35, } , // 32 pipes (8 PKRs) 4 bpe @ SW_256K_{Z,R}_X 4xaa
    {   3,   13,  153,  310,   90, } , // 32 pipes (8 PKRs) 8 bpe @ SW_256K_{Z,R}_X 4xaa
    {   3,   14,  153,  311,   91, } , // 32 pipes (8 PKRs) 16 bpe @ SW_256K_{Z,R}_X 4xaa
    {   3,   10,  154,  238,   38, } , // 16 pipes (16 PKRs) 1 bpe @ SW_256K_{Z,R}_X 4xaa
    {   3,   11,  154,  239,   66, } , // 16 pipes (16 PKRs) 2 bpe @ SW_256K_{Z,R}_X 4xaa
    {   3,   12,  154,  240,   92, } , // 16 pipes (16 PKRs) 4 bpe @ SW_256K_{Z,R}_X 4xaa
    {   3,   13,  156,  312,   93, } , // 16 pipes (16 PKRs) 8 bpe @ SW_256K_{Z,R}_X 4xaa
    {   3,   14,  156,  313,   94, } , // 16 pipes (16 PKRs) 16 bpe @ SW_256K_{Z,R}_X 4xaa
    {   3,   10,  154,  243,   25, } , // 32 pipes (16 PKRs) 1 bpe @ SW_256K_{Z,R}_X 4xaa
    {   3,   11,  154,  281,   41, } , // 32 pipes (16 PKRs) 2 bpe @ SW_256K_{Z,R}_X 4xaa
    {   3,   12,  154,  314,   42, } , // 32 pipes (16 PKRs) 4 bpe @ SW_256K_{Z,R}_X 4xaa
    {   3,   13,  156,  315,   95, } , // 32 pipes (16 PKRs) 8 bpe @ SW_256K_{Z,R}_X 4xaa
    {   3,   14,  156,  316,   96, } , // 32 pipes (16 PKRs) 16 bpe @ SW_256K_{Z,R}_X 4xaa
    {   3,   10,  154,   83,   25, } , // 64 pipes (16 PKRs) 1 bpe @ SW_256K_{Z,R}_X 4xaa
    {   3,   11,  154,  286,   43, } , // 64 pipes (16 PKRs) 2 bpe @ SW_256K_{Z,R}_X 4xaa
    {   3,   12,  154,  317,   44, } , // 64 pipes (16 PKRs) 4 bpe @ SW_256K_{Z,R}_X 4xaa
    {   3,   13,  156,  318,   97, } , // 64 pipes (16 PKRs) 8 bpe @ SW_256K_{Z,R}_X 4xaa
    {   3,   14,  156,  319,   68, } , // 64 pipes (16 PKRs) 16 bpe @ SW_256K_{Z,R}_X 4xaa
    {   3,   10,  157,  250,   47, } , // 32 pipes (32 PKRs) 1 bpe @ SW_256K_{Z,R}_X 4xaa
    {   3,   11,  157,  289,   71, } , // 32 pipes (32 PKRs) 2 bpe @ SW_256K_{Z,R}_X 4xaa
    {   3,   12,  157,  320,   98, } , // 32 pipes (32 PKRs) 4 bpe @ SW_256K_{Z,R}_X 4xaa
    {   3,   13,  159,  321,   99, } , // 32 pipes (32 PKRs) 8 bpe @ SW_256K_{Z,R}_X 4xaa
    {   3,   14,  159,  322,  100, } , // 32 pipes (32 PKRs) 16 bpe @ SW_256K_{Z,R}_X 4xaa
    {   3,   10,  157,   93,   25, } , // 64 pipes (32 PKRs) 1 bpe @ SW_256K_{Z,R}_X 4xaa
    {   3,   11,  157,  292,   43, } , // 64 pipes (32 PKRs) 2 bpe @ SW_256K_{Z,R}_X 4xaa
    {   3,   12,  157,  323,   50, } , // 64 pipes (32 PKRs) 4 bpe @ SW_256K_{Z,R}_X 4xaa
    {   3,   13,  159,  324,   74, } , // 64 pipes (32 PKRs) 8 bpe @ SW_256K_{Z,R}_X 4xaa
    {   3,   14,  159,  325,  101, } , // 64 pipes (32 PKRs) 16 bpe @ SW_256K_{Z,R}_X 4xaa
};

const ADDR_SW_PATINFO GFX11_SW_256K_ZR_X_8xaa_PATINFO[] =
{
    {   2,   15,  210,  255,   52, } , // 1 pipes (1 PKRs) 1 bpe @ SW_256K_{Z,R}_X 8xaa
    {   2,   16,  120,   29,    8, } , // 1 pipes (1 PKRs) 2 bpe @ SW_256K_{Z,R}_X 8xaa
    {   2,   17,  211,  256,   53, } , // 1 pipes (1 PKRs) 4 bpe @ SW_256K_{Z,R}_X 8xaa
    {   2,   18,  221,  296,   76, } , // 1 pipes (1 PKRs) 8 bpe @ SW_256K_{Z,R}_X 8xaa
    {   2,   19,  230,  326,  102, } , // 1 pipes (1 PKRs) 16 bpe @ SW_256K_{Z,R}_X 8xaa
    {   3,   15,  212,  257,   52, } , // 2 pipes (1-2 PKRs) 1 bpe @ SW_256K_{Z,R}_X 8xaa
    {   3,   16,  125,   34,    8, } , // 2 pipes (1-2 PKRs) 2 bpe @ SW_256K_{Z,R}_X 8xaa
    {   3,   17,  213,  258,   53, } , // 2 pipes (1-2 PKRs) 4 bpe @ SW_256K_{Z,R}_X 8xaa
    {   3,   18,  223,  298,   76, } , // 2 pipes (1-2 PKRs) 8 bpe @ SW_256K_{Z,R}_X 8xaa
    {   3,   19,  231,  327,  103, } , // 2 pipes (1-2 PKRs) 16 bpe @ SW_256K_{Z,R}_X 8xaa
    {   3,   15,  129,  259,   52, } , // 4 pipes (1-2 PKRs) 1 bpe @ SW_256K_{Z,R}_X 8xaa
    {   3,   16,  130,   39,    8, } , // 4 pipes (1-2 PKRs) 2 bpe @ SW_256K_{Z,R}_X 8xaa
    {   3,   17,  214,  260,   53, } , // 4 pipes (1-2 PKRs) 4 bpe @ SW_256K_{Z,R}_X 8xaa
    {   3,   18,  225,  300,   76, } , // 4 pipes (1-2 PKRs) 8 bpe @ SW_256K_{Z,R}_X 8xaa
    {   3,   19,  232,  328,  103, } , // 4 pipes (1-2 PKRs) 16 bpe @ SW_256K_{Z,R}_X 8xaa
    {   3,   15,  134,  262,   55, } , // 8 pipes (2 PKRs) 1 bpe @ SW_256K_{Z,R}_X 8xaa
    {   3,   16,  135,  263,   15, } , // 8 pipes (2 PKRs) 2 bpe @ SW_256K_{Z,R}_X 8xaa
    {   3,   17,  215,  264,   56, } , // 8 pipes (2 PKRs) 4 bpe @ SW_256K_{Z,R}_X 8xaa
    {   3,   18,  226,  302,  104, } , // 8 pipes (2 PKRs) 8 bpe @ SW_256K_{Z,R}_X 8xaa
    {   3,   19,  233,  329,  105, } , // 8 pipes (2 PKRs) 16 bpe @ SW_256K_{Z,R}_X 8xaa
    {   3,   15,  139,  266,   18, } , // 4 pipes (4 PKRs) 1 bpe @ SW_256K_{Z,R}_X 8xaa
    {   3,   16,  140,  214,   20, } , // 4 pipes (4 PKRs) 2 bpe @ SW_256K_{Z,R}_X 8xaa
    {   3,   17,  216,  267,   20, } , // 4 pipes (4 PKRs) 4 bpe @ SW_256K_{Z,R}_X 8xaa
    {   3,   18,  227,  303,   81, } , // 4 pipes (4 PKRs) 8 bpe @ SW_256K_{Z,R}_X 8xaa
    {   3,   19,  234,  330,  106, } , // 4 pipes (4 PKRs) 16 bpe @ SW_256K_{Z,R}_X 8xaa
    {   3,   15,  144,  262,   55, } , // 8 pipes (4 PKRs) 1 bpe @ SW_256K_{Z,R}_X 8xaa
    {   3,   16,  145,  218,   15, } , // 8 pipes (4 PKRs) 2 bpe @ SW_256K_{Z,R}_X 8xaa
    {   3,   17,  217,  268,   56, } , // 8 pipes (4 PKRs) 4 bpe @ SW_256K_{Z,R}_X 8xaa
    {   3,   18,  228,  305,   83, } , // 8 pipes (4 PKRs) 8 bpe @ SW_256K_{Z,R}_X 8xaa
    {   3,   19,  235,  331,  107, } , // 8 pipes (4 PKRs) 16 bpe @ SW_256K_{Z,R}_X 8xaa
    {   3,   15,  146,  271,   41, } , // 16 pipes (4 PKRs) 1 bpe @ SW_256K_{Z,R}_X 8xaa
    {   3,   16,  146,  272,   58, } , // 16 pipes (4 PKRs) 2 bpe @ SW_256K_{Z,R}_X 8xaa
    {   3,   17,  146,  273,   59, } , // 16 pipes (4 PKRs) 4 bpe @ SW_256K_{Z,R}_X 8xaa
    {   3,   18,  236,  332,  108, } , // 16 pipes (4 PKRs) 8 bpe @ SW_256K_{Z,R}_X 8xaa
    {   3,   19,  237,  333,  109, } , // 16 pipes (4 PKRs) 16 bpe @ SW_256K_{Z,R}_X 8xaa
    {   3,   15,  218,  275,   61, } , // 8 pipes (8 PKRs) 1 bpe @ SW_256K_{Z,R}_X 8xaa
    {   3,   16,  168,  228,   87, } , // 8 pipes (8 PKRs) 2 bpe @ SW_256K_{Z,R}_X 8xaa
    {   3,   17,  238,  276,  110, } , // 8 pipes (8 PKRs) 4 bpe @ SW_256K_{Z,R}_X 8xaa
    {   3,   18,  239,  308,  111, } , // 8 pipes (8 PKRs) 8 bpe @ SW_256K_{Z,R}_X 8xaa
    {   3,   19,  239,  334,  112, } , // 8 pipes (8 PKRs) 16 bpe @ SW_256K_{Z,R}_X 8xaa
    {   3,   15,  152,  278,   41, } , // 16 pipes (8 PKRs) 1 bpe @ SW_256K_{Z,R}_X 8xaa
    {   3,   16,  152,  230,   27, } , // 16 pipes (8 PKRs) 2 bpe @ SW_256K_{Z,R}_X 8xaa
    {   3,   17,  152,  279,   63, } , // 16 pipes (8 PKRs) 4 bpe @ SW_256K_{Z,R}_X 8xaa
    {   3,   18,  240,  309,   89, } , // 16 pipes (8 PKRs) 8 bpe @ SW_256K_{Z,R}_X 8xaa
    {   3,   19,  241,  335,  113, } , // 16 pipes (8 PKRs) 16 bpe @ SW_256K_{Z,R}_X 8xaa
    {   3,   15,  152,  281,   41, } , // 32 pipes (8 PKRs) 1 bpe @ SW_256K_{Z,R}_X 8xaa
    {   3,   16,  152,  282,   64, } , // 32 pipes (8 PKRs) 2 bpe @ SW_256K_{Z,R}_X 8xaa
    {   3,   17,  152,  283,   65, } , // 32 pipes (8 PKRs) 4 bpe @ SW_256K_{Z,R}_X 8xaa
    {   3,   18,  240,  311,   91, } , // 32 pipes (8 PKRs) 8 bpe @ SW_256K_{Z,R}_X 8xaa
    {   3,   19,  241,  336,   89, } , // 32 pipes (8 PKRs) 16 bpe @ SW_256K_{Z,R}_X 8xaa
    {   3,   15,  154,  239,   66, } , // 16 pipes (16 PKRs) 1 bpe @ SW_256K_{Z,R}_X 8xaa
    {   3,   16,  154,  240,   92, } , // 16 pipes (16 PKRs) 2 bpe @ SW_256K_{Z,R}_X 8xaa
    {   3,   17,  154,  273,   63, } , // 16 pipes (16 PKRs) 4 bpe @ SW_256K_{Z,R}_X 8xaa
    {   3,   18,  242,  313,   94, } , // 16 pipes (16 PKRs) 8 bpe @ SW_256K_{Z,R}_X 8xaa
    {   3,   19,  243,  337,  114, } , // 16 pipes (16 PKRs) 16 bpe @ SW_256K_{Z,R}_X 8xaa
    {   3,   15,  154,  281,   41, } , // 32 pipes (16 PKRs) 1 bpe @ SW_256K_{Z,R}_X 8xaa
    {   3,   16,  154,  314,   42, } , // 32 pipes (16 PKRs) 2 bpe @ SW_256K_{Z,R}_X 8xaa
    {   3,   17,  154,  338,   68, } , // 32 pipes (16 PKRs) 4 bpe @ SW_256K_{Z,R}_X 8xaa
    {   3,   18,  242,  316,   96, } , // 32 pipes (16 PKRs) 8 bpe @ SW_256K_{Z,R}_X 8xaa
    {   3,   19,  243,  339,  115, } , // 32 pipes (16 PKRs) 16 bpe @ SW_256K_{Z,R}_X 8xaa
    {   3,   15,  154,  286,   43, } , // 64 pipes (16 PKRs) 1 bpe @ SW_256K_{Z,R}_X 8xaa
    {   3,   16,  154,  317,   44, } , // 64 pipes (16 PKRs) 2 bpe @ SW_256K_{Z,R}_X 8xaa
    {   3,   17,  154,  340,   68, } , // 64 pipes (16 PKRs) 4 bpe @ SW_256K_{Z,R}_X 8xaa
    {   3,   18,  242,  341,  116, } , // 64 pipes (16 PKRs) 8 bpe @ SW_256K_{Z,R}_X 8xaa
    {   3,   19,  243,  342,  115, } , // 64 pipes (16 PKRs) 16 bpe @ SW_256K_{Z,R}_X 8xaa
    {   3,   15,  157,  289,   71, } , // 32 pipes (32 PKRs) 1 bpe @ SW_256K_{Z,R}_X 8xaa
    {   3,   16,  157,  320,   98, } , // 32 pipes (32 PKRs) 2 bpe @ SW_256K_{Z,R}_X 8xaa
    {   3,   17,  157,  343,  117, } , // 32 pipes (32 PKRs) 4 bpe @ SW_256K_{Z,R}_X 8xaa
    {   3,   18,  244,  322,  100, } , // 32 pipes (32 PKRs) 8 bpe @ SW_256K_{Z,R}_X 8xaa
    {   3,   19,  245,  344,  118, } , // 32 pipes (32 PKRs) 16 bpe @ SW_256K_{Z,R}_X 8xaa
    {   3,   15,  157,  292,   43, } , // 64 pipes (32 PKRs) 1 bpe @ SW_256K_{Z,R}_X 8xaa
    {   3,   16,  157,  323,   50, } , // 64 pipes (32 PKRs) 2 bpe @ SW_256K_{Z,R}_X 8xaa
    {   3,   17,  157,  345,  119, } , // 64 pipes (32 PKRs) 4 bpe @ SW_256K_{Z,R}_X 8xaa
    {   3,   18,  244,  325,  101, } , // 64 pipes (32 PKRs) 8 bpe @ SW_256K_{Z,R}_X 8xaa
    {   3,   19,  245,  346,  120, } , // 64 pipes (32 PKRs) 16 bpe @ SW_256K_{Z,R}_X 8xaa
};

const ADDR_SW_PATINFO GFX11_SW_4K_S3_PATINFO[] =
{
    {   1,   20,  246,    0,    0, } , // 1 pipes (1 PKRs) 1 bpe @ SW_4K_S3
    {   1,   21,  247,    0,    0, } , // 1 pipes (1 PKRs) 2 bpe @ SW_4K_S3
    {   1,   22,  248,    0,    0, } , // 1 pipes (1 PKRs) 4 bpe @ SW_4K_S3
    {   1,   23,  249,    0,    0, } , // 1 pipes (1 PKRs) 8 bpe @ SW_4K_S3
    {   1,   24,  250,    0,    0, } , // 1 pipes (1 PKRs) 16 bpe @ SW_4K_S3
    {   1,   20,  246,    0,    0, } , // 2 pipes (1-2 PKRs) 1 bpe @ SW_4K_S3
    {   1,   21,  247,    0,    0, } , // 2 pipes (1-2 PKRs) 2 bpe @ SW_4K_S3
    {   1,   22,  248,    0,    0, } , // 2 pipes (1-2 PKRs) 4 bpe @ SW_4K_S3
    {   1,   23,  249,    0,    0, } , // 2 pipes (1-2 PKRs) 8 bpe @ SW_4K_S3
    {   1,   24,  250,    0,    0, } , // 2 pipes (1-2 PKRs) 16 bpe @ SW_4K_S3
    {   1,   20,  246,    0,    0, } , // 4 pipes (1-2 PKRs) 1 bpe @ SW_4K_S3
    {   1,   21,  247,    0,    0, } , // 4 pipes (1-2 PKRs) 2 bpe @ SW_4K_S3
    {   1,   22,  248,    0,    0, } , // 4 pipes (1-2 PKRs) 4 bpe @ SW_4K_S3
    {   1,   23,  249,    0,    0, } , // 4 pipes (1-2 PKRs) 8 bpe @ SW_4K_S3
    {   1,   24,  250,    0,    0, } , // 4 pipes (1-2 PKRs) 16 bpe @ SW_4K_S3
    {   1,   20,  246,    0,    0, } , // 8 pipes (2 PKRs) 1 bpe @ SW_4K_S3
    {   1,   21,  247,    0,    0, } , // 8 pipes (2 PKRs) 2 bpe @ SW_4K_S3
    {   1,   22,  248,    0,    0, } , // 8 pipes (2 PKRs) 4 bpe @ SW_4K_S3
    {   1,   23,  249,    0,    0, } , // 8 pipes (2 PKRs) 8 bpe @ SW_4K_S3
    {   1,   24,  250,    0,    0, } , // 8 pipes (2 PKRs) 16 bpe @ SW_4K_S3
    {   1,   20,  246,    0,    0, } , // 4 pipes (4 PKRs) 1 bpe @ SW_4K_S3
    {   1,   21,  247,    0,    0, } , // 4 pipes (4 PKRs) 2 bpe @ SW_4K_S3
    {   1,   22,  248,    0,    0, } , // 4 pipes (4 PKRs) 4 bpe @ SW_4K_S3
    {   1,   23,  249,    0,    0, } , // 4 pipes (4 PKRs) 8 bpe @ SW_4K_S3
    {   1,   24,  250,    0,    0, } , // 4 pipes (4 PKRs) 16 bpe @ SW_4K_S3
    {   1,   20,  246,    0,    0, } , // 8 pipes (4 PKRs) 1 bpe @ SW_4K_S3
    {   1,   21,  247,    0,    0, } , // 8 pipes (4 PKRs) 2 bpe @ SW_4K_S3
    {   1,   22,  248,    0,    0, } , // 8 pipes (4 PKRs) 4 bpe @ SW_4K_S3
    {   1,   23,  249,    0,    0, } , // 8 pipes (4 PKRs) 8 bpe @ SW_4K_S3
    {   1,   24,  250,    0,    0, } , // 8 pipes (4 PKRs) 16 bpe @ SW_4K_S3
    {   1,   20,  246,    0,    0, } , // 16 pipes (4 PKRs) 1 bpe @ SW_4K_S3
    {   1,   21,  247,    0,    0, } , // 16 pipes (4 PKRs) 2 bpe @ SW_4K_S3
    {   1,   22,  248,    0,    0, } , // 16 pipes (4 PKRs) 4 bpe @ SW_4K_S3
    {   1,   23,  249,    0,    0, } , // 16 pipes (4 PKRs) 8 bpe @ SW_4K_S3
    {   1,   24,  250,    0,    0, } , // 16 pipes (4 PKRs) 16 bpe @ SW_4K_S3
    {   1,   20,  246,    0,    0, } , // 8 pipes (8 PKRs) 1 bpe @ SW_4K_S3
    {   1,   21,  247,    0,    0, } , // 8 pipes (8 PKRs) 2 bpe @ SW_4K_S3
    {   1,   22,  248,    0,    0, } , // 8 pipes (8 PKRs) 4 bpe @ SW_4K_S3
    {   1,   23,  249,    0,    0, } , // 8 pipes (8 PKRs) 8 bpe @ SW_4K_S3
    {   1,   24,  250,    0,    0, } , // 8 pipes (8 PKRs) 16 bpe @ SW_4K_S3
    {   1,   20,  246,    0,    0, } , // 16 pipes (8 PKRs) 1 bpe @ SW_4K_S3
    {   1,   21,  247,    0,    0, } , // 16 pipes (8 PKRs) 2 bpe @ SW_4K_S3
    {   1,   22,  248,    0,    0, } , // 16 pipes (8 PKRs) 4 bpe @ SW_4K_S3
    {   1,   23,  249,    0,    0, } , // 16 pipes (8 PKRs) 8 bpe @ SW_4K_S3
    {   1,   24,  250,    0,    0, } , // 16 pipes (8 PKRs) 16 bpe @ SW_4K_S3
    {   1,   20,  246,    0,    0, } , // 32 pipes (8 PKRs) 1 bpe @ SW_4K_S3
    {   1,   21,  247,    0,    0, } , // 32 pipes (8 PKRs) 2 bpe @ SW_4K_S3
    {   1,   22,  248,    0,    0, } , // 32 pipes (8 PKRs) 4 bpe @ SW_4K_S3
    {   1,   23,  249,    0,    0, } , // 32 pipes (8 PKRs) 8 bpe @ SW_4K_S3
    {   1,   24,  250,    0,    0, } , // 32 pipes (8 PKRs) 16 bpe @ SW_4K_S3
    {   1,   20,  246,    0,    0, } , // 16 pipes (16 PKRs) 1 bpe @ SW_4K_S3
    {   1,   21,  247,    0,    0, } , // 16 pipes (16 PKRs) 2 bpe @ SW_4K_S3
    {   1,   22,  248,    0,    0, } , // 16 pipes (16 PKRs) 4 bpe @ SW_4K_S3
    {   1,   23,  249,    0,    0, } , // 16 pipes (16 PKRs) 8 bpe @ SW_4K_S3
    {   1,   24,  250,    0,    0, } , // 16 pipes (16 PKRs) 16 bpe @ SW_4K_S3
    {   1,   20,  246,    0,    0, } , // 32 pipes (16 PKRs) 1 bpe @ SW_4K_S3
    {   1,   21,  247,    0,    0, } , // 32 pipes (16 PKRs) 2 bpe @ SW_4K_S3
    {   1,   22,  248,    0,    0, } , // 32 pipes (16 PKRs) 4 bpe @ SW_4K_S3
    {   1,   23,  249,    0,    0, } , // 32 pipes (16 PKRs) 8 bpe @ SW_4K_S3
    {   1,   24,  250,    0,    0, } , // 32 pipes (16 PKRs) 16 bpe @ SW_4K_S3
    {   1,   20,  246,    0,    0, } , // 64 pipes (16 PKRs) 1 bpe @ SW_4K_S3
    {   1,   21,  247,    0,    0, } , // 64 pipes (16 PKRs) 2 bpe @ SW_4K_S3
    {   1,   22,  248,    0,    0, } , // 64 pipes (16 PKRs) 4 bpe @ SW_4K_S3
    {   1,   23,  249,    0,    0, } , // 64 pipes (16 PKRs) 8 bpe @ SW_4K_S3
    {   1,   24,  250,    0,    0, } , // 64 pipes (16 PKRs) 16 bpe @ SW_4K_S3
    {   1,   20,  246,    0,    0, } , // 32 pipes (32 PKRs) 1 bpe @ SW_4K_S3
    {   1,   21,  247,    0,    0, } , // 32 pipes (32 PKRs) 2 bpe @ SW_4K_S3
    {   1,   22,  248,    0,    0, } , // 32 pipes (32 PKRs) 4 bpe @ SW_4K_S3
    {   1,   23,  249,    0,    0, } , // 32 pipes (32 PKRs) 8 bpe @ SW_4K_S3
    {   1,   24,  250,    0,    0, } , // 32 pipes (32 PKRs) 16 bpe @ SW_4K_S3
    {   1,   20,  246,    0,    0, } , // 64 pipes (32 PKRs) 1 bpe @ SW_4K_S3
    {   1,   21,  247,    0,    0, } , // 64 pipes (32 PKRs) 2 bpe @ SW_4K_S3
    {   1,   22,  248,    0,    0, } , // 64 pipes (32 PKRs) 4 bpe @ SW_4K_S3
    {   1,   23,  249,    0,    0, } , // 64 pipes (32 PKRs) 8 bpe @ SW_4K_S3
    {   1,   24,  250,    0,    0, } , // 64 pipes (32 PKRs) 16 bpe @ SW_4K_S3
};

const ADDR_SW_PATINFO GFX11_SW_4K_S3_X_PATINFO[] =
{
    {   1,   20,  246,    0,    0, } , // 1 pipes (1 PKRs) 1 bpe @ SW_4K_S3_X
    {   1,   21,  247,    0,    0, } , // 1 pipes (1 PKRs) 2 bpe @ SW_4K_S3_X
    {   1,   22,  248,    0,    0, } , // 1 pipes (1 PKRs) 4 bpe @ SW_4K_S3_X
    {   1,   23,  249,    0,    0, } , // 1 pipes (1 PKRs) 8 bpe @ SW_4K_S3_X
    {   1,   24,  250,    0,    0, } , // 1 pipes (1 PKRs) 16 bpe @ SW_4K_S3_X
    {   3,   20,  251,    0,    0, } , // 2 pipes (1-2 PKRs) 1 bpe @ SW_4K_S3_X
    {   3,   21,  252,    0,    0, } , // 2 pipes (1-2 PKRs) 2 bpe @ SW_4K_S3_X
    {   3,   22,  253,    0,    0, } , // 2 pipes (1-2 PKRs) 4 bpe @ SW_4K_S3_X
    {   3,   23,  254,    0,    0, } , // 2 pipes (1-2 PKRs) 8 bpe @ SW_4K_S3_X
    {   3,   24,  255,    0,    0, } , // 2 pipes (1-2 PKRs) 16 bpe @ SW_4K_S3_X
    {   3,   20,  256,    0,    0, } , // 4 pipes (1-2 PKRs) 1 bpe @ SW_4K_S3_X
    {   3,   21,  257,    0,    0, } , // 4 pipes (1-2 PKRs) 2 bpe @ SW_4K_S3_X
    {   3,   22,  258,    0,    0, } , // 4 pipes (1-2 PKRs) 4 bpe @ SW_4K_S3_X
    {   3,   23,  259,    0,    0, } , // 4 pipes (1-2 PKRs) 8 bpe @ SW_4K_S3_X
    {   3,   24,  260,    0,    0, } , // 4 pipes (1-2 PKRs) 16 bpe @ SW_4K_S3_X
    {   3,   20,  261,    0,    0, } , // 8 pipes (2 PKRs) 1 bpe @ SW_4K_S3_X
    {   3,   21,  262,    0,    0, } , // 8 pipes (2 PKRs) 2 bpe @ SW_4K_S3_X
    {   3,   22,  263,    0,    0, } , // 8 pipes (2 PKRs) 4 bpe @ SW_4K_S3_X
    {   3,   23,  264,    0,    0, } , // 8 pipes (2 PKRs) 8 bpe @ SW_4K_S3_X
    {   3,   24,  265,    0,    0, } , // 8 pipes (2 PKRs) 16 bpe @ SW_4K_S3_X
    {   3,   20,  256,    0,    0, } , // 4 pipes (4 PKRs) 1 bpe @ SW_4K_S3_X
    {   3,   21,  257,    0,    0, } , // 4 pipes (4 PKRs) 2 bpe @ SW_4K_S3_X
    {   3,   22,  258,    0,    0, } , // 4 pipes (4 PKRs) 4 bpe @ SW_4K_S3_X
    {   3,   23,  259,    0,    0, } , // 4 pipes (4 PKRs) 8 bpe @ SW_4K_S3_X
    {   3,   24,  260,    0,    0, } , // 4 pipes (4 PKRs) 16 bpe @ SW_4K_S3_X
    {   3,   20,  261,    0,    0, } , // 8 pipes (4 PKRs) 1 bpe @ SW_4K_S3_X
    {   3,   21,  262,    0,    0, } , // 8 pipes (4 PKRs) 2 bpe @ SW_4K_S3_X
    {   3,   22,  263,    0,    0, } , // 8 pipes (4 PKRs) 4 bpe @ SW_4K_S3_X
    {   3,   23,  264,    0,    0, } , // 8 pipes (4 PKRs) 8 bpe @ SW_4K_S3_X
    {   3,   24,  265,    0,    0, } , // 8 pipes (4 PKRs) 16 bpe @ SW_4K_S3_X
    {   3,   20,  266,    0,    0, } , // 16 pipes (4 PKRs) 1 bpe @ SW_4K_S3_X
    {   3,   21,  267,    0,    0, } , // 16 pipes (4 PKRs) 2 bpe @ SW_4K_S3_X
    {   3,   22,  268,    0,    0, } , // 16 pipes (4 PKRs) 4 bpe @ SW_4K_S3_X
    {   3,   23,  269,    0,    0, } , // 16 pipes (4 PKRs) 8 bpe @ SW_4K_S3_X
    {   3,   24,  270,    0,    0, } , // 16 pipes (4 PKRs) 16 bpe @ SW_4K_S3_X
    {   3,   20,  261,    0,    0, } , // 8 pipes (8 PKRs) 1 bpe @ SW_4K_S3_X
    {   3,   21,  262,    0,    0, } , // 8 pipes (8 PKRs) 2 bpe @ SW_4K_S3_X
    {   3,   22,  263,    0,    0, } , // 8 pipes (8 PKRs) 4 bpe @ SW_4K_S3_X
    {   3,   23,  264,    0,    0, } , // 8 pipes (8 PKRs) 8 bpe @ SW_4K_S3_X
    {   3,   24,  265,    0,    0, } , // 8 pipes (8 PKRs) 16 bpe @ SW_4K_S3_X
    {   3,   20,  266,    0,    0, } , // 16 pipes (8 PKRs) 1 bpe @ SW_4K_S3_X
    {   3,   21,  267,    0,    0, } , // 16 pipes (8 PKRs) 2 bpe @ SW_4K_S3_X
    {   3,   22,  268,    0,    0, } , // 16 pipes (8 PKRs) 4 bpe @ SW_4K_S3_X
    {   3,   23,  269,    0,    0, } , // 16 pipes (8 PKRs) 8 bpe @ SW_4K_S3_X
    {   3,   24,  270,    0,    0, } , // 16 pipes (8 PKRs) 16 bpe @ SW_4K_S3_X
    {   3,   20,  266,    0,    0, } , // 32 pipes (8 PKRs) 1 bpe @ SW_4K_S3_X
    {   3,   21,  267,    0,    0, } , // 32 pipes (8 PKRs) 2 bpe @ SW_4K_S3_X
    {   3,   22,  268,    0,    0, } , // 32 pipes (8 PKRs) 4 bpe @ SW_4K_S3_X
    {   3,   23,  269,    0,    0, } , // 32 pipes (8 PKRs) 8 bpe @ SW_4K_S3_X
    {   3,   24,  270,    0,    0, } , // 32 pipes (8 PKRs) 16 bpe @ SW_4K_S3_X
    {   3,   20,  266,    0,    0, } , // 16 pipes (16 PKRs) 1 bpe @ SW_4K_S3_X
    {   3,   21,  267,    0,    0, } , // 16 pipes (16 PKRs) 2 bpe @ SW_4K_S3_X
    {   3,   22,  268,    0,    0, } , // 16 pipes (16 PKRs) 4 bpe @ SW_4K_S3_X
    {   3,   23,  269,    0,    0, } , // 16 pipes (16 PKRs) 8 bpe @ SW_4K_S3_X
    {   3,   24,  270,    0,    0, } , // 16 pipes (16 PKRs) 16 bpe @ SW_4K_S3_X
    {   3,   20,  266,    0,    0, } , // 32 pipes (16 PKRs) 1 bpe @ SW_4K_S3_X
    {   3,   21,  267,    0,    0, } , // 32 pipes (16 PKRs) 2 bpe @ SW_4K_S3_X
    {   3,   22,  268,    0,    0, } , // 32 pipes (16 PKRs) 4 bpe @ SW_4K_S3_X
    {   3,   23,  269,    0,    0, } , // 32 pipes (16 PKRs) 8 bpe @ SW_4K_S3_X
    {   3,   24,  270,    0,    0, } , // 32 pipes (16 PKRs) 16 bpe @ SW_4K_S3_X
    {   3,   20,  266,    0,    0, } , // 64 pipes (16 PKRs) 1 bpe @ SW_4K_S3_X
    {   3,   21,  267,    0,    0, } , // 64 pipes (16 PKRs) 2 bpe @ SW_4K_S3_X
    {   3,   22,  268,    0,    0, } , // 64 pipes (16 PKRs) 4 bpe @ SW_4K_S3_X
    {   3,   23,  269,    0,    0, } , // 64 pipes (16 PKRs) 8 bpe @ SW_4K_S3_X
    {   3,   24,  270,    0,    0, } , // 64 pipes (16 PKRs) 16 bpe @ SW_4K_S3_X
    {   3,   20,  266,    0,    0, } , // 32 pipes (32 PKRs) 1 bpe @ SW_4K_S3_X
    {   3,   21,  267,    0,    0, } , // 32 pipes (32 PKRs) 2 bpe @ SW_4K_S3_X
    {   3,   22,  268,    0,    0, } , // 32 pipes (32 PKRs) 4 bpe @ SW_4K_S3_X
    {   3,   23,  269,    0,    0, } , // 32 pipes (32 PKRs) 8 bpe @ SW_4K_S3_X
    {   3,   24,  270,    0,    0, } , // 32 pipes (32 PKRs) 16 bpe @ SW_4K_S3_X
    {   3,   20,  266,    0,    0, } , // 64 pipes (32 PKRs) 1 bpe @ SW_4K_S3_X
    {   3,   21,  267,    0,    0, } , // 64 pipes (32 PKRs) 2 bpe @ SW_4K_S3_X
    {   3,   22,  268,    0,    0, } , // 64 pipes (32 PKRs) 4 bpe @ SW_4K_S3_X
    {   3,   23,  269,    0,    0, } , // 64 pipes (32 PKRs) 8 bpe @ SW_4K_S3_X
    {   3,   24,  270,    0,    0, } , // 64 pipes (32 PKRs) 16 bpe @ SW_4K_S3_X
};

const ADDR_SW_PATINFO GFX11_SW_64K_S3_PATINFO[] =
{
    {   1,   20,  246,  347,    0, } , // 1 pipes (1 PKRs) 1 bpe @ SW_64K_S3
    {   1,   21,  247,  348,    0, } , // 1 pipes (1 PKRs) 2 bpe @ SW_64K_S3
    {   1,   22,  248,  349,    0, } , // 1 pipes (1 PKRs) 4 bpe @ SW_64K_S3
    {   1,   23,  249,  350,    0, } , // 1 pipes (1 PKRs) 8 bpe @ SW_64K_S3
    {   1,   24,  250,  351,    0, } , // 1 pipes (1 PKRs) 16 bpe @ SW_64K_S3
    {   1,   20,  246,  347,    0, } , // 2 pipes (1-2 PKRs) 1 bpe @ SW_64K_S3
    {   1,   21,  247,  348,    0, } , // 2 pipes (1-2 PKRs) 2 bpe @ SW_64K_S3
    {   1,   22,  248,  349,    0, } , // 2 pipes (1-2 PKRs) 4 bpe @ SW_64K_S3
    {   1,   23,  249,  350,    0, } , // 2 pipes (1-2 PKRs) 8 bpe @ SW_64K_S3
    {   1,   24,  250,  351,    0, } , // 2 pipes (1-2 PKRs) 16 bpe @ SW_64K_S3
    {   1,   20,  246,  347,    0, } , // 4 pipes (1-2 PKRs) 1 bpe @ SW_64K_S3
    {   1,   21,  247,  348,    0, } , // 4 pipes (1-2 PKRs) 2 bpe @ SW_64K_S3
    {   1,   22,  248,  349,    0, } , // 4 pipes (1-2 PKRs) 4 bpe @ SW_64K_S3
    {   1,   23,  249,  350,    0, } , // 4 pipes (1-2 PKRs) 8 bpe @ SW_64K_S3
    {   1,   24,  250,  351,    0, } , // 4 pipes (1-2 PKRs) 16 bpe @ SW_64K_S3
    {   1,   20,  246,  347,    0, } , // 8 pipes (2 PKRs) 1 bpe @ SW_64K_S3
    {   1,   21,  247,  348,    0, } , // 8 pipes (2 PKRs) 2 bpe @ SW_64K_S3
    {   1,   22,  248,  349,    0, } , // 8 pipes (2 PKRs) 4 bpe @ SW_64K_S3
    {   1,   23,  249,  350,    0, } , // 8 pipes (2 PKRs) 8 bpe @ SW_64K_S3
    {   1,   24,  250,  351,    0, } , // 8 pipes (2 PKRs) 16 bpe @ SW_64K_S3
    {   1,   20,  246,  347,    0, } , // 4 pipes (4 PKRs) 1 bpe @ SW_64K_S3
    {   1,   21,  247,  348,    0, } , // 4 pipes (4 PKRs) 2 bpe @ SW_64K_S3
    {   1,   22,  248,  349,    0, } , // 4 pipes (4 PKRs) 4 bpe @ SW_64K_S3
    {   1,   23,  249,  350,    0, } , // 4 pipes (4 PKRs) 8 bpe @ SW_64K_S3
    {   1,   24,  250,  351,    0, } , // 4 pipes (4 PKRs) 16 bpe @ SW_64K_S3
    {   1,   20,  246,  347,    0, } , // 8 pipes (4 PKRs) 1 bpe @ SW_64K_S3
    {   1,   21,  247,  348,    0, } , // 8 pipes (4 PKRs) 2 bpe @ SW_64K_S3
    {   1,   22,  248,  349,    0, } , // 8 pipes (4 PKRs) 4 bpe @ SW_64K_S3
    {   1,   23,  249,  350,    0, } , // 8 pipes (4 PKRs) 8 bpe @ SW_64K_S3
    {   1,   24,  250,  351,    0, } , // 8 pipes (4 PKRs) 16 bpe @ SW_64K_S3
    {   1,   20,  246,  347,    0, } , // 16 pipes (4 PKRs) 1 bpe @ SW_64K_S3
    {   1,   21,  247,  348,    0, } , // 16 pipes (4 PKRs) 2 bpe @ SW_64K_S3
    {   1,   22,  248,  349,    0, } , // 16 pipes (4 PKRs) 4 bpe @ SW_64K_S3
    {   1,   23,  249,  350,    0, } , // 16 pipes (4 PKRs) 8 bpe @ SW_64K_S3
    {   1,   24,  250,  351,    0, } , // 16 pipes (4 PKRs) 16 bpe @ SW_64K_S3
    {   1,   20,  246,  347,    0, } , // 8 pipes (8 PKRs) 1 bpe @ SW_64K_S3
    {   1,   21,  247,  348,    0, } , // 8 pipes (8 PKRs) 2 bpe @ SW_64K_S3
    {   1,   22,  248,  349,    0, } , // 8 pipes (8 PKRs) 4 bpe @ SW_64K_S3
    {   1,   23,  249,  350,    0, } , // 8 pipes (8 PKRs) 8 bpe @ SW_64K_S3
    {   1,   24,  250,  351,    0, } , // 8 pipes (8 PKRs) 16 bpe @ SW_64K_S3
    {   1,   20,  246,  347,    0, } , // 16 pipes (8 PKRs) 1 bpe @ SW_64K_S3
    {   1,   21,  247,  348,    0, } , // 16 pipes (8 PKRs) 2 bpe @ SW_64K_S3
    {   1,   22,  248,  349,    0, } , // 16 pipes (8 PKRs) 4 bpe @ SW_64K_S3
    {   1,   23,  249,  350,    0, } , // 16 pipes (8 PKRs) 8 bpe @ SW_64K_S3
    {   1,   24,  250,  351,    0, } , // 16 pipes (8 PKRs) 16 bpe @ SW_64K_S3
    {   1,   20,  246,  347,    0, } , // 32 pipes (8 PKRs) 1 bpe @ SW_64K_S3
    {   1,   21,  247,  348,    0, } , // 32 pipes (8 PKRs) 2 bpe @ SW_64K_S3
    {   1,   22,  248,  349,    0, } , // 32 pipes (8 PKRs) 4 bpe @ SW_64K_S3
    {   1,   23,  249,  350,    0, } , // 32 pipes (8 PKRs) 8 bpe @ SW_64K_S3
    {   1,   24,  250,  351,    0, } , // 32 pipes (8 PKRs) 16 bpe @ SW_64K_S3
    {   1,   20,  246,  347,    0, } , // 16 pipes (16 PKRs) 1 bpe @ SW_64K_S3
    {   1,   21,  247,  348,    0, } , // 16 pipes (16 PKRs) 2 bpe @ SW_64K_S3
    {   1,   22,  248,  349,    0, } , // 16 pipes (16 PKRs) 4 bpe @ SW_64K_S3
    {   1,   23,  249,  350,    0, } , // 16 pipes (16 PKRs) 8 bpe @ SW_64K_S3
    {   1,   24,  250,  351,    0, } , // 16 pipes (16 PKRs) 16 bpe @ SW_64K_S3
    {   1,   20,  246,  347,    0, } , // 32 pipes (16 PKRs) 1 bpe @ SW_64K_S3
    {   1,   21,  247,  348,    0, } , // 32 pipes (16 PKRs) 2 bpe @ SW_64K_S3
    {   1,   22,  248,  349,    0, } , // 32 pipes (16 PKRs) 4 bpe @ SW_64K_S3
    {   1,   23,  249,  350,    0, } , // 32 pipes (16 PKRs) 8 bpe @ SW_64K_S3
    {   1,   24,  250,  351,    0, } , // 32 pipes (16 PKRs) 16 bpe @ SW_64K_S3
    {   1,   20,  246,  347,    0, } , // 64 pipes (16 PKRs) 1 bpe @ SW_64K_S3
    {   1,   21,  247,  348,    0, } , // 64 pipes (16 PKRs) 2 bpe @ SW_64K_S3
    {   1,   22,  248,  349,    0, } , // 64 pipes (16 PKRs) 4 bpe @ SW_64K_S3
    {   1,   23,  249,  350,    0, } , // 64 pipes (16 PKRs) 8 bpe @ SW_64K_S3
    {   1,   24,  250,  351,    0, } , // 64 pipes (16 PKRs) 16 bpe @ SW_64K_S3
    {   1,   20,  246,  347,    0, } , // 32 pipes (32 PKRs) 1 bpe @ SW_64K_S3
    {   1,   21,  247,  348,    0, } , // 32 pipes (32 PKRs) 2 bpe @ SW_64K_S3
    {   1,   22,  248,  349,    0, } , // 32 pipes (32 PKRs) 4 bpe @ SW_64K_S3
    {   1,   23,  249,  350,    0, } , // 32 pipes (32 PKRs) 8 bpe @ SW_64K_S3
    {   1,   24,  250,  351,    0, } , // 32 pipes (32 PKRs) 16 bpe @ SW_64K_S3
    {   1,   20,  246,  347,    0, } , // 64 pipes (32 PKRs) 1 bpe @ SW_64K_S3
    {   1,   21,  247,  348,    0, } , // 64 pipes (32 PKRs) 2 bpe @ SW_64K_S3
    {   1,   22,  248,  349,    0, } , // 64 pipes (32 PKRs) 4 bpe @ SW_64K_S3
    {   1,   23,  249,  350,    0, } , // 64 pipes (32 PKRs) 8 bpe @ SW_64K_S3
    {   1,   24,  250,  351,    0, } , // 64 pipes (32 PKRs) 16 bpe @ SW_64K_S3
};

const ADDR_SW_PATINFO GFX11_SW_64K_S3_X_PATINFO[] =
{
    {   1,   20,  246,  347,    0, } , // 1 pipes (1 PKRs) 1 bpe @ SW_64K_S3_X
    {   1,   21,  247,  348,    0, } , // 1 pipes (1 PKRs) 2 bpe @ SW_64K_S3_X
    {   1,   22,  248,  349,    0, } , // 1 pipes (1 PKRs) 4 bpe @ SW_64K_S3_X
    {   1,   23,  249,  350,    0, } , // 1 pipes (1 PKRs) 8 bpe @ SW_64K_S3_X
    {   1,   24,  250,  351,    0, } , // 1 pipes (1 PKRs) 16 bpe @ SW_64K_S3_X
    {   3,   20,  251,  347,    0, } , // 2 pipes (1-2 PKRs) 1 bpe @ SW_64K_S3_X
    {   3,   21,  252,  348,    0, } , // 2 pipes (1-2 PKRs) 2 bpe @ SW_64K_S3_X
    {   3,   22,  253,  349,    0, } , // 2 pipes (1-2 PKRs) 4 bpe @ SW_64K_S3_X
    {   3,   23,  254,  350,    0, } , // 2 pipes (1-2 PKRs) 8 bpe @ SW_64K_S3_X
    {   3,   24,  255,  351,    0, } , // 2 pipes (1-2 PKRs) 16 bpe @ SW_64K_S3_X
    {   3,   20,  256,  347,    0, } , // 4 pipes (1-2 PKRs) 1 bpe @ SW_64K_S3_X
    {   3,   21,  257,  348,    0, } , // 4 pipes (1-2 PKRs) 2 bpe @ SW_64K_S3_X
    {   3,   22,  258,  349,    0, } , // 4 pipes (1-2 PKRs) 4 bpe @ SW_64K_S3_X
    {   3,   23,  259,  350,    0, } , // 4 pipes (1-2 PKRs) 8 bpe @ SW_64K_S3_X
    {   3,   24,  260,  351,    0, } , // 4 pipes (1-2 PKRs) 16 bpe @ SW_64K_S3_X
    {   3,   20,  261,  347,    0, } , // 8 pipes (2 PKRs) 1 bpe @ SW_64K_S3_X
    {   3,   21,  262,  348,    0, } , // 8 pipes (2 PKRs) 2 bpe @ SW_64K_S3_X
    {   3,   22,  263,  349,    0, } , // 8 pipes (2 PKRs) 4 bpe @ SW_64K_S3_X
    {   3,   23,  264,  350,    0, } , // 8 pipes (2 PKRs) 8 bpe @ SW_64K_S3_X
    {   3,   24,  265,  351,    0, } , // 8 pipes (2 PKRs) 16 bpe @ SW_64K_S3_X
    {   3,   20,  256,  347,    0, } , // 4 pipes (4 PKRs) 1 bpe @ SW_64K_S3_X
    {   3,   21,  257,  348,    0, } , // 4 pipes (4 PKRs) 2 bpe @ SW_64K_S3_X
    {   3,   22,  258,  349,    0, } , // 4 pipes (4 PKRs) 4 bpe @ SW_64K_S3_X
    {   3,   23,  259,  350,    0, } , // 4 pipes (4 PKRs) 8 bpe @ SW_64K_S3_X
    {   3,   24,  260,  351,    0, } , // 4 pipes (4 PKRs) 16 bpe @ SW_64K_S3_X
    {   3,   20,  261,  347,    0, } , // 8 pipes (4 PKRs) 1 bpe @ SW_64K_S3_X
    {   3,   21,  262,  348,    0, } , // 8 pipes (4 PKRs) 2 bpe @ SW_64K_S3_X
    {   3,   22,  263,  349,    0, } , // 8 pipes (4 PKRs) 4 bpe @ SW_64K_S3_X
    {   3,   23,  264,  350,    0, } , // 8 pipes (4 PKRs) 8 bpe @ SW_64K_S3_X
    {   3,   24,  265,  351,    0, } , // 8 pipes (4 PKRs) 16 bpe @ SW_64K_S3_X
    {   3,   20,  266,  347,    0, } , // 16 pipes (4 PKRs) 1 bpe @ SW_64K_S3_X
    {   3,   21,  267,  348,    0, } , // 16 pipes (4 PKRs) 2 bpe @ SW_64K_S3_X
    {   3,   22,  268,  349,    0, } , // 16 pipes (4 PKRs) 4 bpe @ SW_64K_S3_X
    {   3,   23,  269,  350,    0, } , // 16 pipes (4 PKRs) 8 bpe @ SW_64K_S3_X
    {   3,   24,  270,  351,    0, } , // 16 pipes (4 PKRs) 16 bpe @ SW_64K_S3_X
    {   3,   20,  261,  347,    0, } , // 8 pipes (8 PKRs) 1 bpe @ SW_64K_S3_X
    {   3,   21,  262,  348,    0, } , // 8 pipes (8 PKRs) 2 bpe @ SW_64K_S3_X
    {   3,   22,  263,  349,    0, } , // 8 pipes (8 PKRs) 4 bpe @ SW_64K_S3_X
    {   3,   23,  264,  350,    0, } , // 8 pipes (8 PKRs) 8 bpe @ SW_64K_S3_X
    {   3,   24,  265,  351,    0, } , // 8 pipes (8 PKRs) 16 bpe @ SW_64K_S3_X
    {   3,   20,  266,  347,    0, } , // 16 pipes (8 PKRs) 1 bpe @ SW_64K_S3_X
    {   3,   21,  267,  348,    0, } , // 16 pipes (8 PKRs) 2 bpe @ SW_64K_S3_X
    {   3,   22,  268,  349,    0, } , // 16 pipes (8 PKRs) 4 bpe @ SW_64K_S3_X
    {   3,   23,  269,  350,    0, } , // 16 pipes (8 PKRs) 8 bpe @ SW_64K_S3_X
    {   3,   24,  270,  351,    0, } , // 16 pipes (8 PKRs) 16 bpe @ SW_64K_S3_X
    {   3,   20,  271,  352,    0, } , // 32 pipes (8 PKRs) 1 bpe @ SW_64K_S3_X
    {   3,   21,  272,  353,    0, } , // 32 pipes (8 PKRs) 2 bpe @ SW_64K_S3_X
    {   3,   22,  273,  354,    0, } , // 32 pipes (8 PKRs) 4 bpe @ SW_64K_S3_X
    {   3,   23,  274,  355,    0, } , // 32 pipes (8 PKRs) 8 bpe @ SW_64K_S3_X
    {   3,   24,  275,  356,    0, } , // 32 pipes (8 PKRs) 16 bpe @ SW_64K_S3_X
    {   3,   20,  266,  347,    0, } , // 16 pipes (16 PKRs) 1 bpe @ SW_64K_S3_X
    {   3,   21,  267,  348,    0, } , // 16 pipes (16 PKRs) 2 bpe @ SW_64K_S3_X
    {   3,   22,  268,  349,    0, } , // 16 pipes (16 PKRs) 4 bpe @ SW_64K_S3_X
    {   3,   23,  269,  350,    0, } , // 16 pipes (16 PKRs) 8 bpe @ SW_64K_S3_X
    {   3,   24,  270,  351,    0, } , // 16 pipes (16 PKRs) 16 bpe @ SW_64K_S3_X
    {   3,   20,  271,  352,    0, } , // 32 pipes (16 PKRs) 1 bpe @ SW_64K_S3_X
    {   3,   21,  272,  353,    0, } , // 32 pipes (16 PKRs) 2 bpe @ SW_64K_S3_X
    {   3,   22,  273,  354,    0, } , // 32 pipes (16 PKRs) 4 bpe @ SW_64K_S3_X
    {   3,   23,  274,  355,    0, } , // 32 pipes (16 PKRs) 8 bpe @ SW_64K_S3_X
    {   3,   24,  275,  356,    0, } , // 32 pipes (16 PKRs) 16 bpe @ SW_64K_S3_X
    {   3,   20,  276,  357,    0, } , // 64 pipes (16 PKRs) 1 bpe @ SW_64K_S3_X
    {   3,   21,  277,  358,    0, } , // 64 pipes (16 PKRs) 2 bpe @ SW_64K_S3_X
    {   3,   22,  278,  359,    0, } , // 64 pipes (16 PKRs) 4 bpe @ SW_64K_S3_X
    {   3,   23,  279,  360,    0, } , // 64 pipes (16 PKRs) 8 bpe @ SW_64K_S3_X
    {   3,   24,  280,  361,    0, } , // 64 pipes (16 PKRs) 16 bpe @ SW_64K_S3_X
    {   3,   20,  271,  352,    0, } , // 32 pipes (32 PKRs) 1 bpe @ SW_64K_S3_X
    {   3,   21,  272,  353,    0, } , // 32 pipes (32 PKRs) 2 bpe @ SW_64K_S3_X
    {   3,   22,  273,  354,    0, } , // 32 pipes (32 PKRs) 4 bpe @ SW_64K_S3_X
    {   3,   23,  274,  355,    0, } , // 32 pipes (32 PKRs) 8 bpe @ SW_64K_S3_X
    {   3,   24,  275,  356,    0, } , // 32 pipes (32 PKRs) 16 bpe @ SW_64K_S3_X
    {   3,   20,  276,  357,    0, } , // 64 pipes (32 PKRs) 1 bpe @ SW_64K_S3_X
    {   3,   21,  277,  358,    0, } , // 64 pipes (32 PKRs) 2 bpe @ SW_64K_S3_X
    {   3,   22,  278,  359,    0, } , // 64 pipes (32 PKRs) 4 bpe @ SW_64K_S3_X
    {   3,   23,  279,  360,    0, } , // 64 pipes (32 PKRs) 8 bpe @ SW_64K_S3_X
    {   3,   24,  280,  361,    0, } , // 64 pipes (32 PKRs) 16 bpe @ SW_64K_S3_X
};

const ADDR_SW_PATINFO GFX11_SW_64K_S3_T_PATINFO[] =
{
    {   1,   20,  246,  347,    0, } , // 1 pipes (1 PKRs) 1 bpe @ SW_64K_S3_T
    {   1,   21,  247,  348,    0, } , // 1 pipes (1 PKRs) 2 bpe @ SW_64K_S3_T
    {   1,   22,  248,  349,    0, } , // 1 pipes (1 PKRs) 4 bpe @ SW_64K_S3_T
    {   1,   23,  249,  350,    0, } , // 1 pipes (1 PKRs) 8 bpe @ SW_64K_S3_T
    {   1,   24,  250,  351,    0, } , // 1 pipes (1 PKRs) 16 bpe @ SW_64K_S3_T
    {   3,   20,  251,  347,    0, } , // 2 pipes (1-2 PKRs) 1 bpe @ SW_64K_S3_T
    {   3,   21,  252,  348,    0, } , // 2 pipes (1-2 PKRs) 2 bpe @ SW_64K_S3_T
    {   3,   22,  253,  349,    0, } , // 2 pipes (1-2 PKRs) 4 bpe @ SW_64K_S3_T
    {   3,   23,  254,  350,    0, } , // 2 pipes (1-2 PKRs) 8 bpe @ SW_64K_S3_T
    {   3,   24,  255,  351,    0, } , // 2 pipes (1-2 PKRs) 16 bpe @ SW_64K_S3_T
    {   3,   20,  256,  347,    0, } , // 4 pipes (1-2 PKRs) 1 bpe @ SW_64K_S3_T
    {   3,   21,  257,  348,    0, } , // 4 pipes (1-2 PKRs) 2 bpe @ SW_64K_S3_T
    {   3,   22,  258,  349,    0, } , // 4 pipes (1-2 PKRs) 4 bpe @ SW_64K_S3_T
    {   3,   23,  259,  350,    0, } , // 4 pipes (1-2 PKRs) 8 bpe @ SW_64K_S3_T
    {   3,   24,  260,  351,    0, } , // 4 pipes (1-2 PKRs) 16 bpe @ SW_64K_S3_T
    {   3,   20,  281,  347,    0, } , // 8 pipes (2 PKRs) 1 bpe @ SW_64K_S3_T
    {   3,   21,  282,  348,    0, } , // 8 pipes (2 PKRs) 2 bpe @ SW_64K_S3_T
    {   3,   22,  283,  349,    0, } , // 8 pipes (2 PKRs) 4 bpe @ SW_64K_S3_T
    {   3,   23,  284,  350,    0, } , // 8 pipes (2 PKRs) 8 bpe @ SW_64K_S3_T
    {   3,   24,  285,  351,    0, } , // 8 pipes (2 PKRs) 16 bpe @ SW_64K_S3_T
    {   3,   20,  256,  347,    0, } , // 4 pipes (4 PKRs) 1 bpe @ SW_64K_S3_T
    {   3,   21,  257,  348,    0, } , // 4 pipes (4 PKRs) 2 bpe @ SW_64K_S3_T
    {   3,   22,  258,  349,    0, } , // 4 pipes (4 PKRs) 4 bpe @ SW_64K_S3_T
    {   3,   23,  259,  350,    0, } , // 4 pipes (4 PKRs) 8 bpe @ SW_64K_S3_T
    {   3,   24,  260,  351,    0, } , // 4 pipes (4 PKRs) 16 bpe @ SW_64K_S3_T
    {   3,   20,  281,  347,    0, } , // 8 pipes (4 PKRs) 1 bpe @ SW_64K_S3_T
    {   3,   21,  282,  348,    0, } , // 8 pipes (4 PKRs) 2 bpe @ SW_64K_S3_T
    {   3,   22,  283,  349,    0, } , // 8 pipes (4 PKRs) 4 bpe @ SW_64K_S3_T
    {   3,   23,  284,  350,    0, } , // 8 pipes (4 PKRs) 8 bpe @ SW_64K_S3_T
    {   3,   24,  285,  351,    0, } , // 8 pipes (4 PKRs) 16 bpe @ SW_64K_S3_T
    {   3,   20,  286,  347,    0, } , // 16 pipes (4 PKRs) 1 bpe @ SW_64K_S3_T
    {   3,   21,  287,  348,    0, } , // 16 pipes (4 PKRs) 2 bpe @ SW_64K_S3_T
    {   3,   22,  288,  349,    0, } , // 16 pipes (4 PKRs) 4 bpe @ SW_64K_S3_T
    {   3,   23,  289,  350,    0, } , // 16 pipes (4 PKRs) 8 bpe @ SW_64K_S3_T
    {   3,   24,  290,  351,    0, } , // 16 pipes (4 PKRs) 16 bpe @ SW_64K_S3_T
    {   3,   20,  281,  347,    0, } , // 8 pipes (8 PKRs) 1 bpe @ SW_64K_S3_T
    {   3,   21,  282,  348,    0, } , // 8 pipes (8 PKRs) 2 bpe @ SW_64K_S3_T
    {   3,   22,  283,  349,    0, } , // 8 pipes (8 PKRs) 4 bpe @ SW_64K_S3_T
    {   3,   23,  284,  350,    0, } , // 8 pipes (8 PKRs) 8 bpe @ SW_64K_S3_T
    {   3,   24,  285,  351,    0, } , // 8 pipes (8 PKRs) 16 bpe @ SW_64K_S3_T
    {   3,   20,  286,  347,    0, } , // 16 pipes (8 PKRs) 1 bpe @ SW_64K_S3_T
    {   3,   21,  287,  348,    0, } , // 16 pipes (8 PKRs) 2 bpe @ SW_64K_S3_T
    {   3,   22,  288,  349,    0, } , // 16 pipes (8 PKRs) 4 bpe @ SW_64K_S3_T
    {   3,   23,  289,  350,    0, } , // 16 pipes (8 PKRs) 8 bpe @ SW_64K_S3_T
    {   3,   24,  290,  351,    0, } , // 16 pipes (8 PKRs) 16 bpe @ SW_64K_S3_T
    {   3,   20,  291,  352,    0, } , // 32 pipes (8 PKRs) 1 bpe @ SW_64K_S3_T
    {   3,   21,  292,  353,    0, } , // 32 pipes (8 PKRs) 2 bpe @ SW_64K_S3_T
    {   3,   22,  293,  354,    0, } , // 32 pipes (8 PKRs) 4 bpe @ SW_64K_S3_T
    {   3,   23,  294,  355,    0, } , // 32 pipes (8 PKRs) 8 bpe @ SW_64K_S3_T
    {   3,   24,  295,  356,    0, } , // 32 pipes (8 PKRs) 16 bpe @ SW_64K_S3_T
    {   3,   20,  286,  347,    0, } , // 16 pipes (16 PKRs) 1 bpe @ SW_64K_S3_T
    {   3,   21,  287,  348,    0, } , // 16 pipes (16 PKRs) 2 bpe @ SW_64K_S3_T
    {   3,   22,  288,  349,    0, } , // 16 pipes (16 PKRs) 4 bpe @ SW_64K_S3_T
    {   3,   23,  289,  350,    0, } , // 16 pipes (16 PKRs) 8 bpe @ SW_64K_S3_T
    {   3,   24,  290,  351,    0, } , // 16 pipes (16 PKRs) 16 bpe @ SW_64K_S3_T
    {   3,   20,  291,  352,    0, } , // 32 pipes (16 PKRs) 1 bpe @ SW_64K_S3_T
    {   3,   21,  292,  353,    0, } , // 32 pipes (16 PKRs) 2 bpe @ SW_64K_S3_T
    {   3,   22,  293,  354,    0, } , // 32 pipes (16 PKRs) 4 bpe @ SW_64K_S3_T
    {   3,   23,  294,  355,    0, } , // 32 pipes (16 PKRs) 8 bpe @ SW_64K_S3_T
    {   3,   24,  295,  356,    0, } , // 32 pipes (16 PKRs) 16 bpe @ SW_64K_S3_T
    {   3,   20,  246,  362,    0, } , // 64 pipes (16 PKRs) 1 bpe @ SW_64K_S3_T
    {   3,   21,  247,  363,    0, } , // 64 pipes (16 PKRs) 2 bpe @ SW_64K_S3_T
    {   3,   22,  248,  364,    0, } , // 64 pipes (16 PKRs) 4 bpe @ SW_64K_S3_T
    {   3,   23,  249,  365,    0, } , // 64 pipes (16 PKRs) 8 bpe @ SW_64K_S3_T
    {   3,   24,  250,  366,    0, } , // 64 pipes (16 PKRs) 16 bpe @ SW_64K_S3_T
    {   3,   20,  291,  352,    0, } , // 32 pipes (32 PKRs) 1 bpe @ SW_64K_S3_T
    {   3,   21,  292,  353,    0, } , // 32 pipes (32 PKRs) 2 bpe @ SW_64K_S3_T
    {   3,   22,  293,  354,    0, } , // 32 pipes (32 PKRs) 4 bpe @ SW_64K_S3_T
    {   3,   23,  294,  355,    0, } , // 32 pipes (32 PKRs) 8 bpe @ SW_64K_S3_T
    {   3,   24,  295,  356,    0, } , // 32 pipes (32 PKRs) 16 bpe @ SW_64K_S3_T
    {   3,   20,  246,  362,    0, } , // 64 pipes (32 PKRs) 1 bpe @ SW_64K_S3_T
    {   3,   21,  247,  363,    0, } , // 64 pipes (32 PKRs) 2 bpe @ SW_64K_S3_T
    {   3,   22,  248,  364,    0, } , // 64 pipes (32 PKRs) 4 bpe @ SW_64K_S3_T
    {   3,   23,  249,  365,    0, } , // 64 pipes (32 PKRs) 8 bpe @ SW_64K_S3_T
    {   3,   24,  250,  366,    0, } , // 64 pipes (32 PKRs) 16 bpe @ SW_64K_S3_T
};

const ADDR_SW_PATINFO GFX11_SW_256K_S3_X_PATINFO[] =
{
    {   1,   20,  246,  347,  121, } , // 1 pipes (1 PKRs) 1 bpe @ SW_256K_S3_X
    {   1,   21,  247,  348,  121, } , // 1 pipes (1 PKRs) 2 bpe @ SW_256K_S3_X
    {   1,   22,  248,  349,  122, } , // 1 pipes (1 PKRs) 4 bpe @ SW_256K_S3_X
    {   1,   23,  249,  350,  123, } , // 1 pipes (1 PKRs) 8 bpe @ SW_256K_S3_X
    {   1,   24,  250,  351,  123, } , // 1 pipes (1 PKRs) 16 bpe @ SW_256K_S3_X
    {   3,   20,  251,  347,  121, } , // 2 pipes (1-2 PKRs) 1 bpe @ SW_256K_S3_X
    {   3,   21,  252,  348,  121, } , // 2 pipes (1-2 PKRs) 2 bpe @ SW_256K_S3_X
    {   3,   22,  253,  349,  122, } , // 2 pipes (1-2 PKRs) 4 bpe @ SW_256K_S3_X
    {   3,   23,  254,  350,  123, } , // 2 pipes (1-2 PKRs) 8 bpe @ SW_256K_S3_X
    {   3,   24,  255,  351,  123, } , // 2 pipes (1-2 PKRs) 16 bpe @ SW_256K_S3_X
    {   3,   20,  256,  347,  121, } , // 4 pipes (1-2 PKRs) 1 bpe @ SW_256K_S3_X
    {   3,   21,  257,  348,  121, } , // 4 pipes (1-2 PKRs) 2 bpe @ SW_256K_S3_X
    {   3,   22,  258,  349,  122, } , // 4 pipes (1-2 PKRs) 4 bpe @ SW_256K_S3_X
    {   3,   23,  259,  350,  123, } , // 4 pipes (1-2 PKRs) 8 bpe @ SW_256K_S3_X
    {   3,   24,  260,  351,  123, } , // 4 pipes (1-2 PKRs) 16 bpe @ SW_256K_S3_X
    {   3,   20,  261,  347,  121, } , // 8 pipes (2 PKRs) 1 bpe @ SW_256K_S3_X
    {   3,   21,  262,  348,  121, } , // 8 pipes (2 PKRs) 2 bpe @ SW_256K_S3_X
    {   3,   22,  263,  349,  122, } , // 8 pipes (2 PKRs) 4 bpe @ SW_256K_S3_X
    {   3,   23,  264,  350,  123, } , // 8 pipes (2 PKRs) 8 bpe @ SW_256K_S3_X
    {   3,   24,  265,  351,  123, } , // 8 pipes (2 PKRs) 16 bpe @ SW_256K_S3_X
    {   3,   20,  256,  347,  121, } , // 4 pipes (4 PKRs) 1 bpe @ SW_256K_S3_X
    {   3,   21,  257,  348,  121, } , // 4 pipes (4 PKRs) 2 bpe @ SW_256K_S3_X
    {   3,   22,  258,  349,  122, } , // 4 pipes (4 PKRs) 4 bpe @ SW_256K_S3_X
    {   3,   23,  259,  350,  123, } , // 4 pipes (4 PKRs) 8 bpe @ SW_256K_S3_X
    {   3,   24,  260,  351,  123, } , // 4 pipes (4 PKRs) 16 bpe @ SW_256K_S3_X
    {   3,   20,  261,  347,  121, } , // 8 pipes (4 PKRs) 1 bpe @ SW_256K_S3_X
    {   3,   21,  262,  348,  121, } , // 8 pipes (4 PKRs) 2 bpe @ SW_256K_S3_X
    {   3,   22,  263,  349,  122, } , // 8 pipes (4 PKRs) 4 bpe @ SW_256K_S3_X
    {   3,   23,  264,  350,  123, } , // 8 pipes (4 PKRs) 8 bpe @ SW_256K_S3_X
    {   3,   24,  265,  351,  123, } , // 8 pipes (4 PKRs) 16 bpe @ SW_256K_S3_X
    {   3,   20,  266,  347,  121, } , // 16 pipes (4 PKRs) 1 bpe @ SW_256K_S3_X
    {   3,   21,  267,  348,  121, } , // 16 pipes (4 PKRs) 2 bpe @ SW_256K_S3_X
    {   3,   22,  268,  349,  122, } , // 16 pipes (4 PKRs) 4 bpe @ SW_256K_S3_X
    {   3,   23,  269,  350,  123, } , // 16 pipes (4 PKRs) 8 bpe @ SW_256K_S3_X
    {   3,   24,  270,  351,  123, } , // 16 pipes (4 PKRs) 16 bpe @ SW_256K_S3_X
    {   3,   20,  261,  347,  121, } , // 8 pipes (8 PKRs) 1 bpe @ SW_256K_S3_X
    {   3,   21,  262,  348,  121, } , // 8 pipes (8 PKRs) 2 bpe @ SW_256K_S3_X
    {   3,   22,  263,  349,  122, } , // 8 pipes (8 PKRs) 4 bpe @ SW_256K_S3_X
    {   3,   23,  264,  350,  123, } , // 8 pipes (8 PKRs) 8 bpe @ SW_256K_S3_X
    {   3,   24,  265,  351,  123, } , // 8 pipes (8 PKRs) 16 bpe @ SW_256K_S3_X
    {   3,   20,  266,  347,  121, } , // 16 pipes (8 PKRs) 1 bpe @ SW_256K_S3_X
    {   3,   21,  267,  348,  121, } , // 16 pipes (8 PKRs) 2 bpe @ SW_256K_S3_X
    {   3,   22,  268,  349,  122, } , // 16 pipes (8 PKRs) 4 bpe @ SW_256K_S3_X
    {   3,   23,  269,  350,  123, } , // 16 pipes (8 PKRs) 8 bpe @ SW_256K_S3_X
    {   3,   24,  270,  351,  123, } , // 16 pipes (8 PKRs) 16 bpe @ SW_256K_S3_X
    {   3,   20,  271,  352,  121, } , // 32 pipes (8 PKRs) 1 bpe @ SW_256K_S3_X
    {   3,   21,  272,  353,  121, } , // 32 pipes (8 PKRs) 2 bpe @ SW_256K_S3_X
    {   3,   22,  273,  354,  122, } , // 32 pipes (8 PKRs) 4 bpe @ SW_256K_S3_X
    {   3,   23,  274,  355,  123, } , // 32 pipes (8 PKRs) 8 bpe @ SW_256K_S3_X
    {   3,   24,  275,  356,  123, } , // 32 pipes (8 PKRs) 16 bpe @ SW_256K_S3_X
    {   3,   20,  266,  347,  121, } , // 16 pipes (16 PKRs) 1 bpe @ SW_256K_S3_X
    {   3,   21,  267,  348,  121, } , // 16 pipes (16 PKRs) 2 bpe @ SW_256K_S3_X
    {   3,   22,  268,  349,  122, } , // 16 pipes (16 PKRs) 4 bpe @ SW_256K_S3_X
    {   3,   23,  269,  350,  123, } , // 16 pipes (16 PKRs) 8 bpe @ SW_256K_S3_X
    {   3,   24,  270,  351,  123, } , // 16 pipes (16 PKRs) 16 bpe @ SW_256K_S3_X
    {   3,   20,  271,  352,  121, } , // 32 pipes (16 PKRs) 1 bpe @ SW_256K_S3_X
    {   3,   21,  272,  353,  121, } , // 32 pipes (16 PKRs) 2 bpe @ SW_256K_S3_X
    {   3,   22,  273,  354,  122, } , // 32 pipes (16 PKRs) 4 bpe @ SW_256K_S3_X
    {   3,   23,  274,  355,  123, } , // 32 pipes (16 PKRs) 8 bpe @ SW_256K_S3_X
    {   3,   24,  275,  356,  123, } , // 32 pipes (16 PKRs) 16 bpe @ SW_256K_S3_X
    {   3,   20,  276,  357,  121, } , // 64 pipes (16 PKRs) 1 bpe @ SW_256K_S3_X
    {   3,   21,  277,  358,  121, } , // 64 pipes (16 PKRs) 2 bpe @ SW_256K_S3_X
    {   3,   22,  278,  359,  122, } , // 64 pipes (16 PKRs) 4 bpe @ SW_256K_S3_X
    {   3,   23,  279,  360,  123, } , // 64 pipes (16 PKRs) 8 bpe @ SW_256K_S3_X
    {   3,   24,  280,  361,  123, } , // 64 pipes (16 PKRs) 16 bpe @ SW_256K_S3_X
    {   3,   20,  271,  352,  121, } , // 32 pipes (32 PKRs) 1 bpe @ SW_256K_S3_X
    {   3,   21,  272,  353,  121, } , // 32 pipes (32 PKRs) 2 bpe @ SW_256K_S3_X
    {   3,   22,  273,  354,  122, } , // 32 pipes (32 PKRs) 4 bpe @ SW_256K_S3_X
    {   3,   23,  274,  355,  123, } , // 32 pipes (32 PKRs) 8 bpe @ SW_256K_S3_X
    {   3,   24,  275,  356,  123, } , // 32 pipes (32 PKRs) 16 bpe @ SW_256K_S3_X
    {   3,   20,  276,  357,  121, } , // 64 pipes (32 PKRs) 1 bpe @ SW_256K_S3_X
    {   3,   21,  277,  358,  121, } , // 64 pipes (32 PKRs) 2 bpe @ SW_256K_S3_X
    {   3,   22,  278,  359,  122, } , // 64 pipes (32 PKRs) 4 bpe @ SW_256K_S3_X
    {   3,   23,  279,  360,  123, } , // 64 pipes (32 PKRs) 8 bpe @ SW_256K_S3_X
    {   3,   24,  280,  361,  123, } , // 64 pipes (32 PKRs) 16 bpe @ SW_256K_S3_X
};

const ADDR_SW_PATINFO GFX11_SW_64K_D3_X_PATINFO[] =
{
    {   1,   20,  246,  347,    0, } , // 1 pipes (1 PKRs) 1 bpe @ SW_64K_D3_X
    {   1,   21,  247,  348,    0, } , // 1 pipes (1 PKRs) 2 bpe @ SW_64K_D3_X
    {   1,   22,  248,  349,    0, } , // 1 pipes (1 PKRs) 4 bpe @ SW_64K_D3_X
    {   1,   23,  249,  350,    0, } , // 1 pipes (1 PKRs) 8 bpe @ SW_64K_D3_X
    {   1,   24,  250,  351,    0, } , // 1 pipes (1 PKRs) 16 bpe @ SW_64K_D3_X
    {   2,   20,  296,  367,    0, } , // 2 pipes (1-2 PKRs) 1 bpe @ SW_64K_D3_X
    {   2,   21,  296,  368,    0, } , // 2 pipes (1-2 PKRs) 2 bpe @ SW_64K_D3_X
    {   2,   22,  297,  369,    0, } , // 2 pipes (1-2 PKRs) 4 bpe @ SW_64K_D3_X
    {   2,   23,  298,  351,    0, } , // 2 pipes (1-2 PKRs) 8 bpe @ SW_64K_D3_X
    {   3,   24,  299,  351,    0, } , // 2 pipes (1-2 PKRs) 16 bpe @ SW_64K_D3_X
    {   3,   20,  300,  370,    0, } , // 4 pipes (1-2 PKRs) 1 bpe @ SW_64K_D3_X
    {   3,   21,  300,  371,    0, } , // 4 pipes (1-2 PKRs) 2 bpe @ SW_64K_D3_X
    {   3,   22,  301,  372,    0, } , // 4 pipes (1-2 PKRs) 4 bpe @ SW_64K_D3_X
    {   4,   23,  302,  373,    0, } , // 4 pipes (1-2 PKRs) 8 bpe @ SW_64K_D3_X
    {   4,   24,  303,  373,    0, } , // 4 pipes (1-2 PKRs) 16 bpe @ SW_64K_D3_X
    {   3,   20,  304,  370,    0, } , // 8 pipes (2 PKRs) 1 bpe @ SW_64K_D3_X
    {   3,   21,  304,  371,    0, } , // 8 pipes (2 PKRs) 2 bpe @ SW_64K_D3_X
    {   3,   22,  305,  372,    0, } , // 8 pipes (2 PKRs) 4 bpe @ SW_64K_D3_X
    {   4,   23,  306,  373,    0, } , // 8 pipes (2 PKRs) 8 bpe @ SW_64K_D3_X
    {   4,   24,  307,  373,    0, } , // 8 pipes (2 PKRs) 16 bpe @ SW_64K_D3_X
    {   3,   20,  308,  374,    0, } , // 4 pipes (4 PKRs) 1 bpe @ SW_64K_D3_X
    {   3,   21,  309,  375,    0, } , // 4 pipes (4 PKRs) 2 bpe @ SW_64K_D3_X
    {   3,   22,  310,  376,    0, } , // 4 pipes (4 PKRs) 4 bpe @ SW_64K_D3_X
    {   4,   23,  311,  377,    0, } , // 4 pipes (4 PKRs) 8 bpe @ SW_64K_D3_X
    {   4,   24,  312,  378,    0, } , // 4 pipes (4 PKRs) 16 bpe @ SW_64K_D3_X
    {   3,   20,  313,  379,    0, } , // 8 pipes (4 PKRs) 1 bpe @ SW_64K_D3_X
    {   3,   21,  314,  371,    0, } , // 8 pipes (4 PKRs) 2 bpe @ SW_64K_D3_X
    {   3,   22,  315,  372,    0, } , // 8 pipes (4 PKRs) 4 bpe @ SW_64K_D3_X
    {   4,   23,  316,  373,    0, } , // 8 pipes (4 PKRs) 8 bpe @ SW_64K_D3_X
    {   4,   24,  317,  373,    0, } , // 8 pipes (4 PKRs) 16 bpe @ SW_64K_D3_X
    {   3,   20,  318,  380,    0, } , // 16 pipes (4 PKRs) 1 bpe @ SW_64K_D3_X
    {   3,   21,  319,  371,    0, } , // 16 pipes (4 PKRs) 2 bpe @ SW_64K_D3_X
    {   3,   22,  320,  372,    0, } , // 16 pipes (4 PKRs) 4 bpe @ SW_64K_D3_X
    {   4,   23,  321,  373,    0, } , // 16 pipes (4 PKRs) 8 bpe @ SW_64K_D3_X
    {   4,   24,  322,  373,    0, } , // 16 pipes (4 PKRs) 16 bpe @ SW_64K_D3_X
    {   3,   20,  323,  381,    0, } , // 8 pipes (8 PKRs) 1 bpe @ SW_64K_D3_X
    {   3,   21,  323,  382,    0, } , // 8 pipes (8 PKRs) 2 bpe @ SW_64K_D3_X
    {   3,   22,  323,  383,    0, } , // 8 pipes (8 PKRs) 4 bpe @ SW_64K_D3_X
    {   4,   23,  324,  384,    0, } , // 8 pipes (8 PKRs) 8 bpe @ SW_64K_D3_X
    {   4,   24,  325,  384,    0, } , // 8 pipes (8 PKRs) 16 bpe @ SW_64K_D3_X
    {   3,   20,  326,  379,    0, } , // 16 pipes (8 PKRs) 1 bpe @ SW_64K_D3_X
    {   3,   21,  327,  371,    0, } , // 16 pipes (8 PKRs) 2 bpe @ SW_64K_D3_X
    {   3,   22,  328,  372,    0, } , // 16 pipes (8 PKRs) 4 bpe @ SW_64K_D3_X
    {   4,   23,  329,  373,    0, } , // 16 pipes (8 PKRs) 8 bpe @ SW_64K_D3_X
    {   4,   24,  330,  373,    0, } , // 16 pipes (8 PKRs) 16 bpe @ SW_64K_D3_X
    {   3,   20,  326,  385,    0, } , // 32 pipes (8 PKRs) 1 bpe @ SW_64K_D3_X
    {   3,   21,  331,  386,    0, } , // 32 pipes (8 PKRs) 2 bpe @ SW_64K_D3_X
    {   3,   22,  331,  387,    0, } , // 32 pipes (8 PKRs) 4 bpe @ SW_64K_D3_X
    {   4,   23,  332,  388,    0, } , // 32 pipes (8 PKRs) 8 bpe @ SW_64K_D3_X
    {   4,   24,  333,  388,    0, } , // 32 pipes (8 PKRs) 16 bpe @ SW_64K_D3_X
    {   3,   20,  334,  389,    0, } , // 16 pipes (16 PKRs) 1 bpe @ SW_64K_D3_X
    {   3,   21,  335,  390,    0, } , // 16 pipes (16 PKRs) 2 bpe @ SW_64K_D3_X
    {   3,   22,  336,  391,    0, } , // 16 pipes (16 PKRs) 4 bpe @ SW_64K_D3_X
    {   4,   23,  337,  392,    0, } , // 16 pipes (16 PKRs) 8 bpe @ SW_64K_D3_X
    {   4,   24,  338,  392,    0, } , // 16 pipes (16 PKRs) 16 bpe @ SW_64K_D3_X
    {   3,   20,  334,  393,    0, } , // 32 pipes (16 PKRs) 1 bpe @ SW_64K_D3_X
    {   3,   21,  335,  394,    0, } , // 32 pipes (16 PKRs) 2 bpe @ SW_64K_D3_X
    {   3,   22,  336,  395,    0, } , // 32 pipes (16 PKRs) 4 bpe @ SW_64K_D3_X
    {   4,   23,  337,  396,    0, } , // 32 pipes (16 PKRs) 8 bpe @ SW_64K_D3_X
    {   4,   24,  338,  396,    0, } , // 32 pipes (16 PKRs) 16 bpe @ SW_64K_D3_X
    {   3,   20,  334,  397,    0, } , // 64 pipes (16 PKRs) 1 bpe @ SW_64K_D3_X
    {   3,   21,  339,  398,    0, } , // 64 pipes (16 PKRs) 2 bpe @ SW_64K_D3_X
    {   3,   22,  339,  399,    0, } , // 64 pipes (16 PKRs) 4 bpe @ SW_64K_D3_X
    {   4,   23,  340,  400,    0, } , // 64 pipes (16 PKRs) 8 bpe @ SW_64K_D3_X
    {   4,   24,  341,  400,    0, } , // 64 pipes (16 PKRs) 16 bpe @ SW_64K_D3_X
    {   3,   20,  342,  401,    0, } , // 32 pipes (32 PKRs) 1 bpe @ SW_64K_D3_X
    {   3,   21,  343,  402,    0, } , // 32 pipes (32 PKRs) 2 bpe @ SW_64K_D3_X
    {   3,   22,  344,  403,    0, } , // 32 pipes (32 PKRs) 4 bpe @ SW_64K_D3_X
    {   4,   23,  345,  404,    0, } , // 32 pipes (32 PKRs) 8 bpe @ SW_64K_D3_X
    {   4,   24,  346,  404,    0, } , // 32 pipes (32 PKRs) 16 bpe @ SW_64K_D3_X
    {   3,   20,  342,  405,    0, } , // 64 pipes (32 PKRs) 1 bpe @ SW_64K_D3_X
    {   3,   21,  343,  406,    0, } , // 64 pipes (32 PKRs) 2 bpe @ SW_64K_D3_X
    {   3,   22,  344,  407,    0, } , // 64 pipes (32 PKRs) 4 bpe @ SW_64K_D3_X
    {   4,   23,  345,  408,    0, } , // 64 pipes (32 PKRs) 8 bpe @ SW_64K_D3_X
    {   4,   24,  346,  408,    0, } , // 64 pipes (32 PKRs) 16 bpe @ SW_64K_D3_X
};


const UINT_64 GFX11_SW_PATTERN_NIBBLE01[][8] =
{
    {X0,            X1,            Y0,            X2,            Y1,            Y2,            X3,            Y3,            }, // 0
    {0,             X0,            Y0,            X1,            Y1,            X2,            Y2,            X3,            }, // 1
    {0,             0,             X0,            Y0,            X1,            Y1,            X2,            Y2,            }, // 2
    {0,             0,             0,             X0,            Y0,            X1,            X2,            Y1,            }, // 3
    {0,             0,             0,             0,             X0,            Y0,            X1,            Y1,            }, // 4
    {S0,            X0,            Y0,            X1,            Y1,            X2,            Y2,            X3,            }, // 5
    {0,             S0,            X0,            Y0,            X1,            Y1,            X2,            Y2,            }, // 6
    {0,             0,             S0,            X0,            Y0,            X1,            Y1,            X2,            }, // 7
    {0,             0,             0,             S0,            X0,            Y0,            X1,            Y1,            }, // 8
    {0,             0,             0,             0,             S0,            X0,            Y0,            X1,            }, // 9
    {S0,            S1,            X0,            Y0,            X1,            Y1,            X2,            Y2,            }, // 10
    {0,             S0,            S1,            X0,            Y0,            X1,            Y1,            X2,            }, // 11
    {0,             0,             S0,            S1,            X0,            Y0,            X1,            Y1,            }, // 12
    {0,             0,             0,             S0,            S1,            X0,            Y0,            X1,            }, // 13
    {0,             0,             0,             0,             S0,            S1,            X0,            Y0,            }, // 14
    {S0,            S1,            S2,            X0,            Y0,            X1,            Y1,            X2,            }, // 15
    {0,             S0,            S1,            S2,            X0,            Y0,            X1,            Y1,            }, // 16
    {0,             0,             S0,            S1,            S2,            X0,            Y0,            X1,            }, // 17
    {0,             0,             0,             S0,            S1,            S2,            X0,            Y0,            }, // 18
    {0,             0,             0,             0,             S0,            S1,            S2,            X0,            }, // 19
    {X0,            X1,            Z0,            Y0,            Y1,            Z1,            X2,            Z2,            }, // 20
    {0,             X0,            Z0,            Y0,            X1,            Z1,            Y1,            Z2,            }, // 21
    {0,             0,             X0,            Y0,            X1,            Z0,            Y1,            Z1,            }, // 22
    {0,             0,             0,             X0,            Y0,            Z0,            X1,            Z1,            }, // 23
    {0,             0,             0,             0,             X0,            Z0,            Y0,            Z1,            }, // 24
};

const UINT_64 GFX11_SW_PATTERN_NIBBLE2[][4] =
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
    {Y4^X5,         Z0^X4^Y5,      Y5,            X5,            }, // 11
    {Y3^X5,         Z0^X4^Y4,      Y4,            X5,            }, // 12
    {Y3^X4,         Z0^X3^Y4,      Y4,            X4,            }, // 13
    {Y2^X4,         Z0^X3^Y3,      Y3,            X4,            }, // 14
    {Y2^X3,         Z0^X2^Y3,      Y3,            X3,            }, // 15
    {Y4^X6,         X4^Y6,         Z0^X5^Y5,      X5,            }, // 16
    {Y3^X6,         X4^Y5,         Z0^Y4^X5,      X5,            }, // 17
    {Y3^X5,         X3^Y5,         Z0^X4^Y4,      X4,            }, // 18
    {Y2^X5,         X3^Y4,         Z0^Y3^X4,      X4,            }, // 19
    {Y2^X4,         X2^Y4,         Z0^X3^Y3,      X3,            }, // 20
    {Z1^Y4^X5,      Z0^X4^Y5,      Y5,            X5,            }, // 21
    {Z1^Y3^X5,      Z0^X4^Y4,      Y4,            X5,            }, // 22
    {Z1^Y3^X4,      Z0^X3^Y4,      Y4,            X4,            }, // 23
    {Z1^Y2^X4,      Z0^X3^Y3,      Y3,            X4,            }, // 24
    {Z1^Y2^X3,      Z0^X2^Y3,      Y3,            X3,            }, // 25
    {Y4^X6,         Z1^X4^Y6,      Z0^X5^Y5,      X5,            }, // 26
    {Y3^X6,         Z1^X4^Y5,      Z0^Y4^X5,      X5,            }, // 27
    {Y3^X5,         Z1^X3^Y5,      Z0^X4^Y4,      X4,            }, // 28
    {Y2^X5,         Z1^X3^Y4,      Z0^Y3^X4,      X4,            }, // 29
    {Y2^X4,         Z1^X2^Y4,      Z0^X3^Y3,      X3,            }, // 30
    {Y4^X7,         X4^Y7,         Z1^Y5^X6,      Z0^X5^Y6,      }, // 31
    {Y3^X7,         X4^Y6,         Z1^Y4^X6,      Z0^X5^Y5,      }, // 32
    {Y3^X6,         X3^Y6,         Z1^Y4^X5,      Z0^X4^Y5,      }, // 33
    {Y2^X6,         X3^Y5,         Z1^Y3^X5,      Z0^X4^Y4,      }, // 34
    {Y2^X5,         X2^Y5,         Z1^Y3^X4,      Z0^X3^Y4,      }, // 35
    {Z2^Y4^X6,      Z1^X4^Y6,      Z0^X5^Y5,      X5,            }, // 36
    {Z2^Y3^X6,      Z1^X4^Y5,      Z0^Y4^X5,      X5,            }, // 37
    {Z2^Y3^X5,      Z1^X3^Y5,      Z0^X4^Y4,      X4,            }, // 38
    {Y2^Z2^X5,      Z1^X3^Y4,      Z0^Y3^X4,      X4,            }, // 39
    {Y2^Z2^X4,      Z1^X2^Y4,      Z0^X3^Y3,      X3,            }, // 40
    {Y4^X7,         Z2^X4^Y7,      Z1^Y5^X6,      Z0^X5^Y6,      }, // 41
    {Y3^X7,         Z2^X4^Y6,      Z1^Y4^X6,      Z0^X5^Y5,      }, // 42
    {Y3^X6,         Z2^X3^Y6,      Z1^Y4^X5,      Z0^X4^Y5,      }, // 43
    {Y2^X6,         Z2^X3^Y5,      Z1^Y3^X5,      Z0^X4^Y4,      }, // 44
    {Y2^X5,         X2^Z2^Y5,      Z1^Y3^X4,      Z0^X3^Y4,      }, // 45
    {Y4^X7,         X4^Y7,         Z2^Y5^X6,      Z1^X5^Y6,      }, // 46
    {Y3^X7,         X4^Y6,         Z2^Y4^X6,      Z1^X5^Y5,      }, // 47
    {Y3^X6,         X3^Y6,         Z2^Y4^X5,      Z1^X4^Y5,      }, // 48
    {Y2^X6,         X3^Y5,         Z2^Y3^X5,      Z1^X4^Y4,      }, // 49
    {Y2^X5,         X2^Y5,         Z2^Y3^X4,      Z1^X3^Y4,      }, // 50
    {Z3^Y4^X7,      Z2^X4^Y7,      Z1^Y5^X6,      Z0^X5^Y6,      }, // 51
    {Y3^Z3^X7,      Z2^X4^Y6,      Z1^Y4^X6,      Z0^X5^Y5,      }, // 52
    {Y3^Z3^X6,      Z2^X3^Y6,      Z1^Y4^X5,      Z0^X4^Y5,      }, // 53
    {Y2^Z3^X6,      Z2^X3^Y5,      Z1^Y3^X5,      Z0^X4^Y4,      }, // 54
    {Y2^Z3^X5,      X2^Z2^Y5,      Z1^Y3^X4,      Z0^X3^Y4,      }, // 55
    {Y4^X7,         Z3^X4^Y7,      Z2^Y5^X6,      Z1^X5^Y6,      }, // 56
    {Y3^X7,         Z3^X4^Y6,      Z2^Y4^X6,      Z1^X5^Y5,      }, // 57
    {Y3^X6,         X3^Z3^Y6,      Z2^Y4^X5,      Z1^X4^Y5,      }, // 58
    {Y2^X6,         X3^Z3^Y5,      Z2^Y3^X5,      Z1^X4^Y4,      }, // 59
    {Y2^X5,         X2^Z3^Y5,      Z2^Y3^X4,      Z1^X3^Y4,      }, // 60
    {Y4^X7,         X4^Y7,         Z3^Y5^X6,      Z2^X5^Y6,      }, // 61
    {Y3^X7,         X4^Y6,         Z3^Y4^X6,      Z2^X5^Y5,      }, // 62
    {Y3^X6,         X3^Y6,         Z3^Y4^X5,      Z2^X4^Y5,      }, // 63
    {Y2^X6,         X3^Y5,         Y3^Z3^X5,      Z2^X4^Y4,      }, // 64
    {Y2^X5,         X2^Y5,         Y3^Z3^X4,      Z2^X3^Y4,      }, // 65
    {Y4^X8,         X4^Y8,         Z2^Y5^X7,      Z1^X5^Y7,      }, // 66
    {Y3^X8,         X4^Y7,         Z2^Y4^X7,      Z1^X5^Y6,      }, // 67
    {Y3^X7,         X3^Y7,         Z2^Y4^X6,      Z1^X4^Y6,      }, // 68
    {Y2^X7,         X3^Y6,         Z2^Y3^X6,      Z1^X4^Y5,      }, // 69
    {Y2^X6,         X2^Y6,         Z2^Y3^X5,      Z1^X3^Y5,      }, // 70
    {Y4^X8,         Z3^X4^Y8,      Z2^Y5^X7,      Z1^X5^Y7,      }, // 71
    {Y3^X8,         Z3^X4^Y7,      Z2^Y4^X7,      Z1^X5^Y6,      }, // 72
    {Y3^X7,         X3^Z3^Y7,      Z2^Y4^X6,      Z1^X4^Y6,      }, // 73
    {Y2^X7,         X3^Z3^Y6,      Z2^Y3^X6,      Z1^X4^Y5,      }, // 74
    {Y2^X6,         X2^Z3^Y6,      Z2^Y3^X5,      Z1^X3^Y5,      }, // 75
    {Y4^X9,         X4^Y9,         Z3^Y5^X8,      Z2^X5^Y8,      }, // 76
    {Y3^X9,         X4^Y8,         Z3^Y4^X8,      Z2^X5^Y7,      }, // 77
    {Y3^X8,         X3^Y8,         Z3^Y4^X7,      Z2^X4^Y7,      }, // 78
    {Y2^X8,         X3^Y7,         Y3^Z3^X7,      Z2^X4^Y6,      }, // 79
    {Y2^X7,         X2^Y7,         Y3^Z3^X6,      Z2^X3^Y6,      }, // 80
    {Y4^Z4^X8,      Z3^X4^Y8,      Z2^Y5^X7,      Z1^X5^Y7,      }, // 81
    {Y3^Z4^X8,      Z3^X4^Y7,      Z2^Y4^X7,      Z1^X5^Y6,      }, // 82
    {Y3^Z4^X7,      X3^Z3^Y7,      Z2^Y4^X6,      Z1^X4^Y6,      }, // 83
    {Y2^Z4^X7,      X3^Z3^Y6,      Z2^Y3^X6,      Z1^X4^Y5,      }, // 84
    {Y2^Z4^X6,      X2^Z3^Y6,      Z2^Y3^X5,      Z1^X3^Y5,      }, // 85
    {Y4^X9,         X4^Z4^Y9,      Z3^Y5^X8,      Z2^X5^Y8,      }, // 86
    {Y3^X9,         X4^Z4^Y8,      Z3^Y4^X8,      Z2^X5^Y7,      }, // 87
    {Y3^X8,         X3^Z4^Y8,      Z3^Y4^X7,      Z2^X4^Y7,      }, // 88
    {Y2^X8,         X3^Z4^Y7,      Y3^Z3^X7,      Z2^X4^Y6,      }, // 89
    {Y2^X7,         X2^Z4^Y7,      Y3^Z3^X6,      Z2^X3^Y6,      }, // 90
    {X4^Y4,         X4,            Y5,            X5,            }, // 91
    {Y3^X4,         X4,            Y4,            X5,            }, // 92
    {X3^Y3,         X3,            Y4,            X4,            }, // 93
    {Y2^X3,         X3,            Y3,            X4,            }, // 94
    {X2^Y2,         X2,            Y3,            X3,            }, // 95
    {Y4^X5,         X4^Y5,         Y5,            X5,            }, // 96
    {Y3^X5,         X4^Y4,         Y4,            X5,            }, // 97
    {Y3^X4,         X3^Y4,         Y4,            X4,            }, // 98
    {Y2^X4,         X3^Y3,         Y3,            X4,            }, // 99
    {Y2^X3,         X2^Y3,         Y3,            X3,            }, // 100
    {Y4^X6,         X4^Y6,         X5^Y5,         X5,            }, // 101
    {Y3^X6,         X4^Y5,         Y4^X5,         X5,            }, // 102
    {Y3^X5,         X3^Y5,         X4^Y4,         X4,            }, // 103
    {Y2^X5,         X3^Y4,         Y3^X4,         X4,            }, // 104
    {Y2^X4,         X2^Y4,         X3^Y3,         X3,            }, // 105
    {Y4^X7,         X4^Y7,         Y5^X6,         X5^Y6,         }, // 106
    {Y3^X7,         X4^Y6,         Y4^X6,         X5^Y5,         }, // 107
    {Y3^X6,         X3^Y6,         Y4^X5,         X4^Y5,         }, // 108
    {Y2^X6,         X3^Y5,         Y3^X5,         X4^Y4,         }, // 109
    {Y2^X5,         X2^Y5,         Y3^X4,         X3^Y4,         }, // 110
    {Y4,            X4,            Y5^X7,         X5^Y7,         }, // 111
    {Y3,            X4,            Y4^X7,         X5^Y6,         }, // 112
    {Y3,            X3,            Y4^X6,         X4^Y6,         }, // 113
    {Y2,            X3,            Y3^X6,         X4^Y5,         }, // 114
    {Y2,            X2,            Y3^X5,         X3^Y5,         }, // 115
    {X4,            Y4,            X5^Y8,         Y5^X8,         }, // 116
    {Y3,            X4,            Y4^X8,         X5^Y7,         }, // 117
    {X3,            Y3,            X4^Y7,         Y4^X7,         }, // 118
    {Y2,            X3,            Y3^X7,         X4^Y7,         }, // 119
    {X2,            Y2,            X3^Y7,         Y3^X6,         }, // 120
    {Z0^X4^Y4,      Y4,            X5,            Y5^X9,         }, // 121
    {Z0^X4^Y4,      Y3,            Y4,            X5^Y8,         }, // 122
    {Z0^X4^Y4,      X3,            Y3,            Y4^X8,         }, // 123
    {Z0^X4^Y4,      Y2,            X3,            Y3^X8,         }, // 124
    {Z0^X4^Y4,      X2,            Y2,            Y3^X7,         }, // 125
    {Y4^X5^Y5,      Z0^X4^Y4,      X5,            Y5,            }, // 126
    {Y4^X5^Y5,      Z0^X4^Y4,      Y3,            X5,            }, // 127
    {Y4^X5^Y5,      Z0^X4^Y4,      X3,            Y3,            }, // 128
    {Y4^X5^Y5,      Z0^X4^Y4,      Y2,            X3,            }, // 129
    {Y4^X5^Y5,      Z0^X4^Y4,      X2,            Y2,            }, // 130
    {Y4^X5^Y5,      Z0^X4^Y4,      X5^Y5,         Y5,            }, // 131
    {Y4^X5^Y5,      Z0^X4^Y4,      X5^Y5,         Y3,            }, // 132
    {Y4^X5^Y5,      Z0^X4^Y4,      X5^Y5,         X3,            }, // 133
    {Y4^X5^Y5,      Z0^X4^Y4,      X5^Y5,         Y2,            }, // 134
    {Y4^X5^Y5,      Z0^X4^Y4,      X5^Y5,         X2,            }, // 135
    {Y4^X6^Y6,      Z1^X4^Y4,      X5,            X6,            }, // 136
    {Y4^X6^Y6,      Z1^X4^Y4,      Y3,            X5,            }, // 137
    {Y4^X6^Y6,      Z1^X4^Y4,      X3,            Y3,            }, // 138
    {Y4^X6^Y6,      Z1^X4^Y4,      Y2,            X3,            }, // 139
    {Y4^X6^Y6,      Z1^X4^Y4,      X2,            Y2,            }, // 140
    {Y4^X6^Y6,      Z1^X4^Y4,      Z0^X5^Y5,      X5,            }, // 141
    {Y4^X6^Y6,      Z1^X4^Y4,      Z0^X5^Y5,      Y3,            }, // 142
    {Y4^X6^Y6,      Z1^X4^Y4,      Z0^X5^Y5,      X3,            }, // 143
    {Y4^X6^Y6,      Z1^X4^Y4,      Z0^X5^Y5,      Y2,            }, // 144
    {Y4^X6^Y6,      Z1^X4^Y4,      Z0^X5^Y5,      X2,            }, // 145
    {Y4^X6^Y6,      Z1^X4^Y4,      Z0^X5^Y5,      X5^Y6,         }, // 146
    {Y4^X7^Y7,      Z1^X4^Y4,      Z0^Y5^X6,      X6,            }, // 147
    {Y4^X7^Y7,      Z1^X4^Y4,      Z0^Y5^X6,      Y3,            }, // 148
    {Y4^X7^Y7,      Z1^X4^Y4,      Z0^Y5^X6,      X3,            }, // 149
    {Y4^X7^Y7,      Z2^X4^Y4,      Z1^Y5^X6,      Y2,            }, // 150
    {Y4^X7^Y7,      Z2^X4^Y4,      Z1^Y5^X6,      X2,            }, // 151
    {Y4^X7^Y7,      Z1^X4^Y4,      Z0^Y5^X6,      X5^Y6,         }, // 152
    {Y4^X7^Y7,      Z2^X4^Y4,      Z1^Y5^X6,      Z0^X5^Y6,      }, // 153
    {Y4^X8^Y8,      Z1^X4^Y4,      Z0^Y5^X7,      X5^Y7,         }, // 154
    {Y4^X8^Y8,      Z3^X4^Y4,      Z2^Y5^X7,      Z1^X5^Y7,      }, // 155
    {Y4^X8^Y8,      Z2^X4^Y4,      Z1^Y5^X7,      Z0^X5^Y7,      }, // 156
    {Y4^X9^Y9,      Z1^X4^Y4,      Z0^Y5^X8,      X5^Y8,         }, // 157
    {Y4^X9^Y9,      Z3^X4^Y4,      Z2^Y5^X8,      Z1^X5^Y8,      }, // 158
    {Y4^X9^Y9,      Z2^X4^Y4,      Z1^Y5^X8,      Z0^X5^Y8,      }, // 159
    {Y3,            X4,            Y4^X8,         Y5^X7,         }, // 160
    {X3,            Y3,            Y4^X7,         X4^Y7,         }, // 161
    {X2,            Y2,            Y3^X6,         X3^Y7,         }, // 162
    {Z0^X4^Y4,      Y3,            Y4,            Y5^X8,         }, // 163
    {Z0^X4^Y4,      X2,            X3,            Y3^X8,         }, // 164
    {Y4^X5^Y5,      Z0^X4^Y4,      X2,            X3,            }, // 165
    {Y4^X6^Y6,      Z1^X4^Y4,      Z0^X5^Y5,      X2^X5^Y6,      }, // 166
    {Y4^X6^Y6,      Z1^X4^Y4,      Z0^X5^Y5,      Y1^X5^Y6,      }, // 167
    {Y4^X7^Y7,      Z1^X4^Y4,      Z0^Y5^X6,      X2,            }, // 168
    {Y4^X7^Y7,      Z1^X4^Y4,      Z0^Y5^X6,      Y1^X5^Y6,      }, // 169
    {Y4^X8^Y8,      Z1^X4^Y4,      Z0^Y5^X7,      Y1^X5^Y7,      }, // 170
    {Y4^X9^Y9,      Z1^X4^Y4,      Z0^Y5^X8,      Y1^X5^Y8,      }, // 171
    {Z0^X4^Y4,      X3,            Y3,            X5^Y7,         }, // 172
    {Y4^X5^Y5,      Z0^X4^Y4,      Y1^X5^Y5,      X2,            }, // 173
    {Y4^X6^Y6,      Z1^X4^Y4,      X2,            X3,            }, // 174
    {Y4^X6^Y6,      Z0^X4^Y4,      X2,            X3,            }, // 175
    {Y4^X6^Y6,      Z0^X4^Y4,      Y1^X5^Y5,      X2,            }, // 176
    {Y4^X6^Y6,      Z0^X4^Y4,      Y1^X5^Y5,      X1^X5^Y6,      }, // 177
    {Y4^X7^Y7,      Z1^X4^Y4,      Y1^Y5^X6,      X3,            }, // 178
    {Y4^X7^Y7,      Z0^X4^Y4,      Y1^Y5^X6,      X3,            }, // 179
    {Y4^X7^Y7,      Z1^X4^Y4,      Y1^Y5^X6,      Z0^X5^Y6,      }, // 180
    {Y4^X7^Y7,      Z0^X4^Y4,      Y1^Y5^X6,      X1^X5^Y6,      }, // 181
    {Y4^X8^Y8,      Z1^X4^Y4,      Y1^Y5^X7,      Z0^X5^Y7,      }, // 182
    {Y4^X8^Y8,      Z0^X4^Y4,      Y1^Y5^X7,      X1^X5^Y7,      }, // 183
    {Y4^X9^Y9,      Z1^X4^Y4,      Y1^Y5^X8,      Z0^X5^Y8,      }, // 184
    {Y4^X9^Y9,      Z0^X4^Y4,      Y1^Y5^X8,      X1^X5^Y8,      }, // 185
    {X3,            Y3,            Y4^X6,         X4^Y7,         }, // 186
    {Y2,            X3,            Y3^X6,         X4^Y7,         }, // 187
    {Z0^X4^Y4,      X3,            Y3,            Y4^X6,         }, // 188
    {Z0^X4^Y4,      X2,            X3,            Y3^X7,         }, // 189
    {Z0^X4^Y4,      X2,            Y2,            X3^Y7,         }, // 190
    {Y4^X5^Y5,      Y0^X4^Y4,      X2,            X3,            }, // 191
    {Y4^X5^Y5,      Z0^X4^Y4,      Y2^X5^Y5,      X2,            }, // 192
    {Y4^X5^Y5,      Y0^X4^Y4,      X1^X5^Y5,      X2,            }, // 193
    {Y4^X6^Y6,      Z0^X4^Y4,      X3,            Y3,            }, // 194
    {Y4^X6^Y6,      Y0^X4^Y4,      X3,            Y3,            }, // 195
    {Y4^X6^Y6,      Z0^X4^Y4,      Y1^X5^Y5,      X3,            }, // 196
    {Y4^X6^Y6,      Y0^X4^Y4,      Y1^X5^Y5,      X3,            }, // 197
    {Y4^X6^Y6,      Z1^X4^Y4,      Z0^X5^Y5,      Y2^X5^Y6,      }, // 198
    {Y4^X6^Y6,      Z0^X4^Y4,      Y1^X5^Y5,      X2^X5^Y6,      }, // 199
    {Y4^X6^Y6,      Y0^X4^Y4,      Y1^X5^Y5,      Y2^X5^Y6,      }, // 200
    {Y4^X7^Y7,      Y0^X4^Y4,      Y1^Y5^X6,      X3,            }, // 201
    {Y4^X7^Y7,      Z1^X4^Y4,      Z0^Y5^X6,      Y2^X5^Y6,      }, // 202
    {Y4^X7^Y7,      Y0^X4^Y4,      Y1^Y5^X6,      X1^X5^Y6,      }, // 203
    {Y4^X8^Y8,      Z1^X4^Y4,      Z0^Y5^X7,      Y2^X5^Y7,      }, // 204
    {Y4^X8^Y8,      Y0^X4^Y4,      Y1^Y5^X7,      X1^X5^Y7,      }, // 205
    {Y4^X8^Y8,      Z1^X4^Y4,      Z0^Y5^X7,      X2^X5^Y7,      }, // 206
    {Y4^X9^Y9,      Z1^X4^Y4,      Z0^Y5^X8,      X2^X5^Y8,      }, // 207
    {Y4^X9^Y9,      Y0^X4^Y4,      Y1^Y5^X8,      X1^X5^Y8,      }, // 208
    {Y4^X9^Y9,      X4^Y4^Z4,      Z3^Y5^X8,      Z2^X5^Y8,      }, // 209
    {Y2,            X3,            Y3^X7,         Y4^X6,         }, // 210
    {Y1,            X2,            Y2^X7,         Y3^X6,         }, // 211
    {Z0^X4^Y4,      Y2,            Y3,            Y4^X7,         }, // 212
    {Z0^X4^Y4,      Y1,            Y2,            Y3^X6,         }, // 213
    {Y4^X5^Y5,      Z0^X4^Y4,      Y1,            Y2,            }, // 214
    {Y4^X5^Y5,      Z0^X4^Y4,      X5^Y5,         Y1,            }, // 215
    {Y4^X6^Y6,      Z1^X4^Y4,      Y1,            X2,            }, // 216
    {Y4^X6^Y6,      Z1^X4^Y4,      Z0^X5^Y5,      Y1,            }, // 217
    {Y4^X7^Y7,      Z1^X4^Y4,      Z0^Y5^X6,      Y2,            }, // 218
    {Y4^X7^Y7,      Z2^X4^Y4,      Z1^Y5^X6,      Y1,            }, // 219
    {Y1,            X2,            Y2^X6,         X3^Y7,         }, // 220
    {X1,            Y1,            X2^Y7,         Y2^X6,         }, // 221
    {Z0^X4^Y4,      Y1,            X2,            Y2^X7,         }, // 222
    {Z0^X4^Y4,      X1,            Y1,            Y2^X6,         }, // 223
    {Y4^X5^Y5,      Z0^X4^Y4,      Y1,            X2,            }, // 224
    {Y4^X5^Y5,      Z0^X4^Y4,      X1,            Y1,            }, // 225
    {Y4^X5^Y5,      Z0^X4^Y4,      X5^Y5,         X1,            }, // 226
    {Y4^X6^Y6,      Z1^X4^Y4,      X1,            Y1,            }, // 227
    {Y4^X6^Y6,      Z1^X4^Y4,      Z0^X5^Y5,      X1,            }, // 228
    {Y4^X7^Y7,      Z2^X4^Y4,      Z1^Y5^X6,      X1,            }, // 229
    {Y0,            X1,            Y1^X7,         Y2^X6,         }, // 230
    {Z0^X4^Y4,      Y0,            Y1,            Y2^X6,         }, // 231
    {Y4^X5^Y5,      Z0^X4^Y4,      Y0,            Y1,            }, // 232
    {Y4^X5^Y5,      Z0^X4^Y4,      X5^Y5,         Y0,            }, // 233
    {Y4^X6^Y6,      Z1^X4^Y4,      Y0,            X1,            }, // 234
    {Y4^X6^Y6,      Z1^X4^Y4,      Z0^X5^Y5,      Y0,            }, // 235
    {Y4^X6^Y6,      Z1^X4^Y4,      Z0^X5^Y5,      Y3^X5,         }, // 236
    {Y4^X6^Y6,      Z1^X4^Y4,      Z0^X5^Y5,      X3^Y5,         }, // 237
    {Y4^X7^Y7,      Z1^X4^Y4,      Z0^Y5^X6,      Y1,            }, // 238
    {Y4^X7^Y7,      Z1^X4^Y4,      Z0^Y5^X6,      X1,            }, // 239
    {Y4^X7^Y7,      Z1^X4^Y4,      Z0^Y5^X6,      Z2^X5^Y6,      }, // 240
    {Y4^X7^Y7,      Z1^X4^Y4,      Z0^Y5^X6,      Y0^X5^Y6,      }, // 241
    {Y4^X8^Y8,      Z1^X4^Y4,      Z0^Y5^X7,      Z2^X5^Y7,      }, // 242
    {Y4^X8^Y8,      Z1^X4^Y4,      Z0^Y5^X7,      Y0^X5^Y7,      }, // 243
    {Y4^X9^Y9,      Z1^X4^Y4,      Z0^Y5^X8,      Z2^X5^Y8,      }, // 244
    {Y4^X9^Y9,      Z1^X4^Y4,      Z0^Y5^X8,      Y0^X5^Y8,      }, // 245
    {Y2,            X3,            Z3,            Y3,            }, // 246
    {Y2,            X2,            Z3,            Y3,            }, // 247
    {Y2,            X2,            Z2,            Y3,            }, // 248
    {Y1,            X2,            Z2,            Y2,            }, // 249
    {Y1,            X1,            Z2,            Y2,            }, // 250
    {Y2^X3^Z3,      X3,            Z3,            Y3,            }, // 251
    {X2^Y2^Z3,      X2,            Z3,            Y3,            }, // 252
    {X2^Y2^Z2,      X2,            Z2,            Y3,            }, // 253
    {Y1^X2^Z2,      X2,            Z2,            Y2,            }, // 254
    {X1^Y1^Z2,      X1,            Z2,            Y2,            }, // 255
    {Y2^X4^Z4,      X3^Y3^Z3,      Z3,            Y3,            }, // 256
    {Y2^X3^Z4,      X2^Y3^Z3,      Z3,            Y3,            }, // 257
    {Y2^X3^Z3,      X2^Z2^Y3,      Z2,            Y3,            }, // 258
    {Y1^X3^Z3,      X2^Y2^Z2,      Z2,            Y2,            }, // 259
    {Y1^X2^Z3,      X1^Y2^Z2,      Z2,            Y2,            }, // 260
    {Y2^X5^Z5,      X3^Y4^Z4,      Y3^Z3^X4,      Y3,            }, // 261
    {Y2^X4^Z5,      X2^Y4^Z4,      X3^Y3^Z3,      Y3,            }, // 262
    {Y2^X4^Z4,      X2^Z3^Y4,      Z2^X3^Y3,      Y3,            }, // 263
    {Y1^X4^Z4,      X2^Y3^Z3,      Y2^Z2^X3,      Y2,            }, // 264
    {Y1^X3^Z4,      X1^Y3^Z3,      X2^Y2^Z2,      Y2,            }, // 265
    {Y2^X6^Z6,      X3^Y5^Z5,      Z3^Y4^X5,      Y3^X4^Z4,      }, // 266
    {Y2^X5^Z6,      X2^Y5^Z5,      Z3^X4^Y4,      X3^Y3^Z4,      }, // 267
    {Y2^X5^Z5,      X2^Z4^Y5,      Z2^X4^Y4,      X3^Y3^Z3,      }, // 268
    {Y1^X5^Z5,      X2^Y4^Z4,      Z2^Y3^X4,      Y2^X3^Z3,      }, // 269
    {Y1^X4^Z5,      X1^Y4^Z4,      Z2^X3^Y3,      X2^Y2^Z3,      }, // 270
    {Y2^X7^Z7,      X3^Y6^Z6,      Z3^Y5^X6,      Y3^X5^Z5,      }, // 271
    {Y2^X6^Z7,      X2^Y6^Z6,      Z3^X5^Y5,      Y3^X4^Z5,      }, // 272
    {Y2^X6^Z6,      X2^Z5^Y6,      Z2^X5^Y5,      Y3^X4^Z4,      }, // 273
    {Y1^X6^Z6,      X2^Y5^Z5,      Z2^Y4^X5,      Y2^X4^Z4,      }, // 274
    {Y1^X5^Z6,      X1^Y5^Z5,      Z2^X4^Y4,      Y2^X3^Z4,      }, // 275
    {Y2^X8^Z8,      X3^Y7^Z7,      Z3^Y6^X7,      Y3^X6^Z6,      }, // 276
    {Y2^X7^Z8,      X2^Y7^Z7,      Z3^X6^Y6,      Y3^X5^Z6,      }, // 277
    {Y2^X7^Z7,      X2^Z6^Y7,      Z2^X6^Y6,      Y3^X5^Z5,      }, // 278
    {Y1^X7^Z7,      X2^Y6^Z6,      Z2^Y5^X6,      Y2^X5^Z5,      }, // 279
    {Y1^X6^Z7,      X1^Y6^Z6,      Z2^X5^Y5,      Y2^X4^Z5,      }, // 280
    {Y2^X5,         X3^Y4^Z4,      Y3^Z3^X4,      Y3,            }, // 281
    {Y2^X4,         X2^Y4^Z4,      X3^Y3^Z3,      Y3,            }, // 282
    {Y2^X4,         X2^Z3^Y4,      Z2^X3^Y3,      Y3,            }, // 283
    {Y1^X4,         X2^Y3^Z3,      Y2^Z2^X3,      Y2,            }, // 284
    {Y1^X3,         X1^Y3^Z3,      X2^Y2^Z2,      Y2,            }, // 285
    {Y2,            X3,            Z3^Y4^X5,      Y3^X4^Z4,      }, // 286
    {Y2,            X2,            Z3^X4^Y4,      X3^Y3^Z4,      }, // 287
    {Y2,            X2,            Z2^X4^Y4,      X3^Y3^Z3,      }, // 288
    {Y1,            X2,            Z2^Y3^X4,      Y2^X3^Z3,      }, // 289
    {Y1,            X1,            Z2^X3^Y3,      X2^Y2^Z3,      }, // 290
    {Y2,            X3,            Z3,            Y3^X5,         }, // 291
    {Y2,            X2,            Z3,            Y3^X4,         }, // 292
    {Y2,            X2,            Z2,            Y3^X4,         }, // 293
    {Y1,            X2,            Z2,            Y2^X4,         }, // 294
    {Y1,            X1,            Z2,            Y2^X3,         }, // 295
    {X4^Y4,         Y2,            Z3,            Y3,            }, // 296
    {X4^Y4,         Y2,            Z2,            Y3,            }, // 297
    {X4^Y4,         Y1,            Z2,            Y2,            }, // 298
    {Y1^X4^Y4,      X1,            Z2,            Y2,            }, // 299
    {Y4^X5^Y5,      X4^Y4,         Y2,            Z3,            }, // 300
    {Y4^X5^Y5,      X4^Y4,         Y2,            Z2,            }, // 301
    {Z3^Y4^X5^Y5,   X4^Y4,         Y1,            Z2,            }, // 302
    {Z3^Y4^X5^Y5,   Y1^X4^Y4,      X1,            Z2,            }, // 303
    {Y4^X5^Y5,      X4^Y4,         Z3^X5,         Y2,            }, // 304
    {Y4^X5^Y5,      X4^Y4,         Z2^X5,         Y2,            }, // 305
    {Z3^Y4^X5^Y5,   X4^Y4,         Z2^X5,         Y1,            }, // 306
    {Z3^Y4^X5^Y5,   Y1^X4^Y4,      Z2^X5,         X1,            }, // 307
    {Y4^X6^Y6,      X4^Y4,         Y2,            Y3,            }, // 308
    {Y4^X6^Y6,      X4^Y4,         Z3,            Y3,            }, // 309
    {Y4^X6^Y6,      X4^Y4,         Z2,            Y3,            }, // 310
    {Z3^Y4^X6^Y6,   X4^Y4,         Z2,            Y2,            }, // 311
    {Z3^Y4^X6^Y6,   Y1^X4^Y4,      Z2,            Y2,            }, // 312
    {Y4^X6^Y6,      X4^Y4,         X5^Y5,         Y2,            }, // 313
    {Y4^X6^Y6,      X4^Y4,         Y2^X5^Y5,      Z3,            }, // 314
    {Y4^X6^Y6,      X4^Y4,         Y2^X5^Y5,      Z2,            }, // 315
    {Z3^Y4^X6^Y6,   X4^Y4,         Y1^X5^Y5,      Z2,            }, // 316
    {Z3^Y4^X6^Y6,   Y1^X4^Y4,      X1^X5^Y5,      Z2,            }, // 317
    {Y4^X6^Y6,      X4^Y4,         X5^Y5,         Z3^X6,         }, // 318
    {Y4^X6^Y6,      X4^Y4,         Y2^X5^Y5,      Z3^X6,         }, // 319
    {Y4^X6^Y6,      X4^Y4,         Y2^X5^Y5,      Z2^X6,         }, // 320
    {Z3^Y4^X6^Y6,   X4^Y4,         Y1^X5^Y5,      Z2^X6,         }, // 321
    {Z3^Y4^X6^Y6,   Y1^X4^Y4,      X1^X5^Y5,      Z2^X6,         }, // 322
    {Y4^X7^Y7,      X4^Y4,         Y2^Y5^X6,      Y3,            }, // 323
    {Z3^Y4^X7^Y7,   X4^Y4,         Y1^Y5^X6,      Y2,            }, // 324
    {Z3^Y4^X7^Y7,   Y1^X4^Y4,      X1^Y5^X6,      Y2,            }, // 325
    {Y4^X7^Y7,      X4^Y4,         Y2^Y5^X6,      X5^Y6,         }, // 326
    {Y4^X7^Y7,      X4^Y4,         Y2^Y5^X6,      Z3^X5^Y6,      }, // 327
    {Y4^X7^Y7,      X4^Y4,         Y2^Y5^X6,      Z2^X5^Y6,      }, // 328
    {Z3^Y4^X7^Y7,   X4^Y4,         Y1^Y5^X6,      Z2^X5^Y6,      }, // 329
    {Z3^Y4^X7^Y7,   Y1^X4^Y4,      X1^Y5^X6,      Z2^X5^Y6,      }, // 330
    {Y4^X7^Y7,      X4^Y4,         Y2^Y5^X6,      Y3^X5^Y6,      }, // 331
    {Z3^Y4^X7^Y7,   X4^Y4,         Y1^Y5^X6,      Y2^X5^Y6,      }, // 332
    {Z3^Y4^X7^Y7,   Y1^X4^Y4,      X1^Y5^X6,      Y2^X5^Y6,      }, // 333
    {Y4^X8^Y8,      X4^Y4,         Y2^Y5^X7,      X5^Y7,         }, // 334
    {Y4^X8^Y8,      X4^Y4,         Y2^Y5^X7,      Z3^X5^Y7,      }, // 335
    {Y4^X8^Y8,      X4^Y4,         Y2^Y5^X7,      Z2^X5^Y7,      }, // 336
    {Z3^Y4^X8^Y8,   X4^Y4,         Y1^Y5^X7,      Z2^X5^Y7,      }, // 337
    {Z3^Y4^X8^Y8,   Y1^X4^Y4,      X1^Y5^X7,      Z2^X5^Y7,      }, // 338
    {Y4^X8^Y8,      X4^Y4,         Y2^Y5^X7,      Y3^X5^Y7,      }, // 339
    {Z3^Y4^X8^Y8,   X4^Y4,         Y1^Y5^X7,      Y2^X5^Y7,      }, // 340
    {Z3^Y4^X8^Y8,   Y1^X4^Y4,      X1^Y5^X7,      Y2^X5^Y7,      }, // 341
    {Y4^X9^Y9,      X4^Y4,         Y2^Y5^X8,      X5^Y8,         }, // 342
    {Y4^X9^Y9,      X4^Y4,         Y2^Y5^X8,      Z3^X5^Y8,      }, // 343
    {Y4^X9^Y9,      X4^Y4,         Y2^Y5^X8,      Z2^X5^Y8,      }, // 344
    {Z3^Y4^X9^Y9,   X4^Y4,         Y1^Y5^X8,      Z2^X5^Y8,      }, // 345
    {Z3^Y4^X9^Y9,   Y1^X4^Y4,      X1^Y5^X8,      Z2^X5^Y8,      }, // 346
};

const UINT_64 GFX11_SW_PATTERN_NIBBLE3[][4] =
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
    {X6^Y7,         Y6^X7,         X7,            Y7,            }, // 26
    {X5^Y6,         Y5^X6,         X6,            Y6,            }, // 27
    {Y4^X6,         X5^Y6,         Y5,            X6,            }, // 28
    {X4^Y6,         Y4^X5,         X5,            Y5,            }, // 29
    {X6^Y8,         Y6^X8,         X7^Y7,         Y7,            }, // 30
    {X6^Y7,         Y5^X8,         Y6^X7,         Y6,            }, // 31
    {X5^Y7,         Y5^X7,         X6^Y6,         Y6,            }, // 32
    {X5^Y7,         Y4^X7,         X6^Y6,         Y5,            }, // 33
    {X3^Y7,         Y4^X6,         X5^Y6,         Y5,            }, // 34
    {X6^Y9,         Y6^X9,         X7^Y8,         Y7^X8,         }, // 35
    {X6^Y8,         Y5^X9,         X7^Y7,         Y6^X8,         }, // 36
    {X5^Y8,         Y5^X8,         X6^Y7,         Y6^X7,         }, // 37
    {Y3^X8,         X5^Y7,         X6^Y6,         Y5^X7,         }, // 38
    {Y3^X7,         X3^Y7,         X5^Y6,         Y5^X6,         }, // 39
    {X6,            Y6^X9,         X7^Y8,         Y7^X8,         }, // 40
    {Y5,            X6^Y8,         X7^Y7,         Y6^X8,         }, // 41
    {Y3,            Y5^X8,         X6^Y7,         Y6^X7,         }, // 42
    {X3,            Y3^X8,         X6^Y6,         Y5^X7,         }, // 43
    {Y2,            Y3^X7,         X3^Y6,         Y5^X6,         }, // 44
    {Y6^X9,         X7^Y8,         Y7^X8,         Z0^X5^Y5,      }, // 45
    {X6^Y8,         Y6^X8,         X7^Y7,         Z0^X5^Y5,      }, // 46
    {X5^Y8,         X6^Y7,         Y6^X7,         Z0^X5^Y5,      }, // 47
    {Y3^X7,         X5^Y7,         X6^Y6,         Z0^X5^Y5,      }, // 48
    {X3^Y7,         Y3^X6,         X5^Y6,         Z0^X5^Y5,      }, // 49
    {X5,            X6^Y8,         Y6^X8,         X7^Y7,         }, // 50
    {Y3,            X5^Y8,         X6^Y7,         Y6^X7,         }, // 51
    {X3,            Y3^X7,         X5^Y7,         X6^Y6,         }, // 52
    {Y2,            X3^Y7,         Y3^X6,         X5^Y6,         }, // 53
    {X6,            Y6,            X7^Y8,         Y7^X8,         }, // 54
    {Y3,            X6,            Y6^X8,         X7^Y7,         }, // 55
    {X3,            Y3,            X6^Y7,         Y6^X7,         }, // 56
    {Y2,            X3,            Y3^X7,         X6^Y6,         }, // 57
    {X2,            Y2,            X3^Y6,         Y3^X6,         }, // 58
    {Y6,            X7^Y8,         Y7^X8,         X5^Y6,         }, // 59
    {X6,            X7^Y7,         Y6^X8,         X5^Y6,         }, // 60
    {Y3,            X6^Y7,         Y6^X7,         X5^Y6,         }, // 61
    {X3,            Y3^X7,         X6^Y6,         Z0^X5^Y6,      }, // 62
    {Y2,            Y3^X6,         X3^Y6,         Z0^X5^Y6,      }, // 63
    {Y3,            X6,            X7^Y7,         Y6^X8,         }, // 64
    {X2,            Y2,            Y3^X6,         X3^Y6,         }, // 65
    {X6^Y6,         Y6,            X7,            Y7^X8,         }, // 66
    {X6^Y6,         Y3,            Y6,            X7^Y7,         }, // 67
    {X6^Y6,         X3,            Y3,            Y6^X7,         }, // 68
    {X6^Y6,         Y2,            X3,            Y3^X7,         }, // 69
    {X3^Y6,         X2,            Y2,            Y3^X6,         }, // 70
    {X6,            X7,            Y7^X8,         X6^Y6,         }, // 71
    {Y3,            X6,            X7^Y7,         X6^Y6,         }, // 72
    {X3,            Y3,            X6^Y7,         X6^Y6,         }, // 73
    {Y2,            X3,            Y3^X7,         Z0^X6^Y6,      }, // 74
    {X2,            X3,            Y3^X6,         Y2^X6^Y6,      }, // 75
    {X6^Y6,         X6,            X7,            Y7^X8,         }, // 76
    {X6^Y6,         Y3,            X6,            X7^Y7,         }, // 77
    {X6^Y6,         X3,            Y3,            X6^Y7,         }, // 78
    {Z0^X6^Y6,      Y2,            X3,            Y3^X7,         }, // 79
    {Y2^X6^Y6,      X2,            X3,            Y3^X6,         }, // 80
    {X6^Y6,         X6^Y8,         X7,            Y7,            }, // 81
    {X6^Y6,         X6^Y8,         Y3,            X7,            }, // 82
    {X6^Y6,         X6^Y8,         X3,            Y3,            }, // 83
    {Z0^X6^Y6,      X3^Y8,         Y2,            Y3,            }, // 84
    {Y2^X6^Y6,      X3^Y8,         X2,            Y3,            }, // 85
    {Y6^X7,         X7,            Y7,            X6^Y7,         }, // 86
    {Y6^X7,         Y3,            X7,            X6^Y7,         }, // 87
    {Y6^X7,         X3,            Y3,            X6^Y7,         }, // 88
    {Y2^Y6^X7,      X3,            Y3,            Z0^X6^Y7,      }, // 89
    {Y2^Y6^X7,      X3,            Y3,            X2^X6^Y7,      }, // 90
    {Y6^X7,         X6^Y7,         X7,            Y7,            }, // 91
    {Y6^X7,         X6^Y7,         Y3,            X7,            }, // 92
    {Y6^X7,         X6^Y7,         X3,            Y3,            }, // 93
    {Y2^Y6^X7,      Z0^X6^Y7,      X3,            Y3,            }, // 94
    {Y2^Y6^X7,      X2^X6^Y7,      X3,            Y3,            }, // 95
    {X5^Y7,         X6^Y6,         X6,            Y7,            }, // 96
    {Y5^X6,         X5^Y6,         Y6,            Y2^X6,         }, // 97
    {Y4^X6,         X5^Y6,         Y5,            X2^X6,         }, // 98
    {Y4^X5,         X4^Y6,         Y5,            Y1^X5,         }, // 99
    {X5^Y8,         Y6^X7,         X6^Y7,         Y7,            }, // 100
    {Y5^X7,         X5^Y7,         X6^Y6,         Y2^X6,         }, // 101
    {Y4^X7,         X5^Y6,         Y5^X6,         Y2^X6,         }, // 102
    {Y4^X6,         X3^Y6,         X5^Y5,         Y1^X5,         }, // 103
    {Y5^X9,         Y6^X8,         X6^Y8,         X7^Y7,         }, // 104
    {Y5^X8,         X5^Y8,         Y6^X7,         Y2^X6^Y7,      }, // 105
    {Y3^X8,         X5^Y7,         Y5^X7,         Y2^X6^Y6,      }, // 106
    {Y3^X7,         X3^Y7,         Y5^X6,         Y1^X5^Y6,      }, // 107
    {X5,            Y6^X8,         X6^Y8,         X7^Y7,         }, // 108
    {Y3,            X5^Y8,         Y6^X7,         Y2^X6^Y7,      }, // 109
    {X3,            Y3^X7,         X5^Y7,         Y2^X6^Y6,      }, // 110
    {Y2,            Y3^X6,         X3^Y7,         Y1^X5^Y6,      }, // 111
    {Y6^X8,         X6^Y8,         X7^Y7,         Z0^X5^Y5,      }, // 112
    {X5^Y8,         Y6^X7,         Y2^X6^Y7,      Z0^X5^Y5,      }, // 113
    {Y3^X7,         X5^Y7,         X2^X6^Y6,      Z0^X5^Y5,      }, // 114
    {Y3^X6,         X3^Y7,         Y1^X5^Y6,      Z0^X5^Y5,      }, // 115
    {X3,            Y3^X7,         X5^Y7,         X2^X6^Y6,      }, // 116
    {Y3,            X5,            X6^Y8,         X7^Y7,         }, // 117
    {X3,            Y3,            X5^Y8,         X6^Y7,         }, // 118
    {X3,            Y3,            X5^Y8,         Y2^X6^Y7,      }, // 119
    {Y2,            X3,            Y3^X6,         X5^Y6,         }, // 120
    {X2,            Y2,            Y3^X5,         X3^Y6,         }, // 121
    {X6,            Y6^X8,         X7^Y7,         X5^Y6,         }, // 122
    {Y3,            Y6^X7,         Y2^X6^Y7,      X5^Y6,         }, // 123
    {X3,            Y3^X7,         Y2^X6^Y6,      Z0^X5^Y6,      }, // 124
    {X3,            Y3^X7,         Y2^X6^Y6,      Y1^X5^Y6,      }, // 125
    {X3,            Y3,            Y6^X7,         Y2^X6^Y7,      }, // 126
    {X2,            X3,            Y3^X7,         Y2^X6^Y6,      }, // 127
    {X6^Y6,         X3,            Y3,            Y2^X6^Y7,      }, // 128
    {X3,            Y3,            Y2^X6^Y7,      X6^Y6,         }, // 129
    {X3,            Y3,            X2^X6^Y7,      Y2^X6^Y6,      }, // 130
    {Y2^X6^Y6,      X3,            Y3,            X2^X6^Y7,      }, // 131
    {X6^Y6,         X6^Y8,         Y3,            Y7,            }, // 132
    {X6^Y6,         Y2^X6^Y8,      X3,            Y3,            }, // 133
    {Y2^X6^Y6,      X2^X6^Y8,      X3,            Y3,            }, // 134
    {Y6^X7,         Y3,            Y7,            X6^Y7,         }, // 135
    {Y6^X7,         X3,            Y3,            Y2^X6^Y7,      }, // 136
    {Y6^X7,         X6^Y7,         Y3,            Y7,            }, // 137
    {Y6^X7,         Y2^X6^Y7,      X3,            Y3,            }, // 138
    {X5^Y6,         Y5^X6,         X6,            Y2^Y6,         }, // 139
    {X5^Y6,         Y5^X6,         X2^X6,         Y2^Y6,         }, // 140
    {Y4^X6,         X5^Y6,         X2^X6,         Y1^Y5,         }, // 141
    {X4^Y6,         Y4^X5,         X1^X5,         Y1^Y5,         }, // 142
    {Y4^X8,         X6^Y6,         Y5^X7,         Y2^X7,         }, // 143
    {X5^Y6,         Y5^X7,         X2^X6^Y6,      Y2^X6,         }, // 144
    {X5^Y6,         Y4^X7,         X2^Y5^X6,      Y1^X6,         }, // 145
    {X3^Y6,         Y4^X6,         X1^X5^Y5,      Y1^X5,         }, // 146
    {X5^Y8,         X6^Y7,         Y5^X8,         Y2^Y6^X7,      }, // 147
    {X5^Y8,         Y5^X8,         X2^Y6^X7,      Y2^X6^Y7,      }, // 148
    {Y3^X8,         X5^Y7,         X2^Y5^X7,      Y1^X6^Y6,      }, // 149
    {Y3^X7,         X3^Y7,         X1^Y5^X6,      Y1^X5^Y6,      }, // 150
    {Y3,            X6^Y7,         Y5^X8,         Y2^Y6^X7,      }, // 151
    {Y3,            Y5^X8,         X2^Y6^X7,      Y2^X6^Y7,      }, // 152
    {X3,            Y3^X8,         X2^Y5^X7,      Y1^X6^Y6,      }, // 153
    {Y2,            Y3^X6,         X3^Y6,         X1^X5^Y5,      }, // 154
    {X5^Y8,         X6^Y7,         Y2^Y6^X7,      Z0^X5^Y5,      }, // 155
    {X5^Y8,         X2^X6^Y7,      Y2^Y6^X7,      Z0^X5^Y5,      }, // 156
    {Y3^X8,         Y2^Y5^X7,      Y1^X6^Y6,      Z0^X5^Y5,      }, // 157
    {Y3^X7,         Y2^X6^Y6,      X1^X5^Y7,      Y1^X5^Y5,      }, // 158
    {Y3,            X5^Y8,         X6^Y7,         Y2^Y6^X7,      }, // 159
    {Y3,            X5^Y8,         X2^X6^Y7,      Y2^Y6^X7,      }, // 160
    {X3,            Y3^X8,         Y2^Y5^X7,      Y1^X6^Y6,      }, // 161
    {X3,            Y3^X7,         Y2^X6^Y6,      X1^X5^Y7,      }, // 162
    {X3,            Y3,            X6^Y7,         Y2^Y6^X7,      }, // 163
    {X3,            Y3,            X2^X6^Y7,      Y2^Y6^X7,      }, // 164
    {X2,            X3,            Y3^X7,         Y2^Y5^X6,      }, // 165
    {X2,            X3,            Y3^X6,         Y2^X5^Y6,      }, // 166
    {Y3,            X6^Y7,         Y2^Y6^X7,      X5^Y6,         }, // 167
    {Y3,            X2^Y6^X7,      Y2^X6^Y7,      X5^Y6,         }, // 168
    {Y3,            X2^Y6^X7,      Y2^X6^Y7,      Z0^X5^Y6,      }, // 169
    {Y3,            X2^Y6^X7,      Y2^X6^Y7,      X1^X5^Y6,      }, // 170
    {X3,            Y3,            X2^Y6^X7,      Y2^X6^Y7,      }, // 171
    {X6^Y6,         X3,            Y3,            Y2^Y6^X7,      }, // 172
    {Y2^X6^Y6,      X3,            Y3,            X2^X6^Y6,      }, // 173
    {X3,            Y3,            Y2^Y6^X7,      X6^Y6,         }, // 174
    {Y2^Y6^X7,      X3,            Y3,            X6^Y7,         }, // 175
    {Y2^Y6^X7,      X6^Y7,         X3,            Y3,            }, // 176
    {X5^Y5,         Y1^X5^Y6,      X2^X6,         Y2^Y6,         }, // 177
    {Y4^X5,         X1^X5^Y6,      Y1^Y5,         X2^X6,         }, // 178
    {Y4^X5,         Y0^X4^Y6,      X1^X5,         Y1^Y5,         }, // 179
    {X5^Y5,         Y1^X5^Y7,      X2^X6^Y6,      Y2^Y6,         }, // 180
    {Y4^X6,         Y1^X5^Y6,      X1^X5^Y5,      Y2^X6,         }, // 181
    {Y3^X6,         Y0^X4^Y6,      X1^Y4^X5,      Y1^X5,         }, // 182
    {Y5^X8,         Y1^X5^Y8,      X2^X6^Y7,      Y2^Y6^X7,      }, // 183
    {Y3^X8,         Y1^X5^Y7,      X1^Y5^X7,      Y2^X6^Y6,      }, // 184
    {Y3^X7,         Y1^X4^Y7,      Y2^X5^Y6,      X1^Y5^X6,      }, // 185
    {Y3,            X5^Y8,         X2^Y6^X7,      Y2^X6^Y7,      }, // 186
    {Y3,            Y1^X5^Y8,      X2^X6^Y7,      Y2^Y6^X7,      }, // 187
    {X3,            Y3^X7,         Y1^X5^Y6,      X1^Y5^X6,      }, // 188
    {X3,            Y3^X6,         Y1^X4^Y6,      Y2^X5^Y5,      }, // 189
    {Y1^X5^Y8,      X2^X6^Y7,      Y2^Y6^X7,      Z0^X5^Y5,      }, // 190
    {X1^X5^Y8,      Y2^Y6^X7,      X2^X6^Y7,      Y1^X5^Y5,      }, // 191
    {X1^X5^Y8,      X2^X6^Y7,      Y2^Y6^X7,      Y1^X5^Y5,      }, // 192
    {Y3,            X1^X5^Y8,      Y2^Y6^X7,      X2^X6^Y7,      }, // 193
    {Y3,            X1^X5^Y8,      X2^X6^Y7,      Y2^Y6^X7,      }, // 194
    {X3,            Y3,            Y1^X5^Y7,      X2^X6^Y6,      }, // 195
    {X3,            Y3,            X1^X5^Y7,      Y2^X6^Y6,      }, // 196
    {X3,            Y3,            X1^X5^Y7,      X2^X6^Y6,      }, // 197
    {Y3,            X2^Y6^X7,      Y1^X6^Y7,      Y2^X5^Y6,      }, // 198
    {X3,            Y3,            X2^Y6^X7,      Y1^X6^Y7,      }, // 199
    {X2^X6^Y6,      X3,            Y3,            Y1^X6^Y6,      }, // 200
    {X2^X6^Y6,      X3,            Y3,            Y2^X6^Y6,      }, // 201
    {X3,            Y3,            Y1^X6^Y7,      X2^X6^Y6,      }, // 202
    {Y2^X6^Y6,      X3,            Y3,            Y1^X6^Y7,      }, // 203
    {Y2^X6^Y6,      Y1^X6^Y8,      X3,            Y3,            }, // 204
    {Y2^Y6^X7,      X3,            Y3,            Y1^X6^Y7,      }, // 205
    {X6,            Y6^X10,        X7^Y9,         Y7^X9,         }, // 206
    {Y5,            X6^Y9,         X7^Y8,         Y6^X9,         }, // 207
    {Y3,            Y5^X9,         X6^Y8,         Y6^X8,         }, // 208
    {X3,            Y3^X9,         X6^Y7,         Y5^X8,         }, // 209
    {Y2,            Y3^X8,         X3^Y7,         Y5^X7,         }, // 210
    {Y6^X10,        X7^Y9,         Y7^X9,         X8^Y8,         }, // 211
    {X5^Y9,         X6^Y8,         Y6^X8,         X7^Y7,         }, // 212
    {Y3^X8,         X5^Y8,         X6^Y7,         Y6^X7,         }, // 213
    {X3^Y8,         Y3^X7,         X5^Y7,         X6^Y6,         }, // 214
    {X5,            X6^Y9,         Y6^X9,         X7^Y8,         }, // 215
    {Y3,            X5^Y9,         X6^Y8,         Y6^X8,         }, // 216
    {X3,            Y3^X8,         X5^Y8,         X6^Y7,         }, // 217
    {Y2,            X3^Y8,         Y3^X7,         X5^Y7,         }, // 218
    {X6,            Y6,            X7^Y10,        Y7^X10,        }, // 219
    {Y3,            X6,            Y6^X10,        X7^Y9,         }, // 220
    {X3,            Y3,            X6^Y9,         Y6^X9,         }, // 221
    {Y2,            X3,            Y3^X9,         X6^Y8,         }, // 222
    {X2,            Y2,            X3^Y8,         Y3^X8,         }, // 223
    {Y6,            X7^Y10,        Y7^X10,        X8^Y9,         }, // 224
    {X6,            X7^Y9,         Y6^X10,        X8^Y8,         }, // 225
    {Y3,            X6^Y9,         Y6^X9,         X7^Y8,         }, // 226
    {X3,            Y3^X9,         X6^Y8,         X7^Y7,         }, // 227
    {Y2,            Y3^X8,         X3^Y8,         X6^Y7,         }, // 228
    {Y3,            X6,            X7^Y9,         Y6^X10,        }, // 229
    {X2,            Y2,            Y3^X8,         X3^Y8,         }, // 230
    {X6^Y6,         Y6,            X7,            Y7^X10,        }, // 231
    {X6^Y6,         Y3,            Y6,            X7^Y9,         }, // 232
    {X6^Y6,         X3,            Y3,            Y6^X9,         }, // 233
    {X6^Y6,         Y2,            X3,            Y3^X9,         }, // 234
    {X6^Y6,         X2,            Y2,            Y3^X8,         }, // 235
    {X6,            X7,            Y7^X10,        X8^Y9,         }, // 236
    {Y3,            X6,            X7^Y9,         Y7^X9,         }, // 237
    {X3,            Y3,            X6^Y9,         X7^Y8,         }, // 238
    {Y2,            X3,            Y3^X8,         X6^Y8,         }, // 239
    {X2,            Y2,            X3^Y8,         Y3^X7,         }, // 240
    {X6^Y6,         X6,            X7,            Y7^X10,        }, // 241
    {X6^Y6,         Y3,            X6,            X7^Y9,         }, // 242
    {X6^Y6,         X3,            Y3,            X6^Y9,         }, // 243
    {Z0^X6^Y6,      Y2,            X3,            Y3^X8,         }, // 244
    {Z0^X6^Y6,      X2,            Y2,            X3^Y8,         }, // 245
    {Z0^X6^Y6,      X6^Y8,         Y2,            X3,            }, // 246
    {Z0^X6^Y6,      X6^Y8,         X2,            Y2,            }, // 247
    {Y6^X7,         X7,            Y7,            X8^Y9,         }, // 248
    {Y6^X7,         Y3,            X7,            X8^Y8,         }, // 249
    {Y6^X7,         X3,            Y3,            X7^Y8,         }, // 250
    {Z1^Y6^X7,      Y2,            X3,            Y3^X8,         }, // 251
    {Z1^Y6^X7,      X2,            Y2,            Y3^X7,         }, // 252
    {Z1^Y6^X7,      Z0^X6^Y7,      Y2,            X3,            }, // 253
    {Z1^Y6^X7,      Z0^X6^Y7,      X2,            Y2,            }, // 254
    {X4^Y6,         X5^Y5,         X5,            Y6,            }, // 255
    {X3^Y6,         Y4^X5,         X4,            Y5,            }, // 256
    {X3^Y7,         Y5^X6,         X5^Y6,         Y6,            }, // 257
    {X2^Y7,         Y4^X5,         X3^Y6,         Y5,            }, // 258
    {Y3^X8,         Y5^X7,         X5^Y7,         X6^Y6,         }, // 259
    {Y3^X6,         X2^Y7,         X3^Y6,         X5^Y5,         }, // 260
    {X5,            Y6^X9,         X6^Y9,         Y7^X8,         }, // 261
    {X3,            Y3^X8,         X5^Y8,         Y6^X7,         }, // 262
    {Y2,            Y3^X7,         X3^Y8,         X5^Y7,         }, // 263
    {Y2,            Y3^X6,         X2^Y8,         X3^Y7,         }, // 264
    {Y6^X9,         X6^Y9,         Y7^X8,         X7^Y8,         }, // 265
    {Y3^X8,         X5^Y8,         Y6^X7,         X6^Y7,         }, // 266
    {Y2^X7,         Y3^X6,         X3^Y7,         X5^Y6,         }, // 267
    {X2,            Y2^X7,         Y3^X6,         X3^Y7,         }, // 268
    {Y3,            X5,            X6^Y10,        Y7^X9,         }, // 269
    {X3,            Y3,            X5^Y10,        X6^Y9,         }, // 270
    {Y2,            X3,            Y3^X8,         X5^Y9,         }, // 271
    {X2,            Y2,            X3^Y9,         Y3^X7,         }, // 272
    {Y1,            X2,            Y2^X7,         Y3^X6,         }, // 273
    {X6,            Y6^X10,        Y7^X9,         X7^Y9,         }, // 274
    {X3,            Y3^X9,         Y6^X8,         X6^Y8,         }, // 275
    {Y2,            Y3^X7,         X2^Y8,         X3^Y7,         }, // 276
    {Y3,            X6,            Y6^X10,        Y7^X9,         }, // 277
    {Y2,            X3,            Y3^X9,         Y6^X8,         }, // 278
    {Y1,            Y2,            Y3^X7,         X2^Y8,         }, // 279
    {X6^Y6,         Y3,            X6,            Y7^X9,         }, // 280
    {X6^Y6,         Y2,            X3,            Y3^X8,         }, // 281
    {X6^Y6,         X2,            Y2,            Y3^X7,         }, // 282
    {X6^Y6,         Y1,            Y2,            Y3^X6,         }, // 283
    {Y3,            X6,            Y7^X9,         X7^Y9,         }, // 284
    {Z0^X6^Y6,      Y1,            X2,            Y2^X7,         }, // 285
    {X6^Y6,         X6^Y8,         Y2,            X3,            }, // 286
    {Z0^X6^Y6,      X3^Y8,         Y1,            X2,            }, // 287
    {Y6^X7,         Y3,            X7,            Y7^X9,         }, // 288
    {Y6^X7,         Y2,            X3,            Y3^X8,         }, // 289
    {Z0^Y6^X7,      X2,            Y2,            Y3^X7,         }, // 290
    {Z0^Y6^X7,      X2,            X3,            Y3^X8,         }, // 291
    {Y6^X7,         X6^Y7,         Y2,            X3,            }, // 292
    {Z0^Y6^X7,      Z4^X6^Y7,      X2,            Y2,            }, // 293
    {Z0^Y6^X7,      Y1^X6^Y7,      X2,            X3,            }, // 294
    {Y3^X5,         X4^Y6,         Y4,            X5,            }, // 295
    {X3^Y6,         Y3^X5,         X4,            Y4,            }, // 296
    {X3^Y7,         Y3^X6,         X5^Y6,         Y4,            }, // 297
    {X2^Y7,         Y3^X5,         X3^Y6,         Y4,            }, // 298
    {Y2^X7,         X3^Y7,         Y3^X6,         X5^Y6,         }, // 299
    {Y2^X6,         X2^Y7,         Y3^X5,         X3^Y6,         }, // 300
    {X2,            Y2^X8,         X3^Y7,         Y3^X7,         }, // 301
    {Y1,            Y2^X6,         X2^Y7,         Y3^X5,         }, // 302
    {X2^Y7,         Y2^X6,         X3^Y6,         Y3^X5,         }, // 303
    {X2,            Y2^X7,         X3^Y7,         Y3^X6,         }, // 304
    {Y1,            X2^Y7,         Y2^X6,         X3^Y6,         }, // 305
    {Y1,            X2,            Y2^X8,         X3^Y7,         }, // 306
    {X1,            Y1,            X2^Y7,         Y2^X7,         }, // 307
    {Y1,            Y2^X7,         X2^Y7,         Y3^X6,         }, // 308
    {X1,            Y1,            Y2^X7,         X2^Y7,         }, // 309
    {X6^Y6,         Y1,            X2,            Y2^X8,         }, // 310
    {X3^Y6,         X1,            Y1,            Y2^X7,         }, // 311
    {Y1,            X2,            Y2^X8,         Y3^X7,         }, // 312
    {X2,            Y2,            Y3^X7,         X3^Y8,         }, // 313
    {X6^Y6,         X2,            Y2,            X3^Y8,         }, // 314
    {Z3^X6^Y6,      Y1,            X2,            Y2^X8,         }, // 315
    {Y1^X6^Y6,      X2,            Y2,            Y3^X7,         }, // 316
    {X6^Y6,         X6^Y8,         X2,            Y2,            }, // 317
    {Z3^X6^Y6,      X3^Y8,         Y1,            X2,            }, // 318
    {Y1^X6^Y6,      X1^X6^Y8,      X2,            Y2,            }, // 319
    {Y6^X7,         X2,            Y2,            Y3^X7,         }, // 320
    {Y1^Y6^X7,      X2,            X3,            Y3^X8,         }, // 321
    {Y1^Y6^X7,      X3,            Y3,            X2^Y7^X8,      }, // 322
    {Y6^X7,         X6^Y7,         X2,            Y2,            }, // 323
    {Y1^Y6^X7,      Z3^X6^Y7,      X2,            X3,            }, // 324
    {Y1^Y6^X7,      X1^X6^Y7,      X3,            Y3,            }, // 325
    {X2^Y6,         Y3^X5,         X3,            Y4,            }, // 326
    {X1^Y7,         Y3^X5,         X2^Y6,         Y4,            }, // 327
    {Y2^X6,         X1^Y7,         Y3^X5,         X2^Y6,         }, // 328
    {Y1,            Y2^X6,         X1^Y7,         Y3^X5,         }, // 329
    {Y1^X7,         Y2^X6,         X2^Y6,         Y3^X5,         }, // 330
    {X1,            Y1^X7,         Y2^X6,         X2^Y6,         }, // 331
    {X1,            Y1,            X2^Y8,         Y2^X6,         }, // 332
    {Y0,            X1,            Y1^X7,         Y2^X6,         }, // 333
    {X2,            Y2^X8,         Y3^X7,         X3^Y7,         }, // 334
    {X1,            X2,            Y2^X8,         Y3^X7,         }, // 335
    {Y1^X6^Y6,      X1,            X2,            Y2^X7,         }, // 336
    {X2,            X3,            Y3^X8,         Y2^X7^Y7,      }, // 337
    {X6^Y6,         Y1,            X2,            Y2^X7,         }, // 338
    {Y1^X6^Y6,      X2,            X3,            Y3^X8,         }, // 339
    {X6^Y6,         Y2^X6^Y8,      Y1,            X2,            }, // 340
    {Y1^X6^Y6,      X2^X6^Y8,      Y2,            X3,            }, // 341
    {Y1^X6^Y6,      Y3^X8,         X2,            X3,            }, // 342
    {Y6^X7,         X2,            X3,            Y3^X8,         }, // 343
    {Y1^Y6^X7,      X3,            Y2,            Y3^X8^Y8,      }, // 344
    {Y6^X7,         Y2^X6^Y7,      X2,            X3,            }, // 345
    {Y1^Y6^X7,      X1^X6^Y7,      X3,            Y2,            }, // 346
    {X4,            Z4,            Y4,            X5,            }, // 347
    {X3,            Z4,            Y4,            X4,            }, // 348
    {X3,            Z3,            Y4,            X4,            }, // 349
    {X3,            Z3,            Y3,            X4,            }, // 350
    {X2,            Z3,            Y3,            X3,            }, // 351
    {X4^Y4^Z4,      Z4,            Y4,            X5,            }, // 352
    {X3^Y4^Z4,      Z4,            Y4,            X4,            }, // 353
    {X3^Z3^Y4,      Z3,            Y4,            X4,            }, // 354
    {X3^Y3^Z3,      Z3,            Y3,            X4,            }, // 355
    {X2^Y3^Z3,      Z3,            Y3,            X3,            }, // 356
    {X4^Y5^Z5,      Y4^Z4^X5,      Y4,            X5,            }, // 357
    {X3^Y5^Z5,      X4^Y4^Z4,      Y4,            X4,            }, // 358
    {X3^Z4^Y5,      Z3^X4^Y4,      Y4,            X4,            }, // 359
    {X3^Y4^Z4,      Y3^Z3^X4,      Y3,            X4,            }, // 360
    {X2^Y4^Z4,      X3^Y3^Z3,      Y3,            X3,            }, // 361
    {X4,            Y4^Z4^X5,      Y4,            X5,            }, // 362
    {X3,            X4^Y4^Z4,      Y4,            X4,            }, // 363
    {X3,            Z3^X4^Y4,      Y4,            X4,            }, // 364
    {X3,            Y3^Z3^X4,      Y3,            X4,            }, // 365
    {X2,            X3^Y3^Z3,      Y3,            X3,            }, // 366
    {X3,            Z4,            Y4,            X5,            }, // 367
    {X2,            Z4,            Y4,            X3,            }, // 368
    {X2,            Z3,            Y4,            X3,            }, // 369
    {Y3,            X3,            Z4,            X5,            }, // 370
    {Y3,            X2,            Z4,            X3,            }, // 371
    {Y3,            X2,            Z3,            X3,            }, // 372
    {Y2,            X2,            Y3,            X3,            }, // 373
    {Z3,            X3,            Z4,            X5^Y5,         }, // 374
    {X2,            Z4,            X3,            Y2^X5^Y5,      }, // 375
    {X2,            Z3,            X3,            Y2^X5^Y5,      }, // 376
    {X2,            Y3,            X3,            Y1^X5^Y5,      }, // 377
    {X2,            Y3,            X3,            X1^X5^Y5,      }, // 378
    {Y3,            Z3,            X3,            Z4,            }, // 379
    {Y2,            Y3,            X3,            Z4,            }, // 380
    {Z3,            X3,            Z4,            X5^Y6,         }, // 381
    {X2,            Z4,            X3,            Z3^X5^Y6,      }, // 382
    {X2,            Z3,            X3,            Z2^X5^Y6,      }, // 383
    {X2,            Y3,            X3,            Z2^X5^Y6,      }, // 384
    {Z3^X7,         Y3,            X3,            Z4,            }, // 385
    {Z3^X7,         X2,            Z4,            X3,            }, // 386
    {Z2^X7,         X2,            Z3,            X3,            }, // 387
    {Z2^X7,         X2,            Y3,            X3,            }, // 388
    {Z3,            X3,            Z4,            Y3^X6^Y6,      }, // 389
    {X2,            Z4,            X3,            Y3^X6^Y6,      }, // 390
    {X2,            Z3,            X3,            Y3^X6^Y6,      }, // 391
    {X2,            Y3,            X3,            Y2^X6^Y6,      }, // 392
    {Y3^X6^Y6,      Z3,            X3,            Z4,            }, // 393
    {Y3^X6^Y6,      X2,            Z4,            X3,            }, // 394
    {Y3^X6^Y6,      X2,            Z3,            X3,            }, // 395
    {Y2^X6^Y6,      X2,            Y3,            X3,            }, // 396
    {Y3^X6^Y6,      Z3^X8,         X3,            Z4,            }, // 397
    {X2^X6^Y6,      Z3^X8,         Z4,            X3,            }, // 398
    {X2^X6^Y6,      Z2^X8,         Z3,            X3,            }, // 399
    {X2^X6^Y6,      Z2^X8,         Y3,            X3,            }, // 400
    {Y3^Y6^X7,      X3,            Z4,            Z3^X6^Y7,      }, // 401
    {Y3^Y6^X7,      Z4,            X3,            X2^X6^Y7,      }, // 402
    {Y3^Y6^X7,      Z3,            X3,            X2^X6^Y7,      }, // 403
    {Y2^Y6^X7,      Y3,            X3,            X2^X6^Y7,      }, // 404
    {Y3^Y6^X7,      Z3^X6^Y7,      X3,            Z4,            }, // 405
    {Y3^Y6^X7,      X2^X6^Y7,      Z4,            X3,            }, // 406
    {Y3^Y6^X7,      X2^X6^Y7,      Z3,            X3,            }, // 407
    {Y2^Y6^X7,      X2^X6^Y7,      Y3,            X3,            }, // 408
};

const UINT_64 GFX11_SW_PATTERN_NIBBLE4[][4] =
{
    {0,             0,             0,             0,             }, // 0
    {Y8,            X8,            0,             0,             }, // 1
    {Y7,            X8,            0,             0,             }, // 2
    {Y7,            X7,            0,             0,             }, // 3
    {Y6,            X7,            0,             0,             }, // 4
    {Y6,            X6,            0,             0,             }, // 5
    {X8,            Y8,            0,             0,             }, // 6
    {X7,            Y7,            0,             0,             }, // 7
    {X6,            Y6,            0,             0,             }, // 8
    {X8,            Y7,            0,             0,             }, // 9
    {X7,            Y6,            0,             0,             }, // 10
    {X8^Y8,         Y8,            0,             0,             }, // 11
    {Y7^X8,         Y7,            0,             0,             }, // 12
    {X7^Y7,         Y7,            0,             0,             }, // 13
    {Y6^X7,         Y6,            0,             0,             }, // 14
    {X6^Y6,         Y6,            0,             0,             }, // 15
    {Y8,            Z0^X5^Y5,      0,             0,             }, // 16
    {X8,            Z0^X5^Y5,      0,             0,             }, // 17
    {Y7,            Z0^X5^Y5,      0,             0,             }, // 18
    {X7,            Z0^X5^Y5,      0,             0,             }, // 19
    {Y6,            Z0^X5^Y5,      0,             0,             }, // 20
    {Y7^X8,         X8,            0,             0,             }, // 21
    {Y6^X7,         X7,            0,             0,             }, // 22
    {X8^Y9,         Y8^X9,         0,             0,             }, // 23
    {Y7^X9,         X8^Y8,         0,             0,             }, // 24
    {X7^Y8,         Y7^X8,         0,             0,             }, // 25
    {Y6^X8,         X7^Y7,         0,             0,             }, // 26
    {X6^Y7,         Y6^X7,         0,             0,             }, // 27
    {Y8^X9,         X5^Y6,         0,             0,             }, // 28
    {Y7^X9,         X5^Y6,         0,             0,             }, // 29
    {Y7^X8,         X5^Y6,         0,             0,             }, // 30
    {Y6^X8,         Z0^X5^Y6,      0,             0,             }, // 31
    {Y6^X7,         Z0^X5^Y6,      0,             0,             }, // 32
    {X8^Y8,         Y7^X9,         0,             0,             }, // 33
    {X7^Y7,         Y6^X8,         0,             0,             }, // 34
    {X3^Y7,         Y6^X7,         0,             0,             }, // 35
    {Y8^X9,         X6^Y6,         0,             0,             }, // 36
    {X8^Y8,         X6^Y6,         0,             0,             }, // 37
    {Y7^X8,         X6^Y6,         0,             0,             }, // 38
    {X7^Y7,         Z0^X6^Y6,      0,             0,             }, // 39
    {X6^Y7,         Z0^X6^Y6,      0,             0,             }, // 40
    {X6^Y8,         X7^Y7,         0,             0,             }, // 41
    {Y3^X7,         X6^Y7,         0,             0,             }, // 42
    {Y3^X8,         X7^Y7,         0,             0,             }, // 43
    {X3^Y7,         Y3^X7,         0,             0,             }, // 44
    {Y8^X9,         X6^Y7,         0,             0,             }, // 45
    {Y7^X9,         X6^Y7,         0,             0,             }, // 46
    {Y7^X8,         X6^Y7,         0,             0,             }, // 47
    {X7^Y7,         Z0^X6^Y7,      0,             0,             }, // 48
    {X3^Y7,         Z0^X6^Y7,      0,             0,             }, // 49
    {Y3^X7,         X3^Y7,         0,             0,             }, // 50
    {X7,            Y8,            0,             0,             }, // 51
    {X6,            Y7,            0,             0,             }, // 52
    {X5,            Y6,            0,             0,             }, // 53
    {X7^Y8,         Y8,            0,             0,             }, // 54
    {X6^Y7,         Y7,            0,             0,             }, // 55
    {X5^Y6,         Y6,            0,             0,             }, // 56
    {X7^Y9,         X8^Y8,         0,             0,             }, // 57
    {X5^Y8,         X6^Y7,         0,             0,             }, // 58
    {X3^Y8,         X5^Y7,         0,             0,             }, // 59
    {X8^Y8,         X5^Y6,         0,             0,             }, // 60
    {X7^Y7,         X5^Y6,         0,             0,             }, // 61
    {X6^Y6,         Z0^X5^Y6,      0,             0,             }, // 62
    {X3^Y7,         X6^Y6,         0,             0,             }, // 63
    {X3^Y8,         X6^Y7,         0,             0,             }, // 64
    {X2^Y8,         X3^Y7,         0,             0,             }, // 65
    {X7^Y7,         X6^Y6,         0,             0,             }, // 66
    {X3^Y7,         Z0^X6^Y6,      0,             0,             }, // 67
    {Y3^X6,         X3^Y7,         0,             0,             }, // 68
    {Y2^X7,         Y3^X6,         0,             0,             }, // 69
    {X8^Y8,         X6^Y7,         0,             0,             }, // 70
    {X7^Y7,         X6^Y7,         0,             0,             }, // 71
    {X3^Y7,         Z4^X6^Y7,      0,             0,             }, // 72
    {Y2^X7^Y7,      Y1^X6^Y7,      0,             0,             }, // 73
    {Y3^X8,         Y2^X7^Y7,      0,             0,             }, // 74
    {Y5,            X6,            0,             0,             }, // 75
    {X5,            Y5,            0,             0,             }, // 76
    {X6,            Y5,            0,             0,             }, // 77
    {X6^Y6,         Y5,            0,             0,             }, // 78
    {X3^Y6,         Y5,            0,             0,             }, // 79
    {X6,            Z0^X5^Y5,      0,             0,             }, // 80
    {X5,            Z0^X5^Y5,      0,             0,             }, // 81
    {X5^Y6,         X6,            0,             0,             }, // 82
    {Y3^X5,         X5,            0,             0,             }, // 83
    {Y3^X7,         X6^Y6,         0,             0,             }, // 84
    {X3^Y6,         Y3^X6,         0,             0,             }, // 85
    {Y6^X8,         X5^Y6,         0,             0,             }, // 86
    {Y6^X7,         X5^Y6,         0,             0,             }, // 87
    {X3^Y6,         Z0^X5^Y6,      0,             0,             }, // 88
    {Y3^X6,         X3^Y6,         0,             0,             }, // 89
    {X3^Y6,         Y3^X7,         0,             0,             }, // 90
    {X2^Y6,         Y3^X6,         0,             0,             }, // 91
    {X6^Y7,         X6^Y6,         0,             0,             }, // 92
    {X3^Y6,         Z3^X6^Y6,      0,             0,             }, // 93
    {X1^X6^Y7,      Y1^X6^Y6,      0,             0,             }, // 94
    {Y3^X7,         X3^Y6,         0,             0,             }, // 95
    {X3^Y8,         X1^X6^Y7,      0,             0,             }, // 96
    {Y2^X8,         Y3^X7,         0,             0,             }, // 97
    {X3^Y7,         X6^Y7,         0,             0,             }, // 98
    {Y2^X7^Y7,      Z3^X6^Y7,      0,             0,             }, // 99
    {Y2^X7^Y8,      X1^X6^Y7,      0,             0,             }, // 100
    {X2^Y7^X8,      Y2^X7^Y8,      0,             0,             }, // 101
    {X4,            Y5,            0,             0,             }, // 102
    {X3,            Y5,            0,             0,             }, // 103
    {X3^Y6,         X5,            0,             0,             }, // 104
    {X2^Y6,         X3,            0,             0,             }, // 105
    {X3,            Z0^X5^Y5,      0,             0,             }, // 106
    {Y3^X5,         X3,            0,             0,             }, // 107
    {X3^Y7,         X5^Y6,         0,             0,             }, // 108
    {X2^Y6,         Y3^X5,         0,             0,             }, // 109
    {X6^Y6,         X5^Y6,         0,             0,             }, // 110
    {X3^Y6,         Z2^X5^Y6,      0,             0,             }, // 111
    {Y1^X6^Y6,      Y0^X5^Y6,      0,             0,             }, // 112
    {X3^Y7,         Y1^X6^Y6,      0,             0,             }, // 113
    {X1^X6^Y8,      Y1^X6^Y6,      0,             0,             }, // 114
    {Y2^X7^Y7,      X1^X6^Y8,      0,             0,             }, // 115
    {Y3^X7,         X1^X6^Y7,      0,             0,             }, // 116
    {Y1^X7^Y7,      Y2^X6^Y7,      0,             0,             }, // 117
    {X2^Y7^X9,      X1^X6^Y7,      0,             0,             }, // 118
    {Y3^X8,         Y1^X7^Y7,      0,             0,             }, // 119
    {Y3^X8^Y8,      X2^Y7^X9,      0,             0,             }, // 120
    {Z5,            Y5,            0,             0,             }, // 121
    {Z4,            Y5,            0,             0,             }, // 122
    {Z4,            Y4,            0,             0,             }, // 123
};

const UINT_8 GFX11_DCC_64K_R_X_PATIDX[] =
{
       0, // 1 bpe ua @ SW_64K_{Z,R}_X 1xaa
       1, // 2 bpe ua @ SW_64K_{Z,R}_X 1xaa
       2, // 4 bpe ua @ SW_64K_{Z,R}_X 1xaa
       3, // 8 bpe ua @ SW_64K_{Z,R}_X 1xaa
       4, // 16 bpe ua @ SW_64K_{Z,R}_X 1xaa
       0, // 1 pipes (1 PKRs) 1 bpe pa @ SW_64K_{Z,R}_X 1xaa
       1, // 1 pipes (1 PKRs) 2 bpe pa @ SW_64K_{Z,R}_X 1xaa
       2, // 1 pipes (1 PKRs) 4 bpe pa @ SW_64K_{Z,R}_X 1xaa
       3, // 1 pipes (1 PKRs) 8 bpe pa @ SW_64K_{Z,R}_X 1xaa
       4, // 1 pipes (1 PKRs) 16 bpe pa @ SW_64K_{Z,R}_X 1xaa
       5, // 2 pipes (1-2 PKRs) 1 bpe pa @ SW_64K_{Z,R}_X 1xaa
       6, // 2 pipes (1-2 PKRs) 2 bpe pa @ SW_64K_{Z,R}_X 1xaa
       7, // 2 pipes (1-2 PKRs) 4 bpe pa @ SW_64K_{Z,R}_X 1xaa
       8, // 2 pipes (1-2 PKRs) 8 bpe pa @ SW_64K_{Z,R}_X 1xaa
       9, // 2 pipes (1-2 PKRs) 16 bpe pa @ SW_64K_{Z,R}_X 1xaa
      10, // 4 pipes (1-2 PKRs) 1 bpe pa @ SW_64K_{Z,R}_X 1xaa
      11, // 4 pipes (1-2 PKRs) 2 bpe pa @ SW_64K_{Z,R}_X 1xaa
      12, // 4 pipes (1-2 PKRs) 4 bpe pa @ SW_64K_{Z,R}_X 1xaa
      13, // 4 pipes (1-2 PKRs) 8 bpe pa @ SW_64K_{Z,R}_X 1xaa
      14, // 4 pipes (1-2 PKRs) 16 bpe pa @ SW_64K_{Z,R}_X 1xaa
      15, // 8 pipes (2 PKRs) 1 bpe pa @ SW_64K_{Z,R}_X 1xaa
      16, // 8 pipes (2 PKRs) 2 bpe pa @ SW_64K_{Z,R}_X 1xaa
      17, // 8 pipes (2 PKRs) 4 bpe pa @ SW_64K_{Z,R}_X 1xaa
      18, // 8 pipes (2 PKRs) 8 bpe pa @ SW_64K_{Z,R}_X 1xaa
      19, // 8 pipes (2 PKRs) 16 bpe pa @ SW_64K_{Z,R}_X 1xaa
      20, // 4 pipes (4 PKRs) 1 bpe pa @ SW_64K_{Z,R}_X 1xaa
      21, // 4 pipes (4 PKRs) 2 bpe pa @ SW_64K_{Z,R}_X 1xaa
      22, // 4 pipes (4 PKRs) 4 bpe pa @ SW_64K_{Z,R}_X 1xaa
      23, // 4 pipes (4 PKRs) 8 bpe pa @ SW_64K_{Z,R}_X 1xaa
      24, // 4 pipes (4 PKRs) 16 bpe pa @ SW_64K_{Z,R}_X 1xaa
      25, // 8 pipes (4 PKRs) 1 bpe pa @ SW_64K_{Z,R}_X 1xaa
      26, // 8 pipes (4 PKRs) 2 bpe pa @ SW_64K_{Z,R}_X 1xaa
      27, // 8 pipes (4 PKRs) 4 bpe pa @ SW_64K_{Z,R}_X 1xaa
      28, // 8 pipes (4 PKRs) 8 bpe pa @ SW_64K_{Z,R}_X 1xaa
      29, // 8 pipes (4 PKRs) 16 bpe pa @ SW_64K_{Z,R}_X 1xaa
      30, // 16 pipes (4 PKRs) 1 bpe pa @ SW_64K_{Z,R}_X 1xaa
      31, // 16 pipes (4 PKRs) 2 bpe pa @ SW_64K_{Z,R}_X 1xaa
      32, // 16 pipes (4 PKRs) 4 bpe pa @ SW_64K_{Z,R}_X 1xaa
      33, // 16 pipes (4 PKRs) 8 bpe pa @ SW_64K_{Z,R}_X 1xaa
      34, // 16 pipes (4 PKRs) 16 bpe pa @ SW_64K_{Z,R}_X 1xaa
      35, // 8 pipes (8 PKRs) 1 bpe pa @ SW_64K_{Z,R}_X 1xaa
      36, // 8 pipes (8 PKRs) 2 bpe pa @ SW_64K_{Z,R}_X 1xaa
      37, // 8 pipes (8 PKRs) 4 bpe pa @ SW_64K_{Z,R}_X 1xaa
      38, // 8 pipes (8 PKRs) 8 bpe pa @ SW_64K_{Z,R}_X 1xaa
      39, // 8 pipes (8 PKRs) 16 bpe pa @ SW_64K_{Z,R}_X 1xaa
      35, // 16 pipes (8 PKRs) 1 bpe pa @ SW_64K_{Z,R}_X 1xaa
      36, // 16 pipes (8 PKRs) 2 bpe pa @ SW_64K_{Z,R}_X 1xaa
      37, // 16 pipes (8 PKRs) 4 bpe pa @ SW_64K_{Z,R}_X 1xaa
      40, // 16 pipes (8 PKRs) 8 bpe pa @ SW_64K_{Z,R}_X 1xaa
      41, // 16 pipes (8 PKRs) 16 bpe pa @ SW_64K_{Z,R}_X 1xaa
      42, // 32 pipes (8 PKRs) 1 bpe pa @ SW_64K_{Z,R}_X 1xaa
      43, // 32 pipes (8 PKRs) 2 bpe pa @ SW_64K_{Z,R}_X 1xaa
      44, // 32 pipes (8 PKRs) 4 bpe pa @ SW_64K_{Z,R}_X 1xaa
      45, // 32 pipes (8 PKRs) 8 bpe pa @ SW_64K_{Z,R}_X 1xaa
      46, // 32 pipes (8 PKRs) 16 bpe pa @ SW_64K_{Z,R}_X 1xaa
      47, // 16 pipes (16 PKRs) 1 bpe pa @ SW_64K_{Z,R}_X 1xaa
      48, // 16 pipes (16 PKRs) 2 bpe pa @ SW_64K_{Z,R}_X 1xaa
      49, // 16 pipes (16 PKRs) 4 bpe pa @ SW_64K_{Z,R}_X 1xaa
      50, // 16 pipes (16 PKRs) 8 bpe pa @ SW_64K_{Z,R}_X 1xaa
      51, // 16 pipes (16 PKRs) 16 bpe pa @ SW_64K_{Z,R}_X 1xaa
      47, // 32 pipes (16 PKRs) 1 bpe pa @ SW_64K_{Z,R}_X 1xaa
      48, // 32 pipes (16 PKRs) 2 bpe pa @ SW_64K_{Z,R}_X 1xaa
      49, // 32 pipes (16 PKRs) 4 bpe pa @ SW_64K_{Z,R}_X 1xaa
      52, // 32 pipes (16 PKRs) 8 bpe pa @ SW_64K_{Z,R}_X 1xaa
      53, // 32 pipes (16 PKRs) 16 bpe pa @ SW_64K_{Z,R}_X 1xaa
      54, // 64 pipes (16 PKRs) 1 bpe pa @ SW_64K_{Z,R}_X 1xaa
      55, // 64 pipes (16 PKRs) 2 bpe pa @ SW_64K_{Z,R}_X 1xaa
      56, // 64 pipes (16 PKRs) 4 bpe pa @ SW_64K_{Z,R}_X 1xaa
      57, // 64 pipes (16 PKRs) 8 bpe pa @ SW_64K_{Z,R}_X 1xaa
      58, // 64 pipes (16 PKRs) 16 bpe pa @ SW_64K_{Z,R}_X 1xaa
      59, // 32 pipes (32 PKRs) 1 bpe pa @ SW_64K_{Z,R}_X 1xaa
      60, // 32 pipes (32 PKRs) 2 bpe pa @ SW_64K_{Z,R}_X 1xaa
      61, // 32 pipes (32 PKRs) 4 bpe pa @ SW_64K_{Z,R}_X 1xaa
      62, // 32 pipes (32 PKRs) 8 bpe pa @ SW_64K_{Z,R}_X 1xaa
      63, // 32 pipes (32 PKRs) 16 bpe pa @ SW_64K_{Z,R}_X 1xaa
      59, // 64 pipes (32 PKRs) 1 bpe pa @ SW_64K_{Z,R}_X 1xaa
      60, // 64 pipes (32 PKRs) 2 bpe pa @ SW_64K_{Z,R}_X 1xaa
      61, // 64 pipes (32 PKRs) 4 bpe pa @ SW_64K_{Z,R}_X 1xaa
      64, // 64 pipes (32 PKRs) 8 bpe pa @ SW_64K_{Z,R}_X 1xaa
      65, // 64 pipes (32 PKRs) 16 bpe pa @ SW_64K_{Z,R}_X 1xaa
};

const UINT_8 GFX11_DCC_256K_R_X_PATIDX[] =
{
       0, // 1 bpe ua @ SW_256K_{Z,R}_X 1xaa
       1, // 2 bpe ua @ SW_256K_{Z,R}_X 1xaa
       2, // 4 bpe ua @ SW_256K_{Z,R}_X 1xaa
       3, // 8 bpe ua @ SW_256K_{Z,R}_X 1xaa
       4, // 16 bpe ua @ SW_256K_{Z,R}_X 1xaa
       0, // 1 pipes (1 PKRs) 1 bpe pa @ SW_256K_{Z,R}_X 1xaa
       1, // 1 pipes (1 PKRs) 2 bpe pa @ SW_256K_{Z,R}_X 1xaa
       2, // 1 pipes (1 PKRs) 4 bpe pa @ SW_256K_{Z,R}_X 1xaa
       3, // 1 pipes (1 PKRs) 8 bpe pa @ SW_256K_{Z,R}_X 1xaa
       4, // 1 pipes (1 PKRs) 16 bpe pa @ SW_256K_{Z,R}_X 1xaa
       5, // 2 pipes (1-2 PKRs) 1 bpe pa @ SW_256K_{Z,R}_X 1xaa
       6, // 2 pipes (1-2 PKRs) 2 bpe pa @ SW_256K_{Z,R}_X 1xaa
       7, // 2 pipes (1-2 PKRs) 4 bpe pa @ SW_256K_{Z,R}_X 1xaa
       8, // 2 pipes (1-2 PKRs) 8 bpe pa @ SW_256K_{Z,R}_X 1xaa
       9, // 2 pipes (1-2 PKRs) 16 bpe pa @ SW_256K_{Z,R}_X 1xaa
      10, // 4 pipes (1-2 PKRs) 1 bpe pa @ SW_256K_{Z,R}_X 1xaa
      11, // 4 pipes (1-2 PKRs) 2 bpe pa @ SW_256K_{Z,R}_X 1xaa
      12, // 4 pipes (1-2 PKRs) 4 bpe pa @ SW_256K_{Z,R}_X 1xaa
      13, // 4 pipes (1-2 PKRs) 8 bpe pa @ SW_256K_{Z,R}_X 1xaa
      14, // 4 pipes (1-2 PKRs) 16 bpe pa @ SW_256K_{Z,R}_X 1xaa
      15, // 8 pipes (2 PKRs) 1 bpe pa @ SW_256K_{Z,R}_X 1xaa
      16, // 8 pipes (2 PKRs) 2 bpe pa @ SW_256K_{Z,R}_X 1xaa
      17, // 8 pipes (2 PKRs) 4 bpe pa @ SW_256K_{Z,R}_X 1xaa
      18, // 8 pipes (2 PKRs) 8 bpe pa @ SW_256K_{Z,R}_X 1xaa
      19, // 8 pipes (2 PKRs) 16 bpe pa @ SW_256K_{Z,R}_X 1xaa
      20, // 4 pipes (4 PKRs) 1 bpe pa @ SW_256K_{Z,R}_X 1xaa
      21, // 4 pipes (4 PKRs) 2 bpe pa @ SW_256K_{Z,R}_X 1xaa
      22, // 4 pipes (4 PKRs) 4 bpe pa @ SW_256K_{Z,R}_X 1xaa
      23, // 4 pipes (4 PKRs) 8 bpe pa @ SW_256K_{Z,R}_X 1xaa
      24, // 4 pipes (4 PKRs) 16 bpe pa @ SW_256K_{Z,R}_X 1xaa
      25, // 8 pipes (4 PKRs) 1 bpe pa @ SW_256K_{Z,R}_X 1xaa
      26, // 8 pipes (4 PKRs) 2 bpe pa @ SW_256K_{Z,R}_X 1xaa
      27, // 8 pipes (4 PKRs) 4 bpe pa @ SW_256K_{Z,R}_X 1xaa
      28, // 8 pipes (4 PKRs) 8 bpe pa @ SW_256K_{Z,R}_X 1xaa
      29, // 8 pipes (4 PKRs) 16 bpe pa @ SW_256K_{Z,R}_X 1xaa
      30, // 16 pipes (4 PKRs) 1 bpe pa @ SW_256K_{Z,R}_X 1xaa
      31, // 16 pipes (4 PKRs) 2 bpe pa @ SW_256K_{Z,R}_X 1xaa
      32, // 16 pipes (4 PKRs) 4 bpe pa @ SW_256K_{Z,R}_X 1xaa
      33, // 16 pipes (4 PKRs) 8 bpe pa @ SW_256K_{Z,R}_X 1xaa
      34, // 16 pipes (4 PKRs) 16 bpe pa @ SW_256K_{Z,R}_X 1xaa
      35, // 8 pipes (8 PKRs) 1 bpe pa @ SW_256K_{Z,R}_X 1xaa
      36, // 8 pipes (8 PKRs) 2 bpe pa @ SW_256K_{Z,R}_X 1xaa
      37, // 8 pipes (8 PKRs) 4 bpe pa @ SW_256K_{Z,R}_X 1xaa
      38, // 8 pipes (8 PKRs) 8 bpe pa @ SW_256K_{Z,R}_X 1xaa
      39, // 8 pipes (8 PKRs) 16 bpe pa @ SW_256K_{Z,R}_X 1xaa
      35, // 16 pipes (8 PKRs) 1 bpe pa @ SW_256K_{Z,R}_X 1xaa
      36, // 16 pipes (8 PKRs) 2 bpe pa @ SW_256K_{Z,R}_X 1xaa
      37, // 16 pipes (8 PKRs) 4 bpe pa @ SW_256K_{Z,R}_X 1xaa
      40, // 16 pipes (8 PKRs) 8 bpe pa @ SW_256K_{Z,R}_X 1xaa
      41, // 16 pipes (8 PKRs) 16 bpe pa @ SW_256K_{Z,R}_X 1xaa
      42, // 32 pipes (8 PKRs) 1 bpe pa @ SW_256K_{Z,R}_X 1xaa
      43, // 32 pipes (8 PKRs) 2 bpe pa @ SW_256K_{Z,R}_X 1xaa
      44, // 32 pipes (8 PKRs) 4 bpe pa @ SW_256K_{Z,R}_X 1xaa
      45, // 32 pipes (8 PKRs) 8 bpe pa @ SW_256K_{Z,R}_X 1xaa
      66, // 32 pipes (8 PKRs) 16 bpe pa @ SW_256K_{Z,R}_X 1xaa
      47, // 16 pipes (16 PKRs) 1 bpe pa @ SW_256K_{Z,R}_X 1xaa
      48, // 16 pipes (16 PKRs) 2 bpe pa @ SW_256K_{Z,R}_X 1xaa
      49, // 16 pipes (16 PKRs) 4 bpe pa @ SW_256K_{Z,R}_X 1xaa
      50, // 16 pipes (16 PKRs) 8 bpe pa @ SW_256K_{Z,R}_X 1xaa
      67, // 16 pipes (16 PKRs) 16 bpe pa @ SW_256K_{Z,R}_X 1xaa
      47, // 32 pipes (16 PKRs) 1 bpe pa @ SW_256K_{Z,R}_X 1xaa
      48, // 32 pipes (16 PKRs) 2 bpe pa @ SW_256K_{Z,R}_X 1xaa
      49, // 32 pipes (16 PKRs) 4 bpe pa @ SW_256K_{Z,R}_X 1xaa
      52, // 32 pipes (16 PKRs) 8 bpe pa @ SW_256K_{Z,R}_X 1xaa
      68, // 32 pipes (16 PKRs) 16 bpe pa @ SW_256K_{Z,R}_X 1xaa
      54, // 64 pipes (16 PKRs) 1 bpe pa @ SW_256K_{Z,R}_X 1xaa
      55, // 64 pipes (16 PKRs) 2 bpe pa @ SW_256K_{Z,R}_X 1xaa
      56, // 64 pipes (16 PKRs) 4 bpe pa @ SW_256K_{Z,R}_X 1xaa
      69, // 64 pipes (16 PKRs) 8 bpe pa @ SW_256K_{Z,R}_X 1xaa
      70, // 64 pipes (16 PKRs) 16 bpe pa @ SW_256K_{Z,R}_X 1xaa
      59, // 32 pipes (32 PKRs) 1 bpe pa @ SW_256K_{Z,R}_X 1xaa
      60, // 32 pipes (32 PKRs) 2 bpe pa @ SW_256K_{Z,R}_X 1xaa
      61, // 32 pipes (32 PKRs) 4 bpe pa @ SW_256K_{Z,R}_X 1xaa
      71, // 32 pipes (32 PKRs) 8 bpe pa @ SW_256K_{Z,R}_X 1xaa
      72, // 32 pipes (32 PKRs) 16 bpe pa @ SW_256K_{Z,R}_X 1xaa
      59, // 64 pipes (32 PKRs) 1 bpe pa @ SW_256K_{Z,R}_X 1xaa
      60, // 64 pipes (32 PKRs) 2 bpe pa @ SW_256K_{Z,R}_X 1xaa
      61, // 64 pipes (32 PKRs) 4 bpe pa @ SW_256K_{Z,R}_X 1xaa
      73, // 64 pipes (32 PKRs) 8 bpe pa @ SW_256K_{Z,R}_X 1xaa
      74, // 64 pipes (32 PKRs) 16 bpe pa @ SW_256K_{Z,R}_X 1xaa
};

const UINT_8 GFX11_HTILE_PATIDX[] =
{
       0, // 1xaa ua @ HTILE_64K
       0, // 2xaa ua @ HTILE_64K
       0, // 4xaa ua @ HTILE_64K
       0, // 8xaa ua @ HTILE_64K
       0, // 1 pipes (1-2 PKRs) 1xaa pa @ HTILE_64K
       0, // 1 pipes (1-2 PKRs) 2xaa pa @ HTILE_64K
       0, // 1 pipes (1-2 PKRs) 4xaa pa @ HTILE_64K
       0, // 1 pipes (1-2 PKRs) 8xaa pa @ HTILE_64K
       1, // 2 pipes (1-2 PKRs) 1xaa pa @ HTILE_64K
       1, // 2 pipes (1-2 PKRs) 2xaa pa @ HTILE_64K
       1, // 2 pipes (1-2 PKRs) 4xaa pa @ HTILE_64K
       1, // 2 pipes (1-2 PKRs) 8xaa pa @ HTILE_64K
       2, // 4 pipes (1-2 PKRs) 1xaa pa @ HTILE_64K
       2, // 4 pipes (1-2 PKRs) 2xaa pa @ HTILE_64K
       2, // 4 pipes (1-2 PKRs) 4xaa pa @ HTILE_64K
       2, // 4 pipes (1-2 PKRs) 8xaa pa @ HTILE_64K
       3, // 8 pipes (1-2 PKRs) 1xaa pa @ HTILE_64K
       3, // 8 pipes (1-2 PKRs) 2xaa pa @ HTILE_64K
       3, // 8 pipes (1-2 PKRs) 4xaa pa @ HTILE_64K
       3, // 8 pipes (1-2 PKRs) 8xaa pa @ HTILE_64K
       1, // 2 pipes (4 PKRs) 1xaa pa @ HTILE_64K
       1, // 2 pipes (4 PKRs) 2xaa pa @ HTILE_64K
       1, // 2 pipes (4 PKRs) 4xaa pa @ HTILE_64K
       1, // 2 pipes (4 PKRs) 8xaa pa @ HTILE_64K
       4, // 4 pipes (4 PKRs) 1xaa pa @ HTILE_64K
       4, // 4 pipes (4 PKRs) 2xaa pa @ HTILE_64K
       4, // 4 pipes (4 PKRs) 4xaa pa @ HTILE_64K
       4, // 4 pipes (4 PKRs) 8xaa pa @ HTILE_64K
       5, // 8 pipes (4 PKRs) 1xaa pa @ HTILE_64K
       5, // 8 pipes (4 PKRs) 2xaa pa @ HTILE_64K
       5, // 8 pipes (4 PKRs) 4xaa pa @ HTILE_64K
       5, // 8 pipes (4 PKRs) 8xaa pa @ HTILE_64K
       6, // 16 pipes (4 PKRs) 1xaa pa @ HTILE_64K
       6, // 16 pipes (4 PKRs) 2xaa pa @ HTILE_64K
       6, // 16 pipes (4 PKRs) 4xaa pa @ HTILE_64K
       6, // 16 pipes (4 PKRs) 8xaa pa @ HTILE_64K
       7, // 4 pipes (8 PKRs) 1xaa pa @ HTILE_64K
       7, // 4 pipes (8 PKRs) 2xaa pa @ HTILE_64K
       7, // 4 pipes (8 PKRs) 4xaa pa @ HTILE_64K
       7, // 4 pipes (8 PKRs) 8xaa pa @ HTILE_64K
       8, // 8 pipes (8 PKRs) 1xaa pa @ HTILE_64K
       8, // 8 pipes (8 PKRs) 2xaa pa @ HTILE_64K
       8, // 8 pipes (8 PKRs) 4xaa pa @ HTILE_64K
       8, // 8 pipes (8 PKRs) 8xaa pa @ HTILE_64K
       9, // 16 pipes (8 PKRs) 1xaa pa @ HTILE_64K
       9, // 16 pipes (8 PKRs) 2xaa pa @ HTILE_64K
       9, // 16 pipes (8 PKRs) 4xaa pa @ HTILE_64K
       9, // 16 pipes (8 PKRs) 8xaa pa @ HTILE_64K
      10, // 32 pipes (8 PKRs) 1xaa pa @ HTILE_64K
      10, // 32 pipes (8 PKRs) 2xaa pa @ HTILE_64K
      10, // 32 pipes (8 PKRs) 4xaa pa @ HTILE_64K
      10, // 32 pipes (8 PKRs) 8xaa pa @ HTILE_64K
      11, // 8 pipes (16 PKRs) 1xaa pa @ HTILE_64K
      11, // 8 pipes (16 PKRs) 2xaa pa @ HTILE_64K
      11, // 8 pipes (16 PKRs) 4xaa pa @ HTILE_64K
      11, // 8 pipes (16 PKRs) 8xaa pa @ HTILE_64K
      12, // 16 pipes (16 PKRs) 1xaa pa @ HTILE_64K
      12, // 16 pipes (16 PKRs) 2xaa pa @ HTILE_64K
      12, // 16 pipes (16 PKRs) 4xaa pa @ HTILE_64K
      12, // 16 pipes (16 PKRs) 8xaa pa @ HTILE_64K
      13, // 32 pipes (16 PKRs) 1xaa pa @ HTILE_64K
      13, // 32 pipes (16 PKRs) 2xaa pa @ HTILE_64K
      13, // 32 pipes (16 PKRs) 4xaa pa @ HTILE_64K
      13, // 32 pipes (16 PKRs) 8xaa pa @ HTILE_64K
      14, // 64 pipes (16 PKRs) 1xaa pa @ HTILE_64K
      14, // 64 pipes (16 PKRs) 2xaa pa @ HTILE_64K
      14, // 64 pipes (16 PKRs) 4xaa pa @ HTILE_64K
      14, // 64 pipes (16 PKRs) 8xaa pa @ HTILE_64K
      15, // 16 pipes (32 PKRs) 1xaa pa @ HTILE_64K
      15, // 16 pipes (32 PKRs) 2xaa pa @ HTILE_64K
      15, // 16 pipes (32 PKRs) 4xaa pa @ HTILE_64K
      15, // 16 pipes (32 PKRs) 8xaa pa @ HTILE_64K
      16, // 32 pipes (32 PKRs) 1xaa pa @ HTILE_64K
      16, // 32 pipes (32 PKRs) 2xaa pa @ HTILE_64K
      16, // 32 pipes (32 PKRs) 4xaa pa @ HTILE_64K
      16, // 32 pipes (32 PKRs) 8xaa pa @ HTILE_64K
      17, // 64 pipes (32 PKRs) 1xaa pa @ HTILE_64K
      17, // 64 pipes (32 PKRs) 2xaa pa @ HTILE_64K
      17, // 64 pipes (32 PKRs) 4xaa pa @ HTILE_64K
      17, // 64 pipes (32 PKRs) 8xaa pa @ HTILE_64K
};

const UINT_64 GFX11_DCC_R_X_SW_PATTERN[][17] =
{
    {0,             X4,            Y4,            X5,            Y5,            X6,            Y6,            X7,            Y7,            X8,            Y8,            X9,            Y9,            0,             0,             0,             0,             }, //0
    {0,             Y3,            X4,            Y4,            X5,            Y5,            X6,            Y6,            X7,            Y7,            X8,            Y8,            X9,            0,             0,             0,             0,             }, //1
    {0,             X3,            Y3,            X4,            Y4,            X5,            Y5,            X6,            Y6,            X7,            Y7,            X8,            Y8,            0,             0,             0,             0,             }, //2
    {0,             Y2,            X3,            Y3,            X4,            Y4,            X5,            Y5,            X6,            Y6,            X7,            Y7,            X8,            0,             0,             0,             0,             }, //3
    {0,             X2,            Y2,            X3,            Y3,            X4,            Y4,            X5,            Y5,            X6,            Y6,            X7,            Y7,            0,             0,             0,             0,             }, //4
    {0,             Y4,            X5,            Y5,            X6,            Y6,            X7,            Y7,            X8,            Z0^X4^Y4,      Y8,            X9,            Y9,            0,             0,             0,             0,             }, //5
    {0,             Y3,            Y4,            X5,            Y5,            X6,            Y6,            X7,            Y7,            Z0^X4^Y4,      X8,            Y8,            X9,            0,             0,             0,             0,             }, //6
    {0,             X3,            Y3,            Y4,            X5,            Y5,            X6,            Y6,            X7,            Z0^X4^Y4,      Y7,            X8,            Y8,            0,             0,             0,             0,             }, //7
    {0,             Y2,            X3,            Y3,            Y4,            X5,            Y5,            X6,            Y6,            Z0^X4^Y4,      X7,            Y7,            X8,            0,             0,             0,             0,             }, //8
    {0,             X2,            Y2,            X3,            Y3,            Y4,            X5,            Y5,            X6,            Z0^X4^Y4,      Y6,            X7,            Y7,            0,             0,             0,             0,             }, //9
    {0,             X5,            Y5,            X6,            Y6,            X7,            Y7,            X8,            Y8,            Y4^X5^Y5,      Z0^X4^Y4,      X9,            Y9,            0,             0,             0,             0,             }, //10
    {0,             Y3,            X5,            Y5,            X6,            Y6,            X7,            Y7,            X8,            Y4^X5^Y5,      Z0^X4^Y4,      Y8,            X9,            0,             0,             0,             0,             }, //11
    {0,             X3,            Y3,            X5,            Y5,            X6,            Y6,            X7,            Y7,            Y4^X5^Y5,      Z0^X4^Y4,      X8,            Y8,            0,             0,             0,             0,             }, //12
    {0,             Y2,            X3,            Y3,            X5,            Y5,            X6,            Y6,            X7,            Y4^X5^Y5,      Z0^X4^Y4,      Y7,            X8,            0,             0,             0,             0,             }, //13
    {0,             X2,            Y2,            X3,            Y3,            X5,            Y5,            X6,            Y6,            Y4^X5^Y5,      Z0^X4^Y4,      X7,            Y7,            0,             0,             0,             0,             }, //14
    {0,             Y5,            X6,            Y6,            X7,            Y7,            X8,            Y8,            X9,            Y4^X5^Y5,      Z0^X4^Y4,      X5^Y5,         Y9,            0,             0,             0,             0,             }, //15
    {0,             Y3,            Y5,            X6,            Y6,            X7,            Y7,            X8,            Y8,            Y4^X5^Y5,      Z0^X4^Y4,      X5^Y5,         X9,            0,             0,             0,             0,             }, //16
    {0,             X3,            Y3,            Y5,            X6,            Y6,            X7,            Y7,            X8,            Y4^X5^Y5,      Z0^X4^Y4,      X5^Y5,         Y8,            0,             0,             0,             0,             }, //17
    {0,             Y2,            X3,            Y3,            Y5,            X6,            Y6,            X7,            Y7,            Y4^X5^Y5,      Z0^X4^Y4,      X5^Y5,         X8,            0,             0,             0,             0,             }, //18
    {0,             X2,            Y2,            X3,            Y3,            Y5,            X6,            Y6,            X7,            Y4^X5^Y5,      Z0^X4^Y4,      X5^Y5,         Y7,            0,             0,             0,             0,             }, //19
    {0,             X5,            X6,            Y6,            X7,            Y7,            X8,            Y8,            X9,            Y4^X6^Y6,      Z1^X4^Y4,      X5^Y5,         Y9,            0,             0,             0,             0,             }, //20
    {0,             Y3,            X5,            X6,            Y6,            X7,            Y7,            X8,            Y8,            Y4^X6^Y6,      Z1^X4^Y4,      X5^Y5,         X9,            0,             0,             0,             0,             }, //21
    {0,             X3,            Y3,            X5,            X6,            Y6,            X7,            Y7,            X8,            Y4^X6^Y6,      Z1^X4^Y4,      X5^Y5,         Y8,            0,             0,             0,             0,             }, //22
    {0,             Y2,            X3,            Y3,            X5,            X6,            Y6,            X7,            Y7,            Y4^X6^Y6,      Z1^X4^Y4,      X5^Y5,         X8,            0,             0,             0,             0,             }, //23
    {0,             X2,            Y2,            X3,            Y3,            X5,            X6,            Y6,            X7,            Y4^X6^Y6,      Z1^X4^Y4,      X5^Y5,         Y7,            0,             0,             0,             0,             }, //24
    {0,             X5,            X6,            Y6,            X7,            Y7,            X8,            Y8,            X9,            Y4^X6^Y6,      Z1^X4^Y4,      Z0^X5^Y5,      Y9,            0,             0,             0,             0,             }, //25
    {0,             Y3,            X5,            X6,            Y6,            X7,            Y7,            X8,            Y8,            Y4^X6^Y6,      Z1^X4^Y4,      Z0^X5^Y5,      X9,            0,             0,             0,             0,             }, //26
    {0,             X3,            Y3,            X5,            X6,            Y6,            X7,            Y7,            X8,            Y4^X6^Y6,      Z1^X4^Y4,      Z0^X5^Y5,      Y8,            0,             0,             0,             0,             }, //27
    {0,             Y2,            X3,            Y3,            X5,            X6,            Y6,            X7,            Y7,            Y4^X6^Y6,      Z1^X4^Y4,      Z0^X5^Y5,      X8,            0,             0,             0,             0,             }, //28
    {0,             X2,            Y2,            X3,            Y3,            X5,            X6,            Y6,            X7,            Y4^X6^Y6,      Z1^X4^Y4,      Z0^X5^Y5,      Y7,            0,             0,             0,             0,             }, //29
    {0,             X6,            Y6,            X7,            Y7,            X8,            Y8,            X9,            Y9,            Y4^X6^Y6,      Z1^X4^Y4,      Z0^X5^Y5,      X5^Y6,         0,             0,             0,             0,             }, //30
    {0,             Y3,            X6,            Y6,            X7,            Y7,            X8,            Y8,            X9,            Y4^X6^Y6,      Z1^X4^Y4,      Z0^X5^Y5,      X5^Y6,         0,             0,             0,             0,             }, //31
    {0,             X3,            Y3,            X6,            Y6,            X7,            Y7,            X8,            Y8,            Y4^X6^Y6,      Z1^X4^Y4,      Z0^X5^Y5,      X5^Y6,         0,             0,             0,             0,             }, //32
    {0,             Y2,            X3,            Y3,            X6,            Y6,            X7,            Y7,            X8,            Y4^X6^Y6,      Z1^X4^Y4,      Z0^X5^Y5,      X5^Y6,         0,             0,             0,             0,             }, //33
    {0,             X2,            Y2,            X3,            Y3,            X6,            Y6,            X7,            Y7,            Y4^X6^Y6,      Z1^X4^Y4,      Z0^X5^Y5,      X5^Y6,         0,             0,             0,             0,             }, //34
    {0,             X6,            Y6,            X7,            Y7,            X8,            Y8,            X9,            Y9,            Y4^X7^Y7,      Z1^X4^Y4,      Z0^Y5^X6,      X5^Y6,         0,             0,             0,             0,             }, //35
    {0,             Y3,            X6,            Y6,            X7,            Y7,            X8,            Y8,            X9,            Y4^X7^Y7,      Z1^X4^Y4,      Z0^Y5^X6,      X5^Y6,         0,             0,             0,             0,             }, //36
    {0,             X3,            Y3,            X6,            Y6,            X7,            Y7,            X8,            Y8,            Y4^X7^Y7,      Z1^X4^Y4,      Z0^Y5^X6,      X5^Y6,         0,             0,             0,             0,             }, //37
    {0,             Y2,            X3,            Y3,            X6,            Y6,            X7,            Y7,            X8,            Y4^X7^Y7,      Z2^X4^Y4,      Z1^Y5^X6,      X5^Y6,         0,             0,             0,             0,             }, //38
    {0,             X2,            Y2,            X3,            Y3,            X6,            Y6,            X7,            Y7,            Y4^X7^Y7,      Z2^X4^Y4,      Z1^Y5^X6,      X5^Y6,         0,             0,             0,             0,             }, //39
    {0,             Y2,            X3,            Y3,            X6,            Y6,            X7,            Y7,            X8,            Y4^X7^Y7,      Z2^X4^Y4,      Z1^Y5^X6,      Z0^X5^Y6,      0,             0,             0,             0,             }, //40
    {0,             X2,            Y2,            X3,            Y3,            X6,            Y6,            X7,            Y7,            Y4^X7^Y7,      Z2^X4^Y4,      Z1^Y5^X6,      Z0^X5^Y6,      0,             0,             0,             0,             }, //41
    {0,             Y6,            X7,            Y7,            X8,            Y8,            X9,            Y9,            X10,           Y4^X7^Y7,      Z1^X4^Y4,      Z0^Y5^X6,      X5^Y6,         X6^Y6,         0,             0,             0,             }, //42
    {0,             Y3,            Y6,            X7,            Y7,            X8,            Y8,            X9,            Y9,            Y4^X7^Y7,      Z1^X4^Y4,      Z0^Y5^X6,      X5^Y6,         X6^Y6,         0,             0,             0,             }, //43
    {0,             X3,            Y3,            Y6,            X7,            Y7,            X8,            Y8,            X9,            Y4^X7^Y7,      Z1^X4^Y4,      Z0^Y5^X6,      X5^Y6,         X6^Y6,         0,             0,             0,             }, //44
    {0,             Y2,            X3,            Y3,            Y6,            X7,            Y7,            X8,            Y8,            Y4^X7^Y7,      Z2^X4^Y4,      Z1^Y5^X6,      Z0^X5^Y6,      X6^Y6,         0,             0,             0,             }, //45
    {0,             X2,            Y2,            Y3,            X6,            Y6,            X7,            Y7,            X8,            Y4^X7^Y7,      Z2^X4^Y4,      Z1^Y5^X6,      Z0^X5^Y6,      X3^Y6,         0,             0,             0,             }, //46
    {0,             X6,            X7,            Y7,            X8,            Y8,            X9,            Y9,            X10,           Y4^X8^Y8,      Z1^X4^Y4,      Z0^Y5^X7,      X5^Y7,         X6^Y6,         0,             0,             0,             }, //47
    {0,             Y3,            X6,            X7,            Y7,            X8,            Y8,            X9,            Y9,            Y4^X8^Y8,      Z1^X4^Y4,      Z0^Y5^X7,      X5^Y7,         X6^Y6,         0,             0,             0,             }, //48
    {0,             X3,            Y3,            X6,            X7,            Y7,            X8,            Y8,            X9,            Y4^X8^Y8,      Z1^X4^Y4,      Z0^Y5^X7,      X5^Y7,         X6^Y6,         0,             0,             0,             }, //49
    {0,             Y2,            X3,            Y3,            X6,            X7,            Y7,            X8,            Y8,            Y4^X8^Y8,      Z3^X4^Y4,      Z2^Y5^X7,      Z1^X5^Y7,      X6^Y6,         0,             0,             0,             }, //50
    {0,             X2,            X3,            Y3,            X6,            X7,            Y7,            Y2,            X8,            Y4^X8^Y8,      Z2^X4^Y4,      Z1^Y5^X7,      Z0^X5^Y7,      X6^Y6,         0,             0,             0,             }, //51
    {0,             Y2,            X3,            Y3,            X6,            X7,            Y7,            X8,            Y8,            Y4^X8^Y8,      Z3^X4^Y4,      Z2^Y5^X7,      Z1^X5^Y7,      Z0^X6^Y6,      0,             0,             0,             }, //52
    {0,             X2,            X3,            Y3,            X6,            X7,            Y7,            Y2,            X8,            Y4^X8^Y8,      Z2^X4^Y4,      Z1^Y5^X7,      Z0^X5^Y7,      Y2^X6^Y6,      0,             0,             0,             }, //53
    {0,             X7,            Y7,            X8,            Y8,            X9,            Y9,            X10,           Y10,           Y4^X8^Y8,      Z1^X4^Y4,      Z0^Y5^X7,      X5^Y7,         X6^Y6,         X6^Y8,         0,             0,             }, //54
    {0,             Y3,            X7,            Y7,            X8,            Y8,            X9,            Y9,            X10,           Y4^X8^Y8,      Z1^X4^Y4,      Z0^Y5^X7,      X5^Y7,         X6^Y6,         X6^Y8,         0,             0,             }, //55
    {0,             X3,            Y3,            X7,            Y7,            X8,            Y8,            X9,            Y9,            Y4^X8^Y8,      Z1^X4^Y4,      Z0^Y5^X7,      X5^Y7,         X6^Y6,         X6^Y8,         0,             0,             }, //56
    {0,             Y2,            Y3,            X6,            X7,            Y7,            X8,            Y8,            X9,            Y4^X8^Y8,      Z3^X4^Y4,      Z2^Y5^X7,      Z1^X5^Y7,      Z0^X6^Y6,      X3^Y8,         0,             0,             }, //57
    {0,             X2,            Y3,            X6,            X7,            Y7,            X8,            Y2,            Y8,            Y4^X8^Y8,      Z2^X4^Y4,      Z1^Y5^X7,      Z0^X5^Y7,      Y2^X6^Y6,      X3^Y8,         0,             0,             }, //58
    {0,             X7,            Y7,            X8,            Y8,            X9,            Y9,            X10,           Y10,           Y4^X9^Y9,      Z1^X4^Y4,      Z0^Y5^X8,      X5^Y8,         Y6^X7,         X6^Y7,         0,             0,             }, //59
    {0,             Y3,            X7,            Y7,            X8,            Y8,            X9,            Y9,            X10,           Y4^X9^Y9,      Z1^X4^Y4,      Z0^Y5^X8,      X5^Y8,         Y6^X7,         X6^Y7,         0,             0,             }, //60
    {0,             X3,            Y3,            X7,            Y7,            X8,            Y8,            X9,            Y9,            Y4^X9^Y9,      Z1^X4^Y4,      Z0^Y5^X8,      X5^Y8,         Y6^X7,         X6^Y7,         0,             0,             }, //61
    {0,             X3,            Y3,            X7,            Y7,            X8,            Y8,            Y2,            X9,            Y4^X9^Y9,      Z3^X4^Y4,      Z2^Y5^X8,      Z1^X5^Y8,      Y2^Y6^X7,      X6^Y7,         0,             0,             }, //62
    {0,             X3,            Y3,            X7,            Y7,            X8,            Y8,            X2,            Y2,            Y4^X9^Y9,      Z2^X4^Y4,      Z1^Y5^X8,      Z0^X5^Y8,      Y2^Y6^X7,      X6^Y7,         0,             0,             }, //63
    {0,             X3,            Y3,            X7,            Y7,            X8,            Y8,            Y2,            X9,            Y4^X9^Y9,      Z3^X4^Y4,      Z2^Y5^X8,      Z1^X5^Y8,      Y2^Y6^X7,      Z0^X6^Y7,      0,             0,             }, //64
    {0,             X3,            Y3,            X7,            Y7,            X8,            Y8,            X2,            Y2,            Y4^X9^Y9,      Z2^X4^Y4,      Z1^Y5^X8,      Z0^X5^Y8,      Y2^Y6^X7,      X2^X6^Y7,      0,             0,             }, //65
    {0,             X2,            Y2,            X3,            Y3,            Y6,            X7,            Y7,            X8,            Y4^X7^Y7,      Z2^X4^Y4,      Z1^Y5^X6,      Z0^X5^Y6,      X6^Y6,         0,             0,             0,             }, //66
    {0,             X2,            Y2,            X3,            Y3,            X6,            X7,            Y7,            X8,            Y4^X8^Y8,      Z3^X4^Y4,      Z2^Y5^X7,      Z1^X5^Y7,      X6^Y6,         0,             0,             0,             }, //67
    {0,             X2,            Y2,            X3,            Y3,            X6,            X7,            Y7,            X8,            Y4^X8^Y8,      Z3^X4^Y4,      Z2^Y5^X7,      Z1^X5^Y7,      Z0^X6^Y6,      0,             0,             0,             }, //68
    {0,             Y2,            X3,            Y3,            X7,            Y7,            X8,            Y8,            X9,            Y4^X8^Y8,      Z3^X4^Y4,      Z2^Y5^X7,      Z1^X5^Y7,      Z0^X6^Y6,      X6^Y8,         0,             0,             }, //69
    {0,             X2,            Y2,            X3,            Y3,            X7,            Y7,            X8,            Y8,            Y4^X8^Y8,      Z3^X4^Y4,      Z2^Y5^X7,      Z1^X5^Y7,      Z0^X6^Y6,      X6^Y8,         0,             0,             }, //70
    {0,             Y2,            X3,            Y3,            X7,            Y7,            X8,            Y8,            X9,            Y4^X9^Y9,      X4^Y4^Z4,      Z3^Y5^X8,      Z2^X5^Y8,      Z1^Y6^X7,      X6^Y7,         0,             0,             }, //71
    {0,             X2,            Y2,            X3,            Y3,            X7,            Y7,            X8,            Y8,            Y4^X9^Y9,      X4^Y4^Z4,      Z3^Y5^X8,      Z2^X5^Y8,      Z1^Y6^X7,      X6^Y7,         0,             0,             }, //72
    {0,             Y2,            X3,            Y3,            X7,            Y7,            X8,            Y8,            X9,            Y4^X9^Y9,      X4^Y4^Z4,      Z3^Y5^X8,      Z2^X5^Y8,      Z1^Y6^X7,      Z0^X6^Y7,      0,             0,             }, //73
    {0,             X2,            Y2,            X3,            Y3,            X7,            Y7,            X8,            Y8,            Y4^X9^Y9,      X4^Y4^Z4,      Z3^Y5^X8,      Z2^X5^Y8,      Z1^Y6^X7,      Z0^X6^Y7,      0,             0,             }, //74
};

const UINT_64 GFX11_HTILE_SW_PATTERN[][18] =
{
    {0,             0,             0,             X3,            Y3,            X4,            Y4,            X5,            Y5,            X6,            Y6,            X7,            Y7,            0,             0,             0,             0,             0,             }, //0
    {0,             0,             0,             X3,            Y3,            Y4,            X5,            Y5,            X6,            Z0^X4^Y4,      Y6,            X7,            Y7,            0,             0,             0,             0,             0,             }, //1
    {0,             0,             0,             X3,            Y3,            X5,            Y5,            X6,            Y6,            Y4^X5^Y5,      Z0^X4^Y4,      X7,            Y7,            X8,            0,             0,             0,             0,             }, //2
    {0,             0,             0,             X3,            Y3,            Y5,            X6,            Y6,            X7,            Y4^X5^Y5,      Z0^X4^Y4,      X5^Y5,         Y7,            X8,            Y8,            0,             0,             0,             }, //3
    {0,             0,             0,             X3,            Y3,            X5,            X6,            Y6,            X7,            Y4^X6^Y6,      Z1^X4^Y4,      Y7,            X8,            Y8,            X5^Y5,         0,             0,             0,             }, //4
    {0,             0,             0,             X3,            Y3,            X5,            X6,            Y6,            X7,            Y4^X6^Y6,      Z1^X4^Y4,      Z0^X5^Y5,      Y7,            X8,            Y8,            0,             0,             0,             }, //5
    {0,             0,             0,             X3,            Y3,            X6,            Y6,            X7,            Y7,            Y4^X6^Y6,      Z1^X4^Y4,      Z0^X5^Y5,      X5^Y6,         X8,            Y8,            X9,            0,             0,             }, //6
    {0,             0,             0,             X3,            Y3,            Y4,            X5,            X6,            Y6,            Z1^X4^Y4,      Z0^X5^Y5,      X7,            Y7,            X8,            0,             0,             0,             0,             }, //7
    {0,             0,             0,             X3,            Y3,            X6,            Y6,            X7,            Y7,            Y4^X7^Y7,      Z1^X4^Y4,      Z0^Y5^X6,      X8,            Y8,            X9,            X5^Y6,         0,             0,             }, //8
    {0,             0,             0,             X3,            Y3,            X6,            Y6,            X7,            Y7,            Y4^X7^Y7,      Z1^X4^Y4,      Z0^Y5^X6,      X5^Y6,         X8,            Y8,            X9,            0,             0,             }, //9
    {0,             0,             0,             X3,            Y3,            Y6,            X7,            Y7,            X8,            Y4^X7^Y7,      Z1^X4^Y4,      Z0^Y5^X6,      X5^Y6,         X6^Y6,         Y8,            X9,            Y9,            0,             }, //10
    {0,             0,             0,             X3,            Y3,            Y4,            X6,            Y6,            X7,            Z1^X4^Y4,      Z0^Y5^X6,      X5^Y6,         Y7,            X8,            Y8,            0,             0,             0,             }, //11
    {0,             0,             0,             X3,            Y3,            X6,            X7,            Y7,            X8,            Y4^X8^Y8,      Z1^X4^Y4,      Z0^Y5^X7,      X5^Y7,         Y8,            X9,            Y9,            X6^Y6,         0,             }, //12
    {0,             0,             0,             X3,            Y3,            X6,            X7,            Y7,            X8,            Y4^X8^Y8,      Z1^X4^Y4,      Z0^Y5^X7,      X5^Y7,         X6^Y6,         Y8,            X9,            Y9,            0,             }, //13
    {0,             0,             0,             X3,            Y3,            X7,            Y7,            X8,            Y8,            Y4^X8^Y8,      Z1^X4^Y4,      Z0^Y5^X7,      X5^Y7,         X6^Y6,         X6^Y8,         X9,            Y9,            X10,           }, //14
    {0,             0,             0,             X3,            Y3,            Y4,            X6,            X7,            Y7,            Z1^X4^Y4,      Z0^Y5^X7,      X5^Y7,         X6^Y6,         X8,            Y8,            X9,            0,             0,             }, //15
    {0,             0,             0,             X3,            Y3,            X7,            Y7,            X8,            Y8,            Y4^X9^Y9,      Z1^X4^Y4,      Z0^Y5^X8,      X5^Y8,         Y6^X7,         X9,            Y9,            X10,           X6^Y7,         }, //16
    {0,             0,             0,             X3,            Y3,            X7,            Y7,            X8,            Y8,            Y4^X9^Y9,      Z1^X4^Y4,      Z0^Y5^X8,      X5^Y8,         Y6^X7,         X6^Y7,         X9,            Y9,            X10,           }, //17
};

}// V2
} // Addr

#endif
