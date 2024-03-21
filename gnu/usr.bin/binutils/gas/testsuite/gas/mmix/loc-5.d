#readelf: -Ssrx1 -x2
There are 7 section headers, starting at offset .*:

Section Headers:
 +\[Nr\] +Name +Type +Address +Offset
 +Size +EntSize +Flags +Link +Info +Align
 +\[ 0\] +NULL +0+ +0+
 +0+ +0+ +0 +0 +0
 +\[ 1\] \.text +PROGBITS +0+ +0+40
 +0+8 +0+ +AX +0 +0 +4
 +\[ 2\] \.data +PROGBITS +0+ +0+48
 +0+24 +0+ +WA +0 +0 +4
 +\[ 3\] \.bss +NOBITS +0+ +0+6c
 +0+ +0+ +WA +0 +0 +1
 +\[ 4\] \.symtab +SYMTAB +0+ .*
 +0+c0 +0+18 +5 +6 +8
 +\[ 5\] \.strtab +STRTAB +0+ .*
 +0+27 +0+ +0 +0 +1
 +\[ 6\] \.shstrtab +STRTAB +0+ +[0-9a-f]+
 +0+2c +0+ +0 +0 +1
Key to Flags:
#...

There are no relocations in this file\.

Symbol table '\.symtab' contains 8 entries:
 +Num: +Value +Size +Type +Bind +Vis +Ndx +Name
 +0: 0+ +0 +NOTYPE +LOCAL +DEFAULT +UND 
 +1: 0+ +0 +SECTION +LOCAL +DEFAULT +1.*
 +2: 0+ +0 +SECTION +LOCAL +DEFAULT +2.*
 +3: 0+ +0 +SECTION +LOCAL +DEFAULT +3.*
 +4: 0+ +0 +NOTYPE +LOCAL +DEFAULT +1 t
 +5: 2000000000000000 +0 +NOTYPE +LOCAL +DEFAULT +ABS Data_Segment
 +6: 0+4 +0 +FUNC +GLOBAL +DEFAULT +1 Main
 +7: 2000000000000000 +0 +NOTYPE +GLOBAL +DEFAULT +ABS __\.MMIX\.start\.\.data

Hex dump of section '\.text':
  0x0+ fd001807 fd090101                   .*

Hex dump of section '\.data':
  0x0+ 00000100 00000000 00000000 00000000 .*
  0x00000010 00000000 00000000 00000000 00000000 .*
  0x00000020 00000038                            .*
