CopyRight = '''
/*
 * Copyright 2015-2019 Advanced Micro Devices, Inc.
 *
 * SPDX-License-Identifier: MIT
 */
'''

from collections import defaultdict
import functools
import itertools
import json
import os.path
import re
import sys

AMD_REGISTERS = os.path.abspath(os.path.join(os.path.dirname(sys.argv[0]), "../registers"))
sys.path.append(AMD_REGISTERS)

from regdb import Object, RegisterDatabase


def string_to_chars(string):
    return "'" + "', '".join(string) + "', '\\0',"


class StringTable:
    """
    A class for collecting multiple strings in a single larger string that is
    used by indexing (to avoid relocations in the resulting binary)
    """
    def __init__(self):
        self.table = []
        self.length = 0

    def add(self, string):
        # We might get lucky with string being a suffix of a previously added string
        for te in self.table:
            if te[0].endswith(string):
                idx = te[1] + len(te[0]) - len(string)
                te[2].add(idx)
                return idx

        idx = self.length
        self.table.append((string, idx, set((idx,))))
        self.length += len(string) + 1

        return idx

    def emit(self, filp, name, static=True):
        """
        Write
        [static] const char name[] = "...";
        to filp.
        """
        fragments = [
            '%s /* %s (%s) */' % (
                string_to_chars(te[0].encode('unicode_escape').decode()),
                te[0].encode('unicode_escape').decode(),
                ', '.join(str(idx) for idx in sorted(te[2]))
            )
            for te in self.table
        ]
        filp.write('%sconst char %s[] = {\n%s\n};\n' % (
            'static ' if static else '',
            name,
            '\n'.join('\t' + fragment for fragment in fragments)
        ))

class IntTable:
    """
    A class for collecting multiple arrays of integers in a single big array
    that is used by indexing (to avoid relocations in the resulting binary)
    """
    def __init__(self, typename):
        self.typename = typename
        self.table = []
        self.idxs = set()

    def add(self, array):
        # We might get lucky and find the array somewhere in the existing data
        try:
            idx = 0
            while True:
                idx = self.table.index(array[0], idx, len(self.table) - len(array) + 1)

                for i in range(1, len(array)):
                    if array[i] != self.table[idx + i]:
                        break
                else:
                    self.idxs.add(idx)
                    return idx

                idx += 1
        except ValueError:
            pass

        idx = len(self.table)
        self.table += array
        self.idxs.add(idx)
        return idx

    def emit(self, filp, name, static=True):
        """
        Write
        [static] const typename name[] = { ... };
        to filp.
        """
        idxs = sorted(self.idxs) + [len(self.table)]

        fragments = [
            ('\t/* %s */ %s' % (
                idxs[i],
                ' '.join((str(elt) + ',') for elt in self.table[idxs[i]:idxs[i+1]])
            ))
            for i in range(len(idxs) - 1)
        ]

        filp.write('%sconst %s %s[] = {\n%s\n};\n' % (
            'static ' if static else '',
            self.typename, name,
            '\n'.join(fragments)
        ))

class Field:
    def __init__(self, name, bits):
        self.name = name
        self.bits = bits   # [first, last]
        self.values = []   # [(name, value), ...]

    def format(self, string_table, idx_table):
        mask = ((1 << (self.bits[1] - self.bits[0] + 1)) - 1) << self.bits[0]
        if len(self.values):
            values_offsets = []
            for value in self.values:
                while value[1] >= len(values_offsets):
                    values_offsets.append(-1)
                values_offsets[value[1]] = string_table.add(value[0])
            return '{{{0}, 0x{mask:X}, {1}, {2}}}'.format(
                string_table.add(self.name),
                len(values_offsets), idx_table.add(values_offsets),
                **locals()
            )
        else:
            return '{{{0}, 0x{mask:X}}}'.format(string_table.add(self.name), **locals())

    def __eq__(self, other):
        return (self.name == other.name and
                self.bits[0] == other.bits[0] and self.bits[1] == other.bits[1] and
                len(self.values) == len(other.values) and
                all(a[0] == b[0] and a[1] == b[1] for a, b, in zip(self.values, other.values)))

    def __ne__(self, other):
        return not (self == other)


