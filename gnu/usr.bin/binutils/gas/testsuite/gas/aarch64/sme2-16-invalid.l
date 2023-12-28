[^ :]+: Assembler messages:
[^ :]+:[0-9]+: Error: expected a register at operand 1 -- `sdot 0,{z0\.h-z1\.h},z0\.h\[0\]'
[^ :]+:[0-9]+: Error: expected '{' at operand 2 -- `sdot za\.s\[w8,0\],0,z0\.h\[0\]'
[^ :]+:[0-9]+: Error: expected a register or register list at operand 3 -- `sdot za\.s\[w8,0\],{z0\.h-z1\.h},0'
[^ :]+:[0-9]+: Error: missing braces at operand 2 -- `sdot za\.h\[w8,0\],z0\.h,z0\.h'
[^ :]+:[0-9]+: Error: operand mismatch -- `sdot za\.h\[w8,0\],{z0\.h-z1\.h},z0\.h'
[^ :]+:[0-9]+: Info:    did you mean this\?
[^ :]+:[0-9]+: Info:    	sdot za\.s\[w8, 0\], {z0\.h-z1\.h}, z0\.h
[^ :]+:[0-9]+: Error: expected a selection register in the range w8-w11 at operand 1 -- `sdot za\.s\[w7,0\],{z0\.h-z1\.h},z0\.h\[0\]'
[^ :]+:[0-9]+: Error: expected a selection register in the range w8-w11 at operand 1 -- `sdot za\.s\[w12,0\],{z0\.h-z1\.h},z0\.h\[0\]'
[^ :]+:[0-9]+: Error: immediate offset out of range 0 to 7 at operand 1 -- `sdot za\.s\[w8,-1\],{z0\.h-z1\.h},z0\.h\[0\]'
[^ :]+:[0-9]+: Error: immediate offset out of range 0 to 7 at operand 1 -- `sdot za\.s\[w8,8\],{z0\.h-z1\.h},z0\.h\[0\]'
[^ :]+:[0-9]+: Error: expected a list of 2 or 4 registers at operand 2 -- `sdot za\.s\[w8,0\],{z0\.h-z2\.h},z0\.h\[0\]'
[^ :]+:[0-9]+: Error: start register out of range at operand 2 -- `sdot za\.s\[w8,0\],{z1\.h-z2\.h},z0\.h\[0\]'
[^ :]+:[0-9]+: Error: register element index out of range 0 to 3 at operand 3 -- `sdot za\.s\[w8,0\],{z0\.h-z1\.h},z0\.h\[-1\]'
[^ :]+:[0-9]+: Error: register element index out of range 0 to 3 at operand 3 -- `sdot za\.s\[w8,0\],{z0\.h-z1\.h},z0\.h\[4\]'
[^ :]+:[0-9]+: Error: z0-z15 expected at operand 3 -- `sdot za\.s\[w8,0\],{z0\.h-z1\.h},z16\.h\[0\]'
[^ :]+:[0-9]+: Error: expected a selection register in the range w8-w11 at operand 1 -- `sdot za\.s\[w7,0\],{z0\.h-z3\.h},z0\.h\[0\]'
[^ :]+:[0-9]+: Error: expected a selection register in the range w8-w11 at operand 1 -- `sdot za\.s\[w12,0\],{z0\.h-z3\.h},z0\.h\[0\]'
[^ :]+:[0-9]+: Error: immediate offset out of range 0 to 7 at operand 1 -- `sdot za\.s\[w8,-1\],{z0\.h-z3\.h},z0\.h\[0\]'
[^ :]+:[0-9]+: Error: immediate offset out of range 0 to 7 at operand 1 -- `sdot za\.s\[w8,8\],{z0\.h-z3\.h},z0\.h\[0\]'
[^ :]+:[0-9]+: Error: start register out of range at operand 2 -- `sdot za\.s\[w8,0\],{z1\.h-z4\.h},z0\.h\[0\]'
[^ :]+:[0-9]+: Error: start register out of range at operand 2 -- `sdot za\.s\[w8,0\],{z2\.h-z5\.h},z0\.h\[0\]'
[^ :]+:[0-9]+: Error: start register out of range at operand 2 -- `sdot za\.s\[w8,0\],{z3\.h-z6\.h},z0\.h\[0\]'
[^ :]+:[0-9]+: Error: register element index out of range 0 to 3 at operand 3 -- `sdot za\.s\[w8,0\],{z0\.h-z3\.h},z0\.h\[-1\]'
[^ :]+:[0-9]+: Error: register element index out of range 0 to 3 at operand 3 -- `sdot za\.s\[w8,0\],{z0\.h-z3\.h},z0\.h\[4\]'
[^ :]+:[0-9]+: Error: z0-z15 expected at operand 3 -- `sdot za\.s\[w8,0\],{z0\.h-z3\.h},z16\.h\[0\]'
[^ :]+:[0-9]+: Error: expected a list of 4 registers at operand 2 -- `sdot za\.s\[w8,0,vgx4\],{z0\.h-z1\.h},z0\.h\[0\]'
[^ :]+:[0-9]+: Error: expected a list of 2 registers at operand 2 -- `sdot za\.s\[w8,0,vgx2\],{z0\.h-z3\.h},z0\.h\[0\]'
[^ :]+:[0-9]+: Error: operand mismatch -- `sdot za\[w8,0\],{z0\.h-z1\.h},z0\.h\[0\]'
[^ :]+:[0-9]+: Info:    did you mean this\?
[^ :]+:[0-9]+: Info:    	sdot za\.s\[w8, 0\], {z0\.h-z1\.h}, z0\.h\[0\]
[^ :]+:[0-9]+: Error: missing type suffix at operand 2 -- `sdot za\.s\[w8,0\],{z0-z1},z0\.h\[0\]'
[^ :]+:[0-9]+: Error: operand mismatch -- `sdot za\.s\[w8,0\],{z0\.h-z1\.h},z0\[0\]'
[^ :]+:[0-9]+: Info:    did you mean this\?
[^ :]+:[0-9]+: Info:    	sdot za\.s\[w8, 0\], {z0\.h-z1\.h}, z0\.h\[0\]
[^ :]+:[0-9]+: Error: operand mismatch -- `sdot za\.h\[w8,0\],{z0\.h-z1\.h},z0\.h\[0\]'
[^ :]+:[0-9]+: Info:    did you mean this\?
[^ :]+:[0-9]+: Info:    	sdot za\.s\[w8, 0\], {z0\.h-z1\.h}, z0\.h\[0\]
[^ :]+:[0-9]+: Error: operand mismatch -- `sdot za\.h\[w8,0\],{z0\.s-z1\.s},z0\.s\[0\]'
[^ :]+:[0-9]+: Info:    did you mean this\?
[^ :]+:[0-9]+: Info:    	sdot za\.s\[w8, 0\], {z0\.h-z1\.h}, z0\.h\[0\]
[^ :]+:[0-9]+: Error: expected a list of 2 or 4 registers at operand 2 -- `sdot za\.s\[w8,0\],{z0\.h-z2\.h},z0\.h'
[^ :]+:[0-9]+: Error: too many registers in vector register list at operand 2 -- `sdot za\.s\[w8,0\],{z0\.h-z4\.h},z0\.h'
[^ :]+:[0-9]+: Error: expected a list of 2 or 4 registers at operand 2 -- `sdot za\.s\[w8,0\],{z0\.h,z1\.h,z2\.h},z0\.h'
[^ :]+:[0-9]+: Error: invalid register list at operand 2 -- `sdot za\.s\[w8,0\],{z0\.h,z1\.h,z5\.h},z0\.h'
[^ :]+:[0-9]+: Error: expected a list of 4 registers at operand 2 -- `sdot za\.s\[w8,0,vgx4\],{z0\.h-z1\.h},z0\.h'
[^ :]+:[0-9]+: Error: expected a list of 2 registers at operand 2 -- `sdot za\.s\[w8,0,vgx2\],{z0\.h-z3\.h},z0\.h'
[^ :]+:[0-9]+: Error: operand mismatch -- `sdot za\[w8,0\],{z0\.h-z1\.h},z0\.h'
[^ :]+:[0-9]+: Info:    did you mean this\?
[^ :]+:[0-9]+: Info:    	sdot za\.s\[w8, 0\], {z0\.h-z1\.h}, z0\.h
[^ :]+:[0-9]+: Error: missing type suffix at operand 2 -- `sdot za\.s\[w8,0\],{z0-z1},z0\.h'
[^ :]+:[0-9]+: Error: operand mismatch -- `sdot za\.s\[w8,0\],{z0\.h-z1\.h},z0'
[^ :]+:[0-9]+: Info:    did you mean this\?
[^ :]+:[0-9]+: Info:    	sdot za\.s\[w8, 0\], {z0\.h-z1\.h}, z0\.h
[^ :]+:[0-9]+: Error: operand mismatch -- `sdot za\[w8,0\],{z0\.h-z1\.h},z0'
[^ :]+:[0-9]+: Info:    did you mean this\?
[^ :]+:[0-9]+: Info:    	sdot za\.s\[w8, 0\], {z0\.h-z1\.h}, z0\.h
[^ :]+:[0-9]+: Error: expected a selection register in the range w8-w11 at operand 1 -- `sdot za\.s\[w7,0\],{z0\.h-z1\.h},{z0\.h-z1\.h}'
[^ :]+:[0-9]+: Error: expected a selection register in the range w8-w11 at operand 1 -- `sdot za\.s\[w12,0\],{z0\.h-z1\.h},{z0\.h-z1\.h}'
[^ :]+:[0-9]+: Error: immediate offset out of range 0 to 7 at operand 1 -- `sdot za\.s\[w8,-1\],{z0\.h-z1\.h},{z0\.h-z1\.h}'
[^ :]+:[0-9]+: Error: immediate offset out of range 0 to 7 at operand 1 -- `sdot za\.s\[w8,8\],{z0\.h-z1\.h},{z0\.h-z1\.h}'
[^ :]+:[0-9]+: Error: start register out of range at operand 2 -- `sdot za\.s\[w8,0\],{z1\.h-z2\.h},{z0\.h-z1\.h}'
[^ :]+:[0-9]+: Error: start register out of range at operand 3 -- `sdot za\.s\[w8,0\],{z0\.h-z1\.h},{z15\.h-z16\.h}'
[^ :]+:[0-9]+: Error: start register out of range at operand 3 -- `sdot za\.s\[w8,0\],{z0\.h-z1\.h},{z31\.h,z0\.h}'
[^ :]+:[0-9]+: Error: expected a selection register in the range w8-w11 at operand 1 -- `sdot za\.s\[w7,0\],{z0\.h-z3\.h},{z0\.h-z3\.h}'
[^ :]+:[0-9]+: Error: expected a selection register in the range w8-w11 at operand 1 -- `sdot za\.s\[w12,0\],{z0\.h-z3\.h},{z0\.h-z3\.h}'
[^ :]+:[0-9]+: Error: immediate offset out of range 0 to 7 at operand 1 -- `sdot za\.s\[w8,-1\],{z0\.h-z3\.h},{z0\.h-z3\.h}'
[^ :]+:[0-9]+: Error: immediate offset out of range 0 to 7 at operand 1 -- `sdot za\.s\[w8,8\],{z0\.h-z3\.h},{z0\.h-z3\.h}'
[^ :]+:[0-9]+: Error: start register out of range at operand 2 -- `sdot za\.s\[w8,0\],{z1\.h-z4\.h},{z0\.h-z3\.h}'
[^ :]+:[0-9]+: Error: start register out of range at operand 2 -- `sdot za\.s\[w8,0\],{z2\.h-z5\.h},{z0\.h-z3\.h}'
[^ :]+:[0-9]+: Error: start register out of range at operand 2 -- `sdot za\.s\[w8,0\],{z3\.h-z6\.h},{z0\.h-z3\.h}'
[^ :]+:[0-9]+: Error: start register out of range at operand 3 -- `sdot za\.s\[w8,0\],{z0\.h-z3\.h},{z15\.h-z18\.h}'
[^ :]+:[0-9]+: Error: start register out of range at operand 3 -- `sdot za\.s\[w8,0\],{z0\.h-z3\.h},{z29\.h,z30\.h,z31\.h,z0\.h}'
[^ :]+:[0-9]+: Error: expected a list of 2 or 4 registers at operand 2 -- `sdot za\.s\[w8,0\],{z0\.h-z2\.h},{z0\.h-z1\.h}'
[^ :]+:[0-9]+: Error: expected a list of 4 registers at operand 3 -- `sdot za\.s\[w8,0\],{z0\.h-z3\.h},{z0\.h-z1\.h}'
[^ :]+:[0-9]+: Error: expected a list of 2 registers at operand 3 -- `sdot za\.s\[w8,0\],{z0\.h-z1\.h},{z0\.h-z2\.h}'
[^ :]+:[0-9]+: Error: expected a list of 2 registers at operand 3 -- `sdot za\.s\[w8,0\],{z0\.h-z1\.h},{z0\.h-z3\.h}'
[^ :]+:[0-9]+: Error: too many registers in vector register list at operand 3 -- `sdot za\.s\[w8,0\],{z0\.h-z1\.h},{z0\.h-z4\.h}'
[^ :]+:[0-9]+: Error: expected a list of 4 registers at operand 2 -- `sdot za\.s\[w8,0,vgx4\],{z0\.h-z1\.h},{z0\.h-z3\.h}'
[^ :]+:[0-9]+: Error: expected a list of 4 registers at operand 3 -- `sdot za\.s\[w8,0,vgx4\],{z0\.h-z3\.h},{z0\.h-z1\.h}'
[^ :]+:[0-9]+: Error: expected a list of 2 registers at operand 3 -- `sdot za\.s\[w8,0,vgx2\],{z0\.h-z1\.h},{z0\.h-z3\.h}'
[^ :]+:[0-9]+: Error: expected a list of 2 registers at operand 2 -- `sdot za\.s\[w8,0,vgx2\],{z0\.h-z3\.h},{z0\.h-z1\.h}'
[^ :]+:[0-9]+: Error: operand mismatch -- `sdot za\[w8,0\],{z0\.h-z1\.h},{z0\.h-z1\.h}'
[^ :]+:[0-9]+: Info:    did you mean this\?
[^ :]+:[0-9]+: Info:    	sdot za\.s\[w8, 0\], {z0\.h-z1\.h}, {z0\.h-z1\.h}
[^ :]+:[0-9]+: Error: operand mismatch -- `sdot za\[w8,0\],{z0\.h-z3\.h},{z0\.h-z3\.h}'
[^ :]+:[0-9]+: Info:    did you mean this\?
[^ :]+:[0-9]+: Info:    	sdot za\.s\[w8, 0\], {z0\.h-z3\.h}, {z0\.h-z3\.h}
[^ :]+:[0-9]+: Error: the last offset is equal to the first offset at operand 1 -- `sdot za\.s\[w8,0:0\],{z0\.h-z3\.h},{z0\.h-z3\.h}'
[^ :]+:[0-9]+: Error: expected a single offset rather than a range at operand 1 -- `sdot za\.s\[w8,0:1\],{z0\.h-z3\.h},{z0\.h-z3\.h}'
[^ :]+:[0-9]+: Error: expected a single offset rather than a range at operand 1 -- `sdot za\.s\[w8,0:2\],{z0\.h-z3\.h},{z0\.h-z3\.h}'
[^ :]+:[0-9]+: Error: expected a single offset rather than a range at operand 1 -- `sdot za\.s\[w8,0:3\],{z0\.h-z3\.h},{z0\.h-z3\.h}'
[^ :]+:[0-9]+: Error: the last offset is less than the first offset at operand 1 -- `sdot za\.s\[w8,1:0\],{z0\.h-z3\.h},{z0\.h-z3\.h}'
[^ :]+:[0-9]+: Error: expected a constant immediate offset at operand 1 -- `sdot za\.s\[w8,foo:1\],{z0\.h-z3\.h},{z0\.h-z3\.h}'
[^ :]+:[0-9]+: Error: expected a constant immediate offset at operand 1 -- `sdot za\.s\[w8,1:foo\],{z0\.h-z3\.h},{z0\.h-z3\.h}'
[^ :]+:[0-9]+: Error: expected a constant immediate offset at operand 1 -- `sdot za\.s\[w8,foo:bar\],{z0\.h-z3\.h},{z0\.h-z3\.h}'
