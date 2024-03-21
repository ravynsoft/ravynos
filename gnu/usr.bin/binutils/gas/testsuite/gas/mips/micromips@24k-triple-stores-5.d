#objdump: -dr
#as: -mfix-24k -32
#name: 24K: Triple Store (Mix byte/half/word size check)
#source: 24k-triple-stores-5.s

.*: +file format .*mips.*

Disassembly of section \.text:

[0-9a-f]+ <.*>:
 *[0-9a-f]+:	3848 0007 	sh	v0,7\(t0\)
 *[0-9a-f]+:	1868 0000 	sb	v1,0\(t0\)
 *[0-9a-f]+:	f888 0001 	sw	a0,1\(t0\)
 *[0-9a-f]+:	4680      	break
 *[0-9a-f]+:	3848 0016 	sh	v0,22\(t0\)
 *[0-9a-f]+:	1868 000f 	sb	v1,15\(t0\)
 *[0-9a-f]+:	f888 0018 	sw	a0,24\(t0\)
 *[0-9a-f]+:	4680      	break
 *[0-9a-f]+:	3848 0000 	sh	v0,0\(t0\)
 *[0-9a-f]+:	1868 0009 	sb	v1,9\(t0\)
 *[0-9a-f]+:	f888 0002 	sw	a0,2\(t0\)
 *[0-9a-f]+:	4680      	break
 *[0-9a-f]+:	3848 0006 	sh	v0,6\(t0\)
 *[0-9a-f]+:	1868 0010 	sb	v1,16\(t0\)
 *[0-9a-f]+:	f888 000c 	sw	a0,12\(t0\)
 *[0-9a-f]+:	4680      	break
 *[0-9a-f]+:	3848 000a 	sh	v0,10\(t0\)
 *[0-9a-f]+:	1868 000f 	sb	v1,15\(t0\)
 *[0-9a-f]+:	f888 0004 	sw	a0,4\(t0\)
 *[0-9a-f]+:	4680      	break
 *[0-9a-f]+:	3848 000a 	sh	v0,10\(t0\)
 *[0-9a-f]+:	1868 0010 	sb	v1,16\(t0\)
 *[0-9a-f]+:	f888 0004 	sw	a0,4\(t0\)
 *[0-9a-f]+:	4680      	break
	\.\.\.
