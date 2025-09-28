[^ :]+: Assembler messages:
[^ :]+:[0-9]+: Error: expected a register or register list at operand 1 -- `ld1b 0,pn8/z,\[x0\]'
[^ :]+:[0-9]+: Error: expected an SVE predicate register at operand 2 -- `ld1b {z0\.b-z1\.b},0,\[x0\]'
[^ :]+:[0-9]+: Error: invalid addressing mode at operand 3 -- `ld1b {z0\.b-z1\.b},pn8/z,0'
[^ :]+:[0-9]+: Error: expected a list of 2 or 4 registers at operand 1 -- `ld1b {z0\.b},pn8/z,\[x0\]'
[^ :]+:[0-9]+: Error: expected a list of 2 or 4 registers at operand 1 -- `ld1b {z0\.b-z2\.b},pn8/z,\[x0\]'
[^ :]+:[0-9]+: Error: start register out of range at operand 1 -- `ld1b {z1\.b-z2\.b},pn8/z,\[x0\]'
[^ :]+:[0-9]+: Error: expected a single-register list at operand 1 -- `ld1b {z0\.b-z1\.b},p8/z,\[x0\]'
[^ :]+:[0-9]+: Error: operand mismatch -- `ld1b {z0\.b-z1\.b},pn8,\[x0\]'
[^ :]+:[0-9]+: Info:    did you mean this\?
[^ :]+:[0-9]+: Info:    	ld1b {z0\.b-z1\.b}, pn8/z, \[x0\]
[^ :]+:[0-9]+: Error: operand mismatch -- `ld1b {z0\.b-z1\.b},pn8/m,\[x0\]'
[^ :]+:[0-9]+: Info:    did you mean this\?
[^ :]+:[0-9]+: Info:    	ld1b {z0\.b-z1\.b}, pn8/z, \[x0\]
[^ :]+:[0-9]+: Error: operand mismatch -- `ld1b {z0\.b-z1\.b},pn8\.b,\[x0\]'
[^ :]+:[0-9]+: Info:    did you mean this\?
[^ :]+:[0-9]+: Info:    	ld1b {z0\.b-z1\.b}, pn8/z, \[x0\]
[^ :]+:[0-9]+: Error: pn8-pn15 expected at operand 2 -- `ld1b {z0\.b-z1\.b},pn7/z,\[x0\]'
[^ :]+:[0-9]+: Error: expected a 64-bit base register at operand 3 -- `ld1b {z0\.b-z1\.b},pn8/z,\[w0\]'
[^ :]+:[0-9]+: Error: invalid base register at operand 3 -- `ld1b {z0\.b-z1\.b},pn8/z,\[xzr\]'
[^ :]+:[0-9]+: Error: invalid addressing mode at operand 3 -- `ld1b {z0\.b-z1\.b},pn8/z,\[x0,#1\]'
[^ :]+:[0-9]+: Error: immediate offset out of range -16 to 14 at operand 3 -- `ld1b {z0\.b-z1\.b},pn8/z,\[x0,#-32,mul vl\]'
[^ :]+:[0-9]+: Error: immediate offset out of range -16 to 14 at operand 3 -- `ld1b {z0\.b-z1\.b},pn8/z,\[x0,#-18,mul vl\]'
[^ :]+:[0-9]+: Error: immediate value must be a multiple of 2 at operand 3 -- `ld1b {z0\.b-z1\.b},pn8/z,\[x0,#-15,mul vl\]'
[^ :]+:[0-9]+: Error: immediate value must be a multiple of 2 at operand 3 -- `ld1b {z0\.b-z1\.b},pn8/z,\[x0,#-1,mul vl\]'
[^ :]+:[0-9]+: Error: immediate value must be a multiple of 2 at operand 3 -- `ld1b {z0\.b-z1\.b},pn8/z,\[x0,#1,mul vl\]'
[^ :]+:[0-9]+: Error: immediate value must be a multiple of 2 at operand 3 -- `ld1b {z0\.b-z1\.b},pn8/z,\[x0,#13,mul vl\]'
[^ :]+:[0-9]+: Error: immediate offset out of range -16 to 14 at operand 3 -- `ld1b {z0\.b-z1\.b},pn8/z,\[x0,#15,mul vl\]'
[^ :]+:[0-9]+: Error: immediate offset out of range -16 to 14 at operand 3 -- `ld1b {z0\.b-z1\.b},pn8/z,\[x0,#16,mul vl\]'
[^ :]+:[0-9]+: Error: start register out of range at operand 1 -- `ld1b {z1\.b-z4\.b},pn8/z,\[x0\]'
[^ :]+:[0-9]+: Error: start register out of range at operand 1 -- `ld1b {z2\.b-z5\.b},pn8/z,\[x0\]'
[^ :]+:[0-9]+: Error: start register out of range at operand 1 -- `ld1b {z3\.b-z6\.b},pn8/z,\[x0\]'
[^ :]+:[0-9]+: Error: expected a single-register list at operand 1 -- `ld1b {z0\.b-z3\.b},p8/z,\[x0\]'
[^ :]+:[0-9]+: Error: operand mismatch -- `ld1b {z0\.b-z3\.b},pn8,\[x0\]'
[^ :]+:[0-9]+: Info:    did you mean this\?
[^ :]+:[0-9]+: Info:    	ld1b {z0\.b-z3\.b}, pn8/z, \[x0\]
[^ :]+:[0-9]+: Error: operand mismatch -- `ld1b {z0\.b-z3\.b},pn8/m,\[x0\]'
[^ :]+:[0-9]+: Info:    did you mean this\?
[^ :]+:[0-9]+: Info:    	ld1b {z0\.b-z3\.b}, pn8/z, \[x0\]
[^ :]+:[0-9]+: Error: operand mismatch -- `ld1b {z0\.b-z3\.b},pn8\.b,\[x0\]'
[^ :]+:[0-9]+: Info:    did you mean this\?
[^ :]+:[0-9]+: Info:    	ld1b {z0\.b-z3\.b}, pn8/z, \[x0\]
[^ :]+:[0-9]+: Error: pn8-pn15 expected at operand 2 -- `ld1b {z0\.b-z3\.b},pn0/z,\[x0\]'
[^ :]+:[0-9]+: Error: pn8-pn15 expected at operand 2 -- `ld1b {z0\.b-z3\.b},pn7/z,\[x0\]'
[^ :]+:[0-9]+: Error: expected a 64-bit base register at operand 3 -- `ld1b {z0\.b-z3\.b},pn8/z,\[w0\]'
[^ :]+:[0-9]+: Error: invalid base register at operand 3 -- `ld1b {z0\.b-z3\.b},pn8/z,\[xzr\]'
[^ :]+:[0-9]+: Error: invalid addressing mode at operand 3 -- `ld1b {z0\.b-z3\.b},pn8/z,\[x0,#4\]'
[^ :]+:[0-9]+: Error: immediate offset out of range -32 to 28 at operand 3 -- `ld1b {z0\.b-z3\.b},pn8/z,\[x0,#-36,mul vl\]'
[^ :]+:[0-9]+: Error: immediate value must be a multiple of 4 at operand 3 -- `ld1b {z0\.b-z3\.b},pn8/z,\[x0,#-31,mul vl\]'
[^ :]+:[0-9]+: Error: immediate value must be a multiple of 4 at operand 3 -- `ld1b {z0\.b-z3\.b},pn8/z,\[x0,#-30,mul vl\]'
[^ :]+:[0-9]+: Error: immediate value must be a multiple of 4 at operand 3 -- `ld1b {z0\.b-z3\.b},pn8/z,\[x0,#-29,mul vl\]'
[^ :]+:[0-9]+: Error: immediate value must be a multiple of 4 at operand 3 -- `ld1b {z0\.b-z3\.b},pn8/z,\[x0,#-14,mul vl\]'
[^ :]+:[0-9]+: Error: immediate value must be a multiple of 4 at operand 3 -- `ld1b {z0\.b-z3\.b},pn8/z,\[x0,#-3,mul vl\]'
[^ :]+:[0-9]+: Error: immediate value must be a multiple of 4 at operand 3 -- `ld1b {z0\.b-z3\.b},pn8/z,\[x0,#-2,mul vl\]'
[^ :]+:[0-9]+: Error: immediate value must be a multiple of 4 at operand 3 -- `ld1b {z0\.b-z3\.b},pn8/z,\[x0,#-1,mul vl\]'
[^ :]+:[0-9]+: Error: immediate value must be a multiple of 4 at operand 3 -- `ld1b {z0\.b-z3\.b},pn8/z,\[x0,#1,mul vl\]'
[^ :]+:[0-9]+: Error: immediate value must be a multiple of 4 at operand 3 -- `ld1b {z0\.b-z3\.b},pn8/z,\[x0,#2,mul vl\]'
[^ :]+:[0-9]+: Error: immediate value must be a multiple of 4 at operand 3 -- `ld1b {z0\.b-z3\.b},pn8/z,\[x0,#3,mul vl\]'
[^ :]+:[0-9]+: Error: immediate value must be a multiple of 4 at operand 3 -- `ld1b {z0\.b-z3\.b},pn8/z,\[x0,#14,mul vl\]'
[^ :]+:[0-9]+: Error: immediate value must be a multiple of 4 at operand 3 -- `ld1b {z0\.b-z3\.b},pn8/z,\[x0,#25,mul vl\]'
[^ :]+:[0-9]+: Error: immediate value must be a multiple of 4 at operand 3 -- `ld1b {z0\.b-z3\.b},pn8/z,\[x0,#26,mul vl\]'
[^ :]+:[0-9]+: Error: immediate value must be a multiple of 4 at operand 3 -- `ld1b {z0\.b-z3\.b},pn8/z,\[x0,#27,mul vl\]'
[^ :]+:[0-9]+: Error: immediate offset out of range -32 to 28 at operand 3 -- `ld1b {z0\.b-z3\.b},pn8/z,\[x0,#29,mul vl\]'
[^ :]+:[0-9]+: Error: immediate offset out of range -32 to 28 at operand 3 -- `ld1b {z0\.b-z3\.b},pn8/z,\[x0,#30,mul vl\]'
[^ :]+:[0-9]+: Error: immediate offset out of range -32 to 28 at operand 3 -- `ld1b {z0\.b-z3\.b},pn8/z,\[x0,#31,mul vl\]'
[^ :]+:[0-9]+: Error: immediate offset out of range -32 to 28 at operand 3 -- `ld1b {z0\.b-z3\.b},pn8/z,\[x0,#32,mul vl\]'
[^ :]+:[0-9]+: Error: type mismatch in vector register list at operand 1 -- `ld1b {z0\.b,z8\.b,z16\.s},pn8/z,\[x0\]'
[^ :]+:[0-9]+: Error: type mismatch in vector register list at operand 1 -- `ld1b {z0\.b,z4\.b,z8\.s},pn8/z,\[x0\]'
[^ :]+:[0-9]+: Error: the register list must have a stride of 1 or 8 at operand 1 -- `ld1b {z0\.b,z2\.b},pn8/z,\[x0\]`
[^ :]+:[0-9]+: Error: the register list must have a stride of 1 or 8 at operand 1 -- `ld1b {z0\.b,z3\.b},pn8/z,\[x0\]`
[^ :]+:[0-9]+: Error: the register list must have a stride of 1 or 8 at operand 1 -- `ld1b {z0\.b,z4\.b},pn8/z,\[x0\]`
[^ :]+:[0-9]+: Error: the register list must have a stride of 1 or 8 at operand 1 -- `ld1b {z0\.b,z5\.b},pn8/z,\[x0\]`
[^ :]+:[0-9]+: Error: the register list must have a stride of 1 or 8 at operand 1 -- `ld1b {z0\.b,z6\.b},pn8/z,\[x0\]`
[^ :]+:[0-9]+: Error: the register list must have a stride of 1 or 8 at operand 1 -- `ld1b {z0\.b,z7\.b},pn8/z,\[x0\]`
[^ :]+:[0-9]+: Error: the register list must have a stride of 1 or 8 at operand 1 -- `ld1b {z0\.b,z9\.b},pn8/z,\[x0\]`
[^ :]+:[0-9]+: Error: the register list must have a stride of 1 or 8 at operand 1 -- `ld1b {z0\.b,z15\.b},pn8/z,\[x0\]`
[^ :]+:[0-9]+: Error: the register list must have a stride of 1 or 8 at operand 1 -- `ld1b {z0\.b,z16\.b},pn8/z,\[x0\]`
[^ :]+:[0-9]+: Error: the register list must have a stride of 1 or 8 at operand 1 -- `ld1b {z0\.b,z23\.b},pn8/z,\[x0\]`
[^ :]+:[0-9]+: Error: the register list must have a stride of 1 or 8 at operand 1 -- `ld1b {z0\.b,z24\.b},pn8/z,\[x0\]`
[^ :]+:[0-9]+: Error: the register list must have a stride of 1 or 8 at operand 1 -- `ld1b {z0\.b,z31\.b},pn8/z,\[x0\]`
[^ :]+:[0-9]+: Error: start register out of range at operand 1 -- `ld1b {z8\.b,z16\.b},pn8/z,\[x0\]'
[^ :]+:[0-9]+: Error: start register out of range at operand 1 -- `ld1b {z9\.b,z17\.b},pn8/z,\[x0\]'
[^ :]+:[0-9]+: Error: start register out of range at operand 1 -- `ld1b {z10\.b,z18\.b},pn8/z,\[x0\]'
[^ :]+:[0-9]+: Error: start register out of range at operand 1 -- `ld1b {z11\.b,z19\.b},pn8/z,\[x0\]'
[^ :]+:[0-9]+: Error: start register out of range at operand 1 -- `ld1b {z12\.b,z20\.b},pn8/z,\[x0\]'
[^ :]+:[0-9]+: Error: start register out of range at operand 1 -- `ld1b {z13\.b,z21\.b},pn8/z,\[x0\]'
[^ :]+:[0-9]+: Error: start register out of range at operand 1 -- `ld1b {z14\.b,z22\.b},pn8/z,\[x0\]'
[^ :]+:[0-9]+: Error: start register out of range at operand 1 -- `ld1b {z15\.b,z23\.b},pn8/z,\[x0\]'
[^ :]+:[0-9]+: Error: start register out of range at operand 1 -- `ld1b {z24\.b,z0\.b},pn8/z,\[x0\]'
[^ :]+:[0-9]+: Error: start register out of range at operand 1 -- `ld1b {z25\.b,z1\.b},pn8/z,\[x0\]'
[^ :]+:[0-9]+: Error: start register out of range at operand 1 -- `ld1b {z26\.b,z2\.b},pn8/z,\[x0\]'
[^ :]+:[0-9]+: Error: start register out of range at operand 1 -- `ld1b {z27\.b,z3\.b},pn8/z,\[x0\]'
[^ :]+:[0-9]+: Error: start register out of range at operand 1 -- `ld1b {z28\.b,z4\.b},pn8/z,\[x0\]'
[^ :]+:[0-9]+: Error: start register out of range at operand 1 -- `ld1b {z29\.b,z5\.b},pn8/z,\[x0\]'
[^ :]+:[0-9]+: Error: start register out of range at operand 1 -- `ld1b {z30\.b,z6\.b},pn8/z,\[x0\]'
[^ :]+:[0-9]+: Error: start register out of range at operand 1 -- `ld1b {z31\.b,z7\.b},pn8/z,\[x0\]'
[^ :]+:[0-9]+: Error: the register list must have a stride of 1 or 8 at operand 1 -- `ld1b {z8\.b,z0\.b},pn8/z,\[x0\]`
[^ :]+:[0-9]+: Error: type mismatch in vector register list at operand 1 -- `ld1b {z0\.h,z8\.b},pn8/z,\[x0\]'
[^ :]+:[0-9]+: Error: type mismatch in vector register list at operand 1 -- `ld1b {z0\.b,z8\.h},pn8/z,\[x0\]'
[^ :]+:[0-9]+: Error: operand mismatch -- `ld1b {z0\.h,z8\.h},pn8/z,\[x0\]'
[^ :]+:[0-9]+: Info:    did you mean this\?
[^ :]+:[0-9]+: Info:    	ld1b {z0\.b, z8\.b}, pn8/z, \[x0\]
[^ :]+:[0-9]+: Error: missing type suffix at operand 1 -- `ld1b {z0,z8},pn8/z,\[x0\]'
[^ :]+:[0-9]+: Error: expected a single-register list at operand 1 -- `ld1b {z0\.b,z8\.b},p8/z,\[x0\]'
[^ :]+:[0-9]+: Error: expected a 64-bit base register at operand 3 -- `ld1b {z0\.b,z8\.b},pn8/z,\[w0\]'
[^ :]+:[0-9]+: Error: invalid base register at operand 3 -- `ld1b {z0\.b,z8\.b},pn8/z,\[xzr\]'
[^ :]+:[0-9]+: Error: invalid addressing mode at operand 3 -- `ld1b {z0\.b,z8\.b},pn8/z,\[x0,#-16\]'
[^ :]+:[0-9]+: Error: invalid addressing mode at operand 3 -- `ld1b {z0\.b,z8\.b},pn8/z,\[x0,#-1\]'
[^ :]+:[0-9]+: Error: immediate offset out of range -16 to 14 at operand 3 -- `ld1b {z0\.b,z8\.b},pn8/z,\[x0,#-32,mul vl\]'
[^ :]+:[0-9]+: Error: immediate offset out of range -16 to 14 at operand 3 -- `ld1b {z0\.b,z8\.b},pn8/z,\[x0,#-17,mul vl\]'
[^ :]+:[0-9]+: Error: immediate value must be a multiple of 2 at operand 3 -- `ld1b {z0\.b,z8\.b},pn8/z,\[x0,#-15,mul vl\]'
[^ :]+:[0-9]+: Error: immediate value must be a multiple of 2 at operand 3 -- `ld1b {z0\.b,z8\.b},pn8/z,\[x0,#-1,mul vl\]'
[^ :]+:[0-9]+: Error: immediate value must be a multiple of 2 at operand 3 -- `ld1b {z0\.b,z8\.b},pn8/z,\[x0,#13,mul vl\]'
[^ :]+:[0-9]+: Error: immediate offset out of range -16 to 14 at operand 3 -- `ld1b {z0\.b,z8\.b},pn8/z,\[x0,#15,mul vl\]'
[^ :]+:[0-9]+: Error: immediate offset out of range -16 to 14 at operand 3 -- `ld1b {z0\.b,z8\.b},pn8/z,\[x0,#16,mul vl\]'
[^ :]+:[0-9]+: Error: start register out of range at operand 1 -- `ld1b {z4\.b,z8\.b,z12\.b,z16\.b},pn8/z,\[x0\]'
[^ :]+:[0-9]+: Error: start register out of range at operand 1 -- `ld1b {z5\.b,z9\.b,z13\.b,z17\.b},pn8/z,\[x0\]'
[^ :]+:[0-9]+: Error: start register out of range at operand 1 -- `ld1b {z6\.b,z10\.b,z14\.b,z18\.b},pn8/z,\[x0\]'
[^ :]+:[0-9]+: Error: start register out of range at operand 1 -- `ld1b {z7\.b,z11\.b,z15\.b,z19\.b},pn8/z,\[x0\]'
[^ :]+:[0-9]+: Error: start register out of range at operand 1 -- `ld1b {z8\.b,z12\.b,z16\.b,z20\.b},pn8/z,\[x0\]'
[^ :]+:[0-9]+: Error: start register out of range at operand 1 -- `ld1b {z9\.b,z13\.b,z17\.b,z21\.b},pn8/z,\[x0\]'
[^ :]+:[0-9]+: Error: start register out of range at operand 1 -- `ld1b {z10\.b,z14\.b,z18\.b,z22\.b},pn8/z,\[x0\]'
[^ :]+:[0-9]+: Error: start register out of range at operand 1 -- `ld1b {z11\.b,z15\.b,z19\.b,z23\.b},pn8/z,\[x0\]'
[^ :]+:[0-9]+: Error: start register out of range at operand 1 -- `ld1b {z12\.b,z16\.b,z20\.b,z24\.b},pn8/z,\[x0\]'
[^ :]+:[0-9]+: Error: start register out of range at operand 1 -- `ld1b {z13\.b,z17\.b,z21\.b,z25\.b},pn8/z,\[x0\]'
[^ :]+:[0-9]+: Error: start register out of range at operand 1 -- `ld1b {z14\.b,z18\.b,z22\.b,z26\.b},pn8/z,\[x0\]'
[^ :]+:[0-9]+: Error: start register out of range at operand 1 -- `ld1b {z15\.b,z19\.b,z23\.b,z27\.b},pn8/z,\[x0\]'
[^ :]+:[0-9]+: Error: start register out of range at operand 1 -- `ld1b {z20\.b,z24\.b,z28\.b,z0\.b},pn8/z,\[x0\]'
[^ :]+:[0-9]+: Error: the register list must have a stride of 1 or 4 at operand 1 -- `ld1b {z0\.b,z2\.b,z4\.b,z6\.b},pn8/z,\[x0\]`
[^ :]+:[0-9]+: Error: the register list must have a stride of 1 or 4 at operand 1 -- `ld1b {z0\.b,z3\.b,z6\.b,z9\.b},pn8/z,\[x0\]`
[^ :]+:[0-9]+: Error: the register list must have a stride of 1 or 4 at operand 1 -- `ld1b {z0\.b,z8\.b,z16\.b,z24\.b},pn8/z,\[x0\]`
[^ :]+:[0-9]+: Error: invalid register list at operand 1 -- `ld1b {z0\.b,z4\.b,z5\.b,z6\.b},pn8/z,\[x0\]'
[^ :]+:[0-9]+: Error: invalid register list at operand 1 -- `ld1b {z0\.b,z4\.b,z8\.b,z9\.b},pn8/z,\[x0\]'
[^ :]+:[0-9]+: Error: invalid register list at operand 1 -- `ld1b {z0\.b,z1\.b,z3\.b,z7\.b},pn8/z,\[x0\]'
[^ :]+:[0-9]+: Error: type mismatch in vector register list at operand 1 -- `ld1b {z0\.h,z4\.h,z8\.b,z12\.b},pn8/z,\[x0\]'
[^ :]+:[0-9]+: Error: type mismatch in vector register list at operand 1 -- `ld1b {z0\.b,z4\.b,z8\.h,z12\.h},pn8/z,\[x0\]'
[^ :]+:[0-9]+: Error: operand mismatch -- `ld1b {z0\.h,z4\.h,z8\.h,z12\.h},pn8/z,\[x0\]'
[^ :]+:[0-9]+: Info:    did you mean this\?
[^ :]+:[0-9]+: Info:    	ld1b {z0\.b, z4\.b, z8\.b, z12\.b}, pn8/z, \[x0\]
[^ :]+:[0-9]+: Error: expected a single-register list at operand 1 -- `ld1b {z0\.b,z4\.b,z8\.b,z12\.b},p8/z,\[x0\]'
[^ :]+:[0-9]+: Error: expected a 64-bit base register at operand 3 -- `ld1b {z0\.b,z4\.b,z8\.b,z12\.b},pn8/z,\[w0\]'
[^ :]+:[0-9]+: Error: invalid base register at operand 3 -- `ld1b {z0\.b,z4\.b,z8\.b,z12\.b},pn8/z,\[xzr\]'
[^ :]+:[0-9]+: Error: invalid addressing mode at operand 3 -- `ld1b {z0\.b,z4\.b,z8\.b,z12\.b},pn8/z,\[x0,#-32\]'
[^ :]+:[0-9]+: Error: invalid addressing mode at operand 3 -- `ld1b {z0\.b,z4\.b,z8\.b,z12\.b},pn8/z,\[x0,#-1\]'
[^ :]+:[0-9]+: Error: immediate offset out of range -32 to 28 at operand 3 -- `ld1b {z0\.b,z4\.b,z8\.b,z12\.b},pn8/z,\[x0,#-64,mul vl\]'
[^ :]+:[0-9]+: Error: immediate offset out of range -32 to 28 at operand 3 -- `ld1b {z0\.b,z4\.b,z8\.b,z12\.b},pn8/z,\[x0,#-36,mul vl\]'
[^ :]+:[0-9]+: Error: immediate value must be a multiple of 4 at operand 3 -- `ld1b {z0\.b,z4\.b,z8\.b,z12\.b},pn8/z,\[x0,#-31,mul vl\]'
[^ :]+:[0-9]+: Error: immediate value must be a multiple of 4 at operand 3 -- `ld1b {z0\.b,z4\.b,z8\.b,z12\.b},pn8/z,\[x0,#-3,mul vl\]'
[^ :]+:[0-9]+: Error: immediate value must be a multiple of 4 at operand 3 -- `ld1b {z0\.b,z4\.b,z8\.b,z12\.b},pn8/z,\[x0,#-2,mul vl\]'
[^ :]+:[0-9]+: Error: immediate value must be a multiple of 4 at operand 3 -- `ld1b {z0\.b,z4\.b,z8\.b,z12\.b},pn8/z,\[x0,#-1,mul vl\]'
[^ :]+:[0-9]+: Error: immediate value must be a multiple of 4 at operand 3 -- `ld1b {z0\.b,z4\.b,z8\.b,z12\.b},pn8/z,\[x0,#1,mul vl\]'
[^ :]+:[0-9]+: Error: immediate value must be a multiple of 4 at operand 3 -- `ld1b {z0\.b,z4\.b,z8\.b,z12\.b},pn8/z,\[x0,#2,mul vl\]'
[^ :]+:[0-9]+: Error: immediate value must be a multiple of 4 at operand 3 -- `ld1b {z0\.b,z4\.b,z8\.b,z12\.b},pn8/z,\[x0,#3,mul vl\]'
[^ :]+:[0-9]+: Error: immediate value must be a multiple of 4 at operand 3 -- `ld1b {z0\.b,z4\.b,z8\.b,z12\.b},pn8/z,\[x0,#25,mul vl\]'
[^ :]+:[0-9]+: Error: immediate value must be a multiple of 4 at operand 3 -- `ld1b {z0\.b,z4\.b,z8\.b,z12\.b},pn8/z,\[x0,#26,mul vl\]'
[^ :]+:[0-9]+: Error: immediate value must be a multiple of 4 at operand 3 -- `ld1b {z0\.b,z4\.b,z8\.b,z12\.b},pn8/z,\[x0,#27,mul vl\]'
[^ :]+:[0-9]+: Error: immediate offset out of range -32 to 28 at operand 3 -- `ld1b {z0\.b,z4\.b,z8\.b,z12\.b},pn8/z,\[x0,#29,mul vl\]'
[^ :]+:[0-9]+: Error: immediate offset out of range -32 to 28 at operand 3 -- `ld1b {z0\.b,z4\.b,z8\.b,z12\.b},pn8/z,\[x0,#32,mul vl\]'
[^ :]+:[0-9]+: Error: expected a list of 2 or 4 registers at operand 1 -- `ld1b {z0\.b-z2\.b},pn8/z,\[x0,x1\]'
[^ :]+:[0-9]+: Error: start register out of range at operand 1 -- `ld1b {z1\.b-z2\.b},pn8/z,\[x0,x1\]'
[^ :]+:[0-9]+: Error: expected a single-register list at operand 1 -- `ld1b {z0\.b-z1\.b},p8/z,\[x0,x1\]'
[^ :]+:[0-9]+: Error: operand mismatch -- `ld1b {z0\.b-z1\.b},pn8,\[x0,x1\]'
[^ :]+:[0-9]+: Info:    did you mean this\?
[^ :]+:[0-9]+: Info:    	ld1b {z0\.b-z1\.b}, pn8/z, \[x0, x1\]
[^ :]+:[0-9]+: Error: operand mismatch -- `ld1b {z0\.b-z1\.b},pn8/m,\[x0,x1\]'
[^ :]+:[0-9]+: Info:    did you mean this\?
[^ :]+:[0-9]+: Info:    	ld1b {z0\.b-z1\.b}, pn8/z, \[x0, x1\]
[^ :]+:[0-9]+: Error: operand mismatch -- `ld1b {z0\.b-z1\.b},pn8\.b,\[x0,x1\]'
[^ :]+:[0-9]+: Info:    did you mean this\?
[^ :]+:[0-9]+: Info:    	ld1b {z0\.b-z1\.b}, pn8/z, \[x0, x1\]
[^ :]+:[0-9]+: Error: pn8-pn15 expected at operand 2 -- `ld1b {z0\.b-z1\.b},pn0/z,\[x0,x1\]'
[^ :]+:[0-9]+: Error: pn8-pn15 expected at operand 2 -- `ld1b {z0\.b-z1\.b},pn7/z,\[x0,x1\]'
[^ :]+:[0-9]+: Error: expected a 64-bit base register at operand 3 -- `ld1b {z0\.b-z1\.b},pn8/z,\[w0,w1\]'
[^ :]+:[0-9]+: Error: invalid base register at operand 3 -- `ld1b {z0\.b-z1\.b},pn8/z,\[xzr,x1\]'
[^ :]+:[0-9]+: Error: invalid offset register at operand 3 -- `ld1b {z0\.b-z1\.b},pn8/z,\[sp,sp\]'
[^ :]+:[0-9]+: Error: invalid addressing mode at operand 3 -- `ld1b {z0\.b-z1\.b},pn8/z,\[x0,x1,lsl#1\]'
[^ :]+:[0-9]+: Error: invalid addressing mode at operand 3 -- `ld1b {z0\.b-z1\.b},pn8/z,\[x0,x1,lsl#2\]'
[^ :]+:[0-9]+: Error: invalid addressing mode at operand 3 -- `ld1b {z0\.b-z1\.b},pn8/z,\[x0,x1,lsl#3\]'
[^ :]+:[0-9]+: Error: invalid addressing mode at operand 3 -- `ld1b {z0\.b-z1\.b},pn8/z,\[x0,x1,lsl#4\]'
[^ :]+:[0-9]+: Error: invalid addressing mode at operand 3 -- `ld1b {z0\.b-z1\.b},pn8/z,\[x0,w1,sxtw\]'
[^ :]+:[0-9]+: Error: invalid addressing mode at operand 3 -- `ld1b {z0\.b-z1\.b},pn8/z,\[x0,w1,uxtw\]'
[^ :]+:[0-9]+: Error: start register out of range at operand 1 -- `ld1b {z1\.b-z4\.b},pn8/z,\[x0,x1\]'
[^ :]+:[0-9]+: Error: start register out of range at operand 1 -- `ld1b {z2\.b-z5\.b},pn8/z,\[x0,x1\]'
[^ :]+:[0-9]+: Error: start register out of range at operand 1 -- `ld1b {z3\.b-z6\.b},pn8/z,\[x0,x1\]'
[^ :]+:[0-9]+: Error: expected a single-register list at operand 1 -- `ld1b {z0\.b-z3\.b},p8/z,\[x0,x1\]'
[^ :]+:[0-9]+: Error: operand mismatch -- `ld1b {z0\.b-z3\.b},pn8,\[x0,x1\]'
[^ :]+:[0-9]+: Info:    did you mean this\?
[^ :]+:[0-9]+: Info:    	ld1b {z0\.b-z3\.b}, pn8/z, \[x0, x1\]
[^ :]+:[0-9]+: Error: operand mismatch -- `ld1b {z0\.b-z3\.b},pn8/m,\[x0,x1\]'
[^ :]+:[0-9]+: Info:    did you mean this\?
[^ :]+:[0-9]+: Info:    	ld1b {z0\.b-z3\.b}, pn8/z, \[x0, x1\]
[^ :]+:[0-9]+: Error: operand mismatch -- `ld1b {z0\.b-z3\.b},pn8\.b,\[x0,x1\]'
[^ :]+:[0-9]+: Info:    did you mean this\?
[^ :]+:[0-9]+: Info:    	ld1b {z0\.b-z3\.b}, pn8/z, \[x0, x1\]
[^ :]+:[0-9]+: Error: pn8-pn15 expected at operand 2 -- `ld1b {z0\.b-z3\.b},pn0/z,\[x0,x1\]'
[^ :]+:[0-9]+: Error: pn8-pn15 expected at operand 2 -- `ld1b {z0\.b-z3\.b},pn7/z,\[x0,x1\]'
[^ :]+:[0-9]+: Error: expected a 64-bit base register at operand 3 -- `ld1b {z0\.b-z3\.b},pn8/z,\[w0,w1\]'
[^ :]+:[0-9]+: Error: invalid base register at operand 3 -- `ld1b {z0\.b-z3\.b},pn8/z,\[xzr,x1\]'
[^ :]+:[0-9]+: Error: invalid offset register at operand 3 -- `ld1b {z0\.b-z3\.b},pn8/z,\[sp,sp\]'
[^ :]+:[0-9]+: Error: invalid addressing mode at operand 3 -- `ld1b {z0\.b-z3\.b},pn8/z,\[x0,x1,lsl#1\]'
[^ :]+:[0-9]+: Error: invalid addressing mode at operand 3 -- `ld1b {z0\.b-z3\.b},pn8/z,\[x0,x1,lsl#2\]'
[^ :]+:[0-9]+: Error: invalid addressing mode at operand 3 -- `ld1b {z0\.b-z3\.b},pn8/z,\[x0,x1,lsl#3\]'
[^ :]+:[0-9]+: Error: invalid addressing mode at operand 3 -- `ld1b {z0\.b-z3\.b},pn8/z,\[x0,x1,lsl#4\]'
[^ :]+:[0-9]+: Error: invalid addressing mode at operand 3 -- `ld1b {z0\.b-z3\.b},pn8/z,\[x0,w1,sxtw\]'
[^ :]+:[0-9]+: Error: invalid addressing mode at operand 3 -- `ld1b {z0\.b-z3\.b},pn8/z,\[x0,w1,uxtw\]'
[^ :]+:[0-9]+: Error: the register list must have a stride of 1 or 8 at operand 1 -- `ld1b {z0\.b,z2\.b},pn8/z,\[x0,x1\]`
[^ :]+:[0-9]+: Error: the register list must have a stride of 1 or 8 at operand 1 -- `ld1b {z0\.b,z3\.b},pn8/z,\[x0,x1\]`
[^ :]+:[0-9]+: Error: the register list must have a stride of 1 or 8 at operand 1 -- `ld1b {z0\.b,z4\.b},pn8/z,\[x0,x1\]`
[^ :]+:[0-9]+: Error: start register out of range at operand 1 -- `ld1b {z8\.b,z16\.b},pn8/z,\[x0,x1\]'
[^ :]+:[0-9]+: Error: start register out of range at operand 1 -- `ld1b {z24\.b,z0\.b},pn8/z,\[x0,x1\]'
[^ :]+:[0-9]+: Error: the register list must have a stride of 1 or 8 at operand 1 -- `ld1b {z8\.b,z0\.b},pn8/z,\[x0,x1\]`
[^ :]+:[0-9]+: Error: operand mismatch -- `ld1b {z0\.h,z8\.h},pn8/z,\[x0,x1\]'
[^ :]+:[0-9]+: Info:    did you mean this\?
[^ :]+:[0-9]+: Info:    	ld1b {z0\.b, z8\.b}, pn8/z, \[x0, x1\]
[^ :]+:[0-9]+: Error: expected a single-register list at operand 1 -- `ld1b {z0\.b,z8\.b},p8/z,\[x0,x1\]'
[^ :]+:[0-9]+: Error: expected a 64-bit base register at operand 3 -- `ld1b {z0\.b,z8\.b},pn8/z,\[w0,w30\]'
[^ :]+:[0-9]+: Error: invalid base register at operand 3 -- `ld1b {z0\.b,z8\.b},pn8/z,\[xzr,xzr\]'
[^ :]+:[0-9]+: Error: invalid offset register at operand 3 -- `ld1b {z0\.b,z8\.b},pn8/z,\[x0,sp\]'
[^ :]+:[0-9]+: Error: invalid addressing mode at operand 3 -- `ld1b {z0\.b,z8\.b},pn8/z,\[x0,x1,lsl#1\]'
[^ :]+:[0-9]+: Error: start register out of range at operand 1 -- `ld1b {z4\.b,z8\.b,z12\.b,z16\.b},pn8/z,\[x0,x1\]'
[^ :]+:[0-9]+: Error: start register out of range at operand 1 -- `ld1b {z20\.b,z24\.b,z28\.b,z0\.b},pn8/z,\[x0,x1\]'
[^ :]+:[0-9]+: Error: operand mismatch -- `ld1b {z0\.h,z4\.h,z8\.h,z12\.h},pn8/z,\[x0,x1\]'
[^ :]+:[0-9]+: Info:    did you mean this\?
[^ :]+:[0-9]+: Info:    	ld1b {z0\.b, z4\.b, z8\.b, z12\.b}, pn8/z, \[x0, x1\]
[^ :]+:[0-9]+: Error: expected a single-register list at operand 1 -- `ld1b {z0\.b,z4\.b,z8\.b,z12\.b},p8/z,\[x0,x1\]'
[^ :]+:[0-9]+: Error: expected a 64-bit base register at operand 3 -- `ld1b {z0\.b,z4\.b,z8\.b,z12\.b},pn8/z,\[w0,w30\]'
[^ :]+:[0-9]+: Error: invalid base register at operand 3 -- `ld1b {z0\.b,z4\.b,z8\.b,z12\.b},pn8/z,\[xzr,xzr\]'
[^ :]+:[0-9]+: Error: invalid offset register at operand 3 -- `ld1b {z0\.b,z4\.b,z8\.b,z12\.b},pn8/z,\[x0,sp\]'
[^ :]+:[0-9]+: Error: invalid addressing mode at operand 3 -- `ld1b {z0\.b,z4\.b,z8\.b,z12\.b},pn8/z,\[x0,x1,lsl#1\]'
