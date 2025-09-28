#readelf: -Ssrx1

There are 7 section headers, starting at offset .*:

Section Headers:
 +\[Nr\] +Name +Type +Address +Offset
 +Size +EntSize +Flags +Link +Info +Align
 +\[ 0\] +NULL +0000000000000000 +00000000
 +0000000000000000 +0000000000000000 +0 +0 +0
 +\[ 1\] \.text +PROGBITS +0000000000000000 +00000040
 +0000000000000008 +0000000000000000 +AX +0 +0 +4
 +\[ 2\] \.data +PROGBITS +0000000000000000 +00000048
 +0000000000000000 +0000000000000000 +WA +0 +0 +1
 +\[ 3\] \.bss +NOBITS +0000000000000000 +00000048
 +0000000000000000 +0000000000000000 +WA +0 +0 +1
 +\[ 4\] \.symtab +SYMTAB +0000000000000000 .*
 +0000000000000090 +0000000000000018 +5 +4 +8
 +\[ 5\] \.strtab +STRTAB +0000000000000000 .*
 +000000000000001a +0000000000000000 +0 +0 +1
 +\[ 6\] \.shstrtab +STRTAB +0000000000000000 +[0-9a-f]+
 +000000000000002c +0000000000000000 +0 +0 +1
Key to Flags:
#...

There are no relocations in this file\.

Symbol table '\.symtab' contains 6 entries:
 +Num: +Value +Size +Type +Bind +Vis +Ndx +Name
 +0: 0+ +0 +NOTYPE +LOCAL +DEFAULT +UND 
 +1: 0+ +0 +SECTION +LOCAL +DEFAULT +1.*
 +2: 0+ +0 +SECTION +LOCAL +DEFAULT +2.*
 +3: 0+ +0 +SECTION +LOCAL +DEFAULT +3.*
 +4: 0+4 +0 +FUNC +GLOBAL +DEFAULT +1 Main
 +5: 0+100 +0 +NOTYPE +GLOBAL +DEFAULT +ABS __\.MMIX\.start\.\.text

Hex dump of section '\.text':
  0x00000000 fd010102 fd000070                   .*
