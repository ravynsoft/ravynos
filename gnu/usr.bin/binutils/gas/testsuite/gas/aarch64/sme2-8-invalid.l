[^ :]+: Assembler messages:
[^ :]+:[0-9]+: Error: expected '{' at operand 1 -- `zero 0'
[^ :]+:[0-9]+: Error: expected '{' at operand 1 -- `zero zt0'
[^ :]+:[0-9]+: Error: syntax error in register list at operand 1 -- `zero {'
[^ :]+:[0-9]+: Error: expected ZT0 or a ZA mask at operand 1 -- `zero {foo}'
[^ :]+:[0-9]+: Error: expected ZT0 or a ZA mask at operand 1 -- `zero {zt}'
[^ :]+:[0-9]+: Error: expected ZT0 or a ZA mask at operand 1 -- `zero {x0}'
[^ :]+:[0-9]+: Error: expected ZT0 or a ZA mask at operand 1 -- `zero {z0}'
[^ :]+:[0-9]+: Error: expected '}' after ZT0 at operand 1 -- `zero {zt0'
[^ :]+:[0-9]+: Error: expected '}' after ZT0 at operand 1 -- `zero {zt0\.b}'
[^ :]+:[0-9]+: Error: expected '}' after ZT0 at operand 1 -- `zero {zt0,zt0}'
[^ :]+:[0-9]+: Error: expected a register at operand 1 -- `movt 0,zt0\[0\]'
[^ :]+:[0-9]+: Error: expected a register at operand 2 -- `movt x0,0'
[^ :]+:[0-9]+: Error: missing register index at operand 1 -- `movt zt0,x0'
[^ :]+:[0-9]+: Error: unexpected register type at operand 1 -- `movt za\[0\],x0'
[^ :]+:[0-9]+: Error: unexpected register type at operand 1 -- `movt za0\[0\],x0'
[^ :]+:[0-9]+: Error: bad expression at operand 1 -- `movt zt0\[#0\],x0'
[^ :]+:[0-9]+: Error: register element index out of range 0 to 56 at operand 1 -- `movt zt0\[-1\],x0'
[^ :]+:[0-9]+: Error: byte index must be a multiple of 8 at operand 1 -- `movt zt0\[1\],x0'
[^ :]+:[0-9]+: Error: byte index must be a multiple of 8 at operand 1 -- `movt zt0\[2\],x0'
[^ :]+:[0-9]+: Error: byte index must be a multiple of 8 at operand 1 -- `movt zt0\[4\],x0'
[^ :]+:[0-9]+: Error: byte index must be a multiple of 8 at operand 1 -- `movt zt0\[7\],x0'
[^ :]+:[0-9]+: Error: byte index must be a multiple of 8 at operand 1 -- `movt zt0\[49\],x0'
[^ :]+:[0-9]+: Error: byte index must be a multiple of 8 at operand 1 -- `movt zt0\[50\],x0'
[^ :]+:[0-9]+: Error: byte index must be a multiple of 8 at operand 1 -- `movt zt0\[52\],x0'
[^ :]+:[0-9]+: Error: register element index out of range 0 to 56 at operand 1 -- `movt zt0\[57\],x0'
[^ :]+:[0-9]+: Error: register element index out of range 0 to 56 at operand 1 -- `movt zt0\[64\],x0'
[^ :]+:[0-9]+: Error: register element index out of range 0 to 56 at operand 1 -- `movt zt0\[1<<32\],x0'
[^ :]+:[0-9]+: Error: missing register index at operand 1 -- `movt zt0\.b\[0\],x0'
[^ :]+:[0-9]+: Error: missing register index at operand 1 -- `movt zt0/z\[0\],x0'
[^ :]+:[0-9]+: Error: expected an integer or zero register at operand 2 -- `movt zt0\[0\],sp'
[^ :]+:[0-9]+: Error: operand mismatch -- `movt zt0\[0\],w0'
[^ :]+:[0-9]+: Info:    did you mean this\?
[^ :]+:[0-9]+: Info:    	movt zt0\[0\], x0
[^ :]+:[0-9]+: Error: expected an integer or zero register at operand 2 -- `movt zt0\[0\],wsp'
[^ :]+:[0-9]+: Error: operand mismatch -- `movt zt0\[0\],wzr'
[^ :]+:[0-9]+: Info:    did you mean this\?
[^ :]+:[0-9]+: Info:    	movt zt0\[0\], xzr
[^ :]+:[0-9]+: Error: expected an integer or zero register at operand 2 -- `movt zt0\[0\],0'
[^ :]+:[0-9]+: Error: expected a register at operand 1 -- `ldr 0,\[x0\]'
[^ :]+:[0-9]+: Error: invalid addressing mode at operand 2 -- `ldr zt0,0'
[^ :]+:[0-9]+: Error: operand 2 must be an address with base register \(no offset\) -- `ldr zt0,\[x0,#0\]'
[^ :]+:[0-9]+: Error: expected a register at operand 1 -- `ldr Zt0,\[x0\]'
[^ :]+:[0-9]+: Error: expected a register at operand 1 -- `ldr zT0,\[x0\]'
[^ :]+:[0-9]+: Error: '\]' expected at operand 2 -- `ldr zt0,\[x0,#0,mul vl\]'
[^ :]+:[0-9]+: Error: expected a 64-bit base register at operand 2 -- `ldr zt0,\[w0\]'
[^ :]+:[0-9]+: Error: missing offset in the pre-indexed address at operand 2 -- `ldr zt0,\[x0\]!'
[^ :]+:[0-9]+: Error: invalid base register at operand 2 -- `ldr zt0,\[xzr\]'
[^ :]+:[0-9]+: Error: expected a 64-bit base register at operand 2 -- `ldr zt0,\[wsp\]'
[^ :]+:[0-9]+: Error: invalid addressing mode at operand 2 -- `ldr zt0,\[x0,xzr\]'
[^ :]+:[0-9]+: Error: invalid addressing mode at operand 2 -- `ldr zt0,\[x1,x2\]'
[^ :]+:[0-9]+: Error: register element index out of range 0 to 15 at operand 3 -- `luti2 z0\.b,zt0,z0\[-1\]'
[^ :]+:[0-9]+: Error: register element index out of range 0 to 15 at operand 3 -- `luti2 z0\.b,zt0,z0\[16\]'
[^ :]+:[0-9]+: Error: operand mismatch -- `luti2 z0\.b,zt0,z0\.b\[0\]'
[^ :]+:[0-9]+: Info:    did you mean this\?
[^ :]+:[0-9]+: Info:    	luti2 z0\.b, zt0, z0\[0\]
[^ :]+:[0-9]+: Info:    other valid variant\(s\):
[^ :]+:[0-9]+: Info:    	luti2 z0\.h, zt0, z0\[0\]
[^ :]+:[0-9]+: Info:    	luti2 z0\.s, zt0, z0\[0\]
[^ :]+:[0-9]+: Error: operand mismatch -- `luti2 z0,zt0,z0\[0\]'
[^ :]+:[0-9]+: Info:    did you mean this\?
[^ :]+:[0-9]+: Info:    	luti2 z0\.b, zt0, z0\[0\]
[^ :]+:[0-9]+: Info:    other valid variant\(s\):
[^ :]+:[0-9]+: Info:    	luti2 z0\.h, zt0, z0\[0\]
[^ :]+:[0-9]+: Info:    	luti2 z0\.s, zt0, z0\[0\]
[^ :]+:[0-9]+: Error: operand mismatch -- `luti2 z0\.d,zt0,z0\[0\]'
[^ :]+:[0-9]+: Info:    did you mean this\?
[^ :]+:[0-9]+: Info:    	luti2 z0\.b, zt0, z0\[0\]
[^ :]+:[0-9]+: Info:    other valid variant\(s\):
[^ :]+:[0-9]+: Info:    	luti2 z0\.h, zt0, z0\[0\]
[^ :]+:[0-9]+: Info:    	luti2 z0\.s, zt0, z0\[0\]
[^ :]+:[0-9]+: Error: operand mismatch -- `luti2 z0\.q,zt0,z0\[0\]'
[^ :]+:[0-9]+: Info:    did you mean this\?
[^ :]+:[0-9]+: Info:    	luti2 z0\.b, zt0, z0\[0\]
[^ :]+:[0-9]+: Info:    other valid variant\(s\):
[^ :]+:[0-9]+: Info:    	luti2 z0\.h, zt0, z0\[0\]
[^ :]+:[0-9]+: Info:    	luti2 z0\.s, zt0, z0\[0\]
[^ :]+:[0-9]+: Error: expected an SVE vector register at operand 3 -- `luti2 z0\.b,zt0,zt0'
[^ :]+:[0-9]+: Error: expected a register or register list at operand 1 -- `luti2 0,zt0,z0\[0\]'
[^ :]+:[0-9]+: Error: expected a register at operand 2 -- `luti2 z0\.b,0,z0\[0\]'
[^ :]+:[0-9]+: Error: expected an SVE vector register at operand 3 -- `luti2 z0\.b,zt0,0'
[^ :]+:[0-9]+: Error: start register out of range at operand 1 -- `luti2 {z1\.b-z2\.b},zt0,z0\[0\]'
[^ :]+:[0-9]+: Error: unexpected register type at operand 2 -- `luti2 {z0\.b-z1\.b},z0,z0\[0\]'
[^ :]+:[0-9]+: Error: unexpected register type at operand 2 -- `luti2 {z0\.b-z1\.b},za,z0\[0\]'
[^ :]+:[0-9]+: Error: operand mismatch -- `luti2 {z0\.h-z1\.h},zt0,z0\.h\[0\]'
[^ :]+:[0-9]+: Info:    did you mean this\?
[^ :]+:[0-9]+: Info:    	luti2 {z0\.h-z1\.h}, zt0, z0\[0\]
[^ :]+:[0-9]+: Info:    other valid variant\(s\):
[^ :]+:[0-9]+: Info:    	luti2 {z0\.b-z1\.b}, zt0, z0\[0\]
[^ :]+:[0-9]+: Info:    	luti2 {z0\.s-z1\.s}, zt0, z0\[0\]
[^ :]+:[0-9]+: Error: register element index out of range 0 to 7 at operand 3 -- `luti2 {z0\.h-z1\.h},zt0,z0\[-1\]'
[^ :]+:[0-9]+: Error: register element index out of range 0 to 7 at operand 3 -- `luti2 {z0\.h-z1\.h},zt0,z0\[8\]'
[^ :]+:[0-9]+: Error: operand mismatch -- `luti2 {z0\.d-z1\.d},zt0,z0\[0\]'
[^ :]+:[0-9]+: Info:    did you mean this\?
[^ :]+:[0-9]+: Info:    	luti2 {z0\.b-z1\.b}, zt0, z0\[0\]
[^ :]+:[0-9]+: Info:    other valid variant\(s\):
[^ :]+:[0-9]+: Info:    	luti2 {z0\.h-z1\.h}, zt0, z0\[0\]
[^ :]+:[0-9]+: Info:    	luti2 {z0\.s-z1\.s}, zt0, z0\[0\]
[^ :]+:[0-9]+: Error: operand mismatch -- `luti2 {z0\.q-z1\.q},zt0,z0\[0\]'
[^ :]+:[0-9]+: Info:    did you mean this\?
[^ :]+:[0-9]+: Info:    	luti2 {z0\.b-z1\.b}, zt0, z0\[0\]
[^ :]+:[0-9]+: Info:    other valid variant\(s\):
[^ :]+:[0-9]+: Info:    	luti2 {z0\.h-z1\.h}, zt0, z0\[0\]
[^ :]+:[0-9]+: Info:    	luti2 {z0\.s-z1\.s}, zt0, z0\[0\]
[^ :]+:[0-9]+: Error: start register out of range at operand 1 -- `luti2 {z1\.s-z4\.s},zt0,z0\[0\]'
[^ :]+:[0-9]+: Error: start register out of range at operand 1 -- `luti2 {z2\.s-z5\.s},zt0,z0\[0\]'
[^ :]+:[0-9]+: Error: start register out of range at operand 1 -- `luti2 {z3\.s-z6\.s},zt0,z0\[0\]'
[^ :]+:[0-9]+: Error: unexpected register type at operand 2 -- `luti2 {z0\.s-z3\.s},z0,z0\[0\]'
[^ :]+:[0-9]+: Error: unexpected register type at operand 2 -- `luti2 {z0\.b-z3\.b},za,z0\[0\]'
[^ :]+:[0-9]+: Error: operand mismatch -- `luti2 {z0\.b-z3\.b},zt0,z0\.b\[0\]'
[^ :]+:[0-9]+: Info:    did you mean this\?
[^ :]+:[0-9]+: Info:    	luti2 {z0\.b-z3\.b}, zt0, z0\[0\]
[^ :]+:[0-9]+: Info:    other valid variant\(s\):
[^ :]+:[0-9]+: Info:    	luti2 {z0\.h-z3\.h}, zt0, z0\[0\]
[^ :]+:[0-9]+: Info:    	luti2 {z0\.s-z3\.s}, zt0, z0\[0\]
[^ :]+:[0-9]+: Error: register element index out of range 0 to 3 at operand 3 -- `luti2 {z0\.b-z3\.b},zt0,z0\[-1\]'
[^ :]+:[0-9]+: Error: register element index out of range 0 to 3 at operand 3 -- `luti2 {z0\.b-z3\.b},zt0,z0\[4\]'
[^ :]+:[0-9]+: Error: operand mismatch -- `luti2 {z0\.d-z3\.d},zt0,z0\[0\]'
[^ :]+:[0-9]+: Info:    did you mean this\?
[^ :]+:[0-9]+: Info:    	luti2 {z0\.b-z3\.b}, zt0, z0\[0\]
[^ :]+:[0-9]+: Info:    other valid variant\(s\):
[^ :]+:[0-9]+: Info:    	luti2 {z0\.h-z3\.h}, zt0, z0\[0\]
[^ :]+:[0-9]+: Info:    	luti2 {z0\.s-z3\.s}, zt0, z0\[0\]
[^ :]+:[0-9]+: Error: operand mismatch -- `luti2 {z0\.q-z3\.q},zt0,z0\[0\]'
[^ :]+:[0-9]+: Info:    did you mean this\?
[^ :]+:[0-9]+: Info:    	luti2 {z0\.b-z3\.b}, zt0, z0\[0\]
[^ :]+:[0-9]+: Info:    other valid variant\(s\):
[^ :]+:[0-9]+: Info:    	luti2 {z0\.h-z3\.h}, zt0, z0\[0\]
[^ :]+:[0-9]+: Info:    	luti2 {z0\.s-z3\.s}, zt0, z0\[0\]
[^ :]+:[0-9]+: Error: expected a register or register list at operand 1 -- `luti4 0,zt0,z0\[0\]'
[^ :]+:[0-9]+: Error: expected a register at operand 2 -- `luti4 z0\.b,0,z0\[0\]'
[^ :]+:[0-9]+: Error: expected an SVE vector register at operand 3 -- `luti4 z0\.b,zt0,0'
[^ :]+:[0-9]+: Error: register element index out of range 0 to 7 at operand 3 -- `luti4 z0\.h,zt0,z0\[-1\]'
[^ :]+:[0-9]+: Error: register element index out of range 0 to 7 at operand 3 -- `luti4 z0\.h,zt0,z0\[8\]'
[^ :]+:[0-9]+: Error: operand mismatch -- `luti4 z0\.h,zt0,z0\.h\[0\]'
[^ :]+:[0-9]+: Info:    did you mean this\?
[^ :]+:[0-9]+: Info:    	luti4 z0\.h, zt0, z0\[0\]
[^ :]+:[0-9]+: Info:    other valid variant\(s\):
[^ :]+:[0-9]+: Info:    	luti4 z0\.b, zt0, z0\[0\]
[^ :]+:[0-9]+: Info:    	luti4 z0\.s, zt0, z0\[0\]
[^ :]+:[0-9]+: Error: operand mismatch -- `luti4 z0,zt0,z0\[0\]'
[^ :]+:[0-9]+: Info:    did you mean this\?
[^ :]+:[0-9]+: Info:    	luti4 z0\.b, zt0, z0\[0\]
[^ :]+:[0-9]+: Info:    other valid variant\(s\):
[^ :]+:[0-9]+: Info:    	luti4 z0\.h, zt0, z0\[0\]
[^ :]+:[0-9]+: Info:    	luti4 z0\.s, zt0, z0\[0\]
[^ :]+:[0-9]+: Error: operand mismatch -- `luti4 z0\.d,zt0,z0\[0\]'
[^ :]+:[0-9]+: Info:    did you mean this\?
[^ :]+:[0-9]+: Info:    	luti4 z0\.b, zt0, z0\[0\]
[^ :]+:[0-9]+: Info:    other valid variant\(s\):
[^ :]+:[0-9]+: Info:    	luti4 z0\.h, zt0, z0\[0\]
[^ :]+:[0-9]+: Info:    	luti4 z0\.s, zt0, z0\[0\]
[^ :]+:[0-9]+: Error: operand mismatch -- `luti4 z0\.q,zt0,z0\[0\]'
[^ :]+:[0-9]+: Info:    did you mean this\?
[^ :]+:[0-9]+: Info:    	luti4 z0\.b, zt0, z0\[0\]
[^ :]+:[0-9]+: Info:    other valid variant\(s\):
[^ :]+:[0-9]+: Info:    	luti4 z0\.h, zt0, z0\[0\]
[^ :]+:[0-9]+: Info:    	luti4 z0\.s, zt0, z0\[0\]
[^ :]+:[0-9]+: Error: expected an SVE vector register at operand 3 -- `luti4 z0\.h,zt0,zt0'
[^ :]+:[0-9]+: Error: start register out of range at operand 1 -- `luti4 {z1\.h-z2\.h},zt0,z0\[0\]'
[^ :]+:[0-9]+: Error: unexpected register type at operand 2 -- `luti4 {z0\.h-z1\.h},z0,z0\[0\]'
[^ :]+:[0-9]+: Error: unexpected register type at operand 2 -- `luti4 {z0\.h-z1\.h},za,z0\[0\]'
[^ :]+:[0-9]+: Error: operand mismatch -- `luti4 {z0\.h-z1\.h},zt0,z0\.h\[0\]'
[^ :]+:[0-9]+: Info:    did you mean this\?
[^ :]+:[0-9]+: Info:    	luti4 {z0\.h-z1\.h}, zt0, z0\[0\]
[^ :]+:[0-9]+: Info:    other valid variant\(s\):
[^ :]+:[0-9]+: Info:    	luti4 {z0\.b-z1\.b}, zt0, z0\[0\]
[^ :]+:[0-9]+: Info:    	luti4 {z0\.s-z1\.s}, zt0, z0\[0\]
[^ :]+:[0-9]+: Error: register element index out of range 0 to 3 at operand 3 -- `luti4 {z0\.h-z1\.h},zt0,z0\[-1\]'
[^ :]+:[0-9]+: Error: register element index out of range 0 to 3 at operand 3 -- `luti4 {z0\.h-z1\.h},zt0,z0\[4\]'
[^ :]+:[0-9]+: Error: operand mismatch -- `luti4 {z0\.d-z1\.d},zt0,z0\[0\]'
[^ :]+:[0-9]+: Info:    did you mean this\?
[^ :]+:[0-9]+: Info:    	luti4 {z0\.b-z1\.b}, zt0, z0\[0\]
[^ :]+:[0-9]+: Info:    other valid variant\(s\):
[^ :]+:[0-9]+: Info:    	luti4 {z0\.h-z1\.h}, zt0, z0\[0\]
[^ :]+:[0-9]+: Info:    	luti4 {z0\.s-z1\.s}, zt0, z0\[0\]
[^ :]+:[0-9]+: Error: operand mismatch -- `luti4 {z0\.q-z1\.q},zt0,z0\[0\]'
[^ :]+:[0-9]+: Info:    did you mean this\?
[^ :]+:[0-9]+: Info:    	luti4 {z0\.b-z1\.b}, zt0, z0\[0\]
[^ :]+:[0-9]+: Info:    other valid variant\(s\):
[^ :]+:[0-9]+: Info:    	luti4 {z0\.h-z1\.h}, zt0, z0\[0\]
[^ :]+:[0-9]+: Info:    	luti4 {z0\.s-z1\.s}, zt0, z0\[0\]
[^ :]+:[0-9]+: Error: start register out of range at operand 1 -- `luti4 {z1\.s-z4\.s},zt0,z0\[0\]'
[^ :]+:[0-9]+: Error: start register out of range at operand 1 -- `luti4 {z2\.s-z5\.s},zt0,z0\[0\]'
[^ :]+:[0-9]+: Error: start register out of range at operand 1 -- `luti4 {z3\.s-z6\.s},zt0,z0\[0\]'
[^ :]+:[0-9]+: Error: unexpected register type at operand 2 -- `luti4 {z0\.s-z3\.s},z0,z0\[0\]'
[^ :]+:[0-9]+: Error: unexpected register type at operand 2 -- `luti4 {z0\.s-z3\.s},za,z0\[0\]'
[^ :]+:[0-9]+: Error: operand mismatch -- `luti4 {z0\.s-z3\.s},zt0,z0\.s\[0\]'
[^ :]+:[0-9]+: Info:    did you mean this\?
[^ :]+:[0-9]+: Info:    	luti4 {z0\.s-z3\.s}, zt0, z0\[0\]
[^ :]+:[0-9]+: Info:    other valid variant\(s\):
[^ :]+:[0-9]+: Info:    	luti4 {z0\.b-z3\.b}, zt0, z0\[0\]
[^ :]+:[0-9]+: Info:    	luti4 {z0\.h-z3\.h}, zt0, z0\[0\]
[^ :]+:[0-9]+: Error: register element index out of range 0 to 1 at operand 3 -- `luti4 {z0\.s-z3\.s},zt0,z0\[-1\]'
[^ :]+:[0-9]+: Error: register element index out of range 0 to 1 at operand 3 -- `luti4 {z0\.s-z3\.s},zt0,z0\[2\]'
[^ :]+:[0-9]+: Error: expected a list of 2 registers at operand 1 -- `luti4 {z0\.b-z3\.b},zt0,z0\[0\]'
[^ :]+:[0-9]+: Error: operand mismatch -- `luti4 {z0\.d-z3\.d},zt0,z0\[0\]'
[^ :]+:[0-9]+: Info:    did you mean this\?
[^ :]+:[0-9]+: Info:    	luti4 {z0\.b-z3\.b}, zt0, z0\[0\]
[^ :]+:[0-9]+: Info:    other valid variant\(s\):
[^ :]+:[0-9]+: Info:    	luti4 {z0\.h-z3\.h}, zt0, z0\[0\]
[^ :]+:[0-9]+: Info:    	luti4 {z0\.s-z3\.s}, zt0, z0\[0\]
[^ :]+:[0-9]+: Error: operand mismatch -- `luti4 {z0\.q-z3\.q},zt0,z0\[0\]'
[^ :]+:[0-9]+: Info:    did you mean this\?
[^ :]+:[0-9]+: Info:    	luti4 {z0\.b-z3\.b}, zt0, z0\[0\]
[^ :]+:[0-9]+: Info:    other valid variant\(s\):
[^ :]+:[0-9]+: Info:    	luti4 {z0\.h-z3\.h}, zt0, z0\[0\]
[^ :]+:[0-9]+: Info:    	luti4 {z0\.s-z3\.s}, zt0, z0\[0\]
