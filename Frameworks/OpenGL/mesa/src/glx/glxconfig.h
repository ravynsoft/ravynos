/*
 * (C) Copyright IBM Corporation 2003
 * All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * on the rights to use, copy, modify, merge, publish, distribute, sub
 * license, and/or sell copies of the Software, and to permit persons to whom
 * the Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the next
 * paragraph) shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT.  IN NO EVENT SHALL
 * VA LINUX SYSTEM, IBM AND/OR THEIR SUPPLIERS BE LIABLE FOR ANY CLAIM,
 * DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
 * OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE
 * USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

/**
 * \file glcontextmodes.h
 * \author Ian Romanick <idr@us.ibm.com>
 */

#ifndef GLCONTEXTMODES_H
#define GLCONTEXTMODES_H

struct glx_config {
    struct glx_config * next;

    GLuint doubleBufferMode;
    GLuint stereoMode;

    GLint redBits, greenBits, blueBits, alphaBits;	/* bits per comp */
    GLuint redMask, greenMask, blueMask, alphaMask;
    GLuint redShift, greenShift, blueShift, alphaShift;
    GLint rgbBits;		/* total bits for rgb */
    GLint indexBits;		/* total bits for colorindex */

    GLint accumRedBits, accumGreenBits, accumBlueBits, accumAlphaBits;
    GLint depthBits;
    GLint stencilBits;

    GLint numAuxBuffers;

    GLint level;

    /* GLX */
    GLint visualID;
    GLint visualType;     /**< One of the GLX X visual types. (i.e., 
			   * \c GLX_TRUE_COLOR, etc.)
			   */

    /* EXT_visual_rating / GLX 1.2 */
    GLint visualRating;

    /* EXT_visual_info / GLX 1.2 */
    GLint transparentPixel;
				/*    colors are floats scaled to ints */
    GLint transparentRed, transparentGreen, transparentBlue, transparentAlpha;
    GLint transparentIndex;

    /* ARB_multisample / SGIS_multisample */
    GLint sampleBuffers;
    GLint samples;

    /* SGIX_fbconfig / GLX 1.3 */
    GLint drawableType;
    GLint renderType;
    GLint xRenderable;
    GLint fbconfigID;

    /* SGIX_pbuffer / GLX 1.3 */
    GLint maxPbufferWidth;
    GLint maxPbufferHeight;
    GLint maxPbufferPixels;
    GLint optimalPbufferWidth;   /* Only for SGIX_pbuffer. */
    GLint optimalPbufferHeight;  /* Only for SGIX_pbuffer. */

    /* SGIX_visual_select_group */
    GLint visualSelectGroup;

    GLint screen;

    /* EXT_texture_from_pixmap */
    GLint bindToTextureRgb;
    GLint bindToTextureRgba;
    GLint bindToMipmapTexture;
    GLint bindToTextureTargets;
    GLint yInverted;

    /* EXT_framebuffer_sRGB */
    GLint sRGBCapable;

    /* NV_float_buffer */
    GLint floatComponentsNV;
};

extern GLint _gl_convert_from_x_visual_type(int visualType);

extern int
glx_config_get(struct glx_config * mode, int attribute, int *value_return);
extern struct glx_config *
glx_config_create_list(unsigned count);
extern void
glx_config_destroy_list(struct glx_config *configs);
extern struct glx_config *
glx_config_find_visual(struct glx_config *configs, int vid);
extern struct glx_config *
glx_config_find_fbconfig(struct glx_config *configs, int fbid);

#endif /* GLCONTEXTMODES_H */

