# Copyright (c) 2015-2017 Intel Corporation
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

import argparse
import builtins
import collections
import os
import re
import sys
import textwrap

import xml.etree.ElementTree as et

hashed_funcs = {}

c_file = None
_c_indent = 0

def c(*args):
    code = ' '.join(map(str,args))
    for line in code.splitlines():
        text = ''.rjust(_c_indent) + line
        c_file.write(text.rstrip() + "\n")

# indented, but no trailing newline...
def c_line_start(code):
    c_file.write(''.rjust(_c_indent) + code)
def c_raw(code):
    c_file.write(code)

def c_indent(n):
    global _c_indent
    _c_indent = _c_indent + n
def c_outdent(n):
    global _c_indent
    _c_indent = _c_indent - n

header_file = None
_h_indent = 0

def h(*args):
    code = ' '.join(map(str,args))
    for line in code.splitlines():
        text = ''.rjust(_h_indent) + line
        header_file.write(text.rstrip() + "\n")

def h_indent(n):
    global _c_indent
    _h_indent = _h_indent + n
def h_outdent(n):
    global _c_indent
    _h_indent = _h_indent - n


def emit_fadd(tmp_id, args):
    c("double tmp{0} = {1} + {2};".format(tmp_id, args[1], args[0]))
    return tmp_id + 1

# Be careful to check for divide by zero...
def emit_fdiv(tmp_id, args):
    c("double tmp{0} = {1};".format(tmp_id, args[1]))
    c("double tmp{0} = {1};".format(tmp_id + 1, args[0]))
    c("double tmp{0} = tmp{1} ? tmp{2} / tmp{1} : 0;".format(tmp_id + 2, tmp_id + 1, tmp_id))
    return tmp_id + 3

def emit_fmax(tmp_id, args):
    c("double tmp{0} = {1};".format(tmp_id, args[1]))
    c("double tmp{0} = {1};".format(tmp_id + 1, args[0]))
    c("double tmp{0} = MAX(tmp{1}, tmp{2});".format(tmp_id + 2, tmp_id, tmp_id + 1))
    return tmp_id + 3

def emit_fmul(tmp_id, args):
    c("double tmp{0} = {1} * {2};".format(tmp_id, args[1], args[0]))
    return tmp_id + 1

def emit_fsub(tmp_id, args):
    c("double tmp{0} = {1} - {2};".format(tmp_id, args[1], args[0]))
    return tmp_id + 1

def emit_read(tmp_id, args):
    type = args[1].lower()
    c("uint64_t tmp{0} = results->accumulator[query->{1}_offset + {2}];".format(tmp_id, type, args[0]))
    return tmp_id + 1

def emit_uadd(tmp_id, args):
    c("uint64_t tmp{0} = {1} + {2};".format(tmp_id, args[1], args[0]))
    return tmp_id + 1

# Be careful to check for divide by zero...
def emit_udiv(tmp_id, args):
    c("uint64_t tmp{0} = {1};".format(tmp_id, args[1]))
    c("uint64_t tmp{0} = {1};".format(tmp_id + 1, args[0]))
    if args[0].isdigit():
        assert int(args[0]) > 0
        c("uint64_t tmp{0} = tmp{2} / tmp{1};".format(tmp_id + 2, tmp_id + 1, tmp_id))
    else:
        c("uint64_t tmp{0} = tmp{1} ? tmp{2} / tmp{1} : 0;".format(tmp_id + 2, tmp_id + 1, tmp_id))
    return tmp_id + 3

def emit_umul(tmp_id, args):
    c("uint64_t tmp{0} = {1} * {2};".format(tmp_id, args[1], args[0]))
    return tmp_id + 1

def emit_usub(tmp_id, args):
    c("uint64_t tmp{0} = {1} - {2};".format(tmp_id, args[1], args[0]))
    return tmp_id + 1

def emit_umin(tmp_id, args):
    c("uint64_t tmp{0} = MIN({1}, {2});".format(tmp_id, args[1], args[0]))
    return tmp_id + 1

