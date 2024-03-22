#
# Copyright (c) 2020 Valve Corporation
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
import re
import sys
import os.path
import struct
import string
import copy
from math import floor

if os.isatty(sys.stdout.fileno()):
    set_red = "\033[31m"
    set_green = "\033[1;32m"
    set_normal = "\033[0m"
else:
    set_red = ''
    set_green = ''
    set_normal = ''

initial_code = '''
import re

def insert_code(code):
    insert_queue.append(CodeCheck(code, current_position))

def insert_pattern(pattern):
    insert_queue.append(PatternCheck(pattern, False, current_position))

def vector_gpr(prefix, name, size, align):
    insert_code(f'{name} = {name}0')
    for i in range(size):
        insert_code(f'{name}{i} = {name}0 + {i}')
    insert_code(f'success = {name}0 + {size - 1} == {name}{size - 1}')
    insert_code(f'success = {name}0 % {align} == 0')
    return f'{prefix}[#{name}0:#{name}{size - 1}]'

def sgpr_vector(name, size, align):
    return vector_gpr('s', name, size, align)

funcs.update({
    's64': lambda name: vector_gpr('s', name, 2, 2),
    's96': lambda name: vector_gpr('s', name, 3, 2),
    's128': lambda name: vector_gpr('s', name, 4, 4),
    's256': lambda name: vector_gpr('s', name, 8, 4),
    's512': lambda name: vector_gpr('s', name, 16, 4),
})
for i in range(2, 14):
    funcs['v%d' % (i * 32)] = lambda name: vector_gpr('v', name, i, 1)

def _match_func(names):
    for name in names.split(' '):
        insert_code(f'funcs["{name}"] = lambda _: {name}')
    return ' '.join(f'${name}' for name in names.split(' '))

funcs['match_func'] = _match_func

def search_re(pattern):
    global success
    success = re.search(pattern, output.read_line()) != None and success

'''

class Check:
    def __init__(self, data, position):
        self.data = data.rstrip()
        self.position = position

    def run(self, state):
        pass

class CodeCheck(Check):
    def run(self, state):
        indent = 0
        first_line = [l for l in self.data.split('\n') if l.strip() != ''][0]
        indent_amount = len(first_line) - len(first_line.lstrip())
        indent = first_line[:indent_amount]
        new_lines = []
        for line in self.data.split('\n'):
            if line.strip() == '':
                new_lines.append('')
                continue
            if line[:indent_amount] != indent:
                state.result.log += 'unexpected indent in code check:\n'
                state.result.log += self.data + '\n'
                return False
            new_lines.append(line[indent_amount:])
        code = '\n'.join(new_lines)

        try:
            exec(code, state.g)
            state.result.log += state.g['log']
            state.g['log'] = ''
        except BaseException as e:
            state.result.log += 'code check at %s raised exception:\n' % self.position
            state.result.log += code + '\n'
            state.result.log += str(e)
            return False
        if not state.g['success']:
            state.result.log += 'code check at %s failed:\n' % self.position
            state.result.log += code + '\n'
            return False
        return True

class StringStream:
    class Pos:
        def __init__(self):
            self.line = 1
            self.column = 1

    def __init__(self, data, name):
        self.name = name
        self.data = data
        self.offset = 0
        self.pos = StringStream.Pos()

    def reset(self):
        self.offset = 0
        self.pos = StringStream.Pos()

    def peek(self, num=1):
        return self.data[self.offset:self.offset+num]

    def peek_test(self, chars):
        c = self.peek(1)
        return c != '' and c in chars

    def read(self, num=4294967296):
        res = self.peek(num)
        self.offset += len(res)
        for c in res:
            if c == '\n':
                self.pos.line += 1
                self.pos.column = 1
            else:
                self.pos.column += 1
        return res

    def get_line(self, num):
        return self.data.split('\n')[num - 1].rstrip()

    def read_line(self):
        line = ''
        while self.peek(1) not in ['\n', '']:
            line += self.read(1)
        self.read(1)
        return line

    def skip_whitespace(self, inc_line):
        chars = [' ', '\t'] + (['\n'] if inc_line else [])
        while self.peek(1) in chars:
            self.read(1)

    def get_number(self):
        num = ''
        while self.peek() in string.digits:
            num += self.read(1)
        return num

    def check_identifier(self):
        return self.peek_test(string.ascii_letters + '_')

    def get_identifier(self):
        res = ''
        if self.check_identifier():
            while self.peek_test(string.ascii_letters + string.digits + '_'):
                res += self.read(1)
        return res

