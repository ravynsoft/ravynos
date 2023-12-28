/* Undefined symbol reference in retained section .data.retained_var requires
   symbol definition to be pulled out of library.  */
	.section	.data.retained_var,"awR"
	.global	retained_var
	.type	retained_var, %object
retained_var:
	.long bar

	.section	.text._start,"ax"
	.global	_start
	.type	_start, %function
_start:
	.word 0
