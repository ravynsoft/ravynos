#source: tls-gd-2.s --pic
#as: --no-underscore --em=criself
#ld: -m crislinux --shared --hash-style=sysv
#readelf: -a -x 7

# Undefined reference for a R_CRIS_32_GOT_GD in a DSO.  Not an error;
# it's ok for a DSO to have undefined references upon creation for
# global symbols that can be overridden.  Just make sure GOT, dynsyms
# and dynrelocs look right.

#...
Relocation section '.rela.dyn' at offset 0x.* contains 1 entry:
 Offset +Info +Type +Sym.Value +Sym. Name \+ Addend
00002190 +00000217 R_CRIS_DTP +00000000 +x \+ 0

The decoding of unwind sections for machine type Axis Communications 32-bit embedded processor is not currently supported.

Symbol table '.dynsym' contains . entries:
 +Num: +Value +Size +Type +Bind +Vis +Ndx +Name
#...
 +[0-9]: 00000000 +0 +TLS +GLOBAL +DEFAULT +UND x
#...
Symbol table '.symtab' contains .. entries:
#...
Hex dump of section '.got':
  0x0+2184 14210000 00000000 00000000 00000000 .*
  0x0+2194 00000000                            .*