def format_error_lines(at, line_num, column_num, ctx, line):
    pred = '%s line %d, column %d of %s: "' % (at, line_num, column_num, ctx)
    return [pred + line + '"',
            '-' * (column_num - 1 + len(pred)) + '^']

class MatchResult:
    def __init__(self, pattern):
        self.success = True
        self.func_res = None
        self.pattern = pattern
        self.pattern_pos = StringStream.Pos()
        self.output_pos = StringStream.Pos()
        self.fail_message = ''

    def set_pos(self, pattern, output):
        self.pattern_pos.line = pattern.pos.line
        self.pattern_pos.column = pattern.pos.column
        self.output_pos.line = output.pos.line
        self.output_pos.column = output.pos.column

    def fail(self, msg):
        self.success = False
        self.fail_message = msg

    def format_pattern_pos(self):
        pat_pos = self.pattern_pos
        pat_line = self.pattern.get_line(pat_pos.line)
        res = format_error_lines('at', pat_pos.line, pat_pos.column, 'pattern', pat_line)
        func_res = self.func_res
        while func_res:
            pat_pos = func_res.pattern_pos
            pat_line = func_res.pattern.get_line(pat_pos.line)
            res += format_error_lines('in', pat_pos.line, pat_pos.column, func_res.pattern.name, pat_line)
            func_res = func_res.func_res
        return '\n'.join(res)

def do_match(g, pattern, output, skip_lines, in_func=False):
    assert(not in_func or not skip_lines)

    if not in_func:
        output.skip_whitespace(False)
    pattern.skip_whitespace(False)

    old_g = copy.copy(g)
    old_g_keys = list(g.keys())
    res = MatchResult(pattern)
    escape = False
    while True:
        res.set_pos(pattern, output)

        c = pattern.read(1)
        fail = False
        if c == '':
            break
        elif output.peek() == '':
            res.fail('unexpected end of output')
        elif c == '\\':
            escape = True
            continue
        elif c == '\n':
            old_line = output.pos.line
            output.skip_whitespace(True)
            if output.pos.line == old_line:
                res.fail('expected newline in output')
        elif not escape and c == '#':
            num = output.get_number()
            if num == '':
                res.fail('expected number in output')
            elif pattern.check_identifier():
                name = pattern.get_identifier()
                if name in g and int(num) != g[name]:
                    res.fail('unexpected number for \'%s\': %d (expected %d)' % (name, int(num), g[name]))
                elif name != '_':
                    g[name] = int(num)
        elif not escape and c == '$':
            name = pattern.get_identifier()

            val = ''
            while not output.peek_test(string.whitespace):
                val += output.read(1)

            if name in g and val != g[name]:
                res.fail('unexpected value for \'%s\': \'%s\' (expected \'%s\')' % (name, val, g[name]))
            elif name != '_':
                g[name] = val
        elif not escape and c == '%' and pattern.check_identifier():
            if output.read(1) != '%':
                res.fail('expected \'%\' in output')
            else:
                num = output.get_number()
                if num == '':
                    res.fail('expected number in output')
                else:
                    name = pattern.get_identifier()
                    if name in g and int(num) != g[name]:
                        res.fail('unexpected number for \'%s\': %d (expected %d)' % (name, int(num), g[name]))
                    elif name != '_':
                        g[name] = int(num)
        elif not escape and c == '@' and pattern.check_identifier():
            name = pattern.get_identifier()
            args = ''
            if pattern.peek_test('('):
                pattern.read(1)
                while pattern.peek() not in ['', ')']:
                    args += pattern.read(1)
                assert(pattern.read(1) == ')')
            func_res = g['funcs'][name](args)
            match_res = do_match(g, StringStream(func_res, 'expansion of "%s(%s)"' % (name, args)), output, False, True)
            if not match_res.success:
                res.func_res = match_res
                res.output_pos = match_res.output_pos
                res.fail(match_res.fail_message)
        elif not escape and c == ' ':
            while pattern.peek_test(' '):
                pattern.read(1)

            read_whitespace = False
            while output.peek_test(' \t'):
                output.read(1)
                read_whitespace = True
            if not read_whitespace:
                res.fail('expected whitespace in output, got %r' % (output.peek(1)))
        else:
            outc = output.peek(1)
            if outc != c:
                res.fail('expected %r in output, got %r' % (c, outc))
            else:
                output.read(1)
        if not res.success:
            if skip_lines and output.peek() != '':
                g.clear()
                g.update(old_g)
                res.success = True
                output.read_line()
                pattern.reset()
                output.skip_whitespace(False)
                pattern.skip_whitespace(False)
            else:
                return res

        escape = False

    if not in_func:
        while output.peek() in [' ', '\t']:
            output.read(1)

        if output.read(1) not in ['', '\n']:
            res.fail('expected end of output')
            return res

    return res

