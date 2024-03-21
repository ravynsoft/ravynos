# Test C674x instructions generating PC-relative relocations.
.text
.nocmp
.globl ext1
.globl ext2
.globl ext3
.globl a1
.globl b1
.globl irp
.globl nrp
f:
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	addkpc .S2 f,b1,3
	[a2] addkpc .S2 f+4,b3,7
	addkpc .S2 g,b4,0
	addkpc .S2 ext1+8,b5,4
g:
	nop
	nop
	nop
	nop
	nop
f2:
	nop
	nop
	b .S2 ext3+4
	b .S1 ext2
	b .S2 (nrp)
	b .S2 (irp)
	b .S1 (a1)
	b .S2 f2
	[b2] b .S2 f2+4
	b .S2 g2
	b .S2 (b1)
g2:
	nop
	nop
	nop
	nop
	nop
f3:
	nop
	nop
	call .S2 ext3+4
	call .S1 ext2
	call .S2 (nrp)
	call .S2 (irp)
	call .S1 (a1)
	call .S2 f3
	[b2] call .S2 f3+4
	call .S2 g3
	call .S2 (b1)
g3:
	nop
	nop
	nop
	nop
	nop
f4:
	nop
	nop
	bdec .S2 ext3+4,b2
	bdec .S1 ext2,a2
	bdec .S2 (nrp),b2
	bdec .S2 (irp),b2
	bdec .S1 (a1),a2
	bdec .S2 f4,b2
	[!a1] bdec .S2 f4+4,b2
	bdec .S2 g4,b2
	bdec .S2 (b1),b2
g4:
	nop
	nop
	nop
	nop
	nop
f5:
	nop
	nop
	bpos .S2 ext3+4,b2
	bpos .S1 ext2,a2
	bpos .S2 (nrp),b2
	bpos .S2 (irp),b2
	bpos .S1 (a1),a2
	bpos .S2 f5,b2
	[!b1] bpos .S2 f5+4,b2
	bpos .S2 g5,b2
	bpos .S2 (b1),b2
g5:
	nop
	nop
	nop
	nop
	nop
f6:
	nop
	nop
	bnop .S2 ext3+4,0
	bnop .S1 ext2,1
	bnop (nrp),2
	bnop .S2 (irp),3
	bnop .S1 (a1),4
	bnop .S2 f6,5
	[!b1] bnop .S2 f6+4,6
	bnop g6,7
	bnop .S2 (b1),0
g6:
	nop
	nop
	nop
	nop
	nop
f7:
	nop
	nop
	callnop .S2 ext3+4,0
	callnop .S1 ext2,1
	callnop (nrp),2
	callnop .S2 (irp),3
	callnop .S1 (a1),4
	callnop .S2 f7,5
	[a0] callnop .S2 f7+4,6
	callnop g7,7
	callnop .S2 (b1),0
g7:
	nop
	nop
	nop
	nop
	nop
f8:
	nop
	nop
	callp .S2 ext3+4,b3
	callp .S1 ext2,a3
	callp .S1 (nrp),a3
	callp .S2 (irp),b3
	callp .S1 (a1),a3
	callp .S2 f8,b3
	callp .S2 f8+4,b3
	callp .S1 g8,a3
	callp .S2 (b1),b3
g8:
	nop
	nop
	nop
	nop
	nop
f9:
	nop
	nop
	callret .S2 ext3+4
	callret .S1 ext2
	callret .S2 (nrp)
	callret .S2 (irp)
	callret .S1 (a1)
	callret .S2 f9
	[b2] callret .S2 f9+4
	callret .S2 g9
	callret .S2 (b1)
g9:
	nop
	nop
	nop
	nop
	nop
f10:
	nop
	nop
	ret .S2 ext3+4
	ret .S1 ext2
	ret .S2 (nrp)
	ret .S2 (irp)
	ret .S1 (a1)
	ret .S2 f10
	[b2] ret .S2 f10+4
	ret .S2 g10
	ret .S2 (b1)
g10:
	nop
	nop
	nop
	nop
	nop
f11:
	nop
	nop
	retp .S2 ext3+4,b3
	retp .S1 ext2,a3
	retp .S1 (nrp),a3
	retp .S2 (irp),b3
	retp .S1 (a1),a3
	retp .S2 f11,b3
	retp .S2 f11+4,b3
	retp .S1 g11,a3
	retp .S2 (b1),b3
g11:
	nop
	nop
	nop
	nop
	nop
g12:
	.word 0x3014a120
	.word 0x2010a120
	.word 0x00000410
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	.word 0x80801021
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
g13:
	.word 0x3014a120
	.word 0x2010a120
	.word 0x00000410	
	nop
	nop
	nop
	nop
	.word 0xe0000000
	nop
	nop
	nop
	nop
	nop
	nop
	.word 0x80801021
	.word 0xe0000000
