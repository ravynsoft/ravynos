ENTRY(_start)
SECTIONS
{
	.got  0x1000 : {
		*(.got)
	}
	.data 0x2000: {
		*(.data)
	}
        .text 0x900000000 : {
                *(.text)
        }
}
