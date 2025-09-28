SECTIONS
{
	.data : {
		EXCLUDE_FILE (*-b.o) * (.data .data.*)
	}

	/DISCARD/ : {
		* (*)
	}
}
