# encoding=utf-8

# Copyright © 2022 Imagination Technologies Ltd.

# based on anv driver gen_pack_header.py which is:
# Copyright © 2016 Intel Corporation

# based on v3dv driver gen_pack_header.py which is:
# Copyright (C) 2016 Broadcom

# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:

# The above copyright notice and this permission notice (including the next
# paragraph) shall be included in all copies or substantial portions of the
# Software.

# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
# SOFTWARE.

from __future__ import annotations

import copy
import os
import textwrap
import typing as t
import xml.parsers.expat as expat
from abc import ABC
from ast import literal_eval


MIT_LICENSE_COMMENT = """/*
 * Copyright © %(copyright)s
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the next
 * paragraph) shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */"""

PACK_FILE_HEADER = """%(license)s

/* Enums, structures and pack functions for %(platform)s.
 *
 * This file has been generated, do not hand edit.
 */

#ifndef %(guard)s
#define %(guard)s

#include "csbgen/pvr_packet_helpers.h"

"""


def safe_name(name: str) -> str:
    if not name[0].isalpha():
        name = "_" + name

    return name


def num_from_str(num_str: str) -> int:
    if num_str.lower().startswith("0x"):
        return int(num_str, base=16)

    if num_str.startswith("0") and len(num_str) > 1:
        raise ValueError("Octal numbers not allowed")

    return int(num_str)


class Node(ABC):
    __slots__ = ["parent", "name"]

    parent: Node
    name: str

    def __init__(self, parent: Node, name: str, *, name_is_safe: bool = False) -> None:
        self.parent = parent
        if name_is_safe:
            self.name = name
        else:
            self.name = safe_name(name)

    @property
    def full_name(self) -> str:
        if self.name[0] == "_":
            return self.parent.prefix + self.name.upper()

        return self.parent.prefix + "_" + self.name.upper()

    @property
    def prefix(self) -> str:
        return self.parent.prefix

    def add(self, element: Node) -> None:
        raise RuntimeError("Element cannot be nested in %s. Element Type: %s"
                           % (type(self).__name__.lower(), type(element).__name__))


class Csbgen(Node):
    __slots__ = ["prefix_field", "filename", "_defines", "_enums", "_structs", "_streams"]

    prefix_field: str
    filename: str
    _defines: t.List[Define]
    _enums: t.Dict[str, Enum]
    _structs: t.Dict[str, Struct]
    _streams: t.Dict[str, Stream]

    def __init__(self, name: str, prefix: str, filename: str) -> None:
        super().__init__(None, name.upper())
        self.prefix_field = safe_name(prefix.upper())
        self.filename = filename

        self._defines = []
        self._enums = {}
        self._structs = {}
        self._streams = {}

    @property
    def full_name(self) -> str:
        return self.name + "_" + self.prefix_field

    @property
    def prefix(self) -> str:
        return self.full_name

    def add(self, element: Node) -> None:
        if isinstance(element, Enum):
            if element.name in self._enums:
                raise RuntimeError("Enum redefined. Enum: %s" % element.name)

            self._enums[element.name] = element
        elif isinstance(element, Struct):
            if element.name in self._structs:
                raise RuntimeError("Struct redefined. Struct: %s" % element.name)

            self._structs[element.name] = element
        elif isinstance(element, Stream):
            if element.name in self._streams:
                raise RuntimeError("Stream redefined. Stream: %s" % element.name)

            self._streams[element.name] = element
        elif isinstance(element, Define):
            define_names = [d.full_name for d in self._defines]
            if element.full_name in define_names:
                raise RuntimeError("Define redefined. Define: %s" % element.full_name)

            self._defines.append(element)
        else:
            super().add(element)

    def _gen_guard(self) -> str:
        return os.path.basename(self.filename).replace(".xml", "_h").upper()

    def emit(self) -> None:
        print(PACK_FILE_HEADER % {
            "license": MIT_LICENSE_COMMENT % {"copyright": "2022 Imagination Technologies Ltd."},
            "platform": self.name,
            "guard": self._gen_guard(),
        })

        for define in self._defines:
            define.emit()

        print()

        for enum in self._enums.values():
            enum.emit()

        for struct in self._structs.values():
            struct.emit(self)

        for stream in self._streams.values():
            stream.emit(self)

        print("#endif /* %s */" % self._gen_guard())

    def is_known_struct(self, struct_name: str) -> bool:
        return struct_name in self._structs.keys()

    def is_known_enum(self, enum_name: str) -> bool:
        return enum_name in self._enums.keys()

    def get_enum(self, enum_name: str) -> Enum:
        return self._enums[enum_name]

    def get_struct(self, struct_name: str) -> Struct:
        return self._structs[struct_name]


