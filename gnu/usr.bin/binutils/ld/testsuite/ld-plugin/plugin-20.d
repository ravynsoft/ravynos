hook called: all symbols read.
Input: func.c \(tmpdir[/\\]libfunc.a\)
Sym: '_?func' Resolution: LDPR_PREVAILING_DEF.*
Sym: '_?func' Resolution: LDPR_PREVAILING_DEF.*
.*: tmpdir/main.o: in function `main':
.*main.c.*: undefined reference to `\.?func'
#?.*main.c.*: undefined reference to `\.?func'
hook called: cleanup.
