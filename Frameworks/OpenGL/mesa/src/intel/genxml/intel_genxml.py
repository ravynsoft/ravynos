#!/usr/bin/env python3
# Copyright © 2019, 2022 Intel Corporation
# SPDX-License-Identifier: MIT

from __future__ import annotations
from collections import OrderedDict
import copy
import io
import pathlib
import os.path
import re
import xml.etree.ElementTree as et
import typing

if typing.TYPE_CHECKING:
    class Args(typing.Protocol):

        files: typing.List[pathlib.Path]
        validate: bool
        quiet: bool


def get_filename(element: et.Element) -> str:
    return element.attrib['filename']

def get_name(element: et.Element) -> str:
    return element.attrib['name']

def get_value(element: et.Element) -> int:
    return int(element.attrib['value'], 0)

def get_start(element: et.Element) -> int:
    return int(element.attrib['start'], 0)


BASE_TYPES = {
    'address',
    'offset',
    'int',
    'uint',
    'bool',
    'float',
    'mbz',
    'mbo',
}

FIXED_PATTERN = re.compile(r"(s|u)(\d+)\.(\d+)")

def is_base_type(name: str) -> bool:
    return name in BASE_TYPES or FIXED_PATTERN.match(name) is not None

def add_struct_refs(items: typing.OrderedDict[str, bool], node: et.Element) -> None:
    if node.tag == 'field':
        if 'type' in node.attrib and not is_base_type(node.attrib['type']):
            t = node.attrib['type']
            items[t] = True
        return
    if node.tag not in {'struct', 'group'}:
        return
    for c in node:
        add_struct_refs(items, c)


class Struct(object):
    def __init__(self, xml: et.Element):
        self.xml = xml
        self.name = xml.attrib['name']
        self.deps: typing.OrderedDict[str, Struct] = OrderedDict()

    def find_deps(self, struct_dict, enum_dict) -> None:
        deps: typing.OrderedDict[str, bool] = OrderedDict()
        add_struct_refs(deps, self.xml)
        for d in deps.keys():
            if d in struct_dict:
                self.deps[d] = struct_dict[d]

    def add_xml(self, items: typing.OrderedDict[str, et.Element]) -> None:
        for d in self.deps.values():
            d.add_xml(items)
        items[self.name] = self.xml


# ordering of the various tag attributes
GENXML_DESC = {
    'genxml'      : [ 'name', 'gen', ],
    'import'      : [ 'name', ],
    'exclude'     : [ 'name', ],
    'enum'        : [ 'name', 'value', 'prefix', ],
    'struct'      : [ 'name', 'length', ],
    'field'       : [ 'name', 'start', 'end', 'type', 'default', 'prefix', 'nonzero' ],
    'instruction' : [ 'name', 'bias', 'length', 'engine', ],
    'value'       : [ 'name', 'value', 'dont_use', ],
    'group'       : [ 'count', 'start', 'size', ],
    'register'    : [ 'name', 'length', 'num', ],
}


def node_validator(old: et.Element, new: et.Element) -> bool:
    """Compare to ElementTree Element nodes.
    
    There is no builtin equality method, so calling `et.Element == et.Element` is
    equivalent to calling `et.Element is et.Element`. We instead want to compare
    that the contents are the same, including the order of children and attributes
    """
    return (
        # Check that the attributes are the same
        old.tag == new.tag and
        old.text == new.text and
        (old.tail or "").strip() == (new.tail or "").strip() and
        list(old.attrib.items()) == list(new.attrib.items()) and
        len(old) == len(new) and

        # check that there are no unexpected attributes
        set(new.attrib).issubset(GENXML_DESC[new.tag]) and

        # check that the attributes are sorted
        list(new.attrib) == list(old.attrib) and
        all(node_validator(f, s) for f, s in zip(old, new))
    )


def process_attribs(elem: et.Element) -> None:
    valid = GENXML_DESC[elem.tag]
    # sort and prune attributes
    elem.attrib = OrderedDict(sorted(((k, v) for k, v in elem.attrib.items() if k in valid),
                                     key=lambda x: valid.index(x[0])))
    for e in elem:
        process_attribs(e)


def sort_xml(xml: et.ElementTree) -> None:
    genxml = xml.getroot()

    imports = xml.findall('import')

    enums = sorted(xml.findall('enum'), key=get_name)
    enum_dict: typing.Dict[str, et.Element] = {}
    for e in enums:
        e[:] = sorted(e, key=get_value)
        enum_dict[e.attrib['name']] = e

    # Structs are a bit annoying because they can refer to each other. We sort
    # them alphabetically and then build a graph of dependencies. Finally we go
    # through the alphabetically sorted list and print out dependencies first.
    structs = sorted(xml.findall('./struct'), key=get_name)
    wrapped_struct_dict: typing.Dict[str, Struct] = {}
    for s in structs:
        s[:] = sorted(s, key=get_start)
        ws = Struct(s)
        wrapped_struct_dict[ws.name] = ws

    for ws in wrapped_struct_dict.values():
        ws.find_deps(wrapped_struct_dict, enum_dict)

    sorted_structs: typing.OrderedDict[str, et.Element] = OrderedDict()
    for s in structs:
        _s = wrapped_struct_dict[s.attrib['name']]
        _s.add_xml(sorted_structs)

    instructions = sorted(xml.findall('./instruction'), key=get_name)
    for i in instructions:
        i[:] = sorted(i, key=get_start)

    registers = sorted(xml.findall('./register'), key=get_name)
    for r in registers:
        r[:] = sorted(r, key=get_start)

    new_elems = (imports + enums + list(sorted_structs.values()) +
                 instructions + registers)
    for n in new_elems:
        process_attribs(n)
    genxml[:] = new_elems


