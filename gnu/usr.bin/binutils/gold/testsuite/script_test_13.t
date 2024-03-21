SECTIONS
{
	.text 0 : { *(.text) }

	.rela.dyn 0x10000 : { *(.rela.init) }
	.rel.dyn 0x10000 : { *(.rela.init) }
}
