	.text
	.file 1 "foo/bar.s"
	.loc_mark_labels 1
	.globl foobar
	.type   foobar, %function
foobar:
	.quad   0x1
	.size foobar, .-foobar

	.globl baz
	.type  baz, %function
baz:
	.quad 0x1
	.size baz, .-baz
