#source: ../x86-64-mem.s
#as: -J
#objdump: -dw -Mintel
#name: x86-64 (ILP32) mem (Intel mode)

.*: +file format .*

Disassembly of section .text:

0+ <_start>:
[ 	]*[a-f0-9]+:	0f 01 06             	sgdt   \[rsi\]
[ 	]*[a-f0-9]+:	0f 01 0e             	sidt   \[rsi\]
[ 	]*[a-f0-9]+:	0f 01 16             	lgdt   \[rsi\]
[ 	]*[a-f0-9]+:	0f 01 1e             	lidt   \[rsi\]
[ 	]*[a-f0-9]+:	0f 01 3e             	invlpg BYTE PTR \[rsi\]
[ 	]*[a-f0-9]+:	0f c7 0e             	cmpxchg8b QWORD PTR \[rsi\]
[ 	]*[a-f0-9]+:	48 0f c7 0e          	cmpxchg16b OWORD PTR \[rsi\]
[ 	]*[a-f0-9]+:	0f c7 36             	vmptrld QWORD PTR \[rsi\]
[ 	]*[a-f0-9]+:	66 0f c7 36          	vmclear QWORD PTR \[rsi\]
[ 	]*[a-f0-9]+:	f3 0f c7 36          	vmxon  QWORD PTR \[rsi\]
[ 	]*[a-f0-9]+:	0f c7 3e             	vmptrst QWORD PTR \[rsi\]
[ 	]*[a-f0-9]+:	0f ae 06             	fxsave \[rsi\]
[ 	]*[a-f0-9]+:	0f ae 0e             	fxrstor \[rsi\]
[ 	]*[a-f0-9]+:	0f ae 16             	ldmxcsr DWORD PTR \[rsi\]
[ 	]*[a-f0-9]+:	0f ae 1e             	stmxcsr DWORD PTR \[rsi\]
[ 	]*[a-f0-9]+:	0f ae 3e             	clflush BYTE PTR \[rsi\]
[ 	]*[a-f0-9]+:	0f 01 06             	sgdt   \[rsi\]
[ 	]*[a-f0-9]+:	0f 01 0e             	sidt   \[rsi\]
[ 	]*[a-f0-9]+:	0f 01 16             	lgdt   \[rsi\]
[ 	]*[a-f0-9]+:	0f 01 1e             	lidt   \[rsi\]
[ 	]*[a-f0-9]+:	0f 01 3e             	invlpg BYTE PTR \[rsi\]
[ 	]*[a-f0-9]+:	0f c7 0e             	cmpxchg8b QWORD PTR \[rsi\]
[ 	]*[a-f0-9]+:	48 0f c7 0e          	cmpxchg16b OWORD PTR \[rsi\]
[ 	]*[a-f0-9]+:	0f c7 36             	vmptrld QWORD PTR \[rsi\]
[ 	]*[a-f0-9]+:	66 0f c7 36          	vmclear QWORD PTR \[rsi\]
[ 	]*[a-f0-9]+:	f3 0f c7 36          	vmxon  QWORD PTR \[rsi\]
[ 	]*[a-f0-9]+:	0f c7 3e             	vmptrst QWORD PTR \[rsi\]
[ 	]*[a-f0-9]+:	0f ae 06             	fxsave \[rsi\]
[ 	]*[a-f0-9]+:	0f ae 0e             	fxrstor \[rsi\]
[ 	]*[a-f0-9]+:	0f ae 16             	ldmxcsr DWORD PTR \[rsi\]
[ 	]*[a-f0-9]+:	0f ae 1e             	stmxcsr DWORD PTR \[rsi\]
[ 	]*[a-f0-9]+:	0f ae 3e             	clflush BYTE PTR \[rsi\]
#pass