def emit_lshft(tmp_id, args):
    c("uint64_t tmp{0} = {1} << {2};".format(tmp_id, args[1], args[0]))
    return tmp_id + 1

def emit_rshft(tmp_id, args):
    c("uint64_t tmp{0} = {1} >> {2};".format(tmp_id, args[1], args[0]))
    return tmp_id + 1

def emit_and(tmp_id, args):
    c("uint64_t tmp{0} = {1} & {2};".format(tmp_id, args[1], args[0]))
    return tmp_id + 1

def emit_ulte(tmp_id, args):
    c("uint64_t tmp{0} = {1} <= {2};".format(tmp_id, args[1], args[0]))
    return tmp_id + 1

def emit_ult(tmp_id, args):
    c("uint64_t tmp{0} = {1} < {2};".format(tmp_id, args[1], args[0]))
    return tmp_id + 1

def emit_ugte(tmp_id, args):
    c("uint64_t tmp{0} = {1} >= {2};".format(tmp_id, args[1], args[0]))
    return tmp_id + 1

def emit_ugt(tmp_id, args):
    c("uint64_t tmp{0} = {1} > {2};".format(tmp_id, args[1], args[0]))
    return tmp_id + 1

ops = {}
#             (n operands, emitter)
ops["FADD"] = (2, emit_fadd)
ops["FDIV"] = (2, emit_fdiv)
ops["FMAX"] = (2, emit_fmax)
ops["FMUL"] = (2, emit_fmul)
ops["FSUB"] = (2, emit_fsub)
ops["READ"] = (2, emit_read)
ops["UADD"] = (2, emit_uadd)
ops["UDIV"] = (2, emit_udiv)
ops["UMUL"] = (2, emit_umul)
ops["USUB"] = (2, emit_usub)
ops["UMIN"] = (2, emit_umin)
ops["<<"]   = (2, emit_lshft)
ops[">>"]   = (2, emit_rshft)
ops["AND"]  = (2, emit_and)
ops["UGTE"] = (2, emit_ugte)
ops["UGT"]  = (2, emit_ugt)
ops["ULTE"] = (2, emit_ulte)
ops["ULT"]  = (2, emit_ult)


def brkt(subexp):
    if " " in subexp:
        return "(" + subexp + ")"
    else:
        return subexp

def splice_bitwise_and(args):
    return brkt(args[1]) + " & " + brkt(args[0])

def splice_bitwise_or(args):
    return brkt(args[1]) + " | " + brkt(args[0])

def splice_logical_and(args):
    return brkt(args[1]) + " && " + brkt(args[0])

def splice_umul(args):
    return brkt(args[1]) + " * " + brkt(args[0])

def splice_ult(args):
    return brkt(args[1]) + " < " + brkt(args[0])

def splice_ugte(args):
    return brkt(args[1]) + " >= " + brkt(args[0])

def splice_ulte(args):
    return brkt(args[1]) + " <= " + brkt(args[0])

def splice_ugt(args):
    return brkt(args[1]) + " > " + brkt(args[0])

def splice_lshft(args):
    return brkt(args[1]) + " << " + brkt(args[0])

def splice_equal(args):
    return brkt(args[1]) + " == " + brkt(args[0])

exp_ops = {}
#                 (n operands, splicer)
exp_ops["AND"]  = (2, splice_bitwise_and)
exp_ops["OR"]   = (2, splice_bitwise_or)
exp_ops["UGTE"] = (2, splice_ugte)
exp_ops["ULT"]  = (2, splice_ult)
exp_ops["&&"]   = (2, splice_logical_and)
exp_ops["UMUL"] = (2, splice_umul)
exp_ops["<<"]   = (2, splice_lshft)
exp_ops["=="]   = (2, splice_equal)


