.syntax unified
# cx1{a}
# Immediate out of range.
# Each register out of range.
# r13 => constrained unpredictable
# itblock => constrained unpredictable
# Error given when using coprocessor number not enabled on command line.
# Too many arguments
# Too little arguments
# r15 instead of APSR_nzcv

cx1 p0, r0, #8192
cx1a p0, r0, #8192
cx1 p0, r0, #-1
cx1a p0, r0, #-1

cx1 p8, r0, #0
cx1a p8, r0, #0

cx1 p0, r16, #0
cx1a p0, r16, #0

cx1 p0, r13, #0
cx1a p0, r13, #0

itttt ne
cx1 p0, r0, #0
cx1ne p0, r0, #0
cx1a p0, r0, #0
cx1aeq p0, r0, #0

cx1 p1, r0, #0
cx1a p1, r0, #0

cx1 p0, r0, r0, #0
cx1a p0, r0, r0, #0

cx1 p0, #0
cx1a p0, #0

cx1 p0, r15, #0
cx1a p0, r15, #0

# cx1d{a}
# Immediate out of range.
# Each register out of range.
# APSR_nzcv disallowed as destination register.
# rd<odd> => constrained unpredictable
# r< N > 10 > => constrained unpredictable
# IT block => constrained unpredictable
#
# Error given when using coprocessor number not enabled on command line.
# Disallow non-incrementing values in destination.
# Too many arguments
# Too little arguments

cx1d p0, r0, r1, #8192
cx1da p0, r0, r1, #8192
cx1d p0, r0, r1, #-1
cx1da p0, r0, r1, #-1

cx1d p8, r0, r1, #0
cx1da p8, r0, r1, #0

cx1d p0, r16, r17, #0
cx1da p0, r16, r17, #0

cx1d p0, APSR_nzcv, r15, #0
cx1da p0, APSR_nzcv, r15, #0

cx1d p0, r9, r10, #0
cx1da p0, r9, r10, #0

cx1d p0, r13, r14, #0
cx1da p0, r13, r14, #0

itttt ne
cx1d p0, r0, r1, #0
cx1da p0, r0, r1, #0
cx1dne p0, r0, r1, #0
cx1daeq p0, r0, r1, #0

cx1d p1, r0, r1, #0
cx1da p1, r0, r1, #0

cx1d p0, r0, r2, #0
cx1da p0, r0, r2, #0

cx1d p0, r0, r1, r0, #0
cx1da p0, r0, r1, r0, #0

cx1d p0, r0, #0
cx1da p0, r0, #0

# cx2{a}
# Immediate out of range.
# Each register out of range.
# rd13 => constrained unpredictable
# rn13 => constrained unpredictable
# IT block => constrained unpredictable
#
# Error given when using coprocessor number not enabled on command line.
# Too many arguments
# Too little arguments.
# r15 instead of APSR_nzcv

cx2 p0, r0, r0, #512
cx2a p0, r0, r0, #512
cx2 p0, r0, r0, #-1
cx2a p0, r0, r0, #-1

cx2 p8, r0, r0, #0
cx2a p8, r0, r0, #0

cx2 p0, r16, r0, #0
cx2a p0, r16, r0, #0

cx2 p0, r0, r16, #0
cx2a p0, r0, r16, #0

cx2 p0, r13, r0, #0
cx2a p0, r13, r0, #0

cx2 p0, r0, r13, #0
cx2a p0, r0, r13, #0

itttt ne
cx2 p0, r0, r0, #0
cx2a p0, r0, r0, #0
cx2ne p0, r0, r0, #0
cx2aeq p0, r0, r0, #0

cx2 p1, r0, r0, #0
cx2a p1, r0, r0, #0

cx2 p0, r0, r0, r0, #0
cx2a p0, r0, r0, r0, #0

cx2 p0, r0, #0
cx2a p0, r0, #0

