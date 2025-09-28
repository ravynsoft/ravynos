OUTPUT_FORMAT("binary")

ENTRY(_main);
SECTIONS {
	. = 0;
	.setup : { *(.setup) }
}
