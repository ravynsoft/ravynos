	.data
	.balign		16
	.dc.a		protected_foo
	.balign		16
	.dc.a		protected_bar
	.balign		16
	.dc.a		hidden_foo
	.balign		16
	.dc.a		hidden_bar
	.balign		16
	.dc.a		internal_foo
	.balign		16
	.dc.a		internal_bar
	.balign		32
	.xdef		protected_baz
	.protected	protected_baz
protected_baz:
	.balign		32
	.xdef		hidden_baz
	.hidden		hidden_baz
hidden_baz:
	.balign		32
	.xdef		internal_baz
	.internal	internal_baz
internal_baz:
