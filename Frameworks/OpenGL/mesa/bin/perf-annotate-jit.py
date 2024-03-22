#!/usr/bin/env python3
#
# Copyright 2012 VMware Inc
# Copyright 2008-2009 Jose Fonseca
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in
# all copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
# THE SOFTWARE.
#

"""Perf annotate for JIT code.

Linux `perf annotate` does not work with JIT code.  This script takes the data
produced by `perf script` command, plus the diassemblies outputted by gallivm
into /tmp/perf-XXXXX.map.asm and produces output similar to `perf annotate`.

See docs/llvmpipe.rst for usage instructions.

The `perf script` output parser was derived from the gprof2dot.py script.
"""


import sys
import os.path
import re
import optparse
import subprocess


class Parser:
    """Parser interface."""

    def __init__(self):
        pass

    def parse(self):
        raise NotImplementedError


class LineParser(Parser):
    """Base class for parsers that read line-based formats."""

    def __init__(self, file):
        Parser.__init__(self)
        self._file = file
        self.__line = None
        self.__eof = False
        self.line_no = 0

    def readline(self):
        line = self._file.readline()
        if not line:
            self.__line = ''
            self.__eof = True
        else:
            self.line_no += 1
        self.__line = line.rstrip('\r\n')

    def lookahead(self):
        assert self.__line is not None
        return self.__line

    def consume(self):
        assert self.__line is not None
        line = self.__line
        self.readline()
        return line

    def eof(self):
        assert self.__line is not None
        return self.__eof


mapFile = None

def lookupMap(filename, matchSymbol):
    global mapFile
    mapFile = filename
    stream = open(filename, 'rt')
    for line in stream:
        start, length, symbol = line.split()

        start = int(start, 16)
        length = int(length,16)

        if symbol == matchSymbol:
            return start

    return None

def lookupAsm(filename, desiredFunction):
    stream = open(filename + '.asm', 'rt')
    while stream.readline() != desiredFunction + ':\n':
        pass

    asm = []
    line = stream.readline().strip()
    while line:
        addr, instr = line.split(':', 1)
        addr = int(addr)
        asm.append((addr, instr))
        line = stream.readline().strip()

    return asm



samples = {}


class PerfParser(LineParser):
    """Parser for linux perf callgraph output.

    It expects output generated with

        perf record -g
        perf script
    """

    def __init__(self, infile, symbol):
        LineParser.__init__(self, infile)
        self.symbol = symbol

    def readline(self):
        # Override LineParser.readline to ignore comment lines
        while True:
            LineParser.readline(self)
            if self.eof() or not self.lookahead().startswith('#'):
                break

    def parse(self):
        # read lookahead
        self.readline()

        while not self.eof():
            self.parse_event()

        asm = lookupAsm(mapFile, self.symbol)

        addresses = samples.keys()
        addresses.sort()
        total_samples = 0

        sys.stdout.write('%s:\n' % self.symbol)
        for address, instr in asm:
            try:
                sample = samples.pop(address)
            except KeyError:
                sys.stdout.write(6*' ')
            else:
                sys.stdout.write('%6u' % (sample))
                total_samples += sample
            sys.stdout.write('%6u: %s\n' % (address, instr))
        print('total:', total_samples)
        assert len(samples) == 0

        sys.exit(0)

    def parse_event(self):
        if self.eof():
            return

        line = self.consume()
        assert line

        callchain = self.parse_callchain()
        if not callchain:
            return

    def parse_callchain(self):
        callchain = []
        while self.lookahead():
            function = self.parse_call(len(callchain) == 0)
            if function is None:
                break
            callchain.append(function)
        if self.lookahead() == '':
            self.consume()
        return callchain

    call_re = re.compile(r'^\s+(?P<address>[0-9a-fA-F]+)\s+(?P<symbol>.*)\s+\((?P<module>[^)]*)\)$')

    def parse_call(self, first):
        line = self.consume()
        mo = self.call_re.match(line)
        assert mo
        if not mo:
            return None

        if not first:
            return None

        function_name = mo.group('symbol')
        if not function_name:
            function_name = mo.group('address')

        module = mo.group('module')

        function_id = function_name + ':' + module

        address = mo.group('address')
        address = int(address, 16)

        if function_name != self.symbol:
            return None

        start_address = lookupMap(module, function_name)
        address -= start_address

        #print(function_name, module, address)

        samples[address] = samples.get(address, 0) + 1

        return True


def main():
    """Main program."""

    optparser = optparse.OptionParser(
        usage="\n\t%prog [options] symbol_name")
    (options, args) = optparser.parse_args(sys.argv[1:])
    if len(args) != 1:
        optparser.error('wrong number of arguments')

    symbol = args[0]

    p = subprocess.Popen(['perf', 'script'], stdout=subprocess.PIPE, stderr=subprocess.PIPE)
    parser = PerfParser(p.stdout, symbol)
    parser.parse()


if __name__ == '__main__':
    main()


# vim: set sw=4 et:
