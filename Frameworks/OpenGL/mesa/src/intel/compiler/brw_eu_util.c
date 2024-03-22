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


#include "brw_eu_defines.h"
#include "brw_eu.h"


void brw_math_invert( struct brw_codegen *p,
			     struct brw_reg dst,
			     struct brw_reg src)
{
   gfx4_math(p,
	     dst,
	     BRW_MATH_FUNCTION_INV,
	     0,
	     src,
	     BRW_MATH_PRECISION_FULL);
}



void brw_copy4(struct brw_codegen *p,
	       struct brw_reg dst,
	       struct brw_reg src,
	       unsigned count)
{
   unsigned i;

   dst = vec4(dst);
   src = vec4(src);

   for (i = 0; i < count; i++)
   {
      unsigned delta = i*32;
      brw_MOV(p, byte_offset(dst, delta),    byte_offset(src, delta));
      brw_MOV(p, byte_offset(dst, delta+16), byte_offset(src, delta+16));
   }
}


void brw_copy8(struct brw_codegen *p,
	       struct brw_reg dst,
	       struct brw_reg src,
	       unsigned count)
{
   unsigned i;

   dst = vec8(dst);
   src = vec8(src);

   for (i = 0; i < count; i++)
   {
      unsigned delta = i*32;
      brw_MOV(p, byte_offset(dst, delta),    byte_offset(src, delta));
   }
}


void brw_copy_indirect_to_indirect(struct brw_codegen *p,
				   struct brw_indirect dst_ptr,
				   struct brw_indirect src_ptr,
				   unsigned count)
{
   unsigned i;

   for (i = 0; i < count; i++)
   {
      unsigned delta = i*32;
      brw_MOV(p, deref_4f(dst_ptr, delta),    deref_4f(src_ptr, delta));
      brw_MOV(p, deref_4f(dst_ptr, delta+16), deref_4f(src_ptr, delta+16));
   }
}


void brw_copy_from_indirect(struct brw_codegen *p,
			    struct brw_reg dst,
			    struct brw_indirect ptr,
			    unsigned count)
{
   unsigned i;

   dst = vec4(dst);

   for (i = 0; i < count; i++)
   {
      unsigned delta = i*32;
      brw_MOV(p, byte_offset(dst, delta),    deref_4f(ptr, delta));
      brw_MOV(p, byte_offset(dst, delta+16), deref_4f(ptr, delta+16));
   }
}
