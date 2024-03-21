[^ :]+: Assembler messages:
[^ :]+:[0-9]+: Error: expected a register or register list at operand 1 -- `ld1d 0,pn8/z,\[x0\]'
[^ :]+:[0-9]+: Error: expected an SVE predicate register at operand 2 -- `ld1d {z0\.d-z1\.d},0,\[x0\]'
[^ :]+:[0-9]+: Error: invalid addressing mode at operand 3 -- `ld1d {z0\.d-z1\.d},pn8/z,0'
[^ :]+:[0-9]+: Error: expected a list of 2 or 4 registers at operand 1 -- `ld1d {z0\.d-z2\.d},pn8/z,\[x0,x1,lsl#3\]'
[^ :]+:[0-9]+: Error: start register out of range at operand 1 -- `ld1d {z1\.d-z2\.d},pn8/z,\[x0,x1,lsl#3\]'
[^ :]+:[0-9]+: Error: expected a single-register list at operand 1 -- `ld1d {z0\.d-z1\.d},p8/z,\[x0,x1,lsl#3\]'
[^ :]+:[0-9]+: Error: operand mismatch -- `ld1d {z0\.d-z1\.d},pn8,\[x0,x1,lsl#3\]'
[^ :]+:[0-9]+: Info:    did you mean this\?
[^ :]+:[0-9]+: Info:    	ld1d {z0\.d-z1\.d}, pn8/z, \[x0, x1, lsl #3\]
[^ :]+:[0-9]+: Error: operand mismatch -- `ld1d {z0\.d-z1\.d},pn8/m,\[x0,x1,lsl#3\]'
[^ :]+:[0-9]+: Info:    did you mean this\?
[^ :]+:[0-9]+: Info:    	ld1d {z0\.d-z1\.d}, pn8/z, \[x0, x1, lsl #3\]
[^ :]+:[0-9]+: Error: operand mismatch -- `ld1d {z0\.d-z1\.d},pn8\.d,\[x0,x1,lsl#3\]'
[^ :]+:[0-9]+: Info:    did you mean this\?
[^ :]+:[0-9]+: Info:    	ld1d {z0\.d-z1\.d}, pn8/z, \[x0, x1, lsl #3\]
[^ :]+:[0-9]+: Error: pn8-pn15 expected at operand 2 -- `ld1d {z0\.d-z1\.d},pn0/z,\[x0,x1,lsl#3\]'
[^ :]+:[0-9]+: Error: pn8-pn15 expected at operand 2 -- `ld1d {z0\.d-z1\.d},pn7/z,\[x0,x1,lsl#3\]'
[^ :]+:[0-9]+: Error: expected a 64-bit base register at operand 3 -- `ld1d {z0\.d-z1\.d},pn8/z,\[w0,w1,lsl#3\]'
[^ :]+:[0-9]+: Error: invalid base register at operand 3 -- `ld1d {z0\.d-z1\.d},pn8/z,\[xzr,x1,lsl#3\]'
[^ :]+:[0-9]+: Error: invalid offset register at operand 3 -- `ld1d {z0\.d-z1\.d},pn8/z,\[sp,sp,lsl#3\]'
[^ :]+:[0-9]+: Error: invalid addressing mode at operand 3 -- `ld1d {z0\.d-z1\.d},pn8/z,\[x0,x1\]'
[^ :]+:[0-9]+: Error: invalid addressing mode at operand 3 -- `ld1d {z0\.d-z1\.d},pn8/z,\[x0,x1,lsl#1\]'
[^ :]+:[0-9]+: Error: invalid addressing mode at operand 3 -- `ld1d {z0\.d-z1\.d},pn8/z,\[x0,x1,lsl#2\]'
[^ :]+:[0-9]+: Error: invalid addressing mode at operand 3 -- `ld1d {z0\.d-z1\.d},pn8/z,\[x0,x1,lsl#4\]'
[^ :]+:[0-9]+: Error: invalid addressing mode at operand 3 -- `ld1d {z0\.d-z1\.d},pn8/z,\[x0,w1,sxtw#3\]'
[^ :]+:[0-9]+: Error: invalid addressing mode at operand 3 -- `ld1d {z0\.d-z1\.d},pn8/z,\[x0,w1,uxtw#3\]'
[^ :]+:[0-9]+: Error: start register out of range at operand 1 -- `ld1d {z1\.d-z4\.d},pn8/z,\[x0,x1,lsl#3\]'
[^ :]+:[0-9]+: Error: start register out of range at operand 1 -- `ld1d {z2\.d-z5\.d},pn8/z,\[x0,x1,lsl#3\]'
[^ :]+:[0-9]+: Error: start register out of range at operand 1 -- `ld1d {z3\.d-z6\.d},pn8/z,\[x0,x1,lsl#3\]'
[^ :]+:[0-9]+: Error: expected a single-register list at operand 1 -- `ld1d {z0\.d-z3\.d},p8/z,\[x0,x1,lsl#3\]'
[^ :]+:[0-9]+: Error: operand mismatch -- `ld1d {z0\.d-z3\.d},pn8,\[x0,x1,lsl#3\]'
[^ :]+:[0-9]+: Info:    did you mean this\?
[^ :]+:[0-9]+: Info:    	ld1d {z0\.d-z3\.d}, pn8/z, \[x0, x1, lsl #3\]
[^ :]+:[0-9]+: Error: operand mismatch -- `ld1d {z0\.d-z3\.d},pn8/m,\[x0,x1,lsl#3\]'
[^ :]+:[0-9]+: Info:    did you mean this\?
[^ :]+:[0-9]+: Info:    	ld1d {z0\.d-z3\.d}, pn8/z, \[x0, x1, lsl #3\]
[^ :]+:[0-9]+: Error: operand mismatch -- `ld1d {z0\.d-z3\.d},pn8\.d,\[x0,x1,lsl#3\]'
[^ :]+:[0-9]+: Info:    did you mean this\?
[^ :]+:[0-9]+: Info:    	ld1d {z0\.d-z3\.d}, pn8/z, \[x0, x1, lsl #3\]
[^ :]+:[0-9]+: Error: pn8-pn15 expected at operand 2 -- `ld1d {z0\.d-z3\.d},pn0/z,\[x0,x1,lsl#3\]'
[^ :]+:[0-9]+: Error: pn8-pn15 expected at operand 2 -- `ld1d {z0\.d-z3\.d},pn7/z,\[x0,x1,lsl#3\]'
[^ :]+:[0-9]+: Error: expected a 64-bit base register at operand 3 -- `ld1d {z0\.d-z3\.d},pn8/z,\[w0,w1,lsl#3\]'
[^ :]+:[0-9]+: Error: invalid base register at operand 3 -- `ld1d {z0\.d-z3\.d},pn8/z,\[xzr,x1,lsl#3\]'
[^ :]+:[0-9]+: Error: invalid offset register at operand 3 -- `ld1d {z0\.d-z3\.d},pn8/z,\[sp,sp,lsl#3\]'
[^ :]+:[0-9]+: Error: invalid addressing mode at operand 3 -- `ld1d {z0\.d-z3\.d},pn8/z,\[x0,x1\]'
[^ :]+:[0-9]+: Error: invalid addressing mode at operand 3 -- `ld1d {z0\.d-z3\.d},pn8/z,\[x0,x1,lsl#1\]'
[^ :]+:[0-9]+: Error: invalid addressing mode at operand 3 -- `ld1d {z0\.d-z3\.d},pn8/z,\[x0,x1,lsl#2\]'
[^ :]+:[0-9]+: Error: invalid addressing mode at operand 3 -- `ld1d {z0\.d-z3\.d},pn8/z,\[x0,x1,lsl#4\]'
[^ :]+:[0-9]+: Error: invalid addressing mode at operand 3 -- `ld1d {z0\.d-z3\.d},pn8/z,\[x0,w1,sxtw#3\]'
[^ :]+:[0-9]+: Error: invalid addressing mode at operand 3 -- `ld1d {z0\.d-z3\.d},pn8/z,\[x0,w1,uxtw#3\]'
[^ :]+:[0-9]+: Error: the register list must have a stride of 1 or 8 at operand 1 -- `ld1d {z0\.d,z2\.d},pn8/z,\[x0,x1,lsl#3\]`
[^ :]+:[0-9]+: Error: the register list must have a stride of 1 or 8 at operand 1 -- `ld1d {z0\.d,z3\.d},pn8/z,\[x0,x1,lsl#3\]`
[^ :]+:[0-9]+: Error: the register list must have a stride of 1 or 8 at operand 1 -- `ld1d {z0\.d,z4\.d},pn8/z,\[x0,x1,lsl#3\]`
[^ :]+:[0-9]+: Error: start register out of range at operand 1 -- `ld1d {z8\.d,z16\.d},pn8/z,\[x0,x1,lsl#3\]'
[^ :]+:[0-9]+: Error: start register out of range at operand 1 -- `ld1d {z24\.d,z0\.d},pn8/z,\[x0,x1,lsl#3\]'
[^ :]+:[0-9]+: Error: the register list must have a stride of 1 or 8 at operand 1 -- `ld1d {z8\.d,z0\.d},pn8/z,\[x0,x1,lsl#3\]`
[^ :]+:[0-9]+: Error: operand mismatch -- `ld1d {z0\.h,z8\.h},pn8/z,\[x0,x1,lsl#3\]'
[^ :]+:[0-9]+: Info:    did you mean this\?
[^ :]+:[0-9]+: Info:    	ld1d {z0\.d, z8\.d}, pn8/z, \[x0, x1, lsl #3\]
[^ :]+:[0-9]+: Error: expected a single-register list at operand 1 -- `ld1d {z0\.d,z8\.d},p8/z,\[x0,x1,lsl#3\]'
[^ :]+:[0-9]+: Error: expected a 64-bit base register at operand 3 -- `ld1d {z0\.d,z8\.d},pn8/z,\[w0,w30,lsl#3\]'
[^ :]+:[0-9]+: Error: invalid base register at operand 3 -- `ld1d {z0\.d,z8\.d},pn8/z,\[xzr,xzr,lsl#3\]'
[^ :]+:[0-9]+: Error: invalid offset register at operand 3 -- `ld1d {z0\.d,z8\.d},pn8/z,\[x0,sp,lsl#3\]'
[^ :]+:[0-9]+: Error: invalid addressing mode at operand 3 -- `ld1d {z0\.d,z8\.d},pn8/z,\[x0,x1,lsl#1\]'
[^ :]+:[0-9]+: Error: start register out of range at operand 1 -- `ld1d {z4\.d,z8\.d,z12\.d,z16\.d},pn8/z,\[x0,x1,lsl#3\]'
[^ :]+:[0-9]+: Error: start register out of range at operand 1 -- `ld1d {z20\.d,z24\.d,z28\.d,z0\.d},pn8/z,\[x0,x1,lsl#3\]'
[^ :]+:[0-9]+: Error: operand mismatch -- `ld1d {z0\.h,z4\.h,z8\.h,z12\.h},pn8/z,\[x0,x1,lsl#3\]'
[^ :]+:[0-9]+: Info:    did you mean this\?
[^ :]+:[0-9]+: Info:    	ld1d {z0\.d, z4\.d, z8\.d, z12\.d}, pn8/z, \[x0, x1, lsl #3\]
[^ :]+:[0-9]+: Error: expected a single-register list at operand 1 -- `ld1d {z0\.d,z4\.d,z8\.d,z12\.d},p8/z,\[x0,x1,lsl#3\]'
[^ :]+:[0-9]+: Error: expected a 64-bit base register at operand 3 -- `ld1d {z0\.d,z4\.d,z8\.d,z12\.d},pn8/z,\[w0,w30,lsl#3\]'
[^ :]+:[0-9]+: Error: invalid base register at operand 3 -- `ld1d {z0\.d,z4\.d,z8\.d,z12\.d},pn8/z,\[xzr,xzr,lsl#3\]'
[^ :]+:[0-9]+: Error: invalid offset register at operand 3 -- `ld1d {z0\.d,z4\.d,z8\.d,z12\.d},pn8/z,\[x0,sp,lsl#3\]'
[^ :]+:[0-9]+: Error: invalid addressing mode at operand 3 -- `ld1d {z0\.d,z4\.d,z8\.d,z12\.d},pn8/z,\[x0,x1,lsl#1\]'
