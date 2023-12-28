#name: i386 balign (-n)
#source: align-1.s
#as: -mtune=generic32 -n
#objdump: -dr

.*: +file format .*i386.*


Disassembly of section .text:

0+ <_start>:
[ 	]*[a-f0-9]+:	89 f8                	mov    %edi,%eax
[ 	]*[a-f0-9]+:	90                   	nop
[ 	]*[a-f0-9]+:	90                   	nop
[ 	]*[a-f0-9]+:	90                   	nop
[ 	]*[a-f0-9]+:	90                   	nop
[ 	]*[a-f0-9]+:	90                   	nop
[ 	]*[a-f0-9]+:	90                   	nop
[ 	]*[a-f0-9]+:	ba 00 00 00 00       	mov    \$0x0,%edx
[ 	]*[a-f0-9]+:	90                   	nop
[ 	]*[a-f0-9]+:	90                   	nop
[ 	]*[a-f0-9]+:	90                   	nop
[ 	]*[a-f0-9]+:	01 c2                	add    %eax,%edx
#pass
