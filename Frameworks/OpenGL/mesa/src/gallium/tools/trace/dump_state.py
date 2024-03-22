#!/usr/bin/env python3
##########################################################################
#
# Copyright 2008-2021, VMware, Inc.
# All Rights Reserved.
#
# Permission is hereby granted, free of charge, to any person obtaining a
# copy of this software and associated documentation files (the
# "Software"), to deal in the Software without restriction, including
# without limitation the rights to use, copy, modify, merge, publish,
# distribute, sub license, and/or sell copies of the Software, and to
# permit persons to whom the Software is furnished to do so, subject to
# the following conditions:
#
# The above copyright notice and this permission notice (including the
# next paragraph) shall be included in all copies or substantial portions
# of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
# OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
# MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT.
# IN NO EVENT SHALL VMWARE AND/OR ITS SUPPLIERS BE LIABLE FOR
# ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
# TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
# SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
#
##########################################################################


import sys
import struct
import json
import binascii
import re
import copy
import argparse

import model
import format
import parse as parser


try:
    from struct import unpack_from
except ImportError:
    def unpack_from(fmt, buf, offset=0):
        size = struct.calcsize(fmt)
        return struct.unpack(fmt, buf[offset:offset + size])

#
# Some constants
#
PIPE_BUFFER = 'PIPE_BUFFER'


def serialize(obj):
    '''JSON serializer function for non-standard Python objects.'''
    if isinstance(obj, bytearray) or isinstance(obj, bytes):
        # TODO: Decide on a single way of dumping blobs
        if False:
            # Don't dump full blobs, but merely a description of their size and
            # CRC32 hash.
            crc32 = binascii.crc32(obj)
            if crc32 < 0:
                crc32 += 0x100000000
            return 'blob(size=%u,crc32=0x%08x)' % (len(obj), crc32)
        if True:
            # Dump blobs as an array of 16byte hexadecimals
            res = []
            for i in range(0, len(obj), 16):
                res.append(obj[i : i + 16].hex())
            return res
        # Dump blobs as a single hexadecimal string
        return obj.hex()

    # If the object has a __json__ method, use it.
    try:
        method = obj.__json__
    except AttributeError:
        raise TypeError(obj)
    else:
        return method()


class Struct:
    """C-like struct.

    Python doesn't have C structs, but do its dynamic nature, any object is
    pretty close.
    """

    def __json__(self):
        '''Convert the structure to a standard Python dict, so it can be
        serialized.'''

        obj = {}
        for name, value in self.__dict__.items():
            if not name.startswith('_'):
                obj[name] = value
        return obj

    def __repr__(self):
        return repr(self.__json__())


class Translator(model.Visitor):
    """Translate model arguments into regular Python objects"""

    def __init__(self, interpreter):
        self.interpreter = interpreter
        self.result = None

    def visit(self, node):
        self.result = None
        node.visit(self)
        return self.result

    def visit_literal(self, node):
        self.result = node.value

    def visit_blob(self, node):
        self.result = node

    def visit_named_constant(self, node):
        self.result = node.name

    def visit_array(self, node):
        array = []
        for element in node.elements:
            array.append(self.visit(element))
        self.result = array

    def visit_struct(self, node):
        struct = Struct()
        for member_name, member_node in node.members:
            member_name = member_name.replace('.', '_')
            member_value = self.visit(member_node)
            setattr(struct, member_name, member_value)
        self.result = struct

    def visit_pointer(self, node):
        self.result = self.interpreter.lookup_object(node.address)


class Dispatcher:
    '''Base class for classes whose methods can dispatch Gallium calls.'''

    def __init__(self, interpreter):
        self.interpreter = interpreter


class Global(Dispatcher):
    '''Global name space.

    For calls that are not associated with objects, i.e, functions and not
    methods.
    '''

    def pipe_screen_create(self):
        return Screen(self.interpreter)

    def pipe_context_create(self, screen):
        return screen.context_create()


class Transfer:
    '''pipe_transfer'''

    def __init__(self, resource, usage, subresource, box):
        self.resource = resource
        self.usage = usage
        self.subresource = subresource
        self.box = box


