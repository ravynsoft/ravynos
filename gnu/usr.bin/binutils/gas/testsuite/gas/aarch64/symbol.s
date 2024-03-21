// symbol.s Test file for the parsing of symbols

	.struct 0
$codesize:
	.space 4
$CPU_mode:
	.space 4
$entry_point:
	.space 8
CPU_mode:
	.space 4

.text
	ldr	w1, [x0, #$CPU_mode]
	ldr	w1, [x0, $CPU_mode]
	ldr	w1, [x0, #CPU_mode]
	ldr	w1, [x0, CPU_mode]

	// Symbol that has the same name as that of a register
	// is allowed as long as there is no ambiguity.
.set	x2, 10
	add	x0, x1, x2
	add	x0, x1, #x2
.set	s2, 11
	sub	x0, x1, s2
