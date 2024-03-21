
.*:     file format .*

Disassembly of section \.text:

.* <.*\.plt_call\.__tls_get_addr_opt@@GLIBC_2\.22>:
.*:	(e8 03 00 00|00 00 03 e8) 	ld      r0,0\(r3\)
.*:	(e9 83 00 08|08 00 83 e9) 	ld      r12,8\(r3\)
.*:	(2c 20 00 00|00 00 20 2c) 	cmpdi   r0,0
.*:	(7c 60 1b 78|78 1b 60 7c) 	mr      r0,r3
.*:	(7c 6c 6a 14|14 6a 6c 7c) 	add     r3,r12,r13
.*:	(4d 82 00 20|20 00 82 4d) 	beqlr
.*:	(7c 03 03 78|78 03 03 7c) 	mr      r3,r0
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
.*:	(f8 41 00 18|18 00 41 f8) 	std     r2,24\(r1\)
.*:	(e9 82 80 28|28 80 82 e9) 	ld      r12,-32728\(r2\)
.*:	(7d 89 03 a6|a6 03 89 7d) 	mtctr   r12
.*:	(4e 80 04 21|21 04 80 4e) 	bctrl
.*:	(e8 41 00 18|18 00 41 e8) 	ld      r2,24\(r1\)
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
	\.\.\.

.* <_start>:
.*:	(38 62 80 08|08 80 62 38) 	addi    r3,r2,-32760
.*:	(4b ff ff 5d|5d ff ff 4b) 	bl      .* <.*\.plt_call\.__tls_get_addr_opt@@GLIBC_2\.22>
.*:	(60 00 00 00|00 00 00 60) 	nop
.*:	(60 00 00 00|00 00 00 60) 	nop
.*:	(00 00 00 00|f8 02 01 00) 	.*
.*:	(00 01 02 f8|00 00 00 00) 	.*

.* <__glink_PLTresolve>:
.*:	(7c 08 02 a6|a6 02 08 7c) 	mflr    r0
.*:	(42 9f 00 05|05 00 9f 42) 	bcl     .*
.*:	(7d 68 02 a6|a6 02 68 7d) 	mflr    r11
.*:	(7c 08 03 a6|a6 03 08 7c) 	mtlr    r0
.*:	(e8 0b ff f0|f0 ff 0b e8) 	ld      r0,-16\(r11\)
.*:	(7d 8b 60 50|50 60 8b 7d) 	subf    r12,r11,r12
.*:	(7d 60 5a 14|14 5a 60 7d) 	add     r11,r0,r11
.*:	(38 0c ff d4|d4 ff 0c 38) 	addi    r0,r12,-44
.*:	(e9 8b 00 00|00 00 8b e9) 	ld      r12,0\(r11\)
.*:	(78 00 f0 82|82 f0 00 78) 	srdi    r0,r0,2
.*:	(7d 89 03 a6|a6 03 89 7d) 	mtctr   r12
.*:	(e9 6b 00 08|08 00 6b e9) 	ld      r11,8\(r11\)
.*:	(4e 80 04 20|20 04 80 4e) 	bctr

.* <__tls_get_addr_opt@plt>:
.*:	(4b ff ff cc|cc ff ff 4b) 	b       .* <__glink_PLTresolve>
