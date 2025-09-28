ENTRY(_start)
SECTIONS {
	.text 0x90000000 : {
		*(.text*)
	}
}