class Enum(Node):
    __slots__ = ["_values"]

    _values: t.Dict[str, Value]

    def __init__(self, parent: Node, name: str) -> None:
        super().__init__(parent, name)

        self._values = {}

        self.parent.add(self)

    # We override prefix so that the values will contain the enum's name too.
    @property
    def prefix(self) -> str:
        return self.full_name

    def get_value(self, value_name: str) -> Value:
        return self._values[value_name]

    def add(self, element: Node) -> None:
        if not isinstance(element, Value):
            super().add(element)

        if element.name in self._values:
            raise RuntimeError("Value is being redefined. Value: '%s'" % element.name)

        if element.value in self._values.values():
            raise RuntimeError("Ambiguous enum value detected. Value: '%s'" % element.value)

        self._values[element.name] = element

    def _emit_to_str(self) -> None:
        print(textwrap.dedent("""\
            static const char *
            %s_to_str(const enum %s value)
            {""") % (self.full_name, self.full_name))

        print("    switch (value) {")
        for value in self._values.values():
            print("    case %s: return \"%s\";" % (value.full_name, value.name))
        print("    default: return NULL;")
        print("    }")

        print("}\n")

    def emit(self) -> None:
        # This check is invalid if tags other than Value can be nested within an enum.
        if not self._values.values():
            raise RuntimeError("Enum definition is empty. Enum: '%s'" % self.full_name)

        print("enum %s {" % self.full_name)
        for value in self._values.values():
            value.emit()
        print("};\n")

        self._emit_to_str()


class Value(Node):
    __slots__ = ["value"]

    value: int

    def __init__(self, parent: Node, name: str, value: int) -> None:
        super().__init__(parent, name)

        self.value = value

        self.parent.add(self)

    def emit(self):
        print("    %-36s = %6d," % (self.full_name, self.value))


class Struct(Node):
    __slots__ = ["length", "size", "_children"]

    length: int
    size: int
    _children: t.Dict[str, t.Union[Condition, Field]]

    def __init__(self, parent: Node, name: str, length: int) -> None:
        super().__init__(parent, name)

        self.length = length
        self.size = self.length * 32

        if self.length <= 0:
            raise ValueError("Struct length must be greater than 0. Struct: '%s'." % self.full_name)

        self._children = {}

        self.parent.add(self)

    @property
    def fields(self) -> t.List[Field]:
        # TODO: Should we cache? See TODO in equivalent Condition getter.

        fields = []
        for child in self._children.values():
            if isinstance(child, Condition):
                fields += child.fields
            else:
                fields.append(child)

        return fields

    @property
    def prefix(self) -> str:
        return self.full_name

    def add(self, element: Node) -> None:
        # We don't support conditions and field having the same name.
        if isinstance(element, Field):
            if element.name in self._children.keys():
                raise ValueError("Field is being redefined. Field: '%s', Struct: '%s'"
                                 % (element.name, self.full_name))

            self._children[element.name] = element

        elif isinstance(element, Condition):
            # We only save ifs, and ignore the rest. The rest will be linked to
            # the if condition so we just need to call emit() on the if and the
            # rest will also be emitted.
            if element.type == "if":
                self._children[element.name] = element
            else:
                if element.name not in self._children.keys():
                    raise RuntimeError("Unknown condition: '%s'" % element.name)

        else:
            super().add(element)

    def _emit_header(self, root: Csbgen) -> None:
        default_fields = []
        for field in (f for f in self.fields if f.default is not None):
            if field.is_builtin_type:
                default_fields.append("    .%-35s = %6d" % (field.name, field.default))
            else:
                if not root.is_known_enum(field.type):
                    # Default values should not apply to structures
                    raise RuntimeError(
                        "Unknown type. Field: '%s' Type: '%s'"
                        % (field.name, field.type)
                    )

                enum = root.get_enum(field.type)

                try:
                    value = enum.get_value(field.default)
                except KeyError:
                    raise ValueError("Unknown enum value. Value: '%s', Enum: '%s', Field: '%s'"
                                     % (field.default, enum.full_name, field.name))

                default_fields.append("    .%-35s = %s" % (field.name, value.full_name))

        print("#define %-40s\\" % (self.full_name + "_header"))
        print(",  \\\n".join(default_fields))
        print("")

    def _emit_helper_macros(self) -> None:
        for field in (f for f in self.fields if f.defines):
            print("/* Helper macros for %s */" % field.name)

            for define in field.defines:
                define.emit()

            print()

    def _emit_pack_function(self, root: Csbgen) -> None:
        print(textwrap.dedent("""\
            static inline __attribute__((always_inline)) void
            %s_pack(__attribute__((unused)) void * restrict dst,
                  %s__attribute__((unused)) const struct %s * restrict values)
            {""") % (self.full_name, ' ' * len(self.full_name), self.full_name))

        group = Group(0, 1, self.size, self.fields)
        dwords, length = group.collect_dwords_and_length()
        if length:
            # Cast dst to make header C++ friendly
            print("    uint32_t * restrict dw = (uint32_t * restrict) dst;")

        group.emit_pack_function(root, dwords, length)

        print("}\n")

    def _emit_unpack_function(self, root: Csbgen) -> None:
        print(textwrap.dedent("""\
            static inline __attribute__((always_inline)) void
            %s_unpack(__attribute__((unused)) const void * restrict src,
                    %s__attribute__((unused)) struct %s * restrict values)
            {""") % (self.full_name, ' ' * len(self.full_name), self.full_name))

        group = Group(0, 1, self.size, self.fields)
        dwords, length = group.collect_dwords_and_length()
        if length:
            # Cast src to make header C++ friendly
            print("    const uint32_t * restrict dw = (const uint32_t * restrict) src;")

        group.emit_unpack_function(root, dwords, length)

        print("}\n")

    def emit(self, root: Csbgen) -> None:
        print("#define %-33s %6d" % (self.full_name + "_length", self.length))

        self._emit_header(root)

        self._emit_helper_macros()

        print("struct %s {" % self.full_name)
        for child in self._children.values():
            child.emit(root)
        print("};\n")

        self._emit_pack_function(root)
        self._emit_unpack_function(root)


