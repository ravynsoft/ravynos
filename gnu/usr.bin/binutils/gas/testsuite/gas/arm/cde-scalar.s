.syntax unified
# Extra tests everywhere:
# Ensure that setting the register to something in r[1-12] works.

# cx1{a} Has arguments in the following form
# 111a111000iiiiiidddd0pppi0iiiiii
#
# Variants to test:
# - Base (everything we can set to zero)
# - immediates that set each set of `i` to ones in turn.
#   (imm = op1:op2:op3  , which is each group of `i` from left to write
#   concatenated)
# - Each register 9 (0b1001), APSR_nzcv, or all zeros
# - Coprocessor num set to 7
# - Everything again with the `a` version (to double check parsing).
# - Accumulator versions without optional argument (to check parsing)
#
# IT blocks:
#  Non-accumulator versions are UNPREDICTABLE in IT blocks.
#  Accumulator versions are allowed in IT blocks.

# cx1{a} extra tests.
# Arm conditional
cx1 p0, r0, #0
cx1 p0, r0, #8064
cx1 p0, r0, #64
cx1 p0, r0, #63
cx1 p7, r0, #0
cx1 p0, APSR_nzcv, #0
cx1 p0, r9, #0
cx1a p0, r0, #0
cx1a p0, r0, #8064
cx1a p0, r0, #64
cx1a p0, r0, #63
cx1a p7, r0, #0
cx1a p0, APSR_nzcv, #0
cx1a p0, r9, #0

it ne
cx1ane p0, r0, #0

# cx1d{a} encoding of following form:
# 111a111000iiiiiidddd0pppi1iiiiii
#
# Variants to test:
# - Base (everything we can set to zero)
# - immediates that set each set of `i` to ones in turn.
#   (imm = op1:op2:op3  , which is each group of `i` from left to write
#   concatenated)
# - Destination register 10 (0b1010) or all zeros
# - Coprocessor num set to 7
# - Everything again with the `a` version (to double check parsing).
# - Accumulator versions without optional argument (to check parsing)
cx1d p0, r0, r1, #0
cx1d p0, r0, r1, #8064
cx1d p0, r0, r1, #64
cx1d p0, r0, r1, #63
cx1d p7, r0, r1, #0
cx1d p0, r10, r11, #0
cx1da p0, r0, r1, #0
cx1da p0, r0, r1, #8064
cx1da p0, r0, r1, #64
cx1da p0, r0, r1, #63
cx1da p7, r0, r1, #0
cx1da p0, r10, r11, #0

it ne
cx1dane p0, r0, r1, #0


# cx2{a} Has arguments of the following form:
# 111a111001iinnnndddd0pppi0iiiiii
#
# Variants to test:
# - Base (everything we can set to zero)
# - immediates that set each set of `i` to ones in turn.
#   (imm = op1:op2:op3  , which is each group of `i` from left to write
#   concatenated)
# - Each register 9 (0b1001), APSR_nzcv, or all zeros
# - Coprocessor num set to 7
# - Everything again with the `a` version (to double check parsing).
# - Accumulator versions without optional argument (to check parsing)
cx2 p0, r0, r0, #0
cx2 p0, r0, r0, #384
cx2 p0, r0, r0, #64
cx2 p0, r0, r0, #63
cx2 p7, r0, r0, #0
cx2 p0, APSR_nzcv, r0, #0
cx2 p0, r9, r0, #0
cx2 p0, r0, APSR_nzcv, #0
cx2 p0, r0, r9, #0
cx2a p0, r0, r0, #0
cx2a p0, r0, r0, #384
cx2a p0, r0, r0, #64
cx2a p0, r0, r0, #63
cx2a p7, r0, r0, #0
cx2a p0, APSR_nzcv, r0, #0
cx2a p0, r9, r0, #0
cx2a p0, r0, APSR_nzcv, #0
cx2a p0, r0, r9, #0

it ne
cx2ane p0, r0, r0, #0

