#source: copro-arm_v5teplus-thumb_v6t2plus.s
#objdump: -dr --prefix-addresses --show-raw-insn
#name: ARMv6T2 Thumb CoProcessor Instructions (3)
#as: -march=armv6t2 -mthumb -EL

# Test the standard ARM co-processor instructions:

.*: +file format .*arm.*

Disassembly of section .text:
0+000 <[^>]*> ec50 7d04 	mrrc	13, 0, r7, r0, cr4
0+004 <[^>]*> ec40 7e05 	mcrr	14, 0, r7, r0, cr5
0+008 <[^>]*> ec50 7fff 	mrrc	15, 15, r7, r0, cr15
0+00c <[^>]*> ec40 7efe 	mcrr	14, 15, r7, r0, cr14
