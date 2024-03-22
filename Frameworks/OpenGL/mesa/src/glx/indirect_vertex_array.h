/*
 * (C) Copyright IBM Corporation 2004, 2005
 * All Rights Reserved.
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sub license,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice (including the next
 * paragraph) shall be included in all copies or substantial portions of the
 * Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT.  IN NO EVENT SHALL
 * IBM,
 * AND/OR THEIR SUPPLIERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF
 * OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#ifndef INDIRECT_VERTEX_ARRAY_H
#define INDIRECT_VERTEX_ARRAY_H

extern const GLuint __glXTypeSize_table[16];

#define __glXTypeSize(e) ((((e) & ~0x0f) != 0x1400) \
    ? 0 : __glXTypeSize_table[ (e) & 0x0f ])

extern void __glXArrayDisableAll(__GLXattribute * state);

extern GLboolean __glXSetArrayEnable(__GLXattribute * state,
                                     GLenum key, unsigned index,
                                     GLboolean enable);

extern GLboolean __glXGetArrayEnable(const __GLXattribute * const state,
                                     GLenum key, unsigned index,
                                     GLintptr * dest);
extern GLboolean __glXGetArraySize(const __GLXattribute * const state,
                                   GLenum key, unsigned index,
                                   GLintptr * dest);
extern GLboolean __glXGetArrayType(const __GLXattribute * const state,
                                   GLenum key, unsigned index,
                                   GLintptr * dest);
extern GLboolean __glXGetArrayStride(const __GLXattribute * const state,
                                     GLenum key, unsigned index,
                                     GLintptr * dest);
extern GLboolean __glXGetArrayPointer(const __GLXattribute * const state,
                                      GLenum key, unsigned index,
                                      void **dest);
extern GLboolean __glXGetArrayNormalized(const __GLXattribute * const state,
                                         GLenum key, unsigned index,
                                         GLintptr * dest);

extern void __glXPushArrayState(__GLXattribute * state);
extern void __glXPopArrayState(__GLXattribute * state);

extern GLuint __glXGetActiveTextureUnit(const __GLXattribute * const state);

#endif /* INDIRECT_VERTEX_ARRAY_H */
