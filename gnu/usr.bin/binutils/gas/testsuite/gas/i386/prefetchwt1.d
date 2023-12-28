#as:
#objdump: -dw
#name: i386 PREFETCHWT1 insns

.*: +file format .*


Disassembly of section .text:

0+ <_start>:
[ 	]*[a-f0-9]+:	0f 0d 11             	prefetchwt1 \(%ecx\)
[ 	]*[a-f0-9]+:	0f 0d 94 f4 c0 1d fe ff 	prefetchwt1 -0x1e240\(%esp,%esi,8\)
[ 	]*[a-f0-9]+:	0f 0d 11             	prefetchwt1 \(%ecx\)
[ 	]*[a-f0-9]+:	0f 0d 94 f4 c0 1d fe ff 	prefetchwt1 -0x1e240\(%esp,%esi,8\)
#pass
