[^:]*: Assembler messages:
.*: Error: expected an SVE predicate register at operand 2 -- `fmov z1,z2'
.*: Error: operand mismatch -- `fmov z1,#1\.0'
.*: Info:    did you mean this\?
.*: Info:    	fmov z1\.h, #1\.000000000000000000e\+00
.*: Info:    other valid variant\(s\):
.*: Info:    	fmov z1\.s, #1\.000000000000000000e\+00
.*: Info:    	fmov z1\.d, #1\.000000000000000000e\+00
.*: Error: operand mismatch -- `fmov z1,#0\.0'
.*: Info:    did you mean this\?
.*: Info:    	fmov z1\.h, #0\.0
.*: Info:    other valid variant\(s\):
.*: Info:    	fmov z1\.s, #0\.0
.*: Info:    	fmov z1\.d, #0\.0
.*: Error: missing predication type at operand 2 -- `not z0\.s,p1/'
.*: Error: missing predication type at operand 2 -- `not z0\.s,p1/,z2\.s'
.*: Error: unexpected character `c' in predication type at operand 2 -- `not z0\.s,p1/c,z2\.s'
.*: Error: operand mismatch -- `movprfx z0\.h,z1\.h'
.*: Info:    did you mean this\?
.*: Info:    	movprfx z0, z1
.*: Error: operand mismatch -- `movprfx z0,z1\.h'
.*: Info:    did you mean this\?
.*: Info:    	movprfx z0, z1
.*: Error: operand mismatch -- `movprfx z0\.h,z1'
.*: Info:    did you mean this\?
.*: Info:    	movprfx z0, z1
.*: Error: operand mismatch -- `movprfx z0\.h,z1\.s'
.*: Info:    did you mean this\?
.*: Info:    	movprfx z0, z1
.*: Error: operand mismatch -- `movprfx z0,p1/m,z1'
.*: Info:    did you mean this\?
.*: Info:    	movprfx z0\.b, p1/m, z1\.b
.*: Info:    other valid variant\(s\):
.*: Info:    	movprfx z0\.b, p1/z, z1\.b
.*: Info:    	movprfx z0\.h, p1/z, z1\.h
.*: Info:    	movprfx z0\.h, p1/m, z1\.h
.*: Info:    	movprfx z0\.s, p1/z, z1\.s
.*: Info:    	movprfx z0\.s, p1/m, z1\.s
.*: Info:    	movprfx z0\.d, p1/z, z1\.d
.*: Info:    	movprfx z0\.d, p1/m, z1\.d
.*: Error: operand mismatch -- `movprfx z0,p1/z,z1'
.*: Info:    did you mean this\?
.*: Info:    	movprfx z0\.b, p1/z, z1\.b
.*: Info:    other valid variant\(s\):
.*: Info:    	movprfx z0\.b, p1/m, z1\.b
.*: Info:    	movprfx z0\.h, p1/z, z1\.h
.*: Info:    	movprfx z0\.h, p1/m, z1\.h
.*: Info:    	movprfx z0\.s, p1/z, z1\.s
.*: Info:    	movprfx z0\.s, p1/m, z1\.s
.*: Info:    	movprfx z0\.d, p1/z, z1\.d
.*: Info:    	movprfx z0\.d, p1/m, z1\.d
.*: Error: operand mismatch -- `movprfx z0\.b,p1/m,z1'
.*: Info:    did you mean this\?
.*: Info:    	movprfx z0\.b, p1/m, z1\.b
.*: Info:    other valid variant\(s\):
.*: Info:    	movprfx z0\.b, p1/z, z1\.b
.*: Info:    	movprfx z0\.h, p1/z, z1\.h
.*: Info:    	movprfx z0\.h, p1/m, z1\.h
.*: Info:    	movprfx z0\.s, p1/z, z1\.s
.*: Info:    	movprfx z0\.s, p1/m, z1\.s
.*: Info:    	movprfx z0\.d, p1/z, z1\.d
.*: Info:    	movprfx z0\.d, p1/m, z1\.d
.*: Error: operand mismatch -- `movprfx z0\.b,p1/z,z1'
.*: Info:    did you mean this\?
.*: Info:    	movprfx z0\.b, p1/z, z1\.b
.*: Info:    other valid variant\(s\):
.*: Info:    	movprfx z0\.b, p1/m, z1\.b
.*: Info:    	movprfx z0\.h, p1/z, z1\.h
.*: Info:    	movprfx z0\.h, p1/m, z1\.h
.*: Info:    	movprfx z0\.s, p1/z, z1\.s
.*: Info:    	movprfx z0\.s, p1/m, z1\.s
.*: Info:    	movprfx z0\.d, p1/z, z1\.d
.*: Info:    	movprfx z0\.d, p1/m, z1\.d
.*: Error: operand mismatch -- `movprfx z0,p1/m,z1\.b'
.*: Info:    did you mean this\?
.*: Info:    	movprfx z0\.b, p1/m, z1\.b
.*: Info:    other valid variant\(s\):
.*: Info:    	movprfx z0\.b, p1/z, z1\.b
.*: Info:    	movprfx z0\.h, p1/z, z1\.h
.*: Info:    	movprfx z0\.h, p1/m, z1\.h
.*: Info:    	movprfx z0\.s, p1/z, z1\.s
.*: Info:    	movprfx z0\.s, p1/m, z1\.s
.*: Info:    	movprfx z0\.d, p1/z, z1\.d
.*: Info:    	movprfx z0\.d, p1/m, z1\.d
.*: Error: operand mismatch -- `movprfx z0,p1/z,z1\.b'
.*: Info:    did you mean this\?
.*: Info:    	movprfx z0\.b, p1/z, z1\.b
.*: Info:    other valid variant\(s\):
.*: Info:    	movprfx z0\.b, p1/m, z1\.b
.*: Info:    	movprfx z0\.h, p1/z, z1\.h
.*: Info:    	movprfx z0\.h, p1/m, z1\.h
.*: Info:    	movprfx z0\.s, p1/z, z1\.s
.*: Info:    	movprfx z0\.s, p1/m, z1\.s
.*: Info:    	movprfx z0\.d, p1/z, z1\.d
.*: Info:    	movprfx z0\.d, p1/m, z1\.d
.*: Error: operand mismatch -- `movprfx z0\.h,p1/m,z1\.b'
.*: Info:    did you mean this\?
.*: Info:    	movprfx z0\.b, p1/m, z1\.b
.*: Info:    other valid variant\(s\):
.*: Info:    	movprfx z0\.b, p1/z, z1\.b
.*: Info:    	movprfx z0\.h, p1/z, z1\.h
.*: Info:    	movprfx z0\.h, p1/m, z1\.h
.*: Info:    	movprfx z0\.s, p1/z, z1\.s
.*: Info:    	movprfx z0\.s, p1/m, z1\.s
.*: Info:    	movprfx z0\.d, p1/z, z1\.d
.*: Info:    	movprfx z0\.d, p1/m, z1\.d
.*: Error: operand mismatch -- `movprfx z0\.h,p1/z,z1\.b'
.*: Info:    did you mean this\?
.*: Info:    	movprfx z0\.b, p1/z, z1\.b
.*: Info:    other valid variant\(s\):
.*: Info:    	movprfx z0\.b, p1/m, z1\.b
.*: Info:    	movprfx z0\.h, p1/z, z1\.h
.*: Info:    	movprfx z0\.h, p1/m, z1\.h
.*: Info:    	movprfx z0\.s, p1/z, z1\.s
.*: Info:    	movprfx z0\.s, p1/m, z1\.s
.*: Info:    	movprfx z0\.d, p1/z, z1\.d
.*: Info:    	movprfx z0\.d, p1/m, z1\.d
.*: Error: operand mismatch -- `movprfx z0\.b,p1,z1\.b'
.*: Info:    did you mean this\?
.*: Info:    	movprfx z0\.b, p1/z, z1\.b
.*: Info:    other valid variant\(s\):
.*: Info:    	movprfx z0\.b, p1/m, z1\.b
.*: Info:    	movprfx z0\.h, p1/z, z1\.h
.*: Info:    	movprfx z0\.h, p1/m, z1\.h
.*: Info:    	movprfx z0\.s, p1/z, z1\.s
.*: Info:    	movprfx z0\.s, p1/m, z1\.s
.*: Info:    	movprfx z0\.d, p1/z, z1\.d
.*: Info:    	movprfx z0\.d, p1/m, z1\.d
.*: Error: expected an SVE vector register at operand 1 -- `movprfx p0,p1'
.*: Error: operand mismatch -- `ldr p0\.b,\[x1\]'
.*: Info:    did you mean this\?
.*: Info:    	ldr p0, \[x1\]
.*: Error: operand mismatch -- `ldr z0\.b,\[x1\]'
.*: Info:    did you mean this\?
.*: Info:    	ldr z0, \[x1\]
.*: Error: operand mismatch -- `str p0\.b,\[x1\]'
.*: Info:    did you mean this\?
.*: Info:    	str p0, \[x1\]
.*: Error: operand mismatch -- `str z0\.b,\[x1\]'
.*: Info:    did you mean this\?
.*: Info:    	str z0, \[x1\]
.*: Error: operand mismatch -- `mov z0,b0'
.*: Info:    did you mean this\?
.*: Info:    	mov z0\.b, b0
.*: Info:    other valid variant\(s\):
.*: Info:    	mov z0\.h, h0
.*: Info:    	mov z0\.s, s0
.*: Info:    	mov z0\.d, d0
.*: Info:    	mov z0\.q, q0
.*: Error: operand mismatch -- `mov z0,z1'
.*: Info:    did you mean this\?
.*: Info:    	mov z0\.d, z1\.d
.*: Error: operand mismatch -- `mov p0,p1'
.*: Info:    did you mean this\?
.*: Info:    	mov p0\.b, p1\.b
.*: Error: operand mismatch -- `add z0,z0,z2'
.*: Info:    did you mean this\?
.*: Info:    	add z0\.b, z0\.b, z2\.b
.*: Info:    other valid variant\(s\):
.*: Info:    	add z0\.h, z0\.h, z2\.h
.*: Info:    	add z0\.s, z0\.s, z2\.s
.*: Info:    	add z0\.d, z0\.d, z2\.d
.*: Error: operand mismatch -- `add z0,z0,#2'
.*: Info:    did you mean this\?
.*: Info:    	add z0\.b, z0\.b, #2
.*: Info:    other valid variant\(s\):
.*: Info:    	add z0\.h, z0\.h, #2
.*: Info:    	add z0\.s, z0\.s, #2
.*: Info:    	add z0\.d, z0\.d, #2
.*: Error: operand mismatch -- `add z0,z1,z2'
.*: Info:    did you mean this\?
.*: Info:    	add z0\.b, z1\.b, z2\.b
.*: Info:    other valid variant\(s\):
.*: Info:    	add z0\.h, z1\.h, z2\.h
.*: Info:    	add z0\.s, z1\.s, z2\.s
.*: Info:    	add z0\.d, z1\.d, z2\.d
.*: Error: operand 2 must be the same register as operand 1 -- `add z0,z1,#1'
.*: Error: operand 2 must be the same register as operand 1 -- `add z0\.b,z1\.b,#1'
.*: Error: operand mismatch -- `add z0\.b,z0\.h,#1'
.*: Info:    did you mean this\?
.*: Info:    	add z0\.b, z0\.b, #1
.*: Info:    other valid variant\(s\):
.*: Info:    	add z0\.h, z0\.h, #1
.*: Info:    	add z0\.s, z0\.s, #1
.*: Info:    	add z0\.d, z0\.d, #1
.*: Error: constant expression required at operand 2 -- `mov z0\.b,z32\.b'
.*: Error: expected an SVE predicate register at operand 2 -- `mov p0\.b,p16\.b'
.*: Error: p0-p7 expected at operand 2 -- `cmpeq p0\.b,p8/z,z1\.b,z2\.b'
.*: Error: p0-p7 expected at operand 2 -- `cmpeq p0\.b,p15/z,z1\.b,z2\.b'
.*: Error: operand mismatch -- `ld1w z0\.s,p0,\[x0\]'
.*: Info:    did you mean this\?
.*: Info:    	ld1w \{z0\.s\}, p0/z, \[x0\]
.*: Error: operand mismatch -- `ld1w z0\.s,p0/m,\[x0\]'
.*: Info:    did you mean this\?
.*: Info:    	ld1w \{z0\.s\}, p0/z, \[x0\]
.*: Error: operand mismatch -- `cmpeq p0\.b,p0,z1\.b,z2\.b'
.*: Info:    did you mean this\?
.*: Info:    	cmpeq p0\.b, p0/z, z1\.b, z2\.b
.*: Info:    other valid variant\(s\):
.*: Info:    	cmpeq p0\.h, p0/z, z1\.h, z2\.h
.*: Info:    	cmpeq p0\.s, p0/z, z1\.s, z2\.s
.*: Info:    	cmpeq p0\.d, p0/z, z1\.d, z2\.d
.*: Error: operand mismatch -- `cmpeq p0\.b,p0/m,z1\.b,z2\.b'
.*: Info:    did you mean this\?
.*: Info:    	cmpeq p0\.b, p0/z, z1\.b, z2\.b
.*: Info:    other valid variant\(s\):
.*: Info:    	cmpeq p0\.h, p0/z, z1\.h, z2\.h
.*: Info:    	cmpeq p0\.s, p0/z, z1\.s, z2\.s
.*: Info:    	cmpeq p0\.d, p0/z, z1\.d, z2\.d
.*: Error: operand mismatch -- `add z0\.s,p0,z0\.s,z1\.s'
.*: Info:    did you mean this\?
.*: Info:    	add z0\.s, p0/m, z0\.s, z1\.s
.*: Info:    other valid variant\(s\):
.*: Info:    	add z0\.b, p0/m, z0\.b, z1\.b
.*: Info:    	add z0\.h, p0/m, z0\.h, z1\.h
.*: Info:    	add z0\.d, p0/m, z0\.d, z1\.d
.*: Error: operand mismatch -- `add z0\.s,p0/z,z0\.s,z1\.s'
.*: Info:    did you mean this\?
.*: Info:    	add z0\.s, p0/m, z0\.s, z1\.s
.*: Info:    other valid variant\(s\):
.*: Info:    	add z0\.b, p0/m, z0\.b, z1\.b
.*: Info:    	add z0\.h, p0/m, z0\.h, z1\.h
.*: Info:    	add z0\.d, p0/m, z0\.d, z1\.d
.*: Error: operand mismatch -- `st1w z0\.s,p0/z,\[x0\]'
.*: Info:    did you mean this\?
.*: Info:    	st1w \{z0\.s\}, p0, \[x0\]
.*: Error: operand mismatch -- `st1w z0\.s,p0/m,\[x0\]'
.*: Info:    did you mean this\?
.*: Info:    	st1w \{z0\.s\}, p0, \[x0\]
.*: Error: missing type suffix at operand 1 -- `ld1b z0,p1/z,\[x1\]'
.*: Error: missing type suffix at operand 1 -- `ld1h z0,p1/z,\[x1\]'
.*: Error: missing type suffix at operand 1 -- `ld1w z0,p1/z,\[x1\]'
.*: Error: missing type suffix at operand 1 -- `ld1d z0,p1/z,\[x1\]'
.*: Error: missing type suffix at operand 1 -- `ldff1b z0,p1/z,\[x1,xzr\]'
.*: Error: missing type suffix at operand 1 -- `ldff1h z0,p1/z,\[x1,xzr,lsl#1\]'
.*: Error: missing type suffix at operand 1 -- `ldff1w z0,p1/z,\[x1,xzr,lsl#2\]'
.*: Error: missing type suffix at operand 1 -- `ldff1d z0,p1/z,\[x1,xzr,lsl#3\]'
.*: Error: missing type suffix at operand 1 -- `ldnf1b z0,p1/z,\[x1\]'
.*: Error: missing type suffix at operand 1 -- `ldnf1h z0,p1/z,\[x1\]'
.*: Error: missing type suffix at operand 1 -- `ldnf1w z0,p1/z,\[x1\]'
.*: Error: missing type suffix at operand 1 -- `ldnf1d z0,p1/z,\[x1\]'
.*: Error: missing type suffix at operand 1 -- `ldnt1b z0,p1/z,\[x1\]'
.*: Error: missing type suffix at operand 1 -- `ldnt1h z0,p1/z,\[x1\]'
.*: Error: missing type suffix at operand 1 -- `ldnt1w z0,p1/z,\[x1\]'
.*: Error: missing type suffix at operand 1 -- `ldnt1d z0,p1/z,\[x1\]'
.*: Error: missing type suffix at operand 1 -- `st1b z0,p1/z,\[x1\]'
.*: Error: missing type suffix at operand 1 -- `st1h z0,p1/z,\[x1\]'
.*: Error: missing type suffix at operand 1 -- `st1w z0,p1/z,\[x1\]'
.*: Error: missing type suffix at operand 1 -- `st1d z0,p1/z,\[x1\]'
.*: Error: missing type suffix at operand 1 -- `stnt1b z0,p1/z,\[x1\]'
.*: Error: missing type suffix at operand 1 -- `stnt1h z0,p1/z,\[x1\]'
.*: Error: missing type suffix at operand 1 -- `stnt1w z0,p1/z,\[x1\]'
.*: Error: missing type suffix at operand 1 -- `stnt1d z0,p1/z,\[x1\]'
.*: Error: missing type suffix at operand 1 -- `ld1b {z0},p1/z,\[x1\]'
.*: Error: missing type suffix at operand 1 -- `ld1h {z0},p1/z,\[x1\]'
.*: Error: missing type suffix at operand 1 -- `ld1w {z0},p1/z,\[x1\]'
.*: Error: missing type suffix at operand 1 -- `ld1d {z0},p1/z,\[x1\]'
.*: Error: missing type suffix at operand 1 -- `ldff1b {z0},p1/z,\[x1,xzr\]'
.*: Error: missing type suffix at operand 1 -- `ldff1h {z0},p1/z,\[x1,xzr,lsl#1\]'
.*: Error: missing type suffix at operand 1 -- `ldff1w {z0},p1/z,\[x1,xzr,lsl#2\]'
.*: Error: missing type suffix at operand 1 -- `ldff1d {z0},p1/z,\[x1,xzr,lsl#3\]'
.*: Error: missing type suffix at operand 1 -- `ldnf1b {z0},p1/z,\[x1\]'
.*: Error: missing type suffix at operand 1 -- `ldnf1h {z0},p1/z,\[x1\]'
.*: Error: missing type suffix at operand 1 -- `ldnf1w {z0},p1/z,\[x1\]'
.*: Error: missing type suffix at operand 1 -- `ldnf1d {z0},p1/z,\[x1\]'
.*: Error: missing type suffix at operand 1 -- `ldnt1b {z0},p1/z,\[x1\]'
.*: Error: missing type suffix at operand 1 -- `ldnt1h {z0},p1/z,\[x1\]'
.*: Error: missing type suffix at operand 1 -- `ldnt1w {z0},p1/z,\[x1\]'
.*: Error: missing type suffix at operand 1 -- `ldnt1d {z0},p1/z,\[x1\]'
.*: Error: missing type suffix at operand 1 -- `st1b {z0},p1/z,\[x1\]'
.*: Error: missing type suffix at operand 1 -- `st1h {z0},p1/z,\[x1\]'
.*: Error: missing type suffix at operand 1 -- `st1w {z0},p1/z,\[x1\]'
.*: Error: missing type suffix at operand 1 -- `st1d {z0},p1/z,\[x1\]'
.*: Error: missing type suffix at operand 1 -- `stnt1b {z0},p1/z,\[x1\]'
.*: Error: missing type suffix at operand 1 -- `stnt1h {z0},p1/z,\[x1\]'
.*: Error: missing type suffix at operand 1 -- `stnt1w {z0},p1/z,\[x1\]'
.*: Error: missing type suffix at operand 1 -- `stnt1d {z0},p1/z,\[x1\]'
.*: Error: expected an SVE vector register or ZA tile slice at operand 1 -- `ld1b {x0},p1/z,\[x1\]'
.*: Error: expected an SVE vector register or ZA tile slice at operand 1 -- `ld1b {b0},p1/z,\[x1\]'
.*: Error: expected an SVE vector register or ZA tile slice at operand 1 -- `ld1b {h0},p1/z,\[x1\]'
.*: Error: expected an SVE vector register or ZA tile slice at operand 1 -- `ld1b {s0},p1/z,\[x1\]'
.*: Error: expected an SVE vector register or ZA tile slice at operand 1 -- `ld1b {d0},p1/z,\[x1\]'
.*: Error: expected an SVE vector register or ZA tile slice at operand 1 -- `ld1b {v0\.2s},p1/z,\[x1\]'
.*: Error: type mismatch in vector register list at operand 1 -- `ld2b {z0\.b,z1},p1/z,\[x1\]'
.*: Error: type mismatch in vector register list at operand 1 -- `ld2b {z0\.b,z1\.h},p1/z,\[x1\]'
.*: Error: type mismatch in vector register list at operand 1 -- `ld2b {z0\.b,z1\.s},p1/z,\[x1\]'
.*: Error: type mismatch in vector register list at operand 1 -- `ld2b {z0\.b,z1\.d},p1/z,\[x1\]'
.*: Error: type mismatch in vector register list at operand 1 -- `ld2b {z0\.h,z1},p1/z,\[x1\]'
.*: Error: type mismatch in vector register list at operand 1 -- `ld2b {z0\.h,z1\.s},p1/z,\[x1\]'
.*: Error: type mismatch in vector register list at operand 1 -- `ld2b {z0\.h,z1\.d},p1/z,\[x1\]'
.*: Error: type mismatch in vector register list at operand 1 -- `ld2b {z0\.s,z1},p1/z,\[x1\]'
.*: Error: type mismatch in vector register list at operand 1 -- `ld2b {z0\.s,z1\.d},p1/z,\[x1\]'
.*: Error: type mismatch in vector register list at operand 1 -- `ld2b {z0\.d,z1},p1/z,\[x1\]'
.*: Error: immediate offset out of range -8 to 7 at operand 3 -- `ld1b z0\.b,p1/z,\[x1,#-9,mul vl\]'
.*: Error: only 'MUL VL' is permitted at operand 3 -- `ld1b z0\.b,p1/z,\[x1,#0,mul#1\]'
.*: Error: '\]' expected at operand 3 -- `ld1b z0\.b,p1/z,\[x1,#0,mul vl#1\]'
.*: Error: constant offset required at operand 3 -- `ld1b z0\.b,p1/z,\[x1,#foo,mul vl\]'
.*: Error: invalid addressing mode at operand 3 -- `ld1b z0\.b,p1/z,\[x1,#1\]'
.*: Error: invalid addressing mode at operand 3 -- `ld1b z0\.b,p1/z,\[x1,#7,mul vl\]!'
.*: Error: immediate offset out of range -8 to 7 at operand 3 -- `ld1b z0\.b,p1/z,\[x1,#8,mul vl\]'
.*: Error: immediate offset out of range -16 to 14 at operand 3 -- `ld2b \{z0\.b,z1\.b\},p1/z,\[x1,#-18,mul vl\]'
.*: Error: immediate offset out of range -16 to 14 at operand 3 -- `ld2b \{z0\.b,z1\.b\},p1/z,\[x1,#-17,mul vl\]'
.*: Error: constant offset required at operand 3 -- `ld2b \{z0\.b,z1\.b\},p1/z,\[x1,#foo,mul vl\]'
.*: Error: immediate value must be a multiple of 2 at operand 3 -- `ld2b \{z0\.b,z1\.b\},p1/z,\[x1,#1,mul vl\]'
.*: Error: invalid addressing mode at operand 3 -- `ld2b \{z0\.b,z1\.b\},p1/z,\[x1,#14,mul vl\]!'
.*: Error: immediate offset out of range -16 to 14 at operand 3 -- `ld2b \{z0\.b,z1\.b\},p1/z,\[x1,#16,mul vl\]'
.*: Error: immediate offset out of range -24 to 21 at operand 3 -- `ld3b \{z0\.b-z2\.b\},p1/z,\[x1,#-27,mul vl\]'
.*: Error: immediate offset out of range -24 to 21 at operand 3 -- `ld3b \{z0\.b-z2\.b\},p1/z,\[x1,#-26,mul vl\]'
.*: Error: immediate offset out of range -24 to 21 at operand 3 -- `ld3b \{z0\.b-z2\.b\},p1/z,\[x1,#-25,mul vl\]'
.*: Error: constant offset required at operand 3 -- `ld3b \{z0\.b-z2\.b\},p1/z,\[x1,#foo,mul vl\]'
.*: Error: immediate value must be a multiple of 3 at operand 3 -- `ld3b \{z0\.b-z2\.b\},p1/z,\[x1,#1,mul vl\]'
.*: Error: immediate value must be a multiple of 3 at operand 3 -- `ld3b \{z0\.b-z2\.b\},p1/z,\[x1,#2,mul vl\]'
.*: Error: invalid addressing mode at operand 3 -- `ld3b \{z0\.b-z2\.b\},p1/z,\[x1,#21,mul vl\]!'
.*: Error: immediate offset out of range -24 to 21 at operand 3 -- `ld3b \{z0\.b-z2\.b\},p1/z,\[x1,#24,mul vl\]'
.*: Error: immediate offset out of range -32 to 28 at operand 3 -- `ld4b \{z0\.b-z3\.b\},p1/z,\[x1,#-36,mul vl\]'
.*: Error: immediate offset out of range -32 to 28 at operand 3 -- `ld4b \{z0\.b-z3\.b\},p1/z,\[x1,#-35,mul vl\]'
.*: Error: immediate offset out of range -32 to 28 at operand 3 -- `ld4b \{z0\.b-z3\.b\},p1/z,\[x1,#-34,mul vl\]'
.*: Error: immediate offset out of range -32 to 28 at operand 3 -- `ld4b \{z0\.b-z3\.b\},p1/z,\[x1,#-33,mul vl\]'
.*: Error: constant offset required at operand 3 -- `ld4b \{z0\.b-z3\.b\},p1/z,\[x1,#foo,mul vl\]'
.*: Error: immediate value must be a multiple of 4 at operand 3 -- `ld4b \{z0\.b-z3\.b\},p1/z,\[x1,#1,mul vl\]'
.*: Error: immediate value must be a multiple of 4 at operand 3 -- `ld4b \{z0\.b-z3\.b\},p1/z,\[x1,#2,mul vl\]'
.*: Error: immediate value must be a multiple of 4 at operand 3 -- `ld4b \{z0\.b-z3\.b\},p1/z,\[x1,#3,mul vl\]'
.*: Error: invalid addressing mode at operand 3 -- `ld4b \{z0\.b-z3\.b\},p1/z,\[x1,#28,mul vl\]!'
.*: Error: immediate offset out of range -32 to 28 at operand 3 -- `ld4b \{z0\.b-z3\.b\},p1/z,\[x1,#32,mul vl\]'
.*: Error: immediate offset out of range -32 to 31 at operand 3 -- `prfb pldl1keep,p1,\[x1,#-33,mul vl\]'
.*: Error: constant offset required at operand 3 -- `prfb pldl1keep,p1,\[x1,#foo,mul vl\]'
.*: Error: invalid addressing mode at operand 3 -- `prfb pldl1keep,p1,\[x1,#1\]'
.*: Error: invalid addressing mode at operand 3 -- `prfb pldl1keep,p1,\[x1,#31,mul vl\]!'
.*: Error: immediate offset out of range -32 to 31 at operand 3 -- `prfb pldl1keep,p1,\[x1,#32,mul vl\]'
.*: Error: immediate offset out of range -256 to 255 at operand 2 -- `ldr z0,\[x1,#-257,mul vl\]'
.*: Error: constant offset required at operand 2 -- `ldr z0,\[x1,#foo,mul vl\]'
.*: Error: invalid addressing mode at operand 2 -- `ldr z0,\[x1,#1\]'
.*: Error: invalid addressing mode at operand 2 -- `ldr z0,\[x1,#255,mul vl\]!'
.*: Error: immediate offset out of range -256 to 255 at operand 2 -- `ldr z0,\[x1,#256,mul vl\]'
.*: Error: immediate offset out of range 0 to 63 at operand 3 -- `ld1rb z0\.b,p1/z,\[x1,#-1\]'
.*: Error: constant offset required at operand 3 -- `ld1rb z0\.b,p1/z,\[x1,#foo\]'
.*: Error: invalid addressing mode at operand 3 -- `ld1rb z0\.b,p1/z,\[x1,#1,mul vl\]'
.*: Error: invalid addressing mode at operand 3 -- `ld1rb z0\.b,p1/z,\[x1,#63\]!'
.*: Error: invalid addressing mode at operand 3 -- `ld1rb z0\.b,p1/z,\[x1\],#63'
.*: Error: immediate offset out of range 0 to 63 at operand 3 -- `ld1rb z0\.b,p1/z,\[x1,#64\]'
.*: Error: immediate offset out of range 0 to 126 at operand 3 -- `ld1rh z0\.h,p1/z,\[x1,#-2\]'
.*: Error: immediate offset out of range 0 to 126 at operand 3 -- `ld1rh z0\.h,p1/z,\[x1,#-1\]'
.*: Error: constant offset required at operand 3 -- `ld1rh z0\.h,p1/z,\[x1,#foo\]'
.*: Error: immediate value must be a multiple of 2 at operand 3 -- `ld1rh z0\.h,p1/z,\[x1,#1\]'
.*: Error: invalid addressing mode at operand 3 -- `ld1rh z0\.h,p1/z,\[x1,#2,mul vl\]'
.*: Error: invalid addressing mode at operand 3 -- `ld1rh z0\.h,p1/z,\[x1,#126\]!'
.*: Error: invalid addressing mode at operand 3 -- `ld1rh z0\.h,p1/z,\[x1\],#126'
.*: Error: immediate offset out of range 0 to 126 at operand 3 -- `ld1rh z0\.h,p1/z,\[x1,#128\]'
.*: Error: immediate offset out of range 0 to 252 at operand 3 -- `ld1rw z0\.s,p1/z,\[x1,#-4\]'
.*: Error: immediate offset out of range 0 to 252 at operand 3 -- `ld1rw z0\.s,p1/z,\[x1,#-1\]'
.*: Error: constant offset required at operand 3 -- `ld1rw z0\.s,p1/z,\[x1,#foo\]'
.*: Error: immediate value must be a multiple of 4 at operand 3 -- `ld1rw z0\.s,p1/z,\[x1,#1\]'
.*: Error: immediate value must be a multiple of 4 at operand 3 -- `ld1rw z0\.s,p1/z,\[x1,#2\]'
.*: Error: invalid addressing mode at operand 3 -- `ld1rw z0\.s,p1/z,\[x1,#4,mul vl\]'
.*: Error: invalid addressing mode at operand 3 -- `ld1rw z0\.s,p1/z,\[x1,#252\]!'
.*: Error: invalid addressing mode at operand 3 -- `ld1rw z0\.s,p1/z,\[x1\],#252'
.*: Error: immediate offset out of range 0 to 252 at operand 3 -- `ld1rw z0\.s,p1/z,\[x1,#256\]'
.*: Error: immediate offset out of range 0 to 504 at operand 3 -- `ld1rd z0\.d,p1/z,\[x1,#-8\]'
.*: Error: immediate offset out of range 0 to 504 at operand 3 -- `ld1rd z0\.d,p1/z,\[x1,#-1\]'
.*: Error: constant offset required at operand 3 -- `ld1rd z0\.d,p1/z,\[x1,#foo\]'
.*: Error: immediate value must be a multiple of 8 at operand 3 -- `ld1rd z0\.d,p1/z,\[x1,#1\]'
.*: Error: immediate value must be a multiple of 8 at operand 3 -- `ld1rd z0\.d,p1/z,\[x1,#2\]'
.*: Error: immediate value must be a multiple of 8 at operand 3 -- `ld1rd z0\.d,p1/z,\[x1,#4\]'
.*: Error: invalid addressing mode at operand 3 -- `ld1rd z0\.d,p1/z,\[x1,#8,mul vl\]'
.*: Error: invalid addressing mode at operand 3 -- `ld1rd z0\.d,p1/z,\[x1,#504\]!'
.*: Error: invalid addressing mode at operand 3 -- `ld1rd z0\.d,p1/z,\[x1\],#504'
.*: Error: immediate offset out of range 0 to 504 at operand 3 -- `ld1rd z0\.d,p1/z,\[x1,#512\]'
.*: Error: register offset not allowed in pre-indexed addressing mode at operand 3 -- `ld1b z0\.b,p1/z,\[x1,x2\]!'
.*: Error: invalid addressing mode at operand 3 -- `ld1b z0\.b,p1/z,\[x1\],x2'
.*: Error: invalid addressing mode at operand 3 -- `ld1b z0\.b,p1/z,\[x1,x2,lsl#1\]'
.*: Error: invalid addressing mode at operand 3 -- `ld1b z0\.b,p1/z,\[x1,x2,lsl#2\]'
.*: Error: invalid addressing mode at operand 3 -- `ld1b z0\.b,p1/z,\[x1,x2,lsl#3\]'
.*: Error: constant shift amount required at operand 3 -- `ld1b z0\.b,p1/z,\[x1,x2,lsl x3\]'
.*: Error: invalid addressing mode at operand 3 -- `ld1b z0\.b,p1/z,\[x1,w2,sxtw\]'
.*: Error: invalid addressing mode at operand 3 -- `ld1b z0\.b,p1/z,\[x1,w2,uxtw\]'
.*: Error: invalid addressing mode at operand 3 -- `ld1h z0\.h,p1/z,\[x1,x2\]'
.*: Error: register offset not allowed in pre-indexed addressing mode at operand 3 -- `ld1h z0\.h,p1/z,\[x1,x2,lsl#1\]!'
.*: Error: invalid addressing mode at operand 3 -- `ld1h z0\.h,p1/z,\[x1,x2,lsl#2\]'
.*: Error: invalid addressing mode at operand 3 -- `ld1h z0\.h,p1/z,\[x1,x2,lsl#3\]'
.*: Error: constant shift amount required at operand 3 -- `ld1h z0\.h,p1/z,\[x1,x2,lsl x3\]'
.*: Error: invalid addressing mode at operand 3 -- `ld1h z0\.h,p1/z,\[x1,w2,sxtw\]'
.*: Error: invalid addressing mode at operand 3 -- `ld1h z0\.h,p1/z,\[x1,w2,uxtw\]'
.*: Error: invalid addressing mode at operand 3 -- `ld1w z0\.s,p1/z,\[x1,x2\]'
.*: Error: invalid addressing mode at operand 3 -- `ld1w z0\.s,p1/z,\[x1,x2,lsl#1\]'
.*: Error: register offset not allowed in pre-indexed addressing mode at operand 3 -- `ld1w z0\.s,p1/z,\[x1,x2,lsl#2\]!'
.*: Error: invalid addressing mode at operand 3 -- `ld1w z0\.s,p1/z,\[x1,x2,lsl#3\]'
.*: Error: constant shift amount required at operand 3 -- `ld1w z0\.s,p1/z,\[x1,x2,lsl x3\]'
.*: Error: invalid addressing mode at operand 3 -- `ld1w z0\.s,p1/z,\[x1,w2,sxtw\]'
.*: Error: invalid addressing mode at operand 3 -- `ld1w z0\.s,p1/z,\[x1,w2,uxtw\]'
.*: Error: invalid addressing mode at operand 3 -- `ld1d z0\.d,p1/z,\[x1,x2\]'
.*: Error: invalid addressing mode at operand 3 -- `ld1d z0\.d,p1/z,\[x1,x2,lsl#1\]'
.*: Error: invalid addressing mode at operand 3 -- `ld1d z0\.d,p1/z,\[x1,x2,lsl#2\]'
.*: Error: register offset not allowed in pre-indexed addressing mode at operand 3 -- `ld1d z0\.d,p1/z,\[x1,x2,lsl#3\]!'
.*: Error: constant shift amount required at operand 3 -- `ld1d z0\.d,p1/z,\[x1,x2,lsl x3\]'
.*: Error: invalid addressing mode at operand 3 -- `ld1d z0\.d,p1/z,\[x1,w2,sxtw\]'
.*: Error: invalid addressing mode at operand 3 -- `ld1d z0\.d,p1/z,\[x1,w2,uxtw\]'
.*: Error: invalid addressing mode at operand 3 -- `ld1b z0\.d,p1/z,\[x1,z2\.d,lsl#1\]'
.*: Error: invalid addressing mode at operand 3 -- `ld1b z0\.d,p1/z,\[x1,z2\.d,lsl#2\]'
.*: Error: invalid addressing mode at operand 3 -- `ld1b z0\.d,p1/z,\[x1,z2\.d,lsl#3\]'
.*: Error: constant shift amount required at operand 3 -- `ld1b z0\.d,p1/z,\[x1,z2\.d,lsl x3\]'
.*: Error: invalid addressing mode at operand 3 -- `ld1h z0\.d,p1/z,\[x1,z2\.d,lsl#2\]'
.*: Error: invalid addressing mode at operand 3 -- `ld1h z0\.d,p1/z,\[x1,z2\.d,lsl#3\]'
.*: Error: constant shift amount required at operand 3 -- `ld1h z0\.d,p1/z,\[x1,z2\.d,lsl x3\]'
.*: Error: invalid addressing mode at operand 3 -- `ld1w z0\.d,p1/z,\[x1,z2\.d,lsl#1\]'
.*: Error: invalid addressing mode at operand 3 -- `ld1w z0\.d,p1/z,\[x1,z2\.d,lsl#3\]'
.*: Error: constant shift amount required at operand 3 -- `ld1w z0\.d,p1/z,\[x1,z2\.d,lsl x3\]'
.*: Error: invalid addressing mode at operand 3 -- `ld1d z0\.d,p1/z,\[x1,z2\.d,lsl#1\]'
.*: Error: invalid addressing mode at operand 3 -- `ld1d z0\.d,p1/z,\[x1,z2\.d,lsl#2\]'
.*: Error: constant shift amount required at operand 3 -- `ld1d z0\.d,p1/z,\[x1,z2\.d,lsl x3\]'
.*: Error: invalid addressing mode at operand 3 -- `ld1b z0\.s,p1/z,\[x1,z2\.s,sxtw#1\]'
.*: Error: invalid addressing mode at operand 3 -- `ld1b z0\.s,p1/z,\[x1,z2\.s,sxtw#2\]'
.*: Error: invalid addressing mode at operand 3 -- `ld1b z0\.s,p1/z,\[x1,z2\.s,sxtw#3\]'
.*: Error: constant shift amount required at operand 3 -- `ld1b z0\.s,p1/z,\[x1,z2\.s,sxtw x3\]'
.*: Error: invalid addressing mode at operand 3 -- `ld1h z0\.s,p1/z,\[x1,z2\.s,sxtw#2\]'
.*: Error: invalid addressing mode at operand 3 -- `ld1h z0\.s,p1/z,\[x1,z2\.s,sxtw#3\]'
.*: Error: constant shift amount required at operand 3 -- `ld1h z0\.s,p1/z,\[x1,z2\.s,sxtw x3\]'
.*: Error: invalid addressing mode at operand 3 -- `ld1w z0\.s,p1/z,\[x1,z2\.s,sxtw#1\]'
.*: Error: invalid addressing mode at operand 3 -- `ld1w z0\.s,p1/z,\[x1,z2\.s,sxtw#3\]'
.*: Error: constant shift amount required at operand 3 -- `ld1w z0\.s,p1/z,\[x1,z2\.s,sxtw x3\]'
.*: Error: invalid addressing mode at operand 3 -- `ld1b z0\.s,p1/z,\[x1,z2\.s,uxtw#1\]'
.*: Error: invalid addressing mode at operand 3 -- `ld1b z0\.s,p1/z,\[x1,z2\.s,uxtw#2\]'
.*: Error: invalid addressing mode at operand 3 -- `ld1b z0\.s,p1/z,\[x1,z2\.s,uxtw#3\]'
.*: Error: constant shift amount required at operand 3 -- `ld1b z0\.s,p1/z,\[x1,z2\.s,uxtw x3\]'
.*: Error: invalid addressing mode at operand 3 -- `ld1h z0\.s,p1/z,\[x1,z2\.s,uxtw#2\]'
.*: Error: invalid addressing mode at operand 3 -- `ld1h z0\.s,p1/z,\[x1,z2\.s,uxtw#3\]'
.*: Error: constant shift amount required at operand 3 -- `ld1h z0\.s,p1/z,\[x1,z2\.s,uxtw x3\]'
.*: Error: invalid addressing mode at operand 3 -- `ld1w z0\.s,p1/z,\[x1,z2\.s,uxtw#1\]'
.*: Error: invalid addressing mode at operand 3 -- `ld1w z0\.s,p1/z,\[x1,z2\.s,uxtw#3\]'
.*: Error: constant shift amount required at operand 3 -- `ld1w z0\.s,p1/z,\[x1,z2\.s,uxtw x3\]'
.*: Error: invalid addressing mode at operand 3 -- `ld1b z0\.d,p1/z,\[x1,z2\.d,sxtw#1\]'
.*: Error: invalid addressing mode at operand 3 -- `ld1b z0\.d,p1/z,\[x1,z2\.d,sxtw#2\]'
.*: Error: invalid addressing mode at operand 3 -- `ld1b z0\.d,p1/z,\[x1,z2\.d,sxtw#3\]'
.*: Error: constant shift amount required at operand 3 -- `ld1b z0\.d,p1/z,\[x1,z2\.d,sxtw x3\]'
.*: Error: invalid addressing mode at operand 3 -- `ld1h z0\.d,p1/z,\[x1,z2\.d,sxtw#2\]'
.*: Error: invalid addressing mode at operand 3 -- `ld1h z0\.d,p1/z,\[x1,z2\.d,sxtw#3\]'
.*: Error: constant shift amount required at operand 3 -- `ld1h z0\.d,p1/z,\[x1,z2\.d,sxtw x3\]'
.*: Error: invalid addressing mode at operand 3 -- `ld1w z0\.d,p1/z,\[x1,z2\.d,sxtw#1\]'
.*: Error: invalid addressing mode at operand 3 -- `ld1w z0\.d,p1/z,\[x1,z2\.d,sxtw#3\]'
.*: Error: constant shift amount required at operand 3 -- `ld1w z0\.d,p1/z,\[x1,z2\.d,sxtw x3\]'
.*: Error: invalid addressing mode at operand 3 -- `ld1d z0\.d,p1/z,\[x1,z2\.d,sxtw#1\]'
.*: Error: invalid addressing mode at operand 3 -- `ld1d z0\.d,p1/z,\[x1,z2\.d,sxtw#2\]'
.*: Error: constant shift amount required at operand 3 -- `ld1d z0\.d,p1/z,\[x1,z2\.d,sxtw x3\]'
.*: Error: invalid addressing mode at operand 3 -- `ld1b z0\.d,p1/z,\[x1,z2\.d,uxtw#1\]'
.*: Error: invalid addressing mode at operand 3 -- `ld1b z0\.d,p1/z,\[x1,z2\.d,uxtw#2\]'
.*: Error: invalid addressing mode at operand 3 -- `ld1b z0\.d,p1/z,\[x1,z2\.d,uxtw#3\]'
.*: Error: constant shift amount required at operand 3 -- `ld1b z0\.d,p1/z,\[x1,z2\.d,uxtw x3\]'
.*: Error: invalid addressing mode at operand 3 -- `ld1h z0\.d,p1/z,\[x1,z2\.d,uxtw#2\]'
.*: Error: invalid addressing mode at operand 3 -- `ld1h z0\.d,p1/z,\[x1,z2\.d,uxtw#3\]'
.*: Error: constant shift amount required at operand 3 -- `ld1h z0\.d,p1/z,\[x1,z2\.d,uxtw x3\]'
.*: Error: invalid addressing mode at operand 3 -- `ld1w z0\.d,p1/z,\[x1,z2\.d,uxtw#1\]'
.*: Error: invalid addressing mode at operand 3 -- `ld1w z0\.d,p1/z,\[x1,z2\.d,uxtw#3\]'
.*: Error: constant shift amount required at operand 3 -- `ld1w z0\.d,p1/z,\[x1,z2\.d,uxtw x3\]'
.*: Error: invalid addressing mode at operand 3 -- `ld1d z0\.d,p1/z,\[x1,z2\.d,uxtw#1\]'
.*: Error: invalid addressing mode at operand 3 -- `ld1d z0\.d,p1/z,\[x1,z2\.d,uxtw#2\]'
.*: Error: constant shift amount required at operand 3 -- `ld1d z0\.d,p1/z,\[x1,z2\.d,uxtw x3\]'
.*: Error: immediate offset out of range 0 to 31 at operand 3 -- `ld1b z0\.d,p1/z,\[z2\.d,#-1\]'
.*: Error: constant offset required at operand 3 -- `ld1b z0\.d,p1/z,\[z2\.d,#foo\]'
.*: Error: invalid addressing mode at operand 3 -- `ld1b z0\.d,p1/z,\[z2\.d,#1,mul vl\]'
.*: Error: immediate offset out of range 0 to 31 at operand 3 -- `ld1b z0\.d,p1/z,\[z2\.d,#32\]'
.*: Error: immediate offset out of range 0 to 62 at operand 3 -- `ld1h z0\.d,p1/z,\[z2\.d,#-2\]'
.*: Error: immediate offset out of range 0 to 62 at operand 3 -- `ld1h z0\.d,p1/z,\[z2\.d,#-1\]'
.*: Error: constant offset required at operand 3 -- `ld1h z0\.d,p1/z,\[z2\.d,#foo\]'
.*: Error: immediate value must be a multiple of 2 at operand 3 -- `ld1h z0\.d,p1/z,\[z2\.d,#1\]'
.*: Error: invalid addressing mode at operand 3 -- `ld1h z0\.d,p1/z,\[z2\.d,#2,mul vl\]'
.*: Error: immediate offset out of range 0 to 62 at operand 3 -- `ld1h z0\.d,p1/z,\[z2\.d,#64\]'
.*: Error: immediate offset out of range 0 to 124 at operand 3 -- `ld1w z0\.d,p1/z,\[z2\.d,#-4\]'
.*: Error: immediate offset out of range 0 to 124 at operand 3 -- `ld1w z0\.d,p1/z,\[z2\.d,#-1\]'
.*: Error: constant offset required at operand 3 -- `ld1w z0\.d,p1/z,\[z2\.d,#foo\]'
.*: Error: immediate value must be a multiple of 4 at operand 3 -- `ld1w z0\.d,p1/z,\[z2\.d,#1\]'
.*: Error: immediate value must be a multiple of 4 at operand 3 -- `ld1w z0\.d,p1/z,\[z2\.d,#2\]'
.*: Error: invalid addressing mode at operand 3 -- `ld1w z0\.d,p1/z,\[z2\.d,#4,mul vl\]'
.*: Error: immediate offset out of range 0 to 124 at operand 3 -- `ld1w z0\.d,p1/z,\[z2\.d,#128\]'
.*: Error: immediate offset out of range 0 to 248 at operand 3 -- `ld1d z0\.d,p1/z,\[z2\.d,#-8\]'
.*: Error: immediate offset out of range 0 to 248 at operand 3 -- `ld1d z0\.d,p1/z,\[z2\.d,#-1\]'
.*: Error: constant offset required at operand 3 -- `ld1d z0\.d,p1/z,\[z2\.d,#foo\]'
.*: Error: immediate value must be a multiple of 8 at operand 3 -- `ld1d z0\.d,p1/z,\[z2\.d,#1\]'
.*: Error: immediate value must be a multiple of 8 at operand 3 -- `ld1d z0\.d,p1/z,\[z2\.d,#2\]'
.*: Error: immediate value must be a multiple of 8 at operand 3 -- `ld1d z0\.d,p1/z,\[z2\.d,#4\]'
.*: Error: invalid addressing mode at operand 3 -- `ld1d z0\.d,p1/z,\[z2\.d,#8,mul vl\]'
.*: Error: immediate offset out of range 0 to 248 at operand 3 -- `ld1d z0\.d,p1/z,\[z2\.d,#256\]'
.*: Error: shift amount out of range 0 to 63 at operand 2 -- `adr z0\.s,\[z1\.s,z2\.s,lsl#-1\]'
.*: Error: invalid addressing mode at operand 2 -- `adr z0\.s,\[z1\.s,z2\.s,lsl#4\]'
.*: Error: constant shift amount required at operand 2 -- `adr z0\.s,\[z1\.s,z2\.s,lsl x3\]'
.*: Error: offset has different size from base at operand 2 -- `adr z0\.s,\[z1\.s,z2\.d\]'
.*: Error: offset has different size from base at operand 2 -- `adr z0\.s,\[z1\.s,x2\]'
.*: Error: offset has different size from base at operand 2 -- `adr z0\.s,\[z1\.d,z2\.s\]'
.*: Error: invalid use of 32-bit register offset at operand 2 -- `adr z0\.s,\[z1\.d,w2\]'
.*: Error: offset has different size from base at operand 2 -- `adr z0\.s,\[x1,z2\.s\]'
.*: Error: invalid addressing mode at operand 2 -- `adr z0\.s,\[x1,z2\.d\]'
.*: Error: invalid addressing mode at operand 2 -- `adr z0\.s,\[z1\.d,x2\]'
.*: Error: invalid addressing mode at operand 2 -- `adr z0\.s,\[x1,x2\]'
.*: Error: shift amount out of range 0 to 63 at operand 2 -- `adr z0\.d,\[z1\.d,z2\.d,lsl#-1\]'
.*: Error: invalid addressing mode at operand 2 -- `adr z0\.d,\[z1\.d,z2\.d,lsl#4\]'
.*: Error: constant shift amount required at operand 2 -- `adr z0\.d,\[z1\.d,z2\.d,lsl x3\]'
.*: Error: invalid addressing mode at operand 2 -- `adr z0\.s,\[z1\.s,z2\.s,sxtw\]'
.*: Error: shift amount out of range 0 to 63 at operand 2 -- `adr z0\.d,\[z1\.d,z2\.d,sxtw#-1\]'
.*: Error: invalid addressing mode at operand 2 -- `adr z0\.d,\[z1\.d,z2\.d,sxtw#4\]'
.*: Error: constant shift amount required at operand 2 -- `adr z0\.d,\[z1\.d,z2\.d,sxtw x3\]'
.*: Error: invalid addressing mode at operand 2 -- `adr z0\.s,\[z1\.s,z2\.s,uxtw\]'
.*: Error: shift amount out of range 0 to 63 at operand 2 -- `adr z0\.d,\[z1\.d,z2\.d,uxtw#-1\]'
.*: Error: invalid addressing mode at operand 2 -- `adr z0\.d,\[z1\.d,z2\.d,uxtw#4\]'
.*: Error: constant shift amount required at operand 2 -- `adr z0\.d,\[z1\.d,z2\.d,uxtw x3\]'
.*: Error: index register xzr is not allowed at operand 3 -- `ld1b z0\.b,p0/z,\[x1,xzr\]'
.*: Error: index register xzr is not allowed at operand 3 -- `ld1b z0\.h,p0/z,\[x1,xzr\]'
.*: Error: index register xzr is not allowed at operand 3 -- `ld1b z0\.s,p0/z,\[x1,xzr\]'
.*: Error: index register xzr is not allowed at operand 3 -- `ld1b z0\.d,p0/z,\[x1,xzr\]'
.*: Error: index register xzr is not allowed at operand 3 -- `ld1sb z0\.h,p0/z,\[x1,xzr\]'
.*: Error: index register xzr is not allowed at operand 3 -- `ld1sb z0\.s,p0/z,\[x1,xzr\]'
.*: Error: index register xzr is not allowed at operand 3 -- `ld1sb z0\.d,p0/z,\[x1,xzr\]'
.*: Error: index register xzr is not allowed at operand 3 -- `ld1h z0\.h,p0/z,\[x1,xzr,lsl#1\]'
.*: Error: index register xzr is not allowed at operand 3 -- `ld1h z0\.s,p0/z,\[x1,xzr,lsl#1\]'
.*: Error: index register xzr is not allowed at operand 3 -- `ld1h z0\.d,p0/z,\[x1,xzr,lsl#1\]'
.*: Error: index register xzr is not allowed at operand 3 -- `ld1sh z0\.s,p0/z,\[x1,xzr,lsl#1\]'
.*: Error: index register xzr is not allowed at operand 3 -- `ld1sh z0\.d,p0/z,\[x1,xzr,lsl#1\]'
.*: Error: index register xzr is not allowed at operand 3 -- `ld1w z0\.s,p0/z,\[x1,xzr,lsl#2\]'
.*: Error: index register xzr is not allowed at operand 3 -- `ld1w z0\.d,p0/z,\[x1,xzr,lsl#2\]'
.*: Error: index register xzr is not allowed at operand 3 -- `ld1sw z0\.d,p0/z,\[x1,xzr,lsl#2\]'
.*: Error: index register xzr is not allowed at operand 3 -- `ld1d z0\.d,p0/z,\[x1,xzr,lsl#3\]'
.*: Error: index register xzr is not allowed at operand 3 -- `ld2b {z0\.b-z1\.b},p0/z,\[x1,xzr\]'
.*: Error: index register xzr is not allowed at operand 3 -- `ld2h {z0\.h-z1\.h},p0/z,\[x1,xzr,lsl#1\]'
.*: Error: index register xzr is not allowed at operand 3 -- `ld2w {z0\.s-z1\.s},p0/z,\[x1,xzr,lsl#2\]'
.*: Error: index register xzr is not allowed at operand 3 -- `ld2d {z0\.d-z1\.d},p0/z,\[x1,xzr,lsl#3\]'
.*: Error: index register xzr is not allowed at operand 3 -- `ld3b {z0\.b-z2\.b},p0/z,\[x1,xzr\]'
.*: Error: index register xzr is not allowed at operand 3 -- `ld3h {z0\.h-z2\.h},p0/z,\[x1,xzr,lsl#1\]'
.*: Error: index register xzr is not allowed at operand 3 -- `ld3w {z0\.s-z2\.s},p0/z,\[x1,xzr,lsl#2\]'
.*: Error: index register xzr is not allowed at operand 3 -- `ld3d {z0\.d-z2\.d},p0/z,\[x1,xzr,lsl#3\]'
.*: Error: index register xzr is not allowed at operand 3 -- `ld4b {z0\.b-z3\.b},p0/z,\[x1,xzr\]'
.*: Error: index register xzr is not allowed at operand 3 -- `ld4h {z0\.h-z3\.h},p0/z,\[x1,xzr,lsl#1\]'
.*: Error: index register xzr is not allowed at operand 3 -- `ld4w {z0\.s-z3\.s},p0/z,\[x1,xzr,lsl#2\]'
.*: Error: index register xzr is not allowed at operand 3 -- `ld4d {z0\.d-z3\.d},p0/z,\[x1,xzr,lsl#3\]'
.*: Error: index register xzr is not allowed at operand 3 -- `ldnt1b z0\.b,p0/z,\[x1,xzr\]'
.*: Error: index register xzr is not allowed at operand 3 -- `ldnt1h z0\.h,p0/z,\[x1,xzr,lsl#1\]'
.*: Error: index register xzr is not allowed at operand 3 -- `ldnt1w z0\.s,p0/z,\[x1,xzr,lsl#2\]'
.*: Error: index register xzr is not allowed at operand 3 -- `ldnt1d z0\.d,p0/z,\[x1,xzr,lsl#3\]'
.*: Error: index register xzr is not allowed at operand 3 -- `st1b z0\.b,p0,\[x1,xzr\]'
.*: Error: index register xzr is not allowed at operand 3 -- `st1b z0\.h,p0,\[x1,xzr\]'
.*: Error: index register xzr is not allowed at operand 3 -- `st1b z0\.s,p0,\[x1,xzr\]'
.*: Error: index register xzr is not allowed at operand 3 -- `st1b z0\.d,p0,\[x1,xzr\]'
.*: Error: index register xzr is not allowed at operand 3 -- `st1h z0\.h,p0,\[x1,xzr,lsl#1\]'
.*: Error: index register xzr is not allowed at operand 3 -- `st1h z0\.s,p0,\[x1,xzr,lsl#1\]'
.*: Error: index register xzr is not allowed at operand 3 -- `st1h z0\.d,p0,\[x1,xzr,lsl#1\]'
.*: Error: index register xzr is not allowed at operand 3 -- `st1w z0\.s,p0,\[x1,xzr,lsl#2\]'
.*: Error: index register xzr is not allowed at operand 3 -- `st1w z0\.d,p0,\[x1,xzr,lsl#2\]'
.*: Error: index register xzr is not allowed at operand 3 -- `st1d z0\.d,p0,\[x1,xzr,lsl#3\]'
.*: Error: index register xzr is not allowed at operand 3 -- `st2b {z0\.b-z1\.b},p0,\[x1,xzr\]'
.*: Error: index register xzr is not allowed at operand 3 -- `st2h {z0\.h-z1\.h},p0,\[x1,xzr,lsl#1\]'
.*: Error: index register xzr is not allowed at operand 3 -- `st2w {z0\.s-z1\.s},p0,\[x1,xzr,lsl#2\]'
.*: Error: index register xzr is not allowed at operand 3 -- `st2d {z0\.d-z1\.d},p0,\[x1,xzr,lsl#3\]'
.*: Error: index register xzr is not allowed at operand 3 -- `st3b {z0\.b-z2\.b},p0,\[x1,xzr\]'
.*: Error: index register xzr is not allowed at operand 3 -- `st3h {z0\.h-z2\.h},p0,\[x1,xzr,lsl#1\]'
.*: Error: index register xzr is not allowed at operand 3 -- `st3w {z0\.s-z2\.s},p0,\[x1,xzr,lsl#2\]'
.*: Error: index register xzr is not allowed at operand 3 -- `st3d {z0\.d-z2\.d},p0,\[x1,xzr,lsl#3\]'
.*: Error: index register xzr is not allowed at operand 3 -- `st4b {z0\.b-z3\.b},p0,\[x1,xzr\]'
.*: Error: index register xzr is not allowed at operand 3 -- `st4h {z0\.h-z3\.h},p0,\[x1,xzr,lsl#1\]'
.*: Error: index register xzr is not allowed at operand 3 -- `st4w {z0\.s-z3\.s},p0,\[x1,xzr,lsl#2\]'
.*: Error: index register xzr is not allowed at operand 3 -- `st4d {z0\.d-z3\.d},p0,\[x1,xzr,lsl#3\]'
.*: Error: index register xzr is not allowed at operand 3 -- `stnt1b z0\.b,p0,\[x1,xzr\]'
.*: Error: index register xzr is not allowed at operand 3 -- `stnt1h z0\.h,p0,\[x1,xzr,lsl#1\]'
.*: Error: index register xzr is not allowed at operand 3 -- `stnt1w z0\.s,p0,\[x1,xzr,lsl#2\]'
.*: Error: index register xzr is not allowed at operand 3 -- `stnt1d z0\.d,p0,\[x1,xzr,lsl#3\]'
.*: Error: index register xzr is not allowed at operand 3 -- `prfb pldl1keep,p0,\[x1,xzr\]'
.*: Error: index register xzr is not allowed at operand 3 -- `prfh pldl1keep,p0,\[x1,xzr,lsl#1\]'
.*: Error: index register xzr is not allowed at operand 3 -- `prfw pldl1keep,p0,\[x1,xzr,lsl#2\]'
.*: Error: index register xzr is not allowed at operand 3 -- `prfd pldl1keep,p0,\[x1,xzr,lsl#3\]'
.*: Error: immediate too big for element size at operand 3 -- `add z0\.b,z0\.b,#-257'
.*: Error: immediate too big for element size at operand 3 -- `add z0\.b,z0\.b,#256'
.*: Error: no shift amount allowed for 8-bit constants at operand 3 -- `add z0\.b,z0\.b,#1,lsl#1'
.*: Error: no shift amount allowed for 8-bit constants at operand 3 -- `add z0\.b,z0\.b,#0,lsl#8'
.*: Error: no shift amount allowed for 8-bit constants at operand 3 -- `add z0\.b,z0\.b,#1,lsl#8'
.*: Error: immediate too big for element size at operand 3 -- `add z0\.h,z0\.h,#-65537'
.*: Error: invalid arithmetic immediate at operand 3 -- `add z0\.h,z0\.h,#-65536\+257'
.*: Error: invalid arithmetic immediate at operand 3 -- `add z0\.h,z0\.h,#-32767'
.*: Error: invalid arithmetic immediate at operand 3 -- `add z0\.h,z0\.h,#-32768\+255'
.*: Error: invalid arithmetic immediate at operand 3 -- `add z0\.h,z0\.h,#-257'
.*: Error: invalid arithmetic immediate at operand 3 -- `add z0\.h,z0\.h,#-255'
.*: Error: invalid arithmetic immediate at operand 3 -- `add z0\.h,z0\.h,#-129'
.*: Error: invalid arithmetic immediate at operand 3 -- `add z0\.h,z0\.h,#-128'
.*: Error: invalid arithmetic immediate at operand 3 -- `add z0\.h,z0\.h,#-127'
.*: Error: invalid arithmetic immediate at operand 3 -- `add z0\.h,z0\.h,#-1'
.*: Error: invalid arithmetic immediate at operand 3 -- `add z0\.h,z0\.h,#257'
.*: Error: invalid arithmetic immediate at operand 3 -- `add z0\.h,z0\.h,#32768-255'
.*: Error: invalid arithmetic immediate at operand 3 -- `add z0\.h,z0\.h,#32767'
.*: Error: invalid arithmetic immediate at operand 3 -- `add z0\.h,z0\.h,#65536-255'
.*: Error: invalid arithmetic immediate at operand 3 -- `add z0\.h,z0\.h,#65536-129'
.*: Error: invalid arithmetic immediate at operand 3 -- `add z0\.h,z0\.h,#65536-128'
.*: Error: invalid arithmetic immediate at operand 3 -- `add z0\.h,z0\.h,#65535'
.*: Error: immediate too big for element size at operand 3 -- `add z0\.h,z0\.h,#65536'
.*: Error: shift amount must be 0 or 8 at operand 3 -- `add z0\.h,z0\.h,#1,lsl#1'
.*: Error: immediate too big for element size at operand 3 -- `add z0\.h,z0\.h,#-257,lsl#8'
.*: Error: immediate too big for element size at operand 3 -- `add z0\.h,z0\.h,#256,lsl#8'
.*: Error: invalid arithmetic immediate at operand 3 -- `add z0\.s,z0\.s,#-256'
.*: Error: invalid arithmetic immediate at operand 3 -- `add z0\.s,z0\.s,#-255'
.*: Error: invalid arithmetic immediate at operand 3 -- `add z0\.s,z0\.s,#-129'
.*: Error: invalid arithmetic immediate at operand 3 -- `add z0\.s,z0\.s,#-128'
.*: Error: invalid arithmetic immediate at operand 3 -- `add z0\.s,z0\.s,#-1'
.*: Error: invalid arithmetic immediate at operand 3 -- `add z0\.s,z0\.s,#257'
.*: Error: invalid arithmetic immediate at operand 3 -- `add z0\.s,z0\.s,#32768-255'
.*: Error: invalid arithmetic immediate at operand 3 -- `add z0\.s,z0\.s,#32767'
.*: Error: invalid arithmetic immediate at operand 3 -- `add z0\.s,z0\.s,#65536'
.*: Error: immediate too big for element size at operand 3 -- `add z0\.s,z0\.s,#0x100000000'
.*: Error: shift amount must be 0 or 8 at operand 3 -- `add z0\.s,z0\.s,#1,lsl#1'
.*: Error: invalid arithmetic immediate at operand 3 -- `add z0\.s,z0\.s,#-1,lsl#8'
.*: Error: invalid arithmetic immediate at operand 3 -- `add z0\.s,z0\.s,#256,lsl#8'
.*: Error: invalid arithmetic immediate at operand 3 -- `add z0\.d,z0\.d,#-256'
.*: Error: invalid arithmetic immediate at operand 3 -- `add z0\.d,z0\.d,#-255'
.*: Error: invalid arithmetic immediate at operand 3 -- `add z0\.d,z0\.d,#-129'
.*: Error: invalid arithmetic immediate at operand 3 -- `add z0\.d,z0\.d,#-128'
.*: Error: invalid arithmetic immediate at operand 3 -- `add z0\.d,z0\.d,#-1'
.*: Error: invalid arithmetic immediate at operand 3 -- `add z0\.d,z0\.d,#257'
.*: Error: invalid arithmetic immediate at operand 3 -- `add z0\.d,z0\.d,#32768-255'
.*: Error: invalid arithmetic immediate at operand 3 -- `add z0\.d,z0\.d,#32767'
.*: Error: invalid arithmetic immediate at operand 3 -- `add z0\.d,z0\.d,#65536'
.*: Error: invalid arithmetic immediate at operand 3 -- `add z0\.d,z0\.d,#0x100000000'
.*: Error: shift amount must be 0 or 8 at operand 3 -- `add z0\.d,z0\.d,#1,lsl#1'
.*: Error: invalid arithmetic immediate at operand 3 -- `add z0\.d,z0\.d,#-1,lsl#8'
.*: Error: invalid arithmetic immediate at operand 3 -- `add z0\.d,z0\.d,#256,lsl#8'
.*: Error: immediate too big for element size at operand 2 -- `dup z0\.b,#-257'
.*: Error: immediate too big for element size at operand 2 -- `dup z0\.b,#256'
.*: Error: no shift amount allowed for 8-bit constants at operand 2 -- `dup z0\.b,#1,lsl#1'
.*: Error: no shift amount allowed for 8-bit constants at operand 2 -- `dup z0\.b,#0,lsl#8'
.*: Error: no shift amount allowed for 8-bit constants at operand 2 -- `dup z0\.b,#1,lsl#8'
.*: Error: immediate too big for element size at operand 2 -- `dup z0\.h,#-65537'
.*: Error: invalid arithmetic immediate at operand 2 -- `dup z0\.h,#-32767'
.*: Error: invalid arithmetic immediate at operand 2 -- `dup z0\.h,#-32768\+255'
.*: Error: invalid arithmetic immediate at operand 2 -- `dup z0\.h,#-257'
.*: Error: invalid arithmetic immediate at operand 2 -- `dup z0\.h,#-255'
.*: Error: invalid arithmetic immediate at operand 2 -- `dup z0\.h,#-129'
.*: Error: invalid arithmetic immediate at operand 2 -- `dup z0\.h,#128'
.*: Error: invalid arithmetic immediate at operand 2 -- `dup z0\.h,#255'
.*: Error: invalid arithmetic immediate at operand 2 -- `dup z0\.h,#257'
.*: Error: invalid arithmetic immediate at operand 2 -- `dup z0\.h,#32768-255'
.*: Error: invalid arithmetic immediate at operand 2 -- `dup z0\.h,#32767'
.*: Error: invalid arithmetic immediate at operand 2 -- `dup z0\.h,#65536-255'
.*: Error: invalid arithmetic immediate at operand 2 -- `dup z0\.h,#65536-129'
.*: Error: immediate too big for element size at operand 2 -- `dup z0\.h,#65536'
.*: Error: shift amount must be 0 or 8 at operand 2 -- `dup z0\.h,#1,lsl#1'
.*: Error: immediate too big for element size at operand 2 -- `dup z0\.h,#-257,lsl#8'
.*: Error: immediate too big for element size at operand 2 -- `dup z0\.h,#256,lsl#8'
.*: Error: invalid arithmetic immediate at operand 2 -- `dup z0\.s,#-65536'
.*: Error: invalid arithmetic immediate at operand 2 -- `dup z0\.s,#-32769'
.*: Error: invalid arithmetic immediate at operand 2 -- `dup z0\.s,#-32767'
.*: Error: invalid arithmetic immediate at operand 2 -- `dup z0\.s,#-32768\+255'
.*: Error: invalid arithmetic immediate at operand 2 -- `dup z0\.s,#-257'
.*: Error: invalid arithmetic immediate at operand 2 -- `dup z0\.s,#-255'
.*: Error: invalid arithmetic immediate at operand 2 -- `dup z0\.s,#-129'
.*: Error: invalid arithmetic immediate at operand 2 -- `dup z0\.s,#128'
.*: Error: invalid arithmetic immediate at operand 2 -- `dup z0\.s,#255'
.*: Error: invalid arithmetic immediate at operand 2 -- `dup z0\.s,#257'
.*: Error: invalid arithmetic immediate at operand 2 -- `dup z0\.s,#32768-255'
.*: Error: invalid arithmetic immediate at operand 2 -- `dup z0\.s,#32767'
.*: Error: invalid arithmetic immediate at operand 2 -- `dup z0\.s,#32768'
.*: Error: invalid arithmetic immediate at operand 2 -- `dup z0\.s,#65536'
.*: Error: invalid arithmetic immediate at operand 2 -- `dup z0\.s,#0xffffff7f'
.*: Error: immediate too big for element size at operand 2 -- `dup z0\.s,#0x100000000'
.*: Error: shift amount must be 0 or 8 at operand 2 -- `dup z0\.s,#1,lsl#1'
.*: Error: invalid arithmetic immediate at operand 2 -- `dup z0\.s,#-129,lsl#8'
.*: Error: invalid arithmetic immediate at operand 2 -- `dup z0\.s,#128,lsl#8'
.*: Error: invalid arithmetic immediate at operand 2 -- `dup z0\.d,#-65536'
.*: Error: invalid arithmetic immediate at operand 2 -- `dup z0\.d,#-32769'
.*: Error: invalid arithmetic immediate at operand 2 -- `dup z0\.d,#-32767'
.*: Error: invalid arithmetic immediate at operand 2 -- `dup z0\.d,#-32768\+255'
.*: Error: invalid arithmetic immediate at operand 2 -- `dup z0\.d,#-257'
.*: Error: invalid arithmetic immediate at operand 2 -- `dup z0\.d,#-255'
.*: Error: invalid arithmetic immediate at operand 2 -- `dup z0\.d,#-129'
.*: Error: invalid arithmetic immediate at operand 2 -- `dup z0\.d,#128'
.*: Error: invalid arithmetic immediate at operand 2 -- `dup z0\.d,#255'
.*: Error: invalid arithmetic immediate at operand 2 -- `dup z0\.d,#257'
.*: Error: invalid arithmetic immediate at operand 2 -- `dup z0\.d,#32768-255'
.*: Error: invalid arithmetic immediate at operand 2 -- `dup z0\.d,#32767'
.*: Error: invalid arithmetic immediate at operand 2 -- `dup z0\.d,#32768'
.*: Error: invalid arithmetic immediate at operand 2 -- `dup z0\.d,#65536'
.*: Error: invalid arithmetic immediate at operand 2 -- `dup z0\.d,#0xffffff7f'
.*: Error: invalid arithmetic immediate at operand 2 -- `dup z0\.d,#0x100000000'
.*: Error: shift amount must be 0 or 8 at operand 2 -- `dup z0\.d,#1,lsl#1'
.*: Error: invalid arithmetic immediate at operand 2 -- `dup z0\.d,#-129,lsl#8'
.*: Error: invalid arithmetic immediate at operand 2 -- `dup z0\.d,#128,lsl#8'
.*: Error: immediate out of range at operand 3 -- `and z0\.b,z0\.b,#0x0101'
.*: Error: immediate out of range at operand 3 -- `and z0\.b,z0\.b,#0x01010101'
.*: Error: immediate out of range at operand 3 -- `and z0\.b,z0\.b,#0x0101010101010101'
.*: Error: immediate out of range at operand 3 -- `and z0\.b,z0\.b,#0x7f7f'
.*: Error: immediate out of range at operand 3 -- `and z0\.b,z0\.b,#0x7f7f7f7f'
.*: Error: immediate out of range at operand 3 -- `and z0\.b,z0\.b,#0x7f7f7f7f7f7f7f7f'
.*: Error: immediate out of range at operand 3 -- `and z0\.b,z0\.b,#0x8080'
.*: Error: immediate out of range at operand 3 -- `and z0\.b,z0\.b,#0x80808080'
.*: Error: immediate out of range at operand 3 -- `and z0\.b,z0\.b,#0x8080808080808080'
.*: Error: immediate out of range at operand 3 -- `and z0\.b,z0\.b,#0xfefe'
.*: Error: immediate out of range at operand 3 -- `and z0\.b,z0\.b,#0xfefefefe'
.*: Error: immediate out of range at operand 3 -- `and z0\.b,z0\.b,#0xfefefefefefefefe'
.*: Error: immediate out of range at operand 3 -- `and z0\.b,z0\.b,#0x00010001'
.*: Error: immediate out of range at operand 3 -- `and z0\.b,z0\.b,#0x0001000100010001'
.*: Error: immediate out of range at operand 3 -- `and z0\.b,z0\.b,#0x7fff'
.*: Error: immediate out of range at operand 3 -- `and z0\.b,z0\.b,#0x7fff7fff'
.*: Error: immediate out of range at operand 3 -- `and z0\.b,z0\.b,#0x7fff7fff7fff7fff'
.*: Error: immediate out of range at operand 3 -- `and z0\.b,z0\.b,#0x8000'
.*: Error: immediate out of range at operand 3 -- `and z0\.b,z0\.b,#0x80008000'
.*: Error: immediate out of range at operand 3 -- `and z0\.b,z0\.b,#0x8000800080008000'
.*: Error: immediate out of range at operand 3 -- `and z0\.b,z0\.b,#0xfffe'
.*: Error: immediate out of range at operand 3 -- `and z0\.b,z0\.b,#0xfffefffe'
.*: Error: immediate out of range at operand 3 -- `and z0\.b,z0\.b,#0xfffefffefffefffe'
.*: Error: immediate out of range at operand 3 -- `and z0\.b,z0\.b,#0x0000000100000001'
.*: Error: immediate out of range at operand 3 -- `and z0\.b,z0\.b,#0x7fffffff'
.*: Error: immediate out of range at operand 3 -- `and z0\.b,z0\.b,#0x7fffffff7fffffff'
.*: Error: immediate out of range at operand 3 -- `and z0\.b,z0\.b,#0x80000000'
.*: Error: immediate out of range at operand 3 -- `and z0\.b,z0\.b,#0x8000000080000000'
.*: Error: immediate out of range at operand 3 -- `and z0\.b,z0\.b,#0xfffffffe'
.*: Error: immediate out of range at operand 3 -- `and z0\.b,z0\.b,#0xfffffffefffffffe'
.*: Error: immediate out of range at operand 3 -- `and z0\.b,z0\.b,#0x7fffffffffffffff'
.*: Error: immediate out of range at operand 3 -- `and z0\.b,z0\.b,#0x8000000000000000'
.*: Error: immediate out of range at operand 3 -- `and z0\.h,z0\.h,#0x01010101'
.*: Error: immediate out of range at operand 3 -- `and z0\.h,z0\.h,#0x0101010101010101'
.*: Error: immediate out of range at operand 3 -- `and z0\.h,z0\.h,#0x7f7f7f7f'
.*: Error: immediate out of range at operand 3 -- `and z0\.h,z0\.h,#0x7f7f7f7f7f7f7f7f'
.*: Error: immediate out of range at operand 3 -- `and z0\.h,z0\.h,#0x80808080'
.*: Error: immediate out of range at operand 3 -- `and z0\.h,z0\.h,#0x8080808080808080'
.*: Error: immediate out of range at operand 3 -- `and z0\.h,z0\.h,#0xfefefefe'
.*: Error: immediate out of range at operand 3 -- `and z0\.h,z0\.h,#0xfefefefefefefefe'
.*: Error: immediate out of range at operand 3 -- `and z0\.h,z0\.h,#0x00010001'
.*: Error: immediate out of range at operand 3 -- `and z0\.h,z0\.h,#0x0001000100010001'
.*: Error: immediate out of range at operand 3 -- `and z0\.h,z0\.h,#0x7fff7fff'
.*: Error: immediate out of range at operand 3 -- `and z0\.h,z0\.h,#0x7fff7fff7fff7fff'
.*: Error: immediate out of range at operand 3 -- `and z0\.h,z0\.h,#0x80008000'
.*: Error: immediate out of range at operand 3 -- `and z0\.h,z0\.h,#0x8000800080008000'
.*: Error: immediate out of range at operand 3 -- `and z0\.h,z0\.h,#0xfffefffe'
.*: Error: immediate out of range at operand 3 -- `and z0\.h,z0\.h,#0xfffefffefffefffe'
.*: Error: immediate out of range at operand 3 -- `and z0\.h,z0\.h,#0x0000000100000001'
.*: Error: immediate out of range at operand 3 -- `and z0\.h,z0\.h,#0x7fffffff'
.*: Error: immediate out of range at operand 3 -- `and z0\.h,z0\.h,#0x7fffffff7fffffff'
.*: Error: immediate out of range at operand 3 -- `and z0\.h,z0\.h,#0x80000000'
.*: Error: immediate out of range at operand 3 -- `and z0\.h,z0\.h,#0x8000000080000000'
.*: Error: immediate out of range at operand 3 -- `and z0\.h,z0\.h,#0xfffffffe'
.*: Error: immediate out of range at operand 3 -- `and z0\.h,z0\.h,#0xfffffffefffffffe'
.*: Error: immediate out of range at operand 3 -- `and z0\.h,z0\.h,#0x7fffffffffffffff'
.*: Error: immediate out of range at operand 3 -- `and z0\.h,z0\.h,#0x8000000000000000'
.*: Error: immediate out of range at operand 3 -- `and z0\.s,z0\.s,#0x0101010101010101'
.*: Error: immediate out of range at operand 3 -- `and z0\.s,z0\.s,#0x7f7f7f7f7f7f7f7f'
.*: Error: immediate out of range at operand 3 -- `and z0\.s,z0\.s,#0x8080808080808080'
.*: Error: immediate out of range at operand 3 -- `and z0\.s,z0\.s,#0xfefefefefefefefe'
.*: Error: immediate out of range at operand 3 -- `and z0\.s,z0\.s,#0x0001000100010001'
.*: Error: immediate out of range at operand 3 -- `and z0\.s,z0\.s,#0x7fff7fff7fff7fff'
.*: Error: immediate out of range at operand 3 -- `and z0\.s,z0\.s,#0x8000800080008000'
.*: Error: immediate out of range at operand 3 -- `and z0\.s,z0\.s,#0xfffefffefffefffe'
.*: Error: immediate out of range at operand 3 -- `and z0\.s,z0\.s,#0x0000000100000001'
.*: Error: immediate out of range at operand 3 -- `and z0\.s,z0\.s,#0x7fffffff7fffffff'
.*: Error: immediate out of range at operand 3 -- `and z0\.s,z0\.s,#0x8000000080000000'
.*: Error: immediate out of range at operand 3 -- `and z0\.s,z0\.s,#0xfffffffefffffffe'
.*: Error: immediate out of range at operand 3 -- `and z0\.s,z0\.s,#0x7fffffffffffffff'
.*: Error: immediate out of range at operand 3 -- `and z0\.s,z0\.s,#0x8000000000000000'
.*: Error: immediate out of range at operand 3 -- `and z0\.d,z0\.d,#0xd'
.*: Error: immediate out of range at operand 3 -- `bic z0\.b,z0\.b,#0x0101'
.*: Error: immediate out of range at operand 3 -- `bic z0\.b,z0\.b,#0x01010101'
.*: Error: immediate out of range at operand 3 -- `bic z0\.b,z0\.b,#0x0101010101010101'
.*: Error: immediate out of range at operand 3 -- `bic z0\.b,z0\.b,#0x7f7f'
.*: Error: immediate out of range at operand 3 -- `bic z0\.b,z0\.b,#0x7f7f7f7f'
.*: Error: immediate out of range at operand 3 -- `bic z0\.b,z0\.b,#0x7f7f7f7f7f7f7f7f'
.*: Error: immediate out of range at operand 3 -- `bic z0\.b,z0\.b,#0x8080'
.*: Error: immediate out of range at operand 3 -- `bic z0\.b,z0\.b,#0x80808080'
.*: Error: immediate out of range at operand 3 -- `bic z0\.b,z0\.b,#0x8080808080808080'
.*: Error: immediate out of range at operand 3 -- `bic z0\.b,z0\.b,#0xfefe'
.*: Error: immediate out of range at operand 3 -- `bic z0\.b,z0\.b,#0xfefefefe'
.*: Error: immediate out of range at operand 3 -- `bic z0\.b,z0\.b,#0xfefefefefefefefe'
.*: Error: immediate out of range at operand 3 -- `bic z0\.b,z0\.b,#0x00010001'
.*: Error: immediate out of range at operand 3 -- `bic z0\.b,z0\.b,#0x0001000100010001'
.*: Error: immediate out of range at operand 3 -- `bic z0\.b,z0\.b,#0x7fff'
.*: Error: immediate out of range at operand 3 -- `bic z0\.b,z0\.b,#0x7fff7fff'
.*: Error: immediate out of range at operand 3 -- `bic z0\.b,z0\.b,#0x7fff7fff7fff7fff'
.*: Error: immediate out of range at operand 3 -- `bic z0\.b,z0\.b,#0x8000'
.*: Error: immediate out of range at operand 3 -- `bic z0\.b,z0\.b,#0x80008000'
.*: Error: immediate out of range at operand 3 -- `bic z0\.b,z0\.b,#0x8000800080008000'
.*: Error: immediate out of range at operand 3 -- `bic z0\.b,z0\.b,#0xfffe'
.*: Error: immediate out of range at operand 3 -- `bic z0\.b,z0\.b,#0xfffefffe'
.*: Error: immediate out of range at operand 3 -- `bic z0\.b,z0\.b,#0xfffefffefffefffe'
.*: Error: immediate out of range at operand 3 -- `bic z0\.b,z0\.b,#0x0000000100000001'
.*: Error: immediate out of range at operand 3 -- `bic z0\.b,z0\.b,#0x7fffffff'
.*: Error: immediate out of range at operand 3 -- `bic z0\.b,z0\.b,#0x7fffffff7fffffff'
.*: Error: immediate out of range at operand 3 -- `bic z0\.b,z0\.b,#0x80000000'
.*: Error: immediate out of range at operand 3 -- `bic z0\.b,z0\.b,#0x8000000080000000'
.*: Error: immediate out of range at operand 3 -- `bic z0\.b,z0\.b,#0xfffffffe'
.*: Error: immediate out of range at operand 3 -- `bic z0\.b,z0\.b,#0xfffffffefffffffe'
.*: Error: immediate out of range at operand 3 -- `bic z0\.b,z0\.b,#0x7fffffffffffffff'
.*: Error: immediate out of range at operand 3 -- `bic z0\.b,z0\.b,#0x8000000000000000'
.*: Error: immediate out of range at operand 3 -- `bic z0\.h,z0\.h,#0x01010101'
.*: Error: immediate out of range at operand 3 -- `bic z0\.h,z0\.h,#0x0101010101010101'
.*: Error: immediate out of range at operand 3 -- `bic z0\.h,z0\.h,#0x7f7f7f7f'
.*: Error: immediate out of range at operand 3 -- `bic z0\.h,z0\.h,#0x7f7f7f7f7f7f7f7f'
.*: Error: immediate out of range at operand 3 -- `bic z0\.h,z0\.h,#0x80808080'
.*: Error: immediate out of range at operand 3 -- `bic z0\.h,z0\.h,#0x8080808080808080'
.*: Error: immediate out of range at operand 3 -- `bic z0\.h,z0\.h,#0xfefefefe'
.*: Error: immediate out of range at operand 3 -- `bic z0\.h,z0\.h,#0xfefefefefefefefe'
.*: Error: immediate out of range at operand 3 -- `bic z0\.h,z0\.h,#0x00010001'
.*: Error: immediate out of range at operand 3 -- `bic z0\.h,z0\.h,#0x0001000100010001'
.*: Error: immediate out of range at operand 3 -- `bic z0\.h,z0\.h,#0x7fff7fff'
.*: Error: immediate out of range at operand 3 -- `bic z0\.h,z0\.h,#0x7fff7fff7fff7fff'
.*: Error: immediate out of range at operand 3 -- `bic z0\.h,z0\.h,#0x80008000'
.*: Error: immediate out of range at operand 3 -- `bic z0\.h,z0\.h,#0x8000800080008000'
.*: Error: immediate out of range at operand 3 -- `bic z0\.h,z0\.h,#0xfffefffe'
.*: Error: immediate out of range at operand 3 -- `bic z0\.h,z0\.h,#0xfffefffefffefffe'
.*: Error: immediate out of range at operand 3 -- `bic z0\.h,z0\.h,#0x0000000100000001'
.*: Error: immediate out of range at operand 3 -- `bic z0\.h,z0\.h,#0x7fffffff'
.*: Error: immediate out of range at operand 3 -- `bic z0\.h,z0\.h,#0x7fffffff7fffffff'
.*: Error: immediate out of range at operand 3 -- `bic z0\.h,z0\.h,#0x80000000'
.*: Error: immediate out of range at operand 3 -- `bic z0\.h,z0\.h,#0x8000000080000000'
.*: Error: immediate out of range at operand 3 -- `bic z0\.h,z0\.h,#0xfffffffe'
.*: Error: immediate out of range at operand 3 -- `bic z0\.h,z0\.h,#0xfffffffefffffffe'
.*: Error: immediate out of range at operand 3 -- `bic z0\.h,z0\.h,#0x7fffffffffffffff'
.*: Error: immediate out of range at operand 3 -- `bic z0\.h,z0\.h,#0x8000000000000000'
.*: Error: immediate out of range at operand 3 -- `bic z0\.s,z0\.s,#0x0101010101010101'
.*: Error: immediate out of range at operand 3 -- `bic z0\.s,z0\.s,#0x7f7f7f7f7f7f7f7f'
.*: Error: immediate out of range at operand 3 -- `bic z0\.s,z0\.s,#0x8080808080808080'
.*: Error: immediate out of range at operand 3 -- `bic z0\.s,z0\.s,#0xfefefefefefefefe'
.*: Error: immediate out of range at operand 3 -- `bic z0\.s,z0\.s,#0x0001000100010001'
.*: Error: immediate out of range at operand 3 -- `bic z0\.s,z0\.s,#0x7fff7fff7fff7fff'
.*: Error: immediate out of range at operand 3 -- `bic z0\.s,z0\.s,#0x8000800080008000'
.*: Error: immediate out of range at operand 3 -- `bic z0\.s,z0\.s,#0xfffefffefffefffe'
.*: Error: immediate out of range at operand 3 -- `bic z0\.s,z0\.s,#0x0000000100000001'
.*: Error: immediate out of range at operand 3 -- `bic z0\.s,z0\.s,#0x7fffffff7fffffff'
.*: Error: immediate out of range at operand 3 -- `bic z0\.s,z0\.s,#0x8000000080000000'
.*: Error: immediate out of range at operand 3 -- `bic z0\.s,z0\.s,#0xfffffffefffffffe'
.*: Error: immediate out of range at operand 3 -- `bic z0\.s,z0\.s,#0x7fffffffffffffff'
.*: Error: immediate out of range at operand 3 -- `bic z0\.s,z0\.s,#0x8000000000000000'
.*: Error: immediate out of range at operand 3 -- `bic z0\.d,z0\.d,#0xd'
.*: Error: immediate zero expected at operand 4 -- `fcmeq p0\.s,p1/z,z2\.s,#1'
.*: Error: immediate zero expected at operand 4 -- `fcmeq p0\.s,p1/z,z2\.s,#1\.0'
.*: Error: floating-point value must be 0\.5 or 1\.0 at operand 4 -- `fadd z0\.s,p1/m,z0\.s,#0'
.*: Error: floating-point value must be 0\.5 or 1\.0 at operand 4 -- `fadd z0\.s,p1/m,z0\.s,#0\.0'
.*: Error: floating-point value must be 0\.5 or 1\.0 at operand 4 -- `fadd z0\.s,p1/m,z0\.s,#1\.5'
.*: Error: floating-point value must be 0\.5 or 1\.0 at operand 4 -- `fadd z0\.s,p1/m,z0\.s,#2'
.*: Error: floating-point value must be 0\.5 or 1\.0 at operand 4 -- `fadd z0\.s,p1/m,z0\.s,#2\.0'
.*: Error: floating-point value must be 0\.5 or 2\.0 at operand 4 -- `fmul z0\.s,p1/m,z0\.s,#0'
.*: Error: floating-point value must be 0\.5 or 2\.0 at operand 4 -- `fmul z0\.s,p1/m,z0\.s,#0\.0'
.*: Error: floating-point value must be 0\.5 or 2\.0 at operand 4 -- `fmul z0\.s,p1/m,z0\.s,#1'
.*: Error: floating-point value must be 0\.5 or 2\.0 at operand 4 -- `fmul z0\.s,p1/m,z0\.s,#1\.0'
.*: Error: floating-point value must be 0\.5 or 2\.0 at operand 4 -- `fmul z0\.s,p1/m,z0\.s,#1\.5'
.*: Error: floating-point value must be 0\.0 or 1\.0 at operand 4 -- `fmax z0\.s,p1/m,z0\.s,#0\.5'
.*: Error: floating-point value must be 0\.0 or 1\.0 at operand 4 -- `fmax z0\.s,p1/m,z0\.s,#1\.5'
.*: Error: floating-point value must be 0\.0 or 1\.0 at operand 4 -- `fmax z0\.s,p1/m,z0\.s,#2'
.*: Error: floating-point value must be 0\.0 or 1\.0 at operand 4 -- `fmax z0\.s,p1/m,z0\.s,#2\.0'
.*: Error: operand 2 must be an enumeration value such as POW2 -- `ptrue p1\.b,vl0'
.*: Error: operand 2 must be an enumeration value such as POW2 -- `ptrue p1\.b,vl255'
.*: Error: operand 2 must be an enumeration value such as POW2 -- `ptrue p1\.b,#-1'
.*: Error: operand 2 must be an enumeration value such as POW2 -- `ptrue p1\.b,#32'
.*: Error: immediate operand required at operand 2 -- `ptrue p1\.b,x0'
.*: Error: immediate operand required at operand 2 -- `ptrue p1\.b,z0\.s'
.*: Error: operand 2 must be an enumeration value such as POW2 -- `cntb x0,vl0'
.*: Error: operand 2 must be an enumeration value such as POW2 -- `cntb x0,vl255'
.*: Error: operand 2 must be an enumeration value such as POW2 -- `cntb x0,#-1'
.*: Error: operand 2 must be an enumeration value such as POW2 -- `cntb x0,#32'
.*: Error: immediate operand required at operand 2 -- `cntb x0,x0'
.*: Error: immediate operand required at operand 2 -- `cntb x0,z0\.s'
.*: Error: operand 2 must be an enumeration value such as POW2 -- `cntb x0,mul#1'
.*: Error: multiplier out of range 1 to 16 at operand 2 -- `cntb x0,pow2,mul#0'
.*: Error: multiplier out of range 1 to 16 at operand 2 -- `cntb x0,pow2,mul#17'
.*: Error: shift expression expected at operand 2 -- `cntb x0,pow2,#1'
.*: Error: operand 1 must be an enumeration value such as PLDL1KEEP -- `prfb pldl0keep,p1,\[x0\]'
.*: Error: operand 1 must be an enumeration value such as PLDL1KEEP -- `prfb pldl4keep,p1,\[x0\]'
.*: Error: operand 1 must be an enumeration value such as PLDL1KEEP -- `prfb #-1,p1,\[x0\]'
.*: Error: operand 1 must be an enumeration value such as PLDL1KEEP -- `prfb #16,p1,\[x0\]'
.*: Error: immediate operand required at operand 1 -- `prfb x0,p1,\[x0\]'
.*: Error: immediate operand required at operand 1 -- `prfb z0\.s,p1,\[x0\]'
.*: Error: immediate value out of range 0 to 7 at operand 3 -- `lsl z0\.b,z0\.b,#-1'
.*: Error: immediate value out of range 0 to 7 at operand 3 -- `lsl z0\.b,z0\.b,#8'
.*: Error: immediate value out of range 0 to 7 at operand 3 -- `lsl z0\.b,z0\.b,#9'
.*: Error: expected an SVE vector register at operand 3 -- `lsl z0\.b,z0\.b,x0'
.*: Error: immediate value out of range 0 to 15 at operand 3 -- `lsl z0\.h,z0\.h,#-1'
.*: Error: immediate value out of range 0 to 15 at operand 3 -- `lsl z0\.h,z0\.h,#16'
.*: Error: immediate value out of range 0 to 15 at operand 3 -- `lsl z0\.h,z0\.h,#17'
.*: Error: immediate value out of range 0 to 31 at operand 3 -- `lsl z0\.s,z0\.s,#-1'
.*: Error: immediate value out of range 0 to 31 at operand 3 -- `lsl z0\.s,z0\.s,#32'
.*: Error: immediate value out of range 0 to 31 at operand 3 -- `lsl z0\.s,z0\.s,#33'
.*: Error: immediate value out of range 0 to 63 at operand 3 -- `lsl z0\.d,z0\.d,#-1'
.*: Error: immediate value out of range 0 to 63 at operand 3 -- `lsl z0\.d,z0\.d,#64'
.*: Error: immediate value out of range 0 to 63 at operand 3 -- `lsl z0\.d,z0\.d,#65'
.*: Error: immediate value out of range 0 to 7 at operand 4 -- `lsl z0\.b,p1/m,z0\.b,#-1'
.*: Error: immediate value out of range 0 to 7 at operand 4 -- `lsl z0\.b,p1/m,z0\.b,#8'
.*: Error: immediate value out of range 0 to 7 at operand 4 -- `lsl z0\.b,p1/m,z0\.b,#9'
.*: Error: expected an SVE vector register at operand 4 -- `lsl z0\.b,p1/m,z0\.b,x0'
.*: Error: immediate value out of range 0 to 15 at operand 4 -- `lsl z0\.h,p1/m,z0\.h,#-1'
.*: Error: immediate value out of range 0 to 15 at operand 4 -- `lsl z0\.h,p1/m,z0\.h,#16'
.*: Error: immediate value out of range 0 to 15 at operand 4 -- `lsl z0\.h,p1/m,z0\.h,#17'
.*: Error: immediate value out of range 0 to 31 at operand 4 -- `lsl z0\.s,p1/m,z0\.s,#-1'
.*: Error: immediate value out of range 0 to 31 at operand 4 -- `lsl z0\.s,p1/m,z0\.s,#32'
.*: Error: immediate value out of range 0 to 31 at operand 4 -- `lsl z0\.s,p1/m,z0\.s,#33'
.*: Error: immediate value out of range 0 to 63 at operand 4 -- `lsl z0\.d,p1/m,z0\.d,#-1'
.*: Error: immediate value out of range 0 to 63 at operand 4 -- `lsl z0\.d,p1/m,z0\.d,#64'
.*: Error: immediate value out of range 0 to 63 at operand 4 -- `lsl z0\.d,p1/m,z0\.d,#65'
.*: Error: immediate value out of range 1 to 8 at operand 3 -- `lsr z0\.b,z0\.b,#-1'
.*: Error: immediate value out of range 1 to 8 at operand 3 -- `lsr z0\.b,z0\.b,#0'
.*: Error: immediate value out of range 1 to 8 at operand 3 -- `lsr z0\.b,z0\.b,#9'
.*: Error: expected an SVE vector register at operand 3 -- `lsr z0\.b,z0\.b,x0'
.*: Error: immediate value out of range 1 to 16 at operand 3 -- `lsr z0\.h,z0\.h,#-1'
.*: Error: immediate value out of range 1 to 16 at operand 3 -- `lsr z0\.h,z0\.h,#0'
.*: Error: immediate value out of range 1 to 16 at operand 3 -- `lsr z0\.h,z0\.h,#17'
.*: Error: immediate value out of range 1 to 32 at operand 3 -- `lsr z0\.s,z0\.s,#-1'
.*: Error: immediate value out of range 1 to 32 at operand 3 -- `lsr z0\.s,z0\.s,#0'
.*: Error: immediate value out of range 1 to 32 at operand 3 -- `lsr z0\.s,z0\.s,#33'
.*: Error: immediate value out of range 1 to 64 at operand 3 -- `lsr z0\.d,z0\.d,#-1'
.*: Error: immediate value out of range 1 to 64 at operand 3 -- `lsr z0\.d,z0\.d,#0'
.*: Error: immediate value out of range 1 to 64 at operand 3 -- `lsr z0\.d,z0\.d,#65'
.*: Error: immediate value out of range 1 to 8 at operand 4 -- `lsr z0\.b,p1/m,z0\.b,#-1'
.*: Error: immediate value out of range 1 to 8 at operand 4 -- `lsr z0\.b,p1/m,z0\.b,#0'
.*: Error: immediate value out of range 1 to 8 at operand 4 -- `lsr z0\.b,p1/m,z0\.b,#9'
.*: Error: expected an SVE vector register at operand 4 -- `lsr z0\.b,p1/m,z0\.b,x0'
.*: Error: immediate value out of range 1 to 16 at operand 4 -- `lsr z0\.h,p1/m,z0\.h,#-1'
.*: Error: immediate value out of range 1 to 16 at operand 4 -- `lsr z0\.h,p1/m,z0\.h,#0'
.*: Error: immediate value out of range 1 to 16 at operand 4 -- `lsr z0\.h,p1/m,z0\.h,#17'
.*: Error: immediate value out of range 1 to 32 at operand 4 -- `lsr z0\.s,p1/m,z0\.s,#-1'
.*: Error: immediate value out of range 1 to 32 at operand 4 -- `lsr z0\.s,p1/m,z0\.s,#0'
.*: Error: immediate value out of range 1 to 32 at operand 4 -- `lsr z0\.s,p1/m,z0\.s,#33'
.*: Error: immediate value out of range 1 to 64 at operand 4 -- `lsr z0\.d,p1/m,z0\.d,#-1'
.*: Error: immediate value out of range 1 to 64 at operand 4 -- `lsr z0\.d,p1/m,z0\.d,#0'
.*: Error: immediate value out of range 1 to 64 at operand 4 -- `lsr z0\.d,p1/m,z0\.d,#65'
.*: Error: immediate value out of range -16 to 15 at operand 2 -- `index z0\.s,#-17,#1'
.*: Error: immediate value out of range -16 to 15 at operand 2 -- `index z0\.s,#16,#1'
.*: Error: immediate value out of range -16 to 15 at operand 3 -- `index z0\.s,#0,#-17'
.*: Error: immediate value out of range -16 to 15 at operand 3 -- `index z0\.s,#0,#16'
.*: Error: immediate value out of range -32 to 31 at operand 3 -- `addpl x0,sp,#-33'
.*: Error: immediate value out of range -32 to 31 at operand 3 -- `addpl sp,x0,#32'
.*: Error: expected an integer or stack pointer register at operand 2 -- `addpl x0,xzr,#1'
.*: Error: expected an integer or stack pointer register at operand 1 -- `addpl xzr,x0,#1'
.*: Error: immediate value out of range -128 to 127 at operand 3 -- `mul z0\.b,z0\.b,#-129'
.*: Error: immediate value out of range -128 to 127 at operand 3 -- `mul z0\.b,z0\.b,#128'
.*: Error: immediate value out of range -128 to 127 at operand 3 -- `mul z0\.s,z0\.s,#-129'
.*: Error: immediate value out of range -128 to 127 at operand 3 -- `mul z0\.s,z0\.s,#128'
.*: Error: immediate value out of range 0 to 7 at operand 4 -- `ftmad z0\.s,z0\.s,z1\.s,#-1'
.*: Error: immediate value out of range 0 to 7 at operand 4 -- `ftmad z0\.s,z0\.s,z1\.s,#8'
.*: Error: immediate operand required at operand 4 -- `ftmad z0\.s,z0\.s,z1\.s,z2\.s'
.*: Error: immediate value out of range 0 to 127 at operand 4 -- `cmphi p0\.s,p1/z,z2\.s,#-1'
.*: Error: immediate value out of range 0 to 127 at operand 4 -- `cmphi p0\.s,p1/z,z2\.s,#128'
.*: Error: immediate value out of range 0 to 255 at operand 3 -- `umax z0\.s,z0\.s,#-1'
.*: Error: immediate value out of range 0 to 255 at operand 3 -- `umax z0\.s,z0\.s,#256'
.*: Error: immediate value out of range 0 to 255 at operand 4 -- `ext z0\.b,z0\.b,z1\.b,#-1'
.*: Error: immediate value out of range 0 to 255 at operand 4 -- `ext z0\.b,z0\.b,z1\.b,#256'
.*: Error: register element index out of range 0 to 63 at operand 2 -- `dup z0\.b,z1\.b\[-1\]'
.*: Error: register element index out of range 0 to 63 at operand 2 -- `dup z0\.b,z1\.b\[64\]'
.*: Error: constant expression required at operand 2 -- `dup z0\.b,z1\.b\[x0\]'
.*: Error: operand mismatch -- `dup z0\.b,z1\[0\]'
.*: Info:    did you mean this\?
.*: Info:    	dup z0\.b, z1\.b\[0\]
.*: Info:    other valid variant\(s\):
.*: Info:    	dup z0\.h, z1\.h\[0\]
.*: Info:    	dup z0\.s, z1\.s\[0\]
.*: Info:    	dup z0\.d, z1\.d\[0\]
.*: Info:    	dup z0\.q, z1\.q\[0\]
.*: Error: register element index out of range 0 to 31 at operand 2 -- `dup z0\.h,z1\.h\[-1\]'
.*: Error: register element index out of range 0 to 31 at operand 2 -- `dup z0\.h,z1\.h\[32\]'
.*: Error: constant expression required at operand 2 -- `dup z0\.h,z1\.h\[x0\]'
.*: Error: register element index out of range 0 to 15 at operand 2 -- `dup z0\.s,z1\.s\[-1\]'
.*: Error: register element index out of range 0 to 15 at operand 2 -- `dup z0\.s,z1\.s\[16\]'
.*: Error: constant expression required at operand 2 -- `dup z0\.s,z1\.s\[x0\]'
.*: Error: register element index out of range 0 to 7 at operand 2 -- `dup z0\.d,z1\.d\[-1\]'
.*: Error: register element index out of range 0 to 7 at operand 2 -- `dup z0\.d,z1\.d\[8\]'
.*: Error: constant expression required at operand 2 -- `dup z0\.d,z1\.d\[x0\]'
.*: Error: operand mismatch -- `fabd z0\.b,p0/m,z0\.b,z0\.b'
.*: Info:    did you mean this\?
.*: Info:    	fabd z0\.h, p0/m, z0\.h, z0\.h
.*: Info:    other valid variant\(s\):
.*: Info:    	fabd z0\.s, p0/m, z0\.s, z0\.s
.*: Info:    	fabd z0\.d, p0/m, z0\.d, z0\.d
.*: Error: operand mismatch -- `fabd z0\.q,p0/m,z0\.q,z0\.q'
.*: Info:    did you mean this\?
.*: Info:    	fabd z0\.h, p0/m, z0\.h, z0\.h
.*: Info:    other valid variant\(s\):
.*: Info:    	fabd z0\.s, p0/m, z0\.s, z0\.s
.*: Info:    	fabd z0\.d, p0/m, z0\.d, z0\.d
.*: Error: operand mismatch -- `fcadd z0\.b,p0/m,z0\.b,z0\.b,#90'
.*: Info:    did you mean this\?
.*: Info:    	fcadd z0\.h, p0/m, z0\.h, z0\.h, #90
.*: Info:    other valid variant\(s\):
.*: Info:    	fcadd z0\.s, p0/m, z0\.s, z0\.s, #90
.*: Info:    	fcadd z0\.d, p0/m, z0\.d, z0\.d, #90
.*: Error: rotate expected to be 90 or 270 at operand 5 -- `fcadd z0\.h,p0/m,z0\.h,z0\.h,#-180'
.*: Error: rotate expected to be 90 or 270 at operand 5 -- `fcadd z0\.h,p0/m,z0\.h,z0\.h,#-90'
.*: Error: rotate expected to be 90 or 270 at operand 5 -- `fcadd z0\.h,p0/m,z0\.h,z0\.h,#0'
.*: Error: rotate expected to be 90 or 270 at operand 5 -- `fcadd z0\.h,p0/m,z0\.h,z0\.h,#89'
.*: Error: unexpected characters following instruction at operand 5 -- `fcadd z0\.h,p0/m,z0\.h,z0\.h,#90\.0'
.*: Error: rotate expected to be 90 or 270 at operand 5 -- `fcadd z0\.h,p0/m,z0\.h,z0\.h,#180'
.*: Error: rotate expected to be 90 or 270 at operand 5 -- `fcadd z0\.h,p0/m,z0\.h,z0\.h,#360'
.*: Error: rotate expected to be 90 or 270 at operand 5 -- `fcadd z0\.h,p0/m,z0\.h,z0\.h,#450'
.*: Error: operand mismatch -- `fcadd z0\.h,p0/z,z0\.h,z0\.h,#90'
.*: Info:    did you mean this\?
.*: Info:    	fcadd z0\.h, p0/m, z0\.h, z0\.h, #90
.*: Info:    other valid variant\(s\):
.*: Info:    	fcadd z0\.s, p0/m, z0\.s, z0\.s, #90
.*: Info:    	fcadd z0\.d, p0/m, z0\.d, z0\.d, #90
.*: Error: operand 3 must be the same register as operand 1 -- `fcadd z0\.h,p0/m,z1\.h,z0\.h,#90'
.*: Error: operand mismatch -- `fcadd z0\.q,p0/m,z0\.q,z0\.q,#90'
.*: Info:    did you mean this\?
.*: Info:    	fcadd z0\.h, p0/m, z0\.h, z0\.h, #90
.*: Info:    other valid variant\(s\):
.*: Info:    	fcadd z0\.s, p0/m, z0\.s, z0\.s, #90
.*: Info:    	fcadd z0\.d, p0/m, z0\.d, z0\.d, #90
.*: Error: operand mismatch -- `fcmla z0\.b,p0/m,z0\.b,z0\.b,#90'
.*: Info:    did you mean this\?
.*: Info:    	fcmla z0\.h, p0/m, z0\.h, z0\.h, #90
.*: Info:    other valid variant\(s\):
.*: Info:    	fcmla z0\.s, p0/m, z0\.s, z0\.s, #90
.*: Info:    	fcmla z0\.d, p0/m, z0\.d, z0\.d, #90
.*: Error: rotate expected to be 0, 90, 180 or 270 at operand 5 -- `fcmla z0\.h,p0/m,z0\.h,z0\.h,#-180'
.*: Error: rotate expected to be 0, 90, 180 or 270 at operand 5 -- `fcmla z0\.h,p0/m,z0\.h,z0\.h,#-90'
.*: Error: rotate expected to be 0, 90, 180 or 270 at operand 5 -- `fcmla z0\.h,p0/m,z0\.h,z0\.h,#89'
.*: Error: unexpected characters following instruction at operand 5 -- `fcmla z0\.h,p0/m,z0\.h,z0\.h,#90\.0'
.*: Error: rotate expected to be 0, 90, 180 or 270 at operand 5 -- `fcmla z0\.h,p0/m,z0\.h,z0\.h,#360'
.*: Error: rotate expected to be 0, 90, 180 or 270 at operand 5 -- `fcmla z0\.h,p0/m,z0\.h,z0\.h,#450'
.*: Error: operand mismatch -- `fcmla z0\.h,p0/z,z0\.h,z0\.h,#90'
.*: Info:    did you mean this\?
.*: Info:    	fcmla z0\.h, p0/m, z0\.h, z0\.h, #90
.*: Info:    other valid variant\(s\):
.*: Info:    	fcmla z0\.s, p0/m, z0\.s, z0\.s, #90
.*: Info:    	fcmla z0\.d, p0/m, z0\.d, z0\.d, #90
.*: Error: operand mismatch -- `fcmla z0\.q,p0/m,z0\.q,z0\.q,#90'
.*: Info:    did you mean this\?
.*: Info:    	fcmla z0\.h, p0/m, z0\.h, z0\.h, #90
.*: Info:    other valid variant\(s\):
.*: Info:    	fcmla z0\.s, p0/m, z0\.s, z0\.s, #90
.*: Info:    	fcmla z0\.d, p0/m, z0\.d, z0\.d, #90
.*: Error: operand mismatch -- `fcmla z0\.b,z1\.b,z2\.b\[0\],#0'
.*: Info:    did you mean this\?
.*: Info:    	fcmla z0\.h, z1\.h, z2\.h\[0\], #0
.*: Error: register element index out of range 0 to 3 at operand 3 -- `fcmla z0\.h,z1\.h,z2\.h\[-1\],#0'
.*: Error: register element index out of range 0 to 3 at operand 3 -- `fcmla z0\.h,z1\.h,z2\.h\[4\],#0'
.*: Error: z0-z7 expected at operand 3 -- `fcmla z0\.h,z1\.h,z8\.h\[0\],#0'
.*: Error: rotate expected to be 0, 90, 180 or 270 at operand 4 -- `fcmla z0\.h,z1\.h,z2\.h\[0\],#-180'
.*: Error: rotate expected to be 0, 90, 180 or 270 at operand 4 -- `fcmla z0\.h,z1\.h,z2\.h\[0\],#-90'
.*: Error: rotate expected to be 0, 90, 180 or 270 at operand 4 -- `fcmla z0\.h,z1\.h,z2\.h\[0\],#89'
.*: Error: unexpected characters following instruction at operand 4 -- `fcmla z0\.h,z1\.h,z2\.h\[0\],#90\.0'
.*: Error: rotate expected to be 0, 90, 180 or 270 at operand 4 -- `fcmla z0\.h,z1\.h,z2\.h\[0\],#360'
.*: Error: rotate expected to be 0, 90, 180 or 270 at operand 4 -- `fcmla z0\.h,z1\.h,z2\.h\[0\],#450'
.*: Error: register element index out of range 0 to 1 at operand 3 -- `fcmla z0\.s,z1\.s,z2\.s\[-1\],#0'
.*: Error: register element index out of range 0 to 1 at operand 3 -- `fcmla z0\.s,z1\.s,z2\.s\[2\],#0'
.*: Error: z0-z15 expected at operand 3 -- `fcmla z0\.s,z1\.s,z16\.s\[0\],#0'
.*: Error: rotate expected to be 0, 90, 180 or 270 at operand 4 -- `fcmla z0\.s,z1\.s,z2\.s\[0\],#-180'
.*: Error: rotate expected to be 0, 90, 180 or 270 at operand 4 -- `fcmla z0\.s,z1\.s,z2\.s\[0\],#-90'
.*: Error: rotate expected to be 0, 90, 180 or 270 at operand 4 -- `fcmla z0\.s,z1\.s,z2\.s\[0\],#89'
.*: Error: unexpected characters following instruction at operand 4 -- `fcmla z0\.s,z1\.s,z2\.s\[0\],#90\.0'
.*: Error: rotate expected to be 0, 90, 180 or 270 at operand 4 -- `fcmla z0\.s,z1\.s,z2\.s\[0\],#360'
.*: Error: rotate expected to be 0, 90, 180 or 270 at operand 4 -- `fcmla z0\.s,z1\.s,z2\.s\[0\],#450'
.*: Error: operand mismatch -- `fcmla z0\.q,z1\.q,z2\.q\[0\],#0'
.*: Info:    did you mean this\?
.*: Info:    	fcmla z0\.h, z1\.h, z2\.h\[0\], #0
.*: Error: operand mismatch -- `fmla z0\.b,z1\.b,z2\.b\[0\]'
.*: Info:    did you mean this\?
.*: Info:    	fmla z0\.h, z1\.h, z2\.h\[0\]
.*: Error: register element index out of range 0 to 7 at operand 3 -- `fmla z0\.h,z1\.h,z2\.h\[-1\]'
.*: Error: register element index out of range 0 to 7 at operand 3 -- `fmla z0\.h,z1\.h,z2\.h\[8\]'
.*: Error: z0-z7 expected at operand 3 -- `fmla z0\.h,z1\.h,z8\.h\[0\]'
.*: Error: register element index out of range 0 to 3 at operand 3 -- `fmla z0\.s,z1\.s,z2\.s\[-1\]'
.*: Error: register element index out of range 0 to 3 at operand 3 -- `fmla z0\.s,z1\.s,z2\.s\[4\]'
.*: Error: z0-z7 expected at operand 3 -- `fmla z0\.s,z1\.s,z8\.s\[0\]'
.*: Error: register element index out of range 0 to 1 at operand 3 -- `fmla z0\.d,z1\.d,z2\.d\[-1\]'
.*: Error: register element index out of range 0 to 1 at operand 3 -- `fmla z0\.d,z1\.d,z2\.d\[2\]'
.*: Error: z0-z15 expected at operand 3 -- `fmla z0\.d,z1\.d,z16\.d\[0\]'
.*: Error: operand mismatch -- `fmla z0\.q,z1\.q,z2\.q\[0\]'
.*: Info:    did you mean this\?
.*: Info:    	fmla z0\.h, z1\.h, z2\.h\[0\]
.*: Error: operand mismatch -- `fmls z0\.b,z1\.b,z2\.b\[0\]'
.*: Info:    did you mean this\?
.*: Info:    	fmls z0\.h, z1\.h, z2\.h\[0\]
.*: Error: register element index out of range 0 to 7 at operand 3 -- `fmls z0\.h,z1\.h,z2\.h\[-1\]'
.*: Error: register element index out of range 0 to 7 at operand 3 -- `fmls z0\.h,z1\.h,z2\.h\[8\]'
.*: Error: z0-z7 expected at operand 3 -- `fmls z0\.h,z1\.h,z8\.h\[0\]'
.*: Error: register element index out of range 0 to 3 at operand 3 -- `fmls z0\.s,z1\.s,z2\.s\[-1\]'
.*: Error: register element index out of range 0 to 3 at operand 3 -- `fmls z0\.s,z1\.s,z2\.s\[4\]'
.*: Error: z0-z7 expected at operand 3 -- `fmls z0\.s,z1\.s,z8\.s\[0\]'
.*: Error: register element index out of range 0 to 1 at operand 3 -- `fmls z0\.d,z1\.d,z2\.d\[-1\]'
.*: Error: register element index out of range 0 to 1 at operand 3 -- `fmls z0\.d,z1\.d,z2\.d\[2\]'
.*: Error: z0-z15 expected at operand 3 -- `fmls z0\.d,z1\.d,z16\.d\[0\]'
.*: Error: operand mismatch -- `fmls z0\.q,z1\.q,z2\.q\[0\]'
.*: Info:    did you mean this\?
.*: Info:    	fmls z0\.h, z1\.h, z2\.h\[0\]
.*: Error: operand mismatch -- `fmul z0\.b,z1\.b,z2\.b\[0\]'
.*: Info:    did you mean this\?
.*: Info:    	fmul z0\.h, z1\.h, z2\.h\[0\]
.*: Error: register element index out of range 0 to 7 at operand 3 -- `fmul z0\.h,z1\.h,z2\.h\[-1\]'
.*: Error: register element index out of range 0 to 7 at operand 3 -- `fmul z0\.h,z1\.h,z2\.h\[8\]'
.*: Error: z0-z7 expected at operand 3 -- `fmul z0\.h,z1\.h,z8\.h\[0\]'
.*: Error: register element index out of range 0 to 3 at operand 3 -- `fmul z0\.s,z1\.s,z2\.s\[-1\]'
.*: Error: register element index out of range 0 to 3 at operand 3 -- `fmul z0\.s,z1\.s,z2\.s\[4\]'
.*: Error: z0-z7 expected at operand 3 -- `fmul z0\.s,z1\.s,z8\.s\[0\]'
.*: Error: register element index out of range 0 to 1 at operand 3 -- `fmul z0\.d,z1\.d,z2\.d\[-1\]'
.*: Error: register element index out of range 0 to 1 at operand 3 -- `fmul z0\.d,z1\.d,z2\.d\[2\]'
.*: Error: z0-z15 expected at operand 3 -- `fmul z0\.d,z1\.d,z16\.d\[0\]'
.*: Error: operand mismatch -- `fmul z0\.q,z1\.q,z2\.q\[0\]'
.*: Info:    did you mean this\?
.*: Info:    	fmul z0\.h, z1\.h, z2\.h\[0\]
.*: Error: operand mismatch -- `ld1rqb {z0\.b},p0,\[x0,#0\]'
.*: Info:    did you mean this\?
.*: Info:    	ld1rqb {z0\.b}, p0/z, \[x0\]
.*: Error: operand mismatch -- `ld1rqb {z0\.b},p0/m,\[x0,#0\]'
.*: Info:    did you mean this\?
.*: Info:    	ld1rqb {z0\.b}, p0/z, \[x0\]
.*: Error: p0-p7 expected at operand 2 -- `ld1rqb {z0\.b},p8/z,\[x0,#0\]'
.*: Error: immediate offset out of range -128 to 112 at operand 3 -- `ld1rqb {z0\.b},p0/z,\[x0,#-144\]'
.*: Error: immediate value must be a multiple of 16 at operand 3 -- `ld1rqb {z0\.b},p0/z,\[x0,#-15\]'
.*: Error: immediate value must be a multiple of 16 at operand 3 -- `ld1rqb {z0\.b},p0/z,\[x0,#-14\]'
.*: Error: immediate value must be a multiple of 16 at operand 3 -- `ld1rqb {z0\.b},p0/z,\[x0,#-13\]'
.*: Error: immediate value must be a multiple of 16 at operand 3 -- `ld1rqb {z0\.b},p0/z,\[x0,#-12\]'
.*: Error: immediate value must be a multiple of 16 at operand 3 -- `ld1rqb {z0\.b},p0/z,\[x0,#-11\]'
.*: Error: immediate value must be a multiple of 16 at operand 3 -- `ld1rqb {z0\.b},p0/z,\[x0,#-10\]'
.*: Error: immediate value must be a multiple of 16 at operand 3 -- `ld1rqb {z0\.b},p0/z,\[x0,#-9\]'
.*: Error: immediate value must be a multiple of 16 at operand 3 -- `ld1rqb {z0\.b},p0/z,\[x0,#-8\]'
.*: Error: immediate value must be a multiple of 16 at operand 3 -- `ld1rqb {z0\.b},p0/z,\[x0,#-7\]'
.*: Error: immediate value must be a multiple of 16 at operand 3 -- `ld1rqb {z0\.b},p0/z,\[x0,#-6\]'
.*: Error: immediate value must be a multiple of 16 at operand 3 -- `ld1rqb {z0\.b},p0/z,\[x0,#-5\]'
.*: Error: immediate value must be a multiple of 16 at operand 3 -- `ld1rqb {z0\.b},p0/z,\[x0,#-4\]'
.*: Error: immediate value must be a multiple of 16 at operand 3 -- `ld1rqb {z0\.b},p0/z,\[x0,#-3\]'
.*: Error: immediate value must be a multiple of 16 at operand 3 -- `ld1rqb {z0\.b},p0/z,\[x0,#-2\]'
.*: Error: immediate value must be a multiple of 16 at operand 3 -- `ld1rqb {z0\.b},p0/z,\[x0,#-1\]'
.*: Error: immediate value must be a multiple of 16 at operand 3 -- `ld1rqb {z0\.b},p0/z,\[x0,#1\]'
.*: Error: immediate value must be a multiple of 16 at operand 3 -- `ld1rqb {z0\.b},p0/z,\[x0,#2\]'
.*: Error: immediate value must be a multiple of 16 at operand 3 -- `ld1rqb {z0\.b},p0/z,\[x0,#3\]'
.*: Error: immediate value must be a multiple of 16 at operand 3 -- `ld1rqb {z0\.b},p0/z,\[x0,#4\]'
.*: Error: immediate value must be a multiple of 16 at operand 3 -- `ld1rqb {z0\.b},p0/z,\[x0,#5\]'
.*: Error: immediate value must be a multiple of 16 at operand 3 -- `ld1rqb {z0\.b},p0/z,\[x0,#6\]'
.*: Error: immediate value must be a multiple of 16 at operand 3 -- `ld1rqb {z0\.b},p0/z,\[x0,#7\]'
.*: Error: immediate value must be a multiple of 16 at operand 3 -- `ld1rqb {z0\.b},p0/z,\[x0,#8\]'
.*: Error: immediate value must be a multiple of 16 at operand 3 -- `ld1rqb {z0\.b},p0/z,\[x0,#9\]'
.*: Error: immediate value must be a multiple of 16 at operand 3 -- `ld1rqb {z0\.b},p0/z,\[x0,#10\]'
.*: Error: immediate value must be a multiple of 16 at operand 3 -- `ld1rqb {z0\.b},p0/z,\[x0,#11\]'
.*: Error: immediate value must be a multiple of 16 at operand 3 -- `ld1rqb {z0\.b},p0/z,\[x0,#12\]'
.*: Error: immediate value must be a multiple of 16 at operand 3 -- `ld1rqb {z0\.b},p0/z,\[x0,#13\]'
.*: Error: immediate value must be a multiple of 16 at operand 3 -- `ld1rqb {z0\.b},p0/z,\[x0,#14\]'
.*: Error: immediate value must be a multiple of 16 at operand 3 -- `ld1rqb {z0\.b},p0/z,\[x0,#15\]'
.*: Error: immediate offset out of range -128 to 112 at operand 3 -- `ld1rqb {z0\.b},p0/z,\[x0,#128\]'
.*: Error: operand mismatch -- `ld1rqb {z0\.h},p0/z,\[x0,#0\]'
.*: Info:    did you mean this\?
.*: Info:    	ld1rqb {z0\.b}, p0/z, \[x0\]
.*: Error: operand mismatch -- `ld1rqb {z0\.s},p0/z,\[x0,#0\]'
.*: Info:    did you mean this\?
.*: Info:    	ld1rqb {z0\.b}, p0/z, \[x0\]
.*: Error: operand mismatch -- `ld1rqb {z0\.d},p0/z,\[x0,#0\]'
.*: Info:    did you mean this\?
.*: Info:    	ld1rqb {z0\.b}, p0/z, \[x0\]
.*: Error: operand mismatch -- `ld1rqb {z0\.q},p0/z,\[x0,#0\]'
.*: Info:    did you mean this\?
.*: Info:    	ld1rqb {z0\.b}, p0/z, \[x0\]
.*: Error: index register xzr is not allowed at operand 3 -- `ld1rqb {z0\.b},p0/z,\[x0,xzr\]'
.*: Error: invalid addressing mode at operand 3 -- `ld1rqb {z0\.b},p0/z,\[x0,x1,lsl#1\]'
.*: Error: invalid addressing mode at operand 3 -- `ld1rqb {z0\.b},p0/z,\[x0,x1,lsl#2\]'
.*: Error: invalid addressing mode at operand 3 -- `ld1rqb {z0\.b},p0/z,\[x0,x1,lsl#3\]'
.*: Error: index register xzr is not allowed at operand 3 -- `ld1rqh {z0\.h},p0/z,\[x0,xzr,lsl#1\]'
.*: Error: invalid addressing mode at operand 3 -- `ld1rqh {z0\.h},p0/z,\[x0,x1\]'
.*: Error: invalid addressing mode at operand 3 -- `ld1rqh {z0\.h},p0/z,\[x0,x1,lsl#2\]'
.*: Error: invalid addressing mode at operand 3 -- `ld1rqh {z0\.h},p0/z,\[x0,x1,lsl#3\]'
.*: Error: index register xzr is not allowed at operand 3 -- `ld1rqw {z0\.s},p0/z,\[x0,xzr,lsl#2\]'
.*: Error: invalid addressing mode at operand 3 -- `ld1rqw {z0\.s},p0/z,\[x0,x1\]'
.*: Error: invalid addressing mode at operand 3 -- `ld1rqw {z0\.s},p0/z,\[x0,x1,lsl#1\]'
.*: Error: invalid addressing mode at operand 3 -- `ld1rqw {z0\.s},p0/z,\[x0,x1,lsl#3\]'
.*: Error: index register xzr is not allowed at operand 3 -- `ld1rqd {z0\.d},p0/z,\[x0,xzr,lsl#3\]'
.*: Error: invalid addressing mode at operand 3 -- `ld1rqd {z0\.d},p0/z,\[x0,x1\]'
.*: Error: invalid addressing mode at operand 3 -- `ld1rqd {z0\.d},p0/z,\[x0,x1,lsl#1\]'
.*: Error: invalid addressing mode at operand 3 -- `ld1rqd {z0\.d},p0/z,\[x0,x1,lsl#2\]'
.*: Error: operand mismatch -- `sdot z0\.b,z1\.b,z2\.b'
.*: Info:    did you mean this\?
.*: Info:    	sdot z0\.s, z1\.b, z2\.b
.*: Info:    other valid variant\(s\):
.*: Info:    	sdot z0\.d, z1\.h, z2\.h
.*: Error: operand mismatch -- `sdot z0\.h,z1\.h,z2\.h'
.*: Info:    did you mean this\?
.*: Info:    	sdot z0\.s, z1\.h, z2\.h
.*: Error: operand mismatch -- `sdot z0\.s,z1\.s,z2\.s'
.*: Info:    did you mean this\?
.*: Info:    	sdot z0\.s, z1\.h, z2\.h
.*: Error: operand mismatch -- `sdot z0\.d,z1\.d,z2\.d'
.*: Info:    did you mean this\?
.*: Info:    	sdot z0\.d, z1\.h, z2\.h
.*: Info:    other valid variant\(s\):
.*: Info:    	sdot z0\.s, z1\.b, z2\.b
.*: Error: operand mismatch -- `sdot z0\.b,z1\.b,z2\.b\[0\]'
.*: Info:    did you mean this\?
.*: Info:    	sdot z0\.s, z1\.b, z2\.b\[0\]
.*: Error: operand mismatch -- `sdot z0\.h,z1\.h,z2\.h\[0\]'
.*: Info:    did you mean this\?
.*: Info:    	sdot z0\.d, z1\.h, z2\.h\[0\]
.*: Error: operand mismatch -- `sdot z0\.s,z1\.s,z2\.s\[0\]'
.*: Info:    did you mean this\?
.*: Info:    	sdot z0\.s, z1\.b, z2\.b\[0\]
.*: Error: operand mismatch -- `sdot z0\.d,z1\.d,z2\.d\[0\]'
.*: Info:    did you mean this\?
.*: Info:    	sdot z0\.d, z1\.h, z2\.h\[0\]
.*: Error: operand mismatch -- `udot z0\.b,z1\.b,z2\.b'
.*: Info:    did you mean this\?
.*: Info:    	udot z0\.s, z1\.b, z2\.b
.*: Info:    other valid variant\(s\):
.*: Info:    	udot z0\.d, z1\.h, z2\.h
.*: Error: operand mismatch -- `udot z0\.h,z1\.h,z2\.h'
.*: Info:    did you mean this\?
.*: Info:    	udot z0\.s, z1\.h, z2\.h
.*: Error: operand mismatch -- `udot z0\.s,z1\.s,z2\.s'
.*: Info:    did you mean this\?
.*: Info:    	udot z0\.s, z1\.h, z2\.h
.*: Error: operand mismatch -- `udot z0\.d,z1\.d,z2\.d'
.*: Info:    did you mean this\?
.*: Info:    	udot z0\.d, z1\.h, z2\.h
.*: Info:    other valid variant\(s\):
.*: Info:    	udot z0\.s, z1\.b, z2\.b
.*: Error: operand mismatch -- `udot z0\.b,z1\.b,z2\.b\[0\]'
.*: Info:    did you mean this\?
.*: Info:    	udot z0\.s, z1\.b, z2\.b\[0\]
.*: Error: operand mismatch -- `udot z0\.h,z1\.h,z2\.h\[0\]'
.*: Info:    did you mean this\?
.*: Info:    	udot z0\.d, z1\.h, z2\.h\[0\]
.*: Error: operand mismatch -- `udot z0\.s,z1\.s,z2\.s\[0\]'
.*: Info:    did you mean this\?
.*: Info:    	udot z0\.s, z1\.b, z2\.b\[0\]
.*: Error: operand mismatch -- `udot z0\.d,z1\.d,z2\.d\[0\]'
.*: Info:    did you mean this\?
.*: Info:    	udot z0\.d, z1\.h, z2\.h\[0\]
.*: Error: syntax error in register list at operand 1 -- `ld2b {},p0/z,\[x0\]'
.*: Error: syntax error in register list at operand 1 -- `ld2b {.b},p0/z,\[x0\]'
.*: Error: syntax error in register list at operand 1 -- `ld2b {z0.b-},p0/z,\[x0\]'
.*: Error: syntax error in register list at operand 1 -- `ld2b {z0.b,},p0/z,\[x0\]'
.*: Error: operand 1 must be a list of SVE vector registers -- `ld2b {z0\.b-z32\.b},p0/z,\[x0\]'
.*: Error: operand 1 must be a list of SVE vector registers -- `ld2b {z0\.b-v1\.16b},p0/z,\[x0\]'
