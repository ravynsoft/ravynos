#
# Copyright (C) 2018 Valve Corporation
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

import unittest

import sys
import os
sys.path.insert(1, os.path.join(sys.path[0], '..'))

from nir_algebraic import SearchAndReplace, AlgebraicPass

# These tests check that the bitsize validator correctly rejects various
# different kinds of malformed expressions, and documents what the error
# message looks like.

a = 'a'
b = 'b'
c = 'c'

class ValidatorTests(unittest.TestCase):
    pattern = ()
    message = ''

    algebraic_pass = AlgebraicPass("test", [])

    def common(self, pattern, message):
        with self.assertRaises(AssertionError) as context:
            SearchAndReplace(pattern, self.algebraic_pass)

        self.assertEqual(message, str(context.exception))

    def test_wrong_src_count(self):
        self.common((('iadd', a), ('fadd', a, a)),
            "Expression ('iadd', 'a') has 1 sources, expected 2")

    def test_var_bitsize(self):
        self.common((('iadd', 'a@32', 'a@64'), ('fadd', a, a)),
            "Variable a has conflicting bit size requirements: " \
            "it must have bit size 32 and 64")

    def test_var_bitsize_2(self):
        self.common((('iadd', a, 'a@32'), ('fadd', 'a@64', a)),
            "Variable a has conflicting bit size requirements: " \
            "it must have bit size 32 and 64")

    def test_search_src_bitsize(self):
        self.common((('iadd', 'a@32', 'b@64'), ('fadd', a, b)),
            "Source a@32 of ('iadd', 'a@32', 'b@64') must have bit size 32, " \
            "while source b@64 must have incompatible bit size 64")

    def test_replace_src_bitsize(self):
        self.common((('iadd', a, ('b2i', b)), ('iadd', a, b)),
            "Sources a (bit size of a) and b (bit size of b) " \
            "of ('iadd', 'a', 'b') may not have the same bit size " \
            "when building the replacement expression.")

    def test_search_src_bitsize_fixed(self):
        self.common((('ishl', a, 'b@64'), ('ishl', a, b)),
            "b@64 must have 64 bits, but as a source of nir_op_ishl " \
            "it must have 32 bits")

    def test_replace_src_bitsize_fixed(self):
        self.common((('iadd', a, b), ('ishl', a, b)),
            "b has the bit size of b, but as a source of nir_op_ishl " \
            "it must have 32 bits, which may not be the same")

    def test_search_dst_bitsize(self):
        self.common((('iadd@32', 'a@64', b), ('iadd', a, b)),
            "('iadd@32', 'a@64', 'b') must have the bit size of 32, " \
            "while its source a@64 must have incompatible bit size 64")

    def test_replace_dst_bitsize(self):
        self.common((('iadd', a, b), ('iadd@32', a, b)),
            "('iadd@32', 'a', 'b') must have 32 bits, but its source a " \
            "(bit size of b) may not have that bit size when building " \
            "the replacement.")

    def test_search_dst_bitsize_fixed(self):
        self.common((('ufind_msb@64', a), ('ineg', a)),
            "('ufind_msb@64', 'a') must have 64 bits, "\
            "but as a destination of nir_op_ufind_msb it must have 32 bits")

    def test_replace_dst_bitsize_fixed(self):
        self.common((('ineg', 'a@64'), ('ufind_msb@64', a)),
            "('ufind_msb@64', 'a') must have 64 bits, " \
            "but as a destination of nir_op_ufind_msb it must have 32 bits")

    def test_ambiguous_bitsize(self):
        self.common((('ineg', 'a@32'), ('i2b', ('b2i', a))),
            "Ambiguous bit size for replacement value ('b2i', 'a'): it "\
            "cannot be deduced from a variable, a fixed bit size somewhere, "
            "or the search expression.")

    def test_search_replace_mismatch(self):
        self.common((('b2i', ('i2b', a)), a),
            "The search expression bit size ('b2i', ('i2b', 'a')) and " \
            "replace expression bit size a may not be the same")

unittest.main()
