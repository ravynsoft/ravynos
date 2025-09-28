        .section .init,"ax",@progbits
	.globl	_init
	.ent	_init
	.type	_init, @function
_init:
        ld      $2,%call16(bar)
	.end	_init

        .section .fini,"ax",@progbits
	.globl	_fini
	.ent	_fini
	.type	_fini, @function
_fini:
	.end	_fini

	.data
foo:
	.dword	bar

