# Source file to test branch swapping with the ALNV.PS instruction.

	.text
foo:
	alnv.ps	$f4, $f2, $f0, $3
	b	foo
	alnv.ps	$f4, $f2, $f0, $3
	bal	foo
	alnv.ps	$f4, $f2, $f0, $3
	bltzal	$3, foo
	alnv.ps	$f4, $f2, $f0, $3
	jalr	$3
	alnv.ps	$f4, $f2, $f0, $3
	jalr	$4, $3
	alnv.ps	$f4, $f2, $f0, $3
	jalr	$3, $31

	alnv.ps	$f4, $f2, $f0, $31
	b	foo
	alnv.ps	$f4, $f2, $f0, $31
	bal	foo
	alnv.ps	$f4, $f2, $f0, $31
	bltzal	$3, foo
	alnv.ps	$f4, $f2, $f0, $31
	jalr	$3
	alnv.ps	$f4, $f2, $f0, $31
	jalr	$4, $3
	alnv.ps	$f4, $f2, $f0, $31
	jalr	$3, $31

# Force at least 8 (non-delay-slot) zero bytes, to make 'objdump' print ...
	.align	2
	.space	8
