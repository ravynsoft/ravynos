#as:
#objdump: -dw
#name: i386 CLWB insns
#source: clwb.s

.*: +file format .*


Disassembly of section \.text:

00000000 <_start>:
[ 	]*[a-f0-9]+:[ 	]*66 0f ae 31[ 	]*clwb   \(%ecx\)
[ 	]*[a-f0-9]+:[ 	]*66 0f ae b4 f4 c0 1d fe ff[ 	]*clwb   -0x1e240\(%esp,%esi,8\)
[ 	]*[a-f0-9]+:[ 	]*66 0f ae 31[ 	]*clwb   \(%ecx\)
[ 	]*[a-f0-9]+:[ 	]*66 0f ae b4 f4 c0 1d fe ff[ 	]*clwb   -0x1e240\(%esp,%esi,8\)
#pass