cx2 p0, r0, r15, #0
cx2a p0, r0, r15, #0

cx2 p0, r15, r0, #0
cx2a p0, r15, r0, #0

# cx2d{a}
# Immediate out of range.
# Each register out of range.
# APSR_nzcv disallowed as destination register.
# rd<odd> => constrained unpredictable
# rd< N > 10 > => constrained unpredictable
# rn13 => constrained unpredictable
# IT block => constrained unpredictable
#
# Error given when using coprocessor number not enabled on command line.
# Disallow non-incrementing values in destination.
# Too many arguments
# Too little arguments
cx2d p0, r0, r1, r0, #512
cx2da p0, r0, r1, r0, #512
cx2d p0, r0, r1, r0, #-1
cx2da p0, r0, r1, r0, #-1

cx2d p8, r0, r1, r0, #0
cx2da p8, r0, r1, r0, #0

cx2d p0, r16, r17, r0, #0
cx2da p0, r16, r17, r0, #0

cx2d p0, r0, r1, r16, #0
cx2da p0, r0, r1, r16, #0

cx2d p0, APSR_nzcv, r15, r0, #0
cx2da p0, APSR_nzcv, r15, r0, #0

cx2d p0, r9, r10, r0, #0
cx2da p0, r9, r10, r0, #0

cx2d p0, r12, r13, r0, #0
cx2da p0, r12, r13, r0, #0

cx2d p0, r0, r1, r13, #0
cx2da p0, r0, r1, r13, #0

cx2d p0, r0, r1, r15, #0
cx2da p0, r0, r1, r15, #0

itttt ne
cx2d p0, r0, r1, r0, #0
cx2da p0, r0, r1, r0, #0
cx2dne p0, r0, r1, r0, #0
cx2daeq p0, r0, r1, r0, #0

cx2d p1, r0, r1, r0, #0
cx2da p1, r0, r1, r0, #0

cx2d p0, r0, r2, r0, #0
cx2da p0, r0, r2, r0, #0

cx2d p0, r0, r1, r0, r0, #0
cx2da p0, r0, r1, r0, r0, #0

cx2d p0, r0, r0, #0
cx2da p0, r0, r0, #0

# cx2{a}
# Immediate out of range.
# Each register out of range.
# rd13 => constrained unpredictable
# rn13 => constrained unpredictable
# rm13 => constrained unpredictable
# IT block => constrained unpredictable
#
# Error given when using coprocessor number not enabled on command line.
# Too many arguments
# Too little arguments.
# r15 instead of APSR_nzcv

cx3 p0, r0, r0, r0, #64
cx3a p0, r0, r0, r0, #64
cx3 p0, r0, r0, r0, #-1
cx3a p0, r0, r0, r0, #-1

cx3 p8, r0, r0, r0, #0
cx3a p8, r0, r0, r0, #0

cx3 p0, r16, r0, r0, #0
cx3a p0, r16, r0, r0, #0

cx3 p0, r0, r16, r0, #0
cx3a p0, r0, r16, r0, #0

cx3 p0, r0, r0, r16, #0
cx3a p0, r0, r0, r16, #0

cx3 p0, r13, r0, r0, #0
cx3a p0, r13, r0, r0, #0

cx3 p0, r0, r13, r0, #0
cx3a p0, r0, r13, r0, #0

cx3 p0, r0, r0, r13, #0
cx3a p0, r0, r0, r13, #0

itttt ne
cx3 p0, r0, r0, r0, #0
cx3a p0, r0, r0, r0, #0
cx3ne p0, r0, r0, r0, #0
cx3aeq p0, r0, r0, r0, #0

cx3 p1, r0, r0, r0, #0
cx3a p1, r0, r0, r0, #0

cx3 p0, r0, r0, r0, r0, #0
cx3a p0, r0, r0, r0, r0, #0

cx3 p0, r0, r0, #0
cx3a p0, r0, r0, #0

