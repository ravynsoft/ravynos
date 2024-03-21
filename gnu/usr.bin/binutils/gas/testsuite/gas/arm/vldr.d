# name: VFP VLDR
# as: -mfpu=vfp3 -mcpu=cortex-a8 -mthumb
# source: vldr.s
# objdump: -dr --prefix-addresses --show-raw-insn
# skip: *-*-pe *-*-wince

.*: +file format .*arm.*

Disassembly of section .text:
0[0-9a-f]+ <[^>]+> bf00      	nop
0[0-9a-f]+ <[^>]+> ed9f 0b03 	vldr	d0, \[pc, #12\]	@ 00000010 <float>
0[0-9a-f]+ <[^>]+> ed9f 0b02 	vldr	d0, \[pc, #8\]	@ 00000010 <float>
0[0-9a-f]+ <[^>]+> bf00      	nop
0[0-9a-f]+ <[^>]+> bf00      	nop
0[0-9a-f]+ <[^>]+> bf00      	nop
	...
