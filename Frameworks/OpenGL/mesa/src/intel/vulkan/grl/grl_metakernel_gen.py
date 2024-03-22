#!/bin/env python
COPYRIGHT = """\
/*
 * Copyright 2021 Intel Corporation
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sub license, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice (including the
 * next paragraph) shall be included in all copies or substantial portions
 * of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT.
 * IN NO EVENT SHALL VMWARE AND/OR ITS SUPPLIERS BE LIABLE FOR
 * ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */
"""

import argparse
import os.path
import re
import sys

from grl_parser import parse_grl_file

class Writer(object):
    def __init__(self, file):
        self._file = file
        self._indent = 0
        self._new_line = True

    def push_indent(self, levels=4):
        self._indent += levels

    def pop_indent(self, levels=4):
        self._indent -= levels

    def write(self, s, *fmt):
        if self._new_line:
            s = '\n' + s
        self._new_line = False
        if s.endswith('\n'):
            self._new_line = True
            s = s[:-1]
        if fmt:
            s = s.format(*fmt)
        self._file.write(s.replace('\n', '\n' + ' ' * self._indent))

# Internal Representation

class Value(object):
    def __init__(self, name=None, zone=None):
        self.name = name
        self._zone = zone
        self.live = False

    @property
    def zone(self):
        assert self._zone is not None
        return self._zone

    def is_reg(self):
        return False

    def c_val(self):
        if not self.name:
            print(self)
        assert self.name
        return self.name

    def c_cpu_val(self):
        assert self.zone == 'cpu'
        return self.c_val()

    def c_gpu_val(self):
        if self.zone == 'gpu':
            return self.c_val()
        else:
            return 'mi_imm({})'.format(self.c_cpu_val())

class Constant(Value):
    def __init__(self, value):
        super().__init__(zone='cpu')
        self.value = value

    def c_val(self):
        if self.value < 100:
            return str(self.value)
        elif self.value < (1 << 32):
            return '0x{:x}u'.format(self.value)
        else:
            return '0x{:x}ull'.format(self.value)

class Register(Value):
    def __init__(self, name):
        super().__init__(name=name, zone='gpu')

    def is_reg(self):
        return True

class FixedGPR(Register):
    def __init__(self, num):
        super().__init__('REG{}'.format(num))
        self.num = num

    def write_c(self, w):
        w.write('UNUSED struct mi_value {} = mi_reserve_gpr(&b, {});\n',
                self.name, self.num)

class GroupSizeRegister(Register):
    def __init__(self, comp):
        super().__init__('DISPATCHDIM_' + 'XYZ'[comp])
        self.comp = comp

class Member(Value):
    def __init__(self, value, member):
        super().__init__(zone=value.zone)
        self.value = value
        self.member = member

    def is_reg(self):
        return self.value.is_reg()

    def c_val(self):
        c_val = self.value.c_val()
        if self.zone == 'gpu':
            assert isinstance(self.value, Register)
            if self.member == 'hi':
                return 'mi_value_half({}, true)'.format(c_val)
            elif self.member == 'lo':
                return 'mi_value_half({}, false)'.format(c_val)
            else:
                assert False, 'Invalid member: {}'.format(self.member)
        else:
            return '.'.join([c_val, self.member])

class OffsetOf(Value):
    def __init__(self, mk, expr):
        super().__init__(zone='cpu')
        assert isinstance(expr, tuple) and expr[0] == 'member'
        self.type = mk.m.get_type(expr[1])
        self.field = expr[2]

    def c_val(self):
        return 'offsetof({}, {})'.format(self.type.c_name, self.field)

class Scope(object):
    def __init__(self, m, mk, parent):
        self.m = m
        self.mk = mk
        self.parent = parent
        self.defs = {}

    def add_def(self, d, name=None):
        if name is None:
            name = d.name
        assert name not in self.defs
        self.defs[name] = d

    def get_def(self, name):
        if name in self.defs:
            return self.defs[name]
        assert self.parent, 'Unknown definition: "{}"'.format(name)
        return self.parent.get_def(name)

class Statement(object):
    def __init__(self, srcs=[]):
        assert isinstance(srcs, (list, tuple))
        self.srcs = list(srcs)

