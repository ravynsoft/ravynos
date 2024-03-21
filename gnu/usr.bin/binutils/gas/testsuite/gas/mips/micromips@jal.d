#objdump: -dr --prefix-addresses --show-raw-insn
#name: MIPS ELF jal
#source: jal.s
#as: -32

# Test the jal macro (microMIPS).

.*: +file format .*mips.*

Disassembly of section \.text:
[0-9a-f]+ <[^>]*> 45d9      	jalr	t9
[0-9a-f]+ <[^>]*> 0000 0000 	nop
[0-9a-f]+ <[^>]*> 0099 0f3c 	jalr	a0,t9
[0-9a-f]+ <[^>]*> 0000 0000 	nop
[0-9a-f]+ <[^>]*> f400 0000 	jal	0+0000 <text_label>
			e: R_MICROMIPS_26_S1	text_label
[0-9a-f]+ <[^>]*> 0000 0000 	nop
[0-9a-f]+ <[^>]*> f400 0000 	jal	0+0000 <text_label>
			16: R_MICROMIPS_26_S1	external_text_label
[0-9a-f]+ <[^>]*> 0000 0000 	nop
[0-9a-f]+ <[^>]*> d400 0000 	j	0+0000 <text_label>
			1e: R_MICROMIPS_26_S1	text_label
[0-9a-f]+ <[^>]*> 0c00      	nop
[0-9a-f]+ <[^>]*> d400 0000 	j	0+0000 <text_label>
			24: R_MICROMIPS_26_S1	external_text_label
[0-9a-f]+ <[^>]*> 0c00      	nop
[0-9a-f]+ <[^>]*> 0c00      	nop
	\.\.\.
