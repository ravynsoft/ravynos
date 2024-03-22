#encoding=utf-8
#
# Copyright © 2017 Intel Corporation
#
# Permission is hereby granted, free of charge, to any person obtaining a
# copy of this software and associated documentation files (the "Software"),
# to deal in the Software without restriction, including without limitation
# the rights to use, copy, modify, merge, publish, distribute, sublicense,
# and/or sell copies of the Software, and to permit persons to whom the
# Software is furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice (including the next
# paragraph) shall be included in all copies or substantial portions of the
# Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
# THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
# FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
# IN THE SOFTWARE.
#

import sys
import zlib
import xml.etree.ElementTree as et

def main():
    if len(sys.argv) < 2:
        print("No input xml file specified")
        sys.exit(1)

    compress = zlib.compressobj()

    print("static const struct {")
    print("   uint32_t ver_10;")
    print("   uint32_t offset;")
    print("   uint32_t length;")
    print("} genxml_files_table[] = {")

    xml_offset = 0
    compressed_data = b''
    for i in range(1, len(sys.argv)):
        filename = sys.argv[i]
        xml = open(filename, "rb").read()
        xml_length = len(xml)
        root = et.fromstring(xml)

        print("   { %i, %i, %i }," %
              (int(float(root.attrib['gen']) * 10), xml_offset, xml_length))

        compressed_data += compress.compress(xml)
        xml_offset += xml_length

    print("};")

    compressed_data += compress.flush()

    print("")
    print("static const uint8_t compress_genxmls[] = {")
    print("   ", end='')
    for i, c in enumerate(bytearray(compressed_data), start=1):
        print("0x%.2x, " % c, end='\n   ' if not i % 12 else '')
    print('\n};')


if __name__ == '__main__':
    main()
