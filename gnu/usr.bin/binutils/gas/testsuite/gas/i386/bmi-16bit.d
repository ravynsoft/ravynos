#as: -I${srcdir}/$subdir
#objdump: -dwMaddr16 -Mdata16
#name: i386 16-bit BMI

.*: +file format .*


Disassembly of section .text:

0+ <_start>:
[ 	]*[a-f0-9]+:	f3 0f bc d8          	tzcnt  %ax,%bx
[ 	]*[a-f0-9]+:	67 f3 0f bc 19       	tzcnt  \(%ecx\),%bx
[ 	]*[a-f0-9]+:	c4 e2 60 f2 f0       	andn   %eax,%ebx,%esi
[ 	]*[a-f0-9]+:	67 c4 e2 60 f2 31    	andn   \(%ecx\),%ebx,%esi
[ 	]*[a-f0-9]+:	c4 e2 78 f7 f3       	bextr  %eax,%ebx,%esi
[ 	]*[a-f0-9]+:	67 c4 e2 60 f7 31    	bextr  %ebx,\(%ecx\),%esi
[ 	]*[a-f0-9]+:	66 f3 0f bc d8       	tzcnt  %eax,%ebx
[ 	]*[a-f0-9]+:	67 66 f3 0f bc 19    	tzcnt  \(%ecx\),%ebx
[ 	]*[a-f0-9]+:	c4 e2 60 f3 d8       	blsi   %eax,%ebx
[ 	]*[a-f0-9]+:	67 c4 e2 60 f3 19    	blsi   \(%ecx\),%ebx
[ 	]*[a-f0-9]+:	c4 e2 60 f3 d0       	blsmsk %eax,%ebx
[ 	]*[a-f0-9]+:	67 c4 e2 60 f3 11    	blsmsk \(%ecx\),%ebx
[ 	]*[a-f0-9]+:	c4 e2 60 f3 c8       	blsr   %eax,%ebx
[ 	]*[a-f0-9]+:	67 c4 e2 60 f3 09    	blsr   \(%ecx\),%ebx
[ 	]*[a-f0-9]+:	f3 0f bc d8          	tzcnt  %ax,%bx
[ 	]*[a-f0-9]+:	67 f3 0f bc 19       	tzcnt  \(%ecx\),%bx
[ 	]*[a-f0-9]+:	67 f3 0f bc 19       	tzcnt  \(%ecx\),%bx
[ 	]*[a-f0-9]+:	c4 e2 60 f2 f0       	andn   %eax,%ebx,%esi
[ 	]*[a-f0-9]+:	67 c4 e2 60 f2 31    	andn   \(%ecx\),%ebx,%esi
[ 	]*[a-f0-9]+:	67 c4 e2 60 f2 31    	andn   \(%ecx\),%ebx,%esi
[ 	]*[a-f0-9]+:	c4 e2 78 f7 f3       	bextr  %eax,%ebx,%esi
[ 	]*[a-f0-9]+:	67 c4 e2 60 f7 31    	bextr  %ebx,\(%ecx\),%esi
[ 	]*[a-f0-9]+:	67 c4 e2 60 f7 31    	bextr  %ebx,\(%ecx\),%esi
[ 	]*[a-f0-9]+:	66 f3 0f bc d8       	tzcnt  %eax,%ebx
[ 	]*[a-f0-9]+:	67 66 f3 0f bc 19    	tzcnt  \(%ecx\),%ebx
[ 	]*[a-f0-9]+:	67 66 f3 0f bc 19    	tzcnt  \(%ecx\),%ebx
[ 	]*[a-f0-9]+:	c4 e2 60 f3 d8       	blsi   %eax,%ebx
[ 	]*[a-f0-9]+:	67 c4 e2 60 f3 19    	blsi   \(%ecx\),%ebx
[ 	]*[a-f0-9]+:	67 c4 e2 60 f3 19    	blsi   \(%ecx\),%ebx
[ 	]*[a-f0-9]+:	c4 e2 60 f3 d0       	blsmsk %eax,%ebx
[ 	]*[a-f0-9]+:	67 c4 e2 60 f3 11    	blsmsk \(%ecx\),%ebx
[ 	]*[a-f0-9]+:	67 c4 e2 60 f3 11    	blsmsk \(%ecx\),%ebx
[ 	]*[a-f0-9]+:	c4 e2 60 f3 c8       	blsr   %eax,%ebx
[ 	]*[a-f0-9]+:	67 c4 e2 60 f3 09    	blsr   \(%ecx\),%ebx
[ 	]*[a-f0-9]+:	67 c4 e2 60 f3 09    	blsr   \(%ecx\),%ebx
#pass
