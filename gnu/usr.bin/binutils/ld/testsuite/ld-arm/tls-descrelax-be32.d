.*:     file format elf32-.*
architecture: arm.*, flags 0x[0-9a-f]+:
EXEC_P, HAS_SYMS, D_PAGED
start address 0x[0-9a-f]+

Disassembly of section .text:

00008000 <foo>:
    8000:	e59f0004 	ldr	r0, \[pc, #4\]	@ 800c <foo\+0xc>
    8004:	e79f0000 	ldr	r0, \[pc, r0\]
    8008:	e1a00000 	nop			@ .*
    800c:	00008138 	.word	0x00008138
    8010:	e59f0004 	ldr	r0, \[pc, #4\]	@ 801c <foo\+0x1c>
    8014:	e79f0000 	ldr	r0, \[pc, r0\]
    8018:	e1a00000 	nop			@ .*
    801c:	00008128 	.word	0x00008128
    8020:	e59f0004 	ldr	r0, \[pc, #4\]	@ 802c <foo\+0x2c>
    8024:	e1a00000 	nop			@ .*
    8028:	e1a00000 	nop			@ .*
    802c:	0000000c 	.word	0x0000000c
    8030:	e59f0004 	ldr	r0, \[pc, #4\]	@ 803c <foo\+0x3c>
    8034:	e1a00000 	nop			@ .*
    8038:	e1a00000 	nop			@ .*
    803c:	0000000c 	.word	0x0000000c
    8040:	e59f000c 	ldr	r0, \[pc, #12\]	@ 8054 <foo\+0x54>
    8044:	e08f0000 	add	r0, pc, r0
    8048:	e5901000 	ldr	r1, \[r0\]
    804c:	e1a00001 	mov	r0, r1
    8050:	e1a00000 	nop			@ .*
    8054:	000080f8 	.word	0x000080f8
    8058:	e59f000c 	ldr	r0, \[pc, #12\]	@ 806c <foo\+0x6c>
    805c:	e08f0000 	add	r0, pc, r0
    8060:	e5901000 	ldr	r1, \[r0\]
    8064:	e1a00001 	mov	r0, r1
    8068:	e1a00000 	nop			@ .*
    806c:	000080e0 	.word	0x000080e0
    8070:	e59f000c 	ldr	r0, \[pc, #12\]	@ 8084 <foo\+0x84>
    8074:	e1a00000 	nop			@ .*
    8078:	e1a00000 	nop			@ .*
    807c:	e1a00000 	nop			@ .*
    8080:	e1a00000 	nop			@ .*
    8084:	0000000c 	.word	0x0000000c
    8088:	e59f000c 	ldr	r0, \[pc, #12\]	@ 809c <foo\+0x9c>
    808c:	e1a00000 	nop			@ .*
    8090:	e1a00000 	nop			@ .*
    8094:	e1a00000 	nop			@ .*
    8098:	e1a00000 	nop			@ .*
    809c:	0000000c 	.word	0x0000000c

000080a0 <bar>:
    80a0:	4801      	ldr	r0, \[pc, #4\]	@ \(80a8 <bar\+0x8>\)
    80a2:	4478      	add	r0, pc
    80a4:	6800      	ldr	r0, \[r0, #0\]
    80a6:	46c0      	nop			@ .*
    80a8:	0000809e 	.word	0x0000809e
    80ac:	4801      	ldr	r0, \[pc, #4\]	@ \(80b4 <bar\+0x14>\)
    80ae:	4478      	add	r0, pc
    80b0:	6800      	ldr	r0, \[r0, #0\]
    80b2:	46c0      	nop			@ .*
    80b4:	00008092 	.word	0x00008092
    80b8:	4801      	ldr	r0, \[pc, #4\]	@ \(80c0 <bar\+0x20>\)
    80ba:	4478      	add	r0, pc
    80bc:	6800      	ldr	r0, \[r0, #0\]
    80be:	46c0      	nop			@ .*
    80c0:	0000808a 	.word	0x0000808a
    80c4:	4801      	ldr	r0, \[pc, #4\]	@ \(80cc <bar\+0x2c>\)
    80c6:	46c0      	nop			@ .*
    80c8:	46c0      	nop			@ .*
    80ca:	46c0      	nop			@ .*
    80cc:	0000000c 	.word	0x0000000c
    80d0:	4801      	ldr	r0, \[pc, #4\]	@ \(80d8 <bar\+0x38>\)
    80d2:	bf00      	nop
    80d4:	bf00      	nop
    80d6:	46c0      	nop			@ .*
    80d8:	0000000c 	.word	0x0000000c
    80dc:	4801      	ldr	r0, \[pc, #4\]	@ \(80e4 <bar\+0x44>\)
    80de:	bf00      	nop
    80e0:	bf00      	nop
    80e2:	46c0      	nop			@ .*
    80e4:	00000014 	.word	0x00000014
    80e8:	4802      	ldr	r0, \[pc, #8\]	@ \(80f4 <bar\+0x54>\)
    80ea:	4478      	add	r0, pc
    80ec:	6801      	ldr	r1, \[r0, #0\]
    80ee:	1c08      	adds	r0, r1, #0
    80f0:	46c0      	nop			@ .*
    80f2:	46c0      	nop			@ .*
    80f4:	00008056 	.word	0x00008056
    80f8:	4802      	ldr	r0, \[pc, #8\]	@ \(8104 <bar\+0x64>\)
    80fa:	4478      	add	r0, pc
    80fc:	6801      	ldr	r1, \[r0, #0\]
    80fe:	4608      	mov	r0, r1
    8100:	46c0      	nop			@ .*
    8102:	46c0      	nop			@ .*
    8104:	00008046 	.word	0x00008046
    8108:	4802      	ldr	r0, \[pc, #8\]	@ \(8114 <bar\+0x74>\)
    810a:	46c0      	nop			@ .*
    810c:	46c0      	nop			@ .*
    810e:	46c0      	nop			@ .*
    8110:	46c0      	nop			@ .*
    8112:	46c0      	nop			@ .*
    8114:	0000000c 	.word	0x0000000c
    8118:	4802      	ldr	r0, \[pc, #8\]	@ \(8124 <bar\+0x84>\)
    811a:	46c0      	nop			@ .*
    811c:	46c0      	nop			@ .*
    811e:	46c0      	nop			@ .*
    8120:	46c0      	nop			@ .*
    8122:	46c0      	nop			@ .*
    8124:	0000000c 	.word	0x0000000c
