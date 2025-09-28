#as:
#objdump: -dw
#name: i386 CLFLUSHOPT insns
#source: clflushopt.s

.*: +file format .*


Disassembly of section \.text:

00000000 <_start>:
[ 	]*[a-f0-9]+:[ 	]*66 0f ae 39[ 	]*clflushopt \(%ecx\)
[ 	]*[a-f0-9]+:[ 	]*66 0f ae bc f4 c0 1d fe ff[ 	]*clflushopt -0x1e240\(%esp,%esi,8\)
[ 	]*[a-f0-9]+:[ 	]*66 0f ae 39[ 	]*clflushopt \(%ecx\)
[ 	]*[a-f0-9]+:[ 	]*66 0f ae bc f4 c0 1d fe ff[ 	]*clflushopt -0x1e240\(%esp,%esi,8\)
#pass