class Stream(Node):
    __slots__ = ["length", "size", "_children"]

    length: int
    size: int
    _children: t.Dict[str, t.Union[Condition, Field]]

    def __init__(self, parent: Node, name: str, length: int) -> None:
        self._children = {}

        super().__init__(parent, name)

    @property
    def fields(self) -> t.List[Field]:
        fields = []

    @property
    def prefix(self) -> str:
        return self.full_name

    def add(self, element: Node) -> None:
        # We don't support conditions and field having the same name.
        if isinstance(element, Field):
            if element.name in self._children.keys():
                raise ValueError("Field is being redefined. Field: '%s', Struct: '%s'"
                                 % (element.name, self.full_name))

            self._children[element.name] = element

        elif isinstance(element, Condition):
            # We only save ifs, and ignore the rest. The rest will be linked to
            # the if condition so we just need to call emit() on the if and the
            # rest will also be emitted.
            if element.type == "if":
                self._children[element.name] = element
            else:
                if element.name not in self._children.keys():
                    raise RuntimeError("Unknown condition: '%s'" % element.name)

        else:
            super().add(element)

    def _emit_header(self, root: Csbgen) -> None:
        pass

    def _emit_helper_macros(self) -> None:
        pass

    def _emit_pack_function(self, root: Csbgen) -> None:
        pass

    def _emit_unpack_function(self, root: Csbgen) -> None:
        pass

    def emit(self, root: Csbgen) -> None:
        pass

