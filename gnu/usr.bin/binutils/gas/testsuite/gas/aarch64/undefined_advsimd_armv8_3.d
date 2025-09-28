#as: -march=armv8.3-a
#objdump: -dr

.*:     file format .*

Disassembly of section \.text:

0+ <.*>:
[^:]+:\s+6ec3c441 	fcmla	v1.2d, v2.2d, v3.2d, #0
[^:]+:\s+6e03c441 	.inst	0x6e03c441 ; undefined
[^:]+:\s+2ec3c441 	.inst	0x2ec3c441 ; undefined
[^:]+:\s+2e83c441 	fcmla	v1.2s, v2.2s, v3.2s, #0
[^:]+:\s+2e03c441 	.inst	0x2e03c441 ; undefined
[^:]+:\s+2ec3c441 	.inst	0x2ec3c441 ; undefined
[^:]+:\s+6e83c441 	fcmla	v1.4s, v2.4s, v3.4s, #0
[^:]+:\s+6e03c441 	.inst	0x6e03c441 ; undefined
[^:]+:\s+2ec3c441 	.inst	0x2ec3c441 ; undefined
[^:]+:\s+2e43c441 	fcmla	v1.4h, v2.4h, v3.4h, #0
[^:]+:\s+2e03c441 	.inst	0x2e03c441 ; undefined
[^:]+:\s+2ec3c441 	.inst	0x2ec3c441 ; undefined
[^:]+:\s+6e43c441 	fcmla	v1.8h, v2.8h, v3.8h, #0
[^:]+:\s+6e03c441 	.inst	0x6e03c441 ; undefined
[^:]+:\s+2ec3c441 	.inst	0x2ec3c441 ; undefined
[^:]+:\s+6f831041 	fcmla	v1.4s, v2.4s, v3.s\[0\], #0
[^:]+:\s+6f031041 	.inst	0x6f031041 ; undefined
[^:]+:\s+6fc31041 	.inst	0x6fc31041 ; undefined
[^:]+:\s+2f431841 	.inst	0x2f431841 ; undefined
[^:]+:\s+6fa31041 	.inst	0x6fa31041 ; undefined
[^:]+:\s+2f831041 	.inst	0x2f831041 ; undefined
[^:]+:\s+2f431041 	fcmla	v1.4h, v2.4h, v3.h\[0\], #0
[^:]+:\s+2f031041 	.inst	0x2f031041 ; undefined
[^:]+:\s+2fc31041 	.inst	0x2fc31041 ; undefined
[^:]+:\s+2f431841 	.inst	0x2f431841 ; undefined
[^:]+:\s+2fa31041 	.inst	0x2fa31041 ; undefined
[^:]+:\s+2f831041 	.inst	0x2f831041 ; undefined
[^:]+:\s+6f431041 	fcmla	v1.8h, v2.8h, v3.h\[0\], #0
[^:]+:\s+6f031041 	.inst	0x6f031041 ; undefined
[^:]+:\s+6fc31041 	.inst	0x6fc31041 ; undefined
[^:]+:\s+2f431841 	.inst	0x2f431841 ; undefined
[^:]+:\s+6fa31041 	.inst	0x6fa31041 ; undefined
[^:]+:\s+2f831041 	.inst	0x2f831041 ; undefined
[^:]+:\s+6ec3e441 	fcadd	v1.2d, v2.2d, v3.2d, #90
[^:]+:\s+6e03e441 	.inst	0x6e03e441 ; undefined
[^:]+:\s+2ec3e441 	.inst	0x2ec3e441 ; undefined
[^:]+:\s+2e83e441 	fcadd	v1.2s, v2.2s, v3.2s, #90
[^:]+:\s+2e03e441 	.inst	0x2e03e441 ; undefined
[^:]+:\s+2ec3e441 	.inst	0x2ec3e441 ; undefined
[^:]+:\s+6e83e441 	fcadd	v1.4s, v2.4s, v3.4s, #90
[^:]+:\s+6e03e441 	.inst	0x6e03e441 ; undefined
[^:]+:\s+2ec3e441 	.inst	0x2ec3e441 ; undefined
[^:]+:\s+2e43e441 	fcadd	v1.4h, v2.4h, v3.4h, #90
[^:]+:\s+2e03e441 	.inst	0x2e03e441 ; undefined
[^:]+:\s+2ec3e441 	.inst	0x2ec3e441 ; undefined
[^:]+:\s+6e43e441 	fcadd	v1.8h, v2.8h, v3.8h, #90
[^:]+:\s+6e03e441 	.inst	0x6e03e441 ; undefined
[^:]+:\s+2ec3e441 	.inst	0x2ec3e441 ; undefined
