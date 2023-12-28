.text
.global _start
.type _start, %function
_start:
	nop
.reloc 0, R_AARCH64_NONE, foo

.section .foo,"ax"
.global foo
foo:
	nop

.section .bar,"ax"
.global bar
bar:
	nop
