# Test relaxation of CALL instructions into branches.
        .text
foo:
        sethi %hi(0), %i0
        call	bar, 0
          restore %i0, %lo(0), %o0
bar:
        sethi %hi(0), %i0
        call	_undefined, 0
	  restore %i0, %lo(0), %o0
