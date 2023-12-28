#objdump: -dr
#as: -mfix-24k -32
#name: 24K: Triple Store (Range Check, sc)
#source: 24k-triple-stores-2-llsc.s

.*: +file format .*mips.*

Disassembly of section \.text:

[0-9a-f]+ <.*>:
 *[0-9a-f]+:	605d b020 	sc	v0,32\(sp\)
 *[0-9a-f]+:	607d b008 	sc	v1,8\(sp\)
 *[0-9a-f]+:	609d bff8 	sc	a0,-8\(sp\)
 *[0-9a-f]+:	60bd b000 	sc	a1,0\(sp\)
 *[0-9a-f]+:	60dd b020 	sc	a2,32\(sp\)
	\.\.\.
