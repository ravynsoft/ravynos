	.data
	.balign		32
	.xdef		protected_bar
	.protected	protected_bar
protected_bar:
	.balign		32
	.xdef		protected_foo
	.protected	protected_foo
protected_foo:
	.balign		32
	.xdef		hidden_bar
	.hidden		hidden_bar
hidden_bar:
	.balign		32
	.xdef		hidden_foo
	.hidden		hidden_foo
hidden_foo:
	.balign		32
	.xdef		internal_bar
	.internal	internal_bar
internal_bar:
	.balign		32
	.xdef		internal_foo
	.internal	internal_foo
internal_foo:
