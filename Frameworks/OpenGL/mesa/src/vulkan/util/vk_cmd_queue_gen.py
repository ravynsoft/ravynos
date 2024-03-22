COPYRIGHT=u"""
/* Copyright © 2015-2021 Intel Corporation
 * Copyright © 2021 Collabora, Ltd.
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
import re
from collections import namedtuple
import xml.etree.ElementTree as et

from mako.template import Template

# Mesa-local imports must be declared in meson variable
# '{file_without_suffix}_depend_files'.
from vk_entrypoints import EntrypointParam, get_entrypoints_from_xml
from vk_extensions import filter_api, get_all_required

# These have hand-typed implementations in vk_cmd_enqueue.c
MANUAL_COMMANDS = [
    # This script doesn't know how to copy arrays in structs in arrays
    'CmdPushDescriptorSetKHR',

    # The size of the elements is specified in a stride param
    'CmdDrawMultiEXT',
    'CmdDrawMultiIndexedEXT',

    # The VkPipelineLayout object could be released before the command is
    # executed
    'CmdBindDescriptorSets',
]

NO_ENQUEUE_COMMANDS = [
    # pData's size cannot be calculated from the xml
    'CmdPushConstants2KHR',
    'CmdPushDescriptorSet2KHR',
    'CmdPushDescriptorSetWithTemplate2KHR',
    'CmdPushDescriptorSetWithTemplateKHR',

    # These don't return void
    'CmdSetPerformanceMarkerINTEL',
    'CmdSetPerformanceStreamMarkerINTEL',
    'CmdSetPerformanceOverrideINTEL',
]

TEMPLATE_H = Template(COPYRIGHT + """\
/* This file generated from ${filename}, don't edit directly. */

#pragma once

#include "util/list.h"

#define VK_PROTOTYPES
#include <vulkan/vulkan_core.h>
#ifdef VK_ENABLE_BETA_EXTENSIONS
#include <vulkan/vulkan_beta.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

struct vk_device_dispatch_table;

struct vk_cmd_queue {
   const VkAllocationCallbacks *alloc;
   struct list_head cmds;
};

enum vk_cmd_type {
% for c in commands:
% if c.guard is not None:
#ifdef ${c.guard}
% endif
   ${to_enum_name(c.name)},
% if c.guard is not None:
#endif // ${c.guard}
% endif
% endfor
};

extern const char *vk_cmd_queue_type_names[];
extern size_t vk_cmd_queue_type_sizes[];

% for c in commands:
% if len(c.params) <= 1:             # Avoid "error C2016: C requires that a struct or union have at least one member"
<% continue %>
% endif
% if c.guard is not None:
#ifdef ${c.guard}
% endif
struct ${to_struct_name(c.name)} {
% for p in c.params[1:]:
   ${to_field_decl(p.decl)};
% endfor
};
% if c.guard is not None:
#endif // ${c.guard}
% endif
% endfor

struct vk_cmd_queue_entry;

/* this ordering must match vk_cmd_queue_entry */
struct vk_cmd_queue_entry_base {
   struct list_head cmd_link;
   enum vk_cmd_type type;
   void *driver_data;
   void (*driver_free_cb)(struct vk_cmd_queue *queue,
                          struct vk_cmd_queue_entry *cmd);
};

/* this ordering must match vk_cmd_queue_entry_base */
struct vk_cmd_queue_entry {
   struct list_head cmd_link;
   enum vk_cmd_type type;
   void *driver_data;
   void (*driver_free_cb)(struct vk_cmd_queue *queue,
                          struct vk_cmd_queue_entry *cmd);
   union {
% for c in commands:
% if len(c.params) <= 1:
<% continue %>
% endif
% if c.guard is not None:
#ifdef ${c.guard}
% endif
      struct ${to_struct_name(c.name)} ${to_struct_field_name(c.name)};
% if c.guard is not None:
#endif // ${c.guard}
% endif
% endfor
   } u;
};

% for c in commands:
% if c.name in manual_commands or c.name in no_enqueue_commands:
<% continue %>
% endif
% if c.guard is not None:
#ifdef ${c.guard}
% endif
  VkResult vk_enqueue_${to_underscore(c.name)}(struct vk_cmd_queue *queue
% for p in c.params[1:]:
   , ${p.decl}
% endfor
  );
% if c.guard is not None:
#endif // ${c.guard}
% endif

% endfor

void vk_free_queue(struct vk_cmd_queue *queue);

