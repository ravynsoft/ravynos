	.data
	.type foo,%object
foo:
	.byte 0
	.size foo,.-foo
	.globl foo
	.symver foo,foo@@VERS_1,remove
	.globl bar
	.symver bar,bar@VERS_1,remove
	.type bar,%object
bar:
	.byte 0
	.size bar,.-bar
	.balign 8
	.dc.a foo
	.dc.a bar
