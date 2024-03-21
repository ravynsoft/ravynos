#as: -mmnemonic=att
#objdump: -d -Matt-mnemonic
#name: i386 float AT&T mnemonic

.*: +file format .*

Disassembly of section .text:

0+ <.text>:
[ 	]*[a-f0-9]+:	dc e3                	fsub   %st,%st\(3\)
[ 	]*[a-f0-9]+:	d8 e3                	fsub   %st\(3\),%st
[ 	]*[a-f0-9]+:	de e1                	fsubp  %st,%st\(1\)
[ 	]*[a-f0-9]+:	de e3                	fsubp  %st,%st\(3\)
[ 	]*[a-f0-9]+:	de e3                	fsubp  %st,%st\(3\)
[ 	]*[a-f0-9]+:	dc eb                	fsubr  %st,%st\(3\)
[ 	]*[a-f0-9]+:	d8 eb                	fsubr  %st\(3\),%st
[ 	]*[a-f0-9]+:	de e9                	fsubrp %st,%st\(1\)
[ 	]*[a-f0-9]+:	de eb                	fsubrp %st,%st\(3\)
[ 	]*[a-f0-9]+:	de eb                	fsubrp %st,%st\(3\)
[ 	]*[a-f0-9]+:	dc f3                	fdiv   %st,%st\(3\)
[ 	]*[a-f0-9]+:	d8 f3                	fdiv   %st\(3\),%st
[ 	]*[a-f0-9]+:	de f1                	fdivp  %st,%st\(1\)
[ 	]*[a-f0-9]+:	de f3                	fdivp  %st,%st\(3\)
[ 	]*[a-f0-9]+:	de f3                	fdivp  %st,%st\(3\)
[ 	]*[a-f0-9]+:	dc fb                	fdivr  %st,%st\(3\)
[ 	]*[a-f0-9]+:	d8 fb                	fdivr  %st\(3\),%st
[ 	]*[a-f0-9]+:	de f9                	fdivrp %st,%st\(1\)
[ 	]*[a-f0-9]+:	de fb                	fdivrp %st,%st\(3\)
[ 	]*[a-f0-9]+:	de fb                	fdivrp %st,%st\(3\)
#pass
