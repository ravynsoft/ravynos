
.*


Disassembly of section \.plt:

00008000 <.plt>:
    8000:	e52de004 	push	{lr}		@ \(str lr, \[sp, #-4\]!\)
    8004:	e59fe004 	ldr	lr, \[pc, #4\]	@ 8010 <.*>
    8008:	e08fe00e 	add	lr, pc, lr
    800c:	e5bef008 	ldr	pc, \[lr, #8\]!
    8010:	00001004 	\.word	0x00001004
00008014 <bar@plt>:
    8014:	4778      	bx	pc
    8016:	e7fd      	b.n	.+ <.+>
    8018:	e28fc600 	add	ip, pc, #0, 12
    801c:	e28cca01 	add	ip, ip, #4096	@ 0x1000
    8020:	e5bcf000 	ldr	pc, \[ip, #0\]!

Disassembly of section \.text:

00008ff0 <foo>:
    8ff0:	46c0      	nop			@ \(mov r8, r8\)
    8ff2:	f240 0000 	movw	r0, #0
    8ff6:	f240 0000 	movw	r0, #0
    8ffa:	f240 0000 	movw	r0, #0
    8ffe:	f000 b803 	b\.w	9008 <foo\+0x18>
    9002:	0000      	movs	r0, r0
    9004:	0000      	movs	r0, r0
    9006:	0000      	movs	r0, r0
    9008:	d001      	beq\.n	900e <foo\+0x1e>
    900a:	f7ff bffa 	b\.w	9002 <foo\+0x12>
    900e:	f7ff b801 	b\.w	8014 <bar@plt>
