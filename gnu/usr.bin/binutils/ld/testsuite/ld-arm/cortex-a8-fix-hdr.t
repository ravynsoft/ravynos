

SECTIONS {
	. = SIZEOF_HEADERS;
	. += 0xf80;
	.text : {
	   *(.text)	 
	} = 0
	/DISCARD/ : { *(*) }
}
