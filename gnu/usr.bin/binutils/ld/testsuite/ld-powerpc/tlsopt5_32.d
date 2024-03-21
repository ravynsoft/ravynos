#source: tlsopt5_32.s
#as: -a32
#ld: -shared --gc-sections --secure-plt tlsdll32.so
#objdump: -dr
#target: powerpc*-*-*

.*

Disassembly of section \.text:

.* <_start>:
.*:	(f0 ff 21 94|94 21 ff f0) 	stwu    r1,-16\(r1\)
.*:	(a6 02 08 7c|7c 08 02 a6) 	mflr    r0
.*:	(05 00 9f 42|42 9f 00 05) 	bcl     .*
.*:	(08 00 c1 93|93 c1 00 08) 	stw     r30,8\(r1\)
.*:	(a6 02 c8 7f|7f c8 02 a6) 	mflr    r30
.*:	(01 00 de 3f|3f de 00 01) 	addis   r30,r30,1
.*:	(14 00 01 90|90 01 00 14) 	stw     r0,20\(r1\)
.*:	(.. .. de 3b|3b de .. ..) 	addi    r30,r30,.*
.*:	(f8 ff 7e 38|38 7e ff f8) 	addi    r3,r30,-8
.*:	(1d 00 00 48|48 00 00 1d) 	bl      .* <.*__tls_get_addr_opt.*>
.*:	(14 00 01 80|80 01 00 14) 	lwz     r0,20\(r1\)
.*:	(08 00 c1 83|83 c1 00 08) 	lwz     r30,8\(r1\)
.*:	(a6 03 08 7c|7c 08 03 a6) 	mtlr    r0
.*:	(10 00 21 38|38 21 00 10) 	addi    r1,r1,16
.*:	(20 00 80 4e|4e 80 00 20) 	blr
.*

.* <.*__tls_get_addr_opt.*>:
.*:	(00 00 63 81|81 63 00 00) 	lwz     r11,0\(r3\)
.*:	(04 00 83 81|81 83 00 04) 	lwz     r12,4\(r3\)
.*:	(78 1b 60 7c|7c 60 1b 78) 	mr      r0,r3
.*:	(00 00 0b 2c|2c 0b 00 00) 	cmpwi   r11,0
.*:	(14 12 6c 7c|7c 6c 12 14) 	add     r3,r12,r2
.*:	(20 00 82 4d|4d 82 00 20) 	beqlr
.*:	(78 03 03 7c|7c 03 03 78) 	mr      r3,r0
.*:	(00 00 00 60|60 00 00 00) 	nop
.*:	(0c 00 7e 81|81 7e 00 0c) 	lwz     r11,12\(r30\)
.*:	(a6 03 69 7d|7d 69 03 a6) 	mtctr   r11
.*:	(20 04 80 4e|4e 80 04 20) 	bctr
.*:	(00 00 00 60|60 00 00 00) 	nop

.* <__glink(_PLTresolve)?>:
.*:	(00 00 6b 3d|3d 6b 00 00) 	addis   r11,r11,0
.*:	(a6 02 08 7c|7c 08 02 a6) 	mflr    r0
.*:	(05 00 9f 42|42 9f 00 05) 	bcl     .*
.*:	(0c 00 6b 39|39 6b 00 0c) 	addi    r11,r11,12
.*:	(a6 02 88 7d|7d 88 02 a6) 	mflr    r12
.*:	(a6 03 08 7c|7c 08 03 a6) 	mtlr    r0
.*:	(50 58 6c 7d|7d 6c 58 50) 	subf    r11,r12,r11
.*:	(01 00 8c 3d|3d 8c 00 01) 	addis   r12,r12,1
.*:	(.. .. 0c 80|80 0c .. ..) 	lwz     r0,.*\(r12\)
.*:	(.. .. 8c 81|81 8c .. ..) 	lwz     r12,.*\(r12\)
.*:	(a6 03 09 7c|7c 09 03 a6) 	mtctr   r0
.*:	(14 5a 0b 7c|7c 0b 5a 14) 	add     r0,r11,r11
.*:	(14 5a 60 7d|7d 60 5a 14) 	add     r11,r0,r11
.*:	(20 04 80 4e|4e 80 04 20) 	bctr
.*:	(00 00 00 60|60 00 00 00) 	nop
.*:	(00 00 00 60|60 00 00 00) 	nop
