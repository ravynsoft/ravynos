#source: copro-arm_v5plus-thumb_v6t2plus.s
#objdump: -dr --prefix-addresses --show-raw-insn --disassembler-options=force-thumb
#name: ARMv6T2 Thumb CoProcessor Instructions (2)
#as: -march=armv6t2 -mthumb -EL

# Test the standard ARM co-processor instructions:

.*: +file format .*arm.*

Disassembly of section .text:
0+000 <[^>]*> fe42 1103 	cdp2	1, 4, cr1, cr2, cr3, \{0\}
0+004 <[^>]*> fd93 9500 	ldc2	5, cr9, \[r3\]
0+008 <[^>]*> fdd1 e108 	ldc2l	1, cr14, \[r1, #32\]
0+00c <[^>]*> fd9f 8000 	ldc2	0, cr8, \[pc\]	@ .* <foo>
0+010 <[^>]*> fd83 0500 	stc2	5, cr0, \[r3\]
0+014 <[^>]*> fdc0 f302 	stc2l	3, cr15, \[r0, #8\]
0+018 <[^>]*> fd8f 7100 	stc2	1, cr7, \[pc\]	@ .* <bar>
0+01c <[^>]*> fe71 5212 	mrc2	2, 3, r5, cr1, cr2, \{0\}
0+020 <[^>]*> fe21 5711 	mcr2	7, 1, r5, cr1, cr1, \{0\}
0+024 <[^>]*> fc92 5502 	ldc2	5, cr5, \[r2\], \{2\}
0+028 <[^>]*> fc83 4603 	stc2	6, cr4, \[r3\], \{3\}
0+02c <[^>]*> fcd6 1c06 	ldc2l	12, cr1, \[r6\], \{6\}
0+030 <[^>]*> fcc7 0c07 	stc2l	12, cr0, \[r7\], \{7\}
