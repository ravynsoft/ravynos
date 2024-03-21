#objdump: -dr --prefix-addresses --show-raw-insn
#name: MIPS ALNV.PS instruction branch swapping
#as: -32
#source: alnv_ps-swap.s

# Check that a register dependency between ALNV.PS and the following
# branch prevents from branch swapping (microMIPS).

.*: +file format .*mips.*

Disassembly of section \.text:
([0-9a-f]+) <[^>]*> cfff      	b	\1 <foo>
[ 	]*[0-9a-f]+: R_MICROMIPS_PC10_S1	foo
[0-9a-f]+ <[^>]*> 5402 20d9 	alnv\.ps	\$f4,\$f2,\$f0,v1
([0-9a-f]+) <[^>]*> 4060 fffe 	bal	\1 <foo\+0x[0-9a-f]+>
[ 	]*[0-9a-f]+: R_MICROMIPS_PC16_S1	foo
[0-9a-f]+ <[^>]*> 5402 20d9 	alnv\.ps	\$f4,\$f2,\$f0,v1
([0-9a-f]+) <[^>]*> 4023 fffe 	bltzal	v1,\1 <foo\+0x[0-9a-f]+>
[ 	]*[0-9a-f]+: R_MICROMIPS_PC16_S1	foo
[0-9a-f]+ <[^>]*> 5402 20d9 	alnv\.ps	\$f4,\$f2,\$f0,v1
[0-9a-f]+ <[^>]*> 45c3      	jalr	v1
[0-9a-f]+ <[^>]*> 5402 20d9 	alnv\.ps	\$f4,\$f2,\$f0,v1
[0-9a-f]+ <[^>]*> 0083 0f3c 	jalr	a0,v1
[0-9a-f]+ <[^>]*> 5402 20d9 	alnv\.ps	\$f4,\$f2,\$f0,v1
[0-9a-f]+ <[^>]*> 5402 20d9 	alnv\.ps	\$f4,\$f2,\$f0,v1
[0-9a-f]+ <[^>]*> 007f 0f3c 	jalr	v1,ra
[0-9a-f]+ <[^>]*> 0000 0000 	nop
([0-9a-f]+) <[^>]*> cfff      	b	\1 <foo\+0x[0-9a-f]+>
[ 	]*[0-9a-f]+: R_MICROMIPS_PC10_S1	foo
[0-9a-f]+ <[^>]*> 5402 27d9 	alnv\.ps	\$f4,\$f2,\$f0,ra
[0-9a-f]+ <[^>]*> 5402 27d9 	alnv\.ps	\$f4,\$f2,\$f0,ra
([0-9a-f]+) <[^>]*> 4060 fffe 	bal	\1 <foo\+0x[0-9a-f]+>
[ 	]*[0-9a-f]+: R_MICROMIPS_PC16_S1	foo
[0-9a-f]+ <[^>]*> 0000 0000 	nop
[0-9a-f]+ <[^>]*> 5402 27d9 	alnv\.ps	\$f4,\$f2,\$f0,ra
([0-9a-f]+) <[^>]*> 4023 fffe 	bltzal	v1,\1 <foo\+0x[0-9a-f]+>
[ 	]*[0-9a-f]+: R_MICROMIPS_PC16_S1	foo
[0-9a-f]+ <[^>]*> 0000 0000 	nop
[0-9a-f]+ <[^>]*> 5402 27d9 	alnv\.ps	\$f4,\$f2,\$f0,ra
[0-9a-f]+ <[^>]*> 45c3      	jalr	v1
[0-9a-f]+ <[^>]*> 0000 0000 	nop
[0-9a-f]+ <[^>]*> 0083 0f3c 	jalr	a0,v1
[0-9a-f]+ <[^>]*> 5402 27d9 	alnv\.ps	\$f4,\$f2,\$f0,ra
[0-9a-f]+ <[^>]*> 007f 0f3c 	jalr	v1,ra
[0-9a-f]+ <[^>]*> 5402 27d9 	alnv\.ps	\$f4,\$f2,\$f0,ra
	\.\.\.
