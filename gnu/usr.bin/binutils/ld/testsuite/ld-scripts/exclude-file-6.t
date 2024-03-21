SECTIONS
{
	.data : {
		* (SORT_BY_ALIGNMENT (SORT_BY_NAME (EXCLUDE_FILE (*-b.o) .data)))
                * (SORT_BY_NAME (SORT_BY_ALIGNMENT (EXCLUDE_FILE (*-a.o) .data.*)))
	}

	/DISCARD/ : {
		* (*)
	}
}
