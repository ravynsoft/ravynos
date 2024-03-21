#as: -I${srcdir}/$subdir
#objdump: -dwMaddr16 -Mdata16
#name: i386 16-bit BMI2

.*: +file format .*


Disassembly of section .text:

0+ <_start>:
[ 	]*[a-f0-9]+:	c4 e3 7b f0 d8 07    	rorx   \$0x7,%eax,%ebx
[ 	]*[a-f0-9]+:	67 c4 e3 7b f0 19 07 	rorx   \$0x7,\(%ecx\),%ebx
[ 	]*[a-f0-9]+:	c4 e2 63 f6 f0       	mulx   %eax,%ebx,%esi
[ 	]*[a-f0-9]+:	67 c4 e2 63 f6 31    	mulx   \(%ecx\),%ebx,%esi
[ 	]*[a-f0-9]+:	c4 e2 63 f5 f0       	pdep   %eax,%ebx,%esi
[ 	]*[a-f0-9]+:	67 c4 e2 63 f5 31    	pdep   \(%ecx\),%ebx,%esi
[ 	]*[a-f0-9]+:	c4 e2 62 f5 f0       	pext   %eax,%ebx,%esi
[ 	]*[a-f0-9]+:	67 c4 e2 62 f5 31    	pext   \(%ecx\),%ebx,%esi
[ 	]*[a-f0-9]+:	c4 e2 78 f5 f3       	bzhi   %eax,%ebx,%esi
[ 	]*[a-f0-9]+:	67 c4 e2 60 f5 31    	bzhi   %ebx,\(%ecx\),%esi
[ 	]*[a-f0-9]+:	c4 e2 7a f7 f3       	sarx   %eax,%ebx,%esi
[ 	]*[a-f0-9]+:	67 c4 e2 62 f7 31    	sarx   %ebx,\(%ecx\),%esi
[ 	]*[a-f0-9]+:	c4 e2 79 f7 f3       	shlx   %eax,%ebx,%esi
[ 	]*[a-f0-9]+:	67 c4 e2 61 f7 31    	shlx   %ebx,\(%ecx\),%esi
[ 	]*[a-f0-9]+:	c4 e2 7b f7 f3       	shrx   %eax,%ebx,%esi
[ 	]*[a-f0-9]+:	67 c4 e2 63 f7 31    	shrx   %ebx,\(%ecx\),%esi
[ 	]*[a-f0-9]+:	c4 e3 7b f0 d8 07    	rorx   \$0x7,%eax,%ebx
[ 	]*[a-f0-9]+:	67 c4 e3 7b f0 19 07 	rorx   \$0x7,\(%ecx\),%ebx
[ 	]*[a-f0-9]+:	67 c4 e3 7b f0 19 07 	rorx   \$0x7,\(%ecx\),%ebx
[ 	]*[a-f0-9]+:	c4 e2 63 f6 f0       	mulx   %eax,%ebx,%esi
[ 	]*[a-f0-9]+:	67 c4 e2 63 f6 31    	mulx   \(%ecx\),%ebx,%esi
[ 	]*[a-f0-9]+:	67 c4 e2 63 f6 31    	mulx   \(%ecx\),%ebx,%esi
[ 	]*[a-f0-9]+:	c4 e2 63 f5 f0       	pdep   %eax,%ebx,%esi
[ 	]*[a-f0-9]+:	67 c4 e2 63 f5 31    	pdep   \(%ecx\),%ebx,%esi
[ 	]*[a-f0-9]+:	67 c4 e2 63 f5 31    	pdep   \(%ecx\),%ebx,%esi
[ 	]*[a-f0-9]+:	c4 e2 62 f5 f0       	pext   %eax,%ebx,%esi
[ 	]*[a-f0-9]+:	67 c4 e2 62 f5 31    	pext   \(%ecx\),%ebx,%esi
[ 	]*[a-f0-9]+:	67 c4 e2 62 f5 31    	pext   \(%ecx\),%ebx,%esi
[ 	]*[a-f0-9]+:	c4 e2 78 f5 f3       	bzhi   %eax,%ebx,%esi
[ 	]*[a-f0-9]+:	67 c4 e2 60 f5 31    	bzhi   %ebx,\(%ecx\),%esi
[ 	]*[a-f0-9]+:	67 c4 e2 60 f5 31    	bzhi   %ebx,\(%ecx\),%esi
[ 	]*[a-f0-9]+:	c4 e2 7a f7 f3       	sarx   %eax,%ebx,%esi
[ 	]*[a-f0-9]+:	67 c4 e2 62 f7 31    	sarx   %ebx,\(%ecx\),%esi
[ 	]*[a-f0-9]+:	67 c4 e2 62 f7 31    	sarx   %ebx,\(%ecx\),%esi
[ 	]*[a-f0-9]+:	c4 e2 79 f7 f3       	shlx   %eax,%ebx,%esi
[ 	]*[a-f0-9]+:	67 c4 e2 61 f7 31    	shlx   %ebx,\(%ecx\),%esi
[ 	]*[a-f0-9]+:	67 c4 e2 61 f7 31    	shlx   %ebx,\(%ecx\),%esi
[ 	]*[a-f0-9]+:	c4 e2 7b f7 f3       	shrx   %eax,%ebx,%esi
[ 	]*[a-f0-9]+:	67 c4 e2 63 f7 31    	shrx   %ebx,\(%ecx\),%esi
[ 	]*[a-f0-9]+:	67 c4 e2 63 f7 31    	shrx   %ebx,\(%ecx\),%esi
#pass
