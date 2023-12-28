#objdump: -dwMintel
#name: i386 EPT (Intel disassembly)
#source: ept.s

.*: +file format .*

Disassembly of section .text:

0+ <_start>:
[ 	]*[a-f0-9]+:	66 0f 38 80 19       	invept ebx,OWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	66 0f 38 81 19       	invvpid ebx,OWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	66 0f 38 80 19       	invept ebx,OWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	66 0f 38 81 19       	invvpid ebx,OWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	67 66 0f 38 80 19    	invept ebx,OWORD PTR \[bx\+di\]
[ 	]*[a-f0-9]+:	67 66 0f 38 81 19    	invvpid ebx,OWORD PTR \[bx\+di\]
[ 	]*[a-f0-9]+:	67 66 0f 38 80 19    	invept ebx,OWORD PTR \[bx\+di\]
[ 	]*[a-f0-9]+:	67 66 0f 38 81 19    	invvpid ebx,OWORD PTR \[bx\+di\]
#pass
