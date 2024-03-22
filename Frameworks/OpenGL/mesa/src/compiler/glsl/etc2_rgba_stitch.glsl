/*
 * Copyright 2020-2022 Matias N. Goldberg
 * Copyright 2022 Intel Corporation
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */

// RGB and Alpha components of ETC2 RGBA are computed separately.
// This compute shader merely stitches them together to form the final result
// It's also used by RG11 driver to stitch two R11 into one RG11

#version 310 es

%s // include "CrossPlatformSettings_piece_all.glsl"

layout( local_size_x = 8,  //
		local_size_y = 8,  //
		local_size_z = 1 ) in;

layout( binding = 0 ) uniform highp usampler2D srcRGB;
layout( binding = 1 ) uniform highp usampler2D srcAlpha;
layout( rgba32ui ) uniform restrict writeonly highp uimage2D dstTexture;

void main()
{
	uint2 etcRgb = OGRE_Load2D( srcRGB, int2( gl_GlobalInvocationID.xy ), 0 ).xy;
	uint2 etcAlpha = OGRE_Load2D( srcAlpha, int2( gl_GlobalInvocationID.xy ), 0 ).xy;

	imageStore( dstTexture, int2( gl_GlobalInvocationID.xy ), uint4( etcAlpha.xy, etcRgb.xy ) );
}
