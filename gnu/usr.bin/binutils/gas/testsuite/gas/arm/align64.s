.syntax unified
.thumb
foo:
	mov r0, #1
.p2align 6,,63
	mov r0, #2

.arm
foo2:
	mov r0, #3
.p2align 6,,63
	mov r0, #4
