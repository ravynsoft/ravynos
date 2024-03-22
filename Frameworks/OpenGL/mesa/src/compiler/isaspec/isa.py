#
# Copyright © 2020 Google, Inc.
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

from xml.etree import ElementTree
import os
import re

def dbg(str):
    if False:
        print(str)

class BitSetPattern(object):
    """Class that encapsulated the pattern matching, ie.
       the match/dontcare/mask bitmasks.  The following
       rules should hold

          (match ^ dontcare) == 0
          (match || dontcare) == mask

       For a leaf node, the mask should be (1 << size) - 1
       (ie. all bits set)
    """
    def __init__(self, bitset):
        self.match      = bitset.match
        self.dontcare   = bitset.dontcare
        self.mask       = bitset.mask
        self.field_mask = bitset.field_mask

    def merge(self, pattern):
        p = BitSetPattern(pattern)
        p.match      = p.match      | self.match
        p.dontcare   = p.dontcare   | self.dontcare
        p.mask       = p.mask       | self.mask
        p.field_mask = p.field_mask | self.field_mask
        return p

    def defined_bits(self):
        return self.match | self.dontcare | self.mask | self.field_mask

def get_bitrange(field):
    if 'pos' in field.attrib:
        assert('low' not in field.attrib)
        assert('high' not in field.attrib)
        low = int(field.attrib['pos'])
        high = low
    else:
        low = int(field.attrib['low'])
        high = int(field.attrib['high'])
    assert low <= high
    return low, high

def extract_pattern(xml, name, is_defined_bits=None):
    low, high = get_bitrange(xml)
    mask = ((1 << (1 + high - low)) - 1) << low

    patstr = xml.text.strip()

    assert (len(patstr) == (1 + high - low)), "Invalid {} length in {}: {}..{}".format(xml.tag, name, low, high)
    if is_defined_bits is not None:
        assert not is_defined_bits(mask), "Redefined bits in {} {}: {}..{}".format(xml.tag, name, low, high)

    match = 0
    dontcare = 0

    for n in range(0, len(patstr)):
        match = match << 1
        dontcare = dontcare << 1
        if patstr[n] == '1':
            match |= 1
        elif patstr[n] == 'x':
            dontcare |= 1
        elif patstr[n] != '0':
            assert 0, "Invalid {} character in {}: {}".format(xml.tag, name, patstr[n])

    dbg("{}: {}.{} => {:016x} / {:016x} / {:016x}".format(xml.tag, name, patstr, match << low, dontcare << low, mask))

    return match << low, dontcare << low, mask

def get_c_name(name):
    return name.lower().replace('#', '__').replace('-', '_').replace('.', '_')

class BitSetField(object):
    """Class that encapsulates a field defined in a bitset
    """
    def __init__(self, isa, xml):
        self.isa = isa
        self.low, self.high = get_bitrange(xml)
        self.name = xml.attrib['name']
        self.type = xml.attrib['type']
        self.params = []
        for param in xml.findall('param'):
            aas = name = param.attrib['name']
            if 'as' in param.attrib:
                aas = param.attrib['as']
            self.params.append([name, aas])
        self.expr = None
        self.display = None
        self.call = 'call' in xml.attrib and xml.attrib['call'] == 'true'
        if 'display' in xml.attrib:
            self.display = xml.attrib['display'].strip()

    def get_c_name(self):
        return get_c_name(self.name)

    def get_c_typename(self):
        if self.type in self.isa.enums:
            return 'TYPE_ENUM'
        if self.type in self.isa.bitsets:
            return 'TYPE_BITSET'
        return 'TYPE_' + self.type.upper()

    def mask(self):
        return ((1 << self.get_size()) - 1) << self.low

    def get_size(self):
        return 1 + self.high - self.low

class BitSetAssertField(BitSetField):
    """Similar to BitSetField, but for <assert/>s, which can be
       used to specify that a certain bitpattern is expected in
       place of (for example) unused bitfields
    """
    def __init__(self, case, xml):
        self.isa = case.bitset.isa
        self.low, self.high = get_bitrange(xml)
        self.name = case.bitset.name + '#assert' + str(len(case.fields))
        self.type = 'uint'
        self.expr = None
        self.display = None

        match, dontcare, mask = extract_pattern(xml, case.bitset.name)
        self.val = match >> self.low

        assert dontcare == 0, "'x' (dontcare) is not valid in an assert"

    def get_c_typename(self):
        return 'TYPE_ASSERT'

