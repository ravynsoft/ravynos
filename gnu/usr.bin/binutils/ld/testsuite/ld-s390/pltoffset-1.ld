SECTIONS
{
	.text : {
		. = . + 4;
		*(.plt)
	}
	.test : {
		*(.text)
		*(.got)
		*(.got.plt)
		*(.rodata)
		*(.eh_frame)
		*(.interp)
		*(.data)
		*(.bss)
	}

	/* For old binutils which otherwise complain about nonrepresentable
	   sections.  */
	.dynsym : { *(.dynsym) }
	.gnu.version : { *(.gnu.version) }

	/DISCARD/ : {
		*(.rela.text)
		*(.rela.plt)
		*(.rela.got.plt)
		*(.rela.data)
		*(.rela.rodata)
		*(.rela.bss)
		*(.rela.text)
		*(.comment*)
		*(.note*)
	}
}
