#source: erratum843419_tls_ie.s
#as:
#ld: --fix-cortex-a53-843419 -e0 --section-start .e843419=0x20000000 -Ttext=0x400000 -Tdata=0x40000000
#objdump: -dr
#...

Disassembly of section .e843419:

0*20000000 <farbranch>:
[ ]*20000000:	d10043ff 	sub	sp, sp, #0x10
[ ]*20000004:	d28001a7 	mov	x7, #0xd                   	// #13
[ ]*20000008:	b9000fe7 	str	w7, \[sp, #12\]
[ ]*2000000c:	140003fb 	b	20000ff8 <e843419>
	...

0*20000ff8 <e843419>:
[ ]*20000ff8:	d2a00000 	movz	x0, #0x0, lsl #16
[ ]*20000ffc:	f800c007 	stur	x7, \[x0, #12\]
[ ]*20001000:	d2800128 	mov	x8, #0x9                   	// #9
[ ]*20001004:	f2800208 	movk	x8, #0x10
[ ]*20001008:	8b050020 	add	x0, x1, x5
[ ]*2000100c:	b9400fe7 	ldr	w7, \[sp, #12\]
[ ]*20001010:	0b0700e0 	add	w0, w7, w7
[ ]*20001014:	910043ff 	add	sp, sp, #0x10
[ ]*20001018:	d65f03c0 	ret
[ ]*2000101c:	00000000 	udf	#0
[ ]*20001020:	14000400 	b	20002020 <e843419\+0x1028>
[ ]*20001024:	d503201f 	nop
[ ]*20001028:	00000000 	udf	#0
[ ]*2000102c:	17fffff7 	b	20001008 <e843419\+0x10>
	...

Disassembly of section .text:

0*400000 <main>:
[ ]*400000:	d10043ff 	sub	sp, sp, #0x10
[ ]*400004:	d28001a7 	mov	x7, #0xd                   	// #13
[ ]*400008:	b9000fe7 	str	w7, \[sp, #12\]
[ ]*40000c:	14000005 	b	400020 <__farbranch_veneer>
[ ]*400010:	d65f03c0 	ret
[ ]*400014:	d503201f 	nop
[ ]*400018:	14000400 	b	401018 <__farbranch_veneer\+0xff8>
[ ]*40001c:	d503201f 	nop

0*400020 <__farbranch_veneer>:
[ ]*400020:	900fe010 	adrp	x16, 20000000 <farbranch>
[ ]*400024:	91000210 	add	x16, x16, #0x0
[ ]*400028:	d61f0200 	br	x16
	...
