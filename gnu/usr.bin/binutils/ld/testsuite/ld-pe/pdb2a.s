.text

.global foo
foo:
	.secrel32 bar
	.long 0
	.long 0
	.long 0

.section "gcsect"

.global baz
baz:
	.long 0x12345678
