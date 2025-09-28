[^:]*: Assembler messages:
[^:]*:[0-9]+: Error: ZA tile number out of range at operand 3 -- `mova z0\.b,p0/m,za1h\.b\[w12,#0\]'
[^:]*:[0-9]+: Error: ZA tile number out of range at operand 3 -- `mova z0\.h,p0/m,za2h\.h\[w12,#0\]'
[^:]*:[0-9]+: Error: ZA tile number out of range at operand 3 -- `mova z0\.s,p0/m,za4h\.s\[w12,#0\]'
[^:]*:[0-9]+: Error: ZA tile number out of range at operand 3 -- `mova z0\.d,p0/m,za8h\.d\[w12,#0\]'
[^:]*:[0-9]+: Error: expected a ZA tile slice at operand 3 -- `mova z0\.q,p0/m,za16h.q\[w12\]'
[^:]*:[0-9]+: Error: immediate offset out of range 0 to 15 at operand 3 -- `mova z31\.b,p7/m,za0v\.b\[w15,#16\]'
[^:]*:[0-9]+: Error: immediate offset out of range 0 to 7 at operand 3 -- `mova z31\.h,p7/m,za1v\.h\[w15,#8\]'
[^:]*:[0-9]+: Error: immediate offset out of range 0 to 3 at operand 3 -- `mova z31\.s,p7/m,za3v\.s\[w15,#4\]'
[^:]*:[0-9]+: Error: immediate offset out of range 0 to 1 at operand 3 -- `mova z31\.d,p7/m,za7v\.d\[w15,#2\]'
[^:]*:[0-9]+: Error: immediate offset must be 0 at operand 3 -- `mova z31\.q,p7/m,za15v\.q\[w15,#1\]'
[^:]*:[0-9]+: Error: missing immediate offset at operand 3 -- `mova z31\.q,p7/m,za15v\.q\[w15\]'
[^:]*:[0-9]+: Error: expected '\[' at operand 3 -- `mova z0\.b,p0/m,za0v.b'
[^:]*:[0-9]+: Error: expected a 32-bit selection register at operand 3 -- `mova z31\.b,p7/m,za0v\.b\[15,w15\]'
[^:]*:[0-9]+: Error: missing immediate offset at operand 3 -- `mova z0\.h,p0/m,za0v\.h\[w12\. 0\]'
[^:]*:[0-9]+: Error: expected a 32-bit selection register at operand 3 -- `mova z0\.s,p0/m,za0v\.s\[x12,0]'
[^:]*:[0-9]+: Error: expected a selection register in the range w12-w15 at operand 3 -- `mova z0\.d,p0/m,za0v\.d\[w21,0\]'
[^:]*:[0-9]+: Error: expected a 32-bit selection register at operand 3 -- `mova z0\.q,p0/m,za0v\.q\[s12\]'
[^:]*:[0-9]+: Error: expected a 32-bit selection register at operand 3 -- `mova z0\.q,p0/m,za0v\.q\[d12\]'
[^:]*:[0-9]+: Error: expected a constant immediate offset at operand 3 -- `mova z0.q,p0/m,za0v\.q\[w12,\]'
[^:]*:[0-9]+: Error: missing immediate offset at operand 3 -- `mova z0\.q,p0/m,za0v\.q\[w12\.\]'
[^:]*:[0-9]+: Error: expected a constant immediate offset at operand 3 -- `mova z0\.q,p0/m,za0v\.q\[w12,abc\]'
[^:]*:[0-9]+: Error: expected a constant immediate offset at operand 3 -- `mova z0\.q,p0/m,za0v\.q\[w12,#abc\]'
[^:]*:[0-9]+: Error: expected '\]' at operand 3 -- `mova z0\.q,p0/m,za0v\.q\[w12,1a\]'
[^:]*:[0-9]+: Error: expected '\]' at operand 3 -- `mova z0\.q,p0/m,za0v\.q\[w12,#1a\]'
[^:]*:[0-9]+: Error: expected '\]' at operand 3 -- `mova z0\.q,p0/m,za0v\.q\[w12,1a2\]'
[^:]*:[0-9]+: Error: expected '\]' at operand 3 -- `mova z0\.q,p0/m,za0v\.q\[w12,#1a2\]'
[^:]*:[0-9]+: Error: unexpected vector group size at operand 3 -- `mova z0.b,p0/m,za0h.b\[w12,#0,vgx2\]'
[^:]*:[0-9]+: Error: unexpected vector group size at operand 3 -- `mova z0.b,p0/m,za0h.b\[w12,#0,vgx4\]'
[^:]*:[0-9]+: Error: invalid vector group size at operand 3 -- `mova z0.b,p0/m,za0h.b\[w12,#0,vgx8\]'
[^:]*:[0-9]+: Error: the last offset is less than the first offset at operand 3 -- `mova z0.b,p0/m,za0h.b\[w12,1:0\]'
[^:]*:[0-9]+: Error: the last offset is equal to the first offset at operand 3 -- `mova z0.b,p0/m,za0h.b\[w12,0:0\]'
[^:]*:[0-9]+: Error: expected a single offset rather than a range at operand 3 -- `mova z0.b,p0/m,za0h.b\[w12,0:1\]'
[^:]*:[0-9]+: Error: expected a single offset rather than a range at operand 3 -- `mova z0.b,p0/m,za0h.b\[w12,0:2\]'
[^:]+:[0-9]+: Error: operand mismatch -- `mova z0\.b,p0/m,za0h\.h\[w12,0:1\]'
[^:]+:[0-9]+: Info:    did you mean this\?
[^:]+:[0-9]+: Info:    	mova z0\.b, p0/m, za0h\.b\[w12, 0:1\]
[^:]+:[0-9]+: Info:    other valid variant\(s\):
[^:]+:[0-9]+: Info:    	mova z0\.h, p0/m, za0h\.h\[w12, 0:1\]
[^:]+:[0-9]+: Info:    	mova z0\.s, p0/m, za0h\.s\[w12, 0:1\]
[^:]+:[0-9]+: Info:    	mova z0\.d, p0/m, za0h\.d\[w12, 0:1\]
[^:]+:[0-9]+: Info:    	mova z0\.q, p0/m, za0h\.q\[w12, 0:1\]
[^:]*:[0-9]+: Error: expected a constant immediate offset at operand 3 -- `mova z0.b,p0/m,za0h.b\[w12,0:foo\]'
