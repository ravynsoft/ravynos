.data
	.global	bar
	.balign 2
bar:
	.short	42
	.short	43

	.global foo
foo:
.skip 0xff

	.global	foo2
	.balign 2
foo2:
	.short	4

.text

  .balign 2
  .global byte
byte:
  .word foo-bar
  .word foo2-bar

  .global uleb
  .balign 2
uleb:
	.uleb128 foo-bar  ; this value can be stored in one byte
	.uleb128 foo2-bar ; this value requires 2 bytes

  .balign 2
  .global _start
  _start:
  nop
