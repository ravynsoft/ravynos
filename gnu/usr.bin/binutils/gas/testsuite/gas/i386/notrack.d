#objdump: -dw
#name: i386 NOTRACK prefix

.*: +file format .*


Disassembly of section .text:

0+ <_start>:
[ 	]*[a-f0-9]+:	3e ff d0             	notrack call \*%eax
[ 	]*[a-f0-9]+:	3e 66 ff d0          	notrack call \*%ax
[ 	]*[a-f0-9]+:	3e ff e0             	notrack jmp \*%eax
[ 	]*[a-f0-9]+:	3e 66 ff e0          	notrack jmp \*%ax
[ 	]*[a-f0-9]+:	3e ff 10             	notrack call \*\(%eax\)
[ 	]*[a-f0-9]+:	3e 66 ff 10          	notrack callw \*\(%eax\)
[ 	]*[a-f0-9]+:	3e ff 20             	notrack jmp \*\(%eax\)
[ 	]*[a-f0-9]+:	3e 66 ff 20          	notrack jmpw \*\(%eax\)
[ 	]*[a-f0-9]+:	3e f2 ff d0          	notrack bnd call \*%eax
[ 	]*[a-f0-9]+:	3e 66 f2 ff d0       	notrack bnd call \*%ax
[ 	]*[a-f0-9]+:	3e f2 ff e0          	notrack bnd jmp \*%eax
[ 	]*[a-f0-9]+:	3e 66 f2 ff e0       	notrack bnd jmp \*%ax
[ 	]*[a-f0-9]+:	3e f2 ff 10          	notrack bnd call \*\(%eax\)
[ 	]*[a-f0-9]+:	3e 66 f2 ff 10       	notrack bnd callw \*\(%eax\)
[ 	]*[a-f0-9]+:	3e f2 ff 20          	notrack bnd jmp \*\(%eax\)
[ 	]*[a-f0-9]+:	3e 66 f2 ff 20       	notrack bnd jmpw \*\(%eax\)
[ 	]*[a-f0-9]+:	3e f2 ff d0          	notrack bnd call \*%eax
[ 	]*[a-f0-9]+:	3e 66 f2 ff d0       	notrack bnd call \*%ax
[ 	]*[a-f0-9]+:	3e f2 ff 10          	notrack bnd call \*\(%eax\)
[ 	]*[a-f0-9]+:	3e 66 f2 ff 10       	notrack bnd callw \*\(%eax\)
[ 	]*[a-f0-9]+:	3e ff d0             	notrack call \*%eax
[ 	]*[a-f0-9]+:	3e 66 ff d0          	notrack call \*%ax
[ 	]*[a-f0-9]+:	3e ff e0             	notrack jmp \*%eax
[ 	]*[a-f0-9]+:	3e 66 ff e0          	notrack jmp \*%ax
[ 	]*[a-f0-9]+:	3e ff 10             	notrack call \*\(%eax\)
[ 	]*[a-f0-9]+:	3e 66 ff 10          	notrack callw \*\(%eax\)
[ 	]*[a-f0-9]+:	3e ff 20             	notrack jmp \*\(%eax\)
[ 	]*[a-f0-9]+:	3e 66 ff 20          	notrack jmpw \*\(%eax\)
[ 	]*[a-f0-9]+:	3e f2 ff d0          	notrack bnd call \*%eax
[ 	]*[a-f0-9]+:	3e 66 f2 ff d0       	notrack bnd call \*%ax
[ 	]*[a-f0-9]+:	3e f2 ff e0          	notrack bnd jmp \*%eax
[ 	]*[a-f0-9]+:	3e 66 f2 ff e0       	notrack bnd jmp \*%ax
[ 	]*[a-f0-9]+:	3e f2 ff 10          	notrack bnd call \*\(%eax\)
[ 	]*[a-f0-9]+:	3e 66 f2 ff 10       	notrack bnd callw \*\(%eax\)
[ 	]*[a-f0-9]+:	3e f2 ff 20          	notrack bnd jmp \*\(%eax\)
[ 	]*[a-f0-9]+:	3e 66 f2 ff 20       	notrack bnd jmpw \*\(%eax\)
[ 	]*[a-f0-9]+:	3e f2 ff d0          	notrack bnd call \*%eax
[ 	]*[a-f0-9]+:	3e 66 f2 ff d0       	notrack bnd call \*%ax
[ 	]*[a-f0-9]+:	3e f2 ff 10          	notrack bnd call \*\(%eax\)
[ 	]*[a-f0-9]+:	3e 66 f2 ff 10       	notrack bnd callw \*\(%eax\)
[ 	]*[a-f0-9]+:	f2 3e ff d0          	bnd notrack call \*%eax
[ 	]*[a-f0-9]+:	66 3e ff d0          	notrack call \*%ax
#pass