class SSAStatement(Statement, Value):
    _count = 0

    def __init__(self, zone, srcs):
        Statement.__init__(self, srcs)
        Value.__init__(self, None, zone)
        self.c_name = '_tmp{}'.format(SSAStatement._count)
        SSAStatement._count += 1

    def c_val(self):
        return self.c_name

    def write_c_refs(self, w):
        assert self.zone == 'gpu'
        assert self.uses > 0
        if self.uses > 1:
            w.write('mi_value_add_refs(&b, {}, {});\n',
                    self.c_name, self.uses - 1)

class Half(SSAStatement):
    def __init__(self, value, half):
        assert half in ('hi', 'lo')
        super().__init__(None, [value])
        self.half = half

    @property
    def zone(self):
        return self.srcs[0].zone

    def write_c(self, w):
        assert self.half in ('hi', 'lo')
        if self.zone == 'cpu':
            if self.half == 'hi':
                w.write('uint32_t {} = (uint64_t)({}) >> 32;\n',
                        self.c_name, self.srcs[0].c_cpu_val())
            else:
                w.write('uint32_t {} = {};\n',
                        self.c_name, self.srcs[0].c_cpu_val())
        else:
            if self.half == 'hi':
                w.write('struct mi_value {} = mi_value_half({}, true);\n',
                        self.c_name, self.srcs[0].c_gpu_val())
            else:
                w.write('struct mi_value {} = mi_value_half({}, false);\n',
                        self.c_name, self.srcs[0].c_gpu_val())
            self.write_c_refs(w)

class Expression(SSAStatement):
    def __init__(self, mk, op, *srcs):
        super().__init__(None, srcs)
        self.op = op

    @property
    def zone(self):
        zone = 'cpu'
        for s in self.srcs:
            if s.zone == 'gpu':
                zone = 'gpu'
        return zone

    def write_c(self, w):
        if self.zone == 'cpu':
            w.write('uint64_t {} = ', self.c_name)
            c_cpu_vals = [s.c_cpu_val() for s in self.srcs]
            if len(self.srcs) == 1:
                w.write('({} {})', self.op, c_cpu_vals[0])
            elif len(self.srcs) == 2:
                w.write('({} {} {})', c_cpu_vals[0], self.op, c_cpu_vals[1])
            else:
                assert len(self.srcs) == 3 and op == '?'
                w.write('({} ? {} : {})', *c_cpu_vals)
            w.write(';\n')
            return

        w.write('struct mi_value {} = ', self.c_name)
        if self.op == '~':
            w.write('mi_inot(&b, {});\n', self.srcs[0].c_gpu_val())
        elif self.op == '+':
            w.write('mi_iadd(&b, {}, {});\n',
                    self.srcs[0].c_gpu_val(), self.srcs[1].c_gpu_val())
        elif self.op == '-':
            w.write('mi_isub(&b, {}, {});\n',
                    self.srcs[0].c_gpu_val(), self.srcs[1].c_gpu_val())
        elif self.op == '&':
            w.write('mi_iand(&b, {}, {});\n',
                    self.srcs[0].c_gpu_val(), self.srcs[1].c_gpu_val())
        elif self.op == '|':
            w.write('mi_ior(&b, {}, {});\n',
                    self.srcs[0].c_gpu_val(), self.srcs[1].c_gpu_val())
        elif self.op == '<<':
            if self.srcs[1].zone == 'cpu':
                w.write('mi_ishl_imm(&b, {}, {});\n',
                        self.srcs[0].c_gpu_val(), self.srcs[1].c_cpu_val())
            else:
                w.write('mi_ishl(&b, {}, {});\n',
                        self.srcs[0].c_gpu_val(), self.srcs[1].c_gpu_val())
        elif self.op == '>>':
            if self.srcs[1].zone == 'cpu':
                w.write('mi_ushr_imm(&b, {}, {});\n',
                        self.srcs[0].c_gpu_val(), self.srcs[1].c_cpu_val())
            else:
                w.write('mi_ushr(&b, {}, {});\n',
                        self.srcs[0].c_gpu_val(), self.srcs[1].c_gpu_val())
        elif self.op == '==':
            w.write('mi_ieq(&b, {}, {});\n',
                    self.srcs[0].c_gpu_val(), self.srcs[1].c_gpu_val())
        elif self.op == '<':
            w.write('mi_ult(&b, {}, {});\n',
                    self.srcs[0].c_gpu_val(), self.srcs[1].c_gpu_val())
        elif self.op == '>':
            w.write('mi_ult(&b, {}, {});\n',
                    self.srcs[1].c_gpu_val(), self.srcs[0].c_gpu_val())
        elif self.op == '<=':
            w.write('mi_uge(&b, {}, {});\n',
                    self.srcs[1].c_gpu_val(), self.srcs[0].c_gpu_val())
        else:
            assert False, 'Unknown expression opcode: {}'.format(self.op)
        self.write_c_refs(w)

