	.text
	.globl g
	.type g, @function
g:
        calll .Lfoo
.Lfoo:
        popl %eax
.Lbar:
        addl $_GLOBAL_OFFSET_TABLE_+(.Lbar-.Lfoo), %eax
	movl f@GOT(%eax), %eax
	ret

	.globl h
	.type h, @function
h:
        leal zed, %eax
        movl zed, %ebx
        addl %ebx, %eax
	ret

	.data
zed:
        .long f - .
