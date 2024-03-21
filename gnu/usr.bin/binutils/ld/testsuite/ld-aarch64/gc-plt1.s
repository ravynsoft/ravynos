plt_relocs:
	adrp x0, :pg_hi21:var
	ldr  x0, [x0, :lo12:var]

	adrp x1, :pg_hi21_nc:var
	ldr  x1, [x1, :lo12:var]

	b bar
	bl foo
