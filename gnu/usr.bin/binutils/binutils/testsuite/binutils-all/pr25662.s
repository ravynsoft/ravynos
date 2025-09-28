/* PR 25662: objcopy sets invalid sh_offset for the first section in a
   no_contents segment containing program headers.

   Several conditions are required for the bug to manifest:
   - The first loadable segment (which contains the program headers) must only
     contain SHT_NOBITS sections. .bss is the SHT_NOBITS section in this test.
   - The next loadable segment must have a !SHT_NOBITS loadable section. .data
     is the !SHT_NOBITS section in this test.
   - .bss must be positioned after .data in the executable file itself.
   - The size of .data must be such that the calculated VMA of the .bss
     section that follows it is not congruent with the file offset of .bss,
     modulo the p_align of its segment, i.e.:
       (VMA(.data) + sizeof(.data)) % (.bss_segment.p_align) != 0
     This will force the sh_offset of .bss to be aligned so it appears within
     .data.
   - The size of .data must be larger than the program headers in the first
     loadable segment, so that the file offset of .bss is immediately
     after .data, and not padded to a valid alignment by the program headers.

   The bug originally only manifested for ELF targets, but there's no reason not
   to run this testcase for other file formats.  */

	.section .bss
aaa:
	.zero	0x2

	.section .data
ccc:
	.zero	0x201

	.section .text
	.global	_start
_start:
	.long 0
