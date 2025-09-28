SECTIONS
{
	.data : {
		* (SORT_BY_INIT_PRIORITY (EXCLUDE_FILE (*-b.o) .data))
                * (SORT_BY_ALIGNMENT (SORT_BY_ALIGNMENT (EXCLUDE_FILE (*-a.o) .data.*)))
	}

	/DISCARD/ : {
		* (*)
	}
}