hw_vars = {}
hw_vars["$EuCoresTotalCount"] = "perf->sys_vars.n_eus"
hw_vars["$VectorEngineTotalCount"] = "perf->sys_vars.n_eus"
hw_vars["$EuSlicesTotalCount"] = "perf->sys_vars.n_eu_slices"
hw_vars["$EuSubslicesTotalCount"] = "perf->sys_vars.n_eu_sub_slices"
hw_vars["$XeCoreTotalCount"] = "perf->sys_vars.n_eu_sub_slices"
hw_vars["$EuDualSubslicesTotalCount"] = "perf->sys_vars.n_eu_sub_slices"
hw_vars["$EuDualSubslicesSlice0123Count"] = "perf->sys_vars.n_eu_slice0123"
hw_vars["$EuThreadsCount"] = "perf->devinfo.num_thread_per_eu"
hw_vars["$VectorEngineThreadsCount"] = "perf->devinfo.num_thread_per_eu"
hw_vars["$SliceMask"] = "perf->sys_vars.slice_mask"
hw_vars["$SliceTotalCount"] = "perf->sys_vars.n_eu_slices"
# subslice_mask is interchangeable with subslice/dual-subslice since Gfx12+
# only has dual subslices which can be assimilated with 16EUs subslices.
hw_vars["$SubsliceMask"] = "perf->sys_vars.subslice_mask"
hw_vars["$DualSubsliceMask"] = "perf->sys_vars.subslice_mask"
hw_vars["$XeCoreMask"] = "perf->sys_vars.subslice_mask"
hw_vars["$GpuTimestampFrequency"] = "perf->devinfo.timestamp_frequency"
hw_vars["$GpuMinFrequency"] = "perf->sys_vars.gt_min_freq"
hw_vars["$GpuMaxFrequency"] = "perf->sys_vars.gt_max_freq"
hw_vars["$SkuRevisionId"] = "perf->devinfo.revision"
hw_vars["$QueryMode"] = "perf->sys_vars.query_mode"

def resolve_variable(name, set, allow_counters):
    if name in hw_vars:
        return hw_vars[name]
    m = re.search(r'\$GtSlice([0-9]+)$', name)
    if m:
        return 'intel_device_info_slice_available(&perf->devinfo, {0})'.format(m.group(1))
    m = re.search(r'\$GtSlice([0-9]+)XeCore([0-9]+)$', name)
    if m:
        return 'intel_device_info_subslice_available(&perf->devinfo, {0}, {1})'.format(m.group(1), m.group(2))
    if allow_counters and name in set.counter_vars:
        return set.read_funcs[name[1:]] + "(perf, query, results)"
    return None

def output_rpn_equation_code(set, counter, equation):
    c("/* RPN equation: " + equation + " */")
    tokens = equation.split()
    stack = []
    tmp_id = 0
    tmp = None

    for token in tokens:
        stack.append(token)
        while stack and stack[-1] in ops:
            op = stack.pop()
            argc, callback = ops[op]
            args = []
            for i in range(0, argc):
                operand = stack.pop()
                if operand[0] == "$":
                    resolved_variable = resolve_variable(operand, set, True)
                    if resolved_variable == None:
                        raise Exception("Failed to resolve variable " + operand + " in equation " + equation + " for " + set.name + " :: " + counter.get('name'));
                    operand = resolved_variable
                args.append(operand)

            tmp_id = callback(tmp_id, args)

            tmp = "tmp{0}".format(tmp_id - 1)
            stack.append(tmp)

    if len(stack) != 1:
        raise Exception("Spurious empty rpn code for " + set.name + " :: " +
                counter.get('name') + ".\nThis is probably due to some unhandled RPN function, in the equation \"" +
                equation + "\"")

    value = stack[-1]

    if value[0] == "$":
        resolved_variable = resolve_variable(value, set, True)
        if resolved_variable == None:
            raise Exception("Failed to resolve variable " + operand + " in equation " + equation + " for " + set.name + " :: " + counter.get('name'));
        value = resolved_variable

    c("\nreturn " + value + ";")

