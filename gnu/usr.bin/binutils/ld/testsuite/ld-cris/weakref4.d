#source: start1.s
#source: expdref4.s
#as: --no-underscore --em=criself
#ld: -m crislinux --hash-style=sysv
#ld_after_inputfiles: tmpdir/libdso-15.so
#readelf: -a -x 11

# Like weakref3.d, but just the expobj2 referenced from .data.  We
# should avoid a copy reloc (instead emitting a R_CRIS_GLOB_DAT or
# R_CRIS_32 against the weak symbol), but for the time being, make
# sure we get a valid reloc.

#...
 +\[[0-9]+\] .data +PROGBITS +[0-9a-f]* [0-9a-f]* 000004 .*
#...
 +\[[0-9]+\] .bss +NOBITS +[0-9a-f]* [0-9a-f]* 00000c .*
#...
Relocation section '.rela.dyn' at offset 0x... contains 1 entry:
#...
.* R_CRIS_COPY .* __expobj2@TST3 \+ 0

The decoding of unwind sections for machine type Axis Communications 32-bit embedded processor is not currently supported.

Symbol table '.dynsym' contains . entries:
#...
 +.: [0-9a-f]* +4 +OBJECT +GLOBAL +DEFAULT +12 __expobj2@TST3 \(2\)
#...
Symbol table '.symtab' contains .. entries:
#...
Hex dump of section '.data':
  0x00082250 54220800                            .*