class StoreReg(Statement):
    def __init__(self, mk, reg, value):
        super().__init__([mk.load_value(value)])
        self.reg = mk.parse_value(reg)
        assert self.reg.is_reg()

    def write_c(self, w):
        value = self.srcs[0]
        w.write('mi_store(&b, {}, {});\n',
                self.reg.c_gpu_val(), value.c_gpu_val())

class LoadMem(SSAStatement):
    def __init__(self, mk, bit_size, addr):
        super().__init__('gpu', [mk.load_value(addr)])
        self.bit_size = bit_size

    def write_c(self, w):
        addr = self.srcs[0]
        w.write('struct mi_value {} = ', self.c_name)
        if addr.zone == 'cpu':
            w.write('mi_mem{}(anv_address_from_u64({}));\n',
                    self.bit_size, addr.c_cpu_val())
        else:
            assert self.bit_size == 64
            w.write('mi_load_mem64_offset(&b, anv_address_from_u64(0), {});\n',
                    addr.c_gpu_val())
        self.write_c_refs(w)

class StoreMem(Statement):
    def __init__(self, mk, bit_size, addr, src):
        super().__init__([mk.load_value(addr), mk.load_value(src)])
        self.bit_size = bit_size

    def write_c(self, w):
        addr, data = tuple(self.srcs)
        if addr.zone == 'cpu':
            w.write('mi_store(&b, mi_mem{}(anv_address_from_u64({})), {});\n',
                    self.bit_size, addr.c_cpu_val(), data.c_gpu_val())
        else:
            assert self.bit_size == 64
            w.write('mi_store_mem64_offset(&b, anv_address_from_u64(0), {}, {});\n',
                    addr.c_gpu_val(), data.c_gpu_val())

class GoTo(Statement):
    def __init__(self, mk, target_id, cond=None, invert=False):
        cond = [mk.load_value(cond)] if cond is not None else []
        super().__init__(cond)
        self.target_id = target_id
        self.invert = invert
        self.mk = mk

    def write_c(self, w):
        # Now that we've parsed the entire metakernel, we can look up the
        # actual target from the id
        target = self.mk.get_goto_target(self.target_id)

        if self.srcs:
            cond = self.srcs[0]
            if self.invert:
                w.write('mi_goto_if(&b, mi_inot(&b, {}), &{});\n', cond.c_gpu_val(), target.c_name)
            else:
                w.write('mi_goto_if(&b, {}, &{});\n', cond.c_gpu_val(), target.c_name)
        else:
            w.write('mi_goto(&b, &{});\n', target.c_name)

class GoToTarget(Statement):
    def __init__(self, mk, name):
        super().__init__()
        self.name = name
        self.c_name = '_goto_target_' + name
        self.goto_tokens = []

        mk = mk.add_goto_target(self)

    def write_decl(self, w):
        w.write('struct mi_goto_target {} = MI_GOTO_TARGET_INIT;\n',
                self.c_name)

    def write_c(self, w):
        w.write('mi_goto_target(&b, &{});\n', self.c_name)

