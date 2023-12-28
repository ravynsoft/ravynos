	.text
	.global postmod
postmod:
	ldrd    r0,[r1],r2      ; tricky because r1 is implied as destination

        strb    r12,[r12],r3    ; stores are okay
        strd    r12,[r13],r3

        ldr     r0,[r0],r0      ; ERROR

        ldr     r0,[r0,+128]    ; ok
        ldrd    r12,[r13],-256  ; ERROR
	ldrb    r12,[r12],20    ; ERROR
        strd    r12,[r13],-256  ; ok
