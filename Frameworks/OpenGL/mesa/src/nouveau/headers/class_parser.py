#! /usr/bin/env python3

# script to parse nvidia CL headers and generate inlines to be used in pushbuffer encoding.
# probably needs python3.9

import argparse
import os.path
import sys

from mako.template import Template

METHOD_ARRAY_SIZES = {
    'BIND_GROUP_CONSTANT_BUFFER'        : 16,
    'CALL_MME_DATA'                     : 256,
    'CALL_MME_MACRO'                    : 256,
    'LOAD_CONSTANT_BUFFER'              : 16,
    'SET_ANTI_ALIAS_SAMPLE_POSITIONS'   : 4,
    'SET_BLEND'                         : 8,
    'SET_BLEND_PER_TARGET_*'            : 8,
    'SET_COLOR_TARGET_*'                : 8,
    'SET_COLOR_COMPRESSION'             : 8,
    'SET_COLOR_CLEAR_VALUE'             : 4,
    'SET_CT_WRITE'                      : 8,
    'SET_MME_SHADOW_SCRATCH'            : 256,
    'SET_PIPELINE_*'                    : 6,
    'SET_SCISSOR_*'                     : 16,
    'SET_STREAM_OUT_BUFFER_*'           : 4,
    'SET_STREAM_OUT_CONTROL_*'          : 4,
    'SET_VIEWPORT_*'                    : 16,
    'SET_VERTEX_ATTRIBUTE_*'            : 16,
    'SET_VERTEX_STREAM_*'               : 16,
}

METHOD_IS_FLOAT = [
    'SET_BLEND_CONST_*',
    'SET_DEPTH_BIAS',
    'SET_SLOPE_SCALE_DEPTH_BIAS',
    'SET_DEPTH_BIAS_CLAMP',
    'SET_DEPTH_BOUNDS_M*',
    'SET_LINE_WIDTH_FLOAT',
    'SET_ALIASED_LINE_WIDTH_FLOAT',
    'SET_VIEWPORT_SCALE_*',
    'SET_VIEWPORT_OFFSET_*',
    'SET_VIEWPORT_CLIP_MIN_Z',
    'SET_VIEWPORT_CLIP_MAX_Z',
    'SET_Z_CLEAR_VALUE',
]

TEMPLATE_H = Template("""\
/* parsed class ${nvcl} */

#include "nvtypes.h"
#include "${clheader}"

#include <assert.h>
#include <stdio.h>
#include "util/u_math.h"

%for mthd in mthddict:
struct nv_${nvcl.lower()}_${mthd} {
  %for field_name in mthddict[mthd].field_name_start:
    uint32_t ${field_name.lower()};
  %endfor
};

static inline void
__${nvcl}_${mthd}(uint32_t *val_out, struct nv_${nvcl.lower()}_${mthd} st)
{
    uint32_t val = 0;
  %for field_name in mthddict[mthd].field_name_start:
    <%
        field_start = int(mthddict[mthd].field_name_start[field_name])
        field_end = int(mthddict[mthd].field_name_end[field_name])
        field_width = field_end - field_start + 1
    %>
    %if field_width == 32:
    val |= st.${field_name.lower()};
    %else:
    assert(st.${field_name.lower()} < (1ULL << ${field_width}));
    val |= st.${field_name.lower()} << ${field_start};
    %endif
  %endfor
    *val_out = val;
}

#define V_${nvcl}_${mthd}(val, args...) { ${bs}
  %for field_name in mthddict[mthd].field_name_start:
    %for d in mthddict[mthd].field_defs[field_name]:
    UNUSED uint32_t ${field_name}_${d} = ${nvcl}_${mthd}_${field_name}_${d}; ${bs}
    %endfor
  %endfor
  %if len(mthddict[mthd].field_name_start) > 1:
    struct nv_${nvcl.lower()}_${mthd} __data = args; ${bs}
  %else:
<% field_name = next(iter(mthddict[mthd].field_name_start)).lower() %>\
    struct nv_${nvcl.lower()}_${mthd} __data = { .${field_name} = (args) }; ${bs}
  %endif
    __${nvcl}_${mthd}(&val, __data); ${bs}
}

%if mthddict[mthd].is_array:
#define VA_${nvcl}_${mthd}(i) V_${nvcl}_${mthd}
%else:
#define VA_${nvcl}_${mthd} V_${nvcl}_${mthd}
%endif

%if mthddict[mthd].is_array:
#define P_${nvcl}_${mthd}(push, idx, args...) do { ${bs}
%else:
#define P_${nvcl}_${mthd}(push, args...) do { ${bs}
%endif
  %for field_name in mthddict[mthd].field_name_start:
    %for d in mthddict[mthd].field_defs[field_name]:
    UNUSED uint32_t ${field_name}_${d} = ${nvcl}_${mthd}_${field_name}_${d}; ${bs}
    %endfor
  %endfor
    uint32_t nvk_p_ret; ${bs}
    V_${nvcl}_${mthd}(nvk_p_ret, args); ${bs}
    %if mthddict[mthd].is_array:
    nv_push_val(push, ${nvcl}_${mthd}(idx), nvk_p_ret); ${bs}
    %else:
    nv_push_val(push, ${nvcl}_${mthd}, nvk_p_ret); ${bs}
    %endif
} while(0)

%endfor

const char *P_PARSE_${nvcl}_MTHD(uint16_t idx);
void P_DUMP_${nvcl}_MTHD_DATA(FILE *fp, uint16_t idx, uint32_t data,
                              const char *prefix);
""")