# `default_imports` documents which files should be imported for our
# genxml files. This is only useful if a genxml file does not already
# include imports.
#
# Basically, this allows the genxml_import.py tool used with the
# --import switch to know which files should be added as an import.
# (genxml_import.py uses GenXml.add_xml_imports, which relies on
# `default_imports`.)
default_imports = OrderedDict([
    ('gen4.xml', ()),
    ('gen45.xml', ('gen4.xml',)),
    ('gen5.xml', ('gen45.xml',)),
    ('gen6.xml', ('gen5.xml',)),
    ('gen7.xml', ('gen6.xml',)),
    ('gen75.xml', ('gen7.xml',)),
    ('gen8.xml', ('gen75.xml',)),
    ('gen9.xml', ('gen8.xml',)),
    ('gen11.xml', ('gen9.xml',)),
    ('gen12.xml', ('gen11.xml',)),
    ('gen125.xml', ('gen12.xml',)),
    ('gen20.xml', ('gen125.xml',)),
    ('gen20_rt.xml', ('gen125_rt.xml',)),
    ])
known_genxml_files = list(default_imports.keys())


def genxml_path_to_key(path):
    try:
        return known_genxml_files.index(path.name)
    except ValueError:
        return len(known_genxml_files)


def sort_genxml_files(files):
    files.sort(key=genxml_path_to_key)

