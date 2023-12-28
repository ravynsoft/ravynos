#as: -J
#objdump: -dw
#name: i386 branch

.*: +file format .*

Disassembly of section .text:

0+ <.*>:
[ 	]*[a-f0-9]+:	3e 74 03[ 	]+je,pt  +[0-9a-fx]+ <.*>
[ 	]*[a-f0-9]+:	2e 74 00[ 	]+je,pn  +[0-9a-fx]+ <.*>
#pass