class Dispatch(Statement):
    def __init__(self, mk, kernel, group_size, args, postsync):
        if group_size is None:
            srcs = [mk.scope.get_def('DISPATCHDIM_{}'.format(d)) for d in 'XYZ']
        else:
            srcs = [mk.load_value(s) for s in group_size]
        srcs += [mk.load_value(a) for a in args]
        super().__init__(srcs)
        self.kernel = mk.m.kernels[kernel]
        self.indirect = group_size is None
        self.postsync = postsync

    def write_c(self, w):
        w.write('{\n')
        w.push_indent()

        group_size = self.srcs[:3]
        args = self.srcs[3:]
        if not self.indirect:
            w.write('const uint32_t _group_size[3] = {{ {}, {}, {} }};\n',
                    *[s.c_cpu_val() for s in group_size])
            gs = '_group_size'
        else:
            gs = 'NULL'

        w.write('const struct anv_kernel_arg _args[] = {\n')
        w.push_indent()
        for arg in args:
            w.write('{{ .u64 = {} }},\n', arg.c_cpu_val())
        w.pop_indent()
        w.write('};\n')

        w.write('genX(grl_dispatch)(cmd_buffer, {},\n', self.kernel.c_name)
        w.write('                   {}, ARRAY_SIZE(_args), _args);\n', gs)
        w.pop_indent()
        w.write('}\n')

class SemWait(Statement):
    def __init__(self, scope, wait):
        super().__init__()
        self.wait = wait

class Control(Statement):
    def __init__(self, scope, wait):
        super().__init__()
        self.wait = wait

    def write_c(self, w):
        w.write('cmd_buffer->state.pending_pipe_bits |=\n')
        w.write('    ANV_PIPE_CS_STALL_BIT |\n')
        w.write('    ANV_PIPE_DATA_CACHE_FLUSH_BIT |\n')
        w.write('    ANV_PIPE_UNTYPED_DATAPORT_CACHE_FLUSH_BIT;\n')
        w.write('genX(cmd_buffer_apply_pipe_flushes)(cmd_buffer);\n')

TYPE_REMAPS = {
    'dword' : 'uint32_t',
    'qword' : 'uint64_t',
}

class Module(object):
    def __init__(self, grl_dir, elems):
        assert isinstance(elems[0], tuple)
        assert elems[0][0] == 'module-name'
        self.grl_dir = grl_dir
        self.name = elems[0][1]
        self.kernels = {}
        self.structs = {}
        self.constants = []
        self.metakernels = []
        self.regs = {}

        scope = Scope(self, None, None)
        for e in elems[1:]:
            if e[0] == 'kernel':
                k = Kernel(self, *e[1:])
                assert k.name not in self.kernels
                self.kernels[k.name] = k
            elif e[0] == 'kernel-module':
                m = KernelModule(self, *e[1:])
                for k in m.kernels:
                    assert k.name not in self.kernels
                    self.kernels[k.name] = k
            elif e[0] == 'struct':
                s = Struct(self, *e[1:])
                assert s.name not in self.kernels
                self.structs[s.name] = s
            elif e[0] == 'named-constant':
                c = NamedConstant(*e[1:])
                scope.add_def(c)
                self.constants.append(c)
            elif e[0] == 'meta-kernel':
                mk = MetaKernel(self, scope, *e[1:])
                self.metakernels.append(mk)
            elif e[0] == 'import':
                assert e[2] == 'struct'
                self.import_struct(e[1], e[3])
            else:
                assert False, 'Invalid module-level token: {}'.format(t[0])

    def import_struct(self, filename, struct_name):
        elems = parse_grl_file(os.path.join(self.grl_dir, filename), [])
        assert elems
        for e in elems[1:]:
            if e[0] == 'struct' and e[1] == struct_name:
                s = Struct(self, *e[1:])
                assert s.name not in self.kernels
                self.structs[s.name] = s
                return
        assert False, "Struct {0} not found in {1}".format(struct_name, filename)

    def get_type(self, name):
        if name in self.structs:
            return self.structs[name]
        return BasicType(TYPE_REMAPS.get(name, name))

    def get_fixed_gpr(self, num):
        assert isinstance(num, int)
        if num in self.regs:
            return self.regs[num]

        reg = FixedGPR(num)
        self.regs[num] = reg
        return reg

    def optimize(self):
        progress = True
        while progress:
            progress = False

            # Copy Propagation
            for mk in self.metakernels:
                if mk.opt_copy_prop():
                    progress = True

            # Dead Code Elimination
            for r in self.regs.values():
                r.live = False
            for c in self.constants:
                c.live = False
            for mk in self.metakernels:
                mk.opt_dead_code1()
            for mk in self.metakernels:
                if mk.opt_dead_code2():
                    progress = True
            for n in list(self.regs.keys()):
                if not self.regs[n].live:
                    del self.regs[n]
                    progress = True
            self.constants = [c for c in self.constants if c.live]

    def compact_regs(self):
        old_regs = self.regs
        self.regs = {}
        for i, reg in enumerate(old_regs.values()):
            reg.num = i
            self.regs[i] = reg

    def write_h(self, w):
        for s in self.structs.values():
            s.write_h(w)
        for mk in self.metakernels:
            mk.write_h(w)

    def write_c(self, w):
        for c in self.constants:
            c.write_c(w)
        for mk in self.metakernels:
            mk.write_c(w)