static inline void
vk_cmd_queue_init(struct vk_cmd_queue *queue, VkAllocationCallbacks *alloc)
{
   queue->alloc = alloc;
   list_inithead(&queue->cmds);
}

static inline void
vk_cmd_queue_reset(struct vk_cmd_queue *queue)
{
   vk_free_queue(queue);
   list_inithead(&queue->cmds);
}

static inline void
vk_cmd_queue_finish(struct vk_cmd_queue *queue)
{
   vk_free_queue(queue);
   list_inithead(&queue->cmds);
}

void vk_cmd_queue_execute(struct vk_cmd_queue *queue,
                          VkCommandBuffer commandBuffer,
                          const struct vk_device_dispatch_table *disp);

#ifdef __cplusplus
}
#endif
""")

TEMPLATE_C = Template(COPYRIGHT + """
/* This file generated from ${filename}, don't edit directly. */

#include "${header}"

#define VK_PROTOTYPES
#include <vulkan/vulkan_core.h>
#ifdef VK_ENABLE_BETA_EXTENSIONS
#include <vulkan/vulkan_beta.h>
#endif

#include "vk_alloc.h"
#include "vk_cmd_enqueue_entrypoints.h"
#include "vk_command_buffer.h"
#include "vk_dispatch_table.h"
#include "vk_device.h"

const char *vk_cmd_queue_type_names[] = {
% for c in commands:
% if c.guard is not None:
#ifdef ${c.guard}
% endif
   "${to_enum_name(c.name)}",
% if c.guard is not None:
#endif // ${c.guard}
% endif
% endfor
};

size_t vk_cmd_queue_type_sizes[] = {
% for c in commands:
% if c.guard is not None:
#ifdef ${c.guard}
% endif
% if len(c.params) > 1:
   sizeof(struct ${to_struct_name(c.name)}) +
% endif
   sizeof(struct vk_cmd_queue_entry_base),
% if c.guard is not None:
#endif // ${c.guard}
% endif
% endfor
};

% for c in commands:
% if c.guard is not None:
#ifdef ${c.guard}
% endif
static void
vk_free_${to_underscore(c.name)}(struct vk_cmd_queue *queue,
${' ' * len('vk_free_' + to_underscore(c.name) + '(')}\\
struct vk_cmd_queue_entry *cmd)
{
   if (cmd->driver_free_cb)
      cmd->driver_free_cb(queue, cmd);
   else
      vk_free(queue->alloc, cmd->driver_data);
% for p in c.params[1:]:
% if p.len:
   vk_free(queue->alloc, (${remove_suffix(p.decl.replace("const", ""), p.name)})cmd->u.${to_struct_field_name(c.name)}.${to_field_name(p.name)});
% elif '*' in p.decl:
   ${get_struct_free(c, p, types)}
% endif
% endfor
   vk_free(queue->alloc, cmd);
}

% if c.name not in manual_commands and c.name not in no_enqueue_commands:
VkResult vk_enqueue_${to_underscore(c.name)}(struct vk_cmd_queue *queue
% for p in c.params[1:]:
, ${p.decl}
% endfor
)
{
   struct vk_cmd_queue_entry *cmd = vk_zalloc(queue->alloc, vk_cmd_queue_type_sizes[${to_enum_name(c.name)}], 8,
                                              VK_SYSTEM_ALLOCATION_SCOPE_OBJECT);
   if (!cmd) return VK_ERROR_OUT_OF_HOST_MEMORY;

   cmd->type = ${to_enum_name(c.name)};
   \
   <% need_error_handling = False %>
% for p in c.params[1:]:
% if p.len:
   if (${p.name}) {
      ${get_array_copy(c, p)}
   }\
   <% need_error_handling = True %>
% elif '[' in p.decl:
   memcpy(cmd->u.${to_struct_field_name(c.name)}.${to_field_name(p.name)}, ${p.name},
          sizeof(*${p.name}) * ${get_array_len(p)});
% elif p.type == "void":
   cmd->u.${to_struct_field_name(c.name)}.${to_field_name(p.name)} = (${remove_suffix(p.decl.replace("const", ""), p.name)}) ${p.name};
% elif '*' in p.decl:
   ${get_struct_copy("cmd->u.%s.%s" % (to_struct_field_name(c.name), to_field_name(p.name)), p.name, p.type, 'sizeof(%s)' % p.type, types)}\
   <% need_error_handling = True %>
% else:
   cmd->u.${to_struct_field_name(c.name)}.${to_field_name(p.name)} = ${p.name};
% endif
% endfor

   list_addtail(&cmd->cmd_link, &queue->cmds);
   return VK_SUCCESS;

% if need_error_handling:
err:
   if (cmd)
      vk_free_${to_underscore(c.name)}(queue, cmd);
   return VK_ERROR_OUT_OF_HOST_MEMORY;
% endif
}
% endif
% if c.guard is not None:
#endif // ${c.guard}
% endif

