@ Check that \@ is not destroyed when assembling for the ARM.

.macro bar
	mov	r0, #\@
	mov	r0, #\@@comment
	mov	r0, #\@ @comment
.endm

	.byte	'\\
	.byte	'\a
	.byte 	0
	.byte	0
foo:
	bar
	bar
	bar