class PatternCheck(Check):
    def __init__(self, data, search, position):
        Check.__init__(self, data, position)
        self.search = search

    def run(self, state):
        pattern_stream = StringStream(self.data.rstrip(), 'pattern')
        res = do_match(state.g, pattern_stream, state.g['output'], self.search)
        if not res.success:
            state.result.log += 'pattern at %s failed: %s\n' % (self.position, res.fail_message)
            state.result.log += res.format_pattern_pos() + '\n\n'
            if not self.search:
                out_line = state.g['output'].get_line(res.output_pos.line)
                state.result.log += '\n'.join(format_error_lines('at', res.output_pos.line, res.output_pos.column, 'output', out_line))
            else:
                state.result.log += 'output was:\n'
                state.result.log += state.g['output'].data.rstrip() + '\n'
            return False
        return True

class CheckState:
    def __init__(self, result, variant, checks, output):
        self.result = result
        self.variant = variant
        self.checks = checks

        self.checks.insert(0, CodeCheck(initial_code, None))
        self.insert_queue = []

        self.g = {'success': True, 'funcs': {}, 'insert_queue': self.insert_queue,
                  'variant': variant, 'log': '', 'output': StringStream(output, 'output'),
                  'CodeCheck': CodeCheck, 'PatternCheck': PatternCheck,
                  'current_position': ''}

class TestResult:
    def __init__(self, expected):
        self.result = ''
        self.expected = expected
        self.log = ''

def check_output(result, variant, checks, output):
    state = CheckState(result, variant, checks, output)

    while len(state.checks):
        check = state.checks.pop(0)
        state.current_position = check.position
        if not check.run(state):
            result.result = 'failed'
            return

        for check in state.insert_queue[::-1]:
            state.checks.insert(0, check)
        state.insert_queue.clear()

    result.result = 'passed'
    return

def parse_check(variant, line, checks, pos):
    if line.startswith(';'):
        line = line[1:]
        if len(checks) and isinstance(checks[-1], CodeCheck):
            checks[-1].data += '\n' + line
        else:
            checks.append(CodeCheck(line, pos))
    elif line.startswith('!'):
        checks.append(PatternCheck(line[1:], False, pos))
    elif line.startswith('>>'):
        checks.append(PatternCheck(line[2:], True, pos))
    elif line.startswith('~'):
        end = len(line)
        start = len(line)
        for c in [';', '!', '>>']:
            if line.find(c) != -1 and line.find(c) < end:
                end = line.find(c)
        if end != len(line):
            match = re.match(line[1:end], variant)
            if match and match.end() == len(variant):
                parse_check(variant, line[end:], checks, pos)

