#objdump: -dr --prefix-addresses --show-raw-insn
#name: MIPS ALNV.PS instruction branch swapping
#as: -32

# Check that a register dependency between ALNV.PS and the following
# branch prevents from branch swapping.

.*: +file format .*mips.*

Disassembly of section \.text:
[0-9a-f]+ <[^>]*> 1000ffff 	b	0+0000 <foo>
[0-9a-f]+ <[^>]*> 4c60111e 	alnv\.ps	\$f4,\$f2,\$f0,v1
[0-9a-f]+ <[^>]*> 0411fffd 	bal	0+0000 <foo>
[0-9a-f]+ <[^>]*> 4c60111e 	alnv\.ps	\$f4,\$f2,\$f0,v1
[0-9a-f]+ <[^>]*> 0470fffb 	bltzal	v1,0+0000 <foo>
[0-9a-f]+ <[^>]*> 4c60111e 	alnv\.ps	\$f4,\$f2,\$f0,v1
[0-9a-f]+ <[^>]*> 0060f809 	jalr	v1
[0-9a-f]+ <[^>]*> 4c60111e 	alnv\.ps	\$f4,\$f2,\$f0,v1
[0-9a-f]+ <[^>]*> 00602009 	jalr	a0,v1
[0-9a-f]+ <[^>]*> 4c60111e 	alnv\.ps	\$f4,\$f2,\$f0,v1
[0-9a-f]+ <[^>]*> 4c60111e 	alnv\.ps	\$f4,\$f2,\$f0,v1
[0-9a-f]+ <[^>]*> 03e01809 	jalr	v1,ra
[0-9a-f]+ <[^>]*> 00000000 	nop
[0-9a-f]+ <[^>]*> 1000fff2 	b	0+0000 <foo>
[0-9a-f]+ <[^>]*> 4fe0111e 	alnv\.ps	\$f4,\$f2,\$f0,ra
[0-9a-f]+ <[^>]*> 4fe0111e 	alnv\.ps	\$f4,\$f2,\$f0,ra
[0-9a-f]+ <[^>]*> 0411ffef 	bal	0+0000 <foo>
[0-9a-f]+ <[^>]*> 00000000 	nop
[0-9a-f]+ <[^>]*> 4fe0111e 	alnv\.ps	\$f4,\$f2,\$f0,ra
[0-9a-f]+ <[^>]*> 0470ffec 	bltzal	v1,0+0000 <foo>
[0-9a-f]+ <[^>]*> 00000000 	nop
[0-9a-f]+ <[^>]*> 4fe0111e 	alnv\.ps	\$f4,\$f2,\$f0,ra
[0-9a-f]+ <[^>]*> 0060f809 	jalr	v1
[0-9a-f]+ <[^>]*> 00000000 	nop
[0-9a-f]+ <[^>]*> 00602009 	jalr	a0,v1
[0-9a-f]+ <[^>]*> 4fe0111e 	alnv\.ps	\$f4,\$f2,\$f0,ra
[0-9a-f]+ <[^>]*> 03e01809 	jalr	v1,ra
[0-9a-f]+ <[^>]*> 4fe0111e 	alnv\.ps	\$f4,\$f2,\$f0,ra
	\.\.\.
