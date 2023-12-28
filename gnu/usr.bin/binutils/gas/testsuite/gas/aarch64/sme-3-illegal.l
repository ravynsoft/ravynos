[^:]*: Assembler messages:
[^:]*:[0-9]+: Error: ZA tile number out of range at operand 1 -- `mova za1v\.b\[w12,#0\],p0/m,z0.b'
[^:]*:[0-9]+: Error: ZA tile number out of range at operand 1 -- `mova za2v\.h\[w12,#0\],p0/m,z0.h'
[^:]*:[0-9]+: Error: ZA tile number out of range at operand 1 -- `mova za4v\.s\[w12,#0\],p0/m,z0.s'
[^:]*:[0-9]+: Error: ZA tile number out of range at operand 1 -- `mova za8v\.d\[w12,#0\],p0/m,z0.d'
[^:]*:[0-9]+: Error: expected a register or register list at operand 1 -- `mova za16v\.q\[w12\],p0/m,z0.q'
[^:]*:[0-9]+: Error: immediate offset out of range 0 to 15 at operand 1 -- `mova za0v\.b\[w15,#16\],p7/m,z31.b'
[^:]*:[0-9]+: Error: immediate offset out of range 0 to 7 at operand 1 -- `mova za1v\.h\[w15,#8\],p7/m,z31.h'
[^:]*:[0-9]+: Error: immediate offset out of range 0 to 3 at operand 1 -- `mova za3v\.s\[w15,#4\],p7/m,z31.s'
[^:]*:[0-9]+: Error: immediate offset out of range 0 to 1 at operand 1 -- `mova za7v\.d\[w15,#2\],p7/m,z31.d'
[^:]*:[0-9]+: Error: immediate offset must be 0 at operand 1 -- `mova za15v\.q\[w15,#1\],p7/m,z31.q'
[^:]*:[0-9]+: Error: unexpected vector group size at operand 1 -- `mova za0h\.b\[w12,#0,vgx2\],p0/m,z0\.b'
[^:]*:[0-9]+: Error: unexpected vector group size at operand 1 -- `mova za0h\.b\[w12,#0,vgx4\],p0/m,z0\.b'
[^:]*:[0-9]+: Error: invalid vector group size at operand 1 -- `mova za0h\.b\[w12,#0,vgx8\],p0/m,z0\.b'
[^:]*:[0-9]+: Error: operand mismatch -- `mova za0h\.b\[w12,#0,vgx2\],p0/z,z0\.b'
[^:]*:[0-9]+: Info:    did you mean this\?
[^:]*:[0-9]+: Info:    	mova za0h\.b\[w12, 0, vgx2\], p0/m, z0\.b
[^:]*:[0-9]+: Info:    other valid variant\(s\):
[^:]*:[0-9]+: Info:    	mova za0h\.h\[w12, 0, vgx2\], p0/m, z0\.h
[^:]*:[0-9]+: Info:    	mova za0h\.s\[w12, 0, vgx2\], p0/m, z0\.s
[^:]*:[0-9]+: Info:    	mova za0h\.d\[w12, 0, vgx2\], p0/m, z0\.d
[^:]*:[0-9]+: Info:    	mova za0h\.q\[w12, 0, vgx2\], p0/m, z0\.q
