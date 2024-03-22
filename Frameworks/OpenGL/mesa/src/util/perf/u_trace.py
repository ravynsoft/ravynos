#
# Copyright (C) 2020 Google, Inc.
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
#

from mako.template import Template
from collections import namedtuple
from enum import IntEnum
import os

TRACEPOINTS = {}
TRACEPOINTS_TOGGLES = {}

class Tracepoint(object):
    """Class that represents all the information about a tracepoint
    """
    def __init__(self, name, args=[], toggle_name=None,
                 tp_struct=None, tp_print=None, tp_perfetto=None,
                 tp_markers=None, end_of_pipe=False, need_cs_param=True):
        """Parameters:

        - name: the tracepoint name, a tracepoint function with the given
          name (prefixed by 'trace_') will be generated with the specied
          args (following a u_trace ptr).  Calling this tracepoint will
          emit a trace, if tracing is enabled.
        - args: the tracepoint func args, an array of TracepointArg
        - tp_print: (optional) array of format string followed by expressions
        - tp_perfetto: (optional) driver provided callback which can generate
          perfetto events
        - tp_markers: (optional) driver provided printf-style callback which can
          generate CS markers, this requires 'need_cs_param' as the first param
          is the CS that the label should be emitted into
        - need_cs_param: whether tracepoint functions need an additional cs
          parameter.
        """
        assert isinstance(name, str)
        assert isinstance(args, list)
        assert name not in TRACEPOINTS


        self.name = name
        self.args = args
        if tp_struct is None:
           tp_struct = args
        self.tp_struct = tp_struct
        self.has_variable_arg = False
        for arg in self.tp_struct:
            if arg.length_arg != None:
                self.has_variable_arg = True
                break
        self.tp_print = tp_print
        self.tp_perfetto = tp_perfetto
        self.tp_markers = tp_markers
        self.end_of_pipe = end_of_pipe
        self.toggle_name = toggle_name
        self.need_cs_param = need_cs_param

        TRACEPOINTS[name] = self
        if toggle_name is not None and toggle_name not in TRACEPOINTS_TOGGLES:
            TRACEPOINTS_TOGGLES[toggle_name] = len(TRACEPOINTS_TOGGLES)

    def can_generate_print(self):
        return self.args is not None and len(self.args) > 0

    def enabled_expr(self, trace_toggle_name):
        if trace_toggle_name is None:
            return "true"
        assert self.toggle_name is not None
        return "({0} & {1}_{2})".format(trace_toggle_name,
                                        trace_toggle_name.upper(),
                                        self.toggle_name.upper())

class TracepointArgStruct():
    """Represents struct that is being passed as an argument
    """
    def __init__(self, type, var):
        """Parameters:

        - type: argument's C type.
        - var: name of the argument
        """
        assert isinstance(type, str)
        assert isinstance(var, str)

        self.type = type
        self.var = var

class TracepointArg(object):
    """Class that represents either an argument being passed or a field in a struct
    """
    def __init__(self, type, var, c_format, name=None, to_prim_type=None, length_arg=None, copy_func=None):
        """Parameters:

        - type: argument's C type.
        - var: either an argument name or a field in the struct
        - c_format: printf format to print the value.
        - name: (optional) name that will be used in intermidiate structs and will
          be displayed in output or perfetto, otherwise var will be used.
        - to_prim_type: (optional) C function to convert from arg's type to a type
          compatible with c_format.
        - length_arg: whether this argument is a variable length array
        """
        assert isinstance(type, str)
        assert isinstance(var, str)
        assert isinstance(c_format, str)

        self.type = type
        self.var = var
        self.c_format = c_format
        if name is None:
           name = var
        self.name = name
        self.to_prim_type = to_prim_type
        self.length_arg = length_arg
        self.copy_func = copy_func


HEADERS = []

class HeaderScope(IntEnum):
    HEADER = (1 << 0)
    SOURCE = (1 << 1)
    PERFETTO = (1 << 2)

class Header(object):
    """Class that represents a header file dependency of generated tracepoints
    """
    def __init__(self, hdr, scope=HeaderScope.HEADER):
        """Parameters:

        - hdr: the required header path
        """
        assert isinstance(hdr, str)
        self.hdr = hdr
        self.scope = scope

        HEADERS.append(self)


