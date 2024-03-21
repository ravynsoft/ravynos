#as:
#objdump: -dw -Mintel
#name: i386 CLFLUSHOPT insns (Intel disassembly)
#source: clflushopt.s

.*: +file format .*


Disassembly of section \.text:

00000000 <_start>:
[ 	]*[a-f0-9]+:[ 	]*66 0f ae 39[ 	]*clflushopt BYTE PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*66 0f ae bc f4 c0 1d fe ff[ 	]*clflushopt BYTE PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*66 0f ae 39[ 	]*clflushopt BYTE PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*66 0f ae bc f4 c0 1d fe ff[ 	]*clflushopt BYTE PTR \[esp\+esi\*8-0x1e240\]
#pass
