#objdump: -dr --prefix-addresses --show-raw-insn
#name: MIPS MIPS32 cop2 instructions
#source: micromips@mips32-cp2.s
#as: -32

# Check MIPS32 cop2 instruction assembly (microMIPS).

.*: +file format .*mips.*

Disassembly of section \.text:
[0-9a-f]+ <[^>]*> 4280 fffe 	bc2f	0+0000 <text_label>
			0: R_MICROMIPS_PC16_S1	text_label
[0-9a-f]+ <[^>]*> 0c00      	nop
[0-9a-f]+ <[^>]*> 42a0 fffe 	bc2t	0+0006 <text_label\+0x6>
			6: R_MICROMIPS_PC16_S1	.*
[0-9a-f]+ <[^>]*> 0c00      	nop
[0-9a-f]+ <[^>]*> 9400 fffe 	b	0+000c <text_label\+0xc>
			c: R_MICROMIPS_PC16_S1	text_label
[0-9a-f]+ <[^>]*> 0c00      	nop
[0-9a-f]+ <[^>]*> 42a0 fffe 	bc2t	0+0012 <.*>
			12: R_MICROMIPS_PC16_S1	text_label
[0-9a-f]+ <[^>]*> 0c00      	nop
[0-9a-f]+ <[^>]*> 4280 fffe 	bc2f	0+0018 <.*\+0x6>
			18: R_MICROMIPS_PC16_S1	.*
[0-9a-f]+ <[^>]*> 0c00      	nop
[0-9a-f]+ <[^>]*> 9400 fffe 	b	0+001e <.*\+0xc>
			1e: R_MICROMIPS_PC16_S1	text_label
[0-9a-f]+ <[^>]*> 0c00      	nop
[0-9a-f]+ <[^>]*> 0022 cd3c 	cfc2	at,\$2
[0-9a-f]+ <[^>]*> 0009 1a2a 	cop2	0x12345
[0-9a-f]+ <[^>]*> 0043 dd3c 	ctc2	v0,\$3
[0-9a-f]+ <[^>]*> 0064 4d3c 	mfc2	v1,\$4
[0-9a-f]+ <[^>]*> 00c7 5d3c 	mtc2	a2,\$7
[0-9a-f]+ <[^>]*> 4280 fffe 	bc2f	0+0038 <.*\+0x14>
			38: R_MICROMIPS_PC16_S1	text_label
[0-9a-f]+ <[^>]*> 0c00      	nop
[0-9a-f]+ <[^>]*> 42a4 fffe 	bc2t	\$cc1,0+003e <.*\+0x1a>
			3e: R_MICROMIPS_PC16_S1	.*
[0-9a-f]+ <[^>]*> 0c00      	nop
[0-9a-f]+ <[^>]*> 9400 fffe 	b	0+0044 <.*\+0x20>
			44: R_MICROMIPS_PC16_S1	text_label
[0-9a-f]+ <[^>]*> 0c00      	nop
[0-9a-f]+ <[^>]*> 42b8 fffe 	bc2t	\$cc6,0+004a <.*>
			4a: R_MICROMIPS_PC16_S1	text_label
[0-9a-f]+ <[^>]*> 0c00      	nop
[0-9a-f]+ <[^>]*> 429c fffe 	bc2f	\$cc7,0+0050 <.*\+0x6>
			50: R_MICROMIPS_PC16_S1	.*
[0-9a-f]+ <[^>]*> 0c00      	nop
[0-9a-f]+ <[^>]*> 9400 fffe 	b	0+0056 <.*\+0xc>
			56: R_MICROMIPS_PC16_S1	text_label
[0-9a-f]+ <[^>]*> 0c00      	nop
	\.\.\.
