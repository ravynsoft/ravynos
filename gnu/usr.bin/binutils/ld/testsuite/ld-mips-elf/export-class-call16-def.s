	.text
	.balign		16
	.xdef		protected_foo
	.protected	protected_foo
	.ent		protected_foo
protected_foo:
	jr		$31
	.end		protected_foo
	.balign		16
	.xdef		hidden_foo
	.hidden		hidden_foo
	.ent		hidden_foo
hidden_foo:
	jr		$31
	.end		hidden_foo
	.balign		16
	.xdef		internal_foo
	.internal	internal_foo
	.ent		internal_foo
internal_foo:
	jr		$31
	.end		internal_foo
