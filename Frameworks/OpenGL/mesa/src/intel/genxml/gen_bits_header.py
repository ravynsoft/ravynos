#encoding=utf-8
# Copyright © 2017 Intel Corporation

# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:

# The above copyright notice and this permission notice shall be included in
# all copies or substantial portions of the Software.

# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
# SOFTWARE.

import argparse
import intel_genxml
import os

from mako.template import Template
from util import *

TEMPLATE = Template("""\
<%!
from operator import itemgetter
%>\
/*
 * Copyright © 2017 Intel Corporation
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the next
 * paragraph) shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 */

/* THIS FILE HAS BEEN GENERATED, DO NOT HAND EDIT.
 *
 * Sizes of bitfields in genxml instructions, structures, and registers.
 */

#ifndef ${guard}
#define ${guard}

#include <stdint.h>

#include "dev/intel_device_info.h"
#include "util/macros.h"

<%def name="emit_per_gen_prop_func(item, prop, protect_defines)">
%if item.has_prop(prop):
% for gen, value in sorted(item.iter_prop(prop), reverse=True):
%  if protect_defines:
#ifndef ${gen.prefix(item.token_name)}_${prop}
#define ${gen.prefix(item.token_name)}_${prop}  ${value}
#endif
%  else:
#define ${gen.prefix(item.token_name)}_${prop}  ${value}
%  endif
% endfor

static inline uint32_t ATTRIBUTE_PURE
${item.token_name}_${prop}(const struct intel_device_info *devinfo)
{
   switch (devinfo->verx10) {
   case 200: return ${item.get_prop(prop, 20)};
   case 125: return ${item.get_prop(prop, 12.5)};
   case 120: return ${item.get_prop(prop, 12)};
   case 110: return ${item.get_prop(prop, 11)};
   case 90: return ${item.get_prop(prop, 9)};
   case 80: return ${item.get_prop(prop, 8)};
   case 75: return ${item.get_prop(prop, 7.5)};
   case 70: return ${item.get_prop(prop, 7)};
   case 60: return ${item.get_prop(prop, 6)};
   case 50: return ${item.get_prop(prop, 5)};
   case 45: return ${item.get_prop(prop, 4.5)};
   case 40: return ${item.get_prop(prop, 4)};
   default:
      unreachable("Invalid hardware generation");
   }
}
%endif
</%def>

#ifdef __cplusplus
extern "C" {
#endif
% for _, container in sorted(containers.items(), key=itemgetter(0)):
%  if container.allowed:

/* ${container.name} */

${emit_per_gen_prop_func(container, 'length', True)}

%   for _, field in sorted(container.fields.items(), key=itemgetter(0)):
%    if field.allowed:

/* ${container.name}::${field.name} */

${emit_per_gen_prop_func(field, 'bits', False)}

${emit_per_gen_prop_func(field, 'start', False)}
%    endif
%   endfor
%  endif
% endfor

#ifdef __cplusplus
}
#endif

#endif /* ${guard} */""")

class Gen(object):

    def __init__(self, z):
        # Convert potential "major.minor" string
        self.tenx = int(float(z) * 10)

    def __lt__(self, other):
        return self.tenx < other.tenx

    def __hash__(self):
        return hash(self.tenx)

    def __eq__(self, other):
        return self.tenx == other.tenx

    def prefix(self, token):
        gen = self.tenx

        if gen % 10 == 0:
            gen //= 10

        if token[0] == '_':
            token = token[1:]

        return 'GFX{}_{}'.format(gen, token)

class Container(object):

    def __init__(self, name):
        self.name = name
        self.token_name = safe_name(name)
        self.length_by_gen = {}
        self.fields = {}
        self.allowed = False

    def add_gen(self, gen, xml_attrs):
        assert isinstance(gen, Gen)
        if 'length' in xml_attrs:
            self.length_by_gen[gen] = xml_attrs['length']

    def get_field(self, field_name, create=False):
        key = to_alphanum(field_name)
        if key not in self.fields:
            if create:
                self.fields[key] = Field(self, field_name)
            else:
                return None
        return self.fields[key]

    def has_prop(self, prop):
        if prop == 'length':
            return bool(self.length_by_gen)
        else:
            raise ValueError('Invalid property: "{0}"'.format(prop))

    def iter_prop(self, prop):
        if prop == 'length':
            return self.length_by_gen.items()
        else:
            raise ValueError('Invalid property: "{0}"'.format(prop))

    def get_prop(self, prop, gen):
        if not isinstance(gen, Gen):
            gen = Gen(gen)

        if prop == 'length':
            return self.length_by_gen.get(gen, 0)
        else:
            raise ValueError('Invalid property: "{0}"'.format(prop))