class FieldTable:
    """
    A class for collecting multiple arrays of register fields in a single big
    array that is used by indexing (to avoid relocations in the resulting binary)
    """
    def __init__(self):
        self.table = []
        self.idxs = set()
        self.name_to_idx = defaultdict(lambda: [])

    def add(self, array):
        """
        Add an array of Field objects, and return the index of where to find
        the array in the table.
        """
        # Check if we can find the array in the table already
        for base_idx in self.name_to_idx.get(array[0].name, []):
            if base_idx + len(array) > len(self.table):
                continue

            for i, a in enumerate(array):
                b = self.table[base_idx + i]
                if a != b:
                    break
            else:
                return base_idx

        base_idx = len(self.table)
        self.idxs.add(base_idx)

        for field in array:
            self.name_to_idx[field.name].append(len(self.table))
            self.table.append(field)

        return base_idx

    def emit(self, filp, string_table, idx_table):
        """
        Write
        static const struct si_field sid_fields_table[] = { ... };
        to filp.
        """
        idxs = sorted(self.idxs) + [len(self.table)]

        filp.write('static const struct si_field sid_fields_table[] = {\n')

        for start, end in zip(idxs, idxs[1:]):
            filp.write('\t/* %s */\n' % (start))
            for field in self.table[start:end]:
                filp.write('\t%s,\n' % (field.format(string_table, idx_table)))

        filp.write('};\n')


def parse_packet3(filp):
    """
    Parse PKT3 commands from the given header file.
    """
    packets = []
    for line in filp:
        if not line.startswith('#define '):
            continue

        line = line[8:].strip()

        if line.startswith('PKT3_') and line.find('0x') != -1 and line.find('(') == -1:
            packets.append(line.split()[0])
    return packets


class TableWriter(object):
    def __init__(self):
        self.__strings = StringTable()
        self.__strings_offsets = IntTable('int')
        self.__fields = FieldTable()

    def write(self, regdb, packets, file=sys.stdout):
        def out(*args):
            print(*args, file=file)

        out('/* This file is autogenerated by sid_tables.py from sid.h. Do not edit directly. */')
        out()
        out(CopyRight.strip())
        out('''
#ifndef SID_TABLES_H
#define SID_TABLES_H

struct si_field {
        unsigned name_offset;
        unsigned mask;
        unsigned num_values;
        unsigned values_offset; /* offset into sid_strings_offsets */
};

struct si_reg {
        unsigned name_offset;
        unsigned offset;
        unsigned num_fields;
        unsigned fields_offset;
};

struct si_packet3 {
        unsigned name_offset;
        unsigned op;
};
''')

        out('static const struct si_packet3 packet3_table[] = {')
        for pkt in packets:
            out('\t{%s, %s},' % (self.__strings.add(pkt[5:]), pkt))
        out('};')
        out()

        regmaps_by_chip = defaultdict(list)

        for regmap in regdb.register_mappings():
            for chip in regmap.chips:
                regmaps_by_chip[chip].append(regmap)

        regtypes = {}

        # Sorted iteration over chips for deterministic builds
        for chip in sorted(regmaps_by_chip.keys()):
            regmaps = regmaps_by_chip[chip]
            regmaps.sort(key=lambda regmap: (regmap.map.to, regmap.map.at))

            out('static const struct si_reg {chip}_reg_table[] = {{'.format(**locals()))

            for regmap in regmaps:
                if hasattr(regmap, 'type_ref'):
                    if not regmap.type_ref in regtypes:
                        regtype = regdb.register_type(regmap.type_ref)
                        fields = []
                        for dbfield in regtype.fields:
                            field = Field(dbfield.name, dbfield.bits)
                            if hasattr(dbfield, 'enum_ref'):
                                enum = regdb.enum(dbfield.enum_ref)
                                for entry in enum.entries:
                                    field.values.append((entry.name, entry.value))
                            fields.append(field)

                        num_fields = len(regtype.fields)
                        fields_offset = self.__fields.add(fields)
                        regtypes[regmap.type_ref] = (num_fields, fields_offset)
                    else:
                        num_fields, fields_offset = regtypes[regmap.type_ref]

                    print('\t{{{0}, {regmap.map.at}, {num_fields}, {fields_offset}}},'
                          .format(self.__strings.add(regmap.name), **locals()))
                else:
                    print('\t{{{0}, {regmap.map.at}}},'
                          .format(self.__strings.add(regmap.name), **locals()))

            out('};\n')

        self.__fields.emit(file, self.__strings, self.__strings_offsets)

        out()

        self.__strings.emit(file, "sid_strings")

        out()

        self.__strings_offsets.emit(file, "sid_strings_offsets")

        out()
        out('#endif')


def main():
    # Parse PKT3 types
    with open(sys.argv[1], 'r') as filp:
        packets = parse_packet3(filp)

    # Register database parse
    regdb = None
    for filename in sys.argv[2:]:
        with open(filename, 'r') as filp:
            try:
                db = RegisterDatabase.from_json(json.load(filp))
                if regdb is None:
                    regdb = db
                else:
                    regdb.update(db)
            except json.JSONDecodeError as e:
                print('Error reading {}'.format(sys.argv[1]), file=sys.stderr)
                raise

    # The ac_debug code only distinguishes by gfx_level
    regdb.merge_chips(['gfx8', 'fiji', 'stoney'], 'gfx8')

    # Write it all out
    w = TableWriter()
    w.write(regdb, packets)

if __name__ == '__main__':
    main()

# kate: space-indent on; indent-width 4; replace-tabs on;