class Kernel(object):
    def __init__(self, m, name, ann):
        self.name = name
        self.source_file = ann['source']
        self.kernel_name = self.source_file.replace('/', '_')[:-3].upper()
        self.entrypoint = ann['kernelFunction']

        assert self.source_file.endswith('.cl')
        self.c_name = '_'.join([
            'GRL_CL_KERNEL',
            self.kernel_name,
            self.entrypoint.upper(),
        ])

class KernelModule(object):
    def __init__(self, m, name, source, kernels):
        self.name = name
        self.kernels = []
        self.libraries = []

        for k in kernels:
            if k[0] == 'kernel':
                k[2]['source'] = source
                self.kernels.append(Kernel(m, *k[1:]))
            elif k[0] == 'library':
                # Skip this for now.
                pass

class BasicType(object):
    def __init__(self, name):
        self.name = name
        self.c_name = name

class Struct(object):
    def __init__(self, m, name, fields, align):
        assert align == 0
        self.name = name
        self.c_name = 'struct ' + '_'.join(['grl', m.name, self.name])
        self.fields = [(m.get_type(t), n) for t, n in fields]

    def write_h(self, w):
        w.write('{} {{\n', self.c_name)
        w.push_indent()
        for f in self.fields:
            w.write('{} {};\n', f[0].c_name, f[1])
        w.pop_indent()
        w.write('};\n')

class NamedConstant(Value):
    def __init__(self, name, value):
        super().__init__(name, 'cpu')
        self.name = name
        self.value = Constant(value)
        self.written = False

    def set_module(self, m):
        pass

    def write_c(self, w):
        if self.written:
            return
        w.write('static const uint64_t {} = {};\n',
                self.name, self.value.c_val())
        self.written = True

class MetaKernelParameter(Value):
    def __init__(self, mk, type, name):
        super().__init__(name, 'cpu')
        self.type = mk.m.get_type(type)

