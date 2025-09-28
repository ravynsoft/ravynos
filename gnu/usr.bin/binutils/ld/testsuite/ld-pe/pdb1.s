.text

	.long 0x12345678
	.long 0x9abcdef0

.global foo
foo:	# section 0001, offset 00000008
	.secrel32 bar

.data
	.long 0x12345678

.global bar
bar:	# section 0002, offset 00000004
	.long 0x9abcdef0

.section "gcsect"

.global baz
baz:	# unreferenced, will be GC'd out
	.long 0x12345678
