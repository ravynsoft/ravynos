/* Simple script for testing call26 relaxation via linker stubs.
   This script is used for a bunch of tests that vary the placement of
   section text0 near a 256 memory segment boundary, by using
   --section-start command-line options.  */

OUTPUT_FORMAT("elf32-littlenios2", "elf32-littlenios2", "elf32-littlenios2")
OUTPUT_ARCH(nios2)
ENTRY(_start)
SECTIONS
{
	_start = .;
	text0 : { *(text0) *(text1) }
	text2 0x40000000 : { *(text2) }
}
