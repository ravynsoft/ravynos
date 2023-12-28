#as:
#objdump: -dwMintel
#name: x86-64 PREFETCHI insns (Intel disassembly)
#source: x86-64-prefetchi.s

.*: +file format .*


Disassembly of section .text:

0+ <_start>:
[ 	]*[a-f0-9]+:	0f 18 3d 78 56 34 12 	prefetchit0 BYTE PTR \[rip\+0x12345678\]        # [0-9a-f]+ <_start\+0x[0-9a-f]+>
[ 	]*[a-f0-9]+:	0f 18 35 78 56 34 12 	prefetchit1 BYTE PTR \[rip\+0x12345678\]        # [0-9a-f]+ <_start\+0x[0-9a-f]+>
[ 	]*[a-f0-9]+:	0f 18 3d 78 56 34 12 	prefetchit0 BYTE PTR \[rip\+0x12345678\]        # [0-9a-f]+ <_start\+0x[0-9a-f]+>
[ 	]*[a-f0-9]+:	0f 18 35 78 56 34 12 	prefetchit1 BYTE PTR \[rip\+0x12345678\]        # [0-9a-f]+ <_start\+0x[0-9a-f]+>
#pass
