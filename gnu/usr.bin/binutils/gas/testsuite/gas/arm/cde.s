.syntax unified

.include "cde-scalar.s"

# vcx1{a} encoding has the following form
# 111a110i0d10iiiidddd0pppi1iiiiii (vector form)

# 111a110s0d10iiiidddd0pppi0iiiiii (S/D register form)
#
# Variants to test:
# - immediates that set each set of `i` to ones in turn.
# - each register set to something non-zero
#   (where each block of register sets is set to all-ones if possible)
# - coprocessor set to 7

# vcx2{a} encoding has the following form
# 111a110i0d11iiiidddd0pppi1mimmmm (vector form)
# 111a110s0d11iiiidddd0pppi0mimmmm (S/D register form)
#
# Variants to test:
# - immediates that set each set of `i` to ones in turn.
# - each register set to something non-zero
#   (where each block of register sets is set to all-ones if possible)
# - coprocessor set to 7

# vcx3{a} encoding has the following form
# 111a110i1diinnnndddd0pppn1mimmmm (vector form)
# 111a110s1diinnnndddd0pppn0mimmmm (S/D register form)
#
# Variants to test:
# - immediates that set each set of `i` to ones in turn.
# - each register set to something non-zero
#   (where each block of register sets is set to all-ones if possible)
# - coprocessor set to 7
.include "cde-mve.s"
.include "cde-mve-or-neon.s"