% endfor

void
vk_free_queue(struct vk_cmd_queue *queue)
{
   struct vk_cmd_queue_entry *tmp, *cmd;
   LIST_FOR_EACH_ENTRY_SAFE(cmd, tmp, &queue->cmds, cmd_link) {
      switch(cmd->type) {
% for c in commands:
% if c.guard is not None:
#ifdef ${c.guard}
% endif
      case ${to_enum_name(c.name)}:
         vk_free_${to_underscore(c.name)}(queue, cmd);
         break;
% if c.guard is not None:
#endif // ${c.guard}
% endif
% endfor
      }
   }
}

void
vk_cmd_queue_execute(struct vk_cmd_queue *queue,
                     VkCommandBuffer commandBuffer,
                     const struct vk_device_dispatch_table *disp)
{
   list_for_each_entry(struct vk_cmd_queue_entry, cmd, &queue->cmds, cmd_link) {
      switch (cmd->type) {
% for c in commands:
% if c.guard is not None:
#ifdef ${c.guard}
% endif
      case ${to_enum_name(c.name)}:
          disp->${c.name}(commandBuffer
% for p in c.params[1:]:
             , cmd->u.${to_struct_field_name(c.name)}.${to_field_name(p.name)}\\
% endfor
          );
          break;
% if c.guard is not None:
#endif // ${c.guard}
% endif
% endfor
      default: unreachable("Unsupported command");
      }
   }
}

% for c in commands:
% if c.name in no_enqueue_commands:
/* TODO: Generate vk_cmd_enqueue_${c.name}() */
<% continue %>
% endif

% if c.guard is not None:
#ifdef ${c.guard}
% endif
<% assert c.return_type == 'void' %>

% if c.name in manual_commands:
/* vk_cmd_enqueue_${c.name}() is hand-typed in vk_cmd_enqueue.c */
% else:
VKAPI_ATTR void VKAPI_CALL
vk_cmd_enqueue_${c.name}(${c.decl_params()})
{
   VK_FROM_HANDLE(vk_command_buffer, cmd_buffer, commandBuffer);

   if (vk_command_buffer_has_error(cmd_buffer))
      return;
% if len(c.params) == 1:
   VkResult result = vk_enqueue_${to_underscore(c.name)}(&cmd_buffer->cmd_queue);
% else:
   VkResult result = vk_enqueue_${to_underscore(c.name)}(&cmd_buffer->cmd_queue,
                                       ${c.call_params(1)});
% endif
   if (unlikely(result != VK_SUCCESS))
      vk_command_buffer_set_error(cmd_buffer, result);
}
% endif

