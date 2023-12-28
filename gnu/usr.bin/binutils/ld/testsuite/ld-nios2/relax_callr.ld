/* Simple script for testing relaxation */

OUTPUT_FORMAT("elf32-littlenios2", "elf32-littlenios2", "elf32-littlenios2")
OUTPUT_ARCH(nios2)
ENTRY(_start)
SECTIONS
{
	_start = .;
	text1 0 : { *(text1) }
	text2 0x08000000 : { *(text2) }
}
