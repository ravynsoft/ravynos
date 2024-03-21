	.machinemode zarch
	.machine "z900"
.text
	.align	8
.globl start
	.type	start, @function
start:
	larl	%r1,foo@GOTENT
	lg	%r1,0(%r1)
	lgf	%r2,0(%r1)
	br	%r14
	.size	start, .-start
.globl foo
.bss
	.align	4
	.type	foo, @object
	.size	foo, 4
foo:
	.zero	4
