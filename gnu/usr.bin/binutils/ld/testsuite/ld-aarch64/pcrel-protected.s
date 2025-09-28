.protected protected_a, protected_b, protected_c
.type protected_b, %object
.type protected_c, %function

.text
	adrp    x0, protected_a
	add     x0, x0, :lo12:protected_a
	adrp    x0, protected_b
	add     x0, x0, :lo12:protected_b
	adrp    x0, protected_c
	add     x0, x0, :lo12:protected_c
