[^ :]+: Assembler messages:
[^ :]+:[0-9]+: Error: expected a register or register list at operand 1 -- `add 0,{z0\.s-z1\.s}'
[^ :]+:[0-9]+: Error: expected '{' at operand 2 -- `add za\.s\[w8,0\],0'
[^ :]+:[0-9]+: Error: expected a selection register in the range w8-w11 at operand 1 -- `add za\.s\[w7,0\],{z0\.s-z1\.s}'
[^ :]+:[0-9]+: Error: expected a selection register in the range w8-w11 at operand 1 -- `add za\.s\[w12,0\],{z0\.s-z1\.s}'
[^ :]+:[0-9]+: Error: immediate offset out of range 0 to 7 at operand 1 -- `add za\.s\[w8,-1\],{z0\.s-z1\.s}'
[^ :]+:[0-9]+: Error: immediate offset out of range 0 to 7 at operand 1 -- `add za\.s\[w8,8\],{z0\.s-z1\.s}'
[^ :]+:[0-9]+: Error: expected a list of 2 or 4 registers at operand 2 -- `add za\.s\[w8,0\],{z0\.s-z2\.s}'
[^ :]+:[0-9]+: Error: start register out of range at operand 2 -- `add za\.s\[w8,0\],{z1\.s-z2\.s}'
[^ :]+:[0-9]+: Error: expected a selection register in the range w8-w11 at operand 1 -- `add za\.s\[w7,0\],{z0\.s-z3\.s}'
[^ :]+:[0-9]+: Error: expected a selection register in the range w8-w11 at operand 1 -- `add za\.s\[w12,0\],{z0\.s-z3\.s}'
[^ :]+:[0-9]+: Error: immediate offset out of range 0 to 7 at operand 1 -- `add za\.s\[w8,-1\],{z0\.s-z3\.s}'
[^ :]+:[0-9]+: Error: immediate offset out of range 0 to 7 at operand 1 -- `add za\.s\[w8,8\],{z1\.s-z3\.s}'
[^ :]+:[0-9]+: Error: start register out of range at operand 2 -- `add za\.s\[w8,0\],{z1\.s-z4\.s}'
[^ :]+:[0-9]+: Error: start register out of range at operand 2 -- `add za\.s\[w8,0\],{z2\.s-z5\.s}'
[^ :]+:[0-9]+: Error: start register out of range at operand 2 -- `add za\.s\[w8,0\],{z3\.s-z6\.s}'
[^ :]+:[0-9]+: Error: expected a list of 4 registers at operand 2 -- `add za\.s\[w8,0,vgx4\],{z0\.s-z1\.s}'
[^ :]+:[0-9]+: Error: expected a list of 2 registers at operand 2 -- `add za\.s\[w8,0,vgx2\],{z0\.s-z3\.s}'
[^ :]+:[0-9]+: Error: operand mismatch -- `add za\[w8,0\],{z0\.s-z1\.s}'
[^ :]+:[0-9]+: Info:    did you mean this\?
[^ :]+:[0-9]+: Info:    	add za\.s\[w8, 0\], {z0\.s-z1\.s}
[^ :]+:[0-9]+: Info:    other valid variant\(s\):
[^ :]+:[0-9]+: Info:    	add za\.d\[w8, 0\], {z0\.d-z1\.d}
[^ :]+:[0-9]+: Error: missing type suffix at operand 2 -- `add za\.s\[w8,0\],{z0-z1}'
[^ :]+:[0-9]+: Error: expected a register or register list at operand 1 -- `add 0,{z0\.s-z1\.s},z0\.s'
[^ :]+:[0-9]+: Error: expected '{' at operand 2 -- `add za\.s\[w8,0\],0,z0\.s'
[^ :]+:[0-9]+: Error: expected a register or register list at operand 3 -- `add za\.s\[w8,0\],{z0\.s-z1\.s},0'
[^ :]+:[0-9]+: Error: expected a selection register in the range w8-w11 at operand 1 -- `add za\.s\[w0,0\],{z0\.s-z1\.s},z0\.s'
[^ :]+:[0-9]+: Error: expected a 32-bit selection register at operand 1 -- `add za\.s\[w31,0\],{z0\.s-z1\.s},z0\.s'
[^ :]+:[0-9]+: Error: immediate offset out of range 0 to 7 at operand 1 -- `add za\.s\[w8,1<<63\],{z0\.s-z1\.s},z0\.s'
[^ :]+:[0-9]+: Error: z0-z15 expected at operand 3 -- `add za\.s\[w8,0\],{z0\.s-z1\.s},z31\.s'
[^ :]+:[0-9]+: Error: the last offset is equal to the first offset at operand 1 -- `add za\.s\[w8,0:0\],{z0\.s-z1\.s},z0\.s'
[^ :]+:[0-9]+: Error: the last offset is less than the first offset at operand 1 -- `add za\.s\[w8,0:-1\],{z0\.s-z1\.s},z0\.s'
[^ :]+:[0-9]+: Error: expected a single offset rather than a range at operand 1 -- `add za\.s\[w8,0:1\],{z0\.s-z1\.s},z0\.s'
[^ :]+:[0-9]+: Error: expected a single offset rather than a range at operand 1 -- `add za\.s\[w8,0:100\],{z0\.s-z1\.s},z0\.s'
[^ :]+:[0-9]+: Error: expected a selection register in the range w8-w11 at operand 1 -- `add za\.s\[w7,0\],{z0\.s-z1\.s},z0\.s'
[^ :]+:[0-9]+: Error: expected a selection register in the range w8-w11 at operand 1 -- `add za\.s\[w12,0\],{z0\.s-z1\.s},z0\.s'
[^ :]+:[0-9]+: Error: immediate offset out of range 0 to 7 at operand 1 -- `add za\.s\[w8,-1\],{z0\.s-z1\.s},z0\.s'
[^ :]+:[0-9]+: Error: immediate offset out of range 0 to 7 at operand 1 -- `add za\.s\[w8,8\],{z0\.s-z1\.s},z0\.s'
[^ :]+:[0-9]+: Error: z0-z15 expected at operand 3 -- `add za\.s\[w8,0\],{z0\.s-z1\.s},z16\.s'
[^ :]+:[0-9]+: Error: expected a selection register in the range w8-w11 at operand 1 -- `add za\.s\[w7,0\],{z0\.s-z3\.s},z0\.s'
[^ :]+:[0-9]+: Error: expected a selection register in the range w8-w11 at operand 1 -- `add za\.s\[w12,0\],{z0\.s-z3\.s},z0\.s'
[^ :]+:[0-9]+: Error: immediate offset out of range 0 to 7 at operand 1 -- `add za\.s\[w8,-1\],{z0\.s-z3\.s},z0\.s'
[^ :]+:[0-9]+: Error: immediate offset out of range 0 to 7 at operand 1 -- `add za\.s\[w8,8\],{z0\.s-z3\.s},z0\.s'
[^ :]+:[0-9]+: Error: z0-z15 expected at operand 3 -- `add za\.s\[w8,0\],{z0\.s-z3\.s},z16\.s'
[^ :]+:[0-9]+: Error: expected a list of 2 or 4 registers at operand 2 -- `add za\.s\[w8,0\],{z0\.s-z2\.s},z0\.s'
[^ :]+:[0-9]+: Error: too many registers in vector register list at operand 2 -- `add za\.s\[w8,0\],{z0\.s-z4\.s},z0\.s'
[^ :]+:[0-9]+: Error: expected a list of 2 or 4 registers at operand 2 -- `add za\.s\[w8,0\],{z0\.s,z1\.s,z2\.s},z0\.s'
[^ :]+:[0-9]+: Error: invalid register list at operand 2 -- `add za\.s\[w8,0\],{z0\.s,z1\.s,z5\.s},z0\.s'
[^ :]+:[0-9]+: Error: expected a list of 4 registers at operand 2 -- `add za\.s\[w8,0,vgx4\],{z0\.s-z1\.s},z0\.s'
[^ :]+:[0-9]+: Error: expected a list of 2 registers at operand 2 -- `add za\.s\[w8,0,vgx2\],{z0\.s-z3\.s},z0\.s'
[^ :]+:[0-9]+: Error: operand mismatch -- `add za\[w8,0\],{z0\.s-z1\.s},z0\.s'
[^ :]+:[0-9]+: Info:    did you mean this\?
[^ :]+:[0-9]+: Info:    	add za\.s\[w8, 0\], {z0\.s-z1\.s}, z0\.s
[^ :]+:[0-9]+: Info:    other valid variant\(s\):
[^ :]+:[0-9]+: Info:    	add za\.d\[w8, 0\], {z0\.d-z1\.d}, z0\.d
[^ :]+:[0-9]+: Error: missing type suffix at operand 2 -- `add za\.s\[w8,0\],{z0-z1},z0\.s'
[^ :]+:[0-9]+: Error: operand mismatch -- `add za\.s\[w8,0\],{z0\.s-z1\.s},z0'
[^ :]+:[0-9]+: Info:    did you mean this\?
[^ :]+:[0-9]+: Info:    	add za\.s\[w8, 0\], {z0\.s-z1\.s}, z0\.s
[^ :]+:[0-9]+: Info:    other valid variant\(s\):
[^ :]+:[0-9]+: Info:    	add za\.d\[w8, 0\], {z0\.d-z1\.d}, z0\.d
[^ :]+:[0-9]+: Error: operand mismatch -- `add za\[w8,0\],{z0\.s-z1\.s},z0'
[^ :]+:[0-9]+: Info:    did you mean this\?
[^ :]+:[0-9]+: Info:    	add za\.s\[w8, 0\], {z0\.s-z1\.s}, z0\.s
[^ :]+:[0-9]+: Info:    other valid variant\(s\):
[^ :]+:[0-9]+: Info:    	add za\.d\[w8, 0\], {z0\.d-z1\.d}, z0\.d
[^ :]+:[0-9]+: Error: expected a selection register in the range w8-w11 at operand 1 -- `add za\.s\[w7,0\],{z0\.s-z1\.s},{z0\.s-z1\.s}'
[^ :]+:[0-9]+: Error: expected a selection register in the range w8-w11 at operand 1 -- `add za\.s\[w12,0\],{z0\.s-z1\.s},{z0\.s-z1\.s}'
[^ :]+:[0-9]+: Error: immediate offset out of range 0 to 7 at operand 1 -- `add za\.s\[w8,-1\],{z0\.s-z1\.s},{z0\.s-z1\.s}'
[^ :]+:[0-9]+: Error: immediate offset out of range 0 to 7 at operand 1 -- `add za\.s\[w8,8\],{z0\.s-z1\.s},{z0\.s-z1\.s}'
[^ :]+:[0-9]+: Error: start register out of range at operand 2 -- `add za\.s\[w8,0\],{z1\.s-z2\.s},{z0\.s-z1\.s}'
[^ :]+:[0-9]+: Error: start register out of range at operand 3 -- `add za\.s\[w8,0\],{z0\.s-z1\.s},{z15\.s-z16\.s}'
[^ :]+:[0-9]+: Error: start register out of range at operand 3 -- `add za\.s\[w8,0\],{z0\.s-z1\.s},{z31\.s,z0\.s}'
[^ :]+:[0-9]+: Error: expected a selection register in the range w8-w11 at operand 1 -- `add za\.s\[w7,0\],{z0\.s-z3\.s},{z0\.s-z3\.s}'
[^ :]+:[0-9]+: Error: expected a selection register in the range w8-w11 at operand 1 -- `add za\.s\[w12,0\],{z0\.s-z3\.s},{z0\.s-z3\.s}'
[^ :]+:[0-9]+: Error: immediate offset out of range 0 to 7 at operand 1 -- `add za\.s\[w8,-1\],{z0\.s-z3\.s},{z0\.s-z3\.s}'
[^ :]+:[0-9]+: Error: immediate offset out of range 0 to 7 at operand 1 -- `add za\.s\[w8,8\],{z0\.s-z3\.s},{z0\.s-z3\.s}'
[^ :]+:[0-9]+: Error: start register out of range at operand 2 -- `add za\.s\[w8,0\],{z1\.s-z4\.s},{z0\.s-z3\.s}'
[^ :]+:[0-9]+: Error: start register out of range at operand 2 -- `add za\.s\[w8,0\],{z2\.s-z5\.s},{z0\.s-z3\.s}'
[^ :]+:[0-9]+: Error: start register out of range at operand 2 -- `add za\.s\[w8,0\],{z3\.s-z6\.s},{z0\.s-z3\.s}'
[^ :]+:[0-9]+: Error: start register out of range at operand 3 -- `add za\.s\[w8,0\],{z0\.s-z3\.s},{z15\.s-z18\.s}'
[^ :]+:[0-9]+: Error: start register out of range at operand 3 -- `add za\.s\[w8,0\],{z0\.s-z3\.s},{z29\.s,z30\.s,z31\.s,z0\.s}'
[^ :]+:[0-9]+: Error: expected a list of 2 or 4 registers at operand 2 -- `add za\.s\[w8,0\],{z0\.s-z2\.s},{z0\.s-z1\.s}'
[^ :]+:[0-9]+: Error: expected a list of 4 registers at operand 3 -- `add za\.s\[w8,0\],{z0\.s-z3\.s},{z0\.s-z1\.s}'
[^ :]+:[0-9]+: Error: expected a list of 2 registers at operand 3 -- `add za\.s\[w8,0\],{z0\.s-z1\.s},{z0\.s-z2\.s}'
[^ :]+:[0-9]+: Error: expected a list of 2 registers at operand 3 -- `add za\.s\[w8,0\],{z0\.s-z1\.s},{z0\.s-z3\.s}'
[^ :]+:[0-9]+: Error: too many registers in vector register list at operand 3 -- `add za\.s\[w8,0\],{z0\.s-z1\.s},{z0\.s-z4\.s}'
[^ :]+:[0-9]+: Error: expected a list of 4 registers at operand 2 -- `add za\.s\[w8,0,vgx4\],{z0\.s-z1\.s},{z0\.s-z3\.s}'
[^ :]+:[0-9]+: Error: expected a list of 4 registers at operand 3 -- `add za\.s\[w8,0,vgx4\],{z0\.s-z3\.s},{z0\.s-z1\.s}'
[^ :]+:[0-9]+: Error: expected a list of 2 registers at operand 3 -- `add za\.s\[w8,0,vgx2\],{z0\.s-z1\.s},{z0\.s-z3\.s}'
[^ :]+:[0-9]+: Error: expected a list of 2 registers at operand 2 -- `add za\.s\[w8,0,vgx2\],{z0\.s-z3\.s},{z0\.s-z1\.s}'
[^ :]+:[0-9]+: Error: operand mismatch -- `add za\[w8,0\],{z0\.s-z1\.s},{z0\.s-z1\.s}'
[^ :]+:[0-9]+: Info:    did you mean this\?
[^ :]+:[0-9]+: Info:    	add za\.s\[w8, 0\], {z0\.s-z1\.s}, {z0\.s-z1\.s}
[^ :]+:[0-9]+: Info:    other valid variant\(s\):
[^ :]+:[0-9]+: Info:    	add za\.d\[w8, 0\], {z0\.d-z1\.d}, {z0\.d-z1\.d}
[^ :]+:[0-9]+: Error: operand mismatch -- `add za\[w8,0\],{z0\.s-z3\.s},{z0\.s-z3\.s}'
[^ :]+:[0-9]+: Info:    did you mean this\?
[^ :]+:[0-9]+: Info:    	add za\.s\[w8, 0\], {z0\.s-z3\.s}, {z0\.s-z3\.s}
[^ :]+:[0-9]+: Info:    other valid variant\(s\):
[^ :]+:[0-9]+: Info:    	add za\.d\[w8, 0\], {z0\.d-z3\.d}, {z0\.d-z3\.d}
[^ :]+:[0-9]+: Error: operand 2 must be the same register as operand 1 -- `add {z0\.b-z1\.b},{z0\.b-z2\.b},z0\.b'
[^ :]+:[0-9]+: Error: operand 2 must be the same register as operand 1 -- `add {z0\.b-z1\.b},{z0\.b-z3\.b},z0\.b'
[^ :]+:[0-9]+: Error: expected a list of 2 or 4 registers at operand 1 -- `add {z0\.b-z2\.b},{z0\.b-z2\.b},z0\.b'
[^ :]+:[0-9]+: Error: operand 2 must be the same register as operand 1 -- `add {z0\.b-z1\.b},{z2\.b-z3\.b},z0\.b'
[^ :]+:[0-9]+: Error: start register out of range at operand 1 -- `add {z1\.b-z2\.b},{z1\.b-z2\.b},z0\.b'
[^ :]+:[0-9]+: Error: start register out of range at operand 1 -- `add {z31\.b,z0\.b},{z31\.b,z0\.b},z0\.b'
[^ :]+:[0-9]+: Error: z0-z15 expected at operand 3 -- `add {z0\.b-z1\.b},{z0\.b-z1\.b},z16\.b'
[^ :]+:[0-9]+: Error: z0-z15 expected at operand 3 -- `add {z0\.b-z1\.b},{z0\.b-z1\.b},z31\.b'
[^ :]+:[0-9]+: Error: operand mismatch -- `add {z0\.b-z1\.b},{z0\.h-z1\.h},z0\.b'
[^ :]+:[0-9]+: Info:    did you mean this\?
[^ :]+:[0-9]+: Info:    	add {z0\.b-z1\.b}, {z0\.b-z1\.b}, z0\.b
[^ :]+:[0-9]+: Info:    other valid variant\(s\):
[^ :]+:[0-9]+: Info:    	add {z0\.h-z1\.h}, {z0\.h-z1\.h}, z0\.h
[^ :]+:[0-9]+: Info:    	add {z0\.s-z1\.s}, {z0\.s-z1\.s}, z0\.s
[^ :]+:[0-9]+: Info:    	add {z0\.d-z1\.d}, {z0\.d-z1\.d}, z0\.d
[^ :]+:[0-9]+: Error: operand mismatch -- `add {z0\.b-z1\.b},{z0\.b-z1\.b},z0\.h'
[^ :]+:[0-9]+: Info:    did you mean this\?
[^ :]+:[0-9]+: Info:    	add {z0\.b-z1\.b}, {z0\.b-z1\.b}, z0\.b
[^ :]+:[0-9]+: Info:    other valid variant\(s\):
[^ :]+:[0-9]+: Info:    	add {z0\.h-z1\.h}, {z0\.h-z1\.h}, z0\.h
[^ :]+:[0-9]+: Info:    	add {z0\.s-z1\.s}, {z0\.s-z1\.s}, z0\.s
[^ :]+:[0-9]+: Info:    	add {z0\.d-z1\.d}, {z0\.d-z1\.d}, z0\.d
[^ :]+:[0-9]+: Error: operand mismatch -- `add {z0\.q-z1\.q},{z0\.q-z1\.q},z0\.q'
[^ :]+:[0-9]+: Info:    did you mean this\?
[^ :]+:[0-9]+: Info:    	add {z0\.b-z1\.b}, {z0\.b-z1\.b}, z0\.b
[^ :]+:[0-9]+: Info:    other valid variant\(s\):
[^ :]+:[0-9]+: Info:    	add {z0\.h-z1\.h}, {z0\.h-z1\.h}, z0\.h
[^ :]+:[0-9]+: Info:    	add {z0\.s-z1\.s}, {z0\.s-z1\.s}, z0\.s
[^ :]+:[0-9]+: Info:    	add {z0\.d-z1\.d}, {z0\.d-z1\.d}, z0\.d
[^ :]+:[0-9]+: Error: operand 2 must be the same register as operand 1 -- `add {z0\.b-z3\.b},{z0\.b-z2\.b},z0\.b'
[^ :]+:[0-9]+: Error: operand 2 must be the same register as operand 1 -- `add {z0\.b-z3\.b},{z0\.b-z1\.b},z0\.b'
[^ :]+:[0-9]+: Error: operand 2 must be the same register as operand 1 -- `add {z0\.b-z3\.b},{z2\.b-z5\.b},z0\.b'
[^ :]+:[0-9]+: Error: start register out of range at operand 1 -- `add {z1\.b-z4\.b},{z1\.b-z4\.b},z0\.b'
[^ :]+:[0-9]+: Error: start register out of range at operand 1 -- `add {z2\.b-z5\.b},{z2\.b-z5\.b},z0\.b'
[^ :]+:[0-9]+: Error: start register out of range at operand 1 -- `add {z3\.b-z6\.b},{z3\.b-z6\.b},z0\.b'
[^ :]+:[0-9]+: Error: start register out of range at operand 1 -- `add {z31\.b,z0\.b,z1\.b,z2\.b},{z31\.b,z0\.b,z1\.b,z2\.b},z0\.b'
[^ :]+:[0-9]+: Error: z0-z15 expected at operand 3 -- `add {z0\.b-z3\.b},{z0\.b-z3\.b},z16\.b'
[^ :]+:[0-9]+: Error: z0-z15 expected at operand 3 -- `add {z0\.b-z3\.b},{z0\.b-z3\.b},z31\.b'
[^ :]+:[0-9]+: Error: operand mismatch -- `add {z0\.b-z3\.b},{z0\.h-z3\.h},z0\.b'
[^ :]+:[0-9]+: Info:    did you mean this\?
[^ :]+:[0-9]+: Info:    	add {z0\.b-z3\.b}, {z0\.b-z3\.b}, z0\.b
[^ :]+:[0-9]+: Info:    other valid variant\(s\):
[^ :]+:[0-9]+: Info:    	add {z0\.h-z3\.h}, {z0\.h-z3\.h}, z0\.h
[^ :]+:[0-9]+: Info:    	add {z0\.s-z3\.s}, {z0\.s-z3\.s}, z0\.s
[^ :]+:[0-9]+: Info:    	add {z0\.d-z3\.d}, {z0\.d-z3\.d}, z0\.d
[^ :]+:[0-9]+: Error: operand mismatch -- `add {z0\.b-z3\.b},{z0\.b-z3\.b},z0\.h'
[^ :]+:[0-9]+: Info:    did you mean this\?
[^ :]+:[0-9]+: Info:    	add {z0\.b-z3\.b}, {z0\.b-z3\.b}, z0\.b
[^ :]+:[0-9]+: Info:    other valid variant\(s\):
[^ :]+:[0-9]+: Info:    	add {z0\.h-z3\.h}, {z0\.h-z3\.h}, z0\.h
[^ :]+:[0-9]+: Info:    	add {z0\.s-z3\.s}, {z0\.s-z3\.s}, z0\.s
[^ :]+:[0-9]+: Info:    	add {z0\.d-z3\.d}, {z0\.d-z3\.d}, z0\.d
[^ :]+:[0-9]+: Error: operand mismatch -- `add {z0\.q-z3\.q},{z0\.q-z3\.q},z0\.q'
[^ :]+:[0-9]+: Info:    did you mean this\?
[^ :]+:[0-9]+: Info:    	add {z0\.b-z3\.b}, {z0\.b-z3\.b}, z0\.b
[^ :]+:[0-9]+: Info:    other valid variant\(s\):
[^ :]+:[0-9]+: Info:    	add {z0\.h-z3\.h}, {z0\.h-z3\.h}, z0\.h
[^ :]+:[0-9]+: Info:    	add {z0\.s-z3\.s}, {z0\.s-z3\.s}, z0\.s
[^ :]+:[0-9]+: Info:    	add {z0\.d-z3\.d}, {z0\.d-z3\.d}, z0\.d
[^ :]+:[0-9]+: Error: expected a register at operand 1 -- `sub {z0\.b-z1\.b},{z0\.b-z1\.b},z0\.b'
[^ :]+:[0-9]+: Error: expected a register at operand 1 -- `sub {z0\.h-z1\.h},{z0\.h-z1\.h},z0\.h'
[^ :]+:[0-9]+: Error: expected a register at operand 1 -- `sub {z0\.s-z1\.s},{z0\.s-z1\.s},z0\.s'
[^ :]+:[0-9]+: Error: expected a register at operand 1 -- `sub {z0\.d-z1\.d},{z0\.d-z1\.d},z0\.d'
[^ :]+:[0-9]+: Error: expected a register at operand 1 -- `sub {z0\.b-z3\.b},{z0\.b-z3\.b},z0\.b'
[^ :]+:[0-9]+: Error: expected a register at operand 1 -- `sub {z0\.h-z3\.h},{z0\.h-z3\.h},z0\.h'
[^ :]+:[0-9]+: Error: expected a register at operand 1 -- `sub {z0\.s-z3\.s},{z0\.s-z3\.s},z0\.s'
[^ :]+:[0-9]+: Error: expected a register at operand 1 -- `sub {z0\.d-z3\.d},{z0\.d-z3\.d},z0\.d'
[^ :]+:[0-9]+: Error: operand mismatch -- `fadd za\.b\[w8,0\],{z0\.b-z1\.b}'
[^ :]+:[0-9]+: Info:    did you mean this\?
[^ :]+:[0-9]+: Info:    	fadd za\.s\[w8, 0\], {z0\.s-z1\.s}
[^ :]+:[0-9]+: Info:    other valid variant\(s\):
[^ :]+:[0-9]+: Info:    	fadd za\.d\[w8, 0\], {z0\.d-z1\.d}
[^ :]+:[0-9]+: Error: operand mismatch -- `fadd za\.h\[w8,0\],{z0\.h-z1\.h}'
[^ :]+:[0-9]+: Info:    did you mean this\?
[^ :]+:[0-9]+: Info:    	fadd za\.s\[w8, 0\], {z0\.s-z1\.s}
[^ :]+:[0-9]+: Info:    other valid variant\(s\):
[^ :]+:[0-9]+: Info:    	fadd za\.d\[w8, 0\], {z0\.d-z1\.d}
