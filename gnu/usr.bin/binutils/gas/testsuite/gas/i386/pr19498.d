#objdump: -dwr

.*: +file format .*


Disassembly of section .text:

0+ <callmefirst>:
[ 	]*[a-f0-9]+:	90                   	nop

0+1 <callmesecond>:
[ 	]*[a-f0-9]+:	eb fd                	jmp    0 <callmefirst>
#pass
