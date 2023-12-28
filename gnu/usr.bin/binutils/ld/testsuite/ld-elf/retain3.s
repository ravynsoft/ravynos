/* The retention of bar should also prevent foo from being gc'ed, since bar
   references foo.  */
	.section	.text.foo,"ax"
	.global	foo
	.type	foo, %function
foo:
	.word 0

	.section	.data.bar,"awR"
	.global	bar
	.type	bar, %object
bar:
	.long foo

	.section	.text._start,"ax"
	.global	_start
	.type	_start, %function
_start:
	.word 0
