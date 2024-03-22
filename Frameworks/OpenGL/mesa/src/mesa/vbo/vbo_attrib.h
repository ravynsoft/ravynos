/*
 Copyright (C) Intel Corp.  2006.  All Rights Reserved.
 Intel funded Tungsten Graphics to
 develop this 3D driver.
 
 Permission is hereby granted, free of charge, to any person obtaining
 a copy of this software and associated documentation files (the
 "Software"), to deal in the Software without restriction, including
 without limitation the rights to use, copy, modify, merge, publish,
 distribute, sublicense, and/or sell copies of the Software, and to
 permit persons to whom the Software is furnished to do so, subject to
 the following conditions:
 
 The above copyright notice and this permission notice (including the
 next paragraph) shall be included in all copies or substantial
 portions of the Software.
 
 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 IN NO EVENT SHALL THE COPYRIGHT OWNER(S) AND/OR ITS SUPPLIERS BE
 LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
 OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
 WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 
 **********************************************************************/
 /*
  * Authors:
  *   Keith Whitwell <keithw@vmware.com>
  */

#ifndef VBO_ATTRIB_H
#define VBO_ATTRIB_H

/*
 * Note: The first 32 attributes match the VERT_ATTRIB_* definitions.
 * However, we have extra attributes for storing per-vertex glMaterial
 * values.  The material attributes get shifted into the generic positions
 * at draw time.
 *
 * One reason we can't alias materials and generics here is display lists.
 * A display list might contain both generic attributes and material
 * attributes which are selected at draw time depending on whether we're
 * using fixed function or a shader.  <sigh>
 */
enum vbo_attrib {
   VBO_ATTRIB_POS,
   VBO_ATTRIB_NORMAL,
   VBO_ATTRIB_COLOR0,
   VBO_ATTRIB_COLOR1,
   VBO_ATTRIB_FOG,
   VBO_ATTRIB_COLOR_INDEX,
   VBO_ATTRIB_TEX0,
   VBO_ATTRIB_TEX1,
   VBO_ATTRIB_TEX2,
   VBO_ATTRIB_TEX3,
   VBO_ATTRIB_TEX4,
   VBO_ATTRIB_TEX5,
   VBO_ATTRIB_TEX6,
   VBO_ATTRIB_TEX7,
   VBO_ATTRIB_POINT_SIZE,

   VBO_ATTRIB_GENERIC0, /* Not used? */
   VBO_ATTRIB_GENERIC1,
   VBO_ATTRIB_GENERIC2,
   VBO_ATTRIB_GENERIC3,
   VBO_ATTRIB_GENERIC4,
   VBO_ATTRIB_GENERIC5,
   VBO_ATTRIB_GENERIC6,
   VBO_ATTRIB_GENERIC7,
   VBO_ATTRIB_GENERIC8,
   VBO_ATTRIB_GENERIC9,
   VBO_ATTRIB_GENERIC10,
   VBO_ATTRIB_GENERIC11,
   VBO_ATTRIB_GENERIC12,
   VBO_ATTRIB_GENERIC13,
   VBO_ATTRIB_GENERIC14,
   VBO_ATTRIB_GENERIC15,
   VBO_ATTRIB_EDGEFLAG,

   /* XXX: in the vertex program inputs_read flag, we alias
    * materials and generics and use knowledge about the program
    * (whether it is a fixed-function emulation) to
    * differentiate.  Here we must keep them apart instead.
    */
   VBO_ATTRIB_MAT_FRONT_AMBIENT,
   VBO_ATTRIB_MAT_BACK_AMBIENT,
   VBO_ATTRIB_MAT_FRONT_DIFFUSE,
   VBO_ATTRIB_MAT_BACK_DIFFUSE,
   VBO_ATTRIB_MAT_FRONT_SPECULAR,
   VBO_ATTRIB_MAT_BACK_SPECULAR,
   VBO_ATTRIB_MAT_FRONT_EMISSION,
   VBO_ATTRIB_MAT_BACK_EMISSION,
   VBO_ATTRIB_MAT_FRONT_SHININESS,
   VBO_ATTRIB_MAT_BACK_SHININESS,
   VBO_ATTRIB_MAT_FRONT_INDEXES,
   VBO_ATTRIB_MAT_BACK_INDEXES,

   /* Offset into HW GL_SELECT result buffer. */
   VBO_ATTRIB_SELECT_RESULT_OFFSET,

   VBO_ATTRIB_MAX
};

#define VBO_ATTRIB_FIRST_MATERIAL VBO_ATTRIB_MAT_FRONT_AMBIENT
#define VBO_ATTRIB_LAST_MATERIAL VBO_ATTRIB_MAT_BACK_INDEXES


/** VBO_ATTRIB_POS .. VBO_ATTRIB_POINT_SIZE */
#define VBO_ATTRIBS_LEGACY  (BITFIELD64_MASK(VBO_ATTRIB_GENERIC0) | \
                             BITFIELD64_BIT(VBO_ATTRIB_EDGEFLAG))

/** VBO_ATTRIB_MAT_FRONT_AMBIENT .. VBO_ATTRIB_MAT_BACK_INDEXES */
#define VBO_ATTRIBS_MATERIALS BITFIELD64_RANGE(VBO_ATTRIB_MAT_FRONT_AMBIENT, \
                     VBO_ATTRIB_LAST_MATERIAL - VBO_ATTRIB_FIRST_MATERIAL + 1)

/**
 * Move material attribs to the last generic attribs, moving LAST_MATERIAL
 * to GENERIC15, etc.
 */
#define VBO_MATERIAL_SHIFT (VBO_ATTRIB_LAST_MATERIAL - VBO_ATTRIB_GENERIC15)



#define VBO_MAX_COPIED_VERTS 31

#endif