class BitSetDerivedField(BitSetField):
    """Similar to BitSetField, but for derived fields
    """
    def __init__(self, isa, xml):
        self.isa = isa
        self.low = 0
        self.high = 0
        # NOTE: a width should be provided for 'int' derived fields, ie.
        # where sign extension is needed.  We just repurpose the 'high'
        # field for that to make '1 + high - low' work out
        if 'width' in xml.attrib:
            self.high = int(xml.attrib['width']) - 1
        self.name = xml.attrib['name']
        self.type = xml.attrib['type']
        if 'expr' in xml.attrib:
            self.expr = xml.attrib['expr']
        else:
            e = isa.parse_one_expression(xml, self.name)
            self.expr = e.name
        self.display = None
        if 'display' in xml.attrib:
            self.display = xml.attrib['display'].strip()
        self.call = 'call' in xml.attrib and xml.attrib['call'] == 'true'

class BitSetCase(object):
    """Class that encapsulates a single bitset case
    """
    def __init__(self, bitset, xml, update_field_mask, expr=None):
        self.bitset = bitset
        if expr is not None:
            self.name = bitset.name + '#case' + str(len(bitset.cases))
        else:
            self.name = bitset.name + "#default"
        self.expr = expr
        self.fields = {}

        for derived in xml.findall('derived'):
            f = BitSetDerivedField(bitset.isa, derived)
            self.fields[f.name] = f

        for assrt in xml.findall('assert'):
            f = BitSetAssertField(self, assrt)
            update_field_mask(self, f)
            self.fields[f.name] = f

        for field in xml.findall('field'):
            dbg("{}.{}".format(self.name, field.attrib['name']))
            f = BitSetField(bitset.isa, field)
            update_field_mask(self, f)
            self.fields[f.name] = f

        self.display = None
        for d in xml.findall('display'):
            # Allow <display/> for empty display string:
            if d.text is not None:
                self.display = d.text.strip()
            else:
                self.display = ''
            dbg("found display: '{}'".format(self.display))

    def get_c_name(self):
        return get_c_name(self.name)

class BitSetEncode(object):
    """Additional data that may be associated with a root bitset node
       to provide additional information needed to generate helpers
       to encode the bitset, such as source data type and "opcode"
       case prefix (ie. how to choose/enumerate which leaf node bitset
       to use to encode the source data
    """
    def __init__(self, xml):
        self.type = None
        if 'type' in xml.attrib:
            self.type = xml.attrib['type']
        self.case_prefix = None
        if 'case-prefix' in xml.attrib:
            self.case_prefix = xml.attrib['case-prefix']
        # The encode element may also contain mappings from encode src
        # to individual field names:
        self.maps = {}
        self.forced = {}
        for map in xml.findall('map'):
            name = map.attrib['name']
            self.maps[name] = map.text.strip()
            if 'force' in map.attrib and map.attrib['force']  == 'true':
                self.forced[name] = 'true'

class BitSetDecode(object):
    """Additional data that may be associated with a root bitset node
       to provide additional information needed to generate helpers
       to decode the bitset.
    """
    def __init__(self, xml):
        pass

