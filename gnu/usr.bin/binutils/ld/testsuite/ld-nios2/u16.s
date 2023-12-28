# Test for Nios II 32-bit, 16 and 8-bit relocations

.set noat
.set some_other_sym, 0x1000
.text
.global _start
_start:
# unsigned 16-bit relocation
	andi r1, r1, some_sym
	andi r1, r1, min
	andi r1, r1, max
	andi r1, r1, some_sym + some_other_sym + 1
	andi r1, r1, some_sym - some_other_sym + 1



