# Test for Nios II 32-bit, 16 and 8-bit relocations

.set noat
.set some_other_sym, 0x1000
.text
.global _start
_start:
# signed 16-bit relocation
	addi r1, r1, some_sym
	addi r1, r1, min
	addi r1, r1, max
	addi r1, r1, some_sym + some_other_sym + 1
	addi r1, r1, some_sym - some_other_sym + 1