cx3 p0, r15, r0, r0, #0
cx3a p0, r15, r0, r0, #0

cx3 p0, r0, r15, r0, #0
cx3a p0, r0, r15, r0, #0

cx3 p0, r0, r0, r15, #0
cx3a p0, r0, r0, r15, #0

# cx3d{a}
# Immediate out of range.
# Each register out of range.
# APSR_nzcv disallowed as destination register.
# rd<odd> => constrained unpredictable
# rd< N > 10 > => constrained unpredictable
# rn13 => constrained unpredictable
# rm13 => constrained unpredictable
# rn15 disallowed (pattern matches APSR_nzcv)
# rm15 disallowed (pattern matches APSR_nzcv)
# IT block => constrained unpredictable
#
# Error given when using coprocessor number not enabled on command line.
# Disallow non-incrementing values in destination.
# Too many arguments
# Too little arguments
cx3d p0, r0, r1, r0, r0, #64
cx3da p0, r0, r1, r0, r0, #64
cx3d p0, r0, r1, r0, r0, #-1
cx3da p0, r0, r1, r0, r0, #-1

cx3d p8, r0, r1, r0, r0, #0
cx3da p8, r0, r1, r0, r0, #0

cx3d p0, r16, r17, r0, r0, #0
cx3da p0, r16, r17, r0, r0, #0

cx3d p0, r0, r1, r16, r0, #0
cx3da p0, r0, r1, r16, r0, #0

cx3d p0, r0, r1, r0, r16, #0
cx3da p0, r0, r1, r0, r16, #0

cx3d p0, APSR_nzcv, r15, r0, r0, #0
cx3da p0, APSR_nzcv, r15, r0, r0, #0

cx3d p0, r9, r10, r0, r0, #0
cx3da p0, r9, r10, r0, r0, #0

cx3d p0, r12, r13, r0, r0, #0
cx3da p0, r12, r13, r0, r0, #0

cx3d p0, r0, r1, r13, r0, #0
cx3da p0, r0, r1, r13, r0, #0

cx3d p0, r0, r1, r0, r13, #0
cx3da p0, r0, r1, r0, r13, #0

cx3d p0, r0, r1, r15, r0, #0
cx3da p0, r0, r1, r15, r0, #0

cx3d p0, r0, r1, r0, r15, #0
cx3da p0, r0, r1, r0, r15, #0

itttt ne
cx3d p0, r0, r1, r0, r0, #0
cx3da p0, r0, r1, r0, r0, #0
cx3dne p0, r0, r1, r0, r0, #0
cx3daeq p0, r0, r1, r0, r0, #0

cx3d p1, r0, r1, r0, r0, #0
cx3da p1, r0, r1, r0, r0, #0

cx3d p0, r0, r2, r0, r0, #0
cx3da p0, r0, r2, r0, r0, #0

cx3d p0, r0, r1, r0, r0, r0, #0
cx3da p0, r0, r1, r0, r0, r0, #0

cx3d p0, r0, r0, r0, #0
cx3da p0, r0, r0, r0, #0

# vcx1{a}
# Immediate out of range.
# Each register out of range.
# IT block => constrained unpredictable
#
# Error given when using coprocessor number not enabled on command line.
# Too many arguments
# Too little arguments

vcx1 p0, q0, #4096
vcx1a p0, q0, #4096
vcx1 p0, q0, #-1
vcx1a p0, q0, #-1

vcx1 p8, q0, #0
vcx1a p8, q0, #0
vcx1 p0, q8, #0
vcx1a p0, q8, #0

itttt ne
vcx1 p0, q0, #0
vcx1ne p0, q0, #0
vcx1a p0, q0, #0
vcx1ane p0, q0, #0

vcx1 p1, q0, #0
vcx1a p1, q0, #0

vcx1 p0, q0, q0, #0
vcx1a p0, q0, q0, #0
vcx1 p0, #0
vcx1a p0, #0


