#name: undefined weak symbol overflow
#source: undefweak-overflow.s
#ld: -Ttext=0x20000000 -e start
#objdump: -dr --show-raw-insn
#...
[0-9a-f]+ <start>:
[ 0-9a-f]+:	d85fffff 	beqzc	v0,20000000 <start>
[ 0-9a-f]+:	00000000 	nop
[ 0-9a-f]+:	f85ffffd 	bnezc	v0,20000000 <start>
[ 0-9a-f]+:	ec4ffffd 	lwpc	v0,20000000 <start>
[ 0-9a-f]+:	ec5bfffe 	ldpc	v0,20000000 <start>
[ 0-9a-f]+:	cbfffffa 	bc	20000000 <start>
[ 0-9a-f]+:	1000fff9 	b	20000000 <start>
[ 0-9a-f]+:	00000000 	nop
[ 0-9a-f]+:	0411fff7 	bal	20000000 <start>
[ 0-9a-f]+:	3c...... 	lui	a0,0x....
[ 0-9a-f]+:	0c000000 	jal	20000000 <start>
[ 0-9a-f]+:	00000000 	nop
[ 0-9a-f]+:	08000000 	j	20000000 <start>
[ 0-9a-f]+:	00000000 	nop

[0-9a-f]+ <micro>:
[ 0-9a-f]+:	8e63      	beqz	a0,20000000 <start>
[ 0-9a-f]+:	0c00      	nop
[ 0-9a-f]+:	cfe1      	b	20000000 <start>
[ 0-9a-f]+:	0c00      	nop
[ 0-9a-f]+:	9400 ffde 	b	20000000 <start>
[ 0-9a-f]+:	0c00      	nop
[ 0-9a-f]+:	4060 ffdb 	bal	20000000 <start>
[ 0-9a-f]+:	0000 0000 	nop
[ 0-9a-f]+:	f400 0000 	jal	20000000 <start>
[ 0-9a-f]+:	0000 0000 	nop
[ 0-9a-f]+:	d400 0000 	j	20000000 <start>
[ 0-9a-f]+:	0c00      	nop

[0-9a-f]+ <mips16>:
[ 0-9a-f]+:	f7df 1010 	b	20000000 <start>
[ 0-9a-f]+:	1800 0000 	jal	20000000 <start>
[ 0-9a-f]+:	6500      	nop
#pass
