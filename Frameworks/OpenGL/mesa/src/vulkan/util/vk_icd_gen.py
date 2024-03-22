# Copyright 2017 Intel Corporation
#
# Permission is hereby granted, free of charge, to any person obtaining a
# copy of this software and associated documentation files (the
# "Software"), to deal in the Software without restriction, including
# without limitation the rights to use, copy, modify, merge, publish,
# distribute, sub license, and/or sell copies of the Software, and to
# permit persons to whom the Software is furnished to do so, subject to
# the following conditions:
#
# The above copyright notice and this permission notice (including the
# next paragraph) shall be included in all copies or substantial portions
# of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
# OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
# MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT.
# IN NO EVENT SHALL VMWARE AND/OR ITS SUPPLIERS BE LIABLE FOR
# ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
# TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
# SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

import argparse
import json
import os.path
import re
import xml.etree.ElementTree as et

def get_xml_patch_version(xml_file):
    xml = et.parse(xml_file)
    for d in xml.findall('.types/type'):
        if d.get('category', None) != 'define':
            continue

        name = d.find('.name')
        if name.text != 'VK_HEADER_VERSION':
            continue;

        return name.tail.strip()

if __name__ == '__main__':
    parser = argparse.ArgumentParser()
    parser.add_argument('--api-version', required=True,
                        help='Vulkan API version.')
    parser.add_argument('--xml', required=False,
                        help='Vulkan registry XML for patch version')
    parser.add_argument('--lib-path', required=True,
                        help='Path to installed library')
    parser.add_argument('--out', required=False,
                        help='Output json file.')
    parser.add_argument('--use-backslash', action='store_true',
                        help='Use backslash (Windows).')
    args = parser.parse_args()

    version = args.api_version
    if args.xml:
        re.match(r'\d+\.\d+', version)
        version = version + '.' + get_xml_patch_version(args.xml)
    else:
        re.match(r'\d+\.\d+\.\d+', version)

    lib_path = args.lib_path
    if args.use_backslash:
        lib_path = lib_path.replace('/', '\\')

    json_data = {
        'file_format_version': '1.0.0',
        'ICD': {
            'library_path': lib_path,
            'api_version': version,
        },
    }

    json_params = {
        'indent': 4,
        'sort_keys': True,
        'separators': (',', ': '),
    }

    if args.out:
        with open(args.out, 'w', encoding='utf-8') as f:
            json.dump(json_data, f, **json_params)
    else:
        print(json.dumps(json_data, **json_params))
