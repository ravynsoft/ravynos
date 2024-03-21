#as: -march=armv8.8-a
#objdump: -dr

.*:     file format .*

Disassembly of section \.text:

0+ <.*>:
[^:]+:\s+d5184300 	msr	allint, x0
[^:]+:\s+d518430f 	msr	allint, x15
[^:]+:\s+d518431e 	msr	allint, x30
[^:]+:\s+d518431f 	msr	allint, xzr
[^:]+:\s+d5384300 	mrs	x0, allint
[^:]+:\s+d5384310 	mrs	x16, allint
[^:]+:\s+d538431e 	mrs	x30, allint
[^:]+:\s+d501401f 	msr	allint, #0x0
[^:]+:\s+d501411f 	msr	allint, #0x1
[^:]+:\s+d501421f 	msr	s0_1_c4_c2_0, xzr
[^:]+:\s+d538c9a0 	mrs	x0, icc_nmiar1_el1
