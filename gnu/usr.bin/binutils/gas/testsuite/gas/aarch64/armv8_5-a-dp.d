#as: -march=armv8.5-a
# objdump: -d

.*: .*


Disassembly of section \.text:

0+0 <func>:
.*:	d500403f 	xaflag
.*:	d500405f 	axflag
.*:	1e284041 	frint32z	s1, s2
.*:	1e684062 	frint32z	d2, d3
.*:	1e28c041 	frint32x	s1, s2
.*:	1e68c062 	frint32x	d2, d3
.*:	1e294041 	frint64z	s1, s2
.*:	1e694062 	frint64z	d2, d3
.*:	1e29c041 	frint64x	s1, s2
.*:	1e69c062 	frint64x	d2, d3
.*:	4e61e820 	frint32z	v0.2d, v1.2d
.*:	0e21e820 	frint32z	v0.2s, v1.2s
.*:	4e21e820 	frint32z	v0.4s, v1.4s
.*:	6e61e820 	frint32x	v0.2d, v1.2d
.*:	2e21e820 	frint32x	v0.2s, v1.2s
.*:	6e21e820 	frint32x	v0.4s, v1.4s
.*:	4e61f820 	frint64z	v0.2d, v1.2d
.*:	0e21f820 	frint64z	v0.2s, v1.2s
.*:	4e21f820 	frint64z	v0.4s, v1.4s
.*:	6e61f820 	frint64x	v0.2d, v1.2d
.*:	2e21f820 	frint64x	v0.2s, v1.2s
.*:	6e21f820 	frint64x	v0.4s, v1.4s
