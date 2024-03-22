#!/usr/bin/env python3
# coding=utf-8
##########################################################################
#
# pytracediff - Compare Gallium XML trace files
# (C) Copyright 2022 Matti 'ccr' Hämäläinen <ccr@tnsp.org>
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
##########################################################################

from parse import *
import os
import sys
import re
import signal
import functools
import argparse
import difflib
import subprocess

assert sys.version_info >= (3, 6), 'Python >= 3.6 required'


###
### ANSI color codes
###
PKK_ANSI_ESC       = '\33['
PKK_ANSI_NORMAL    = '0m'
PKK_ANSI_RED       = '31m'
PKK_ANSI_GREEN     = '32m'
PKK_ANSI_YELLOW    = '33m'
PKK_ANSI_PURPLE    = '35m'
PKK_ANSI_BOLD      = '1m'
PKK_ANSI_ITALIC    = '3m'


###
### Utility functions and classes
###
def pkk_fatal(msg):
    print(f"ERROR: {msg}", file=sys.stderr)
    if outpipe is not None:
        outpipe.terminate()
    sys.exit(1)


def pkk_info(msg):
    print(msg, file=sys.stderr)


def pkk_output(outpipe, msg):
    if outpipe is not None:
        print(msg, file=outpipe.stdin)
    else:
        print(msg)


def pkk_signal_handler(signal, frame):
    print("\nQuitting due to SIGINT / Ctrl+C!")
    if outpipe is not None:
        outpipe.terminate()
    sys.exit(1)


def pkk_arg_range(vstr, vmin, vmax):
    try:
        value = int(vstr)
    except Exception as e:
        raise argparse.ArgumentTypeError(f"value '{vstr}' is not an integer")

    value = int(vstr)
    if value < vmin or value > vmax:
        raise argparse.ArgumentTypeError(f"value {value} not in range {vmin}-{vmax}")
    else:
        return value


class PKKArgumentParser(argparse.ArgumentParser):
    def print_help(self):
        print("pytracediff - Compare Gallium XML trace files\n"
        "(C) Copyright 2022 Matti 'ccr' Hämäläinen <ccr@tnsp.org>\n")
        super().print_help()
        print("\nList of junk calls:")
        for klass, call in sorted(trace_ignore_calls):
            print(f"  {klass}::{call}")

    def error(self, msg):
        self.print_help()
        print(f"\nERROR: {msg}", file=sys.stderr)
        sys.exit(2)


class PKKTraceParser(TraceParser):
    def __init__(self, stream, options, state):
        TraceParser.__init__(self, stream, options, state)
        self.call_stack = []

    def handle_call(self, call):
        self.call_stack.append(call)


class PKKPrettyPrinter(PrettyPrinter):
    def __init__(self, options):
        self.options = options

    def entry_start(self, show_args):
        self.data = []
        self.line = ""
        self.show_args = show_args

    def entry_get(self):
        if self.line != "":
            self.data.append(self.line)
        return self.data

    def text(self, text):
        self.line += text

    def literal(self, text):
        self.line += text

    def function(self, text):
        self.line += text

    def variable(self, text):
        self.line += text

    def address(self, text):
        self.line += text

    def newline(self):
        self.data.append(self.line)
        self.line = ""


    def visit_literal(self, node):
        if node.value is None:
            self.literal("NULL")
        elif isinstance(node.value, str):
            self.literal('"' + node.value + '"')
        else:
            self.literal(repr(node.value))

    def visit_blob(self, node):
        self.address("blob()")

    def visit_named_constant(self, node):
        self.literal(node.name)

    def visit_array(self, node):
        self.text("{")
        sep = ""
        for value in node.elements:
            self.text(sep)
            if sep != "":
                self.newline()
            value.visit(self)
            sep = ", "
        self.text("}")

    def visit_struct(self, node):
        self.text("{")
        sep = ""
        for name, value in node.members:
            self.text(sep)
            if sep != "":
                self.newline()
            self.variable(name)
            self.text(" = ")
            value.visit(self)
            sep = ", "
        self.text("}")

    def visit_pointer(self, node):
        if self.options.named_ptrs:
            self.address(node.named_address())
        else:
            self.address(node.address)

    def visit_call(self, node):
        if not self.options.suppress_variants:
            self.text(f"[{node.no:8d}] ")

        if node.klass is not None:
            self.function(node.klass +"::"+ node.method)
        else:
            self.function(node.method)

        if not self.options.method_only or self.show_args:
            self.text("(")
            if len(node.args):
                self.newline()
                sep = ""
                for name, value in node.args:
                    self.text(sep)
                    if sep != "":
                        self.newline()
                    self.variable(name)
                    self.text(" = ")
                    value.visit(self)
                    sep = ", "
                self.newline()

            self.text(")")

            if node.ret is not None:
                self.text(" = ")
                node.ret.visit(self)

        if not self.options.suppress_variants and node.time is not None:
            self.text(" // time ")
            node.time.visit(self)


