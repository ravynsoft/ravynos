#as: -mrelax-relocations=yes
#objdump: -dwr

.*: +file format .*


Disassembly of section .text:

0+ <_start>:
[ 	]*[a-f0-9]+:	b8 00 00 00 00       	mov    \$0x0,%eax	1: R_386_GOT32	foo
[ 	]*[a-f0-9]+:	8b 05 00 00 00 00    	mov    0x0,%eax	7: R_386_GOT32X	foo
[ 	]*[a-f0-9]+:	8b 80 00 00 00 00    	mov    0x0\(%eax\),%eax	d: R_386_GOT32X	foo
[ 	]*[a-f0-9]+:	05 00 00 00 00       	add    \$0x0,%eax	12: R_386_GOT32	foo
[ 	]*[a-f0-9]+:	03 05 00 00 00 00    	add    0x0,%eax	18: R_386_GOT32X	foo
[ 	]*[a-f0-9]+:	03 80 00 00 00 00    	add    0x0\(%eax\),%eax	1e: R_386_GOT32X	foo
[ 	]*[a-f0-9]+:	a9 00 00 00 00       	test   \$0x0,%eax	23: R_386_GOT32	foo
[ 	]*[a-f0-9]+:	85 05 00 00 00 00    	test   %eax,0x0	29: R_386_GOT32X	foo
[ 	]*[a-f0-9]+:	85 80 00 00 00 00    	test   %eax,0x0\(%eax\)	2f: R_386_GOT32X	foo
[ 	]*[a-f0-9]+:	ff 15 00 00 00 00    	call   \*0x0	35: R_386_GOT32X	foo
[ 	]*[a-f0-9]+:	ff 90 00 00 00 00    	call   \*0x0\(%eax\)	3b: R_386_GOT32X	foo
[ 	]*[a-f0-9]+:	ff 25 00 00 00 00    	jmp    \*0x0	41: R_386_GOT32X	foo
[ 	]*[a-f0-9]+:	ff a0 00 00 00 00    	jmp    \*0x0\(%eax\)	47: R_386_GOT32X	foo
[ 	]*[a-f0-9]+:	0f 03 05 00 00 00 00 	lsl    0x0,%eax	4e: R_386_GOT32	foo
[ 	]*[a-f0-9]+:	0f 03 80 00 00 00 00 	lsl    0x0\(%eax\),%eax	55: R_386_GOT32	foo
[ 	]*[a-f0-9]+:	f2 0f 1b 05 00 00 00 00 	bndcn  0x0,%bnd0	5d: R_386_GOT32	foo
[ 	]*[a-f0-9]+:	f2 0f 1b 80 00 00 00 00 	bndcn  0x0\(%eax\),%bnd0	65: R_386_GOT32	foo
[ 	]*[a-f0-9]+:	0f 13 05 00 00 00 00 	movlps %xmm0,0x0	6c: R_386_GOT32	foo
[ 	]*[a-f0-9]+:	0f 13 80 00 00 00 00 	movlps %xmm0,0x0\(%eax\)	73: R_386_GOT32	foo
[ 	]*[a-f0-9]+:	b8 00 00 00 00       	mov    \$0x0,%eax	78: R_386_GOT32	foo
[ 	]*[a-f0-9]+:	8b 05 00 00 00 00    	mov    0x0,%eax	7e: R_386_GOT32X	foo
[ 	]*[a-f0-9]+:	8b 80 00 00 00 00    	mov    0x0\(%eax\),%eax	84: R_386_GOT32X	foo
[ 	]*[a-f0-9]+:	05 00 00 00 00       	add    \$0x0,%eax	89: R_386_GOT32	foo
[ 	]*[a-f0-9]+:	03 05 00 00 00 00    	add    0x0,%eax	8f: R_386_GOT32X	foo
[ 	]*[a-f0-9]+:	03 80 00 00 00 00    	add    0x0\(%eax\),%eax	95: R_386_GOT32X	foo
[ 	]*[a-f0-9]+:	ff 90 00 00 00 00    	call   \*0x0\(%eax\)	9b: R_386_GOT32X	foo
[ 	]*[a-f0-9]+:	ff 15 00 00 00 00    	call   \*0x0	a1: R_386_GOT32X	foo
[ 	]*[a-f0-9]+:	ff a0 00 00 00 00    	jmp    \*0x0\(%eax\)	a7: R_386_GOT32X	foo
[ 	]*[a-f0-9]+:	ff 25 00 00 00 00    	jmp    \*0x0	ad: R_386_GOT32X	foo
#pass
