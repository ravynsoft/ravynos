	.text
.globl main
main:
	lock
        lock addl $0x1,(%eax)
