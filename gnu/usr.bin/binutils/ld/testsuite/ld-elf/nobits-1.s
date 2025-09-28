	.globl _entry
	.section .foo,"awx",%progbits
_entry:
	.byte 0
	.section .bar,"ax",%nobits
	.byte 0

	.section	.note.GNU-stack,"",%progbits
