SECTIONS
{
	.data : {
		* (EXCLUDE_FILE (*-b.o) .data EXCLUDE_FILE (*-a.o) .data.*)
	}

	/DISCARD/ : {
		* (*)
	}
}
