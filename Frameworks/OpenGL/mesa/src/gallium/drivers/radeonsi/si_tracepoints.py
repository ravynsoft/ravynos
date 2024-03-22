#
# Copyright 2023 Advanced Micro Devices, Inc.
#
# SPDX-License-Identifier: MIT
#

import argparse
import sys

# List of the default tracepoints enabled. By default most tracepoints are
# enabled, set tp_default=False to disable them by default.
#
si_default_tps = []

#
# Tracepoint definitions:
#
def define_tracepoints(args):
    from u_trace import Header, HeaderScope
    from u_trace import ForwardDecl
    from u_trace import Tracepoint
    from u_trace import TracepointArg as Arg
    from u_trace import TracepointArgStruct as ArgStruct

    Header('si_perfetto.h', scope=HeaderScope.HEADER)
    

    def begin_end_tp(name, tp_args=[], tp_struct=None, tp_print=None,
                     tp_default_enabled=True, end_pipelined=True,
                     need_cs_param=False):
        global si_default_tps
        if tp_default_enabled:
            si_default_tps.append(name)
        Tracepoint('si_begin_{0}'.format(name),
                   toggle_name=name,
                   tp_perfetto='si_ds_begin_{0}'.format(name),
                   need_cs_param=need_cs_param)
        Tracepoint('si_end_{0}'.format(name),
                   toggle_name=name,
                   args=tp_args,
                   tp_struct=tp_struct,
                   tp_perfetto='si_ds_end_{0}'.format(name),
                   tp_print=tp_print,
                   end_of_pipe=end_pipelined,
                   need_cs_param=need_cs_param)

    # Various draws/dispatch, radeonsi
    begin_end_tp('draw',
                 tp_args=[Arg(type='uint32_t', var='count', c_format='%u')])

    begin_end_tp('compute',
                 tp_args=[Arg(type='uint32_t', var='group_x', c_format='%u'),
                          Arg(type='uint32_t', var='group_y', c_format='%u'),
                          Arg(type='uint32_t', var='group_z', c_format='%u'),],
                 tp_print=['group=%ux%ux%u', '__entry->group_x', '__entry->group_y', '__entry->group_z'])

def generate_code(args):
    from u_trace import utrace_generate
    from u_trace import utrace_generate_perfetto_utils

    utrace_generate(cpath=args.src, hpath=args.hdr,
                    ctx_param='struct si_ds_device *dev',
                    trace_toggle_name='si_gpu_tracepoint',
                    trace_toggle_defaults=si_default_tps)
    utrace_generate_perfetto_utils(hpath=args.perfetto_hdr)

def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('-p', '--import-path', required=True)
    parser.add_argument('-C','--src', required=True)
    parser.add_argument('-H','--hdr', required=True)
    parser.add_argument('--perfetto-hdr', required=True)
    args = parser.parse_args()
    sys.path.insert(0, args.import_path)
    define_tracepoints(args)
    generate_code(args)

if __name__ == '__main__':
    main()
