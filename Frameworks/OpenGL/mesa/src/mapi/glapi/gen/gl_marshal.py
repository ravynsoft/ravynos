
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

import contextlib
import gl_XML
import license
import marshal_XML
import sys
import collections
import apiexec

header = """
#include "context.h"
#include "glthread_marshal.h"
#include "bufferobj.h"
#include "dispatch.h"

#define COMPAT (ctx->API != API_OPENGL_CORE)

UNUSED static inline int safe_mul(int a, int b)
{
    if (a < 0 || b < 0) return -1;
    if (a == 0 || b == 0) return 0;
    if (a > INT_MAX / b) return -1;
    return a * b;
}
"""


file_index = 0
file_count = 1
current_indent = 0


def out(str):
    if str:
        print(' '*current_indent + str)
    else:
        print('')


@contextlib.contextmanager
def indent(delta = 3):
    global current_indent
    current_indent += delta
    yield
    current_indent -= delta


class PrintCode(gl_XML.gl_print_base):
    def __init__(self):
        super(PrintCode, self).__init__()

        self.name = 'gl_marshal.py'
        self.license = license.bsd_license_template % (
            'Copyright (C) 2012 Intel Corporation', 'INTEL CORPORATION')

    def printRealHeader(self):
        print(header)

    def printRealFooter(self):
        pass

    def print_call(self, func, unmarshal=0):
        ret = 'return ' if func.return_type != 'void' and not unmarshal else '';
        call = 'CALL_{0}(ctx->Dispatch.Current, ({1}))'.format(
            func.name, func.get_called_parameter_string())
        out('{0}{1};'.format(ret, call))
        if func.marshal_call_after and ret == '' and not unmarshal:
            out(func.marshal_call_after);

    def print_sync_body(self, func):
        out('/* {0}: marshalled synchronously */'.format(func.name))
        out('{0}{1} GLAPIENTRY'.format('static ' if func.marshal_is_static() else '', func.return_type))
        out('_mesa_marshal_{0}({1})'.format(func.name, func.get_parameter_string()))
        out('{')
        with indent():
            out('GET_CURRENT_CONTEXT(ctx);')
            if func.marshal_call_before:
                out(func.marshal_call_before);
            out('_mesa_glthread_finish_before(ctx, "{0}");'.format(func.name))
            self.print_call(func)
        out('}')
        out('')
        out('')

    def print_async_body(self, func):
        fixed_params = func.get_fixed_params()
        variable_params = func.get_variable_params()

        out('/* {0}: marshalled asynchronously */'.format(func.name))
        func.print_struct()

        out('uint32_t')
        out(('_mesa_unmarshal_{0}(struct gl_context *ctx, '
             'const struct marshal_cmd_{0} *restrict cmd)').format(func.name))
        out('{')
        with indent():
            for p in fixed_params:
                if p.count:
                    p_decl = '{0} *{1} = cmd->{1};'.format(
                            p.get_base_type_string(), p.name)
                else:
                    p_decl = '{0} {1} = cmd->{1};'.format(
                            marshal_XML.get_marshal_type(func.name, p), p.name)

                if not p_decl.startswith('const ') and p.count:
                    # Declare all local function variables as const, even if
                    # the original parameter is not const.
                    p_decl = 'const ' + p_decl

                out(p_decl)

            if variable_params:
                for p in variable_params:
                    out('{0} *{1};'.format(
                            p.get_base_type_string(), p.name))
                out('const char *variable_data = (const char *) (cmd + 1);')
                i = 1
                for p in variable_params:
                    out('{0} = ({1} *) variable_data;'.format(
                            p.name, p.get_base_type_string()))

                    if p.img_null_flag:
                        out('if (cmd->{0}_null)'.format(p.name))
                        with indent():
                            out('{0} = NULL;'.format(p.name))
                        if i < len(variable_params):
                            out('else')
                            with indent():
                                out('variable_data += {0};'.format(p.size_string(False, marshal=1)))
                    elif i < len(variable_params):
                        out('variable_data += {0};'.format(p.size_string(False, marshal=1)))
                    i += 1

            self.print_call(func, unmarshal=1)
            if variable_params:
                out('return cmd->cmd_base.cmd_size;')
            else:
                struct = 'struct marshal_cmd_{0}'.format(func.name)
                out('const unsigned cmd_size = (align(sizeof({0}), 8) / 8);'.format(struct))
                out('assert(cmd_size == cmd->cmd_base.cmd_size);')
                out('return cmd_size;')
        out('}')

        out('{0}{1} GLAPIENTRY'.format('static ' if func.marshal_is_static() else '', func.return_type))
        out('_mesa_marshal_{0}({1})'.format(
                func.name, func.get_parameter_string()))
        out('{')
        with indent():
            out('GET_CURRENT_CONTEXT(ctx);')
            if func.marshal_call_before:
                out(func.marshal_call_before);

            if not func.marshal_sync:
                for p in func.variable_params:
                    out('int {0}_size = {1};'.format(p.name, p.size_string(marshal=1)))

            struct = 'struct marshal_cmd_{0}'.format(func.name)
            size_terms = ['sizeof({0})'.format(struct)]
            if not func.marshal_sync:
                for p in func.variable_params:
                    if p.img_null_flag:
                        size_terms.append('({0} ? {0}_size : 0)'.format(p.name))
                    else:
                        size_terms.append('{0}_size'.format(p.name))
            out('int cmd_size = {0};'.format(' + '.join(size_terms)))
            out('{0} *cmd;'.format(struct))

            if func.marshal_sync:
                out('if ({0}) {{'.format(func.marshal_sync))
                with indent():
                    out('_mesa_glthread_finish_before(ctx, "{0}");'.format(func.name))
                    self.print_call(func)
                    out('return;')
                out('}')
            else:
                # Fall back to syncing if variable-length sizes can't be handled.
                #
                # Check that any counts for variable-length arguments might be < 0, in
                # which case the command alloc or the memcpy would blow up before we
                # get to the validation in Mesa core.
                list = []
                for p in func.parameters:
                    if p.is_variable_length():
                        list.append('{0}_size < 0'.format(p.name))
                        list.append('({0}_size > 0 && !{0})'.format(p.name))

                if len(list) != 0:
                    list.append('(unsigned)cmd_size > MARSHAL_MAX_CMD_SIZE')

                    out('if (unlikely({0})) {{'.format(' || '.join(list)))
                    with indent():
                        out('_mesa_glthread_finish_before(ctx, "{0}");'.format(func.name))
                        self.print_call(func)
                        out('return;')
                    out('}')

            # Add the call into the batch.
            out('cmd = _mesa_glthread_allocate_command(ctx, '
                'DISPATCH_CMD_{0}, cmd_size);'.format(func.name))

            for p in fixed_params:
                type = marshal_XML.get_marshal_type(func.name, p)

                if p.count:
                    out('memcpy(cmd->{0}, {0}, {1});'.format(
                            p.name, p.size_string()))
                elif type == 'GLenum16':
                    out('cmd->{0} = MIN2({0}, 0xffff); /* clamped to 0xffff (invalid enum) */'.format(p.name))
                elif type == 'int16_t':
                    out('cmd->{0} = CLAMP({0}, INT16_MIN, INT16_MAX);'.format(p.name))
                else:
                    out('cmd->{0} = {0};'.format(p.name))
            if variable_params:
                out('char *variable_data = (char *) (cmd + 1);')
                i = 1
                for p in variable_params:
                    if p.img_null_flag:
                        out('cmd->{0}_null = !{0};'.format(p.name))
                        out('if (!cmd->{0}_null) {{'.format(p.name))
                        with indent():
                            out(('memcpy(variable_data, {0}, {0}_size);').format(p.name))
                            if i < len(variable_params):
                                out('variable_data += {0}_size;'.format(p.name))
                        out('}')
                    else:
                        out(('memcpy(variable_data, {0}, {0}_size);').format(p.name))
                        if i < len(variable_params):
                            out('variable_data += {0}_size;'.format(p.name))
                    i += 1

            if not fixed_params and not variable_params:
                out('(void) cmd;')

            if func.marshal_call_after:
                out(func.marshal_call_after)

            # Uncomment this if you want to call _mesa_glthread_finish for debugging
            #out('_mesa_glthread_finish(ctx);')

            if func.return_type == 'GLboolean':
                out('return GL_TRUE;')  # for glUnmapBuffer
        out('}')
        out('')
        out('')

    def print_init_marshal_table(self, functions):
        out('void')
        out('_mesa_glthread_init_dispatch%u(struct gl_context *ctx, '
                                           'struct _glapi_table *table)' % file_index)
        out('{')
        with indent():
            # Collect SET_* calls by the condition under which they should
            # be called.
            settings_by_condition = collections.defaultdict(lambda: [])

            for func in functions:
                condition = apiexec.get_api_condition(func)
                if not condition:
                    continue

                settings_by_condition[condition].append(
                    'SET_{0}(table, _mesa_marshal_{0});'.format(func.name))

            # Print out an if statement for each unique condition, with
            # the SET_* calls nested inside it.
            for condition in sorted(settings_by_condition.keys()):
                out('if ({0}) {{'.format(condition))
                with indent():
                    for setting in sorted(settings_by_condition[condition]):
                        for line in setting.split('\n'):
                            out(line)
                out('}')
        out('}')

    def printBody(self, api):
        # Don't generate marshal/unmarshal functions for skipped and custom functions
        functions = [func for func in api.functionIterateAll()
                     if func.marshal_flavor() not in ('skip', 'custom')]
        # Divide the functions between files
        func_per_file = len(functions) // file_count + 1
        functions = functions[file_index*func_per_file:(file_index+1)*func_per_file]

        for func in functions:
            flavor = func.marshal_flavor()
            if flavor == 'async':
                self.print_async_body(func)
            elif flavor == 'sync':
                self.print_sync_body(func)
            else:
                assert False

        # The first file will also set custom functions
        if file_index == 0:
            functions += [func for func in api.functionIterateAll()
                          if func.marshal_flavor() == 'custom']

        self.print_init_marshal_table(functions)


def show_usage():
    print('Usage: %s [file_name] [file_index] [total file count]' % sys.argv[0])
    sys.exit(1)


if __name__ == '__main__':
    try:
        file_name = sys.argv[1]
        file_index = int(sys.argv[2])
        file_count = int(sys.argv[3])
    except Exception:
        show_usage()

    printer = PrintCode()

    api = gl_XML.parse_GL_API(file_name, marshal_XML.marshal_item_factory())
    printer.Print(api)
