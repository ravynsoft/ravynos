#
# Copyright (C) 2019 Vasily Khoruzhick <anarsoul@gmail.com>
# Copyright (C) 2021 Pavel Ondračka
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

# Convenience variables
a = 'a'
b = 'b'
c = 'c'
d = 'd'
e = 'e'

# Transform input to range [-PI, PI]:
#
# y = frac(x / 2PI + 0.5) * 2PI - PI
#
transform_trig_input_vs_r500 = [
        (('fsin', 'a'), ('fsin', ('fadd', ('fmul', ('ffract', ('fadd', ('fmul', 'a', 1 / (2 * pi)) , 0.5)), 2 * pi), -pi))),
        (('fcos', 'a'), ('fcos', ('fadd', ('fmul', ('ffract', ('fadd', ('fmul', 'a', 1 / (2 * pi)) , 0.5)), 2 * pi), -pi))),
]

# Transform input to range [-PI, PI]:
#
# y = frac(x / 2PI)
#
transform_trig_input_fs_r500 = [
        (('fsin', 'a'), ('fsin', ('ffract', ('fmul', 'a', 1 / (2 * pi))))),
        (('fcos', 'a'), ('fcos', ('ffract', ('fmul', 'a', 1 / (2 * pi))))),
]

# The is a pattern produced by wined3d for A0 register load.
# The specific pattern wined3d emits looks like this
# A0.x = (int(floor(abs(R0.x) + 0.5) * sign(R0.x)));
# however we lower both sign and floor so here we check for the already lowered
# sequence.
r300_nir_fuse_fround_d3d9 = [
        (('fmul', ('fadd', ('fadd', ('fabs', 'a') , 0.5),
                           ('fneg', ('ffract', ('fadd', ('fabs', 'a') , 0.5)))),
                  ('fadd', ('b2f', ('!flt', 0.0, 'a')),
                           ('fneg', ('b2f', ('!flt', 'a', 0.0))))),
         ('fround_even', 'a'))
]

# Here are some specific optimizations for code reordering such that the backend
# has easier task of recognizing output modifiers and presubtract patterns.
r300_nir_prepare_presubtract = [
        # Backend can only recognize 1 - x pattern.
        (('fadd', ('fneg', a), 1.0), ('fadd', 1.0, ('fneg', a))),
        (('fadd', a, -1.0), ('fneg', ('fadd', 1.0, ('fneg', a)))),
        (('fadd', -1.0, a), ('fneg', ('fadd', 1.0, ('fneg', a)))),
        # Bias presubtract 1 - 2 * x expects MAD -a 2.0 1.0 form.
        (('ffma', 2.0, ('fneg', a), 1.0), ('ffma', ('fneg', a), 2.0, 1.0)),
        (('ffma', a, -2.0, 1.0), ('fneg', ('ffma', ('fneg', a), 2.0, 1.0))),
        (('ffma', -2.0, a, 1.0), ('fneg', ('ffma', ('fneg', a), 2.0, 1.0))),
        (('ffma', 2.0, a, -1.0), ('fneg', ('ffma', ('fneg', a), 2.0, 1.0))),
        (('ffma', a, 2.0, -1.0), ('fneg', ('ffma', ('fneg', a), 2.0, 1.0))),
        # x * 2 can be usually folded into output modifier for the previous
        # instruction, but that only works if x is a temporary. If it is input or
        # constant just convert it to add instead.
        (('fmul', 'a(is_ubo_or_input)', 2.0), ('fadd', a, a)),
]

for multiplier in [2.0, 4.0, 8.0, 16.0, 0.5, 0.25, 0.125, 0.0625]:
    r300_nir_prepare_presubtract.extend([
        (('fmul', a, ('fmul(is_used_once)', 'b(is_ubo_or_input)', multiplier)), ('fmul', multiplier, ('fmul', a, b))),
])

# Previous prepare_presubtract pass can sometimes produce double fneg patterns.
# The backend copy propagate could handle it, but the nir to tgsi translation
# does not and blows up. Just run a simple pass to clean it up.
r300_nir_clean_double_fneg = [
        (('fneg', ('fneg', a)), a)
]

# This is very late flrp lowering to clean up after bcsel->fcsel->flrp.
r300_nir_lower_flrp = [
        (('flrp', a, b, c), ('ffma', b, c, ('ffma', ('fneg', a), c, a)))
]

# Lower fcsel_ge from ftrunc on r300
r300_nir_lower_fcsel_r300 = [
        (('fcsel_ge', a, b, c), ('flrp', c, b, ('sge', a, 0.0)))
]

