/**************************************************************************
 * 
 * Copyright 2010 VMware, Inc.
 * All Rights Reserved.
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sub license, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 * 
 * The above copyright notice and this permission notice (including the
 * next paragraph) shall be included in all copies or substantial portions
 * of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS AND/OR ITS SUPPLIERS BE LIABLE FOR
 * ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 * 
 **************************************************************************/

/**
 * Implementation limits for LLVMpipe driver.
 */

#ifndef LP_LIMITS_H
#define LP_LIMITS_H


/**
 * Tile size (width and height). This needs to be a power of two.
 */
#define TILE_ORDER 6
#define TILE_SIZE (1 << TILE_ORDER)


/**
 * Max texture sizes
 */
/**
 * 2GB is the actual max currently (we always use 32bit offsets, and both
 * llvm GEP as well as avx2 gather use signed offsets).
 */
#define LP_MAX_TEXTURE_SIZE (2 * 1024 * 1024 * 1024ULL)
#define LP_MAX_TEXTURE_2D_LEVELS 15  /* 16K x 16K for now */
#define LP_MAX_TEXTURE_3D_LEVELS 12  /* 2K x 2K x 2K for now */
#define LP_MAX_TEXTURE_CUBE_LEVELS 15  /* 16K x 16K for now */
#define LP_MAX_TEXTURE_ARRAY_LAYERS 2048 /* 16K x 2048 / 16K x 16K x 2048 */


/** This must be the larger of LP_MAX_TEXTURE_2D/3D_LEVELS */
#define LP_MAX_TEXTURE_LEVELS LP_MAX_TEXTURE_2D_LEVELS


/**
 * Max drawing surface size is the max texture size
 */
#define LP_MAX_HEIGHT (1 << (LP_MAX_TEXTURE_LEVELS - 1))
#define LP_MAX_WIDTH  (1 << (LP_MAX_TEXTURE_LEVELS - 1))

#define LP_MAX_SAMPLES 4

#define LP_MAX_THREADS 32


/**
 * Max number of shader variants (for all shaders combined,
 * per context) that will be kept around.
 */
#define LP_MAX_SHADER_VARIANTS 1024

/**
 * Max number of instructions (for all fragment shaders combined per context)
 * that will be kept around (counted in terms of llvm ir).
 */
#define LP_MAX_SHADER_INSTRUCTIONS (2048 * LP_MAX_SHADER_VARIANTS)

/**
 * Max number of setup variants that will be kept around.
 *
 * These are determined by the combination of the fragment shader
 * input signature and a small amount of rasterization state (eg
 * flatshading).  It is likely that many active fragment shaders will
 * share the same setup variant.
 */
#define LP_MAX_SETUP_VARIANTS 64

/*
 * Max point size reported. Cap vertex shader point sizes to this.
 */
#define LP_MAX_POINT_WIDTH 255.0f
#endif /* LP_LIMITS_H */