FORWARD_DECLS = []

class ForwardDecl(object):
   """Class that represents a forward declaration
   """
   def __init__(self, decl):
        assert isinstance(decl, str)
        self.decl = decl

        FORWARD_DECLS.append(self)


hdr_template = """\
/* Copyright (C) 2020 Google, Inc.
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

<% guard_name = '_' + hdrname + '_H' %>
#ifndef ${guard_name}
#define ${guard_name}

% for header in HEADERS:
#include "${header.hdr}"
% endfor

#include "util/perf/u_trace.h"

#ifdef __cplusplus
extern "C" {
#endif

% for declaration in FORWARD_DECLS:
${declaration.decl};
% endfor

% if trace_toggle_name is not None:
enum ${trace_toggle_name.lower()} {
%    for toggle_name, config_id in TRACEPOINTS_TOGGLES.items():
   ${trace_toggle_name.upper()}_${toggle_name.upper()} = 1ull << ${config_id},
%    endfor
};

extern uint64_t ${trace_toggle_name};

void ${trace_toggle_name}_config_variable(void);
% endif

% for trace_name, trace in TRACEPOINTS.items():

/*
 * ${trace_name}
 */
struct trace_${trace_name} {
%    for arg in trace.tp_struct:
   ${arg.type} ${arg.name}${"[0]" if arg.length_arg else ""};
%    endfor
%    if len(trace.args) == 0:
#ifdef __cplusplus
   /* avoid warnings about empty struct size mis-match in C vs C++..
    * the size mis-match is harmless because (a) nothing will deref
    * the empty struct, and (b) the code that cares about allocating
    * sizeof(struct trace_${trace_name}) (and wants this to be zero
    * if there is no payload) is C
    */
   uint8_t dummy;
#endif
%    endif
};
%    if trace.tp_perfetto is not None:
#ifdef HAVE_PERFETTO
void ${trace.tp_perfetto}(
   ${ctx_param},
   uint64_t ts_ns,
   uint16_t tp_idx,
   const void *flush_data,
   const struct trace_${trace_name} *payload);
#endif
%    endif
void __trace_${trace_name}(
       struct u_trace *ut
     , enum u_trace_type enabled_traces
%    if trace.need_cs_param:
     , void *cs
%    endif
%    for arg in trace.args:
     , ${arg.type} ${arg.var}
%    endfor
);
static ALWAYS_INLINE void trace_${trace_name}(
     struct u_trace *ut
%    if trace.need_cs_param:
   , void *cs
%    endif
%    for arg in trace.args:
   , ${arg.type} ${arg.var}
%    endfor
) {
   enum u_trace_type enabled_traces = p_atomic_read_relaxed(&ut->utctx->enabled_traces);
   if (!unlikely(enabled_traces != 0 &&
                 ${trace.enabled_expr(trace_toggle_name)}))
      return;
   __trace_${trace_name}(
        ut
      , enabled_traces
%    if trace.need_cs_param:
      , cs
%    endif
%    for arg in trace.args:
      , ${arg.var}
%    endfor
   );
}
% endfor

#ifdef __cplusplus
}
#endif

#endif /* ${guard_name} */
"""

