#source: tlsopt5.s
#as: -a64
#ld: -shared --gc-sections --no-plt-localentry tlsdll.so
#objdump: -dr
#target: powerpc64*-*-*

.*

Disassembly of section \.text:

.* <.*\.plt_call\.aaaaa>:
.*:	(18 00 41 f8|f8 41 00 18) 	std     r2,24\(r1\)
.*:	(30 80 82 e9|e9 82 80 30) 	ld      r12,-32720\(r2\)
.*:	(a6 03 89 7d|7d 89 03 a6) 	mtctr   r12
.*:	(20 04 80 4e|4e 80 04 20) 	bctr
	\.\.\.

.* <.*\.plt_call\.__tls_get_addr_opt@@GLIBC_2\.22>:
.*:	(00 00 03 e8|e8 03 00 00) 	ld      r0,0\(r3\)
.*:	(08 00 83 e9|e9 83 00 08) 	ld      r12,8\(r3\)
.*:	(00 00 20 2c|2c 20 00 00) 	cmpdi   r0,0
.*:	(78 1b 60 7c|7c 60 1b 78) 	mr      r0,r3
.*:	(14 6a 6c 7c|7c 6c 6a 14) 	add     r3,r12,r13
.*:	(20 00 82 4d|4d 82 00 20) 	beqlr *
.*:	(78 03 03 7c|7c 03 03 78) 	mr      r3,r0
.*:	(a6 02 08 7c|7c 08 02 a6) 	mflr    r0
.*:	(10 00 01 f8|f8 01 00 10) 	std     r0,16\(r1\)
.*:	(c0 ff 81 f8|f8 81 ff c0) 	std     r4,-64\(r1\)
.*:	(c8 ff a1 f8|f8 a1 ff c8) 	std     r5,-56\(r1\)
.*:	(d0 ff c1 f8|f8 c1 ff d0) 	std     r6,-48\(r1\)
.*:	(d8 ff e1 f8|f8 e1 ff d8) 	std     r7,-40\(r1\)
.*:	(e0 ff 01 f9|f9 01 ff e0) 	std     r8,-32\(r1\)
.*:	(e8 ff 21 f9|f9 21 ff e8) 	std     r9,-24\(r1\)
.*:	(f0 ff 41 f9|f9 41 ff f0) 	std     r10,-16\(r1\)
.*:	(f8 ff 61 f9|f9 61 ff f8) 	std     r11,-8\(r1\)
.*:	(a1 ff 21 f8|f8 21 ff a1) 	stdu    r1,-96\(r1\)
.*:	(18 00 41 f8|f8 41 00 18) 	std     r2,24\(r1\)
.*:	(28 80 82 e9|e9 82 80 28) 	ld      r12,-32728\(r2\)
.*:	(a6 03 89 7d|7d 89 03 a6) 	mtctr   r12
.*:	(21 04 80 4e|4e 80 04 21) 	bctrl
.*:	(18 00 41 e8|e8 41 00 18) 	ld      r2,24\(r1\)
.*:	(20 00 81 e8|e8 81 00 20) 	ld      r4,32\(r1\)
.*:	(28 00 a1 e8|e8 a1 00 28) 	ld      r5,40\(r1\)
.*:	(30 00 c1 e8|e8 c1 00 30) 	ld      r6,48\(r1\)
.*:	(38 00 e1 e8|e8 e1 00 38) 	ld      r7,56\(r1\)
.*:	(40 00 01 e9|e9 01 00 40) 	ld      r8,64\(r1\)
.*:	(48 00 21 e9|e9 21 00 48) 	ld      r9,72\(r1\)
.*:	(50 00 41 e9|e9 41 00 50) 	ld      r10,80\(r1\)
.*:	(58 00 61 e9|e9 61 00 58) 	ld      r11,88\(r1\)
.*:	(60 00 21 38|38 21 00 60) 	addi    r1,r1,96
.*:	(10 00 01 e8|e8 01 00 10) 	ld      r0,16\(r1\)
.*:	(a6 03 08 7c|7c 08 03 a6) 	mtlr    r0
.*:	(20 00 80 4e|4e 80 00 20) 	blr
	\.\.\.

.* <_start>:
.*:	(08 80 62 38|38 62 80 08) 	addi    r3,r2,-32760
.*:	(5d ff ff 4b|4b ff ff 5d) 	bl      .* <.*\.plt_call\.__tls_get_addr_opt@@GLIBC_2\.22>
.*:	(00 00 00 60|60 00 00 00) 	nop
.*:	(35 ff ff 4b|4b ff ff 35) 	bl      .* <.*\.plt_call\.aaaaa>
.*:	(18 00 41 e8|e8 41 00 18) 	ld      r2,24\(r1\)
.*:	(00 00 00 60|60 00 00 00) 	nop
.*
.*

.* <__glink_PLTresolve>:
.*:	(a6 02 08 7c|7c 08 02 a6) 	mflr    r0
.*:	(05 00 9f 42|42 9f 00 05) 	bcl     .*
.*:	(a6 02 68 7d|7d 68 02 a6) 	mflr    r11
.*:	(a6 03 08 7c|7c 08 03 a6) 	mtlr    r0
.*:	(f0 ff 0b e8|e8 0b ff f0) 	ld      r0,-16\(r11\)
.*:	(50 60 8b 7d|7d 8b 60 50) 	subf    r12,r11,r12
.*:	(14 5a 60 7d|7d 60 5a 14) 	add     r11,r0,r11
.*:	(d4 ff 0c 38|38 0c ff d4) 	addi    r0,r12,-44
.*:	(00 00 8b e9|e9 8b 00 00) 	ld      r12,0\(r11\)
.*:	(82 f0 00 78|78 00 f0 82) 	srdi    r0,r0,2
.*:	(a6 03 89 7d|7d 89 03 a6) 	mtctr   r12
.*:	(08 00 6b e9|e9 6b 00 08) 	ld      r11,8\(r11\)
.*:	(20 04 80 4e|4e 80 04 20) 	bctr

.* <__tls_get_addr_opt@plt>:
.*	(cc ff ff 4b|4b ff ff cc) 	b       .*

.* <aaaaa@plt>:
.*:	(c8 ff ff 4b|4b ff ff c8) 	b       .*
