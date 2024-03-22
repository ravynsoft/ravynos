#
# Copyright 2017-2019 Advanced Micro Devices, Inc.
#
# SPDX-License-Identifier: MIT
#
"""
Python package containing common tools for manipulating register JSON.
"""

import itertools
import json
import re
import sys

from collections import defaultdict
from contextlib import contextmanager

class UnionFind(object):
    """
    Simplistic implementation of a union-find data structure that also keeps
    track of the sets that have been unified.

    - add: add an element to the implied global set of elements
    - union: unify the sets containing the two given elements
    - find: return the representative element of the set containing the
            given element
    - get_set: get the set containing the given element
    - sets: iterate over all sets (the sets form a partition of the set of all
            elements that have ever been added)
    """
    def __init__(self):
        self.d = {}

    def add(self, k):
        if k not in self.d:
            self.d[k] = set([k])

    def union(self, k1, k2):
        k1 = self.find(k1)
        k2 = self.find(k2)
        if k1 == k2:
            return
        if len(k1) < len(k2):
            k1, k2 = k2, k1
        self.d[k1].update(self.d[k2])
        self.d[k2] = (k1,)

    def find(self, k):
        e = self.d[k]
        if isinstance(e, set):
            return k
        assert isinstance(e, tuple)
        r = self.find(e[0])
        self.d[k] = (r,)
        return r

    def get_set(self, k):
        k = self.find(k)
        assert isinstance(self.d[k], set)
        return self.d[k]

    def sets(self):
        for v in self.d.values():
            if isinstance(v, set):
                yield v


class Object(object):
    """
    Convenience helper class that essentially acts as a dictionary for convenient
    conversion from and to JSON while allowing the use of .field notation
    instead of subscript notation for member access.
    """
    def __init__(self, **kwargs):
        for k, v in kwargs.items():
            setattr(self, k, v)

    def update(self, **kwargs):
        for key, value in kwargs.items():
            setattr(self, key, value)
        return self

    def __str__(self):
        return 'Object(' + ', '.join(
            '{k}={v}'.format(**locals()) for k, v, in self.__dict__.items()
        ) + ')'

    @staticmethod
    def from_json(json, keys=None):
        if isinstance(json, list):
            return [Object.from_json(v) for v in json]
        elif isinstance(json, dict):
            obj = Object()
            for k, v in json.items():
                if keys is not None and k in keys:
                    v = keys[k](v)
                else:
                    v = Object.from_json(v)
                setattr(obj, k, v)
            return obj
        else:
            return json

    @staticmethod
    def to_json(obj):
        if isinstance(obj, Object):
            return dict((k, Object.to_json(v)) for k, v in obj.__dict__.items())
        elif isinstance(obj, dict):
            return dict((k, Object.to_json(v)) for k, v in obj.items())
        elif isinstance(obj, list):
            return [Object.to_json(v) for v in obj]
        else:
            return obj

class MergeError(Exception):
    def __init__(self, msg):
        super(MergeError, self).__init__(msg)

class RegisterDatabaseError(Exception):
    def __init__(self, msg):
        super(RegisterDatabaseError, self).__init__(msg)

@contextmanager
def merge_scope(name):
    """
    Wrap a merge handling function in a "scope" whose name will be added when
    propagating MergeErrors.
    """
    try:
        yield
    except Exception as e:
        raise MergeError('{name}: {e}'.format(**locals()))

def merge_dicts(dicts, keys=None, values=None):
    """
    Generic dictionary merging function.

    dicts -- list of (origin, dictionary) pairs to merge
    keys -- optional dictionary to provide a merge-strategy per key;
            the merge strategy is a callable which will receive a list of
            (origin, value) pairs
    value -- optional function which provides a merge-strategy for values;
             the merge strategy is a callable which will receive the name of
             the key and a list of (origin, value) pairs

    The default strategy is to allow merging keys if all origin dictionaries
    that contain the key have the same value for it.
    """
    ks = set()
    for _, d in dicts:
        ks.update(d.keys())

    result = {}
    for k in ks:
        vs = [(o, d[k]) for o, d in dicts if k in d]
        with merge_scope('Key {k}'.format(**locals())):
            if keys is not None and k in keys:
                result[k] = keys[k](vs)
            elif values is not None:
                result[k] = values(k, vs)
            else:
                base_origin, base = vs[0]
                for other_origin, other in vs[1:]:
                    if base != other:
                        raise MergeError('{base} (from {base_origin}) != {other} (from {other_origin})'.format(**locals()))
                result[k] = base
    return result

