	.file	"ifunc.c"
	.text
	.p2align 4

	.type	resolve_local_foo, %function
resolve_local_foo:
	.nop
	.size	resolve_local_foo, .-resolve_local_foo
	
	.globl	global_foo
	.type	global_foo, %gnu_indirect_function
	.set	global_foo,resolve_local_foo
	
	.globl	resolve_global_foo
	.set	resolve_global_foo,resolve_local_foo

	.type	local_foo, %gnu_indirect_function
	.set	local_foo,resolve_local_foo
