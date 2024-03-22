# Copyright 2022 Alyssa Rosenzweig
# Copyright 2021 Collabora, Ltd.
# Copyright 2016 Intel Corporation
# SPDX-License-Identifier: MIT

import argparse
import sys
import math

a = 'a'
b = 'b'
c = 'c'
d = 'd'
e = 'e'

lower_sm5_shift = []

# Our shifts differ from SM5 for the upper bits. Mask to match the NIR
# behaviour. Because this happens as a late lowering, NIR won't optimize the
# masking back out (that happens in the main nir_opt_algebraic).
for s in [8, 16, 32, 64]:
    for shift in ["ishl", "ishr", "ushr"]:
        lower_sm5_shift += [((shift, f'a@{s}', b),
                             (shift, a, ('iand', b, s - 1)))]

lower_pack = [
    (('pack_half_2x16_split', a, b),
     ('pack_32_2x16_split', ('f2f16', a), ('f2f16', b))),

    # We don't have 8-bit ALU, so we need to lower this. But if we lower it like
    # this, we can at least coalesce the pack_32_2x16_split and only pay the
    # cost of the iors and ishl. (u2u16 of 8-bit is assumed free.)
    (('pack_32_4x8_split', a, b, c, d),
     ('pack_32_2x16_split', ('ior', ('u2u16', a), ('ishl', ('u2u16', b), 8)),
                            ('ior', ('u2u16', c), ('ishl', ('u2u16', d), 8)))),

    (('unpack_half_2x16_split_x', a), ('f2f32', ('unpack_32_2x16_split_x', a))),
    (('unpack_half_2x16_split_y', a), ('f2f32', ('unpack_32_2x16_split_y', a))),

    (('extract_u16', 'a@32', 0), ('u2u32', ('unpack_32_2x16_split_x', a))),
    (('extract_u16', 'a@32', 1), ('u2u32', ('unpack_32_2x16_split_y', a))),
    (('extract_i16', 'a@32', 0), ('i2i32', ('unpack_32_2x16_split_x', a))),
    (('extract_i16', 'a@32', 1), ('i2i32', ('unpack_32_2x16_split_y', a))),

    # For optimizing extract->convert sequences for unpack/pack norm
    (('u2f32', ('u2u32', a)), ('u2f32', a)),
    (('i2f32', ('i2i32', a)), ('i2f32', a)),

    # Chew through some 8-bit before the backend has to deal with it
    (('f2u8', a), ('u2u8', ('f2u16', a))),
    (('f2i8', a), ('i2i8', ('f2i16', a))),

    # Based on the VIR lowering
    (('f2f16_rtz', 'a@32'),
     ('bcsel', ('flt', ('fabs', a), ('fabs', ('f2f32', ('f2f16_rtne', a)))),
      ('isub', ('f2f16_rtne', a), 1), ('f2f16_rtne', a))),

    # These are based on the lowerings from nir_opt_algebraic, but conditioned
    # on the number of bits not being constant. If the bit count is constant
    # (the happy path) we can use our native instruction instead.
    (('ibitfield_extract', 'value', 'offset', 'bits(is_not_const)'),
     ('bcsel', ('ieq', 0, 'bits'),
      0,
      ('ishr',
       ('ishl', 'value', ('isub', ('isub', 32, 'bits'), 'offset')),
       ('isub', 32, 'bits')))),

    (('ubitfield_extract', 'value', 'offset', 'bits(is_not_const)'),
     ('iand',
      ('ushr', 'value', 'offset'),
      ('bcsel', ('ieq', 'bits', 32),
       0xffffffff,
       ('isub', ('ishl', 1, 'bits'), 1)))),

    # Codegen depends on this trivial case being optimized out.
    (('ubitfield_extract', 'value', 'offset', 0), 0),
    (('ibitfield_extract', 'value', 'offset', 0), 0),

    # At this point, bitfield extracts are constant. We can only do constant
    # unsigned bitfield extract, so lower signed to unsigned + sign extend.
    (('ibitfield_extract', a, b, '#bits'),
     ('ishr', ('ishl', ('ubitfield_extract', a, b, 'bits'), ('isub', 32, 'bits')),
      ('isub', 32, 'bits'))),
]