class BitSet(object):
    """Class that encapsulates a single bitset rule
    """
    def __init__(self, isa, xml):
        self.isa = isa
        self.xml = xml
        self.name = xml.attrib['name']
        self.display_name = xml.attrib['displayname'] if 'displayname' in xml.attrib else self.name
        self.meta = {}

        # Used for generated encoder, to de-duplicate encoding for
        # similar instructions:
        self.snippets = {}

        if 'size' in xml.attrib:
            assert('extends' not in xml.attrib)
            self.size = int(xml.attrib['size'])
            self.extends = None
        else:
            self.size = None
            self.extends = xml.attrib['extends']

        self.encode = None
        if xml.find('encode') is not None:
            self.encode = BitSetEncode(xml.find('encode'))
        self.decode = None
        if xml.find('decode') is not None:
            self.decode = BitSetDecode(xml.find('encode'))

        self.gen_min = 0
        self.gen_max = (1 << 32) - 1

        for gen in xml.findall('gen'):
            if 'min' in gen.attrib:
                self.gen_min = int(gen.attrib['min'])
            if 'max' in gen.attrib:
                self.gen_max = int(gen.attrib['max'])

        if xml.find('meta') is not None:
            self.meta = xml.find('meta').attrib

        # Collect up the match/dontcare/mask bitmasks for
        # this bitset case:
        self.match = 0
        self.dontcare = 0
        self.mask = 0
        self.field_mask = 0

        self.cases = []

        # Helper to check for redefined bits:
        def is_defined_bits(m):
            return ((self.field_mask | self.mask | self.dontcare | self.match) & m) != 0

        def update_default_bitmask_field(bs, field):
            m = field.mask()
            dbg("field: {}.{} => {:016x}".format(self.name, field.name, m))
            # For default case, we don't expect any bits to be doubly defined:
            assert not is_defined_bits(m), "Redefined bits in field {}.{}: {}..{}".format(
                self.name, field.name, field.low, field.high)
            self.field_mask |= m

        def update_override_bitmask_field(bs, field):
            m = field.mask()
            dbg("field: {}.{} => {:016x}".format(self.name, field.name, m))
            assert self.field_mask ^ ~m

        dflt = BitSetCase(self, xml, update_default_bitmask_field)

        for override in xml.findall('override'):
            if 'expr' in override.attrib:
                expr = override.attrib['expr']
            else:
                e = isa.parse_one_expression(override, self.name)
                expr = e.name
            c = BitSetCase(self, override, update_override_bitmask_field, expr)
            self.cases.append(c)

        # Default case is expected to be the last one:
        self.cases.append(dflt)

        for pattern in xml.findall('pattern'):
            match, dontcare, mask = extract_pattern(pattern, self.name, is_defined_bits)

            self.match    |= match
            self.dontcare |= dontcare
            self.mask     |= mask

    def get_pattern(self):
        if self.extends is not None:
            parent = self.isa.bitsets[self.extends]
            ppat = parent.get_pattern()
            pat  = BitSetPattern(self)

            assert ((ppat.defined_bits() & pat.defined_bits()) == 0), "bitset conflict in {}: {:x}".format(self.name, (ppat.defined_bits() & pat.defined_bits()))

            return pat.merge(ppat)

        return BitSetPattern(self)

    def get_size(self):
        if self.extends is not None:
            parent = self.isa.bitsets[self.extends]
            return parent.get_size()
        return self.size

    def get_gen_min(self):
        if self.extends is not None:
            parent = self.isa.bitsets[self.extends]

            assert (self.gen_min == 0) or (self.gen_min >= parent.get_gen_min()), "bitset {} should not have min gen lower than the parent's one".format(self.name)

            return max(self.gen_min, parent.get_gen_min())
        return self.gen_min

    def get_gen_max(self):
        if self.extends is not None:
            parent = self.isa.bitsets[self.extends]

            assert (self.gen_max == (1 << 32) - 1) or (self.gen_max <= parent.get_gen_max()), "bitset {} should not have max gen higher than the parent's one".format(self.name)

            return min(self.gen_max, parent.get_gen_max())
        return self.gen_max

    def has_gen_restriction(self):
        return self.gen_min != 0 or self.gen_max != (1 << 32) - 1

    def get_c_name(self):
        return get_c_name(self.name)

    def get_root(self):
        if self.extends is not None:
            return self.isa.bitsets[self.extends].get_root()
        return self

    def get_meta(self):
        meta = {}

        if self.extends is not None:
            meta = self.isa.bitsets[self.extends].get_meta()

        if self.meta:
            meta.update(self.meta)

        return meta

class BitSetTemplate(object):
    """Class that encapsulates a template declaration
    """
    def __init__(self, isa, xml):
        self.isa = isa
        self.name = xml.attrib['name']
        self.display = xml.text.strip()
        dbg("found template '{}: {}'".format(self.name, self.display))

