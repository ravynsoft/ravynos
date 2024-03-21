	.syntax unified
	.arch armv8-a
	.arch_extension crypto

	.arm
	vmull.p64	q0, d0, d0
	vmull.p64	q15, d31, d31
	aese.8	q0, q0
	aese.8	q7, q7
	aese.8	q8, q8
	aese.8	q15, q15
	aesd.8	q0, q0
	aesd.8	q7, q7
	aesd.8	q8, q8
	aesd.8	q15, q15
	aesmc.8	q0, q0
	aesmc.8	q7, q7
	aesmc.8	q8, q8
	aesmc.8	q15, q15
	aesimc.8	q0, q0
	aesimc.8	q7, q7
	aesimc.8	q8, q8
	aesimc.8	q15, q15
	sha1c.32	q0, q0, q0
	sha1c.32	q7, q7, q7
	sha1c.32	q8, q8, q8
	sha1c.32	q15, q15, q15
	sha1p.32	q0, q0, q0
	sha1p.32	q7, q7, q7
	sha1p.32	q8, q8, q8
	sha1p.32	q15, q15, q15
	sha1m.32	q0, q0, q0
	sha1m.32	q7, q7, q7
	sha1m.32	q8, q8, q8
	sha1m.32	q15, q15, q15
	sha1su0.32	q0, q0, q0
	sha1su0.32	q7, q7, q7
	sha1su0.32	q8, q8, q8
	sha1su0.32	q15, q15, q15
	sha256h.32	q0, q0, q0
	sha256h.32	q7, q7, q7
	sha256h.32	q8, q8, q8
	sha256h.32	q15, q15, q15
	sha256h2.32	q0, q0, q0
	sha256h2.32	q7, q7, q7
	sha256h2.32	q8, q8, q8
	sha256h2.32	q15, q15, q15
	sha256su1.32	q0, q0, q0
	sha256su1.32	q7, q7, q7
	sha256su1.32	q8, q8, q8
	sha256su1.32	q15, q15, q15
	sha1h.32	q0, q0
	sha1h.32	q7, q7
	sha1h.32	q8, q8
	sha1h.32	q15, q15
	sha1su1.32	q0, q0
	sha1su1.32	q7, q7
	sha1su1.32	q8, q8
	sha1su1.32	q15, q15
	sha256su0.32	q0, q0
	sha256su0.32	q7, q7
	sha256su0.32	q8, q8
	sha256su0.32	q15, q15

	.thumb
	vmull.p64	q0, d0, d0
	vmull.p64	q15, d31, d31
	aese.8	q0, q0
	aese.8	q7, q7
	aese.8	q8, q8
	aese.8	q15, q15
	aesd.8	q0, q0
	aesd.8	q7, q7
	aesd.8	q8, q8
	aesd.8	q15, q15
	aesmc.8	q0, q0
	aesmc.8	q7, q7
	aesmc.8	q8, q8
	aesmc.8	q15, q15
	aesimc.8	q0, q0
	aesimc.8	q7, q7
	aesimc.8	q8, q8
	aesimc.8	q15, q15
	sha1c.32	q0, q0, q0
	sha1c.32	q7, q7, q7
	sha1c.32	q8, q8, q8
	sha1c.32	q15, q15, q15
	sha1p.32	q0, q0, q0
	sha1p.32	q7, q7, q7
	sha1p.32	q8, q8, q8
	sha1p.32	q15, q15, q15
	sha1m.32	q0, q0, q0
	sha1m.32	q7, q7, q7
	sha1m.32	q8, q8, q8
	sha1m.32	q15, q15, q15
	sha1su0.32	q0, q0, q0
	sha1su0.32	q7, q7, q7
	sha1su0.32	q8, q8, q8
	sha1su0.32	q15, q15, q15
	sha256h.32	q0, q0, q0
	sha256h.32	q7, q7, q7
	sha256h.32	q8, q8, q8
	sha256h.32	q15, q15, q15
	sha256h2.32	q0, q0, q0
	sha256h2.32	q7, q7, q7
	sha256h2.32	q8, q8, q8
	sha256h2.32	q15, q15, q15
	sha256su1.32	q0, q0, q0
	sha256su1.32	q7, q7, q7
	sha256su1.32	q8, q8, q8
	sha256su1.32	q15, q15, q15
	sha1h.32	q0, q0
	sha1h.32	q7, q7
	sha1h.32	q8, q8
	sha1h.32	q15, q15
	sha1su1.32	q0, q0
	sha1su1.32	q7, q7
	sha1su1.32	q8, q8
	sha1su1.32	q15, q15
	sha256su0.32	q0, q0
	sha256su0.32	q7, q7
	sha256su0.32	q8, q8
	sha256su0.32	q15, q15