def parse_test_source(test_name, variant, fname):
    in_test = False
    test = []
    expected_result = 'passed'
    line_num = 1
    for line in open(fname, 'r').readlines():
        if line.startswith('BEGIN_TEST(%s)' % test_name):
            in_test = True
        elif line.startswith('BEGIN_TEST_TODO(%s)' % test_name):
            in_test = True
            expected_result = 'todo'
        elif line.startswith('BEGIN_TEST_FAIL(%s)' % test_name):
            in_test = True
            expected_result = 'failed'
        elif line.startswith('END_TEST'):
            in_test = False
        elif in_test:
            test.append((line_num, line.strip()))
        line_num += 1

    checks = []
    for line_num, check in [(line_num, l[2:]) for line_num, l in test if l.startswith('//')]:
         parse_check(variant, check, checks, 'line %d of %s' % (line_num, os.path.split(fname)[1]))

    return checks, expected_result

def parse_and_check_test(test_name, variant, test_file, output, current_result):
    checks, expected = parse_test_source(test_name, variant, test_file)

    result = TestResult(expected)
    if len(checks) == 0:
        result.result = 'empty'
        result.log = 'no checks found'
    elif current_result != None:
        result.result, result.log = current_result
    else:
        check_output(result, variant, checks, output)
        if result.result == 'failed' and expected == 'todo':
            result.result = 'todo'

    return result

def print_results(results, output, expected):
    results = {name: result for name, result in results.items() if result.result == output}
    results = {name: result for name, result in results.items() if (result.result == result.expected) == expected}

    if not results:
        return 0

    print('%s tests (%s):' % (output, 'expected' if expected else 'unexpected'))
    for test, result in results.items():
        color = '' if expected else set_red
        print('   %s%s%s' % (color, test, set_normal))
        if result.log.strip() != '':
            for line in result.log.rstrip().split('\n'):
                print('      ' + line.rstrip())
    print('')

    return len(results)

def get_cstr(fp):
    res = b''
    while True:
        c = fp.read(1)
        if c == b'\x00':
            return res.decode('utf-8')
        else:
            res += c

if __name__ == "__main__":
   results = {}

   stdin = sys.stdin.buffer
   while True:
       packet_type = stdin.read(4)
       if packet_type == b'':
           break;

       test_name = get_cstr(stdin)
       test_variant = get_cstr(stdin)
       if test_variant != '':
           full_name = test_name + '/' + test_variant
       else:
           full_name = test_name

       test_source_file = get_cstr(stdin)
       current_result = None
       if ord(stdin.read(1)):
           current_result = (get_cstr(stdin), get_cstr(stdin))
       code_size = struct.unpack("=L", stdin.read(4))[0]
       code = stdin.read(code_size).decode('utf-8')

       results[full_name] = parse_and_check_test(test_name, test_variant, test_source_file, code, current_result)

   result_types = ['passed', 'failed', 'todo', 'empty']
   num_expected = 0
   num_unexpected = 0
   for t in result_types:
       num_expected += print_results(results, t, True)
   for t in result_types:
       num_unexpected += print_results(results, t, False)
   num_expected_skipped = print_results(results, 'skipped', True)
   num_unexpected_skipped = print_results(results, 'skipped', False)

   num_unskipped = len(results) - num_expected_skipped - num_unexpected_skipped
   color = set_red if num_unexpected else set_green
   print('%s%d (%.0f%%) of %d unskipped tests had an expected result%s' % (color, num_expected, floor(num_expected / num_unskipped * 100), num_unskipped, set_normal))
   if num_unexpected_skipped:
       print('%s%d tests had been unexpectedly skipped%s' % (set_red, num_unexpected_skipped, set_normal))

   if num_unexpected:
       sys.exit(1)