class MetaKernel(object):
    def __init__(self, m, m_scope, name, params, ann, statements):
        self.m = m
        self.name = name
        self.c_name = '_'.join(['grl', m.name, self.name])
        self.goto_targets = {}
        self.num_tmps = 0

        mk_scope = Scope(m, self, m_scope)

        self.params = [MetaKernelParameter(self, *p) for p in params]
        for p in self.params:
            mk_scope.add_def(p)

        mk_scope.add_def(GroupSizeRegister(0), name='DISPATCHDIM_X')
        mk_scope.add_def(GroupSizeRegister(1), name='DISPATCHDIM_Y')
        mk_scope.add_def(GroupSizeRegister(2), name='DISPATCHDIM_Z')

        self.statements = []
        self.parse_stmt(mk_scope, statements)
        self.scope = None

    def get_tmp(self):
        tmpN = '_tmp{}'.format(self.num_tmps)
        self.num_tmps += 1
        return tmpN

    def add_stmt(self, stmt):
        self.statements.append(stmt)
        return stmt

    def parse_value(self, v):
        if isinstance(v, Value):
            return v
        elif isinstance(v, str):
            if re.match(r'REG\d+', v):
                return self.m.get_fixed_gpr(int(v[3:]))
            else:
                return self.scope.get_def(v)
        elif isinstance(v, int):
            return Constant(v)
        elif isinstance(v, tuple):
            if v[0] == 'member':
                return Member(self.parse_value(v[1]), v[2])
            elif v[0] == 'offsetof':
                return OffsetOf(self, v[1])
            else:
                op = v[0]
                srcs = [self.parse_value(s) for s in v[1:]]
                return self.add_stmt(Expression(self, op, *srcs))
        else:
            assert False, 'Invalid value: {}'.format(v[0])

    def load_value(self, v):
        v = self.parse_value(v)
        if isinstance(v, Member) and v.zone == 'gpu':
            v = self.add_stmt(Half(v.value, v.member))
        return v

    def parse_stmt(self, scope, s):
        self.scope = scope
        if isinstance(s, list):
            subscope = Scope(self.m, self, scope)
            for stmt in s:
                self.parse_stmt(subscope, stmt)
        elif s[0] == 'define':
            scope.add_def(self.parse_value(s[2]), name=s[1])
        elif s[0] == 'assign':
            self.add_stmt(StoreReg(self, *s[1:]))
        elif s[0] == 'dispatch':
            self.add_stmt(Dispatch(self, *s[1:]))
        elif s[0] == 'load-dword':
            v = self.add_stmt(LoadMem(self, 32, s[2]))
            self.add_stmt(StoreReg(self, s[1], v))
        elif s[0] == 'load-qword':
            v = self.add_stmt(LoadMem(self, 64, s[2]))
            self.add_stmt(StoreReg(self, s[1], v))
        elif s[0] == 'store-dword':
            self.add_stmt(StoreMem(self, 32, *s[1:]))
        elif s[0] == 'store-qword':
            self.add_stmt(StoreMem(self, 64, *s[1:]))
        elif s[0] == 'goto':
            self.add_stmt(GoTo(self, s[1]))
        elif s[0] == 'goto-if':
            self.add_stmt(GoTo(self, s[1], s[2]))
        elif s[0] == 'goto-if-not':
            self.add_stmt(GoTo(self, s[1], s[2], invert=True))
        elif s[0] == 'label':
            self.add_stmt(GoToTarget(self, s[1]))
        elif s[0] == 'control':
            self.add_stmt(Control(self, s[1]))
        elif s[0] == 'sem-wait-while':
            self.add_stmt(Control(self, s[1]))
        else:
            assert False, 'Invalid statement: {}'.format(s[0])

    def add_goto_target(self, t):
        assert t.name not in self.goto_targets
        self.goto_targets[t.name] = t

    def get_goto_target(self, name):
        return self.goto_targets[name]

    def opt_copy_prop(self):
        progress = False
        copies = {}
        for stmt in self.statements:
            for i in range(len(stmt.srcs)):
                src = stmt.srcs[i]
                if isinstance(src, FixedGPR) and src.num in copies:
                    stmt.srcs[i] = copies[src.num]
                    progress = True

            if isinstance(stmt, StoreReg):
                reg = stmt.reg
                if isinstance(reg, Member):
                    reg = reg.value

                if isinstance(reg, FixedGPR):
                    copies.pop(reg.num, None)
                    if not stmt.srcs[0].is_reg():
                        copies[reg.num] = stmt.srcs[0]
            elif isinstance(stmt, (GoTo, GoToTarget)):
                copies = {}

        return progress

    def opt_dead_code1(self):
        for stmt in self.statements:
            # Mark every register which is read as live
            for src in stmt.srcs:
                if isinstance(src, Register):
                    src.live = True

            # Initialize every SSA statement to dead
            if isinstance(stmt, SSAStatement):
                stmt.live = False

    def opt_dead_code2(self):
        def yield_live(statements):
            gprs_read = set(self.m.regs.keys())
            for stmt in statements:
                if isinstance(stmt, SSAStatement):
                    if not stmt.live:
                        continue
                elif isinstance(stmt, StoreReg):
                    reg = stmt.reg
                    if isinstance(reg, Member):
                        reg = reg.value

                    if not stmt.reg.live:
                        continue

                    if isinstance(reg, FixedGPR):
                        if reg.num in gprs_read:
                            gprs_read.remove(reg.num)
                        else:
                            continue
                elif isinstance(stmt, (GoTo, GoToTarget)):
                    gprs_read = set(self.m.regs.keys())

                for src in stmt.srcs:
                    src.live = True
                    if isinstance(src, FixedGPR):
                        gprs_read.add(src.num)
                yield stmt

        old_stmt_list = self.statements
        old_stmt_list.reverse()
        self.statements = list(yield_live(old_stmt_list))
        self.statements.reverse()
        return len(self.statements) != len(old_stmt_list)

    def count_ssa_value_uses(self):
        for stmt in self.statements:
            if isinstance(stmt, SSAStatement):
                stmt.uses = 0

            for src in stmt.srcs:
                if isinstance(src, SSAStatement):
                    src.uses += 1

    def write_h(self, w):
        w.write('void\n')
        w.write('genX({})(\n', self.c_name)
        w.push_indent()
        w.write('struct anv_cmd_buffer *cmd_buffer')
        for p in self.params:
            w.write(',\n{} {}', p.type.c_name, p.name)
        w.write(');\n')
        w.pop_indent()

    def write_c(self, w):
        w.write('void\n')
        w.write('genX({})(\n', self.c_name)
        w.push_indent()
        w.write('struct anv_cmd_buffer *cmd_buffer')
        for p in self.params:
            w.write(',\n{} {}', p.type.c_name, p.name)
        w.write(')\n')
        w.pop_indent()
        w.write('{\n')
        w.push_indent()

        w.write('struct mi_builder b;\n')
        w.write('mi_builder_init(&b, cmd_buffer->device->info, &cmd_buffer->batch);\n')
        w.write('/* TODO: use anv_mocs? */\n');
        w.write('const uint32_t mocs = isl_mocs(&cmd_buffer->device->isl_dev, 0, false);\n');
        w.write('mi_builder_set_mocs(&b, mocs);\n');
        w.write('\n')

        for r in self.m.regs.values():
            r.write_c(w)
        w.write('\n')

        for t in self.goto_targets.values():
            t.write_decl(w)
        w.write('\n')

        self.count_ssa_value_uses()
        for s in self.statements:
            s.write_c(w)

        w.pop_indent()

        w.write('}\n')

