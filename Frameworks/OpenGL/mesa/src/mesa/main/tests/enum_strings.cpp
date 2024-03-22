/*
 * Copyright © 2012 Intel Corporation
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the next
 * paragraph) shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */

#include <gtest/gtest.h>

#include "main/enums.h"

struct enum_info {
   int value;
   const char *name;
};

extern const struct enum_info everything[];

TEST(EnumStrings, LookUpByNumber)
{
   for (unsigned i = 0; everything[i].name != NULL; i++) {
      EXPECT_STREQ(everything[i].name,
		   _mesa_enum_to_string(everything[i].value));
   }
}

TEST(EnumStrings, LookUpUnknownNumber)
{
   EXPECT_STRCASEEQ("0xEEEE", _mesa_enum_to_string(0xEEEE));
}

const struct enum_info everything[] = {
   /* A core enum, that should take precedence over _EXT and _OES. */
   { 0x0007, "GL_QUADS" },

   /* A core enum, that should take precedence over _EXT, _ARB, and _OES. */
   { 0x000a, "GL_LINES_ADJACENCY" },

   /* A core enum, that should take precedence over a _BIT. */
   { 0x0100, "GL_ACCUM" },

   /* An enum with "_BIT" that shouldn't get stripped out when we drop most
    * "*_BIT" enums.
    */
   { 0x0d55, "GL_ALPHA_BITS" },

   /* An EXT-only extension that we never expect to see show up in ARB/core.
    */
   { 0x8062, "GL_REPLACE_EXT" },

   /* An extension that made it from vendor to _EXT, but we never expect to
    * see go farther.
    */
   { 0x80a1, "GL_1PASS_EXT" },

   /* A vendor-only extension that we never expect to see show up in
    * EXT/ARB/core.
    */
   { 0x8503, "GL_COMBINE4_NV" },

   /* An extension that got promoted from _EXT to _ARB, but we don't expect to
    * see go any further.
    */
   { 0x850a, "GL_MODELVIEW1_ARB" },

   /* An EXT-only enum that should take precedence over a _BIT. */
   { 0x8000, "GL_ABGR_EXT" },

   /* An unusually-large enum */
   { 0x19262, "GL_RASTER_POSITION_UNCLIPPED_IBM" },

   /* Bitfields like GL_SCISSOR_BIT and GL_ALL_ATTRIB_BITS should not appear
    * in the table.
    */
   { 0x00080000, "0x80000" },
   { 0x000fffff, "0xfffff" },
   { (int)0xffffffff, "0xffffffff" },

   { 0, NULL }
};
