	.text
	.global foo
	.global bar
	.global alias_foo
	.global alias_bar
	.variant_cc foo
	.variant_cc alias_foo
	.set alias_bar, foo
	.set alias_foo, bar
foo:
bar:
	call	foo
	call	bar
	call	alias_foo
	call	alias_bar
