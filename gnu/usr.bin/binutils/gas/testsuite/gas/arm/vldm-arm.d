# name: VFP VLDM and VSTM, ARM mode
# as: -mfpu=vfp3 
# source: vldm.s
# objdump: -dr --prefix-addresses --show-raw-insn
# This test is only valid on ELF based ports.
#notarget: *-*-pe *-*-wince

.*: +file format .*arm.*

Disassembly of section .text:
0[0-9a-f]+ <[^>]+> ec9f0b04 	vldmia	pc, {d0-d1}
0[0-9a-f]+ <[^>]+> ea000003 	b	00000018 <bar>
0[0-9a-f]+ <[^>]+> 00000000 	.word	0x00000000
0[0-9a-f]+ <[^>]+> 3ff00000 	.word	0x3ff00000
0[0-9a-f]+ <[^>]+> 9999999a 	.word	0x9999999a
0[0-9a-f]+ <[^>]+> 3ff19999 	.word	0x3ff19999
0[0-9a-f]+ <[^>]+> ec8f0b04 	vstmia	pc, {d0-d1}
0[0-9a-f]+ <[^>]+> ea000003 	b	00000030 <foo2>
0[0-9a-f]+ <[^>]+> 00000000 	.word	0x00000000
0[0-9a-f]+ <[^>]+> 3ff00000 	.word	0x3ff00000
0[0-9a-f]+ <[^>]+> 9999999a 	.word	0x9999999a
0[0-9a-f]+ <[^>]+> 3ff19999 	.word	0x3ff19999
0[0-9a-f]+ <[^>]+> e1a00000 	nop.*
0[0-9a-f]+ <[^>]+> e1a00000 	nop.*