src_template = """\
/* Copyright (C) 2020 Google, Inc.
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

#include "${hdr}"

% for header in HEADERS:
#include "${header.hdr}"
% endfor

#define __NEEDS_TRACE_PRIV
#include "util/u_debug.h"
#include "util/perf/u_trace_priv.h"

% if trace_toggle_name is not None:
static const struct debug_control config_control[] = {
%    for toggle_name in TRACEPOINTS_TOGGLES.keys():
   { "${toggle_name}", ${trace_toggle_name.upper()}_${toggle_name.upper()}, },
%    endfor
   { NULL, 0, },
};
uint64_t ${trace_toggle_name} = 0;

static void
${trace_toggle_name}_variable_once(void)
{
   uint64_t default_value = 0
%    for name in trace_toggle_defaults:
     | ${trace_toggle_name.upper()}_${name.upper()}
%    endfor
     ;

   ${trace_toggle_name} =
      parse_enable_string(getenv("${trace_toggle_name.upper()}"),
                          default_value,
                          config_control);
}

void
${trace_toggle_name}_config_variable(void)
{
   static once_flag process_${trace_toggle_name}_variable_flag = ONCE_FLAG_INIT;

   call_once(&process_${trace_toggle_name}_variable_flag,
             ${trace_toggle_name}_variable_once);
}
% endif

% for index, (trace_name, trace) in enumerate(TRACEPOINTS.items()):
/*
 * ${trace_name}
 */
 % if trace.can_generate_print():
static void __print_${trace_name}(FILE *out, const void *arg) {
   const struct trace_${trace_name} *__entry =
      (const struct trace_${trace_name} *)arg;
  % if trace.tp_print is not None:
   fprintf(out, "${trace.tp_print[0]}\\n"
   % for arg in trace.tp_print[1:]:
           , ${arg}
   % endfor
  % else:
   fprintf(out, ""
   % for arg in trace.tp_struct:
      "${arg.name}=${arg.c_format}, "
   % endfor
         "\\n"
   % for arg in trace.tp_struct:
    % if arg.to_prim_type:
   ,${arg.to_prim_type.format('__entry->' + arg.name)}
    % else:
   ,__entry->${arg.name}
    % endif
   % endfor
  % endif
   );
}

static void __print_json_${trace_name}(FILE *out, const void *arg) {
   const struct trace_${trace_name} *__entry =
      (const struct trace_${trace_name} *)arg;
  % if trace.tp_print is not None:
   fprintf(out, "\\"unstructured\\": \\"${trace.tp_print[0]}\\""
   % for arg in trace.tp_print[1:]:
           , ${arg}
   % endfor
  % else:
   fprintf(out, ""
   % for arg in trace.tp_struct:
      "\\"${arg.name}\\": \\"${arg.c_format}\\""
      % if arg != trace.tp_struct[-1]:
         ", "
      % endif
   % endfor
   % for arg in trace.tp_struct:
    % if arg.to_prim_type:
   ,${arg.to_prim_type.format('__entry->' + arg.name)}
    % else:
   ,__entry->${arg.name}
    % endif
   % endfor
  % endif
   );
}

 % else:
#define __print_${trace_name} NULL
#define __print_json_${trace_name} NULL
 % endif
 % if trace.tp_markers is not None:

__attribute__((format(printf, 3, 4))) void ${trace.tp_markers}(struct u_trace_context *utctx, void *, const char *, ...);

static void __emit_label_${trace_name}(struct u_trace_context *utctx, void *cs, struct trace_${trace_name} *entry) {
   ${trace.tp_markers}(utctx, cs, "${trace_name}("
   % for idx,arg in enumerate(trace.tp_struct):
      "${"," if idx != 0 else ""}${arg.name}=${arg.c_format}"
   % endfor
      ")"
   % for arg in trace.tp_struct:
    % if arg.to_prim_type:
      ,${arg.to_prim_type.format('entry->' + arg.name)}
    % else:
      ,entry->${arg.name}
    % endif
   % endfor
   );
}

 % endif
static const struct u_tracepoint __tp_${trace_name} = {
    ALIGN_POT(sizeof(struct trace_${trace_name}), 8),   /* keep size 64b aligned */
    "${trace_name}",
    ${"true" if trace.end_of_pipe else "false"},
    ${index},
    __print_${trace_name},
    __print_json_${trace_name},
 % if trace.tp_perfetto is not None:
#ifdef HAVE_PERFETTO
    (void (*)(void *pctx, uint64_t, uint16_t, const void *, const void *))${trace.tp_perfetto},
#endif
 % endif
};
void __trace_${trace_name}(
     struct u_trace *ut
   , enum u_trace_type enabled_traces
 % if trace.need_cs_param:
   , void *cs
 % endif
 % for arg in trace.args:
   , ${arg.type} ${arg.var}
 % endfor
) {
   struct trace_${trace_name} entry;
   UNUSED struct trace_${trace_name} *__entry =
      enabled_traces & U_TRACE_TYPE_REQUIRE_QUEUING ?
 % if trace.has_variable_arg:
      (struct trace_${trace_name} *)u_trace_appendv(ut, ${"cs," if trace.need_cs_param else "NULL,"} &__tp_${trace_name},
                                                    0
  % for arg in trace.tp_struct:
   % if arg.length_arg is not None:
                                                    + ${arg.length_arg}
   % endif
  % endfor
                                                    ) :
 % else:
      (struct trace_${trace_name} *)u_trace_append(ut, ${"cs," if trace.need_cs_param else "NULL,"} &__tp_${trace_name}) :
 % endif
      &entry;
 % for arg in trace.tp_struct:
  % if arg.length_arg is None:
   __entry->${arg.name} = ${arg.var};
  % else:
   ${arg.copy_func}(__entry->${arg.name}, ${arg.var}, ${arg.length_arg});
  % endif
 % endfor
 % if trace.tp_markers is not None:
   if (enabled_traces & U_TRACE_TYPE_MARKERS)
      __emit_label_${trace_name}(ut->utctx, cs, __entry);
 % endif
}

% endfor
"""

