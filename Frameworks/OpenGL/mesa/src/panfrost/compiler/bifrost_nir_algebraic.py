# Copyright (C) 2021 Collabora, Ltd.
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

import argparse
import sys
import math

a = 'a'
b = 'b'
c = 'c'

# In general, bcsel is cheaper than bitwise arithmetic on Mali. On
# Bifrost, we can implement bcsel as either CSEL or MUX to schedule to either
# execution unit. On Valhall, bitwise arithmetic may be on the SFU whereas MUX
# is on the higher throughput CVT unit. We get a zero argument for free relative
# to the bitwise op, which would be LSHIFT_* internally taking a zero anyway.
#
# As such, it's beneficial to reexpress bitwise arithmetic of booleans as bcsel.
opt_bool_bitwise = [
    (('iand', 'a@1', 'b@1'), ('bcsel', a, b, False)),
    (('ior', 'a@1', 'b@1'), ('bcsel', a, a, b)),
    (('iand', 'a@1', ('inot', 'b@1')), ('bcsel', b, 0, a)),
    (('ior', 'a@1', ('inot', 'b@1')), ('bcsel', b, a, True)),
]

algebraic_late = [
    # Canonical form. The scheduler will convert back if it makes sense.
    (('fmul', a, 2.0), ('fadd', a, a)),

    # Fuse Mali-specific clamps
    (('fmin', ('fmax', a, -1.0), 1.0), ('fsat_signed_mali', a)),
    (('fmax', ('fmin', a, 1.0), -1.0), ('fsat_signed_mali', a)),
    (('fmax', a, 0.0), ('fclamp_pos_mali', a)),

    (('fabs', ('fddx', a)), ('fabs', ('fddx_must_abs_mali', a))),
    (('fabs', ('fddy', b)), ('fabs', ('fddy_must_abs_mali', b))),

    (('b32csel', 'b@32', ('iadd', 'a@32', 1), a), ('iadd', a, ('b2i32', b))),

    # We don't have an 8-bit CSEL, so this is the best we can do.
    # Note that we use 8-bit booleans internally to preserve vectorization.
    (('imin', 'a@8', 'b@8'), ('b8csel', ('ilt8', a, b), a, b)),
    (('imax', 'a@8', 'b@8'), ('b8csel', ('ilt8', a, b), b, a)),
    (('umin', 'a@8', 'b@8'), ('b8csel', ('ult8', a, b), a, b)),
    (('umax', 'a@8', 'b@8'), ('b8csel', ('ult8', a, b), b, a)),

    # Floats are at minimum 16-bit, which means when converting to an 8-bit
    # integer, the vectorization changes. So there's no one-shot hardware
    # instruction for f2i8. Instead, lower to two NIR instructions that map
    # directly to the hardware.
    (('f2i8', a), ('i2i8', ('f2i16', a))),
    (('f2u8', a), ('u2u8', ('f2u16', a))),

    # XXX: Duplicate of nir_lower_pack
    (('unpack_64_2x32', a), ('vec2', ('unpack_64_2x32_split_x', a),
                                     ('unpack_64_2x32_split_y', a))),
]

# Handling all combinations of boolean and float sizes for b2f is nontrivial.
# bcsel has the same problem in more generality; lower b2f to bcsel in NIR to
# reuse the efficient implementations of bcsel. This includes special handling
# to allow vectorization in places the hardware does not directly.
#
# Because this lowering must happen late, NIR won't squash inot in
# automatically. Do so explicitly. (The more specific pattern must be first.)
for bsz in [8, 16, 32]:
    for fsz in [16, 32]:
        algebraic_late += [
                ((f'b2f{fsz}', ('inot', f'a@{bsz}')), (f'b{bsz}csel', a, 0.0, 1.0)),
                ((f'b2f{fsz}', f'a@{bsz}'), (f'b{bsz}csel', a, 1.0, 0.0)),
        ]


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('-p', '--import-path', required=True)
    args = parser.parse_args()
    sys.path.insert(0, args.import_path)
    run()


def run():
    import nir_algebraic  # pylint: disable=import-error

    print('#include "bifrost_nir.h"')

    print(nir_algebraic.AlgebraicPass("bifrost_nir_opt_boolean_bitwise",
                                      opt_bool_bitwise).render())
    print(nir_algebraic.AlgebraicPass("bifrost_nir_lower_algebraic_late",
                                      algebraic_late).render())


if __name__ == '__main__':
    main()