def merge_objects(objects, keys=None):
    """
    Like merge_dicts, but applied to instances of Object.
    """
    return Object(**merge_dicts([(origin, obj.__dict__) for origin, obj in objects], keys=keys))

class RegisterDatabase(object):
    """
    A register database containing:

    - enums: these are lists of named values that can occur in a register field
    - register types: description of a register type or template as a list of
                      fields
    - register mappings: named and typed registers mapped at locations in an
                         address space
    """
    def __init__(self):
        self.__enums = {}
        self.__register_types = {}
        self.__register_mappings = []
        self.__regmap_by_addr = None
        self.__chips = None

    def __post_init(self):
        """
        Perform some basic canonicalization:
        - enum entries are sorted by value
        - register type fields are sorted by starting bit
        - __register_mappings is sorted by offset
        - the chips field of register mappings is sorted

        Lazily computes the set of all chips mentioned by register mappings.
        """
        if self.__regmap_by_addr is not None:
            return

        for enum in self.__enums.values():
            enum.entries.sort(key=lambda entry: entry.value)

        for regtype in self.__register_types.values():
            regtype.fields.sort(key=lambda field: field.bits[0])

        self.__regmap_by_addr = defaultdict(list)
        self.__chips = set()

        # Merge register mappings using sort order and garbage collect enums
        # and register types.
        old_register_mappings = self.__register_mappings
        old_register_mappings.sort(key=lambda regmap: regmap.map.at)

        self.__register_mappings = []
        for regmap in old_register_mappings:
            addr = (regmap.map.to, regmap.map.at)
            chips = set(getattr(regmap, 'chips', ['undef']))
            type_ref = getattr(regmap, 'type_ref', None)

            self.__chips.update(chips)

            merged = False
            for other in reversed(self.__register_mappings):
                if other.name != regmap.name:
                    break

                other_addr = (other.map.to, other.map.at)
                other_chips = getattr(other, 'chips', ['undef'])
                other_type_ref = getattr(other, 'type_ref', None)

                if addr == other_addr and\
                   (type_ref is None or other_type_ref is None or type_ref == other_type_ref):
                    other.chips = sorted(list(chips.union(other_chips)))
                    if type_ref is not None:
                        other.type_ref = type_ref
                    merged = True
                    break

            if merged:
                continue

            addrmappings = self.__regmap_by_addr[addr]

            for other in addrmappings:
                other_type_ref = getattr(other, 'type_ref', None)
                other_chips = getattr(other, 'chips', ['undef'])
                if type_ref is not None and other_type_ref is not None and \
                   type_ref != other_type_ref and chips.intersection(other_chips):
                    raise RegisterDatabaseError(
                        'Registers {0} and {1} overlap and have conflicting types'.format(
                            other.name, regmap.name))

            addrmappings.append(regmap)
            self.__register_mappings.append(regmap)

    def garbage_collect(self):
        """
        Remove unreferenced enums and register types.
        """
        old_enums = self.__enums
        old_register_types = self.__register_types

        self.__enums = {}
        self.__register_types = {}
        for regmap in self.__register_mappings:
            if hasattr(regmap, 'type_ref') and regmap.type_ref not in self.__register_types:
                regtype = old_register_types[regmap.type_ref]
                self.__register_types[regmap.type_ref] = regtype
                for field in regtype.fields:
                    if hasattr(field, 'enum_ref') and field.enum_ref not in self.__enums:
                        self.__enums[field.enum_ref] = old_enums[field.enum_ref]

    def __validate_register_type(self, regtype):
        for field in regtype.fields:
            if hasattr(field, 'enum_ref') and field.enum_ref not in self.__enums:
                raise RegisterDatabaseError(
                    'Register type field {0} has unknown enum_ref {1}'.format(
                        field.name, field.enum_ref))

    def __validate_register_mapping(self, regmap):
        if hasattr(regmap, 'type_ref') and regmap.type_ref not in self.__register_types:
            raise RegisterDatabaseError(
                'Register mapping {0} has unknown type_ref {1}'.format(
                    regmap.name, regmap.type_ref))

    def __validate(self):
        for regtype in self.__register_types.values():
            self.__validate_register_type(regtype)
        for regmap in self.__register_mappings:
            self.__validate_register_mapping(regmap)

    @staticmethod
    def enum_key(enum):
        """
        Return a key that uniquely describes the signature of the given
        enum (assuming that it has been canonicalized). Two enums with the
        same key can be merged.
        """
        return ''.join(
            ':{0}:{1}'.format(entry.name, entry.value)
            for entry in enum.entries
        )

    def add_enum(self, name, enum):
        if name in self.__enums:
            raise RegisterDatabaseError('Duplicate enum ' + name)
        self.__enums[name] = enum

    @staticmethod
    def __merge_enums(enums, union=False):
        def merge_entries(entries_lists):
            values = defaultdict(list)
            for origin, enum in entries_lists:
                for entry in enum:
                    values[entry.value].append((origin, entry))

            if not union:
                if any(len(entries) != len(enums) for entries in values.values()):
                    raise RegisterDatabaseError(
                        'Attempting to merge enums with different values')

            return [
                merge_objects(entries)
                for entries in values.values()
            ]

        return merge_objects(
            enums,
            keys={
                'entries': merge_entries,
            }
        )

    def merge_enums(self, names, newname, union=False):
        """
        Given a list of enum names, merge them all into one with a new name and
        update all references.
        """
        if newname not in names and newname in self.__enums:
            raise RegisterDatabaseError('Enum {0} already exists'.format(newname))

        newenum = self.__merge_enums(
            [(name, self.__enums[name]) for name in names],
            union=union
        )

        for name in names:
            del self.__enums[name]
        self.__enums[newname] = newenum

        for regtype in self.__register_types.values():
            for field in regtype.fields:
                if getattr(field, 'enum_ref', None) in names:
                    field.enum_ref = newname

        self.__regmap_by_addr = None

    def add_register_type(self, name, regtype):
        if regtype in self.__register_types:
            raise RegisterDatabaseError('Duplicate register type ' + name)
        self.__register_types[name] = regtype
        self.__validate_register_type(regtype)

    def register_type(self, name):
        self.__post_init()
        return self.__register_types[name]

    @staticmethod
    def __merge_register_types(regtypes, union=False, field_keys={}):
        def merge_fields(fields_lists):
            fields = defaultdict(list)
            for origin, fields_list in fields_lists:
                for field in fields_list:
                    fields[field.bits[0]].append((origin, field))

            if not union:
                if any(len(entries) != len(regtypes) for entries in fields.values()):
                    raise RegisterDatabaseError(
                        'Attempting to merge register types with different fields')

            return [
                merge_objects(field, keys=field_keys)
                for field in fields.values()
            ]

        with merge_scope('Register types {0}'.format(', '.join(name for name, _ in regtypes))):
            return merge_objects(
                regtypes,
                keys={
                    'fields': merge_fields,
                }
            )

    def merge_register_types(self, names, newname, union=False):
        """
        Given a list of register type names, merge them all into one with a
        new name and update all references.
        """
        if newname not in names and newname in self.__register_types:
            raise RegisterDatabaseError('Register type {0} already exists'.format(newname))

        newregtype = self.__merge_register_types(
            [(name, self.__register_types[name]) for name in names],
            union=union
        )

        for name in names:
            del self.__register_types[name]
        self.__register_types[newname] = newregtype

        for regmap in self.__register_mappings:
            if getattr(regmap, 'type_ref', None) in names:
                regmap.type_ref = newname

        self.__regmap_by_addr = None

    def add_register_mapping(self, regmap):
        self.__regmap_by_addr = None
        self.__register_mappings.append(regmap)
        self.__validate_register_mapping(regmap)

    def remove_register_mappings(self, regmaps_to_remove):
        self.__post_init()

        regmaps_to_remove = set(regmaps_to_remove)

        regmaps = self.__register_mappings
        self.__register_mappings = []
        for regmap in regmaps:
            if regmap not in regmaps_to_remove:
                self.__register_mappings.append(regmap)

        self.__regmap_by_addr = None

    def enum(self, name):
        """
        Return the enum of the given name, if any.
        """
        self.__post_init()
        return self.__enums.get(name, None)

    def enums(self):
        """
        Yields all (name, enum) pairs.
        """
        self.__post_init()
        for name, enum in self.__enums.items():
            yield (name, enum)

    def fields(self):
        """
        Yields all (register_type, fields) pairs.
        """
        self.__post_init()
        for regtype in self.__register_types.values():
            for field in regtype.fields:
                yield (regtype, field)

    def register_types(self):
        """
        Yields all (name, register_type) pairs.
        """
        self.__post_init()
        for name, regtype in self.__register_types.items():
            yield (name, regtype)

    def register_mappings_by_name(self, name):
        """
        Return a list of register mappings with the given name.
        """
        self.__post_init()

        begin = 0
        end = len(self.__register_mappings)
        while begin < end:
            middle = (begin + end) // 2
            if self.__register_mappings[middle].name < name:
                begin = middle + 1
            elif name < self.__register_mappings[middle].name:
                end = middle
            else:
                break

        if begin >= end:
            return []

        # We now have begin <= mid < end with begin.name <= name, mid.name == name, name < end.name
        # Narrow down begin and end
        hi = middle
        while begin < hi:
            mid = (begin + hi) // 2
            if self.__register_mappings[mid].name < name:
                begin = mid + 1
            else:
                hi = mid

        lo = middle + 1
        while lo < end:
            mid = (lo + end) // 2
            if self.__register_mappings[mid].name == name:
                lo = mid + 1
            else:
                end = mid

        return self.__register_mappings[begin:end]

    def register_mappings(self):
        """
        Yields all register mappings.
        """
        self.__post_init()
        for regmap in self.__register_mappings:
            yield regmap

    def chips(self):
        """
        Yields all chips.
        """
        self.__post_init()
        return iter(self.__chips)

    def merge_chips(self, chips, newchip):
        """
        Merge register mappings of the given chips into a single chip of the
        given name. Recursively merges register types and enums when appropriate.
        """
        self.__post_init()

        chips = set(chips)

        regtypes_merge = UnionFind()
        enums_merge = UnionFind()

        # Walk register mappings to find register types that should be merged.
        for idx, regmap in itertools.islice(enumerate(self.__register_mappings), 1, None):
            if not hasattr(regmap, 'type_ref'):
                continue
            if chips.isdisjoint(regmap.chips):
                continue

            for other in self.__register_mappings[idx-1::-1]:
                if regmap.name != other.name:
                    break
                if chips.isdisjoint(other.chips):
                    continue
                if regmap.map.to != other.map.to or regmap.map.at != other.map.at:
                    raise RegisterDatabaseError(
                        'Attempting to merge chips with incompatible addresses of {0}'.format(regmap.name))
                if not hasattr(regmap, 'type_ref'):
                    continue

                if regmap.type_ref != other.type_ref:
                    regtypes_merge.add(regmap.type_ref)
                    regtypes_merge.add(other.type_ref)
                    regtypes_merge.union(regmap.type_ref, other.type_ref)

        # Walk over regtype sets that are to be merged and find enums that
        # should be merged.
        for type_refs in regtypes_merge.sets():
            fields_merge = defaultdict(set)
            for type_ref in type_refs:
                regtype = self.__register_types[type_ref]
                for field in regtype.fields:
                    if hasattr(field, 'enum_ref'):
                        fields_merge[field.name].add(field.enum_ref)

            for enum_refs in fields_merge.values():
                if len(enum_refs) > 1:
                    enum_refs = list(enum_refs)
                    enums_merge.add(enum_refs[0])
                    for enum_ref in enum_refs[1:]:
                        enums_merge.add(enum_ref)
                        enums_merge.union(enum_ref, enum_refs[0])

        # Merge all mergeable enum sets
        remap_enum_refs = {}
        for enum_refs in enums_merge.sets():
            enum_refs = sorted(enum_refs)
            newname = enum_refs[0] + '_' + newchip
            i = 0
            while newname in self.__enums:
                newname = enum_refs[0] + '_' + newchip + str(i)
                i += 1

            for enum_ref in enum_refs:
                remap_enum_refs[enum_ref] = newname

            # Don't use self.merge_enums, because we don't want to automatically
            # update _all_ references to the merged enums (some may be from
            # register types that aren't going to be merged).
            self.add_enum(newname, self.__merge_enums(
                [(enum_ref, self.__enums[enum_ref]) for enum_ref in enum_refs],
                union=True
            ))

        # Merge all mergeable type refs
        remap_type_refs = {}
        for type_refs in regtypes_merge.sets():
            type_refs = sorted(type_refs)
            newname = type_refs[0] + '_' + newchip
            i = 0
            while newname in self.__enums:
                newname = type_refs[0] + '_' + newchip + str(i)
                i += 1

            updated_regtypes = []
            for type_ref in type_refs:
                remap_type_refs[type_ref] = newname

                regtype = Object.from_json(Object.to_json(self.__register_types[type_ref]))
                for field in regtype.fields:
                    if hasattr(field, 'enum_ref'):
                        field.enum_ref = remap_enum_refs.get(enum_ref, enum_ref)

                updated_regtypes.append(regtype)

            def merge_enum_refs(enum_refs):
                enum_refs = set(
                    remap_enum_refs.get(enum_ref, enum_ref)
                    for origin, enum_ref in enum_refs
                )
                assert len(enum_refs) == 1 # should be ensured by how we determine the enums to be merged
                return enum_refs.pop()

            self.add_register_type(newname, self.__merge_register_types(
                [(type_ref, self.__register_types[type_ref]) for type_ref in type_refs],
                field_keys={
                    'enum_ref': merge_enum_refs,
                },
                union=True
            ))

        # Merge register mappings
        register_mappings = self.__register_mappings
        self.__register_mappings = []

        regmap_accum = None
        for regmap in register_mappings:
            if regmap_accum and regmap.name != regmap_accum.name:
                regmap_accum.chips = [newchip]
                self.__register_mappings.append(regmap_accum)
                regmap_accum = None

            joining_chips = chips.intersection(regmap.chips)
            if not joining_chips:
                self.__register_mappings.append(regmap)
                continue
            remaining_chips = set(regmap.chips).difference(chips)

            type_ref = getattr(regmap, 'type_ref', None)
            if type_ref is None:
                regmap.chips = sorted(remaining_chips.union([newchip]))
                self.__register_mappings.append(regmap)
                continue

            type_ref = remap_type_refs.get(type_ref, type_ref)
            if remaining_chips:
                regmap.chips = sorted(remaining_chips)
                self.__register_mappings.append(regmap)
                if not regmap_accum:
                    regmap = Object.from_json(Object.to_json(regmap))
                    if type_ref is not None:
                        regmap.type_ref = type_ref

            if not regmap_accum:
                regmap_accum = regmap
            else:
                if not hasattr(regmap_accum.type_ref, 'type_ref'):
                    if type_ref is not None:
                        regmap_accum.type_ref = type_ref
                else:
                    assert type_ref is None or type_ref == regmap_accum.type_ref
        if regmap_accum:
            self.__register_mappings.append(regmap_accum)

    def update(self, other):
        """
        Add the contents of the other database to self.

        Doesn't de-duplicate entries.
        """
        self.__post_init()
        other.__post_init()

        enum_remap = {}
        regtype_remap = {}

        for regmap in other.__register_mappings:
            regmap = Object.from_json(Object.to_json(regmap))

            type_ref = getattr(regmap, 'type_ref', None)
            if type_ref is not None and type_ref not in regtype_remap:
                regtype = Object.from_json(Object.to_json(other.__register_types[type_ref]))

                chips = getattr(regmap, 'chips', [])
                suffix = '_' + chips[0] if chips else ''

                for field in regtype.fields:
                    enum_ref = getattr(field, 'enum_ref', None)
                    if enum_ref is not None and enum_ref not in enum_remap:
                        enum = Object.from_json(Object.to_json(other.__enums[enum_ref]))

                        remapped = enum_ref + suffix if enum_ref in self.__enums else enum_ref
                        i = 0
                        while remapped in self.__enums:
                            remapped = enum_ref + suffix + str(i)
                            i += 1
                        self.add_enum(remapped, enum)
                        enum_remap[enum_ref] = remapped

                    if enum_ref is not None:
                        field.enum_ref = enum_remap[enum_ref]

                remapped = type_ref + suffix if type_ref in self.__register_types else type_ref
                i = 0
                while remapped in self.__register_types:
                    remapped = type_ref + suffix + str(i)
                    i += 1
                self.add_register_type(remapped, regtype)
                regtype_remap[type_ref] = remapped

            if type_ref is not None:
                regmap.type_ref = regtype_remap[type_ref]

            self.add_register_mapping(regmap)

    def to_json(self):
        self.__post_init()
        return {
            'enums': Object.to_json(self.__enums),
            'register_types': Object.to_json(self.__register_types),
            'register_mappings': Object.to_json(self.__register_mappings),
        }

    def encode_json_pretty(self):
        """
        Use a custom JSON encoder which pretty prints, but keeps inner structures compact
        """
        # Since the JSON module isn't very extensible, this ends up being
        # really hacky.
        obj = self.to_json()

        replacements = []
        def placeholder(s):
            placeholder = "JSON-{key}-NOSJ".format(key=len(replacements))
            replacements.append(json.dumps(s, sort_keys=True))
            return placeholder

        # Pre-create non-indented encodings for inner objects
        for enum in obj['enums'].values():
            enum['entries'] = [
                placeholder(entry)
                for entry in enum['entries']
            ]

        for regtype in obj['register_types'].values():
            regtype['fields'] = [
                placeholder(field)
                for field in regtype['fields']
            ]

        for regmap in obj['register_mappings']:
            regmap['map'] = placeholder(regmap['map'])
            if 'chips' in regmap:
                regmap['chips'] = placeholder(regmap['chips'])

        # Now create the 'outer' encoding with indentation and search-and-replace
        # placeholders
        result = json.dumps(obj, indent=1, sort_keys=True)

        result = re.sub(
            '"JSON-([0-9]+)-NOSJ"',
            lambda m: replacements[int(m.group(1))],
            result
        )

        return result

    @staticmethod
    def from_json(json):
        db = RegisterDatabase()

        db.__enums = dict((k, Object.from_json(v)) for k, v in json['enums'].items())
        if 'register_types' in json:
            db.__register_types = dict(
                (k, Object.from_json(v))
                for k, v in json['register_types'].items()
            )
        if 'register_mappings' in json:
            db.__register_mappings = Object.from_json(json['register_mappings'])

        # Old format
        if 'registers' in json:
            for reg in json['registers']:
                type_ref = None
                if 'fields' in reg and reg['fields']:
                    type_ref = reg['names'][0]
                    db.add_register_type(type_ref, Object(
                        fields=Object.from_json(reg['fields'])
                    ))

                for name in reg['names']:
                    regmap = Object(
                        name=name,
                        map=Object.from_json(reg['map'])
                    )
                    if type_ref is not None:
                        regmap.type_ref = type_ref
                    db.add_register_mapping(regmap)

        db.__post_init()
        return db

