	.text
	.global	_start
	.type	_start, %function
_start:
	bl foo
	bl bar
	bl baz
baz:
	ret

	.section	.far,"ax",@progbits
	.global	foo
	.type	foo, %function
foo:
	bl baz
bar:
	bl baz
	bl foo

	.section	.note.gnu.property,"a"
	.align	3
	.word	4
	.word	16
	.word	5
	.string	"GNU"
	.word	0xc0000000
	.word	4
	.word	1
	.align	3
