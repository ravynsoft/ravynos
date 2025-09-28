/* text: over the 32-bit address range.
   data: far away from text, and nearly-zero. */

ENTRY(_start)
MEMORY
{
	rom (rx)  : ORIGIN = 0x800000000, LENGTH = 0x1000
	ram (!rx) : ORIGIN = 0x10, LENGTH = 0x1000
}
SECTIONS {
	.text : {
		*(.text*)
	} >rom

	.data : {
		*(.data*)
	} >ram

	__global_pointer$ = .;

	.got : {
		*(.got*)
	} > ram
}
