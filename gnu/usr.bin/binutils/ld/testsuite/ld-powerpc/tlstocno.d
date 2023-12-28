#source: tlslib.s
#source: tlstoc.s
#as: -a64
#ld: --no-tls-optimize
#objdump: -dr
#target: powerpc64*-*-*

.*

Disassembly of section \.text:

.* <\.__tls_get_addr>:
.*:	(4e 80 00 20|20 00 80 4e) 	blr

.* <\._start>:
.*:	(38 62 80 00|00 80 62 38) 	addi    r3,r2,-32768
.*:	(4b ff ff f9|f9 ff ff 4b) 	bl      .*
.*:	(60 00 00 00|00 00 00 60) 	nop
.*:	(38 62 80 10|10 80 62 38) 	addi    r3,r2,-32752
.*:	(4b ff ff ed|ed ff ff 4b) 	bl      .*
.*:	(60 00 00 00|00 00 00 60) 	nop
.*:	(38 62 80 20|20 80 62 38) 	addi    r3,r2,-32736
.*:	(4b ff ff e1|e1 ff ff 4b) 	bl      .*
.*:	(60 00 00 00|00 00 00 60) 	nop
.*:	(38 62 80 30|30 80 62 38) 	addi    r3,r2,-32720
.*:	(4b ff ff d5|d5 ff ff 4b) 	bl      .*
.*:	(60 00 00 00|00 00 00 60) 	nop
.*:	(39 23 80 50|50 80 23 39) 	addi    r9,r3,-32688
.*:	(3d 23 00 00|00 00 23 3d) 	addis   r9,r3,0
.*:	(81 49 80 58|58 80 49 81) 	lwz     r10,-32680\(r9\)
.*:	(e9 22 80 40|40 80 22 e9) 	ld      r9,-32704\(r2\)
.*:	(7d 49 18 2a|2a 18 49 7d) 	ldx     r10,r9,r3
.*:	(e9 22 80 48|48 80 22 e9) 	ld      r9,-32696\(r2\)
.*:	(7d 49 6a 2e|2e 6a 49 7d) 	lhzx    r10,r9,r13
.*:	(89 4d 90 70|70 90 4d 89) 	lbz     r10,-28560\(r13\)
.*:	(3d 2d 00 00|00 00 2d 3d) 	addis   r9,r13,0
.*:	(99 49 90 78|78 90 49 99) 	stb     r10,-28552\(r9\)
