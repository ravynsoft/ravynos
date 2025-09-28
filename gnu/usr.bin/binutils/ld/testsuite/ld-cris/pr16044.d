#source: dso-4.s
#source: dso-2b.s
#source: dso-1c.s
#as: --pic --no-underscore --em=criself -I$srcdir/$subdir
#ld: --shared -m crislinux --hash-style=sysv
#readelf: -s -r

# PR 16044 is about a (compile-time-non-local) hidden function symbol,
# entered as an undef reference with a R_CRIS_32_PLT_GOTREL relocation
# referring to a hidden symbol, later defined.  Here, we invalidly
# incremented the h->plt.refcount (from -1) as part of that relocation
# processing.  There are some PLTGOT relocations.  As there are no
# circumstances requiring a PLT entry for this symbol, its PLT entry
# can be eliminated and the PLTGOT relocations can be made to a static
# element in the GOT, relocated with the absolute-to-relative
# R_CRIS_RELATIVE relocation without symbol lookup.  As part of
# eliminating unneeded PLT entries (and PLTGOT to "static" GOT
# elimination), a later pass noticed the inconsistency through an
# assert.
#
# The key points in this dump that may need future adjustments are the
# single dynamic relocation, that the dsofn symbol it points to, is
# local, its absence from the dynamic symbol table and that the
# relocation and symbol values match.

Relocation section '\.rela\.dyn' at offset 0x[0-9a-f]+ contains 1 entry:
 Offset[ 	]+Info[ 	]+Type[ 	]+Sym\.Value  Sym\. Name \+ Addend
[0-9a-f]+  0+[0-9a-f]+ R_CRIS_RELATIVE[ 	]+128

Symbol table '\.dynsym' contains 4 entries:
 +Num: +Value +Size +Type +Bind +Vis +Ndx +Name
 +0: 0+ +0 +NOTYPE +LOCAL +DEFAULT +UND 
 +1: [0-9a-f]+ +0 +SECTION +LOCAL +DEFAULT +5.*
 +2: [0-9a-f]+ +0 +FUNC +GLOBAL +DEFAULT +5 export_1
 +3: [0-9a-f]+ +0 +FUNC +GLOBAL +DEFAULT +5 export_2

Symbol table '\.symtab' contains [0-9]+ entries:
#...
 +[0-9]+: 0+128  +2 FUNC + LOCAL + DEFAULT + 5 dsofn
#...
