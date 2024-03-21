        .machinemode zarch
        .machine "z900"
.text
        .align  8
.globl start
        .type   start, @function
start:
	larl	%r1, test
.globl	test
.data
	.align	4
	.byte	23
test:
	.zero	4