TEMPLATE_C = Template("""\
#include "${header}"

#include <stdio.h>

const char*
P_PARSE_${nvcl}_MTHD(uint16_t idx)
{
    switch (idx) {
%for mthd in mthddict:
  %if mthddict[mthd].is_array and mthddict[mthd].array_size == 0:
    <% continue %>
  %endif
  %if mthddict[mthd].is_array:
    %for i in range(mthddict[mthd].array_size):
    case ${nvcl}_${mthd}(${i}):
        return "${nvcl}_${mthd}(${i})";
    %endfor
  % else:
    case ${nvcl}_${mthd}:
        return "${nvcl}_${mthd}";
  %endif
%endfor
    default:
        return "unknown method";
    }
}

void
P_DUMP_${nvcl}_MTHD_DATA(FILE *fp, uint16_t idx, uint32_t data,
                         const char *prefix)
{
    uint32_t parsed;
    switch (idx) {
%for mthd in mthddict:
  %if mthddict[mthd].is_array and mthddict[mthd].array_size == 0:
    <% continue %>
  %endif
  %if mthddict[mthd].is_array:
    %for i in range(mthddict[mthd].array_size):
    case ${nvcl}_${mthd}(${i}):
    %endfor
  % else:
    case ${nvcl}_${mthd}:
  %endif
  %for field_name in mthddict[mthd].field_name_start:
    <%
        field_start = int(mthddict[mthd].field_name_start[field_name])
        field_end = int(mthddict[mthd].field_name_end[field_name])
        field_width = field_end - field_start + 1
    %>
    %if field_width == 32:
        parsed = data;
    %else:
        parsed = (data >> ${field_start}) & ((1u << ${field_width}) - 1);
    %endif
        fprintf(fp, "%s.${field_name} = ", prefix);
    %if len(mthddict[mthd].field_defs[field_name]):
        switch (parsed) {
      %for d in mthddict[mthd].field_defs[field_name]:
        case ${nvcl}_${mthd}_${field_name}_${d}:
            fprintf(fp, "${d}${bs}n");
            break;
      %endfor
        default:
            fprintf(fp, "0x%x${bs}n", parsed);
            break;
        }
    %else:
      %if mthddict[mthd].is_float:
        fprintf(fp, "%ff (0x%x)${bs}n", uif(parsed), parsed);
      %else:
        fprintf(fp, "(0x%x)${bs}n", parsed);
      %endif
    %endif
  %endfor
        break;
%endfor
    default:
        fprintf(fp, "%s.VALUE = 0x%x${bs}n", prefix, data);
        break;
    }
}
""")

def glob_match(glob, name):
    if glob.endswith('*'):
        return name.startswith(glob[:-1])
    else:
        assert '*' not in glob
        return name == glob

