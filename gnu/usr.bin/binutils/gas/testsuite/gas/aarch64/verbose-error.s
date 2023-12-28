// verbose-error.s Test file for -mverbose-error

.text
	strb	w7, [x30, x0, lsl]
	ubfm	w0, x1, 8, 31
	bfm	w0, w1, 8, 43
	strb	w7, [x30, x0, lsl #1]
	st2	{v4.2d,v5.2d},[x3,#3]
	fmov	v1.D[0],x0
	ld1r	{v1.4s, v2.4s, v3.4s}, [x3], x4
	svc
	add	v0.4s, v1.4s, v2.2s
	urecpe	v0.1d,v7.1d
	adds	w0, wsp, x0, uxtx #1
	fmov	d0, s0
	ldnp	h3, h7, [sp], #16
	# QL_V2SAME
	suqadd v0.8b, v1.16b
	# QL_V2SAME
	ursqrte v2.8b, v3.8b
	# QL_V2SAMEBH
	rev32 v4.2s, v5.2s
	#QL_V2SAMESD
	frintn v6.8b, v7.8b
	#QL_V2SAMEBHS
	rev64 v8.2d, v9.2d
	#QL_V2SAMEB
	rev16 v10.2s, v11.2s
	#QL_V2PAIRWISELONGBHS
	saddlp v12.8b, v13.8b
	#QL_V2LONGBHS
	shll v14.8b, v15.8h, #1
	#QL_V2LONGBHS2
	shll2 v14.8b, v15.8h, #1
	#QL_V2NARRS
	fcvtxn v22.8b, v23.8b
	#QL_V2NARRS2
	fcvtxn2 v24.8b, v25.8b
	#QL_V2NARRHS
	fcvtn v25.4s, v26.4s
	#QL_V2NARRHS2
	fcvtn2 v27.4s, v28.4s
	#QL_V2LONGHS
	fcvtl v29.8b, v30.8b
	#QL_V2LONGHS2
	fcvtl2 v1.2d, v2.2d
	#QL_V3SAME
	sqadd v16.8b, v17.8h, v18.8h
	#QL_V3SAMEBHS
	shadd v19.8b, v20.8h, v21.8h
	#QL_V3SAME4S
	sha1su0 v1.16b, v2.16b, v3.16b
	#QL_V3SAMEEB
	shadd v1.2d, v2.2d, v3.2d
	#QL_V3SAMEEHS
	sqdmulh v1.16b, v2.16b, v3.16b
	#QL_V3LONGHS2
	sqdmlal2 v1.16b, v2.16b, v3.16b
