
# Copyright (C) 2012 Intel Corporation
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

# marshal_XML.py: factory for interpreting XML for the purpose of
# building thread marshalling code.

import gl_XML

# We decrease the type size when it's safe, such as when the maximum value
# and all greater values are invalid.
def get_marshal_type(func_name, param):
    type = param.type_string()

    if type == 'GLenum':
        return 'GLenum16' # clamped to 0xffff (always invalid enum)

    # Use int16_t for the vertex stride, the max value is usually 2048.
    if ((type, param.name) == ('GLsizei', 'stride') and
        ('Vertex' in func_name or 'Pointer' in func_name or 'Interleaved' in func_name)):
        return 'int16_t' # clamped to INT16_MAX (always invalid value)

    return type

def get_type_size(func_name, param):
    type = get_marshal_type(func_name, param)

    if type.find('*') != -1:
        return 8;

    mapping = {
        'GLboolean': 1,
        'GLbyte': 1,
        'GLubyte': 1,
        'GLenum16': 2, # clamped by glthread
        'GLshort': 2,
        'GLushort': 2,
        'GLhalfNV': 2,
        'int16_t': 2, # clamped by glthread
        'GLint': 4,
        'GLuint': 4,
        'GLbitfield': 4,
        'GLsizei': 4,
        'GLfloat': 4,
        'GLclampf': 4,
        'GLfixed': 4,
        'GLclampx': 4,
        'GLhandleARB': 4,
        'int': 4,
        'float': 4,
        'GLdouble': 8,
        'GLclampd': 8,
        'GLintptr': 8,
        'GLsizeiptr': 8,
        'GLint64': 8,
        'GLuint64': 8,
        'GLuint64EXT': 8,
        'GLsync': 8,
    }
    val = mapping.get(type, 9999)
    if val == 9999:
        print('Unhandled type in marshal_XML.get_type_size: ' + type, file=sys.stderr)
        assert False
    return val

class marshal_item_factory(gl_XML.gl_item_factory):
    """Factory to create objects derived from gl_item containing
    information necessary to generate thread marshalling code."""

    def create_function(self, element, context):
        return marshal_function(element, context)


class marshal_function(gl_XML.gl_function):
    def process_element(self, element):
        # Do normal processing.
        super(marshal_function, self).process_element(element)

        # Only do further processing when we see the canonical
        # function name.
        if element.get('name') != self.name:
            return

        # Classify fixed and variable parameters.
        self.fixed_params = []
        self.variable_params = []
        for p in self.parameters:
            if p.is_padding:
                continue
            if p.is_variable_length():
                self.variable_params.append(p)
            else:
                self.fixed_params.append(p)

        # Store the "marshal" attribute, if present.
        self.marshal = element.get('marshal')
        self.marshal_sync = element.get('marshal_sync')
        self.marshal_call_before = element.get('marshal_call_before')
        self.marshal_call_after = element.get('marshal_call_after')
        self.marshal_struct = element.get('marshal_struct')

    def marshal_flavor(self):
        """Find out how this function should be marshalled between
        client and server threads."""
        # If a "marshal" attribute was present, that overrides any
        # determination that would otherwise be made by this function.
        if self.marshal is not None:
            return self.marshal

        if self.exec_flavor == 'skip':
            # Functions marked exec="skip" are not yet implemented in
            # Mesa, so don't bother trying to marshal them.
            return 'skip'

        if self.return_type != 'void':
            return 'sync'
        for p in self.parameters:
            if p.is_output:
                return 'sync'
            if (p.is_pointer() and not (p.count or p.counter or p.marshal_count)):
                return 'sync'
            if p.count_parameter_list and not p.marshal_count:
                # Parameter size is determined by enums; haven't
                # written logic to handle this yet.  TODO: fix.
                return 'sync'
        return 'async'

    def marshal_is_static(self):
        return (self.marshal_flavor() != 'custom' and
                self.name[0:8] != 'Internal' and
                self.exec_flavor != 'beginend')

    def get_fixed_params(self):
        # We want glthread to ignore variable-sized parameters if the only thing
        # we want is to pass the pointer parameter as-is, e.g. when a PBO is bound.
        # Making it conditional on marshal_sync is kinda hacky, but it's the easiest
        # path towards handling PBOs in glthread, which use marshal_sync to check whether
        # a PBO is bound.
        if self.marshal_sync:
            return self.fixed_params + self.variable_params
        else:
            return self.fixed_params

    def get_variable_params(self):
        if self.marshal_sync:
            return []
        else:
            return self.variable_params

    def print_struct(self, is_header=False):
        fixed_params = self.get_fixed_params()
        variable_params = self.get_variable_params()

        if (self.marshal_struct == 'public') == is_header:
            print('struct marshal_cmd_{0}'.format(self.name))
            print('{')
            print('   struct marshal_cmd_base cmd_base;')

            # Sort the parameters according to their size to pack the structure optimally
            for p in sorted(fixed_params, key=lambda p: get_type_size(self.name, p)):
                if p.count:
                    print('   {0} {1}[{2}];'.format(
                            p.get_base_type_string(), p.name, p.count))
                else:
                    print('   {0} {1};'.format(get_marshal_type(self.name, p), p.name))

            for p in variable_params:
                if p.img_null_flag:
                    print('   bool {0}_null; /* If set, no data follows '
                        'for "{0}" */'.format(p.name))

            for p in variable_params:
                if p.count_scale != 1:
                    print(('   /* Next {0} bytes are '
                         '{1} {2}[{3}][{4}] */').format(
                            p.size_string(marshal=1), p.get_base_type_string(),
                            p.name, p.counter, p.count_scale))
                else:
                    print(('   /* Next {0} bytes are '
                         '{1} {2}[{3}] */').format(
                            p.size_string(marshal=1), p.get_base_type_string(),
                            p.name, p.counter))
            print('};')
        elif self.marshal_flavor() in ('custom', 'async'):
            print('struct marshal_cmd_{0};'.format(self.name))
