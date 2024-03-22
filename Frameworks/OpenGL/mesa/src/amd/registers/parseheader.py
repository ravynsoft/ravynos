#
# Copyright 2017-2019 Advanced Micro Devices, Inc.
#
# SPDX-License-Identifier: MIT
#
"""
Helper script that parses a register header and produces a register database
as output. Use as:

  python3 parseheader.py ADDRESS_SPACE < header.h

This script is included for reference -- we should be able to remove this in
the future.
"""

import json
import math
import re
import sys

from regdb import Object, RegisterDatabase, deduplicate_enums, deduplicate_register_types


RE_comment = re.compile(r'(/\*(.*)\*/)$|(//(.*))$')
RE_prefix = re.compile(r'([RSV])_([0-9a-fA-F]+)_')
RE_set_value = re.compile(r'\(\(\(unsigned\)\(x\) & ([0-9a-fA-Fx]+)\) << ([0-9]+)\)')
RE_set_value_no_shift = re.compile(r'\((\(unsigned\))?\(x\) & ([0-9a-fA-Fx]+)\)')

class HeaderParser(object):
    def __init__(self, address_space):
        self.regdb = RegisterDatabase()
        self.chips = ['gfx6', 'gfx7', 'gfx8', 'fiji', 'stoney', 'gfx9']
        self.address_space = address_space

    def __fini_field(self):
        if self.__field is None:
            return

        if self.__enumentries:
            self.__field.enum_ref = self.__regmap.name + '__' + self.__field.name
            self.regdb.add_enum(self.__field.enum_ref, Object(
                entries=self.__enumentries
            ))
        self.__fields.append(self.__field)

        self.__enumentries = None
        self.__field = None

    def __fini_register(self):
        if self.__regmap is None:
            return

        if self.__fields:
            self.regdb.add_register_type(self.__regmap.name, Object(
                fields=self.__fields
            ))
            self.__regmap.type_ref = self.__regmap.name
        self.regdb.add_register_mapping(self.__regmap)

        self.__regmap = None
        self.__fields = None

    def parse_header(self, filp):
        regdb = RegisterDatabase()
        chips = ['gfx6', 'gfx7', 'gfx8', 'fiji', 'stoney', 'gfx9']

        self.__regmap = None
        self.__fields = None
        self.__field = None
        self.__enumentries = None

        for line in filp:
            if not line.startswith('#define '):
                continue

            line = line[8:].strip()

            comment = None
            m = RE_comment.search(line)
            if m is not None:
                comment = m.group(2) or m.group(4)
                comment = comment.strip()
                line = line[:m.span()[0]].strip()

            split = line.split(None, 1)
            name = split[0]

            m = RE_prefix.match(name)
            if m is None:
                continue

            prefix = m.group(1)
            prefix_address = int(m.group(2), 16)
            name = name[m.span()[1]:]

            if prefix == 'V':
                value = int(split[1], 0)

                for entry in self.__enumentries:
                    if name == entry.name:
                        sys.exit('Duplicate value define: name = {0}'.format(name))

                entry = Object(name=name, value=value)
                if comment is not None:
                    entry.comment = comment
                self.__enumentries.append(entry)
                continue

            if prefix == 'S':
                self.__fini_field()

                if not name.endswith('(x)'):
                    sys.exit('Missing (x) in S line: {0}'.line)
                name = name[:-3]

                for field in self.__fields:
                    if name == field.name:
                        sys.exit('Duplicate field define: {0}'.format(name))

                m = RE_set_value.match(split[1])
                if m is not None:
                    unshifted_mask = int(m.group(1), 0)
                    shift = int(m.group(2), 0)
                else:
                    m = RE_set_value_no_shift.match(split[1])
                    if m is not None:
                        unshifted_mask = int(m.group(2), 0)
                        shift = 0
                    else:
                        sys.exit('Bad S_xxx_xxx define: {0}'.format(line))

                num_bits = int(math.log2(unshifted_mask + 1))
                if unshifted_mask != (1 << num_bits) - 1:
                    sys.exit('Bad unshifted mask in {0}'.format(line))

                self.__field = Object(
                    name=name,
                    bits=[shift, shift + num_bits - 1],
                )
                if comment is not None:
                    self.__field.comment = comment
                self.__enumentries = []

            if prefix == 'R':
                self.__fini_field()
                self.__fini_register()

                if regdb.register_mappings_by_name(name):
                    sys.exit('Duplicate register define: {0}'.format(name))

                address = int(split[1], 0)
                if address != prefix_address:
                    sys.exit('Inconsistent register address: {0}'.format(line))

                self.__regmap = Object(
                    name=name,
                    chips=self.chips,
                    map=Object(to=self.address_space, at=address),
                )
                self.__fields = []

        self.__fini_field()
        self.__fini_register()

def main():
    map_to = sys.argv[1]

    parser = HeaderParser(map_to)
    parser.parse_header(sys.stdin)

    deduplicate_enums(parser.regdb)
    deduplicate_register_types(parser.regdb)

    print(parser.regdb.encode_json_pretty())


if __name__ == '__main__':
    main()

# kate: space-indent on; indent-width 4; replace-tabs on;