class method(object):
    @property
    def array_size(self):
        for (glob, value) in METHOD_ARRAY_SIZES.items():
            if glob_match(glob, self.name):
                return value
        return 0

    @property
    def is_float(self):
        for glob in METHOD_IS_FLOAT:
            if glob_match(glob, self.name):
                assert len(self.field_defs) == 1
                return True
        return False

def parse_header(nvcl, f):
    # Simple state machine
    # state 0 looking for a new method define
    # state 1 looking for new fields in a method
    # state 2 looking for enums for a fields in a method
    # blank lines reset the state machine to 0

    state = 0
    mthddict = {}
    curmthd = {}
    for line in f:

        if line.strip() == "":
            state = 0
            if (curmthd):
                if not len(curmthd.field_name_start):
                    del mthddict[curmthd.name]
            curmthd = {}
            continue

        if line.startswith("#define"):
            list = line.split();
            if "_cl_" in list[1]:
                continue

            if not list[1].startswith(nvcl):
                continue

            if list[1].endswith("TYPEDEF"):
                continue

            if state == 2:
                teststr = nvcl + "_" + curmthd.name + "_" + curfield + "_"
                if ":" in list[2]:
                    state = 1
                elif teststr in list[1]:
                    curmthd.field_defs[curfield][list[1].removeprefix(teststr)] = list[2]
                else:
                    state = 1

            if state == 1:
                teststr = nvcl + "_" + curmthd.name + "_"
                if teststr in list[1]:
                    if ("0x" in list[2]):
                        state = 1
                    else:
                        field = list[1].removeprefix(teststr)
                        bitfield = list[2].split(":")
                        curmthd.field_name_start[field] = bitfield[1]
                        curmthd.field_name_end[field] = bitfield[0]
                        curmthd.field_defs[field] = {}
                        curfield = field
                        state = 2
                else:
                    if not len(curmthd.field_name_start):
                        del mthddict[curmthd.name]
                        curmthd = {}
                    state = 0

            if state == 0:
                if (curmthd):
                    if not len(curmthd.field_name_start):
                        del mthddict[curmthd.name]
                teststr = nvcl + "_"
                is_array = 0
                if (':' in list[2]):
                    continue
                name = list[1].removeprefix(teststr)
                if name.endswith("(i)"):
                    is_array = 1
                    name = name.removesuffix("(i)")
                if name.endswith("(j)"):
                    is_array = 1
                    name = name.removesuffix("(j)")
                x = method()
                x.name = name
                x.addr = list[2]
                x.is_array = is_array
                x.field_name_start = {}
                x.field_name_end = {}
                x.field_defs = {}
                mthddict[x.name] = x

                curmthd = x
                state = 1

    return mthddict

def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('--out_h', required=True, help='Output C header.')
    parser.add_argument('--out_c', required=True, help='Output C file.')
    parser.add_argument('--in_h',
                        help='Input class header file.',
                        required=True)
    args = parser.parse_args()

    clheader = os.path.basename(args.in_h)
    nvcl = clheader
    nvcl = nvcl.removeprefix("cl")
    nvcl = nvcl.removesuffix(".h")
    nvcl = nvcl.upper()
    nvcl = "NV" + nvcl

    with open(args.in_h, 'r', encoding='utf-8') as f:
        mthddict = parse_header(nvcl, f)

    environment = {
        'clheader': clheader,
        'header': os.path.basename(args.out_h),
        'nvcl': nvcl,
        'mthddict': mthddict,
        'bs': '\\'
    }

    try:
        with open(args.out_h, 'w', encoding='utf-8') as f:
            f.write(TEMPLATE_H.render(**environment))
        with open(args.out_c, 'w', encoding='utf-8') as f:
            f.write(TEMPLATE_C.render(**environment))

    except Exception:
        # In the event there's an error, this imports some helpers from mako
        # to print a useful stack trace and prints it, then exits with
        # status 1, if python is run with debug; otherwise it just raises
        # the exception
        import sys
        from mako import exceptions
        print(exceptions.text_error_template().render(), file=sys.stderr)
        sys.exit(1)

if __name__ == '__main__':
    main()
