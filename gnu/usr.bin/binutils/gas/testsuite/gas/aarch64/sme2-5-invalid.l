[^ :]+: Assembler messages:
[^ :]+:[0-9]+: Error: expected a register or register list at operand 1 -- `ld1w 0,pn8/z,\[x0\]'
[^ :]+:[0-9]+: Error: expected an SVE predicate register at operand 2 -- `ld1w {z0\.s-z1\.s},0,\[x0\]'
[^ :]+:[0-9]+: Error: invalid addressing mode at operand 3 -- `ld1w {z0\.s-z1\.s},pn8/z,0'
[^ :]+:[0-9]+: Error: expected a list of 2 or 4 registers at operand 1 -- `ld1w {z0\.s-z2\.s},pn8/z,\[x0,x1,lsl#2\]'
[^ :]+:[0-9]+: Error: start register out of range at operand 1 -- `ld1w {z1\.s-z2\.s},pn8/z,\[x0,x1,lsl#2\]'
[^ :]+:[0-9]+: Error: expected a single-register list at operand 1 -- `ld1w {z0\.s-z1\.s},p8/z,\[x0,x1,lsl#2\]'
[^ :]+:[0-9]+: Error: operand mismatch -- `ld1w {z0\.s-z1\.s},pn8,\[x0,x1,lsl#2\]'
[^ :]+:[0-9]+: Info:    did you mean this\?
[^ :]+:[0-9]+: Info:    	ld1w {z0\.s-z1\.s}, pn8/z, \[x0, x1, lsl #2\]
[^ :]+:[0-9]+: Error: operand mismatch -- `ld1w {z0\.s-z1\.s},pn8/m,\[x0,x1,lsl#2\]'
[^ :]+:[0-9]+: Info:    did you mean this\?
[^ :]+:[0-9]+: Info:    	ld1w {z0\.s-z1\.s}, pn8/z, \[x0, x1, lsl #2\]
[^ :]+:[0-9]+: Error: operand mismatch -- `ld1w {z0\.s-z1\.s},pn8\.s,\[x0,x1,lsl#2\]'
[^ :]+:[0-9]+: Info:    did you mean this\?
[^ :]+:[0-9]+: Info:    	ld1w {z0\.s-z1\.s}, pn8/z, \[x0, x1, lsl #2\]
[^ :]+:[0-9]+: Error: pn8-pn15 expected at operand 2 -- `ld1w {z0\.s-z1\.s},pn0/z,\[x0,x1,lsl#2\]'
[^ :]+:[0-9]+: Error: pn8-pn15 expected at operand 2 -- `ld1w {z0\.s-z1\.s},pn7/z,\[x0,x1,lsl#2\]'
[^ :]+:[0-9]+: Error: expected a 64-bit base register at operand 3 -- `ld1w {z0\.s-z1\.s},pn8/z,\[w0,w1,lsl#2\]'
[^ :]+:[0-9]+: Error: invalid base register at operand 3 -- `ld1w {z0\.s-z1\.s},pn8/z,\[xzr,x1,lsl#2\]'
[^ :]+:[0-9]+: Error: invalid offset register at operand 3 -- `ld1w {z0\.s-z1\.s},pn8/z,\[sp,sp,lsl#2\]'
[^ :]+:[0-9]+: Error: invalid addressing mode at operand 3 -- `ld1w {z0\.s-z1\.s},pn8/z,\[x0,x1\]'
[^ :]+:[0-9]+: Error: invalid addressing mode at operand 3 -- `ld1w {z0\.s-z1\.s},pn8/z,\[x0,x1,lsl#1\]'
[^ :]+:[0-9]+: Error: invalid addressing mode at operand 3 -- `ld1w {z0\.s-z1\.s},pn8/z,\[x0,x1,lsl#3\]'
[^ :]+:[0-9]+: Error: invalid addressing mode at operand 3 -- `ld1w {z0\.s-z1\.s},pn8/z,\[x0,x1,lsl#4\]'
[^ :]+:[0-9]+: Error: invalid addressing mode at operand 3 -- `ld1w {z0\.s-z1\.s},pn8/z,\[x0,w1,sxtw#2\]'
[^ :]+:[0-9]+: Error: invalid addressing mode at operand 3 -- `ld1w {z0\.s-z1\.s},pn8/z,\[x0,w1,uxtw#2\]'
[^ :]+:[0-9]+: Error: start register out of range at operand 1 -- `ld1w {z1\.s-z4\.s},pn8/z,\[x0,x1,lsl#2\]'
[^ :]+:[0-9]+: Error: start register out of range at operand 1 -- `ld1w {z2\.s-z5\.s},pn8/z,\[x0,x1,lsl#2\]'
[^ :]+:[0-9]+: Error: start register out of range at operand 1 -- `ld1w {z3\.s-z6\.s},pn8/z,\[x0,x1,lsl#2\]'
[^ :]+:[0-9]+: Error: expected a single-register list at operand 1 -- `ld1w {z0\.s-z3\.s},p8/z,\[x0,x1,lsl#2\]'
[^ :]+:[0-9]+: Error: operand mismatch -- `ld1w {z0\.s-z3\.s},pn8,\[x0,x1,lsl#2\]'
[^ :]+:[0-9]+: Info:    did you mean this\?
[^ :]+:[0-9]+: Info:    	ld1w {z0\.s-z3\.s}, pn8/z, \[x0, x1, lsl #2\]
[^ :]+:[0-9]+: Error: operand mismatch -- `ld1w {z0\.s-z3\.s},pn8/m,\[x0,x1,lsl#2\]'
[^ :]+:[0-9]+: Info:    did you mean this\?
[^ :]+:[0-9]+: Info:    	ld1w {z0\.s-z3\.s}, pn8/z, \[x0, x1, lsl #2\]
[^ :]+:[0-9]+: Error: operand mismatch -- `ld1w {z0\.s-z3\.s},pn8\.s,\[x0,x1,lsl#2\]'
[^ :]+:[0-9]+: Info:    did you mean this\?
[^ :]+:[0-9]+: Info:    	ld1w {z0\.s-z3\.s}, pn8/z, \[x0, x1, lsl #2\]
[^ :]+:[0-9]+: Error: pn8-pn15 expected at operand 2 -- `ld1w {z0\.s-z3\.s},pn0/z,\[x0,x1,lsl#2\]'
[^ :]+:[0-9]+: Error: pn8-pn15 expected at operand 2 -- `ld1w {z0\.s-z3\.s},pn7/z,\[x0,x1,lsl#2\]'
[^ :]+:[0-9]+: Error: expected a 64-bit base register at operand 3 -- `ld1w {z0\.s-z3\.s},pn8/z,\[w0,w1,lsl#2\]'
[^ :]+:[0-9]+: Error: invalid base register at operand 3 -- `ld1w {z0\.s-z3\.s},pn8/z,\[xzr,x1,lsl#2\]'
[^ :]+:[0-9]+: Error: invalid offset register at operand 3 -- `ld1w {z0\.s-z3\.s},pn8/z,\[sp,sp,lsl#2\]'
[^ :]+:[0-9]+: Error: invalid addressing mode at operand 3 -- `ld1w {z0\.s-z3\.s},pn8/z,\[x0,x1\]'
[^ :]+:[0-9]+: Error: invalid addressing mode at operand 3 -- `ld1w {z0\.s-z3\.s},pn8/z,\[x0,x1,lsl#1\]'
[^ :]+:[0-9]+: Error: invalid addressing mode at operand 3 -- `ld1w {z0\.s-z3\.s},pn8/z,\[x0,x1,lsl#3\]'
[^ :]+:[0-9]+: Error: invalid addressing mode at operand 3 -- `ld1w {z0\.s-z3\.s},pn8/z,\[x0,x1,lsl#4\]'
[^ :]+:[0-9]+: Error: invalid addressing mode at operand 3 -- `ld1w {z0\.s-z3\.s},pn8/z,\[x0,w1,sxtw#2\]'
[^ :]+:[0-9]+: Error: invalid addressing mode at operand 3 -- `ld1w {z0\.s-z3\.s},pn8/z,\[x0,w1,uxtw#2\]'
[^ :]+:[0-9]+: Error: the register list must have a stride of 1 or 8 at operand 1 -- `ld1w {z0\.s,z2\.s},pn8/z,\[x0,x1,lsl#2\]`
[^ :]+:[0-9]+: Error: the register list must have a stride of 1 or 8 at operand 1 -- `ld1w {z0\.s,z3\.s},pn8/z,\[x0,x1,lsl#2\]`
[^ :]+:[0-9]+: Error: the register list must have a stride of 1 or 8 at operand 1 -- `ld1w {z0\.s,z4\.s},pn8/z,\[x0,x1,lsl#2\]`
[^ :]+:[0-9]+: Error: start register out of range at operand 1 -- `ld1w {z8\.s,z16\.s},pn8/z,\[x0,x1,lsl#2\]'
[^ :]+:[0-9]+: Error: start register out of range at operand 1 -- `ld1w {z24\.s,z0\.s},pn8/z,\[x0,x1,lsl#2\]'
[^ :]+:[0-9]+: Error: the register list must have a stride of 1 or 8 at operand 1 -- `ld1w {z8\.s,z0\.s},pn8/z,\[x0,x1,lsl#2\]`
[^ :]+:[0-9]+: Error: operand mismatch -- `ld1w {z0\.h,z8\.h},pn8/z,\[x0,x1,lsl#2\]'
[^ :]+:[0-9]+: Info:    did you mean this\?
[^ :]+:[0-9]+: Info:    	ld1w {z0\.s, z8\.s}, pn8/z, \[x0, x1, lsl #2\]
[^ :]+:[0-9]+: Error: expected a single-register list at operand 1 -- `ld1w {z0\.s,z8\.s},p8/z,\[x0,x1,lsl#2\]'
[^ :]+:[0-9]+: Error: expected a 64-bit base register at operand 3 -- `ld1w {z0\.s,z8\.s},pn8/z,\[w0,w30,lsl#2\]'
[^ :]+:[0-9]+: Error: invalid base register at operand 3 -- `ld1w {z0\.s,z8\.s},pn8/z,\[xzr,xzr,lsl#2\]'
[^ :]+:[0-9]+: Error: invalid offset register at operand 3 -- `ld1w {z0\.s,z8\.s},pn8/z,\[x0,sp,lsl#2\]'
[^ :]+:[0-9]+: Error: invalid addressing mode at operand 3 -- `ld1w {z0\.s,z8\.s},pn8/z,\[x0,x1,lsl#1\]'
[^ :]+:[0-9]+: Error: start register out of range at operand 1 -- `ld1w {z4\.s,z8\.s,z12\.s,z16\.s},pn8/z,\[x0,x1,lsl#2\]'
[^ :]+:[0-9]+: Error: start register out of range at operand 1 -- `ld1w {z20\.s,z24\.s,z28\.s,z0\.s},pn8/z,\[x0,x1,lsl#2\]'
[^ :]+:[0-9]+: Error: operand mismatch -- `ld1w {z0\.h,z4\.h,z8\.h,z12\.h},pn8/z,\[x0,x1,lsl#2\]'
[^ :]+:[0-9]+: Info:    did you mean this\?
[^ :]+:[0-9]+: Info:    	ld1w {z0\.s, z4\.s, z8\.s, z12\.s}, pn8/z, \[x0, x1, lsl #2\]
[^ :]+:[0-9]+: Error: expected a single-register list at operand 1 -- `ld1w {z0\.s,z4\.s,z8\.s,z12\.s},p8/z,\[x0,x1,lsl#2\]'
[^ :]+:[0-9]+: Error: expected a 64-bit base register at operand 3 -- `ld1w {z0\.s,z4\.s,z8\.s,z12\.s},pn8/z,\[w0,w30,lsl#2\]'
[^ :]+:[0-9]+: Error: invalid base register at operand 3 -- `ld1w {z0\.s,z4\.s,z8\.s,z12\.s},pn8/z,\[xzr,xzr,lsl#2\]'
[^ :]+:[0-9]+: Error: invalid offset register at operand 3 -- `ld1w {z0\.s,z4\.s,z8\.s,z12\.s},pn8/z,\[x0,sp,lsl#2\]'
[^ :]+:[0-9]+: Error: invalid addressing mode at operand 3 -- `ld1w {z0\.s,z4\.s,z8\.s,z12\.s},pn8/z,\[x0,x1,lsl#1\]'
