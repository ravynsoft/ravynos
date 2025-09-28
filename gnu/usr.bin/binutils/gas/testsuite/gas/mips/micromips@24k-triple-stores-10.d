#objdump: -drz
#as: -mfix-24k -32
#name: 24K: Triple Store (Intervening data #2)
#source: 24k-triple-stores-10.s

.*: +file format .*mips.*

Disassembly of section \.text:

[0-9a-f]+ <.*>:
 *[0-9a-f]+:	1848 0000 	sb	v0,0\(t0\)
 *[0-9a-f]+:	1868 0008 	sb	v1,8\(t0\)
 *[0-9a-f]+:	1888 0010 	sb	a0,16\(t0\)
#pass
