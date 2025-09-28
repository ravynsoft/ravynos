MEMORY
{
	region : ORIGIN = 0x40000000, LENGTH = 8M
}

SECTIONS
{
	.text : ALIGN (4) { *(.text) } > region
	.rodata : ALIGN (4) { *(.rodata) } > region
	/DISCARD/ : { *(.reginfo) *(.MIPS.abiflags) *(.MIPS.options) *(.trampolines) *(.riscv.attributes) }
}