vcx1 p0, d0, #2048
vcx1a p0, d0, #2048
vcx1 p0, d0, #-1
vcx1a p0, d0, #-1

vcx1 p8, d0, #0
vcx1a p8, d0, #0
vcx1 p0, d16, #0
vcx1a p0, d16, #0

itttt ne
vcx1 p0, d0, #0
vcx1ne p0, d0, #0
vcx1a p0, d0, #0
vcx1ane p0, d0, #0

vcx1 p1, d0, #0
vcx1a p1, d0, #0

vcx1 p0, d0, d0, #0
vcx1a p0, d0, d0, #0
vcx1 p0, #0
vcx1a p0, #0


vcx1 p0, s0, #2048
vcx1a p0, s0, #2048
vcx1 p0, s0, #-1
vcx1a p0, s0, #-1

vcx1 p8, s0, #0
vcx1a p8, s0, #0
vcx1 p0, s32, #0
vcx1a p0, s32, #0

itttt ne
vcx1 p0, s0, #0
vcx1ne p0, s0, #0
vcx1a p0, s0, #0
vcx1ane p0, s0, #0

vcx1 p1, s0, #0
vcx1a p1, s0, #0

vcx1 p0, s0, s0, #0
vcx1a p0, s0, s0, #0
vcx1 p0, #0
vcx1a p0, #0


# vcx2{a}
# Immediate out of range.
# Each register out of range.
# IT block => constrained unpredictable
#
# Error given when using coprocessor number not enabled on command line.
# Too many arguments
# Too little arguments

vcx2 p0, q0, q0, #128
vcx2a p0, q0, q0, #128
vcx2 p0, q0, q0, #-1
vcx2a p0, q0, q0, #-1

vcx2 p8, q0, q0, #0
vcx2a p8, q0, q0, #0
vcx2 p0, q8, q0, #0
vcx2a p0, q8, q0, #0
vcx2 p0, q0, q8, #0
vcx2a p0, q0, q8, #0

itttt ne
vcx2 p0, q0, q0, #0
vcx2ne p0, q0, q0, #0
vcx2a p0, q0, q0, #0
vcx2ane p0, q0, q0, #0

vcx2 p1, q0, q0, #0
vcx2a p1, q0, q0, #0

vcx2 p0, q0, q0, q0, #0
vcx2a p0, q0, q0, q0, #0
vcx2 p0, q0, #0
vcx2a p0, q0, #0


vcx2 p0, d0, d0, #64
vcx2a p0, d0, d0, #64
vcx2 p0, d0, d0, #-1
vcx2a p0, d0, d0, #-1

vcx2 p8, d0, d0, #0
vcx2a p8, d0, d0, #0
vcx2 p0, d16, d0, #0
vcx2a p0, d16, d0, #0
vcx2 p0, d0, d16, #0
vcx2a p0, d0, d16, #0

itttt ne
vcx2 p0, d0, d0, #0
vcx2ne p0, d0, d0, #0
vcx2a p0, d0, d0, #0
vcx2ane p0, d0, d0, #0

vcx2 p1, d0, d0, #0
vcx2a p1, d0, d0, #0

vcx2 p0, d0, d0, d0, #0
vcx2a p0, d0, d0, d0, #0
vcx2 p0, d0, #0
vcx2a p0, d0, #0


vcx2 p0, s0, s0, #64
vcx2a p0, s0, s0, #64
vcx2 p0, s0, s0, #-1
vcx2a p0, s0, s0, #-1

vcx2 p8, s0, s0, #0
vcx2a p8, s0, s0, #0
vcx2 p0, s32, s0, #0
vcx2a p0, s32, s0, #0
vcx2 p0, s0, s32, #0
vcx2a p0, s0, s32, #0

itttt ne
vcx2 p0, s0, s0, #0
vcx2ne p0, s0, s0, #0
vcx2a p0, s0, s0, #0
vcx2ane p0, s0, s0, #0

