[^ :]+: Assembler messages:
[^ :]+:[0-9]+: Error: expected a ZA array vector at operand 1 -- `suvdot 0,{z0\.b-z3\.b},z0\.b\[0\]'
[^ :]+:[0-9]+: Error: expected '{' at operand 2 -- `suvdot za\.s\[w8,0\],0,z0\.b\[0\]'
[^ :]+:[0-9]+: Error: expected an SVE vector register at operand 3 -- `suvdot za\.s\[w8,0\],{z0\.b-z3\.b},0'
[^ :]+:[0-9]+: Error: missing braces at operand 2 -- `suvdot za\.h\[w8,0\],z0\.h,z0\.h'
[^ :]+:[0-9]+: Error: operand 3 must be an indexed SVE vector register -- `suvdot za\.h\[w8,0\],{z0\.h-z1\.h},z0\.h'
[^ :]+:[0-9]+: Error: operand mismatch -- `suvdot za\.s\[w8,0\],{z0\.h-z1\.h},z0\.h\[0\]'
[^ :]+:[0-9]+: Info:    did you mean this\?
[^ :]+:[0-9]+: Info:    	suvdot za\.s\[w8, 0\], {z0\.b-z1\.b}, z0\.b\[0\]
[^ :]+:[0-9]+: Error: operand mismatch -- `suvdot za\.s\[w8,0:1\],{z0\.b-z3\.b},z0\.h\[0\]'
[^ :]+:[0-9]+: Info:    did you mean this\?
[^ :]+:[0-9]+: Info:    	suvdot za\.s\[w8, 0:1\], {z0\.b-z3\.b}, z0\.b\[0\]
[^ :]+:[0-9]+: Error: operand mismatch -- `suvdot za\.s\[w8,0,vgx4\],{z0\.b-z3\.b},z0\.h\[0\]'
[^ :]+:[0-9]+: Info:    did you mean this\?
[^ :]+:[0-9]+: Info:    	suvdot za\.s\[w8, 0, vgx4\], {z0\.b-z3\.b}, z0\.b\[0\]
[^ :]+:[0-9]+: Error: expected a selection register in the range w8-w11 at operand 1 -- `suvdot za\.s\[w7,0\],{z0\.b-z3\.b},z0\.b\[0\]'
[^ :]+:[0-9]+: Error: expected a selection register in the range w8-w11 at operand 1 -- `suvdot za\.s\[w12,0\],{z0\.b-z3\.b},z0\.b\[0\]'
[^ :]+:[0-9]+: Error: immediate offset out of range 0 to 7 at operand 1 -- `suvdot za\.s\[w8,-1\],{z0\.b-z3\.b},z0\.b\[0\]'
[^ :]+:[0-9]+: Error: immediate offset out of range 0 to 7 at operand 1 -- `suvdot za\.s\[w8,8\],{z0\.b-z3\.b},z0\.b\[0\]'
[^ :]+:[0-9]+: Error: expected a list of 4 registers at operand 2 -- `suvdot za\.s\[w8,0\],{z0\.b-z1\.b},z0\.b\[0\]'
[^ :]+:[0-9]+: Error: expected a list of 4 registers at operand 2 -- `suvdot za\.s\[w8,0\],{z0\.b-z2\.b},z0\.b\[0\]'
[^ :]+:[0-9]+: Error: start register out of range at operand 2 -- `suvdot za\.s\[w8,0\],{z1\.b-z4\.b},z0\.b\[0\]'
[^ :]+:[0-9]+: Error: start register out of range at operand 2 -- `suvdot za\.s\[w8,0\],{z2\.b-z5\.b},z0\.b\[0\]'
[^ :]+:[0-9]+: Error: start register out of range at operand 2 -- `suvdot za\.s\[w8,0\],{z3\.b-z6\.b},z0\.b\[0\]'
[^ :]+:[0-9]+: Error: register element index out of range 0 to 3 at operand 3 -- `suvdot za\.s\[w8,0\],{z0\.b-z3\.b},z0\.b\[-1\]'
[^ :]+:[0-9]+: Error: register element index out of range 0 to 3 at operand 3 -- `suvdot za\.s\[w8,0\],{z0\.b-z3\.b},z0\.b\[4\]'
[^ :]+:[0-9]+: Error: z0-z15 expected at operand 3 -- `suvdot za\.s\[w8,0\],{z0\.b-z3\.b},z16\.b\[0\]'
