#source: elfv2.s
#as: -a64
#ld: -melf64ppc -shared
#objdump: -dr

.*

Disassembly of section \.text:

.* <.*\.plt_call\.f4>:
.*:	(f8 41 00 18|18 00 41 f8) 	std     r2,24\(r1\)
.*:	(e9 82 80 40|40 80 82 e9) 	ld      r12,-32704\(r2\)
.*:	(7d 89 03 a6|a6 03 89 7d) 	mtctr   r12
.*:	(4e 80 04 20|20 04 80 4e) 	bctr
	\.\.\.

.* <.*\.plt_call\.f3>:
.*:	(f8 41 00 18|18 00 41 f8) 	std     r2,24\(r1\)
.*:	(e9 82 80 30|30 80 82 e9) 	ld      r12,-32720\(r2\)
.*:	(7d 89 03 a6|a6 03 89 7d) 	mtctr   r12
.*:	(4e 80 04 20|20 04 80 4e) 	bctr
	\.\.\.

.* <.*\.plt_call\.f5>:
.*:	(f8 41 00 18|18 00 41 f8) 	std     r2,24\(r1\)
.*:	(e9 82 80 28|28 80 82 e9) 	ld      r12,-32728\(r2\)
.*:	(7d 89 03 a6|a6 03 89 7d) 	mtctr   r12
.*:	(4e 80 04 20|20 04 80 4e) 	bctr
	\.\.\.

.* <.*\.plt_call\.f1>:
.*:	(f8 41 00 18|18 00 41 f8) 	std     r2,24\(r1\)
.*:	(e9 82 80 48|48 80 82 e9) 	ld      r12,-32696\(r2\)
.*:	(7d 89 03 a6|a6 03 89 7d) 	mtctr   r12
.*:	(4e 80 04 20|20 04 80 4e) 	bctr
	\.\.\.

.* <.*\.plt_call\.f2>:
.*:	(f8 41 00 18|18 00 41 f8) 	std     r2,24\(r1\)
.*:	(e9 82 80 38|38 80 82 e9) 	ld      r12,-32712\(r2\)
.*:	(7d 89 03 a6|a6 03 89 7d) 	mtctr   r12
.*:	(4e 80 04 20|20 04 80 4e) 	bctr
	\.\.\.

.* <f1>:
.*:	(3c 4c 00 02|02 00 4c 3c) 	addis   r2,r12,2
.*:	(38 42 .. ..|.. .. 42 38) 	addi    r2,r2,.*
.*:	(7c 08 02 a6|a6 02 08 7c) 	mflr    r0
.*:	(f8 21 ff e1|e1 ff 21 f8) 	stdu    r1,-32\(r1\)
.*:	(f8 01 00 30|30 00 01 f8) 	std     r0,48\(r1\)
.*:	(4b .. .. ..|.. .. .. 4b) 	bl      .*\.plt_call\.f1>
.*:	(e8 62 80 08|08 80 62 e8) 	ld      r3,-32760\(r2\)
.*:	(4b .. .. ..|.. .. .. 4b) 	bl      .*\.plt_call\.f2>
.*:	(e8 41 00 18|18 00 41 e8) 	ld      r2,24\(r1\)
.*:	(38 62 80 50|50 80 62 38) 	addi    r3,r2,-32688
.*:	(4b .. .. ..|.. .. .. 4b) 	bl      .*\.plt_call\.f3>
.*:	(e8 41 00 18|18 00 41 e8) 	ld      r2,24\(r1\)
.*:	(4b .. .. ..|.. .. .. 4b) 	bl      .*\.plt_call\.f4>
.*:	(e8 41 00 18|18 00 41 e8) 	ld      r2,24\(r1\)
.*:	(4b .. .. ..|.. .. .. 4b) 	bl      .*\.plt_call\.f5>
.*:	(e8 41 00 18|18 00 41 e8) 	ld      r2,24\(r1\)
.*:	(e8 01 00 30|30 00 01 e8) 	ld      r0,48\(r1\)
.*:	(38 21 00 20|20 00 21 38) 	addi    r1,r1,32
.*:	(7c 08 03 a6|a6 03 08 7c) 	mtlr    r0
.*:	(4e 80 00 20|20 00 80 4e) 	blr

.* <f5>:
.*:	(4e 80 00 20|20 00 80 4e) 	blr
.*:	(60 00 00 00|00 00 00 60) 	nop
.*
.*

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

.* <f5@plt>:
.*:	(4b ff ff cc|cc ff ff 4b) 	b       .* <__glink_PLTresolve>

.* <f3@plt>:
.*:	(4b ff ff c8|c8 ff ff 4b) 	b       .* <__glink_PLTresolve>

.* <f2@plt>:
.*:	(4b ff ff c4|c4 ff ff 4b) 	b       .* <__glink_PLTresolve>

.* <f4@plt>:
.*:	(4b ff ff c0|c0 ff ff 4b) 	b       .* <__glink_PLTresolve>

.* <f1@plt>:
.*:	(4b ff ff bc|bc ff ff 4b) 	b       .* <__glink_PLTresolve>
