#source: dso-1.s
#source: dso-2.s
#as: --pic --no-underscore --em=criself
#ld: --shared -m crislinux --hash-style=sysv --version-script $srcdir/$subdir/hide1
#readelf: -S -s -r

# Use "dsofn" from dso-1 in a GOTPLT reloc, but hide it in a
# version script.  This will change the incoming GOTPLT reloc to
# instead be a (local) GOT reloc.  There are no other .rela.got
# entries.  This formerly SEGV:ed because .rela.got was created
# too late to have it mapped to an output section.

There are 13 section headers.*
#...
 +\[ 1\] \.hash +HASH +.*
 +\[ 2\] \.dynsym +DYNSYM +.*
 +\[ 3\] \.dynstr +STRTAB +.*
 +\[ 4\] \.gnu\.version +VERSYM +.*
 +\[ 5\] \.gnu\.version_d +VERDEF +.*
 +\[ 6\] \.rela\.dyn +RELA +.*
 +\[ 7\] \.text +PROGBITS .*
 +\[ 8\] \.dynamic +DYNAMIC +.*
 +\[ 9\] \.got +PROGBITS .*
 +\[10\] \.symtab +SYMTAB +.*
 +\[11\] \.strtab +STRTAB +.*
 +\[12\] \.shstrtab +STRTAB +.*
#...
Relocation section '\.rela\.dyn' at offset 0x[0-9a-f]+ contains 1 entry:
#...
00002[12][0-9a-f][048c] +0000000c R_CRIS_RELATIVE +150
#...
Symbol table '\.dynsym' contains 4 entries:
 +Num: +Value +Size +Type +Bind +Vis +Ndx +Name
 +0: 0+ +0 +NOTYPE +LOCAL +DEFAULT +UND 
 +1: [0-9a-f]+ +0 +SECTION +LOCAL +DEFAULT +7.*
 +2: 0+ +0 +OBJECT +GLOBAL +DEFAULT +ABS TST1
 +3: 0+154 +0 +FUNC +GLOBAL +DEFAULT +7 export_1@@TST1

Symbol table '\.symtab' contains 15 entries:
 +Num: +Value +Size +Type +Bind +Vis +Ndx +Name
 +0: 0+ +0 +NOTYPE +LOCAL +DEFAULT +UND 
 +1: [0-9a-f]+ +0 +SECTION +LOCAL +DEFAULT +1.*
 +2: [0-9a-f]+ +0 +SECTION +LOCAL +DEFAULT +2.*
 +3: [0-9a-f]+ +0 +SECTION +LOCAL +DEFAULT +3.*
 +4: [0-9a-f]+ +0 +SECTION +LOCAL +DEFAULT +4.*
 +5: [0-9a-f]+ +0 +SECTION +LOCAL +DEFAULT +5.*
 +6: [0-9a-f]+ +0 +SECTION +LOCAL +DEFAULT +6.*
 +7: [0-9a-f]+ +0 +SECTION +LOCAL +DEFAULT +7.*
 +8: [0-9a-f]+ +0 +SECTION +LOCAL +DEFAULT +8.*
 +9: [0-9a-f]+ +0 +SECTION +LOCAL +DEFAULT +9.*
 +10: 0+2..[046c] +0 +OBJECT +LOCAL +DEFAULT +ABS _DYNAMIC
 +11: 0+2..[046c] +0 +OBJECT +LOCAL +DEFAULT +ABS _GLOBAL_OFFSET_TABLE_
 +12: 0+150 +2 +FUNC +LOCAL +DEFAULT +7 dsofn
 +13: 0+ +0 +OBJECT +GLOBAL +DEFAULT +ABS TST1
 +14: 0+154 +0 +FUNC +GLOBAL +DEFAULT +7 export_1
