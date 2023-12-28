# Test the %hi, lo and %hiadj relocations

.set noat

.text
.global _start
_start:
	addi r1, r1, %hi(long_symbol)
	addi r1, r1, %lo(long_symbol)
	addi r1, r1, %hiadj(long_symbol)
