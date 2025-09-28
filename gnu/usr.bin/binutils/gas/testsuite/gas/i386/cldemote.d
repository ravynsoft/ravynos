#as:
#objdump: -dw
#name: i386 CLDEMOTE insns
#source: cldemote.s

.*: +file format .*


Disassembly of section \.text:

00000000 <_start>:
[ 	]*[a-f0-9]+:[ 	]*0f 1c 01[ 	]*cldemote \(%ecx\)
[ 	]*[a-f0-9]+:[ 	]*0f 1c 84 f4 c0 1d fe ff[ 	]*cldemote -0x1e240\(%esp,%esi,8\)
[ 	]*[a-f0-9]+:[ 	]*0f 1c 01[ 	]*cldemote \(%ecx\)
[ 	]*[a-f0-9]+:[ 	]*0f 1c 84 f4 c0 1d fe ff[ 	]*cldemote -0x1e240\(%esp,%esi,8\)
#pass
