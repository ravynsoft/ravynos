[^:]*: Assembler messages:
[^:]*:4: Error: missing shift amount at operand 2 -- `strb w7,\[x30,x0,lsl\]'
[^:]*:5: Error: operand mismatch -- `ubfm w0,x1,8,31'
[^:]*:5: Info:    did you mean this\?
[^:]*:5: Info:    	ubfm w0, w1, #8, #31
[^:]*:5: Info:    other valid variant\(s\):
[^:]*:5: Info:    	ubfm x0, x1, #8, #31
[^:]*:6: Error: immediate value out of range 0 to 31 at operand 4 -- `bfm w0,w1,8,43'
[^:]*:7: Error: invalid shift amount at operand 2 -- `strb w7,\[x30,x0,lsl#1\]'
[^:]*:8: Error: invalid addressing mode at operand 2 -- `st2 {v4.2d,v5.2d},\[x3,#3\]'
[^:]*:9: Error: the top half of a 128-bit FP/SIMD register is expected at operand 1 -- `fmov v1.D\[0\],x0'
[^:]*:10: Error: expected a single-register list at operand 1 -- `ld1r \{v1.4s,v2.4s,v3.4s\},\[x3\],x4'
[^:]*:11: Error: missing immediate expression at operand 1 -- `svc'
[^:]*:12: Error: operand mismatch -- `add v0.4s,v1.4s,v2.2s'
[^:]*:12: Info:    did you mean this\?
[^:]*:12: Info:    	add v0.4s, v1.4s, v2.4s
[^:]*:12: Info:    other valid variant\(s\):
[^:]*:12: Info:    	add v0.8b, v1.8b, v2.8b
[^:]*:12: Info:    	add v0.16b, v1.16b, v2.16b
[^:]*:12: Info:    	add v0.4h, v1.4h, v2.4h
[^:]*:12: Info:    	add v0.8h, v1.8h, v2.8h
[^:]*:12: Info:    	add v0.2s, v1.2s, v2.2s
[^:]*:12: Info:    	add v0.2d, v1.2d, v2.2d
[^:]*:13: Error: operand mismatch -- `urecpe v0.1d,v7.1d'
[^:]*:13: Info:    did you mean this\?
[^:]*:13: Info:    	urecpe v0.2s, v7.2s
[^:]*:13: Info:    other valid variant\(s\):
[^:]*:13: Info:    	urecpe v0.4s, v7.4s
[^:]*:14: Error: operand mismatch -- `adds w0,wsp,x0,uxtx#1'
[^:]*:14: Info:    did you mean this\?
[^:]*:14: Info:    	adds w0, wsp, w0, uxtx #1
[^:]*:14: Info:    other valid variant\(s\):
[^:]*:14: Info:    	adds x0, sp, w0, uxtx #1
[^:]*:14: Info:    	adds x0, sp, x0, lsl #1
[^:]*:15: Error: operand mismatch -- `fmov d0,s0'
[^:]*:15: Info:    did you mean this\?
[^:]*:15: Info:    	fmov s0, s0
[^:]*:15: Info:    other valid variant\(s\):
[^:]*:15: Info:    	fmov d0, d0
[^:]*:16: Error: operand mismatch -- `ldnp h3,h7,\[sp\],#16'
[^:]*:16: Info:    did you mean this\?
[^:]*:16: Info:    	ldnp s3, s7, \[sp\], #16
[^:]*:16: Info:    other valid variant\(s\):
[^:]*:16: Info:    	ldnp d3, d7, \[sp\], #16
[^:]*:16: Info:    	ldnp q3, q7, \[sp\], #16
[^:]*:18: Error: operand mismatch -- `suqadd v0.8b,v1.16b'
[^:]*:18: Info:    did you mean this\?
[^:]*:18: Info:    	suqadd v0.8b, v1.8b
[^:]*:18: Info:    other valid variant\(s\):
[^:]*:18: Info:    	suqadd v0.16b, v1.16b
[^:]*:18: Info:    	suqadd v0.4h, v1.4h
[^:]*:18: Info:    	suqadd v0.8h, v1.8h
[^:]*:18: Info:    	suqadd v0.2s, v1.2s
[^:]*:18: Info:    	suqadd v0.4s, v1.4s
[^:]*:18: Info:    	suqadd v0.2d, v1.2d
[^:]*:20: Error: operand mismatch -- `ursqrte v2.8b,v3.8b'
[^:]*:20: Info:    did you mean this\?
[^:]*:20: Info:    	ursqrte v2.2s, v3.2s
[^:]*:20: Info:    other valid variant\(s\):
[^:]*:20: Info:    	ursqrte v2.4s, v3.4s
[^:]*:22: Error: operand mismatch -- `rev32 v4.2s,v5.2s'
[^:]*:22: Info:    did you mean this\?
[^:]*:22: Info:    	rev32 v4.8b, v5.8b
[^:]*:22: Info:    other valid variant\(s\):
[^:]*:22: Info:    	rev32 v4.16b, v5.16b
[^:]*:22: Info:    	rev32 v4.4h, v5.4h
[^:]*:22: Info:    	rev32 v4.8h, v5.8h
[^:]*:24: Error: operand mismatch -- `frintn v6.8b,v7.8b'
[^:]*:24: Info:    did you mean this\?
[^:]*:24: Info:    	frintn v6.4h, v7.4h
[^:]*:24: Info:    other valid variant\(s\):
[^:]*:24: Info:    	frintn v6.8h, v7.8h
[^:]*:26: Error: operand mismatch -- `rev64 v8.2d,v9.2d'
[^:]*:26: Info:    did you mean this\?
[^:]*:26: Info:    	rev64 v8.8b, v9.8b
[^:]*:26: Info:    other valid variant\(s\):
[^:]*:26: Info:    	rev64 v8.16b, v9.16b
[^:]*:26: Info:    	rev64 v8.4h, v9.4h
[^:]*:26: Info:    	rev64 v8.8h, v9.8h
[^:]*:26: Info:    	rev64 v8.2s, v9.2s
[^:]*:26: Info:    	rev64 v8.4s, v9.4s
[^:]*:28: Error: operand mismatch -- `rev16 v10.2s,v11.2s'
[^:]*:28: Info:    did you mean this\?
[^:]*:28: Info:    	rev16 v10.8b, v11.8b
[^:]*:28: Info:    other valid variant\(s\):
[^:]*:28: Info:    	rev16 v10.16b, v11.16b
[^:]*:30: Error: operand mismatch -- `saddlp v12.8b,v13.8b'
[^:]*:30: Info:    did you mean this\?
[^:]*:30: Info:    	saddlp v12.4h, v13.8b
[^:]*:30: Info:    other valid variant\(s\):
[^:]*:30: Info:    	saddlp v12.8h, v13.16b
[^:]*:30: Info:    	saddlp v12.2s, v13.4h
[^:]*:30: Info:    	saddlp v12.4s, v13.8h
[^:]*:30: Info:    	saddlp v12.1d, v13.2s
[^:]*:30: Info:    	saddlp v12.2d, v13.4s
[^:]*:32: Error: operand mismatch -- `shll v14.8b,v15.8h,#1'
[^:]*:32: Info:    did you mean this\?
[^:]*:32: Info:    	shll v14.8h, v15.8b, #1
[^:]*:32: Info:    other valid variant\(s\):
[^:]*:32: Info:    	shll v14.4s, v15.4h, #1
[^:]*:32: Info:    	shll v14.2d, v15.2s, #1
[^:]*:34: Error: operand mismatch -- `shll2 v14.8b,v15.8h,#1'
[^:]*:34: Info:    did you mean this\?
[^:]*:34: Info:    	shll2 v14.4s, v15.8h, #1
[^:]*:34: Info:    other valid variant\(s\):
[^:]*:34: Info:    	shll2 v14.8h, v15.16b, #1
[^:]*:34: Info:    	shll2 v14.2d, v15.4s, #1
[^:]*:36: Error: operand mismatch -- `fcvtxn v22.8b,v23.8b'
[^:]*:36: Info:    did you mean this\?
[^:]*:36: Info:    	fcvtxn v22.2s, v23.2d
[^:]*:38: Error: operand mismatch -- `fcvtxn2 v24.8b,v25.8b'
[^:]*:38: Info:    did you mean this\?
[^:]*:38: Info:    	fcvtxn2 v24.4s, v25.2d
[^:]*:40: Error: operand mismatch -- `fcvtn v25.4s,v26.4s'
[^:]*:40: Info:    did you mean this\?
[^:]*:40: Info:    	fcvtn v25.4h, v26.4s
[^:]*:40: Info:    other valid variant\(s\):
[^:]*:40: Info:    	fcvtn v25.2s, v26.2d
[^:]*:42: Error: operand mismatch -- `fcvtn2 v27.4s,v28.4s'
[^:]*:42: Info:    did you mean this\?
[^:]*:42: Info:    	fcvtn2 v27.8h, v28.4s
[^:]*:42: Info:    other valid variant\(s\):
[^:]*:42: Info:    	fcvtn2 v27.4s, v28.2d
[^:]*:44: Error: operand mismatch -- `fcvtl v29.8b,v30.8b'
[^:]*:44: Info:    did you mean this\?
[^:]*:44: Info:    	fcvtl v29.4s, v30.4h
[^:]*:44: Info:    other valid variant\(s\):
[^:]*:44: Info:    	fcvtl v29.2d, v30.2s
[^:]*:46: Error: operand mismatch -- `fcvtl2 v1.2d,v2.2d'
[^:]*:46: Info:    did you mean this\?
[^:]*:46: Info:    	fcvtl2 v1.2d, v2.4s
[^:]*:46: Info:    other valid variant\(s\):
[^:]*:46: Info:    	fcvtl2 v1.4s, v2.8h
[^:]*:48: Error: operand mismatch -- `sqadd v16.8b,v17.8h,v18.8h'
[^:]*:48: Info:    did you mean this\?
[^:]*:48: Info:    	sqadd v16.8h, v17.8h, v18.8h
[^:]*:48: Info:    other valid variant\(s\):
[^:]*:48: Info:    	sqadd v16.8b, v17.8b, v18.8b
[^:]*:48: Info:    	sqadd v16.16b, v17.16b, v18.16b
[^:]*:48: Info:    	sqadd v16.4h, v17.4h, v18.4h
[^:]*:48: Info:    	sqadd v16.2s, v17.2s, v18.2s
[^:]*:48: Info:    	sqadd v16.4s, v17.4s, v18.4s
[^:]*:48: Info:    	sqadd v16.2d, v17.2d, v18.2d
[^:]*:50: Error: operand mismatch -- `shadd v19.8b,v20.8h,v21.8h'
[^:]*:50: Info:    did you mean this\?
[^:]*:50: Info:    	shadd v19.8h, v20.8h, v21.8h
[^:]*:50: Info:    other valid variant\(s\):
[^:]*:50: Info:    	shadd v19.8b, v20.8b, v21.8b
[^:]*:50: Info:    	shadd v19.16b, v20.16b, v21.16b
[^:]*:50: Info:    	shadd v19.4h, v20.4h, v21.4h
[^:]*:50: Info:    	shadd v19.2s, v20.2s, v21.2s
[^:]*:50: Info:    	shadd v19.4s, v20.4s, v21.4s
[^:]*:52: Error: operand mismatch -- `sha1su0 v1.16b,v2.16b,v3.16b'
[^:]*:52: Info:    did you mean this\?
[^:]*:52: Info:    	sha1su0 v1.4s, v2.4s, v3.4s
[^:]*:54: Error: operand mismatch -- `shadd v1.2d,v2.2d,v3.2d'
[^:]*:54: Info:    did you mean this\?
[^:]*:54: Info:    	shadd v1.8b, v2.8b, v3.8b
[^:]*:54: Info:    other valid variant\(s\):
[^:]*:54: Info:    	shadd v1.16b, v2.16b, v3.16b
[^:]*:54: Info:    	shadd v1.4h, v2.4h, v3.4h
[^:]*:54: Info:    	shadd v1.8h, v2.8h, v3.8h
[^:]*:54: Info:    	shadd v1.2s, v2.2s, v3.2s
[^:]*:54: Info:    	shadd v1.4s, v2.4s, v3.4s
[^:]*:56: Error: operand mismatch -- `sqdmulh v1.16b,v2.16b,v3.16b'
[^:]*:56: Info:    did you mean this\?
[^:]*:56: Info:    	sqdmulh v1.4h, v2.4h, v3.4h
[^:]*:56: Info:    other valid variant\(s\):
[^:]*:56: Info:    	sqdmulh v1.8h, v2.8h, v3.8h
[^:]*:56: Info:    	sqdmulh v1.2s, v2.2s, v3.2s
[^:]*:56: Info:    	sqdmulh v1.4s, v2.4s, v3.4s
[^:]*:58: Error: operand mismatch -- `sqdmlal2 v1.16b,v2.16b,v3.16b'
[^:]*:58: Info:    did you mean this\?
[^:]*:58: Info:    	sqdmlal2 v1.4s, v2.8h, v3.8h
[^:]*:58: Info:    other valid variant\(s\):
[^:]*:58: Info:    	sqdmlal2 v1.2d, v2.4s, v3.4s
