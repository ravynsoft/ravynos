#source: expdref1.s
#as: --pic --no-underscore --em=criself -I$srcdir/$subdir
#ld: --shared -m crislinux --hash-style=sysv
#ld_after_inputfiles: tmpdir/libdso-1b.so
#objdump: -s -T

# A DSO linked to another DSO that has two versioned symbols, to which
# this DSO refers with two relocs each, a GOT and a PLT reference.
# There was a bug such that GOT markups were lost, resulting in wrong
# offsets into the GOT, with a tell-tale sign being one or more
# R_CRIS_NONE relocs.  There should be two GOT relocs here, one each
# for the symbols.

.*:     file format elf32-cris

DYNAMIC SYMBOL TABLE:
#...
0+[ 	]+DF \*UND\*	0+[ 	]+\(TST2\)[	 ]+expobj
0+[ 	]+DF \*UND\*	0+[ 	]+\(TST2\)[	 ]+expfn
#...
Contents of section .rela.dyn:
 0140 50220000 0a020000 00000000 54220000  .*
 0150 0a030000 00000000                    .*
Contents of section .plt:
 0158 fce17e7e 0401307a 08013009 00000000  .*
 0168 00000000 6f0d0c00 00003009 3f7e0000  .*
 0178 00002ffe ecffffff 6f0d1000 00003009  .*
 0188 3f7e0000 00002ffe ecffffff           .*
Contents of section .text:
 0194 6fae0c00 00006fae ccffffff 6fae1000  .*
 01a4 00006fae d4ffffff                    .*
#...
Contents of section .got:
 2244 ac210000 00000000 00000000 00000000  .*
 2254 00000000                             .*