fuse_extr = []
for start in range(32):
    fuse_extr.extend([
        (('ior', ('ushr', 'a@32', start), ('ishl', 'b@32', 32 - start)),
         ('extr_agx', a, b, start, 0)),
    ])

fuse_ubfe = []
for bits in range(1, 32):
    fuse_ubfe.extend([
        (('iand', ('ushr', 'a@32', b), (1 << bits) - 1),
         ('ubitfield_extract', a, b, bits))
    ])

# (x * y) + s = (x * y) + (s << 0)
def imad(x, y, z):
    return ('imadshl_agx', x, y, z, 0)

# (x * y) - s = (x * y) - (s << 0)
def imsub(x, y, z):
    return ('imsubshl_agx', x, y, z, 0)

# x + (y << s) = (x * 1) + (y << s)
def iaddshl(x, y, s):
    return ('imadshl_agx', x, 1, y, s)

# x - (y << s) = (x * 1) - (y << s)
def isubshl(x, y, s):
    return ('imsubshl_agx', x, 1, y, s)

fuse_imad = [
    # Reassociate imul+iadd chain in order to fuse imads. This pattern comes up
    # in compute shader lowering.
    (('iadd', ('iadd(is_used_once)', ('imul(is_used_once)', a, b),
              ('imul(is_used_once)', c, d)), e),
     imad(a, b, imad(c, d, e))),

    # Fuse regular imad
    (('iadd', ('imul(is_used_once)', a, b), c), imad(a, b, c)),
    (('isub', ('imul(is_used_once)', a, b), c), imsub(a, b, c)),
]

for s in range(1, 5):
    fuse_imad += [
        # Definitions
        (('iadd', a, ('ishl(is_used_once)', b, s)), iaddshl(a, b, s)),
        (('isub', a, ('ishl(is_used_once)', b, s)), isubshl(a, b, s)),

        # ineg(x) is 0 - x
        (('ineg', ('ishl(is_used_once)', b, s)), isubshl(0, b, s)),

        # Definitions
        (imad(a, b, ('ishl(is_used_once)', c, s)), ('imadshl_agx', a, b, c, s)),
        (imsub(a, b, ('ishl(is_used_once)', c, s)), ('imsubshl_agx', a, b, c, s)),

        # a + (a << s) = a + a * (1 << s) = a * (1 + (1 << s))
        (('imul', a, 1 + (1 << s)), iaddshl(a, a, s)),

        # a - (a << s) = a - a * (1 << s) = a * (1 - (1 << s))
        (('imul', a, 1 - (1 << s)), isubshl(a, a, s)),

        # a - (a << s) = a * (1 - (1 << s)) = -(a * (1 << s) - 1)
        (('ineg', ('imul(is_used_once)', a, (1 << s) - 1)), isubshl(a, a, s)),

        # iadd is SCIB, general shfit is IC (slower)
        (('ishl', a, s), iaddshl(0, a, s)),
    ]

# Discard lowering generates this pattern, clean it up
ixor_bcsel = [
   (('ixor', ('bcsel', a, '#b', '#c'), '#d'),
    ('bcsel', a, ('ixor', b, d), ('ixor', c, d))),
]

def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('-p', '--import-path', required=True)
    args = parser.parse_args()
    sys.path.insert(0, args.import_path)
    run()

def run():
    import nir_algebraic  # pylint: disable=import-error

    print('#include "agx_nir.h"')

    print(nir_algebraic.AlgebraicPass("agx_nir_lower_algebraic_late",
                                      lower_sm5_shift + lower_pack).render())
    print(nir_algebraic.AlgebraicPass("agx_nir_fuse_algebraic_late",
                                      fuse_extr + fuse_ubfe + fuse_imad).render())
    print(nir_algebraic.AlgebraicPass("agx_nir_opt_ixor_bcsel",
                                      ixor_bcsel).render())


if __name__ == '__main__':
    main()
