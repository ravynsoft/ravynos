
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

# This script generates the file api_exec_init.c, which contains
# _mesa_init_dispatch().  It is responsible for populating all
# entries in the "OutsideBeginEnd" dispatch table.

import argparse
import collections
import license
import gl_XML
import sys
import apiexec


exec_flavor_map = {
    'beginend': None,
    'dlist': '_mesa_',
    'mesa': '_mesa_',
    'skip': None,
    }


header = """/**
 * \\file api_exec_init.c
 * Initialize dispatch table.
 */


#include "api_exec_decl.h"
#include "glapi/glapi.h"
#include "main/context.h"
#include "main/dispatch.h"


/**
 * Initialize a context's OutsideBeginEnd table with pointers to Mesa's supported
 * GL functions.
 *
 * This function depends on ctx->Version.
 *
 * \param ctx  GL context
 */
void
_mesa_init_dispatch(struct gl_context *ctx)
{
   struct _glapi_table *table = ctx->Dispatch.OutsideBeginEnd;

   assert(table != NULL);
   assert(ctx->Version > 0);
"""


footer = """
}
"""


class PrintCode(gl_XML.gl_print_base):

    def __init__(self):
        gl_XML.gl_print_base.__init__(self)

        self.name = 'api_exec_init.py'
        self.license = license.bsd_license_template % (
            'Copyright (C) 2012 Intel Corporation',
            'Intel Corporation')

    def printRealHeader(self):
        print(header)

    def printRealFooter(self):
        print(footer)

    def printBody(self, api):
        # Collect SET_* calls by the condition under which they should
        # be called.
        settings_by_condition = collections.defaultdict(lambda: [])
        for f in api.functionIterateAll():
            if f.exec_flavor not in exec_flavor_map:
                raise Exception(
                    'Unrecognized exec flavor {0!r}'.format(f.exec_flavor))
            condition = apiexec.get_api_condition(f)
            if not condition:
                continue
            prefix = exec_flavor_map[f.exec_flavor]
            if prefix is None:
                # This function is not implemented, or is dispatched
                # via beginend.
                continue
            if f.has_no_error_variant:
                no_error_condition = '_mesa_is_no_error_enabled(ctx) && ({0})'.format(condition)
                error_condition = '!_mesa_is_no_error_enabled(ctx) && ({0})'.format(condition)
                settings_by_condition[no_error_condition].append(
                    'SET_{0}(table, {1}{0}_no_error);'.format(f.name, prefix, f.name))
                settings_by_condition[error_condition].append(
                    'SET_{0}(table, {1}{0});'.format(f.name, prefix, f.name))
            else:
                settings_by_condition[condition].append(
                    'SET_{0}(table, {1}{0});'.format(f.name, prefix, f.name))
        # Print out an if statement for each unique condition, with
        # the SET_* calls nested inside it.
        for condition in sorted(settings_by_condition.keys()):
            print('   if ({0}) {{'.format(condition))
            for setting in sorted(settings_by_condition[condition]):
                print('      {0}'.format(setting))
            print('   }')


if __name__ == '__main__':
    apiexec.print_glapi_file(PrintCode())
