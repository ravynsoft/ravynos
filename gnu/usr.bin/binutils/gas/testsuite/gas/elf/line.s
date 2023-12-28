	.macro m1 args:vararg
	.warning "m1/1: \args"
	.nop
	.warning "m1/2: \args"
	.endm

	.macro m2 args:vararg
	.file "Line2.s"
	.line 9
	.warning "m2/1: \args"
	.nop
	.warning "m2/2: \args"
	.endm

	.text

# 10018 "line.s"
	.warning

macro:
	m1 123
	m1 abc
	m1 XYZ
	.warning

	m2 987
	m2 zyx
	m2 CBA
	.warning

# 20032 "line.s"

irp:
	.irp arg, 123, 456
	.warning "irp/1: \arg"
	.nop
	.warning "irp/2: \arg"
	.endr
	.warning

# 30042 "line.s"

rept:
	.rept 3
	.warning "rept/1"
	.nop
	.warning "rept/2"
	.endr
	.warning
