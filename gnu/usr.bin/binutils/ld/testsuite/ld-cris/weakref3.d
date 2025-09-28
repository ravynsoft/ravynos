#source: start1.s
#source: expdref3.s
#as: --no-underscore --em=criself
#ld: -m crislinux --hash-style=sysv
#ld_after_inputfiles: tmpdir/libdso-15.so
#readelf: -a -x 10

# Like libdso-15b.d, but referencing the weak symbol and function from
# a program.  At some time we broke emitting a copy reloc for the
# object, instead yielding NULL.

#...
 +\[[0-9]+\] .got +PROGBITS +[0-9a-f]* [0-9a-f]* 000010 .*
#...
 +\[[0-9]+\] .bss +NOBITS +[0-9a-f]* [0-9a-f]* 000010 .*
#...
Relocation section '.rela.dyn' at offset 0x... contains 1 entry:
 Offset +Info +Type +Sym.Value +Sym. Name \+ Addend
.* R_CRIS_COPY .* __expobj2@TST3 \+ 0

Relocation section '.rela.plt' at offset 0x... contains 1 entry:
 Offset +Info +Type +Sym.Value +Sym. Name \+ Addend
.* R_CRIS_JUMP_SLOT .* expfn2@TST3 \+ 0

The decoding of unwind sections for machine type Axis Communications 32-bit embedded processor is not currently supported.

Symbol table '.dynsym' contains . entries:
#...
 +.: [0-9a-f]* +4 +OBJECT +GLOBAL +DEFAULT +13 __expobj2@TST3 \(2\)
#...
 +.: [0-9a-f]* +0 +FUNC +GLOBAL +DEFAULT +UND expfn2@TST3 \(2\)
#...
Symbol table '.symtab' contains .. entries:
#...
Hex dump of section '\.text':
  0x000801f8 41b20000 6faed022 08006fae e4010800 .*
