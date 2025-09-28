.globl _start
_start:
  .ifdef UNDERSCORE
	.dc.a	___start__foo
  .else
	.dc.a	__start__foo
  .endif
	.section	_foo,"aw",%progbits
foo:
	.ascii "This is "
