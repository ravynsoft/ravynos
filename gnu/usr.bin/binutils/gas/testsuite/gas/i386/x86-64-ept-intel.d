#objdump: -drwMintel
#name: x86-64 EPT (Intel mode)
#source: x86-64-ept.s

.*: +file format .*

Disassembly of section .text:

0+ <_start>:
[ 	]*[a-f0-9]+:	66 0f 38 80 19       	invept rbx,OWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	66 44 0f 38 80 19    	invept r11,OWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	66 0f 38 81 19       	invvpid rbx,OWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	66 44 0f 38 81 19    	invvpid r11,OWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	66 0f 38 80 19       	invept rbx,OWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	66 44 0f 38 80 19    	invept r11,OWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	66 0f 38 81 19       	invvpid rbx,OWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	66 44 0f 38 81 19    	invvpid r11,OWORD PTR \[rcx\]
#pass
