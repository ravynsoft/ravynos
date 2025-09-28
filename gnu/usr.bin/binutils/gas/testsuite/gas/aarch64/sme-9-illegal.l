[^:]*: Assembler messages:
[^:]*:[0-9]+: Error: missing immediate offset at operand 3 -- `psel p1,p15,p3.b\[w12\]'
[^:]*:[0-9]+: Error: missing immediate offset at operand 3 -- `psel p1,p15,p3.q\[w15\]'
[^:]*:[0-9]+: Error: operand mismatch -- `psel p1,p15,p3.q\[w15,#0\]'
[^:]*:[0-9]+: Info:    did you mean this\?
[^:]*:[0-9]+: Info:    	psel p1, p15, p3.b\[w15, 0\]
[^:]*:[0-9]+: Info:    other valid variant\(s\):
[^:]*:[0-9]+: Info:    	psel p1, p15, p3.h\[w15, 0\]
[^:]*:[0-9]+: Info:    	psel p1, p15, p3.s\[w15, 0\]
[^:]*:[0-9]+: Info:    	psel p1, p15, p3.d\[w15, 0\]
[^:]*:[0-9]+: Error: operand mismatch -- `psel p1,p15,p3\[w15,#0\]'
[^:]*:[0-9]+: Info:    did you mean this\?
[^:]*:[0-9]+: Info:    	psel p1, p15, p3.b\[w15, 0\]
[^:]*:[0-9]+: Info:    other valid variant\(s\):
[^:]*:[0-9]+: Info:    	psel p1, p15, p3.h\[w15, 0\]
[^:]*:[0-9]+: Info:    	psel p1, p15, p3.s\[w15, 0\]
[^:]*:[0-9]+: Info:    	psel p1, p15, p3.d\[w15, 0\]
[^:]*:[0-9]+: Error: missing immediate offset at operand 3 -- `psel p1,p15,p3.b\[w11\]'
[^:]*:[0-9]+: Error: missing immediate offset at operand 3 -- `psel p8,p11,p15.h\[w16\]'
[^:]*:[0-9]+: Error: missing immediate offset at operand 3 -- `psel p2,p7,p15.s\[w3\]'
[^:]*:[0-9]+: Error: missing immediate offset at operand 3 -- `psel p13,p3,p1.d\[w17\]'
[^:]*:[0-9]+: Error: expected a selection register in the range w12-w15 at operand 3 -- `psel p1,p15,p3.b\[w11,#0\]'
[^:]*:[0-9]+: Error: expected a selection register in the range w12-w15 at operand 3 -- `psel p8,p11,p15.h\[w16,#0\]'
[^:]*:[0-9]+: Error: expected a selection register in the range w12-w15 at operand 3 -- `psel p2,p7,p15.s\[w3,#0\]'
[^:]*:[0-9]+: Error: expected a selection register in the range w12-w15 at operand 3 -- `psel p13,p3,p1.d\[w17,#0\]'
[^:]*:[0-9]+: Error: immediate offset out of range 0 to 15 at operand 3 -- `psel p5,p12,p9.b\[w15,#16\]'
[^:]*:[0-9]+: Error: immediate offset out of range 0 to 7 at operand 3 -- `psel p1,p8,p6.h\[w14,#8\]'
[^:]*:[0-9]+: Error: immediate offset out of range 0 to 3 at operand 3 -- `psel p8,p4,p15.s\[w13,#4\]'
[^:]*:[0-9]+: Error: immediate offset out of range 0 to 1 at operand 3 -- `psel p1,p1,p1.d\[w12,#2\]'
[^:]*:[0-9]+: Error: unexpected vector group size at operand 3 -- `psel p0,p0,p0\.b\[w12,#0,vgx2\]'
[^:]*:[0-9]+: Error: unexpected vector group size at operand 3 -- `psel p0,p0,p0\.b\[w12,#0,vgx4\]'
[^:]*:[0-9]+: Error: invalid vector group size at operand 3 -- `psel p0,p0,p0\.b\[w12,#0,vgx8\]'
[^:]*:[0-9]+: Error: operand mismatch -- `psel p0\.b,p0\.b,p0\.b\[w12,#0,vgx2\]'
[^:]*:[0-9]+: Info:    did you mean this\?
[^:]*:[0-9]+: Info:    	psel p0, p0, p0\.b\[w12, 0\]
[^:]*:[0-9]+: Info:    other valid variant\(s\):
[^:]*:[0-9]+: Info:    	psel p0, p0, p0\.h\[w12, 0\]
[^:]*:[0-9]+: Info:    	psel p0, p0, p0\.s\[w12, 0\]
[^:]*:[0-9]+: Info:    	psel p0, p0, p0\.d\[w12, 0\]
[^:]*:[0-9]+: Error: expected a single offset rather than a range at operand 3 -- `psel p0,p0,p0\.b\[w12,0:1\]'
[^:]*:[0-9]+: Error: expected a single offset rather than a range at operand 3 -- `psel p0,p0,p0\.b\[w12,0:1,vgx2\]'
[^:]*:[0-9]+: Error: operand mismatch -- `psel p0\.b,p0\.b,p0\.b\[w12,0:1,vgx2\]'
[^:]*:[0-9]+: Info:    did you mean this\?
[^:]*:[0-9]+: Info:    	psel p0, p0, p0\.b\[w12, 0\]
[^:]*:[0-9]+: Info:    other valid variant\(s\):
[^:]*:[0-9]+: Info:    	psel p0, p0, p0\.h\[w12, 0\]
[^:]*:[0-9]+: Info:    	psel p0, p0, p0\.s\[w12, 0\]
[^:]*:[0-9]+: Info:    	psel p0, p0, p0\.d\[w12, 0\]
[^:]*:[0-9]+: Error: operand mismatch -- `revd z0.q,p0/m,z0.b'
[^:]*:[0-9]+: Info:    did you mean this\?
[^:]*:[0-9]+: Info:    	revd z0.q, p0/m, z0.q
[^:]*:[0-9]+: Error: operand mismatch -- `sclamp z8.b,z1.b,z31.q'
[^:]*:[0-9]+: Info:    did you mean this\?
[^:]*:[0-9]+: Info:    	sclamp z8.b, z1.b, z31.b
[^:]*:[0-9]+: Info:    other valid variant\(s\):
[^:]*:[0-9]+: Info:    	sclamp z8.h, z1.h, z31.h
[^:]*:[0-9]+: Info:    	sclamp z8.s, z1.s, z31.s
[^:]*:[0-9]+: Info:    	sclamp z8.d, z1.d, z31.d
[^:]*:[0-9]+: Error: operand mismatch -- `sclamp z31.h,z0.h,z17.q'
[^:]*:[0-9]+: Info:    did you mean this\?
[^:]*:[0-9]+: Info:    	sclamp z31.h, z0.h, z17.h
[^:]*:[0-9]+: Info:    other valid variant\(s\):
[^:]*:[0-9]+: Info:    	sclamp z31.b, z0.b, z17.b
[^:]*:[0-9]+: Info:    	sclamp z31.s, z0.s, z17.s
[^:]*:[0-9]+: Info:    	sclamp z31.d, z0.d, z17.d
[^:]*:[0-9]+: Error: operand mismatch -- `sclamp z0.s,z31.s,z17.q'
[^:]*:[0-9]+: Info:    did you mean this\?
[^:]*:[0-9]+: Info:    	sclamp z0.s, z31.s, z17.s
[^:]*:[0-9]+: Info:    other valid variant\(s\):
[^:]*:[0-9]+: Info:    	sclamp z0.b, z31.b, z17.b
[^:]*:[0-9]+: Info:    	sclamp z0.h, z31.h, z17.h
[^:]*:[0-9]+: Info:    	sclamp z0.d, z31.d, z17.d
[^:]*:[0-9]+: Error: operand mismatch -- `sclamp z31.d,z0.d,z17.q'
[^:]*:[0-9]+: Info:    did you mean this\?
[^:]*:[0-9]+: Info:    	sclamp z31.d, z0.d, z17.d
[^:]*:[0-9]+: Info:    other valid variant\(s\):
[^:]*:[0-9]+: Info:    	sclamp z31.b, z0.b, z17.b
[^:]*:[0-9]+: Info:    	sclamp z31.h, z0.h, z17.h
[^:]*:[0-9]+: Info:    	sclamp z31.s, z0.s, z17.s
[^:]*:[0-9]+: Error: operand mismatch -- `sclamp z31.q,z0.d,z17.q'
[^:]*:[0-9]+: Info:    did you mean this\?
[^:]*:[0-9]+: Info:    	sclamp z31.d, z0.d, z17.d
[^:]*:[0-9]+: Info:    other valid variant\(s\):
[^:]*:[0-9]+: Info:    	sclamp z31.b, z0.b, z17.b
[^:]*:[0-9]+: Info:    	sclamp z31.h, z0.h, z17.h
[^:]*:[0-9]+: Info:    	sclamp z31.s, z0.s, z17.s
[^:]*:[0-9]+: Error: operand mismatch -- `uclamp z8.b,z1.b,z31.q'
[^:]*:[0-9]+: Info:    did you mean this\?
[^:]*:[0-9]+: Info:    	uclamp z8.b, z1.b, z31.b
[^:]*:[0-9]+: Info:    other valid variant\(s\):
[^:]*:[0-9]+: Info:    	uclamp z8.h, z1.h, z31.h
[^:]*:[0-9]+: Info:    	uclamp z8.s, z1.s, z31.s
[^:]*:[0-9]+: Info:    	uclamp z8.d, z1.d, z31.d
[^:]*:[0-9]+: Error: operand mismatch -- `uclamp z31.h,z0.h,z17.q'
[^:]*:[0-9]+: Info:    did you mean this\?
[^:]*:[0-9]+: Info:    	uclamp z31.h, z0.h, z17.h
[^:]*:[0-9]+: Info:    other valid variant\(s\):
[^:]*:[0-9]+: Info:    	uclamp z31.b, z0.b, z17.b
[^:]*:[0-9]+: Info:    	uclamp z31.s, z0.s, z17.s
[^:]*:[0-9]+: Info:    	uclamp z31.d, z0.d, z17.d
[^:]*:[0-9]+: Error: operand mismatch -- `uclamp z0.s,z31.s,z17.q'
[^:]*:[0-9]+: Info:    did you mean this\?
[^:]*:[0-9]+: Info:    	uclamp z0.s, z31.s, z17.s
[^:]*:[0-9]+: Info:    other valid variant\(s\):
[^:]*:[0-9]+: Info:    	uclamp z0.b, z31.b, z17.b
[^:]*:[0-9]+: Info:    	uclamp z0.h, z31.h, z17.h
[^:]*:[0-9]+: Info:    	uclamp z0.d, z31.d, z17.d
[^:]*:[0-9]+: Error: operand mismatch -- `uclamp z31.d,z0.d,z17.q'
[^:]*:[0-9]+: Info:    did you mean this\?
[^:]*:[0-9]+: Info:    	uclamp z31.d, z0.d, z17.d
[^:]*:[0-9]+: Info:    other valid variant\(s\):
[^:]*:[0-9]+: Info:    	uclamp z31.b, z0.b, z17.b
[^:]*:[0-9]+: Info:    	uclamp z31.h, z0.h, z17.h
[^:]*:[0-9]+: Info:    	uclamp z31.s, z0.s, z17.s
[^:]*:[0-9]+: Error: operand mismatch -- `uclamp z31.q,z0.d,z17.q'
[^:]*:[0-9]+: Info:    did you mean this\?
[^:]*:[0-9]+: Info:    	uclamp z31.d, z0.d, z17.d
[^:]*:[0-9]+: Info:    other valid variant\(s\):
[^:]*:[0-9]+: Info:    	uclamp z31.b, z0.b, z17.b
[^:]*:[0-9]+: Info:    	uclamp z31.h, z0.h, z17.h
[^:]*:[0-9]+: Info:    	uclamp z31.s, z0.s, z17.s
