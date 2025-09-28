# Test the hi16/lo16 relocations

.text
.global __start
__start:
	movih	r1, (long_symbol) >> 16
	ori	r1, r1, (long_symbol) & 0xffff