class Screen(Dispatcher):
    '''pipe_screen'''

    def __init__(self, interpreter):
        Dispatcher.__init__(self, interpreter)

    def destroy(self):
        pass

    def context_create(self, priv=None, flags=0):
        return Context(self.interpreter)

    def resource_create(self, templat):
        resource = templat
        # Normalize state to avoid spurious differences
        if resource.nr_samples == 0:
            resource.nr_samples = 1
        if resource.target == PIPE_BUFFER:
            # We will keep track of buffer contents
            resource.data = bytearray(resource.width)
            # Ignore format
            del resource.format
        return resource

    resource_create_unbacked = resource_create

    def allocate_memory(screen, size):
        pass

    def free_memory(screen, pmem):
        pass

    def map_memory(screen, pmem):
        pass

    def unmap_memory(screen, pmem):
        pass

    def resource_bind_backing(screen, resource, pmem, offset):
        pass

    def resource_destroy(self, resource):
        self.interpreter.unregister_object(resource)

    def fence_finish(self, fence, timeout=None, ctx=None):
        pass

    def fence_signalled(self, fence):
        pass

    def fence_reference(self, dst, src):
        pass

    def flush_frontbuffer(self, resource):
        pass


class Context(Dispatcher):
    '''pipe_context'''

    # Internal methods variable should be prefixed with '_'

    def __init__(self, interpreter):
        Dispatcher.__init__(self, interpreter)

        # Setup initial state
        self._state = Struct()
        self._state.scissors = []
        self._state.viewports = []
        self._state.vertex_buffers = []
        self._state.vertex_elements = []
        self._state.vs = Struct()
        self._state.tcs = Struct()
        self._state.tes = Struct()
        self._state.gs = Struct()
        self._state.fs = Struct()
        self._state.vs.shader = None
        self._state.tcs.shader = None
        self._state.tes.shader = None
        self._state.gs.shader = None
        self._state.fs.shader = None
        self._state.vs.sampler = []
        self._state.tcs.sampler = []
        self._state.tes.sampler = []
        self._state.gs.sampler = []
        self._state.fs.sampler = []
        self._state.vs.sampler_views = []
        self._state.tcs.sampler_views = []
        self._state.tes.sampler_views = []
        self._state.gs.sampler_views = []
        self._state.fs.sampler_views = []
        self._state.vs.constant_buffer = []
        self._state.tcs.constant_buffer = []
        self._state.tes.constant_buffer = []
        self._state.gs.constant_buffer = []
        self._state.fs.constant_buffer = []
        self._state.render_condition_condition = 0
        self._state.render_condition_mode = 0
        self._state.sample_mask = 0xffffffff

        self._draw_no = 0

    def destroy(self):
        pass

    def create_blend_state(self, state):
        # Normalize state to avoid spurious differences
        if not state.logicop_enable:
            del state.logicop_func
        if not state.rt[0].blend_enable:
            del state.rt[0].rgb_src_factor
            del state.rt[0].rgb_dst_factor
            del state.rt[0].rgb_func
            del state.rt[0].alpha_src_factor
            del state.rt[0].alpha_dst_factor
            del state.rt[0].alpha_func
        return state

    def bind_blend_state(self, state):
        # Normalize state
        self._state.blend = state

    def delete_blend_state(self, state):
        pass

    def create_sampler_state(self, state):
        return state

    def delete_sampler_state(self, state):
        pass

    def bind_sampler_states(self, shader, start, num_states, states):
        # FIXME: Handle non-zero start
        assert start == 0
        self._get_stage_state(shader).sampler = states

    def bind_vertex_sampler_states(self, num_states, states):
        # XXX: deprecated method
        self._state.vs.sampler = states

    def bind_geometry_sampler_states(self, num_states, states):
        # XXX: deprecated method
        self._state.gs.sampler = states

    def bind_fragment_sampler_states(self, num_states, states):
        # XXX: deprecated method
        self._state.fs.sampler = states

    def create_rasterizer_state(self, state):
        return state

    def bind_rasterizer_state(self, state):
        self._state.rasterizer = state

    def delete_rasterizer_state(self, state):
        pass

    def create_depth_stencil_alpha_state(self, state):
        # Normalize state to avoid spurious differences
        if not state.alpha_enabled:
            del state.alpha_func
            del state.alpha_ref_value
        for i in range(2):
            if not state.stencil[i].enabled:
                del state.stencil[i].func
        return state

    def bind_depth_stencil_alpha_state(self, state):
        self._state.depth_stencil_alpha = state

    def delete_depth_stencil_alpha_state(self, state):
        pass

    _tokenLabelRE = re.compile('^\s*\d+: ', re.MULTILINE)

    def _create_shader_state(self, state):
        # Strip the labels from the tokens
        if state.tokens is not None:
            state.tokens = self._tokenLabelRE.sub('', state.tokens)
        return state

    create_vs_state = _create_shader_state
    create_tcs_state = _create_shader_state
    create_tes_state = _create_shader_state
    create_gs_state = _create_shader_state
    create_fs_state = _create_shader_state

    def bind_vs_state(self, state):
        self._state.vs.shader = state

    def bind_tcs_state(self, state):
        self._state.tcs.shader = state

    def bind_tes_state(self, state):
        self._state.tes.shader = state

    def bind_gs_state(self, state):
        self._state.gs.shader = state

    def bind_fs_state(self, state):
        self._state.fs.shader = state

    def bind_tcz_state(self, state):
        self._state.tcs.shader = state

    def _delete_shader_state(self, state):
        return state

    delete_vs_state = _delete_shader_state
    delete_tcs_state = _delete_shader_state
    delete_tes_state = _delete_shader_state
    delete_gs_state = _delete_shader_state
    delete_fs_state = _delete_shader_state

    def set_blend_color(self, state):
        self._state.blend_color = state

    def set_stencil_ref(self, state):
        self._state.stencil_ref = state

    def set_clip_state(self, state):
        self._state.clip = state

    def _dump_constant_buffer(self, buffer):
        if not self.interpreter.verbosity(2):
            return

        data = self.real.buffer_read(buffer)
        format = '4f'
        index = 0
        for offset in range(0, len(data), struct.calcsize(format)):
            x, y, z, w = unpack_from(format, data, offset)
            sys.stdout.write('\tCONST[%2u] = {%10.4f, %10.4f, %10.4f, %10.4f}\n' % (index, x, y, z, w))
            index += 1
        sys.stdout.flush()

    def _get_stage_state(self, shader):
        if shader == 'PIPE_SHADER_VERTEX':
            return self._state.vs
        if shader == 'PIPE_SHADER_TESS_CTRL':
            return self._state.tcs
        if shader == 'PIPE_SHADER_TESS_EVAL':
            return self._state.tes
        if shader == 'PIPE_SHADER_GEOMETRY':
            return self._state.gs
        if shader == 'PIPE_SHADER_FRAGMENT':
            return self._state.fs
        assert False

    def set_constant_buffer(self, shader, index, take_ownership, constant_buffer):
        self._update(self._get_stage_state(shader).constant_buffer, index, 1, [constant_buffer])

    def set_framebuffer_state(self, state):
        self._state.fb = state

    def set_polygon_stipple(self, state):
        self._state.polygon_stipple = state

    def _update(self, array, start_slot, num_slots, states):
        if not isinstance(states, list):
            # XXX: trace is not serializing multiple scissors/viewports properly yet
            num_slots = 1
            states = [states]
        while len(array) < start_slot + num_slots:
            array.append(None)
        for i in range(num_slots):
            array[start_slot + i] = states[i]

    def set_scissor_states(self, start_slot, num_scissors, states):
        self._update(self._state.scissors, start_slot, num_scissors, states)

    def set_viewport_states(self, start_slot, num_viewports, states):
        self._update(self._state.viewports, start_slot, num_viewports, states)

    def create_sampler_view(self, resource, templ):
        templ.resource = resource
        return templ

    def sampler_view_destroy(self, view):
        pass

    def set_sampler_views(self, shader, start, num, unbind_num_trailing_slots, take_ownership, views):
        # FIXME: Handle non-zero start
        assert start == 0
        self._get_stage_state(shader).sampler_views = views

    def set_fragment_sampler_views(self, num, views):
        # XXX: deprecated
        self._state.fs.sampler_views = views

    def set_geometry_sampler_views(self, num, views):
        # XXX: deprecated
        self._state.gs.sampler_views = views

    def set_vertex_sampler_views(self, num, views):
        # XXX: deprecated
        self._state.vs.sampler_views = views

    def set_vertex_buffers(self, num_buffers, unbind_num_trailing_slots, take_ownership, buffers):
        self._update(self._state.vertex_buffers, 0, num_buffers, buffers)

    def create_vertex_elements_state(self, num_elements, elements):
        return elements[0:num_elements]

    def bind_vertex_elements_state(self, state):
        self._state.vertex_elements = state

    def delete_vertex_elements_state(self, state):
        pass

    def set_index_buffer(self, ib):
        self._state.index_buffer = ib

    def set_patch_vertices(self, patch_vertices):
        pass

    # Don't dump more than this number of indices/vertices
    MAX_ELEMENTS = 16

    def _merge_indices(self, info):
        '''Merge the vertices into our state.'''

        index_size = self._state.index_buffer.index_size

        format = {
            1: 'B',
            2: 'H',
            4: 'I',
        }[index_size]

        assert struct.calcsize(format) == index_size

        if self._state.index_buffer.buffer is None:
            # Could happen with index in user memory
            return 0, 0

        data = self._state.index_buffer.buffer.data
        max_index, min_index = 0, 0xffffffff

        count = min(info.count, self.MAX_ELEMENTS)
        indices = []
        for i in range(info.start, info.start + count):
            offset = self._state.index_buffer.offset + i*index_size
            if offset + index_size > len(data):
                index = 0
            else:
                index, = unpack_from(format, data, offset)
            indices.append(index)
            min_index = min(min_index, index)
            max_index = max(max_index, index)

        self._state.indices = indices

        return min_index + info.index_bias, max_index + info.index_bias

    def _merge_vertices(self, start, count):
        '''Merge the vertices into our state.'''

        count = min(count, self.MAX_ELEMENTS)
        vertices = []
        for index in range(start, start + count):
            if index >= start + 16:
                sys.stdout.write('\t...\n')
                break
            vertex = []
            for velem in self._state.vertex_elements:
                vbuf = self._state.vertex_buffers[velem.vertex_buffer_index]
                resource = vbuf.buffer_resource
                if resource is None:
                    continue

                data = resource.data

                offset = vbuf.buffer_offset + velem.src_offset + vbuf.stride*index
                format = {
                    'PIPE_FORMAT_R32_FLOAT': 'f',
                    'PIPE_FORMAT_R32G32_FLOAT': '2f',
                    'PIPE_FORMAT_R32G32B32_FLOAT': '3f',
                    'PIPE_FORMAT_R32G32B32A32_FLOAT': '4f',
                    'PIPE_FORMAT_R32_UINT': 'I',
                    'PIPE_FORMAT_R32G32_UINT': '2I',
                    'PIPE_FORMAT_R32G32B32_UINT': '3I',
                    'PIPE_FORMAT_R32G32B32A32_UINT': '4I',
                    'PIPE_FORMAT_R8_UINT': 'B',
                    'PIPE_FORMAT_R8G8_UINT': '2B',
                    'PIPE_FORMAT_R8G8B8_UINT': '3B',
                    'PIPE_FORMAT_R8G8B8A8_UINT': '4B',
                    'PIPE_FORMAT_A8R8G8B8_UNORM': '4B',
                    'PIPE_FORMAT_R8G8B8A8_UNORM': '4B',
                    'PIPE_FORMAT_B8G8R8A8_UNORM': '4B',
                    'PIPE_FORMAT_R16G16B16_SNORM': '3h',
                }[velem.src_format]

                data = resource.data
                attribute = unpack_from(format, data, offset)
                vertex.append(attribute)

            vertices.append(vertex)

        self._state.vertices = vertices

    def render_condition(self, query, condition = 0, mode = 0):
        self._state.render_condition_query = query
        self._state.render_condition_condition = condition
        self._state.render_condition_mode = mode

    def set_stream_output_targets(self, num_targets, tgs, offsets):
        self._state.so_targets = tgs
        self._state.offsets = offsets

    def set_sample_mask(self, sample_mask):
        self._state.sample_mask = sample_mask

    def set_min_samples(self, min_samples):
        pass

    def draw_vbo(self, info, drawid_offset, indirect, draws, num_draws):
        self._draw_no += 1

        if self.interpreter.call_no < self.interpreter.options.call and \
            self._draw_no < self.interpreter.options.draw:
                return

        # Merge the all draw state

        self._state.draw = info

        if info.index_size != 0:
            min_index, max_index = self._merge_indices(info)
        else:
            min_index = draws[0].start
            max_index = draws[0].start + draws[0].count - 1
        self._merge_vertices(min_index, max_index - min_index + 1)

        self._dump_state()

    _dclRE = re.compile('^DCL\s+(IN|OUT|SAMP|SVIEW)\[([0-9]+)\].*$', re.MULTILINE)

    def _normalize_stage_state(self, stage):
        if stage.shader is not None and stage.shader.tokens is not None:
            registers = {}

            for mo in self._dclRE.finditer(stage.shader.tokens):
                file_ = mo.group(1)
                index = mo.group(2)
                register = registers.setdefault(file_, set())
                register.add(int(index))

            if 'SAMP' in registers and 'SVIEW' not in registers:
                registers['SVIEW'] = registers['SAMP']

            mapping = [
                #("CONST", "constant_buffer"),
                ("SAMP", "sampler"),
                ("SVIEW", "sampler_views"),
            ]

            for fileName, attrName in mapping:
                register = registers.setdefault(fileName, set())
                attr = getattr(stage, attrName)
                for index in range(len(attr)):
                    if index not in register:
                        attr[index] = None
                while attr and attr[-1] is None:
                    attr.pop()

    def _dump_state(self):
        '''Dump our state to JSON and terminate.'''

        state = copy.deepcopy(self._state)

        self._normalize_stage_state(state.vs)
        self._normalize_stage_state(state.tcs)
        self._normalize_stage_state(state.tes)
        self._normalize_stage_state(state.gs)
        self._normalize_stage_state(state.fs)

        json.dump(
            obj = state,
            fp = sys.stdout,
            default = serialize,
            sort_keys = True,
            indent = 4,
            separators = (',', ': ')
        )

        sys.exit(0)

    def resource_copy_region(self, dst, dst_level, dstx, dsty, dstz, src, src_level, src_box):
        if dst.target == PIPE_BUFFER or src.target == PIPE_BUFFER:
            assert dst.target == PIPE_BUFFER and src.target == PIPE_BUFFER
            assert dst_level == 0
            assert dsty == 0
            assert dstz == 0
            assert src_level == 0
            assert src_box.y == 0
            assert src_box.z == 0
            assert src_box.height == 1
            assert src_box.depth == 1
            dst.data[dstx : dstx + src_box.width] = src.data[src_box.x : src_box.x + src_box.width]
        pass

    def is_resource_referenced(self, texture, face, level):
        pass

    def get_transfer(self, texture, sr, usage, box):
        if texture is None:
            return None
        transfer = Transfer(texture, sr, usage, box)
        return transfer

    def tex_transfer_destroy(self, transfer):
        self.interpreter.unregister_object(transfer)

    def buffer_subdata(self, resource, usage, data, box=None, offset=None, size=None, level=None, stride=None, layer_stride=None):
        if box is not None:
            # XXX trace_context_transfer_unmap generates brokens buffer_subdata
            assert offset is None
            assert size is None
            assert level == 0
            offset = box.x
            size = box.width
            box = None

        if resource is not None and resource.target == PIPE_BUFFER:
            data = data.getValue()
            assert len(data) >= size
            assert offset + size <= len(resource.data)
            resource.data[offset : offset + size] = data[:size]

    def texture_subdata(self, resource, level, usage, box, data, stride, layer_stride):
        pass

    def transfer_inline_write(self, resource, level, usage, box, stride, layer_stride, data):
        if resource is not None and resource.target == PIPE_BUFFER:
            data = data.getValue()
            assert len(data) >= box.width
            assert box.x + box.width <= len(resource.data)
            resource.data[box.x : box.x + box.width] = data[:box.width]

    def flush(self, flags):
        # Return a fake fence
        return self.interpreter.call_no

    def clear(self, buffers, color, depth, stencil, scissor_state=None):
        pass

    def clear_render_target(self, dst, rgba, dstx, dsty, width, height, render_condition_enabled):
        pass

    def clear_depth_stencil(self, dst, clear_flags, depth, stencil, dstx, dsty, width, height, render_condition_enabled):
        pass

    def clear_texture(self, res, level, box, **color):
        pass

    def create_surface(self, resource, surf_tmpl):
        assert resource is not None
        surf_tmpl.resource = resource
        return surf_tmpl

    def surface_destroy(self, surface):
        self.interpreter.unregister_object(surface)

    def create_query(self, query_type, index):
        return query_type

    def destroy_query(self, query):
        pass

    def begin_query(self, query):
        pass

    def end_query(self, query):
        pass

    def create_stream_output_target(self, res, buffer_offset, buffer_size):
        so_target = Struct()
        so_target.resource = res
        so_target.offset = buffer_offset
        so_target.size = buffer_size
        return so_target


