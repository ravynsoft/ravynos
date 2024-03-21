# Test handling of code alignment.
.text
.nocmp
.globl f
f:
# Fetch packet.
	nop 2
.align 0
	nop 3
.align 1
	nop 4
.align 2
	nop 5
	nop 5
.align 3
	nop 6
.align 4
# Fetch packet.
	nop 7
.align 5
# Fetch packet.
	nop 4
# Fetch packet.
	nop 2
||	nop
||	nop
||	nop
||	nop
||	nop
||	nop
||	nop
# Fetch packet.
	nop 2
||	nop
||	nop
||	nop
||	nop
||	nop
||	nop
||	nop
.align 5
# Fetch packet.
	nop
||	nop
.align 3
	nop 2
||	nop
||	nop
||	nop
.align 4
# Fetch packet.
	nop 4
||	nop
||	nop
||	nop
||	nop
||	nop
||	nop
# Fetch packet.
	nop 5
||	nop
||	nop
||	nop
||	nop
||	nop
# Fetch packet.
	nop 4
||	nop
||	nop
||	nop
||	nop
||	nop
||	nop
.align 5
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop 3
