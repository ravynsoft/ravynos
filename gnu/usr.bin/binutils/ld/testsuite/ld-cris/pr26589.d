#source: dso-1.s
#source: locref3.s
#as: --pic --no-underscore --em=criself
#ld: --shared -m crislinux
#readelf: --dyn-syms -r

# Besides typical DSO stuff (libdso-1.d), we here have a data section
# with an absolute reloc to a local symbol.  For the original
# test-case, this happened for a destructor (.dtors).

Relocation section '\.rela\.dyn' at offset 0x[0-9a-f]+ contains 1 entry:
 Offset[ 	]+Info[ 	]+Type[ 	]+Sym\.Value  Sym\. Name \+ Addend
[a-f0-9]+[ ]+0+c R_CRIS_RELATIVE[ ]+ [a-f0-9]+

Symbol table '\.dynsym' contains 3 entries:
#pass