class Interpreter(parser.SimpleTraceDumper):
    '''Specialization of a trace parser that interprets the calls as it goes
    along.'''

    ignoredCalls = set((
            ('pipe_screen', 'is_format_supported'),
            ('pipe_screen', 'get_name'),
            ('pipe_screen', 'get_vendor'),
            ('pipe_screen', 'get_device_uuid'),
            ('pipe_screen', 'get_driver_uuid'),
            ('pipe_screen', 'get_compiler_options'),
            ('pipe_screen', 'get_param'),
            ('pipe_screen', 'get_paramf'),
            ('pipe_screen', 'get_shader_param'),
            ('pipe_screen', 'get_compute_param'),
            ('pipe_screen', 'get_disk_shader_cache'),
            ('pipe_context', 'clear_render_target'), # XXX workaround trace bugs
            ('pipe_context', 'flush_resource'),
            ('pipe_context', 'buffer_map'),
            ('pipe_context', 'texture_map'),
            ('pipe_context', 'transfer_unmap'),
    ))

    def __init__(self, stream, options, formatter, state):
        parser.SimpleTraceDumper.__init__(self, stream, options, formatter, state)
        self.objects = {}
        self.result = None
        self.globl = Global(self)
        self.call_no = None

    def register_object(self, address, object):
        self.objects[address] = object

    def unregister_object(self, object):
        # TODO
        pass

    def lookup_object(self, address):
        try:
            return self.objects[address]
        except KeyError:
            # Could happen, e.g., with user memory pointers
            return address

    def interpret(self, trace):
        for call in trace.calls:
            self.interpret_call(call)

    def handle_call(self, call):
        if (call.klass, call.method) in self.ignoredCalls:
            return

        self.call_no = call.no

        if self.verbosity(1):
            # Write the call to stderr (as stdout would corrupt the JSON output)
            sys.stderr.flush()
            sys.stdout.flush()
            parser.TraceDumper.handle_call(self, call)
            sys.stderr.flush()
            sys.stdout.flush()

        args = [(str(name), self.interpret_arg(arg)) for name, arg in call.args]

        if call.klass:
            name, obj = args[0]
            args = args[1:]
        else:
            obj = self.globl

        method = getattr(obj, call.method)
        ret = method(**dict(args))

        # Keep track of created pointer objects.
        if call.ret and isinstance(call.ret, model.Pointer):
            if ret is None:
                sys.stderr.write('warning: NULL returned\n')
            self.register_object(call.ret.address, ret)

        self.call_no = None

    def interpret_arg(self, node):
        translator = Translator(self)
        return translator.visit(node)

    def verbosity(self, level):
        return self.options.verbosity >= level


