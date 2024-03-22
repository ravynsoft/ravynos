# Copyright © 2021 Igalia S.L.
# SPDX-License-Identifier: MIT

import argparse
import sys

#
# TODO can we do this with less boilerplate?
#
parser = argparse.ArgumentParser()
parser.add_argument('-p', '--import-path', required=True)
parser.add_argument('--utrace-src', required=True)
parser.add_argument('--utrace-hdr', required=True)
parser.add_argument('--perfetto-hdr', required=True)
args = parser.parse_args()
sys.path.insert(0, args.import_path)


from u_trace import Header, HeaderScope
from u_trace import ForwardDecl
from u_trace import Tracepoint
from u_trace import TracepointArg as Arg
from u_trace import TracepointArgStruct as ArgStruct
from u_trace import utrace_generate
from u_trace import utrace_generate_perfetto_utils

Header('vk_enum_to_str.h', scope=HeaderScope.SOURCE|HeaderScope.PERFETTO)
Header('vk_format.h')
Header('tu_cmd_buffer.h', scope=HeaderScope.SOURCE)
Header('tu_device.h', scope=HeaderScope.SOURCE)

# we can't use tu_common.h because it includes ir3 headers which are not
# compatible with C++
ForwardDecl('struct tu_cmd_buffer')
ForwardDecl('struct tu_device')
ForwardDecl('struct tu_framebuffer')
ForwardDecl('struct tu_tiling_config')

# List of the default tracepoints enabled. By default tracepoints are enabled,
# set tp_default_enabled=False to disable them by default.
tu_default_tps = []

#
# Tracepoint definitions:
#

def begin_end_tp(name, args=[], tp_struct=None, tp_print=None,
                 tp_default_enabled=True, marker_tp=True,
                 queue_tp=True):
    global tu_default_tps
    if tp_default_enabled:
        tu_default_tps.append(name)
    Tracepoint('start_{0}'.format(name),
               toggle_name=name,
               args=args,
               tp_struct=tp_struct,
               tp_perfetto='tu_perfetto_start_{0}'.format(name) if queue_tp else None,
               tp_print=tp_print if queue_tp else None,
               tp_markers='tu_cs_trace_start' if marker_tp else None)
    Tracepoint('end_{0}'.format(name),
               toggle_name=name,
               tp_perfetto='tu_perfetto_end_{0}'.format(name),
               tp_markers='tu_cs_trace_end' if marker_tp else None)

begin_end_tp('cmd_buffer',
    args=[ArgStruct(type='const struct tu_cmd_buffer *', var='cmd')],
    tp_struct=[Arg(type='VkCommandBufferLevel', name='level', var='cmd->vk.level', c_format='%s', to_prim_type='vk_CommandBufferLevel_to_str({})'),
               Arg(type='uint8_t', name='render_pass_continue', var='!!(cmd->usage_flags & VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT)', c_format='%u')])

begin_end_tp('render_pass',
    args=[ArgStruct(type='const struct tu_framebuffer *', var='fb'),
          ArgStruct(type='const struct tu_tiling_config *', var='tiling')],
    tp_struct=[Arg(type='uint16_t', name='width',        var='fb->width',                                    c_format='%u'),
               Arg(type='uint16_t', name='height',       var='fb->height',                                   c_format='%u'),
               Arg(type='uint8_t',  name='attachment_count', var='fb->attachment_count',                     c_format='%u'),
            #    Arg(type='uint8_t',  name='samples',      var='fb->samples',                                  c_format='%u'),
               Arg(type='uint16_t', name='numberOfBins', var='tiling->tile_count.width * tiling->tile_count.height', c_format='%u'),
               Arg(type='uint16_t', name='binWidth',     var='tiling->tile0.width',                                  c_format='%u'),
               Arg(type='uint16_t', name='binHeight',    var='tiling->tile0.height',                                 c_format='%u')])

begin_end_tp('binning_ib')
begin_end_tp('draw_ib_sysmem')
begin_end_tp('draw_ib_gmem')

begin_end_tp('gmem_clear',
    args=[Arg(type='enum VkFormat',  var='format',  c_format='%s', to_prim_type='vk_format_description({})->short_name'),
          Arg(type='uint8_t',        var='samples', c_format='%u')])

begin_end_tp('sysmem_clear',
    args=[Arg(type='enum VkFormat',  var='format',      c_format='%s', to_prim_type='vk_format_description({})->short_name'),
          Arg(type='uint8_t',        var='uses_3d_ops', c_format='%u'),
          Arg(type='uint8_t',        var='samples',     c_format='%u')])

begin_end_tp('sysmem_clear_all',
    args=[Arg(type='uint8_t',        var='mrt_count',   c_format='%u'),
          Arg(type='uint8_t',        var='rect_count',  c_format='%u')])

begin_end_tp('gmem_load',
    args=[Arg(type='enum VkFormat',  var='format',   c_format='%s', to_prim_type='vk_format_description({})->short_name'),
          Arg(type='uint8_t',        var='force_load', c_format='%u')])

begin_end_tp('gmem_store',
    args=[Arg(type='enum VkFormat',  var='format',   c_format='%s', to_prim_type='vk_format_description({})->short_name'),
          Arg(type='uint8_t',        var='fast_path', c_format='%u'),
          Arg(type='uint8_t',        var='unaligned', c_format='%u')])

begin_end_tp('sysmem_resolve',
    args=[Arg(type='enum VkFormat',  var='format',   c_format='%s', to_prim_type='vk_format_description({})->short_name')])

begin_end_tp('blit',
    # TODO: add source megapixels count and target megapixels count arguments
    args=[Arg(type='uint8_t',        var='uses_3d_blit', c_format='%u'),
          Arg(type='enum VkFormat',  var='src_format',   c_format='%s', to_prim_type='vk_format_description({})->short_name'),
          Arg(type='enum VkFormat',  var='dst_format',   c_format='%s', to_prim_type='vk_format_description({})->short_name'),
          Arg(type='uint8_t',        var='layers',       c_format='%u')])

begin_end_tp('compute',
    args=[Arg(type='uint8_t',  var='indirect',       c_format='%u'),
          Arg(type='uint16_t', var='local_size_x',   c_format='%u'),
          Arg(type='uint16_t', var='local_size_y',   c_format='%u'),
          Arg(type='uint16_t', var='local_size_z',   c_format='%u'),
          Arg(type='uint16_t', var='num_groups_x',   c_format='%u'),
          Arg(type='uint16_t', var='num_groups_y',   c_format='%u'),
          Arg(type='uint16_t', var='num_groups_z',   c_format='%u')])


# Annotations for Cmd(Begin|End)DebugUtilsLabelEXT
for suffix in ["", "_rp"]:
    begin_end_tp('cmd_buffer_annotation' + suffix,
                    args=[ArgStruct(type='unsigned', var='len'),
                          ArgStruct(type='const char *', var='str'),],
                    tp_struct=[Arg(type='uint8_t', name='dummy', var='0', c_format='%hhu'),
                               Arg(type='char', name='str', var='str', c_format='%s', length_arg='len + 1', copy_func='strncpy'),])

utrace_generate(cpath=args.utrace_src,
                hpath=args.utrace_hdr,
                ctx_param='struct tu_device *dev',
                trace_toggle_name='tu_gpu_tracepoint',
                trace_toggle_defaults=tu_default_tps)
utrace_generate_perfetto_utils(hpath=args.perfetto_hdr)
