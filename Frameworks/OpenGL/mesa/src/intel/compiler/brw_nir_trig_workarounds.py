#
# Copyright (C) 2016 Intel Corporation
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

# Prior to Kaby Lake, The SIN and COS instructions on Intel hardware can
# produce values slightly outside of the [-1.0, 1.0] range for a small set of
# values.  Obviously, this can break everyone's expectations about trig
# functions.  This appears to be fixed in Kaby Lake.
#
# According to an internal presentation, the COS instruction can produce
# a value up to 1.000027 for inputs in the range (0.08296, 0.09888).  One
# suggested workaround is to multiply by 0.99997, scaling down the
# amplitude slightly.  Apparently this also minimizes the error function,
# reducing the maximum error from 0.00006 to about 0.00003.

import argparse
import sys
from math import pi

TRIG_WORKAROUNDS = [
    (('fsin', 'x(is_not_const)'), ('fmul', ('fsin', 'x'), 0.99997)),
    (('fcos', 'x(is_not_const)'), ('fmul', ('fcos', 'x'), 0.99997)),
]

LIMIT_TRIG_INPUT_RANGE_WORKAROUND = [
    (('fsin', 'x(is_not_const)'), ('fsin', ('fmod', 'x', 2.0 * pi))),
    (('fcos', 'x(is_not_const)'), ('fcos', ('fmod', 'x', 2.0 * pi))),
]

def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('-p', '--import-path', required=True)
    args = parser.parse_args()
    sys.path.insert(0, args.import_path)
    run()


def run():
    import nir_algebraic  # pylint: disable=import-error

    print('#include "brw_nir.h"')
    print(nir_algebraic.AlgebraicPass("brw_nir_apply_trig_workarounds",
                                      TRIG_WORKAROUNDS).render())
    print(nir_algebraic.AlgebraicPass("brw_nir_limit_trig_input_range_workaround",
                                      LIMIT_TRIG_INPUT_RANGE_WORKAROUND).render())


if __name__ == '__main__':
    main()