class Field(Node):
    __slots__ = ["start", "end", "type", "default", "shift", "_defines"]

    start: int
    end: int
    type: str
    default: t.Optional[t.Union[str, int]]
    shift: t.Optional[int]
    _defines: t.Dict[str, Define]

    def __init__(self, parent: Node, name: str, start: int, end: int, ty: str, *,
                 default: t.Optional[str] = None, shift: t.Optional[int] = None) -> None:
        super().__init__(parent, name)

        self.start = start
        self.end = end
        self.type = ty

        self._defines = {}

        self.parent.add(self)

        if self.start > self.end:
            raise ValueError("Start cannot be after end. Start: %d, End: %d, Field: '%s'"
                             % (self.start, self.end, self.name))

        if self.type == "bool" and self.end != self.start:
            raise ValueError("Bool field can only be 1 bit long. Field '%s'" % self.name)

        if default is not None:
            if not self.is_builtin_type:
                # Assuming it's an enum type.
                self.default = safe_name(default)
            else:
                self.default = num_from_str(default)
        else:
            self.default = None

        if shift is not None:
            if self.type != "address":
                raise RuntimeError("Only address fields can have a shift attribute. Field: '%s'" % self.name)

            self.shift = int(shift)

            Define(self, "ALIGNMENT", 2**self.shift)
        else:
            if self.type == "address":
                raise RuntimeError("Field of address type requires a shift attribute. Field '%s'" % self.name)

            self.shift = None

    @property
    def defines(self) -> t.Iterator[Define]:
        return self._defines.values()

    # We override prefix so that the defines will contain the field's name too.
    @property
    def prefix(self) -> str:
        return self.full_name

    @property
    def is_builtin_type(self) -> bool:
        builtins = {"address", "bool", "float", "mbo", "offset", "int", "uint"}
        return self.type in builtins

    def _get_c_type(self, root: Csbgen) -> str:
        if self.type == "address":
            return "__pvr_address_type"
        elif self.type == "bool":
            return "bool"
        elif self.type == "float":
            return "float"
        elif self.type == "offset":
            return "uint64_t"
        elif self.type == "int":
            return "int32_t"
        elif self.type == "uint":
            if self.end - self.start <= 32:
                return "uint32_t"
            elif self.end - self.start <= 64:
                return "uint64_t"

            raise RuntimeError("No known C type found to hold %d bit sized value. Field: '%s'"
                               % (self.end - self.start, self.name))
        elif self.type == "uint_array":
            return "uint8_t"
        elif root.is_known_struct(self.type):
            return "struct " + self.type
        elif root.is_known_enum(self.type):
            return "enum " + root.get_enum(self.type).full_name
        raise RuntimeError("Unknown type. Type: '%s', Field: '%s'" % (self.type, self.name))

    def add(self, element: Node) -> None:
        if self.type == "mbo":
            raise RuntimeError("No element can be nested in an mbo field. Element Type: %s, Field: %s"
                               % (type(element).__name__, self.name))

        if isinstance(element, Define):
            if element.name in self._defines:
                raise RuntimeError("Duplicate define. Define: '%s'" % element.name)

            self._defines[element.name] = element
        else:
            super().add(element)

    def emit(self, root: Csbgen) -> None:
        if self.type == "mbo":
            return

        if self.type == "uint_array":
            print("    %-36s %s[%u];" % (self._get_c_type(root), self.name, (self.end - self.start) / 8))
        else:
            print("    %-36s %s;" % (self._get_c_type(root), self.name))


class Define(Node):
    __slots__ = ["value"]

    value: int

    def __init__(self, parent: Node, name: str, value: int) -> None:
        super().__init__(parent, name)

        self.value = value

        self.parent.add(self)

    def emit(self) -> None:
        print("#define %-40s %d" % (self.full_name, self.value))


