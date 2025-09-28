#source: tls.s
#as: -a64
#ld: -shared
#objdump: -dr
#target: powerpc64*-*-*

.*

Disassembly of section \.text:

.* <.*plt_call\.__tls_get_addr(|_opt)>:
.*	(f8 41 00 28|28 00 41 f8) 	std     r2,40\(r1\)
.*	(e9 82 80 98|98 80 82 e9) 	ld      r12,-32616\(r2\)
.*	(7d 89 03 a6|a6 03 89 7d) 	mtctr   r12
.*	(e8 42 80 a0|a0 80 42 e8) 	ld      r2,-32608\(r2\)
.*	(28 22 00 00|00 00 22 28) 	cmpldi  r2,0
.*	(4c e2 04 20|20 04 e2 4c) 	bnectr\+ *
.*	(48 00 00 ..|.. 00 00 48) 	b       .* <__tls_get_addr@plt>

.* <\._start>:
.*	(38 62 80 20|20 80 62 38) 	addi    r3,r2,-32736
.*	(4b ff ff ..|.. ff ff 4b) 	bl      .*plt_call.__tls_get_addr.*
.*	(e8 41 00 28|28 00 41 e8) 	ld      r2,40\(r1\)
.*	(38 62 80 40|40 80 62 38) 	addi    r3,r2,-32704
.*	(4b ff ff ..|.. ff ff 4b) 	bl      .*plt_call.__tls_get_addr.*
.*	(e8 41 00 28|28 00 41 e8) 	ld      r2,40\(r1\)
.*	(38 62 80 58|58 80 62 38) 	addi    r3,r2,-32680
.*	(4b ff ff ..|.. ff ff 4b) 	bl      .*plt_call.__tls_get_addr.*
.*	(e8 41 00 28|28 00 41 e8) 	ld      r2,40\(r1\)
.*	(38 62 80 30|30 80 62 38) 	addi    r3,r2,-32720
.*	(4b ff ff ..|.. ff ff 4b) 	bl      .*plt_call.__tls_get_addr.*
.*	(e8 41 00 28|28 00 41 e8) 	ld      r2,40\(r1\)
.*	(39 23 80 40|40 80 23 39) 	addi    r9,r3,-32704
.*	(3d 23 00 00|00 00 23 3d) 	addis   r9,r3,0
.*	(81 49 80 48|48 80 49 81) 	lwz     r10,-32696\(r9\)
.*	(e9 22 80 50|50 80 22 e9) 	ld      r9,-32688\(r2\)
.*	(7d 49 18 2a|2a 18 49 7d) 	ldx     r10,r9,r3
.*	(e9 22 80 68|68 80 22 e9) 	ld      r9,-32664\(r2\)
.*	(7d 49 6a 2e|2e 6a 49 7d) 	lhzx    r10,r9,r13
.*	(89 4d 00 00|00 00 4d 89) 	lbz     r10,0\(r13\)
.*	(3d 2d 00 00|00 00 2d 3d) 	addis   r9,r13,0
.*	(99 49 00 00|00 00 49 99) 	stb     r10,0\(r9\)
.*	(38 62 80 08|08 80 62 38) 	addi    r3,r2,-32760
.*	(4b ff ff ..|.. ff ff 4b) 	bl      .*plt_call.__tls_get_addr.*
.*	(e8 41 00 28|28 00 41 e8) 	ld      r2,40\(r1\)
.*	(38 62 80 70|70 80 62 38) 	addi    r3,r2,-32656
.*	(4b ff ff ..|.. ff ff 4b) 	bl      .*plt_call.__tls_get_addr.*
.*	(e8 41 00 28|28 00 41 e8) 	ld      r2,40\(r1\)
.*	(f9 43 80 08|08 80 43 f9) 	std     r10,-32760\(r3\)
.*	(3d 23 00 00|00 00 23 3d) 	addis   r9,r3,0
.*	(91 49 80 10|10 80 49 91) 	stw     r10,-32752\(r9\)
.*	(e9 22 80 18|18 80 22 e9) 	ld      r9,-32744\(r2\)
.*	(7d 49 19 2a|2a 19 49 7d) 	stdx    r10,r9,r3
.*	(e9 22 80 68|68 80 22 e9) 	ld      r9,-32664\(r2\)
.*	(7d 49 6b 2e|2e 6b 49 7d) 	sthx    r10,r9,r13
.*	(e9 4d 00 02|02 00 4d e9) 	lwa     r10,0\(r13\)
.*	(3d 2d 00 00|00 00 2d 3d) 	addis   r9,r13,0
.*	(a9 49 00 00|00 00 49 a9) 	lha     r10,0\(r9\)
.*	(60 00 00 00|00 00 00 60) 	nop
.*	(00 00 00 00|b8 02 01 00) .*
.*	(00 01 02 b8|00 00 00 00) .*
.* <__glink_PLTresolve>:
.*	(7d 88 02 a6|a6 02 88 7d) 	mflr    r12
.*	(42 9f 00 05|05 00 9f 42) 	bcl     20,4\*cr7\+so,.*
.*	(7d 68 02 a6|a6 02 68 7d) 	mflr    r11
.*	(e8 4b ff f0|f0 ff 4b e8) 	ld      r2,-16\(r11\)
.*	(7d 88 03 a6|a6 03 88 7d) 	mtlr    r12
.*	(7d 62 5a 14|14 5a 62 7d) 	add     r11,r2,r11
.*	(e9 8b 00 00|00 00 8b e9) 	ld      r12,0\(r11\)
.*	(e8 4b 00 08|08 00 4b e8) 	ld      r2,8\(r11\)
.*	(7d 89 03 a6|a6 03 89 7d) 	mtctr   r12
.*	(e9 6b 00 10|10 00 6b e9) 	ld      r11,16\(r11\)
.*	(4e 80 04 20|20 04 80 4e) 	bctr
.* <__tls_get_addr@plt>:
.*	(38 00 00 00|00 00 00 38) 	li      r0,0
.*	(4b ff ff d0|d0 ff ff 4b) 	b       .*