class BitSetEnumValue(object):
    """Class that encapsulates an enum value
    """
    def __init__(self, isa, xml):
        self.isa = isa
        self.displayname = xml.attrib['display']
        self.value = xml.attrib['val']
        self.name = xml.attrib.get('name')

    def __str__(self):
        return self.displayname

    def get_name(self):
        return self.name or self.displayname

    def get_value(self):
        return self.value

class BitSetEnum(object):
    """Class that encapsulates an enum declaration
    """
    def __init__(self, isa, xml):
        self.isa = isa
        self.name = xml.attrib['name']

        # Table mapping value to name
        self.values = {}
        for value in xml.findall('value'):
            v = BitSetEnumValue(self, value)
            self.values[v.get_value()] = v

    def get_c_name(self):
        return 'enum_' + get_c_name(self.name)

class BitSetExpression(object):
    """Class that encapsulates an <expr> declaration
    """
    def __init__(self, isa, xml):
        self.isa = isa
        if 'name' in xml.attrib:
            self.name = xml.attrib['name']
        else:
            self.name = 'anon_' + str(isa.anon_expression_count)
            isa.anon_expression_count = isa.anon_expression_count + 1
        expr = xml.text.strip()
        self.fieldnames = list(set(re.findall(r"{([a-zA-Z0-9_]+)}", expr)))
        self.expr = re.sub(r"{([a-zA-Z0-9_]+)}", r"\1", expr)
        dbg("'{}' -> '{}'".format(expr, self.expr))

    def get_c_name(self):
        return 'expr_' + get_c_name(self.name)

