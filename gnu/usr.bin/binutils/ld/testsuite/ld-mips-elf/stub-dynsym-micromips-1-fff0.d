.*: +file format .*mips.*

Disassembly of section \.MIPS\.stubs:

.* <_MIPS_STUBS_>:
.*:	ff3c 8010 	lw	t9,-32752\(gp\)
.*:	0dff      	move	t7,ra
.*:	45d9      	jalr	t9
.*:	5300 fff0 	li	t8,0xfff0
.*:	0000 0000 	nop
.*:	0000 0000 	nop
.*:	0000 0000 	nop

Disassembly of section \.text:
#pass
