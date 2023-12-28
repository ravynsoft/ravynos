#objdump: -dr --prefix-addresses --show-raw-insn
#skip: *-*-pe *-*-wince
#name: MOVW/MOVT relocations against local symbols

.*: +file format .*arm.*

Disassembly of section .text:
0[0-9a-f]+ <[^>]+> e3000000 	movw	r0, #0
			0: R_ARM_MOVW_ABS_NC	bar
0[0-9a-f]+ <[^>]+> e3400000 	movt	r0, #0
			4: R_ARM_MOVT_ABS	bar
0[0-9a-f]+ <[^>]+> f240 0000 	movw	r0, #0
			8: R_ARM_THM_MOVW_ABS_NC	bar
0[0-9a-f]+ <[^>]+> f2c0 0000 	movt	r0, #0
			c: R_ARM_THM_MOVT_ABS	bar
#...