class ISA(object):
    """Class that encapsulates all the parsed bitset rules
    """
    def __init__(self, xmlpath):
        self.base_path = os.path.dirname(xmlpath)

        # Counter used to name inline (anonymous) expressions:
        self.anon_expression_count = 0

        # Table of (globally defined) expressions:
        self.expressions = {}

        # Table of templates:
        self.templates = {}

        # Table of enums:
        self.enums = {}

        # Table of toplevel bitset hierarchies:
        self.roots = {}

        # Table of leaf nodes of bitset hierarchies:
        # Note that there may be multiple leaves for a particular name
        # (distinguished by gen), so the values here are lists.
        self.leafs = {}

        # Table of all non-ambiguous bitsets (i.e. no per-gen ambiguity):
        self.bitsets = {}

        # Max needed bitsize for one instruction
        self.bitsize = 0

        root = ElementTree.parse(xmlpath).getroot()
        self.parse_file(root)
        self.validate_isa()

    def parse_expressions(self, root):
        e = None
        for expr in root.findall('expr'):
            e = BitSetExpression(self, expr)
            self.expressions[e.name] = e
        return e

    def parse_one_expression(self, root, name):
        assert len(root.findall('expr')) == 1, "expected a single expression in: {}".format(name)
        return self.parse_expressions(root)

    def parse_file(self, root):
        # Handle imports up-front:
        for imprt in root.findall('import'):
            p = os.path.join(self.base_path, imprt.attrib['file'])
            self.parse_file(ElementTree.parse(p))

        # Extract expressions:
        self.parse_expressions(root)

        # Extract templates:
        for template in root.findall('template'):
            t = BitSetTemplate(self, template)
            self.templates[t.name] = t

        # Extract enums:
        for enum in root.findall('enum'):
            e = BitSetEnum(self, enum)
            self.enums[e.name] = e

        # Extract bitsets:
        for bitset in root.findall('bitset'):
            b = BitSet(self, bitset)
            if b.size is not None:
                dbg("toplevel: " + b.name)
                self.roots[b.name] = b
                self.bitsize = max(self.bitsize, b.size)
            else:
                dbg("derived: " + b.name)
            self.bitsets[b.name] = b
            self.leafs.setdefault(b.name, []).append(b)

        # Resolve all templates:
        for _, bitset in self.bitsets.items():
            for case in bitset.cases:
                if case.display:
                    case.display = self.resolve_templates(case.display)

    def validate_isa(self):
        # We only support multiples of 32 bits for now
        assert self.bitsize % 32 == 0

        # Do one-time fixups
        # Remove non-leaf nodes from the leafs table:
        for name, bitsets in list(self.leafs.items()):
            for bitset in bitsets:
                if bitset.extends in self.leafs:
                    del self.leafs[bitset.extends]

        # Fix multi-gen leaves in bitsets
        for name, bitsets in self.leafs.items():
            if len(bitsets) == 1:
                continue

            del self.bitsets[name]

        # Validate that all bitset fields have valid types, and in
        # the case of bitset type, the sizes match:
        builtin_types = ['branch', 'absbranch', 'int', 'uint', 'hex', 'offset', 'uoffset', 'float', 'bool', 'bool_inv', 'enum', 'custom']
        for bitset_name, bitset in self.bitsets.items():
            if bitset.extends is not None:
                assert bitset.extends in self.bitsets, "{} extends invalid type: {}".format(
                    bitset_name, bitset.extends)
            for case in bitset.cases:
                for field_name, field in case.fields.items():
                    if field.type == 'float':
                        assert field.get_size() == 32 or field.get_size() == 16

                    if not isinstance(field, BitSetDerivedField):
                        assert field.high < bitset.get_size(), \
                            "{}.{}: invalid bit range: [{}, {}] is not in [{}, {}]".format(
                            bitset_name, field_name, field.low, field.high, 0, bitset.get_size() - 1)

                    if field.type in builtin_types:
                        continue
                    if field.type in self.enums:
                        continue
                    assert field.type in self.bitsets, "{}.{}: invalid type: {}".format(
                        bitset_name, field_name, field.type)
                    bs = self.bitsets[field.type]
                    assert field.get_size() == bs.get_size(), "{}.{}: invalid size: {} vs {}".format(
                        bitset_name, field_name, field.get_size(), bs.get_size())

        # Validate that all the leaf node bitsets have no remaining
        # undefined bits
        for name, bitsets in self.leafs.items():
            for bitset in bitsets:
                pat = bitset.get_pattern()
                sz  = bitset.get_size()
                assert ((pat.mask | pat.field_mask) == (1 << sz) - 1), "leaf bitset {} has undefined bits: {:x}".format(
                    bitset.name, ~(pat.mask | pat.field_mask) & ((1 << sz) - 1))

        # TODO somehow validating that only one bitset in a hierarchy
        # matches any given bit pattern would be useful.

        # TODO we should probably be able to look at the contexts where
        # an expression is evaluated and verify that it doesn't have any
        # {VARNAME} references that would be unresolved at evaluation time

    def format(self):
        ''' Generate format string used by printf(..) and friends '''
        parts = []
        words = self.bitsize / 32

        for i in range(int(words)):
            parts.append('%08x')

        fmt = ''.join(parts)

        return f"\"{fmt[1:]}\""

    def value(self):
        ''' Generate format values used by printf(..) and friends '''
        parts = []
        words = self.bitsize / 32

        for i in range(int(words) - 1, -1, -1):
            parts.append('v[' + str(i) + ']')

        return ', '.join(parts)

    def split_bits(self, value, bitsize):
        ''' Split `value` into a list of bitsize-bit integers '''
        mask, parts = (1 << bitsize) - 1, []
        words = self.bitsize / bitsize

        while value:
            parts.append(hex(value & mask))
            value >>= bitsize

        # Add 'missing' words
        while len(parts) < words:
            parts.append('0x0')

        return parts

    # Returns all bitsets in the ISA, including all per-gen variants, in
    # (name, bitset) pairs.
    def all_bitsets(self):
        for name, bitset in self.bitsets.items():
            yield name, bitset
        for name, bitsets in self.leafs.items():
            if len(bitsets) == 1:
                continue
            for bitset in bitsets:
                yield name, bitset

    def resolve_templates(self, display_string):
        matches = re.findall(r'\{([^\}]+)\}', display_string)
        for m in matches:
            if m in self.templates:
                display_string = display_string.replace("{" + m + "}", self.templates[m].display)

        return display_string

    def instructions(self):
        instr = []

        for _, root in self.roots.items():
            for _, leafs in self.leafs.items():
                for leaf in leafs:
                    if leaf.get_root() == root:
                        if not leaf.get_c_name().startswith('__'):
                            instr.append(leaf)

        return instr
