#source: copro-arm_v5teplus-thumb_v6t2plus.s
#objdump: -dr --prefix-addresses --show-raw-insn
#name: ARMv5TE ARM CoProcessor Instructions
#as: -march=armv5te -EL

# Test the standard ARM co-processor instructions:

.*: +file format .*arm.*

Disassembly of section .text:
0+000 <[^>]*> ec507d04 	mrrc	13, 0, r7, r0, cr4
0+004 <[^>]*> ec407e05 	mcrr	14, 0, r7, r0, cr5
0+008 <[^>]*> ec507fff 	mrrc	15, 15, r7, r0, cr15
0+00c <[^>]*> ec407efe 	mcrr	14, 15, r7, r0, cr14
