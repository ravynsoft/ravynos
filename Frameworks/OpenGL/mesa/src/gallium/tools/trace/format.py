#!/usr/bin/env python3
##########################################################################
#
# Copyright 2008 VMware, Inc.
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


class Formatter:
    '''Plain formatter'''

    def __init__(self, stream):
        self.stream = stream

    def text(self, text):
        self.stream.write(text)

    def newline(self):
        self.text('\n')

    def function(self, name):
        self.text(name)

    def variable(self, name):
        self.text(name)

    def literal(self, value):
        self.text(str(value))

    def address(self, addr):
        self.text(str(addr))


class AnsiFormatter(Formatter):
    '''Formatter for plain-text files which outputs ANSI escape codes. See
    http://en.wikipedia.org/wiki/ANSI_escape_code for more information
    concerning ANSI escape codes.
    '''

    _csi = '\33['

    _normal = '0m'
    _bold = '1m'
    _italic = '3m' # Not widely supported
    _red = '31m'
    _green = '32m'
    _blue = '34m'

    def _escape(self, code):
        self.text(self._csi + code)

    def function(self, name):
        self._escape(self._bold)
        Formatter.function(self, name)
        self._escape(self._normal)

    def variable(self, name):
        Formatter.variable(self, name)

    def literal(self, value):
        self._escape(self._blue)
        Formatter.literal(self, value)
        self._escape(self._normal)

    def address(self, value):
        self._escape(self._green)
        Formatter.address(self, value)
        self._escape(self._normal)


class WindowsConsoleFormatter(Formatter):
    '''Formatter for the Windows Console. See 
    http://code.activestate.com/recipes/496901/ for more information.
    '''

    STD_INPUT_HANDLE  = -10
    STD_OUTPUT_HANDLE = -11
    STD_ERROR_HANDLE  = -12

    FOREGROUND_BLUE      = 0x01
    FOREGROUND_GREEN     = 0x02
    FOREGROUND_RED       = 0x04
    FOREGROUND_INTENSITY = 0x08
    BACKGROUND_BLUE      = 0x10
    BACKGROUND_GREEN     = 0x20
    BACKGROUND_RED       = 0x40
    BACKGROUND_INTENSITY = 0x80

    _normal = FOREGROUND_BLUE | FOREGROUND_GREEN | FOREGROUND_RED
    _bold = FOREGROUND_BLUE | FOREGROUND_GREEN | FOREGROUND_RED | FOREGROUND_INTENSITY
    _italic = FOREGROUND_BLUE | FOREGROUND_GREEN | FOREGROUND_RED
    _red = FOREGROUND_RED | FOREGROUND_INTENSITY
    _green = FOREGROUND_GREEN | FOREGROUND_INTENSITY
    _blue = FOREGROUND_BLUE | FOREGROUND_INTENSITY

    def __init__(self, stream):
        Formatter.__init__(self, stream)

        if stream is sys.stdin:
            nStdHandle = self.STD_INPUT_HANDLE
        elif stream is sys.stdout:
            nStdHandle = self.STD_OUTPUT_HANDLE
        elif stream is sys.stderr:
            nStdHandle = self.STD_ERROR_HANDLE
        else:
            nStdHandle = None

        if nStdHandle:
            import ctypes
            self.handle = ctypes.windll.kernel32.GetStdHandle(nStdHandle)
        else:
            self.handle = None

    def _attribute(self, attr):
        if self.handle:
            import ctypes
            ctypes.windll.kernel32.SetConsoleTextAttribute(self.handle, attr)

    def function(self, name):
        self._attribute(self._bold)
        Formatter.function(self, name)
        self._attribute(self._normal)

    def variable(self, name):
        self._attribute(self._italic)
        Formatter.variable(self, name)
        self._attribute(self._normal)

    def literal(self, value):
        self._attribute(self._blue)
        Formatter.literal(self, value)
        self._attribute(self._normal)

    def address(self, value):
        self._attribute(self._green)
        Formatter.address(self, value)
        self._attribute(self._normal)


def DefaultFormatter(stream):
    if sys.platform in ('linux2', 'linux', 'cygwin'):
        return AnsiFormatter(stream)
    elif sys.platform in ('win32', ):
        return WindowsConsoleFormatter(stream)
    else:
        return Formatter(stream)

