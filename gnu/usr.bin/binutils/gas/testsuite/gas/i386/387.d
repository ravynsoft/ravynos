#objdump: -dw
#name: i386 387 (cmdline)
#as: -march=i386+387

.*: +file format .*

Disassembly of section .text:

0+ <_387>:
[ 	]*[0-9a-f]+:	d9 ff[ 	]+fcos
[ 	]*[0-9a-f]+:	d9 f5[ 	]+fprem1
[ 	]*[0-9a-f]+:	d9 fe[ 	]+fsin
[ 	]*[0-9a-f]+:	d9 fb[ 	]+fsincos
[ 	]*[0-9a-f]+:	dd e1[ 	]+fucom[ 	]+%st\(1\)
[ 	]*[0-9a-f]+:	dd e9[ 	]+fucomp[ 	]+%st\(1\)
[ 	]*[0-9a-f]+:	da e9[ 	]+fucompp
#pass