class Field(object):

    def __init__(self, container, name):
        self.name = name
        self.token_name = safe_name('_'.join([container.name, self.name]))
        self.bits_by_gen = {}
        self.start_by_gen = {}
        self.allowed = False

    def add_gen(self, gen, xml_attrs):
        assert isinstance(gen, Gen)
        start = int(xml_attrs['start'])
        end = int(xml_attrs['end'])
        self.start_by_gen[gen] = start
        self.bits_by_gen[gen] = 1 + end - start

    def has_prop(self, prop):
        return True

    def iter_prop(self, prop):
        if prop == 'bits':
            return self.bits_by_gen.items()
        elif prop == 'start':
            return self.start_by_gen.items()
        else:
            raise ValueError('Invalid property: "{0}"'.format(prop))

    def get_prop(self, prop, gen):
        if not isinstance(gen, Gen):
            gen = Gen(gen)

        if prop == 'bits':
            return self.bits_by_gen.get(gen, 0)
        elif prop == 'start':
            return self.start_by_gen.get(gen, 0)
        else:
            raise ValueError('Invalid property: "{0}"'.format(prop))

class XmlParser(object):

    def __init__(self, containers):
        self.gen = None
        self.containers = containers
        self.container_stack = []
        self.container_stack.append(None)

    def emit_genxml(self, genxml):
        root = genxml.et.getroot()
        self.gen = Gen(root.attrib['gen'])
        for item in root:
            self.process_item(item)

    def process_item(self, item):
        name = item.tag
        attrs = item.attrib
        if name in ('instruction', 'struct', 'register'):
            self.start_container(attrs)
            for struct_item in item:
                self.process_item(struct_item)
            self.container_stack.pop()
        elif name == 'group':
            self.container_stack.append(None)
            for group_item in item:
                self.process_item(group_item)
            self.container_stack.pop()
        elif name == 'field':
            self.process_field(attrs)
        elif name in ('enum', 'import'):
            pass
        else:
            assert False

    def start_container(self, attrs):
        assert self.container_stack[-1] is None
        name = attrs['name']
        if name not in self.containers:
            self.containers[name] = Container(name)
        self.container_stack.append(self.containers[name])
        self.container_stack[-1].add_gen(self.gen, attrs)

    def process_field(self, attrs):
        if self.container_stack[-1] is None:
            return

        field_name = attrs.get('name', None)
        if not field_name:
            return

        self.container_stack[-1].get_field(field_name, True).add_gen(self.gen, attrs)

def parse_args():
    p = argparse.ArgumentParser()
    p.add_argument('-o', '--output', type=str,
                   help="If OUTPUT is unset or '-', then it defaults to '/dev/stdout'")
    p.add_argument('--cpp-guard', type=str,
                   help='If unset, then CPP_GUARD is derived from OUTPUT.')
    p.add_argument('--engines', nargs='?', type=str, default='render',
                   help="Comma-separated list of engines whose instructions should be parsed (default: %(default)s)")
    p.add_argument('--include-symbols', type=str, action='store',
                   help='List of instruction/structures to generate',
                   required=True)
    p.add_argument('xml_sources', metavar='XML_SOURCE', nargs='+')

    pargs = p.parse_args()

    if pargs.output in (None, '-'):
        pargs.output = '/dev/stdout'

    if pargs.cpp_guard is None:
        pargs.cpp_guard = os.path.basename(pargs.output).upper().replace('.', '_')

    return pargs

def main():
    pargs = parse_args()

    engines = set(pargs.engines.split(','))
    valid_engines = [ 'render', 'blitter', 'video' ]
    if engines - set(valid_engines):
        print("Invalid engine specified, valid engines are:\n")
        for e in valid_engines:
            print("\t%s" % e)
        sys.exit(1)

    # Maps name => Container
    containers = {}

    for source in pargs.xml_sources:
        p = XmlParser(containers)
        genxml = intel_genxml.GenXml(source)
        genxml.filter_engines(engines)
        genxml.merge_imported()
        p.emit_genxml(genxml)

    included_symbols_list = pargs.include_symbols.split(',')
    for _name_field in included_symbols_list:
        name_field = _name_field.split('::')
        container = containers[name_field[0]]
        container.allowed = True
        if len(name_field) > 1:
            field = container.get_field(name_field[1])
            assert field
            field.allowed = True

    with open(pargs.output, 'w', encoding='utf-8') as f:
        f.write(TEMPLATE.render(containers=containers, guard=pargs.cpp_guard))

if __name__ == '__main__':
    main()
