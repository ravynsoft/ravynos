
.*:     file format .*

Disassembly of section \.text:

0+10000100 <__tls_get_addr_desc>:
.*:	(7c 08 02 a6|a6 02 08 7c) 	mflr    r0
.*:	(f8 01 00 10|10 00 01 f8) 	std     r0,16\(r1\)
.*:	(f8 81 ff c0|c0 ff 81 f8) 	std     r4,-64\(r1\)
.*:	(f8 a1 ff c8|c8 ff a1 f8) 	std     r5,-56\(r1\)
.*:	(f8 c1 ff d0|d0 ff c1 f8) 	std     r6,-48\(r1\)
.*:	(f8 e1 ff d8|d8 ff e1 f8) 	std     r7,-40\(r1\)
.*:	(f9 01 ff e0|e0 ff 01 f9) 	std     r8,-32\(r1\)
.*:	(f9 21 ff e8|e8 ff 21 f9) 	std     r9,-24\(r1\)
.*:	(f9 41 ff f0|f0 ff 41 f9) 	std     r10,-16\(r1\)
.*:	(f9 61 ff f8|f8 ff 61 f9) 	std     r11,-8\(r1\)
.*:	(f8 21 ff a1|a1 ff 21 f8) 	stdu    r1,-96\(r1\)
.*:	(48 00 00 35|35 00 00 48) 	bl      .* <__tls_get_addr>
.*:	(e8 81 00 20|20 00 81 e8) 	ld      r4,32\(r1\)
.*:	(e8 a1 00 28|28 00 a1 e8) 	ld      r5,40\(r1\)
.*:	(e8 c1 00 30|30 00 c1 e8) 	ld      r6,48\(r1\)
.*:	(e8 e1 00 38|38 00 e1 e8) 	ld      r7,56\(r1\)
.*:	(e9 01 00 40|40 00 01 e9) 	ld      r8,64\(r1\)
.*:	(e9 21 00 48|48 00 21 e9) 	ld      r9,72\(r1\)
.*:	(e9 41 00 50|50 00 41 e9) 	ld      r10,80\(r1\)
.*:	(e9 61 00 58|58 00 61 e9) 	ld      r11,88\(r1\)
.*:	(38 21 00 60|60 00 21 38) 	addi    r1,r1,96
.*:	(e8 01 00 10|10 00 01 e8) 	ld      r0,16\(r1\)
.*:	(7c 08 03 a6|a6 03 08 7c) 	mtlr    r0
.*:	(4e 80 00 20|20 00 80 4e) 	blr

0+10000160 <__tls_get_addr>:
.*:	(4e 80 00 20|20 00 80 4e) 	blr

0+10000164 <_start>:
.*:	(38 62 80 08|08 80 62 38) 	addi    r3,r2,-32760
.*:	(4b ff ff 99|99 ff ff 4b) 	bl      .* <__tls_get_addr_desc>
.*:	(60 00 00 00|00 00 00 60) 	nop