def splice_rpn_expression(set, counter_name, expression):
    tokens = expression.split()
    stack = []

    for token in tokens:
        stack.append(token)
        while stack and stack[-1] in exp_ops:
            op = stack.pop()
            argc, callback = exp_ops[op]
            args = []
            for i in range(0, argc):
                operand = stack.pop()
                if operand[0] == "$":
                    resolved_variable = resolve_variable(operand, set, False)
                    if resolved_variable == None:
                        raise Exception("Failed to resolve variable " + operand + " in expression " + expression + " for " + set.name + " :: " + counter_name)
                    operand = resolved_variable
                args.append(operand)

            subexp = callback(args)

            stack.append(subexp)

    if len(stack) != 1:
        raise Exception("Spurious empty rpn expression for " + set.name + " :: " +
                counter_name + ".\nThis is probably due to some unhandled RPN operation, in the expression \"" +
                expression + "\"")

    value = stack[-1]

    if value[0] == "$":
        resolved_variable = resolve_variable(value, set, False)
        if resolved_variable == None:
            raise Exception("Failed to resolve variable " + operand + " in expression " + expression + " for " + set.name + " :: " + counter_name)
        value = resolved_variable

    return value

def output_counter_read(gen, set, counter):
    c("\n")
    c("/* {0} :: {1} */".format(set.name, counter.get('name')))

    if counter.read_hash in hashed_funcs:
        c("#define %s \\" % counter.read_sym)
        c_indent(3)
        c("%s" % hashed_funcs[counter.read_hash])
        c_outdent(3)
    else:
        ret_type = counter.get('data_type')
        if ret_type == "uint64":
            ret_type = "uint64_t"

        read_eq = counter.get('equation')

        c("static " + ret_type)
        c(counter.read_sym + "(UNUSED struct intel_perf_config *perf,\n")
        c_indent(len(counter.read_sym) + 1)
        c("const struct intel_perf_query_info *query,\n")
        c("const struct intel_perf_query_result *results)\n")
        c_outdent(len(counter.read_sym) + 1)

        c("{")
        c_indent(3)
        output_rpn_equation_code(set, counter, read_eq)
        c_outdent(3)
        c("}")

        hashed_funcs[counter.read_hash] = counter.read_sym


def output_counter_max(gen, set, counter):
    max_eq = counter.get('max_equation')

    if not counter.has_custom_max_func():
        return

    c("\n")
    c("/* {0} :: {1} */".format(set.name, counter.get('name')))

    if counter.max_hash in hashed_funcs:
        c("#define %s \\" % counter.max_sym)
        c_indent(3)
        c("%s" % hashed_funcs[counter.max_hash])
        c_outdent(3)
    else:
        ret_type = counter.get('data_type')
        if ret_type == "uint64":
            ret_type = "uint64_t"

        c("static " + ret_type)
        c(counter.max_sym + "(struct intel_perf_config *perf,\n")
        c_indent(len(counter.read_sym) + 1)
        c("const struct intel_perf_query_info *query,\n")
        c("const struct intel_perf_query_result *results)\n")
        c_outdent(len(counter.read_sym) + 1)
        c("{")
        c_indent(3)
        output_rpn_equation_code(set, counter, max_eq)
        c_outdent(3)
        c("}")

        hashed_funcs[counter.max_hash] = counter.max_sym


c_type_sizes = { "uint32_t": 4, "uint64_t": 8, "float": 4, "double": 8, "bool": 4 }
def sizeof(c_type):
    return c_type_sizes[c_type]

def pot_align(base, pot_alignment):
    return (base + pot_alignment - 1) & ~(pot_alignment - 1);

semantic_type_map = {
    "duration": "raw",
    "ratio": "event"
    }

def output_availability(set, availability, counter_name):
    expression = splice_rpn_expression(set, counter_name, availability)
    lines = expression.split(' && ')
    n_lines = len(lines)
    if n_lines == 1:
        c("if (" + lines[0] + ") {")
    else:
        c("if (" + lines[0] + " &&")
        c_indent(4)
        for i in range(1, (n_lines - 1)):
            c(lines[i] + " &&")
        c(lines[(n_lines - 1)] + ") {")
        c_outdent(4)


def output_units(unit):
    return unit.replace(' ', '_').upper()


# should a unit be visible in description?
units_map = {
    "bytes" : True,
    "cycles" : True,
    "eu atomic requests to l3 cache lines" : False,
    "eu bytes per l3 cache line" : False,
    "eu requests to l3 cache lines" : False,
    "eu sends to l3 cache lines" : False,
    "events" : True,
    "hz" : True,
    "messages" : True,
    "ns" : True,
    "number" : False,
    "percent" : True,
    "pixels" : True,
    "texels" : True,
    "threads" : True,
    "us" : True,
    "utilization" : False,
    "gbps" : True,
    }


