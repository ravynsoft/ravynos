.*: +file format .*mips.*

Disassembly of section \.MIPS\.stubs:

.* <_MIPS_STUBS_>:
.*:	ff3c 8010 	lw	t9,-32752\(gp\)
.*:	001f 7a90 	move	t7,ra
.*:	03f9 0f3c 	jalr	t9
.*:	3300 7fff 	li	t8,32767
.*:	0000 0000 	nop
.*:	0000 0000 	nop
.*:	0000 0000 	nop
.*:	0000 0000 	nop

Disassembly of section \.text:
#pass
