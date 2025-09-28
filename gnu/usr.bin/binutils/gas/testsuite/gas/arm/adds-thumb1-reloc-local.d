#objdump: -dr --prefix-addresses --show-raw-insn
#skip: *-*-pe *-*-wince
#name: ADDS relocations against local symbols for armv6s-m

.*: +file format .*arm.*

Disassembly of section .text:
0[0-9a-f]+ <[^>]+> 3000      	adds	r0, #0
			0: R_ARM_THM_ALU_ABS_G3_NC	bar
0[0-9a-f]+ <[^>]+> 3000      	adds	r0, #0
			2: R_ARM_THM_ALU_ABS_G2_NC	bar
0[0-9a-f]+ <[^>]+> 3000      	adds	r0, #0
			4: R_ARM_THM_ALU_ABS_G1_NC	bar
0[0-9a-f]+ <[^>]+> 3000      	adds	r0, #0
			6: R_ARM_THM_ALU_ABS_G0_NC	bar
#...
