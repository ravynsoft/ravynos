	.syntax unified
	.text
	.thumb

.global foo
foo:
	nop
	bl  log_func
	b.n .L1
	bl  func

.global func
func:
	nop
.L1:
	nop
