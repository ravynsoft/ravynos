/*
************************************************************************************************************************
*
*  Copyright (C) 2007-2022 Advanced Micro Devices, Inc.  All rights reserved.
*  SPDX-License-Identifier: MIT
*
***********************************************************************************************************************/

#if !defined (__GFX11_GB_REG_H__)
#define __GFX11_GB_REG_H__

/*
*    gfx11_gb_reg.h
*
*    Register Spec Release:  1.0
*
*/

//
// Make sure the necessary endian defines are there.
//
#if defined(LITTLEENDIAN_CPU)
#elif defined(BIGENDIAN_CPU)
#else
#error "BIGENDIAN_CPU or LITTLEENDIAN_CPU must be defined"
#endif

union GB_ADDR_CONFIG_GFX11
{
    struct
    {
#if defined(LITTLEENDIAN_CPU)
                unsigned int NUM_PIPES            :  3;
                unsigned int PIPE_INTERLEAVE_SIZE :  3;
                unsigned int MAX_COMPRESSED_FRAGS :  2;
                unsigned int NUM_PKRS             :  3;
                unsigned int                      :  8;
                unsigned int NUM_SHADER_ENGINES   :  2;
                unsigned int                      :  5;
                unsigned int NUM_RB_PER_SE        :  2;
                unsigned int                      :  4;
#elif defined(BIGENDIAN_CPU)
                unsigned int                      :  4;
                unsigned int NUM_RB_PER_SE        :  2;
                unsigned int                      :  5;
                unsigned int NUM_SHADER_ENGINES   :  2;
                unsigned int                      :  8;
                unsigned int NUM_PKRS             :  3;
                unsigned int MAX_COMPRESSED_FRAGS :  2;
                unsigned int PIPE_INTERLEAVE_SIZE :  3;
                unsigned int NUM_PIPES            :  3;
#endif
    } bitfields, bits;
    unsigned int    u32All;
    int             i32All;
    float           f32All;
};

#endif

