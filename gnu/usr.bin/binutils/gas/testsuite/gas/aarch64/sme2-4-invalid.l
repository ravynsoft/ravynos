[^ :]+: Assembler messages:
[^ :]+:[0-9]+: Error: expected a register or register list at operand 1 -- `ld1h 0,pn8/z,\[x0\]'
[^ :]+:[0-9]+: Error: expected an SVE predicate register at operand 2 -- `ld1h {z0\.h-z1\.h},0,\[x0\]'
[^ :]+:[0-9]+: Error: invalid addressing mode at operand 3 -- `ld1h {z0\.h-z1\.h},pn8/z,0'
[^ :]+:[0-9]+: Error: expected a list of 2 or 4 registers at operand 1 -- `ld1h {z0\.h-z2\.h},pn8/z,\[x0,x1,lsl#1\]'
[^ :]+:[0-9]+: Error: start register out of range at operand 1 -- `ld1h {z1\.h-z2\.h},pn8/z,\[x0,x1,lsl#1\]'
[^ :]+:[0-9]+: Error: expected a single-register list at operand 1 -- `ld1h {z0\.h-z1\.h},p8/z,\[x0,x1,lsl#1\]'
[^ :]+:[0-9]+: Error: operand mismatch -- `ld1h {z0\.h-z1\.h},pn8,\[x0,x1,lsl#1\]'
[^ :]+:[0-9]+: Info:    did you mean this\?
[^ :]+:[0-9]+: Info:    	ld1h {z0\.h-z1\.h}, pn8/z, \[x0, x1, lsl #1\]
[^ :]+:[0-9]+: Error: operand mismatch -- `ld1h {z0\.h-z1\.h},pn8/m,\[x0,x1,lsl#1\]'
[^ :]+:[0-9]+: Info:    did you mean this\?
[^ :]+:[0-9]+: Info:    	ld1h {z0\.h-z1\.h}, pn8/z, \[x0, x1, lsl #1\]
[^ :]+:[0-9]+: Error: operand mismatch -- `ld1h {z0\.h-z1\.h},pn8\.h,\[x0,x1,lsl#1\]'
[^ :]+:[0-9]+: Info:    did you mean this\?
[^ :]+:[0-9]+: Info:    	ld1h {z0\.h-z1\.h}, pn8/z, \[x0, x1, lsl #1\]
[^ :]+:[0-9]+: Error: pn8-pn15 expected at operand 2 -- `ld1h {z0\.h-z1\.h},pn0/z,\[x0,x1,lsl#1\]'
[^ :]+:[0-9]+: Error: pn8-pn15 expected at operand 2 -- `ld1h {z0\.h-z1\.h},pn7/z,\[x0,x1,lsl#1\]'
[^ :]+:[0-9]+: Error: expected a 64-bit base register at operand 3 -- `ld1h {z0\.h-z1\.h},pn8/z,\[w0,w1,lsl#1\]'
[^ :]+:[0-9]+: Error: invalid base register at operand 3 -- `ld1h {z0\.h-z1\.h},pn8/z,\[xzr,x1,lsl#1\]'
[^ :]+:[0-9]+: Error: invalid offset register at operand 3 -- `ld1h {z0\.h-z1\.h},pn8/z,\[sp,sp,lsl#1\]'
[^ :]+:[0-9]+: Error: invalid addressing mode at operand 3 -- `ld1h {z0\.h-z1\.h},pn8/z,\[x0,x1\]'
[^ :]+:[0-9]+: Error: invalid addressing mode at operand 3 -- `ld1h {z0\.h-z1\.h},pn8/z,\[x0,x1,lsl#2\]'
[^ :]+:[0-9]+: Error: invalid addressing mode at operand 3 -- `ld1h {z0\.h-z1\.h},pn8/z,\[x0,x1,lsl#3\]'
[^ :]+:[0-9]+: Error: invalid addressing mode at operand 3 -- `ld1h {z0\.h-z1\.h},pn8/z,\[x0,x1,lsl#4\]'
[^ :]+:[0-9]+: Error: invalid addressing mode at operand 3 -- `ld1h {z0\.h-z1\.h},pn8/z,\[x0,w1,sxtw#1\]'
[^ :]+:[0-9]+: Error: invalid addressing mode at operand 3 -- `ld1h {z0\.h-z1\.h},pn8/z,\[x0,w1,uxtw#1\]'
[^ :]+:[0-9]+: Error: start register out of range at operand 1 -- `ld1h {z1\.h-z4\.h},pn8/z,\[x0,x1,lsl#1\]'
[^ :]+:[0-9]+: Error: start register out of range at operand 1 -- `ld1h {z2\.h-z5\.h},pn8/z,\[x0,x1,lsl#1\]'
[^ :]+:[0-9]+: Error: start register out of range at operand 1 -- `ld1h {z3\.h-z6\.h},pn8/z,\[x0,x1,lsl#1\]'
[^ :]+:[0-9]+: Error: expected a single-register list at operand 1 -- `ld1h {z0\.h-z3\.h},p8/z,\[x0,x1,lsl#1\]'
[^ :]+:[0-9]+: Error: operand mismatch -- `ld1h {z0\.h-z3\.h},pn8,\[x0,x1,lsl#1\]'
[^ :]+:[0-9]+: Info:    did you mean this\?
[^ :]+:[0-9]+: Info:    	ld1h {z0\.h-z3\.h}, pn8/z, \[x0, x1, lsl #1\]
[^ :]+:[0-9]+: Error: operand mismatch -- `ld1h {z0\.h-z3\.h},pn8/m,\[x0,x1,lsl#1\]'
[^ :]+:[0-9]+: Info:    did you mean this\?
[^ :]+:[0-9]+: Info:    	ld1h {z0\.h-z3\.h}, pn8/z, \[x0, x1, lsl #1\]
[^ :]+:[0-9]+: Error: operand mismatch -- `ld1h {z0\.h-z3\.h},pn8\.h,\[x0,x1,lsl#1\]'
[^ :]+:[0-9]+: Info:    did you mean this\?
[^ :]+:[0-9]+: Info:    	ld1h {z0\.h-z3\.h}, pn8/z, \[x0, x1, lsl #1\]
[^ :]+:[0-9]+: Error: pn8-pn15 expected at operand 2 -- `ld1h {z0\.h-z3\.h},pn0/z,\[x0,x1,lsl#1\]'
[^ :]+:[0-9]+: Error: pn8-pn15 expected at operand 2 -- `ld1h {z0\.h-z3\.h},pn7/z,\[x0,x1,lsl#1\]'
[^ :]+:[0-9]+: Error: expected a 64-bit base register at operand 3 -- `ld1h {z0\.h-z3\.h},pn8/z,\[w0,w1,lsl#1\]'
[^ :]+:[0-9]+: Error: invalid base register at operand 3 -- `ld1h {z0\.h-z3\.h},pn8/z,\[xzr,x1,lsl#1\]'
[^ :]+:[0-9]+: Error: invalid offset register at operand 3 -- `ld1h {z0\.h-z3\.h},pn8/z,\[sp,sp,lsl#1\]'
[^ :]+:[0-9]+: Error: invalid addressing mode at operand 3 -- `ld1h {z0\.h-z3\.h},pn8/z,\[x0,x1\]'
[^ :]+:[0-9]+: Error: invalid addressing mode at operand 3 -- `ld1h {z0\.h-z3\.h},pn8/z,\[x0,x1,lsl#2\]'
[^ :]+:[0-9]+: Error: invalid addressing mode at operand 3 -- `ld1h {z0\.h-z3\.h},pn8/z,\[x0,x1,lsl#3\]'
[^ :]+:[0-9]+: Error: invalid addressing mode at operand 3 -- `ld1h {z0\.h-z3\.h},pn8/z,\[x0,x1,lsl#4\]'
[^ :]+:[0-9]+: Error: invalid addressing mode at operand 3 -- `ld1h {z0\.h-z3\.h},pn8/z,\[x0,w1,sxtw#1\]'
[^ :]+:[0-9]+: Error: invalid addressing mode at operand 3 -- `ld1h {z0\.h-z3\.h},pn8/z,\[x0,w1,uxtw#1\]'
[^ :]+:[0-9]+: Error: the register list must have a stride of 1 or 8 at operand 1 -- `ld1h {z0\.h,z2\.h},pn8/z,\[x0,x1,lsl#1\]`
[^ :]+:[0-9]+: Error: the register list must have a stride of 1 or 8 at operand 1 -- `ld1h {z0\.h,z3\.h},pn8/z,\[x0,x1,lsl#1\]`
[^ :]+:[0-9]+: Error: the register list must have a stride of 1 or 8 at operand 1 -- `ld1h {z0\.h,z4\.h},pn8/z,\[x0,x1,lsl#1\]`
[^ :]+:[0-9]+: Error: start register out of range at operand 1 -- `ld1h {z8\.h,z16\.h},pn8/z,\[x0,x1,lsl#1\]'
[^ :]+:[0-9]+: Error: start register out of range at operand 1 -- `ld1h {z24\.h,z0\.h},pn8/z,\[x0,x1,lsl#1\]'
[^ :]+:[0-9]+: Error: the register list must have a stride of 1 or 8 at operand 1 -- `ld1h {z8\.h,z0\.h},pn8/z,\[x0,x1,lsl#1\]`
[^ :]+:[0-9]+: Error: operand mismatch -- `ld1h {z0\.d,z8\.d},pn8/z,\[x0,x1,lsl#1\]'
[^ :]+:[0-9]+: Info:    did you mean this\?
[^ :]+:[0-9]+: Info:    	ld1h {z0\.h, z8\.h}, pn8/z, \[x0, x1, lsl #1\]
[^ :]+:[0-9]+: Error: expected a single-register list at operand 1 -- `ld1h {z0\.h,z8\.h},p8/z,\[x0,x1,lsl#1\]'
[^ :]+:[0-9]+: Error: expected a 64-bit base register at operand 3 -- `ld1h {z0\.h,z8\.h},pn8/z,\[w0,w30,lsl#1\]'
[^ :]+:[0-9]+: Error: invalid base register at operand 3 -- `ld1h {z0\.h,z8\.h},pn8/z,\[xzr,xzr,lsl#1\]'
[^ :]+:[0-9]+: Error: invalid offset register at operand 3 -- `ld1h {z0\.h,z8\.h},pn8/z,\[x0,sp,lsl#1\]'
[^ :]+:[0-9]+: Error: invalid addressing mode at operand 3 -- `ld1h {z0\.h,z8\.h},pn8/z,\[x0,x1,lsl#2\]'
[^ :]+:[0-9]+: Error: start register out of range at operand 1 -- `ld1h {z4\.h,z8\.h,z12\.h,z16\.h},pn8/z,\[x0,x1,lsl#1\]'
[^ :]+:[0-9]+: Error: start register out of range at operand 1 -- `ld1h {z20\.h,z24\.h,z28\.h,z0\.h},pn8/z,\[x0,x1,lsl#1\]'
[^ :]+:[0-9]+: Error: operand mismatch -- `ld1h {z0\.d,z4\.d,z8\.d,z12\.d},pn8/z,\[x0,x1,lsl#1\]'
[^ :]+:[0-9]+: Info:    did you mean this\?
[^ :]+:[0-9]+: Info:    	ld1h {z0\.h, z4\.h, z8\.h, z12\.h}, pn8/z, \[x0, x1, lsl #1\]
[^ :]+:[0-9]+: Error: expected a single-register list at operand 1 -- `ld1h {z0\.h,z4\.h,z8\.h,z12\.h},p8/z,\[x0,x1,lsl#1\]'
[^ :]+:[0-9]+: Error: expected a 64-bit base register at operand 3 -- `ld1h {z0\.h,z4\.h,z8\.h,z12\.h},pn8/z,\[w0,w30,lsl#1\]'
[^ :]+:[0-9]+: Error: invalid base register at operand 3 -- `ld1h {z0\.h,z4\.h,z8\.h,z12\.h},pn8/z,\[xzr,xzr,lsl#1\]'
[^ :]+:[0-9]+: Error: invalid offset register at operand 3 -- `ld1h {z0\.h,z4\.h,z8\.h,z12\.h},pn8/z,\[x0,sp,lsl#1\]'
[^ :]+:[0-9]+: Error: invalid addressing mode at operand 3 -- `ld1h {z0\.h,z4\.h,z8\.h,z12\.h},pn8/z,\[x0,x1,lsl#3\]'
