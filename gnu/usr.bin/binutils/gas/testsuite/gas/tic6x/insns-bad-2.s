# Test bad registers depending on architecture
.text
.globl f
f:
	abs .L1 a15,a15
	abs .L1 a31,a15
	abs .L1 a15,a31
	abs .L2X a15,b31
	abs .L2X a31,b15
.arch c64x
	abs .L1 a15,a15
	abs .L1 a31,a15
	abs .L1 a15,a31
	abs .L2X a15,b31
	abs .L2X a31,b15
.arch c64x+
	abs .L1 a15,a15
	abs .L1 a31,a15
	abs .L1 a15,a31
	abs .L2X a15,b31
	abs .L2X a31,b15
.arch c67x
	abs .L1 a15,a15
	abs .L1 a31,a15
	abs .L1 a15,a31
	abs .L2X a15,b31
	abs .L2X a31,b15
.arch c67x+
	abs .L1 a15,a15
	abs .L1 a31,a15
	abs .L1 a15,a31
	abs .L2X a15,b31
	abs .L2X a31,b15
.arch c674x
	abs .L1 a15,a15
	abs .L1 a31,a15
	abs .L1 a15,a31
	abs .L2X a15,b31
	abs .L2X a31,b15
.arch c62x
	abs .L1 a15,a15
	abs .L1 a31,a15
	abs .L1 a15,a31
	abs .L2X a15,b31
	abs .L2X a31,b15
	ldb .D1T1 *a31,a0
	ldb .D1T1 *+a1[a30],a0
