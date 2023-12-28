[^ :]+: Assembler messages:
[^ :]+:[0-9]+: Error: expected a selection register in the range w8-w11 at operand 1 -- `fmla za\.d\[w7,0\],{z0\.d-z1\.d},z0\.d\[0\]'
[^ :]+:[0-9]+: Error: expected a selection register in the range w8-w11 at operand 1 -- `fmla za\.d\[w12,0\],{z0\.d-z1\.d},z0\.d\[0\]'
[^ :]+:[0-9]+: Error: immediate offset out of range 0 to 7 at operand 1 -- `fmla za\.d\[w8,-1\],{z0\.d-z1\.d},z0\.d\[0\]'
[^ :]+:[0-9]+: Error: immediate offset out of range 0 to 7 at operand 1 -- `fmla za\.d\[w8,8\],{z0\.d-z1\.d},z0\.d\[0\]'
[^ :]+:[0-9]+: Error: expected a list of 4 registers at operand 2 -- `fmla za\.d\[w8,0,vgx4\],{z0\.d-z1\.d},z0\.d\[0\]'
[^ :]+:[0-9]+: Error: expected a list of 2 or 4 registers at operand 2 -- `fmla za\.d\[w8,0\],{z0\.d-z2\.d},z0\.d\[0\]'
[^ :]+:[0-9]+: Error: start register out of range at operand 2 -- `fmla za\.d\[w8,0\],{z1\.d-z2\.d},z0\.d\[0\]'
[^ :]+:[0-9]+: Error: z0-z15 expected at operand 3 -- `fmla za\.d\[w8,0\],{z0\.d-z1\.d},z16\.d\[0\]'
[^ :]+:[0-9]+: Error: register element index out of range 0 to 1 at operand 3 -- `fmla za\.d\[w8,0\],{z0\.d-z1\.d},z0\.d\[-1\]'
[^ :]+:[0-9]+: Error: register element index out of range 0 to 1 at operand 3 -- `fmla za\.d\[w8,0\],{z0\.d-z1\.d},z0\.d\[2\]'
[^ :]+:[0-9]+: Error: expected a selection register in the range w8-w11 at operand 1 -- `fmla za\.d\[w7,0\],{z0\.d-z3\.d},z0\.d\[0\]'
[^ :]+:[0-9]+: Error: expected a selection register in the range w8-w11 at operand 1 -- `fmla za\.d\[w12,0\],{z0\.d-z3\.d},z0\.d\[0\]'
[^ :]+:[0-9]+: Error: immediate offset out of range 0 to 7 at operand 1 -- `fmla za\.d\[w8,-1\],{z0\.d-z3\.d},z0\.d\[0\]'
[^ :]+:[0-9]+: Error: immediate offset out of range 0 to 7 at operand 1 -- `fmla za\.d\[w8,8\],{z0\.d-z3\.d},z0\.d\[0\]'
[^ :]+:[0-9]+: Error: expected a list of 2 registers at operand 2 -- `fmla za\.d\[w8,0,vgx2\],{z0\.d-z3\.d},z0\.d\[0\]'
[^ :]+:[0-9]+: Error: too many registers in vector register list at operand 2 -- `fmla za\.d\[w8,0\],{z0\.d-z4\.d},z0\.d\[0\]'
[^ :]+:[0-9]+: Error: start register out of range at operand 2 -- `fmla za\.d\[w8,0\],{z1\.d-z4\.d},z0\.d\[0\]'
[^ :]+:[0-9]+: Error: start register out of range at operand 2 -- `fmla za\.d\[w8,0\],{z2\.d-z5\.d},z0\.d\[0\]'
[^ :]+:[0-9]+: Error: start register out of range at operand 2 -- `fmla za\.d\[w8,0\],{z3\.d-z6\.d},z0\.d\[0\]'
[^ :]+:[0-9]+: Error: z0-z15 expected at operand 3 -- `fmla za\.d\[w8,0\],{z0\.d-z3\.d},z16\.d\[0\]'
[^ :]+:[0-9]+: Error: register element index out of range 0 to 1 at operand 3 -- `fmla za\.d\[w8,0\],{z0\.d-z3\.d},z0\.d\[-1\]'
[^ :]+:[0-9]+: Error: register element index out of range 0 to 1 at operand 3 -- `fmla za\.d\[w8,0\],{z0\.d-z3\.d},z0\.d\[2\]'
[^ :]+:[0-9]+: Error: expected a selection register in the range w8-w11 at operand 1 -- `fmla za\.d\[w0,0\],{z0\.d-z1\.d},z0\.d'
[^ :]+:[0-9]+: Error: expected a 32-bit selection register at operand 1 -- `fmla za\.d\[w31,0\],{z0\.d-z1\.d},z0\.d'
[^ :]+:[0-9]+: Error: immediate offset out of range 0 to 7 at operand 1 -- `fmla za\.d\[w8,1<<63\],{z0\.d-z1\.d},z0\.d'
[^ :]+:[0-9]+: Error: z0-z15 expected at operand 3 -- `fmla za\.d\[w8,0\],{z0\.d-z1\.d},z31\.d'
[^ :]+:[0-9]+: Error: the last offset is equal to the first offset at operand 1 -- `fmla za\.d\[w8,0:0\],{z0\.d-z1\.d},z0\.d'
[^ :]+:[0-9]+: Error: the last offset is less than the first offset at operand 1 -- `fmla za\.d\[w8,0:-1\],{z0\.d-z1\.d},z0\.d'
[^ :]+:[0-9]+: Error: expected a single offset rather than a range at operand 1 -- `fmla za\.d\[w8,0:1\],{z0\.d-z1\.d},z0\.d'
[^ :]+:[0-9]+: Error: expected a single offset rather than a range at operand 1 -- `fmla za\.d\[w8,0:100\],{z0\.d-z1\.d},z0\.d'
[^ :]+:[0-9]+: Error: expected a selection register in the range w8-w11 at operand 1 -- `fmla za\.d\[w7,0\],{z0\.d-z1\.d},z0\.d'
[^ :]+:[0-9]+: Error: expected a selection register in the range w8-w11 at operand 1 -- `fmla za\.d\[w12,0\],{z0\.d-z1\.d},z0\.d'
[^ :]+:[0-9]+: Error: immediate offset out of range 0 to 7 at operand 1 -- `fmla za\.d\[w8,-1\],{z0\.d-z1\.d},z0\.d'
[^ :]+:[0-9]+: Error: immediate offset out of range 0 to 7 at operand 1 -- `fmla za\.d\[w8,8\],{z0\.d-z1\.d},z0\.d'
[^ :]+:[0-9]+: Error: z0-z15 expected at operand 3 -- `fmla za\.d\[w8,0\],{z0\.d-z1\.d},z16\.d'
[^ :]+:[0-9]+: Error: expected a selection register in the range w8-w11 at operand 1 -- `fmla za\.d\[w7,0\],{z0\.d-z3\.d},z0\.d'
[^ :]+:[0-9]+: Error: expected a selection register in the range w8-w11 at operand 1 -- `fmla za\.d\[w12,0\],{z0\.d-z3\.d},z0\.d'
[^ :]+:[0-9]+: Error: immediate offset out of range 0 to 7 at operand 1 -- `fmla za\.d\[w8,-1\],{z0\.d-z3\.d},z0\.d'
[^ :]+:[0-9]+: Error: immediate offset out of range 0 to 7 at operand 1 -- `fmla za\.d\[w8,8\],{z0\.d-z3\.d},z0\.d'
[^ :]+:[0-9]+: Error: z0-z15 expected at operand 3 -- `fmla za\.d\[w8,0\],{z0\.d-z3\.d},z16\.d'
[^ :]+:[0-9]+: Error: expected a list of 2 or 4 registers at operand 2 -- `fmla za\.d\[w8,0\],{z0\.d-z2\.d},z0\.d'
[^ :]+:[0-9]+: Error: too many registers in vector register list at operand 2 -- `fmla za\.d\[w8,0\],{z0\.d-z4\.d},z0\.d'
[^ :]+:[0-9]+: Error: expected a list of 2 or 4 registers at operand 2 -- `fmla za\.d\[w8,0\],{z0\.d,z1\.d,z2\.d},z0\.d'
[^ :]+:[0-9]+: Error: invalid register list at operand 2 -- `fmla za\.d\[w8,0\],{z0\.d,z1\.d,z5\.d},z0\.d'
[^ :]+:[0-9]+: Error: expected a list of 4 registers at operand 2 -- `fmla za\.d\[w8,0,vgx4\],{z0\.d-z1\.d},z0\.d'
[^ :]+:[0-9]+: Error: expected a list of 2 registers at operand 2 -- `fmla za\.d\[w8,0,vgx2\],{z0\.d-z3\.d},z0\.d'
[^ :]+:[0-9]+: Error: operand mismatch -- `fmla za\[w8,0\],{z0\.d-z1\.d},z0\.d'
[^ :]+:[0-9]+: Info:    did you mean this\?
[^ :]+:[0-9]+: Info:    	fmla za\.d\[w8, 0\], {z0\.d-z1\.d}, z0\.d
[^ :]+:[0-9]+: Info:    other valid variant\(s\):
[^ :]+:[0-9]+: Info:    	fmla za\.s\[w8, 0\], {z0\.s-z1\.s}, z0\.s
[^ :]+:[0-9]+: Error: missing type suffix at operand 2 -- `fmla za\.d\[w8,0\],{z0-z1},z0\.d'
[^ :]+:[0-9]+: Error: operand mismatch -- `fmla za\.d\[w8,0\],{z0\.d-z1\.d},z0'
[^ :]+:[0-9]+: Info:    did you mean this\?
[^ :]+:[0-9]+: Info:    	fmla za\.d\[w8, 0\], {z0\.d-z1\.d}, z0\.d
[^ :]+:[0-9]+: Info:    other valid variant\(s\):
[^ :]+:[0-9]+: Info:    	fmla za\.s\[w8, 0\], {z0\.s-z1\.s}, z0\.s
[^ :]+:[0-9]+: Error: operand mismatch -- `fmla za\[w8,0\],{z0\.d-z1\.d},z0'
[^ :]+:[0-9]+: Info:    did you mean this\?
[^ :]+:[0-9]+: Info:    	fmla za\.d\[w8, 0\], {z0\.d-z1\.d}, z0\.d
[^ :]+:[0-9]+: Info:    other valid variant\(s\):
[^ :]+:[0-9]+: Info:    	fmla za\.s\[w8, 0\], {z0\.s-z1\.s}, z0\.s
[^ :]+:[0-9]+: Error: expected a selection register in the range w8-w11 at operand 1 -- `fmla za\.d\[w7,0\],{z0\.d-z1\.d},{z0\.d-z1\.d}'
[^ :]+:[0-9]+: Error: expected a selection register in the range w8-w11 at operand 1 -- `fmla za\.d\[w12,0\],{z0\.d-z1\.d},{z0\.d-z1\.d}'
[^ :]+:[0-9]+: Error: immediate offset out of range 0 to 7 at operand 1 -- `fmla za\.d\[w8,-1\],{z0\.d-z1\.d},{z0\.d-z1\.d}'
[^ :]+:[0-9]+: Error: immediate offset out of range 0 to 7 at operand 1 -- `fmla za\.d\[w8,8\],{z0\.d-z1\.d},{z0\.d-z1\.d}'
[^ :]+:[0-9]+: Error: start register out of range at operand 2 -- `fmla za\.d\[w8,0\],{z1\.d-z2\.d},{z0\.d-z1\.d}'
[^ :]+:[0-9]+: Error: start register out of range at operand 3 -- `fmla za\.d\[w8,0\],{z0\.d-z1\.d},{z15\.d-z16\.d}'
[^ :]+:[0-9]+: Error: start register out of range at operand 3 -- `fmla za\.d\[w8,0\],{z0\.d-z1\.d},{z31\.d,z0\.d}'
[^ :]+:[0-9]+: Error: expected a selection register in the range w8-w11 at operand 1 -- `fmla za\.d\[w7,0\],{z0\.d-z3\.d},{z0\.d-z3\.d}'
[^ :]+:[0-9]+: Error: expected a selection register in the range w8-w11 at operand 1 -- `fmla za\.d\[w12,0\],{z0\.d-z3\.d},{z0\.d-z3\.d}'
[^ :]+:[0-9]+: Error: immediate offset out of range 0 to 7 at operand 1 -- `fmla za\.d\[w8,-1\],{z0\.d-z3\.d},{z0\.d-z3\.d}'
[^ :]+:[0-9]+: Error: immediate offset out of range 0 to 7 at operand 1 -- `fmla za\.d\[w8,8\],{z0\.d-z3\.d},{z0\.d-z3\.d}'
[^ :]+:[0-9]+: Error: start register out of range at operand 2 -- `fmla za\.d\[w8,0\],{z1\.d-z4\.d},{z0\.d-z3\.d}'
[^ :]+:[0-9]+: Error: start register out of range at operand 2 -- `fmla za\.d\[w8,0\],{z2\.d-z5\.d},{z0\.d-z3\.d}'
[^ :]+:[0-9]+: Error: start register out of range at operand 2 -- `fmla za\.d\[w8,0\],{z3\.d-z6\.d},{z0\.d-z3\.d}'
[^ :]+:[0-9]+: Error: start register out of range at operand 3 -- `fmla za\.d\[w8,0\],{z0\.d-z3\.d},{z15\.d-z18\.d}'
[^ :]+:[0-9]+: Error: start register out of range at operand 3 -- `fmla za\.d\[w8,0\],{z0\.d-z3\.d},{z29\.d,z30\.d,z31\.d,z0\.d}'
[^ :]+:[0-9]+: Error: expected a list of 2 or 4 registers at operand 2 -- `fmla za\.d\[w8,0\],{z0\.d-z2\.d},{z0\.d-z1\.d}'
[^ :]+:[0-9]+: Error: expected a list of 4 registers at operand 3 -- `fmla za\.d\[w8,0\],{z0\.d-z3\.d},{z0\.d-z1\.d}'
[^ :]+:[0-9]+: Error: expected a list of 2 registers at operand 3 -- `fmla za\.d\[w8,0\],{z0\.d-z1\.d},{z0\.d-z2\.d}'
[^ :]+:[0-9]+: Error: expected a list of 2 registers at operand 3 -- `fmla za\.d\[w8,0\],{z0\.d-z1\.d},{z0\.d-z3\.d}'
[^ :]+:[0-9]+: Error: too many registers in vector register list at operand 3 -- `fmla za\.d\[w8,0\],{z0\.d-z1\.d},{z0\.d-z4\.d}'
[^ :]+:[0-9]+: Error: expected a list of 4 registers at operand 2 -- `fmla za\.d\[w8,0,vgx4\],{z0\.d-z1\.d},{z0\.d-z3\.d}'
[^ :]+:[0-9]+: Error: expected a list of 4 registers at operand 3 -- `fmla za\.d\[w8,0,vgx4\],{z0\.d-z3\.d},{z0\.d-z1\.d}'
[^ :]+:[0-9]+: Error: expected a list of 2 registers at operand 3 -- `fmla za\.d\[w8,0,vgx2\],{z0\.d-z1\.d},{z0\.d-z3\.d}'
[^ :]+:[0-9]+: Error: expected a list of 2 registers at operand 2 -- `fmla za\.d\[w8,0,vgx2\],{z0\.d-z3\.d},{z0\.d-z1\.d}'
[^ :]+:[0-9]+: Error: operand mismatch -- `fmla za\[w8,0\],{z0\.d-z1\.d},{z0\.d-z1\.d}'
[^ :]+:[0-9]+: Info:    did you mean this\?
[^ :]+:[0-9]+: Info:    	fmla za\.d\[w8, 0\], {z0\.d-z1\.d}, {z0\.d-z1\.d}
[^ :]+:[0-9]+: Info:    other valid variant\(s\):
[^ :]+:[0-9]+: Info:    	fmla za\.s\[w8, 0\], {z0\.s-z1\.s}, {z0\.s-z1\.s}
[^ :]+:[0-9]+: Error: operand mismatch -- `fmla za\[w8,0\],{z0\.d-z3\.d},{z0\.d-z3\.d}'
[^ :]+:[0-9]+: Info:    did you mean this\?
[^ :]+:[0-9]+: Info:    	fmla za\.d\[w8, 0\], {z0\.d-z3\.d}, {z0\.d-z3\.d}
[^ :]+:[0-9]+: Info:    other valid variant\(s\):
[^ :]+:[0-9]+: Info:    	fmla za\.s\[w8, 0\], {z0\.s-z3\.s}, {z0\.s-z3\.s}
