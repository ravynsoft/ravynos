#source: startv1.s
#source: funref2.s
#as: -a64
#ld: -melf64ppc --emit-stub-syms
#ld_after_inputfiles: tmpdir/funv1.so
#warning: .*requires lazy plt linking.*
#readelf: -rs --wide
# Check that we do the right thing with funref2.s that doesn't have
# anything to mark it as ELFv1 or ELFv2.  Since my_func address is
# taken in a read-only section we should get a copy reloc for the OPD
# entry.

Relocation section .* contains 1 entry:
.*
.* R_PPC64_COPY .* my_func \+ 0

Symbol table '\.dynsym' contains 2 entries:
#...
.*: 0*[1-9a-f][0-9a-f]* +24 FUNC +GLOBAL DEFAULT +1[23] my_func
#...
Symbol table '\.symtab' contains .* entries:
#...
.*: 0*[1-9a-f][0-9a-f]* +24 FUNC +GLOBAL DEFAULT +1[23] my_func
#pass
