#as:
#objdump: -dw -Mintel
#name: i386 CLDEMOTE insns (Intel disassembly)
#source: cldemote.s

.*: +file format .*


Disassembly of section \.text:

00000000 <_start>:
[ 	]*[a-f0-9]+:[ 	]*0f 1c 01[ 	]*cldemote BYTE PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*0f 1c 84 f4 c0 1d fe ff[ 	]*cldemote BYTE PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*0f 1c 01[ 	]*cldemote BYTE PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*0f 1c 84 f4 c0 1d fe ff[ 	]*cldemote BYTE PTR \[esp\+esi\*8-0x1e240\]
#pass
