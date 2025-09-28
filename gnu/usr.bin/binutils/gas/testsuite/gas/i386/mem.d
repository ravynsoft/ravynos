#objdump: -dw
#name: i386 mem

.*:     file format .*

Disassembly of section .text:

0+ <_start>:
[ 	]*[a-f0-9]+:	0f 01 06             	sgdtl  \(%esi\)
[ 	]*[a-f0-9]+:	0f 01 0e             	sidtl  \(%esi\)
[ 	]*[a-f0-9]+:	0f 01 16             	lgdtl  \(%esi\)
[ 	]*[a-f0-9]+:	0f 01 1e             	lidtl  \(%esi\)
[ 	]*[a-f0-9]+:	0f 01 3e             	invlpg \(%esi\)
[ 	]*[a-f0-9]+:	0f c7 0e             	cmpxchg8b \(%esi\)
[ 	]*[a-f0-9]+:	0f c7 36             	vmptrld \(%esi\)
[ 	]*[a-f0-9]+:	66 0f c7 36          	vmclear \(%esi\)
[ 	]*[a-f0-9]+:	f3 0f c7 36          	vmxon  \(%esi\)
[ 	]*[a-f0-9]+:	0f c7 3e             	vmptrst \(%esi\)
[ 	]*[a-f0-9]+:	0f ae 06             	fxsave \(%esi\)
[ 	]*[a-f0-9]+:	0f ae 0e             	fxrstor \(%esi\)
[ 	]*[a-f0-9]+:	0f ae 16             	ldmxcsr \(%esi\)
[ 	]*[a-f0-9]+:	0f ae 1e             	stmxcsr \(%esi\)
[ 	]*[a-f0-9]+:	0f ae 3e             	clflush \(%esi\)
[ 	]*[a-f0-9]+:	0f 01 06             	sgdtl  \(%esi\)
[ 	]*[a-f0-9]+:	0f 01 0e             	sidtl  \(%esi\)
[ 	]*[a-f0-9]+:	0f 01 16             	lgdtl  \(%esi\)
[ 	]*[a-f0-9]+:	0f 01 1e             	lidtl  \(%esi\)
[ 	]*[a-f0-9]+:	0f 01 3e             	invlpg \(%esi\)
[ 	]*[a-f0-9]+:	0f c7 0e             	cmpxchg8b \(%esi\)
[ 	]*[a-f0-9]+:	0f c7 36             	vmptrld \(%esi\)
[ 	]*[a-f0-9]+:	66 0f c7 36          	vmclear \(%esi\)
[ 	]*[a-f0-9]+:	f3 0f c7 36          	vmxon  \(%esi\)
[ 	]*[a-f0-9]+:	0f c7 3e             	vmptrst \(%esi\)
[ 	]*[a-f0-9]+:	0f ae 06             	fxsave \(%esi\)
[ 	]*[a-f0-9]+:	0f ae 0e             	fxrstor \(%esi\)
[ 	]*[a-f0-9]+:	0f ae 16             	ldmxcsr \(%esi\)
[ 	]*[a-f0-9]+:	0f ae 1e             	stmxcsr \(%esi\)
[ 	]*[a-f0-9]+:	0f ae 3e             	clflush \(%esi\)
#pass
