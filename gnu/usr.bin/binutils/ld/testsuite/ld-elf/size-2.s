	.text
	.long 1,2,3,4

	# thread local storage sections
	.section .tdata
	.long 5,6,7,8,9,10,11,12

	.section .tbss
	.long 0,0,0,0,0,0,0,0,0,0,0,0

	.section	.note.GNU-stack,"",%progbits
