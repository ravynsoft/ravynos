#as:
#objdump: -dwMintel
#name: i386 PREFETCHWT1 insns (Intel disassembly)
#source: prefetchwt1.s

.*: +file format .*


Disassembly of section .text:

0+ <_start>:
[ 	]*[a-f0-9]+:	0f 0d 11             	prefetchwt1 BYTE PTR \[ecx\]
[ 	]*[a-f0-9]+:	0f 0d 94 f4 c0 1d fe ff 	prefetchwt1 BYTE PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:	0f 0d 11             	prefetchwt1 BYTE PTR \[ecx\]
[ 	]*[a-f0-9]+:	0f 0d 94 f4 c0 1d fe ff 	prefetchwt1 BYTE PTR \[esp\+esi\*8-0x1e240\]
#pass
