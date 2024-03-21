/* Simple script for testing relaxation */

OUTPUT_FORMAT("elf32-littlenios2", "elf32-littlenios2", "elf32-littlenios2")
OUTPUT_ARCH(nios2)
ENTRY(_start)
SECTIONS
{
	_start = .;
	text2 0 : { *(text2) }
	text1 0x8000 : { *(text1) }
}
