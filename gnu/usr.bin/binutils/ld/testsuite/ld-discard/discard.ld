ENTRY(_start)
SECTIONS
{
  . = SIZEOF_HEADERS;
  /* Sections to be discarded */
  /DISCARD/ : {
	*(.data.exit)
	*(.text.exit)
	}
}
