.*: +file format .*mips.*

Disassembly of section \.MIPS\.stubs:

.* <_MIPS_STUBS_>:
.*:	ff3c 8010 	lw	t9,-32752\(gp\)
.*:	0dff      	move	t7,ra
.*:	41b8 0002 	lui	t8,0x2
.*:	45d9      	jalr	t9
.*:	5318 fe80 	ori	t8,t8,0xfe80
.*:	0000 0000 	nop
.*:	0000 0000 	nop
.*:	0000 0000 	nop
.*:	0000 0000 	nop

Disassembly of section \.text:
#pass
