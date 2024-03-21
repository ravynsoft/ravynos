SECTIONS {
	.text : {
		_text = .;
		*(.text)
	}
	_end = .;
	/DISCARD/ : { *(.*) }
}
. = ASSERT((_end - _text <= (512 * 1024 * 1024)), "foo");
