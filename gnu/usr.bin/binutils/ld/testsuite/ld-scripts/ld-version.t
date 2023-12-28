SECTIONS
{
	.comment :
	{
		*(.comment);
		LINKER_VERSION;
	}
	/DISCARD/ : { *(*) }
}
