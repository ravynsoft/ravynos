#
# Copyright (C) 2019 Vasily Khoruzhick <anarsoul@gmail.com>
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
import sys
from math import pi

# Utgard scales fsin/fcos arguments by 2*pi.
# Pass must be run only once, after the main loop

scale_trig = [
        (('fsin', 'a'), ('fsin', ('fmul', 'a', 1.0 / (2.0 * pi)))),
        (('fcos', 'a'), ('fcos', ('fmul', 'a', 1.0 / (2.0 * pi)))),
]

# GP has fsign op, so we can use cheaper lowering than one in generic opt_algebraic
lower_ftrunc = [
        (('ftrunc', 'a'), ('fmul', ('fsign', 'a'), ('ffloor', ('fmax', 'a', ('fneg', 'a')))))
]

def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('-p', '--import-path', required=True)
    args = parser.parse_args()
    sys.path.insert(0, args.import_path)
    run()


def run():
    import nir_algebraic  # pylint: disable=import-error

    print('#include "ir/lima_ir.h"')

    print(nir_algebraic.AlgebraicPass("lima_nir_scale_trig",
                                      scale_trig).render())
    print(nir_algebraic.AlgebraicPass("lima_nir_lower_ftrunc",
                                      lower_ftrunc).render())

if __name__ == '__main__':
    main()