HEADER_PROLOGUE = COPYRIGHT + '''
#include "anv_private.h"
#include "grl/genX_grl.h"

#ifndef {0}
#define {0}

#ifdef __cplusplus
extern "C" {{
#endif

'''

HEADER_EPILOGUE = '''
#ifdef __cplusplus
}}
#endif

#endif /* {0} */
'''

C_PROLOGUE = COPYRIGHT + '''
#include "{0}"

#include "genxml/gen_macros.h"
#include "genxml/genX_pack.h"
#include "genxml/genX_rt_pack.h"

/* We reserve :
 *    - GPR 14 for secondary command buffer returns
 *    - GPR 15 for conditional rendering
 */
#define MI_BUILDER_NUM_ALLOC_GPRS 14
#define __gen_get_batch_dwords anv_batch_emit_dwords
#define __gen_address_offset anv_address_add
#define __gen_get_batch_address(b, a) anv_batch_address(b, a)
#include "common/mi_builder.h"

#define MI_PREDICATE_RESULT mi_reg32(0x2418)
#define DISPATCHDIM_X mi_reg32(0x2500)
#define DISPATCHDIM_Y mi_reg32(0x2504)
#define DISPATCHDIM_Z mi_reg32(0x2508)
'''

def parse_libraries(filenames):
    libraries = {}
    for fname in filenames:
        lib_package = parse_grl_file(fname, [])
        for lib in lib_package:
            assert lib[0] == 'library'
            # Add the directory of the library so that CL files can be found.
            lib[2].append(('path', os.path.dirname(fname)))
            libraries[lib[1]] = lib
    return libraries

def main():
    argparser = argparse.ArgumentParser()
    argparser.add_argument('--out-c', help='Output C file')
    argparser.add_argument('--out-h', help='Output C file')
    argparser.add_argument('--library', dest='libraries', action='append',
                           default=[], help='Libraries to include')
    argparser.add_argument('grl', help="Input  file")
    args = argparser.parse_args()

    grl_dir = os.path.dirname(args.grl)

    libraries = parse_libraries(args.libraries)

    ir = parse_grl_file(args.grl, libraries)

    m = Module(grl_dir, ir)
    m.optimize()
    m.compact_regs()

    with open(args.out_h, 'w') as f:
        guard = os.path.splitext(os.path.basename(args.out_h))[0].upper()
        w = Writer(f)
        w.write(HEADER_PROLOGUE, guard)
        m.write_h(w)
        w.write(HEADER_EPILOGUE, guard)

    with open(args.out_c, 'w') as f:
        w = Writer(f)
        w.write(C_PROLOGUE, os.path.basename(args.out_h))
        m.write_c(w)

if __name__ == '__main__':
    main()