class Condition(Node):
    __slots__ = ["type", "_children", "_child_branch"]

    type: str
    _children: t.Dict[str, t.Union[Condition, Field]]
    _child_branch: t.Optional[Condition]

    def __init__(self, parent: Node, name: str, ty: str) -> None:
        super().__init__(parent, name, name_is_safe=True)

        self.type = ty
        if not Condition._is_valid_type(self.type):
            raise RuntimeError("Unknown type: '%s'" % self.name)

        self._children = {}

        # This is the link to the next branch for the if statement so either
        # elif, else, or endif. They themselves will also have a link to the
        # next branch up until endif which terminates the chain.
        self._child_branch = None

        self.parent.add(self)

    @property
    def fields(self) -> t.List[Field]:
        # TODO: Should we use some kind of state to indicate the all of the
        # child nodes have been added and then cache the fields in here on the
        # first call so that we don't have to traverse them again per each call?
        # The state could be changed wither when we reach the endif and pop from
        # the context, or when we start emitting.

        fields = []

        for child in self._children.values():
            if isinstance(child, Condition):
                fields += child.fields
            else:
                fields.append(child)

        if self._child_branch is not None:
            fields += self._child_branch.fields

        return fields

    @staticmethod
    def _is_valid_type(ty: str) -> bool:
        types = {"if", "elif", "else", "endif"}
        return ty in types

    def _is_compatible_child_branch(self, branch):
        types = ["if", "elif", "else", "endif"]
        idx = types.index(self.type)
        return (branch.type in types[idx + 1:] or
                self.type == "elif" and branch.type == "elif")

    def _add_branch(self, branch: Condition) -> None:
        if branch.type == "elif" and branch.name == self.name:
            raise RuntimeError("Elif branch cannot have same check as previous branch. Check: '%s'" % branch.name)

        if not self._is_compatible_child_branch(branch):
            raise RuntimeError("Invalid branch. Check: '%s', Type: '%s'" % (branch.name, branch.type))

        self._child_branch = branch

    # Returns the name of the if condition. This is used for elif branches since
    # they have a different name than the if condition thus we have to traverse
    # the chain of branches.
    # This is used to discriminate nested if conditions from branches since
    # branches like 'endif' and 'else' will have the same name as the 'if' (the
    # elif is an exception) while nested conditions will have different names.
    #
    # TODO: Redo this to improve speed? Would caching this be helpful? We could
    # just save the name of the if instead of having to walk towards it whenever
    # a new condition is being added.
    def _top_branch_name(self) -> str:
        if self.type == "if":
            return self.name

        # If we're not an 'if' condition, our parent must be another condition.
        assert isinstance(self.parent, Condition)
        return self.parent._top_branch_name()

    def add(self, element: Node) -> None:
        if isinstance(element, Field):
            if element.name in self._children.keys():
                raise ValueError("Duplicate field. Field: '%s'" % element.name)

            self._children[element.name] = element
        elif isinstance(element, Condition):
            if element.type == "elif" or self._top_branch_name() == element.name:
                self._add_branch(element)
            else:
                if element.type != "if":
                    raise RuntimeError("Branch of an unopened if condition. Check: '%s', Type: '%s'."
                                       % (element.name, element.type))

                # This is a nested condition and we made sure that the name
                # doesn't match _top_branch_name() so we can recognize the else
                # and endif.
                # We recognized the elif by its type however its name differs
                # from the if condition thus when we add an if condition with
                # the same name as the elif nested in it, the _top_branch_name()
                # check doesn't hold true as the name matched the elif and not
                # the if statement which the elif was a branch of, thus the
                # nested if condition is not recognized as an invalid branch of
                # the outer if statement.
                #   Sample:
                #   <condition type="if" check="ROGUEXE"/>
                #       <condition type="elif" check="COMPUTE"/>
                #           <condition type="if" check="COMPUTE"/>
                #           <condition type="endif" check="COMPUTE"/>
                #       <condition type="endif" check="COMPUTE"/>
                #   <condition type="endif" check="ROGUEXE"/>
                #
                # We fix this by checking the if condition name against its
                # parent.
                if element.name == self.name:
                    raise RuntimeError("Invalid if condition. Check: '%s'" % element.name)

                self._children[element.name] = element
        else:
            super().add(element)

    def emit(self, root: Csbgen) -> None:
        if self.type == "if":
            print("/* if %s is supported use: */" % self.name)
        elif self.type == "elif":
            print("/* else if %s is supported use: */" % self.name)
        elif self.type == "else":
            print("/* else %s is not-supported use: */" % self.name)
        elif self.type == "endif":
            print("/* endif %s */" % self.name)
            return
        else:
            raise RuntimeError("Unknown condition type. Implementation error.")

        for child in self._children.values():
            child.emit(root)

        self._child_branch.emit(root)


