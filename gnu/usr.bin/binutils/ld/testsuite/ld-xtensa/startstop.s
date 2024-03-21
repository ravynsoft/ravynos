	.file	"startstop.s"
	.text
	.section	.rodata
	.align	4
.LC0:
	.section	TEST_SECTION,"aw"
	.align	4
	.literal_position
	.literal .LC1, __start_TEST_SECTION
	.literal .LC2, __stop_TEST_SECTION
	.align	4
	.global	_start
	.type	_start, @function
_start:
	l32r	a2, .LC1
	l32r	a3, .LC2
