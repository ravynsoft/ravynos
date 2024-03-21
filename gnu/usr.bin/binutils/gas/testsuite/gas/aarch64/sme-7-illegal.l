[^:]*: Assembler messages:
[^:]*:[0-9]+: Error: expected a selection register in the range w12-w15 at operand 1 -- `ldr za\[w11,0\],\[x0\]'
[^:]*:[0-9]+: Error: invalid addressing mode at operand 2 -- `ldr za\[w12,1\],\[sp,x0\]'
[^:]*:[0-9]+: Error: operand 2 must have the same immediate value as operand 1 -- `ldr za\[w12,0\],\[sp,#1,mul vl\]'
[^:]*:[0-9]+: Error: operand 2 must have the same immediate value as operand 1 -- `ldr za\[w13,9\],\[x17,#19,mul vl\]'
[^:]*:[0-9]+: Error: immediate offset out of range 0 to 15 at operand 1 -- `ldr za\[w13,21\],\[x17,#21,mul vl\]'
[^:]*:[0-9]+: Error: operand 2 must have the same immediate value as operand 1 -- `ldr za\[w15,32\],\[x17,#15,mul vl\]'
[^:]*:[0-9]+: Error: expected a selection register in the range w12-w15 at operand 1 -- `ldr za\[w16,15\],\[sp,#15,mul vl\]'
[^:]*:[0-9]+: Error: only 'MUL VL' is permitted at operand 2 -- `ldr za\[w12,0\],\[x0,#0,mul#1\]'
[^:]*:[0-9]+: Error: only 'MUL VL' is permitted at operand 2 -- `ldr za\[w13,0\],\[sp,#0,mul#2\]'
[^:]*:[0-9]+: Error: only 'MUL VL' is permitted at operand 2 -- `ldr za\[w14,9\],\[x17,#9,mul#3\]'
[^:]*:[0-9]+: Error: only 'MUL VL' is permitted at operand 2 -- `ldr za\[w15,15\],\[sp,#15,mul#4\]'
[^:]*:[0-9]+: Error: expected a selection register in the range w12-w15 at operand 1 -- `str za\[w11,0\],\[x0\]'
[^:]*:[0-9]+: Error: invalid addressing mode at operand 2 -- `str za\[w12,1\],\[sp,x0\]'
[^:]*:[0-9]+: Error: operand 2 must have the same immediate value as operand 1 -- `str za\[w12,0\],\[sp,#1,mul vl\]'
[^:]*:[0-9]+: Error: operand 2 must have the same immediate value as operand 1 -- `str za\[w13,9\],\[x17,#19,mul vl\]'
[^:]*:[0-9]+: Error: immediate offset out of range 0 to 15 at operand 1 -- `str za\[w13,21\],\[x17,#21,mul vl\]'
[^:]*:[0-9]+: Error: operand 2 must have the same immediate value as operand 1 -- `str za\[w15,32\],\[x17,#15,mul vl\]'
[^:]*:[0-9]+: Error: expected a selection register in the range w12-w15 at operand 1 -- `str za\[w16,15\],\[sp,#15,mul vl\]'
[^:]*:[0-9]+: Error: only 'MUL VL' is permitted at operand 2 -- `str za\[w12,0\],\[x0,#0,mul#1\]'
[^:]*:[0-9]+: Error: only 'MUL VL' is permitted at operand 2 -- `str za\[w13,0\],\[sp,#0,mul#2\]'
[^:]*:[0-9]+: Error: only 'MUL VL' is permitted at operand 2 -- `str za\[w14,9\],\[x17,#9,mul#3\]'
[^:]*:[0-9]+: Error: only 'MUL VL' is permitted at operand 2 -- `str za\[w15,15\],\[sp,#15,mul#4\]'
[^:]*:[0-9]+: Error: operand 2 must have the same immediate value as operand 1 -- `ldr za\[w13,13\],\[x17,#23,mul vl\]'
[^:]*:[0-9]+: Error: operand 2 must have the same immediate value as operand 1 -- `str za\[w13,13\],\[x17,#23,mul vl\]'
[^:]*:[0-9]+: Error: operand 2 must have the same immediate value as operand 1 -- `ldr za\[w13,23\],\[x17,#13,mul vl\]'
[^:]*:[0-9]+: Error: operand 2 must have the same immediate value as operand 1 -- `str za\[w13,23\],\[x17,#13,mul vl\]'
[^:]*:[0-9]+: Error: immediate offset out of range 0 to 15 at operand 1 -- `ldr za\[w13,16\],\[x17,#16,mul vl\]'
[^:]*:[0-9]+: Error: immediate offset out of range 0 to 15 at operand 1 -- `str za\[w13,16\],\[x17,#16,mul vl\]'
[^:]*:[0-9]+: Error: operand 2 must have the same immediate value as operand 1 -- `ldr za\[w13,-1\],\[x17,#1,mul vl\]'
[^:]*:[0-9]+: Error: operand 2 must have the same immediate value as operand 1 -- `str za\[w13,-1\],\[x17,#1,mul vl\]'
[^:]*:[0-9]+: Error: operand 2 must have the same immediate value as operand 1 -- `ldr za\[w13,1\],\[x17,#-1,mul vl\]'
[^:]*:[0-9]+: Error: operand 2 must have the same immediate value as operand 1 -- `str za\[w13,1\],\[x17,#-1,mul vl\]'
[^:]*:[0-9]+: Error: operand mismatch -- `ldr za.b\[w12,0\],\[x0\]'
[^:]*:[0-9]+: Info:    did you mean this\?
[^:]*:[0-9]+: Info:    	ldr za\[w12, 0\], \[x0\]
[^:]*:[0-9]+: Error: operand mismatch -- `ldr za.h\[w12,0\],\[x0\]'
[^:]*:[0-9]+: Info:    did you mean this\?
[^:]*:[0-9]+: Info:    	ldr za\[w12, 0\], \[x0\]
[^:]*:[0-9]+: Error: operand mismatch -- `ldr za.s\[w12,0\],\[x0\]'
[^:]*:[0-9]+: Info:    did you mean this\?
[^:]*:[0-9]+: Info:    	ldr za\[w12, 0\], \[x0\]
[^:]*:[0-9]+: Error: operand mismatch -- `ldr za.d\[w12,0\],\[x0\]'
[^:]*:[0-9]+: Info:    did you mean this\?
[^:]*:[0-9]+: Info:    	ldr za\[w12, 0\], \[x0\]
[^:]*:[0-9]+: Error: operand mismatch -- `ldr za.q\[w12,0\],\[x0\]'
[^:]*:[0-9]+: Info:    did you mean this\?
[^:]*:[0-9]+: Info:    	ldr za\[w12, 0\], \[x0\]
[^:]*:[0-9]+: Error: expected '\[' at operand 1 -- `ldr za/z\[w12,0\],\[x0\]'
[^:]*:[0-9]+: Error: unexpected character `2' in element size at operand 1 -- `ldr za.2b\[w12,0\],\[x0\]'
[^:]*:[0-9]+: Error: expected 'za' rather than a ZA tile at operand 1 -- `ldr za0\[w12,0\],\[x0\]'
[^:]*:[0-9]+: Error: expected 'za' rather than a ZA tile at operand 1 -- `ldr za0.b\[w12,0\],\[x0\]'
[^:]*:[0-9]+: Error: expected 'za' rather than a ZA tile at operand 1 -- `ldr za0h\[w12,0\],\[x0\]'
[^:]*:[0-9]+: Error: expected 'za' rather than a ZA tile at operand 1 -- `ldr za0h.h\[w12,0\],\[x0\]'
[^:]*:[0-9]+: Error: expected 'za' rather than a ZA tile at operand 1 -- `ldr za0v\[w12,0\],\[x0\]'
[^:]*:[0-9]+: Error: expected 'za' rather than a ZA tile at operand 1 -- `ldr za0v.s\[w12,0\],\[x0\]'
[^:]*:[0-9]+: Error: unexpected vector group size at operand 1 -- `ldr za\[w12,0,vgx2\],\[x0\]'
[^:]*:[0-9]+: Error: unexpected vector group size at operand 1 -- `ldr za\[w12,0,vgx4\],\[x0\]'
[^:]*:[0-9]+: Error: invalid vector group size at operand 1 -- `ldr za\[w12,0,vgx8\],\[x0\]'
[^:]*:[0-9]+: Error: unexpected vector group size at operand 1 -- `str za\[w12,0,vgx2\],\[x0\]'
[^:]*:[0-9]+: Error: unexpected vector group size at operand 1 -- `str za\[w12,0,vgx4\],\[x0\]'
[^:]*:[0-9]+: Error: invalid vector group size at operand 1 -- `str za\[w12,0,vgx8\],\[x0\]'
[^:]*:[0-9]+: Error: operand mismatch -- `ldr za\.b\[w12,0,vgx2\],\[x0\]'
[^:]*:[0-9]+: Info:    did you mean this\?
[^:]*:[0-9]+: Info:    	ldr za\[w12, 0, vgx2\], \[x0\]
[^:]*:[0-9]+: Error: operand mismatch -- `str za\.b\[w12,0,vgx4\],\[x0\]'
[^:]*:[0-9]+: Info:    did you mean this\?
[^:]*:[0-9]+: Info:    	str za\[w12, 0, vgx4\], \[x0\]
[^:]*:[0-9]+: Error: expected a single offset rather than a range at operand 1 -- `ldr za\[w12,0:1\],\[x0\]'
[^:]*:[0-9]+: Error: expected a single offset rather than a range at operand 1 -- `str za\[w12,0:2,vgx4\],\[x0\]'
[^:]*:[0-9]+: Error: operand mismatch -- `ldr za\.b\[w12,0:1\],\[x0\]'
[^:]*:[0-9]+: Info:    did you mean this\?
[^:]*:[0-9]+: Info:    	ldr za\[w12, 0:1\], \[x0\]
[^:]*:[0-9]+: Error: operand mismatch -- `str za\.b\[w12,0:2,vgx4\],\[x0\]'
[^:]*:[0-9]+: Info:    did you mean this\?
[^:]*:[0-9]+: Info:    	str za\[w12, 0:2, vgx4\], \[x0\]
