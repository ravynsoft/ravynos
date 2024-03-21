    MOV r1, r0
    LDR =garbage // no destination register
    LDR =garbage // and it should only warn once
    MOV r2, r3

	// The warning should only be triggered by a "foo = bar"
	// type of expression.  Other ways of creating symbols
	// should allow ARM instruction names.
b:
   nop

.set   bx,  fred
.equiv ldr, bar
.eqv   nop, fred
.equ   mov, foo