class Group:
    __slots__ = ["start", "count", "size", "fields"]

    start: int
    count: int
    size: int
    fields: t.List[Field]

    def __init__(self, start: int, count: int, size: int, fields) -> None:
        self.start = start
        self.count = count
        self.size = size
        self.fields = fields

    class DWord:
        __slots__ = ["size", "fields", "addresses"]

        size: int
        fields: t.List[Field]
        addresses: t.List[Field]

        def __init__(self) -> None:
            self.size = 32
            self.fields = []
            self.addresses = []

    def collect_dwords(self, dwords: t.Dict[int, Group.DWord], start: int) -> None:
        for field in self.fields:
            index = (start + field.start) // 32
            if index not in dwords:
                dwords[index] = self.DWord()

            clone = copy.copy(field)
            clone.start = clone.start + start
            clone.end = clone.end + start
            dwords[index].fields.append(clone)

            if field.type == "address":
                # assert dwords[index].address == None
                dwords[index].addresses.append(clone)

            # Coalesce all the dwords covered by this field. The two cases we
            # handle are where multiple fields are in a 64 bit word (typically
            # and address and a few bits) or where a single struct field
            # completely covers multiple dwords.
            while index < (start + field.end) // 32:
                if index + 1 in dwords and not dwords[index] == dwords[index + 1]:
                    dwords[index].fields.extend(dwords[index + 1].fields)
                    dwords[index].addresses.extend(dwords[index + 1].addresses)
                dwords[index].size = 64
                dwords[index + 1] = dwords[index]
                index = index + 1

    def collect_dwords_and_length(self) -> t.Tuple[t.Dict[int, Group.DWord], int]:
        dwords = {}
        self.collect_dwords(dwords, 0)

        # Determine number of dwords in this group. If we have a size, use
        # that, since that'll account for MBZ dwords at the end of a group
        # (like dword 8 on BDW+ 3DSTATE_HS). Otherwise, use the largest dword
        # index we've seen plus one.
        if self.size > 0:
            length = self.size // 32
        elif dwords:
            length = max(dwords.keys()) + 1
        else:
            length = 0

        return dwords, length

    def emit_pack_function(self, root: Csbgen, dwords: t.Dict[int, Group.DWord], length: int) -> None:
        for index in range(length):
            # Handle MBZ dwords
            if index not in dwords:
                print("")
                print("    dw[%d] = 0;" % index)
                continue

            # For 64 bit dwords, we aliased the two dword entries in the dword
            # dict it occupies. Now that we're emitting the pack function,
            # skip the duplicate entries.
            dw = dwords[index]
            if index > 0 and index - 1 in dwords and dw == dwords[index - 1]:
                continue

            # Special case: only one field and it's a struct at the beginning
            # of the dword. In this case we pack directly into the
            # destination. This is the only way we handle embedded structs
            # larger than 32 bits.
            if len(dw.fields) == 1:
                field = dw.fields[0]
                if root.is_known_struct(field.type) and field.start % 32 == 0:
                    print("")
                    print("    %s_pack(data, &dw[%d], &values->%s);"
                          % (self.parser.gen_prefix(safe_name(field.type)), index, field.name))
                    continue

            # Pack any fields of struct type first so we have integer values
            # to the dword for those fields.
            field_index = 0
            for field in dw.fields:
                if root.is_known_struct(field.type):
                    print("")
                    print("    uint32_t v%d_%d;" % (index, field_index))
                    print("    %s_pack(data, &v%d_%d, &values->%s);"
                          % (self.parser.gen_prefix(safe_name(field.type)), index, field_index, field.name))
                    field_index = field_index + 1

            print("")
            dword_start = index * 32
            address_count = len(dw.addresses)

            if dw.size == 32 and not dw.addresses:
                v = None
                print("    dw[%d] =" % index)
            elif len(dw.fields) > address_count:
                v = "v%d" % index
                print("    const uint%d_t %s =" % (dw.size, v))
            else:
                v = "0"

            field_index = 0
            non_address_fields = []
            for field in dw.fields:
                if field.type == "mbo":
                    non_address_fields.append("__pvr_mbo(%d, %d)"
                                              % (field.start - dword_start, field.end - dword_start))
                elif field.type == "address":
                    pass
                elif field.type == "uint":
                    non_address_fields.append("__pvr_uint(values->%s, %d, %d)"
                                              % (field.name, field.start - dword_start, field.end - dword_start))
                elif root.is_known_enum(field.type):
                    non_address_fields.append("__pvr_uint(values->%s, %d, %d)"
                                              % (field.name, field.start - dword_start, field.end - dword_start))
                elif field.type == "int":
                    non_address_fields.append("__pvr_sint(values->%s, %d, %d)"
                                              % (field.name, field.start - dword_start, field.end - dword_start))
                elif field.type == "bool":
                    non_address_fields.append("__pvr_uint(values->%s, %d, %d)"
                                              % (field.name, field.start - dword_start, field.end - dword_start))
                elif field.type == "float":
                    non_address_fields.append("__pvr_float(values->%s)" % field.name)
                elif field.type == "offset":
                    non_address_fields.append("__pvr_offset(values->%s, %d, %d)"
                                              % (field.name, field.start - dword_start, field.end - dword_start))
                elif field.type == "uint_array":
                    pass
                elif field.is_struct_type():
                    non_address_fields.append("__pvr_uint(v%d_%d, %d, %d)"
                                              % (index, field_index, field.start - dword_start,
                                                 field.end - dword_start))
                    field_index = field_index + 1
                else:
                    non_address_fields.append(
                        "/* unhandled field %s," " type %s */\n" % (field.name, field.type)
                    )

            if non_address_fields:
                print(" |\n".join("      " + f for f in non_address_fields) + ";")

            if dw.size == 32:
                for addr in dw.addresses:
                    print("    dw[%d] = __pvr_address(values->%s, %d, %d, %d) | %s;"
                          % (index, addr.name, addr.shift, addr.start - dword_start,
                             addr.end - dword_start, v))
                continue

            v_accumulated_addr = ""
            for i, addr in enumerate(dw.addresses):
                v_address = "v%d_address" % i
                v_accumulated_addr += "v%d_address" % i
                print("    const uint64_t %s =" % v_address)
                print("      __pvr_address(values->%s, %d, %d, %d);"
                      % (addr.name, addr.shift, addr.start - dword_start, addr.end - dword_start))
                if i < (address_count - 1):
                    v_accumulated_addr += " |\n            "

            if dw.addresses:
                if len(dw.fields) > address_count:
                    print("    dw[%d] = %s | %s;" % (index, v_accumulated_addr, v))
                    print("    dw[%d] = (%s >> 32) | (%s >> 32);" % (index + 1, v_accumulated_addr, v))
                    continue
                else:
                    v = v_accumulated_addr

            print("    dw[%d] = %s;" % (index, v))
            print("    dw[%d] = %s >> 32;" % (index + 1, v))

    def emit_unpack_function(self, root: Csbgen, dwords: t.Dict[int, Group.DWord], length: int) -> None:
        for index in range(length):
            # Ignore MBZ dwords
            if index not in dwords:
                continue

            # For 64 bit dwords, we aliased the two dword entries in the dword
            # dict it occupies. Now that we're emitting the unpack function,
            # skip the duplicate entries.
            dw = dwords[index]
            if index > 0 and index - 1 in dwords and dw == dwords[index - 1]:
                continue

            # Special case: only one field and it's a struct at the beginning
            # of the dword. In this case we unpack directly from the
            # source. This is the only way we handle embedded structs
            # larger than 32 bits.
            if len(dw.fields) == 1:
                field = dw.fields[0]
                if root.is_known_struct(field.type) and field.start % 32 == 0:
                    prefix = root.get_struct(field.type)
                    print("")
                    print("    %s_unpack(data, &dw[%d], &values->%s);" % (prefix, index, field.name))
                    continue

            dword_start = index * 32

            if dw.size == 32:
                v = "dw[%d]" % index
            elif dw.size == 64:
                v = "v%d" % index
                print("    const uint%d_t %s = dw[%d] | ((uint64_t)dw[%d] << 32);" % (dw.size, v, index, index + 1))
            else:
                raise RuntimeError("Unsupported dword size %d" % dw.size)

            # Unpack any fields of struct type first.
            for field_index, field in enumerate(f for f in dw.fields if root.is_known_struct(f.type)):
                prefix = root.get_struct(field.type).prefix
                vname = "v%d_%d" % (index, field_index)
                print("")
                print("    uint32_t %s = __pvr_uint_unpack(%s, %d, %d);"
                      % (vname, v, field.start - dword_start, field.end - dword_start))
                print("    %s_unpack(data, &%s, &values->%s);" % (prefix, vname, field.name))

            for field in dw.fields:
                dword_field_start = field.start - dword_start
                dword_field_end = field.end - dword_start

                if field.type == "mbo" or root.is_known_struct(field.type):
                    continue
                elif field.type == "uint" or root.is_known_enum(field.type) or field.type == "bool":
                    print("    values->%s = __pvr_uint_unpack(%s, %d, %d);"
                          % (field.name, v, dword_field_start, dword_field_end))
                elif field.type == "int":
                    print("    values->%s = __pvr_sint_unpack(%s, %d, %d);"
                          % (field.name, v, dword_field_start, dword_field_end))
                elif field.type == "float":
                    print("    values->%s = __pvr_float_unpack(%s);" % (field.name, v))
                elif field.type == "offset":
                    print("    values->%s = __pvr_offset_unpack(%s, %d, %d);"
                          % (field.name, v, dword_field_start, dword_field_end))
                elif field.type == "address":
                    print("    values->%s = __pvr_address_unpack(%s, %d, %d, %d);"
                          % (field.name, v, field.shift, dword_field_start, dword_field_end))
                else:
                    print("/* unhandled field %s, type %s */" % (field.name, field.type))