def desc_units(unit):
    val = units_map.get(unit)
    if val is None:
        raise Exception("Unknown unit: " + unit)
    if val == False:
        return ""
    if unit == 'hz':
        unit = 'Hz'
    return "Unit: " + unit + "."


counter_key_tuple = collections.namedtuple(
    'counter_key',
    [
        'name',
        'description',
        'symbol_name',
        'mdapi_group',
        'semantic_type',
        'data_type',
        'units',
    ]
)


def counter_key(counter):
    return counter_key_tuple._make([counter.get(field) for field in counter_key_tuple._fields])


def output_counter_struct(set, counter, idx,
                          name_to_idx, desc_to_idx,
                          symbol_name_to_idx, category_to_idx):
    data_type = counter.data_type
    data_type_uc = data_type.upper()

    semantic_type = counter.semantic_type
    if semantic_type in semantic_type_map:
        semantic_type = semantic_type_map[semantic_type]

    semantic_type_uc = semantic_type.upper()

    c("[" + str(idx) + "] = {\n")
    c_indent(3)
    c(".name_idx = " + str(name_to_idx[counter.name]) + ",\n")
    c(".desc_idx = " + str(desc_to_idx[counter.description + " " + desc_units(counter.units)]) + ",\n")
    c(".symbol_name_idx = " + str(symbol_name_to_idx[counter.symbol_name]) + ",\n")
    c(".category_idx = " + str(category_to_idx[counter.mdapi_group]) + ",\n")
    c(".type = INTEL_PERF_COUNTER_TYPE_" + semantic_type_uc + ",\n")
    c(".data_type = INTEL_PERF_COUNTER_DATA_TYPE_" + data_type_uc + ",\n")
    c(".units = INTEL_PERF_COUNTER_UNITS_" + output_units(counter.units) + ",\n")
    c_outdent(3)
    c("},\n")


def output_counter_report(set, counter, counter_to_idx, current_offset):
    data_type = counter.get('data_type')
    data_type_uc = data_type.upper()
    c_type = data_type

    if "uint" in c_type:
        c_type = c_type + "_t"

    semantic_type = counter.get('semantic_type')
    if semantic_type in semantic_type_map:
        semantic_type = semantic_type_map[semantic_type]

    semantic_type_uc = semantic_type.upper()

    c("\n")

    availability = counter.get('availability')
    if availability:
        output_availability(set, availability, counter.get('name'))
        c_indent(3)

    key = counter_key(counter)
    idx = str(counter_to_idx[key])

    current_offset = pot_align(current_offset, sizeof(c_type))

    if data_type == 'uint64':
        c("intel_perf_query_add_counter_uint64(query, " + idx + ", " +
          str(current_offset) + ", " +
          set.max_funcs[counter.get('symbol_name')] + "," +
          set.read_funcs[counter.get('symbol_name')] + ");\n")
    else:
        c("intel_perf_query_add_counter_float(query, " + idx + ", " +
          str(current_offset) + ", " +
          set.max_funcs[counter.get('symbol_name')] + "," +
          set.read_funcs[counter.get('symbol_name')] + ");\n")


    if availability:
        c_outdent(3);
        c("}")

    return current_offset + sizeof(c_type)


def str_to_idx_table(strs):
    sorted_strs = sorted(strs)

    str_to_idx = collections.OrderedDict()
    str_to_idx[sorted_strs[0]] = 0
    previous = sorted_strs[0]

    for i in range(1, len(sorted_strs)):
        str_to_idx[sorted_strs[i]] = str_to_idx[previous] + len(previous) + 1
        previous = sorted_strs[i]

    return str_to_idx


def output_str_table(name: str, str_to_idx):
    c("\n")
    c("static const char " + name + "[] = {\n")
    c_indent(3)
    c("\n".join(f"/* {idx} */ \"{val}\\0\"" for val, idx in str_to_idx.items()))
    c_outdent(3)
    c("};\n")