def deduplicate_enums(regdb):
    """
    Find enums that have the exact same entries and merge them.
    """
    buckets = defaultdict(list)
    for name, enum in regdb.enums():
        buckets[RegisterDatabase.enum_key(enum)].append(name)

    for bucket in buckets.values():
        if len(bucket) > 1:
            regdb.merge_enums(bucket, bucket[0])

def deduplicate_register_types(regdb):
    """
    Find register types with the exact same fields (identified by name and
    bit range) and merge them.

    However, register types *aren't* merged if they have different enums for
    the same field (as an exception, if one of them has an enum and the other
    one doesn't, we assume that one is simply missing a bit of information and
    merge the register types).
    """
    buckets = defaultdict(list)
    for name, regtype in regdb.register_types():
        key = ''.join(
            ':{0}:{1}:{2}:'.format(
                field.name, field.bits[0], field.bits[1],
            )
            for field in regtype.fields
        )
        buckets[key].append((name, regtype.fields))

    for bucket in buckets.values():
        # Register types in the same bucket have the same fields in the same
        # places, but they may have different enum_refs. Allow merging when
        # one has an enum_ref and another doesn't, but don't merge if they
        # have enum_refs that differ.
        bucket_enum_refs = [
            [getattr(field, 'enum_ref', None) for field in fields]
            for name, fields in bucket
        ]
        while bucket:
            regtypes = [bucket[0][0]]
            enum_refs = bucket_enum_refs[0]
            del bucket[0]
            del bucket_enum_refs[0]

            idx = 0
            while idx < len(bucket):
                if all([
                    not lhs or not rhs or lhs == rhs
                    for lhs, rhs in zip(enum_refs, bucket_enum_refs[idx])
                ]):
                    regtypes.append(bucket[idx][0])
                    enum_refs = [lhs or rhs for lhs, rhs in zip(enum_refs, bucket_enum_refs[idx])]
                    del bucket[idx]
                    del bucket_enum_refs[idx]
                else:
                    idx += 1

            if len(regtypes) > 1:
                regdb.merge_register_types(regtypes, regtypes[0])

# kate: space-indent on; indent-width 4; replace-tabs on;
