/* Simple script for testing call26 relaxation via linker stubs.
   In this case, input sections text0 and text1 cannot share stubs
   because they are in different output sections.  */

OUTPUT_FORMAT("elf32-littlenios2", "elf32-littlenios2", "elf32-littlenios2")
OUTPUT_ARCH(nios2)
ENTRY(_start)
SECTIONS
{
	_start = .;
	text0 0 : { *(text0) }
	text1 : { *(text1) }
	text2 0x40000000 : { *(text2) }
}
