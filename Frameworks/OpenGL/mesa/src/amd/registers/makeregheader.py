COPYRIGHT = '''
/*
 * Copyright 2015-2019 Advanced Micro Devices, Inc.
 *
 * SPDX-License-Identifier: MIT
 */
'''
"""
Create the (combined) register header from register JSON. Use --help for usage.
"""

import argparse
from collections import defaultdict
import itertools
import json
import re
import sys

from regdb import Object, RegisterDatabase, deduplicate_enums, deduplicate_register_types


######### BEGIN HARDCODED CONFIGURATION

# Chips are sorted chronologically
CHIPS = [
    Object(name='gfx6', disambiguation='GFX6'),
    Object(name='gfx7', disambiguation='GFX7'),
    Object(name='gfx8', disambiguation='GFX8'),
    Object(name='gfx81', disambiguation='GFX81'),
    Object(name='gfx9', disambiguation='GFX9'),
    Object(name='gfx940', disambiguation='GFX940'),
    Object(name='gfx10', disambiguation='GFX10'),
    Object(name='gfx103', disambiguation='GFX103'),
    Object(name='gfx11', disambiguation='GFX11'),
    Object(name='gfx115', disambiguation='GFX115'),
]

######### END HARDCODED CONFIGURATION

def get_chip_index(chip):
    """
    Given a chip name, return its index in the global CHIPS list.
    """
    return next(idx for idx, obj in enumerate(CHIPS) if obj.name == chip)

def get_disambiguation_suffix(chips):
    """
    Disambiguation suffix to be used for an enum entry or field name that
    is supported in the given set of chips.
    """
    oldest_chip_index = min([get_chip_index(chip) for chip in chips])
    return CHIPS[oldest_chip_index].disambiguation

def get_chips_comment(chips, parent=None):
    """
    Generate a user-friendly comment describing the given set of chips.

    The return value may be None, if such a comment is deemed unnecessary.

    parent is an optional set of chips supporting a parent structure, e.g.
    where chips may be the set of chips supporting a specific enum value,
    parent would be the set of chips supporting the field containing the enum,
    the idea being that no comment is necessary if all chips that support the
    parent also support the child.
    """
    chipflags = [chip.name in chips for chip in CHIPS]
    if all(chipflags):
        return None

    if parent is not None:
        parentflags = [chip.name in parent for chip in CHIPS]
        if all(childflag or not parentflag for childflag, parentflag in zip(chipflags, parentflags)):
            return None

    prefix = 0
    for idx, chip, flag in zip(itertools.count(), CHIPS, chipflags):
        if not flag:
            break
        prefix = idx + 1

    suffix = len(CHIPS)
    for idx, chip, flag in zip(itertools.count(), reversed(CHIPS), reversed(chipflags)):
        if not flag:
            break
        suffix = len(CHIPS) - idx - 1

    comment = []
    if prefix > 0:
        comment.append('<= {0}'.format(CHIPS[prefix - 1].name))
    for chip, flag in zip(CHIPS[prefix:suffix], chipflags[prefix:suffix]):
        if flag:
            comment.append(chip.name)
    if suffix < len(CHIPS):
        comment.append('>= {0}'.format(CHIPS[suffix].name))

    return ', '.join(comment)

def detect_conflict(regdb, field_in_type1, field_in_type2):
    """
    Returns False if field_in_type1 and field_in_type2 can be merged
    into a single field = if writing to field_in_type1 bits won't
    overwrite adjacent fields in type2, and the other way around.
    """
    for idx, type_refs in enumerate([field_in_type1.type_refs, field_in_type2.type_refs]):
        ref = field_in_type2 if idx == 0 else field_in_type1
        for type_ref in type_refs:
            for field in regdb.register_type(type_ref).fields:
                # If a different field in the other type starts in
                # the tested field's bits[0, 1] interval
                if (field.bits[0] > ref.bits[0] and
                    field.bits[0] <= ref.bits[1]):
                    return True

    return False

