#
# Copyright © 2021 Intel Corporation
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
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
# SOFTWARE.
#

import argparse
import sys

# List of the default tracepoints enabled. By default most tracepoints are
# enabled, set tp_default=False to disable them by default.
#
# Currently only stall is disabled by default
intel_default_tps = []

#
# Tracepoint definitions:
#
def define_tracepoints(args):
    from u_trace import Header, HeaderScope
    from u_trace import ForwardDecl
    from u_trace import Tracepoint
    from u_trace import TracepointArg as Arg
    from u_trace import TracepointArgStruct as ArgStruct

    Header('intel_driver_ds.h', scope=HeaderScope.SOURCE)
    Header('blorp/blorp_priv.h', scope=HeaderScope.HEADER)
    Header('ds/intel_driver_ds.h', scope=HeaderScope.HEADER)

    def begin_end_tp(name, tp_args=[], tp_struct=None, tp_print=None,
                     tp_default_enabled=True, end_pipelined=True,
                     need_cs_param=False):
        global intel_default_tps
        if tp_default_enabled:
            intel_default_tps.append(name)
        Tracepoint('intel_begin_{0}'.format(name),
                   toggle_name=name,
                   tp_perfetto='intel_ds_begin_{0}'.format(name),
                   need_cs_param=need_cs_param)
        Tracepoint('intel_end_{0}'.format(name),
                   toggle_name=name,
                   args=tp_args,
                   tp_struct=tp_struct,
                   tp_perfetto='intel_ds_end_{0}'.format(name),
                   tp_print=tp_print,
                   end_of_pipe=end_pipelined,
                   need_cs_param=need_cs_param)

    # Frame tracepoints
    begin_end_tp('frame',
                 tp_args=[Arg(type='uint32_t', var='frame', c_format='%u'),],
                 end_pipelined=False,
                 need_cs_param=True)

    # Annotations for Queue(Begin|End)DebugUtilsLabelEXT
    begin_end_tp('queue_annotation',
                 tp_args=[ArgStruct(type='unsigned', var='len'),
                          ArgStruct(type='const char *', var='str'),],
                 tp_struct=[Arg(type='uint8_t', name='dummy', var='0', c_format='%hhu'),
                            Arg(type='char', name='str', var='str', c_format='%s', length_arg='len + 1', copy_func='strncpy'),],
                 end_pipelined=False,
                 need_cs_param=True)

    # Batch buffer tracepoints, only for Iris
    begin_end_tp('batch',
                 tp_args=[Arg(type='uint8_t', var='name', c_format='%hhu'),],
                 end_pipelined=False)

    # Command buffer tracepoints, only for Anv
    begin_end_tp('cmd_buffer',
                 tp_args=[Arg(type='uint8_t', var='level', c_format='%hhu'),],
                 end_pipelined=False)

    # Annotations for Cmd(Begin|End)DebugUtilsLabelEXT
    begin_end_tp('cmd_buffer_annotation',
                 tp_args=[ArgStruct(type='unsigned', var='len'),
                          ArgStruct(type='const char *', var='str'),],
                 tp_struct=[Arg(type='uint8_t', name='dummy', var='0', c_format='%hhu'),
                            Arg(type='char', name='str', var='str', c_format='%s', length_arg='len + 1', copy_func='strncpy'),],
                 end_pipelined=True)

    # Transform feedback, only for Anv
    begin_end_tp('xfb',
                 end_pipelined=False)

    # Dynamic rendering tracepoints, only for Anv
    begin_end_tp('render_pass',
                 tp_args=[Arg(type='uint16_t', var='width', c_format='%hu'),
                          Arg(type='uint16_t', var='height', c_format='%hu'),
                          Arg(type='uint8_t', var='att_count', c_format='%hhu'),
                          Arg(type='uint8_t', var='msaa', c_format='%hhu'),])

    # Blorp operations, Anv & Iris
    begin_end_tp('blorp',
                 tp_args=[Arg(type='enum blorp_op', name='op', var='op', c_format='%s', to_prim_type='blorp_op_to_name({})'),
                          Arg(type='uint32_t', name='width', var='width', c_format='%u'),
                          Arg(type='uint32_t', name='height', var='height', c_format='%u'),
                          Arg(type='uint32_t', name='samples', var='samples', c_format='%u'),
                          Arg(type='enum blorp_shader_pipeline', name='blorp_pipe', var='shader_pipe', c_format='%s', to_prim_type='blorp_shader_pipeline_to_name({})'),
                          Arg(type='enum isl_format', name='dst_fmt', var='dst_fmt', c_format='%s', to_prim_type='isl_format_get_short_name({})'),
                          Arg(type='enum isl_format', name='src_fmt', var='src_fmt', c_format='%s', to_prim_type='isl_format_get_short_name({})'),
                          ])

    # vkCmdWriteBufferMarker*, only for Anv
    begin_end_tp('write_buffer_marker',
                 end_pipelined=False)

    # Indirect draw generation, only for Anv
    begin_end_tp('generate_draws')

    # vkCmdResetQuery, only for Anv
    begin_end_tp('query_clear_blorp',
                 tp_args=[Arg(type='uint32_t', var='count', c_format='%u')])
    begin_end_tp('query_clear_cs',
                 tp_args=[Arg(type='uint32_t', var='count', c_format='%u')],
                 end_pipelined=False)

    # vkCmdCopyQueryResults, only for Anv
    begin_end_tp('query_copy_cs',
                 tp_args=[Arg(type='uint32_t', var='count', c_format='%u')],
                 end_pipelined=False)
    begin_end_tp('query_copy_shader',
                 tp_args=[Arg(type='uint32_t', var='count', c_format='%u')])

    # Various draws/dispatch, Anv & Iris
    begin_end_tp('draw',
                 tp_args=[Arg(type='uint32_t', var='count', c_format='%u')])
    begin_end_tp('draw_multi',
                 tp_args=[Arg(type='uint32_t', var='count', c_format='%u'),])
    begin_end_tp('draw_indexed',
                 tp_args=[Arg(type='uint32_t', var='count', c_format='%u'),])
    begin_end_tp('draw_indexed_multi',
                 tp_args=[Arg(type='uint32_t', var='count', c_format='%u'),])
    begin_end_tp('draw_indirect_byte_count',
                 tp_args=[Arg(type='uint32_t', var='instance_count', c_format='%u'),])
    begin_end_tp('draw_indirect',
                 tp_args=[Arg(type='uint32_t', var='draw_count', c_format='%u'),])
    begin_end_tp('draw_indexed_indirect',
                 tp_args=[Arg(type='uint32_t', var='draw_count', c_format='%u'),])
    begin_end_tp('draw_indirect_count',
                 tp_args=[Arg(type='uint32_t', var='max_draw_count', c_format='%u'),])
    begin_end_tp('draw_indexed_indirect_count',
                 tp_args=[Arg(type='uint32_t', var='max_draw_count', c_format='%u'),])

    begin_end_tp('draw_mesh',
                 tp_args=[Arg(type='uint32_t', var='group_x', c_format='%u'),
                          Arg(type='uint32_t', var='group_y', c_format='%u'),
                          Arg(type='uint32_t', var='group_z', c_format='%u'),],
                 tp_print=['group=%ux%ux%u', '__entry->group_x', '__entry->group_y', '__entry->group_z'])
    begin_end_tp('draw_mesh_indirect',
                 tp_args=[Arg(type='uint32_t', var='draw_count', c_format='%u'),])
    begin_end_tp('draw_mesh_indirect_count',
                 tp_args=[Arg(type='uint32_t', var='max_draw_count', c_format='%u'),])

    begin_end_tp('compute',
                 tp_args=[Arg(type='uint32_t', var='group_x', c_format='%u'),
                          Arg(type='uint32_t', var='group_y', c_format='%u'),
                          Arg(type='uint32_t', var='group_z', c_format='%u'),],
                 tp_print=['group=%ux%ux%u', '__entry->group_x', '__entry->group_y', '__entry->group_z'])

    # Used to identify copies generated by utrace
    begin_end_tp('trace_copy',
                 tp_args=[Arg(type='uint32_t', var='count', c_format='%u'),])
    begin_end_tp('trace_copy_cb',
                 tp_args=[Arg(type='uint32_t', var='count', c_format='%u'),],
                 need_cs_param=True)

    begin_end_tp('as_build')

    begin_end_tp('rays',
                 tp_args=[Arg(type='uint32_t', var='group_x', c_format='%u'),
                          Arg(type='uint32_t', var='group_y', c_format='%u'),
                          Arg(type='uint32_t', var='group_z', c_format='%u'),],
                 tp_print=['group=%ux%ux%u', '__entry->group_x', '__entry->group_y', '__entry->group_z'])

    def flag_bits(args):
        bits = [Arg(type='enum intel_ds_stall_flag', name='flags', var='decode_cb(flags)', c_format='0x%x')]
        for a in args:
            bits.append(Arg(type='bool', name=a[1], var='__entry->flags & INTEL_DS_{0}_BIT'.format(a[0]), c_format='%u'))
        return bits

    def stall_args(args):
        fmt = ''
        exprs = []
        for a in args:
            fmt += '%s'
            exprs.append('(__entry->flags & INTEL_DS_{0}_BIT) ? "+{1}" : ""'.format(a[0], a[1]))
        fmt += ' : %s'
        exprs.append('__entry->reason ? __entry->reason : "unknown"')
        # To printout flags
        # fmt += '(0x%08x)'
        # exprs.append('__entry->flags')
        fmt = [fmt]
        fmt += exprs
        return fmt

    stall_flags = [['DEPTH_CACHE_FLUSH',             'depth_flush'],
                   ['DATA_CACHE_FLUSH',              'dc_flush'],
                   ['HDC_PIPELINE_FLUSH',            'hdc_flush'],
                   ['RENDER_TARGET_CACHE_FLUSH',     'rt_flush'],
                   ['TILE_CACHE_FLUSH',              'tile_flush'],
                   ['STATE_CACHE_INVALIDATE',        'state_inval'],
                   ['CONST_CACHE_INVALIDATE',        'const_inval'],
                   ['VF_CACHE_INVALIDATE',           'vf_inval'],
                   ['TEXTURE_CACHE_INVALIDATE',      'tex_inval'],
                   ['INST_CACHE_INVALIDATE',         'ic_inval'],
                   ['STALL_AT_SCOREBOARD',           'pb_stall'],
                   ['DEPTH_STALL',                   'depth_stall'],
                   ['CS_STALL',                      'cs_stall'],
                   ['UNTYPED_DATAPORT_CACHE_FLUSH',  'udp_flush'],
                   ['PSS_STALL_SYNC',                'pss_stall'],
                   ['END_OF_PIPE',                   'eop'],
                   ['CCS_CACHE_FLUSH',               'ccs_flush']]

    begin_end_tp('stall',
                 tp_args=[ArgStruct(type='uint32_t', var='flags'),
                          ArgStruct(type='intel_ds_stall_cb_t', var='decode_cb'),
                          ArgStruct(type='const char *', var='reason'),],
                 tp_struct=[Arg(type='uint32_t', name='flags', var='decode_cb(flags)', c_format='0x%x'),
                            Arg(type='const char *', name='reason', var='reason', c_format='%s'),],
                 tp_print=stall_args(stall_flags),
                 tp_default_enabled=False,
                 end_pipelined=False)


def generate_code(args):
    from u_trace import utrace_generate
    from u_trace import utrace_generate_perfetto_utils

    utrace_generate(cpath=args.utrace_src, hpath=args.utrace_hdr,
                    ctx_param='struct intel_ds_device *dev',
                    trace_toggle_name='intel_gpu_tracepoint',
                    trace_toggle_defaults=intel_default_tps)
    utrace_generate_perfetto_utils(hpath=args.perfetto_hdr,
                                   basename="intel_tracepoint")


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('-p', '--import-path', required=True)
    parser.add_argument('--utrace-src', required=True)
    parser.add_argument('--utrace-hdr', required=True)
    parser.add_argument('--perfetto-hdr', required=True)
    args = parser.parse_args()
    sys.path.insert(0, args.import_path)
    define_tracepoints(args)
    generate_code(args)


if __name__ == '__main__':
    main()
