#source: copro-arm_v5plus-thumb_v6t2plus.s
#objdump: -dr --prefix-addresses --show-raw-insn
#name: ARMv5 ARM CoProcessor Instructions
#as: -march=armv5 -EL

# Test the standard ARM co-processor instructions:

.*: +file format .*arm.*

Disassembly of section .text:
0+000 <[^>]*> fe421103 	cdp2	1, 4, cr1, cr2, cr3, \{0\}
0+004 <[^>]*> fd939500 	ldc2	5, cr9, \[r3\]
0+008 <[^>]*> fdd1e108 	ldc2l	1, cr14, \[r1, #32\]
0+00c <[^>]*> fd1f8001 	ldc2	0, cr8, \[pc, #-4\]	@ .* <foo>
0+010 <[^>]*> fd830500 	stc2	5, cr0, \[r3\]
0+014 <[^>]*> fdc0f302 	stc2l	3, cr15, \[r0, #8\]
0+018 <[^>]*> fd0f7101 	stc2	1, cr7, \[pc, #-4\]	@ .* <bar>
0+01c <[^>]*> fe715212 	mrc2	2, 3, r5, cr1, cr2, \{0\}
0+020 <[^>]*> fe215711 	mcr2	7, 1, r5, cr1, cr1, \{0\}
0+024 <[^>]*> fc925502 	ldc2	5, cr5, \[r2\], \{2\}
0+028 <[^>]*> fc834603 	stc2	6, cr4, \[r3\], \{3\}
0+02c <[^>]*> fcd61c06 	ldc2l	12, cr1, \[r6\], \{6\}
0+030 <[^>]*> fcc70c07 	stc2l	12, cr0, \[r7\], \{7\}
