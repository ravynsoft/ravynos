#as: -32 -mfpxx -modd-spreg
#objdump: -d
#name: FPXX with odd-singles test
.*:     file format .*

Disassembly of section .text:

[ 0-9a-f]+ <.text>:
[ 0-9a-f]+:	44840800 	mtc1	a0,\$f1
[ 0-9a-f]+:	44040800 	mfc1	a0,\$f1
[ 0-9a-f]+:	c4610000 	lwc1	\$f1,0\(v1\)
[ 0-9a-f]+:	e4610000 	swc1	\$f1,0\(v1\)