r300_nir_post_integer_lowering = [
        # If ffloor result is used only for indirect constant load, we can get rid of it
        # completelly as ntt emits ARL by default which already does the flooring.
        # This actually checks for the lowered ffloor(a) = a - ffract(a) patterns.
        (('fadd(is_only_used_by_load_ubo_vec4)', a, ('fneg', ('ffract', a))), a),
        # This is a D3D9 pattern from Wine when shader wants ffloor instead of fround on register load.
        (('fround_even(is_only_used_by_load_ubo_vec4)', ('fadd', a, ('fneg', ('ffract', a)))), a),
        # Lower ftrunc
        (('ftrunc', 'a@32'), ('fcsel_ge', a, ('fadd', ('fabs', a), ('fneg', ('ffract', ('fabs', a)))),
                                     ('fneg', ('fadd', ('fabs', a), ('fneg', ('ffract', ('fabs', a)))))))
]

def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('-p', '--import-path', required=True)
    parser.add_argument('output')
    args = parser.parse_args()
    sys.path.insert(0, args.import_path)

    import nir_algebraic  # pylint: disable=import-error
    ignore_exact = nir_algebraic.ignore_exact

    r300_nir_lower_bool_to_float = [
        (('bcsel@32(is_only_used_as_float)', ignore_exact('feq', 'a@32', 'b@32'), c, d),
             ('fadd', ('fmul', c, ('seq', a, b)), ('fsub', d, ('fmul', d, ('seq', a, b)))),
             "!options->has_fused_comp_and_csel"),
        (('bcsel@32(is_only_used_as_float)', ignore_exact('fneu', 'a@32', 'b@32'), c, d),
             ('fadd', ('fmul', c, ('sne', a, b)), ('fsub', d, ('fmul', d, ('sne', a, b)))),
          "!options->has_fused_comp_and_csel"),
        (('bcsel@32(is_only_used_as_float)', ignore_exact('flt', 'a@32', 'b@32'), c, d),
             ('fadd', ('fmul', c, ('slt', a, b)), ('fsub', d, ('fmul', d, ('slt', a, b)))),
          "!options->has_fused_comp_and_csel"),
        (('bcsel@32(is_only_used_as_float)', ignore_exact('fge', 'a@32', 'b@32'), c, d),
             ('fadd', ('fmul', c, ('sge', a, b)), ('fsub', d, ('fmul', d, ('sge', a, b)))),
          "!options->has_fused_comp_and_csel"),
        (('bcsel@32(is_only_used_as_float)', ('feq', 'a@32', 'b@32'), c, d),
             ('fcsel', ('seq', a, b), c, d), "options->has_fused_comp_and_csel"),
        (('bcsel@32(is_only_used_as_float)', ('fneu', 'a@32', 'b@32'), c, d),
             ('fcsel', ('sne', a, b), c, d), "options->has_fused_comp_and_csel"),
        (('bcsel@32(is_only_used_as_float)', ('flt', 'a@32', 'b@32'), c, d),
             ('fcsel', ('slt', a, b), c, d), "options->has_fused_comp_and_csel"),
        (('bcsel@32(is_only_used_as_float)', ('fge', 'a@32', 'b@32'), c, d),
             ('fcsel', ('sge', a, b), c, d), "options->has_fused_comp_and_csel"),
]

    with open(args.output, 'w') as f:
        f.write('#include "compiler/r300_nir.h"')

        f.write(nir_algebraic.AlgebraicPass("r300_transform_vs_trig_input",
                                            transform_trig_input_vs_r500).render())

        f.write(nir_algebraic.AlgebraicPass("r300_transform_fs_trig_input",
                                            transform_trig_input_fs_r500).render())

        f.write(nir_algebraic.AlgebraicPass("r300_nir_fuse_fround_d3d9",
                                            r300_nir_fuse_fround_d3d9).render())

        f.write(nir_algebraic.AlgebraicPass("r300_nir_lower_bool_to_float",
                                            r300_nir_lower_bool_to_float).render())

        f.write(nir_algebraic.AlgebraicPass("r300_nir_prepare_presubtract",
                                            r300_nir_prepare_presubtract).render())

        f.write(nir_algebraic.AlgebraicPass("r300_nir_clean_double_fneg",
                                            r300_nir_clean_double_fneg).render())

        f.write(nir_algebraic.AlgebraicPass("r300_nir_post_integer_lowering",
                                            r300_nir_post_integer_lowering).render())

        f.write(nir_algebraic.AlgebraicPass("r300_nir_lower_flrp",
                                            r300_nir_lower_flrp).render())

        f.write(nir_algebraic.AlgebraicPass("r300_nir_lower_fcsel_r300",
                                            r300_nir_lower_fcsel_r300).render())

if __name__ == '__main__':
    main()
