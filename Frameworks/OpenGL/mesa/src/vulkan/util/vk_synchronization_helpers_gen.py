COPYRIGHT=u"""
/* Copyright © 2023 Collabora, Ltd.
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
"""

import argparse
import os
import textwrap
import xml.etree.ElementTree as et

from mako.template import Template
from vk_extensions import get_api_list

TEMPLATE_C = Template(COPYRIGHT + """\
#include "vk_synchronization.h"

VkPipelineStageFlags2
vk_expand_pipeline_stage_flags2(VkPipelineStageFlags2 stages)
{
% for (group_stage, stages) in group_stages.items():
    if (stages & ${group_stage})
        stages |= ${' |\\n                  '.join(stages)};

% endfor
    if (stages & VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT) {
% for (guard, stage) in all_commands_stages:
% if guard is not None:
#ifdef ${guard}
% endif
        stages |= ${stage};
% if guard is not None:
#endif
% endif
% endfor
    }

    return stages;
}

VkAccessFlags2
vk_read_access2_for_pipeline_stage_flags2(VkPipelineStageFlags2 stages)
{
    VkAccessFlags2 access = 0;

% for ((guard, stages), access) in stages_read_access.items():
% if guard is not None:
#ifdef ${guard}
% endif
    if (stages & (${' |\\n                  '.join(stages)}))
        access |= ${' |\\n                  '.join(access)};
% if guard is not None:
#endif
% endif

% endfor
    return access;
}

VkAccessFlags2
vk_write_access2_for_pipeline_stage_flags2(VkPipelineStageFlags2 stages)
{
    VkAccessFlags2 access = 0;

% for ((guard, stages), access) in stages_write_access.items():
% if guard is not None:
#ifdef ${guard}
% endif
    if (stages & (${' |\\n                  '.join(stages)}))
        access |= ${' |\\n                  '.join(access)};
% if guard is not None:
#endif
% endif

% endfor
    return access;
}
""")

def get_guards(xml, api):
    guards = {}
    for ext_elem in xml.findall('./extensions/extension'):
        supported = get_api_list(ext_elem.attrib['supported'])
        if api not in supported:
            continue

        for enum in ext_elem.findall('./require/enum[@extends]'):
            if enum.attrib['extends'] not in ('VkPipelineStageFlagBits2',
                                              'VkAccessFlagBits2'):
                continue

            if 'protect' not in enum.attrib:
                continue

            name = enum.attrib['name']
            guard = enum.attrib['protect']
            guards[name] = guard

    return guards

def get_all_commands_stages(xml, guards):
    stages = []
    for stage in xml.findall('./sync/syncstage'):
        stage_name = stage.attrib['name']

        exclude = [
            # This isn't a real stage
            'VK_PIPELINE_STAGE_2_NONE',

            # These are real stages but they're a bit weird to include in
            # ALL_COMMANDS because they're context-dependent, depending on
            # whether they're part of srcStagesMask or dstStagesMask.
            #
            # We could avoid all grouped stages but then if someone adds
            # another group later, the behavior of this function may change in
            # a backwards-compatible way.  Also, the other ones aren't really
            # hurting anything if we add them in.
            'VK_PIPELINE_STAGE_2_TOP_OF_PIPE_BIT',
            'VK_PIPELINE_STAGE_2_BOTTOM_OF_PIPE_BIT',

            # This is all COMMANDS, not host.
            'VK_PIPELINE_STAGE_2_HOST_BIT',
        ]
        if stage_name in exclude:
            continue

        guard = guards.get(stage_name, None)
        stages.append((guard, stage_name))

    return stages

def get_group_stages(xml):
    group_stages = {}
    for stage in xml.findall('./sync/syncstage'):
        name = stage.attrib['name']
        equiv = stage.find('./syncequivalent')
        if equiv is not None:
            stages = equiv.attrib['stage'].split(',')
            group_stages[name] = stages

    return group_stages

def access_is_read(name):
    if 'READ' in name:
        assert 'WRITE' not in name
        return True
    elif 'WRITE' in name:
        return False
    else:
        print(name)
        assert False, "Invalid access bit name"

def get_stages_access(xml, read, guards):
    stages_access = {}
    for access in xml.findall('./sync/syncaccess'):
        access_name = access.attrib['name']
        if access_name == 'VK_ACCESS_2_NONE':
            continue

        if access_is_read(access_name) != read:
            continue

        guard = guards.get(access_name, None)
        support = access.find('./syncsupport')
        if support is not None:
            stages = support.attrib['stage'].split(',')
            stages.sort()
            key = (guard, tuple(stages))
            if key in stages_access:
                stages_access[key].append(access_name)
            else:
                stages_access[key] = [access_name]

    return stages_access

def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('--beta', required=True, help='Enable beta extensions.')
    parser.add_argument('--xml', required=True, help='Vulkan API XML file')
    parser.add_argument('--out-c', required=True, help='Output C file.')
    args = parser.parse_args()

    xml = et.parse(args.xml);

    guards = get_guards(xml, 'vulkan')
    environment = {
        'all_commands_stages': get_all_commands_stages(xml, guards),
        'group_stages': get_group_stages(xml),
        'stages_read_access': get_stages_access(xml, True, guards),
        'stages_write_access': get_stages_access(xml, False, guards),
    }

    try:
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
