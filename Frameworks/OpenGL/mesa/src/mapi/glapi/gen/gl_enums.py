
# (C) Copyright Zack Rusin 2005. All Rights Reserved.
# Copyright (C) 2015 Intel Corporation
# Copyright (C) 2015 Broadcom Corporation
#
# Permission is hereby granted, free of charge, to any person obtaining a
# copy of this software and associated documentation files (the "Software"),
# to deal in the Software without restriction, including without limitation
# on the rights to use, copy, modify, merge, publish, distribute, sub
# license, and/or sell copies of the Software, and to permit persons to whom
# the Software is furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice (including the next
# paragraph) shall be included in all copies or substantial portions of the
# Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT.  IN NO EVENT SHALL
# IBM AND/OR ITS SUPPLIERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
# FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
# IN THE SOFTWARE.
#
# Authors:
#    Zack Rusin <zack@kde.org>

import argparse

import license
import gl_XML
import xml.etree.ElementTree as ET
import sys, getopt
import re

class PrintGlEnums(gl_XML.gl_print_base):

    def __init__(self):
        gl_XML.gl_print_base.__init__(self)

        self.name = "gl_enums.py (from Mesa)"
        self.license = license.bsd_license_template % ( \
"""Copyright (C) 1999-2005 Brian Paul All Rights Reserved.""", "BRIAN PAUL")
        # Mapping from enum value to (name, priority) tuples.
        self.enum_table = {}
        # Mapping from enum name to value
        self.string_to_int = {}


    def printRealHeader(self):
        print('#include <stdio.h>')
        print('#include "util/glheader.h"')
        print('#include "main/enums.h"')
        print('#include "main/mtypes.h"')
        print('')
        print('typedef struct PACKED {')
        print('   uint32_t offset;')
        print('   int n;')
        print('} enum_elt;')
        print('')
        return

    def print_code(self):
        print("""
typedef int (*cfunc)(const void *, const void *);

/**
 * Compare a key enum value to an element in the \c enum_string_table_offsets array.
 *
 * \c bsearch always passes the key as the first parameter and the pointer
 * to the array element as the second parameter.  We can elimiate some
 * extra work by taking advantage of that fact.
 *
 * \param a  Pointer to the desired enum name.
 * \param b  Pointer into the \c enum_string_table_offsets array.
 */
static int compar_nr( const int *a, enum_elt *b )
{
   return a[0] - b->n;
}


static char token_tmp[20];

/**
 * This function always returns a string. If the number is a valid enum, it
 * returns the enum name. Otherwise, it returns a numeric string.
 */
const char *
_mesa_enum_to_string(int nr)
{
   enum_elt *elt;

   elt = bsearch(& nr, enum_string_table_offsets,
                 ARRAY_SIZE(enum_string_table_offsets),
                 sizeof(enum_string_table_offsets[0]),
                 (cfunc) compar_nr);

   if (elt != NULL) {
      return &enum_string_table[elt->offset];
   }
   else {
      /* this is not re-entrant safe, no big deal here */
      snprintf(token_tmp, sizeof(token_tmp) - 1, "0x%x", nr);
      token_tmp[sizeof(token_tmp) - 1] = '\\0';
      return token_tmp;
   }
}

/**
 * Primitive names
 */
static const char *prim_names[PRIM_MAX+3] = {
   "GL_POINTS",
   "GL_LINES",
   "GL_LINE_LOOP",
   "GL_LINE_STRIP",
   "GL_TRIANGLES",
   "GL_TRIANGLE_STRIP",
   "GL_TRIANGLE_FAN",
   "GL_QUADS",
   "GL_QUAD_STRIP",
   "GL_POLYGON",
   "GL_LINES_ADJACENCY",
   "GL_LINE_STRIP_ADJACENCY",
   "GL_TRIANGLES_ADJACENCY",
   "GL_TRIANGLE_STRIP_ADJACENCY",
   "GL_PATCHES",
   "outside begin/end",
   "unknown state"
};


/* Get the name of an enum given that it is a primitive type.  Avoids
 * GL_FALSE/GL_POINTS ambiguity and others.
 */
const char *
_mesa_lookup_prim_by_nr(GLuint nr)
{
   if (nr < ARRAY_SIZE(prim_names))
      return prim_names[nr];
   else
      return "invalid mode";
}


""")
        return


    def printBody(self, xml):
        self.process_enums(xml)

        sorted_enum_values = sorted(self.enum_table.keys())
        string_offsets = {}
        i = 0;
        print('#if defined(__GNUC__)')
        print('# define LONGSTRING __extension__')
        print('#else')
        print('# define LONGSTRING')
        print('#endif')
        print('')
        print('LONGSTRING static const char enum_string_table[] = {')
        # We express the very long concatenation of enum strings as an array
        # of characters rather than as a string literal to work-around MSVC's
        # 65535 character limit.
        for enum in sorted_enum_values:
            (name, pri) = self.enum_table[enum]
            print("  ", end=' ')
            for ch in name:
                print("'%c'," % ch, end=' ')
            print("'\\0',")

            string_offsets[ enum ] = i
            i += len(name) + 1

        print('};')
        print('')


        print('static const enum_elt enum_string_table_offsets[%u] =' % (len(self.enum_table)))
        print('{')
        for enum in sorted_enum_values:
            (name, pri) = self.enum_table[enum]
            print('   { %5u, 0x%08X }, /* %s */' % (string_offsets[enum], enum, name))
        print('};')
        print('')

        self.print_code()
        return

    def add_enum_provider(self, name, priority):
        value = self.string_to_int[name]

        # We don't want the weird GL_SKIP_COMPONENTS1_NV enums.
        if value < 0:
            return
        # We don't want the 64-bit GL_TIMEOUT_IGNORED "enums"
        if value > 0xffffffff:
            return

        # We don't want bitfields in the enum-to-string table --
        # individual bits have so many names, it's pointless.  Note
        # that we check for power-of-two, since some getters have
        # "_BITS" in their name, but none have a power-of-two enum
        # number.
        if not (value & (value - 1)) and '_BIT' in name:
            return

        # Also drop the GL_*_ATTRIB_BITS bitmasks.
        if value == 0xffffffff:
                return

        if value in self.enum_table:
            (n, p) = self.enum_table[value]
            if priority < p:
                self.enum_table[value] = (name, priority)
        else:
            self.enum_table[value] = (name, priority)

    def process_extension(self, extension):
        if extension.get('name').startswith('GL_ARB_'):
            extension_prio = 400
        elif extension.get('name').startswith('GL_EXT_'):
            extension_prio = 600
        else:
            extension_prio = 800

        for enum in extension.findall('require/enum'):
            self.add_enum_provider(enum.get('name'), extension_prio)

    def process_enums(self, xml):
        # First, process the XML entries that define the hex values
        # for all of the enum names.
        for enum in xml.findall('enums/enum'):
            name = enum.get('name')
            value = int(enum.get('value'), base=16)

            # If the same name ever maps to multiple values, that can
            # confuse us.  GL_ACTIVE_PROGRAM_EXT is OK to lose because
            # we choose GL_ACTIVE PROGRAM instead.
            if name in self.string_to_int and name != "GL_ACTIVE_PROGRAM_EXT":
                print("#error Renumbering {0} from {1} to {2}".format(name, self.string_to_int[name], value))

            self.string_to_int[name] = value

        # Now, process all of the API versions and extensions that
        # provide enums, so we can decide what name to call any hex
        # value.
        for feature in xml.findall('feature'):
            feature_name = feature.get('name')

            # When an enum gets renamed in a newer version (generally
            # because of some generalization of the functionality),
            # prefer the newer name.  Also, prefer desktop GL names to
            # ES.
            m = re.match('GL_VERSION_([0-9])_([0-9])', feature_name)
            if m:
                feature_prio = 100 - int(m.group(1) + m.group(2))
            else:
                m = re.match('GL_ES_VERSION_([0-9])_([0-9])', feature_name)
                if m:
                    feature_prio = 200 - int(m.group(1) + m.group(2))
                else:
                    feature_prio = 200

            for enum in feature.findall('require/enum'):
                self.add_enum_provider(enum.get('name'), feature_prio)

        for extension in xml.findall('extensions/extension'):
            self.process_extension(extension)


def _parser():
    parser = argparse.ArgumentParser()
    parser.add_argument('-f', '--input_file',
                        required=True,
                        help="Choose an xml file to parse.")
    return parser.parse_args()


def main():
    args = _parser()
    xml = ET.parse(args.input_file)

    printer = PrintGlEnums()
    printer.Print(xml)


if __name__ == '__main__':
    main()
