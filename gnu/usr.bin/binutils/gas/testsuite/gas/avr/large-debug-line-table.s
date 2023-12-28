        .file 1 "large-debug-line-table.c"
        .text
        .global _start
_start:
        .rept 4000
        .loc 1 1
        nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
        .endr
