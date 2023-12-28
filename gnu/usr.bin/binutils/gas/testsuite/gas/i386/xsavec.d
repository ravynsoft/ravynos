#as:
#objdump: -dw
#name: i386 XSAVEC insns
#source: xsavec.s

.*: +file format .*


Disassembly of section \.text:

00000000 <_start>:
[ 	]*[a-f0-9]+:[ 	]*0f c7 21[ 	]*xsavec \(%ecx\)
[ 	]*[a-f0-9]+:[ 	]*0f c7 a4 f4 c0 1d fe ff[ 	]*xsavec -0x1e240\(%esp,%esi,8\)
[ 	]*[a-f0-9]+:[ 	]*0f c7 21[ 	]*xsavec \(%ecx\)
[ 	]*[a-f0-9]+:[ 	]*0f c7 a4 f4 c0 1d fe ff[ 	]*xsavec -0x1e240\(%esp,%esi,8\)
#pass
