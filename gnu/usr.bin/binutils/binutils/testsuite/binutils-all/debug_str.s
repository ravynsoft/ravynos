/*      This test is derived from a C source file which, when compiled by gcc
	with debugging enabled, managed to create a .debug_str section whose
	first string was ZLIB_VER_SUBVERSION.   The code in bfd/compress.c
	used to just check for the characters "ZLIB" at the start of a section
	and then assume that the section was compressed.  This meant that the BFD
	library then processed the next 8 bytes as if they were the size of the
	decompressed version of the section.  Naturally with this test case the	
	resulting size was gigantic and consequently the library quickly ran out
	of memory.  */

	.section	.debug_str,"MS",@progbits,1
	.string	"ZLIB_VER_SUBREVISION 0"