def utrace_generate(cpath, hpath, ctx_param, trace_toggle_name=None,
                    trace_toggle_defaults=[]):
    """Parameters:

    - cpath: c file to generate.
    - hpath: h file to generate.
    - ctx_param: type of the first parameter to the perfetto vfuncs.
    - trace_toggle_name: (optional) name of the environment variable
      enabling/disabling tracepoints.
    - trace_toggle_defaults: (optional) list of tracepoints enabled by default.
    """
    if cpath is not None:
        hdr = os.path.basename(cpath).rsplit('.', 1)[0] + '.h'
        with open(cpath, 'w', encoding='utf-8') as f:
            f.write(Template(src_template).render(
                hdr=hdr,
                ctx_param=ctx_param,
                trace_toggle_name=trace_toggle_name,
                trace_toggle_defaults=trace_toggle_defaults,
                HEADERS=[h for h in HEADERS if h.scope & HeaderScope.SOURCE],
                TRACEPOINTS=TRACEPOINTS,
                TRACEPOINTS_TOGGLES=TRACEPOINTS_TOGGLES))

    if hpath is not None:
        hdr = os.path.basename(hpath)
        with open(hpath, 'w', encoding='utf-8') as f:
            f.write(Template(hdr_template).render(
                hdrname=hdr.rstrip('.h').upper(),
                ctx_param=ctx_param,
                trace_toggle_name=trace_toggle_name,
                HEADERS=[h for h in HEADERS if h.scope & HeaderScope.HEADER],
                FORWARD_DECLS=FORWARD_DECLS,
                TRACEPOINTS=TRACEPOINTS,
                TRACEPOINTS_TOGGLES=TRACEPOINTS_TOGGLES))


perfetto_utils_hdr_template = """\
/*
 * Copyright © 2021 Igalia S.L.
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
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

<% guard_name = '_' + hdrname + '_H' %>
#ifndef ${guard_name}
#define ${guard_name}

#include <perfetto.h>

% for header in HEADERS:
#include "${header.hdr}"
% endfor

UNUSED static const char *${basename}_names[] = {
% for trace_name, trace in TRACEPOINTS.items():
   "${trace_name}",
% endfor
};

% for trace_name, trace in TRACEPOINTS.items():
static void UNUSED
trace_payload_as_extra_${trace_name}(perfetto::protos::pbzero::GpuRenderStageEvent *event,
                                     const struct trace_${trace_name} *payload)
{
 % if all([trace.tp_perfetto, trace.tp_struct]) and len(trace.tp_struct) > 0:
   char buf[128];

  % for arg in trace.tp_struct:
   {
      auto data = event->add_extra_data();
      data->set_name("${arg.name}");

   % if arg.to_prim_type:
      sprintf(buf, "${arg.c_format}", ${arg.to_prim_type.format('payload->' + arg.name)});
   % else:
      sprintf(buf, "${arg.c_format}", payload->${arg.name});
   % endif

      data->set_value(buf);
   }
  % endfor

 % endif
}
% endfor

#endif /* ${guard_name} */
"""

def utrace_generate_perfetto_utils(hpath,basename="tracepoint"):
    if hpath is not None:
        hdr = os.path.basename(hpath)
        with open(hpath, 'w', encoding='utf-8') as f:
            f.write(Template(perfetto_utils_hdr_template).render(
                basename=basename,
                hdrname=hdr.rstrip('.h').upper(),
                HEADERS=[h for h in HEADERS if h.scope & HeaderScope.PERFETTO],
                TRACEPOINTS=TRACEPOINTS))
