#objdump: -dr --prefix-addresses --show-raw-insn
#name: MIPS mips4 branch-likely instructions
#source: mips4-branch-likely.s
#as: -32

# Test mips4 branch-likely instructions (microMIPS).

.*: +file format .*mips.*

Disassembly of section \.text:
[0-9a-f]+ <[^>]*> 4384 fffe 	bc1f	\$fcc1,0+0000 <text_label>
			0: R_MICROMIPS_PC16_S1	text_label
[0-9a-f]+ <[^>]*> 0c00      	nop
[0-9a-f]+ <[^>]*> 43a8 fffe 	bc1t	\$fcc2,0+0006 <text_label\+0x6>
			6: R_MICROMIPS_PC16_S1	text_label
[0-9a-f]+ <[^>]*> 0c00      	nop
	\.\.\.
