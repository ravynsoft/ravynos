SECTIONS {
	 .text : { *(.text .pr) } >rom
	 INCLUDE include-data.t
	 .bss : { *(.bss) }
	 /DISCARD/ : { *(*) }
}