class DumpStateOptions(parser.ParseOptions):

    def __init__(self, args=None):

        # These will get initialized in ModelOptions.__init__()
        self.verbosity = None
        self.call = None
        self.draw = None

        parser.ParseOptions.__init__(self, args)


class Main(parser.Main):

    def get_optparser(self):
        optparser = argparse.ArgumentParser(
            description="Parse and dump Gallium trace(s) as JSON")

        optparser.add_argument("filename", action="extend", nargs="+",
            type=str, metavar="filename", help="Gallium trace filename (plain or .gz, .bz2)")

        optparser.add_argument("-v", "--verbose", action="count", default=0, dest="verbosity", help="increase verbosity level")
        optparser.add_argument("-q", "--quiet", action="store_const", const=0, dest="verbosity", help="no messages")
        optparser.add_argument("-c", "--call", action="store", type=int, dest="call", default=0xffffffff, help="dump on this call")
        optparser.add_argument("-d", "--draw", action="store", type=int, dest="draw", default=0xffffffff, help="dump on this draw")
        return optparser

    def make_options(self, args):
        return DumpStateOptions(args)

    def process_arg(self, stream, options):
        formatter = format.Formatter(sys.stderr)
        parser = Interpreter(stream, options, formatter, model.TraceStateData())
        parser.parse()


if __name__ == '__main__':
    Main().main()
