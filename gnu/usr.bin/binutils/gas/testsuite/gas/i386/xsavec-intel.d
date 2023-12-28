#as:
#objdump: -dw -Mintel
#name: i386 XSAVEC insns (Intel disassembly)
#source: xsavec.s

.*: +file format .*


Disassembly of section \.text:

00000000 <_start>:
[ 	]*[a-f0-9]+:[ 	]*0f c7 21[ 	]*xsavec \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*0f c7 a4 f4 c0 1d fe ff[ 	]*xsavec \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*0f c7 21[ 	]*xsavec \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*0f c7 a4 f4 c0 1d fe ff[ 	]*xsavec \[esp\+esi\*8-0x1e240\]
#pass
