.arch armv8.1-m.main
.text
.syntax unified
.thumb
future:
	bf	branch, target
	bfcsel	branch, target, else, eq
	bfl	branch, target
	add	r0, r0, r1
branch:
	b	target
else:
