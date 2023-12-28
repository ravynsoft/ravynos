#source: tls-ie-8.s --pic
#source: tls128.s
#source: tls-hx.s
#as: --no-underscore --em=criself
#ld: -m crislinux --shared --hash-style=sysv
#readelf: -a -x 6 -x 8 -x 5

# A R_CRIS_16_GOT_TPREL in a DSO against a hidden symbol, at an offset
# into .data.  Make sure the relocation, GOT, .text and .tdata have
# the right contents.

#...
Relocation section '.rela.dyn' at offset 0x.* contains 1 entry:
 Offset     Info    Type            Sym.Value  Sym. Name \+ Addend
00002220  0000001c R_CRIS_32_TPREL[ ]+80

The decoding of unwind sections for machine type Axis Communications 32-bit embedded processor is not currently supported.

Symbol table '.dynsym' contains 3 entries:
   Num:    Value  Size Type    Bind   Vis      Ndx Name
#...
Symbol table '.symtab' contains [0-9]+ entries:
#...
    ..: 00000080 +4 +TLS +LOCAL +DEFAULT +6 x
#...
Hex dump of section '.text':
  0x00000114 5fae0c00                            .*
#...
Hex dump of section '.tdata':
  0x00002118 2f0+ 0+ 0+ 0+ .*
  0x00002128 0+ 0+ 0+ 0+ .*
#...
  0x00002188 0+ 0+ 0+ 0+ .*
  0x00002198 280+                            .*
#...
Hex dump of section '.got':
  0x0+2214 9c210000 0+ 0+ 80+ .*
