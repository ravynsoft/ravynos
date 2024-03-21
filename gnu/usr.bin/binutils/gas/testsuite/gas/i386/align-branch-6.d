#as: -malign-branch-boundary=32 -D
#objdump: -dw
#warning_output: align-branch-6.e

.*: +file format .*

Disassembly of section .text:

0+ <_start>:
 +[a-f0-9]+:	eb 3c                	jmp    3e <_start\+0x3e>
 +[a-f0-9]+:	8d b4 26 00 00 00 00 	lea    0x0\(%esi,%eiz,1\),%esi
 +[a-f0-9]+:	8d b4 26 00 00 00 00 	lea    0x0\(%esi,%eiz,1\),%esi
 +[a-f0-9]+:	8d b4 26 00 00 00 00 	lea    0x0\(%esi,%eiz,1\),%esi
 +[a-f0-9]+:	8d b4 26 00 00 00 00 	lea    0x0\(%esi,%eiz,1\),%esi
 +[a-f0-9]+:	8d b4 26 00 00 00 00 	lea    0x0\(%esi,%eiz,1\),%esi
 +[a-f0-9]+:	8d b4 26 00 00 00 00 	lea    0x0\(%esi,%eiz,1\),%esi
 +[a-f0-9]+:	8d b4 26 00 00 00 00 	lea    0x0\(%esi,%eiz,1\),%esi
 +[a-f0-9]+:	8d b4 26 00 00 00 00 	lea    0x0\(%esi,%eiz,1\),%esi
 +[a-f0-9]+:	8d 74 26 00          	lea    0x0\(%esi,%eiz,1\),%esi
 +[a-f0-9]+:	f2 73 bf             	bnd jae 0 <_start>
 +[a-f0-9]+:	c3                   	ret
#pass
