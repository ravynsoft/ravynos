        nop
foo:
	nop
        dbne  d0,    foo
        dbne  x,     foo
        dbne  y,     foo
        dbne.b (+y), foo
