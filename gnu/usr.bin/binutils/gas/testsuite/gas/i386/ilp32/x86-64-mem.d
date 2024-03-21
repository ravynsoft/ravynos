#source: ../x86-64-mem.s
#as: -J
#objdump: -dw
#name: x86-64 (ILP32) mem

.*: +file format .*

Disassembly of section .text:

0+ <_start>:
[ 	]*[a-f0-9]+:	0f 01 06             	sgdt   \(%rsi\)
[ 	]*[a-f0-9]+:	0f 01 0e             	sidt   \(%rsi\)
[ 	]*[a-f0-9]+:	0f 01 16             	lgdt   \(%rsi\)
[ 	]*[a-f0-9]+:	0f 01 1e             	lidt   \(%rsi\)
[ 	]*[a-f0-9]+:	0f 01 3e             	invlpg \(%rsi\)
[ 	]*[a-f0-9]+:	0f c7 0e             	cmpxchg8b \(%rsi\)
[ 	]*[a-f0-9]+:	48 0f c7 0e          	cmpxchg16b \(%rsi\)
[ 	]*[a-f0-9]+:	0f c7 36             	vmptrld \(%rsi\)
[ 	]*[a-f0-9]+:	66 0f c7 36          	vmclear \(%rsi\)
[ 	]*[a-f0-9]+:	f3 0f c7 36          	vmxon  \(%rsi\)
[ 	]*[a-f0-9]+:	0f c7 3e             	vmptrst \(%rsi\)
[ 	]*[a-f0-9]+:	0f ae 06             	fxsave \(%rsi\)
[ 	]*[a-f0-9]+:	0f ae 0e             	fxrstor \(%rsi\)
[ 	]*[a-f0-9]+:	0f ae 16             	ldmxcsr \(%rsi\)
[ 	]*[a-f0-9]+:	0f ae 1e             	stmxcsr \(%rsi\)
[ 	]*[a-f0-9]+:	0f ae 3e             	clflush \(%rsi\)
[ 	]*[a-f0-9]+:	0f 01 06             	sgdt   \(%rsi\)
[ 	]*[a-f0-9]+:	0f 01 0e             	sidt   \(%rsi\)
[ 	]*[a-f0-9]+:	0f 01 16             	lgdt   \(%rsi\)
[ 	]*[a-f0-9]+:	0f 01 1e             	lidt   \(%rsi\)
[ 	]*[a-f0-9]+:	0f 01 3e             	invlpg \(%rsi\)
[ 	]*[a-f0-9]+:	0f c7 0e             	cmpxchg8b \(%rsi\)
[ 	]*[a-f0-9]+:	48 0f c7 0e          	cmpxchg16b \(%rsi\)
[ 	]*[a-f0-9]+:	0f c7 36             	vmptrld \(%rsi\)
[ 	]*[a-f0-9]+:	66 0f c7 36          	vmclear \(%rsi\)
[ 	]*[a-f0-9]+:	f3 0f c7 36          	vmxon  \(%rsi\)
[ 	]*[a-f0-9]+:	0f c7 3e             	vmptrst \(%rsi\)
[ 	]*[a-f0-9]+:	0f ae 06             	fxsave \(%rsi\)
[ 	]*[a-f0-9]+:	0f ae 0e             	fxrstor \(%rsi\)
[ 	]*[a-f0-9]+:	0f ae 16             	ldmxcsr \(%rsi\)
[ 	]*[a-f0-9]+:	0f ae 1e             	stmxcsr \(%rsi\)
[ 	]*[a-f0-9]+:	0f ae 3e             	clflush \(%rsi\)
#pass