class GenXml(object):
    def __init__(self, filename, import_xml=False, files=None):
        if files is not None:
            self.files = files
        else:
            self.files = set()
        self.filename = pathlib.Path(filename)

        # Assert that the file hasn't already been loaded which would
        # indicate a loop in genxml imports, and lead to infinite
        # recursion.
        assert self.filename not in self.files

        self.files.add(self.filename)
        self.et = et.parse(self.filename)
        if import_xml:
            self.merge_imported()

    def process_imported(self, merge=False, drop_dupes=False):
        """Processes imported genxml files.

        This helper function scans imported genxml files and has two
        mutually exclusive operating modes.

        If `merge` is True, then items will be merged into the
        `self.et` data structure.

        If `drop_dupes` is True, then any item that is a duplicate to
        an item imported will be droped from the `self.et` data
        structure. This is used by `self.optimize_xml_import` to
        shrink the size of the genxml file by reducing duplications.

        """
        assert merge != drop_dupes
        orig_elements = set(self.et.getroot())
        name_and_obj = lambda i: (get_name(i), i)
        filter_ty = lambda s: filter(lambda i: i.tag == s, orig_elements)
        filter_ty_item = lambda s: dict(map(name_and_obj, filter_ty(s)))

        # orig_by_tag stores items defined directly in the genxml
        # file. If a genxml item is defined in the genxml directly,
        # then any imported items of the same name are ignored.
        orig_by_tag = {
            'enum': filter_ty_item('enum'),
            'struct': filter_ty_item('struct'),
            'instruction': filter_ty_item('instruction'),
            'register': filter_ty_item('register'),
        }

        for item in orig_elements:
            if item.tag == 'import':
                assert 'name' in item.attrib
                filename = os.path.split(item.attrib['name'])
                exceptions = set()
                for e in item:
                    assert e.tag == 'exclude'
                    exceptions.add(e.attrib['name'])
                # We should be careful to restrict loaded files to
                # those under the source or build trees. For now, only
                # allow siblings of the current xml file.
                assert filename[0] == '', 'Directories not allowed with import'
                filename = os.path.join(os.path.dirname(self.filename),
                                        filename[1])
                assert os.path.exists(filename), f'{self.filename} {filename}'

                # Here we load the imported genxml file. We set
                # `import_xml` to true so that any imports in the
                # imported genxml will be merged during the loading
                # process.
                #
                # The `files` parameter is a set of files that have
                # been loaded, and it is used to prevent any cycles
                # (infinite recursion) while loading imported genxml
                # files.
                genxml = GenXml(filename, import_xml=True, files=self.files)
                imported_elements = set(genxml.et.getroot())

                # `to_add` is a set of items that were imported an
                # should be merged into the `self.et` data structure.
                # This is only used when the `merge` parameter is
                # True.
                to_add = set()
                # `to_remove` is a set of items that can safely be
                # imported since the item is equivalent. This is only
                # used when the `drop_duped` parameter is True.
                to_remove = set()
                for i in imported_elements:
                    if i.tag not in orig_by_tag:
                        continue
                    if i.attrib['name'] in exceptions:
                        continue
                    if i.attrib['name'] in orig_by_tag[i.tag]:
                        if merge:
                            # An item with this same name was defined
                            # in the genxml directly. There we should
                            # ignore (not merge) the imported item.
                            continue
                    else:
                        if drop_dupes:
                            # Since this item is not the imported
                            # genxml, we can't consider dropping it.
                            continue
                    if merge:
                        to_add.add(i)
                    else:
                        assert drop_dupes
                        orig_element = orig_by_tag[i.tag][i.attrib['name']]
                        if not node_validator(i, orig_element):
                            continue
                        to_remove.add(orig_element)

                if len(to_add) > 0:
                    # Now that we have scanned through all the items
                    # in the imported genxml file, if any items were
                    # found which should be merged, we add them into
                    # our `self.et` data structure. After this it will
                    # be as if the items had been directly present in
                    # the genxml file.
                    assert len(to_remove) == 0
                    self.et.getroot().extend(list(to_add))
                    sort_xml(self.et)
                elif len(to_remove) > 0:
                    self.et.getroot()[:] = list(orig_elements - to_remove)
                    sort_xml(self.et)

    def merge_imported(self):
        """Merge imported items from genxml imports.

        Genxml <import> tags specify that elements should be brought
        in from another genxml source file. After this function is
        called, these elements will become part of the `self.et` data
        structure as if the elements had been directly included in the
        genxml directly.

        Items from imported genxml files will be completely ignore if
        an item with the same name is already defined in the genxml
        file.

        """
        self.process_imported(merge=True)

    def flatten_imported(self):
        """Flattens the genxml to not include any imports

        Essentially this helper will put the `self.et` into a state
        that includes all imported items directly, and does not
        contain any <import> tags. This is used by the
        genxml_import.py with the --flatten switch to "undo" any
        genxml imports.

        """
        self.merge_imported()
        root = self.et.getroot()
        imports = root.findall('import')
        for i in imports:
            root.remove(i)

    def add_xml_imports(self):
        """Adds imports to the genxml file.

        Using the `default_imports` structure, we add imports to the
        genxml file.

        """
        # `imports` is a set of filenames currently imported by the
        # genxml.
        imports = self.et.findall('import')
        imports = set(map(lambda el: el.attrib['name'], imports))
        new_elements = []
        self_flattened = copy.deepcopy(self)
        self_flattened.flatten_imported()
        old_names = { el.attrib['name'] for el in self_flattened.et.getroot() }
        for import_xml in default_imports.get(self.filename.name, tuple()):
            if import_xml in imports:
                # This genxml is already imported, so we don't need to
                # add it as an import.
                continue
            el = et.Element('import', {'name': import_xml})
            import_path = self.filename.with_name(import_xml)
            imported_genxml = GenXml(import_path, import_xml=True)
            imported_names = { el.attrib['name']
                               for el in imported_genxml.et.getroot()
                               if el.tag != 'import' }
            # Importing this genxml could add some new items. When
            # adding a genxml import, we don't want to add new items,
            # unless they were already in the current genxml. So, we
            # put them into a list of items to exclude when importing
            # the genxml.
            exclude_names = imported_names - old_names
            for n in sorted(exclude_names):
                el.append(et.Element('exclude', {'name': n}))
            new_elements.append(el)
        if len(new_elements) > 0:
            self.et.getroot().extend(new_elements)
            sort_xml(self.et)

    def optimize_xml_import(self):
        """Optimizes the genxml by dropping items that can be imported

        Scans genxml <import> tags, and loads the imported file. If
        any item in the imported file is a duplicate to an item in the
        genxml file, then it will be droped from the `self.et` data
        structure.

        """
        self.process_imported(drop_dupes=True)

    def filter_engines(self, engines):
        changed = False
        items = []
        for item in self.et.getroot():
            # When an instruction doesn't have the engine specified,
            # it is considered to be for all engines. Otherwise, we
            # check to see if it's tagged for the engines requested.
            if item.tag == 'instruction' and 'engine' in item.attrib:
                i_engines = set(item.attrib["engine"].split('|'))
                if not (i_engines & engines):
                    # Drop this instruction because it doesn't support
                    # the requested engine types.
                    changed = True
                    continue
            items.append(item)
        if changed:
            self.et.getroot()[:] = items

    def sort(self):
        sort_xml(self.et)

    def sorted_copy(self):
        clone = copy.deepcopy(self)
        clone.sort()
        return clone

    def is_equivalent_xml(self, other):
        if len(self.et.getroot()) != len(other.et.getroot()):
            return False
        return all(node_validator(old, new)
                   for old, new in zip(self.et.getroot(), other.et.getroot()))

    def write_file(self):
        try:
            old_genxml = GenXml(self.filename)
            if self.is_equivalent_xml(old_genxml):
                return
        except Exception:
            pass

        b_io = io.BytesIO()
        et.indent(self.et, space='  ')
        self.et.write(b_io, encoding="utf-8", xml_declaration=True)
        b_io.write(b'\n')

        tmp = self.filename.with_suffix(f'{self.filename.suffix}.tmp')
        tmp.write_bytes(b_io.getvalue())
        tmp.replace(self.filename)
