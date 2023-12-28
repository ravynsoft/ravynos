#as:
#objdump: -dw -Mintel
#name: i386 CLWB insns (Intel disassembly)
#source: clwb.s

.*: +file format .*


Disassembly of section \.text:

00000000 <_start>:
[ 	]*[a-f0-9]+:[ 	]*66 0f ae 31[ 	]*clwb   BYTE PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*66 0f ae b4 f4 c0 1d fe ff[ 	]*clwb   BYTE PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*66 0f ae 31[ 	]*clwb   BYTE PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*66 0f ae b4 f4 c0 1d fe ff[ 	]*clwb   BYTE PTR \[esp\+esi\*8-0x1e240\]
#pass
