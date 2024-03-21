#name: PR 26001 - distinguish register names from symbols
#objdump: -dw

.*: +file format .*

Disassembly of section .text:

0+00 <start>:
[ 	]+0:[ 	]+09f7 fffc[ 	]+jsr[ 	]+pc, 0 <start>
[ 	]+4:[ 	]+1037 0004[ 	]+mov[ 	]+r0, \$c <space>
[ 	]+8:[ 	]+1dc1 0002[ 	]+mov[ 	]+\$e <r00f>, r1
#pass
