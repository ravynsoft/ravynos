ENTRY(_start)
SECTIONS {
	.text 0x00000000 : {
		*(.text*)
	}
}