register_types = {
    'FLEX': 'flex_regs',
    'NOA': 'mux_regs',
    'OA': 'b_counter_regs',
}

def compute_register_lengths(set):
    register_lengths = {}
    register_configs = set.findall('register_config')
    for register_config in register_configs:
        t = register_types[register_config.get('type')]
        if t not in register_lengths:
            register_lengths[t] = len(register_config.findall('register'))
        else:
            register_lengths[t] += len(register_config.findall('register'))

    return register_lengths


def generate_register_configs(set):
    register_configs = set.findall('register_config')

    for register_config in register_configs:
        t = register_types[register_config.get('type')]

        availability = register_config.get('availability')
        if availability:
            output_availability(set, availability, register_config.get('type') + ' register config')
            c_indent(3)

        registers = register_config.findall('register')
        c("static const struct intel_perf_query_register_prog %s[] = {" % t)
        c_indent(3)
        for register in registers:
            c("{ .reg = %s, .val = %s }," % (register.get('address'), register.get('value')))
        c_outdent(3)
        c("};")
        c("query->config.%s = %s;" % (t, t))
        c("query->config.n_%s = ARRAY_SIZE(%s);" % (t, t))

        if availability:
            c_outdent(3)
            c("}")
        c("\n")


# Wraps a <counter> element from the oa-*.xml files.
class Counter:
    def __init__(self, set, xml):
        self.xml = xml
        self.set = set
        self.read_hash = None
        self.max_hash = None

        self.read_sym = "{0}__{1}__{2}__read".format(self.set.gen.chipset,
                                                     self.set.underscore_name,
                                                     self.xml.get('underscore_name'))
        self.max_sym = self.build_max_sym()

    def get(self, prop):
        return self.xml.get(prop)

    # Compute the hash of a counter's equation by expanding (including all the
    # sub-equations it depends on)
    def compute_hashes(self):
        if self.read_hash is not None:
            return

        def replace_token(token):
            if token[0] != "$":
                return token
            if token not in self.set.counter_vars:
                return token
            self.set.counter_vars[token].compute_hashes()
            return self.set.counter_vars[token].read_hash

        read_eq = self.xml.get('equation')
        self.read_hash = ' '.join(map(replace_token, read_eq.split()))

        max_eq = self.xml.get('max_equation')
        if max_eq:
            self.max_hash = ' '.join(map(replace_token, max_eq.split()))

    def has_custom_max_func(self):
        max_eq = self.xml.get('max_equation')
        if not max_eq:
            return False

        try:
            val = float(max_eq)
            if val == 100:
                return False
        except ValueError:
            pass

        for token in max_eq.split():
            if token[0] == '$' and resolve_variable(token, self.set, True) == None:
                print("unresolved token " + token)
                return False
        return True

    def build_max_sym(self):
        max_eq = self.xml.get('max_equation')
        if not max_eq:
            return "NULL"

        try:
            val = float(max_eq)
            if val == 100:
                if self.xml.get('data_type') == 'uint64':
                    return "percentage_max_uint64"
                else:
                    return "percentage_max_float"
        except ValueError:
            pass

        assert self.has_custom_max_func()
        return "{0}__{1}__{2}__max".format(self.set.gen.chipset,
                                           self.set.underscore_name,
                                           self.xml.get('underscore_name'))


# Wraps a <set> element from the oa-*.xml files.
class Set:
    def __init__(self, gen, xml):
        self.gen = gen
        self.xml = xml

        self.counter_vars = {}
        self.max_funcs = {}
        self.read_funcs = {}

        xml_counters = self.xml.findall("counter")
        self.counters = []
        for xml_counter in xml_counters:
            counter = Counter(self, xml_counter)
            self.counters.append(counter)
            self.counter_vars['$' + counter.get('symbol_name')] = counter
            self.read_funcs[counter.get('symbol_name')] = counter.read_sym
            self.max_funcs[counter.get('symbol_name')] = counter.max_sym

        for counter in self.counters:
            counter.compute_hashes()

    @property
    def hw_config_guid(self):
        return self.xml.get('hw_config_guid')

    @property
    def name(self):
        return self.xml.get('name')

    @property
    def symbol_name(self):
        return self.xml.get('symbol_name')

    @property
    def underscore_name(self):
        return self.xml.get('underscore_name')

    def findall(self, path):
        return self.xml.findall(path)

    def find(self, path):
        return self.xml.find(path)


