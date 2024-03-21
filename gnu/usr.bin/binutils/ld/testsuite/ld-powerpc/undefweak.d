#ld: tmpdir/empty.so
#readelf: --dyn-syms -r -W

#...
.* R_PPC(|64)_ADDR(32|64) .* a \+ 0
#...
.* R_PPC(|64)_JMP_SLOT .* b \+ 0
#...
.* WEAK +DEFAULT +UND b
.* WEAK +DEFAULT +UND a
