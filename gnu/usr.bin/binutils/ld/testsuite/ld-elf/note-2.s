	.globl _entry
	.text
_entry:
	.byte 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
	.section .foo,"awx",%progbits
	.byte 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
	.section .note,"",%note
	.byte 0