vcx2 p1, s0, s0, #0
vcx2a p1, s0, s0, #0

vcx2 p0, s0, s0, s0, #0
vcx2a p0, s0, s0, s0, #0
vcx2 p0, s0, #0
vcx2a p0, s0, #0

# vcx3{a}
# Immediate out of range.
# Each register out of range.
# IT block => constrained unpredictable
#
# Error given when using coprocessor number not enabled on command line.
# Too many arguments
# Too little arguments

vcx3 p0, q0, q0, q0, #16
vcx3a p0, q0, q0, q0, #16
vcx3 p0, q0, q0, q0, #-1
vcx3a p0, q0, q0, q0, #-1

vcx3 p8, q0, q0, q0, #0
vcx3a p8, q0, q0, q0, #0
vcx3 p0, q8, q0, q0, #0
vcx3a p0, q8, q0, q0, #0
vcx3 p0, q8, q0, q0, #0
vcx3a p0, q0, q8, q0, #0
vcx3 p0, q0, q0, q8, #0
vcx3a p0, q0, q0, q8, #0

itttt ne
vcx3 p0, q0, q0, q0, #0
vcx3ne p0, q0, q0, q0, #0
vcx3a p0, q0, q0, q0, #0
vcx3ane p0, q0, q0, q0, #0

vcx3 p1, q0, q0, q0, #0
vcx3a p1, q0, q0, q0, #0

vcx3 p0, q0, q0, q0, q0, #0
vcx3a p0, q0, q0, q0, q0, #0
vcx3 p0, q0, q0, #0
vcx3a p0, q0, q0, #0


vcx3 p0, d0, d0, d0, #8
vcx3a p0, d0, d0, d0, #8
vcx3 p0, d0, d0, d0, #-1
vcx3a p0, d0, d0, d0, #-1

vcx3 p8, d0, d0, d0, #0
vcx3a p8, d0, d0, d0, #0
vcx3 p0, d16, d0, d0, #0
vcx3a p0, d16, d0, d0, #0
vcx3 p0, d0, d16, d0, #0
vcx3a p0, d0, d16, d0, #0
vcx3 p0, d0, d0, d16, #0
vcx3a p0, d0, d0, d16, #0

itttt ne
vcx3 p0, d0, d0, d0, #0
vcx3ne p0, d0, d0, d0, #0
vcx3a p0, d0, d0, d0, #0
vcx3ane p0, d0, d0, d0, #0

vcx3 p1, d0, d0, d0, #0
vcx3a p1, d0, d0, d0, #0

vcx3 p0, d0, d0, d0, d0, #0
vcx3a p0, d0, d0, d0, d0, #0
vcx3 p0, d0, d0, #0
vcx3a p0, d0, d0, #0


vcx3 p0, s0, s0, s0, #8
vcx3a p0, s0, s0, s0, #8
vcx3 p0, s0, s0, s0, #-1
vcx3a p0, s0, s0, s0, #-1

vcx3 p8, s0, s0, s0, #0
vcx3a p8, s0, s0, s0, #0
vcx3 p0, s32, s0, s0, #0
vcx3a p0, s32, s0, s0, #0
vcx3 p0, s0, s32, s0, #0
vcx3a p0, s0, s32, s0, #0
vcx3 p0, s0, s0, s32, #0
vcx3a p0, s0, s0, s32, #0

itttt ne
vcx3 p0, s0, s0, s0, #0
vcx3ne p0, s0, s0, s0, #0
vcx3a p0, s0, s0, s0, #0
vcx3ane p0, s0, s0, s0, #0

vcx3 p1, s0, s0, s0, #0
vcx3a p1, s0, s0, s0, #0

vcx3 p0, s0, s0, s0, s0, #0
vcx3a p0, s0, s0, s0, s0, #0
vcx3 p0, s0, s0, #0
vcx3a p0, s0, s0, #0

