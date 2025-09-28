@ Test that calls to undefined weak functions resolve to call through
@ the PLT in shared libraries.

	.arch armv7
	.syntax unified
	.text
	.thumb_func
foo:
	bl bar
	.weak bar