VKAPI_ATTR void VKAPI_CALL
vk_cmd_enqueue_unless_primary_${c.name}(${c.decl_params()})
{
    VK_FROM_HANDLE(vk_command_buffer, cmd_buffer, commandBuffer);

   if (cmd_buffer->level == VK_COMMAND_BUFFER_LEVEL_PRIMARY) {
      const struct vk_device_dispatch_table *disp =
         cmd_buffer->base.device->command_dispatch_table;

      disp->${c.name}(${c.call_params()});
   } else {
      vk_cmd_enqueue_${c.name}(${c.call_params()});
   }
}
% if c.guard is not None:
#endif // ${c.guard}
% endif
% endfor
""")

def remove_prefix(text, prefix):
    if text.startswith(prefix):
        return text[len(prefix):]
    return text

def remove_suffix(text, suffix):
    if text.endswith(suffix):
        return text[:-len(suffix)]
    return text

def to_underscore(name):
    return remove_prefix(re.sub('([A-Z]+)', r'_\1', name).lower(), '_')

def to_struct_field_name(name):
    return to_underscore(name).replace('cmd_', '')

def to_field_name(name):
    return remove_prefix(to_underscore(name).replace('cmd_', ''), 'p_')

def to_field_decl(decl):
    if 'const*' in decl:
        decl = decl.replace('const*', '*')
    else:
        decl = decl.replace('const ', '')
    [decl, name] = decl.rsplit(' ', 1)
    return decl + ' ' + to_field_name(name)

def to_enum_name(name):
    return "VK_%s" % to_underscore(name).upper()

def to_struct_name(name):
    return "vk_%s" % to_underscore(name)

def get_array_len(param):
    return param.decl[param.decl.find("[") + 1:param.decl.find("]")]

def get_array_copy(command, param):
    field_name = "cmd->u.%s.%s" % (to_struct_field_name(command.name), to_field_name(param.name))
    if param.type == "void":
        field_size = "1"
    else:
        field_size = "sizeof(*%s)" % field_name
    allocation = "%s = vk_zalloc(queue->alloc, %s * (%s), 8, VK_SYSTEM_ALLOCATION_SCOPE_OBJECT);\n   if (%s == NULL) goto err;\n" % (field_name, field_size, param.len, field_name)
    copy = "memcpy((void*)%s, %s, %s * (%s));" % (field_name, param.name, field_size, param.len)
    return "%s\n   %s" % (allocation, copy)

def get_array_member_copy(struct, src_name, member):
    field_name = "%s->%s" % (struct, member.name)
    if member.len == "struct-ptr":
        field_size = "sizeof(*%s)" % (field_name)
    else:
        field_size = "sizeof(*%s) * %s->%s" % (field_name, struct, member.len)
    allocation = "%s = vk_zalloc(queue->alloc, %s, 8, VK_SYSTEM_ALLOCATION_SCOPE_OBJECT);\n   if (%s == NULL) goto err;\n" % (field_name, field_size, field_name)
    copy = "memcpy((void*)%s, %s->%s, %s);" % (field_name, src_name, member.name, field_size)
    return "if (%s->%s) {\n   %s\n   %s\n}\n" % (src_name, member.name, allocation, copy)

def get_pnext_member_copy(struct, src_type, member, types, level):
    if not types[src_type].extended_by:
        return ""
    field_name = "%s->%s" % (struct, member.name)
    pnext_decl = "const VkBaseInStructure *pnext = %s;" % field_name
    case_stmts = ""
    for type in types[src_type].extended_by:
        guard_pre_stmt = ""
        guard_post_stmt = ""
        if type.guard is not None:
            guard_pre_stmt = "#ifdef %s" % type.guard
            guard_post_stmt = "#endif"
        case_stmts += """
%s
         case %s:
            %s
         break;
%s
      """ % (guard_pre_stmt, type.enum, get_struct_copy(field_name, "pnext", type.name, "sizeof(%s)" % type.name, types, level), guard_post_stmt)
    return """
      %s
      if (pnext) {
         switch ((int32_t)pnext->sType) {
         %s
         }
      }
      """ % (pnext_decl, case_stmts)

def get_struct_copy(dst, src_name, src_type, size, types, level=0):
    global tmp_dst_idx
    global tmp_src_idx

    allocation = "%s = vk_zalloc(queue->alloc, %s, 8, VK_SYSTEM_ALLOCATION_SCOPE_OBJECT);\n      if (%s == NULL) goto err;\n" % (dst, size, dst)
    copy = "memcpy((void*)%s, %s, %s);" % (dst, src_name, size)

    level += 1
    tmp_dst = "%s *tmp_dst%d = (void *) %s; (void) tmp_dst%d;" % (src_type, level, dst, level)
    tmp_src = "%s *tmp_src%d = (void *) %s; (void) tmp_src%d;" % (src_type, level, src_name, level)

    member_copies = ""
    if src_type in types:
        for member in types[src_type].members:
            if member.len and member.len != 'null-terminated':
                member_copies += get_array_member_copy("tmp_dst%d" % level, "tmp_src%d" % level, member)
            elif member.name == 'pNext':
                member_copies += get_pnext_member_copy("tmp_dst%d" % level, src_type, member, types, level)

    null_assignment = "%s = NULL;" % dst
    if_stmt = "if (%s) {" % src_name
    indent = "   " * level
    return "%s\n      %s\n      %s\n      %s\n      %s\n      %s\n%s} else {\n      %s\n%s}" % (if_stmt, allocation, copy, tmp_dst, tmp_src, member_copies, indent, null_assignment, indent)

def get_struct_free(command, param, types):
    field_name = "cmd->u.%s.%s" % (to_struct_field_name(command.name), to_field_name(param.name))
    const_cast = remove_suffix(param.decl.replace("const", ""), param.name)
    struct_free = "vk_free(queue->alloc, (%s)%s);" % (const_cast, field_name)
    member_frees = ""
    if (param.type in types):
        for member in types[param.type].members:
            if member.len and member.len != 'null-terminated':
                member_name = "cmd->u.%s.%s->%s" % (to_struct_field_name(command.name), to_field_name(param.name), member.name)
                const_cast = remove_suffix(member.decl.replace("const", ""), member.name)
                member_frees += "vk_free(queue->alloc, (%s)%s);\n" % (const_cast, member_name)
    return "%s      %s\n" % (member_frees, struct_free)

EntrypointType = namedtuple('EntrypointType', 'name enum members extended_by guard')

def get_types_defines(doc):
    """Maps types to extension defines."""
    types_to_defines = {}

    platform_define = {}
    for platform in doc.findall('./platforms/platform'):
        name = platform.attrib['name']
        define = platform.attrib['protect']
        platform_define[name] = define

    for extension in doc.findall('./extensions/extension[@platform]'):
        platform = extension.attrib['platform']
        define = platform_define[platform]

        for types in extension.findall('./require/type'):
            fullname = types.attrib['name']
            types_to_defines[fullname] = define

    return types_to_defines

def get_types(doc, beta, api, types_to_defines):
    """Extract the types from the registry."""
    types = {}

    required = get_all_required(doc, 'type', api, beta)

    for _type in doc.findall('./types/type'):
        if _type.attrib.get('category') != 'struct':
            continue
        if not filter_api(_type, api):
            continue
        if _type.attrib['name'] not in required:
            continue

        members = []
        type_enum = None
        for p in _type.findall('./member'):
            if not filter_api(p, api):
                continue

            mem_type = p.find('./type').text
            mem_name = p.find('./name').text
            mem_decl = ''.join(p.itertext())
            mem_len = p.attrib.get('altlen', p.attrib.get('len', None))
            if mem_len is None and '*' in mem_decl and mem_name != 'pNext':
                mem_len = "struct-ptr"

            member = EntrypointParam(type=mem_type,
                                     name=mem_name,
                                     decl=mem_decl,
                                     len=mem_len)
            members.append(member)

            if mem_name == 'sType':
                type_enum = p.attrib.get('values')
        types[_type.attrib['name']] = EntrypointType(name=_type.attrib['name'], enum=type_enum, members=members, extended_by=[], guard=types_to_defines.get(_type.attrib['name']))

    for _type in doc.findall('./types/type'):
        if _type.attrib.get('category') != 'struct':
            continue
        if not filter_api(_type, api):
            continue
        if _type.attrib['name'] not in required:
            continue
        if _type.attrib.get('structextends') is None:
            continue
        for extended in _type.attrib.get('structextends').split(','):
            if extended not in required:
                continue
            types[extended].extended_by.append(types[_type.attrib['name']])

    return types

def get_types_from_xml(xml_files, beta, api='vulkan'):
    types = {}

    for filename in xml_files:
        doc = et.parse(filename)
        types.update(get_types(doc, beta, api, get_types_defines(doc)))

    return types

def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('--out-c', required=True, help='Output C file.')
    parser.add_argument('--out-h', required=True, help='Output H file.')
    parser.add_argument('--beta', required=True, help='Enable beta extensions.')
    parser.add_argument('--xml',
                        help='Vulkan API XML file.',
                        required=True, action='append', dest='xml_files')
    args = parser.parse_args()

    commands = []
    for e in get_entrypoints_from_xml(args.xml_files, args.beta):
        if e.name.startswith('Cmd') and \
           not e.alias:
            commands.append(e)

    types = get_types_from_xml(args.xml_files, args.beta)

    assert os.path.dirname(args.out_c) == os.path.dirname(args.out_h)

    environment = {
        'header': os.path.basename(args.out_h),
        'commands': commands,
        'filename': os.path.basename(__file__),
        'to_underscore': to_underscore,
        'get_array_len': get_array_len,
        'to_struct_field_name': to_struct_field_name,
        'to_field_name': to_field_name,
        'to_field_decl': to_field_decl,
        'to_enum_name': to_enum_name,
        'to_struct_name': to_struct_name,
        'get_array_copy': get_array_copy,
        'get_struct_copy': get_struct_copy,
        'get_struct_free': get_struct_free,
        'types': types,
        'manual_commands': MANUAL_COMMANDS,
        'no_enqueue_commands': NO_ENQUEUE_COMMANDS,
        'remove_suffix': remove_suffix,
    }

    try:
        with open(args.out_h, 'w', encoding='utf-8') as f:
            guard = os.path.basename(args.out_h).replace('.', '_').upper()
            f.write(TEMPLATE_H.render(guard=guard, **environment))
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
