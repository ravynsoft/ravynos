	.macro m1 args:vararg
	.nop
	.endm

	.macro m2 args:vararg
	.file "Line2.s"
	.line 7
	.nop
	.endm

	.macro m3 args:vararg
# 10013 "line.S"
	.nop
	.endm

	.text
macro:
	m1 1
	m1 2
	m1 3

	m2 1
	m2 2
	m2 3

	m3 1
	m3 2
	m3 3
