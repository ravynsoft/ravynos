# Test handling of code alignment: architecture-dependent whether
# execute packets can cross fetch packet boundaries.
.text
.nocmp
.globl f
f:
.arch c62x
# Fetch packet.
	nop 2
||	nop
||	nop
||	nop
# Fetch packet.
	nop 3
||	nop
||	nop
||	nop
||	nop
||	nop
||	nop
||	nop
.arch c64x
# Fetch packet.
	nop 4
||	nop
||	nop
||	nop
	nop 5
||	nop
||	nop
||	nop
# Fetch packet.
||	nop
	nop 6
||	nop
||	nop
||	nop
||	nop
||	nop
||	nop
.arch c64x+
# Fetch packet.
	nop 4
||	nop
||	nop
||	nop
	nop 5
||	nop
||	nop
||	nop
# Fetch packet.
||	nop
	nop 6
||	nop
||	nop
||	nop
||	nop
||	nop
||	nop
.arch c674x
# Fetch packet.
	nop 4
||	nop
||	nop
||	nop
	nop 5
||	nop
||	nop
||	nop
# Fetch packet.
||	nop
	nop 6
||	nop
||	nop
||	nop
||	nop
||	nop
||	nop
.arch c67x+
# Fetch packet.
	nop 4
||	nop
||	nop
||	nop
	nop 5
||	nop
||	nop
||	nop
# Fetch packet.
||	nop
	nop 6
||	nop
||	nop
||	nop
||	nop
||	nop
||	nop
.arch c67x
# Fetch packet.
	nop 2
||	nop
||	nop
||	nop
# Fetch packet.
	nop 3
||	nop
||	nop
||	nop
||	nop
||	nop
||	nop
||	nop
