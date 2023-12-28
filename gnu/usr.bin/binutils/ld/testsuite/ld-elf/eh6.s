	.section .text.foo, "ax", @progbits
	.globl	foo
	.type	foo, @function
foo:
	.cfi_startproc simple
	.cfi_personality 0x80, indirect_ptr
	ret
	.cfi_endproc
	.size	foo, . - foo

	.section .data.rel.ro, "a", @progbits
indirect_ptr:
	.dc.a my_personality_v0

	.globl my_personality_v0
my_personality_v0:
	.long 0