class HeaderWriter(object):
    def __init__(self, regdb, guard=None):
        self.guard = guard

        # The following contain: Object(address, chips, name, regmap/field/enumentry)
        self.register_lines = []
        self.field_lines = []
        self.value_lines = []

        regtype_emit = defaultdict(set)
        enum_emit = defaultdict(set)

        for regmap in regdb.register_mappings():
            type_ref = getattr(regmap, 'type_ref', None)
            self.register_lines.append(Object(
                address=regmap.map.at,
                chips=set(regmap.chips),
                name=regmap.name,
                regmap=regmap,
                type_refs=set([type_ref]) if type_ref else set(),
            ))

            basename = re.sub(r'[0-9]+', '', regmap.name)
            key = '{type_ref}::{basename}'.format(**locals())
            if type_ref is not None and regtype_emit[key].isdisjoint(regmap.chips):
                regtype_emit[key].update(regmap.chips)

                regtype = regdb.register_type(type_ref)
                for field in regtype.fields:
                    if field.name == 'RESERVED':
                        continue

                    enum_ref = getattr(field, 'enum_ref', None)
                    self.field_lines.append(Object(
                        address=regmap.map.at,
                        chips=set(regmap.chips),
                        name=field.name,
                        field=field,
                        bits=field.bits[:],
                        type_refs=set([type_ref]) if type_ref else set(),
                        enum_refs=set([enum_ref]) if enum_ref else set(),
                    ))

                    key = '{type_ref}::{basename}::{enum_ref}'.format(**locals())
                    if enum_ref is not None and enum_emit[key].isdisjoint(regmap.chips):
                        enum_emit[key].update(regmap.chips)

                        enum = regdb.enum(enum_ref)
                        for entry in enum.entries:
                            self.value_lines.append(Object(
                                address=regmap.map.at,
                                chips=set(regmap.chips),
                                name=entry.name,
                                enumentry=entry,
                                enum_refs=set([enum_ref]) if enum_ref else set(),
                            ))

        # Merge register lines
        lines = self.register_lines
        lines.sort(key=lambda line: (line.address, line.name))

        self.register_lines = []
        for line in lines:
            prev = self.register_lines[-1] if self.register_lines else None
            if prev and prev.address == line.address and prev.name == line.name:
                prev.chips.update(line.chips)
                prev.type_refs.update(line.type_refs)
                continue
            self.register_lines.append(line)

        # Merge field lines
        lines = self.field_lines
        lines.sort(key=lambda line: (line.address, line.name))

        self.field_lines = []
        for line in lines:
            merged = False
            for prev in reversed(self.field_lines):
                if prev.address != line.address or prev.name != line.name:
                    break

                # Can merge fields if they have the same starting bit and the
                # range of the field as intended by the current line does not
                # conflict with any of the regtypes covered by prev.
                if prev.bits[0] != line.bits[0]:
                    continue

                if prev.bits[1] != line.bits[1]:
                    # Current line's field extends beyond the range of prev.
                    # Need to check for conflicts
                    if detect_conflict(regdb, prev, line):
                        continue

                prev.bits[1] = max(prev.bits[1], line.bits[1])
                prev.chips.update(line.chips)
                prev.type_refs.update(line.type_refs)
                prev.enum_refs.update(line.enum_refs)
                merged = True
                break
            if not merged:
                self.field_lines.append(line)

        # Merge value lines
        lines = self.value_lines
        lines.sort(key=lambda line: (line.address, line.name))

        self.value_lines = []
        for line in lines:
            for prev in reversed(self.value_lines):
                if prev.address == line.address and prev.name == line.name and\
                   prev.enumentry.value == line.enumentry.value:
                    prev.chips.update(line.chips)
                    prev.enum_refs.update(line.enum_refs)
                    break
            else:
                self.value_lines.append(line)

        # Disambiguate field and value lines
        for idx, line in enumerate(self.field_lines):
            prev = self.field_lines[idx - 1] if idx > 0 else None
            next = self.field_lines[idx + 1] if idx + 1 < len(self.field_lines) else None
            if (prev and prev.address == line.address and prev.field.name == line.field.name) or\
               (next and next.address == line.address and next.field.name == line.field.name):
                line.name += '_' + get_disambiguation_suffix(line.chips)

        for idx, line in enumerate(self.value_lines):
            prev = self.value_lines[idx - 1] if idx > 0 else None
            next = self.value_lines[idx + 1] if idx + 1 < len(self.value_lines) else None
            if (prev and prev.address == line.address and prev.enumentry.name == line.enumentry.name) or\
               (next and next.address == line.address and next.enumentry.name == line.enumentry.name):
                line.name += '_' + get_disambiguation_suffix(line.chips)

    def print(self, filp, sort='address'):
        """
        Print out the entire register header.
        """
        if sort == 'address':
            self.register_lines.sort(key=lambda line: (line.address, line.name))
        else:
            assert sort == 'name'
            self.register_lines.sort(key=lambda line: (line.name, line.address))

        # Collect and sort field lines by address
        field_lines_by_address = defaultdict(list)
        for line in self.field_lines:
            field_lines_by_address[line.address].append(line)
        for field_lines in field_lines_by_address.values():
            if sort == 'address':
                field_lines.sort(key=lambda line: (line.bits[0], line.name))
            else:
                field_lines.sort(key=lambda line: (line.name, line.bits[0]))

        # Collect and sort value lines by address
        value_lines_by_address = defaultdict(list)
        for line in self.value_lines:
            value_lines_by_address[line.address].append(line)
        for value_lines in value_lines_by_address.values():
            if sort == 'address':
                value_lines.sort(key=lambda line: (line.enumentry.value, line.name))
            else:
                value_lines.sort(key=lambda line: (line.name, line.enumentry.value))

        print('/* Automatically generated by amd/registers/makeregheader.py */\n', file=filp)
        print(file=filp)
        print(COPYRIGHT.strip(), file=filp)
        print(file=filp)

        if self.guard:
            print('#ifndef {self.guard}'.format(**locals()), file=filp)
            print('#define {self.guard}\n'.format(**locals()), file=filp)

        for register_line in self.register_lines:
            comment = get_chips_comment(register_line.chips)

            address = '{0:X}'.format(register_line.address)
            address = address.rjust(3 if register_line.regmap.map.to == 'pkt3' else 6, '0')

            define_name = 'R_{address}_{register_line.name}'.format(**locals()).ljust(63)
            comment = ' /* {0} */'.format(comment) if comment else ''
            print('#define {define_name} 0x{address}{comment}'.format(**locals()), file=filp)

            field_lines = field_lines_by_address[register_line.address]
            field_idx = 0
            while field_idx < len(field_lines):
                field_line = field_lines[field_idx]

                if field_line.type_refs.isdisjoint(register_line.type_refs):
                    field_idx += 1
                    continue
                del field_lines[field_idx]

                comment = get_chips_comment(field_line.chips, register_line.chips)

                mask = (1 << (field_line.bits[1] - field_line.bits[0] + 1)) - 1
                define_name = '_{address}_{field_line.name}(x)'.format(**locals()).ljust(58)
                comment = ' /* {0} */'.format(comment) if comment else ''
                print(
                    '#define   S{define_name} (((unsigned)(x) & 0x{mask:X}) << {field_line.bits[0]}){comment}'
                    .format(**locals()), file=filp)
                print('#define   G{define_name} (((x) >> {field_line.bits[0]}) & 0x{mask:X})'
                         .format(**locals()), file=filp)

                complement = ((1 << 32) - 1) ^ (mask << field_line.bits[0])
                define_name = '_{address}_{field_line.name}'.format(**locals()).ljust(58)
                print('#define   C{define_name} 0x{complement:08X}'
                         .format(**locals()), file=filp)

                value_lines = value_lines_by_address[register_line.address]
                value_idx = 0
                while value_idx < len(value_lines):
                    value_line = value_lines[value_idx]

                    if value_line.enum_refs.isdisjoint(field_line.enum_refs):
                        value_idx += 1
                        continue
                    del value_lines[value_idx]

                    comment = get_chips_comment(value_line.chips, field_line.chips)

                    define_name = 'V_{address}_{value_line.name}'.format(**locals()).ljust(55)
                    comment = ' /* {0} */'.format(comment) if comment else ''
                    print('#define     {define_name} {value_line.enumentry.value}{comment}'
                          .format(**locals()), file=filp)

        if self.guard:
            print('\n#endif // {self.guard}'.format(**locals()), file=filp)


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('--chip', dest='chips', type=str, nargs='*',
                        help='Chip for which to generate the header (all chips if unspecified)')
    parser.add_argument('--sort', choices=['name', 'address'], default='address',
                        help='Sort key for registers, fields, and enum values')
    parser.add_argument('--guard', type=str, help='Name of the #include guard')
    parser.add_argument('files', metavar='FILE', type=str, nargs='+',
                        help='Register database file')
    args = parser.parse_args()

    regdb = None
    for filename in args.files:
        with open(filename, 'r') as filp:
            db = RegisterDatabase.from_json(json.load(filp))
            if regdb is None:
                regdb = db
            else:
                regdb.update(db)

    deduplicate_enums(regdb)
    deduplicate_register_types(regdb)

    w = HeaderWriter(regdb, guard=args.guard)
    w.print(sys.stdout, sort=args.sort)


if __name__ == '__main__':
    main()

# kate: space-indent on; indent-width 4; replace-tabs on;
