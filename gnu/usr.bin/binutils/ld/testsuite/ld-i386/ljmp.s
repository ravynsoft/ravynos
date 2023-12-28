 .global seg1
 .equiv seg1, 8
 .global seg2
 .equiv seg2, 0x18
	/* Bad IA-16 segment values --- will overflow R_386_16 (and
	   R_X86_64_16). */
 .global seg3
 .equiv seg3, 0x10000
 .global seg4
 .equiv seg4, -0x10001
