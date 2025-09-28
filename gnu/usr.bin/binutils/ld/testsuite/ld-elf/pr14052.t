SECTIONS {
	. = SIZEOF_HEADERS;
	.text : {
		*(.text)
	}
	. = ALIGN (0x1000);
	.data : {
		_data_start = .;
		*(.data)
	}
	/DISCARD/ : { *(.*) }
}