# cx2d{a} encoding has following form:
# 111a111001iinnnndddd0pppi1iiiiii
#
# - Base (everything we can set to zero)
# - immediates that set each set of `i` to ones in turn.
#   (imm = op1:op2:op3  , which is each group of `i` from left to write
#   concatenated)
# - Destination register 10 (0b1010) or all zeros
# - Coprocessor num set to 7
# - Everything again with the `a` version (to double check parsing).
# - Accumulator versions without optional argument (to check parsing)
cx2d p0, r0, r1, r0, #0
cx2d p0, r0, r1, r0, #384
cx2d p0, r0, r1, r0, #64
cx2d p0, r0, r1, r0, #63
cx2d p7, r0, r1, r0, #0
cx2d p0, r10, r11, r0, #0
cx2d p0, r0, r1, APSR_nzcv, #0
cx2d p0, r0, r1, r9, #0
cx2da p0, r0, r1, r0, #0
cx2da p0, r0, r1, r0, #384
cx2da p0, r0, r1, r0, #64
cx2da p0, r0, r1, r0, #63
cx2da p7, r0, r1, r0, #0
cx2da p0, r10, r11, r0, #0
cx2da p0, r0, r1, APSR_nzcv, #0
cx2da p0, r0, r1, r9, #0

# cx3{a} Has arguments in the following form:
# 111a11101iiinnnnmmmm0pppi0iidddd
#
# Variants to test:
# - immediates that set each set of `i` to ones in turn.
#   (imm = op1:op2:op3  , which is each group of `i` from left to write
# - Base (everything we can set to zero)
# - immediates that set each set of `i` to ones in turn.
#   (imm = op1:op2:op3  , which is each group of `i` from left to write
#   concatenated)
# - Each register 9 (0b1001), APSR_nzcv, or all zeros
# - Coprocessor num set to 7
# - Everything again with the `a` version (to double check parsing).
# - Accumulator versions without optional argument (to check parsing)
cx3 p0, r0, r0, r0, #0
cx3 p0, r0, r0, r0, #56
cx3 p0, r0, r0, r0, #4
cx3 p0, r0, r0, r0, #3
cx3 p7, r0, r0, r0, #0
cx3 p0, APSR_nzcv, r0, r0, #0
cx3 p0, r9, r0, r0, #0
cx3 p0, r0, APSR_nzcv, r0, #0
cx3 p0, r0, r9, r0, #0
cx3 p0, r0, r0, APSR_nzcv, #0
cx3 p0, r0, r0, r9, #0
cx3a p0, r0, r0, r0, #0
cx3a p0, r0, r0, r0, #56
cx3a p0, r0, r0, r0, #4
cx3a p0, r0, r0, r0, #3
cx3a p7, r0, r0, r0, #0
cx3a p0, APSR_nzcv, r0, r0, #0
cx3a p0, r9, r0, r0, #0
cx3a p0, r0, APSR_nzcv, r0, #0
cx3a p0, r0, r9, r0, #0
cx3a p0, r0, r0, APSR_nzcv, #0
cx3a p0, r0, r0, r9, #0

it ne
cx3ane p0, r0, r0, r0, #0

# cx3d{a} encoding has following form:
# 111a11101iiinnnnmmmm0pppi1iidddd
#
# Variants to test:
# - Toggle 'a'
# - immediates that set each set of `i` to ones in turn.
#   (imm = op1:op2:op3  , which is each group of `i` from left to write
#   concatenated)
# - Destination register 10 (0b1010) or all zeros
# - Source register 9 (0b1001), APSR_nzcv, or all zeros

# No longer allows APSR_nzcv in destination register
cx3d p0, r0, r1, r0, r0, #0
cx3d p0, r0, r1, r0, r0, #56
cx3d p0, r0, r1, r0, r0, #4
cx3d p0, r0, r1, r0, r0, #3
cx3d p7, r0, r1, r0, r0, #0
cx3d p0, r10, r11, r0, r0, #0
cx3d p0, r0, r1, APSR_nzcv, r0, #0
cx3d p0, r0, r1, r9, r0, #0
cx3d p0, r0, r1, r0, APSR_nzcv, #0
cx3d p0, r0, r1, r0, r9, #0
cx3da p0, r0, r1, r0, r0, #0
cx3da p0, r0, r1, r0, r0, #56
cx3da p0, r0, r1, r0, r0, #4
cx3da p0, r0, r1, r0, r0, #3
cx3da p7, r0, r1, r0, r0, #0
cx3da p0, r10, r11, r0, r0, #0
cx3da p0, r0, r1, APSR_nzcv, r0, #0
cx3da p0, r0, r1, r9, r0, #0
cx3da p0, r0, r1, r0, APSR_nzcv, #0
cx3da p0, r0, r1, r0, r9, #0


