	.text
	.globl		protected_foo
	.type		protected_foo, @function
	.protected	protected_foo
protected_foo:
	.word		0
	ret
	.size		protected_foo, . - protected_foo
	.globl		hidden_foo
	.type		hidden_foo, @function
	.hidden		hidden_foo
hidden_foo:
	.word		0
	ret
	.size		hidden_foo, . - hidden_foo
	.globl		internal_foo
	.type		internal_foo, @function
	.internal	internal_foo
internal_foo:
	.word		0
	ret
	.size		internal_foo, . - internal_foo
