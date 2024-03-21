#as: -march=armv8.4-a
#objdump: -dr

.*:     file format .*

Disassembly of section \.text:

0+ <.*>:
[^:]+:\s+5f909000 	fmul	s0, s0, v16.s\[0\]
[^:]+:\s+5ff09000 	.inst	0x5ff09000 ; undefined
[^:]+:\s+5f901000 	fmla	s0, s0, v16.s\[0\]
[^:]+:\s+5ff01000 	.inst	0x5ff01000 ; undefined
[^:]+:\s+5f905000 	fmls	s0, s0, v16.s\[0\]
[^:]+:\s+5ff05000 	.inst	0x5ff05000 ; undefined
[^:]+:\s+7f909000 	fmulx	s0, s0, v16.s\[0\]
[^:]+:\s+7ff09000 	.inst	0x7ff09000 ; undefined
[^:]+:\s+5fd09000 	fmul	d0, d0, v16.d\[0\]
[^:]+:\s+5ff09000 	.inst	0x5ff09000 ; undefined
[^:]+:\s+5fd01000 	fmla	d0, d0, v16.d\[0\]
[^:]+:\s+5ff01000 	.inst	0x5ff01000 ; undefined
[^:]+:\s+5fd05000 	fmls	d0, d0, v16.d\[0\]
[^:]+:\s+5ff05000 	.inst	0x5ff05000 ; undefined
[^:]+:\s+7fd09000 	fmulx	d0, d0, v16.d\[0\]
[^:]+:\s+7ff09000 	.inst	0x7ff09000 ; undefined
[^:]+:\s+4f909000 	fmul	v0.4s, v0.4s, v16.s\[0\]
[^:]+:\s+4ff09000 	.inst	0x4ff09000 ; undefined
[^:]+:\s+4f901000 	fmla	v0.4s, v0.4s, v16.s\[0\]
[^:]+:\s+4ff01000 	.inst	0x4ff01000 ; undefined
[^:]+:\s+4f905000 	fmls	v0.4s, v0.4s, v16.s\[0\]
[^:]+:\s+4ff05000 	.inst	0x4ff05000 ; undefined
[^:]+:\s+6f909000 	fmulx	v0.4s, v0.4s, v16.s\[0\]
[^:]+:\s+6ff09000 	.inst	0x6ff09000 ; undefined
[^:]+:\s+4fd09000 	fmul	v0.2d, v0.2d, v16.d\[0\]
[^:]+:\s+4ff09000 	.inst	0x4ff09000 ; undefined
[^:]+:\s+4fd01000 	fmla	v0.2d, v0.2d, v16.d\[0\]
[^:]+:\s+4ff01000 	.inst	0x4ff01000 ; undefined
[^:]+:\s+4fd05000 	fmls	v0.2d, v0.2d, v16.d\[0\]
[^:]+:\s+4ff05000 	.inst	0x4ff05000 ; undefined
[^:]+:\s+6fd09000 	fmulx	v0.2d, v0.2d, v16.d\[0\]
[^:]+:\s+6ff09000 	.inst	0x6ff09000 ; undefined
