defined = addr1;
SECTIONS {
	.text : { *(.text .pr) }
	. = ALIGN (0x1000);
	.data : { *(.data .rw) }
	addr1  = ADDR (.data);
}
