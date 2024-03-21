# Test LDI32 relaxation

	.text
	.global _start
_start:
	ldi32	r16, long_symbol
__intermediate:
	loop	__end_loop, r22
	ldi32	r16, long_symbol
	ldi32	r16, short_symbol
	ldi	r0, short_symbol
	ldi32	r16, short_symbol + 0x10000
	ldi32	r16, long_symbol - 0x10000
	ldi32	r16, 0x12345678
	ldi32	r16, 0x5678
	ldi	r16, %pmem(__end)
__end_loop:
	qba	__intermediate
__end:

	.data
	.4byte	__end
	.4byte	(__end - __intermediate)
	.2byte	%pmem(__end)
	.2byte	(__end - __intermediate)
	.4byte	%pmem(__end - __intermediate)
	.4byte	%pmem(__intermediate - __end)
	.2byte	%pmem(__end - __intermediate)
	.byte	(__end - __intermediate)
	.byte	0xaa