# Wraps an entire oa-*.xml file.
class Gen:
    def __init__(self, filename):
        self.filename = filename
        self.xml = et.parse(self.filename)
        self.chipset = self.xml.find('.//set').get('chipset').lower()
        self.sets = []

        for xml_set in self.xml.findall(".//set"):
            self.sets.append(Set(self, xml_set))


def main():
    global c_file
    global header_file

    parser = argparse.ArgumentParser()
    parser.add_argument("--header", help="Header file to write", required=True)
    parser.add_argument("--code", help="C file to write", required=True)
    parser.add_argument("xml_files", nargs='+', help="List of xml metrics files to process")

    args = parser.parse_args()

    c_file = open(args.code, 'w')
    header_file = open(args.header, 'w')

    gens = []
    for xml_file in args.xml_files:
        gens.append(Gen(xml_file))


    copyright = textwrap.dedent("""\
        /* Autogenerated file, DO NOT EDIT manually! generated by {}
         *
         * Copyright (c) 2015 Intel Corporation
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
         * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
         * DEALINGS IN THE SOFTWARE.
         */

        """).format(os.path.basename(__file__))

    h(copyright)
    h(textwrap.dedent("""\
        #pragma once

        struct intel_perf_config;

        """))

    c(copyright)
    c(textwrap.dedent("""\
        #include <stdint.h>
        #include <stdbool.h>

        #include <drm-uapi/i915_drm.h>

        #include "util/hash_table.h"
        #include "util/ralloc.h"

        """))

    c("#include \"" + os.path.basename(args.header) + "\"")

    c(textwrap.dedent("""\
        #include "perf/intel_perf.h"
        #include "perf/intel_perf_setup.h"
        """))

    names = builtins.set()
    descs = builtins.set()
    symbol_names = builtins.set()
    categories = builtins.set()
    for gen in gens:
        for set in gen.sets:
            for counter in set.counters:
                names.add(counter.get('name'))
                symbol_names.add(counter.get('symbol_name'))
                descs.add(counter.get('description') + " " + desc_units(counter.get('units')))
                categories.add(counter.get('mdapi_group'))

    name_to_idx = str_to_idx_table(names)
    output_str_table("name", name_to_idx)

    desc_to_idx = str_to_idx_table(descs)
    output_str_table("desc", desc_to_idx)

    symbol_name_to_idx = str_to_idx_table(symbol_names)
    output_str_table("symbol_name", symbol_name_to_idx)

    category_to_idx = str_to_idx_table(categories)
    output_str_table("category", category_to_idx)

    # Print out all equation functions.
    for gen in gens:
        for set in gen.sets:
            for counter in set.counters:
                output_counter_read(gen, set, counter)
                output_counter_max(gen, set, counter)

    c("\n")
    c("static const struct intel_perf_query_counter_data counters[] = {\n")
    c_indent(3)

    counter_to_idx = collections.OrderedDict()
    idx = 0
    for gen in gens:
        for set in gen.sets:
            for counter in set.counters:
                key = counter_key(counter)
                if key not in counter_to_idx:
                    counter_to_idx[key] = idx
                    output_counter_struct(set, key, idx,
                                          name_to_idx,
                                          desc_to_idx,
                                          symbol_name_to_idx,
                                          category_to_idx)
                    idx += 1

    c_outdent(3)
    c("};\n\n")

    c(textwrap.dedent("""\
        static void ATTRIBUTE_NOINLINE
        intel_perf_query_add_counter_uint64(struct intel_perf_query_info *query,
                                            int counter_idx, size_t offset,
                                            intel_counter_read_uint64_t oa_counter_max,
                                            intel_counter_read_uint64_t oa_counter_read)
        {
           struct intel_perf_query_counter *dest = &query->counters[query->n_counters++];
           const struct intel_perf_query_counter_data *counter = &counters[counter_idx];

           dest->name = &name[counter->name_idx];
           dest->desc = &desc[counter->desc_idx];
           dest->symbol_name = &symbol_name[counter->symbol_name_idx];
           dest->category = &category[counter->category_idx];

           dest->offset = offset;
           dest->type = counter->type;
           dest->data_type = counter->data_type;
           dest->units = counter->units;
           dest->oa_counter_max_uint64 = oa_counter_max;
           dest->oa_counter_read_uint64 = oa_counter_read;
        }

        static void ATTRIBUTE_NOINLINE
        intel_perf_query_add_counter_float(struct intel_perf_query_info *query,
                                           int counter_idx, size_t offset,
                                           intel_counter_read_float_t oa_counter_max,
                                           intel_counter_read_float_t oa_counter_read)
        {
           struct intel_perf_query_counter *dest = &query->counters[query->n_counters++];
           const struct intel_perf_query_counter_data *counter = &counters[counter_idx];

           dest->name = &name[counter->name_idx];
           dest->desc = &desc[counter->desc_idx];
           dest->symbol_name = &symbol_name[counter->symbol_name_idx];
           dest->category = &category[counter->category_idx];

           dest->offset = offset;
           dest->type = counter->type;
           dest->data_type = counter->data_type;
           dest->units = counter->units;
           dest->oa_counter_max_float = oa_counter_max;
           dest->oa_counter_read_float = oa_counter_read;
        }

        static float ATTRIBUTE_NOINLINE
        percentage_max_float(struct intel_perf_config *perf,
                             const struct intel_perf_query_info *query,
                             const struct intel_perf_query_result *results)
        {
           return 100;
        }

        static uint64_t ATTRIBUTE_NOINLINE
        percentage_max_uint64(struct intel_perf_config *perf,
                              const struct intel_perf_query_info *query,
                              const struct intel_perf_query_result *results)
        {
           return 100;
        }
        """))

    # Print out all metric sets registration functions for each set in each
    # generation.
    for gen in gens:
        for set in gen.sets:
            counters = set.counters

            c("\n")
            c("\nstatic void\n")
            c("{0}_register_{1}_counter_query(struct intel_perf_config *perf)\n".format(gen.chipset, set.underscore_name))
            c("{\n")
            c_indent(3)

            c("struct intel_perf_query_info *query = intel_query_alloc(perf, %u);\n" % len(counters))
            c("\n")
            c("query->name = \"" + set.name + "\";\n")
            c("query->symbol_name = \"" + set.symbol_name + "\";\n")
            c("query->guid = \"" + set.hw_config_guid + "\";\n")

            c("\n")
            c("struct intel_perf_query_counter *counter = query->counters;\n")

            c("\n")
            c("/* Note: we're assuming there can't be any variation in the definition ")
            c(" * of a query between contexts so it's ok to describe a query within a ")
            c(" * global variable which only needs to be initialized once... */")
            c("\nif (!query->data_size) {")
            c_indent(3)

            generate_register_configs(set)

            offset = 0
            for counter in counters:
                offset = output_counter_report(set, counter, counter_to_idx, offset)


            c("\ncounter = &query->counters[query->n_counters - 1];\n")
            c("query->data_size = counter->offset + intel_perf_query_counter_get_size(counter);\n")

            c_outdent(3)
            c("}");

            c("\n_mesa_hash_table_insert(perf->oa_metrics_table, query->guid, query);")

            c_outdent(3)
            c("}\n")

        h("void intel_oa_register_queries_" + gen.chipset + "(struct intel_perf_config *perf);\n")

        c("\nvoid")
        c("intel_oa_register_queries_" + gen.chipset + "(struct intel_perf_config *perf)")
        c("{")
        c_indent(3)

        for set in gen.sets:
            c("{0}_register_{1}_counter_query(perf);".format(gen.chipset, set.underscore_name))

        c_outdent(3)
        c("}")


if __name__ == '__main__':
    main()