def pkk_parse_trace(filename, options, state):
    pkk_info(f"Parsing {filename} ...")
    try:
        if filename.endswith(".gz"):
            from gzip import GzipFile
            stream = io.TextIOWrapper(GzipFile(filename, "rb"))
        elif filename.endswith(".bz2"):
            from bz2 import BZ2File
            stream = io.TextIOWrapper(BZ2File(filename, "rb"))
        else:
            stream = open(filename, "rt")

    except OSError as e:
        pkk_fatal(str(e))

    parser = PKKTraceParser(stream, options, state)
    parser.parse()

    return parser.call_stack


def pkk_get_line(data, nline):
    if nline < len(data):
        return data[nline]
    else:
        return None


def pkk_format_line(line, indent, width):
    if line is not None:
        tmp = indent + line
        if len(tmp) > width:
            return tmp[0:(width - 3)] + "..."
        else:
            return tmp
    else:
        return ""


###
### Main program starts
###
if __name__ == "__main__":
    ### Check if output is a terminal
    outpipe = None
    redirect = False

    try:
        defwidth = os.get_terminal_size().columns
        redirect = True
    except OSError:
        defwidth = 80

    signal.signal(signal.SIGINT, pkk_signal_handler)

    ### Parse arguments
    optparser = PKKArgumentParser(
        usage="%(prog)s [options] <tracefile #1> <tracefile #2>\n")

    optparser.add_argument("filename1",
        type=str, action="store",
        metavar="<tracefile #1>",
        help="Gallium trace XML filename (plain or .gz, .bz2)")

    optparser.add_argument("filename2",
        type=str, action="store",
        metavar="<tracefile #2>",
        help="Gallium trace XML filename (plain or .gz, .bz2)")

    optparser.add_argument("-p", "--plain",
        dest="plain",
        action="store_true",
        help="disable ANSI color etc. formatting")

    optparser.add_argument("-S", "--sup-variants",
        dest="suppress_variants",
        action="store_true",
        help="suppress some variants in output")

    optparser.add_argument("-C", "--sup-common",
        dest="suppress_common",
        action="store_true",
        help="suppress common sections completely")

    optparser.add_argument("-N", "--named",
        dest="named_ptrs",
        action="store_true",
        help="generate symbolic names for raw pointer values")

    optparser.add_argument("-M", "--method-only",
        dest="method_only",
        action="store_true",
        help="output only call names without arguments")

    optparser.add_argument("-I", "--ignore-junk",
        dest="ignore_junk",
        action="store_true",
        help="filter out/ignore junk calls (see below)")

    optparser.add_argument("-w", "--width",
        dest="output_width",
        type=functools.partial(pkk_arg_range, vmin=16, vmax=512), default=defwidth,
        metavar="N",
        help="output width (default: %(default)s)")

    options = optparser.parse_args()

    ### Parse input files
    stack1 = pkk_parse_trace(options.filename1, options, TraceStateData())
    stack2 = pkk_parse_trace(options.filename2, options, TraceStateData())

    ### Perform diffing
    pkk_info("Matching trace sequences ...")
    sequence = difflib.SequenceMatcher(lambda x : x.is_junk, stack1, stack2, autojunk=False)

    pkk_info("Sequencing diff ...")
    opcodes = sequence.get_opcodes()
    if len(opcodes) == 1 and opcodes[0][0] == "equal":
        print("The files are identical.")
        sys.exit(0)

    ### Redirect output to 'less' if stdout is a tty
    try:
        if redirect:
            outpipe = subprocess.Popen(["less", "-S", "-R"], stdin=subprocess.PIPE, encoding="utf8")

        ### Output results
        pkk_info("Outputting diff ...")
        colwidth = int((options.output_width - 3) / 2)
        colfmt   = "{}{:"+ str(colwidth) +"s}{} {}{}{} {}{:"+ str(colwidth) +"s}{}"

        printer = PKKPrettyPrinter(options)

        prevtag = ""
        for tag, start1, end1, start2, end2 in opcodes:
            if tag == "equal":
                show_args = False
                if options.suppress_common:
                    if tag != prevtag:
                        pkk_output(outpipe, "[...]")
                    continue

                sep = "|"
                ansi1 = ansi2 = ansiend = ""
                show_args = False
            elif tag == "insert":
                sep = "+"
                ansi1 = ""
                ansi2 = PKK_ANSI_ESC + PKK_ANSI_GREEN
                show_args = True
            elif tag == "delete":
                sep = "-"
                ansi1 = PKK_ANSI_ESC + PKK_ANSI_RED
                ansi2 = ""
                show_args = True
            elif tag == "replace":
                sep = ">"
                ansi1 = ansi2 = PKK_ANSI_ESC + PKK_ANSI_BOLD
                show_args = True
            else:
                pkk_fatal(f"Internal error, unsupported difflib.SequenceMatcher operation '{tag}'.")

            # No ANSI, please
            if options.plain:
                ansi1 = ansisep = ansi2 = ansiend = ""
            else:
                ansisep = PKK_ANSI_ESC + PKK_ANSI_PURPLE
                ansiend = PKK_ANSI_ESC + PKK_ANSI_NORMAL


            # Print out the block
            ncall1 = start1
            ncall2 = start2
            last1 = last2 = False
            while True:
                # Get line data
                if ncall1 < end1:
                    if not options.ignore_junk or not stack1[ncall1].is_junk:
                        printer.entry_start(show_args)
                        stack1[ncall1].visit(printer)
                        data1 = printer.entry_get()
                    else:
                        data1 = []
                    ncall1 += 1
                else:
                    data1 = []
                    last1 = True

                if ncall2 < end2:
                    if not options.ignore_junk or not stack2[ncall2].is_junk:
                        printer.entry_start(show_args)
                        stack2[ncall2].visit(printer)
                        data2 = printer.entry_get()
                    else:
                        data2 = []
                    ncall2 += 1
                else:
                    data2 = []
                    last2 = True

                # Check if we are at last call of both
                if last1 and last2:
                    break

                nline = 0
                while nline < len(data1) or nline < len(data2):
                    # Determine line start indentation
                    if nline > 0:
                        if options.suppress_variants:
                            indent = " "*8
                        else:
                            indent = " "*12
                    else:
                        indent = ""

                    line1 = pkk_get_line(data1, nline)
                    line2 = pkk_get_line(data2, nline)

                    # Highlight differing lines if not plain
                    if not options.plain and line1 != line2:
                        if tag == "insert" or tag == "delete":
                            ansi1 = ansi1 + PKK_ANSI_ESC + PKK_ANSI_BOLD
                        elif tag == "replace":
                            ansi1 = ansi2 = ansi1 + PKK_ANSI_ESC + PKK_ANSI_YELLOW

                    # Output line
                    pkk_output(outpipe, colfmt.format(
                        ansi1, pkk_format_line(line1, indent, colwidth), ansiend,
                        ansisep, sep, ansiend,
                        ansi2, pkk_format_line(line2, indent, colwidth), ansiend).
                        rstrip())

                    nline += 1

            if tag == "equal" and options.suppress_common:
                pkk_output(outpipe, "[...]")

            prevtag = tag

    except Exception as e:
        pkk_fatal(str(e))

    if outpipe is not None:
        outpipe.communicate()
