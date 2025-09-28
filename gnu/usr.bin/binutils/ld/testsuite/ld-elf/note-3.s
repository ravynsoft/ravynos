	.globl _entry
	.text
_entry:
	.byte 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
	.global foo
foo:
	.byte 9
	
	.section .note,"",%note
	.byte 0
