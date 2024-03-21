#as: -mmnemonic=intel
#objdump: -d -Mintel-mnemonic
#name: i386 float Intel mnemonic (2)
#source: compat.s

.*: +file format .*

Disassembly of section .text:

0+ <.text>:
[ 	]*[a-f0-9]+:	dc eb                	fsub   st\(3\),st
[ 	]*[a-f0-9]+:	d8 e3                	fsub   st,st\(3\)
[ 	]*[a-f0-9]+:	de e9                	fsubp  st\(1\),st
[ 	]*[a-f0-9]+:	de eb                	fsubp  st\(3\),st
[ 	]*[a-f0-9]+:	de eb                	fsubp  st\(3\),st
[ 	]*[a-f0-9]+:	dc e3                	fsubr  st\(3\),st
[ 	]*[a-f0-9]+:	d8 eb                	fsubr  st,st\(3\)
[ 	]*[a-f0-9]+:	de e1                	fsubrp st\(1\),st
[ 	]*[a-f0-9]+:	de e3                	fsubrp st\(3\),st
[ 	]*[a-f0-9]+:	de e3                	fsubrp st\(3\),st
[ 	]*[a-f0-9]+:	dc fb                	fdiv   st\(3\),st
[ 	]*[a-f0-9]+:	d8 f3                	fdiv   st,st\(3\)
[ 	]*[a-f0-9]+:	de f9                	fdivp  st\(1\),st
[ 	]*[a-f0-9]+:	de fb                	fdivp  st\(3\),st
[ 	]*[a-f0-9]+:	de fb                	fdivp  st\(3\),st
[ 	]*[a-f0-9]+:	dc f3                	fdivr  st\(3\),st
[ 	]*[a-f0-9]+:	d8 fb                	fdivr  st,st\(3\)
[ 	]*[a-f0-9]+:	de f1                	fdivrp st\(1\),st
[ 	]*[a-f0-9]+:	de f3                	fdivrp st\(3\),st
[ 	]*[a-f0-9]+:	de f3                	fdivrp st\(3\),st
#pass
