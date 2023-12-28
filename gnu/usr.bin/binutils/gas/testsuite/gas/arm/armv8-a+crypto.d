#name: Valid v8-a+cryptov1
#objdump: -dr --prefix-addresses --show-raw-insn
#skip: *-*-pe *-*-wince

.*: +file format .*arm.*

Disassembly of section .text:
0[0-9a-f]+ <[^>]+> f2a00e00 	vmull.p64	q0, d0, d0
0[0-9a-f]+ <[^>]+> f2efeeaf 	vmull.p64	q15, d31, d31
0[0-9a-f]+ <[^>]+> f3b00300 	aese.8	q0, q0
0[0-9a-f]+ <[^>]+> f3b0e30e 	aese.8	q7, q7
0[0-9a-f]+ <[^>]+> f3f00320 	aese.8	q8, q8
0[0-9a-f]+ <[^>]+> f3f0e32e 	aese.8	q15, q15
0[0-9a-f]+ <[^>]+> f3b00340 	aesd.8	q0, q0
0[0-9a-f]+ <[^>]+> f3b0e34e 	aesd.8	q7, q7
0[0-9a-f]+ <[^>]+> f3f00360 	aesd.8	q8, q8
0[0-9a-f]+ <[^>]+> f3f0e36e 	aesd.8	q15, q15
0[0-9a-f]+ <[^>]+> f3b00380 	aesmc.8	q0, q0
0[0-9a-f]+ <[^>]+> f3b0e38e 	aesmc.8	q7, q7
0[0-9a-f]+ <[^>]+> f3f003a0 	aesmc.8	q8, q8
0[0-9a-f]+ <[^>]+> f3f0e3ae 	aesmc.8	q15, q15
0[0-9a-f]+ <[^>]+> f3b003c0 	aesimc.8	q0, q0
0[0-9a-f]+ <[^>]+> f3b0e3ce 	aesimc.8	q7, q7
0[0-9a-f]+ <[^>]+> f3f003e0 	aesimc.8	q8, q8
0[0-9a-f]+ <[^>]+> f3f0e3ee 	aesimc.8	q15, q15
0[0-9a-f]+ <[^>]+> f2000c40 	sha1c.32	q0, q0, q0
0[0-9a-f]+ <[^>]+> f20eec4e 	sha1c.32	q7, q7, q7
0[0-9a-f]+ <[^>]+> f2400ce0 	sha1c.32	q8, q8, q8
0[0-9a-f]+ <[^>]+> f24eecee 	sha1c.32	q15, q15, q15
0[0-9a-f]+ <[^>]+> f2100c40 	sha1p.32	q0, q0, q0
0[0-9a-f]+ <[^>]+> f21eec4e 	sha1p.32	q7, q7, q7
0[0-9a-f]+ <[^>]+> f2500ce0 	sha1p.32	q8, q8, q8
0[0-9a-f]+ <[^>]+> f25eecee 	sha1p.32	q15, q15, q15
0[0-9a-f]+ <[^>]+> f2200c40 	sha1m.32	q0, q0, q0
0[0-9a-f]+ <[^>]+> f22eec4e 	sha1m.32	q7, q7, q7
0[0-9a-f]+ <[^>]+> f2600ce0 	sha1m.32	q8, q8, q8
0[0-9a-f]+ <[^>]+> f26eecee 	sha1m.32	q15, q15, q15
0[0-9a-f]+ <[^>]+> f2300c40 	sha1su0.32	q0, q0, q0
0[0-9a-f]+ <[^>]+> f23eec4e 	sha1su0.32	q7, q7, q7
0[0-9a-f]+ <[^>]+> f2700ce0 	sha1su0.32	q8, q8, q8
0[0-9a-f]+ <[^>]+> f27eecee 	sha1su0.32	q15, q15, q15
0[0-9a-f]+ <[^>]+> f3000c40 	sha256h.32	q0, q0, q0
0[0-9a-f]+ <[^>]+> f30eec4e 	sha256h.32	q7, q7, q7
0[0-9a-f]+ <[^>]+> f3400ce0 	sha256h.32	q8, q8, q8
0[0-9a-f]+ <[^>]+> f34eecee 	sha256h.32	q15, q15, q15
0[0-9a-f]+ <[^>]+> f3100c40 	sha256h2.32	q0, q0, q0
0[0-9a-f]+ <[^>]+> f31eec4e 	sha256h2.32	q7, q7, q7
0[0-9a-f]+ <[^>]+> f3500ce0 	sha256h2.32	q8, q8, q8
0[0-9a-f]+ <[^>]+> f35eecee 	sha256h2.32	q15, q15, q15
0[0-9a-f]+ <[^>]+> f3200c40 	sha256su1.32	q0, q0, q0
0[0-9a-f]+ <[^>]+> f32eec4e 	sha256su1.32	q7, q7, q7
0[0-9a-f]+ <[^>]+> f3600ce0 	sha256su1.32	q8, q8, q8
0[0-9a-f]+ <[^>]+> f36eecee 	sha256su1.32	q15, q15, q15
0[0-9a-f]+ <[^>]+> f3b902c0 	sha1h.32	q0, q0
0[0-9a-f]+ <[^>]+> f3b9e2ce 	sha1h.32	q7, q7
0[0-9a-f]+ <[^>]+> f3f902e0 	sha1h.32	q8, q8
0[0-9a-f]+ <[^>]+> f3f9e2ee 	sha1h.32	q15, q15
0[0-9a-f]+ <[^>]+> f3ba0380 	sha1su1.32	q0, q0
0[0-9a-f]+ <[^>]+> f3bae38e 	sha1su1.32	q7, q7
0[0-9a-f]+ <[^>]+> f3fa03a0 	sha1su1.32	q8, q8
0[0-9a-f]+ <[^>]+> f3fae3ae 	sha1su1.32	q15, q15
0[0-9a-f]+ <[^>]+> f3ba03c0 	sha256su0.32	q0, q0
0[0-9a-f]+ <[^>]+> f3bae3ce 	sha256su0.32	q7, q7
0[0-9a-f]+ <[^>]+> f3fa03e0 	sha256su0.32	q8, q8
0[0-9a-f]+ <[^>]+> f3fae3ee 	sha256su0.32	q15, q15
0[0-9a-f]+ <[^>]+> efa0 0e00 	vmull.p64	q0, d0, d0
0[0-9a-f]+ <[^>]+> efef eeaf 	vmull.p64	q15, d31, d31
0[0-9a-f]+ <[^>]+> ffb0 0300 	aese.8	q0, q0
0[0-9a-f]+ <[^>]+> ffb0 e30e 	aese.8	q7, q7
0[0-9a-f]+ <[^>]+> fff0 0320 	aese.8	q8, q8
0[0-9a-f]+ <[^>]+> fff0 e32e 	aese.8	q15, q15
0[0-9a-f]+ <[^>]+> ffb0 0340 	aesd.8	q0, q0
0[0-9a-f]+ <[^>]+> ffb0 e34e 	aesd.8	q7, q7
0[0-9a-f]+ <[^>]+> fff0 0360 	aesd.8	q8, q8
0[0-9a-f]+ <[^>]+> fff0 e36e 	aesd.8	q15, q15
0[0-9a-f]+ <[^>]+> ffb0 0380 	aesmc.8	q0, q0
0[0-9a-f]+ <[^>]+> ffb0 e38e 	aesmc.8	q7, q7
0[0-9a-f]+ <[^>]+> fff0 03a0 	aesmc.8	q8, q8
0[0-9a-f]+ <[^>]+> fff0 e3ae 	aesmc.8	q15, q15
0[0-9a-f]+ <[^>]+> ffb0 03c0 	aesimc.8	q0, q0
0[0-9a-f]+ <[^>]+> ffb0 e3ce 	aesimc.8	q7, q7
0[0-9a-f]+ <[^>]+> fff0 03e0 	aesimc.8	q8, q8
0[0-9a-f]+ <[^>]+> fff0 e3ee 	aesimc.8	q15, q15
0[0-9a-f]+ <[^>]+> ef00 0c40 	sha1c.32	q0, q0, q0
0[0-9a-f]+ <[^>]+> ef0e ec4e 	sha1c.32	q7, q7, q7
0[0-9a-f]+ <[^>]+> ef40 0ce0 	sha1c.32	q8, q8, q8
0[0-9a-f]+ <[^>]+> ef4e ecee 	sha1c.32	q15, q15, q15
0[0-9a-f]+ <[^>]+> ef10 0c40 	sha1p.32	q0, q0, q0
0[0-9a-f]+ <[^>]+> ef1e ec4e 	sha1p.32	q7, q7, q7
0[0-9a-f]+ <[^>]+> ef50 0ce0 	sha1p.32	q8, q8, q8
0[0-9a-f]+ <[^>]+> ef5e ecee 	sha1p.32	q15, q15, q15
0[0-9a-f]+ <[^>]+> ef20 0c40 	sha1m.32	q0, q0, q0
0[0-9a-f]+ <[^>]+> ef2e ec4e 	sha1m.32	q7, q7, q7
0[0-9a-f]+ <[^>]+> ef60 0ce0 	sha1m.32	q8, q8, q8
0[0-9a-f]+ <[^>]+> ef6e ecee 	sha1m.32	q15, q15, q15
0[0-9a-f]+ <[^>]+> ef30 0c40 	sha1su0.32	q0, q0, q0
0[0-9a-f]+ <[^>]+> ef3e ec4e 	sha1su0.32	q7, q7, q7
0[0-9a-f]+ <[^>]+> ef70 0ce0 	sha1su0.32	q8, q8, q8
0[0-9a-f]+ <[^>]+> ef7e ecee 	sha1su0.32	q15, q15, q15
0[0-9a-f]+ <[^>]+> ff00 0c40 	sha256h.32	q0, q0, q0
0[0-9a-f]+ <[^>]+> ff0e ec4e 	sha256h.32	q7, q7, q7
0[0-9a-f]+ <[^>]+> ff40 0ce0 	sha256h.32	q8, q8, q8
0[0-9a-f]+ <[^>]+> ff4e ecee 	sha256h.32	q15, q15, q15
0[0-9a-f]+ <[^>]+> ff10 0c40 	sha256h2.32	q0, q0, q0
0[0-9a-f]+ <[^>]+> ff1e ec4e 	sha256h2.32	q7, q7, q7
0[0-9a-f]+ <[^>]+> ff50 0ce0 	sha256h2.32	q8, q8, q8
0[0-9a-f]+ <[^>]+> ff5e ecee 	sha256h2.32	q15, q15, q15
0[0-9a-f]+ <[^>]+> ff20 0c40 	sha256su1.32	q0, q0, q0
0[0-9a-f]+ <[^>]+> ff2e ec4e 	sha256su1.32	q7, q7, q7
0[0-9a-f]+ <[^>]+> ff60 0ce0 	sha256su1.32	q8, q8, q8
0[0-9a-f]+ <[^>]+> ff6e ecee 	sha256su1.32	q15, q15, q15
0[0-9a-f]+ <[^>]+> ffb9 02c0 	sha1h.32	q0, q0
0[0-9a-f]+ <[^>]+> ffb9 e2ce 	sha1h.32	q7, q7
0[0-9a-f]+ <[^>]+> fff9 02e0 	sha1h.32	q8, q8
0[0-9a-f]+ <[^>]+> fff9 e2ee 	sha1h.32	q15, q15
0[0-9a-f]+ <[^>]+> ffba 0380 	sha1su1.32	q0, q0
0[0-9a-f]+ <[^>]+> ffba e38e 	sha1su1.32	q7, q7
0[0-9a-f]+ <[^>]+> fffa 03a0 	sha1su1.32	q8, q8
0[0-9a-f]+ <[^>]+> fffa e3ae 	sha1su1.32	q15, q15
0[0-9a-f]+ <[^>]+> ffba 03c0 	sha256su0.32	q0, q0
0[0-9a-f]+ <[^>]+> ffba e3ce 	sha256su0.32	q7, q7
0[0-9a-f]+ <[^>]+> fffa 03e0 	sha256su0.32	q8, q8
0[0-9a-f]+ <[^>]+> fffa e3ee 	sha256su0.32	q15, q15