class Parser:
    __slots__ = ["parser", "context", "filename"]

    parser: expat.XMLParserType
    context: t.List[Node]
    filename: str

    def __init__(self) -> None:
        self.parser = expat.ParserCreate()
        self.parser.StartElementHandler = self.start_element
        self.parser.EndElementHandler = self.end_element

        self.context = []
        self.filename = ""

    def start_element(self, name: str, attrs: t.Dict[str, str]) -> None:
        if name == "csbgen":
            if self.context:
                raise RuntimeError(
                    "Can only have 1 csbgen block and it has "
                    + "to contain all of the other elements."
                )

            csbgen = Csbgen(attrs["name"], attrs["prefix"], self.filename)
            self.context.append(csbgen)
            return

        parent = self.context[-1]

        if name == "struct":
            struct = Struct(parent, attrs["name"], int(attrs["length"]))
            self.context.append(struct)

        elif name == "stream":
            stream = Stream(parent, attrs["name"], int(attrs["length"]))
            self.context.append(stream)

        elif name == "field":
            default = None
            if "default" in attrs.keys():
                default = attrs["default"]

            shift = None
            if "shift" in attrs.keys():
                shift = attrs["shift"]

            if "start" in attrs.keys():
                if ":" in str(attrs["start"]):
                    (word, bit) = attrs["start"].split(":")
                    start = (int(word) * 32) + int(bit)
                else:
                    start = int(attrs["start"])
            else:
                element = self.context[-1]
                if isinstance(element, Stream):
                    start = 0
                else:
                    raise RuntimeError("Field requires start attribute outside of stream.")

            if "size" in attrs.keys():
                end = start + int(attrs["size"])
            else:
                end = int(attrs["end"])

            field = Field(parent, name=attrs["name"], start=start, end=end, ty=attrs["type"],
                          default=default, shift=shift)
            self.context.append(field)

        elif name == "enum":
            enum = Enum(parent, attrs["name"])
            self.context.append(enum)

        elif name == "value":
            value = Value(parent, attrs["name"], int(literal_eval(attrs["value"])))
            self.context.append(value)

        elif name == "define":
            define = Define(parent, attrs["name"], int(literal_eval(attrs["value"])))
            self.context.append(define)

        elif name == "condition":
            condition = Condition(parent, name=attrs["check"], ty=attrs["type"])

            # Starting with the if statement we push it in the context. For each
            # branch following (elif, and else) we assign the top of stack as
            # its parent, pop() and push the new condition. So per branch we end
            # up having [..., struct, condition]. We don't push an endif since
            # it's not supposed to have any children and it's supposed to close
            # the whole if statement.

            if condition.type != "if":
                # Remove the parent condition from the context. We were peeking
                # before, now we pop().
                self.context.pop()

            if condition.type == "endif":
                if not isinstance(parent, Condition):
                    raise RuntimeError("Cannot close unopened or already closed condition. Condition: '%s'"
                                       % condition.name)
            else:
                self.context.append(condition)

        else:
            raise RuntimeError("Unknown tag: '%s'" % name)

    def end_element(self, name: str) -> None:
        if name == "condition":
            element = self.context[-1]
            if not isinstance(element, Condition) and not isinstance(element, Struct):
                raise RuntimeError("Expected condition or struct tag to be closed.")

            return

        element = self.context.pop()

        if name == "struct":
            if not isinstance(element, Struct):
                raise RuntimeError("Expected struct tag to be closed.")
        elif name == "stream":
            if not isinstance(element, Stream):
                raise RuntimeError("Expected stream tag to be closed.")
        elif name == "field":
            if not isinstance(element, Field):
                raise RuntimeError("Expected field tag to be closed.")
        elif name == "enum":
            if not isinstance(element, Enum):
                raise RuntimeError("Expected enum tag to be closed.")
        elif name == "value":
            if not isinstance(element, Value):
                raise RuntimeError("Expected value tag to be closed.")
        elif name == "define":
            if not isinstance(element, Define):
                raise RuntimeError("Expected define tag to be closed.")
        elif name == "csbgen":
            if not isinstance(element, Csbgen):
                raise RuntimeError("Expected csbgen tag to be closed.\nSome tags may have not been closed")

            element.emit()
        else:
            raise RuntimeError("Unknown closing element: '%s'" % name)

    def parse(self, filename: str) -> None:
        file = open(filename, "rb")
        self.filename = filename
        self.parser.ParseFile(file)
        file.close()


if __name__ == "__main__":
    import sys

    if len(sys.argv) < 2:
        print("No input xml file specified")
        sys.exit(1)

    input_file = sys.argv[1]

    p = Parser()
    p.parse(input_file)
