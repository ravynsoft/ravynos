.set noat

.sdata

sym1:
.long 0xdead
sym2:
.long 0xbeef
sym3:
.byte 0x7f

.section .sbss, "w"
sym4:
.long 0
sym5:
.long 0
sym6:
.byte 0

.text
.global _start
_start:
	movui gp, _gp
	ldw r1, %gprel(sym1)(gp)
	ldw r2, %gprel(sym2)(gp)
	ldb r3, %gprel(sym3)(gp)
	ldw r1, %gprel(sym4)(gp)
	ldw r2, %gprel(sym5)(gp)
	ldb r3, %gprel(sym6)(gp)
