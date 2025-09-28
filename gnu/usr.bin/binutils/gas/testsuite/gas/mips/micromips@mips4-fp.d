#objdump: -dr --prefix-addresses --show-raw-insn
#name: MIPS mips4 fp
#source: mips4-fp.s
#as: -32

# Test mips4 fp instructions (microMIPS).

.*: +file format .*mips.*

Disassembly of section \.text:
[0-9a-f]+ <[^>]*> 4380 fffe 	bc1f	0+0000 <text_label>
			0: R_MICROMIPS_PC16_S1	text_label
[0-9a-f]+ <[^>]*> 0c00      	nop
[0-9a-f]+ <[^>]*> 4384 fffe 	bc1f	\$fcc1,0+0006 <text_label\+0x6>
			6: R_MICROMIPS_PC16_S1	text_label
[0-9a-f]+ <[^>]*> 0c00      	nop
[0-9a-f]+ <[^>]*> 43a4 fffe 	bc1t	\$fcc1,0+000c <text_label\+0xc>
			c: R_MICROMIPS_PC16_S1	text_label
[0-9a-f]+ <[^>]*> 0c00      	nop
[0-9a-f]+ <[^>]*> 54c4 043c 	c\.f\.d	\$f4,\$f6
[0-9a-f]+ <[^>]*> 54c4 243c 	c\.f\.d	\$fcc1,\$f4,\$f6
[0-9a-f]+ <[^>]*> 5485 10c8 	ldxc1	\$f2,a0\(a1\)
[0-9a-f]+ <[^>]*> 5485 1048 	lwxc1	\$f2,a0\(a1\)
[0-9a-f]+ <[^>]*> 54c4 0089 	madd\.d	\$f0,\$f2,\$f4,\$f6
[0-9a-f]+ <[^>]*> 5402 5201 	madd\.s	\$f10,\$f8,\$f2,\$f0
[0-9a-f]+ <[^>]*> 5485 817b 	movf	a0,a1,\$fcc4
[0-9a-f]+ <[^>]*> 5486 0220 	movf\.d	\$f4,\$f6,\$fcc0
[0-9a-f]+ <[^>]*> 5486 0020 	movf\.s	\$f4,\$f6,\$fcc0
[0-9a-f]+ <[^>]*> 54c6 2138 	movn\.d	\$f4,\$f6,a2
[0-9a-f]+ <[^>]*> 54c6 2038 	movn\.s	\$f4,\$f6,a2
[0-9a-f]+ <[^>]*> 5485 897b 	movt	a0,a1,\$fcc4
[0-9a-f]+ <[^>]*> 5486 0260 	movt\.d	\$f4,\$f6,\$fcc0
[0-9a-f]+ <[^>]*> 5486 0060 	movt\.s	\$f4,\$f6,\$fcc0
[0-9a-f]+ <[^>]*> 54c6 2178 	movz\.d	\$f4,\$f6,a2
[0-9a-f]+ <[^>]*> 54c6 2078 	movz\.s	\$f4,\$f6,a2
[0-9a-f]+ <[^>]*> 54c4 00a9 	msub\.d	\$f0,\$f2,\$f4,\$f6
[0-9a-f]+ <[^>]*> 54c4 00a1 	msub\.s	\$f0,\$f2,\$f4,\$f6
[0-9a-f]+ <[^>]*> 54c4 008a 	nmadd\.d	\$f0,\$f2,\$f4,\$f6
[0-9a-f]+ <[^>]*> 54c4 0082 	nmadd\.s	\$f0,\$f2,\$f4,\$f6
[0-9a-f]+ <[^>]*> 54c4 00aa 	nmsub\.d	\$f0,\$f2,\$f4,\$f6
[0-9a-f]+ <[^>]*> 54c4 00a2 	nmsub\.s	\$f0,\$f2,\$f4,\$f6
[0-9a-f]+ <[^>]*> 5485 21a0 	prefx	0x4,a0\(a1\)
[0-9a-f]+ <[^>]*> 5486 523b 	recip\.d	\$f4,\$f6
[0-9a-f]+ <[^>]*> 5486 123b 	recip\.s	\$f4,\$f6
[0-9a-f]+ <[^>]*> 5486 423b 	rsqrt\.d	\$f4,\$f6
[0-9a-f]+ <[^>]*> 5486 023b 	rsqrt\.s	\$f4,\$f6
[0-9a-f]+ <[^>]*> 5485 2108 	sdxc1	\$f4,a0\(a1\)
[0-9a-f]+ <[^>]*> 5485 2088 	swxc1	\$f4,a0\(a1\)
[0-9a-f]+ <[^>]*> 0c00      	nop
	\.\.\.
