#source: mem.s
#as: -J
#objdump: -dw -Mintel
#name: i386 mem (Intel mode)

.*: +file format .*

Disassembly of section .text:

0+ <_start>:
[ 	]*[a-f0-9]+:	0f 01 06             	sgdtd  \[esi\]
[ 	]*[a-f0-9]+:	0f 01 0e             	sidtd  \[esi\]
[ 	]*[a-f0-9]+:	0f 01 16             	lgdtd  \[esi\]
[ 	]*[a-f0-9]+:	0f 01 1e             	lidtd  \[esi\]
[ 	]*[a-f0-9]+:	0f 01 3e             	invlpg BYTE PTR \[esi\]
[ 	]*[a-f0-9]+:	0f c7 0e             	cmpxchg8b QWORD PTR \[esi\]
[ 	]*[a-f0-9]+:	0f c7 36             	vmptrld QWORD PTR \[esi\]
[ 	]*[a-f0-9]+:	66 0f c7 36          	vmclear QWORD PTR \[esi\]
[ 	]*[a-f0-9]+:	f3 0f c7 36          	vmxon  QWORD PTR \[esi\]
[ 	]*[a-f0-9]+:	0f c7 3e             	vmptrst QWORD PTR \[esi\]
[ 	]*[a-f0-9]+:	0f ae 06             	fxsave \[esi\]
[ 	]*[a-f0-9]+:	0f ae 0e             	fxrstor \[esi\]
[ 	]*[a-f0-9]+:	0f ae 16             	ldmxcsr DWORD PTR \[esi\]
[ 	]*[a-f0-9]+:	0f ae 1e             	stmxcsr DWORD PTR \[esi\]
[ 	]*[a-f0-9]+:	0f ae 3e             	clflush BYTE PTR \[esi\]
[ 	]*[a-f0-9]+:	0f 01 06             	sgdtd  \[esi\]
[ 	]*[a-f0-9]+:	0f 01 0e             	sidtd  \[esi\]
[ 	]*[a-f0-9]+:	0f 01 16             	lgdtd  \[esi\]
[ 	]*[a-f0-9]+:	0f 01 1e             	lidtd  \[esi\]
[ 	]*[a-f0-9]+:	0f 01 3e             	invlpg BYTE PTR \[esi\]
[ 	]*[a-f0-9]+:	0f c7 0e             	cmpxchg8b QWORD PTR \[esi\]
[ 	]*[a-f0-9]+:	0f c7 36             	vmptrld QWORD PTR \[esi\]
[ 	]*[a-f0-9]+:	66 0f c7 36          	vmclear QWORD PTR \[esi\]
[ 	]*[a-f0-9]+:	f3 0f c7 36          	vmxon  QWORD PTR \[esi\]
[ 	]*[a-f0-9]+:	0f c7 3e             	vmptrst QWORD PTR \[esi\]
[ 	]*[a-f0-9]+:	0f ae 06             	fxsave \[esi\]
[ 	]*[a-f0-9]+:	0f ae 0e             	fxrstor \[esi\]
[ 	]*[a-f0-9]+:	0f ae 16             	ldmxcsr DWORD PTR \[esi\]
[ 	]*[a-f0-9]+:	0f ae 1e             	stmxcsr DWORD PTR \[esi\]
[ 	]*[a-f0-9]+:	0f ae 3e             	clflush BYTE PTR \[esi\]
#pass
