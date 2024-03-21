	.type start,"function"
	.global start
start:
	.type _start,"function"
	.global _start
_start:
	.type __start,"function"
	.global __start
__start:
	.type main,"function"
	.global main
main:
  .ifdef UNDERSCORE
	.dc.a ___bss_start
  .else
	.dc.a __bss_start
  .endif
