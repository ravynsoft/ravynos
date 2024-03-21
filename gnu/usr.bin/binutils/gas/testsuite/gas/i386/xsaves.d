#as:
#objdump: -dw
#name: i386 XSAVES insns
#source: xsaves.s

.*: +file format .*


Disassembly of section \.text:

00000000 <_start>:
[ 	]*[a-f0-9]+:[ 	]*0f c7 29[ 	]*xsaves \(%ecx\)
[ 	]*[a-f0-9]+:[ 	]*0f c7 ac f4 c0 1d fe ff[ 	]*xsaves -0x1e240\(%esp,%esi,8\)
[ 	]*[a-f0-9]+:[ 	]*0f c7 19[ 	]*xrstors \(%ecx\)
[ 	]*[a-f0-9]+:[ 	]*0f c7 9c f4 c0 1d fe ff[ 	]*xrstors -0x1e240\(%esp,%esi,8\)
[ 	]*[a-f0-9]+:[ 	]*0f c7 29[ 	]*xsaves \(%ecx\)
[ 	]*[a-f0-9]+:[ 	]*0f c7 ac f4 c0 1d fe ff[ 	]*xsaves -0x1e240\(%esp,%esi,8\)
[ 	]*[a-f0-9]+:[ 	]*0f c7 19[ 	]*xrstors \(%ecx\)
[ 	]*[a-f0-9]+:[ 	]*0f c7 9c f4 c0 1d fe ff[ 	]*xrstors -0x1e240\(%esp,%esi,8\)
#pass
